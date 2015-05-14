#!/bin/bash

# This script adds the CSF , the public key,signature and certificate to a u-boot binary.

# $1 is u-boot binary file to use.
# By default, u-boot's flash header / DCD specifies the ROM to copy 256kB bytes
# u-boot size is usually around ~200kB, so it left ~50kB free for the CSF+key,
# which is more than needed (max required would be 1-2kB).

#All sizes are in bytes.

#This offset specifies where the CSF+key will be placed.
# +0x64000
pubk_offset=409600
# +0x64000 (pubk_offset+0x400)
csf_offset=410624
# +0xE0000 (cert offset)
cert_offset=917504
# App destination pointer
let "app_dest_ptr = 0x83F00000"
# first address to pad to:
padding_addr=`expr $pubk_offset + $app_dest_ptr`

#if [[ "$1" == "" || "$2" == "" ]]; then
if [ "$1" == "" ]; then
echo "This script adds the CSF and the public key to a u-boot binary"
echo "Usage: $0 u-boot"
exit
fi

echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "!!! Flash header and SRK must be updated with valid pointers !!!"
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
#**************************************************************************************
# 1/ Create a padded u-boot from the original that is ready to be signed.
#**************************************************************************************
# add the zero padding at the end of the u-boot binary to obtain a 250kB size file.
echo "Creating a padded u-boot of $pubk_offset Bytes."
echo "Please sign this file => u-boot_zero_padded_to_sign.S"
echo ""
echo "Note: ensure that the flash header has been updated"
echo "      with the pointers to the public key and CSF!!"
# create a S-record version of the binary for the code signing tools
# (CST accepts binaries but the script to sign expects S-rec :-)
objcopy -I binary -O srec $1.bin --srec-forceS3 --change-address $app_dest_ptr --pad-to $padding_addr $1_zero_padded_to_sign.S

# create the binary version of the padded file:
objcopy -I srec -O binary $1_zero_padded_to_sign.S $1_zero_padded_to_sign.bin

# create a sha256+1024bit-RSA signature for  u-boot_zero_padded_to_sign.bin
openssl dgst -sha256 -sign appkey.pem -out $1_rsasigned.bin $1_zero_padded_to_sign.bin 
echo "u-boot file is signed."

#create a sha256+1024bit-RSA sinature for csf.bin
openssl dgst -sha256 -sign csfkey.pem -out csf_rsasigned.bin csf_pro.bin 
#openssl dgst -sha256 -binary -out csfsha256.bin csf.bin
#openssl rsautl -sign -in csfsha256.bin -inkey csfkey.pem -out csf_rsasigned.bin
echo "csf.bin is signed."

#command sequence file+signature+certification
cat csf_pro.bin csf_rsasigned.bin csf.crt > csf1.bin
cat csf1.bin app.crt $1_rsasigned.bin > $1_csf.bin
#echo ""
#echo "Press Enter when the code is signed."
#echo "The script expects the CSF S-record: e.g. $1_csf.S"
#keytocontinue=0
#while [ "$keytocontinue" = "0" ]
#do
#	read keytocontinue
#done

#**************************************************************************************
# 2/ Create a u-boot with the padded u-boot + public key.
#**************************************************************************************
# add the public key to the padded u-boot
# echo "Creating a u-boot with the public key + CSF => $1_signed.bin"
cat $1_zero_padded_to_sign.bin SRK.CA4.SAND.wtls.bin > $1_zero_padded_to_sign+pubk.tmp

#**************************************************************************************
# 3/ Create a u-boot with the previous u-boot + CSF (signatures/certificates).
#**************************************************************************************
# extract the size of the new u-boot to calculate the padding to the CSF.
rdb_size=`ls -all $1_zero_padded_to_sign+pubk.tmp | gawk '{ print $5 }'`

# calculate the padding size.
padding_size=`expr $csf_offset - $rdb_size`

# create a zero padding file whose size is padding_size
dd if=/dev/zero of=zero_file.tmp bs=1 count=$padding_size &> /dev/null

# create the CSF binary file from the S-record
#objcopy -I srec -O binary $1_csf.S $1_csf.bin

# add the zero padding at the end of the u-boot binary to obtain a 250kB size file.
cat $1_zero_padded_to_sign+pubk.tmp zero_file.tmp $1_csf.bin > $1_signed.tmp

#**************************************************************************************
# 4/ Create a padded final u-boot for USB ATK download.
#**************************************************************************************
# extract the size of the new u-boot to calculate the padding to the CSF.
rdb_size=`ls -all $1_signed.tmp | gawk '{ print $5 }'`

# calculate the padding size.
# check if a multiple of 16-bit
rdb_reminder=`expr $rdb_size % 2`
if [ $rdb_reminder == 1 ]; then
# create a zero padding file whose size is 1byte to add only one 0x00
rm zero_file.tmp
dd if=/dev/zero of=zero_file.tmp bs=1 count=1 &> /dev/null
fi 

# add the zero padding at the end of the u-boot binary to obtain a 16-bit aligned size file.
echo "Creating the signed u-boot => $1_signed.bin"
cat $1_signed.tmp zero_file.tmp > $1_signed.bin


#**************************************************************************************
# 5/ Remove temporary files.
#**************************************************************************************
rm zero_file.tmp
rm $1_zero_padded_to_sign+pubk.tmp
rm $1_zero_padded_to_sign.bin
rm $1_zero_padded_to_sign.S
rm csf_rsasigned.bin
rm $1_rsasigned.bin
rm csf1.bin
rm $1_signed.tmp
rm $1_csf.bin
