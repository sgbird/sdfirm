/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2019
 *    ZETALOG - "Lv ZHENG".  All rights reserved.
 *    Author: Lv "Zetalog" Zheng
 *    Internet: zhenglv@hotmail.com
 *
 * This COPYRIGHT used to protect Personal Intelligence Rights.
 * Redistribution and use in source and binary forms with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the Lv "Zetalog" ZHENG.
 * 3. Neither the name of this software nor the names of its developers may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 4. Permission of redistribution and/or reuse of souce code partially only
 *    granted to the developer(s) in the companies ZETALOG worked.
 * 5. Any modification of this software should be published to ZETALOG unless
 *    the above copyright notice is no longer declaimed.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ZETALOG AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE ZETALOG OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)bench.c: CPU test bench implementation
 * $Id: bench.c,v 1.0 2019-12-20 11:18:00 zhenglv Exp $
 */

#include <target/bench.h>
#include <target/barrier.h>
#include <target/bh.h>
#include <target/delay.h>
#include <target/paging.h>
#include <target/smp.h>
#include <target/spinlock.h>
#include <target/percpu.h>
#include <target/timer.h>
#include <target/page.h>
#include <target/cmdline.h>

#define CMD_INVALID_CPU_MASKS	0

#define CPU_OFF			0
#define CPU_ON			1
#define CPU_RUN			2

/* CPU control states */
#define CPU_STATE_HALT		0
#define CPU_STATE_NONE		1
#define CPU_STATE_DIDT		2

/* CPU control events */
#define CPU_EVENT_START		(1<<0)
#define CPU_EVENT_TIME		(1<<1)
#define CPU_EVENT_SYNC		(1<<2)
#define CPU_EVENT_STOP		(1<<3)

#define __printf(...)		do { } while (0)
#ifdef CONFIG_TEST_BENCH_DEBUG
#define do_printf		printf
#else
#define do_printf		__printf
#endif

#ifdef CONFIG_TEST_BENCH_START_DELAY
#define BENCH_START_DELAY_MS	CONFIG_TEST_BENCH_START_DELAY
#else
#define BENCH_START_DELAY_MS	0 /* non-SMP mode, no start aligning */
#endif

struct cpu_context {
	uint8_t async_state;
	uint8_t async_event;
	tick_t async_timeout;
	tick_t async_exec_period;
	tick_t async_wait_interval;
	tick_t async_start_delay;
	struct cpu_exec_test *didt_entry;
	int didt_result;
	int didt_repeats;
	tid_t timer;
	bh_t bh;
} __cache_aligned;

static void __bench_sync(unsigned long sync_state,
			 uint64_t this_cpu_mask, bool entry_or_exit,
			 uint32_t flags, bool wait, tick_t timeout);
static caddr_t bench_percpu_area(cpu_t cpu);

static struct cpu_context cpu_ctxs[NR_CPUS];

static uint64_t cmd_bench_cpu_mask = 0;
static int cmd_bench_repeats = 0;
static struct cpu_exec_test *cmd_bench_test_set = NULL;
static tick_t cmd_bench_timeout = CPU_WAIT_INFINITE;
static tick_t cmd_bench_interval = CPU_WAIT_INFINITE;
static tick_t cmd_bench_start_delay = BENCH_START_DELAY_MS;

/* Variables for testos_cpu_state:
 * closed: all CPUs have no synchronous pattern running on them
 * opened: all CPUs are about to execute the synchronous pattern
 */
#define CPU_EXEC_CLOSED	0
#define CPU_EXEC_OPENED	1

#define CPU_EXEC_IDLE	0
#define CPU_EXEC_PEND	1
#define CPU_EXEC_BUSY	2

/* The following global variables are not good parallel programming
 * examples, should be tuned to gain maximum performance.
 */
static unsigned long cpu_exec_stage = CPU_EXEC_IDLE;
static unsigned long cpu_exec_state;
static DEFINE_SPINLOCK(cpu_exec_lock);
static uint64_t cpu_exec_sync;
static unsigned long cpu_exec_good;
static int cpu_exec_test_id;
static int cpu_didt_pages;
static struct page *cpu_didt_alloc;
static uint8_t cpu_didt_refcnt;
static unsigned long cpu_didt_cpu_mask;

#ifdef CONFIG_TEST_BENCH_DEBUG
static const char *bench_event_name(uint8_t event)
{
	switch (event) {
	case CPU_EVENT_START:
		return "START";
	case CPU_EVENT_TIME:
		return "TIME";
	case CPU_EVENT_SYNC:
		return "SYNC";
	case CPU_EVENT_STOP:
		return "STOP";
	default:
		return "UNKNOWN";
	}
}

static const char *bench_state_name(uint8_t state)
{
	switch (state) {
	case CPU_STATE_HALT:
		return "HALT";
	case CPU_STATE_NONE:
		return "NONE";
	case CPU_STATE_DIDT:
		return "DIDT";
	default:
		return "UNKNOWN";
	}
}
#endif

static void bench_raise_event(uint8_t event)
{
	cpu_t cpu = smp_processor_id();

	do_printf("Event %s\n", bench_event_name(event));
	cpu_ctxs[cpu].async_event |= event;
	bh_resume(cpu_ctxs[cpu].bh);
}

static void inline cpu_exec_open(uint64_t cpu_mask, bool is_entry,
				 uint32_t flags, bool wait,
				 tick_t timeout)
{
	__bench_sync(CPU_EXEC_OPENED, cpu_mask, is_entry,
		     flags,  wait, timeout);
}

static void inline cpu_exec_close(uint64_t cpu_mask, bool is_exit,
				  uint32_t flags, bool wait,
				  tick_t timeout)
{
	__bench_sync(CPU_EXEC_CLOSED, cpu_mask, is_exit,
		     flags, wait, timeout);
}

#ifdef CONFIG_TEST_BENCH_DIDT
#ifdef CONFIG_TEST_BENCH_START_DELAY
#define BENCH_START_DELAY_MS	CONFIG_TEST_BENCH_START_DELAY
#else
#define BENCH_START_DELAY_MS	0 /* non-SMP mode, no start aligning */
#endif

static uint8_t cpu_exec_refcnt;

static inline tick_t __get_testfn_timeout(struct cpu_exec_test *fn,
					  tick_t timeout)
{
	if (fn->timeout != CPU_WAIT_INFINITE) {
		tick_t test_timeout = tick_get_counter() + fn->timeout;
		if (time_after(timeout, test_timeout))
			return test_timeout;
	}
	return timeout;
}

static void bench_timer_start(void)
{
	cpu_t cpu = smp_processor_id();
	tick_t current_s = tick_get_counter();

	if (cpu_ctxs[cpu].async_start_delay) {
		cpu_ctxs[cpu].async_timeout = ALIGN_UP(current_s,
			cpu_ctxs[cpu].async_start_delay);
	} else {
		cpu_ctxs[cpu].async_timeout = current_s;
	}
}

static bool bench_should_suspend(cpu_t cpu, struct cpu_exec_test *fn)
{
	tick_t end_time;
	bool is_endless;

	end_time = cpu_ctxs[cpu].async_timeout +
		   cpu_ctxs[cpu].async_exec_period;
	is_endless = !!(cpu_ctxs[cpu].async_exec_period ==
			CPU_WAIT_INFINITE);
	if (is_endless || !cpu_ctxs[cpu].async_wait_interval ||
	    time_before(tick_get_counter(), end_time)) {
		do_printf("%02d(%020lld): %s count down %d before %020lld\n",
			  cpu, tick_get_counter(), fn->name,
			  cpu_ctxs[cpu].didt_repeats, end_time);
		return false;
	}
	return true;
}

static bool bench_should_resume(cpu_t cpu)
{
	tick_t current_time;

	cpu_ctxs[cpu].async_timeout +=
		cpu_ctxs[cpu].async_wait_interval;
	current_time = tick_get_counter();
	if (time_after(current_time, cpu_ctxs[cpu].async_timeout)) {
		/* Effectively avoid dIdT mode */
		cpu_ctxs[cpu].async_timeout = current_time;
		return true;
	}
	return false;
}

static void bench_reset_timeout(cpu_t cpu)
{
	tick_t tick = tick_get_counter();

	if (!time_before(tick, cpu_ctxs[cpu].async_timeout))
		bench_raise_event(CPU_EVENT_TIME);
	else
		timer_schedule_shot(cpu_ctxs[cpu].timer,
				    cpu_ctxs[cpu].async_timeout - tick);
}

static bool __bench_sync_wait(bool wait, tick_t timeout)
{
	return !!(wait && time_after(tick_get_counter(), timeout));
}

static void bench_timer_handler(void)
{
	bench_raise_event(CPU_EVENT_TIME);
}

struct timer_desc bench_timer = {
	TIMER_BH,
	bench_timer_handler,
};

