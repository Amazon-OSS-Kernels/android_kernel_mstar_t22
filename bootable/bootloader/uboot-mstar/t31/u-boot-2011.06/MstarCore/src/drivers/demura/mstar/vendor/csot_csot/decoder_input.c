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
#include <stdio.h>
#include <MsDebug.h>
#include <MsTypes.h>
#include <ShareType.h>
#include <malloc.h>

#include <CommonDataType.h>
#include <dmalloc.h>
#include <read_text.h>
#include <demura.h>
#include <ms_utils.h>

#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include <convert_entry.h>
#include "vendor.h"
#include "crc_libs.h"
#include "demura_config.h"

#define SAMPLE_DAT_START    1024     // Byte
#define SAMPLE_DAT_LEN      4096     // Byte
#define FLASH_MAX_SIZE   (1024 * 1024)

#define sscanf_s sscanf
extern int snprintf(char *str, size_t size, const char *fmt, ...);

typedef union
{
    MS_U32 block_data;
    unsigned char data[4];

    struct
    {
        int nReserved:8;
        int b3:6;
        int b2:6;
        int b1:6;
        int b0:6;
    };
}DATA;

static MS_BOOL Gen_Sf_Signature(CSOT_CSOT_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", phdr->TABLE_CRC, sample_crc32);
    setenv(ENV_DEMURA_SIG, strbuf);
    saveenv();
    return TRUE;
}


