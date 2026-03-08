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
#ifndef PUTK
#define print_err(Err)
#endif

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

#define NAND_TIMEOUT_RETRY_CNT     5
#define OPTYPE_ERASE        1
#define OPTYPE_WRITE        2

//========================================================
// HAL misc. function definitions
//========================================================
#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5

U32 NC_ConfigContext(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	if (pNandDrv->u16_ECCType < ECC_TYPE_24BIT1KB)
		pNandDrv->u16_SectorByteCnt = 0x200;
	else
		pNandDrv->u16_SectorByteCnt = 0x400;

	pNandDrv->u8_BlkPageCntBits = drvNAND_CountBits(pNandDrv->u16_BlkPageCnt);
	pNandDrv->u8_PageByteCntBits = drvNAND_CountBits(pNandDrv->u16_PageByteCnt);
	//pNandDrv->u8_SpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SpareByteCnt);
	pNandDrv->u8_SectorByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorByteCnt);

	pNandDrv->u16_BlkPageCntMask = (1<<pNandDrv->u8_BlkPageCntBits)-1;
	//pNandDrv->u16_PageByteCntMask = (1<<pNandDrv->u8_PageByteCntBits)-1;
	//pNandDrv->u16_SectorByteCntMask = (1<<pNandDrv->u8_SectorByteCntBits)-1;
	//pNandDrv->u16_SpareByteCntMask = (1<<pNandDrv->u8_SpareByteCntBits)-1;

	pNandDrv->u16_PageSectorCnt = pNandDrv->u16_PageByteCnt >> pNandDrv->u8_SectorByteCntBits;
	pNandDrv->u8_PageSectorCntBits = drvNAND_CountBits(pNandDrv->u16_PageSectorCnt);
	//pNandDrv->u16_PageSectorCntMask = (1<<pNandDrv->u8_PageSectorCntBits)-1;
	pNandDrv->u16_SectorSpareByteCnt = pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits;
	pNandDrv->u16_SectorSpareByteCnt &= ~1;
	pNandDrv->u16_SpareByteCnt = pNandDrv->u16_SectorSpareByteCnt << pNandDrv->u8_PageSectorCntBits;
	//pNandDrv->u8_SectorSpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorSpareByteCnt);

	//pNandDrv->u8_BlkSectorCntBits = pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageSectorCntBits;
	//pNandDrv->u16_BlkSectorCnt = 1<<pNandDrv->u8_BlkSectorCntBits;
	//pNandDrv->u16_BlkSectorCntMask = pNandDrv->u16_BlkSectorCnt-1;

	return UNFD_ST_SUCCESS;
}


U32 NC_Init(void)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();

	// disable NC
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_NC_EN);
	REG_WRITE_UINT16(NC_CTRL , 0);

	// reset NC
	#if 0
	u32_RetVal = NC_ResetFCIE();
	if (UNFD_ST_SUCCESS != u32_RetVal)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_Init, ErrCode:%Xh \r\n", UNFD_ST_ERR_NO_NFIE);
		return u32_RetVal;
	}
    #endif
    // disable interupts
    //REG_CLR_BITS_UINT16(NC_MIE_INT_EN, BIT_MMA_DATA_END | BIT_NC_JOB_END);
    // clean int events
    REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
    
    // enable NC
    REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_NC_EN);
    
    pNandDrv->u16_Reg40_Signal =
        (BIT_NC_CE_AUTO|BIT_NC_CE_H|BIT_NC_WP_AUTO|BIT_NC_WP_H) &
        ~(BIT_NC_CHK_RB_EDGEn | BIT_NC_CE_SEL_MASK);
    //REG_WRITE_UINT16(NC_SIGNAL, pNandDrv->u16_Reg40_Signal);
    pNandDrv->u16_Reg48_Spare = (pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits);

    
    #if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
    pNandDrv->u16_Reg48_Spare |= BIT_SPARE_ADR_AUTO_INC;
	#endif
        
    pNandDrv->u16_Reg49_SpareSize = pNandDrv->u16_SpareByteCnt;
    
    pNandDrv->u16_Reg50_EccCtrl = REG50_ECC_CTRL_INIT_VALUE;
    //if (pNandDrv->u8_WordMode)
    //  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_WORD_MODE;
    nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1 ,"UNFD, WordMode:%X\r\n", pNandDrv->u8_WordMode);
    
    switch (pNandDrv->u16_PageByteCnt)
    {
        //case 0x0200:      //not support 512 byte page nand flash
        //  pNandDrv->u16_Reg50_EccCtrl &= ~BIT_NC_PAGE_SIZE_512Bn;
        //  pNandDrv->u16_Reg48_Spare |= BIT_NC_ONE_COL_ADDR;
        //  break;
        case 0x0800:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_2KB;  break;
        case 0x1000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_4KB;  break;
        case 0x2000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_8KB;  break;
        case 0x4000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_16KB;  break;
        case 0x8000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_32KB;  break;
        default:
            nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: invalid Page Size: %Xh bytes!\r\n", pNandDrv->u16_PageByteCnt);
            return UNFD_ST_ERR_INVALID_PARAM;
    }
    
    switch (pNandDrv->u16_ECCType)
    {
        case ECC_TYPE_4BIT:
            //FCIE5 not support 4bit ecc -> using 8 bit ecc instead. for NNI compatible.
        case ECC_TYPE_8BIT:
            pNandDrv->u16_Reg50_EccCtrl &= ~BIT_NC_ECC_TYPE_8b512Bn;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_8BIT;
            if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 4;
            break;
        case ECC_TYPE_16BIT:
            pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_16b512B;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_16BIT;
            if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 8;
            break;
            #if 0
        case ECC_TYPE_24BIT:
            pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_24b512B;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_24BIT;
            break;
        case ECC_TYPE_24BIT1KB:
            pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_24b1KB;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_24BIT1KB;
            break;
            #endif
        case ECC_TYPE_40BIT1KB:
            pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_40b1KB;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_40BIT1KB;
            //pNandDrv->u16_ECCCorretableBit = 40;
            if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 20;
            break;
        case ECC_TYPE_60BIT1KB:
            pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_60b1KB;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_60BIT1KB;
            //pNandDrv->u16_ECCCorretableBit = 60;
            if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 30;
            break;
        case ECC_TYPE_72BIT1KB:
            pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_72b1KB;
            pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_72BIT1KB;
            if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 36;
            //pNandDrv->u16_ECCCorretableBit = 72;
            break;  
        default:
            nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: invalid ECC Type: %Xh \r\n", pNandDrv->u16_ECCType);
            return UNFD_ST_ERR_INVALID_PARAM;
    }
    pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECCERR_NSTOP;

    NC_Config();
    
    return UNFD_ST_SUCCESS;
}


