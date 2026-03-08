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
#include "../../inc/common/drvNAND_utl.h"

#if defined(CONFIG_TEE_LOADER)

extern U8 gpu8_PageDataBuf[]; // 32K
extern U8 gpu8_SpareBuf[];// 4K

U32 drvNAND_SearchCIS(void)
{
    PAIRED_PAGE_MAP_t   *pPairedPageMap = drvNAND_get_DrvContext_PPM();
    NAND_DRIVER         *pNandDrv = drvNAND_get_DrvContext_address();
    U8                  *au8_PageBuf = gpu8_PageDataBuf;
    U8                  *au8_SpareBuf = gpu8_SpareBuf;
    U16                 u16_PBA;
    NAND_FLASH_INFO_t   *pNandInfo = (NAND_FLASH_INFO_t*)au8_PageBuf;
    BLK_INFO_t          *pBlkInfo = (BLK_INFO_t*)au8_SpareBuf;
    U32                 u32_PageIdx;
    U32                 u32_Err;
    U32                 u32_ChkSum;
    U32                 u16_i;
    //U8                        u8_CisType;
    U8                  u8_BlkPageCntBits = 6; /* 64 pages per block */
    #if defined(FCIE_LFSR) && FCIE_LFSR
    U8 u8_RetryRandomizer;
    #endif

    pNandDrv->u32_CISRow = 0xFFFFFFFF;

    for (u16_PBA = 0; u16_PBA < 40; u16_PBA++) // max 40 for 3D MLC, 256 pages per block
    {
        #if defined(MICRON_READ_RETRY) && MICRON_READ_RETRY
        if(pNandDrv->au8_ID[0] == 0x2C)
            pNandDrv->u8_RequireReadRetry = 1;
        else
            pNandDrv->u8_RequireReadRetry = 0;
        #endif

        #if defined(FCIE_LFSR) && FCIE_LFSR
        u8_RetryRandomizer = 0;
        LABEL_TRY_RANDOMIZER:
        //force disable Randomizer
        if(!u8_RetryRandomizer)
            NC_DisableLFSR();
        else
            NC_EnableLFSR();
        #endif

        pNandDrv->u16_BlkPageCnt = 0x40;
        pNandDrv->u16_PageByteCnt = 0x800;
        #if defined(NANDINFO_ECC_TYPE)
        pNandDrv->u16_ECCType = NANDINFO_ECC_TYPE;
        #else
        pNandDrv->u16_ECCType = ECC_TYPE_40BIT1KB;
        #endif
        #if defined(ECC_TYPE_72BIT1KB) && ECC_TYPE_72BIT1KB
        if(pNandDrv->u16_ECCType == ECC_TYPE_72BIT1KB)
            pNandDrv->u16_SpareByteCnt = 0x180;
        else
        #endif
            pNandDrv->u16_SpareByteCnt = 0x100;
        //REG_WRITE_UINT16(DEBUG_REG, 0xBE);

        NC_ConfigContext();
        NC_Init();
        //NC_ReInit();
        //NC_Config();


        pNandDrv->u16_Reg48_Spare |= BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
        NC_Config();
        u32_PageIdx = u16_PBA << u8_BlkPageCntBits;
        nand_debug(UNFD_DEBUG_LEVEL_HIGH, 0, "%uPages: %d\n", pNandDrv->u16_BlkPageCnt, u16_PBA);

        // ------------------------------------
        if(pNandDrv->u8_Flag_004A30)
        {
            pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C4TFS0;
        }

        LABEL_TRY_005A30:

        // ------------------------------------
        //REG_WRITE_UINT16(DEBUG_REG, 0xBD);
        u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_PageBuf, au8_SpareBuf, 1);

        if (u32_Err != UNFD_ST_SUCCESS || pBlkInfo->u8_BadBlkMark != 0xFF)
        {
            //REG_WRITE_UINT16(DEBUG_REG, 0xCE);
            if(1==pNandDrv->u8_Flag_004A30 && ADR_C5TFS0!=pNandDrv->u8_OpCode_RW_AdrCycle)
            {
                //nand_debug(0,1,"switch to 005A30, 0 \n");
                pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C5TFS0;
                goto LABEL_TRY_005A30;
            }

            #if defined(FCIE_LFSR) && FCIE_LFSR
            if(!u8_RetryRandomizer)
            {
                u8_RetryRandomizer = 1;
                goto LABEL_TRY_RANDOMIZER;
            }
            #endif

            continue;
        }

        //REG_WRITE_UINT16(DEBUG_REG, 0xBB);
        if (drvNAND_CompareCISTag(pNandInfo->au8_Tag))
        {
            if(1==pNandDrv->u8_Flag_004A30 && ADR_C5TFS0!=pNandDrv->u8_OpCode_RW_AdrCycle)
            {
                //nand_debug(0,1,"switch to 005A30, 1 \n");
                pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C5TFS0;
                goto LABEL_TRY_005A30;
            }

            continue;
        }

        u32_ChkSum = drvNAND_CheckSum(au8_PageBuf + 0x24, 0x32 - 0x24);
        if (u32_ChkSum != pNandInfo->u32_ChkSum)
        {
            nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "Skip Blk 0x%08x, ChkSum: 0x%08x != 0x%08x\n",
                u16_PBA, u32_ChkSum, pNandInfo->u32_ChkSum);
            //REG_WRITE_UINT16(DEBUG_REG, u32_ChkSum);
            //REG_WRITE_UINT16(0x1f206704, pNandInfo->u32_ChkSum);
            //REG_WRITE_UINT16(DEBUG_REG, au8_PageBuf[32]);
            //while(1);
            if(1==pNandDrv->u8_Flag_004A30 && ADR_C5TFS0!=pNandDrv->u8_OpCode_RW_AdrCycle)
            {
                //nand_debug(0,1,"switch to 005A30, 2 \n");
                pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C5TFS0;
                goto LABEL_TRY_005A30;
            }

            continue;
        }

        //u8_CISIdx = pBlkInfo->u16_BlkAddr;
        //REG_WRITE_UINT16(DEBUG_REG, 0xBA);

        //nand_debug(1, 0, "Complete auto-checking 8/16 bits:0x%X\n", pNandDrv->u8_WordMode);
        pNandDrv->u16_BlkCnt       = pNandInfo->u16_BlkCnt;
        pNandDrv->u8_IDByteCnt     = pNandInfo->u8_IDByteCnt;
        pNandDrv->u16_PageByteCnt  = pNandInfo->u16_PageByteCnt;
        pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
        pNandDrv->u16_BlkPageCnt   = pNandInfo->u16_BlkPageCnt;
        pNandDrv->u16_ECCType      = pNandInfo->u16_ECCType;

        pNandDrv->u32_CISRow = u32_PageIdx;
        #if 0
        pNandDrv->u8_CellType = pNandInfo->u32_Config&0x01;
        pNandDrv->u8_SlcMode = ((pNandInfo->u32_Config>>21) & 1);
        if(pNandDrv->u8_SlcMode == 1)
            pNandDrv->u16_BlkPageCnt   = (pNandInfo->u16_BlkPageCnt>>1);
        #if defined(MICRON_READ_RETRY) && MICRON_READ_RETRY
        if(pNandDrv->au8_ID[0] == 0x2C)
            pNandDrv->u8_RequireReadRetry = ((pNandInfo->u32_Config>>15) & 1);
        else
            pNandDrv->u8_RequireReadRetry = 0;
        #endif
        #if defined(FCIE_LFSR) && FCIE_LFSR
        pNandDrv->u8_RequireRandomizer = (pNandInfo->u32_Config>>8) & 0x1;
        #endif
        pNandDrv->u8_PairPageMapLoc = pNandInfo->u8_PairPageMapLoc;
        pNandDrv->u8_BL0PBA = pNandInfo->u8_BL0PBA;
        pNandDrv->u8_BL1PBA = pNandInfo->u8_BL1PBA;
        pNandDrv->u8_UBOOTPBA = pNandInfo->u8_UBOOTPBA;
        pNandDrv->u8_AddrCycleIdx = pNandInfo->u8_AddrCycleIdx;
        pNandDrv->u8_HashPBA[0][0] = pNandInfo->u8_HashPBA[0][0];
        pNandDrv->u8_HashPBA[0][1] = pNandInfo->u8_HashPBA[0][1];
        pNandDrv->u8_HashPBA[1][0] = pNandInfo->u8_HashPBA[1][0];
        pNandDrv->u8_HashPBA[1][1] = pNandInfo->u8_HashPBA[1][1];
        pNandDrv->u8_HashPBA[2][0] = pNandInfo->u8_HashPBA[2][0];
        pNandDrv->u8_HashPBA[2][1] = pNandInfo->u8_HashPBA[2][1];
        pNandDrv->u8_BitflipThreshold = pNandInfo->u8_BitflipThreshold;
        pNandDrv->u8_HashPageIdx[0] = pNandInfo->u8_HashPageIdx[0];
        pNandDrv->u8_HashPageIdx[1] = pNandInfo->u8_HashPageIdx[1];
        pNandDrv->u8_BootVerion = (pNandInfo->u32_Config>>16)&0x03;
        pNandDrv->u32_BootSize = pNandInfo->u32_BootSize;

        //pNandDrv->u32_CISRow = u32_PageIdx;
        //pNandDrv->u8_UBOOT1PBA = pNandInfo->u8_UBOOT1PBA;
        if(pNandDrv->u8_AddrCycleIdx != 0 && pNandDrv->u8_AddrCycleIdx >= 2)
        {
            pNandDrv->u8_OpCode_RW_AdrCycle = seq[pNandDrv->u8_AddrCycleIdx  - 2].u8_OpCode_RW_AdrCycle;
            pNandDrv->u16_One_Col_Addr = seq[pNandDrv->u8_AddrCycleIdx - 2].u16_Reg48_Spare;
            pNandDrv->u16_Reg48_Spare &= ~BIT_NC_ONE_COL_ADDR;
            pNandDrv->u16_Reg48_Spare |= seq[pNandDrv->u8_AddrCycleIdx - 2].u16_Reg48_Spare;
        }

        //REG_WRITE_UINT16(DEBUG_REG, 0xB9);
        REG_WRITE_UINT16(DEBUG_REG, pNandInfo->u8_BL0PBA);
        #endif

        NC_ConfigContext();
        NC_Init();
        pNandDrv->u16_Reg48_Spare &= ~BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
        NC_Config();
        #if defined(FCIE_LFSR) && FCIE_LFSR
        if(pNandDrv->u8_RequireRandomizer)
        {
            nand_debug(0,1,"Enable Randomizer\n");
            NC_EnableLFSR();    //Compatible with ROM Code
        }
        #endif
        //NC_ReInit();
        //NC_Config();
        //MLC or SLC if MLC read pair page map from cis 4 th Lsb page
        #if defined(SUPPORT_3D_NAND) && SUPPORT_3D_NAND
        if(pNandDrv->u8_CellType == NAND_CellType_SLC || pNandDrv->u8_SlcMode == 1)
        #else
        if(pNandDrv->u8_CellType == NAND_CellType_SLC)
        #endif
        {
            for(u16_i=0; u16_i< pNandDrv->u16_BlkPageCnt; u16_i++)
            {
                pPairedPageMap[u16_i].u16_LSB = u16_i;
                pPairedPageMap[u16_i].u16_MSB = u16_i;
            }
            pNandDrv->u16_BlkLowPCnt = pNandDrv->u16_BlkPageCnt;
        }

        nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "CIS%d is found in blk 0x%04x\n",
           u8_CISIdx, pNandDrv->u8_CISBlk);
        //REG_WRITE_UINT16(DEBUG_REG, 0xB8);

        return UNFD_ST_SUCCESS;
    }

    REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8131);
    //REG_WRITE_UINT16(DEBUG_REG, 0xB7);
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "No CIS found\n");
    #if defined(FCIE_LFSR) && FCIE_LFSR
    NC_DisableLFSR();
    #endif

    return UNFD_ST_ERR_NO_CIS;
}


