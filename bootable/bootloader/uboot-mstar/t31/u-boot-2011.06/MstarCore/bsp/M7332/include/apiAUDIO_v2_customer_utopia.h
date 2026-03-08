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

#if defined(AUDIO_CWRAP) || defined(ROLLS_ROYCE) || defined(AUDIO_CWRAP_DBG)

#ifndef _AUDIO_V2_CUSTOMER_H_
#define _AUDIO_V2_CUSTOMER_H_

#include "MsTypes.h"
#include "drvAUDIO_if.h"
#include "apiAUDIO_v2_customer_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
//  Enum / Struct
//-------------------------------------------------------------------------------------------------
///< Select AD output type
enum
{
    AD_OUT_SPEAKER, ///< AD out in speaker only
    AD_OUT_HP,      ///< AD out in HP only
    AD_OUT_BOTH,    ///< AD out both in speaker & HP
    AD_OUT_NONE     ///< no AD out
};

///< Audio source switch group
typedef enum
{
    E_AUDIO_GROUP_MAIN,     ///< Main speaker, HP, Line-out , S/PDIF PCM mode
    E_AUDIO_GROUP_SCART,    ///< SCART
    E_AUDIO_GROUP_SPDIF,    ///< SPDIF non-pcm
    E_AUDIO_GROUP_SUB,      ///< SUB source for PIP application
    E_AUDIO_GROUP_INVALID
} AUDIO_SWITCH_GROUP;

///< Decoder command type
typedef enum
{
    MSAPI_AUD_DVB_DECCMD_STOP = 0x0,        ///< 1st DSP stop
    MSAPI_AUD_DVB_DECCMD_PLAY,              ///< 1st DSP play
    MSAPI_AUD_DVB_DECCMD_RESYNC,
    MSAPI_AUD_DVB_DECCMD_FREE_RUN,
    MSAPI_AUD_DVB_DECCMD_AVSYNC,
    MSAPI_AUD_DVB_DECCMD_PLAYFILE,          ///< 1st DSP play file
    MSAPI_AUD_DVB_DECCMD_PAUSE,             ///< 1st DSP pause
    MSAPI_AUD_DVB_DECCMD_PLAYFILETSP,       ///< 1st DSP play TS file
    MSAPI_AUD_DVB_DECCMD_STARTBROWSE,
    MSAPI_AUD_DVB_DECCMD_PLAYFRAME,         ///<1st DSP play MM (for uniplayer/Mstreamer)
    MSAPI_AUD_DVB_DECCMD_PLAYFRAME_STAGE,   ///<1st DSP play MM (for stagecraft)
    MSAPI_AUD_DVB_DECCMD_PLAYFRAME_GS,      ///<1st DSP play MM (for uniplayer/Gstreamer ES copy)
    MSAPI_AUD_DVB_DECCMD_PLAYMM_FILE2,
    MSAPI_AUD_DVB_DECCMD_PLAY_AD,
    MSAPI_AUD_DVB_DECCMD_STOP_AD,
    // SE DSP
    MSAPI_AUD_DVB2_DECCMD_STOP=0x10,        ///< 2nd DSP stop
    MSAPI_AUD_DVB2_DECCMD_PLAY,             ///< 2nd DSP play
    MSAPI_AUD_DVB2_DECCMD_RESYNC,
    MSAPI_AUD_DVB2_DECCMD_FREE_RUN,
    MSAPI_AUD_DVB2_DECCMD_AVSYNC,
    MSAPI_AUD_DVB2_DECCMD_PLAYFILE,         ///< 2nd DSP play file
    MSAPI_AUD_DVB2_DECCMD_PAUSE,            ///< 2nd DSP pause
    MSAPI_AUD_DVB2_DECCMD_PLAYFILETSP       ///< 2nd DSP play TS file

} En_DVB_decCmdType;

///< ATV SIF Command
typedef enum
{
    MSAPI_AUD_SIF_CMD_SET_STOP                   = 0x00,
    MSAPI_AUD_SIF_CMD_SET_PLAY                   = 0x01,
    MSAPI_AUD_SIF_CMD_ENABLE_HIDEV               = 0x02,
    MSAPI_AUD_SIF_CMD_SET_HIDEV_FILTER_BW_LEVEL  = 0x03,
    MSAPI_AUD_SIF_CMD_RESET_FC_TRACKING          = 0x04,
    MSAPI_AUD_SIF_CMD_ENABLE_FC_TRACKING         = 0x05,
    MSAPI_AUD_SIF_CMD_SET_ADC_FROM_VIF_PATH      = 0x06,
    MSAPI_AUD_SIF_CMD_ENABLE_AUTO_MUTE           = 0x07,
    MSAPI_AUD_SIF_CMD_ENABLE_BEEPER_FUNCTION     = 0x08,
    MSAPI_AUD_SIF_CMD_SET_BEEPER_TONE            = 0x09,
    MSAPI_AUD_SIF_CMD_ENABLE_AGC                 = 0x0A,
    MSAPI_AUD_SIF_CMD_RESET_AGC                  = 0x0B,
    MSAPI_AUD_SIF_STANDARD_RESET                 = 0x0C,
    MSAPI_AUD_SIF_CMD_DETECT_MAIN_STD_ONLY       = 0x0D,
    MSAPI_AUD_SIF_CMD_ENABLE_AUTO_SOUNDMODE      = 0x0E,
    MSAPI_AUD_SIF_CMD_FMTX_SET_PREEMP            = 0x0F,
    MSAPI_AUD_SIF_CMD_SET_ENC_CMD                = 0x10,
    MSAPI_AUD_SIF_CMD_SET_ENC_Input_attenuation  = 0x11,
    MSAPI_AUD_SIF_CMD_SET_ENC_Output_scaling     = 0x12,
    MSAPI_AUD_SIF_CMD_SET_BTSC_Enc_M_Gain        = 0x13,
    MSAPI_AUD_SIF_CMD_SET_BTSC_Enc_D_Gain        = 0x14,
    MSAPI_AUD_SIF_CMD_SET_BTSC_Enc_SAP_Gain      = 0x15,
    MSAPI_AUD_SIF_CMD_SET_FM_CH2_FILTER_BW_LEVEL = 0x16,
    MSAPI_AUD_SIF_CMD_SET_EU_MODE                = 0x17,
    MSAPI_AUD_SIF_CMD2_ADC_VIA_VIF_PIN           = 0x01
} En_AUD_SIF_CmdType;

///< Audio SIF Standard Type
typedef enum
{
    E_AUDIOSTANDARD_BG          = 0x00,         ///< Audio standard BG
    E_AUDIOSTANDARD_BG_A2       = 0x01,         ///< Audio standard BG A2
    E_AUDIOSTANDARD_BG_NICAM    = 0x02,         ///< Audio standard BG NICAM
    E_AUDIOSTANDARD_I           = 0x03,         ///< Audio standard I
    E_AUDIOSTANDARD_DK          = 0x04,         ///< Audio standard DK
    E_AUDIOSTANDARD_DK1_A2      = 0x05,         ///< Audio standard DK1 A2
    E_AUDIOSTANDARD_DK2_A2      = 0x06,         ///< Audio standard DK2 A2
    E_AUDIOSTANDARD_DK3_A2      = 0x07,         ///< Audio standard DK3 A2
    E_AUDIOSTANDARD_DK_NICAM    = 0x08,         ///< Audio standard DK NICAM
    E_AUDIOSTANDARD_L           = 0x09,         ///< Audio standard L
    E_AUDIOSTANDARD_M           = 0x0A,         ///< Audio standard M
    E_AUDIOSTANDARD_M_BTSC      = 0x0B,         ///< Audio standard M BTSC
    E_AUDIOSTANDARD_M_A2        = 0x0C,         ///< Audio standard M A2
    E_AUDIOSTANDARD_M_EIA_J     = 0x0D,         ///< Audio standard M EIA J
    E_AUDIOSTANDARD_NOTSTANDARD = 0x0F          ///< Not Audio standard
} AUDIOSTANDARD_TYPE;

