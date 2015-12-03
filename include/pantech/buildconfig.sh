#!/bin/bash
############################################################################
#
#                            M8974 Build Script 
#
############################################################################

############################################################################
# DO NOT EDIT BELOW LINES
############################################################################

if [ -n "$BUILD_PATH" ] ; then
    export PATH=${PATH/$BUILD_PATH/}
    export PATH=${PATH/%:/}
fi

export LC_ALL=C
export ARMTOOLS=ARMCT5.01
export ARMLMD_LICENSE_FILE=8224@61.111.130.101:8224@61.111.130.102:8224@61.111.130.90
export PLAT_DEVICE=msm8974
export ARCH=arm

export ARMROOT=/pkg/qct/software/arm/RVDS/5.01bld94
export ARMLIB=$ARMROOT/lib
export ARMINCLUDE=$ARMROOT/include
export ARMINC=$ARMINCLUDE
export ARMBIN=$ARMROOT/bin64

export ARMHOME=$ARMROOT
export ARMPATH=$ARMBIN
export ARMINCLUDE=$ARMINC

export PYTHON_PATH=/pkg/python/2.6.6
export JAVAPATH=/pkg/jdk/jdk1.6.0_26
export JAVA_HOME=/pkg/jdk/jdk1.6.0_26

export HEXAGON_ROOT=/pkg/Qualcomm/HEXAGON_Tools
#export HEXAGON_RTOS_RELEASE=5.0.13
export HEXAGON_Q6VERSION=v5
export HEXAGON_IMAGE_ENTRY=0x08400000

export OEM_PRODUCT_MANUFACTURER=PANTECH

export BUILD_PATH=$PYTHON_PATH/bin:$JAVAPATH/bin:$JAVAPATH/jre/bin:$ARMPATH
export PATH=$BUILD_PATH:$PATH:
