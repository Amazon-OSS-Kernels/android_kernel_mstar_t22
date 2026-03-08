#!/bin/bash

#----------------------------------------
# PATH Define
#----------------------------------------
InputFilePath=$1
TARGET_DIR=../out
MAIN_SCRIPT_DIR=../mboot_update.txt
TOOLDIR=.
SECURE_TOOLDIR=../secure
TMP_UPGRADE_FILE=$TARGET_DIR/TMP_MstarUpgrade.bin
UPGRADE_FILE=$TARGET_DIR/MbootUpgrade.bin
SCRIPT_FILE=$TARGET_DIR/upgrade_script.txt

#----------------------------------------
# Globe Value Define
#----------------------------------------
SPILT_SIZE=16384
SCRIPT_SIZE=0x4000
CurrentImageOffset=$SCRIPT_SIZE
LastImageOffset=$CurrentImageOffset
IMAGE_ALIGNMENT_SIZE=0x10000
MAX_DRAM_BUF_SIZE=0 # value setting in func_init_process

#----------------------------------------
# Control Flow Define
#----------------------------------------
#DFUITD: Download Full upgrade Image to DRAM
#DFUITD do not support compress mode.
DFUITD=0
DFUITD_PATTERN="0x20400000"
DFUITD_DRAM_BUFFER=0x20000000
ENABLE_COMPRESS=1
ENABLE_REAL_KEY=0
SECURE_UPGRADE=$2 

function func_alignment
{
    InputAlignSize=$(($(($1 + $2 - 1)) & $[~($2-1)]))
    echo $InputAlignSize
}

function func_subContent_process
{
    InputStr=$1 
    subContent_prefix=$(echo $InputStr | awk '{print $1}')
    if [ "$subContent_prefix" == "tftp" ]; then
        DRAM_BUF_ADDR=$(echo $InputStr | awk '{print $2}')
        ImagePath=$(echo $InputStr | awk '{print $3} ' | cut -d '/' -f 4)   
        printf "Processing : $ImagePath\n"
        if [ "$ENABLE_COMPRESS" == "1" ] && [ "$DFUITD" == "0" ]; then
            $TOOLDIR/mscompress7 e 0 $TARGET_DIR/$ImagePath $TARGET_DIR/$ImagePath.compress
            OriImageSize=$(stat -c%s $TARGET_DIR/$ImagePath)
            ImageSize=$(stat -c%s $TARGET_DIR/$ImagePath.compress)
            DRAM_DECOMPRESS_BUF_ADDR=$(echo "obase=16;$(($DRAM_BUF_ADDR + $MAX_DRAM_BUF_SIZE))"|bc)  
            printf "filepartload 0x%s \$(UpgradeImage) 0x%x 0x%x #$filename\n" $DRAM_DECOMPRESS_BUF_ADDR $CurrentImageOffset $ImageSize >> $SCRIPT_FILE
            printf "mscompress7 d 0 0x%s %x 0x%x #$filename\n" $DRAM_DECOMPRESS_BUF_ADDR $ImageSize $DRAM_BUF_ADDR >> $SCRIPT_FILE
            
            # == Add crc check==
            cp $TARGET_DIR/$ImagePath $TARGET_DIR/$ImagePath.tmp
            CRC_VALUE=$( $TOOLDIR/crc -a $TARGET_DIR/$ImagePath.tmp | grep "CRC32" | awk '{print $3}')
            rm $TARGET_DIR/$ImagePath.tmp
            DRAM_DECOMPRESS_BUF_ADDR_CRC=$(echo "obase=16;$((0x$DRAM_DECOMPRESS_BUF_ADDR + 0x4))"|bc)            
            printf "crc32 0x%x 0x%x 0x%s #$filename\n" $DRAM_BUF_ADDR $OriImageSize $DRAM_DECOMPRESS_BUF_ADDR  >> $SCRIPT_FILE
            printf "mw 0x%s %s 4 #$filename\n" $DRAM_DECOMPRESS_BUF_ADDR_CRC $CRC_VALUE >> $SCRIPT_FILE
            printf "cmp.b 0x%s 0x%s 4 #$filename\n" $DRAM_DECOMPRESS_BUF_ADDR $DRAM_DECOMPRESS_BUF_ADDR_CRC >> $SCRIPT_FILE

            # cat binary to MstarUpgrade.bin
            cat $TARGET_DIR/$ImagePath.compress >> $TMP_UPGRADE_FILE
        else
            ImageSize=$(stat -c%s $TARGET_DIR/$ImagePath)
            if [ "$DFUITD" == "0" ]; then
                printf "filepartload %s \$(UpgradeImage) 0x%x 0x%x #$filename\n" $DramBuffer $CurrentImageOffset $ImageSize >> $SCRIPT_FILE
            fi
            cat $TARGET_DIR/$ImagePath >> $TMP_UPGRADE_FILE
        fi    
        $TOOLDIR/align $TMP_UPGRADE_FILE $IMAGE_ALIGNMENT_SIZE
        ImageAlignSize=$(func_alignment $ImageSize $IMAGE_ALIGNMENT_SIZE)
        LastImageOffset=$CurrentImageOffset
        CurrentImageOffset=$(($CurrentImageOffset+$ImageAlignSize)) 
    else
        if [ "$DFUITD" == "1" ]; then
            CurrentDramBuffer=$(echo "obase=16;$(($LastImageOffset|$DFUITD_DRAM_BUFFER))"|bc)           
            InputStr=$(echo $InputStr|sed "s/${DFUITD_PATTERN}/0x${CurrentDramBuffer}/")
        fi
        InputStr=$(echo $InputStr | tr -d '\r\n')
        echo $InputStr "#"$filename >> $SCRIPT_FILE
    fi
} 

