#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Eden_sboot_all
CONFIG_PATH=./configs/eden

iConfigNumber=13

# Board 165A
configName[0]=.config.eden.165a.d01a_ddr2_64m
outBinPath[0]=${OUT_BIN_PATH}/BD_MST165A_D01A_S
outBinName[0]=sboot_64.bin

configName[1]=.config.eden.165a.d01b_ddr2_64m
outBinPath[1]=${OUT_BIN_PATH}/BD_MST165A_D01B_S
outBinName[1]=sboot_64.bin

# Board 165B
configName[2]=.config.eden.165b.d01a_ddr2_64m
outBinPath[2]=${OUT_BIN_PATH}/BD_MST165B_D01A_S
outBinName[2]=sboot_64.bin

# Board 165C
configName[3]=.config.eden.165c.d01a_ddr2_64m
outBinPath[3]=${OUT_BIN_PATH}/BD_MST165C_D01A_S
outBinName[3]=sboot_64.bin

configName[4]=.config.eden.165c.d01b_ddr2_64m
outBinPath[4]=${OUT_BIN_PATH}/BD_MST165C_D01B_S
outBinName[4]=sboot_64.bin

# Board 202A
configName[5]=.config.eden.202a.d01a_ddr2_64m
outBinPath[5]=${OUT_BIN_PATH}/BD_MST202A_D01A_S
outBinName[5]=sboot_64.bin

# Board 165C2(197A)
configName[6]=.config.eden.165c2.d01b_ddr2_64m
outBinPath[6]=${OUT_BIN_PATH}/BD_MST165C2_D01B_S
outBinName[6]=sboot_64.bin

# Board 043B
configName[7]=.config.eden.043b.10alb_ddr2_64m
outBinPath[7]=${OUT_BIN_PATH}/BD_MST043B_10ALB_12351
outBinName[7]=sboot_64.bin

configName[8]=.config.eden.043b.10blb_ddr2_64m
outBinPath[8]=${OUT_BIN_PATH}/BD_MST043B_10BLB_12413
outBinName[8]=sboot_64.bin

# Board 042B
configName[9]=.config.eden.042b.10blu_ddr2_64m
outBinPath[9]=${OUT_BIN_PATH}/BD_MST042B_10BLU_12423
outBinName[9]=sboot_64.bin

configName[10]=.config.eden.042b.10blu_ddr2_32m
outBinPath[10]=${OUT_BIN_PATH}/BD_MST042B_10BLU_12423_32
outBinName[10]=sboot_32.bin

# Board 044B
configName[11]=.config.eden.044b.10bla_ddr2_64m
outBinPath[11]=${OUT_BIN_PATH}/BD_MST044B_10BLA_12423
outBinName[11]=sboot_64.bin

configName[12]=.config.eden.044b.10bla_ddr2_32m
outBinPath[12]=${OUT_BIN_PATH}/BD_MST044B_10BLA_12423_32
outBinName[12]=sboot_32.bin


#=======================================================
echo "===================================================="
echo "[Eden] Building all sboot binaries starts ..."
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
echo "[Eden] building sboot binaries is done."
echo "===================================================="


cd configs/eden