static int __do_one_testfn(struct cpu_exec_test *start,
			   int nr_tests, int *local_test_id,
			   uint64_t cpu_mask, uint32_t flags,
			   bool wait, tick_t timeout,
			   void (*notify)(bool))
{
	int i = 0, test_id;
	int ret = 1;
	tick_t expected_time = timeout;
	struct cpu_exec_test *fn;
	bool sync = false;
	cpu_t cpu = smp_processor_id();
	bool owned = false;

again:
	test_id = *local_test_id;
	if (cpu_exec_test_id == -1) {
		if (flags & CPU_EXEC_RAND)
			test_id = (rand() % nr_tests);
		else
			test_id++;
		if (flags & CPU_EXEC_SYNC)
			owned = true;
	} else {
		/* This implies CPU_EXEC_SYNC mode as the global test
		 * id is only reset in SYNC mode.
		 */
		test_id = cpu_exec_test_id;
	}
	if (test_id >= nr_tests)
		return -EEXIST;
	*local_test_id = test_id;
	fn = (struct cpu_exec_test *)((uintptr_t)start +
	     (test_id * sizeof (struct cpu_exec_test)));
	if (fn->flags & CPU_EXEC_META)
		goto again;
	if (owned)
		cpu_exec_test_id = test_id;

	if (!(flags & CPU_EXEC_SYNC)) {
		if (flags & CPU_EXEC_RAND) {
			/* SYNC+RAND mode solution 1/2 switches here:
			 * Choose to reset global test id after all CPUs
			 * seeing the selected test id.
			 * In order to have all CPUs to see the selected
			 * test id, all CPUs need to wait for each other.
			 */
			sync = true;
		} else
			sync = fn->flags & CPU_EXEC_SYNC ? true : false;
	} else
		sync = true;

	do_printf("%s %.3d/%.3d on %d\n", fn->name,
		  test_id, nr_tests, smp_processor_id());

repeat:
	if (!(cpu_exec_good & cpu_mask)) {
		ret = 0;
		goto end;
	}
	if (sync) {
		expected_time = __get_testfn_timeout(fn, timeout);
		cpu_exec_open(cpu_mask,
			      i == 0 ? true : false,
			      flags, wait, expected_time);
	}
	if (cpu_exec_test_id != -1 &&
	    flags & CPU_EXEC_SYNC) {
		/* SYNC+RAND mode solution 2:
		 * The first CPU will reset the global test id here.
		 */
		cpu_exec_test_id = -1;
	}
	spin_unlock(&cpu_exec_lock);
	if (notify)
		notify(false);
	/* TODO: The percpu area is not working for raven cases, and
	 *       raven cases are now the only pattern running with
	 *       bench_exec().
	 */
	ret = fn->func(bench_percpu_area(cpu));
	if (!ret) {
		printf("%s failure on %d\n",
		       fn->name, smp_processor_id());
		spin_lock(&cpu_exec_lock);
		cpu_exec_good &= ~cpu_mask;
		goto end;
	}
	if (notify)
		notify(true);
	do_printf("%s success on %d\n",
		  fn->name, smp_processor_id());
	spin_lock(&cpu_exec_lock);
	if (sync)
		cpu_exec_close(cpu_mask,
			       (i + 1) == fn->repeat ? true : false,
			       flags, wait, expected_time);
	if (++i < fn->repeat)
		goto repeat;
end:
	return ret;
}

static int do_one_testfn(struct cpu_exec_test *start, int nr_tests,
			 int *local_test_id, uint64_t cpu_mask,
			 uint32_t flags, bool wait, tick_t timeout,
			 void (*notify)(bool))
{
	return __do_one_testfn(start, nr_tests, local_test_id, cpu_mask,
			       flags, wait, timeout, notify);
}