void NC_Config(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	REG_WRITE_UINT16(NC_SIGNAL, pNandDrv->u16_Reg40_Signal);
	/*sector spare size*/
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);
	/* page spare size*/
	REG_WRITE_UINT16(NC_SPARE_SIZE, (U16)pNandDrv->u16_Reg49_SpareSize);
	/* page size and ECC type*/
	REG_WRITE_UINT16(NC_ECC_CTRL, pNandDrv->u16_Reg50_EccCtrl);

	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	#if defined(ENABLE_TOGGLE_MODE_SUPPORT) && ENABLE_TOGGLE_MODE_SUPPORT
	#if (defined(ENABLE_8BIT_MACRO) && ENABLE_8BIT_MACRO)
	REG_WRITE_UINT16(REG_EMMC_PLL_RX6C, pNandDrv->u16_Emmc_Pll_Reg6C);
	#elif (defined(ENABLE_32BIT_MACRO) && ENABLE_32BIT_MACRO)
	REG_WRITE_UINT16(REG_EMMC_PLL_RX09, pNandDrv->u16_Emmc_Pll_Reg09);
	#endif

	// mark the following line, to save power (disable CLK) for ONFI-DDR
	//REG_WRITE_UINT16(NC_DDR_CTRL, pNandDrv->u16_Reg58_DDRCtrl);
	//
	/*Delay for auto random commmad*/
	REG_SET_BITS_UINT16(NC_RAND_W_CMD, pNandDrv->u16_Reg56_HWCmdDelay & 0xFF00);

	/*set timing for nand tRR, tCS*/
	REG_SET_BITS_UINT16(NC_LFSR_CTRL, pNandDrv->u16_Reg59_TrrTcs & 0x00FF);

	/*set timing for tCLHZ*/
	REG_WRITE_UINT16(NC_NAND_TIMING, pNandDrv->u16_Reg5A_tWHR_tCLHZ);
	#endif

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_WRITE_UINT16(NC_LFSR_CTRL, pNandDrv->u16_Reg59_LFSRCtrl);
	#endif
}


