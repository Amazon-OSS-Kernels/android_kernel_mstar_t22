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
# **********************************************
# Environment
# **********************************************
#export PATH="/tools/arm-2011.03/bin:/usr/local/sbin":$PATH
STARG=src/$CHIP/
OUTPUTDIR=OutTbl
OUTOBJ=AllTbl.o
OUTDIR=out
OUTBIN=AllTbl.bin
PADINC="-I ./src -I ./src/$CHIP/include -I ./inc/$CHIP/board/ -I ./src/$CHIP"
TARGET_SRC=gen_TBL.c
target_autoConfig=include/autoconf.h

TARGET_TBL_512=\
(
    padInitTbl_PreInit
)
TARGET_TBL_1024=\
(
    padInitTbl
    MIU0_DDR_Init
    MIU1_DDR_Init
    MIU2_DDR_Init
    MIU0_DDR_Init_Str
    MIU1_DDR_Init_Str
    MIU2_DDR_Init_Str
    MIU_ReLoad8GbPara_Miu0
    MIU_ReLoad8GbPara_Miu1
    MIU_ReLoad8GbPara_Miu2
)
TARGET_TBL_1024_Dou=\
(
    padInitTbl
    MIU0_DDR_Init
    MIU1_DDR_Init
    MIU0_DDR_Init_Str
    MIU1_DDR_Init_Str
)
MISC_TBL=\
(
    ChunkHeader
    BufferMEM
)
function CreatedFile_gen_TBL_C()
{
    rm -rf $TARGET_SRC
	if [ $CHIP == "maxim" ]; then
        echo "#include \"include/autoconf.h\"                                                                         ">>$TARGET_SRC
        echo "#include \"drvRIU.h\"                                                                                   ">>$TARGET_SRC
        echo "#include \"drv_miu_init.h\"                                                                             ">>$TARGET_SRC
        echo "#include \"drv_miu_init_str.h\"                                                                         ">>$TARGET_SRC
        echo "#include \"drvPadConf.c\"                                                                               ">>$TARGET_SRC
        echo "                                                                                                        ">>$TARGET_SRC
        echo "const U8 padInitTbl_PreInit[] __attribute__((section(\".padInitTbl_PreInit\")));                        ">>$TARGET_SRC
        echo "const U8 padInitTbl[] __attribute__((section(\".padInitTbl\")));                                        ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU0_DDR_Init[] __attribute__((section(\".MIU0_DDR_Init\")));                         ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU1_DDR_Init[] __attribute__((section(\".MIU1_DDR_Init\")));                         ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU0_DDR_Init_Str[] __attribute__((section(\".MIU0_DDR_Init_Str\")));                 ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU1_DDR_Init_Str[] __attribute__((section(\".MIU1_DDR_Init_Str\")));                 ">>$TARGET_SRC
        echo "void main( void )                                                                                       ">>$TARGET_SRC
        echo "{                                                                                                       ">>$TARGET_SRC
        echo "                                                                                                        ">>$TARGET_SRC
        echo "}                                                                                                        ">>$TARGET_SRC

    else
        echo "#include \"include/autoconf.h\"                                                                         ">>$TARGET_SRC
        echo "#include \"drvRIU.h\"                                                                                   ">>$TARGET_SRC
        echo "#include \"drv_miu_init.h\"                                                                             ">>$TARGET_SRC
        echo "#include \"drv_miu_init_str.h\"                                                                         ">>$TARGET_SRC
        echo "#include \"drv_miu_ext.h\"                                                                              ">>$TARGET_SRC
        echo "#include \"drvPadConf.c\"                                                                               ">>$TARGET_SRC
        echo "                                                                                                        ">>$TARGET_SRC
        echo "const U8 padInitTbl_PreInit[] __attribute__((section(\".padInitTbl_PreInit\")));                          ">>$TARGET_SRC
        echo "const U8 padInitTbl[] __attribute__((section(\".padInitTbl\")));                                          ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU0_DDR_Init[] __attribute__((section(\".MIU0_DDR_Init\")));                           ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU1_DDR_Init[] __attribute__((section(\".MIU1_DDR_Init\")));                           ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU2_DDR_Init[] __attribute__((section(\".MIU2_DDR_Init\")));                           ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU0_DDR_Init_Str[] __attribute__((section(\".MIU0_DDR_Init_Str\")));                   ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU1_DDR_Init_Str[] __attribute__((section(\".MIU1_DDR_Init_Str\")));                   ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU2_DDR_Init_Str[] __attribute__((section(\".MIU2_DDR_Init_Str\")));                   ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU_ReLoad8GbPara_Miu0[] __attribute__((section(\".MIU_ReLoad8GbPara_Miu0\")));         ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU_ReLoad8GbPara_Miu1[] __attribute__((section(\".MIU_ReLoad8GbPara_Miu1\")));         ">>$TARGET_SRC
        echo "const MS_REG_INIT MIU_ReLoad8GbPara_Miu2[] __attribute__((section(\".MIU_ReLoad8GbPara_Miu2\")));         ">>$TARGET_SRC
        echo "void main( void )                                                                                       ">>$TARGET_SRC
        echo "{                                                                                                       ">>$TARGET_SRC
        echo "                                                                                                        ">>$TARGET_SRC
        echo "}                                                                                                       ">>$TARGET_SRC
    fi
        
}
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
    sed -i  "s/Num_Tbl=[0-9][0-9]*[0-9]*/Num_Tbl=$num/g" gen_sign.sh
}

