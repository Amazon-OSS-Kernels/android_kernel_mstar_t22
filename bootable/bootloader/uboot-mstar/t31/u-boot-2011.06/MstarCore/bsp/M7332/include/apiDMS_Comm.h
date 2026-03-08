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

#ifndef _API_DMS_COMM_H_
#define _API_DMS_COMM_H_
#include "UFO.h"
////////////////////////////////////////////////////////////////////////////////
/// @file drvDMS.h
/// @brief DMS control driver
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif


//-------------------------------------------------------------------------------------------------
//  Debug
//-------------------------------------------------------------------------------------------------
#ifdef API_DMS_C
#define API_DMS_COMM_INTERFACE
#else
#define API_DMS_COMM_INTERFACE extern
#endif

#define DMS_DBGL_ERR       (0x0001)
#define DMS_DBGL_WARNING  (0x0002)
#define DMS_DBGL_NOTICE  (0x0004)
#define DMS_DBGL_INFO       (0x0008)
#define DMS_DBGL_FLIP       (0x0010)
#define DMS_DBGL_BUF_CTRL  (0x0020)
#define DMS_DBGL_API_FLOW  (0x0040)
#define DMS_DBGL_TIME  (0x0080)
#define DMS_DBGL_AIPQ  (0x0100)
#define DMS_DBGL_SVP  (0x0200)
#define DMS_DBGL_ALL  (0xFFFF)

#ifndef ANDROID
#if defined(MSOS_TYPE_LINUX_KERNEL)
#define sysprint printk
#else
#define sysprint printf
#endif
//don't use DMS_PRINF in you code, use DMS_LEVEL_PRINTF instead.
#define DMS_PRINTF sysprint
#else
#include <sys/mman.h>
#include <cutils/ashmem.h>
#include <cutils/log.h>
//don't use DMS_PRINF in you code, use DMS_LEVEL_PRINTF instead.
#define DMS_PRINTF ALOGD
#endif

API_DMS_COMM_INTERFACE MS_U16 gu16DMSDbgSwitch_InternalUseOnly;

#define DMS_LEVEL_PRINTF(_dbgSwitch_,_fmt, _args...)\
{\
    if( (_dbgSwitch_ & gu16DMSDbgSwitch_InternalUseOnly) != 0 )  \
    {\
        if (_dbgSwitch_ & DMS_DBGL_ERR)                \
        {\
            DMS_PRINTF("[DMS] DMS_ERROR [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_WARNING)\
        {\
            DMS_PRINTF("[DMS] DMS_WARNING [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_INFO)\
        {\
            DMS_PRINTF("[DMS] DMS_INFO [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if (_dbgSwitch_ & DMS_DBGL_FLIP)                \
        {\
            DMS_PRINTF("[DMS] DMS_FLIP [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_BUF_CTRL)\
        {\
            DMS_PRINTF("[DMS] DMS_BUFCTRL [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_API_FLOW)\
        {\
            DMS_PRINTF("[DMS] DMS_FLOW [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_TIME)\
        {\
            DMS_PRINTF("[DMS] DMS_TIME [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_NOTICE)\
        {\
            DMS_PRINTF("[DMS] DMS_NOTICE [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_AIPQ)\
        {\
            DMS_PRINTF("[DMS] DMS_AIPQ [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else if(_dbgSwitch_ & DMS_DBGL_SVP)\
        {\
            DMS_PRINTF("[DMS] DMS_SVP [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
        else\
        {\
            DMS_PRINTF("[DMS] DMS_LEVEL_PRINTF NOT DEFINE [%s,%d]: " _fmt, __FUNCTION__,__LINE__,##_args);  \
        }\
    }\
}

#endif //end _API_DMS_COMM_H_
