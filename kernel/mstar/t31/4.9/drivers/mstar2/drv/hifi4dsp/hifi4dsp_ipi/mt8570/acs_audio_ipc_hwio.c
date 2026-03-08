/*
 * acs_audio_ipc_hwio.c
 *
 * acs audio ipc driver for Mstar T31
 *
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 * Shuishun Zhou        (shuishuz@amazon.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "adsp_ipi.h"
#include "hifi4dsp_spi.h"
#include "acs_audio_ipc_hwio.h"

typedef struct {
    aceAudIPCHW_callback_t cb;
    void *user;
} aceAudIPCHW_cbdata_t;
static aceAudIPCHW_cbdata_t s_cb;

static void ipchw_callback(int id, void *data, unsigned int len)
{
    void *user = s_cb.user;
    aceAudIPCHW_callback_t cb = s_cb.cb;

    //pr_info("%s(%d, %p, %d)\n", __func__, id, data, len);
    if (ADSP_IPI_DSPA2AP != id) {
        pr_err("%s: invalid input\n", __func__);
        return;
    }

    if (cb) {
        cb(ACE_AUD_IPC_AUDMGR, data, len, user);
    }
}

static int32_t ipchw_init(void *args)
{
    pr_info("%s(%p)\n", __func__, args);
    memset(&s_cb, 0, sizeof(s_cb));
    return adsp_ipi_registration(ADSP_IPI_DSPA2AP, ipchw_callback, "IPI_DSPA2AP");
}

static void ipchw_deinit(void)
{
    pr_info("%s\n", __func__);
    memset(&s_cb, 0, sizeof(s_cb));
}

static int32_t ipchw_get_memory(uint32_t align, uint32_t size, aceAudIPCHW_buf_t *buf)
{
    static aceAudIPCHW_buf_t sbuf = {NULL, 0};

    pr_info("%s(%d, %d, %p)\n", __func__, align, size, buf);
    if ((!buf) || (size < align) || (align & 1)) {
        pr_err("%s: invalid input\n", __func__);
        return ADSP_IPI_ERROR;
    }

    if (NULL != sbuf.addr) {
        if ((sbuf.size == size) && (((uint32_t)sbuf.addr & (align - 1)) == 0)) {
            *buf = sbuf;
            pr_info("%s: reuse buffer (%p, %d)\n", __func__, buf->addr, buf->size);
            return ADSP_IPI_DONE;
        }
        kfree(sbuf.addr);
        sbuf.addr = NULL;
        sbuf.size = 0;
    }

    buf->addr = kmalloc(size, GFP_KERNEL);
    if (NULL == buf->addr) {
        pr_err("%s: no more memory\n", __func__);
        return ADSP_IPI_ERROR;
    }

    buf->size = size;
    if (((uint32_t)buf->addr & (align - 1)) != 0) {
        pr_err("%s: memory address [%p] not align\n", __func__, buf->addr);
        kfree(buf->addr);
        return ADSP_IPI_ERROR;
    }

    sbuf = *buf;
    pr_info("%s: get buffer (%p, %d)\n", __func__, buf->addr, buf->size);
    return 0;
}

static int32_t ipchw_msg_register(aceAudIPCHW_callback_t cb, void *user)
{
    s_cb.cb = cb;
    s_cb.user = user;
    pr_info("%s(%p, %p)\n", __func__, cb, user);
    return 0;
}

static int32_t ipchw_msg_post(aceAudIPC_user_t dst, void *msg, uint32_t size)
{
    //pr_info("%s(%d, %p, %d)\n", __func__, dst, msg, size);
    if (ACE_AUD_IPC_AUDMGR == dst) {
        return adsp_ipi_send(ADSP_IPI_AP2DSPA, msg, size, 1, 0);
    } else {
        pr_err("%s : unsupport\n");
        return ADSP_IPI_ERROR;
    }
}

static int32_t ipchw_dat_transfer(aceAudIPCHW_transfer_block_t *blocks, uint32_t number)
{
    uint32_t i;

    if ((NULL == blocks) || (0 == number)) {
        return ADSP_IPI_ERROR;
    }

    for (i = 0; i < number; i++) {
        //pr_info("%s [%d](%d, %d, %d)\n", __func__, i, blocks->src, blocks->dst, blocks->size);
        if ((ACE_AUD_IPC_CLIENT == blocks->src) && (ACE_AUD_IPC_AUDMGR == blocks->dst)) {
            dsp_spi_write(blocks->dst_addr.data[0], blocks->src_addr.ptr, blocks->size, SPI_SPEED_HIGH);
        } else if ((ACE_AUD_IPC_CLIENT == blocks->dst) && (ACE_AUD_IPC_AUDMGR == blocks->src)) {
            dsp_spi_read(blocks->src_addr.data[0], blocks->dst_addr.ptr, blocks->size, SPI_SPEED_HIGH);
        } else if ((ACE_AUD_IPC_CLIENT == blocks->src) && (ACE_AUD_IPC_CLIENT == blocks->dst) && (blocks->size <= sizeof(blocks->src_addr))) {
            // It's special case, the source data already in this transfer block message, don't need hw transfer
            memcpy(blocks->dst_addr.ptr, blocks->src_addr.data, blocks->size);
        } else {
            // don't support this data transfer
            pr_err("%s [%d](%d, %d, %d) unsupport\n", __func__, i, blocks->src, blocks->dst, blocks->size);
            return ADSP_IPI_ERROR;
        }
        blocks++;
    }

    return ADSP_IPI_DONE;
}

static int32_t ipchw_dat_transfer_dummy(aceAudIPCHW_transfer_block_t *blocks, uint32_t number)
{
    pr_err("%s unsupport feature\n", __func__);
    return ADSP_IPI_ERROR;
}

const aceAudioIPCHW_imp_t s_imp_tab[] = {
    {
        ACE_AUDIO_IPC_HW_IO,
        ipchw_init,
        ipchw_deinit,
        ipchw_get_memory,
        ipchw_msg_register,
        ipchw_msg_post,
        ipchw_dat_transfer
    },
    {
        ACE_AUDIO_IPC_SW_SSHM,
        ipchw_init,
        ipchw_deinit,
        ipchw_get_memory,
        ipchw_msg_register,
        ipchw_msg_post,
        ipchw_dat_transfer_dummy,
    }
};

const aceAudioIPCHW_imp_t* aceAudioIPCHW_getImp(aceAudioIPC_type_t type, void *args)
{
    uint32_t i;

    pr_info("%s(%d, %p)\n", __func__, type, args);
    for (i = 0; i < (sizeof(s_imp_tab)/sizeof(*s_imp_tab)); i++) {
        if (type == s_imp_tab[i].type) {
            pr_info("%s: get [%d @ %d]\n", __func__, type, i);
            return &s_imp_tab[i];
        }
    }
    pr_err("%s: don't support this type [%d]\n", __func__, type);
    return NULL;
}

EXPORT_SYMBOL_GPL(aceAudioIPCHW_getImp);
