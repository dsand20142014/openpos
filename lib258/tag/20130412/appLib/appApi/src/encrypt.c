#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <linux/ioctl.h>
#include <string.h>
#include "encrypt.h"

#define  MMI_DEB 0

#define KEY_PCI_FILE            "/dev/imx_keycrypt"
#define KEY_HASH_FILE           "/dev/imx_rtic"
#define KEY_INFO_FILE_HEAD      "/daemon/keyinfo/"//"/mnt/nfs/keyinfo/"
#define KEY_PROTECTINFO_FILE    "/daemon/keyinfo/protect.txt"//"/mnt/nfs/keyinfo/protect.txt"

#define KEY_INFO_FILE_HEAD_BACKUP      "/daemon/keyinfo1/"
#define KEY_PROTECTINFO_FILE_BACKUP    "/daemon/keyinfo1/protect.txt"


#define KEY_DEFAULTVALUE1  "sanddef1"  //73 61 6e 64 64 65 66 31
#define KEY_DEFAULTVALUE2  "sanddef2"   //73 61 6e 64 64 65 66 32




#define KEYCRYPT_IOC_NUM  'C'
#define RTIC_IOC_NUM   'R'

#define IOCTL_CRYPT_FUSEKEY                                  _IOWR(KEYCRYPT_IOC_NUM, 1,int)
#define IOCTL_CRYPT_USERKEY                                  _IOWR(KEYCRYPT_IOC_NUM, 2,int)
#define IOCTL_CRYPT_NOPCI                                       _IOWR(KEYCRYPT_IOC_NUM, 3,int)
#define IOCTL_GET_RANDOM_NUMBER                        _IOWR(KEYCRYPT_IOC_NUM, 4,int)
#define IOCTL_SECURITY_STATUS                                _IOWR(KEYCRYPT_IOC_NUM, 5,int)

#define IOCTL_HASH_COMPUTE                                     _IOWR(RTIC_IOC_NUM, 1,int)

#define KEY_SINGLE_VALUE         0x55
#define KEY_DOUBLE_VALUE		     0x77

#define  KEY_MAX_KEYNUMBER     800
#define  KEY_MAX_PROTNUMBER    10


typedef struct 
{
   unsigned char data[HASH_INDATA_MAXLEN];
   unsigned int  hashlen;
   unsigned char hashtype;  
   unsigned int  output[HASH_OUTDATA_MAXLEN];
}RTIC_HASH_DATA;

typedef struct 
{
   unsigned char ucType;
   unsigned int  ucStatus;  
}SECURITY_DATA;


typedef struct
{
   unsigned char       keyindex;                                               // 保护密钥的索引号
   unsigned char       keyvalue[ENCRYPT_MAX_KEYDATA];                          // 密钥具体信息 加密的数据
}KEY_PROTINFO;

typedef struct
{
   unsigned char        keyvalid;                                             // 该组密钥是否合法 0x55 代表合法 8个字节  0x77 代表 合法的16个字节
   unsigned char        keyvalue[ENCRYPT_MAX_KEYDATA];                             // 密钥具体信息 加密的数据
}KEY_DATAINFO;

typedef struct 
{
   unsigned char crypydir;
   unsigned int cryptkeylen;
   unsigned char cryptkeyin[DES_KEY_MAXSIZE];
   unsigned char cryptkeyout[DES_KEY_MAXSIZE];
}KEY_DATA_FUSEKEY;

typedef struct
{
   unsigned char ucMode;
   unsigned char aucInitVector[8];
   unsigned char auckey[TDES_KEY_MAXSIZE];
   unsigned int    uiKeyLen;
   unsigned int    uiInPutLen;
   unsigned char  aucDataIn[MAX_CRYPTDATA_SIZE];
   unsigned char  aucDataOut[MAX_CRYPTDATA_SIZE];
   unsigned int     uiOutPutLen;
}KEY_DATA_USERKEY;


RTIC_HASH_DATA        Keyhashdata;
KEY_DATA_FUSEKEY        Keycryptdata; 
KEY_DATA_USERKEY      usercryptdata;
KEY_PROTINFO          Keyprotdata;
KEY_DATAINFO          Keydatainfo;


int KEY_keycrypt_Hash(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata);
int ENCRYPT_LoadProtectKey(ENCRYPT_DRV_DATA *keydrvdata);
unsigned char KEY_LoadTransKeyUseProtectKey(
	unsigned short uiAppIndex,			// 保存传输密钥的应用号
	unsigned char ucDecryptType,        // 解密方式
	unsigned char ucDecryptKeyIndex,    // 保护密钥索引
	unsigned short ussaveIndex,       // 存传输密钥索引
	unsigned char *pucInData );
int KEY_keycrypt_imx(unsigned char *key,unsigned char crptdir,unsigned int keylen);
int KEY_prot_put_key(unsigned char index,unsigned char *key);
int KEY_prot_get_key(unsigned char index,unsigned char *key);
int ENCRYPT_Creat_Directory(void);	
int KEY_cipher_get_key(unsigned short appid,unsigned short index,unsigned char *key,unsigned char ucDoubleFlag);
int KEY_cipher_put_key(unsigned short appid,unsigned short index,unsigned char *key,unsigned char ucDoubleFlag);
unsigned char KEY_LoadTransKeyUseAppKey(
	unsigned short uiTransKeyAppId,		// 保存传输密钥的应用号
	unsigned short uiDecryptKeyAppId,     // 解密传输密钥的应用号
	unsigned short uscryptindex,      // 解密密钥索引
	unsigned char ucDecryptType,        // 解密方式
	unsigned short ussaveIndex,       // 存传输密钥索引
	unsigned char *pucInData );
unsigned char Key_EncryptDataUseProtectKey(
	unsigned char ucProtectKeyIndex,
	unsigned char ucEncryptType,
	unsigned char *pucInData,
	unsigned char *pucOutData );
unsigned char Key_EncryptDataUseProtectKey(
	unsigned char ucProtectKeyIndex,
	unsigned char ucEncryptType,
	unsigned char *pucInData,
	unsigned char *pucOutData );
unsigned char DYMN_Encrypt8ByteSingleKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData);
unsigned char DYMN_TEncryptKey(
	unsigned char ucEncryptType,
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData);
int ENCRYPT_LoadProtectKey(ENCRYPT_DRV_DATA *keydrvdata);
unsigned char DYMN_Decrypt8ByteSingleKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData);	
unsigned char DYMN_TDecryptKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData);		
unsigned char DYMN_3TDecryptKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData);
	
#if 0
int manager(void *p) {printf("manager\n");}

ENCRYPT_HASH_DATA     hashtest;
ENCRYPT_DRV_DATA      encrpttest;
KEY_APPID_DATA        appiddata;

