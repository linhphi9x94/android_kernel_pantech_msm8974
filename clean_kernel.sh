#!/bin/bash
###############################################################################
#
#                           Kernel Build Script 
#
###############################################################################

export ARCH=arm
export PATH=/home/xdavn/bluros/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin:$PATH
export CROSS_COMPILE=arm-linux-androideabi-


make mrproper
rm -rf ./kernel_log.txt
rm -rf ./obj
rm -rf ./zImage
rm -rf ./dt.img
