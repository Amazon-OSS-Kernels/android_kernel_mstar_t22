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

#ifndef __DRV_URSA_20_C__
#define __DRV_URSA_20_C__

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <drvGPIO.h>

#include "ursa_20.h"
#include <panel/panel.h>
#include <MsSystem.h>

//-------------------------------------------------------------------------------------------------
//  Define
//-------------------------------------------------------------------------------------------------
#define URSA_20_CMD_DATA_SUM             (7)

#define URSA_20_I2C_BUS_NUM              (1)
#define URSA_20_DDC2BI_MODE_ADDR         (0x040)
#define URSA_20_I2C_SUB_ADDR            (0)

#define URSA_URSA20_PANEL_INIT_DELAY           (1200)  //1200ms

typedef enum
{
    UC_INIT_RX_OSD_2LANE,
    UC_INIT_RX_OSD_4LANE,
    UC_INIT_RX_VIDEO_2LANE,
    UC_INIT_RX_VIDEO_4LANE,
    UC_SET_IN_OSD_FULLHD,
    UC_SET_IN_OSD_4K2K,
    UC_SET_IN_VIDEO_FULLHD,
    UC_SET_IN_VIDEO_4K2K,
    UC_SET_LVDS_RX_RECONFIG,//     = 0x106,
    UC_SET_3D_MODE,            //  = 0x301,
    UC_WRITE_REGISTER_1,           //0x31B
    UC_WRITE_REGISTER_2,
    UC_WRITE_REGISTER_3,
    UC_WRITE_REGISTER_MASK_1,
    UC_WRITE_REGISTER_MASK_2,
    UC_URSA_MOD_POWER_ON,
    UC_INIT_RX_VIDEO_8LANE,
    UC_URSA_GAIN_PATCH,
    UC_URSA_MONACO_PATCH,
    UC_URSA_DISABLE_OSD,
#if defined(CONFIG_URSA_TXRX_SUPPORT)
    UC_URSA_INIT_OUT_ALL_1, //0x010
    UC_URSA_INIT_OUT_ALL_2, //0x011
    UC_URSA_INIT_OUT_ALL_3, //0x012
    UC_URSA_INIT_OUT_ALL_4, //0x013
    UC_URSA_INIT_OUT_ALL_5_VByOne_index0, //0x014 VByOne index 0
    UC_URSA_INIT_OUT_ALL_5_VByOne_index1, //0x014 VByOne index 1
    UC_URSA_INIT_OUT_ALL_5_VByOne_index2, //0x014 VByOne index 2
    UC_URSA_INIT_OUT_ALL_5_VByOne_index3, //0x014 VByOne index 3
    UC_URSA_INIT_OUT_ALL_5_VByOne_index4, //0x014 VByOne index 4
    UC_URSA_INIT_OUT_ALL_5_VByOne_index5, //0x014 VByOne index 5
    UC_URSA_INIT_OUT_ALL_6, //0x015
    UC_URSA_INIT_FINISH,     //0x100
#endif
    UC_URSA_PANEL_ON, //0xD01
    UC_URSA_PANEL_OFF, //0xD01
    UC_READ_GPIO, //0xD12
    UC_SET_GPIO,  //0x401
}URSA_20_CMD_TYPE;

#if 0 // Move to board.h
static SWI2C_BusCfg Ursa_20_I2cBusCfg[URSA_20_I2C_BUS_NUM]=
{
    // Bus-0
    {PAD_DDCR_CK, PAD_DDCR_DA, 100},//IS_SW_I2C  /SCL_PAD /SDA_PAD /Delay
    // Bus-1
   // {PAD_TGPIO2, PAD_TGPIO3, 400},   //IS_SW_I2C  /SCL_PAD /SDA_PAD /Delay
    // Bus-2
   // {PAD_GPIO36, PAD_GPIO37, 480},   //IS_SW_I2C  /SCL_PAD /SDA_PAD /Delay
    // Bus-3
   // {PAD_GPIO28, PAD_GPIO29, 480},   //IS_SW_I2C  /SCL_PAD /SDA_PAD /Delay
    // Others, add other bus here
};
#endif

//-------------------------------------------------------------------------------------------------
//  Function define
//-------------------------------------------------------------------------------------------------
void MDrv_Ursa_20_SWI2C_Init(void)
{
    static MS_U8 is_first_init = FALSE;
    unsigned long lpanelVCCOnTimer = 0;
    unsigned long ltotalTime = 0;
    unsigned long lursaDelayTime = 0;
    extern MS_BOOL g_msI2C_init_ready;

    UBOOT_TRACE("IN\n");
    if(is_first_init == FALSE)
    {
        UBOOT_DEBUG("[URSA20] Initialize SW I2C\n");
        // according to spec, before sending i2c command to U20, we must delay about 1200ms
        lpanelVCCOnTimer = MApi_PNL_GetPanelVccOnTime();
        ltotalTime = MsSystemGetBootTime();
        if(ltotalTime > lpanelVCCOnTimer)
        {
            lursaDelayTime = ltotalTime - lpanelVCCOnTimer;
            if(lursaDelayTime < URSA_URSA20_PANEL_INIT_DELAY)
            {
                lursaDelayTime = URSA_URSA20_PANEL_INIT_DELAY - lursaDelayTime;
                printf("[%s][%d]ursa20 delay time: %ld\n", __FUNCTION__, __LINE__, lursaDelayTime);
                mdelay(lursaDelayTime);
            }
        }
#if 0
        //MApi_SWI2C_Init(Ursa_20_I2cBusCfg, URSA_20_I2C_BUS_NUM);
        if(g_msI2C_init_ready == FALSE)
        {
            UBOOT_ERROR("SWI2C is not init, please enable CONFIG_MSTARI2C\n");
            UBOOT_ERROR("SWI2C is not init, please enable CONFIG_MSTARI2C\n");
            UBOOT_ERROR("SWI2C is not init, please enable CONFIG_MSTARI2C\n");
        }
#endif
        is_first_init = TRUE;
    }
    else
    {
        UBOOT_ERROR("[URSA20] SW I2C is already initialized\n");
        return;
    }
    UBOOT_TRACE("OK\n");
}