int main(void)
{
	    unsigned char i;
	    
	    printf("begin!!");
	    memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_LOAD_PROTKEY;    
     encrpttest.protindex = 0x30;
     memcpy(encrpttest.keyvalue,"1234567890123456",16);
     ENCRYPT_Decrypt_Load_Key(&encrpttest); 
	    
	 /*  memset(&appiddata,0,sizeof(KEY_APPID_DATA));
     appiddata.delflag = KEY_DEL_PART;
     appiddata.appnumber = 2;
     appiddata.appid[0] = 4386;
     appiddata.appid[1] = 3398;
     KEY_KeyCheckApp(&appiddata);*/
	    

	   /* memset(&hashtest,0,sizeof(hashtest));
	     hashtest.hashtype = HASH_SHA1;  //  HASH_SHA256  HASH_SHA1
	    hashtest.hashlen = 16;
	    memcpy(hashtest.hashindata,"1234567890123456",hashtest.hashlen);
	    ENCRYPT_Hash(&hashtest);*/
	
	 /*  memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_LOAD_PROTKEY;    
     encrpttest.protindex = 0x30;
     memcpy(encrpttest.keyvalue,"1234567890123456",16);
     ENCRYPT_Decrypt_Load_Key(&encrpttest); 
     
     memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_LOAD_PROTKEY;    
     encrpttest.protindex = 0x32;
     memcpy(encrpttest.keyvalue,"12345678901234561234567890123456",32);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);  */
     
   /*  for(i=0;i<11;i++)
     {
     	  memset(&encrpttest,0,sizeof(encrpttest));
	     encrpttest.keytype = ENCRYPT_LOAD_PROTKEY;    
	     encrpttest.protindex = 0x30+i;
	     memset(encrpttest.keyvalue,i,32);
	     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     }*/
     
   /* memset(&appiddata,0,sizeof(KEY_APPID_DATA));
     appiddata.delflag = KEY_DEL_ALL;
     KEY_KeyCheckApp(&appiddata);*/
	    
	 /*  memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYPROTKEY;    
     encrpttest.protindex = 0x30;
     encrpttest.saveindex = 0x00;
     encrpttest.saveappid = 0x11;
     encrpttest.crypttype = ENCRYPT_DES_SINGLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"1234567890123456",16);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     
     memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYPROTKEY;    
     encrpttest.protindex = 0x31;
     encrpttest.saveindex = 0x01;
     encrpttest.saveappid = 0x1122;
     encrpttest.crypttype = ENCRYPT_3DES_SINGLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"1234567890123456",16);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     
     memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYPROTKEY;    
     encrpttest.protindex = 0x32;
     encrpttest.saveindex = 0x02;
     encrpttest.saveappid = 0x3456;
     encrpttest.crypttype = ENCRYPT_3DESDOUBLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"12345678901234561111111111111111",32);
     ENCRYPT_Decrypt_Load_Key(&encrpttest); */
     
    /* memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYPROTKEY;    
     encrpttest.protindex = 0x31;
     encrpttest.saveindex = 3;
     encrpttest.crypttype = ENCRYPT_3DESDOUBLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"12345678901234561111111111111111",32);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);*/
     
   /*  for(i=0;i<6;i++)
     {
     	   printf("\n*********************************8\n");
     	   memset(&encrpttest,0,sizeof(encrpttest));
		     encrpttest.keytype = ENCRYPT_DECRYPT_BYPROTKEY;    
		     encrpttest.protindex = 0x31;
		     encrpttest.crypttype = ENCRYPT_DES_SINGLE+i;
		     memcpy(encrpttest.keyvalue,"1234567890123456",16);
		     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     }*/
     
   /*  memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYTRANSKEY;    
     encrpttest.cryptappid = 0x11;
     encrpttest.cryptindex = 0x00;
     encrpttest.saveindex = 0x06;
     encrpttest.saveappid = 0x00;
     encrpttest.crypttype = ENCRYPT_DES_SINGLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"1234567890123456",16);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     
      memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYTRANSKEY;    
      encrpttest.cryptappid = 0x11;
     encrpttest.cryptindex = 0x00;
     encrpttest.saveindex = 0x07;
     encrpttest.saveappid = 0x00;
     encrpttest.crypttype = ENCRYPT_3DES_SINGLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"1234567890123456",16);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     
      memset(&encrpttest,0,sizeof(encrpttest));
     encrpttest.keytype = ENCRYPT_SAVE_BYTRANSKEY;    
      encrpttest.cryptappid = 0x11;
     encrpttest.cryptindex = 0x00;
     encrpttest.saveindex = 0x08;
     encrpttest.saveappid = 0x00;
     encrpttest.crypttype = ENCRYPT_3DESDOUBLE_DECRYPT;
     memcpy(encrpttest.keyvalue,"12345678901234561111111111111111",32);
     ENCRYPT_Decrypt_Load_Key(&encrpttest);*/
     
    /*  for(i=0;i<6;i++)
     {
     	   printf("\n*********************************8\n");
     	   memset(&encrpttest,0,sizeof(encrpttest));
		     encrpttest.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;    
		      encrpttest.cryptappid = 0x11;
          encrpttest.cryptindex = 0x00;
		     encrpttest.crypttype = ENCRYPT_DES_SINGLE+i;
		     memcpy(encrpttest.keyvalue,"1234567890123456",16);
		     ENCRYPT_Decrypt_Load_Key(&encrpttest);
     }*/
     
	   
	
}	
#endif

/*int ENCRYPT_Hash(ENCRYPT_HASH_DATA * hashdata)
{
	  int ret,i;
	  unsigned char aucOutdata[HASH_SHA256_MAXLEN];
	  unsigned char ucHashType;
	
	  ucHashType = hashdata->hashtype;
	  if((ucHashType == HASH_SHA1)||(ucHashType == HASH_SHA256))
	  {
	  	 memset(aucOutdata,0,sizeof(aucOutdata));
	     ret = KEY_keycrypt_Hash(hashdata->hashindata,hashdata->hashlen,ucHashType,aucOutdata); 	     
	  }else
	  {
	  	 printf("\n hashtype is error!");
	  	 ret = -1;
	  }
#if MMI_DEB
  printf("\n ret=%d \n",ret);
	printf("\n hash data \n");
	for(i=0;i<HASH_SHA256_MAXLEN;i++)
	{
		printf("%02x ",aucOutdata[i]);
	}
	printf("\n\r");
#endif
	  
	  memset(hashdata->hashoutdata,0,HASH_SHA256_MAXLEN);
    if(ucHashType == HASH_SHA1)
    {
    	   memcpy(hashdata->hashoutdata,aucOutdata,HASH_SHA1_MAXLEN);
    	   hashdata->hashlen = HASH_SHA1_MAXLEN;
    }else
    {
    	   memcpy(hashdata->hashoutdata,aucOutdata,HASH_SHA256_MAXLEN);
    	   hashdata->hashlen = HASH_SHA256_MAXLEN;
    }
	 
	  return ret; 

}*/

int HASH_Hardware(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata)
{
	int fd;
	int ret,i;
	
	if(uiLen > HASH_INDATA_MAXLEN)
		return(HASH_PARAMERR);
	memset(&Keyhashdata,0,sizeof( RTIC_HASH_DATA));
	
	Keyhashdata.hashlen  = uiLen;
	Keyhashdata.hashtype = ucType;
	memcpy(Keyhashdata.data,aucIndata,uiLen);
   
	fd = open(KEY_HASH_FILE, O_RDWR);
	if(fd < 0)
	{
		close(fd);
		printf("can not open imx_rtic!\n");  
		return -1;
	}
#if MMI_DEB
	printf("\nucType=%02x,fd=%d,uiLen=%d\n",ucType,fd,uiLen);
	for(i=0;i<uiLen;i++)
	{
		printf("%02x ",i,aucIndata[i]);
	}
#endif
	ret = ioctl(fd,IOCTL_HASH_COMPUTE,&Keyhashdata);
	if(ret == 0)
	{
#if MMI_DEB
		for(i=0;i<HASH_OUTDATA_MAXLEN;i++)
			printf("\nhashdata[%d]=%08x ",i,Keyhashdata.output[i]);
#endif
	    if(ucType == HASH_SHA1)
	    	memcpy(aucOutdata,(unsigned char*)Keyhashdata.output,HASH_SHA1_MAXLEN);
	    else
	    	memcpy(aucOutdata,(unsigned char*)Keyhashdata.output,HASH_SHA256_MAXLEN);
	}
	else
	{
	    printf("keycrpyt error = %d\n",ret);
	    ret = -1;
	}

	close(fd);
	return ret;	
}

