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
//=============================================================================
// Include Files
//=============================================================================
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/pfn.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/uaccess.h>

#ifndef MSOS_TYPE_LINUX_KERNEL
#define MSOS_TYPE_LINUX_KERNEL
#endif

#ifndef MI_ENABLE_DBG
#define MI_ENABLE_DBG  1
#endif

#ifndef MI_OS_WRAPPER_PRINTF
#define MI_OS_WRAPPER_PRINTF 1
#endif
//mi header file
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_os.h"

//driver header files
#include "mstar_chip.h"
#include "mdrv_graphic_adapter.h"
#include "mdrv_graphic_fbdev.h"
#include "mdrv_graphic_virtualfb.h"
//ini parser.h
#include "iniparser.h"
#include "uapi/mstarFb.h"


#define LOG_TAG "[FBDEV] "

#define MAX_SURFACE_CNT 3
#define ALIGN_DOWNTO_16(_val_) (((_val_) >> 4) << 4)
//TODO: ALIGN_UP do nothing temp
//#define ALIGN_UP(value,factor) ((value + factor-1) & (~(factor-1)))
#define ALIGN_UP(value,factor) value
#define MIU0_BUS_OFFSET   ARM_MIU0_BUS_BASE
#define MIU1_BUS_OFFSET   ARM_MIU1_BUS_BASE
#define MIU2_BUS_OFFSET   ARM_MIU2_BUS_BASE
#define MIU1_INTERVAL     ARM_MIU1_BASE_ADDR
#define MIU2_INTERVAL     ARM_MIU2_BASE_ADDR
#define CHECK_IFNOT(exp, dst, ret) if ((exp) != (dst)) { return ret; }
#define CHECK_NULL_RET(x) \
    do { \
        if (x==NULL)\
        {\
            printk("\33[0;36m %s:%d NULL item check! \33[m \n",__FUNCTION__,__LINE__); \
        }\
       }while(false)


#define PALETTE_TABLE_SIZE 256

static struct fb_ops mstar_fb_ops =
{
    .owner = THIS_MODULE,
    .fb_open = mstar_fb_open,
    .fb_release = mstar_fb_release,
    .fb_mmap = mstar_fb_mmap,
    .fb_set_par = mstar_fb_set_par,
    .fb_check_var = mstar_fb_check_var,
    .fb_setcmap = mstar_fb_setcmap,
    .fb_blank = mstar_fb_blank,
    .fb_pan_display = mstar_fb_pan_display,
    .fb_setcolreg = mstar_fb_setcolreg,
    .fb_fillrect = mstar_fb_fillrect,
    .fb_copyarea = mstar_fb_copyarea,
    .fb_imageblit = mstar_fb_imageblit,
    .fb_destroy = mstar_fb_destroy,
    .fb_ioctl = mstar_fb_ioctl,
    .fb_compat_ioctl = mstar_fb_ioctl,
};

struct fb_ops virtaulfb_ops =  {
    .owner = THIS_MODULE,
    .fb_open = virtualfb_open,
    .fb_release = virtualfb_release,
    .fb_mmap = virtualfb_mmap,
    .fb_set_par = virtualfb_set_par,
    .fb_check_var = virtualfb_check_var,
    .fb_setcmap = virtualfb_setcmap,
    .fb_blank = virtualfb_blank,
    .fb_pan_display = virtualfb_pan_display,
    .fb_setcolreg = virtualfb_setcolreg,
    .fb_fillrect = virtualfb_fillrect,
    .fb_copyarea = virtualfb_copyarea,
    .fb_imageblit = virtualfb_imageblit,
    .fb_destroy = virtualfb_destroy,
    .fb_ioctl = virtualfb_ioctl,
    .fb_compat_ioctl = virtualfb_ioctl,
};

//-------------------------------------------------------------------------------------------------
//  MstarFB sturct and function
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MI_HANDLE hLayerHandle;
    MI_OSD_LayerInfo_t stLayerInfo;
    MI_BOOL bShown;
    MI_BOOL bPremultiply;
    MI_BOOL bEnableMultiAlpha;
    MI_BOOL bEnableClrKey;
    MI_OSD_TransparentColor_t stTransparentColor;
    MI_U16 brightness;
    MI_OSD_ContrastColor_t contrastcolor;
}MI_FBDEV_LayerCtrl_t;

typedef struct
{
    MI_HANDLE hWindowHandle;
    MI_OSD_WindowInfo_t stWindowInfo;
    MI_U8 u8GlobalAlpha;
}MI_FBDEV_WindowCtrl_t;

typedef struct
{
    MI_HANDLE hSurfaceHandle;
    MI_OSD_SurfaceInfo_t stSurfaceInfo;
}MI_FBDEV_SurfaceCtrl_t;

typedef struct
{
    MI_FBDEV_LayerCtrl_t stFbdevLayerCtrl;
    MI_FBDEV_WindowCtrl_t stFbdevWindowCtrl;
    MI_FBDEV_SurfaceCtrl_t stFbdevSurfaceCtrl[MAX_SURFACE_CNT];
    MI_FBDEV_SurfaceCtrl_t *pTempFbdevSurfaceCtrl;
    //set Block meminfo while fliping
    MI_BOOL bUseExBlockMem;
    //Memory Info
    MI_PHY phyAddr;
    //Memory Info for iommu  IOMMUphyaddr = IOVA - 0x200000000
    MI_PHY IOMMUphyaddr;
    MI_U32 length;
    MI_SYS_MiuType_e eMiuSel;
    //reserved for dfb
    MI_BOOL bReserveDFB;
    //open reference count
    MI_U32 ref_count;
    //Surface count Double buffer or triple buffer
    MI_U8 surfaceCnt;
    //surface flip mode queue or waitsync
    MI_FB_FlipMode_e u8FlipMode;
    //swicth for GOP Auto detect
    MI_BOOL bEnable_GOP_AutoDetect;
    //buffercnt for flip by queue mode,not alaways less than surfaceCnt
    MI_U8 u8QueueBufferCnt;
    //Has been initialized
    MI_U8 bInitialized;
    //used to store pseudo_palette
    MI_U32 pseudo_palette[16];
    //flag which com from OsdDefine.ini name FB_VIRTUAL = 1
    MI_U8  u8IsVirtualFB;
    //whether fb device use iommu or not
    MI_U8  u8UseIOMMU;
}MI_FBDEV_Controller_t;

typedef struct
{
    MI_U8 u8Blue;
    MI_U8 u8Green;
    MI_U8 u8Red;
    MI_U8 u8Alpha;
} MI_FBDEV_RgbColor_t;

static const char* SYSINI_PATH = "/vendor/tvconfig/config/sys.ini";

static const char* AN_OSDDEFINE_PATH = "/vendor/tvconfig/config/OSD/OsdDefine.ini";
static const char* LINUX_OSDDEFINE_PATH = "/config/OSD/OsdDefine.ini";

static const char* fbdev_setction_name = "FBDEV";
static const char* bootlogo_section_name = "MBOOTLOGO";
static const char* bootargsfile = "/proc/cmdline";

static const char* SysConfigSectinName = "sys_config";
static const char* OsdDefineKeyName = "gOSDDefineConfigINI";


//Macro for get OSD mirror info
#define MI_KEYNAMELENGTH_MAX 100
#define MODELPARAM_MAX 2
#define MI_INI_KEYNAME_MODELNAME  "model:gModelName"
static char* pMIRRORINI_KEYNAME[] = {"MISC_MIRROR_CFG:MIRROR_OSD","MISC_MIRROR_CFG:MIRROR_OSD_TYPE"};
// 0:normaltype 1: Hmirror only 2:Vmirroronly 3:HV-mirror
static int MIRRORPARAM[MODELPARAM_MAX] = {0};

static struct fb_fix_screeninfo* mstar_fb_fix_infos  = NULL;
static struct fb_var_screeninfo* mstar_fb_var_infos = NULL;
static MI_FBDEV_Controller_t* fbdevControllers = NULL;
static int numFbHwlayer = 0;
static int first_fb_node = 0;
static int gBootlogoIdx = -1;


//Flag for bootlogo disable info
static MI_BOOL bBootlogoDisabled = FALSE;
static MI_BOOL bBootlogoOpened=FALSE;
//For bBootLogostatus status -1: error 0: turn off 1: turn on
int bBootLogostatus = -1;
static struct fb_info *stBootlogoInfo=NULL;

//palette table use in I8 mode
MI_FBDEV_RgbColor_t u32I8Palette_a[PALETTE_TABLE_SIZE] = {{0,0,0,0}};

#define IOMMU_PHYADDROFFSET  0x200000000
#define IOMMU_DEFAULT_COUNT  2

//force turn on bootlogo
#define FORCEBOOTLOGOON   1

#define SURFACEPATTERN 0

#if (ENABLE_GFLIP_IOCTL == TRUE)
extern MI_BOOL MDrv_GFLIP_WaitForVsync(MI_U32 u32GopIdx);
#endif


/*using ioremap mapping physical address to kernelVA will occupy Vmalloc zone,*/
/* bUseMapping = 1 mapping to kernel VA for dumpbuffer*/
/* bUseMapping = 0 default value ,not mapping */
int bUseMapping = 0;

static int mstar_fb_set_par(struct fb_info *pinfo)
{
    struct fb_var_screeninfo *var = &pinfo->var;
    int bits_per_pixel = 32;
    u32 u32Stride = 7680;
    switch (var->bits_per_pixel)
    {
        case 32:
        case 16:
            pinfo->fix.visual = FB_VISUAL_TRUECOLOR;
            bits_per_pixel = var->bits_per_pixel;
            break;
        case 1:
            pinfo->fix.visual = FB_VISUAL_MONO01;
            bits_per_pixel = 8;
            break;
        default:
            pinfo->fix.visual = FB_VISUAL_PSEUDOCOLOR;
            bits_per_pixel = 8;
            break;
    }
    u32Stride = (var->xres_virtual * bits_per_pixel) / 8;
    pinfo->fix.line_length = ALIGN_UP(u32Stride, 64);
    pinfo->fix.xpanstep = 1;
    /* activate this new configuration */
    return 0;
}


static int mstar_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{

    unsigned int line_length,bits_per_pixel;
    MI_OSD_ColorFormat_e ColorFmt = E_MI_OSD_COLOR_FORMAT_GENERIC;
    /*
     *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
     *  as FB_VMODE_SMOOTH_XPAN is only used internally
     */
     if (var->vmode & FB_VMODE_CONUPDATE)
     {
        var->vmode |= FB_VMODE_YWRAP;
        var->xoffset = info->var.xoffset;
        var->yoffset = info->var.yoffset;
     }
     //Alignment xres and xres_virtual down to 16 pixel and stored it
     var->xres = ALIGN_DOWNTO_16(var->xres);
     var->xres_virtual = ALIGN_DOWNTO_16(var->xres_virtual);
      /*
      *  Some very basic checks
      */
    if (!var->xres)
        var->xres = info->var.xres;
    if (!var->yres)
        var->yres = info->var.yres;
    if (var->xres > var->xres_virtual)
        var->xres_virtual = var->xres;
    if (var->yres > var->yres_virtual)
        var->yres_virtual = var->yres;
    if (var->bits_per_pixel <= 1)
    {
        var->bits_per_pixel = 1;
        bits_per_pixel = 8;
    }else if (var->bits_per_pixel <= 8)
    {
        var->bits_per_pixel = 8;
        bits_per_pixel = 8;
     }else if (var->bits_per_pixel <= 16)
     {
        var->bits_per_pixel = 16;
        bits_per_pixel = 16;
     }else if (var->bits_per_pixel <= 32)
     {
        var->bits_per_pixel = 32;
        bits_per_pixel = 32;
     }else
         return -EINVAL;
    if (var->xres_virtual < var->xoffset + var->xres)
        var->xres_virtual = var->xoffset + var->xres;
    if (var->yres_virtual < var->yoffset + var->yres)
        var->yres_virtual = var->yoffset + var->yres;
    /*
     *  Memory limit
     */
    line_length = get_line_length(var->xres_virtual,bits_per_pixel);
    if(line_length * var->yres_virtual > info->fix.smem_len)
        return -ENOMEM;
    /*
     * Now that we checked it we alter var. The reason being is that the video
     * mode passed in might not work but slight changes to it might make it
     * work. This way we let the user know what is acceptable.
     */
    ColorFmt = get_color_fmt(var);
    if(ColorFmt == E_MI_OSD_COLOR_FORMAT_GENERIC)
        return -EINVAL;
    return 0;
}

static int mstar_fb_setcmap(struct fb_cmap * cmap,struct fb_info * info)
{
    return 0;
}

static int mstar_fb_blank(int blank, struct fb_info *info)
{
    return 0;
}

