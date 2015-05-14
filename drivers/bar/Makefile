include $(PWD)/../Makeenv

obj-m	:= bar_scanner_triger.o

ALL:
	$(MAKE) -C $(KERNELSRC) M=$(PWD) modules
	
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers Module.markers  modules.order

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
