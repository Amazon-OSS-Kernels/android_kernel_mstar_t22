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
#include "hwreg.h"
#include "MApp_Wakeup.h"
#include "MsTypes.h"
#include "uartdebug.h"
#include "Board.h"

//***********************************************************
//              Macro Definitions
//***********************************************************
//#define PMSLEEP_REG_BASE                        (0x0700*2)
#define REG_PM_WK_IRQ_POL                       ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x09*2))
#define REG_PM_UART_RX_ENABLE                   ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x09*2))
#define REG_PM_HK51_UART0_EN                    ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x09*2))
#define REG_PM_WAKEUP_RST_CHIP_TOP_EN           ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x09*2))
#define REG_PM_WAKEUP_RST_51_EN                 ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x09*2))
#define REG_PM_DEEP_SLEEP                       ((0x00*0x10000) + (PMSLEEP_REG_BASE + 0x09*2))

#define REG_PM_IRQ_SYNC                       BIT3
#define REG_PM_IRQ_RTC0                       BIT4
#define REG_PM_IRQ_DDC                        BIT5
#define REG_PM_IRQ_AVLINK                     BIT6
#define REG_PM_IRQ_RTC1                       BIT7


//***********************************************************
//              Global Functions Declaration
//***********************************************************
void InitialSystemPowerDown(void);
#if BLOCK_POWER_FUNC
void MDrv_Block_PowerUp(void);
#endif

//***********************************************************
//              Local Functions Definition
//***********************************************************
#if BLOCK_POWER_FUNC
void MDrv_Block_PowerUp(void)
{
    //(1) Block Codec
    MDrv_WriteRegBit(0x101e1e, 1, BIT5);
    MDrv_WriteRegBit(0x101e1e, 1, BIT1);
    MDrv_WriteRegBit(0x101e3a, 0, BIT0);
    //(2) Block Demod
    MDrv_WriteRegBit(0x101e1e, 1, BIT6);
    MDrv_WriteRegBit(0x101e1e, 1, BIT2);
    MDrv_WriteRegBit(0x101e3a, 0, BIT1);
    //(3) Block SC
    MDrv_WriteRegBit(0x101e1e, 1, BIT4);
    MDrv_WriteRegBit(0x101e1e, 1, BIT0);
    MDrv_WriteRegBit(0x101e3a, 0, BIT2);
    //(4) Block Vivaldi
    MDrv_WriteRegBit(0x101e1e, 1, BIT7);
    MDrv_WriteRegBit(0x101e1e, 1, BIT3);
    MDrv_WriteRegBit(0x101e3a, 0, BIT3);

}
#endif

//***********************************************************
//              Global Functions Definition
//***********************************************************



