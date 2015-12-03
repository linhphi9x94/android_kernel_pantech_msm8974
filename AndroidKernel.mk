#Android makefile to build kernel as a part of Android Build
PERL		= perl

ifeq ($(TARGET_PREBUILT_KERNEL),)

KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
KERNEL_CONFIG := $(KERNEL_OUT)/.config
ifeq ($(TARGET_KERNEL_APPEND_DTB), true)
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage-dtb
else
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage
endif
KERNEL_HEADERS_INSTALL := $(KERNEL_OUT)/usr
KERNEL_MODULES_INSTALL := system
KERNEL_MODULES_OUT := $(TARGET_OUT)/lib/modules
KERNEL_IMG=$(KERNEL_OUT)/arch/arm/boot/Image

DTS_NAMES ?= $(shell $(PERL) -e 'while (<>) {$$a = $$1 if /CONFIG_ARCH_((?:MSM|QSD|MPQ)[a-zA-Z0-9]+)=y/; $$r = $$1 if /CONFIG_MSM_SOC_REV_(?!NONE)(\w+)=y/; $$arch = $$arch.lc("$$a$$r ") if /CONFIG_ARCH_((?:MSM|QSD|MPQ)[a-zA-Z0-9]+)=y/} print $$arch;' $(KERNEL_CONFIG))
KERNEL_USE_OF ?= $(shell $(PERL) -e '$$of = "n"; while (<>) { if (/CONFIG_USE_OF=y/) { $$of = "y"; break; } } print $$of;' kernel/arch/arm/configs/$(KERNEL_DEFCONFIG))

ifeq "$(KERNEL_USE_OF)" "y"
ifeq ($(OEM_PRODUCT_MANUFACTURER),PANTECH)
PANTECH_PRODUCT_MODEL := $(shell echo $(PROJECT_NAME) | tr A-Z a-z)
PANTECH_BOARD_VERSION := $(shell echo $(PANTECH_BOARD_VER) | tr A-Z a-z)
ifeq ($(MSM_VER),V30)
ifeq ($(PANTECH_BOARD_VERSION),)
DTS_FILES = $(wildcard $(TOP)/kernel/arch/arm/boot/dts/msm8974pro-ab-pm8941-$(PANTECH_PRODUCT_MODEL).dts)
else
DTS_FILES = $(wildcard $(TOP)/kernel/arch/arm/boot/dts/msm8974pro-ab-pm8941-$(PANTECH_PRODUCT_MODEL)-$(PANTECH_BOARD_VERSION).dts)
endif
else
ifeq ($(PANTECH_BOARD_VERSION),)
DTS_FILES = $(wildcard $(TOP)/kernel/arch/arm/boot/dts/msm8974-v2.2-$(PANTECH_PRODUCT_MODEL).dts)
else
DTS_FILES = $(wildcard $(TOP)/kernel/arch/arm/boot/dts/msm8974-v2.2-$(PANTECH_PRODUCT_MODEL)-$(PANTECH_BOARD_VERSION).dts)
endif
endif
else
DTS_FILES = $(wildcard $(TOP)/kernel/arch/arm/boot/dts/$(DTS_NAME)*.dts)
endif
DTS_FILE = $(lastword $(subst /, ,$(1)))
DTB_FILE = $(addprefix $(KERNEL_OUT)/arch/arm/boot/,$(patsubst %.dts,%.dtb,$(call DTS_FILE,$(1))))
ZIMG_FILE = $(addprefix $(KERNEL_OUT)/arch/arm/boot/,$(patsubst %.dts,%-zImage,$(call DTS_FILE,$(1))))
KERNEL_ZIMG = $(KERNEL_OUT)/arch/arm/boot/zImage
DTC = $(KERNEL_OUT)/scripts/dtc/dtc

define append-dtb
mkdir -p $(KERNEL_OUT)/arch/arm/boot;\
$(foreach DTS_NAME, $(DTS_NAMES), \
   $(foreach d, $(DTS_FILES), \
      $(DTC) -p 1024 -O dtb -o $(call DTB_FILE,$(d)) $(d); \
      cat $(KERNEL_ZIMG) $(call DTB_FILE,$(d)) > $(call ZIMG_FILE,$(d));))
endef
else

