#OBJS = fc8150.o bbm.o fc8150_bb.o fci_tun.o fci_hal.o fci_i2c.o fc8150_i2c.o fc8150_isr.o fc8150_tun.o fc8150_spi.o fc8150_spib.o fc8150_ppi.o fc8150_hpi.o fci_oal.o fci_ringbuffer.o fci_bypass.o
#
#EXTRA_CFLAGS += -I$(PWD)/../inc
#KDIR = /home/android/workspace/S5PV210/kernel
#ODIR = $(PWD)/../
#
#obj-m = isdbt.o
#isdbt-objs = $(OBJS)
#
#all:
#	$(MAKE) -C $(KDIR) M=$(PWD) modules
#	mv *.ko			$(ODIR)../
#	mv *.o			$(ODIR)
#	mv isdbt.mod.c		$(ODIR)
#	mv Module.symvers	$(ODIR)
#	mv modules.order	$(ODIR)
#	mv .*.o.cmd		$(ODIR)
#	mv .*.ko.cmd		$(ODIR)
#clean:
#	rm -rf $(ODIR)../*.ko
#	rm -rf $(ODIR)*.o
#	rm -rf $(ODIR)isdbt.mod.c
#	rm -rf $(ODIR)Module.symvers
#	rm -rf $(ODIR)modules.order
#	rm -rf $(ODIR).*.o.cmd
#	rm -rf $(ODIR).*.ko.cmd

obj-y += fc8150_drv.o

    fc8150_drv-y := fc8150_wrapper.o
    #fc8150_drv-y += fc8150.o
    fc8150_drv-y += bbm.o
    fc8150_drv-y += fc8150_bb.o
    fc8150_drv-y += fci_tun.o
    fc8150_drv-y += fci_hal.o
    fc8150_drv-y += fci_i2c.o
    fc8150_drv-y += fc8150_i2c.o
    fc8150_drv-y += fc8150_isr.o
    fc8150_drv-y += fc8150_tun.o
    fc8150_drv-y += fc8150_spi.o
    fc8150_drv-y += fc8150_spib.o
    fc8150_drv-y += fc8150_ppi.o
    fc8150_drv-y += fc8150_hpi.o
    fc8150_drv-y += fci_oal.o
    fc8150_drv-y += fci_ringbuffer.o
    fc8150_drv-y += fci_bypass.o
