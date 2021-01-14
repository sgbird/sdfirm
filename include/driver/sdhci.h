#ifndef __SDHCI_H_INCLUDE__
#define __SDHCI_H_INCLUDE__

#include <target/arch.h>

#ifndef SDHC_REG
#define SDHC_REG(n, offset)	(SDHC##n##_BASE + (offset))
#endif

/* Controller registers */
#define SDHC_SDMA_SYSTEM_ADDRESS(n)		SDHC_REG(n, 0x00) /* 32-bits */
#define SDHC_32BIT_BLOCK_COUNT(n)		SDHC_REG(n, 0x00) /* 32-bits */
#define SDHC_BLOCK_SIZE(n)			SDHC_REG(n, 0x04) /* 16-bits */
#define SDHC_16BIT_BLOCK_COUNT(n)		SDHC_REG(n, 0x06) /* 16-bits */
#define SDHC_ARGUMENT(n)			SDHC_REG(n, 0x08) /* 32-bits */
#define SDHC_TRANSFER_MODE(n)			SDHC_REG(n, 0x0C) /* 16-bits */
#define SDHC_COMMAND(n)				SDHC_REG(n, 0x0E) /* 32-bits */
#define SDHC_RESPONSE(n, i)			SDHC_REG(n, 0x10 + (i))
						                  /* 128-bits */
#define SDHC_BUFFER_DATA_PORT(n)		SDHC_REG(n, 0x20) /* 32-bits */
#define SDHC_PRESENT_STATE(n)			SDHC_REG(n, 0x24) /* 32-bits */
#define SDHC_HOST_CONTROL_1(n)			SDHC_REG(n, 0x28) /* 8-bits */
#define SDHC_POWER_CONTROL(n)			SDHC_REG(n, 0x29) /* 8-bits */
#define SDHC_CLOCK_CONTROL(n)			SDHC_REG(n, 0x2C) /* 16-bits */
#define SDHC_TIMEOUT_CONTROL(n)			SDHC_REG(n, 0x2E) /* 16-bits */
#define SDHC_SOFTWARE_RESET(n)			SDHC_REG(n, 0x2F) /* 8-bits */
#define SDHC_NORMAL_INTERRUPT_STATUS(n)		SDHC_REG(n, 0x30) /* 16-bits */
#define SDHC_ERROR_INTERRUPT_STATUS(n)		SDHC_REG(n, 0x32) /* 16-bits */
#define SDHC_NORMAL_INTERRUPT_STATUS_ENABLE(n)	SDHC_REG(n, 0x34) /* 16-bits */
#define SDHC_ERROR_INTERRUPT_STATUS_ENABLE(n)	SDHC_REG(n, 0x36) /* 16-bits */
#define SDHC_NORMAL_INTERRUPT_SIGNAL_ENABLE(n)	SDHC_REG(n, 0x38) /* 16-bits */
#define SDHC_ERROR_INTERRUPT_SIGNAL_ENABLE(n)	SDHC_REG(n, 0x3A) /* 16-bits */
#define SDHC_CAPABILITIES(n)			SDHC_REG(n, 0x40) /* 64-bits */
#define SDHC_HOST_CONTROLLER_VERSION(n)		SDHC_REG(n, 0xFE) /* 16-bits */

/* Registers for aligned architecture */
#define SDHC_CLOCK_TIMEOUT_CONTROL(n)		SDHC_REG(n, 0x2C) /* 32-bits */
#define SDHC_INTERRUPT_STATUS(n)		SDHC_REG(n, 0x30) /* 32-bits */
#define SDHC_INTERRUPT_ENABLE(n)		SDHC_REG(n, 0x34) /* 32-bits */
#define SDHC_INTERRUPT_SIGNAL(n)		SDHC_REG(n, 0x38) /* 32-bits */
#define SDHC_CAPABILITIES_0(n)			SDHC_REG(n, 0x40) /* 32-bits */
#define SDHC_CAPABILITIES_1(n)			SDHC_REG(n, 0x44) /* 32-bits */

/* 2.2.2 Block Size Register (Cat.A Offset 004h) */
#define SDHC_SDMA_BUFFER_BOUNDARY_OFFSET	12
#define SDHC_SDMA_BUFFER_BOUNDARY_MASK		REG_3BIT_MASK
#define SDHC_SDMA_BUFFER_BOUNDARY(value)	\
	_SET_FV(SDHC_SDMA_BUFFER_BOUNDARY, value)
#define SDHC_TRANSFER_BLOCK_SIZE_OFFSET		0
#define SDHC_TRANSFER_BLOCK_SIZE_MASK		REG_12BIT_MASK
#define SDHC_TRANSFER_BLOCK_SIZE(value)		\
	_SET_FV(SDHC_TRANSFER_BLOCK_SIZE, value)
/* 2.2.5 Transfer Mode Register (Cat.A Offset 00Ch) */
#define SDHC_RESPONSE_INTERRUPT_DISABLE		_BV(8)
#define SDHC_RESPONSE_ERROR_CHECK_ENABLE	_BV(7)
#define SDHC_RESPONSE_TYPE_R1_R5		_BV(6)
#define SDHC_MULTI_SINGLE_BLOCK_SELECT		_BV(5)
#define SDHC_DATA_TRANSFER_DIRECTION_SELECT	_BV(4)
#define SDHC_AUTO_CMD_ENABLE_OFFSET		2
#define SDHC_AUTO_CMD_ENABLE_MASK		REG_2BIT_MASK
#define SDHC_AUTO_CMD_ENABLE(value)		\
	_SET_FV(SDHC_AUTO_CMD_ENABLE, value)
