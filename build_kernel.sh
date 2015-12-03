#!/bin/bash
export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/arm-eabi-4.7/bin/arm-eabi-
cp defconfig .config
make -j4
