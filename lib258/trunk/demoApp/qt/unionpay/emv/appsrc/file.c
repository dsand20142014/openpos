#include "file.h"
#include <include.h>
#include <interfaceparam.h>
#include <oslib.h>
 
//2009-9-6 16:24 cbf==================================================================================
unsigned char	File_CheckExistFile(unsigned char* pszFileName,unsigned short uiFileLen)
{
	int iFileSize;
	int iFileHandel;
	
	if((iFileSize  =  OSAPP_FileSize((char*)pszFileName))<0)
	{ 	       
		iFileHandel = OSAPP_OpenFile((char*)pszFileName,O_CREATE);
		if(iFileHandel>=0)
		{				
			OSAPP_FileClose(iFileHandel);			
		}
	}
	else
	{	      
		if((iFileSize!=uiFileLen)&&uiFileLen)
		{	
			  iFileHandel = OSAPP_OpenFile((char *)pszFileName,O_READ);
			  OSAPP_FileTrunc(iFileHandel,0);					
		}
	}	
	return SUCCESS;
}

unsigned char File_ReadBasicConfigFile(BASICCONFIG * pBasicConfigParam)
{
	int iFileHandel;
	int iFileErrNo;	
	
	if(OSAPP_FileSize((char *)INTERFACEBASICCONFIGDATA)>0)
	{	       
		iFileHandel = OSAPP_OpenFile((char *)INTERFACEBASICCONFIGDATA,O_READ);		 
		iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
		if(!iFileErrNo)
		{
		      iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char*)pBasicConfigParam,sizeof(BASICCONFIG));
		}	    
		OSAPP_FileClose(iFileHandel);
	}
	return  OSAPP_FileGetLastError();
}

unsigned char File_WriteBasicConfigFile(BASICCONFIG * pBasicConfigParam)
{
	int	iHandle,iFileLen;
	unsigned char iFileResult;	
	
	iHandle = OSAPP_OpenFile((char *)INTERFACEBASICCONFIGDATA,O_WRITE); 
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(unsigned char*)pBasicConfigParam,sizeof(BASICCONFIG));
	}
	OSAPP_FileClose(iHandle);
	return OSAPP_FileGetLastError();
}

unsigned char File_ReadInterfaceKeyFile(INTERFACEKEYFILE * pInterfacekeyFile)
{
	int iFileHandel;
	int iFileErrNo;	
	
	if(OSAPP_FileSize((char *)INTERFACEKEYDATA)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)INTERFACEKEYDATA,O_READ);	 
		iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
		if(!iFileErrNo)
		{
		      iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char*)pInterfacekeyFile,sizeof(INTERFACEKEYFILE));
		}	    
		OSAPP_FileClose(iFileHandel);
	}
	return  OSAPP_FileGetLastError();
}

unsigned char File_WriteInterfaceKeyFile(INTERFACEKEYFILE * pInterfacekeyFile)
{
	int	iHandle,iFileLen;
	unsigned char iFileResult;
	
	
	iHandle = OSAPP_OpenFile((char *)INTERFACEKEYDATA,O_WRITE); 
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);		
		if(!iFileResult) 
			iFileLen=OSAPP_FileWrite(iHandle,(unsigned char*)pInterfacekeyFile,sizeof(INTERFACEKEYFILE));
	}
	OSAPP_FileClose(iHandle);
	return OSAPP_FileGetLastError();
}

void OnEve_INTERFACE_Init(void)
{
	  memset(&INTERFACEINITData,0,sizeof(INTERFACEINITData));
	  INTERFACEINITData.pWriteBasicConfigFile = File_WriteBasicConfigFile;
	  INTERFACEINITData.pReadBasicConfigFile = File_ReadBasicConfigFile;
	  INTERFACEINITData.pWriteInterfaceKeyFile = File_WriteInterfaceKeyFile;
	  INTERFACEINITData.pReadInterfaceKeyFile = File_ReadInterfaceKeyFile;	  
	  
	  INTERFACE_TermInit(&INTERFACEINITData);
}
//End=================================================================================================================