///< sif carrier status define
typedef enum
{
    // Never change the value of enumeration. Because each bit has weight.
    E_STATE_AUDIO_NO_CARRIER           = 0x00, ///< No carrier detect
    E_STATE_AUDIO_PRIMARY_CARRIER      = 0x01, ///< Carrier 1 exist
    E_STATE_AUDIO_SECONDARY_CARRIER    = 0x02, ///< Carrier 2 exist
    E_STATE_AUDIO_NICAM                = 0x04, ///< Nicam lock state
    E_STATE_AUDIO_STEREO               = 0x08, ///< A2 Stereo exist
    E_STATE_AUDIO_BILINGUAL            = 0x10, ///< A2 Dual exist
    E_STATE_AUDIO_PILOT                = 0x20, ///< A2 Pilot exist
    E_STATE_AUDIO_DK2                  = 0x40, ///< Sound standard is DK2
    E_STATE_AUDIO_DK3                  = 0x80  ///< Sound standard is DK3
} AUDIOSTATUS;

///< Audio mode type
typedef enum
{
    E_AUDIOMODE_INVALID       = 0x00,  ///< Audio Mode Invalid
    E_AUDIOMODE_MONO          = 0x01,  ///< Audio Mode MONO
    E_AUDIOMODE_FORCED_MONO   = 0x02,  ///< Audio Mode Forced MONO
    E_AUDIOMODE_G_STEREO      = 0x03,  ///< Audio Mode G Stereo
    E_AUDIOMODE_K_STEREO      = 0x04,  ///< Audio Mode  K Stereo
    E_AUDIOMODE_MONO_SAP      = 0x05,  ///< Audio Mode MONO SAP
    E_AUDIOMODE_STEREO_SAP    = 0x06,  ///< Audio Mode Stereo SAP
    E_AUDIOMODE_DUAL_A        = 0x07,  ///< Audio Mode Dual A
    E_AUDIOMODE_DUAL_B        = 0x08,  ///< Audio Mode Dual B
    E_AUDIOMODE_DUAL_AB       = 0x09,  ///< Audio Mode Dual AB
    E_AUDIOMODE_NICAM_MONO    = 0x0A,  ///< Audio Mode NICAM MONO
    E_AUDIOMODE_NICAM_STEREO  = 0x0B,  ///< Audio Mode NICAM Stereo
    E_AUDIOMODE_NICAM_DUAL_A  = 0x0C,  ///< Audio Mode NICAM DUAL A
    E_AUDIOMODE_NICAM_DUAL_B  = 0x0D,  ///< Audio Mode NICAM DUAL B
    E_AUDIOMODE_NICAM_DUAL_AB = 0x0E,  ///< Audio Mode NICAM DUAL AB
    E_AUDIOMODE_HIDEV_MONO    = 0x0F,  ///< Audio Mode HIDEV MONO
    E_AUDIOMODE_LEFT_LEFT     = 0x10,  ///< Audio Mode Left left
    E_AUDIOMODE_RIGHT_RIGHT   = 0x11,  ///< Audio Mode right right
    E_AUDIOMODE_LEFT_RIGHT    = 0x12   ///< Audio Mode left right
} AUDIOMODE_TYPE;

typedef enum
{
    HDMITX_AUDIO_FREQ_NO_SIG  = 0,
    HDMITX_AUDIO_32K          = 1,
    HDMITX_AUDIO_44K          = 2,
    HDMITX_AUDIO_48K          = 3,
    HDMITX_AUDIO_88K          = 4,
    HDMITX_AUDIO_96K          = 5,
    HDMITX_AUDIO_176K         = 6,
    HDMITX_AUDIO_192K         = 7,
    HDMITX_AUDIO_FREQ_MAX_NUM = 8,
} HDMITX_AUDIO_FREQUENCY;

typedef enum
{
    HDMITX_AUDIO_PCM        = 0, // PCM
    HDMITX_AUDIO_NONPCM     = 1, // non-PCM
} HDMITX_AUDIO_CODING_TYPE;

typedef enum
{
    HDMITX_AUDIO_CH_2  = 2, // 2 channels
    HDMITX_AUDIO_CH_8  = 8, // 8 channels
} HDMITX_AUDIO_CHANNEL_COUNT;

typedef enum
{
    HDMITX_AUDIO_FORMAT_PCM   = 0,
    HDMITX_AUDIO_FORMAT_DSD   = 1,
    HDMITX_AUDIO_FORMAT_HBR   = 2,
    HDMITX_AUDIO_FORMAT_NA    = 3,
} HDMITX_AUDIO_SOURCE_FORMAT;

typedef enum
{
    E_HDCP_DISABLE  = 0, // HDCP disable
    E_HDCP_FAIL     = 1, // HDCP fail
    E_HDCP_PASS     = 2, // HDCP pass
} HDMITX_HDCP_STATUS;

//need to be consistent with VDEC_EX_Result of apiVDEC_EX.h
//function return enumerator
typedef enum
{
    ///failed
    E_VDEC_EX_FAIL = 0,
    ///success
    E_VDEC_EX_OK,
    ///invalid parameter
    E_VDEC_EX_RET_INVALID_PARAM,
    ///access not allow
    E_VDEC_EX_RET_ILLEGAL_ACCESS,
    ///hardware abnormal
    E_VDEC_EX_RET_HARDWARE_BREAKDOWN,
     ///unsupported
    E_VDEC_EX_RET_UNSUPPORTED,
     ///timeout
    E_VDEC_EX_RET_TIMEOUT,
    ///not ready
    E_VDEC_EX_RET_NOT_READY,
    ///not initial
    E_VDEC_EX_RET_NOT_INIT,
    ///not exit after last initialization
    E_VDEC_EX_RET_NOT_EXIT,
    ///not running, counter does not change
    E_VDEC_EX_RET_NOT_RUNNING,
    ///max value
    E_VDEC_EX_RET_NUM,
} VDEC_EX_Result;

//need to be consistent with VDEC_StreamId of apiVDEC_EX.h
typedef struct
{
    MS_U32 u32Version;
    MS_U32 u32Id;
} VDEC_StreamId;

