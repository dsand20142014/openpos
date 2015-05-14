rm ./u-boot.bin -rvf
make -j10
#rm -rvf /tftpboot/u-boot.bin
#cp -ravf u-boot.bin /tftpboot/

rm -rvf /mnt/tftp/u-boot.bin
cp -ravf u-boot.bin /mnt/tftp/