#if defined(FCIE_LFSR) && FCIE_LFSR
U32 NC_EnableLFSR(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	volatile U16 u16_RegVal;

	if(0 == IF_LFSR_ENABLED())
	{
		// tune timing: RE_LATCH
		if(0 == (pNandDrv->u16_Reg58_DDRCtrl & BIT_DDR_MASM))
		{
			REG_READ_UINT16(NC_LATCH_DATA, u16_RegVal);
			u16_RegVal = u16_RegVal & BIT_NC_LATCH_DATA_MASK;
            #if 0
			if(u16_RegVal > BIT_NC_LATCH_DATA_6_0_T) 
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error, RD_LATCH already > 1T, can NOT use LFSR.\n");
				return UNFD_ST_ERR_LFSR_RD_LATCH;
			}
            #endif
			pNandDrv->u16_Reg57_RELatch &= ~BIT_NC_LATCH_DATA_MASK;
			pNandDrv->u16_Reg57_RELatch += u16_RegVal + BIT_NC_LATCH_DATA_1_0_T;
		}

		#if defined(FCIE5_DDR) && FCIE5_DDR
		// tune timing: DDR_TIMING
		REG_READ_UINT16(NC_LATCH_DATA, u16_RegVal);
		u16_RegVal = u16_RegVal & BIT_RE_DDR_TIMING_MASK;
		if(u16_RegVal == BIT_RE_DDR_TIMING_MASK) 
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error, DDR_TIMING full, can NOT use LFSR.\n");
			return UNFD_ST_ERR_LFSR_DDRTIMING;
		}
		pNandDrv->u16_Reg57_RELatch += 1<<BIT_RE_DDR_TIMING_SHIFT;
        // walk around timing bug
		pNandDrv->u16_Reg57_RELatch &= ~BIT_RE_SEC_TURN_CNT_MASK;
		pNandDrv->u16_Reg57_RELatch |= 0xE << BIT_RE_SEC_TURN_CNT_SHIFT;
		#endif		

		REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

		// set LFST
		pNandDrv->u16_Reg59_LFSRCtrl |= BIT_LFSR_ENABLE;
		REG_SET_BITS_UINT16(NC_LFSR_CTRL, BIT_LFSR_ENABLE);
	}

	return UNFD_ST_SUCCESS;
}

U32 NC_DisableLFSR(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	volatile U16 u16_RegVal;

	REG_READ_UINT16(NC_LFSR_CTRL, u16_RegVal);

	if(IF_LFSR_ENABLED())
	{
		if(0 == (pNandDrv->u16_Reg58_DDRCtrl & BIT_DDR_MASM))
			pNandDrv->u16_Reg57_RELatch -= BIT_NC_LATCH_DATA_1_0_T;

		#if defined(FCIE5_DDR) && FCIE5_DDR
		pNandDrv->u16_Reg57_RELatch -= 1<<BIT_RE_DDR_TIMING_SHIFT;
		#endif

		REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

		// clear LFST
		pNandDrv->u16_Reg59_LFSRCtrl &= ~BIT_LFSR_ENABLE;
		REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_LFSR_ENABLE);
	}
	return UNFD_ST_SUCCESS;
}
#endif


U32  NC_WaitComplete(U32 u32_EventReg, U16 u16_WaitEvent, U32 u32_MicroSec);
U32 NC_WaitGetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt);

