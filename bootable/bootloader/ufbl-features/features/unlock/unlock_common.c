/*
 * Copyright (C) 2016 - 2021 Amazon.com Inc. or its affiliates.  All Rights Reserved.
 */
#include <stdio.h>
#include <stdint.h>

#ifndef SUPPORT_UBOOT
#include <debug.h>
#endif

#include "amzn_unlock.h"
#include "amzn_unlock_internal.h"
#include "ufbl_debug.h" /* for dprintf */

int amzn_verify_limited_unlock(const unsigned char *sig, unsigned int sig_len,
	unsigned int ctr_min, unsigned int ctr_max)
{
	int ret = -ERR_UNLOCK_SUCCESS;
	unsigned int ctr;
	uint8_t code[UNLOCK_CODE_LEN];
	unsigned int code_len = sizeof(code);

	unsigned int key_len = 0;
	const uint8_t *key = amzn_get_unlock_key(&key_len);
	if (!key || !key_len) {
		dprintf(CRITICAL, "%s: Failed to get unlock key\n", __FUNCTION__);
		ret = -ERR_UNLOCK_BAD_KEY;
		goto done;
	}

	for (ctr = ctr_min; ctr <= ctr_max; ctr++) {
		ret = amzn_get_limited_unlock_code(code, &code_len, ctr);
		if (ret != 0) {
			dprintf(CRITICAL, "%s: Failed to get unlock code\n",
				__FUNCTION__);
			ret = -ERR_UNLOCK_GET_CODE_FAIL;
			goto done;
		}
		ret = amzn_verify_code_internal(code, code_len,
							sig, sig_len, key, key_len);
		if (!ret)
			break;
	}
done:
	return ret;

}

int amzn_verify_unlock(const unsigned char *sig, unsigned int sig_len)
{
	int ret = ERR_UNLOCK_SUCCESS;
	uint8_t code[UNLOCK_CODE_LEN];
	unsigned int code_len = sizeof(code);

	unsigned int key_len = 0;
	const uint8_t *key = NULL;

	key = amzn_get_unlock_key(&key_len);
	if (!key || !key_len) {
		dprintf(CRITICAL, "%s: Failed to get unlock key\n", __FUNCTION__);
		ret = -ERR_UNLOCK_BAD_KEY;
		goto done;
	}
	if (amzn_get_unlock_code(code, &code_len)) {
		dprintf(CRITICAL, "%s: Failed to get unlock code\n", __FUNCTION__);
		ret = -ERR_UNLOCK_GET_CODE_FAIL;
		goto done;
	}

	ret = amzn_verify_code_internal(code, code_len,
						sig, sig_len, key, key_len);
done:
	return ret;
}
