/*
 * acs_audio_ipc_hwio.h
 *
 * acs audio ipc common header file for platforms
 *
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 * Shuishun Zhou        (shuishuz@amazon.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef ACS_AUDIO_IPC_HWIO_H
#define ACS_AUDIO_IPC_HWIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ACE_AUDIO_IPC_SW_SSHM = 0,
    ACE_AUDIO_IPC_HW_IO,
} aceAudioIPC_type_t;

/**
 * @brief ace audio ipc user type definition
 */
typedef enum {
    ACE_AUD_IPC_INVALID_USER = 0,

    ACE_AUD_IPC_CLIENT = 1,      /**< It's client 1 user (MCU - default) */
    ACE_AUD_IPC_CLIENT_2,        /**< It's client 2 user (MCU 2) */
    ACE_AUD_IPC_CLIENT_3,        /**< It's client 3 user (MCU 3) */
    ACE_AUD_IPC_CLIENT_4,        /**< It's client 4 user (MCU 4) */
    ACE_AUD_IPC_CLIENT_MAX = 32, /**< Support upto 32 audio client user*/

    ACE_AUD_IPC_AUDMGR,   /**< It's AM user (MCU or DSP 1 - default) */
    ACE_AUD_IPC_AUDMGR_2, /**< It's AM 2 user (DSP 2) */
    ACE_AUD_IPC_AUDMGR_3, /**< It's AM 3 user (DSP 3) */
    ACE_AUD_IPC_AUDMGR_4, /**< It's AM 4 user (DSP 4) */
} aceAudIPC_user_t;

typedef struct {
    union {
        void *ptr;
        uint64_t addr;
        uint32_t data[2];
    };
} aceAudIPCHW_addr_t;

typedef struct {
    void *addr;
	uint32_t size;
} aceAudIPCHW_buf_t;

typedef struct {
    aceAudIPCHW_addr_t src_addr;
    aceAudIPCHW_addr_t dst_addr;
    uint32_t size;
    uint16_t src;
    uint16_t dst;
} aceAudIPCHW_transfer_block_t;

typedef void (*aceAudIPCHW_callback_t)(aceAudIPC_user_t src, void *msg, uint32_t size, void *user);

typedef struct {
    uint32_t type;
	int32_t (*init)(void *args);
	void    (*deinit)(void);
    int32_t (*get_memory)(uint32_t align, uint32_t size, aceAudIPCHW_buf_t *buf);
	int32_t (*msg_register)(aceAudIPCHW_callback_t cb, void *user);
	int32_t (*msg_post)(aceAudIPC_user_t dst, void *msg, uint32_t size);
	int32_t (*dat_transfer)(aceAudIPCHW_transfer_block_t *blocks, uint32_t number);
} aceAudioIPCHW_imp_t;

const aceAudioIPCHW_imp_t* aceAudioIPCHW_getImp(aceAudioIPC_type_t type, void *args);

#ifdef __cplusplus
}
#endif

#endif  /** #ifndef ACS_AUDIO_IPC_HWIO_H */
