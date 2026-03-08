/*
 * adf_debug.h
 *
 * debugfs include operate of log/cli/state
 *
 * Copyright 2020-2021 Amazon Technologies, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef _ADF_DEBUG_H_
#define _ADF_DEBUG_H_

#include <linux/workqueue.h>
#include <linux/kthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ADF_DEBUG_LOG_DUMP_MIN_PERIOD (20)

typedef int (*adf_mem_cpy) (uintptr_t, int);
typedef int (*adf_chk_run) (void);

void adfDebug_printLog(uint32_t opt);
void adfDebug_init(void *debugCheckRunFunc, void *debugReadFunc,int32_t interval, uint32_t log_buf_size);

#ifdef __cplusplus
}
#endif

#endif  /* _ADF_DEBUG_H_ */
