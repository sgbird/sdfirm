#include <target/mmc.h>
#include <target/delay.h>
#include <target/jiffies.h>

struct sdhci_host sdhci_hosts[NR_MMC_SLOTS];

struct sdhci_host *mmc2sdhci(void)
{
	if (mmc_sid >= NR_MMC_SLOTS)
		return NULL;
	return sdhci_hosts + mmc_sid;
}

static void sdhci_reset(uint8_t mask)
{
	uint8_t tmp;

	/* Wait max 100 ms */
	__raw_writeb(mask, SDHC_SOFTWARE_RESET(mmc_sid));
	if (!__raw_read_poll(b, SDHC_SOFTWARE_RESET(mmc_sid), tmp,
			     !(tmp & mask), 0, 100)) {
		printf("%s: Reset 0x%x never completed.\n",
		       __func__, (int)mask);
		return;
	}
}

static void sdhci_transfer_pio(uint32_t *block)
{
	uint8_t type = mmc_get_block_data();
	int i;
	uint32_t *offs;

	for (i = 0; i < mmc_slot_ctrl.block_len; i += 4) {
		offs = block + i;
		if (type == MMC_SLOT_BLOCK_READ)
			*offs = __raw_readl(SDHC_BUFFER_DATA_PORT(mmc_sid));
		else
			__raw_writel(*offs, SDHC_BUFFER_DATA_PORT(mmc_sid));
	}
}

static void sdhci_transfer_data(void)
{
	unsigned int stat, rdy, mask, timeout, block = 0;
	bool transfer_done = false;
	uint32_t *buf = (uint32_t *)mmc_slot_ctrl.block_data;
#ifdef CONFIG_SDHC_SDMA
	uint32_t start_addr = 0;
	unsigned char ctrl;

	sdhc_config_dma(mmc_sid, SDHC_SDMA);
#endif

	timeout = 1000000;
	rdy = SDHC_BUFFER_READ_READY | SDHC_BUFFER_WRITE_READY;
	mask = SDHC_BUFFER_READ_ENABLE | SDHC_BUFFER_WRITE_ENABLE;
	do {
		stat = sdhc_irq_status(mmc_sid);
		if (stat & SDHC_ERROR_INTERRUPT) {
			printf("%s: Error detected in status(0x%X)!\n",
			       __func__, stat);
			mmc_cmd_failure(MMC_ERR_CARD_LOOSE_BUS);
			return;
		}
		if (!transfer_done && (stat & rdy)) {
			if (!sdhc_state_present(mmc_sid, mask))
				continue;
			sdhc_clear_irq(mmc_sid, rdy);
			sdhci_transfer_pio(buf);
			block += mmc_slot_ctrl.block_len;
			if (++block >= mmc_slot_ctrl.block_cnt) {
				/* Keep looping until the SDHC_INT_DATA_END is
				 * cleared, even if we finished sending all the
				 * blocks.
				 */
				transfer_done = true;
				continue;
			}
		}
#ifdef CONFIG_SDHC_SDMA
		if (!transfer_done && (stat & SDHC_DMA_INTERRUPT)) {
			sdhc_clear_irq(mmc_sid, SDHC_DMA_INTERRUPT);
			start_addr &= ~(SDHC_DEFAULT_BOUNDARY_SIZE - 1);
			start_addr += SDHC_DEFAULT_BOUNDARY_SIZE;
			__raw_writel(start_addr,
				     SDHC_SDMA_SYSTEM_ADDRESS(mmc_sid));
		}
#endif
		if (timeout-- > 0)
			udelay(10);
		else {
			printf("%s: Transfer data timeout\n", __func__);
			mmc_cmd_failure(MMC_ERR_TIMEOUT);
			return;
		}
	} while (!(stat & SDHC_TRANSFER_COMPLETE));
}

/* No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value.
 */
static void sdhc_wait_transfer(void)
{
	while (sdhc_state_present(mmc_sid, SDHC_COMMAND_INHIBIT));
}

