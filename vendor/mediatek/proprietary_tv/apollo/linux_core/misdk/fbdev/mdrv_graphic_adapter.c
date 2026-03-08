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
#include "mdrv_graphic_adapter.h"
#include "mi_disp.h"
#include "mi_osd.h"
#include "mi_common.h"
#include "mi_sys.h"
#define LOG_TAG "[FBDEV] "
//=============================================================================
// Macros
//=============================================================================
#define ALIGN_CHECK(value,factor) ((value + factor-1) & (~(factor-1)))

/*Surfacecnt using for print all created mi osd surfacecnt,mainly for miosd surface leak debug*/
int gSurfacecnt = 0;

/*mstar_fb_open will wait midaemon init done
  midaemon will Init all mi module
*/
MI_BOOL mstar_FB_InitContext(void)
{
    MI_OSD_InitParams_t stInitPar;
    MI_RESULT ret = MI_OK;
    memset(&stInitPar,0,sizeof(MI_OSD_InitParams_t));
    stInitPar.bAutoFlip = FALSE;
    stInitPar.bWaitIdle = MI_HANDLE_NULL;
    stInitPar.bWaitSync = FALSE;
    stInitPar.bWaitIdle = MI_HANDLE_NULL;
    stInitPar.eCanvasBufMode = E_MI_OSD_WINDOW_CANVAS_AUTO_SWITCH;
    MI_OSD_Init(&stInitPar);
    return (ret == MI_OK);
}
MI_BOOL mstar_FB_LayerCreate(MI_OSD_LayerInfo_t* pstLayerInfo,
    MI_HANDLE* retLayerHandle)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerCreate(pstLayerInfo, retLayerHandle);
    if (ret != MI_OK)
        *retLayerHandle = MI_HANDLE_NULL;

    pr_info(LOG_TAG "%s ret=%d LayerHandle=%x \n",__FUNCTION__,ret, *retLayerHandle);
    return (ret == MI_OK);
}
MI_BOOL mstar_FB_WindowCreate(MI_OSD_WindowInfo_t* pstWindowInfo,
    MI_HANDLE* retWindowHandle)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_WindowCreate(pstWindowInfo, retWindowHandle);
    if (ret != MI_OK)
        *retWindowHandle = MI_HANDLE_NULL;

     pr_info(LOG_TAG "%s ret=%d,WindowHandle=%x\n",__FUNCTION__,ret,*retWindowHandle);

     return (ret == MI_OK);
}

MI_BOOL mstar_FB_SurfaceCreate(MI_OSD_SurfaceInfo_t* pstSurfaceInfo,
    MI_HANDLE* retSurfaceHandle)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_SurfaceCreate(pstSurfaceInfo, retSurfaceHandle);
    if (ret != MI_OK) {
        pr_info(LOG_TAG "%s physicaladdr=0x%llx,surface create fail ret=%d\n",__FUNCTION__,pstSurfaceInfo->phyAddr,ret);
        *retSurfaceHandle = MI_HANDLE_NULL;
         return FALSE;
     } else {
        ++gSurfacecnt;
        pr_info(LOG_TAG "%s ret=%d physicaladdr=0x%llx,surfacehandle=%x gSurfacecnt=%d\n",\
              __FUNCTION__,ret,pstSurfaceInfo->phyAddr,*retSurfaceHandle,gSurfacecnt);
        return TRUE;
    }
}

MI_BOOL mstar_FB_SurfaceClear(const MI_HANDLE paraSurfaceHandle)
{
    MI_RESULT ret = MI_OK;

    //use for surface pattern
    MI_OSD_RgbColor_t  stcolor;
    memset(&stcolor,0x00,sizeof(stcolor));
    ret = MI_OSD_SurfaceClear(paraSurfaceHandle,&stcolor,NULL);
    return (ret == MI_OK);
}


MI_BOOL mstar_FB_SurfaceClearColor(const MI_HANDLE paraSurfaceHandle,const MI_OSD_RgbColor_t* pstcolor)
{
    MI_RESULT ret = MI_OK;

    ret = MI_OSD_SurfaceClear(paraSurfaceHandle,pstcolor,NULL);
    return (ret == MI_OK);
}