/* return 0: same, 1: different */
U32 drvNAND_CompareCISTag(U8 *tag)
{
    const char *str = "MSTARSEMIUNFDCIS";
    int i = 0;

    for (i = 0; i < 16; i++) {
        if (tag[i] != str[i])
            return 1;
    }

    return 0;
}


U32 drvNAND_CheckSum(U8 *pu8_Data, U16 u16_ByteCnt)
{
    U32 u32_Sum = 0;

    while (u16_ByteCnt--)
        u32_Sum += *pu8_Data++;

    return u32_Sum;
}


void drvNAND_SetForNni(U8 Enable)
{
    NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
    static U16 PageByteCnt, SpareByteCnt, u16_ECCType;

    if(Enable){
        // backup
        PageByteCnt = pNandDrv->u16_PageByteCnt;
        SpareByteCnt = pNandDrv->u16_SpareByteCnt;
        u16_ECCType = pNandDrv->u16_ECCType;
        // change
        pNandDrv->u16_PageByteCnt = 2048;
        pNandDrv->u16_SpareByteCnt = 0x100;
        #if defined(NANDINFO_ECC_TYPE)
        pNandDrv->u16_ECCType = NANDINFO_ECC_TYPE;
        #else
        pNandDrv->u16_ECCType = ECC_TYPE_40BIT1KB;
        #endif
        #if defined(ECC_TYPE_72BIT1KB) && ECC_TYPE_72BIT1KB
        if(pNandDrv->u16_ECCType == ECC_TYPE_72BIT1KB)
            pNandDrv->u16_SpareByteCnt = 0x180;
        else
        #endif
            pNandDrv->u16_SpareByteCnt = 0x100;

        NC_ConfigNandFlashContext();
        NC_RegInit();
        pNandDrv->u16_Reg48_Spare |= BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
        NC_Config();
    }
    else{
        // restore
        pNandDrv->u16_PageByteCnt = PageByteCnt;
        pNandDrv->u16_SpareByteCnt = SpareByteCnt;
        pNandDrv->u16_ECCType = u16_ECCType;
        NC_ConfigNandFlashContext();
        NC_RegInit();
        pNandDrv->u16_Reg48_Spare &= ~BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
        NC_Config();
    }
}

