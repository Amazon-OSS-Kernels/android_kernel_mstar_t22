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

#if (defined(NC_SEL_FCIE5) && (NC_SEL_FCIE5) && defined(FCIE_REG_TEE_BASE_ADDR)) 

#define CHECK_HAL_PARAM      1 // normal can be 0    

//========================================================
// HAL pre-processors
//========================================================
#if IF_FCIE_SHARE_PINS
  #define NC_PAD_SWITCH(enable)    nand_pads_switch(enable);
#else
    // NULL to save CPU a JMP/RET time
  #define NC_PAD_SWITCH(enable)
#endif

#if IF_FCIE_SHARE_IP
  // re-config FCIE3 for NFIE mode
  #define NC_TEE_REConfig()   {NC_TEE_ReConfig(); nand_clock_setting(pNandDrv->u32_Clk);}
#else
    // NULL to save CPU a JMP/RET time
  #define NC_TEE_REConfig()
#endif

#define NAND_TIMEOUT_RETRY_CNT     5

#if defined(NEED_FLUSH_MIU_PIPE) && NEED_FLUSH_MIU_PIPE
#define FLUSH_MIU_PIPE()    nand_flush_miu_pipe() // Only used in U4 now
#else
#define FLUSH_MIU_PIPE()
#endif
//========================================================
// HAL misc. function definitions
//========================================================
void NC_TEE_Config(void);

void NC_TEE_DumpRegisters(void);
void NC_TEE_DumpDebugBus(void);
U32  NC_TEE_CheckEWStatus(U8 u8_OpType);
U32  NC_TEE_WaitComplete(U16 u16_WaitEvent, U32 u32_MicroSec);
U32 NC_TEE_WaitCIFD_Event(U16 u16_WaitEvent, U32	u32_MicroSec);

U32 NC_TEE_WaitGetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt, U32 u32_MicroSec);
U32 NC_TEE_WaitSetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt);

  #define OPTYPE_ERASE        1
  #define OPTYPE_WRITE        2

// ============================================================
#if defined(DDR_NAND_SUPPORT) && DDR_NAND_SUPPORT

#define NAND_CMD_READ_ID                0x90
#define NAND_CMD_SET_FEATURE            0xEF
#define NAND_CMD_GET_FEATURE            0xEE
#define NAND_CMD_READ_PARAMETER         0xEC

#define NAND_ADR_READ_JEDEC             0x40
#define NAND_ADR_READ_ONFI              0x20
#define NAND_ADR_READ_PARAMETER         0x00
#define NAND_ADR_TIMING_MODE            0x01
#define NAND_ADR_INTERFACE_MODE         0x80

#define NAND_DDR_REMOVE_2NDBYTE_NA      0
#define NAND_DDR_REMOVE_2NDBYTE_TOGGLE  1
#define NAND_DDR_REMOVE_2NDBYTE_ONFI    2

static U32 NC_TEE_RandomIn_1Cmd1Addr(U8 u8_Cmd, U8 u8_Addr, U8 u8_Remove2ndByte, U8 u8_ByteCnt, U8 *pu8_DataBuf);
U32 NC_TEE_SetToggleDDRMode(void);
U32 NC_ReadToggleParamPage(void);
#endif

U32 NC_SendReadRetryCmd(U8 u8_RetryIndex, U8 u8_SetToDefault);
// ============================================================
//static U8 gu8_ReadRetryHappens = 0;