int TripleDES_Hardware(unsigned char *pucIndata, unsigned int uiInputLen,unsigned char uiMode, 
	unsigned char *pucInitVector,unsigned char *pucKey,unsigned int uiKeylen,unsigned char *pucOutdata)
{
    int fd;
	int ret;   

    if(uiInputLen > MAX_CRYPTDATA_SIZE)
		return(KEYCRYPT_PARAMERR);
    memset(&usercryptdata,0,sizeof(KEY_DATA_USERKEY));
    usercryptdata.ucMode= uiMode;
    usercryptdata.uiInPutLen= uiInputLen;
    if(pucInitVector)
		memcpy(usercryptdata.aucInitVector,pucInitVector,8);
    memcpy(usercryptdata.auckey,pucKey,uiKeylen);
    usercryptdata.uiKeyLen = uiKeylen;
    memcpy(usercryptdata.aucDataIn,pucIndata,uiInputLen);
    fd = open("/dev/imx_keycrypt", O_RDWR);
    //printf("fd=%d\n",fd);
    if(fd < 0)
   	{
		printf("can not open imx_keycrypt!\n");  
		return -1;
   	}
	ret = ioctl(fd,IOCTL_CRYPT_USERKEY,&usercryptdata);
	if(ret == KEYCRYPT_OK)
	{	  
		memcpy(pucOutdata,usercryptdata.aucDataOut,usercryptdata.uiInPutLen);
	}
	else
	    printf("pci keycrpyt error = %d\n",ret);
	close(fd);
	return ret;
}

int Get_Random(unsigned char uiLen,unsigned char *pucRandom)
{
	RANDOM_DATA_GET randomdata;
	int fd;
	int ret;

	ret =0;
	if(uiLen > RANDOM_DATALEN)
		return(GETRANDOM_PARAMERR);
	memset(&randomdata,0,sizeof(randomdata));
	randomdata.numberlen = uiLen;
	fd = open("/dev/imx_keycrypt", O_RDWR);
   	// printf("fd=%d\n",fd);
	if(fd < 0)
   	{
		printf("can not open imx_keycrypt!\n");  
		return -1;
   	}
	ret = ioctl(fd,IOCTL_GET_RANDOM_NUMBER,&randomdata);
	//printf("ret = %d\n",ret);
	if(ret == 0)
	{	  
		memcpy(pucRandom,randomdata.random_data,randomdata.numberlen);
	}
	else
		printf("get random  error = %d\n",ret);
	close(fd);
	return ret;
}

int Get_Security_Status(unsigned int *puiStatus)
{
	int fd;
	int ret;
	SECURITY_DATA  security_status;

	ret = 0;
	if(!puiStatus)
		return (0xF0);
	memset(&security_status,0,sizeof(security_status));
	security_status.ucType = 1;
	fd = open("/dev/imx_keycrypt", O_RDWR);
	if(fd < 0)
   	{
		printf("can not open imx_keycrypt!\n");  
		return -1;
   	}
	ret = ioctl(fd,IOCTL_SECURITY_STATUS,&security_status);
	if(ret == 0)
	{
		*puiStatus = security_status.ucStatus;
	}
	else
	{
		printf("get security status  error = %d\n",ret);
	}
	close(fd);
	return(ret);
}

int Set_Security_Initial_Status(void)
{
	int fd;
	int ret;
	SECURITY_DATA  security_status;

	ret = 0;
	memset(&security_status,0,sizeof(security_status));
	security_status.ucType = 0;
	fd = open("/dev/imx_keycrypt", O_RDWR);
	if(fd < 0)
   	{
		printf("can not open imx_keycrypt!\n");  
		return -1;
   	}
	ret = ioctl(fd,IOCTL_SECURITY_STATUS,&security_status);
	if(ret)
	{
		printf("set security status  error = %d\n",ret);
	}
	close(fd);
	return(ret);
}

int KEY_KeyCheckApp(KEY_APPID_DATA *keyappiddata)
{
	unsigned char ucAppCount;
	char aucAppId[6],buf[100];
	unsigned int ucI;
	int fd,ret; 
	int retval = 0;
	
	if(keyappiddata->appnumber>KEY_MAX_APPNUMBER)
	{
		printf("\n appnumber is error!\n");
		return (-1);
	}
	if((keyappiddata->delflag != KEY_DEL_ALL)&&(keyappiddata->delflag != KEY_DEL_PART))
	{
		printf("\n delflag is error!\n");
		return (-1);
	}
	if(keyappiddata->delflag == KEY_DEL_ALL)
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"rm -rf %s",KEY_INFO_FILE_HEAD); 
		ret=system(buf);
#if MMI_DEB
		printf("\nret=%d\n ",ret);
#endif
		if(ret != 0)
		{
			printf("\n del is fail!\n");
			return (-1);
		}
	}
	   
	memset(buf,0,sizeof(buf));
	sprintf(buf,"rm -rf %s",KEY_INFO_FILE_HEAD_BACKUP); 
	ret=system(buf);
#if MMI_DEB     
	printf("\nrm ret=%d\n ",ret);
#endif    
	memset(buf,0,sizeof(buf));
	sprintf(buf,"cp -rf %s %s ",KEY_INFO_FILE_HEAD,KEY_INFO_FILE_HEAD_BACKUP); 
	ret=system(buf);
	if(ret != 0)
	{
		printf("\n cp is fail!\n");
		return (-1);
	}
		 
	memset(buf,0,sizeof(buf));
	sprintf(buf,"rm -rf %s",KEY_INFO_FILE_HEAD); 
	ret=system(buf);
#if MMI_DEB     
	printf("\nret=%d\n ",ret);
#endif     
	if(ret != 0)
	{
		printf("\n del 1 is fail!\n");
		return (-1);
	}	  
	memset(buf,0,sizeof(buf));
	sprintf(buf,"mkdir %s",KEY_INFO_FILE_HEAD); 
	ret=system(buf);
#if MMI_DEB     
	printf("\nret=%d\n ",ret);
#endif
	if(ret != 0)
	{
		printf("\n Creat is fail!\n");
		return (-1);
	} 
	   
	ucAppCount = keyappiddata->appnumber;
	for( ucI=0;ucI<ucAppCount;ucI++ )
	{
		memset(aucAppId,0,sizeof(aucAppId));
		short_asc(aucAppId,5,&keyappiddata->appid[ucI]);
		memset(buf,0,sizeof(buf));
        //sprintf(buf,"rm -rf %s%s.txt",KEY_INFO_FILE_HEAD,aucAppId); 
		sprintf(buf,"cp -rf %s%s.txt  %s%s.txt",KEY_INFO_FILE_HEAD_BACKUP,aucAppId,KEY_INFO_FILE_HEAD,aucAppId);
		ret=system(buf);
#if MMI_DEB   		      
		printf("\nucI=%02x,ret=%d,aucAppId=%s\n ",ucI,ret,aucAppId);
#endif		      
		/*if(ret != 0)
		{
			printf("\n del is fail!\n");
			return (-1);
		}*/       		 	  
	}	
		 
	memset(buf,0,sizeof(buf));
	sprintf(buf,"cp -rf %s %s ",KEY_PROTECTINFO_FILE_BACKUP,KEY_PROTECTINFO_FILE); 
	ret=system(buf);
	if(ret != 0)
	{
		printf("\n cp pro is fail!\n");
	}
		 
	memset(buf,0,sizeof(buf));
	sprintf(buf,"rm -rf %s",KEY_INFO_FILE_HEAD_BACKUP); 
	ret=system(buf);
#if MMI_DEB        
	printf("\nret=%d\n ",ret);
#endif
	if(ret != 0)
	{
		printf("\n del 1 is fail!\n");
		return (-1);
	}	  
	return retval;   
}

int ENCRYPT_Creat_Directory(void)
{
	char buf[100];
	int ret = 0;
	struct stat fst;

	if (stat(KEY_INFO_FILE_HEAD, &fst) < 0)
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"mkdir %s",KEY_INFO_FILE_HEAD); 
		ret=system(buf);
#if MMI_DEB			  
		printf("\nret=%d\n",ret);
#endif			 
		if(ret != 0)
		{
			printf("\n Creat is fail!\n");
			return (-1);
		}
	}
	return ret;
}