MS_BOOL MDrv_Ursa_20_SWI2C_WriteBytes(MS_U16 u16BusNumSlaveID, MS_U8 u8addrnum, MS_U8* pu8addr, MS_U16 u16size, MS_U8* pu8data)
{
    return MApi_SWI2C_WriteBytes(u16BusNumSlaveID, u8addrnum, pu8addr, u16size, pu8data);
}

MS_BOOL MDrv_Ursa_20_SWI2C_ReadBytes(MS_U16 u16BusNumSlaveID, MS_U8 u8addrnum, MS_U8* pu8addr, MS_U16 u16size, MS_U8* pu8data)
{
    return MApi_SWI2C_ReadBytes(u16BusNumSlaveID, u8addrnum, pu8addr, u16size, pu8data);
}

void CalCheckSum(MS_U8 *pBuf, MS_U8 u8BufLen )
{
    UBOOT_TRACE("IN\n");
    MS_U8 u8CheckSum =0;

    while( u8BufLen > 1 )//calculate pBuf[1]~pBuf[End] do not count pBuf[0],it is checksum
    {
        -- u8BufLen;
        u8CheckSum += pBuf[u8BufLen] & 0x0F;
    }
    u8CheckSum &= 0x0F;
    pBuf[0] &= 0x0F;
    pBuf[0] |= ((u8CheckSum )<<4);
    UBOOT_TRACE("OK\n");
}

MS_BOOL MDrv_Ursa_20_Write_Cmd(MS_U8 *pu8CmdData)
{
    MS_U16 u16BusNumSlaveID = URSA_20_DDC2BI_MODE_ADDR;
    // MUST re-calculator check sum pre I2C CMD.
    CalCheckSum(pu8CmdData, MFC_CMD_DATA_SUM);
    printf("[Ursa20]Write cmd:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\n", pu8CmdData[0], pu8CmdData[1], pu8CmdData[2], pu8CmdData[3], pu8CmdData[4], pu8CmdData[5], pu8CmdData[6]);

    return MDrv_Ursa_20_SWI2C_WriteBytes(u16BusNumSlaveID, 0, NULL, URSA_20_CMD_DATA_SUM, pu8CmdData);
}

MS_BOOL MDrv_Ursa_20_Read_Cmd(MS_U8 *pu8CmdData)
{
    UBOOT_TRACE("IN\n");
    MS_U16 u16BusNumSlaveID = URSA_20_DDC2BI_MODE_ADDR;
    MS_U8 u8SubAddr = URSA_20_I2C_SUB_ADDR;
    MS_BOOL bRet = FALSE;

    MDrv_Ursa_20_SWI2C_Init();
    printf("[Ursa9]Read cmd\r\n");
    bRet = MDrv_Ursa_20_SWI2C_ReadBytes(u16BusNumSlaveID, u8SubAddr, NULL, 1, pu8CmdData);

    UBOOT_TRACE("OK\n");
    return bRet;
}

