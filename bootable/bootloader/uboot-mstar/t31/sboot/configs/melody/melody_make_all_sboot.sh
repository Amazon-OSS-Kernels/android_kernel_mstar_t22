#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Melody_sboot_all
CONFIG_PATH=./configs/melody

iConfigNumber=12

# Board 242A
configName[0]=.config.melody.242a_ddr2_64m
outBinPath[0]=${OUT_BIN_PATH}/BD_MST242A_D01A_S
outBinName[0]=sboot_64.bin

# Board 242B
configName[1]=.config.melody.242b_ddr2_64m
outBinPath[1]=${OUT_BIN_PATH}/BD_MST242B_D01A_S
outBinName[1]=sboot_64.bin

# Board 242C
configName[2]=.config.melody.242c_ddr2_64m
outBinPath[2]=${OUT_BIN_PATH}/BD_MST242C_D01A_S
outBinName[2]=sboot_64.bin

# Board 242D
configName[3]=.config.melody.242d_ddr2_64m
outBinPath[3]=${OUT_BIN_PATH}/BD_MST242D_D01A_S
outBinName[3]=sboot_64.bin

# Board 094B
configName[4]=.config.melody.094b_ddr2_64m
outBinPath[4]=${OUT_BIN_PATH}/BD_MST094B_10ARJ_15374
outBinName[4]=sboot_64.bin

# Board 094B_20ARJ_16234
configName[5]=.config.melody.094B_20ARJ_16234_ddr2_64m
outBinPath[5]=${OUT_BIN_PATH}/BD_MST094B_20ARJ_16234
outBinName[5]=sboot_64.bin

# Board 096B
configName[6]=.config.melody.096b_ddr2_64m
outBinPath[6]=${OUT_BIN_PATH}/BD_MST096B_10ARU_15384
outBinName[6]=sboot_64.bin

# Board 097B
configName[7]=.config.melody.097b_ddr2_64m
outBinPath[7]=${OUT_BIN_PATH}/BD_MST097B_10ARB_15384
outBinName[7]=sboot_64.bin

# Board 106B
configName[8]=.config.melody.106b_ddr2_64m
outBinPath[8]=${OUT_BIN_PATH}/BD_MST106B_10ARU_16115
outBinName[8]=sboot_64.bin

# Board 143B
configName[9]=.config.melody.143b_ddr2_64m
outBinPath[9]=${OUT_BIN_PATH}/BD_MST143B_10ARU_17474
outBinName[9]=sboot_64.bin

# Board 269G
configName[10]=.config.melody.269g_ddr2_64m
outBinPath[10]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[10]=sboot_64.bin

# Board 158B
configName[10]=.config.melody.158b_ddr2_64m
outBinPath[10]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[10]=sboot_64.bin

#=======================================================
echo "===================================================="
echo "Start build Melody all sboot/PM binaries ..."

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
    cp -v  ./out/*.bin ${outBinPath[$iBuildIndex]}/${outBinName[$iBuildIndex]}
    cp -v  ./out/*.bin ${outBinPath[$iBuildIndex]}/sboot_32.bin

    iBuildIndex=$(($iBuildIndex+1))
done


cd configs/melody
