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
//=======================================================================
//
//  drvNAND_platform.c - Storage Team, 2009/08/20
//
//  Design Notes: defines common platform-dependent functions.
//
//    1. 2009/08/25 - support C5 eCos platform
//
//=======================================================================
#include "../../inc/common/drvNAND.h"

//=============================================================
#if (defined(NAND_DRV_TV_SBOOT) && NAND_DRV_TV_SBOOT)

U32 nand_hw_timer_delay(U32 u32usTick)
{
    U32 u32HWTimer = 0;
	volatile U16 u16TimerLow = 0;
	volatile U16 u16TimerHigh = 0;

	// reset HW timer
	REG_WRITE_UINT16(TIMER0_MAX_LOW, 0xFFFF);
	REG_WRITE_UINT16(TIMER0_MAX_HIGH, 0xFFFF);
	REG_WRITE_UINT16(TIMER0_ENABLE, 0);

	// start HW timer
	REG_SET_BITS_UINT16(TIMER0_ENABLE, 0x0001);

	while( u32HWTimer < 12*u32usTick ) // wait for u32usTick micro seconds
	{
		REG_READ_UINT16(TIMER0_CAP_LOW, u16TimerLow);
		REG_READ_UINT16(TIMER0_CAP_HIGH, u16TimerHigh);

		u32HWTimer = (u16TimerHigh<<16) | u16TimerLow;
	}

	REG_WRITE_UINT16(TIMER0_ENABLE, 0);

    return u32usTick+1;
}


//static UNFD_ALIGN0 NAND_DRIVER sg_NandDrv UNFD_ALIGN1;

void *drvNAND_get_DrvContext_address(void) // exposed API
{
	return (void*)(CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET);
}

void *drvNAND_get_DrvContext_PPM(void) // exposed API
{
    #if defined(CONFIG_TEE_LOADER)
    extern PAIRED_PAGE_MAP_t ga_tPairedPageMap[];

    return (void *)&ga_tPairedPageMap[0];
    #else
    return (void*)(CONFIG_BOOTRAM_LOADADDR - 0x4000);
    #endif
}

#if defined(REG_DMAFUNC_ADDR_HIGH) && defined(REG_DMAFUNC_ADDR_LOW)
U32 drvNAND_get_ReadSectors_address(void)
{
    return ((REG(REG_DMAFUNC_ADDR_HIGH)<<16)+REG(REG_DMAFUNC_ADDR_LOW));
}
#endif
U32 nand_flush_cache(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	#if defined(CONFIG_MCU_MIPS32)
	flush_cache(u32_DMAAddr, u32_ByteCnt);
	#else
	//u32_ByteCnt = u32_ByteCnt;
	#endif
	return u32_DMAAddr;
}

U32 nand_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
#if defined(CONFIG_MCU_MIPS32)
	u32_ByteCnt = u32_ByteCnt;
	return (u32_DMAAddr & 0x1fffffff);
#elif defined(CONFIG_MSTAR_CLIPPERS)
    u32_ByteCnt = u32_ByteCnt;
    
    if(u32_DMAAddr < CONFIG_MIU1_BUSADDR) // MIU0
    {
        REG_CLR_BITS_UINT16(NC_MIU_DMA_SEL, BIT_MIU0_SELn);
        return (u32_DMAAddr - CONFIG_MIU0_BUSADDR);
    }
    else
    {   REG_SET_BITS_UINT16(NC_MIU_DMA_SEL, BIT_MIU1_SEL);
        return (u32_DMAAddr - CONFIG_MIU1_BUSADDR);
    }

    return u32_DMAAddr;
#else
	//u32_ByteCnt = u32_ByteCnt;
	return (u32_DMAAddr & 0x1fffffff);
#endif
}

void nand_switch_sram(U32 u32_Offset)
{
	//u32_Offset = u32_Offset;
	#if  defined(CONFIG_MSTAR_MADISON)
	if( u32_Offset >= 8192 )
		REG_SET_BITS_UINT16(NC_REG_PAD_SWITCH, BIT6);
	else
		REG_CLR_BITS_UINT16(NC_REG_PAD_SWITCH, BIT6);
	#endif
}

#ifdef REMAP_PPM_ADDR
void nand_remap_ppm_addr(PAIRED_PAGE_MAP_t **pptPairedPageMapSRAM)
{
	U16 u16_Reg;

	REG_READ_UINT16(REG_CHIP_REVISION, u16_Reg);
	u16_Reg = (u16_Reg>>8)&0xFF;

	#if defined(CONFIG_MSTAR_EIFFEL)
	if(u16_Reg >= 1)
		*pptPairedPageMapSRAM = (PAIRED_PAGE_MAP_t *) (ROM_PPM_ADDR_U02);
	#elif defined(CONFIG_MSTAR_NIKE)
	if(u16_Reg >= 2)
		*pptPairedPageMapSRAM = (PAIRED_PAGE_MAP_t *) (ROM_PPM_ADDR_U03);
	#elif defined(CONFIG_MSTAR_NADAL)
	if(u16_Reg >= 0)
		*pptPairedPageMapSRAM = (PAIRED_PAGE_MAP_t *) (ROM_PPM_ADDR_U01);
	#elif defined(CONFIG_MSTAR_NUGGET)
	if(u16_Reg >= 1)
		*pptPairedPageMapSRAM = (PAIRED_PAGE_MAP_t *) (ROM_PPM_ADDR_U02);
	#endif
}
#endif

#ifdef PUTK
void print_err(U32 Err)
{
	char x;

	PUTK('N');
	x = ((char)((Err>>4)&0xF));
	if(x<0xA)
		PUTK((x+0x30));
	else
		PUTK((x+0x37));

	x = ((char)(Err&0xF));
	if(x<0xA)
		PUTK((x+0x30));
	else
		PUTK((x+0x37));

	PUTK('\n');
	PUTK('\r');
}
#endif

#else

    #error "Error! no platform functions."

#endif

