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

#ifndef _DRVCOMMON_VR_H_
#define _DRVCOMMON_VR_H_

#ifdef _MDRV_COMMON_VR_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
#ifdef __cplusplus
extern "C"
{
#endif


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Write 2 Byte for VR
/// @param  u32Reg
/// @param  u16Val
/// @return NONE
//-------------------------------------------------------------------------------------------------
INTERFACE void MDrv_COMM_VR_W2BYTE(MS_U32 u32Reg, MS_U16 u16Val);
//-------------------------------------------------------------------------------------------------
/// Read 2 Byte for VR
/// @param  u32Reg
/// @return u16Val
//-------------------------------------------------------------------------------------------------
INTERFACE MS_U16 MDrv_COMM_VR_R2BYTE(MS_U32 u32Reg);
//-------------------------------------------------------------------------------------------------
/// Read Byte for VR
/// @param  u32Reg
/// @return u8Val
//-------------------------------------------------------------------------------------------------
INTERFACE MS_U8 MDrv_COMM_VR_RBYTE(MS_U32 u32Reg);
//-------------------------------------------------------------------------------------------------
/// Read 2 Byte for VR (Mask)
/// @param  u32Reg
/// @param  u16Mask
/// @return u16Val
//-------------------------------------------------------------------------------------------------
INTERFACE MS_U16 MDrv_COMM_VR_R2BYTEMSK(MS_U32 u32Reg, MS_U16 u16Mask);
//-------------------------------------------------------------------------------------------------
/// Write Byte for VR
/// @param  u32Reg
/// @param  u8Val
/// @return NONE
//-------------------------------------------------------------------------------------------------
INTERFACE void MDrv_COMM_VR_WBYTE(MS_U32 u32Reg, MS_U8 u8Val);
//-------------------------------------------------------------------------------------------------
/// Write 2 Byte for VR (Mask)
/// @param  u32Reg
/// @param  u16Mask
/// @param  u16Val
/// @return NONE
//-------------------------------------------------------------------------------------------------
INTERFACE void MDrv_COMM_VR_W2BYTEMSK(MS_U32 u32Reg, MS_U16 u16Val, MS_U16 u16Mask);

#undef INTERFACE

#ifdef __cplusplus
}
#endif

#endif /* _DRVCOMMON_VR_H_ */
