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

#include "datatype.h"
#include "risc32_spr.h"
#include "risc32.h"

extern U32 RISC32_DCACHE_SIZE;      // Size of data cache in bytes
extern U32 RISC32_DCACHE_LINE_SIZE; //Bytes in a data cache line
extern U32 RISC32_DCACHE_WAYS;      // Associativity of the cache

#define RISC32_DISABLE_INTERRUPTS(_old_)                    \
    MST_MACRO_START                                         \
    _old_ = mfspr(SPR_SR);                                  \
    mtspr(SPR_SR, _old_ & ~(SPR_SR_IEE|SPR_SR_TEE));        \
    MST_MACRO_END

// Copy interrupt flags from argument into Supervisor Register
#define RISC32_RESTORE_INTERRUPTS(_old_)                    \
    MST_MACRO_START                                         \
    U32 t1,t2;                                              \
    t1 = mfspr(SPR_SR) & ~(SPR_SR_IEE|SPR_SR_TEE);          \
    t2 = (_old_) & (SPR_SR_IEE|SPR_SR_TEE);                 \
    mtspr(SPR_SR, t1 | t2);                                 \
    MST_MACRO_END

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
// Aeon2 does not have an explicit invalidate memory instruction, so use
// flush are necessary.
#if 1   // To fix compile warning and need to check after, 2009/11/23
void RISC32_DCACHE_FLUSH(U32 u32Base, U32 u32Size);
#endif

void RISC32_DCACHE_FLUSH(U32 u32Base, U32 u32Size)
{
    U32 u32SizeOnce = u32Size, u32BaseOnce = u32Base;
    U32 u32Addr, u32End, u32Inten;
    u32End = u32BaseOnce + ((u32SizeOnce < ((U32)(&RISC32_DCACHE_SIZE))) ?
                            u32SizeOnce : ((U32)(&RISC32_DCACHE_SIZE)));
    for(u32Addr = u32End; u32Addr >= u32BaseOnce; u32Addr -= (U32)(&RISC32_DCACHE_LINE_SIZE))
    {
        RISC32_DISABLE_INTERRUPTS(u32Inten);
        __asm__ __volatile__(
            "l.flush_line       0(%0), 0\n" /* way 0 */
            "l.flush_line       0(%0), 1\n" /* way 1 */
            "l.invalidate_line  0(%0), 0\n" /* way 0 */
            "l.invalidate_line  0(%0), 1\n" /* way 1 */
            : : "r"(u32Addr)
            );
        RISC32_RESTORE_INTERRUPTS(u32Inten);
    }
    __asm__ __volatile__( "l.syncwritebuffer\n" );
}
