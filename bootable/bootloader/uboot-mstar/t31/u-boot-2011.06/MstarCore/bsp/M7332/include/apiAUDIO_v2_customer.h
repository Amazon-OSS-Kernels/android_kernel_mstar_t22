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

#ifndef _API_AUDIO_CUSTOMER_H_
#define _API_AUDIO_CUSTOMER_H_

#include "MsCommon.h"


#ifdef __cplusplus
extern "C"
{
#endif

#if 1 //New APIs
#if defined (__aarch64__)
typedef MS_U64 CAUD_PHY;
typedef MS_U64 CAUD_VIRT;
#else
typedef MS_U32 CAUD_PHY;
typedef MS_U32 CAUD_VIRT;
#endif //#if defined (__aarch64__)

//-------------------------------------------------------------------------------------------------------------------------------------
/* Initialize, STR */
//-------------------------------------------------------------------------------------------------------------------------------------
//need to be consistent with AUDIO_OUTPUT_TYPE of drvAudio_if.h
typedef enum {
    CAUD_OUTPUT_TYPE_INVALID = -1,
    CAUD_OUTPUT_LINE0,                 ///< 0: DAC0 OUTPUT
    CAUD_OUTPUT_LINE1,                 ///< 1: T2=>AA0 OUTPUT    T3=>HP OUTPUT
    CAUD_OUTPUT_LINE2,                 ///< 2: T2=>AA1 OUTPUT    T3=>DAC2 OUTPUT
    CAUD_OUTPUT_LINE3,                 ///< 3: T2=>NULL          T3=>AA OUTPUT
    CAUD_OUTPUT_SPDIF,                 ///< 4: SPDIF OUTPUT
    CAUD_OUTPUT_I2S,                   ///< 5: I2S OUTPUT
    CAUD_OUTPUT_HP,                    ///< 6: OUTPUT with headphone driver
    CAUD_OUTPUT_I2S2,                  ///< 7: I2S2 OUTPUT
    CAUD_OUTPUT_HDMI_ARC,              ///< 8: HDMI ARC OUTPUT(HDMI_Rx,TV)
    CAUD_OUTPUT_HDMI,                  ///< 9: HDMI Tx OUTPUT(HDMI_Tx,STB)
    CAUD_OUTPUT_NULL = 0xFF,           ///< 0xFF: Null output
    CAUD_OUTPUT_TYPE_MAX,
} CAUD_OUTPUT_TYPE;

//need to be consistent with AUDIO_OUT_INFO of drvAudio_if.h
typedef struct {
    CAUD_OUTPUT_TYPE speakerOut;  ///< Audio output port for Speaker
    CAUD_OUTPUT_TYPE hpOut;       ///< Audio output port for HP
    CAUD_OUTPUT_TYPE monitorOut;  ///< Audio output port for Monitor out
    CAUD_OUTPUT_TYPE scartOut;    ///< Audio output port for Scart out
    CAUD_OUTPUT_TYPE spdifOut;    ///< Audio output port for S/PDIF out
    CAUD_OUTPUT_TYPE arcOut;      ///< Audio output port for ARC(HDMI_Rx) out
    CAUD_OUTPUT_TYPE hdmiOut;     ///< Audio output port for HDMI_Tx out
} caudOutputInfo_t;

typedef enum {
    CAUD_PLATFORM_INVALID = -1,
    CAUD_PLATFORM_TV,
    CAUD_PLATFORM_STB,
} CAUD_CHIP_PLATFORM;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Connect & Disconnect */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_PARSER_MODE_INVALID = -1,
    CAUD_PARSER_MODE0,
    CAUD_PARSER_MODE1,
    CAUD_PARSER_MODE2,
    CAUD_PARSER_MODE3,
    CAUD_PARSER_MODE_MAX,
} CAUD_PARSER_MODE;

typedef enum {
    CAUD_PARSER_INPUT_INVALID = -1,
    CAUD_PARSER_INPUT_AFIFO0,
    CAUD_PARSER_INPUT_AFIFO1,
    CAUD_PARSER_INPUT_AFIFO2,
    CAUD_PARSER_INPUT_AFIFO3,
    CAUD_PARSER_INPUT_MAX,
} CAUD_PARSER_INPUT;

typedef enum {
    CAUD_ADEC_IDX_INVALID = -1,
    CAUD_ADEC0          = 0,
    CAUD_ADEC1          = 1,
    CAUD_ADEC2          = 3,
    CAUD_ADEC3          = 4,
    CAUD_ASND           = 5, //<--MAX is here!!!
    CAUD_ADEC_ATV       = 2,
    CAUD_ADEC_IDX_MAX   = CAUD_ASND + 1,
} CAUD_ADEC_IDX;

typedef enum {
    CAUD_ADEC_INPUT_INVALID   = -1,
    CAUD_ADEC_INPUT_DTV       = 0,
    CAUD_ADEC_INPUT_AD        = 9, //<--MAX is here!!!
    CAUD_ADEC_INPUT_HDMI      = 1,
    CAUD_ADEC_INPUT_SPDIF     = 7,
    CAUD_ADEC_INPUT_MM        = 2,
    CAUD_ADEC_INPUT_MM_UNI    = 6,
    CAUD_ADEC_INPUT_MM_TS     = 8,
    CAUD_ADEC_INPUT_CLIP      = 3,
    CAUD_ADEC_INPUT_ATV       = 4,
    CAUD_ADEC_INPUT_NONE      = 5,
    CAUD_ADEC_INPUT_MAX       = CAUD_ADEC_INPUT_AD + 1,
} CAUD_ADEC_INPUT;

typedef enum {
    CAUD_ADEC_PCM_PATH_INVALID = -1,
    CAUD_ADEC_PCM_PATH_DSP_SRC,
    CAUD_ADEC_PCM_PATH_MCU,
    CAUD_ADEC_PCM_PATH_MAX,
} CAUD_ADEC_PCM_PATH;

typedef enum {
    CAUD_ADC_IDX_INVALID = -1,
    CAUD_ADC0,
    CAUD_ADC1,
    CAUD_ADC_IDX_MAX,
} CAUD_ADC_IDX;

typedef enum {
    CAUD_ADC_IN_PORT_INVALID  = -1,
    CAUD_ADC0_IN_PORT0       = 0x02,
    CAUD_ADC0_IN_PORT1       = 0x12,
    CAUD_ADC0_IN_PORT2       = 0x22,
    CAUD_ADC0_IN_PORT3       = 0x32,
    CAUD_ADC0_IN_PORT4       = 0xA2,
    CAUD_ADC0_IN_PORT5       = 0xB2,
    CAUD_ADC0_MIC_IN         = 0x72,
    CAUD_ADC1_IN_PORT0       = 0x09,
    CAUD_ADC1_IN_PORT1       = 0x19,
    CAUD_ADC1_IN_PORT2       = 0x29,
    CAUD_ADC1_IN_PORT3       = 0x39,
    CAUD_ADC1_IN_PORT4       = 0xA9,
    CAUD_ADC1_IN_PORT5       = 0xB9,
    CAUD_ADC1_MIC_IN         = 0x79,
} CAUD_ADC_IN_PORT;

typedef enum {
    CAUD_PCM_MIXER_IDX_INVALID = -1,
    CAUD_PCM_MIXER0,
    CAUD_PCM_MIXER_IDX_MAX,
} CAUD_PCM_MIXER_IDX;

typedef enum {
    CAUD_PCM_MIXER_INPUT_INVALID = -1,
    CAUD_PCM_MIXER_INPUT_AMIX0,
    CAUD_PCM_MIXER_INPUT_AMIX1,
    CAUD_PCM_MIXER_INPUT_AMIX2,
    CAUD_PCM_MIXER_INPUT_AMIX3,
    CAUD_PCM_MIXER_INPUT_AMIX4,
    CAUD_PCM_MIXER_INPUT_AMIX5,
    CAUD_PCM_MIXER_INPUT_AMIX6,
    CAUD_PCM_MIXER_INPUT_AMIX7,
    CAUD_PCM_MIXER_INPUT_MAX,
} CAUD_PCM_MIXER_INPUT;

typedef enum {
    CAUD_CH_IDX_INVALID = -1,
    CAUD_CH1        = 4,
    CAUD_CH2        = 5,
    CAUD_CH3        = 6,
    CAUD_CH4        = 7, //<--MAX is here!!!
    CAUD_CH5        = 0,
    CAUD_CH6        = 1,
    CAUD_CH7        = 2,
    CAUD_CH8        = 3,
    CAUD_CH_IDX_MAX = CAUD_CH4 + 1,
} CAUD_CH_IDX;

typedef enum {
    CAUD_CH_INPUT_INVALID     = -1,
    CAUD_CH_INPUT_ADEC0       = 0,
    CAUD_CH_INPUT_ADEC1       = 1,
    CAUD_CH_INPUT_ADEC2       = 12,
    CAUD_CH_INPUT_ADEC3       = 13, //<--MAX is here!!!
    CAUD_CH_INPUT_ADEC_ATV    = 2,
    CAUD_CH_INPUT_HDMI        = 3,
    CAUD_CH_INPUT_ADC0        = 4,
    CAUD_CH_INPUT_ADC1        = 5,
    CAUD_CH_INPUT_SPDIF       = 6,
    CAUD_CH_INPUT_R2DMA1      = 7,
    CAUD_CH_INPUT_R2DMA2      = 8,
    CAUD_CH_INPUT_SWDMA1      = 9,
    CAUD_CH_INPUT_HWDMA1      = 10,
    CAUD_CH_INPUT_NONE        = 11,
    CAUD_CH_INPUT_MAX         = CAUD_CH_INPUT_ADEC3 + 1,
} CAUD_CH_INPUT;

typedef  enum {
    CAUD_FWM_IDX_INVALID = -1,
    CAUD_FWM0,
    CAUD_FWM1,
    CAUD_FWM2,
    CAUD_FWM_IDX_MAX,
} CAUD_FWM_IDX;

typedef  enum {
    CAUD_FWM_INPUT_INVALID = -1,
    CAUD_FWM_INPUT_CH1     = 4,
    CAUD_FWM_INPUT_CH2     = 5,
    CAUD_FWM_INPUT_CH3     = 6,
    CAUD_FWM_INPUT_CH4     = 7, //<--MAX is here!!!
    CAUD_FWM_INPUT_CH5     = 0,
    CAUD_FWM_INPUT_CH6     = 1,
    CAUD_FWM_INPUT_CH7     = 2,
    CAUD_FWM_INPUT_CH8     = 3,
    CAUD_FWM_INPUT_MAX     = CAUD_FWM_INPUT_CH4 + 1,
} CAUD_FWM_INPUT;

typedef  enum {
    CAUD_SE_IDX_INVALID = -1,
    CAUD_SE_PRER2,
    CAUD_SE_DSPSE,
    CAUD_SE_POSTR2,
    CAUD_SE_IDX_MAX,
} CAUD_SE_IDX;

typedef  enum {
    CAUD_SE_INPUT_INVALID = -1,
    CAUD_SE_INPUT_DELAY0,
    CAUD_SE_INPUT_DELAY1,
    CAUD_SE_INPUT_MAX,
} CAUD_SE_INPUT;

typedef enum {
    CAUD_SNDOUT_IDX_INVALID = -1,
    CAUD_SNDOUT_I2S         = 0, //SD0
    CAUD_SNDOUT_I2S2        = 8, //SD1 //<--MAX is here!!!
    CAUD_SNDOUT_LINEOUT0    = 1,
    CAUD_SNDOUT_LINEOUT1    = 2,
    CAUD_SNDOUT_LINEOUT2    = 3,
    CAUD_SNDOUT_LINEOUT3    = 4,
    CAUD_SNDOUT_SPDIF       = 5,
    CAUD_SNDOUT_HDMI_ARC    = 6,
    CAUD_SNDOUT_HDMI_TX     = 7,
    CAUD_SNDOUT_IDX_MAX     = CAUD_SNDOUT_I2S2 + 1,
} CAUD_SNDOUT_IDX;

typedef enum {
    CAUD_SNDOUT_INPUT_INVALID        = -1,
    CAUD_SNDOUT_INPUT_NULL           = 0,
    CAUD_SNDOUT_INPUT_CH7            = 1,
    CAUD_SNDOUT_INPUT_MCH_SWDMA_LR   = 12,
    CAUD_SNDOUT_INPUT_MCH_SWDMA_LsRs = 13, //<--MAX is here!!!
    CAUD_SNDOUT_INPUT_FWM0           = 2,
    CAUD_SNDOUT_INPUT_DLY0           = 3,
    CAUD_SNDOUT_INPUT_FWM1           = 4,
    CAUD_SNDOUT_INPUT_DLY1           = 5,
    CAUD_SNDOUT_INPUT_FWM2           = 6,
    CAUD_SNDOUT_INPUT_DLY2           = 7,
    CAUD_SNDOUT_INPUT_DELAY         = 11,
    CAUD_SNDOUT_INPUT_PRE_R2         = 8,
    CAUD_SNDOUT_INPUT_DSPSE          = 9,
    CAUD_SNDOUT_INPUT_POST_R2        = 10,
    CAUD_SNDOUT_INPUT_MAX            = CAUD_SNDOUT_INPUT_MCH_SWDMA_LsRs + 1,
} CAUD_SNDOUT_INPUT;

typedef enum {
    CAUD_PCM_CAPTURE_IDX_INVALID = -1,
    CAUD_PCM_CAPTURE0,
    CAUD_PCM_CAPTURE1,
    CAUD_PCM_CAPTURE2,
    CAUD_PCM_CAPTURE_IDX_MAX,
} CAUD_PCM_CAPTURE_IDX;

typedef enum {
    CAUD_PCM_CAPTURE_INPUT_INVALID     = -1,
    CAUD_PCM_CAPTURE_INPUT_CH1         = 11,
    CAUD_PCM_CAPTURE_INPUT_CH2         = 12,
    CAUD_PCM_CAPTURE_INPUT_CH3         = 13,
    CAUD_PCM_CAPTURE_INPUT_CH4         = 14,
    CAUD_PCM_CAPTURE_INPUT_CH5         = 0,
    CAUD_PCM_CAPTURE_INPUT_CH6         = 1,
    CAUD_PCM_CAPTURE_INPUT_CH7         = 2,
    CAUD_PCM_CAPTURE_INPUT_CH8         = 3,
    CAUD_PCM_CAPTURE_INPUT_PCM         = 4,
    CAUD_PCM_CAPTURE_INPUT_PCM_DELAY   = 5,
    CAUD_PCM_CAPTURE_INPUT_PCM_SE      = 6,
    CAUD_PCM_CAPTURE_INPUT_MIXER       = 7,
    CAUD_PCM_CAPTURE_INPUT_FWM0        = 8,
    CAUD_PCM_CAPTURE_INPUT_FWM1        = 9,
    CAUD_PCM_CAPTURE_INPUT_FWM2        = 10,
    CAUD_PCM_CAPTURE_INPUT_I2S_RX0     = 15,
    CAUD_PCM_CAPTURE_INPUT_I2S_RX1     = 16,
    CAUD_PCM_CAPTURE_INPUT_I2S_RX0_RX1 = 17,
    CAUD_PCM_CAPTURE_INPUT_SPEAKER     = 18,
    CAUD_PCM_CAPTURE_INPUT_ADV_SE      = 19,
    CAUD_PCM_CAPTURE_INPUT_PCMR_6CH    = 20, //<--MAX is here!!!
    CAUD_PCM_CAPTURE_INPUT_MAX         = CAUD_PCM_CAPTURE_INPUT_PCMR_6CH + 1,
} CAUD_PCM_CAPTURE_INPUT;

typedef enum {
    CAUD_MP3_ENC_IDX_INVALID = -1,
    CAUD_MP3_ENC0,
    CAUD_MP3_ENC_IDX_MAX,
} CAUD_MP3_ENC_IDX;

typedef enum {
    CAUD_MP3_ENC_INPUT_INVALID = -1,
    CAUD_MP3_ENC_INPUT_CH5,
    CAUD_MP3_ENC_INPUT_CH6,
    CAUD_MP3_ENC_INPUT_MAX,
} CAUD_MP3_ENC_INPUT;

typedef enum {
    CAUD_AAC_ENC_IDX_INVALID = -1,
    CAUD_AAC_ENC0,
    CAUD_AAC_ENC_IDX_MAX,
} CAUD_AAC_ENC_IDX;

typedef enum {
    CAUD_AAC_ENC_INPUT_INVALID = -1,
    CAUD_AAC_ENC_INPUT_PCM_CAPTURE0,
    CAUD_AAC_ENC_INPUT_PCM_CAPTURE1,
    CAUD_AAC_ENC_INPUT_MAX,
} CAUD_AAC_ENC_INPUT;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Start & Stop */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_CODEC_TYPE_INVALID      = -1,
    CAUD_CODEC_UNKNOWN           = 0,
    CAUD_CODEC_PCM               = 1, // for HDMI PCM
    CAUD_CODEC_AC3               = 2,
    CAUD_CODEC_EAC3              = 3,
    CAUD_CODEC_EAC3_ATMOS        = 36,
    CAUD_CODEC_MPEG              = 4,
    CAUD_CODEC_AAC               = 5,
    CAUD_CODEC_HEAAC             = 6,
    CAUD_CODEC_DRA               = 7,
    CAUD_CODEC_MP3               = 8,
    CAUD_CODEC_DTS               = 9,
    CAUD_CODEC_SIF               = 10,
    CAUD_CODEC_SIF_BTSC          = 11,
    CAUD_CODEC_SIF_A2            = 12,
    CAUD_CODEC_DEFAULT           = 13,
    CAUD_CODEC_NONE              = 14,
    CAUD_CODEC_DTS_HD_MA         = 15,
    CAUD_CODEC_DTS_EXPRESS       = 16,
    CAUD_CODEC_DTS_CD            = 17,
    CAUD_CODEC_WMA               = 18,
    CAUD_CODEC_WMA_PRO           = 19,
    CAUD_CODEC_XPCM              = 20,
    CAUD_CODEC_RA8LBR            = 21,
    CAUD_CODEC_FLAC              = 22,
    CAUD_CODEC_VORBIS            = 23,
    CAUD_CODEC_AMR_NB            = 24,
    CAUD_CODEC_AMR_WB            = 25,
    CAUD_CODEC_DolbyTrueHDBypass = 26,
    CAUD_CODEC_DVI               = 27, // for HDMI DVI
    CAUD_CODEC_ESBypass          = 28,
    CAUD_CODEC_OPUS              = 29,
    CAUD_CODEC_MPEG_H            = 30,
    CAUD_CODEC_AC4               = 31,
    CAUD_CODEC_MAT               = 32,
    CAUD_CODEC_MAT_ATMOS         = 37,
    CAUD_CODEC_TRUEHD            = 38,
    CAUD_CODEC_TRUEHD_ATMOS      = 39, //<--MAX is here!!!
    CAUD_CODEC_ES_SKIP           = 33,
    CAUD_CODEC_DTSX_P1           = 34,
    CAUD_CODEC_DTSX_P2           = 35,
} CAUD_CODEC_TYPE;

typedef enum {
    CAUD_UPPER_PLAYCMD_INVALID = -1,
    CAUD_UPPER_PLAY            = 0,
    CAUD_UPPER_STOP            = 1,
    CAUD_UPPER_PAUSE           = 2,
} CAUD_UPPER_PLAYCMD;

//-------------------------------------------------------------------------------------------------------------------------------------
/* SPDIF */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_SPDIF_TX_MODE_INVALID   = -1,
    CAUD_SPDIF_TX_NONE           = 0,
    CAUD_SPDIF_TX_PCM            = 1,
    CAUD_SPDIF_TX_AUTO           = 2,
    CAUD_SPDIF_TX_BYPASS         = 3,
    CAUD_SPDIF_TX_TRANSCODE      = 4,        /* Netflix */
    CAUD_SPDIF_TX_DD             = 5,
} CAUD_SPDIF_TX_TYPE;

typedef enum {
    CAUD_SPDIF_COPYRIGHT_INVALID  = -1,
    CAUD_SPDIF_COPY_FREE          = 0,    /* cp-bit : 1, L-bit : 0 */
    CAUD_SPDIF_COPY_NO_MORE       = 1,    /* cp-bit : 0, L-bit : 1 */
    CAUD_SPDIF_COPY_ONCE          = 2,    /* cp-bit : 0, L-bit : 0 */
    CAUD_SPDIF_COPY_NEVER         = 3,    /* cp-bit : 0, L-bit : 1 */
} CAUD_SPDIF_COPYRIGHT;

//====== SPDIF OUTPUT CS TYPE =========
typedef enum {
    CAUD_SPDIF_CS_CategoryCode               = 0x0,      ///<
    CAUD_SPDIF_CS_CopyRight                  = 0x1,      ///<
    CAUD_SPDIF_CS_SrcNum                     = 0x2,      ///<
    CAUD_SPDIF_CS_ChNum                      = 0x3,      ///<
    CAUD_SPDIF_CS_SmpFreq                    = 0x4,      ///<
    CAUD_SPDIF_CS_ClockAcc                   = 0x5,      ///<
    CAUD_SPDIF_CS_ChAB                       = 0x6,      ///<
    CAUD_SPDIF_CS_CopyProtect                = 0x7,      ///<
} CAUD_SPDIF_CS_TYPE;

//====== SPDIF OUTPUT CS Status =======
typedef enum {
    CAUD_SPDIF_CS_Category_BroadCast         = 0x00,     ///<
    CAUD_SPDIF_CS_Category_General           = 0x01,     ///<
    CAUD_SPDIF_CS_Category_BroadCast_JP      = 0x02,     ///<
    CAUD_SPDIF_CS_Category_BroadCast_EU      = 0x03,     ///<
    CAUD_SPDIF_CS_Category_BroadCast_USA     = 0x04,     ///<
    CAUD_SPDIF_CS_Category_Rev4              = 0x05,     ///<
    CAUD_SPDIF_CS_Category_Rev5              = 0x06,     ///<
    CAUD_SPDIF_CS_Category_Rev6              = 0x07,     ///<
    CAUD_SPDIF_CS_CopyRight_CP               = 0x08,     ///<
    CAUD_SPDIF_CS_CopyRight_NonCP            = 0x09,     ///<
    CAUD_SPDIF_CS_SrcNum_2                   = 0x0A,     ///<
    CAUD_SPDIF_CS_SrcNum_5                   = 0x0B,     ///<
    CAUD_SPDIF_CS_SrcNum_6                   = 0x0C,     ///<
    CAUD_SPDIF_CS_SrcNum_Rev1                = 0x0D,     ///<
    CAUD_SPDIF_CS_SrcNum_Rev2                = 0x0E,     ///<
    CAUD_SPDIF_CS_SrcNum_Rev3                = 0x0F,     ///<
    CAUD_SPDIF_CS_ChNum_A_Left               = 0x10,     ///<
    CAUD_SPDIF_CS_ChNum_B_Right              = 0x11,     ///<
    CAUD_SPDIF_CS_ChNum_C                    = 0x12,     ///<
    CAUD_SPDIF_CS_ChNum_D                    = 0x13,     ///<
    CAUD_SPDIF_CS_ChNum_E                    = 0x14,     ///<
    CAUD_SPDIF_CS_ChNum_F                    = 0x15,     ///<
    CAUD_SPDIF_CS_ChNum_G                    = 0x16,     ///<
    CAUD_SPDIF_CS_ChNum_H                    = 0x17,     ///<
    CAUD_SPDIF_CS_ChNum_I                    = 0x18,     ///<
    CAUD_SPDIF_CS_ChNum_J                    = 0x19,     ///<
    CAUD_SPDIF_CS_ChNum_K                    = 0x1A,     ///<
    CAUD_SPDIF_CS_SmpFreq_192                = 0x1B,     ///<
    CAUD_SPDIF_CS_SmpFreq_176                = 0x1C,     ///<
    CAUD_SPDIF_CS_SmpFreq_128                = 0x1D,     ///<
    CAUD_SPDIF_CS_SmpFreq_96                 = 0x1E,     ///<
    CAUD_SPDIF_CS_SmpFreq_88                 = 0x1F,     ///<
    CAUD_SPDIF_CS_SmpFreq_64                 = 0x20,     ///<
    CAUD_SPDIF_CS_SmpFreq_48                 = 0x21,     ///<
    CAUD_SPDIF_CS_SmpFreq_44                 = 0x22,     ///<
    CAUD_SPDIF_CS_SmpFreq_32                 = 0x23,     ///<
    CAUD_SPDIF_CS_SmpFreq_24                 = 0x24,     ///<
    CAUD_SPDIF_CS_SmpFreq_22                 = 0x25,     ///<
    CAUD_SPDIF_CS_SmpFreq_16                 = 0x26,     ///<
    CAUD_SPDIF_CS_SmpFreq_Rev1               = 0x27,     ///<
    CAUD_SPDIF_CS_SmpFreq_Rev2               = 0x28,     ///<
    CAUD_SPDIF_CS_SmpFreq_Rev3               = 0x29,     ///<
    CAUD_SPDIF_CS_SmpFreq_Rev4               = 0x2A,     ///<
    CAUD_SPDIF_CS_SmpFreq_Rev5               = 0x2B,     ///<
    CAUD_SPDIF_CS_ClockAcc_Lev1              = 0x2C,     ///<
    CAUD_SPDIF_CS_ClockAcc_Lev2              = 0x2D,     ///<
    CAUD_SPDIF_CS_ClockAcc_Lev3              = 0x2E,     ///<
    CAUD_SPDIF_CS_ClockAcc_NotMatch          = 0x2F,     ///<
    CAUD_SPDIF_ChAB_Invalid_ON               = 0x30,     ///<
    CAUD_SPDIF_ChAB_Invalid_OFF              = 0x31,     ///<
    CAUD_SPDIF_CS_CopyProtect_CopyFree       = 0x32,     ///<
    CAUD_SPDIF_CS_CopyProtect_CopyOnce       = 0x33,     ///<
    CAUD_SPDIF_CS_CopyProtect_CopyNever      = 0x34,     ///<
} CAUD_SPDIF_CS_TYPE_STATUS;