static int mstar_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *pinfo)
{
    MI_FBDEV_Controller_t* par = pinfo->par;
    MI_U16 u16FbWidth = par->stFbdevWindowCtrl.stWindowInfo.u32SurfaceWidth;
    MI_U16 u16FbHeight = par->stFbdevWindowCtrl.stWindowInfo.u32SurfaceHeight;
    MI_BOOL bShown = FALSE;
    MI_OSD_LayerInfo_t stLayerInfo;
    MI_OSD_WindowInfo_t stWindowInfo;
    MI_OSD_SurfaceInfo_t stSurfacInfo;
    MI_HANDLE hWindow = MI_HANDLE_NULL;
    MI_BOOL result = TRUE;
    MI_HANDLE hRetSurface = MI_HANDLE_NULL;
    MI_HANDLE hSurfaces[MAX_SURFACE_CNT] = {MI_HANDLE_NULL};
    MI_OSD_ColorFormat_e eclrfmt = get_color_fmt(&pinfo->var);
    MI_PHY phyaddr2Flip = par->phyAddr;
    int surfacecnt = MAX_SURFACE_CNT;
    MI_U32 bytes_per_pixel;
    MI_U32 curTimingWidth=1920;
    MI_U32 curTimingHeight=1080;
    //Effective buffercnt,not always equal to surfacecnt,set by FBIOGET_VSCREENINFO
    MI_U8  u8Usedbuffercnt=0;
    MI_BOOL bSupportPalette = FALSE;
    MI_BOOL bUpdateGwin = FALSE;
    int i = 0;

    //invoked by linux framebuffer framework fbcon notify
    if (!par->ref_count)  {
        pr_info(LOG_TAG "%s fb:%d invoked by fbcon notify do nothing\n",__FUNCTION__,pinfo->node);
        return 0;
    }

    if (!par->bInitialized)  {
        pr_err(LOG_TAG "%s fb%d is not initialized!\n",__FUNCTION__,pinfo->node);
        return -EPERM;
    }

    if (!phyaddr2Flip)  {
        pr_err(LOG_TAG "%s fb%d is not set meminfo!\n",__FUNCTION__,pinfo->node);
        return -ENOMEM;
    }

    /*
      var.yres_virtual/var->yres : user set surfacecount, set in ioctl varinfo para
      par->surfaceCnt :original surfacecnt  in general is initial surface count(calc by MMAP meminfo)
      var.yres_virtual/var->yres > par->surfaceCnt : need enter in update customer size flow
      var.yres_virtual/var->yres < par->surfaceCnt : only flip part of surface surfacecnt keep original par->surfaceCnt
    */
    memcpy(&stLayerInfo, &par->stFbdevLayerCtrl.stLayerInfo, sizeof(MI_OSD_LayerInfo_t));
    memcpy(&stWindowInfo, &par->stFbdevWindowCtrl.stWindowInfo, sizeof(MI_OSD_WindowInfo_t));
    mstar_FB_getCurTiming(&curTimingWidth, &curTimingHeight);
    if ((stLayerInfo.stLayerCustomSize.u32DstWidth != curTimingWidth) ||    \
        (stLayerInfo.stLayerCustomSize.u32DstHeight != curTimingHeight) ||  \
        (var->xres != u16FbWidth) || (var->yres != u16FbHeight) ||          \
        (pinfo->var.yres_virtual > u16FbHeight*par->surfaceCnt) ||         \
        (eclrfmt != par->stFbdevWindowCtrl.stWindowInfo.eColorFormat) ||    \
        (par->bUseExBlockMem))  {
            pr_info(LOG_TAG "fb%d  UpdatelayerCustomerSize !! \n",pinfo->node);
            pr_info(LOG_TAG "fb%d  Original LayerDstWidth:%d LayerDstHeight:%d LayerWidth:%d LayerHeight:%d colorformat:%d SurfaceCount:%d UseBlockMemory:%d \n",\
                                       pinfo->node,stLayerInfo.stLayerCustomSize.u32DstWidth,stLayerInfo.stLayerCustomSize.u32DstHeight,u16FbWidth,u16FbHeight,\
                                       par->stFbdevWindowCtrl.stWindowInfo.eColorFormat,par->surfaceCnt,par->bUseExBlockMem);
            pr_info(LOG_TAG "fb%d  Current  LayerDstWidth:%d LayerDstHeight:%d LayerWidth:%d LayerHeight:%d colorformat:%d SurfaceCount:%d UseBlockMemory:%d \n",\
                                       pinfo->node,curTimingWidth,curTimingHeight,var->xres,var->yres,eclrfmt,var->yres_virtual/var->yres,par->bUseExBlockMem);

            if ((var->xres > u16FbWidth) && (var->yres > u16FbHeight))
            {
                bUpdateGwin = TRUE;
            }

            mstar_FB_BeginTransaction(par->stFbdevLayerCtrl.hLayerHandle);

            //Disable GWIN for SetLayerCustomerInfo
            _fb_gwin_enable(par->stFbdevLayerCtrl.hLayerHandle, FALSE);

            /*for (i = 0; i < par->surfaceCnt; i++)  {
                if (par->stFbdevSurfaceCtrl[i].hSurfaceHandle != MI_HANDLE_NULL)  {
                    mstar_FB_SurfaceDestroy(par->stFbdevSurfaceCtrl[i].hSurfaceHandle);
                    par->stFbdevSurfaceCtrl[i].hSurfaceHandle = MI_HANDLE_NULL;
                }
            }*/

            stLayerInfo.stLayerCustomSize.u32X = 0;
            stLayerInfo.stLayerCustomSize.u32Y = 0;
            stLayerInfo.stLayerCustomSize.u32LayerWidth = var->xres;
            stLayerInfo.stLayerCustomSize.u32LayerHeight = var->yres;
            stLayerInfo.stLayerCustomSize.u32DstWidth = curTimingWidth;
            stLayerInfo.stLayerCustomSize.u32DstHeight = curTimingHeight;
            result = mstar_FB_LayerSetCustomerSize(par->stFbdevLayerCtrl.hLayerHandle,&stLayerInfo.stLayerCustomSize);
            if (!result)  {
                pr_err(LOG_TAG "fb%d  UpdatelayerCustomerSize failed !! \n",pinfo->node);
            }

            if (par->stFbdevWindowCtrl.hWindowHandle != MI_HANDLE_NULL)  {
                mstar_FB_WindowDestroy(par->stFbdevWindowCtrl.hWindowHandle);
                par->stFbdevWindowCtrl.hWindowHandle = MI_HANDLE_NULL;
            }
            for (i = 0; i < par->surfaceCnt; i++)  {
                if (par->stFbdevSurfaceCtrl[i].hSurfaceHandle != MI_HANDLE_NULL)  {
                    mstar_FB_SurfaceDestroy(par->stFbdevSurfaceCtrl[i].hSurfaceHandle);
                    par->stFbdevSurfaceCtrl[i].hSurfaceHandle = MI_HANDLE_NULL;
                }
            }

            stWindowInfo.stRect.u32Width = var->xres;
            stWindowInfo.stRect.u32Height = var->yres;
            stWindowInfo.u32SurfaceWidth = var->xres;
            stWindowInfo.u32SurfaceHeight = var->yres;
            stWindowInfo.eColorFormat = eclrfmt;
            result = mstar_FB_WindowCreate(&stWindowInfo, &hWindow);
            if (!result)  {
                pr_err(LOG_TAG "fb%d  mstar_FB_WindowCreate failed !! \n",pinfo->node);
            }

            surfacecnt = (var->yres_virtual / var->yres);
            surfacecnt = (surfacecnt > MAX_SURFACE_CNT) ? MAX_SURFACE_CNT : surfacecnt;
            stSurfacInfo.bReArrange = FALSE;
            stSurfacInfo.eColorFormat = eclrfmt;
            stSurfacInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
            stSurfacInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;
            stSurfacInfo.u32Width = var->xres;
            stSurfacInfo.u32Height= var->yres;
            stSurfacInfo.u32Pitch = pinfo->fix.line_length;
            for (i = 0; i < surfacecnt; i++)  {
                stSurfacInfo.phyAddr = par->phyAddr + i * stSurfacInfo.u32Pitch *  var->yres;
                result = mstar_FB_SurfaceCreate(&stSurfacInfo, &hRetSurface);
                if (!result)  {
                    pr_err(LOG_TAG "fb%d  Surface:%d phyaddr:%llx Create failed!! \n",pinfo->node,i,stSurfacInfo.phyAddr);
                    break;
                }
                hSurfaces[i] = hRetSurface;
                result = mstar_FB_SurfaceClear(hRetSurface);
                if (!result)  {
                    pr_err(LOG_TAG "fb%d  Surface:%x surfaceclear failed!! \n",pinfo->node,hRetSurface);
                    break;
                }
            }

            if (!result){
                pr_err(LOG_TAG "fb%d  MI operation failed,return -EINVAL\n",pinfo->node);
                for (i = 0; i < surfacecnt; i++) {
                    if (hSurfaces[i] != MI_HANDLE_NULL)
                        mstar_FB_SurfaceDestroy(hSurfaces[i]);
                }

                if (hWindow != MI_HANDLE_NULL) {
                    mstar_FB_WindowDestroy(hWindow);
                }

                mstar_FB_EndTransaction(par->stFbdevLayerCtrl.hLayerHandle);
                return -EINVAL;
            }

            mstar_FB_EndTransaction(par->stFbdevLayerCtrl.hLayerHandle);
            mstar_FB_GetMiConfigUpdated(par->stFbdevLayerCtrl.hLayerHandle);
            //Update Layer,Window,Surface info
            memcpy(&par->stFbdevLayerCtrl.stLayerInfo, &stLayerInfo, sizeof(MI_OSD_LayerInfo_t));
            memcpy(&par->stFbdevWindowCtrl.stWindowInfo, &stWindowInfo, sizeof(MI_OSD_WindowInfo_t));
            par->stFbdevWindowCtrl.hWindowHandle = hWindow;
            par->surfaceCnt = surfacecnt;
            par->stFbdevLayerCtrl.bShown = FALSE;
            for (i=0; i < surfacecnt; i++)  {
                stSurfacInfo.phyAddr = par->phyAddr + i * stSurfacInfo.u32Pitch *  var->yres;
                memcpy(&(par->stFbdevSurfaceCtrl[i].stSurfaceInfo),&stSurfacInfo, sizeof(MI_OSD_SurfaceInfo_t));
                par->stFbdevSurfaceCtrl[i].hSurfaceHandle = hSurfaces[i];
            }
            par->bUseExBlockMem = FALSE;
        }

        par->u8QueueBufferCnt = var->yres_virtual / var->yres;
    if( eclrfmt == E_MI_OSD_COLOR_FORMAT_I8 && pinfo->cmap.len > 100 )  {
       memset(u32I8Palette_a,0x00,sizeof(u32I8Palette_a));
       for( i = 0;i < PALETTE_TABLE_SIZE;i++ )  {
    #if( FIXED_PALETTE == 1) //BGRA
         u32I8Palette_a[i].u8Blue = 0x00;
         u32I8Palette_a[i].u8Green = 0x00;
         u32I8Palette_a[i].u8Red = 0xFF;
         u32I8Palette_a[i].u8Alpha = 0xFF;
   #else
         u32I8Palette_a[i].u8Blue = pinfo->cmap.blue[i];
         u32I8Palette_a[i].u8Green = pinfo->cmap.green[i];
         u32I8Palette_a[i].u8Red = pinfo->cmap.red[i];
         u32I8Palette_a[i].u8Alpha = pinfo->cmap.transp[i];
   #endif
       }

       mstar_FB_LayerSupportPalette(par->stFbdevLayerCtrl.hLayerHandle,&bSupportPalette);
       if(bSupportPalette)  {
           result = mstar_FB_SetLayerPalette(par->stFbdevLayerCtrl.hLayerHandle,(MI_U32*)u32I8Palette_a);
           if (!result)
               pr_err(LOG_TAG "set Layer[%d] palette error !!\n",stLayerInfo.eLayerId);
       }
       else
          pr_err(LOG_TAG "layer[%d] not support palette,skip it \n",stLayerInfo.eLayerId);
    }

    //find surface need to flip by phyaddres
    if (var->bits_per_pixel == 1)
        bytes_per_pixel = 1;
    else
        bytes_per_pixel = (var->bits_per_pixel) >> 3;
    if (var->xoffset || var->yoffset)  {
        phyaddr2Flip +=  (var->xoffset + var->xres_virtual * var->yoffset)* bytes_per_pixel;
    }

    hRetSurface = MI_HANDLE_NULL;
    u8Usedbuffercnt = par->u8QueueBufferCnt;

    for (i= 0; i < par->surfaceCnt; i++)  {
        if (par->stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr == phyaddr2Flip) {
            hRetSurface = par->stFbdevSurfaceCtrl[i].hSurfaceHandle;
            break;
        }
    }

    //pr_info(LOG_TAG "%s fb%d u8QueueBufferCnt is %d  Flip buffer index is %d\n",__FUNCTION__,pinfo->node,u8Usedbuffercnt,var->yoffset/var->yres);
    //pr_info(LOG_TAG "%s fb%d phyaddr2Flip is  %llx hRetSurface= %x\n",__FUNCTION__,pinfo->node,phyaddr2Flip,hRetSurface);

    //create temp surface if can not find in par->stFbdevSurfaceCtrl array by physical addr
    if (hRetSurface == MI_HANDLE_NULL)  {
        if (!par->pTempFbdevSurfaceCtrl)  {
            par->pTempFbdevSurfaceCtrl = kzalloc(sizeof(MI_FBDEV_SurfaceCtrl_t), GFP_KERNEL);
        }
        //destroy original surface
        else if (par->pTempFbdevSurfaceCtrl->hSurfaceHandle != MI_HANDLE_NULL)  {
            mstar_FB_SurfaceDestroy(par->pTempFbdevSurfaceCtrl->hSurfaceHandle);
        }
        if(!par->pTempFbdevSurfaceCtrl) {
            pr_err(LOG_TAG "%s pTempFbdevSurfaceCtrl is null \n",__FUNCTION__);
            return -EINVAL;
        }
        //create temp surface
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.bReArrange = FALSE;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.eColorFormat = eclrfmt;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.phyAddr = phyaddr2Flip;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.u32Width = var->xres;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.u32Height = var->yres;
        par->pTempFbdevSurfaceCtrl->stSurfaceInfo.u32Pitch = pinfo->fix.line_length;
        mstar_FB_SurfaceCreate( &par->pTempFbdevSurfaceCtrl->stSurfaceInfo, &hRetSurface);
        par->pTempFbdevSurfaceCtrl->hSurfaceHandle = hRetSurface;
        pr_info(LOG_TAG "%s Create tempsurface:%x  \n",__FUNCTION__,hRetSurface);
    }

    if (hRetSurface == MI_HANDLE_NULL)  {
        pr_err(LOG_TAG "%s error flip surfaceHandle is null \n",__FUNCTION__);
        return -EINVAL;
    }

    if( par->u8FlipMode == E_MI_FB_FlipMode_Queue )  {
       if(!mstar_FB_WindowFlipByExternQueue(par->stFbdevWindowCtrl.hWindowHandle, hRetSurface,u8Usedbuffercnt))  {
           pr_err(LOG_TAG "%s mstar_FB_WindowFlipByExternQueue failed!\n",__FUNCTION__);
           return -EINVAL;
       }
    }
    else if( par->u8FlipMode == E_MI_FB_FlipMode_WaitSync )  {
          mstar_FB_BeginTransaction(par->stFbdevLayerCtrl.hLayerHandle);
         if(!mstar_FB_WindowFlipByExternSurface(par->stFbdevWindowCtrl.hWindowHandle, hRetSurface))  {
            pr_err(LOG_TAG "%s mstar_FB_WindowFlipByExternSurface failed!\n",__FUNCTION__);
            return -EINVAL;
         }
          mstar_FB_EndTransaction(par->stFbdevLayerCtrl.hLayerHandle);
          mstar_FB_GetMiConfigUpdated(par->stFbdevLayerCtrl.hLayerHandle);
    }
    else  {
         pr_err(LOG_TAG "Unknow Flip mode Error !! \n");
         return -EINVAL;
    }

    if (bUpdateGwin)
    {
        bShown = par->stFbdevLayerCtrl.bShown;
        if (bShown == FALSE)  {
            _fb_gwin_enable(par->stFbdevLayerCtrl.hLayerHandle, TRUE);
            par->stFbdevLayerCtrl.bShown = TRUE;
        }
    }
    return 0;
}


//It's not necessary to support pseudo_palette
static int mstar_fb_setcolreg(unsigned regno, unsigned red, unsigned green,
                              unsigned blue, unsigned transp, struct fb_info *info)
{
    /* grayscale works only partially under directcolor */
    if(info->var.grayscale)
    {
        /* grayscale = 0.30*R + 0.59*G + 0.11*B */
        red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;
    }

    if(info->fix.visual == FB_VISUAL_TRUECOLOR || info->fix.visual == FB_VISUAL_DIRECTCOLOR)
    {
        u32 v;

        if(regno >= 16)
            return -EINVAL;

        v = (red << info->var.red.offset) | (green << info->var.green.offset) | (blue << info->var.blue.offset) | (transp << info->var.transp.offset);
            ((u32*)(info->pseudo_palette))[regno] = v;
    }
    return 0;
}

