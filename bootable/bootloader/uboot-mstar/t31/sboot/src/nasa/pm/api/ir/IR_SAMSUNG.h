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

#ifndef IR_FORMAT_H
#define IR_FORMAT_H

//*************************************************************************
// Customer IR Specification parameter define (Please modify them by IR SPEC)
//*************************************************************************
#define IR_MODE_SEL                         IR_TYPE_SWDECODE_NEC_MODE // IR_TYPE_SWDECODE_NEC_MODE
#define IR_CHANNEL_USE_AS_UPDOWN            0
#define IR_VOLUME_USE_AS_LEFTRIGHT          0

// IR Header code define
#define IR_HEADER_CODE0                     0x80    // Custom 0     //0x04
#define IR_HEADER_CODE1                     0x7F    // Custom 1     //0xFB

#define IR_IS_USER_HEADER_CODE(x)           ((x) == 0x0007)  // (((x) >> 8) == 0xFB)   // user remoter
#define IR_IS_FACT_HEADER_CODE(x)           ((x) == 0x0007)  // (((x) >> 8) == 0xFC)  // factory remoter
#define IR_IS_THIRD_HEADER_CODE(x)          ((x) == 0x0F00)  // (((x) >> 8) == 0xFC)  // The Third remoter
#define IR_IS_AUTOIR_HEADER_CODE(x)          ((x) == 0x001F)  // (((x) >> 8) == 0xFC)  // The auto remoter

// IR Timing define
#define IR_HEADER_CODE_TIME                 9000    // us
#define IR_OFF_CODE_TIME                    4500    // us
#define IR_OFF_CODE_RP_TIME                 2500    // us
#define IR_LOGI_01H_TIME                    560     // us
#define IR_LOGI_0_TIME                      1120    // us
#define IR_LOGI_1_TIME                      2240    // us
#define IR_TIMEOUT_CYC                      140000  // us


#define IR_HEADER_CODE_TIME_UB              20
#define IR_HEADER_CODE_TIME_LB              -20
#define IR_OFF_CODE_TIME_UB                 20
#define IR_OFF_CODE_TIME_LB                 -20
#define IR_OFF_CODE_RP_TIME_UB              20
#define IR_OFF_CODE_RP_TIME_LB              -20
#define IR_LOGI_01H_TIME_UB                 35
#define IR_LOGI_01H_TIME_LB                 -30
#define IR_LOGI_0_TIME_UB                   20
#define IR_LOGI_0_TIME_LB                   -20
#define IR_LOGI_1_TIME_UB                   20
#define IR_LOGI_1_TIME_LB                   -20

// IR Format define
#define IRKEY_DUMY                          0xFF
#define IRDA_KEY_MAPPING_POWER              IRKEY_POWER

#define IR_LEADER_CODE_CHECKING_OPTION      0xBF
#define ENABLE_IR_MSTAR_SOURCE_HOTKEY   	0

