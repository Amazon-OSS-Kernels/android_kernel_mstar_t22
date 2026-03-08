# Copyright (c) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved

#
# Common list of objects files we want to include
#
CRYPTO_IMPL_OBJECTS += \
    amzn_common.o

CRYPTO_IMPL_HEADERS += \
    -I$(UFBL_PATH)/include

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
CRYPTO_IMPL_OBJECTS += \
    amzn_ltc.o

CRYPTO_IMPL_CFLAGS += \
    -DLTC_SOURCE -DLTC_NO_FILE \
    -DUSE_LTM -DLTM_DESC \
    -D_WCHAR_T -DLTC_CLEAN_STACK

CRYPTO_IMPL_HEADERS = \
    -I$(UFBL_PATH)/features/libtomcrypt/ \
    -I$(UFBL_PATH)/features/libtomcrypt/src/headers
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
CRYPTO_IMPL_OBJECTS += \
    amzn_mbedtls.o

CRYPTO_IMPL_CFLAGS += \
    -D__KERNEL_STRICT_NAMES \
    -DMBEDTLS_CONFIG_FILE='<config-ufbl.h>'

CRYPTO_IMPL_HEADERS = \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/configs \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/include
endif

ifeq (true,$(strip $(FEATURE_COMMON_OPENSSL)))
CRYPTO_IMPL_OBJECTS += \
    amzn_openssl.o
endif

# Begin rules for kbuild U-Boot
ifeq (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))

ccflags-y += $(CRYPTO_IMPL_CFLAGS) $(CRYPTO_IMPL_HEADERS)
obj-y += $(CRYPTO_IMPL_OBJECTS)

else # default rules

include $(TOPDIR)/config.mk

CRYPTO_IMPL_LIB = $(obj)libcrypto_impl.o

CFLAGS += \
    $(CRYPTO_IMPL_CFLAGS) \
    $(CRYPTO_IMPL_HEADERS)

COBJS  := $(CRYPTO_IMPL_OBJECTS)
SRCS   := $(COBJS:.o=.c)
OBJS   := $(addprefix $(obj),$(COBJS))

$(CRYPTO_IMPL_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif # (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))
