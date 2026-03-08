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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file    drvPQ.h
/// @brief  PQ interface
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRVPQ_CUS_H_
#define _DRVPQ_CUS_H_

#ifdef _MDRV_PQ_CUS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
#ifdef __cplusplus
extern "C"
{
#endif
#include "utopia_dapi.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
/**
*   PQ GRule Support List
*/
typedef enum
{
    /// PQ GRule Support Film
    E_PQ_SUPPORTED_FILM,
    /// PQ GRule Support NR
    E_PQ_SUPPORTED_NR,
    /// PQ GRule Support MPEG NR
    E_PQ_SUPPORTED_MPEG_NR,
    /// PQ GRule Support Black Level
    E_PQ_SUPPORTED_BLACK_LEVEL,
    /// PQ GRule Support Ultra Clear
    E_PQ_SUPPORTED_ULTRA_CLEAR,
    /// PQ GRule Support HDR
    E_PQ_SUPPORTED_HDR,
    /// PQ GRule Support Max number
    E_PQ_SUPPORTED_MAX,
} PQ_SUPPORTED_TYPE;


typedef enum
{
    E_PQ_SOURCETYPE_VGB_HDMIPC = 0,
    E_PQ_SOURCETYPE_HDMI,
    E_PQ_SOURCETYPE_YPBPR,
    E_PQ_SOURCETYPE_DTV,
    E_PQ_SOURCETYPE_MULTIMEDIA,
    E_PQ_SOURCETYPE_CVBS_SCARTRGB,
    E_PQ_SOURCETYPE_ATV,
    E_PQ_SOURCETYPE_SV,
    E_PQ_SOURCETYPE_AV,
    E_PQ_SOURCETYPE_MAX,
} EN_PQ_SOURCETYPE;

/**
 * PQ MPEG_NR_FUNCTION_TYPE
 */
typedef enum
{
    /// off
    PQ_MPEG_NR_MIN,
    /// off
    PQ_MPEG_NR_OFF = PQ_MPEG_NR_MIN,
    /// Low
    PQ_MPEG_NR_LOW,
    /// Middle
    PQ_MPEG_NR_MID,
    /// High
    PQ_MPEG_NR_HIGH,
    /// Auto
    PQ_MPEG_NR_AUTO,
    /// Default
    PQ_MPEG_NR_DEFAULT,
    /// The max support number of PQ MPEG Noise reduction
    PQ_MPEG_NR_NUM,
}PQ_MPEG_NR_FUNCTION_TYPE;

/**
 * PQ PQ_XVYCC_TYPE
 */
typedef enum
{
    PQ_XVYCC_NORMAL,
    PQ_XVYCC_ON_XVYCC,
    PQ_XVYCC_ON_SRGB,
    PQ_XVYCC_NUM,
}PQ_XVYCC_TYPE;

typedef struct
{
    MS_U32 u32Version;
    MS_U32 u32Id;
} PQ_DEVICE_ID;

/**
 * EN_PQ_CFD_CTRL_TYPE
 */
typedef enum
{
    /// Initialize
    E_PQ_CFD_CTRL_SET_INIT = 0,
    /// VGA
    E_PQ_CFD_CTRL_SET_VGA = 1,
    /// TV (ATV)
    E_PQ_CFD_CTRL_SET_TV = 2,
    /// CVBS (AV)
    E_PQ_CFD_CTRL_SET_CVBS = 3,
    /// S-Video
    E_PQ_CFD_CTRL_SET_SVIDEO = 4,
    /// YPbPr
    E_PQ_CFD_CTRL_SET_YPBPR = 5,
    /// Scart
    E_PQ_CFD_CTRL_SET_SCART = 6,
    /// HDMI
    E_PQ_CFD_CTRL_SET_HDMI = 7,
    /// DTV
    E_PQ_CFD_CTRL_SET_DTV = 8,
    /// DVI
    E_PQ_CFD_CTRL_SET_DVI = 9,
    /// MM
    E_PQ_CFD_CTRL_SET_MM = 10,
    /// Panel
    E_PQ_CFD_CTRL_SET_PANEL = 11,
    /// HDR
    E_PQ_CFD_CTRL_SET_HDR = 12,
    /// EDID
    E_PQ_CFD_CTRL_SET_EDID = 13,
    /// OSD
    E_PQ_CFD_CTRL_SET_OSD = 14,
    /// Fire
    E_PQ_CFD_CTRL_SET_FIRE = 15,
    /// DLC
    E_PQ_CFD_CTRL_SET_DLC = 16,
    /// Linear RGB
    E_PQ_CFD_CTRL_SET_LINEAR_RGB = 17,

    E_PQ_CFD_CTRL_MAX,
} EN_PQ_CFD_CTRL_TYPE;


typedef enum
{
    E_PQ_DNR,
    E_PQ_UCNR,
    E_PQ_ZNR,
    E_PQ_SPF,
    E_PQ_DBK,
    E_PQ_DMS_12_LINE,
    E_PQ_UCD,
    E_PQ_DLC,
    E_PQ_BWLE,
    E_PQ_SWDR,
    E_PQ_VIP_ALL_BYPASS,
    E_PQ_HSY,
    E_PQ_COLORENG_CBCR_BYPASS,
    E_PQ_FCC,
    E_PQ_FWC,
    E_PQ_ABF,
    E_PQ_PRE_CTI,
    E_PQ_POST_CTI,
    E_PQ_SHARPNESS,
    E_PQ_PQ_GAMMA,
    E_PQ_PANEL_GAMMA,
    E_PQ_3D_RGB,
    E_PQ_RGB_GAIN,
    E_PQ_LINEAR_RGB,
    E_PQ_PQ_DRIVER,
    E_PQ_SCALING_FILTER_BYPASS,
    E_PQ_BRIGHTNESS,
    E_PQ_RGB_CLIP,
    E_PQ_MAX,
} EN_PQ_FUNCTION_INFO_INDEX;

typedef struct
{
    /// Structure version
    MS_U32 u32Version;
    /// Structure length
    MS_U16 u16Length;

    /// Window (main or sub window)
    MS_U8 u8Win;

    /// Color format, reference EN_KDRV_XC_CFD_COLOR_FORMAT.
    MS_U8 u8ColorFormat;
    /// Color data format, reference EN_KDRV_XC_CFD_COLOR_DATA_FORMAT.
    MS_U8 u8ColorDataFormat;
    /// Full range
    MS_BOOL bIsFullRange;
    /// Color primaries
    MS_U8 u8ColorPrimaries;
    /// Transfer characteristics
    MS_U8 u8TransferCharacteristics;
    /// Matrix coefficients
    MS_U8 u8MatrixCoefficients;

} ST_PQ_CFD_ANALOG;

typedef struct
{
    /// Structure version
    MS_U32 u32Version;
    /// Structure length
    MS_U16 u16Length;

    /// Window (main or sub window)
    MS_U8 u8Win;
    /// Linear RGB enable
    MS_BOOL bEnable;
} ST_PQ_CFD_LINEAR_RGB;

/// CFD control information, reference mdrv_xc_st.h in kernel.
typedef struct  __attribute__((packed))
{
    /// Control type, reference EN_MAPI_CFD_CTRL_TYPE
    EN_PQ_CFD_CTRL_TYPE enCtrlType;
    /// Control parameter
    void *pParam;
#if !defined (__aarch64__)
    /// Dummy parameter
    void *pDummy;
#endif
    /// Control parameter length
    MS_U32 u32ParamLen;
    /// Error code
    MS_U16 u16ErrCode;
} ST_PQ_CFD_CONTROL_INFO;

typedef enum
{
    E_PQ_SWDRMODEGRULE,
    E_PQ_TMOMODEGRULE,
    E_PQ_UCDMODEGRULE,
    E_PQ_ENABLEHDRMODE,
    E_PQ_ENABLEGAMEMODE,
    E_PQ_GRULEMAX,
}EN_PQ_MI_TEST_CASE;

/**
 * PQ PQ_ADAPTIVE_TUNING_CLIENT
 */
typedef enum
{
    PQ_ADAPTIVE_TUNING_CLIENT_SCALER,
    PQ_ADAPTIVE_TUNING_CLIENT_SCALER_1,
    PQ_ADAPTIVE_TUNING_CLIENT_DIP,
    PQ_ADAPTIVE_TUNING_CLIENT_MAX,
}PQ_ADAPTIVE_TUNING_CLIENT;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
/// map inputsource type into index for QMap
/// @param  eWindow                          \b IN: PQ Window Main or sub
/// @param  enInputSourceType                   \b IN: the query input source type
/// @return corresponding index
//-------------------------------------------------------------------------------------------------
//INTERFACE MS_U16 QM_InputSourceToIndex(void* pInstance, PQ_WIN eWindow, MS_U8 enInputSourceType);

//-------------------------------------------------------------------------------------------------
/// Set ColorthroughMode
/// @param  eWindow                      \b IN: PQ Window Main or sub
/// @param  bEn                          \b IN: enable/disable ColorthroughMode
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_SetColorThroughMode(PQ_WIN eWindow, MS_BOOL bEn);

//INTERFACE MS_U16 QM_InputSourceToIndex_UFSC(PQ_WIN eWindow, MS_U8 enInputSourceType);
//-------------------------------------------------------------------------------------------------
/// Load FilmMode Table
/// @param  eWindow                          \b IN: TBD
/// @param  enFilmModeType               \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_LoadFilmModeTable(PQ_WIN eWindow, PQ_FILM_MODE_FUNCTION_TYPE enFilmModeType);

//-------------------------------------------------------------------------------------------------
/// Load DynamicContrast Table
/// @param  eWindow                          \b IN: TBD
/// @param  enDynamicContrastType   \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_LoadDynamicContrastTable(PQ_WIN eWindow, PQ_DYNAMIC_CONTRAST_FUNCTION_TYPE enDynamicContrastType);

//-------------------------------------------------------------------------------------------------
/// Set DLC Mode Status
/// @param  enWindow                          \b IN: TBD
/// @param  enDynamicContrastType      \b IN: TBD
/// @retval Support or not
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetDLCModeStatus(PQ_WIN enWindow,PQ_DYNAMIC_CONTRAST_FUNCTION_TYPE enDynamicContrastType);

//-------------------------------------------------------------------------------------------------
/// Get DLC Mode Status
/// @param  enWindow                          \b IN: TBD
/// @retval Support or not
//-------------------------------------------------------------------------------------------------
INTERFACE PQ_DYNAMIC_CONTRAST_FUNCTION_TYPE SYMBOL_WEAK MDrv_PQ_GetDLCModeStatus(PQ_WIN enWindow);

//-------------------------------------------------------------------------------------------------
/// Load NR Table
/// @param  eWindow                          \b IN: TBD
/// @param  en3DNRType                       \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_LoadNRTable(PQ_WIN eWindow, PQ_3D_NR_FUNCTION_TYPE en3DNRType);

//-------------------------------------------------------------------------------------------------
/// Load MPEG NR Table
/// @param  eWindow                          \b IN: TBD
/// @param  enMPEGNRType                \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_LoadMPEGNRTable(PQ_WIN eWindow, PQ_MPEG_NR_FUNCTION_TYPE enMPEGNRType);

//-------------------------------------------------------------------------------------------------
/// Load UltraClear Table
/// @param  eWindow                          \b IN: TBD
/// @param  enCtrlType                       \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_Load_ULTRACLEAR_Table(PQ_WIN eWindow, PQ_FEATURE_SWITCH_TYPE enCtrlType);

//-------------------------------------------------------------------------------------------------
/// Load XvYCC Table
/// @param  eWindow                          \b IN: TBD
/// @param  enXvyccType                       \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_Load_XVYCC_Table(PQ_WIN eWindow, PQ_XVYCC_TYPE enXvyccType);

//-------------------------------------------------------------------------------------------------
/// Load customer IP setting base on CFD+
/// @param  eWindow                          \b IN: TBD
/// @param  EN_PQ_XC_CUSTOMER_IP             \b IN: IP Customer want to set
/// @param  pu8data                          \b IN: point to CustomerIP setting
/// @param  u16Size                          \b IN: the size of CustomerIP setting
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_Set_CustomerIp_Parameter(PQ_WIN eWindow, EN_PQ_XC_CUSTOMER_IP enCustomerIp, MS_U8 *pu8data, MS_U16 u16Size);

//-------------------------------------------------------------------------------------------------
/// Set Dynamic Scaling on/off
/// @param  enType                             \b IN: TBD
/// @retval Support or not
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_GRULE_Get_Support_Status(PQ_SUPPORTED_TYPE enType);

//-------------------------------------------------------------------------------------------------
/// Set Memory format
/// @param  eWindow                          \b IN: TBD
/// @param  bMemFmt422                       \b IN: TBD
/// @param  bFBL                             \b IN: TBD
/// @param  u8BitsPerPixel                   \b IN: TBD
/// @return TBD
//-------------------------------------------------------------------------------------------------
INTERFACE PQ_DEINTERLACE_MODE SYMBOL_WEAK MDrv_PQ_SetMemFormat(PQ_WIN eWindow,
                                                MS_BOOL bMemFmt422,
                                                MS_BOOL bFBL,
                                                MS_U8 *u8BitsPerPixel);

//-------------------------------------------------------------------------------------------------
/// Reduce Bandwidth for OSD
/// @param  eWindow                          \b IN: TBD
/// @param  bOSD_On                          \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_ReduceBW_ForOSD(PQ_WIN eWindow, MS_BOOL bOSD_On);

//-------------------------------------------------------------------------------------------------
/// Set PQ array of the display type.
/// @param  u16DisplayWidth                \b IN: Panel width
/// @param  enDisplaType                   \b IN: One, PIP, POP
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_Init_DisplayType(MS_U16 u16DisplayWidth, PQ_DISPLAY_TYPE enDisplaType);

//-------------------------------------------------------------------------------------------------
/// Set PQ array of the display type.
/// @param  u16DisplayWidth                \b IN: Panel width
/// @param  enDisplaType                   \b IN: One, PIP, POP
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_Set_DisplayType(MS_U16 u16DisplayWidth, PQ_DISPLAY_TYPE enDisplaType);

//------------------------------------------------------------------------------------------------
/// Update the Panel ID in PQDS case
/// @param *pInstance
/// @param u16DisplayWidth                   \b IN: Panel width
/// @return NONE
//------------------------------------------------------------------------------------------------
void SYMBOL_WEAK MDrv_PQDS_Update_PanelID(void *pInstance, MS_U16 u16DisplayWidth);

//-------------------------------------------------------------------------------------------------
/// get OSD BW Grule MADI setting
/// @param  eWindow                        \b IN: PQ Window Main or sub
/// @param  *pu16TabIdx_MADi               \b IN: MADI table Index
/// @param   *pu16TabIdx_MADi_Motion       \b IN: MADI motion table Index
/// @return TRUE: Success, FALSE: failed
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_GetMADiFromOSDBWGrule(PQ_WIN eWindow, MS_U16 *pu16TabIdx_MADi, MS_U16 *pu16TabIdx_MADi_Motion);

//-------------------------------------------------------------------------------------------------
/// get InGeneral MADI setting
/// @param  eWindow                        \b IN: PQ Window Main or sub
/// @param  *pu16TabIdx_MADi               \b IN: MADI table Index
/// @param   *pu16TabIdx_MADi_Motion       \b IN: MADI motion table Index
/// @return TRUE: Success, FALSE: failed
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_GetMADiInGeneral(PQ_WIN eWindow,MS_U16 *pu16TabIdx_MADi,MS_U16 *pu16TabIdx_MADi_Motion);

//-------------------------------------------------------------------------------------------------
/// Load PTP Table
/// @param  eWindow                          \b IN: PQ Window Main or sub
/// @param  enPTP_Type                       \b IN: QMap PTP type
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_LoadPTPTable(PQ_WIN eWindow, PQ_PTP_FUNCTION_TYPE enPTP_Type);

//-------------------------------------------------------------------------------------------------
/// Check current input timing is suitable for PTP or not
/// @param  pstXC_SetWin_Info                \b IN: Set Win Info
/// @param  eWindow                          \b IN: PQ Window Main or sub
/// @return TRUE: suitable for PTP, FALSE: not suitable
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_Check_PointToPoint_Condition(XC_SETWIN_INFO *pstXC_SetWin_Info, PQ_WIN eWindow);

//-------------------------------------------------------------------------------------------------
/// Enable/Disable PostCCS
/// @param  eWindow                          \b IN: PQ Window Main or sub
/// @param  bEnable                        \b IN: Enable/Disable
/// @return TRUE: Success, FALSE: Fail
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetPostCCSOnOff(PQ_WIN eWindow, MS_BOOL bEnable);

//-------------------------------------------------------------------------------------------------
/// Adaptive tuning
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_AdaptiveTuning(void);

#ifdef UFO_XC_PQ_SUPPORT_SWDRIVING_MULTI_DEVICES
//-------------------------------------------------------------------------------------------------
/// Adaptive tuning for dual XC
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_EX_AdaptiveTuning(PQ_DEVICE_ID *pDeviceId);
#endif

//-------------------------------------------------------------------------------------------------
/// Adaptive tuning by client
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_AdaptiveTuningByClient(PQ_ADAPTIVE_TUNING_CLIENT enClient);

//-------------------------------------------------------------------------------------------------
/// Film Mode Any Candence Enable
/// @param  bEnable                           \b IN: enable or disable
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_PQ_FilmMode_AnyCandence_Enable(MS_BOOL bEnable);

//-------------------------------------------------------------------------------------------------
/// Load table by context
/// @param  eWindow                             \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_BW_LoadTableByContext(PQ_WIN eWindow);

//-------------------------------------------------------------------------------------------------
/// Bandwidth load initial table
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_BW_LoadInitTable(void);

//-------------------------------------------------------------------------------------------------
/// Set H264 information
/// @param  bEn                                \b IN: it's H264? TRUE or FALSE
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetH264_OnOff(MS_BOOL bEn);

//-------------------------------------------------------------------------------------------------
/// Set G3D information
/// @param  bEn                                \b IN: it's G3D? TRUE or FALSE
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetG3D_OnOff(MS_BOOL bEn);

//-------------------------------------------------------------------------------------------------
/// Set 4kx2k MVC information
/// @param  bEn                                \b IN: it's H264? TRUE or FALSE
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetMVC4kx1k_OnOff(MS_BOOL bEn);

//-------------------------------------------------------------------------------------------------
/// Set Network mm information
/// @param  bEn                                \b IN: it's network mm case? TRUE or FALSE
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetNetworkMM_OnOff(MS_BOOL bEn);


//-------------------------------------------------------------------------------------------------
/// Set MM information
/// @param  bEn                                \b IN: it's MM? TRUE or FALSE
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetMM_OnOff(MS_BOOL bEn);

//-------------------------------------------------------------------------------------------------
/// Set Rmvb information
/// @param  bEn                                \b IN: it's Rmvb? TRUE or FALSE
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetRmvb_OnOff(MS_BOOL bEn, PQ_WIN ePQWindow);

//-------------------------------------------------------------------------------------------------
/// Set VIP all function bypass
/// @param  bEnable                                \b IN: VIP all function bypass TRUE or FALSE
/// @param  eWindow                                \b IN: PQ Window
/// @return MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetVIPBypass_OnOff(MS_BOOL bEnable,PQ_WIN eWindow);

//-------------------------------------------------------------------------------------------------
/// Enable or diable peaking
/// @param  bEnable                          \b IN: Enable or diable peaking
/// @param  eWindow                          \b IN: PQ Window
/// @return  MS_BOOL
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetPeaking_OnOff(MS_BOOL bEnable, PQ_WIN eWindow);

//------------------------------------------------------------------------------
/// Film mode fake out customization setting
/// @return                             \b Out: OK or Fail
//------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_SetFakeOutEnable(MS_BOOL bEnable);
//------------------------------------------------------------------------------------------------
/// Reset BW table to deafult
/// @param NONE
/// @return NONE
//------------------------------------------------------------------------------------------------
void SYMBOL_WEAK MDrv_BW_resetToDefault(void);

//-------------------------------------------------------------------------------------------------
/// Bandwidth load BW table depends on special customer mode
//-------------------------------------------------------------------------------------------------
INTERFACE void SYMBOL_WEAK MDrv_BW_CusMode_LoadTable(MS_U8 u8TabIdx);

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
//-------------------------------------------------------------------------------------------------
/// For Procfs Debug Message
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQMdbIoctl_EchoCmd(MDBCMD_CMDLINE_PARAMETER *paraCmdLine);
INTERFACE void SYMBOL_WEAK MDrv_PQMdbIoctl_Cat(MDBCMD_GETINFO_PARAMETER *paraGetInfo);
#endif

//-------------------------------------------------------------------------------------------------
/// Get FBL status. This function can be used before XC set window to get FBL status. If use DS, you need enable DS before call
/// MDrv_PQ_Get_FBLStatus API.
/// @param  eWindow                             \b IN: TBD
/// @param  enInputSourceType                   \b IN: TBD
/// @param  pstPQModeInfo                       \b IN: TBD
/// @param bDsEnable                       \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_Get_FBLStatus(PQ_WIN eWindow, PQ_INPUT_SOURCE_TYPE enInputSourceType, MS_PQ_Mode_Info *pstPQModeInfo, MS_BOOL bDsEnable);

//-------------------------------------------------------------------------------------------------
/// Get FBL status. This function can be used before XC set window to get FBL status. If use DS, you need enable DS before call
/// MDrv_PQ_Get_ZNRStatus API.
/// @param  eWindow                             \b IN: TBD
/// @param  enInputSourceType                   \b IN: TBD
/// @param  pstPQModeInfo                       \b IN: TBD
/// @param bDsEnable                       \b IN: TBD
//-------------------------------------------------------------------------------------------------
INTERFACE MS_BOOL SYMBOL_WEAK MDrv_PQ_Get_ZNRStatus(PQ_WIN eWindow, PQ_INPUT_SOURCE_TYPE enInputSourceType, MS_PQ_Mode_Info *pstPQModeInfo, MS_BOOL bDsEnable);

//-------------------------------------------------------------------------------------------------
/// For Load PQ bin method
/// @param *pInstance                \b IN:  pInstance
/// @param BinName                   \b IN:  Bin name
/// @param u32FileSize               \b IN:  Bin size
/// @return                          \b Out: PQ bin address
//-------------------------------------------------------------------------------------------------
INTERFACE MS_U8 *MDrv_PQ_LoadPQBinMethod(void* pInstance, char *BinName, MS_U32 *u32FileSize);

//-------------------------------------------------------------------------------------------------
/// Replace PQ bin
/// @param *pInstance                \b IN:  pInstance
/// @param enBinType                 \b IN:  Bin type
/// @param bUnitTest                 \b IN:  Enable or diable UnitTest
/// @return NONE
//-------------------------------------------------------------------------------------------------
INTERFACE void MDrv_PQ_ReplaceBin(void *pInstance, PQ_BIN_TYPE enBinType, MS_BOOL bUnitTest);


#undef INTERFACE

#ifdef __cplusplus
}
#endif

#endif /* _DRVPQ_CUS_H_ */
