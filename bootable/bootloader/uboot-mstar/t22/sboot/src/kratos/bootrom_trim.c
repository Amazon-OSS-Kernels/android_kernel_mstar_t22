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
#include "drv_miu_init.h"

typedef unsigned short MS_U16;

#define BK_EFUSE(x) *(volatile MS_U16 *)(0xBF000000+(0x002000<<1)+((x)<<2))
#define BK_PM_SLP(x) *(volatile MS_U16 *)(0xBF000000+(0x000E00<<1)+((x)<<2))
#define BK_MIU_ATOP(x) *(volatile MS_U16 *)(0xBF000000+(0x110D00<<1)+((x)<<2))

/* avoid for compile warning */
void BootRomPreTrimSequence(void);
void BootRomPostTrimSequence(void);

void BootRomPreTrimSequence(void)
{
    MS_U16 u16EfVal;
    MS_U16 u16RegVal;

    /* PM_SAR_ATOP */
    //set reg_1x128_addr[3] to 1'b0
    BK_EFUSE(0x01) &= 0xFFF7;

    u16EfVal = BK_EFUSE(0x1D);
    //PM_vol_bgap
    u16RegVal = BK_PM_SLP(0x19);
    u16RegVal &= 0x00FF;
    u16RegVal |= ((u16EfVal>>2) & 0x3F00);
    BK_PM_SLP(0x19) = u16RegVal;

    //CLK_4M
    u16RegVal = BK_PM_SLP(0x30);
    u16RegVal &= 0xE1FF;
    u16RegVal |= ((u16EfVal<<4) & 0x1E00);
    BK_PM_SLP(0x30) = u16RegVal;
}

void BootRomPostTrimSequence(void)
{
    MS_U16 u16EfVal;
    MS_U16 u16RegVal;

    /* DDR_ATOP - MIU_zq */
    /* check Activateefuse flag */
    u16EfVal = BK_EFUSE(0x19);
    if(u16EfVal & 0x0040)
    {
        u16RegVal = BK_MIU_ATOP(0x3D);
        u16RegVal &= 0x00FF;
        u16RegVal |= ((u16EfVal<<9) & 0x7E00);

        u16EfVal = BK_EFUSE(0x18);
        u16RegVal |= ((u16EfVal & 0x8000)?(0x0100):(0x0000));
        
        BK_MIU_ATOP(0x3D) = u16RegVal;
    }
}

