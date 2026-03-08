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
#ifndef TEE_CLIENT_API_GENERAL_H
#define TEE_CLIENT_API_GENERAL_H

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
#include "tee.h"
#endif
#include "tee_client_api_2.4.h"
#include "tee_client_api_1.0.1.h"
#include "tee_client_api_3.2.h"

typedef struct {
	char devname[256];
	union {
		struct tee_context *ctx;
#ifdef CONFIG_ARM64
		uintptr_t fd;
#else
		int fd;
#endif
	};
	bool reg_mem; //optee 3.2
} TEEC_Context;

typedef struct {
	uint32_t session_id;
	union {
#ifdef CONFIG_ARM64
		uintptr_t fd;
#else
		int fd;
#endif
		struct tee_session *session;
	};
	TEEC_Context_2 ctx;
	TEEC_Context_3 ctx_3;
} TEEC_Session;

typedef struct {
	void *buffer;
	size_t size;
	uint32_t flags;
	/*
	 * Implementation-Defined, must match what the kernel driver have
	 *
	 * Identifier can store a handle (int) or a structure pointer (void *).
	 * Define this union to match case where sizeof(int)!=sizeof(void *).
	 */
	uint32_t reserved;
	union {
		int fd;
		void *ptr;
	} d;
	uint64_t registered;

	/* 2.x */
	int id;
	size_t alloced_size;
	void *shadow_buffer;
	int registered_fd;
	/* 3.2 */
	bool buffer_allocated;
} TEEC_SharedMemory;

typedef struct {
	void *buffer;
	size_t size;
} TEEC_TempMemoryReference;

typedef struct {
	TEEC_SharedMemory *parent;
	size_t size;
	size_t offset;
} TEEC_RegisteredMemoryReference;

typedef struct {
	uint32_t a;
	uint32_t b;
} TEEC_Value;

typedef union {
	TEEC_TempMemoryReference tmpref;
	TEEC_RegisteredMemoryReference memref;
	TEEC_Value value;
} TEEC_Parameter;

typedef struct {
	uint32_t started;
	uint32_t paramTypes;
	union{
		TEEC_Parameter params[TEEC_CONFIG_PAYLOAD_REF_COUNT];
		TEEC_Parameter_1 params_1[TEEC_CONFIG_PAYLOAD_REF_COUNT];
		TEEC_Parameter_2 params_2[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	};

	TEEC_Session *session;

	/* 1.x */
	TEEC_SharedMemory memRefs[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	uint64_t flags;
} TEEC_Operation;

typedef enum{
	GENERAL_TO_1_0_1 = 0,
	GENERAL_FROM_1_0_1 = 1,
	GENERAL_TO_2_x = 2,
	GENERAL_FROM_2_x = 3,
	GENERAL_TO_3_x = 4,
	GENERAL_FROM_3_x = 5,
}DIRECTION;

TEEC_Result TEEC_Operation_general_to_1_x(TEEC_Operation *general, size_t general_size, TEEC_Operation_1 *data_1, size_t data_1_size, DIRECTION from);
TEEC_Result TEEC_Operation_general_to_2_x(TEEC_Operation *general, size_t general_size, TEEC_Operation_2 *data_2, size_t data_2_size, DIRECTION from);
TEEC_Result TEEC_Operation_general_to_3_x(TEEC_Operation *general, size_t general_size, TEEC_Operation_3 *data_3, size_t data_3_size, DIRECTION from);
TEEC_Result TEEC_Context_general_to_1_x(TEEC_Context *general, size_t general_size, TEEC_Context_1 *data_1, size_t data_1_size, DIRECTION from);
TEEC_Result TEEC_Context_general_to_2_x(TEEC_Context *general, size_t general_size, TEEC_Context_2 *data_2, size_t data_2_size, DIRECTION from);
TEEC_Result TEEC_Context_general_to_3_x(TEEC_Context *general, size_t general_size, TEEC_Context_3 *data_3, size_t data_3_size, DIRECTION from);
TEEC_Result TEEC_Session_general_to_1_x(TEEC_Session *general, size_t general_size, TEEC_Session_1 *data_1, size_t data_1_size, DIRECTION from);
TEEC_Result TEEC_Session_general_to_2_x(TEEC_Session *general, size_t general_size, TEEC_Session_2 *data_2, size_t data_2_size, DIRECTION from);
TEEC_Result TEEC_Session_general_to_3_x(TEEC_Session *general, size_t general_size, TEEC_Session_3 *data_3, size_t data_3_size, DIRECTION from);

#define TEEC_Operation(_ver, general, data, from) TEEC_Operation_general_to_##_ver(general, sizeof(*general), data, sizeof(*data), from);
#define TEEC_Context(_ver, general, data, from) TEEC_Context_general_to_##_ver(general, sizeof(*general), data, sizeof(*data), from);
#define TEEC_Session(_ver, general, data, from) TEEC_Session_general_to_##_ver(general, sizeof(*general), data, sizeof(*data), from);

#define TEEC_Session_INIT { .fd = -1, .session_id = -1, .ctx = {.fd = -1}}
#define TEEC_Context_INIT { .fd = -1, .devname = ""}
#define TEEC_Operation_INIT { .started = 0, .paramTypes = 0, .session = NULL, .flags = 0}

#endif
