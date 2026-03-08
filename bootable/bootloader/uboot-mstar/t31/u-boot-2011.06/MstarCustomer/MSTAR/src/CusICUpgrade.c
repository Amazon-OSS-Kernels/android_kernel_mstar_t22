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
#include <common.h>
#include <malloc.h>
#include <MsTypes.h>
#include <MsSystem.h>
#include <MsUtility.h>
#include <MsDebug.h>
#include <MsVfs.h>
#include <MsBoot.h>
#include <MsAppCB.h>
#include <MsOS.h>
#include <apiSWI2C.h>
#include <CusICUpgrade.h>
#include <CusConfig.h>
#include <bootlogo/iniparser.h>
#include "MsEeprom.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define CUS_BUFFER_SIZE 256
#define PMIC_I2C_BUS_NUM        (1)

static SWI2C_BusCfg Pmic_I2cBusCfg[PMIC_I2C_BUS_NUM]=
{
    // Bus-0
    {PAD_DDCR_CK, PAD_DDCR_DA, 100}, //IS_SW_I2C  /SCL_PAD /SDA_PAD /Delay

    // Bus-1
    //{PAD_GPIO28, PAD_GPIO29, 100}, //IS_SW_I2C  /SCL_PAD /SDA_PAD /Delay

    // Others, add other bus here
};

typedef struct IC_INFO_s
{
    int Bus;
    int Addr;
    int Offset;
    int Size;
    int BurnOffset;
    int BurnCmd;
    int ReadMode;
    int ControlReg;
    int ControlRegData;
    int NotCompareLen;
    int PanelVcc;
    int DelayTime;
    int WPNum;
    int WPPOL;
}IC_INFO_t;

MS_U8 g_bus = 0;
MS_U8 g_i2c_slave_addr = 0x0;
MS_U8 g_i2c_slave_addr_mode = 0x0;
char gPMICBinPath[BUFFER_SIZE];
char gPGAMMAICBinPath[BUFFER_SIZE];