void NC_GetCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt)
{
	U32 u32_i, u32_BufPos;
	U16 u16_Tmp;

	if (u32_CIFDPos & 1) {
		REG_READ_UINT16(NC_WBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		pu8_Buf[0] = (U8)(u16_Tmp >> 8);
		u32_CIFDPos += 1;
		u32_ByteCnt -= 1;
		u32_BufPos = 1;
	} else
		u32_BufPos = 0;

	for (u32_i=0; u32_i<u32_ByteCnt>>1; u32_i++) {
		REG_READ_UINT16(NC_WBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
		pu8_Buf[(u32_i<<1)+u32_BufPos+1] = (U8)(u16_Tmp>>8);
	}

	if (u32_ByteCnt - (u32_i<<1)) {
		REG_READ_UINT16(NC_WBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
	}

}

U32 NC_WaitGetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt)
{
	if(u32_ByteCnt > NC_CIFD_WBUF_BYTE_CNT)
	{
		return UNFD_ST_ERR_INVAL_CIFD_CNT;
	}
	if (NC_WaitComplete(NC_CIFD_EVENT, BIT_WBUF_FULL, WAIT_READ_TIME)== (WAIT_READ_TIME ))
	{
		REG_WRITE_UINT16(NC_PART_MODE, 0);
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_R_TIMEOUT_RM; // timeout
	}
	NC_GetCIFD(pu8_DataBuf , 0, u32_ByteCnt);

	REG_WRITE_UINT16(NC_CIFD_EVENT,BIT_WBUF_FULL);
	
	REG_WRITE_UINT16(NC_CIFD_EVENT, BIT_WBUF_EMPTY_TRI);

	return UNFD_ST_SUCCESS;
}


U32 NC_RegInit(void)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();

	// config NC
	pNandDrv->u16_Reg48_Spare = (pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits);
	if (NC_MAX_SECTOR_SPARE_BYTE_CNT < pNandDrv->u16_Reg48_Spare || (pNandDrv->u16_Reg48_Spare & 3))
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: invalid Sector Spare Size: %Xh bytes!\r\n", pNandDrv->u16_Reg48_Spare);
		return UNFD_ST_ERR_INVALID_PARAM;
	}

	#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
	pNandDrv->u16_Reg48_Spare |= BIT_SPARE_ADR_AUTO_INC;
	#endif

	pNandDrv->u16_Reg49_SpareSize = pNandDrv->u16_SpareByteCnt;
	if (NC_MAX_TOTAL_SPARE_BYTE_CNT < pNandDrv->u16_Reg49_SpareSize || (pNandDrv->u16_Reg49_SpareSize & 3))
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: invalid Total Spare Size: %Xh bytes!\r\n", pNandDrv->u16_Reg49_SpareSize);
		return UNFD_ST_ERR_INVALID_PARAM;
	}

	pNandDrv->u16_Reg50_EccCtrl = REG50_ECC_CTRL_INIT_VALUE;
	if (pNandDrv->u8_WordMode)
		pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_WORD_MODE;
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1 ,"UNFD, WordMode:%X\r\n", pNandDrv->u8_WordMode);

	switch (pNandDrv->u16_PageByteCnt)
	{
		case 0x0200:
			pNandDrv->u16_Reg50_EccCtrl &= ~BIT_NC_PAGE_SIZE_512Bn;
			pNandDrv->u16_Reg48_Spare |= BIT_NC_ONE_COL_ADDR;
			break;
		case 0x0800:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_2KB;  break;
		case 0x1000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_4KB;  break;
		case 0x2000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_8KB;  break;
		case 0x4000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_16KB;  break;
		case 0x8000:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_32KB;  break;
		default:
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: invalid Page Size: %Xh bytes!\r\n", pNandDrv->u16_PageByteCnt);
			return UNFD_ST_ERR_INVALID_PARAM;
	}

	switch (pNandDrv->u16_ECCType)
	{
		case ECC_TYPE_4BIT:
			//FCIE5 not support 4bit ecc -> using 8 bit ecc instead. for NNI compatible.	
		case ECC_TYPE_8BIT:
			pNandDrv->u16_Reg50_EccCtrl &= ~BIT_NC_ECC_TYPE_8b512Bn;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_8BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 4;
			pNandDrv->u16_ECCCorretableBit = 8;
			break;
		case ECC_TYPE_16BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_16b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_16BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 8;
			pNandDrv->u16_ECCCorretableBit = 16;
			break;
		case ECC_TYPE_24BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_24b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_24BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 12;
			pNandDrv->u16_ECCCorretableBit = 24;
			break;
		case ECC_TYPE_32BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_32b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_32BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 16;
			pNandDrv->u16_ECCCorretableBit = 32;
			break;
		case ECC_TYPE_40BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_40b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_40BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 20;
			pNandDrv->u16_ECCCorretableBit = 40;
			break;
		case ECC_TYPE_60BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_60b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_60BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 30;
			pNandDrv->u16_ECCCorretableBit = 60;
			break;
		case ECC_TYPE_72BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_72b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_72BIT;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 36;
			pNandDrv->u16_ECCCorretableBit = 72;
			break;
		case ECC_TYPE_8BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_8b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_8BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 4;
			pNandDrv->u16_ECCCorretableBit = 8;
			break;
		case ECC_TYPE_16BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_16b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_16BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 8;
			pNandDrv->u16_ECCCorretableBit = 16;
			break;
		case ECC_TYPE_24BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_24b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_24BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 12;
			pNandDrv->u16_ECCCorretableBit = 24;
			break;
		case ECC_TYPE_32BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_32b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_32BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 16;
			pNandDrv->u16_ECCCorretableBit = 32;
			break;
		case ECC_TYPE_40BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_40b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_40BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 20;
			pNandDrv->u16_ECCCorretableBit = 40;
			break;
		case ECC_TYPE_60BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_60b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_60BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 30;
			pNandDrv->u16_ECCCorretableBit = 60;
			break;
		case ECC_TYPE_72BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_72b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_72BIT1KB;
			if(!pNandDrv->u16_BitflipThreshold)
				pNandDrv->u16_BitflipThreshold = 36;
			pNandDrv->u16_ECCCorretableBit = 72;
			break;
		default:
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: invalid ECC Type: %Xh \r\n", pNandDrv->u16_ECCType);
			return UNFD_ST_ERR_INVALID_PARAM;
	}
	pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECCERR_NSTOP;
	return UNFD_ST_SUCCESS;
}