static void mstar_fb_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
#ifdef CONFIG_FB_VIRTUAL
         // printk("Donnot use virtual framebuffer skip it !!\n");
        //sys_fillrect(p, rect);
#endif
}

static void mstar_fb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
#ifdef CONFIG_FB_VIRTUAL
        //  printk("Donnot use virtual framebuffer skip it !!\n");
        //sys_copyarea(p, area);
#endif
}
static void mstar_fb_imageblit(struct fb_info *p, const struct fb_image *image)
{
#ifdef CONFIG_FB_VIRTUAL
         //  printk("Donnot use virtual framebuffer skip it !!\n");
        // sys_imageblit(p, image);  // donnot use virtualFramebuffer  so comment it
#endif
}
static void mstar_fb_destroy(struct fb_info *info)
{
    MI_FBDEV_Controller_t* par = info->par;
    pr_info(LOG_TAG "%s fb%d destroy fbdevice info  \n",__FUNCTION__,info->node);
    _fb_miosd_deinit(info);

    if(info->screen_base && !par->u8UseIOMMU && bUseMapping) {
      iounmap(info->screen_base);
      info->screen_base = NULL;
    }
}
static unsigned int get_line_length(int xres_virtual, int bpp)
{
    unsigned int length;

    length = xres_virtual * bpp;
    length = (length + 31) & ~31;
    length >>= 3;

    return (length);
}

static MI_OSD_ColorFormat_e get_color_fmt(struct fb_var_screeninfo *var)
{
    MI_OSD_ColorFormat_e ColorFmt = E_MI_OSD_COLOR_FORMAT_GENERIC;
    switch (var->bits_per_pixel) {
        case 1:
        case 8:
            var->red.offset = 0;
            var->red.length = 8;
            var->green.offset = 0;
            var->green.length = 8;
            var->blue.offset = 0;
            var->blue.length = 8;
            var->transp.offset = 0;
            var->transp.length = 0;
            ColorFmt = E_MI_OSD_COLOR_FORMAT_I8;
            break;
        case 16:
            if ( (var->transp.length) && (var->transp.offset == 15))
            {
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 5;
                var->red.offset = 10;
                var->red.length = 5;
                var->transp.offset = 15;
                var->transp.length = 1;
                ColorFmt = E_MI_OSD_COLOR_FORMAT_ARGB1555;
            }else if ((var->transp.length) && (var->transp.offset == 12))
            {
                var->blue.offset = 0;
                var->blue.length = 4;
                var->green.offset = 4;
                var->green.length = 4;
                var->red.offset = 8;
                var->red.length = 4;
                var->transp.offset = 12;
                var->transp.length = 4;
                ColorFmt = E_MI_OSD_COLOR_FORMAT_ARGB4444;
            }else {
                 /* RGB 565 */
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 6;
                var->red.offset = 11;
                var->red.length = 5;
                var->transp.offset = 0;
                var->transp.length = 0;
                ColorFmt = E_MI_OSD_COLOR_FORMAT_RGB565;
            }
            break;
            case 32:/* ARGB 8888 */
            {
                var->red.offset = 0;
                var->red.length = 8;
                var->green.offset = 8;
                var->green.length = 8;
                var->blue.offset = 16;
                var->blue.length = 8;
                var->transp.offset = 24;
                var->transp.length = 8;
                ColorFmt = E_MI_OSD_COLOR_FORMAT_ARGB8888;
            }
            break;
    }
    var->red.msb_right = 0;
    var->green.msb_right = 0;
    var->blue.msb_right = 0;
    var->transp.msb_right = 0;
    return ColorFmt;
}

//-------------------------------------------------------------------------------------------------
// Restore FbInfo when open /dev/fb
//-------------------------------------------------------------------------------------------------
static u8 getBufferCount(u32 fbLength, u16 width, u16 height, u16 bpx)
{
    u8 bufferCount = 0;
    u32 bufferSize = height* ALIGN_UP(width * bpx, 64);
    if (fbLength / bufferSize >= 3) {
        bufferCount = 3;
    } else {
        bufferCount = fbLength / bufferSize;
    }
    return bufferCount;
}

/*
  return -1 : getKeyValue fail or KeyName not set
  others : value of KeyName with int value
*/
static int _fb_GetIniKeyValue(IniSectionNode *SectionNode,const char *KeyName) {
    int Retval = -1;
    const char* StrKeyname = NULL;
    if( SectionNode == NULL || KeyName == NULL ) {
        pr_err(LOG_TAG "%s Inputpara not valid \n",__FUNCTION__);
        return Retval;
    }

    StrKeyname = get_key_value(SectionNode, KeyName);
    if(StrKeyname == NULL) {
        return Retval;
    }

    kstrtos32(StrKeyname, 10, &Retval);
    return Retval;
}

// -1:GetFail 0:normaltype 1: Hmirror only 2:Vmirroronly 3:HV-mirror
static int _fb_GetMirrorInfo(void)
{
    int RetMirrorMode = -1;
    MI_RESULT retErr = MI_ERR_FAILED;
    /* initial value must be sysini path */
    char pINIPath[MI_KEYNAMELENGTH_MAX] = "/vendor/tvconfig/config/sys.ini";
    char pszKeyName[MI_KEYNAMELENGTH_MAX] = {0};
    MI_SYS_ConfigData_t stConfigData;
    MI_HANDLE Sysinihandle = MI_HANDLE_NULL;
    MI_HANDLE Modelinihandle = MI_HANDLE_NULL;

    /* get modelini path in /vendor/tvconfig/config/sys.ini */
    retErr = MI_SYS_OpenConfigFile((MI_U8*)pINIPath, &Sysinihandle);
    if(retErr != MI_OK) {
        return RetMirrorMode;
    }
    memset(pszKeyName,0x00,sizeof(MI_U8)*MI_KEYNAMELENGTH_MAX);
    memset(&stConfigData, 0x00, sizeof(MI_SYS_ConfigData_t));
    strncpy(pszKeyName,MI_INI_KEYNAME_MODELNAME,strlen(MI_INI_KEYNAME_MODELNAME));
    stConfigData.eDataType = E_MI_SYS_CONFIG_DATA_TYPE_DATA;
    retErr = MI_SYS_GetConfigData(Sysinihandle, (MI_U8*)pszKeyName, &stConfigData);
    if(retErr != MI_OK) {
        MI_SYS_CloseConfigFile(Sysinihandle);
        return RetMirrorMode;
    }

    /*open Model ini file path hanle*/
    memset(pINIPath,0x00,sizeof(MI_U8)*MI_KEYNAMELENGTH_MAX);
    strncpy(pINIPath,stConfigData.stData.pBuf,stConfigData.stData.u32Len);
    retErr = MI_SYS_OpenConfigFile((MI_U8*)pINIPath, &Modelinihandle);
    if(retErr != MI_OK) {
        MI_SYS_CloseConfigFile(Sysinihandle);
        return RetMirrorMode;
    }

    memset(pszKeyName,0x00,sizeof(MI_U8)*MI_KEYNAMELENGTH_MAX);
    memset(&stConfigData, 0x00, sizeof(MI_SYS_ConfigData_t));
    strncpy(pszKeyName, pMIRRORINI_KEYNAME[0],strlen(pMIRRORINI_KEYNAME[0]));
    stConfigData.eDataType = E_MI_SYS_CONFIG_DATA_TYPE_DATA;
    retErr = MI_SYS_GetConfigData(Modelinihandle, (MI_U8*)pszKeyName, &stConfigData);
    if(retErr != MI_OK) {
        MI_SYS_CloseConfigFile(Sysinihandle);
        MI_SYS_CloseConfigFile(Modelinihandle);
        return RetMirrorMode;
    }

    memset(MIRRORPARAM,0x00,sizeof(MIRRORPARAM));
    // ignore case of both arguments
    if(!strcasecmp(stConfigData.stData.pBuf,"True"))
        MIRRORPARAM[0] = 1;
    else
        RetMirrorMode = 0;

    if(MIRRORPARAM[0])	{
        memset(pszKeyName,0x00,sizeof(MI_U8)*MI_KEYNAMELENGTH_MAX);
        memset(&stConfigData, 0x00, sizeof(MI_SYS_ConfigData_t));
        strncpy(pszKeyName, pMIRRORINI_KEYNAME[1],strlen(pMIRRORINI_KEYNAME[1]));
        stConfigData.eDataType = E_MI_SYS_CONFIG_DATA_TYPE_U32;
        retErr = MI_SYS_GetConfigData(Modelinihandle, (MI_U8*)pszKeyName, &stConfigData);
        if(retErr != MI_OK) {
            MI_SYS_CloseConfigFile(Sysinihandle);
            MI_SYS_CloseConfigFile(Modelinihandle);
            return RetMirrorMode;
        }
        MIRRORPARAM[1] = stConfigData.u32Data;
        RetMirrorMode = MIRRORPARAM[1];
     }

    MI_SYS_CloseConfigFile(Sysinihandle);
    MI_SYS_CloseConfigFile(Modelinihandle);

    return RetMirrorMode;
}

static void _fb_iommu_allocate(struct fb_info *pinfo) {
    MI_FBDEV_Controller_t* par = pinfo->par;
    MI_RESULT ret = MI_OK;

    MI_OS_IommuAllocate_t stIommuAllocate;
    memset(&stIommuAllocate,0x00,sizeof(stIommuAllocate));
    /*kernel mma requests stIommuAllocate.au8TagName less than 15 chars*/
    strcpy(stIommuAllocate.au8TagName,"fbdev");
    stIommuAllocate.bSecure = 0;
    stIommuAllocate.eMmaMappingType = E_MI_OS_MAPPING_TYPE_NONCACHE;
    stIommuAllocate.u32Size = par->length;

    ret = MI_OS_AllocateIommuMemory(&stIommuAllocate);
    if (ret == MI_OK) {
        pr_info(LOG_TAG "fb%d MI_OS_AllocateIommuMemory  phyaddress:0x%llx size:%x success\n",pinfo->node,stIommuAllocate.phyAddr,stIommuAllocate.u32Size);

        par->phyAddr = stIommuAllocate.phyAddr;
        par->IOMMUphyaddr = stIommuAllocate.phyAddr - IOMMU_PHYADDROFFSET;
        par->eMiuSel = E_MI_SYS_MIU_TYPE_INVALID;
        pinfo->fix.smem_start = par->IOMMUphyaddr;
        pinfo->fix.smem_len = par->length;
    } else {
        pr_err(LOG_TAG "MI_OS_AllocateIommuMemory failed ret is %d\n",ret);
    }
}


static void _fb_iommu_free(struct fb_info *pinfo) {
    MI_FBDEV_Controller_t* par = pinfo->par;
    MI_OS_IommuFree_t stIommuFree_t;
    MI_RESULT ret = MI_OK;

    if(par->u8UseIOMMU && par->phyAddr) {
        stIommuFree_t.phyAddr = par->phyAddr;
        stIommuFree_t.u32Size = par->length;
        ret = MI_OS_FreeIommuMemory(&stIommuFree_t);
        pr_info(LOG_TAG "%s MI_OS_FreeIommuMemory phAddr:0x%llx u32Size:%x ret=%d\n",__FUNCTION__,stIommuFree_t.phyAddr,stIommuFree_t.u32Size,ret);
        par->IOMMUphyaddr = NULL;
        par->phyAddr = NULL;

    }
}

static void _fb_buf_init(struct fb_info *pinfo, unsigned long pa)
{
    MI_FBDEV_Controller_t* par = pinfo->par;
    MI_RESULT ret = MI_ERR_FAILED;
    MI_VIRT KernelVA = NULL;
    pr_info(LOG_TAG "%s fb%d MIU=%d,PA=0x%lx,length=0x%x UseIOMMU = %d \n",\
              __FUNCTION__,pinfo->node,par->eMiuSel,pa,pinfo->fix.smem_len,par->u8UseIOMMU);

    if(!par->u8UseIOMMU) {
        if (pa) {
            if (par->eMiuSel == E_MI_SYS_MIU_TYPE_0) {
                pinfo->screen_base = (char __iomem *)ioremap(pa + MIU0_BUS_OFFSET, pinfo->fix.smem_len);
            } else if (par->eMiuSel == E_MI_SYS_MIU_TYPE_1) {
                pinfo->screen_base = (char __iomem *)ioremap(pa - MIU1_INTERVAL +  MIU1_BUS_OFFSET, pinfo->fix.smem_len);
            }
            pr_info(LOG_TAG "%s fb%d pinfo->screen_base=%p\n",__FUNCTION__,pinfo->node,pinfo->screen_base);
        } else {
            pr_err(LOG_TAG "%s fb%d The framebuffer physical address is null,the meminfo did not set!\n",__FUNCTION__,pinfo->node);
        }
    } else {
        if(par->phyAddr) {
            ret = MI_OS_Pa2NonCachedVa(par->phyAddr,&KernelVA);
            if(ret == MI_OK) {
                pinfo->screen_base = (char __iomem *)(KernelVA);
                pr_info(LOG_TAG "fb%d MI_OS_Pa2NonCachedVa success IOMMUPA = 0x%llx VAaddress is %p\n",pinfo->node,par->phyAddr,pinfo->screen_base);
            } else {
                pr_err(LOG_TAG "fb%d MI_OS_Pa2NonCachedVa fail IOMMUPA = %llx ret = %d \n",pinfo->node,par->phyAddr,ret); 
            }
        } else {
            pr_err(LOG_TAG "%s fb%d UseIOMMU = %d The par->phyAddr address is null!\n",__FUNCTION__,pinfo->node,par->u8UseIOMMU);
        }
    }

#if 0
    if(par->phyAddr && par->u8UseIOMMU) {
        pr_err(LOG_TAG "memset iommu memory as 0xFF\n");
        void * tempvaddr = (void*)KernelVA;
        memset(tempvaddr,0xFF,par->length);
        memset(tempvaddr,0xAA,par->length/2);
    }
#endif
}