// find CIS1,
// check ECC bits,
// refresh/restore if needed,
// lock.
U32 drvNAND_CheckCIS(U8 IfLockCIS)
{
    PAIRED_PAGE_MAP_t *pPairedPageMap = drvNAND_get_DrvContext_PPM();
    NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
    U16 u16_i, PBACIS[2], ErrBits[2];//={0xFFFF,0xFFFF};
    U16 PBA=0, u16_j;
    U32 u32_Err, u32_PageIdx;
    U8 *au8_SectorBuf = (U8*)gpu8_PageDataBuf;
    U8 *au8_SpareBuf = (U8*)gpu8_SpareBuf;

    if(0xFFFFFFFF == pNandDrv->u32_CISRow)
    {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"no CIS found \n");
        return UNFD_ST_ERR_INVALID_PARAM;
    }
    PBACIS[0] = pNandDrv->u32_CISRow >> pNandDrv->u8_BlkPageCntBits;
    PBACIS[1] = ErrBits[0] = ErrBits[1] = 0xFFFF;

    // ------------------------------------
    // check CIS0 error bits
    u32_PageIdx = (PBACIS[0] << pNandDrv->u8_BlkPageCntBits) + pPairedPageMap[1].u16_LSB;
    u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf, 1);
    if (u32_Err == UNFD_ST_SUCCESS)
        ErrBits[0] = NC_GetECCBits();

    nand_debug(0,1,"CIS0 @ %Xh PBA, ErrBits: %Xh \n", PBACIS[0], ErrBits[0]);

    // ------------------------------------
    // find CIS1
    for(u16_i=PBACIS[0]+1; u16_i<10; u16_i++)
    {
        u32_PageIdx = u16_i << pNandDrv->u8_BlkPageCntBits;
        u32_Err = drvNAND_CheckPartInfo(u32_PageIdx + pPairedPageMap[1].u16_LSB);
        if (u32_Err == UNFD_ST_SUCCESS)
        {
            PBACIS[1] = u16_i;
            ErrBits[1] = NC_GetECCBits();
            break;
        }
    }
    nand_debug(0,1,"CIS1 @ %Xh PBA, ErrBits: %Xh \n", PBACIS[1], ErrBits[1]);

    if(PBACIS[0]>=10 && PBACIS[1]>=10){
        nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"no CIS: %Xh %Xh \n", PBACIS[0], PBACIS[1]);
        return UNFD_ST_ERR_NO_CIS;
    }
    #if 1
    // ------------------------------------
    // refresh
    for(u16_i=0; u16_i<2; u16_i++)
    {
        if(ErrBits[u16_i] <= pNandDrv->u16_BitflipThreshold)
            continue;
        
        // find good PBA
        LABEL_FIND_GOODBLK:
        if(0xFFFF==PBACIS[u16_i])
        {
            for(u16_j=PBA; u16_j<10; u16_j++){
                //if(drvNAND_IsGoodBlk(u16_j))
                u32_PageIdx = u16_j << pNandDrv->u8_BlkPageCntBits;
                u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf, 1);
                if(0xFF==au8_SpareBuf[0] && (u32_Err==UNFD_ST_SUCCESS||u32_Err==UNFD_ST_ERR_ECC_FAIL))
                    break;
            }
            if(u16_j == 10){
                nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"no good PBA to refresh CIS%u \n", u16_i);
                return UNFD_ST_ERR_NO_GOOD_BLK;
            }
            PBA= u16_j;                
        }
        if((0==u16_i && PBA==PBACIS[1]) || (1==u16_i && PBA==PBACIS[0])){
            PBA++;
            goto LABEL_FIND_GOODBLK;
        }

        PBACIS[u16_i] = PBA;
        nand_debug(0,1,"refresh CIS%u PBA:%Xh ..\n", u16_i, PBACIS[u16_i]);

        // refresh CIS
        u32_Err = NC_EraseBlk(PBACIS[u16_i] << pNandDrv->u8_BlkPageCntBits);
        if(UNFD_ST_SUCCESS != u32_Err)
            goto LABEL_FIND_GOODBLK;

        // read nni
        drvNAND_SetForNni(1);
        u32_PageIdx = PBACIS[u16_i^1] << pNandDrv->u8_BlkPageCntBits;
        
        u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf, 1);
        if(UNFD_ST_SUCCESS != u32_Err){
            drvNAND_SetForNni(0);
            nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"read nni fail: %Xh %Xh %Xh \n", 
                u16_i, PBACIS[0], PBACIS[1]);
            return UNFD_ST_ERR_NO_CIS;
        }
        // write nni
        u32_PageIdx = PBACIS[u16_i] << pNandDrv->u8_BlkPageCntBits;
        u32_Err = NC_WriteSectors(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf, 1);
        drvNAND_SetForNni(0);
        if (u32_Err != UNFD_ST_SUCCESS){
            nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "write nni fail: %Xh %Xh %Xh \n", 
                u16_i, PBACIS[0], PBACIS[1]);
            drvNAND_MarkBadBlk(PBACIS[u16_i]);
            goto LABEL_FIND_GOODBLK;
        }
        
        // read pni
        u32_PageIdx = PBACIS[u16_i^1] << pNandDrv->u8_BlkPageCntBits;
        u32_Err = NC_ReadSectors(u32_PageIdx+1, 0, au8_SectorBuf, au8_SpareBuf, 1);
        if(UNFD_ST_SUCCESS != u32_Err){
            nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"read pni fail: %Xh %Xh %Xh \n", 
                u16_i, PBACIS[0], PBACIS[1]);
            return UNFD_ST_ERR_NO_CIS;
        }
        // write pni
        u32_PageIdx = PBACIS[u16_i] << pNandDrv->u8_BlkPageCntBits;
        u32_Err = NC_WriteSectors(u32_PageIdx+1, 0, au8_SectorBuf, au8_SpareBuf, 1);
        if (u32_Err != UNFD_ST_SUCCESS){
            nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "write pni fail: %Xh %Xh %Xh \n", 
                u16_i, PBACIS[0], PBACIS[1]);
            drvNAND_MarkBadBlk(PBACIS[u16_i]);
            goto LABEL_FIND_GOODBLK;
        }

        nand_debug(0,1,"refresh CIS%u done \n", u16_i);
    }
    #endif

    // ------------------------------------
    // write-protect CIS blocks
    #if defined(CONFIG_MSTAR_BUILD_TEE_LOADER_DAT)
    if(IfLockCIS){
        U32 row0, row1;

        row0 = (PBACIS[0] < PBACIS[1]) ?
            PBACIS[0]<<pNandDrv->u8_BlkPageCntBits :
            PBACIS[1]<<pNandDrv->u8_BlkPageCntBits;
        row1 = (PBACIS[0] < PBACIS[1]) ?
            PBACIS[1]<<pNandDrv->u8_BlkPageCntBits :
            PBACIS[0]<<pNandDrv->u8_BlkPageCntBits;

        nand_set_WP_CIS(row0, row1+pNandDrv->u16_BlkPageCnt-1);
    }
    #endif

    return UNFD_ST_SUCCESS;
}


