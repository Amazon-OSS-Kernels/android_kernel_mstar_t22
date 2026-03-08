#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Milan_sboot_all
CONFIG_PATH=./configs/milan

iConfigNumber=7

configName[0]=.config.milan.chakra3.214a.64m.1066
outBinPath[0]=${OUT_BIN_PATH}/BD_MST214A_D01A_S_MILAN
outBinName[0]=sboot_64.bin

configName[1]=.config.milan.chakra3.214b.64m.1066
outBinPath[1]=${OUT_BIN_PATH}/BD_MST214B_D01A_S_MILAN
outBinName[1]=sboot_64.bin

configName[2]=.config.milan.chakra3.214b.128m.1600
outBinPath[2]=${OUT_BIN_PATH}/BD_MST214B_D01A_S_MILAN
outBinName[2]=sboot_128.bin

configName[3]=.config.milan.chakra3.214c.128m.1600
outBinPath[3]=${OUT_BIN_PATH}/BD_MST214C_D01A_S_MILAN
outBinName[3]=sboot_128.bin

configName[4]=.config.milan.chakra3.214d.128m.1600
outBinPath[4]=${OUT_BIN_PATH}/BD_MST214D_D01A_S_MILAN
outBinName[4]=sboot_128.bin

configName[5]=.config.milan.chakra3.073b.128m.1600
outBinPath[5]=${OUT_BIN_PATH}/BD_MST073B_10AGS
outBinName[5]=sboot_128.bin

configName[6]=.config.milan.chakra3.074b.64m.1066
outBinPath[6]=${OUT_BIN_PATH}/BD_MST074B_10AGS
outBinName[6]=sboot_64.bin

#=======================================================
echo "===================================================="
echo "Start build Milan all sboot ..."

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
    cp -v  ./out/sboot.bin ${outBinPath[$iBuildIndex]}/${outBinName[$iBuildIndex]}
            
    iBuildIndex=$(($iBuildIndex+1))
done


cd configs/milan