void sdhci_send_command(uint8_t cmd, uint32_t arg)
{
	uint8_t type = mmc_get_block_data();
	__unused size_t trans_bytes;
	uint32_t mask, flags, mode;
	uint8_t rsp = mmc_slot_ctrl.rsp;

	sdhc_wait_transfer();
	sdhci_start_transfer();

	mask = SDHC_COMMAND_COMPLETE;
	if (!(rsp & MMC_RSP_PRESENT))
		flags = SDHC_RESPONSE_TYPE_SELECT(SDHC_NO_RESPONSE);
	else if (rsp & MMC_RSP_136)
		flags = SDHC_RESPONSE_TYPE_SELECT(SDHC_RESPONSE_LENGTH_136);
	else if (rsp & MMC_RSP_BUSY) {
		flags = SDHC_RESPONSE_TYPE_SELECT(SDHC_RESPONSE_LENGTH_48_BUSY);
		if (type)
			mask |= SDHC_TRANSFER_COMPLETE;
	} else
		flags = SDHC_RESPONSE_TYPE_SELECT(SDHC_RESPONSE_LENGTH_48);

	if (rsp & MMC_RSP_CRC)
		flags |= SDHC_COMMAND_CRC_CHECK_ENABLE;
	if (rsp & MMC_RSP_OPCODE)
		flags |= SDHC_COMMAND_INDEX_CHECK_ENABLE;

	if (type)
		flags |= SDHC_DATA_PRESENT_SELECT;

	/* Set Transfer mode regarding to data flag */
	if (type) {
		__raw_writeb(0xE, SDHC_TIMEOUT_CONTROL(mmc_sid));
		mode = SDHC_BLOCK_COUNT_ENABLE;
		trans_bytes = mmc_slot_ctrl.block_cnt *
			      mmc_slot_ctrl.block_len;
		if (mmc_slot_ctrl.block_cnt > 1)
			mode |= SDHC_MULTI_SINGLE_BLOCK_SELECT;
		if (type == MMC_SLOT_BLOCK_READ)
			mode |= SDHC_DATA_TRANSFER_DIRECTION_SELECT;

#ifdef CONFIG_SDHC_SDMA
		/* TODO: Bounce buffer */
		start_addr = (uint32_t)mmc_slot_ctrl->block_data;
		__raw_writel(start_addr, SDHC_SDMA_SYSTEM_ADDRESS(mmc_sid));
		mode |= SDHC_DMA_ENABLE;
#endif
		__raw_writew(
			SDHC_SDMA_BUFFER_BOUNDARY(SDHC_DEFAULT_BOUNDARY_ARG) |
			SDHC_TRANSFER_BLOCK_SIZE(mmc_slot_ctrl.block_len),
			SDHC_BLOCK_SIZE(mmc_sid));
		__raw_writew(mmc_slot_ctrl.block_cnt,
			     SDHC_16BIT_BLOCK_COUNT(mmc_sid));
		__raw_writew(mode, SDHC_TRANSFER_MODE(mmc_sid));
	} else if (rsp & MMC_RSP_BUSY) {
		__raw_writeb(0xE, SDHC_TIMEOUT_CONTROL(mmc_sid));
	}

	__raw_writel(arg, SDHC_ARGUMENT(mmc_sid));
	__raw_writel(SDHC_CMD(cmd, flags), SDHC_COMMAND(mmc_sid));
}

static void sdhci_decode_reg(uint8_t *resp, uint8_t size, uint32_t reg)
{
	if (size > 0)
		resp[3] = HIBYTE(HIWORD(reg));
	if (size > 1)
		resp[2] = LOBYTE(HIWORD(reg));
	if (size > 2)
		resp[1] = HIBYTE(LOWORD(reg));
	if (size > 3)
		resp[0] = LOBYTE(LOWORD(reg));
}

void sdhci_recv_response(uint8_t *resp, uint8_t size)
{
	uint8_t rsp = mmc_slot_ctrl.rsp;
	int i;
	uint32_t reg;
	uint8_t len;

	if (rsp & MMC_RSP_136) {
		/* CRC is stripped so we need to do some shifting. */
		len = 0;
		for (i = 0; i < 4; i++) {
			len += 4;
			reg = __raw_readl(SDHC_RESPONSE(
				mmc_sid, (3-i)*4)) << 8;
			if (i != 3)
				reg |= __raw_readb(SDHC_RESPONSE(
					mmc_sid, (3-i)*4-1));
			sdhci_decode_reg(resp + i,
					 size >= len ? 4 : 0, reg);
		}
	} else {
		reg = __raw_readl(SDHC_RESPONSE(mmc_sid, 0));
		sdhci_decode_reg(resp, size, reg);
	}
	sdhci_stop_transfer();
}

