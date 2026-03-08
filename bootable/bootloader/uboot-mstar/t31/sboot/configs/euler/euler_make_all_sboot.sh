#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Euler_sboot_all
CONFIG_PATH=./configs/euler

iConfigNumber=10

# Board 165A
configName[0]=.config.euler.165a_ddr3_128m
outBinPath[0]=${OUT_BIN_PATH}/BD_MST165A_D01B_S
outBinName[0]=sboot.bin

configName[1]=.config.euler.165a_ddr2_64m
outBinPath[1]=${OUT_BIN_PATH}/BD_MST165A_D01B_S
outBinName[1]=sboot_64.bin

# Board 165B
configName[2]=.config.euler.165b_ddr3_128m
outBinPath[2]=${OUT_BIN_PATH}/BD_MST165B_D01B_S
outBinName[2]=sboot.bin

configName[3]=.config.euler.165b_ddr2_64m
outBinPath[3]=${OUT_BIN_PATH}/BD_MST165B_D01B_S
outBinName[3]=sboot_64.bin

# Board 165C
configName[4]=.config.euler.165c_ddr3_128m
outBinPath[4]=${OUT_BIN_PATH}/BD_MST165C_D01B_S
outBinName[4]=sboot.bin

configName[5]=.config.euler.165c_ddr2_64m
outBinPath[5]=${OUT_BIN_PATH}/BD_MST165C_D01B_S
outBinName[5]=sboot_64.bin

# Board 042B
configName[6]=.config.euler.042b_ddr3_128m
outBinPath[6]=${OUT_BIN_PATH}/BD_MST042B_EULER
outBinName[6]=sboot.bin

configName[7]=.config.euler.042b_ddr2_64m
outBinPath[7]=${OUT_BIN_PATH}/BD_MST042B_EULER
outBinName[7]=sboot_64.bin

# Board 047B
configName[8]=.config.euler.047b_ddr3_128m
outBinPath[8]=${OUT_BIN_PATH}/BD_MST047B_EULER
outBinName[8]=sboot.bin

configName[9]=.config.euler.047b_ddr2_64m
outBinPath[9]=${OUT_BIN_PATH}/BD_MST047B_EULER
outBinName[9]=sboot_64.bin


#=======================================================
echo "===================================================="
echo "[Euler] Building all sboot binaries starts ..."
echo "===================================================="

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

echo "===================================================="
echo "[Euler] building sboot binaries is done."
echo "===================================================="


cd configs/euler