/* There are several implementations for SYNC+RAND mode:
 * 1. The global test id is cleared after the last CPU having seen the
 *    selected test id.
 *    To achieve this, cpu_exec_open() need to be invoked with sync mode.
 *    Merit:  all CPUs will execute the cases with SYNC flag cleared.
 *    Demrit: cases with SYNC flag cleared will be executed in SYNC mode
 *            even if the RAND mode is also specified.
 * 2. The global test id is cleared by the first CPU seeing the selected
 *    test id.
 *    To achieve this, cpu_exec_open() need to be invoked without sync
 *    mode.
 *    Merit:  CPUs have chance to use a different order than the other
 *            CPUs to execute cases with SYNC flag cleared.
 *    Demrit: cases with SYNC flag cleared might not be executed by the
 *            CPUs if the first CPU reset the global test id too early.
 *            This is also OK as in RAND mode, only cases with SYNC flag
 *            set need to be executed altogether.
 * 3. The most complicated implementation is to ensure cases executed on
 *    all CPUs are executed in the same order with respect to the case
 *    specific SYNC flag to achieve maximum performance. There could be
 *    tow possible approaches:
 *    3.1. The first one is invoking cpu_exec_open() without sync mode, and
 *         not resetting global test is but queueing the 2nd+ test ids up.
 *    3.2. The second one is using percpu PRNG to make sure case numbers
 *         generated seperately by the different CPUs are in the same
 *         order.
 */
int bench_exec(struct cpu_exec_test *start, int nr_tests,
	       uint64_t init_cpu_mask, uint32_t flags,
	       tick_t timeout, void (*notify)(bool))
{
	int ret;
	tick_t expected_time = timeout;
	uint64_t cpu_mask = UINT64_C(1) << smp_processor_id();
	bool wait = false;
	int local_test_id;

	if (timeout != CPU_WAIT_INFINITE) {
		expected_time += tick_get_counter();
		wait = true;
	}

	srand(tick_get_counter());
	spin_lock(&cpu_exec_lock);
	if (!cpu_exec_refcnt) {
		cpu_exec_good = init_cpu_mask;
		cpu_exec_test_id = -1;
	}
	local_test_id = -1;
	cpu_exec_refcnt++;
	while (!wait ||
	       time_before(tick_get_counter(), expected_time)) {
		ret = do_one_testfn(start, nr_tests, &local_test_id,
				    cpu_mask, flags, wait,
				    expected_time, notify);
		if (ret == -EEXIST)
			break;
		if (!ret && !cpu_exec_good) {
			ret = -EFAULT;
			goto end;
		}
	}
	ret = cpu_exec_good & cpu_mask ? 0 : -EFAULT;
end:
	cpu_exec_refcnt--;
	if (!cpu_exec_refcnt)
		cpu_exec_good = 0;
	spin_unlock(&cpu_exec_lock);
	return ret;
}

static void bench_timer_init(cpu_t cpu)
{
	cpu_ctxs[cpu].timer = timer_register(&bench_timer);
}
#else
static inline tick_t __get_testfn_timeout(struct cpu_exec_test *fn,
					  tick_t timeout)
{
	return 0;
}

static void bench_reset_timeout(cpu_t cpu)
{
	cpu_ctxs[cpu].async_timeout = 0;
	bench_raise_event(CPU_EVENT_TIME);
}

static bool __bench_sync_wait(bool wait, tick_t timeout)
{
	return false;
}

static bool bench_should_suspend(cpu_t cpu, struct cpu_exec_test *fn)
{
	cpu_ctxs[cpu].async_timeout++;
	return !(cpu_ctxs[cpu].async_timeout & 1);
}

static bool bench_should_resume(cpu_t cpu)
{
	return true;
}

#define bench_timer_init(cpu)			do { } while (0)
#define bench_timer_start(cpu)			do { } while (0)
#endif

static void bench_start(void)
{
	cpu_t cpu = smp_processor_id();

	bench_timer_start();
	bench_reset_timeout(cpu);
}

static void bench_stop(void)
{
	bool locked = false;
	__unused struct page *ptr;

	spin_lock(&cpu_exec_lock);
	cpu_didt_refcnt--;
	if (!cpu_didt_refcnt) {
		locked = true;
		ptr = cpu_didt_alloc;
		cpu_didt_cpu_mask = 0;
		cpu_didt_alloc = NULL;
	}
	spin_unlock(&cpu_exec_lock);
	if (locked) {
		do_printf("free: cpuexec: %016llx(%d)\n",
			  (uint64_t)ptr, cpu_didt_pages);
		page_free_pages(ptr, cpu_didt_pages);
		spin_lock(&cpu_exec_lock);
		cpu_exec_stage = CPU_EXEC_IDLE;
		spin_unlock(&cpu_exec_lock);
	}
}

static void bench_enter_state(cpu_t cpu, uint8_t state)
{
	do_printf("State %s\n", bench_state_name(state));

	cpu_ctxs[cpu].async_state = state;
	switch (cpu_ctxs[cpu].async_state) {
	case CPU_STATE_NONE:
		break;
	case CPU_STATE_DIDT:
		bench_start();
		break;
	default:
		break;
	}
}