void NC_TEE_DumpDebugBus(void)
{
	volatile U16 u16_Reg, u16_i;

	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "\n");
	for(u16_i=0; u16_i<16; u16_i++)
	{
		REG_CLR_BITS_UINT16(NC_TEE_DEBUG_DBUS1, BIT_FCIE_DEBUG_MODE);
		REG_SET_BITS_UINT16(NC_TEE_DEBUG_DBUS1, u16_i << 8);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "Reg debug mode: 0x%X\n", u16_i);
		REG_READ_UINT16(NC_TEE_DEBUG_DBUS0, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_TEE_DEBUG_DBUS0: %X\n", u16_Reg);
		REG_READ_UINT16(NC_TEE_DEBUG_DBUS1, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_TEE_DEBUG_DBUS1: %X\n", u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "\n");
	}

	REG_CLR_BITS_UINT16(NC_TEE_DEBUG_DBUS1, BIT_FCIE_DEBUG_MODE);
	REG_SET_BITS_UINT16(NC_TEE_DEBUG_DBUS1, BIT0 << 8);

	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "\n");
	for(u16_i=0; u16_i<16; u16_i++)
	{
		REG_CLR_BITS_UINT16(NC_TEE_WIDTH, BIT_NC_DEB_SEL);
		REG_SET_BITS_UINT16(NC_TEE_WIDTH, u16_i<<BIT_NC_DEB_SEL_SHIFT);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "REG_NC_DEB_SEL: %X\n", u16_i);

		REG_READ_UINT16(NC_TEE_WIDTH, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_TEE_WIDTH: %X\n", u16_Reg);

		REG_READ_UINT16(NC_TEE_DEBUG_DBUS0, u16_Reg);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_TEE_DEBUG_DBUS0: %X\n", u16_Reg);

		REG_READ_UINT16(NC_TEE_DEBUG_DBUS1, u16_Reg);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_TEE_DEBUG_DBUS1: %X\n", u16_Reg);
		
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "\n");
	}
	REG_CLR_BITS_UINT16(NC_TEE_WIDTH, BIT_NC_DEB_SEL);

}


void NC_TEE_DumpRegisters(void)
{
	volatile U16 u16_Reg;
	U16 u16_idx = 0;

	//nand_DumpPadClk();
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Dump FCIE Register\n");

	REG_READ_UINT16(NC_TEE_RPT_CNT, u16_Reg);
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_RPT_CNT(0x0C) = %04X\n", u16_Reg);

	REG_READ_UINT16(NC_TEE_TR_RPT_CNT, u16_Reg);
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_TR_RPT_CNT(0x0C) = %04X\n", u16_Reg);

	nand_debug(UNFD_DEBUG_LEVEL,0,"FCIE Reg: ");
	for(u16_idx=0x30 ; u16_idx<0x80; u16_idx++)
	{
		if(0 == u16_idx%8)
			nand_debug(UNFD_DEBUG_LEVEL,0, "\n%02Xh:| ", u16_idx);
		
		REG_READ_UINT16(GET_REG_ADDR(FCIE_REG_TEE_BASE_ADDR, u16_idx), u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL, 0, "%04Xh ", u16_Reg);
	
		if(7 == u16_idx%8)
			nand_debug(UNFD_DEBUG_LEVEL,0,"| %02Xh ", u16_idx);
	}
	nand_debug(UNFD_DEBUG_LEVEL, 0,"\n");
}

//========================================================
// HAL function definitions
//========================================================
#if 0
U32 NC_TEE_ResetFCIE(void)
{
	//volatile U16 u16_i=0;
	volatile U16 u16Reg, u16Cnt;

	REG_WRITE_UINT16(NC_TEE_RESERVED_FOR_SW, NAND_DRIVER_VERSION);
    //nand_debug(1, 1, "1: %Xh\n", REG(NC_TEE_RESERVED_FOR_SW));

	NC_PlatformResetPre();

	// soft reset
	REG_CLR_BITS_UINT16(NC_TEE_FCIE_RST, BIT_FCIE_SOFT_RST); /* active low */

	//As reset is active, Check Reset Status from 0 -> 1
	#if 1
	u16Cnt=0;
	do
	{
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	  	if(0x1000 == u16Cnt++)
		{
			nand_DumpPadClk();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_ResetFCIE Err1, ErrCode: 0x%03X\n", UNFD_ST_ERR_NO_NFIE);
			return UNFD_ST_ERR_NO_NFIE;
		}

		REG_READ_UINT16(NC_TEE_FCIE_RST, u16Reg);

	}while (BIT_RST_STS_MASK  != (u16Reg  & BIT_RST_STS_MASK));
	#endif
	
	//nand_hw_timer_delay(HW_TIMER_DELAY_1ms);
	REG_SET_BITS_UINT16(NC_TEE_FCIE_RST, BIT_FCIE_SOFT_RST);
	//nand_hw_timer_delay(HW_TIMER_DELAY_1ms);

	//Restore reset bit, check reset status from 1 -> 0
	#if 1
	u16Cnt=0;
	do
	{
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	  	if(0x1000 == u16Cnt++)
		{
			nand_DumpPadClk();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_ResetFCIE Err2, ErrCode: 0x%03X\n", UNFD_ST_ERR_NO_NFIE);
			return UNFD_ST_ERR_NO_NFIE;
		}

		REG_READ_UINT16(NC_TEE_FCIE_RST, u16Reg);

	}while (0  != (u16Reg  & BIT_RST_STS_MASK));
	#endif

	NC_PlatformResetPost();

    //nand_debug(1, 1, "2: %Xh\n", REG(NC_TEE_RESERVED_FOR_SW));
    
	return UNFD_ST_SUCCESS; // ok
}
#endif

