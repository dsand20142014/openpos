/*******************************************************
* encrypt interface
*******************************************************
*/
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <linux/ioctl.h>
//#include <dos.h> 



#include <string.h>
#include "osdrv.h"
#include "oscfg_param.h"


#define MMI_DEB 0


#define KEY_INFO_FILE     "/daemon/keyinfo.dat"//"/mnt/pos/config/keyinfo.dat"
#define CFG_BASIC_FILE    "/daemon/osbasic.cfg"//"/mnt/pos/config/osbasic.cfg"
#define KEY_PCI_FILE       "/dev/imx_keycrypt"
#define KEY_HASH_FILE      "/dev/imx_rtic"

#define KEY_MAX_APPNUMBER   20
#define KEY_CREAT_DATETIME  12
#define KEY_MAX_KEYNUMBER   800
#define KEY_MAX_KEYDATA     8
#define KEY_MAX_PROTNUMBER  10

#define KEY_VALID_FLAG     0x55
#define KEY_INVALID_FLAG   0x00


#define KEY_DOUBLE_YES     0x01
#define KEY_DOUBLE_NO      0x00

#define KEYCRYPT_OK 0


#define PCI_KEY_SIZE 16
#define MAX_CRYPTDATA_SIZE 256
#define NOPCI_MKEY_SIZE   8  
#define NOPCI_KEY_MAXSIZE  16
#define RANDOM_DATALEN    128


//#define KEY_PACKAGE_HAVEAPPID   

#ifdef KEY_PACKAGE_HAVEAPPID
#define KEY_PACKAGE_START 2
#else
#define KEY_PACKAGE_START 0
#endif

#define KEYCRYPT_IOC_NUM  'C'
#define RTIC_IOC_NUM   'R'

#define IOCTL_ENCRYT_PCITMK                                  _IOWR(KEYCRYPT_IOC_NUM, 1,int)
#define IOCTL_CRYPTION_PCI                               _IOWR(KEYCRYPT_IOC_NUM, 2,int)
#define IOCTL_CRYPTION_NOPCI                          _IOWR(KEYCRYPT_IOC_NUM, 3,int)//KEYCRYPT_DATA_NOPCI
#define IOCTL_GET_RANDOM_NUMBER                  _IOWR(KEYCRYPT_IOC_NUM, 4,int)
#define IOCTL_HASH_COMPUTE                                  _IOWR(RTIC_IOC_NUM, 1,int)

#define HASH_SHA1           1
#define HASH_SHA256         2
#define HASH_SHA1_MAXLEN    20
#define HASH_SHA256_MAXLEN    32

#define KEY_DRVSTATUS_OK   0
#define KEY_DRVSTATUS_NOK  1


#define SCC_ENCRYPT  0
#define SCC_DECRYPT  1

#define KEYTYPE_SINGLE      0
#define KEYTYPE_DOUBLE      1

#define KEY_DEFAULTVALUE1  "sanddef1"  //73 61 6e 64 64 65 66 31
#define KEY_DEFAULTVALUE2  "sanddef2"   //73 61 6e 64 64 65 66 32

#define HASH_OUTDATA_MAXLEN  8
#define HASH_INDATA_MAXLEN   1024*8



typedef struct 
{
   unsigned char crypydir;
   unsigned int cryptkeylen;
   unsigned char cryptkeyin[NOPCI_KEY_MAXSIZE];
   unsigned char cryptkeyout[NOPCI_KEY_MAXSIZE];
}KEYCRYPT_DATA_NOPCI;




typedef struct
{
   unsigned char       keyvalid;                                             // 该组密钥是否合法 0x55 代表合法
   unsigned char       keydata[KEY_MAX_KEYDATA];                             // 密钥具体信息 加密的数据
}KEYAPPKEYINFO_DATA;

typedef struct
{
   unsigned short      keyappid;                                             // 应用号
   unsigned char       keyvalid;                                             // 0x55 代表创建成功
   unsigned short      keynumber;                                            // 具体某套应用里面已经存在的密钥个数
   unsigned short      keymarker[KEY_MAX_KEYNUMBER];                         // 密钥存储标识
   unsigned short      keyindex[KEY_MAX_KEYNUMBER];                          // 密钥标识对应的存储空间的索引号
}KEY_APPINDEXLIST;

typedef struct
{
   unsigned char       keycreatdatetime[KEY_CREAT_DATETIME];                 // yymmddhhmmss
   unsigned char       keyvalid;                                             // 0x55 代表创建成功
   unsigned char       appnumber;                                            // 当前共有多少套APP KEY
}KEYSYSTEMINFO_DATA;

typedef struct
{
   unsigned char       keyindex;                                             // 保护密钥的索引号
   unsigned char       keydata[KEY_MAX_KEYDATA*2];                             // 密钥具体信息 加密的数据
}KEYPROTINFO_DATA;


typedef struct
{
   KEYSYSTEMINFO_DATA  keyinfodata;                                          // 密钥系统信息说明区
   KEY_APPINDEXLIST    keyappindexlist[KEY_MAX_APPNUMBER];                   // 具体应用密钥存储索引列表  共20套应用，最大支持
   KEYAPPKEYINFO_DATA  keyappdata[KEY_MAX_APPNUMBER*KEY_MAX_KEYNUMBER];      // 密钥存储空间   
   KEYPROTINFO_DATA    keyprotdata[KEY_MAX_PROTNUMBER];                      // 保护密钥                 
}KEYSYSTEM_DATA;

typedef struct 
{
   unsigned char data[HASH_INDATA_MAXLEN];
   unsigned int   hashlen;
   unsigned char hashtype;  
   unsigned int   output[HASH_OUTDATA_MAXLEN];
}RTIC_HASH_DATA;


unsigned short        ucOSCurrAppID;
KEYCRYPT_DATA_NOPCI   Keycryptdata;   
KEYSYSTEMINFO_DATA    Keyinfodata; 
KEY_APPINDEXLIST      Keyappindexlist;
KEYAPPKEYINFO_DATA    Keyappdata; 
KEYPROTINFO_DATA      Keyprotdata;
RTIC_HASH_DATA        Keyhashdata;

struct _exec {
    unsigned char  type;
	int  (*pfunc)(DRVIN *drvin,DRVOUT *drvout);
};

static int MMI_init(void);
static int MMI_exec(DRVIN *drvin,DRVOUT *drvout);
static int MMI_KeyCheckApp(DRVIN *drvin,DRVOUT *drvout);
static int MMI_Decrypt8ByteSingleKey(DRVIN *pdrvin,DRVOUT *pdrvout);
int MMI_cipher_get_key(unsigned short appid,unsigned char array,unsigned char index,unsigned char *key,unsigned char ucDoubleFlag);
int MMI_keycrypt_imx(unsigned char *key,unsigned char crptdir,unsigned int keylen);
static int MMI_Decrypt8ByteDoubleKey(DRVIN *pdrvin,DRVOUT *pdrvout);
static int MMI_LoadSingleKeyUseSingleKey(DRVIN *pdrvin,DRVOUT *pdrvout);
int MMI_cipher_put_key(unsigned short appid,unsigned char array,unsigned char index,unsigned char *key,unsigned char ucDoubleFlag);
int key_search(unsigned short *a,unsigned short key, short low, short high);
int insert(unsigned short *a,unsigned short high,unsigned short key);
static int MMI_LoadSingleKeyUseDoubleKey(DRVIN *pdrvin,DRVOUT *pdrvout);
static int MMI_LoadDoubleUseDoubleKey(DRVIN *pdrvin,DRVOUT *pdrvout);
static int MMI_Encrypt8ByteSingleKey(DRVIN *pdrvin,DRVOUT *pdrvout);
static int MMI_Encrypt3DES(DRVIN *pdrvin,DRVOUT *pdrvout);
static int MMI_LoadProtectKey(DRVIN *pdrvin,DRVOUT *pdrvout);
int MMI_prot_put_key(unsigned char index,unsigned char *key);
int MMI_prot_get_key(unsigned char index,unsigned char *key);
static int MMI_LoadTransKeyUseProtectKey (DRVIN *pdrvin,DRVOUT *pdrvout);
unsigned char KEY_LoadTransKeyUseProtectKey_old(
	unsigned int uiAppIndex,			// 保存传输密钥的应用号
	unsigned char ucDecryptType,        // 解密方式
	unsigned char ucDecryptKeyIndex,    // 保护密钥索引
	unsigned char ucDestKeyArray,       // 存传输密钥行号
	unsigned char ucDestKeyIndex,       // 存传输密钥列号
	unsigned char *pucInData );
unsigned char Key_EncryptDataUseProtectKey_old(
	unsigned char ucProtectKeyIndex,
	unsigned char ucEncryptType,
	unsigned char *pucInData,
	unsigned char *pucOutData );
unsigned char KEY_LoadTransKeyUseAppKey_old(
	unsigned int uiTransKeyAppId,		// 保存传输密钥的应用号
	unsigned int uiDecryptKeyAppId,     // 解密传输密钥的应用号
	unsigned char ucDecryptKeyCol,      // 解密密钥行号
	unsigned char ucDecryptKeyRow,      // 解密密钥列号
	unsigned char ucDecryptType,        // 解密方式
	unsigned char ucDestKeyArray,       // 存传输密钥行号
	unsigned char ucDestKeyIndex,       // 存传输密钥列号
	unsigned char *pucInData );
unsigned char DYMN_Encrypt8ByteSingleKey_old(
	unsigned int uiAppIndex,
	unsigned char ucKeyArray,
	unsigned char ucKeyIndex,
	unsigned char *pucInData,
	unsigned char *pucOutData);
unsigned char DYMN_TEncryptKey_old(
	unsigned char ucEncryptType,
	unsigned int uiAppIndex,
	unsigned char ucKeyArray,
	unsigned char ucKeyIndex,
	unsigned char *pucInData,
	unsigned char *pucOutData);
int MMI_keycrypt_Hash(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata);	
static int MMI_EncryptHASH(DRVIN *pdrvin,DRVOUT *pdrvout);			

/*
1.类型  0x30(用保护密钥解压保存传输密钥) 0x31用存储的密钥解压保存传输密钥) 0x40(用保护密钥加解密)0x41(用存储的密钥加解密 )
2.保存传输密钥的应用号
3.解密传输密钥的应用号
4.解密密钥行号
5.解密密钥列号
6.解密方式      0x30 单des 单倍长  0x31 3des  0x32 3des双倍长  都是正向，不是反向
7.保护密钥索引
8.存传输密钥行号
9.存传输密钥列号
10.密钥
*/  




DRVOP  gbl_mmi_op = {
    .drv_init = MMI_init,
	.drv_exec = MMI_exec,
};

struct _exec exec_tbl[] = {
	{0x01,     MMI_KeyCheckApp},            // test 
	{0x3F,	   MMI_Decrypt8ByteSingleKey},  // test 
	{0x40,     MMI_Decrypt8ByteDoubleKey},   // test 
	{0x42,	   MMI_LoadSingleKeyUseSingleKey},  // test 
	{0x43,     MMI_LoadSingleKeyUseDoubleKey},  // test 
	{0x44,     MMI_LoadDoubleUseDoubleKey},     // test 
	{0x45,     MMI_LoadProtectKey},              // test 
  {0x46,     MMI_LoadTransKeyUseProtectKey},   
	{0x47,	   MMI_Encrypt8ByteSingleKey},  // test 
	{0x48,	   MMI_Encrypt3DES},     // test 
	{0x4a,	   MMI_EncryptHASH},     // test 
	{0xff,     0}
};