U32 drvNAND_CheckPartInfo(U32 u32_PageIdx)
{
    //NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
    U8 *au8_SectorBuf = (U8*)gpu8_PageDataBuf;
    U8 *au8_SpareBuf = (U8*)gpu8_SpareBuf;
    U32 u32_Err;
    U32 u32_ChkSum;
    //U32 u32_ByteCnt;

    PARTITION_INFO_t *pPartInfo = (PARTITION_INFO_t*)au8_SectorBuf;

    //u32_Err = NC_ReadSector_RIUMode(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf);
    u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
        return u32_Err;
    }

    u32_ChkSum = drvNAND_CheckSum(au8_SectorBuf + 0x04, 0x200 - 0x04);
    if (u32_ChkSum != pPartInfo->u32_ChkSum) {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Skip PartInfo (ChkSum: 0x%08X != 0x%08X)\n",
                   u32_ChkSum, pPartInfo->u32_ChkSum);

        return UNFD_ST_ERR_CIS_PART_ERR;
    }

    //u32_ByteCnt = pPartInfo->u16_PartCnt * pPartInfo->u16_UnitByteCnt;
    //memcpy(pNandDrv->pPartInfo, pPartInfo, sizeof(*pPartInfo) + u32_ByteCnt);

    //dump_part_info(pPartInfo);
    return UNFD_ST_SUCCESS;
}


