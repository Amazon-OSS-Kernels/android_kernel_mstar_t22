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
//  drvNAND_hal.c - Storage Team, 2009/08/20
//
//  Design Notes:
//    2009/08/21 - FCIE3 physical layer functions
//
//=======================================================================

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


//========================================================
// HAL misc. function definitions
//========================================================
#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
U32 NC_waitFifoClkReady(void);
#if defined(MIU_CHECK_LAST_DONE)&&MIU_CHECK_LAST_DONE
U32 NC_wait_MIULastDone(void);
#endif
U32  NC_WaitComplete(U32 u32_EventReg, U16 u16_WaitEvent, U32 u32_MicroSec);

#define WAIT_FIFOCLK_READY_CNT  0x10000
U32 NC_waitFifoClkReady(void)
{
	volatile U32 u32_i;
	volatile U16 u16_Reg;

	for(u32_i=0; u32_i<WAIT_FIFOCLK_READY_CNT; u32_i++)
	{
		REG_READ_UINT16(NC_MMA_PRI_REG, u16_Reg);
		if(u16_Reg & BIT_NC_FIFO_CLKRDY)
			break;
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	}

	if(WAIT_FIFOCLK_READY_CNT == u32_i)
	{
	    // add debug register
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	return UNFD_ST_SUCCESS;

}

#if defined(MIU_CHECK_LAST_DONE)&&MIU_CHECK_LAST_DONE
U32 NC_wait_MIULastDone(void)
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
		nand_hw_timer_delay(1);
	}

	if(u32_retry == DELAY_1s_in_us)
	{
	    // add debug register
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	return UNFD_ST_SUCCESS;
}
#endif

U32  NC_ReadSectors(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_SectorCnt)
{
    NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
    U32         u32_DataDMAAddr;
    U32         u32_Err;
    U16         u16_Tmp;

    // HW setting
    NC_PAD_SWITCH(1);
    NC_CLK_SETTING(pNandDrv->u32_Clk);
    NC_RECONFIG();
    NC_SET_DDR_MODE(); // to turn on ONFI clk

    REG_WRITE_UINT16(NC_MIE_EVENT, (BIT_NC_JOB_END | BIT_MMA_DATA_END));

    u32_DataDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_DataBuf, pNandDrv->u16_SectorByteCnt*u32_SectorCnt);

    REG_WRITE_UINT16(NC_JOB_BL_CNT, u32_SectorCnt);

    #if 0
    REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN);
    REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_CNT_MASK);
    REG_SET_BITS_UINT16(NC_PART_MODE, (u32_SectorCnt - 1) << BIT_START_SECTOR_CNT_SHIFT);
    REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
    REG_SET_BITS_UINT16(NC_PART_MODE, u8_SectorInPage << BIT_START_SECTOR_IDX_SHIFT);
    #else
    REG_WRITE_UINT16(NC_PART_MODE, (BIT_PARTIAL_MODE_EN)|((u32_SectorCnt - 1) << BIT_START_SECTOR_CNT_SHIFT)|(u8_SectorInPage << BIT_START_SECTOR_IDX_SHIFT));
    #endif

    REG_WRITE_UINT16(NC_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_WRITE_UINT16(NC_SDIO_ADDR1, u32_DataDMAAddr >> 16);
    //REG_CLR_BITS_UINT16(NC_MMA_PRI_REG, BIT_NC_DMA_DIR_W);
	NC_waitFifoClkReady();

    REG_SET_BITS_UINT16(NC_PATH_CTL, BIT_MMA_EN);

    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);

    REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage << pNandDrv->u8_SectorByteCntBits);

    REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
    REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx >> 16);
	#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
	if( pNandDrv->u8_bbm_wa == 0xBB)
	{
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	}
	#endif

    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
	if( pNandDrv->u8_bbm_wa == 0xBB)
	{
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_ONE|OP_ADR_SET_1) << 8) | CMD_0x80);
	}
	#endif
    REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle<< 8) | (CMD_0x00));

    REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0x30);
    REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_SER_DIN);

    REG_WRITE_UINT16(NC_CTRL, (BIT_NC_CIFD_ACCESS|BIT_NC_JOB_START) );
    if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END, WAIT_READ_TIME) == (WAIT_READ_TIME))
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

        // add debug register
        //REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0xC020);
        return UNFD_ST_ERR_ECC_FAIL;
	}

    #if defined(MIU_CHECK_LAST_DONE) && MIU_CHECK_LAST_DONE
    // check until MIU is done
    if((u32_Err = NC_wait_MIULastDone()) != UNFD_ST_SUCCESS)
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

    //REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0xC100);

	return UNFD_ST_SUCCESS;
}

