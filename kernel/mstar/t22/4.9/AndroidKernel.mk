#
# Copyright (C) 2009-2011 The Android-x86 Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#

# This module is primarily intended for building kernels from source for
# non-Amazon devices, e.g. Nexus devices, for the purposes of major rebases
# of FireOS features. Vendors typically provide their own kernel makefiles.
# Platform integrators can adopt this makefile if desired, but it is
# not a FireOS integration requirement.

ifneq ($(KERNEL_DEFCONFIG),)

ROOTDIR := $(abspath $(TOP))
MAKEK := make SHELL=/bin/bash -j8
KERNEL_DIR ?= $(ROOTDIR)/$(KERNEL_PATH)
KERNEL_IMAGE_NAME ?= Image
KERNEL_DTB_NAME ?= m7322_an_$(TARGET_PRODUCT).dtb

KERNEL_CROSS_COMPILE := $(ROOTDIR)/prebuilts/gcc/linux-x86/aarch64/linaro-aarch64_linux-2014.09/bin/aarch64-linux-gnu-

KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
KERNEL_OUT_ABS := $(abspath $(KERNEL_OUT))
TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/$(TARGET_KERNEL_ARCH)/boot/$(KERNEL_IMAGE_NAME)
TARGET_DTIMAGE := $(KERNEL_OUT)/arch/$(TARGET_KERNEL_ARCH)/boot/dts/$(KERNEL_DTB_NAME)
KERNEL_DEFCONFIG_FILE := $(KERNEL_DIR)/arch/$(TARGET_KERNEL_ARCH)/configs/$(KERNEL_DEFCONFIG)
TARGET_KERNEL_HAS_MODULE := $(shell grep -q "CONFIG_MODULES=y" $(KERNEL_DEFCONFIG_FILE) && echo true)

TARGET_KERNEL_CONFIG := $(KERNEL_OUT)/.config
KERNEL_MODULES_OUT :=  $(PRODUCT_OUT)/vendor/lib/modules

INSTALLED_DTIMAGE_TARGET := $(PRODUCT_OUT)/dtb.bin

ifeq ($(KERNEL_CROSS_COMPILE),)
ifeq ($(TARGET_KERNEL_ARCH),arm)
KERNEL_CROSS_COMPILE := arm-eabi-
else
KERNEL_CROSS_COMPILE := $(abspath $(TARGET_TOOLS_PREFIX))
endif
endif

ifeq ($(TARGET_BUILD_VARIANT), user)
TARGET_KERNEL_STRIP_CMD:="$(KERNEL_CROSS_COMPILE)strip --strip-unneeded"
endif

KERNEL_CROSS_COMPILE := "$(ROOTDIR)/prebuilts/misc/linux-x86/ccache/ccache $(KERNEL_CROSS_COMPILE)"

$(warning "[S]KERNEL_CROSS_COMPILE is $(KERNEL_CROSS_COMPILE)")

KERNEL_MAKEFLAGS := -C $(KERNEL_DIR) O=$(KERNEL_OUT_ABS) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) REGDB_SUFFIX=$(REGDB_SUFFIX) CONFIG_DEBUG_SECTION_MISMATCH=y TARGET_KERNEL_STRIP_CMD=$(TARGET_KERNEL_STRIP_CMD)
ifneq ($(strip $(SHOW_COMMANDS)),)
KERNEL_MAKEFLAGS += V=1
endif

define mv-modules
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`;\
ko=`find $$mpath/kernel -type f -name *.ko`;\
for i in $$ko; do mv $$i $(KERNEL_MODULES_OUT)/; done;\
fi
endef

define clean-module-folder
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`; rm -rf $$mpath;\
fi
endef

$(KERNEL_OUT):
	$(hide) mkdir -p $@

$(KERNEL_MODULES_OUT):
	$(hide) mkdir -p $@

.PHONY: kernel kernel-defconfig kernel-menuconfig kernel-modules clean-kernel


#KERNEL_DEFCONFIG := olddefconfig
$(TARGET_KERNEL_CONFIG) kernel-defconfig: | $(KERNEL_OUT)
	$(shell) ($(KERNEL_DIR)/genlink.sh)
	$(MAKEK) $(KERNEL_MAKEFLAGS) $(KERNEL_DEFCONFIG)


$(TARGET_PREBUILT_KERNEL): $(TARGET_KERNEL_CONFIG) | $(KERNEL_OUT)
	$(hide) $(MAKEK) $(KERNEL_MAKEFLAGS) $(KERNEL_EXTRA_BUILD_OPTIONS)
ifeq ($(TARGET_KERNEL_HAS_MODULE),true)
	$(hide) $(MAKEK) $(KERNEL_MAKEFLAGS) modules
	$(hide) $(MAKEK) $(KERNEL_MAKEFLAGS) INSTALL_MOD_PATH=$(ROOTDIR)/$(KERNEL_MODULES_OUT) modules_install

kernel-modules: $(TARGET_PREBUILT_KERNEL) | $(KERNEL_MODULES_OUT)
	$(mv-modules)
	$(clean-module-folder)

systemimage: kernel-modules

_INSTALLED_RAMDISK_TARGET := $(PRODUCT_OUT)/ramdisk.img


endif # TARGET_KERNEL_HAS_MODULE

ifneq ($(strip $(TARGET_NO_KERNEL)),true)
INSTALLED_KERNEL_TARGET ?= $(PRODUCT_OUT)/kernel
endif

$(INSTALLED_KERNEL_TARGET): $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(copy-file-to-target)

$(TARGET_DTIMAGE): $(TARGET_PREBUILT_KERNEL)

$(INSTALLED_DTIMAGE_TARGET): $(TARGET_DTIMAGE) | $(ACP)
ifdef TARGET_PREBUILT_DTB
	@mkdir -p $(PRODUCT_OUT)
	@mkdir -p $(PRODUCT_OUT)/unsigned
	$(ACP) "$<" $(PRODUCT_OUT)/unsigned/dtb.bin.unsigned
	$(ACP) $(ROOTDIR)/$(TARGET_PREBUILT_DTB) $(PRODUCT_OUT)/dtb.bin
else
	$(copy-file-to-target)
endif

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_DTIMAGE_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_DTIMAGE_TARGET)

droidcore: $(INSTALLED_DTIMAGE_TARGET)

clean-kernel:
	$(hide) rm -rf $(KERNEL_OUT)
	$(hide) rm -rf $(KERNEL_MODULES_OUT)

endif # KERNEL_DEFCONFIG
