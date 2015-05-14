
#include <include.h>
#include <TypeDef.h>
#include <EMVError.h>

#include <global.h>
#include <util.h>
#include <xdata.h>
#include <emvinterface.h>
#ifdef TMS
#include <tms.h>
#include <tmsapp.h>
#include <interfaceparam.h>
#include <interface.h>
#endif
#if SANDREADER
#include    <QPBOC.h>
#endif
extern UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);

UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr);


void OnEve_power_on(void)
{
	unsigned char ucResult;
	unsigned int ucTMSIZE;

	ucResult = SUCCESS;
	ucResult = CFG_Check_File_All();
#ifdef	TMS
	util_Printf("\nDataSave0.ConstantParamData.ucTMSFlag = %02x\n",DataSave0.ConstantParamData.ucTMSFlag);
	if(!ucResult)
		ucResult = TMSAPP_CREATETMSFile();
	util_Printf("ucResutl = %02x\n",ucResult);

	ucTMSIZE = OSAPP_FileSize((char *)TMSDATANAME) > 0 ? true:false;
	util_Printf("TMS 文件大小 ucTMSIZE = %d\n",ucTMSIZE);

	if ((ucResult == SUCCESS)&&(DataSave0.ConstantParamData.ucTMSFlag == 0x31))
	{
		util_Printf("\nTMS参数开始更新了\n");
		TMSAPP_UpdateTMSData(TMS_WriteFile);
	}
	util_Printf("\n\nTMS参数更新结束了\n");
#endif
	UTIL_CheckProjAndVer();
	//2009-9-6 16:21 cbf
	util_Printf("\nReaderSupport=%d",ReaderSupport);
	util_Printf("\nReaderType =%d",ReaderType);
	if(ReaderSupport)
	{
		if(ReaderType==READER_HONGBAO)
		{
			ucResult = File_CheckExistFile((unsigned char*)INTERFACEBASICCONFIGDATA,sizeof(BASICCONFIG));
			util_Printf("\nFile_CheckExistFile=Abc=%d",ucResult);
			if( ucResult == SUCCESS )
				ucResult = File_CheckExistFile((unsigned char*)INTERFACEKEYDATA,sizeof(INTERFACEKEYFILE));

			if( ucResult == SUCCESS )
				OnEve_INTERFACE_Init();
			util_Printf("\nFile_CheckExistFile=Abc2=%d",ucResult);
		}
	}
#if 0
	DataSave0.ChangeParamData.ucCashierLogonFlag=0x55;
	XDATA_Write_Vaild_File(DataSaveChange);
#endif
}