function func_init_process
{
    # check Secure Upgrade Flag
    if [ "$SECURE_UPGRADE" == "" ]; then       
        SECURE_UPGRADE=0
    fi

    # check input file
    if [ "$InputFilePath" != "" ]; then       
        MAIN_SCRIPT_DIR=$InputFilePath 
    fi
    # check output path
    #filename=$(grep "\[\[" $MAIN_SCRIPT_DIR | head -n 1 | awk '{print $2}')
    #fsPath=$(grep  "^tftp" $TARGET_DIR/$filename | head -n 1 | awk '{print $3}' | cut -d '/' -f 2)

	# delete related file
    rm -f $SCRIPT_FILE
    rm -f $UPGRADE_FILE
    rm -f $TMP_UPGRADE_FILE
	
    # set MAX_DRAM_BUF_SIZE
    MAX_DRAM_BUF_SIZE=$(func_alignment $(ls -S $TARGET_DIR | head -n 1 | xargs -I FILE stat -c "%s" $TARGET_DIR/FILE) 0x100000)
    printf "DRAM_BUF_SIZE(MAX_FILE_SIZE):0x%x \n" $MAX_DRAM_BUF_SIZE
}

function func_script_finish
{
    echo "setenv MstarUpgrade_complete 1" >>$SCRIPT_FILE
    echo "saveenv" >>$SCRIPT_FILE
    echo "printenv" >>$SCRIPT_FILE
    if [ "$BRICKTERMINATOR_ENABLE" != "1" ] ; then
        echo "reset" >>$SCRIPT_FILE
    fi
    echo "% <- this is end of script symbol" >>$SCRIPT_FILE
}

function func_post_process
{
    # pad script to script_file size
    $TOOLDIR/align $SCRIPT_FILE $SCRIPT_SIZE
    
    # create $UPGRADE_FILE
    cat $SCRIPT_FILE > $UPGRADE_FILE
    cat $TMP_UPGRADE_FILE >> $UPGRADE_FILE
    rm $TMP_UPGRADE_FILE
    
    # pad script crc to $UPGRADE_FILE
    CRC_VALUE=`$TOOLDIR/crc -a $SCRIPT_FILE | grep "CRC32" | awk '{print $3;}'`
    split -d -a 2 -b $SPILT_SIZE $SCRIPT_FILE $SCRIPT_FILE.
    printf "script file crc %s\n" $CRC_VALUE
    cat $SCRIPT_FILE.01 >> $UPGRADE_FILE
    rm -f $SCRIPT_FILE.*
    
    #pad $UPGRADE_FILE crc to $UPGRADE_FILE
    $TOOLDIR/crc -a $UPGRADE_FILE
    
    #copy the first 16 bytes to last
    dd if=$UPGRADE_FILE of=./first16.bin bs=16 count=1;
    cat ./first16.bin >> $UPGRADE_FILE
    rm -f ./first16.bin
	
	#==============Pack.sh=================================
    if [ "$SECURE_UPGRADE" == "1" ] ; then
        cd $SECURE_TOOLDIR
        ./Pack.sh $SECURE_TOOLDIR/RSAupgrade_priv.txt $SECURE_TOOLDIR/RSAupgrade_pub.bin $SECURE_TOOLDIR/AESupgrade.bin $UPGRADE_FILE $TARGET_DIR/abc.bin 1 655360 $ENABLE_REAL_KEY		
        cp -f abc.bin.aes UPGRADE_FILE
    fi
    #==============Pack.sh end=================================
}

#-----------------------------------------------
# Main():Generate Upgrade Image via Tftp Script
#-----------------------------------------------
IFS="
"
func_init_process;
echo "UpgradeImage Generating....."
echo "# Start auto_update.txt parsing" >> $SCRIPT_FILE
mainScript=$(grep -Ev "^$|#|%" $MAIN_SCRIPT_DIR)
for mainContent in $mainScript
do
    if [ "$(echo $mainContent | awk '{print $1}')" == "mstar" ];then
        filepath=$(echo $mainContent|awk '{print $2}')
        #filename=$(echo $filepath | cut -d '/' -f 2 | cut -d '[' -f 3)
		filename="MBOOT"		
        echo "" >> $SCRIPT_FILE
        echo "# File Partition: "$filename >> $SCRIPT_FILE
        filecontent=$(grep -Ev "^\s*$|#|%" $TARGET_DIR/$filepath)
        for subContent in $filecontent
        do
            func_subContent_process $subContent
        done
    else
        if [ "$mainContent" != "reset" ] && [ "$mainContent" != "printenv" ];then
            echo $mainContent >> $SCRIPT_FILE
        fi
    fi
done
func_script_finish;
func_post_process;
