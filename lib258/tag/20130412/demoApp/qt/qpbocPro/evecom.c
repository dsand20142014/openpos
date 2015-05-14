
#include <include.h>
#include <QTypeDef.h>
#include <QError.h>
#include <Global.h>
#include <Util.h>


extern UCHAR	QICC(QICCIN *pQICCIn ,QICCOUT *pQICCOut);

UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr);

unsigned char OnEve_power_on(void)
{
	unsigned char ucResult;
	USHORT	 uiRecNum,uiI;
	
	ucResult  = QERROR_SUCCESS;

	if( ucResult == QERROR_SUCCESS )                     
		ucResult = OnEve_CheckExistFile((PUCHAR)CONSTPARAMFILE,sizeof(QEMVCONSTPARAM),(PUCHAR)&QConstParam);
	if( ucResult == QERROR_SUCCESS )                     
		ucResult = OnEve_CheckExistFile((PUCHAR)EMVCONFIGFILE,sizeof(QTERMCONFIG),(PUCHAR)&QConfig);
	if( ucResult == QERROR_SUCCESS )                     
		ucResult = OnEve_CheckExistFile((PUCHAR)READERPARAMFILE,sizeof(QREADERPARAM),(PUCHAR)&QReaderParam);
	
	if(!ucResult)
	{
		if( ucResult == QERROR_SUCCESS)
		ucResult= OnEve_CheckExistFile((PUCHAR)CAPKFILE,0,NULL);
		if( ucResult == QERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)QTERMSUPPORTAPPFILE,0,NULL);
		
		if( ucResult == QERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)TRANSLOG,0,NULL);	
		if( ucResult == QERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)EXCEPTFILE,0,NULL);
		if( ucResult == QERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)BATCHRECORD,0,NULL);
		if( ucResult == QERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)IPKREVOKEFILE,0,NULL);
		
		ucResult =FILE_ReadRecordNumByFileName(CAPKFILE, &ucQTermCAPKNum);
		if(!ucResult)
			for(uiI =0;!ucResult &&uiI< ucQTermCAPKNum && uiI < QMAXTERMCAPKNUMS;uiI++)
				ucResult = FILE_ReadRecordByFileName(CAPKFILE, uiI+1, &QTermCAPK[uiI], sizeof(QCAPK));
		if(!ucResult) 
		{
			QConfig.ucCAPKNums = ucQTermCAPKNum;
			QConfig.pTermSupportCAPK =QTermCAPK;
			
		}

//		if(!ucResult)
		{
			ucResult =FILE_ReadRecordNumByFileName(QTERMSUPPORTAPPFILE, &ucQTermAIDNum);
			if(!ucResult)
				for(uiI =0;!ucResult &&uiI< ucQTermAIDNum && uiI < QMAXTERMAIDNUMS;uiI++)
					ucResult = FILE_ReadRecordByFileName(QTERMSUPPORTAPPFILE, uiI+1, &QTermAID[uiI], sizeof(QTERMSUPPORTAPP));
			if(!ucResult) 
			{
				QConfig.ucCAPKNums = ucQTermAIDNum;
				QConfig.pTermSupportApp=QTermAID;
				
			}
		}

//		if(!ucResult)
		{
			ucResult =FILE_ReadRecordNumByFileName(EXCEPTFILE, &ucQExceptFileNum);
			if(!ucResult)
				for(uiI =0;!ucResult &&uiI< ucQExceptFileNum && uiI <QMAXEXCEPTFILENUMS;uiI++)
					ucResult = FILE_ReadRecordByFileName(EXCEPTFILE, uiI+1, &QExceptFile[uiI], sizeof(QEXCEPTPAN));
			if(!ucResult) 
			{
				QConfig.ucQExceptFileNum= ucQExceptFileNum;
				QConfig.pExceptPAN=QExceptFile;
				
			}
		}

//		if(!ucResult)
		{
			ucResult =FILE_ReadRecordNumByFileName(IPKREVOKEFILE, &ucQIPKRevokeNum);
			if(!ucResult)
				for(uiI =0;!ucResult &&uiI< ucQIPKRevokeNum && uiI <QMAXIPKREVOKENUMS ;uiI++)
					ucResult = FILE_ReadRecordByFileName(IPKREVOKEFILE, uiI+1, &QIPKRevoke[uiI], sizeof(QIPKREVOKE));
			if(!ucResult) 
			{
				QConfig.ucQIPKRevokeNum= ucQIPKRevokeNum;
				QConfig.pIPKRevoke=QIPKRevoke;
				
			}
		}


	}

	if(strlen((char*)QConstParam.aucTerminalID)==0||strlen((char*)QConstParam.aucMerchantID)==0)
		TERMSETTING_LoadDefaultSetting();

	QConfig.pfnQICC = QICC;
	QConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	QConfig.pfnOnlinePIN = UTIL_OnlinePIN;
	QConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	
	return ucResult;	

	
}

UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr)
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;

	
	if((iFileSize  =  OSAPP_FileSize((char*)pszFileName))<0)
	{  		
		iFileHandel = OSAPP_OpenFile((char*)pszFileName,O_CREATE);

		if(iFileHandel>=0)
		{				
			OSAPP_FileClose(iFileHandel);
			memset(pLoadAddr,0x00,uiFileLen);
		}
	}else
	{		
		if(iFileSize==uiFileLen)
		{	
			iFileHandel = OSAPP_OpenFile((char*)pszFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo&&pLoadAddr)		
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)pLoadAddr,iFileSize);
					
		}else 
		if(uiFileLen)
		{
			iFileHandel = OSAPP_OpenFile((char *)pszFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
			memset(pLoadAddr,0x00,uiFileLen);
		}		
		OSAPP_FileClose(iFileHandel);
	}	
	//return(OSAPP_FileGetLastError());
	return QERROR_SUCCESS;
}