function update_drvLoadTbl_header()
{
    sed -i  "s/#define OUTERSIZE.*/#define OUTERSIZE      $KSUM/g" src/$CHIP/drvLoadTbl.h


    v=$((${#ARRAY[@]}*512))
    val=$(printf "0x%x" $v)
    sed -i  "s/#define PAD_PRE_SECT_SIZE.*/#define PAD_PRE_SECT_SIZE      $val/g" src/$CHIP/drvLoadTbl.h
    v=$((${#ARRAY[@]}*1024))
    val=$(printf "0x%x" $v)
    sed -i  "s/#define PAD_SECT_SIZE.*/#define PAD_SECT_SIZE          $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_SECT_SIZE.*/#define MIU0_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_SECT_SIZE.*/#define MIU1_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU2_SECT_SIZE.*/#define MIU2_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_STR_SECT_SIZE.*/#define MIU0_STR_SECT_SIZE     $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_STR_SECT_SIZE.*/#define MIU1_STR_SECT_SIZE     $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU2_STR_SECT_SIZE.*/#define MIU2_STR_SECT_SIZE     $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_RELOAD_SECT_SIZE.*/#define MIU0_RELOAD_SECT_SIZE     $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_RELOAD_SECT_SIZE.*/#define MIU1_RELOAD_SECT_SIZE     $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU2_RELOAD_SECT_SIZE.*/#define MIU2_RELOAD_SECT_SIZE     $val/g" src/$CHIP/drvLoadTbl.h
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

    ALIGN=1024
	if [ $CHIP == "maxim" ]; then
    for ((jndex=0; jndex<${#TARGET_TBL_1024_Dou[@]}; jndex++)); do
        for ((index=0; index<${#ARRAY[@]}; index++)); do
            SECTION_NAME=".$index.${TARGET_TBL_1024_Dou[$jndex]}"
            INPUT=$OUTPUTDIR/${TARGET_TBL_1024_Dou[$jndex]}_$index.o
            OUTPUT=$OUTPUTDIR/${TARGET_TBL_1024_Dou[$jndex]}_$index.tem
            offset=$(arm-none-linux-gnueabi-readelf -h $INPUT | grep "this header" | awk '{print $5}')
            cnt=$(arm-none-linux-gnueabi-readelf -s $INPUT | grep $SECTION_NAME | awk '{print $3}')
            echo offset=$offset cnt=$cnt SECTION_NAME=$SECTION_NAME INPUT=$INPUT OUTPUT=$OUTPUT

            dd if=/dev/zero of=$OUTPUT bs=1 count=$ALIGN
            dd if=$INPUT of=$OUTPUT bs=1 skip=$offset count=$cnt conv=notrunc
            cat    $OUTPUT >>$OUTPUTDIR/$OUTBIN
        done
                cnt_tbl_1024=$(($cnt_tbl_1024+1))
    done
    else    
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
    fi
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
	if [ $CHIP == "maxim" ]; then
        for ((jndex=0; jndex<${#TARGET_TBL_1024_Dou[@]}; jndex++)); do
            arm-none-linux-gnueabi-objcopy -j.${TARGET_TBL_1024_Dou[$jndex]} $OUTPUTDIR/$OUTOBJ $OUTPUTDIR/${TARGET_TBL_1024_Dou[$jndex]}_$index.o
            arm-none-linux-gnueabi-objcopy --prefix-section .${index} $OUTPUTDIR/${TARGET_TBL_1024_Dou[$jndex]}_$index.o
            arm-none-linux-gnueabi-objcopy --prefix-symbols .${index}. $OUTPUTDIR/${TARGET_TBL_1024_Dou[$jndex]}_$index.o
        done    
    else
        for ((jndex=0; jndex<${#TARGET_TBL_1024[@]}; jndex++)); do
            arm-none-linux-gnueabi-objcopy -j.${TARGET_TBL_1024[$jndex]} $OUTPUTDIR/$OUTOBJ $OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
            arm-none-linux-gnueabi-objcopy --prefix-section .${index} $OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
            arm-none-linux-gnueabi-objcopy --prefix-symbols .${index}. $OUTPUTDIR/${TARGET_TBL_1024[$jndex]}_$index.o
        done
    fi
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
    sh gen_sign.sh
fi
