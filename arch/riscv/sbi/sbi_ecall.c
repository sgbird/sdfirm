/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <target/sbi.h>

#define SBI_ECALL_VERSION_MAJOR 0
#define SBI_ECALL_VERSION_MINOR 1

bool sbi_trap_log_on;

bool sbi_trap_log_enabled(void)
{
	return sbi_trap_log_on;
}

uint16_t sbi_ecall_version_major(void)
{
	return SBI_ECALL_VERSION_MAJOR;
}

uint16_t sbi_ecall_version_minor(void)
{
	return SBI_ECALL_VERSION_MINOR;
}

int sbi_ecall_handler(uint32_t hartid, ulong mcause, struct pt_regs *regs,
		      struct sbi_scratch *scratch)
{
	int ret = -ENOTSUP;
	struct unpriv_trap uptrap;
	__unused struct sbi_tlb_info tlb_info;
	uint32_t source_hart = sbi_current_hartid();

	switch (regs->a7) {
	case SBI_ECALL_SET_TIMER:
		sbi_trap_log("%d: ECALL_SET_TIMER\n", source_hart);
#if __riscv_xlen == 32
		sbi_timer_event_start(scratch,
				      (((uint64_t)regs->a1 << 32) |
				        (uint64_t)regs->a0));
#else
		sbi_timer_event_start(scratch, (uint64_t)regs->a0);
#endif
		ret = 0;
		break;
	case SBI_ECALL_CONSOLE_PUTCHAR:
		sbi_putc(regs->a0);
		ret = 0;
		break;
	case SBI_ECALL_CONSOLE_GETCHAR:
		regs->a0 = sbi_getc();
		ret = 0;
		break;
	case SBI_ECALL_CLEAR_IPI:
		sbi_trap_log("%d: ECALL_CLEAR_IPI\n", source_hart);
		sbi_ipi_clear_smode(scratch);
		ret = 0;
		break;
	case SBI_ECALL_SEND_IPI:
		sbi_trap_log("%d: ECALL_SEND_IPI\n", source_hart);
		ret = sbi_ipi_send_many(scratch, &uptrap, (ulong *)regs->a0,
					SBI_IPI_EVENT_SOFT, NULL);
		break;
	case SBI_ECALL_REMOTE_FENCE_I:
		sbi_trap_log("%d: ECALL_REMOTE_FENCE_I\n", source_hart);
		tlb_info.start = 0;
		tlb_info.size  = 0;
		tlb_info.type  = SBI_ITLB_FLUSH;
		tlb_info.shart_mask = 1UL << source_hart;
		ret = sbi_ipi_send_many(scratch, &uptrap, (ulong *)regs->a0,
					SBI_IPI_EVENT_FENCE_I, &tlb_info);
		break;
	case SBI_ECALL_REMOTE_SFENCE_VMA:
		sbi_trap_log("%d: ECALL_REMOTE_SFENCE_VMA\n", source_hart);
		tlb_info.start = (unsigned long)regs->a1;
		tlb_info.size  = (unsigned long)regs->a2;
		tlb_info.type  = SBI_TLB_FLUSH_VMA;
		tlb_info.shart_mask = 1UL << source_hart;

		ret = sbi_ipi_send_many(scratch, &uptrap, (ulong *)regs->a0,
					SBI_IPI_EVENT_SFENCE_VMA, &tlb_info);
		break;
	case SBI_ECALL_REMOTE_SFENCE_VMA_ASID:
		sbi_trap_log("%d: ECALL_REMOTE_SFENCE_VMA_ASID\n", source_hart);
		tlb_info.start = (unsigned long)regs->a1;
		tlb_info.size  = (unsigned long)regs->a2;
		tlb_info.asid  = (unsigned long)regs->a3;
		tlb_info.type  = SBI_TLB_FLUSH_VMA_ASID;
		tlb_info.shart_mask = 1UL << source_hart;

		ret = sbi_ipi_send_many(scratch, &uptrap, (ulong *)regs->a0,
					SBI_IPI_EVENT_SFENCE_VMA_ASID,
					&tlb_info);
		break;
	case SBI_ECALL_SHUTDOWN:
		sbi_trap_log("%d: ECALL_SHUTDOWN\n", source_hart);
		sbi_system_shutdown(scratch, 0);
		ret = 0;
		break;
	case SBI_ECALL_FINISH:
		sbi_trap_log("%d: ECALL_FINISH\n", source_hart);
		sbi_system_finish(scratch, regs->a0);
		ret = 0;
		break;
	case SBI_ECALL_ENABLE_LOG:
		sbi_trap_log_on = true;
		ret = 0;
		break;
	case SBI_ECALL_DISABLE_LOG:
		sbi_trap_log_on = false;
		ret = 0;
		break;
	case SBI_ECALL_GET_CLK_FREQ:
		sbi_trap_log("%d: ECALL_GET_CLK_FREQ\n", source_hart);
		regs->a0 = sbi_clock_get_freq(regs->a0);
		ret = 0;
		break;
	case SBI_ECALL_SET_CLK_FREQ:
		sbi_trap_log("%d: ECALL_SET_CLK_FREQ\n", source_hart);
		sbi_clock_set_freq(regs->a0, regs->a1);
		ret = 0;
		break;
	case SBI_ECALL_ENABLE_CLK:
		sbi_trap_log("%d: ECALL_ENABLE_CLK\n", source_hart);
		sbi_clock_enable(regs->a0);
		ret = 0;
		break;
	case SBI_ECALL_DISABLE_CLK:
		sbi_trap_log("%d: ECALL_DISABLE_CLK\n", source_hart);
		sbi_clock_disable(regs->a0);
		ret = 0;
		break;
	case SBI_ECALL_CONFIG_PIN_MUX:
		sbi_trap_log("%d: ECALL_CONFIG_PIN_MUX\n", source_hart);
		sbi_pin_config_mux(regs->a0, regs->a1);
		ret = 0;
		break;
	case SBI_ECALL_CONFIG_PIN_PAD:
		sbi_trap_log("%d: ECALL_CONFIG_PIN_PAD\n", source_hart);
		sbi_pin_config_pad(regs->a0, regs->a1);
		ret = 0;
		break;
	default:
		regs->a0 = -ENOTSUP;
		ret	 = 0;
		break;
	};

	if (!ret) {
		regs->epc += 4;
	} else if (ret == ETRAP) {
		ret = 0;
		sbi_trap_redirect(regs, scratch, regs->epc,
				  uptrap.cause, uptrap.tval);
	}

	return ret;
}
