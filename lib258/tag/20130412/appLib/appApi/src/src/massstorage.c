/****************************
create by liuyifeng 2011/12/26 17:58:38
modified by lichao ,add tf card operate 2013/1/11 14:22
usb/tf operate	
*****************************/
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>	
#include "massstorage.h"

#define MASSSTORAGE_USB_PATH 			"/dev/sda1"
#define MASSSTORAGE_TF_PATH				"/dev/mmcblk0p1"
#define MASSSTORAGE_MOUNTPATH 			"/mnt/mt"

FILE *MASSSTORAGE_HANDLE;
char _pPath[100];

unsigned int Os__massstorage_waitInsert(const char device_type, unsigned int timeout)
{
	int iret=-1;
	unsigned char ucBuf[50];
	
	MASSSTORAGE_HANDLE=NULL;
	sleep(timeout);
	
	if(device_type == TYPE_TF)
		iret=access(MASSSTORAGE_TF_PATH,F_OK);
	else if(device_type == TYPE_USB)
		iret=access(MASSSTORAGE_USB_PATH,F_OK);

	if(iret!= 0)
	  return MASSSTORAGE_FAIL;

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",MASSSTORAGE_MOUNTPATH);
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	if(device_type == TYPE_TF)
		sprintf(ucBuf,"mount %s %s >/dev/null 2>&1",MASSSTORAGE_TF_PATH,MASSSTORAGE_MOUNTPATH);
	else if(device_type == TYPE_USB)
		sprintf(ucBuf,"mount %s %s >/dev/null 2>&1",MASSSTORAGE_USB_PATH,MASSSTORAGE_MOUNTPATH);
	
	system(ucBuf); 
	
	return MASSSTORAGE_OK;
}

//读取文件，文件不存在时MASSSTORAGE_HANDLE为NULL
static void _massstorage_loadFile(const char *pPath)
{
  unsigned char ucTmp[100];
  int fp=0;
  
  memset(ucTmp,0,sizeof(ucTmp));
  if(pPath!=NULL)
  	sprintf(ucTmp,"%s%s",MASSSTORAGE_MOUNTPATH,pPath);
  memset(_pPath,0,sizeof(_pPath));
  sprintf(_pPath,"%s",pPath); 
  MASSSTORAGE_HANDLE = fopen (ucTmp, "r+");	
}

//创建文件夹
static void _massstorage_mkdir(const char *pPath)
{
	char buf[100],ucTmp[200];
  	char *e;
	
	e=strrchr(pPath,'/');
 	memset(buf,0,sizeof(buf));
 	memcpy(buf,pPath,e-pPath);
 	memset(ucTmp,0,sizeof(ucTmp));
	
	sprintf(ucTmp,"mkdir -p %s/%s >/dev/null 2>&1",MASSSTORAGE_MOUNTPATH,buf);
	system(ucTmp);
 	memset(ucTmp,0,sizeof(ucTmp));
 	
	sprintf(ucTmp,"echo '' >%s/%s/%s",MASSSTORAGE_MOUNTPATH,buf,e+1); 
  	system(ucTmp);
}

unsigned int Os__massstorage_selectFile(const char *pPath)
{
	unsigned char ucTmp[100];
 
	if(pPath==NULL)
		return MASSSTORAGE_NAMEERR;
	if(strlen(pPath)>MASSSTORAGE_MAXLENGTH)
		return MASSSTORAGE_NAMETOOLONG;
	if(pPath[0]!='/')
		return MASSSTORAGE_NAMEERR;

	_massstorage_loadFile(pPath);
	if(MASSSTORAGE_HANDLE==NULL)
	{     
		_massstorage_mkdir(pPath);
		_massstorage_loadFile(pPath);
	}
	return MASSSTORAGE_OK;		
}

