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
#include <command.h>
#include <MsTypes.h>
#include <apiPNL.h>
#include <MsDebug.h>
#include <MsBoot.h>
#include <MsEnvironment.h>
#include <bootlogo/MsPoolDB.h>
#include <panel/MsDrvPanel.h>
#include <MsApiPanel.h>
#include <drvPWM.h>
#if(ENABLE_URSA_6M30 == 1)
#include <ursa/ursa_6m30.h>
#endif

#if(ENABLE_URSA_6M40 == 1)
#include <ursa/ursa_6m40.h>
#endif

#if(ENABLE_URSA_8 == 1)
#include <ursa/ursa_8.h>
#endif

#include <MsMmap.h>
#include <MsSystem.h>
#include <mstarstr.h>
#include <msAPI_Power.h>

#ifdef UFBL_FEATURE_IDME
#include <idme.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
void cmp(PanelType *p1, PanelType *p2);

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define PANEL_DEBUG 0

//-------------------------------------------------------------------------------------------------
//  External Functions
//-------------------------------------------------------------------------------------------------
extern int is_quiescent_mode(void);
//-----------------------------------------------------------------------------------------------n
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static MS_BOOL bPanleReady=FALSE;
static GetPnlTypeSet_cb fpCusGetPnlTypeSet = NULL;
static MS_BOOL g_preTconOutput = FALSE;
static MS_BOOL g_preODEnable = FALSE;

#if (ENABLE_HDMI_RESOLUTION_RESET==1)
#if (ENABLE_HDMI_TX_RESOLUTION == 0)
static char* DacPanelIndexTbl[] = {
    "DACOUT_480P_60",
    "DACOUT_576P_50",
    "DACOUT_720P_50",
    "DACOUT_720P_60",
    "DACOUT_1080P_50",
    "DACOUT_1080P_60",
    "DACOUT_480I_60",
    "DACOUT_576I_50",
    "DACOUT_1080I_50",
    "DACOUT_1080I_60",
    "DACOUT_1080P_30",
    "DACOUT_1080P_25",
    "DACOUT_1080P_24",
    "DACOUT_640X480_60",
    "DACOUT_1920X2205P_24",
    "DACOUT_1280X1470P_50",
    "DACOUT_1280X1470P_60",
    "DACOUT_4K2KP_24",
    "DACOUT_4K2KP_25",
    "DACOUT_4K2KP_30",
    "DACOUT_4K2KP_50",
    "DACOUT_4K2KP_60",
    "DACOUT_4096X2160P_24",
    "DACOUT_4096X2160P_25",
    "DACOUT_4096X2160P_30",
    "DACOUT_4096X2160P_50",
    "DACOUT_4096X2160P_60",
};

typedef enum{
    DACOUT_RES_480P_60              = 0,
    DACOUT_RES_576P_50              = 1,
    DACOUT_RES_720P_50              = 2,
    DACOUT_RES_720P_60              = 3,
    DACOUT_RES_1080P_50             = 4,
    DACOUT_RES_1080P_60             = 5,
    DACOUT_RES_480I_60              = 6,
    DACOUT_RES_576I_50              = 7,
    DACOUT_RES_1080I_50             = 8,
    DACOUT_RES_1080I_60             = 9,
    DACOUT_RES_1080P_30             = 10,
    DACOUT_RES_1080P_25             = 11,
    DACOUT_RES_1080P_24             = 12,
    DACOUT_RES_640x480_60           = 13,
    DACOUT_RES_1920x2205P_24        = 14,
    DACOUT_RES_1280x1470P_50        = 15,
    DACOUT_RES_1280x1470P_60        = 16,
    DACOUT_RES_3840x2160P_24        = 17,
    DACOUT_RES_3840x2160P_25        = 18,
    DACOUT_RES_3840x2160P_30        = 19,
    DACOUT_RES_3840x2160P_50        = 20,
    DACOUT_RES_3840x2160P_60        = 21,
    DACOUT_RES_4096x2160P_24        = 22,
    DACOUT_RES_4096x2160P_25        = 23,
    DACOUT_RES_4096x2160P_30        = 24,
    DACOUT_RES_4096x2160P_50        = 25,
    DACOUT_RES_4096x2160P_60        = 26,
}DACOUT_VIDEO_TIMING;

#else
#if (ENABLE_HDMITX_MSTAR_ROCKET2 == 1)
#include <hdmitx/mstar/rocket2/MsDrvRocket.h>
static char* HdmiTxPanelIndexTable[] = {
    "",//0
    "",//1
    "",//2
    "HDMITX_VB1_480_60P",//3/
    "HDMITX_VB1_576_50P",//4/
    "HDMITX_VB1_720_50P",//5
    "HDMITX_VB1_720_60P",//6
    "",//7
    "",//8
    "",//9
    "",//10
    "",//11
    "HDMITX_VB1_1080_50P",//12
    "HDMITX_VB1_1080_60P",//13
    "HDMITX_VB1_4K2K_30P",//14
    "",//15
    "",//16
    "",//17
    "",//18
    "",//19
    "",//20
    "HDMITX_VB1_4K2K_25P",//21
    "HDMITX_VB1_4K2K_24P",//22
    "HDMITX_VB1_4K2K_50P",//23
    "HDMITX_VB1_4K2K_60P",//24
    "",//25
    "",//26
    "",//27
    "HDMITX_VB1_4096_24P",//28
};

static char* HdmiTxTimingIndexTable[] = {
    "",//0
    "",//1
    "",//2
    "HDMITX_RES_720x480p",//3
    "HDMITX_RES_720x576p",//4
    "HDMITX_RES_1280x720p_50Hz",//5
    "HDMITX_RES_1280x720p_60Hz",//6
    "",//7
    "",//8
    "",//9
    "",//10
    "",//11
    "HDMITX_RES_1920x1080p_50Hz",//12
    "HDMITX_RES_1920x1080p_60Hz",//13
    "HDMITX_RES_4K2Kp_30Hz",//14
    "",//15
    "",//16
    "",//17
    "",//18
    "",//19
    "",//20
    "HDMITX_RES_4K2Kp_25Hz",//21
    "HDMITX_RES_4K2Kp_24Hz",//22
    "HDMITX_RES_4K2Kp_50Hz",//23
    "HDMITX_RES_4K2Kp_60Hz",//24
    "",//25
    "",//26
    "",//27
    "HDMITX_RES_4096_24P",//28
};
#else
#include <apiHDMITx.h>
static char* HdmiTxPanelIndexTable[] = {
    "",
    "HDMITX_480_60I",
    "HDMITX_576_50I",
    "HDMITX_480_60P",
    "HDMITX_576_50P",
    "HDMITX_720_50P",
    "HDMITX_720_60P",
    "HDMITX_1080_50I",
    "HDMITX_1080_60I",
    "HDMITX_1080_24P",
    "HDMITX_1080_25P",
    "HDMITX_1080_30P",
    "HDMITX_1080_50P",
    "HDMITX_1080_60P",
    "HDMITX_4K2K_30P",
    "HDMITX_1470_50P",
    "HDMITX_1470_60P",
    "HDMITX_1470_24P",
    "HDMITX_1470_30P",
    "HDMITX_2205_24P",
    "HDMITX_2205_30P",
    "HDMITX_4K2K_25P",
};

static char* HdmiTxTimingIndexTable[] = {
    "HDMITX_RES_640x480p",
    "HDMITX_RES_720x480i",
    "HDMITX_RES_720x576i",
    "HDMITX_RES_720x480p",
    "HDMITX_RES_720x576p",
    "HDMITX_RES_1280x720p_50Hz",
    "HDMITX_RES_1280x720p_60Hz",
    "HDMITX_RES_1920x1080i_50Hz",
    "HDMITX_RES_1920x1080i_60Hz",
    "HDMITX_RES_1920x1080p_24Hz",
    "HDMITX_RES_1920x1080p_25Hz",
    "HDMITX_RES_1920x1080p_30Hz",
    "HDMITX_RES_1920x1080p_50Hz",
    "HDMITX_RES_1920x1080p_60Hz",
    "HDMITX_RES_4K2Kp_30Hz",
    "HDMITX_RES_1280x1470p_50Hz",
    "HDMITX_RES_1280x1470p_60Hz",
    "HDMITX_RES_1280x1470p_24Hz",
    "HDMITX_RES_1280x1470p_30Hz",
    "HDMITX_RES_1920x2205p_24Hz",
    "HDMITX_RES_1920x2205p_30Hz",
    "HDMITX_RES_4K2Kp_25Hz",
};
#endif
#endif
#endif

#if (CONFIG_TV_CHIP==0 )
typedef struct
{
    MS_U8                   u8ResolutionEnv;
    PANEL_RESOLUTION_TYPE   enResolutionType;
}RESOLUTION_DAC_MAP;

