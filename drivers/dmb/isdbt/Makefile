#
# Makefile for the kernel ISDB-T device drivers.
#
#########################################################

obj-y +=isdbt_device.o

  isdbt_device-y := isdbt_dev.o
  isdbt_device-y += isdbt_bb.o
  isdbt_device-y += isdbt_test.o

ifeq ($(CONFIG_SKY_ISDBT_FC8150_BB),y)
  obj-y += fc8150/
endif

ifeq ($(CONFIG_SKY_ISDBT_SHARP_BB),y)
  obj-y += sharp/
endif
