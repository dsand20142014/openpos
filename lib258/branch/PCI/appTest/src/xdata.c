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
#include <Global.h>

#define DATASAVE
#include <xdata.h>

#undef DATASAVE


FILEINFO  const FileInfo[DataSaveMaxIndex]=
{
	{"constant"},			//存储固定参数
	{"change"},			//存储动态参数 Eg:流水号..
	{"savetrans"},
	{"transinfo"},	
	{0}
};



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
				case DataSaveConstant:
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Constant,xDATA_ConstantSize);
					break;
				case DataSaveChange:
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
					break;
				case DataSaveTransInfo: 
					iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_TransInfo,xDATA_TransInfoSize);
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
            ucResult = ERR_WRITEFILE;
        }
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
			case DataSaveConstant:
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Constant,xDATA_ConstantSize);
				break;
			case DataSaveChange:
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
				break;
			case DataSaveTransInfo: 
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_TransInfo,xDATA_TransInfoSize);
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
            ucResult = ERR_WRITEFILE;
        }
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
	}

	OSAPP_FileClose(iFileHandel);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            ucResult = ERR_WRITEFILE;
        }
	return(ucResult);
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
			iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_SaveTrans,xDATA_NormalTransSize);
		}
		OSAPP_FileClose(iFileHandel);
	}
	return(OSAPP_FileGetLastError());

}

