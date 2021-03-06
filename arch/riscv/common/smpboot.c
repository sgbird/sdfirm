#include <target/smp.h>
#include <target/arch.h>
#include <target/panic.h>
#include <target/barrier.h>
#include <target/irq.h>
#include <target/atomic.h>
#include <target/sbi.h>

enum ipi_message_type {
	IPI_RESCHEDULE,
	IPI_CALL_FUNC,
	IPI_CPU_OFF,
	IPI_MAX
};

/* A collection of single bit ipi messages.  */
static struct {
	unsigned long bits;
} ipi_data[NR_CPUS] __cache_aligned;

void *__cpu_up_entry[NR_CPUS];

void smp_hw_cpu_on(cpu_t cpu, caddr_t ep)
{
	smp_mb();
	WRITE_ONCE(__cpu_up_entry[cpu], ep);
}

void smp_hw_cpu_boot(void)
{
	smp_boot_cpu = smp_processor_id();
}

static void send_ipi_single(int cpu, enum ipi_message_type op)
{
	cpu_mask_t mask;
	int hartid = smp_hw_cpu_hart(cpu);

	memset(&mask, 0, sizeof (mask));
	smp_mb__before_atomic();
	set_bit(op, (bits_t *)&ipi_data[cpu].bits);
	cpumask_set_cpu(hartid, &mask);
	smp_mb__after_atomic();

	sbi_send_ipi(cpumask_bits(&mask));
}

void smp_hw_handle_ipi(void)
{
	unsigned long *pending_ipis = &ipi_data[smp_processor_id()].bits;

	irqc_clear_irq(IRQ_SOFT);

	while (true) {
		unsigned long ops;

		/* Order bit clearing and data access. */
		mb();

		ops = xchg(pending_ipis, 0);
		if (ops == 0)
			return;

		if (ops & (1 << IPI_RESCHEDULE))
			printf("%d: IPI_RESCHEDULE rcvd\n", smp_processor_id());
		if (ops & (1 << IPI_CALL_FUNC))
			printf("%d: IPI_CALL_FUNC rcvd\n", smp_processor_id());
		if (ops & (1 << IPI_CPU_OFF))
			printf("%d: IPI_CPU_OFF rcvd\n", smp_processor_id());

		BUG_ON((ops >> IPI_MAX) != 0);

		/* Order data access and bit testing. */
		mb();
	}
}

void smp_hw_cpu_off(cpu_t cpu)
{
	send_ipi_single(cpu, IPI_CPU_OFF);
}

void smp_hw_ctrl_init(void)
{
	irq_register_vector(IRQ_SOFT, smp_hw_handle_ipi);
	irqc_enable_irq(IRQ_SOFT);
}
