/*
 * Copyright (C) 2016 - 2021 Amazon.com Inc. or its affiliates.  All Rights Reserved.
 */
#include <stdio.h>

#include <stdint.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/sha256.h>

#ifndef SUPPORT_UBOOT
#include <debug.h>
#endif

#include "amzn_unlock.h"
#include "amzn_unlock_internal.h"
#include "ufbl_debug.h" /* for dprintf */

int amzn_verify_code_internal(const unsigned char *data, unsigned int data_len,
	const unsigned char *sig, unsigned int sig_len,
	const unsigned char *key, unsigned int key_len)
{
	int ret = -ERR_UNLOCK_RSA_VERIFY_FAIL;
	uint8_t digest[32];
	mbedtls_pk_context ctx;

	if (!data || !data_len || !sig || sig_len < 256 || !key || !key_len) {
		dprintf(CRITICAL, "%s: Invalid arg\n", __func__);
		return -ERR_UNLOCK_BAD_ARGUMENT;
	}

	mbedtls_pk_init(&ctx);

	/* Parse the unlock key */
	ret = mbedtls_pk_parse_public_key(&ctx, key, key_len);
	if (ret) {
		dprintf(CRITICAL, "%s: Failed to parse public key (%d)\n", __func__, ret);
		goto free_ctx;
	}

	if (!mbedtls_pk_can_do(&ctx, MBEDTLS_PK_RSASSA_PSS)) {
		ret = -ERR_UNLOCK_BAD_KEY;
		dprintf(CRITICAL, "%s: Failed to recognize RSA key\n", __func__);
		goto free_ctx;
	}

	ret = mbedtls_sha256_ret(data, data_len, digest, 0);
	if (ret) {
		dprintf(CRITICAL, "%s: Failed to hash data (%d)\n", __func__, ret);
		goto free_ctx;
	}

	/* PSS-Verify */
	ret = mbedtls_rsa_rsassa_pss_verify(mbedtls_pk_rsa(ctx), NULL, NULL,
			MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, sizeof(digest), digest, sig);
	if (ret) {
		dprintf(CRITICAL, "%s: Failed to verify signature (%d)\n", __func__, ret);
		ret = -ERR_UNLOCK_RSA_VERIFY_FAIL;
	} else {
		ret = ERR_UNLOCK_SUCCESS;
	}

free_ctx:
	mbedtls_pk_free(&ctx);

	return ret;
}