//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------
static MS_BOOL SWI2C_WriteDatas(MS_U8 *u8RegOffset, MS_U8 addrSize, MS_U8 *pdatabuf, MS_U32 dataSize)
{
    MS_U16 u16BusNumSlaveID = (g_bus << 8) | (g_i2c_slave_addr);
    MS_BOOL bRet = FALSE;

    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("u16BusNumSlaveID:%x\n",u16BusNumSlaveID);
    bRet = MApi_SWI2C_WriteBytes(u16BusNumSlaveID,addrSize,u8RegOffset,(MS_U16)dataSize,pdatabuf);
    if(bRet == FALSE)
    {
        UBOOT_ERROR("Bus %d IIC write data failure.\n ",g_bus);
        return FALSE;
    }
    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL SWI2C_ReadDatas(MS_U8 *u8RegOffset, MS_U8 addrSize, MS_U8 *pdatabuf, MS_U32 dataSize)
{
    MS_U16 u16BusNumSlaveID = (g_bus << 8) | (g_i2c_slave_addr) | (g_i2c_slave_addr_mode);
    MS_BOOL bRet = FALSE;

    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("u16BusNumSlaveID:%x\n",u16BusNumSlaveID);
    bRet = MApi_SWI2C_ReadBytes(u16BusNumSlaveID,addrSize,u8RegOffset,(MS_U16)dataSize,pdatabuf);
    if(bRet == FALSE)
    {
        UBOOT_ERROR("Bus %d IIC read data failure.\n ",g_bus);
        return FALSE;
    }
    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL do_ic_register_read(IC_INFO_t *sIC, unsigned char *memaddr)
{
    MS_U8 u8RegOffset = sIC->Offset;
    MS_U32 u32Size = sIC->Size;
    MS_U8 u8ReadControlData[2];
    unsigned char *icRegData;
    MS_BOOL bRet = FALSE;
    MS_U8 u8Index = 0;

    UBOOT_TRACE("IN\n");
    g_bus = sIC->Bus;
    if(sIC->Addr != 0)
    {
        g_i2c_slave_addr = sIC->Addr;
        UBOOT_DEBUG("g_i2c_slave_addr:%x\n",g_i2c_slave_addr);
    }
    else
    {
        UBOOT_ERROR("g_i2c_slave_addr is zero!!");
        return FALSE;
    }

    if(u32Size == 0)
    {
        UBOOT_ERROR("Write data size is zero!!\n");
        return FALSE;
    }
    else
    {
        UBOOT_DEBUG("Write data size:%x\n",u32Size);
    }

    icRegData = (unsigned char *)malloc(u32Size);
    memset(icRegData, 0, sizeof(u32Size));
    if(sIC->ReadMode == 1 || sIC->ReadMode == 2 || sIC->ReadMode == 3)
    {
        if(sIC->ReadMode == 1 || sIC->ReadMode == 3)
        {
            u8ReadControlData[0] = sIC->ControlReg;
            u8ReadControlData[1] = sIC->ControlRegData;
            UBOOT_DEBUG("Control reg:%x Control data:%x\n",u8ReadControlData[0],u8ReadControlData[1]);
            bRet = SWI2C_WriteDatas(&u8RegOffset,0,u8ReadControlData,2);
            if(bRet == FALSE)
            {
                UBOOT_ERROR("SWI2C_ReadDatas execution failure. Please check HW or IIC setting.\n");
                goto READ_END;
            }
        }

        bRet = SWI2C_WriteDatas(&u8RegOffset,0,&u8RegOffset,1);
        if(bRet == FALSE)
        {
            UBOOT_ERROR("SWI2C_ReadDatas execution failure. Please check HW or IIC setting.\n");
            goto READ_END;
        }

        if(sIC->ReadMode == 1 || sIC->ReadMode == 2)
        {
            g_i2c_slave_addr_mode = 0x1;
            UBOOT_DEBUG("g_i2c_slave_addr_mode:%x\n",g_i2c_slave_addr_mode);
        }

        bRet = SWI2C_ReadDatas(&u8RegOffset,0,icRegData,u32Size);
        if(bRet ==FALSE)
        {
            UBOOT_ERROR("SWI2C_ReadDatas execution failure. Please check HW or IIC setting.\n");
            goto READ_END;
        }
    }
    else
    {
        UBOOT_DEBUG("IC Read Mode is not correct:%d\n",sIC->ReadMode);
        goto READ_END;
    }

    for(u8Index = 0; u8Index < (u32Size - sIC->NotCompareLen); u8Index++)
    {
        if(icRegData[u8Index] != memaddr[u8Index])
        {
            free(icRegData);
            return TRUE;
        }
    }

READ_END:
    free(icRegData);
    UBOOT_TRACE("OK\n");
    return FALSE;
}

static MS_BOOL do_pm_ic_upgrade(IC_INFO_t *sPMIC, MS_U8 *pu8Buffer)
{
    MS_U8 u8RegOffset = sPMIC->Offset;
    MS_U32 u32Size = sPMIC->Size;
    MS_BOOL bRet;

    UBOOT_TRACE("IN\n");
    g_bus = sPMIC->Bus;
    if(sPMIC->Addr != 0)
    {
        g_i2c_slave_addr = sPMIC->Addr;
        UBOOT_DEBUG("g_i2c_slave_addr:%x\n",g_i2c_slave_addr);
    }
    else
    {
        UBOOT_ERROR("g_i2c_slave_addr is zero!!");
        return FALSE;
    }

    if(u32Size == 0)
    {
        UBOOT_ERROR("Write data size is zero!!\n");
        return FALSE;
    }
    else
    {
        UBOOT_DEBUG("Write data size:%x\n",u32Size);
    }

    bRet = SWI2C_WriteDatas(&u8RegOffset,1,pu8Buffer,u32Size);
    if(bRet == FALSE)
    {
        UBOOT_ERROR("SWI2C_WriteDatas execution failure. Please check HW or IIC setting.\n");
        return FALSE;
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL do_pm_ic_burn(IC_INFO_t *sIC)
{
    MS_U8 u8WriteEEPROM = sIC->BurnCmd;
    MS_U8 u8RegOffset = sIC->BurnOffset;
    MS_U32 DelayTime = sIC->DelayTime;
    MS_BOOL bRet;

    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("BurnOffset:%x BurnCmd:%x\n",u8RegOffset,u8WriteEEPROM);
    bRet = SWI2C_WriteDatas(&u8RegOffset,1,&u8WriteEEPROM,1);
    if(bRet == FALSE)
    {
        UBOOT_ERROR("SWI2C_WriteDatas execution failure. Please check HW or IIC setting.\n");
        return FALSE;
    }

    UBOOT_DEBUG("PanelVcc:%x ;DelayTime:%d\n",sIC->PanelVcc,DelayTime);
    if(sIC->PanelVcc)
    {
        mdelay(DelayTime);
        PANEL_CTL_Off();
        mdelay(DelayTime);
        PANEL_CTL_On();
        mdelay(DelayTime);
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL do_ic_burn(IC_INFO_t *sIC)
{
    MS_U8 u8WriteEEPROM = sIC->BurnCmd;
    MS_U8 u8RegOffset = sIC->BurnOffset;
    MS_U32 DelayTime = sIC->DelayTime;
    MS_BOOL bRet;

    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("BurnOffset:%x BurnCmd:%x\n",u8RegOffset,u8WriteEEPROM);
    bRet = SWI2C_WriteDatas(&u8RegOffset,1,&u8WriteEEPROM,1);
    if(bRet == FALSE)
    {
        UBOOT_ERROR("SWI2C_WriteDatas execution failure. Please check HW or IIC setting.\n");
        return FALSE;
    }

    UBOOT_DEBUG("PanelVcc:%x ;DelayTime:%d\n",sIC->PanelVcc,DelayTime);
    if(sIC->PanelVcc)
    {
        mdelay(DelayTime);
        PANEL_CTL_Off();
        mdelay(DelayTime);
        PANEL_CTL_On();
        mdelay(DelayTime);
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL do_pgamma_ic_upgrade(IC_INFO_t *sPGAMMAIC, MS_U8 *pu8Buffer)
{
    UBOOT_INFO("Start upgrade PGAMMA IC bin file!~\n");
    MS_U8 u8RegOffset = sPGAMMAIC->Offset;
    MS_U8 u8WriteBit = 0x1;
    MS_U32 u32Size = sPGAMMAIC->Size-26;
    MS_BOOL bRet;

    g_bus = sPGAMMAIC->Bus;
    if(sPGAMMAIC->Addr != 0)
    {
        g_i2c_slave_addr = sPGAMMAIC->Addr;
        UBOOT_DEBUG("g_i2c_slave_addr:%x\n",g_i2c_slave_addr);
    }
    else
    {
        UBOOT_ERROR("g_i2c_slave_addr is zero!!");
        return FALSE;
    }

    if(u32Size == 0){
        UBOOT_ERROR("Write data size is zero!!\n");
        return FALSE;
    }
    else
    {
        UBOOT_DEBUG("Write data size:%x\n",u32Size);
    }

    pu8Buffer[u32Size-2] |= (u8WriteBit<<6);
    bRet = SWI2C_WriteDatas(&u8RegOffset,1,pu8Buffer,u32Size);
    if(bRet == FALSE)
    {
        UBOOT_ERROR("SWI2C_WriteDatas execution failure. Please check HW or IIC setting.\n");
        return FALSE;
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL Read_FlashDataToAddr(IC_INFO_t *sICInfo, U32 *pu32Addr, char *BinPath)
{
    UBOOT_TRACE("IN\n");

    vfs_mount(CONFIG);
    if(sICInfo->Size == 0)
    {
        UBOOT_ERROR("vfs_getsize error: %s !\n", BinPath);
        return FALSE;
    }

    if(pu32Addr == NULL)
    {
        UBOOT_ERROR("malloc pu32Addr memory error !");
        return FALSE;
    }
    memset((void*)pu32Addr,0,sICInfo->Size);

    if (vfs_read((void *)pu32Addr,BinPath,0,sICInfo->Size) != 0)
    {
        UBOOT_ERROR("vfs_read data fail ...>>>\n");
        return FALSE;
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}

static MS_BOOL Read_ICBinFileToMem(IC_INFO_t *sICInfo, U32 *pu32Addr, char *BinPath)
{
    MS_BOOL bret = 0;
    UBOOT_TRACE("IN\n");
    bret = Read_FlashDataToAddr(sICInfo, pu32Addr, BinPath);
    UBOOT_TRACE("OK\n");
    return bret;
}

unsigned char* _load_ICBinFile(IC_INFO_t *sICInfo, char *BinPath)
{
    void* Dram_Addr = NULL;

    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("[S] BinPath:%s\n",BinPath);
    Dram_Addr=malloc(sICInfo->Size);
    if(Dram_Addr==NULL)
    {
        UBOOT_ERROR("malloc fail!!\n");
        return NULL;
    }
    if(Read_ICBinFileToMem(sICInfo, (U32)Dram_Addr, BinPath) == FALSE)//read logo from spi/emmc to dram
    {
        UBOOT_ERROR("Error: Read_PMICAddr Fail\n");
        free((void*)Dram_Addr);

        return NULL;
    }
    UBOOT_TRACE("OK\n");

    return (unsigned char*)Dram_Addr;
}

static MS_BOOL parse_customer_profile(char* path,IC_INFO_t *sPMIC, IC_INFO_t *sPGAMMAIC)
{
    char **ptr;
    char str[BUFFER_SIZE];
    char *script = NULL;
    U32 filesize = 0;

    UBOOT_TRACE("IN %s\n",CONFIG);
    vfs_mount(CONFIG);
    script = loadscript(path,&filesize);
    if(script == NULL)
    {
        while (*path== '/') path++;
        path=strstr(path,"/");
        UBOOT_DEBUG("[S] filter /cusdata or /config, and find file again!!\n");
        script = loadscript(path,&filesize);
        if(script == NULL)
        {
            while(*path == '/') path++;
            path=strstr(path,"/");
            UBOOT_DEBUG("filter /cusdata or /config, and find file again!!\n");
            script = loadscript(path,&filesize);
            if(script == NULL)
            {
                return FALSE;
            }
        }
    }
    UBOOT_DEBUG("[S] sizeof( model ini script ) --------> %u bytes\n",filesize);
    Profile_Init((char *)script,filesize);

    if(Profile_GetString("TCON_BIN", "PMIC_FILE","", str, sizearray(str)))
    {
        UBOOT_TRACE("PMIC start\n");
        char* tmpPMICBinPath;
        tmpPMICBinPath=str;
        if(FALSE==vfs_getsize(tmpPMICBinPath))
        {
            while (*tmpPMICBinPath== '/') tmpPMICBinPath++;
            tmpPMICBinPath=strstr(tmpPMICBinPath,"/");
            UBOOT_ERROR("filter /cusdata or /config, and find file again!!\n");
            UBOOT_ERROR("tmpPMICBinPath:%s\n",tmpPMICBinPath);

            if(FALSE==vfs_getsize(tmpPMICBinPath))
            {
                while (*tmpPMICBinPath== '/') tmpPMICBinPath++;
                tmpPMICBinPath=strstr(tmpPMICBinPath,"/");
                UBOOT_DEBUG("filter /cusdata or /config, and find file again!!\n");
                UBOOT_DEBUG("tmpPMICBinPath:%s\n",tmpPMICBinPath);
                if(FALSE==vfs_getsize(tmpPMICBinPath))
                {
                    tmpPMICBinPath=str;
                }
            }
        }
        snprintf(gPMICBinPath,BUFFER_SIZE,"%s",tmpPMICBinPath);
        sPMIC->Size = vfs_getsize(gPMICBinPath);
        UBOOT_DEBUG("PMIC path --> %s\n",gPMICBinPath);
        UBOOT_DEBUG("PMIC size:%x\n",sPMIC->Size);
        if(sPMIC->Size != 0)
        {
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_bus","", str, sizearray(str)))
            {
                sPMIC->Bus = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->Bus:%x\n",sPMIC->Bus);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_addr","", str, sizearray(str)))
            {
                sPMIC->Addr = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->Addr:%x\n",sPMIC->Addr);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_offset","", str, sizearray(str)))
            {
                sPMIC->Offset = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->Offset:%x\n",sPMIC->Offset);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_BURN_offset","", str, sizearray(str)))
            {
                sPMIC->BurnOffset = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->BurnOffset:%x\n",sPMIC->BurnOffset);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_BURN_cmd","", str, sizearray(str)))
            {
                sPMIC->BurnCmd = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->BurnCmd:%x\n",sPMIC->BurnCmd);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_READ_MODE","", str, sizearray(str)))
            {
                sPMIC->ReadMode = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->ReadMode:%x\n",sPMIC->ReadMode);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_CONTROL_REG","", str, sizearray(str)))
            {
                sPMIC->ControlReg = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->ControlReg:%x\n",sPMIC->ControlReg);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_CONTROL_REG_DATA","", str, sizearray(str)))
            {
                sPMIC->ControlRegData = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->ControlRegData:%x\n",sPMIC->ControlRegData);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_NOT_COMPARE_LEN","", str, sizearray(str)))
            {
                sPMIC->NotCompareLen = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->NotCompareLen:%x\n",sPMIC->NotCompareLen);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_TOGGLE_PANEL_VCC_FLAG","", str, sizearray(str)))
            {
                sPMIC->PanelVcc = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->PanelVcc:%x\n",sPMIC->PanelVcc);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_TOGGLE_PANEL_VCC_DELAY","", str, sizearray(str)))
            {
                sPMIC->DelayTime = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->DelayTime:%x\n",sPMIC->DelayTime);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_WP_NUM","", str, sizearray(str)))
            {
                sPMIC->WPNum = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->WPNum:%x\n",sPMIC->WPNum);
            }
            if(Profile_GetString("TCON_BIN", "PMIC_I2C_WP_POL","", str, sizearray(str)))
            {
                sPMIC->WPPOL = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPMIC->WPPOL:%x\n",sPMIC->WPPOL);
            }
        }
    }

    if(Profile_GetString("TCON_BIN", "Pgamma_FILE","", str, sizearray(str)))
    {
        UBOOT_DEBUG("Pgamma IC start\n");
        char* tmpPgammaBinPath;
        tmpPgammaBinPath=str;
        if(FALSE==vfs_getsize(tmpPgammaBinPath))
        {
            while (*tmpPgammaBinPath== '/') tmpPgammaBinPath++;
            tmpPgammaBinPath=strstr(tmpPgammaBinPath,"/");
            UBOOT_DEBUG("filter /cusdata or /config, and find file again!!\n");
            UBOOT_DEBUG("tmpPgammaBinPath:%s\n",tmpPgammaBinPath);

            if(FALSE==vfs_getsize(tmpPgammaBinPath))
            {
                while (*tmpPgammaBinPath== '/') tmpPgammaBinPath++;
                tmpPgammaBinPath=strstr(tmpPgammaBinPath,"/");
                UBOOT_DEBUG("filter /cusdata or /config, and find file again!!\n");
                UBOOT_DEBUG("tmpPgammaBinPath:%s\n",tmpPgammaBinPath);
                if(FALSE==vfs_getsize(tmpPgammaBinPath))
                {
                    tmpPgammaBinPath=str;
                }
            }
        }
        snprintf(gPGAMMAICBinPath,BUFFER_SIZE,"%s",tmpPgammaBinPath);
        sPGAMMAIC->Size = vfs_getsize(gPGAMMAICBinPath);
        UBOOT_DEBUG("Pgamma path --> %s\n",gPGAMMAICBinPath);
        UBOOT_DEBUG("PGAMMAIC size:%x\n",sPGAMMAIC->Size);
        if(sPGAMMAIC->Size != 0)
        {
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_bus","", str, sizearray(str)))
            {
                sPGAMMAIC->Bus = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->Bus:%x\n",sPGAMMAIC->Bus);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_addr","", str, sizearray(str)))
            {
                sPGAMMAIC->Addr = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->Addr:%x\n",sPGAMMAIC->Addr);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_offset","", str, sizearray(str)))
            {
                sPGAMMAIC->Offset = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->Offset:%x\n",sPGAMMAIC->Offset);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_BURN_offset","", str, sizearray(str)))
            {
                sPGAMMAIC->BurnOffset = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->BurnOffset:%x\n",sPGAMMAIC->BurnOffset);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_BURN_cmd","", str, sizearray(str)))
            {
                sPGAMMAIC->BurnCmd = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->BurnCmd:%x\n",sPGAMMAIC->BurnCmd);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_READ_MODE","", str, sizearray(str)))
            {
                sPGAMMAIC->ReadMode = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->ReadMode:%x\n",sPGAMMAIC->ReadMode);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_CONTROL_REG","", str, sizearray(str)))
            {
                sPGAMMAIC->ControlReg = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->ControlReg:%x\n",sPGAMMAIC->ControlReg);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_CONTROL_REG_DATA","", str, sizearray(str)))
            {
                sPGAMMAIC->ControlRegData = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->ControlRegData:%x\n",sPGAMMAIC->ControlRegData);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_NOT_COMPARE_LEN","", str, sizearray(str)))
            {
                sPGAMMAIC->NotCompareLen = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->NotCompareLen:%x\n",sPGAMMAIC->NotCompareLen);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_TOGGLE_PANEL_VCC_FLAG","", str, sizearray(str)))
            {
                sPGAMMAIC->PanelVcc = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->PanelVcc:%x\n",sPGAMMAIC->PanelVcc);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_TOGGLE_PANEL_VCC_DELAY","", str, sizearray(str)))
            {
                sPGAMMAIC->DelayTime = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->DelayTime:%x\n",sPGAMMAIC->DelayTime);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_WP_NUM","", str, sizearray(str)))
            {
                sPGAMMAIC->WPNum = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->WPNum:%x\n",sPGAMMAIC->WPNum);
            }
            if(Profile_GetString("TCON_BIN", "Pgamma_I2C_WP_POL","", str, sizearray(str)))
            {
                sPGAMMAIC->WPPOL = (unsigned int)strtol(str,ptr,16);
                UBOOT_DEBUG("sPGAMMAIC->WPPOL:%x\n",sPGAMMAIC->WPPOL);
            }
        }
    }
    UBOOT_TRACE("OK");
}