int ENCRYPT_Decrypt_Load_Key(ENCRYPT_DRV_DATA *keydrvdata)
{
	int  retval=0;
	unsigned char array,index;
	unsigned char key[ENCRYPT_MAX_KEYDATA],encrypt_data[ENCRYPT_MAX_KEYDATA],encrypted_data[ENCRYPT_MAX_KEYDATA];
	unsigned char inkey[ENCRYPT_MAX_KEYDATA];
	unsigned int  uiTransKeyAppId,uiDeTransKeyAppId;
	int i;
	unsigned char ucResult;
	unsigned char aucInput[ENCRYPT_MAX_KEYDATA+1],aucOutput[ENCRYPT_MAX_KEYDATA+1];
	struct stat fst;
	
	 i = ENCRYPT_Creat_Directory();
	if(i != 0)
	{
		printf("\nCreat fail,stop do ENCRYPT_Decrypt_Load_Key\n ");
		return (-1);
	}
	

#if 0
    uiTransKeyAppId   = get_app_id();
    uiDeTransKeyAppId = get_app_id();
#else  
	uiTransKeyAppId   = keydrvdata->saveappid;
	uiDeTransKeyAppId = keydrvdata->cryptappid;
#endif
	
	memset(key,0,sizeof(key));
	asc_hex(key,ENCRYPT_MAX_KEYDATA,keydrvdata->keyvalue,ENCRYPT_MAX_KEYDATA*2);
	
#if MMI_DEB
	printf("\nuiTransKeyAppId=%d,uiDeTransKeyAppId=%d,key =\n",uiTransKeyAppId,uiDeTransKeyAppId);
	for(i=0;i<ENCRYPT_MAX_KEYDATA;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
	printf("\nkeydrvdata->keytype=%02x",keydrvdata->keytype);
#endif	

	switch(keydrvdata->keytype)
	{
		case ENCRYPT_SAVE_BYPROTKEY:
		  	  ucResult = KEY_LoadTransKeyUseProtectKey( 
					uiTransKeyAppId,			// 保存传输密钥的应用号
					keydrvdata->crypttype,		// 解密方式
					keydrvdata->protindex,		// 保护密钥索引
					keydrvdata->saveindex,// 存传输密钥列号
					key );
		  	  break;
		case ENCRYPT_SAVE_BYTRANSKEY:
		  	  ucResult = KEY_LoadTransKeyUseAppKey( 
					uiTransKeyAppId,			// 保存传输密钥的应用号
					uiDeTransKeyAppId,			// 解密传输密钥的应用号
					keydrvdata->cryptindex,// 解密密钥索引
					keydrvdata->crypttype,		// 解密方式
					keydrvdata->saveindex,// 存传输密钥索引
					key );
		  	   break;
		case ENCRYPT_DECRYPT_BYPROTKEY:
		  	  ucResult =  Key_EncryptDataUseProtectKey( 
					keydrvdata->protindex,		// 保护密钥索引
					keydrvdata->crypttype,		// 解密方式
					key,aucOutput );		// 输入输出数据
		  	   break;
		case ENCRYPT_DECRYPT_BYTRANSKEY:
		  	switch( keydrvdata->crypttype)
			{
				case ENCRYPT_DES_SINGLE:
						  ucResult =  DYMN_Encrypt8ByteSingleKey( 
							uiDeTransKeyAppId,			// 指定解密的应用号
							keydrvdata->cryptindex,// 加密密钥索引	
							key,aucOutput );		// 输入输出数据
						break;
				case ENCRYPT_3DES_SINGLE:
						 ucResult = DYMN_TEncryptKey(
							0,							// 加密方式
							uiDeTransKeyAppId,			// 指定解密的应用号
							keydrvdata->cryptindex,// 加密密钥索引	
							key,aucOutput);		// 输入输出数据
						break;
				case ENCRYPT_3DESDOUBLE:
						 ucResult = DYMN_TEncryptKey(
							1,							// 加密方式
							uiDeTransKeyAppId,			// 指定解密的应用号
							keydrvdata->cryptindex,// 加密密钥索引	
							key,aucOutput);		// 输入输出数据
						break;
				case ENCRYPT_DES_SINGLE_DECRYPT:
						  ucResult =  DYMN_Decrypt8ByteSingleKey( 
							uiDeTransKeyAppId,			// 指定解密的应用号
							keydrvdata->cryptindex,// 加密密钥索引	
							key,aucOutput );		// 输入输出数据
						break;
				case ENCRYPT_3DES_SINGLE_DECRYPT:
						  ucResult =  DYMN_TDecryptKey( 
							uiDeTransKeyAppId,			// 指定解密的应用号
							keydrvdata->cryptindex,// 加密密钥索引	
							key,aucOutput );		// 输入输出数据
						break;
				case ENCRYPT_3DESDOUBLE_DECRYPT:
						  ucResult =  DYMN_3TDecryptKey( 
							uiDeTransKeyAppId,			// 指定解密的应用号
							keydrvdata->cryptindex,// 加密密钥索引	
							key,aucOutput );		// 输入输出数据
						break;
				default:
					return (-1);
					break;
			}
			break;
		case ENCRYPT_LOAD_PROTKEY:
				ucResult = ENCRYPT_LoadProtectKey(keydrvdata);		// 输入输出数据
				break;
		default:  
        	return (-1);
			break;
	}
	if(ucResult)
	{
		return (-1);
	}
	else
	{
		hex_asc(keydrvdata->outdata,aucOutput,ENCRYPT_MAX_KEYDATA*2);
		if(keydrvdata->crypttype == ENCRYPT_3DESDOUBLE)
			keydrvdata->outlen = ENCRYPT_MAX_KEYDATA*2;
		else
			keydrvdata->outlen = ENCRYPT_MAX_KEYDATA;
	}
		
#if MMI_DEB	
	printf("\n keydrvdata->outlen=%d", keydrvdata->outlen);
#endif
	return retval;
}

unsigned char KEY_LoadTransKeyUseProtectKey(
	unsigned short uiAppIndex,			// 保存传输密钥的应用号
	unsigned char ucDecryptType,        // 解密方式
	unsigned char ucDecryptKeyIndex,    // 保护密钥索引
	unsigned short ussaveIndex,       // 存传输密钥索引
	unsigned char *pucInData )
	
{
	unsigned char ucI,ucResult,ucKeyType;
	unsigned char aucInData[ENCRYPT_MAX_KEYDATA],aucKey[ENCRYPT_MAX_KEYDATA],aucOutKey[ENCRYPT_MAX_KEYDATA];
#if MMI_DEB	
	printf("\nuiAppIndex=%d,ucDecryptType=%02x,ucDecryptKeyIndex=%02x,ussaveIndex=%d\n",uiAppIndex,ucDecryptType,ucDecryptKeyIndex,ussaveIndex);
#endif
	
	memset( aucInData,0,sizeof(aucInData) );
	if((ucDecryptType!=ENCRYPT_DES_SINGLE_DECRYPT)&&(ucDecryptType!=ENCRYPT_3DES_SINGLE_DECRYPT)&&(ucDecryptType!=ENCRYPT_3DESDOUBLE_DECRYPT) )
	{
		printf("\nucDecryptType is error");
		return( 1 );
	}
	
	if( (ucDecryptKeyIndex<0x30)||(ucDecryptKeyIndex>0x39) )
	{
		printf("\nucDecryptKeyIndex is error");
		return( 1 );
	}
		
	  
	if( 0 != KEY_prot_get_key(ucDecryptKeyIndex,aucInData))
	{       
		return (1);
	}
#if MMI_DEB		
	printf("\nKEY_prot_get_key\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucInData[ucI]);
	}
#endif		
	if(0 != KEY_keycrypt_imx(aucInData,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA))
	{
		return (1);
	} 
#if MMI_DEB
	printf("\nKEY_keycrypt_imx\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucInData[ucI]);
	} 
		
	printf("\npucInData\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",pucInData[ucI]);
	} 
#endif
	memcpy(aucKey,aucInData,ENCRYPT_MAX_KEYDATA);
	if( ucDecryptType==ENCRYPT_3DESDOUBLE_DECRYPT )
	{
		memcpy( aucInData,pucInData,ENCRYPT_MAX_KEYDATA );
	
	}
	else
	{
		memcpy( aucInData,pucInData,ENCRYPT_MAX_KEYDATA/2 );			
	}
	if( ucDecryptType==ENCRYPT_3DESDOUBLE_DECRYPT )
		ucKeyType = KEYTYPE_DOUBLE;
	else
		ucKeyType = KEYTYPE_SINGLE;
	memset( aucOutKey,0,sizeof(aucOutKey) );
	
	desm1( aucInData,aucOutKey,aucKey );
	if( ucDecryptType!=ENCRYPT_DES_SINGLE_DECRYPT )
	{
		des( aucOutKey,aucInData,aucKey+ENCRYPT_MAX_KEYDATA/2 );
		desm1( aucInData,aucOutKey,aucKey );
	}
	if( ucDecryptType==ENCRYPT_3DESDOUBLE_DECRYPT )
	{
		desm1( aucInData+ENCRYPT_MAX_KEYDATA/2,aucInData,aucKey );
		des( aucInData,aucInData+ENCRYPT_MAX_KEYDATA/2,aucKey+ENCRYPT_MAX_KEYDATA/2 );
		desm1( aucInData+ENCRYPT_MAX_KEYDATA/2,aucOutKey+ENCRYPT_MAX_KEYDATA/2,aucKey );
	}
#if MMI_DEB	
	printf("\naucOutKey\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucOutKey[ucI]);
	} 
#endif	
	if(ucKeyType == KEYTYPE_DOUBLE)
	{
		if( 0 != KEY_keycrypt_imx(aucOutKey,SCC_ENCRYPT,ENCRYPT_MAX_KEYDATA))
		{
			return 1;
		}         //encrypt for save
#if MMI_DEB
		printf("\nKEY_keycrypt_imx  KEYTYPE_DOUBLE\n");
		for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
		{
			printf(" %02x",aucOutKey[ucI]);
		} 
#endif
		/* put the decrypt data into dest index */
		if ( 0 != KEY_cipher_put_key(uiAppIndex,ussaveIndex,aucOutKey,KEY_DOUBLE_VALUE) )
		{
			return 1;
		}
			   
	}
	else
	{
		if( 0 != KEY_keycrypt_imx(aucOutKey,SCC_ENCRYPT,ENCRYPT_MAX_KEYDATA/2))
		{
			return 1;
		}         //encrypt for save
#if MMI_DEB
		printf("\nMMI_keycrypt_imx  KEY_DOUBLE_NO\n");
		for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
		{
			printf(" %02x",aucOutKey[ucI]);
		} 
#endif
		/* put the decrypt data into dest index */
		if ( 0 != KEY_cipher_put_key(uiAppIndex,ussaveIndex,aucOutKey,KEY_SINGLE_VALUE) )
		{
			return 1;
		}
	}

	return( 0 );
}

