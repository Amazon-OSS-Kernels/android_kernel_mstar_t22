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

#ifndef _BOOT_INC_
#include "boot.inc"
#endif

@//-------------------------------------------------------------------------------------------------
@// BOOTRAM_Entry_Hash:
@// @param  None
@// @return None
@// @note   Entry point of Boot code
@//-------------------------------------------------------------------------------------------------
    .globl      BOOTRAM_Entry_Hash

BOOTRAM_Entry_Hash:

    RREG        r0, (MS_BUS_RIU_ADDR + (0x103380 << 1))
    ldr         r1, =0x0100
    and         r0, r0, #0xff00
    cmp         r0, r1
    beq         BootRam_AuthenticationHash3_bk     @//reg[0x103381]#0=1b, MBOOT partition authen failed, try MBOOTBAK

BootRam_AuthenticationHash3:
    PUTK 'A'
    ldr         r0, =CONFIG_BOOTRAM_LOADADDR
    ldr         r1, =_ld_bootram2_size
    ldr         r2, =0x1002                                      @//Read Hash3 at MBOOT Partition, addr=0x200400 (0x1002<<9)
    bl          eMMC_ReadData_MIU

    ldr         r0, =CONFIG_BOOTRAM_LOADADDR                     @//Set the starting address of HASH for Authentication
    ldr         r1, =(_ld_bootram2_size - 0x400)                                      @//Set the HASH size for Authentication
    ldr         r2, =CONFIG_BOOTRAM_LOADADDR + _ld_bootram2_size - 0x400            @//Set the starting address of SIG for Authentication
    ldr         r3, =0x0
    bl          MDrv_AESDMA_SecureMain

    cmp         r0, #1
    beq         BootRam_Entry_Point
    WREG_B      (MS_BUS_RIU_ADDR + (0x103380 << 1) + 1), 0x01    @//Set Hash3 of MBOOT partition authen fail flag, try MBOOTBAK

BootRam_AuthenticationHash3_bk:
    PUTK 'B'
    ldr         r0, =CONFIG_BOOTRAM_LOADADDR
    ldr         r1, =_ld_bootram2_size
    ldr         r2, =0x8002                                      @//Read Hash3_bak at MBOOTBAK Partition, addr=0x1000400 (0x8002<<9)
    bl          eMMC_ReadData_MIU

    ldr         r0, =CONFIG_BOOTRAM_LOADADDR                     @//Set the starting address of HASH for Authentication
    ldr         r1, =(_ld_bootram2_size - 0x400)                                      @//Set the HASH size for Authentication
    ldr         r2, =CONFIG_BOOTRAM_LOADADDR + _ld_bootram2_size - 0x400            @//Set the starting address of SIG for Authentication
    ldr         r3, =0x0
    bl          MDrv_AESDMA_SecureMain

    cmp         r0, #1
    beq         BootRam_Entry_Point
    WREG_B      (MS_BUS_RIU_ADDR + (0x103380 << 1) + 1), 0x03    @//Set MBOOT & MBOOTBAK partition both authen fail flag
    bl          BootRam_AuthenticationFail                       @//HASH3 & HASH3_bak both authen failed

BootRam_Entry_Point:
    @//Jump to the entry of BOOTRAM_Entry
    ldr         pc, =BOOTRAM_Entry

BootRam_AuthenticationFail:
    PUTK        '\n'
    PUTK        '\r'
    PUTK       'R'
    PUTK       'A'
    PUTK       'M'
    PUTK       '_'
    PUTK       'A'
    PUTK       'U'
    PUTK       'T'
    PUTK       'H'
    PUTK       '_'
    PUTK       'F'
    PUTK       'A'
    PUTK       'I'
    PUTK       'L'
BootRam_AuthenticationFailLoop:
    b           BootRam_AuthenticationFailLoop
