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

#define MAPP_MAIN_C

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "datatype.h"
#include "hwreg.h"
#include "drvGlobal.h"
#include "drvAeonUART.h"
#include "risc32_spr.h"
#include "drvPadConf.h"
#include "drvISR.h"

//###############
//
//  utopia include files
//
//###############
#include "MsCommon.h"
#include "drvMMIO.h"
#include "drvBDMA.h"
#include "drvWDT.h"
#include "drvGPIO.h"
#include "drvUART.h"
#include "drvPM.h"

//###############
//
//  other include files
//
//###############
#include "MApp_IR.h"
#include "MApp_Wakeup.h"
#include "uartdebug.h"

//------------------------------------------------------------------------------
// Locals
//------------------------------------------------------------------------------
//Open this definition to avoid message interference while using mstv_tool to read registers
//#define MSTV_TOOL_DBG

#define HEART_BEAT
#ifdef HEART_BEAT
#define HEART_BEAT_TIME     5
#endif
//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

int main(void)
{
    U32 u32LoopCount=0;

    // system initialization for customization
    // turn ON WDT
    //######################
    //
    // Put Initialized Functions below
    //
    //######################

    MApp_IR_Init();

    MApp_Keypad_Init();

    printf(" Enter NASA PM Standby\r\n");

    while(1)
    {
        #ifndef MSTV_TOOL_DBG
        if ( msAPI_UART_DecodeCommand() )
        {
            continue;
        }
        #endif

        MApp_CheckWakeupEvent();

        #ifdef HEART_BEAT
        if ( gSystemTimeCount >= (u32LoopCount+HEART_BEAT_TIME))
        {
            printf("\r\n [NASA PM] \r\n");
            u32LoopCount = gSystemTimeCount;
        }
        #endif
    }//main while end

    return 0;
}




////////////////////////////////////////////////////////////////////////////////
#undef MAPP_MAIN_C