//need to be consistent with VDEC_EX_User_Cmd of apiVDEC_EX.h
//VDEC user command id
typedef enum
{
    //Group1:Set Control command================================
    E_VDEC_EX_USER_CMD_SET_CONTROL_BASE  = 0x0000,
    E_VDEC_EX_USER_CMD_REPEAT_LAST_FIELD,                       // Param: 1(ON), 0(OFF)
    E_VDEC_EX_USER_CMD_AVSYNC_REPEAT_TH,                        // Param:0x01 ~ 0xFF(repeat times), 0xFF:always repeat when av is not sync
    E_VDEC_EX_USER_CMD_DISP_ONE_FIELD,                          // Param: 1(ON), 0(OFF)
    E_VDEC_EX_USER_CMD_FD_MASK_DELAY_COUNT,                     // Param: unit is in vsync base for mute the fd_mask
    E_VDEC_EX_USER_CMD_FRC_OUTPUT,                              // Param: the address of VDEC_FRC_OutputParam
    E_VDEC_EX_USER_CMD_FRC_DROP_TYPE,                           // Param: 1(FRC_DROP_FIELD), 0(FRC_DROP_FRAME), default:0
    E_VDEC_EX_USER_CMD_FAST_DISPLAY,                            // Param: TRUE(Fast display), FALSE(Display until synced)
    E_VDEC_EX_USER_CMD_IGNORE_ERR_REF,                          // Param: TRUE(Ignore error reference), FALSE(Enable error reference handle)
    E_VDEC_EX_USER_CMD_FORCE_FOLLOW_DTV_SPEC,                   // Param: 1(ON), 0(OFF)
    E_VDEC_EX_USER_CMD_AVC_MIN_FRM_GAP,                         // Param: Set the theshold of H264 frame gap, 0xFFFFFFFF don't care frame gap
    E_VDEC_EX_USER_CMD_DISABLE_SEQ_CHG,                         // Param: 1(Disable), 0(Enable)
    E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE,              // Param: 1(ON) used for Openmax, 0(OFF) used for mstreamer and mm mode ,default : off
    E_VDEC_EX_USER_CMD_SET_DTV_USER_DATA_MODE,                  // Param: 0(Support normal DVB CC, default case), 1(Support ATSC DirectTV CC), 2,3,4(Reserved)
    E_VDEC_EX_USER_CMD_SET_SINGLE_TASK_MODE,
    E_VDEC_EX_USER_CMD_AVC_DISABLE_ANTI_VDEAD,
    E_VDEC_EX_USER_CMD_DTV_RESET_MVD_PARSER,                    // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_PVR_FLUSH_FRAME_BUFFER,
    E_VDEC_EX_USER_CMD_FORCE_INTERLACE_MODE,
    E_VDEC_EX_USER_CMD_RELEASE_FD_MASK,                         // Param: 1 to release fd mask when zooming or slow motion
    E_VDEC_EX_USER_CMD_SET_DECODE_MODE,
    E_VDEC_EX_USER_CMD_SUPPORT_AVC_TO_MVC,                      // Param: 0(Do not support), 1(Support AVC to MVC)
    E_VDEC_EX_USER_CMD_3DLR_VIEW_EXCHANGE,                      // Param: 0(Disable), 1(View L/R exhange)
    E_VDEC_EX_USER_CMD_SET_VSIZE_ALIGN,                         // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER,                       // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_AVC_DISP_IGNORE_CROP,                    // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE,
    E_VDEC_EX_USER_CMD_SET_AVSYNC_MODE,
    E_VDEC_EX_USER_CMD_SUSPEND_DYNAMIC_SCALE,                   // Param: 0(Disable, non-suspend DS), 1(Enable, suspend DS)
    E_VDEC_EX_USER_CMD_FORCE_AUTO_MUTE,
    E_VDEC_EX_USER_CMD_AVC_NEW_SLOW_MOTION,                     // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_PUSH_DISPQ_WITH_REF_NUM,                 // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_DS_RESV_N_BUFFER,                        // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_RM_ENABLE_PTS_TBL,                       // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_FLUSH_PTS_BUF,
    E_VDEC_EX_USER_CMD_SET_IDCT_MODE,                           // Param: 0(Original), 1(new IDCT)
    E_VDEC_EX_USER_CMD_DROP_ERR_FRAME,                          // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_SET_CC608_INFO_ENHANCE_MODE,
    E_VDEC_EX_USER_CMD_IGNORE_PIC_OVERRUN,                      // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_SET_SELF_SEQCHANGE,
    E_VDEC_EX_USER_CMD_AUTO_EXHAUST_ES_MODE,                    // Param: set the upper bound (arg[31:16]), and lower bound (arg[15:0])of ES level, Unit = 1KBytes, Auto drop display to consume ES data as soon as possible when ES level is higher than upper bound
    E_VDEC_EX_USER_CMD_CTL_SPEED_IN_DISP_ONLY,                  // Param: 0(Original: Dec and disp time), 1(In Disp only)
    E_VDEC_EX_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE,   // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_RETURN_INVALID_AFD,                      // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_FIELD_POLARITY_DISPLAY_ONE_FIELD,        // Param : VDEC_EX_Field_Polarity
    E_VDEC_EX_USER_CMD_AVC_FORCE_BROKEN_BY_US,                  // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_SHOW_FIRST_FRAME_DIRECT,                 // Param: 0(Disable), 1(Enable), Push first frame to display queue directly..
    E_VDEC_EX_USER_CMD_AVC_RESIZE_DOS_DISP_PEND_BUF,            // Param:  size of AVC display pending buffer for display outside mode
    E_VDEC_EX_USER_CMD_SET_XC_LOW_DELAY_PARA,                   // Param: arg0 for diff_field_number...
    E_VDEC_EX_USER_CMD_SET_SECURE_MODE,                         // Param: use enum VDEC_EX_SecureMode
    E_VDEC_EX_USER_CMD_RVU_SETTING_MODE,                        // Param: 0(Disable), 1(drop B-frame and force IDR)
    E_VDEC_EX_USER_CMD_FRAMERATE_HANDLING,                      // Arg 0~60000, 0: Disable, 1000 ~ 60000: Used the arg to set frame rate when the sequence did not have frame rate info. and arg is not zero. (The frame unit is (arg/1000)fps, Exp: 30000 = 30.000 fps), others: Do not thing.
    E_VDEC_EX_USER_CMD_DUAL_NON_BLOCK_MODE,                     // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_IGNORE_PIC_STRUCT_DISPLAY,               // Param: 0(Disable), 1(Enable) Ignore Pic_struct when display progressive frame.
    E_VDEC_EX_USER_CMD_INPUT_PTS_FREERUN_MODE,                  // Param: 0(Disable), 1(Enable) Video free run when the difference between input PTS and current STC is large than E_HVD_CMD_FREERUN_THRESHOLD + 1s;
    E_VDEC_EX_USER_CMD_ERR_CONCEAL_SLICE_1ST_MB,                // Param: 0(disable), Error concealment from current/last MB position; 1(enale) Error concealment from current slice first MB.(Need enable E_HVD_CMD_ERR_CONCEAL)
    E_VDEC_EX_USER_CMD_SET_EXTERNAL_DS_BUFFER,                  // Param: External DS Buffer info.
    E_VDEC_EX_USER_CMD_SET_MIN_TSP_DATA_SIZE,                   // Param: Resize HVD_FW_AVC_ES_MIN_TSP_DATA_SIZE
    E_VDEC_EX_USER_CMD_SET_DMX_FRAMERATE,
    E_VDEC_EX_USER_CMD_SET_DMX_FRAMERATEBASE,
    E_VDEC_EX_USER_CMD_ENABLE_CC_608_EXTERNAL_BUFFER,           // Param: u32_ccinfo 32bits-->([31:8]+[7:0] = addr+size), addr is kb unit, if u32_ccinfo ==0, it will turn off this feature
    E_VDEC_EX_USER_CMD_ENABLE_CC_708_EXTERNAL_BUFFER,           // Param: u32_ccinfo 32bits-->([31:8]+[7:0] = addr+size), addr is kb unit, if u32_ccinfo ==0, it will turn off this feature
    E_VDEC_EX_USER_CMD_SET_TIME_INC_PREDICT_PARA,
    E_VDEC_EX_USER_CMD_ENABLE_DECODE_ENGINE_TIMEOUT,            // Param: Enable/Disable decode timeout solution, timeout value unit:ms (VDEC_EX_Decode_Timeout_Param)
    E_VDEC_EX_USER_CMD_AUTO_FREE_ES,                            // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_FRAMEBUFFER_AUTO_MODE,                   // Param: 0(Disable),1(Enable), this cmd is used for MVD.
    E_VDEC_EX_USER_CMD_SET_SMOOTH_REWIND,                       // enable/disable or support smooth rewind
    E_VDEC_EX_USER_CMD_SET_ERROR_TOLERANCE,
    E_VDEC_EX_USER_CMD_AUTO_DROP_DISPLAY_QUEUE,                 // Param: 0(Disable), N = 1~16: Drop display queue when display queue above than N frames.
    E_VDEC_EX_USER_CMD_USE_CPB_REMOVAL_DEALY,                   // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_SKIP_N_FRAME,                            // Param: 0:disable, N = 1~63. Skip N frame.
    E_VDEC_EX_USER_CMD_SET_PTS_US_MODE,                         // Param: 1(enable), 0(disable ) PTS output by micro second level,
    E_VDEC_EX_USER_CMD_AUTO_INSERT_DUMMY_DATA,                  // Param: 1(enable),0(disable), Enable/Disable utopia auto insert dummy pattern in SLQ/BBU mode.
    E_VDEC_EX_USER_CMD_DROP_ONE_PTS,
    E_VDEC_EX_USER_CMD_PVR_TIMESHIFT_SEAMLESS_MODE,
    E_VDEC_EX_USER_CMD_AUTO_REDUCE_ES_DATA,
    E_VDEC_EX_USER_CMD_RM_FORCE_MCU_MODE_ES,                    // Param: 0(Disable), 1(Enable)
    E_VDEC_EX_USER_CMD_FORCE_PROGRESSIVE_MODE,                  // Param: 1(enable),0(disable), Enable/Disable force progressive mode
    E_VDEC_EX_USER_CMD_SET_FRAMEBUFF2,                          // Param[0]=Addr and Param[1]=size for the second frame buffer
    E_VDEC_EX_USER_CMD_ENABLE_PTS_DECTECTOR,                    // Param: 1(ON), 0(OFF)
    E_VDEC_EX_USER_CMD_SET_AVSYNC_DISP_AUTO_DROP,
    E_VDEC_EX_USER_CMD_SET_DYNAMIC_DISP_PATH,

    E_VDEC_EX_USER_CMD_MVC_SET_CMD_BASE  = 0x0800,
    E_VDEC_EX_USER_CMD_MVC_BBU2_PUSH_PACKET,                    // Param: Packet Info.
    E_VDEC_EX_USER_CMD_MVC_BBU2_FIRE_DECCMD,                    // Param: Non

    E_VDEC_EX_USER_CMD_UT_SET_CMD_BASE = 0x0900,
    E_VDEC_EX_USER_CMD_UT_SET_DBG_MODE,                         // Param: for enable the specify dbg mode for UT
    E_VDEC_EX_USER_CMD_UT_CLR_DBG_MODE,                         // Param: for disable the specify dbg mode for UT

    //Group2:Get Control command================================
    E_VDEC_EX_USER_CMD_GET_CONTROL_BASE  = 0x1000,
    E_VDEC_EX_USER_CMD_GET_CHROMA_TYPE,
    E_VDEC_EX_USER_CMD_GET_REAL_FRAMERATE,                      // Get Real FrameRate reported by decoder
    E_VDEC_EX_USER_CMD_GET_COLOR_MATRIX,                        // Get color matrix coefficients reported by decoder
    E_VDEC_EX_USER_CMD_GET_MAIN_STREAM_ID,                      // Get activated main stream ID
    E_VDEC_EX_USER_CMD_GET_SUB_STREAM_ID,                       // Get activated sub stream ID
    E_VDEC_EX_USER_CMD_GET_DYNSCALE_ENABLED,
    E_VDEC_EX_USER_CMD_GET_FPA_SEI,                             //Get SEI info
    E_VDEC_EX_USER_CMD_GET_U64PTS,
    E_VDEC_EX_USER_CMD_GET_ORI_INTERLACE_MODE,
    E_VDEC_EX_USER_CMD_GET_MBS_ONLY_FLAG,
    E_VDEC_EX_USER_CMD_GET_CRC_VALUE,                           //Get frame Y/UV crc value
    E_VDEC_EX_USER_CMD_GET_BBU_Q_NUM,
    E_VDEC_EX_USER_CMD_GET_DISP_FRAME_NUM,
    E_VDEC_EX_USER_CMD_GET_FPA_SEI_EX,                          //Get SEI info(enhancement)
    E_VDEC_EX_USER_CMD_GET_ES_BUFFER_STATUS,                    //Get ES buffer over/under flow status
    E_VDEC_EX_USER_CMD_GET_CODEC_TYPE,                          // Get Codec type
    E_VDEC_EX_USER_CMD_GET_SHAREMEMORY_BASE,
    E_VDEC_EX_USER_CMD_GET_IS_LEAST_DISPQ_SIZE_FLAG,
    E_VDEC_EX_USER_CMD_GET_FIELD_PIC_FLAG,                      // Param: Get Field Pic Flag
    E_VDEC_EX_USER_CMD_GET_SUPPORT_2ND_MVOP_INTERFACE,          // Param: TRUE : support, FALSE : not support
    E_VDEC_EX_USER_CMD_GET_FB_USAGE_MEM,                        // Get FrameBuufer Size needed by decoder
    E_VDEC_EX_USER_CMD_GET_XC_LOW_DELAY_INT_STATE,              // Get xc_low_delay int state...
    E_VDEC_EX_USER_CMD_GET_VSYNC_BRIDGE_ADDR,
    E_VDEC_EX_USER_CMD_GET_FRAME_INFO_EX,
    E_VDEC_EX_USER_CMD_GET_FLUSH_PATTEN_ENTRY_NUM,
    E_VDEC_EX_USER_CMD_GET_DS_BUF_MIU_SEL,                      //For those chips which has 3 MIU, use this get control to get correct miu select of DS buffer
    E_VDEC_EX_USER_CMD_GET_FW_STATUS_FLAG,
    E_VDEC_EX_USER_CMD_GET_HW_MAX_PIXEL,
    E_VDEC_EX_USER_CMD_GET_FLOW_CONTROL_U64PTS_DIFF,            //based on PTS table Rdptr and Wrptr, support TSP mode only
    E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT,            //replace of E_VDEC_EX_USER_CMD_GET_FRAME_INFO_EX
    E_VDEC_EX_USER_CMD_GET_VSYNC_BRIDGE_EXT_ADDR,               //get vsync bridge ext addr
    E_VDEC_EX_USER_CMD_GET_DCV_SEI,
    E_VDEC_EX_USER_CMD_GET_VUI_DISP_INFO,
    E_VDEC_EX_USER_CMD_GET_CODEC_CAP,
    E_VDEC_EX_USER_CMD_GET_PRE_PAS_U64PTS,
    E_VDEC_EX_USER_CMD_GET_CLLI_SEI,                            // Content light level Info
    E_VDEC_EX_USER_CMD_GET_SEQ_CHANGE_INFO,                     // Get the reason why seq changes, parameter is MS_U32, definition is VDEC_EX_SeqChangeInfo

    //MVC Get Control command================================
    E_VDEC_EX_USER_CMD_MVC_GET_CMD_BASE  = 0x1800,
    E_VDEC_EX_USER_CMD_GET_MVC_SUB_FRAME_DISP_INFO,             // Param: VDEC_FrameInfo pointer.
    E_VDEC_EX_USER_CMD_GET_MVC_BBU2_DECQ_VACANCY,               // Param: BBU2 Dec Q Vacancy.
    E_VDEC_EX_USER_CMD_GET_MVC_ES2_READ_PTR,                    // Param: ES2 read pointer.
    E_VDEC_EX_USER_CMD_GET_MVC_ES2_WRITE_PTR,                   // Param: ES2 Write pointer.
    E_VDEC_EX_USER_CMD_GET_ES_QUANTITY,                         // Param: Get ES buffer Level.
    E_VDEC_EX_USER_CMD_GET_ES2_QUANTITY,                        // Param: Get ES2 buffer Level.


    //Group3:System Preset Control command======================
    //Group3-1:Common system Preset Control command
    E_VDEC_EX_USER_CMD_SYSTEM_PRESET_CONTROL_BASE  = 0x2000,
    E_VDEC_EX_USER_CMD_SYSTEM_PRESET_VPU_CLOCK,                 //Param: VDEC_EX_ClockSpeed

    //Group3-2:HVD System Preset Control command
    E_VDEC_EX_USER_CMD_SYSTEM_PRESET_HVD_BASE      = 0x2100,
    E_VDEC_EX_USER_CMD_SYSTEM_PRESET_HVD_CLOCK,                 //Param: VDEC_EX_ClockSpeed

    //Group3-3:MVD System Preset Control command
    E_VDEC_EX_USER_CMD_SYSTEM_PRESET_MVD_BASE      = 0x2200,
    E_VDEC_EX_USER_CMD_SYSTEM_PRESET_MVD_CLOCK,                 //Param: VDEC_EX_ClockSpeed
    E_VDEC_EX_USER_CMD_VPU_SECURITY_MODE,                       //Param: 0:disable,1:enable

    //Group3-4:Preset Control command=============================
    E_VDEC_EX_USER_CMD_PRESET_CONTROL_BASE           = 0x2300,
    E_VDEC_EX_USER_CMD_HVD_ONE_PENDING_BUFFER_MODE,             //Param: 0(Disable), 1(Enable), use only one pending buffer instead of two for HVD
    E_VDEC_EX_USER_CMD_MVD_HWBUFFER_REMAPPING_MODE,             //Param: 0(Disable), 1(Enable),Allcate HW buffer to start of frame buffer
    E_VDEC_EX_USER_CMD_SET_SHAREMEMORY_BASE,
    E_VDEC_EX_USER_CMD_HVD_COL_BBU_MODE,                        //Param: HVD use colocated BBU mode, 0: disable, 1: enable /*johnny.ko*/
    E_VDEC_EX_USER_CMD_HVD_IAPGN_BUF_SHARE_BW_MODE,             //Param: HVD IAP GN Buffer address,
    /***/E_VDEC_EX_USER_CMD_DTV_DEBUG_MODE,
    E_VDEC_EX_USER_CMD_HVD_TS_IN_BBU_MODE,
    E_VDEC_EX_USER_CMD_AUTO_ARRANGE_FRAMEBUFFER_USAGE,          //Param: 0:disable,1:enable, address:PA,size:unit is byte
    E_VDEC_EX_USER_CMD_THUMBNAIL_MODE,                          //Param: 0(Disable), 1(Enable), use small frame buffer to decdoe thumbnail
    E_VDEC_EX_USER_CMD_FORCE_8BIT_DEC_MODE,                     //Param: force 8bit decode mode, 0: disable, 1: enable
    E_VDEC_EX_USER_CMD_MFCODEC_MODE,                            //Param: VDEC_EX_MFCodec_mode
    E_VDEC_EX_USER_CMD_VDEC_FEATURE,                            //AP control VDEC features
    E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE,                        //enable dynamic cma features
    E_VDEC_EX_USER_CMD_CONNECT_DISPLAY_PATH,

    //Group4:System Postset Control command======================
    E_VDEC_EX_USER_CMD_SYSTEM_POSTSET_CONTROL_BASE  = 0x3000,
    E_VDEC_EX_USER_CMD_SYSTEM_POSTSET_CLEAR_PROCESS_RELATED,

    //Group5:System PreGet Control command======================
    //Group5-1:Common system Preget Control command
    E_VDEC_EX_USER_CMD_SYSTEM_PREGET_CONTROL_BASE  = 0x4000,
    E_VDEC_EX_USER_CMD_SYSTEM_PREGET_FB_MEMORY_USAGE_SIZE,

} VDEC_EX_User_Cmd;