typedef enum _IrCommandType
{
    IRKEY_POWER                         = 0x02,
    IRKEY_INPUT_SOURCE              = 0x01,
    IRKEY_SLEEP                 = 0x03,
    IRKEY_NUM_1                 = 0x04,
    IRKEY_NUM_2                 = 0x05,
    IRKEY_NUM_3                 = 0x06,
    IRKEY_NUM_4                 = 0x08,
    IRKEY_NUM_5                 = 0x09,
    IRKEY_NUM_6                 = 0x0A,
    IRKEY_NUM_7                 = 0x0C,
    IRKEY_NUM_8                 = 0x0D,
    IRKEY_NUM_9                 = 0x0E,
    IRKEY_NUM_0                 = 0x11,
    IRKEY_DASH                   = 0x23,
    IRKEY_CHANNEL_RETURN        = 0x13,
    IRKEY_VOLUME_PLUS             = 0x07,
    IRKEY_VOLUME_MINUS           = 0x0B,
    IRKEY_MUTE                          = 0x0F,
    IRKEY_CHANNEL_LIST            = 0x6B,
    IRKEY_CHANNEL_PLUS           = 0x12,
    IRKEY_CHANNEL_MINUS         = 0x10,
    IRKEY_MEDIA_PLAYER            = 0x8C, // new add
    IRKEY_MENU                          = 0x1A,
    IRKEY_HDMI                          = 0x8B,
    IRKEY_TOOL                          = 0x4B, // new add
    IRKEY_INFO                           = 0x1F,
    //IRKEY_RETURN                       = 0x52, // new add
    IRKEY_BACK                           = 0x58, // new add
    IRKEY_EXIT                            = 0x2D,
    IRKEY_UP                               = 0x60,
    IRKEY_DOWN                          = 0x61,
    IRKEY_LEFT                            = 0x65,
    IRKEY_RIGHT                          = 0x62,
    IRKEY_SELECT                         = 0x68,
    IRKEY_EMANUAL                       =0x3F,
    IRKEY_RED                              = 0x6C,
    IRKEY_GREEN                          = 0x14,
    IRKEY_YELLOW                        = 0x15,
    IRKEY_BLUE                             = 0x16,
    IRKEY_ZOOM                            = 0x3E,
    IRKEY_I_II                               = 0x00, // new add
    IRKEY_SRS                               = 0x6E, // new add
    IRKEY_STOP                              = 0x46,
    IRKEY_BACKWARD                      = 0x45,
    IRKEY_FORWARD                        = 0x48,
    IRKEY_PLAY                               = 0x47,
    IRKEY_PAUSE                             = 0x4A,
    IRKEY_FACTORY                           =0x3B, //new add

    IRKEY_CAPTURE                           =0xAA, //new add
    IRKEY_SOCCER                           =0xB8, //new add

    //ohya++ for ttx
    IRKEY_TTX                   = 0x2C, //
    //IRKEY_REVEAL                = 0x1F, //
    //IRKEY_HOLD                  = 0x58, //
    //IRKEY_SIZE                  = 0x1A, //
    //IRKEY_INDEX                 = 0x4B, //
    //IRKEY_KEY_CHANNEL_LIST      = 0x01, //
    //IRKEY_KEY_SUBCODE           = 0X13, //



    IRKEY_TV_ANTENNA            = IRKEY_DUMY-51, // no use
    IRKEY_CHANNEL_FAV_LIST      = IRKEY_DUMY-49, // no use
    IRKEY_AUDIO                 = IRKEY_DUMY-48, // no use
    IRKEY_FREEZE                = IRKEY_DUMY-47, // no use
    IRKEY_PAGE_DOWN             = IRKEY_DUMY-45, // no use
    IRKEY_CLOCK                 = IRKEY_DUMY-44, // no use
    IRKEY_MTS                   = IRKEY_DUMY-43, // no use
    IRKEY_NINE_LATTICE          = IRKEY_DUMY-42, // no use
    IRKEY_CC                    = IRKEY_DUMY-40, // no use
    IRKEY_CRADRD                = IRKEY_DUMY-39, // no use
    IRKEY_PICTURE               = IRKEY_DUMY-38, // no use
    IRKEY_RECORD                = IRKEY_DUMY-37, // no use
    IRKEY_MEMORY_RECALL         = IRKEY_DUMY-36, // no use
    IRKEY_AV                    = IRKEY_DUMY-35, // no use
    IRKEY_SCART                 = IRKEY_DUMY-34, // no use
    IRKEY_PC                    = IRKEY_DUMY-33, // no use
    IRKEY_DTV                   = IRKEY_DUMY-32, // no use
    IRKEY_TV                    = IRKEY_DUMY-31, // no use
    IRKEY_COMPONENT             = IRKEY_DUMY-30, // no use
    IRKEY_SV                    = IRKEY_DUMY-29, // no use
    //IRKEY_SLEEP                 = IRKEY_DUMY-28, // no use
    IRKEY_EPG                   = IRKEY_DUMY-27, // no use
    IRKEY_P_CHECK               = IRKEY_DUMY-26, // no use
    IRKEY_S_CHECK               = IRKEY_DUMY-25, // no use
    IRKEY_MIX                   = IRKEY_DUMY-24, // no use
    IRKEY_INDEX                 = IRKEY_DUMY-23, // no use
    IRKEY_HOLD                  = IRKEY_DUMY-22, // no use
    IRKEY_PREVIOUS              = IRKEY_DUMY-21, // no use
    IRKEY_NEXT                  = IRKEY_DUMY-20, // no use
    IRKEY_POWERONLY             = IRKEY_DUMY-19, // no use
    IRKEY_TTX_MODE              = IRKEY_DUMY-18, // no use
    IRKEY_RED2                  = IRKEY_DUMY-17, // no use
    IRKEY_GREEN2                = IRKEY_DUMY-16, // no use
    IRKEY_UPDATE                = IRKEY_DUMY-15, // no use
    IRKEY_SUBTITLE              = IRKEY_DUMY-14, // no use
    IRKEY_TIME                  = IRKEY_DUMY-13, // no use
    IRKEY_SIZE                  = IRKEY_DUMY-12, // no use
    IRKEY_REVEAL                = IRKEY_DUMY-11, // no use
    IRKEY_ADJUST                = IRKEY_DUMY-9, // no use
    IRKEY_TV_INPUT              = IRKEY_DUMY-8, // no use
    IRKEY_KEY_DISABLE_KEYPAD    = IRKEY_DUMY-7, // no use
    //IRKEY_BACK                  = IRKEY_DUMY-6, // no use
    IRKEY_SUBPAGE               = IRKEY_DUMY-5, // no use
    IRKEY_SUBCODE               = IRKEY_DUMY-4, // no use
    IRKEY_PHOTO                 = IRKEY_DUMY-3, // no use
    IRKEY_MINU                  = IRKEY_DUMY-2, // no use
    IRKEY_GUIDE                 = IRKEY_DUMY-1, // no use
    IRKEY_HOME                  = IRKEY_DUMY, // no use

}IrCommandType;
//*************************************************************************

