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

#include <stdio.h>
#include "datatype.h"
#include "drvGlobal.h"
#include "drvAeonUART.h"
#include "risc32_spr.h"
#include "hwreg.h"
#include "Board.h"
#include "drv_miu_init.h"

///////////////////////////////////////////////////////////////////////////////
// power up and turn on clock
///////////////////////////////////////////////////////////////////////////////
#define MIU0_SIZE 0x4000000
#define CHIP_MIU_UNIT 0x8
#define CHIP_FPGA_VERIFY 0
#define RTL_SIMULATION  0
#define AUTO_DETECT_FLASH_MODE  1

///////////////////////////////////////////////////////////////////////////////
// MIU Speed for DDR2
///////////////////////////////////////////////////////////////////////////////
#define DDR2_1066MHz    0x01
#define DDR2_1333MHz    0x02

#define MIU_SPEED   DDR2_1066MHz//DDR2_1333MHz

#define DDRSIZE_64M   (BIT4|BIT3) //512Mb: for DDR2
#define DDRSIZE_32M   (BIT4)          //256Mb: for DDR2
#define DDRSIZE_128M (BIT3)          //1Gb: for DDR3

static BOOL g_bDramDDR3=FALSE;

///////////////////////////////////////////////////////////////////////////////
// power up and turn on clock
///////////////////////////////////////////////////////////////////////////////
#define  MDrv_MaskAllMiuRequest()   do{\
                              } while(0)

#define  MDrv_UnMaskAllMiuRequest()   do{\
                              } while(0)


#define MDrv_OpenAllMiumask()  do{\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0023*2)),  0x0000);   /* de_mask all miu_0/miu_1 request & reset */\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0033*2)),  0x0000);   /* de_mask all miu_0/miu_1 request & reset */\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0043*2)),  0x0000);   /* de_mask all miu_0/miu_1 request & reset */\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0053*2)),  0x0000);   /* de_mask all miu_0/miu_1 request & reset */\
                              } while(0)

#define MDrv_CloseAllMiumask()  do{\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0023*2)), 0x7FFE);   /* mask all miu_0 request & reset but G(0,0) & G(0,15)*/\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0033*2)), 0xFFFF);   /* mask all miu_0 request & reset */\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0043*2)), 0xFFFF);   /* mask all miu_0 request & reset */\
                                MDrv_Write2Byte((RIUBASE_MIU  + (0x0053*2)), 0xFFFF);   /* mask all miu_0 request & reset */\
                              } while(0)

#if 1//add to fix bist error system blocked issue
static void WholeChipReset(void);
static void WholeChipReset(void)
{
    MDrv_Write4Byte(0x0EA0, 0x51685168);
    MDrv_WriteRegBit(0x2E52, 1, BIT7);
    MDrv_WriteRegBit(0x2E52, 1, BIT6);
    MDrv_WriteByte(0x2E5C,0xFF);
    MDrv_WriteByte(0x2E5C,0x79);
    while(1);
}
#endif

// Auto DQS phase detection
#define ENABLE_AUTO_DQS                     1
#define AUTO_DQS_CNTCHK                     1
#define AUTO_DQS_CNTCHK_MAXPHS      5
#define AUTO_DQS_CNTCHK_ALGTEST     0 //debug usage (use test case to test this algorithm)
#define AUTO_DQS_START_ADDRESS      0x1FFF000
#define SET_AUTO_DQS_START_ADDR     (AUTO_DQS_START_ADDRESS/0x1000)  //4kbyte/unit in register setting
#define AUTO_DQS_VERIFY_SIZE        0x80000
#define SET_AUTO_DQS_VERIFY_SIZE    (AUTO_DQS_VERIFY_SIZE/0x8)      //16 byte/unit in register setting

// Auto DQS phase detection
#define AUTO_DQS_DUMP(x) x

U8 hex[] = "0123456789ABCDEF";
//const U8 g_MemTestMode[] = { 0x07, 0x09, 0x05, 0x03, 0x0B };
U8 g_MemTestMode[] = { 0x81,0x89,0x83,0x87, 0x85};  //, 0x05, 0x03, 0x0B };
U8 g_MemTestMask[] = { (U8)~33, (U8)~0xCC };

#define DQSN(step)  (3 - step / 16) // (step / 16)
//define Flash Manufacture ID support flash dual read mode
#define MID_MXIC                0xC2
#define MID_WB                  0xEF
#define MID_GD                  0xC8

static U8 MDrv_MIU_AutoDQSPhase( U8 u8Miu, U32 u32DqsPhAddr )
{
#if ENABLE_AUTO_DQS
    #if AUTO_DQS_CNTCHK
    U8 u8DqsCntInValid = 0;
    U8 u8PreStep = 0;
    #endif
    U8 u8Step;
    U8 u8Mode;
    U16 u16DqsSum;
    U16 u16DqsCnt,u16TotalGoodPhase;
    U16 u16DqsPhase;
    U16 u16DefDqsPhase;
    U8 bUseDefDqsPh;
    U32 u32PhaseResult = 0;
    U16 u16TmpDqsPhase=0, u16SftVal=0;

#if (AUTO_DQS_CNTCHK) && (AUTO_DQS_CNTCHK_ALGTEST)
    U8 k=0, u8DQSTestStep=0,u8DQSTestStepTemp=0;

for(k=0; k<8; k++)
{
    u8DqsCntInValid = 0; //reset
    u8DQSTestStep = k+4;
    u8DQSTestStepTemp = u8DQSTestStep;
    printf("\n\ru8DQSTestStep=%d\n\r",u8DQSTestStep);
#endif

    u8Miu=u8Miu;
    //set test area here
    MDrv_Write2Byte( (RIUBASE_MIU +(0x0071 *2)) , SET_AUTO_DQS_START_ADDR );  //0x3FFF );     //(4kbytes)/unit
    MDrv_Write4Byte( (RIUBASE_MIU +(0x0072 *2)) , SET_AUTO_DQS_VERIFY_SIZE ); //0x8000 );     //(16 byte)/unit
    MDrv_Write2Byte( (RIUBASE_MIU + (0x0074 *2)), 0x5aa5);   //test data

    u16DqsSum = 0;
    u16DqsCnt = 0;
    u16DqsPhase = 0;
    bUseDefDqsPh = 0;
    u16TotalGoodPhase=0;

    u16DefDqsPhase = MDrv_Read2Byte( u32DqsPhAddr );

    for ( u8Step = 0; u8Step < (16*4); u8Step++ )
    {
        AUTO_DQS_DUMP( if (!(u8Step & 0x0F)) uart_putc( '[' ) );
        
        u16SftVal = (DQSN( u8Step ) * 4);
        u16TmpDqsPhase = (u16DefDqsPhase & ((U16)(~(((U16)0x0F)<<u16SftVal)))) | (((U16)(u8Step & 0x0F))<<u16SftVal);
        //printf("\n\r[AUTO-DQS] u16TmpDqsPhase =0x%04X, ((U16)(~(((U16)0x0F)<<u16SftVal)))=%04X\n\r",u16TmpDqsPhase, ((U16)(~(((U16)0x0F)<<u16SftVal))));
        MDrv_Write2Byte(u32DqsPhAddr, u16TmpDqsPhase);

        for ( u8Mode = 0 ; u8Mode < sizeof(g_MemTestMode) ; ++u8Mode )
        {
            // enable test, order (high byte then low byte) is important
            MDrv_WriteByte( (RIUBASE_MIU +(0x0070 *2)+1), 0x00 );
            MDrv_WriteByte( (RIUBASE_MIU +(0x0070 *2)), g_MemTestMode[u8Mode] );  // pattern from data
            while ( !MDrv_ReadRegBit( (RIUBASE_MIU +(0x0070 *2)+1), BIT7 ) ) ;
            #if (AUTO_DQS_CNTCHK) && (AUTO_DQS_CNTCHK_ALGTEST)
            if ( MDrv_ReadRegBit( (RIUBASE_MIU +(0x0070 *2)+1), BIT6 ) ||((u8Step%16)==u8DQSTestStep))
            {
                AUTO_DQS_DUMP( uart_putc('#'));
                if((u8Step%16)==u8DQSTestStep)
                    printf("(%d)",u8DQSTestStep);
                u8DQSTestStep = u8DQSTestStepTemp;
                break;
            }
            #else
            if ( MDrv_ReadRegBit( (RIUBASE_MIU +(0x0070 *2)+1), BIT6 ) )
            {
                break;
            }
            #endif
            MDrv_WriteByte( (RIUBASE_MIU +(0x0070 *2)), 0x00 );
        }
        MDrv_WriteByte( (RIUBASE_MIU +(0x0070 *2)), 0x00 );

        if ( u8Mode ==  sizeof( g_MemTestMode ) )
        {
            AUTO_DQS_DUMP( uart_putc( (u8Step % 16) < 10 ? ('0' + (u8Step % 16)) : ('A' + (u8Step % 16) - 10)) );
            #if AUTO_DQS_CNTCHK
            if((u16DqsCnt>0)&&(u8Step!=(u8PreStep+1)))
            {
                if(u16DqsCnt<AUTO_DQS_CNTCHK_MAXPHS)
                {
                    u16DqsSum = (u8Step & 0x0F);
                    u16DqsCnt = 1;
                    u8PreStep = u8Step;
                }
                else
                {
                    u8Step = (u8Step/16)*16+15;
                }
            }
            else
            {
                u16DqsSum += (u8Step & 0x0F);
                ++u16DqsCnt;
                u8PreStep = u8Step;
            }
            #else
            u16DqsSum += (u8Step & 0x0F);
            ++u16DqsCnt;
            #endif
            u32PhaseResult |= (1 << u8Step);
        }

        if ( (u8Step % 16 == 15) )
        {
            #if AUTO_DQS_CNTCHK
            if(u16DqsCnt<AUTO_DQS_CNTCHK_MAXPHS)
            {
                printf("(GD-PHASE CNT %d < 5)",u16DqsCnt);
            }
            u8DqsCntInValid |= ((u16DqsCnt<AUTO_DQS_CNTCHK_MAXPHS)? (1<<((u8Step/16)&0x03)) : 0);
            //printf("\n\ru8DqsCntInValid = 0x%02X\n\r",u8DqsCntInValid);
            #endif
            if (u16DqsCnt == 0)
            {
                bUseDefDqsPh = 1;
            }
            else
            {
                u16DqsPhase |= ((u16DqsSum / u16DqsCnt) << (DQSN( u8Step ) * 4));
                u16DqsSum = 0;
                u16TotalGoodPhase+=u16DqsCnt;
                u16DqsCnt = 0;
            }
            AUTO_DQS_DUMP( uart_putc( ']' ) );
        }
    }

    MDrv_Write2Byte( u32DqsPhAddr, bUseDefDqsPh ? u16DefDqsPhase : u16DqsPhase );
    MDrv_Write4Byte(0x103390, u32PhaseResult);
    //clear miu1 result
    MDrv_Write4Byte(0x103394, 0);
    #if AUTO_DQS_CNTCHK
    if(u8DqsCntInValid!=0)
    {
        printf("\n\rAuto-DQS : Failure\n\r");
        #if AUTO_DQS_CNTCHK_ALGTEST
        #else
        while(1);//Block here
        return 0;
        #endif
    }
    #endif
    AUTO_DQS_DUMP( uart_putc( ' ' );
                   printf( "\n\r%04X\n", MDrv_Read2Byte(u32DqsPhAddr) );
                   uart_putc( '\n' ) );

#if (AUTO_DQS_CNTCHK) && (AUTO_DQS_CNTCHK_ALGTEST)
}
#endif

    return( u16TotalGoodPhase );
#else
    u8Miu=u8Miu;
    return 0;
#endif
}

extern U16 risc32_regbase[];

unsigned char *loader_2nd_lma;
unsigned char *loader_2nd_vma;
unsigned char *loader_2nd_len;
extern void loadAP(void);
extern void CopyFLH2MIU(U32 dst, U32 src, U32 size);
void MDrv_WriteRegTbl32( const U16 * pRegTable );
//================================================
void MDrv_WriteRegTbl32( const U16 * pRegTable )
{
   U16 u16Index; // register index
   U16 u16Dummy;

   u16Dummy = 2000;

   u16Index=0;
   while( u16Dummy )
   {
       if ( pRegTable[u16Index] == 0xffff )
          break;
       RIU[ (0x100000ul+pRegTable[u16Index]) ]=pRegTable [u16Index+1];
       u16Index+=2;
   }
}

static void MDrv_SysDelayUs(U32 u32InputTime)
{
    U32 u32DelayTime;
    U16 u16TMRStatus;

    u32DelayTime = 12 * u32InputTime;
    MDrv_Write2Byte(0x003024, u32DelayTime & 0x0000FFFF);
    MDrv_Write2Byte(0x003026, u32DelayTime >>16);
    MDrv_Write2Byte(0x003020, (MDrv_Read2Byte(0x003020) | BIT1));

    do{
        u16TMRStatus = MDrv_Read2Byte(0x003022);
        u16TMRStatus &= BIT0;
    }while(u16TMRStatus == 0);
}

void BL_Init(void);
#define UART0_BAUDRATE 115200
#define UART0_BAUDRATE_FPGA     38400

