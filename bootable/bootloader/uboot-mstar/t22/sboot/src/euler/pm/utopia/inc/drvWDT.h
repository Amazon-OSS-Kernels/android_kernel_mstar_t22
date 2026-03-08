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
/// @brief  Piu Watch Dog Timer Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_WDT_H_
#define _DRV_WDT_H_



#ifdef __cplusplus
extern "C"
{
#endif

#include "MsTypes.h"

////////////////////////////////////////////////////////////////////////////////
//  Type and Structure
////////////////////////////////////////////////////////////////////////////////
#define MSIF_WDT_LIB_CODE              {'W','D','T','_'}    //Lib code
#define MSIF_WDT_LIBVER                {'0','1'}            //LIB version
#define MSIF_WDT_BUILDNUM              {'0','4'}            //Build Number
#define MSIF_WDT_CHANGELIST            {'0','0','3','8','2','1','7','1'} //P4 ChangeList Number

#define WDT_DRV_VERSION                /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_WDT_LIB_CODE,                  /* IP__                                             */  \
    MSIF_WDT_LIBVER,                    /* 0.0 ~ Z.Z                                        */  \
    MSIF_WDT_BUILDNUM,                  /* 00 ~ 99                                          */  \
    MSIF_WDT_CHANGELIST,                /* CL#                                              */  \
    MSIF_OS

#define MAX_TIMER_NUM			2

typedef enum _WDT_DbgLv
{
    E_WDT_DBGLV_NONE,			/* no debug message */
    E_WDT_DBGLV_ERR_ONLY,		/* show error only */
    E_WDT_DBGLV_INFO,			/* show error & informaiton */
    E_WDT_DBGLV_ALL			/* show error, information & funciton name */
}WDT_DbgLv;

typedef enum
{
    E_WDT_OK,
    E_WDT_FAIL,
} WDT_Result;

typedef enum _E_PIU_Timer
{
    E_TIMER_0,
    E_TIMER_1,
} E_PIU_Timer;

typedef struct _tmr_interrupt {
	MS_U32 		u32TmrInit;						/* Initial value of timer 						*/
 	MS_U32 		u32TmrMax;						/* Max value of timer 						*/
	void 		(*TmrFnct)(void *, void *, void *);	/* Function to execute when timer times out	*/
	void			*TmrFnctArg0;					/* Arguments supplied to user defined function 	*/
	void			*TmrFnctArg1;					/* Arguments supplied to user defined function 	*/
	void			*TmrFnctArg2;					/* Arguments supplied to user defined function 	*/
	MS_BOOL 	bTmrEn;							/* Flag indicating whether timer is enable 		*/
}tmr_interrupt;

////////////////////////////////////////////////////////////////////////////////
//  Function and Variable
////////////////////////////////////////////////////////////////////////////////
WDT_Result MDrv_WDT_GetLibVer(const MSIF_Version **ppVersion);
WDT_Result MDrv_WDT_Init(WDT_DbgLv eLevel);
WDT_Result MDrv_WDT_Stop(WDT_DbgLv eLevel);
WDT_Result MDrv_WDT_Clear(void);
WDT_Result MDrv_WDT_ClearRstFlag(void);
WDT_Result MDrv_WDT_IsReset(void);
WDT_Result MDrv_WDT_IsEnable(void);
WDT_Result MDrv_WDT_SetTimer(WDT_DbgLv eLevel, MS_U16 sec);
WDT_Result MDrv_WDT_SetTimer_ms(WDT_DbgLv eLevel, MS_U16 msec);
WDT_Result MDrv_WDT_SetTimer_us(WDT_DbgLv eLevel, MS_U16 usec);
WDT_Result MDrv_WDT_SetIntTimer(WDT_DbgLv eLevel, MS_U16 sec);

void MDrv_TIMER_Init(void);
void MDrv_TIMER_CfgFnct(E_PIU_Timer eTimer, void (*fnct)(void *, void *, void *), void *arg0, void *arg1, void *arg2 );
void MDrv_TIMER_Count(E_PIU_Timer eTimer, MS_BOOL bEnable);
void MDrv_TIMER_INT(E_PIU_Timer eTimer, MS_BOOL bEnable);
void MDrv_TIMER_Rst(E_PIU_Timer eTimer);
void MDrv_TIMER_SetMaxMatch(E_PIU_Timer eTimer, MS_U32 u32MaxTimer);
MS_BOOL MDrv_TIMER_HitMaxMatch(E_PIU_Timer eTimer);
MS_U32 MDrv_TIMER_GetMaxMatch(E_PIU_Timer eTimer);
MS_U32 MDrv_TIMER_GetCounter(E_PIU_Timer eTimer);
MS_U32 MDrv_TIMER_GetSecond(E_PIU_Timer eTimer);
MS_U32 MDrv_TIMER_GetMs(E_PIU_Timer eTimer);
MS_U32 MDrv_TIMER_GetUs(E_PIU_Timer eTimer);
void MDrv_TIMER_Delay(E_PIU_Timer eTimer, MS_U32 u32Second);
void MDrv_TIMER_DelayMs(E_PIU_Timer eTimer, MS_U32 u32MS);
void MDrv_TIMER_DelayUs(E_PIU_Timer eTimer, MS_U32 u32US);
void MDrv_TIMER_Exit(void);

#ifdef __cplusplus
}
#endif


#endif // _DRV_WDT_H_