static caddr_t bench_percpu_area(cpu_t cpu)
{
	size_t size;

	if (!cpu_ctxs[cpu].didt_entry)
		return (caddr_t)0;
	size = ALIGN_UP(cpu_ctxs[cpu].didt_entry->alloc_size,
			cpu_ctxs[cpu].didt_entry->alloc_align);
	return (caddr_t)((uint64_t)cpu_didt_alloc + cpu * size);
}

static void __bench_exec(cpu_t cpu)
{
	struct cpu_exec_test *fn;

	if (!cpu_ctxs[cpu].didt_entry) {
		cpu_ctxs[cpu].didt_result = -EEXIST;
		bench_raise_event(CPU_EVENT_STOP);
		return;
	} else
		fn = cpu_ctxs[cpu].didt_entry;

	while (!bench_should_suspend(cpu, fn)) {
		if (!fn->func(bench_percpu_area(cpu))) {
			cpu_ctxs[cpu].didt_result = -EFAULT;
			bench_raise_event(CPU_EVENT_STOP);
			break;
		}
	}

	cpu_ctxs[cpu].didt_repeats--;
	if (cpu_ctxs[cpu].didt_repeats &&
	    cpu_ctxs[cpu].async_wait_interval != CPU_WAIT_INFINITE) {
		if (bench_should_resume(cpu))
			bench_raise_event(CPU_EVENT_TIME);
		else
			bench_reset_timeout(cpu);
	} else
		bench_raise_event(CPU_EVENT_STOP);
}

static void bench_bh_handler(uint8_t __event)
{
	cpu_t cpu = smp_processor_id();
	uint8_t event = cpu_ctxs[cpu].async_event;

	cpu_ctxs[cpu].async_event = 0;
	switch (cpu_ctxs[cpu].async_state) {
	case CPU_STATE_HALT:
		bench_didt(cmd_bench_cpu_mask, cmd_bench_test_set,
			   cmd_bench_interval, cmd_bench_timeout,
			   cmd_bench_repeats, cmd_bench_start_delay);
		break;
	case CPU_STATE_NONE:
		if (event & CPU_EVENT_START)
			bench_enter_state(cpu, CPU_STATE_DIDT);
		break;
	case CPU_STATE_DIDT:
		if (event & CPU_EVENT_TIME)
			__bench_exec(cpu);
		if (event & CPU_EVENT_STOP) {
			bench_stop();
			bench_enter_state(cpu, CPU_STATE_NONE);
		}
		break;
	default:
		break;
	}
}

static void __bench_sync(unsigned long sync_state,
			 uint64_t this_cpu_mask, bool entry_or_exit,
			 uint32_t flags, bool wait, tick_t timeout)
{
	uint64_t all_cpu_mask;

	if (sync_state == CPU_EXEC_OPENED) {
		all_cpu_mask = cpu_exec_good;
		cpu_exec_sync |= this_cpu_mask;
	} else {
		all_cpu_mask = 0;
		cpu_exec_sync &= ~this_cpu_mask;
	}

	while (cpu_exec_state != sync_state) {
		spin_unlock(&cpu_exec_lock);
		if (__bench_sync_wait(wait,timeout)) {
			spin_lock(&cpu_exec_lock);
			if (sync_state == CPU_EXEC_OPENED)
				cpu_exec_good &= cpu_exec_sync;
			else
				cpu_exec_good &= ~cpu_exec_sync;
			spin_unlock(&cpu_exec_lock);
		}
		cpu_relax();
		spin_lock(&cpu_exec_lock);
		if (all_cpu_mask == cpu_exec_sync &&
		    cpu_exec_state != sync_state) {
			cpu_exec_state = sync_state;
			/* SYNC+RAND mode solution 2:
			 * The last CPU will reset the global test id
			 * here.
			 */
			if (sync_state == CPU_EXEC_OPENED &&
			    entry_or_exit)
				cpu_exec_test_id = -1;
		}
	}
}

struct cpu_exec_test *bench_test_find(const char *name)
{
	int nr_tests = ((uintptr_t)__testfn_end -
			(uintptr_t)__testfn_start) /
		       sizeof (struct cpu_exec_test);
	struct cpu_exec_test *start = __testfn_start;
	int i;

	for (i = 0; i < nr_tests; i++) {
		if (strcmp((start + i)->name, name) == 0)
			return start + i;
	}
	return NULL;
}