U32  NC_ReadSector_RIUMode(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf )
{
	volatile U16 u16_Tmp, u16_i, u16_j, u16_k, u16_Cnt, u16_ByteLoc;
	U16 u16_Col;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U16 *pu16_DataBuf = (U16*)pu8_DataBuf;
	U32 u32_Tmp;
	U32 u32_Err = UNFD_ST_ERR_R_TIMEOUT_RM;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_CLK_SETTING(pNandDrv->u32_Clk);
	NC_RECONFIG();
	NC_SET_DDR_MODE();
	
	REG_SET_BITS_UINT16(NC_R2N_CTRL, BIT_R2N_MODE_EN); // enable RIU Mode

	// if Partial Mode (if not 512B/page, set Partial Mode)
	#if 0
	REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN);
	REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
	REG_SET_BITS_UINT16(NC_PART_MODE, u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT);
	#else
	REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN|(u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT));
	#endif
	REG_WRITE_UINT16(NC_JOB_BL_CNT, 1); /* read 1 sector only */
	//REG_WRITE_UINT16(NC_SDIO_ADDR0, 0);
	//REG_WRITE_UINT16(NC_SDIO_ADDR1, 0);
	NC_waitFifoClkReady();
	REG_SET_BITS_UINT16(NC_PATH_CTL, BIT_MMA_EN);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	u16_Col = u8_SectorInPage << pNandDrv->u8_SectorByteCntBits;
	REG_WRITE_UINT16(NC_AUXREG_DAT, u16_Col);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx >> 16);
	#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
	if( pNandDrv->u8_bbm_wa == 0xBB)
	{
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	}
	#endif

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
	if( pNandDrv->u8_bbm_wa == 0xBB)
	{
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_ONE|OP_ADR_SET_1) << 8) | CMD_0x80);
	}
	#endif
	REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle<< 8) | (CMD_0x00));

	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0x30);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_SER_DIN);

	REG_WRITE_UINT16(NC_CTRL,
           (BIT_NC_CIFD_ACCESS|BIT_NC_JOB_START));

	u16_Cnt = pNandDrv->u16_SectorByteCnt >> 9; // how many 512B per sector
	for (u16_j=0; u16_j<u16_Cnt; u16_j++)
	{   // read data
		if (NC_WaitComplete(NC_R2N_STAT,BIT_RIU_RDY_MMA, WAIT_READ_TIME)==  WAIT_READ_TIME)
		{	
			goto out;
		}

		if (0 == u16_j)
			REG_SET_BITS_UINT16(NC_R2N_CTRL, BIT_R2N_DI_START);

		u16_k = u16_j << 8;

		for (u16_i=0; u16_i<256; u16_i++)
		{
			if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_R2N_RDY, WAIT_READ_TIME) == WAIT_READ_TIME)
			{
				goto out;
			}
			REG_READ_UINT16(NC_R2N_DATA_RD, pu16_DataBuf[u16_k + u16_i]);
			REG_SET_BITS_UINT16(NC_R2N_CTRL, BIT_R2N_DI_EN);
		}
	}
  	REG_WRITE_UINT16(NC_R2N_CTRL, BIT_R2N_MODE_EN|BIT_R2N_DI_END);

	// wait for events
	u32_Tmp = 0;
	do{
		u16_Tmp = 0; // do not remove this line
		REG_READ_UINT16(NC_MIE_EVENT, u16_Tmp);
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
		u32_Tmp++;
		if(u32_Tmp > WAIT_READ_TIME)
  		{
			goto out;
		}
	}while(((u16_Tmp & BIT_NC_R2N_ECC)==0) && ((u16_Tmp & (BIT_NC_JOB_END | BIT_MMA_DATA_END)) != (BIT_NC_JOB_END | BIT_MMA_DATA_END)));

    // if ECC Fail
	REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
	if ((u16_Tmp & BIT_NC_ECC_FLGA_MASK) == BIT_NC_ECC_FLGA_FAIL)
	{
		goto out;
	}

	REG_READ_UINT16(NC_MIE_EVENT, u16_Tmp);
	if (u16_Tmp & BIT_NC_R2N_ECC)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "RIU Mode, ECC happens: ");
		// make CIFD available for SW
		// [CAUTION]: before clear BIT_NC_R2N_ECC, JOB_END would not set.
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_R2N_ECC);
		if(NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END, WAIT_READ_TIME) == WAIT_READ_TIME)
		{
			REG_CLR_BITS_UINT16(NC_PATH_CTL, BIT_MMA_EN);
			REG_CLR_BITS_UINT16(NC_R2N_CTRL, BIT_R2N_MODE_EN);
			NC_CLR_DDR_MODE();
		}

	// SW correct ECC bits
		if(pu8_SpareBuf)
			*((U16*)pu8_SpareBuf) = REG(NC_CIFD_ADDR(0));

		REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
		u16_Cnt = (u16_Tmp & BIT_NC_ECC_CNT_MASK) >> BIT_NC_ECC_CNT_SHIFT;
		for (u16_i=0; u16_i<u16_Cnt; u16_i++)
		{
			REG_CLR_BITS_UINT16(NC_ECC_STAT2, BIT_NC_ECC_SEL_LOC_MASK);
			REG_SET_BITS_UINT16(NC_ECC_STAT2, u16_i<<BIT_NC_ECC_SEL_LOC_SHIFT);
			LABEL_ECC_LOC_READ:
			//REG_READ_UINT16(NC_ECC_LOC, u16_Tmp);
			REG_READ_UINT16(NC_ECC_LOC, u16_j);
			REG_READ_UINT16(NC_ECC_LOC, u16_k);
			if(u16_j != u16_k)
				goto LABEL_ECC_LOC_READ;
			u16_ByteLoc = u16_j >> 3;
			if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt)
			{
				pu8_DataBuf[u16_ByteLoc] ^= 1<<(u16_j%8);
			}
			else if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt + 2)
			{
				if(pu8_SpareBuf)
					pu8_SpareBuf[u16_ByteLoc-pNandDrv->u16_SectorByteCnt] ^= 1<<(u16_j%8);
			}			
		}

    	u16_Tmp = BIT_NC_R2N_ECC;
  	}
	else{
		if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END, WAIT_READ_TIME) == WAIT_READ_TIME)
		{
			goto out;
		}

		if(pu8_SpareBuf)
			*((U16*)pu8_SpareBuf) = REG(NC_CIFD_ADDR(0));
	}

	u32_Err = UNFD_ST_SUCCESS;
