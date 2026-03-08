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
//
////////////////////////////////////////////////////////////////////////////////

#define _DRV_ISR_C_

#include <stdio.h>
#include <string.h>
#include "risc32_spr.h"
#include "hwreg.h"
#include "drvGlobal.h"
#include "drvISR.h"
#include "uartdebug.h"


//###########
//
// ISR Functions
//
//##########
static void ProcessCPU0toCPU2Isr(MHAL_SavedRegisters *pHalReg, U32 vector)
{
    UNUSED(pHalReg);
    UNUSED(vector);

    MsOS_EnableInterrupt(E_INT_FIQ_8051_TO_BEON);
}

static void ProcessCPU1toCPU2Isr(MHAL_SavedRegisters *pHalReg, U32 vector)
{
    UNUSED(pHalReg);
    UNUSED(vector);

	MsOS_EnableInterrupt(E_INT_FIQ_AEON_TO_BEON);
}

static void MDrv_Sys_DisableWatchDog(void)
{
    MDrv_Write2Byte(0x3008, 0x0000);
    MDrv_Write2Byte(0x300A, 0x0000);
}

static void TimerISR(void)
{
    gTimerCount0++;
    gu8100msTH++;

    // Use a threshold to check the 100 ms. If the threshold
    // is greater or equal than the 100 ms. Increase the 100 ms
    // counter.
    if (gu8100msTH >= 100)
    {
        g100msTimeCount++;
        gu8100msTH = 0;
    }

    //--------------------------------

    if (gTimerCount0%1000==0)
    {
        gSystemTimeCount++;
    }

    //--------------------------------
    //down time counter:
    if ( gTimerDownCount0 > 0 )
        gTimerDownCount0--;

    if ( gTimerDownCount1 > 0 )
        gTimerDownCount1--;
    //-------------------------------

#if (ENABLE_MSTV_UART_DEBUG )
    if (g_Uart0CheckTick)
    {
        --g_Uart0CheckTick;
        if ( g_Uart0CheckTick == 0 )
        {
            g_bUart0Detected = FALSE;
            g_UartCommand.Index = 0;
        }
    }
#endif

#if(USE_2ND_UART) 
    if (g_Uart2CheckTick)
    {
        --g_Uart2CheckTick;
        if ( g_Uart2CheckTick == 0 )
        {
            g_bUart2Detected = FALSE;
            g_Uart2Command.Index = 0;
        }
    }
#endif

}

//################
//
//  Global API Functions
//
//################
void ProcessSysTrap(MHAL_SavedRegisters *pHalReg, U32 vector)
{
   printf("system trap");

   UNUSED(pHalReg);
   UNUSED(vector);

   MDrv_Sys_DisableWatchDog();
   while(1);
}

void MDrv_ISR_Init(void)
{
    MsOS_AttachInterrupt(E_INT_FIQ_8051_TO_BEON, (InterruptCb)ProcessCPU0toCPU2Isr);
    MsOS_AttachInterrupt(E_INT_FIQ_AEON_TO_BEON, (InterruptCb)ProcessCPU1toCPU2Isr);
}

void MDrv_Timer_ISR_Register(void)
{
    MS_U32 check_timer_status;
    check_timer_status = MsOS_CreateTimer( (TimerCb)TimerISR,
                                                  0,
                                                  1,
                                                  TRUE,
                                                  (char*)"Check timer");
}

void MDrv_Timer_Init(void) // Now support mode 1
{
    gTimerCount0 = 0;
    gTimerDownCount0 = 0;
    gTimerDownCount1 = 0;
    gu8100msTH = 0;
    g_u16ElapsedTimeSecond = 0;
}

void MDrv_Interrupt_Init(void)
{
    MsOS_CPU_AttachException( E_EXCEPTION_TRAP, ProcessSysTrap, E_EXCEPTION_TRAP );
    MsOS_CPU_UnMaskInterrupt( E_INTERRUPT_FIQ );
    MsOS_CPU_UnMaskInterrupt( E_INTERRUPT_IRQ );
    MsOS_CPU_EnableInterrupt();
}

