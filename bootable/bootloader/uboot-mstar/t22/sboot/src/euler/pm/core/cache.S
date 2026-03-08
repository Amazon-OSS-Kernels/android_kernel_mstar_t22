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
#include "./inc/risc32.inc"
#include "./inc/risc32_spr.h"

    .extern _RISC32_DCACHE_SIZE
    .extern _RISC32_DCACHE_WAYS
    .extern _RISC32_DCACHE_LINE_SIZE

.section ".text", "ax"

ENTRY(mhal_dcache_flush)
    l.movhi r6,hi(_RISC32_DCACHE_SIZE)
    l.addi  r6,r6,lo(_RISC32_DCACHE_SIZE)
    l.movhi r7,hi(_RISC32_DCACHE_WAYS)
    l.addi  r7,r7,lo(_RISC32_DCACHE_WAYS)
    l.movhi r5,hi(_RISC32_DCACHE_LINE_SIZE)
    l.addi  r5,r5,lo(_RISC32_DCACHE_LINE_SIZE)

    ## (RISC32_DCACHE_SIZE / RISC32_DCACHE_WAYS - RISC32_DCACHE_LINE_SIZE)
    l.divu  r6,r6,r7
    l.sub   r6,r6,r5

    l.sfleu r4,r6
#ifdef __AEONR2__
    l.add   r4,r3,r4
    l.bf    disable_interrupt
#else
    l.bf    disable_interrupt
    l.add   r4,r3,r4
#endif
    l.add   r4,r3,r6

disable_interrupt:
    l.addi  r7,r0,SPR_SR
    l.mfspr r23,r7,0        ## save SR in r23
    l.addi  r6,r0,-7        ## ~(SPR_SR_IEE | SPR_SR_TEE)
    l.and   r6,r23,r6
    l.mtspr r7,r6,0

flush_line:
    l.flush_line       0(r3), 0
    l.flush_line       0(r3), 1
    l.invalidate_line  0(r3), 0
    l.invalidate_line  0(r3), 1

    l.sfltu r3,r4
#ifdef __AEONR2__
    l.add   r3,r3,r5
    l.bf    flush_line
#else
    l.bf    flush_line
    l.add   r3,r3,r5
#endif

#ifdef __AEONR2__
    l.syncwritebuffer

    l.mtspr r7,r23,0        ## restore interrupt mask
    l.jr    r9
#else
    l.syncwritebuffer
    l.jr    r9
    l.mtspr r7,r23,0        ## restore interrupt mask
#endif
