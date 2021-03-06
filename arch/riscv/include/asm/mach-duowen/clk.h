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
 * @(#)clk.h: DUOWEN clock controller definitions
 * $Id: clk.h,v 1.1 2019-11-06 10:36:00 zhenglv Exp $
 */

#ifndef __CLK_DUOWEN_H_INCLUDE__
#define __CLK_DUOWEN_H_INCLUDE__

#include <target/arch.h>
#include <asm/mach/crcntl.h>

#ifdef CONFIG_CRCNTL
#ifndef ARCH_HAVE_CLK
#define ARCH_HAVE_CLK		1
#else
#error "Multiple CLK controller defined"
#endif
#endif

#define NR_FREQPLANS		1
#define FREQPLAN_RUN		0
#define INVALID_FREQPLAN	NR_FREQPLANS

#define invalid_clk		clkid(0xFF, 0xFF)

#define CLK_INPUT		((clk_cat_t)0)
#define XO_CLK			((clk_clk_t)0)
#define TIC_CLK			((clk_clk_t)1)
#define JTAG_CLK		((clk_clk_t)2)
#define NR_INPUT_CLKS		(JTAG_CLK + 1)
#define xo_clk			clkid(CLK_INPUT, XO_CLK)
#define tic_clk			clkid(CLK_INPUT, TIC_CLK)
#define jrag_clk		clkid(CLK_INPUT, JTAG_CLK)

#define CLK_PLL			((clk_cat_t)1)
#define DDR_PLL			((clk_clk_t)0)
#define SOC_PLL			((clk_clk_t)1)
#define COHFAB_PLL		((clk_clk_t)2)
#define CL0_PLL			((clk_clk_t)3)
#define CL1_PLL			((clk_clk_t)4)
#define CL2_PLL			((clk_clk_t)5)
#define CL3_PLL			((clk_clk_t)6)
#define NR_PLL_CLKS		(CL3_PLL + 1)
#define ddr_pll			clkid(CLK_PLL, DDR_PLL)
#define soc_pll			clkid(CLK_PLL, SOC_PLL)
#define cohfab_pll		clkid(CLK_PLL, COHFAB_PLL)
#define cl0_pll			clkid(CLK_PLL, CL0_PLL)
#define cl1_pll			clkid(CLK_PLL, CL1_PLL)
#define cl2_pll			clkid(CLK_PLL, CL2_PLL)
#define cl3_pll			clkid(CLK_PLL, CL3_PLL)

#define CLK_SELECT		((clk_cat_t)2)
/* CLK_SEL_CFG */
#define SYSFAB_CLK_SEL		((clk_clk_t)0)
#define DDR_CLK_SEL		((clk_clk_t)1)
#define DDR_CLK_DIV4_SEL	((clk_clk_t)2)
#define NR_SELECT_CLKS		(DDR_CLK_DIV4_SEL + 1)
#define sysfab_clk_sel		clkid(CLK_SELECT, SYSFAB_CLK_SEL)
#define ddr_clk_sel		clkid(CLK_SELECT, DDR_CLK_SEL)
#define ddr_clk_div4_sel	clkid(CLK_SELECT, DDR_CLK_DIV4_SEL)
#define soc_clk			sysfab_clk_sel

#define CLK_OUTPUT		((clk_cat_t)3)
/* CLK_EN_CFG0 */
#define DMA_CLK			((clk_clk_t)0)
#define DDR_CLK			((clk_clk_t)1)
#define DDR_BYPASS_PCLK		((clk_clk_t)2)
#define PCIE_CLK		((clk_clk_t)4)
#define SYSFAB_DBG_CLK		((clk_clk_t)9)
#define SYSFAB_TIC_CLK		((clk_clk_t)10)
#define CLUSTER0_HCLK		((clk_clk_t)12)
#define CLUSTER1_HCLK		((clk_clk_t)13)
#define CLUSTER2_HCLK		((clk_clk_t)14)
#define CLUSTER3_HCLK		((clk_clk_t)15)
#define COHFAB_HCLK		((clk_clk_t)16)
#define SCSR_PCLK		((clk_clk_t)19)
#define TLMM_PCLK		((clk_clk_t)20)
#define PLIC_HCLK		((clk_clk_t)21)
/* #define TIC_CLK			((clk_clk_t)23)  */
#define CORESIGHT_CLK		((clk_clk_t)24)
#define TIMER0_PCLK		((clk_clk_t)25) /* SW_RST only */
#define TIMER1_PCLK		((clk_clk_t)26) /* SW_RST only */
#define TIMER2_PCLK		((clk_clk_t)27) /* SW_RST only */
#define TIMER3_PCLK		((clk_clk_t)28) /* SW_RST only */
#define WDT0_PCLK		((clk_clk_t)29)
#define WDT1_PCLK		((clk_clk_t)30)

/* CLK_EN_CFG1 */
#define GPIO0_PCLK		((clk_clk_t)32)
#define GPIO1_PCLK		((clk_clk_t)33)
#define GPIO2_PCLK		((clk_clk_t)34)
#define UART0_CLK		((clk_clk_t)35)
#define UART1_CLK		((clk_clk_t)36)
#define UART2_CLK		((clk_clk_t)37)
#define UART3_CLK		((clk_clk_t)38)
#define I2C0_CLK		((clk_clk_t)39)
#define I2C1_CLK		((clk_clk_t)40)
#define I2C2_CLK		((clk_clk_t)41)
#define I2C3_CLK		((clk_clk_t)42)
#define I2C4_CLK		((clk_clk_t)43)
#define I2C5_CLK		((clk_clk_t)44)
#define I2C6_CLK		((clk_clk_t)45)
#define I2C7_CLK		((clk_clk_t)46)
#define I2C8_CLK		((clk_clk_t)47)
#define I2C9_CLK		((clk_clk_t)48)
#define I2C10_CLK		((clk_clk_t)49)
#define I2C11_CLK		((clk_clk_t)50)
#define SPI0_CLK		((clk_clk_t)51)
#define SPI1_CLK		((clk_clk_t)52)
#define SPI2_CLK		((clk_clk_t)53)
#define SPI3_CLK		((clk_clk_t)54)
#define SPI4_CLK		((clk_clk_t)55)
#define SD_HCLK			((clk_clk_t)57)

/* CLK_EN_CFG2 */
#define TIMER0_1_CLK		((clk_clk_t)64)
#define TIMER0_2_CLK		((clk_clk_t)65)
#define TIMER0_3_CLK		((clk_clk_t)66)
#define TIMER0_4_CLK		((clk_clk_t)67)
#define TIMER0_5_CLK		((clk_clk_t)68)
#define TIMER0_6_CLK		((clk_clk_t)69)
#define TIMER0_7_CLK		((clk_clk_t)70)
#define TIMER0_8_CLK		((clk_clk_t)71)
#define TIMER1_1_CLK		((clk_clk_t)72)
#define TIMER1_2_CLK		((clk_clk_t)73)
#define TIMER1_3_CLK		((clk_clk_t)74)
#define TIMER1_4_CLK		((clk_clk_t)75)
#define TIMER1_5_CLK		((clk_clk_t)76)
#define TIMER1_6_CLK		((clk_clk_t)77)
#define TIMER1_7_CLK		((clk_clk_t)78)
#define TIMER1_8_CLK		((clk_clk_t)79)
#define TIMER2_1_CLK		((clk_clk_t)80)
#define TIMER2_2_CLK		((clk_clk_t)81)
#define TIMER2_3_CLK		((clk_clk_t)82)
#define TIMER2_4_CLK		((clk_clk_t)83)
#define TIMER2_5_CLK		((clk_clk_t)84)
#define TIMER2_6_CLK		((clk_clk_t)85)
#define TIMER2_7_CLK		((clk_clk_t)86)
#define TIMER2_8_CLK		((clk_clk_t)87)
#define TIMER3_CLK		((clk_clk_t)88)
#define TSENSOR0_CLK		((clk_clk_t)92)
#define TSENSOR1_CLK		((clk_clk_t)93)
#define TSENSOR2_CLK		((clk_clk_t)94)
#define TSENSOR3_CLK		((clk_clk_t)95)

#define NR_OUTPUT_CLKS		(TSENSOR3_CLK + 1)

/* CLK_EN_CFG0 */
/* Integrated overall clocks */
#define dma_clk			clkid(CLK_OUTPUT, DMA_CLK)
#define ddr_clk			clkid(CLK_OUTPUT, DDR_CLK)
#define ddr_bypass_pclk		clkid(CLK_OUTPUT, DDR_BYPASS_PCLK)
#define pcie_clk		clkid(CLK_OUTPUT, PCIE_CLK)
#define sysfab_dbg_clk		clkid(CLK_OUTPUT, SYSFAB_DBG_CLK)
#define sysfab_tic_clk		clkid(CLK_OUTPUT, SYSFAB_TIC_CLK)
/* Internal bus clocks */
#define cluster0_hclk		clkid(CLK_OUTPUT, CLUSTER0_HCLK)
#define cluster1_hclk		clkid(CLK_OUTPUT, CLUSTER1_HCLK)
#define cluster2_hclk		clkid(CLK_OUTPUT, CLUSTER2_HCLK)
#define cluster3_hclk		clkid(CLK_OUTPUT, CLUSTER3_HCLK)
#define cohfab_hclk		clkid(CLK_OUTPUT, COHFAB_HCLK)
#define scsr_pclk		clkid(CLK_OUTPUT, SCSR_PCLK)
#define tlmm_pclk		clkid(CLK_OUTPUT, TLMM_PCLK)
#define plic_hclk		clkid(CLK_OUTPUT, PLIC_HCLK)
/* #define tic_clk		clkid(CLK_OUTPUT, TIC_CLK) */
#define coresight_clk		clkid(CLK_OUTPUT, CORESIGHT_CLK)
#define timer0_pclk		clkid(CLK_OUTPUT, TIMER0_PCLK)
#define timer1_pclk		clkid(CLK_OUTPUT, TIMER1_PCLK)
#define timer2_pclk		clkid(CLK_OUTPUT, TIMER2_PCLK)
#define __timer3_pclk		clkid(CLK_OUTPUT, TIMER3_PCLK)
#define timer3_pclk		sysfab_half_clk
#define wdt0_pclk		clkid(CLK_OUTPUT, WDT0_PCLK)
#define wdt1_pclk		clkid(CLK_OUTPUT, WDT1_PCLK)
/* Alias for AO domain clocks */
#define imc_clk			sysfab_clk
#define ram_aclk		sysfab_clk
#define brom_hclk		sysfab_clk
/* Alias for overall clocks */
#define ram_clk			ram_aclk
#define rom_clk			brom_hclk
#define plic_clk		plic_hclk
#define scsr_clk		scsr_pclk
#define tlmm_clk		tlmm_pclk
#define wdt0_clk		wdt0_pclk
#define wdt1_clk		wdt0_pclk
/* Alias for internal bus clocks */
#define pcie_aclk		pcie_clk
#define pcie_aux_clk		xo_clk
/* TODO: dependency clocks:
 * dma_hclk: depends dma_clk, sources sysfab_half_clk
 * ddr_pclk: depends ddr_clk, sources sysfab_half_clk
 * pcie_pclk: depends pcie_clk, sources sysfab_half_clk
 * pcie_ref_clk_p: depends pcie_clk, sources soc_pll_div10
 * pcie_ref_clk_n: depends pcie_clk, sources soc_pll_div10
 */

/* CLK_EN_CFG1 */
/* Internal bus clocks */
#define gpio0_pclk		clkid(CLK_OUTPUT, GPIO0_PCLK)
#define gpio1_pclk		clkid(CLK_OUTPUT, GPIO1_PCLK)
#define gpio2_pclk		clkid(CLK_OUTPUT, GPIO2_PCLK)
/* Integrated overall clocks */
#define uart0_clk		clkid(CLK_OUTPUT, UART0_CLK)
#define uart1_clk		clkid(CLK_OUTPUT, UART1_CLK)
#define uart2_clk		clkid(CLK_OUTPUT, UART2_CLK)
#define uart3_clk		clkid(CLK_OUTPUT, UART3_CLK)
#define i2c0_clk		clkid(CLK_OUTPUT, I2C0_CLK)
#define i2c1_clk		clkid(CLK_OUTPUT, I2C1_CLK)
#define i2c2_clk		clkid(CLK_OUTPUT, I2C2_CLK)
#define i2c3_clk		clkid(CLK_OUTPUT, I2C3_CLK)
#define i2c4_clk		clkid(CLK_OUTPUT, I2C4_CLK)
#define i2c5_clk		clkid(CLK_OUTPUT, I2C5_CLK)
#define i2c6_clk		clkid(CLK_OUTPUT, I2C6_CLK)
#define i2c7_clk		clkid(CLK_OUTPUT, I2C7_CLK)
#define i2c8_clk		clkid(CLK_OUTPUT, I2C8_CLK)
#define i2c9_clk		clkid(CLK_OUTPUT, I2C9_CLK)
#define i2c10_clk		clkid(CLK_OUTPUT, I2C10_CLK)
#define i2c11_clk		clkid(CLK_OUTPUT, I2C11_CLK)
#define spi0_clk		clkid(CLK_OUTPUT, SPI0_CLK)
#define spi1_clk		clkid(CLK_OUTPUT, SPI1_CLK)
#define spi2_clk		clkid(CLK_OUTPUT, SPI2_CLK)
#define spi3_clk		clkid(CLK_OUTPUT, SPI3_CLK)
#define spi4_clk		clkid(CLK_OUTPUT, SPI4_CLK)
#define sd_hclk			clkid(CLK_OUTPUT, SD_HCLK)
#define sd_bclk			sd_hclk
#define sd_cclk_tx		sd_hclk
#define sd_cclk_rx		sd_hclk
/* Alias for internal bus clocks */
#define uart0_pclk		uart0_clk
#define uart1_pclk		uart1_clk
#define uart2_pclk		uart2_clk
#define uart3_pclk		uart3_clk
#define spi0_pclk		spi0_clk
#define spi1_pclk		spi1_clk
#define spi2_pclk		spi2_clk
#define spi3_pclk		spi3_clk
#define spi4_pclk		spi4_clk
#define i2c0_pclk		i2c0_clk
#define i2c1_pclk		i2c1_clk
#define i2c2_pclk		i2c2_clk
#define i2c3_pclk		i2c3_clk
#define i2c4_pclk		i2c4_clk
#define i2c5_pclk		i2c5_clk
#define i2c6_pclk		i2c6_clk
#define i2c7_pclk		i2c7_clk
#define i2c8_pclk		i2c8_clk
#define i2c9_pclk		i2c9_clk
/* Alias for overall clocks */
#define sd_clk			sd_hclk
#define gpio0_clk		gpio0_pclk
#define gpio1_clk		gpio1_pclk
#define gpio2_clk		gpio2_pclk
/* TODO: dependency clocks:
 * sd_tm_clk: depends sd_clk, sources soc_pll_div10/100
 */

/* CLK_EN_CFG2 */
/* Integrated overall clocks */
#define timer0_1_clk		clkid(CLK_OUTPUT, TIMER0_1_CLK)
#define timer0_2_clk		clkid(CLK_OUTPUT, TIMER0_2_CLK)
#define timer0_3_clk		clkid(CLK_OUTPUT, TIMER0_3_CLK)
#define timer0_4_clk		clkid(CLK_OUTPUT, TIMER0_4_CLK)
#define timer0_5_clk		clkid(CLK_OUTPUT, TIMER0_5_CLK)
#define timer0_6_clk		clkid(CLK_OUTPUT, TIMER0_6_CLK)
#define timer0_7_clk		clkid(CLK_OUTPUT, TIMER0_7_CLK)
#define timer0_8_clk		clkid(CLK_OUTPUT, TIMER0_8_CLK)
#define timer1_1_clk		clkid(CLK_OUTPUT, TIMER1_1_CLK)
#define timer1_2_clk		clkid(CLK_OUTPUT, TIMER1_2_CLK)
#define timer1_3_clk		clkid(CLK_OUTPUT, TIMER1_3_CLK)
#define timer1_4_clk		clkid(CLK_OUTPUT, TIMER1_4_CLK)
#define timer1_5_clk		clkid(CLK_OUTPUT, TIMER1_5_CLK)
#define timer1_6_clk		clkid(CLK_OUTPUT, TIMER1_6_CLK)
#define timer1_7_clk		clkid(CLK_OUTPUT, TIMER1_7_CLK)
#define timer1_8_clk		clkid(CLK_OUTPUT, TIMER1_8_CLK)
#define timer2_1_clk		clkid(CLK_OUTPUT, TIMER2_1_CLK)
#define timer2_2_clk		clkid(CLK_OUTPUT, TIMER2_2_CLK)
#define timer2_3_clk		clkid(CLK_OUTPUT, TIMER2_3_CLK)
#define timer2_4_clk		clkid(CLK_OUTPUT, TIMER2_4_CLK)
#define timer2_5_clk		clkid(CLK_OUTPUT, TIMER2_5_CLK)
#define timer2_6_clk		clkid(CLK_OUTPUT, TIMER2_6_CLK)
#define timer2_7_clk		clkid(CLK_OUTPUT, TIMER2_7_CLK)
#define timer2_8_clk		clkid(CLK_OUTPUT, TIMER2_8_CLK)
#define timer3_clk		clkid(CLK_OUTPUT, TIMER3_CLK)
#define tsensor0_clk		clkid(CLK_OUTPUT, TSENSOR0_CLK)
#define tsensor1_clk		clkid(CLK_OUTPUT, TSENSOR1_CLK)
#define tsensor2_clk		clkid(CLK_OUTPUT, TSENSOR2_CLK)
#define tsensor3_clk		clkid(CLK_OUTPUT, TSENSOR3_CLK)
/* TODO: dependency clocks:
 * tsensor0_pclk: depends tsensor0_clk, sources sysfab_half_clk
 * tsensor1_pclk: depends tsensor1_clk, sources sysfab_half_clk
 * tsensor2_pclk: depends tsensor2_clk, sources sysfab_half_clk
 * tsensor3_pclk: depends tsensor3_clk, sources sysfab_half_clk
 */

#define CLK_DIV			((clk_cat_t)4)
#define SYSFAB_CLK		((clk_clk_t)0)
#define SYSFAB_HALF_CLK		((clk_clk_t)1)
#define SOC_PLL_DIV10		((clk_clk_t)2)
#define SD_TM_CLK		((clk_clk_t)3)
#define DDR_PLL_DIV4		((clk_clk_t)4)
#define XO_CLK_DIV4		((clk_clk_t)5)
#define NR_DIV_CLKS		(XO_CLK_DIV4 + 1)
#define sysfab_clk		clkid(CLK_DIV, SYSFAB_CLK)
#define sysfab_half_clk		clkid(CLK_DIV, SYSFAB_HALF_CLK)
#define soc_pll_div10		clkid(CLK_DIV, SOC_PLL_DIV10)
#define sd_tm_clk		clkid(CLK_DIV, SD_TM_CLK)
#define ddr_pll_div4		clkid(CLK_DIV, DDR_PLL_DIV4)
#define xo_clk_div4		clkid(CLK_DIV, XO_CLK_DIV4)

/* Enable clock tree core */
void clk_hw_ctrl_init(void);
#ifdef CONFIG_MMU
#define clk_hw_mmu_init()	do { } while (0)
#endif

#endif /* __CLK_DUOWEN_H_INCLUDE__ */