uint64_t bench_get_cpu_mask(void)
{
	return cpu_didt_cpu_mask;
}

int bench_didt(uint64_t init_cpu_mask, struct cpu_exec_test *fn,
	       tick_t interval, tick_t period, int repeats,
	       tick_t start_delay)
{
	cpu_t cpu = smp_processor_id();
	bool locked = false;
	int nr_cpus;

	spin_lock(&cpu_exec_lock);
	cpu_didt_refcnt++;
	if (cpu_didt_refcnt == 1) {
		cpu_didt_cpu_mask = init_cpu_mask;
		nr_cpus = hweight64(init_cpu_mask);
		locked = true;
	}
	if (!locked) {
		while (cpu_exec_stage != CPU_EXEC_BUSY) {
			spin_unlock(&cpu_exec_lock);
			cpu_relax();
			spin_lock(&cpu_exec_lock);
		}
	}
	if (locked) {
		int cpus = hweight64(init_cpu_mask);
		size_t size = ALIGN_UP(fn->alloc_size, fn->alloc_align);

		cpu_didt_pages = ALIGN_UP(size * cpus, PAGE_SIZE) /
				 PAGE_SIZE;
		if (cpu_didt_pages) {
			cpu_didt_alloc = page_alloc_pages(cpu_didt_pages);
			memory_set((caddr_t)cpu_didt_alloc, 0,
				   cpu_didt_pages * PAGE_SIZE);
		}
		do_printf("alloc: cpuexec: %016llx-%016llx(%d-%d)\n",
			  (uint64_t)cpu_didt_alloc, size,
			  cpus, cpu_didt_pages);
		while (cpu_didt_refcnt != nr_cpus) {
			spin_unlock(&cpu_exec_lock);
			cpu_relax();
			spin_lock(&cpu_exec_lock);
		}
		cpu_exec_stage = CPU_EXEC_BUSY;
		locked = false;
	}
	spin_unlock(&cpu_exec_lock);

	cpu_ctxs[cpu].async_wait_interval = interval;
	cpu_ctxs[cpu].async_exec_period = period;
	cpu_ctxs[cpu].async_start_delay = start_delay;
	cpu_ctxs[cpu].didt_entry = fn;
	cpu_ctxs[cpu].didt_repeats = repeats;
	cpu_ctxs[cpu].didt_result = 0;
	cpu_ctxs[cpu].async_state = CPU_STATE_NONE;
	bench_raise_event(CPU_EVENT_START);

	do {
		bh_sync();
	} while (cpu_ctxs[cpu].async_state != CPU_STATE_NONE);
	cpu_ctxs[cpu].async_state = CPU_STATE_HALT;
	return cpu_ctxs[cpu].didt_result;
}

static int err_bench(const char *hint)
{
	printf("--ERROR: %s\n", hint);
	(void)cmd_help("bench");
	return -EINVAL;
}

#if 0
static void cmd_bench_run_complete(uint64_t *results)
{
	int id;

	if (!results)
		return;

	printf("bench completed.\n");
	for (id = 0; id < MAX_CPU_NUM; id++) {
		if (cmd_bench_cpu_mask & C(id)) {
			printf("%d result: %s(%lld)\n", id,
			       results[id] == 0 ? "success" : "failure",
			       results[id]);
		}
	}
}
#endif

void bench_init(void)
{
	cpu_t cpu = smp_processor_id();

	cpu_ctxs[cpu].bh = bh_register_handler(bench_bh_handler);
	bench_timer_init(cpu);
}

static uint64_t cmd_get_cpu_mask_field(char *str, int len)
{
	uint64_t cpu_mask = CMD_INVALID_CPU_MASKS;
	int x;

	if (isdigit(str[0])) {
		if (len < 1)
			return CMD_INVALID_CPU_MASKS;
		x = strtoul(str, 0, 0);
		if (x >= MAX_CPU_NUM)
			return CMD_INVALID_CPU_MASKS;
		cpu_mask = CPU_TO_MASK(x);
	} else if (str[0] == 'c') {
		if (len < 2 || !isdigit(str[1]))
			return CMD_INVALID_CPU_MASKS;
		x = strtoul((str + 1), 0, 0);
		if (x >= MAX_CPU_CLUSTERS)
			return CMD_INVALID_CPU_MASKS;
		cpu_mask = CLUSTER_TO_CPU_MASK(x);
	} else if (str[0] == 'r') {
		if (len < 2 || !isdigit(str[1]))
			return CMD_INVALID_CPU_MASKS;
		x = strtoul((str + 1), 0, 0);
		if (x >= MAX_CPU_RAILS)
			return CMD_INVALID_CPU_MASKS;
		cpu_mask = RAIL_TO_CPU_MASK(x);
	} else if (strncmp(str, "all", 3) == 0) {
		cpu_mask = CPU_ALL;
	} else {
		return CMD_INVALID_CPU_MASKS;
	}
	return cpu_mask;
}

