#!/bin/bash

#----------------------------------------
# Control Flow Define
#----------------------------------------
SECURE_UPGRADE=0
MAIN_SCRIPT_DIR=./mboot_update.txt
MBOOT_CONFIG_FILE=../.config
CPU_PLATFORM="arm"
MBOOT_STORAGE="emmc"
TARGET_SCRIPT="NULL"

# ARM or MIPS
if grep --quiet CONFIG_MCU_ARM=y $MBOOT_CONFIG_FILE; then
    CPU_PLATFORM="arm"
else
    CPU_PLATFORM="mips"
fi


# EMMC or NAND or SPI
if grep --quiet CONFIG_MBOOT_IN_NAND_FLASH=y $MBOOT_CONFIG_FILE; then
    MBOOT_STORAGE="nand"
elif grep --quiet CONFIG_MBOOT_IN_MMC_FLASH=y $MBOOT_CONFIG_FILE; then
    MBOOT_STORAGE="emmc"
else
    MBOOT_STORAGE="spi"
fi

TARGET_SCRIPT="../mscript/"$CPU_PLATFORM"_"$MBOOT_STORAGE".txt"
echo filepath: $TARGET_SCRIPT
sed -i "1c mstar $TARGET_SCRIPT" $MAIN_SCRIPT_DIR
#cat $MAIN_SCRIPT_DIR

sh script_parsing_engine.sh $MAIN_SCRIPT_DIR $SECURE_UPGRADE;