//-------------------------------------------------------------------------------------------------------------------------------------
/* HDMI */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_HDMI_TX_MODE_INVALID = -1,
    CAUD_HDMI_TX_PCM,                       ///< HDMI OUTPUT PCM
    CAUD_HDMI_TX_NONPCM,                    ///< HDMI OUTPUT non-PCM (DD)
    CAUD_HDMI_TX_EAC3_BYPASS,               ///< HDMI OUTPUT DD/DDP Bypass
    CAUD_HDMI_TX_EAC3,                      ///< HDMI OUTPUT DDP Encode (MS12)
} CAUD_HDMI_TX_TYPE;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Digital out */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_DIGITALOUT_TYPE_INVALID = -1,
    CAUD_DIGITALOUT_SPDIF    = 0,
    CAUD_DIGITALOUT_HDMI_ARC = 1,
    CAUD_DIGITALOUT_HDMI_TX  = 2,
    CAUD_DIGITALOUT_I2S      = 3, //<--MAX is here!!!
    CAUD_DIGITALOUT_TYPE_MAX = CAUD_DIGITALOUT_I2S + 1,
} CAUD_DIGITAL_OUTPUT_TYPE;

//need to be consistent with AUDIO_FORMAT_CODE of drvAudio_if.h
typedef enum {
    CAUD_DIGITALOUT_CODEC_INVALID = -1,
    CAUD_DIGITALOUT_NONE        = 0x0,
    CAUD_DIGITALOUT_LPCM        = 0x1,      ///< Support LPCM
    CAUD_DIGITALOUT_DD          = 0x2,      ///< Support DD
    CAUD_DIGITALOUT_MPEG1       = 0x3,      ///< Support MPEG1
    CAUD_DIGITALOUT_MP3         = 0x4,      ///< Support MP3
    CAUD_DIGITALOUT_MPEG2       = 0x5,      ///< Support MPEG2
    CAUD_DIGITALOUT_AAC         = 0x6,      ///< Support AAC
    CAUD_DIGITALOUT_DTS         = 0x7,      ///< Support DTS
    CAUD_DIGITALOUT_ATRAC       = 0x8,      ///< Support ATRAC
    CAUD_DIGITALOUT_ONEBITAUDIO = 0x9,      ///< Support One-Bit Audio
    CAUD_DIGITALOUT_DDP         = 0xA,      ///< Support DDP
    CAUD_DIGITALOUT_DTSHD       = 0xB,      ///< Support DTSHD
    CAUD_DIGITALOUT_TRUEHD      = 0xC,      ///< Support MLP/TRUE-HD
    CAUD_DIGITALOUT_DST         = 0xD,      ///< Support DST
    CAUD_DIGITALOUT_WMAPRO      = 0xE,      ///< Support WMA-Pro
    CAUD_DIGITALOUT_CODEC_MAX   = 0xF,
} CAUD_DIGITALOUT_CODEC;

//need to be consistent with Digital_Out_Device_Capability_t of drvAudio_if.h
typedef struct {
    MS_U8 supportCodec;        // Short_Audio_Descriptor Byte1 [3:6], 0: default, 1: support, 2: not suppot
    MS_U8 supportCh;           // Short_Audio_Descriptor Byte1 [0:2]
    MS_U8 supportSampleRate;   // Short_Audio_Descriptor Byte2 [0:6]
    MS_U8 supportSadByte3;     // Short_Audio_Descriptor Byte3 [0:7]
} caudDigitalOutCapability_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* ATV */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_SIF_INPUT_INVALID    = -1,
    CAUD_SIF_INPUT_EXTERNAL   = 0,
    CAUD_SIF_INPUT_INTERNAL   = 1,
} CAUD_SIF_INPUT;

typedef enum {
    CAUD_SIF_TYPE_INVALID         = -1,
    CAUD_SIF_TYPE_NONE            = 0x0000,       ///< INIT TYPE : NONE
    CAUD_SIF_ATSC_SELECT          = 0x0001,       ///< INIT TYPE : TV Systems for A2 enabled in default ATSC system
    CAUD_SIF_KOREA_A2_SELECT      = 0x0002,       ///< INIT TYPE : TV Systems for A2 enabled in Korea A2 system
    CAUD_SIF_BTSC_SELECT          = 0x0004,       ///< INIT TYPE : TV Systems for BTSC enabled in ATSC(CO, CF) or DVB(Taiwan) system
    CAUD_SIF_BTSC_BR_SELECT       = 0x0008,       ///< INIT TYPE : TV Systems for BTSC enabled in ATSC(Brazil) system
    CAUD_SIF_BTSC_US_SELECT       = 0x0010,       ///< INIT TYPE : TV Systems for BTSC enabled in ATSC(US) system
    CAUD_SIF_DVB_SELECT           = 0x0020,       ///< INIT TYPE : TV Systems for EU in default DVB system
    CAUD_SIF_DVB_ID_SELECT        = 0x0040,       ///< INIT TYPE : TV Systems for ID(Indonesia) in DVB(PAL B/G) system
    CAUD_SIF_DVB_IN_SELECT        = 0x0080,       ///< INIT TYPE : TV Systems for IN(India) in DVB(PAL B) system
    CAUD_SIF_DVB_CN_SELECT        = 0x0100,       ///< INIT TYPE : TV Systems for CN(China, Hong Kone) in DVB system
    CAUD_SIF_DVB_AJJA_SELECT      = 0x0200,       ///< INIT TYPE : TV Systems for AJ(Asia JooDong), JA(JooAang Asia) in DVB system
    CAUD_SIF_TYPE_MAX             = 0x0FFF,       ///< INIT TYPE : MAX
} CAUD_SIF_TYPE;

typedef enum {
    CAUD_SIF_STD_INVALID      = -1,
    CAUD_SIF_BG               = 0x00,         ///< Audio standard BG
    CAUD_SIF_BG_A2            = 0x01,         ///< Audio standard BG A2
    CAUD_SIF_BG_NICAM         = 0x02,         ///< Audio standard BG NICAM
    CAUD_SIF_I                = 0x03,         ///< Audio standard I
    CAUD_SIF_DK               = 0x04,         ///< Audio standard DK
    CAUD_SIF_DK1_A2           = 0x05,         ///< Audio standard DK1 A2
    CAUD_SIF_DK2_A2           = 0x06,         ///< Audio standard DK2 A2
    CAUD_SIF_DK3_A2           = 0x07,         ///< Audio standard DK3 A2
    CAUD_SIF_DK_NICAM         = 0x08,         ///< Audio standard DK NICAM
    CAUD_SIF_L                = 0x09,         ///< Audio standard L
    CAUD_SIF_M                = 0x0A,         ///< Audio standard M
    CAUD_SIF_M_BTSC           = 0x0B,         ///< Audio standard M BTSC
    CAUD_SIF_M_A2             = 0x0C,         ///< Audio standard M A2
    CAUD_SIF_M_EIA_J          = 0x0D,         ///< Audio standard M EIA J
    CAUD_SIF_NOT_STD          = 0x0F          ///< Not Audio standard
} CAUD_SIF_STD;

typedef enum {
    CAUD_SIF_MODE_SET_INVALID             =     -1,
    CAUD_SIF_SET_PAL_MONO                 =   0x00,   // PAL Mono
    CAUD_SIF_SET_PAL_MONO_FORCED          =   0x01,   // PAL Mono Force Mono
    CAUD_SIF_SET_PAL_STEREO               =   0x02,   // PAL Stereo
    CAUD_SIF_SET_PAL_STEREO_FORCED        =   0x03,   // PAL Stereo Force Mono
    CAUD_SIF_SET_PAL_DUALI                =   0x04,   // PAL Dual I
    CAUD_SIF_SET_PAL_DUALII               =   0x05,   // PAL Dual II
    CAUD_SIF_SET_PAL_DUALI_II             =   0x06,   // PAL Dual I+II
    CAUD_SIF_SET_PAL_NICAM_MONO           =   0x07,   // PAL NICAM Mono
    CAUD_SIF_SET_PAL_NICAM_MONO_FORCED    =   0x08,   // PAL NICAM Mono Force Mono
    CAUD_SIF_SET_PAL_NICAM_STEREO         =   0x09,   // PAL NICAM Stereo
    CAUD_SIF_SET_PAL_NICAM_STEREO_FORCED  =   0x0A,   // PAL NICAM Stereo Force Mono
    CAUD_SIF_SET_PAL_NICAM_DUALI          =   0x0B,   // PAL NICAM Dual I
    CAUD_SIF_SET_PAL_NICAM_DUALII         =   0x0C,   // PAL NICAM Dual II
    CAUD_SIF_SET_PAL_NICAM_DUALI_II       =   0x0D,   // PAL NICAM Dual I+II
    CAUD_SIF_SET_PAL_NICAM_DUAL_FORCED    =   0x0E,   // PAL NICAM Dual Forced Mono(Not Supported)
    CAUD_SIF_SET_PAL_UNKNOWN              =   0x0F,   // PAL Unkown State
    CAUD_SIF_SET_NTSC_A2_MONO             =   0x10,   // NTSC(A2) Mono
    CAUD_SIF_SET_NTSC_A2_STEREO           =   0x11,   // NTSC(A2) Stereo
    CAUD_SIF_SET_NTSC_A2_SAP              =   0x12,   // NTSC(A2) SAP
    CAUD_SIF_SET_NTSC_A2_UNKNOWN          =   0x13,   // NTSC(A2) Unkown State
    CAUD_SIF_SET_NTSC_BTSC_MONO           =   0x14,   // NTSC(BTSC) Mono
    CAUD_SIF_SET_NTSC_BTSC_STEREO         =   0x15,   // NTSC(BTSC) Stereo
    CAUD_SIF_SET_NTSC_BTSC_SAP_MONO       =   0x16,   // NTSC(BTSC) SAP Mono
    CAUD_SIF_SET_NTSC_BTSC_SAP_STEREO     =   0x17,   // NTSC(BTSC) SAP Stereo
    CAUD_SIF_SET_NTSC_BTSC_UNKNOWN        =   0x18,   // NTSC(BTSC) Unkown State
} CAUD_SIF_MODE_SET;

typedef enum {
    CAUD_SIF_HHIDEV_FILTER_BW1        = 0x10, // 220kHz~315kHz
    CAUD_SIF_HHIDEV_FILTER_BW2        = 0x20, // 320kHz~415kHz
    CAUD_SIF_HHIDEV_FILTER_BW3        = 0x30, // 400kHz~495kHz
} CAUD_SIF_HIDEV_BW;

///< Audio mode type
typedef enum {
    CAUD_SIF_AUDIOMODE_TYPE_INVALID   = 0x00,           ///< Audio Mode Invalid
    CAUD_SIF_AUDIOMODE_MONO           = 0x01,           ///< Audio Mode MONO
    CAUD_SIF_AUDIOMODE_FORCED_MONO    = 0x02,           ///< Audio Mode Forced MONO
    CAUD_SIF_AUDIOMODE_G_STEREO       = 0x03,           ///< Audio Mode G Stereo
    CAUD_SIF_AUDIOMODE_K_STEREO       = 0x04,           ///< Audio Mode K Stereo
    CAUD_SIF_AUDIOMODE_MONO_SAP       = 0x05,           ///< Audio Mode MONO SAP
    CAUD_SIF_AUDIOMODE_STEREO_SAP     = 0x06,           ///< Audio Mode Stereo SAP
    CAUD_SIF_AUDIOMODE_DUAL_A         = 0x07,           ///< Audio Mode Dual A
    CAUD_SIF_AUDIOMODE_DUAL_B         = 0x08,           ///< Audio Mode Dual B
    CAUD_SIF_AUDIOMODE_DUAL_AB        = 0x09,           ///< Audio Mode Dual AB
    CAUD_SIF_AUDIOMODE_NICAM_MONO     = 0x0A,           ///< Audio Mode NICAM MONO
    CAUD_SIF_AUDIOMODE_NICAM_STEREO   = 0x0B,           ///< Audio Mode NICAM Stereo
    CAUD_SIF_AUDIOMODE_NICAM_DUAL_A   = 0x0C,           ///< Audio Mode NICAM DUAL A
    CAUD_SIF_AUDIOMODE_NICAM_DUAL_B   = 0x0D,           ///< Audio Mode NICAM DUAL B
    CAUD_SIF_AUDIOMODE_NICAM_DUAL_AB  = 0x0E,           ///< Audio Mode NICAM DUAL AB
    CAUD_SIF_AUDIOMODE_HIDEV_MONO     = 0x0F,           ///< Audio Mode HIDEV MONO
    CAUD_SIF_AUDIOMODE_LEFT_LEFT      = 0x10,           ///< Audio Mode Left left
    CAUD_SIF_AUDIOMODE_RIGHT_RIGHT    = 0x11,           ///< Audio Mode right right
    CAUD_SIF_AUDIOMODE_LEFT_RIGHT     = 0x12            ///< Audio Mode left right
} CAUD_SIF_AUDIOMODE_TYPE;

///< Threshold table structure
typedef struct {
    MS_U8 hiByte;                 ///< Threshold high-byte
    MS_U8 lowByte;                ///< Threshold low-byte
} caudSifThrTbl_t;

typedef enum {
    CAUD_SIF_PAL_MONO,
    CAUD_SIF_PAL_HIDEV,
    CAUD_SIF_PAL_A2,
    CAUD_SIF_PAL_NICAM
} CAUD_SIF_PAL_TYPE;

///< ATV SIF Command
typedef enum {
    CAUD_SIF_CMD_SET_STOP                  = 0x00,
    CAUD_SIF_CMD_SET_PLAY                  = 0x01,
    CAUD_SIF_CMD_ENABLE_HIDEV              = 0x02,
    CAUD_SIF_CMD_SET_HIDEV_FILTER_BW_LVL   = 0x03,
    CAUD_SIF_CMD_RESET_FC_TRACKING         = 0x04,
    CAUD_SIF_CMD_ENABLE_FC_TRACKING        = 0x05,
    CAUD_SIF_CMD_SET_ADC_FROM_VIF_PATH     = 0x06,
    CAUD_SIF_CMD_ENABLE_AUTO_MUTE          = 0x07,
    CAUD_SIF_CMD_ENABLE_BEEPER_FUNCTION    = 0x08,
    CAUD_SIF_CMD_SET_BEEPER_TONE           = 0x09,
    CAUD_SIF_CMD_ENABLE_AGC                = 0x0A,
    CAUD_SIF_CMD_RESET_AGC                 = 0x0B,
    CAUD_SIF_CMD_STD_RESET                 = 0x0C,
    CAUD_SIF_CMD_DETECT_MAIN_STD_ONLY      = 0x0D,
    CAUD_SIF_CMD_ENABLE_AUTO_SND_MODE      = 0x0E,
    CAUD_SIF_CMD_FMTX_SET_PREEMP           = 0x0F,
    CAUD_SIF_CMD_SET_ENC_CMD               = 0x10,
    CAUD_SIF_CMD_SET_ENC_Input_attenuation = 0x11,
    CAUD_SIF_CMD_SET_ENC_Output_scaling    = 0x12,
    CAUD_SIF_CMD_SET_BTSC_Enc_M_Gain       = 0x13,
    CAUD_SIF_CMD_SET_BTSC_Enc_D_Gain       = 0x14,
    CAUD_SIF_CMD_SET_BTSC_Enc_SAP_Gain     = 0x15,
    CAUD_SIF_CMD2_ADC_VIA_VIF_PIN          = 0x01
} CAUD_SIF_CMD_TYPE;

///< sif carrier status define
typedef enum {
    // Never change the value of enumeration. Because each bit has weight.
    CAUD_SIF_STATE_NO_CARRIER         = 0x00, ///< No carrier detect
    CAUD_SIF_STATE_PRIMARY_CARRIER    = 0x01, ///< Carrier 1 exist
    CAUD_SIF_STATE_SECONDARY_CARRIER  = 0x02, ///< Carrier 2 exist
    CAUD_SIF_STATE_NICAM              = 0x04, ///< Nicam lock state
    CAUD_SIF_STATE_STEREO             = 0x08, ///< A2 Stereo exist
    CAUD_SIF_STATE_BILINGUAL          = 0x10, ///< A2 Dual exist
    CAUD_SIF_STATE_PILOT              = 0x20, ///< A2 Pilot exist
    CAUD_SIF_STATE_DK2                = 0x40, ///< Sound standard is DK2
    CAUD_SIF_STATE_DK3                = 0x80  ///< Sound standard is DK3
} CAUD_SIF_AUDIOSTATUS;

//======SIF Gain Type========
typedef enum {
    CAUD_SIF_PRESCALE_STEP_ONE_DB         = 0x04,
    CAUD_SIF_SET_PRESCALE_A2_FM           = 0x00,
    CAUD_SIF_SET_PRESCALE_NICAM           = 0x01,
    CAUD_SIF_SET_PRESCALE_AM              = 0x02,
    CAUD_SIF_SET_PRESCALE_HIDEV           = 0x03,
    CAUD_SIF_SET_PRESCALE_FM_M            = 0x04,
    CAUD_SIF_SET_PRESCALE_HIDEV_M         = 0x05,

    CAUD_SIF_SET_PRESCALE_BTSC            = 0x00,
    CAUD_SIF_SET_PRESCALE_BTSC_MONO       = 0x01,
    CAUD_SIF_SET_PRESCALE_BTSC_STEREO     = 0x02,
    CAUD_SIF_SET_PRESCALE_BTSC_SAP        = 0x03,

    CAUD_SIF_SET_PRESCALE_FM_RADIO        = 0x00,
    CAUD_SIF_SET_PRESCALE_FM_RADIO_MONO   = 0x01,
    CAUD_SIF_SET_PRESCALE_FM_RADIO_STEREO = 0x02

} CAUD_SIF_GAIN_TYPE;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Decoder */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_DOLBY_DRC_MODE_INVALID = -1,
    CAUD_DOLBY_DRC_LINE,
    CAUD_DOLBY_DRC_RF,
} CAUD_DOLBY_DRC_MODE;

typedef enum {
    CAUD_DOWNMIX_MODE_INVALID = -1,
    CAUD_DOWNMIX_LTRT,
    CAUD_DOWNMIX_LORO,
} CAUD_DOWNMIX_MODE;

typedef enum {
    CAUD_DUALMONO_MODE_INVALID = -1,
    CAUD_DUALMONO_LR,
    CAUD_DUALMONO_LL,
    CAUD_DUALMONO_RR,
    CAUD_DUALMONO_MIX,
} CAUD_DUALMONO_MODE;

typedef enum {
    CAUD_SAMPLE_RATE_INVALID    =       -1,
    CAUD_SAMPLE_RATE_NONE       =        0,
    CAUD_SAMPLE_RATE_4_KHZ      =     4000,
    CAUD_SAMPLE_RATE_8_KHZ      =     8000,
    CAUD_SAMPLE_RATE_11_025KHZ  =    11025,
    CAUD_SAMPLE_RATE_12_KHZ     =    12000,
    CAUD_SAMPLE_RATE_16_KHZ     =    16000,
    CAUD_SAMPLE_RATE_22_05KHZ   =    22050,
    CAUD_SAMPLE_RATE_24_KHZ     =    24000,
    CAUD_SAMPLE_RATE_32_KHZ     =    32000,
    CAUD_SAMPLE_RATE_44_1KHZ    =    44100,
    CAUD_SAMPLE_RATE_48_KHZ     =    48000,
    CAUD_SAMPLE_RATE_64_KHZ     =    64000,
    CAUD_SAMPLE_RATE_88_2KHZ    =    88200,
    CAUD_SAMPLE_RATE_96_KHZ     =    96000,
    CAUD_SAMPLE_RATE_128_KHZ    =   128000,
    CAUD_SAMPLE_RATE_176_4KHZ   =   176400,
    CAUD_SAMPLE_RATE_192_KHZ    =   192000,
    CAUD_SAMPLE_RATE_768_KHZ    =   768000,
    CAUD_SAMPLE_RATE_DEFAULT    =   999000,
} CAUD_SAMPLE_RATE;

typedef enum {
    CAUD_CH_MODE_INVALID      = -1,
    CAUD_CH_MODE_DUAL_MONO    = 0,  //MPEG/Dolby
    CAUD_CH_MODE_MONO         = 1,  //MPEG/Dolby
    CAUD_CH_MODE_STEREO       = 2,  //MPEG/Dolby
    CAUD_CH_MODE_3_0          = 3,  //     Dolby
    CAUD_CH_MODE_2_1          = 4,  //     Dolby
    CAUD_CH_MODE_3_1          = 5,  //     Dolby
    CAUD_CH_MODE_2_2          = 6,  //     Dolby
    CAUD_CH_MODE_3_2          = 7,  //     Dolby
    CAUD_CH_MODE_JOINT_STEREO = 8,  //MPEG/
    CAUD_CH_MODE_3_3          = 9,  //DRA
    CAUD_CH_MODE_3_2_2        = 10, //DRA /Dolby
    CAUD_CH_MODE_3_2_3        = 11, //DRA
    CAUD_CH_MODE_NONE         = 12,
} CAUD_CH_MODE;

//need to be consistent with XPCM_TYPE of drvAudio_if.h
typedef enum {
    CAUD_XPCM_LPCM            = 1,
    CAUD_XPCM_MS_ADPCM        = 2,
    CAUD_XPCM_G711_A_LAW      = 6,
    CAUD_XPCM_G711_u_LAW      = 7,
    CAUD_XPCM_DVD_LPCM        = 8,
    CAUD_XPCM_PRIVATE1_LPCM   = 9,
    CAUD_XPCM_IMA_ADPCM       = 17,
    CAUD_XPCM_IMA_ADPCM_APPLE = 18, // apple IMA4
} CAUD_XPCM_TYPE;

//need to be consistent with Audio_XPCM_Param of drvAudio_if.h
typedef struct {
    CAUD_XPCM_TYPE type;
    MS_U32 sampleRate;
    MS_U16 blockSize;
    MS_U16 samplePerBlock;
    MS_U8  chNum;
    MS_U8  bitsPerSample;
    MS_U32 dwChMask;        //  #define SPEAKER_FRONT_LEFT             0x1
                            //  #define SPEAKER_FRONT_RIGHT            0x2
                            //  #define SPEAKER_FRONT_CENTER           0x4
                            //  #define SPEAKER_LOW_FREQUENCY          0x8
                            //  #define SPEAKER_BACK_LEFT              0x10
                            //  #define SPEAKER_BACK_RIGHT             0x20 // <--- only support to here
                            //  #define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
                            //  #define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
                            //  #define SPEAKER_BACK_CENTER            0x100
                            //  #define SPEAKER_SIDE_LEFT              0x200
                            //  #define SPEAKER_SIDE_RIGHT             0x400
                            //  #define SPEAKER_TOP_CENTER             0x800
                            //  #define SPEAKER_TOP_FRONT_LEFT         0x1000
                            //  #define SPEAKER_TOP_FRONT_CENTER       0x2000
                            //  #define SPEAKER_TOP_FRONT_RIGHT        0x4000
                            //  #define SPEAKER_TOP_BACK_LEFT          0x8000
                            //  #define SPEAKER_TOP_BACK_CENTER        0x10000
                            //  #define SPEAKER_TOP_BACK_RIGHT         0x20000
                            //  #define SPEAKER_RESERVED               0x80000000
} caudXpcm_t;

//need to be consistent with Audio_COOK_Param of drvAudio_if.h
typedef struct {
    #define COOK_MAX_NUM_CODECS 5
    MS_U16 chNum[COOK_MAX_NUM_CODECS];
    MS_U16 regions[COOK_MAX_NUM_CODECS];
    MS_U16 cplStart[COOK_MAX_NUM_CODECS];
    MS_U16 cplQbits[COOK_MAX_NUM_CODECS];
    MS_U16 frmSize[COOK_MAX_NUM_CODECS];
    MS_U16 numCodecs;
    MS_U16 samples;
    MS_U16 sampleRate;
} caudCook_t;

typedef enum {
    CAUD_TRICK_MODE_INVALID       = -1,
    CAUD_TRICK_NONE               = 0,    ///<  rate : None, TP Live Play
    CAUD_TRICK_PAUSE              = 1,    ///<  rate : Pause, DVR Play
    CAUD_TRICK_NORMAL_PLAY        = 2,    ///<  rate : 1.00 Play
    CAUD_TRICK_SLOW_MOTION_0P25X  = 3,    ///<  rate : 0.25 Play
    CAUD_TRICK_SLOW_MOTION_0P50X  = 4,    ///<  rate : 0.50 Play
    CAUD_TRICK_SLOW_MOTION_0P75X  = 10,   ///<  rate : 0.75 Play
    CAUD_TRICK_SLOW_MOTION_0P80X  = 8,    ///<  rate : 0.80 Play
    CAUD_TRICK_FAST_FORWARD_1P20X = 9,    ///<  rate : 1.20 Play
    CAUD_TRICK_FAST_FORWARD_1P25X = 5,    ///<  rate : 1.25 Play
    CAUD_TRICK_FAST_FORWARD_1P50X = 6,    ///<  rate : 1.50 Play
    CAUD_TRICK_FAST_FORWARD_1P75X = 11,   ///<  rate : 1.75 Play //<--MAX is here!!!
    CAUD_TRICK_FAST_FORWARD_2P00X = 7,    ///<  rate : 2.00 Play
} CAUD_TRICK_MODE;

typedef enum {
    CAUD_MPEG_LAYER_INVALID             = -1,
    CAUD_MPEG_LAYER_RESERVED,
    CAUD_MPEG_LAYER1,
    CAUD_MPEG_LAYER2,
    CAUD_MPEG_LAYER3,
    CAUD_MPEG_LAYER_NOT_DETERMINE_YET   = 0xFF,
} CAUD_MPEG_LAYER;