#define SDHC_AUTO_COMMAND_DISABLE		0
#define SDHC_AUTO_CMD12_ENABLE			1
#define SDHC_AUTO_CMD23_ENABLE			2
#define SDHC_ACMD12				_BV(2)
#define SDHC_ACMD23				_BV(3)
#ifdef CONFIG_SDHC_SPEC_4_10
#define SDHC_AUTO_CMD_AUTO_SELECT		3
#endif
#define SDHC_BLOCK_COUNT_ENABLE			_BV(1)
#define SDHC_DMA_ENABLE				_BV(0)
/* 2.2.6 Command Register (Cat.A Offset 00Eh) */
#define SDHC_COMMAND_INDEX_OFFSET		8
#define SDHC_COMMAND_INDEX_MASK			REG_6BIT_MASK
#define SDHC_COMMAND_INDEX(value)		\
	_SET_FV(SDHC_COMMAND_INDEX, value)
#define SDHC_COMMAND_TYPE_OFFSET		6
#define SDHC_COMMAND_TYPE_MASK			REG_2BIT_MASK
#define SDHC_COMMAND_TYPE(value)		\
	_SET_FV(SDHC_COMMAND_TYPE, value)
#define SDHC_NORMAL_COMMAND			0
#define SDHC_SUSPEND_COMMAND			1
#define SDHC_RESUME_COMMAND			2
#define SDHC_ABORT_COMMAND			3
#define SDHC_DATA_PRESENT_SELECT		_BV(5)
#define SDHC_COMMAND_INDEX_CHECK_ENABLE		_BV(4)
#define SDHC_COMMAND_CRC_CHECK_ENABLE		_BV(3)
#define SDHC_SUB_COMMAND_FLAG			_BV(2)
#define SDHC_RESPONSE_TYPE_SELECT_OFFSET	0
#define SDHC_RESPONSE_TYPE_SELECT_MASK		REG_2BIT_MASK
#define SDHC_RESPONSE_TYPE_SELECT(value)	\
	_SET_FV(SDHC_RESPONSE_TYPE_SELECT, value)
#define SDHC_NO_RESPONSE			0
#define SDHC_RESPONSE_LENGTH_136		1
#define SDHC_RESPONSE_LENGTH_48			2
#define SDHC_RESPONSE_LENGTH_48_BUSY		3

#define SDHC_CMD(c, f)				\
	(((c & 0xff) << 8) | (f & 0xff))
/* 2.2.9 Present State Register (Cat.C Offset 024h) */
#ifdef CONFIG_SDHC_UHSII
#define SDHC_UHSII_IF_DETECTION			_BV(31)
#define SDHC_LANE_SYNCHRONIZATION		_BV(30)
#define SDHC_IN_DORMANT_STATUS			_BV(29)
#endif
#define SDHC_SUB_COMMAND_STATUS			_BV(28)
#define SDHC_COMMAND_NOT_ISSUED_BY_ERROR	_BV(27)
#ifdef CONFIG_SDHC_SPEC_4_10
#define SDHC_HOST_REGULATOR_VOLTAGE_STABLE	_BV(26)
#endif
#ifdef CONFIG_SDHC_SD
#define SDHC_CMD_LINE_SIGNAL_LEVEL		_BV(25)
#define SDHC_DAT30_LINE_SIGNAL_LEVEL_OFFSET	20
#define SDHC_DAT30_LINE_SIGNAL_LEVEL_MASK	REG_4BIT_MASK
#define SDHC_DAT30_LINE_SIGNAL_LEVEL(value)	\
	_GET_FV(SDHC_DAT30_LINE_SIGNAL_LEVEL, value)
#define SDHC_DAT3_LINE_SIGNAL_LEVEL		_BV(23)
#define SDHC_DAT2_LINE_SIGNAL_LEVEL		_BV(22)
#define SDHC_DAT1_LINE_SIGNAL_LEVEL		_BV(21)
#define SDHC_DAT0_LINE_SIGNAL_LEVEL		_BV(20)
#endif
#define SDHC_WRITE_PROTECT_SWITCH_PIN_LEVEL	_BV(19)
#define SDHC_CARD_DETECT_PIN_LEVEL		_BV(18)
#define SDHC_CARD_STATE_STABLE			_BV(17)
#define SDHC_CARD_INSERTED			_BV(16)
#define SDHC_BUFFER_READ_ENABLE			_BV(11)
#define SDHC_BUFFER_WRITE_ENABLE		_BV(10)
#ifdef CONFIG_SDHC_SD
#define SDHC_READ_TRANSFER_ACTIVE		_BV(9)
#define SDHC_WRITE_TRANSFER_ACTIVE		_BV(8)
#endif
#ifdef CONFIG_SDHC_EMMC
#define SDHC_DAT74_LINE_SIGNAL_LEVEL_OFFSET	4
#define SDHC_DAT74_LINE_SIGNAL_LEVEL_MASK	REG_4BIT_MASK
#define SDHC_DAT74_LINE_SIGNAL_LEVEL(value)	\
	_GET_FV(SDHC_DAT74_LINE_SIGNAL_LEVEL, value)
