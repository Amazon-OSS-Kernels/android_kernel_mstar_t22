/*
 * Copyright (c) 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include "amzn_crypto_impl.h"

#define __WEAK __attribute__((weak))

__WEAK int amzn_crypto_impl_sha256(const unsigned char *input, size_t ilen,
	unsigned char output[32])
{
	return -1;
}

/**
 * Helper function for base64 encode.
 */
__WEAK int amzn_crypto_impl_b64_encode(const unsigned char *in, size_t len,
	unsigned char *out, size_t *outlen)
{
	return -1;
}

/**
 * Helper function for base64 decode.
 */
__WEAK int amzn_crypto_impl_b64_decode(const unsigned char *b64_str, size_t len_in,
	unsigned char *b64_dec, size_t *len_out)
{
	return -1;
}
