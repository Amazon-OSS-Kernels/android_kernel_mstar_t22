#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Whisky_sboot_all
CONFIG_PATH=./configs/whisky

iConfigNumber=14

# Board 233A
configName[0]=.config.whisky.233a_ddr3_128m
outBinPath[0]=${OUT_BIN_PATH}/BD_MST233A_D01A_S
outBinName[0]=sboot.bin

configName[1]=.config.whisky.233a_ddr2_64m
outBinPath[1]=${OUT_BIN_PATH}/BD_MST233A_D01A_S
outBinName[1]=sboot_64.bin

# Board 233B
configName[2]=.config.whisky.233b_ddr3_128m
outBinPath[2]=${OUT_BIN_PATH}/BD_MST233B_D01A_S
outBinName[2]=sboot.bin

configName[3]=.config.whisky.233b_ddr2_64m
outBinPath[3]=${OUT_BIN_PATH}/BD_MST233B_D01A_S
outBinName[3]=sboot_64.bin

# Board 223B
configName[4]=.config.whisky.223b_ddr3_128m
outBinPath[4]=${OUT_BIN_PATH}/BD_MST223B_D01A_S
outBinName[4]=sboot.bin

configName[5]=.config.whisky.223b_ddr2_64m
outBinPath[5]=${OUT_BIN_PATH}/BD_MST223B_D01A_S
outBinName[5]=sboot_64.bin

# Board 090B
configName[6]=.config.whisky.090b_ddr3_128m
outBinPath[6]=${OUT_BIN_PATH}/BD_MST090B_10ANU_15213
outBinName[6]=sboot.bin

configName[7]=.config.whisky.090b_ddr2_64m
outBinPath[7]=${OUT_BIN_PATH}/BD_MST090B_10ANU_15213
outBinName[7]=sboot_64.bin

# Board 089B
configName[8]=.config.whisky.089b_ddr3_128m
outBinPath[8]=${OUT_BIN_PATH}/BD_MST089B_10ANB_15213
outBinName[8]=sboot.bin

configName[9]=.config.whisky.089b_ddr2_64m
outBinPath[9]=${OUT_BIN_PATH}/BD_MST089B_10ANB_15213
outBinName[9]=sboot_64.bin

# Board 269G
configName[10]=.config.whisky.269g_ddr3_128m
outBinPath[10]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[10]=sboot.bin

configName[11]=.config.whisky.269g_ddr2_64m
outBinPath[11]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[11]=sboot_64.bin

# Board 158B
configName[12]=.config.whisky.158b_ddr3_128m
outBinPath[12]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[12]=sboot.bin

configName[13]=.config.whisky.158b_ddr2_64m
outBinPath[13]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[13]=sboot_64.bin

#=======================================================
echo "===================================================="
echo "Start build Whisky all sboot/PM binaries ..."

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


cd configs/whisky