#define SDHC_DAT7_LINE_SIGNAL_LEVEL		_BV(7)
#define SDHC_DAT6_LINE_SIGNAL_LEVEL		_BV(6)
#define SDHC_DAT5_LINE_SIGNAL_LEVEL		_BV(5)
#define SDHC_DAT4_LINE_SIGNAL_LEVEL		_BV(4)
#define SDHC_DAT_LINE_SIGNAL_LEVEL(value)	\
	(SDHC_DAT30_LINE_SIGNAL_LEVEL(value) |	\
	 (value) & SDHC_DAT74_LINE_SIGNAL_LEVEL_MASK)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_RETUNING_REQUEST			_BV(3)
#endif
#ifdef CONFIG_SDHC_SD
#define SDHC_DAT_LINE_ACTIVE			_BV(2)
#define SDHC_COMMAND_INHIBIT_DAT		_BV(1)
#else
#define SDHC_COMMAND_INHIBIT_DAT		0
#endif
#define SDHC_COMMAND_INHIBIT_CMD		_BV(0)
#define SDHC_COMMAND_INHIBIT			\
	(SDHC_COMMAND_INHIBIT_DAT | SDHC_COMMAND_INHIBIT_CMD)

/* 2.2.11 Host Control 1 Register (Cat.C Offset 028h) */
#define SDHC_CARD_DETECT_SIGNAL_SELECTION	_BV(7)
#define SDHC_CARD_DETECT_TEST_LEVEL		_BV(6)
#ifdef CONFIG_SDHC_SD
#define SDHC_EXTENDED_DATA_TRANSFER_WIDTH	_BV(5)
#endif
#define SDHC_DMA_SELECT_OFFSET			3
#define SDHC_DMA_SELECT_MASK			REG_2BIT_MASK
#define SDHC_DMA_SELECT(value)			\
	_SET_FV(SDHC_DMA_SELECT, value)
#define SDHC_SDMA				0
#if defined(CONFIG_SDHC_SPEC_4)
#define SDHC_ADMA2				2
#define SDHC_ADMA2_ADMA3			3
#elif defined(CONFIG_SDHC_SPEC_3)
#define SDHC_32BIT_ADMA2			2
#define SDHC_64BIT_ADMA2			3
#endif
#ifdef CONFIG_SDHC_SD
#define SDHC_HIGH_SPEED_ENABLE			_BV(2)
#define SDHC_DATA_TRANSFER_WIDTH		_BV(1)
#endif
#define SDHC_LED_CONTROL			_BV(0)

/* 2.2.12 Power Control Register (Cat.C Offset 029h) */
#define SDHC_SD_BUS_POWER_FOR_VDD1			_BV(0)
#define SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD1_OFFSET	1
#define SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD1_MASK	REG_3BIT_MASK
#define SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD1(value)	\
	_SET_FV(SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD1, value)
#ifdef CONFIG_SDHC_UHSII
#define SDHC_SD_BUS_POWER_FOR_VDD2			_BV(4)
#define SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2_OFFSET	5
#define SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2_MASK	REG_3BIT_MASK
#define SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2(value)	\
	_SET_FV(SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2, value)
#endif
#define SDHC_SD_BUS_POWER_120				0x4
#define SDHC_SD_BUS_POWER_180				0x5
#define SDHC_SD_BUS_POWER_300				0x6
#define SDHC_SD_BUS_POWER_330				0x7

/* 2.2.15 Clock Control Register (Cat.C Offset 02Ch) */
#define SDHC_8BIT_DIVIDED_CLOCK_OFFSET		8
#define SDHC_8BIT_DIVIDED_CLOCK_MASK		REG_8BIT_MASK
#define SDHC_8BIT_DIVIDED_CLOCK(value)		\
	_SET_FV(SDHC_8BIT_DIVIDED_CLOCK, value)
#define SDHC_10BIT_DIVIDED_CLOCK_OFFSET		6
#define SDHC_10BIT_DIVIDED_CLOCK_MASK		REG_10BIT_MASK
#define SDHC_10BIT_DIVIDED_CLOCK(value)		\
	_SET_FV(SDHC_10BIT_DIVIDED_CLOCK, value)
#define SDHC_CLOCK_GENERATOR_SELECT		_BV(5)
/* 0: Divided Clock Mode, 1: Programmable Clock Mode */
#ifdef CONFIG_SDHC_SPEC_4_10
#define SDHC_PLL_ENABLE				_BV(3)
#endif
#define SDHC_CLOCK_ENABLE			_BV(2)
#define SDHC_INTERNAL_CLOCK_STABLE		_BV(1)
#define SDHC_INTERNAL_CLOCK_ENABLE		_BV(0)

/* 2.2.17 Software Reset Register (Cat.C Offset 02Fh) */
#ifdef CONFIG_SDHC_SD
#define SDHC_SOFTWARE_RESET_FOR_DAT_LINE	_BV(2)
#endif
#define SDHC_SOFTWARE_RESET_FOR_CMD_LINE	_BV(1)
#define SDHC_SOFTWARE_RESET_FOR_ALL		_BV(0)

