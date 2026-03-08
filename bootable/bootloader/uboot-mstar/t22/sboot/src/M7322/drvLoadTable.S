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
#include "drvLoadTbl.h"
#ifndef _BOOT_INC_
#include "boot.inc"
#endif

#ifndef _BOARD_H_
#include "Board.h"
#endif
@@ INPUT:
@//# *************************************************************
@//#  PAD_SAR1   Board   OnebinSet      config
@//#     L       150B       0           configs/M7322/.config.m7322.supernova.MST150B_10ANQHT_18151.rom_emmc_32bit.nonUMA
@//#     H       148B       1           configs/M7322/.config.m7322.supernova.MST148B_10NQAT_18115.rom_emmc_32bit.optee.mi
@//# *************************************************************
    .global Get_Packge_CFG
Get_Packge_CFG:
  push {lr}

   @//set PAD_SAR1 to GPIO Input mode
   RREG        r2, (MS_BUS_RIU_ADDR + (0x1422 << 1))
   bic         r2, r2, #(0x01 << 1)     @//reg[1422]#1 = 0b
   orr         r2, r2, #(0x01 << 9)     @//reg[1423]#1 = 1b
   REG_C2M     r2, (MS_BUS_RIU_ADDR + (0x1422 << 1))

   @//Read PAD_SAR1 status HIGH or LOW
   RREG        r2, (MS_BUS_RIU_ADDR + (0x1424 << 1))  @//reg[1425]#1
   and         r1, r2, #(0x01 << 9)
   lsr         r1, #9
  pop {pc}

@@ INPUT:
@r0: Load address (destination)
@r1: efuse (assume efuse number is the offset of board define )
@r2: type
@r3: offset for
    .global drv_LoadTbl
drv_LoadTbl:
  push {lr}

  cmp r2, #PADPRE
  beq PADPRE_HANDLE

  cmp r2, #PAD
  beq PAD_HANDLE

  cmp r2, #MIU0_Pre
  beq MIU0_PRE_HANDLE
  cmp r2, #MIU1_Pre
  beq MIU1_PRE_HANDLE

  cmp r2, #MIU0
  beq MIU0_HANDLE
  cmp r2, #MIU1
  beq MIU1_HANDLE

  cmp r2, #MIU0_STR_Pre
  beq MIU0_STR_Pre_HANDLE
  cmp r2, #MIU1_STR_Pre
  beq MIU1_STR_Pre_HANDLE

  cmp r2, #MIU0_STR
  beq MIU0_STR_HANDLE
  cmp r2, #MIU1_STR
  beq MIU1_STR_HANDLE


  @should not been here!!!
  @//DEAD_END:
  @//mov  r0, #0
  @//pop  {pc}
MIU0_STR_Pre_HANDLE :
  ldr TBL_SIZE, =MIU_PRE_STR_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU0_STR_PRE_SECT_OFF)
  b  MAIN_HANDLE
MIU1_STR_Pre_HANDLE :
  ldr TBL_SIZE, =MIU_PRE_STR_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU1_STR_PRE_SECT_OFF)
  b  MAIN_HANDLE

MIU0_PRE_HANDLE :
  ldr TBL_SIZE, =MIU_PRE_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU0_PRE_SECT_OFF)
  b  MAIN_HANDLE
MIU1_PRE_HANDLE :
  ldr TBL_SIZE, =MIU_PRE_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU1_PRE_SECT_OFF)
  b  MAIN_HANDLE


PADPRE_HANDLE:
    ldr TBL_SIZE, =PADPRE_TBL_SIZE
    ldr TYPE_OFF, =(OUTERSIZE+PADPRE_SEC_OFF)
    b  MAIN_HANDLE
PAD_HANDLE:
    ldr TBL_SIZE, =PAD_TBL_SIZE
    ldr TYPE_OFF, =(OUTERSIZE+PAD_SECT_OFF)
    b  MAIN_HANDLE

MIU0_HANDLE :
  ldr TBL_SIZE, =MIU_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU0_SECT_OFF)
  b  MAIN_HANDLE
MIU1_HANDLE :
  ldr TBL_SIZE, =MIU_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU1_SECT_OFF)
  b  MAIN_HANDLE

MIU0_STR_HANDLE :
  ldr TBL_SIZE, =MIU_STR_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU0_STR_SECT_OFF)
  b  MAIN_HANDLE
MIU1_STR_HANDLE :
  ldr TBL_SIZE, =MIU_STR_TBL_SIZE
  ldr TYPE_OFF, =(OUTERSIZE+MIU1_STR_SECT_OFF)

MAIN_HANDLE :
  mul TBL_OFF, TBL_SIZE, r1
  add TYPE_OFF, TYPE_OFF, TBL_OFF

  mov   r1, TBL_SIZE
  mov   r2, TYPE_OFF
  mov   r2, r2, lsr #9
  bl    eMMC_LoadImages
  mov  r0, #1
  pop {pc}
