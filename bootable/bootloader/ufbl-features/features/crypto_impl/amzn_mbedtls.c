/*
 * Copyright (c) 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include "amzn_crypto_impl.h"

int amzn_crypto_impl_sha256(const unsigned char *input, size_t ilen,
	unsigned char output[32])
{
	return mbedtls_sha256_ret(input, ilen, output, 0);
}

/**
 * Helper function for base64 encode.
 */
int amzn_crypto_impl_b64_encode(const unsigned char *in, size_t len,
	unsigned char *out, size_t *outlen)
{
	return mbedtls_base64_encode(out, *outlen, outlen, in, len);
}

/**
 * Helper function for base64 decode.
 */
int amzn_crypto_impl_b64_decode(const unsigned char *b64_str, size_t len_in,
	unsigned char *b64_dec, size_t *len_out)
{
	return mbedtls_base64_decode(b64_dec, *len_out, len_out, b64_str, len_in);
}