static RESOLUTION_DAC_MAP stMapTypeIndex[] = {
    {0, DACOUT_480P},
    {1, DACOUT_576P},
    {2, DACOUT_720P_50},
    {3, DACOUT_720P_60},
    {4, DACOUT_1080P_50},
    {5, DACOUT_1080P_60},
    {6, DACOUT_480I},
    {7, DACOUT_576I},
    {8, DACOUT_1080I_50},
    {9, DACOUT_1080I_60}
};
#endif

//-------------------------------------------------------------------------------------------------
//  Extern Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------
int panel_sinit(void);
int panel_dinit(void);
static PANEL_RESOLUTION_TYPE _GetPnlTypeSetting(void);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int IsPanelReady(void)
{
    if(bPanleReady==TRUE)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

APIPNL_LINK_EXT_TYPE GetPanelLinkExtType(void)
{
    APIPNL_LINK_EXT_TYPE eType = LINK_EPI34_8P;
    st_sys_misc_setting misc_setting;
    Read_MiscSetting_ToFlash(&misc_setting);
    if((APIPNL_LINK_EXT_TYPE)misc_setting.m_u16Panel_ext_type!=0)
        eType = (APIPNL_LINK_EXT_TYPE)misc_setting.m_u16Panel_ext_type;
    return eType;
}

void RegisterCBGetPnlTypeSetting(GetPnlTypeSet_cb cb)
{
    UBOOT_TRACE("IN\n");
    fpCusGetPnlTypeSet=cb;
    UBOOT_TRACE("OK\n");
}

#if (CONFIG_TV_CHIP==0 )
static PANEL_RESOLUTION_TYPE _GetPnlTypeSettingFromEnv(void)
{
    int resolution_index = 0;
    PANEL_RESOLUTION_TYPE PanelType = DACOUT_1080I_50;
    char * p_str;
    p_str = getenv ("resolution");

    if(p_str != NULL)
    {
        resolution_index = (int)simple_strtol(p_str, NULL, 10);
        if(resolution_index < sizeof(stMapTypeIndex)/sizeof(RESOLUTION_DAC_MAP))
        {
            return stMapTypeIndex[resolution_index].enResolutionType;
        }
    }

    return PanelType;
}
#endif
static PANEL_RESOLUTION_TYPE _GetPnlTypeSetting(void)
{
    PANEL_RESOLUTION_TYPE PanelType = 0;
    UBOOT_TRACE("IN\n");
    if(fpCusGetPnlTypeSet!=NULL)
    {
        PanelType = fpCusGetPnlTypeSet();
    }
    else
    {
        PanelType = MApi_PNL_GetPnlTypeSetting();
    }
    UBOOT_TRACE("OK\n");
    return PanelType;
}


int panel_sinit(void)
{
    PanelType* panel_data = NULL;
    PANEL_RESOLUTION_TYPE enPanelType;
    UBOOT_TRACE("IN\n");

#if (CONFIG_TV_CHIP==0 )
    RegisterCBGetPnlTypeSetting(_GetPnlTypeSettingFromEnv);
#endif

    enPanelType = _GetPnlTypeSetting();
    if(MApi_PNL_PreInit(E_PNL_NO_OUTPUT)!=TRUE)
    {
        bPanleReady=FALSE;
        return -1;
    }

    panel_data=MApi_GetPanelSpec(enPanelType);
    if(panel_data==NULL)
    {
        bPanleReady=FALSE;
        return -1;
    }

    if(panel_data->m_wPanelWidth == 3840 && panel_data->m_wPanelHeight == 2160)
    {
        UBOOT_DEBUG(">> m_ePanelLinkExtType = 51 <<<\n");
        setLinkExtType(51);
    }
    //pane init
    #ifdef BD_LVDS_CONNECT_TYPE
	MApi_BD_LVDS_Output_Type(BD_LVDS_CONNECT_TYPE);
    #endif
    if(MsDrv_PNL_Init(panel_data)==FALSE)
    {
        bPanleReady=FALSE;
        return -1;
    }
    bPanleReady=TRUE;
    UBOOT_TRACE("OK\n");
    return 0;
}

#if (ENABLE_HDMI_RESOLUTION_RESET==1)
static int BootArgs_reset(void)
{
    int resolution_index = 5;
    char *p = NULL;
    char *p_reset = NULL;
    char buf[50] = "\0";
    #if (ENABLE_HDMI_TX_RESOLUTION == 1)
    #if (ENABLE_HDMITX_MSTAR_ROCKET2 == 1)
    EN_MAPI_DEVICE_ROCKY_VIDEO_TIMING ePANELType = E_MAPI_ROCKY_RES_1920x1080p_50Hz;
    #else
    HDMITX_VIDEO_TIMING ePANELType = HDMITX_RES_1920x1080p_50Hz;
    #endif
    #else
    DACOUT_VIDEO_TIMING ePANELType = DACOUT_RES_1080P_50;
    #endif

    p = getenv("resolution");
    p_reset = getenv("resolution_reset");
    snprintf(buf, sizeof(buf), "%ld", ePANELType);
    if(isBootToRecovery())
    {
        // if boot to Recovery mode, set resolution 1080P50
        if((p != NULL) && (0 == strcmp(p, buf)))
            return 0;
        setenv("resolution", buf);
    }
    else
    {
        if ((p_reset == NULL) && (p != NULL))
        {
            setenv("resolution_reset", p);
        }
        else if((p_reset != NULL) && (p == NULL))
        {
            setenv("resolution", p_reset);
        }
        else if((p == NULL) && (p_reset == NULL))
        {
            setenv("resolution", buf);  //1080P50
            setenv("resolution_reset", buf);  //1080P50
        }
        else
        {
            resolution_index = (int)simple_strtol(p_reset, NULL, 10);
            if (0 == strcmp(p, p_reset))
            {
                #if (ENABLE_HDMI_TX_RESOLUTION == 1)
                snprintf(buf, sizeof(buf),"/config/panel/%s.ini", HdmiTxPanelIndexTable[resolution_index]);
                #else
                snprintf(buf, sizeof(buf),"/config/panel/%s.ini", DacPanelIndexTbl[resolution_index]);
                #endif
                char* p_panel = getenv("panel_path");
                char res_string[CONFIG_SYS_CBSIZE];
                if ( get_bootargs_cfg("resolution=", res_string, CONFIG_SYS_CBSIZE) != 0)
                {
                    UBOOT_DEBUG("get_bootargs_cfg(resolution=) failed!\n");
                }

                if((p_panel != NULL) && (strcmp(buf, p_panel) == 0))
                {
                #if (ENABLE_HDMI_TX_RESOLUTION == 0)
                    UBOOT_DEBUG("res_string = %s\n", res_string);
                    UBOOT_DEBUG("DacPanelIndexTbl = %s\n", DacPanelIndexTbl[resolution_index]);
                    if ((res_string != NULL) && (strcmp(res_string, DacPanelIndexTbl[resolution_index]) == 0))
                    {
                        return 0;
                    }
                #else
                    return 0;
                #endif
                }
            }
            else
            {
                setenv("resolution", p_reset);
            }
        }
    }

    p = getenv ("resolution");
    if(p != NULL)
    {
        resolution_index = (int)simple_strtol(p, NULL, 10);
        #if (ENABLE_HDMI_TX_RESOLUTION == 1)
        MApi_SetEnv2BootArgs("resolution=", HdmiTxTimingIndexTable[resolution_index]);
        snprintf(buf, sizeof(buf),"/config/panel/%s.ini", HdmiTxPanelIndexTable[resolution_index]);
        MApi_SetEnv2BootArgs("panel_path=", buf);
        setenv("panel_path", buf);
        setenv("panel_name", HdmiTxPanelIndexTable[resolution_index]);
        #else
        MApi_SetEnv2BootArgs("resolution=", DacPanelIndexTbl[resolution_index]);
        snprintf(buf, sizeof(buf),"/config/panel/%s.ini", DacPanelIndexTbl[resolution_index]);
        setenv("panel_path", buf);
        setenv("panel_name", DacPanelIndexTbl[resolution_index]);
        #endif
        UBOOT_DEBUG("bootArgs need to reset !!\n");
        UBOOT_DEBUG("panel_path = %s\n", buf);
        UBOOT_DEBUG("resolution = %s\n", p);
        setenv("db_table", "0");
        run_command("dbtable_init", 0);
    }
    saveenv();
    return 0;
}

int Set_BootArgs_Resolution(void)
{
    int resolution_index = 5;
    char *p = NULL;

    p = getenv ("resolution");
    if(p != NULL)
    {
        resolution_index = (int)simple_strtol(p, NULL, 10);
        printf("set resolution info to bootargs\n");

        #if (ENABLE_HDMI_TX_RESOLUTION == 1)
        MApi_SetEnv2BootArgs("resolution=", HdmiTxTimingIndexTable[resolution_index]);
        #else
        MApi_SetEnv2BootArgs("resolution=", DacPanelIndexTbl[resolution_index]);
        #endif
    }

    return 0;
}
#endif

extern int isBootToRecovery(void);
int panel_dinit(void)
{
    MS_U16 u16Panel_SwingLevel;
    PanelType panelpara;
    st_board_para boardpara;
    st_sys_misc_setting misc_setting;
    UBOOT_TRACE("IN\n");
    char config_name_buf[32] = {0};

    memset(&panelpara, 0, sizeof(panelpara));
    memset(&boardpara, 0, sizeof(boardpara));

    #if (ENABLE_HDMI_RESOLUTION_RESET==1)
    BootArgs_reset();
    #endif
    //load panel para
#if (defined(CONFIG_URSA6_VB1) || defined(CONFIG_NOVA_KS2) || defined(CONFIG_URSA9_VB1))
	if(is_str_resume())
	{
		MS_PHYADDR PanelConfigsAddr=0, PanelConfigsSize=0;
        // When suspending, SN will store the latest panel param
        // for mboot getting the right timing.
		if(get_map_addr_and_size_from_env(E_VDEC_CPU, NO_DEFAULT_MMAP_VALUE, &PanelConfigsAddr, NO_DEFAULT_MMAP_VALUE, &PanelConfigsSize)!=0)
		{
			UBOOT_ERROR("get %s mmap fail\n",E_MMAP_ID_VDEC_CPU_ADR);
			bPanleReady=FALSE;
			return -1;
		}
		UBOOT_DEBUG("E_MMAP_ID_VDEC_CPU = 0x%llx\n", PanelConfigsAddr);
		UBOOT_DEBUG("(U32)(PA2NVA(PanelConfigsAddr)) = 0x%x\n", (U32)(PA2NVA(PanelConfigsAddr)));
		memcpy(&panelpara, (U32*)(PA2NVA(PanelConfigsAddr)), sizeof(PanelType));
        // Error handle when SN failed to pass panel param.
        //
        // Only for Utopia2k STR bringup,
        // this code should be moved to ursa(backend) kernel driver later.
        if(!(panelpara.m_wPanelWidth == 3840 && panelpara.m_wPanelHeight == 2160)
                && !(panelpara.m_wPanelWidth == 1920 && panelpara.m_wPanelHeight == 1080))
        {
            UBOOT_DEBUG(">>> can't get the right param from share memory, try flash anyway.");
            if(Read_PanelParaFromflash(&panelpara)!=0)
            {
                bPanleReady=FALSE;
                UBOOT_ERROR("%s: Read_PanelParaFromflash() failed, at %d\n", __func__, __LINE__);
                return -1;
            }
        }
	}
	else
	{
		if(Read_PanelParaFromflash(&panelpara)!=0)
		{
			bPanleReady=FALSE;
			UBOOT_ERROR("%s: Read_PanelParaFromflash() failed, at %d\n", __func__, __LINE__);
			return -1;
		}
	}
#else
    if(Read_PanelParaFromflash(&panelpara)!=0)
    {
        bPanleReady=FALSE;
        UBOOT_ERROR("%s: Read_PanelParaFromflash() failed, at %d\n", __func__, __LINE__);
        return -1;
    }
#endif

    //load board para
    if(Read_BoardParaFromflash(&boardpara)!=0)
    {
        bPanleReady=FALSE;
        UBOOT_ERROR("%s: Read_BoardParaFromflash() failed, at %d\n", __func__, __LINE__);
        return -1;
    }

    //panel setting by each board
    panelpara.m_bPanelPDP10BIT = boardpara.m_bPANEL_PDP_10BIT;
    panelpara.m_bPanelSwapLVDS_POL = boardpara.m_bPANEL_SWAP_LVDS_POL;
    panelpara.m_bPanelSwapLVDS_CH = boardpara.m_bPANEL_SWAP_LVDS_CH;
    panelpara.m_bPanelSwapPort ^= boardpara.m_bPANEL_CONNECTOR_SWAP_PORT;
    panelpara.m_u16LVDSTxSwapValue = (boardpara.m_u16LVDS_PN_SWAP_H << 8) + boardpara.m_u16LVDS_PN_SWAP_L;


    MS_U16 u16PanelDCLK=0;

    Read_MiscSetting_ToFlash(&misc_setting);
    u16PanelDCLK = misc_setting.m_u16PanelDCLK;

    if(panelpara.m_ePanelLinkType >= LINK_EXT )
    {
        UBOOT_DEBUG(">> SW setting: m_ePanelLinkExtType = %d <<<\n",panelpara.m_ePanelLinkType);
        UBOOT_DEBUG(">> GetPanelLinkExtType = %d <<<\n",GetPanelLinkExtType());
        UBOOT_DEBUG(">> u16PanelDCLK = %u <<<\n",u16PanelDCLK);
        setLinkExtType(GetPanelLinkExtType());
    }


#if (ENABLE_ENABLE_URSA == 1)
#if (ENABLE_URSA_6M40 == 1)
    MDrv_Ursa_6M40_Set_VB1_Init(GetPanelLinkExtType());
#endif
#if (ENABLE_URSA_8 ==1 )||(ENABLE_URSA_6M40 == 1)
       ursa_cmd_table cmd_table={0};
	   if(Read_Ursa_Para(&cmd_table)==0)
	   {
	   	#if (ENABLE_URSA_8 == 1)
		   Ursa_8_Setting(&cmd_table);
		#else
			Ursa_6M40_Syetting(&cmd_table);
		#endif
	   }
	   else
	   {
		  UBOOT_ERROR("read ursa_8 data fail ...>>>\n");
	   }

#endif

#if (ENABLE_URSA_6M30 == 1)
    MDrv_Ursa_6M30_Initialize();
    if(bMst6m30Installed)
    {
        ursa_6m30_cmd_table cmd_table={0};
        if(Read_Ursa_6m30_Para(&cmd_table)==0)
        {
            Ursa_6M30_Setting(&cmd_table);
        }
        else
        {
           UBOOT_ERROR("read ursa_6m30 data fail ...>>>\n");
        }
    }
    else
    {
           UBOOT_ERROR("ursa_6m30 installed fail ...>>>\n");
    }
#endif
#endif

#if PANEL_DEBUG
    PanelType* panel_data = NULL;
    PANEL_RESOLUTION_TYPE enPanelType;
    enPanelType = MApi_PNL_GetPnlTypeSetting();
    MApi_PNL_PreInit(E_PNL_NO_OUTPUT);
    panel_data=MApi_GetPanelSpec(enPanelType);
    cmp(panel_data,&panelpara);
#endif

    MApi_BD_LVDS_Output_Type(boardpara.m_u16BOARD_LVDS_CONNECT_TYPE);
    UBOOT_DEBUG("MApi_BD_LVDS_Output_Type =0x%x\n",boardpara.m_u16BOARD_LVDS_CONNECT_TYPE);
    if(MsDrv_PNL_Init(&panelpara)==FALSE)
    {
        bPanleReady=FALSE;
        return -1;
    }

    //set swing level
    st_sys_misc_setting misc_data;
    Read_MiscSetting_ToFlash(&misc_data);
    u16Panel_SwingLevel = misc_data.m_u16Panel_SwingLevel;
#if ( ENABLE_HDMITX_MSTAR_ROCKET==0)
    if(MApi_PNL_Control_Out_Swing(u16Panel_SwingLevel)!=TRUE)
    {
        bPanleReady=FALSE;
        return -1;
    }
#endif
    bPanleReady=TRUE;

    idme_get_var_external("config_name", config_name_buf, (sizeof(config_name_buf) - 1));

    if (strstr(config_name_buf, "ABC") != NULL) {
        if(!is_quiescent_mode()) {
            printf("[%s] Turn on backlight\n", __FUNCTION__);
            Panel_VCC_ON();
            MDrv_PWM_Shift(E_PWM_CH0, 0x0);
        }
        else {
            printf("[%s] Silent OTA, do not turn on backlight\n", __FUNCTION__);
            PWM_init();
            Panel_VCC_OFF();
            MDrv_PWM_Shift(E_PWM_CH0, 0x80000);
        }
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

int do_panel_pre_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    UBOOT_TRACE("IN\n");
    if (argc < 2)
    {
#if (CONFIG_STATIC_PANEL_PARA)
        ret = panel_sinit();
#else
        ret = panel_dinit();
#endif
    }
    else
    {
#if CONFIG_MINIUBOOT
#else
       if(strncmp(argv[1], "-d", 2) == 0)
       {
            ret = panel_dinit();
       }
       else if (strncmp(argv[1], "-s", 2) == 0)
       {
            ret = panel_sinit();
       }
       else
#endif
       {
           cmd_usage(cmdtp);
       }
    }
    printf("[AT][MB][panel_pre_init][%lu]\n", MsSystemGetBootTime());
    UBOOT_TRACE("OK\n");
    return ret;
}
int do_panel_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    UBOOT_TRACE("IN\n");
    if (argc < 2)
    {
#if (CONFIG_STATIC_PANEL_PARA)
        ret = panel_sinit();
#else
        ret = panel_dinit();
#endif
    }
    else
    {
       if(strncmp(argv[1], "-d", 2) == 0)
       {
            ret = panel_dinit();
       }
       else if (strncmp(argv[1], "-s", 2) == 0)
       {
            ret = panel_sinit();
       }
       else
       {
           cmd_usage(cmdtp);
		   return ret;
       }
    }
	if(bPanleReady)
	{
   		MsDrv_PNL_BackLight_On();
	}
    UBOOT_TRACE("OK\n");
    return ret;
}

int do_backLight_on(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    char *ldm_led_device_name = NULL;
    UBOOT_TRACE("IN\n");

    Load_LDMSavePower_FromINI();

#if (CONFIG_TV_CHIP==1)
    if(is_quiescent_mode())
    {
        if (msAPI_Power_QueryPowerOnMode == EN_POWER_DC_BOOT)
        {
            MsDrv_PNL_BackLight_On();
        }
        else
        {
#if (CONFIG_LOCAL_DIMMING)
            ldm_led_device_name = getenv("ldm_led_device_name");

            if((ldm_led_device_name != NULL) && (strncmp(ldm_led_device_name, "ape5030", strlen(ldm_led_device_name))==0))
            {
                MsDrv_PNL_BackLight_On();
                printf("turn on the backlight as the VCC for Ldm led device APE5030, but disable all led channel.\n");
            }
            else
            {
                printf(" Do not turn on backlight for quiescent mode\n");
            }
#else
            printf(" Do not turn on backlight for quiescent mode\n");
#endif
        }
    }
    else
    {
        printf("turn on the backlight\n");
        MsDrv_PNL_BackLight_On();
    }
#endif
    UBOOT_TRACE("OK\n");
    return ret;
}

#if (CONFIG_LOCAL_DIMMING)
#include <drvMSPI.h>
//#include <drvLDMA.h>
#include <bootlogo/MsPoolDB.h>
#include <MsUboot.h>

#define msAPI_Timer_Delayms(x) udelay(1000*x)

#define MHAL_LD_PACKLENGTH    (32)
#define LDFALIGN              (8)          //align bits
#define LD_BIN_LENGTH         (0x80000)    //bin buffer
#define BRIGHT_LEVEL_DEFAULT  (0xff)
#define LD_MAX_BLOCK          (10*1024)    //follow kernel setting in file linaro\mstar2\drv\ldm\include\Mdrv_ldm_algorithm.h

/* APE5030 Registers */
#define LDM_APE5030_DELAY1           0x16    // Defines the delay time of the PWM, 0x16 ~ 0x35,
#define LDM_APE5030_PWM_HTIME1       0x37    // Defines PWM high time, 0x37 ~ 0x56,

#define LDM_APE5030_BROADCAST        0x1
#define LDM_APE5030_SINGLE_BYTE      0x1
#define LDM_APE5030_MULTI_BYTE       0x0
#define LDM_APE5030_SINGLE_DEV       0x0
#define LDM_APE5030_MULTI_DEV        0x1

#define LDM_APE5030_WRITE_IDX        0x0
#define LDM_APE5030_READ_IDX         0x1

/* APE5030 Device IDs*/
#define LDM_APE5030_ADDR1        0x01
#define LDM_APE5030_BCAST_MULTI_BYTE_SAME   0x00
#define LDM_APE5030_BCAST_SINGLE_BYTE_SAME   0x00
#define LDM_APE5030_BCAST_MULTI_BYTE_DIFF   0x3F

#define LOCAL_DIMMING_MSPI_CH E_MSPI1
#define LOCAL_DIMMING_MSPI_MODE E_MSPI_MODE0
#define LOCAL_DIMMING_MSPI_DC_TRSTART (0x00)
#define LOCAL_DIMMING_MSPI_DC_TREND (0x00)
#define LOCAL_DIMMING_MSPI_DC_TB (0x03)//0x0A
#define LOCAL_DIMMING_MSPI_DC_TRW (0x00)

#define LOCAL_DIMMING_PWM_CH    1

#define VSYNC_50   (0x0)
#define VSYNC_60   (0x1)
#define VSYNC_100  (0x2)
#define VSYNC_120  (0x3)

//modify it according to the default frequency of panel when power on
#define DEFAULT_VSYNC  VSYNC_120

//24MHZ crystal oscillator && div is 0x0b
//Osc = Period * Frequency * Div
//you can modify the div and duty if you need!!!
#define LOCAL_DIMMING_PWM_VSYNC_Div             (0x0)

#define LOCAL_DIMMING_PWM_VSYNC_50_Period       (0x4E1F)
#define LOCAL_DIMMING_PWM_VSYNC_50_DutyCycle    (0x20)

#define LOCAL_DIMMING_PWM_VSYNC_60_Period       (0x4119)
#define LOCAL_DIMMING_PWM_VSYNC_60_DutyCycle    (0x22)

#define LOCAL_DIMMING_PWM_VSYNC_100_Period      (0x2710)
#define LOCAL_DIMMING_PWM_VSYNC_100_DutyCycle   (0x22)

#define LOCAL_DIMMING_PWM_VSYNC_120_Period      (0x208C)
#define LOCAL_DIMMING_PWM_VSYNC_120_DutyCycle   (0x25)

#define DUAL_SPI  0 // use two spi channel to send data

/* APE5030 Vars. */
#define MAX_DEVICE_NUM 6
static MS_U8 buffer[3] = {0};
static MS_U8 u8TailZero[MAX_DEVICE_NUM]={0};

//For APE5030, max device_num = 6
static MS_U8 g_u8Ape5030_dev_num = 0;
static MS_U16 g_u16LED_CMD_CUR_ON_1[6] = {0};
static MS_U16 g_u16LED_CMD_CUR_ON_2[6] = {0};
static MS_U16 g_u16LED_CMD_FAULT_1[6] = {0};
static MS_U16 g_u16LED_CMD_CURR_CTRL[6] = {0};
static MS_U16 g_u16LED_CMD_FB_ON_1[6] = {0};
static MS_U16 g_u16LED_CMD_FB_ON_2[6] = {0};
static MS_U16 g_u16LED_CMD_PWM1_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM1_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM2_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM2_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM3_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM3_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM4_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM4_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM5_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM5_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM6_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM6_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM7_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM7_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM8_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM8_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM9_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM9_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM10_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM10_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM11_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM11_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM12_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM12_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM13_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM13_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM14_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM14_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM15_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM15_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_PWM16_DLY_L[6] = {0};
static MS_U16 g_u16LED_CMD_PWM16_DLY_H[6] = {0};
static MS_U16 g_u16LED_CMD_SHORT_COMP_CTRL_1[6] = {0};

int MSPI_Write_SingleAPE5030_SingleData(int u8DeiveID,int u8RegisterAddr, int u8InputdData)
{
    int ret = 0;

    // 1. Set device ID
    // Write same value to same addr for all device
    buffer[0] = (U8)(u8DeiveID | (0x0<<7) | (0x1<<6));

    // 2. Set addr and data
    buffer[1] = (U8)(u8RegisterAddr | (0x0 << 7));
    buffer[2] = u8InputdData;

    // 3. Send SPI command
    MDrv_MSPI_SlaveEnable(true);
    MDrv_MSPI_Write(buffer,3);
    if((g_u8Ape5030_dev_num - 1) > 0)
    {
        MDrv_MSPI_Write(u8TailZero, g_u8Ape5030_dev_num - 1);
    }

    MDrv_MSPI_SlaveEnable(false);
    return ret;

}
int MSPI_Read_APE5030_SingleData(int u8deviceidx,int u8RegisterAddr)
{

    //U8 pu8InputdData[5] = 0;
    U8 pu8InputdData = 0;
    buffer[0] = (U8)(u8deviceidx | (LDM_APE5030_SINGLE_DEV<<7) | (LDM_APE5030_SINGLE_BYTE<<6));
    buffer[1] = (U8)(u8RegisterAddr | (LDM_APE5030_READ_IDX << 7));
    buffer[2] = 0;

    MDrv_MSPI_SlaveEnable(true);
    MDrv_MSPI_Write(buffer,2);

    MDrv_MSPI_Write(u8TailZero, g_u8Ape5030_dev_num);

    MDrv_MSPI_Read(&pu8InputdData,1);
    MDrv_MSPI_SlaveEnable(false);

    return pu8InputdData;//[3];
}
void MSPI_Write_AllAPE5030_SingleData(int u8RegisterAddr, int u8InputdData)
{
    // 1. Set device ID
    // Write same value to same addr for all device
    buffer[0] = (U8)(LDM_APE5030_BCAST_MULTI_BYTE_SAME | (LDM_APE5030_BROADCAST<<7) | (LDM_APE5030_SINGLE_BYTE<<6));

    // 2. Set addr and data
    buffer[1] = (U8)(u8RegisterAddr | (LDM_APE5030_WRITE_IDX << 7));
    buffer[2] = u8InputdData;

    // 3. Send SPI command
    MDrv_MSPI_SlaveEnable(true);
    MDrv_MSPI_Write(buffer,3);
    if((g_u8Ape5030_dev_num - 1) > 0)
    {
        MDrv_MSPI_Write(u8TailZero, g_u8Ape5030_dev_num - 1);
    }

    MDrv_MSPI_SlaveEnable(false);

}

void APE5030_Init(void)
{
    U8 i, u8temp = 0;

    UBOOT_DEBUG("-- APE5030_Init LD --\n");

    //Please see init excel of APE5030....

    //FAULT_1
    for(i = 0; i < g_u8Ape5030_dev_num; i++)
    {
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x03, g_u16LED_CMD_FAULT_1[i]);
    }

    MSPI_Write_AllAPE5030_SingleData(0x04,0x10);//GPIO_CTIL
    MSPI_Write_AllAPE5030_SingleData(0x05,0x00);//FB_SEL1
    MSPI_Write_AllAPE5030_SingleData(0x06,0x00);//FB_SEL2

    //CURR_CTRL
    for(i = 0; i < g_u8Ape5030_dev_num; i++)
    {
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x07, g_u16LED_CMD_CURR_CTRL[i]);
    }

    MSPI_Write_AllAPE5030_SingleData(0x0D,0x03);//VDAC_L
    MSPI_Write_AllAPE5030_SingleData(0x0C,0x94);//VDAC_H

    //FB_ON_1
    for(i = 0; i < g_u8Ape5030_dev_num; i++)
    {
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x0E, g_u16LED_CMD_FB_ON_1[i]);
    }

    //FB_ON_2
    for(i = 0; i < g_u8Ape5030_dev_num; i++)
    {
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x0F, g_u16LED_CMD_FB_ON_2[i]);
    }

    MSPI_Write_AllAPE5030_SingleData(0x10,0x00);//IDAC_FB1_COUNTER
    MSPI_Write_AllAPE5030_SingleData(0x11,0x00);//IDAC_FB2_COUNTER
    MSPI_Write_AllAPE5030_SingleData(0x12,0x05);//FBLOOP_CTRL
    MSPI_Write_AllAPE5030_SingleData(0x13,0x41);//PWMCTRL
    MSPI_Write_AllAPE5030_SingleData(0x14,0xCF);//RETRIAL_TIMEL
    MSPI_Write_AllAPE5030_SingleData(0x15,0x07);//RETRIAL_TIMEH

    //PWM1_DLY ~ PWM16_DLY (reg 0x16 ~ 0x35)
    for(i = 0; i < g_u8Ape5030_dev_num; i++)
    {
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x16, g_u16LED_CMD_PWM1_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x17, g_u16LED_CMD_PWM1_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x18, g_u16LED_CMD_PWM2_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x19, g_u16LED_CMD_PWM2_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x1A, g_u16LED_CMD_PWM3_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x1B, g_u16LED_CMD_PWM3_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x1C, g_u16LED_CMD_PWM4_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x1D, g_u16LED_CMD_PWM4_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x1E, g_u16LED_CMD_PWM5_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x1F, g_u16LED_CMD_PWM5_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x20, g_u16LED_CMD_PWM6_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x21, g_u16LED_CMD_PWM6_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x22, g_u16LED_CMD_PWM7_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x23, g_u16LED_CMD_PWM7_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x24, g_u16LED_CMD_PWM8_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x25, g_u16LED_CMD_PWM8_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x26, g_u16LED_CMD_PWM9_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x27, g_u16LED_CMD_PWM9_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x28, g_u16LED_CMD_PWM10_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x29, g_u16LED_CMD_PWM10_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x2A, g_u16LED_CMD_PWM11_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x2B, g_u16LED_CMD_PWM11_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x2C, g_u16LED_CMD_PWM12_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x2D, g_u16LED_CMD_PWM12_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x2E, g_u16LED_CMD_PWM13_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x2F, g_u16LED_CMD_PWM13_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x30, g_u16LED_CMD_PWM14_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x31, g_u16LED_CMD_PWM14_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x32, g_u16LED_CMD_PWM15_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x33, g_u16LED_CMD_PWM15_DLY_H[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x34, g_u16LED_CMD_PWM16_DLY_L[i]);
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x35, g_u16LED_CMD_PWM16_DLY_H[i]);
    }

    // set duty to 50%
    // NOTE:
    // 14’h0001 : 0.0061%
    // 14’h0002 : 0.0122%
    // 14’h0003 : 0.0183%
    // 14’h3FFF : 100%
    // PFMBR/16383*100= Brightness percentage
    MSPI_Write_AllAPE5030_SingleData(0x37,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x38,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x39,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x3A,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x3B,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x3C,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x3D,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x3E,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x3F,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x40,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x41,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x42,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x43,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x44,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x45,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x46,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x47,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x48,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x49,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x4A,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x4B,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x4C,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x4D,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x4E,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x4F,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x50,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x51,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x52,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x53,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x54,0x1F);
    MSPI_Write_AllAPE5030_SingleData(0x55,0xFF);
    MSPI_Write_AllAPE5030_SingleData(0x56,0x1F);

    MSPI_Write_AllAPE5030_SingleData(0x63,0x02); //BIST_CTRL_1

    //SHORT_COMP_CTRL_1
    for(i = 0; i < g_u8Ape5030_dev_num; i++)
    {
        MSPI_Write_SingleAPE5030_SingleData((i+1), 0x64, g_u16LED_CMD_SHORT_COMP_CTRL_1[i]);
    }

    MSPI_Write_AllAPE5030_SingleData(0x65,0x28); //BRI_MINI
    MSPI_Write_AllAPE5030_SingleData(0x66,0x06); //HDR_MODE
    MSPI_Write_AllAPE5030_SingleData(0x6D,0x63); //ASW_VDAC_TH_L
    MSPI_Write_AllAPE5030_SingleData(0x6E,0x00); //ASW_VDAC_TH_H
    MSPI_Write_AllAPE5030_SingleData(0x6F,0xCC); //ASW_BRI_TH_L
    MSPI_Write_AllAPE5030_SingleData(0x70,0x2C); //ASW_BRI_TH_H

    if(is_quiescent_mode() && (msAPI_Power_QueryPowerOnMode != EN_POWER_DC_BOOT))
    {
        printf(" Do not turn ON all led channel of APE5030 for quiescent mode\n");
    }
    else
    {
        //CUR_ON_1
        for(i = 0; i < g_u8Ape5030_dev_num; i++)
        {
            MSPI_Write_SingleAPE5030_SingleData((i+1), 0x01, g_u16LED_CMD_CUR_ON_1[i]);
        }

        //CUR_ON_2
        for(i = 0; i < g_u8Ape5030_dev_num; i++)
        {
            MSPI_Write_SingleAPE5030_SingleData((i+1), 0x02, g_u16LED_CMD_CUR_ON_2[i]);
        }
    }

    UBOOT_DEBUG("--end of APE5030_Init--\n");
}

