/*
 * (C) Copyright 2002
 * Richard Jones, rjones@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <s_record.h>
#include <net.h>
#include <ata.h>
#include <part.h>
#include <fat.h>
#include <librsa.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <configs/mx25_3stack.h>
//#include <checkdef.h>

extern int imx_hash_compute(unsigned char hashtype,unsigned int pinput, unsigned int uilen, unsigned int *output );


#define CERTIFICATE_ERROR      2

unsigned char ucPubKey_modulus[256]={ 0x9d,0x5f,0x32,0xb5,0xc2,0xfd,0x02,0x71,0xe1,0x24,0xda,0xa4,0xf9,0x48,
								    0xed,0x8f,0x84,0x6d,0xf6,0x85,0x4d,0xa3,0xc5,0xbe,0x6b,0x85,0x7d,0x1f,0x67,
								    0xf3,0xd2,0x22,0x04,0x3f,0x3a,0xcd,0xa7,0xec,0x5a,0xab,0xfa,0x55,0x91,0xbd,
								    0xe3,0x55,0xbd,0x4a,0xba,0x74,0x4e,0xce,0x19,0x5a,0x9a,0x21,0xc1,0x8f,0x49,
								    0xab,0x7e,0x02,0xfd,0x5e,0x94,0x58,0xb1,0x34,0x76,0x96,0x8e,0x43,0xcf,0x7b,
								    0xcf,0xf0,0x1e,0x96,0x29,0x4b,0x8e,0x16,0x6d,0x03,0x1c,0x9f,0xa8,0xe2,0xed,
								    0xce,0xf6,0x0d,0x39,0x1c,0xe6,0xc6,0x39,0x19,0xe3,0xee,0x9c,0x3b,0xc8,0x6e,
								    0x3c,0x86,0xa2,0x3f,0xf6,0x43,0x72,0x29,0x01,0x12,0x1e,0x2c,0x11,0x77,0x11,
								    0xac,0x64,0x6e,0xa3,0x9d,0xfd,0x6a,0x79,0xef,0x9e,0x82,0xa3,0xa7,0xbf,0xb5,
								    0x86,0xed,0x2b,0xfd,0x5f,0x26,0x45,0x49,0xa1,0x08,0x63,0x16,0x66,0x7b,0x76,
								    0xdd,0x9d,0x6e,0x12,0xe9,0xdd,0xd1,0xea,0xc8,0xb8,0x78,0xb4,0xda,0x63,0xeb,
								    0x16,0x8f,0xc7,0x71,0x9d,0x56,0x59,0xd8,0x88,0xb9,0xd6,0xfc,0xd3,0x38,0x0c,
								    0x3e,0xa0,0x3e,0x77,0x59,0x1e,0x3f,0xc7,0x02,0x28,0x78,0x24,0xbe,0xbc,0xaa,
								    0x67,0x8c,0x7a,0xcb,0x94,0x88,0xac,0xa0,0x9b,0x7b,0x46,0x1e,0xbd,0x43,0x75,
								    0xf0,0x83,0x6a,0xae,0x66,0x4c,0x6f,0xb4,0x31,0x0f,0x85,0x16,0xc8,0xa1,0x12,
								    0x37,0x53,0xcd,0x64,0xd9,0xfb,0x1a,0x08,0x7e,0xb6,0xcb,0x4d,0x38,0x26,0xb9,
								    0xa5,0xba,0xba,0x2a,0xcc,0x4c,0x62,0x2a,0x62,0x35,0x9a,0x36,0x9b,0x41,0x36,
								    0x36,0xef
								};

unsigned char ucPubkey_Exp[3]={0x01, 0x00, 0x01};

#if 0
unsigned char  RSA_recovery_check(unsigned char *pucBegin, unsigned long ulFilesize)
{
      unsigned char hash_value[32];
      unsigned char rsa_recover[256];
      unsigned char hash_value_rec[32];
      unsigned char ucPubKey_mod[256];
      unsigned char ucPubkey_Exp[256];
      unsigned short Explen,Modlen;
      int i,j;
      unsigned char *pucPtr;
      unsigned char cert_valid;
      unsigned char cert_data[1024];
      unsigned int size;
	nand_info_t *nand;

      cert_valid = 0;
	memset(cert_data,0,sizeof(cert_data));
     nand = &nand_info[0];
	 size = 1024;
      nand_read_skip_bad(nand, 0xE0000, &size,(unsigned char *)cert_data);	
	//for(j=0;j<1024;j++)
	//printf("[%02x]/",cert_data[j]);
	pucPtr = cert_data;
	Explen = 0;
	Modlen = 0;
	//printf("pucPtr = %0x08\n",pucPtr);
	for(i=0;i<1024;i++)   //certificate is no more than 1K
	{
		if((*pucPtr == 0x02) && (*(++pucPtr) == 0))
		{
		  pucPtr += 1;		   
			Explen |= (*((unsigned char *)pucPtr) << 8);
			pucPtr++;
			Explen |= *((unsigned char *)pucPtr);
			printf("Explen=%d\n",Explen);
			if(Explen > 256)
				return(CERTIFICATE_ERROR);
			pucPtr ++;
			memcpy(ucPubkey_Exp,(unsigned char *)pucPtr,Explen);
			//for(j=0;j<Explen;j++)
			//	printf("[%02x]",ucPubkey_Exp[j]);
			pucPtr +=Explen;
			Modlen |= (*((unsigned char *)pucPtr) << 8);
			pucPtr++;
			Modlen |=  *((unsigned char *)pucPtr);
			printf("Modlen=%d\n",Modlen);
			if(Modlen > 256)
				return(CERTIFICATE_ERROR);
			pucPtr ++;
			memcpy(ucPubKey_mod,(unsigned char *)pucPtr,Modlen);      
			//for(j=0;j<Modlen;j++)
				//printf("[%02x]/",ucPubKey_mod[j]);
			cert_valid = 1;
			break;
		}
		else
			pucPtr++;
	}
	if(!cert_valid)
		return(CERTIFICATE_ERROR);
	 printf("rsa check\n");
	 imx_hash_compute(2,(unsigned int)pucBegin,(unsigned int)ulFilesize,(unsigned int *)hash_value);
	for(i=0;i<32;i++)
	    printf("hash0[%d]=%02x/",i,hash_value[i]);
	//for(i=0;i<256;i++)
		//printf("rsadata[%d] = %02x/",i,*((unsigned char *)(pucBegin+ulFilesize+i)));
	printf("\n");
	 RSA_PublicDecrypt(ucPubKey_mod,Modlen,ucPubkey_Exp,Explen,
					(unsigned char *)(pucBegin+ulFilesize),256,
					rsa_recover);
        for(i=224;i<256;i++)
		printf("rsa_recov[%d]=%02x/",i,rsa_recover[i]);
	for(i=0;i<32;i++)
		hash_value_rec[i]=rsa_recover[224+i];
	if(!memcmp(hash_value,hash_value_rec,32))
		return 0;
	else
		return 1;

}
#endif
unsigned char  RSA_recovery_check(unsigned char *pucBegin, unsigned long ulFilesize)
{
      unsigned char hash_value[32];
      unsigned char rsa_recover[256];
      unsigned char hash_value_rec[32]; 
      int i;

	 printf("rsa check\n");
	 imx_hash_compute(2,(unsigned int)pucBegin,(unsigned int)ulFilesize,(unsigned int *)hash_value);
	for(i=0;i<32;i++)
	    printf("hash0[%d]=%02x/",i,hash_value[i]);
	//for(i=0;i<256;i++)
		//printf("rsadata[%d] = %02x/",i,*((unsigned char *)(pucBegin+ulFilesize+i)));
	printf("\n");
	 RSA_PublicDecrypt(ucPubKey_modulus,256,ucPubkey_Exp,3,
					(unsigned char *)(pucBegin+ulFilesize),256,
					rsa_recover);
        for(i=224;i<256;i++)
		printf("rsa_recov[%d]=%02x/",i,rsa_recover[i]);
	for(i=0;i<32;i++)
		hash_value_rec[i]=rsa_recover[224+i];
	if(!memcmp(hash_value,hash_value_rec,32))
		return 0;
	else
		return 1;

}

int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	long size;
	unsigned long offset;
	unsigned long count;
	char buf [12];
	block_dev_desc_t *dev_desc=NULL;
	int dev=0;
	int part=1;
	char *ep;
#ifdef PCI_CHECK
	unsigned long filesize;
	unsigned long i;
	unsigned char rsaresult;
#endif

	if (argc < 5) {
		printf( "usage: fatload <interface> <dev[:part]> "
			"<addr> <filename> [bytes]\n");
		return 1;
	}

	//printf("in fatload\n");

	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	//printf("in 68 fatload\n");
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatload **\n",
			argv[1], dev, part);
		return 1;
	}
	//printf("in 74 fatload\n");
	offset = simple_strtoul(argv[3], NULL, 16);
	if (argc == 6)
		count = simple_strtoul(argv[5], NULL, 16);
	else
		count = 0;
//	printf("before file_fat_read()\n");
	size = file_fat_read(argv[4], (unsigned char *)offset, count);   
	if(size==-1) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n",
			argv[4], argv[1], dev, part);
		return 1;
	}
	printf("\n%ld bytes read\n", size); 
#ifdef PCI_CHECK
	  //size = file_fat_read(argv[4], (unsigned char *)(offset-4), count);   //ljm  first 4 byte is length length is not saved	
	filesize = 0;	
	//filesize = *(unsigned  int *)(offset-4);
	filesize = size -256;
	printf("\nfilesize=%ld \n", filesize);        
	//printf("\n%ld bytes read\n", size);
       
	/*if(filesize > 0x10000000)
	{
		printf("filesize is out of 256M!\n");
		printf("clear loaded data!\n");
		for(i=0;i<size;i++)
			//*((unsigned char *)(offset -4 + i)) = 0;  //clear loaded data
			*((unsigned char *)(offset + i)) = 0;  //clear loaded data
		offset = 0;
		return offset;
	}*/
        rsaresult = RSA_recovery_check((unsigned char *)offset, filesize);
	if(rsaresult)
	{
	      printf("rsa check fail!\n");
	      printf("clear loaded data!\n");
		for(i=0;i<size;i++)
			//*((unsigned char *)(offset -4 + i)) = 0;  //clear loaded data
			*((unsigned char *)(offset + i)) = 0;  //clear loaded data
	     size = 0;
	     return -1;
	}
	else
	{
		printf("rsa check ok!\n");
	}
