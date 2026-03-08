#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Maya_sbootPM_all
CONFIG_PATH=./configs/maya

iConfigNumber=6

# Board 222B
configName[0]=.config.maya.222b
outBinPath[0]=${OUT_BIN_PATH}/BD_MST222B_D01A_S
outBinName[0]=sboot.bin

configName[1]=.config.maya.222b.pm
outBinPath[1]=${OUT_BIN_PATH}/BD_MST222B_D01A_S
outBinName[1]=PM.bin

# Board 222F
configName[2]=.config.maya.222f
outBinPath[2]=${OUT_BIN_PATH}/BD_MST222F_D01A_S
outBinName[2]=sboot.bin

configName[3]=.config.maya.222f.pm
outBinPath[3]=${OUT_BIN_PATH}/BD_MST222F_D01A_S
outBinName[3]=PM.bin

# Board 084B
configName[4]=.config.maya.084b
outBinPath[4]=${OUT_BIN_PATH}/BD_MST084B_10AHB_15043
outBinName[4]=sboot.bin

configName[5]=.config.maya.084b
outBinPath[5]=${OUT_BIN_PATH}/BD_MST084B_10AHB_15043
outBinName[5]=PM.bin


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
