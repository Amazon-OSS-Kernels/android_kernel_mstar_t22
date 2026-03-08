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
#include "../../inc/common/drvNAND.h"

//=============================================================
#if (defined(NAND_DRV_TV_SBOOT) && NAND_DRV_TV_SBOOT)

U32 nand_clock_setting(U32 u32ClkParam)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)(CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET);

	u32ClkParam = FCIE3_SW_DEFAULT_CLK;

	if((REG(NC_DDR_CTRL) & BIT_DDR_MASM) || pNandDrv->u8_CellType == 1)
		return UNFD_ST_SUCCESS;

    #if defined(CONFIG_MSTAR_NIKE) || defined(CONFIG_MSTAR_MADISON) || \
        defined(CONFIG_MSTAR_NADAL) || defined(CONFIG_MSTAR_NUGGET)

    /*div4 enable*/
    REG_SET_BITS_UINT16(REG_CLK_NFIE, BIT_CLK_ENABLE);

   #if defined(CONFIG_MSTAR_MADISON)
   REG_SET_BITS_UINT16(REG_CLK_NFIE_DIV4_EN, BIT_CLK_SEL_1X);
   #endif

    /*set FCIE 4x clock*/
    REG_CLR_BITS_UINT16(REG_CLK_EMMC, BIT9|BIT8|EMMC_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_EMMC, u32ClkParam << 8);

    #elif defined(CONFIG_MSTAR_EINSTEIN) || defined(CONFIG_MSTAR_NAPOLI)

    /*div4 enable*/
    REG_SET_BITS_UINT16(REG_CLK_NFIE, BIT_CLK_ENABLE);

    /*set FCIE 4x clock*/
    REG_CLR_BITS_UINT16(REG_CLK_EMMC, BIT1|BIT0|EMMC_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_EMMC, u32ClkParam << 2);

    #elif defined(CONFIG_MSTAR_NIKON) || defined(CONFIG_MSTAR_KENYA) || defined(CONFIG_MSTAR_KRITI)

    /*div4 enable*/
    REG_SET_BITS_UINT16(REG_CLK_NFIE, BIT_CLK_ENABLE);

    /*set FCIE 4x clock*/
    REG_CLR_BITS_UINT16(REG_CLK_EMMC, BIT1|BIT0);
    REG_CLR_BITS_UINT16(REG_CLK_EMMC, EMMC_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_EMMC, u32ClkParam << 0);

	#elif defined(CONFIG_MSTAR_MONACO)   || defined(CONFIG_MSTAR_MIAMI) || \
		  defined(CONFIG_MSTAR_CLIPPERS) || defined(CONFIG_MSTAR_MUJI) || \
		  defined(CONFIG_MSTAR_CELTICS)  || defined(CONFIG_MSTAR_MONET) || \
		  defined(CONFIG_MSTAR_MUNICH)   || defined(CONFIG_MSTAR_MANHATTAN) || \
		  defined(CONFIG_MSTAR_MESSI)    || defined(CONFIG_MSTAR_MASERATI) || \
		  defined(CONFIG_MSTAR_MACAN)    || defined(CONFIG_MSTAR_MUSTANG) || \
		  defined(CONFIG_MSTAR_MOONEY)   || defined(CONFIG_MSTAR_MAXIM) || \
		  defined(CONFIG_MSTAR_MAINZ)    || defined(CONFIG_MSTAR_MATISSE) || \
          defined(CONFIG_MSTAR_M7621)    || defined(CONFIG_MSTAR_M5621) || \
          defined(CONFIG_MSTAR_M7221)    || defined(CONFIG_MSTAR_M7622) || \
          defined(CONFIG_MSTAR_M3822)    || defined(CONFIG_MSTAR_M7322) || \
          defined(CONFIG_MSTAR_MAZDA)

	REG_SET_BITS_UINT16(REG_CLK_NFIE, BIT_CLK_ENABLE);
	REG_CLR_BITS_UINT16(REG_CLK_NFIE, BIT1|BIT0);
	REG_SET_BITS_UINT16(REG_CLK_NFIE, u32ClkParam);

    #elif defined(CONFIG_MSTAR_KRATOS)

    /*div4 enable*/
    REG_SET_BITS_UINT16(REG_CLK_NFIE, BIT_CLK_SELECT);

    /*set FCIE 4x clock*/
    REG_CLR_BITS_UINT16(REG_CLK_NFIE, BIT1|BIT0);
    REG_CLR_BITS_UINT16(REG_CLK_NFIE, NFIE_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_NFIE, u32ClkParam << 0);

    #elif defined(CONFIG_MSTAR_MILAN) || defined(CONFIG_MSTAR_MARLON)

    /*select clock nfie*/
    REG_CLR_BITS_UINT16(REG_CLK_NFIE, BIT_CLK_SELECT);

    /*div4 enable*/
    REG_SET_BITS_UINT16(REG_CLK_NFIE_DIV4_EN, BIT_CLK_DIV4_ENABLE);

    /*set FCIE 4x clock*/
    REG_CLR_BITS_UINT16(REG_CLK_NFIE, BIT1|BIT0);
    REG_CLR_BITS_UINT16(REG_CLK_NFIE, NFIE_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_NFIE, u32ClkParam);

    #elif defined(CONFIG_MSTAR_M5321)

    REG_SET_BITS_UINT16(REG_CLK_NFIE_DIV4_EN, BIT_CLK_DIV4_ENABLE);

    REG_CLR_BITS_UINT16(REG_CLK_NFIE, BIT9|BIT8);
    REG_CLR_BITS_UINT16(REG_CLK_NFIE, NFIE_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_NFIE, u32ClkParam << 0);

    #else
	REG_CLR_BITS_UINT16(reg_ckg_fcie, BIT6-1); // enable FCIE clk, set to lowest clk

	REG_SET_BITS_UINT16(reg_ckg_fcie, u32ClkParam);

	#if defined(CONFIG_MSTAR_EIFFEL)
	//Default clock is 32MHz
	//u32ClkParam = sgau16_FCIEClk_1X_To_4X_[(u32ClkParam>>2)&0x0F];
	u32ClkParam = 0;
	/*set FCIE 4x clock*/
	REG_CLR_BITS_UINT16(REG_CLK_EMMC, BIT9|BIT8);
    REG_CLR_BITS_UINT16(REG_CLK_EMMC, EMMC_CLK_MASK);
    REG_SET_BITS_UINT16(REG_CLK_EMMC, u32ClkParam << 8);
	#endif

    #endif  // (ENABLE_NIKE) || defined(CONFIG_MSTAR_MADISON) || defined(CONFIG_MSTAR_MIAMI) || defined(CONFIG_MSTAR_MONACO) || defined(ENABLE_NADAL) || (ENABLE_NUGGET)  || defined(CONFIG_MSTAR_CLIPPERS) || defined(CONFIG_MSTAR_MUJI) || defined(CONFIG_MSTAR_MONET) || defined(CONFIG_MSTAR_MOONEY)


	#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
	REG_WRITE_UINT16(NC_LATCH_DATA, BIT_RE_DATA_LATCH_SEL_1T);
	#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	REG_WRITE_UINT16(NC_LATCH_DATA, BIT_NC_LATCH_DATA_2_0_T|BIT_NC_LATCH_STS_2_0_T);
	#endif

    return UNFD_ST_SUCCESS;
}