//need to be consistent with AUDIO_AAC_TYPE of drvAudio_if.h
typedef enum {
    CAUD_GENERIC_AAC,
    CAUD_GENERIC_HEAACv1,
    CAUD_GENERIC_HEAACv2,
    CAUD_DOLBY_AAC,
    CAUD_DOLBY_HEAACv1,
    CAUD_DOLBY_HEAACv2,
} CAUD_AAC_TYPE;

//need to be consistent with Buffer_Info_t of drvAudio_if.h
typedef struct {
    MS_U32 size;
    MS_U64 pts;
    MS_U64 metadataIdx;
    MS_U8  *pBuf;
} caudFrmDecodeBuf_t;

//need to be consistent with Frm_Decode_Pcm_Info_t in drvAudio_if.h
typedef struct {
    MS_U32 pcmBufBase;
    MS_U32 pcmBufSize;
    MS_U32 pcmWrPtr;
    MS_U64 pts;
    MS_U32 size;
    MS_U32 lvl;
    MS_U32 rdIdx;
} caudFrmDecodePcm_t;

//need to be consistent with DECODER_PCM_OUTPUT_TYPE of drvAudio_if.h
typedef enum {
    CAUD_DEC_PCM_OUT_TYPE1_DMX,         /* Downmix, target channel = 2 */
    CAUD_DEC_PCM_OUT_TYPE2_6CH,         /* Multi-Channel(5.1), target channel = 6 */
    CAUD_DEC_PCM_OUT_TYPE3_8CH,         /* Multi-Channel(7.1), target channel = 8 */
    CAUD_DEC_PCM_OUT_TYPE4_DMX_8CH,     /* Downmix, Multi-Channel(7.1), target channel = 10 */
    CAUD_DEC_PCM_OUT_TYPE5_DMX_8CH_PTR, /* Downmix, Multi-Channel(7.1), nonPcmPtr, target channel = 12 */
} CAUD_DEC_PCM_OUT_TYPE;

#define CAUD_VDP_WMA_PAYLOAD_HEADER_SIZE    16
#define CAUD_VDP_AC4_PAYLOAD_HEADER_SIZE    7
#define CAUD_VDP_OPUS_SYNC_HEADER_SIZE      7
#define CAUD_VDP_OPUS_DATA_HEADER_SIZE      14

typedef struct {
    MS_BOOL bFirstEs;
    MS_U8 chNum;
    MS_U8 preSkip[2];
    MS_U8 outputGain[2];
    MS_U8 mappingFamily;
    MS_U8 streamCnt;
    MS_U8 coupledCnt;
    MS_U8 chMapping[6];
} caudOpusHeader_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Common Cmd */
//-------------------------------------------------------------------------------------------------------------------------------------
//need to be consistent with AUDIO_COMMON_CMD_t of drvAudio_if.h
typedef struct {
    MS_U8 cmd[64];
    void *pData;
} caudCommonCmd_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Common */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_PARAM_TYPE_INVALID                       = -1,

    //--------------------------------------------------------------------------------------
    // Decoder (CAUD_"ADEC"_PARAM_XXXXXXXX)
    //--------------------------------------------------------------------------------------
    //---MPEG---
    CAUD_ADEC_PARAM_MPEG_SND_MODE                 = 0,

    //---Dolby AC3P---
    CAUD_ADEC_PARAM_AC3P_DRC_MODE                 = 1,
    CAUD_ADEC_PARAM_AC3P_DOWNMIX_MODE             = 2,
    CAUD_ADEC_PARAM_AC3P_DOLBY_BULLETIN11         = 3,
    CAUD_ADEC_PARAM_AC3P_HDMITX_BYPASS_EN         = 4,
    CAUD_ADEC_PARAM_AC3P_DRC_HIGHCUT_SCALE        = 5,
    CAUD_ADEC_PARAM_AC3P_DRC_LOWBOOT_SCALE        = 6,
    CAUD_ADEC_PARAM_AC3P_1PID_AD_EN               = 37,
    CAUD_ADEC_PARAM_AC3P_RESET_ATMOS_EVO_IDX      = 45,

    //---Dolby AC4---
    CAUD_ADEC_PARAM_AC4_1ST_PREF_LANG             = 48,
    CAUD_ADEC_PARAM_AC4_2ND_PREF_LANG             = 49,
    CAUD_ADEC_PARAM_AC4_AD_TYPE                   = 50,
    CAUD_ADEC_PARAM_AC4_PRIORITIZE_AD_TYPE        = 51,
    CAUD_ADEC_PARAM_AC4_DE_GAIN                   = 54,
    CAUD_ADEC_PARAM_AC4_MD_PRESENTATION_IDX       = 55,
    CAUD_ADEC_PARAM_AC4_AD_PRESENTATION_IDX       = 56,
    CAUD_ADEC_PARAM_AC4_LANGUAGE_TYPE             = 57,

    //---AAC---
    CAUD_ADEC_PARAM_AAC_DRC_MODE                  = 7,
    CAUD_ADEC_PARAM_AAC_DOWNMIX_MODE              = 8,

    //---WMA---
    CAUD_ADEC_PARAM_WMA_VERSION                   = 9,
    CAUD_ADEC_PARAM_WMA_CH_NUM                    = 10,
    CAUD_ADEC_PARAM_WMA_SAMPLERATE                = 11,
    CAUD_ADEC_PARAM_WMA_BYTERATE                  = 12,
    CAUD_ADEC_PARAM_WMA_BLOCKALIGN                = 13,
    CAUD_ADEC_PARAM_WMA_ENCOPT                    = 14,
    CAUD_ADEC_PARAM_WMA_PARSINGBYAPP              = 15,
    CAUD_ADEC_PARAM_WMA_BITS_PER_SAMPLE           = 16,
    CAUD_ADEC_PARAM_WMA_CH_MASK                   = 17,
    CAUD_ADEC_PARAM_WMA_DRC_PARAM_EXIST           = 18,
    CAUD_ADEC_PARAM_WMA_DRC_RMS_AMP_REF           = 19,
    CAUD_ADEC_PARAM_WMA_DRC_RMS_AMP_TARGET        = 20,
    CAUD_ADEC_PARAM_WMA_DRC_PEAK_AMP_REF          = 21,
    CAUD_ADEC_PARAM_WMA_DRC_PEAK_AMP_TARGET       = 22,
    CAUD_ADEC_PARAM_WMA_MAX_PACKET_SIZE           = 23,

    //---XPCM---
    CAUD_ADEC_PARAM_XPCM                          = 24,

    //---COOK---
    CAUD_ADEC_PARAM_COOK                          = 42,

    //---Underrun threshold in ms, Debounce Cnt---
    CAUD_ADEC_PARAM_UNDERRUN_THRESHOLD_IN_MS      = 26,
    CAUD_ADEC_PARAM_UNDERRUN_DEBOUNCE_CNT         = 27,

    //---Decoder Setting---
    CAUD_ADEC_PARAM_AVDLY                         = 38,
    CAUD_ADEC_PARAM_FORCEMCHOUT                   = 36,
    CAUD_ADEC_PARAM_DUALMONO                      = 47,
    CAUD_ADEC_PARAM_DUAL_MAIN_DECODE              = 63,

    //---MM Uni---
    CAUD_ADEC_PARAM_MM_UNI_NEED_DECODE_FRMCNT     = 35,
    CAUD_ADEC_PARAM_MM_UNI_SPDIF_PCM_LVL          = 40,
    CAUD_ADEC_PARAM_MM_UNI_ES_WPTR                = 41,
    CAUD_ADEC_PARAM_MM_UNI_PCM_OUT_CNT            = 43,
    CAUD_ADEC_PARAM_MM_UNI_MS12_ATMOS_VALID       = 46,
    CAUD_ADEC_PARAM_MM_UNI_MS12_EVO_PCMR_META_IDX = 44,

    //--------------------------------------------------------------------------------------
    // ASND (CAUD_"ASND"_PARAM_XXXXXXXX)
    //--------------------------------------------------------------------------------------
    //---atmos---
    CAUD_ASND_PARAM_ATMOS_LOCK                    = 39,
    CAUD_ASND_PARAM_AVR_SUPPORT_ATMOS             = 53,

    //--------------------------------------------------------------------------------------
    // AENC (CAUD_"AENC"_PARAM_XXXXXXXX)
    //--------------------------------------------------------------------------------------
    //---AENC start time
    CAUD_AENC_PARAM_AAC_START_TIME                = 64,

    //--------------------------------------------------------------------------------------
    // Common (CAUD_"COMMON"_PARAM_XXXXXXXX)
    //--------------------------------------------------------------------------------------
    //---Audio Dump Info---
    CAUD_COMMON_PARAM_DUMP_INFO_Off               = 25,
    CAUD_COMMON_PARAM_DUMP_INFO_AVSYNC            = 28,
    CAUD_COMMON_PARAM_DUMP_INFO_ADEC0             = 29,
    CAUD_COMMON_PARAM_DUMP_INFO_IO_INFO           = 30,
    CAUD_COMMON_PARAM_DUMP_INFO_MM_FILE_AVSYNC    = 31,
    CAUD_COMMON_PARAM_DUMP_INFO_ALL               = 32,
    CAUD_COMMON_PARAM_DUMP_INFO_AUTOTEST          = 33,

    //---Common Cmd---
    CAUD_COMMON_PARAM_COMMON_CMD                  = 34,
    CAUD_COMMON_PARAM_SYNC_STC_IN_MMTS            = 52,
    CAUD_COMMON_PARAM_ENC_CH_LOCK                 = 58,
    CAUD_COMMON_PARAM_DDI_VERSION                 = 65, //<--MAX is here!!!

    //---Power saving---
    CAUD_COMMON_PARAM_LINEIN_POWER_ON             = 59,
    CAUD_COMMON_PARAM_LINEOUT_POWER_ON            = 60,
    CAUD_COMMON_PARAM_ADC_POWER_ON                = 61,
    CAUD_COMMON_PARAM_DAC_POWER_ON                = 62,

    //---MAX---
    CAUD_PARAM_TYPE_MAX = CAUD_COMMON_PARAM_DDI_VERSION + 1,
} CAUD_PARAM_TYPE;

typedef enum {
    CAUD_ADEC_INFO_TYPE_INVALID                       = -1,

    //--------------------------------------------------------------------------------------
    // Decoder (CAUD_"ADEC"_INFO_XXXXXXXX)
    //--------------------------------------------------------------------------------------
    //---MPEG---
    CAUD_ADEC_INFO_MPEG_SND_MODE                      = 0,
    CAUD_ADEC_INFO_MPEG_LAYER                         = 25,

    //---Dolby AC3P---
    CAUD_ADEC_INFO_AC3P_DRC_MODE                      = 1,
    CAUD_ADEC_INFO_AC3P_DOWNMIX_MODE                  = 2,
    CAUD_ADEC_INFO_AC3P_IS_ATMOS_STREAM               = 18,

    //---AAC---
    CAUD_ADEC_INFO_AAC_DRC_MODE                       = 3,
    CAUD_ADEC_INFO_AAC_DOWNMIX_MODE                   = 4,
    CAUD_ADEC_INFO_AAC_TYPE                           = 23,

    //---Decode Info---
    CAUD_ADEC_INFO_CONNECT                            = 51,
    CAUD_ADEC_INFO_AD_EN                              = 52,
    CAUD_ADEC_INFO_IS_MM                              = 53, //<--MAX is here!!!
    CAUD_ADEC_INFO_SAMPLERATE                         = 9,
    CAUD_ADEC_INFO_ACMODE                             = 10,
    CAUD_ADEC_INFO_AVSYNC_STATE                       = 11,
    CAUD_ADEC_INFO_FORCE_MCH_OUT                      = 26,
    CAUD_ADEC_INFO_PCM_OUT_TYPE                       = 27,
    CAUD_ADEC_INFO_PCM_OUT_CH                         = 36,
    CAUD_ADEC_INFO_CODEC                              = 38,

    //---MM Uni---
    CAUD_ADEC_INFO_MM_UNI_PCM1_DECODE_FRM_CNT         = 33,
    CAUD_ADEC_INFO_MM_UNI_PCM1_DECODE_WPTR            = 34,
    CAUD_ADEC_INFO_MM_UNI_PCM1_DECODE_WBYTES          = 35,
    CAUD_ADEC_INFO_MM_UNI_MS12_EVO_PCMR_META_IDX      = 37,

    //--------------------------------------------------------------------------------------
    // Common (CAUD_"COMMON"_INFO_XXXXXXXX)
    //--------------------------------------------------------------------------------------
    //---ES---
    CAUD_COMMON_INFO_ES_BUF_BASE                      = 5,
    CAUD_COMMON_INFO_ES_BUF_SIZE                      = 6,
    CAUD_COMMON_INFO_ES_BUF_CUR_LVL                   = 32,

    //---AD_ES (Audio description)---
    CAUD_COMMON_INFO_AD_ES_BUF_BASE                   = 14,
    CAUD_COMMON_INFO_AD_ES_BUF_SIZE                   = 15,

    //---PCM---
    CAUD_COMMON_INFO_PCM_BUF_BASE                     = 28,
    CAUD_COMMON_INFO_PCM_BUF_SIZE                     = 29,
    CAUD_COMMON_INFO_PCM_BUF_WPTR                     = 30,
    CAUD_COMMON_INFO_PCM_BUF_CUR_LVL                  = 7,
    CAUD_COMMON_INFO_PCM_BUF_CUR_LVL_IN_MS            = 12,

    //---PTS---
    CAUD_COMMON_INFO_33BIT_PTS                        = 8,
    CAUD_COMMON_INFO_AD_33BIT_PTS                     = 24,

    //---OK, ERR Frame Cnt, IS_MAD_LOCK---
    CAUD_COMMON_INFO_OK_FRM_CNT                       = 13,
    CAUD_COMMON_INFO_ERR_FRM_CNT                      = 16,
    CAUD_COMMON_INFO_IS_MAD_LOCK                      = 21,
    CAUD_COMMON_INFO_IS_R2_DECODE                     = 31,
    CAUD_COMMON_INFO_SYNC_STC_IN_MMTS                 = 49,

    //---Mute---
    CAUD_COMMON_INFO_I2S_MUTE                         = 39,
    CAUD_COMMON_INFO_LINEOUT0_MUTE                    = 40,
    CAUD_COMMON_INFO_LINEOUT1_MUTE                    = 41,
    CAUD_COMMON_INFO_LINEOUT2_MUTE                    = 42,
    CAUD_COMMON_INFO_LINEOUT3_MUTE                    = 43,
    CAUD_COMMON_INFO_SPDIF_MUTE                       = 44,
    CAUD_COMMON_INFO_ARC_MUTE                         = 45,
    CAUD_COMMON_INFO_PCM_CAPTURE0_MUTE                = 46,
    CAUD_COMMON_INFO_PCM_CAPTURE1_MUTE                = 47,
    CAUD_COMMON_INFO_PCM_CAPTURE2_MUTE                = 48,

    //---Delay---
    CAUD_COMMON_INFO_AUDIO_DELAY                      = 17,
    CAUD_COMMON_INFO_SEDSP_MS12_DDE_DELAY             = 19,
    CAUD_COMMON_INFO_SNDR2_MS12_DDPE_DELAY            = 20,
    CAUD_COMMON_INFO_SNDR2_MS12_DAP_DELAY             = 22,
    CAUD_COMMON_INFO_ADEC_INTERNAL_DELAY              = 50,

    //---MAX---
    CAUD_INFO_TYPE_MAX = CAUD_ADEC_INFO_IS_MM + 1,
} CAUD_INFO_TYPE;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Customized patch */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_PATCH_PARAM_TYPE_INVALID = -1,

    //--------------------------------------------------------------------------------------
    // (CAUD_"CUSTOMIZED_PATCH"_PARAM_XXXXXXXX), PARAM: Set
    //--------------------------------------------------------------------------------------
    //---ES Passthrough---
    CAUD_PATCH_PARAM_ES_PASSTHROUGH_DELAY_SAMPLE                = 0,
    CAUD_PATCH_PARAM_ES_PASSTHROUGH_PCM_SAMPLERATE              = 1,
    CAUD_PATCH_PARAM_ES_PASSTHROUGH_HDMI_SAMPLERATE             = 2,

    //---PCM Capture set rptr---
    CAUD_PATCH_PARAM_PCM_CAPTURE1_RPTR                          = 3,
    CAUD_PATCH_PARAM_PCM_CAPTURE2_RPTR                          = 4,

    //--- HDMI RX Bypass---
    CAUD_PATCH_PARAM_HDMI_RX_BYPASS_EN                          = 5,

    //---Low Latency Limiter---
    CAUD_PATCH_PARAM_LOW_LATENCY_LIMITER_EN                     = 6,
    CAUD_PATCH_PARAM_LOW_LATENCY_LIMITER_ES_THRESHOLD           = 7,
    CAUD_PATCH_PARAM_LOW_LATENCY_LIMITER_PCM_THRESHOLD          = 8,
    CAUD_PATCH_PARAM_LOW_LATENCY_LIMITER_PCM_SKIP_TARGET        = 20,

    //---DOLBY_ATMOS_CONTROL---
    CAUD_PATCH_PARAM_DOLBY_ATMOS_EN                             = 17,
    CAUD_PATCH_PARAM_DOLBY_ATMOS_DISABLE                        = 18,

    //---MCU Dump R2 Log---
    CAUD_PATCH_PARAM_MCU_DUMP_R2_LOG_EN                         = 21,
    CAUD_PATCH_PARAM_MCU_DUMP_R2_LOG_OPTION                     = 22,

    //---ES Repeat Play---
    CAUD_PATCH_PARAM_ES_REPLAY_EN                               = 23,

    //---HDMI_TX / SPDIF_TX SE-DSP Force DD/DDP Encode without Main sound---
    CAUD_PATCH_PARAM_HDMI_TX_SEDSP_DDE_WO_MAIN_SND_EN           = 24,
    CAUD_PATCH_PARAM_HDMI_TX_SEDSP_DDPE_WO_MAIN_SND_EN          = 25,
    CAUD_PATCH_PARAM_SPDIF_TX_SEDSP_DDE_WO_MAIN_SND_EN          = 26,
    CAUD_PATCH_PARAM_SEDSP_MS12_ENC_WO_MAIN_SND_DISABLE         = 27,

    //---HDMI_ARC / SPDIF_TX SNDR2 Force DD/DDP Encode without Main sound---
    CAUD_PATCH_PARAM_HDMI_ARC_SEDSP_DDE_WO_MAIN_SND_EN          = 29,
    CAUD_PATCH_PARAM_HDMI_ARC_SNDR2_DDPE_WO_MAIN_SND_EN         = 30,
    CAUD_PATCH_PARAM_SNDR2_MS12_ENC_WO_MAIN_SND_DISABLE         = 31,

    //---HDMI_RX_SWITCH_TO_PCM_DEBOUNCE---
    CAUD_PATCH_PARAM_HDMI_RX_SWITCH_DEBOUNCE                    = 19,

    //---HDMI_RX_MONITOR_EVENT_CALLBACK_REGISTER---
    CAUD_PATCH_PARAM_HDMI_RX_MONITOR_EVENT_CALLBACK_REGISTER    = 28,

    //---Frame Decode only cmd---
    CAUD_PATCH_PARAM_FRM_DECODE_CMD                             = 32,
    CAUD_PATCH_PARAM_FRM_DECODE_PCM_RD_IDX                      = 83,

    //---SDM L/R swap---
    CAUD_PATCH_PARAM_SDM_SWAP_LINEOUT0                          = 33,
    CAUD_PATCH_PARAM_SDM_SWAP_LINEOUT1                          = 34,
    CAUD_PATCH_PARAM_SDM_SWAP_LINEOUT2                          = 35,
    CAUD_PATCH_PARAM_SDM_SWAP_LINEOUT3                          = 36,

    //---PCM capture request size in sample unit, must be multiple of 16---
    CAUD_PATCH_PARAM_PCM_CAPTURE_TARGET_LVL                     = 84,

    //---PCM capture for multichannel(5.1)
    CAUD_PATCH_PARAM_PCM_CAPTURE_MCH_EN                         = 85,

    //---Let LFE mix into downmix PCM data
    CAUD_PATCH_PARAM_LFE_MIXED_INTO_DMX_EN                      = 87,
    CAUD_PATCH_PARAM_LFE_MIXED_INTO_DMX_GAIN                    = 90, //<--MAX is here!!!

    //--------------------------------------------------------------------------------------
    // (CAUD_"CUS*SE*"_PARAM_XXXXXXXX), PARAM: Set
    //--------------------------------------------------------------------------------------
    //---CUS1SE1---
    CAUD_CUS1SE1_ENABLE                                                    = 9,
    CAUD_CUS1SE1_AUTO_VOLUME_CTRL                                          = 10,
    CAUD_CUS1SE1_WIDE_STEREO_CTRL                                          = 11,
    CAUD_CUS1SE1_WALL_FILTER_CTRL                                          = 12,
    CAUD_CUS1SE1_HEADROOM_GAIN_CTRL                                        = 13,
    CAUD_CUS1SE1_GAIN_ADJUSTMENT_CTRL                                      = 14,
    CAUD_CUS1SE1_HARD_LIMITER_CTRL                                         = 15,
    CAUD_CUS1SE1_MODE_IDENTIFICATION_CTRL                                  = 16,
    //---CUS2SE1---
    CAUD_CUS2SE1_PATH0_ENABLE                                              = 82,
    CAUD_CUS2SE1_PATH1_ENABLE                                              = 86,
    CAUD_CUS2SE1_PATH0_PARAM_ENABLE                                        = 88,
    CAUD_CUS2SE1_PATH1_PARAM_ENABLE                                        = 89,
    CAUD_CUS2SE1_PARAM_FN_MODE_REGISTER                                    = 37,
    CAUD_CUS2SE1_PARAM_RESET                                               = 38,
    CAUD_CUS2SE1_PARAM_INIT_FLAG                                           = 39,
    CAUD_CUS2SE1_PARAM_FN_NUMBER_OF_OUTPUTS                                = 40,
    CAUD_CUS2SE1_PARAM_FN_OUT_SAMPLING_FREQUENCY                           = 41,
    CAUD_CUS2SE1_PARAM_FNMAIN_INIT                                         = 42,
    CAUD_CUS2SE1_PARAM_FNMAIN_VARIABLES                                    = 43,
    CAUD_CUS2SE1_PARAM_FN000_INIT                                          = 44,
    CAUD_CUS2SE1_PARAM_FN000_VARIABLES                                     = 45,
    CAUD_CUS2SE1_PARAM_FN001_INIT                                          = 46,
    CAUD_CUS2SE1_PARAM_FN001_VARIABLES                                     = 47,
    CAUD_CUS2SE1_PARAM_FN004_MODE1_VARIABLES                               = 48,
    CAUD_CUS2SE1_PARAM_FN004_MODE2_VARIABLES                               = 49,
    CAUD_CUS2SE1_PARAM_FN008_INIT                                          = 50,
    CAUD_CUS2SE1_PARAM_FN008_VARIABLES                                     = 51,
    CAUD_CUS2SE1_PARAM_FN009_INIT                                          = 52,
    CAUD_CUS2SE1_PARAM_FN009_VARIABLES                                     = 53,
    CAUD_CUS2SE1_PARAM_FN010_INIT                                          = 54,
    CAUD_CUS2SE1_PARAM_FN010_VARIABLES                                     = 55,
    CAUD_CUS2SE1_PARAM_FN010_OUT                                           = 56,
    CAUD_CUS2SE1_PARAM_FN011_INIT                                          = 57,
    CAUD_CUS2SE1_PARAM_FN011_VARIABLES                                     = 58,
    CAUD_CUS2SE1_PARAM_FN014_INIT                                          = 59,
    CAUD_CUS2SE1_PARAM_FN014_VARIABLES                                     = 60,
    CAUD_CUS2SE1_PARAM_FN014_OUT                                           = 61,
    CAUD_CUS2SE1_PARAM_FN016_INIT                                          = 62,
    CAUD_CUS2SE1_PARAM_FN016_VARIABLES                                     = 63,
    CAUD_CUS2SE1_PARAM_FN017_VARIABLES                                     = 64,
    CAUD_CUS2SE1_PARAM_FN018_INIT                                          = 65,
    CAUD_CUS2SE1_PARAM_FN018_VARIABLES                                     = 66,
    CAUD_CUS2SE1_PARAM_FN019_INIT                                          = 67,
    CAUD_CUS2SE1_PARAM_FN019_VARIABLES                                     = 68,
    CAUD_CUS2SE1_PARAM_FN020_VARIABLES                                     = 69,
    CAUD_CUS2SE1_PARAM_FN022_INIT                                          = 70,
    CAUD_CUS2SE1_PARAM_FN022_VARIABLES                                     = 71,
    CAUD_CUS2SE1_PARAM_FN023_INIT                                          = 72,
    CAUD_CUS2SE1_PARAM_FN023_VARIABLES                                     = 73,
    CAUD_CUS2SE1_PARAM_FN024_INIT                                          = 74,
    CAUD_CUS2SE1_PARAM_FN024_VARIABLES                                     = 75,
    CAUD_CUS2SE1_PARAM_FN026_INIT                                          = 76,
    CAUD_CUS2SE1_PARAM_FN026_VARIABLES                                     = 77,
    CAUD_CUS2SE1_PARAM_FN027_INIT                                          = 78,
    CAUD_CUS2SE1_PARAM_FN027_VARIABLES                                     = 79,
    CAUD_CUS2SE1_PARAM_FN028_VARIABLES                                     = 80,
    CAUD_CUS2SE1_PARAM_FN029_VARIABLES                                     = 81,

    //---MAX---
    CAUD_PATCH_PARAM_MAX = CAUD_PATCH_PARAM_LFE_MIXED_INTO_DMX_GAIN + 1,
} CAUD_PATCH_PARAM_TYPE;

