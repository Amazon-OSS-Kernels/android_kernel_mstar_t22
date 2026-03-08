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

#include "../../inc/common/drvNAND.h"


extern U32  NC_WaitComplete_dram(U32 u32_EventReg, U16 u16_WaitEvent, U32 u32_MicroSec);
extern void *drvNAND_get_DrvContext_address_dram(void); // exposed API


U32 NC_Micron_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);

U32 NC_SendReadRetryCmd(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address_dram();

	//REG_WRITE_UINT16(NC_CF_SSO_CTL, 0xDEAD);
	//REG_WRITE_UINT16(NC_CF_SSO_CTL, pNandDrv->au8_ID[0]);

	return NC_Micron_ReadRetrySequence(u8_RetryIndex, u8_SetToDefault);
}


U32 NC_Micron_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8   u8_ValueIdx;
	U8 	au8_RR_Value[4]={0,0,0,0};
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address_dram();
	pNandDrv = pNandDrv;	//fix warning	

	if(u8_SetToDefault)
	{
		u8_ValueIdx = 0;
	}
	else
	{
		u8_ValueIdx = u8_RetryIndex + 1;
	}
	au8_RR_Value[0] = u8_ValueIdx;

	// data go through CIFD
		
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END);
	
	NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0xEF);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (0x89<<8)|0x89);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ADR_C2T1S0 << 8) | CMD_REG8L);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | ACT_RAN_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);

	REG_WRITE_UINT16(NC_CIFD_ADDR(0), (au8_RR_Value[1]<<8) | au8_RR_Value[0]);
	REG_WRITE_UINT16(NC_CIFD_ADDR(1), (au8_RR_Value[3]<<8) | au8_RR_Value[2]);
	

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 4);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);	/*offset 0*/

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_CIFD_ACCESS|BIT_NC_JOB_START|BIT_NC_IF_DIR_W);

	if (NC_WaitComplete_dram(NC_MIE_EVENT,BIT_NC_JOB_END, DELAY_100ms_in_us) == DELAY_100ms_in_us)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error! timeout!\r\n");

		return UNFD_ST_ERR_SET_FEATURE_TIMEOUT;
	}
	
	//REG_WRITE_UINT16(NC_CF_SSO_CTL, 0xDEAD);

	NC_CLR_DDR_MODE();
	return UNFD_ST_SUCCESS;	// ok
}