#if 0
U32 NC_TEE_ConfigNandFlashContext(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	if (pNandDrv->u16_ECCType < ECC_TYPE_24BIT1KB || 
		pNandDrv->u16_ECCType == ECC_TYPE_32BIT || pNandDrv->u16_ECCType == ECC_TYPE_40BIT 
		|| pNandDrv->u16_ECCType == ECC_TYPE_60BIT || pNandDrv->u16_ECCType == ECC_TYPE_72BIT)
		pNandDrv->u16_SectorByteCnt = 0x200;
	else
		pNandDrv->u16_SectorByteCnt = 0x400;

	pNandDrv->u8_BlkPageCntBits = drvNAND_CountBits(pNandDrv->u16_BlkPageCnt);
	pNandDrv->u8_PageByteCntBits = drvNAND_CountBits(pNandDrv->u16_PageByteCnt);
	//pNandDrv->u8_SpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SpareByteCnt);
	pNandDrv->u8_SectorByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorByteCnt);

	pNandDrv->u16_BlkPageCntMask = (1<<pNandDrv->u8_BlkPageCntBits)-1;
	pNandDrv->u16_PageByteCntMask = (1<<pNandDrv->u8_PageByteCntBits)-1;
	pNandDrv->u16_SectorByteCntMask = (1<<pNandDrv->u8_SectorByteCntBits)-1;
	//pNandDrv->u16_SpareByteCntMask = (1<<pNandDrv->u8_SpareByteCntBits)-1;

	pNandDrv->u16_PageSectorCnt = pNandDrv->u16_PageByteCnt >> pNandDrv->u8_SectorByteCntBits;
	pNandDrv->u8_PageSectorCntBits = drvNAND_CountBits(pNandDrv->u16_PageSectorCnt);
	pNandDrv->u16_PageSectorCntMask = (1<<pNandDrv->u8_PageSectorCntBits)-1;
	pNandDrv->u16_SectorSpareByteCnt = pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits;
	pNandDrv->u16_SectorSpareByteCnt &= ~1;
	pNandDrv->u16_SpareByteCnt = pNandDrv->u16_SectorSpareByteCnt << pNandDrv->u8_PageSectorCntBits;
	//pNandDrv->u8_SectorSpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorSpareByteCnt);

	pNandDrv->u8_BlkSectorCntBits = pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageSectorCntBits;
	pNandDrv->u16_BlkSectorCnt = 1<<pNandDrv->u8_BlkSectorCntBits;
	pNandDrv->u16_BlkSectorCntMask = pNandDrv->u16_BlkSectorCnt-1;

	return UNFD_ST_SUCCESS;
}
#endif