typedef enum {
    CAUD_PATCH_INFO_TYPE_INVALID = -1,

    //--------------------------------------------------------------------------------------
    // (CAUD_"CUSTOMIZED_PATCH"_INFO_XXXXXXXX), INFO: Get
    //--------------------------------------------------------------------------------------
    //---SPDIF npcm buffer get wptr---
    CAUD_PATCH_INFO_SPDIF_NONPCM_WPTR           = 0,

    //---PCM Capture get wptr---
    CAUD_PATCH_INFO_PCM_CAPTURE1_WPTR           = 1,
    CAUD_PATCH_INFO_PCM_CAPTURE2_WPTR           = 2,

    //---ADEC_R2 UART info---
    CAUD_PATCH_INFO_R2_UART_BUF_BASE            = 3,
    CAUD_PATCH_INFO_R2_UART_BUF_SIZE            = 4,
    CAUD_PATCH_INFO_R2_UART_BUF_WPTR            = 5,

    //---ADEC PCM buffer info---
    CAUD_PATCH_INFO_ADEC_PCM_BUF_BASE           = 6,
    CAUD_PATCH_INFO_ADEC_PCM_BUF_SIZE           = 7,
    CAUD_PATCH_INFO_ADEC_PCM_BUF_WPTR           = 8,

    //---SPDIF NPCM buffer info---
    CAUD_PATCH_INFO_SPDIF_NPCM_BUF_BASE         = 22,
    CAUD_PATCH_INFO_SPDIF_NPCM_BUF_SIZE         = 23,
    CAUD_PATCH_INFO_SPDIF_NPCM_BUF_WPTR         = 24,

    //---HDMI NPCM buffer info---
    CAUD_PATCH_INFO_HDMI_NPCM_BUF_BASE          = 9,
    CAUD_PATCH_INFO_HDMI_NPCM_BUF_SIZE          = 10,
    CAUD_PATCH_INFO_HDMI_NPCM_BUF_RPTR          = 11,
    CAUD_PATCH_INFO_HDMI_NPCM_BUF_WPTR          = 12,

    //---MS12 META DATA buffer info---
    CAUD_PATCH_INFO_MS12_DDPE_METADATA_BUF_BASE = 13,
    CAUD_PATCH_INFO_MS12_DDPE_METADATA_BUF_SIZE = 14,
    CAUD_PATCH_INFO_MS12_DDPE_METADATA_BUF_WPTR = 15,

    //---ADEC ES buffer info---
    CAUD_PATCH_INFO_ADEC_ES_BUF_BASE            = 16,
    CAUD_PATCH_INFO_ADEC_ES_BUF_SIZE            = 17,
    CAUD_PATCH_INFO_ADEC_ES_BUF_RPTR            = 18,
    CAUD_PATCH_INFO_ADEC_ES_BUF_WPTR            = 19,

    //---SPDIF RX AC3 ES dump (in background)---
    CAUD_PATCH_INFO_SPDIF_RX_AC3_DUMP_BASE      = 26,
    CAUD_PATCH_INFO_SPDIF_RX_AC3_DUMP_SIZE      = 27,
    CAUD_PATCH_INFO_SPDIF_RX_AC3_DUMP_RPTR      = 29,

    //---HDMI RX ADEC AC3 ES Dump---
    CAUD_PATCH_INFO_ADEC_AC3_ES_DUMP_BASE       = 30,
    CAUD_PATCH_INFO_ADEC_AC3_ES_DUMP_SIZE       = 31,
    CAUD_PATCH_INFO_ADEC_AC3_ES_DUMP_PTR        = 32,

    //---Pipeline Delay---
    CAUD_PATCH_INFO_SPDIFTX_PIPELINE_DELAY      = 20,
    CAUD_PATCH_INFO_HDMITX_PIPELINE_DELAY       = 21,

    //---SW DMA1 info---
    CAUD_PATCH_INFO_SWDMA1_SAMPLE_PER_CH              = 28,
    CAUD_PATCH_INFO_SWDMA1_TOTAL_OUTPUT_SAMPLE_PER_CH = 80,
    CAUD_PATCH_INFO_SWDMA1_SAMPLES_OUTPUT             = 82,
    CAUD_PATCH_INFO_SWDMA1_ZEROES_OUTPUT              = 83,

    //---MCH SW DMA info---
    CAUD_PATCH_INFO_MCH_SWDMA_SAMPLE_PER_CH     = 25,
    CAUD_PATCH_INFO_MCH_SWDMA_SAMPLES_OUTPUT    = 84,
    CAUD_PATCH_INFO_MCH_SWDMA_ZEROES_OUTPUT     = 85,

    //---Frame decode pcm info---
    CAUD_PATCH_INFO_FRM_DECODE_PCM_SIZE         = 33,
    CAUD_PATCH_INFO_FRM_DECODE_BUF              = 34,
    CAUD_PATCH_INFO_FRM_DECODE_PCM_INFO         = 81,

    //---PCM capture for multichannel(5.1)
    CAUD_PATCH_INFO_PCM_CAPTURE_MCH             = 86, //<--MAX is here!!!

    //--------------------------------------------------------------------------------------
    // (CAUD_"CUS*SE*"_INFO_XXXXXXXX), PARAM: Set
    //--------------------------------------------------------------------------------------
    //---CUS2SE1---
    CAUD_CUS2SE1_INFO_FN_MODE_REGISTER                     = 35,
    CAUD_CUS2SE1_INFO_RESET                                = 36,
    CAUD_CUS2SE1_INFO_INIT_FLAG                            = 37,
    CAUD_CUS2SE1_INFO_FN_NUMBER_OF_OUTPUTS                 = 38,
    CAUD_CUS2SE1_INFO_FN_OUT_SAMPLING_FREQUENCY            = 39,
    CAUD_CUS2SE1_INFO_FNMAIN_INIT                          = 40,
    CAUD_CUS2SE1_INFO_FNMAIN_VARIABLES                     = 41,
    CAUD_CUS2SE1_INFO_FN000_INIT                           = 42,
    CAUD_CUS2SE1_INFO_FN000_VARIABLES                      = 43,
    CAUD_CUS2SE1_INFO_FN001_INIT                           = 44,
    CAUD_CUS2SE1_INFO_FN001_VARIABLES                      = 45,
    CAUD_CUS2SE1_INFO_FN004_MODE1_VARIABLES                = 46,
    CAUD_CUS2SE1_INFO_FN004_MODE2_VARIABLES                = 47,
    CAUD_CUS2SE1_INFO_FN008_INIT                           = 48,
    CAUD_CUS2SE1_INFO_FN008_VARIABLES                      = 49,
    CAUD_CUS2SE1_INFO_FN009_INIT                           = 50,
    CAUD_CUS2SE1_INFO_FN009_VARIABLES                      = 51,
    CAUD_CUS2SE1_INFO_FN010_INIT                           = 52,
    CAUD_CUS2SE1_INFO_FN010_VARIABLES                      = 53,
    CAUD_CUS2SE1_INFO_FN010_OUT                            = 54,
    CAUD_CUS2SE1_INFO_FN011_INIT                           = 55,
    CAUD_CUS2SE1_INFO_FN011_VARIABLES                      = 56,
    CAUD_CUS2SE1_INFO_FN014_INIT                           = 57,
    CAUD_CUS2SE1_INFO_FN014_VARIABLES                      = 58,
    CAUD_CUS2SE1_INFO_FN014_OUT                            = 59,
    CAUD_CUS2SE1_INFO_FN016_INIT                           = 60,
    CAUD_CUS2SE1_INFO_FN016_VARIABLES                      = 61,
    CAUD_CUS2SE1_INFO_FN017_VARIABLES                      = 62,
    CAUD_CUS2SE1_INFO_FN018_INIT                           = 63,
    CAUD_CUS2SE1_INFO_FN018_VARIABLES                      = 64,
    CAUD_CUS2SE1_INFO_FN019_INIT                           = 65,
    CAUD_CUS2SE1_INFO_FN019_VARIABLES                      = 66,
    CAUD_CUS2SE1_INFO_FN020_VARIABLES                      = 67,
    CAUD_CUS2SE1_INFO_FN022_INIT                           = 68,
    CAUD_CUS2SE1_INFO_FN022_VARIABLES                      = 69,
    CAUD_CUS2SE1_INFO_FN023_INIT                           = 70,
    CAUD_CUS2SE1_INFO_FN023_VARIABLES                      = 71,
    CAUD_CUS2SE1_INFO_FN024_INIT                           = 72,
    CAUD_CUS2SE1_INFO_FN024_VARIABLES                      = 73,
    CAUD_CUS2SE1_INFO_FN026_INIT                           = 74,
    CAUD_CUS2SE1_INFO_FN026_VARIABLES                      = 75,
    CAUD_CUS2SE1_INFO_FN027_INIT                           = 76,
    CAUD_CUS2SE1_INFO_FN027_VARIABLES                      = 77,
    CAUD_CUS2SE1_INFO_FN028_VARIABLES                      = 78,
    CAUD_CUS2SE1_INFO_FN029_VARIABLES                      = 79,

    //---MAX---
    CAUD_PATCH_INFO_TYPE_MAX = CAUD_PATCH_INFO_PCM_CAPTURE_MCH + 1,
} CAUD_PATCH_INFO_TYPE;

//---CUS2SE1---
typedef struct {
    MS_U32 u32Data;
    MS_U32 *pData;
    MS_U16 u16Data;
} caudCus2Se1Data_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Clip Play for ES */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
    CAUD_CODEC_TYPE clipCodec;
    MS_U32 repeatNum;       /* The play number of audio clip. */
} caudClipDecoder_t;

typedef enum {
    CAUD_CLIP_STATUS_INVALID  = -1,
    CAUD_CLIP_NONE            = 0,
    CAUD_CLIP_PLAY            = 1,
    CAUD_CLIP_STOP            = 2,
    CAUD_CLIP_RESUME          = 3,
    CAUD_CLIP_PAUSE           = 4,
    CAUD_CLIP_DONE            = 5,
} CAUD_CLIP_STATUS;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Clip Play for PCM */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_MIXER_IDX_INVALID = -1,
    CAUD_MIXER0             = 0,
    CAUD_MIXER1             = 1,
    CAUD_MIXER2             = 2,
    CAUD_MIXER3             = 3,
    CAUD_MIXER4             = 4,
    CAUD_MIXER5             = 5,
    CAUD_MIXER6             = 6,
    CAUD_MIXER7             = 7,
    CAUD_MIXER_IDX_MAX      = CAUD_MIXER7 + 1,
} CAUD_MIXER_IDX;

typedef enum {
    CAUD_PCM_ENDIAN_INVALID   = -1,
    CAUD_PCM_LITTLE_ENDIAN    = 0,
    CAUD_PCM_BIG_ENDIAN       = 1,
} CAUD_PCM_ENDIAN;

typedef enum {
    CAUD_PCM_SIGNED_STATUS_INVALID   = -1,
    CAUD_PCM_SIGNED           = 0,
    CAUD_PCM_UNSIGNED         = 1,
} CAUD_PCM_SIGNED_TYPE;

typedef struct {
    MS_U32               chNum;              /* 2  : stereo, 1 : mono,  8 : 8 channel */
    MS_U32               bitPerSample;       /* 16 : 16 bit, 8 : 8 bit 24 : 24bit */
    CAUD_SAMPLE_RATE     sampleRate;         /* 48000 : 48Khz, 44100 : 44.1Khz */
    CAUD_PCM_ENDIAN      endianType;         /* 0  : little endian, 1 : big endian */
    CAUD_PCM_SIGNED_TYPE signedType;         /* 0  : signed PCM, 1 : unsigned PCM */
    MS_U32               repeatNum;          /* The play number of audio clip. */
} caudClipMixer_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Direct Clip Play */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
    CAUD_CODEC_TYPE codec;
    MS_U32 chNum;
    MS_U32 bitPerSample;
    CAUD_SAMPLE_RATE sampleRate;
} caudDirect_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Gain, Mute & Delay */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_ADMIX_GAIN_IDX_INVALID = -1,
    CAUD_ADMIX_MD_GAIN      = 0,
    CAUD_ADMIX_AD_GAIN      = 1,
    CAUD_ADMIX_GAIN_IDX_MAX = CAUD_ADMIX_AD_GAIN + 1,
} CAUD_ADMIX_GAIN_IDX;

typedef enum {
    CAUD_LINEOUT_IDX_INVALID = -1,
    CAUD_LINEOUT0          = 0,
    CAUD_LINEOUT1          = 1,
    CAUD_LINEOUT2          = 2,
    CAUD_LINEOUT3          = 3,
    CAUD_LINEOUT_IDX_MAX   = CAUD_LINEOUT3 + 1,
} CAUD_LINEOUT_IDX;

//-------------------------------------------------------------------------------------------------------------------------------------
/* AENC */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_AENC_IDX_INVALID  = -1,
    CAUD_AENC0             = 0,
    CAUD_AENC1             = 1,
    CAUD_AENC_IDX_MAX      = CAUD_AENC1 + 1,
} CAUD_AENC_IDX;

typedef enum {
    CAUD_AENC_STATUS_INVALID = -1,
    CAUD_AENC_STOP         = 0,
    CAUD_AENC_PLAY         = 1,
    CAUD_AENC_ABNORMAL     = 2,
} CAUD_AENC_STATUS;

typedef enum {
    CAUD_AENC_FORMAT_INVALID = -1,
    CAUD_AENC_MP3              = 0,    /* Encode MP3 format */
    CAUD_AENC_AAC              = 1,    /* Encode AAC format */
} CAUD_AENC_FORMAT;

typedef enum {
    CAUD_AENC_CH_INVALID = -1,
    CAUD_AENC_MONO       = 0,
    CAUD_AENC_STEREO     = 1,
} CAUD_AENC_CH_NUM;

typedef enum {
    CAUD_AENC_BITRATE_INVALID = -1,
    CAUD_AENC_BIT_48K         = 0,
    CAUD_AENC_BIT_56K         = 1,
    CAUD_AENC_BIT_64K         = 2,
    CAUD_AENC_BIT_80K         = 3,
    CAUD_AENC_BIT_112K        = 4,
    CAUD_AENC_BIT_128K        = 5,
    CAUD_AENC_BIT_160K        = 6,
    CAUD_AENC_BIT_192K        = 7,
    CAUD_AENC_BIT_224K        = 8,
    CAUD_AENC_BIT_256K        = 9,
    CAUD_AENC_BIT_320K        = 10,
} CAUD_AENC_BITRATE;

typedef struct {
    //Get Info for debugging on DDI
    CAUD_AENC_STATUS   status;   // current ENC Status
    CAUD_AENC_FORMAT   codec;    // current ENC Codec

    MS_U32  errCnt;       // current ENC error counter
    MS_U32  inputCnt;     // current ENC input counter - we distinguish whether or not enter data from input.
    MS_U32  underflowCnt; // current ENC underflowCnt in kernel space - we distinguish which module have problem between muxer and encdoer
    MS_U32  overflowCnt;  // current ENC overflowCnt - we distinguish

    //Set Info - it is applied realtime, no matter stop&start
    CAUD_AENC_CH_NUM    chNum;       // number of chNum
    CAUD_AENC_BITRATE   bitrate;     // bitrate
} caudAenc_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* PCM(Sound Bar Buletooth, PCM Capture) */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
    CAUD_PCM_CAPTURE_IDX device;
    MS_U32 targetLvl;
} caudPcmCapture_t;

typedef struct {
    MS_U32 base;
    MS_U32 size;
    MS_U32 wrPtr;
} caudPcmBuf_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* PCM IO */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MS_U8 connectFlag;      /* it is TRUE when opened */
    MS_U8 startFlag;        /* it is TRUE when started and opened */
    MS_U8 name[32];         /* The PCM name requested */
    MS_U8 nonBlockingFlag;  /* it is TRUE when request for nonblocking mode */
    MS_U8 multiChFlag;      /* it is TRUE when request for a multi-chNum PCM IO */
    MS_U8 mixingFlag;       /* it is TRUE when request for a mixing PCM IO */
    MS_U32 mixingGroup;     /* PCM mixing group, i.e. AUDIO_PCMMIXER_GROUP_ALL */
    MS_U32 bufDuration;     /* PCM buffer size in mini-second unit, and up to a maximun size */
    MS_U32 chNum;           /* PCM chNum number */
    MS_U32 sampleRate;      /* PCM sample rate */
    MS_U32 bitWidth;        /* PCM sample's bit width of each chNum */
    MS_U32 bigEndian;       /* PCM endian, TRUE means Big Endian, FALSE means Little Endian */
    MS_U32 timestamp;       /* PCM timestamp */
    MS_U32 weighting;       /* PCM weighting for mixing case, differs from volume it's an extra attenuation  for PCM level, the range is 0~100, and 100 means no any attenuation */
    MS_U32 vol;             /* PCM volume, the range is 0 ~ 1016, and 96 means 0dB */
    MS_U32 bufLvl;          /* PCM bufffer level in byte unit */
    MS_U8 captureFlag;      /* it is TRUE when request for a PCM Capture IO  */
    MS_U8 delayFlag;
} caudPcmio_t;

typedef enum {
    CAUD_PCM_CMD_ALL,             /*       GET */
    CAUD_PCM_CMD_NONBLOCKING,     /* SET / GET */
    CAUD_PCM_CMD_MCH,             /*       GET */
    CAUD_PCM_CMD_MIXING,          /*       GET */
    CAUD_PCM_CMD_MIXING_GROUP,    /* SET / GET */
    CAUD_PCM_CMD_BUF,             /*       GET */
    CAUD_PCM_CMD_BUF_DURATION,    /* SET / GET */
    CAUD_PCM_CMD_RPTR,            /*       GET */
    CAUD_PCM_CMD_WPTR,            /*       GET */
    CAUD_PCM_CMD_CH,              /* SET / GET */
    CAUD_PCM_CMD_SAMPLERATE,      /* SET / GET */
    CAUD_PCM_CMD_BITWIDTH,        /* SET / GET */
    CAUD_PCM_CMD_BIGENDIAN,       /* SET / GET */
    CAUD_PCM_CMD_TIMESTAMP,       /* SET / GET */
    CAUD_PCM_CMD_WEIGHTING,       /* SET / GET */
    CAUD_PCM_CMD_VOL,             /* SET / GET */
    CAUD_PCM_CMD_BUF_LVL,         /*       GET */
    CAUD_PCM_CMD_MUTE,            /* SET / GET */
    CAUD_PCM_CMD_BUF_SIZE,        /*       GET */
    CAUD_PCM_CMD_DATAPORT,        /* SET       */
    CAUD_PCM_CMD_AUTORUN,         /* SET / GET */
    CAUD_PCM_CMD_VOL_DB,          /* SET / GET */
    CAUD_PCM_CMD_EASING_TYPE,     /* SET / GET */
    CAUD_PCM_CMD_EASING_DURATION, /* SET / GET */
    CAUD_PCM_CMD_EASING_GAIN      /* SET / GET */
} CAUD_PCM_CMD;

#define CAUD_PCM_SWMIXER_GROUP_NONE   0x00000000
#define CAUD_PCM_SWMIXER_GROUP_ALL    0xFFFFFFFF
#define CAUD_PCM_SWMIXER_GROUP_1      0x00000001
#define CAUD_PCM_SWMIXER_GROUP_2      0x00000002
#define CAUD_PCM_SWMIXER_GROUP_3      0x00000004
#define CAUD_PCM_SWMIXER_GROUP_4      0x00000008
#define CAUD_PCM_SWMIXER_GROUP_5      0x00000010
#define CAUD_PCM_SWMIXER_GROUP_6      0x00000020
#define CAUD_PCM_SWMIXER_GROUP_7      0x00000040
#define CAUD_PCM_SWMIXER_GROUP_8      0x00000080

//-------------------------------------------------------------------------------------------------------------------------------------
/* MM New Mode */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_MEMORY_BASE = 0x0, ///< for audio DDR Base
    CAUD_MEMORY_SIZE = 0x1, ///< for audio DDR Size
} CAUD_DDR_INFO;

typedef struct {
    MS_U32 baseAddr;   ///<  base address of ES buffer
    MS_U32 endAddr;    ///<  end address of ES buffer
    MS_U32 bufSize;    ///<  ES buffer size
    MS_U32 wrPtr;      ///<  write pointer
    MS_U32 rdPtr;      ///<  read pointer
    MS_U32 freeSpace;  ///<  free space of ES buffer
    MS_U32 lvl;        ///<  level of ES buffer
    MS_U32 ptsCnt;     ///<  count of total received PTS
    MS_U64 lastPts;    ///<  last received PTS
} caudAes_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Basic Sound Effect */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef enum {
    CAUD_GEQ,
    CAUD_PEQ,
    CAUD_HPF,
    CAUD_DRC,
    CAUD_AVC,
    CAUD_BALANCE,
    CAUD_BASIC_SE_MAX,
} CAUD_BASIC_SE_TYPE;

typedef struct {
    MS_U8    u8band;       //0~12
    MS_BOOL  bEnable;      //0,1
    MS_U8    u8Type;       //0~4,  0:peak, 1:high pass, 2:low pass, 3:high shelving, 4:low shelving
    MS_U16   u16Gain;      //0~360, -18db~+18db, step 0.1dB
    MS_U16   u16Frequency; //20~20000(Hz) for fc
    MS_U16   u16QValue;    //5~160
} caudBasicSePeqCoef_t;

typedef enum {
    CAUD_AVC_LINEAR,
    CAUD_AVC_SHIFT,
    CAUD_AVC_NORMAL,
    CAUD_AVC_R,
} CAUD_BASIC_SE_AVC_MODE;

typedef struct {
    MS_U32   u32TargetLevel;   //0x1000~0x7FFFFF (-48~18 dB)(Conversion formula: 10^(gain_db/20)*0x100000)
    MS_U32   u32AttackStepDb;  //0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
    MS_U32   u32ReleaseStepDb; //0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
} caudBasicSeAvcParam_t;

typedef struct {
    MS_U8    u8Band;           //0: BAND_LOW, 2: BAND_HIGH, 3: BAND_ALL
    MS_U32   u32TargetLevel;   //0x1000~0x7FFFFF (-48~18 dB)(Conversion formula: 10^(gain_db/20)*0x100000)
    MS_U32   u32AttackStepDb;  //0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
    MS_U32   u32ReleaseStepDb; //0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
    MS_U8    u8ProcMode;       //0,1
} caudBasicSeDrcParam_t;

//-------------------------------------------------------------------------------------------------------------------------------------
/* Advanced Sound Effect */
//-------------------------------------------------------------------------------------------------------------------------------------

                                                                                        // Description:
typedef enum                                                                            // Enable SRS Main function, Only when the hashkey is correct,
{                                                                                       // the SRS main function will be activated sucessfully.
    CAUD_DTSSE_TYPE_INVALID      = -1,                                                  // ============================================================================
    CAUD_DTSSE_TSXT              = 0,                                                   // for SRS TSXT Hashkey
    CAUD_DTSSE_TSHD              = 1,                                                   // for SRS TSHD Hashkey
    CAUD_DTSSE_THEATERSOUND      = 2,                                                   // for SRS TheaterSound HD Hashkey
    CAUD_DTSSE_PURESND           = 3,                                                   // for SRS Pure Sound Hashkey or for DTS StudioSound 3D Hashkey.
    CAUD_DTSSE_STUDIOSOUND_3D    = 4,                                                   // for DTS StudioSound 3D Hashkey
    CAUD_DTSSE_DISABLE           = 5, /*<-MAX*/                                         // for DISABLE all DTS SE
    CAUD_DTSSE_TYPE_MAX          = CAUD_DTSSE_DISABLE + 1,
} CAUD_DTSSE_TYPE;

                                                                                        //  ____________________________________________________________________________________________________________
                                                                                        // |Hashkey:                                                 |Description:
                                                                                        // |_____________________________________ ___________________|__________________________________________________
typedef enum                                                                            // |SRS_TSXT|SRS_TSHD|SRS_TheaterSound_HD|SRS_Puresound_Only |Enable/Disable
{                                                                                       // |        |        |DTS_StudioSound_3D |(SE-DSP)           |for the following:
    CAUD_DTSSE_UNIT_INVALID                                     = -1,                   // |________|________|___________________|___________________|
                                                                                        // |           TSHD                                          |
                                                                                        // |________ ________ ___________________ ___________________|__________________________________________________
    CAUD_DTSSE_THEATERSOUND_TOTAL                               = 0,                    // |   V    |   V    |        V          |                   |DTS StudioSound main function
    CAUD_DTSSE_THEATERSOUND_TSHD                                = 1,                    // |   V    |   V    |        V          |                   |TruSurround HD
    CAUD_DTSSE_THEATERSOUND_DEFINITION                          = 2,                    // |        |   V    |        V          |                   |Definition processing component of TruSurround HD
    CAUD_DTSSE_THEATERSOUND_DC                                  = 3,                    // |   V    |   V    |        V          |                   |Dialog Clarity processing component of TruSurround HD
    CAUD_DTSSE_THEATERSOUND_TSHD_SURR                           = 4,                    // |   V    |   V    |        V          |                   |Surround processing component of TruSurround HD
    CAUD_DTSSE_THEATERSOUND_TRUBASS                             = 5,                    // |   V    |   V    |        V          |                   |TruBass processing component of TruSurround HD
    CAUD_DTSSE_THEATERSOUND_TSHD_LVL_INDPT                      = 6,                    // |   V    |   V    |        V          |                   |Level Independent processing component of TruBass
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |     CC3D Controls (CC3D must enable)                    |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_CC3D_EN                             = 7,                    // |   V    |   V    |        V          |                   |CC3D processing
    CAUD_DTSSE_THEATERSOUND_CC3D_DEPTH_PROCESS_EN               = 8,                    // |        |        |        V          |                   |Depth processing in CC3D
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |       TRUVOLUME HD                                      |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_TRUVOLUME                           = 9,                    // |        |        |        V          |                   |TruVolume processing
    CAUD_DTSSE_THEATERSOUND_TRUVOLUME_NORMALIZER                = 10,                   // |        |        |        V          |                   |it will detect the level of the incoming signal and if necessary, dynamically boosts the signal level to ensure that it is always within the operational range of TruVolume.
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |         TRUDIALOG                                       |
                                                                                        // |_____________________________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_TRUDIALOG                           = 11,                   // |        |        |        V          |                   |TruDialog processing
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |           TBHDX                                         |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_EN                       = 12,                   // |        |        |        V          |                   |TruBass HDX low frequency compensation processing
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |            GEQ                                          |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_GEQ_EN                              = 13,                   // |        |        |        V          |                   |Graphic EQ processing
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |    PURESOUND HRADLIMITER                                |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_HL                                     = 14,                   // |        |   V    |        V          |        V          |HardLimiter processing
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |       PURESOUND AEQ                                     |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_AEQ                                    = 15,                   // |        |        |        V          |        V          |Aective EQ processing
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |       PURESOUND HPF                                     |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_HPF                                    = 16,                   // |        |        |                   |                   |High Pass Filter processing
                                                                                        // |________|________|___________________|___________________|
                                                                                        // |       PURESOUND TBHD                                    |
                                                                                        // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_TBHD                                   = 17, /*<-MAX*/         // |        |        |                   |        V          |TruBass in SE-DSP
                                                                                        // |________|________|___________________|___________________|

    CAUD_DTSSE_UNIT_MAX = CAUD_DTSSE_PURESOUND_TBHD + 1,
} CAUD_DTSSE_UNIT;

                                                                                       //  _________________________________________________________
                                                                                       // |Hashkey:                                                 |
                                                                                       // |________ ________ ___________________ ___________________|__________________________________________________________________________________________________________
