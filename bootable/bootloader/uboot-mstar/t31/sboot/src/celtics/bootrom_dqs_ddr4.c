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

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Define
//-------------------------------------------------------------------------------------------------
#define MIU_RIU_REG_BASE                   0x1F000000

#define MIU0_RIU_ATOP                      0x110d
#define MIU1_RIU_ATOP                      0x1616
#define MIU2_RIU_ATOP                      0x1616
#define MIU0_RIU_DTOP                      0x1012
#define MIU1_RIU_DTOP                      0x1006
#define MIU2_RIU_DTOP                      0x1006

#define REG_ATOP_REGION                    0x2F
#define DEBUG

typedef unsigned char   u8;
typedef unsigned int   u16;
typedef unsigned long  u32;

#define MHal_MIU_ReadReg16(u32bank, u32reg ) *( ( volatile u16* ) (MIU_RIU_REG_BASE + (u32bank)*0x100*2 + ((u32reg) << 1)) )
#define MHal_MIU_WritReg16(u32bank, u32reg, u16val ) *( ( volatile u16* ) (MIU_RIU_REG_BASE + (u32bank)*0x100*2 + ((u32reg) << 1))  ) = (u16val)

//-------------------------------------------------------------------------------------------------
//  Prototypes
//-------------------------------------------------------------------------------------------------
void putn( u8 n );
void putk( char c );
void BootRom_MiuTxAutoDqs( u8 u8Miu );
void BootRom_MiuDdr4Dqs(void);
void MHal_MIU_Dqs_Phase2( u8 u8Miu );

//-------------------------------------------------------------------------------------------------
//  Local variables
//-------------------------------------------------------------------------------------------------
u16 u16PhsIIPatTbl[4][16] = {{0xffff, 0xffff, 0xcccc, 0xcccc, 0x3333, 0x3333, 0xaaaa, 0xaaaa, 0xffff, 0xffff, 0xffff, 0xffff, 0xcccc, 0xcccc, 0xaaaa, 0xaaaa},
                             {0xffff, 0xffff, 0x9999, 0x9999, 0x6666, 0x6666, 0x5555, 0x5555, 0xffff, 0xffff, 0xffff, 0xffff, 0x9999, 0x9999, 0x5555, 0x5555},
                             {0xffff, 0xffff, 0x3333, 0x3333, 0xcccc, 0xcccc, 0xaaaa, 0xaaaa, 0xffff, 0xffff, 0xffff, 0xffff, 0x3333, 0x3333, 0xaaaa, 0xaaaa},
                             {0xffff, 0xffff, 0x6666, 0x6666, 0x9999, 0x9999, 0x5555, 0x5555, 0xffff, 0xffff, 0xffff, 0xffff, 0x6666, 0x6666, 0x5555, 0x5555}};


//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void putn(u8 n)
{
    char c = '0' + n;
          
   *(volatile unsigned int*)(0x1F201300) = c;
}

void putk(char c)
{
   *(volatile unsigned int*)(0x1F201300) = c;
}