/// DMX status indicator
typedef enum
{
    DMX_FILTER_STATUS_OK,                   ///< DMX status OK
    DMX_FILTER_STATUS_ERROR,                ///< DMX status ERROR
    DMX_FILTER_STATUS_NOT_SUPPORT,          ///< DMX status NOT SUPPORT
    DMX_FILTER_STATUS_MULTIPRC_RUNNING      ///< DMX status multiple process still sunning
} DMX_FILTER_STATUS;

//-------------------------------------------------------------------------------------------------
/* Advanced Sound Effect */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
//============================================================
// System
//============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
void _MApi_AUDIO_WritePreInitTable(void);
void _MDrv_AUDIO_SetDspBaseAddr(MS_U8 u8Index, MS_PHY u32Bin_Base_Address, MS_PHY u32Mad_Base_Buffer_Adr);
void _MApi_AUDIO_Initialize(void);
void _MApi_Audio_Monitor(void);
MS_U32 _MDrv_AUDIO_GetDspMadBaseAddr(MS_U8 u8Index);
void _MApi_AUDIO_ExitAudioSystem(void);
void _MApi_AUDIO_SetPowerOn(MS_BOOL bFlag);
#else //User Space
void MApi_AUDIO_WritePreInitTable(void);
void MApi_AUDIO_SetDspBaseAddr(MS_U8 u8Index, MS_U32 u32Bin_Base_Address, MS_U32 u32Mad_Base_Buffer_Adr);
void MApi_AUDIO_Initialize(void);
void MApi_Audio_Monitor(void);
MS_U32 MApi_AUDIO_GetDspMadBaseAddr(MS_U8 u8Index);
void MApi_AUDIO_ExitAudioSystem(void);
void MApi_AUDIO_SetPowerOn(MS_BOOL bFlag);
#endif //User Space