typedef enum                                                                           // |SRS_TSXT|SRS_TSHD|SRS_TheaterSound_HD|SRS_Puresound_Only |
{                                                                                      // |        |        |DTS_StudioSound_3D |(SE-DSP)           |Default               Range                               Description
    CAUD_DTSSE_PARAM_INVALID                                       = -1,               // |________|________|___________________|___________________|__________________________________________________________________________________________________________
                                                                                       // |           TSHD                                          |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_INPUT_GAIN                             = 0,                // |   V    |   V    |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trusurround input gain. Input Gain. Adjusts the level of the signal at the input to TSHD processing.
    CAUD_DTSSE_THEATERSOUND_OUTPUT_GAIN                            = 50,               // |   V    |   V    |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trusurround output gain. Output Gain. Adjusts the level of the signal at the output to TSHD processing.
    CAUD_DTSSE_THEATERSOUND_DEFINITION_CTRL                        = 1,                // |        |   V    |        V          |                   |0.6                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trusurround definition control. Definition Level. Controls the level of high frequency enhancement.
    CAUD_DTSSE_THEATERSOUND_DC_CTRL                                = 2,                // |   V    |   V    |        V          |                   |0.5                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trusurround dialog clarity control. Dialog Clarity Level. Establishes the amount of dialog clarity enhancement that is applied to the audio signal.
    CAUD_DTSSE_THEATERSOUND_SURR_LVL_CTRL                          = 3,                // |   V    |   V    |        V          |                   |0.6                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trusurround surround level. Surround Level. Controls the overall mix level of the surround channels.
    CAUD_DTSSE_THEATERSOUND_TRUBASS_CTRL                           = 4,                // |   V    |   V    |        V          |                   |0.3                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trubass HD front level control
    CAUD_DTSSE_THEATERSOUND_TRUBASS_COMPRESSOR_CTRL                = 5,                // |   V    |   V    |        V          |                   |0.25                  0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Trubass HD compressor control
    CAUD_DTSSE_THEATERSOUND_TRUBASS_PROCESS_MODE                   = 6,                // |   V    |   V    |        V          |                   |1:stereo              0:mono  1:stereo                    Trubass HD process mode
    CAUD_DTSSE_THEATERSOUND_SPEAKERSIZE                            = 7,                // |   V    |   V    |        V          |                   |8:Custom              0 ~ 8                               Trubass HD speaker size. 0:40Hz, 1:60Hz, 2:100Hz, 3:150Hz, 4:200Hz, 5:250Hz, 6:300Hz, 7:400Hz, 8:Custom (default), Trubass HD speaker size
    CAUD_DTSSE_THEATERSOUND_TRUBASS_SPEAKER_AUDIO                  = 8,                // |   V    |   V    |        V          |                   |13:150Hz              0 ~ 38 (20Hz ~ 400 Hz, step:10Hz)   Trubass HD customer audio speaker size
    CAUD_DTSSE_THEATERSOUND_TRUBASS_SPEAKER_ANALYSIS               = 9,                // |   V    |   V    |        V          |                   |10:120Hz              0 ~ 38 (20Hz ~ 400 Hz, step:10Hz)   Trubass HD customer analysis frequency
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |       Main Controls                                     |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_HEADROOM_GAIN                          = 10,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Headroom Gain. Provides headroom for intermediate calculations so as to avoid clipping.The value is typically set to the inverse of the setting for the Boost Gain control of the HardLimiter. Note that the value of this control must differ for TVOL on versus off.
    CAUD_DTSSE_THEATERSOUND_BYPASS_GAIN                            = 11,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Bypass Gain. Adjusts the signal level when TheaterSound HD/DTS StudioSound 3D processing is turned off.
    CAUD_DTSSE_THEATERSOUND_CC3D_PROCESS_PATH                      = 12,               // |        |        |        V          |                   |2:PATH_1              0 ~ 2                               Processing Path. Selects the processing path: PATH_NONE:NONE, PATH_0:csdecoder + trudiag + tshd, PATH_1:trudiag + cc3d (depth + stereo tshd). 0:SRS_TS3D_PROCESSINGPATH_NONE, 1:SRS_TS3D_PROCESSINGPATH_0; 2:SRS_TS3D_PROCESSINGPATH_1.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |       CC3D Controls                                     |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_CC3D_INPUT_GAIN                        = 13,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Input Gain. Adjusts the level of the signal at the input to CC3D processing.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |       TRUVOLUME HD                                      |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_TRUVOLUME_MODE                         = 14,               // |        |        |        V          |                   |1:SRS_TVOLHD_NORM     0 ~ 1                               TruVolume HD Mode. Selects the mode of operation for TruVolume HD.There are two modes supported:Light and Normal.The two modes represent different tunings of the algorithm from lighter to more aggressive. 0:SRS_TVOLHD_LIGHT, 1:SRS_TVOLHD_NORM.
    CAUD_DTSSE_THEATERSOUND_TRUVOLUME_REF_LVL                      = 15,               // |        |        |        V          |                   |0.125                 0x2C04 ~ 0x7FFFFF (1.0e-3 ~ 1.0)    LKFS. Sets the desired absolute loudness level of the signal
    CAUD_DTSSE_THEATERSOUND_TRUVOLUME_MAX_GAIN                     = 16,               // |        |        |        V          |                   |2.0                   0x1FFFFF ~ 0xFFFFFE0 (0.25 ~ 32.00) Max Gain. Controls the maximum amount that the signal may be increased.
    CAUD_DTSSE_THEATERSOUND_TRUVOLUME_NORMALIZE_THRESH             = 17,               // |        |        |        V          |                   |0.25                  0x346 ~ 0x3FFFFF (1.0e-4 ~ 0.5)     Norm Threshold. Determines if the gain boost value is greater than 1 or not.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |         TRUDIALOG                                       |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_TRUDIALOG_INPUT_GAIN                   = 18,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Input Gain. Adjusts the level of the signal at the input.
    CAUD_DTSSE_THEATERSOUND_TRUDIALOG_CLARITY_GAIN                 = 19,               // |        |        |        V          |                   |0.5                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Clarity Gain. Adjusts the amount of vocal enhancement that is applied to the audio signal.
    CAUD_DTSSE_THEATERSOUND_TRUDIALOG_PROCESS_GAIN                 = 20,               // |        |        |        V          |                   |2.82                  0x3FFFFF ~ 0x17FFFFD (0.5 ~ 3.0)    Process Gain. Controls how much of the calculated final outout gain is applied to the output signal, effectively controlling how much the process will raise the dialog signal.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |          TBHDX                                          |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_INPUT_GAIN                  = 21,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Input Gain. Adjusts the amount of vocal enhancement that is applied to the audio signal.
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_BASSLEVEL                   = 22,               // |        |        |        V          |                   |0.333521432           0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Bass Level. Determines the level of psychoacoustic bass enhancement.
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_SPEAKERSIZE                 = 23,               // |        |        |        V          |                   |2:100Hz               0 ~ 10                              Speaker Size. The range of low frequency limitations in the speakers that TruBass HDX is compensating for. 0:40Hz, 1:60Hz, 2:100Hz, 3:120HZ, 4:150Hz, 5:200Hz, 6:250Hz, 7:300Hz, 8:400Hz, 9:500Hz, 10:600Hz.
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_HP_ORDER                    = 24,               // |        |        |        V          |                   |8                     1 ~ 8                               High Pass Order. The slope of the filter that removes the low frequencies that cannot be replicated by the speaker is adjusted with this control.
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_DYNAMICS                    = 25,               // |        |        |        V          |                   |0.3                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Dynamics. Modifies the amount of dynamics processing applied to the signal.The ratio/range of the compressor is dynamically adjusted as the incoming signal is monitored.
    CAUD_DTSSE_THEATERSOUND_CC3D_TBHDX_MODE                        = 26,               // |        |        |        V          |                   |1                     0:MONO 1:STEREO                     Mode. Determines if the left and right signals are summed and processed then sent to both channels or if the left and right channels are processed by TruBass HDX separetely. 0:SRS_TBHDX_MODE_MONO, 1:SRS_TBHDX_MODE_STEREO
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |        CS DECODER                                       |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_CS_INPUT_GAIN                          = 27,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Input Gain. Adjusts the level of the signal at the input to CS Decoder.
    CAUD_DTSSE_THEATERSOUND_CS_PROCESS_MODE                        = 28,               // |        |        |        V          |                   |1:music               0:cinema, 1:music                   Mode. Specifies the format of the source material.
    CAUD_DTSSE_THEATERSOUND_CS_LR_OUTPUT_GAIN                      = 29,               // |        |        |        V          |                   |2.0                   0xB645A ~ 0x1FFFFFC (0.089 ~ 4.0)   L/R Output Gain. Controls the output level of the front channel of CS Decoder.
    CAUD_DTSSE_THEATERSOUND_CS_LsRs_OUTPUT_GAIN                    = 30,               // |        |        |        V          |                   |1.0                   0xB645A ~ 0x1FFFFFC (0.089 ~ 4.0)   Ls/Rs Output Gain. Controls the output level of the surround channel of CS Decoder.
    CAUD_DTSSE_THEATERSOUND_CS_Center_OUTPUT_GAIN                  = 31,               // |        |        |        V          |                   |2.0                   0xB645A ~ 0x1FFFFFC (0.089 ~ 4.0)   Center Output Gain. Controls the output level of the center channel of CS Decoder.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |            GEQ                                          |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_THEATERSOUND_GEQ_INPUT_GAIN                         = 32,               // |        |        |        V          |                   |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          Input Gain. Adjusts the level of the signal at the input to Graphic EQ processing.
    CAUD_DTSSE_THEATERSOUND_GEQ_BAND0_GAIN                         = 33,               // |        |        |        V          |                   |SRS_GEQ_PLUS_05DB     0x16A7EF ~ 0x2D0A3D1(0.177 ~ 5.63)  Band Gain[5]. Adjusts the gain for each band.A different control is available for each band.
    CAUD_DTSSE_THEATERSOUND_GEQ_BAND1_GAIN                         = 34,               // |        |        |        V          |                   |SRS_GEQ_PLUS_03DB     0x16A7EF ~ 0x2D0A3D1(0.177 ~ 5.63)  SRS_GEQ_MINUS_15DB:0.177, SRS_GEQ_MINUS_11DB:0.282, SRS_GEQ_MINUS_07DB:0.45, SRS_GEQ_MINUS_03DB:0.71, SRS_GEQ_PLUS_01DB:1.12, SRS_GEQ_PLUS_05DB:1.78, SRS_GEQ_PLUS_09DB 2.82, SRS_GEQ_PLUS_13DB:4.47
    CAUD_DTSSE_THEATERSOUND_GEQ_BAND2_GAIN                         = 35,               // |        |        |        V          |                   |SRS_GEQ_0DB           0x16A7EF ~ 0x2D0A3D1(0.177 ~ 5.63)  SRS_GEQ_MINUS_14DB:0.20 , SRS_GEQ_MINUS_10DB:0.316, SRS_GEQ_MINUS_06DB:0.50, SRS_GEQ_MINUS_02DB:0.79, SRS_GEQ_PLUS_02DB:1.26, SRS_GEQ_PLUS_06DB:2.00, SRS_GEQ_PLUS_10DB 3.16, SRS_GEQ_PLUS_14DB:5.01
    CAUD_DTSSE_THEATERSOUND_GEQ_BAND3_GAIN                         = 36,               // |        |        |        V          |                   |SRS_GEQ_PLUS_03DB     0x16A7EF ~ 0x2D0A3D1(0.177 ~ 5.63)  SRS_GEQ_MINUS_13DB:0.224, SRS_GEQ_MINUS_09DB:0.355, SRS_GEQ_MINUS_05DB:0.56, SRS_GEQ_MINUS_01DB:0.89, SRS_GEQ_PLUS_03DB:1.41, SRS_GEQ_PLUS_07DB:2.24, SRS_GEQ_PLUS_11DB 3.55, SRS_GEQ_PLUS_15DB:5.63
    CAUD_DTSSE_THEATERSOUND_GEQ_BAND4_GAIN                         = 37,               // |        |        |        V          |                   |SRS_GEQ_PLUS_05DB     0x16A7EF ~ 0x2D0A3D1(0.177 ~ 5.63)  SRS_GEQ_MINUS_12DB:0.251, SRS_GEQ_MINUS_08DB:0.40 , SRS_GEQ_MINUS_04DB:0.63, SRS_GEQ_0DB:1.00       , SRS_GEQ_PLUS_04DB:1.58, SRS_GEQ_PLUS_08DB:2.51, SRS_GEQ_PLUS_12DB 4.00,
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |     PURESOUND HRADLIMITER                               |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_HL_INPUT_GAIN                             = 38,               // |        |   V    |        V          |        V          |1.0                   0x0 ~ 0x1FFFFFC (0.0 ~ 4.0)         HardLimiter Input Gain. Adjusts the value used for adjusting the audio level after the HardLimiter process
    CAUD_DTSSE_PURESOUND_HL_OUTPUT_GAIN                            = 39,               // |        |   V    |        V          |        V          |1.0                   0x0 ~ 0x1FFFFFC (0.0 ~ 4.0)         HardLimiter Output Gain. Adjusts the value used for adjusting the audio level before the HardLimiter process.
    CAUD_DTSSE_PURESOUND_HL_BYPASS_GAIN                            = 40,               // |        |   V    |        V          |        V          |1.0                   0x0 ~ 0x7FFFFF  (0.0 ~ 1.0)         HardLimiter Bypass Gain. Adjusts the value used for adjusting the audio level when disable the HardLimiter process.
    CAUD_DTSSE_PURESOUND_HL_LIMITERBOOST                           = 41,               // |        |   V    |        V          |        V          |2.5                   0x7FFFFF ~ 0xFFFFFE0 (1.0 ~ 32.0)   (20*log(v/32))dB, v is 1.0 ~ 32.0. so 32.0 is 0dB (MAX). HardLimiter BoostGain. Modifies the signal level within the context of the HardLimiter, so that the signal will not exceed the Hard Limit Level parameter
    CAUD_DTSSE_PURESOUND_HL_HARDLIMIT                              = 42,               // |        |   V    |        V          |        V          |1.0                   0x0 ~ 0x7FFFFF  (0.0 ~ 1.0)         HardLimiter limit level.
    CAUD_DTSSE_PURESOUND_HL_DELAYLEN                               = 43,               // |        |   V    |        V          |        V          |14                    6 ~ 48                              HardLimiter Delay Length. Specifies the length of the look-ahead delay line utilized in the HardLimiter algorithm.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |       PURESOUND AEQ                                     |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_AEQ_INPUT_GAIN                            = 44,               // |        |        |        V          |        V          |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          AEQ Input Gain. Adjusts the value used for adjusting the audio level after the AEQ process.
    CAUD_DTSSE_PURESOUND_AEQ_OUTPUT_GAIN                           = 45,               // |        |        |        V          |        V          |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          AEQ Output Gain. Adjusts the value used for adjusting the audio level before the AEQ process.
    CAUD_DTSSE_PURESOUND_AEQ_BYPASS_GAIN                           = 46,               // |        |        |        V          |        V          |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)          AEQ Bypass Gain. Adjusts the value used for adjusting the audio level when disable the AEQ process.
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |       PURESOUND HPF                                     |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_HPF_FREQUENCY                             = 47,               // |        |        |        V          |        V          |1                     0 ~ 3                               HPF Frequency. 0:60Hz, 1:80Hz, 2:100Hz, 3:120Hz
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |      PURESOUND Input/output gain                        |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_INPUT_GAIN                                = 48,               // |        |        |                   |        V          |                      0x0 ~ 0x7FFFFF (0.0 ~ 1.0)
    CAUD_DTSSE_PURESOUND_OUTPUT_GAIN                               = 49,               // |        |        |                   |        V          |                      0x0 ~ 0x7FFFFF (0.0 ~ 1.0)
                                                                                       // |________|________|___________________|___________________|
                                                                                       // |      PURESOUND TBHD                                     |
                                                                                       // |________ ________ ___________________ ___________________|
    CAUD_DTSSE_PURESOUND_TBHD_TRUBASS_LVL                          = 51,               // |        |        |                   |        V          |1.0                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)
    CAUD_DTSSE_PURESOUND_TBHD_LVL_INDEPENDENT_EN                   = 52,               // |        |        |                   |        V          |1                     0 ~ 1
    CAUD_DTSSE_PURESOUND_TBHD_COMPRESSOR_LVL                       = 53,               // |        |        |                   |        V          |0.8                   0x0 ~ 0x7FFFFF (0.0 ~ 1.0)
    CAUD_DTSSE_PURESOUND_TBHD_SPEAKER_AUDIO                        = 54,               // |        |        |                   |        V          |0                     0 ~ 8                               0:40Hz, 1:60Hz, 2:100Hz, 3:120Hz, 4:150Hz, 5:200Hz, 6:250Hz, 7:300Hz, 8:400Hz
    CAUD_DTSSE_PURESOUND_TBHD_SPEAKER_ANALYSIS                     = 55, /*<-MAX*/     // |        |        |                   |        V          |1                     0 ~ 8                               0:40Hz, 1:60Hz, 2:100Hz, 3:120Hz, 4:150Hz, 5:200Hz, 6:250Hz, 7:300Hz, 8:400Hz
                                                                                       // |________|________|___________________|___________________|

    CAUD_DTSSE_PARAM_MAX = CAUD_DTSSE_PURESOUND_TBHD_SPEAKER_ANALYSIS + 1,
} CAUD_DTSSE_PARAM;

typedef struct {
    MS_U32 iirBandNum;
    MS_S32 scrambleIirCoefs[50];
    MS_U32 iirOrder;
    MS_U32 firTapNum;
    MS_S32 scrambleFirCoefs[67];
    MS_U32 firOrder;
} caudDtsSeAeqCoef_t;


//For Dolby DAP's parameter setting structure.
#define DOLBY_DAP_MAX_BANDS          (20)
#define DOLBY_DAP_IEQ_MAX_BANDS      DOLBY_DAP_MAX_BANDS
#define DOLBY_DAP_GEQ_MAX_BANDS      DOLBY_DAP_MAX_BANDS
#define DOLBY_DAP_REG_MAX_BANDS      DOLBY_DAP_MAX_BANDS
#define DOLBY_DAP_OPT_MAX_BANDS      DOLBY_DAP_MAX_BANDS
#define DOLBY_DAP_MAX_CHANNELS       6
#define DOLBY_DAP_BIN_PARAM_MAX_SIZE (16*1024)

                                                                            //  ________________________________________________________
                                                                            // |
                                                                            // |Range               Default
                                                                            // |________________________________________________________
typedef struct                                                              // |
{                                                                           // |
    int pregain;                                                            // |-2080 ~ 480         0
    int postgain;                                                           // |-2080 ~ 480         0
    int systemgain;                                                         // |-2080 ~ 480         0
    int surround_decoder_enable;                                            // |    0 ~ 1           1
    int virtualizer_enable;                                                 // |    0 ~ 1           1
    int headphone_reverb;                                                   // |-2080 ~ 192         0
    int speaker_angle;                                                      // |    0 ~ 27          5
    int speaker_start;                                                      // |   20 ~ 20000       200
    int surround_boost;                                                     // |    0 ~ 96          96
    int mi_ieq_enable;                                                      // |    0 ~ 1           0
    int mi_dv_enable;                                                       // |    0 ~ 1           0
    int mi_de_enable;                                                       // |    0 ~ 1           0
    int mi_surround_enable;                                                 // |    0 ~ 1           0
                                                                            // |
    int calibration_boost;                                                  // |    0 ~ 192         0
                                                                            // |
    int leveler_amount;                                                     // |    0 ~ 10          7
    int leveler_input;                                                      // | -640 ~ 0           -496
    int leveler_output;                                                     // | -640 ~ 0           -496
    int leveler_enable;                                                     // |    0 ~ 1           0
                                                                            // |
    int modeler_enable;                                                     // |    0 ~ 1           0
    int modeler_calibration;                                                // | -320 ~ 320         0
                                                                            // |
    int ieq_enable;                                                         // |    0 ~ 1           0
    int ieq_amount;                                                         // |    0 ~ 16          10
    int ieq_nb_bands;                                                       // |    1 ~ 20          20
    int a_ieq_band_center[DOLBY_DAP_IEQ_MAX_BANDS];                         // |   20 ~ 20000       {65,  136, 223, 332, 467, 634, 841, 1098, 1416, 1812, 2302, 2909, 3663, 4598, 5756, 7194, 8976, 11186, 13927, 17326}
    int a_ieq_band_target[DOLBY_DAP_IEQ_MAX_BANDS];                         // | -480 ~ 480         {117, 133, 188, 176, 141, 149, 175, 185,  185,  200,  236,  242,  228,  213,  182,  132,  110,  68,    -27,   -240}
                                                                            // |
    int de_enable;                                                          // |    0 ~ 1           1
    int de_amount;                                                          // |    0 ~ 16          9
    int de_ducking;                                                         // |    0 ~ 16          0
                                                                            // |
    int volmax_boost;                                                       // |    0 ~ 192
                                                                            // |
    int geq_enable;                                                         // |    0 ~ 1           0
    int geq_nb_bands;                                                       // |    1 ~ 20          0
    int a_geq_band_center[DOLBY_DAP_GEQ_MAX_BANDS];                         // |   20 ~ 20000       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    int a_geq_band_target[DOLBY_DAP_GEQ_MAX_BANDS];                         // | -576 ~ 576         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
                                                                            // |
    int optimizer_enable;                                                   // |    0 ~ 1           1
    int optimizer_nb_bands;                                                 // |    1 ~ 20          20
    int a_opt_band_center_freq[DOLBY_DAP_OPT_MAX_BANDS];                    // |   20 ~ 20000       {47, 141, 234, 328, 469, 656, 844, 1031, 1313, 1688,2250, 3000, 3750, 4688, 5813, 7125, 9000, 11250, 13875, 19688}
    int a_opt_band_gain[DOLBY_DAP_MAX_CHANNELS][DOLBY_DAP_OPT_MAX_BANDS];   // | -480 ~ 480         {...} show in AU_CUS_AQ_Tuning_Dolby_DAP_SetParam(void)
                                                                            // |
    int bass_enable;                                                        // |    0 ~ 1           1
    int bass_boost;                                                         // |    0 ~ 384         177
    int bass_cutoff;                                                        // |   20 ~ 2000        140
    int bass_width;                                                         // |    2 ~ 64          9
                                                                            // |
    int reg_nb_bands;                                                       // |    1 ~ 20          20
    int a_reg_band_center[DOLBY_DAP_REG_MAX_BANDS];                         // |   20 ~ 20000       {47,   141,  234,  328,  469,  656,  844,  1031, 1313, 1688, 2250, 3000, 3750, 4688, 5813, 7125, 9000, 11250, 13875, 19688}
    int a_reg_low_thresholds[DOLBY_DAP_REG_MAX_BANDS];                      // |-2080 ~ 0           {-496, -192, -448, -384, -192, -192, -192, -192, -192, -192, -192, -192, -192, -192, -192, -192, -192, -192,  -192,  -192}
    int a_reg_high_thresholds[DOLBY_DAP_REG_MAX_BANDS];                     // |-2080 ~ 0           {-304, 0, -256, -192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    int a_reg_isolated_bands[DOLBY_DAP_REG_MAX_BANDS];                      // |    0 ~ 1           {1,    0, 1,    1,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
                                                                            // |
    int regulator_overdrive;                                                // |    0 ~ 192         0
    int regulator_timbre;                                                   // |    0 ~ 16          12
    int regulator_distortion;                                               // |    0 ~ 144         96
    int regulator_mode;                                                     // |    0 ~ 1           1
    int regulator_enable;                                                   // |    0 ~ 1           1
    int virtual_bass_mode;                                                  // |    0 ~ 3           3
    int virtual_bass_low_src_freq;                                          // |   30 ~ 90          55
    int virtual_bass_high_src_freq;                                         // |   90 ~ 270         110
    int virtual_bass_overall_gain;                                          // | -480 ~ 0           0
    int virtual_bass_slope_gain;                                            // |   -3 ~ 0           -2
    int virtual_bass_subgain[3];                                            // | -480 ~ 0           {-80, -240, -400}
    int virtual_bass_mix_low_freq;                                          // |    0 ~ 375         110
    int virtual_bass_mix_high_freq;                                         // |  281 ~ 938         281
    int virtual_bass_enable;                                                // |    0 ~ 1           1
                                                                            // |
    int a_opt_band_gain2[2][DOLBY_DAP_OPT_MAX_BANDS];                       // | -480 ~ 480         0 //(4 fractional bits: [-30.00 dB, 30.00 dB])
    int height_filter_mode;                                                 // |    0 ~ 2           0 //0:Height filter is disabled. 1:Height filter configured for front firing speakers. 2:Height filter configured for up firing speakers
    int leveler_ignore_il;                                                  // |    0 ~ 1           1 //Value is a boolean (0 or 1)
    int lfe_to_lr_gain;                                                     // |  -96 ~ 0           0 //dB
    int b_lfe_to_lr_enable;                                                 // |    0 ~ 1           0 //Value is a boolean (0 or 1)
    int b_lfe_10db_boost_enable;                                            // |    0 ~ 1           0 //Value is a boolean (0 or 1)
    int b_bass_extraction;                                                  // |    0 ~ 1           0 //Value is a boolean (0 or 1)
    int bass_extraction_cutoff_freq;                                        // |   45 ~ 200         0 //(unit is Hz)
} caudDolbyDapParam_t;

