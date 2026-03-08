#
# Copyright 2019 - 2021 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
UFBL_CFLAGS += -DUFBL_FEATURE_UNLOCK


ifeq (true,$(strip $(FEATURE_MBEDTLS)))
UFBL_CFLAGS += -DUFBL_FEATURE_UNLOCK_MBEDTLS
UFBL_SOURCE_FILES += \
	features/unlock/unlock_common.c \
	features/unlock/unlock_mbedtls.c
else
ifeq (true,$(strip $(FEATURE_UNLOCK)))
UFBL_CFLAGS += -DUFBL_FEATURE_SECURE_BOOT_OPENSSL
UFBL_SOURCE_FILES += \
	features/unlock/unlock_openssl.c
endif
endif

UFBL_SOURCE_FILES += \
	features/unlock/unlock.c
