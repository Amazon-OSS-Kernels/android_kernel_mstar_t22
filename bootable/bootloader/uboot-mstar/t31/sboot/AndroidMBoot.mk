LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

INSTALLED_MBOOT_TARGET := $(PRODUCT_OUT)/mboot.bin

MBOOT_OUT := $(abspath $(PRODUCT_OUT))/obj/MBOOT_OBJ
TARGET_MBOOT_CONFIG := $(MBOOT_OUT)/.config
TARGET_BUILT_MBOOT := $(MBOOT_OUT)/out/mboot.bin

MAKE_SBOOT = make SHELL=/bin/bash -j1

MBOOT_CC := $(ROOTDIR)/prebuilts/gcc/linux-x86/arm/gcc-4.5.2-arm-eabi-2011.03_x86_64/bin/arm-none-eabi-gcc
ARCH := arm
MBOOT_CROSS_COMPILE := $(ROOTDIR)/prebuilts/gcc/linux-x86/arm/gcc-4.5.2-arm-eabi-2011.03_x86_64/bin/arm-none-eabi-

export BOARD_UFBL_PROJECT BOARD_KERNEL_SEPARATED_DT TARGET_PREBUILT_DTB TARGET_PREBUILT_DTBO

# Set common UFBL project
UFBL_PROJECT := mtk_t31

$(TARGET_MBOOT_CONFIG): $(MBOOT_DIR)/$(MBOOT_DEFCONFIG) | $(ACP)
	@echo "Building oldconfig for mboot"
	mkdir -p $(MBOOT_OUT)
	$(ACP) $(MBOOT_DIR)/$(MBOOT_DEFCONFIG) $@
	yes "" | $(MAKE_SBOOT) -C $(MBOOT_DIR) O=$(MBOOT_OUT) oldconfig

$(TARGET_BUILT_MBOOT): | $(ACP)
	@echo "Doing mbootclean"
	@rm -rf $(MBOOT_OUT)
	@rm -rf $(PRODUCT_OUT)/mboot.bin
	@rm -rf $(PRODUCT_OUT)/rom_emmc_boot.bin
	@rm -rf $(PRODUCT_OUT)/prebuilt_versions
	@echo "Building oldconfig for mboot"
	mkdir -p $(MBOOT_OUT)
	$(ACP) $(MBOOT_DIR)/$(MBOOT_DEFCONFIG) $(TARGET_MBOOT_CONFIG)
	yes "" | $(MAKE_SBOOT) -C $(MBOOT_DIR) MTK_SIGN_BOOTLOADER=$(MTK_SIGN_BOOTLOADER) O=$(MBOOT_OUT) oldconfig
	@echo "Building mboot from $(MBOOT_DIR)"
	$(MAKE_SBOOT) UFBL_PROJECT=$(UFBL_PROJECT) MTK_SIGN_BOOTLOADER=$(MTK_SIGN_BOOTLOADER) -C $(MBOOT_DIR) PRODUCT_OUT=$(abspath $(PRODUCT_OUT)) O=$(MBOOT_OUT)
	@rm -Rf $(MBOOT_DIR)/include
	@rm $(MBOOT_DIR)/.config
ifneq ($(MTK_SIGN_BOOTLOADER), true)
ifeq ($(wildcard $(TARGET_PREBUILT_SBOOT)),) 
TARGET_PREBUILT_SBOOT := $(MBOOT_OUT)/out/rom_emmc_boot.bin
endif
else
TARGET_PREBUILT_SBOOT := $(MBOOT_OUT)/out/rom_emmc_boot.bin
endif

INSTALLED_SBOOT_TARGET := $(foreach s, $(TARGET_PREBUILT_SBOOT), \
	$(addprefix $(PRODUCT_OUT)/, $(notdir $(s))))

$(INSTALLED_SBOOT_TARGET): $(INSTALLED_MBOOT_TARGET)

$(INSTALLED_MBOOT_TARGET): $(TARGET_BUILT_MBOOT) | $(ACP)
ifneq ($(MTK_SIGN_BOOTLOADER), true)
ifeq ($(wildcard $(TARGET_PREBUILT_MBOOT)),)
	@echo Copies the built mboot to $(PRODUCT_OUT)
	$(ACP) $(MBOOT_OUT)/out/mboot.bin $(PRODUCT_OUT)/mboot.bin
else
	@echo Copies the prebuilt mboot to $(PRODUCT_OUT)
	$(ACP) $(TARGET_PREBUILT_MBOOT) $(PRODUCT_OUT)/mboot.bin
endif
ifneq ($(TARGET_PREBUILT_VERSIONS),)
	@echo Copies the prebuilt_versions to $(PRODUCT_OUT)
	$(ACP) $(TARGET_PREBUILT_VERSIONS) $(PRODUCT_OUT)/prebuilt_versions
endif
else
	@echo Copies the built mboot to $(PRODUCT_OUT)
	$(ACP) $(MBOOT_OUT)/out/mboot.bin $(PRODUCT_OUT)/mboot.bin
endif
	@echo Copies the prebuilt sboot to $(PRODUCT_OUT)
	$(foreach f, $(TARGET_PREBUILT_SBOOT), \
		$(ACP) $(f) $(PRODUCT_OUT))

droidcore: $(INSTALLED_MBOOT_TARGET)

mbootclean:
	@echo "Doing mbootclean"
	@rm -rf $(MBOOT_OUT)
	@rm -rf $(PRODUCT_OUT)/mboot.bin
	@rm -rf $(PRODUCT_OUT)/rom_emmc_boot.bin

# This is for developer to only build mboot.
# when make any change above, please make chanege to this section as well if needed
mboot: | $(ACP)
	@echo "Doing mbootclean"
	@rm -rf $(MBOOT_OUT)
	@rm -rf $(PRODUCT_OUT)/mboot.bin
	@rm -rf $(PRODUCT_OUT)/rom_emmc_boot.bin
	@echo "Building oldconfig for mboot"
	mkdir -p $(MBOOT_OUT)
	$(ACP) $(MBOOT_DIR)/$(MBOOT_DEFCONFIG) $(TARGET_MBOOT_CONFIG)
	yes "" | $(MAKE_SBOOT) -C $(MBOOT_DIR) O=$(MBOOT_OUT) oldconfig
	@echo "Building mboot from $(MBOOT_DIR)"
	$(MAKE_SBOOT) UFBL_PROJECT=$(UFBL_PROJECT) MTK_SIGN_BOOTLOADER=$(MTK_SIGN_BOOTLOADER) -C $(MBOOT_DIR) PRODUCT_OUT=$(abspath $(PRODUCT_OUT)) O=$(MBOOT_OUT)
ifneq ($(MTK_SIGN_BOOTLOADER), true)
ifeq ($(wildcard $(TARGET_PREBUILT_MBOOT)),)
	@echo Copies the built mboot to $(PRODUCT_OUT)
	$(ACP) $(MBOOT_OUT)/out/mboot.bin $(PRODUCT_OUT)/mboot.bin
else
	@echo Copies the prebuilt mboot to $(PRODUCT_OUT)
	$(ACP) $(TARGET_PREBUILT_MBOOT) $(PRODUCT_OUT)/mboot.bin
endif
else
	@echo Copies the built mboot to $(PRODUCT_OUT)
	$(ACP) $(MBOOT_OUT)/out/mboot.bin $(PRODUCT_OUT)/mboot.bin
endif
	@echo Copies the prebuilt sboot to $(PRODUCT_OUT)
	$(foreach f, $(TARGET_PREBUILT_SBOOT), \
		$(ACP) $(f) $(PRODUCT_OUT))

.PHONY: mboot mbootclean