static int MMI_init(void)
{
	struct stat fst;
	int fd; 
	int retval = 0;
  unsigned char aucBuf[7];
	int length, res,infolen,listlen,keylen,protlen;
	int i;

//printf("**************************************************xiao bian ****fuck*********\n");
        
	if (stat(KEY_INFO_FILE, &fst) < 0)
	{   
		/* create new key file */
		memset(&Keyinfodata,0,sizeof(KEYSYSTEMINFO_DATA));
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,"201211",6);
    //Os__read_date(aucBuf); // DayDayMonthMonthYearYear\0
 #if MMI_DEB
//     printf("date=%s\n",aucBuf);
 #endif
     memcpy(&Keyinfodata.keycreatdatetime[0],&aucBuf[4],2);
     memcpy(&Keyinfodata.keycreatdatetime[2],&aucBuf[2],2);
     memcpy(&Keyinfodata.keycreatdatetime[4],&aucBuf[0],2);
     memset(aucBuf,0,sizeof(aucBuf));
    // Os__read_time_sec(aucBuf); //HourHourMinuteMinuteSecondSecond\0
     memcpy(aucBuf,"091011",6);
  #if MMI_DEB
//     printf("time=%s\n",aucBuf);
  #endif
     memcpy(&Keyinfodata.keycreatdatetime[6],&aucBuf[0],2);
     memcpy(&Keyinfodata.keycreatdatetime[8],&aucBuf[2],2);
     memcpy(&Keyinfodata.keycreatdatetime[10],&aucBuf[4],2);
    Keyinfodata.keyvalid = KEY_VALID_FLAG;
		
		if ((fd = open(KEY_INFO_FILE, O_CREAT|O_RDWR|O_TRUNC, 0777)) < 0 )
		{
			fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
			retval = -1;
			goto exit;
		}else
		{
			  infolen = sizeof(KEYSYSTEMINFO_DATA); 
		#if MMI_DEB
			  printf("infolen=%d\n",infolen);
		#endif
        if ((res = write(fd, &Keyinfodata, infolen)) != infolen) 
		    { 
		      fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		      retval = -1;
			    goto exit;
		   }
		   memset(&Keyappindexlist,0,sizeof(KEY_APPINDEXLIST));
		   listlen = sizeof(KEY_APPINDEXLIST);
		#if MMI_DEB
		   printf("listlen=%d\n",listlen);
		#endif
		   for(i=0;i<KEY_MAX_APPNUMBER;i++)
		   {
		       if ((res = write(fd, &Keyappindexlist, listlen)) != listlen) 
				   { 
				      fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
				      retval = -1;
					    goto exit;
				   }
		   } 
		   memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));
		   keylen = sizeof(KEYAPPKEYINFO_DATA);
		#if MMI_DEB
		   printf("keylen=%d\n",keylen);
		#endif
		   for(i=0;i<KEY_MAX_APPNUMBER*KEY_MAX_KEYNUMBER;i++)
		   {
		       if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
				   { 
				      fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
				      retval = -1;
					    goto exit;
				   }
		   } 
		   memset(&Keyprotdata,0,sizeof(KEYPROTINFO_DATA));
		   protlen = sizeof(KEYPROTINFO_DATA);
	#if MMI_DEB
		   printf("protlen=%d\n",protlen);
  #endif
		   for(i=0;i<KEY_MAX_PROTNUMBER;i++)
		   {
		       if ((res = write(fd, &Keyprotdata, protlen)) != protlen) 
				   { 
				      fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
				      retval = -1;
					    goto exit;
				   }
		   } 		   		   
		}
		close(fd);
	}else
	{
		    //printf("exist \n ");
	}
      	

exit:
	//printf("");

	close(fd);
	return retval;
}

static int MMI_exec(DRVIN *drvin,DRVOUT *drvout)
{
    int retval =0;
    struct _exec *pexec ;

        pexec = pexec = (struct _exec *)exec_tbl;

	while(pexec->type != 0xff){
		if (drvin->ucType == pexec->type){ /* type is matched */
			if (pexec->pfunc){
				retval = (pexec->pfunc)(drvin,drvout);/* excute this function */
				return retval;
			}
		}
		pexec++;
	}

    fprintf(stderr,"[%s:%d] Encryption command %02x not support \n",__FUNCTION__,__LINE__,drvin->ucType);
    drvout->ucGenStatus = DRVST_ENDED;
	drvout->ucDrvStatus = 1;//NOT OK

	return (DRVERR_BADCMD);
}

static int MMI_KeyCheckApp(DRVIN *drvin,DRVOUT *drvout)
{
     unsigned char ucAppCount=0;
     unsigned char aucAppId[KEY_MAX_APPNUMBER*2+1];
     unsigned short auiAppId[KEY_MAX_APPNUMBER];
     unsigned int ucI,ucJ,ucK,ucM;
     int fd; 
     int nbytes;
     int offset;
     int length, res,infolen,listlen,keylen;
     int retval = 0;
     
#if MMI_DEB    
	    printf("\ndrvin->ucLen=%d",drvin->ucLen);
#endif
	 
	   if(drvin->ucLen>KEY_MAX_APPNUMBER*2)
	   	  return (-1);
     if(drvin->ucLen)
	      ucAppCount = drvin->ucLen/2;
	      
	   if(ucAppCount>=KEY_MAX_APPNUMBER)
	   	   ucAppCount = KEY_MAX_APPNUMBER;
	   
	  // memcpy(drvin->aucData,"\x00\x68",2);
     memcpy(aucAppId,drvin->aucData,drvin->ucLen);
     memset(auiAppId,0,sizeof(auiAppId));
     for( ucI=0,ucJ=0;ucI<ucAppCount;ucJ+=2,ucI++ )
     {
		   auiAppId[ucI] = aucAppId[ucJ]*256+aucAppId[ucJ+1];
		   printf("\n1auiAppId[%d]=%d",ucI,auiAppId[ucI]);
		 }

     if((fd = open(KEY_INFO_FILE,O_RDWR)) < 0)
     {
		      fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		      retval = -1;
			    goto exit;
	   }else
	   {
	        memset(&Keyinfodata,0,sizeof(KEYSYSTEMINFO_DATA));
	        infolen = sizeof(KEYSYSTEMINFO_DATA);
	        listlen = sizeof(KEY_APPINDEXLIST); 
	        keylen = sizeof(KEYAPPKEYINFO_DATA);	
	     #if MMI_DEB
	        printf("\nucAppCount=%d",ucAppCount);
	     #endif
	   	    if ((res = read(fd,&Keyinfodata,infolen)) !=  infolen)
	   	    {
	   	    	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
					      retval = -1;
						    goto exit;
	   	    }
	   	#if MMI_DEB
	   	    printf("\nKeyinfodata.keyvalid=%02x",Keyinfodata.keyvalid);
	   	#endif
	   	    if((Keyinfodata.keyvalid != KEY_VALID_FLAG)&&(Keyinfodata.keyvalid != KEY_INVALID_FLAG))
	   	    {
	   	    	    printf("key system is error\n ");
	   	    	    retval = -1;
						    goto exit;
	   	    }
	        if(Keyinfodata.keyvalid == KEY_VALID_FLAG)
	        {
	     #if MMI_DEB
	        	  printf("\nin to clear key");
	     #endif
	          
	           
	           listlen = sizeof(KEY_APPINDEXLIST);
	           for(ucJ=0,ucK=0;ucJ<KEY_MAX_APPNUMBER;ucJ++)
	           {
	           	   lseek(fd,infolen+listlen*ucJ,SEEK_SET);
	           	   memset(&Keyappindexlist,0,sizeof(KEY_APPINDEXLIST));
   	    	   	   if ((res = read(fd,&Keyappindexlist,listlen)) !=  listlen)
   	    	   	   {
   	    	   	        fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
								      retval = -1;
									    goto exit; 
   	    	   	   }
	           	   if(Keyappindexlist.keyvalid == KEY_VALID_FLAG)
	           	   {	           	   	    
	           	   	    for(ucI=0;ucI<ucAppCount;ucI++)
	           	   	    {
	           	   	    	   if(Keyappindexlist.keyappid == auiAppId[ucI])
	           	   	    	   	   break;
	           	   	    }
	           	   	    if(ucI == ucAppCount)
	           	   	    {
	           	   	    	    memset(&Keyappindexlist,0,sizeof(KEY_APPINDEXLIST));
	   	    	   	            lseek(fd,0 - listlen,SEEK_CUR);
	   	    	   	            if ((res = write(fd, &Keyappindexlist, listlen)) != listlen) 
	   	    	   	            {
	   	    	   	                fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
													      retval = -1;
														    goto exit;
	   	    	   	            }	
	   	    	   	            lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen,SEEK_SET);	
	   	    	   	            memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));
	   	    	   	            keylen = sizeof(KEYAPPKEYINFO_DATA);	
	   	    	   	            for(ucM=0;ucM<KEY_MAX_KEYNUMBER;ucM++)
	   	    	   	            {			   	    	   	                 
	   	    	   	                 if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
			   	    	   	             {
			   	    	   	                fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
															      retval = -1;
																    goto exit;
			   	    	   	             }
	   	    	   	            }
	   	    	   	            if(Keyinfodata.appnumber)
			   	    	   	            Keyinfodata.appnumber--;	   	     
	           	   	    }else
	           	   	    {
	           	   	    	 ucK++;
	           	   	    }
	           	   }
	           	   if(ucK >= Keyinfodata.appnumber)
			   	    	   	 break;	
	           }
	           
	     
	     
	     
	            
			   	    lseek(fd,0,SEEK_SET);
			   	#if MMI_DEB
			   	    printf("\nrewrite keyinfo");
			   	#endif
			   	    if ((res = write(fd, &Keyinfodata, infolen)) != infolen) 
             {
                fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
					      retval = -1;
						    goto exit;
             }
	        }
	   }
	   
	exit:
	   printf("");

	close(fd);
	drvout->ucGenStatus = DRVST_ENDED;
	drvout->ucDrvStatus = KEY_DRVSTATUS_OK;//OK
	drvout->aucData[0] = 0x30;
	if(retval == -1)
		drvout->aucData[0] = 0x31;
	return retval;   

}

static int MMI_Decrypt8ByteSingleKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
  int  retval=0;
	unsigned char array,index;
	unsigned char key[KEY_MAX_KEYDATA*2],decrypt_data[KEY_MAX_KEYDATA],decrypted_data[KEY_MAX_KEYDATA];
	unsigned short i;
	
	

#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif
     
	array = pdrvin->aucData[KEY_PACKAGE_START]-0x30;
	index = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;
	asc_hex(decrypt_data,KEY_MAX_KEYDATA,&(pdrvin->aucData[KEY_PACKAGE_START+2]),KEY_MAX_KEYDATA*2);
#if MMI_DEB	
	printf("\narray=%02x,index=%02x,ucOSCurrAppID=%d",array,index,ucOSCurrAppID);
	printf("\nkeydata=%16s",&(pdrvin->aucData[KEY_PACKAGE_START+2]));
#endif	
	
	
    /* load key from key file */
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,index,key,KEY_DOUBLE_NO) ){
		pdrvout->ucGenStatus = DRVST_ENDED;
		pdrvout->ucDrvStatus = KEY_DRVSTATUS_NOK;//NOT OK
		pdrvout->aucData[0] = 0x31;
		return (-1);
	}
#if MMI_DEB
	printf("\nMMI_cipher_get_key ok");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		  printf(" %02x",key[i]);
	}
#endif
	if(0 != MMI_keycrypt_imx(key,SCC_DECRYPT,KEY_MAX_KEYDATA)){
	       pdrvout->ucGenStatus = DRVST_ENDED;
		pdrvout->ucDrvStatus = KEY_DRVSTATUS_NOK;//NOT OK
		pdrvout->aucData[0] = 0x31;
                return (-1);
		} 
#if MMI_DEB 
	printf("\nMMI_keycrypt_imx ok\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		  printf(" %02x",key[i]);
	}
#endif 		
	/* des decypt */
	desm1(decrypt_data,decrypted_data,key);
	memset(key,0,sizeof(key)); //clear plain text
    /* put into out buffer */
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = KEY_DRVSTATUS_OK;//OK
	pdrvout->aucData[0] = 0x30;
	
#if MMI_DEB
	printf("\ndecrypted_data\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		  printf(" %02x",decrypted_data[i]);
	}
#endif
	hex_asc(&(pdrvout->aucData[1]),decrypted_data,KEY_MAX_KEYDATA*2);
	return retval;

}



