#!/bin/bash
if [ "${1}" = "mrproper" ] ; then
	make mrproper -j$(($(cat /proc/cpuinfo | grep "^processor" | wc -l)*8))
fi
git reset --hard
git ls-files . --ignored --exclude-standard --others --directory | while read file; do echo $file; rm -rf $file; done
git ls-files . --exclude-standard --others --directory | while read file; do echo $file; rm -rf $file; done
cp defconfig .config