//#################################
// Step 1. Turn On Required Clocks
//#################################
static const BootConf TurnOnAllAtop[] =
{
    ////////////////////////////////////////////              			   
   // CODEC       Domain Power on                            			   
   ////////////////////////////////////////////    
    {0, 0x101e1e, BIT5, BIT5}, // : codec     block PWR_RQ        : B0 [ 5] 
    //{1, 0x101e1f, BIT1, 0x7f}, // : check     block PWR_GNT       : B1 [ 1]                     			   
    {0, 0x101e1e, BIT1, BIT1}, // : codec     block ISO_OFF       : B0 [ 1]           			   
    {0, 0x101e3a, 0, BIT0},    // : codec     block Release RESET : B0 [ 0]  

   ////////////////////////////////////////////              			   
   // SC          Domain Power on                            			   
   ////////////////////////////////////////////  
    {0, 0x101e1e, BIT4, BIT4}, // : sc        block PWR_RQ        : B0 [ 4]           			   
    //{1, 0x101e1f, BIT0, 0x7f}, // : check     block PWR_GNT       : B1 [ 0]           			   
    {0, 0x101e1e, BIT0, BIT0}, // : sc        block ISO_OFF       : B0 [ 0]           			   
    {0, 0x101e3a, 0, BIT2},    // : sc        block Release RESET : B0 [ 2] 

   ////////////////////////////////////////////              			   
   // DEMOD       Domain Power on                            			   
   ////////////////////////////////////////////              			   
    {0, 0x101e1e, BIT6, BIT6},  // : demod     block PWR_RQ        : B0 [ 6]           			   
    //{1, 0x101e1f, BIT2, 0x7f},  // : check     block PWR_GNT       : B1 [ 2]           			   
    {0, 0x101e1e, BIT2, BIT2},  // : demod     block ISO_OFF       : B0 [ 2]           			   
    {0, 0x101e3a, 0, BIT1},     // : demod     block Release RESET : B0 [ 1]           			   
   		
   ////////////////////////////////////////////              			   
   // VIVALDI     Domain Power on                            			   
   ////////////////////////////////////////////              			   
    {0, 0x101e1e, BIT7, BIT7},  // : vivaldi   block PWR_RQ        : B0 [ 7]           			   
    //{1, 0x101e1f, BIT3, 0x7f},  // : check     block PWR_GNT       : B1 [ 3]           			   
    {0, 0x101e1e, BIT3, BIT3},  // : vivaldi   block ISO_OFF       : B0 [ 3]           			   
    {0, 0x101e3a, 0, BIT3},     // : vivaldi   block Release RESET : B0 [ 3]           			   
   			   
   ////////////////////////////////////////////              			   
   // HDMI    Domain Power on                                			   
   ////////////////////////////////////////////              			   
    {0, 0x101e9c, BIT1, BIT1},  // : hdmi      block PWR_RQ        : B0 [ 1]           			   
    //{1, 0x101e9c, BIT2, 0x07},  // : check     block PWR_GNT       : B0 [ 2]           			   
    {0, 0x101e9c, BIT0, BIT0},  // : hdmi      block ISO_OFF       : B0 [ 0]           			   
    {0, 0x101e3a, 0, BIT5},     // : hdmi      block Release RESET : B0 [ 5]   

    #if 0 //diff with maya
   ////////////////////////////////////////////              			   
   // TSP         Domain Power on                            			   
   ////////////////////////////////////////////              			   
    {0, 0x101e1f, BIT5, BIT5},  // : tsp       block PWR_RQ        : B1 [ 5]           			   
    //{1, 0x101e1e, BIT6, 0xff},  // : check     block PWR_GNT       : B1 [ 6]           			   
    //{1, 0x101e1f, BIT6, 0x7f},  // : check     block PWR_GNT       : B1 [ 6]           			   
    {0, 0x101e1f, BIT4, BIT4},  // : tsp       block ISO_OFF       : B1 [ 4]           			   
    {0, 0x101e3a, 0, BIT4},     // : tsp       block Release RESET : B0 [ 4]           			   
    #endif
////////////////////////////////////////////////////                                                  			   
////////////////////////////////////////////////////                                                  			   
// EN ATOP PART                                              			   
////////////////////////////////////////////////////                                                  			   
//////////////////////////////////////////////////// 




   ////////////////////////////////////////////              			   
   // DMD_ATOP Enable                                        			   
   ////////////////////////////////////////////              			   
    {0, 0x112003, 0, BIT5},  // Release Demod ATOP SW reset                         			   
    {0, 0x112840, 0, BIT4},  // Powerdown ADC reference voltage                     			   
    {0, 0x112818, 0, BIT2},  // Powerdown clamp buffer                              			   
    {0, 0x11281e, 0, BIT7},  // Powerdown calibration buffer                        			   
    {0, 0x112818, 0, BIT0},  // Powerdown ADC I                                     			   
    {0, 0x112879, 0, BIT7},  // Powerdown 2.6V LDO                                  			   
    {0, 0x112879, 0, BIT4},  // Powerdown 2.5V LDOI                                 			   
    {0, 0x112879, 0, BIT5},  // Powerdown 1.1V LDO_CLK                              			   
    {0, 0x112879, 0, BIT6},  // Powerdown 1.1V LDO_CMP                              			   
    {0, 0x11286a, 0, BIT7},  // Powerdown MPLL                                      			   
    {0, 0x11286b, 0, BIT5},  // Power down demod PLL's ref clock
    {0, 0x112866, 0x01, 0x03}, // reg_mpll_loop_div_first                                    			   
    {0, 0x112867, 0x12, 0xFF},  // reg_mpll_loop_div_second 
    {0, 0x112860, 0, BIT1},  // Power down demod_ADC's clock                        			   
    {0, 0x112860, 0, BIT2},  // Digital divider (/2) power down                     			   
    {0, 0x112860, 0, BIT3},  // Digital divider (/3) power down                     			   
    {0, 0x112860, 0, BIT4},  // Digital divider (/4) power down                     			   
    {0, 0x112860, 0, BIT5},  // Digital divider (/8) power down                     			   
    {0, 0x112860, 0, BIT6},  // Digital divider (/10) power down                    			   
    {0, 0x11286c, BIT4, BIT4},  // Enable IFAGC_T  
    {0, 0x11288c, 0xa3, 0xff}, //reg_bond_overwrite_passwd[15:0]=0x87a3
    {0, 0x11288d, 0x87, 0xff},
    {0, 0x11288e, BIT1, BIT1|BIT0},  //reg_ext_xtali_acl_isel[1:0]=2'b10
    {0, 0x11288c, 0x00, 0xff},  //reg_bond_overwrite_passwd[15:0]=0x0000
    {0, 0x11288d, 0x00, 0xff},

   ////////////////////////////////////////////                                                                                                                                 	   
   // Enable AUPLL_ATOP                                                                                                                                                         	   
   ////////////////////////////////////////////                                                                                                                                 	   

#if 0 //(mark-out to remove burst sound)
   ////////////////////////////////////////////              			   
   // Enable AUSDM_ATOP                                      			   
   ////////////////////////////////////////////              			   
    {0, 0x112cdf, 0x78, 0xff}, // EN_CK_DAC [14:11] ;EN_DAC_DISCH [10 ] ;EN_ITEST_DAC [9];                                                                                                               	   
    {0, 0x112cde, 0x00, 0xff}, // EN_CK_DAC [14:11] ;EN_DAC_DISCH [10 ] ;EN_ITEST_DAC [9];SEL_BIAS_DAC[3 : 2];EN_IPRTSELN_EAR[1 : 0]                                                                     	   
    {0, 0x112ce0, 0x00, 0xff}, // PD_IBIAS_EAR[12   ]   ;RESERVED[11:10]   ;EN_IPRTSELP_EAR     [ 9: 8]                                                                                                  	   
    {0, 0x112ce1, 0x00, 0xff}, // PD_IBIAS_EAR[12   ]   ;RESERVED[11:10]   ;EN_IPRTSELP_EAR     [ 9: 8]                                                                                                  	   
    {0, 0x112ce2, 0x01, 0xff}, // SEL_CH_INMUX0[ 7: 4]; SEL_CH_INMUX1[ 3: 0]                                                                                                                             	   
    {0, 0x112ce3, 0x10, 0xff}, // SEL_IN_EAR[13:12]; EN_MUTE_INMUX[11:10]; PD_INMUX[ 9: 8];                                                                                                              	   
    {0, 0x112ce5, 0x00, 0xff}, // SEL_GAIN_INMUX0[15:13] ;SEL_GAIN_INMUX1[12:10] ;SEL_MICGAIN_INMUX[ 9: 8]                                                                                               	   
    {0, 0x112ce6, 0x00, 0xff}, // PD_MIDTOP[1    ]   ;PD_VI[0    ]                                                                                                                                       	   
    {0, 0x112ce7, 0x00, 0xff}, // SEL_IBIAS_INMUX[15:14]                                                                                                                                                 	   
    {0, 0x112ce8, 0x00, 0xff}, // SEL_CK_AU[4];EN_SHRT_R_ADC1[3];EN_SHRT_L_ADC1[2];EN_SHRT_R_ADC0[1];EN_SHRT_L_ADC0[0]                                                                                   	   
    {0, 0x112ce9, 0x00, 0xff}, // PD_VREF[15];EN_VREF_DISCH[14];EN_VREF_SFTDCH[13:12];                                                                                                                   	   
    {0, 0x112cea, 0x00, 0xff}, // EN_TST_IBIAS_ADC[5:4];PD_OPLP_EAR[3];EN_VMID2GND_EAR[2];SEL_TC_EAR[1:0]                                                                                                	   
    {0, 0x112ceb, 0x00, 0xff}, // SEL_IBIAS_ADC[15:11];                                                                                                                                                  	   
    {0, 0x112cec, 0x00, 0xff}, // PD_LDO_ADC[7];RESERVED[6];EN_QS_LDO_ADC[5];PD_LDO_DAC[4];EN_QS_LDO_DAC[3];RESERVED[2];SEL_VO_LDO_DAC[1:0]                                                              	   
    {0, 0x112ced, 0x18, 0xff}, // RESERVED[15:13];EN_STG2_AB_EAR[12];EN_STG2_LP_EAR[11];RESERVED[10];EN_BYP_INMUX[9:8];                                                                                  	   
    {0, 0x112cee, 0x00, 0xff}, // EN_MUTE_EAR[7];RESERVED[6];EN_TST_IPRTCTRN_EAR[5:4];EN_TST_IPRTCTRP_EAR[3:2];SEL_DEPOP_EAR[1:0]                                                                        	   
    {0, 0x112cef, 0x03, 0xff}, // RESERVED[15:13];EN_MSP[12];SEL_POPRES_EAR[11:10];EN_LTX_EAR[9];EN_RTX_EAR[8];                                                                                          	   
    {0, 0x112cf2, 0x21, 0xff}, // REG_ADC1_LR_SWAP[7];REG_ADC2_LR_SWAP[6];REG_AUSDM_EN_ADC_DITHER[5];RESERVED[4:1];REG_DEPOP_SEL[0]                                                                      	   
    {0, 0x112cf3, 0x18, 0xff}, // RESERVED[15:12];REG_ADC1_L_INV[11];REG_ADC1_R_INV[10];REG_ADC2_L_INV[9];REG_ADC2_R_INV[8]                                                                              	   
    {0, 0x112cf4, 0x00, 0xff}, // EN_SW_TST                                                                                                                                                              	   
    {0, 0x112cf5, 0x00, 0xff}, // EN_SW_TST                                                                                                                                                              	   
    {0, 0x112cda, 0xeb, 0xff}, // SEL_CHOP_ADC0[7];SEL_CHOP_ADC1[6];EN_DIT_ADC0[5];RESERVED[4];EN_DIT_ADC1[3];RESERVED[2];PD_ADC0[1];PD_ADC1[0]                                                          	   
    {0, 0x112cdb, 0x00, 0xff}, // RESET_ADC0[15];RESERVED[14];RESET_ADC1[13];RESERVED[12];SEL_DIT_LVL_ADC0[11];SEL_DIT_LVL_ADC1[10];EN_CHOP_ADC0[9];EN_CHOP_ADC1[8]                                      	   
    {0, 0x112cdc, 0x00, 0xff}, // PD_R2_DAC[7];PD_R3_DAC[6];PD_REF_DAC[5];PD_BIAS_DAC[4];RESET_DAC[3:0]                                                                                                  	   
    {0, 0x112cdd, 0x00, 0xff}, // SEL_CK_PHASE_ADC[15];RESERVED[14];PD_L0_DAC[13];PD_L1_DAC[12];PD_L2_DAC[11];PD_L3_DAC[10];PD_R0_DAC[9];PD_R1_DAC[8]                                                    	   
    {0, 0x112cda, 0xe8, 0xff}, // SEL_CHOP_ADC0[7];SEL_CHOP_ADC1[6];EN_DIT_ADC0[5];RESERVED[4];EN_DIT_ADC1[3];RESERVED[2];PD_ADC0[1];PD_ADC1[0]                                                          	   
    {0, 0x112cdb, 0x00, 0xff}, // RESET_ADC0[15];RESERVED[14];RESET_ADC1[13];RESERVED[12];SEL_DIT_LVL_ADC0[11];SEL_DIT_LVL_ADC1[10];EN_CHOP_ADC0[9];EN_CHOP_ADC1[8];                                     	   
#endif

   ////////////////////////////////////////////              			   
   // Enable MOD_ATOP                                        			   
   ////////////////////////////////////////////              			   
    {0, 0x103275, 0x01, 0xf0}, // reg_gcr_bank_clk_sel  [15:12]  //(### REVIEW ###)                      			   
    {0, 0x103275, 0x01, 0x0c}, // reserved              [11:10]                       			   
    {0, 0x103275, 0, BIT1},    // reg_gcr_reg_ref_sel   [ 9   ]                       			   
    {0, 0x103275, 0, BIT0},    // reg_gcr_en_reg        [ 8   ]                       			   
    {0, 0x103274, 0x00, 0xf0}, // reg_gcr_reg_level     [ 7: 4]                       			   
    {0, 0x103274, 0, BIT3},    // reserved              [ 3   ]                       			   
    {0, 0x103274, 0, BIT2},    // reg_gcr_en_vby1       [ 2   ]                       			   
    {0, 0x103274, 0, BIT1},    // reg_gcr_en_mini_skew  [ 1   ]                       			   
    {0, 0x103274, 0, BIT0},    // reg_pd_ib_mod         [ 0   ]                       			   
				
   ////////////////////////////////////////////
   // Enable USB_ATOP
   ////////////////////////////////////////////

    {0, 0x103a88, 0x0f, 0xff}, // UTMI0 (0x103a80)
    {0, 0x103a89, 0x04, 0xff},
    {0, 0x103a80, 0x05, 0xff},
    {0, 0x103a81, 0x7f, 0xff},

    {0, 0x103a08, 0x0f, 0xff}, // UTMI1 (0x103a00)
    {0, 0x103a09, 0x04, 0xff},
    {0, 0x103a00, 0x05, 0xff},
    {0, 0x103a01, 0x7f, 0xff},

   //=========================

                               // Module: USB0 (0x100700)
    {0, 0x100700, 0x0a, 0xff}, // Disable MAC initial suspend, Reset UHC
    {0, 0x100700, 0x28, 0xff}, // Release UHC reset, enable UHC and OTG XIU function

    {0, 0x103aa2, 0x88, 0xff}, // Module: UTMI0 (0x103a80)
    {0, 0x103aa3, 0x20, 0xff},
    {0, 0x103aa0, 0x51, 0xff}, // PLL_TEST[15]: Bypass 480MHz clock divider
    {0, 0x103aa1, 0x80, 0xff},
    {0, 0x103ab9, 0x68, 0xff}, // Turn on 320M (Whisky needed)

    {0, 0x103a80, 0xc3, 0xff}, // reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    {0, 0x103a81, 0x6b, 0xff},
    {2, 1000, 0, 0}, // delay 1ms

    {0, 0x103a80, 0xc3, 0xff}, // Turn on UPLL, reg_pdn: bit<9>
    {0, 0x103a81, 0x69, 0xff},
    {2, 2000, 0, 0}, // delay 2ms

    {0, 0x103a80, 0x01, 0xff}, // Turn all (including hs_current) use override mode
    {0, 0x103a81, 0x00, 0xff},

   //=========================

                               // Module: USB1 (0x100780)
    {0, 0x100780, 0x0a, 0xff}, // Disable MAC initial suspend, Reset UHC
    {0, 0x100780, 0x28, 0xff}, // Release UHC reset, enable UHC and OTG XIU function

    {0, 0x103a22, 0x88, 0xff}, // Module: UTMI1 (0x103a00)
    {0, 0x103a23, 0x20, 0xff},
    {0, 0x103a20, 0x51, 0xff}, // PLL_TEST[15]: Bypass 480MHz clock divider
    {0, 0x103a21, 0x80, 0xff},
    {0, 0x103a39, 0x68, 0xff}, // Turn on 320M (Whisky needed)

    {0, 0x103a00, 0xc3, 0xff}, // reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    {0, 0x103a01, 0x6b, 0xff},
    {2, 1000, 0, 0}, // delay 1ms

    {0, 0x103a00, 0xc3, 0xff}, // Turn on UPLL, reg_pdn: bit<9>
    {0, 0x103a01, 0x69, 0xff},
    {2, 2000, 0, 0}, // delay 2ms

    {0, 0x103a00, 0x01, 0xff}, // Turn all (including hs_current) use override mode
    {0, 0x103a01, 0x00, 0xff},

   ////////////////////////////////////////////              			   
   // Enable HDMI_ATOP                                       			   
   ////////////////////////////////////////////              			   
    {0, 0x000e94, 0   , BIT7}, // [7]     PD_IBGREX        = 1'd0                                                                                                                                      	   
    {0, 0x000e97, 0x00, 0x03}, // [9]     P0_PD_CLKIN_OFFL = 1'd0                                                                                                                                      	   
    {0, 0x000e98, 0   , BIT0}, // [0]     PD_BG            = 1'd0                                                                                                                                      	   
    {0, 0x1109be, 0   , BIT0}, // [0]     P0_PD_ENVDET     = 1'd0                                                                                                                                      	   
    {0, 0x1109c0, 0x00, 0xa7}, // [10:08] P0_PD_MXD        = 3'd0                                                                                                                                      	   
    {0, 0x1109c1, 0x00, 0x07}, // [10:08] P0_PD_MXD        = 3'd0                                                                                                                                      	   
    {0, 0x1109d3, 0x00, 0x07}, // [10:08] PD_P0_DPLPHI(Q)  = 3'd0                                                                                                                                      	   
    {0, 0x002ec6, 0x01, 0x0f}, // [3:0]   MHL3_ATOP_P0 NODIE_PD_RT = 4'd0
    {0, 0x002ec6, 0x10, 0xf0}, // [7:4]   MHL3_ATOP_P1 NODIE_PD_RT = 4'd0
    {0, 0x002ec7, 0x01, 0x0f}, // [11:8]  MHL3_ATOP_P2 NODIE_PD_RT = 4'd0

   ////////////////////////////////////////////
   // Enable VIDEO_ATOP
   ////////////////////////////////////////////
    {0, 0x10250e, 0x3f, 0x3f}, // soft reset                                                                                                                                                             	   
    {0, 0x10250e, 0x00, 0x3f}, // soft reset                                                                                                                                                             	   

    {0, 0x102506, BIT1, BIT1}, //Y ADC clock sel => 0: ADCPLA 1:ADCPLB  ATOP

    {0, 0x102503, BIT6, BIT6}, // en  EN_MUX_RGB                                                                                                                                                         	   
    {0, 0x102509,    0, BIT2}, // en  PD_ICLP_RGB                                                                                                                                                        	   
    {0, 0x102509,    0, BIT3}, // en  PD_ICLP_VDY                                                                                                                                                        	   
    {0, 0x102509,    0, BIT4}, // en  PD_ICLP_VDC                                                                                                                                                        	   
    {0, 0x10250a,    0, BIT2}, // en  PD_SOG_DAC                                                                                                                                                         	   
    {0, 0x10250a,    0, BIT3}, // en  PD_SOG_MAIN                                                                                                                                                        	   
    {0, 0x10257e,    0, BIT0}, // en  PD_SOG_MUX                                                                                                                                                         	   
    {0, 0x10250a,    0, BIT4}, // en  PD_SOGOFF_DAC                                                                                                                                                      	   
    {0, 0x10250a,    0, BIT5}, // en  PD_SOGOFF_MAIN                                                                                                                                                     	   
    {0, 0x10257e,    0, BIT1}, // en  PD_SOGOFF_MUX                                                                                                                                                      	   
    {0, 0x102508,    0, BIT2}, // en  PD_BG                                                                                                                                                              	   
    {0, 0x102508,    0, BIT3}, // en  PD_REF                                                                                                                                                             	   
    {0, 0x102508,    0, BIT0}, // en  PD_REF_RGB                                                                                                                                                         	   
    {0, 0x102508,    0, BIT1}, // en  PD_REF_VD                                                                                                                                                          	   
    {0, 0x102508,    0, BIT4}, // en  PD_REF_YUV                                                                                                                                                         	   
    {0, 0x102505, BIT0, BIT0}, // en  EN_MUX_VD_Y                                                                                                                                                        	   
    {0, 0x102505, BIT1, BIT1}, // en  EN_MUX_VD_C                                                                                                                                                        	   
    {0, 0x102570, BIT1, BIT1}, // en  EN_VDLPF_C                                                                                                                                                         	   
    {0, 0x102570, BIT0, BIT0}, // en  EN_VDLPF_Y                                                                                                                                                         	   
    {0, 0x102590, BIT3, BIT3}, // en  EN_IDAC1                                                                                                                                                           	   
    {0, 0x103d00, BIT4, BIT4}, // en  EN_IDAC_REF                                                                                                                                                        	   
    {0, 0x10250b,    0, BIT0}, // en  PD_HSYNC0_COMP                                                                                                                                                     	   
    {0, 0x10250b,    0, BIT1}, // en  PD_HSYNC1_COMP                                                                                                                                                     	   
    {0, 0x10250b,    0, BIT2}, // en  PD_HSYNC2_COMP                                                                                                                                                     	   
    {0, 0x102509,    0, BIT1}, // en  PD_ADCPLA_REG                                                                                                                                                      	   
    {0, 0x102509,    0, BIT0}, // en  PD_PGDIG_A                                                                                                                                                         	   
    {0, 0x102508,    0, BIT5}, // en  PD_ADCR                                                                                                                                                            	   
    {0, 0x102508,    0, BIT6}, // en  PD_ADCG                                                                                                                                                            	   
    {0, 0x102508,    0, BIT7}, // en  PD_ADCB                                                                                                                                                            	   
    {0, 0x102509,    0, BIT6}, // en  PD_ADCY                                                                                                                                                            	   
    {0, 0x102580,    0, BIT6}, // en  PD_FBLANK                                                                                                                                                          	   
    {0, 0x1025bc,    0, BIT6}, // en  PD_ADCA_PHDAC_R                                                                                                                                                    	   
    {0, 0x1025bc,    0, BIT7}, // en  PD_ADCA_PHDAC_G                                                                                                                                                    	   
    {0, 0x1025bd,    0, BIT0}, // en  PD_ADCA_PHDAC_B                                                                                                                                                    	   
    {0, 0x1025bd,    0, BIT1}, // en  PD_ADCA_PHDAC_Y                                                                                                                                                    	   
    {0, 0x1025b4,    0, BIT5}, // en  PD_LDO11                                                                                                                                                           	   
    {0, 0x1025b4,    0, BIT6}, // en  PD_LDO25_ADCA                                                                                                                                                      	   
    {0, 0x1025b4,    0, BIT7}, // en  PD_LDO25_ADCB                                                                                                                                                      	   
    {0, 0x10250b,    0, BIT3}, // en  PD_PHIDG_D                                                                                                                                                         	   
    {0, 0x10250c, 0x00, 0xff}, // en  clock setup                                                                                                                                                        	   
    {0, 0x10250d, 0x00, 0xff}, // en  clock setup                                                                                                                                                        	   
    {0, 0x102590, BIT3, BIT3}, // en  CVBSO                                                                                                                                                              	   
    {0, 0x1025a0,    0, BIT0}, // en  CVBSO 

    #if 0//(### REVIEW ###)
    //Close  clock for all input source
    {0, 0x100b02, BIT4, BIT4},
    {0, 0x000f60, BIT0, BIT0},
    {0, 0x000f60, BIT1, BIT1},
    {0, 0x100b29, BIT0, BIT0},
    {0, 0x100b34, BIT0, BIT0},
    {0, 0x100b4a, BIT0, BIT0},
    {0, 0x100b48, BIT0, BIT0},
    {0, 0x100b49, BIT0, BIT0},
    {0, 0x100b35, BIT0, BIT0},
    {0, 0x100b32, BIT0, BIT0}, //clk_aesdma  , while secure-R2 use , must be open clock
    {0, 0x100be0, BIT0, BIT0}, //clk_sec_r2 , while secure-R2 use , must be open clock
    {0, 0x100b2e, BIT0, BIT0},
    {0, 0x100b30, BIT0, BIT0},	
    #endif

    {0,0,0,0} //End
};