//============================================================
// path
//============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
void _MApi_AUDIO_SetOutputInfo(AUDIO_OUT_INFO *pout_info);
void _MApi_AUDIO_SetPathInfo(AUDIO_PATH_INFO *path_info);
void _MApi_AUDIO_InputSwitch(AUDIO_INPUT_TYPE enSource, AUDIO_SWITCH_GROUP enGroup);
void _MApi_AUDIO_SetSourceInfo(AUDIO_SOURCE_INFO_TYPE eSourceType);
#else //User Space
void MApi_AUDIO_SetOutputInfo(AUDIO_OUT_INFO *pout_info);
void MApi_AUDIO_SetPathInfo(AUDIO_PATH_INFO *path_info);
void MApi_AUDIO_InputSwitch(AUDIO_INPUT_TYPE enSource, AUDIO_SWITCH_GROUP enGroup);
void MApi_AUDIO_SetSourceInfo(AUDIO_SOURCE_INFO_TYPE eSourceType);
#endif //User Space

//=============================================================
// volume
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
void _MApi_AUDIO_SetMixModeMute(AUDIO_SOURCE_INFO_TYPE eSourceType, AUDIO_MIX_VOL_TYPE VolType, MS_BOOL EnMute);
void _MApi_AUDIO_SetMixModeVolume(AUDIO_SOURCE_INFO_TYPE eSourceType, AUDIO_MIX_VOL_TYPE VolType, MS_U8 u8Vol1, MS_U8 u8Vol2);
void _MApi_AUDIO_SetAbsoluteVolume(MS_U8 u8Path, MS_U8 u8Vol1, MS_U8 u8Vol2);
void _MApi_AUDIO_SetMute(MS_U8 u8Path, MS_BOOL EnMute );
void _MApi_AUDIO_SPDIF_SetMute(MS_BOOL mute_en);
#else //User Space
void MApi_AUDIO_SetMixModeMute(AUDIO_SOURCE_INFO_TYPE eSourceType, AUDIO_MIX_VOL_TYPE VolType, MS_BOOL EnMute);
void MApi_AUDIO_SetMixModeVolume(AUDIO_SOURCE_INFO_TYPE eSourceType, AUDIO_MIX_VOL_TYPE VolType, MS_U8 u8Vol1, MS_U8 u8Vol2);
void MApi_AUDIO_SetAbsoluteVolume(MS_U8 u8Path, MS_U8 u8Vol1, MS_U8 u8Vol2);
void MApi_AUDIO_SetMute(MS_U8 u8Path, MS_BOOL EnMute );
void MApi_AUDIO_SPDIF_SetMute(MS_BOOL mute_en);
#endif //User Space