void MHal_MIU_Dqs_Phase2( u8 u8Miu )
{
    int u16TrigNum;
    int u16MprNum;
    int u16DqsNum;
    u16 u16RegVal;
    u16 u16Bank;
    
    if(u8Miu == 0)
        u16Bank = MIU0_RIU_DTOP;
    else if(u8Miu == 1)
        u16Bank = MIU1_RIU_DTOP;
    else
        u16Bank = MIU2_RIU_DTOP;

    /*MPR0 Write*/
    MHal_MIU_WritReg16(0x1012, 0x18, 0x0400);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x841f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x851f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x841f);
    MHal_MIU_WritReg16(0x1012, 0x18, 0x00ac);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x801f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x811f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x801f);
    MHal_MIU_WritReg16(0x1012, 0x18, 0x00ac);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x891f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);

    /*MPR1 Write*/
    MHal_MIU_WritReg16(0x1012, 0x18, 0x00bd);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x891f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);

    /*MPR2 Write*/
    MHal_MIU_WritReg16(0x1012, 0x18, 0x00ce);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x891f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);

    /*MPR2 Write*/
    MHal_MIU_WritReg16(0x1012, 0x18, 0x00df);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x891f);
    MHal_MIU_WritReg16(0x1012, 0x00, 0x881f);

    for(u16TrigNum = 0; u16TrigNum < 5; u16TrigNum++)
    { 
        u16RegVal = 0;
        u16RegVal = (u16TrigNum << 8) | u16TrigNum;
        MHal_MIU_WritReg16(0x110d, 0x74, u16RegVal);
        MHal_MIU_WritReg16(0x110d, 0x76, u16RegVal);
        
        for(u16MprNum = 0; u16MprNum < 4; u16MprNum++)
        {
        #if 0
            for(u16DqsNum = 0; u16DqsNum < 16; u16DqsNum++)
            {
                if(u16DqsNum < 8)
                {
                    u16RegVal = 0x0;
                    u16RegVal = u16RegVal + 0x400*u16DqsNum;
                    MHal_MIU_WritReg16(0x1012, 0x2a, 0x0000);
                }
                else
                {
                    u16RegVal = 0x2000;
                    u16RegVal = u16RegVal + 0x400*(u16DqsNum - 8);
                    MHal_MIU_WritReg16(0x1012, 0x2a, 0x80);
                }
                
                MHal_MIU_WritReg16(0x1012, 0xe0, u16RegVal);
                u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
                if(u16RegVal == u16PhsIIPatTbl[u16MprNum][u16DqsNum])
                {
                    putk('k');
                    break;
                }
            }
        #else
            u16RegVal = 0;
            u16RegVal = (u16TrigNum << 13) | u16TrigNum;
            MHal_MIU_WritReg16(0x1012, 0x18, u16RegVal);
            MHal_MIU_WritReg16(0x1012, 0x00, 0x8a1f);
            MHal_MIU_WritReg16(0x1012, 0x00, 0x8b1f);
            MHal_MIU_WritReg16(0x1012, 0x00, 0x8a1f);

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x0000);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][0])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x0400);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][1])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x0800);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][2])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x0c00);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][3])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x1000);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][4])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x1400);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][5])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x1800);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][6])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x1c00);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][7])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0x2a, 0x0080);

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x2000);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][8])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x2400);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][9])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x2800);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][10])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x2c00);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][11])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x3000);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][12])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x3400);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][13])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x3800);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][14])
            {
                putk('k');
                continue;
            }

            MHal_MIU_WritReg16(0x1012, 0xe0, 0x3c00);
            u16RegVal = MHal_MIU_ReadReg16(0x1012, 0xea);
            if(u16RegVal == u16PhsIIPatTbl[u16MprNum][15])
            {
                putk('k');
                continue;
            }
        #endif
        }
    }
}

void MHal_MIU_Dqs_Phase4(u8 u8Miu )
{
    int u16VerVal;
    u16 u16RegVal;
    u16 u16MaxMarg = 0x0000;
    u16 u16MinMarg = 0xffff;
    u16 u16MaxVerVal = 0;
    u16 u16MinVerVal = 0;

    for(u16VerVal = 0; u16VerVal < 22; u16VerVal ++)
    {
        u16RegVal = 0;
        u16RegVal = (0x0001 << 7) | u16VerVal;
        MHal_MIU_WritReg16(0x1012, 0x18, u16RegVal);
        MHal_MIU_WritReg16(0x1012, 0x00, 0x801f);
        MHal_MIU_WritReg16(0x1012, 0x00, 0x811f);
        MHal_MIU_WritReg16(0x1012, 0x00, 0x801f);
         
        /*DPAT engine pre-setting*/
        MHal_MIU_WritReg16(0x1615, 0x78, 0x0004);
        MHal_MIU_WritReg16(0x1615, 0x78, 0x0000);
        MHal_MIU_WritReg16(0x1615, 0x70, 0x0600);
        MHal_MIU_WritReg16(0x1615, 0x72, 0x0000);
        MHal_MIU_WritReg16(0x1615, 0x74, 0x0833);

        /*Enable TX DQ deskew scan (by DPAT engine)*/
        MHal_MIU_WritReg16(0x1615, 0x76, 0x0002);
        MHal_MIU_WritReg16(0x1615, 0x78, 0x0013);
        MHal_MIU_WritReg16(0x1615, 0x7c, 0x8009);

        u16RegVal = MHal_MIU_ReadReg16(0x1615, 0x7e);

        if(u16RegVal > u16MaxMarg)
        {
            u16MaxMarg = u16RegVal;
            u16MaxVerVal = u16VerVal;
        }

        if(u16RegVal < u16MinMarg)
        {
            u16MinMarg = u16RegVal;
            u16MinVerVal = u16VerVal;
        }
    }
}

