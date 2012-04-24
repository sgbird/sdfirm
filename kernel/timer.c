#include <target/timer.h>
#include <target/bh.h>
#include <target/gpt.h>
#include <target/jiffies.h>

/* use the lowest bit to allow the maximum timeout values */
#define TIMER_FLAG_SHOT			((timeout_t)1)
#define TIMER_IDLE			((timeout_t)0)
#define TIMER_SHOT(timeout)		(timeout & TIMER_FLAG_SHOT)
/* thus timeout values are always an even value */
#define TIMER_TIME(timeout)		((timeout_t)((timeout_t)(timeout) >> 1))
#define TIMER_MAKE(shot, time)		((shot) | ((timeout_t)(time) << 1))

struct timer_entry {
	timeout_t timeout;
};

timer_desc_t *timer_descs[NR_TIMERS];
struct timer_entry timer_entries[NR_TIMERS];
DECLARE_BITMAP(timer_regs, NR_TIMERS);
bh_t timer_bh = INVALID_BH;
tid_t timer_running_tid = INVALID_TID;
tid_t timer_orders[NR_TIMERS];

void __timer_del(tid_t tid)
{
	tid_t i;

	for (i = 0; i < NR_TIMERS; i++) {
		if (timer_orders[i] == INVALID_TID)
			return;
		if (timer_orders[i] == tid)
			break;
	}
	for (; i < NR_TIMERS-1; i++) {
		if (timer_orders[i] != INVALID_TID) {
			tid_t swp = timer_orders[i+1];
			timer_orders[i+1] = timer_orders[i];
			timer_orders[i] = swp;
		}
	}
	timer_orders[i] = INVALID_TID;
}

void __timer_add(tid_t tid)
{
	tid_t i, t;

	for (i = 0; i < NR_TIMERS; i++) {
		t = timer_orders[NR_TIMERS-i-1];
		/* should be already deleted */
		BUG_ON(t == tid);
		if (t != INVALID_TID) {
			/* should not exceed NR_TIMERS */
			BUG_ON(i == 0);
			if (TIMER_TIME(timer_entries[t].timeout) >
			    TIMER_TIME(timer_entries[tid].timeout)) {
				timer_orders[NR_TIMERS-i] = t;
				timer_orders[NR_TIMERS-i-1] = INVALID_TID;
			} else {
				timer_orders[NR_TIMERS-i] = tid;
				return;
			}
		}
	}
	BUG_ON(timer_orders[0] != INVALID_TID);
	timer_orders[0] = tid;
}

void __timer_reset_timeout(tid_t tid, timeout_t tout_ms)
{
	BUG_ON(tout_ms > MAX_TIMEOUT);
	timer_entries[tid].timeout = TIMER_MAKE(TIMER_FLAG_SHOT, tout_ms);
	__timer_del(tid);
	__timer_add(tid);
}

void __timer_run(tid_t tid)
{
	timer_desc_t *timer = timer_descs[tid];

	idle_debug(IDLE_DEBUG_TID, tid);
	BUG_ON(!timer || !timer->handler);
	timer_running_tid = tid;
	__timer_del(tid);
	timer->handler();
	timer_running_tid = INVALID_TID;
}

/* Register a timer delay to execute the state machine
 *
 * IN bh: the state machine will be executed when the timer is due
 * IN timeout: the period that will be delayed
 * OUT tid: return NR_TIMERS on error
 */
tid_t timer_register(timer_desc_t *timer)
{
	tid_t tid = find_first_clear_bit(timer_regs, NR_TIMERS);
	BUG_ON(tid == NR_TIMERS);
	timer_descs[tid] = timer;
	timer_entries[tid].timeout = TIMER_IDLE;
	set_bit(tid, timer_regs);
	timer_running_tid = tid;
	return tid;
}

void timer_unregister(tid_t tid)
{
	BUG_ON(tid != timer_running_tid);
	__timer_del(tid);
	timer_descs[tid] = NULL;
	clear_bit(tid, timer_regs);
	timer_running_tid = INVALID_TID;
}

#ifdef CONFIG_TICK
tick_t timer_last_tick = 0;