MS_BOOL If_Need_Decode(void)
{
    UBOOT_TRACE("IN\n");

    char *sig_str = getenv(ENV_DEMURA_SIG);
    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        //lut_checksum_sf = get_lut_checksum();
        lut_checksum_sf = get_CSOT_CSOT_CRC(3);

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf_s(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

static void dump_interface_info(interface_info *pDataInfo)
{
    printf("PLANE_NUM = %d  \n", pDataInfo->reg_dmc_plane_num);
    printf("H_BLOCK = %d, V_BLOCK = %d  \n", pDataInfo->reg_dmc_h_block, pDataInfo->reg_dmc_v_block);
    printf("RGB_MODE = %d   \n", pDataInfo->reg_dmc_rgb_mode);
    printf("PANEL_H_SIZE = %d   \n", pDataInfo->reg_dmc_panel_h_size);
    printf("BLACK_LIMIT = 0x%x, WHITE_LIMIT = 0x%x  \n", pDataInfo->reg_dmc_black_limit, pDataInfo->reg_dmc_white_limit);

    printf("PLANE_LEVEL 1 = 0x%x   \n", pDataInfo->reg_dmc_plane_level1);
    printf("PLANE_LEVEL 2 = 0x%x   \n", pDataInfo->reg_dmc_plane_level2);
    printf("PLANE_LEVEL 3 = 0x%x   \n", pDataInfo->reg_dmc_plane_level3);
    printf("PLANE_LEVEL 4 = 0x%x   \n", pDataInfo->reg_dmc_plane_level4);
    printf("PLANE_LEVEL 5 = 0x%x   \n", pDataInfo->reg_dmc_plane_level5);
    printf("PLANE_LEVEL 6 = 0x%x   \n", pDataInfo->reg_dmc_plane_level6);
    printf("PLANE_LEVEL 7 = 0x%x   \n", pDataInfo->reg_dmc_plane_level7);
    printf("PLANE_LEVEL 8 = 0x%x   \n", pDataInfo->reg_dmc_plane_level8);

    printf("DITHER_4_FRAME = %d, DITHER_RST_NUM = %d, DITHER_EN = %d    \n",
        pDataInfo->reg_dmc_dither_rst_e_4_frame, pDataInfo->reg_dmc_dither_pse_rst_num, pDataInfo->reg_dmc_dither_en);

    printf("ROI_EN = %d, ROI_H_START = %d, ROI_H_END = %d, ROI_V_START = %d, ROI_V_END = %d \n",
        pDataInfo->bROI_en, pDataInfo->iROI_hor_start_offset, pDataInfo->iROI_hor_end_offset,
        pDataInfo->iROI_ver_start_offset, pDataInfo->iROI_ver_end_offset);
}

int Get_INDEX(int *array, MS_U8 array_num)
{
    int i = 0;
    int min = array[0];
    for(i = 0; i < array_num; i++)
    {
        if(array[i] < min)
            min = i;
    }
    return min;
}

static void CSOT_CSOT_Init_U13_Param(CSOT_CSOT_Demura_Header *phdr, interface_info *pDataInfo)
{
    CSOT_CSOT_Demura_Header header;
    MS_U16 n_Hnode = 0;

    memcpy(&header, phdr, sizeof(CSOT_CSOT_Demura_Header));

    n_Hnode = header.DEMURA_TBL_H;
    
    pDataInfo->reg_dmc_h_block            = header.DEMURA_BLK_H;         //  3 bit
    pDataInfo->reg_dmc_v_block            = header.DEMURA_BLK_V;         //  3 bit

    pDataInfo->reg_dmc_plane_num              = header.DEMURA_PLANE_NUM;         //  4 bit
    pDataInfo->reg_dmc_rgb_mode               = header.DEMURA_MODE;         //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = (n_Hnode - 1)*(1 << (pDataInfo->reg_dmc_h_block));//H_node = H_size/H_block_size + 1;       // 13 bit
    pDataInfo->reg_dmc_black_limit            = header.DEMURA_BLACK_LIMIT<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = header.DEMURA_PLANE_LEVEL1<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = header.DEMURA_PLANE_LEVEL2<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = header.DEMURA_PLANE_LEVEL3<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = header.DEMURA_PLANE_LEVEL4<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = header.DEMURA_PLANE_LEVEL5<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = header.DEMURA_PLANE_LEVEL6<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = header.DEMURA_PLANE_LEVEL7<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = header.DEMURA_PLANE_LEVEL8<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = header.DEMURA_WHITE_LIMIT<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = 0x1;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    pDataInfo->bROI_en                        = 0x0;
    pDataInfo->iROI_hor_end_offset            = 0x0;
    pDataInfo->iROI_hor_start_offset          = 0x0;
    pDataInfo->iROI_ver_end_offset            = 0x0;
    pDataInfo->iROI_ver_start_offset          = 0x0;
}

MS_BOOL CSOT_CSOT_Set_U13_Format(CSOT_CSOT_Demura_Header *phdr, interface_info *pinfo)
{
    // CSOT only porting mono mode.
    int i, j, k, c, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    int burst_num = 0, data_per_burst = 0, data_last_burst = 0, skip_last_byte = 0, burst_bit = 128;
    MS_BOOL pol = 0;
    MS_U8 IntBitWidth = 0, DecimalsBitWidth = 0, DataBit = CSOT_CSOT_DATA_BIT_NUM;
    //MS_U16 gain = 0, offset = 0;
    MS_U16 gain[3] = {0}, offset[3] = {0};
    double LUT_data = 0;

    CSOT_CSOT_Init_U13_Param(phdr, pinfo);
        
    data_per_burst = burst_bit/DataBit; // 128bit per burst, 12bit per data => 10 data and 8bit dummy byte.
    //printf("data_per_burst = %d \n", data_per_burst);
    burst_num = (phdr->DEMURA_TBL_H/data_per_burst) + ((phdr->DEMURA_TBL_H%data_per_burst > 0)?1:0);
    //printf("burst_num = %d \n", burst_num);
    data_last_burst = phdr->DEMURA_TBL_H%data_per_burst;
    //printf("data_last_dummy = %d \n", data_last_burst);
    skip_last_byte = burst_bit/8 - data_last_burst;
    //printf("skip_last_byte = %d \n", skip_last_byte);

    IntBitWidth = phdr->DEMURA_INT_BIT_WIDTH;
    DecimalsBitWidth = phdr->DEMURA_DEC_BIT_WIDTH;
    //printf("IntBitWidth = %d, DecimalsBitWidth = %d \n", IntBitWidth, DecimalsBitWidth);

    // Get CSOT gain and offset value
    gain[0] = phdr->DEMURA_GAIN_R;
    offset[0] = phdr->DEMURA_OFFSET_R;
    gain[1] = phdr->DEMURA_GAIN_G;
    offset[1] = phdr->DEMURA_OFFSET_G;
    gain[2] = phdr->DEMURA_GAIN_B;
    offset[2] = phdr->DEMURA_OFFSET_B;

    if(phdr->DEMURA_MODE== 1) // RGB mode
    {
        // assign LUT 12 bit format
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for(k = 0; k < pinfo->reg_dmc_plane_num; k++)
            {
                for(c = 0; c < 3; c++)
                {
                    // RGB data
                    for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
                    {				  
                        // LUT index for the plane
                        idx_image_size = i*phdr->DEMURA_TBL_H+j;

                        // 12 bit each for 10 bit value
                        if(pol == 0) //8bit + 4bit
                        {
                            data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                            pol = 1;
                            table_addr+=1;
                        }
                        else
                        {
                            data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                            pol = 0;
                            table_addr+=2;
                        }

                        if((j+1)%data_per_burst == 0) // The last data of the burst
                        {
                            table_addr++;
                            pol = 0;
                        }
                        if(j == (phdr->DEMURA_TBL_H - 1)) // The last data of the H
                        {
                            table_addr+=skip_last_byte; // skip dummy 15 bytes
                            pol = 0;
                        }

                        if((data & _BIT11) == _BIT11) // signed bit
                            data = (0x1000 - data)*(-1);

                        switch(k)
                        {
                            case 0:
                                nLayer = pinfo->reg_dmc_plane_level1;
                                break;
                            case 1:
                                nLayer = pinfo->reg_dmc_plane_level2;
                                break;
                            case 2:
                                nLayer = pinfo->reg_dmc_plane_level3;
                                break;
                            case 3:
                                nLayer = pinfo->reg_dmc_plane_level4;
                                break;
                            case 4:
                                nLayer = pinfo->reg_dmc_plane_level5;
                                break;
                            case 5:
                                nLayer = pinfo->reg_dmc_plane_level6;
                                break;
                            case 6:
                                nLayer = pinfo->reg_dmc_plane_level7;
                                break;
                            case 7:
                                nLayer = pinfo->reg_dmc_plane_level8;
                                break;
                            default:
                                break;
                        }

                        // LUT_comp_value = LUT_data*gain/16 + offset
                        LUT_data = ((double)(data*gain[c]/16.0 + offset[c])/(1<<DecimalsBitWidth)) + (double)(nLayer/4.0); // 10bit data + layer (12bit to 10bit)

                        switch(c)
                        {
                            case 0 :
                                // R data
                                pinfo->Lut_in[k][idx_image_size].dbr   = LUT_data;
                                pinfo->Lut_in[k][idx_image_size].r   = (int)LUT_data;
                                break;
                            case 1 :
                                // G data
                                pinfo->Lut_in[k][idx_image_size].dbg   = LUT_data;
                                pinfo->Lut_in[k][idx_image_size].g   = (int)LUT_data;
                                break;
                            case 2 :
                                // B data
                                pinfo->Lut_in[k][idx_image_size].dbb   = LUT_data;
                                pinfo->Lut_in[k][idx_image_size].b   = (int)LUT_data;
                                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                                pinfo->Lut_in[k][idx_image_size].dbw   = (pinfo->Lut_in[k][idx_image_size].dbr + pinfo->Lut_in[k][idx_image_size].dbg + pinfo->Lut_in[k][idx_image_size].dbb)/3;
                                pinfo->Lut_in[k][idx_image_size].w   = (int)pinfo->Lut_in[k][idx_image_size].dbw;
                                #endif
                                break;
                            default :
                                break;
                        }
                    }
                }
            }
        }
    }
    else // Mono mode
    {
        // assign LUT 12 bit format
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for(k = 0; k < pinfo->reg_dmc_plane_num; k++)
            {
                for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
                {				  
                    // LUT index for the plane
                    idx_image_size = i*phdr->DEMURA_TBL_H+j;

                    // 12 bit each for 10 bit value
                    if(pol == 0) //8bit + 4bit
                    {
                        data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                        pol = 1;
                        table_addr+=1;
                    }
                    else
                    {
                        data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                        pol = 0;
                        table_addr+=2;
                    }

                    if((j+1)%data_per_burst == 0) // The last data of the burst
                    {
                        table_addr++;
                        pol = 0;
                    }
                    if(j == (phdr->DEMURA_TBL_H - 1)) // The last data of the H
                    {
                        table_addr+=skip_last_byte; // skip dummy 15 bytes
                        pol = 0;
                    }

                    if((data & _BIT11) == _BIT11) // signed bit
                        data = (0x1000 - data)*(-1);

                    switch(k)
                    {
                        case 0:
                            nLayer = pinfo->reg_dmc_plane_level1;
                            break;
                        case 1:
                            nLayer = pinfo->reg_dmc_plane_level2;
                            break;
                        case 2:
                            nLayer = pinfo->reg_dmc_plane_level3;
                            break;
                        case 3:
                            nLayer = pinfo->reg_dmc_plane_level4;
                            break;
                        case 4:
                            nLayer = pinfo->reg_dmc_plane_level5;
                            break;
                        case 5:
                            nLayer = pinfo->reg_dmc_plane_level6;
                            break;
                        case 6:
                            nLayer = pinfo->reg_dmc_plane_level7;
                            break;
                        case 7:
                            nLayer = pinfo->reg_dmc_plane_level8;
                            break;
                        default:
                            break;
                    }

                    // LUT_comp_value = LUT_data*gain/16 + offset
                    LUT_data = ((double)(data*gain[0]/16.0 + offset[0])/(1<<DecimalsBitWidth)) + (double)(nLayer/4.0); // 10bit data + layer (12bit to 10bit)

                    pinfo->Lut_in[k][idx_image_size].dbr   = LUT_data;
                    pinfo->Lut_in[k][idx_image_size].dbg   = LUT_data;
                    pinfo->Lut_in[k][idx_image_size].dbb   = LUT_data;
                    #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                    pinfo->Lut_in[k][idx_image_size].dbw   = LUT_data;
                    #endif

                    pinfo->Lut_in[k][idx_image_size].r   = (int)LUT_data;
                    pinfo->Lut_in[k][idx_image_size].g   = (int)LUT_data;
                    pinfo->Lut_in[k][idx_image_size].b   = (int)LUT_data;
                    #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                    pinfo->Lut_in[k][idx_image_size].w   = (int)LUT_data;
                    #endif
                }
            }
        }
    }
    
    return TRUE;
}