MI_BOOL mstar_FB_LayerDestroy(MI_HANDLE layerHandle)
{
     MI_RESULT ret = MI_OK;
     ret = MI_OSD_LayerDestroy(layerHandle);
     return (ret == MI_OK);
}
MI_BOOL mstar_FB_WindowDestroy(MI_HANDLE windowHandle)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_WindowDestroy(windowHandle);
    pr_info(LOG_TAG "%s windowHandle:%x ret=%d\n",__FUNCTION__,windowHandle,ret);
    return (ret == MI_OK);
}
MI_BOOL mstar_FB_SurfaceDestroy(MI_HANDLE surfaceHandle)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_SurfaceDestroy(surfaceHandle);
    if (ret != MI_OK) {
        pr_info(LOG_TAG "%s SurfaceDestroy fail %x ret=%d\n",__FUNCTION__,surfaceHandle,ret);
        return FALSE;
    } else {
       --gSurfacecnt;
       pr_info(LOG_TAG "%s surfacehandle:%x gSurfacecnt=%d\n",__FUNCTION__,surfaceHandle,gSurfacecnt);
       return TRUE;
    }
}

void mstar_FB_SetHMirror(MI_HANDLE hLayer, MI_BOOL bEnable)
{
    MI_OSD_Mirror_e eOsdMirrorMode = E_MI_OSD_MIRROR_NONE ;
    MI_RESULT ret = MI_OK;

    ret = MI_OSD_LayerGetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MIRROR, NULL, &eOsdMirrorMode);

    if (ret == MI_OK)
    {
        if (bEnable)
        {
            if (eOsdMirrorMode==E_MI_OSD_MIRROR_VERTICAL)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL;
            }
            else if (eOsdMirrorMode == E_MI_OSD_MIRROR_NONE ||
                eOsdMirrorMode == E_MI_OSD_MIRROR_MAX)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_HORIZONTAL;
            }
            else if (eOsdMirrorMode==E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL ||
                eOsdMirrorMode==E_MI_OSD_MIRROR_HORIZONTAL)
            {
                return;
            }
        }
        else
        {
            if (eOsdMirrorMode==E_MI_OSD_MIRROR_VERTICAL ||
                eOsdMirrorMode == E_MI_OSD_MIRROR_NONE ||
                eOsdMirrorMode==E_MI_OSD_MIRROR_MAX)
            {
                return;
            }
            else if (eOsdMirrorMode == E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_VERTICAL;
            }
            else if (eOsdMirrorMode == E_MI_OSD_MIRROR_HORIZONTAL)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_NONE;
            }
        }
        MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MIRROR, &eOsdMirrorMode);
    }
}


void mstar_FB_SetVMirror(MI_HANDLE hLayer, MI_BOOL bEnable)
{
    MI_OSD_Mirror_e eOsdMirrorMode;
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerGetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MIRROR, NULL, &eOsdMirrorMode);
    if (ret==MI_OK)
    {
        if (bEnable)
        {
            if (eOsdMirrorMode==E_MI_OSD_MIRROR_HORIZONTAL)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL;
            }
            else if (eOsdMirrorMode==E_MI_OSD_MIRROR_NONE ||
                eOsdMirrorMode == E_MI_OSD_MIRROR_MAX)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_VERTICAL;
            }
            else if (eOsdMirrorMode==E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL ||
                eOsdMirrorMode==E_MI_OSD_MIRROR_VERTICAL)
            {
                return;
            }
        }
        else
        {
            if (eOsdMirrorMode==E_MI_OSD_MIRROR_HORIZONTAL ||
                eOsdMirrorMode == E_MI_OSD_MIRROR_NONE ||
                eOsdMirrorMode==E_MI_OSD_MIRROR_MAX)
            {
                return;
            }
            else if (eOsdMirrorMode == E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_HORIZONTAL;
            }
            else if (eOsdMirrorMode == E_MI_OSD_MIRROR_VERTICAL)
            {
                eOsdMirrorMode = E_MI_OSD_MIRROR_NONE;
            }
        }
        MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MIRROR, &eOsdMirrorMode);
    }
}


void mstar_FB_SetHVMirror(MI_HANDLE hLayer, MI_BOOL bEnable)
{
    MI_OSD_Mirror_e eOsdMirrorMode;
    if (bEnable) {
         eOsdMirrorMode = E_MI_OSD_MIRROR_HORIZONTAL_VERTICAL;
    } else {
         eOsdMirrorMode = E_MI_OSD_MIRROR_NONE;
    }

    MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_MIRROR, &eOsdMirrorMode);
}


void mstar_FB_EnableTransClr_EX(MI_HANDLE hLayer, MI_BOOL bEnable)
{
    if (bEnable)
    {
        MI_OSD_LayerEnableTransparentColor(hLayer);
    }
    else
    {
        MI_OSD_LayerDisableTransparentColor(hLayer);
    }
}

