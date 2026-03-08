#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Melody_PM_all
CONFIG_PATH=./configs/melody

iConfigNumber=12

# Board 242A
configName[0]=.config.melody.242a.pm
outBinPath[0]=${OUT_BIN_PATH}/BD_MST242A_D01A_S
outBinName[0]=PM.bin

# Board 242B
configName[1]=.config.melody.242b.pm
outBinPath[1]=${OUT_BIN_PATH}/BD_MST242B_D01A_S
outBinName[1]=PM.bin

# Board 242C
configName[2]=.config.melody.242c.pm
outBinPath[2]=${OUT_BIN_PATH}/BD_MST242C_D01A_S
outBinName[2]=PM.bin

# Board 242D
configName[3]=.config.melody.242d.pm
outBinPath[3]=${OUT_BIN_PATH}/BD_MST242D_D01A_S
outBinName[3]=PM.bin

# Board 094B
configName[4]=.config.melody.094b.pm
outBinPath[4]=${OUT_BIN_PATH}/BD_MST094B_10ARJ_15374
outBinName[4]=PM.bin

# Board 094B_20ARJ_16234
configName[5]=.config.melody.094B_20ARJ_16234.pm
outBinPath[5]=${OUT_BIN_PATH}/BD_MST094B_20ARJ_16234
outBinName[5]=PM.bin

# Board 096B
configName[6]=.config.melody.096b.pm
outBinPath[6]=${OUT_BIN_PATH}/BD_MST096B_10ARU_15384
outBinName[6]=PM.bin

# Board 097B
configName[7]=.config.melody.097b.pm
outBinPath[7]=${OUT_BIN_PATH}/BD_MST097B_10ARB_15384
outBinName[7]=PM.bin

# Board 106B
configName[8]=.config.melody.106b.pm
outBinPath[8]=${OUT_BIN_PATH}/BD_MST106B_10ARU_16115
outBinName[8]=PM.bin

# Board 143B
configName[9]=.config.melody.143b.pm
outBinPath[9]=${OUT_BIN_PATH}/BD_MST143B_10ARU_17474
outBinName[9]=PM.bin

# Board 269G
configName[10]=.config.melody.269g.pm
outBinPath[10]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[10]=PM.bin

# Board 158B
configName[11]=.config.melody.158b.pm
outBinPath[11]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[11]=PM.bin

#=======================================================
echo "===================================================="
echo "Start build Melody all PM binaries ..."

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