/* 2.2.18 Normal Interrupt Status Register (Cat.C Offset 030h)
 * 2.2.19 Error Interrupt Status Register (Cat.C Offset 032h)
 * 2.2.20 Normal Interrupt Status Enable Register (Cat.C Offset 034h)
 * 2.2.21 Error Interrupt Status Enable Register (Cat.C Offset 036h)
 * 2.2.22 Normal Interrupt Signal Enable Register (Cat.C Offset 038h)
 * 2.2.23 Error Interrupt Signal Enable Register (Cat.C Offset 03Ah)
 */
#ifdef CONFIG_SDHC_SPEC_4
#define SDHC_RESPONSE_ERROR		_BV(27)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_TUNING_ERROR		_BV(26)
#endif
#define SDHC_ADMA_ERROR			_BV(25)
#ifdef CONFIG_SDHC_SD
#define SDHC_AUTO_CMD_ERROR		_BV(24)
#endif
#define SDHC_CURRENT_LIMIT_ERROR	_BV(23)
#ifdef CONFIG_SDHC_SD
#define SDHC_DATA_END_BIT_ERROR		_BV(22)
#define SDHC_DATA_CRC_ERROR		_BV(21)
#define SDHC_DATA_TIMEOUT_ERROR		_BV(20)
#define SDHC_COMMAND_INDEX_ERROR	_BV(19)
#define SDHC_COMMAND_END_BIT_ERROR	_BV(18)
#define SDHC_COMMAND_CRC_ERROR		_BV(17)
#define SDHC_COMMAND_TIMEOUT_ERROR	_BV(16)
#endif
#define SDHC_ERROR_INTERRUPT		_BV(15)
#ifdef CONFIG_SDHC_SPEC_4_10
#define SDHC_FX_EVENT			_BV(13)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_RETUNING_EVENT		_BV(12)
#endif
#define SDHC_INT_C			_BV(11)
#define SDHC_INT_B			_BV(10)
#define SDHC_INT_A			_BV(9)
#define SDHC_CARD_INTERRUPT		_BV(8)
#define SDHC_CARD_REMOVAL		_BV(7)
#define SDHC_CARD_INSERTION		_BV(6)
#define SDHC_BUFFER_READ_READY		_BV(5)
#define SDHC_BUFFER_WRITE_READY		_BV(4)
#define SDHC_DMA_INTERRUPT		_BV(3)
#define SDHC_BLOCK_GAP_EVENT		_BV(2)
#define SDHC_TRANSFER_COMPLETE		_BV(1)
#define SDHC_COMMAND_COMPLETE		_BV(0)
#define SDHC_NORMAL_INTERRUPT_MASK	0x00007FFF
#define SDHC_ERROR_INTERRUPT_MASK	0xFFFF8000
#define SDHC_ALL_INTERRUPT_MASK		\
	(SDHC_NORMAL_INTERRUPT_MASK | SDHC_ERROR_INTERRUPT_MASK)
#define SDHC_TRANSFER_FAILURE					\
	(SDHC_COMMAND_CRC_ERROR | SDHC_COMMAND_END_BIT_ERROR |	\
         SDHC_DATA_CRC_ERROR | SDHC_DATA_END_BIT_ERROR)
#define SDHC_TRANSFER_TIMEOUT					\
	(SDHC_COMMAND_TIMEOUT_ERROR | SDHC_DATA_TIMEOUT_ERROR)
#define SDHC_TRANSFER_SUCCESS					\
	(SDHC_TRANSFER_COMPLETE | SDHC_COMMAND_COMPLETE)

/* 2.2.18 Normal Interrupt Status Register (Cat.C Offset 030h)
 * 2.2.20 Normal Interrupt Status Enable Register (Cat.C Offset 036h)
 * 2.2.22 Normal Interrupt Signal Enable Register (Cat.C Offset 038h)
 */
#define SDHC_NOR_ERROR_INTERRUPT	_BV(15)
#ifdef CONFIG_SDHC_SPEC_4_10
#define SDHC_NOR_FX_EVENT		_BV(13)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_NOR_RETUNING_EVENT		_BV(12)
#endif
#define SDHC_NOR_INT_C			_BV(11)
#define SDHC_NOR_INT_B			_BV(10)
#define SDHC_NOR_INT_A			_BV(9)
#define SDHC_NOR_CARD_INTERRUPT		_BV(8)
#define SDHC_NOR_CARD_REMOVAL		_BV(7)
#define SDHC_NOR_CARD_INSERTION		_BV(6)
#define SDHC_NOR_BUFFER_READ_READY	_BV(5)
#define SDHC_NOR_BUFFER_WRITE_READY	_BV(4)
#define SDHC_NOR_DMA_INTERRUPT		_BV(3)
#define SDHC_NOR_BLOCK_GAP_EVENT	_BV(2)
#define SDHC_NOR_TRANSFER_COMPLETE	_BV(1)
#define SDHC_NOR_COMMAND_COMPLETE	_BV(0)
/* 2.2.19 Error Interrupt Status Register (Cat.C Offset 032h)
 * 2.2.21 Error Interrupt Status Enable Register (Cat.C Offset 036h)
 * 2.2.23 Error Interrupt Signal Enable Register (Cat.C Offset 03Ah)
 */
