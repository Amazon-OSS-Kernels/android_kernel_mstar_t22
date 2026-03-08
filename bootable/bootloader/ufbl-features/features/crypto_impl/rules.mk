#
# Copyright (c) 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/../include -I$(LOCAL_DIR)

OBJS += \
    $(LOCAL_DIR)/amzn_common.o

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
OBJS += \
    $(LOCAL_DIR)/amzn_ltc.o
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
OBJS += \
    $(LOCAL_DIR)/amzn_mbedtls.o
endif

ifeq (true,$(strip $(FEATURE_COMMON_OPENSSL)))
OBJS += \
    $(LOCAL_DIR)/amzn_openssl.o
endif