//#################################
// Step 4. System Clock Initialization
//#################################
static const BootConf InitialSystemClk[] =
{
   ////////////////////////////////////////////              			   
   // MPLL Enable                                            			   
   ////////////////////////////////////////////              			   
    {0, 0x101e38, 0, BIT0},  // DMD PD_3P31                                         			   
    {0, 0x101e38, 0, BIT1},  // DMD PD_3P32                                         			   
    {0, 0x110c02, 0, BIT0},  // reg_mpll_in_select                                  			   
    {0, 0x110c03, 0, BIT0},  // reg_mpll_pd                                         			   
    {0, 0x110c03, 0, BIT1},  // reg_mpll_clk_adc216m_pd                             			   
    {0, 0x110c03, 0, BIT2},  // reg_mpll_clk_adc432m_pd   


   ////////////////////////////////////////////              			   
   // MPLL_LVDS Enable                                       			   
   ////////////////////////////////////////////              			   
   //===== Setting LPLL ======//                                			   
    {0, 0x10311e, 0xd8, 0xff},                                                                                                                                                                                       	   
    {0, 0x10311f, 0x54, 0xff},                                                                                                                                                                                       	   
    {0, 0x103120, 0x1a, 0xff},                                                                                                                                                                                       	   
    {0, 0x103121, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10312a, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10312b, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103102, 0x03, 0xff},                                                                                                                                                                                       	   
    {0, 0x103103, 0x02, 0xff},                                                                                                                                                                                       	   
    {0, 0x103104, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103105, 0x17, 0xff},                                                                                                                                                                                       	   
    {0, 0x103106, 0x0e, 0xff},                                                                                                                                                                                       	   
    {0, 0x103107, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103108, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103109, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10315c, 0xb0, 0xff},                                                                                                                                                                                       	   
    {0, 0x10315d, 0x07, 0xff},                                                                                                                                                                                       	   
    {0, 0x103160, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103161, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103162, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103163, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103164, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103165, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103166, 0x21, 0xff},                                                                                                                                                                                       	   
    {0, 0x103167, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103168, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103169, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10316a, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10316b, 0x03, 0xff},                                                                                                                                                                                       	   
    {0, 0x10316c, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10316d, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10316e, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x10316f, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x103172, 0x10, 0xff},                                                                                                                                                                                       	   
    {0, 0x103173, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c0, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c1, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c2, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c3, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c4, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c5, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c6, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c7, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c8, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031c9, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031ca, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031cb, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031cc, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031cd, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031e2, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1031e3, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1032f0, 0x00, 0xff},                                                                                                                                                                                       	   
    {0, 0x1032f1, 0x03, 0xff},                                                                                                                                                                                       	   
    {0, 0x103106, 0x0e, 0xff},                                                                                                                                                                                       	   
    {0, 0x103107, 0x00, 0xff},

   ////////////////////////////////////////////              			   
   // AUR2PLL Enable                                         			   
   ////////////////////////////////////////////              			   
    {0, 0x160343, 0, BIT0},    // en  AUR2PLL_PD                                      			   
    {0, 0x160342, 0x40, 0xff}, // en  MPLL_TEST                                       			   
    {0, 0x160343, 0x00, 0xff}, // en  MPLL_TEST                                       			   
    {0, 0x160342, 0x00, 0xff}, // en  AUR2PLL_PD[8] ; AUR2PLL_IN_SEL[0]               			   
    {0, 0x160343, 0x00, 0xff}, // en  AUR2PLL_PD[8] ; AUR2PLL_IN_SEL[0]               			   
    {0, 0x160344, 0x20, 0xff}, // AUR2PLL_INPUT_DIV_FIRST [5:4]                       			   
    {0, 0x160345, 0x01, 0xff}, // AUR2PLL_LOOP_DIV_FIRST[9:8];                        			   
    {0, 0x160346, 0x36, 0xff}, //  AUR2PLL_LOOP_DIV_SECOND [7:0]                      			   
    {0, 0x160347, 0x20, 0xff}, // MIPSPLL_OUTPUT_DIV_FIRST[13:12]; AUR2PLL_OUTPUT_DIV[10:8];                                   			   
    {0, 0x160348, 0x01, 0xff}, // MPLL_ICP_ICTRL [6:4];MPLL_TEST_EN[0]                			   
    {0, 0x160349, 0x84, 0xff}, // AUR2PLL_PRDT_EN[15]; AUR2PLL_XTAL_SEL[10]; MPLL_ENFRUN[8];                                   			   
    {0, 0x16034a, 0x1f, 0xff}, // reg_en_occclk2[4]; reg_en_occclk12[3];  reg_en_occclk10[2];  reg_en_mipspll_vco_div5[1] reg_en_cpuclk[0]     


   ////////////////////////////////////////////              			   
   // AU_PLL Enable                                          			   
   ////////////////////////////////////////////              			   
    {0, 0x112ca2, 0x18, 0xff},  // 2nd order synthesizer [4    ]                        			   
    {0, 0x112c9e, 0x20, 0xff},  // N.F [23:8]                                           			   
    {0, 0x112c9f, 0x1c, 0xff},  // N.F [23:8]                                           			   
    {0, 0x112c9c, 0x00, 0xff},  // 2nd order fix synthesizer                            			   
    {0, 0x112c9d, 0xc0, 0xff},  // 2nd order fix synthesizer                            			   
    {0, 0x112c70, 0x1a, 0xff},  // AUPLL_FBDIV[7:2] ; AUPLL_KP0[1:0]                    			   
    {0, 0x112c71, 0x00, 0xff},  // AUPLL_DDIV[13:12] ;AUPLL_PRELPDIV[9:8]               			   
    {0, 0x112c72, 0x00, 0xff},  // AUPLL_ICP_ICTRL[2]  ;AUPLL_ENDISC AUPLL_ENFRUN[1] ;AUPLL_RES_SEL[0]                           			   
    {0, 0x112c73, 0xaf, 0xff},  // AUPLL_KP1[15:13]; AUPL L_KPDIV AUPLL_IBIAS_ICTRL[12:]                                         			   
    {0, 0x112c74, 0xa8, 0xff},  // AUPLL_TEST                                           			   
    {0, 0x112c75, 0x00, 0xff},  // AUPLL_TEST                                           			   
    {0, 0x112c76, 0x00, 0xff},  // AUPLL_PD[4];  AUPLL_PD_KP0[3];   AUPLL_PD_KP1[2];   AUPLL_PD_KPDIV[1] ;AUPLL_PD_KF0[0]                                                 			   


////////////////////////////////////////////////////                                                  			   
////////////////////////////////////////////////////                                                  			   
// EN CLK                                                    			   
////////////////////////////////////////////////////                                                  			   
//////////////////////////////////////////////////// 
    #if 0//(### REVIEW ###)                                       			   
    {0, 0x100b00, 0x04, 0xff},                                      			   
    {0, 0x100b01, 0x00, 0xff},                                      			   
    //{0, 0x100b3e, 0x10, 0xff},   //240M , 128bits MIU bus
    {0, 0x100b3a, 0x08, 0xff},  //150M, 256bits MIU bus for sc
    #endif 

    {0, 0x100b3e, 0x00, 0xff},  //288M
    {0, 0x100b3f, 0x00, 0xff},                                      			   
			   
			  
   ////////////////////////////////////////////              			   
   // Enable CLK (Every CLK)                                 			   
   ////////////////////////////////////////////              			   
    {0, 0x100b2e, 0, BIT0}, // en  clk_spi_m                                                                                                                                                          	   
    {0, 0x100b54, 0, BIT0}, // en  clk_tsp                                                                                                                                                            	   
    {0, 0x100b50, 0, BIT0}, // en  clk_ts0                                                                                                                                                            	   
    {0, 0x100b51, 0, BIT0}, // en  clk_ts1                                                                                                                                                            	   
    {0, 0x103332, 0, BIT0}, // en  clk_aesdma                                                                                                                                                         	   
    {0, 0x100b55, 0, BIT0}, // en  clk_stc0                                                                                                                                                           	   
    {0, 0x100b72, 0, BIT0}, // en  clk_mvd                                                                                                                                                            	   
    {0, 0x103366, 0, BIT0}, // en  clk_parser                                                                                                                                                         	   
    {0, 0x100b74, 0, BIT0}, // en  clk_mvd_chroma                                                                                                                                                     	   
    {0, 0x100b75, 0, BIT0}, // en  clk_mvd_luma_a                                                                                                                                                     	   
    {0, 0x100b76, 0, BIT0}, // en  clk_mvd_luma_b                                                                                                                                                     	   
    {0, 0x100b77, 0, BIT0}, // en  clk_mvd_luma_c                                                                                                                                                     	   
    {0, 0x100b78, 0, BIT0}, // en  clk_mvd_rmem                                                                                                                                                       	   
    {0, 0x100b79, 0, BIT0}, // en  clk_mvd_rmem1                                                                                                                                                      	   
    {0, 0x100b7c, 0, BIT0}, // en  clk_mvd_rrefdat                                                                                                                                                    	   
    {0, 0x100b98, 0, BIT0}, // en  clk_dc0(MVOP)                                                                                                                                                      	   
    //{0, 0x100b2e, 0, BIT0}, // en  clk_vd                                                                                                                                                             	   
    {0, 0x100b41, 0, BIT0}, // en  clk_vd                                                                                                                                                             	   
    //{0, 0x100b2e, 0, BIT0}, // en  clk_vd2x                                                                                                                                                           	   
    {0, 0x100b46, 0, BIT0}, // en  clk_vd2x                                                                                                                                                           	   
    //{0, 0x100b2e, 0, BIT0}, // en  clk_vd_32fsc
    {0, 0x100b47, 0, BIT0}, // en  clk_vd_32fsc
    {0, 0x100b49, 0, BIT0}, // en  clk_vedac(CVBS_DAC)                                                                                                                                                	   
    {0, 0x100b4c, 0, BIT0}, // en  clk_daca2(By application)                                                                                                                                          	   
    {0, 0x100b4d, 0, BIT0}, // en  clk_dacb2(By application)                                                                                                                                          	   
    {0, 0x100b6a, 0, BIT0}, // en  clk_jpd                                                                                                                                                            	   
    {0, 0x100b62, 0, BIT0}, // en  clk_hvd                                                                                                                                                            	   
    {0, 0x100b69, 0, BIT0}, // en  clk_evd                                                                                                                                                            	   
    {0, 0x100b67, 0, BIT0}, // en  clk_evd_ppu                                                                                                                                                        	   
    {0, 0x100b60, 0, BIT0}, // en  clk_vd_mheg5                                                                                                                                                       	   
    {0, 0x103308, 0, BIT0}, // en  clk_atsc_ts                                                                                                                                                        	   
    {0, 0x103309, 0, BIT0}, // en  clk_dvbtc_ts                                                                                                                                                       	   
    {0, 0x103314, 0, BIT0}, // en  clk_dvbtc_adc                                                                                                                                                      	   
    {0, 0x100b1a, 0, BIT0}, // en  clk_vifdbb_dac                                                                                                                                                     	   
    {0, 0x100b1b, 0, BIT0}, // en  clk_vifdbb_vdac                                                                                                                                                    	   
    {0, 0x100b21, 0, BIT0}, // en  clk_mcu                                                                                                                                                            	   
    {0, 0x100b27, 0, BIT0}, // en  clk_uart0                                                                                                                                                          	   
    {0, 0x100b28, 0, BIT0}, // en  clk_uart1                                                                                                                                                          	   
    {0, 0x100b3e, 0, BIT0}, // en  clk_miu                                                                                                                                                            	   
    {0, 0x100b3e, 0, BIT0}, // en  clk_miu_rec                                                                                                                                                        	   
    {0, 0x100b3a, 0, BIT0}, // en  clk_miu_256                                                                                                                                                        	   
    {0, 0x100b90, 0, BIT0}, // en  clk_ge                                                                                                                                                             	   
    {0, 0x100b64, 0, BIT0}, // en  clk_idclk_lpll                                                                                                                                                     	   
    {0, 0x100ba8, 0, BIT0}, // en  clk_idclk0                                                                                                                                                         	   
    {0, 0x100ba9, 0, BIT0}, // en  clk_idclk1                                                                                                                                                         	   
    {0, 0x100baa, 0, BIT0}, // en  clk_idclk2                                                                                                                                                         	   
    {0, 0x100ba8, 0, BIT0}, // en  clk_sidclk0                                                                                                                                                        	   
    {0, 0x100baa, 0, BIT0}, // en  clk_sidclk2                                                                                                                                                        	   
    {0, 0x100ba5, 0, BIT0}, // en  clk_fclk                                                                                                                                                           	   
    {0, 0x100ba2, 0, BIT0}, // en  clk_ficlk_f1                                                                                                                                                       	   
    {0, 0x100ba3, 0, BIT0}, // en  clk_ficlk_f2                                                                                                                                                       	   
    {0, 0x100bae, 0, BIT0}, // en  clk_fifo_mini                                                                                                                                                      	   
    {0, 0x100baf, 0, BIT0}, // en  clk_test_mod                                                                                                                                                       	   
    {0, 0x100ba6, 0, BIT0}, // en  clk_odclk                                                                                                                                                          	   
    //{0, 0x100b42, 0, BIT0}, // en  clk_vdmcu                                                                                                                                                          	   
    {0, 0x100b42, 0x10, 0xff}, // en  clk_vdmcu                                                                                                                                                          	   
    {0, 0x10331e, 0, BIT0}, // en  clk_dmdmcu                                                                                                                                                         	   
    //{0, 0x100b2e, 0, BIT0}, // en  clk_mcu_mail0                                                                                                                                                      	   
    {0, 0x100b44, 0x0c, 0xff}, // en  clk_mcu_mail0                                                                                                                                                      	   
    //{0, 0x100b2e, 0, BIT0}, // en  clk_mcu_mail1                                                                                                                                                      	   
    {0, 0x100b45, 0x0c, 0xff}, // en  clk_mcu_mail1                                                                                                                                                      	   
    {0, 0x100b2c, 0, BIT0}, // en  clk_spi                                                                                                                                                            	   
    {0, 0x100b34, 0, BIT0}, // en  clk_pcm                                                                                                                                                            	   
    {0, 0x100b35, 0, BIT0}, // en  clk_tck                                                                                                                                                            	   
    {0, 0x100b24, 0, BIT0}, // en  clk_aeon                                                                                                                                                           	   
    {0, 0x100b25, 0, BIT0}, // en  clk_mps_aeon                                                                                                                                                       	   
    {0, 0x103360, 0, BIT0}, // en  clk_miic                                                                                                                                                           	   
    {0, 0x103362, 0, BIT0}, // en  clk_clko_1x_4digital                                                                                                                                               	   
    {0, 0x100ba3, 0, BIT4}, // en  clk_ficlk2_f2                                                                                                                                                      	   
    {0, 0x100b5a, 0, BIT0}, // en  clk_stamp                                                                                                                                                          	   
    {0, 0x100b30, 0, BIT0}, // en  clk_spi_m1                                                                                                                                                         	   
    {0, 0x100b2d, 0, BIT0}, // en  clk_mcu_au                                                                                                                                                         	   
    {0, 0x100be0, 0, BIT0}, // en  clk_r2_secure                                                                                                                                                      	   
    {0, 0x10335c, 0, BIT0}, // en  clk_xtal_0                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT1}, // en  clk_xtal_1                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT2}, // en  clk_xtal_2                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT3}, // en  clk_xtal_3                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT4}, // en  clk_xtal_4                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT5}, // en  clk_xtal_5                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT6}, // en  clk_xtal_6                                                                                                                                                         	   
    {0, 0x10335c, 0, BIT7}, // en  clk_xtal_7                                                                                                                                                         	   
    {0, 0x000f60, 0, BIT0}, // en  clk_xtal_pm0 (dbus)                                                                                                                                                	   
    {0, 0x000f60, 0, BIT1}, // en  clk_xtal_pm1 (sbus)                                                                                                                                                	   
    {0, 0x000f60, 0, BIT2}, // en  clk_xtal_pm2                                                                                                                                                       	   
    {0, 0x000f60, 0, BIT3}, // en  clk_xtal_pm3                                                                                                                                                       	   
    {0, 0x100b02, 0, BIT4}, // en  clk_bist_patgen                                     			   

    {0,0,0,0} //End
};