MS_BOOL MDrv_Ursa_20_SendCmd(URSA_20_CMD_TYPE eCmdType)
{
    UBOOT_TRACE("IN\n");
    MS_U8 pCmdData[URSA_20_CMD_DATA_SUM] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    MS_BOOL bUnkown = FALSE;
    MS_BOOL bRet = FALSE;
#if defined(CONFIG_URSA_TXRX_SUPPORT)
	st_PNL_UrsaRXTX_PanelParameter stUrsaRXTX_PanelParameter = GetUrsaRxTxPanelParameter();
#endif

    switch(eCmdType)
    {
        case UC_INIT_RX_OSD_2LANE:
            {
                //0xE0, 0x01, 0x02, 0x05, 0x4C, 0x5C, 0x00
                pCmdData[0] = 0xE0;
                pCmdData[1] = 0x01;
                pCmdData[2] = 0x02;
                pCmdData[3] = 0x05;
                pCmdData[4] = 0x8C;
                pCmdData[5] = 0x9C;
                pCmdData[6] = 0x00;
            }
            break;
        case UC_INIT_RX_OSD_4LANE:
            {
                //0xF0, 0x01, 0x02, 0x0F, 0x46, 0x57, 0x00
                pCmdData[0] = 0x60;
                pCmdData[1] = 0x01;
                pCmdData[2] = 0x02;
                pCmdData[3] = 0x0F;
                pCmdData[4] = 0x89;
                pCmdData[5] = 0xAB;
                pCmdData[6] = 0x00;
            }
            break;
        case UC_INIT_RX_VIDEO_2LANE:
            {
                //0xC0, 0x02, 0x11, 0xC0, 0xCC, 0xC1, 0xCC}
                pCmdData[0] = 0x50;
                pCmdData[1] = 0x02;
                pCmdData[2] = 0x11;
                pCmdData[3] = 0x44;
                pCmdData[4] = 0x44;
                pCmdData[5] = 0x55;
                pCmdData[6] = 0x55;
            }
            break;
        case UC_INIT_RX_VIDEO_4LANE:
            {
                //0xD0, 0x02, 0x55, 0xC0, 0xC2, 0xC1, 0xC3}
                pCmdData[0] = 0xD0;
                pCmdData[1] = 0x02;
                pCmdData[2] = 0x55;
                pCmdData[3] = 0xC0;
                pCmdData[4] = 0xC2;
                pCmdData[5] = 0xC1;
                pCmdData[6] = 0xC3;
            }
            break;
        case UC_INIT_RX_VIDEO_8LANE:
            {
                //0xD0, 0x02, 0x55, 0xC0, 0xC2, 0xC1, 0xC3}
                pCmdData[0] = 0x70;
                pCmdData[1] = 0x02;
                pCmdData[2] = 0xFF;
                pCmdData[3] = 0x62;
                pCmdData[4] = 0x40;
                pCmdData[5] = 0x73;
                pCmdData[6] = 0x51;
            }
            break;
        case UC_SET_IN_OSD_FULLHD:
            {
                //0xA1, 0x04, 0x03, 0x07, 0x80, 0x04, 0x38
                pCmdData[0] = 0xA1;
                pCmdData[1] = 0x04;
                pCmdData[2] = 0x03;
                pCmdData[3] = 0x07;
                pCmdData[4] = 0x80;
                pCmdData[5] = 0x04;
                pCmdData[6] = 0x38;
            }
            break;
        case UC_SET_IN_OSD_4K2K:
            {
                //0xE1, 0x04, 0x03, 0x0F, 0x00, 0x08, 0x70
                pCmdData[0] = 0xE1;
                pCmdData[1] = 0x04;
                pCmdData[2] = 0x03;
                pCmdData[3] = 0x0F;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x08;
                pCmdData[6] = 0x70;
            }
            break;
        case  UC_SET_IN_VIDEO_FULLHD:
            {
                //0x81, 0x05, 0x07, 0x80, 0x04, 0x38, 0x00
                pCmdData[0] = 0x81;
                pCmdData[1] = 0x05;
                pCmdData[2] = 0x07;
                pCmdData[3] = 0x80;
                pCmdData[4] = 0x04;
                pCmdData[5] = 0x38;
                pCmdData[6] = 0x00;
            }
            break;
        case  UC_SET_IN_VIDEO_4K2K:
            {
                //0xC1, 0x05, 0x0F, 0x00, 0x08, 0x70, 0x00
                pCmdData[0] = 0xC1;
                pCmdData[1] = 0x05;
                pCmdData[2] = 0x0F;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x08;
                pCmdData[5] = 0x70;
                pCmdData[6] = 0x00;
            }
            break;
        case UC_SET_LVDS_RX_RECONFIG:
            {
                //0x61, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00
                pCmdData[0] = 0x61;
                pCmdData[1] = 0x06;
                pCmdData[2] = 0x00;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;
            }
            break;
        case UC_SET_3D_MODE:
            {
                //0x81,0x05,0x07,0x80,0x04,0x38,0x00
                pCmdData[0] = 0x13;
                pCmdData[1] = 0x01;
                pCmdData[2] = 0x00;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;
            }
            break;
        case UC_WRITE_REGISTER_1:
            {
                //0x23,0x1B,0x10,0x27,0xC2,0xEE,0xD0,
                pCmdData[0] = 0x23;
                pCmdData[1] = 0x1B;
                pCmdData[2] = 0x10;
                pCmdData[3] = 0x27;
                pCmdData[4] = 0xC2;
                pCmdData[5] = 0xEE;
                pCmdData[6] = 0xD0;
            }
            break;
        case UC_WRITE_REGISTER_2:
            {
                //0x23,0x1B,0x10,0x27,0x40,0x40,0xD0
                pCmdData[0] = 0x23;
                pCmdData[1] = 0x1B;
                pCmdData[2] = 0x10;
                pCmdData[3] = 0x27;
                pCmdData[4] = 0x40;
                pCmdData[5] = 0x40;
                pCmdData[6] = 0xD0;
            }
            break;
        case UC_WRITE_REGISTER_3:
            {
                //0x33,0x1B,0x10,0x27,0xC3,0xEE,0xD0
                pCmdData[0] = 0x33;
                pCmdData[1] = 0x1B;
                pCmdData[2] = 0x10;
                pCmdData[3] = 0x27;
                pCmdData[4] = 0xC3;
                pCmdData[5] = 0xEE;
                pCmdData[6] = 0xD0;
            }
            break;
        case UC_WRITE_REGISTER_MASK_1:
            {
                //0x33,0x1C,0x13,0x00,0x04,0x40,0x40
                pCmdData[0] = 0x33;
                pCmdData[1] = 0x1C;
                pCmdData[2] = 0x13;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x04;
                pCmdData[5] = 0x40;
                pCmdData[6] = 0x40;
            }
            break;
        case UC_WRITE_REGISTER_MASK_2:
            {
                //0x33,0x1C,0x13,0x00,0x04,0x00,0x40
                pCmdData[0] = 0x33;
                pCmdData[1] = 0x1C;
                pCmdData[2] = 0x13;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x04;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x40;
            }
            break;
        case UC_URSA_MOD_POWER_ON:
            {
               //URSA MOD Power On command: 0x93, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00
                pCmdData[0] = 0x93;
                pCmdData[1] = 0x08;
                pCmdData[2] = 0x01;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;

            }
            break;
        case UC_URSA_GAIN_PATCH:
            {

                pCmdData[0] = 0x99;
                pCmdData[1] = 0x08;
                pCmdData[2] = 0x01;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;

            }
            break;
        case UC_URSA_MONACO_PATCH:
            {

                pCmdData[0] = 0x01;
                pCmdData[1] = 0x10;
                pCmdData[2] = 0x01;
                pCmdData[3] = 0x10;
                pCmdData[4] = 0xFF;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;

            }
            break;
        case UC_URSA_DISABLE_OSD:
            {

                pCmdData[0] = 0x41;
                pCmdData[1] = 0x04;
                pCmdData[2] = 0x00;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;

            }
            break;
#if defined(CONFIG_URSA_TXRX_SUPPORT)
        case UC_URSA_INIT_OUT_ALL_1:
            {   //BMS 48 inch , 0x010 09 02 D8 B0 80
                MS_U8 u8Data2 = 0x39;
                MS_U8 u8Data3 = 0x02;
                MS_U8 u8Data4 = 0xD8;
                MS_U8 u8Data5 = 0xB1;
                MS_U8 u8Data6 = 0x80;

                #if (CONFIG_PANEL_INIT == 1)
                u8Data2 = (stUrsaRXTX_PanelParameter.m_eMfc_Panel_3D_Type & 0x7F);
                u8Data3 = 0;
                u8Data4 = stUrsaRXTX_PanelParameter.m_u8MfcLVDSSwapCh;
                u8Data5 = 0;
                u8Data6 = 0;

                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_mask_HS & 0x01) << 7);
                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_mask_VS & 0x01) << 6);
                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_mask_DE & 0x01) << 5);
                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_invert_HS & 0x01) << 4);
                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_invert_VS & 0x01) << 3);
                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_invert_DE & 0x01) << 2);
                u8Data3 |= ((stUrsaRXTX_PanelParameter.m_bMfc_tx_dither_8_bits & 0x01) << 1);
                u8Data3 |= (stUrsaRXTX_PanelParameter.m_bMfc_tx_dither_enable & 0x01);
                u8Data5 |= ((stUrsaRXTX_PanelParameter.m_eMfc_lvds_tx_bit_num & 0x03) << 3);
                u8Data5 |= ((stUrsaRXTX_PanelParameter.m_eMfc_panel_pair_swap_type & 0x03) << 1);
                u8Data5 |= (stUrsaRXTX_PanelParameter.m_bMfc_panel_lvds_tx_swap_pol & 0x01);
                u8Data6 |= ((stUrsaRXTX_PanelParameter.m_eMfc_panel_inc_vtotal_for_50hz & 0x01) << 7);
                u8Data6 |= ((stUrsaRXTX_PanelParameter.m_eMfc_dither_tailcut & 0x01) << 6);
                #endif
                pCmdData[0] = 0x00;
                pCmdData[1] = 0x10;
                pCmdData[2] = u8Data2;
                pCmdData[3] = u8Data3;
                pCmdData[4] = u8Data4;
                pCmdData[5] = u8Data5;
                pCmdData[6] = u8Data6;
                //MDrv_Ursa_19_CalCheckSum(pCmdData, URSA_19_CMD_DATA_SUM);
            }
            break;

        case UC_URSA_INIT_OUT_ALL_2:
            {   //BMS 48 inch , 0x011 11 30 18 CA 80
                 MS_U8 u8Data2 = 0x11;
                 MS_U8 u8Data3 = 0x30;
                 MS_U8 u8Data4 = 0x18;
                 MS_U8 u8Data5 = 0xCA;
                 MS_U8 u8Data6 = 0x80;
                 #if (CONFIG_PANEL_INIT == 1)
                 u8Data2 = 0;
                 u8Data3 = 0;
                 u8Data4 = 0;
                 u8Data5 = 0;
                 u8Data6 = 0;
                 u8Data2 |= (MS_U8)((stUrsaRXTX_PanelParameter.m_u16Mfc_panel_htotal & 0x1F00) >> 8);
                 u8Data3 |= (MS_U8)(stUrsaRXTX_PanelParameter.m_u16Mfc_panel_htotal & 0x00FF);
                 u8Data4 |= (MS_U8)((stUrsaRXTX_PanelParameter.m_u16Mfc_panel_vtotal & 0x0F00) >> 8);
                 u8Data5 |= (MS_U8)(stUrsaRXTX_PanelParameter.m_u16Mfc_panel_vtotal & 0x00FF);
                 u8Data4 |= (MS_U8)((stUrsaRXTX_PanelParameter.m_u16Mfc_panel_hstart & 0x0700) >> 3);
                 u8Data6 |= (MS_U8)(stUrsaRXTX_PanelParameter.m_u16Mfc_panel_hstart & 0x00FF);
                 #endif

                 pCmdData[0] = 0x00;
                 pCmdData[1] = 0x11;
                 pCmdData[2] = u8Data2;
                 pCmdData[3] = u8Data3;
                 pCmdData[4] = u8Data4;
                 pCmdData[5] = u8Data5;
                 pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_3:
            {   //BMS 48 inch , 0x012 0F 00 08 70 1E
                 MS_U8 u8Data2 = 0x0F;
                 MS_U8 u8Data3 = 0x00;
                 MS_U8 u8Data4 = 0x08;
                 MS_U8 u8Data5 = 0x70;
                 MS_U8 u8Data6 = 0x1E;
                 #if (CONFIG_PANEL_INIT == 1)
                 u8Data2 = 0;
                 u8Data3 = 0;
                 u8Data4 = 0;
                 u8Data5 = 0;
                 u8Data6 = 0;
                 u8Data2 |= (MS_U8)((stUrsaRXTX_PanelParameter.m_u16Mfc_panel_width & 0x1F00) >> 8);
                 u8Data3 |= (MS_U8)(stUrsaRXTX_PanelParameter.m_u16Mfc_panel_width & 0x00FF);
                 u8Data4 |= (MS_U8)((stUrsaRXTX_PanelParameter.m_u16Mfc_panel_height & 0x0F00) >> 8);
                 u8Data5 |= (MS_U8)(stUrsaRXTX_PanelParameter.m_u16Mfc_panel_height & 0x00FF);
                 u8Data4 |= (MS_U8)((stUrsaRXTX_PanelParameter.m_u16Mfc_panel_vstart & 0x0F00) >> 4);
                 u8Data6 |= (MS_U8)(stUrsaRXTX_PanelParameter.m_u16Mfc_panel_vstart & 0x00FF);
                 #endif

                 pCmdData[0] = 0x00;
                 pCmdData[1] = 0x12;
                 pCmdData[2] = u8Data2;
                 pCmdData[3] = u8Data3;
                 pCmdData[4] = u8Data4;
                 pCmdData[5] = u8Data5;
                 pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_4:
            {   //BMS 48 inch , 0x013 41 08 78 1C 00
                 MS_U8 u8Data2 = 0x41;
                 MS_U8 u8Data3 = 0x0B;
                 MS_U8 u8Data4 = 0x78;
                 MS_U8 u8Data5 = 0x1C;
                 #if (CONFIG_PANEL_INIT == 1)
                 u8Data2 = 0;
                 u8Data3 = 0x08;
                 u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_panel_vfreq;
                 u8Data5 = 0;
                 u8Data2 |= ((stUrsaRXTX_PanelParameter.m_bMfcPanelSwapLR & 0x01) << 6);
                 u8Data2 |= ((stUrsaRXTX_PanelParameter.m_eMfc_Passive_First_Line & 0x01) << 2);
                 u8Data2 |= (stUrsaRXTX_PanelParameter.m_eMfc_Passive_Line_Order & 0x03);
                 u8Data3 |= (stUrsaRXTX_PanelParameter.m_eMfc_Sequence_Type & 0x07);
                 u8Data5 |= (stUrsaRXTX_PanelParameter.m_u8Mfc_ctrl_swing & 0x7F);
                 #endif
                 pCmdData[0] = 0x00;
                 pCmdData[1] = 0x13;
                 pCmdData[2] = u8Data2;
                 pCmdData[3] = u8Data3;
                 pCmdData[4] = u8Data4;
                 pCmdData[5] = u8Data5;
                 pCmdData[6] = 0x00;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_5_VByOne_index0:
            {   //BMS 48 inch , 0x014 00 14 14 14 14

                 MS_U8 u8Data3 = 0x00;
                 MS_U8 u8Data4 = 0x02;
                 MS_U8 u8Data5 = 0x04;
                 MS_U8 u8Data6 = 0x06;

                 #if (CONFIG_PANEL_INIT == 1)
                 u8Data3 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_0_ChanMap_0;
                 u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_0_ChanMap_1;
                 u8Data5 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_0_ChanMap_2;
                 u8Data6 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_0_ChanMap_3;
                 #endif
                 pCmdData[0] = 0x00;
                 pCmdData[1] = 0x14;
                 pCmdData[2] = 0x00;
                 pCmdData[3] = u8Data3;
                 pCmdData[4] = u8Data4;
                 pCmdData[5] = u8Data5;
                 pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_5_VByOne_index1:
            {   //BMS 48 inch , 0x014 01 0F 0E 0B 0A
                 MS_U8 u8Data3 = 0x01;
                 MS_U8 u8Data4 = 0x03;
                 MS_U8 u8Data5 = 0x05;
                 MS_U8 u8Data6 = 0x07;

                 #if (CONFIG_PANEL_INIT == 1)
                 u8Data3 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_1_ChanMap_0;
                 u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_1_ChanMap_1;
                 u8Data5 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_1_ChanMap_2;
                 u8Data6 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_1_ChanMap_3;
                 #endif

                 pCmdData[0] = 0x00;
                 pCmdData[1] = 0x14;
                 pCmdData[2] = 0x01;
                 pCmdData[3] = u8Data3;
                 pCmdData[4] = u8Data4;
                 pCmdData[5] = u8Data5;
                 pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_5_VByOne_index2:
            {   //BMS 48 inch , 0x014 02 0D 0C 09 08
                  MS_U8 u8Data3 = 0x08;
                  MS_U8 u8Data4 = 0x0A;
                  MS_U8 u8Data5 = 0x0C;
                  MS_U8 u8Data6 = 0x0E;

                  #if (CONFIG_PANEL_INIT == 1)
                  u8Data3 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_2_ChanMap_0;
                  u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_2_ChanMap_1;
                  u8Data5 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_2_ChanMap_2;
                  u8Data6 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_2_ChanMap_3;
                  #endif

                  pCmdData[0] = 0x00;
                  pCmdData[1] = 0x14;
                  pCmdData[2] = 0x02;
                  pCmdData[3] = u8Data3;
                  pCmdData[4] = u8Data4;
                  pCmdData[5] = u8Data5;
                  pCmdData[6] = u8Data6;

            }
            break;
        case UC_URSA_INIT_OUT_ALL_5_VByOne_index3:
            {   //BMS 48 inch ,0x014 03 07 06 03 02
                  MS_U8 u8Data3 = 0x09;
                  MS_U8 u8Data4 = 0x0B;
                  MS_U8 u8Data5 = 0x0D;
                  MS_U8 u8Data6 = 0x0F;

                  #if (CONFIG_PANEL_INIT == 1)
                  u8Data3 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_3_ChanMap_0;
                  u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_3_ChanMap_1;
                  u8Data5 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_3_ChanMap_2;
                  u8Data6 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_3_ChanMap_3;
                  #endif
                  pCmdData[0] = 0x00;
                  pCmdData[1] = 0x14;
                  pCmdData[2] = 0x03;
                  pCmdData[3] = u8Data3;
                  pCmdData[4] = u8Data4;
                  pCmdData[5] = u8Data5;
                  pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_5_VByOne_index4:
            {   //BMS 48 inch , 0x014 04 05 04 01 00
                  MS_U8 u8Data3 = 0x14;
                  MS_U8 u8Data4 = 0x14;
                  MS_U8 u8Data5 = 0x14;
                  MS_U8 u8Data6 = 0x14;

                  #if (CONFIG_PANEL_INIT == 1)
                  u8Data3 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_4_ChanMap_0;
                  u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_4_ChanMap_1;
                  u8Data5 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_4_ChanMap_2;
                  u8Data6 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_4_ChanMap_3;
                  #endif
                  pCmdData[0] = 0x00;
                  pCmdData[1] = 0x14;
                  pCmdData[2] = 0x04;
                  pCmdData[3] = u8Data3;
                  pCmdData[4] = u8Data4;
                  pCmdData[5] = u8Data5;
                  pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_5_VByOne_index5:
            {
                  MS_U8 u8Data3 = 0x14;
                  MS_U8 u8Data4 = 0x14;
                  MS_U8 u8Data5 = 0x14;
                  MS_U8 u8Data6 = 0x14;

                  #if (CONFIG_PANEL_INIT == 1)
                  u8Data3 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_5_ChanMap_0;
                  u8Data4 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_5_ChanMap_1;
                  u8Data5 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_5_ChanMap_2;
                  u8Data6 = stUrsaRXTX_PanelParameter.m_u8Mfc_VbyOne_5_ChanMap_3;
                  #endif
                  pCmdData[0] = 0x00;
                  pCmdData[1] = 0x14;
                  pCmdData[2] = 0x05;
                  pCmdData[3] = u8Data3;
                  pCmdData[4] = u8Data4;
                  pCmdData[5] = u8Data5;
                  pCmdData[6] = u8Data6;
            }
            break;
        case UC_URSA_INIT_OUT_ALL_6:
            {
                MS_U8 u8Data2 = 0x01;
                MS_U8 u8Data3 = 0x05;
                #if (CONFIG_PANEL_INIT == 1)
                u8Data3 = 0x08;

                u8Data2 = (stUrsaRXTX_PanelParameter.m_eMfc_panel_type & 0x1F);
                u8Data3  = (stUrsaRXTX_PanelParameter.m_eMfc_panel_channel & 0x1F);
                #endif
                pCmdData[0] = 0x00;
                pCmdData[1] = 0x15;
                pCmdData[2] = u8Data2;
                pCmdData[3] = u8Data3;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;
            }
            break;
        case UC_URSA_INIT_FINISH:
            {   //0x100 00 00 00 00 00
                pCmdData[0] = 0x01;
                pCmdData[1] = 0x00;
                pCmdData[2] = 0x00;
                pCmdData[3] = 0x00;
                pCmdData[4] = 0x00;
                pCmdData[5] = 0x00;
                pCmdData[6] = 0x00;
            }
            break;
#endif
        case UC_URSA_PANEL_ON:
            {
                MS_U16 u16OnTiming1=50, u16OnTiming2=1000;

                #if (CONFIG_PANEL_INIT == 1)
                #if defined(CONFIG_PNL_OLED_LG_SUPPORT)
                if(!IsTVOled())
                #endif
                {
                    u16OnTiming1 = GetOnTiming1();
                    u16OnTiming2 = GetOnTiming2();
                }
                #endif
                pCmdData[0] = 0x0D;
                pCmdData[1] = 0x01;
                pCmdData[2] = 0x01;//Enable
                pCmdData[3] = (MS_U8)((u16OnTiming1>>8) & 0xFF);
                pCmdData[4] = (MS_U8)((u16OnTiming1) & 0xFF);
                pCmdData[5] = (MS_U8)((u16OnTiming2>>8) & 0xFF);
                pCmdData[6] = (MS_U8)((u16OnTiming2) & 0xFF);

                #if (CONFIG_PANEL_INIT == 1)
                #if defined(CONFIG_PNL_OLED_LG_SUPPORT)
                if(IsTVOled())
                {
                    u16OnTiming2 = GetOnTiming2();
                    MsOS_DelayTask(u16OnTiming2);
                    UBOOT_DEBUG("[OLED] Delay Time after panel on (u16OnTiming2): %d",u16OnTiming2);
                }
                #endif
                #endif
            }
            break;
        case UC_URSA_PANEL_OFF:
            {
                MS_U16 u16OffTiming1=250, u16OffTiming2=50;

                #if (CONFIG_PANEL_INIT == 1)
                #if defined(CONFIG_PNL_OLED_LG_SUPPORT)
                if(!IsTVOled())
                #endif
                {
                    u16OffTiming1 = GetOffTiming1();
                    u16OffTiming2 = GetOffTiming2();
                }
                #endif

                pCmdData[0] = 0x0D;
                pCmdData[1] = 0x01;
                pCmdData[2] = 0x00; //Disable
                pCmdData[3] = (MS_U8)((u16OffTiming1>>8) & 0xFF);
                pCmdData[4] = (MS_U8)((u16OffTiming1) & 0xFF);
                pCmdData[5] = (MS_U8)((u16OffTiming2>>8) & 0xFF);
                pCmdData[6] = (MS_U8)((u16OffTiming2) & 0xFF);

                #if (CONFIG_PANEL_INIT == 1)
                #if defined(CONFIG_PNL_OLED_LG_SUPPORT)
                if(IsTVOled())
                {
                    MS_U16 u16OffTiming3 = 0;
                    u16OffTiming3 = GetOffTiming3();
                    MsOS_DelayTask(u16OffTiming3);
                }
                #endif
                #endif
            }
            break;
        default:
            bUnkown = TRUE;
            break;
    }

    if(!bUnkown)
    {
        bRet  = MDrv_Ursa_20_Write_Cmd(pCmdData);
        printf("[Ursa20]Send cmd:%d, return:%d\n", eCmdType, bRet);
    }
    else
    {
        bRet = FALSE;
        printf("[Ursa20]Can not find the command, please check!\n");
    }
    UBOOT_TRACE("OK\n");
    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Member Function
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_Ursa_20_Set_Lane_VB1_per_init(MS_U8 u8OsdLane, MS_U8 u8VideoLane)
{
    UBOOT_TRACE("IN\n");
    MS_BOOL bRet = FALSE;

#if(ENABLE_OSDC_Init==1)
    if (u8OsdLane == 4)
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_INIT_RX_OSD_4LANE);
    }
    else
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_INIT_RX_OSD_2LANE);
    }
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa20 cmd fail\n");
        return FALSE;
    }
    udelay(50);
#endif

    if (u8VideoLane == 8)
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_INIT_RX_VIDEO_8LANE);
    }
    else if (u8VideoLane == 4)
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_INIT_RX_VIDEO_4LANE);
    }
    else
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_INIT_RX_VIDEO_2LANE);
    }
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa20 cmd fail\n");
        return FALSE;
    }
    udelay(50);
    MDrv_Ursa_20_SendCmd(UC_SET_LVDS_RX_RECONFIG);
    udelay(50);
    UBOOT_TRACE("OK\n");
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa20 cmd fail\n");
        return FALSE;
    }
    return bRet;
}