void mstar_FB_SetBlending(MI_HANDLE hWindow, MI_BOOL bPixelAlpha, MI_U8 u8coef)
{
    MI_OSD_WindowAlphaInfo_t stWinAlphaInfo;
    stWinAlphaInfo.bPixelAlpha = bPixelAlpha;
    stWinAlphaInfo.u8GlobalAlpha = u8coef;
    MI_OSD_WindowSetAlpha(hWindow, &stWinAlphaInfo);
}
void mstar_FB_EnableGwin(MI_HANDLE hLayerHandle, MI_BOOL bEnable)
{
    if (bEnable)
    {
        MI_OSD_LayerShow(hLayerHandle);
    }
    else
    {
        MI_OSD_LayerHide(hLayerHandle);
    }
}

void mstar_FB_BeginTransaction(MI_HANDLE hLayerHandle)
{
    MI_OSD_LayerBeginConfig(hLayerHandle);
}
void mstar_FB_EndTransaction(MI_HANDLE hLayerHandle)
{
    MI_OSD_LayerApplyConfig(hLayerHandle);
}
MI_U16 mstar_FB_GetBpp(MI_OSD_ColorFormat_e eColorFmt)
{
    MI_U16 u16bpp=0;

    switch (eColorFmt)
    {
        case E_MI_OSD_COLOR_FORMAT_RGB565:
        case E_MI_OSD_COLOR_FORMAT_ARGB1555:
        case E_MI_OSD_COLOR_FORMAT_ARGB4444:
        case E_MI_OSD_COLOR_FORMAT_YUV422_YVYU:
        case E_MI_OSD_COLOR_FORMAT_YUV422_YUYV:
        case E_MI_OSD_COLOR_FORMAT_YUV422_UYVY:
            u16bpp = 2;
            break;
        case E_MI_OSD_COLOR_FORMAT_ARGB8888:
            u16bpp = 4;
            break;
        case E_MI_OSD_COLOR_FORMAT_I8:
        case E_MI_OSD_COLOR_FORMAT_I4:
        case E_MI_OSD_COLOR_FORMAT_I2:
        case E_MI_OSD_COLOR_FORMAT_I1:
            u16bpp = 1;
            break;
        default:
            u16bpp = 0xFFFF;
            break;
    }
    return u16bpp;
}
void mstar_FB_SetTransClr_8888(MI_HANDLE hLayer, MI_U32 clr)
{
    MI_OSD_TransparentColor_t stTransparentColor;
    stTransparentColor.eFormat = E_MI_OSD_TRANSPARENT_COLOR_FORMAT_RGB;
    stTransparentColor.u32Color = clr;
    MI_OSD_LayerSetTransparentColor(hLayer, &stTransparentColor);
}
void mstar_FB_EnableMultiAlpha(MI_HANDLE hLayerHandle, MI_BOOL bEnable)
{
    if (bEnable)
    {
        MI_OSD_LayerEnableGlobalAlpha(hLayerHandle);
    }
    else
    {
        MI_OSD_LayerDisableGlobalAlpha(hLayerHandle);
    }
}

void mstar_FB_SetNewAlphaMode(MI_HANDLE hLayerHandle, MI_BOOL bEnable)
{
    if (bEnable)
    {
        MI_OSD_LayerEnableAlphaPremultiply(hLayerHandle);
    }
    else
    {
        MI_OSD_LayerDisableAlphaPremultiply(hLayerHandle);
    }
}