UINT FILE_SearchRecordByFileName(UCHAR* pucFileName,USHORT uiRecordSize,USHORT uiRecordItemAddrOffset,
							  UCHAR *pucItemValue,USHORT uiItemValueLen,UCHAR *pRecordBuff)
{
	int iHandle,uiFileResult;
	UINT uiRecordNum=0,uiI;
	UCHAR	ucResult;
	UINT uiLow,uiHigh,uiMid;
	short	iCmpResult;

#if 0
	{
		USHORT uiJ;
		
		util_Printf("\nSearch Record Len:%d",uiItemValueLen);
		
		for(uiJ=0;uiJ<uiItemValueLen;uiJ++)
		{
			if(!(uiJ%20)) util_Printf("\n");
			util_Printf("%02X ",*(pucItemValue+uiJ));
		}
		util_Printf("\nRecord Offset Value:%d",uiRecordItemAddrOffset);
	}
#endif	
	uiI=0;
	iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ);
	if(iHandle>=0)
	{
		ucResult=FILE_ReadRecordNums(iHandle,&uiRecordNum);
		if(!ucResult&&uiRecordNum)
		{
//			util_Printf("\n File Record Nums %d",uiRecordNum);
			uiLow=1;
			uiHigh=uiRecordNum;
			while(uiLow<=uiHigh)
			{
				uiMid=(uiLow+uiHigh)/2;
				uiFileResult=FILE_ReadRecord(iHandle,uiMid,pRecordBuff,uiRecordSize);
#if 0
				{
					USHORT uiJ;
					util_Printf("\nRead Record Data:");
					for(uiJ=0;uiJ<uiRecordSize;uiJ++)
					{
						if(!(uiJ%20)) util_Printf("\n");
						util_Printf("%02X ",*(pRecordBuff+uiJ));
					}
				}
#endif
				if(uiFileResult==uiRecordSize)
				{
					iCmpResult=memcmp(pRecordBuff+uiRecordItemAddrOffset,pucItemValue,uiItemValueLen);
					if(!iCmpResult) 
						break;
					else if(iCmpResult>0) 
						uiHigh=uiMid-1;
					else uiLow=uiMid+1;
				}
				else ucResult=SALEERR_READFILE;
						
			}
			
			if(uiLow>uiHigh||ucResult) 
			{
				uiI=0;
			}
			else uiI=uiMid;
			
		}
		
		uiFileResult=OSAPP_FileClose(iHandle);
		
		
	}
	
	return uiI;
}

UINT FILE_InsertRecordByFileName(UCHAR* pucFileName,UCHAR *pucRecord,USHORT uiRecordSize)
{
	int iHandle,uiFileResult;
	UINT uiRecordNum=0;
	UCHAR ucResult;
	long	lFileLong;
	
	ucResult=SALEERR_SUCCESS;
	
	iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ|O_WRITE);
	if(iHandle>=0)
	{
		lFileLong=OSAPP_FileSize((char*)pucFileName);
		if(lFileLong==0)
		{
			uiRecordNum=0;
			ucResult=FILE_WriteRecordNums(iHandle,uiRecordNum);
		}
		if(!ucResult)
		{
			ucResult=FILE_ReadRecordNums(iHandle,&uiRecordNum);
		}
		if(!ucResult)
		{
//			if(uiRecordNum>999) uiRecordNum=0;//yj
			uiRecordNum++;
			ucResult=FILE_WriteRecord(iHandle,uiRecordNum,pucRecord,uiRecordSize);
		}
		if(!ucResult) ucResult=FILE_WriteRecordNums(iHandle,uiRecordNum);
		
		uiFileResult=OSAPP_FileClose(iHandle);
	}
	if(ucResult) uiRecordNum=0;
	
	return uiRecordNum;
}

UCHAR	FILE_ReadRecordNums(int iHandle,UINT* puiRecordNum)
{
	UINT	uiOutLen;
	UCHAR	ucResult;
	int		iFileResult;
	
	ucResult=SALEERR_SUCCESS;
	
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult)
		{
			uiOutLen=sizeof(UINT);
			iFileResult=OSAPP_FileRead(iHandle,(UCHAR*)puiRecordNum,uiOutLen);
			if(iFileResult!=uiOutLen) ucResult=SALEERR_READFILE;			
		}
		else ucResult=SALEERR_SEEKFILE;		
	}
	else ucResult=SALEERR_OPENFILE;	
	return ucResult;
}

UCHAR	FILE_WriteRecordNums(int iHandle,UINT uiRecordNum)
{
	UINT	uiOutLen;
	UCHAR	ucResult;
	int		iFileResult;
	
	ucResult=SALEERR_SUCCESS;
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		if(!iFileResult)
		{
			uiOutLen=sizeof(UINT);
			iFileResult=OSAPP_FileWrite(iHandle,(UCHAR*)&uiRecordNum,uiOutLen);
			if(iFileResult!=uiOutLen) ucResult=SALEERR_WRITEFILE;
		}
		else ucResult=SALEERR_SEEKFILE;
	}
	else ucResult=SALEERR_OPENFILE;
	return ucResult;
}

