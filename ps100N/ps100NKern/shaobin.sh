rm arch/arm/boot/uImage
rm arch/arm/boot/zImage
rm arch/arm/boot/Image

make -j20 uImage
rm /tftpboot/uImage1 -rvf
\cp -ravf arch/arm/boot/uImage /tftpboot/uImage1

\cp -ravf arch/arm/boot/uImage /media/4AC9-5B59/
\cp -ravf arch/arm/boot/uImage /media/4AC9-5B59/uImage1
umount  /media/4AC9-5B59/
 