#if defined(CONFIG_MSTAR_KENYA)

void flush_cache_dram(unsigned long start_addr, unsigned long size);
U32 nand_hw_timer_delay_dram(U32 u32usTick);
void *drvNAND_get_DrvContext_address_dram(void);
U32 nand_flush_cache_dram(U32 u32_DMAAddr, U32 u32_ByteCnt);
U32 nand_translate_DMA_address_Ex_dram(U32 u32_DMAAddr, U32 u32_ByteCnt);


#include "cache.h"

#define Hit_Invalidate_D    0x11
#define Hit_Writeback_Inv_D 0x15
#define CONFIG_SYS_CACHELINE_SIZE   16
#define Hit_Invalidate_I 0x10
#define _BIT(x)                      (1<<(x))
#define cache_op(op,addr)                       \
	__asm__ __volatile__(                       \
	"   .set    push                    \n" \
	"   .set    noreorder               \n" \
	"   .set    mips3\n\t               \n" \
	"   cache   %0, %1                  \n" \
	"   .set    pop                 \n" \
	:                               \
	: "i" (op), "R" (*(unsigned char *)(addr)))

void flush_cache_dram(unsigned long start_addr, unsigned long size)
{
	unsigned long lsize = 16;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);
	unsigned int    dwReadData = 0;
	while (1) {
		cache_op(Hit_Writeback_Inv_D, addr);
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
	//Flush L2 All and Invalid
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData &= ~(0x1F);
	dwReadData |= 0x04;
	//Flush All and Invalid
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData |= _BIT(4);
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	do
	{
		dwReadData = *(volatile unsigned int *)(0xbf203104);
		dwReadData &= _BIT(10);
		//Check Status of Flush All Finish
	} while(dwReadData == 0);
	//Flush MIU pipe
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData &= ~(0x1F);
	dwReadData |= 0x02;
	//Flush MIU Pipe
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData |= _BIT(4);
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	do
	{
		dwReadData = *(volatile unsigned int *)(0xbf203104);
		dwReadData &= _BIT(9);
		//Check Status of Flush MIU Pipe Finish
	} while(dwReadData == 0);
}

U32 nand_hw_timer_delay_dram(U32 u32usTick)
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

void *drvNAND_get_DrvContext_address_dram(void) // exposed API
{
	return (void*)(CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET);
}

U32 nand_flush_cache_dram(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	#if defined(CONFIG_MCU_MIPS32)
	flush_cache_dram(u32_DMAAddr, u32_ByteCnt);
	#else
	u32_ByteCnt = u32_ByteCnt;
	#endif
	return u32_DMAAddr;
}

U32 nand_translate_DMA_address_Ex_dram(U32 u32_DMAAddr, U32 u32_ByteCnt)
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
	u32_ByteCnt = u32_ByteCnt;
	return (u32_DMAAddr & 0x1fffffff);
#endif
}

#endif

#endif
