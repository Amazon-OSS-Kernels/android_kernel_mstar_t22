/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#ifndef __RISC32_UTIL_H__
#define __RISC32_UTIL_H__

#include "datatype.h"
#include "risc32_spr.h"

#if defined (__mips__)

extern U32 hal_lsbit_index(U32 mask);
extern U32 hal_msbit_index(U32 mask);

#define __mhal_lsbit_index(_value_) hal_lsbit_index(_value_)
#define __mhal_msbit_index(_value_) hal_msbit_index(_value_)

extern U32 hal_disable_interrupts(void);
#define __mhal_interrupt_disable(_old_) { (_old_) = hal_disable_interrupts(); }

extern void hal_enable_interrupts(void);
#define __mhal_interrupt_enable() hal_enable_interrupts()

extern void hal_restore_interrupts( U32 old );
#define __mhal_interrupt_restore(_old_) hal_restore_interrupts( _old_ )

#define ASM_CLEAR_EHB   "nop;nop;nop;\n"

#define __mhal_interrupt_mask( _vector_ )       \
MST_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        ASM_CLEAR_EHB                           \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
MST_MACRO_END

#define __mhal_interrupt_unmask( _vector_ )     \
MST_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "or     $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        ASM_CLEAR_EHB                           \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
MST_MACRO_END


#else


#define __mhal_lsbit_index(_value_)                                         \
    ({                                                                      \
    unsigned long _index_;                                                  \
    __asm__ __volatile__ ("l.ff1\t\t%0,%1" : "=r" (_index_) : "r" (_value_));\
    _index_;                                                                \
    })

#define __mhal_msbit_index(_value_)                                         \
    ({                                                                      \
    unsigned long _index_;                                                  \
    __asm__ __volatile__ ("l.fl1\t\t%0,%1" : "=r" (_index_) : "r" (_value_));\
    _index_;                                                                \
    })

#define __mhal_interrupt_disable(_old_)                                     \
    MST_MACRO_START                                                         \
    _old_ = mfspr(SPR_SR);                                                  \
    mtspr(SPR_SR, (_old_) & ~(SPR_SR_IEE | SPR_SR_TEE));                    \
    MST_MACRO_END

#define __mhal_interrupt_enable()                                           \
    mtspr(SPR_SR, (SPR_SR_IEE|SPR_SR_TEE) | mfspr(SPR_SR))

#define __mhal_interrupt_restore(_old_)                                     \
    mtspr(SPR_SR, (~(SPR_SR_IEE|SPR_SR_TEE) & mfspr(SPR_SR) ) |             \
                  ( (SPR_SR_IEE|SPR_SR_TEE) & (_old_) ))

#define __mhal_interrupt_mask(_intr_)                                       \
    MST_MACRO_START                                                         \
    if (_intr_ != MHAL_INTERRUPT_TICK_TIMER)                                \
    {                                                                       \
        mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(1 << (_intr_ - 1)));          \
    }                                                                       \
    MST_MACRO_END

#define __mhal_interrupt_unmask(_intr_)                                     \
    MST_MACRO_START                                                         \
    if (_intr_ != MHAL_INTERRUPT_TICK_TIMER)                                \
    {                                                                       \
        mtspr(SPR_PICMR, mfspr(SPR_PICMR) | (1 << (_intr_ - 1)));           \
    }                                                                       \
    MST_MACRO_END

#define __mhal_mmu_disable(_old_)                                     \
        MST_MACRO_START                                                         \
        _old_ = mfspr(SPR_SR);                                      \
        mtspr(SPR_SR, (_old_) & ~(SPR_SR_IME | SPR_SR_DME));                    \
        MST_MACRO_END

#endif

#endif /* __RISC32_UTIL_H__ */