void sdhci_tran_data(uint8_t *dat, uint32_t len, uint16_t cnt)
{
	mmc_dat_success();
}

bool sdhci_card_busy(void)
{
	return false;
}

static void sdhci_set_power(uint8_t power)
{
	if (power != (uint8_t)-1) {
		sdhc_power_off(mmc_sid);
		return;
	}

	switch (1 << power) {
	case MMC_OCR_170_195:
		sdhc_power_on_vdd1(mmc_sid, SDHC_SD_BUS_POWER_180);
		sdhc_power_on_vdd2(mmc_sid, SDHC_SD_BUS_POWER_180);
		break;
	case MMC_OCR_29_30:
	case MMC_OCR_30_31:
		sdhc_power_on_vdd1(mmc_sid, SDHC_SD_BUS_POWER_300);
		break;
	case MMC_OCR_32_33:
	case MMC_OCR_33_34:
		sdhc_power_on_vdd1(mmc_sid, SDHC_SD_BUS_POWER_330);
		break;
	default:
		sdhc_power_off(mmc_sid);
		break;
	}
}

void sdhc_start_clock(void)
{
	sdhc_set_clock(mmc_sid, SDHC_CLOCK_ENABLE);
}

void sdhc_stop_clock(void)
{
	sdhc_wait_transfer();
	sdhc_clear_clock(mmc_sid, SDHC_CLOCK_ENABLE);
}

static bool sdhc_clock_stabilised(void)
{
	tick_t timeout = SDHC_INTERNAL_CLOCK_STABLE_TOUT_MS;

	do {
		if (__raw_readw(SDHC_CLOCK_CONTROL(mmc_sid)) &
		    SDHC_INTERNAL_CLOCK_STABLE)
			break;
		if (timeout == 0) {
			printf("Internal clock not stabilised.\n");
			return false;
		}
		timeout--;
		mdelay(1);
	} while (1);
	return true;
}

#ifdef CONFIG_SDHC_SPEC_4_10
static void sdhc_enable_pll(void)
{
	sdhc_set_clock(mmc_sid, SDHC_PLL_ENABLE);
	if (!sdhc_clock_stabilised())
		return;
}
static void sdhc_disable_pll(void)
{
	sdhc_clear_clock(mmc_sid, SDHC_PLL_ENABLE);
}
#else
#define sdhc_enable_pll()	do { } while (0)
#define sdhc_disable_pll()	do { } while (0)
#endif