typedef struct {
    MS_U8 param[DOLBY_DAP_BIN_PARAM_MAX_SIZE];
    MS_U32 size;
} caudDolbyDapBinParam_t;

//For SonicEmotion ABS3D's parameter setting structure.
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct InputCtrl_Tuning                     // |________________________________________________________________________________________________
{                                                   // |
    int Vol_dB;                                     // |    [-80 ~ 12] * 1000           -12 * 1000
} InputCtrl_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct CrossOverIIR_Tuning                  // |________________________________________________________________________________________________
{                                                   // |
    int Fc_Hz;                                      // |    [1 ~ 1000] * 1000           200 * 1000
    int Enable;                                     // |    [0 ~ 1]    * 1000           1   * 1000
} CrossOverIIR_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct AnalyzerTD_V2_Tuning                 // |________________________________________________________________________________________________
{                                                   // |
    int Ambience;                                   // |    [0 ~ 1] * 1000              0.9  * 1000
    int Balance;                                    // |    [0 ~ 1] * 1000              0.5  * 1000
    int CenterFocus;                                // |    [0 ~ 1] * 1000              0.75 * 1000
    int Enable;                                     // |    [0 ~ 1] * 1000              1    * 1000
} AnalyzerTD_V2_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct WCMatrix_V3_Tuning                   // |________________________________________________________________________________________________
{                                                   // |
    int AnalyzerLatency;                            // |    [0 ~ 256]  * 1000           256 * 1000
    int Enable;                                     // |    [0 ~ 1]    * 1000           1   * 1000
                                                    // |
    int LRWCVol_dB;                                 // |    [-80 ~ 12] * 1000           0   * 1000
    int ClCrVol_dB;                                 // |    [-80 ~ 12] * 1000           0   * 1000
    int C51Vol_dB;                                  // |    [-80 ~ 12] * 1000           0   * 1000
    int LsRsVol_dB;                                 // |    [-80 ~ 12] * 1000           0   * 1000
    int CSDiffVol_dB;                               // |    [-80 ~ 12] * 1000           0   * 1000
                                                    // |
    int LRWCwidth;                                  // |    [0 ~ 1]    * 1000           1   * 1000
    int ClCrFocus;                                  // |    [0 ~ 1]    * 1000           0.5 * 1000
    int C51Focus;                                   // |    [0 ~ 1]    * 1000           1   * 1000
                                                    // |
    int C51Delay_ms;                                // |    [0 ~ 20]   * 1000           0   * 1000
    int LsRsDelay_ms;                               // |    [0 ~ 20]   * 1000           0   * 1000
                                                    // |
    int CS_Dist;                                    // |    [0 ~ 1]    * 1000           1   * 1000
    int LRWCFocusCSDist;                            // |    [0 ~ 1]    * 1000           0.5 * 1000
    int ClCrFocusCSDist;                            // |    [0 ~ 1]    * 1000           0.6 * 1000
    int C51FocusCSDist;                             // |    [0 ~ 1]    * 1000           0.5 * 1000
    int LsRsFocusCSDist;                            // |    [0 ~ 1]    * 1000           0.7 * 1000
} WCMatrix_V3_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct LevelEqualizer_V3_Tuning             // |________________________________________________________________________________________________
{                                                   // |
    int Enable;                                     // |    [0 ~ 1]     * 1000          1             * 1000
    int Threshold_dB;                               // |    [-40 ~ -3]  * 1000          -30           * 1000
    int LevelRange_dB;                              // |    [3 ~ 40]    * 1000          20            * 1000
    int MinGain_dB[2];                              // |    [-20 ~ 20]  * 1000          {0,0}         * 1000
    int MaxGain_dB[2];                              // |    [-20 ~ 20]  * 1000          {0,0}         * 1000
    int Frequency_Hz[2];                            // |    [20 ~ 20000]* 1000          {2500,5500}   * 1000
    int Q[2];                                       // |    [0.01 ~ 10] * 1000          {0.707,0.707} * 1000
    int AttackTime_ms;                              // |    [0.01 ~ 1000]*1000          500           * 1000
    int ReleaseTime_ms;                             // |    [0.01 ~ 1000]*1000          500           * 1000
} LevelEqualizer_V3_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct ParametricEqualizer_V1_Tuning        // |________________________________________________________________________________________________
{                                                   // |
    int Enable;                                     // |    [0 ~ 1]      * 1000         1                         * 1000
    int GlobalMakeUpGain_dB;                        // |    [-80 ~ 12]   * 1000         0                         * 1000
                                                    // |
    int EQAllChannels_Q[4];                         // |    [0.5 ~ 20]   * 1000         {0.707,0.707,0.707,0.707} * 1000
    int EQAllChannels_Frequency_Hz[4];              // |    [10 ~ 20000] * 1000         {125,250,500,1000}        * 1000
    int EQAllChannels_Gain_dB[4];                   // |    [-12 ~ 12]   * 1000         {0,0,0,0}                 * 1000
    int EQAllChannels_FilterType[4];                // |    [0 ~ 4]      * 1000         {0,0,0,0}                 * 1000
    int EQAllChannels_Enable[4];                    // |    [0 ~ 1]      * 1000         {1,1,1,1}                 * 1000
} ParametricEqualizer_V1_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct Abs3DRenderer_V3_Tuning              // |________________________________________________________________________________________________
{                                                   // |
    int Spacing_m;                                  // |    [0.1 ~ 1] * 1000            0.5      * 1000
    int Angle_deg[2];                               // |    [10 ~ 90],[270 ~ 350]*1000  {50,310} * 1000
                                                    // |
    int CTC_Enable;                                 // |    [0 ~ 1]   * 1000            1        * 1000
    int Binaural_Enable;                            // |    [0 ~ 1]   * 1000
                                                    // |
    int FilterCTC_FileIndex;                        // |    [0]       * 1000            0        * 1000
    int FilterBinaural_FileIndex;                   // |    [0]       * 1000            0        * 1000
    int DelayBinaural_FileIndex;                    // |    [0]       * 1000            0        * 1000
} Abs3DRenderer_V3_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct BassEnhancer_V3_Tuning               // |________________________________________________________________________________________________
{                                                   // |
    int Eq_Enable;                                  // |    [0 ~ 1]     * 1000          1     * 1000
    int Compressor_Enable;                          // |    [0 ~ 1]     * 1000          1     * 1000
                                                    // |
    int Ratio;                                      // |    [1 ~ 20]    * 1000          1.7   * 1000
    int LevelAttackTime_ms;                         // |    [0 ~ 200]   * 1000          1     * 1000
    int LevelReleaseTime_ms;                        // |    [0 ~ 200]   * 1000          20    * 1000
    int SmoothingAttackTime_ms;                     // |    [0 ~ 200]   * 1000          20    * 1000
    int SmoothingReleaseTime_ms;                    // |    [0 ~ 200]   * 1000          1     * 1000
    int Threshold_dB;                               // |    [-40 ~ 0]   * 1000          -25   * 1000
    int MakeupGain_dB;                              // |    [-20 ~ 20]  * 1000          -3    * 1000
                                                    // |
    int HighPass_Enable;                            // |    [0 ~ 1]     * 1000          1     * 1000
    int HighPassFc_Hz;                              // |    [10 ~ 1000] * 1000          50    * 1000
    int HighPassDecay_dB;                           // |    [-12 ~ -24] * 1000          -24   * 1000
                                                    // |
    int PeakFilter1_Enable;                         // |    [0 ~ 1]     * 1000          1     * 1000
    int Peakfilter1Gain_dB;                         // |    [-20 ~ 20]  * 1000          0     * 1000
    int Peakfilter1Fc_Hz;                           // |    [10 ~ 1000] * 1000          140   * 1000
    int Peakfilter1Q;                               // |    [0.5 ~ 20]  * 1000          0.707 * 1000
                                                    // |
    int PeakFilter2_Enable;                         // |    [0 ~ 1]     * 1000          1     * 1000
    int Peakfilter2Gain_dB;                         // |    [-20 ~ 20]  * 1000          0     * 1000
    int Peakfilter2Fc_Hz;                           // |    [10 ~ 1000] * 1000          200   * 1000
    int Peakfilter2Q;                               // |    [0.5 ~ 20]  * 1000          0.707 * 1000
                                                    // |
    int Bwe_Enable;                                 // |    [0 ~ 1]     * 1000          1     * 1000
    int Bwe_PostNLDLowPassShapingFilterFc_Hz;       // |    [0 ~ 1]     * 1000          520   * 1000
    int Bwe_OrigGain_dB;                            // |    [-20 ~ 0]   * 1000          0     * 1000
    int Bwe_NLDGain_dB;                             // |    [-6 ~ 20]   * 1000          6     * 1000
    int Bwe_AttackRemoverTime_ms;                   // |    [100 ~ 500] * 1000          100   * 1000
    int Bwe_AttackRemoverShortTermTime_ms;          // |    [0 ~ 50]    * 1000          0     * 1000
    int Bwe_GainSmooAttackTime_ms;                  // |    [0 ~ 20]    * 1000          5     * 1000
    int Bwe_GainSmooReleaseTime_ms;                 // |    [0 ~ 200]   * 1000          15    * 1000
    int Bwe_HarmonicDecay_dB;                       // |    [-10 ~ -24] * 1000          -10   * 1000
    int Bwe_H1Level_dB;                             // |    [-80 ~ 20]  * 1000          -80   * 1000
    int Bwe_H2Level_dB;                             // |    [-20 ~ 20]  * 1000          6     * 1000
    int Bwe_Compressor_Enable;                      // |    [0 ~ 1]     * 1000          1     * 1000
    int Bwe_PostNLDHighPass_Enable;                 // |    [0 ~ 1]     * 1000          0     * 1000
    int Bwe_PostNLDLowPassShapingFilter_Enable;     // |    [1 ~ 4xcrossOverFc]*1000    0     * 1000
                                                    // |
    int LevelEstimatorBass_Enable;                  // |    [0 ~ 1]     * 1000          1     * 1000
    int LevelEstimatorBass_Threshold_dB;            // |    [-40 ~ 0]   * 1000          -30   * 1000
    int LevelEstimatorBass_LevelRange_dB;           // |    [0 ~ 20]    * 1000          20    * 1000
    int LevelEstimatorBass_MinGain_dB;              // |    [-10 ~ 3]   * 1000          0     * 1000
    int LevelEstimatorBass_MaxGain_dB;              // |    [0 ~ 20]    * 1000          15    * 1000
    int LevelEstimatorBass_AttackTime_ms;           // |    [0 ~ 5000]  * 1000          500   * 1000
    int LevelEstimatorBass_ReleaseTime_ms;          // |    [0 ~ 5000]  * 1000          500   * 1000
    int LevelEqualizer_Q;                           // |    [0.01 ~ 10] * 1000          1     * 1000
    int LevelEqualizer_Fc_Hz;                       // |    [1 ~ crossOverFc]*1000      110   * 1000
} BassEnhancer_V3_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct BassRenderer_V2_Tuning               // |________________________________________________________________________________________________
{                                                   // |
    int ManualDelay_ms;                             // |    [0 ~ 20]   * 1000           0   * 1000
    int LatencyLf_Samples;                          // |    [0 ~ 256]  * 1000           116 * 1000
    int LatencyHf_Samples;                          // |    [0 ~ 256]  * 1000           256 * 1000
    int DeviceMode;                                 // |    [0,3]      * 1000           2   * 1000
} BassRenderer_V2_Tuning;
                                                    //  ________________________________________________________________________________________________
                                                    // |
                                                    // |    Range                       Default
typedef struct ECompressor_V3_Tuning                // |________________________________________________________________________________________________
{                                                   // |
    int LevelAttackTime_ms;                         // |    [0 ~ 200]  * 1000           0.5  * 1000
    int LevelReleaseTime_ms;                        // |    [0 ~ 200]  * 1000           2    * 1000
    int Threshold_dB;                               // |    [-40 ~ 0]  * 1000           -3   * 1000
    int Ratio;                                      // |    [1 ~ 20]   * 1000           20   * 1000
    int MakeupGain_dB;                              // |    [-20 ~ 20] * 1000           0    * 1000
    int SmoothingAttackTime_ms;                     // |    [0 ~ 200]  * 1000           10   * 1000
    int SmoothingReleaseTime_ms;                    // |    [0 ~ 200]  * 1000           100  * 1000
    int Enable;                                     // |    {0 ~ 1}    * 1000           1    * 1000
} ECompressor_V3_Tuning;


typedef struct {
    InputCtrl_Tuning InputCtrl;
    CrossOverIIR_Tuning CrossOverIIR;
    AnalyzerTD_V2_Tuning AnalyzerTD_V2;
    WCMatrix_V3_Tuning WCMatrix_V3;
    LevelEqualizer_V3_Tuning LevelEqualizer_V3[2];
    ParametricEqualizer_V1_Tuning ParametricEqualizer_V1[5];
    Abs3DRenderer_V3_Tuning Abs3DRenderer_V3;
    BassEnhancer_V3_Tuning BassEnhancer_V3;
    BassRenderer_V2_Tuning BassRenderer_V2;
    ECompressor_V3_Tuning ECompressor_V3[2];
} caudSonicEmotionAbs3dParam_t;

// For DTS VirtualX's parameter setting structure.
#if 0
#define kTSXbitMskFront     ( 0x003 )   /**< Channel mask type for Front                    */
#define kTSXbitMskCtr       ( 0x004 )   /**< Channel mask type for Center                   */
#define kTSXbitMskLfe       ( 0x008 )   /**< Channel mask type for Low Frequency Effects    */
#define kTSXbitMskRear      ( 0x030 )   /**< Channel mask type for Rear                     */
#define kTSXbitMskSide      ( 0x0C0 )   /**< Channel mask type for Side                     */
#define kTSXbitMskFrntHgt   ( 0x300 )   /**< Channel mask type for Front Height             */
#define kTSXbitMskRearHgt   ( 0xC00 )   /**< Channel mask type for Rear Front               */

// VirtualXLib1 Input channel layout
typedef enum {
    kTSXIn_Stereo = kTSXbitMskFront,                                                                                          /**< Represents 2.0 Ch   */
    kTSXIn_5p1    = kTSXbitMskFront | kTSXbitMskCtr | kTSXbitMskLfe | kTSXbitMskRear,                                         /**< Represents 5.1 Ch   */
    kTSXIn_7p1    = kTSXbitMskFront | kTSXbitMskCtr | kTSXbitMskLfe | kTSXbitMskRear | kTSXbitMskSide,                        /**< Represents 7.1 Ch   */
    kTSXIn_5p1p2  = kTSXbitMskFront | kTSXbitMskCtr | kTSXbitMskLfe | kTSXbitMskRear | kTSXbitMskFrntHgt,                     /**< Represents 5.1.2 Ch */
    kTSXIn_5p1p4  = kTSXbitMskFront | kTSXbitMskCtr | kTSXbitMskLfe | kTSXbitMskRear | kTSXbitMskFrntHgt | kTSXbitMskRearHgt, /**< Represents 5.1.4 Ch */
} DTS_TRUSRNDX_INPUT_MODE;

// VirtualXLib1 Output channel layout
typedef enum {
    kTSXOut_Stereo = kTSXbitMskFront,
} DTS_TRUSRNDX_OUTPUT_MODE;

// VirtualXLib1 Horizontal plane effect controls
typedef enum {
    kTSX_HorizEffct_Default,  /**< Default Horizontal plane effect control */
    kTSX_HorizEffct_Mild,     /**< Mild Horizontal plane effect control    */
} DTS_TSX_HORIZN_EFFECT_CTRL;
                                                            // VirtualX API control params:
                                                            // Structure containing VirtualX API specific control params.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct dtsVXApiCtrl_t                               // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int                       nVXLib1Enable;                // |    0x1                 0 ~ 1                                                                   Control to enable or disable VirtualX Lib1 processing
    DTS_TRUSRNDX_INPUT_MODE   nInputMode;                   // |    0x3                 3:"2.0",63:"5.1",255:"7.1",831:"5.1.2",3903:"5.1.4"                     Control to select VirtualX input channel layout
    DTS_TRUSRNDX_OUTPUT_MODE  nOutputMode;                  // |    0x3                 3:"2.0"                                                                 Control to select VirtualX output channel layout
    int                       nHeadRoom;                    // |    0x40000000(1.0)     0x8000000 ~ 0x40000000(normalized:         0.125 ~ 1.0)                 Control to specify Headroom gain
    int                       nProOutputGain;               // |    0x10000000(1.0)     0x8000000 ~ 0x40000000(normalized:           0.5 ~ 4.0)                 Control to specify processing output gain
    int                       nReferenceLevel;              // |    0x4000000(-24 dB)   0xCCCCD   ~ 0x4000000 (normalized:7.8e-4(-62 dB) ~ 0.0625(-24 dB))      Control for virtualxLib1 reference level( Read-only )
} dtsVXApiCtrl_t;
                                                            // TruSrndX API control params:
                                                            // Structure containing TruSrndX API specific control params.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct dtsTSXApiCtrl_t                              // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int                         nTsxEnable;                 // |    0x1                          0 ~ 1                                                          Control to enable or disable TruSurroundX processing
    int                         nPassiveMtrxEnable;         // |    0x1                          0 ~ 1                                                          Control to enable or disable TruSurroundX Passive Matrix
    int                         nHeightUpmixEnable;         // |    0x1                          0 ~ 1                                                          Control to enable or disable TruSurroundX Height Upmix
    DTS_TSX_HORIZN_EFFECT_CTRL  nHorizEffectStrength;       // |    0x0                     0:Mild ~ 1:Default                                                  Control to specify TruSurroundX Horizontal plane effect strength
    int                         nLpRtoCtrMixGain;           // |    0x20000000(1.0)            0x0 ~ 0x40000000(normalized: 0.0 ~ 2.0)                          Control to specify TruSurroundX LpR to Center Mix Gain
    int                         nCtrGain;                   // |    0x20000000(1.0)     0x20000000 ~ 0x40000000(normalized: 1.0 ~ 2.0)                          Control to specify TruSurroundX Center gain
    int                         nHeightMixCoeff;            // |    0x20000000(1.0)     0x10000000 ~ 0x40000000(normalized: 0.5 ~ 2.0)                          Control to specify TruSurroundX Height Mix Coefficient
    int                         nTsxProcessDiscrad;         // |    0x0                          0 ~ 1                                                          Control to discard VirtualX Lib1 processing completely including headroom
    int                         nTsxHeightDiscrad;          // |    0x0                          0 ~ 1                                                          Control to discard VirtualX Lib1 Height Processing completely
    int                         nFrontControl;              // |    0x20000000(1.0)     0x10000000 ~ 0x40000000(normalized: 0.5 ~ 2.0)                          Control the Front widening effect gain
    int                         nSurroundControl;           // |    0x20000000(1.0)     0x10000000 ~ 0x40000000(normalized: 0.5 ~ 2.0)                          Control the Surround effect gain
} dtsTSXApiCtrl_t;
                                                            // SubComponent control params:
                                                            // Structure containing control params for Dialog Clarity & Definition functionalities.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct dtsVirtualXLib1SubCompApiCtrl_t              // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int nEnable;                                            // |    0x0                   0 ~ 1                                                                 Control to enable Dialog Clarity or Definition
    int nControl;                                           // |    0x199999A0(0.4)     0x0 ~ 0x40000000(normalized: 0.0 ~ 1.0)                                 Represents Dialog Clarity control or Definition control
} dtsVirtualXLib1SubCompApiCtrl_t;
                                                            // VirtualXLib1 API Control structure :
                                                            // Master structure containing control params for VirtualXLib1, TrusrndX, Definition and Dialog Clarity.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Description
typedef struct                                              // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    dtsVXApiCtrl_t                  vxApiControls;          // |    VirtualX controls.       Refer #dtsVXApiCtrl_t
    dtsTSXApiCtrl_t                 tsxApiControls;         // |    TrusrndX controls.       Refer #dtsTSXApiCtrl_t
    dtsVirtualXLib1SubCompApiCtrl_t focusApiControls;       // |    Dialog Clarity controls. Refer #dtsVirtualXLib1SubCompApiCtrl_t
    dtsVirtualXLib1SubCompApiCtrl_t defApiControls;         // |    Definition controls.     Refer #dtsVirtualXLib1SubCompApiCtrl_t
} dtsVirtualXLib1ApiCtrl_t;
                                                            // MBHL common API control structure [Multi-band Hard Limiter]
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct dtsMbhlCommonApiCtrl                         // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int nMbhlEnable;                                        // |    0x1                 0(bypass) ~ 1(enable)                                                   Control to enable Dialog Clarity or Definition
    int nMbhlBypassGain;                                    // |    0x40000000(1.0)           0x0 ~ 0x40000000(normalized:   0.0 ~ 1.0)                         The bypass gain is only applied when the enable control is adjusted to Bypass setting
    int nMbhlReferenceLevel;                                // |    0x40000000(1.0)      0x10624E ~ 0x40000000(normalized: 0.001 ~ 1.0)                         Set this control to the amount of external headroom applied prior to MBHL processing
    int nMbhlVolume;                                        // |    0x40000000(1.0)           0x0 ~ 0x40000000(normalized:   0.0 ~ 1.0)                         Adjust the volume level based on the master volume level of the audio system
    int nMbhlVolumeStep;                                    // |    0x64                      0x0 ~ 0x64      (normalized:     0 ~ 100)                         Adjust the volume level based on the master volume step of the audio system displayed to the user
    int nMbhlBalanceStep;                                   // |    0x0                       -10 ~ 10        (normalized:   -10 ~ 10)                          Adjust the stereo balance or panning based on the user selected value
    int nMbhlOutputGain;                                    // |    0x40000000(1.0)           0x0 ~ 0x40000000(normalized:   0.0 ~ 1.0)                         This is a straight gain after the Hard Limiter and should only be used in cases
    int nMbhlMode;                                          // |    0x0                 0:Limiter,1:         Compressor Limiter,2:Low   Band Compressor Limiter The Mode control determines the dynamics processing applied to the signal
                                                            // |                                  3:Two Band Compressor Limiter,4:Three Band Compressor Limiter
    int nMbhlProcessDiscrad;                                // |    0x0                         0 ~ 1                                                           Control to discard MBHL processing completely including application of bypass gain
} dtsMbhlCommonApiCtrl;

                                                            // MBHL compressor API Control structure
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct dtsMbhlCompressorApiCtrl                     // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int nMbhlCrossLow;                                      // |    0x7                  0:40   Hz,  1:60 Hz,    2:100 Hz,   3:120 Hz                           The Low frequency cutoff of the crossover for multi-band copression modes
                                                            // |                         4:150  Hz,  5:200 Hz,   6:250 Hz,   7:300 Hz
                                                            // |                         8:400  Hz,  9:500 Hz,  10:600 Hz,  11:800 Hz
                                                            // |                        12:1000 Hz, 13:1600 Hz, 14:2000 Hz, 15:3000 Hz
    int nMbhlCrossMid;                                      // |    0xF                 16:4000 Hz, 17:5000 Hz, 18:6000 Hz, 19:8000 Hz, 20:CUSTOM Hz            The Mid frequency cutoff of the crossover for multi-band copression modes
    int nMbhlCompAttack;                                    // |    0x5                       0x0 ~ 0x64      (normalized:     0 ~ 100)                         The Compressor Attack setting adjusts the attack time constant of all employed compressors for the current mode
    int nMbhlCompLowRelease;                                // |    0xFA(250)                0x32 ~ 0x7D0     (normalized:    50 ~ 2000)                        A time of 500ms is recommended for a more subtle effect. Going below 250ms is not recommended
    int nMbhlCompLowRatio;                                  // |    0x10000000(4.0)     0x4000000 ~ 0x50000000(normalized:   1.0 ~ 20.0)                        Ratio of 4 is recommended for a more subtle effect. Higher ratios increase the amount of compression
    int nMbhlCompLowThresh;                                 // |    0x4026808(0.501)    0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      Each of the sub bands in the MBHL has a separate control for this
    int nMbhlCompLowMakeup;                                 // |    0x8000000(1.0)      0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      There is a separate Make Up Gain for each of the sub bands
    int nMbhlCompMidRelease;                                // |    0xFA(250)               0x32  ~ 0x7D0     (normalized:    50 ~ 2000)                        A time of 500ms is recommended for a more subtle effect. Going below 250ms is not recommended
    int nMbhlCompMidRatio;                                  // |    0x10000000(4.0)     0x4000000 ~ 0x50000000(normalized:   1.0 ~ 20.0)                        Ratio of 4 is recommended for a more subtle effect.
    int nMbhlCompMidThresh;                                 // |    0x4026808(0.501)    0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      Each of the sub bands in the MBHL has a separate control for this
    int nMbhlCompMidMakeup;                                 // |    0x8000000(1.0)      0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      There is a separate Make Up Gain for each of the sub bands
    int nMbhlCompHighRelease;                               // |    0xFA(250)               0x32  ~ 0x7D0     (normalized:    50 ~ 2000)                        A time of 500ms is recommended for a more subtle effect. Going below 250ms is not recommended
    int nMbhlCompHighRatio;                                 // |    0x10000000(4.0)     0x4000000 ~ 0x50000000(normalized:   1.0 ~ 20.0)                        Ratio of 4 is recommended for a more subtle effect
    int nMbhlCompHighThresh;                                // |    0x4026808(0.501)    0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      Each of the sub bands in the MBHL has a separate control for this
    int nMbhlCompHighMakeup;                                // |    0x8000000(1.0)      0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      There is a separate Make Up Gain for each of the sub bands
    int nMbhlCompGainReduction[3];                          // |    0x8000000(1.0)      0x83126F  ~ 0x7EC8B400(normalized: 0.064 ~ 15.848)                      Realtime compressor gain
} dtsMbhlCompressorApiCtrl;
                                                            // MBHL limiter API Control structure
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct dtsMbhlLimiterApiCtrl                        // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int nMbhlBoost;                                         // |    0x200000(1.0)           0x831 ~ 0x7D000000(normalized: 0.001 ~ 1000.0)                      Adjusts the amount of boost that is applied by the limiter
    int nMbhlThreshold;                                     // |    0x40000000(1.0)     0x4189378 ~ 0x40000000(normalized: 0.064 ~ 1.0)                         The limiter threshold controls the level at which the final limiter will begin to apply its dynamic processing
    int nMbhlSlowOffset;                                    // |    0x20000000(1.0)     0xA24DD30 ~ 0x652F1A80(normalized: 0.317 ~ 3.162)                       This control allows the threshold of the slow gain smoother
    int nMbhlFastAttack;                                    // |    0x28000000(5.0)           0x0 ~ 0x50000000(normalized:   0.0 ~ 10.0)                        The fast attack ballistic of the final limiter determining the rate at which the dynamic gain will be reduced
    int nMbhlFastRelease;                                   // |    0x32(50)                  0xA ~ 0x1F4     (normalized:    10 ~ 500)                         The fast release ballistic of the final limiter for quickly recovery from short-term transient events
    int nMbhlSlowAttack;                                    // |    0x1F4(500)               0x64 ~ 0x3E8     (normalized:   100 ~ 1000)                        The slow attack ballistic of the final limiter
    int nMbhlSlowRelease;                                   // |    0x1F4(500)               0x64 ~ 0x3E8     (normalized:   100 ~ 1000)                        The slow release ballistic of the final limiter
    int nMbhlDelay;                                         // |    0x8(5P3MS)           1:0P6MS, 2:1P3MS, 3:1P9MS, 4:2P6MS                                     The amount of limiter look-ahead delay employed.
                                                            // |                         5:3P3MS, 6:3P9MS, 7:4P6MS, 8:5P3MS                                     More look-ahead will result in better sound quality but will
                                                            // |                         9:5P9MS,10:6P6MS,11:7P3MS,12:7P9MS                                     increase the processing delay
                                                            // |                        13:8P6MS,14:9P3MS,15:9P9MS,16:10P6MS
    int nMbhlEnvelopeFrequency;                             // |    0x14(20)                  0x5 ~ 0x1F4     (normalized:     5 ~ 500)                         The Peak Hold Frequency control will adjust the analysis window of time
    int nMbhlGainReduction;                                 // |    0x40000000(1.0)     0x4189378 ~ 0x40000000(normalized: 0.064 ~ 1.0)                         Realtime final limiter gain reduction
} dtsMbhlLimiterApiCtrl;
                                                            // Multi-band Hard Limiter API Control structure
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Description
typedef struct dtsMbhlApiCtrl                               // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    dtsMbhlCommonApiCtrl     mbhlCommonApiControls;         // |    VirtualX controls.          Refer #dtsMbhlCommonApiCtrl
    dtsMbhlCompressorApiCtrl mbhlCompApiControls;           // |    MBHL Compressor controls.   Refer #dtsMbhlCompressorApiCtrl
    dtsMbhlLimiterApiCtrl    mbhlLimApiControls;            // |    MBHL limiter controls.      Refer #dtsMbhlLimiterApiCtrl
} dtsMbhlApiCtrl;
                                                            // TBHDX API control params:
                                                            // Structure containing TBHDX API specific control params.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