//============================================================
// Decoder
//============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
AUDIO_DEC_ID _MApi_AUDIO_OpenDecodeSystem(AudioDecStatus_t * p_AudioDecStatus);
MS_BOOL _MApi_AUDIO_GetDecodeSystem(AUDIO_DEC_ID DecId,  AudioDecStatus_t * p_AudioDecStatus);
MS_BOOL _MApi_AUDIO_SetDecodeSystem(AUDIO_DEC_ID DecId,  AudioDecStatus_t * p_AudioDecStatus);
MS_BOOL _MApi_AUDIO_ReleaseDecodeSystem(AUDIO_DEC_ID DecId);
MS_BOOL _MApi_AUDIO_SetMpegInfo(Audio_MPEG_infoType infoType, MS_U32 param1, MS_U32 param2);
MS_U32  _MApi_AUDIO_GetMpegInfo(Audio_MPEG_infoType infoType);
MS_BOOL _MApi_AUDIO_SetAC3PInfo(Audio_AC3P_infoType infoType, MS_U32 param1, MS_U32 param2);
MS_U32  _MApi_AUDIO_GetAC3PInfo(Audio_AC3P_infoType infoType);
MS_BOOL _MApi_AUDIO_SetAACInfo(Audio_AAC_infoType infoType, MS_U32 param1, MS_U32 param2);
void    _MApi_AUDIO_SetCommand(En_DVB_decCmdType enDecComamnd);
void    _MApi_AUDIO_SetADOutputMode(MS_U8 out_mode);
void    _MApi_AUDIO_SetASFParm(WMA_ASF_PARMTYPE parm_type, MS_U32 value);
MS_BOOL _MApi_AUDIO_SetDecodeCmd(AUDIO_DEC_ID DecId, En_DVB_decCmdType enDecComamnd);
MS_BOOL _MApi_AUDIO_GetMAD_LOCK(void);
#else //User Space
AUDIO_DEC_ID MApi_AUDIO_OpenDecodeSystem(AudioDecStatus_t * p_AudioDecStatus);
MS_BOOL MApi_AUDIO_GetDecodeSystem(AUDIO_DEC_ID DecId,  AudioDecStatus_t * p_AudioDecStatus);
MS_BOOL MApi_AUDIO_SetDecodeSystem(AUDIO_DEC_ID DecId,  AudioDecStatus_t * p_AudioDecStatus);
MS_BOOL MApi_AUDIO_ReleaseDecodeSystem(AUDIO_DEC_ID DecId);
MS_BOOL MApi_AUDIO_SetMpegInfo(Audio_MPEG_infoType infoType, MS_U32 param1, MS_U32 param2);
MS_U32  MApi_AUDIO_GetMpegInfo(Audio_MPEG_infoType infoType);
MS_BOOL MApi_AUDIO_SetAC3PInfo(Audio_AC3P_infoType infoType, MS_U32 param1, MS_U32 param2);
MS_U32  MApi_AUDIO_GetAC3PInfo(Audio_AC3P_infoType infoType);
MS_BOOL MApi_AUDIO_SetAACInfo(Audio_AAC_infoType infoType, MS_U32 param1, MS_U32 param2);
void    MApi_AUDIO_SetCommand(En_DVB_decCmdType enDecComamnd);
void    MApi_AUDIO_SetADOutputMode(MS_U8 out_mode);
void    MApi_AUDIO_SetASFParm(WMA_ASF_PARMTYPE parm_type, MS_U32 value);
MS_BOOL MApi_AUDIO_SetDecodeCmd(AUDIO_DEC_ID DecId, En_DVB_decCmdType enDecComamnd);
MS_BOOL MApi_AUDIO_GetMAD_LOCK(void);
#endif //User Space

//============================================================
// Encode
//============================================================

//=============================================================
// Common Info
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_BOOL   _MApi_AUDIO_SetCommAudioInfo(Audio_COMM_infoType infoType, MS_VIRT param1, MS_VIRT param2);
long long _MApi_AUDIO_GetCommAudioInfo(Audio_COMM_infoType infoType);
MS_BOOL   _MApi_AUDIO_SetAudioParam2(AUDIO_DEC_ID DecId, Audio_ParamType paramType, MS_VIRT Param);
MS_BOOL   _MApi_AUDIO_GetAudioInfo2(AUDIO_DEC_ID DecId, Audio_InfoType infoType, void * pInfo);
#else //User Space
MS_BOOL   MApi_AUDIO_SetCommAudioInfo(Audio_COMM_infoType infoType, MS_VIRT param1, MS_VIRT param2);
long long MApi_AUDIO_GetCommAudioInfo(Audio_COMM_infoType infoType);
MS_BOOL   MApi_AUDIO_SetAudioParam2(AUDIO_DEC_ID DecId, Audio_ParamType paramType, MS_VIRT Param);
MS_BOOL   MApi_AUDIO_GetAudioInfo2(AUDIO_DEC_ID DecId, Audio_InfoType infoType, void * pInfo);
#endif //User Space

//=============================================================
// Capture
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_BOOL _MApi_AUDIO_PCMCapture_Read(const AUDIO_DEVICE_TYPE eID, void *buffer, const MS_U32 bytes);
MS_BOOL _MApi_AUDIO_PCMCapture_Init(const AUDIO_DEVICE_TYPE eID, const AUDIO_CAPTURE_SOURCE_TYPE eSource);
MS_BOOL _MApi_AUDIO_PCMCapture_Start(const AUDIO_DEVICE_TYPE eID);
MS_BOOL _MApi_AUDIO_PCMCapture_Stop(const AUDIO_DEVICE_TYPE eID);
#else //User Space
MS_BOOL MApi_AUDIO_PCMCapture_Read(const AUDIO_DEVICE_TYPE eID, void *buffer, const MS_U32 bytes);
MS_BOOL MApi_AUDIO_PCMCapture_Init(const AUDIO_DEVICE_TYPE eID, const AUDIO_CAPTURE_SOURCE_TYPE eSource);
MS_BOOL MApi_AUDIO_PCMCapture_Start(const AUDIO_DEVICE_TYPE eID);
MS_BOOL MApi_AUDIO_PCMCapture_Stop(const AUDIO_DEVICE_TYPE eID);
#endif //User Space

//============================================================
// ATV
//============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
void    _MApi_AUDIO_SIF_SendCmd(En_AUD_SIF_CmdType enAudSifCommand, MS_U8 comm_arg1, MS_U8 comm_arg2);
MS_BOOL _MApi_AUDIO_SIF_SetBandSetup(MS_U8 SifBand);
void    _MApi_AUDIO_SIF_SetStandard(AUDIOSTANDARD_TYPE standard_type);
MS_BOOL _MApi_AUDIO_SIF_IsPALType(AUDIO_SIF_PAL_TYPE pal_type);
void    _MApi_AUDIO_SIF_SetPALType(AUDIO_SIF_PAL_TYPE pal_type);
MS_BOOL _MApi_AUDIO_SIF_SetSoundMode(MS_U8 u8SifSoundMode);
void    _MDrv_AUDIO_TriggerSifPLL(void);
MS_BOOL _MApi_AUDIO_SIF_GetAudioStatus(AUDIOSTATUS * eAudioStatus);
MS_U8   _MApi_AUDIO_SIF_GetSoundMode(void);
void    _MApi_AUDIO_SIF_StartAutoStandardDetection(void);
AUDIOSTANDARD_TYPE _MApi_AUDIO_SIF_GetResultOfAutoStandardDetection(void);
void    _MApi_AUDIO_SIF_SetThreshold(THR_TBL_TYPE *ThrTbl);
void    _MApi_AUDIO_SIF_SetPrescale(MS_U8 gain_type, MS_S32 db_value);
MS_BOOL _MApi_AUDIO_SetBtscA2ThresholdLevel(MS_U16 thresholdLevel, MS_U16 *pRF2NSR_map);
#if(UTPA_SUPPORT_SIF_SetBtscA2ThresholdLevel)
void    _MApi_AUDIO_SetThresholdType(THR_TBL_TYPE_C *ThrTbl, MS_U8 num,  MS_U8 standardtype);
#endif //#if(UTPA_SUPPORT_SIF_SetBtscA2ThresholdLevel)
#else //User Space
void    MApi_AUDIO_SIF_SendCmd(En_AUD_SIF_CmdType enAudSifCommand, MS_U8 comm_arg1, MS_U8 comm_arg2);
MS_BOOL MApi_AUDIO_SIF_SetBandSetup(MS_U8 SifBand);
void    MApi_AUDIO_SIF_SetStandard(AUDIOSTANDARD_TYPE standard_type);
MS_BOOL MApi_AUDIO_SIF_IsPALType(AUDIO_SIF_PAL_TYPE pal_type);
void    MApi_AUDIO_SIF_SetPALType(AUDIO_SIF_PAL_TYPE pal_type);
MS_BOOL MApi_AUDIO_SIF_SetSoundMode(MS_U8 u8SifSoundMode);
void    MApi_AUDIO_TriggerSifPLL(void);
MS_BOOL MApi_AUDIO_SIF_GetAudioStatus(AUDIOSTATUS * eAudioStatus);
MS_U8   MApi_AUDIO_SIF_GetSoundMode(void);
void    MApi_AUDIO_SIF_StartAutoStandardDetection(void);
AUDIOSTANDARD_TYPE MApi_AUDIO_SIF_GetResultOfAutoStandardDetection(void);
void    MApi_AUDIO_SIF_SetThreshold(THR_TBL_TYPE *ThrTbl);
void    MApi_AUDIO_SIF_SetPrescale(MS_U8 gain_type, MS_S32 db_value);
MS_BOOL MApi_AUDIO_SetBtscA2ThresholdLevel(MS_U16 thresholdLevel, MS_U16 *pRF2NSR_map);
#if(UTPA_SUPPORT_SIF_SetBtscA2ThresholdLevel)
void    MApi_AUDIO_SetThresholdType(THR_TBL_TYPE_C *ThrTbl, MS_U8 num,  MS_U8 standardtype);
#endif //#if(UTPA_SUPPORT_SIF_SetBtscA2ThresholdLevel)
#endif //User Space

