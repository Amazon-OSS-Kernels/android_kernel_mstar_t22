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
#ifndef _MDRV_GRAPHIC_ADAPTER_H
#define _MDRV_GRAPHIC_ADAPTER_H

#ifndef MSOS_TYPE_LINUX_KERNEL
#define MSOS_TYPE_LINUX_KERNEL
#endif

#ifndef MI_ENABLE_DBG
#define MI_ENABLE_DBG 1
#endif

#ifndef MI_OS_WRAPPER_PRINTF
#define MI_OS_WRAPPER_PRINTF 1
#endif

/* MI header file */
#include "mi_common.h"
#include "mi_osd.h"
#include "uapi/mstarFb.h"


#ifdef _MDRV_GRAPHIC_ADAPTER_H
#define INTERFACE
#else
#define INTERFACE extern
#endif

#if defined(__cplusplus)
extern "C" {
#endif


//=============================================================================
// Function
//=============================================================================
INTERFACE MI_BOOL mstar_FB_InitContext(void);
INTERFACE MI_BOOL mstar_FB_LayerCreate(MI_OSD_LayerInfo_t* pstLayerInfo,
    MI_HANDLE* retLayerHandle);
INTERFACE MI_BOOL mstar_FB_WindowCreate(MI_OSD_WindowInfo_t* pstWindowInfo,
    MI_HANDLE* retWindowHandle);
INTERFACE MI_BOOL mstar_FB_SurfaceCreate(MI_OSD_SurfaceInfo_t* pstSurfaceInfo,
    MI_HANDLE* retSurfaceHandle);
INTERFACE MI_BOOL mstar_FB_LayerDestroy(MI_HANDLE layerHandle);
INTERFACE MI_BOOL mstar_FB_WindowDestroy(MI_HANDLE windowHandle);
INTERFACE MI_BOOL mstar_FB_SurfaceDestroy(MI_HANDLE surfaceHandle);
INTERFACE MI_BOOL mstar_FB_WindowFlipByExternSurface(MI_HANDLE windowHandle, MI_HANDLE surfaceHandle);
INTERFACE MI_BOOL mstar_FB_WindowFlipByExternQueue(MI_HANDLE windowHandle, MI_HANDLE surfaceHandle,MI_U8 surfaceCount);
INTERFACE void mstar_FB_SetHMirror(MI_HANDLE hLayer, MI_BOOL bEnable);
INTERFACE void  mstar_FB_SetVMirror(MI_HANDLE hLayer, MI_BOOL bEnable);
INTERFACE void  mstar_FB_SetHVMirror(MI_HANDLE hLayer, MI_BOOL bEnable);
INTERFACE void mstar_FB_EnableTransClr_EX(MI_HANDLE hLayer, MI_BOOL bEnable);
INTERFACE void mstar_FB_SetBlending(MI_HANDLE hWindow, MI_BOOL bPixelAlpha, MI_U8 u8coef);

INTERFACE MI_BOOL mstar_FB_SetLayerSize(MI_HANDLE hLayer, MI_U32 xPos, MI_U32 yPos,
           MI_U32 srcWidth, MI_U32 srcHeight, MI_U32 dstWidth, MI_U32 dstHeight);
INTERFACE void mstar_FB_EnableGwin(MI_HANDLE hLayerHandle, MI_BOOL bEnable);

INTERFACE void mstar_FB_BeginTransaction(MI_HANDLE hLayerHandle);
INTERFACE void mstar_FB_EndTransaction(MI_HANDLE hLayerHandle);
INTERFACE MI_U16 mstar_FB_GetBpp(MI_OSD_ColorFormat_e eColorFmt);
INTERFACE void mstar_FB_SetTransClr_8888(MI_HANDLE hLayerHandle, MI_U32 clr);
INTERFACE void mstar_FB_EnableMultiAlpha(MI_HANDLE hLayerHandle, MI_BOOL bEnable);
INTERFACE void mstar_FB_SetNewAlphaMode(MI_HANDLE hLayerHandle, MI_BOOL bEnable);
INTERFACE int mstar_FB_getCurTiming(int *ret_width, int *ret_height);
INTERFACE MI_BOOL mstar_FB_DisalbeBootlogo(void);
INTERFACE MI_BOOL mstar_FB_SetLayerBrightness(MI_HANDLE hLayer, const MI_U16 DstBrightness);
INTERFACE MI_BOOL mstar_FB_SetLayerContrast(MI_HANDLE hLayer, const MI_OSD_ContrastColor_t *pstContrastColor);
INTERFACE MI_BOOL mstar_FB_SetLayerDst(MI_HANDLE hLayer, const MI_OSD_LayerDestination_e eLayerDst);
INTERFACE MI_BOOL mstar_FB_SurfaceClear(const MI_HANDLE paraSurfaceHandle);
INTERFACE MI_BOOL mstar_FB_SurfaceClearColor(const MI_HANDLE paraSurfaceHandle,const MI_OSD_RgbColor_t* pstcolor);
INTERFACE MI_BOOL mstar_FB_GetLayerAttr(MI_HANDLE hLayer,MI_FB_LayerAttr_t* stLayerAttr);
INTERFACE MI_BOOL mstar_FB_SetLayerAttr(MI_HANDLE hLayer,const MI_FB_LayerAttr_t stLayerAttr);
INTERFACE MI_BOOL mstar_FB_RegisterCallback(const MI_OSD_CallbackInputParams_t* pstCallbackInputParams);
INTERFACE MI_BOOL mstar_FB_SetLayerPalette(MI_HANDLE hLayer, const MI_U32 *u32Palettetable);
INTERFACE MI_BOOL mstar_FB_LayerSupportPalette(MI_HANDLE hLayer,MI_U8* pu8SupportPalette);
INTERFACE MI_BOOL mstar_FB_LayerSetCustomerSize(MI_HANDLE hLayer, MI_OSD_LayerCustomSize_t* pstLayerInfo);
INTERFACE MI_BOOL mstar_FB_GetMiDaemonInitStatus(void);
INTERFACE MI_BOOL mstar_FB_SetVsyncMode(MI_HANDLE hLayer,MI_BOOL bVsyncMode);
INTERFACE MI_BOOL mstar_FB_GetMiConfigUpdated(MI_HANDLE hLayer);

#if defined(__cplusplus)
}
#endif

#undef INTERFACE

#endif //_MDRV_GRAPHIC_H