U32 NC_ConfigNandFlashContext(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	if (pNandDrv->u16_ECCType < ECC_TYPE_24BIT1KB || 
		pNandDrv->u16_ECCType == ECC_TYPE_32BIT || 
		pNandDrv->u16_ECCType == ECC_TYPE_40BIT || 
		pNandDrv->u16_ECCType == ECC_TYPE_60BIT ||
		pNandDrv->u16_ECCType == ECC_TYPE_72BIT)
		pNandDrv->u16_SectorByteCnt = 0x200;
	else
		pNandDrv->u16_SectorByteCnt = 0x400;

	pNandDrv->u8_BlkPageCntBits = drvNAND_CountBits(pNandDrv->u16_BlkPageCnt);
	pNandDrv->u8_PageByteCntBits = drvNAND_CountBits(pNandDrv->u16_PageByteCnt);
	//pNandDrv->u8_SpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SpareByteCnt);
	pNandDrv->u8_SectorByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorByteCnt);

	//pNandDrv->u16_BlkPageCntMask = (1<<pNandDrv->u8_BlkPageCntBits)-1;
	//pNandDrv->u16_PageByteCntMask = (1<<pNandDrv->u8_PageByteCntBits)-1;
	//pNandDrv->u16_SectorByteCntMask = (1<<pNandDrv->u8_SectorByteCntBits)-1;
	//pNandDrv->u16_SpareByteCntMask = (1<<pNandDrv->u8_SpareByteCntBits)-1;

	pNandDrv->u16_PageSectorCnt = pNandDrv->u16_PageByteCnt >> pNandDrv->u8_SectorByteCntBits;
	pNandDrv->u8_PageSectorCntBits = drvNAND_CountBits(pNandDrv->u16_PageSectorCnt);
	//pNandDrv->u16_PageSectorCntMask = (1<<pNandDrv->u8_PageSectorCntBits)-1;
	pNandDrv->u16_SectorSpareByteCnt = pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits;
	pNandDrv->u16_SectorSpareByteCnt &= ~1;
	pNandDrv->u16_SpareByteCnt = pNandDrv->u16_SectorSpareByteCnt << pNandDrv->u8_PageSectorCntBits;
	//pNandDrv->u8_SectorSpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorSpareByteCnt);

	//pNandDrv->u8_BlkSectorCntBits = pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageSectorCntBits;
	//pNandDrv->u16_BlkSectorCnt = 1<<pNandDrv->u8_BlkSectorCntBits;
	//pNandDrv->u16_BlkSectorCntMask = pNandDrv->u16_BlkSectorCnt-1;

	return UNFD_ST_SUCCESS;
}


U32  NC_GetECCBits(void)
{
	U16 u16_Tmp;

	REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
	if (u16_Tmp & BIT_NC_ECC_FAIL)
		return(U32)(0-1);

	return(u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK) >> 1;
}

U32  NC_ReadSectors(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_SectorCnt)
{
	U16 u16_Tmp;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32	u32_DataDMAAddr, U32_SpareDMAAddr;

	// HW setting
	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_CLK_SETTING(pNandDrv->u32_Clk);
	NC_RECONFIG();
	NC_SET_DDR_MODE(); // to turn on ONFI clk
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

//	u32_DataDMAAddr = ((U32)pu8_DataBuf)&0x1FFFFFFF;
//	U32_SpareDMAAddr = ((U32)pu8_SpareBuf)&0x1FFFFFFF;

	u32_DataDMAAddr =  nand_translate_DMA_address_Ex((U32)pu8_DataBuf, pNandDrv->u16_SectorByteCnt*u32_SectorCnt);
	U32_SpareDMAAddr =  nand_translate_DMA_address_Ex((U32)pu8_SpareBuf, pNandDrv->u16_SpareByteCnt);
	
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR0, (U32)U32_SpareDMAAddr & 0xFFFF);
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR1, (U32)U32_SpareDMAAddr >>16);

	REG_WRITE_UINT16(NC_PART_MODE, (BIT_PARTIAL_MODE_EN)|((u32_SectorCnt - 1) << BIT_START_SECTOR_CNT_SHIFT)|(u8_SectorInPage << BIT_START_SECTOR_IDX_SHIFT));
	
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, (U32)u32_DataDMAAddr & 0xFFFF);
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, (U32)u32_DataDMAAddr >> 16);

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

	REG_WRITE_UINT16(NC_TOTAL_SEC_NUM, u32_SectorCnt-1);
	
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, WAIT_READ_TIME) == (WAIT_READ_TIME))
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_ReadSectors Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_R_TIMEOUT);
		REG_WRITE_UINT16(NC_PART_MODE, 0);
		print_err(UNFD_ST_ERR_R_TIMEOUT);
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	// Check ECC
	REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
	if (u16_Tmp & BIT_NC_ECC_FAIL)
	{
		NC_CLR_DDR_MODE();
		REG_WRITE_UINT16(NC_PART_MODE, 0);
		print_err(UNFD_ST_ERR_ECC_FAIL);
		return UNFD_ST_ERR_ECC_FAIL;
	}
	else
	{
        #if defined(DEBUG_REG_ECC_STATUS)
        u16_Tmp = ((u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK) >> 1);
        if(u16_Tmp > pNandDrv->u8_BitflipThreshold)
        {
            pNandDrv->u8_BitflipThreshold = u16_Tmp;
            u16_Tmp = ((u16_Tmp<<8)&0xFF00) | ((U16)(u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits)&0x00FF);
            REG_WRITE_UINT16(DEBUG_REG_ECC_STATUS, u16_Tmp);
        }
        #endif
	}

	REG_WRITE_UINT16(NC_PART_MODE, 0);

	NC_CLR_DDR_MODE();
	return UNFD_ST_SUCCESS;
}