#endif
	//sprintf(buf, "%lX", size-4);
/*
#else
       size = file_fat_read(argv[4], (unsigned char *)offset, count);   //ljm  first 4 byte is length length is not saved

	if(size==-1) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n",
			argv[4], argv[1], dev, part);
		return 1;
	}
       printf("\n%ld bytes read\n", size);  
       sprintf(buf, "%lX", size);
#endif
*/
       sprintf(buf, "%lX", size);
	setenv("filesize", buf);

	return 0;
}


U_BOOT_CMD(
	fatload,	6,	0,	do_fat_fsload,
	"load binary file from a dos filesystem",
	"<interface> <dev[:part]>  <addr> <filename> [bytes]\n"
	"    - load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from dos filesystem"
);

int do_fat_ls (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename = "/";
	int ret;
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;

	if (argc < 3) {
		printf("usage: fatls <interface> <dev[:part]> [directory]\n");
		return 0;
	}
	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatls **\n",
			argv[1], dev, part);
		return 1;
	}
	if (argc == 4)
		ret = file_fat_ls(argv[3]);
	else
		ret = file_fat_ls(filename);

	if(ret!=0)
		printf("No Fat FS detected\n");
	return ret;
}

U_BOOT_CMD(
	fatls,	4,	1,	do_fat_ls,
	"list files in a directory (default /)",
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'"
);

int do_fat_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;

	if (argc < 2) {
		printf("usage: fatinfo <interface> <dev[:part]>\n");
		return 0;
	}
	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatinfo **\n",
			argv[1], dev, part);
		return 1;
	}
	return file_fat_detectfs();
}

U_BOOT_CMD(
	fatinfo,	3,	1,	do_fat_fsinfo,
	"print information about filesystem",
	"<interface> <dev[:part]>\n"
	"    - print information about filesystem from 'dev' on 'interface'"
);
