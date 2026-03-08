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
typedef unsigned int   U16;
typedef unsigned long  U32;

#define RIU_MAP  0xA0000000
#define RIU      ((unsigned short volatile *) RIU_MAP)

#define BIT0        0x0001
#define BIT1        0x0002
#define BIT2        0x0004
#define BIT3        0x0008
#define BIT4        0x0010
#define BIT5        0x0020
#define BIT6        0x0040
#define BIT7        0x0080
#define BIT8        0x0100
#define BIT9        0x0200
#define BIT10       0x0400
#define BIT11       0x0800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000

#define BRINGUP_MCU51   0


void PM_MCU_Init(void);
void PM_MCU_Init(void)
{



     #if (BRINGUP_MCU51)
    U32 start_addr = 0x00000;
    U32 end_addr = 0xff0000;
    U32 u32temCnt;
    #endif
    U16 u16temp;

    #if 1//gating mcu51 clock
    u16temp = *((unsigned short volatile *) 0xBF001ca4);
    u16temp = u16temp & (~BIT12);
    *((unsigned short volatile *) 0xBF001ca4) = u16temp;
    #endif

    #if (BRINGUP_MCU51)
    //set reset password 0x0e40=0x0c00
    //*((unsigned short volatile *) 0xBF001c80) = 0x0c00;

    #if 1//---Important!!!
    *((unsigned short volatile *) 0xBF001c80) &= (U16)(~BIT14); //spi_clk=xtal
    *((unsigned short volatile *) 0xBF001c80) &= (U16)(~BIT7);  //mcu51 clk=xtal
    *((unsigned short volatile *) 0xBF002030) &= (U16)(~BIT3);  // i_cache rstz
    *((unsigned short volatile *) 0xBF005740) |= (U16)(BIT0);   //disable i cache
    #endif

      *((unsigned short volatile *) 0xBF002028) = (U16)(start_addr & 0x0000FFFF);
    *((unsigned short volatile *) 0xBF002020) = (U16)((start_addr>>16) & 0x000000FF);
    *((unsigned short volatile *) 0xBF00202c) = (U16)(end_addr & 0x0000FFFF);
    *((unsigned short volatile *) 0xBF002024) = (U16)((end_addr>>16) & 0x000000FF);

    #if 1 //T3
    //set spi offset set reg_spi_offset_addr[7:0]     0x0010f9 = 0x01     => 0x10000
    //set reg_spi_offset_en               0x0010f8 = 0x01
    *((unsigned short volatile *) 0xBF0021f0) = 0x0101;
    #else //U3

    *((unsigned short volatile *) 0xBF007834) = 0x01;

    #endif
    //set reset password 0x0e54=0x829f
     *((unsigned short volatile *) 0xBF001cA8) = 0x829f;

    //reg_fire[3:0]                   0x000e53 = 0x01

    //reg_fire[3:0]                   0x000e53 = 0x00

    u16temp = *((unsigned short volatile *) 0xBF001ca4);
    u16temp = u16temp | (BIT8);
    *((unsigned short volatile *) 0xBF001ca4) = u16temp;

    for(u32temCnt=0; u32temCnt<0xffff; u32temCnt++);

    u16temp = *((unsigned short volatile *) 0xBF001ca4);
    u16temp = u16temp & (~BIT8);
    *((unsigned short volatile *) 0xBF001ca4) = u16temp;

    u16temp = *((unsigned short volatile *) 0xBF001ca4);
    u16temp = u16temp & (~BIT12);
    *((unsigned short volatile *) 0xBF001ca4) = u16temp;

    for(u32temCnt=0; u32temCnt<0xffff; u32temCnt++);

    u16temp = *((unsigned short volatile *) 0xBF001ca4);
    u16temp = u16temp | (BIT12);
    *((unsigned short volatile *) 0xBF001ca4) = u16temp;

    *((unsigned short volatile *) 0xBF001c48) = 0x08;
    u16temp=0;

    while(*((unsigned short volatile *) 0xBF001c48) == 0x08)
    {
        *((unsigned short volatile *) 0xBF206700) = (u16temp++);
        *((unsigned short volatile *) 0xBF206704) = (0x1234);
        //*((unsigned short volatile *) 0xBF206708) = (0x5678);

    }

	while(1)
	{
	    *((unsigned short volatile *) 0xBF206708) = (u16temp++);
	    *((unsigned short volatile *) 0xBF20670A) = (0x5678);
	}


#endif

   //while(1);

}



