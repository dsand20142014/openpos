#ifndef __LUXAPP_FILEOPS__
#define FILEOPS_EXTERN extern
#else
#define FILEOPS_EXTERN
#endif

#include "sandos_def.h"

#define INI_LINE_LENGTH 64	/* 一行最多字符数*/
#define INI_MACHINE_ITEM_LINE  21+1  //???????óò?DD,INI_MACHINE_ITEM_LINE?μ?óò?
#define INI_TAXRATE_ITEM_LINE	6+1

#define OSFS_FLASHPREFIX	"/mnt/pos/data"		/* 路径的前缀*/
#define OSFS_FLASHPREFIXLEN 8+5

/*文件操作过程返回码*/
#define FILESYSERR_SUCCESS      0x00
#define FILESYSERR_GENERAL      0x01
#define FILESYSERR_INVALIDPARAM 0x02
#define FILESYSERR_CORRUPT      0x03
#define FILESYSERR_NOFS         0x04
#define FILESYSERR_NOSPACE      0x05
#define FILESYSERR_NOFAT        0x06
#define FILESYSERR_FATBROKEN    0x07
#define FILESYSERR_FATCRC       0x08
#define FILESYSERR_READ         0x09
#define FILESYSERR_WRITE        0x0A
#define FILESYSERR_INVALIDID    0x0B
#define FILESYSERR_FILENOTOPEN  0x0C
#define FILESYSERR_BADOFFSET    0x0D
#define FILESYSERR_NOPRIVILEGE  0x0E
#define FILESYSERR_FILENAME     0x0F
#define FILESYSERR_WRITE_2      0x10

#define FILEFS_FILENAMELEN      255
FILEOPS_EXTERN int  set_app_id(unsigned short appid);
FILEOPS_EXTERN int OSAPP_OpenFile(char *pcFileName,unsigned char ucMode);
FILEOPS_EXTERN int OSAPP_OpenFileEx(char *pcFileName,unsigned char ucMode,unsigned char *pucAttr);
FILEOPS_EXTERN int OSAPP_FileRead(int iHandle,unsigned char *pucOutData,unsigned int uiOutLen);
FILEOPS_EXTERN int OSAPP_FileWrite(int iHandle,unsigned char *pucInData,unsigned int uiInLen);
FILEOPS_EXTERN int OSAPP_FileClose(int iHandle);
FILEOPS_EXTERN int OSAPP_FileSeek(int iHandle,long lOffset,unsigned char ucLocation);

FILEOPS_EXTERN unsigned long OSAPP_GetUnusedFlashSize(void);
FILEOPS_EXTERN long int OSAPP_FileSize(char *pcFileName);

FILEOPS_EXTERN unsigned long OSAPP_GetUnusedFlashSize(void);

FILEOPS_EXTERN long int  OSAPP_GetFlashSize(void);

FILEOPS_EXTERN char OSAPP_SetFlashSize(long int size);
FILEOPS_EXTERN long int  OSAPP_GetFlashSize(void);
FILEOPS_EXTERN int OSAPP_FileDelete(char *pcFileName);
FILEOPS_EXTERN int OSAPP_FileTrunc(int iHandle,unsigned int uiLen);
FILEOPS_EXTERN unsigned char OSAPP_FileGetLastError(void);

FILEOPS_EXTERN unsigned char Os__saved_swap (unsigned char PageNb);
FILEOPS_EXTERN void Os__saved_copy(unsigned char * ptSource, unsigned char * ptDest, unsigned short Length);
FILEOPS_EXTERN void Os__saved_set(unsigned char * ptDest, unsigned char Charac, unsigned short Length);
FILEOPS_EXTERN void OSMEM_Memset(unsigned char *ptDest, unsigned char Charac,  unsigned short Length);

FILEOPS_EXTERN void fd2pathname(int fd,char *pathname);


int get_app_id(void);

