# How to use:
#@sh	gen_TBL.sh  KSUM
# arg1 = KSUM
# arg2 = AUTHEN status
KSUM=$1
AUTHEN=$2
CHIP=$3
#INPUTCONFIG=$4
dos2unix ./src/$CHIP/usercfg.sh
source  ./src/$CHIP/usercfg.sh
# **********************************************
# Environment
# **********************************************
TempFOLDER=OnebinTemp
CSVfile=$TempFOLDER/configfile.csv
INPUTCONFIG=.config
target_defConfig=scripts/defconfig


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
	if [[ $CHIP == "M7221" || $CHIP == "M7322" ]]; then
        for ((index=0; index<${#Chip_config[@]}; index++)); do
                config=${Chip_config[$index]}
                echo $config >>$CSVfile
        done
	fi


}
function do_Main()
{
    do_backup_config
    echo $CHIP
    genConfig
    sh gen_TBL_v2.sh $KSUM $AUTHEN $CHIP $CSVfile
    sh Recovery.sh
}

rm -rf $TempFOLDER
mkdir $TempFOLDER
do_Main
