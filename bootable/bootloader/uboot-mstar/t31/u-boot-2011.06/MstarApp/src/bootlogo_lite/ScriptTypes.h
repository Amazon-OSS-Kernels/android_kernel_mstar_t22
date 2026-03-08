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
#ifndef __SCRIPTTYPES_H__
#define __SCRIPTTYPES_H__

typedef enum
{
    E_OUTPUT_TIMING_720X480_60P = 0,
    E_OUTPUT_TIMING_720X576_50P,// 1
    E_OUTPUT_TIMING_1280X720_50P,// 2
    E_OUTPUT_TIMING_1280X720_60P,// 3
    E_OUTPUT_TIMING_1920X1080_50P,// 4
    E_OUTPUT_TIMING_1920X1080_60P,//5 
    E_OUTPUT_TIMING_720X480_60I,//6
    E_OUTPUT_TIMING_720X576_50I,//7
    E_OUTPUT_TIMING_1920X1080_50I,// 8
    E_OUTPUT_TIMING_1920X1080_60I,//9
    E_OUTPUT_TIMING_AUTO          = 255,
}EN_OUTPUT_TIMING_TYPE;


typedef enum
{
    E_OUTPUT_RESOLUTION_720X480,
    E_OUTPUT_RESOLUTION_720X576,
    E_OUTPUT_RESOLUTION_1280X720,
    E_OUTPUT_RESOLUTION_1920X1080,
}_EN_OUTPUT_RESOLUTION;



typedef enum
{
    E_HDMI_COLOR_FORMAT_YUV = 0,
    E_HDMI_COLOR_FORMAT_RGB,
}EN_HDMI_COLOR_FORMAT;

typedef enum
{
    E_BUFF_ID_JPD_READ,
    E_BUFF_ID_JPD_INTER,
    E_BUFF_ID_JPD_OUTPUT,
    E_BUFF_ID_GOP_OUTPUT,
    E_BUFF_ID_VE_FRAME,
}EN_BUFF_ID;


typedef struct
{
    MS_U16 u16Width;
    MS_U16 u16Height;
    MS_U16 u16Pitch;
}ST_RectInfo;

typedef struct
{
    MS_U32 u32VirtualAddr;
    MS_U32 u32PhysicalAddr;
    MS_U32 u32Size;
}ST_BuffInfo;

typedef struct
{
    MS_U16 u16StartX;
    MS_U16 u16StartY;   
    MS_U16 u16Width;
    MS_U16 u16Height;
    MS_U16 u16Alpha;
    MS_U16 u16Red;
    MS_U16 u16Green;
    MS_U16 u16Blue;
    MS_U32 u32GeOutputAddr;
}ST_FillRectInfo;
typedef struct
{
    MS_U16 u16SrcWidth;
    MS_U16 u16SrcHeight;
    MS_U16 u16SrcPitch;
    MS_U16 u16DstStartX;
    MS_U16 u16DstStartY;
    MS_U16 u16StretchWidth;
    MS_U16 u16StretchHeight;
    MS_U32 u32JpdOutputAddr;
    MS_U32 u32GeOutputAddr;
}ST_BitBlitInfo;
MS_BOOL Script_MemMgr_SetBufInfo(EN_BUFF_ID eBufID, ST_BuffInfo* pstBuffInfo);
ST_BuffInfo* Script_MemMgr_GetBufInfo(EN_BUFF_ID eBufID);

#endif
