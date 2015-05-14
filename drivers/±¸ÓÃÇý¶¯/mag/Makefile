include $(PWD)/../Makeenv

obj-m	:= trimag_spi.o

ALL:
	$(MAKE) -C $(KERNELSRC) M=$(PWD) modules
	
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend
	

ifeq (.depend,$(wildcard .depend))
include .depend
endif