static MI_BOOL _fb_miosd_reinit(struct fb_info *pinfo) {
    MI_FBDEV_Controller_t* par = pinfo->par;
    int i = 0;
    MI_U32 fbWidth = par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerWidth;
    MI_U32 fbheight = par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerHeight;
    MI_OSD_ColorFormat_e fbColorFormat = par->stFbdevWindowCtrl.stWindowInfo.eColorFormat;
    MI_HANDLE hWindow = MI_HANDLE_NULL;
    MI_HANDLE hRetSurface = MI_HANDLE_NULL;
    MI_BOOL result = TRUE;
    MI_U16 bpx = 0;
    MI_U16 bufferCnt = 0;
    MI_OSD_RgbColor_t stRgbColor;

    pr_info(LOG_TAG "%s fb%d osdwidth:%d osdheight:%d colorformat:%d\n",\
                    __FUNCTION__,pinfo->node,fbWidth,fbheight,fbColorFormat);

    if( par->stFbdevLayerCtrl.hLayerHandle == NULL) {
        pr_info(LOG_TAG "%s fb%d MI Layer has not been created\n",__FUNCTION__,pinfo->node);
        return FALSE;
    }

    for (i = 0; i < par->surfaceCnt; i++)  {
        if (par->stFbdevSurfaceCtrl[i].hSurfaceHandle != MI_HANDLE_NULL)  {
            pr_err(LOG_TAG "%s fb%d Surface:%x must be destroy in freeiommu\n",\
                        __FUNCTION__,pinfo->node,par->stFbdevSurfaceCtrl[i].hSurfaceHandle);
            return FALSE;
        }
    }

     if (par->stFbdevWindowCtrl.hWindowHandle != MI_HANDLE_NULL)  {
        pr_err(LOG_TAG "%s fb%d window:%x must be destroy in freeiommu\n",\
            __FUNCTION__,pinfo->node,par->stFbdevWindowCtrl.hWindowHandle);
        return FALSE;
     }

    _fb_gwin_enable(par->stFbdevLayerCtrl.hLayerHandle, FALSE);
    par->stFbdevLayerCtrl.bShown = false;
    result = mstar_FB_WindowCreate(&par->stFbdevWindowCtrl.stWindowInfo, &hWindow);
    if (!result) {
        pr_err(LOG_TAG "%s fb%d  mstar_FB_WindowCreate failed !! \n",__FUNCTION__,pinfo->node);
        return FALSE;
    }
    par->stFbdevWindowCtrl.hWindowHandle = hWindow;

    bpx = mstar_FB_GetBpp(fbColorFormat);
    if (fbWidth && fbheight && bpx && pinfo->fix.smem_len) {
        bufferCnt = getBufferCount( pinfo->fix.smem_len,fbWidth,fbheight, bpx);
        pr_info(LOG_TAG "%s fb%d getBufferCount = %d \n",__FUNCTION__,pinfo->node,bufferCnt);
    } else {
        bufferCnt = 0;
        pr_err(LOG_TAG "%s fb%d getBufferCount = 0 \n",__FUNCTION__,pinfo->node);
    }

    par->surfaceCnt = bufferCnt;
    for (i = 0; i < par->surfaceCnt; i++) {
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.bReArrange = FALSE;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Width = fbWidth;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Height = fbheight;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Pitch = fbWidth*bpx;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.eColorFormat = fbColorFormat;
        par->stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr = par->phyAddr + i * ALIGN_UP(fbWidth*bpx, 64) * fbheight;
        result = mstar_FB_SurfaceCreate(&(par->stFbdevSurfaceCtrl[i].stSurfaceInfo),&hRetSurface);
        if (!result) {
            pr_err(LOG_TAG "%s fb%d  mstar_FB_SurfaceCreate failed !! \n",__FUNCTION__,pinfo->node);
            return FALSE;
        }
        par->stFbdevSurfaceCtrl[i].hSurfaceHandle = hRetSurface;

#if (SURFACEPATTERN == 1)
        stRgbColor.u8Alpha = 0xFF;
        stRgbColor.u8Red = 0x00;
        stRgbColor.u8Green = 0x00;
        stRgbColor.u8Blue = 0xFF;
        result = mstar_FB_SurfaceClearColor(hRetSurface,&stRgbColor);
#else
        result = mstar_FB_SurfaceClear(hRetSurface);
#endif
        if (!result) {
            pr_err(LOG_TAG "%s fb%d  mstar_FB_SurfaceClear failed !! \n",__FUNCTION__,pinfo->node);
            return FALSE;
        }
    }

    (pinfo->var).yres_virtual = bufferCnt*fbheight;
    return TRUE;
}