#ifdef CONFIG_SDHC_SPEC_4
#define SDHC_ERR_RESPONSE_ERROR		_BV(11)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_ERR_TUNING_ERROR		_BV(10)
#endif
#define SDHC_ERR_ADMA_ERROR		_BV(9)
#ifdef CONFIG_SDHC_SD
#define SDHC_ERR_AUTO_CMD_ERROR		_BV(8)
#endif
#define SDHC_ERR_CURRENT_LIMIT_ERROR	_BV(7)
#ifdef CONFIG_SDHC_SD
#define SDHC_ERR_DATA_END_BIT_ERROR	_BV(6)
#define SDHC_ERR_DATA_CRC_ERROR		_BV(5)
#define SDHC_ERR_DATA_TIMEOUT_ERROR	_BV(4)
#define SDHC_ERR_COMMAND_INDEX_ERROR	_BV(3)
#define SDHC_ERR_COMMAND_END_BIT_ERROR	_BV(2)
#define SDHC_ERR_COMMAND_CRC_ERROR	_BV(1)
#define SDHC_ERR_COMMAND_TIMEOUT_ERROR	_BV(0)
#endif

#define SDHC_COMMAND_MASK					\
	(SDHC_COMMAND_COMPLETE | SDHC_COMMAND_TIMEOUT_ERROR |	\
	 SDHC_COMMAND_CRC_ERROR | SDHC_COMMAND_END_BIT_ERROR |	\
	 SDHC_COMMAND_INDEX_ERROR)
#define SDHC_TRANSFER_MASK					\
	(SDHC_TRANSFER_COMPLETE | SDHC_DMA_INTERRUPT |		\
	 SDHC_BUFFER_READ_READY | SDHC_BUFFER_WRITE_READY |	\
	 SDHC_DATA_TIMEOUT_ERROR | SDHC_DATA_CRC_ERROR |	\
	 SDHC_DATA_END_BIT_ERROR | SDHC_ADMA_ERROR)
#define SDHC_CARD_DETECTION_MASK				\
	(SDHC_CARD_INSERTION | SDHC_CARD_REMOVAL)

/* 2.2.26 Capabilities Register (Cat.C Offset 040h) */
#define SDHC_180_VDD2_SUPPORT			_BV_ULL(60)
#define SDHC_ADMA3_SUPPORT			_BV_ULL(59)
#define SDHC_CLOCK_MULTIPLIER_OFFSET		48
#define SDHC_CLOCK_MULTIPLIER_MASK		REG_8BIT_MASK
#define SDHC_CLOCK_MULTIPLIER(value)		\
	_GET_FV_ULL(SDHC_CLOCK_MULTIPLIER, value)
#ifdef CONFIG_SDHC_UHSI
#define SDHC_RE_TUNING_MODES_OFFSET		46
#define SDHC_RE_TUNING_MODES_MASK		REG_2BIT_MASK
#define SDHC_RE_TUNING_MODES(value)		\
	_GET_FV_ULL(SDHC_RE_TUNING_MODES, value)
#define SDHC_RE_TUNING_MODE_1			0
#define SDHC_RE_TUNING_MODE_2			1
#define SDHC_RE_TUNING_MODE_3			2
#define SDHC_USE_TUNING_FOR_SDR50		_BV_ULL(45)
#define SDHC_TIMER_COUNT_FOR_RE_TUNING_OFFSET	40
#define SDHC_TIMER_COUNT_FOR_RE_TUNING_MASK	REG_4BIT_MASK
#define SDHC_TIMER_COUNT_FOR_RE_TUNING(value)	\
	_GET_FV_ULL(SDHC_TIMER_COUNT_FOR_RE_TUNING, value)
#define SDHC_DRIVER_TYPE_D_SUPPORT		_BV_ULL(38)
#define SDHC_DRIVER_TYPE_C_SUPPORT		_BV_ULL(37)
#define SDHC_DRIVER_TYPE_A_SUPPORT		_BV_ULL(36)
#endif
#ifdef CONFIG_SDHC_UHSII
#define SDHC_UHSII_SUPPORT			_BV_ULL(35)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_DDR50_SUPPORT			_BV_ULL(34)
#define SDHC_SDR104_SUPPORT			_BV_ULL(33)
#define SDHC_SDR50_SUPPORT			_BV_ULL(32)
#endif
#define SDHC_SLOT_TYPE_OFFSET			30
#define SDHC_SLOT_TYPE_MASK			REG_2BIT_MASK
#define SDHC_SLOT_TYPE(value)			\
	_GET_FV_ULL(SDHC_SLOT_TYPE, value)