static const BootConf InitialMIU_DDR3[] =
{            			   
   ////////////////////////////////////////////              			   
   // Enable MIU                                             			   
   ////////////////////////////////////////////              			   
   {0, 0x101220, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101221, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x101246, 0xfe, 0xff},                                                                                                                                                                                         
   {0, 0x101247, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101266, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101267, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101286, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101287, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x1012a6, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x1012a7, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x110d78, 0x02, 0xff},                                                                                                                                                                                         
   {0, 0x110d79, 0x01, 0xff},
//-----------------------
//set DDRPLL0 1600MHz
//-----------------------
   {0, 0x110d36, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d37, 0x40, 0xff},                                                                                                                                                                                         
   {0, 0x110d34, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d35, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d30, 0x1e, 0xff},                                                                                                                                                                                         
   {0, 0x110d31, 0x93, 0xff},                                                                                                                                                                                         
   {0, 0x110d32, 0x22, 0xff},                                                                                                                                                                                         
   {0, 0x110d33, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d20, 0x20, 0xff},                                                                                                                                                                                         
   {0, 0x110d21, 0x00, 0xff},
//------------------------
//set DDR3-16-8X-CL11_1600
//------------------------
   {0, 0x101202, 0xa3, 0xff},                                                                                                                                                                                         
   {0, 0x101203, 0x03, 0xff},                                                                                                                                                                                         
   {0, 0x101204, 0x0b, 0xff},                                                                                                                                                                                         
   {0, 0x101205, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101206, 0x30, 0xff},                                                                                                                                                                                         
   {0, 0x101207, 0x14, 0xff},                                                                                                                                                                                         
   {0, 0x101208, 0xbb, 0xff},                                                                                                                                                                                         
   {0, 0x101209, 0x1c, 0xff},                                                                                                                                                                                         
   {0, 0x10120a, 0xb6, 0xff},                                                                                                                                                                                         
   {0, 0x10120b, 0x27, 0xff},                                                                                                                                                                                         
   {0, 0x10120c, 0xc8, 0xff},                                                                                                                                                                                         
   {0, 0x10120d, 0xc6, 0xff},                                                                                                                                                                                         
   {0, 0x10120e, 0x60, 0xff},                                                                                                                                                                                         
   {0, 0x10120f, 0x40, 0xff},                                                                                                                                                                                         
   {0, 0x101210, 0x70, 0xff},                                                                                                                                                                                         
   {0, 0x101211, 0x1d, 0xff},                                                                                                                                                                                         
   {0, 0x101212, 0x04, 0xff},                                                                                                                                                                                         
   {0, 0x101213, 0x40, 0xff},                                                                                                                                                                                         
   {0, 0x101214, 0x18, 0xff},                                                                                                                                                                                         
   {0, 0x101215, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x101216, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101217, 0xc0, 0xff},                                                                                                                                                                                         
   {0, 0x110d54, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d55, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d00, 0x18, 0xff},                                                                                                                                                                                         
   {0, 0x110d01, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d02, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d03, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d04, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x110d05, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d38, 0x44, 0xff},                                                                                                                                                                                         
   {0, 0x110d39, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d3a, 0x53, 0xff},                                                                                                                                                                                         
   {0, 0x110d3b, 0x50, 0xff},                                                                                                                                                                                         
   {0, 0x110d3c, 0x33, 0xff},                                                                                                                                                                                         
   {0, 0x110d3d, 0x11, 0xff},                                                                                                                                                                                         
   {0, 0x110d4e, 0x77, 0xff},                                                                                                                                                                                         
   {0, 0x110d4f, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d3e, 0x11, 0xff},                                                                                                                                                                                         
   {0, 0x110d3f, 0x10, 0xff},                                                                                                                                                                                         
   {0, 0x110d0a, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0b, 0x70, 0xff},                                                                                                                                                                                         
   {0, 0x110d50, 0x11, 0xff},                                                                                                                                                                                         
   {0, 0x110d51, 0x11, 0xff},                                                                                                                                                                                         
   {0, 0x110d52, 0x22, 0xff},                                                                                                                                                                                         
   {0, 0x110d53, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0e, 0xa7, 0xff},                                                                                                                                                                                         
   {0, 0x110d0f, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0e, 0xe7, 0xff},                                                                                                                                                                                         
   {0, 0x110d0f, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0e, 0xa7, 0xff},                                                                                                                                                                                         
   {0, 0x110d0f, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d78, 0x02, 0xff},                                                                                                                                                                                         
   {0, 0x110d79, 0x01, 0xff},                                                                                                                                                                                         
   //{0, 0x110d7a, 0x00, 0xff},                                                                                                                                                                                         
   //{0, 0x110d7b, 0x38, 0xff},                                                                                                                                                                                         
   {0, 0x110d6e, 0x33, 0xff},                                                                                                                                                                                         
   {0, 0x110d6f, 0x44, 0xff},                                                                                                                                                                                         
   {0, 0x110d6a, 0x24, 0xff},                                                                                                                                                                                         
   {0, 0x110d6b, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x110d68, 0x24, 0xff},                                                                                                                                                                                         
   {0, 0x110d69, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x110d54, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d55, 0xc0, 0xff},                                                                                                                                                                                         
   {0, 0x110d7c, 0x0f, 0xff},                                                                                                                                                                                         
   {0, 0x110d7d, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x1a, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x1a, 0xff},
   {2, 1000, 0, 0}, //delay 1ms
   {0, 0x110d00, 0x18, 0xff},                                                                                                                                                                                         
   {0, 0x110d01, 0x00, 0xff},                                                                                                                                                                                         
   {2, 1000, 0, 0}, //delay 1ms
   {0, 0x110d00, 0x08, 0xff},                                                                                                                                                                                         
   {0, 0x110d01, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d00, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d01, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d02, 0xaa, 0xff},                                                                                                                                                                                         
   {0, 0x110d03, 0xaa, 0xff},                                                                                                                                                                                         
   {0, 0x110d7e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d7f, 0x10, 0xff},                                                                                                                                                                                         
   {0, 0x110d00, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d01, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d80, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d81, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d08, 0x3f, 0xff},                                                                                                                                                                                         
   {0, 0x110d09, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d88, 0x3f, 0xff},                                                                                                                                                                                         
   {0, 0x110d89, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0e, 0xa7, 0xff},                                                                                                                                                                                         
   {0, 0x110d0f, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0e, 0xe7, 0xff},                                                                                                                                                                                         
   {0, 0x110d0f, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d0e, 0xa7, 0xff},                                                                                                                                                                                         
   {0, 0x110d0f, 0x00, 0xff},                                                                                                                                                                                         

    {0, 0x110d58, 0x04, 0xff}, 
    {0, 0x110d59, 0x04, 0xff},
    {0, 0x110d5a, 0x44, 0xff}, 
    {0, 0x110d5b, 0x44, 0xff},
    {0, 0x110d5c, 0x44, 0xff}, 
    {0, 0x110d5d, 0x44, 0xff},
    {0, 0x110d5e, 0x44, 0xff}, 
    {0, 0x110d5f, 0x44, 0xff},

    {0, 0x110d60, 0x0c, 0xff}, 
    {0, 0x110d61, 0x00, 0xff}, //dll_pd
    {0, 0x110d60, 0x08, 0xff}, 
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d60, 0x70, 0xff}, 
    {0, 0x110d61, 0x23, 0xff},
                                                                                                                                                                                        
   {0, 0x101200, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101201, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101200, 0x08, 0xff},                                                                                                                                                                                         
   {0, 0x101201, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101200, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x101201, 0x00, 0xff},
   {2, 1000, 0, 0}, //delay 1ms
   {0, 0x101200, 0x0e, 0xff},                                                                                                                                                                                         
   {0, 0x101201, 0x00, 0xff},
   {2, 1000, 0, 0}, //delay 1ms
   {0, 0x101200, 0x1f, 0xff},                                                                                                                                                                                         
   {0, 0x101201, 0x00, 0xff},
   {1, 0x101201, BIT7, BIT7}, //check reg_101200 #15 for wait initial done                         			   
   {0, 0x101240, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x101241, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101260, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x101261, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101280, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x101281, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x1012a0, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x1012a1, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101246, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101247, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101266, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101267, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101286, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101287, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x1012a6, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x1012a7, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101240, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x101241, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x101260, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x101261, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x101280, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x101281, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x1012a0, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x1012a1, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x08, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x1a, 0xff},                                                                                                                                                                                         
   {0, 0x1012d2, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x1012d3, 0x00, 0xff}, //dram size decided by next procedure                                                                                                                                                                                          
   {0, 0x1012fe, 0x87, 0xff},                                                                                                                                                                                         
   {0, 0x1012ff, 0x00, 0xff}, 
   {0, 0x1012f0, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x1012f1, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101252, 0xff, 0xff}, 
   {0, 0x101253, 0xff, 0xff},
   {0, 0x101272, 0xff, 0xff}, 
   {0, 0x101273, 0xff, 0xff},
   {0, 0x101292, 0xff, 0xff}, 
   {0, 0x101293, 0xff, 0xff},
   {0, 0x1012b2, 0xff, 0xff}, 
   {0, 0x1012b3, 0xff, 0xff},

   {0,0,0,0} //End
};

