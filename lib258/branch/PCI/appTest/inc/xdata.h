/*
	SAND China
--------------------------------------------------------------------------
	FILE  xdata0.h								(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Include file for the application.

    List of routines in file :

    File history :

*/

#ifndef DATASAVE
#define DATASAVE extern
#endif


#include	<include.h>


DATASAVE DATASAVEPAGE0		DataSave0;
DATASAVE DATASAVEPAGE1		DataSave1;




#define DataSaveConstant				0
#define DataSaveChange				1
#define DataSaveSaveTrans			2
#define DataSaveTransInfo			3


#define DataSaveMaxIndex			9

#define xDATA_ConstantSize				sizeof(CONSTANTPARAM)
#define xDATA_ChangeSize				sizeof(CHANGEPARAM)
#define xDATA_NormalTransSize			sizeof(NORMALTRANS)
#define xDATA_TransInfoSize			       sizeof(TRANSINFO)

typedef struct
{
	char aucFileName[30];
}FILEINFO;

DATASAVE FILEINFO  const FileInfo[DataSaveMaxIndex];

#define xDATA_Constant			DataSave0.ConstantParamData
#define xDATA_Change			DataSave0.ChangeParamData
#define xDATA_SaveTrans			DataSave1.SaveTrans
#define xDATA_TransInfo			DataSave0.TransInfoData





unsigned char XDATA_Read_Vaild_File(unsigned char ucFileIndex);
unsigned char XDATA_Write_Vaild_File(unsigned char ucFileIndex);



