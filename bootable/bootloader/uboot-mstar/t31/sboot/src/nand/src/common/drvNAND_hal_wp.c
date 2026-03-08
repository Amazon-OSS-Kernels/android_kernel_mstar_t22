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

//#include <linux/string.h>
#include "drvNAND.h"
#include "drvNAND_utl.h"

#define SECURE_CH_CNT         4

#if defined (CONFIG_NAND_SECURITY_GEN) && CONFIG_NAND_SECURITY_GEN==1

#define TZPC_REG_BASE_ADDR    (((0x1239*0x80)<<REG_OFFSET_SHIFT_BITS)+RIU_PM_BASE)

#define reg_tee_fcie_wp1_startaddr_3116    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x35)
#define reg_tee_fcie_wp1_startaddr_1500    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x34)
#define reg_tee_fcie_wp1_endaddr_3116      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x37)
#define reg_tee_fcie_wp1_endaddr_1500      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x36)

#define reg_tee_fcie_wp2_startaddr_3116    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x39)
#define reg_tee_fcie_wp2_startaddr_1500    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x38)
#define reg_tee_fcie_wp2_endaddr_3116      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x3b)
#define reg_tee_fcie_wp2_endaddr_1500      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x3a)

#define reg_tee_fcie_wp3_startaddr_3116    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x3d)
#define reg_tee_fcie_wp3_startaddr_1500    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x3c)
#define reg_tee_fcie_wp3_endaddr_3116      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x3f)
#define reg_tee_fcie_wp3_endaddr_1500      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x3e)

#define reg_tee_fcie_wp0_startaddr_3116    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x61)
#define reg_tee_fcie_wp0_startaddr_1500    GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x60)
#define reg_tee_fcie_wp0_endaddr_3116      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x63)
#define reg_tee_fcie_wp0_endaddr_1500      GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x62)

#define reg_tee_fcie_wp_ctrl               GET_REG_ADDR(TZPC_REG_BASE_ADDR, 0x64)
#define bit_tee_fcie_wp_enbase             0
#define bit_tee_fcie_wp0_en                BIT0
#define bit_tee_fcie_wp1_en                BIT1
#define bit_tee_fcie_wp2_en                BIT2
#define bit_tee_fcie_wp3_en                BIT3
#define bit_tee_fcie_wp_lockbase           8
#define bit_tee_fcie_wp0_lock              BIT8
#define bit_tee_fcie_wp1_lock              BIT9
#define bit_tee_fcie_wp2_lock              BIT10
#define bit_tee_fcie_wp3_lock              BIT11

void drvNAND_SetSecureRange(U32 StartRow, U32 EndRow, U8 PortNo)
{
    switch(PortNo)
    {
        case 0:
            REG_WRITE_UINT16(reg_tee_fcie_wp0_startaddr_3116, StartRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp0_startaddr_1500, StartRow & 0xFFFF);
		    REG_WRITE_UINT16(reg_tee_fcie_wp0_endaddr_3116, EndRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp0_endaddr_1500, EndRow & 0xFFFF);
            break;
        case 1:
            REG_WRITE_UINT16(reg_tee_fcie_wp1_startaddr_3116, StartRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp1_startaddr_1500, StartRow & 0xFFFF);
		    REG_WRITE_UINT16(reg_tee_fcie_wp1_endaddr_3116, EndRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp1_endaddr_1500, EndRow & 0xFFFF);
            break;
        case 2:
            REG_WRITE_UINT16(reg_tee_fcie_wp2_startaddr_3116, StartRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp2_startaddr_1500, StartRow & 0xFFFF);
		    REG_WRITE_UINT16(reg_tee_fcie_wp2_endaddr_3116, EndRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp2_endaddr_1500, EndRow & 0xFFFF);
            break;
        case 3:
            REG_WRITE_UINT16(reg_tee_fcie_wp3_startaddr_3116, StartRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp3_startaddr_1500, StartRow & 0xFFFF);
		    REG_WRITE_UINT16(reg_tee_fcie_wp3_endaddr_3116, EndRow >> 16);
            REG_WRITE_UINT16(reg_tee_fcie_wp3_endaddr_1500, EndRow & 0xFFFF);
            break;            
    }
}

