/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2020
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
 * @(#)uart.c: DUOWEN machine specific UART implementation
 * $Id: uart.c,v 1.1 2020-07-01 10:32:00 zhenglv Exp $
 */

#include <target/console.h>
#include <target/paging.h>

void uart_hw_con_init(void)
{
	gpio_config_mux(UART_CON_PORT_IO, UART_CON_PIN_SIN,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_IO, UART_CON_PIN_SOUT,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_MODEM, UART_CON_PIN_CTS,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_MODEM, UART_CON_PIN_RTS,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_MODEM, UART_CON_PIN_DSR,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_MODEM, UART_CON_PIN_DTR,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_MODEM, UART_CON_PIN_RI,
			TLMM_GROUP_FUNCTION);
	gpio_config_mux(UART_CON_PORT_MODEM, UART_CON_PIN_DCD,
			TLMM_GROUP_FUNCTION);

	board_init_clock();
	clk_enable(UART_CLK_ID);
	dw_uart_con_init(clk_get_frequency(UART_CLK_ID));
}

#ifdef CONFIG_MMU
void uart_hw_mmu_init(void)
{
	duowen_mmu_map_uart(UART_CON_ID);
	uart_hw_con_init();
	duowen_mmu_dump_maps();
}
#endif
