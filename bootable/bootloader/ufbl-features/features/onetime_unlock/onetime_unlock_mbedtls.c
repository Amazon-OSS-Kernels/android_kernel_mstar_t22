/*
 * Copyright (c) 2015 - 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include <stdio.h>
#include <string.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#include "amzn_onetime_unlock.h"
#ifndef SUPPORT_UBOOT
#include <debug.h>
#endif
#include "ufbl_debug.h" /* for dprintf */
#ifdef SUPPORT_BOLT
#include <platform/bcm_platform.h>
#endif

#define UNUSED(p) ((void)p)

static int entropy_dummy_source(void *data,
        unsigned char *output, size_t len, size_t *olen)
{
    UNUSED(data);

    memset(output, 0xab, len);
    *olen = len;

    return 0;
}

/**
 * Get one time random number.
 * @param entropy Entropy for Fortuna PRNG
 * @param entropy_size Size of entropy
 * @param output_buf Pointer to output random bytes buffer
 * @param output_size Size of output random number buffer
 * @return 0 - success, fail otherwise
 */
int amzn_get_onetime_random_number(const unsigned char *entropy, size_t entropy_size,
                                   unsigned char *output_buf, size_t output_size)
{
    mbedtls_entropy_context e_ctx;
    mbedtls_ctr_drbg_context ctx;
    int ret;

    if (!entropy || !entropy_size) {
        dprintf(CRITICAL, "Error Empty entropy\n");
        return -1;
    }

    if (!output_buf || !output_size) {
        dprintf(CRITICAL, "Invalid output buffer\n");
        return -1;
    }

    mbedtls_entropy_init(&e_ctx);
    mbedtls_ctr_drbg_init(&ctx);

    /**
     * mbedtls requires at least one *strong* source of entropy. However, the
     * only source of entropy we have is passed to this function as a static data.
     * Use a dummy entropy declared as *strong* (although it is weak).
     * FIXME: find a better source of entropy
     */
    ret = mbedtls_entropy_add_source(&e_ctx, &entropy_dummy_source, NULL, 16,
            MBEDTLS_ENTROPY_SOURCE_STRONG);
    if (ret) {
        dprintf(CRITICAL, "Failed to add entropy source (%d)\n", ret);
        goto free_ctx;
    }

    ret = mbedtls_ctr_drbg_seed(&ctx, mbedtls_entropy_func, &e_ctx, entropy, entropy_size);
    if (ret) {
        dprintf(CRITICAL, "Failed to initialize drbg seed (%d)\n", ret);
        goto free_ctx;
    }

    ret = mbedtls_ctr_drbg_random(&ctx, output_buf, output_size);
    if (ret)
        dprintf(CRITICAL, "Failed to randomize (%d)\n", ret);

free_ctx:
    mbedtls_ctr_drbg_free(&ctx);
    mbedtls_entropy_free(&e_ctx);

    return ret;
}
