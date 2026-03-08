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

#ifndef _IIC_H_
#define _IIC_H_

#ifdef _IIC_C_
#define _IICDEC_
#else
#define _IICDEC_ extern
#endif

#include "Board.h"

  // Speed Mapping: Base on the Driver loading, maybe you must to use the oscilloscope to measure this Value
  // 250K => SwIicDly =1
  // 200K => SwIicDly =10
  // 150K => SwIicDly =20
  // 100K => SwIicDly =40

#if (USE_SW_I2C_HIGHSPEED==1)
#if 1//base on CPU running at 172.8MHz
#define I2C_SPEED_MAPPING_400K           1
#define I2C_SPEED_MAPPING_350K           4
#define I2C_SPEED_MAPPING_300K           8
#define I2C_SPEED_MAPPING_250K          12
#define I2C_SPEED_MAPPING_200K          20
#define I2C_SPEED_MAPPING_150K          30
#define I2C_SPEED_MAPPING_100K          60
#else//base on CPU running at 216MHz
#define I2C_SPEED_MAPPING_400K          14
#define I2C_SPEED_MAPPING_350K          20
#define I2C_SPEED_MAPPING_300K          30
#define I2C_SPEED_MAPPING_250K          44
#define I2C_SPEED_MAPPING_200K          60
#define I2C_SPEED_MAPPING_150K         100
#define I2C_SPEED_MAPPING_100K         150
#endif
#else
//base on CPU running at 384MHz
#define I2C_SPEED_MAPPING_400K           50
#define I2C_SPEED_MAPPING_350K           60
#define I2C_SPEED_MAPPING_300K           70
#define I2C_SPEED_MAPPING_250K           80
#define I2C_SPEED_MAPPING_200K          100
#define I2C_SPEED_MAPPING_150K          135
#define I2C_SPEED_MAPPING_100K          135//200
#endif

typedef enum {
    E_I2C_BUS_DDCROM,
    E_I2C_BUS_SYS = E_I2C_BUS_DDCROM,
    E_I2C_BUS_DDCA0,
    E_I2C_BUS_DDCD0,
    E_I2C_BUS_DDCD1,
    E_I2C_BUS_DDCD2,
    E_I2C_BUS_USR0,
    E_I2C_BUS_USR1,
    E_I2C_BUS_USR2,
    E_I2C_BUS_USR3,
} E_I2C_BUS_CHANNEL;

_IICDEC_ void MDrv_IIC_Init(void);
_IICDEC_ BOOLEAN MDrv_IIC_WriteBytes(U16 u16BusNumSlaveID, U8 u8addrcount, U8* pu8addr, U16 u16size, U8* pu8data);
_IICDEC_ BOOLEAN MDrv_IIC_ReadBytes(U16 u16BusNumSlaveID, U8 u8AddrNum, U8* paddr, U16 u16size, U8* pu8data);
_IICDEC_ BOOLEAN MDrv_IIC_ReadByte(U16 u16BusNumSlaveID, U8 u8RegAddr, U8 *pu8Data);
_IICDEC_ BOOLEAN MDrv_IIC_WriteByte(U16 u16BusNumSlaveID, U8 u8RegAddr, U8 u8Data);

_IICDEC_ BOOLEAN MDrv_IIC_Write2Bytes(U16 u16BusNumSlaveID, U8 u8addr, U16 u16data);
_IICDEC_ U16 MDrv_IIC_Read2Bytes(U16 u16BusNumSlaveID, U8 u8addr);
_IICDEC_ BOOLEAN MDrv_IIC_Write4Bytes(U16 u16BusNumSlaveID, U32 u32Data, U8 u8EndData);

_IICDEC_ BOOLEAN MDrv_IIC_ReadByteDirectly(U16 u16BusNumSlaveID, U8 * pu8Data);

_IICDEC_ BOOLEAN MDrv_IIC_WriteGroupBytes(U16 u16BusNumSlaveID, U8 u8SubGroup, U16 u16Addr, U16 u16Data);
_IICDEC_ U16 MDrv_IIC_ReadGroupBytes(U16 u16BusNumSlaveID, U8 u8SubGroup, U16 u16Addr);

_IICDEC_ U32 MDrv_IIC_Speed_Setting(U8 u8BusNum, U32 u32Speed_K);

_IICDEC_ U8 IIC_GetByte (U16  bAck);
_IICDEC_ BOOLEAN IIC_SendByte(U8 u8dat);
_IICDEC_ BOOLEAN IIC_AccessStart(U8 ucSlaveAdr, U8 trans_t);
_IICDEC_ void IIC_Stop(void);

#undef _IICDEC_

#endif // _IIC_H_
