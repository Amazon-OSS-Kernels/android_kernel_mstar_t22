#
# Copyright (c) 2016 - 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
#
UFBL_CFLAGS += -DUFBL_FEATURE_ONETIME_UNLOCK

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT))
UFBL_CFLAGS += -DUFBL_FEATURE_ONETIME_UNLOCK_LTC
UFBL_SOURCE_FILES += \
    features/onetime_unlock/onetime_unlock_ltc.c
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
UFBL_CFLAGS += -DUFBL_FEATURE_ONETIME_UNLOCK_MBEDTLS
UFBL_SOURCE_FILES += \
    features/onetime_unlock/onetime_unlock_common.c \
    features/onetime_unlock/onetime_unlock_mbedtls.c
endif

UFBL_SOURCE_FILES += \
    features/onetime_unlock/onetime_unlock.c