/* Decdoe a CPU mask string.
 *
 * Parameters:
 *   str: CPU mask string, formatted as XX|cXX|rXX.
 *        Where:
 *          XX indicates a CPU;
 *          cXX indicates a CPU cluster and
 *          rXX indicates a CPU rail.
          For example, "0|c1|r2".
 */
uint64_t cmd_get_cpu_mask(char *str)
{
	int i = -1;
	int j = 0;
	uint64_t cpu_mask = CMD_INVALID_CPU_MASKS;
	uint64_t mask;
	char sub_str[3];

	memset(sub_str, 0, sizeof(sub_str));
	do {
		i++;
		if (str[i] != '|' && str[i] != '\0') {
			switch (j) {
			case 0:
				if (!isdigit(str[i]) &&
				    str[i] != 'c' &&
				    str[i] != 'r' &&
				    str[i] != 'a')
					return CMD_INVALID_CPU_MASKS;
				break;
			case 1:
				if (sub_str[0] == 'a' && str[i] != 'l')
					return CMD_INVALID_CPU_MASKS;
				if (sub_str[0] != 'a' && !isdigit(str[i]))
					return CMD_INVALID_CPU_MASKS;
				break;
			case 2:
				if (isdigit(sub_str[0]))
					return CMD_INVALID_CPU_MASKS;
				if (sub_str[0] == 'a' && str[i] != 'l')
					return CMD_INVALID_CPU_MASKS;
				if (sub_str[0] != 'a' && !isdigit(str[i]))
					return CMD_INVALID_CPU_MASKS;
				break;
			default:
				return CMD_INVALID_CPU_MASKS;
			}
			sub_str[j++] = str[i];
		} else {
			mask = cmd_get_cpu_mask_field(sub_str, j);
			if (mask == CMD_INVALID_CPU_MASKS)
				return CMD_INVALID_CPU_MASKS;
			cpu_mask |= mask;
			j = 0;
			memset(sub_str, 0, sizeof(sub_str));
		}
	} while (str[i] != '\0');
	return cpu_mask;
}

static int cmd_bench_didt(uint64_t cpu_mask, struct cpu_exec_test *fn,
			  tick_t period, tick_t interval,
			  int repeats, tick_t timeout, bool sync)
{
	cpu_t cpu;

	spin_lock(&cpu_exec_lock);
	if (cpu_exec_stage != CPU_EXEC_IDLE) {
		printf("Bench busy...\n");
		spin_unlock(&cpu_exec_lock);
		return -EBUSY;
	}
	cpu_exec_stage = CPU_EXEC_PEND;
	spin_unlock(&cpu_exec_lock);

	cmd_bench_cpu_mask = (cpu_mask & CPU_ALL);
	cmd_bench_test_set = fn;
	cmd_bench_interval = interval;
	cmd_bench_timeout = period;
	cmd_bench_repeats = repeats;
	cmd_bench_start_delay = BENCH_START_DELAY_MS;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (C(cpu) & cpu_mask)
			bh_resume_smp(cpu_ctxs[cpu].bh, cpu);
	}

	if (sync) {
		spin_lock(&cpu_exec_lock);
		while (1) {
			spin_unlock(&cpu_exec_lock);
			bh_sync();
			spin_lock(&cpu_exec_lock);
			if (cpu_exec_stage == CPU_EXEC_IDLE)
				break;
		}
		spin_unlock(&cpu_exec_lock);
	}
	return 0;
}

