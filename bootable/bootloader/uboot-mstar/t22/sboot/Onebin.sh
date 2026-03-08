# How to use:
#@sh	gen_TBL.sh  KSUM
# arg1 = KSUM
# arg2 = AUTHEN status
KSUM=$1
AUTHEN=$2
CHIP_FOLDER=$3
#INPUTCONFIG=$4
# **********************************************
# Environment
# **********************************************
TempFOLDER=OnebinTemp
CSVfile=$TempFOLDER/configfile.csv
INPUTCONFIG=.config
target_defConfig=scripts/defconfig
# *************************************************************
#PAD_GPIO35	PAD_GPIO34	DRAM Brand       Speed Mode   OnebinSet
#     L          L          Micron        Low  Speed       0
#     L          L          Micron        High Speed       1
#     H          L          Hynix         Low  Speed       2
#     H          L          Hynix         High Speed       3
#     L          H          New Samsung   Low  Speed       4
#     L          H          New Samsung   High Speed       5
#     H          H          Old  Samsung  Low  Speed       6
#     H          H          Old  Samsung  High Speed       7
# *************************************************************
# *******************************************************************************
#formula:
#OnebinSet=( PAD_GPIO35 + PAD_GPIO34 * 2 )*2 + Speed Mode
#And
# L = 0
# H = 1
# Low  Speed=0
# High Speed=1
# *******************************************************************************
#exsample  :
#           If
#           DRAM Brand:Hynix
#           Speed Mode:High Speed
#           MIU_MODE:MIU0_8X_MODE
#Use table and calculate OnebinSet
#OnebinSet=( PAD_GPIO35 + PAD_GPIO34 * 2 )*2 + Speed Mode
#    3   =(      1     +      0     *2  )*2 +     1
# *******************************************************************************
DRAM_Brand=\
(
    CONFIG_MICRON_DRAM_EMBEDDED
	CONFIG_HYNIX_DRAM_EMBEDDED
    CONFIG_SAMSUNG_DRAM_EMBEDDED
    CONFIG_SAMSUNG_DRAM_EMBEDDED_OLD_DIE
)
DRAM_SPEED=\
(
    CONFIG_DDR4_LOW_SPEED
    CONFIG_DDR4_HIGH_SPEED
)
maxim_config=\
(
    configs/maxim/.config.maxim.android.111d_23x23_vby1_samsung.rom_emmc.64bit.optee.kernel
    configs/maxim/.config.maxim.android.111d_23x23_vby1_nanya.rom_emmc.64bit.optee.kernel
)
function AddDDRtypeInConfig()
{
	if [ $CHIP_FOLDER == "maxim" ]; then
        sed -i  "s/CONFIG_NANYA_DRAM_EMBEDDED=y/# CONFIG_NANYA_DRAM_EMBEDDED  is not set/g" $1
        sed -i  "s/CONFIG_SAMSUNG_DRAM_EMBEDDED=y/# CONFIG_SAMSUNG_DRAM_EMBEDDED  is not set/g" $1
        sed -i  "1 i${DRAM_Brand_maxim[$2]}=y" $1
        sed -i  "s/CONFIG_ONEBIN_ENABLE=y/#CONFIG_ONEBIN_ENABLE  is not set/g" $1      
    else
        #Set DDR type
        sed -i  "s/CONFIG_MICRON_DRAM_EMBEDDED=y/# CONFIG_MICRON_DRAM_EMBEDDED  is not set/g" $1
        sed -i  "s/CONFIG_HYNIX_DRAM_EMBEDDED=y/# CONFIG_HYNIX_DRAM_EMBEDDED  is not set/g" $1
        sed -i  "s/CONFIG_SAMSUNG_DRAM_EMBEDDED=y/# CONFIG_SAMSUNG_DRAM_EMBEDDED  is not set/g" $1
        sed -i  "s/CONFIG_SAMSUNG_DRAM_EMBEDDED_OLD_DIE=y/#CONFIG_SAMSUNG_DRAM_EMBEDDED_OLD_DIE  is not set/g" $1
        sed -i  "1 i${DRAM_Brand[$2]}=y" $1

        #Set DDR Mode
        sed -i  "s/CONFIG_DDR4_LOW_SPEED=y/# CONFIG_DDR4_LOW_SPEED  is not set/g" $1
        sed -i  "s/CONFIG_DDR4_HIGH_SPEED=y/# CONFIG_DDR4_HIGH_SPEED  is not set/g" $1
        sed -i  "2 i${DRAM_SPEED[$3]}=y" $1

        sed -i  "s/CONFIG_ONEBIN_ENABLE=y/#CONFIG_ONEBIN_ENABLE  is not set/g" $1
    fi
}
#backup config
function do_backup_config()
{
    if [  -f include/autoconf.h ]; then
        echo "backup autoconf.h"
        cp include/autoconf.h autoconf.h_backup
    fi

    if [  -f ".config" ]; then
        echo "backup .config"
        cp .config .config_backup
    fi

    if [  -f $target_defConfig  ]; then
        echo "backup $target_defConfig"
        #backup scripts/defconfig
        cp  $target_defConfig $target_defConfig"_backup"
    fi
}
function genConfig()
{
	if [ $CHIP_FOLDER == "maxim" ]; then
        for ((index=0; index<${#maxim_config[@]}; index++)); do        
                config=${maxim_config[$index]}
                echo $config >>$CSVfile
        done
    else
        for ((index=0; index<${#DRAM_Brand[@]}; index++)); do
            for ((index_j=0; index_j<${#DRAM_SPEED[@]}; index_j++)); do
                CONFIGNUM=$((($((($index*2)))+$index_j)))

                target_source=$TempFOLDER/$CONFIGNUM'_config'
                DRAMBRAND=$index
                DRAMSPEEDMode=$index_j

                #Copy config
                cp -f $INPUTCONFIG $target_source
                AddDDRtypeInConfig $target_source $DRAMBRAND $DRAMSPEEDMode

                #save config list in CSVfile
                echo $target_source >>$CSVfile
            done
        done
	fi


}
function do_Main()
{
    do_backup_config
    echo $CHIP_FOLDER
    genConfig
    sh gen_TBL.sh $KSUM $AUTHEN $CHIP_FOLDER $CSVfile
    sh Recovery.sh
}

rm -rf $TempFOLDER
mkdir $TempFOLDER
do_Main
