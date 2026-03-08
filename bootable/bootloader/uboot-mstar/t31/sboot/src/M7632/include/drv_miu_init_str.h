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

#ifndef _DRV_MIU_INIT_STR_H_
#define _DRV_MIU_INIT_STR_H_

#ifndef _BOARD_H_
#include "Board.h"
#endif

#ifndef _C_RIUBASE_H_
#include "c_riubase.h"
#endif

#define MIU_VER                         'M','I','U','_','M','7','6','3','2',' ','V','e','r',':','1','_','0','_','0'
#define REG_ADDR_BASE                   0x1f000000

#ifndef __ASSEMBLER__

#ifndef __DRV_RIU_H__
#include "drvRIU.h"
#endif

#if (ENABLE_MSTAR_TITANIA_BD_FPGA == 1)
#ifndef _MIU_FPGA_M7632_H_
#include "MIU_FPGA_M7632.h"
#endif
#endif

#ifndef _MIU_MTV19006_MT5871_M7632_STR_H_
#include "MIU_MTV19006_MT5871_M7632_STR.h"
#endif

#ifndef _MIU_MTV19008_MT5871_M7632_STR_H_
#include "MIU_MTV19008_MT5871_M7632_STR.h"
#endif

#ifndef _MIU_MT164B_10AT_M7632_STR_H_
#include "MIU_MT164B_10AT_M7632_STR.h"
#endif

const MS_REG_INIT MIU_PreInit_Str[] =
{
    //MIU Channel Config Setting
    _RV32_2(0x1012d6, 0x0020),
    _RV32_2(0x1006d6, 0x0020),
    _RV32_2(0x1012dc, 0x000c),
    _RV32_2(0x152b36, 0x00c0),
    _RV32_2(0x152b28, 0x1000),
    _RV32_2(0x152c28, 0x0001),

    //MIU Reset & Mask all
    _RV32_2(0x10121e, 0x0c00),
    _RV32_2(0x10121e, 0x0c00),
    _RV32_2(0x10121e, 0x0c00),
    _RV32_2(0x10121e, 0x0c01),
    _RV32_2(0x1615e6, 0xfffe),
    _RV32_2(0x110d78, 0x0101),
    _RV32_2(0x110d36, 0x0000),
    _RV32_2(0x110d34, 0x0102),

    //MIU Reset & Mask all
    _RV32_2(0x10061e, 0x0c00),
    _RV32_2(0x10061e, 0x0c00),
    _RV32_2(0x10061e, 0x0c00),
    _RV32_2(0x10061e, 0x0c01),
    _RV32_2(0x1622e6, 0xfffe),
    _RV32_2(0x161678, 0x0101),
    _RV32_2(0x161636, 0x0000),
    _RV32_2(0x161634, 0x0102),

    //Reset VCO avoid overtone
    _RV32_2(0x110d8c, 0x0400),
    _RV32_2(0x16168c, 0x0400),

    _END_OF_TBL32_,
    MIU_VER
};

//-----------------------------------------------
const MS_REG_INIT MIU_PostInit_Str[] =
{
    //Unmask clients
    _RV32_2(0x310206, 0x9fff),
    _RV32_2(0x310306, 0x0000),
    _RV32_2(0x310406, 0x0000),
    _RV32_2(0x310506, 0x0000),
    _RV32_2(0x310606, 0x0000),
    _RV32_2(0x310706, 0x0000),
    _RV32_2(0x310806, 0x0000),
    _RV32_2(0x310906, 0x0000),
    _RV32_2(0x311006, 0x0000),
    _RV32_2(0x3102f0, 0x0000),
    _RV32_2(0x3103f0, 0x0000),
    _RV32_2(0x3104f0, 0x0000),
    _RV32_2(0x3105f0, 0x0000),
    _RV32_2(0x3106f0, 0x0000),
    _RV32_2(0x3107f0, 0x0000),
    _RV32_2(0x3108f0, 0x0000),
    _RV32_2(0x3109f0, 0x0000),
    _RV32_2(0x3110f0, 0x0000),
    _RV32_2(0x3102f2, 0x0000),
    _RV32_2(0x3103f2, 0x0000),
    _RV32_2(0x3104f2, 0x0000),
    _RV32_2(0x3105f2, 0x0000),
    _RV32_2(0x3106f2, 0x0000),
    _RV32_2(0x3107f2, 0x0000),
    _RV32_2(0x3108f2, 0x0000),
    _RV32_2(0x3109f2, 0x0000),
    _RV32_2(0x3110f2, 0x0000),
    _RV32_2(0x3102f4, 0x0000),
    _RV32_2(0x3103f4, 0x0000),
    _RV32_2(0x3104f4, 0x0000),
    _RV32_2(0x3105f4, 0x0000),
    _RV32_2(0x3106f4, 0x0000),
    _RV32_2(0x3107f4, 0x0000),
    _RV32_2(0x3108f4, 0x0000),
    _RV32_2(0x3109f4, 0x0000),
    _RV32_2(0x3110f4, 0x0000),
    _RV32_2(0x3102f6, 0x0000),
    _RV32_2(0x3103f6, 0x0000),
    _RV32_2(0x3104f6, 0x0000),
    _RV32_2(0x3105f6, 0x0000),
    _RV32_2(0x3106f6, 0x0000),
    _RV32_2(0x3107f6, 0x0000),
    _RV32_2(0x3108f6, 0x0000),
    _RV32_2(0x3109f6, 0x0000),
    _RV32_2(0x3110f6, 0x0000),
    _RV32_2(0x31023e, 0x0000),
    _RV32_2(0x31033e, 0x0000),
    _RV32_2(0x31043e, 0x0000),
    _RV32_2(0x31053e, 0x0000),
    _RV32_2(0x31063e, 0x0000),
    _RV32_2(0x31073e, 0x0000),
    _RV32_2(0x31083e, 0x0000),
    _RV32_2(0x31093e, 0x0000),
    _RV32_2(0x31103e, 0x0000),
    _RV32_2(0x1615e6, 0x0000),
    _RV32_2(0x10121e, 0x0c08),

    _END_OF_TBL32_,
    MIU_VER
};


#endif /* !__ASSEMBLER__ */

#endif /* _DRV_MIU_INIT_H_ */
