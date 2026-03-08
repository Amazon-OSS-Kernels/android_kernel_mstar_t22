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


//========================================================
// HAL pre-processors
//========================================================
#if IF_FCIE_SHARE_PINS
    #define NC_PAD_SWITCH(enable)
#else
    #define NC_PAD_SWITCH(enable)
#endif

#if IF_FCIE_SHARE_CLK
    #define NC_CLK_SETTING(ClkParam)
#else
    #define NC_CLK_SETTING(ClkParam)
#endif

#if IF_FCIE_SHARE_IP
    #define NC_LOCK_FCIE()
    #define NC_UNLOCK_FCIE()
    #define NC_RECONFIG()
#else
    #define NC_LOCK_FCIE()
    #define NC_UNLOCK_FCIE()
    #define NC_RECONFIG()
#endif

#if defined(NEED_FLUSH_MIU_PIPE) && NEED_FLUSH_MIU_PIPE
    #define FLUSH_MIU_PIPE()
#else
    #define FLUSH_MIU_PIPE()
#endif

extern U32 nand_hw_timer_delay_dram(U32 u32usTick);
extern void *drvNAND_get_DrvContext_address_dram(void); // exposed API
extern U32 nand_flush_cache_dram(U32 u32_DMAAddr, U32 u32_ByteCnt);
extern U32 nand_translate_DMA_address_Ex_dram(U32 u32_DMAAddr, U32 u32_ByteCnt);

U32 NC_waitFifoClkReady_dram(void);
#if defined(MIU_CHECK_LAST_DONE)&&MIU_CHECK_LAST_DONE
U32 NC_wait_MIULastDone_dram(void);
#endif
U32  NC_WaitComplete_dram(U32 u32_EventReg, U16 u16_WaitEvent, U32 u32_MicroSec);
U32  NC_ReadSectors_dram(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_SectorCnt);
U32 Micron_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);


U32 NC_WaitComplete_dram(U32 u32_EventReg, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	volatile U32 u32_Count;
	volatile U16 u16_Reg;

	for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++)
	{
		REG_READ_UINT16(u32_EventReg, u16_Reg);
		if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
			break;

		nand_hw_timer_delay_dram(HW_TIMER_DELAY_1us);
	}

	if (u32_Count < u32_MicroSec)
		REG_W1C_BITS_UINT16(u32_EventReg, u16_WaitEvent); /*clear events*/

	return u32_Count;

}


