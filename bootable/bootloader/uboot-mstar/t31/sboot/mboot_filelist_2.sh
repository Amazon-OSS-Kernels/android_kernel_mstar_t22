#!/bin/sh
# for nct4 tracking file list

OutDir=$BSP_OBJ_ROOT
MBOOT_OUT=$OutDir/mboot
UBOOT_ROOT=$MBOOT_SRC_DIR/u-boot-2011.06
SBOOT_ROOT=$MBOOT_SRC_DIR/sboot
FUSION_LOADER_ROOT=$SBOOT_ROOT/bin/mtk_loader/$(echo $MST_CHIP_NAME | tr a-z A-Z)

find $MBOOT_SRC_DIR ! -path '*.git/*' > $MBOOT_SRC_DIR/filelist_2.txt

diff $MBOOT_SRC_DIR/filelist_1.txt $MBOOT_SRC_DIR/filelist_2.txt | grep "> " | sed s/'> '//g > $MBOOT_SRC_DIR/diff.txt

READFILE="$MBOOT_SRC_DIR/diff.txt"
while read line; do
    if [ -f "$line" ]
    then
        lineq=${line/$MBOOT_SRC_DIR\//}

        DIR=$(dirname "$lineq")
        if [ ! -d $MBOOT_OUT/$DIR  ]
        then
           mkdir -p $MBOOT_OUT/$DIR
        fi

        #echo "mv $line $MBOOT_OUT/$lineq" >> $MBOOT_SRC_DIR/movelist.txt
        mv $line $MBOOT_OUT/$lineq
    fi
done < $READFILE

# files to recovery
mv $UBOOT_ROOT/include/version_bak.h $UBOOT_ROOT/include/version.h

if [ -f "$FUSION_LOADER_ROOT/bootloader_bak.bin" ]
then
    cp $FUSION_LOADER_ROOT/bootloader_bak.bin $FUSION_LOADER_ROOT/bootloader.bin
fi

# reset symbolic link
unlink $UBOOT_ROOT/arch/arm/include/asm/arch
unlink $UBOOT_ROOT/arch/arm/include/asm/proc
unlink $UBOOT_ROOT/arch/arm/lib/mstar
unlink $UBOOT_ROOT/board/mstar
unlink $UBOOT_ROOT/drivers/mstar
unlink $UBOOT_ROOT/lib/mstar

# remove temp files
rm -rf $MBOOT_SRC_DIR/*.txt