int Burn_CustomerBinToExternalFlash2(char* path)
{
    IC_INFO_t sPMIC,sPGAMMAIC;
    unsigned char *memaddr;
    MS_BOOL bRet;

    UBOOT_TRACE("IN\n");
    memset(&sPMIC, 0, sizeof(IC_INFO_t));
    memset(&sPGAMMAIC, 0, sizeof(IC_INFO_t));
    bRet = parse_customer_profile(path,&sPMIC,&sPGAMMAIC);
    if(bRet == FALSE)
    {
        UBOOT_DEBUG("Parse customer ini file failure!\n");
        return -1;
    }

    if(sPMIC.Size > 0)
    {
        PANEL_CTL_On();
        mdelay(sPMIC.DelayTime);

        memaddr = _load_ICBinFile(&sPMIC,gPMICBinPath);
        if(memaddr != NULL)
        {
            if(sPMIC.WPNum != 0)
            {
                UBOOT_DEBUG("sPMIC.WPNum:%d; sPMIC.WPPOL:%d\n",sPMIC.WPNum, sPMIC.WPPOL);
                if(!sPMIC.WPPOL)
                    mdrv_gpio_set_low(sPMIC.WPNum);
                else
                    mdrv_gpio_set_high(sPMIC.WPNum);

                mdelay(5);
            }

            MApi_SWI2C_Init(Pmic_I2cBusCfg, PMIC_I2C_BUS_NUM);
            bRet = do_ic_register_read(&sPMIC,memaddr);
            if(bRet == TRUE)
            {
                UBOOT_DEBUG("IC register compare failure, start to burn bin file\n");
                bRet = do_pm_ic_upgrade(&sPMIC,memaddr);
                if(bRet == TRUE)
                {
                    UBOOT_DEBUG("sPMIC->BurnOffset:%x; sPMIC->BurnCmd:%x\n",sPMIC.BurnOffset, sPMIC.BurnCmd);
                    if(sPMIC.BurnCmd != 0)
                    {
                        do_ic_burn(&sPMIC);
                    }
                }
            }

            if(sPMIC.WPNum != 0)
            {
                UBOOT_DEBUG("GPIO %d  set input.\n",sPMIC.WPNum);
                mdrv_gpio_set_input(sPMIC.WPNum);
                mdelay(5);
            }
        }
    }

    if(sPGAMMAIC.Size > 0)
    {
        PANEL_CTL_On();
        mdelay(sPGAMMAIC.DelayTime);

        memaddr = _load_ICBinFile(&sPGAMMAIC,gPGAMMAICBinPath);
        if(memaddr != NULL)
        {
            if(sPGAMMAIC.WPNum != 0)
            {
                UBOOT_DEBUG("sPGAMMAIC.WPNum:%d; sPGAMMAIC.WPPOL:%d\n",sPGAMMAIC.WPNum, sPGAMMAIC.WPPOL);
                if(!sPGAMMAIC.WPPOL)
                    mdrv_gpio_set_low(sPGAMMAIC.WPNum);
                else
                    mdrv_gpio_set_high(sPGAMMAIC.WPNum);

                mdelay(5);
            }

            bRet = do_ic_register_read(&sPGAMMAIC,memaddr);
            if(bRet == TRUE)
            {
                UBOOT_DEBUG("IC register compare failure, start to burn bin file\n");
                bRet = do_pgamma_ic_upgrade(&sPGAMMAIC,memaddr);
                if(bRet == TRUE)
                {
                    UBOOT_DEBUG("sPGAMMAIC->BurnOffset:%x; sPGAMMAIC->BurnCmd:%x\n",sPGAMMAIC.BurnOffset, sPGAMMAIC.BurnCmd);
                    if(sPGAMMAIC.BurnCmd != 0)
                    {
                        do_ic_burn(&sPGAMMAIC);
                    }
                }
            }

            if(sPGAMMAIC.WPNum != 0)
            {
                UBOOT_DEBUG("GPIO %d  set input.\n",sPGAMMAIC.WPNum);
                mdrv_gpio_set_input(sPGAMMAIC.WPNum);
                mdelay(5);
            }
        }
    }
    UBOOT_TRACE("OK\n");
    return 0;
}
//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
int Burn_CustomerBinToExternalFlash(char* path)
{
    IC_INFO_t sPMIC,sPGAMMAIC;
    unsigned char *memaddr;
    MS_BOOL bRet;

    UBOOT_TRACE("IN\n");
    memset(&sPMIC, 0, sizeof(IC_INFO_t));
    memset(&sPGAMMAIC, 0, sizeof(IC_INFO_t));
    bRet = parse_customer_profile(path,&sPMIC,&sPGAMMAIC);
    if(bRet == FALSE)
    {
        UBOOT_DEBUG("Parse customer ini file failure!\n");
        return -1;
    }

    if(sPMIC.Size > 0)
    {
        PANEL_CTL_On();
        mdelay(sPMIC.DelayTime);

        memaddr = _load_ICBinFile(&sPMIC,gPMICBinPath);
        if(memaddr != NULL)
        {
            if(sPMIC.WPNum != 0)
            {
                UBOOT_DEBUG("sPMIC.WPNum:%d; sPMIC.WPPOL:%d\n",sPMIC.WPNum, sPMIC.WPPOL);
                if(!sPMIC.WPPOL)
                    mdrv_gpio_set_low(sPMIC.WPNum);
                else
                    mdrv_gpio_set_high(sPMIC.WPNum);

                mdelay(5);
            }

            bRet = do_pm_ic_upgrade(&sPMIC,memaddr);
            if(bRet == TRUE)
            {
                UBOOT_DEBUG("sPMIC->BurnOffset:%x; sPMIC->BurnCmd:%x\n",sPMIC.BurnOffset, sPMIC.BurnCmd);
                if(sPMIC.BurnCmd != 0)
                {
                    do_ic_burn(&sPMIC);
                }
            }

            if(sPMIC.WPNum != 0)
            {
                UBOOT_DEBUG("GPIO %d  set input.\n",sPMIC.WPNum);
                mdrv_gpio_set_input(sPMIC.WPNum);
                mdelay(5);
            }
        }
    }

    if(sPGAMMAIC.Size > 0)
    {
        PANEL_CTL_On();
        mdelay(sPGAMMAIC.DelayTime);

        memaddr = _load_ICBinFile(&sPGAMMAIC,gPGAMMAICBinPath);
        if(memaddr != NULL)
        {
            if(sPGAMMAIC.WPNum != 0)
            {
                UBOOT_DEBUG("sPGAMMAIC.WPNum:%d; sPGAMMAIC.WPPOL:%d\n",sPGAMMAIC.WPNum, sPGAMMAIC.WPPOL);
                if(!sPGAMMAIC.WPPOL)
                    mdrv_gpio_set_low(sPGAMMAIC.WPNum);
                else
                    mdrv_gpio_set_high(sPGAMMAIC.WPNum);

                mdelay(5);
            }

            bRet = do_pgamma_ic_upgrade(&sPGAMMAIC,memaddr);
            if(bRet == TRUE)
            {
                UBOOT_DEBUG("sPGAMMAIC->BurnOffset:%x; sPGAMMAIC->BurnCmd:%x\n",sPGAMMAIC.BurnOffset, sPGAMMAIC.BurnCmd);
                if(sPGAMMAIC.BurnCmd != 0)
                {
                    do_ic_burn(&sPGAMMAIC);
                }
            }
            if(sPGAMMAIC.WPNum != 0)
            {
                UBOOT_DEBUG("GPIO %d  set input.\n",sPGAMMAIC.WPNum);
                mdrv_gpio_set_input(sPGAMMAIC.WPNum);
                mdelay(5);
            }
        }
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

int get_customer_ini_path(char* path,char* ModelPath)
{
    int ret = -1;
    char *script = NULL;
    char str[CUS_BUFFER_SIZE];
    U32 filesize = 0;
    UBOOT_TRACE("IN\n");

    script = loadscript(path,&filesize);
    if(script == NULL)
    {
        return ret;
    }
    UBOOT_INFO("sizeof( sys ini script ) --------> %u bytes\n",filesize);

    Profile_Init((char *)script,filesize);
    if(Profile_GetString("sys_config", "gCustomerConfigINI", "",str, sizearray(str)))
    {
        strcpy(ModelPath,str);
        ret = 0;
    }
    else
    {
        UBOOT_INFO("Can't get Customer_1.ini path from %s\n", path);
    }

    UBOOT_TRACE("OK\n");
    return ret;
}

int do_customer_ic_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char InPath [CUS_BUFFER_SIZE]="\0";
    char OutPath[CUS_BUFFER_SIZE]="\0";
    char CustomerInPath[CUS_BUFFER_SIZE];
    int ret = -1;

    UBOOT_TRACE("IN\n");
    vfs_mount(CONFIG);
    snprintf(InPath,sizeof(InPath),"%s/sys.ini",CONFIG_PATH);
    ret = get_customer_ini_path(InPath,OutPath);
    if(ret)
    {
        UBOOT_ERROR("Error : parse sys.ini failure\n");
        return ret;
    }

    memcpy(CustomerInPath,OutPath,sizeof(OutPath));
    UBOOT_INFO("Customer_1.ini path:%s\n", CustomerInPath);
    ret = Burn_CustomerBinToExternalFlash2(CustomerInPath);
    if(ret)
        UBOOT_ERROR("Error : burn customer bin file failure\n");

    UBOOT_TRACE("OK\n");
    return ret;
}
