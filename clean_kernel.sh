#!/bin/bash
<<<<<<< HEAD
if [ "${1}" = "mrproper" ] ; then
	make mrproper -j$(($(cat /proc/cpuinfo | grep "^processor" | wc -l)*8))
fi
git reset --hard
git ls-files . --ignored --exclude-standard --others --directory | while read file; do echo $file; rm -rf $file; done
git ls-files . --exclude-standard --others --directory | while read file; do echo $file; rm -rf $file; done
cp defconfig .config
=======

############################################################################
#kernel build Script
#
###############################################################################
# 2011-10-24 effectivesky : modified
# 2010-12-29 allydrop     : created
###############################################################################
#export ARCH=arm
#export PATH=$(pwd)/../../toolchain_arm-eabi-4.6/arm-eabi-4.6/bin:$PATH
#export CROSS_COMPILE=arm-eabi-
##export PATH=$(pwd)/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$PATH

export PATH=$(pwd)/../../../../arm-eabi-4.6/bin:$PATH
export CROSS_COMPILE=arm-eabi-


make mrproper
make O=./obj/KERNEL_OBJ/ clean
if [ -f ./zImage ]
then
    rm ./zImage
fi

if [ -d ./obj/ ]
then
    rm -r ./obj/
fi

>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
