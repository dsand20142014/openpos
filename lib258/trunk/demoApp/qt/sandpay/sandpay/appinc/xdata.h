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

DATASAVE DATASAVEPAGE0	DataSave0;
DATASAVE DATASAVEPAGE1	DataSave1;







#define DataSaveMaxIndex			20

#define DataSaveConstant			    0
#define DataSaveChange				    1
#define DataSaveTransInfo			    2 
#define DataSaveSaveISO8583       3
#define DataSaveSendISO8583       4
#define DataSaveReversalISO8583   5
#define DataSaveSaveTrans         6
#define DataSaveBatchFlags        7
#define DataSaveBlackFeilds       8
#define DataSaveZoneCtrls         9
#define DataSaveCardTables        10
#define DataSaveRedoTrans                  11
#define DataSaveRate        12
#define DataSaveTemp					13
#define DataSaveIssueInfo					14
#define DataSaveLastIssueInfo					15
#define DataSaveExpInfo					16



 




#define xDATA_ConstantSize			        sizeof(CONSTANTPARAM)
#define xDATA_ChangeSize			          sizeof(CHANGEPARAM)
#define xDATA_TransInfoSize			        sizeof(TRANSINFO)
#define xDATA_BackupISO8583Size		      sizeof(BACKUPISO8583)
#define xDATA_ReversalISO8583Size			  sizeof(REVERSALISO8583)
#define xDATA_SaveTransSize			        sizeof(NORMALTRANS)
#define xDATA_BatchFlagsSize			      TRANS_MAXNB
#define xDATA_BlackFeildsSize			       sizeof(BLACKFIELD)
#define xDATA_ZoneCtrlsSize			        sizeof(CARDZONE)
#define xDATA_CardTablesSize			      sizeof(CARDTABLE)
#define xDATA_RedoTransSize	              sizeof(REDOTRANS)
#define xDATA_RateSize			      sizeof(RATETABLE)
#define xDATA_TempSize	  		sizeof(NORMALTRANS)+BaseSize
#define xDATA_ExpSize			      sizeof(EXPDATA)






typedef struct
{
	char aucFileName[30];
}FILEINFO;


DATASAVE FILEINFO 	const	FileInfo[DataSaveMaxIndex];

#define xDATA_Constant				         DataSave0.ConstantParamData	
#define xDATA_Change				           DataSave0.ChangeParamData
#define xDATA_TransInfo                DataSave0.TransInfoData
#define xDATA_SaveISO8583              DataSave0.SaveISO8583Data
#define xDATA_SendISO8583              DataSave0.SendISO8583Data
#define xDATA_ReversalISO8583          DataSave0.ReversalISO8583Data
#define xDATA_SaveTrans                DataSave0.SaveTransData 
#define xDATA_ZoneCtrls                DataSave1.ZoneCtrl
#define xDATA_CardTables               DataSave1.CardTable
#define xDATA_Rate              DataSave1.RateTable
#define xDATA_Black              DataSave1.BlackFeild




unsigned char XDATA_Write_Vaild_File(unsigned char ucFileIndex);
unsigned char XDATA_Read_Vaild_File(unsigned char ucFileIndex);



