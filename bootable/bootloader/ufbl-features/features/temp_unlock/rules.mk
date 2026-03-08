LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/../include -I$(LOCAL_DIR)

# Define temp_unlock
CFLAGS += -DUFBL_FEATURE_TEMP_UNLOCK

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
CFLAGS += -DUFBL_FEATURE_TEMP_UNLOCK_LTC
OBJS += \
    $(LOCAL_DIR)/temp_unlock.o \
    $(LOCAL_DIR)/temp_unlock_ltc.o
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
CFLAGS += -DUFBL_FEATURE_TEMP_UNLOCK_MBEDTLS
OBJS += \
    $(LOCAL_DIR)/temp_unlock_common.o
endif
