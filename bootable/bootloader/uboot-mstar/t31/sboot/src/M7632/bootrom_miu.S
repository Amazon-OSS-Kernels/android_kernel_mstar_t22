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


#ifndef _BOOT_INC_
#include "boot.inc"
#endif

#ifndef _BOARD_H_
#include "Board.h"
#endif

#ifndef _DRV_MIU_INIT_H_
#include "drv_miu_init.h"
#endif

#ifndef _MIUSEL_M7632_2MIU_H_
#include "MIUSEL_M7632_2MIU.h"
#endif
#ifdef CONFIG_ONEBIN_ENABLE
#include "drvLoadTbl.h"
#endif

#define DRAM_SEL        r7
#define DRAM_VENDOR     r8
#define MIU0_SIZE_REG   r9
#define MIU1_SIZE_REG   r10

    .text

MIU_MESSAGE_BIST0:      .short   'B', 'I', 'S', 'T', '0', '-', 0x00
MIU_MESSAGE_BIST1:      .short   'B', 'I', 'S', 'T', '1', '-', 0x00

MIU_MESSAGE_OK:         .short   'O', 'K', '\n', '\r', 0x00
MIU_MESSAGE_FAIL:       .short   'F', 'A', 'I', 'L', '\n', '\r', 0x00

MIU_MESSAGE_NOTSUPPORT: .short   'D', 'R', 'A', 'M', ' ', 'N', 'o', 't', ' ', 'S', 'u', 'p', 'p', 'o', 'r', 't', '\n', '\r', 0x00

    .align      4
    .global     BootRom_InitMiu
BootRom_InitMiu:

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         r8, lr
#else
    push        {lr}
#endif

#if defined(CONFIG_MSTAR_HAPS) && defined(CONFIG_MSTAR_TITANIA_BD_FPGA)
    //-------------------------------------------------------
    // MIU Channel Config
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x152b26 << 1)), 0x300a
    WREG        (MS_BUS_RIU_ADDR + (0x152b2a << 1)), 0x0931
    WREG        (MS_BUS_RIU_ADDR + (0x152c2a << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x1012d6 << 1)), 0x0020
    WREG        (MS_BUS_RIU_ADDR + (0x1006d6 << 1)), 0x0020
    WREG        (MS_BUS_RIU_ADDR + (0x1012dc << 1)), 0x000c
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x152b28 << 1)), 0x1000
    WREG        (MS_BUS_RIU_ADDR + (0x152c28 << 1)), 0x0001

    //-------------------------------------------------------
    // MIU DDRPLL Setting
    //-------------------------------------------------------
    // MIU reset & mask all
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c01
    WREG        (MS_BUS_RIU_ADDR + (0x1615e6 << 1)), 0xfffe

    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c01
    WREG        (MS_BUS_RIU_ADDR + (0x1622e6 << 1)), 0xfffe
    WREG        (MS_BUS_RIU_ADDR + (0x110d78 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x110d36 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d34 << 1)), 0x0102

    DELAYUS     1000 // delay 1ms

    //----------------------
    //set DDRPLL  =  2133 MHZ
    //----------------------
    WREG        (MS_BUS_RIU_ADDR + (0x110d30 << 1)), 0xec8f
    WREG        (MS_BUS_RIU_ADDR + (0x110d32 << 1)), 0x0019
    WREG        (MS_BUS_RIU_ADDR + (0x110d8a << 1)), 0x0001
    WREG        (MS_BUS_RIU_ADDR + (0x110d84 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d86 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161678 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x161636 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161634 << 1)), 0x0102
     DELAYUS     1000 // delay 1ms

    //----------------------
    //set DDRPLL  =  2133 MHZ
    //----------------------
    WREG        (MS_BUS_RIU_ADDR + (0x161630 << 1)), 0xec8f
    WREG        (MS_BUS_RIU_ADDR + (0x161632 << 1)), 0x0019
    WREG        (MS_BUS_RIU_ADDR + (0x16168a << 1)), 0x0001
    WREG        (MS_BUS_RIU_ADDR + (0x161684 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161686 << 1)), 0x0000

    //----------------------
    // Wait PLL Stable!!
    //----------------------
    DELAYUS     10000 // delay 10ms
    WREG        (MS_BUS_RIU_ADDR + (0x110d22 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d2c << 1)), 0x0114
    WREG        (MS_BUS_RIU_ADDR + (0x161622 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x16162c << 1)), 0x0014

    //-------------------------------------------------------
    // MIU Table Setting
    //-------------------------------------------------------
    // MIU0
    // set DDR3_16_4X_CL14_2133
    WREG        (MS_BUS_RIU_ADDR + (0x101202 << 1)), 0x0200
    WREG        (MS_BUS_RIU_ADDR + (0x101204 << 1)), 0x0053
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101228 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x152b20 << 1)), 0x3103
    WREG        (MS_BUS_RIU_ADDR + (0x152b22 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x152b40 << 1)), 0x0e0e
    WREG        (MS_BUS_RIU_ADDR + (0x152b42 << 1)), 0x0724
    WREG        (MS_BUS_RIU_ADDR + (0x152b44 << 1)), 0x3508
    WREG        (MS_BUS_RIU_ADDR + (0x152b46 << 1)), 0x100a
    WREG        (MS_BUS_RIU_ADDR + (0x152b48 << 1)), 0x0e08
    WREG        (MS_BUS_RIU_ADDR + (0x152b4a << 1)), 0x0604
    WREG        (MS_BUS_RIU_ADDR + (0x152b4c << 1)), 0x0528
    WREG        (MS_BUS_RIU_ADDR + (0x152b4e << 1)), 0x0116
    WREG        (MS_BUS_RIU_ADDR + (0x152b50 << 1)), 0xe000
    WREG        (MS_BUS_RIU_ADDR + (0x152b52 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x152b54 << 1)), 0x0e00
    WREG        (MS_BUS_RIU_ADDR + (0x152b5e << 1)), 0x304e
    WREG        (MS_BUS_RIU_ADDR + (0x152b60 << 1)), 0x1124
    WREG        (MS_BUS_RIU_ADDR + (0x152b62 << 1)), 0x4004
    WREG        (MS_BUS_RIU_ADDR + (0x152b64 << 1)), 0x8028
    WREG        (MS_BUS_RIU_ADDR + (0x152b66 << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x152b68 << 1)), 0x0017
    WREG        (MS_BUS_RIU_ADDR + (0x152b6a << 1)), 0x0017
    WREG        (MS_BUS_RIU_ADDR + (0x152b6c << 1)), 0x0017
    WREG        (MS_BUS_RIU_ADDR + (0x152b80 << 1)), 0x8010
    WREG        (MS_BUS_RIU_ADDR + (0x152b86 << 1)), 0x0010

    WREG        (MS_BUS_RIU_ADDR + (0x110d02 << 1)), 0xaaaa
    WREG        (MS_BUS_RIU_ADDR + (0x110d04 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x110d0e << 1)), 0x0097
    WREG        (MS_BUS_RIU_ADDR + (0x110d2e << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110db6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110ddc << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x110de0 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x110de2 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x110de4 << 1)), 0x0010
    WREG        (MS_BUS_RIU_ADDR + (0x110de6 << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110de8 << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110dea << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110dec << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110dee << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110df0 << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x110df2 << 1)), 0x0011

    WREG        (MS_BUS_RIU_ADDR + (0x151500 << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x151504 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151508 << 1)), 0x5757
    WREG        (MS_BUS_RIU_ADDR + (0x15150a << 1)), 0x5757
    WREG        (MS_BUS_RIU_ADDR + (0x151510 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151512 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151514 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151516 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15151e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151520 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151522 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151524 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151526 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151528 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15152a << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15152c << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15152e << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151530 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151532 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151534 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151536 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151538 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15153a << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15153c << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15153e << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151560 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151562 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151564 << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x151566 << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x151568 << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x15156a << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x151580 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151582 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151584 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151586 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151588 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15158a << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15158c << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15158e << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151590 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151592 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151594 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151596 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151598 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15159a << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15159c << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15159e << 1)), 0x0101

    //MIU1
    // set DDR3_16_4X_CL13_2133
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0x0200
    WREG        (MS_BUS_RIU_ADDR + (0x100604 << 1)), 0x0053
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100628 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x152c20 << 1)), 0x3003
    WREG        (MS_BUS_RIU_ADDR + (0x152c22 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x152c40 << 1)), 0x0e0e
    WREG        (MS_BUS_RIU_ADDR + (0x152c42 << 1)), 0x0724
    WREG        (MS_BUS_RIU_ADDR + (0x152c44 << 1)), 0x3508
    WREG        (MS_BUS_RIU_ADDR + (0x152c46 << 1)), 0x100a
    WREG        (MS_BUS_RIU_ADDR + (0x152c48 << 1)), 0x0e08
    WREG        (MS_BUS_RIU_ADDR + (0x152c4a << 1)), 0x0604
    WREG        (MS_BUS_RIU_ADDR + (0x152c4c << 1)), 0x0528
    WREG        (MS_BUS_RIU_ADDR + (0x152c4e << 1)), 0x0116
    WREG        (MS_BUS_RIU_ADDR + (0x152c50 << 1)), 0xe000
    WREG        (MS_BUS_RIU_ADDR + (0x152c52 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x152c54 << 1)), 0x0e00
    WREG        (MS_BUS_RIU_ADDR + (0x152c5e << 1)), 0x304e
    WREG        (MS_BUS_RIU_ADDR + (0x152c60 << 1)), 0x1124
    WREG        (MS_BUS_RIU_ADDR + (0x152c62 << 1)), 0x4004
    WREG        (MS_BUS_RIU_ADDR + (0x152c64 << 1)), 0x8028
    WREG        (MS_BUS_RIU_ADDR + (0x152c66 << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x152c68 << 1)), 0x0017
    WREG        (MS_BUS_RIU_ADDR + (0x152c6a << 1)), 0x0017
    WREG        (MS_BUS_RIU_ADDR + (0x152c6c << 1)), 0x0017
    WREG        (MS_BUS_RIU_ADDR + (0x152c80 << 1)), 0x8010
    WREG        (MS_BUS_RIU_ADDR + (0x152c86 << 1)), 0x0010
    WREG        (MS_BUS_RIU_ADDR + (0x161602 << 1)), 0xaaaa
    WREG        (MS_BUS_RIU_ADDR + (0x161604 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x16160e << 1)), 0x0097
    WREG        (MS_BUS_RIU_ADDR + (0x16162e << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616b6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x1616dc << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x1616e0 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x1616e2 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x1616e4 << 1)), 0x0010
    WREG        (MS_BUS_RIU_ADDR + (0x1616e6 << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616e8 << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616ea << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616ec << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616ee << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616f0 << 1)), 0x1111
    WREG        (MS_BUS_RIU_ADDR + (0x1616f2 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x151600 << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x151604 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151608 << 1)), 0x5757
    WREG        (MS_BUS_RIU_ADDR + (0x15160a << 1)), 0x5757
    WREG        (MS_BUS_RIU_ADDR + (0x151610 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151612 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151614 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151616 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15161e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x151620 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151622 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151624 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151626 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151628 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15162a << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15162c << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15162e << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151630 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151632 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151634 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151636 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151638 << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15163a << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15163c << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x15163e << 1)), 0x0202
    WREG        (MS_BUS_RIU_ADDR + (0x151660 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151662 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151664 << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x151666 << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x151668 << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x15166a << 1)), 0x0808
    WREG        (MS_BUS_RIU_ADDR + (0x151680 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151682 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151684 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151686 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151688 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15168a << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15168c << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15168e << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151690 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151692 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151694 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151696 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x151698 << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15169a << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15169c << 1)), 0x0101
    WREG        (MS_BUS_RIU_ADDR + (0x15169e << 1)), 0x0101

    b TEST
    .ltorg

