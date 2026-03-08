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
MAKEK := make SHELL=/bin/bash -j8
KERNEL_ABS_DIR := $(ROOTDIR)/$(KERNEL_DIR)
KERNEL_DEFCONFIG_FILE := $(KERNEL_ABS_DIR)/arch/$(TARGET_KERNEL_ARCH)/configs/$(KERNEL_CONFIG)

KERNEL_MODULES_OUT :=  $(PRODUCT_OUT)/vendor/lib/modules
TARGET_KERNEL_CONFIG := $(KERNEL_OUT_ABS)/.config
TARGET_KERNEL_HAS_MODULE := $(shell grep -q "CONFIG_MODULES=y" $(KERNEL_DEFCONFIG_FILE) && echo true)

export KERNEL_PATH:=$(KERNEL_ABS_DIR)
export KERNEL_CONFIG_FILENAME:=$(KERNEL_CONFIG)

INSTALLED_DTIMAGE_TARGET := $(PRODUCT_OUT)/dtb.bin
ifeq ($(BUILD_DTO),true)
ifeq ("$(TARGET_AMMO_SUPPORT)","true")
DTBOIMAGE_TARGET :=
$(foreach v, $(TARGET_AMMO_VAR_LIST), \
	$(eval TARGET_DTBOIMAGE_$(v) := $(KERNEL_OUT_ABS)/arch/$(TARGET_KERNEL_ARCH)/boot/dts/$(KERNEL_DTBO_NAME_$(v))) \
	$(eval DTBOIMAGE_TARGET_$(v) := $(PRODUCT_OUT)/$(v)/overlay.dtbo))
else
TARGET_DTBOIMAGE := $(KERNEL_OUT_ABS)/arch/$(TARGET_KERNEL_ARCH)/boot/dts/$(KERNEL_DTBO_NAME)
DTBOIMAGE_TARGET := $(PRODUCT_OUT)/overlay.dtbo
endif
endif
$(info **  TARGET_OUT_INTERMEDIATES $(TARGET_OUT_INTERMEDIATES)++)
$(info **  0 $(PRODUCT_OUT)++)
$(info **  1 $(KERNEL_DEFCONFIG) $(KERNEL_PATH)++)
$(info ++  2 $(TARGET_KERNEL_ARCH) $(ROOTDIR)++)
$(info --  3 $(KERNEL_ABS_DIR) $(KERNEL_OUT)--)
$(info ++  4 $(KERNEL_OUT_ABS) $(TARGET_PREBUILT_KERNEL)--)
$(info --  5 $(TARGET_DTIMAGE) $(KERNEL_DEFCONFIG_FILE)--)
$(info --  6 $(TARGET_KERNEL_HAS_MODULE) $(TARGET_KERNEL_CONFIG)--)
$(info --  7 $(KERNEL_CROSS_COMPILE)--)
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

KERNEL_MAKEFLAGS := -C $(KERNEL_ABS_DIR) O=$(KERNEL_OUT_ABS) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) REGDB_SUFFIX=$(REGDB_SUFFIX) CONFIG_DEBUG_SECTION_MISMATCH=y TARGET_KERNEL_STRIP_CMD=$(TARGET_KERNEL_STRIP_CMD)
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
	$(hide) cp $(BOARD_AMAZON_SYSTEM_VERITY_PUBK) $(KERNEL_DIR)/certs/amazon_verity.x509.pem
	echo "Make kernel-defconfig..."
	$(MAKEK) -p $(KERNEL_MAKEFLAGS) $(KERNEL_DEFCONFIG)


$(TARGET_PREBUILT_KERNEL): $(TARGET_KERNEL_CONFIG) | $(KERNEL_OUT)
	echo "Building kernel..."
	$(hide) $(MAKEK) $(KERNEL_MAKEFLAGS) $(KERNEL_EXTRA_BUILD_OPTIONS)
ifeq ($(BUILD_DTO),true)
	echo "Build Kernel: Copy overlay to out"
ifeq ($(TARGET_AMMO_SUPPORT),true)
	$(foreach v, $(TARGET_AMMO_VAR_LIST), \
		mkdir -p $(dir $(DTBOIMAGE_TARGET_$(v))); \
		$(ACP) $(TARGET_DTBOIMAGE_$(v)) $(DTBOIMAGE_TARGET_$(v));)
else
	$(ACP) $(TARGET_DTBOIMAGE) $(DTBOIMAGE_TARGET)
endif
endif
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
	$(ACP) "$<" $(PRODUCT_OUT)/unsigned/dtb.unsigned
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