U32 drvNAND_MarkBadBlk(U16 u16_PBA)
{
    NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
    U8 *au8_PageBuf = (U8*)gpu8_PageDataBuf;
    U8 *au8_SpareBuf = (U8*)gpu8_SpareBuf;
    U32 u32_Err;
    U32 u32_PageIdx;
    #if (defined(NC_SEL_FCIE5) && (NC_SEL_FCIE5))
    U8  u8_LFSR=0;
    #endif

    u32_Err = NC_EraseBlk(u16_PBA<<pNandDrv->u8_BlkPageCntBits);
    if (u32_Err != UNFD_ST_SUCCESS)
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Mark Blk 0x%04x failed with ErrCode: 0x%08x\n",
            (unsigned int)u16_PBA, (unsigned int)u32_Err);

    //memset(au8_PageBuf, '\0', pNandDrv->u16_PageByteCnt);
    au8_SpareBuf[0]='\0';//pNandDrv->u16_SpareByteCnt);
    au8_SpareBuf[1]='\0';

    #if (defined(NC_SEL_FCIE5) && (NC_SEL_FCIE5))
    if(IF_LFSR_ENABLED())
    {u8_LFSR = 1;  NC_DisableLFSR();}
    #endif

    /* Clear the 1st page (main + spare) of the block */
    u32_PageIdx = u16_PBA << pNandDrv->u8_BlkPageCntBits;
    u32_Err = NC_WritePages(u32_PageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Mark Blk 0x%04x failed with ErrCode: 0x%08x\n",
            (unsigned int)u16_PBA, (unsigned int)u32_Err);
    /* Clear the last page (main + spare) of the block */
    u32_PageIdx += pNandDrv->u16_BlkPageCnt - 1;
    u32_Err = NC_WritePages(u32_PageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Mark Blk 0x%04x failed with ErrCode: 0x%08x\n",
            (unsigned int)u16_PBA, (unsigned int)u32_Err);

    #if (defined(NC_SEL_FCIE5) && (NC_SEL_FCIE5))
    if(u8_LFSR)
        NC_EnableLFSR();
    #endif

    return u32_Err;
}



