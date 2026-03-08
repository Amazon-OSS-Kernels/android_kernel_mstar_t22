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
#include "halMIU.h"
#include "../../drv/drvMIU.h"


U16 HAL_MIU_Read2Byte( U32 u32Reg)
{
    return RIU[u32Reg];
}

U32 HAL_MIU_Read4Byte( U32 u32Reg)
{
    return ( (RIU[u32Reg] & 0xFFFF) + (RIU[(u32Reg + 0x2)] << 16) );
}

void HAL_MIU_Write2Byte( U32 u32Reg, U16 u16Value)
{
    RIU[u32Reg] = u16Value;
}

void HAL_MIU_Write2ByteBit(U32 u32Reg, BOOL bEnable, U16 u16Mask)
{
    U16 val = HAL_MIU_Read2Byte(u32Reg);
    if(val >= 0x0 && val <= 0xFFFF)
    {
        val = (bEnable) ? (val | u16Mask) : (val & ~u16Mask);
        HAL_MIU_Write2Byte(u32Reg, val);
    }
}

void HAL_MIU_Write4Byte( U32 u32Reg, U32 u32Value)
{
    RIU[u32Reg] = u32Value;
    RIU[u32Reg + 2] = (u32Value >> 16);
}

void HAL_MIU_RestoreProtect(void)
{
//TODO : chip back verify
#if 1
    U16 u16count;
    U16 u16count_all = 0;
    //Read PM Domain
    U32 volatile address = 0x0;
    U16 volatile *pdata = 0x0;
    U16 data = 0x0;

    address = HAL_MIU_Read4Byte(REG_MIU_PM_PROTECT_ARRAY_ADDR);
// IMPORTANT!!! DO NOT CHANGE SEQUENCE OF RESTORE MIU PROTECT!!!
    if( address != 0x0)
    {
        pdata = (U32 volatile *)address;
        //check data
        if(pdata[u16count_all] != 0x1234)
        {
            //DRAM Data or DRAM address has been change
            return;
        }
        u16count_all += 1;

        // Disable Protect
        HAL_MIU_Write4Byte(REG_MIU_PROTECT_EN, DISABLE);

        // Protect Dram Size
        for(u16count = 0; u16count < MIU_PROTECT_DRAM_SIZE_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT_DDR_BOUND + (u16count * 2), data);
        }
        u16count_all += u16count;

        // Protect ID
        for(u16count = 0; u16count < MIU_PROTECT_ID_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT0_ID0 + (u16count * 2), data);
        }
        u16count_all += u16count;
        for(u16count = 0; u16count < MIU_PROTECT_ID_GP1_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT0_GROUP1_ID0 + (u16count * 2), data);
        }
        u16count_all += u16count;
        // Enable IDs
        for(u16count = 0; u16count < MIU_PROTECT_ID_ENABLE_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT0_ID_ENABLE + (u16count * 2), data);
        }
        u16count_all += u16count;
        // Select ID Group
        for(u16count = 0; u16count < MIU_PROTECT_ID_GP_Select_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT_GROUP_SEL + (u16count * 2), data);
        }
        u16count_all += u16count;
        // Protect Address
        for(u16count = 0; u16count < MIU_PROTECT_ADDR_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT0_START + (u16count * 2), data);
        }
        u16count_all += u16count;
        // Enable Protect
        for(u16count = 0; u16count < MIU_PROTECT_ENABLE_NUMBER; u16count++)
        {
            data = pdata[u16count + u16count_all];
            HAL_MIU_Write2Byte(REG_MIU_PROTECT_EN + (u16count * 2), data);
        }
        // Clean Protect Hit Log
        HAL_MIU_Write2ByteBit(REG_MIU_PROTECT_HIE_LOG, ENABLE, REG_MIU_PROTECT_LOG_CLR);
        HAL_MIU_Write2ByteBit(REG_MIU_PROTECT_HIE_LOG, DISABLE, REG_MIU_PROTECT_LOG_CLR);
    }
#endif
    return;
}
