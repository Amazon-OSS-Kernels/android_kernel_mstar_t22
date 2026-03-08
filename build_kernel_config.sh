################################################################################
#
#  build_kernel_config.sh
#
#  Copyright (c) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

KERNEL_SUBPATH="kernel/mstar/t22/4.9"
DEFCONFIG_NAME="t22_defconfig"
TARGET_ARCH="arm64"
TOOLCHAIN_REPO="https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9"
TOOLCHAIN_PREFIX="aarch64-linux-android-"
MAKE_DTBS=y

# Expected image files are seperated with ":"
KERNEL_IMAGES="arch/arm64/boot/Image:arch/arm64/boot/Image.gz"
