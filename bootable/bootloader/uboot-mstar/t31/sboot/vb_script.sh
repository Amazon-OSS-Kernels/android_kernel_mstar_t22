ORIG_PATH=$PATH

make distclean >/dev/null 2>&1
echo "--- configs/M7621/.config.m7621.supernova.288a.rom_emmc.32bit.tee.kdrv.ddr3 MBoot ---"
cp configs/M7621/.config.m7621.supernova.288a.rom_emmc.32bit.tee.kdrv.ddr3 scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M7621/.config.m7621.supernova.288a.rom_emmc.32bit.tee.kdrv.ddr3 MBoot build failed ---"
    exit 1
fi
echo "--- configs/M7621/.config.m7621.supernova.288a.rom_emmc.32bit.tee.kdrv.ddr3 MBoot Done ---"


make distclean >/dev/null 2>&1
echo "--- configs/M7622/.config.m7622.supernova.302a.rom_emmc.64bit.tee.kdrv.ddr3 MBoot ---"
cp configs/M7622/.config.m7622.supernova.302a.rom_emmc.64bit.tee.kdrv.ddr3 scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M7622/.config.m7622.supernova.302a.rom_emmc.64bit.tee.kdrv.ddr3 MBoot build failed ---"
    exit 1
fi
echo "--- configs/M7622/.config.m7622.supernova.302a.rom_emmc.64bit.tee.kdrv.ddr3 MBoot Done ---"


make distclean >/dev/null 2>&1
echo "--- configs/M7632/.config.m7632.linux.MTV19006_MT5871.rom_emmc.optee.32bit MBoot ---"
cp configs/M7632/.config.m7632.linux.MTV19006_MT5871.rom_emmc.optee.32bit scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M7632/.config.m7632.linux.MTV19006_MT5871.rom_emmc.optee.32bit MBoot build failed ---"
    exit 1
fi
echo "--- configs/M7632/.config.m7632.linux.MTV19006_MT5871.rom_emmc.optee.32bit MBoot Done ---"


make distclean >/dev/null 2>&1
echo "--- configs/mazda/.config.mazda.supernova.137b.rom_nand.32bit.kernel.tee.1x1"
cp configs/mazda/.config.mazda.supernova.137b.rom_nand.32bit.kernel.tee.1x1 scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/mazda/.config.mazda.supernova.137b.rom_nand.32bit.kernel.tee.1x1 MBoot build failed ---"
    exit 1
fi
echo "--- configs/mazda/.config.mazda.supernova.137b.rom_nand.32bit.kernel.tee.1x1 MBoot Done ---"


make distclean >/dev/null 2>&1
echo "--- configs/M7221/.config.m7221.android.140B.rom_emmc_64bit.optee.kdrv MBoot ---"
cp configs/M7221/.config.m7221.android.140B.rom_emmc_64bit.optee.kdrv scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M7221/.config.m7221.android.140B.rom_emmc_64bit.optee.kdrv MBoot build failed ---"
    exit 1
fi
echo "--- configs/M7221/.config.m7221.android.140B.rom_emmc_64bit.optee.kdrv MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/M7221/.config.m7221.android.295a.rom_emmc_64bit.optee.kdrv MBoot ---"
cp configs/M7221/.config.m7221.android.295a.rom_emmc_64bit.optee.kdrv scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M7221/.config.m7221.android.295a.rom_emmc_64bit.optee.kdrv MBoot build failed ---"
    exit 1
fi
echo "--- configs/M7221/.config.m7221.android.295a.rom_emmc_64bit.optee.kdrv MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/mainz/.config.mainz.android.125b.rom_emmc.64bit.kernel.optee MBoot ---"
cp configs/mainz/.config.mainz.android.125b.rom_emmc.64bit.kernel.optee scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/mainz/.config.mainz.android.125b.rom_emmc.64bit.kernel.optee MBoot build failed ---"
    exit 1
fi
echo "--- configs/mainz/.config.mainz.android.125b.rom_emmc.64bit.kernel.optee MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/M5621/.config.M5621.android.151b.rom.emmc.32bit.kernel.optee.MTK.avb MBoot ---"
cp configs/M5621/.config.M5621.android.151b.rom.emmc.32bit.kernel.optee.MTK.avb scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M5621/.config.M5621.android.151b.rom.emmc.32bit.kernel.optee.MTK.avb MBoot build failed ---"
    exit 1
fi
echo "--- configs/M5621/.config.M5621.android.151b.rom.emmc.32bit.kernel.optee.MTK.avb MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/M7322/.config.m7322.android.MST148B_10NQAT_18115.rom_emmc_64bit.optee.kdrv.asymetric_UMA.avb MBoot ---"
cp configs/M7322/.config.m7322.android.MST148B_10NQAT_18115.rom_emmc_64bit.optee.kdrv.asymetric_UMA.avb scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M7322/.config.m7322.android.MST148B_10NQAT_18115.rom_emmc_64bit.optee.kdrv.asymetric_UMA.avb MBoot build failed ---"
    exit 1
fi
echo "--- configs/M7322/.config.m7322.android.MST148B_10NQAT_18115.rom_emmc_64bit.optee.kdrv.asymetric_UMA.avb MBoot Done ---"

echo "------ All done ------"