int do_local_dimming( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ST_DRV_MSPI_INFO mspi_info;
    ST_DRV_LD_DMA_INFO dma_info;
    ST_DRV_LD_LED_DEVICE_INFO device_info;
    MS_U8 * pLEDVirBuffer;
    MS_U32 LEDVirBufferSize;
    MS_PHYADDR BaseAddr = 0;
    unsigned int u32Size = 0;
    MS_U8 u8Bright = BRIGHT_LEVEL_DEFAULT;
    MS_PHYADDR LDFAddr_L0 = 0;
    MS_PHYADDR LDFAddr_L1 = 0;
    MS_PHYADDR LDFAddr_R0 = 0;
    MS_PHYADDR LDFAddr_R1 = 0;
    MS_PHYADDR LDBAddr_L0 = 0;
    MS_PHYADDR LDBAddr_L1 = 0;
    MS_PHYADDR LDBAddr_R0 = 0;
    MS_PHYADDR LDBAddr_R1 = 0;
    MS_U32 LDF_pack_per_row = 0;
    MS_U32 LDF_mem_size = 0;
    MS_U8 u8LDBWidth = 0;
    MS_U8 u8LDBHeight = 0;
    MS_U32 LDB_pack_per_row = 0;
    MS_U32 LDB_mem_size = LD_MAX_BLOCK;
    MS_PHYADDR LDBBaseAddr = 0;
    MS_PHYADDR Edge2DBaseAddr = 0;
    MS_PHYADDR DMABaseOffset = 0;
    MS_U8 u8temp = 0;
    MS_U16 u16temp = 0;
    MS_U8 i =0;
    char * support_ldm = NULL;
    int support_ldm_value = 0;
    MS_U8 u8ErrorCnt = 0;

    UBOOT_TRACE("IN");
    if (argc >= 2) {
        u8Bright = simple_strtoul(argv[1], NULL, 16);
    }

    support_ldm = getenv("support_ldm");
    if(support_ldm)
    {
        support_ldm_value = atoi(support_ldm);
        UBOOT_DEBUG("support_ldm_value = %d!\n", support_ldm_value);

        if(support_ldm_value == true)
        {
            UBOOT_DEBUG("do local dimming.\n");
        }
        else
        {
            UBOOT_DEBUG("support_ldm_value is not enabled, do nothing.\n");
            return 0;
        }
    }
    else
    {
        UBOOT_DEBUG("support_ldm is empty! do nothing.\n");
        return 0;
    }

    memset(&mspi_info,0,sizeof(ST_DRV_MSPI_INFO));
    memset(&dma_info,0,sizeof(ST_DRV_LD_DMA_INFO));
    memset(&device_info,0,sizeof(ST_DRV_LD_LED_DEVICE_INFO));

    Load_LDMPara_FromINI(&mspi_info,&device_info);

    if(device_info.bAPE5030 == 1)
    {
        mspi_info.u32MspiClk = 6000000;
        mspi_info.u8MspiChanel = 0; // 0: E_MSPI1
        mspi_info.u8MspiMode = 0; //0: E_MSPI_MODE0
        mspi_info.u8TB = LOCAL_DIMMING_MSPI_DC_TB;
        mspi_info.u8TrEnd = LOCAL_DIMMING_MSPI_DC_TREND;
        mspi_info.u8TrStart = LOCAL_DIMMING_MSPI_DC_TRSTART;
        mspi_info.u8TRW = LOCAL_DIMMING_MSPI_DC_TRW;
        for(i = 0 ; i < 8 ; i++)
        {
            mspi_info.u8RBitConfig[i]= 0x07;
            mspi_info.u8WBitConfig[i]= 0x07;
        }
    }

    //Init MSPI
    MDrv_MasterSPI_CsPadConfig(0,0xFF);//default cs pin use mspi config3
    MDrv_MSPI_Info_Config(&mspi_info);
    //MDrv_MSPI_RWBytes(0,0x00);//set DMA read bytes
    //MDrv_MSPI_RWBytes(1,0x01);//set DMA write bytes

    if(device_info.bAPE5030 == 1)
    {
        g_u8Ape5030_dev_num = device_info.u8Ape5030_dev_num;

        // Wait APE5030 power on
        while(true)
        {
            u8temp = MSPI_Read_APE5030_SingleData(1,0x60);
            UBOOT_DEBUG("check APE5030 power status: %x\n", u8temp);

            if((u8temp == 0xFF)||(u8temp&BIT0)== 0)
            {
                if(++u8ErrorCnt >= 100) //1 sec for timeout.
                {
                    UBOOT_DEBUG("Not detected the power of APE5030!!\n");
                    return 0;
                }
            }
            else
            {
                UBOOT_DEBUG("APE5030 power on\n");
                break;
            }

            msAPI_Timer_Delayms(10);
        }

        // prepare specific register value from ini.
        for(i = 0 ;i < g_u8Ape5030_dev_num; i++)
        {
            g_u16LED_CMD_CUR_ON_1[i] = device_info.u16LED_CMD_CUR_ON_1[i];
            g_u16LED_CMD_CUR_ON_2[i] = device_info.u16LED_CMD_CUR_ON_2[i];
            g_u16LED_CMD_FAULT_1[i] = device_info.u16LED_CMD_FAULT_1[i];
            g_u16LED_CMD_CURR_CTRL[i] = device_info.u16LED_CMD_CURR_CTRL[i];
            g_u16LED_CMD_FB_ON_1[i] = device_info.u16LED_CMD_FB_ON_1[i];
            g_u16LED_CMD_FB_ON_2[i] = device_info.u16LED_CMD_FB_ON_2[i];
            g_u16LED_CMD_PWM1_DLY_L[i] = device_info.u16LED_CMD_PWM1_DLY_L[i];
            g_u16LED_CMD_PWM1_DLY_H[i] = device_info.u16LED_CMD_PWM1_DLY_H[i];
            g_u16LED_CMD_PWM2_DLY_L[i] = device_info.u16LED_CMD_PWM2_DLY_L[i];
            g_u16LED_CMD_PWM2_DLY_H[i] = device_info.u16LED_CMD_PWM2_DLY_H[i];
            g_u16LED_CMD_PWM3_DLY_L[i] = device_info.u16LED_CMD_PWM3_DLY_L[i];
            g_u16LED_CMD_PWM3_DLY_H[i] = device_info.u16LED_CMD_PWM3_DLY_H[i];
            g_u16LED_CMD_PWM4_DLY_L[i] = device_info.u16LED_CMD_PWM4_DLY_L[i];
            g_u16LED_CMD_PWM4_DLY_H[i] = device_info.u16LED_CMD_PWM4_DLY_H[i];
            g_u16LED_CMD_PWM5_DLY_L[i] = device_info.u16LED_CMD_PWM5_DLY_L[i];
            g_u16LED_CMD_PWM5_DLY_H[i] = device_info.u16LED_CMD_PWM5_DLY_H[i];
            g_u16LED_CMD_PWM6_DLY_L[i] = device_info.u16LED_CMD_PWM6_DLY_L[i];
            g_u16LED_CMD_PWM6_DLY_H[i] = device_info.u16LED_CMD_PWM6_DLY_H[i];
            g_u16LED_CMD_PWM7_DLY_L[i] = device_info.u16LED_CMD_PWM7_DLY_L[i];
            g_u16LED_CMD_PWM7_DLY_H[i] = device_info.u16LED_CMD_PWM7_DLY_H[i];
            g_u16LED_CMD_PWM8_DLY_L[i] = device_info.u16LED_CMD_PWM8_DLY_L[i];
            g_u16LED_CMD_PWM8_DLY_H[i] = device_info.u16LED_CMD_PWM8_DLY_H[i];
            g_u16LED_CMD_PWM9_DLY_L[i] = device_info.u16LED_CMD_PWM9_DLY_L[i];
            g_u16LED_CMD_PWM9_DLY_H[i] = device_info.u16LED_CMD_PWM9_DLY_H[i];
            g_u16LED_CMD_PWM10_DLY_L[i] = device_info.u16LED_CMD_PWM10_DLY_L[i];
            g_u16LED_CMD_PWM10_DLY_H[i] = device_info.u16LED_CMD_PWM10_DLY_H[i];
            g_u16LED_CMD_PWM11_DLY_L[i] = device_info.u16LED_CMD_PWM11_DLY_L[i];
            g_u16LED_CMD_PWM11_DLY_H[i] = device_info.u16LED_CMD_PWM11_DLY_H[i];
            g_u16LED_CMD_PWM12_DLY_L[i] = device_info.u16LED_CMD_PWM12_DLY_L[i];
            g_u16LED_CMD_PWM12_DLY_H[i] = device_info.u16LED_CMD_PWM12_DLY_H[i];
            g_u16LED_CMD_PWM13_DLY_L[i] = device_info.u16LED_CMD_PWM13_DLY_L[i];
            g_u16LED_CMD_PWM13_DLY_H[i] = device_info.u16LED_CMD_PWM13_DLY_H[i];
            g_u16LED_CMD_PWM14_DLY_L[i] = device_info.u16LED_CMD_PWM14_DLY_L[i];
            g_u16LED_CMD_PWM14_DLY_H[i] = device_info.u16LED_CMD_PWM14_DLY_H[i];
            g_u16LED_CMD_PWM15_DLY_L[i] = device_info.u16LED_CMD_PWM15_DLY_L[i];
            g_u16LED_CMD_PWM15_DLY_H[i] = device_info.u16LED_CMD_PWM15_DLY_H[i];
            g_u16LED_CMD_PWM16_DLY_L[i] = device_info.u16LED_CMD_PWM16_DLY_L[i];
            g_u16LED_CMD_PWM16_DLY_H[i] = device_info.u16LED_CMD_PWM16_DLY_H[i];
            g_u16LED_CMD_SHORT_COMP_CTRL_1[i] = device_info.u16LED_CMD_SHORT_COMP_CTRL_1[i];

            UBOOT_DEBUG("g_u16LED_CMD_CUR_ON_1[%d] = 0x%x\n", i, g_u16LED_CMD_CUR_ON_1[i]);
            UBOOT_DEBUG("g_u16LED_CMD_CUR_ON_2[%d] = 0x%x\n", i, g_u16LED_CMD_CUR_ON_2[i]);
            UBOOT_DEBUG("g_u16LED_CMD_FAULT_1[%d] = 0x%x\n", i, g_u16LED_CMD_FAULT_1[i]);
            UBOOT_DEBUG("g_u16LED_CMD_CURR_CTRL[%d] = 0x%x\n", i, g_u16LED_CMD_CURR_CTRL[i]);
            UBOOT_DEBUG("g_u16LED_CMD_FB_ON_1[%d] = 0x%x\n", i, g_u16LED_CMD_FB_ON_1[i]);
            UBOOT_DEBUG("g_u16LED_CMD_FB_ON_2[%d] = 0x%x\n", i, g_u16LED_CMD_FB_ON_2[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM1_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM1_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM1_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM1_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM2_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM2_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM2_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM2_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM3_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM3_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM3_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM3_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM4_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM4_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM4_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM4_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM5_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM5_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM5_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM5_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM6_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM6_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM6_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM6_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM7_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM7_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM7_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM7_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM8_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM8_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM8_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM8_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM9_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM9_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM9_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM9_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM10_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM10_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM10_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM10_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM11_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM11_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM11_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM11_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM12_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM12_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM12_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM12_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM13_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM13_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM13_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM13_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM14_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM14_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM14_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM14_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM15_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM15_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM15_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM15_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM16_DLY_L[%d] = 0x%x\n", i, g_u16LED_CMD_PWM16_DLY_L[i]);
            UBOOT_DEBUG("g_u16LED_CMD_PWM16_DLY_H[%d] = 0x%x\n", i, g_u16LED_CMD_PWM16_DLY_H[i]);
            UBOOT_DEBUG("g_u16LED_CMD_SHORT_COMP_CTRL_1[%d] = 0x%x\n", i, g_u16LED_CMD_SHORT_COMP_CTRL_1[i]);
        }

        APE5030_Init();

        WriteByte(0x3229A4,0x00); //change pad mux to vsync_like
        if( DEFAULT_VSYNC == VSYNC_50)
        {
            SetPWM(LOCAL_DIMMING_PWM_VSYNC_50_Period,LOCAL_DIMMING_PWM_VSYNC_50_DutyCycle,LOCAL_DIMMING_PWM_VSYNC_Div,LOCAL_DIMMING_PWM_CH);
        }
        else if( DEFAULT_VSYNC == VSYNC_60)
        {
            SetPWM(LOCAL_DIMMING_PWM_VSYNC_60_Period,LOCAL_DIMMING_PWM_VSYNC_60_DutyCycle,LOCAL_DIMMING_PWM_VSYNC_Div,LOCAL_DIMMING_PWM_CH);
        }
        else if( DEFAULT_VSYNC == VSYNC_100)
        {
            SetPWM(LOCAL_DIMMING_PWM_VSYNC_100_Period,LOCAL_DIMMING_PWM_VSYNC_100_DutyCycle,LOCAL_DIMMING_PWM_VSYNC_Div,LOCAL_DIMMING_PWM_CH);
        }
        else if( DEFAULT_VSYNC == VSYNC_120)
        {
            SetPWM(LOCAL_DIMMING_PWM_VSYNC_120_Period,LOCAL_DIMMING_PWM_VSYNC_120_DutyCycle,LOCAL_DIMMING_PWM_VSYNC_Div,LOCAL_DIMMING_PWM_CH);
        }
        MDrv_PWM_ResetEn(LOCAL_DIMMING_PWM_CH,true);
    }
	mdrv_gpio_set_high(PAD_PWM1);
    UBOOT_TRACE("OK\n");
    return 0;
}
#endif

void do_Tcon_preload(MS_BOOL m_b_preTconOutput,MS_BOOL m_b_preODEnable)
{
    UBOOT_TRACE("IN\n");
    g_preTconOutput = m_b_preTconOutput;
    g_preODEnable = m_b_preODEnable;
    UBOOT_TRACE("OK\n");
}

MS_BOOL Get_TCON_Enable(void)
{
    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("g_preTconOutput = %d\n", g_preTconOutput);
    return g_preTconOutput;
    UBOOT_TRACE("OK\n");
}

MS_BOOL Get_OD_Enable(void)
{
    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("g_preODEnable = %d\n", g_preODEnable);
    return g_preODEnable;
    UBOOT_TRACE("OK\n");
}

MS_BOOL Check_TCON_Enable_Fromflash(void)
{
    UBOOT_TRACE("IN\n");
    st_board_para board_data;
    int ret = -1;
    ret = Read_BoardParaFromflash(&board_data);
    UBOOT_DEBUG("m_bTconOutput = %d\n", board_data.m_bTconOutput);
    UBOOT_TRACE("OK\n");
    if (ret != 0)
        return FALSE;
    else
        return board_data.m_bTconOutput;
}

MS_BOOL Check_OD_Enable_Fromflash(void)
{
    UBOOT_TRACE("IN\n");
    st_board_para board_data;
    int ret = -1;
    ret = Read_BoardParaFromflash(&board_data);
    UBOOT_DEBUG("m_bODEnable = %d\n", board_data.m_bODEnable);
    UBOOT_TRACE("OK\n");
    if(ret != 0)
        return FALSE;
    else
        return board_data.m_bODEnable;
}

#if PANEL_DEBUG
void cmp(PanelType *p1, PanelType *p2)
{
    if((p1 ==NULL) || (p2==NULL))
    {
        printf("null return\n");
        return;
    }
//    printf("compare: '%s', '%s'\n", p1->m_pPanelName, p2->m_pPanelName);
    if(p1->m_bPanelDither != p2->m_bPanelDither)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelDither, p2->m_bPanelDither, __LINE__);
    }
    if(p1->m_ePanelLinkType != p2->m_ePanelLinkType)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ePanelLinkType, p2->m_ePanelLinkType, __LINE__);
    }
    if(p1->m_bPanelDualPort != p2->m_bPanelDualPort)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelDualPort, p2->m_bPanelDualPort, __LINE__);
    }
    if(p1->m_bPanelSwapPort != p2->m_bPanelSwapPort)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapPort, p2->m_bPanelSwapPort, __LINE__);
    }
    if(p1->m_bPanelSwapOdd_ML != p2->m_bPanelSwapOdd_ML)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapOdd_ML, p2->m_bPanelSwapOdd_ML, __LINE__);
    }
    if(p1->m_bPanelSwapEven_ML != p2->m_bPanelSwapEven_ML)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapEven_ML, p2->m_bPanelSwapEven_ML, __LINE__);
    }
    if(p1->m_bPanelSwapOdd_RB != p2->m_bPanelSwapOdd_RB)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapOdd_RB, p2->m_bPanelSwapOdd_RB, __LINE__);
    }
    if(p1->m_bPanelSwapEven_RB != p2->m_bPanelSwapEven_RB)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapEven_RB, p2->m_bPanelSwapEven_RB, __LINE__);
    }
    if(p1->m_bPanelSwapLVDS_POL != p2->m_bPanelSwapLVDS_POL)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapLVDS_POL, p2->m_bPanelSwapLVDS_POL, __LINE__);
    }
    if(p1->m_bPanelSwapLVDS_CH != p2->m_bPanelSwapLVDS_CH)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapLVDS_CH, p2->m_bPanelSwapLVDS_CH, __LINE__);
    }
    if(p1->m_bPanelPDP10BIT != p2->m_bPanelPDP10BIT)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelPDP10BIT, p2->m_bPanelPDP10BIT, __LINE__);
    }
    if(p1->m_bPanelLVDS_TI_MODE != p2->m_bPanelLVDS_TI_MODE)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelLVDS_TI_MODE, p2->m_bPanelLVDS_TI_MODE, __LINE__);
    }
    if(p1->m_ucPanelDCLKDelay != p2->m_ucPanelDCLKDelay)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelDCLKDelay, p2->m_ucPanelDCLKDelay, __LINE__);
    }
    if(p1->m_bPanelInvDCLK != p2->m_bPanelInvDCLK)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelInvDCLK, p2->m_bPanelInvDCLK, __LINE__);
    }
    if(p1->m_bPanelInvDE != p2->m_bPanelInvDE)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelInvDE, p2->m_bPanelInvDE, __LINE__);
    }
    if(p1->m_bPanelInvHSync != p2->m_bPanelInvHSync)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelInvHSync, p2->m_bPanelInvHSync, __LINE__);
    }
    if(p1->m_bPanelInvVSync != p2->m_bPanelInvVSync)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelInvVSync, p2->m_bPanelInvVSync, __LINE__);
    }
    if(p1->m_ucPanelDCKLCurrent != p2->m_ucPanelDCKLCurrent)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelDCKLCurrent, p2->m_ucPanelDCKLCurrent, __LINE__);
    }
    if(p1->m_ucPanelDECurrent != p2->m_ucPanelDECurrent)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelDECurrent, p2->m_ucPanelDECurrent, __LINE__);
    }
    if(p1->m_ucPanelODDDataCurrent != p2->m_ucPanelODDDataCurrent)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelODDDataCurrent, p2->m_ucPanelODDDataCurrent, __LINE__);
    }
    if(p1->m_ucPanelEvenDataCurrent != p2->m_ucPanelEvenDataCurrent)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelEvenDataCurrent, p2->m_ucPanelEvenDataCurrent, __LINE__);
    }
    if(p1->m_wPanelOnTiming1 != p2->m_wPanelOnTiming1)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelOnTiming1, p2->m_wPanelOnTiming1, __LINE__);
    }
    if(p1->m_wPanelOnTiming2 != p2->m_wPanelOnTiming2)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelOnTiming2, p2->m_wPanelOnTiming2, __LINE__);
    }
    if(p1->m_wPanelOffTiming1 != p2->m_wPanelOffTiming1)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelOffTiming1, p2->m_wPanelOffTiming1, __LINE__);
    }
    if(p1->m_wPanelOffTiming2 != p2->m_wPanelOffTiming2)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelOffTiming2, p2->m_wPanelOffTiming2, __LINE__);
    }
    if(p1->m_ucPanelHSyncWidth != p2->m_ucPanelHSyncWidth)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelHSyncWidth, p2->m_ucPanelHSyncWidth, __LINE__);
    }
    if(p1->m_ucPanelHSyncBackPorch != p2->m_ucPanelHSyncBackPorch)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelHSyncBackPorch, p2->m_ucPanelHSyncBackPorch, __LINE__);
    }
    if(p1->m_ucPanelVSyncWidth != p2->m_ucPanelVSyncWidth)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelVSyncWidth, p2->m_ucPanelVSyncWidth, __LINE__);
    }
    if(p1->m_ucPanelVBackPorch != p2->m_ucPanelVBackPorch)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelVBackPorch, p2->m_ucPanelVBackPorch, __LINE__);
    }
    if(p1->m_wPanelHStart != p2->m_wPanelHStart)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelHStart, p2->m_wPanelHStart, __LINE__);
    }
    if(p1->m_wPanelVStart != p2->m_wPanelVStart)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelVStart, p2->m_wPanelVStart, __LINE__);
    }
    if(p1->m_wPanelWidth != p2->m_wPanelWidth)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelWidth, p2->m_wPanelWidth, __LINE__);
    }
    if(p1->m_wPanelHeight != p2->m_wPanelHeight)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelHeight, p2->m_wPanelHeight, __LINE__);
    }
    if(p1->m_wPanelMaxHTotal != p2->m_wPanelMaxHTotal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelMaxHTotal, p2->m_wPanelMaxHTotal, __LINE__);
    }
    if(p1->m_wPanelHTotal != p2->m_wPanelHTotal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelHTotal, p2->m_wPanelHTotal, __LINE__);
    }
    if(p1->m_wPanelMinHTotal != p2->m_wPanelMinHTotal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelMinHTotal, p2->m_wPanelMinHTotal, __LINE__);
    }
    if(p1->m_wPanelMaxVTotal != p2->m_wPanelMaxVTotal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelMaxVTotal, p2->m_wPanelMaxVTotal, __LINE__);
    }
    if(p1->m_wPanelVTotal != p2->m_wPanelVTotal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelVTotal, p2->m_wPanelVTotal, __LINE__);
    }
    if(p1->m_wPanelMinVTotal != p2->m_wPanelMinVTotal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wPanelMinVTotal, p2->m_wPanelMinVTotal, __LINE__);
    }
    if(p1->m_dwPanelMaxDCLK != p2->m_dwPanelMaxDCLK)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_dwPanelMaxDCLK, p2->m_dwPanelMaxDCLK, __LINE__);
    }
    if(p1->m_dwPanelDCLK != p2->m_dwPanelDCLK)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_dwPanelDCLK, p2->m_dwPanelDCLK, __LINE__);
    }
    if(p1->m_dwPanelMinDCLK != p2->m_dwPanelMinDCLK)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_dwPanelMinDCLK, p2->m_dwPanelMinDCLK, __LINE__);
    }
    if(p1->m_wSpreadSpectrumStep != p2->m_wSpreadSpectrumStep)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wSpreadSpectrumStep, p2->m_wSpreadSpectrumStep, __LINE__);
    }
    if(p1->m_wSpreadSpectrumSpan != p2->m_wSpreadSpectrumSpan)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_wSpreadSpectrumSpan, p2->m_wSpreadSpectrumSpan, __LINE__);
    }
    if(p1->m_ucDimmingCtl != p2->m_ucDimmingCtl)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucDimmingCtl, p2->m_ucDimmingCtl, __LINE__);
    }
    if(p1->m_ucMaxPWMVal != p2->m_ucMaxPWMVal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucMaxPWMVal, p2->m_ucMaxPWMVal, __LINE__);
    }
    if(p1->m_ucMinPWMVal != p2->m_ucMinPWMVal)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucMinPWMVal, p2->m_ucMinPWMVal, __LINE__);
    }
    if(p1->m_bPanelDeinterMode != p2->m_bPanelDeinterMode)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelDeinterMode, p2->m_bPanelDeinterMode, __LINE__);
    }
    if(p1->m_ucPanelAspectRatio != p2->m_ucPanelAspectRatio)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucPanelAspectRatio, p2->m_ucPanelAspectRatio, __LINE__);
    }
    if(p1->m_u16LVDSTxSwapValue != p2->m_u16LVDSTxSwapValue)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_u16LVDSTxSwapValue, p2->m_u16LVDSTxSwapValue, __LINE__);
    }
    if(p1->m_ucTiBitMode != p2->m_ucTiBitMode)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucTiBitMode, p2->m_ucTiBitMode, __LINE__);
    }
    if(p1->m_ucOutputFormatBitMode != p2->m_ucOutputFormatBitMode)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucOutputFormatBitMode, p2->m_ucOutputFormatBitMode, __LINE__);
    }
    if(p1->m_bPanelSwapOdd_RG != p2->m_bPanelSwapOdd_RG)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapOdd_RG, p2->m_bPanelSwapOdd_RG, __LINE__);
    }
    if(p1->m_bPanelSwapEven_RG != p2->m_bPanelSwapEven_RG)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapEven_RG, p2->m_bPanelSwapEven_RG, __LINE__);
    }
    if(p1->m_bPanelSwapOdd_GB != p2->m_bPanelSwapOdd_GB)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapOdd_GB, p2->m_bPanelSwapOdd_GB, __LINE__);
    }
    if(p1->m_bPanelSwapEven_GB != p2->m_bPanelSwapEven_GB)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelSwapEven_GB, p2->m_bPanelSwapEven_GB, __LINE__);
    }
    if(p1->m_bPanelDoubleClk != p2->m_bPanelDoubleClk)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelDoubleClk, p2->m_bPanelDoubleClk, __LINE__);
    }
    if(p1->m_dwPanelMaxSET != p2->m_dwPanelMaxSET)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_dwPanelMaxSET, p2->m_dwPanelMaxSET, __LINE__);
    }
    if(p1->m_dwPanelMinSET != p2->m_dwPanelMinSET)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_dwPanelMinSET, p2->m_dwPanelMinSET, __LINE__);
    }
    if(p1->m_ucOutTimingMode != p2->m_ucOutTimingMode)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_ucOutTimingMode, p2->m_ucOutTimingMode, __LINE__);
    }
    if(p1->m_bPanelNoiseDith != p2->m_bPanelNoiseDith)
    {
        printf("diff: '%u', '%u', at %u\n", p1->m_bPanelNoiseDith, p2->m_bPanelNoiseDith, __LINE__);
    }
}
#endif //PANEL_DEBUG
