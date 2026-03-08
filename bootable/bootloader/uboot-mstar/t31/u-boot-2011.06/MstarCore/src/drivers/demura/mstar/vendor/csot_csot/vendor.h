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
#ifndef _VENDOR_H_
#define _VENDOR_H_

#define HEADER_START   0
#define HEADER_LEN     0x110
#define LUT_START      0x110
#define CSOT_CSOT_DATA_BIT_NUM 12

typedef enum
{
    E_MONO_MODE  = 0,
    E_RGB_MODE   = 1,
} E_COLOR_MODE;

typedef enum
{
    E_FILE_TATOL_CRC = 0,
    E_FILE_HEADER_CRC,
    E_DEMURA_PARA_CRC,
    E_DEMURA_LUT_CRC,
} E_FILE_CRC;

typedef struct
{
    MS_U8           DEMURA_PLANE_NUM;
    MS_BOOL         DEMURA_MODE;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U32          TABLE_CRC;
    MS_U16          DEMURA_BLACK_LIMIT;
    MS_U16          DEMURA_PLANE_LEVEL1;
    MS_U16          DEMURA_PLANE_LEVEL2;
    MS_U16          DEMURA_PLANE_LEVEL3;
    MS_U16          DEMURA_PLANE_LEVEL4;
    MS_U16          DEMURA_PLANE_LEVEL5;
    MS_U16          DEMURA_PLANE_LEVEL6;
    MS_U16          DEMURA_PLANE_LEVEL7;
    MS_U16          DEMURA_PLANE_LEVEL8;
    MS_U16          DEMURA_WHITE_LIMIT;
    MS_U16          DEMURA_GAIN_R;
    MS_U16          DEMURA_GAIN_G;
    MS_U16          DEMURA_GAIN_B;
    MS_U16          DEMURA_OFFSET_R;
    MS_U16          DEMURA_OFFSET_G;
    MS_U16          DEMURA_OFFSET_B;
    MS_U8           DEMURA_INT_BIT_WIDTH;
    MS_U8           DEMURA_DEC_BIT_WIDTH;
    // Header Buffer
    MS_U8           *hdr_buffer;
    // Lut Buffer
    MS_U8           *lut_buffer;
    MS_U32          lut_offset;
    MS_U32          lut_length;
}CSOT_CSOT_Demura_Header;

MS_U16 get_CSOT_CSOT_CRC(E_FILE_CRC eCRCType);
MS_BOOL load_vendor_header(CSOT_CSOT_Demura_Header *phdr);
MS_BOOL parse_vendor_header(CSOT_CSOT_Demura_Header *phdr);
void dump_vendor_header(CSOT_CSOT_Demura_Header *phdr);
MS_BOOL load_vendor_lut(CSOT_CSOT_Demura_Header *phdr);

#endif
