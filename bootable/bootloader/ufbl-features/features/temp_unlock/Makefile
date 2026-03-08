#
# Common list of objects files we want to include
#
# Common set of CFLAGS we need
TEMP_UNLOCK_CFLAGS = \
    -DSUPPORT_UBOOT \
    -std=c99 \
    -ffunction-sections -fdata-sections \
    -DUFBL_FEATURE_TEMP_UNLOCK

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
TEMP_UNLOCK_OBJECTS += \
    temp_unlock.o \
    temp_unlock_ltc.o

TEMP_UNLOCK_CFLAGS += \
    -DUFBL_FEATURE_TEMP_UNLOCK_LTC \
    -DLTC_SOURCE \
    -DLTC_NO_FILE \
    -DUSE_LTM -DLTM_DESC \
    -D_WCHAR_T -DLTC_CLEAN_STACK

TEMP_UNLOCK_HEADERS = \
    -I$(UFBL_PATH)/features/libtomcrypt/ \
    -I$(UFBL_PATH)/features/libtomcrypt/src/headers
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
TEMP_UNLOCK_OBJECTS += \
    temp_unlock_common.o

TEMP_UNLOCK_CFLAGS += \
    -DUFBL_FEATURE_TEMP_UNLOCK_MBEDTLS \
    -DMBEDTLS_CONFIG_FILE='<config-ufbl.h>'

TEMP_UNLOCK_HEADERS = \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/configs \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/include
endif

# Begin rules for kbuild U-Boot
ifeq (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))

ccflags-y    += $(TEMP_UNLOCK_CFLAGS) $(TEMP_UNLOCK_HEADERS)
obj-y        += $(TEMP_UNLOCK_OBJECTS)

else # default rules

include $(TOPDIR)/config.mk

TEMP_UNLOCK_LIB = $(obj)libtemp_unlock.o

# Default CFLAGS
CFLAGS += \
    $(TEMP_UNLOCK_CFLAGS) \
    $(TEMP_UNLOCK_HEADERS)

COBJS := $(TEMP_UNLOCK_OBJECTS)
SRCS  := $(COBJS:.o=.c)
OBJS  := $(addprefix $(obj),$(COBJS))

$(TEMP_UNLOCK_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif # (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))
