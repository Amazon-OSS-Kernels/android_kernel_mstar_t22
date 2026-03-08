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

#ifndef _DRVMDLA_H_
#define _DRVMDLA_H_

////////////////////////////////////////////////////////////////////////////////
/// @file drvMDLA.h
/// @brief MDLA control driver
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

#include "MsOS.h"
#include "MsTypes.h"
#include "MsDevice.h"
#include "UFO.h"

#define MDLA_UTOPIA20       (1)

////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////
typedef struct _MDLA_BUF
{
	MS_U32	size;
	MS_U32	mva;
	MS_U64	pa;
	MS_U64	kva;
	MS_U32	id;
	MS_U8	type;
	MS_U32	mva_h;
} MDLA_BUF;

typedef struct _MDLA_CMD
{
	MS_U64	kva;
	MS_U32	mva;
	MS_U32	count;
	MS_U32	id;
	MS_U32	mva_h;
} MDLA_CMD;

typedef struct _MDLA_CONFIG
{
	MS_U32	op;
	MS_U32	arg_count;
	MS_U64	arg[8];
} MDLA_CONFIG;

////////////////////////////////////////////////////////////////////////////////
// Extern Function
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// include utopia v2  header files here
////////////////////////////////////////////////////////////////////////////////

SYMBOL_WEAK MS_BOOL MDrv_MDLA_Malloc(MDLA_BUF *pBuf);
SYMBOL_WEAK MS_BOOL MDrv_MDLA_Free(MDLA_BUF *pBuf);
SYMBOL_WEAK MS_BOOL MDrv_MDLA_Run_Cmd_Sync(MDLA_CMD *pCmd);
SYMBOL_WEAK MS_BOOL MDrv_MDLA_Run_Cmd_Async(MDLA_CMD *pCmd);
SYMBOL_WEAK MS_BOOL MDrv_MDLA_Wait_Cmd(MDLA_CMD *pCmd);
SYMBOL_WEAK MS_BOOL MDrv_MDLA_GSM_Info(MDLA_BUF *pBuf);
SYMBOL_WEAK MS_BOOL MDrv_MDLA_EFUSE_Info(MS_BOOL *pSupport);

#ifdef __cplusplus
}
#endif
#endif