static const BootConf InitialMIU_DDR2_0[] =
{            			   
   ////////////////////////////////////////////              			   
   // Enable MIU                                             			   
   ////////////////////////////////////////////              			   
   {0, 0x101220, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101221, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x101246, 0xfe, 0xff},                                                                                                                                                                                         
   {0, 0x101247, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101266, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101267, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101286, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101287, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x1012a6, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x1012a7, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x110d78, 0x02, 0xff},                                                                                                                                                                                         
   {0, 0x110d79, 0x01, 0xff},
//-----------------------
//set DDRPLL0 Speed
//-----------------------
   {0, 0x110d36, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d37, 0x40, 0xff},                                                                                                                                                                                         
   {0, 0x110d34, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d35, 0x00, 0xff}, 
   #if (MIU_SPEED==DDR2_1066MHz)
   {0, 0x110d30, 0x56, 0xff},                                                                                                                                                                                         
   {0, 0x110d31, 0xdf, 0xff},                                                                                                                                                                                         
   {0, 0x110d32, 0x33, 0xff},                                                                                                                                                                                         
   {0, 0x110d33, 0x00, 0xff},                                                                                                                                                                                         
   #elif (MIU_SPEED==DDR2_1333MHz)
   {0, 0x110d30, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d31, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x110d32, 0x29, 0xff},                                                                                                                                                                                         
   {0, 0x110d33, 0x00, 0xff},                                                                                                                                                                                         
   #else
    #error "Please select correct DDR2 Speed"
   #endif
   {0, 0x110d20, 0x20, 0xff},                                                                                                                                                                                         
   {0, 0x110d21, 0x00, 0xff},
//------------------------
//set DDRPLL0 Speed
//------------------------
    //{0, 0x101202, 0x92, 0xff}, //move this DDR2 column setting before this table
    {0, 0x101203, 0x03, 0xff},
    {0, 0x101204, 0x0a, 0xff},
    {0, 0x101205, 0x00, 0xff},
    {0, 0x101206, 0x30, 0xff},
    {0, 0x101207, 0x1b, 0xff},
    {0, 0x101208, 0x99, 0xff}, 
    {0, 0x101209, 0x1e, 0xff},
    {0, 0x10120a, 0x77, 0xff}, 
    {0, 0x10120b, 0x27, 0xff},
    {0, 0x10120c, 0x98, 0xff}, 
    {0, 0x10120d, 0x95, 0xff},
    {0, 0x10120e, 0x46, 0xff}, 
    {0, 0x10120f, 0x20, 0xff},
    {0, 0x101210, 0x03, 0xff}, 
    {0, 0x101211, 0x00, 0xff},
    {0, 0x101212, 0x04, 0xff}, 
    {0, 0x101213, 0x40, 0xff},
    {0, 0x101214, 0x00, 0xff}, 
    {0, 0x101215, 0x80, 0xff},
    {0, 0x101216, 0x00, 0xff}, 
    {0, 0x101217, 0xc0, 0xff},
    {0, 0x101228, 0x20, 0xff},
    {0, 0x101229, 0x00, 0xff},
    {0, 0x110d02, 0xaa, 0xff}, 
    {0, 0x110d03, 0xaa, 0xff},
    {0, 0x110d04, 0x80, 0xff}, 
    {0, 0x110d05, 0x00, 0xff},
    {0, 0x110d3c, 0x22, 0xff}, 
    {0, 0x110d3d, 0x00, 0xff},
    {0, 0x110d3e, 0x00, 0xff}, 
    {0, 0x110d3f, 0x00, 0xff},
    {0, 0x110d38, 0x77, 0xff},//@@00
    {0, 0x110d39, 0x00, 0xff},//@@00
    {0, 0x110d3a, 0x03, 0xff},//@@00
    {0, 0x110d3b, 0x00, 0xff},//@@00
    {0, 0x110d0a, 0x44, 0xff}, 
    {0, 0x110d0b, 0xa2, 0xff},
    {0, 0x110d50, 0x00, 0xff},
    {0, 0x110d51, 0x00, 0xff},
    {0, 0x110d52, 0x44, 0xff}, 
    {0, 0x110d53, 0x00, 0xff},
    {0, 0x110d2e, 0x00, 0xff}, 
    {0, 0x110d2f, 0x22, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xe5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d68, 0x20, 0xff}, 
    {0, 0x110d69, 0x20, 0xff},
    {0, 0x110d6a, 0x20, 0xff}, 
    {0, 0x110d6b, 0x20, 0xff},
    {0, 0x110d6e, 0x55, 0xff},//@@00
    {0, 0x110d6f, 0x55, 0xff},//@@00
    {0, 0x110d6c, 0x00, 0xff}, 
    {0, 0x110d6d, 0x00, 0xff},
    {0, 0x110d18, 0x00, 0xff}, 
    {0, 0x110d19, 0x00, 0xff},
    {0, 0x110d00, 0x08, 0xff}, 
    {0, 0x110d01, 0x20, 0xff},
    {0, 0x110d54, 0x00, 0xff}, 
    {0, 0x110d55, 0x00, 0xff},
    {0, 0x110d4e, 0x22, 0xff},//@@00 Only
    {0, 0x110d4f, 0x00, 0xff},//@@00 Only
    {0, 0x10121e, 0x01, 0xff}, 
    {0, 0x10121f, 0x0c, 0xff},
    {0, 0x10121e, 0x00, 0xff}, 
    {0, 0x10121f, 0x0c, 0xff},
    {0, 0x110d00, 0x00, 0xff}, 
    {0, 0x110d01, 0x00, 0xff},
    {0, 0x110d7c, 0x00, 0xff}, 
    {0, 0x110d7d, 0xfc, 0xff},
    {0, 0x110d60, 0x0a, 0xff}, 
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d00, 0x01, 0xff}, 
    {0, 0x110d01, 0x00, 0xff},
    {0, 0x110d08, 0x3f, 0xff}, 
    {0, 0x110d09, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xe5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d58, 0x04, 0xff}, 
    {0, 0x110d59, 0x04, 0xff},
    {0, 0x110d5a, 0x44, 0xff}, 
    {0, 0x110d5b, 0x44, 0xff},
    {0, 0x110d5c, 0x44, 0xff}, 
    {0, 0x110d5d, 0x44, 0xff},
    {0, 0x110d5e, 0x44, 0xff}, 
    {0, 0x110d5f, 0x44, 0xff},
    {0, 0x110d60, 0x0c, 0xff}, 
    {0, 0x110d61, 0x00, 0xff}, //dll_pd
    {0, 0x110d60, 0x08, 0xff}, 
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d60, 0x70, 0xff}, 
    {0, 0x110d61, 0x23, 0xff},
    {0, 0x101200, 0x00, 0xff}, 
    {0, 0x101201, 0x00, 0xff},
    {0, 0x101200, 0x08, 0xff}, 
    {0, 0x101201, 0x00, 0xff},
    {0, 0x101200, 0x0c, 0xff}, 
    {0, 0x101201, 0x00, 0xff},

    {2, 1000, 0, 0}, //delay 1ms
    {0, 0x101200, 0x0e, 0xff},                                                                                                                                                                                         
    {0, 0x101201, 0x00, 0xff},
    {2, 1000, 0, 0}, //delay 1ms
#if (MIU_SPEED==DDR2_1066MHz)
    {0, 0x101200, 0x0f, 0xff}, //@@turn off ODT for 1066MHz
#elif (MIU_SPEED==DDR2_1333MHz)
    {0, 0x101200, 0x1f, 0xff}, //@@turn on ODT for 1333MHz                                                                                                                                                                                         
#else
 #error "Please select correct DDR2 Speed"
#endif
    {0, 0x101201, 0x00, 0xff},
    {1, 0x101201, BIT7, BIT7}, //check reg_101200 #15 for wait initial done                         			   

    {0, 0x101240, 0x01, 0xff}, 
    {0, 0x101241, 0x00, 0xff},
    {0, 0x101260, 0x01, 0xff}, 
    {0, 0x101261, 0x00, 0xff},
    {0, 0x101280, 0x01, 0xff}, 
    {0, 0x101281, 0x00, 0xff},
    {0, 0x1012a0, 0x01, 0xff}, 
    {0, 0x1012a1, 0x00, 0xff},
    {0, 0x101246, 0x00, 0xff}, 
    {0, 0x101247, 0x00, 0xff},
    {0, 0x101266, 0x00, 0xff}, 
    {0, 0x101267, 0x00, 0xff},
    {0, 0x101286, 0x00, 0xff}, 
    {0, 0x101287, 0x00, 0xff},
    {0, 0x1012a6, 0x00, 0xff}, 
    {0, 0x1012a7, 0x00, 0xff},
    {0, 0x101240, 0x01, 0xff}, 
    {0, 0x101241, 0x80, 0xff},
    {0, 0x101260, 0x01, 0xff}, 
    {0, 0x101261, 0x80, 0xff},
    {0, 0x101280, 0x01, 0xff}, 
    {0, 0x101281, 0x80, 0xff},
    {0, 0x1012a0, 0x01, 0xff}, 
    {0, 0x1012a1, 0x80, 0xff},
    {0, 0x10121e, 0x08, 0xff}, 
    {0, 0x10121f, 0x1a, 0xff},
    {0, 0x1012d2, 0x00, 0xff}, 
    {0, 0x1012d3, 0x00, 0xff},
    {0, 0x1012fe, 0x87, 0xff}, 
    {0, 0x1012ff, 0x00, 0xff},
    {0, 0x1012e0, 0x00, 0xff}, 
    {0, 0x1012f0, 0x00, 0xff}, 
    {0, 0x1012f1, 0x00, 0xff},
    {0, 0x101252, 0xff, 0xff}, 
    {0, 0x101253, 0xff, 0xff},
    {0, 0x101272, 0xff, 0xff}, 
    {0, 0x101273, 0xff, 0xff},
    {0, 0x101292, 0xff, 0xff}, 
    {0, 0x101293, 0xff, 0xff},
    {0, 0x1012b2, 0xff, 0xff}, 
    {0, 0x1012b3, 0xff, 0xff},
   
    {0,0,0,0} //End
};

static const BootConf InitialMIU_DDR2_3[] =
{            			   
   ////////////////////////////////////////////              			   
   // Enable MIU                                             			   
   ////////////////////////////////////////////              			   
   {0, 0x101220, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x101221, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x01, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x10121e, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x10121f, 0x0c, 0xff},                                                                                                                                                                                         
   {0, 0x101246, 0xfe, 0xff},                                                                                                                                                                                         
   {0, 0x101247, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101266, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101267, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101286, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x101287, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x1012a6, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x1012a7, 0xff, 0xff},                                                                                                                                                                                         
   {0, 0x110d78, 0x02, 0xff},                                                                                                                                                                                         
   {0, 0x110d79, 0x01, 0xff},
//-----------------------
//set DDRPLL0 Speed
//-----------------------
   {0, 0x110d36, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d37, 0x40, 0xff},                                                                                                                                                                                         
   {0, 0x110d34, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d35, 0x00, 0xff}, 
   #if (MIU_SPEED==DDR2_1066MHz)
   {0, 0x110d30, 0x56, 0xff},                                                                                                                                                                                         
   {0, 0x110d31, 0xdf, 0xff},                                                                                                                                                                                         
   {0, 0x110d32, 0x33, 0xff},                                                                                                                                                                                         
   {0, 0x110d33, 0x00, 0xff},                                                                                                                                                                                         
   #elif (MIU_SPEED==DDR2_1333MHz)
   {0, 0x110d30, 0x00, 0xff},                                                                                                                                                                                         
   {0, 0x110d31, 0x80, 0xff},                                                                                                                                                                                         
   {0, 0x110d32, 0x29, 0xff},                                                                                                                                                                                         
   {0, 0x110d33, 0x00, 0xff},                                                                                                                                                                                         
   #else
    #error "Please select correct DDR2 Speed"
   #endif
   {0, 0x110d20, 0x20, 0xff},                                                                                                                                                                                         
   {0, 0x110d21, 0x00, 0xff},
//------------------------
//set DDRPLL0 Speed
//------------------------
    //{0, 0x101202, 0x92, 0xff}, //move this DDR2 column setting before this table
    {0, 0x101203, 0x03, 0xff},
    {0, 0x101204, 0x0a, 0xff},
    {0, 0x101205, 0x00, 0xff},
    {0, 0x101206, 0x30, 0xff},
    {0, 0x101207, 0x1b, 0xff},
    {0, 0x101208, 0x99, 0xff}, 
    {0, 0x101209, 0x1e, 0xff},
    {0, 0x10120a, 0x77, 0xff}, 
    {0, 0x10120b, 0x27, 0xff},
    {0, 0x10120c, 0x98, 0xff}, 
    {0, 0x10120d, 0x95, 0xff},
    {0, 0x10120e, 0x46, 0xff}, 
    {0, 0x10120f, 0x20, 0xff},
    {0, 0x101210, 0x03, 0xff}, 
    {0, 0x101211, 0x00, 0xff},
    {0, 0x101212, 0x04, 0xff}, 
    {0, 0x101213, 0x40, 0xff},
    {0, 0x101214, 0x00, 0xff}, 
    {0, 0x101215, 0x80, 0xff},
    {0, 0x101216, 0x00, 0xff}, 
    {0, 0x101217, 0xc0, 0xff},
    {0, 0x101228, 0x20, 0xff},
    {0, 0x101229, 0x00, 0xff},
    {0, 0x110d02, 0xaa, 0xff}, 
    {0, 0x110d03, 0xaa, 0xff},
    {0, 0x110d04, 0x80, 0xff}, 
    {0, 0x110d05, 0x00, 0xff},
    {0, 0x110d3c, 0x22, 0xff}, 
    {0, 0x110d3d, 0x00, 0xff},
    {0, 0x110d3e, 0x00, 0xff}, 
    {0, 0x110d3f, 0x00, 0xff},
    {0, 0x110d38, 0x54, 0xff},//@@11
    {0, 0x110d39, 0x00, 0xff},//@@11
    {0, 0x110d3a, 0x00, 0xff},//@@11
    {0, 0x110d3b, 0x00, 0xff},//@@11
    {0, 0x110d0a, 0x44, 0xff}, 
    {0, 0x110d0b, 0xa2, 0xff},
    {0, 0x110d50, 0x00, 0xff},
    {0, 0x110d51, 0x00, 0xff},
    {0, 0x110d52, 0x44, 0xff}, 
    {0, 0x110d53, 0x00, 0xff},
    {0, 0x110d2e, 0x00, 0xff}, 
    {0, 0x110d2f, 0x22, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xe5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d68, 0x20, 0xff}, 
    {0, 0x110d69, 0x20, 0xff},
    {0, 0x110d6a, 0x20, 0xff}, 
    {0, 0x110d6b, 0x20, 0xff},
    {0, 0x110d6e, 0x55, 0xff},//@@11
    {0, 0x110d6f, 0x55, 0xff},//@@11
    {0, 0x110d6c, 0x00, 0xff}, 
    {0, 0x110d6d, 0x00, 0xff},
    {0, 0x110d18, 0x00, 0xff}, 
    {0, 0x110d19, 0x00, 0xff},
    {0, 0x110d00, 0x08, 0xff}, 
    {0, 0x110d01, 0x20, 0xff},
    {0, 0x110d54, 0x00, 0xff}, 
    {0, 0x110d55, 0x00, 0xff},
    {0, 0x110d74, 0x22, 0xff},//@@11 Only
    {0, 0x110d75, 0x22, 0xff},//@@11 Only
#if (MS_BOARD_TYPE_SEL == BD_MST269G_D01A_S)
    {0, 0x110db8, 0x66, 0xff},//@@Jason-cs.Lee @20181214
    {0, 0x110db9, 0x00, 0xff},//@@Jason-cs.Lee @20181214
#endif
    {0, 0x10121e, 0x01, 0xff}, 
    {0, 0x10121f, 0x0c, 0xff},
    {0, 0x10121e, 0x00, 0xff}, 
    {0, 0x10121f, 0x0c, 0xff},
    {0, 0x110d00, 0x00, 0xff}, 
    {0, 0x110d01, 0x00, 0xff},
    {0, 0x110d7c, 0x00, 0xff}, 
    {0, 0x110d7d, 0xfc, 0xff},
    {0, 0x110d60, 0x0a, 0xff}, 
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d00, 0x01, 0xff}, 
    {0, 0x110d01, 0x00, 0xff},
    {0, 0x110d08, 0x3f, 0xff}, 
    {0, 0x110d09, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xe5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff}, 
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d58, 0x04, 0xff}, 
    {0, 0x110d59, 0x04, 0xff},
    {0, 0x110d5a, 0x44, 0xff}, 
    {0, 0x110d5b, 0x44, 0xff},
    {0, 0x110d5c, 0x44, 0xff}, 
    {0, 0x110d5d, 0x44, 0xff},
    {0, 0x110d5e, 0x44, 0xff}, 
    {0, 0x110d5f, 0x44, 0xff},
    {0, 0x110d60, 0x0c, 0xff}, 
    {0, 0x110d61, 0x00, 0xff}, //dll_pd
    {0, 0x110d60, 0x08, 0xff}, 
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d60, 0x70, 0xff}, 
    {0, 0x110d61, 0x23, 0xff},
    {0, 0x101200, 0x00, 0xff}, 
    {0, 0x101201, 0x00, 0xff},
    {0, 0x101200, 0x08, 0xff}, 
    {0, 0x101201, 0x00, 0xff},
    {0, 0x101200, 0x0c, 0xff}, 
    {0, 0x101201, 0x00, 0xff},

    {2, 1000, 0, 0}, //delay 1ms
    {0, 0x101200, 0x0e, 0xff},                                                                                                                                                                                         
    {0, 0x101201, 0x00, 0xff},
    {2, 1000, 0, 0}, //delay 1ms