void MHal_MIU_Dqs_Phase7( u8 u8Miu )
{
    int u16TrigNum;
    int u16MaxDqs0_7 = 0;
    int u16MaxDqs8_15 = 0;
    int u16MaxDqs16_23 = 0;
    int u16MaxDqs24_31 = 0;
    int u16Dqs0_7;
    int u16Dqs8_15;
    int u16Dqs16_23;
    int u16Dqs24_31;
    int u16TrigNum0_7 = 0;
    int u16TrigNum8_15 = 0;
    int u16TrigNum16_23 = 0;
    int u16TrigNum24_31 = 0;
    u16 u16RegVal;
    
    for(u16TrigNum = 0; u16TrigNum < 12; u16TrigNum++)
    {
        u16RegVal = 0;
        u16RegVal = (u16TrigNum << 8) | u16TrigNum;
        MHal_MIU_WritReg16(0x110d, 0x74, u16RegVal);
        MHal_MIU_WritReg16(0x110d, 0x76, u16RegVal);
 
        MHal_MIU_WritReg16(0x1615, 0x78, 0x0004);
        MHal_MIU_WritReg16(0x1615, 0x78, 0x0000);
        MHal_MIU_WritReg16(0x110d, 0x70, 0x0000);
        MHal_MIU_WritReg16(0x110d, 0x90, 0xf0f0);
        MHal_MIU_WritReg16(0x110d, 0x70, 0x0800);

        MHal_MIU_WritReg16(0x1615, 0x7c, 0x800e);
        u16RegVal = MHal_MIU_ReadReg16(0x1615, 0x7e);
        u16Dqs0_7 = u16RegVal & 0x007f;
        u16Dqs8_15 = (u16RegVal >> 8) & 0x007f;
        
        MHal_MIU_WritReg16(0x1615, 0x7c, 0x800f);
        u16RegVal = MHal_MIU_ReadReg16(0x1615, 0x7e);
        u16Dqs16_23 = u16RegVal & 0x007f;
        u16Dqs24_31 = (u16RegVal >> 8) & 0x007f;
        
        if( u16Dqs0_7 > u16MaxDqs0_7)
        {
            u16MaxDqs0_7 = u16Dqs0_7;
            u16TrigNum0_7 = u16TrigNum;
        }
        
        if( u16Dqs8_15 > u16MaxDqs8_15)
        {
            u16MaxDqs8_15 = u16Dqs8_15;
            u16TrigNum8_15 = u16TrigNum;
        }

        if( u16Dqs16_23 > u16MaxDqs16_23)
        {
            u16MaxDqs16_23 = u16Dqs16_23;
            u16TrigNum16_23 = u16TrigNum;
        }

        if( u16Dqs24_31 > u16MaxDqs24_31)
        {
            u16MaxDqs24_31 = u16Dqs24_31;
            u16TrigNum24_31 = u16TrigNum;
        }
    }

    u16RegVal = (u16TrigNum8_15 << 8) | u16TrigNum0_7;
    MHal_MIU_WritReg16(0x110d, 0x74, u16RegVal);
    u16RegVal = (u16TrigNum24_31 << 8) | u16TrigNum16_23;
    MHal_MIU_WritReg16(0x110d, 0x76, u16RegVal);
}

void BootRom_MiuDdr4Dqs(void)
{
#if 1
    putk('p');
    putk('h');
    putk('a');
    putk('s');
    putk('e');
    putk('2');
    putk('_');
    putk('s');
    putk('t');
    putk('a');
    putk('r');
    putk('t');
    putk('\n');
    putk('\r');
#endif
    MHal_MIU_Dqs_Phase2(0);
#if 1
    putk('p');
    putk('h');
    putk('a');
    putk('s');
    putk('e');
    putk('2');
    putk('_');
    putk('e');
    putk('n');
    putk('d');
    putk('\n');
    putk('\r');
#endif

#if 1
    putk('p');
    putk('h');
    putk('a');
    putk('s');
    putk('e');
    putk('4');
    putk('_');
    putk('s');
    putk('t');
    putk('a');
    putk('r');
    putk('t');
    putk('\n');
    putk('\r');
#endif
    MHal_MIU_Dqs_Phase4(0);
#if 1
    putk('p');
    putk('h');
    putk('a');
    putk('s');
    putk('e');
    putk('4');
    putk('_');
    putk('e');
    putk('n');
    putk('d');
    putk('\n');
    putk('\r');
#endif

#if 1
    putk('p');
    putk('h');
    putk('a');
    putk('s');
    putk('e');
    putk('7');
    putk('_');
    putk('s');
    putk('t');
    putk('a');
    putk('r');
    putk('t');
    putk('\n');
    putk('\r');
#endif
    MHal_MIU_Dqs_Phase7(0);
#if 1
    putk('p');
    putk('h');
    putk('a');
    putk('s');
    putk('e');
    putk('7');
    putk('_');
    putk('e');
    putk('n');
    putk('d');
    putk('\n');
    putk('\r');
#endif

}
