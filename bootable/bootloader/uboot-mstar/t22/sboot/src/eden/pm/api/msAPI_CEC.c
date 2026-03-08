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
/// file    msAPI_CEC.c
/// @brief  HDMI CEC API Interface
///////////////////////////////////////////////////////////////////////////////////////////////////

#define MSAPI_CEC_C

#include <stdio.h>
#include "Board.h"
#include "datatype.h"
#include "drvGlobal.h"
#include "hwreg.h"
#include "MsCommon.h"
#include "apiCEC.h"
#include "msAPI_CEC.h"
#include "MApp_Wakeup.h"

//**************************************************************************

#define CEC_DPRINTF(x)     //x

///////////////////////////////////////////////////////////////////////////////////////////////////

extern void SystemWholeChipReset(void);

///////////////////////////////////////////////////////////////////////////////////////////////////


#define ONE_TOUCH_PLAY_SUPPORT          1
#define ROUTING_CONTROL_SUPPORT         1
#define STANDBY_COMMAND_SUPPORT         1
#define SYSTEM_INFORMATION_SUPPORT      1


//**************************************************************************
//  [Function Name]:
//                   MsAPI_CecInit()
//  [Description]
//                  CEC initial sequence
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void msAPI_CEC_Init(void)
{
    MS_CEC_INIT_INFO stCECInfo;

    stCECInfo.u32XTAL_CLK_Hz = FREQ_12MHZ;

    MApi_CEC_InitChip(&stCECInfo);
}

