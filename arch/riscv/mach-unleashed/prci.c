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
 * @(#)prci.c: FU540 (unleashed) power reset clocking interrupt (PRCI) driver
 * $Id: prci.c,v 1.1 2019-10-16 15:43:00 zhenglv Exp $
 */

#include <target/clk.h>

void pll_config_freq(uint8_t pll, uint32_t output_freq)
{
	uint32_t cfg = (PLL_RANGE(PLL_RANGE_33MHZ) | PLL_FSE);
	uint32_t f = div32u(output_freq, FREQ_33MHZ);
	uint8_t q = 1;

	if (f < 2) {
		q += 5;
		f <<= 5;
	} else if (f < 4) {
		q += 4;
		f <<= 4;
	} else if (f < 8) {
		q += 3;
		f <<= 3;
	} else if (f < 16) {
		q += 2;
		f <<= 2;
	} else if (f < 32) {
		q += 1;
		f <<= 1;
	}
	f -= 1;
	cfg |= (PLL_DIVR(0) | PLL_DIVF(f) | PLL_DIVQ(q));
	__raw_writel(cfg, PRCI_PLLCFG(pll));
	while (!pll_config_locked(pll));
	pll_output_enable(pll);
}
