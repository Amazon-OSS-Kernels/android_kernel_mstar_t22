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
#include <command.h>
#include <common.h>
#include <malloc.h>
#include <stdio.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <MsTypes.h>
#include <MsDebug.h>
#include <dmalloc.h>
#include <ms_utils.h>
#include <demura.h>
//#include <convert_entry.h>

#include "vendor.h"
#include "parse_header.h"

//G(x) = X16 + X15 + X2 + 1
//CRC16 G(x): 0x8005
const MS_U16 DEMURA_CRC16_TABLE[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

MS_U16 CalcCRC16(MS_U8 *pData,MS_U32 len)
{
    MS_U8 dat;
    MS_U16 crcData = 0x0000;
    while(len--){
        dat = (MS_U8)(crcData>>8);
        crcData <<= 8;
        crcData ^= DEMURA_CRC16_TABLE[dat^*pData++];
    }
    return crcData;
}

MS_U16 get_CSOT_CSOT_CRC(E_FILE_CRC eCRCType)
{
    MS_U8   buffer[2];
    MS_BOOL bRet;
    MS_U16  u16Crc = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("Init_spi_flash error!\n");
        return u16Crc;
    }

    switch (eCRCType)
    {
        case E_FILE_TATOL_CRC:
            bRet = read_spi_flash(buffer, 0x1C, sizeof(buffer));
            break;
        case E_FILE_HEADER_CRC:
            bRet = read_spi_flash(buffer, 0x1E, sizeof(buffer));
            break;
        case E_DEMURA_PARA_CRC:
            bRet = read_spi_flash(buffer, 0x2C, sizeof(buffer));
            break;
        case E_DEMURA_LUT_CRC:
            bRet = read_spi_flash(buffer, 0x3C, sizeof(buffer));
            break;
        default:
            bRet = FALSE;
            break;
    }

    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT CRC from spi flash failed !\n");
        return u16Crc;
    }

    u16Crc = ((MS_U16)buffer[1] << 8) | buffer[0];
    return u16Crc;
}


MS_BOOL load_vendor_header(CSOT_CSOT_Demura_Header *phdr)
{
    MS_U16 read_len = HEADER_LEN ;
    MS_U8 *buffer = (MS_U8 *)malloc(read_len);
    MS_U16 demura_para_crc = 0;
    MS_U16 demura_para_crc_cal = 0;

    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT Header from spi flash failed !\n");
        free(buffer);
        return FALSE;
    }

    demura_para_crc = get_CSOT_CSOT_CRC(2);
    demura_para_crc_cal = CalcCRC16((buffer + 0x40),(read_len - 0x40));
    UBOOT_DEBUG("CSOT_CSOT demura_para_crc_cal CRC = 0x%x \n", demura_para_crc_cal);
    if(demura_para_crc != demura_para_crc_cal)
    {
        UBOOT_ERROR("=====CSOT_CSOT demura para crc CRC FAIL=====\n");
        dfree(buffer);
        return FALSE;
    }
    else
        UBOOT_DEBUG("=====CSOT_CSOT demura para crc CRC OK=====\n");

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header(CSOT_CSOT_Demura_Header *phdr)
{
    MS_U8  *buffer = phdr->hdr_buffer;
    CSOT_CSOT_BIN_Header *pbhdr = (CSOT_CSOT_BIN_Header *)buffer;

    UBOOT_TRACE("IN\n");

    phdr->DEMURA_MODE = pbhdr->enDemuraMode;
    if (( pbhdr->u8PlaneNum >= 1) && ( pbhdr->u8PlaneNum <= 8))
    {
         phdr->DEMURA_PLANE_NUM = pbhdr->u8PlaneNum;
    }
    else
    {
        phdr->DEMURA_PLANE_NUM = 3;
    }

    if (pbhdr->u8HblockSize == 4)  // 2:4pixs 3:8pixs 4:16pixs
    {
        phdr->DEMURA_BLK_H = 2;
    }
    else if (pbhdr->u8HblockSize == 8)
    {
        phdr->DEMURA_BLK_H = 3;
    }
    else if (pbhdr->u8HblockSize == 16)
    {
        phdr->DEMURA_BLK_H = 4;
    }

    if (pbhdr->u8VblockSize == 4)  // 2:4pixs 3:8pixs 4:16pixs
    {
        phdr->DEMURA_BLK_V = 2;
    }
    else if (pbhdr->u8VblockSize == 8)
    {
        phdr->DEMURA_BLK_V = 3;
    }
    else if (pbhdr->u8VblockSize == 16)
    {
        phdr->DEMURA_BLK_V = 4;
    }

    phdr->DEMURA_TBL_H = (pbhdr->u16HLutNum[1] << 8) | pbhdr->u16HLutNum[0];
    phdr->DEMURA_TBL_V = (pbhdr->u16VLutNum[1] << 8) | pbhdr->u16VLutNum[0];
    phdr->lut_length = (pbhdr->lut_size[3] << 24) | (pbhdr->lut_size[2] << 16 ) | (pbhdr->lut_size[1] << 8) | pbhdr->lut_size[0];
    phdr->TABLE_CRC = ( pbhdr->lut_section_CRC[1] << 8) | pbhdr->lut_section_CRC[0];
    phdr->DEMURA_BLACK_LIMIT = ((MS_U16)pbhdr->u16BlackLimitR[1] << 8) | pbhdr->u16BlackLimitR[0];
    phdr->DEMURA_PLANE_LEVEL1 = ((MS_U16)pbhdr->u16Plane1LevelR[1] << 8) | pbhdr->u16Plane1LevelR[0];
    phdr->DEMURA_PLANE_LEVEL2 = ((MS_U16)pbhdr->u16Plane2LevelR[1] << 8) | pbhdr->u16Plane2LevelR[0];
    phdr->DEMURA_PLANE_LEVEL3 = ((MS_U16)pbhdr->u16Plane3LevelR[1] << 8) | pbhdr->u16Plane3LevelR[0];
    phdr->DEMURA_PLANE_LEVEL4 = ((MS_U16)pbhdr->u16Plane4LevelR[1] << 8) | pbhdr->u16Plane4LevelR[0];
    phdr->DEMURA_PLANE_LEVEL5 = ((MS_U16)pbhdr->u16Plane5LevelR[1] << 8) | pbhdr->u16Plane5LevelR[0];
    phdr->DEMURA_PLANE_LEVEL6 = ((MS_U16)pbhdr->u16Plane6LevelR[1] << 8) | pbhdr->u16Plane6LevelR[0];
    phdr->DEMURA_PLANE_LEVEL7 = ((MS_U16)pbhdr->u16Plane7LevelR[1] << 8) | pbhdr->u16Plane7LevelR[0];
    phdr->DEMURA_PLANE_LEVEL8 = ((MS_U16)pbhdr->u16Plane8LevelR[1] << 8) | pbhdr->u16Plane8LevelR[0];
    phdr->DEMURA_WHITE_LIMIT = ((MS_U16)pbhdr->u16WhiteLimitR[1] << 8) | pbhdr->u16WhiteLimitR[0];
    phdr->DEMURA_GAIN_R = ((MS_U16)pbhdr->u16CompValGainR[1] << 8) | pbhdr->u16CompValGainR[0];
    phdr->DEMURA_GAIN_G = ((MS_U16)pbhdr->u16CompValGainG[1] << 8) | pbhdr->u16CompValGainG[0];
    phdr->DEMURA_GAIN_B = ((MS_U16)pbhdr->u16CompValGainB[1] << 8) | pbhdr->u16CompValGainB[0];
    phdr->DEMURA_OFFSET_R = ((MS_U16)pbhdr->u16CompValOffsetR[1] << 8) | pbhdr->u16CompValOffsetR[0];
    phdr->DEMURA_OFFSET_G = ((MS_U16)pbhdr->u16CompValOffsetG[1] << 8) | pbhdr->u16CompValOffsetG[0];
    phdr->DEMURA_OFFSET_B = ((MS_U16)pbhdr->u16CompValOffsetB[1] << 8) | pbhdr->u16CompValOffsetB[0];
    phdr->DEMURA_INT_BIT_WIDTH = pbhdr->u8IntBitWidth;
    phdr->DEMURA_DEC_BIT_WIDTH= pbhdr->u8DecimalsBitWidth;

    UBOOT_TRACE("OK\n");
    return TRUE;
}