#if (MIU_SPEED==DDR2_1066MHz)
    {0, 0x101200, 0x0f, 0xff}, //@@turn off ODT for 1066MHz
#elif (MIU_SPEED==DDR2_1333MHz)
    {0, 0x101200, 0x1f, 0xff}, //@@turn on ODT for 1333MHz                                                                                                                                                                                         
#else
 #error "Please select correct DDR2 Speed"
#endif
    {0, 0x101201, 0x00, 0xff},
    {1, 0x101201, BIT7, BIT7}, //check reg_101200 #15 for wait initial done                         			   

    {0, 0x101240, 0x01, 0xff}, 
    {0, 0x101241, 0x00, 0xff},
    {0, 0x101260, 0x01, 0xff}, 
    {0, 0x101261, 0x00, 0xff},
    {0, 0x101280, 0x01, 0xff}, 
    {0, 0x101281, 0x00, 0xff},
    {0, 0x1012a0, 0x01, 0xff}, 
    {0, 0x1012a1, 0x00, 0xff},
    {0, 0x101246, 0x00, 0xff}, 
    {0, 0x101247, 0x00, 0xff},
    {0, 0x101266, 0x00, 0xff}, 
    {0, 0x101267, 0x00, 0xff},
    {0, 0x101286, 0x00, 0xff}, 
    {0, 0x101287, 0x00, 0xff},
    {0, 0x1012a6, 0x00, 0xff}, 
    {0, 0x1012a7, 0x00, 0xff},
    {0, 0x101240, 0x01, 0xff}, 
    {0, 0x101241, 0x80, 0xff},
    {0, 0x101260, 0x01, 0xff}, 
    {0, 0x101261, 0x80, 0xff},
    {0, 0x101280, 0x01, 0xff}, 
    {0, 0x101281, 0x80, 0xff},
    {0, 0x1012a0, 0x01, 0xff}, 
    {0, 0x1012a1, 0x80, 0xff},
    {0, 0x10121e, 0x08, 0xff}, 
    {0, 0x10121f, 0x1a, 0xff},
    {0, 0x1012d2, 0x00, 0xff}, 
    {0, 0x1012d3, 0x00, 0xff},
    {0, 0x1012fe, 0x87, 0xff}, 
    {0, 0x1012ff, 0x00, 0xff},
    {0, 0x1012e0, 0x00, 0xff}, 
    {0, 0x1012f0, 0x00, 0xff}, 
    {0, 0x1012f1, 0x00, 0xff},
    {0, 0x101252, 0xff, 0xff}, 
    {0, 0x101253, 0xff, 0xff},
    {0, 0x101272, 0xff, 0xff}, 
    {0, 0x101273, 0xff, 0xff},
    {0, 0x101292, 0xff, 0xff}, 
    {0, 0x101293, 0xff, 0xff},
    {0, 0x1012b2, 0xff, 0xff}, 
    {0, 0x1012b3, 0xff, 0xff},

    {0,0,0,0} //End
};

static const BootConf InitialMIU_DDR2_1[] =
{
   ////////////////////////////////////////////
   // Enable MIU
   ////////////////////////////////////////////
   {0, 0x101220, 0x00, 0xff},
   {0, 0x101221, 0x00, 0xff},
   {0, 0x10121e, 0x00, 0xff},
   {0, 0x10121f, 0x0c, 0xff},
   {0, 0x10121e, 0x00, 0xff},
   {0, 0x10121f, 0x0c, 0xff},
   {0, 0x10121e, 0x00, 0xff},
   {0, 0x10121f, 0x0c, 0xff},
   {0, 0x10121e, 0x01, 0xff},
   {0, 0x10121f, 0x0c, 0xff},
   {0, 0x10121e, 0x00, 0xff},
   {0, 0x10121f, 0x0c, 0xff},
   {0, 0x101246, 0xfe, 0xff},
   {0, 0x101247, 0xff, 0xff},
   {0, 0x101266, 0xff, 0xff},
   {0, 0x101267, 0xff, 0xff},
   {0, 0x101286, 0xff, 0xff},
   {0, 0x101287, 0xff, 0xff},
   {0, 0x1012a6, 0xff, 0xff},
   {0, 0x1012a7, 0xff, 0xff},
   {0, 0x110d78, 0x02, 0xff},
   {0, 0x110d79, 0x01, 0xff},
//-----------------------
//set DDRPLL0 Speed
//-----------------------
   {0, 0x110d36, 0x00, 0xff},
   {0, 0x110d37, 0x40, 0xff},
   {0, 0x110d34, 0x00, 0xff},
   {0, 0x110d35, 0x00, 0xff},
   #if (MIU_SPEED==DDR2_1066MHz)
   {0, 0x110d30, 0x56, 0xff},
   {0, 0x110d31, 0xdf, 0xff},
   {0, 0x110d32, 0x33, 0xff},
   {0, 0x110d33, 0x00, 0xff},
   #elif (MIU_SPEED==DDR2_1333MHz)
   {0, 0x110d30, 0x00, 0xff},
   {0, 0x110d31, 0x80, 0xff},
   {0, 0x110d32, 0x29, 0xff},
   {0, 0x110d33, 0x00, 0xff},
   #else
    #error "Please select correct DDR2 Speed"
   #endif
   {0, 0x110d20, 0x20, 0xff},
   {0, 0x110d21, 0x00, 0xff},
//------------------------
//set DDRPLL0 Speed
//------------------------
    //{0, 0x101202, 0x92, 0xff},//move this DDR2 column setting before this table
    {0, 0x101203, 0x03, 0xff},
    {0, 0x101204, 0x0a, 0xff},
    {0, 0x101205, 0x00, 0xff},
    {0, 0x101206, 0x30, 0xff},
    {0, 0x101207, 0x1b, 0xff},
    {0, 0x101208, 0x99, 0xff},
    {0, 0x101209, 0x1e, 0xff},
    {0, 0x10120a, 0x77, 0xff},
    {0, 0x10120b, 0x27, 0xff},
    {0, 0x10120c, 0x98, 0xff},
    {0, 0x10120d, 0x95, 0xff},
    {0, 0x10120e, 0x46, 0xff},
    {0, 0x10120f, 0x20, 0xff},
    {0, 0x101210, 0x03, 0xff},
    {0, 0x101211, 0x00, 0xff},
    {0, 0x101212, 0x04, 0xff},
    {0, 0x101213, 0x40, 0xff},
    {0, 0x101214, 0x00, 0xff},
    {0, 0x101215, 0x80, 0xff},
    {0, 0x101216, 0x00, 0xff},
    {0, 0x101217, 0xc0, 0xff},
    {0, 0x101228, 0x20, 0xff},
    {0, 0x101229, 0x00, 0xff},
    {0, 0x110d02, 0xaa, 0xff},
    {0, 0x110d03, 0xaa, 0xff},
    {0, 0x110d04, 0x80, 0xff},
    {0, 0x110d05, 0x00, 0xff},
    {0, 0x110d3c, 0x22, 0xff},
    {0, 0x110d3d, 0x00, 0xff},
    {0, 0x110d3e, 0x00, 0xff},
    {0, 0x110d3f, 0x00, 0xff},
    {0, 0x110d38, 0x55, 0xff},//@@01
    {0, 0x110d39, 0x00, 0xff},//@@01
    {0, 0x110d3a, 0x30, 0xff},//@@01
    {0, 0x110d3b, 0x30, 0xff},//@@01
    {0, 0x110d0a, 0x44, 0xff},
    {0, 0x110d0b, 0xa2, 0xff},
    {0, 0x110d50, 0x00, 0xff},
    {0, 0x110d51, 0x00, 0xff},
    {0, 0x110d52, 0x44, 0xff},
    {0, 0x110d53, 0x00, 0xff},
    {0, 0x110d2e, 0x00, 0xff},
    {0, 0x110d2f, 0x22, 0xff},
    {0, 0x110d0e, 0xa5, 0xff},
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xe5, 0xff},
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff},
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d68, 0x20, 0xff},
    {0, 0x110d69, 0x20, 0xff},
    {0, 0x110d6a, 0x20, 0xff},
    {0, 0x110d6b, 0x20, 0xff},
    {0, 0x110d6e, 0x55, 0xff},//@@11
    {0, 0x110d6f, 0x55, 0xff},//@@11
    {0, 0x110d6c, 0x00, 0xff},
    {0, 0x110d6d, 0x00, 0xff},
    {0, 0x110d18, 0x00, 0xff},
    {0, 0x110d19, 0x00, 0xff},
    {0, 0x110d00, 0x08, 0xff},
    {0, 0x110d01, 0x20, 0xff},
    {0, 0x110d54, 0x00, 0xff},
    {0, 0x110d55, 0x00, 0xff},
    {0, 0x110d74, 0x22, 0xff},//@@11 Only
    {0, 0x110d75, 0x22, 0xff},//@@11 Only
    {0, 0x10121e, 0x01, 0xff},
    {0, 0x10121f, 0x0c, 0xff},
    {0, 0x10121e, 0x00, 0xff},
    {0, 0x10121f, 0x0c, 0xff},
    {0, 0x110d00, 0x00, 0xff},
    {0, 0x110d01, 0x00, 0xff},
    {0, 0x110d7c, 0x00, 0xff},
    {0, 0x110d7d, 0xfc, 0xff},
    {0, 0x110d60, 0x0a, 0xff},
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d00, 0x01, 0xff},
    {0, 0x110d01, 0x00, 0xff},
    {0, 0x110d08, 0x3f, 0xff},
    {0, 0x110d09, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff},
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xe5, 0xff},
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d0e, 0xa5, 0xff},
    {0, 0x110d0f, 0x00, 0xff},
    {0, 0x110d58, 0x04, 0xff},
    {0, 0x110d59, 0x04, 0xff},
    {0, 0x110d5a, 0x44, 0xff},
    {0, 0x110d5b, 0x44, 0xff},
    {0, 0x110d5c, 0x44, 0xff},
    {0, 0x110d5d, 0x44, 0xff},
    {0, 0x110d5e, 0x44, 0xff},
    {0, 0x110d5f, 0x44, 0xff},
    {0, 0x110d60, 0x0c, 0xff},
    {0, 0x110d61, 0x00, 0xff},//dll_pd
    {0, 0x110d60, 0x08, 0xff},
    {0, 0x110d61, 0x00, 0xff},
    {0, 0x110d60, 0x70, 0xff},
    {0, 0x110d61, 0x23, 0xff},
    {0, 0x101200, 0x00, 0xff},
    {0, 0x101201, 0x00, 0xff},
    {0, 0x101200, 0x08, 0xff},
    {0, 0x101201, 0x00, 0xff},
    {0, 0x101200, 0x0c, 0xff},
    {0, 0x101201, 0x00, 0xff},

    {2, 1000, 0, 0},//delay 1ms
    {0, 0x101200, 0x0e, 0xff},
    {0, 0x101201, 0x00, 0xff},
    {2, 1000, 0, 0},//delay 1ms
#if (MIU_SPEED==DDR2_1066MHz)
    {0, 0x101200, 0x0f, 0xff},//@@turn off ODT for 1066MHz
#elif (MIU_SPEED==DDR2_1333MHz)
    {0, 0x101200, 0x1f, 0xff},//@@turn on ODT for 1333MHz
#else
 #error "Please select correct DDR2 Speed"
#endif
    {0, 0x101201, 0x00, 0xff},
    {1, 0x101201, BIT7, BIT7},//check reg_101200 #15 for wait initial done

    {0, 0x101240, 0x01, 0xff},
    {0, 0x101241, 0x00, 0xff},
    {0, 0x101260, 0x01, 0xff},
    {0, 0x101261, 0x00, 0xff},
    {0, 0x101280, 0x01, 0xff},
    {0, 0x101281, 0x00, 0xff},
    {0, 0x1012a0, 0x01, 0xff},
    {0, 0x1012a1, 0x00, 0xff},
    {0, 0x101246, 0x00, 0xff},
    {0, 0x101247, 0x00, 0xff},
    {0, 0x101266, 0x00, 0xff},
    {0, 0x101267, 0x00, 0xff},
    {0, 0x101286, 0x00, 0xff},
    {0, 0x101287, 0x00, 0xff},
    {0, 0x1012a6, 0x00, 0xff},
    {0, 0x1012a7, 0x00, 0xff},
    {0, 0x101240, 0x01, 0xff},
    {0, 0x101241, 0x80, 0xff},
    {0, 0x101260, 0x01, 0xff},
    {0, 0x101261, 0x80, 0xff},
    {0, 0x101280, 0x01, 0xff},
    {0, 0x101281, 0x80, 0xff},
    {0, 0x1012a0, 0x01, 0xff},
    {0, 0x1012a1, 0x80, 0xff},
    {0, 0x10121e, 0x08, 0xff},
    {0, 0x10121f, 0x1a, 0xff},
    {0, 0x1012d2, 0x00, 0xff},
    {0, 0x1012d3, 0x00, 0xff},
    {0, 0x1012fe, 0x87, 0xff},
    {0, 0x1012ff, 0x00, 0xff},
    {0, 0x1012e0, 0x00, 0xff},
    {0, 0x1012f0, 0x00, 0xff},
    {0, 0x1012f1, 0x00, 0xff},
    {0, 0x101252, 0xff, 0xff},
    {0, 0x101253, 0xff, 0xff},
    {0, 0x101272, 0xff, 0xff},
    {0, 0x101273, 0xff, 0xff},
    {0, 0x101292, 0xff, 0xff},
    {0, 0x101293, 0xff, 0xff},
    {0, 0x1012b2, 0xff, 0xff},
    {0, 0x1012b3, 0xff, 0xff},
   
    {0,0,0,0} //End
};

static const BootConf InitialDRAMOBF[] =
{
    #if 1//(### REVIEW ###))
    //=======================
    // Set DRAM OBF & RC Clock
    //=======================
    {2, 1000, 0, 0}, 
    {0, 0x1012a7, 0xff, 0xFF},

    //Before SW enables DRAMOBF, we need to get related clock sources ready.
    {0, 0x123920, 0xF, 0xF}, //enable rc clock

    //initialize MIU PLL
    //initialize MIU

    {0, 0x103332, 0x0, 0xF}, //release clock gating for clk_aesdma (CLKGEN1)
    {0, 0x123922, 0x1, 0x1}, //release sw reset of rng engine

    //delay 10us to let rc clock & RNG engine stable

    //enable DRAMOBF
    //secure public0, enable MIU0 dramobf
    {0, 0x1239EC, 0x00, 0xFF},
    {0, 0x1239ED, 0xC0, 0xFF},

    //polling until DRAMOBF status is done:
    //secure public0, miu0 dramobf is enabled
    {1, 0x1239EE, 0x1, 0x1},
#endif//(### REVIEW ###))    

    {0,0,0,0} //End
};