static UNFD_ALIGN0 TEST_ALIGN_PACK_t g_TestAlignPack_t UNFD_ALIGN1;

U32 drvNAND_CheckAlignPack(U8 u8_AlignByteCnt)
{
    // check alignment
    if((U32)&(g_TestAlignPack_t.u8_0) & (u8_AlignByteCnt-1))
    {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR, not aliged. expect %X but %X \n",
            u8_AlignByteCnt, (U32)&(g_TestAlignPack_t.u8_0));
        //nand_die();
        return UNFD_ST_ERR_NOT_ALIGN;
    }
    // check packed - 0
    if((U32)&(g_TestAlignPack_t.u16_0)-(U32)&(g_TestAlignPack_t.u8_0) != 1 ||
       (U32)&(g_TestAlignPack_t.u32_0)-(U32)&(g_TestAlignPack_t.u8_0) != 3 ||
       (U32)&(g_TestAlignPack_t.u32_1)-(U32)&(g_TestAlignPack_t.u8_0) != 7)
    {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR, not packed. check err.0 \n");
        //nand_die();
        return UNFD_ST_ERR_NOT_PACKED;
    }
    // check packed - 1
    if((U32)&(g_TestAlignPack_t.u16_0)-(U32)&(g_TestAlignPack_t.u8_0) != 1 ||
       (U32)&(g_TestAlignPack_t.u32_0)-(U32)&(g_TestAlignPack_t.u16_0)!= 2 ||
       (U32)&(g_TestAlignPack_t.u32_1)-(U32)&(g_TestAlignPack_t.u32_0)!= 4)
    {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR, not packed. check err.1 \n");
        //nand_die();
        return UNFD_ST_ERR_NOT_PACKED;
    }

    nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "ok\n");
    return UNFD_ST_SUCCESS;
}


