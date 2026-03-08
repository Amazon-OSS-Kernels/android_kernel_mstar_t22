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
#ifndef XC_SYS_H
#define XC_SYS_H

////////////////////////////////////////////////////////////////////////////////

#include "Enum_Device.h"
#include "drvXC_IOPort.h"
#include "apiXC.h"
#include "xc_Analog_Reg.h"
#include "apiXC_ModeParse.h"
#include "apiXC_Cus.h"
#include "MApp_Wakeup.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef  _XC_SYS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

////////////////////////////////////////////////////////////////////////////////


typedef struct
{
    XC_MODEPARSE_RESULT eModeParseResult;           ///< the result after mode parse

    XC_MODEPARSE_INPUT_INFO sModeParseInputInfo;    ///< prepare the input info for mode parse

    MS_U16 u16Hperiod;
} PCMode_Info;
INTERFACE PCMode_Info gXC_PCMode_Info;

////////////////////////////////////////////////////////////////////////////////

#define MST_XTAL_CLOCK_HZ           FREQ_12MHZ

#define MST_H_PERIOD_MASK           0x3FFF
#define MST_V_TOTAL_MASK            0x0FFF

////////////////////////////////////////////////////////////////////////////////

#define ENABLE_IP_AUTO_COAST        0
#define ENABLE_VGA_EIA_TIMING       0
#define ENABLE_VGA_JUST_CHECK_SYNC  0


#if ENABLE_VGA_EIA_TIMING
#define MIN_HFREQ_OF_RGB            140
#else
#define MIN_HFREQ_OF_RGB            270
#endif
#define MAX_HFREQ_OF_RGB            950

#if ENABLE_VGA_EIA_TIMING
#define MIN_VFREQ_OF_RGB            240
#else
#define MIN_VFREQ_OF_RGB            470
#endif

#if ENABLE_RGB_SUPPORT_85HZ
#define MAX_VFREQ_OF_RGB            870
#else
#define MAX_VFREQ_OF_RGB            770
#endif

#define MIN_VTOTAL_OF_RGB           415
#define MAX_VTOTAL_OF_RGB           1550

// Range of DVI OOR
#define MIN_HFREQ_OF_DVI            140
#define MAX_HFREQ_OF_DVI            950
#define MIN_VFREQ_OF_DVI            230
#define MAX_VFREQ_OF_DVI            770
#define MIN_VTOTAL_OF_DVI           250
#define MAX_WIDTH_OF_DVI            1930
#define MAX_VTOTAL_OF_DVI           1300
#define MAX_HEIGHT_OF_DVI           1210

////////////////////////////////////////////////////////////////////////////////

// NONPM
#define REG_MIU0_BASE               0x101200
#define REG_MIU1_BASE               0x100600
#define REG_CHIPTOP_BASE            0x100B00
#define REG_UHC0_BASE               0x102400
#define REG_UHC1_BASE               0x100D00
#define REG_ADC_ATOP_BASE           0x102500
#define REG_ADC_DTOP_BASE           0x102600
#define REG_HDMI_BASE               0x102700
#define REG_IPMUX_BASE              0x102E00
#define REG_SCALER_BASE             0x102F00
#define REG_LPLL_BASE               0x103100
#define REG_MOD_BASE                0x103200
#define REG_AFEC_BASE               0x103500
#define REG_COMB_BASE               0x103600

#define REG_DVI_ATOP_BASE           0x110900
#define REG_DVI_DTOP_BASE           0x110A00
#define REG_DVI_EQ_BASE             0x110A80     // EQ started from 0x80
#define REG_HDCP_BASE               0x110AC0     // HDCP started from 0xC0
#define REG_ADC_DTOPB_BASE          0x111200     // ADC DTOPB





////////////////////////////////////////////////////////////////////////////////

INTERFACE void msDelayTime(U32 u32Delay);

INTERFACE void _MApi_XC_Sys_Init_XC(void);

INTERFACE XC_MODEPARSE_RESULT XC_GetModeParseResult(void);
INTERFACE void XC_Get_ModeStatus(void);

#if ENABLE_VGA_JUST_CHECK_SYNC
INTERFACE BOOLEAN XC_Check_HVSync(void);
INTERFACE void XC_Clear_HVSyncCount(void);
#endif

#if ENABLE_SC_BLOCK_SYNC_WAKEUP
INTERFACE void XC_Recovery_VGA(void);
#endif

////////////////////////////////////////////////////////////////////////////////

#undef INTERFACE

#endif // XC_SYS_H