static const BootConf DisableSystemClk_Tbl[] =
{
////////////////////////////////////////////////////                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   
// POWER DOWN PRE SETTING                                                                                                                                                       	   
////////////////////////////////////////////////////                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   
    {0, 0x100b20, 0x00, 0xff}, // switch clk_mcu       =  12Mhz                                                                                                                                          	   
    {0, 0x100b2c, 0x1c, 0xff}, // switch clk_spi       =  12Mhz                                                                                                                                          	   
    {0, 0x000f40, 0x00, 0xff}, // switch clk_mcu_pm    =  12Mhz                                                                                                                                          	   
    {0, 0x000f41, 0x10, 0xff}, // switch clk_spi_pm    =  12Mhz                                                                                                                                          	   
    {0, 0x100b25, 0x00, 0xff}, // switch clk_mps_aeon  =  12Mhz                                                                                                                                          	   
    {0, 0x100b24, 0x00, 0xff}, // switch clk_aeon      =  12Mhz                                                                                                                                          	   
    {0, 0x101e87, BIT4, BIT4}, // set pad oen = 1      : pad_pwm0                                                                                                                                        	   
    {0, 0x101e87, BIT5, BIT5}, // set pad oen = 1      : pad_pwm1                                                                                                                                        	   
    {0, 0x101e7e, BIT0, BIT0}, // set pad oen = 1      : pad_gpio0                                                                                                                                       	   
    {0, 0x101e7e, BIT1, BIT1}, // set pad oen = 1      : pad_gpio1                                                                                                                                       	   
    {0, 0x101ef2, BIT2, BIT2}, // set reg_tcon_config2 : pad_tcon2 = gpio mode                                                                                                                           	   
    {0, 0x101eae, 0   , BIT0}, // set reg_spdifout2arc : pad_arc   = gpio mode  	   
	   
                                                                                                                                                                             	   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable CLK (Every CLK)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   			   
   //////////////////////////////////////////// 
     {0, 0x100b2e, BIT0, BIT0}, // disable  clk_spi_m                                                                                                                                                     	   
    {0, 0x100b54, BIT0, BIT0}, // disable  clk_tsp                                                                                                                                                       	   
    {0, 0x100b50, BIT0, BIT0}, // disable  clk_ts0                                                                                                                                                       	   
    {0, 0x100b51, BIT0, BIT0}, // disable  clk_ts1                                                                                                                                                       	   
    {0, 0x103332, BIT0, BIT0}, // disable  clk_aesdma                                                                                                                                                    	   
    {0, 0x100b55, BIT0, BIT0}, // disable  clk_stc0                                                                                                                                                      	   
    {0, 0x100b72, BIT0, BIT0}, // disable  clk_mvd                                                                                                                                                       	   
    {0, 0x103366, BIT0, BIT0}, // disable  clk_parser                                                                                                                                                    	   
    {0, 0x100b74, BIT0, BIT0}, // disable  clk_mvd_chroma                                                                                                                                                	   
    {0, 0x100b75, BIT0, BIT0}, // disable  clk_mvd_luma_a                                                                                                                                                	   
    {0, 0x100b76, BIT0, BIT0}, // disable  clk_mvd_luma_b                                                                                                                                                	   
    {0, 0x100b77, BIT0, BIT0}, // disable  clk_mvd_luma_c                                                                                                                                                	   
    {0, 0x100b78, BIT0, BIT0}, // disable  clk_mvd_rmem                                                                                                                                                  	   
    {0, 0x100b79, BIT0, BIT0}, // disable  clk_mvd_rmem1                                                                                                                                                 	   
    {0, 0x100b7c, BIT0, BIT0}, // disable  clk_mvd_rrefdat                                                                                                                                               	   
    {0, 0x100b98, BIT0, BIT0}, // disable  clk_dc0(MVOP)                                                                                                                                                 	   
    {0, 0x100b2e, BIT0, BIT0}, // disable  clk_vd                                                                                                                                                        	   
    {0, 0x100b2e, BIT0, BIT0}, // disable  clk_vd2x                                                                                                                                                      	   
    {0, 0x100b2e, BIT0, BIT0}, // disable  clk_vd_32fsc                                                                                                                                                  	   
    {0, 0x100b49, BIT0, BIT0}, // disable  clk_vedac(CVBS_DAC)                                                                                                                                           	   
    {0, 0x100b4c, BIT0, BIT0}, // disable  clk_daca2(By application)                                                                                                                                     	   
    {0, 0x100b4d, BIT0, BIT0}, // disable  clk_dacb2(By application)                                                                                                                                     	   
    {0, 0x100b6a, BIT0, BIT0}, // disable  clk_jpd                                                                                                                                                       	   
    {0, 0x100b62, BIT0, BIT0}, // disable  clk_hvd                                                                                                                                                       	   
    {0, 0x100b69, BIT0, BIT0}, // disable  clk_evd                                                                                                                                                       	   
    {0, 0x100b67, BIT0, BIT0}, // disable  clk_evd_ppu                                                                                                                                                   	   
    {0, 0x100b60, BIT0, BIT0}, // disable  clk_vd_mheg5                                                                                                                                                  	   
    {0, 0x103308, BIT0, BIT0}, // disable  clk_atsc_ts                                                                                                                                                   	   
    {0, 0x103309, BIT0, BIT0}, // disable  clk_dvbtc_ts                                                                                                                                                  	   
    {0, 0x103314, BIT0, BIT0}, // disable  clk_dvbtc_adc                                                                                                                                                 	   
    {0, 0x100b1a, BIT0, BIT0}, // disable  clk_vifdbb_dac                                                                                                                                                	   
    {0, 0x100b1b, BIT0, BIT0}, // disable  clk_vifdbb_vdac
    {0, 0x100b27, BIT0, BIT0}, // disable  clk_uart0                                                                                                                                                     	   
    {0, 0x100b28, BIT0, BIT0}, // disable  clk_uart1                                                                                                                                                     	   
    {0, 0x100b3e, BIT0, BIT0}, // disable  clk_miu                                                                                                                                                       	   
    {0, 0x100b3e, BIT0, BIT0}, // disable  clk_miu_rec                                                                                                                                                   	   
    {0, 0x100b3a, BIT0, BIT0}, // disable  clk_miu_256                                                                                                                                                   	   
    {0, 0x100b90, BIT0, BIT0}, // disable  clk_ge                                                                                                                                                        	   
    {0, 0x100b64, BIT0, BIT0}, // disable  clk_idclk_lpll                                                                                                                                                	   
    #if ENABLE_SC_BLOCK_SYNC_WAKEUP
    {0, 0x100ba8, BIT0, BIT0}, // disable  clk_idclk0                                                                                                                                                    	   
    {0, 0x100ba9, BIT0, BIT0}, // disable  clk_idclk1                                                                                                                                                    	   
    {0, 0x100baa, BIT0, BIT0}, // disable  clk_idclk2                                                                                                                                                    	   
    {0, 0x100ba8, BIT0, BIT0}, // disable  clk_sidclk0                                                                                                                                                   	   
    {0, 0x100baa, BIT0, BIT0}, // disable  clk_sidclk2                                                                                                                                                   	   
    #endif
    {0, 0x100ba5, BIT0, BIT0}, // disable  clk_fclk                                                                                                                                                      	   
    {0, 0x100ba2, BIT0, BIT0}, // disable  clk_ficlk_f1                                                                                                                                                  	   
    {0, 0x100ba3, BIT0, BIT0}, // disable  clk_ficlk_f2                                                                                                                                                  	   
    {0, 0x100bae, BIT0, BIT0}, // disable  clk_fifo_mini                                                                                                                                                 	   
    {0, 0x100baf, BIT0, BIT0}, // disable  clk_test_mod                                                                                                                                                  	   
    {0, 0x100ba6, BIT0, BIT0}, // disable  clk_odclk                                                                                                                                                     	   
    {0, 0x100b42, BIT0, BIT0}, // disable  clk_vdmcu                                                                                                                                                     	   
    {0, 0x10331e, BIT0, BIT0}, // disable  clk_dmdmcu                                                                                                                                                    	   
    {0, 0x100b2e, BIT0, BIT0}, // disable  clk_mcu_mail0                                                                                                                                                 	   
    {0, 0x100b2e, BIT0, BIT0}, // disable  clk_mcu_mail1                                                                                                                                                 	   
    {0, 0x100b34, BIT0, BIT0}, // disable  clk_pcm                                                                                                                                                       	   
    {0, 0x100b35, BIT0, BIT0}, // disable  clk_tck                                                                                                                                                       	   
    {0, 0x100b24, BIT0, BIT0}, // disable  clk_aeon                                                                                                                                                      	   
    {0, 0x100b25, BIT0, BIT0}, // disable  clk_mps_aeon                                                                                                                                                  	   
    {0, 0x103360, BIT0, BIT0}, // disable  clk_miic                                                                                                                                                      	   
    {0, 0x103362, BIT0, BIT0}, // disable  clk_clko_1x_4digital                                                                                                                                          	   
    {0, 0x100ba3, BIT4, BIT4}, // disable  clk_ficlk2_f2                                                                                                                                                 	   
    {0, 0x100b5a, BIT0, BIT0}, // disable  clk_stamp                                                                                                                                                     	   
    {0, 0x100b30, BIT0, BIT0}, // disable  clk_spi_m1                                                                                                                                                    	   
    {0, 0x100b2d, BIT0, BIT0}, // disable  clk_mcu_au                                                                                                                                                    	   
    {0, 0x100be0, BIT0, BIT0}, // disable  clk_r2_secure                                                                                                                                                 	   
    {0, 0x10335c, BIT0, BIT0}, // disable  clk_xtal_0                                                                                                                                                    	   
    {0, 0x10335c, BIT1, BIT1}, // disable  clk_xtal_1                                                                                                                                                    	   
    {0, 0x10335c, BIT2, BIT2}, // disable  clk_xtal_2                                                                                                                                                    	   
    {0, 0x10335c, BIT3, BIT3}, // disable  clk_xtal_3                                                                                                                                                    	   
    {0, 0x10335c, BIT4, BIT4}, // disable  clk_xtal_4                                                                                                                                                    	   
    {0, 0x10335c, BIT5, BIT5}, // disable  clk_xtal_5                                                                                                                                                    	   
    {0, 0x10335c, BIT6, BIT6}, // disable  clk_xtal_6                                                                                                                                                    	   
    {0, 0x10335c, BIT7, BIT7}, // disable  clk_xtal_7                                                                                                                                                    	   
    {0, 0x000f60, BIT1, BIT1}, // disable  clk_xtal_pm1 (sbus)                                                                                                                                           	   
    {0, 0x000f60, BIT2, BIT2}, // disable  clk_xtal_pm2                                                                                                                                                  	   
    {0, 0x000f60, BIT3, BIT3}, // disable  clk_xtal_pm3                                                                                                                                                  	   
    {0, 0x100b02, BIT4, BIT4}, // disable  clk_bist_patgen                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // MPLL Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			   
   ////////////////////////////////////////////     
   {0, 0x110c03, BIT0, BIT0}, // reg_mpll_pd    
   {0, 0x110c03, BIT1, BIT1}, // reg_mpll_clk_adc216m_pd 
   {0, 0x110c03, BIT2, BIT2}, // reg_mpll_clk_adc432m_pd        
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // AUR2PLL Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           			   
   ////////////////////////////////////////////    
   {0, 0x160343, BIT0, BIT0}, // en  AUR2PLL_PD
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                			                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // AU_PLL Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
   ////////////////////////////////////////////    
   {0, 0x112c76, BIT7, BIT7}, // en  AUPLL_PD  
   {0, 0x112c76, BIT3, BIT3}, // en  AUPLL_PD_KP0 
   {0, 0x112c76, BIT2, BIT2}, // en  AUPLL_PD_KP1
   {0, 0x112c76, BIT1, BIT1}, // en  AUPLL_PD_KPDIV 
   {0, 0x112c76, BIT0, BIT0}, // en  AUPLL_PD_KF0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    			   
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // MPLL_LVDS Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         			   
   ////////////////////////////////////////////   
   {0, 0x103106, 0x2e, 0xFF},
   {0, 0x103106, BIT5, BIT5}, // Disable LPLL    
		   
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // XTAL Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // DDRPLL Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
   ////////////////////////////////////////////     
   {0, 0x110d33, BIT7, BIT7}, // reg_ddrpll_pd atop 
   {0, 0x110db3, BIT7, BIT7}, // reg_ddrpll_pd atop2

   #if 0 //(### REVIEW ###)
   {0, 0x110d00, 0x18, 0xFF},
   {0, 0x110d01, 0x00, 0xFF},
   {0, 0x110d08, 0x00, 0xFF},
   {0, 0x110d09, 0x00, 0xFF},
   {0, 0x110d2e, 0xFF, 0xFF},
   {0, 0x110d2f, 0xFF, 0xFF},
   {0, 0x110d32, 0x00, 0xFF},
   {0, 0x110d33, 0x80, 0xFF},
   {0, 0x110d54, 0x70, 0xFF},
   {0, 0x110d55, 0x00, 0xFF},
   {0, 0x110d60, 0x02, 0xFF},
   {0, 0x110d61, 0x00, 0xFF},
   {0, 0x110d7c, 0x20, 0xFF},
   {0, 0x110d7d, 0x00, 0xFF},
   {0, 0x110d80, 0x18, 0xFF},
   {0, 0x110d81, 0x00, 0xFF},
   {0, 0x110d88, 0x00, 0xFF},
   {0, 0x110d89, 0x00, 0xFF},
   {0, 0x110dae, 0xFF, 0xFF},
   {0, 0x110daf, 0xFF, 0xFF},
   {0, 0x110db2, 0x00, 0xFF},
   {0, 0x110db3, 0x80, 0xFF},
   {0, 0x110dd4, 0x70, 0xFF},
   {0, 0x110dd5, 0x00, 0xFF},
   {0, 0x110de0, 0x02, 0xFF},
   {0, 0x110de1, 0x00, 0xFF},
   {0, 0x110dfc, 0x20, 0xFF},
   {0, 0x110dfd, 0x00, 0xFF},
    #endif

   {0,0,0,0} //End
};


