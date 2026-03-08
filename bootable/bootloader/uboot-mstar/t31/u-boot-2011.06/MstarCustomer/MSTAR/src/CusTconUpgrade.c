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
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <command.h>
#include <common.h>
#include <MsDebug.h>
#include <drvIIC.h>
#include <drvGPIO.h>
#include <bootlogo/iniparser.h>
#include <MsUtility.h>
#include <MsVfs.h>
#include <CusConfig.h>
#include <idme.h>

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define BUFFER_SIZE        128
#if defined(CONFIG_MTK_BD_MT168B_10AT_19133_MT5870_M7332_ABC) || defined(CONFIG_MTK_BD_MT164B_10AT_M7632_SHELLY)
#define REGBUG_LEN         64
#else
#define REGBUG_LEN         32
#endif
#define REG_WRITE_DELAY_MS 100
#define NVM_DELAY          50 //5 seconds

#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_SHELLY)
#define COST_FORMAT_TYPE1_LEN                  42      //CS602 0x00 to 0x29 (total 42 bytes) , 0x28 = CRC[17:10], 0x29=CRC[7:0]
#define COST_GAMMA_DATA_ADDR1                  0X0FE000 //Auto Pgamma XB Flash address 0X0FE000 , Pgamma start from 0x0FE000 + 0x10
#endif

#define TCON_CONFIG_SECTION "TCON_POWER_IC_CONFIG"
#define TCON_INI_FILE "TCON_CONFIG_FILE"
#define TCON_INI_DEFVAL ""