//=============================================================
// SPDIF
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
void    _MApi_Audio_SPDIF_Monitor(void);
void    _MApi_AUDIO_SPDIF_SetMode(MS_U8 spdif_mode);
MS_U8   _MApi_AUDIO_SPDIF_GetMode(void);
void    _MApi_AUDIO_SPDIF_SetSCMS(MS_U8 C_bit_en, MS_U8 L_bit_en);
void    _MApi_AUDIO_SPDIF_HWEN(MS_BOOL spdif_en);
MS_BOOL _MApi_AUDIO_SPDIF_ChannelStatus_CTRL(AUDIO_SPDIF_CS_TYPE cs_mode, AUDIO_SPDIF_CS_TYPE_STATUS status);
#if(UTPA_SUPPORT_SPDIF_RX_MONITOR)
MS_BOOL _MApi_AUDIO_SPDIF_Rx_GetNonpcmFlag(void);
AUDIO_SPDIF_RX_TYPE _MApi_AUDIO_SPDIF_RX_GetNonPCM(void);
AUDIO_SPDIF_RX_TYPE _MApi_AUDIO_SPDIF_RX_SetNonpcm(MS_U8 nonPCM_en);
#endif //#if(UTPA_SUPPORT_SPDIF_RX_MONITOR)

#else //User Space
void    MApi_Audio_SPDIF_Monitor(void);
void    MApi_AUDIO_SPDIF_SetMode(MS_U8 spdif_mode);
MS_U8   MApi_AUDIO_SPDIF_GetMode(void);
void    MApi_AUDIO_SPDIF_SetSCMS(MS_U8 C_bit_en, MS_U8 L_bit_en);
void    MApi_AUDIO_SPDIF_HWEN(MS_BOOL spdif_en);
MS_BOOL MApi_AUDIO_SPDIF_ChannelStatus_CTRL(AUDIO_SPDIF_CS_TYPE cs_mode, AUDIO_SPDIF_CS_TYPE_STATUS status);
#if(UTPA_SUPPORT_SPDIF_RX_MONITOR)
MS_BOOL MApi_AUDIO_SPDIF_Rx_GetNonpcmFlag(void);
AUDIO_SPDIF_RX_TYPE MApi_AUDIO_SPDIF_RX_GetNonPCM(void);
AUDIO_SPDIF_RX_TYPE MApi_AUDIO_SPDIF_RX_SetNonpcm(MS_U8 nonPCM_en);
#endif //#if(UTPA_SUPPORT_SPDIF_RX_MONITOR)
#endif //User Space

//=============================================================
// HDMI
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
#ifdef CONFIG_API_HDMITX //TV dont have HDMI TX
MS_BOOL MApi_HDMITx_Init(void);
void    MApi_HDMITx_SetAudioConfiguration(HDMITX_AUDIO_FREQUENCY freq, HDMITX_AUDIO_CHANNEL_COUNT ch, HDMITX_AUDIO_CODING_TYPE type);
void    MApi_HDMITx_SetAudioOnOff(MS_BOOL state);
void    MApi_HDMITx_SetAudioFrequency(HDMITX_AUDIO_FREQUENCY freq);
void    MApi_HDMITx_SetAudioSourceFormat(HDMITX_AUDIO_SOURCE_FORMAT fmt);
HDMITX_HDCP_STATUS MApi_HDMITx_GetHDCPStatus(void);
#endif //#ifdef CONFIG_API_HDMITX

MS_BOOL _MApi_AUDIO_DigitalOut_SetDeviceCapability(DIGITAL_OUTPUT_TYPE eoutType, AUDIO_FORMAT_CODE ecodecType, Digital_Out_Device_Capability_t *p_codecCapability);
MS_BOOL _MApi_AUDIO_HDMI_GetNonpcmFlag(void);
void    _MApi_AUDIO_HDMI_Tx_GetStatus(MS_BOOL *onOff, AUDIO_FS_TYPE *hdmi_SmpFreq, HDMI_TX_OUTPUT_TYPE *outType );
void    _MApi_AUDIO_HDMI_TX_SetMode(HDMI_TX_OUTPUT_TYPE outType);
AUDIO_HDMI_RX_TYPE _MApi_AUDIO_HDMI_RX_SetNonpcm(MS_U8 nonPCM_en);
AUDIO_HDMI_RX_TYPE _MApi_AUDIO_HDMI_RX_GetNonPCM(void);

#else //User Space
#ifdef CONFIG_API_HDMITX //TV dont have HDMI TX
MS_BOOL MApi_HDMITx_Init(void);
void    MApi_HDMITx_SetAudioConfiguration(HDMITX_AUDIO_FREQUENCY freq, HDMITX_AUDIO_CHANNEL_COUNT ch, HDMITX_AUDIO_CODING_TYPE type);
void    MApi_HDMITx_SetAudioOnOff(MS_BOOL state);
void    MApi_HDMITx_SetAudioFrequency(HDMITX_AUDIO_FREQUENCY freq);
void    MApi_HDMITx_SetAudioSourceFormat(HDMITX_AUDIO_SOURCE_FORMAT fmt);
HDMITX_HDCP_STATUS MApi_HDMITx_GetHDCPStatus(void);
#endif //#ifdef CONFIG_API_HDMITX

MS_BOOL MApi_AUDIO_DigitalOut_SetDeviceCapability(DIGITAL_OUTPUT_TYPE eoutType, AUDIO_FORMAT_CODE ecodecType, Digital_Out_Device_Capability_t *p_codecCapability);
MS_BOOL MApi_AUDIO_HDMI_GetNonpcmFlag(void);
void    MApi_AUDIO_HDMI_Tx_GetStatus(MS_BOOL *onOff, AUDIO_FS_TYPE *hdmi_SmpFreq, HDMI_TX_OUTPUT_TYPE *outType );
void    MApi_AUDIO_HDMI_TX_SetMode(HDMI_TX_OUTPUT_TYPE outType);
AUDIO_HDMI_RX_TYPE MApi_AUDIO_HDMI_RX_SetNonpcm(MS_U8 nonPCM_en);
AUDIO_HDMI_RX_TYPE MApi_AUDIO_HDMI_RX_GetNonPCM(void);
#endif //User Space

//=============================================================
// Digital Output Type
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_S32 _MApi_AUDIO_SYSTEM_Control(MS_U8 *pStrCmd, MS_U32 u32StrCmdLength);
#else //User Space
MS_S32 MApi_AUDIO_SYSTEM_Control(MS_U8 *pStrCmd, MS_U32 u32StrCmdLength);
#endif //User Space

