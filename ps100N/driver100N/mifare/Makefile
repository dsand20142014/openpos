include $(PWD)/../Makeenv
DEBFLAGS = -O3
EXTRA_CFLAGS += $(DEBFLAGS)

obj-m := mifare.o

mifare-objs := halmifare.o pcd.o rc531.o

ALL:
	$(MAKE) -C $(KERNELSRC) M=$(PWD) modules	

clean:
	rm -rf *~ *.mod.o core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif





