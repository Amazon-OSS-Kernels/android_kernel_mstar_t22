# How to use:
#@sh    gen_TBL.sh  KSUM
# arg1 = KSUM
# arg2 = AUTHEN status
# arg3 = CHIP
# arg4 = config list
KSUM=$1
AUTHEN=$2
CHIP=$3
filename=$4
dos2unix $filename
dos2unix ./src/$CHIP/usercfg.sh
source  ./src/$CHIP/usercfg.sh
# **********************************************
# Environment
# **********************************************
STARG=src/$CHIP/
OUTPUTDIR=OutTbl
OUTOBJ=AllTbl.o
OUTDIR=out
OUTBIN=AllTbl.bin
PADINC="-I ./src -I ./src/$CHIP/include -I ./inc/$CHIP/board/ -I ./src/$CHIP"
TARGET_SRC=gen_TBL.c
target_autoConfig=include/autoconf.h

function ReadCVSFile()
{
    itemndex=0
    while read line
    do
        ARRAY[$itemndex]=$(echo $line | awk -F","  '{print $1}')
        itemndex=$[$itemndex+1];
    done < $filename
    for ((i=0; i<${#ARRAY[@]}; i++)); do
        echo ${ARRAY[$i]}
    done
}


function update_gen_sign_script()
{
    num=${#ARRAY[@]}
    sed -i  "s/Num_Tbl=[0-9][0-9]*[0-9]*/Num_Tbl=$num/g" gen_sign_v2.sh
    # num=${#TARGET_TBL_512[@]}
    # sed -i  "s/Num_Type_512=[0-9][0-9]*[0-9]*/Num_Type_512=$num/g" gen_sign_v2.sh
    # num=512
    # sed -i  "s/Tbl_Size_512=[0-9][0-9]*[0-9]*/Tbl_Size_512=$num/g" gen_sign_v2.sh
    # table2=${#TARGET_TBL_1024[@]}
    # sed -i  "s/Num_Type_1024=[0-9][0-9]*[0-9]*/Num_Type_1024=$table2/g" gen_sign_v2.sh
    # num=2048
    # sed -i  "s/Tbl_Size_1024=[0-9][0-9]*[0-9]*/Tbl_Size_1024=$num/g" gen_sign_v2.sh
}

function do_CopyTBL2Bin()
{
    ALIGN=512
        cnt_tbl_512=0
        cnt_tbl_1024=0

    for ((jndex=0; jndex<${#TARGET_TBL_512[@]}; jndex++)); do
        for ((index=0; index<${#ARRAY[@]}; index++)); do
            SECTION_NAME=".$index.${TARGET_TBL_512[$jndex]}"
            INPUT=$OUTPUTDIR/${TARGET_TBL_512[$jndex]}_$index.o
            OUTPUT=$OUTPUTDIR/${TARGET_TBL_512[$jndex]}_$index.tem
            offset=$(arm-none-linux-gnueabi-readelf -h $INPUT | grep "this header" | awk '{print $5}')
            cnt=$(arm-none-linux-gnueabi-readelf -s $INPUT | grep $SECTION_NAME | awk '{print $3}')
            echo offset=$offset cnt=$cnt SECTION_NAME=$SECTION_NAME INPUT=$INPUT OUTPUT=$OUTPUT

            dd if=/dev/zero of=$OUTPUT bs=1 count=$ALIGN
            dd if=$INPUT of=$OUTPUT bs=1 skip=$offset count=$cnt conv=notrunc
            cat    $OUTPUT >> $OUTPUTDIR/$OUTBIN
        done
                cnt_tbl_512=$(($cnt_tbl_512+1))
    done

    ALIGN=2048

    for ((jndex=0; jndex<${#TARGET_TBL_1024[@]}; jndex++)); do
        for ((index=0; index<${#ARRAY[@]}; index++)); do
            SECTION_NAME=".$index.${TARGET_TBL_1024[$jndex]}"
            INPUT=$OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
            OUTPUT=$OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.tem
            offset=$(arm-none-linux-gnueabi-readelf -h $INPUT | grep "this header" | awk '{print $5}')
            cnt=$(arm-none-linux-gnueabi-readelf -s $INPUT | grep $SECTION_NAME | awk '{print $3}')
            echo offset=$offset cnt=$cnt SECTION_NAME=$SECTION_NAME INPUT=$INPUT OUTPUT=$OUTPUT

            dd if=/dev/zero of=$OUTPUT bs=1 count=$ALIGN
            dd if=$INPUT of=$OUTPUT bs=1 skip=$offset count=$cnt conv=notrunc
            cat    $OUTPUT >>$OUTPUTDIR/$OUTBIN
        done
                cnt_tbl_1024=$(($cnt_tbl_1024+1))
    done
        update_gen_sign_script
        update_drvLoadTbl_header

}
function do_genObj()
{
    arm-none-linux-gnueabi-gcc $PADINC -c -o $OUTPUTDIR/$OUTOBJ $TARGET_SRC
    if [ ! -f $OUTPUTDIR/$OUTOBJ ]; then
        echo -e "\033[41;36m$config : Build Fail!!!\033[0m"
        exit 100
    fi
}

function do_objcopy()
{
    for ((jndex=0; jndex<${#TARGET_TBL_512[@]}; jndex++)); do
        arm-none-linux-gnueabi-objcopy -j.${TARGET_TBL_512[$jndex]} $OUTPUTDIR/$OUTOBJ $OUTPUTDIR/${TARGET_TBL_512[$jndex]}_$index.o
        arm-none-linux-gnueabi-objcopy --prefix-section .${index} $OUTPUTDIR/${TARGET_TBL_512[$jndex]}_$index.o
        arm-none-linux-gnueabi-objcopy --prefix-symbols .${index}. $OUTPUTDIR/${TARGET_TBL_512[$jndex]}_$index.o
    done

    for ((jndex=0; jndex<${#TARGET_TBL_1024[@]}; jndex++)); do
        arm-none-linux-gnueabi-objcopy -j.${TARGET_TBL_1024[$jndex]} $OUTPUTDIR/$OUTOBJ $OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
        arm-none-linux-gnueabi-objcopy --prefix-section .${index} $OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
        arm-none-linux-gnueabi-objcopy --prefix-symbols .${index}. $OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
    done

}
function do_Main()
{
    ReadCVSFile
    if [ -d $OUTPUTDIR ]; then
       echo directory exist, remove it
       rm $OUTPUTDIR -rf
    fi

    mkdir $OUTPUTDIR
    CreatedFile_gen_TBL_C
    for ((index=0; index<${#ARRAY[@]}; index++)); do
        config=${ARRAY[$index]}
        echo the config is: $config

        cp $config scripts/defconfig
        make defconfig

        do_genObj
        #cp  $OUTPUTDIR/$OUTOBJ $OUTPUTDIR/${index}
        do_objcopy
    done
    do_CopyTBL2Bin

    cp  $OUTPUTDIR/$OUTBIN $OUTDIR
}
do_Main
if [ "$AUTHEN" == "AUTHEN_ENABLE" ]; then
    echo "ENABLE AUTHEN!!!"
    sh gen_sign_v2.sh
fi