MS_BOOL MDrv_Ursa_20_Set_Lane_VB1(MS_U16 u16OsdWidth, MS_U16 u16VideoWidth)
{

    UBOOT_TRACE("IN\n");
    MS_BOOL bRet = FALSE;

    if (u16VideoWidth > 1920)
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_SET_IN_VIDEO_4K2K);
    }
    else
    {
        bRet = MDrv_Ursa_20_SendCmd(UC_SET_IN_VIDEO_FULLHD);
    }
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa20 cmd fail\n");
        return FALSE;
    }
#if(ENABLE_OSDC_Init==1)
    udelay(50);
    if (u16OsdWidth > 1920)
    {
        MDrv_Ursa_20_SendCmd(UC_SET_IN_OSD_4K2K);
    }
    else
    {
        MDrv_Ursa_20_SendCmd(UC_SET_IN_OSD_FULLHD);
    }
#endif
    udelay(50);
    MDrv_Ursa_20_SendCmd(UC_SET_3D_MODE);
    udelay(50);

    MDrv_Ursa_20_SendCmd(UC_URSA_MOD_POWER_ON);
    #if(ENABLE_OSDC_Init==0)
    udelay(50);
    MDrv_Ursa_20_SendCmd(UC_URSA_DISABLE_OSD);
    #endif
    UBOOT_TRACE("OK\n");

    return bRet;

}

