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

#ifndef __DRVGPIO_H__
#define __DRVGPIO_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include "MsTypes.h"

#define MSIF_GPIO_LIB_CODE                     {'G','P','I','O'}    //Lib code
#define MSIF_GPIO_LIBVER                       {'0','3'}            //LIB version
#define MSIF_GPIO_BUILDNUM                     {'0','6'}            //Build Number
#define MSIF_GPIO_CHANGELIST                   {'0','0','5','5','7','0','9','3'} //P4 ChangeList Number

#define GPIO_DRV_VERSION                /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_GPIO_LIB_CODE,                        /* IP__                                             */  \
    MSIF_GPIO_LIBVER,                          /* 0.0 ~ Z.Z                                        */  \
    MSIF_GPIO_BUILDNUM,                        /* 00 ~ 99                                          */  \
    MSIF_GPIO_CHANGELIST,                      /* CL#                                              */  \
    MSIF_OS

typedef enum _GPIO_DbgLv
{
    E_GPIO_DBGLV_NONE,          //no debug message
    E_GPIO_DBGLV_ERR_ONLY,      //show error only
    E_GPIO_DBGLV_REG_DUMP,      //show error & reg dump
    E_GPIO_DBGLV_INFO,          //show error & informaiton
    E_GPIO_DBGLV_ALL            //show error, information & funciton name
}GPIO_DbgLv;

typedef enum _GPIO_Result
{
    E_GPIO_NOT_SUPPORT = -1,
    E_GPIO_FAIL = 0,
    E_GPIO_OK = 1
}GPIO_Result;

typedef struct _GPIO_Info
{
    MS_U32      u32GPIONum;
    MS_U32      u32IOMap;
    MS_U32      u32IOMap_PM;
}GPIO_Info;

typedef struct _GPIO_Status
{
    MS_U8   u8DbgLevel;
    MS_BOOL bInit;
}GPIO_Status;

typedef int MS_GPIO_NUM;        ///< GPIO handle, for default operations,


MS_BOOL MDrv_GPIO_SetIOMapBase(void);

GPIO_Result MDrv_GPIO_GetLibVer(const MSIF_Version **ppVersion);
GPIO_Result MDrv_GPIO_SetDbgLevel(GPIO_DbgLv eLevel);
const GPIO_Info* MDrv_GPIO_GetInfo(void);
void MDrv_GPIO_GetStatus(GPIO_Status *pStatus);

void mdrv_gpio_init(void);
void mdrv_gpio_set_high(MS_GPIO_NUM gpio);
void mdrv_gpio_set_low(MS_GPIO_NUM gpio);
void mdrv_gpio_set_input(MS_GPIO_NUM gpio);
//void mdrv_gpio_set_output(MS_GPIO_NUM gpio);
int mdrv_gpio_get_inout(MS_GPIO_NUM gpio);
int mdrv_gpio_get_level(MS_GPIO_NUM gpio);

typedef enum _GPIO_Edge
{
    E_GPIO_RISING_EDGE,
    E_GPIO_FALLING_EDGE
} GPIO_Edge;

/// Callback function which is called in GPIO ISR.
typedef void ( *GPIO_Callback ) (void);

GPIO_Result mdrv_gpio_attach_interrupt(MS_GPIO_NUM gpio_num,GPIO_Edge gpio_edge_type,GPIO_Callback pCallback);
GPIO_Result mdrv_gpio_detach_interrupt(MS_GPIO_NUM gpio_num);
void mdrv_gpio_interrupt_action(void);
void mdrv_gpio_enable_interrupt_all(void);
void mdrv_gpio_disable_interrupt_all(void);
GPIO_Result mdrv_gpio_enable_interrupt(MS_GPIO_NUM gpio);
GPIO_Result mdrv_gpio_disable_interrupt(MS_GPIO_NUM gpio);


#ifdef __cplusplus
}
#endif

#endif /* __DRVGPIO_H__ */

