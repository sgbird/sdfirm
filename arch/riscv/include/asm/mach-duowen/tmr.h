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
 * @(#)tmr.h: DUOWEN specific timer controller (TMR) driver
 * $Id: tmr.h,v 1.1 2019-11-21 12:32:00 zhenglv Exp $
 */

#ifndef __TMR_DUOWEN_H_INCLUDE__
#define __TMR_DUOWEN_H_INCLUDE__

#define TMR_BASE		TIMER3_BASE
#define TMR_REG(offset)		(TMR_BASE + (offset))

#define TMR_CNT_CTRL		TMR_REG(0x000)
#define TMR_CNT_LO		TMR_REG(0x040)
#define TMR_CNT_HI		TMR_REG(0x044)
#define TMR_WDT_CFG(n)		TMR_REG(0x100 + ((n) << 5))
#define TMR_WDT_PERIOD(n)	TMR_REG(0x104 + ((n) << 5))
#define TMR_WDT_CMP_LO(n)	TMR_REG(0x108 + ((n) << 5))
#define TMR_WDT_CMP_HI(n)	TMR_REG(0x10C + ((n) << 5))
#define TMR_WDT_DN_CNT(n)	TMR_REG(0x110 + ((n) << 5))
#define TMR_CFG			TMR_REG(0x200)
#define TMR_TVAL		TMR_REG(0x204)
#define TMR_CMP_LO		TMR_REG(0x208)
#define TMR_CMP_HI		TMR_REG(0x20C)
#define TMR_INTR_STATUS		TMR_REG(0x210)
#define TMR_INTR_CLR		TMR_REG(0x214)

/* TMR_CNT_CTRL */
#define TMR_MTIME_EN		_BV(0)
#define TMR_HALT_ON_DEBUG	_BV(1)

/* WDT_CFG_n */
#define TMR_WDT_EN		_BV(0)

/* TMR_CFG */
#define TMR_EN			_BV(0)
#define TMR_INTR_EN		_BV(1)

/* TMR_INTR_STATUS
 * TMR_INTR_CLR
 */
#define TMR_INTR		_BV(0)

#define tmr_enable_cnt()	__raw_setl(TMR_MTIME_EN, TMR_CNT_CTRL)
#define tmr_disable_cnt()	__raw_clearl(TMR_MTIME_EN, TMR_CNT_CTRL)
#define tmr_enable_cmp()	__raw_setl(TMR_EN, TMR_CFG)
#define tmr_disable_cmp()	__raw_clearl(TMR_EN, TMR_CFG)
#define tmr_enable_irq()	__raw_setl(TMR_INTR_EN, TMR_CFG)
#define tmr_disable_irq()	__raw_clearl(TMR_INTR_EN, TMR_CFG)
#define tmr_irq_status()	(__raw_readl(TMR_INTR_STATUS) & TMR_INTR)
#define tmr_irq_clear()		__raw_setl(TMR_INTR, TMR_INTR_CLR)

#ifndef __ASSEMBLY__
#ifdef CONFIG_DUOWEN_TMR
uint64_t tmr_read_counter(void);
void tmr_write_compare(uint64_t count);
void tmr_ctrl_init(void);
#else
#define tmr_read_counter()		0
#define tmr_write_compare(count)	do { } while (0)
#define tmr_ctrl_init()			do { } while (0)
#endif
#endif

#endif /* __TMR_DUOWEN_H_INCLUDE__ */