U8 drvNAND_CheckAll0xFF(U8* pu8_Buf, U32 u32_ByteCnt)
{
    register U32 u32_i;
    #if 1 // faster
    register U32 u32_j, *pu32;
    register U8 *pu8;

    pu8 = pu8_Buf;
    pu32 = (U32*)((U32)(pu8_Buf + 3) & ~3);
    u32_ByteCnt -= (U32)pu32 - (U32)pu8;

    for(; (U32)pu8<(U32)pu32; pu8++)
        if(0xFF != *pu8)
            return 0;

    u32_j = u32_ByteCnt>>2;

    for(u32_i=0; u32_i < u32_j; u32_i++)
        if(0xFFFFFFFF != pu32[u32_i])
            return 0;

    if(u32_ByteCnt == u32_i<<2)
        return 0xFF;

    u32_ByteCnt -= u32_i<<2;
    pu8 = (U8*)&pu32[u32_i];

    for(u32_i=0; u32_i < u32_ByteCnt; u32_i++)
        if(0xFF != pu8[u32_i])
            return 0;
    #else
    for(u32_i=0; u32_i < u32_ByteCnt; u32_i++)
        if(0xFF != pu8_Buf[u32_i])
            return 0;
    #endif
    return 0xFF;
}


// return free LSB Idx
U16 drvNAND_FindFreeLSBPageIdx(U16 u16_PBA) // binary search
{
    PAIRED_PAGE_MAP_t *pPairedPageMap = drvNAND_get_DrvContext_PPM();
    NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
    U8  *pu8_SpareBuf = gpu8_SpareBuf;
    U8  *pu8_DataBuf = gpu8_PageDataBuf;
    U32 u32_Row, u32_Err;
    U16 u16_PIdx1, u16_PIdx2, u16_PIdxN;
    U16 u16_BlkPageCnt;

    //----------------------------------
    // binary search

    u16_BlkPageCnt = pNandDrv->u16_BlkLowPCnt;//pNandDrv->u16_BlkPageCnt >> pNandDrv->u8_CellType;
    u16_PIdx1 = 0;
    u16_PIdx2 = u16_BlkPageCnt - 1;
    u32_Row = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + pPairedPageMap[u16_PIdx1].u16_LSB;
    //u32_Err = NC_ReadPages(u32_Row, pu8_DataBuf, pu8_SpareBuf, 1);
    u32_Err = NC_ReadSectors(u32_Row, 0, pu8_DataBuf, pu8_SpareBuf, 1);
    if(0xFF == drvNAND_CheckAll0xFF(pu8_SpareBuf, pNandDrv->u16_SectorSpareByteCnt)
            && u32_Err == UNFD_ST_SUCCESS)
        return 0;

    u32_Row = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + pPairedPageMap[u16_PIdx2].u16_LSB;
    //u32_Err = NC_ReadPages(u32_Row, pu8_DataBuf, pu8_SpareBuf, 1);
    u32_Err = NC_ReadSectors(u32_Row, 0, pu8_DataBuf, pu8_SpareBuf, 1);
    if(0 == drvNAND_CheckAll0xFF(pu8_SpareBuf, pNandDrv->u16_SectorSpareByteCnt)
        && u32_Err == UNFD_ST_SUCCESS)
        return u16_BlkPageCnt;//u16_PIdx2;

    while( u16_PIdx1 < u16_PIdx2 -1)
    {
        u16_PIdxN = (u16_PIdx1+u16_PIdx2)/2;

        u32_Row = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + pPairedPageMap[u16_PIdxN].u16_LSB;
        //read page pn
        //u32_Err = NC_ReadPages(u32_Row, pu8_DataBuf, pu8_SpareBuf,  1);
        u32_Err = NC_ReadSectors(u32_Row, 0, pu8_DataBuf, pu8_SpareBuf, 1);
        if(0xFF == drvNAND_CheckAll0xFF(pu8_SpareBuf, pNandDrv->u16_SectorSpareByteCnt)
            && u32_Err == UNFD_ST_SUCCESS)
            u16_PIdx2 = u16_PIdxN;
        else
            u16_PIdx1 = u16_PIdxN;
    }

    //u32_Row = ((u16_PBA << pNandDrv->u8_BlkPageCntBits) +pPairedPageMap[u16_PIdx1 + 1].u16_LSB);

    return u16_PIdx1 + 1; //(u32_Row & pNandDrv->u16_BlkPageCntMask);
}



