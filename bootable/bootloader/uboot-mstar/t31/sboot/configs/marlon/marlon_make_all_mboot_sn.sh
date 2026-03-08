#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Marlon_mboot_sn
CONFIG_PATH=./configs/marlon

iConfigNumber=9

configName[0]=.config.marlon.supernova.269b.rom.nand
outBinPath[0]=${OUT_BIN_PATH}/marlon.269b.rom.nand

configName[1]=.config.marlon.supernova.269c.rom.nand
outBinPath[1]=${OUT_BIN_PATH}/marlon.269c.rom.nand

configName[2]=.config.marlon.supernova.269f.rom.nand
outBinPath[2]=${OUT_BIN_PATH}/marlon.269f.rom.nand

configName[3]=.config.marlon.supernova.120b.rom.nand
outBinPath[3]=${OUT_BIN_PATH}/marlon.120b.rom.nand

configName[4]=.config.marlon.supernova.122b.rom.nand
outBinPath[4]=${OUT_BIN_PATH}/marlon.122b.rom.nand

configName[5]=.config.marlon.supernova.127b.rom.nand
outBinPath[5]=${OUT_BIN_PATH}/marlon.127b.rom.nand

configName[6]=.config.marlon.supernova.120b.rom.nand.str
outBinPath[6]=${OUT_BIN_PATH}/marlon.120b.rom.nand.str

configName[7]=.config.marlon.supernova.122b.rom.nand.str
outBinPath[7]=${OUT_BIN_PATH}/marlon.122b.rom.nand.str

configName[8]=.config.marlon.supernova.127b.rom.nand.str
outBinPath[8]=${OUT_BIN_PATH}/marlon.127b.rom.nand.str

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
    mkdir -v ${outBinPath[$iBuildIndex]}
    cp -v  ./out/* ${outBinPath[$iBuildIndex]}

    iBuildIndex=$(($iBuildIndex+1))
done


cd configs/marlon
