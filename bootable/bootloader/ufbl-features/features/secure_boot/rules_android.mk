#
# Copyright 2019 - 2021 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
UFBL_CFLAGS += -DUFBL_FEATURE_SECURE_BOOT

ifeq (true,$(strip $(FEATURE_SECURE_BOOT_OPENSSL)))
UFBL_CFLAGS += -DUFBL_FEATURE_SECURE_BOOT_OPENSSL
UFBL_SOURCE_FILES += \
	features/secure_boot/image_verify_openssl.c
endif

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
UFBL_DEFINES += -DUFBL_FEATURE_SECURE_BOOT_LTC
UFBL_SOURCE_FILES += \
	features/secure_boot/image_verify_ltc.c
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
UFBL_CFLAGS += -DUFBL_FEATURE_SECURE_BOOT_MBEDTLS
UFBL_SOURCE_FILES += \
	features/secure_boot/image_verify_mbedtls.c
endif

UFBL_SOURCE_FILES += \
	features/secure_boot/image_verify.c