int MMI_cipher_get_key(unsigned short appid,unsigned char array,unsigned char index,unsigned char *key,unsigned char ucDoubleFlag)
{
	int  fd;
	int ret;
	unsigned int ucI,ucJ,ucK;
	unsigned int uiI;	
	unsigned char ucFlag =0;
	unsigned short usmarker;
	int result;
	int length, res,infolen,listlen,keylen;
	int retval = 0;

	ret = 0;


     memset(key,0,KEY_MAX_KEYDATA);
#if MMI_DEB
     printf("\nMMI_cipher_get_key ");
#endif	
	    if((fd = open(KEY_INFO_FILE,O_RDONLY)) < 0)
     {
		      fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		      retval = -1;
			    goto exit;
	   }else
	   {
	        memset(&Keyinfodata,0,sizeof(KEYSYSTEMINFO_DATA));
	        infolen = sizeof(KEYSYSTEMINFO_DATA);
	        listlen = sizeof(KEY_APPINDEXLIST); 
	        keylen = sizeof(KEYAPPKEYINFO_DATA);
#if MMI_DEB
	        printf("\ninfolen=%d",infolen);
#endif
	   	    if ((res = read(fd,&Keyinfodata,infolen)) !=  infolen)
	   	    {
	   	    		printf("%s  %d line  res = %d , infolen = %d\n", __func__, __LINE__, res, infolen);
	   	    	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
					      retval = -1;
						    goto exit;
	   	    }
#if MMI_DEB	   	   
	   	    printf("\nKeyinfodata.keyvalid=%02x",Keyinfodata.keyvalid);
#endif	   	    
	   	    if((Keyinfodata.keyvalid != KEY_VALID_FLAG)&&(Keyinfodata.keyvalid != KEY_INVALID_FLAG))
	   	    {
	   	    	    printf("key system is error\n ");
	   	    	    retval = -1;
						    goto exit;
	   	    }
	        if(Keyinfodata.keyvalid == KEY_VALID_FLAG)
	        {
             lseek(fd,infolen,SEEK_SET);			   	         
             listlen = sizeof(KEY_APPINDEXLIST);
             //printf("\nlistlen=%d",listlen);
   	    	   for(ucJ=0,ucK=0;ucJ<KEY_MAX_APPNUMBER;ucJ++)
   	    	   {
#if MMI_DEB   	    	   	
   	    	   //	   printf("\nucJ=%02x,ucK=%02x",ucJ,ucK);
#endif   	    	   	   
   	    	       memset(&Keyappindexlist,0,sizeof(KEY_APPINDEXLIST));
   	    	   	   if ((res = read(fd,&Keyappindexlist,listlen)) !=  listlen)
   	    	   	   {
   	    	   	        fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
								      retval = -1;
									    goto exit; 
   	    	   	   }
#if MMI_DEB   	    	   	   
   	    	   	   printf("\nKeyappindexlist.keyvalid=%02x,Keyappindexlist.keyappid=%d",Keyappindexlist.keyvalid,Keyappindexlist.keyappid); 
#endif  	    	   	     	    	   	
   	    	   	   if(Keyappindexlist.keyvalid == KEY_VALID_FLAG)
   	    	   	   {
   	    	   	        ucK++;
#if MMI_DEB   	    	   	        
   	    	   	        printf("\nappid=%d",appid);
#endif  	    	   	        
   	    	   	        if(Keyappindexlist.keyappid == appid)
   	    	   	        {
   	    	   	               if(ucDoubleFlag == KEY_DOUBLE_NO)
   	    	   	        	       usmarker = array*256+index;
				         else
						  usmarker = array*256+index*2;
#if MMI_DEB   	    	   	        	  
   	    	   	        	  printf("\nusmarker=%d,Keyappindexlist.keynumber=%d",usmarker,Keyappindexlist.keynumber);   
#endif   	    	   	        	 	    	   	      
   	    	   	        	  if(Keyappindexlist.keynumber>0)  	    	   	        	  	
   	    	   	        	      result = key_search(Keyappindexlist.keymarker,usmarker,0,(Keyappindexlist.keynumber-1));
   	    	   	        	  else
   	    	   	        	  	  result = key_search(Keyappindexlist.keymarker,usmarker,0,0);
#if MMI_DEB   	    	   	        	  	  
   	    	   	        	  printf("\nresult=%d\n",result);
#endif   	    	   	        	  
   	    	   	        	  if(result == -1)
   	    	   	        	  {
   	    	   	        	  	 break;
   	    	   	        	  }else
   	    	   	        	  {   	
   	    	   	        	  	  keylen = sizeof(KEYAPPKEYINFO_DATA);	
   	    	   	        	  	  memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));    	   	  
   	    	   	                lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen+keylen*Keyappindexlist.keyindex[result],SEEK_SET);
   	    	   	                if ((res = read(fd,&Keyappdata,keylen)) !=  keylen)
   	    	   	                {
   	    	   	                	  printf("key system is error\n ");
										   	    	    retval = -1;
															    goto exit;
   	    	   	                }
   	    	   	                if(Keyappdata.keyvalid == KEY_VALID_FLAG)
   	    	   	                {
   	    	   	                    memcpy(key,Keyappdata.keydata,KEY_MAX_KEYDATA);
   	    	   	                    if(ucDoubleFlag == KEY_DOUBLE_NO)
   	    	   	                    {
   	    	   	                    	  ucFlag = 1;
   	    	   	                    	  break;
   	    	   	                    }
   	    	   	                }else
   	    	   	                {
   	    	   	                	 //  if(ucDoubleFlag == KEY_DOUBLE_NO)
   	    	   	                	   	   break; 
   	    	   	                }
   	    	   	                memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));    	   	  
   	    	   	                lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen+keylen*Keyappindexlist.keyindex[result+1],SEEK_SET);
   	    	   	                if ((res = read(fd,&Keyappdata,keylen)) !=  keylen)
   	    	   	                {
   	    	   	                	  printf("key system is error\n ");
										   	    	    retval = -1;
															    goto exit;
   	    	   	                }
   	    	   	                if(Keyappdata.keyvalid == KEY_VALID_FLAG)
   	    	   	                {
   	    	   	                    memcpy(&key[KEY_MAX_KEYDATA],Keyappdata.keydata,KEY_MAX_KEYDATA);   	    	   	                    
   	    	   	                    ucFlag = 1;  	    	   	                	   	                   
   	    	   	                }
   	    	   	                break;
   	    	   	        	  }   	    	   	        	    	
   	    	   	        }
   	    	   	   }
   	    	   	   if(ucK >= Keyinfodata.appnumber)
   	    	   	       break;			   	    	   	   			   	    	   	  
   	    	   }			   	    
	        }
	   }
	
	if(ucFlag == 0)
	{
		    printf("key  is not exist\n ");
		    memcpy(key,KEY_DEFAULTVALUE1,KEY_MAX_KEYDATA);
		    if(ucDoubleFlag == KEY_DOUBLE_YES)
		    	 memcpy(&key[KEY_MAX_KEYDATA],KEY_DEFAULTVALUE2,KEY_MAX_KEYDATA);  		    
  	  //   close(fd);
		  //  return (0);
	}
	
	exit:
	   printf("");

#if MMI_DEB	
	int i,j;
	printf("get key = ");
	j = KEY_MAX_KEYDATA;
	if(ucDoubleFlag == KEY_DOUBLE_YES)
		 j *= 2;
	for(i=0;i<j;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif	
	close(fd);
	return retval;
}


int MMI_keycrypt_imx(unsigned char *key,unsigned char crptdir,unsigned int keylen)
{
   int fd;
   int ret;
 	
 //	printf("\n\n     MMI_keycrypt_imx return [0] of 0717   \n\n");
 //	return 0;
 
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
	    return -1;
   }
	ret = ioctl(fd,IOCTL_ENCRYT_PCITMK,&Keycryptdata);
//#if MMI_DEB
	printf("\nret=%d,keylen=%dKeycryptdata.cryptkeyout[0]=%02x\n",ret,keylen,Keycryptdata.cryptkeyout[0]);
//#endif
	if(ret == KEYCRYPT_OK)
	{
	    memset(key,0,sizeof(key));
	    memcpy(key,Keycryptdata.cryptkeyout,keylen);
	}else
	    printf("keycrpyt error = %d\n",ret);
	close(fd); 
	    
	return ret;	
}

static int MMI_Decrypt8ByteDoubleKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
  int  retval=0;
	unsigned char array,index;
	unsigned char key[KEY_MAX_KEYDATA*2],decrypt_data[KEY_MAX_KEYDATA],decrypted_data[KEY_MAX_KEYDATA];
	unsigned char i;
	
#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif
	array = pdrvin->aucData[KEY_PACKAGE_START]-0x30;
	index = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;  
  	asc_hex(decrypt_data,KEY_MAX_KEYDATA,&(pdrvin->aucData[KEY_PACKAGE_START+2]),KEY_MAX_KEYDATA*2);
#if MMI_DEB  
  printf("\narray=%02x,index=%02x,ucOSCurrAppID=%d,decrypt_data=%s",array,index,ucOSCurrAppID,&(pdrvin->aucData[KEY_PACKAGE_START+2]));
#endif
     /* load key from key file */
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,index,key,KEY_DOUBLE_YES) ){
		pdrvout->ucGenStatus = DRVST_ENDED;
		pdrvout->ucDrvStatus = KEY_DRVSTATUS_NOK;//NOT OK
		pdrvout->aucData[0] = 0x31;
		return (-1);
	}
#if MMI_DEB
	printf("\nMMI_cipher_get_key ok\n");

	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	if(0 != MMI_keycrypt_imx(key,SCC_DECRYPT,KEY_MAX_KEYDATA*2)){
	       pdrvout->ucGenStatus = DRVST_ENDED;
		pdrvout->ucDrvStatus = KEY_DRVSTATUS_NOK;//NOT OK
		pdrvout->aucData[0] = 0x31;
                return (-1);
		}
#if MMI_DEB  
		printf("\nMMI_keycrypt_imx ok\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	/* 3des decypt */
	desm1(decrypt_data,decrypted_data,key);
	des(decrypted_data,decrypt_data,&key[KEY_MAX_KEYDATA]);
	desm1(decrypt_data,decrypted_data,key);
	memset(key,0,sizeof(key));
    /* put into out buffer */
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = KEY_DRVSTATUS_OK;//OK
	pdrvout->aucData[0] = 0x30;
	hex_asc(&(pdrvout->aucData[1]),decrypted_data,KEY_MAX_KEYDATA*2);
#if MMI_DEB
	printf("\nDecrypt outdata\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
#endif
	return retval;
}

static int MMI_LoadSingleKeyUseSingleKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
    int  retval=0;
	unsigned char array,src_idx,dest_idx;
	unsigned char key[KEY_MAX_KEYDATA*2],decrypt_data[KEY_MAX_KEYDATA],decrypted_data[KEY_MAX_KEYDATA*2];
	unsigned char i;
       
	
  memset(key,0,sizeof(key));
	memset(decrypt_data,0,sizeof(decrypt_data));
	memset(decrypted_data,0,sizeof(decrypted_data));
	
#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif

	array    = pdrvin->aucData[KEY_PACKAGE_START]-0x30;
	src_idx  = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;
	dest_idx = pdrvin->aucData[KEY_PACKAGE_START+2]-0x30; 
	asc_hex(decrypt_data,KEY_MAX_KEYDATA,&(pdrvin->aucData[KEY_PACKAGE_START+3]),KEY_MAX_KEYDATA*2);
#if MMI_DEB
	printf("\narray=%02x,src_idx=%02x,dest_idx=%02x,ucOSCurrAppID=%d,decrypt_data=%s",array,src_idx,dest_idx,ucOSCurrAppID,&(pdrvin->aucData[KEY_PACKAGE_START+3]));
#endif
  
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,src_idx,key,KEY_DOUBLE_NO) ){
		retval = -1;
		goto exit;
	}
#if MMI_DEB
	printf("\nMMI_cipher_get_key\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	if( 0 != MMI_keycrypt_imx(key,SCC_DECRYPT,KEY_MAX_KEYDATA)){
		retval = -1;
    goto exit;
	}         //decrypt key 