#define SDHC_SLOT_REMOVABLE_CARD_SLOT			0
#define SDHC_SLOT_EMBEDDED_SLOT_FOR_ONE_DEVICE		1
#define SDHC_SLOT_SHARED_BUS_SLOT_SD_MODE		2
#define SDHC_SLOT_UHSII_MULTIPLE_EMBEDDED_DEVICE	3
#define SDHC_ASYNCHRONOUS_INTERRUPT_SUPPORT	_BV_ULL(29)
#define SDHC_64BIT_SYSTEM_ADDRESS_SUPPORT_V3	_BV_ULL(28)
#define SDHC_64BIT_SYSTEM_ADDRESS_SUPPORT_V4	_BV_ULL(27)
#define SDHC_VOLTAGE_SUPPORT_180		_BV_ULL(26)
#define SDHC_VOLTAGE_SUPPORT_300		_BV_ULL(25)
#define SDHC_VOLTAGE_SUPPORT_330		_BV_ULL(24)
#define SDHC_SUSPEND_RESUME_SUPPORT		_BV_ULL(23)
#define SDHC_SDMA_SUPPORT			_BV_ULL(22)
#define SDHC_HIGH_SPEED_SUPPORT			_BV_ULL(21)
#define SDHC_ADMA2_SUPPORT			_BV_ULL(19)
#define SDHC_8BIT_SUPPORT_FOR_EMBEDDED_DEVICE	_BV_ULL(18)
#define SDHC_MAX_BLOCK_LENGTH_OFFSET		16
#define SDHC_MAX_BLOCK_LENGTH_MASK		REG_2BIT_MASK
#define SDHC_MAX_BLOCK_LENGTH(value)		\
	_GET_FV_ULL(SDHC_MAX_BLOCK_LENGTH, value)
#define SDHC_MAX_BLOCK_LENGTH_512		0
#define SDHC_MAX_BLOCK_LENGTH_1024		1
#define SDHC_MAX_BLOCK_LENGTH_2048		2
#define SDHC_6BIT_BASE_CLOCK_FREQUENCY_OFFSET	8
#define SDHC_8BIT_BASE_CLOCK_FREQUENCY_OFFSET	8
#ifdef CONFIG_SDHC_SPEC_3
#define SDHC_8BIT_BASE_CLOCK_FREQUENCY_MASK	REG_8BIT_MASK
#else
#define SDHC_8BIT_BASE_CLOCK_FREQUENCY_MASK	REG_6BIT_MASK
#endif
#define SDHC_6BIT_BASE_CLOCK_FREQUENCY_MASK	REG_6BIT_MASK
#define SDHC_6BIT_BASE_CLOCK_FREQUENCY(value)	\
	_GET_FV_ULL(SDHC_6BIT_BASE_CLOCK_FREQUENCY, value)
#define SDHC_8BIT_BASE_CLOCK_FREQUENCY(value)	\
	_GET_FV_ULL(SDHC_8BIT_BASE_CLOCK_FREQUENCY, value)
#define SDHC_TIMEOUT_CLOCK_UNIT			_BV_ULL(7)
#define SDHC_TIMEOUT_CLOCK_FREQUENCY_OFFSET	0
#define SDHC_TIMEOUT_CLOCK_FREQUENCY_MASK	REG_6BIT_MASK
#define SDHC_TIMEOUT_CLOCK_FREQUENCY(value)	\
	_GET_FV_ULL(SDHC_TIMEOUT_CLOCK_FREQUENCY, value)
/* 32-bits aligned version */
/* CAPABILITIES_1 */
#define SDHC_CAP_180_VDD2_SUPPORT			_BV(28)
#define SDHC_CAP_ADMA3_SUPPORT				_BV(27)
#define SDHC_CAP_CLOCK_MULTIPLIER_OFFSET		16
#define SDHC_CAP_CLOCK_MULTIPLIER_MASK			REG_8BIT_MASK
#define SDHC_CAP_CLOCK_MULTIPLIER(value)		\
	_GET_FV(SDHC_CAP_CLOCK_MULTIPLIER, value)
#ifdef CONFIG_SDHC_UHSI
#define SDHC_CAP_RE_TUNING_MODES_OFFSET			14
#define SDHC_CAP_RE_TUNING_MODES_MASK			REG_2BIT_MASK
#define SDHC_CAP_RE_TUNING_MODES(value)			\
	_GET_FV(SDHC_CAP_RE_TUNING_MODES, value)
#define SDHC_CAP_USE_TUNING_FOR_SDR50			_BV(13)
#define SDHC_CAP_TIMER_COUNT_FOR_RE_TUNING_OFFSET	8
#define SDHC_CAP_TIMER_COUNT_FOR_RE_TUNING_MASK		REG_4BIT_MASK
#define SDHC_CAP_TIMER_COUNT_FOR_RE_TUNING(value)	\
	_GET_FV(SDHC_CAP_TIMER_COUNT_FOR_RE_TUNING, value)
#define SDHC_CAP_DRIVER_TYPE_D_SUPPORT			_BV(6)
#define SDHC_CAP_DRIVER_TYPE_C_SUPPORT			_BV(5)
#define SDHC_CAP_DRIVER_TYPE_A_SUPPORT			_BV(4)
#endif
#ifdef CONFIG_SDHC_UHSII
#define SDHC_CAP_UHSII_SUPPORT				_BV(3)
#endif
#ifdef CONFIG_SDHC_UHSI
#define SDHC_CAP_DDR50_SUPPORT				_BV(2)
#define SDHC_CAP_SDR104_SUPPORT				_BV(1)
#define SDHC_CAP_SDR50_SUPPORT				_BV(0)
#endif
/* CAPABILITIES_0 */
#define SDHC_CAP_SLOT_TYPE_OFFSET			30
#define SDHC_CAP_SLOT_TYPE_MASK				REG_2BIT_MASK
#define SDHC_CAP_SLOT_TYPE(value)			\
	_GET_FV(SDHC_CAP_SLOT_TYPE, value)
