/*
 * Copyright (c) 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include <tomcrypt.h>
#include "amzn_crypto_impl.h"

int amzn_crypto_impl_sha256(const unsigned char *input, size_t ilen,
	unsigned char output[32])
{
	unsigned long digest_len = sha256_desc.hashsize;
	int sha256_idx = find_hash("sha256");

	return hash_memory(sha256_idx, input, ilen, output, &digest_len);
}

/**
 * Helper function for base64 encode.
 */
int amzn_crypto_impl_b64_encode(const unsigned char *in, size_t len,
	unsigned char *out, size_t *outlen)
{
	return base64_encode(in, len, out, outlen);
}

/**
 * Helper function for base64 decode.
 */
int amzn_crypto_impl_b64_decode(const unsigned char *b64_str, size_t len_in,
	unsigned char *b64_dec, size_t *len_out)
{
	return base64_decode(b64_str, len_in, b64_dec, len_out);
}
