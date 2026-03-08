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

#ifndef MAPP_WAKEUP_H
#define MAPP_WAKEUP_H

////////////////////////////////////////////////////////////////////////////////

#ifdef  MAPP_WAKEUP_C
#define WAKEUP_INTERFACE
#else
#define WAKEUP_INTERFACE extern
#endif

////////////////////////////////////////////////////////////////////////////////

WAKEUP_INTERFACE BOOLEAN gSyncDetect;
WAKEUP_INTERFACE BOOLEAN gDviClockDetect;
WAKEUP_INTERFACE U8 u8HDMIWakeUpDelayCount;

//***********************************************************
//              Macro Definitions
//***********************************************************
#define BLOCK_POWER_FUNC                        1
#define ENABLE_SC_BLOCK_SYNC_WAKEUP             0//default set to disable

//SYNC register
#define PMSLEEP_REG_BASE                        (0x0700*2)
#define REG_PM_WK_IRQ_MASK                      ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x08*2))
#define REG_PM_WK_IRQ_FORCE                     ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x08*2))

#define REG_PM_SD_IRQ_MASK                      ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x16*2))
#define REG_PM_SD_IRQ_FORCE                     ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x16*2))
#define REG_PM_SD_IRQ_CLR                       ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x17*2))
#define REG_PM_SD_POL                           ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x17*2))

#define REG_RTC_BASE_0                          (0x1280)

//RTC register
#define RTC_SOFT_RSTZ_BIT                       0x0001
#define RTC_CNT_EN_BIT                          0x0002
#define RTC_WRAP_EN_BIT                         0x0004
#define RTC_LOAD_EN_BIT                         0x0008
#define RTC_READ_EN_BIT                         0x0010
#define RTC_INT_MASK_BIT                        0x0020
#define RTC_INT_FORCE_BIT                       0x0040
#define RTC_INT_CLEAR_BIT                       0x0080

#define REG_RTC_CTRL_REG                        ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x00*2))
#define REG_RTC_FREQ_CW_L                       ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x01*2))
#define REG_RTC_FREQ_CW_H                       ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x02*2))

#define REG_RTC_LOAD_VAL                        ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x03*2))

#define REG_RTC_MATCH_VAL_L                     ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x05*2))
#define REG_RTC_MATCH_VAL_H                     ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x06*2))
    #define RTC_DISABLE_VALUE                   0xFFFFFFFF

#define REG_RTC_INT                             ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x07*2))
#define REG_RTC_CNT                             ((0x00*0x10000) + (REG_RTC_BASE_0 + 0x08*2))


#define REG_PM_DVI_DET_CMP_P0                   ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x2F*2))
#define REG_PM_DVI_DET_CMP_P1                   ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x2F*2 + 1))
#define REG_PM_DVI_CTRL_SW_RST                  ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x31*2))

#define REG_PM_CMPVALUE1_P0                     ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x40*2))
#define REG_PM_CMPVALUE2_P0                     ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x41*2))
#define REG_PM_K_RPT_P0                         ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x42*2))

#define REG_PM_CMPVALUE1_P1                     ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x44*2))
#define REG_PM_CMPVALUE2_P1                     ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x45*2))
#define REG_PM_K_RPT_P1                         ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x46*2))

#define REG_PM_DM_SEL_IB                        ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))
#define REG_PM_DM_SEL_IB_ONEPORT                ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))
#define REG_PM_DM_CLKSEL                        ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))
#define REG_PM_DM_CLKSEL_ONEPORT                ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))
#define REG_PM_PD_IBGREX                        ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))
#define REG_PM_PD_IBGREX_ONEPORT                ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))

#define REG_PM_GCR_SWCK                         ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_GCR_SWCK_OFFL                    ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_GCR_SWCK_ONEPORT                 ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_PD_CLKIN                         ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_PD_CLKIN_OFFL                    ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4A*2))
#define REG_PM_PD_CLKIN_ONEPORT                 ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_PD_RCK                           ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_PD_RCK_ONEPORT                   ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))
#define REG_PM_DVI_SWCK_HW_EN                   ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4B*2))