#if defined(CONFIG_URSA_TXRX_SUPPORT)
MS_BOOL MDrv_Ursa_20_Panel_Initialize(void)
{
    UBOOT_TRACE("IN\n");

    MS_BOOL bRet = TRUE;

    udelay(50);
    bRet = MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_1);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_2);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_3);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_4);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_5_VByOne_index0);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_5_VByOne_index1);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_5_VByOne_index2);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_5_VByOne_index3);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_5_VByOne_index4);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_OUT_ALL_6);
    udelay(50);
    bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_INIT_FINISH);
    #if defined(CONFIG_PNL_OLED_LG_SUPPORT)
    if(!IsTVOled())
    {
        udelay(50);
        bRet &= MDrv_Ursa_20_SendCmd(UC_URSA_PANEL_ON);
    }
    #endif
    UBOOT_TRACE("OK\n");
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa cmd fail\n");
    }

    return bRet;

}
#endif

MS_BOOL MDrv_Ursa_20_ReadUrsaGpio(MS_U8 u8gpioId)
{
    UBOOT_TRACE("IN\n");
    MS_BOOL bRet = FALSE;
    MS_U8 u8Data = 0;
    MS_U8 cmd[URSA_20_CMD_DATA_SUM]={0x0D, 0x12, u8gpioId, 0x00, 0x00, 0x00, 0x00};//U19:0x402, U20:0xD12

    CalCheckSum(cmd, 7);

    if(MDrv_Ursa_20_Write_Cmd(cmd) == TRUE)
    {
        udelay(50*1000);//delay 50 ms,Ursa return value need times.
        bRet = MDrv_Ursa_20_Read_Cmd(&u8Data);
    }

    UBOOT_TRACE("OK\n");
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa20 cmd fail\n");
        return FALSE;
    }
    return u8Data;

}