void timer_bh_timeout(void)
{
	tid_t tid, i;
	timeout_t span;
	
	span = (timeout_t)(jiffies - timer_last_tick);
	timer_last_tick = jiffies;

	for (i = 0; i < NR_TIMERS; i++) {
		tid = timer_orders[i];
		if (tid == INVALID_TID)
			break;
		if (TIMER_SHOT(timer_entries[tid].timeout)) {
			timeout_t tid_tout = TIMER_TIME(timer_entries[tid].timeout);
			if (span >= tid_tout) {
				timer_entries[tid].timeout = TIMER_IDLE;
				__timer_run(tid);
			} else {
				timer_entries[tid].timeout = TIMER_MAKE(TIMER_FLAG_SHOT, tid_tout-span);
			}
		}
	}
}

#define timer_start()		bh_resume(timer_bh)
#define timer_restart()		timer_start()
#else
timeout_t timer_timeout = TIMER_MAKE(TIMER_FLAG_SHOT, 0);
timeout_t timer_unshot_timeout = 0;

void timer_recalc_timeout(void)
{
	tid_t tid, i;

	timer_timeout = TIMER_IDLE;
	for (i = 0; i < NR_TIMERS; i++) {
		tid = timer_orders[i];
		if (tid == INVALID_TID)
			break;
		if (TIMER_SHOT(timer_entries[tid].timeout)) {
			if (!TIMER_SHOT(timer_timeout)) {
				timer_timeout = timer_entries[tid].timeout;
			} else {
				if (TIMER_TIME(timer_entries[tid].timeout) < TIMER_TIME(timer_timeout))
					timer_timeout = timer_entries[tid].timeout;
			}
		}
	}
}

void timer_bh_timeout(void)
{
	tid_t tid, i;
	timeout_t tout;

	BUG_ON(TIMER_SHOT(timer_timeout) != TIMER_FLAG_SHOT);
	tout = TIMER_TIME(timer_timeout);
	for (i = 0; i < NR_TIMERS; i++) {
		tid = timer_orders[i];
		if (tid == INVALID_TID)
			break;
		if (timer_entries[tid].timeout == TIMER_IDLE) {
			__timer_run(tid);
		}
	}

	timer_recalc_timeout();
}

void timer_shot_timeout(timeout_t to_shot)
{
	if (to_shot < GPT_MAX_TIMEOUT) {
		timer_unshot_timeout = 0;
		if (to_shot == 0)
			to_shot = 1;
		gpt_oneshot_timeout(to_shot);
	} else {
		timer_unshot_timeout = to_shot - GPT_MAX_TIMEOUT;
		/* XXX: GPT_MAX_TIMEOUT Validation
		 *
		 * if GPT_MAX_TIMEOUT > max of timeout_t, following code
		 * could not get reached, so its safe to force it here
		 */
		gpt_oneshot_timeout((timeout_t)GPT_MAX_TIMEOUT);
	}
}

void timer_execute_shot(void)
{
	if (TIMER_SHOT(timer_timeout)) {
		timer_shot_timeout(TIMER_TIME(timer_timeout));
	}
}

void timer_start(void)
{
	gpt_hw_ctrl_init();
	bh_resume(timer_bh);
}

void timer_restart(void)
{
	timer_execute_shot();
}

void timer_irq_timeout(void)
{
	tid_t tid, i;
	timeout_t tout;
	timer_desc_t *timer;
	boolean recalc_irq = false, recalc_bh = false;

	BUG_ON(TIMER_SHOT(timer_timeout) != TIMER_FLAG_SHOT);
	tout = TIMER_TIME(timer_timeout);
	for (i = 0; i < NR_TIMERS; i++) {
		tid = timer_orders[i];
		if (tid == INVALID_TID)
			break;
		timer = timer_descs[tid];
		if (TIMER_SHOT(timer_entries[tid].timeout)) {
			timeout_t tid_tout = TIMER_TIME(timer_entries[tid].timeout);
			if (tout >= tid_tout) {
				timer_entries[tid].timeout = TIMER_IDLE;
				if (timer->flags & TIMER_IRQ) {
					__timer_run(tid);
					if (TIMER_SHOT(timer_entries[tid].timeout))
						recalc_irq = true;
				} else {
					recalc_bh = true;
					bh_resume(timer_bh);
				}
			} else {
				tid_tout -= tout;
				timer_entries[tid].timeout = TIMER_MAKE(TIMER_FLAG_SHOT, tid_tout);
			}
		}
	}

	if (recalc_irq || !recalc_bh)
		timer_recalc_timeout();
}

