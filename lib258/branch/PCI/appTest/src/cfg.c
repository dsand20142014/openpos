#include <include.h>
#include <Global.h>
#include <xdata.h>

static void *OSMEM_Memset(void *pvDest,int iValue,unsigned int uiLen)
{
    return(memset(pvDest,iValue,uiLen));
}

unsigned char CFG_CheckConstantParamData(void)
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult=SUCCESS;


	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveConstant].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveConstant].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_Constant,0,xDATA_ConstantSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{
		if(iFileSize==xDATA_ConstantSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveConstant].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Constant,xDATA_ConstantSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveConstant].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}

	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}


unsigned char CFG_CheckChangeParamData(void)
{
	unsigned char ucResult=SUCCESS;
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;


	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveChange].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveChange].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_Change,0,xDATA_ChangeSize);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{
		if(iFileSize==xDATA_ChangeSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveChange].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveChange].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}

unsigned char CFG_CheckSaveTrans(void)
{
	long iFileSize;
	int iFileHandel;
	unsigned char ucResult=SUCCESS;

	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSAPP_FileClose(iFileHandel);
		}
	}

	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}

unsigned char CFG_ConstantParamBaseData(void)
{
       unsigned char 	ucResult,aucIp[2];
	unsigned char 	aucBuf[50],aucTmpBuf[5];
	unsigned short 	uiSpeed,usI;
	unsigned long	traceNum;
	unsigned char	ucNextKey,ucIpType,ucChar;
	unsigned char ucInputFlag,ucCompare;

	ucResult = SUCCESS;
	Os__clr_display(0);
	Os__clr_display(1);
	Os__clr_display(2);
	Os__clr_display(3);
	Os__clr_display(4);
	Os__clr_display(5);

	if(!ucResult)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__GB2312_display(0, 0, (uchar *)"TerminalNo:\r");
		Os__GB2312_display(0, 0, (uchar *)"终端号：\r");
		memcpy(aucBuf,DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
		if (UTIL_Input(1,true,8,8,'N',aucBuf,0) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		else
		{
		       memcpy(DataSave0.ConstantParamData.aucTerminalID,aucBuf,PARAM_TERMINALIDLEN);
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}

	if(!ucResult)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		Os__clr_display(0);
		Os__clr_display(1);
		//Os__GB2312_display(0, 0, (uchar *)"MerchantID");
		Os__GB2312_display(0, 0, (uchar *)"商户号：");
		memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
		if (UTIL_Input(1,true,15,15,'N',aucBuf,0) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}else
		{
			memcpy(DataSave0.ConstantParamData.aucMerchantID,aucBuf,PARAM_MERCHANTIDLEN);
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}

	memset(DataSave0.ConstantParamData.aucMerchantName,0,PARAM_MERCHANTNAMELEN);
	memcpy(DataSave0.ConstantParamData.aucMerchantName,"SANDTEST",8);
     
      DataSave0.ConstantParamData.font = 0x1e;
      DataSave0.ConstantParamData.printtimes = 2;
      XDATA_Write_Vaild_File(DataSaveConstant);

      if(ucResult == SUCCESS)
	{
		memset(aucBuf,0x31,sizeof(aucBuf));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					KEYARRAY_GOLDENCARDSH,KEYINDEX_PURSECARD_MASTERKEY,
						aucBuf,aucBuf);
	}

	  if(ucResult == SUCCESS)
	{
 		ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
				KEYARRAY_GOLDENCARDSH,KEYINDEX_PURSECARD_MASTERKEY,
				KEYINDEX_PURSECARD_MASTERKEY,aucBuf);
	}
	Os__clr_display(0);
	Os__clr_display(1);
	Os__clr_display(2);
	Os__clr_display(3);
	Os__clr_display(4);
	Os__clr_display(5);
	Os__GB2312_display(4, 3, (uchar *)"设置成功！");
	sleep(1);

	 return(ucResult);	
        
}

unsigned char CFG_CheckTransInfoData(void)
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;
       unsigned char ucResult=SUCCESS;


	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveTransInfo].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTransInfo].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_TransInfo,0,xDATA_TransInfoSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{	
		if(iFileSize==xDATA_TransInfoSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTransInfo].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_TransInfo,xDATA_TransInfoSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTransInfo].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}




