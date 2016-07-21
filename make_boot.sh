#!/bin/bash
export KERNELDIR=`readlink -f .`
export RAMFS_SOURCE=`readlink -f $KERNELDIR/ramdisk/a910`
export USE_SEC_FIPS_MODE=true

echo "kerneldir = $KERNELDIR"
echo "ramfs_source = $RAMFS_SOURCE"

RAMFS_TMP="/tmp/linhphi-a910-ramdisk"

echo "ramfs_tmp = $RAMFS_TMP"
cd $KERNELDIR

if [ "${1}" = "skip" ] ; then
	echo "Skipping Compilation"
else
	echo "Compiling kernel"
	cp defconfig .config
	make "$@" || exit 1
fi

echo "Building new ramdisk"
#remove previous ramfs files
rm -rf '$RAMFS_TMP'*
rm -rf $RAMFS_TMP
rm -rf $RAMFS_TMP.cpio
#copy ramfs files to tmp directory
cp -ax $RAMFS_SOURCE $RAMFS_TMP
cd $RAMFS_TMP

find . | fakeroot cpio -H newc -o | lzop -9 > $RAMFS_TMP.cpio.lzo
ls -lh $RAMFS_TMP.cpio.lzo
cd $KERNELDIR

echo "Making new boot image"
./dtbTool -o ./obj/arch/arm/boot/dt.img -s 2048 -p ./obj/scripts/dtc/ ./obj/arch/arm/boot/
./mkbootimg --kernel $KERNELDIR/obj/arch/arm/boot/zImage --dt $KERNELDIR/obj/arch/arm/boot/dt.img --ramdisk $RAMFS_TMP.cpio.lzo --cmdline 'console=ttyHSL0,115200,n8 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 androidboot.bootdevice=msm_sdcc.1 androidboot.selinux=permissive' --base 0x00000000 --pagesize 2048 --kernel_offset 0x00008000 --ramdisk_offset 0x02000000 --tags_offset 0x01e00000 --second_offset 0x00f00000 -o $KERNELDIR/boot.img
echo -n "SEANDROIDENFORCE" >> boot.img
if [ "${1}" = "CC=\$(CROSS_COMPILE)gcc" ] ; then
	dd if=/dev/zero bs=$((11534336-$(stat -c %s boot.img))) count=1 >> boot.img
fi

echo "done"
ls -al boot.img
echo ""