#if MMI_DEB
	printf("\nMMI_keycrypt_imx\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	/* des decypt */
	desm1(decrypt_data,decrypted_data,key);
	memset(key,0,sizeof(key));	
#if MMI_DEB
	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif
	if( 0 != MMI_keycrypt_imx(decrypted_data,SCC_ENCRYPT,KEY_MAX_KEYDATA)){
		retval = -1;
    goto exit;
	}         //encrypt for save
#if MMI_DEB
	printf("\nMMI_keycrypt_imx\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
#endif
	/* put the decrypt data into dest index */
	if ( 0 != MMI_cipher_put_key(ucOSCurrAppID,array,dest_idx,decrypted_data,KEY_DOUBLE_NO) ){
		retval = -1;
		goto exit;
	}
    /* put result into out buffer */
	retval = 0;
exit:	
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = -retval; 
	pdrvout->aucData[0] = 0x30;
	if(retval == -1)
	pdrvout->aucData[0] = 0x31;
	return retval;
}

int MMI_cipher_put_key(unsigned short appid,unsigned char array,unsigned char index,unsigned char *key,unsigned char ucDoubleFlag)
{
	int  fd,retval=0;
	int  nbytes;
	int i,j,result;
	int length, res,infolen,listlen,keylen;
	unsigned int ucI,ucJ,ucK,ucM,ucN;
	unsigned short usmarker;
	unsigned char ucContinueFlag = 0;
	unsigned char ucFlag =0;
	
    
#if MMI_DEB


   printf("\n***************************\n");
	j = KEY_MAX_KEYDATA;
	if(ucDoubleFlag == KEY_DOUBLE_YES)
		 j *= 2;
	for(i=0;i<j;i++){
		printf("%02x ",key[i]);
	}
	printf("\n***************************\n");

#endif	
	if ( (fd = open(KEY_INFO_FILE,O_RDWR)) < 0)
	{
		  printf("%s:%02x\n",__func__,errno);
		  return (-1);
	}else
	{
	    memset(&Keyinfodata,0,sizeof(KEYSYSTEMINFO_DATA));
      infolen = sizeof(KEYSYSTEMINFO_DATA);
	        listlen = sizeof(KEY_APPINDEXLIST); 
	        keylen = sizeof(KEYAPPKEYINFO_DATA);
	       
 
 	    if ((res = read(fd,&Keyinfodata,infolen)) !=  infolen)
 	    {
    	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		      retval = -1;
			    goto exit;
 	    }
#if MMI_DEB
 	    printf("\nKeyinfodata.keyvalid=%02x",Keyinfodata.keyvalid);
#endif 	    
 	    if((Keyinfodata.keyvalid != KEY_VALID_FLAG)&&(Keyinfodata.keyvalid != KEY_INVALID_FLAG))
 	    {
    	    //printf("key system is error\n ");
    	    retval = -1;
			    goto exit;

 	    }
 	    if(Keyinfodata.keyvalid == KEY_VALID_FLAG)
 	    {
 	    	  lseek(fd,infolen,SEEK_SET);			   	         
          listlen = sizeof(KEY_APPINDEXLIST);
          keylen = sizeof(KEYAPPKEYINFO_DATA);	
           if(ucDoubleFlag == KEY_DOUBLE_NO)
   	    	   	        	       usmarker = array*256+index;
				         else
						  usmarker = array*256+index*2;
          
    	    for(ucJ=0,ucK=0;ucJ<KEY_MAX_APPNUMBER;ucJ++)
    	    {
    	    	 //printf("\nucJ=%02x,ucK=%02x",ucJ,ucK);
    	    	 memset(&Keyappindexlist,0,sizeof(KEY_APPINDEXLIST));
    	   	   if ((res = read(fd,&Keyappindexlist,listlen)) !=  listlen)
    	   	   {
    	   	        fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
						      retval = -1;
							    goto exit; 
    	   	   }
#if MMI_DEB    	   	  
    	   	   printf("\nKeyappindexlist.keyvalid=%d,Keyappindexlist.keyappid=%d,appid=%d",Keyappindexlist.keyvalid,Keyappindexlist.keyappid,appid);
#endif 
    	   	   if(Keyappindexlist.keyvalid == KEY_VALID_FLAG)
    	   	   { 	   	   	   
    	   	   	   if(Keyappindexlist.keyappid == appid)
    	   	   	   {    	   	   	   	   
  	   	        	   if(Keyappindexlist.keynumber>0)  	    	   	        	  	
		  	   	        	   result = insert(Keyappindexlist.keymarker,Keyappindexlist.keynumber-1,usmarker);
		  	   	         else
		  	   	        	   result = insert(Keyappindexlist.keymarker,0,usmarker);
#if MMI_DEB		  	   	        	   
		  	   	         printf("\nresult=%d",result);
 #endif
  	   	        	   if(result != -1)
  	   	        	   {
  	   	        	   	       
  	   	        	   	       if(ucDoubleFlag == KEY_DOUBLE_YES)
  	   	        	   	       {
  	   	        	   	       	  if(Keyappindexlist.keynumber >= (KEY_MAX_KEYNUMBER-1))
  	   	        	   	       	  {
  	   	        	   	       	  	     printf("key system is full\n ");
												   	    	    retval = -1;
																	    goto exit;
  	   	        	   	       	  }
  	   	        	   	       	  ucM = Keyappindexlist.keynumber+1;
  #if MMI_DEB
  	   	        	   	       	  printf("\nresult=%d,ucM=%d",result,ucM);
  	#endif
  	   	        	   	       	  for(ucI=0;ucI<Keyappindexlist.keynumber-result;ucI++)
	  	   	        	   	   	    {
	  	   	        	   	   	   	    Keyappindexlist.keymarker[ucM-ucI] = Keyappindexlist.keymarker[ucM-ucI-2];
	  	   	        	   	   	   	    Keyappindexlist.keyindex[ucM-ucI] =  Keyappindexlist.keyindex[ucM-ucI-2];	   	        	   	   	   	    
	  	   	        	   	   	    }
	  	   	        	   	   	    Keyappindexlist.keynumber+=2;
  	   	        	   	       }else
  	   	        	   	       {
  	   	        	   	       	  if(Keyappindexlist.keynumber >= (KEY_MAX_KEYNUMBER))
  	   	        	   	       	  {
  	   	        	   	       	  	    printf("key system is full\n ");
												   	    	    retval = -1;
																	    goto exit;
  	   	        	   	       	  }
  	   	        	   	       	  ucM = Keyappindexlist.keynumber;
  #if MMI_DEB
  	   	        	   	       	  printf("\nresult=%d,ucM=%d",result,ucM);
  #endif
  	   	        	   	       	  for(ucI=0;ucI<ucM-result;ucI++)
	  	   	        	   	   	    {
	  	   	        	   	   	   	    Keyappindexlist.keymarker[ucM-ucI] = Keyappindexlist.keymarker[ucM-ucI-1];
	  	   	        	   	   	   	    Keyappindexlist.keyindex[ucM-ucI] =  Keyappindexlist.keyindex[ucM-ucI-1];	   	        	   	   	   	    
	  	   	        	   	   	    } 
	  	   	        	   	   	    Keyappindexlist.keynumber++;
  	   	        	   	       }  	   	        	   	        	   	        	   	   	 	   	        	   	   	   
  	   	        	   	   	   lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen,SEEK_SET);  
  	   	        	   	   	   for(ucN=0;ucN<KEY_MAX_KEYNUMBER;ucN++)
  	   	        	   	   	   {
  	   	        	   	   	   	   keylen = sizeof(KEYAPPKEYINFO_DATA);	
   	    	   	        	  	     memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA)); 
   	    	   	        	  	     if((res = read(fd,&Keyappdata,keylen)) !=  keylen)
   	    	   	        	  	     {
   	    	   	        	  	     	    printf("key system is error\n ");
												   	    	    retval = -1;
																	    goto exit;
   	    	   	        	  	     }
   #if MMI_DEB
   	    	   	        	  	      printf("\nkeylen=%d,Keyappdata.keyvalid=%02x",keylen,Keyappdata.keyvalid);
   #endif
   	    	   	        	  	     if(Keyappdata.keyvalid == KEY_INVALID_FLAG)
   	    	   	        	  	     {
   	    	   	        	  	     	    lseek(fd,0-keylen,SEEK_CUR);
   	    	   	        	  	     	    memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));  
				   	    	   	        	  	  Keyappdata.keyvalid = KEY_VALID_FLAG;
				   	    	   	        	  	  if(ucContinueFlag == 1)
				   	    	   	        	  	  {
				   	    	   	        	  	      memcpy(Keyappdata.keydata,&key[KEY_MAX_KEYDATA],KEY_MAX_KEYDATA);
				   	    	   	        	  	      Keyappindexlist.keymarker[result+ucContinueFlag] = usmarker+1;
				   	    	   	        	  	      printf("Keyappindexlist.keymarker[result+ucContinueFlag] = usmarker+1;\n");
				   	    	   	        	  	  }	 
				   	    	   	        	  	  else
				   	    	   	        	  	  {
				   	    	   	        	  	      memcpy(Keyappdata.keydata,key,KEY_MAX_KEYDATA); 
				   	    	   	        	  	      Keyappindexlist.keymarker[result+ucContinueFlag] = usmarker;
				   	    	   	        	  	      printf("Keyappindexlist.keymarker[result+ucContinueFlag] = usmarker;\n");
				   	    	   	        	  	  }
				   	    	   	        	  	     
				   	    	   	        	  	  
				   	    	   	        	  	  Keyappindexlist.keyindex[result+ucContinueFlag] = ucN;
				   	    	   	        	  	  if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
				   	    	   	                {
				   	    	   	                	  printf("key system is error\n ");
														   	    	    retval = -1;
																			    goto exit;
				   	    	   	                }
				   	    	   	                if(ucDoubleFlag != KEY_DOUBLE_YES)
				   	    	   	                	break;
				   	    	   	                else
				   	    	   	                {
				   	    	   	                	  if(ucContinueFlag == 1)
				   	    	   	                	  	break;
				   	    	   	                	  ucContinueFlag = 1;
				   	    	   	                }
   	    	   	        	  	     }
  	   	        	   	   	   }
  	   	        	   	   	   lseek(fd,infolen+listlen*ucJ,SEEK_SET);
  	   	        	   	   	   if ((res = write(fd, &Keyappindexlist, listlen)) != listlen) 
   	    	   	                {
   	    	   	                	  printf("key system is error\n ");
										   	    	    retval = -1;
															    goto exit;
   	    	   	                }
  	   	        	   	   ucFlag = 1;	
  	   	        	   	   break;	  	   	        	   
  	   	        	   }else
  	   	        	   {
  	   	        	   	        result = key_search(Keyappindexlist.keymarker,usmarker,0,Keyappindexlist.keynumber-1);
  	   	        	   	        keylen = sizeof(KEYAPPKEYINFO_DATA);	
   	    	   	        	  	  memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));  
   	    	   	        	  	  Keyappdata.keyvalid = KEY_VALID_FLAG;
   	    	   	        	  	  memcpy(Keyappdata.keydata,key,KEY_MAX_KEYDATA);  	   	  
   	    	   	                lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen+keylen*Keyappindexlist.keyindex[result],SEEK_SET);
   #if MMI_DEB
   	    	   	                printf("\n111keylen=%d,result=%d",keylen,result);
   	#endif
   	    	   	                if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
   	    	   	                {
   	    	   	                	  printf("key system is error\n ");
										   	    	    retval = -1;
															    goto exit;
   	    	   	                }
   	    	   	                if(ucDoubleFlag == KEY_DOUBLE_NO)
    	   	                    {
    	   	                    	  ucFlag = 1;
    	   	                    	  break;
    	   	                    }  	    	   	               
   	    	   	                memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));  
   	    	   	                Keyappdata.keyvalid = KEY_VALID_FLAG;
   	    	   	        	  	  memcpy(Keyappdata.keydata,&key[KEY_MAX_KEYDATA],KEY_MAX_KEYDATA);    	   	  
   	    	   	                lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen+keylen*Keyappindexlist.keyindex[result+1],SEEK_SET);
   	    	   	                
   	    	   	                if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
   	    	   	                {
   	    	   	                	  printf("key system is error\n ");
										   	    	    retval = -1;
															    goto exit;
   	    	   	                }
   	#if MMI_DEB
   	    	   	                printf("\nres=%d",res);
   	#endif
   	    	   	                ucFlag = 1;
    	   	                    break;
  	   	        	   }
    	   	   	   }
    	   	   	   ucK++;
    	   	   }
    	   	   if(ucK >= Keyinfodata.appnumber)
   	    	   	   break;		
    	    }
    	    if(ucJ == KEY_MAX_APPNUMBER)
    	    {
    	    	  //printf("key system app is full\n ");
   	    	    retval = -1;
					    goto exit;
    	    }
    	    if(ucK >= Keyinfodata.appnumber)
    	    {
    	    	  lseek(fd,infolen,SEEK_SET);	
    	
    	    	  for(ucJ=0;ucJ<KEY_MAX_APPNUMBER;ucJ++)
    	    	  {
    	    	  	 memset(&Keyappindexlist,0,sizeof(KEY_APPINDEXLIST));
    	 #if MMI_DEB
    	    	  	 printf("\nucJ=%02x",ucJ);
    	 #endif
    	   	       if ((res = read(fd,&Keyappindexlist,listlen)) !=  listlen)
    	   	       {
		    	   	        fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
								      retval = -1;
									    goto exit; 
    	   	       }
    #if MMI_DEB
    	   	       printf("\nKeyappindexlist.keyvalid=%02x",Keyappindexlist.keyvalid);
    #endif	
    	   	       if(Keyappindexlist.keyvalid == KEY_INVALID_FLAG)
    	   	       {
    	   	       	   keylen = sizeof(KEYAPPKEYINFO_DATA);	
   	        	  	   memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));  
   	        	  	   Keyappdata.keyvalid = KEY_VALID_FLAG;
   	        	  	   memcpy(Keyappdata.keydata,key,KEY_MAX_KEYDATA); 
    	   	       	   lseek(fd,infolen+listlen*KEY_MAX_APPNUMBER+ucJ*KEY_MAX_KEYNUMBER*keylen,SEEK_SET);
   	                 if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
   	                 {
   	                	  printf("key system is error\n ");
					   	    	    retval = -1;
										    goto exit;
   	                 } 
   	             	                 
    	   	       	   Keyappindexlist.keyvalid = KEY_VALID_FLAG;
    	   	       	   Keyappindexlist.keyappid = appid;
    	   	       	   Keyappindexlist.keynumber++;
    	   	       	   Keyappindexlist.keymarker[0] = usmarker;
    	   	       	   Keyappindexlist.keyindex[0] = 0;
    	   	       	   if(ucDoubleFlag == KEY_DOUBLE_YES)
    	   	       	   {
    	   	       	   	  Keyappindexlist.keynumber++;
    	   	       	      Keyappindexlist.keymarker[1] = usmarker+1;
    	   	       	      printf("first load\n");
    	   	       	      Keyappindexlist.keyindex[1] = 1;
    	   	       	      memset(&Keyappdata,0,sizeof(KEYAPPKEYINFO_DATA));  
		   	        	  	   Keyappdata.keyvalid = KEY_VALID_FLAG;
		   	        	  	   memcpy(Keyappdata.keydata,&key[KEY_MAX_KEYDATA],KEY_MAX_KEYDATA); 
		   	                 if ((res = write(fd, &Keyappdata, keylen)) != keylen) 
		   	                 {
		   	                	  printf("key system is error\n ");
							   	    	    retval = -1;
												    goto exit;
		   	                 }
    	   	       	   }
    	   	       	  
    	   	       	   lseek(fd,infolen+listlen*ucJ,SEEK_SET);
    	   	       	   if ((res = write(fd, &Keyappindexlist, listlen)) != listlen) 
   	                 {
   	                	  printf("key system is error\n ");
					   	    	    retval = -1;
										    goto exit;
   	                 }
   	               
   	                 lseek(fd,0,SEEK_SET);
   	                 Keyinfodata.appnumber++;
   	                 if ((res = write(fd, &Keyinfodata, infolen)) != infolen) 
   	                 {
   	                	  printf("key system is error\n ");
					   	    	    retval = -1;
										    goto exit;
   	                 }
   	               
    	   	       	   ucFlag = 1;
    	   	       	   break;
    	   	       }
    	    	  }
    	    }
 	    }
	}
  if(ucFlag == 0)
	{
		    //printf("key  is not exist\n ");
  	    close(fd);
		    return (-1);
	}
	exit:
	   printf("");
	
	close(fd);
	return 0;
}