U32  NC_ReadSector_RIUMode(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf )
{
	volatile U16 u16_Tmp, u16_i, u16_j, u16_k, u16_Cnt, u16_ByteLoc;
	U16 u16_Col;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_Tmp, u32_Err;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_CLK_SETTING(pNandDrv->u32_Clk);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN); // enable RIU Mode
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	
	// if Partial Mode (if not 512B/page, set Partial Mode)	
	REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN|(u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT));
	
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

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	//read for data
	u16_Cnt = pNandDrv->u16_SectorByteCnt >> 6; // how many 64B per sector
	u16_Cnt = (pNandDrv->u16_SectorByteCnt & (0x40 -1)) ? u16_Cnt + 1 : u16_Cnt;
	for (u16_j=0; u16_j<u16_Cnt; u16_j++)
	{   // read data
		u16_Tmp =( (pNandDrv->u16_SectorByteCnt - (u16_j << 6)) >= 0x40) ? 
			0x40 : (pNandDrv->u16_SectorByteCnt - (u16_j << 6) );
		u32_Err = NC_WaitGetCIFD((U8*)(pu8_DataBuf + (u16_j << 6)), u16_Tmp);
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_CLR_DDR_MODE();
			return u32_Err;
		}		
	}
	
	//read for spare
	u16_Cnt = pNandDrv->u16_SectorSpareByteCnt >> 6; // how many 64B per sector
	u16_Cnt = (pNandDrv->u16_SectorSpareByteCnt & (0x40 -1)) ? u16_Cnt + 1 : u16_Cnt;
	for (u16_j=0; u16_j<u16_Cnt; u16_j++)
	{   // read data
		u16_Tmp =( (pNandDrv->u16_SectorSpareByteCnt - (u16_j << 6)) >= 0x40) ? 
			0x40: (pNandDrv->u16_SectorSpareByteCnt - (u16_j << 6));
		u32_Err = NC_WaitGetCIFD( (U8*)(pu8_SpareBuf + (u16_j << 6)), u16_Tmp);
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_CLR_DDR_MODE();
			return u32_Err;
		}		
	}

	// wait for events
	u32_Tmp = 0;
	do{
		u16_Tmp = 0; // do not remove this line
		REG_READ_UINT16(NC_MIE_EVENT, u16_Tmp);
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
		u32_Tmp++;
		if(u32_Tmp > WAIT_READ_TIME)
  		{
			REG_WRITE_UINT16(NC_PART_MODE, 0);						
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_R_TIMEOUT_RM;
		}
	}while(((u16_Tmp & BIT_NC_R2N_ECCCOR)==0) && ((u16_Tmp & (BIT_NC_JOB_END )) != (BIT_NC_JOB_END )));

    // if ECC Fail
	REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
	if ((u16_Tmp & BIT_NC_ECC_FLGA_MASK) == BIT_NC_ECC_FLGA_FAIL)
	{
		REG_WRITE_UINT16(NC_PART_MODE, 0);				
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);	
		return UNFD_ST_ERR_ECC_FAIL_RM;
	}

	REG_READ_UINT16(NC_MIE_EVENT, u16_Tmp);
	if (u16_Tmp & BIT_NC_R2N_ECCCOR)
	{
//		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "RIU Mode, ECC happens: ");
		// make CIFD available for SW
		// [CAUTION]: before clear BIT_NC_R2N_ECC, JOB_END would not set.
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_R2N_ECCCOR);
		if(NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, WAIT_READ_TIME) == WAIT_READ_TIME)
		{
			REG_WRITE_UINT16(NC_PART_MODE, 0);			
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);			
			NC_CLR_DDR_MODE();
		}

		REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
		u16_Cnt = (u16_Tmp & BIT_NC_ECC_CNT_MASK) >> BIT_NC_ECC_CNT_SHIFT;
//		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "%u bits\r\n", u16_Cnt);
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
//			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "bit loc: %02Xh \r\n", u16_j);
			u16_ByteLoc = u16_j >> 3;
			if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt)
			{
				pu8_DataBuf[u16_ByteLoc] ^= 1<<(u16_j%8);
			}
			else if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt +
			       pNandDrv->u16_SectorSpareByteCnt - pNandDrv->u16_ECCCodeByteCnt)
			{
				pu8_SpareBuf[u16_ByteLoc-pNandDrv->u16_SectorByteCnt] ^= 1<<(u16_j%8);
			}
		}

        #if defined(DEBUG_REG_ECC_STATUS)
        if(u16_Cnt > pNandDrv->u8_BitflipThreshold)
        {
            pNandDrv->u8_BitflipThreshold = u16_Cnt;
            u16_Cnt = ((u16_Cnt<<8)&0xFF00) | ((U16)(u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits)&0x00FF);
            REG_WRITE_UINT16(DEBUG_REG_ECC_STATUS, u16_Cnt);
        }
        #endif
    	//u16_Tmp = BIT_NC_R2N_ECCCOR;
  	}

	else if(0 == (u16_Tmp & BIT_NC_R2N_ECCCOR)){
		if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, WAIT_READ_TIME) == WAIT_READ_TIME)
		{
			//U16 u16_Reg;
			//REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
			REG_WRITE_UINT16(NC_PART_MODE, 0);
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			return UNFD_ST_ERR_R_TIMEOUT_RM; // timeout
		}
	}

	NC_CLR_DDR_MODE();
  	REG_WRITE_UINT16(NC_PART_MODE, 0);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	return UNFD_ST_SUCCESS;
}