TEST:
    //-------------------------------------------------------
    // MIU pre-inivec Setting MIU0
    //-------------------------------------------------------
    // program DLL
    WREG        (MS_BUS_RIU_ADDR + (0x110d62 << 1)), 0x007f
    WREG        (MS_BUS_RIU_ADDR + (0x110d64 << 1)), 0xf000
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cf
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c3
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c3
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c2
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x110d60 << 1)), 0x33c8
    WREG        (MS_BUS_RIU_ADDR + (0x110d70 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d98 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d9a << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d90 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d70 << 1)), 0x0800

    // program ana general setting
    WREG        (MS_BUS_RIU_ADDR + (0x110d70 << 1)), 0x0800
    //Driving setting
    //DRVN
    WREG        (MS_BUS_RIU_ADDR + (0x1515a0 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515a2 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515a4 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515a6 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515a8 << 1)), 0x1616

    //DRVP
    WREG        (MS_BUS_RIU_ADDR + (0x1515aa << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515ac << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515ae << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515b0 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1515b2 << 1)), 0x1616
    //trig. lvl setting
    WREG        (MS_BUS_RIU_ADDR + (0x110d6c << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x110d6e << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x110d74 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x110d76 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x110d1a << 1)), 0x83aa
    WREG        (MS_BUS_RIU_ADDR + (0x110d1c << 1)), 0x0020
    WREG        (MS_BUS_RIU_ADDR + (0x110d08 << 1)), 0x003f

    //---------------------------
    //DQSM RST
    //---------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005

    //-------------------------------------------------------
    // MIU pre-inivec Setting MIU1
    //-------------------------------------------------------
    // program DLL
    WREG        (MS_BUS_RIU_ADDR + (0x161662 << 1)), 0x007f
    WREG        (MS_BUS_RIU_ADDR + (0x161664 << 1)), 0xf000
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cf
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c3
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c3
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00cb
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c2
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x161660 << 1)), 0x33c8
    WREG        (MS_BUS_RIU_ADDR + (0x161670 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161698 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x16169a << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161690 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161670 << 1)), 0x0800

    // program ana general setting
    WREG        (MS_BUS_RIU_ADDR + (0x161670 << 1)), 0x0800
    //Driving setting
    //DRVN
    WREG        (MS_BUS_RIU_ADDR + (0x1516a0 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516a2 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516a4 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516a6 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516a8 << 1)), 0x1616

    //DRVP
    WREG        (MS_BUS_RIU_ADDR + (0x1516aa << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516ac << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516ae << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516b0 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x1516b2 << 1)), 0x1616
    //trig. lvl setting
    WREG        (MS_BUS_RIU_ADDR + (0x16166c << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x16166e << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x161674 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x161676 << 1)), 0x1616
    WREG        (MS_BUS_RIU_ADDR + (0x16161a << 1)), 0x83aa
    WREG        (MS_BUS_RIU_ADDR + (0x16161c << 1)), 0x0020
    WREG        (MS_BUS_RIU_ADDR + (0x161608 << 1)), 0x003f

    //---------------------------
    //DQSM RST
    //---------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005

    //-------------------------------------------------------
    // miu_pre_inivec_dig setting start MIU0
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x1615e4 << 1)), 0x000e
    //program dig general setting
    WREG        (MS_BUS_RIU_ADDR + (0x152bdc << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x152b88 << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x152b8a << 1)), 0xff12
    WREG        (MS_BUS_RIU_ADDR + (0x152b80 << 1)), 0x80f0
    WREG        (MS_BUS_RIU_ADDR + (0x101236 << 1)), 0xc803
    WREG        (MS_BUS_RIU_ADDR + (0x1012fc << 1)), 0x951a

    //-------------------------------------------------------
    // miu_pre_inivec_dig setting start MIU1
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x1622e4 << 1)), 0x000e
    //program dig general setting
    WREG        (MS_BUS_RIU_ADDR + (0x152cdc << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x152c88 << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x152c8a << 1)), 0xff12
    WREG        (MS_BUS_RIU_ADDR + (0x152c80 << 1)), 0x80f0
    WREG        (MS_BUS_RIU_ADDR + (0x100636 << 1)), 0xc803
    WREG        (MS_BUS_RIU_ADDR + (0x1006fc << 1)), 0x951a

    // MIU SW reset
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x8c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x8c00

    //--------------------------------------
    // MIU post-inivec Setting MIU0
    //--------------------------------------
    //disable GPIO
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x2010
    WREG        (MS_BUS_RIU_ADDR + (0x110d00 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d18 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d06 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d7c << 1)), 0x0005

    //Release ATOP PD mode
    WREG        (MS_BUS_RIU_ADDR + (0x110d54 << 1)), 0xc000

    // DQSM RST
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005

    //select Mapping
    WREG        (MS_BUS_RIU_ADDR + (0x110d3e << 1)), 0x0006

    //--------------------------------------
    // MIU post-inivec Setting MIU1
    //--------------------------------------
    //disable GPIO
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x2010
    WREG        (MS_BUS_RIU_ADDR + (0x161600 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161618 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x16167c << 1)), 0x0005

    //Release ATOP PD mode
    WREG        (MS_BUS_RIU_ADDR + (0x161654 << 1)), 0xc000

    // DQSM RST
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005

    //select Mapping
    WREG        (MS_BUS_RIU_ADDR + (0x16163e << 1)), 0x0006

    // miu_post_inivec_dig setting start
    //WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0x0a00

    //--------------------------------------
    // MIU DRAM init
    //--------------------------------------
    // begin init DRAM
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0000
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000c
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000e
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x001f
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0000
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000c
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000e
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x001f
    DELAYUS     1000

    //WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x0000
    //WREG        (MS_BUS_RIU_ADDR + (0x152c36 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d14 << 1)), 0x0e00
    WREG        (MS_BUS_RIU_ADDR + (0x161614 << 1)), 0x0e00

    DELAYUS     10000

    //--------------------------------------
    // Secure setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x152bc8 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x152cc8 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x113bec << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x152bc8 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x152cc8 << 1)), 0x0011

    //--------------------------------------
    // Arbitration setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x310212 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310312 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310412 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310512 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310612 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310712 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310812 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310912 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310a12 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310230 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310200 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310330 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310300 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310430 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310400 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310530 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310500 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310630 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310600 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310730 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310700 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310830 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310844 << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x310800 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310930 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310900 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310a30 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310a00 << 1)), 0x0303

    //--------------------------------------
    //CTRL Arbiter setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x161504 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161506 << 1)), 0x7ffe
    WREG        (MS_BUS_RIU_ADDR + (0x16150a << 1)), 0xffff

