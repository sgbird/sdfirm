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
 * @(#)spinlock.h: SMP spin lock interfaces
 * $Id: spinlock.h,v 1.0 2019-12-18 15:49:00 zhenglv Exp $
 */

#ifndef __SPINLOCK_H_INCLUDE__
#define __SPINLOCK_H_INCLUDE__

#include <target/generic.h>
#include <target/irq.h>
#include <target/barrier.h>

#ifdef CONFIG_SMP
#include <asm/spinlock.h>
#else
typedef uint8_t spinlock_t;

#define DEFINE_SPINLOCK(lock)		spinlock_t lock = 0

#define smp_hw_spin_init(lock)		(*(lock) = 0)
#define smp_hw_spin_locked(lock)		\
	((void)(lock), 0)
#define smp_hw_spin_lock(lock)			\
	do { barrier(); (void)(lock); } while (0)
#define smp_hw_spin_unlock(lock)		\
	do { barrier(); (void)(lock); } while (0)
#define smp_hw_spin_trylock(lock)		\
	({ barrier(); (void)(lock); 1; })
#endif

#define spin_lock_init(lock)	smp_hw_spin_init(lock)
#define spin_locked(lock)	smp_hw_spin_locked(lock)
#define spin_lock(lock)		smp_hw_spin_lock(lock)
#define spin_unlock(lock)	smp_hw_spin_unlock(lock)
#define spin_trylock(lock)	smp_hw_spin_trylock(lock)
#define spin_lock_irqsave(lock, flags)		\
	do {					\
		irq_local_save(flags);		\
		smp_hw_spin_lock(lock);		\
	} while (0)
#define spin_unlock_irqrestore(lock, flags)	\
	do {					\
		smp_hw_spin_unlock(lock);	\
		irq_local_restore(flags);	\
	} while (0)

#endif /* __SPINLOCK_H_INCLUDE__ */