int bench_simple(uint64_t cpu_mask, struct cpu_exec_test *fn, bool sync)
{
	cpu_t cpu;

	spin_lock(&cpu_exec_lock);
	if (cpu_exec_stage != CPU_EXEC_IDLE) {
		printf("Bench busy...\n");
		spin_unlock(&cpu_exec_lock);
		return -EBUSY;
	}
	cpu_exec_stage = CPU_EXEC_PEND;
	spin_unlock(&cpu_exec_lock);

	cmd_bench_cpu_mask = (cpu_mask & CPU_ALL);
	cmd_bench_test_set = fn;
	cmd_bench_interval = 0;
	cmd_bench_timeout = 0;
	cmd_bench_repeats = 1;
	cmd_bench_start_delay = 0;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (C(cpu) & cpu_mask)
			bh_resume_smp(cpu_ctxs[cpu].bh, cpu);
	}

	if (sync) {
		spin_lock(&cpu_exec_lock);
		while (1) {
			spin_unlock(&cpu_exec_lock);
			bh_sync();
			spin_lock(&cpu_exec_lock);
			if (cpu_exec_stage == CPU_EXEC_IDLE)
				break;
		}
		spin_unlock(&cpu_exec_lock);
	}
	return 0;
}

static int cmd_bench(int argc, char **argv)
{
	int nr_tests = ((uintptr_t)__testfn_end -
			(uintptr_t)__testfn_start) /
		       sizeof (struct cpu_exec_test);
	struct cpu_exec_test *start = __testfn_start, *fn;
	uint64_t cpu_mask;
	int i, ret;
	tick_t timeout, period, interval;
	int repeats;

	if (argc < 2)
		return err_bench("Missing sub-command.\n");
	if (strcmp(argv[1], "show") == 0) {
		for (i = 0; i < nr_tests; i++) {
			if ((start + i)->flags & CPU_EXEC_META)
				printf("%20s: %4s 0x%016llx\n",
				       (start + i)->name,
				       (start + i)->flags & CPU_EXEC_SYNC ? "sync" : "",
				       (uint64_t)((start + i)->func));
		}
	} else if (strcmp(argv[1], "help") == 0) {
		(void)cmd_help("bench");
	} else if (strcmp(argv[1], "sync") == 0 ||
		   strcmp(argv[1], "async") == 0) {
		if (argc < 7)
			return err_bench("Missing parameters.\n");
		cpu_mask = cmd_get_cpu_mask(argv[2]);
		if (cpu_mask == CMD_INVALID_CPU_MASKS)
			return err_bench("Invalid CPU mask.\n");
		fn = bench_test_find(argv[3]);
		if (!fn || !(fn->flags & CPU_EXEC_META))
			return err_bench("Invalid test function.\n");
		period = strtoul(argv[4], 0, 0);
		interval = strtoul(argv[5], 0, 0);
		repeats = strtoul(argv[6], 0, 0);
		if (period == CPU_WAIT_INFINITE)
			return err_bench("Invalid period.\n");
		if (interval == CPU_WAIT_INFINITE)
			return err_bench("Invalid interval.\n");
		if (repeats < 0)
			return err_bench("Invalid repeat times.\n");
		if (argc > 7) {
			timeout = strtoul(argv[7], 0, 0);
		} else
			timeout = CPU_WAIT_INFINITE;
		/* Try to avoid dIdT mode */
		if (period >= interval) {
			printf("Switch to the non-didt mode.\n");
			interval = 0;
		}
		ret = cmd_bench_didt(cpu_mask, fn, period, interval,
				     repeats, timeout,
				     !!(strcmp(argv[1], "sync") == 0));
		if (ret) {
			printf("--ERROR: failed to run %s\n", argv[3]);
			return -EFAULT;
		}
	} else
		return err_bench("Invalid sub-command.\n");
	return 0;
}

DEFINE_COMMAND(bench, cmd_bench, "Run pre-registered patterns on CPUs",
	" show\n"
	"    -List the pre-registered patterns\n"
	" async/sync N|cN|rN|all pattern period interval repeats [timeout]\n"
	"    -(A)synchronously run pre-registered pattern\n"
	"     Run pattern specified \"repeats\" until \"timeout\".\n"
	"     Each run starts simultaneously on specified CPUs.\n"
	"     Each run repeatedly executing the pattern during \"period\".\n"
	"     Next run starts \"interval\" later than the previous run.\n"
	"     CPU specifications (N|cN|rN|all):\n"
	"        N: CPU N, N starts from 0.\n"
	"       cN: Cluster N, N starts from 0.\n"
	"       rN: Rail N, N starts from 0.\n"
	"      all: All CPUs.\n"
	"     dIdT mode: When interval > period, after running pattern for a\n"
	"                while (period), CPU will enter idle state for a\n"
	"                while (interval - period).\n"
	"     norm mode: When interval <= period, CPU won't enter idle state,\n"
	"                When interval = 0, ensured to run at least once.\n"
);
