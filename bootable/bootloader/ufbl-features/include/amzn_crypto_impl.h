/*
 * Copyright (C) 2021 Amazon.com Inc. or its affiliates.  All Rights Reserved.
 */
#ifndef AMZN_CRYPTO_IMPL_H
#define AMZN_CRYPTO_IMPL_H

#include <stddef.h>

int amzn_crypto_impl_sha256(const unsigned char *input, size_t ilen,
	unsigned char output[32]);

int amzn_crypto_impl_b64_encode(const unsigned char *in, size_t len,
	unsigned char *out, size_t *outlen);

int amzn_crypto_impl_b64_decode(const unsigned char *b64_str, size_t len_in,
	unsigned char *b64_dec, size_t *len_out);
#endif /* AMZN_CRYPTO_IMPL_H */