typedef struct _dtsTBHDxApiCtrl                             // |____________________________________________________________________________________________________________________________________________________
{                                                           // |
    int nEnable;                                            // |    0x0(disable)               0 ~ 1                                                            Enable/Disable TruBass HDx processing
    int nProcMode;                                          // |    0x0(disable)               0 ~ 1                                                            TruBass HDX processing mode
    int nSpkSize;                                           // |    0x2(80Hz)           0:40Hz , 1:60Hz , 2:80Hz , 3:100Hz, 4:120Hz, 5:150Hz, 6:200Hz,          Sets the speaker size setting for tbhdx
                                                            // |                        7:250Hz, 8:300Hz, 9:400Hz,10:500Hz,11:600Hz,12:Custom
    int nDynamics;                                          // |    0x13333333(0.3)          0x0 ~ 0x40000000(normalized:   0.0 ~ 1.0)                          TruBass HDX dynamic range
    int nHPEnable;                                          // |    0x1(enable)                0 ~ 1                                                            Enables/Disables the high-pass filter of the process
    int nHpOrder;                                           // |    0x4                        1 ~ 8                                                            Sets the order of the high-pass filter
    int nBassLvl;                                           // |    0x15586a47(0.334)   0x10624E ~ 0x40000000(normalized: 0.001 ~ 1.0)                          TruBass HDX bass level
    int nTbhdxProcessDiscrad;                               // |    0x0(disable)               0 ~ 1                                                            Control to discard TBHDX processing completely including application of bypass gain
} dtsTBHDxApiCtrl;


typedef struct {
    dtsVirtualXLib1ApiCtrl_t VirtualxLib1APICtrls;
    dtsMbhlApiCtrl           MbhlAPICtrls;
    dtsTBHDxApiCtrl          TBHDxAPICtrls;
} caudDtsVxParam_t;


                                                            // TBHDX APP control params:
                                                            // Structure containing TBHDX APP specific control params.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
                                                            // |____________________________________________________________________________________________________________________________________________________
typedef struct                                              // |
{                                                           // |
    int     spkSize;                                        // |    80                        40 ~ 600                                                          Sets the speaker size setting for tbhdx
    float   bassLvl;                                        // |    0.333521432f        0.333521432f                                                            TruBass HDX bass level
    float   hpRatio;                                        // |    0.5                        0 ~ 1.0                                                          Sets the cut-off frequency of the high-pass filter as a function of the speaker size
    float   extBass;                                        // |    0.8                        0 ~ 1.0                                                          Controls the amount of bass added to the sub-speaker region
} dtsTBHDxAppCtrl;
                                                            // Multi-band Hard Limiter APP control params:
                                                            // Structure containing Multi-band Hard Limiter APP specific control params.
                                                            //  ____________________________________________________________________________________________________________________________________________________
                                                            // |
                                                            // |    Default             Range                                                                   Description
                                                            // |____________________________________________________________________________________________________________________________________________________
typedef struct                                              // |
{                                                           // |
    float   lowCrossFreq;                                   // |    300                       40 ~ 10000                                                        MBHL Low Cross Freq
    float   midCrossFreq;                                   // |    5000                      40 ~ 10000                                                        MBHL Mid Cross Freq
} dtsMbhlAppCtrl;


typedef struct {
    dtsTBHDxAppCtrl TBHDxAPPCtrls;
    dtsMbhlAppCtrl  MbhlAPPCtrls;
} caudDtsVxAppCtrls_t;
#endif
                                                                  // [VirtualX library]
                                                                  //  ____________________________________________________________________________________________________________________________________________________
                                                                  // |
typedef enum                                                      // |    CLI                                         Default     Range
{                                                                 // |____________________________________________________________________________________________________________________________________________________
    CAUD_dtsVx_Cli_Invalid                                = -1,   // |
    CAUD_dtsVxApiCtrl_nVXLib1Enable                       = 0,    // |    "vxlib1.enable 1"                           1           0 ~ 1
    CAUD_dtsVxApiCtrl_nInputMode                          = 1,    // |    "vxlib1.in_mode 4"                          4           0:2.0, 1:5.1, 2:7.1, 3.5.1.2, 4:5.1.4
    CAUD_dtsVxApiCtrl_nOutputMode                         = 2,    // |    "vxlib1.out_mode 0"                         0           0:2.0
    CAUD_dtsVxApiCtrl_nHeadRoom                           = 3,    // |    "vxlib1.headroom_gain 1.0"                  1.0         0.125 ~ 1.0
    CAUD_dtsVxApiCtrl_nProOutputGain                      = 4,    // |    "vxlib1.processing_output_gain 1.0"         1.0         0.5 ~ 4.0
    CAUD_dtsVxApiCtrl_nReferenceLevel                     = 5,    // |    "vxlib1.referenceLevel 0.0625"              0.0625      7.8e-4(-62 dB) ~ 0.0625(-24 dB)
    CAUD_dtsTSXApiCtrl_nTsxEnable                         = 6,    // |    "vxlib1.tsx.enable 1"                       1           0 ~ 1
    CAUD_dtsTSXApiCtrl_nPassiveMtrxEnable                 = 7,    // |    "vxlib1.tsx.pssv_mtrx_enable 1"             1           0 ~ 1
    CAUD_dtsTSXApiCtrl_nHeightUpmixEnable                 = 8,    // |    "vxlib1.tsx.hght_upmix_enable 1"            1           0 ~ 1
    CAUD_dtsTSXApiCtrl_nHorizEffectStrength               = 9,    // |    "vxlib1.tsx.horiznt_effect_ctrl 1"          1           0 ~ 1
    CAUD_dtsTSXApiCtrl_nLpRtoCtrMixGain                   = 10,   // |    "vxlib1.tsx.LpRtoCtr_mix_gain 1.0"          1.0         0.0 ~ 2.0
    CAUD_dtsTSXApiCtrl_nCtrGain                           = 11,   // |    "vxlib1.tsx.ctrgain 1.0"                    1.0         1.0 ~ 2.0
    CAUD_dtsTSXApiCtrl_nHeightMixCoeff                    = 12,   // |    "vxlib1.tsx.heightmix_coeff 1.0"            1.0         0.5 ~ 2.0
    CAUD_dtsTSXApiCtrl_nTsxProcessDiscrad                 = 13,   // |    "vxlib1.tsx.discard 0"                      0           0 ~ 1
    CAUD_dtsTSXApiCtrl_nTsxHeightDiscrad                  = 14,   // |    "vxlib1.height.discard 0"                   0           0 ~ 1
    CAUD_dtsTSXApiCtrl_nFrontControl                      = 15,   // |    "vxlib1.tsx.front_control 1.0"              1.0         0.5 ~ 2.0
    CAUD_dtsTSXApiCtrl_nSurroundControl                   = 16,   // |    "vxlib1.tsx.surround_control 1.0"           1.0         0.5 ~ 2.0
    CAUD_dtsVirtualXLib1ApiCtrl_focusApiControls_nEnable  = 17,   // |    "vxlib1.dialogClarity.enable 0"             0           0 ~ 1
    CAUD_dtsVirtualXLib1ApiCtrl_focusApiControls_nControl = 18,   // |    "vxlib1.dialogClarity.level 0.4"            0.4         0.0 ~ 1.0
    CAUD_dtsVirtualXLib1ApiCtrl_defApiControls_nEnable    = 19,   // |    "vxlib1.definition.enable 0"                0           0 ~ 1
    CAUD_dtsVirtualXLib1ApiCtrl_defApiControls_nControl   = 20,   // |    "vxlib1.definition.level 0.4"               0.4         0.0 ~ 1.0

                                                                  // [Multi-band Hard Limiter]
                                                                  //  ____________________________________________________________________________________________________________________________________________________
                                                                  // |
                                                                  // |    CLI                                         Default     Range
                                                                  // |____________________________________________________________________________________________________________________________________________________
                                                                  // |
    CAUD_dtsMbhlCommonApiCtrl_nMbhlEnable                 = 21,   // |    "mbhl.enable 1"                             1           0 ~ 1
    CAUD_dtsMbhlCommonApiCtrl_nMbhlBypassGain             = 22,   // |    "mbhl.bypass_gain 1.0"                      1.0         0.0 ~ 1.0
    CAUD_dtsMbhlCommonApiCtrl_nMbhlReferenceLevel         = 23,   // |    "mbhl.reference_level 1.0"                  1.0         0.0009 ~ 1.0
    CAUD_dtsMbhlCommonApiCtrl_nMbhlVolume                 = 24,   // |    "mbhl.volume 1.0"                           1.0         0.0 ~ 1.0
    CAUD_dtsMbhlCommonApiCtrl_nMbhlVolumeStep             = 25,   // |    "mbhl.volume_step 100"                      100         0 ~ 100
    CAUD_dtsMbhlCommonApiCtrl_nMbhlBalanceStep            = 26,   // |    "mbhl.balance_step 0"                       0           -10 ~ 10
    CAUD_dtsMbhlCommonApiCtrl_nMbhlOutputGain             = 27,   // |    "mbhl.output_gain 1.0"                      1.0         0.0 ~ 1.0
    CAUD_dtsMbhlCommonApiCtrl_nMbhlMode                   = 28,   // |    "mbhl.mode 0"                               0           0:Limiter,1:Compressor Limiter,2:Low Band Compressor Limiter,3:Two Band Compressor Limiter,4:Three Band Compressor Limiter
    CAUD_dtsMbhlCommonApiCtrl_nMbhlProcessDiscrad         = 29,   // |    "mbhl.discard 0"                            0           0 ~ 1
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCrossLow           = 30,   // |    "mbhl.cross_low 7"                          7           0:40hz,1:60hz,2:100hz,3:120hz,4:150hz,5:200hz,6:250hz,7:300hz,8:400hz,9:500hz,10:600hz
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCrossMid           = 31,   // |    "mbhl.cross_mid 15"                         15          11:800hz,12:1000hz,13:1600hz,14:2000hz,15:3000hz,16:4000hz,17:5000hz,18:6000hz,19:8000hz,20:Custom
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompAttack         = 32,   // |    "mbhl.comp_attack 5"                        5           0 ~ 100
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompLowRelease     = 33,   // |    "mbhl.comp_low_release 250"                 250         50 ~ 2000
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompLowRatio       = 34,   // |    "mbhl.comp_low_ratio 4.0"                   4.0         1.0 ~ 20.0
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompLowThresh      = 35,   // |    "mbhl.comp_low_thresh 0.5009"               0.5009      0.0640 ~ 15.8479
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompLowMakeup      = 36,   // |    "mbhl.comp_low_makeup 1.0"                  1.0         0.0640 ~ 15.8479
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompMidRelease     = 37,   // |    "mbhl.comp_mid_release 250"                 250         50 ~ 2000
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompMidRatio       = 38,   // |    "mbhl.comp_mid_ratio 4.0"                   4.0         1.0 ~ 20.0
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompMidThresh      = 39,   // |    "mbhl.comp_mid_thresh 0.5009"               0.5009      0.0640 ~ 15.8479
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompMidMakeup      = 40,   // |    "mbhl.comp_mid_makeup 1.0"                  1.0         0.0640 ~ 15.8479
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompHighRelease    = 41,   // |    "mbhl.comp_high_release 250"                250         50 ~ 2000
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompHighRatio      = 42,   // |    "mbhl.comp_high_ratio 4.0"                  4.0         1.0 ~ 20.0
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompHighThresh     = 43,   // |    "mbhl.comp_high_thresh 0.5009"              0.5009      0.0640 ~ 15.8479
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompHighMakeup     = 44,   // |    "mbhl.comp_high_makeup 1.0"                 1.0         0.0640 ~ 15.8479
    CAUD_dtsMbhlCompressorApiCtrl_nMbhlCompGainReduction  = 45,   // |    "mbhl.comp_gain_reduction 1.0 1.0 1.0"      1.0         0.0640 ~ 15.8479
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlBoost                 = 46,   // |    "mbhl.boost 1.0"                            1.0         0.0010 ~ 1000
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlThreshold             = 47,   // |    "mbhl.threshold 1.0"                        1.0         0.0640 ~ 1.0
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlSlowOffset            = 48,   // |    "mbhl.slow_offset 1.0"                      1.0         0.3170 ~ 3.1619
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlFastAttack            = 49,   // |    "mbhl.fast_attack 5"                        5           0 ~ 10
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlFastRelease           = 50,   // |    "mbhl.fast_release 50"                      50          10 ~ 500
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlSlowAttack            = 51,   // |    "mbhl.slow_attack 500"                      500         100 ~ 1000
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlSlowRelease           = 52,   // |    "mbhl.slow_release 500"                     500         100 ~ 2000
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlDelay                 = 53,   // |    "mbhl.delay 8"                              8           1:0.6ms,2:1.3ms,3:1.9ms,4:2.6ms,5:3.3ms,6:3.9ms,7:4.6ms,8:5.3ms,9:5.9ms,10:6.6ms,11:7.3ms,12:7.9ms,13:8.6ms,14:9.3ms,15:9.9ms,16:10.6ms
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlEnvelopeFrequency     = 54,   // |    "mbhl.envelope_frequency 20"                20          5 ~ 500
    CAUD_dtsMbhlLimiterApiCtrl_nMbhlGainReduction         = 55,   // |    "mbhl.gain_reduction 1.0"                   1.0         0.0640 ~ 1.0

                                                                  // [TBHDX]
                                                                  //  ____________________________________________________________________________________________________________________________________________________
                                                                  // |
                                                                  // |    CLI                                         Default     Range
                                                                  // |____________________________________________________________________________________________________________________________________________________
                                                                  // |
    CAUD_dtsTBHDxApiCtrl_nEnable                          = 56,   // |    "tbhdx.enable 0"                            0           0 ~ 1
    CAUD_dtsTBHDxApiCtrl_nProcMode                        = 57,   // |    "tbhdx.mono 0"                              0           0 ~ 1
    CAUD_dtsTBHDxApiCtrl_nSpkSize                         = 58,   // |    "tbhdx.speaker_size 2"                      2           0:40Hz,1:60Hz,2:80Hz,3:100Hz,4:120Hz,5:150Hz,6:200Hz,7:250Hz,8:300Hz,9:400Hz,10:500Hz,11:600Hz,12:Custom
    CAUD_dtsTBHDxApiCtrl_nDynamics                        = 59,   // |    "tbhdx.max_gain 0.3"                        0.3         0.0 ~ 1.0
    CAUD_dtsTBHDxApiCtrl_nHPEnable                        = 60,   // |    "tbhdx.hp_enable 1"                         1           0 ~ 1
    CAUD_dtsTBHDxApiCtrl_nHpOrder                         = 61,   // |    "tbhdx.hp_order 4"                          4           1 ~ 8
    CAUD_dtsTBHDxApiCtrl_nBassLvl                         = 62,   // |    "tbhdx.temporal_gain 0.3339"                0.3339      0.0 ~ 1.0
    CAUD_dtsTBHDxApiCtrl_nTbhdxProcessDiscrad             = 63,   // |    "tbhdx.discard 0"                           0           0 ~ 1

                                                                  // == DTS_VirtualX APP Ctrl ==
                                                                  // [TBHDX]
                                                                  //  ____________________________________________________________________________________________________________________________________________________
                                                                  // |
                                                                  // |    CLI                                         Default     Range
                                                                  // |____________________________________________________________________________________________________________________________________________________
                                                                  // |
    CAUD_dtsTBHDxAppCtrl_spkSize                          = 64,   // |    "tbhdx.app_custom_speaker_size 80"          80          40 ~ 600
    CAUD_dtsTBHDxAppCtrl_bassLvl                          = 65,   // |    NA
    CAUD_dtsTBHDxAppCtrl_hpRatio                          = 66,   // |    "tbhdx.app_hp_ratio 0.5"                    0.5         0.0 ~ 1.0
    CAUD_dtsTBHDxAppCtrl_extBass                          = 67,   // |    "tbhdx.app_extbass_level 0.8"               0.8         0.0 ~ 1.0

                                                                  // [MBHL limiter]
                                                                  //  ____________________________________________________________________________________________________________________________________________________
                                                                  // |
                                                                  // |    CLI                                         Default     Range
                                                                  // |____________________________________________________________________________________________________________________________________________________
                                                                  // |
    CAUD_dtsMbhlAppCtrl_lowCrossFreq                      = 68,   // |    "mbhl.frt_lowcross 300.0"                   300.0       40.0 ~ 10000.0
    CAUD_dtsMbhlAppCtrl_midCrossFreq                      = 69,   // |    "mbhl.frt_midcross 500.0"                   500.0       40.0 ~ 10000.0

    CAUD_dtsVx_Cli_Max = CAUD_dtsMbhlAppCtrl_midCrossFreq + 1,
} CAUD_DTXVX_CLI;

//-------------------------------------------------------------------------------------------------------------------------------------
/* callback functions */
//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MS_U8   idx;      // PCM idx
    MS_U32  pts;        // PTS(unit : 90Khz clock base, max value : 0xFFFFFFFF)
    MS_U8   *pData;     // pointer to Audio Data
    MS_U32  dataLen;    // Audio Data Length
} caudPcmData_t;

typedef struct {
    MS_U32  encFormat;  // Encode format, 0:MP3, 1:AAC
    MS_U64  pts;        // PTS
    MS_U8   *pData;     // pointer to Audio Data
    MS_U32  dataLen;    // Audio Data Length
    MS_U8   *pStart;   // start pointer of buffer
    MS_U8   *pEnd;     // end pointer of buffer
} caudAencData_t;

typedef enum {
    CAUD_HDMI_MONITOR_EVENT_INVALID      = -1,
    CAUD_HDMI_MONITOR_EVENT_IS_HDMI      = 0,
    CAUD_HDMI_MONITOR_EVENT_IS_NONPCM    = 1,
    CAUD_HDMI_MONITOR_EVENT_CODEC_CHANGE = 2,
    CAUD_HDMI_MONITOR_EVENT_MAX,
} CAUD_HDMI_MONITOR_EVENT;

typedef enum {
    CAUD_HDMI_MONITOR_EVENT_INFO_INVALID      = -1,
    CAUD_HDMI_MONITOR_EVENT_INFO_DVI_MODE     = 0,
    CAUD_HDMI_MONITOR_EVENT_INFO_HDMI_MODE    = 1,
    CAUD_HDMI_MONITOR_EVENT_INFO_PCM          = 2,
    CAUD_HDMI_MONITOR_EVENT_INFO_NONPCM       = 3,
    CAUD_HDMI_MONITOR_EVENT_INFO_DD           = 4,
    CAUD_HDMI_MONITOR_EVENT_INFO_DTS          = 5,
    CAUD_HDMI_MONITOR_EVENT_INFO_DTS_HD_MA    = 10,
    CAUD_HDMI_MONITOR_EVENT_INFO_DDP          = 7,
    CAUD_HDMI_MONITOR_EVENT_INFO_DDP_ATMOS    = 13,
    CAUD_HDMI_MONITOR_EVENT_INFO_DP           = 8,
    CAUD_HDMI_MONITOR_EVENT_INFO_MAT          = 11,
    CAUD_HDMI_MONITOR_EVENT_INFO_MAT_ATMOS    = 14,
    CAUD_HDMI_MONITOR_EVENT_INFO_TRUEHD       = 12,
    CAUD_HDMI_MONITOR_EVENT_INFO_TRUEHD_ATMOS = 15, //<--MAX is here!!!
    CAUD_HDMI_MONITOR_EVENT_INFO_OTHER        = 9,
    CAUD_HDMI_MONITOR_EVENT_INFO_MAX,
} CAUD_HDMI_MONITOR_EVENT_INFO;

typedef MS_BOOL (*pfnCaudAdecoderClipDone)(CAUD_ADEC_IDX adecIdx);
typedef MS_BOOL (*pfnCaudAmixerClipDone)(CAUD_MIXER_IDX amixIdx);
typedef MS_BOOL (*pfnCaudAENCDataHandling)(caudAencData_t *pstAencData);
typedef MS_BOOL (*pfnCaudPCMDataHandling)(caudPcmData_t *pststPcmData);
typedef MS_BOOL (*pfnCaudPCMSending)(MS_U8 *pBuf, MS_U16 len);
typedef MS_BOOL (*pfnCaudMP3EncodeDone)(MS_U8 *pCopyBuf, MS_U16 copyLen);
typedef MS_BOOL (*pfnCaudAdecDecodeDone)(CAUD_ADEC_IDX adecIdx);
typedef MS_BOOL (*pfnCaudAdecEndOfStream)(CAUD_ADEC_IDX adecIdx);
typedef MS_BOOL (*pfnCaudAdecUnderrun)(CAUD_ADEC_IDX adecIdx);
typedef MS_BOOL (*pfnCaudHDMIMonitorEvent)(CAUD_HDMI_MONITOR_EVENT eventType, CAUD_HDMI_MONITOR_EVENT_INFO info);

/******************************************************************************
    (Function Declaration)
******************************************************************************/
/* Initialize, STR */
MS_BOOL Caud_InitializeModule(MS_U32 miuNo, CAUD_VIRT mad_va, caudOutputInfo_t stOutputInfo, CAUD_CHIP_PLATFORM chipPlatform);
MS_BOOL Caud_InitializeModule_IsFinished(void);
MS_BOOL Caud_Suspend(void);
MS_BOOL Caud_Resume(void);