U32 NC_TEE_Init(void)
{
	//U32 u32_RetVal;
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();

    //LDR_PUTS(" NC_TEE_Init \n");
    
	// disable NC
	REG_CLR_BITS_UINT16(NC_TEE_FUN_CTL, BIT_NC_EN);
	REG_WRITE_UINT16(NC_TEE_CTRL , 0);
	// reset NC
	#if 0
	u32_RetVal = NC_TEE_ResetFCIE();
	if (UNFD_ST_SUCCESS != u32_RetVal)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_Init, ErrCode:%Xh \r\n", UNFD_ST_ERR_NO_NFIE);
		return u32_RetVal;
	}
    #endif

	// disable interupts
	//REG_CLR_BITS_UINT16(NC_TEE_MIE_INT_EN, BIT_MMA_DATA_END | BIT_NC_JOB_END);
	// clean int events
	REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, BIT_NC_JOB_END);
	// enable NC
	REG_WRITE_UINT16(NC_TEE_FUN_CTL, BIT_NC_EN);

	REG_WRITE_UINT16(NC_TEE_SIGNAL, pNandDrv->u16_Reg40_Signal);
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

	#if defined(NC_HWCMD_DELAY) && NC_HWCMD_DELAY
	REG_READ_UINT16(NC_TEE_RAND_W_CMD, pNandDrv->u16_Reg56_Rand_W_Cmd);
	#endif

	#if defined(NC_TRR_TCS) && NC_TRR_TCS
	REG_READ_UINT16(NC_TEE_LFSR_CTRL, pNandDrv->u16_Reg59_LFSRCtrl);
	#endif

	//pNandDrv->u8_PadMode =NAND_PAD_BYPASS_MODE;

    // ===================================================
	NC_TEE_Config();

    #if 0
	nand_debug(UNFD_DEBUG_LEVEL_LOW,1, "NC_TEE_ResetNandFlash\n");
	#if defined(DDR_NAND_SUPPORT) && DDR_NAND_SUPPORT
	u32_RetVal = NC_TEE_FCIE5SetInterface(0,0,0,0);
	#else
	u32_RetVal = NC_TEE_ResetNandFlash();
	#endif
	if(UNFD_ST_SUCCESS != u32_RetVal)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"ERROR, NAND reset flash, Err Code:%08Xh \n", u32_RetVal);
		return u32_RetVal;
	}
	nand_debug(UNFD_DEBUG_LEVEL_HIGH,1, "NC_TEE_ReadID\n");

	u32_RetVal = NC_TEE_ReadID();
	if (u32_RetVal != UNFD_ST_SUCCESS) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Failed to read ID with EC: 0x%08x\n", u32_RetVal);
		u32_RetVal = UNFD_ST_ERR_UNKNOWN_ID;
		return u32_RetVal;
	}
	#endif
	// ===================================================
	#if 0//defined(DDR_NAND_SUPPORT) && DDR_NAND_SUPPORT
	REG_READ_UINT16(NC_TEE_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
	REG_READ_UINT16(NC_TEE_DDR_CTRL, pNandDrv->u16_Reg58_DDRCtrl);
    // detect if Toggle-DDR
	pNandDrv->u16_Reg58_DDRCtrl &= ~BIT_DDR_MASM;
	u32_RetVal = NC_TEE_ProbeIfToggleDDR();
	if(UNFD_ST_ERR_NOT_TOGGLE_DDR == u32_RetVal)
	{
		nand_debug(UNFD_DEBUG_LEVEL_HIGH,1,"NOT Toggle DDR \n");

		if(UNFD_ST_ERR_NOT_ONFI_DDR == NC_TEE_ProbeIfONFIDDR())
		{
			nand_debug(UNFD_DEBUG_LEVEL_HIGH,1,"NOT ONFI DDR \n");
			nand_debug(0,1,"SDR NAND is detected\n");
			pNandDrv->u8_PadMode =NAND_PAD_BYPASS_MODE;
		}
		else
		{
			nand_debug(0,1,"YES ONFI DDR \n");

			u32_RetVal = NC_TEE_SetONFISyncMode(1);
			if(UNFD_ST_ERR_PAD_UNSUPPORT_DDR_NAND == u32_RetVal)
			{
				nand_debug(0,1, "SDR NAND is detected\n");
				NC_TEE_Config();
				return UNFD_ST_SUCCESS;
			}
			else if(UNFD_ST_SUCCESS != u32_RetVal)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"ERROR, NAND SetFeature ONFI-DDR, Err Code:%08Xh \n", u32_RetVal);
				return u32_RetVal;
			}

			//setup pll before pad switch

			pNandDrv->u16_Reg58_DDRCtrl |= BIT_DDR_ONFI;
			nand_clock_setting(pNandDrv->u32_Clk);		//set clock to 4x mode

			nand_pads_switch(NAND_PAD_ONFI_SYNC_MODE);
			pNandDrv->u8_PadMode = NAND_PAD_ONFI_SYNC_MODE;

			u32_RetVal= NC_TEE_DetectDDRTiming();
			if(UNFD_ST_SUCCESS != u32_RetVal)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"ERROR, NAND config timing, ONFI-DDR, Err Code:%08Xh \n", u32_RetVal);
				return u32_RetVal;
			}
		}
	}
	else
	{
		nand_debug(0,1,"Yes Toggle DDR \n");
		//setup pll before pad switch

		pNandDrv->u16_Reg58_DDRCtrl |= BIT_DDR_TOGGLE;
		nand_clock_setting(pNandDrv->u32_Clk);		//set clock to 4x mode

		nand_pads_switch(NAND_PAD_TOGGLE_MODE);
		pNandDrv->u8_PadMode = NAND_PAD_TOGGLE_MODE;

		u32_RetVal = NC_TEE_DetectDDRTiming();
		if(UNFD_ST_SUCCESS != u32_RetVal)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"ERROR, NAND config timing, Toggle-DDR, Err Code:%08Xh \n", u32_RetVal);
			return u32_RetVal;
		}
	}
	// ===================================================
	#endif

	NC_TEE_Config();
	return UNFD_ST_SUCCESS;
}