int mstar_FB_getCurTiming(int *ret_width, int *ret_height)
{
    MI_DISP_Timing_e eOutputTiming = E_MI_DISP_TIMING_MAX;
    MI_DISP_OpenControllerParams_t stOpenControllerParams;
    MI_HANDLE hDispController = MI_HANDLE_NULL;
    MI_U32 errCode = MI_ERR_FAILED;

    MI_DISP_GetControllerParams_t stGetControllerParams;
    memset(&stGetControllerParams, 0x0, sizeof(MI_DISP_GetControllerParams_t));
    errCode = MI_DISP_GetController(&stGetControllerParams, &hDispController);
    if(errCode != MI_OK)
    {
        memset(&stOpenControllerParams,0,sizeof(MI_DISP_GetControllerParams_t));
        MI_DISP_OpenController(&stOpenControllerParams, &hDispController);
        pr_err(LOG_TAG "get controller failed!, try to open controller handle !!\n");
    }

    MI_DISP_GetOutputTiming(hDispController,&eOutputTiming);

    switch (eOutputTiming)
    {
        case E_MI_DISP_TIMING_720X480_60I:
        case E_MI_DISP_TIMING_720X480_60P:
        {
            *ret_width = 720;
            *ret_height = 480;
        }
        break;
        case E_MI_DISP_TIMING_720X576_50I:
        case E_MI_DISP_TIMING_720X576_50P:
        {
            *ret_width = 720;
            *ret_height = 576;
        }
        break;
        case E_MI_DISP_TIMING_1280X720_50P:
        case E_MI_DISP_TIMING_1280X720_60P:
        {
            *ret_width = 1280;
            *ret_height = 720;
        }
        break;
        case E_MI_DISP_TIMING_1366X768_60I:
        case E_MI_DISP_TIMING_1366X768_60P:
        case E_MI_DISP_TIMING_1366X768_50I:
        case E_MI_DISP_TIMING_1366X768_50P:
           *ret_width = 1366;
           *ret_height = 768;
        break;
        case E_MI_DISP_TIMING_1368X768_50P:
        case E_MI_DISP_TIMING_1368X768_60P:
           *ret_width = 1368;
           *ret_height = 768;
        break;
        case E_MI_DISP_TIMING_1920X1080_50I:
        case E_MI_DISP_TIMING_1920X1080_60I:
        case E_MI_DISP_TIMING_1920X1080_24P:
        case E_MI_DISP_TIMING_1920X1080_25P:
        case E_MI_DISP_TIMING_1920X1080_30P:
        case E_MI_DISP_TIMING_1920X1080_50P:
        case E_MI_DISP_TIMING_1920X1080_60P:
        {
            *ret_width = 1920;
            *ret_height  =1080;
        }
        break;
        case E_MI_DISP_TIMING_3840X2160_24P:
        case E_MI_DISP_TIMING_3840X2160_25P:
        case E_MI_DISP_TIMING_3840X2160_30P:
        case E_MI_DISP_TIMING_3840X2160_50P:
        case E_MI_DISP_TIMING_3840X2160_60P:
        {
            *ret_width = 3840;
            *ret_height = 2160;
        }
        break;
        case E_MI_DISP_TIMING_4096X2160_24P:
        case E_MI_DISP_TIMING_4096X2160_25P:
        case E_MI_DISP_TIMING_4096X2160_30P:
        case E_MI_DISP_TIMING_4096X2160_50P:
        case E_MI_DISP_TIMING_4096X2160_60P:
        {
            *ret_width = 4096;
            *ret_height = 2160;
        }
        break;
        case E_MI_DISP_TIMING_MAX:
        default:
        {
             pr_err(LOG_TAG "Invalid timming type:%d  \n",eOutputTiming);
        }
    }
    return 0;
}
MI_BOOL mstar_FB_WindowFlipByExternSurface(MI_HANDLE windowHandle, MI_HANDLE surfaceHandle)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_WindowFlipByExternSurface(windowHandle, surfaceHandle);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_WindowFlipByExternQueue(MI_HANDLE windowHandle, MI_HANDLE surfaceHandle,MI_U8 surfaceCount)
{
    MI_RESULT ret = MI_OK;

    MI_OSD_FlipQueueInfo_t stFlipQueueInfo;
    MI_OSD_RenderJob_t stRenderJob;
    memset(&stRenderJob,0x00,sizeof(MI_OSD_RenderJob_t));

    stFlipQueueInfo.hSurface = surfaceHandle;
    stFlipQueueInfo.u8BufferCount = surfaceCount;
    stFlipQueueInfo.stRenderJob = stRenderJob;

    ret = MI_OSD_WindowFlipByExternQueue(windowHandle, &stFlipQueueInfo);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_SetLayerSize(MI_HANDLE hLayer, MI_U32 xPos, MI_U32 yPos,
     MI_U32 srcWidth, MI_U32 srcHeight, MI_U32 dstWidth, MI_U32 dstHeight)
{
    MI_OSD_LayerCustomSize_t stCustomSize;
    MI_RESULT ret = MI_OK;
    stCustomSize.u32X = xPos;
    stCustomSize.u32Y = yPos;
    stCustomSize.u32LayerWidth = srcWidth;
    stCustomSize.u32LayerHeight = srcHeight;
    stCustomSize.u32DstWidth = dstWidth;
    stCustomSize.u32DstHeight = dstHeight;
    ret = MI_OSD_LayerSetCustomSize(hLayer, &stCustomSize);
    return ret == MI_OK;
}
 MI_BOOL mstar_FB_DisalbeBootlogo(void)
 {
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_DisableBootLogo();
    return ret == MI_OK;
 }

 MI_BOOL mstar_FB_SetLayerBrightness(MI_HANDLE hLayer, const MI_U16 DstBrightness)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerSetBrightness(hLayer, DstBrightness);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_SetLayerContrast(MI_HANDLE hLayer, const MI_OSD_ContrastColor_t *pstContrastColor)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerSetContrast(hLayer, pstContrastColor);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_SetLayerDst(MI_HANDLE hLayer, const MI_OSD_LayerDestination_e eLayerDst)
{
    MI_RESULT ret = MI_OK;

    ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_DESTINATION,&eLayerDst);
    return ret == MI_OK;
}