define append-dtb
endef
endif

ifeq ($(TARGET_USES_UNCOMPRESSED_KERNEL),true)
$(info Using uncompressed kernel)
TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/piggy
else
TARGET_PREBUILT_KERNEL := $(TARGET_PREBUILT_INT_KERNEL)
endif

define mv-modules
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`;\
ko=`find $$mpath/kernel -type f -name *.ko`;\
for i in $$ko; do mv $$i $(KERNEL_MODULES_OUT)/; done;\
fi
endef

define clean-module-folder
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`; rm -rf $$mpath;\
fi
endef

# 20131223 LS1-p13795 modified for exFAT file system
ifeq ($(PANTECH_SDXC_EXFAT_FS),true)
ifeq ($(PANTECH_SDXC_EXFAT_BUILD),true)
define exfat-module
if test -e kheaders*.tar.bz2;then\
rm -rf kheaders*.tar.bz2;\
fi
if test -e tuxera-exfat-3012.4.9.2-apq8064.tgz;then\
rm -rf tuxera-exfat-3012.4.9.2-apq8064.tgz;\
rm -rf tuxera-exfat-3012.4.9.2-apq8064;\
fi
if test -e tuxera-exfat-*-msm8974*.tgz;then\
rm -rf tuxera_exfat_hash;\
rm -rf tuxera-exfat-*-msm8974*.tgz;\
rm -rf tuxera-exfat-*-msm8974*;\
fi
./tuxera_update.sh --source-dir $(TOP)/kernel --output-dir $(KERNEL_OUT) -a --target target/pantech.d/msm8974 --user pantech --pass feke57aze93ni --use-cache --cache-dir $(TOP)/tuxera_exfat_hash
tar -xvzf tuxera-exfat-*-msm8974*.tgz
cp tuxera-exfat-*-msm8974*/exfat/kernel-module/texfat.ko $(KERNEL_MODULES_OUT)
mpath=`find $(TARGET_OUT) -type d -name bin`;\
if [ "$$mdpath" == "" ];then\
mkdir -p $(TARGET_OUT)/bin;\
fi
cp tuxera-exfat-*-msm8974*/exfat/tools/* $(TARGET_OUT)/bin
endef
else
define exfat-module
cp tuxera-exfat-*-msm8974*/exfat/kernel-module/texfat.ko $(KERNEL_MODULES_OUT)
cp tuxera-exfat-*-msm8974*/exfat/tools/* $(TARGET_OUT)/bin
endef
endif
else
define exfat-module
endef
endif

$(KERNEL_OUT):
	mkdir -p $(KERNEL_OUT)

ifeq ($(PANTECH_PERF_BUILD),true)	
$(KERNEL_CONFIG): $(KERNEL_OUT)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- $(KERNEL_DEFCONFIG) PERF_DEFCONFIG=msm8974_pantech_perf_defconfig
else	
$(KERNEL_CONFIG): $(KERNEL_OUT)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- $(KERNEL_DEFCONFIG)
endif	

$(KERNEL_OUT)/piggy : $(TARGET_PREBUILT_INT_KERNEL)
	$(hide) gunzip -c $(KERNEL_OUT)/arch/arm/boot/compressed/piggy.gzip > $(KERNEL_OUT)/piggy

$(TARGET_PREBUILT_INT_KERNEL): $(KERNEL_OUT) $(KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- modules
	$(MAKE) -C kernel O=../$(KERNEL_OUT) INSTALL_MOD_PATH=../../$(KERNEL_MODULES_INSTALL) INSTALL_MOD_STRIP=1 ARCH=arm CROSS_COMPILE=arm-eabi- modules_install
	$(mv-modules)
	$(clean-module-folder)
	$(append-dtb)
	$(exfat-module)

$(KERNEL_HEADERS_INSTALL): $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- headers_install

kerneltags: $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- tags

kernelconfig: $(KERNEL_OUT) $(KERNEL_CONFIG)
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- xconfig
#	env KCONFIG_NOTIMESTAMP=true \ #20130225 jylee modify menuconfig -> xconfig
#	     $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- menuconfig
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- savedefconfig
	cp $(KERNEL_OUT)/defconfig kernel/arch/arm/configs/$(KERNEL_DEFCONFIG)

endif
