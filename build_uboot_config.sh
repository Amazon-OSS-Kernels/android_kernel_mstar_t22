################################################################################
#
#  build_uboot_config.sh
#
#  Copyright (c) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

UBOOT_SUBPATH="bootable/bootloader/uboot-mstar/t31/sboot"

# Expected image files are seperated with ":"
UBOOT_IMAGES="out/unsigned/chunk_header.bin:out/unsigned/hash0.bin:out/unsigned/sboot.bin.unsigned:out/unsigned/u-boot.bin"

################################################################################
# NOTE: You must fill in the following with the path to a copy of an
#       arm_eabi-2011.03 compiler
################################################################################
CROSS_COMPILER_PATH=""