U32  NC_WriteSectors(     U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_SectorCnt )
{
	U16 u16_Tmp;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_DataDMAAddr = 0, u32_SpareDMAAddr;
    U32 u32_Ret;

    //nand_debug(0,0,"ws b:%Xh p:%Xh \n", u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
    //    u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask);

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();
	
    //if(pNandDrv->u8_MaxCENum)
    //    u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL,
		((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) & BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	u32_DataDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_DataBuf, pNandDrv->u16_SectorByteCnt);
    #if 0
	if(pu8_SpareBuf == NULL)
	{
		memset(pNandDrv->pu8_PageSpareBuf, 0xff,  pNandDrv->u16_SpareByteCnt);
		u32_SpareDMAAddr = nand_translate_Spare_DMA_address_Ex((U32)pNandDrv->pu8_PageSpareBuf, pNandDrv->u16_SectorSpareByteCnt, WRITE_TO_NAND);		
	}
	else
    #endif
		u32_SpareDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_SpareBuf, pNandDrv->u16_SectorSpareByteCnt);

	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR0, u32_SpareDMAAddr & 0xFFFF);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR1, u32_SpareDMAAddr >>16);

	// if Partial Mode (if not 512B/page, set Partial Mode)
	REG_READ_UINT16(NC_ECC_CTRL, u16_Tmp);
	if (u16_Tmp & BIT_NC_PAGE_SIZE_MASK)
	{
		REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN);
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_CNT_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, (u32_SectorCnt-1)<<BIT_START_SECTOR_CNT_SHIFT);
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT);
	}
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, u32_DataDMAAddr & 0xFFFF);//>>MIU_BUS_WIDTH_BITS));
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, u32_DataDMAAddr >> 16);//(MIU_BUS_WIDTH_BITS+16)));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if(0 == pNandDrv->u8_WordMode)
		REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage << pNandDrv->u8_SectorByteCntBits);
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage << (pNandDrv->u8_SectorByteCntBits-1));
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx >> 16);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle<< 8) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_0x10 << 8) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_0x70 << 8)| ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8)| ACT_CHKSTATUS);

	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	//while(1)  nand_reset_WatchDog();
	if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_WriteSectors Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		REG_WRITE_UINT16(NC_PART_MODE, 0);
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	NC_CLR_DDR_MODE();
	REG_WRITE_UINT16(NC_PART_MODE, 0);

    u32_Ret = NC_CheckEWStatus(OPTYPE_WRITE);
    if(UNFD_ST_SUCCESS != u32_Ret)
        nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"write blk:%Xh p:%Xh fail:%Xh \n", 
            u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
            u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask, u32_Ret);
    
    return u32_Ret;
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
U32 NC_ReadID(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_CLK_SETTING(pNandDrv->u32_Clk);
	NC_RECONFIG();
	NC_SET_DDR_MODE();
	
	// data go through CIFD
	REG_SET_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ADR_C2T1S0 << 8) | CMD_0x90);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_RAN_DIN);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, NAND_ID_BYTE_CNT+1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); /*offset 0*/

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	if(NC_WaitGetCIFD(pNandDrv->au8_ID, NAND_ID_BYTE_CNT+1) != UNFD_ST_SUCCESS)
	{
		NC_CLR_DDR_MODE();
		REG_CLR_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, DELAY_100ms_in_us) == DELAY_100ms_in_us)
	{
		NC_CLR_DDR_MODE();
		REG_CLR_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	NC_CLR_DDR_MODE();
	REG_CLR_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);
	return UNFD_ST_SUCCESS; // ok
}
#endif