U8 gau8_OneBitCnt[256]={
// 0   1   2   3   4   5   6   7   8   9   A   B   C  D   E   F
   0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, // 0
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 1
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 2
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 3
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 4
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 5
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 6
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // 7
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, // 8
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // 9
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // A
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // B
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, // C
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // D
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, // E
   4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8  // F
};
U8 gau8_ZeroBitCnt[256]={
// 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
   8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4, // 0
   7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, // 1
   7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, // 2
   6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, // 3
   7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, // 4
   6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, // 5
   6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, // 6
   5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, // 7
   7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, // 8
   6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, // 9
   6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, // A
   5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, // B
   6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, // C
   5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, // D
   5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, // E
   4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0  // F
};
 /*
  * Check whether the cause of ECC fail is reading a non-all0xff empty page.
  * @main: Main Data of a NAND Page
  * @spare: Spare Data of a NAND Page
  * @return 1: if False Alarm 0: Real ECC Fail
  */
int nand_CheckEmptyPageFalseAlarm(U8 *main, U8 *spare)
{
    NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
    int  i, j, ECCErrBitCnt;

    //check if false alarm casued by empty page
    for(j = 0; j < pNandDrv->u16_PageSectorCnt; j++)
    {
        ECCErrBitCnt = 0;
        for(i = 0; i < pNandDrv->u16_SectorSpareByteCnt; i++)
        {
            if(j*pNandDrv->u16_SectorSpareByteCnt + i < 512)
                ECCErrBitCnt += gau8_ZeroBitCnt[spare[ j*pNandDrv->u16_SectorSpareByteCnt + i]];
            if(ECCErrBitCnt > pNandDrv->u16_ECCCorretableBit)
                return 0;
        }

        for(i = 0; i < pNandDrv->u16_SectorByteCnt; i++)
        {
            ECCErrBitCnt += gau8_ZeroBitCnt[main[ j*pNandDrv->u16_SectorByteCnt + i]];
            if(ECCErrBitCnt > pNandDrv->u16_ECCCorretableBit)
                return 0;
        }
    }
    memset(spare, 0xFF, pNandDrv->u16_SpareByteCnt);
    memset(main, 0xFF, pNandDrv->u16_PageByteCnt);
    return 1;
}

 int nand_CheckEmptySectorsFalseAlarm(U8 *main, U8 *spare, U16 u16_SectorCnt)
{
    NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
    int  i, j, ECCErrBitCnt;

    //check if false alarm casued by empty page
    for(j = 0; j < u16_SectorCnt; j++)
    {
        ECCErrBitCnt = 0;
        for(i = 0; i < pNandDrv->u16_SectorSpareByteCnt; i++)
        {
            if(j*pNandDrv->u16_SectorSpareByteCnt + i < 512)
                ECCErrBitCnt += gau8_ZeroBitCnt[spare[ j*pNandDrv->u16_SectorSpareByteCnt + i]];
            if(ECCErrBitCnt > pNandDrv->u16_ECCCorretableBit)
                return 0;
        }

        for(i = 0; i < pNandDrv->u16_SectorByteCnt; i++)
        {
            ECCErrBitCnt += gau8_ZeroBitCnt[main[ j*pNandDrv->u16_SectorByteCnt + i]];
            if(ECCErrBitCnt > pNandDrv->u16_ECCCorretableBit)
                return 0;
        }
    }
     memset(spare, 0xFF, pNandDrv->u16_SectorSpareByteCnt*u16_SectorCnt);
     memset(main, 0xFF, pNandDrv->u16_SectorByteCnt*u16_SectorCnt);
    return 1;
}


#endif