U32 NC_TEE_EnableLFSR(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	volatile U16 u16_RegVal;

	if(0 == IF_LFSR_ENABLED())
	{
		// tune timing: RE_LATCH
		if(0 == (pNandDrv->u16_Reg58_DDRCtrl & BIT_DDR_MASM))
		{
			REG_READ_UINT16(NC_TEE_LATCH_DATA, u16_RegVal);
			u16_RegVal = u16_RegVal & BIT_NC_LATCH_DATA_MASK;
			if(u16_RegVal > BIT_NC_LATCH_DATA_6_0_T)
			{
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error, RD_LATCH already > 1T, can NOT use LFSR.\n");
				return UNFD_ST_ERR_LFSR_RD_LATCH;
			}
			pNandDrv->u16_Reg57_RELatch &= ~BIT_NC_LATCH_DATA_MASK;
			pNandDrv->u16_Reg57_RELatch += u16_RegVal + BIT_NC_LATCH_DATA_1_0_T;
		}

		#if defined(DDR_NAND_SUPPORT) && DDR_NAND_SUPPORT
		// tune timing: DDR_TIMING
		REG_READ_UINT16(NC_TEE_LATCH_DATA, u16_RegVal);
		u16_RegVal = u16_RegVal & BIT_RE_DDR_TIMING_MASK;
		if(u16_RegVal == BIT_RE_DDR_TIMING_MASK) 
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error, DDR_TIMING full, can NOT use LFSR.\n");
			return UNFD_ST_ERR_LFSR_DDRTIMING;
		}
		pNandDrv->u16_Reg57_RELatch += 1<<BIT_RE_DDR_TIMING_SHIFT;
		#endif

		// walk around timing bug
		pNandDrv->u16_Reg57_RELatch &= ~BIT_RE_SEC_TURN_CNT_MASK;
		pNandDrv->u16_Reg57_RELatch |= 0xE << BIT_RE_SEC_TURN_CNT_SHIFT;

		REG_WRITE_UINT16(NC_TEE_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

		// set LFST
		pNandDrv->u16_Reg59_LFSRCtrl |= BIT_LFSR_ENABLE;
		REG_SET_BITS_UINT16(NC_TEE_LFSR_CTRL, BIT_LFSR_ENABLE);
	}

	return UNFD_ST_SUCCESS;
}