static MI_BOOL _fb_miosd_init(struct fb_info *pinfo)
{
    MI_FBDEV_Controller_t* par = pinfo->par;
    int MirrorMode = -1;
    //default timing info set as 1080P
    int curTimingWidth = 1920;
    int curTimingHeight = 1080;
    MI_BOOL result = MI_OK;
    MI_HANDLE hRet = MI_HANDLE_NULL;
    int i = 0;
    MI_OSD_RgbColor_t stRgbColor;
    mstar_FB_getCurTiming(&curTimingWidth, &curTimingHeight);
    //Stretch win dst
    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth
        = curTimingWidth;
    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight
        = curTimingHeight;
    pr_info(LOG_TAG "_fb_miosd_init curTimingWidth=%d,curTimingHeight=%d\n",
        par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth,
        par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight);
    do
    {
        result = mstar_FB_LayerCreate(&(par->stFbdevLayerCtrl.stLayerInfo),
            &hRet);
        if (!result)
        {
            pr_err(LOG_TAG "%s:%d MI layer create failed! \n",__FUNCTION__,__LINE__);
            hRet = MI_HANDLE_NULL;
            break;
        }
        par->stFbdevLayerCtrl.hLayerHandle = hRet;
        par->stFbdevWindowCtrl.stWindowInfo.hLayer = hRet;
        result = mstar_FB_WindowCreate(&(par->stFbdevWindowCtrl.stWindowInfo),
            &hRet);
        if (!result)
        {
            pr_err(LOG_TAG "%s:%d MI window create failed! \n",__FUNCTION__,__LINE__);
            hRet = MI_HANDLE_NULL;
            break;
        }
        par->stFbdevWindowCtrl.hWindowHandle = hRet;
        for (i=0; i < par->surfaceCnt; i++)
        {
            result = mstar_FB_SurfaceCreate(&((par->stFbdevSurfaceCtrl[i]).stSurfaceInfo),
                &hRet);
            if (!result)
            {
                pr_err(LOG_TAG "%s:%d MI surface create failed! \n",__FUNCTION__,__LINE__);
                hRet = MI_HANDLE_NULL;
                break;
            }
            par->stFbdevSurfaceCtrl[i].hSurfaceHandle = hRet;
#if (SURFACEPATTERN == 1)
            stRgbColor.u8Alpha = 0xFF;
            stRgbColor.u8Red = 0xFF;
            stRgbColor.u8Green = 0x00;
            stRgbColor.u8Blue = 0x00;
            result = mstar_FB_SurfaceClearColor(hRet,&stRgbColor);
#else
            result = mstar_FB_SurfaceClear(hRet);
#endif
            if (!result) {
                pr_err(LOG_TAG "%s:%d MI surfaceclear failed! \n",__FUNCTION__,__LINE__);
                break;
            }
        }
    }while(0);
    if (!result)
    {
        for (i=0; i < par->surfaceCnt; i++)
        {
            if (par->stFbdevSurfaceCtrl[i].hSurfaceHandle != MI_HANDLE_NULL)
            {
                mstar_FB_SurfaceDestroy(par->stFbdevSurfaceCtrl[i].hSurfaceHandle);
                par->stFbdevSurfaceCtrl[i].hSurfaceHandle = MI_HANDLE_NULL;
             }
        }
        if (par->stFbdevWindowCtrl.hWindowHandle != MI_HANDLE_NULL)
        {
            mstar_FB_WindowDestroy(par->stFbdevWindowCtrl.hWindowHandle);
            par->stFbdevWindowCtrl.hWindowHandle = MI_HANDLE_NULL;
        }
        if (par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
        {
            mstar_FB_LayerDestroy(par->stFbdevLayerCtrl.hLayerHandle);
            par->stFbdevLayerCtrl.hLayerHandle = NULL;
        }
    }
    else
    {
        mstar_FB_SetVsyncMode(par->stFbdevLayerCtrl.hLayerHandle, FALSE);
        mstar_FB_EnableTransClr_EX(par->stFbdevLayerCtrl.hLayerHandle, FALSE);
        mstar_FB_SetBlending(par->stFbdevWindowCtrl.hWindowHandle, TRUE, 0xFF);
        mstar_FB_EnableMultiAlpha(par->stFbdevLayerCtrl.hLayerHandle, FALSE);
    }

    MirrorMode = _fb_GetMirrorInfo();
    if( MirrorMode == 0 ) {
        pr_info(LOG_TAG "Layer:%x MirrorMode is none",par->stFbdevLayerCtrl.hLayerHandle);
    }else if( MirrorMode == 1 ) {
        pr_info(LOG_TAG "Layer:%x MirrorMode is Hmirror",par->stFbdevLayerCtrl.hLayerHandle);
        mstar_FB_SetHMirror(par->stFbdevLayerCtrl.hLayerHandle,TRUE);
    }else if( MirrorMode == 2 ) {
        pr_info(LOG_TAG "Layer:%x MirrorMode is Vmirror",par->stFbdevLayerCtrl.hLayerHandle);
        mstar_FB_SetVMirror(par->stFbdevLayerCtrl.hLayerHandle,TRUE);
    }else if( MirrorMode == 3 ) {
        pr_info(LOG_TAG "Layer:%x MirrorMode is HVmirror",par->stFbdevLayerCtrl.hLayerHandle);
        mstar_FB_SetHVMirror(par->stFbdevLayerCtrl.hLayerHandle,TRUE);
    }else  {
        pr_err(LOG_TAG "Layer:%x GetMirrorMode failed!!",par->stFbdevLayerCtrl.hLayerHandle);
    }

    return result;
}

static int _fb_miosd_deinit(struct fb_info *pinfo)
{
    MI_FBDEV_Controller_t* par = pinfo->par;
    int i = 0;
    //destroy temp surface
    if (par->pTempFbdevSurfaceCtrl)
    {
        if (par->pTempFbdevSurfaceCtrl->hSurfaceHandle != MI_HANDLE_NULL)
        {
            pr_info(LOG_TAG "%s destroy tempsurface \n",__FUNCTION__);
            CHECK_IFNOT(MI_OSD_SurfaceDestroy(par->pTempFbdevSurfaceCtrl->hSurfaceHandle), MI_OK, -1)
        }
        kfree(par->pTempFbdevSurfaceCtrl);
        par->pTempFbdevSurfaceCtrl = NULL;
    }
    //destroy layer
    if(par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
    {
        pr_info(LOG_TAG "%s destroy layer handle=0x%x\n",__FUNCTION__,par->stFbdevLayerCtrl.hLayerHandle);
        CHECK_IFNOT(MI_OSD_LayerDestroy(par->stFbdevLayerCtrl.hLayerHandle), MI_OK, -1);
    }
    par->stFbdevLayerCtrl.hLayerHandle = MI_HANDLE_NULL;
    //destroy layer will destroy correspond window
    par->stFbdevWindowCtrl.hWindowHandle=MI_HANDLE_NULL;
    //destroy surface
    for (i=0; i < par->surfaceCnt; i++)
    {
        if (par->stFbdevSurfaceCtrl[i].hSurfaceHandle != MI_HANDLE_NULL) {
            pr_info(LOG_TAG "%s destroy surface par->stFbdevSurfaceCtrl[%d].hSurfaceHandle=%x,phyAddr=0x%llx\n",__FUNCTION__,\
                i,par->stFbdevSurfaceCtrl[i].hSurfaceHandle,par->stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr);
            CHECK_IFNOT(MI_OSD_SurfaceDestroy(par->stFbdevSurfaceCtrl[i].hSurfaceHandle), MI_OK, -1);
            par->stFbdevSurfaceCtrl[i].hSurfaceHandle = MI_HANDLE_NULL;
        }
    }

    return 0;
}

static void _fb_gwin_enable(MI_HANDLE hLayerHandle, unsigned char bEnable)
{
    mstar_FB_EnableGwin(hLayerHandle, bEnable);
}

static int _fb_GetBootlogoUsed(void)
{
    struct file* filp = NULL;
    mm_segment_t fs;
    loff_t buffersize = 800;
    char* pbuffer = NULL;
    char *pstr = NULL;
    ssize_t ret = 0;
    int result = 0;

    filp = filp_open(bootargsfile, O_RDONLY, 0644);
    if (IS_ERR_OR_NULL(filp))
    {
        pr_err(LOG_TAG "filp_open bootargs node /proc/cmdline failed !\n");
        result = -1;
        return result;
    }
    do
    {
        fs = get_fs();
        set_fs(KERNEL_DS);

        pbuffer = (char*)kzalloc(buffersize, GFP_KERNEL);
        if (IS_ERR_OR_NULL(pbuffer))
        {
            pr_err(LOG_TAG "Can not allocate buffer from kernel !\n");
            result = -1;
            break;
        }

        filp->f_op->llseek(filp, 0, SEEK_SET);
        ret = vfs_read(filp, pbuffer, buffersize, &filp->f_pos);
        pstr = strstr(pbuffer,"BOOTLOGO_IN_MBOOT");

        if(pstr) // pstr != NULL find BOOTLOGO_IN_MBOOT
        {
           result = 1;
        }
        else //  pstr == NULL cannot find BOOTLOGO_IN_MBOOT
        {
           result = 0;
        }
    }while(0);
    kfree(pbuffer);
    set_fs(fs);
    filp_close(filp, NULL);
#if (FORCEBOOTLOGOON == 1)
    return 1;
#else
    return result;
#endif
}


static int Parse_Sysconfig(char* path) {
    struct file* filp = NULL;
    mm_segment_t fs;
    /*pointer for store sys.ini content*/
    char* psysConfig = NULL;
    loff_t size = 0;
    ssize_t ret = 0;
    int result = 0;
    IniSectionNode* root = NULL;
    IniSectionNode* SysConfigSection = NULL;
    const char* StrKeyname = NULL;

    filp = filp_open(SYSINI_PATH, O_RDONLY, 0644);
    if (IS_ERR_OR_NULL(filp)) {
        pr_err(LOG_TAG "filp_open %s failed!\n",SYSINI_PATH);
        return -1;
    }
    do
    {
        fs = get_fs();
        set_fs(KERNEL_DS);
        size = filp->f_op->llseek(filp, 0, SEEK_END);
        psysConfig = (char*)kzalloc(size + 1, GFP_KERNEL);
        if (IS_ERR_OR_NULL(psysConfig)) {
            pr_err(LOG_TAG "kzalloc allocate from GFP_KERNEL fail\n");
            result = -1;
            goto FAIL;
        }
        filp->f_op->llseek(filp, 0, SEEK_SET);
        ret = vfs_read(filp, psysConfig, size, &filp->f_pos);
        if (ret != size) {
            pr_err(LOG_TAG "fs_read return ret=%zu, But real size=%lld\n",ret,size);
            kfree(psysConfig);
            result = -1;
            goto FAIL;
        }
    }while(0);

    alloc_and_init_ini_tree(&root, psysConfig);
    SysConfigSection = get_section(root,SysConfigSectinName);
    StrKeyname = get_key_value(SysConfigSection,OsdDefineKeyName);
    if(StrKeyname != NULL) {
        strcpy(path,StrKeyname);
    }

FAIL:
    set_fs(fs);
    if(filp != NULL) {
        filp_close(filp, NULL);
    }
    return result;
}


static int Read_FbdevConfile(char** data)
{
    struct file* filp = NULL;
    mm_segment_t fs;
    loff_t size = 0;
    ssize_t ret = 0;
    int result = 0;
    //array for get OsdDefine.ini path
    char OsdDefinePath[100] = {0};

    result = Parse_Sysconfig(OsdDefinePath);
    if(result < 0) {
      pr_err(LOG_TAG "parse OsdDefine.ini path from sys.ini failed\n");

      result = 0;
      filp = filp_open(LINUX_OSDDEFINE_PATH, O_RDONLY, 0644);
      if (IS_ERR_OR_NULL(filp)) {
          pr_err(LOG_TAG "open %s failed!\n",LINUX_OSDDEFINE_PATH);
          filp = filp_open(AN_OSDDEFINE_PATH, O_RDONLY, 0644);
          if (IS_ERR_OR_NULL(filp)) {
               pr_err(LOG_TAG "open %s failed!\n",AN_OSDDEFINE_PATH);
               return -1;
          }
     }
    }else {
      pr_info(LOG_TAG "Get INI file path %s\n",OsdDefinePath);
      filp = filp_open(OsdDefinePath, O_RDONLY, 0644);
      if (IS_ERR_OR_NULL(filp)) {
           pr_err(LOG_TAG "open %s failed!\n",OsdDefinePath);
           return -1;
      }
    }
    do
    {
        fs = get_fs();
        set_fs(KERNEL_DS);
        size = filp->f_op->llseek(filp, 0, SEEK_END);
        *data = (char*)kzalloc(size + 1, GFP_KERNEL);
        if (IS_ERR_OR_NULL(*data)) {
            pr_err(LOG_TAG "Can not allocate buffer from kernel!\n");
            result = -1;
            break;
        }
        filp->f_op->llseek(filp, 0, SEEK_SET);
        ret = vfs_read(filp, *data, size, &filp->f_pos);
        if (ret != size) {
            pr_err(LOG_TAG "fs_read return ret=%zu, But real size=%lld\n",ret,size);
            kfree(*data);
            result = -1;
            break;
        }
    }while(0);
    set_fs(fs);
    filp_close(filp, NULL);
    return result;
}


static void set_fb_bitfield(MI_OSD_ColorFormat_e eColorFmt , struct fb_var_screeninfo *var)
{
        switch (eColorFmt)
        {
            case E_MI_OSD_COLOR_FORMAT_RGB565:
            {
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 6;
                var->red.offset = 11;
                var->red.length = 5;
                var->transp.offset = 0;
                var->transp.length = 0;
            }
            break;
            case E_MI_OSD_COLOR_FORMAT_ARGB4444:
            {
                var->blue.offset = 0;
                var->blue.length = 4;
                var->green.offset = 4;
                var->green.length= 4;
                var->red.offset = 8;
                var->red.length = 4;
                var->transp.offset = 12;
                var->transp.length = 4;
            }
            break;
            case E_MI_OSD_COLOR_FORMAT_ARGB8888:
            {
                var->red.offset = 16;
                var->red.length = 8;
                var->green.offset = 8;
                var->green.length = 8;
                var->blue.offset = 0;
                var->blue.length = 8;
                var->transp.offset = 24;
                var->transp.length = 8;
            }
            break;
            case E_MI_OSD_COLOR_FORMAT_ARGB1555:
            {
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 5;
                var->red.offset = 10;
                var->red.length = 5;
                var->transp.offset = 15;
                var->transp.length = 1;
            }
            break;
            case E_MI_OSD_COLOR_FORMAT_YUV422_YVYU:
            default:
            {
                pr_info(LOG_TAG "%s set_fb_bitfield colorFmt=%d \n",__FUNCTION__,eColorFmt);
            }
        break;
    }
}

static void restoreFbInfo(struct fb_info* pinfo)
{
    MI_FBDEV_Controller_t* par = pinfo->par;
    int fbIdx = pinfo->node - first_fb_node;
    MI_U16 bpx = 4;
    pr_info(LOG_TAG "%s fb:%d \n",__FUNCTION__,pinfo->node);
    memcpy(par, &fbdevControllers[fbIdx], sizeof(MI_FBDEV_Controller_t));
    //restore fb_var_screeninfo
    pinfo->var.xres = par->stFbdevWindowCtrl.stWindowInfo.u32SurfaceWidth;
    pinfo->var.yres = par->stFbdevWindowCtrl.stWindowInfo.u32SurfaceHeight;
    pinfo->var.xres_virtual =  pinfo->var.xres;
    pinfo->var.yres_virtual = pinfo->var.yres * par->surfaceCnt;
    pinfo->var.xoffset = 0;
    pinfo->var.yoffset = 0;
    bpx = mstar_FB_GetBpp(par->stFbdevWindowCtrl.stWindowInfo.eColorFormat);
    pinfo->var.bits_per_pixel = bpx << 3;
    pinfo->var.activate = FB_ACTIVATE_NOW;
    set_fb_bitfield(par->stFbdevWindowCtrl.stWindowInfo.eColorFormat,
        &(pinfo->var));
    //set width, heigth as maxium
    pinfo->var.width = -1;
    pinfo->var.height = -1;
    pinfo->var.grayscale = 0;
    /*timing useless? use the vfb default */
    pinfo->var.pixclock
        = 100000000000LLU / (6 *  1920 * 1080);
    pinfo->var.left_margin = 64;
    pinfo->var.right_margin = 64;
    pinfo->var.upper_margin = 32;
    pinfo->var.lower_margin = 32;
    pinfo->var.hsync_len  =64;
    pinfo->var.vsync_len = 2;
    pinfo->var.vmode  = FB_VMODE_NONINTERLACED;
    //restore pitch,maybe change it via FBIO_OUTPUT
    pinfo->fix.line_length = ALIGN_UP(pinfo->var.xres_virtual * bpx, 64);

    if(par->u8UseIOMMU)
       pinfo->fix.smem_start = par->IOMMUphyaddr;
    else
       pinfo->fix.smem_start = par->phyAddr;
    pinfo->fix.smem_len = par->length;
}


static void parse_hwLayerInfo(IniSectionNode *root, IniSectionNode *hwLayerInfo, const char *name) {
    int FbLayerID,FbFmt,FbDFBReserved,FbWidth,FbHeight,FbmmapId,FbVirtual,FbUseIommu;
    MI_RESULT ret = MI_OK;
    MI_SYS_MmapLayout_t stMmapLayout;
    MI_PHY phyMiuBaseAddr = 0;
    char strId[16] = {0};
    u8 bufferCount = 2;
    int i = 0;
    MI_U16 bpx = 0;

    if (!strcmp(name, bootlogo_section_name)) {
        gBootlogoIdx = _fb_GetIniKeyValue(hwLayerInfo, "BOOTLOGO_GOPINDEX");
        pr_info(LOG_TAG "Bootlogo GOP index = %d\n",gBootlogoIdx);
        return;
    }

    if (strncmp(name, fbdev_setction_name,strlen(fbdev_setction_name))) {
        pr_err("%s parse section name %s skip!!\n",__FUNCTION__,name);
        return;
    }

    FbLayerID = _fb_GetIniKeyValue(hwLayerInfo, "FB_LAYER_ID");
    FbmmapId = _fb_GetIniKeyValue(hwLayerInfo, "FB_MMAP_ID");
    FbWidth = _fb_GetIniKeyValue(hwLayerInfo, "FB_WINDOW_WIDTH");
    FbHeight = _fb_GetIniKeyValue(hwLayerInfo, "FB_WINDOW_HEIGHT");
    FbFmt = _fb_GetIniKeyValue(hwLayerInfo, "FB_WINDOW_FORMAT");
    FbVirtual = _fb_GetIniKeyValue(hwLayerInfo, "FB_VIRTUAL");
    FbDFBReserved = _fb_GetIniKeyValue(hwLayerInfo, "FB_DFB_RESERVED");
    FbUseIommu = _fb_GetIniKeyValue(hwLayerInfo, "FB_IOMMU");

    pr_info(LOG_TAG "fb%d layerId:%d mmapID:%d fbwidth:%d fbheight:%d format:%d virtualFb:%d dfbreserve:%d FbUseIommu:%d\n",\
                  numFbHwlayer,FbLayerID,FbmmapId,FbWidth,FbHeight,FbFmt,FbVirtual,FbDFBReserved,FbUseIommu);

    if(FbLayerID == -1 || FbWidth == -1 || FbHeight == -1 || FbFmt == -1) {
        pr_err(LOG_TAG "Section FBDEV[%d]  KeyValue not set !!\n",numFbHwlayer);
        return;
    }

    if(FbUseIommu == -1 && FbmmapId == -1) {
        pr_err(LOG_TAG "Section [FBDEV%d]  MMAP ID or FB_IOMMU not set!!\n",numFbHwlayer);
        return;
    }

    /*represent for Bytes of colorformat*/
    bpx = mstar_FB_GetBpp((MI_OSD_ColorFormat_e)FbFmt);
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.eLayerId = (MI_OSD_Layer_e)FbLayerID;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32X = 0;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32Y = 0;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.eLayerSize = E_MI_OSD_LAYER_SIZE_CUSTOMIZE;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.hLayerHandle  = MI_HANDLE_NULL;
    fbdevControllers[numFbHwlayer].bReserveDFB = (FbDFBReserved > 0)?1:0;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerWidth = FbWidth;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerHeight = FbHeight;
    /*Default set Dstsize as FbWidth it will assign later*/
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth = FbWidth;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight = FbHeight;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.bShown = FALSE;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.bPremultiply = FALSE;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.bEnableMultiAlpha = FALSE;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.bEnableClrKey = FALSE;

    /*Add system default value contrast Y:16 U:16 V:16 brightness:0 */
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.contrastcolor.u16ContrastU = 16;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.contrastcolor.u16ContrastV = 16;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.contrastcolor.u16ContrastY = 16;
    fbdevControllers[numFbHwlayer].stFbdevLayerCtrl.brightness = 0;


    fbdevControllers[numFbHwlayer].u8FlipMode = E_MI_FB_FlipMode_Queue;
    fbdevControllers[numFbHwlayer].bEnable_GOP_AutoDetect = FALSE;
    fbdevControllers[numFbHwlayer].u8QueueBufferCnt = 0;
    /*Initialize value bUseExBlockMem */
    fbdevControllers[numFbHwlayer].bUseExBlockMem = FALSE;
    fbdevControllers[numFbHwlayer].u8IsVirtualFB = (FbVirtual > 0)?1:0;
    fbdevControllers[numFbHwlayer].u8UseIOMMU = (FbUseIommu > 0)?1:0;

    /*Initialize fbdevControllers WindowInfo */
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.hWindowHandle = MI_HANDLE_NULL;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.hLayer = MI_HANDLE_NULL;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.bIsFullFlip = FALSE;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.eBufType = E_MI_OSD_WINDOW_BUFFER_EXTERNAL;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.bPixelAlpha = TRUE;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.stRect.u32X = 0;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.stRect.u32Y = 0;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.stRect.u32Width = FbWidth;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.stRect.u32Height = FbHeight;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.u32SurfaceWidth = FbWidth;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.u32SurfaceHeight = FbHeight;
    fbdevControllers[numFbHwlayer].stFbdevWindowCtrl.stWindowInfo.eColorFormat = (MI_OSD_ColorFormat_e)FbFmt;

    if( FbUseIommu <= 0 ) {
        memset(&stMmapLayout, 0, sizeof(MI_SYS_MmapLayout_t));
        ret = MI_SYS_GetMmapLayout((MI_SYS_MmapId_e)FbmmapId, &stMmapLayout);
        if (ret == MI_OK) {
            phyMiuBaseAddr = stMmapLayout.phyMiuBaseAddr;
            fbdevControllers[numFbHwlayer].phyAddr = stMmapLayout.phyMemAddr + phyMiuBaseAddr;
            fbdevControllers[numFbHwlayer].length = stMmapLayout.u32MemLen;
            fbdevControllers[numFbHwlayer].eMiuSel = stMmapLayout.eMiuType;
        }

        if (FbWidth && FbHeight && bpx && fbdevControllers[numFbHwlayer].length) {
            bufferCount= getBufferCount(fbdevControllers[numFbHwlayer].length,FbWidth,FbHeight,bpx);
            fbdevControllers[numFbHwlayer].surfaceCnt = bufferCount;
        } else {
             bufferCount = 0;
             pr_info(LOG_TAG "Init surface buffercount fail reset to zero!!\n");
             fbdevControllers[numFbHwlayer].surfaceCnt = 0;
        }
    } else {
        pr_info(LOG_TAG "fb%d will allocate iommu memory\n",numFbHwlayer);
        bufferCount = 0;
        fbdevControllers[numFbHwlayer].phyAddr = 0x00;
        fbdevControllers[numFbHwlayer].IOMMUphyaddr = 0x00;
        fbdevControllers[numFbHwlayer].length = 0;
        fbdevControllers[numFbHwlayer].surfaceCnt = 0;
        fbdevControllers[numFbHwlayer].eMiuSel = E_MI_SYS_MIU_TYPE_INVALID;
    }

    /* Init stFbdevSurfaceCtrl Info  */
    /* Buffercount will be zero in iommu case*/
    for ( i = 0; i < bufferCount; i++)
    {
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].hSurfaceHandle = MI_HANDLE_NULL;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.eOwner = E_MI_OSD_SURFACE_OWNER_AP;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.eMemoryType = E_MI_OSD_MEMORY_PHY_OS;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.bReArrange = FALSE;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Width = FbWidth;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Height = FbHeight;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Pitch = ALIGN_UP(FbWidth * bpx, 64);
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.eColorFormat = (MI_OSD_ColorFormat_e)FbFmt;
        fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr = \
                fbdevControllers[numFbHwlayer].phyAddr + i* ALIGN_UP(FbWidth * bpx, 64) * FbHeight;
        pr_info(LOG_TAG "Init Surface %d  physical addr=0x%llx\n",i,fbdevControllers[numFbHwlayer].stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr);
     }

    /*Init Fbdev fixinfo & varinfo*/
    snprintf(strId, sizeof(strId), "MStar FB%d",numFbHwlayer);
    memcpy(mstar_fb_fix_infos[numFbHwlayer].id, strId, strlen(strId));
    mstar_fb_fix_infos[numFbHwlayer].type = FB_TYPE_PACKED_PIXELS;
    mstar_fb_fix_infos[numFbHwlayer].visual = FB_VISUAL_TRUECOLOR;
    mstar_fb_fix_infos[numFbHwlayer].xpanstep = 1;
    mstar_fb_fix_infos[numFbHwlayer].ypanstep = 1;
    mstar_fb_fix_infos[numFbHwlayer].ywrapstep = 1;
    mstar_fb_fix_infos[numFbHwlayer].line_length = ALIGN_UP(FbWidth * bpx, 64);
    /**
    *mmio_start and mmio_len was used in fb_mmap
    If fb_mmap has been implement by vendor,it'll
    invoke vendor implement.the mstar_fb_mmap is
    the implementation of MStar, it's not necessary to
    init mmio_start and mmio_len. So hardcode it.
    */
    mstar_fb_fix_infos[numFbHwlayer].mmio_start = 0x08000000;
    mstar_fb_fix_infos[numFbHwlayer].mmio_len = 0x10000000;
    mstar_fb_fix_infos[numFbHwlayer].accel = FB_ACCEL_NONE;
    mstar_fb_fix_infos[numFbHwlayer].smem_start = fbdevControllers[numFbHwlayer].phyAddr;
    mstar_fb_fix_infos[numFbHwlayer].smem_len = fbdevControllers[numFbHwlayer].length;
    //init mstar_fb_var_infos
    mstar_fb_var_infos[numFbHwlayer].xres = FbWidth;
    mstar_fb_var_infos[numFbHwlayer].yres = FbHeight;
    mstar_fb_var_infos[numFbHwlayer].xres_virtual = FbWidth;
    mstar_fb_var_infos[numFbHwlayer].yres_virtual = FbHeight * bufferCount;
    mstar_fb_var_infos[numFbHwlayer].bits_per_pixel = bpx << 3;
    mstar_fb_var_infos[numFbHwlayer].activate = FB_ACTIVATE_NOW;
    set_fb_bitfield((MI_FB_ColorFmt_e)FbFmt, &mstar_fb_var_infos[numFbHwlayer]);
    //set width, heigth as maxium
    mstar_fb_var_infos[numFbHwlayer].width = -1;
    mstar_fb_var_infos[numFbHwlayer].height = -1;
    mstar_fb_var_infos[numFbHwlayer].grayscale = 0;
    /*timing useless? use the vfb default */
    mstar_fb_var_infos[numFbHwlayer].pixclock = 100000000000LLU / (6 *  1920 * 1080);
    mstar_fb_var_infos[numFbHwlayer].left_margin = 64;
    mstar_fb_var_infos[numFbHwlayer].right_margin = 64;
    mstar_fb_var_infos[numFbHwlayer].upper_margin = 32;
    mstar_fb_var_infos[numFbHwlayer].lower_margin = 32;
    mstar_fb_var_infos[numFbHwlayer].hsync_len  =64;
    mstar_fb_var_infos[numFbHwlayer].vsync_len = 2;
    mstar_fb_var_infos[numFbHwlayer].vmode  = FB_VMODE_NONINTERLACED;
    numFbHwlayer++;
}


static void calFbdevCounts (IniSectionNode *root, IniSectionNode *node,const char *name)
{
    if (!strncmp(name,fbdev_setction_name,strlen(fbdev_setction_name))) {
        numFbHwlayer++;
    }
}
//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static MI_RESULT _FBDEV_BOOTLOGO_CallBack(MI_HANDLE hOsd, MI_U32 u32Event, void *pEventParams, void *pUserParams)
{
    MI_RESULT Result = MI_OK;
    struct fb_info* pinfo = (struct fb_info*)pUserParams;
    MI_FBDEV_Controller_t* par = pinfo->par;
    bBootlogoDisabled = TRUE;
    pr_info(LOG_TAG "fb[%d] enter into %s \n",pinfo->node,__FUNCTION__);
    if( par->stFbdevLayerCtrl.stLayerInfo.eLayerId == (MI_OSD_Layer_e)gBootlogoIdx )
    {
        pr_err(LOG_TAG "bBootlogoOpened is %d",bBootlogoOpened);
        if(bBootlogoOpened)
        {
            /*In iommu case  will init using ioctl FBIO_MIOSD_INIT*/
            if(!par->u8UseIOMMU) {
                if (_fb_miosd_init(pinfo)) {
                    par->bInitialized = TRUE;
                    pr_info(LOG_TAG "%s _fb_miosd_init success! \n", __FUNCTION__);
                } else {
                    pr_err(LOG_TAG "%s functioncallback _fb_miosd_init failed!\n",__FUNCTION__);
                    Result = MI_ERR_FAILED;
                }
            } else {
               par->bInitialized = TRUE;
               pr_info(LOG_TAG "fb[%d] using IOMMU mark Initialized default\n", pinfo->node);
            }
        }
    }
    return Result;
}

static int mstar_fb_probe(struct platform_device *dev)
{
    struct fb_info *pinfo = NULL;
    struct fb_info** pFbInfos = NULL;
    struct fb_info *pCurrentFbInfo = NULL;
    MI_OSD_CallbackInputParams_t stCallbackInputParams;
    int retval = 0;
    IniSectionNode* root = NULL;
    char* contents = NULL;
    int i = 0;
    int j = 0;
    int ret = 0;
    if (NULL == dev)
    {
        pr_err(LOG_TAG "ERROR: in mstar_fb_probe: dev is NULL pointer \r\n");
        return -ENOTTY;
    }
    ret = Read_FbdevConfile(&contents);
    if (ret < 0) {
        pr_err(LOG_TAG "ERROR: in mstar_fb_probe: read OsdDefine.ini failed\n");
        retval= -1;
        goto out;
    }

    if (alloc_and_init_ini_tree(&root, contents)) {
        retval= -1;
        goto out;
    }
    dump_ini(root);
    foreach_section(root, calFbdevCounts);
    if (numFbHwlayer == 0)
    {
        pr_err(LOG_TAG "ERROR: in mstar_fb_probe: can not find FBDEV section in OsdConfig.ini failed\n");
        retval = -1;
        goto out;
    }

    //Alocate memory for fbdevControllers and mstar_fb_fix_infos mstar_fb_var_infos
    fbdevControllers = (MI_FBDEV_Controller_t*)kzalloc(
        numFbHwlayer * sizeof(MI_FBDEV_Controller_t), GFP_KERNEL);
    if (IS_ERR_OR_NULL(fbdevControllers))
    {
        retval = -1;
        printk(KERN_ERR "allocate memory for MI_FBDEV_Controller_t failed!\n");
        goto out;
    }

    mstar_fb_fix_infos = (struct fb_fix_screeninfo*)kzalloc(
            numFbHwlayer * sizeof(struct fb_fix_screeninfo), GFP_KERNEL);
    if (IS_ERR_OR_NULL(mstar_fb_fix_infos))
    {
        printk(KERN_ERR "allocate memory for mstar_fb_fix_infos failed!\n");
        retval = -1;
        goto out;
    }

    mstar_fb_var_infos = (struct fb_var_screeninfo*)kzalloc(
            numFbHwlayer * sizeof(struct fb_var_screeninfo), GFP_KERNEL);
    if (IS_ERR_OR_NULL(mstar_fb_fix_infos))
    {
        printk(KERN_ERR "allocate memory for mstar_fb_var_infos failed!\n");
        retval = -1;
        goto out;
    }
    pFbInfos = (struct fb_info**)kzalloc(
        numFbHwlayer* sizeof(struct fb_info*), GFP_KERNEL);
    if (IS_ERR_OR_NULL(pFbInfos))
    {
        printk(KERN_ERR "allocate memory for pFbInfos failed!\n");
        retval = -1;
        goto out;
    }
    //parse FBDEV section
    numFbHwlayer = 0;
    foreach_section(root, parse_hwLayerInfo);

    //register framebufferInfo
    for (i=0; i < numFbHwlayer; i++)
    {
        pinfo = framebuffer_alloc(sizeof(MI_FBDEV_Controller_t), &dev->dev);
        if (!pinfo) {
            printk(KERN_ERR "framebuffer_alloc:%d failed!\n", i);
            retval = -1;
            break;
         }

         if( fbdevControllers[i].u8IsVirtualFB > 0 )  {
            pinfo->fbops = &virtaulfb_ops;
         } else {
            pinfo->fbops = &mstar_fb_ops;
         }

         pinfo->var = mstar_fb_var_infos[i];
         pinfo->fix = mstar_fb_fix_infos[i];
         pinfo->pseudo_palette = fbdevControllers[i].pseudo_palette;
         memcpy(pinfo->par, &(fbdevControllers[i]), sizeof(MI_FBDEV_Controller_t));
         pinfo->flags = FBINFO_FLAG_DEFAULT;
         pFbInfos[i] = pinfo;

        //register callback to bootlogo
         if (fbdevControllers[i].stFbdevLayerCtrl.stLayerInfo.eLayerId==(MI_OSD_Layer_e)gBootlogoIdx)
         {
            bBootLogostatus = _fb_GetBootlogoUsed();
            //Use bootlogo gop index  & Bootlogo turn on
            if( bBootLogostatus > 0 )
            {
                stBootlogoInfo=pinfo;
                memset(&stCallbackInputParams, 0x00, sizeof(MI_OSD_CallbackInputParams_t));
                stCallbackInputParams.pfEventCallback = (MI_OSD_EventCallback)_FBDEV_BOOTLOGO_CallBack;
                stCallbackInputParams.u32EventFlags = E_MI_OSD_CALLBACK_EVENT_DISABLE_BOOTLOGO;
                stCallbackInputParams.pUserParams  = (void*)stBootlogoInfo;

                if (!mstar_FB_RegisterCallback(&stCallbackInputParams))
                {
                    pr_err(LOG_TAG "%s:%d mstar_FB_RegisterCallback failed!\n",__FUNCTION__,__LINE__);
                    return -EINVAL;
                }
                pr_info(LOG_TAG "%s fb%d allocated gop was used by bootlogo & Register callback success! \n",\
                         __FUNCTION__,stBootlogoInfo->node);
            } //endof f( bBootLogoTurnON > 0 )
         }
    }
    //If framebuffer_alloc fail, release fb_info already allocated
    if (i < numFbHwlayer)
    {
        for (j = 0; j < i; j++)
        {
            pCurrentFbInfo = pFbInfos[j];
            framebuffer_release(pCurrentFbInfo);
        }
        pCurrentFbInfo = NULL;
        goto out;
    }
    //fb_alloc_cmap for all fb_info
    for (i = 0; i < numFbHwlayer; i++)
    {
        pCurrentFbInfo = pFbInfos[i];
        retval  = fb_alloc_cmap(&pCurrentFbInfo->cmap, 256, 1);
        if (retval < 0)
        {
            printk(KERN_ERR "fb_alloc_cmap:%d failed!\n", i);
            break;
        }
    }
    if (i < numFbHwlayer)
    {
        //release cmap and fb_info already been alocated
        for (j = 0; j < i; j++)
        {
            pCurrentFbInfo = pFbInfos[j];
            fb_dealloc_cmap(&pCurrentFbInfo->cmap);
        }
        for (j=0; j < numFbHwlayer; j++)
        {
            pCurrentFbInfo = pFbInfos[j];
            framebuffer_release(pCurrentFbInfo);
        }
        pCurrentFbInfo = NULL;
        goto out;
    }
    //register framebuffer
    for (i = 0; i < numFbHwlayer; i++)
    {
        pCurrentFbInfo = pFbInfos[i];
        retval = register_framebuffer(pCurrentFbInfo);
        if (retval < 0)
        {
            printk(KERN_ERR "register_framebuffer:%d failed!\n", i);
            break;
        }
    }
    if ( i < numFbHwlayer)
    {
        //unregister framebuffer already registerd
         for (j = 0; j < i; j++)
         {
            pCurrentFbInfo = pFbInfos[j];
            unregister_framebuffer(pCurrentFbInfo);
         }
         //release cmap and fb_info
         for (j = 0; j < numFbHwlayer; j++)
         {
            pCurrentFbInfo = pFbInfos[j];
            fb_dealloc_cmap(&pCurrentFbInfo->cmap);
            framebuffer_release(pCurrentFbInfo);
         }
         pCurrentFbInfo = NULL;
         goto out;
    }
    //save pFirstFbInfo as private data
    platform_set_drvdata(dev, pFbInfos);
    first_fb_node = pFbInfos[0]->node;

    pr_info(LOG_TAG "%s fb%d: Mstar frame buffer device\n",__FUNCTION__,pFbInfos[0]->node);
out:
    if (retval < 0)
    {
        if (fbdevControllers)
        {
            kfree(fbdevControllers);
            fbdevControllers = NULL;
        }
        if (pFbInfos)
        {
            kfree(pFbInfos);
            pFbInfos = NULL;
        }
    }
    if (mstar_fb_fix_infos)
    {
        kfree(mstar_fb_fix_infos);
        mstar_fb_fix_infos = NULL;
    }
    if (mstar_fb_var_infos)
    {
        kfree(mstar_fb_var_infos);
        mstar_fb_var_infos = NULL;
    }
    release_ini_tree(root);
    kfree(contents);
    return retval;
}

static int mstar_fb_remove(struct platform_device *dev)
{
    struct fb_info **pinfo = NULL;
    int i = 0;
    if (NULL == dev)
    {
        printk("ERROR: mstar_fb_remove: dev is NULL pointer \n");
        return -ENOTTY;
    }
    for (i = 0;  i < numFbHwlayer; i++)
    {
        _fb_gwin_enable(fbdevControllers[i].stFbdevWindowCtrl.hWindowHandle,
            FALSE);
        fbdevControllers[i].stFbdevLayerCtrl.bShown = FALSE;
    }
    pinfo = platform_get_drvdata(dev);
    for (i = 0;  i < numFbHwlayer; i++)
    {
        unregister_framebuffer(pinfo[i]);
        fb_dealloc_cmap(&(pinfo[i]->cmap));
        framebuffer_release(pinfo[i]);
    }
    if (fbdevControllers)
    {
        if (fbdevControllers->pTempFbdevSurfaceCtrl)
        {
            kfree(fbdevControllers->pTempFbdevSurfaceCtrl);
            fbdevControllers->pTempFbdevSurfaceCtrl = NULL;
        }
        kfree(fbdevControllers);
        fbdevControllers = NULL;
    }
    if (pinfo)
    {
        kfree(pinfo);
        pinfo = NULL;
    }
    platform_set_drvdata(dev, NULL);
    return 0;
}

static void mstar_fb_platform_release(struct device *device)
{
    if (NULL == device)
    {
        printk("ERROR: in mstar_fb_platform_release, \
                device is NULL pointer !\r\n");
    }
    else
    {
        printk("in mstar_fb_platform_release, module unload!\n");
    }
}
/*device .name and driver .name must be the same, then it will call
       probe function */
static struct platform_driver Mstar_fb_driver =
{
    .probe  = mstar_fb_probe,    //initiailize
    .remove = mstar_fb_remove,   /*it free(mem),
                                   release framebuffer, free irq etc. */
    .driver =
    {
        .name = "Mstar-fbdev",
    },
};

static u64 mstar_fb_device_lcd_dmamask = 0xffffffffUL;

static struct platform_device Mstar_fb_device =
{
    .name = "Mstar-fbdev",
    .id = 0,
    .dev =
    {
        .release = mstar_fb_platform_release,
        .dma_mask = &mstar_fb_device_lcd_dmamask,
        .coherent_dma_mask = 0xffffffffUL
    }
};
static int __init mstar_fbdev_init(void)
{
    int ret = 0;

    ret = platform_driver_register(&Mstar_fb_driver);

    if (!ret)
    {
        /*register driver sucess
          register device*/
        ret = platform_device_register(&Mstar_fb_device);
        if(ret)    /*if register device fail, then unregister the driver.*/
        {
            platform_driver_unregister(&Mstar_fb_driver);
        }
    }
    return ret;
}

static void __exit mstar_fbdev_exit(void)
{
    platform_device_unregister(&Mstar_fb_device);
    platform_driver_unregister(&Mstar_fb_driver);
}

static int mstar_fb_open(struct fb_info *info, int user) {
    MI_FBDEV_Controller_t* par = info->par;
    MI_BOOL bSucess = TRUE;
    MI_BOOL bMiDispInitstatus = FALSE;
    bMiDispInitstatus = mstar_FB_GetMiDaemonInitStatus();
    if(!bMiDispInitstatus) {
        pr_info(LOG_TAG "%s fb:%d Midaemon not init done \n",__FUNCTION__,info->node);
        return -EINVAL;
    }

    //Triger by userspace ap
    if (user)
    {
        if (!par->ref_count)
        {
            restoreFbInfo(info);

            pr_err(LOG_TAG "%s fb:%d  bUseMapping = %d \n",__FUNCTION__,info->node,bUseMapping);
            if(bUseMapping) {
                _fb_buf_init(info, info->fix.smem_start);
             }

            //Mark gop has been initialized
            par->bInitialized = TRUE;
            if (par->stFbdevLayerCtrl.stLayerInfo.eLayerId==(MI_OSD_Layer_e)gBootlogoIdx)
            {
                pr_info(LOG_TAG "%s fb:%d bBootlogoDisabled = %d\n",__FUNCTION__,info->node,bBootlogoDisabled);
                bBootlogoOpened=TRUE;
                /* if par->bInitialized = FALSE must be in below case*/
                /* bBootlogoDisabled = false  : bootlogo hadnot been disabled*/
                /* bBootLogostatus = true  : using bootlogo show */
                if(!bBootlogoDisabled && bBootLogostatus )
                {
                    par->bInitialized = FALSE;
                    pr_info(LOG_TAG "%s bBootlogoDisabled=FALSE\n",__FUNCTION__);
                }
            }
            if (par->bInitialized)
            {
                 /**
                 *SurfaceCnt!=0 means that It
                 *has been assigned OSD resolution
                 *and framebuffer info in config file
                 */
              if (par->surfaceCnt)
              {
                  bSucess = _fb_miosd_init(info);
                   if (!bSucess)
                   {
                       pr_err(LOG_TAG "%s:%d _fb_miosd_init failed!\n",__FUNCTION__,__LINE__);
                       return -ENOTTY;
                   }
              } 
            }
        }
        par->ref_count++;
    }
    else
    {
        pr_info(LOG_TAG "%s fb:%d user=%d \n",__FUNCTION__,info->node,user);
    }
    return 0;
}
static int mstar_fb_release(struct fb_info *info, int user)
{
    MI_FBDEV_Controller_t* par = info->par;
    //Triger by userspace ap
    if (user)
    {
        if (!par->ref_count)
            return -EINVAL;
        if (par->ref_count==1)
        {
            if (info->screen_base != NULL && !par->u8UseIOMMU && bUseMapping) {
                iounmap(info->screen_base);
            }
            info->screen_base = NULL;
            bBootlogoOpened=FALSE;
            _fb_miosd_deinit(info);

            /*comment _fb_iommu_free,if process exit abnormally (eg:ctrl+C) kernel will print trace & reboot*/
            //_fb_iommu_free(info);
            //pr_info(LOG_TAG "%s fb:%d user=%d  _fb_iommu_free\n",__FUNCTION__,info->node,user);
        }
        par->ref_count--;
    }
    else
    {
        pr_info("mstar_fb_release fb:%d user=%d \n",info->node,user);
    }
    return 0;
}

static int mstar_fb_mmap(struct fb_info *pinfo, struct vm_area_struct *vma)
{
    size_t size;
    MI_FBDEV_Controller_t* par;
    MI_SYS_MiuType_e emiuSel;
    size = 0;
    if (NULL == pinfo)
    {
        printk("ERROR: mstar_fb_mmap, pinfo is NULL pointer !\n");
        return -ENOTTY;
    }
    par = pinfo->par;
    emiuSel = par->eMiuSel;
    if (NULL == vma)
    {
        printk("ERROR: mstar_fb_mmap, vma is NULL pointer !\n");
        return -ENOTTY;
    }
    if (0 == pinfo->fix.smem_start)
    {
        printk("ERROR: mstar_fb_mmap, physical addr is NULL pointer !\n");
        return -ENOMEM;
    }
    if (!par->bInitialized)
    {
        pr_err(LOG_TAG "%s:%d the fb%d is not initialized!\n",__FUNCTION__,__LINE__,pinfo->node);
        return -EPERM;
    }
    size = vma->vm_end - vma->vm_start;
    if (emiuSel == E_MI_SYS_MIU_TYPE_0) {
        vma->vm_pgoff = (pinfo->fix.smem_start + MIU0_BUS_OFFSET) >> PAGE_SHIFT;
    } else if (emiuSel == E_MI_SYS_MIU_TYPE_1) {
        vma->vm_pgoff = (pinfo->fix.smem_start - MIU1_INTERVAL + MIU1_BUS_OFFSET) >> PAGE_SHIFT;
    }

      pr_info(LOG_TAG "%s vma->vm_start=%x\n vma->vm_end=%x\n vma->vm_pgoff =%x \n",__FUNCTION__,
      (unsigned int) vma->vm_start, (unsigned int)vma->vm_end ,
      (unsigned int)vma->vm_pgoff);

       //set page to no cache
       #if defined(CONFIG_MIPS)
       {
            pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
            pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;
       }
       #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
       {
            //vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
            // pgprot_val(vma->vm_page_prot) = pgprot_noncached(vma->vm_page_prot);
            vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
       }
       #endif
      // Remap-pfn-range will mark the range VM_IO and VM_RESERVED

      if (remap_pfn_range(vma, vma->vm_start,
          vma->vm_pgoff, size, vma->vm_page_prot))
      return -EAGAIN;

      return 0;
}
static int mstar_fb_ioctl(struct fb_info *pinfo, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;
    MI_BOOL bSuccess = TRUE;
    unsigned int dir;
    int curTimingWidth = 1920;
    int curTimingHeight = 1080;
    MI_FB_LayerAttr_t stLayerAttr;
    MI_FBDEV_Controller_t* par = pinfo->par;
    MI_U32 stretchWinXpos =
    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32X;
    MI_U32 stretchWinYpos =
    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32Y;
    MI_U32 stretchWinDstWidth =
        par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth;
    MI_U32 stretchWinDstHeight =
        par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight;
    MI_BOOL bootLogoUsed = (par->stFbdevLayerCtrl.stLayerInfo.eLayerId \
                               == ((MI_OSD_Layer_e)gBootlogoIdx));
    MI_U16 bufferCnt = 0;
    MI_U16 bpx = 0;
    int i = 0;
#if (ENABLE_GFLIP_IOCTL == TRUE)
    unsigned long vsync_gop = 0;
#endif
    union
    {
        MI_FB_Rectangle_t dispRegion;
        MI_FB_GlobalAlpha_t alpahInfo;
        MI_FB_ColorKey_t colorKeyInfo;
        MI_FB_DisplayLayerAttr_t dispLayerAttr;
        MI_FB_CursorAttr_t hwcursorAttr;
        MI_FB_MemInfo_t stMemInfo;
        MI_FB_OsdInfo_t stOSDInfo;
        u8 bShown;
        u16 u16Brightness;
        MI_OSD_ContrastColor_t stcontrastColor;
        MI_FB_HVMirror_t stHVmirror;
        MI_FB_Rectangle_t stDispTimingRect;
        MI_FB_LayerDst_e  eDispLayerDst;
        MI_FB_LayerAttr_t stLayerAttr;
        MI_FB_FlipMode_e  eFlipMode;
        u8 bGOPAutoDetect;
        u32 u32IOMMULength;
    }data;

    mstar_FB_getCurTiming(&curTimingWidth, &curTimingHeight);
    if (_IOC_TYPE(u32Cmd) != FB_IOC_MAGIC) {
        return -ENOTTY;
    }
    if (_IOC_SIZE(u32Cmd) > sizeof(data)) {
        return  -EINVAL;
    }
    dir = _IOC_DIR(u32Cmd);
    if (dir & _IOC_WRITE) {
    if (copy_from_user(&data, (void __user*)u32Arg,_IOC_SIZE(u32Cmd)))
        return -EFAULT;
    }

    switch (u32Cmd)
    {
        case FBIOGET_SCREEN_LOCATION:
        {
            if (par->bInitialized &&
                par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
            {
                data.dispRegion.u16Xpos =
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32X;
                data.dispRegion.u16Ypos =
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32Y;
                data.dispRegion.u16Width =
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth;
                data.dispRegion.u16Height =
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight;
                retval = copy_to_user((MI_FB_Rectangle_t __user*)u32Arg,
                    &data.dispRegion, sizeof(MI_FB_Rectangle_t));
            }
            else
            {
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_SCREEN_LOCATION:
        {
            MI_BOOL xPosEqual =
                (data.dispRegion.u16Xpos == stretchWinXpos);
            MI_BOOL yPosEqual =
                (data.dispRegion.u16Ypos == stretchWinYpos);
            MI_BOOL widthEqual =
                (data.dispRegion.u16Width == stretchWinDstWidth);
            MI_BOOL heightEqual =
                (data.dispRegion.u16Height== stretchWinDstHeight);
            if (par->bInitialized &&
                par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
            {
                if (xPosEqual && yPosEqual
                && widthEqual && heightEqual) {
                    return retval;
                }
                bSuccess = mstar_FB_SetLayerSize(par->stFbdevLayerCtrl.hLayerHandle,
                    data.dispRegion.u16Xpos,data.dispRegion.u16Ypos,
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerWidth,
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerHeight,
                    data.dispRegion.u16Width,data.dispRegion.u16Height);
                if (bSuccess)
                {
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32X =
                        data.dispRegion.u16Xpos;
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32Y =
                        data.dispRegion.u16Ypos;
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth =
                        data.dispRegion.u16Width;
                    par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight =
                        data.dispRegion.u16Height;
                }
                retval = (bSuccess ? 0 : (-EINVAL));
            }
            else
            {
                pr_err(LOG_TAG "FBIOSET_SCREEN_LOCATION layer has not been create or init!\n");
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_SHOW:
        {
            if (par->bInitialized &&
                par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
            {
                retval =
                    __put_user(par->stFbdevLayerCtrl.bShown,(u8 __user*)u32Arg);
            }
            else
            {
                pr_err(LOG_TAG "FBIOGET_SHOW layer has not been create or init!\n");
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_SHOW:
        {
            if (par->bInitialized &&
                par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
            {
                mstar_FB_EnableGwin(par->stFbdevLayerCtrl.hLayerHandle,
                    data.bShown);
                par->stFbdevLayerCtrl.bShown = data.bShown;
            }
            else
            {
                pr_err(LOG_TAG "FBIOSET_SHOW layer has not been create or init!\n");
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_MEM_INFO:
        {
            if(par->u8UseIOMMU) {
                data.stMemInfo.phyAddr = par->phyAddr;
            } else {
                data.stMemInfo.phyAddr = pinfo->fix.smem_start;
            }
            data.stMemInfo.length = pinfo->fix.smem_len;
            retval = copy_to_user((MI_FB_MemInfo_t __user*)u32Arg,
                &(data.stMemInfo), sizeof(MI_FB_MemInfo_t));
        }
        break;
        case FBIOSET_MEM_INFO:
        {

                par->phyAddr = data.stMemInfo.phyAddr;
                if (par->phyAddr > MIU2_INTERVAL)
                {
                    par->eMiuSel = E_MI_SYS_MIU_TYPE_2;
                }
                else if (par->phyAddr > MIU1_INTERVAL)
                {
                    par->eMiuSel = E_MI_SYS_MIU_TYPE_1;
                }
                else
                {
                    par->eMiuSel = E_MI_SYS_MIU_TYPE_0;
                }
                //iounmap original
                if (pinfo->screen_base != NULL && bUseMapping)
                {
                    iounmap(pinfo->screen_base);
                    pinfo->screen_base = NULL;
                }
                par->length = data.stMemInfo.length;

                pinfo->fix.smem_start = par->phyAddr;
                pinfo->fix.smem_len = par->length;

                //mark layer window surface has been created
                if (par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
                {
                    par->bUseExBlockMem = TRUE;
                }
                  //ioremap
                if(bUseMapping) {
                    _fb_buf_init(pinfo, pinfo->fix.smem_start);
                }
        }
        break;
        case FBIOGET_DFB_RESERVED:
        {
           pr_info(LOG_TAG "FBIOGET_DFB_RESERVED dfbreserved=%d\n",par->bReserveDFB);
            retval =
                __put_user(par->bReserveDFB,(u8 __user*)u32Arg);
        }
        break;
        case FBIOGET_BOOTLOGO_USED:
        {
            //1. bootlogo gop equal to current gop && have call FBIO_DISABLE_BOOTLOGO
            //2. bootlogo gop equal not equal to current gop so open fb will create info
            if( par->bInitialized && (par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL))
            {
                bootLogoUsed = FALSE;
            }

            pr_info(LOG_TAG "%s bootLogoUsed = %d \n",__FUNCTION__,bootLogoUsed);
            retval =
                __put_user(bootLogoUsed, (u32 __user*)u32Arg);
        }
        break;
        case FBIO_DISABLE_BOOTLOGO:
        {
            if (bootLogoUsed)
            {
                if (!par->bInitialized)
                {
                    bSuccess = mstar_FB_DisalbeBootlogo();
                    if (bSuccess)
                        par->bInitialized  = TRUE;
                    retval = bSuccess ? 0 :(-EINVAL);
                }
            }
        }
        break;
        case FBIO_BEGIN_TRANSACTION:
        {
            if (par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
                mstar_FB_BeginTransaction(par->stFbdevLayerCtrl.hLayerHandle);
        }
        break;
        case FBIO_COMMIT_TRANSACTION:
        {
            if (par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL)
                mstar_FB_EndTransaction(par->stFbdevLayerCtrl.hLayerHandle);
        }
        break;
        case FBIOGET_OSD_INFO:
        {
            if (par->bInitialized && par->surfaceCnt)
            {
                data.stOSDInfo.u32Width = pinfo->var.xres;
                data.stOSDInfo.u32Height = pinfo->var.yres;
                data.stOSDInfo.u32Pitch = pinfo->fix.line_length;
                data.stOSDInfo.eColorFmt
                    = (MI_FB_ColorFmt_e)(par->stFbdevWindowCtrl.stWindowInfo.eColorFormat);
                retval = copy_to_user((MI_FB_OsdInfo_t __user*)u32Arg,
                    &(data.stOSDInfo), sizeof(MI_FB_OsdInfo_t));
            }
            else
            {
                pr_err(LOG_TAG "FBIOGET_OSD_INFO the fb%d is not initialized or set osd info!\n",pinfo->node);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_OSD_INFO:
        {
            /**
             *par->stFbdevLayerCtrl.hLayerHandle means that
             *Layer,Window,Surface of MI_OSD term did not create
             *yet, means that no APP use fbdev to display UI
             */
            if (par->stFbdevLayerCtrl.hLayerHandle == MI_HANDLE_NULL)
            {
                if (data.stOSDInfo.u32Pitch * data.stOSDInfo.u32Height
                   > pinfo->fix.smem_len)
                {
                    pr_err(LOG_TAG "FBIOSET_OSD_INFO the osd use memory is larger than current fb mem\n");
                    retval = -EINVAL;
                }
                //Layer info
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32X = 0;
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32Y = 0;
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerWidth
                    = data.stOSDInfo.u32Width;
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32LayerHeight
                    = data.stOSDInfo.u32Height;
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstWidth
                    = curTimingWidth;
                par->stFbdevLayerCtrl.stLayerInfo.stLayerCustomSize.u32DstHeight
                    = curTimingHeight;
                par->stFbdevLayerCtrl.bShown = false;
                //Window info
                par->stFbdevWindowCtrl.stWindowInfo.stRect.u32X = 0;
                par->stFbdevWindowCtrl.stWindowInfo.stRect.u32Y = 0;
                par->stFbdevWindowCtrl.stWindowInfo.stRect.u32Width = data.stOSDInfo.u32Width;
                par->stFbdevWindowCtrl.stWindowInfo.stRect.u32Height = data.stOSDInfo.u32Height;
                par->stFbdevWindowCtrl.stWindowInfo.u32SurfaceWidth = data.stOSDInfo.u32Width;
                par->stFbdevWindowCtrl.stWindowInfo.u32SurfaceHeight = data.stOSDInfo.u32Height;
                par->stFbdevWindowCtrl.stWindowInfo.eColorFormat = data.stOSDInfo.eColorFmt;
                //Surface Info
                bpx = mstar_FB_GetBpp(data.stOSDInfo.eColorFmt);
                if (data.stOSDInfo.u32Width && data.stOSDInfo.u32Height
                    && bpx && pinfo->fix.smem_len)
                {
                    bufferCnt = getBufferCount( pinfo->fix.smem_len,
                        data.stOSDInfo.u32Width, data.stOSDInfo.u32Height, bpx);
                }
                else
                {
                    bufferCnt = 0;
                }
                par->surfaceCnt = bufferCnt;
                for (i=0; i < par->surfaceCnt; i++)
                {
                    par->stFbdevSurfaceCtrl[i].hSurfaceHandle
                        = MI_HANDLE_NULL;
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.eOwner
                        = E_MI_OSD_SURFACE_OWNER_AP;
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.eMemoryType
                        = E_MI_OSD_MEMORY_PHY_OS;
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.bReArrange
                        = FALSE;
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Width
                        = data.stOSDInfo.u32Width;
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Height
                        = data.stOSDInfo.u32Height;
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.u32Pitch
                        = ALIGN_UP(data.stOSDInfo.u32Pitch, 64);
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.eColorFormat
                        = (MI_OSD_ColorFormat_e)(data.stOSDInfo.eColorFmt);
                    par->stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr
                        = par->phyAddr+ i * ALIGN_UP(data.stOSDInfo.u32Pitch, 64) * data.stOSDInfo.u32Height;
                    pr_info(LOG_TAG "Init Surface %d  physical addr=0x%llx\n",
                        i,par->stFbdevSurfaceCtrl[i].stSurfaceInfo.phyAddr);
             }
                //update var info

                 (pinfo->var).yres_virtual = bufferCnt*(data.stOSDInfo.u32Height);
                 (pinfo->var).xres = data.stOSDInfo.u32Width;
                 (pinfo->var).yres = data.stOSDInfo.u32Height;
                 (pinfo->var).xres_virtual = data.stOSDInfo.u32Width;
            }
            else
            {
                pr_err(LOG_TAG "FBIOSET_OSD_INFO the fb%d has been used by App!\n",pinfo->node);
                retval = -EINVAL;
            }
        }
        break;
        case FBIO_MIOSD_INIT:
        {
            if (par->stFbdevLayerCtrl.hLayerHandle == MI_HANDLE_NULL)
            {
                if (par->bInitialized && par->surfaceCnt)
                {
                    bSuccess = _fb_miosd_init(pinfo);
                    if (!bSuccess)
                    {
                        pr_err(LOG_TAG "%s:%d _fb_miosd_init failed!\n",__FUNCTION__,__LINE__);
                        retval = -EINVAL;
                    }
                }
                else
                {
                    pr_err(LOG_TAG "%s:%d not init gop or setmem and osdinfo correct!\n",__FUNCTION__,__LINE__);
                    retval = -EINVAL;
                }
           }
           else
           {
                retval = -EPERM;
           }
        }
        break;
        case FBIOSET_LAYER_BRIGHTNESS:
        {
            MI_U16 u16ParaBrightness = par->stFbdevLayerCtrl.brightness;
            if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL &&  \
                par->bInitialized )
            {
                pr_info(LOG_TAG "Brightness set case layerhandle %x Brightness %d \n", \
                    par->stFbdevLayerCtrl.hLayerHandle,data.u16Brightness);
                if( u16ParaBrightness == data.u16Brightness )
                {
                    pr_info(LOG_TAG "Brightness is equal to previous skipped \n");
                    break;
                }
                mstar_FB_SetLayerBrightness(par->stFbdevLayerCtrl.hLayerHandle,
                    data.u16Brightness);
                par->stFbdevLayerCtrl.brightness = data.u16Brightness;
            }
            else
            {
                pr_err(LOG_TAG "FBIOSET_LAYER_CONTRAST layer has not been create or init!\n");
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_LAYER_CONTRAST:
        {
            MI_OSD_ContrastColor_t stparaContrastColor;
            memcpy(&stparaContrastColor,&(data.stcontrastColor),sizeof(MI_OSD_ContrastColor_t));
            if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL &&  \
                par->bInitialized )
            {
                pr_info(LOG_TAG "contrast set case layerhandle is %x Brightness Y %d U %d V %d\n", \
                    par->stFbdevLayerCtrl.hLayerHandle,data.stcontrastColor.u16ContrastY,data.stcontrastColor.u16ContrastU,data.stcontrastColor.u16ContrastV);
                if( ! memcmp(&stparaContrastColor,&(par->stFbdevLayerCtrl.contrastcolor),sizeof(MI_OSD_ContrastColor_t)))
                {
                    pr_info(LOG_TAG "Contrast Color is equal to previous skipped \n");
                    break;
                }
                mstar_FB_SetLayerContrast(par->stFbdevLayerCtrl.hLayerHandle,
                    &(data.stcontrastColor));
                memcpy(&(par->stFbdevLayerCtrl.contrastcolor),&(data.stcontrastColor),sizeof(MI_OSD_ContrastColor_t));
           }
            else
            {
                pr_err(LOG_TAG "FBIOSET_LAYER_CONTRAST layer has not been create or init!\n");
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_LAYER_HVMIRROR:
        {
            if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL &&  \
                par->bInitialized )
            {
                pr_info(LOG_TAG "HVmirror set case layerhandle %x  HV mirror H %d V %d\n", \
                    par->stFbdevLayerCtrl.hLayerHandle,data.stHVmirror.u8HMirror, data.stHVmirror.u8VMirror);
                if( data.stHVmirror.u8HMirror && !data.stHVmirror.u8VMirror)
                {
                    mstar_FB_SetHMirror(par->stFbdevLayerCtrl.hLayerHandle,true);
                    mstar_FB_SetVMirror(par->stFbdevLayerCtrl.hLayerHandle,false);
                }
                else if( data.stHVmirror.u8VMirror && !data.stHVmirror.u8HMirror)
                {
                    mstar_FB_SetHMirror(par->stFbdevLayerCtrl.hLayerHandle,false);
                    mstar_FB_SetVMirror(par->stFbdevLayerCtrl.hLayerHandle,true);
                }
                else if( data.stHVmirror.u8VMirror && data.stHVmirror.u8HMirror )
                {
                    mstar_FB_SetHMirror(par->stFbdevLayerCtrl.hLayerHandle,true);
                    mstar_FB_SetVMirror(par->stFbdevLayerCtrl.hLayerHandle,true);
                }
                else
                {
                    mstar_FB_SetHMirror(par->stFbdevLayerCtrl.hLayerHandle,false);
                    mstar_FB_SetVMirror(par->stFbdevLayerCtrl.hLayerHandle,false);
                }

            }
            else
            {
                pr_err(LOG_TAG "FBIOSET_LAYER_CONTRAST layer has not been create or init!\n");
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_DISP_TIMING:
        {

          data.stDispTimingRect.u16Xpos = 0;
          data.stDispTimingRect.u16Ypos = 0;
          data.stDispTimingRect.u16Width = curTimingWidth;
          data.stDispTimingRect.u16Height = curTimingHeight;
          retval = copy_to_user((MI_FB_Rectangle_t __user*)u32Arg,
                         &data.stDispTimingRect, sizeof(MI_FB_Rectangle_t));
        }
        break;
        case FBIOSET_LAYER_DST:
        {
         if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL &&  \
            par->bInitialized )
         {
            mstar_FB_SetLayerDst(par->stFbdevLayerCtrl.hLayerHandle,data.eDispLayerDst);
         }
        }
        break;
        case FBIOGET_LAYER_ATTR:
        {

         if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL &&  \
            par->bInitialized )
         {
             retval = mstar_FB_GetLayerAttr(par->stFbdevLayerCtrl.hLayerHandle,&data.stLayerAttr);
         }

         if(retval)
         {
             retval = copy_to_user((MI_FB_LayerAttr_t __user*)u32Arg,\
                                       &(data.stLayerAttr), sizeof(MI_FB_LayerAttr_t));
         }
         else
         {
             retval = -EINVAL;
         }

        }
        break;
        case FBIOSET_LAYER_ATTR:
        {

         if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL && \
            par->bInitialized )
         {
             retval = mstar_FB_SetLayerAttr(par->stFbdevLayerCtrl.hLayerHandle,data.stLayerAttr);
         }
        }
        break;
        case FBIOGET_FlipMode:
        {
            if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL && \
                  par->bInitialized )
             {
                  retval = __put_user(par->u8FlipMode, (MI_FB_FlipMode_e __user*)u32Arg);
             }
        }
        break;
        case FBIOSET_FlipMode:
        {
             if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL && \
                  par->bInitialized )
             {
                   par->u8FlipMode = data.eFlipMode;
             }
             retval = 0;
        }
        break;
        case FBIOSET_GOP_AUTO_DETECT:
        {
             if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL && \
                  par->bInitialized )
             {
                   par->bEnable_GOP_AutoDetect = data.bGOPAutoDetect;

                   stLayerAttr.eLayerAttrType = E_MI_FB_ATTR_TYPE_AUTO_DETECT_BUFFER;
                   stLayerAttr.unLayerAttrParam.stAutoDectBufInfo.bEnable = \
                   par->bEnable_GOP_AutoDetect?TRUE:FALSE;
                   stLayerAttr.unLayerAttrParam.stAutoDectBufInfo.u8AlphaThreshold = 0;
                   stLayerAttr.unLayerAttrParam.stAutoDectBufInfo.bLargeThanThreshold = FALSE;

                   mstar_FB_BeginTransaction(par->stFbdevLayerCtrl.hLayerHandle);
                   retval = mstar_FB_SetLayerAttr(par->stFbdevLayerCtrl.hLayerHandle,stLayerAttr);
                   mstar_FB_EndTransaction(par->stFbdevLayerCtrl.hLayerHandle);
                   if(!retval) {
                       pr_err(LOG_TAG "Set Layer auto detect fail! skip it \n");
                       retval = -EINVAL;
                   }
                   else {
                       //pr_info(LOG_TAG "Set Layer auto detect Success!!! \n");
                       retval = 0;
                   }
             }
        }
        break;
        case FBIOGET_GOP_AUTO_DETECT:
        {
             if ( par->stFbdevLayerCtrl.hLayerHandle != MI_HANDLE_NULL && \
                  par->bInitialized )
             {
                   retval = __put_user(par->bEnable_GOP_AutoDetect, (u8 __user*)u32Arg);
             }
        }
        break;
        case FBIOGET_IOMMU_USED:
        {
             pr_info(LOG_TAG "fb%d Get u8UseIOMMU=%d \n",pinfo->node,par->u8UseIOMMU);
             retval = __put_user(par->u8UseIOMMU, (u8 __user*)u32Arg);
        }
        break;
        case FBIO_AllOCATE_IOMMU:
        {
            par->length = data.u32IOMMULength;
            pr_info(LOG_TAG "fb%d Allocate IOMMU Memory length %x\n",pinfo->node,par->length);

            _fb_iommu_allocate(pinfo);
            if(bUseMapping) {
                _fb_buf_init(pinfo, pinfo->fix.smem_start);
            }
        }
        break;
        case FBIO_FREE_IOMMU:
        {
            if(par->u8UseIOMMU && par->phyAddr >= IOMMU_PHYADDROFFSET) {
                pr_info(LOG_TAG "fb%d Free IOMMU Memory\n",pinfo->node);
                if (par->stFbdevWindowCtrl.hWindowHandle != MI_HANDLE_NULL)  {
                    mstar_FB_WindowDestroy(par->stFbdevWindowCtrl.hWindowHandle);
                    par->stFbdevWindowCtrl.hWindowHandle = MI_HANDLE_NULL;
                }
                for (i = 0; i < par->surfaceCnt; i++)  {
                    if (par->stFbdevSurfaceCtrl[i].hSurfaceHandle != MI_HANDLE_NULL)  {
                        mstar_FB_SurfaceDestroy(par->stFbdevSurfaceCtrl[i].hSurfaceHandle);
                        par->stFbdevSurfaceCtrl[i].hSurfaceHandle = MI_HANDLE_NULL;
                    }
                }
                _fb_iommu_free(pinfo);
            }
        }
        break;
        case FBIO_MIOSD_REINIT:
        {
            bSuccess = _fb_miosd_reinit(pinfo);
            retval = (bSuccess ? 0 : (-EINVAL));
        }
        break;
#if (ENABLE_GFLIP_IOCTL == TRUE)
        case FBIO_WAITFORVSYNC:
        {
             if (copy_from_user(&vsync_gop, (void __user*)u32Arg, _IOC_SIZE(u32Cmd))) {
                 return -EFAULT;
             }

             retval = MDrv_GFLIP_WaitForVsync(vsync_gop);
        }
        break;
#endif
    }
    return retval;
}
#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)
#else
module_init(mstar_fbdev_init);
module_exit(mstar_fbdev_exit);
module_param(bUseMapping,int,S_IRUGO);


MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GRAPHIC ioctrl driver");
MODULE_LICENSE("GPL");
#endif//#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)