bool sdhci_set_clock(uint32_t clock)
{
	struct sdhci_host *host = mmc2sdhci();
	uint32_t div, clk = 0;

	sdhc_stop_clock();
	sdhc_disable_pll();

	/* 1. calculate a divisor */
	if (SDHC_SPEC(host) >= SDHC_SPECIFICATION_VERSION_300) {
		/* Host Controller supports Programmable Clock Mode? */
		if (host->clk_mul) {
			for (div = 1; div <= 1024; div++) {
				if ((host->max_clk / div) <= clock)
					break;
			}

			clk = SDHC_CLOCK_GENERATOR_SELECT;
			div--;
		} else {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock) {
				div = 1;
			} else {
				for (div = 2;
				     div < SDHC_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			div >>= 1;
		}
		clk |= SDHC_10BIT_DIVIDED_CLOCK(div);
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHC_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		div >>= 1;
		clk |= SDHC_8BIT_DIVIDED_CLOCK(div);
	}

	/* 2. set SDCLK/RCLK Frequency Select
	 *    Preset Value Enable?
	 */
	__raw_writew(clk, SDHC_CLOCK_CONTROL(mmc_sid));

	/* 3. set Internal Clock Enable */
	sdhc_set_clock(mmc_sid, SDHC_INTERNAL_CLOCK_ENABLE);

	/* 4. check Internal Clock Stable */
	if (!sdhc_clock_stabilised())
		return false;

	/* 5. set PLL Enable */
	sdhc_enable_pll();

	sdhc_start_clock();

	if (clock > 26000000)
		sdhc_enable_high_speed(mmc_sid);
	else
		sdhc_disable_high_speed(mmc_sid);
	return true;
}

void sdhci_set_width(uint8_t width)
{
	struct sdhci_host *host = mmc2sdhci();
	uint32_t ctrl;

	/* Set bus width */
	ctrl = __raw_readb(SDHC_HOST_CONTROL_1(mmc_sid));
	if (width == 8) {
		ctrl &= ~SDHC_DATA_TRANSFER_WIDTH;
		if ((SDHC_SPEC(host) >=
		     SDHC_SPECIFICATION_VERSION_300))
			ctrl |= SDHC_EXTENDED_DATA_TRANSFER_WIDTH;
	} else {
		if ((SDHC_SPEC(host) >=
		     SDHC_SPECIFICATION_VERSION_300))
			ctrl &= ~SDHC_EXTENDED_DATA_TRANSFER_WIDTH;
		if (width == 4)
			ctrl |= SDHC_DATA_TRANSFER_WIDTH;
		else
			ctrl &= ~SDHC_DATA_TRANSFER_WIDTH;
	}
	__raw_writeb(ctrl, SDHC_HOST_CONTROL_1(mmc_sid));
}

void sdhci_init(void *base, uint32_t f_min, uint32_t f_max)
{
	struct sdhci_host *host = mmc2sdhci();
	uint32_t caps_0, caps_1;

	host->ioaddr = base;
	caps_0 = __raw_readl(SDHC_CAPABILITIES_0(mmc_sid));

#ifdef CONFIG_SDHC_SDMA
	BUG_ON(!(caps_0 & SDHC_CAP_SDMA_SUPPORT));
#endif
	host->version = __raw_readw(SDHC_HOST_CONTROLLER_VERSION(mmc_sid));

	/* Check whether the clock multiplier is supported or not */
	if (SDHC_SPEC(host) >= SDHC_SPECIFICATION_VERSION_300) {
		caps_1 = __raw_readl(SDHC_CAPABILITIES_1(mmc_sid));
		host->clk_mul = SDHC_CAP_CLOCK_MULTIPLIER(caps_1);
	}

	if (host->max_clk == 0) {
		if (SDHC_SPEC(host) >= SDHC_SPECIFICATION_VERSION_300)
			host->max_clk =
				SDHC_CAP_8BIT_BASE_CLOCK_FREQUENCY(caps_0);
		else
			host->max_clk =
				SDHC_CAP_6BIT_BASE_CLOCK_FREQUENCY(caps_0);
		host->max_clk *= 1000000;
		if (host->clk_mul)
			host->max_clk *= host->clk_mul;
	}
	BUG_ON(host->max_clk == 0);
	if (f_max && (f_max < host->max_clk))
		mmc_slot_ctrl.f_max = f_max;
	else
		mmc_slot_ctrl.f_max = host->max_clk;
	if (f_min)
		mmc_slot_ctrl.f_min = f_min;
	else {
		if (SDHC_SPEC(host) >= SDHC_SPECIFICATION_VERSION_300)
			mmc_slot_ctrl.f_min =
				mmc_slot_ctrl.f_max / SDHC_MAX_DIV_SPEC_300;
		else
			mmc_slot_ctrl.f_min =
				mmc_slot_ctrl.f_max / SDHC_MAX_DIV_SPEC_200;
	}
	printf("clock: %dHz ~ %dHz\n",
	       mmc_slot_ctrl.f_min, mmc_slot_ctrl.f_max);

	mmc_slot_ctrl.host_ocr = SD_OCR_HCS;
	if (caps_0 & SDHC_CAP_VOLTAGE_SUPPORT_330)
		mmc_slot_ctrl.host_ocr |= (MMC_OCR_32_33 | MMC_OCR_33_34);
	if (caps_0 & SDHC_CAP_VOLTAGE_SUPPORT_300)
		mmc_slot_ctrl.host_ocr |= (MMC_OCR_29_30 | MMC_OCR_30_31);
	if (caps_0 & SDHC_CAP_VOLTAGE_SUPPORT_180)
		mmc_slot_ctrl.host_ocr |= MMC_OCR_170_195;

	mmc_slot_ctrl.host_scr.bus_widths = 4;
#if 0
	MMC_MODE_HS_52MHz;
#endif

	/* Since Host Controller Version3.0 */
	if (SDHC_SPEC(host) >= SDHC_SPECIFICATION_VERSION_300) {
		if (caps_0 & SDHC_8BIT_SUPPORT_FOR_EMBEDDED_DEVICE)
			mmc_slot_ctrl.host_scr.bus_widths = 8;
	}

	/* Mask all SDHC IRQ sources, let sdhci_irq_init() enables the
	 * required IRQ sources.
	 */
	sdhc_mask_all_irqs(mmc_sid);

	sdhci_reset(SDHC_SOFTWARE_RESET_FOR_ALL);
	sdhci_set_power(__fls32(
			MMC_OCR_VOLTAGE_RANGE(mmc_slot_ctrl.host_ocr)));
}

void sdhc_handle_irq(void)
{
	uint32_t irqs = sdhc_irq_status(mmc_sid);
	uint8_t type = mmc_get_block_data();

	if (irqs & SDHC_CARD_DETECTION_MASK) {
		if (irqs & SDHC_CARD_INSERTION) {
			sdhc_clear_irq(mmc_sid, SDHC_CARD_INSERTION);
			if (sdhc_state_present(mmc_sid,
					       SDHC_CARD_INSERTED))
				mmc_event_raise(MMC_EVENT_CARD_INSERT);
		}
		if (irqs & SDHC_CARD_REMOVAL) {
			sdhc_clear_irq(mmc_sid, SDHC_CARD_REMOVAL);
			if (!sdhc_state_present(mmc_sid,
						SDHC_CARD_INSERTED))
				mmc_event_raise(MMC_EVENT_CARD_REMOVE);
		}
	}
	/* Handle cmd/data IRQs */
	if (irqs & SDHC_ERROR_INTERRUPT_MASK) {
		if (irqs & SDHC_COMMAND_INDEX_ERROR)
			mmc_cmd_failure(MMC_ERR_ILLEGAL_COMMAND);
		else if (irqs & SDHC_CURRENT_LIMIT_ERROR)
			mmc_cmd_failure(MMC_ERR_CARD_NON_COMP_VOLT);
		else if (irqs & SDHC_TRANSFER_TIMEOUT)
			mmc_cmd_failure(MMC_ERR_TIMEOUT);
		else if (irqs & SDHC_TRANSFER_FAILURE)
			mmc_cmd_failure(MMC_ERR_CARD_LOOSE_BUS);
		else
			mmc_cmd_failure(MMC_ERR_CARD_LOOSE_BUS);
		sdhc_clear_irq(mmc_sid, SDHC_ERROR_INTERRUPT_MASK);
		return;
	}
	if (irqs & SDHC_COMMAND_COMPLETE) {
		if (type)
			sdhci_transfer_data();
		mmc_cmd_success();
	}
}

void sdhc_irq_handler(void)
{
	mmc_slot_t slot;
	__unused mmc_slot_t sslot;

	sslot = mmc_slot_save(slot);
	for (slot = 0; slot < NR_MMC_SLOTS; slot++) {
		sslot = mmc_slot_save(slot);
		sdhc_handle_irq();
		mmc_slot_restore(sslot);
	}
}

#ifdef SYS_REALTIME
void sdhci_irq_poll(void)
{
	sdhc_irq_handler();
}
#else
void sdhci_irq_init(void)
{
}
#endif

void sdhci_detect_card(void)
{
	if (sdhc_state_present(mmc_sid, SDHC_CARD_INSERTED))
		mmc_event_raise(MMC_EVENT_CARD_INSERT);
	sdhc_enable_irq(mmc_sid, SDHC_CARD_DETECTION_MASK);
}

void sdhci_start_transfer(void)
{
	sdhc_clear_all_irqs(mmc_sid);
	sdhc_enable_irq(mmc_sid, SDHC_COMMAND_MASK | SDHC_TRANSFER_MASK);
}

void sdhci_stop_transfer(void)
{
	sdhc_disable_irq(mmc_sid, SDHC_COMMAND_MASK | SDHC_TRANSFER_MASK);
	sdhc_clear_all_irqs(mmc_sid);
	sdhci_reset(SDHC_SOFTWARE_RESET_FOR_CMD_LINE);
	sdhci_reset(SDHC_SOFTWARE_RESET_FOR_DAT_LINE);
}
