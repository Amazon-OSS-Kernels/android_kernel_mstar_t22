#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Nasa_sboot_all
CONFIG_PATH=./configs/nasa

iConfigNumber=10

# Board 195A
configName[0]=.config.nasa.195a_ddr2
outBinPath[0]=${OUT_BIN_PATH}/BD_MST195A_D01A_S_NASA
outBinName[0]=sboot_64.bin
outBinName2[0]=sboot_32.bin

# Board 195B
configName[1]=.config.nasa.195b_ddr2
outBinPath[1]=${OUT_BIN_PATH}/BD_MST195B_D01A_S_NASA
outBinName[1]=sboot_64.bin
outBinName2[1]=sboot_32.bin

# Board 195C
configName[2]=.config.nasa.195c_ddr2
outBinPath[2]=${OUT_BIN_PATH}/BD_MST195C_D01A_S_NASA
outBinName[2]=sboot_64.bin
outBinName2[2]=sboot_32.bin

# Board 186A
configName[3]=.config.nasa.186a_ddr2
outBinPath[3]=${OUT_BIN_PATH}/BD_MST186A_D01A_S_NASA
outBinName[3]=sboot_64.bin
outBinName2[3]=sboot_32.bin

# Board 186E
configName[4]=.config.nasa.186e_ddr2
outBinPath[4]=${OUT_BIN_PATH}/BD_MST186E_D01A_S_NASA
outBinName[4]=sboot_64.bin
outBinName2[4]=sboot_32.bin

# Board 233B
configName[5]=.config.nasa.233b_ddr2
outBinPath[5]=${OUT_BIN_PATH}/BD_MST233B_D01A_S_NASA
outBinName[5]=sboot_64.bin
outBinName2[5]=sboot_32.bin

# Board 060B
configName[6]=.config.nasa.060b_ddr2
outBinPath[6]=${OUT_BIN_PATH}/BD_MST060B_10AZBT_13502
outBinName[6]=sboot_64.bin
outBinName2[6]=sboot_32.bin

# Board 061B
configName[7]=.config.nasa.061b_ddr2
outBinPath[7]=${OUT_BIN_PATH}/BD_MST061B_10AZU_14014
outBinName[7]=sboot_64.bin
outBinName2[7]=sboot_32.bin

# Board 269G
configName[8]=.config.nasa.269g_ddr2
outBinPath[8]=${OUT_BIN_PATH}/BD_MST269G_D01A_S_NASA
outBinName[8]=sboot_64.bin
outBinName2[8]=sboot_32.bin

# Board 158B
configName[9]=.config.nasa.158b_ddr2
outBinPath[9]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[9]=sboot_64.bin
outBinName2[9]=sboot_32.bin

#=======================================================
echo "===================================================="
echo "[Nasa] Building all sboot binaries starts ..."
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
    cp -v  ./out/*.bin ${outBinPath[$iBuildIndex]}/${outBinName2[$iBuildIndex]}
            
    iBuildIndex=$(($iBuildIndex+1))
done

echo "===================================================="
echo "[Nasa] building sboot binaries is done."
echo "===================================================="

cd configs/nasa
