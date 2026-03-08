#
# Copyright (c) 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
#
UFBL_SOURCE_FILES += \
    features/crypto_impl/amzn_common.c

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT))
UFBL_SOURCE_FILES += \
    features/crypto_impl/amzn_ltc.c
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
UFBL_SOURCE_FILES += \
    features/crypto_impl/amzn_mbedtls.c
endif

ifeq (true,$(strip $(FEATURE_COMMON_OPENSSL)))
UFBL_SOURCE_FILES += \
    features/crypto_impl/amzn_openssl.o
endif