//#define DEFAULT_VCOM_VAL 0x7E
//#define TCON_VCOM_ADDR   0x8
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Extern Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_SHELLY)
//Start CSOT spec RTPM Flow for Tconless pdf file
//G(x) = X16 + X15 + X2 + 1
//CRC16 G(x): 0x8005
const MS_U16 CSOT_CRC16_TABLE[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

MS_U16 CSOT_Cal_CRC16(MS_U8 *pData,MS_U32 len)
{
    MS_U8 dat;
    MS_U16 crcData = 0x0000;
    while(len--){
        dat = (MS_U8)(crcData>>8);
        crcData <<= 8;
        crcData ^= CSOT_CRC16_TABLE[dat^*pData++];
    }

    return crcData;
}
//End CSOT spec RTPM Flow for Tconless pdf file
#endif

static void Get_VcomValues(char *valueStr, int *Vcom1, int *Vcom2)
{
   char tmp[32];
   unsigned char  idx = 0, ct = 0;

   if (valueStr  == NULL)
       return;

   while ((valueStr[idx] != ',') && (valueStr[idx] != '\0') && (idx < 32)) {
	   tmp[idx] = valueStr[idx];
	   idx++;
   }

   //value string read from IDME is hex value without 0x prefix
   if (valueStr[idx] == '\0' ) {
	 tmp[idx] = '\0';
            *Vcom1 = (int)strtol(tmp, NULL, 16);
	 *Vcom2 = 0;
	 return;
   }

   if (valueStr[idx] == ',') {
       tmp[idx] = '\0';
       *Vcom1 = (int)strtol(tmp, NULL, 16);
       idx++; //skip ,
       ct = idx;
       while (valueStr[idx] != '\0' && idx < 32) {
          tmp[ idx-ct ] = valueStr[idx];
          idx++;
       }

       if (valueStr[idx] == '\0') {
	   tmp[idx-ct] = '\0';
           *Vcom2 = (int)strtol(tmp, NULL, 16);
       }
   }
}

#define SY76XX_NVM_WRITE_BIT  0x8
#define SY76XX_RESET          0x12
#define SY76XX_RESET_STATUS   0x10
#define MAX_LOOP_CT           10

#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_SHELLY)
#define ENV_PNL_FLASH_CRC1    "pnlFlashCrc1"
#define ENV_PNL_FLASH_CRC2    "pnlFlashCrc2"
#define ENV_PGAMMA            "pgamma"
#endif

static int wait_to_store(unsigned short i2c_addr, const char *section )
{
   int loop = 0, ret = 0;
   unsigned char regVal[1];
   unsigned char regCtrl[1] = {0};

   do {
         if (!strncmp(section, "Gamma", 5))
             mdelay(5*REG_WRITE_DELAY_MS);
         else
             mdelay(2*REG_WRITE_DELAY_MS);

         ret = MDrv_IIC_ReadBytesPort0(i2c_addr + 1, 1, regCtrl, 1, regVal);
         if (!ret) {
             UBOOT_INFO("[TCON: %s] iic read failed, ret = %d, retry\n", section, ret);
             loop++;
             continue;
         }
         else {
             UBOOT_INFO("[TCON: %s] wait for NVM write to complete\n", section);
             loop++;
         }
   } while((regVal[0] & SY76XX_NVM_WRITE_BIT) && (loop < NVM_DELAY));// check bit 4 for completion and max wait time is 5 seconds

   if (loop >= NVM_DELAY) {
       UBOOT_ERROR("[TCON: %s] NVM write time out\n", section);
       return 1;
   }
   else {
       UBOOT_INFO("[TCON: %s] NVM write completed\n", section);
       //even the status show the write is complete, it actually still not ready to reload
       //each register takes 17 ms wite to NVM so it will take about 400 ms to complete.
       mdelay(REG_WRITE_DELAY_MS);
       //PMU need to do power reset in order to output 12V to gamma. This looks like a
       //chip hardware bug
       if (!strncmp(section, "PMU", 3)){
           Panel_VCC_OFF();
           //mdrv_gpio_set_low(PAD_SAR2);
           UBOOT_INFO("[TCON: %s] NVM power off reset to ouput 12V.\n", section);
           mdelay(4*REG_WRITE_DELAY_MS);
           Panel_VCC_ON();
           //mdrv_gpio_set_high(PAD_SAR2);
       }
       return 0;
   }
}

static int reload_register_values(unsigned short i2c_addr, const char *section)
{
    unsigned char regVal[1] = {SY76XX_RESET}, value[1];
    unsigned char regCtrl[1] = {0};
    int ret = 0, loop = 0;

    UBOOT_INFO("[TCON: %s] reset to load register value from NVM\n", section);

    do {
         mdelay(REG_WRITE_DELAY_MS);
         ret = MDrv_IIC_WriteBytesPort0(i2c_addr, 1, regCtrl, 1, regVal);
         if (!ret) {
             UBOOT_INFO("[TCON: %s] iic write  error ret = %d, retry.\n", section, ret);
	     loop++;
	     continue;
	 }
    } while ((ret == 0) && (loop < MAX_LOOP_CT));

    if (loop >= MAX_LOOP_CT ) {
       UBOOT_ERROR("[TCON: %s] failed to reload registers, iic write  error , retry ret %d\n", section, ret);
       return -1;
    }

    loop = 0;
    if (!strncmp(section, "Gamma", 5)){
        mdelay(4*REG_WRITE_DELAY_MS);
        UBOOT_INFO("[TCON: %s] wait reset to complete.", section);
    }

    do {
          mdelay(REG_WRITE_DELAY_MS);
	  ret = MDrv_IIC_ReadBytesPort0(i2c_addr + 1, 1, regCtrl, 1, value);
          if (!ret) {
                 UBOOT_INFO("[TCON: %s] iic read  error ret %d, rety\n", section, ret);
		 loop++;
                 continue;
          }
          else {
               UBOOT_INFO("[TCON: %s] wait to load register values from NVM\n", section);
	       loop++;
          }
    } while((value[0] & SY76XX_RESET_STATUS) && (loop < MAX_LOOP_CT));

    if (loop >= MAX_LOOP_CT ) {
        UBOOT_ERROR("[TCON: %s] failed to reload register values\n", section);
	return 1;
    }
    else {
        UBOOT_INFO("[TCON: %s] reload register values completed\n", section);
	return 0;
    }
}

static int read_and_compare_tcon_reg(const char *Section, int *skipVCOM, unsigned char *currentVal)
{
    char Buffer[BUFFER_SIZE * 8];
    int idx, i2c_mode;
    int ret = 0;
    int length = 0;
    int uVcomReg = -1;
    int count = 0;
    int updateFlag = 0;
    const int u16AddrCnt = 1;
    unsigned short vcom_def_val = 0;
    unsigned short u16I2C_Addr;
    unsigned char pu8rBuf[REGBUG_LEN];
    unsigned char u8AddrBuf[REGBUG_LEN];
    unsigned char u8ValueBuf[REGBUG_LEN];

    length = Profile_GetInteger(Section, "Reg_Length", 0);
    UBOOT_DEBUG("[TCON: %s] Reg_Length %d\n", Section, length);
    //the sectoin may not exist
    if (length == 0 )
	return updateFlag;

    Profile_GetString(Section, "Reg_Addr_Table", "", Buffer, sizearray(Buffer));
    if (!Profile_TranslateU8Array(Buffer, length, u8AddrBuf))
        return -1;

    Profile_GetString(Section, "Reg_Value_Table", "", Buffer, sizearray(Buffer));
    if (!Profile_TranslateU8Array(Buffer, length, u8ValueBuf))
        return -1;

    u16I2C_Addr = (unsigned short)Profile_GetInteger(Section, "I2C_Addr", 0);
    UBOOT_DEBUG("[TCON: ] I2C_Addr 0x%x\n", u16I2C_Addr);

    uVcomReg = (int)Profile_GetInteger(Section, "Vcom_Addr", -1);
    UBOOT_DEBUG("[TCON: %s] Vcom_Addr %d\n", Section, uVcomReg);

    // Read, if the tcon register is not contiguous, use single byte read.
    i2c_mode = (int)Profile_GetInteger(Section, "I2C_WR_Model", 0);
    if (i2c_mode == 1){
        for ( idx=0; idx<length; idx++ ) {
           ret = MDrv_IIC_ReadBytesPort0(u16I2C_Addr + 1, u16AddrCnt, &u8AddrBuf[idx], 1, &pu8rBuf[idx]);
	   if (!ret) {
              UBOOT_ERROR("[TCON: %s] iic read error ret %d at 0x%02x\n", Section, ret, idx);
	      return -1;
	   }
	}
    }
    else {
        UBOOT_DEBUG("[TCON: ] read addr 0x%02x, length %d\n", u8AddrBuf[0], length);
        ret = MDrv_IIC_ReadBytesPort0(u16I2C_Addr + 1, u16AddrCnt, u8AddrBuf, length, pu8rBuf);
        if (!ret) {
            UBOOT_ERROR("[TCON: ] I2C read error ret %d\n", ret);
            return -1;
        }
    }
    // Check value of VCOM register. Skip VCOM REG if value is not default value.
    // TODO: this applies to TCON with only one VCOM register, some TCON has more than one VCOM register (VCOM value > 8bit)
    // or there are two VCOM values each has more then 8 bit. one way to fix this is to specify a list of vcom values in ini file
    // just as what address table and value table do. here only look at the first one.
    if (-1 != uVcomReg ) {
        vcom_def_val = (unsigned short)Profile_GetInteger(Section, "Vcom_Default_Value", 0);
        UBOOT_INFO("[TCON: %s] Vcom_Default_Value 0x%x\n", Section, vcom_def_val);
        for(count = 0; count < length; count++) {
            if ( (uVcomReg == u8AddrBuf[count]) && (pu8rBuf[count] != vcom_def_val) ) {
                UBOOT_INFO("[TCON: %s] VCOM 0x%02x:  0x%02x, default value: 0x%02x", Section, uVcomReg,
                            pu8rBuf[count], vcom_def_val);
                *skipVCOM = 1;
                UBOOT_INFO("[TCON: %s] skip vcom flag %d\n", Section, *skipVCOM);
                *currentVal = pu8rBuf[count]; //use calibrated value
                u8ValueBuf[count] = pu8rBuf[count];
                break;
            }
        }
    }

    ret = memcmp(pu8rBuf, u8ValueBuf, length);
    if (ret) {
        updateFlag = 1;
        UBOOT_INFO("Reg mismatch set updateFlag: %d Section: %s\n", updateFlag, Section);
    }

    for (idx=0; idx < length; idx++)
        UBOOT_DEBUG("[TCON: %s] check register values 0x%02x, value 0x%02x\n", Section, u8AddrBuf[idx], pu8rBuf[idx]);
    return updateFlag;
}

static int write_tcon_reg(const char *Section, int store, int skipVCOM, unsigned char currentVal, int *reboot)
{
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_SHELLY)
    int chip = 0;
    MS_U32 gamma_address = COST_GAMMA_DATA_ADDR1;
    MS_U8   rd_buffer[128] = {0};
    MS_U8   wr_buffer[128] = {0};
    MS_U16  sample_crc16 = 0, cal_crc16 = 0;
    MS_U32  u32TempSum1 = 0, u32TempSum2 = 0;
    int errorStatus = 0;
#endif
    char Buffer[BUFFER_SIZE * 8];
    int val_len;
    int length = 0;
    int ret = 0;
    int vcom1_value = 0, vcom2_value = 0;
    int count = 0;
    int uVcomReg = -1, uVcomRegLen = 0, vcom_idx = -1, i2c_mode = 0;
    int idx = 0, wp_gpio = -1, wp_level = -1;
    const int u16AddrCnt = 1;
    unsigned char pu8Addr[1];
    unsigned char pu8wBuf[1];
    unsigned char u8AddrBuf[REGBUG_LEN]   = {0};
    unsigned char u8ValueBuf[REGBUG_LEN]  = {0};
    unsigned char u8ValReload[REGBUG_LEN] = {0};
    unsigned short u16I2C_Addr;

    length = Profile_GetInteger(Section, "Reg_Length", 0);
    UBOOT_INFO("[TCON: %s] Reg_Length %d\n", Section, length);

    Profile_GetString(Section, "Reg_Addr_Table", "", Buffer, sizearray(Buffer));
    if (!Profile_TranslateU8Array(Buffer, length, u8AddrBuf))
        return -1;

    Profile_GetString(Section, "Reg_Value_Table", "", Buffer, sizearray(Buffer));
    if (!Profile_TranslateU8Array(Buffer, length, u8ValueBuf))
        return -1;

#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_SHELLY)
    //debug start
    UBOOT_DEBUG("Reg_Value_Table = {\r\n");
    for(idx = 0; idx < length; idx++)
    {
        UBOOT_DEBUG("0x%x,", u8ValueBuf[idx]);
    }
    UBOOT_DEBUG("}\r\n");
    //debug end

    //replace the value from addr 0x10 to 0x29 in CS602 case start
    chip = Profile_GetInteger(Section, "Chip", 0);
    UBOOT_INFO("[TCON: %s] Chip %d\n", Section, chip);
    if(chip == 3) //CS602
    {
        char *pcheckPgammaEnv = NULL;
        char *pcheckPanelSpiCrcEnv1 = NULL;
        char *pcheckPanelSpiCrcEnv2 = NULL;
        char strbuf[512];

        #if CONFIG_SPI_FLASH
        // mapping and getting valid data from external storage
        if (init_spi_flash() != TRUE)
        {
            UBOOT_ERROR("init_spi_flash error! CANNOT DO Pgamma data write!!!!!\n");
        }
        else
        {
            memset(rd_buffer,0,128);
            read_spi_flash(rd_buffer, gamma_address, length);
            //check data is valid or not
            for(idx=0; idx<length; idx++)  //check for all 0x00 or 0xFF case
            {
                u32TempSum1 += rd_buffer[idx];
                u32TempSum2 += rd_buffer[idx];
            }
            if((u32TempSum1 == 0) || (u32TempSum2 == length * 0xFF))
            {
                UBOOT_DEBUG("Read invalid values !!! \n");
                errorStatus = 1; //invalid values
            }
            else
            {
                sample_crc16 = (((MS_U16)rd_buffer[length-2]) << 8) + rd_buffer[length-1];
                cal_crc16 = CSOT_Cal_CRC16(rd_buffer, (length - 2));
                UBOOT_DEBUG("sample_crc16=0x%x	 cal_crc16=0x%x \n",sample_crc16, cal_crc16);
                if(sample_crc16 == cal_crc16)
                {
                    UBOOT_DEBUG("Get auto gamma data success!!! \n");
                    memcpy(wr_buffer,rd_buffer,length);
                    //overwrite PGamma values
                    for(idx = 0x10; idx <=0x29; idx++) //CS602 reg addr  PGamma 0x10~0x24, VCOM 0x25~0x27, CRC 0x28, 0x29
                    {
                        u8ValueBuf[idx] = wr_buffer[idx];
                    }

                    //boot env var check : ENV_PNL_FLASH_CRC1, ENV_PNL_FLASH_CRC2, pgamma for STR/ panel on/off use: START
                    pcheckPanelSpiCrcEnv1 = getenv(ENV_PNL_FLASH_CRC1);
                    pcheckPanelSpiCrcEnv2 = getenv(ENV_PNL_FLASH_CRC2);
                    if ((pcheckPanelSpiCrcEnv1 == NULL) || (pcheckPanelSpiCrcEnv2 == NULL))
                    {
                        UBOOT_DEBUG("Check  ENV_PNL_FLASH_CRC1, ENV_PNL_FLASH_CRC2\n");
                        //boot env no ENV_PNL_FLASH_CRC1, ENV_PNL_FLASH_CRC2 var find
                        memset(strbuf, 0, sizeof(strbuf));
                        snprintf(strbuf, sizeof(strbuf)-1, "%x", u8ValueBuf[0x28]);
                        setenv(ENV_PNL_FLASH_CRC1, strbuf);
                        memset(strbuf, 0, sizeof(strbuf));
                        snprintf(strbuf, sizeof(strbuf)-1, "%x", u8ValueBuf[0x29]);
                        setenv(ENV_PNL_FLASH_CRC2, strbuf);
                        saveenv();
                        //write panel SPI CRC  value :addr 0x28, 0x29 to boot env

                        //update pgamma to boot env
                        memset(strbuf, 0, sizeof(strbuf));
                        //CS602 reg addr  PGamma 0x10~0x24, VCOM 0x25~0x27, CRC 0x28, 0x29
                        snprintf(strbuf, sizeof(strbuf)-1, "%x,", u8ValueBuf[0x10]);
                        for(idx = 0x11; idx < 0x29; idx++)
                        {
                            snprintf(strbuf, sizeof(strbuf)-1, "%s%x,", strbuf, u8ValueBuf[idx]);
                        }
                        //addr 0x29 is last one, no ',' need
                        snprintf(strbuf, sizeof(strbuf)-1, "%s%x", strbuf, u8ValueBuf[0x29]);
                        setenv(ENV_PGAMMA, strbuf);
                        saveenv();
                    }
                    else
                    {
                        // boot env find, check spi crc and ENV_PNL_FLASH_CRC1,ENV_PNL_FLASH_CRC2 same or not
                        MS_U32  checksum1 = 0;
                        MS_U32  checksum2 = 0;
                        MS_BOOL bCrcCheckOkflag  = FALSE;
                        UBOOT_DEBUG("Check  ENV_PNL_FLASH_CRC1, ENV_PNL_FLASH_CRC2 same or not\n");
                        if((sscanf(pcheckPanelSpiCrcEnv1, "%x", &checksum1) != 0) && (sscanf(pcheckPanelSpiCrcEnv2, "%x", &checksum2) != 0))
                        {
                            UBOOT_DEBUG("checksum1 0x%x, checksum2 0x%x\n", checksum1, checksum2);
                            UBOOT_DEBUG("[0x28] 0x%x, [0x29] 0x%x\n", u8ValueBuf[0x28], u8ValueBuf[0x29]);
                            if((checksum1 == u8ValueBuf[0x28]) && (checksum2 == u8ValueBuf[0x29]))
                            {
                                bCrcCheckOkflag = TRUE;
                            }
                        }
                        UBOOT_DEBUG("bCrcCheckOkflag %d\n", bCrcCheckOkflag);
                        if(bCrcCheckOkflag == FALSE)//CRC not match , need update boot env value
                        {
                            memset(strbuf, 0, sizeof(strbuf));
                            snprintf(strbuf, sizeof(strbuf)-1, "%x", u8ValueBuf[0x28]);
                            setenv(ENV_PNL_FLASH_CRC1, strbuf);
                            memset(strbuf, 0, sizeof(strbuf));
                            snprintf(strbuf, sizeof(strbuf)-1, "%x", u8ValueBuf[0x29]);
                            setenv(ENV_PNL_FLASH_CRC2, strbuf);
                            saveenv();
                            //write panel SPI CRC  value :addr 0x28, 0x29 to boot env

                            //update pgamma to boot env
                            memset(strbuf, 0, sizeof(strbuf));
                            //CS602 reg addr  PGamma 0x10~0x24, VCOM 0x25~0x27, CRC 0x28, 0x29
                            snprintf(strbuf, sizeof(strbuf)-1, "%x,", u8ValueBuf[0x10]);
                            for(idx = 0x11; idx < 0x29; idx++)
                            {
                                snprintf(strbuf, sizeof(strbuf)-1, "%s%x,", strbuf, u8ValueBuf[idx]);
                            }
                            //addr 0x29 is last one, no ',' need
                            snprintf(strbuf, sizeof(strbuf)-1, "%s%x", strbuf, u8ValueBuf[0x29]);
                            setenv(ENV_PGAMMA, strbuf);
                            saveenv();
                        }
                        else
                        {
                            //error handle if ENV_PGAMMA not exist
                            pcheckPgammaEnv = getenv(ENV_PGAMMA);
                            if(pcheckPgammaEnv == NULL)
                            {
                                UBOOT_DEBUG("error handle \n");
                                //update pgamma to boot env
                                memset(strbuf, 0, sizeof(strbuf));
                                //CS602 reg addr  PGamma 0x10~0x24, VCOM 0x25~0x27, CRC 0x28, 0x29
                                snprintf(strbuf, sizeof(strbuf)-1, "%x,", u8ValueBuf[0x10]);
                                for(idx = 0x11; idx < 0x29; idx++)
                                {
                                    snprintf(strbuf, sizeof(strbuf)-1, "%s%x,", strbuf, u8ValueBuf[idx]);
                                }
                                //addr 0x29 is last one, no ',' need
                                snprintf(strbuf, sizeof(strbuf)-1, "%s%x", strbuf, u8ValueBuf[0x29]);
                                setenv(ENV_PGAMMA, strbuf);
                                saveenv();
                            }
                        }
                    }
                    //boot env var check : ENV_PNL_FLASH_CRC1, ENV_PNL_FLASH_CRC2, pgamma for STR/ panel on/off use: END

                    //debug start
                    UBOOT_DEBUG("wr_buffer = {\r\n");
                    for(idx = 0x00; idx < length; idx++)
                    {
                        UBOOT_DEBUG("0x%x,", wr_buffer[idx]);
                    }
                    UBOOT_DEBUG("}\r\n");
                    UBOOT_DEBUG("===================\r\n");
                    UBOOT_DEBUG("Pgamma change 0x10~0x29 result:\r\n");
                    UBOOT_DEBUG("u8ValueBuf = {\r\n");
                    for(idx = 0x00; idx < length; idx++)
                    {
                        UBOOT_DEBUG("0x%x,", u8ValueBuf[idx]);
                    }
                    UBOOT_DEBUG("}\r\n");
                    //debug end
                }
                else
                {
                    UBOOT_DEBUG("Auto Pgamma data is invalid!!! \n");
                    errorStatus = 2; //CRC check result different
                }
            }
            UBOOT_INFO("Pgamma overwrite result %d (OK=0, NG !=0)\n", errorStatus);
        }
        #else
        UBOOT_ERROR("need CONFIG_SPI_FLASH! CANNOT DO Pgamma data write!!!!!\r\n\n\n");
        #endif
    }
    //replace the value from addr 0x10 to 0x29 in CS602 case end
#endif

    uVcomReg = (int)Profile_GetInteger(Section, "Vcom_Addr", -1);
    UBOOT_DEBUG("[TCON: %s] Vcom_Addr %d\n", Section, uVcomReg);
    //validate vcom register address
    for (count = 0 ; count < length ; count++) {
        if ( uVcomReg == u8AddrBuf[count] ) {
            UBOOT_INFO("[TCON: %s] reg 0x%02x (VCOM) is 0x%02x.\n", Section, uVcomReg, currentVal);
            vcom_idx = count;
            break;
        }
    }

    u16I2C_Addr = (unsigned short)Profile_GetInteger(Section, "I2C_Addr", 0);
    UBOOT_DEBUG("[TCON: ] I2C_Addr 0x%x\n", u16I2C_Addr);
    // Check VCOM is default.
    if ( 1 == skipVCOM ) {
       if ( vcom_idx > -1 ){
            u8ValueBuf[vcom_idx] = currentVal;
       }
    }
    else {
       long eeprom = Profile_GetInteger(Section, "Write_Reg_Boot_On", -1l);
       UBOOT_INFO("[TCON: %s] power on register write check %ld\n", Section, eeprom);

       // for all non-EEPROM TCONs, currentVal will be 0 and skipVCOM will be 0 then check IDME
       if ( (vcom_idx > -1) && (eeprom == 1l) ) {
           uVcomRegLen = (int)Profile_GetInteger(Section, "Vcom_Value_Len", 0);
           UBOOT_INFO("[TCON: %s] Vcom Register value length %d\n", Section, uVcomRegLen);
           val_len = Profile_GetString(Section, "Vcom_Idme_Name", "", Buffer, sizearray(Buffer));
           if ( val_len && (uVcomRegLen != 0) ) {
               char value[32];
               if ( 0 == idme_get_var_external(Buffer, value, 32) ) {
                   UBOOT_INFO("[TCON: %s] Vcom string from IDME: %s\n", Section, value);
                   Get_VcomValues(value, &vcom1_value, &vcom2_value);
                   if ( (vcom1_value > 0) || (vcom2_value > 0)) {
                       if (uVcomRegLen == 8) { //not any case for now
                           u8ValueBuf[vcom_idx] = vcom1_value;
                           u8ValueBuf[vcom_idx +1] = vcom2_value;
                       }
                       else if ((uVcomRegLen > 8) && (uVcomRegLen <= 12)){                                    //RT6966, maximal VOCM lenght is 12 bits
                           u8ValueBuf[vcom_idx]   = (unsigned char)(vcom1_value >> 4);                        //bit [11:4] for VCOM1
                           u8ValueBuf[vcom_idx +1 ] = (unsigned char)((vcom1_value << 4) | (vcom2_value >> 8)); //bit [3:0] for VCOM1 and [11:8] for VCOM2
                           u8ValueBuf[vcom_idx + 2] = (unsigned char)(vcom2_value & 0xFF) ;                     //bit [7:0] for VCOM2
                           UBOOT_INFO("[TCON: %s] Vcom Register values %d, %d, %d\n", Section, u8ValueBuf[vcom_idx], u8ValueBuf[vcom_idx+1],u8ValueBuf[vcom_idx+2]);
                       }
                   }
                   else
                       UBOOT_ERROR("[TCON: %s] VCOM values are zeros!\n", Section);
               }
               else
                   UBOOT_ERROR("[TCON: %s] VCOM1 value from IDME not found !\n", Section);
           }
           else
               UBOOT_ERROR("[TCON: %s] VCOM value length is wrong or the name is not found in IDME!\n", Section);
       }
    }
    // gpio is already initialized and the default status is output
    if ( (wp_gpio = Profile_GetInteger(Section, "WP_Pin", -1)) >= 0 ) {
        wp_level = Profile_GetInteger(Section, "WP_Level", -1);
        wp_gpio++; //seems that MTk has two schema and the interanl one starts from 1
        if ( wp_level == 1)
            mdrv_gpio_set_low(wp_gpio);
        else
            mdrv_gpio_set_high(wp_gpio);
        UBOOT_INFO("[TCON: %s] disable TCON register write protection WP pin: %d level:%d\n", Section, wp_gpio, mdrv_gpio_get_level(wp_gpio));
    }
    // there are two different modes for I2c write, multiple or single
    // regiter address has to be contiguous in order to use multiple bytes write, otherwise use
    // single byte.
    UBOOT_INFO("[TCON: %s] write addr 0x%02x, length %d\n", Section, u8AddrBuf[0], length);
    i2c_mode = (int)Profile_GetInteger(Section, "I2C_WR_Model", 0);
    if (i2c_mode == 1){
        for ( idx=0; idx<length; idx++) {
           UBOOT_DEBUG("[TCON: %s] iic write register 0x%02x, value 0x%02x\n", Section, u8AddrBuf[idx], u8ValueBuf[idx]);
           ret = MDrv_IIC_WriteBytesPort0(u16I2C_Addr, u16AddrCnt, &u8AddrBuf[idx], 1, &u8ValueBuf[idx]);
	   if (!ret) {
              UBOOT_ERROR("[TCON: %s] iic write error ret %d\n", Section, ret);
	      return -1;
	   }
	}
    }
    else
       ret = MDrv_IIC_WriteBytesPort0(u16I2C_Addr, u16AddrCnt, u8AddrBuf, length, u8ValueBuf); //multiple bytes write

    // Workaroud for SY7687
    val_len = Profile_GetString(Section, "Name", "", Buffer, sizearray(Buffer));
    if(!strncmp(Buffer, "SY7687", sizeof(Buffer))) {
        UBOOT_DEBUG("[TCON: %s] Name %s\n", Section, Buffer);
        ret = MDrv_IIC_WriteBytesPort0(u16I2C_Addr, u16AddrCnt, u8AddrBuf+1, length-1, u8ValueBuf+1);
    }
    if (!ret) {
        UBOOT_ERROR("[TCON: %s] iic write error ret %d\n", Section, ret);
        return -1;
    }

    // some TCON does not have interanl EEPROM to store DAC register settings and
    // the Write_Cmd_Addr will be set to -1 in this case
    if (store) {
        pu8Addr[0] = Profile_GetInteger(Section, "Write_Cmd_Addr",  0);
        pu8wBuf[0] = Profile_GetInteger(Section, "Write_Cmd_Value", 0);
        if ( pu8wBuf[0] != 0x0  ) {
            UBOOT_INFO("[TCON: %s] commit register write  0x%02x, data 0x%02x\n", Section, pu8Addr[0], pu8wBuf[0]);
            ret = MDrv_IIC_WriteBytesPort0(u16I2C_Addr, u16AddrCnt, pu8Addr, 1, pu8wBuf);
            if (!ret) {
                UBOOT_ERROR("[TCON: %s] iic write error ret %d\n", Section, ret);
                return -1;
            }
        }

        //wait for NVM write to completed.
        if( (!strncmp(Buffer, "SY7634PRC", sizeof(Buffer))) || (!strncmp(Buffer, "SY7687QEC", sizeof(Buffer))) ) {
            if (0 !=  wait_to_store(u16I2C_Addr, Section )) {
                UBOOT_ERROR("[TCON: %s] failed to write register values to NVM %d\n", Section, ret);
                return -1;
            }
            else {
                ret = reload_register_values(u16I2C_Addr, Section);
                if (ret != 0 ) {
                UBOOT_ERROR("[TCON: %s] failed to reload register values %d\n", Section, ret);
                return -1;
                }
                else {
                    if (i2c_mode == 1) {
                        for ( idx=0; idx<length; idx++) {
                            ret = MDrv_IIC_ReadBytesPort0(u16I2C_Addr + 1, u16AddrCnt, &u8AddrBuf[idx], 1, &u8ValReload[idx]);
                            if (!ret) {
                            UBOOT_ERROR("[TCON: %s] iic read error ret %d\n", Section, ret);
                            return -1;
                            }
                        }
                    }
                    else {
                        ret = MDrv_IIC_ReadBytesPort0(u16I2C_Addr + 1, u16AddrCnt, u8AddrBuf, length, u8ValReload);
                        if (!ret) {
                        UBOOT_ERROR("[TCON: %s] fail to read back regs values\n", Section);
                        return -1;
                        }
                    }
                }
                for (idx=0; idx < length; idx++)
                UBOOT_DEBUG("[TCON: %s] read back reg: 0x%02x, value: 0x%02x, expect: 0x%02x\n", Section, u8AddrBuf[idx], u8ValReload[idx], u8ValueBuf[idx]);

                ret = memcmp(u8ValReload, u8ValueBuf, length);
                if (ret) {
                    UBOOT_ERROR("[TCON: %s] register values do not match, ret %d\n", Section, ret);
                    return -1;
                }
                else
                    UBOOT_INFO("[TCON: %s] register values update completed", Section);
                *reboot = 0; //no need to reboot for new 50 inch model, instead using TCON reset.
            }
        }
        else
            *reboot = 1; //need reboot for old models
    }

    if ( wp_gpio > 0 ) {
        if ( wp_level == 1)
            mdrv_gpio_set_high(wp_gpio);
        else
            mdrv_gpio_set_low(wp_gpio);
        UBOOT_DEBUG("[TCON: %s] enable TCON register write protection, WP pin:%d, level:%d\n", Section, wp_gpio, mdrv_gpio_get_level(wp_gpio));
    }
    return 0;
}

