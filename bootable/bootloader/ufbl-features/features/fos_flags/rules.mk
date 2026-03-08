LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/../include -I$(LOCAL_DIR)

CFLAGS += -DUFBL_FEATURE_FOS_FLAGS

OBJS += \
    $(LOCAL_DIR)/fastboot_fos_flags.o \
    $(LOCAL_DIR)/fos_flags.o