#define REG_PM_PD_BG                            ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4C*2))
#define REG_PM_PD_BG_ONEPORT                    ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4C*2))
#define REG_PM_DVI_HW_SW_CNT                    ((0x00*0x10000)+ (PMSLEEP_REG_BASE + 0x4D*2))


////////////////////////////////////////////////////////////////////////////////

void MDrv_UART_ChckWakeupEvent(int c);
void MDrv_UART_ConfigWakeup(void);

void MDrv_RTC_Isr(void);
void MDrv_RTC_ConfigWakeup(void);

void MDrv_SYNC_ClearINT(void);
void MDrv_SYNC_Isr(void);
void MDrv_SYNC_KeepStandby(void);
void MDrv_SYNC_ProcessWakeup(void);
void MDrv_SYNC_ConfigWakeup(void);

void MDrv_DVI_Isr(void);
void MDrv_DVI_ProcessWakeup(void);
void MDrv_DVI_ConfigWakeup(void);
void MDrv_DVI_ManualSwitchPort(void);

void MDrv_ProcessMHLWakeup(void);

void MDrv_ProcessUSBWakeup(void);
void MDrv_USBPrecheck(void);
U8 MDrv_CheckUsbStatus(void);

BOOL MDrv_IR_IsWakeup(U8 u8Key);

void MApi_InitCEC(void);
void MApp_CEC_Handler(void);


void MApp_CheckWakeupEvent(void);


// please sync with AP source file: msAPI_Power.h
#define WakeUp_by_IR        0
#define WakeUp_by_CEC       BIT0
#define WakeUp_by_GPIO      BIT1
#define WakeUp_by_RTC       BIT2
#define WakeUp_by_SAR       BIT3
#define WakeUp_by_DDCD      BIT4
#define WakeUp_by_DVI       BIT5
#define WakeUp_by_SYNC      BIT6
#define WakeUp_by_USB       BIT7
#define WakeUp_by_MHL       BIT8
#define WakeUp_by_None      0xFF
WAKEUP_INTERFACE void msAPI_Power_NotifyWakeupDeviceForAP(U16 u16WakeUpDEvice);


// Wake-up device
// please sync with AP source file: msAPI_Power.h
enum
{
    PM_WAKEUP_BY_KEYPAD         = (1 << 0),
    PM_WAKEUP_BY_CEC            = (1 << 1),
    PM_WAKEUP_BY_RTC            = (1 << 2),
    PM_WAKEUP_BY_DDC            = (1 << 3),

    PM_WAKEUP_BY_VGA            = (1 << 4),
    PM_WAKEUP_BY_DVI            = (1 << 5),
    PM_WAKEUP_BY_GPI            = (1 << 6),
    PM_WAKEUP_BY_IR             = (1 << 7),

    PM_WAKEUP_BY_UART           = (1 << 8),
    PM_WAKEUP_BY_USB            = (1 << 9),
    PM_WAKEUP_BY_MHL            = (1 << 10),
    PM_WAKEUP_BY_DVI1           = (1 << 11),
    PM_WAKEUP_BY_DVI2           = (1 << 12),
    
    PM_WAKEUP_BY_ALL            = PM_WAKEUP_BY_KEYPAD
                                | PM_WAKEUP_BY_CEC
                                | PM_WAKEUP_BY_RTC
                                | PM_WAKEUP_BY_DDC
                                | PM_WAKEUP_BY_VGA
                                | PM_WAKEUP_BY_DVI
                                | PM_WAKEUP_BY_GPI
                                | PM_WAKEUP_BY_IR
                                | PM_WAKEUP_BY_UART
                                | PM_WAKEUP_BY_USB
                                | PM_WAKEUP_BY_MHL
                                | PM_WAKEUP_BY_DVI1
                                | PM_WAKEUP_BY_DVI2,
};

void MDrv_CheckWakeupDeviceFromAP(void);






////////////////////////////////////////////////////////////////////////////////

#undef WAKEUP_INTERFACE

#endif // MAPP_WAKEUP_H