#define SDHC_CAP_ASYNCHRONOUS_INTERRUPT_SUPPORT		_BV(29)
#define SDHC_CAP_64BIT_SYSTEM_ADDRESS_SUPPORT_V3	_BV(28)
#define SDHC_CAP_64BIT_SYSTEM_ADDRESS_SUPPORT_V4	_BV(27)
#define SDHC_CAP_VOLTAGE_SUPPORT_180			_BV(26)
#define SDHC_CAP_VOLTAGE_SUPPORT_300			_BV(25)
#define SDHC_CAP_VOLTAGE_SUPPORT_330			_BV(24)
#define SDHC_CAP_SUSPEND_RESUME_SUPPORT			_BV(23)
#define SDHC_CAP_SDMA_SUPPORT				_BV(22)
#define SDHC_CAP_HIGH_SPEED_SUPPORT			_BV(21)
#define SDHC_CAP_ADMA2_SUPPORT				_BV(19)
#define SDHC_CAP_8BIT_SUPPORT_FOR_EMBEDDED_DEVICE	_BV(18)
#define SDHC_CAP_MAX_BLOCK_LENGTH_OFFSET		16
#define SDHC_CAP_MAX_BLOCK_LENGTH_MASK			REG_2BIT_MASK
#define SDHC_CAP_MAX_BLOCK_LENGTH(value)		\
	_GET_FV(SDHC_CAP_MAX_BLOCK_LENGTH, value)
#define SDHC_CAP_6BIT_BASE_CLOCK_FREQUENCY_OFFSET	8
#define SDHC_CAP_8BIT_BASE_CLOCK_FREQUENCY_OFFSET	8
#ifdef CONFIG_SDHC_SPEC_3
#define SDHC_CAP_8BIT_BASE_CLOCK_FREQUENCY_MASK		REG_8BIT_MASK
#else
#define SDHC_CAP_8BIT_BASE_CLOCK_FREQUENCY_MASK		REG_6BIT_MASK
#endif
#define SDHC_CAP_6BIT_BASE_CLOCK_FREQUENCY_MASK		REG_6BIT_MASK
#define SDHC_CAP_6BIT_BASE_CLOCK_FREQUENCY(value)	\
	_GET_FV(SDHC_CAP_6BIT_BASE_CLOCK_FREQUENCY, value)
#define SDHC_CAP_8BIT_BASE_CLOCK_FREQUENCY(value)	\
	_GET_FV(SDHC_CAP_8BIT_BASE_CLOCK_FREQUENCY, value)
#define SDHC_CAP_TIMEOUT_CLOCK_UNIT			_BV(7)
#define SDHC_CAP_TIMEOUT_CLOCK_FREQUENCY_OFFSET		0
#define SDHC_CAP_TIMEOUT_CLOCK_FREQUENCY_MASK		REG_6BIT_MASK
#define SDHC_CAP_TIMEOUT_CLOCK_FREQUENCY(value)		\
	_GET_FV(SDHC_CAP_TIMEOUT_CLOCK_FREQUENCY, value)

/* 2.3.16 Host Controller Version Register (Cat.C Offset 0FEh) */
#define SDHC_VENDOR_VERSION_NUMBER_OFFSET		8
#define SDHC_VENDOR_VERSION_NUMBER_MASK			REG_8BIT_MASK
#define SDHC_VENDOR_VERSION_NUMBER(value)		\
	_GET_FV(SDHC_VENDOR_VERSION_NUMBER, value)
#define SDHC_SPECIFICATION_VERSION_NUMBER_OFFSET	0
#define SDHC_SPECIFICATION_VERSION_NUMBER_MASK		REG_8BIT_MASK
#define SDHC_SPECIFICATION_VERSION_NUMBER(value)	\
	_GET_FV(SDHC_SPECIFICATION_VERSION_NUMBER, value)
#define SDHC_SPECIFICATION_VERSION_100			0
#define SDHC_SPECIFICATION_VERSION_200			1
#define SDHC_SPECIFICATION_VERSION_300			2
#define SDHC_SPECIFICATION_VERSION_400			3
#define SDHC_SPECIFICATION_VERSION_410			4
#define SDHC_SPECIFICATION_VERSION_420			5

#define SDHC_SPEC(x)					\
	SDHC_SPECIFICATION_VERSION_NUMBER((x)->version)

/* Other definitions */
#define SDHC_MAX_DIV_SPEC_200	256
#define SDHC_MAX_DIV_SPEC_300	2046

#define SDHC_INTERNAL_CLOCK_STABLE_TOUT_MS	150

/* Host SDMA buffer boundary.
 * Valid values from 4K to 512K in powers of 2.
 */
#define SDHC_DEFAULT_BOUNDARY_SIZE	(512 * 1024)
#define SDHC_DEFAULT_BOUNDARY_ARG	(7)