int ENCRYPT_LoadProtectKey(ENCRYPT_DRV_DATA *keydrvdata)
{
	int  retval=0;
	unsigned char array,index;
	unsigned char key[ENCRYPT_MAX_KEYDATA],encrypt_data[ENCRYPT_MAX_KEYDATA],encrypted_data[ENCRYPT_MAX_KEYDATA];
	unsigned char inkey[ENCRYPT_MAX_KEYDATA];
	int i;

    /* ascii to hex*/
	array = keydrvdata->protindex;
	if((array<0x30)||(array>0x39))
	{
		printf("\nProtect index is error!\n");
		return (-1);
	}
		
	memset(key,0,sizeof(key));
#if MMI_DEB
	printf("\narray=%02x,key=%s\n",array,keydrvdata->keyvalue);
#endif	
	asc_hex(key,ENCRYPT_MAX_KEYDATA,keydrvdata->keyvalue,ENCRYPT_MAX_KEYDATA*2);
	if( 0 != KEY_keycrypt_imx(key, SCC_ENCRYPT, ENCRYPT_MAX_KEYDATA))
	{
		return (-1);
	}
	
#if MMI_DEB
	printf("\nKEY_keycrypt_imx");
	for(i=0;i<ENCRYPT_MAX_KEYDATA;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");

#endif
	
	if( 0 != KEY_prot_put_key(array,key))
	{	     
		return (-1);
	}		
	return retval;
}


int KEY_keycrypt_imx(unsigned char *key,unsigned char crptdir,unsigned int keylen)
{
	int fd;
	int ret;
	
	Keycryptdata.crypydir = crptdir;
	Keycryptdata.cryptkeylen = keylen;
	memcpy( Keycryptdata.cryptkeyin,key,keylen);
	
	fd = open(KEY_PCI_FILE, O_RDWR);
#if MMI_DEB   
	printf("fd=%d\n",fd);
#endif
	if(fd < 0)
	{
		printf("can not open imx_keycrypt!\n"); 
		close(fd); 
		return -1;
	}
	ret = ioctl(fd,IOCTL_CRYPT_FUSEKEY,&Keycryptdata);
#if MMI_DEB
	printf("\nret=%d,keylen=%dKeycryptdata.cryptkeyout[0]=%02x\n",ret,keylen,Keycryptdata.cryptkeyout[0]);
#endif
	if(ret == KEYCRYPT_OK)
	{
		memset(key,0,sizeof(key));
	    memcpy(key,Keycryptdata.cryptkeyout,keylen);
	}else
	    printf("keycrpyt error = %d\n",ret);
	
	close(fd);
	return ret;	
}


int KEY_prot_put_key(unsigned char index,unsigned char *key)
{
	int  fd;
	int  nbytes,retval=0;
	int i,j,result;
	int protlen,res;
	struct stat fst;

#if MMI_DEB	
	printf("\nindex=%02x\n",index);
	printf("put key = ");
	j = ENCRYPT_MAX_KEYDATA;
	for(i=0;i<j;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif

	protlen = sizeof(KEY_PROTINFO);
	if(stat(KEY_PROTECTINFO_FILE, &fst) < 0)
	{
		if ((fd = open(KEY_PROTECTINFO_FILE, O_CREAT|O_RDWR|O_TRUNC, 0777)) < 0 )
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);		
			retval = -1;
			goto exit;
		}
		for(i=0;i<KEY_MAX_PROTNUMBER;i++)
		{
			memset(&Keyprotdata,0,sizeof(KEY_PROTINFO));
			if ((res = write(fd, &Keyprotdata, protlen)) != protlen) 
			{
				printf("write protect is error\n ");
				retval = -1;
				goto exit;
			}
		}
	}
	else
	{
		if((fd = open(KEY_PROTECTINFO_FILE,O_RDWR)) < 0)
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);		
			retval = -1;
			goto exit;
		}
	}
	i = index - 0x30;
	lseek(fd,protlen*i,SEEK_SET);	
	memset(&Keyprotdata,0,sizeof(KEY_PROTINFO));
	Keyprotdata.keyindex = index;
	memcpy(&Keyprotdata.keyvalue,key,ENCRYPT_MAX_KEYDATA);
	if ((res = write(fd, &Keyprotdata, protlen)) != protlen) 
	{
		printf("write protect is error\n ");
		retval = -1;
		goto exit;
	}
  
exit:
	printf("");
	close(fd);
	return retval;
}

