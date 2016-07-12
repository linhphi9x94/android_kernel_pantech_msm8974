#
# Makefile for the MSM8974 coreriver TOUCH key drivers.
#


obj-$(CONFIG_KEYBOARD_TC370) += cr_tk_300k.o
#obj-m := cr_tk_300k.o

ANDROID_ROOT_DIR:=../../../../../
KDIR := $(ANDROID_ROOT_DIR)out/target/product/msm8974/obj/KERNEL_OBJ/
PWD := $(shell pwd)
ARCH:=arm
CROSS_COMPILE:=$(ANDROID_ROOT_DIR)prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

#EXTRA_CFLAGS += -include ../../../../../../../BOARD_REV.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f -r *.o *.ko .*cmd *.mod.c .tmp* core *.i modules.order Module.symvers modules.builtin