struct sdhci_host {
	void *ioaddr;
	unsigned int version;
	unsigned int max_clk;   /* Maximum Base Clock frequency */
	unsigned int clk_mul;   /* Clock Multiplier value */
#if 0
	struct gpio_desc pwr_gpio;	/* Power GPIO */
	struct gpio_desc cd_gpio;	/* Card Detect GPIO */
#endif
};

#define __sdhc_enable_irq(mmc, irqs)	\
	__raw_setl(irqs, SDHC_INTERRUPT_ENABLE(mmc))
#define __sdhc_disable_irq(mmc, irqs)	\
	__raw_clearl(irqs, SDHC_INTERRUPT_ENABLE(mmc))
#ifdef SYS_REALTIME
#define sdhc_enable_irq(mmc, irqs)		\
	do {					\
		__sdhc_enable_irq(mmc, irqs);	\
		sdhc_mask_irq(mmc, irqs);	\
	} while (0)
#define sdhc_disable_irq(mmc, irqs)		\
	__sdhc_disable_irq(mmc, irqs)
#else
#define sdhc_enable_irq(mmc, irqs)		\
	do {					\
		__sdhc_enable_irq(mmc, irqs);	\
		sdhc_unmask_irq(mmc, irqs);	\
	} while (0)
#define sdhc_disable_irq(mmc, irqs)		\
	do {					\
		__sdhc_disable_irq(mmc, irqs);	\
		sdhc_mask_irq(mmc, irqs);	\
	} while (0)
#endif
#define sdhc_mask_irq(mmc, irqs)		\
	__raw_clearl(irqs, SDHC_INTERRUPT_SIGNAL(mmc))
#define sdhc_unmask_irq(mmc, irqs)		\
	__raw_setl(irqs, SDHC_INTERRUPT_SIGNAL(mmc))
#define sdhc_clear_irq(mmc, irqs)		\
	__raw_setl(irqs, SDHC_INTERRUPT_STATUS(mmc))
#define sdhc_irq_status(mmc)			\
	__raw_readl(SDHC_INTERRUPT_STATUS(mmc))
#define sdhc_mask_all_irqs(mmc)			\
	__raw_writel(0, SDHC_INTERRUPT_SIGNAL(mmc))
#define sdhc_clear_all_irqs(mmc)		\
	__raw_writel(SDHC_ALL_INTERRUPT_MASK, SDHC_INTERRUPT_STATUS(mmc))

#define sdhc_power_off(mmc)			\
	__raw_writel(0, SDHC_POWER_CONTROL(mmc))
#define sdhc_power_on_vdd1(mmc, power)		\
	__raw_writel(SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD1(power) |	\
		     SDHC_SD_BUS_POWER_FOR_VDD1, SDHC_POWER_CONTROL(mmc))
#ifdef CONFIG_SDHC_UHSII
#define sdhc_power_on_vdd2(mmc, power)		\
	__raw_setl_mask(SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2(power) |	\
			SDHC_SD_BUS_POWER_FOR_VDD2,			\
			SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2(		\
			SDHC_SD_BUS_VOLTAGE_SELECT_FOR_VDD2_MASK) |	\
			SDHC_SD_BUS_POWER_FOR_VDD2,			\
			SDHC_POWER_CONTROL(mmc))
#else
#define sdhc_power_on_vdd2(mmc, power)		do { } while (0)
#endif

#define sdhc_state_present(mmc, state)		\
	(__raw_readl(SDHC_PRESENT_STATE(mmc)) & (state))

#define sdhc_set_clock(mmc, step)		\
	__raw_setw(step, SDHC_CLOCK_CONTROL(mmc))
#define sdhc_clear_clock(mmc, step)		\
	__raw_clearw(step, SDHC_CLOCK_CONTROL(mmc))

#ifdef CONFIG_SDHC_SD
#define sdhc_enable_high_speed(mmc)		\
	__raw_setb(SDHC_HIGH_SPEED_ENABLE, SDHC_HOST_CONTROL_1(mmc_sid))
#define sdhc_disable_high_speed(mmc)		\
	__raw_clearb(SDHC_HIGH_SPEED_ENABLE, SDHC_HOST_CONTROL_1(mmc_sid))
#else
#define sdhc_enable_high_speed(mmc)		do { } while (0)
#define sdhc_disable_high_speed(mmc)		do { } while (0)
#endif
#define sdhc_config_dma(mmc, mode)			\
	__raw_writeb_mask(SDHC_DMA_SELECT(mode),	\
		SDHC_DMA_SELECT(SDHC_DMA_SELECT_MASK),	\
		SDHC_HOST_CONTROL_1(mmc))

void sdhci_send_command(uint8_t cmd, uint32_t arg);
void sdhci_recv_response(uint8_t *resp, uint8_t size);
void sdhci_tran_data(uint8_t *dat, uint32_t len, uint16_t cnt);
void sdhci_detect_card(void);
bool sdhci_set_clock(uint32_t clock);
void sdhci_set_width(uint8_t width);
bool sdhci_card_busy(void);
void sdhci_irq_init(void);
void sdhci_irq_poll(void);
void sdhci_start_transfer(void);
void sdhci_stop_transfer(void);
void sdhci_init(void *base, uint32_t f_min, uint32_t f_max);

#endif /* __SDHCI_H_INCLUDE__ */