/* Connect & Disconnect */
MS_BOOL Caud_SetParserMode(CAUD_PARSER_MODE parserMode);
MS_BOOL Caud_Adec_Connect(CAUD_ADEC_IDX currentConnect, CAUD_ADEC_INPUT inputConnect);
MS_BOOL Caud_Adec_Disconnect(CAUD_ADEC_IDX currentConnect);
MS_BOOL Caud_Adc_Connect(CAUD_ADC_IDX currentConnect, CAUD_ADC_IN_PORT portNum);
MS_BOOL Caud_Adc_Disconnect(CAUD_ADC_IDX currentConnect, CAUD_ADC_IN_PORT portNum);
MS_BOOL Caud_PcmMixer_Connect(CAUD_PCM_MIXER_IDX currentConnect, CAUD_PCM_MIXER_INPUT inputConnect);
MS_BOOL Caud_PcmMixer_Disconnect(CAUD_PCM_MIXER_IDX currentConnect, CAUD_PCM_MIXER_INPUT inputConnect);
MS_BOOL Caud_Ch_Connect(CAUD_CH_IDX currentConnect, CAUD_CH_INPUT inputConnect);
MS_BOOL Caud_Ch_Disconnect(CAUD_CH_IDX currentConnect, CAUD_CH_INPUT inputConnect);
MS_BOOL Caud_FwMixer_Connect(CAUD_FWM_IDX currentConnect, CAUD_FWM_INPUT inputConnect);
MS_BOOL Caud_FwMixer_Disconnect(CAUD_FWM_IDX currentConnect, CAUD_FWM_INPUT inputConnect);
MS_BOOL Caud_Se_Connect(CAUD_SE_IDX currentConnect, CAUD_SE_INPUT inputConnect);
MS_BOOL Caud_Se_Disconnect(CAUD_SE_IDX currentConnect, CAUD_SE_INPUT inputConnect);
MS_BOOL Caud_Sndout_Connect(CAUD_SNDOUT_IDX currentConnect, CAUD_SNDOUT_INPUT inputConnect);
MS_BOOL Caud_Sndout_Disconnect(CAUD_SNDOUT_IDX currentConnect, CAUD_SNDOUT_INPUT inputConnect);
MS_BOOL Caud_PcmCapture_Connect(CAUD_PCM_CAPTURE_IDX currentConnect, CAUD_PCM_CAPTURE_INPUT inputConnect);
MS_BOOL Caud_PcmCapture_Disconnect(CAUD_PCM_CAPTURE_IDX currentConnect, CAUD_PCM_CAPTURE_INPUT inputConnect);
MS_BOOL Caud_Mp3Enc_Connect(CAUD_MP3_ENC_INPUT inputConnect);
MS_BOOL Caud_Mp3Enc_Disconnect(CAUD_MP3_ENC_INPUT inputConnect);
MS_BOOL Caud_AacEnc_Connect(CAUD_AAC_ENC_INPUT inputConnect);
MS_BOOL Caud_AacEnc_Disconnect(CAUD_AAC_ENC_INPUT inputConnect);

/* Start & Stop */
MS_BOOL Caud_SetCodec(CAUD_ADEC_IDX adecIdx, CAUD_CODEC_TYPE codec);
MS_BOOL Caud_StartDecoding(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_StopDecoding(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_PauseDecoding(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_SetMainDecoder(CAUD_ADEC_IDX adecIdx);

/* SPDIF */
MS_BOOL Caud_SpdifTx_SetOutputType(CAUD_SPDIF_TX_TYPE spdifOutputType);
CAUD_SPDIF_TX_TYPE Caud_SpdifTx_GetOutputType(void);
MS_BOOL Caud_SpdifTx_SetCopyRight(CAUD_SPDIF_COPYRIGHT copyRight);
MS_BOOL Caud_SpdifTx_SetCategory(MS_U8 categoryCode);
MS_BOOL Caud_SpdifTx_SetLight(MS_BOOL bOnOff);
MS_BOOL Caud_SpdifTx_SetMonitor(MS_BOOL bOnOff);
MS_BOOL Caud_SpdifTx_SetChStatus(CAUD_SPDIF_CS_TYPE csMode, CAUD_SPDIF_CS_TYPE_STATUS status);
MS_BOOL Caud_SpdifRx_GetCodec(CAUD_CODEC_TYPE *pSpdifRxCodec);
MS_BOOL Caud_SpdifRx_SetMonitor(MS_BOOL bOnOff);

/* HDMI */
MS_BOOL Caud_HdmiRx_GetCodec(CAUD_CODEC_TYPE *pHdmiCodec);
MS_BOOL Caud_HdmiRx_GetCopyRight(CAUD_SPDIF_COPYRIGHT *pCopyRight);
MS_BOOL Caud_HdmiRx_SetArc(MS_BOOL bOnOff);
MS_BOOL Caud_HdmiRx_SetMonitor(MS_BOOL bOnOff);
MS_BOOL Caud_HdmiTx_SetOutputType(CAUD_HDMI_TX_TYPE hdmiOutputType);
MS_BOOL Caud_HdmiTx_SetMonitor(MS_BOOL bOnOff);

/* Digital Output */
MS_BOOL Caud_DigitalOut_SetDeviceCapability(CAUD_DIGITAL_OUTPUT_TYPE outType, CAUD_DIGITALOUT_CODEC codec, caudDigitalOutCapability_t *pstCodecCapability);

/* ATV */
MS_BOOL Caud_Sif_SetInputSrc(CAUD_SIF_INPUT sifSrc);
MS_BOOL Caud_Sif_SetHighDevMode(CAUD_SIF_HIDEV_BW bw, MS_BOOL bOnOff);
MS_BOOL Caud_Sif_SetStd(CAUD_SIF_STD sifStd);
MS_BOOL Caud_Sif_SetSndMode(CAUD_SIF_AUDIOMODE_TYPE sifSndMode);
CAUD_SIF_AUDIOMODE_TYPE Caud_Sif_GetSndMode(void);
MS_BOOL Caud_Sif_SetMonitor(MS_BOOL bOnOff);
MS_BOOL Caud_Sif_SetAutoMute(MS_BOOL bOnOff);
CAUD_SIF_STD Caud_Sif_StartAutoStdDetection(void);
MS_BOOL Caud_Sif_SetThreshold(caudSifThrTbl_t *pstThrTbl);
MS_BOOL Caud_Sif_SetPalType(CAUD_SIF_PAL_TYPE pal_type);
MS_BOOL Caud_Sif_SendCmd(CAUD_SIF_CMD_TYPE sifCmd, MS_U8 cmd1, MS_U8 cmd2);
MS_BOOL Caud_Sif_GetStatus(CAUD_SIF_AUDIOSTATUS *pCurrentAudioStatus);
MS_BOOL Caud_Sif_IsPalType(CAUD_SIF_PAL_TYPE palType);
MS_BOOL Caud_Sif_SetPrescale(CAUD_SIF_GAIN_TYPE gainType, MS_S32 db);        // db: The prescale value, unit is 0.25dB/Step.
                                                                             // 0: 0db, 1: 0.25dB,  2: 0.5dB, ...,  4: 1.0dB, ...,  8: 2dB
                                                                             //        -1:-0.25dB, -2:-0.5dB, ..., -4:-1.0dB, ..., -8:-2dB
MS_BOOL Caud_Sif_SetBand(MS_U8 SifBand);
MS_BOOL Caud_Sif_SetBtscA2StereoLvl(MS_U16 thrLvl);
MS_BOOL Caud_Sif_IsExist(void);

/* Decoder */
MS_BOOL Caud_SetSync(CAUD_ADEC_IDX adecIdx, MS_BOOL bOnOff);
MS_BOOL Caud_IsEsExist(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_SetAd(CAUD_ADEC_IDX adecIdx, MS_BOOL bOnOff);
MS_BOOL Caud_SetTrick(CAUD_ADEC_IDX adecIdx, CAUD_TRICK_MODE eTrickMode);
MS_BOOL Caud_SetAdecPcmPath(CAUD_ADEC_IDX adecIdx, CAUD_ADEC_PCM_PATH pcmPath);
MS_BOOL Caud_Adec_RegisterDecodeDoneCallback(CAUD_ADEC_IDX adecIdx, pfnCaudAdecDecodeDone pfnDecodeDoneCallBack);
MS_BOOL Caud_Adec_RegisterEosCallback(CAUD_ADEC_IDX adecIdx, pfnCaudAdecEndOfStream pfnEndOfStreamCallBack);
MS_BOOL Caud_Adec_RegisterUnderrunCallback(CAUD_ADEC_IDX adecIdx, pfnCaudAdecUnderrun pfnUnderrunCallBack);
MS_BOOL Caud_SetAutoRecovery(MS_BOOL bOnOff);
MS_BOOL Caud_AttachVdpSyncPattern(CAUD_CODEC_TYPE codec, void *pDst, MS_U32 esSize, void *pExtra);

/* Common */
MS_BOOL Caud_SetParam(CAUD_ADEC_IDX adecIdx, CAUD_PARAM_TYPE paramType, void *pParam);
MS_BOOL Caud_GetInfo(CAUD_ADEC_IDX adecIdx, CAUD_INFO_TYPE infoType, void *pInfo);

/* Customized patch */
MS_BOOL Caud_Patch_SetParam(CAUD_ADEC_IDX adecIdx, CAUD_PATCH_PARAM_TYPE paramType, void *pParam);
MS_BOOL Caud_Patch_GetInfo(CAUD_ADEC_IDX adecIdx, CAUD_PATCH_INFO_TYPE infoType,  void *pInfo);

/* Clip Play for ES */
MS_BOOL Caud_PlayClipDecoder(CAUD_ADEC_IDX adecIdx, caudClipDecoder_t stClipDecoder, MS_U32 bufSize, void *pBufClip, pfnCaudAdecoderClipDone pfnCallBack);
MS_BOOL Caud_StopClipDecoder(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_PauseClipDecoder(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_ResumeClipDecoder(CAUD_ADEC_IDX adecIdx);

/* Clip Play for PCM */
MS_BOOL Caud_PlayClipMixer(CAUD_MIXER_IDX mixerIdx, caudClipMixer_t stClipMixer, MS_U32 bufSize, void *pBufClip, pfnCaudAmixerClipDone pfnCallBack);
MS_BOOL Caud_StopClipMixer(CAUD_MIXER_IDX mixerIdx);
MS_BOOL Caud_PauseClipMixer(CAUD_MIXER_IDX mixerIdx);
MS_BOOL Caud_ResumeClipMixer(CAUD_MIXER_IDX mixerIdx);

/* Direct Clip Play */
caudDirect_t *Caud_Direct_Open(CAUD_ADEC_IDX adecIdx, CAUD_MIXER_IDX mixerIdx);
MS_BOOL Caud_Direct_Close(caudDirect_t *pstDirect);
MS_BOOL Caud_Direct_Start(caudDirect_t *pstDirect, CAUD_CODEC_TYPE codec, MS_U32 sampleRate, MS_U32 chNum, MS_U32 bitsPerSample);
MS_BOOL Caud_Direct_Stop(caudDirect_t *pstDirect);
MS_BOOL Caud_Direct_Pause(caudDirect_t *pstDirect);
MS_BOOL Caud_Direct_Resume(caudDirect_t *pstDirect);
MS_BOOL Caud_Direct_SetDelay(caudDirect_t *pstDirect, MS_U32 bOnOff, MS_U32 upperThreshold, MS_U32 lowerThreshold);
MS_BOOL Caud_Direct_Write(caudDirect_t *pstDirect, void *pBuf, MS_U32 bufSize);
MS_BOOL Caud_Direct_Flush(caudDirect_t *pstDirect);

/* Gain, Mute & Delay */
//---Gain---
MS_BOOL Caud_SetAdGain(CAUD_ADEC_IDX adecIdx, CAUD_ADMIX_GAIN_IDX adMixGainIdx, MS_U32 gain);
MS_BOOL Caud_SetPcmMixerGain(CAUD_MIXER_IDX mixerIdx, MS_U32 gain);
MS_BOOL Caud_SetChGain(CAUD_CH_IDX ch, MS_U32 gain);
MS_BOOL Caud_SetFwMixerChGain(CAUD_FWM_IDX currentConnect, CAUD_FWM_INPUT inputConnect, MS_U32 gain);
MS_BOOL Caud_SetI2SOutGain(MS_U32 gain);
MS_BOOL Caud_SetLineOutGain(CAUD_LINEOUT_IDX lineIdx, MS_U32 gain);
MS_BOOL Caud_SetSpdifOutGain(MS_U32 gain);
MS_BOOL Caud_SetArcOutGain(MS_U32 gain);
MS_BOOL Caud_SetHdmiOutGain(MS_U32 gain);
//---Mute---
MS_BOOL Caud_SetAdecMute(CAUD_ADEC_IDX adecIdx, MS_BOOL bOnOff);
MS_BOOL Caud_SetPcmMixerMute(CAUD_MIXER_IDX mixerIdx, MS_BOOL bOnOff);
MS_BOOL Caud_SetChMute(CAUD_CH_IDX ch, MS_BOOL bOnOff);
MS_BOOL Caud_SetFwMixerChMute(CAUD_FWM_IDX currentConnect, CAUD_FWM_INPUT inputConnect, MS_BOOL bOnOff);
MS_BOOL Caud_SetI2SOutMute(MS_BOOL bOnOff);
MS_BOOL Caud_SetLineOutMute(CAUD_LINEOUT_IDX lineIdx, MS_BOOL bOnOff);
MS_BOOL Caud_SetSpdifOutMute(MS_BOOL bOnOff);
MS_BOOL Caud_SetArcOutMute(MS_BOOL bOnOff);
MS_BOOL Caud_SetHdmiOutMute(MS_BOOL bOnOff);
MS_BOOL Caud_SetInputMuteDuringLimitedTime(MS_U32 per50ms, CAUD_FWM_IDX fwm, CAUD_FWM_INPUT input);
MS_BOOL Caud_SetOutputMuteDuringLimitedTime(MS_U32 per50ms, CAUD_OUTPUT_TYPE output);
//---Delay---
MS_BOOL Caud_SetAdecDelay(CAUD_ADEC_IDX adecIdx, MS_U32 delay);
MS_BOOL Caud_SetChDelay(CAUD_CH_IDX ch, MS_U32 delay);
MS_BOOL Caud_SetAudioDelay(MS_U32 delay);
MS_BOOL Caud_SetI2SOutDelay(MS_U32 delay);
MS_BOOL Caud_SetLineOutDelay(CAUD_LINEOUT_IDX lineIdx, MS_U32 delay);
MS_BOOL Caud_SetSpdifOutDelay(MS_U32 delay);
MS_BOOL Caud_SetArcOutDelay(MS_U32 delay);
MS_BOOL Caud_SetHdmiOutDelay(MS_U32 delay);

/* AENC */
MS_BOOL Caud_Aenc_Start(CAUD_AENC_FORMAT encFormat);
MS_BOOL Caud_Aenc_Stop(CAUD_AENC_FORMAT encFormat);
MS_BOOL Caud_Aenc_RegisterCallback(CAUD_AENC_FORMAT encFormat, pfnCaudAENCDataHandling pfnCallBack);
MS_BOOL Caud_Aenc_SetInfo(CAUD_AENC_FORMAT encFormat, caudAenc_t stAenc);
MS_BOOL Caud_Aenc_GetInfo(CAUD_AENC_FORMAT encFormat, caudAenc_t *pstAenc);
MS_BOOL Caud_Aenc_SetGain(CAUD_AENC_FORMAT encFormat, MS_U32 gain);
MS_BOOL Caud_Aenc_CopyData(CAUD_AENC_FORMAT encFormat, MS_U8 *pDest, MS_U8 *pBufAddr, MS_U32 datasize, MS_U8 *pRStart, MS_U8 *pREnd);
MS_BOOL Caud_Aenc_ReleaseData(CAUD_AENC_FORMAT encFormat, MS_U8 *pBufAddr, MS_U32 datasize);

/* PCM Capture */
MS_BOOL Caud_PcmCapture_StartUpload(CAUD_PCM_CAPTURE_IDX currentConnect);
MS_BOOL Caud_PcmCapture_StopUpload(CAUD_PCM_CAPTURE_IDX currentConnect);
MS_BOOL Caud_PcmCapture_RegisterCallback(CAUD_PCM_CAPTURE_IDX currentConnect, pfnCaudPCMSending pfnCallBack);
MS_BOOL Caud_PcmCapture_SetGain(CAUD_PCM_CAPTURE_IDX currentConnect, MS_U32 gain);
MS_BOOL Caud_PcmCapture_SetMute(CAUD_PCM_CAPTURE_IDX currentConnect, MS_BOOL bOnOff);
MS_BOOL Caud_PcmCapture_SetRequestMs(MS_U32 requestMs);

/* PCM IO Control */
MS_S32  Caud_CommonPcm_Open(caudPcmio_t *pstPcmio);
MS_S32  Caud_CommonPcm_Close(MS_S32 deviceId);
MS_S32  Caud_CommonPcm_Start(MS_S32 deviceId);
MS_S32  Caud_CommonPcm_Stop(MS_S32 deviceId);
MS_S32  Caud_CommonPcm_Set(MS_S32 deviceId, MS_U32 cmd, void *pData);
MS_S32  Caud_CommonPcm_Get(MS_S32 deviceId, MS_U32 cmd, void *pData);
MS_U32  Caud_CommonPcm_Read(MS_S32 deviceId, void *pBuf, MS_U32 size);
MS_U32  Caud_CommonPcm_Write(MS_S32 deviceId, const void *pBuf, MS_U32 size);
MS_S32  Caud_CommonPcm_Flush(MS_S32 deviceId);

/* MM New Mode */
MS_PHY  Caud_GetDdrInfo(CAUD_ADEC_IDX adecIdx, CAUD_DDR_INFO ddrInfo);
MS_BOOL Caud_Mm2_InitAesInfo(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_Mm2_CheckAesInfo(CAUD_ADEC_IDX adecIdx, caudAes_t *pstAes);
MS_BOOL Caud_Mm2_InputAesFinished(CAUD_ADEC_IDX adecIdx, MS_U32 esSize, MS_BOOL ptsExist, MS_U64 pts);
MS_BOOL Caud_Mm2Ad_InitAesInfo(CAUD_ADEC_IDX adecIdx);
MS_BOOL Caud_Mm2Ad_CheckAesInfo(CAUD_ADEC_IDX adecIdx, caudAes_t *pstAes);
MS_BOOL Caud_Mm2Ad_InputAesFinished(CAUD_ADEC_IDX adecIdx, MS_U32 esSize, MS_BOOL ptsExist, MS_U64 pts, MS_U8 fade, MS_U8 pan); //fade: 0x0~0xFE, pan: 0xEB~0x15

/* Basic Sound Effect */
MS_BOOL Caud_BasicSe_Enable(CAUD_BASIC_SE_TYPE type, MS_BOOL bOnOff);

///@brief \b Function \b Name: Caud_BasicSe_SetGeq()
///@brief \b Function \b Description: This routine is used to set the the 5-band GEQ u8Level
///@param <IN> \b u8band    :  GEQ band  0~4 // 0:120Hz, 1:500Hz, 2:1500Hz, 3:5000Hz, 4:10000Hz
///@param <IN> \b s8level   :  GEQ level -48~48(-12dB~12dB, step 0.25dB)
MS_BOOL Caud_BasicSe_SetGeq(MS_U8 band, MS_S8 lvl);

//@brief \b Function \b Name: Caud_BasicSe_SetPeqCoef()
///@brief \b Function \b Description: This routine is used to set the PEQ parameters
///@param <IN> \b peq_coef->u8band       :  PEQ band select ( 0 ~ 12 )
///@param <IN> \b peq_coef->bEnable      :  PEQ band enable/disable ( 0:disable  1:enable )
///@param <IN> \b peq_coef->u8Type       :  PEQ type ( 0:peak, 1:high pass, 2:low pass, 3:high shelving, 4:low shelving )
///@param <IN> \b peq_coef->u16Gain      :  PEQ gain ( 0 ~ 360 => -18db~+18db, step 0.1dB)
///@param <IN> \b peq_coef->u16Frequency :  PEQ Center Frequency ( 20 ~ 20000 )
///@param <IN> \b peq_coef->u16QValue    :  PEQ Q Value ( 5 ~ 160 )
MS_BOOL Caud_BasicSe_SetPeqCoef(caudBasicSePeqCoef_t *pstPeqCoef);

///@brief \b Function \b Name: Caud_BasicSe_SetHpfCoef()
///@brief \b Function \b Description: This routine is used to set the HPF parameters
///@param <IN> \b hpf_coef->u8band       :  HPF band ( 13 )
///@param <IN> \b hpf_coef->bEnable      :  HPF enable/disable ( 0:disable  1:enable )
///@param <IN> \b hpf_coef->u8Type       :  HPF type ( 1 )
///@param <IN> \b hpf_coef->u16Gain      :  HPF gain ( 0 ~ 360 => -18db~+18db, step 0.1dB. It is usually 180 )
///@param <IN> \b hpf_coef->u16Frequency :  HPF Cut Off Frequency ( 20 ~ 20000 )
///@param <IN> \b hpf_coef->u16QValue    :  HPF Q Value ( 50 ~ 90 )
MS_BOOL Caud_BasicSe_SetHpfCoef(caudBasicSePeqCoef_t *pstHpfCoef);

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: Caud_BasicSe_SetAvcParam()
/// @brief \b Function \b Description: This routine is used to set the AVC parameters.
/// @param <IN>        \b stAvcParamu.32TargetLevel    :0x1000~0x7FFFFF (-48~18 dB)
/// @param <IN>        \b stAvcParamu.u32AttackStepDb  :0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
/// @param <IN>        \b stAvcParamu.u32ReleaseStepDb :0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
////////////////////////////////////////////////////////////////////////////////
MS_BOOL Caud_BasicSe_SetAvcParam(caudBasicSeAvcParam_t stAvcParam);

///@brief \b Function \b Name: Caud_BasicSe_SetDrcParam()
///@brief \b Function \b Description: This routine is used to set the DRC clipping level
///@param <IN>        \b stDrcParam.u8Band           :0: BAND_LOW, 2: BAND_HIGH, 3: BAND_ALL
///@param <IN>        \b stDrcParam.32TargetLevel    :0x1000~0x7FFFFF (-48~18 dB)
///@param <IN>        \b stDrcParam.u32AttackStepDb  :0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
///@param <IN>        \b stDrcParam.u32ReleaseStepDb :0x7FFFFF~0x400 (1.931872658 ms ~ 15825.90081 ms)
///@param <IN>        \b stDrcParam.u8ProcMode       :0, 1
MS_BOOL Caud_BasicSe_SetDrcParam(caudBasicSeDrcParam_t stDrcParam);

///@brief \b Function \b Name: Caud_BasicSe_SetBalance()
///@brief \b Function \b Description: This routine is used to set the volume balance
///@param <IN> \b Lbalance    :  balance L  0x00(0dB)~0xFF(mute)  step: -0.25dB
///@param <IN> \b Lbalance    :  balance R  0x00(0dB)~0xFF(mute)  step: -0.25dB
MS_BOOL Caud_BasicSe_SetBalance(MS_U16 lBalance, MS_U16 rBalance);

///@brief \b Function \b Name: Caud_BasicSe_SetNrThreshold()
///@brief \b Function \b Description: This routine is used to set noise reduction threshold
///@param <IN> \b u16level    :  NR level
MS_BOOL Caud_BasicSe_SetNrThreshold(MS_U16 lvl);

///@brief \b Function \b Name: Caud_BasicSe_GetNrStatus()
///@brief \b Function \b Description: This routine is used to get noise reduction status
///@param <IN> \b u16level    :  NR level
MS_S32 Caud_BasicSe_GetNrStatus(void);

/* Advanced Sound Effect */
MS_BOOL Caud_DtsSe_Enable(CAUD_DTSSE_TYPE seType);
MS_BOOL Caud_DtsSe_SetUnit(CAUD_DTSSE_UNIT seUnit, MS_BOOL bOnOff);
MS_BOOL Caud_DtsSe_SetParam(CAUD_DTSSE_PARAM seParam, MS_U32 value);
MS_BOOL Caud_DtsSe_SetAeqCoef(caudDtsSeAeqCoef_t *pstAeqCoef);
MS_BOOL Caud_DolbyDap_Enable(MS_BOOL bOnOff);
MS_BOOL Caud_DolbyDap_SetUnit(MS_BOOL bOnOff);
MS_BOOL Caud_DolbyDap_SetParam(caudDolbyDapParam_t stDapParam);
MS_BOOL Caud_DolbyDap_SetBinParam(void *pParam, MS_U32 size);
MS_BOOL Caud_SonicEmotionAbs3d_Enable(MS_BOOL bOnOff);
MS_BOOL Caud_SonicEmotionAbs3d_SetUnit(MS_BOOL bOnOff);
MS_BOOL Caud_SonicEmotionAbs3d_SetParam(caudSonicEmotionAbs3dParam_t stAbs3dParam);
MS_BOOL Caud_DtsVx_Enable(MS_BOOL bOnOff);
MS_BOOL Caud_DtsVx_SetParamCli(CAUD_DTXVX_CLI dtsVxCli, char *cli);

/* Debug */
void Caud_DbgMenu(void *pVoid, MS_BOOL bSupportScanf);
void Caud_ParseDbgCmd(char *paramsStr);
void Caud_ExtLog(const char *format, ...);
void Caud_ExtPrint(const char *format, ...);
void Caud_ExtMonitorLog(const char *format, ...);

//-----------------------------------------------------------------------------------
// [PURPOSE] for SET
//-----------------------------------------------------------------------------------
typedef struct {
    MS_U32 dbgParam;
    MS_U32 dbgParam2;
    MS_U32 dbgParam3;
    MS_U32 dbgParam4;
    MS_U32 dbgParam5;
    MS_U32 dbgParam6;
    MS_U32 dbgParam7;
    MS_U32 dbgParam8;
    MS_U32 dbgParam9;
    MS_U32 dbgParam10;
} caudDbgParam_t;

//-----------------------------------------------------------------------------------
// [PURPOSE] for GET
//-----------------------------------------------------------------------------------
typedef struct {
    MS_U32 dbgInfo;
    MS_U32 dbgInfo2;
    MS_U32 dbgInfo3;
    MS_U32 dbgInfo4;
    MS_U32 dbgInfo5;
    MS_U32 dbgInfo6;
    MS_U32 dbgInfo7;
    MS_U32 dbgInfo8;
    MS_U32 dbgInfo9;
    MS_U32 dbgInfo10;
} caudDbgInfo_t;


//Header-free api structure
typedef struct {
    const char *name;
    void *pData;
} caudTunnel_t;

#endif //New APIs

#ifdef __cplusplus
}
#endif

#endif // _API_AUDIO_CUSTOMER_H_
