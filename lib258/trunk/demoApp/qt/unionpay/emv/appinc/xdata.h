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

#include	<EMV41.h>
#include	<include.h>


DATASAVE DATASAVEPAGE0		DataSave0;
DATASAVE DATASAVEPAGE1		DataSave1;


#define G_SaveData_PageSave	DataSave0.ChangeParamData
#define TRANS_MAXNB			G_SaveData_PageSave.uiMaxTotalNb
//--------------------------------------------------------------------

//MutiApp File System

#define DataSaveConstant				0
#define DataSaveChange				1
#define DataSaveCashier				2
#define DataSaveTransInfo			3
#define DataSaveTrans8583			4
#define DataSaveSaveTrans			5
#define TempData	                		6
#define SaveCollectData	           7


#define DataSaveMaxIndex			9

#define xDATA_ConstantSize				sizeof(CONSTANTPARAM)
#define xDATA_ChangeSize				sizeof(CHANGEPARAM)
#define xDATA_CashierSize				sizeof(SYSCASHIER)
#define xDATA_TransInfoSize			sizeof(TRANSINFO)
#define xDATA_Trans8583Size			sizeof(TRANS_8583_DATA)
#define xDATA_NormalTransSize			sizeof(NORMALTRANS)


typedef struct
{
	char aucFileName[30];
}FILEINFO;

DATASAVE FILEINFO  const FileInfo[DataSaveMaxIndex];

#define xDATA_Constant			DataSave0.ConstantParamData
#define xDATA_Change				DataSave0.ChangeParamData
#define xDATA_Cashier				DataSave0.Cashier_SysCashier_Data
#define xDATA_TransInfo			DataSave0.TransInfoData
#define xDATA_Trans8583			DataSave0.Trans_8583Data
#define xDATA_SaveTrans			DataSave1.SaveTrans

#define xDATA_CollectTrans		DataSave0.SaveTrans1


unsigned char XDATA_Write_SaveTrans_File(unsigned short uiIndex);
unsigned char XDATA_Write_SaveTrans_File_O(unsigned short uiIndex , NORMALTRANS * OldNormalTransData );
unsigned char GetOldTransData(unsigned short  uiIndex);
unsigned char GetOldTransData_O(unsigned short  uiIndex , NORMALTRANS * OldNormalTransData);
unsigned char XDATA_Read_SaveTrans_File(unsigned short uiIndex);
unsigned char XDATA_Read_SaveTrans_File_O(unsigned short uiIndex  , NORMALTRANS * OldNormalTransData);
unsigned char XDATA_Clear_SaveTrans_File(void);
unsigned char XDATA_Read_Vaild_File(unsigned char ucFileIndex);
unsigned char XDATA_Write_Vaild_File(unsigned char ucFileIndex);
unsigned char XDATA_ClearAllAPPFile(void);
unsigned char XDATA_Clear_TempFile(void);