void drvNAND_EnSecureRange(U8 PortNo)
{
    if(PortNo > SECURE_CH_CNT-1) // Ch.0~3
        return;

    REG_SET_BITS_UINT16(reg_tee_fcie_wp_ctrl, 1<<(PortNo+bit_tee_fcie_wp_enbase));
}

void drvNAND_DisSecureRange(U8 PortNo)
{
    if(PortNo > SECURE_CH_CNT-1) // Ch.0~3
        return;

    REG_CLR_BITS_UINT16(reg_tee_fcie_wp_ctrl, 1<<(PortNo+bit_tee_fcie_wp_enbase));    
}

void drvNAND_LockSecureRange(U8 PortNo)
{
    if(PortNo > SECURE_CH_CNT-1) // Ch.0~3
        return;

    REG_SET_BITS_UINT16(reg_tee_fcie_wp_ctrl, 1<<(PortNo+bit_tee_fcie_wp_lockbase));
}


//================================================================
#elif defined (CONFIG_NAND_SECURITY_GEN) && CONFIG_NAND_SECURITY_GEN==2

#define REG_BANK1618_ADDR			    (RIU_PM_BASE+(0x161800<<1))

#define REG_FCIE_WP_START_ROW_ADDR_L	GET_REG_ADDR(REG_BANK1618_ADDR, 0x60)
#define REG_FCIE_WP_START_ROW_ADDR_H	GET_REG_ADDR(REG_BANK1618_ADDR, 0x61)
#define REG_FCIE_WP_END_ROW_ADDR_L		GET_REG_ADDR(REG_BANK1618_ADDR, 0x62)
#define REG_FCIE_WP_END_ROW_ADDR_H		GET_REG_ADDR(REG_BANK1618_ADDR, 0x63)

#define REG_FCIE_WP_START_ROW_ADDR_L1	GET_REG_ADDR(REG_BANK1618_ADDR, 0x34)
#define REG_FCIE_WP_START_ROW_ADDR_H1	GET_REG_ADDR(REG_BANK1618_ADDR, 0x35)
#define REG_FCIE_WP_END_ROW_ADDR_L1		GET_REG_ADDR(REG_BANK1618_ADDR, 0x36)
#define REG_FCIE_WP_END_ROW_ADDR_H1		GET_REG_ADDR(REG_BANK1618_ADDR, 0x37)

#define REG_FCIE_WP_START_ROW_ADDR_L3	GET_REG_ADDR(REG_BANK1618_ADDR, 0x3C)
#define REG_FCIE_WP_START_ROW_ADDR_H3	GET_REG_ADDR(REG_BANK1618_ADDR, 0x3D)
#define REG_FCIE_WP_END_ROW_ADDR_L3		GET_REG_ADDR(REG_BANK1618_ADDR, 0x3E)
#define REG_FCIE_WP_END_ROW_ADDR_H3		GET_REG_ADDR(REG_BANK1618_ADDR, 0x3F)

#define REG_FCIE_WP_ENABLE				GET_REG_ADDR(REG_BANK1618_ADDR, 0x64)
#define BIT_FCIE_WP_ENABLE				BIT0
#define BIT_FCIE_WP_ENABLE1				BIT1
#define BIT_FCIE_WP_ENABLE3				BIT3


void nand_sync_WP_RangeNStart(U32 u32_RowStart, U32 u32_RowEnd)
{
    //nand_debug(0, 1, "Start 0x%X, End 0x%X\n", u32_RowStart, u32_RowEnd);
    //LDR_PUTS("RowStart: "); LDR_PUTDW(u32_RowStart); LDR_PUTS("\n");
    //LDR_PUTS("RowEnd: "); LDR_PUTDW(u32_RowEnd); LDR_PUTS("\n");
    #if 1
    REG_WRITE_UINT16(REG_FCIE_WP_START_ROW_ADDR_L3, u32_RowStart & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_START_ROW_ADDR_H3, (u32_RowStart >> 16) & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_END_ROW_ADDR_L3, u32_RowEnd & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_END_ROW_ADDR_H3, (u32_RowEnd >> 16) & 0xFFFF);
    //REG_SET_BITS_UINT16(REG_FCIE_WP_ENABLE, BIT_FCIE_WP_ENABLE3);
    #endif
}