int do_tcon_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("TCON: ] IN\n");

    char *script = NULL;
    char *Customer_ini_file = NULL;
    char InPath[BUFFER_SIZE]="\0";
    unsigned int filesize;
    char *s_tmp;
    long ac_on_force = 0l; //program TCON register at AC on
    int ret;
    int updateFlag = 0;
    int skipVCOM = 0;
    int ePort = 0, reboot = 0;
    unsigned char currentVal = 0;

    // Load Customer_1 ini file.
    Customer_ini_file = getenv ("customer_ini_path");
    // remove partition name from path
    s_tmp = Customer_ini_file;
    while (*s_tmp == '/') s_tmp++;
    s_tmp = strstr((const char*)s_tmp,"/");
    UBOOT_DEBUG("[TCON:] customer_ini file = %s\n", s_tmp);

    if (s_tmp != NULL) {
        script = (char *)loadscript(s_tmp, &filesize);
        if (script == NULL) {
            // mount tvconfig
            UBOOT_DEBUG("[TCON: ] mount %s\n", CONFIG);
            if(vfs_mount(CONFIG)!= 0) {
                UBOOT_ERROR("[TCON: ] mount %s fail\n", CONFIG);
                return 1;
            }
            script = (char *)loadscript(s_tmp, &filesize);
        }
    }

    if (script == NULL) {
        UBOOT_ERROR("[TCON: ] Unable to load %s\n", s_tmp);
        return 1;
    }

    // Get TCON ini file path
    Profile_Init((char *)script, filesize);
    if (Profile_GetString(TCON_CONFIG_SECTION, TCON_INI_FILE, TCON_INI_DEFVAL, InPath, sizearray(InPath))) {
        // Load TCON ini file
        UBOOT_DEBUG("[TCON: ] INI FILE PATH:%s\n", InPath);
        script = (char *)loadscript(InPath, &filesize);
    }
    else {
        UBOOT_ERROR("[TCON: ] Unable to find [%s] %s\n", TCON_CONFIG_SECTION, TCON_INI_FILE);
        return 1;
    }

    if(script != NULL) {
        Profile_Init((char *)script,filesize);

        ePort = Profile_GetInteger("IIC", "IIC_Port", E_HWI2C_PORT2_1);

        UBOOT_DEBUG("[TCON: IIC] Init IIC port %d\n", ePort);
        MDrv_IIC_InitPort0(ePort);

        ac_on_force = Profile_GetInteger("PMU", "Write_Reg_Boot_On", 0l);
        UBOOT_INFO("[TCON: PMU] program TCON register at AC on %ld\n", ac_on_force);
        //set ac_on_force to 1 for those TCONs without interanl EEPROM
        //for which register vaules have to be loaded from ini file
        //everytime after a power cycle.
        if (ac_on_force != 1l) {
            ret = read_and_compare_tcon_reg("PMU", &skipVCOM, &currentVal);
            if (ret == 1) {
                if (write_tcon_reg("PMU", 1, skipVCOM, currentVal, &reboot)) {
                    UBOOT_ERROR("[TCON: PMU] failed to update regs.\n");
                }
                else
                {
                    updateFlag |= reboot;
                    UBOOT_INFO("[TCON: PMU] regs are updated.\n");
                }
            }
            else if (ret < 0) {
                UBOOT_ERROR("[TCON: PMU] Failed to read tcon Gamma reg.\n");
            }

            ret = read_and_compare_tcon_reg("Gamma", &skipVCOM, &currentVal);
            if (ret == 1) {
                if (write_tcon_reg("Gamma", 1, skipVCOM, currentVal, &reboot)){
                UBOOT_ERROR("[TCON: Gamma] failed to update regs.\n");
                }
                else {
                updateFlag |= reboot;
                UBOOT_INFO("[TCON: Gamma] regs are updated.\n");
                }
            }
            else if (ret < 0) {
                UBOOT_ERROR("[TCON: Gamma] Failed to read tcon PMU reg.\n");
            }

            ret = read_and_compare_tcon_reg("Level_Shift", &skipVCOM, &currentVal);
            if (ret == 1) {
                if (write_tcon_reg("Level_Shift", 1, skipVCOM, currentVal, &reboot)){
                    UBOOT_ERROR("[TCON: Level_Shfit] failed to update regs.\n");
                }
                else {
                    updateFlag |= reboot;
                    UBOOT_INFO("[TCON: Level_Shift] regs are updated.\n");
                }
            }
            else if (ret < 0) {
                UBOOT_ERROR("[TCON: Level_Shift] Failed to update tcon Level_Shift reg.\n");
            }
        }
        else {
            //force to update all regsiters for TCON without internal EEPROM
            skipVCOM = 0;
            if (write_tcon_reg("PMU", 0, skipVCOM, currentVal, &reboot)) {
                UBOOT_ERROR("[TCON: PMU] failed to update regs.\n");
            }
            else
                UBOOT_INFO("[TCON: PMU] regs are updated.\n");

            if (write_tcon_reg("Gamma", 0, skipVCOM, currentVal, &reboot)){
                UBOOT_ERROR("[TCON: Gamma] failed to update regs.\n");
            }
            else
                UBOOT_INFO("[TCON: Gamma] regs are updated.\n");

            if (write_tcon_reg("Level_Shift", 0, skipVCOM, currentVal, &reboot)){
                UBOOT_ERROR("[TCON: Level_Shfit] failed to update regs.\n");
            }
            else
                UBOOT_INFO("[TCON: Level_Shift] regs are updated.\n");
        }
        // can't reset the system for RT6966 for it will lose all the register settings
        if ( (ac_on_force != 1l) && (updateFlag) )
            run_command("reset",0);
    }
    else {
        UBOOT_ERROR("[TCON: ] failed to load tcon ini file\n");
        return 1;
    }

    UBOOT_TRACE("[TCON: ] OK\n");
    return 0;
}
