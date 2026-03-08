#export CC=/usr/bin/gcc

OUT_BIN_PATH=./Bin_Euler_PM_all
CONFIG_PATH=./configs/euler

iConfigNumber=5

# Board 165A
configName[0]=.config.euler.165a.pm
outBinPath[0]=${OUT_BIN_PATH}/BD_MST165A_D01B_S
outBinName[0]=PM.bin

# Board 165B
configName[1]=.config.euler.165b.pm
outBinPath[1]=${OUT_BIN_PATH}/BD_MST165B_D01B_S
outBinName[1]=PM.bin

# Board 165C
configName[2]=.config.euler.165c.pm
outBinPath[2]=${OUT_BIN_PATH}/BD_MST165C_D01B_S
outBinName[2]=PM.bin

# Board 042B
configName[3]=.config.euler.042b.pm
outBinPath[3]=${OUT_BIN_PATH}/BD_MST042B_EULER
outBinName[3]=PM.bin

# Board 047B
configName[4]=.config.euler.047b.pm
outBinPath[4]=${OUT_BIN_PATH}/BD_MST047B_EULER
outBinName[4]=PM.bin


#=======================================================
echo "===================================================="
echo "[Euler] Building all PM binaries starts ..."
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
echo "[Euler] building PM binaries is done."
echo "===================================================="

cd configs/euler