U32 NC_TEE_DisableLFSR(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	if(IF_LFSR_ENABLED())
	{
		if(0 == (pNandDrv->u16_Reg58_DDRCtrl & BIT_DDR_MASM))
			pNandDrv->u16_Reg57_RELatch -= BIT_NC_LATCH_DATA_1_0_T;

		#if defined(DDR_NAND_SUPPORT) && DDR_NAND_SUPPORT
		pNandDrv->u16_Reg57_RELatch -= 1<<BIT_RE_DDR_TIMING_SHIFT;
		#endif

		REG_WRITE_UINT16(NC_TEE_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

		// clear LFST
		pNandDrv->u16_Reg59_LFSRCtrl &= ~BIT_LFSR_ENABLE;
		REG_CLR_BITS_UINT16(NC_TEE_LFSR_CTRL, BIT_LFSR_ENABLE);
	}
	return UNFD_ST_SUCCESS;
}

U32 NC_TEE_RegInit(void)
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

void NC_TEE_Config(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	REG_WRITE_UINT16(NC_TEE_SIGNAL, pNandDrv->u16_Reg40_Signal);
	/*sector spare size*/
	REG_WRITE_UINT16(NC_TEE_SPARE, pNandDrv->u16_Reg48_Spare);
	/* page spare size*/
	REG_WRITE_UINT16(NC_TEE_SPARE_SIZE, (U16)pNandDrv->u16_Reg49_SpareSize);
	/* page size and ECC type*/
	REG_WRITE_UINT16(NC_TEE_ECC_CTRL, pNandDrv->u16_Reg50_EccCtrl);

	REG_WRITE_UINT16(NC_TEE_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	#if defined(NC_TWHR_TCLHZ) && NC_TWHR_TCLHZ
	/*set timing for tCLHZ*/
	REG_WRITE_UINT16(NC_TEE_NAND_TIMING, pNandDrv->u16_Reg5A_tWHR_tCLHZ);
	#endif

	#if defined(NC_TCWAW_TADL) && NC_TCWAW_TADL
	/*set timing for tADL and tCWAW*/
	REG_WRITE_UINT16(NC_TEE_NAND_TIMING1,pNandDrv->u16_Reg5D_tCWAW_tADL);
	#endif

	#if (defined(FCIE_LFSR) && FCIE_LFSR) || (defined(NC_TRR_TCS) && NC_TRR_TCS)
	REG_WRITE_UINT16(NC_TEE_LFSR_CTRL, pNandDrv->u16_Reg59_LFSRCtrl);
	#endif

	#if defined(NC_HWCMD_DELAY) && NC_HWCMD_DELAY
	REG_WRITE_UINT16(NC_TEE_RAND_W_CMD, pNandDrv->u16_Reg56_Rand_W_Cmd);
	#endif
}

void NC_TEE_ReConfig(void) // re-config FCIE3 for NFIE mode
{
	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	// enable interrupts if system allows
	//nand_enable_intr_mode();
	#else
	// disable interupts
	REG_CLR_BITS_UINT16(NC_TEE_MIE_INT_EN,  BIT_NC_JOB_END);
	#endif
	// clean int events
	REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_TEE_FUN_CTL, BIT_NC_EN);

	NC_TEE_ResetFCIE();
    NC_TEE_Config();
}
#if 0
U32 NC_TEE_SelectCE(U32 u32_PhyRowIdx)
{
    NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
    U32 u32_ChipRowIdx;
    U32 u32_ChipPageCnt;
    U16 u16_CESel;

    //nand_debug(UNFD_DEBUG_LEVEL, 1, "Logical row %Xh", u32_PhyRowIdx);
    
    u16_CESel = 0;
    u32_ChipRowIdx = u32_PhyRowIdx;
    u32_ChipPageCnt = (pNandDrv->u16_BlkCnt/(pNandDrv->u8_MaxCENum+1))<<pNandDrv->u8_BlkPageCntBits;
       
    while(u32_ChipRowIdx >= u32_ChipPageCnt)
    {
        u16_CESel++;
        u32_ChipRowIdx -= u32_ChipPageCnt;
    }
    
    REG_CLR_BITS_UINT16(NC_TEE_SIGNAL,  BIT_NC_CE_SEL_MASK);
    REG_SET_BITS_UINT16(NC_TEE_SIGNAL,  u16_CESel);    

    //nand_debug(UNFD_DEBUG_LEVEL, 0, " -> CE%d row %Xh\n", u16_CESel, u32_ChipRowIdx);
    
    return u32_ChipRowIdx;

}
#endif
// can not cross block
U32 NC_TEE_WritePages(    U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_PageCnt )
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
	//NC_RECONFIG();
	REG_WRITE_UINT16(NC_TEE_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_TEE_FUN_CTL, BIT_R2N_MODE_EN);
	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_TEE_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_TEE_LFSR_CTRL,
	  	((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) & BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif
	u32_DataDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_DataBuf, pNandDrv->u16_PageByteCnt * u32_PageCnt);
	u32_SpareDMAAddr = nand_translate_DMA_address_Ex((U32)pu8_SpareBuf, pNandDrv->u16_SpareByteCnt);
	REG_WRITE_UINT16(NC_TEE_WSPARE_DMA_ADR0, u32_SpareDMAAddr & 0xFFFF);
	REG_WRITE_UINT16(NC_TEE_WSPARE_DMA_ADR1, u32_SpareDMAAddr >>16);
	REG_WRITE_UINT16(NC_TEE_WDATA_DMA_ADR0, u32_DataDMAAddr & 0xFFFF);//>>MIU_BUS_WIDTH_BITS));
	REG_WRITE_UINT16(NC_TEE_WDATA_DMA_ADR1, u32_DataDMAAddr >> 16);//(MIU_BUS_WIDTH_BITS+16)));
	REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, u32_PhyRowIdx >> 16);
	REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle<< 8) | (CMD_0x80));
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (CMD_0x10 << 8) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (CMD_0x70 << 8) | ACT_WAITRB);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (ACT_REPEAT << 8) | ACT_CHKSTATUS);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, ACT_BREAK);
	REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, AUXADR_RPTCNT);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, u32_PageCnt - 1);
	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif
	REG_WRITE_UINT16(NC_TEE_CTRL, BIT_NC_JOB_START);
	if (NC_TEE_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME*u32_PageCnt) == WAIT_WRITE_TIME*u32_PageCnt)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_TEE_WritePages Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_TEE_Config();
		//NC_ResetNandFlash();
		#else
		#endif
        LDR_PUTS("\n NC_TEE_WritePages TO\n");
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}
    #if defined(FCIE_WRITE_PROTECT_SUPPORT) && FCIE_WRITE_PROTECT_SUPPORT
	if((REG(NC_TEE_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT)
	{
		U32 u32_SecureRow;
		nand_debug(0, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		u32_SecureRow = (REG(NC_TEE_SECURE_ERR_ADDR_H) << 16)|(REG(NC_TEE_SECURE_ERR_ADDR_L) & 0xFFFF);
		if(u32_SecureRow != u32_PhyRowIdx)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Alert Address 0x%X!= 0x%X\n", u32_SecureRow, u32_PhyRowIdx);
		}					
		NC_TEE_Config();
		REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, BIT_NC_SECURE_ALERT);
		u32_Ret = UNFD_ST_ERR_W_PROTECTED;
	}
	else
    #endif	
	u32_Ret = NC_TEE_CheckEWStatus(OPTYPE_WRITE);
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



U32 NC_TEE_EraseBlk(U32 u32_PhyRowIdx)
{
    NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_Ret;
	//NC_RECONFIG();
    REG_CLR_BITS_UINT16(NC_TEE_SIGNAL, BIT5);
    REG_SET_BITS_UINT16(NC_TEE_SIGNAL, BIT4);
	REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, BIT_NC_JOB_END);
	REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, u32_PhyRowIdx & 0xFFFF);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, u32_PhyRowIdx >> 16);
	REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (pNandDrv->u8_OpCode_Erase_AdrCycle << 8) | CMD_0x60);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0xD0);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (ACT_CHKSTATUS << 8) | CMD_0x70);
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, ACT_BREAK);
	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
    nand_CheckPowerCut();
	#endif
	REG_WRITE_UINT16(NC_TEE_CTRL, BIT_NC_JOB_START);
	if (NC_TEE_WaitComplete(BIT_NC_JOB_END, WAIT_ERASE_TIME) == WAIT_ERASE_TIME)
	{
    	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_TEE_EraseBlk Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_E_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_TEE_Config();
		//NC_ResetNandFlash();
		#else
		#endif
		return UNFD_ST_ERR_E_TIMEOUT;
	}
    #if defined(FCIE_WRITE_PROTECT_SUPPORT) && FCIE_WRITE_PROTECT_SUPPORT
	if((REG(NC_TEE_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT)
	{
		U32 u32_SecureRow;
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		u32_SecureRow = (REG(NC_TEE_SECURE_ERR_ADDR_H) << 16)|(REG(NC_SECURE_ERR_ADDR_L) & 0xFFFF);
		if(u32_SecureRow != u32_PhyRowIdx)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Alert Address 0x%X!= 0x%X\n", u32_SecureRow, u32_PhyRowIdx);
		}
		NC_TEE_Config();
		REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, BIT_NC_SECURE_ALERT);
		u32_Ret = UNFD_ST_ERR_W_PROTECTED;
	}
	else		
    #endif
	u32_Ret = NC_TEE_CheckEWStatus(OPTYPE_ERASE);
    if(UNFD_ST_SUCCESS != u32_Ret)
    {
        LDR_PUTS("erase err: \n"); 
        LDR_PUTDW(u32_Ret); LDR_PUTS("\n");
        nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"erase blk:%Xh fail:%Xh \n", 
            u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, u32_Ret);
    }
	return u32_Ret;
    
}