////////////////////////////////////////////////////                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   
// DISABLE ATOP PART                                                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   
////////////////////////////////////////////////////                                                                                                                            	   



static const BootConf DisableATOP_Tbl[] =
{ 

 ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // DMD_ATOP Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          			   
   ////////////////////////////////////////////    
	{0, 0x112003, 0, BIT5}, // Release Demod ATOP SW reset  
	{0, 0x112840, BIT4, BIT4}, // Powerdown ADC reference voltage 
	{0, 0x112818, BIT2, BIT2}, // Powerdown clamp buffer   
	{0, 0x11281e, BIT7, BIT7}, // Powerdown calibration buffer 
	{0, 0x112818, BIT0, BIT0}, // Powerdown ADC I       
	{0, 0x112879, BIT7, BIT7}, // Powerdown 2.6V LDO
	{0, 0x112879, BIT4, BIT4}, // Powerdown 2.5V LDOI   
	{0, 0x112879, BIT5, BIT5}, // Powerdown 1.1V LDO_CLK   
	{0, 0x112879, BIT6, BIT6}, // Powerdown 1.1V LDO_CMP   
	{0, 0x11286a, BIT7, BIT7}, // Powerdown MPLL   
	{0, 0x11286b, BIT5, BIT5}, // Power down demod PLL's ref clock 
	{0, 0x112860, BIT1, BIT1}, // Power down demod_ADC's clock          
	{0, 0x112860, BIT2, BIT2}, // Digital divider (/2) power down 
	{0, 0x112860, BIT3, BIT3}, // Digital divider (/3) power down 
	{0, 0x112860, BIT4, BIT4}, // Digital divider (/4) power down  
	{0, 0x112860, BIT5, BIT5}, // Digital divider (/8) power down
	{0, 0x112860, BIT6, BIT6}, // Digital divider (/10) power down  
	{0, 0x11286c, 0, BIT4}, // Disable IFAGC_T  
	{0, 0x1128a4, BIT0, BIT0},  // reg_pd_24m_core 
	
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable DDR_ATOP //(### REVIEW ###)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable AUPLL_ATOP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        			   
   ////////////////////////////////////////////         
    {0, 0x112c76, BIT7, BIT7}, // en  AUPLL_PD      
    {0, 0x112c76, BIT3, BIT3}, // en  AUPLL_PD_KP0   
    {0, 0x112c76, BIT2, BIT2}, // en  AUPLL_PD_KP1   
    {0, 0x112c76, BIT1, BIT1}, // en  AUPLL_PD_KPDIV  
    {0, 0x112c76, BIT0, BIT0}, // en  AUPLL_PD_KF0  
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable AUSDM_ATOP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        			   
   ////////////////////////////////////////////      
    {0,0x112ce1 , BIT4, BIT4}, // en  PD_IBIAS_EAR
    {0, 0x112ce0, BIT5, BIT5}, // en  PD_RT_EAR 
    {0, 0x112ce0, 0, BIT4}, // en  PD_LT_EAR    
    {0, 0x112ce3, 0x03, 0x03}, // en  PD_INMUX 
    {0, 0x112ce6, BIT1, BIT1}, // en  PD_MIDTOP   
    {0, 0x112ce6, BIT0, BIT0}, // en  PD_VI    
    {0, 0x112ce9, BIT7, BIT7}, // en  PD_VREF       
    {0, 0x112cea, BIT3, BIT3}, // en  PD_OPLP_EAR   
    {0, 0x112cec, BIT7, BIT7}, // en  PD_LDO_ADC     
    {0, 0x112cec, BIT4, BIT4}, // en  PD_LDO_DAC   
    {0, 0x112cda, BIT1, BIT1}, // en  PD_ADC0    
    {0, 0x112cda, BIT0, BIT0}, // en  PD_ADC1  
    {0, 0x112cdd, BIT5, BIT5}, // en  PD_L0_DAC   
    {0, 0x112cdd, BIT4, BIT4}, // en  PD_L1_DAC   
    {0, 0x112cdd, BIT3, BIT3}, // en  PD_L2_DAC 
    {0, 0x112cdd, BIT2, BIT2}, // en  PD_L3_DAC  
    {0, 0x112cdd, BIT1, BIT1}, // en  PD_R0_DAC    
    {0, 0x112cdd, BIT0, BIT0}, // en  PD_R1_DAC   
    {0, 0x112cdc, BIT7, BIT7}, // en  PD_R2_DAC       
    {0, 0x112cdc, BIT6, BIT6}, // en  PD_R3_DAC 
    {0, 0x112cdc, BIT5, BIT5}, // en  PD_REF_DAC   
    {0, 0x112cdc, BIT4, BIT4}, // en  PD_BIAS_DAC   
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable MOD_ATOP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          			   
   ////////////////////////////////////////////  
    {0, 0x1032f0, 0x01, 0xFF},
    {0, 0x1032f1, 0x00, 0xFF},
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable USB_ATOP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          			   
   ////////////////////////////////////////////    
    {0, 0x103a80, 0xC3, 0xFF},//port0
    {0, 0x103a81, 0xDD, 0xFF},
    {0, 0x103a88, 0x8F, 0xFF},
    {0, 0x103a00, 0xC3, 0xFF},//port1
    {0, 0x103a01, 0xDD, 0xFF},
    {0, 0x103a08, 0x8F, 0xFF},

   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable HDMI_ATOP  //(### REVIEW ###)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               			   
   ////////////////////////////////////////////   
    {0, 0x000e94, BIT7,  BIT7}, // [7]       PD_IBGREX        = 1'd0 
    {0, 0x000e97, 0x03,  0x03}, // [9]       P0_PD_CLKIN_OFFL = 1'd0                                                                                                                                      	   
    #if (ENABLE_SUPPORT_DVI_WAKEUP==0)
    {0, 0x000e98, BIT0,  BIT0}, // [0]       PD_BG            = 1'd0                                                                                                                                      	   
    #endif
    {0, 0x1109be, BIT0,  BIT0}, // [0]       P0_PD_ENVDET     = 1'd0                                                                                                                                      	   
    {0, 0x1109c0, 0xa7,  0xa7}, // [10:08]   P0_PD_MXD        = 3'd0                                                                                                                                      	   
    {0, 0x1109c1, 0x07,  0x07}, // [10:08]   P0_PD_MXD        = 3'd0                                                                                                                                      	   
    {0, 0x1109d3, 0x07,  0x07}, // [10:08]   PD_P0_DPLPHI(Q)  = 3'd0                                                                                                                                      	   
    //modify clock order from bit3 to bit0
    {0, 0x002ec6, 0x0e,  0x0f}, // [3:0]   MHL3_ATOP_P0 NODIE_PD_RT = 4'de                                                                                                                                	   
    {0, 0x002ec6, 0xe0,  0xf0}, // [7:4]   MHL3_ATOP_P1 NODIE_PD_RT = 4'de                                                                                                                                	   
    {0, 0x002ec7, 0x0e,  0x0f}, // [11:8]  MHL3_ATOP_P2 NODIE_PD_RT = 4'de                                                                                                                                	   

   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // Disable VIDEO_ATOP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        			   
   ////////////////////////////////////////////   
    {0, 0x102503, 0, BIT6}, // en  EN_MUX_RGB  
    {0, 0x102509, BIT2, BIT2}, // en  PD_ICLP_RGB   
    {0, 0x102509, BIT3, BIT3}, // en  PD_ICLP_VDY
    {0, 0x102509, BIT4, BIT4}, // en  PD_ICLP_VDC   
    {0, 0x10250a, BIT2, BIT2}, // en  PD_SOG_DAC 
    {0, 0x10250a, BIT3, BIT3}, // en  PD_SOG_MAIN 
    {0, 0x10257e, BIT0, BIT0}, // en  PD_SOG_MUX    
    {0, 0x10250a, BIT4, BIT4}, // en  PD_SOGOFF_DAC   
    {0, 0x10250a, BIT5, BIT5}, // en  PD_SOGOFF_MAIN 
    {0, 0x10257e, BIT1, BIT1}, // en  PD_SOGOFF_MUX  
    {0, 0x102508, BIT2, BIT2}, // en  PD_BG
    {0, 0x102508, BIT3, BIT3}, // en  PD_REF
    {0, 0x102508, BIT0, BIT0}, // en  PD_REF_RGB 
    {0, 0x102508, BIT1, BIT1}, // en  PD_REF_VD  
    {0, 0x102508, BIT4, BIT4}, // en  PD_REF_YUV    
    {0, 0x102505, 0, BIT0}, // en  EN_MUX_VD_Y
    {0, 0x102505, 0, BIT1}, // en  EN_MUX_VD_C
    {0, 0x102570, 0, BIT1}, // en  EN_VDLPF_C  
    {0, 0x102570, 0, BIT0}, // en  EN_VDLPF_Y   
    {0, 0x102590, 0, BIT3}, // en  EN_IDAC1  
    {0, 0x103d00, 0, BIT4}, // en  EN_IDAC_REF   
    #if ENABLE_SC_BLOCK_SYNC_WAKEUP
    {0, 0x10250b, BIT0, BIT0}, // en  PD_HSYNC0_COMP    
    {0, 0x10250b, BIT1, BIT1}, // en  PD_HSYNC1_COMP    
    {0, 0x10250b, BIT2, BIT2}, // en  PD_HSYNC2_COMP
    #endif
    {0, 0x102509, BIT1, BIT1}, // en  PD_ADCPLA_REG   
    {0, 0x102509, BIT0, BIT0}, // en  PD_PGDIG_A 
    {0, 0x102508, BIT5, BIT5}, // en  PD_ADCR
    {0, 0x102508, BIT6, BIT6}, // en  PD_ADCG 
    {0, 0x102508, BIT7, BIT7}, // en  PD_ADCB 
    {0, 0x102509, BIT6, BIT6}, // en  PD_ADCY  
    {0, 0x102580, BIT6, BIT6}, // en  PD_FBLANK  
    {0, 0x1025bc, BIT6, BIT6}, // en  PD_ADCA_PHDAC_R    
    {0, 0x1025bc, BIT7, BIT7}, // en  PD_ADCA_PHDAC_G  
    {0, 0x1025bd, BIT0, BIT0}, // en  PD_ADCA_PHDAC_B  
    {0, 0x1025bd, BIT1, BIT1}, // en  PD_ADCA_PHDAC_Y
    {0, 0x1025b4, BIT5, BIT5}, // en  PD_LDO11  
    {0, 0x1025b4, BIT6, BIT6}, // en  PD_LDO25_ADCA
    {0, 0x1025b4, BIT7, BIT7}, // en  PD_LDO25_ADCB
 
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // DDRPLL Disable                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
   ////////////////////////////////////////////    
    {0, 0x110d54, 0x70, 0x70}, // [6:4]  reg_atop_pd = 3'b111   
    {0, 0x110dd4, 0x70, 0x70}, // [6:4]  reg_atop_pd = 3'b111 

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           			   
////////////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
////////////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
// BLOCK OFF                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    			   
////////////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
////////////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                            			   
   			   
			   
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // CODEC       Domain Power off                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			   
   ////////////////////////////////////////////              
    {0, 0x101e3a, BIT0, BIT0}, // : codec     block Release RESET : B0 [ 0]    
    {0, 0x101e1e, 0, BIT1}, // : codec     block ISO_OFF       : B0 [ 1]  
    {0, 0x101e1e, 0, BIT5}, // : codec     block PWR_RQ        : B0 [ 5]  
    {1, 0x101e1f, 0x7d, 0xFF}, // : check     block PWR_GNT       : B1 [ 1]    
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // SC          Domain Power off                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			   
   ////////////////////////////////////////////        
    #if ENABLE_SC_BLOCK_SYNC_WAKEUP
    {0, 0x101e3a, BIT2, BIT2}, // : sc        block Release RESET : B0 [ 0]               
    {0, 0x101e1e, 0, BIT0}, // : sc        block ISO_OFF       : B0 [ 0]     
    {0, 0x101e1e, 0, BIT4}, // : sc        block PWR_RQ        : B0 [ 4]   
    {1, 0x101e1f, 0x7c, 0xFF},  // : check     block PWR_GNT       : B1 [ 0] 
    #endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // DEMOD       Domain Power off                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			   
   ////////////////////////////////////////////       
    {0, 0x101e3a, BIT1, BIT1}, // : demod     block Release RESET : B0 [ 1]  
    {0, 0x101e1e, 0, BIT2}, // : demod     block ISO_OFF       : B0 [ 2]
    {0, 0x101e1e, 0, BIT6}, // : demod     block PWR_RQ        : B0 [ 6]
    {1, 0x101e1f, 0x78, 0xFF}, // : check     block PWR_GNT       : B1 [ 2]  
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       			   
   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // VIVALDI     Domain Power off                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			   
   ////////////////////////////////////////////   
    {0, 0x101e3a, BIT3, BIT3}, // : vivaldi   block Release RESET : B0 [ 3]     
    {0, 0x101e1e, 0, BIT3}, // : vivaldi   block ISO_OFF       : B0 [ 3] 
    {0, 0x101e1e, 0, BIT7}, // : vivaldi   block PWR_RQ        : B0 [ 7] 
    {1, 0x101e1f, 0x70, 0xFF}, // : check     block PWR_GNT       : B1 [ 3]  

   ////////////////////////////////////////////                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 			   
   // HDMI    Domain Power Off                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  			   
   ////////////////////////////////////////////    
    #if (ENABLE_SUPPORT_DVI_WAKEUP==0)
    {0, 0x101e3a, BIT5, BIT5}, // : hdmi      block Release RESET : B0 [ 5]  
    {0, 0x101e9c, 0, BIT0}, // : hdmi      block ISO_OFF       : B0 [ 0]   
    {0, 0x101e9c, 0, BIT1}, // : hdmi      block PWR_RQ        : B0 [ 1]    
    {1, 0x101e9c, 0, 0xFF}, // : check     block PWR_GNT       : B0 [ 2]   
    #endif

     {0,0,0,0} //End
};

void InitialSystemPowerDown(void)
{
    MDrv_WriteMaskRegTbl( DisableSystemClk_Tbl ); //Close System Clock
    MDrv_WriteMaskRegTbl( DisableATOP_Tbl ); //Close System Block Power
}


