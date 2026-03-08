#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Marlon_sboot_all
CONFIG_PATH=./configs/marlon

iConfigNumber=11

configName[0]=.config.marlon.chakra3.269b
outBinPath[0]=${OUT_BIN_PATH}/BD_MST269B_D01A_S_MARLON
outBinName[0]=sboot.bin

configName[1]=.config.marlon.chakra3.269c
outBinPath[1]=${OUT_BIN_PATH}/BD_MST269C_D01A_S_MARLON
outBinName[1]=sboot.bin

configName[2]=.config.marlon.chakra3.269e
outBinPath[2]=${OUT_BIN_PATH}/BD_MST269E_D01A_S_MARLON
outBinName[2]=sboot.bin

configName[3]=.config.marlon.chakra3.269f
outBinPath[3]=${OUT_BIN_PATH}/BD_MST269F_D01A_S_MARLON
outBinName[3]=sboot.bin

configName[4]=.config.marlon.chakra3.120b
outBinPath[4]=${OUT_BIN_PATH}/BD_MST120B_10ALS
outBinName[4]=sboot.bin

configName[5]=.config.marlon.chakra3.118b
outBinPath[5]=${OUT_BIN_PATH}/BD_MST118B_10ALU
outBinName[5]=sboot.bin

configName[6]=.config.marlon.chakra3.122b
outBinPath[6]=${OUT_BIN_PATH}/BD_MST122B_10ALBT
outBinName[6]=sboot.bin

configName[7]=.config.marlon.chakra3.127b
outBinPath[7]=${OUT_BIN_PATH}/BD_MST127B_10ALSA
outBinName[7]=sboot.bin

configName[8]=.config.marlon.chakra3.269g
outBinPath[8]=${OUT_BIN_PATH}/BD_MST269G_D01A_S_MARLON
outBinName[8]=sboot.bin

configName[9]=.config.marlon.chakra3.158b
outBinPath[9]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354_MARLON
outBinName[9]=sboot.bin

configName[10]=.config.marlon.chakra3.159b
outBinPath[10]=${OUT_BIN_PATH}/BD_MST159B_10ALBCAT_18372_MARLON
outBinName[10]=sboot.bin

#=======================================================
echo "===================================================="
echo "Start build Marlon all sboot ..."

cd ../..
pwd

# Create root bin folder
rm -v -rf ${OUT_BIN_PATH}
mkdir -v ${OUT_BIN_PATH}

#===================================================

iBuildIndex=0

while [ "$iBuildIndex" != "$iConfigNumber" ]
do
    echo "-----------------------------------------"
    echo "[iBuildIndex=$iBuildIndex]"

    # Get config name
    ConfigName=${configName[$iBuildIndex]}
    echo "ConfigName=$ConfigName"

    # delete current .config
    #rm -rf .config

    # Copy config
    cp -v ${CONFIG_PATH}/${ConfigName} ./scripts/defconfig
    make defconfig

    # make
    rm -rf ./out
    make clean;
    make

    # Move bin ...
    #cp -v ./out/sboot.bin ${OUT_BIN_PATH}
    mkdir -v ${outBinPath[$iBuildIndex]}
    cp -v  ./out/sboot.bin ${outBinPath[$iBuildIndex]}/${outBinName[$iBuildIndex]}

    iBuildIndex=$(($iBuildIndex+1))
done


cd configs/marlon
