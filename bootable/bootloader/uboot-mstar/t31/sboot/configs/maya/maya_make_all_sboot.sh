#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Maya_sboot_all
CONFIG_PATH=./configs/maya

iConfigNumber=3

# Board 222B
configName[0]=.config.maya.222b
outBinPath[0]=${OUT_BIN_PATH}/BD_MST222B_D01A_S
outBinName[0]=sboot.bin

# Board 222F
configName[1]=.config.maya.222f
outBinPath[1]=${OUT_BIN_PATH}/BD_MST222F_D01A_S
outBinName[1]=sboot.bin

# Board 084B
configName[2]=.config.maya.084b
outBinPath[2]=${OUT_BIN_PATH}/BD_MST084B_10AHB_15043
outBinName[2]=sboot.bin


#=======================================================
echo "===================================================="
echo "Start build Maya all sboot/PM binaries ..."

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


cd configs/maya
