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


#ifdef CONFIG_MSTAR_KRATOS_BD_FPGA

#include "boot.inc"

#ifndef _ArchDefs_h_
#include "ArchDefs.h"
#endif

#ifndef _BOARD_H_
#include "Board.h"
#endif

#ifndef _DRV_MIU_INIT_H_
#include "drv_miu_init.h"
#endif

#ifndef _MIUSEL_KRATOS_2MIU_H_
#include "MIUSEL_KRATOS_2MIU.h"
#endif

#define MIU_AUTO_DRV_N_TRG_LVL              0
#define MIU_TRIG_LVL                        0
#define MIU_AUTO_DRV                        0
#define MIU_AUTO_SKEW                       0
#define MIU_AUTO_DDL                        0
#define MIU_AUTO_DQS                        0

    .text
	.set    	noreorder

    .global     BootInitMiu
    .ent        BootInitMiu

BootInitMiu:

    move        t7, ra

    DBGREG      0x0020

    #//================================================
    #// Start MIU init !!!
    #//================================================

    # swch 3
    WREG (REG_ADDR_BASE + (0x002BC6 << 1)), 0x3c11
    WREG (REG_ADDR_BASE + (0x002BCA << 1)), 0x403c
    WREG (REG_ADDR_BASE + (0x002BC4 << 1)), 0x0004

    # swch 4
    WREG (REG_ADDR_BASE + (0x101202 << 1)), 0x0858
    WREG (REG_ADDR_BASE + (0x101204 << 1)), 0x0185
    WREG (REG_ADDR_BASE + (0x101206 << 1)), 0x0120
    WREG (REG_ADDR_BASE + (0x101210 << 1)), 0x0031
    WREG (REG_ADDR_BASE + (0x101212 << 1)), 0x0000
    WREG (REG_ADDR_BASE + (0x10123e << 1)), 0x0001
    WREG (REG_ADDR_BASE + (0x10121e << 1)), 0x0c01
    WREG (REG_ADDR_BASE + (0x10121e << 1)), 0x0c08

    WREG (REG_ADDR_BASE + (0x101200 << 1)), 0x0008
    WREG (REG_ADDR_BASE + (0x101200 << 1)), 0x000c
    WREG (REG_ADDR_BASE + (0x101200 << 1)), 0x000e
    WREG (REG_ADDR_BASE + (0x101200 << 1)), 0x000f

_CheckDramData:
    lui         t0, 0xBF20
    ori         t0, t0,0x2400
    lw          t1, 0(t0)
    and         t1, t1, 0x8000
    beqz        t1, _CheckDramData
    nop

    #//================================================
    #// End of MIU init !!!             
    #//================================================

    move        ra, t7
    jr          ra
    nop

    .size       BootInitMiu,.-BootInitMiu
    .end        BootInitMiu

#endif