static void SwitchCPUClock(void)
{
////////////////////////////////////////////////////                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   
// SWITHC CLK                                                                                                                                                                   	   
////////////////////////////////////////////////////                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   
#if AUTO_DETECT_FLASH_MODE
    U8 u8Vender = 0;
    u8Vender = MDrv_ReadByte(0x0009ca); //detect Flash vendor ID
    if(u8Vender==MID_WB)
    {
        MDrv_WriteByte(0x000ae4, 0x02); // switch spi to dual read mode
    }
    else
    {
        MDrv_WriteByte(0x000ae4, 0x01); // switch spi to fast read mode
    }
    MDrv_Write2Byte(0x0009de, 0x0001);  //clear reg_fsp_clear_done_flag_le_w
#else
    MDrv_WriteByte(0x000ae4, 0x01); // switch spi to fast read mode
#endif
//    MDrv_WriteByte(0x000ae4, 0x01); // switch spi to fast read mode
    MDrv_WriteByte(0x100b20, 0x01); // switch clk_mcu       = 216Mhz (deglitch)                                                                                                                               	   
    MDrv_WriteByte(0x100b21, 0x00); // switch clk_mcu       = 216Mhz (deglitch)                                                                                                                               	   
    MDrv_WriteByte(0x100b2c, 0x1c); // switch clk_spi       = 108Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b2c, 0x3c); // switch clk_spi       = 108Mhz (deglitch)                                                                                                                               	   
    MDrv_WriteByte(0x000f40, 0x80); // switch clk_mcu_pm    = 216Mhz (deglitch)
    //#######################
    // 0x00->0x20 : 240MHz
    // 0x14->0x34 : 288MHz
    // 0x18->0x38 : 320MHz
    //#######################
    MDrv_WriteByte(0x100b25, 0x08); // switch clk_mps_aeon  = 216Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b25, 0x28); // switch clk_mps_aeon  = 216Mhz (deglitch)

    MDrv_WriteByte(0x100b24, 0x00); // switch clk_aeon      = 216Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b24, 0x80); // switch clk_aeon      = 216Mhz (deglitch)                                                                                                                               	   
    MDrv_WriteByte(0x10331e, 0x00); // switch clk_dmdmcu    = 172Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b42, 0x00); // switch clk_vdmcu     = 172Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b27, 0x0c); // switch clk_uart0     = 123Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100be0, 0x04); // switch clk_r2_secure = 320Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100be0, 0x24); // switch clk_r2_secure = 320Mhz (deglitch)                                                                                                                               	   
    MDrv_WriteByte(0x103332, 0x00); // switch clk_aesdma    = 172Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x103333, 0x00); // switch clk_aesdma    = 172Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b6a, 0x0c); // switch clk_jpd       = 160Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100ba6, 0x0c); // switch clk_odclk     = 150Mhz(PLL sets)                                                                                                                                	   
    MDrv_WriteByte(0x100ba7, 0x00); // switch clk_odclk     = 150Mhz(PLL sets)                                                                                                                                	   
    MDrv_WriteByte(0x100bae, 0x00); // switch clk_fifo_mini = 150Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100baf, 0xc1); // switch clk_fifo_mini = 150Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x000f41, 0x10); // switch clk_spi_pm    = 54Mhz                                                                                                                                           	   
    MDrv_WriteByte(0x000f41, 0x50); // switch clk_spi_pm    = 54Mhz  (deglitch)                                                                                                                               	   
    MDrv_WriteByte(0x100bfc, 0x01); // switch clk_ddr_syn   = 432Mhz                                                                                                                                          	   
    MDrv_WriteByte(0x100b67, 0x04); // switch clk_evd_ppu   = 288Mhz                                                                                                                                          	   

}

static void MDrv_SetDramDDRType(void)
{
    U8 u8DDRSize;

    u8DDRSize= MDrv_ReadByte(0x101ec1) & (BIT4|BIT3);
    if((MDrv_ReadByte(0x101ec1) & BIT2)==0)//DDR3 for new rule
    {
        g_bDramDDR3 = TRUE;
    }
    else if(u8DDRSize==DDRSIZE_128M)//DDR3 for old rule
    {
        g_bDramDDR3 = TRUE;
    }
    else//DDR2 for all rules
    {
        g_bDramDDR3 = FALSE;
    }
}

static int InitialSystemPowerUp( void )
{
    int i;

    #if CHIP_FPGA_VERIFY
    return 0;
    #endif

    //########################################
    //
    // Very important for SPI fast read mode
    // 
    MDrv_WriteByte(0x0008e2, 0xFF);
    MDrv_WriteByte(0x0008e3, 0x0F);
    MDrv_WriteByte(0x000E6A, 0x30);      // max SPI pad driving
    MDrv_WriteByte(0x0008e4, 0x01);      //Enable fast read mode, (SPI command is 0x0B)

#if AUTO_DETECT_FLASH_MODE
    MDrv_Write2Byte(0x0009c0, 0x009f);
    MDrv_Write2Byte(0x0009d4, 0x0001);  //write_length
    MDrv_Write2Byte(0x0009d6, 0x0004);  //read_length
    MDrv_Write2Byte(0x0009d8, 0x3807);  //control
    MDrv_Write2Byte(0x0009da, 0x0001);  //trigger
#endif

    for (i = 0; i < 500; i++)
        __asm__ __volatile__("l.nop");
    
    // Need a branch for fast mode to take effect,
    // use inline assembly to avoid eliminated by compiler or linker
    __asm__ __volatile__ (
        "\tl.j      1f\n"
        "\tl.nop    0\n"
        "\tl.nop    0\n"
        "1:");
	//########################################

	MDrv_WriteMaskRegTbl( TurnOnAllAtop );
	MDrv_WriteMaskRegTbl( InitialSystemClk );
	MDrv_SysDelayUs(1000);
	SwitchCPUClock();

    #if 0
    //read revision id from chiptop 0x101e_67[15:8]
    if(MDrv_ReadByte(0x101ecf)>=(U8)0x01)
    {
        MDrv_WriteRegBit(0x1032f1,1,BIT0); //enable Bank MOD LDO 0x1032_78[8] =1
    }
    if(MDrv_ReadByte(0x101ecc)==(U8)0x5E)
    {
        MDrv_WriteRegBit(0x1032f1,1,BIT0); //enable Bank MOD LDO 0x1032_78[8] =1
    }
    #endif
    
        return 1;
}

#if 0//Fix compiling warning
static void MDrv_WriteMiuAtopTbl( const MS_REG_INIT * pRegTable )
{
    U32 u32Index; // register index
    while( 1 )
    {
        u32Index = (((U32)pRegTable[0] << 16) + ((U32)pRegTable[1] << 8) + pRegTable[2] );
        if (u32Index == 0xFFFFFF) // check end of table
            break;
        u32Index &= 0x1FFFFF;
        MDrv_Write2Byte( u32Index         , ( pRegTable[3] + (pRegTable[4] << 8))  );   //miu0_atop
        pRegTable += 5;
    }
}

static void MDrv_WriteMiuTbl( const MS_REG_INIT * pRegTable )
{
    U32 u32Index; // register index
    while( 1 )
    {
        u32Index = (((U32)pRegTable[0] << 16) + ((U32)pRegTable[1] << 8) + pRegTable[2] );
        if (u32Index == 0xFFFFFF) // check end of table
            break;
        u32Index &= 0x1FFFFF;
        MDrv_Write2Byte(u32Index           , ( pRegTable[3] + (pRegTable[4] << 8))  ); //miu0
        pRegTable += 5;
    }
}

static void  MDrv_AutoInitMiuCycle(void)
{
#if 0 //double check with yihao
    MDrv_Write2Byte( 0x101200, 0x0000);  /*reg_0x1200= 0x0000,*/
    MDrv_SysDelayUs(200);                   //delay 200us
    MDrv_Write2Byte( 0x101200, 0x0008);  //reg_0x1200= 0x0008, DRAM reset
    MDrv_SysDelayUs(500);                   //delay 500us
    MDrv_Write2Byte( 0x101200, 0x000c);  //reg_0x1200= 0x000c, DRAmreset+DRAM chip select
    MDrv_Write2Byte( 0x101200, 0x000e);  //reg_0x1200= 0x000E,DRAmreset+DRAM chip select+enable CKE + ODT ON
    MDrv_SysDelayUs(1000);                  //delay 1ms
  #if (MIU_SPEED == DDR2_800MHz)//MCP : Internal DDR2
    MDrv_Write2Byte((0x101200 ), 0x000f);//reg_0x1200= 0x000F,DRAmreset+DRAM chip select+enable CKE +auto initial dram cycle
  #else //> 1066
    MDrv_Write2Byte((0x101200 ), 0x000f);//0x001f//reg_0x1200= 0x001F,DRAmreset+DRAM chip select+enable CKE +auto initial dram cycle + ODT ON
  #endif

    MDrv_SysDelayUs(10*1000);               // delay 10ms : wait initial done
    while( (MDrv_Read2Byte( RIUBASE_MIU  ) & 0x8000 ) == 0x0000 );

    MDrv_Write2Byte( 0x101246, 0x7FFE);
    MDrv_Write2Byte( 0x10121e, 0x0c08);
    MDrv_Write2Byte( 0x1012e0, 0x0000);
    MDrv_Write2Byte( 0x1012e2, 0x0000);
    MDrv_Write2Byte( 0x1012e4, 0x0001);
    MDrv_Write2Byte( 0x1012e6, 0x0000);
    MDrv_Write2Byte( 0x1012e8, 0x5aa5);
    MDrv_WriteRegBit( 0x110d0e, 1, BIT6);
    MDrv_Write2Byte( 0x1012e0, 0x0200);
    MDrv_Write2Byte( 0x1012e0, 0x0201);
    MDrv_Write2Byte( 0x1012e8, 0xffff);
    MDrv_Write2Byte( 0x1012e8, 0xffff);
    MDrv_WriteRegBit (0x110d0e, 0, BIT6);
    MDrv_SysDelayUs(10); //delay 10us
#endif

}
#endif

static void MDrv_MiuReset( void )
{
#if 0 //double check with yihao
    MDrv_Write2Byte( 0x10121e, 0x0c01 );
    MDrv_Write2Byte( 0x10121e, 0x0c00 );
    MDrv_SysDelayUs(1000);  // delay 1ms
    //PKG type select ( 0:DDR3 ; 1:DDR2_MCP ; 2:DDR2(External) )
  #if (MIU_SPEED == DDR2_800MHz) //DDR2_MCP
    //MDrv_Write2Byte( 0x110d7e, 0x0088 );
    MDrv_Write2Byte( 0x110d00, 0x001a );
    MDrv_SysDelayUs(1000);  // delay 1ms
    MDrv_Write2Byte( 0x110d00, 0x0012 );
    MDrv_Write2Byte( 0x110d00, 0x0002 );
    MDrv_Write2Byte( 0x110d02, 0xaaaa );
    MDrv_Write2Byte( 0x110d08, 0x00ff );
    #ifdef DDR2_830MHz//MCLK 415MHz setting
    MDrv_Write2Byte( 0x110d74, 0x0077 ); //trig lvl
    MDrv_Write2Byte( 0x110d5c, 0x3333 ); //drvn
    MDrv_Write2Byte( 0x110d5e, 0x3333 ); //drvp
    #else
    MDrv_Write2Byte( 0x110d74, 0x0022 ); //trig lvl
    MDrv_Write2Byte( 0x110d5c, 0x5555 ); //drvn
    MDrv_Write2Byte( 0x110d5e, 0x5555 ); //drvp
    #endif
  #else
    MDrv_Write2Byte( 0x110d00, 0x001a );
    MDrv_SysDelayUs(1000);  // delay 1ms
    MDrv_Write2Byte( 0x110d00, 0x0012 );
    MDrv_Write2Byte( 0x110d00, 0x0002 );
    MDrv_Write2Byte( 0x110d02, 0xaaaa );
    MDrv_Write2Byte( 0x110d08, 0x00ff );
    MDrv_Write2Byte( 0x110d74, 0x0022 ); //trig lvl
    MDrv_Write2Byte( 0x110d5c, 0x5555 ); //drvn
    MDrv_Write2Byte( 0x110d5e, 0x5555 ); //drvp
  #endif
#endif

}

static void MDrv_BandWidth_Adjustment(void)
{
#if 0 //double check with yihao
  #if (MIU_SPEED == DDR2_800MHz)
    MDrv_Write2Byte(0x101240,0x8001);
    MDrv_Write2Byte(0x101260,0x8001);
  #else //> 1066
    MDrv_Write2Byte(0x101240,0xe001);
    MDrv_Write2Byte(0x101260,0xe001);
  #endif
    MDrv_Write2Byte(0x101280,0x8001); //enable group 2 run-robin
    MDrv_Write2Byte(0x1012a0,0x8001);
    MDrv_Write2Byte(0x10121a,0x0001);

  #if 1//default bandwidth settings
    //Let these IPs HVD/MVD/G3D(removed) use MIU Act 2 bank function to improve DRAM total Bandwidth utilization
    MDrv_Write2Byte(0x10122a, 0xa3a4); //act 2 bank for HVD/MVD
    //MDrv_Write2Byte(0x10123e, 0x4000); //reg_post_order_ctrl_en (Removed for U01 / U02 compatible issue)
    MDrv_Write2Byte(0x101242, 0x0020); //limit 128 burst length
    MDrv_Write2Byte(0x101244, 0x0200); //timeout = 512T
    MDrv_Write2Byte(0x101240, 0x8415); //enable round_robin ; member_limit ; timeout ; skip_on
    MDrv_Write2Byte(0x101262, 0x0020); //limit 128 burst length
    MDrv_Write2Byte(0x101264, 0x0200); //timeout = 512T
    MDrv_Write2Byte(0x101260, 0x8415); //enable round_robin ; member_limit ; timeout ; skip_on
    MDrv_Write2Byte(0x101282, 0x0020); //limit 128 burst length
    MDrv_Write2Byte(0x101284, 0x0200); //timeout = 512T
    MDrv_Write2Byte(0x101280, 0x8415); //enable round_robin ; member_limit ; timeout ; skip_on
  #endif
#endif
}

static void MDrv_MiuAtopProgramDLL0(void)
{
#if 0 //double check with yihao
    //start to program DLL0
    MDrv_Write2Byte(0x110d60,0x000c);
    MDrv_Write2Byte(0x110d60,0x0008);
  #if (MIU_SPEED == DDR2_800MHz)
    MDrv_Write2Byte(0x110d62,0x0c7f); //U02 ECO setting for reg_dll_test[15:8]=0x0c
    MDrv_Write2Byte(0x110d64,0xf200);
  #else //> 1066
    MDrv_Write2Byte(0x110d62,0x007f); //U02 ECO setting for reg_dll_test[15:8]=0x0c
    MDrv_Write2Byte(0x110d64,0xf200);
  #endif
    MDrv_Write2Byte(0x110d60,0x2738);

    MDrv_SysDelayUs(10*1000);  // delay 10ms
#endif
}

static void MDrv_MiuBistTest( void )
{
    MDrv_Write2Byte(0x1012e0 , 0x0000);
    MDrv_Write2Byte(0x1012e2 , 0x0000);

    MDrv_Write2Byte((RIUBASE_MIU + (0x0071 *2)), 0x0000);    //test base address
    MDrv_Write4Byte((RIUBASE_MIU + (0x0072 *2)),(MIU0_SIZE / CHIP_MIU_UNIT )  ); //test length
    MDrv_Write2Byte((RIUBASE_MIU + (0x0074 *2)), 0x5aa5);   //test data
    MDrv_Write2Byte((RIUBASE_MIU + (0x0070 *2)), 0x0003);   //enable test

    while (!MDrv_ReadRegBit( RIUBASE_MIU + (0x0070 *2) + 1, BIT7 ) );
    if( MDrv_Read2Byte( RIUBASE_MIU + (0x0070 *2) ) & 0x6000 )
    {
        printf("BIST_0 error.\n");
      #if 1//add to fix bist error system blocked issue
        WholeChipReset();
      #else
        while(1);//Block here
      #endif
    }
    else
    {
        printf("BIST_0 PASS.\n");
}
}

static void MDrv_MiuDDRInitMakeup(void)
{

}

