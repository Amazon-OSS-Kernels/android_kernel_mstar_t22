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
/// @brief MIU header file
///
////////////////////////////////////////////////////////////////////////////////
#ifndef _API_XC_CUS_H_
#define _API_XC_CUS_H_

#ifdef _API_XC_CUS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define ENABLE_HDTV                 ENABLE
#if (ENABLE_HDTV == 1)
    #define SUPPORT_EURO_HDTV       DISABLE
#else
    #define SUPPORT_EURO_HDTV       DISABLE
#endif

#define ENABLE_RGB_SUPPORT_85HZ     (0)
#define ENABLE_3D_PROCESS           0

//------------------------------
// Mode index
//------------------------------
typedef enum
{
    MD_640x350_70,  // 0
#if ENABLE_RGB_SUPPORT_85HZ
    MD_640x350_85,
#endif //#if ENABLE_RGB_SUPPORT_85HZ
    MD_640x350_60,  // 1
    MD_720x400_70,  // 2
#if ENABLE_RGB_SUPPORT_85HZ
    MD_640x400_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_640x400_70,    // 3
#if ENABLE_RGB_SUPPORT_85HZ
    MD_720x400_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_640x480_60,    // 4
    MD_640x480_66,    // 5
    MD_640x480_72,    // 6
    MD_640x480_75,    // 7
#if ENABLE_RGB_SUPPORT_85HZ
    MD_640x480_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_800x600_56,    // 8
    MD_800x600_60,    // 9
    MD_800x600_72,    //10
    MD_800x600_75,  // 11
#if ENABLE_RGB_SUPPORT_85HZ
    MD_800x600_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_832x624_75,  // 12
    MD_848x480_59,    // 13
#if ENABLE_RGB_SUPPORT_85HZ
    MD_1024x768_43I,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_1024x768_60, // 14
    MD_1024x768_70, // 15
    MD_1024x768_75, // 16
#if ENABLE_RGB_SUPPORT_85HZ
    MD_1024x768_85,
#endif //#if ENABLE_RGB_SUPPORT_85HZ
    MD_1152x864_60, // 17
    MD_1152x864_70, // 18
    MD_1152x864_75, // 19
#if ENABLE_RGB_SUPPORT_85HZ
    MD_1152x864_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_1152x870_75, // 20
    MD_1280x768_60, // 21
    MD_1280x768_75, // 22
    MD_1280x960_60, // 23
    MD_1280x960_75, // 24
#if ENABLE_RGB_SUPPORT_85HZ
    MD_1280x960_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_1280x1024_60, // 25
    MD_1280x1024_59, // 26
    MD_1280x1024_72, // 27
    MD_1280x1024_75, // 28
    MD_1280x1024_76, // 29
#if ENABLE_RGB_SUPPORT_85HZ
    MD_1280x1024_85,
#endif // #if ENABLE_RGB_SUPPORT_85HZ
    MD_1600x1200_59, // 30
    MD_1600x1200_60, // 31
    MD_1600x1200_65, // 32
    MD_1600x1200_70, // 33
    MD_1600x1200_75, // 34
    MD_1920x1200_59, // 35
    MD_1920x1200_60, // 36
    MD_1920x1200_65, // 37
    MD_1920x1200_75, // 38
    MD_1400x1050_59, // 39
    MD_1400x1050_60, // 40
    MD_1400x1050_75, // 41
    MD_1440x900_60_RB,  // 42
    MD_1440x900_75,  // 43
    MD_1680x1050_59, // 44
    MD_1680x1050_60, // 45
    MD_1680x1050_75, // 46

    /*++ Component Mode ++*/
    MD_720x480_60I,  // 47
    MD_720x480_60P,  // 48
    MD_720x576_50I,  // 49
    MD_720x576_50P,  // 50
    MD_1280x720_50P, // 51
    MD_1280x720_60P, // 52
    MD_1920x1080_50I,// 53
    MD_1920x1080_60I,// 54
#if (SUPPORT_EURO_HDTV)
    MD_1920x1080_50I_EURO,
#endif
    MD_1920x1080_24P, // 55
    MD_1920x1080_25P,//56
    MD_1920x1080_30P, // 57
    MD_1920x1080_50P, // 58
    MD_1920x1080_60P, // 59
    MD_720x480_60I_P, // 60 patch for DVD 480i -> Sherwood -> component output -> OSD unsupport mode
    MD_1280X1470_50P, // 61
    MD_1280X1470_60P, // 62
    MD_1920X2205_24P, // 63
    /*-- Component Mode --*/

    MD_1152x870_60,   // 64
    MD_1280x800_60,      // 65
    MD_1280x800_75,      // 66
    MD_1600x1024_60,  // 67
    MD_1600x1024_75,  // 68
    MD_1600x900_60,      // 69
    MD_1600x900_75,      // 70
    MD_1280x720_75,      // 71
    MD_640x480_57,    // 72
    MD_1360x768_60,      // 73
    MD_1600x1200_51,  // 74
    MD_848x480_60,    // 75
    MD_1280x768_50,      // 76
    MD_1366x768_60,   // 77
    MD_720X576_60,    // 78
    MD_864X648_60,    // 79
    MD_1920X1080_60,  // 80
    MD_720X480_60,    // 81

    MD_1440x900_60,  // 82
    MD_1280x768_60_RB,  // 83
    MD_1280x720_25P, // 84
    MD_1280x720_30P, // 85
    MD_STD_MODE_MAX_INDEX, // 86

    YPBPR_MD_START = MD_720x480_60I,
    YPBPR_MD_END = MD_1920x1080_60P,
    //PC_MD_END = YPBPR_MD_START - 1
}EN_MODE_TYPE;

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

INTERFACE MS_PCADC_MODETABLE_TYPE* MApi_XC_GetModeTable(void);
INTERFACE MS_U8 MApi_XC_GetTotalModeCount(void);




////////////////////////////////////////////////////////////////////////////////
#undef INTERFACE
#endif /* _API_XC_CUS_H_ */