int insert(unsigned short *a,unsigned short high,unsigned short key)
{
	  int FirstPos,MidPos,LastPos;
	  
	  FirstPos = 0;
	  LastPos = high;
	  if(key<a[FirstPos])
    {
    	 return FirstPos;
    }
    if(key == a[FirstPos]) 
    {
    	  return -1;
    }
    if(key>a[LastPos]) 
    {
    	  return  LastPos+1;
    }
    if(key == a[LastPos])
    {
    	  return -1;
    }
    while(FirstPos<=LastPos)
    {
    	  MidPos = (FirstPos+LastPos)/2;
    	  if(key>a[MidPos])
    	  {
    	  	 FirstPos = MidPos+1;
    	  }else if(key<a[MidPos])
    	  {
    	  	 LastPos = MidPos-1; 
    	  }else
    	  {
    	  	 return -1;
    	  }
    }
    return (LastPos+1);
}


int key_search(unsigned short *a,unsigned short key, short low, short high)
{
	int mid;
	
	if(low > high)
	{
    return -1;
  }
  mid = (low+high)/2;
  if(a[mid] == key)
  {
  	  return mid;
  }else if(a[mid] >key)
  {
  	  return key_search(a,key,low,mid-1);
  }else
  {
  	  return key_search(a,key,mid+1,high);
  }	

}
	


static int MMI_LoadSingleKeyUseDoubleKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
        int  retval=0;
	unsigned char array,src_idx,dest_idx;
	unsigned char key[KEY_MAX_KEYDATA*2],decrypt_data[KEY_MAX_KEYDATA],decrypted_data[KEY_MAX_KEYDATA];
	unsigned char i;
	
  memset(key,0,sizeof(key));
	memset(decrypt_data,0,sizeof(decrypt_data));
	memset(decrypted_data,0,sizeof(decrypted_data));
	
#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif
	
	/* ascii to hex*/
	array    = pdrvin->aucData[KEY_PACKAGE_START+0]-0x30;
	src_idx  = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;
	dest_idx = pdrvin->aucData[KEY_PACKAGE_START+2]-0x30; 
	asc_hex(decrypt_data,KEY_MAX_KEYDATA,&(pdrvin->aucData[KEY_PACKAGE_START+3]),KEY_MAX_KEYDATA*2);
#if MMI_DEB
	printf("\narray=%d,src_idx=%d,dest_idx=%d,decrypt_data=%s",array,src_idx,dest_idx,&(pdrvin->aucData[KEY_PACKAGE_START+3]));
#endif	
    /* load key from key file */
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,src_idx,key,KEY_DOUBLE_YES) )
	{
		  retval = -1;
		  goto exit;
	}
#if MMI_DEB	
	printf("\nMMI_cipher_get_key\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	if( 0 != MMI_keycrypt_imx(key,SCC_DECRYPT,KEY_MAX_KEYDATA*2))
	{
		  retval = -1;
      goto exit;
	}         //decrypt key 
#if MMI_DEB
	printf("\nMMI_keycrypt_imx\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif	
	/* 3des decypt */
	desm1(decrypt_data,decrypted_data,key);
	des(decrypted_data,decrypt_data,&key[KEY_MAX_KEYDATA]);
	desm1(decrypt_data,decrypted_data,key);
#if MMI_DEB
	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif
	memset(key,0,sizeof(key));	
	if( 0 != MMI_keycrypt_imx(decrypted_data,SCC_ENCRYPT,KEY_MAX_KEYDATA))
	{
		   retval = -1;
       goto exit;
	}         //encrypt for save
#if MMI_DEB
	printf("\nMMI_keycrypt_imx\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
#endif
	/* put the decrypt data into dest index */
	if ( 0 != MMI_cipher_put_key(ucOSCurrAppID,array,dest_idx,decrypted_data,KEY_DOUBLE_NO) )
	{
		retval = -1;
		goto exit;
	}
    /* put result into out buffer */
	retval = 0;
exit:	
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = -retval;  
	pdrvout->aucData[0] = 0x30;
	if(retval == -1)
		pdrvout->aucData[0] = 0x31;
	return retval; 

}

static int MMI_LoadDoubleUseDoubleKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
        int  retval=0;
	unsigned char array,src_idx,dest_idx;
	unsigned char key[KEY_MAX_KEYDATA*2],decrypt_data[KEY_MAX_KEYDATA*2],decrypted_data[KEY_MAX_KEYDATA*2];
	unsigned char i;
      
	
  memset(key,0,sizeof(key));
	memset(decrypt_data,0,sizeof(decrypt_data));
	memset(decrypted_data,0,sizeof(decrypted_data));
	
#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif
	array    = pdrvin->aucData[KEY_PACKAGE_START+0]-0x30;
	src_idx  = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;
	dest_idx = pdrvin->aucData[KEY_PACKAGE_START+2]-0x30; 
	asc_hex(decrypt_data,KEY_MAX_KEYDATA*2,&(pdrvin->aucData[KEY_PACKAGE_START+3]),KEY_MAX_KEYDATA*4);
#if MMI_DEB
	printf("\narray=%d,src_idx=%d,dest_idx=%d,decrypt_data=%s",array,src_idx,dest_idx,&(pdrvin->aucData[KEY_PACKAGE_START+3]));
 #endif
    /* load key from key file */
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,src_idx,key,KEY_DOUBLE_YES) )
	{
		 retval = -1;
		 goto exit;
	}
#if MMI_DEB
	printf("\nMMI_cipher_get_key\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	if( 0 != MMI_keycrypt_imx(key,SCC_DECRYPT,KEY_MAX_KEYDATA*2))
	{
		 retval = -1;
     goto exit;
	}         //decrypt key 
#if MMI_DEB
	printf("\nMMI_keycrypt_imx\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",key[i]);
	}
#endif
	/* 3des decypt */
	desm1(decrypt_data,decrypted_data,key);
	des(decrypted_data,decrypt_data,key+KEY_MAX_KEYDATA);
	desm1(decrypt_data,decrypted_data,key);

	desm1(decrypt_data+KEY_MAX_KEYDATA,decrypted_data+KEY_MAX_KEYDATA,key);
	des(decrypted_data+KEY_MAX_KEYDATA,decrypt_data+KEY_MAX_KEYDATA,key+KEY_MAX_KEYDATA);
	desm1(decrypt_data+KEY_MAX_KEYDATA,decrypted_data+KEY_MAX_KEYDATA,key);
#if MMI_DEB	
	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif	
	memset(key,0,sizeof(key));	
	if( 0 != MMI_keycrypt_imx(decrypted_data,SCC_ENCRYPT,KEY_MAX_KEYDATA*2))
	{
		    retval = -1;
        goto exit;
	}         //encrypt for save
#if MMI_DEB	
	printf("\n*************************************\n");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++)
	{
		 printf(" %02x",decrypted_data[i]);
	}
	printf("\n*************************************\n");
#endif
	/* put the decrypt data into dest index */
	if ( 0 != MMI_cipher_put_key(ucOSCurrAppID,array,dest_idx,decrypted_data,KEY_DOUBLE_YES) )
	{
		retval = -1;
		goto exit;
	}
    /* put result into out buffer */
	retval = 0;
exit:	
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = -retval;  
	pdrvout->aucData[0] = 0x30;
	if(retval == -1)
		pdrvout->aucData[0] = 0x31;
	return retval; 

}

static int MMI_EncryptHASH(DRVIN *pdrvin,DRVOUT *pdrvout)
{
	  int ret,i;
	  unsigned char aucOutdata[HASH_SHA256_MAXLEN];
	  unsigned char ucHashType;
	
	  ucHashType = pdrvin->aucData[0];
	  if((ucHashType == HASH_SHA1)||(ucHashType == HASH_SHA256))
	  {
	  	 memset(aucOutdata,0,sizeof(aucOutdata));
	     ret = MMI_keycrypt_Hash(&pdrvin->aucData[1],pdrvin->ucLen-1,pdrvin->aucData[0],aucOutdata);  
	  }else
	  {
	  	 printf("\n hashtype is error!");
	  	 ret = -1;
	  }
 #if MMI_DEB
	printf("\n hash data \n");
	for(i=0;i<HASH_SHA256_MAXLEN;i++){
		printf("%02x ",aucOutdata[i]);
	}
	printf("\n\r");
#endif
	  pdrvout->ucGenStatus = DRVST_ENDED;
	  pdrvout->ucDrvStatus = -ret;  
	  pdrvout->aucData[0] = 0x30;
	  if(ret == -1)
	    pdrvout->aucData[0] = 0x31;
    if(ucHashType == HASH_SHA1)
    {
    	   pdrvout->aucData[1] = HASH_SHA1_MAXLEN; 
    	   memcpy(&(pdrvout->aucData[2]),aucOutdata,HASH_SHA1_MAXLEN);
    }else
    {
    	   pdrvout->aucData[1] = HASH_SHA256_MAXLEN; 
    	   memcpy(&(pdrvout->aucData[2]),aucOutdata,HASH_SHA256_MAXLEN);
    }
	 
	  return ret; 

}

