#ifndef __LUXAPP_SECURITY__
#define SECURITY_EXTERN extern
#else
#define SECURITY_EXTERN
#endif


#define PCI_KEY_SIZE 16
#define MAX_CRYPTDATA_SIZE 256
#define MAXKEYNUM 12
#define MAXKEYINDEX 16
#define DUALDOWNTIME   10

#define SCC_ENCRYPT  0
#define SCC_DECRYPT  1

#define KEYCRYPT_OK 0
#define KEY_VALIDFLAG	 0xA5

#define CMDERR_SUCCESS		    0x00

#define CMDERR_CANCEL            	0x01
#define CMDERR_ABORT             	0x02
#define CMDERR_NOTSUPPORT			0x03
#define CMDERR_BADPARAM				0x04
#define CMDERR_BADIP             	0x05
#define CMDERR_WRITEFLASH        	0x06
#define CMDERR_BYPASS				0x07
#define CMDERR_TIMEOUT        		0x08
#define CMDERR_UARTERROR			0x09
#define CMDERR_READFLASH        	0x0A
#define CMDERR_KEYPIN1					0x31
#define CMDERR_KEYPIN2					0x32
#define CMDERR_30SENCRYPTINTERVAL 		0x33
#define CMDERR_30SINPUTPIN 				0x34
#define CMDERR_PINSAME					0x36
#define CMDERR_MODIFYPASSWORD			0x37

#define CMDERR_TIMER15MIN				0x41
#define CMDERR_TIMER1MIN    			0x42
#define CMDERR_CHECKFLAG				0x43
#define CMDERR_DUALCONTROL  			0x44
#define CMDERR_KEYNOTLOAD				0x45

//#define CMDERR_ABORT     				0x40
                                  
#define CMDERR_DUALCONTROLTIMERSOVER 	0x50
#define CMDERR_NOENTERDOWNLOADFLAG 		0x51
#define CMDERR_DUALCONTROLOK			0x52
#define CMDERR_KEYSAME					0x53
#define CMDERR_CHECKFLAGNG  			0x54
#define CMDERR_KEYDOWNTIMESOVER 		0x55
#define CMDERR_PINMODIFYTIMESOVER 		0x56
#define CMDERR_PASSWORDNOTMODIFYED 		0x57
#define CMDERR_INKEYLOADSTATE 			0x58
#define CMDERR_UNAUTHORIZED 			0x60	
#define LRC_CHECKERROR			0x66
#define CMDERR_ATTACKED					0x71

#define TMKTYPE            1
#define MACKTYPE         2
#define PEKTYPE            3
#define  TMKVALID         0x55
#define  MACKVALID      (0x55 << 8)
#define  PEKVALID          (0x55 << 16)
#define TMKOFFSET       4
#define MACKOFFSET    20
#define PEKOFFSET        36
typedef struct
{
	unsigned char crypydir;
	unsigned char cryptkey[PCI_KEY_SIZE];
	unsigned int    datalen;
	unsigned char cryptdatain[MAX_CRYPTDATA_SIZE];
	unsigned char  cryptdataout[MAX_CRYPTDATA_SIZE];
}KEYCRYPT_DATA;


typedef struct 
{
	unsigned char crypydir;
	unsigned int cryptkeylen;
	unsigned char cryptkeyin[PCI_KEY_SIZE];
	unsigned char cryptkeyout[PCI_KEY_SIZE];
}KEYCRYPT_KEY;


typedef struct
{
	unsigned int    keyvalid;
	unsigned char TMKKey[16];
	unsigned char MACKey[16];
	unsigned char PEKey[16];
	//unsigned char KSKKey[16];
}KEYTABLE;

typedef struct
{
	unsigned char ucdualcontroltime;
	unsigned char firstloadkeypin[MAXKEYNUM];
	unsigned char secloadkeypin[MAXKEYNUM];
	unsigned char ucPasswordModifyed[2];	
	unsigned char aucKeyLoadedFlag[16][3];
}KEYDOWN_CONTROL;

#define KEYCRYPT_IOC_NUM  'C'

#define IOCTL_ENCRYT_PCITMK                                  _IOWR(KEYCRYPT_IOC_NUM, 1,int)
#define IOCTL_CRYPTION_PCI                               _IOWR(KEYCRYPT_IOC_NUM, 2,int)
#define IOCTL_CRYPTION_NOPCI                          _IOWR(KEYCRYPT_IOC_NUM, 3,int)//KEYCRYPT_DATA_NOPCI
#define IOCTL_GET_RANDOM_NUMBER                  _IOWR(KEYCRYPT_IOC_NUM, 4,int)

#define PCIKEY_INFO_FILE     "/mnt/nfs/lijinmei/pcikey/pcikeyinfo.txt"
#define PCIKEY_CONTROL_FILE     "/mnt/nfs/lijinmei/pcikey/pcikeyctl.txt"

int keycrypt_fusekey(unsigned char *key,unsigned char crptdir,unsigned int keylen);
int keycrypt_pci(unsigned char *key,unsigned char  cryptdir , unsigned int cryptlen, unsigned char *cryptkey);
unsigned char pci_get_key(unsigned int group,unsigned char keytype,unsigned char *uckey);
unsigned char  pci_put_key(unsigned int group,unsigned char keytype,unsigned char *uckey);

