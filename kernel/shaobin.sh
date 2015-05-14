rm arch/arm/boot/uImage
rm arch/arm/boot/zImage
rm arch/arm/boot/Image

make -j20 uImage ; make modules
rm -rvf  /tftpboot/uImage 
rm -rvf  /media/FMMC/uImage
\cp -ravf arch/arm/boot/uImage /tftpboot/uImage

\cp -ravf arch/arm/boot/uImage  /media/FMMC
\cp -ravf drivers/usb/gadget/g_zero.ko      /tftpboot/
\cp -ravf drivers/usb/gadget/g_hid.ko  /tftpboot/
\cp -ravf drivers/usb/gadget/arcotg_udc.ko  /tftpboot/