int KEY_prot_get_key(unsigned char index,unsigned char *key)
{
	int  fd;
	int  nbytes;
	int i,j,result,retval=0;
	int res,protlen;
	
#if MMI_DEB	   	
	printf("get key = ");	
#endif

	if((fd = open(KEY_PROTECTINFO_FILE,O_RDWR)) < 0)
	{
		 fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		 memcpy(key,KEY_DEFAULTVALUE1,ENCRYPT_MAX_KEYDATA/2);
		 memcpy(&key[ENCRYPT_MAX_KEYDATA/2],KEY_DEFAULTVALUE2,ENCRYPT_MAX_KEYDATA/2);			      
		 goto exit;
	}
	else
	{
	    i = index - 0x30;
	    protlen = sizeof(KEY_PROTINFO);
	    lseek(fd,protlen*i,SEEK_SET);
	    memset(&Keyprotdata,0,sizeof(KEY_PROTINFO));	
		if ((res = read(fd,&Keyprotdata,protlen)) !=  protlen)
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
			memcpy(key,KEY_DEFAULTVALUE1,ENCRYPT_MAX_KEYDATA/2);
			memcpy(&key[ENCRYPT_MAX_KEYDATA/2],KEY_DEFAULTVALUE2,ENCRYPT_MAX_KEYDATA/2);	
			goto exit;
		}
#if MMI_DEB
		printf("\nindex=%02x,Keyprotdata.keyindex=%02x",index,Keyprotdata.keyindex);
#endif 	 
    
 	    if(Keyprotdata.keyindex == index)
 	    {
			memcpy(key,&Keyprotdata.keyvalue,ENCRYPT_MAX_KEYDATA);   
 	    	   	    	  
 	    }
 	    else
 	    {
			memcpy(key,KEY_DEFAULTVALUE1,ENCRYPT_MAX_KEYDATA/2);
			memcpy(&key[ENCRYPT_MAX_KEYDATA/2],KEY_DEFAULTVALUE2,ENCRYPT_MAX_KEYDATA/2);	
			goto exit;
 	    }
	}
exit:		
	printf(""); 
	close(fd);
	
#if MMI_DEB	
	j = ENCRYPT_MAX_KEYDATA;
	for(i=0;i<j;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif

	return retval;
}


int KEY_cipher_put_key(unsigned short appid,unsigned short index,unsigned char *key,unsigned char ucDoubleFlag)
{
	int  fd;
	int ret;
	int i;
	unsigned char ucFlag =0;
	unsigned short usmarker;
	int result;
	int res,keylen;
	int retval = 0;
	char buf[100],aucAppId[6];
	struct stat fst;

  	ret = 0;
#if MMI_DEB
	printf("\nKEY_cipher_put_key index=%d,appid=%d",index,appid);
#endif	

	if(index>(KEY_MAX_KEYNUMBER-1))
	{
		printf("\nindex is error!");
		return -1;
	}
     	  
	memset(aucAppId,0,sizeof(aucAppId));
	short_asc(aucAppId,5,&appid);
	memset(buf,0,sizeof(buf));
	strcpy(buf,KEY_INFO_FILE_HEAD);
	strcat(buf,aucAppId);
	strcat(buf,".txt");
#if MMI_DEB
	printf("\nbuf=%s",buf);
#endif
	if(stat(buf,&fst) < 0)
	{
		if ((fd = open(buf, O_CREAT|O_RDWR|O_TRUNC, 0777)) < 0 )
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);		
			retval = -1;
			goto exit;
		}
		keylen = sizeof(KEY_DATAINFO);
		for(i=0;i<KEY_MAX_KEYNUMBER;i++)
		{
			memset(&Keydatainfo,0,sizeof(KEY_DATAINFO));
			if ((res = write(fd, &Keydatainfo, keylen)) != keylen) 
			{
				printf("write infokey is error!\n ");
				retval = -1;
				goto exit;
			}
		}
	}
	else
	{
		if((fd = open(buf,O_RDWR)) < 0)
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
			retval = -1;
			goto exit;
		}
	}
	keylen = sizeof(KEY_DATAINFO);
	i = index;
	lseek(fd,keylen*i,SEEK_SET);	
	memset(&Keydatainfo,0,sizeof(KEY_DATAINFO));
	if(ucDoubleFlag == KEY_DOUBLE_VALUE)
	{
		Keydatainfo.keyvalid = KEY_DOUBLE_VALUE;
		memcpy(&Keydatainfo.keyvalue,key,ENCRYPT_MAX_KEYDATA);
	}
	else
	{ 
		Keydatainfo.keyvalid = KEY_SINGLE_VALUE;
		memcpy(&Keydatainfo.keyvalue,key,ENCRYPT_MAX_KEYDATA/2);
	}
	if ((res = write(fd, &Keydatainfo, keylen)) != keylen) 
	{
		printf("write infokey is error!\n ");
		retval = -1;
		goto exit;
	}

exit:
	printf("");

#if MMI_DEB	
	int j;
	printf("get key = ");
	j = ENCRYPT_MAX_KEYDATA/2;
	if(ucDoubleFlag == KEY_DOUBLE_VALUE)
		j *= 2;
	for(i=0;i<j;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif	
	close(fd);
	return retval;
}


int KEY_cipher_get_key(unsigned short appid,unsigned short index,unsigned char *key,unsigned char ucDoubleFlag)
{
	int  fd;
	int ret;
	unsigned int ucI,ucJ,ucK;
	unsigned int uiI;	
	unsigned char ucFlag =0;
	unsigned short usmarker;
	int result;
	int length, res,keylen;
	int retval = 0;
	char buf[100],aucAppId[6];
	struct stat fst;

  	ret = 0;
#if MMI_DEB
     printf("\nKEY_cipher_get_key index=%d,appid=%d",index,appid);
#endif	

	if(index>(KEY_MAX_KEYNUMBER-1))
	{
		printf("\nindex is error!");
		return -1;
	}

	memset(aucAppId,0,sizeof(aucAppId));
	short_asc(aucAppId,5,&appid);
	memset(buf,0,sizeof(buf));
	strcpy(buf,KEY_INFO_FILE_HEAD);
	strcat(buf,aucAppId);
	strcat(buf,".txt");
#if MMI_DEB
	printf("\nbuf=%s",buf);
#endif
	if((fd = open(buf,O_RDWR)) < 0)
	{
		fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		memcpy(key,KEY_DEFAULTVALUE1,ENCRYPT_MAX_KEYDATA/2);
		memcpy(&key[ENCRYPT_MAX_KEYDATA/2],KEY_DEFAULTVALUE2,ENCRYPT_MAX_KEYDATA/2);			      
		goto exit;
	}
	else
	{
		uiI = index;
		keylen = sizeof(KEY_DATAINFO);
		lseek(fd,keylen*uiI,SEEK_SET);
		memset(&Keydatainfo,0,sizeof(KEY_DATAINFO));	
		if ((res = read(fd,&Keydatainfo,keylen)) !=  keylen)
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
			memcpy(key,KEY_DEFAULTVALUE1,ENCRYPT_MAX_KEYDATA/2);
			memcpy(&key[ENCRYPT_MAX_KEYDATA/2],KEY_DEFAULTVALUE2,ENCRYPT_MAX_KEYDATA/2);	
			goto exit;
		}
#if MMI_DEB
		printf("\nindex=%02x,Keydatainfo.keyvalid=%02x",index,Keydatainfo.keyvalid);
#endif 	 
		    
		if(ucDoubleFlag == KEY_DOUBLE_VALUE)
		{
			memcpy(key,Keydatainfo.keyvalue,ENCRYPT_MAX_KEYDATA/2);
			memcpy(&key[ENCRYPT_MAX_KEYDATA/2],KEY_DEFAULTVALUE2,ENCRYPT_MAX_KEYDATA/2);	
			if(Keydatainfo.keyvalid == KEY_DOUBLE_VALUE)
			{
				memcpy(&key[ENCRYPT_MAX_KEYDATA/2],&Keydatainfo.keyvalue[ENCRYPT_MAX_KEYDATA/2],ENCRYPT_MAX_KEYDATA/2);	
			}
		}
		else
		{
			memcpy(key,Keydatainfo.keyvalue,ENCRYPT_MAX_KEYDATA/2);
		}
	}

