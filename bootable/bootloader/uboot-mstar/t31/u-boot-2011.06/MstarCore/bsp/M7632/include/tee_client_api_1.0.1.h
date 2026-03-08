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
/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef TEE_CLIENT_API_1_0_1_H
#define TEE_CLIENT_API_1_0_1_H

#ifndef __KERNEL__
#include <stdint.h>
#include <stddef.h>
#endif /* __KERNEL__ */

#include <tee_client_common.h>

#define TEEC_MAX_DEVNAME_SIZE 256

/**
 * struct TEEC_Context_1 - Represents a connection between a client application
 * and a TEE.
 *
 * Context identifier can be a handle (when opened from user land)
 * or a structure pointer (when opened from kernel land).
 * Identifier is defined as an union to match type sizes on all architectures.
 */
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
} TEEC_Context_1;

/**
 * This type contains a Universally Unique Resource Identifier (UUID) type as
 * defined in RFC4122. These UUID values are used to identify Trusted
 * Applications.
 */
typedef struct {
	uint32_t timeLow;
	uint16_t timeMid;
	uint16_t timeHiAndVersion;
	uint8_t clockSeqAndNode[8];
} TEEC_UUID;

/**
 * In terms of compatible kernel, the data struct shared by client application
 * and TEE driver should be restructrued in "compatible" rules. To keep GP's
 * standard in compatibility mode, the anonymous padding members are filled
 * in the struct definition below.
 */


/**
 * struct TEEC_SharedMemory_1 - Memory to transfer data between a client
 * application and trusted code.
 *
 * @param buffer      The memory buffer which is to be, or has been, shared
 *                    with the TEE.
 * @param size        The size, in bytes, of the memory buffer.
 * @param flags       Bit-vector which holds properties of buffer.
 *                    The bit-vector can contain either or both of the
 *                    TEEC_MEM_INPUT and TEEC_MEM_OUTPUT flags.
 *
 * A shared memory block is a region of memory allocated in the context of the
 * client application memory space that can be used to transfer data between
 * that client application and a trusted application. The user of this struct
 * is responsible to populate the buffer pointer.
 */
typedef struct {
	union {
		void *buffer;
		uint64_t padding_ptr;
	};
	union {
		size_t size;
		uint64_t padding_sz;
	};
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
		uint64_t padding_d;
	} d;
	uint64_t registered;
} TEEC_SharedMemory_1;

/**
 * struct TEEC_TempMemoryReference_1 - Temporary memory to transfer data between
 * a client application and trusted code, only used for the duration of the
 * operation.
 *
 * @param buffer  The memory buffer which is to be, or has been shared with
 *                the TEE.
 * @param size    The size, in bytes, of the memory buffer.
 *
 * A memory buffer that is registered temporarily for the duration of the
 * operation to be called.
 */
typedef struct {
	union {
		void *buffer;
		uint64_t padding_ptr;
	};
	union {
		size_t size;
		uint64_t padding_sz;
	};
} TEEC_TempMemoryReference_1;

/**
 * struct TEEC_RegisteredMemoryReference_1 - use a pre-registered or
 * pre-allocated shared memory block of memory to transfer data between
 * a client application and trusted code.
 *
 * @param parent  Points to a shared memory structure. The memory reference
 *                may utilize the whole shared memory or only a part of it.
 *                Must not be NULL
 *
 * @param size    The size, in bytes, of the memory buffer.
 *
 * @param offset  The offset, in bytes, of the referenced memory region from
 *                the start of the shared memory block.
 *
 */
typedef struct {
	union {
		TEEC_SharedMemory_1 *parent;
		uint64_t padding_ptr;
	};
	union {
		size_t size;
		uint64_t padding_sz;
	};
	union {
		size_t offset;
		uint64_t padding_off;
	};
} TEEC_RegisteredMemoryReference_1;

/**
 * struct TEEC_Value_1 - Small raw data container
 *
 * Instead of allocating a shared memory buffer this structure can be used
 * to pass small raw data between a client application and trusted code.
 *
 * @param a  The first integer value.
 *
 * @param b  The second second value.
 */
typedef struct {
	uint32_t a;
	uint32_t b;
} TEEC_Value_1;

/**
 * union TEEC_Parameter_1 - Memory container to be used when passing data between
 *                        client application and trusted code.
 *
 * Either the client uses a shared memory reference, parts of it or a small raw
 * data container.
 *
 * @param tmpref  A temporary memory reference only valid for the duration
 *                of the operation.
 *
 * @param memref  The entire shared memory or parts of it.
 *
 * @param value   The small raw data container to use
 */
typedef union {
	TEEC_TempMemoryReference_1 tmpref;
	TEEC_RegisteredMemoryReference_1 memref;
	TEEC_Value_1 value;
} TEEC_Parameter_1;

/**
 * struct TEEC_Session_1 - Represents a connection between a client application
 * and a trusted application.
 *
 * Session identifier can be a handle (when opened from user land) or a
 * structure pointer (when opened from kernel land).
 * Identifier is defined as an union to match type sizes on all architectures.
 */
typedef union {
#ifdef CONFIG_ARM64
	uintptr_t fd;
#else
	int fd;
#endif
	struct tee_session *session;
} TEEC_Session_1;

/**
 * struct TEEC_Operation_1 - Holds information and memory references used in
 * TEEC_InvokeCommand_1().
 *
 * @param   started     Client must initialize to zero if it needs to cancel
 *                      an operation about to be performed.
 * @param   paramTypes  Type of data passed. Use TEEC_PARAMS_TYPE macro to
 *                      create the correct flags.
 *                      0 means TEEC_NONE is passed for all params.
 * @param   params      Array of parameters of type TEEC_Parameter_1.
 * @param   session     Internal pointer to the last session used by
 *                      TEEC_InvokeCommand_1 with this operation.
 *
 */
typedef struct {
	uint32_t started;
	uint32_t paramTypes;
	TEEC_Parameter_1 params[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	/* Implementation-Defined */
	union {
		TEEC_Session_1 *session;
		uint64_t padding_ptr;
	};
	TEEC_SharedMemory_1 memRefs[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	uint64_t flags;
} TEEC_Operation_1;

#endif