static MS_BOOL CSOT_CSOT_Decode_To_Mstar_Format(CSOT_CSOT_Demura_Header *phdr, interface_info *pinfo)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
    #else
        bRet = CSOT_CSOT_Set_U13_Format(phdr, pinfo);
    #endif

    return bRet;
}

MS_BOOL Decode_To_Mstar_Format(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet;
    CSOT_CSOT_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header(&header) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        return FALSE;
    }
    dump_vendor_header(&header);

    if (load_vendor_lut(&header) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        return FALSE;
    }

    Gen_Sf_Signature(&header);
    // Alloc space for Lut_in
    int n_Hnode = header.DEMURA_TBL_H;
    int n_Vnode = header.DEMURA_TBL_V;

    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    pbin_info->LevelCount     = header.DEMURA_PLANE_NUM;
    pbin_info->Blk_h_size     = 1 << (header.DEMURA_BLK_H);
    pbin_info->Blk_v_size     = 1 << (header.DEMURA_BLK_V);

    dump_interface_info(pDataInfo);

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        return FALSE;
    }

    // Decode Vendor data
    bRet = CSOT_CSOT_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("CSOT_CSOT_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

    #if defined (CONFIG_DEMURA_URSA13)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + ID_CUS_MSTAR);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Sep_type       =  header.DEMURA_MODE;
    pbin_info->Build_Date     =  0x20060214;

    memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}