//=============================================================
// PCM IO
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_S32 _MApi_AUDIO_PCM_Open(void *pData);
MS_S32 _MApi_AUDIO_PCM_Close(MS_S32 s32DeviceId);
MS_S32 _MApi_AUDIO_PCM_Start(MS_S32 s32DeviceId);
MS_S32 _MApi_AUDIO_PCM_Stop(MS_S32 s32DeviceId);
MS_S32 _MApi_AUDIO_PCM_Set(MS_S32 s32DeviceId, MS_U32 u32Cmd, const void *pData);
MS_S32 _MApi_AUDIO_PCM_Get(MS_S32 s32DeviceId, MS_U32 u32Cmd, void *pData);
MS_U32 _MApi_AUDIO_PCM_Read(MS_S32 s32DeviceId, void *pBuf, MS_U32 u32Size);
MS_U32 _MApi_AUDIO_PCM_Write(MS_S32 s32DeviceId, const void *pBuf, MS_U32 u32Size);
MS_S32 _MApi_AUDIO_PCM_Flush(MS_S32 s32DeviceId);
#else //User Space
MS_S32 MApi_AUDIO_PCM_Open(void *pData);
MS_S32 MApi_AUDIO_PCM_Close(MS_S32 s32DeviceId);
MS_S32 MApi_AUDIO_PCM_Start(MS_S32 s32DeviceId);
MS_S32 MApi_AUDIO_PCM_Stop(MS_S32 s32DeviceId);
MS_S32 MApi_AUDIO_PCM_Set(MS_S32 s32DeviceId, MS_U32 u32Cmd, const void *pData);
MS_S32 MApi_AUDIO_PCM_Get(MS_S32 s32DeviceId, MS_U32 u32Cmd, void *pData);
MS_U32 MApi_AUDIO_PCM_Read(MS_S32 s32DeviceId, void *pBuf, MS_U32 u32Size);
MS_U32 MApi_AUDIO_PCM_Write(MS_S32 s32DeviceId, const void *pBuf, MS_U32 u32Size);
MS_S32 MApi_AUDIO_PCM_Flush(MS_S32 s32DeviceId);
#endif //User Space

//=============================================================
// Basic Sound Effect
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_BOOL _MApi_SND_ProcessEnable(Sound_ENABLE_Type Type, MS_BOOL enable);
MS_BOOL _MApi_SND_SetParam1(Sound_SET_PARAM_Type Type, MS_U16 param1, MS_U16 param2);
MS_U16  _MApi_SND_GetParam1(Sound_GET_PARAM_Type Type, MS_U16 param1);
void    _MApi_AUDIO_SetPEQCoef(AUDIO_PEQ_COEF *peq_coef);
void    _MApi_AUDIO_SetAvcAT(MS_U8 AvcAT);
void    _MApi_AUDIO_SetAvcRT(MS_U8 AvcRT);
MS_BOOL _MApi_AUDIO_SoundEffect_SetParam(const MS_S8 *pStrCmd, MS_U32 u32StrCmdLength, MS_S8 *pData, MS_U32 u32DataSize);
#else //User Space
MS_BOOL MApi_SND_ProcessEnable(Sound_ENABLE_Type Type, MS_BOOL enable);
MS_BOOL MApi_SND_SetParam1(Sound_SET_PARAM_Type Type, MS_U16 param1, MS_U16 param2);
MS_U16  MApi_SND_GetParam1(Sound_GET_PARAM_Type Type, MS_U16 param1);
void    MApi_AUDIO_SetPEQCoef(AUDIO_PEQ_COEF *peq_coef);
void    MApi_AUDIO_SetAvcAT(MS_U8 AvcAT);
void    MApi_AUDIO_SetAvcRT(MS_U8 AvcRT);
MS_BOOL MApi_AUDIO_SoundEffect_SetParam(const MS_S8 *pStrCmd, MS_U32 u32StrCmdLength, MS_S8 *pData, MS_U32 u32DataSize);
#endif //User Space

//=============================================================
// ADV Sound Effect
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_BOOL _MApi_AUDIO_ADVSOUND_ProcessEnable(ADVSND_TYPE type);
MS_BOOL _MApi_AUDIO_ADVSOUND_SubProcessEnable(ADVFUNC proc, MS_BOOL enable);
MS_BOOL _MApi_AUDIO_ADVSND_SetParam(ADVSND_PARAM param, MS_U32 u32value1, MS_U16 u16value2);
void    _MApi_AUDIO_COPY_Parameter(ADVSND_PARAM type, void* Parameter_ptr,MS_U32 size);
#if (UTPA_SUPPORT_CUS2SE1)
MS_BOOL _MApi_AUDIO_ADVSOUND_SetParam2(ADVSND_PARAM param, MS_U32 index, MS_U32 *u32value1, MS_U16 u16value2);
MS_BOOL _MApi_AUDIO_ADVSOUND_GetParam2(ADVSND_PARAM param, MS_U32 index, MS_U32 *u32value1, MS_U16 u16value2);
#endif //#if (UTPA_SUPPORT_CUS2SE1)
#else //User Space
MS_BOOL MApi_AUDIO_ADVSOUND_ProcessEnable(ADVSND_TYPE type);
MS_BOOL MApi_AUDIO_ADVSOUND_SubProcessEnable(ADVFUNC proc, MS_BOOL enable);
MS_BOOL MApi_AUDIO_ADVSND_SetParam(ADVSND_PARAM param, MS_U32 u32value1, MS_U16 u16value2);
void    MApi_AUDIO_COPY_Parameter(ADVSND_PARAM type, void* Parameter_ptr,MS_U32 size);
#if (UTPA_SUPPORT_CUS2SE1)
MS_BOOL MApi_AUDIO_ADVSOUND_SetParam2(ADVSND_PARAM param, MS_U32 index, MS_U32 *u32value1, MS_U16 u16value2);
MS_BOOL MApi_AUDIO_ADVSOUND_GetParam2(ADVSND_PARAM param, MS_U32 index, MS_U32 *u32value1, MS_U16 u16value2);
#endif //#if (UTPA_SUPPORT_CUS2SE1)
#endif //User Space

//=============================================================
// MM
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
MS_PHY  _MApi_AUDIO_GetDDRInfo(AUDIO_DEC_ID DecId, EN_AUDIO_DDRINFO DDRInfo);
MS_BOOL _MApi_AUDIO_MM2_initAesInfo(AUDIO_DEC_ID dec_id);
MS_BOOL _MApi_AUDIO_MM2_checkAesInfo(AUDIO_DEC_ID  dec_id, AES_INFO *aes_info);
MS_BOOL _MApi_AUDIO_MM2_inputAesFinished(AUDIO_DEC_ID dec_id, MS_U32 es_size, MS_BOOL ptsExist, MS_U64 pts);
#else //User Space
MS_PHY  MApi_AUDIO_GetDDRInfo(AUDIO_DEC_ID DecId, EN_AUDIO_DDRINFO DDRInfo);
MS_BOOL MApi_AUDIO_MM2_initAesInfo(AUDIO_DEC_ID dec_id);
MS_BOOL MApi_AUDIO_MM2_checkAesInfo(AUDIO_DEC_ID  dec_id, AES_INFO *aes_info);
MS_BOOL MApi_AUDIO_MM2_inputAesFinished(AUDIO_DEC_ID dec_id, MS_U32 es_size, MS_BOOL ptsExist, MS_U64 pts);
#endif //User Space

//=============================================================
// not audio, use for debug
//=============================================================
#if (defined(MSOS_TYPE_LINUX_KERNEL) && UTPA_SUPPORT_Ioctrl) //Kernel Space + UtopiaIoctl
VDEC_EX_Result MApi_VDEC_EX_GetControl(VDEC_StreamId *pStreamId, VDEC_EX_User_Cmd cmd_id, MS_U32 *param);
MS_U32 MApi_VDEC_EX_GetPTS(VDEC_StreamId *pStreamId);
DMX_FILTER_STATUS MApi_DMX_Stc_Get(MS_U32* pu32Stc32, MS_U32* pu32Stc);
DMX_FILTER_STATUS MApi_DMX_Stc_Set(MS_U32 u32Stc32, MS_U32 u32Stc);
#else //User Space
VDEC_EX_Result MApi_VDEC_EX_GetControl(VDEC_StreamId *pStreamId, VDEC_EX_User_Cmd cmd_id, MS_U32 *param);
MS_U32 MApi_VDEC_EX_GetPTS(VDEC_StreamId *pStreamId);
DMX_FILTER_STATUS MApi_DMX_Stc_Get(MS_U32* pu32Stc32, MS_U32* pu32Stc);
DMX_FILTER_STATUS MApi_DMX_Stc_Set(MS_U32 u32Stc32, MS_U32 u32Stc);
#endif //User Space

#ifdef __cplusplus
}
#endif

#endif //_AUDIO_V2_CUSTOMER_H_

#endif //#if defined(AUDIO_CWRAP) || defined(ROLLS_ROYCE) || defined(AUDIO_CWRAP_DBG)