UCHAR	FILE_ReadRecord(int iHandle,UINT uiRecordNum,UCHAR* pRecordBuff,USHORT uiRecordLen)
{
	UCHAR	ucUINTLen,ucResult;
	int 	iFileResult;
	
	ucResult=SALEERR_SUCCESS;
	ucUINTLen=sizeof(UINT);
	
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,ucUINTLen+uiRecordLen*(uiRecordNum-1),SEEK_SET);
		if(!iFileResult)
		{
			iFileResult=OSAPP_FileRead(iHandle,(UCHAR*)pRecordBuff,uiRecordLen);
			if(iFileResult!=uiRecordLen) ucResult=SALEERR_READFILE;				
		}
		else ucResult=SALEERR_SEEKFILE;

	}
	else ucResult=SALEERR_OPENFILE;
	return ucResult;
}

UCHAR	FILE_ReadRecordByFileName(UCHAR* pucFileName,UINT uiRecordNum,UCHAR* pRecordBuff,USHORT uiRecordLen)
{
	int		iHandle;
	UCHAR	ucResult;
	
	ucResult=SALEERR_SUCCESS;
	
	iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ);
	if(iHandle>=0)
	{
		ucResult=FILE_ReadRecord(iHandle,uiRecordNum,pRecordBuff,uiRecordLen);
		OSAPP_FileClose(iHandle);
	}
	else ucResult=SALEERR_OPENFILE;
	return ucResult;
}

UCHAR	FILE_ReadRecordNumByFileName(UCHAR* pucFileName,UINT *puiRecordNum)
{
	int		iHandle;
	UCHAR	ucResult;
	
	ucResult=SALEERR_SUCCESS;
	
	iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ);
	if(iHandle>=0)
	{
		ucResult=FILE_ReadRecordNums(iHandle,puiRecordNum);
		OSAPP_FileClose(iHandle);
	}
	else ucResult=SALEERR_OPENFILE;
	return ucResult;
}

UCHAR	FILE_WriteRecord(int iHandle,UINT uiRecordNum,UCHAR* pRecordBuff,USHORT uiRecordLen)
{
	UCHAR	ucUINTLen,ucResult;
	int		iFileResult;
	
	ucResult=SALEERR_SUCCESS;
	ucUINTLen=sizeof(UINT);
	
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,ucUINTLen+uiRecordLen*(uiRecordNum-1),SEEK_SET);
		if(!iFileResult)
		{
			iFileResult=OSAPP_FileWrite(iHandle,(UCHAR*)pRecordBuff,uiRecordLen);
			if(iFileResult!=uiRecordLen) ucResult=SALEERR_WRITEFILE;
				
		}
		else ucResult=SALEERR_SEEKFILE;
	}
	else ucResult=SALEERR_OPENFILE;
		
	return ucResult;
}

UCHAR FILE_DeleteRecordByFileName(UCHAR* pucFileName,USHORT uiRecordLen,UINT uiStartNum,UINT uiDeleteNums)
{
	int		iHandle;
	UINT	uiRecordNums,uiI;
	UCHAR	ucUINTLen;
	UCHAR	aucBuff[255],ucResult;
	
	ucResult=SALEERR_SUCCESS;
	ucUINTLen=sizeof(UINT);
	
	iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ|O_WRITE);
	
	if(iHandle<0) return SALEERR_OPENFILE;
		
	ucResult=FILE_ReadRecordNums(iHandle,&uiRecordNums);
	if(ucResult) return ucResult;
	
	if(uiStartNum>uiRecordNums) return SALEERR_DELETERECORD;
	if(uiStartNum+uiDeleteNums-1>uiRecordNums)
		uiDeleteNums=uiRecordNums-uiStartNum;
		
	ucResult=SALEERR_SUCCESS;
	for(uiI=uiStartNum+uiDeleteNums;!ucResult&&uiI<=uiRecordNums;uiI++)
	{
		ucResult=FILE_ReadRecord(iHandle,uiI,aucBuff,uiRecordLen);
		if(!ucResult) ucResult=FILE_WriteRecord(iHandle,uiI-uiDeleteNums,aucBuff,uiRecordLen);
	}
	uiRecordNums-=uiDeleteNums;
	if(!ucResult) ucResult=FILE_WriteRecordNums(iHandle,uiRecordNums);
	if(!ucResult) ucResult=OSAPP_FileTrunc(iHandle,ucUINTLen+uiRecordNums*uiRecordLen);
	if(iHandle>=0) OSAPP_FileClose(iHandle);
	return ucResult;
}


UCHAR	FILE_UpdateRecordByFileName(UCHAR* pucFileName,UINT uiRecordNum,UCHAR* pRecordBuff,USHORT uiRecordLen)
{
	int		iHandle;
	UCHAR	ucResult;
	
	ucResult=SALEERR_SUCCESS;
	
	iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ|O_WRITE);
	if(iHandle>=0)
	{
		ucResult=FILE_WriteRecord(iHandle,uiRecordNum,pRecordBuff,uiRecordLen);
		OSAPP_FileClose(iHandle);
	}
	else ucResult=SALEERR_OPENFILE;
	return ucResult;
}