MS_BOOL MDrv_Ursa_20_SetUrsaGpio(MS_U8 u8gpioID, MS_BOOL bOnOff)
{
    UBOOT_TRACE("IN\n");
    MS_BOOL bRet = FALSE;
    MS_U8 cmd[URSA_20_CMD_DATA_SUM]={0x04, 0x01, u8gpioID, bOnOff, 0x00, 0x00, 0x00};

    CalCheckSum(cmd, 7);
    bRet = MDrv_Ursa_20_Write_Cmd(cmd);

    UBOOT_TRACE("OK\n");
    if(bRet != TRUE)
    {
        UBOOT_ERROR("send ursa cmd fail\n");
    }

    return bRet;
}

void MDrv_Ursa_20_SetUrsaHardReset(void)
{
    printf("\033[41;4m*****!!!Ursa Hard Reset!!!****\033[0m\n");
    URSA_HARD_RESET_ON();
    udelay(100*1000);
    URSA_HARD_RESET_OFF();
}

MS_U32 GetUrsa20VersionInfo(void)
{
    UBOOT_TRACE("IN\n");

    MS_U8 major = 0;
    MS_U8 minor = 0;
    MS_U8 minor2 = 0;
    MS_U32 u32VersionInfo = 0;
    MS_BOOL bRet =  FALSE;

    MS_U8 cmd[URSA_20_CMD_DATA_SUM] = {0x03, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00}; // the templet to get ursa version.

    cmd[2] = 0x00; // major
    if(MDrv_Ursa_20_Write_Cmd(cmd) == TRUE)
    {
        udelay(50*1000);//delay 50 ms,Ursa return value need times.
        bRet = MDrv_Ursa_20_Read_Cmd(&major);
    }

    cmd[2] = 0x01; // minor
    if(MDrv_Ursa_20_Write_Cmd(cmd) == TRUE)
    {
        udelay(50*1000);//delay 50 ms,Ursa return value need times.
        bRet = MDrv_Ursa_20_Read_Cmd(&minor);
    }

    cmd[2] = 0x04; // minor2
    if(MDrv_Ursa_20_Write_Cmd(cmd) == TRUE)
    {
        udelay(50*1000);//delay 50 ms,Ursa return value need times.
        bRet = MDrv_Ursa_20_Read_Cmd(&minor2);
    }

    u32VersionInfo = ((MS_U32)major << 16) | ((MS_U32)minor << 8) | (MS_U32)minor2;
    UBOOT_INFO("Ursa20 GetVersion major = %d, minor = %d, minor2 = %d, bRet = %d\n", major, minor, minor2, bRet);

    return u32VersionInfo;
}

int do_GetUrsa20VersionInfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    GetUrsa20VersionInfo();
    return 0;
}

U_BOOT_CMD(getUrsa20Version, CONFIG_SYS_MAXARGS, 1, do_GetUrsa20VersionInfo,
    "URSA20 Get Version\n",    NULL);

#endif //__DRV_URSA_20_C_
