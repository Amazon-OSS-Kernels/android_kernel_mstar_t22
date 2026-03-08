#!/bin/sh
# for nct4 to tracking file list

UBOOT_ROOT=$MBOOT_SRC_DIR/u-boot-2011.06

# special file to backup
cp $UBOOT_ROOT/include/version.h $UBOOT_ROOT/include/version_bak.h

find $MBOOT_SRC_DIR ! -path '*.git/*'> $MBOOT_SRC_DIR/filelist_1.txt
