
#include <test/include.h>
#include <TypeDef.h>
#include <EMVError.h>
#include <test/Global.h>
#include <test/Util.h>


extern UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);

UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr);

unsigned char OnEve_power_on(void)
{
	unsigned char ucResult;
	USHORT	 uiRecNum,uiI;
	
	ucResult  = EMVERROR_SUCCESS;

	if( ucResult == EMVERROR_SUCCESS )                     
		ucResult = OnEve_CheckExistFile((PUCHAR)CONSTPARAMFILE,sizeof(EMVCONSTPARAM),(PUCHAR)&ConstParam);
	if( ucResult == EMVERROR_SUCCESS )                     
		ucResult = OnEve_CheckExistFile((PUCHAR)EMVCONFIGFILE,sizeof(TERMCONFIG),(PUCHAR)&EMVConfig);
	
	
	if(!ucResult)
	{
		if( ucResult == EMVERROR_SUCCESS)
		ucResult= OnEve_CheckExistFile((PUCHAR)CAPKFILE,0,NULL);
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)TERMSUPPORTAPPFILE,0,NULL);
		
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)TRANSLOG,0,NULL);	
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)EXCEPTFILE,0,NULL);
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)BATCHRECORD,0,NULL);
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)IPKREVOKEFILE,0,NULL);
		
		ucResult =FILE_ReadRecordNumByFileName(CAPKFILE, &ucTermCAPKNum);
		if(!ucResult)
			for(uiI =0;!ucResult &&uiI< ucTermCAPKNum && uiI < MAXTERMCAPKNUMS;uiI++)
				ucResult = FILE_ReadRecordByFileName(CAPKFILE, uiI+1, &TermCAPK[uiI], sizeof(CAPK));
		if(!ucResult) 
		{
			EMVConfig.ucCAPKNums = ucTermCAPKNum;
			EMVConfig.pTermSupportCAPK =TermCAPK;
			
		}

//		if(!ucResult)
		{
			ucResult =FILE_ReadRecordNumByFileName(TERMSUPPORTAPPFILE, &ucTermAIDNum);
			if(!ucResult)
				for(uiI =0;!ucResult &&uiI< ucTermAIDNum && uiI < MAXTERMAIDNUMS;uiI++)
					ucResult = FILE_ReadRecordByFileName(TERMSUPPORTAPPFILE, uiI+1, &TermAID[uiI], sizeof(TERMSUPPORTAPP));
			if(!ucResult) 
			{
				EMVConfig.ucCAPKNums = ucTermAIDNum;
				EMVConfig.pTermSupportApp=TermAID;
				
			}
		}

//		if(!ucResult)
		{
			ucResult =FILE_ReadRecordNumByFileName(EXCEPTFILE, &ucExceptFileNum);
			if(!ucResult)
				for(uiI =0;!ucResult &&uiI< ucExceptFileNum && uiI <MAXEXCEPTFILENUMS;uiI++)
					ucResult = FILE_ReadRecordByFileName(EXCEPTFILE, uiI+1, &ExceptFile[uiI], sizeof(EXCEPTPAN));
			if(!ucResult) 
			{
				EMVConfig.ucExceptFileNum= ucExceptFileNum;
				EMVConfig.pExceptPAN=ExceptFile;
				
			}
		}

//		if(!ucResult)
		{
			ucResult =FILE_ReadRecordNumByFileName(IPKREVOKEFILE, &ucIPKRevokeNum);
			if(!ucResult)
				for(uiI =0;!ucResult &&uiI< ucIPKRevokeNum && uiI <MAXIPKREVOKENUMS ;uiI++)
					ucResult = FILE_ReadRecordByFileName(IPKREVOKEFILE, uiI+1, &IPKRevoke[uiI], sizeof(IPKREVOKE));
			if(!ucResult) 
			{
				EMVConfig.ucIPKRevokeNum= ucIPKRevokeNum;
				EMVConfig.pIPKRevoke=IPKRevoke;
				
			}
		}


	}

	if(strlen((char*)ConstParam.aucTerminalID)==0||strlen((char*)ConstParam.aucMerchantID)==0)
		TERMSETTING_LoadDefaultSetting();

	EMVConfig.pfnEMVICC = EMVICC;
	EMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	EMVConfig.pfnOnlinePIN = UTIL_OnlinePIN;
	EMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	
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
	return EMVERROR_SUCCESS;
}