//*************************************************************************
// IR system parameter define for H/W setting (Please don't modify them)
//*************************************************************************
#define IR_CKDIV_NUM             ((BIU_CLOCK_BOOT + 500000UL) / 1000000UL)
#define IR_CKDIV_NUM_BOOT        13

#define IR_CLK_BOOT             (BIU_CLOCK_BOOT / 1000000.0)
#define IR_CLK                  (BIU_CLOCK / 1000000.0)
#define irGetMinCnt_BOOT(time, tolerance) (((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1-tolerance))
#define irGetMaxCnt_BOOT(time, tolerance) (((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1+tolerance))
#define irGetMinCnt(time, tolerance) (((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1-tolerance))
#define irGetMaxCnt(time, tolerance) (((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1+tolerance))

#define irGetCnt_BOOT(time) (((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))+0x300000UL)
#define irGetCnt(time) (((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))+0x300000UL)

// 12Mhz
#define IR_RP_TIMEOUT_BOOT      irGetCnt_BOOT(IR_TIMEOUT_CYC)
#define IR_HDC_UPB_BOOT         irGetMaxCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB_BOOT         irGetMinCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB_BOOT         irGetMaxCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB_BOOT         irGetMinCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB_BOOT      irGetMaxCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB_BOOT      irGetMinCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB_BOOT       irGetMaxCnt_BOOT(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB_BOOT       irGetMinCnt_BOOT(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_1_TIME, 0.2)

// 90Mhz
#define IR_RP_TIMEOUT           irGetCnt(IR_TIMEOUT_CYC)
#define IR_HDC_UPB              irGetMaxCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB              irGetMinCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB              irGetMaxCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB              irGetMinCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB           irGetMaxCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB           irGetMinCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB            irGetMaxCnt(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB            irGetMinCnt(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB              irGetMaxCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB              irGetMinCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB              irGetMaxCnt(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB              irGetMinCnt(IR_LOGI_1_TIME, 0.2)

// Power off mode
#define PM_IR_TH_VAL            (PM_TH0_VAL & 0x0F)
// PM_IR_TH_GRID specify the time (in us) for each threshold bit.
// So PM_IR_TH_GRID = (1/12M) * (2^14) * 1000000 = (2^14) / 12
#define PM_IR_TH_GRID           (16384/IR_CLK_BOOT)
#define PM_IR_HEADER_CODE_TIME  (IR_HEADER_CODE_TIME-(0x0F-PM_IR_TH_VAL)*PM_IR_TH_GRID)
#define PM_IR_HDC_UPB_BOOT      irGetMaxCnt_BOOT(PM_IR_HEADER_CODE_TIME, 0.6)
#define PM_IR_HDC_LOB_BOOT      irGetMinCnt_BOOT(PM_IR_HEADER_CODE_TIME, 0.6)
//*************************************************************************
#define IR_NEC_HEADER_CNT_LB        7000
#define IR_NEC_HEADER_CNT_UB        9000
#define IR_NEC_REPEATE_CNT_LB       40000
#define IR_NEC_REPEATE_CNT_UB       50000
#define IR_NEC_LOGIC0_CNT_LB        700
#define IR_NEC_LOGIC0_CNT_UB        1500
#define IR_NEC_LOGIC1_CNT_LB        1700
#define IR_NEC_LOGIC1_CNT_UB        2500
#define IR_NEC_REPEATE_TIMEOUT_CNT  150

//********************************
//
//  IR Wakeup keys for standby mode
//
//********************************
#define IR_WAKEUP_KEY0              IRKEY_POWER //configured by application
#define IR_WAKEUP_KEY1              IRKEY_POWER //configured by application
#define IR_WAKEUP_KEY2              IRKEY_POWER //configured by application
#define IR_WAKEUP_KEY3              IRKEY_POWER //configured by application

//********************************
//
//  Support 2nd NEC Full Decode
//
//********************************
// 2nd IR Header code define
#define IR_2ND_NEC_DECODE_EN        DISABLE
#define IR_HEADER_CODE_2ND0         0xFF    // 2nd Custom 0
#define IR_HEADER_CODE_2ND1         0xFF    // 2nd Custom 1

#endif