//	NC_CLR_DDR_MODE();
//  	REG_WRITE_UINT16(NC_PART_MODE, 0);
//  	REG_CLR_BITS_UINT16(NC_R2N_CTRL, BIT_R2N_MODE_EN);
//	return UNFD_ST_SUCCESS;
out:
	REG_WRITE_UINT16(NC_PART_MODE, 0);
	REG_CLR_BITS_UINT16(NC_PATH_CTL, BIT_MMA_EN);
	REG_CLR_BITS_UINT16(NC_R2N_CTRL, BIT_R2N_MODE_EN);
	NC_CLR_DDR_MODE();
	return u32_Err; // timeout

}

U32 NC_WaitComplete(U32 u32_EventReg, U16 u16_WaitEvent, U32 u32_MicroSec)
{
	volatile U32 u32_Count;
	volatile U16 u16_Reg;

	for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++)
	{
		REG_READ_UINT16(u32_EventReg, u16_Reg);
		if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
			break;

		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	}

	if (u32_Count < u32_MicroSec)
		REG_W1C_BITS_UINT16(u32_EventReg, u16_WaitEvent); /*clear events*/

	return u32_Count;

}

#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
void NC_GetCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt)
{
	U32 u32_i, u32_BufPos;
	U16 u16_Tmp;

	//u32_CIFDPos should be 0
	//u32_ByteCnt should be even
	#if 0
	if (u32_CIFDPos & 1) {
		REG_READ_UINT16(NC_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		pu8_Buf[0] = (U8)(u16_Tmp >> 8);
		u32_CIFDPos += 1;
		u32_ByteCnt -= 1;
		u32_BufPos = 1;
	} else
	#endif
		u32_BufPos = 0;

	for (u32_i=0; u32_i<u32_ByteCnt>>1; u32_i++) {
		REG_READ_UINT16(NC_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
		pu8_Buf[(u32_i<<1)+u32_BufPos+1] = (U8)(u16_Tmp>>8);
	}

	#if 0
	if (u32_ByteCnt - (u32_i<<1)) {
		REG_READ_UINT16(NC_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
	}
	#endif
}

U32 NC_ReadID(void)
{
	NAND_DRIVER     *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_CLK_SETTING(pNandDrv->u32_Clk);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ADR_C2T1S0 << 8) | CMD_0x90);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_RAN_DIN);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, NAND_ID_BYTE_CNT+1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);	/*offset 0*/

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_CIFD_ACCESS | BIT_NC_JOB_START);

	if (NC_WaitComplete(BIT_NC_JOB_END, DELAY_10ms_in_us) == DELAY_10ms_in_us)
	{
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	NC_GetCIFD(pNandDrv->au8_ID, 0 ,NAND_ID_BYTE_CNT+1);

	NC_CLR_DDR_MODE();
	return UNFD_ST_SUCCESS;	// ok
}
#endif

#endif // NC_SEL_FCIE3