U32 NC_WritePages(    U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_PageCnt )
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_DataDMAAddr;
	U32 u32_SpareDMAAddr=0;
	U32 u32_Ret;
    #if 0//CHECK_HAL_PARAM
	if ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) + u32_PageCnt > pNandDrv->u16_BlkPageCnt)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_WritePages, ErrCode:%Xh \r\n", UNFD_ST_ERR_HAL_W_INVALID_PARAM);
		return UNFD_ST_ERR_HAL_W_INVALID_PARAM;
	}
    #endif
	NC_RECONFIG();
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL,
	  	((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) & BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif
	u32_DataDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_DataBuf, pNandDrv->u16_PageByteCnt * u32_PageCnt);
	u32_SpareDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_SpareBuf, pNandDrv->u16_SpareByteCnt);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR0, u32_SpareDMAAddr & 0xFFFF);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR1, u32_SpareDMAAddr >>16);
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, u32_DataDMAAddr & 0xFFFF);//>>MIU_BUS_WIDTH_BITS));
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, u32_DataDMAAddr >> 16);//(MIU_BUS_WIDTH_BITS+16)));
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx >> 16);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle<< 8) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_0x10 << 8) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_0x70 << 8) | ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_REPEAT << 8) | ACT_CHKSTATUS);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RPTCNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PageCnt - 1);
	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, WAIT_WRITE_TIME*u32_PageCnt) == WAIT_WRITE_TIME*u32_PageCnt)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_WritePages Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_RECONFIG();
		//NC_ResetNandFlash();
		#else
		#endif
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}
    #if defined(FCIE_WRITE_PROTECT_SUPPORT) && FCIE_WRITE_PROTECT_SUPPORT
	if((REG(NC_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT)
	{
		U32 u32_SecureRow;
		nand_debug(0, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		u32_SecureRow = (REG(NC_SECURE_ERR_ADDR_H) << 16)|(REG(NC_SECURE_ERR_ADDR_L) & 0xFFFF);
		if(u32_SecureRow != u32_PhyRowIdx)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Alert Address 0x%X!= 0x%X\n", u32_SecureRow, u32_PhyRowIdx);
		}					
		NC_RECONFIG();
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_SECURE_ALERT);
		u32_Ret = UNFD_ST_ERR_W_PROTECTED;
	}
	else
    #endif	
	u32_Ret = NC_CheckEWStatus(OPTYPE_WRITE);
    if(UNFD_ST_SUCCESS != u32_Ret)
    {
        LDR_PUTS("write err: \n"); 
        LDR_PUTDW(u32_Ret); LDR_PUTS("\n");
        nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"write blk:%Xh p:%Xh fail:%Xh \n", 
            u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
            u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask, u32_Ret);
    }
	return u32_Ret;
}


U32 NC_EraseBlk(U32 u32_PhyRowIdx)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_Ret;
	NC_RECONFIG();
    REG_CLR_BITS_UINT16(NC_SIGNAL, BIT5);
    REG_SET_BITS_UINT16(NC_SIGNAL, BIT4);
	REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx >> 16);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_Erase_AdrCycle << 8) | CMD_0x60);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0xD0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_CHKSTATUS << 8) | CMD_0x70);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
    nand_CheckPowerCut();
	#endif
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	if (NC_WaitComplete(NC_MIE_EVENT, BIT_NC_JOB_END, WAIT_ERASE_TIME) == WAIT_ERASE_TIME)
	{
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_EraseBlk Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_E_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_RECONFIG();
		//NC_ResetNandFlash();
		#else
		#endif
		return UNFD_ST_ERR_E_TIMEOUT;
	}
    #if defined(FCIE_WRITE_PROTECT_SUPPORT) && FCIE_WRITE_PROTECT_SUPPORT
	if((REG(NC_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT)
	{
		U32 u32_SecureRow;
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		u32_SecureRow = (REG(NC_SECURE_ERR_ADDR_H) << 16)|(REG(NC_SECURE_ERR_ADDR_L) & 0xFFFF);
		if(u32_SecureRow != u32_PhyRowIdx)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Alert Address 0x%X!= 0x%X\n", u32_SecureRow, u32_PhyRowIdx);
		}
		NC_RECONFIG();
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_SECURE_ALERT);
		u32_Ret = UNFD_ST_ERR_W_PROTECTED;
	}
	else		
    #endif
    u32_Ret = NC_CheckEWStatus(OPTYPE_ERASE);
    if(UNFD_ST_SUCCESS != u32_Ret)
    {
        LDR_PUTS("erase err: \n"); 
        LDR_PUTDW(u32_Ret); LDR_PUTS("\n");
        nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"erase blk:%Xh fail:%Xh \n", 
            u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, u32_Ret);
    }
	return u32_Ret;
}

U32 NC_CheckEWStatus(U8 u8_OpType)
{
	volatile U16 u16_Tmp;
	U32 u32_ErrCode = UNFD_ST_SUCCESS;
	REG_READ_UINT16(NC_ST_READ, u16_Tmp);
	if ((u16_Tmp & BIT_ST_READ_FAIL) == 1) { // if fail
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_FAIL;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_FAIL;
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_CheckEWStatus Fail, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	} else if ((u16_Tmp & BIT_ST_READ_BUSYn) == 0) { // if busy
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_BUSY;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_BUSY;
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_CheckEWStatus Busy, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	} else if ((u16_Tmp & BIT_ST_READ_PROTECTn) == 0) { // if protected
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_PROTECTED;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_PROTECTED;
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_CheckEWStatus Protected, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	}
	return u32_ErrCode;
}

#endif // NC_SEL_FCIE5