MI_BOOL mstar_FB_GetLayerAttr(MI_HANDLE hLayer,MI_FB_LayerAttr_t* stLayerAttr)
{
    MI_RESULT ret = MI_OK;
    MI_OSD_Rect_t stRect;
    memset(&stRect,0x00,sizeof(stRect));

    switch( stLayerAttr->eLayerAttrType )
    {
     case E_MI_FB_ATTR_TYPE_V_STRETCH:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_V_STRETCH,NULL,(void*)(&stLayerAttr->unLayerAttrParam.eLayerHStrecthMode));
        break;
     case E_MI_FB_ATTR_TYPE_H_STRETCH:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_H_STRETCH,NULL,(void*)(&stLayerAttr->unLayerAttrParam.eLayerHStrecthMode));
        break;
     case E_MI_FB_ATTR_TYPE_3D_MODE:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_3D_MODE,NULL,(void*)(&stLayerAttr->unLayerAttrParam.eLayer3dMode));
        break;
     case E_MI_FB_ATTR_TYPE_MIRROR:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_MIRROR,NULL,(void*)(&(stLayerAttr->unLayerAttrParam.eMirrorMode)));
        printk("Tdebug the eMirrorMode is %d \n",stLayerAttr->unLayerAttrParam.eMirrorMode);
        break;
     case E_MI_FB_ATTR_TYPE_WAIT_SYNC:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_WAIT_SYNC,NULL,(void*)(&stLayerAttr->unLayerAttrParam.bWaitSync));
         break;
     case E_MI_FB_ATTR_TYPE_DESTINATION:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_DESTINATION,NULL,(void*)(&stLayerAttr->unLayerAttrParam.eLayerDst));
        break;
     case E_MI_FB_ATTR_TYPE_SCREEN_SIZE:
        ret = MI_OSD_LayerGetAttr(hLayer,E_MI_OSD_ATTR_TYPE_SCREEN_SIZE,NULL,(void*)(&stRect));
        stLayerAttr->unLayerAttrParam.stScreenSize.u16Xpos = stRect.u32X;
        stLayerAttr->unLayerAttrParam.stScreenSize.u16Ypos = stRect.u32Y;
        stLayerAttr->unLayerAttrParam.stScreenSize.u16Width = stRect.u32Width;
        stLayerAttr->unLayerAttrParam.stScreenSize.u16Height = stRect.u32Height;
        break;
     case E_MI_FB_ATTR_TYPE_BLINK_RATE:
     default:
        pr_err(LOG_TAG "Invalid para %s %d Please check !! \n",__FUNCTION__,__LINE__);
        break;
    }

    return ret == MI_OK;
}

