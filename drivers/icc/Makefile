include $(PWD)/../Makeenv
DEBFLAGS = -O3 -w
EXTRA_CFLAGS += $(DEBFLAGS)

obj-m := TDA8007_ic.o

TDA8007_ic-objs :=  s3c2410_tda8007.o icccmd.o  libicc.o  haldrv.o qingdao.o
TDA8007_ic-objs += iso7816.o sync.o atr.o protocol.o
TDA8007_ic-objs += shebao.o shsam.o gjpsam.o emv.o gjisam.o

ALL:
	$(MAKE) -C $(KERNELSRC) M=$(PWD) modules	

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers  modules.order Module.markers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif





