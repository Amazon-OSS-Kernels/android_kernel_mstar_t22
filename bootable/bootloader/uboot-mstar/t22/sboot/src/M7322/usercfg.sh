# **********************************************
# Customer INFO
# **********************************************

# **********************************************
# Murphy board config
# **********************************************
Chip_config=\
(
    configs/M7322/.config.m7322.supernova.MST150B_10ANQHT_18151.rom_emmc_32bit.nonUMA
    configs/M7322/.config.m7322.supernova.MST148B_10NQAT_18115.rom_emmc_32bit.optee.mi
)
# **********************************************
# parameter
# **********************************************
TARGET_TBL_512=\
(
    padInitTbl_PreInit
)
TARGET_TBL_1024=\
(
    padInitTbl
    MIU0_DDR_PreInit
    MIU0_DDR_Init
    MIU1_DDR_PreInit
    MIU1_DDR_Init
    MIU0_DDR_PreInit_Str
    MIU0_DDR_Init_Str
    MIU1_DDR_PreInit_Str
    MIU1_DDR_Init_Str
)
function update_drvLoadTbl_header()
{
    sed -i  "s/#define OUTERSIZE.*/#define OUTERSIZE                           $KSUM/g" src/$CHIP/drvLoadTbl.h
    v=$((${#ARRAY[@]}*512))
    val=$(printf "0x%x" $v)
    sed -i  "s/#define PAD_PRE_SECT_SIZE.*/#define PAD_PRE_SECT_SIZE           $val/g" src/$CHIP/drvLoadTbl.h
    v=$((${#ARRAY[@]}*2048))
    val=$(printf "0x%x" $v)
    sed -i  "s/#define PAD_SECT_SIZE.*/#define PAD_SECT_SIZE                   $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_PRE_SECT_SIZE.*/#define MIU0_PRE_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_SECT_SIZE.*/#define MIU0_SECT_SIZE                 $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_PRE_SECT_SIZE.*/#define MIU1_PRE_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_SECT_SIZE.*/#define MIU1_SECT_SIZE                 $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_STR_PRE_SECT_SIZE.*/#define MIU0_STR_PRE_SECT_SIZE $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_STR_PRE_SECT_SIZE.*/#define MIU1_STR_PRE_SECT_SIZE $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU0_STR_SECT_SIZE.*/#define MIU0_STR_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h
    sed -i  "s/#define MIU1_STR_SECT_SIZE.*/#define MIU1_STR_SECT_SIZE         $val/g" src/$CHIP/drvLoadTbl.h

}
function CreatedFile_gen_TBL_C()
{
    rm -rf $TARGET_SRC
    echo "#include \"include/autoconf.h\"                                                                         ">>$TARGET_SRC
    echo "#include \"drvRIU.h\"                                                                                   ">>$TARGET_SRC
    echo "#include \"drv_miu_init.h\"                                                                             ">>$TARGET_SRC
    echo "#include \"drv_miu_init_str.h\"                                                                         ">>$TARGET_SRC
    echo "#include \"drvPadConf.c\"                                                                               ">>$TARGET_SRC
    echo "                                                                                                        ">>$TARGET_SRC
    echo "const U8 padInitTbl_PreInit[] __attribute__((section(\".padInitTbl_PreInit\")));                        ">>$TARGET_SRC
    echo "const U8 padInitTbl[] __attribute__((section(\".padInitTbl\")));                                        ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU0_DDR_PreInit[] __attribute__((section(\".MIU0_DDR_PreInit\")));                   ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU1_DDR_PreInit[] __attribute__((section(\".MIU1_DDR_PreInit\")));                   ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU0_DDR_Init[] __attribute__((section(\".MIU0_DDR_Init\")));                         ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU1_DDR_Init[] __attribute__((section(\".MIU1_DDR_Init\")));                         ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU0_DDR_Init_Str[] __attribute__((section(\".MIU0_DDR_Init_Str\")));                 ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU1_DDR_Init_Str[] __attribute__((section(\".MIU1_DDR_Init_Str\")));                 ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU0_DDR_PreInit_Str[] __attribute__((section(\".MIU0_DDR_PreInit_Str\")));           ">>$TARGET_SRC
    echo "const MS_REG_INIT MIU1_DDR_PreInit_Str[] __attribute__((section(\".MIU1_DDR_PreInit_Str\")));           ">>$TARGET_SRC
    echo "void main( void )                                                                                       ">>$TARGET_SRC
    echo "{                                                                                                       ">>$TARGET_SRC
    echo "                                                                                                        ">>$TARGET_SRC
    echo "}                                                                                                       ">>$TARGET_SRC
}


function CreateDirectory()
{
    if [ -d $1 ]; then
        echo directory:$1 exist, remove it
        rm $1 -rf
    fi
    mkdir $1

}
function MSGRED
{
    echo -e "\033[41;36m$1\033[0m"
}
function MSGYELLOW
{
    echo -e "\033[33m$1\033[0m"
}