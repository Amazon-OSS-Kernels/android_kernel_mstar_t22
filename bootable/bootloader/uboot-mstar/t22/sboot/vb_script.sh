ORIG_PATH=$PATH

make distclean >/dev/null 2>&1
echo "--- configs/M5621/.config.M5621.supernova.285a.rom.emmc MBoot ---"
cp configs/M5621/.config.M5621.supernova.285a.rom.emmc scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/M5621/.config.M5621.supernova.285a.rom.emmc MBoot build failed ---"
    exit 1
fi
echo "--- configs/M5621/.config.M5621.supernova.285a.rom.emmc MBoot Done ---"

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
echo "--- configs/macan/.config.macan.supernova.104b.rom_nand.32bit.kernel.tee.faststr MBoot ---"
cp configs/macan/.config.macan.supernova.104b.rom_nand.32bit.kernel.tee.faststr scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/macan/.config.macan.supernova.104b.rom_nand.32bit.kernel.tee.faststr MBoot build failed ---"
    exit 1
fi
echo "--- configs/macan/.config.macan.supernova.104b.rom_nand.32bit.kernel.tee.faststr MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/macan/.config.macan.android.104b.rom_emmc.tee MBoot ---"
cp configs/macan/.config.macan.android.104b.rom_emmc.tee scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/macan/.config.macan.android.104b.rom_emmc.tee MBoot build failed ---"
    exit 1
fi
echo "--- configs/macan/.config.macan.android.104b.rom_emmc.tee MBoot Done ---"

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
echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.optee.kernel.onebin.amazon MBoot ---"
cp configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.optee.kernel.onebin.amazon scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.optee.kernel.onebin.amazon MBoot build failed ---"
    exit 1
fi
echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.optee.kernel.onebin.amazon MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.optee.kernel MBoot ---"
cp configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.optee.kernel scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.optee.kernel MBoot build failed ---"
    exit 1
fi
echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.optee.kernel MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.tee.kernel MBoot ---"
cp configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.tee.kernel scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.tee.kernel MBoot build failed ---"
    exit 1
fi
echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.tee.kernel MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.tee.kernel MBoot ---"
cp configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.tee.kernel scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.tee.kernel MBoot build failed ---"
    exit 1
fi
echo "--- configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.tee.kernel MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.optee.kernel MBoot ---"
cp configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.optee.kernel scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.optee.kernel MBoot build failed ---"
    exit 1
fi
echo "--- configs/maxim/.config.maxim.android.107b.rom_emmc.64bit.optee.kernel MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/mustang/.config.mustang.supernova.124b.rom_nand.tee.fstr MBoot ---"
cp configs/mustang/.config.mustang.supernova.124b.rom_nand.tee.fstr scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/mustang/.config.mustang.supernova.124b.rom_nand.tee.fstr MBoot build failed ---"
    exit 1
fi
echo "--- configs/mustang/.config.mustang.supernova.124b.rom_nand.tee.fstr MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.kernel MBoot ---"
cp configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.kernel scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.kernel MBoot build failed ---"
    exit 1
fi
echo "--- configs/maserati/.config.maserati.android.095d.rom_emmc.64bit.kernel MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/mooney/.config.mooney.android.116b.rom_emmc.64bit.tee.kernel MBoot ---"
cp configs/mooney/.config.mooney.android.116b.rom_emmc.64bit.tee.kernel scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/mooney/.config.mooney.android.116b.rom_emmc.64bit.tee.kernel MBoot build failed ---"
    exit 1
fi
echo "--- configs/mooney/.config.mooney.android.116b.rom_emmc.64bit.tee.kernel MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.kernel.raptors.onebin MBoot ---"
cp configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.kernel.raptors.onebin scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.kernel.raptors.onebin MBoot build failed ---"
    exit 1
fi
echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.kernel.raptors.onebin MBoot Done ---"

make distclean >/dev/null 2>&1
echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.tee.kernel.onebin.amazon MBoot ---"
cp configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.tee.kernel.onebin.amazon scripts/defconfig
make defconfig > /dev/null 2>%1
make clean
make
if [ $? != 0 ]; then
    echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.tee.kernel.onebin.amazon MBoot build failed ---"
    exit 1
fi
echo "--- configs/maserati/.config.maserati.android.098d.rom_emmc.64bit.tee.kernel.onebin.amazon MBoot Done ---"

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

echo "------ All done ------"