U32 NC_TEE_CheckEWStatus(U8 u8_OpType)
{
	volatile U16 u16_Tmp;
	U32 u32_ErrCode = UNFD_ST_SUCCESS;

	REG_READ_UINT16(NC_TEE_ST_READ, u16_Tmp);

	if ((u16_Tmp & BIT_ST_READ_FAIL) == 1) { // if fail
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_FAIL;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_FAIL;

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_CheckEWStatus Fail, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_TEE_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	} else if ((u16_Tmp & BIT_ST_READ_BUSYn) == 0) { // if busy
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_BUSY;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_BUSY;

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_CheckEWStatus Busy, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_TEE_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	} else if ((u16_Tmp & BIT_ST_READ_PROTECTn) == 0) { // if protected
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_PROTECTED;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_PROTECTED;

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_CheckEWStatus Protected, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_TEE_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	}

	return u32_ErrCode;
}


#if 0
U32 NC_TEE_ResetNandFlash(void)
{
	NAND_DRIVER *pNandDrv=(NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;
	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_TEE_REConfig();
	NC_TEE_SET_DDR_MODE();

  	REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, BIT_NC_JOB_END);

  	REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, AUXADR_INSTQUE);
	#if defined(DDR_NAND_SUPPORT) && DDR_NAND_SUPPORT
	if(pNandDrv->u16_Reg58_DDRCtrl&BIT_DDR_ONFI)
	{
		// set commnad reg
		REG_WRITE_UINT16(NC_TEE_AUXREG_ADR, 0x08);
		REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, 0xFC);
		REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_REG8L);
	}
	else
	  	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0xFF);
	#else
	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0xFF);
	#endif
  	REG_WRITE_UINT16(NC_TEE_AUXREG_DAT, ACT_BREAK);

  	REG_WRITE_UINT16(NC_TEE_CTRL, BIT_NC_JOB_START);

  	if (NC_TEE_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
  	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_TEE_ResetNandFlash, ErrCode:%Xh \r\n", UNFD_ST_ERR_RST_TIMEOUT);
		NC_TEE_ReConfig();
		NC_TEE_CLR_DDR_MODE();
		return UNFD_ST_ERR_RST_TIMEOUT;
  	}

	NC_TEE_CLR_DDR_MODE();
  	return UNFD_ST_SUCCESS;
}
#endif