unsigned char EMV_Check_file(void)
{
	unsigned char ucResult;
	USHORT uiI;

	ucResult  = EMVERROR_SUCCESS;
	util_Printf("READ TO CHECK EMV FILE\n");
	ucResult = OnEve_CheckExistFile((PUCHAR)CONSTPARAMFILE,sizeof(EMV_CONSTPARAM),(PUCHAR)&ConstParam);
	if( ucResult != EMVERROR_SUCCESS)
		util_Printf("------CONSTPARAMFILE----0---err----\n");

	if( ucResult == EMVERROR_SUCCESS )
		ucResult = OnEve_CheckExistFile((PUCHAR)EMVCONFIGFILE,sizeof(TERMCONFIG),(PUCHAR)&EMVConfig);
	if( ucResult != EMVERROR_SUCCESS)
		util_Printf("------EMVCONFIGFILE----0---err----\n");

	if(!ucResult)
	{
		ucResult= OnEve_CheckExistFile((PUCHAR)CAPKFILE,0,NULL);

		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)TERMSUPPORTAPPFILE,0,NULL);//AID
		else util_Printf("------CAPKFILE----0---err----\n");
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)TRANSLOG,0,NULL);
		else util_Printf("------TERMSUPPORTAPPFILE----0---err----\n");
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)EXCEPTFILE,0,NULL);
		else util_Printf("------TRANSLOG----0---err----\n");
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)BATCHRECORD,0,NULL);
		else util_Printf("------EXCEPTFILE----0---err----\n");
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)IPKREVOKEFILE,0,NULL);
		else util_Printf("------BATCHRECORD----0---err----\n");
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)EMVAIDPARAMFILE,0,NULL);
		else util_Printf("------IPKREVOKEFILE----0---err----\n");
		if( ucResult == EMVERROR_SUCCESS)
			ucResult= OnEve_CheckExistFile((PUCHAR)ERRICCTRANS,0,NULL);
		else util_Printf("------EMVAIDPARAMFILE----0---err----\n");
		if( ucResult != EMVERROR_SUCCESS)
			util_Printf("------ERRICCTRANS----1---err----\n");

		ucResult =FILE_ReadRecordNumByFileName(CAPKFILE, &ucTermCAPKNum);
		util_Printf("CHECK CAPKFILENUM: %02x\n",ucResult);
		if(!ucResult)
			for(uiI =0;!ucResult &&uiI< ucTermCAPKNum && uiI < MAXTERMCAPKNUMS;uiI++)
				ucResult = FILE_ReadRecordByFileName(CAPKFILE, uiI+1, &TermCAPK[uiI], sizeof(CAPK));
		util_Printf("CHECK CAPKFILE: %02x\n",ucResult);
		if(!ucResult)
		{
			EMVConfig.ucCAPKNums = ucTermCAPKNum;
			EMVConfig.pTermSupportCAPK =TermCAPK;
			util_Printf("-----EMV_Check_file()-------ucTermCAPKNum = %d\n",ucTermCAPKNum);
		}
		ucResult =FILE_ReadRecordNumByFileName(TERMSUPPORTAPPFILE, &ucTermAIDNum);//AID
		if(!ucResult)
		{
			for(uiI =0;!ucResult &&uiI< ucTermAIDNum && uiI < MAXTERMAIDNUMS;uiI++)
				ucResult = FILE_ReadRecordByFileName(TERMSUPPORTAPPFILE, uiI+1, &TermAID[uiI], sizeof(TERMSUPPORTAPP));
			for(uiI =0;!ucResult &&uiI< ucTermAIDNum && uiI < MAXTERMAIDNUMS;uiI++)
				ucResult = FILE_ReadRecordByFileName(EMVAIDPARAMFILE, uiI+1, &AIDParam[uiI], sizeof(EMVAIDPARAM));
		}
		if(!ucResult)
		{
		        util_Printf("\n==================");
			EMVConfig.pTermSupportApp=TermAID;
			SetSandReaderAIDInfo();
		}
		ucResult =FILE_ReadRecordNumByFileName(EXCEPTFILE, &ucExceptFileNum);
		if(!ucResult)
			for(uiI =0;!ucResult &&uiI< ucExceptFileNum && uiI <MAXEXCEPTFILENUMS;uiI++)
				ucResult = FILE_ReadRecordByFileName(EXCEPTFILE, uiI+1, &ExceptFile[uiI], sizeof(EXCEPTPAN));
		if(!ucResult)
		{
			EMVConfig.ucExceptFileNum= ucExceptFileNum;
			EMVConfig.pExceptPAN=ExceptFile;
		}
		ucResult =FILE_ReadRecordNumByFileName(IPKREVOKEFILE, &ucIPKRevokeNum);
		if(!ucResult)
			for(uiI =0;!ucResult &&uiI< ucIPKRevokeNum && uiI <MAXIPKREVOKENUMS ;uiI++)
		if(!ucResult)
		{
			EMVConfig.ucIPKRevokeNum= ucIPKRevokeNum;
			EMVConfig.pIPKRevoke=IPKRevoke;
		}
	}
	if(!ConstParam.ucFlag )
	{
//		TERMSETTING_LoadDefaultSetting();
	}
	//util_Printf("emv默认参数载入完成:%02x\n",ucResult);

	EMVConfig.pfnEMVICC = EMVICC;
	EMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	EMVConfig.pfnOnlinePIN = UTIL_InputEncryptPIN;
	EMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
	return ucResult;
}

UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr)
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult=SUCCESS;


	if((iFileSize  =  OSAPP_FileSize((char*)pszFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char*)pszFileName,O_CREATE);
		//util_Printf("\n文件句柄:[%d]\n",iFileHandel);

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
			//util_Printf("文件句柄w:[%d]\n",iFileHandel);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			//util_Printf("文件块w:[%d]\n",iFileErrNo);
			if(!iFileErrNo&&pLoadAddr)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)pLoadAddr,iFileSize);
                     //util_Printf("文件块:[%d]\n",iFileErrNo);
		}else
		if(uiFileLen)
		{
			iFileHandel = OSAPP_OpenFile((char *)pszFileName,O_READ);
			//util_Printf("文件句柄q:[%d]\n",iFileHandel);
			OSAPP_FileTrunc(iFileHandel,0);
			memset(pLoadAddr,0x00,uiFileLen);
		}
		OSAPP_FileClose(iFileHandel);
	}
	ucResult = OSAPP_FileGetLastError();
	util_Printf("\nEMV[%s]文件系统值:%02x\n",pszFileName,ucResult);
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}