MI_BOOL mstar_FB_SetLayerAttr(MI_HANDLE hLayer,const MI_FB_LayerAttr_t stLayerAttr)
{
   MI_RESULT ret = MI_OK;

   switch( stLayerAttr.eLayerAttrType )
    {
     case E_MI_FB_ATTR_TYPE_V_STRETCH:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_V_STRETCH,(void*)(&stLayerAttr.unLayerAttrParam.eLayerVStretchMode));
        break;
     case E_MI_FB_ATTR_TYPE_H_STRETCH:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_H_STRETCH,(void*)(&stLayerAttr.unLayerAttrParam.eLayerHStrecthMode));
        break;
     case E_MI_FB_ATTR_TYPE_3D_MODE:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_3D_MODE,(void*)(&stLayerAttr.unLayerAttrParam.eLayer3dMode));
        break;
     case E_MI_FB_ATTR_TYPE_MIRROR:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_MIRROR,(void*)(&stLayerAttr.unLayerAttrParam.eMirrorMode));
        break;
     case E_MI_FB_ATTR_TYPE_BLINK_RATE:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_BLINK_RATE,(void*)(&stLayerAttr.unLayerAttrParam.u16Blink_Rate));
        break;
     case E_MI_FB_ATTR_TYPE_WAIT_SYNC:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_WAIT_SYNC,(void*)(&stLayerAttr.unLayerAttrParam.bWaitSync));
         break;
     case E_MI_FB_ATTR_TYPE_DESTINATION:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_DESTINATION,(void*)(&stLayerAttr.unLayerAttrParam.eLayerDst));
        break;
     case E_MI_FB_ATTR_TYPE_AUTO_DETECT_BUFFER:
        ret = MI_OSD_LayerSetAttr(hLayer,E_MI_OSD_ATTR_TYPE_AUTO_DETECT_BUFFER,(void*)(&stLayerAttr.unLayerAttrParam.stAutoDectBufInfo));
        break;
     default:
        pr_err(LOG_TAG "Invalid para %s %d Please check !! \n",__FUNCTION__,__LINE__);
        break;
    }

    return ret == MI_OK;
}

MI_BOOL mstar_FB_RegisterCallback(const MI_OSD_CallbackInputParams_t* pstCallbackInputParams)
{
   MI_RESULT ret = MI_OK;

   MI_OSD_CallbackOutputParams_t stOutputParams;
   memset(&stOutputParams,0,sizeof(MI_OSD_CallbackOutputParams_t));

   ret = MI_OSD_RegisterCallback(NULL,pstCallbackInputParams,&stOutputParams);

   return ret == MI_OK;
}


MI_BOOL mstar_FB_SetLayerPalette(MI_HANDLE hLayer, const MI_U32 *u32Palettetable)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerSetPalette(hLayer, u32Palettetable);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_LayerSupportPalette(MI_HANDLE hLayer,MI_U8* pu8SupportPalette)
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerQueryCapability(hLayer,E_MI_OSD_LAYER_CAPABILITY_PALETTE,pu8SupportPalette);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_LayerSetCustomerSize(MI_HANDLE hLayer, MI_OSD_LayerCustomSize_t* pstLayerInfo )
{
    MI_RESULT ret = MI_OK;
    ret = MI_OSD_LayerSetCustomSize(hLayer,pstLayerInfo);
    pr_err(LOG_TAG "MI_OSD_LayerSetCustomSize return value is %d\n",ret);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_GetMiDaemonInitStatus(void)
{
    MI_RESULT ret = MI_OK;
    MI_SYS_ModuleInfoParams_t stModuleInfoParams;
    memset(&stModuleInfoParams, 0, sizeof(stModuleInfoParams));
    stModuleInfoParams.eModuleId = E_MI_SYS_MODULE_ID_SYS_USER;

    ret = MI_SYS_GetAttr(E_MI_SYS_ATTR_TYPE_MODULE_STATUS ,(void*)&stModuleInfoParams);
    if(ret != MI_OK ) {
        pr_err(LOG_TAG "MI_SYS_GetAttr  E_MI_SYS_ATTR_TYPE_MODULE_STATUS fail return value is %d\n",ret);
        return FALSE;
    } else {
        return stModuleInfoParams.bInited;
    }
}


MI_BOOL mstar_FB_SetVsyncMode(MI_HANDLE hLayer,MI_BOOL bVsyncMode)
{
    MI_BOOL attrParams;
    MI_RESULT ret = MI_OK;
    if (bVsyncMode) {
        attrParams = TRUE;
    } else {
        attrParams = FALSE;
    }

    ret = MI_OSD_LayerSetAttr(hLayer, E_MI_OSD_ATTR_TYPE_CONFIG_WAIT_SYNC, &attrParams);
    pr_err(LOG_TAG "MI_OSD_LayerSetAttr CONFIG_WAIT_SYNC %d ret=%d ",attrParams,ret);
    return ret == MI_OK;
}


MI_BOOL mstar_FB_GetMiConfigUpdated(MI_HANDLE hLayer)
{
    MI_BOOL bUpdated = false;
    MI_RESULT Ret = MI_OK;

    Ret = MI_OSD_LayerConfigUpdated(hLayer);
    if (Ret == MI_OK) {
        bUpdated = true;
    } else {
        pr_err(LOG_TAG "MI_OSD_LayerConfigUpdated timeout");
        bUpdated = false;
    }

    return bUpdated;
}
