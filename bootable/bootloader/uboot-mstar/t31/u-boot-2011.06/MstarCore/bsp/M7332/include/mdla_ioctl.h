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
#ifndef __MDLA_IOCTL_H__
#define __MDLA_IOCTL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/ioctl.h>
#include <linux/types.h>

enum mem_type {
	MEM_DRAM,
	MEM_IOMMU,
	MEM_GSM
};

enum PMU_INTERFACE {
	PMU_IF_WDEC0 = 0xe,
	PMU_IF_WDEC1 = 0xf,
	PMU_IF_CBLD0 = 0x10,
	PMU_IF_CBLD1 = 0x11,
	PMU_IF_SBLD0 = 0x12,
	PMU_IF_SBLD1 = 0x13,
	PMU_IF_STE0 = 0x14,
	PMU_IF_STE1 = 0x15,
	PMU_IF_CMDE = 0x16,
	PMU_IF_DDE = 0x17,
	PMU_IF_CONV = 0x18,
	PMU_IF_RQU = 0x19,
	PMU_IF_POOLING = 0x1a,
	PMU_IF_EWE = 0x1b,
	PMU_IF_CFLD = 0x1c
};

enum PMU_DDE_EVENT {
	PMU_DDE_WORK_CYC = 0x0,
	PMU_DDE_TILE_DONE_CNT,
	PMU_DDE_EFF_WORK_CYC,
	PMU_DDE_BLOCK_CNT,
	PMU_DDE_READ_CB_WT_CNT,
	PMU_DDE_READ_CB_ACT_CNT,
	PMU_DDE_WAIT_CB_TOKEN_CNT,
	PMU_DDE_WAIT_CONV_RDY_CNT,
	PMU_DDE_WAIT_CB_FCWT_CNT,
};

enum PMU_MODE {
	PMU_ACC_MODE = 0x0,
	PMU_INTERVAL_MODE = 0x1,
};

struct ioctl_malloc {
	__u32 size;
	__u32 mva; // Physical Address for Device
        __u64 pa;  // Physical Address for CPU
	__u64 kva; // Virtual Address for Kernel
	__u8 type;
	__u64 data; // Virtual Address for User
	__u32 mva_h; // Physical Address for Device
};

struct ioctl_run_cmd {
	__u64 kva; // Virtual Address for Kernel
	__u32 mva; // Physical Address for Device
	__u32 count;
	__u32 id;
	__u32 mva_h; // Physical Address for Device
};

struct ioctl_buf_sync {
	__u32 size;
	__u32 mva; // Physical Address for Device
	__u64 kva; // Virtual Address for Kernel
	__u32 type; //1.clean; 2.invalidate 3,clean+invalidate
	__u32 mva_h; // Physical Address for Device
};

struct ioctl_perf {
	int handle;
	__u32 interface;
	__u32 event;
	__u32 counter;
	__u32 start;
	__u32 end;
	__u32 mode;
};

enum MDLA_CONFIG {
	MDLA_CFG_NONE = 0,
	MDLA_CFG_TIMEOUT_GET = 1,
	MDLA_CFG_TIMEOUT_SET = 2,
	MDLA_CFG_FIFO_SZ_GET = 3,
	MDLA_CFG_FIFO_SZ_SET = 4,
	MDLA_CFG_GSM_INFO = 5,
	MDLA_CFG_EFUSE_INFO = 6,
};

struct ioctl_config {
	__u32 op;
	__u32 arg_count;
	__u64 arg[8];
};

struct ioctl_priv_test {
	__u32 op;
	__u32 arg_count;
	__u64 arg[8];
};

#define IOC_MDLA '\x1d'

#define IOCTL_MALLOC		_IOWR(IOC_MDLA, 0, struct ioctl_malloc)
#define IOCTL_FREE		_IOWR(IOC_MDLA, 1, struct ioctl_malloc)
#define IOCTL_RUN_CMD_SYNC	_IOWR(IOC_MDLA, 2, struct ioctl_run_cmd)
#define IOCTL_RUN_CMD_ASYNC	_IOWR(IOC_MDLA, 3, struct ioctl_run_cmd)
#define IOCTL_WAIT_CMD		_IOWR(IOC_MDLA, 4, struct ioctl_run_cmd)
#define IOCTL_PERF_SET_EVENT	_IOWR(IOC_MDLA, 5, struct ioctl_perf)
#define IOCTL_PERF_GET_EVENT	_IOWR(IOC_MDLA, 6, struct ioctl_perf)
#define IOCTL_PERF_GET_CNT	_IOWR(IOC_MDLA, 7, struct ioctl_perf)
#define IOCTL_PERF_UNSET_EVENT	_IOWR(IOC_MDLA, 8, struct ioctl_perf)
#define IOCTL_PERF_GET_START	_IOWR(IOC_MDLA, 9, struct ioctl_perf)
#define IOCTL_PERF_GET_END	_IOWR(IOC_MDLA, 10, struct ioctl_perf)
#define IOCTL_PERF_GET_CYCLE	_IOWR(IOC_MDLA, 11, struct ioctl_perf)
#define IOCTL_PERF_RESET_CNT	_IOWR(IOC_MDLA, 12, struct ioctl_perf)
#define IOCTL_PERF_RESET_CYCLE	_IOWR(IOC_MDLA, 13, struct ioctl_perf)
#define IOCTL_PERF_SET_MODE	_IOWR(IOC_MDLA, 14, struct ioctl_perf)
#define IOCTL_BUF_SYNC		_IOWR(IOC_MDLA, 15, struct ioctl_buf_sync)
#define IOCTL_ABORT_CMD		_IOWR(IOC_MDLA, 16, struct ioctl_run_cmd)
#define IOCTL_CONFIG		_IOWR(IOC_MDLA, 64, struct ioctl_config)

#if defined(MSOS_TYPE_LINUX_KERNEL)
long mdla_kernel_ioctl(unsigned int command, void *arg);
#endif //MSOS_TYPE_LINUX_KERNEL

#ifdef __cplusplus
}
#endif

#endif
