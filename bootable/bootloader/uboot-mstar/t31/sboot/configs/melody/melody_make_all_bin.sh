#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Melody_sbootPM_all
CONFIG_PATH=./configs/melody

iConfigNumber=24

########################
#
# Generate sboot/PM binaries
#
########################
# Board 242A
configName[0]=.config.melody.242a_ddr2_64m
outBinPath[0]=${OUT_BIN_PATH}/BD_MST242A_D01A_S
outBinName[0]=sboot_64.bin

configName[1]=.config.melody.242a.pm
outBinPath[1]=${OUT_BIN_PATH}/BD_MST242A_D01A_S
outBinName[1]=PM.bin

# Board 242B
configName[2]=.config.melody.242b_ddr2_64m
outBinPath[2]=${OUT_BIN_PATH}/BD_MST242B_D01A_S
outBinName[2]=sboot_64.bin

configName[3]=.config.melody.242b.pm
outBinPath[3]=${OUT_BIN_PATH}/BD_MST242B_D01A_S
outBinName[3]=PM.bin

# Board 242C
configName[4]=.config.melody.242c_ddr2_64m
outBinPath[4]=${OUT_BIN_PATH}/BD_MST242C_D01A_S
outBinName[4]=sboot_64.bin

configName[5]=.config.melody.242c.pm
outBinPath[5]=${OUT_BIN_PATH}/BD_MST242C_D01A_S
outBinName[5]=PM.bin

# Board 242D
configName[6]=.config.melody.242d_ddr2_64m
outBinPath[6]=${OUT_BIN_PATH}/BD_MST242D_D01A_S
outBinName[6]=sboot_64.bin

configName[7]=.config.melody.242d.pm
outBinPath[7]=${OUT_BIN_PATH}/BD_MST242D_D01A_S
outBinName[7]=PM.bin

# Board 094B
configName[8]=.config.melody.094b_ddr2_64m
outBinPath[8]=${OUT_BIN_PATH}/BD_MST094B_10ARJ_15374
outBinName[8]=sboot_64.bin

configName[9]=.config.melody.094b.pm
outBinPath[9]=${OUT_BIN_PATH}/BD_MST094B_10ARJ_15374
outBinName[9]=PM.bin

# Board 094B_20ARJ_16234
configName[10]=.config.melody.094B_20ARJ_16234_ddr2_64m
outBinPath[10]=${OUT_BIN_PATH}/BD_MST094B_20ARJ_16234
outBinName[10]=sboot_64.bin

configName[11]=.config.melody.094B_20ARJ_16234.pm
outBinPath[11]=${OUT_BIN_PATH}/BD_MST094B_20ARJ_16234
outBinName[11]=PM.bin

# Board 096B
configName[12]=.config.melody.096b_ddr2_64m
outBinPath[12]=${OUT_BIN_PATH}/BD_MST096B_10ARU_15384
outBinName[12]=sboot_64.bin

configName[13]=.config.melody.096b.pm
outBinPath[13]=${OUT_BIN_PATH}/BD_MST096B_10ARU_15384
outBinName[13]=PM.bin

# Board 097B
configName[14]=.config.melody.097b_ddr2_64m
outBinPath[14]=${OUT_BIN_PATH}/BD_MST097B_10ARB_15384
outBinName[14]=sboot_64.bin

configName[15]=.config.melody.097b.pm
outBinPath[15]=${OUT_BIN_PATH}/BD_MST097B_10ARB_15384
outBinName[15]=PM.bin

# Board 106B
configName[16]=.config.melody.106b_ddr2_64m
outBinPath[16]=${OUT_BIN_PATH}/BD_MST106B_10ARU_16115
outBinName[16]=sboot_64.bin

configName[17]=.config.melody.106b.pm
outBinPath[17]=${OUT_BIN_PATH}/BD_MST106B_10ARU_16115
outBinName[17]=PM.bin

# Board 143B
configName[18]=.config.melody.143b_ddr2_64m
outBinPath[18]=${OUT_BIN_PATH}/BD_MST143B_10ARU_17474
outBinName[18]=sboot_64.bin

configName[19]=.config.melody.143b.pm
outBinPath[19]=${OUT_BIN_PATH}/BD_MST143B_10ARU_17474
outBinName[19]=PM.bin

# Board 269G
configName[20]=.config.melody.269g_ddr2_64m
outBinPath[20]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[20]=sboot_64.bin

configName[21]=.config.melody.269g.pm
outBinPath[21]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[21]=PM.bin

# Board 158B
configName[22]=.config.melody.158b_ddr2_64m
outBinPath[22]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[22]=sboot_64.bin

configName[23]=.config.melody.158b.pm
outBinPath[23]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[23]=PM.bin

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

    iBuildIndex=$(($iBuildIndex+1))
done


cd configs/melody