unsigned int Os__massstorage_closeFile(void)
{
	unsigned char ucBuf[100];
	int ret;

	if(MASSSTORAGE_HANDLE!=NULL)
	{
		fclose(MASSSTORAGE_HANDLE);
		MASSSTORAGE_HANDLE=NULL;
		
		memset(ucBuf,0,sizeof(ucBuf));
		sprintf(ucBuf,"umount %s",MASSSTORAGE_MOUNTPATH);
		ret=system(ucBuf);
		if(ret==-1)
			return MASSSTORAGE_UNMOUNTERR;
		else
		{
			memset(ucBuf,0,sizeof(ucBuf));
			sprintf(ucBuf,"rm -rf %s",MASSSTORAGE_MOUNTPATH);
			system(ucBuf);
		}

		memset(_pPath,0,sizeof(_pPath));

		return MASSSTORAGE_OK; 
	}
	else
		return MASSSTORAGE_OPTERR;
}

unsigned int Os__massstorage_eraseFile(void)
{
	unsigned char ucBuf[100];
	int ret;
	
	if(MASSSTORAGE_HANDLE!=NULL)
	{
		fclose(MASSSTORAGE_HANDLE);
		MASSSTORAGE_HANDLE=NULL;
  
		memset(ucBuf,0,sizeof(ucBuf));
		sprintf(ucBuf,"rm -rf %s%s >/dev/null 2>&1",MASSSTORAGE_MOUNTPATH,_pPath);
		system(ucBuf);
	
		memset(ucBuf,0,sizeof(ucBuf));
		sprintf(ucBuf,"umount %s",MASSSTORAGE_MOUNTPATH);
		ret=system(ucBuf);
		if(ret==-1)
			return MASSSTORAGE_UNMOUNTERR;
		else
		{
			memset(ucBuf,0,sizeof(ucBuf));
			sprintf(ucBuf,"rm -rf %s",MASSSTORAGE_MOUNTPATH);
			system(ucBuf);
		}

		memset(_pPath,0,sizeof(_pPath));

		return MASSSTORAGE_OK;
	}
	else
		return MASSSTORAGE_OPTERR;
}

unsigned int Os__massstorage_getSize(unsigned long *piSize)
{
	unsigned char ucTmp[100];  
	struct stat buf;    
	
	sprintf(ucTmp,"%s/%s",MASSSTORAGE_MOUNTPATH,_pPath);
	if(stat(ucTmp, &buf)<0)    
		return MASSSTORAGE_NAMEERR;  
  
	*piSize =(unsigned long)buf.st_size;
	return MASSSTORAGE_OK;  
}

unsigned int Os__massstorage_read(unsigned int uiOffset, unsigned char *pucData,unsigned long *piLen)
{
	if(MASSSTORAGE_HANDLE==NULL)
		return MASSSTORAGE_OPTERR;
	fseek(MASSSTORAGE_HANDLE,uiOffset, SEEK_SET);
	*piLen=fread(pucData,1,*piLen,MASSSTORAGE_HANDLE);
	return MASSSTORAGE_OK; 
}


unsigned int Os__massstorage_write(unsigned int uiOffset, unsigned char *pucData,unsigned int piLen)
{
	int fd;
	
	if(MASSSTORAGE_HANDLE==NULL)
		return MASSSTORAGE_OPTERR;
	fseek(MASSSTORAGE_HANDLE,uiOffset, SEEK_SET);
	fd=fwrite(pucData,1,piLen,MASSSTORAGE_HANDLE);
	if(fd<0)
		return MASSSTORAGE_OPTERR;
	else 	
	{fsync(fd);return MASSSTORAGE_OK;} 
}

unsigned int Os__massstorage_append(unsigned char *pucData,unsigned int piLen)
{
	int fd;
	if(MASSSTORAGE_HANDLE==NULL)
		return MASSSTORAGE_OPTERR;
	fseek(MASSSTORAGE_HANDLE,0, SEEK_END);
	fd=fwrite(pucData,1,piLen,MASSSTORAGE_HANDLE);
	if(fd<0)
		return MASSSTORAGE_OPTERR;	
	fsync(fd); 
		return MASSSTORAGE_OK; 
}




