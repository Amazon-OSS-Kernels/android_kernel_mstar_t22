/*
 * Copyright (c) 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include <openssl/sha.h>
#include "amzn_crypto_impl.h"

int amzn_crypto_impl_sha256(const unsigned char *input, size_t ilen,
	unsigned char output[32])
{
	/* Using a default software implementation for sha256 */
	SHA256(data, n, digest);

	return 0;
}