void tick_handler(void)
{
	if (timer_unshot_timeout)
		timer_shot_timeout(timer_unshot_timeout);
	else
		timer_irq_timeout();
}
#endif

void timer_schedule_shot(tid_t tid, timeout_t tout_ms)
{
	BUG_ON(tid != timer_running_tid);
	__timer_reset_timeout(tid, tout_ms);
}

void timer_bh_handler(uint8_t event)
{
	BUG_ON(event != BH_WAKEUP);
	timer_bh_timeout();
	timer_restart();
}

#ifdef CONFIG_TIMER_TEST
void timer_test_handler(void)
{
}

timer_desc_t timer_1 = {
	TIMER_BH,
	timer_test_handler,
};
tid_t tid_1;

timer_desc_t timer_2 = {
	TIMER_IRQ,
	timer_test_handler,
};
tid_t tid_2;

timer_desc_t timer_3 = {
	TIMER_IRQ,
	timer_test_handler,
};
tid_t tid_3;

void timer_test(void)
{
	tid_1 = timer_register(&timer_1);
	timer_schedule_shot(tid_1, 20);
	BUG_ON(timer_orders[0] != tid_1);
	BUG_ON(timer_orders[1] != INVALID_TID);
	BUG_ON(timer_orders[2] != INVALID_TID);

	tid_2 = timer_register(&timer_2);
	timer_schedule_shot(tid_2, 30);
	BUG_ON(timer_orders[0] != tid_1);
	BUG_ON(timer_orders[1] != tid_2);
	BUG_ON(timer_orders[2] != INVALID_TID);
	timer_schedule_shot(tid_2, 10);
	BUG_ON(timer_orders[0] != tid_2);
	BUG_ON(timer_orders[1] != tid_1);
	BUG_ON(timer_orders[2] != INVALID_TID);
	timer_schedule_shot(tid_2, 30);
	BUG_ON(timer_orders[0] != tid_1);
	BUG_ON(timer_orders[1] != tid_2);
	BUG_ON(timer_orders[2] != INVALID_TID);

	tid_3 = timer_register(&timer_3);
	timer_schedule_shot(tid_3, 10);
	BUG_ON(timer_orders[0] != tid_3);
	BUG_ON(timer_orders[1] != tid_1);
	BUG_ON(timer_orders[2] != tid_2);
	timer_schedule_shot(tid_3, 25);
	BUG_ON(timer_orders[0] != tid_1);
	BUG_ON(timer_orders[1] != tid_3);
	BUG_ON(timer_orders[2] != tid_2);
	timer_schedule_shot(tid_3, 40);
	BUG_ON(timer_orders[0] != tid_1);
	BUG_ON(timer_orders[1] != tid_2);
	BUG_ON(timer_orders[2] != tid_3);

	timer_running_tid = tid_1;
	timer_unregister(tid_1);
	BUG_ON(timer_orders[0] != tid_2);
	BUG_ON(timer_orders[1] != tid_3);
	BUG_ON(timer_orders[2] != INVALID_TID);

	timer_running_tid = tid_2;
	timer_unregister(tid_2);
	BUG_ON(timer_orders[0] != tid_3);
	BUG_ON(timer_orders[1] != INVALID_TID);
	BUG_ON(timer_orders[2] != INVALID_TID);

	timer_running_tid = tid_3;
	timer_unregister(tid_3);
	BUG_ON(find_first_clear_bit(timer_regs, NR_TIMERS) != 0);
	BUG_ON(timer_orders[0] != INVALID_TID);
	BUG_ON(timer_orders[1] != INVALID_TID);
	BUG_ON(timer_orders[2] != INVALID_TID);
}
#else
#define timer_test()
#endif

void timer_init(void)
{
	tid_t tid;
	for (tid = 0; tid < NR_TIMERS; tid++)
		timer_orders[tid] = INVALID_TID;
	timer_bh = bh_register_handler(timer_bh_handler);
	timer_start();
	timer_test();
}
