UFBL_CFLAGS += -DUFBL_FEATURE_TEMP_UNLOCK

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT))
UFBL_CFLAGS += -DUFBL_FEATURE_TEMP_UNLOCK_LTC
UFBL_SOURCE_FILES += \
    features/temp_unlock/temp_unlock_ltc.c \
    features/temp_unlock/temp_unlock.c
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
UFBL_CFLAGS += -DUFBL_FEATURE_TEMP_UNLOCK_MBEDTLS
UFBL_SOURCE_FILES += \
    features/temp_unlock/temp_unlock_common.c
endif