//**************************************************************************
//  [Function Name]:
//                   MsAPI_CECProcessCmd()
//  [Description]
//                   It used to parse CEC command
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void msAPI_CEC_ProcessCmd(void)
{
    U8 header, para0, para1, para2, len;
    MsCEC_MSGLIST opcode;
    MsCEC_DEVICELA enDevice;
    U8 u8Fifoidx;
    BOOLEAN bWakeup = FALSE;

    header = MApi_CEC_GetRxData( MApi_CEC_GetFifoIdx(), 0);
    opcode = (MsCEC_MSGLIST)  MApi_CEC_GetRxData( MApi_CEC_GetFifoIdx(), 1);
    para0 =  MApi_CEC_GetRxData( MApi_CEC_GetFifoIdx(), 2);
    para1 =  MApi_CEC_GetRxData( MApi_CEC_GetFifoIdx(), 3);
    para2 =  MApi_CEC_GetRxData( MApi_CEC_GetFifoIdx(), 4);
    len = MApi_CEC_GetCmdLen();
    enDevice = (MsCEC_DEVICELA)( (header & 0xF0) >> 4 );

    if (MApi_CEC_GetMsgCnt() == 0 )
    {
        return;  //no received message to process
    }

    CEC_DPRINTF(printf("[CEC] header = 0x%x \r\n", header));
    CEC_DPRINTF(printf("[CEC] opcode = 0x%x \r\n", opcode));
    CEC_DPRINTF(printf("[CEC] para0  = 0x%x \r\n", para0));
    CEC_DPRINTF(printf("[CEC] para1  = 0x%x \r\n", para1));
    CEC_DPRINTF(printf("[CEC] para2  = 0x%x \r\n", para2));
    CEC_DPRINTF(printf(" len      = %d \r\n", len));
    CEC_DPRINTF(printf(" enDevice = 0x%x \r\n", enDevice));

    switch (opcode)
    {

    //----- One Touch Play ---------------
    #if ONE_TOUCH_PLAY_SUPPORT

        case E_MSG_ACTIVE_SOURCE:             //broadcast, if addressed, ignored
            CEC_DPRINTF(printf("got E_MSG_ACTIVE_SOURCE \r\n"));
            if (MApi_CEC_CheckFrame(E_TRANS_BROADCAST_MSG, 4)==0)   break;

            CEC_DPRINTF(printf("Active Source \r\n"));
            bWakeup = TRUE;
            break;

        case E_MSG_OTP_IMAGE_VIEW_ON:     //if in standby, firstly power on
            CEC_DPRINTF(printf("got E_MSG_OTP_IMAGE_VIEW_ON \r\n"));
            if (MApi_CEC_CheckFrame(E_TRANS_DIRECT_MSG, 2)==0)  break;

            CEC_DPRINTF(printf("Image View On \r\n"));
            bWakeup = TRUE;
            break;

        case E_MSG_OTP_TEXT_VIEW_ON:     //if in standby, firstly power on
            CEC_DPRINTF(printf("got E_MSG_OTP_TEXT_VIEW_ON \r\n"));
            if (MApi_CEC_CheckFrame(E_TRANS_DIRECT_MSG, 2)==0)  break;

            CEC_DPRINTF(printf("Text View On \r\n"));
            bWakeup = TRUE;
            break;

    #endif
    //----- One Touch Play end ---------------


    //----- System Information -----------
    #if SYSTEM_INFORMATION_SUPPORT

        case E_MSG_SI_REQUEST_PHY_ADDR:
            CEC_DPRINTF(printf("got E_MSG_SI_REQUEST_PHY_ADDR \r\n"));
            if (MApi_CEC_CheckFrame(E_TRANS_DIRECT_MSG, 2)==0)  break;

            CEC_DPRINTF(printf("Report Power Status \r\n"));
            MApi_CEC_Msg_ReportPhycalAddress();
            break;

        case E_MSG_SI_REPORT_PHY_ADDR:
        {
            MS_U8 txbuf[2];

            txbuf[0] = para0;
            txbuf[1] = para1;

            CEC_DPRINTF(printf("got E_MSG_SI_REPORT_PHY_ADDR \r\n"));
            if(MApi_CEC_CheckFrame(E_TRANS_BROADCAST_MSG, 5) == 0) break;

            MApi_CEC_SetActivePhysicalAddress(para0, para1);
            CEC_DPRINTF(printf(" = SetActivePhysicalAddress 0x%x", para0));
            CEC_DPRINTF(printf(" 0x%x = \r\n", para1));
            MApi_CEC_SetActiveDeviceType((MsCEC_DEVICE_TYPE) para2);
            CEC_DPRINTF(printf(" = SetActiveDeviceType 0x%x = \r\n", para2));

            if (enDevice != E_LA_BROADCAST)
            {
                MApi_CEC_TxSendMsg(MApi_CEC_GetActiveLogicalAddress(), E_MSG_OSDNT_GIVE_OSD_NAME, 0, 0);
            }
        }
            break;
    #endif
    //----- System Information end -----------

// CEC 11.1.14-2
        case E_MSG_PS_GIVE_POWER_STATUS:
        {
            MS_U8 txbuf[1];
            CEC_DPRINTF(printf("got E_MSG_PS_GIVE_POWER_STATUS \r\n"));
            if (MApi_CEC_CheckFrame(E_TRANS_DIRECT_MSG, 2) == 0) break;

            txbuf[0] = E_MSG_PWRSTA_STANDBY;

            MApi_CEC_TxSendMsg(enDevice, E_MSG_PS_REPORT_POWER_STATUS, txbuf, 1);
        }
            break;

        default:
            CEC_DPRINTF(printf("Keep Status \r\n"));
            break;
    }

    if (bWakeup)
    {
        msAPI_Power_NotifyWakeupDeviceForAP(WakeUp_by_CEC);
        SystemWholeChipReset();
    }

    u8Fifoidx = MApi_CEC_GetFifoIdx();

    if((u8Fifoidx+1)>=CEC_FIFO_CNT)
        MApi_CEC_SetFifoIdx(0);
    else
        MApi_CEC_SetFifoIdx(u8Fifoidx+1);

    MApi_CEC_SetMsgCnt(MApi_CEC_GetMsgCnt() - 1);
}

//**************************************************************************
//  [Function Name]:
//                   MsAPI_CecCecHandler()
//  [Description]
//                   CEC handler
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void msAPI_CEC_Handler(void)
{
#if(!ENABLE_CEC_INT) // if not enable CEC interrupt, use sw polling to instead of
    MApi_CEC_ChkRxBuf();
#endif

    msAPI_CEC_ProcessCmd();
}



////////////////////////////////////////////////////////////////////////////////
#undef MSAPI_CEC_C