#define WAIT_FIFOCLK_READY_CNT_DRAM  0x10000
U32 NC_waitFifoClkReady_dram(void)
{
	volatile U32 u32_i;
	volatile U16 u16_Reg;

	for(u32_i=0; u32_i<WAIT_FIFOCLK_READY_CNT_DRAM; u32_i++)
	{
		REG_READ_UINT16(NC_MMA_PRI_REG, u16_Reg);
		if(u16_Reg & BIT_NC_FIFO_CLKRDY)
			break;
		nand_hw_timer_delay_dram(HW_TIMER_DELAY_1us);
	}

	if(WAIT_FIFOCLK_READY_CNT_DRAM == u32_i)
	{
	    // add debug register
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	return UNFD_ST_SUCCESS;

}

#if defined(MIU_CHECK_LAST_DONE)&&MIU_CHECK_LAST_DONE
U32 NC_wait_MIULastDone_dram(void)
{
	volatile U32 u32_retry=0;
	volatile U16 u16_Reg;

	while(u32_retry < DELAY_1s_in_us)
	{
		REG_READ_UINT16(NC_REG_MIU_LAST_DONE, u16_Reg);
		if((u16_Reg & BIT_MIU_LAST_DONE) == BIT_MIU_LAST_DONE)
		{
			REG_WRITE_UINT16(NC_REG_MIU_LAST_DONE, BIT_MIU_LAST_DONE);
			break;
		}

		u32_retry++;
		nand_hw_timer_delay_dram(1);
	}

	if(u32_retry == DELAY_1s_in_us)
	{
	    // add debug register
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	return UNFD_ST_SUCCESS;
}
#endif

U32  NC_ReadSectors_dram(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_SectorCnt)
{
    NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address_dram();
    U32         u32_DataDMAAddr;
    U32         u32_Err;
    U16         u16_Tmp;
	U8			u8_RetryCnt = 0;


    // HW setting
    NC_PAD_SWITCH(1);
    NC_CLK_SETTING(pNandDrv->u32_Clk);
    NC_RECONFIG();
    NC_SET_DDR_MODE(); // to turn on ONFI clk
	

RETRY:
    REG_WRITE_UINT16(NC_MIE_EVENT, (BIT_NC_JOB_END | BIT_MMA_DATA_END));

    u32_DataDMAAddr = nand_translate_DMA_address_Ex_dram((U32)pu8_DataBuf, pNandDrv->u16_SectorByteCnt*u32_SectorCnt);

    REG_WRITE_UINT16(NC_JOB_BL_CNT, u32_SectorCnt);

    REG_WRITE_UINT16(NC_PART_MODE, (BIT_PARTIAL_MODE_EN)|((u32_SectorCnt - 1) << BIT_START_SECTOR_CNT_SHIFT)|(u8_SectorInPage << BIT_START_SECTOR_IDX_SHIFT));

    REG_WRITE_UINT16(NC_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_WRITE_UINT16(NC_SDIO_ADDR1, u32_DataDMAAddr >> 16);
	NC_waitFifoClkReady_dram();

    REG_SET_BITS_UINT16(NC_PATH_CTL, BIT_MMA_EN);

    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);

    REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage << pNandDrv->u8_SectorByteCntBits);

    REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
    REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx >> 16);

    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
    REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle<< 8) | (CMD_0x00));

    REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0x30);
    REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_SER_DIN);

    REG_WRITE_UINT16(NC_CTRL, (BIT_NC_CIFD_ACCESS|BIT_NC_JOB_START) );
    if (NC_WaitComplete_dram(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END, WAIT_READ_TIME) == (WAIT_READ_TIME))
    {
        NC_CLR_DDR_MODE();
        REG_WRITE_UINT16(NC_PART_MODE, 0);

        // add debug register
        //REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0xC010);
        return UNFD_ST_ERR_R_TIMEOUT;
    }

	FLUSH_MIU_PIPE();

    // Check ECC
    REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
    if (u16_Tmp & BIT_NC_ECC_FAIL)
    {
        NC_CLR_DDR_MODE();
        REG_WRITE_UINT16(NC_PART_MODE, 0);
		if((pNandDrv->u8_CellType) && u8_RetryCnt < 8 )
		{
			u8_RetryCnt ++;
			NC_SendReadRetryCmd(u8_RetryCnt,0);
			goto RETRY;
		}
        // add debug register
        //REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0xC020);
        return UNFD_ST_ERR_ECC_FAIL;
	}
    #if defined(MIU_CHECK_LAST_DONE) && MIU_CHECK_LAST_DONE
	// check until MIU is done
	if((u32_Err = NC_wait_MIULastDone_dram()) != UNFD_ST_SUCCESS)
	{
		NC_CLR_DDR_MODE();
		REG_WRITE_UINT16(NC_PART_MODE, 0);

		// add debug register
		//REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0xC030);
		return u32_Err;
	}
	#endif

	if(pu8_SpareBuf)
		*((U16*)pu8_SpareBuf) = REG(NC_CIFD_ADDR(0));
	
	REG_WRITE_UINT16(NC_PART_MODE, 0);

	NC_CLR_DDR_MODE();

	if((pNandDrv->u8_CellType) && u8_RetryCnt)
	{
		NC_SendReadRetryCmd(0,1);
	}

	//REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0xC100);

	return UNFD_ST_SUCCESS;
}

