include $(PWD)/../Makeenv

obj-m	:= ss205fpga.o

KERNELSRC := /opt/imx258/soft/kernel/trunk
PWD       := $(shell pwd)

ALL:
	$(MAKE) -C $(KERNELSRC) M=$(PWD) modules
	
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