static int MMI_Encrypt8ByteSingleKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
  int  retval=0;
	unsigned char array,index;
	unsigned char key[KEY_MAX_KEYDATA*2],encrypt_data[KEY_MAX_KEYDATA],encrypted_data[KEY_MAX_KEYDATA];
	unsigned char inkey[KEY_MAX_KEYDATA];
	int i;

#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif

    /* ascii to hex*/
	array = pdrvin->aucData[KEY_PACKAGE_START+0]-0x30;
	index = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;

	asc_hex(encrypt_data,KEY_MAX_KEYDATA,&(pdrvin->aucData[KEY_PACKAGE_START+2]),KEY_MAX_KEYDATA*2);
    /* load key from key file */
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,index,key,KEY_DOUBLE_NO) )
	{
		pdrvout->ucGenStatus = DRVST_ENDED;
		pdrvout->ucDrvStatus = 1;//NOT OK
		pdrvout->aucData[0] = 0x31;
		return (-1);
	}
#if MMI_DEB
	printf("\nMMI_cipher_get_key  ");
	for(i=0;i<KEY_MAX_KEYDATA;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif
	if( 0 != MMI_keycrypt_imx(key, SCC_DECRYPT, KEY_MAX_KEYDATA))
	{
	        pdrvout->ucGenStatus = DRVST_ENDED;
		      pdrvout->ucDrvStatus = 1;//NOT OK
		      pdrvout->aucData[0] = 0x31;
          return (-1);
		}
	
#if MMI_DEB
	printf("\nMMI_keycrypt_imx ");
	for(i=0;i<KEY_MAX_KEYDATA;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");

	printf("encrypt_data = ");
	for(i=0;i<KEY_MAX_KEYDATA;i++){
		printf("%02x ",encrypt_data[i]);
	}
	printf("\n\r");
#endif
	
	/* des encypt */
	des(encrypt_data,encrypted_data,key);
  memset(encrypt_data,0,sizeof(encrypt_data));
	memset(key,0,sizeof(key));

#if MMI_DEB
	printf("\ndes = ");
	for(i=0;i<KEY_MAX_KEYDATA;i++){
		printf("%02x ",encrypted_data[i]);
	}
	printf("\n\r");
#endif

    /* put into out buffer */
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = 0;//OK
	pdrvout->aucData[0] = 0x30;
	hex_asc(&(pdrvout->aucData[1]),encrypted_data,KEY_MAX_KEYDATA*2);
	return retval;
}


static int MMI_Encrypt3DES(DRVIN *pdrvin,DRVOUT *pdrvout)
{
  int  retval=0;
	unsigned char array,index;
	unsigned char key[KEY_MAX_KEYDATA*2],encrypt_data[KEY_MAX_KEYDATA],encrypted_data[KEY_MAX_KEYDATA];
	unsigned char inkey[KEY_MAX_KEYDATA];
	int i;

#ifdef KEY_PACKAGE_HAVEAPPID
   ucOSCurrAppID = pdrvin->aucData[0]*256+pdrvin->aucData[1];
else
	ucOSCurrAppID =get_app_id();
#endif

    /* ascii to hex*/
	array = pdrvin->aucData[KEY_PACKAGE_START+0]-0x30;
	index = pdrvin->aucData[KEY_PACKAGE_START+1]-0x30;

	asc_hex(encrypt_data,KEY_MAX_KEYDATA,&(pdrvin->aucData[KEY_PACKAGE_START+2]),KEY_MAX_KEYDATA*2);
#if MMI_DEB
	printf("\narray=%02x,index=%02x,encrypt_data=%s\n",array,index,&(pdrvin->aucData[KEY_PACKAGE_START+2]));
#endif	
    /* load key from key file */
	if ( 0 != MMI_cipher_get_key(ucOSCurrAppID,array,index,key,KEY_DOUBLE_YES) )
	{
		pdrvout->ucGenStatus = DRVST_ENDED;
		pdrvout->ucDrvStatus = 1;//NOT OK
		pdrvout->aucData[0] = 0x31;
		return (-1);
	}
#if MMI_DEB	
	printf("\nMMI_cipher_get_key");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif	
	if( 0 != MMI_keycrypt_imx(key, SCC_DECRYPT, KEY_MAX_KEYDATA*2))
	{
	        pdrvout->ucGenStatus = DRVST_ENDED;
		      pdrvout->ucDrvStatus = 1;//NOT OK
		      pdrvout->aucData[0] = 0x31;
          return (-1);
		}
	
#if MMI_DEB
	printf("\nMMI_keycrypt_imx");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");

	
#endif
	
	/* des encypt */
	des(encrypt_data,encrypted_data,key);
	desm1(encrypted_data,encrypt_data,key+KEY_MAX_KEYDATA);
	des(encrypt_data,encrypted_data,key);
  memset(encrypt_data,0,sizeof(encrypt_data));
	memset(key,0,sizeof(key));

#if MMI_DEB
	printf("\n!!!!!!!!!!!!!!!!!!!!\n");
	for(i=0;i<KEY_MAX_KEYDATA;i++){
		printf("%02x ",encrypted_data[i]);
	}
	printf("\n!!!!!!!!!!!!!!!!!!!!\n");
#endif

    /* put into out buffer */
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = 0;//OK
	pdrvout->aucData[0] = 0x30;
	hex_asc(&(pdrvout->aucData[1]),encrypted_data,KEY_MAX_KEYDATA*2);
	return retval;
}

static int MMI_LoadProtectKey(DRVIN *pdrvin,DRVOUT *pdrvout)
{
  int  retval=0;
	unsigned char array,index;
	unsigned char key[KEY_MAX_KEYDATA*2],encrypt_data[KEY_MAX_KEYDATA],encrypted_data[KEY_MAX_KEYDATA];
	unsigned char inkey[KEY_MAX_KEYDATA];
	int i;



    /* ascii to hex*/
	array = pdrvin->aucData[0];
	if((array<0x30)||(array>0x39))
	{
		      printf("\n*********index is error!!!**************\n");
	        pdrvout->ucGenStatus = DRVST_ENDED;
		      pdrvout->ucDrvStatus = 1;//NOT OK
		      pdrvout->aucData[0] = 0x31;
          return (-1);
		}
		
	memset(key,0,sizeof(key));
#if MMI_DEB
	printf("\narray=%02x,key=%s\n",array,&pdrvin->aucData[1]);
#endif	
	asc_hex(key,KEY_MAX_KEYDATA*2,&(pdrvin->aucData[1]),KEY_MAX_KEYDATA*4);
	if( 0 != MMI_keycrypt_imx(key, SCC_ENCRYPT, KEY_MAX_KEYDATA*2))
	{
	        pdrvout->ucGenStatus = DRVST_ENDED;
		      pdrvout->ucDrvStatus = 1;//NOT OK
		      pdrvout->aucData[0] = 0x31;
          return (-1);
		}
	
#if MMI_DEB
	printf("\nMMI_keycrypt_imx");
	for(i=0;i<KEY_MAX_KEYDATA*2;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");


#endif
	
	 if( 0 != MMI_prot_put_key(array,key))
	 {
	        pdrvout->ucGenStatus = DRVST_ENDED;
		      pdrvout->ucDrvStatus = 1;//NOT OK
          return (-1);
		}
	

    /* put into out buffer */
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = 0;//OK
	pdrvout->aucData[0] = 0x30;
	
	
	return retval;
}

int MMI_prot_put_key(unsigned char index,unsigned char *key)
{
	int  fd;
	int  nbytes,retval=0;
	int i,j,result;
	int length, res,infolen,listlen,keylen,protlen;
	unsigned int ucI,ucJ,ucK,ucM,ucN;
	unsigned short usmarker;
	unsigned char ucContinueFlag = 0;
	
    
#if MMI_DEB	
	printf("%s\n",__func__);        	
	printf("put key = ");
	j = KEY_MAX_KEYDATA*2;
	for(i=0;i<j;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif

	
	if ( (fd = open(KEY_INFO_FILE,O_RDWR)) < 0)
	{
		  printf("%s:%02x\n",__func__,errno);
		  return (-1);
	}else
	{
	    memset(&Keyinfodata,0,sizeof(KEYSYSTEMINFO_DATA));
      infolen = sizeof(KEYSYSTEMINFO_DATA); 
      listlen = sizeof(KEY_APPINDEXLIST)*KEY_MAX_APPNUMBER;
      keylen  = sizeof(KEYAPPKEYINFO_DATA)*KEY_MAX_APPNUMBER*KEY_MAX_KEYNUMBER; 
      
      protlen = infolen+listlen+keylen;
 	    if ((res = read(fd,&Keyinfodata,infolen)) !=  infolen)
 	    {
    	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		      retval = -1;
			    goto exit;
 	    }
 #if MMI_DEB	    
 	    printf("\nKeyinfodata.keyvalid=%02x",Keyinfodata.keyvalid);
 #endif
 	    if((Keyinfodata.keyvalid != KEY_VALID_FLAG)&&(Keyinfodata.keyvalid != KEY_INVALID_FLAG))
 	    {
    	    printf("key system is error\n ");
    	    retval = -1;
			    goto exit;
 	    }
 	    if(Keyinfodata.keyvalid == KEY_VALID_FLAG)
 	    {
 	    	  lseek(fd,protlen,SEEK_SET);	
 	    	  protlen = sizeof(KEYPROTINFO_DATA);
 	    	  for(i=0;i<KEY_MAX_PROTNUMBER;i++)
 	    	  {
 	    	  	  memset(&Keyprotdata,0,sizeof(KEYPROTINFO_DATA));
 	    	  	  if ((res = read(fd,&Keyprotdata,protlen)) !=  protlen)
 	    	  	  {
 	    	  	  	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
							      retval = -1;
								    goto exit;
 	    	  	  }
 	 #if MMI_DEB
 	    	  	  printf("\ni=%02x,Keyprotdata.keyindex=%02x",i,Keyprotdata.keyindex);
 	  #endif
 	    	  	  if((Keyprotdata.keyindex == 0)||(Keyprotdata.keyindex == index))
  	  	  	  {
  	  	  	   	    lseek(fd,0-protlen,SEEK_CUR);
  	  	  	   	    Keyprotdata.keyindex = index;
  	  	  	   	    memcpy(&Keyprotdata.keydata,key,KEY_MAX_KEYDATA*2);
  	  	  	   	    if ((res = write(fd, &Keyprotdata, protlen)) != protlen) 
   	                {
   	                	  printf("key system is error\n ");
					   	    	    retval = -1;
										    goto exit;
   	                }
  	  	  	   	    break;
  	  	  	  }
 	    	  }		
 	    	  if(i == KEY_MAX_PROTNUMBER)
 	    	  {
 	    	  	   printf("\n pro key system is full\n ");
					   	 retval = -1;
							 goto exit;
 	    	  }   	                  
 	    }
	}
  
	exit:
	   printf("");
	
	close(fd);
	return retval;
}

int MMI_prot_get_key(unsigned char index,unsigned char *key)
{
	int  fd;
	int  nbytes;
	int i,j,result,retval=0;
	int length, res,infolen,listlen,keylen,protlen;
	unsigned int ucI,ucJ,ucK,ucM,ucN;
	unsigned short usmarker;
	unsigned char ucContinueFlag = 0;
	
    
#if MMI_DEB	
	printf("%s\n",__func__);        	
	printf("put key = ");
	j = KEY_MAX_KEYDATA*2;
	for(i=0;i<j;i++)
	{
		printf("%02x ",key[i]);
	}
	printf("\n\r");
#endif

	
	if ( (fd = open(KEY_INFO_FILE,O_RDWR)) < 0)
	{
		  printf("%s:%02x\n",__func__,errno);
		  return (-1);
	}else
	{
	    memset(&Keyinfodata,0,sizeof(KEYSYSTEMINFO_DATA));
      infolen = sizeof(KEYSYSTEMINFO_DATA); 
      listlen = sizeof(KEY_APPINDEXLIST)*KEY_MAX_APPNUMBER;
      keylen  = sizeof(KEYAPPKEYINFO_DATA)*KEY_MAX_APPNUMBER*KEY_MAX_KEYNUMBER; 
      protlen = infolen+listlen+keylen;
 	    if ((res = read(fd,&Keyinfodata,infolen)) !=  infolen)
 	    {
    	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
		      retval = -1;
			    goto exit;
 	    }
 	    if((Keyinfodata.keyvalid != KEY_VALID_FLAG)&&(Keyinfodata.keyvalid != KEY_INVALID_FLAG))
 	    {
    	    printf("key system is error\n ");
    	    retval = -1;
			    goto exit;
 	    }
#if MMI_DEB
 	    printf("\nindex=%02x,Keyinfodata.keyvalid=%02x",index,Keyinfodata.keyvalid);
#endif 	    
 	    if(Keyinfodata.keyvalid == KEY_VALID_FLAG)
 	    {
 	    	  lseek(fd,protlen,SEEK_SET);	
 	    	  protlen = sizeof(KEYPROTINFO_DATA);
 	    	  for(i=0;i<KEY_MAX_PROTNUMBER;i++)
 	    	  {
 	#if MMI_DEB    	  	
 	    	  	  printf("\ni=%02x",i);
 	 #endif
 	    	  	  memset(&Keyprotdata,0,sizeof(KEYPROTINFO_DATA));
 	    	  	  if ((res = read(fd,&Keyprotdata,protlen)) !=  protlen)
 	    	  	  {
 	    	  	  	    fprintf(stderr,"[%s:%d] %02x\n",__func__,__LINE__,errno);
							      retval = -1;
								    goto exit;
 	    	  	  }
 #if MMI_DEB	    	 
 	    	  	  printf("Keyprotdata.keyindex=%02x",Keyprotdata.keyindex);
 #endif
 	    	  	  if(Keyprotdata.keyindex == index)
  	  	  	  {	  	  	   	    
  	  	  	   	    memcpy(key,&Keyprotdata.keydata,KEY_MAX_KEYDATA*2);  	  	  	   	    
  	  	  	   	    break;
  	  	  	  }
 	    	  }		
 	    	  if(i == KEY_MAX_PROTNUMBER)
 	    	  {
 	    	  	   printf("key system is full\n ");
					   	 retval = -1;
							 goto exit;
 	    	  }   	                  
 	    }
	}
	exit:
	   printf("");
	
	close(fd);
	return retval;
}


static int MMI_LoadTransKeyUseProtectKey(DRVIN  *pdrvin,DRVOUT *pdrvout)
{
  int  retval=0;
	unsigned char array,index;
	unsigned char key[KEY_MAX_KEYDATA*2],encrypt_data[KEY_MAX_KEYDATA],encrypted_data[KEY_MAX_KEYDATA];
	unsigned char inkey[KEY_MAX_KEYDATA];
	unsigned int  uiTransKeyAppId,uiDeTransKeyAppId;
	int i;
	unsigned char ucResult;
	unsigned char aucInput[KEY_MAX_KEYDATA*2+1],aucOutput[KEY_MAX_KEYDATA*2+1];



  uiTransKeyAppId   = pdrvin->aucData[1]*256+pdrvin->aucData[2];
	uiDeTransKeyAppId = pdrvin->aucData[3]*256+pdrvin->aucData[4];
	memset(key,0,sizeof(key));
	asc_hex(key,KEY_MAX_KEYDATA*2,&(pdrvin->aucData[11]),KEY_MAX_KEYDATA*4);
	
#if MMI_DEB
	printf("\nuiTransKeyAppId=%d,uiDeTransKeyAppId=%d,key =\n",uiTransKeyAppId,uiDeTransKeyAppId);
	for(i=0;i<KEY_MAX_KEYDATA*2;i++){
		printf("%02x ",key[i]);
	}
	printf("\n\r");
	printf("\npdrvin->aucData[0]=%02x",pdrvin->aucData[0]);
#endif	

	
	switch(pdrvin->aucData[0])
	{
		  case 0x30:
		  	  ucResult = KEY_LoadTransKeyUseProtectKey_old( 
					uiTransKeyAppId,			// 保存传输密钥的应用号
					pdrvin->aucData[7],		// 解密方式
					pdrvin->aucData[8],		// 保护密钥索引
					pdrvin->aucData[9]-0x30,// 存传输密钥行号

					pdrvin->aucData[10]-0x30,// 存传输密钥列号
					key );
		  	   break;
		  case 0x31:

		  	  ucResult = KEY_LoadTransKeyUseAppKey_old( 
					uiTransKeyAppId,			// 保存传输密钥的应用号
					uiDeTransKeyAppId,			// 解密传输密钥的应用号
					pdrvin->aucData[5]-0x30,// 解密密钥行号
					pdrvin->aucData[6]-0x30,// 解密密钥列号
					pdrvin->aucData[7],		// 解密方式
					pdrvin->aucData[9]-0x30,// 存传输密钥行号
					pdrvin->aucData[10]-0x30,// 存传输密钥列号
					key );
		  	   break;
		  case 0x40:
		  	   ucResult =  Key_EncryptDataUseProtectKey_old( 
					pdrvin->aucData[8],		// 保护密钥索引
					pdrvin->aucData[7],		// 解密方式
					key,aucOutput );		// 输入输出数据
		  	   break;
		  case 0x41:
		  	   switch( pdrvin->aucData[7] )
				{
					case 0x30:
						 ucResult =  DYMN_Encrypt8ByteSingleKey_old( 
							uiDeTransKeyAppId,			// 指定解密的应用号
							pdrvin->aucData[5]-0x30,// 加密密钥行号	
							pdrvin->aucData[6]-0x30,// 加密密钥列号	
							key,aucOutput );		// 输入输出数据
						break;
					case 0x31:
						 ucResult = DYMN_TEncryptKey_old(
							0,							// 加密方式
							uiDeTransKeyAppId,			// 指定解密的应用号
							pdrvin->aucData[5]-0x30,// 加密密钥行号	
							pdrvin->aucData[6]-0x30,// 加密密钥列号	
							key,aucOutput);		// 输入输出数据
						break;
					case 0x32:
						 ucResult = DYMN_TEncryptKey_old(
							1,							// 加密方式
							uiDeTransKeyAppId,			// 指定解密的应用号
							pdrvin->aucData[5]-0x30,// 加密密钥行号	
							pdrvin->aucData[6]-0x30,// 加密密钥列号	
							key,aucOutput);		// 输入输出数据
						break;
					default:
						 ucResult =  0x31;
						break;
				}
		  	   break;
		  default:
		  	   pdrvout->ucGenStatus = DRVST_ENDED;
		       pdrvout->ucDrvStatus = 1;//NOT OK
		       pdrvout->aucData[0] = 0x31;
           return (-1);
		  	   break;
	}
	  if(ucResult)
	  {
	        pdrvout->ucGenStatus = DRVST_ENDED;
		      pdrvout->ucDrvStatus = 1;//NOT OK
		      pdrvout->aucData[0] = 0x31;
          return (-1);
		}else
		{
		         pdrvout->aucData[0] = 0x30;
	
			  hex_asc(&(pdrvout->aucData[1]),aucOutput,KEY_MAX_KEYDATA*4);
		}
	  
	
	
	

    /* put into out buffer */
	pdrvout->ucGenStatus = DRVST_ENDED;
	pdrvout->ucDrvStatus = 0;//OK
	
	
	return retval;
}


unsigned char KEY_LoadTransKeyUseProtectKey_old(
	unsigned int uiAppIndex,			// 保存传输密钥的应用号
	unsigned char ucDecryptType,        // 解密方式
	unsigned char ucDecryptKeyIndex,    // 保护密钥索引
	unsigned char ucDestKeyArray,       // 存传输密钥行号
	unsigned char ucDestKeyIndex,       // 存传输密钥列号
	unsigned char *pucInData )
	
{
  unsigned char ucI,ucResult,ucLoadType,ucKeyType;
	unsigned char aucInData[KEY_MAX_KEYDATA*2],aucKey[KEY_MAX_KEYDATA*2],aucOutKey[KEY_MAX_KEYDATA*2];
	
#if MMI_DEB	
	printf("\nuiAppIndex=%d,ucDecryptType=%02x,ucDecryptKeyIndex=%02x,ucDestKeyArray=%02x,ucDestKeyIndex=%02x\n",uiAppIndex,ucDecryptType,ucDecryptKeyIndex,ucDestKeyArray,ucDestKeyIndex);
#endif
	
	memset( aucInData,0,sizeof(aucInData) );
	if( (ucDecryptType<0x30)||(ucDecryptType>0x32) )
	{
		 printf("\nucDecryptType is error");
		 return( 1 );
	}
	
	if( (ucDecryptKeyIndex<0x30)||(ucDecryptKeyIndex>0x39) )
	{
		 printf("\nucDecryptKeyIndex is error");
		 return( 1 );
	}
		
	  
	  if( 0 != MMI_prot_get_key(ucDecryptKeyIndex,aucInData))
	  {
	       
          return (1);
		}
#if MMI_DEB		
		printf("\nMMI_prot_get_key\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucInData[ucI]);
		}
#endif		
		if(0 != MMI_keycrypt_imx(aucInData,SCC_DECRYPT,KEY_MAX_KEYDATA*2)){
	      
         return (1);
		} 
#if MMI_DEB
		printf("\nMMI_keycrypt_imx\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucInData[ucI]);
		} 
		
		printf("\npucInData\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",pucInData[ucI]);
		} 
#endif
     memcpy(aucKey,aucInData,KEY_MAX_KEYDATA*2);
	
	if( ucDecryptType==0x32 )
	{
		memcpy( aucInData,pucInData,KEY_MAX_KEYDATA*2 );
		ucLoadType = 0x32;
	}else
	{
		memcpy( aucInData,pucInData,KEY_MAX_KEYDATA );
		ucLoadType = 0x31;
	}
	if( ucDecryptType==0x32 )
		ucKeyType = KEYTYPE_DOUBLE;
	else
		ucKeyType = KEYTYPE_SINGLE;
	memset( aucOutKey,0,sizeof(aucOutKey) );
	
	desm1( aucInData,aucOutKey,aucKey );
	if( ucDecryptType!=0x30 )
	{
		des( aucOutKey,aucInData,aucKey+KEY_MAX_KEYDATA );
		desm1( aucInData,aucOutKey,aucKey );
	}
	if( ucDecryptType==0x32 )
	{
		desm1( aucInData+KEY_MAX_KEYDATA,aucInData,aucKey );
		des( aucInData,aucInData+KEY_MAX_KEYDATA,aucKey+KEY_MAX_KEYDATA );
		desm1( aucInData+KEY_MAX_KEYDATA,aucOutKey+KEY_MAX_KEYDATA,aucKey );
	}
#if MMI_DEB	
	  printf("\naucOutKey\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucOutKey[ucI]);
		} 
#endif	
	if(ucKeyType == KEYTYPE_DOUBLE)
	{
		  if( 0 != MMI_keycrypt_imx(aucOutKey,SCC_ENCRYPT,KEY_MAX_KEYDATA*2))
			{
				return 1;
			}         //encrypt for save
#if MMI_DEB
			printf("\nMMI_keycrypt_imx  KEYTYPE_DOUBLE\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucOutKey[ucI]);
		} 
#endif
			/* put the decrypt data into dest index */
			if ( 0 != MMI_cipher_put_key(uiAppIndex,ucDestKeyArray,ucDestKeyIndex,aucOutKey,KEY_DOUBLE_YES) )
			{
				return 1;
			}
		   
	}else
	{
		  if( 0 != MMI_keycrypt_imx(aucOutKey,SCC_ENCRYPT,KEY_MAX_KEYDATA))
			{
				return 1;
			}         //encrypt for save
	#if MMI_DEB
			printf("\nMMI_keycrypt_imx  KEY_DOUBLE_NO\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA;ucI++)
		{
			 printf(" %02x",aucOutKey[ucI]);
		} 
	#endif
			/* put the decrypt data into dest index */
			if ( 0 != MMI_cipher_put_key(uiAppIndex,ucDestKeyArray,ucDestKeyIndex,aucOutKey,KEY_DOUBLE_NO) )
			{
				return 1;
			}
	}
    
	
  

    return( 0 );
}


unsigned char Key_EncryptDataUseProtectKey_old(
	unsigned char ucProtectKeyIndex,
	unsigned char ucEncryptType,
	unsigned char *pucInData,
	unsigned char *pucOutData )
{
	unsigned char ucI,ucResult;
	unsigned char aucInData[KEY_MAX_KEYDATA*2],aucKey[KEY_MAX_KEYDATA*2];
	
#if MMI_DEB	
	   printf("\nucProtectKeyIndex=%02x,ucEncryptType=%02x,pucInData=%02x",ucProtectKeyIndex,ucEncryptType,pucInData[0]);
#endif	
	 if( (ucProtectKeyIndex<0x30)||(ucProtectKeyIndex>0x39) )
	{
		 printf("\nucDecryptKeyIndex is error");
		 return( 1 );
	}
	
	 if( 0 != MMI_prot_get_key(ucProtectKeyIndex,aucInData))
	 {
	       return 1;
		}
#if MMI_DEB
		printf("\nucEncryptType=%02x",ucEncryptType);
		printf("\nucProtectKeyIndex=%02x \n aucOutKey\n",ucProtectKeyIndex);
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucInData[ucI]);
		} 
#endif	
	 if(0 != MMI_keycrypt_imx(aucInData,SCC_DECRYPT,KEY_MAX_KEYDATA*2)){
	      
         return (1);
		} 
#if MMI_DEB		
		printf("\nMMI_keycrypt_imx\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucInData[ucI]);
		} 
#endif		 
	memcpy( aucKey,aucInData,KEY_MAX_KEYDATA*2 );
	memcpy( aucInData,pucInData,KEY_MAX_KEYDATA*2 );
	des( aucInData,pucOutData,aucKey );
	if( ucEncryptType!=0x30 )
	{
		desm1( pucOutData,aucInData,aucKey+KEY_MAX_KEYDATA );
		des( aucInData,pucOutData,aucKey );
	}
	if( ucEncryptType==0x32 )
	{
		des( aucInData+KEY_MAX_KEYDATA,aucInData,aucKey );
		desm1( aucInData,aucInData+KEY_MAX_KEYDATA,aucKey+KEY_MAX_KEYDATA );
		des( aucInData+KEY_MAX_KEYDATA,pucOutData+KEY_MAX_KEYDATA,aucKey );
	}
#if MMI_DEB	
	printf("\npucOutData\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",pucOutData[ucI]);
		} 
#endif	
	
	return( 0 );
}


unsigned char KEY_LoadTransKeyUseAppKey_old(
	unsigned int uiTransKeyAppId,		// 保存传输密钥的应用号
	unsigned int uiDecryptKeyAppId,     // 解密传输密钥的应用号
	unsigned char ucDecryptKeyCol,      // 解密密钥行号
	unsigned char ucDecryptKeyRow,      // 解密密钥列号
	unsigned char ucDecryptType,        // 解密方式
	unsigned char ucDestKeyArray,       // 存传输密钥行号
	unsigned char ucDestKeyIndex,       // 存传输密钥列号
	unsigned char *pucInData )
{
	unsigned char ucResult,ucLoadType,ucKeyType,ucI;
	unsigned char aucInData[KEY_MAX_KEYDATA*2],aucKey[KEY_MAX_KEYDATA*2],aucOutKey[KEY_MAX_KEYDATA*2];
	
 #if MMI_DEB	
	printf("\nuiTransKeyAppId=%d",uiTransKeyAppId);
	printf("\nuiDecryptKeyAppId=%d",uiDecryptKeyAppId);
	printf("\nucDecryptKeyCol=%02x",ucDecryptKeyCol);
	printf("\nucDecryptKeyRow=%02x",ucDecryptKeyRow);
	printf("\nucDecryptKeyRow=%02x",ucDecryptType);
	printf("\nucDestKeyArray=%02x",ucDestKeyArray);
	printf("\nucDestKeyIndex=%02x",ucDestKeyIndex);
#endif	
	if( ucDecryptType==0x30 )
		ucKeyType = KEY_DOUBLE_NO;
	else
		ucKeyType = KEY_DOUBLE_YES;
	memset(aucKey,0,sizeof(aucKey));
		if ( 0 != MMI_cipher_get_key(uiDecryptKeyAppId,ucDecryptKeyCol,ucDecryptKeyRow,aucKey,ucKeyType) )
	{
		 return 1;
	}
#if MMI_DEB
	printf("\nMMI_cipher_get_key\n");
#endif
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucKey[ucI]);
		}
	if( 0 != MMI_keycrypt_imx(aucKey,SCC_DECRYPT,KEY_MAX_KEYDATA*2))
	{
		   return 1;
	}         //decrypt key 
#if MMI_DEB
  	printf("\nMMI_keycrypt_imx\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucKey[ucI]);
		}
		
		printf("\npucInData\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",pucInData[ucI]);
		}
#endif	
    memcpy( aucInData,pucInData,KEY_MAX_KEYDATA*2 );
	memset( aucOutKey,0,sizeof(aucOutKey) );
	ucKeyType = KEY_DOUBLE_NO;
	desm1( aucInData,aucOutKey,aucKey );
	if( ucDecryptType!=0x30 )
	{
		des( aucOutKey,aucInData,aucKey+KEY_MAX_KEYDATA );
		desm1( aucInData,aucOutKey,aucKey );
	}
	if( ucDecryptType==0x32 )
	{
		ucKeyType = KEY_DOUBLE_YES;
		desm1( aucInData+KEY_MAX_KEYDATA,aucInData,aucKey );
		des( aucInData,aucInData+KEY_MAX_KEYDATA,aucKey+KEY_MAX_KEYDATA );
		desm1( aucInData+KEY_MAX_KEYDATA,aucOutKey+KEY_MAX_KEYDATA,aucKey );
	}
 #if MMI_DEB	
	  printf("\naucOutKey\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucOutKey[ucI]);
		} 
	#endif
	if(ucKeyType == KEY_DOUBLE_YES)
	{
		  if( 0 != MMI_keycrypt_imx(aucOutKey,SCC_ENCRYPT,KEY_MAX_KEYDATA*2))
			{
				return 1;
			}         //encrypt for save
 #if MMI_DEB
			 printf("\nMMI_keycrypt_imx  KEYTYPE_DOUBLE\n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucOutKey[ucI]);
		} 
#endif
			/* put the decrypt data into dest index */
			if ( 0 != MMI_cipher_put_key(uiTransKeyAppId,ucDestKeyArray,ucDestKeyIndex,aucOutKey,KEY_DOUBLE_YES) )
			{
				return 1;
			}
		   
	}else
	{
		  if( 0 != MMI_keycrypt_imx(aucOutKey,SCC_ENCRYPT,KEY_MAX_KEYDATA))
			{
				return 1;
			}         //encrypt for save
 #if MMI_DEB
			 printf("\nMMI_keycrypt_imx  \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA;ucI++)
		{
			 printf(" %02x",aucOutKey[ucI]);
		} 
#endif
			/* put the decrypt data into dest index */
			if ( 0 != MMI_cipher_put_key(uiTransKeyAppId,ucDestKeyArray,ucDestKeyIndex,aucOutKey,KEY_DOUBLE_NO) )
			{
				return 1;
			}
	}

    return( 0 );
}

unsigned char DYMN_Encrypt8ByteSingleKey_old(
	unsigned int uiAppIndex,
	unsigned char ucKeyArray,
	unsigned char ucKeyIndex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
    unsigned char ucResult,ucI;
    unsigned char aucKey[KEY_MAX_KEYDATA];
 #if MMI_DEB   
    printf("\n uiAppIndex=%d,ucKeyArray=%02x,ucKeyIndex=%02x,pucInData[0]=%02x",uiAppIndex,ucKeyArray,ucKeyIndex,pucInData[0]);
 #endif   
    if ( 0 != MMI_cipher_get_key(uiAppIndex,ucKeyArray,ucKeyIndex,aucKey,KEY_DOUBLE_NO) )
		{
			 return 1;
		}
	#if MMI_DEB
		printf("\n MMI_cipher_get_key \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA;ucI++)
		{
			 printf(" %02x",aucKey[ucI]);
		}
#endif
		if( 0 != MMI_keycrypt_imx(aucKey,SCC_DECRYPT,KEY_MAX_KEYDATA))
		{
			   return 1;
		}        
 #if MMI_DEB
    printf("\n MMI_keycrypt_imx \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA;ucI++)
		{
			 printf(" %02x",aucKey[ucI]);
		}
 
#endif
    des( pucInData,pucOutData,aucKey );
 #if MMI_DEB
    printf("\n pucOutData \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA;ucI++)
		{
			 printf(" %02x",pucOutData[ucI]);
		}
#endif
    return(0);
}



unsigned char DYMN_TEncryptKey_old(
	unsigned char ucEncryptType,
	unsigned int uiAppIndex,
	unsigned char ucKeyArray,
	unsigned char ucKeyIndex,
	unsigned char *pucInData,
	unsigned char *pucOutData)
{
	  unsigned char ucResult,ucI;
    unsigned char aucKey[KEY_MAX_KEYDATA*2],aucData[KEY_MAX_KEYDATA];
  #if MMI_DEB  
    printf("\nucEncryptType=%02x",ucEncryptType);
    printf("\nuiAppIndex=%d",uiAppIndex);
    printf("\nucKeyArray=%02x",ucKeyArray);
    printf("\nucKeyIndex=%02x",ucKeyIndex);
    printf("\npucInData[0]=%02x",pucInData[0]);
  #endif  
     if ( 0 != MMI_cipher_get_key(uiAppIndex,ucKeyArray,ucKeyIndex,aucKey,KEY_DOUBLE_YES) )
		{
			 return 1;
		}
	#if MMI_DEB
		printf("\n MMI_cipher_get_key \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucKey[ucI]);
		}
	#endif
		if( 0 != MMI_keycrypt_imx(aucKey,SCC_DECRYPT,KEY_MAX_KEYDATA*2))
		{
			   return 1;
		}         //decrypt key 
 #if MMI_DEB
    printf("\n MMI_keycrypt_imx \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",aucKey[ucI]);
		}

#endif
    des( pucInData,pucOutData,aucKey );
    desm1( pucOutData,aucData,aucKey+KEY_MAX_KEYDATA );
    des( aucData,pucOutData,aucKey );
    if( ucEncryptType )
    {
    	des( pucInData+KEY_MAX_KEYDATA,pucOutData+KEY_MAX_KEYDATA,aucKey );
	    desm1( pucOutData+KEY_MAX_KEYDATA,aucData,aucKey+KEY_MAX_KEYDATA );
	    des( aucData,pucOutData+KEY_MAX_KEYDATA,aucKey );
    }
 #if MMI_DEB   
    printf("\n pucOutData \n");
		for(ucI=0;ucI<KEY_MAX_KEYDATA*2;ucI++)
		{
			 printf(" %02x",pucOutData[ucI]);
		}
#endif
	return( 0 );
}



int MMI_keycrypt_Hash(unsigned char *aucIndata,unsigned int uiLen,unsigned char ucType,unsigned char *aucOutdata)
{
   int fd;
   int ret,i;
   
  
   memset(&Keyhashdata,0,sizeof( RTIC_HASH_DATA));
  
   Keyhashdata.hashlen  = uiLen;
   Keyhashdata.hashtype = ucType;
   memcpy(Keyhashdata.data,aucIndata,uiLen);
   
   fd = open(KEY_HASH_FILE, O_RDWR);
 #if MMI_DEB
   printf("\nucType=%02x,fd=%d,uiLen=%d\n",ucType,fd,uiLen);
   for(i=0;i<uiLen;i++)
   {
   	   printf("%02x ",i,aucIndata[i]);
   }
   if(fd < 0)
   {
   	  printf("can not open imx_rtic!\n");  
	    return -1;
  }
#endif
	ret = ioctl(fd,IOCTL_HASH_COMPUTE,&Keyhashdata);
	if(ret == 0){
 #if MMI_DEB
	    for(i=0;i<HASH_OUTDATA_MAXLEN;i++)
	      printf("\nhashdata[%d]=%08x ",i,Keyhashdata.output[i]);
	#endif
	    memcpy(aucOutdata,(unsigned char*)Keyhashdata.output,HASH_SHA256_MAXLEN);
	}else
	{
	    printf("keycrpyt error = %d\n",ret);
	    ret = -1;
	}

	
	return ret;	
}



////////////////////////     END  FILE ///////////////////////////////////