void dump_vendor_header(CSOT_CSOT_Demura_Header *phdr)
{
    printf("========Demura Header Info========\n");
    printf("DEMURA_PLANE_NUM    :   0x%x \n", phdr->DEMURA_PLANE_NUM);
    printf("DEMURA_MODE         :   0x%x \n", phdr->DEMURA_MODE);
    printf("DEMURA_TBL_H        :   0x%x \n", phdr->DEMURA_TBL_H);
    printf("DEMURA_TBL_V        :   0x%x \n", phdr->DEMURA_TBL_V);
    printf("DEMURA_BLK_H        :   0x%x \n", phdr->DEMURA_BLK_H);
    printf("DEMURA_BLK_V        :   0x%x \n", phdr->DEMURA_BLK_V);
    printf("DEMURA_BLACK_LIMIT  :   0x%x \n", phdr->DEMURA_BLACK_LIMIT);
    printf("DEMURA_PLANE_LEVEL1 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL1);
    printf("DEMURA_PLANE_LEVEL2 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL2);
    printf("DEMURA_PLANE_LEVEL3 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL3);
    printf("DEMURA_PLANE_LEVEL4 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL4);
    printf("DEMURA_PLANE_LEVEL5 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL5);
    printf("DEMURA_PLANE_LEVEL6 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL6);
    printf("DEMURA_PLANE_LEVEL7 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL7);
    printf("DEMURA_PLANE_LEVEL8 :   0x%x \n", phdr->DEMURA_PLANE_LEVEL8);
    printf("DEMURA_WHITE_LIMIT  :   0x%x \n", phdr->DEMURA_WHITE_LIMIT);
    printf("TABLE_CRC           :   0x%x \n", phdr->TABLE_CRC);
    printf("DEMURA_TABLE_LENGTH :   %u \n", (unsigned int)phdr->lut_length);
}

MS_BOOL load_vendor_lut(CSOT_CSOT_Demura_Header *phdr)
{
    MS_U32 lut_size   = phdr->lut_length;
    MS_U32 lut_checksum = phdr->TABLE_CRC;
    MS_U16 crc = 0, crc_read = (MS_U16)lut_checksum;
    MS_U8  *lut_buf = dmalloc(lut_size);
    UBOOT_TRACE("IN\n");

    MS_BOOL bRet = read_spi_flash(lut_buf, LUT_START, lut_size);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    crc = CalcCRC16(lut_buf, lut_size);
    UBOOT_DEBUG("CSOT_CSOT lut CRC = 0x%x \n", crc);
    if(crc != crc_read)
    {
        UBOOT_ERROR("=====CSOT_CSOT lut CRC FAIL=====\n");
        dfree(lut_buf);
        return FALSE;
    }
    else
        UBOOT_DEBUG("=====CSOT_CSOT lut CRC OK=====\n");

    phdr->lut_buffer = lut_buf;
    UBOOT_TRACE("OK\n");
    return TRUE;
}