exit:
	printf("");

#if MMI_DEB	
	int i,j;
	printf("get key = ");
	j = ENCRYPT_MAX_KEYDATA/2;
	if(ucDoubleFlag == KEY_DOUBLE_VALUE)
		 j *= 2;
	for(i=0;i<j;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif	
	close(fd);
	return retval;
}


unsigned char KEY_LoadTransKeyUseAppKey(
	unsigned short uiTransKeyAppId,		// 保存传输密钥的应用号
	unsigned short uiDecryptKeyAppId,     // 解密传输密钥的应用号
	unsigned short uscryptindex,      // 解密密钥索引
	unsigned char ucDecryptType,        // 解密方式
	unsigned short ussaveIndex,       // 存传输密钥索引
	unsigned char *pucInData )
{
	unsigned char ucResult,ucLoadType,ucKeyType,ucI;
	unsigned char aucInData[ENCRYPT_MAX_KEYDATA*2],aucKey[ENCRYPT_MAX_KEYDATA*2],aucOutKey[ENCRYPT_MAX_KEYDATA*2];
	
#if MMI_DEB	
	printf("\nuiTransKeyAppId=%d",uiTransKeyAppId);
	printf("\nuiDecryptKeyAppId=%d",uiDecryptKeyAppId);
	printf("\nuscryptindex=%d",uscryptindex);
	printf("\nucDecryptType=%02x",ucDecryptType);
	printf("\nussaveIndex=%d",ussaveIndex);
#endif	
	if( (ucDecryptType!=ENCRYPT_DES_SINGLE_DECRYPT)&&(ucDecryptType!=ENCRYPT_3DES_SINGLE_DECRYPT)&&(ucDecryptType!=ENCRYPT_3DESDOUBLE_DECRYPT) )
	{
		printf("\nucDecryptType is error");
		return( 1 );
	}

	if( ucDecryptType== ENCRYPT_DES_SINGLE_DECRYPT )
		ucKeyType = KEY_SINGLE_VALUE;
	else
		ucKeyType = KEY_DOUBLE_VALUE;
		
	memset(aucKey,0,sizeof(aucKey));
	if ( 0 != KEY_cipher_get_key(uiDecryptKeyAppId,uscryptindex,aucKey,ucKeyType) )
	{
		return 1;
	}
#if MMI_DEB
	printf("\nKEY_cipher_get_key\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	if( 0 != KEY_keycrypt_imx(aucKey,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA))
	{
		return 1;
	}//decrypt key 
#if MMI_DEB
  	printf("\nKEY_keycrypt_imx\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
		
	printf("\npucInData\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",pucInData[ucI]);
	}
#endif	
	memcpy( aucInData,pucInData,ENCRYPT_MAX_KEYDATA);
	memset( aucOutKey,0,sizeof(aucOutKey) );
	ucKeyType = KEYTYPE_SINGLE;
	desm1( aucInData,aucOutKey,aucKey );
	if( ucDecryptType!=ENCRYPT_DES_SINGLE_DECRYPT )
	{
		des( aucOutKey,aucInData,aucKey+ENCRYPT_MAX_KEYDATA/2 );
		desm1( aucInData,aucOutKey,aucKey );
	}
	if(ucDecryptType==ENCRYPT_3DESDOUBLE_DECRYPT )
	{
		ucKeyType = KEYTYPE_DOUBLE;
		desm1( aucInData+ENCRYPT_MAX_KEYDATA/2,aucInData,aucKey );
		des( aucInData,aucInData+ENCRYPT_MAX_KEYDATA/2,aucKey+ENCRYPT_MAX_KEYDATA/2 );
		desm1( aucInData+ENCRYPT_MAX_KEYDATA/2,aucOutKey+ENCRYPT_MAX_KEYDATA/2,aucKey );
	}
#if MMI_DEB	
	printf("\naucOutKey\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucOutKey[ucI]);
	} 
#endif	
	if(ucKeyType == KEYTYPE_DOUBLE)
	{
		if( 0 != KEY_keycrypt_imx(aucOutKey,SCC_ENCRYPT,ENCRYPT_MAX_KEYDATA))
		{
			return 1;
		}//encrypt for save
#if MMI_DEB
		printf("\nKEY_keycrypt_imx  KEYTYPE_DOUBLE\n");
		for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
		{
			printf(" %02x",aucOutKey[ucI]);
		}
#endif 
		/* put the decrypt data into dest index */
		if ( 0 != KEY_cipher_put_key(uiTransKeyAppId,ussaveIndex,aucOutKey,KEY_DOUBLE_VALUE) )
		{
			return 1;
		}
		   
	}
	else
	{
		if( 0 != KEY_keycrypt_imx(aucOutKey,SCC_ENCRYPT,ENCRYPT_MAX_KEYDATA/2))
		{
			return 1;
		}//encrypt for save
#if MMI_DEB
		printf("\nKEY_keycrypt_imx  \n");
		for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
		{
			printf(" %02x",aucOutKey[ucI]);
		} 
#endif
		/* put the decrypt data into dest index */
		if ( 0 != KEY_cipher_put_key(uiTransKeyAppId,ussaveIndex,aucOutKey,KEY_SINGLE_VALUE) )
		{
			return 1;
		}
	}

    return( 0 );
}


unsigned char Key_EncryptDataUseProtectKey(
	unsigned char ucProtectKeyIndex,
	unsigned char ucEncryptType,
	unsigned char *pucInData,
	unsigned char *pucOutData )
{
	unsigned char ucI,ucResult;
	unsigned char aucInData[ENCRYPT_MAX_KEYDATA],aucKey[ENCRYPT_MAX_KEYDATA];
	
#if MMI_DEB	
	printf("\nucProtectKeyIndex=%02x,ucEncryptType=%02x,pucInData=%02x",ucProtectKeyIndex,ucEncryptType,pucInData[0]);
#endif	
	if( (ucProtectKeyIndex<0x30)||(ucProtectKeyIndex>0x39) )
	{
		printf("\nucDecryptKeyIndex is error");
		return( 1 );
	}
	
	if( 0 != KEY_prot_get_key(ucProtectKeyIndex,aucInData))
	{
		return 1;
	}
#if MMI_DEB
	printf("\nucEncryptType=%02x",ucEncryptType);
	printf("\nucProtectKeyIndex=%02x \n aucOutKey\n",ucProtectKeyIndex);
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucInData[ucI]);
	} 
#endif	
	if(0 != KEY_keycrypt_imx(aucInData,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA))
	{
		return (1);
	} 
#if MMI_DEB		
	printf("\nKey__keycrypt_imx\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucInData[ucI]);
	} 
#endif		 
	memcpy( aucKey,aucInData,ENCRYPT_MAX_KEYDATA);
	memcpy( aucInData,pucInData,ENCRYPT_MAX_KEYDATA );
	if((ucEncryptType == ENCRYPT_DES_SINGLE_DECRYPT)||(ucEncryptType == ENCRYPT_3DES_SINGLE_DECRYPT)||(ucEncryptType == ENCRYPT_3DESDOUBLE_DECRYPT))
	{
		desm1( aucInData,pucOutData,aucKey);
		if( ucEncryptType!=ENCRYPT_DES_SINGLE_DECRYPT )
		{
			des( pucOutData,aucInData,aucKey+ENCRYPT_MAX_KEYDATA/2 );
			desm1( aucInData,pucOutData,aucKey );
		}
		if( ucEncryptType==ENCRYPT_3DESDOUBLE_DECRYPT )
		{
			desm1( aucInData+ENCRYPT_MAX_KEYDATA/2,aucInData,aucKey );
			des( aucInData,aucInData+ENCRYPT_MAX_KEYDATA/2,aucKey+ENCRYPT_MAX_KEYDATA/2 );
			desm1( aucInData+ENCRYPT_MAX_KEYDATA/2,pucOutData+ENCRYPT_MAX_KEYDATA/2,aucKey );
		}
	}
	else
	{
		des( aucInData,pucOutData,aucKey );
		if( ucEncryptType!=ENCRYPT_DES_SINGLE )
		{
			desm1( pucOutData,aucInData,aucKey+ENCRYPT_MAX_KEYDATA/2 );
			des( aucInData,pucOutData,aucKey );  
		}
		if( ucEncryptType==ENCRYPT_3DESDOUBLE )
		{
			des( aucInData+ENCRYPT_MAX_KEYDATA/2,aucInData,aucKey );
			desm1( aucInData,aucInData+ENCRYPT_MAX_KEYDATA/2,aucKey+ENCRYPT_MAX_KEYDATA/2 );
			des( aucInData+ENCRYPT_MAX_KEYDATA/2,pucOutData+ENCRYPT_MAX_KEYDATA/2,aucKey );
		}
	}
	