_CheckDramData:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r1]
    and         r0, r0, #0x8000
    cmp         r0, #0
    beq         _CheckDramData

    //unmask client
    WREG        (MS_BUS_RIU_ADDR + (0x310206 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310306 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310406 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310506 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310706 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310806 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310906 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310a06 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af0 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af2 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af4 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af6 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x31023e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31033e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31043e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31053e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31063e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31073e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31083e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31093e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310a3e << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x1615e6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x1622e6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x1012fe << 1)), 0xa000

    //BIST test
    bl          Miu0_BistTest

    //sw init done release
    WREG        (MS_BUS_RIU_ADDR + (0x1012e0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c08

    //set DRAM size for secure module
    WREG        (MS_BUS_RIU_ADDR + (0x1012d2 << 1)), 0xB000

    //PM 51 can access nonPM RIU
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x123900 << 1))
    ldr         r0, [r1, #(0x60 << 1)]
    orr         r0, #(0x01 << 2)
    str         r0, [r1, #(0x60 << 1)]

#elif defined(CONFIG_MSTAR_TITANIA_BD_FPGA)

    //-------------------------------------------------------
    // MIU Channel Config
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x152b26 << 1)), 0x0d06
    WREG        (MS_BUS_RIU_ADDR + (0x1012dc << 1)), 0x000c
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x152b28 << 1)), 0x1400
    WREG        (MS_BUS_RIU_ADDR + (0x152c28 << 1)), 0x0400

    //-------------------------------------------------------
    // MIU DDRPLL Setting
    //-------------------------------------------------------
    // MIU reset & mask all
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c01
    WREG        (MS_BUS_RIU_ADDR + (0x1615e6 << 1)), 0xfffe

    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c01
    WREG        (MS_BUS_RIU_ADDR + (0x1622e6 << 1)), 0xfffe

    //----------------------
    // Wait PLL Stable!!
    //----------------------
    DELAYUS     10000 // delay 10ms

    //-------------------------------------------------------
    // MIU Table Setting
    //-------------------------------------------------------
    // MIU0
    WREG        (MS_BUS_RIU_ADDR + (0x101202 << 1)), 0x0800
    WREG        (MS_BUS_RIU_ADDR + (0x101204 << 1)), 0x01c5
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101228 << 1)), 0x0100
    WREG        (MS_BUS_RIU_ADDR + (0x152b20 << 1)), 0x1200
    WREG        (MS_BUS_RIU_ADDR + (0x152b22 << 1)), 0x1101
    WREG        (MS_BUS_RIU_ADDR + (0x152b40 << 1)), 0x0f0f

    WREG        (MS_BUS_RIU_ADDR + (0x152b42 << 1)), 0x081f
    WREG        (MS_BUS_RIU_ADDR + (0x152b44 << 1)), 0x2f08
    WREG        (MS_BUS_RIU_ADDR + (0x152b46 << 1)), 0x0f01
    WREG        (MS_BUS_RIU_ADDR + (0x152b48 << 1)), 0x070f
    WREG        (MS_BUS_RIU_ADDR + (0x152b4a << 1)), 0x0102
    WREG        (MS_BUS_RIU_ADDR + (0x152b4e << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152b50 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152b54 << 1)), 0x0f09
    WREG        (MS_BUS_RIU_ADDR + (0x152b5e << 1)), 0x2400
    WREG        (MS_BUS_RIU_ADDR + (0x152b60 << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152b80 << 1)), 0x8010
    WREG        (MS_BUS_RIU_ADDR + (0x152b86 << 1)), 0x0000

    //MIU1
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0x0800
    WREG        (MS_BUS_RIU_ADDR + (0x100604 << 1)), 0x01c5
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100628 << 1)), 0x0100
    WREG        (MS_BUS_RIU_ADDR + (0x152c20 << 1)), 0x1200
    WREG        (MS_BUS_RIU_ADDR + (0x152c22 << 1)), 0x1101
    WREG        (MS_BUS_RIU_ADDR + (0x152c40 << 1)), 0x0f0f
    WREG        (MS_BUS_RIU_ADDR + (0x152c42 << 1)), 0x081f
    WREG        (MS_BUS_RIU_ADDR + (0x152c44 << 1)), 0x2f08
    WREG        (MS_BUS_RIU_ADDR + (0x152c46 << 1)), 0x0f01
    WREG        (MS_BUS_RIU_ADDR + (0x152c48 << 1)), 0x070f
    WREG        (MS_BUS_RIU_ADDR + (0x152c4a << 1)), 0x0102
    WREG        (MS_BUS_RIU_ADDR + (0x152c4e << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152c50 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152c54 << 1)), 0x0f09
    WREG        (MS_BUS_RIU_ADDR + (0x152c5e << 1)), 0x2400
    WREG        (MS_BUS_RIU_ADDR + (0x152c60 << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152c80 << 1)), 0x8010
    WREG        (MS_BUS_RIU_ADDR + (0x152c86 << 1)), 0x0000

    //-------------------------------------------------------
    // miu_pre_inivec_dig setting start MIU0
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x1615e4 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x1622e4 << 1)), 0x000e

    //program dig general setting
    WREG        (MS_BUS_RIU_ADDR + (0x152b2a << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152bdc << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x161580 << 1)), 0x0090
    WREG        (MS_BUS_RIU_ADDR + (0x152b8a << 1)), 0xff10
    WREG        (MS_BUS_RIU_ADDR + (0x152b80 << 1)), 0x80f0
    WREG        (MS_BUS_RIU_ADDR + (0x152bde << 1)), 0x8002
    WREG        (MS_BUS_RIU_ADDR + (0x1012fc << 1)), 0x951a

    //-------------------------------------------------------
    // miu_pre_inivec_dig setting start MIU1
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x1615e4 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x1622e4 << 1)), 0x000e

    //program dig general setting
    WREG        (MS_BUS_RIU_ADDR + (0x152c2a << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152cdc << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x162280 << 1)), 0x0090
    WREG        (MS_BUS_RIU_ADDR + (0x152c8a << 1)), 0xff90
    WREG        (MS_BUS_RIU_ADDR + (0x152c80 << 1)), 0x80f0
    WREG        (MS_BUS_RIU_ADDR + (0x152cde << 1)), 0x8002
    WREG        (MS_BUS_RIU_ADDR + (0x1006fc << 1)), 0x951a

    // MIU SW reset
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x8c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x8c00

    //--------------------------------------
    // MIU DRAM init
    //--------------------------------------
    // begin init DRAM
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0000
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000c
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000e
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x001f
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0000
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000c
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000e
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x001f
    DELAYUS     1000

    DELAYUS     10000

    //--------------------------------------
    // Arbitration setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x310212 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310312 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310412 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310512 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310612 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310712 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310812 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310912 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310a12 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310230 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310200 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310330 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310300 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310430 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310400 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310530 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310500 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310630 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310600 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310730 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310700 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310830 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310800 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310930 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310900 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310a30 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310a00 << 1)), 0x0303

    //--------------------------------------
    //CTRL Arbiter setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x161534 << 1)), 0x03fe
    WREG        (MS_BUS_RIU_ADDR + (0x161504 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161506 << 1)), 0x7ffe
    WREG        (MS_BUS_RIU_ADDR + (0x16150a << 1)), 0xffff