static int MDrv_Whisky_MiuInitial(void)
{
    //####################
    //
    //Need DDR2 & DDR3 settings
    //
    //####################
    U8 u8DDRSize;
    
    u8DDRSize = MDrv_ReadByte(0x101ec1) & (BIT4|BIT3);
    if(g_bDramDDR3)//DDRSIZE_128M
    {
        //configure MIU and setup dram
        MDrv_WriteMaskRegTbl( InitialMIU_DDR3 );
        //setup dram size
        MDrv_WriteByteMask(0x1012d3, 0x70, 0xF0);
        printf("[Whisky] DDR3 128M\r\n");
    }
    else
    {
        U8 u8DDRBondSW10;

        // acquire Bond SW01
        u8DDRBondSW10 = MDrv_ReadByte(0x101ec1) & (BIT1|BIT0);
        //setup dram column size for 64M/32M
        MDrv_WriteByte(0x101202, ((u8DDRSize==DDRSIZE_64M)? 0x92 : 0x52)); //(64M:32M)=(0x92:0x52)
        //configure MIU and setup dram
        if(u8DDRBondSW10==0x00)
        {
            MDrv_WriteMaskRegTbl( InitialMIU_DDR2_0 );
            //printf("[Whisky] DDR2 E-TRON\r\n");
        }
        else if (u8DDRBondSW10==0x03)
        {
            MDrv_WriteMaskRegTbl( InitialMIU_DDR2_3 );
            //printf("[Whisky] DDR2 W-BOND\r\n");
        }
        else if (u8DDRBondSW10==0x01)
        {
            MDrv_WriteMaskRegTbl( InitialMIU_DDR2_1 );
            //printf("[Whisky] DDR2 NANYA\r\n");
        }
        else
        {
            printf("[Whisky] DDR2 ESMT: No MIU Table\r\n");
            printf("[Whisky] DDR2 BondSW10 Error!!!\r\n");
            while(1);
        }
        //setup dram size
        MDrv_WriteByteMask(0x1012d3, ((u8DDRSize==DDRSIZE_64M)? 0x60 : 0x50), 0xF0);
        
        if(u8DDRSize==DDRSIZE_64M)
        {
            printf("[Whisky] DDR2 64M\r\n");
        }
        else//DDRSIZE_32M
        {
            printf("[Whisky] DDR2 32M\r\n");
        }
    }

    //configure DRAM OBF
    MDrv_WriteMaskRegTbl( InitialDRAMOBF );

#if 0 //double check with yihao
  #if (ENABLE_MSTAR_WHISKY_MMAP_32MB)
    MDrv_Write2Byte( 0x101202, 0x0252 );       
  #else
    if(MDrv_ReadRegBit(0x101ec1,BIT3))//MCP 64M
    {
        MDrv_Write2Byte( 0x101202, 0x0292 );
    }
    else//MCP 32M
    {
        MDrv_Write2Byte( 0x101202, 0x0252 );
    }
  #endif
#endif

    // compensate DDR initialized settings for U01/U02
    MDrv_MiuDDRInitMakeup();
    MDrv_MiuReset();
    //MDrv_AutoInitMiuCycle();
    MDrv_BandWidth_Adjustment();
    MDrv_MiuAtopProgramDLL0();

    //mask all
    MDrv_CloseAllMiumask();
    MDrv_MIU_AutoDQSPhase( 0 , 0x110D6E);
    //MDrv_MIU_AutoDQSPhase( 0 , 0x110DEE);
    MDrv_MiuBistTest();
    //unmask all
    MDrv_OpenAllMiumask();
    MDrv_WriteByte(0x10121e, 0x08); //double check with yihao
    MDrv_Write2Byte(0x1012f0, 0x0000);
    //for tvtool miu access
    //MDrv_Write2Byte(0x002bc6, 0x3c11);
    //MDrv_Write2Byte(0x002bca, 0x403c);
    //MDrv_Write2Byte(0x002bc4, 0x0004);

    //patch MIU group i64 setting
    //client ttx(0C)
    MDrv_WriteRegBit(0x101E63, BIT4, BIT4);// set i64 for VD TTXLS 
    //client ttx(12)
    MDrv_WriteRegBit(0x101E64, BIT2, BIT2);// set i64 for VD TTX
	
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Clock Speed for R2
///////////////////////////////////////////////////////////////////////////////
#define CHIP_ID_REG  0x001ECC
#define CHIPID_WHSK  0x00A0
#define CHIPID_MDLN  0x00B9
static U32 MDrv_GetR2Clock(void);
static U16 MDrv_GetChipID(void);

static U16 MDrv_GetChipID(void)
{
    return MDrv_Read2Byte(CHIP_ID_REG);
}

static U32 MDrv_GetR2Clock(void)
{
    U32 u32R2Clock = 0;

    if((MDrv_ReadByte(0x100B25)>>5)&0x1)
    {
        switch((MDrv_ReadByte(0x100B25)>>2)&0x7)
        {
            case 0x0:
                u32R2Clock = (MDrv_GetChipID()==CHIPID_MDLN)? 240000000 : 375000; //(whisky) XTAL/32
                break;
            case 0x1:
                u32R2Clock = 1500000; //XTAL/8
                break;
            case 0x2: u32R2Clock = 216000000;
                break;
            case 0x3: u32R2Clock = 192000000;
                break;
            case 0x4: u32R2Clock = 172800000;
                break;
            case 0x5:
                u32R2Clock = (MDrv_GetChipID()==CHIPID_MDLN)? 288000000 : 160000000;
                break;
            case 0x6:
                u32R2Clock = (MDrv_GetChipID()==CHIPID_MDLN)? 320000000 : 12000000; //(whisky) not to define mem_clock
                break;
            case 0x7:
                u32R2Clock = 24000000;
                break;
            default:
                //printf("[UART] Get R2 Frequency error\n");
                break;
        }
    }
    else//0: DFT_LIVE
    {
        u32R2Clock = 12000000;
    }
    return u32R2Clock;
}

static void MDrv_InitConsole( void )
{
    #if CHIP_FPGA_VERIFY
    {
        uart_init(12000000, UART0_BAUDRATE_FPGA);
        mtspr( SPR_TTMR, 0x40000000 | 12000 );
    }
    #else
    {
      #ifdef __AEONR2__
        U32 u32R2Clock = MDrv_GetR2Clock();
        uart_init(u32R2Clock, UART0_BAUDRATE);
        mtspr( SPR_TTMR, 0x40000000 | (u32R2Clock/1000) );
      #else
        uart_init(172800000, UART0_BAUDRATE);
        mtspr( SPR_TTMR, 0x40000000 | 172800 );
      #endif
    }
    #endif

}

#define EFUSE_BANK (0x2000)
#define REG_EFUSE_R_ADDR (EFUSE_BANK + 0x27*2)
#define REG_EFUSE_R_TRIGGER (EFUSE_BANK + 0x26*2)
#define REG_EFUSE_R_DATA_LSB (EFUSE_BANK + 0x28*2)
#define REG_EFUSE_R_DATA_MSB (EFUSE_BANK + 0x29*2)

static U32 MDrv_Efuse_Read2Byte(U8 u8OffsetAddr)
{
	U32 u32Value = 0;

	MDrv_Write2Byte(REG_EFUSE_R_ADDR, (U16) u8OffsetAddr);
	MDrv_Write2Byte(REG_EFUSE_R_TRIGGER, 0x0001);

	MDrv_SysDelayUs(1000);

	u32Value = MDrv_Read2Byte(REG_EFUSE_R_DATA_LSB);
	u32Value = (u32Value & 0xFFFF) |( MDrv_Read2Byte(REG_EFUSE_R_DATA_MSB) <<16);

	return u32Value;
}


static U32 MDrv_Efuse_Read2ByteMask(U8 u8OffsetAddr, U8 u8MaskLSB, U8 u8MaskMSB)
{
	U32 u32Mask = 0;
	U32 u32Value = 0;
	U8 i;

	u32Mask = 0;
	for (i = u8MaskLSB; i<=u8MaskMSB; i++)
		u32Mask |= (1<<i);


	u32Value = MDrv_Efuse_Read2Byte(u8OffsetAddr);

	u32Value &= u32Mask;
	return u32Value;
}

static void MDrv_Efuse_Write2Dummy(U8 u8EfuseAddr, U8 u8Efuse_MaskLSB, U8 u8Efuse_MaskMSB, U32 u32DummyBank, U16 u16DummyAddr, U8 u8Dummy_MaskLSB, U8 u8Dummy_MaskMSB)
{

	U32 u32Efuse_Value = 0;
	U16 u16Mask = 0;
	U8 i;
	U16 u16TempValue = 0;

	u32Efuse_Value = MDrv_Efuse_Read2ByteMask(u8EfuseAddr, u8Efuse_MaskLSB, u8Efuse_MaskMSB);
	u32Efuse_Value = (u32Efuse_Value >> u8Efuse_MaskLSB);
	u32Efuse_Value = (u32Efuse_Value << u8Dummy_MaskLSB);

	u32DummyBank = (u32DummyBank << 8) | (u16DummyAddr*2);

	u16Mask = 0;
	for (i = u8Dummy_MaskLSB; i <= u8Dummy_MaskMSB; i++)
		u16Mask |= (1<<i);

	u16TempValue = MDrv_Read2Byte(u32DummyBank);
	u16TempValue &= (~u16Mask);
	u16TempValue |= ( ((U16) u32Efuse_Value) & u16Mask);

	MDrv_Write2Byte(u32DummyBank, u16TempValue);
	

}

static void MDrv_Efuse_Trimming(void)
{
    //############################################
    // Strongly Suggest the procedure below be put on head of this api
    // For Madeline 512x8 efuse bank
    //############################################
    if(MDrv_GetChipID()==CHIPID_MDLN)
    {
      MDrv_WriteByteMask(0x0020B2, BIT0, BIT0);
    }

    //VIDEO_ATOP (GC_REF_TRIM_SEL[4:0])
    if(MDrv_Efuse_Read2ByteMask(0x4c,7,7)) //BGAP_active_flag
    {
        MDrv_Efuse_Write2Dummy(0x4c, 0, 4, 0x1025, 0x5B, 8, 12);
        //VIDEO_ATOP (BGAP_active_flag)
        MDrv_Efuse_Write2Dummy(0x4c, 7, 7, 0x1025, 0x5B, 13, 13);
    }

#if defined(CONFIG_MSTAR_EFUSE_VIDEO_ATOP_FULL_CURRENT)
    #if (ENABLE_MSTAR_EFUSE_VIDEO_ATOP_FULL_CURRENT ==1) //VIDEO_ATOP (Full current mode)
    if(MDrv_Efuse_Read2ByteMask(0x4c,15,15)) //IDAC1_active_flag
    {
	MDrv_Efuse_Write2Dummy(0x4c, 8, 14, 0x103D, 0x08, 0, 6);
    }
    #endif
#elif defined(CONFIG_MSTAR_EFUSE_VIDEO_ATOP_ONE_HALF_CURRENT)
    #if (ENABLE_MSTAR_EFUSE_VIDEO_ATOP_ONE_HALF_CURRENT ==1) //VIDEO_ATOP (1/2 current mode)
    if(MDrv_Efuse_Read2ByteMask(0x4c,23,23)) //IDAC2_active_flag
    {
        MDrv_Efuse_Write2Dummy(0x4c, 16, 22, 0x103D, 0x08, 0, 6);
    }
    #endif
#elif defined(CONFIG_MSTAR_EFUSE_VIDEO_ATOP_ONE_QUARTER_CURRENT)
    #if (ENABLE_MSTAR_EFUSE_VIDEO_ATOP_ONE_QUARTER_CURRENT ==1) //VIDEO_ATOP (1/4 current mode)
    if(MDrv_Efuse_Read2ByteMask(0x4c,31,31)) //IDAC3_active_flag
    {
        MDrv_Efuse_Write2Dummy(0x4c, 24, 30, 0x103D, 0x08, 0, 6);
    }
    #endif
#else
    #error "Please Choose a IDAC Type from e-fuse Video ATOP List"
#endif

    if(MDrv_Efuse_Read2ByteMask(0x4d,4,4)) //ActivateCBUSefuse
    {
        //CBUS for PAD_HOTPLUG_A (PD_1K_HIGH)
        MDrv_Efuse_Write2Dummy(0x4d, 1, 1, 0x101E, 0x0E, 4, 4);
        //CBUS for PAD_HOTPLUG_A (PD_1K_LOW)
        MDrv_Efuse_Write2Dummy(0x4d, 0, 0, 0x101E, 0x0E, 3, 3);
        //CBUS for PAD_HOTPLUG_A (PD_100K_HIGH)
        MDrv_Efuse_Write2Dummy(0x4d, 3, 3, 0x101E, 0x0E, 6, 6);
        //CBUS for PAD_HOTPLUG_A (PD_100K_LOW)
        MDrv_Efuse_Write2Dummy(0x4d, 2, 2, 0x101E, 0x0E, 5, 5);
    }

    //MOD (GCR_ICON_BIT[5:0])
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x08, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x08, 8, 13);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x09, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x09, 8, 13);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0a, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0a, 8, 13);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0b, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0b, 8, 13);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0c, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0c, 8, 13);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0d, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0d, 8, 13);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0e, 0, 5);
    MDrv_Efuse_Write2Dummy(0x4f, 0, 5, 0x111E, 0x0e, 8, 13);				

    //MHL3_ATOP_4PORT (R_CTRL[4:0])
    if(MDrv_Efuse_Read2ByteMask(0x50,5,5)) //ActivateMHLefuse
    {
        MDrv_WriteRegBit( 0x110A19, 1, BIT1);//enable first
        MDrv_Efuse_Write2Dummy(0x50, 0, 4, 0x110A, 0x0c, 0, 4);
    }

    //MHL3_ATOP_4PORT (EQ calibration)
    MDrv_Efuse_Write2Dummy(0x51, 0, 5, 0x1703, 0x19, 0, 5);
    MDrv_Efuse_Write2Dummy(0x51, 6, 11, 0x1703, 0x19, 8, 13);
    MDrv_Efuse_Write2Dummy(0x51, 12, 17, 0x1703, 0x1a, 0, 5);
			
    #if 1//Whisky DDR3 ZQ trimming
    {
        if(g_bDramDDR3)//DDR3
        {
            //DDR(SEL_ZQ_A[6:0])
            if(MDrv_Efuse_Read2ByteMask(0x56,7,7))//check ZQ valid BK_04[7]
            {
                MDrv_Efuse_Write2Dummy(0x56, 0, 6, 0x110d, 0x3d, 8, 14);
            }
            else
            {
                MDrv_Write2Byte( 0x110d00+(0x3d<<1), 0x3800 );
            }
        }
    }
    #endif

}


void BL_Init(void);
void BL_Init(void)
{
    U16 temp=0;
	
    // system MPLL & CLKs initialization   
    InitialSystemPowerUp();
    // console initialization
    MDrv_InitConsole();
  #if (RTL_SIMULATION == 0)
    printf("[Whisky] %s\r\n", BOARD_NAME);
    printf("[Whisky] SPI BOOT\r\n");
    printf("[Whisky] Console Initial OK\r\n");
  #endif
    //Check and set DRAM type
    MDrv_SetDramDDRType();

    // disable AVD before MIU init 
    temp = MDrv_Read2Byte(0x10362a);
    temp = temp | 0x0010;
    MDrv_Write2Byte(0x10362a, temp);
    // HPD setting in port D to pull low
    MDrv_WriteByteMask(0x101e26, 0, 0xc0);

    //Efuse register value  wirte back dummy register
    MDrv_Efuse_Trimming();

    // MIU initialization
    MDrv_Whisky_MiuInitial();

  #if (RTL_SIMULATION == 0)
    printf("\n\r MIU Initial OK \n\r");
  #endif

  #if (RTL_SIMULATION == 1)
    while(1);
  #endif

    // Other initialization
    // Configure floating state for MHL
    MDrv_Write2Byte(0x2F2E, 0x0020);
    // Demod VIF_ADC
    if(MDrv_GetChipID()==CHIPID_MDLN)
    {
      MDrv_WriteByteMask(0x11280F, BIT2, BIT2);//0x1128_07[10] = 1
    }

}