#if MMI_DEB	
	printf("\npucOutData\n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",pucOutData[ucI]);
	} 
#endif	
	return( 0 );
}


unsigned char DYMN_Encrypt8ByteSingleKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
	unsigned char ucResult,ucI;
	unsigned char aucKey[ENCRYPT_MAX_KEYDATA];
#if MMI_DEB   
	printf("\n uiAppIndex=%d,uscryptindex=%d,pucInData[0]=%02x",uiAppIndex,uscryptindex,pucInData[0]);
#endif   
    if( 0 != KEY_cipher_get_key(uiAppIndex,uscryptindex,aucKey,KEY_SINGLE_VALUE) )
	{
		return 1;
	}
#if MMI_DEB
	printf("\n KEY_cipher_get_key \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	if( 0 != KEY_keycrypt_imx(aucKey,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA/2))
	{
		return 1;
	}        
#if MMI_DEB
	printf("\n MMI_keycrypt_imx \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
    des( pucInData,pucOutData,aucKey );
#if MMI_DEB
	printf("\n pucOutData \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",pucOutData[ucI]);
	}
#endif
    return(0);
}


unsigned char DYMN_TEncryptKey(
	unsigned char ucEncryptType,
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
	unsigned char ucResult,ucI;
	unsigned char aucKey[ENCRYPT_MAX_KEYDATA],aucData[ENCRYPT_MAX_KEYDATA];
#if MMI_DEB  
    printf("\nucEncryptType=%02x",ucEncryptType);
    printf("\nuiAppIndex=%d",uiAppIndex);
    printf("\nuscryptindex=%d",uscryptindex);
    printf("\npucInData[0]=%02x",pucInData[0]);
#endif  
	if ( 0 != KEY_cipher_get_key(uiAppIndex,uscryptindex,aucKey,KEY_DOUBLE_VALUE) )
	{
		return 1;
	}
#if MMI_DEB
	printf("\n KEY_cipher_get_key \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	if( 0 != KEY_keycrypt_imx(aucKey,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA))
	{
		return 1;
	}//decrypt key 
#if MMI_DEB
	printf("\n KEY_keycrypt_imx \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	des( pucInData,pucOutData,aucKey );
	desm1( pucOutData,aucData,aucKey+ENCRYPT_MAX_KEYDATA/2 );
	des( aucData,pucOutData,aucKey );
	if( ucEncryptType )
	{
		des( pucInData+ENCRYPT_MAX_KEYDATA/2,pucOutData+ENCRYPT_MAX_KEYDATA/2,aucKey );
		desm1( pucOutData+ENCRYPT_MAX_KEYDATA/2,aucData,aucKey+ENCRYPT_MAX_KEYDATA/2 );
		des( aucData,pucOutData+ENCRYPT_MAX_KEYDATA/2,aucKey );
	}
#if MMI_DEB   
	printf("\n pucOutData \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",pucOutData[ucI]);
	}
#endif
	return( 0 );
}


unsigned char DYMN_Decrypt8ByteSingleKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
    unsigned char ucResult,ucI;
    unsigned char aucKey[ENCRYPT_MAX_KEYDATA];
#if MMI_DEB   
	printf("\n uiAppIndex=%d,uscryptindex=%d,pucInData[0]=%02x",uiAppIndex,uscryptindex,pucInData[0]);
#endif   
    if ( 0 != KEY_cipher_get_key(uiAppIndex,uscryptindex,aucKey,KEY_SINGLE_VALUE) )
	{
		return 1;
	}
#if MMI_DEB
	printf("\n KEY_cipher_get_key \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	if( 0 != KEY_keycrypt_imx(aucKey,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA/2))
	{
		return 1;
	}
#if MMI_DEB
	printf("\n MMI_keycrypt_imx \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
    desm1( pucInData,pucOutData,aucKey );
#if MMI_DEB
	printf("\n pucOutData \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",pucOutData[ucI]);
	}
#endif
    return(0);
}


unsigned char DYMN_TDecryptKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
    unsigned char ucResult,ucI;
    unsigned char aucKey[ENCRYPT_MAX_KEYDATA];
#if MMI_DEB   
	printf("\n uiAppIndex=%d,uscryptindex=%d,pucInData[0]=%02x",uiAppIndex,uscryptindex,pucInData[0]);
#endif   
	if ( 0 != KEY_cipher_get_key(uiAppIndex,uscryptindex,aucKey,KEY_DOUBLE_VALUE) )
	{
		return 1;
	}
#if MMI_DEB
	printf("\n KEY_cipher_get_key \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	if( 0 != KEY_keycrypt_imx(aucKey,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA))
	{
		return 1;
	}        
#if MMI_DEB
	printf("\n MMI_keycrypt_imx \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
    desm1( pucInData,pucOutData,aucKey );
    des( pucOutData,pucInData,&aucKey[ENCRYPT_MAX_KEYDATA/2] );
    desm1( pucInData,pucOutData,aucKey );
#if MMI_DEB
	printf("\n pucOutData \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA/2;ucI++)
	{
		printf(" %02x",pucOutData[ucI]);
	}
#endif
    return(0);
}

unsigned char DYMN_3TDecryptKey(
	unsigned short uiAppIndex,
	unsigned short uscryptindex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
    unsigned char ucResult,ucI;
    unsigned char aucKey[ENCRYPT_MAX_KEYDATA];
#if MMI_DEB   
    printf("\n uiAppIndex=%d,uscryptindex=%d,pucInData[0]=%02x",uiAppIndex,uscryptindex,pucInData[0]);
#endif   
    if ( 0 != KEY_cipher_get_key(uiAppIndex,uscryptindex,aucKey,KEY_DOUBLE_VALUE) )
	{
		return 1;
	}
#if MMI_DEB
	printf("\n KEY_cipher_get_key \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
	if( 0 != KEY_keycrypt_imx(aucKey,SCC_DECRYPT,ENCRYPT_MAX_KEYDATA))
	{
		return 1;
	}        
#if MMI_DEB
	printf("\n MMI_keycrypt_imx \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",aucKey[ucI]);
	}
#endif
    desm1( pucInData,pucOutData,aucKey );
    des( pucOutData,pucInData,&aucKey[ENCRYPT_MAX_KEYDATA/2] );
    desm1( pucInData,pucOutData,aucKey );
    
    desm1( pucInData+ENCRYPT_MAX_KEYDATA/2,pucOutData+ENCRYPT_MAX_KEYDATA/2,aucKey );
    des( pucOutData+ENCRYPT_MAX_KEYDATA/2,pucInData,&aucKey[ENCRYPT_MAX_KEYDATA/2] );
    desm1( pucInData,pucOutData+ENCRYPT_MAX_KEYDATA/2,aucKey );
#if MMI_DEB
	printf("\n pucOutData \n");
	for(ucI=0;ucI<ENCRYPT_MAX_KEYDATA;ucI++)
	{
		printf(" %02x",pucOutData[ucI]);
	}
#endif
    return(0);
}