_CheckDramData:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r1]
    and         r0, r0, #0x8000
    cmp         r0, #0
    beq         _CheckDramData

    //unmask client
    WREG        (MS_BUS_RIU_ADDR + (0x310206 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310306 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310406 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310506 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310706 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310806 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310906 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310a06 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af0 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af2 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af4 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af6 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x31023e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31033e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31043e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31053e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31063e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31073e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31083e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31093e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310a3e << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x1615e6 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x1622e6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x1012fe << 1)), 0xa000

    bl          Miu0_BistTest

    //sw init done release
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x8c08
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x8c08
#else
    bl          Miu_Check_DramSupport
    bl          Miu_Set_Dram_Sel

    DPUTK_STR   MiuBoardType
    DPUTK_STR   MiuDate
#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r0, =MIU0_Ver
    bl          MDrv_WriteRegTbl32
#endif
#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r0, =MIU1_Ver
    bl          MDrv_WriteRegTbl32
#endif

    bl          Miu_Get_Dram_Sel
    bl          Miu_Set_Version

    //Disable the function that PM51 is blocked from accessing DIE domain riu & miu
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x3900 << 1))
    ldr         r0, [r1, #(0x64 << 1)]
    orr         r0, #0x01
    str         r0, [r1, #(0x64 << 1)]

    //PM 51 can access nonPM RIU
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x123900 << 1))
    ldr         r0, [r1, #(0x60 << 1)]
    orr         r0, #(0x01 << 2)
    str         r0, [r1, #(0x60 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #(0xa0 << 1)]

#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xc8 << 1)]
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xc8 << 1)]
#endif

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldrb        r0, [r1, #((0x30 << 1) + 1)]
    bic         r0, r0, #0x80
    strb        r0, [r1, #((0x30 << 1) + 1)]

    // Disable ZQCS
    ldr         r0, =0x0000
    str         r0, [r1, #(0xd0 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldrb        r0, [r1, #((0x30 << 1) + 1)]
    bic         r0, r0, #0x80
    strb        r0, [r1, #((0x30 << 1) + 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x000a
    str         r0, [r1, #(0xdc << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x000a
    str         r0, [r1, #(0xdc << 1)]

    bl          MIU_Arbiter_Reset_Block

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r2, [r1, #(0xc0 << 1)]
    lsr         r2, r2, #0x08
#if defined(CONFIG_MIU_WW_PACKAGE)
#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU1_MAX_SUPPORT_NUM >=3)
    cmp         r2, #MIU0_3RD_BONDING
    beq         MIU_WW_48BIT
#endif
#if (MIU1_MAX_SUPPORT_NUM >=4)
    cmp         r2, #MIU0_4TH_BONDING
    beq         MIU_WW_2G
#endif
#if (MIU1_MAX_SUPPORT_NUM >=5)
    cmp         r2, #MIU0_5TH_BONDING
    beq         MIU_WW_2G
#endif
#if (MIU1_MAX_SUPPORT_NUM >=6)
    cmp         r2, #MIU0_6TH_BONDING
    beq         MIU_WW_1_75G
#endif
#if (MIU1_MAX_SUPPORT_NUM >=7)
    cmp         r2, #MIU0_7TH_BONDING
    beq         MIU_WW_1_75G
#endif
#if (MIU1_MAX_SUPPORT_NUM >=8)
    cmp         r2, #MIU0_8TH_BONDING
    beq         MIU_WW_2G
#endif
#endif

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x4002
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0231
    str         r0, [r1, #(0x2a << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0231
    str         r0, [r1, #(0x2a << 1)]
#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU1_MAX_SUPPORT_NUM >=3)
    B           MIU_PREINIT_END

MIU_WW_48BIT:
    PUTK        'A'
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0006
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
#endif
#if (MIU1_MAX_SUPPORT_NUM >=4)
    B           MIU_PREINIT_END

MIU_WW_2G:
    PUTK        'B'
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0002
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
#endif
#if (MIU1_MAX_SUPPORT_NUM >=6)
    B           MIU_PREINIT_END

MIU_WW_1_75G:
    PUTK        'C'
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x3016
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0531
    str         r0, [r1, #(0x2a << 1)]
#endif
#endif
#else
#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU1_MAX_SUPPORT_NUM >=3)
    cmp         r2, #MIU0_3RD_BONDING
    beq         MIU_1_25G_PREINIT
#endif
#endif

    PUTK        'A'
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0006
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU1_MAX_SUPPORT_NUM >=3)
    b           MIU_PREINIT_END

MIU_1_25G_PREINIT:
    PUTK        'B'
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x300a
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0131
    str         r0, [r1, #(0x2a << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
#endif
#endif
#endif

MIU_PREINIT_END:

    ldr         r0, =MIU_PreInit
    bl          MDrv_WriteRegTbl32

    DELAYUS     4000

#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))
#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU0_Pre
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU0_DDR_PreInit
#endif
    bl          MDrv_WriteRegTbl32

    DELAYUS     1000

#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161600 << 1))

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU1_Pre
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU1_DDR_PreInit
#endif
    bl          MDrv_WriteRegTbl32

    DELAYUS     1000

#endif

#if !defined(CONFIG_MIU0_DRAM_NONE)

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU0
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU0_DDR_Init
#endif
    bl          MDrv_WriteRegTbl32

#endif /* CONFIG_MIU0_DRAM_NONE */

#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU0_MAX_SUPPORT_NUM >= 2)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r2, [r1, #(0xc0 << 1)]
    lsr         r2, r2, #0x08
    cmp         r2, #MIU0_2ND_BONDING
    beq         MIU0_2ND
#if (MIU0_MAX_SUPPORT_NUM >=3)
    cmp         r2, #MIU0_3RD_BONDING
    beq         MIU0_3RD
#endif
#if (MIU0_MAX_SUPPORT_NUM >=4)
    cmp         r2, #MIU0_4TH_BONDING
    beq         MIU0_4TH
#endif
#if (MIU0_MAX_SUPPORT_NUM >=5)
    cmp         r2, #MIU0_5TH_BONDING
    beq         MIU0_5TH
#endif
#if (MIU0_MAX_SUPPORT_NUM >=6)
    cmp         r2, #MIU0_6TH_BONDING
    beq         MIU0_6TH
#endif
#if (MIU0_MAX_SUPPORT_NUM >=7)
    cmp         r2, #MIU0_7TH_BONDING
    beq         MIU0_7TH
#endif
#if (MIU0_MAX_SUPPORT_NUM >=8)
    cmp         r2, #MIU0_8TH_BONDING
    beq         MIU0_8TH
#endif

    b           MIU0_INIT_END

MIU0_2ND:
    PUTK        '2'
    ldr         r0, =MIU0_DDR_Init_2nd
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
#if (MIU0_MAX_SUPPORT_NUM >=3)
MIU0_3RD:
    PUTK        '3'
    ldr         r0, =MIU0_DDR_Init_3rd
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
#if (MIU0_MAX_SUPPORT_NUM >=4)
MIU0_4TH:
    PUTK        '4'
    ldr         r0, =MIU0_DDR_Init_4th
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
#if (MIU0_MAX_SUPPORT_NUM >=5)
MIU0_5TH:
    PUTK        '5'
    ldr         r0, =MIU0_DDR_Init_5th
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
#if (MIU0_MAX_SUPPORT_NUM >=6)
MIU0_6TH:
    PUTK        '6'
    ldr         r0, =MIU0_DDR_Init_6th
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
#if (MIU0_MAX_SUPPORT_NUM >=7)
MIU0_7TH:
    PUTK        '7'
    ldr         r0, =MIU0_DDR_Init_7th
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
#if (MIU0_MAX_SUPPORT_NUM >=8)
MIU0_8TH:
    PUTK        '8'
    ldr         r0, =MIU0_DDR_Init_8th
    bl          MDrv_WriteRegTbl32
    b           MIU0_INIT_END
#endif
MIU0_INIT_END:
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU1
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU1_DDR_Init
#endif
    bl          MDrv_WriteRegTbl32

#if defined(MIU1_MAX_SUPPORT_NUM)
#if (MIU1_MAX_SUPPORT_NUM >= 2)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r2, [r1, #(0xc0 << 1)]
    lsr         r2, r2, #0x08
    cmp         r2, #MIU1_2ND_BONDING
    beq         MIU1_2ND
#if (MIU1_MAX_SUPPORT_NUM >=3)
    cmp         r2, #MIU1_3RD_BONDING
    beq         MIU1_3RD
#endif
#if (MIU1_MAX_SUPPORT_NUM >=4)
    cmp         r2, #MIU1_4TH_BONDING
    beq         MIU1_4TH
#endif
#if (MIU1_MAX_SUPPORT_NUM >=5)
    cmp         r2, #MIU1_5TH_BONDING
    beq         MIU1_5TH
#endif
#if (MIU1_MAX_SUPPORT_NUM >=6)
    cmp         r2, #MIU1_6TH_BONDING
    beq         MIU1_6TH
#endif
#if (MIU1_MAX_SUPPORT_NUM >=7)
    cmp         r2, #MIU1_7TH_BONDING
    beq         MIU1_7TH
#endif
#if (MIU1_MAX_SUPPORT_NUM >=8)
    cmp         r2, #MIU1_8TH_BONDING
    beq         MIU1_8TH
#endif

    b           MIU1_INIT_END

MIU1_2ND:
    PUTK        '2'
    ldr         r0, =MIU1_DDR_Init_2nd
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
#if (MIU1_MAX_SUPPORT_NUM >=3)
MIU1_3RD:
    PUTK        '3'
    ldr         r0, =MIU1_DDR_Init_3rd
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
#if (MIU1_MAX_SUPPORT_NUM >=4)
MIU1_4TH:
    PUTK        '4'
    ldr         r0, =MIU1_DDR_Init_4th
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
#if (MIU1_MAX_SUPPORT_NUM >=5)
MIU1_5TH:
    PUTK        '5'
    ldr         r0, =MIU1_DDR_Init_5th
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
#if (MIU1_MAX_SUPPORT_NUM >=6)
MIU1_6TH:
    PUTK        '6'
    ldr         r0, =MIU1_DDR_Init_6th
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
#if (MIU1_MAX_SUPPORT_NUM >=7)
MIU1_7TH:
    PUTK        '7'
    ldr         r0, =MIU1_DDR_Init_7th
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
#if (MIU1_MAX_SUPPORT_NUM >=8)
MIU1_8TH:
    PUTK        '8'
    ldr         r0, =MIU1_DDR_Init_8th
    bl          MDrv_WriteRegTbl32
    b           MIU1_INIT_END
#endif
MIU1_INIT_END:
    PUTK        '\n'
    PUTK        '\r'
#endif

#endif /* CONFIG_MIU1_DRAM_NONE */

    bl          MIU_Secure_setting
    bl          MIU_Arbiter_Setting
    bl          Release_Sw_Reset
    bl          MIU_Post_inivec

    DELAYUS     200                         //delay 200us

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //MIU1 INIT Cycle
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r0, =0x0000                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     200                         //delay 200us

    ldr         r0, =0x0008                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     1                           //delay 1us

    ldr         r0, =0x000c                 //DRAM reset+CS
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     500                         //delay 500us

    ldr         r0, =0x000e                 //DRAM reset+CS+CKE
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     1                           //delay 1us

    ldr         r0, =0x001f                 //DRAM reset+CS+CKE+auto cycle+ODE
    str         r0, [r1, #(0x00 << 1)]

#endif /* CONFIG_MIU1_DRAM_NONE */


#if !defined(CONFIG_MIU0_DRAM_NONE)
    //MIU0 INIT Cycle
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0x0000                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     200                         //delay 200us

    ldr         r0, =0x0008                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     1                           //delay 1us

    ldr         r0, =0x000c                 //DRAM reset+CS
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     500                         //delay 500us

    ldr         r0, =0x000e                 //DRAM reset+CS+CKE
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     1                           //delay 1us

    ldr         r0, =0x001f                 //DRAM reset+CS+CKE+auto cycle+ODE
    str         r0, [r1, #(0x00 << 1)]
#endif /* CONFIG_MIU0_DRAM_NONE */

#if !defined(CONFIG_MIU0_DRAM_NONE)    
CheckMiu0Init:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r1, #(0x00 << 1)]
    and         r0, r0, #0x8000
    cmp         r0, #0
    beq         CheckMiu0Init

#endif

    bl          Miu_Improve_Efficiency

#if defined(CONFIG_ENABLE_MIU0_AUTO_DQS)
#if !defined(CONFIG_MIU0_DRAM_NONE)

    //Scan MIU0 DQS
    ldr         r0, =0
    bl          BootRom_MiuDdrDqs

#endif //CONFIG_MIU0_DRAM_NONE
#endif

    DELAYUS     800                                         //delay 800us

#if !defined(CONFIG_MIU0_DRAM_NONE)
#if defined(CONFIG_ENABLE_MIU_SSC)
    ldr         r0, =MIU0_EnableSSC
    bl          MDrv_WriteRegTbl32
#endif
#endif /* CONFIG_MIU0_DRAM_NONE */

#if !defined(CONFIG_MIU1_DRAM_NONE)
#if defined(CONFIG_ENABLE_MIU_SSC)
    ldr         r0, =MIU1_EnableSSC
    bl          MDrv_WriteRegTbl32
#endif
#endif /* CONFIG_MIU1_DRAM_NONE */

    DELAYUS     1000

    bl          MIU_Unmask_Bist

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //BIST test
    bl          Miu0_BistTest
#endif

    //Disable BIST
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r12, #(0xfe << 1)]
    orr         r0, r0, #(0x01 << 11)
    str         r0, [r12, #(0xfe << 1)]

    ldr         r0, =MIU_PostInit
    bl          MDrv_WriteRegTbl32
#endif

    // Set DRAM size for OPTEE tsp used
#if !defined(CONFIG_MIU0_DRAM_NONE)    
    WREG        (MS_BUS_RIU_ADDR + (0x1012d2 << 1)), 0xB000
#endif

#ifdef CONFIG_MSTAR_MOBF_ENABLE
    bl          BootRom_InitMobf_Miu
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         lr, r8
#else
    pop         {lr}
#endif
    bx          lr
    .ltorg

#if !defined(CONFIG_MSTAR_HAPS)
//-------------------------------------------------------------------------------------------------
// MIU_Arbiter_Reset_Block
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Arbiter_Reset_Block:
    //BLOCK ARBITER RESET & CLK_ENABLE
    WREG        (MS_BUS_RIU_ADDR + (0x3102ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3103ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3104ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3105ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3106ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3107ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3108ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3109ae << 1)), 0xd000

    WREG        (MS_BUS_RIU_ADDR + (0x10129c << 1)), 0x0001

    WREG        (MS_BUS_RIU_ADDR + (0x10124c << 1)), 0x01ff
    WREG        (MS_BUS_RIU_ADDR + (0x10124e << 1)), 0x01ff
    WREG        (MS_BUS_RIU_ADDR + (0x101250 << 1)), 0x01ff

    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Arbiter_Setting
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Arbiter_Setting:

    //BLOCK ARBITER Release Reset
    WREG        (MS_BUS_RIU_ADDR + (0x10124e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101250 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x10124c << 1)), 0x0000

#if defined(CONFIG_BLOCK_MCM_ENABLE)
    WREG        (MS_BUS_RIU_ADDR + (0x3102ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3103ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3104ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3105ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3106ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3107ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3108ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3109ae << 1)), 0xc000
#endif

    //Block Arbiter setting
    WREG        (MS_BUS_RIU_ADDR + (0x310212 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310312 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310412 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310512 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310612 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310712 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310812 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310912 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x311012 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310230 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310200 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310330 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310300 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310430 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310400 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310530 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310500 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310630 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310600 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310730 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310700 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310830 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310844 << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x310800 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310930 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310900 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x311030 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x311000 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310294 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310394 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310494 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310594 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310694 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310794 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310894 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x310994 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x311094 << 1)), 0x0030
    WREG        (MS_BUS_RIU_ADDR + (0x3105de << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x3106de << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x3107de << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x3108de << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x3109de << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x1615d0 << 1)), 0x01f0
    WREG        (MS_BUS_RIU_ADDR + (0x310844 << 1)), 0x0003

    //CTRL Arbiter setting
    WREG        (MS_BUS_RIU_ADDR + (0x161504 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161506 << 1)), 0x7ffe
    WREG        (MS_BUS_RIU_ADDR + (0x16150a << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310a0c << 1)), 0x0000

    bx          lr

//-------------------------------------------------------------------------------------------------
// Release_Sw_Reset
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
Release_Sw_Reset:

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //MIU1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r0, =0x8c00
    str         r0, [r1, #(0x1e << 1)]
#endif

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //MIU0
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0x8c00
    str         r0, [r1, #(0x1e << 1)]
#endif

    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Secure_setting
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Secure_setting:

#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xc8 << 1)]
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xc8 << 1)]
#endif

#if defined(CONFIG_MIU0_DRAM_NONE) || defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113b00 << 1))
    ldr         r0, =0xc000
    str         r0, [r1, #(0xec << 1)]
#endif

    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Post_inivec
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Post_inivec:

#if !defined(CONFIG_MIU0_DRAM_NONE)

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))
    ldr         r0, [r1, #(0x00 << 1)]
    ldr         r2, =0xFFF8             // BIT[2:0] (mapping setting need keep original value)
    bic         r0, r0, r2
    ldr         r2, =0x2010
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]
    ldr         r2, =0x0000
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]

    WREG        (MS_BUS_RIU_ADDR + (0x110d18 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d06 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d7c << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x110d54 << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005

#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161600 << 1))
    ldr         r0, [r1, #(0x00 << 1)]
    ldr         r2, =0xFFF8             // BIT[2:0] (mapping setting need keep original value)
    bic         r0, r0, r2
    ldr         r2, =0x2010
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]
    ldr         r2, =0x0000
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]

    WREG        (MS_BUS_RIU_ADDR + (0x161618 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x16167c << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x161654 << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005

#endif
    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Unmask_Bist
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Unmask_Bist:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldr         r0, =0x7ffe
    str         r0, [r1, #(0xe6 << 1)]

#if !defined(CONFIG_MIU_48BIT)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, [r1, #(0xc0 << 1)]
    lsr         r0, r0, #8
    and         r0, r0, #0x28
    cmp         r0, #0
    bne         MIU_Unmask_Bist_End

    // For 48bit settings
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161600 << 1))
    ldr         r0, =0xc020
    str         r0, [r1, #(0x54 << 1)]
    ldr         r0, =0x003d
    str         r0, [r1, #(0x08 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x151600 << 1))
    ldr         r0, =0xffff
    str         r0, [r1, #(0x20 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x22 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x24 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x28 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x2a << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x2c << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x2e << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x14 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x60 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x10 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0xa000
    str         r0, [r1, #(0xfe << 1)]

MIU_Unmask_Bist_End:
#endif
    bx          lr

//-------------------------------------------------------------------------------------------------
// Miu_Improve_Efficiency
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
Miu_Improve_Efficiency:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101800 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #((0xc0 << 1) + 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x313a00 << 1))
    ldr         r0, =0x01
    strb        r0, [r1, #(0x00 << 1)]

    //------------------
    // PRE_DTOP SETTING
    //------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldr         r0, =0x1001
    str         r0, [r1, #(0xfe << 1)]

    ldr         r0, =0x2848
    str         r0, [r1, #(0x96 << 1)]

    ldr         r0, =0x0005
    str         r0, [r1, #(0x9a << 1)]

    ldr         r0, =0x7ffe
    str         r0, [r1, #(0x98 << 1)]

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //------------------
    // DTOP CH0 SETTING
    //------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x1800
    str         r0, [r1, #(0x88 << 1)]

    ldr         r0, =0xff12
    str         r0, [r1, #(0x8a << 1)]

    ldr         r0, =0xce52
    str         r0, [r1, #(0xaa << 1)]

    ldr         r0, =0x0007
    str         r0, [r1, #(0xac << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    //ldr         r0, =0x000f
    //str         r0, [r1, #(0x2e << 1)]

    ldr         r0, =0x4804
    str         r0, [r1, #(0x30 << 1)]
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //------------------
    // DTOP CH1 SETTING
    //------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x1800
    str         r0, [r1, #(0x88 << 1)]

    ldr         r0, =0xff12
    str         r0, [r1, #(0x8a << 1)]

    ldr         r0, =0xce52
    str         r0, [r1, #(0xaa << 1)]

    ldr         r0, =0x0007
    str         r0, [r1, #(0xac << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    //ldr         r0, =0x000f
    //str         r0, [r1, #(0x2e << 1)]

    ldr         r0, =0x4804
    str         r0, [r1, #(0x30 << 1)]
#endif

    bx          lr
#endif

//-------------------------------------------------------------------------------------------------
// MiuBistTest
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
#if !defined(CONFIG_MIU1_DRAM_NONE)
    .global     Miu1_BistTest
Miu1_BistTest:

    DPUTK_STR   MIU_MESSAGE_BIST1
    // disable miu protect
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x301b00 << 1))
    ldr         r11, =0x0000
    str         r11, [r12]
    str         r11, [r12, #(0x02 << 1)]

    //unmask miu request
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r11, =0x7FFE
    str         r11, [r12, #(0xe6 << 1)]

    //reset bist test
    ldr         r11, =0x0000
    str         r11, [r12, #(0xe0 << 1)]

    b           Miu_BistTest
#endif

    .global     Miu0_BistTest
Miu0_BistTest:

    DPUTK_STR   MIU_MESSAGE_BIST0

    //disable MIU protect
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x301a00 << 1))
    ldr         r11, =0x0000
    str         r11, [r12]
    str         r11, [r12, #(0x02 << 1)]

    ldr         r12, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldr         r11, =0x7FFE
    str         r11, [r12, #(0xe6 << 1)]

    //reset bist test
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r11, =0x0000
    str         r11, [r12, #(0xe0 << 1)]

    .global     Miu_BistTest
Miu_BistTest:

    ldr         r11, =0x0000
    str         r11, [r12, #(0xe2 << 1)]    //start address base unit: 16k bytes

    // unit: 32k bytes
    ldr         r0, =0xFFFF
    ldr         r11, =BIST_LEN
    and         r11, r11, r0
    str         r11, [r12, #(0xe4 << 1)]
    ldr         r11, =BIST_LEN
    lsr         r11, #16
    and         r11, r11, r0
    str         r11, [r12, #(0xe6 << 1)]

    ldr         r11, =0x5aa5
    str         r11, [r12, #(0xe8 << 1)]    //bist test pattern

    ldr         r11, =0x0007                //bist test mode:7
    str         r11, [r12, #(0xe0 << 1)]

Miu_CheckBistLoop:
    ldr         r1, [r12, #(0xe0 << 1)]
    and         r0, r1, #0x8000
    cmp         r0, #0
    beq         Miu_CheckBistLoop

    and         r0, r1, #0x6000
    cmp         r0, #0
    beq         Miu_Bist_OK

Miu_Bist_Fail:
    DPUTK_STR   MIU_MESSAGE_FAIL

    ldr         r12, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldrb        r11, [r12, #(0xa0 << 1)]
    cmp         r11, #0x80
    beq         Miu_Bist_FailLoop
    cmp         r11, #0
    bne         Miu_Bist_Fail_Settings_End
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldrb        r11, [r12, #(0x14 << 1)]
    orr         r11, r11, #0x1
    strb        r11, [r12, #(0x14 << 1)]
    ldr         r11, =0x0400
    str         r11, [r12, #(0x1c << 1)]
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r11, =0x0400
    str         r11, [r12, #(0x1c << 1)]
Miu_Bist_Fail_Settings_End:
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldrb        r11, [r12, #(0xa0 << 1)]
    add         r11, r11, #1
    strb        r11, [r12, #(0xa0 << 1)]

    //reset bist test
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r11, =0x0000
    str         r11, [r12, #(0xe0 << 1)]

    ldr         r12, =(MS_BUS_RIU_ADDR + (0x161600 << 1))
    ldr         r11, =0x0023
    str         r11, [r12, #(0x1c << 1)]
    DELAYUS     1
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x161600 << 1))
    ldr         r11, =0x0020
    str         r11, [r12, #(0x1c << 1)]
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))
    ldr         r11, =0x0023
    str         r11, [r12, #(0x1c << 1)]
    DELAYUS     1
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))
    ldr         r11, =0x0020
    str         r11, [r12, #(0x1c << 1)]

    ldr         r12, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldrb        r11, [r12, #(0x1a << 1)]
    orr         r11, r11, #0x01
    strb        r11, [r12, #(0x1a << 1)]
    ldrb        r11, [r12, #(0x1e << 1)]
    bic         r11, r11, #0x0f
    orr         r11, r11, #0x02
    strb        r11, [r12, #(0x1e << 1)]
    ldrb        r11, [r12, #(0x1f << 1)]
    orr         r11, r11, #0x40
    strb        r11, [r12, #(0x1f << 1)]
    ldrb        r11, [r12, #(0x1e << 1)]
    bic         r11, r11, #0x0f
    orr         r11, r11, #0x01
    strb        r11, [r12, #(0x1e << 1)]
    ldrb        r11, [r12, #(0x1f << 1)]
    orr         r11, r11, #0x40
    strb        r11, [r12, #(0x1f << 1)]

    b           Miu0_BistTest

Miu_Bist_FailLoop:
    b           Miu_Bist_FailLoop

Miu_Bist_OK:
    DPUTK_STR   MIU_MESSAGE_OK

    ldr         r12, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldrb        r11, [r12, #(0x14 << 1)]
    bic         r11, r11, #0x1
    strb        r11, [r12, #(0x14 << 1)]

Miu_Bist_Exit:

    bx          lr

#ifdef CONFIG_MSTAR_MOBF_ENABLE
//-------------------------------------------------------------------------------------------------
// BootRom_InitMobf_Miu
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitMobf_Miu
BootRom_InitMobf_Miu:

Obf_CheckTRNGLoop:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113D00 << 1))
    ldr         r2, [r1, #(0x06 << 1)]
    and         r2, r2, #0x0002
    cmp         r0, #0
    beq         Obf_CheckTRNGLoop

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113B00 << 1))
    ldr         r2, [r1, #(0xEC << 1)]
    bic         r2, r2, #0x0060
    str         r2, [r1, #(0xEC << 1)]

    ldr         r3, =0xC000
    orr         r2, r2, r3
    str         r2, [r1, #(0xEC << 1)]

    bx          lr
#endif

//-------------------------------------------------------------------------------------------------
// Miu_Check_DramSupport
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
Miu_Check_DramSupport:

    push        {lr}

    ldr         DRAM_SEL, =0x1
    ldr         DRAM_VENDOR, =MIU0_1ST_VENDOR
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, [r1, #(0xc0 << 1)]
    lsr         r0, r0, #8
    cmp         r0, #MIU0_1ST_BONDING
    beq         Miu_Dram_Support
#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU0_MAX_SUPPORT_NUM >= 2)
    ldr         DRAM_SEL, =0x2
    ldr         DRAM_VENDOR, =MIU0_2ND_VENDOR
    cmp         r0, #MIU0_2ND_BONDING
    beq         Miu_Dram_Support
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 3)
    ldr         DRAM_SEL, =0x3
    ldr         DRAM_VENDOR, =MIU0_3RD_VENDOR
    cmp         r0, #MIU0_3RD_BONDING
    beq         Miu_Dram_Support
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 4)
    ldr         DRAM_SEL, =0x4
    ldr         DRAM_VENDOR, =MIU0_4TH_VENDOR
    cmp         r0, #MIU0_4TH_BONDING
    beq         Miu_Dram_Support
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 5)
    ldr         DRAM_SEL, =0x5
    ldr         DRAM_VENDOR, =MIU0_5TH_VENDOR
    cmp         r0, #MIU0_5TH_BONDING
    beq         Miu_Dram_Support
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 6)
    ldr         DRAM_SEL, =0x6
    ldr         DRAM_VENDOR, =MIU0_6TH_VENDOR
    cmp         r0, #MIU0_6TH_BONDING
    beq         Miu_Dram_Support
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 7)
    ldr         DRAM_SEL, =0x7
    ldr         DRAM_VENDOR, =MIU0_7TH_VENDOR
    cmp         r0, #MIU0_7TH_BONDING
    beq         Miu_Dram_Support
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 8)
    ldr         DRAM_SEL, =0x8
    ldr         DRAM_VENDOR, =MIU0_8TH_VENDOR
    cmp         r0, #MIU0_8TH_BONDING
    beq         Miu_Dram_Support
#endif
#endif

Miu_Dram_NotSupport:
    DPUTK_STR   MIU_MESSAGE_NOTSUPPORT

Miu_Dram_NotSupport_Loop:
    b           Miu_Dram_NotSupport_Loop

Miu_Dram_Support:
    pop         {lr}
    bx          lr

Miu_Set_Dram_Sel:
    push        {DRAM_SEL}
    push        {DRAM_VENDOR}
    bx          lr

Miu_Get_Dram_Sel:
    pop         {DRAM_VENDOR}
    pop         {DRAM_SEL}
    bx          lr

    .ltorg

Miu_Set_Version:
    push        {lr}

#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU0_MAX_SUPPORT_NUM >= 2)
    cmp         DRAM_SEL, #0x2
    beq         Miu_Size_Sel_2nd
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 3)
    cmp         DRAM_SEL, #0x3
    beq         Miu_Size_Sel_3rd
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 4)
    cmp         DRAM_SEL, #0x4
    beq         Miu_Size_Sel_4th
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 5)
    cmp         DRAM_SEL, #0x5
    beq         Miu_Size_Sel_5th
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 6)
    cmp         DRAM_SEL, #0x6
    beq         Miu_Size_Sel_6th
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 7)
    cmp         DRAM_SEL, #0x7
    beq         Miu_Size_Sel_7th
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 8)
    cmp         DRAM_SEL, #0x8
    beq         Miu_Size_Sel_8th
#endif
#endif

    ldr         MIU0_SIZE_REG, =MIU0_1ST_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_1ST_SIZE_Gb
    b           Miu_Ver_Size_set

#if defined(MIU0_MAX_SUPPORT_NUM)
#if (MIU0_MAX_SUPPORT_NUM >= 2)
Miu_Size_Sel_2nd:
    ldr         MIU0_SIZE_REG, =MIU0_2ND_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_2ND_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 3)
Miu_Size_Sel_3rd:
    ldr         MIU0_SIZE_REG, =MIU0_3RD_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_3RD_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 4)
Miu_Size_Sel_4th:
    ldr         MIU0_SIZE_REG, =MIU0_4TH_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_4TH_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 5)
Miu_Size_Sel_5th:
    ldr         MIU0_SIZE_REG, =MIU0_5TH_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_5TH_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 6)
Miu_Size_Sel_6th:
    ldr         MIU0_SIZE_REG, =MIU0_6TH_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_6TH_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 7)
Miu_Size_Sel_7th:
    ldr         MIU0_SIZE_REG, =MIU0_7TH_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_7TH_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#if (MIU0_MAX_SUPPORT_NUM >= 8)
Miu_Size_Sel_8th:
    ldr         MIU0_SIZE_REG, =MIU0_8TH_SIZE_Gb
    ldr         MIU1_SIZE_REG, =MIU1_8TH_SIZE_Gb
    b           Miu_Ver_Size_set
#endif
#endif

Miu_Ver_Size_set:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103300 << 1))
    ldrb        r0, [r1, #((0xa2 << 1) + 1)]
    bic         r0, r0, #0xf0
    lsl         r2, MIU0_SIZE_REG, #4
    orr         r0, r0, r2
    strb        r0, [r1, #((0xa2 << 1) + 1)]
    ldrb        r0, [r1, #((0xb2 << 1) + 1)]
    bic         r0, r0, #0xf0
    lsl         r2, MIU1_SIZE_REG, #4
    orr         r0, r0, r2
    strb        r0, [r1, #((0xb2 << 1) + 1)]

    str         DRAM_VENDOR, [r1, #(0xa6 << 1)]
    str         DRAM_VENDOR, [r1, #(0xb6 << 1)]

    pop         {lr}
    bx          lr

    .size       BootRom_InitMiu,.-BootRom_InitMiu
