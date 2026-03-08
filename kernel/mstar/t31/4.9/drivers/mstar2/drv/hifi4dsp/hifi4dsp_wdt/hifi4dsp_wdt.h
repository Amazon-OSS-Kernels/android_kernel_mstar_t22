/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _H_HIFI4DSP_WDT_H_
#define _H_HIFI4DSP_WDT_H_

#include <linux/notifier.h>

/*
 * Public function API
 */
extern int register_adsp_wdt_notifier(struct notifier_block *nb);
extern int unregister_adsp_wdt_notifier(struct notifier_block *nb);
extern void hifi4dsp_wdt_handler(void);
extern void mtk_dsp_wdt_disable(void);

#endif /*_H_HIFI4DSP_WDT_H_*/
