/*
 * Copyright (C) 2018 MediaTek Inc.
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

#ifndef _H_HIFI4DSP_LOAD_H_
#define _H_HIFI4DSP_LOAD_H_

#include <hifi4dsp_spi/hifi4dsp_spi.h>

/* Callback type define */
typedef void (*callback_fn)(void *arg);

extern int hifi4dsp_run_status(void);
extern int async_load_hifi4dsp_bin_and_run(callback_fn callback, void *param);

extern void mtcmos_init(void);
extern void mtcmos_deinit(void);
extern int hifi4dsp_get_log_buf_size(u32 *log_size);
extern int hifi4dsp_get_log_buf(char *log_buf, u32 log_size);
extern int hifi4dsp_rst(void);
extern void clr_hifi4dsp_run_status(void);

extern void hifi4dsp_hw_rst(void);

#endif /*_H_HIFI4DSP_LOAD_H_*/
