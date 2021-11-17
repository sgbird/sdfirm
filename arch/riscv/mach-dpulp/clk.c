/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2021
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
 * @(#)clk.c: DPU-LP CRU clock framework implementation
 * $Id: clk.c,v 1.1 2021-11-16 11:09:00 zhenglv Exp $
 */

#include <target/clk.h>
#include <target/ddr.h>

struct pll_clk {
	clk_t src;
	clk_t mux;
	uint32_t alt;
	clk_freq_t freq;
	bool enabled;
};

struct pll_clk pll_clks[NR_PLL_CLKS] = {
	[CPU_PLL] = {
		.src = cpu_vco,
		.mux = cpu_clksel,
		.alt = 0,
		.freq = CPU_CLK_FREQ,
		.enabled = false,
	},
	[GPDPU_BUS_PLL] = {
		.src = gpdpu_bus_vco,
		.mux = gpdpu_bus_clksel,
		.alt = 0,
		.freq = GPDPU_BUS_CLK_FREQ,
		.enabled = false,
	},
	[GPDPU_CORE_PLL] = {
		.src = gpdpu_core_vco,
		.mux = gpdpu_core_clksel,
		.alt = 0,
		.freq = GPDPU_CORE_CLK_FREQ,
		.enabled = false,
	},
	[DDR_PLL] = {
		.src = ddr_vco,
		.mux = ddr_clksel,
		.alt = 0,
		.freq = DDR_CLK_FREQ,
		.enabled = false,
	},
	[AXI_PLL] = {
		.src = soc_vco,
		.mux = soc_800_clksel,
		.alt = 0,
		.freq = AXI_CLK_FREQ,
		.enabled = false,
	},
	[VPU_B_PLL] = {
		.src = vpu_b_vco,
		.mux = vpu_bclksel,
		.alt = 0,
		.freq = VPU_BCLK_FREQ,
		.enabled = false,
	},
	[VPU_C_PLL] = {
		.src = vpu_c_vco,
		.mux = vpu_cclksel,
		.alt = 0,
		.freq = VPU_CCLK_FREQ,
		.enabled = false,
	},
	[RAB0_PHY_PLL] = {
		.src = rab0_phy_vco,
		.mux = rab0_phy_clksel,
		.alt = 0,
		.freq = RAB0_PHY_CLK_FREQ,
		.enabled = false,
	},
	[RAB1_PHY_PLL] = {
		.src = rab1_phy_vco,
		.mux = rab1_phy_clksel,
		.alt = 0,
		.freq = RAB1_PHY_CLK_FREQ,
		.enabled = false,
	},
	[ETH0_PHY_PLL] = {
		.src = eth0_phy_vco,
		.mux = invalid_clk,
		.alt = 0,
		.freq = ETH0_PHY_CLK_FREQ,
		.enabled = false,
	},
	[ETH1_PHY_PLL] = {
		.src = eth1_phy_vco,
		.mux = invalid_clk,
		.alt = 0,
		.freq = ETH1_PHY_CLK_FREQ,
		.enabled = false,
	},
	[APB_PLL] = {
		.src = soc_vco,
		.mux = soc_100_clksel,
		.alt = 0,
		.freq = APB_CLK_FREQ,
		.enabled = false,
	},
};

#ifdef CONFIG_CLK_MNEMONICS
const char *pll_clk_names[NR_PLL_CLKS] = {
	[CPU_PLL] = "cpu_pll",
	[GPDPU_BUS_PLL] = "gpdpu_bus_pll",
	[GPDPU_CORE_PLL] = "gpdpu_core_pll",
	[DDR_PLL] = "ddr_pll",
	[AXI_PLL] = "axi_pll",
	[VPU_B_PLL] = "vpu_b_pll",
	[VPU_C_PLL] = "vpu_c_pll",
	[RAB0_PHY_PLL] = "rab0_phy_pll",
	[RAB1_PHY_PLL] = "rab1_phy_pll",
	[ETH0_PHY_PLL] = "eth0_phy_pll",
	[ETH1_PHY_PLL] = "eth1_phy_pll",
	[APB_PLL] = "apb_pll",
};

const char *get_pll_name(clk_clk_t clk)
{
	if (clk >= NR_PLL_CLKS)
		return NULL;
	return pll_clk_names[clk];
}
#else
#define get_pll_name		NULL
#endif

static void __enable_pll(clk_clk_t pll, clk_clk_t clk, bool force)
{
	bool r = !!(clk >= DPULP_MAX_PLLS);
	clk_clk_t vco = r ? clk - DPULP_MAX_PLLS : clk;
	uint32_t clk_sels = 0;
	uint32_t alt;
	clk_freq_t fvco_orig, fvco, fclk;

	if (!pll_clks[pll].enabled || force) {
		cru_trace(true, get_pll_name(clk));
		fclk = pll_clks[pll].freq;
		alt = pll_clks[pll].alt;
		/* XXX: Protect Dynamic PLL Change
		 *
		 * The PLL might be the source of the system clocks (CPU,
		 * BUS, etc.). Since the possible dynamic PLL change may
		 * disable VCO internally to lead to the unpredictable
		 * system hangs, switching to the xo_clk can help to
		 * ensure a safer P/R clkout enabling.
		 */
		clk_deselect_mux(pll_clks[pll].mux);
		/* XXX: Lowest Power Consumption P/R
		 *
		 * Stay sourcing xo_clk to utilize the low power
		 * consumption mode.
		 */
		if (pll_clks[pll].mux != invalid_clk &&
		    alt == 0 && fclk == XO_CLK_FREQ)
			goto exit_xo_clk;
#if 0
		if (alt) {
			/* XXX: No Cohfab Clock Selection Masking
			 *
			 * To avoid complications, no cohfab clock
			 * selection are masked as alternative masks.
			 */
			clk_sels = crcntl_clk_sel_read();
			crcntl_clk_sel_write(clk_sels | alt);
		}
#endif
		fvco_orig = fvco = clk_get_frequency(pll_clks[pll].src);
		if (clk == DDR_PLL)
			fvco = ddr_clk_fvco(fclk, fvco_orig);
		if (fvco != fvco_orig) {
			clk_apply_vco(vco, clk, fvco);
			clk_disable(pll_clks[pll].src);
		}
		clk_enable(pll_clks[pll].src);
		dpulp_div_enable(vco, fvco, fclk, r);
#if 0
		if (alt)
			crcntl_clk_sel_write(clk_sels);
#endif
		clk_select_mux(pll_clks[pll].mux);
exit_xo_clk:
		pll_clks[pll].enabled = true;
	}
}

static void __disable_pll(clk_clk_t pll, clk_clk_t clk)
{
	bool r = !!(clk >= DPULP_MAX_PLLS);
	clk_clk_t vco = r ? clk - DPULP_MAX_PLLS : clk;
	uint32_t clk_sels = 0;
	uint32_t alt;

	if (pll_clks[pll].enabled) {
		cru_trace(false, get_pll_name(clk));
		alt = pll_clks[pll].alt;
		pll_clks[pll].enabled = false;
		/* XXX: Ensure System Clocking
		 *
		 * The PLL might be the source of the system clocks (CPU,
		 * BUS, etc.). Make sure it's still clocking after
		 * disabling the P/R clkout by switching to the xo_clk.
		 */
		clk_deselect_mux(pll_clks[pll].mux);
#if 0
		if (alt) {
			/* XXX: No Cohfab Clock Selection Masking
			 *
			 * To avoid complications, no cohfab clock
			 * selection are masked as alternative masks.
			 */
			clk_sels = crcntl_clk_sel_read();
			crcntl_clk_sel_write(clk_sels | alt);
		}
#endif
		dpulp_div_disable(vco, r);
		/* XXX: Lowest Power Consumption VCO
		 *
		 * And VCO is also disabled when all related P/R outputs
		 * are deselected (clocking with xo_clk).
		 */
		if ((clk_sels & alt) == alt)
			clk_disable(pll_clks[pll].src);
#if 0
		else if (alt)
			crcntl_clk_sel_write(clk_sels);
#endif
	}
}

static int enable_pll(clk_clk_t clk)
{
	if (clk >= NR_PLL_CLKS)
		return -EINVAL;
	__enable_pll(clk, clk, false);
	return 0;
}

static void disable_pll(clk_clk_t clk)
{
	if (clk >= NR_PLL_CLKS)
		return;
	__disable_pll(clk, clk);
}

static clk_freq_t get_pll_freq(clk_clk_t clk)
{
	if (clk >= NR_PLL_CLKS)
		return INVALID_FREQ;
	return pll_clks[clk].freq;
}

static int set_pll_freq(clk_clk_t clk, clk_freq_t freq)
{
	if (clk >= NR_PLL_CLKS)
		return -EINVAL;

	if (pll_clks[clk].freq != freq) {
		clk_apply_pll(clk, clk, freq);
		__enable_pll(clk, clk, true);
	}
	return 0;
}

struct clk_driver clk_pll = {
	.max_clocks = NR_PLL_CLKS,
	.enable = enable_pll,
	.disable = disable_pll,
	.get_freq = get_pll_freq,
	.set_freq = set_pll_freq,
	.select = NULL,
	.get_name = get_pll_name,
};

struct vco_clk {
	clk_freq_t freq;
	clk_freq_t freq_p;
	clk_freq_t freq_r;
	bool enabled;
};

struct vco_clk vco_clks[NR_VCO_CLKS] = {
	[CPU_VCO] = {
		.freq = CPU_VCO_FREQ,
		.freq_p = CPU_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[GPDPU_BUS_VCO] = {
		.freq = GPDPU_BUS_VCO_FREQ,
		.freq_p = GPDPU_BUS_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[GPDPU_CORE_VCO] = {
		.freq = GPDPU_CORE_VCO_FREQ,
		.freq_p = GPDPU_CORE_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[DDR_VCO] = {
		.freq = DDR_VCO_FREQ,
		.freq_p = DDR_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[SOC_VCO] = {
		.freq = SOC_VCO_FREQ,
		.freq_p = AXI_CLK_FREQ,
		.freq_r = APB_CLK_FREQ,
		.enabled = false,
	},
	[VPU_B_VCO] = {
		.freq = VPU_B_VCO_FREQ,
		.freq_p = VPU_BCLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[VPU_C_VCO] = {
		.freq = VPU_C_VCO_FREQ,
		.freq_p = VPU_CCLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[RAB0_PHY_VCO] = {
		.freq = RAB_PHY_VCO_FREQ,
		.freq_p = RAB0_PHY_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[RAB1_PHY_VCO] = {
		.freq = RAB_PHY_VCO_FREQ,
		.freq_p = RAB1_PHY_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[ETH0_PHY_VCO] = {
		.freq = ETH_PHY_VCO_FREQ,
		.freq_p = ETH0_PHY_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
	[ETH1_PHY_VCO] = {
		.freq = ETH_PHY_VCO_FREQ,
		.freq_p = ETH1_PHY_CLK_FREQ,
		.freq_r = INVALID_FREQ,
		.enabled = false,
	},
};

#ifdef CONFIG_CLK_MNEMONICS
const char *vco_clk_names[NR_VCO_CLKS] = {
	[CPU_VCO] = "cpu_vco",
	[GPDPU_BUS_VCO] = "gpdpu_bus_vco",
	[GPDPU_CORE_VCO] = "gpdpu_core_vco",
	[DDR_VCO] = "ddr_vco",
	[SOC_VCO] = "soc_vco",
	[VPU_B_VCO] = "vpu_b_vco",
	[VPU_C_VCO] = "vpu_c_vco",
	[RAB0_PHY_VCO] = "rab0_phy_vco",
	[RAB1_PHY_VCO] = "rab1_phy_vco",
	[ETH0_PHY_VCO] = "eth0_phy_vco",
	[ETH1_PHY_VCO] = "eth1_phy_vco",
};

const char *get_vco_name(clk_clk_t clk)
{
	if (clk >= NR_VCO_CLKS)
		return NULL;
	return vco_clk_names[clk];
}
#else
#define get_vco_name		NULL
#endif

static void __enable_vco(clk_clk_t vco, clk_clk_t clk)
{
	if (!vco_clks[vco].enabled) {
		cru_trace(true, get_vco_name());
		dpulp_pll_enable2(vco, vco_clks[vco].freq,
				  vco_clks[vco].freq_p,
				  vco_clks[vco].freq_r);
		vco_clks[vco].enabled = true;
	}
}

static void __disable_vco(clk_clk_t vco, clk_clk_t clk)
{
	if (vco_clks[vco].enabled) {
		cru_trace(false, get_vco_name(clk));
		vco_clks[vco].enabled = false;
		dpulp_pll_disable(vco);
	}
}

static int enable_vco(clk_clk_t clk)
{
	if (clk >= NR_VCO_CLKS)
		return -EINVAL;
	__enable_vco(clk, clk);
	return 0;
}

static void disable_vco(clk_clk_t clk)
{
	if (clk >= NR_VCO_CLKS)
		return;
	__disable_vco(clk, clk);
}

static clk_freq_t get_vco_freq(clk_clk_t clk)
{
	if (clk >= NR_VCO_CLKS)
		return INVALID_FREQ;
	return vco_clks[clk].freq;
}

static int set_vco_freq(clk_clk_t clk, clk_freq_t freq)
{
	if (clk >= NR_VCO_CLKS)
		return -EINVAL;

	if (vco_clks[clk].freq != freq) {
		__disable_vco(clk, clk);
		clk_apply_vco(clk, clk, freq);
	}
	__enable_vco(clk, clk);
	return 0;
}

struct clk_driver clk_vco = {
	.max_clocks = NR_VCO_CLKS,
	.enable = enable_vco,
	.disable = disable_vco,
	.get_freq = get_vco_freq,
	.set_freq = set_vco_freq,
	.select = NULL,
	.get_name = get_vco_name,
};

uint32_t input_clks[NR_INPUT_CLKS] = {
	[XO_CLK] = XO_CLK_FREQ,
};

#ifdef CONFIG_CLK_MNEMONICS
const char *input_clk_names[NR_INPUT_CLKS] = {
	[XO_CLK] = "xo_clk",
};

static const char *get_input_clk_name(clk_clk_t clk)
{
	if (clk >= NR_INPUT_CLKS)
		return NULL;
	return input_clk_names[clk];
}
#else
#define get_input_clk_name	NULL
#endif

static clk_freq_t get_input_clk_freq(clk_clk_t clk)
{
	if (clk >= NR_INPUT_CLKS)
		return INVALID_FREQ;
	return input_clks[clk];
}

struct clk_driver clk_input = {
	.max_clocks = NR_INPUT_CLKS,
	.enable = NULL,
	.disable = NULL,
	.get_freq = get_input_clk_freq,
	.set_freq = NULL,
	.select = NULL,
	.get_name = get_input_clk_name,
};

void clk_apply_vco(clk_clk_t vco, clk_clk_t clk, clk_freq_t freq)
{
	if (vco >= NR_VCO_CLKS)
		return;
	vco_clks[vco].freq = freq;
}

void clk_apply_pll(clk_clk_t pll, clk_clk_t clk, clk_freq_t freq)
{
	bool r = !!(clk >= DPULP_MAX_PLLS);
	clk_clk_t vco = r ? clk - DPULP_MAX_PLLS : clk;

	if (pll >= NR_PLL_CLKS || vco >= NR_VCO_CLKS)
		return;
	pll_clks[pll].freq = freq;

	/* XXX: Avoid Dynamic PLL Change
	 *
	 * This API should be used right after clk_apply_vco(). The usage
	 * should only affect static PLL changes. So changes P/R clkouts
	 * presets accordingly to avoid dynamic PLL changes in the follow
	 * up clk_enable() invocations.
	 */
	if (r)
		vco_clks[vco].freq_r = freq;
	else
		vco_clks[vco].freq_p = freq;
}

#ifdef CONFIG_CONSOLE_COMMAND
void clk_pll_dump(void)
{
	int i;
	const char *name;

	for (i = 0; i < NR_PLL_CLKS; i++) {
		name = clk_get_mnemonic(clkid(CLK_PLL, i));
		if (name)
			printf("pll  %3d %20s %20s\n", i, name, "xo_clk");
	}
#if 0
	for (i = 0; i < NR_SEL_CLKS; i++) {
		if (sel_clk_names[i]) {
			printf("clk  %3d %20s %20s\n",
			       i, sel_clk_names[i],
			       clk_get_mnemonic(select_clks[i].clk_sels[0]));
			if (select_clks[i].clk_sels[1] != invalid_clk)
				printf("%4s %3s %20s %20s\n", "", "", "",
				       clk_get_mnemonic(
					       select_clks[i].clk_sels[1]));
		}
	}
#endif
}
#endif

void clk_pll_init(void)
{
	clk_register_driver(CLK_INPUT, &clk_input);
	clk_register_driver(CLK_VCO, &clk_vco);
	clk_register_driver(CLK_PLL, &clk_pll);
/*	clk_register_driver(CLK_SELECT, &clk_select); */
}

void board_init_clock(void)
{
}