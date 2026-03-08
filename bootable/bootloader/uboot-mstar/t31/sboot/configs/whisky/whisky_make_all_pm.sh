#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Whisky_PM_all
CONFIG_PATH=./configs/whisky

iConfigNumber=7

# Board 233A
configName[0]=.config.whisky.233a.pm
outBinPath[0]=${OUT_BIN_PATH}/BD_MST233A_D01A_S
outBinName[0]=PM.bin

# Board 233B
configName[1]=.config.whisky.233b.pm
outBinPath[1]=${OUT_BIN_PATH}/BD_MST233B_D01A_S
outBinName[1]=PM.bin

# Board 223B
configName[2]=.config.whisky.223b.pm
outBinPath[2]=${OUT_BIN_PATH}/BD_MST223B_D01A_S
outBinName[2]=PM.bin

# Board 090B
configName[3]=.config.whisky.090b.pm
outBinPath[3]=${OUT_BIN_PATH}/BD_MST090B_10ANU_15213
outBinName[3]=PM.bin

# Board 089B
configName[4]=.config.whisky.089b.pm
outBinPath[4]=${OUT_BIN_PATH}/BD_MST089B_10ANB_15213
outBinName[4]=PM.bin

# Board 269G
configName[5]=.config.whisky.269g.pm
outBinPath[5]=${OUT_BIN_PATH}/BD_MST269G_D01A_S
outBinName[5]=PM.bin

# Board 158B
configName[6]=.config.whisky.158b.pm
outBinPath[6]=${OUT_BIN_PATH}/BD_MST158B_10ALWAT_18354
outBinName[6]=PM.bin

#=======================================================
echo "===================================================="
echo "Start build Whisky all PM binaries ..."

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
