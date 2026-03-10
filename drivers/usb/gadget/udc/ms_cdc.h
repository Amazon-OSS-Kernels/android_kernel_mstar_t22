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
 *	notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in
 *	the documentation and/or other materials provided with the
 *	distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *	contributors may be used to endorse or promote products derived
 *	from this software without specific prior written permission.
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

//------------------------------------------------------------------------------
// FILE
//	  ms_cdc.h
//
// DESCRIPTION
//
// HISTORY
//
//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include "ms_gvar.h"

/* Line Coding Structure */
#define LINE_CODING_LENGTH			0x07
#define dummy_length				0x08

extern u32 volatile cdcRxBufAlen;  //Benson Add
extern u32 volatile cdcRxBufBlen;  //Benson Add

typedef union _LINE_CODING {
	struct {
		unsigned char _byte[LINE_CODING_LENGTH];
	} line_coding1;

	struct {
		unsigned int    dwDTERate;		  // Complex data structure
		unsigned char	bCharFormat;
		unsigned char	bParityType;
		unsigned char	bDataBits1;
	} line_coding2;

	struct {
		unsigned short   wDTERate1;		  // Complex data structure
		unsigned short   wDTERate2;		  // Complex data structure
		unsigned short   Format_Parit;
		unsigned char	bDataBits2;
	} line_coding3;
} LINE_CODING;

typedef union _CONTROL_SIGNAL_BITMAP {
	unsigned char _byte;
	struct {
		unsigned DTE_PRESENT;	   // [0] Not Present  [1] Present
		unsigned CARRIER_CONTROL;   // [0] Deactivate   [1] Activate
	};
} CONTROL_SIGNAL_BITMAP;

typedef union _CDC_REQUEST_FLAG {
	unsigned char _byte;
	struct {
		unsigned LINE_CODING : 1;
		unsigned LINE_STATE  : 1;
		unsigned BUFFA_FULL  : 1;
		unsigned BUFFB_FULL  : 1;
	} cdc_req_flag;
} CDC_REQUEST_FLAG;

s32 USBCheckCDCRequest(USB_INFO_st *pUsbInfo);

extern unsigned char dummy_encapsulated_cmd_response[dummy_length];
extern LINE_CODING volatile line_coding;	// Buffer to store line coding information
extern CONTROL_SIGNAL_BITMAP volatile control_signal_bitmap;
extern CDC_REQUEST_FLAG volatile cdc_request_flag;

#ifdef __cplusplus
}
#endif  //__cplusplus
