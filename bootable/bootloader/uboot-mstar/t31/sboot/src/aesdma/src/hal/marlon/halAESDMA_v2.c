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


#ifndef _HAL_AESDMA_H_
#include "halAESDMA.h"
#endif

void HAL_SHA_SetByPassTable(U8 bEnable)
{
    if (bEnable == TRUE)
    {
        //Enable SHA ByPass Table, 0x8[11] = 1
        RIU[(AESDMA_BASE_ADDR+(0x08<<1))] |= 0x800;
    }
    else
    {
        //Enable SHA ByPass Table, 0x8[11] = 1
        RIU[(AESDMA_BASE_ADDR+(0x08<<1))] &= ~(0x800);
    }
}

void HAL_SHA_SetAddress(U32 u32Addr)
{
    RIU[(AESDMA_BASE_ADDR+(0x0A<<1))] = (U16)((0x0000ffff)&(u32Addr));
    RIU[(AESDMA_BASE_ADDR+(0x0B<<1))] = (U16)((((0xffff0000)&(u32Addr)) - CONFIG_MIU0_BUSADDR)>>16);
}

void HAL_SHA_Start(void)
{
    //Enable SHA ByPass Table, 0x8[0] = 1
    RIU[(AESDMA_BASE_ADDR+(0x08<<1))] &= ~(0x0001);
    RIU[(AESDMA_BASE_ADDR+(0x08<<1))] |= 0x0001;
}

void HAL_AESDMA_Start_FileOutEn(U8 u8AESDMAStart)
{
    // AESDMA file start
    if(u8AESDMAStart==1)
    {
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])&(~(AESDMA_CTRL_FILE_ST|AESDMA_CTRL_FOUT_EN)));
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])|(AESDMA_CTRL_FILE_ST)|(AESDMA_CTRL_FOUT_EN));
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])&(~(AESDMA_CTRL_FILE_ST|AESDMA_CTRL_FOUT_EN)));
    }
}

void HAL_SHA_Clear_Reset(void)
{
    //SHA_Reset
    RIU[SHARNG_BASE_ADDR+(0x08<<1)]= 0x0;
    RIU[SHARNG_BASE_ADDR+(0x08<<1)]= (RIU[SHARNG_BASE_ADDR+(0x08<<1)]|(SHARNG_CTRL_SHA_RST));
    RIU[SHARNG_BASE_ADDR+(0x08<<1)]= (RIU[SHARNG_BASE_ADDR+(0x08<<1)]&(~SHARNG_CTRL_SHA_RST));
}

void HAL_AESDMA_DisableHwKey(void)
{
    // disable normal bank to use efuse key
    RIU[(DMA_SECURE_BASE_ADDR+(0x01<<1))]= ((RIU[(DMA_SECURE_BASE_ADDR+(0x01<<1))])|(REG_USE_SECRET_KEY));
}