U32 NC_TEE_WaitCIFD_Event(U16 u16_WaitEvent, U32  u32_MicroSec)
{
	volatile U32 u32_Count;
	volatile U16 u16_Reg;
	
	for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++)
	{
		REG_READ_UINT16(NC_TEE_CIFD_EVENT, u16_Reg);
		if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
			break;

		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	}
	if (u32_Count >= u32_MicroSec)
	{
		REG_READ_UINT16(NC_TEE_CIFD_EVENT, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "CIFD Event Timout %X\n", u16_Reg);
		//NC_TEE_DumpDebugBus();
		//NC_TEE_DumpRegisters();		
	}
	return u32_Count;	
}


//extern void NC_Test_Reg717274_Set(void);
//extern void NC_Test_Reg717274_Get(void);
U32 NC_TEE_WaitComplete(U16 u16_WaitEvent, U32 u32_MicroSec)
{
	volatile U32 u32_Count;
	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	volatile U32 u32_Err;
	#endif
	volatile U16 u16_Reg;
    
	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	REG_READ_UINT16(NC_TEE_MIE_INT_EN, u16_Reg);
	if(u16_Reg & u16_WaitEvent)
	{
		u32_Err = nand_WaitCompleteIntr(u16_WaitEvent, u32_MicroSec);
		if( u32_Err == UNFD_ST_SUCCESS )
			return 0;
		else
		{
			NC_TEE_DumpDebugBus();
			NC_TEE_DumpRegisters();

			nand_die();
			return u32_MicroSec;
		}
	}
	#endif

    //PWROFF_Ex0();
    
	for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++)
	{
		REG_READ_UINT16(NC_TEE_MIE_EVENT, u16_Reg);
		if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
			break;

		nand_hw_timer_delay(HW_TIMER_DELAY_1us);		
		//nand_reset_WatchDog();
        //PWROFF_Ex1();
	}

	if (u32_Count < u32_MicroSec)
		REG_W1C_BITS_UINT16(NC_TEE_MIE_EVENT, u16_WaitEvent); /*clear events*/
	else
	{
		if(0)//Disable_Err_log == 0)
		{
			//NC_TEE_DumpDebugBus();
			//NC_TEE_DumpRegisters();
		}
	}

	return u32_Count;
}


#endif // NC_SEL_FCIE3
