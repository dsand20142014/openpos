/*
	SAND China
--------------------------------------------------------------------------
	FILE  xdata0.c								(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        This file contains the data saved in the DATA page 0.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>

#define DATASAVE
#include <xdata.h>

#undef DATASAVE


FILEINFO  const FileInfo[DataSaveMaxIndex]=
{
	{"constant"},			//存储固定参数
	{"change"},			//存储动态参数 Eg:流水号..
	{"cashier"},			//存储柜员号
	{"transinfo"},			//存储统计信息.
	{"trans8583"},			//存储8583包
	{"savetrans"},			//存储交易信息
	{"tempdata"},			//预写数据文件
	{"colldata"},			//采集数据文件
	{0}
};

unsigned char XDATA_Read_CollectTrans_File(unsigned short uiIndex)
{
	int iFileHandel;
	int iFileErrNo;

	if(OSAPP_FileSize((char *)FileInfo[SaveCollectData].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[SaveCollectData].aucFileName,O_READ);
		iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
		if(!iFileErrNo)
		{
			iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_CollectTrans,xDATA_NormalTransSize);
		}
		OSAPP_FileClose(iFileHandel);
	}

	return(OSAPP_FileGetLastError());
}

unsigned char XDATA_Write_CollectTrans_File(unsigned short uiIndex)
{
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult;

	iFileHandel = OSAPP_OpenFile((char *)FileInfo[SaveCollectData].aucFileName,O_WRITE);
	iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
	if(!iFileErrNo)
	{
		iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_CollectTrans,xDATA_NormalTransSize);
	}

	OSAPP_FileClose(iFileHandel);
	ucResult = OSAPP_FileGetLastError();
    if (ucResult)
    {
        G_RUNDATA_ucErrorFileCode = ucResult;
        ucResult = ERR_WRITEFILE;
    }
   util_Printf("写[%d]号文件返回值:%02x\n",SaveCollectData,ucResult);
	return(ucResult);
}


unsigned char XDATA_Write_SaveTrans_File(unsigned short uiIndex)
{
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult;

	iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_WRITE);
	iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
	if(!iFileErrNo)
	{
		iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_SaveTrans,xDATA_NormalTransSize);
		util_Printf("=====[The Write File Result= %02d]====\n",iFileErrNo);
	}
	XDATA_Read_SaveTrans_File(uiIndex);

	OSAPP_FileClose(iFileHandel);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        util_Printf("_OO写[%d]号文件返回值:%02x\n",DataSaveSaveTrans,ucResult);
	return(ucResult);
}

unsigned char XDATA_Write_SaveTrans_File_O(unsigned short uiIndex , NORMALTRANS * OldNormalTransData )
{
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult;

	iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_WRITE);
	iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
	if(!iFileErrNo)
	{
		iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)OldNormalTransData,xDATA_NormalTransSize);
		util_Printf("=====[The Write File Result= %d]====\n",iFileErrNo);
	}
	XDATA_Read_SaveTrans_File_O(uiIndex,OldNormalTransData);

	OSAPP_FileClose(iFileHandel);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        util_Printf("_O写[%d]号文件返回值:%02x\n",DataSaveSaveTrans,ucResult);
	return(ucResult);
}


unsigned char GetOldTransData(unsigned short  uiIndex)
{
	int iFileHandel;
	int iFileErrNo;

	if(OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_READ);
		iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
		if(!iFileErrNo)
		{
	//		util_Printf("=GetOld=====[uiIndex=%02x]======\n",uiIndex);
			iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&NormalTransData,xDATA_NormalTransSize);
		}
		OSAPP_FileClose(iFileHandel);
	}

//	util_Printf("====[TheFileSize Result = %d,Structure Size=%d]=====\n",OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName),xDATA_NormalTransSize);

	return(OSAPP_FileGetLastError());
}

unsigned char GetOldTransData_O(unsigned short  uiIndex , NORMALTRANS * OldNormalTransData)
{
	int iFileHandel;
	int iFileErrNo;

	if(OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_READ);
		iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
		if(!iFileErrNo)
		{
	//		util_Printf("=GetOld=====[uiIndex=%02x]======\n",uiIndex);
			iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)OldNormalTransData,xDATA_NormalTransSize);
		}
		OSAPP_FileClose(iFileHandel);
	}

//	util_Printf("====[TheFileSize Result = %d,Structure Size=%d]=====\n",OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName),xDATA_NormalTransSize);

	return(OSAPP_FileGetLastError());
}

unsigned char XDATA_Read_SaveTrans_File(unsigned short uiIndex)
{
	int iFileHandel;
	int iFileErrNo;

	if(OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_READ);
		iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
		if(!iFileErrNo)
		{
			util_Printf("=xData=====[uiIndex=%02x]======\n",uiIndex);
			iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_SaveTrans,xDATA_NormalTransSize);
		}
		OSAPP_FileClose(iFileHandel);
	}
	util_Printf("=第[ %3d ]个索引下的流水号:[ %3d ]=====\n",uiIndex,DataSave1.SaveTrans.ulTraceNumber);
	return(OSAPP_FileGetLastError());

}

unsigned char XDATA_Read_SaveTrans_File_O(unsigned short uiIndex  , NORMALTRANS * OldNormalTransData)
{
	int iFileHandel;
	int iFileErrNo;
	if(OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_READ);
		iFileErrNo = OSAPP_FileSeek(iFileHandel,uiIndex*xDATA_NormalTransSize,SEEK_SET);
		if(!iFileErrNo)
		{
			util_Printf("=xData=====[uiIndex=%02x]======\n",uiIndex);
			iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)OldNormalTransData,xDATA_NormalTransSize);
		}
		OSAPP_FileClose(iFileHandel);
	}
	util_Printf("=====[uiIndex: %d  .   DataSave0.SaveTrans.ulTranceNumber=%d]=====\n",uiIndex,DataSave1.SaveTrans.ulTraceNumber);
	return(OSAPP_FileGetLastError());

}

unsigned char XDATA_Clear_SaveTrans_File(void)
{
	int iFileHandel;

 	if(OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_READ);
		OSAPP_FileTrunc(iFileHandel,0);
		OSAPP_FileClose(iFileHandel);
	}
	return(OSAPP_FileGetLastError());
}

unsigned char XDATA_Read_Vaild_File(unsigned char ucFileIndex)
{
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult;

	if(OSAPP_FileSize((char *)FileInfo[ucFileIndex].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[ucFileIndex].aucFileName,O_READ);
		iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
		if(!iFileErrNo)
		{
			switch(ucFileIndex)
			{
				case DataSaveConstant:// 0
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Constant,xDATA_ConstantSize);
					break;
				case DataSaveChange:// 1
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
					break;
				case DataSaveCashier: // 2
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Cashier,xDATA_CashierSize);
					break;
				case DataSaveTransInfo: // 3
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_TransInfo,xDATA_TransInfoSize);
					break;
				case DataSaveTrans8583:// 4
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Trans8583,xDATA_Trans8583Size);
					break;
				default:
					OSAPP_FileClose(iFileHandel);
					return(FILE_NOEXIST);
			}
		}
		OSAPP_FileClose(iFileHandel);
	}
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        util_Printf("读[%d]号文件返回值:%02x\n",ucFileIndex,ucResult);
	return(ucResult);
}

unsigned char XDATA_Write_Vaild_File(unsigned char ucFileIndex)
{
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult;

	iFileHandel = OSAPP_OpenFile((char *)FileInfo[ucFileIndex].aucFileName,O_WRITE);
	iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
	if(!iFileErrNo)
	{
		switch(ucFileIndex)
		{
			case DataSaveConstant:// 0
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Constant,xDATA_ConstantSize);
				break;
			case DataSaveChange:// 1
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
				break;
			case DataSaveCashier: // 2
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Cashier,xDATA_CashierSize);
				break;
			case DataSaveTransInfo: // 3
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_TransInfo,xDATA_TransInfoSize);
				break;
			case DataSaveTrans8583:// 4
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Trans8583,xDATA_Trans8583Size);
				break;
		       case TempData:   //6
		            util_Printf("交易信息结构大小:%d\n",xDATA_NormalTransSize);
		            iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_TempFileData,(xDATA_NormalTransSize+8500));
		            break;
			default:
				OSAPP_FileClose(iFileHandel);
				return(FILE_NOEXIST);
		}
	}
	OSAPP_FileClose(iFileHandel);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
       util_Printf("写[%d]号文件返回值:%02x\n",ucFileIndex,ucResult);
	return(ucResult);
}
unsigned char XDATA_Clear_TempFile(void)
{
	int iFileHandel;
	unsigned char ucResult;

	if(OSAPP_FileSize((char *)FileInfo[TempData].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[TempData].aucFileName,O_READ);
		OSAPP_FileTrunc(iFileHandel,0);
		OSAPP_FileClose(iFileHandel);
	}

	ucResult = OSAPP_FileGetLastError();
	if (ucResult)
	{
		G_RUNDATA_ucErrorFileCode = ucResult;
		ucResult = ERR_WRITEFILE;
	}
	util_Printf("清除临时文件返回值：%02x\n",ucResult);
	return(ucResult);
}
unsigned char XDATA_ClearAllAPPFile(void)
{
	int iFileHandel;
	int i;

	for(i=0;i<DataSaveMaxIndex;i++)
	{
		if( FileInfo[i].aucFileName != NULL )
		{
			if(OSAPP_FileSize((char *)FileInfo[i].aucFileName)>0)
			{
				iFileHandel = OSAPP_OpenFile((char *)FileInfo[i].aucFileName,O_READ);
				OSAPP_FileTrunc(iFileHandel,0);
				OSAPP_FileClose(iFileHandel);
			}
		}
		else
		{
			break;
		}
	}
	return(OSAPP_FileGetLastError());
}

unsigned char XDATA_Clear_CollectTrans_File(void)
{
	int iFileHandel;
	unsigned short  uiIndex;

	for(uiIndex=0;uiIndex<DataSave0.ChangeParamData.uiSendTotalNb;uiIndex++)
	{
		DataSave0.ChangeParamData.uiSendIndex[uiIndex]=0;
	}
	DataSave0.ConstantParamData.ucSendCollectFlag = '0';					//数据采集失败标识
	DataSave0.ChangeParamData.uiSendTotalNb=0;
	XDATA_Write_Vaild_File(DataSaveChange);
	XDATA_Write_Vaild_File(DataSaveConstant);

	if(OSAPP_FileSize((char *)FileInfo[SaveCollectData].aucFileName)>0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[SaveCollectData].aucFileName,O_READ);
		OSAPP_FileTrunc(iFileHandel,0);
		OSAPP_FileClose(iFileHandel);
	}
	return(OSAPP_FileGetLastError());
}