void nand_set_WP_CIS(U32 u32_RowStart, U32 u32_RowEnd)
{
    //nand_debug(0, 1, "Start 0x%X, End 0x%X\n", u32_RowStart, u32_RowEnd);
    //LDR_PUTS("RowStart: "); LDR_PUTDW(u32_RowStart); LDR_PUTS("\n");
    //LDR_PUTS("RowEnd: "); LDR_PUTDW(u32_RowEnd); LDR_PUTS("\n");
    #if 1
    REG_WRITE_UINT16(REG_FCIE_WP_START_ROW_ADDR_L1, u32_RowStart & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_START_ROW_ADDR_H1, (u32_RowStart >> 16) & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_END_ROW_ADDR_L1, u32_RowEnd & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_END_ROW_ADDR_H1, (u32_RowEnd >> 16) & 0xFFFF);
    REG_SET_BITS_UINT16(REG_FCIE_WP_ENABLE, BIT_FCIE_WP_ENABLE1);
    #endif
}

void nand_set_WP_RangeNStart(U32 u32_RowStart, U32 u32_RowEnd)
{
    nand_debug(0, 1, "Start 0x%X, End 0x%X\n", u32_RowStart, u32_RowEnd);
    REG_WRITE_UINT16(REG_FCIE_WP_START_ROW_ADDR_L, u32_RowStart & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_START_ROW_ADDR_H, (u32_RowStart >> 16) & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_END_ROW_ADDR_L, u32_RowEnd & 0xFFFF);
    REG_WRITE_UINT16(REG_FCIE_WP_END_ROW_ADDR_H, (u32_RowEnd >> 16) & 0xFFFF);
    REG_SET_BITS_UINT16(REG_FCIE_WP_ENABLE, BIT_FCIE_WP_ENABLE);
}

void drvNAND_SetSecureRange(U32 StartRow, U32 EndRow, U8 PortNo)
{
    nand_set_WP_RangeNStart(StartRow, EndRow);
}


void drvNAND_EnSecureRange(U8 PortNo)
{
    if(PortNo > SECURE_CH_CNT-1) // Ch.0~3
        return;

    //REG_SET_BITS_UINT16(reg_tee_fcie_wp_ctrl, 1<<(PortNo+bit_tee_fcie_wp_enbase));
}


#if defined(FCIE_REG_TEE_BASE_ADDR)
void NC_Get_TEE_Grant(void)
{
    //LDR_PUTS(" NC_Get_TEE_Grant + \n");
    REG_SET_BITS_UINT16(NC_TEE_BOOT_MODE, BIT_TEE_REQ);
    while(1)
    {
        if((REG(NC_TEE_BOOT_MODE) & BIT_GRANT2TEE) == BIT_GRANT2TEE)
        {
            nand_debug(0, 1, "Grant to TEE\n");
            break;
        }
    }
    //LDR_PUTS(" NC_Get_TEE_Grant - \n");
}

void NC_Get_REE_Grant(void)
{
    //LDR_PUTS(" NC_Get_REE_Grant +\n");
    REG_SET_BITS_UINT16(NC_BOOT_MODE, BIT_REE_REQ);
    while(1)
    {
        if((REG(NC_BOOT_MODE) & BIT_GRANT2REE) == BIT_GRANT2REE)
        {
            nand_debug(0, 1, "Grant to REE\n");       
            break;
        }
    }
    //LDR_PUTS(" NC_Get_REE_Grant -\n");
}
void NC_Release_TEE_Grant(void)
{
    //LDR_PUTS(" NC_Release_TEE_Grant \n");
    REG_CLR_BITS_UINT16(NC_TEE_BOOT_MODE, BIT_TEE_REQ);
}

void NC_Release_REE_Grant(void)
{
    //LDR_PUTS(" NC_Release_REE_Grant \n");
    REG_CLR_BITS_UINT16(NC_BOOT_MODE, BIT_REE_REQ);
}
#endif


#endif 

