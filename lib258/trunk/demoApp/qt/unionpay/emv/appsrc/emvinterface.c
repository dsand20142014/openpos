

#include <include.h>
#include <xdata.h>
#include	<EMV41.h>
#include	<global.h>
#define EMVINTERFACEDATA
#include <emvinterface.h>
#undef EMVINTERFACEDATA


const EMVTAGATTR  EMVParamTagAttrInfo[]={
							{"\x9F\x06"},//AID
							{"\x9F\x08"},
							{"\x9F\x22"},//
							{"\x9F\x1B"},//FLOOR LIMIT
							{"\xDF\x01"},//ASI
							{"\xDF\x02"},//
							{"\xDF\x03"},//
							{"\xDF\x04"},//
							{"\xDF\x05"},//
							{"\xDF\x06"},//RID
							{"\xDF\x07"},//
							{"\xDF\x11"},//TAC-DEFAULT
							{"\xDF\x12"},//TAC-ONLINE
							{"\xDF\x13"},//TAC-DENY
							{"\xDF\x14"},//DDOL DEFAULT
							{"\xDF\x15"},
							{"\xDF\x16"},
							{"\xDF\x17"},
							{"\xDF\x18"},
							{"\xDF\x19"},//非接终端脱机最低限额2009-9-7 19:46cbf
							{"\xDF\x20"},//非接终端交易限额2009-9-7 19:46cbf
							{"\xDF\x21"},//终端执行CVM限额2009-9-7 19:46cbf
							{"\x9F\x7B"} //终端电子现金交易限额						2009-9-7 19:46cbf
                                        };


unsigned char CAPK_TermCAPKNum(void)
{
	if(ucTermCAPKNum)
		util_Printf("ucTermCAPKNum = %d\n",ucTermCAPKNum);
	return ucTermCAPKNum;
}

unsigned char TERMAPP_TermAppNum(void)
{
	if(ucTermAIDNum)
		util_Printf("ucTermAIDNum = %d\n",ucTermAIDNum);
	return ucTermAIDNum;
}

unsigned char TERMAPP_DeleteAll(void)
{

	memset(AppListTerm,0x00,sizeof(TERMAPP)*16);
	ucTermAIDNum=0;
	return SUCCESS;
}

EMVTAGINFO * EMVINTERFACE_SearchTag(ICTRANSINFO *pICTransInfo,unsigned char *paucTag)
{
	unsigned char ucI,ucScrLen,ucTagLen;
	EMVTAGINFO	*pEMVTagInfo;
	const EMVTAGATTR *pEMVTagAttr;

	ucScrLen=strlen((char *)paucTag);
	pEMVTagInfo=pICTransInfo->EMVTagInfo;
	pEMVTagAttr=pICTransInfo->pEMVTagAttr;

	for(ucI=0;ucI<ICTransInfo.ucTagNum;ucI++,pEMVTagAttr++,pEMVTagInfo++)
	{
		ucTagLen=strlen((char *)pEMVTagAttr->aucTag);
		if(ucScrLen==ucTagLen&&!memcmp(pEMVTagAttr->aucTag,paucTag,ucTagLen))
		{
			//if(pEMVTagInfo->uiLen&&pEMVTagInfo->pTagValue)
				return pEMVTagInfo;
		//	else break;

		}

	}

	return NULL;
}

unsigned char EMVINTERFACE_GetTagValue(ICTRANSINFO *pICTransInfo,unsigned char *paucTag,
								unsigned char *paucTagValue,unsigned short* uiTagValueLen)
{
	EMVTAGINFO	*pEMVTagInfo;
	unsigned short uiBufLen;

	pEMVTagInfo=EMVINTERFACE_SearchTag(pICTransInfo,paucTag);
	if(pEMVTagInfo==NULL)	return EMVTAGERR_TAGNOTEXIST;

	//uiBufLen=pEMVTagInfo->uiLen>*uiTagValueLen?*uiTagValueLen:pEMVTagInfo->uiLen;

	if(pEMVTagInfo->uiLen>*uiTagValueLen)
		uiBufLen=*uiTagValueLen;
	else
		uiBufLen=pEMVTagInfo->uiLen;

	memcpy(paucTagValue,pEMVTagInfo->pTagValue,uiBufLen);
	*uiTagValueLen=uiBufLen;

#ifdef EMVTAG_DEBUG
	util_Printf("\nTag %02X ",*paucTag);
	if(*paucTag&0x1f) util_Printf("%02X ",*(paucTag+1));
	util_Printf("\nTag Value");
	{
		unsigned char ucI;

		for(ucI=0;ucI<*uiTagValueLen;ucI++)
		{
			if(!(ucI%16)) util_Printf("\n");
			util_Printf("%02X ",*(paucTagValue+ucI));
		}
	}
#endif
	return EMVTAGERR_SUCCESS;
}


unsigned char EMVINTERFACE_Unpack(ICTRANSINFO *pICTransInfo)
{
	unsigned char ucI,ucLenAttr;
	unsigned char *pucBuff;
	EMVTAGINFO	*pEMVTagInfo;
	const EMVTAGATTR *pEMVTagAttr;

	util_Printf("\n****EMVTAG_Unpack*****");
	for(ucI=0,pEMVTagInfo=pICTransInfo->EMVTagInfo;ucI<EMVTAG_NUMMAX;ucI++,pEMVTagInfo++)
	{
		pEMVTagInfo->uiLen=0;
		pEMVTagInfo->pTagValue=NULL;
	}

	memset(pICTransInfo->aucEMVTagVaule,0x00,sizeof(pICTransInfo->aucEMVTagVaule));
	pICTransInfo->uiTagBuffLen=0;

	if(pICTransInfo->uiTransInfoLen<2) return EMVTAGERR_DATA;
	pucBuff=pICTransInfo->aucTransInfo;
	while(pucBuff<pICTransInfo->aucTransInfo+pICTransInfo->uiTransInfoLen)
	{
		for(ucI=0;ucI<pICTransInfo->ucTagNum;ucI++)
		{
			pEMVTagAttr=&pICTransInfo->pEMVTagAttr[ucI];
			if(!memcmp(pucBuff,pEMVTagAttr->aucTag,strlen((char *)pEMVTagAttr->aucTag)))
				break;
		}

		if(ucI<pICTransInfo->ucTagNum)
		{
			pucBuff+=strlen((char *)pEMVTagAttr->aucTag);
			pEMVTagInfo=&pICTransInfo->EMVTagInfo[ucI];
			if(!pEMVTagInfo->uiLen)
			{

				pEMVTagInfo->pTagValue=pICTransInfo->aucEMVTagVaule+pICTransInfo->uiTagBuffLen;
				ucLenAttr=EMVTAG_GetTagLenAttr(pucBuff,&pEMVTagInfo->uiLen);
				pucBuff+=ucLenAttr;
				if(pICTransInfo->uiTagBuffLen+pEMVTagInfo->uiLen>EMVTAB_BUFFMAX) return EMVTAGERR_OVERFLOW;
				memcpy(pEMVTagInfo->pTagValue,pucBuff,pEMVTagInfo->uiLen);
				pucBuff+=pEMVTagInfo->uiLen;
				pICTransInfo->uiTagBuffLen+=pEMVTagInfo->uiLen;


			}
			else return EMVTAGERR_DATA;
		}
		else pucBuff++;

	}
#ifdef EMVTAG_DEBUG
	util_Printf("\nbefore EMVTAG_DataDump");
	EMVTAG_DataDump(*pICTransInfo);
#endif

	return EMVTAGERR_SUCCESS;

}


unsigned char EMVINTERFACE_INIT(ICTRANSINFO *pICTransInfo,const EMVTAGATTR  EMVTagAttrInfo[],unsigned char ucArrayNum)
{
	const EMVTAGATTR	*pEMVTagAttr;
	EMVTAGINFO		*pEMVTagInfo;

	pEMVTagAttr=EMVTagAttrInfo;
	pEMVTagInfo=pICTransInfo->EMVTagInfo;

	memset(pICTransInfo,0x00,sizeof(ICTRANSINFO));
	//ucArrayNum=sizeof(EMVTagAttrInfo)/sizeof(EMVTAGATTR);

	if(ucArrayNum>EMVTAG_NUMMAX)	return EMVTAGERR_OVERFLOW;
	/*
	for(ucI=0;ucI<ucArrayNum;ucI++,pEMVTagAttr++,pEMVTagInfo++)
		strcpy(pEMVTagInfo->aucTag,pEMVTagAttr->aucTag);
		*/
	pICTransInfo->pEMVTagAttr=EMVTagAttrInfo;
	pICTransInfo->ucTagNum=ucArrayNum;
	pICTransInfo->uiTransInfoLen=EMVTAB_BUFFMAX;
	return EMVTAGERR_SUCCESS;


}
unsigned char EMV_LoadCAPK(void)
{
	unsigned char ucResult,I,J;

	if(G_RUNDATA_ucDialFlag != 1)
		COMMS_PreComm();

	G_NORMALTRANS_ucTransType=TRANS_SENDSTATS;
	DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_PKQUERY;
	XDATA_Write_Vaild_File(DataSaveChange);
	ucCAPKNum=0;
	ucTermCAPKNum=0;
	ucResult=LOGON_Online();

	if(!ucResult&&ucCAPKNum)
	{
		for(J=0;J<ucCAPKNum;J++)
		{
			for(I=0;I<5;I++)
				util_Printf("CAPKInfo[%d].RID[%d]:%02x",J,I,CAPKInfo[J].RID[I]);
			util_Printf(" CAPKInfo[%d].CAPKI:%02x",J,CAPKInfo[J].CAPKI);
		}

		if(ucCAPKNum)
		{
			for(J=0;J<ucCAPKNum;J++)
			{
			util_Printf("\n-------EMV_LoadCAPK------");
			for(I=0;I<5;I++)
				util_Printf("%02x",CAPKInfo[J].RID[I]);
			util_Printf(" %02x",CAPKInfo[J].CAPKI);
			}
			G_NORMALTRANS_ucTransType=TRANS_LOADPARAM;
			DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_PKLOAD;
			XDATA_Write_Vaild_File(DataSaveChange);
			ucResult=LOGON_Online();
		}
	}

	if(!ucResult)
	{
		G_NORMALTRANS_ucTransType=TRANS_SENDSTATS;
		DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_PKLOADEND;
		XDATA_Write_Vaild_File(DataSaveChange);
		ucResult=LOGON_Online();
	}
	if(G_RUNDATA_ucDialFlag != 1)
		COMMS_FinComm();

	if(G_RUNDATA_ucDialFlag != 1)
	{
		Os__saved_set(&DataSave0.ChangeParamData.ucPackgeHeadEve,0,1);
		XDATA_Write_Vaild_File(DataSaveChange);
	}

	if(!ucResult)
	{
	    ucResult = UTIL_SaveCAPKFile();
	    util_Printf("\nUTIL_SaveCAPKFile-ucResult=%02x",ucResult);
	}
	else
        {
            UTIL_Beep();
            ucCAPKNum=0;
            ucTermCAPKNum=0;
            UTIL_DeleteAllData((PUCHAR)CAPKFILE);

            //Os__clr_display(255);
            //Os__GB2312_display(1,1,(uchar *)"公钥下载失败");
            //Os__GB2312_display(2,1,(uchar *)" 请手工下载");
            UTIL_GetKey(15);
        }

	//2009-9-6 17:00 cbf
	if((!ucResult)&&ReaderSupport)
	{
		if(ReaderType==READER_HONGBAO)
		{
			ucResult = UTIL_SaveReaderCAPKFile();
			util_Printf("\nUTIL_SaveReaderCAPKFile enddPrint1--ucResult=%02x\n",ucResult);
			if(ucResult)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"与reader同步公钥");
				//Os__GB2312_display(1,0,(uchar *)"失败");
				//Os__GB2312_display(2,0,(uchar *)"请手动操作");
				UTIL_WaitGetKey(2);
				ucResult =SUCCESS;
			}
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"与reader同步公钥");
				//Os__GB2312_display(1,0,(uchar *)"成功");
				UTIL_WaitGetKey(2);
				ucResult =SUCCESS;
			}
		}
	}

	if(( ucResult != SUCCESS)&&(ucResult != ERR_CANCEL)&&(ucResult != ERR_END))
	{
		MSG_DisplayErrMsg(ucResult);
		ucResult = ERR_CANCEL;
	}

	return ucResult;

}

unsigned char EMV_SetEMVConfig(void)
{
	unsigned char ucI,ucBuf[17];
	unsigned short ucLen;
	unsigned char aucTermCapab[5];
	TACCODE		tac;

	memset( ucBuf,  0, sizeof(ucBuf));
	ucLen=sizeof(ucBuf);//modify date:080318
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",ucBuf,&ucLen);
	//hex_asc(ucBuf2, ucBuf, ucLen*2);modify date:080403
	G_RUNDATA_ucAIDLen = ucLen;
	memcpy(G_RUNDATA_aucAID,ucBuf,ucLen);
	for(  ucI=0; ucI<ucTermAIDNum; ucI++)
	{
		if(AppListTerm[ucI].ASI)				//匹配类型--完全匹配
		{
			if(!memcmp( TermAID[ucI].aucAID, ucBuf, ucLen))
				break;
		}else
		{
			if(!memcmp( TermAID[ucI].aucAID, ucBuf, TermAID[ucI].ucAIDLen))
				break;
		}
	}
	util_Printf("\n==**AID---ucI=%02x ,%02x\n",ucI,ucTermAIDNum);
	if(!memcmp(ucBuf,"\xA0\x00\x00\x03\x33",5))//设置终端行为代码，PBOC不支持明文PIN
	{
		memcpy( aucTermCapab ,"\xE0\xE1\xC8" ,TERMCAPABLEN);
	}
	else
	{
		memcpy( aucTermCapab ,EMVConfig.aucTermCapab ,TERMCAPABLEN);
	}
	if(!AIDParam[ucI].ucOnlinePINCapab)
	{
		aucTermCapab[1]&=~0x40;
	}
	EMVDATA_SetTermCapab(aucTermCapab);
	if(ucI <ucTermAIDNum)
	{
		EMVDATA_SetAppversion(AIDParam[ucI].aucAppVer);
		EMVDATA_SetDDOL(AIDParam[ucI].aucDDOL,AIDParam[ucI].ucDDOLLen);
		EMVDATA_SetFloorLimit(AIDParam[ucI].ulFloorLimit);
		//modify date:080318
		tac.bDenialExist = TRUE;
		tac.bOnlineExist = TRUE;
		tac.bDefaultExist = TRUE;
		//----end ----//
		memcpy(&tac.aucDefault,&AIDParam[ucI].aucTACDefault,5);
		memcpy(&tac.aucDenial,&AIDParam[ucI].aucTACDenial,5);  //Test
		//memcpy(&tac.aucDenial,"\x00\x00\x00\x00\x00",5);
		memcpy(&tac.aucOnline,&AIDParam[ucI].aucTACOnline,5);

		EMVDATA_SetTAC(tac);
		EMVDATA_SetThreshold(AIDParam[ucI].ulThreshold);
		EMVDATA_SetTragetPercent(AIDParam[ucI].ucTargetPercent,AIDParam[ucI].ucMaxTargetPercent);

		//2009-9-6 2:42 cbf
		if(!memcmp(ucBuf,"\xA0\x00\x00\x03\x33",5))
		{
			util_Printf("\nEMV_SetEMVConfig AID=\xA0\x00\x00\x03\x33");
	  		EMVConfig.bECFLExist = true;
	 		EMVDATA_SetECFloorLimit(true,AIDParam[ucI].ulTelECFloorLimit);
 		}
		return SUCCESS;
	}
	else		//modify date:080318
	{
		return EMV_ERRAPPNUM;
	}
	return ERR_NOTPROC;
}
#if SANDREADER
unsigned char QEMV_SetEMVConfig(void)
{
	unsigned char ucBuf[17];
	unsigned short ucLen;

	memset( ucBuf,  0, sizeof(ucBuf));
	ucLen=sizeof(ucBuf);//modify date:080318
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",ucBuf,&ucLen);
	//hex_asc(ucBuf2, ucBuf, ucLen*2);modify date:080403
	G_RUNDATA_ucAIDLen = ucLen;
	memcpy(G_RUNDATA_aucAID,ucBuf,ucLen);
	util_Printf("\nG_RUNDATA_ucAIDLen=dfd=%d",G_RUNDATA_ucAIDLen);
	return SUCCESS;
}
#endif
unsigned char EMV_LoadAIDParam(void)
{
	unsigned char ucResult=SUCCESS ,ucI;


	if(G_RUNDATA_ucDialFlag != 1)
		COMMS_PreComm();

	SupportAIDNum=0;
	TERMAPP_DeleteAll();
	G_NORMALTRANS_ucTransType=TRANS_SENDSTATS;
	DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_AIDQUERY;
	XDATA_Write_Vaild_File(DataSaveChange);
	ucResult=LOGON_Online();

	if(!ucResult&&SupportAIDNum)
	{
		util_Printf("\nLOADPARAMTYPE_AIDLOAD");
		G_NORMALTRANS_ucTransType=TRANS_LOADPARAM;
		DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_AIDLOAD;
		XDATA_Write_Vaild_File(DataSaveChange);
		ucResult=LOGON_Online();
	}

	if(!ucResult)
	{
		util_Printf("\nLOADPARAMTYPE_AIDLOADEND");
		G_NORMALTRANS_ucTransType=TRANS_LOADPARAM;
		DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_AIDLOADEND;
		XDATA_Write_Vaild_File(DataSaveChange);
		ucResult=LOGON_Online();
	}
	if(G_RUNDATA_ucDialFlag != 1)
		COMMS_FinComm();

	if(G_RUNDATA_ucDialFlag != 1)
	{
		Os__saved_set(&DataSave0.ChangeParamData.ucPackgeHeadEve,0,1);
		XDATA_Write_Vaild_File(DataSaveChange);
	}

	if(!ucResult)
	{
		for(ucI=0;!ucResult && ucI<ucTermAIDNum;ucI++)
		{
			memcpy(&TermAID[ucI] , &AppListTerm[ucI], sizeof(TERMAPP));

		}
		SetSandReaderAIDInfo();
		ucResult = UTIL_SaveAIDFile();
		util_Printf("\nUTIL_SaveAIDFile()--------ucResult=%02x-----------\n",ucResult);
		ucResult = UTIL_SaveAIDParamFile();
		util_Printf("TIL_SaveAIDParamFile()--------ucResult=%02x-----------\n",ucResult);
	}
	else
        {
            UTIL_Beep();
#if 1
            TERMAPP_DeleteAll();
            UTIL_DeleteAllData((PUCHAR)EMVAIDPARAMFILE);
            UTIL_DeleteAllData((PUCHAR)TERMSUPPORTAPPFILE);
#endif
            COMMS_FinComm();

            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(uchar *)"  参数下载失败");
            //Os__GB2312_display(2,1,(uchar *)" 请手工下载");
            UTIL_GetKey(15);
        }
	{
		unsigned char ucI,ucJ;
		util_Printf("\nTermAppNum %d\n",ucTermAIDNum);
		for(ucI=0;ucI<ucTermAIDNum;ucI++)
		{
			for(ucJ=0;ucJ<AppListTerm[ucI].AIDLen;ucJ++)
				util_Printf("%02x",AppListTerm[ucI].AID[ucJ]);
			util_Printf("\n");
		}
	}
	//配置hongbao读卡器和POS共同支持的应用
	if((!ucResult)&&ReaderSupport&&(ReaderType==READER_HONGBAO))
		 ucResult =SetHBReaderAIDInfo();

	if(( ucResult != SUCCESS)&&(ucResult != ERR_CANCEL)&&(ucResult != ERR_END))
	{
		MSG_DisplayErrMsg(ucResult);
		ucResult = ERR_CANCEL;
	}
	return ucResult;
}

unsigned char EMVONLINE_CAPKProcess(unsigned char ucProcessFlag,unsigned char* paucParam,unsigned short uiLen)
{
	CAPK CAPK_Load;
	VISAPK      VISA_Load; //定义一个非接结构体WangAn Add 20090806
	unsigned char ucResult;
	unsigned short uiTagLen,uiExponentLen;
    ICTRANSINFO   *pICTransInfo;

	util_Printf("\n------------------- EMVONLINE_CAPKProcess ----------------------\n");
	pICTransInfo=&ICTransInfo;

	EMVINTERFACE_INIT(pICTransInfo,EMVParamTagAttrInfo,sizeof(EMVParamTagAttrInfo)/sizeof(EMVTAGATTR));

	memcpy(pICTransInfo->aucTransInfo,paucParam,uiLen);
	pICTransInfo->uiTransInfoLen=uiLen;
	ucResult=EMVINTERFACE_Unpack(pICTransInfo);

	util_Printf("\nCAPK--EMVINTERFACE_Unpack()    ucResult = %02X \n",ucResult);
	if(ucResult) return ucResult;
	memset(&CAPK_Load,0x00,sizeof(CAPK_STRUCT));
	memset(&VISA_Load,0x00,sizeof(VISAPK));
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.aucRID);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\x9F\x06",CAPK_Load.aucRID,&uiTagLen);
		if(uiTagLen!=sizeof(CAPK_Load.aucRID))
			ucResult=EMV_ERRLENGTH;
		if(ucResult) util_Printf("CAPK------ 9F 06-----ucResult = %02x\n",ucResult);

		memcpy(VISA_Load.aucRID,CAPK_Load.aucRID,uiTagLen);

	}
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.ucCAPKI);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\x9F\x22",&CAPK_Load.ucCAPKI,&uiTagLen);
		if(uiTagLen!=sizeof(CAPK_Load.ucCAPKI))
			ucResult=EMV_ERRLENGTH;
		if(ucResult) util_Printf("CAPK------ 9F 06-----ucResult = %02x\n",ucResult);
		VISA_Load.ucIndex=CAPK_Load.ucCAPKI;   //(认证中心公钥索引)
	}
	if(!ucResult)
	{
		uiExponentLen=sizeof((unsigned char*)CAPK_Load.aucExponent);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\xDF\x04",CAPK_Load.aucExponent,(unsigned short*)&uiExponentLen);
		CAPK_Load.ucExponentLen=uiExponentLen;
		if(ucResult) util_Printf("CAPK------ DF 04-----ucResult = %02x\n",ucResult);
		VISA_Load.ucExponentLen = CAPK_Load.ucExponentLen; //2009-9-6 17:08c bf(公钥指数)
		memcpy(VISA_Load.aucExponent,CAPK_Load.aucExponent,uiExponentLen);
	}
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.aucModul);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\xDF\x02",CAPK_Load.aucModul,&uiTagLen);
		CAPK_Load.ucModulLen=uiTagLen;
		if(ucResult) util_Printf("CAPK------ DF 02-----ucResult = %02x\n",ucResult);
		VISA_Load.ucModulLen = CAPK_Load.ucModulLen; //2009-9-6 17:09cbf
		memcpy(VISA_Load.aucModul,CAPK_Load.aucModul,uiTagLen);
	}
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.aucCheckSum);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\xDF\x03",CAPK_Load.aucCheckSum,&uiTagLen);
		if(uiTagLen!=sizeof(CAPK_Load.aucCheckSum))
			ucResult=EMV_ERRLENGTH;
		if(ucResult) util_Printf("CAPK------ DF 03-----ucResult = %02x\n",ucResult);
		memcpy(VISA_Load.aucCheckSum,CAPK_Load.aucCheckSum,uiTagLen);	//2009-9-6 17:09cbf
	}
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.aucExpireDate);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\xDF\x05",CAPK_Load.aucExpireDate,&uiTagLen);
		if(uiTagLen!=sizeof(CAPK_Load.aucExpireDate))
			ucResult=EMV_ERRLENGTH;
		if(ucResult) util_Printf("CAPK------ DF 05-----ucResult = %02x\n",ucResult);

	}
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.ucHashInd);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\xDF\x06",&CAPK_Load.ucHashInd,&uiTagLen);

		if(ucResult) util_Printf("CAPK------ DF 06-----ucResult = %02x\n",ucResult);
		VISA_Load.ucHashInd=CAPK_Load.ucHashInd;		//WangAn Add 20090806
	}
	if(!ucResult)
	{
		uiTagLen=sizeof(CAPK_Load.ucArithInd);
		ucResult=EMVINTERFACE_GetTagValue(pICTransInfo,(unsigned char*)"\xDF\x07",&CAPK_Load.ucArithInd,&uiTagLen);

		if(ucResult) util_Printf("CAPK------ DF 07-----ucResult = %02x\n",ucResult);
		VISA_Load.ucArithInd=CAPK_Load.ucArithInd;	//2009-9-6 17:11cbf
	}
	if(!ucResult)
	{
		ucResult=VerifyCAPK(&CAPK_Load);
		if(ucResult) util_Printf("CAPK------ VerifyCAPK()-----ucResult = %02x\n",ucResult);
	}

	if(!ucResult)
	{
		if(ucTermCAPKNum<32)
		{
			memcpy(&TermCAPK[ucTermCAPKNum],&CAPK_Load,sizeof(CAPK));//CAPK_STRUCT
			memcpy(&TermVISAPK[ucTermCAPKNum],&VISA_Load,sizeof(VISAPK));

			ucTermCAPKNum++;
			ucTermCAPKNum = ucTermCAPKNum;
		}
		else ucResult=EMV_DATAOVERFLOW;


	}


#ifdef EMVPARAM_DEBUG
{
	unsigned short uiI;

	for(uiI=0;uiI<sizeof(CAPK_Load);uiI++)
	{
		if(!(uiI%16)) util_Printf("\n");
		util_Printf("%02X ",*((unsigned char*)&CAPK_Load+uiI));
	}
	util_Printf("\n555555");
}
#endif
	//Os__clr_display(255);
//	if(!ucResult)
		//Os__GB2312_display(1,0,(unsigned char *)"公钥下载成功!");
//	else
    //{
        //util_Printf("CAPK------11-----ucResult = %02x\n",ucResult);
		//Os__GB2312_display(1,0,(unsigned char *)"公钥下载失败!");
    //}
	Os__xdelay(30);
	return ucResult;

}

unsigned char EMVONLINE_AddRID(unsigned char* paucParam,unsigned short uiLen)
{
	unsigned char ucResult,ucTagLen;
	unsigned char *pParamEnd;
	CAPKINFO  *pCAPKInfo;
	unsigned char aucRID[5],ucPKIndex,aucExpireDate[8];

	pParamEnd=paucParam+uiLen;
	util_Printf("\nEMVONLINE_AddRID111 %d",ucCAPKNum);
	while(paucParam<pParamEnd&&ucCAPKNum<EMVRIDMAXNUM)
	{
//		util_Printf("\nEMVONLINE_AddRID %d",ucRIDNum);
		pCAPKInfo=&CAPKInfo[ucCAPKNum];

		if(memcmp(paucParam,"\x9F\x06",2))
			return EMV_ERRTAG;
		paucParam+=2;
		ucTagLen=*paucParam;
		if(ucTagLen!=5) return EMV_ERRLENGTH;
		paucParam++;
		//memcpy(pCAPKInfo->RID,paucParam,ucTagLen);
		memcpy(aucRID,paucParam,ucTagLen);
		paucParam+=ucTagLen;
		if(memcmp(paucParam,"\x9F\x22",2))
			return EMV_ERRTAG;
		paucParam+=2;
		ucTagLen=*paucParam;
		if(ucTagLen!=1) return EMV_ERRLENGTH;
		paucParam++;
		//memcpy(&pCAPKInfo->CAPKI,paucParam,ucTagLen);
		ucPKIndex=*paucParam;
		paucParam+=ucTagLen;

		if(memcmp(paucParam,"\xDF\x05",2))
			return EMV_ERRTAG;
		paucParam+=2;
		ucTagLen=*paucParam;
		if(ucTagLen!=8) return EMV_ERRLENGTH;
		paucParam++;
		//memcpy(pCAPKInfo->ExpireDate,paucParam,ucTagLen);
		memcpy(aucExpireDate,paucParam,ucTagLen);
		paucParam+=ucTagLen;
/*
		for(ucI=0;ucI<ucTermCAPKNum;ucI++)
		{
			if(!memcmp(aucRID,TermCAPK[ucI].RID,sizeof(TermCAPK[ucI].RID))&&ucPKIndex==TermCAPK[ucI].CAPKI)
				break;
		}
*/
//		if((ucI<ucTermCAPKNum&&memcmp(aucExpireDate,TermCAPK[ucI].ExpireDate,sizeof(aucExpireDate)))
//			||ucI>=ucTermCAPKNum)
		{
			memcpy(pCAPKInfo->RID,aucRID,sizeof(aucRID));
			pCAPKInfo->CAPKI=ucPKIndex;
			memcpy(pCAPKInfo->ExpireDate,aucExpireDate,sizeof(aucExpireDate));
			ucCAPKNum++;
		}
		if(ucCAPKNum>=EMVRIDMAXNUM) ucResult=EMV_DATAOVERFLOW;
	}

#ifdef EMVPARAM_DEBUG
	{
		unsigned char ucI,ucJ,*pch;

		util_Printf("\nucRIDNum %d\n",ucCAPKNum);
		for(ucI=0;ucI<ucCAPKNum;ucI++)
		{
			pch=(unsigned char*)&CAPKInfo[ucI];
			for(ucJ=0;ucJ<sizeof(CAPKINFO);ucJ++)
			{
				util_Printf("%02X ",*(pch+ucJ));
			}
			util_Printf("\n");
		}
	}
#endif

	return SUCCESS;
}

unsigned char EMVONLINE_AddAID(unsigned char* paucParam,unsigned short uiLen)
{

	unsigned char ucAIDLen;
	unsigned char* pucParamEnd;

	pucParamEnd=paucParam+uiLen;

	while(paucParam<pucParamEnd&&SupportAIDNum<EMVMAXAPPNUM)
	{
		if(memcmp(paucParam,"\x9F\x06",2))
			return EMV_ERRTAG;
		paucParam+=2;

		ucAIDLen=*paucParam;
		if(ucAIDLen>EMVAIDLEN) return EMV_ERRLENGTH;

		paucParam++;
		memcpy(&SupportAID[SupportAIDNum].AID,paucParam,ucAIDLen);
		SupportAID[SupportAIDNum].ucAIDLen=ucAIDLen;

		paucParam+=ucAIDLen;
		SupportAIDNum++;
	}
	return SUCCESS;
}


/******************** Terminal Support App FUNC****************
typedef struct {
	BYTE ASI;  //Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
	BYTE AIDLen;
	BYTE AID[16];//5-16
	BYTE bLocalName;//If display app list using local language considerless of info in card.0-use card info;1-use local language.
	BYTE AppLocalNameLen;
	BYTE AppLocalName[16];
	//ADD BY YJ 06.01.23
	unsigned char aucAppVer[2];
	unsigned char aucTACDefault[5];
	unsigned char aucTACOnline[5];
	unsigned char aucTACDenial[5];
	unsigned long ulFloorLimit;
	unsigned long ulThreshold;
	unsigned char ucMaxTargetPercent;
	unsigned char ucTargetPercent;
	unsigned char ucDDOLLen;
	unsigned char aucDDOL[252];
	unsigned char ucOnlinePINCapab;

}TERMAPP;

****************************************************************/
unsigned char TERMAPP_Insert(TERMAPP TermApp,EMVAIDPARAM AIDParamNew)
{
	unsigned char ucI;
	TERMAPP * pTermApp;
	EMVAIDPARAM* pAIDParam;


	if(ucTermAIDNum>=MAXAPPNUM)
		return EMVERR_APPNUMOVERFLOW;
	pTermApp=AppListTerm;
	for(ucI=0;ucI<ucTermAIDNum;ucI++,pTermApp++)
	{

		if(!memcmp(TermApp.AID,pTermApp->AID,TermApp.AIDLen)&&(TermApp.ASI==pTermApp->ASI))
			break;
	}
	if(ucI<ucTermAIDNum) return EMVERR_TERMAPPEXISTED;

	pTermApp=&AppListTerm[ucTermAIDNum];
	memcpy(pTermApp,&TermApp,sizeof(TERMAPP));

	pAIDParam=&AIDParam[ucTermAIDNum];
	memcpy(pAIDParam,&AIDParamNew,sizeof(EMVAIDPARAM));
	ucTermAIDNum++;

	return SUCCESS;

}


unsigned char EMVONLINE_AIDParamProcess(unsigned char ucProcessFlag,unsigned char* paucParam,unsigned short uiLen)
{
	unsigned char ucResult,aucBuff[255];
	unsigned short uiBuffLen;
	TERMAPP  TermApp;
	EMVAIDPARAM AIDParam_Buf;

	ucResult=SUCCESS;

	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(unsigned char *)"EMV参数下载中...!");

	EMVINTERFACE_INIT(&ICTransInfo,EMVParamTagAttrInfo,sizeof(EMVParamTagAttrInfo)/sizeof(EMVTAGATTR));
	memset(&TermApp,0x00,sizeof(TERMAPP));
	memcpy(ICTransInfo.aucTransInfo,paucParam,uiLen);
	ucResult=EMVINTERFACE_Unpack(&ICTransInfo);
	util_Printf("\nAID----EMVINTERFACE_Unpack()    ucResult = %02X \n",ucResult);
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\x9F\x06",aucBuff,&uiBuffLen);
		if(!ucResult&&uiBuffLen<=EMVAIDLEN)
		{
			memcpy(TermApp.AID,aucBuff,uiBuffLen);
			TermApp.AIDLen=uiBuffLen;
		}
		else
		{
			ucResult=EMV_ERRDATALEN;
		}
		if(ucResult) util_Printf("AID------9F 06-----ucResult = %02x\n",ucResult);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x01",aucBuff,&uiBuffLen);
		if(!ucResult&&uiBuffLen==sizeof(unsigned char ))
		{
			if(aucBuff[0])
				TermApp.ASI=EXACT_MATCH;
			else
				TermApp.ASI=PARTIAL_MATCH;
		}
		else
		{
			ucResult=EMV_ERRDATALEN;
		}
		if(ucResult) util_Printf("AID------ DF 01-----ucResult = %02x\n",ucResult);
	}


	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\x9F\x08",aucBuff,&uiBuffLen);
		if(!ucResult&&uiBuffLen==sizeof(AIDParam_Buf.aucAppVer))
		{
			memcpy(&AIDParam_Buf.aucAppVer,aucBuff,uiBuffLen);
		}
		else
		{
			ucResult=EMV_ERRDATALEN;

		}
		if(ucResult) util_Printf("AID------9F 08-----ucResult = %02x\n",ucResult);

	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x11",aucBuff,&uiBuffLen);
		if(!ucResult)
		{	if(uiBuffLen==sizeof(AIDParam_Buf.aucTACDefault))
				memcpy(AIDParam_Buf.aucTACDefault,aucBuff,uiBuffLen);
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 11-----ucResult = %02x\n",ucResult);

	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x12",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen==sizeof(AIDParam_Buf.aucTACOnline))
				memcpy(AIDParam_Buf.aucTACOnline,aucBuff,uiBuffLen);
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 12-----ucResult = %02x\n",ucResult);

	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x13",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen==sizeof(AIDParam_Buf.aucTACDenial))
				memcpy(AIDParam_Buf.aucTACDenial,aucBuff,uiBuffLen);
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 13-----ucResult = %02x\n",ucResult);

	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\x9F\x1B",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen==sizeof(AIDParam_Buf.ulFloorLimit))
				AIDParam_Buf.ulFloorLimit=tab_long(aucBuff,uiBuffLen);
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 1B-----ucResult = %02x\n",ucResult);

	}


	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x15",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen==sizeof(AIDParam_Buf.ulThreshold))
				AIDParam_Buf.ulThreshold=tab_long(aucBuff,uiBuffLen);
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 15-----ucResult = %02x\n",ucResult);

	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x16",aucBuff,&uiBuffLen);
		if(!ucResult&&uiBuffLen==sizeof(AIDParam_Buf.ucMaxTargetPercent))
		{
			AIDParam_Buf.ucMaxTargetPercent=(UCHAR)bcd_long(aucBuff,2);
			util_Printf("ucTargetPercent=%02x",AIDParam_Buf.ucMaxTargetPercent);
			//memcpy(&AIDParam_Buf.ucMaxTargetPercent,aucBuff,uiBuffLen);
		}
		else
		{
			ucResult=EMV_ERRDATALEN;
		}
		if(ucResult) util_Printf("AID------DF 16-----ucResult = %02x\n",ucResult);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x17",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen==sizeof(AIDParam_Buf.ucTargetPercent))
			{
				AIDParam_Buf.ucTargetPercent=(UCHAR)bcd_long(aucBuff,2);
				util_Printf("ucTargetPercent=%02x",AIDParam_Buf.ucTargetPercent);
				//memcpy(&AIDParam_Buf.ucTargetPercent,aucBuff,uiBuffLen);
			}
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 17-----ucResult = %02x\n",ucResult);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x14",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen<=sizeof(AIDParam_Buf.aucDDOL))
			{
				memcpy(AIDParam_Buf.aucDDOL,aucBuff,uiBuffLen);

				AIDParam_Buf.ucDDOLLen=uiBuffLen;
			}
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 14-----ucResult = %02x\n",ucResult);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x18",aucBuff,&uiBuffLen);
		if(!ucResult)
		{
			if(uiBuffLen<=sizeof(AIDParam_Buf.ucOnlinePINCapab))
			{
				memcpy(&AIDParam_Buf.ucOnlinePINCapab,aucBuff,uiBuffLen);
			}
			else
			{
				ucResult=EMV_ERRDATALEN;
			}
		}
		if(ucResult) util_Printf("AID------DF 18-----ucResult = %02x\n",ucResult);
	}
	//////////////////////////2009-9-6 2:40cbf///////////////////////
	////终端电子现金(EC)交易限额(当授权金额高于此限制时，该交易不为电子现金交易)
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		AIDParam_Buf.ulTelECFloorLimit=0;
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\x9F\x7B",aucBuff,&uiBuffLen); //EC终端最低限额
		util_Printf("\nGetTag-----9F7B-----==%d,%d",ucResult,uiBuffLen);
		if(!ucResult)
		{
			//sizeof(AIDParam_Buf.ulTelECFloorLimit))用此判断有错 Long类型为4个字节
			util_Printf("\nuiBuffLen=%d",uiBuffLen);
			if((uiBuffLen>=1)&&(uiBuffLen<=6))
				AIDParam_Buf.ulTelECFloorLimit=bcd_long(aucBuff,uiBuffLen*2);
		}
		else
		{
			AIDParam_Buf.ulTelECFloorLimit=0;
			ucResult =SUCCESS;
		}
		util_Printf("\nAIDParam_Buf.ulTelECFloorLimit=%d",AIDParam_Buf.ulTelECFloorLimit);
	}

	////非接终端脱机最低限额 (指示终端中的非接触最低限额)
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		AIDParam_Buf.ulUnTouchFloorLimit=0;
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x19",aucBuff,&uiBuffLen);
		util_Printf("\nGetTag-----DF19-----==%d,%d",ucResult,uiBuffLen);
		if(!ucResult)
		{
			util_Printf("\nuiBuffLen=%d",uiBuffLen);
			if((uiBuffLen>=1)&&(uiBuffLen<=6))
				AIDParam_Buf.ulUnTouchFloorLimit=bcd_long(aucBuff,uiBuffLen*2);
		}
		else
		{
			AIDParam_Buf.ulUnTouchFloorLimit=0;
			ucResult =SUCCESS;
		}
		util_Printf("\nAIDParam_Buf.ulUnTouchFloorLimit=%d",AIDParam_Buf.ulUnTouchFloorLimit);
	}

	////非接终端交易限额(如果非接触交易的数值大于或等于此数值，则交易终止允许在其它界面尝试此交易)
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		AIDParam_Buf.ulUnTouchTransLimit=0;
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x20",aucBuff,&uiBuffLen);
		util_Printf("\nGetTag-----DF20-----==%d,%d",ucResult,uiBuffLen);
		if(!ucResult)
		{
			util_Printf("\nuiBuffLen=%d",uiBuffLen);
			if((uiBuffLen>=1)&&(uiBuffLen<=6))
				AIDParam_Buf.ulUnTouchTransLimit=bcd_long(aucBuff,uiBuffLen*2);
		}
		else
		{
			AIDParam_Buf.ulUnTouchTransLimit=0;
			ucResult =SUCCESS;
		}
		util_Printf("\nAIDParam_Buf.ulUnTouchTransLimit=%d",AIDParam_Buf.ulUnTouchTransLimit);
	}

	////终端执行CVM限额(如果非接触交易超过此值，终端要求一个持卡人验证方法（CVM） 联机 PIN 和签名是本部分定义的持卡人验证方法(CVM))
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiBuffLen=sizeof(aucBuff);
		AIDParam_Buf.ulTelCVMLimit=0;
		ucResult=EMVINTERFACE_GetTagValue(&ICTransInfo,(unsigned char*)"\xDF\x21",aucBuff,&uiBuffLen);
		util_Printf("\nGetTag-----DF21-----==%d,%d",ucResult,uiBuffLen);
		if(!ucResult)
		{
			util_Printf("\nuiBuffLen=%d",uiBuffLen);
			if((uiBuffLen>=1)&&(uiBuffLen<=6))
				AIDParam_Buf.ulTelCVMLimit=bcd_long(aucBuff,uiBuffLen*2);
		}
		else
		{
			AIDParam_Buf.ulTelCVMLimit=0;
			ucResult =SUCCESS;
		}
		util_Printf("\nAIDParam_Buf.ulTelCVMLimit=%d,ucResult=%d\n",AIDParam_Buf.ulTelCVMLimit,ucResult);
	}
	////END///////////////////////////////////////////
	if(!ucResult)
		ucResult=TERMAPP_Insert(TermApp,AIDParam_Buf);

	if(!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(unsigned char *)"EMV参数成功!");
	}else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(unsigned char *)"EMV参数失败!");
		util_Printf("AID------11-----ucResult = %02x\n",ucResult);
	}
	Os__xdelay(30);

	return ucResult;

}

BYTE VerifyCAPK(CAPK* capk)
{
	BYTE checkSum[20],checkData[512];
	ushort index;

	memset(checkData,0,sizeof(checkData));

	index=0;
	memcpy(checkData,capk->aucRID,5);
	checkData[5]=capk->ucCAPKI;
	index=6;

	memcpy((BYTE*)&checkData[index],capk->aucModul,capk->ucModulLen);
	index+=capk->ucModulLen;
	memcpy((BYTE*)&checkData[index],capk->aucExponent,capk->ucExponentLen);
	index+=capk->ucExponentLen;

	SHA1_Compute(checkData,index,checkSum);


	{
		unsigned char ucI;
		unsigned short uiI;
		util_Printf("\n\n\nRID:");
		for(ucI=0;ucI<5;ucI++)
			util_Printf("%02x",capk->aucRID[ucI]);
		util_Printf("\nIndex:%02d",capk->ucCAPKI);
		util_Printf("\nModul:\n");
		for(uiI=0;uiI<capk->ucModulLen;uiI++)
			util_Printf("%02x",capk->aucModul[uiI]);
		util_Printf("\nHash:");
		for(ucI=0;ucI<20;ucI++)
			util_Printf("%02x",checkSum[ucI]);

	}
	if(memcmp(capk->aucCheckSum,checkSum,20))
	{
		return ERR_CAPKCHECK;
	}
	else
	{
		return OK;
	}

}
unsigned char VerifyReaderCAPK(unsigned char ucFlag)
{
	unsigned char ucResult=SUCCESS;

	if(DataSave0.ChangeParamData.bReaderSupport)
	{
		if(ucFlag)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"是否让非接读卡器");
			//Os__GB2312_display(1,0,(uchar *)"与POS终端同步EMV");
			//Os__GB2312_display(2,0,(uchar *)"参数和公钥信息?");
        	    	//Os__GB2312_display(3,0,(unsigned char *)"确认/取消？");
        	    	if(MSG_WaitKey(30)!=KEY_ENTER)
        	    		return ERR_CANCEL;
    	       }
    	       //Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"读卡器正在同步信");
		//Os__GB2312_display(2,0,(uchar *)"   息,请稍候! ");
		ucResult = SetHBReaderAIDInfo();

	 	if(!ucResult)
		{
			ucResult = UTIL_ManualSaveReaderCAPKFile();
			util_Printf("\n保存读卡器公钥信息UTIL_SaveReaderCAPKFile--ucResult=%02x\n",ucResult);
		}
		if(ucFlag)
		{
			//Os__clr_display(255);
			if(ucResult ==SUCCESS)
			{
				//Os__GB2312_display(1,0,(uchar *)"读卡器EMV信息与");
				//Os__GB2312_display(2,0,(uchar *)"终端匹配成功");
			}
			else
			{
				//Os__GB2312_display(1,0,(uchar *)"配置读卡器EMV参");
				//Os__GB2312_display(2,0,(uchar *)"数信息失败");
			}
			Os__xget_key();
		}
	}
	else
	{
		if(ucFlag == 0)
			return ucResult;
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"不支持非接卡交易");
		Os__xget_key();
	}
	return ucResult;
}
unsigned char SetHBReaderAIDInfo()
{
		unsigned char ucResult=SUCCESS ,ucI,ucJ,ucLen,ucSendLen,ucTotal=0;
		unsigned char aucBuf[500],aucSendBuf[500];
		memset(TermUntouchAID,0x00,sizeof(INTERFACESUPPORTAPP)*16);
		util_Printf("INTERFACE_Set_SupportAPToReader()---ucTermAIDNum =%02x\n",ucTermAIDNum);
		for(ucI=0,ucJ=0;!ucResult && ucI<ucTermAIDNum;ucI++)
		{
			if(( memcmp(TermAID[ucI].aucAID,"\xA0\x00\x00\x03\x33",5)==0))
			{
				 TermUntouchAID[ucJ].ucASI=TermAID[ucI].ucASI;
				 TermUntouchAID[ucJ].ucAIDLen=TermAID[ucI].ucAIDLen;
				 memcpy(TermUntouchAID[ucJ].aucAID,TermAID[ucI].aucAID,TermAID[ucI].ucAIDLen);
				 ucJ++;
				 util_Printf("\nAppListTerm[%d].ASI=%d",ucJ,TermAID[ucI].ucASI);
				 util_Printf("\nAppListTerm[%d].AIDLen=%d",ucJ,TermAID[ucI].ucAIDLen);
			}
		}
		util_Printf("\nucJ=abc=%d\n",ucJ);
		ucResult = INTERFACE_Set_SupportAPToReader(TermUntouchAID,ucJ);
		util_Printf("INTERFACE_Set_SupportAPToReader-WangAn Print--ucResult=%02x\n",ucResult);

		//与读卡器匹配非接卡参数(非接终端脱机最低限额、非接终端交易限额、终端执行CVM限额)
		//2009-9-6 17:17 交行规则非接终端交易限额会一样,另外2项可能VISA和PBOC不一样
		memset(aucBuf,0,sizeof(aucBuf));
		ucLen=0;
		if(ucResult) return ucResult;
		for(ucI=0;!ucResult && ucI<ucTermAIDNum;ucI++)
		{
	  		if(/*(memcmp(TermAID[ucI].aucAID,"\xA0\x00\x00\x00\x03",5)==0)  //非接只支持VISA和QPBOC
			||*/( memcmp(TermAID[ucI].aucAID,"\xA0\x00\x00\x03\x33",5)==0))
			{
				if((AIDParam[ucI].ulUnTouchFloorLimit ==0)&&(AIDParam[ucI].ulTelCVMLimit==0))
					continue;

				if(AIDParam[ucI].ulUnTouchTransLimit>DataSave0.ChangeParamData.ulUnTouchTransLimit)/*默认5000000*/
				{
					Os__saved_copy(	(unsigned char *)&AIDParam[ucI].ulUnTouchTransLimit,
						(unsigned char *)&DataSave0.ChangeParamData.ulUnTouchTransLimit,sizeof(unsigned long));
					ucResult = XDATA_Write_Vaild_File(DataSaveChange);
	 				if(ucResult != SUCCESS)
						return ucResult;
				}
				aucBuf[ucLen]= TermAID[ucI].ucAIDLen;
				ucLen++;
				memcpy(&aucBuf[ucLen],TermAID[ucI].aucAID,TermAID[ucI].ucAIDLen);
				ucLen+=TermAID[ucI].ucAIDLen;
				memcpy(&aucBuf[ucLen],"\x1B\xDF\x02\x06",4);
				ucLen+=4;
				long_bcd(&aucBuf[ucLen],6,&AIDParam[ucI].ulUnTouchFloorLimit);
				ucLen+=6;
				memcpy(&aucBuf[ucLen],"\xDF\x01\x06",3);
				ucLen+=3;
				long_bcd(&aucBuf[ucLen],6,&AIDParam[ucI].ulTelCVMLimit);
				ucLen+=6;
				memcpy(&aucBuf[ucLen],"\xDF\x00\x06",3);
				ucLen+=3;
				long_bcd(&aucBuf[ucLen],6,&AIDParam[ucI].ulUnTouchTransLimit);
				ucLen+=6;

				ucTotal++;
			}
		}
		util_Printf("\nDataSave0.ConstantParamData.ulUnTouchTransLimit=%0d\n",
					DataSave0.ChangeParamData.ulUnTouchTransLimit);
		/**/
	 	if(!ucResult)
	 	{
	 		if(ucTotal>=1)
	 		{
	 			ucSendLen=0;
	 			memset(aucSendBuf,0,sizeof(aucSendBuf));
				memcpy(aucSendBuf,"\xDF\xCD",2);

	 			if(ucLen<=127)
	 			{
	 				CONV_IntHex(&aucSendBuf[2],1,&ucLen);
	 				ucSendLen=3;
	 			}
	 			else
	 			{
	 				aucSendBuf[2]=0x81;
	 				CONV_IntHex(&aucSendBuf[3],1,&ucLen);
	 				ucSendLen=4;
	 			}
	 			memcpy(&aucSendBuf[ucSendLen],aucBuf,ucLen);
	 			ucSendLen=ucSendLen+ucLen;
#if 0
	        	util_Printf("\nReader Tag DFCD******************************* ");
	        	util_Printf("\nucSendLen=%0d\n",ucSendLen);
	        	memset(aucBuf,0,sizeof(aucBuf));
	        	memcpy(aucBuf,aucSendBuf,ucSendLen);
				for(ucI=0;ucI<ucSendLen;ucI++)
				{
					if((ucI==0)||(ucI%20==0))
						util_Printf("\n");
					util_Printf("%02x ",aucBuf[ucI]);
				}
	        	//util_Printf("\n|aucDRVOut: %s",aucDRVOut);
	        	util_Printf("\n END******************************************* \n");
#endif
				ucResult= INTERFACE_ConfigParamTagsToReader(aucSendBuf,ucSendLen);
				util_Printf("\nINTERFACE_ConfigParamTagsToReader--ucResult=%02x\n",ucResult);
			}
			util_Printf("\nEndSetAID=abc==%d",ucResult);
			if(ucResult)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"与reader同步AID");
				//Os__GB2312_display(1,0,(uchar *)"失败");
				//Os__GB2312_display(2,0,(uchar *)"请手动操作");
				UTIL_WaitGetKey(2);
				ucResult =SUCCESS;
			}
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"与reader同步AID");
				//Os__GB2312_display(1,0,(uchar *)"成功");
				UTIL_WaitGetKey(2);
				ucResult =SUCCESS;
			}
		}
	return SUCCESS;
}

#if SANDREADER
void SetSandReaderAIDInfo(void)
{
	unsigned char ucI;
	unsigned char ucJ;
       util_Printf("\nSetSandReaderAIDInfo =112233=%d",ucTermAIDNum);
	for(ucJ=0,ucI=0;ucI<ucTermAIDNum;ucI++)
	{
	       if(( memcmp(TermAID[ucI].aucAID,"\xA0\x00\x00\x03\x33",5)==0))
	        {
                    util_Printf("\nTermAID[ucI].aucAID=%d",ucJ);
                    memcpy(&QTemAID[ucJ],&TermAID[ucI],sizeof(TERMAPP));
                    QTemAID[ucJ].QReaderParam.bRCFLExist =TRUE;
                    QTemAID[ucJ].QReaderParam.ulRCFLimit =AIDParam[ucI].ulUnTouchFloorLimit;
                    QTemAID[ucJ].QReaderParam.bRCTLExist =TRUE;
                    QTemAID[ucJ].QReaderParam.ulRCTLimit =AIDParam[ucI].ulUnTouchTransLimit;
                    QTemAID[ucJ].QReaderParam.bRCVReqLExist =TRUE;
                    QTemAID[ucJ].QReaderParam.ulRCVMReqLimit =AIDParam[ucI].ulTelCVMLimit;
                    QTemAID[ucJ].QReaderParam.bStatusCheck =false;
                    memcpy(QTemAID[ucJ].QReaderParam.aucTermTransQ,(UCHAR *)"\xA6\x00\x00\x00",4);
                    ucJ++;
        	}
	}
	 util_Printf("\nSetSandReaderAIDInfo =445566=%d",ucJ);
	return;
}
void QAID_Dump(void)
{
	unsigned char ucI,ucJ;
	for(ucI=0;ucI<ucTermAIDNum;ucI++)
	{
		util_Printf("\naucAID:");
		for(ucJ=0;ucJ<QMAXAIDLEN;ucJ++)
			util_Printf("%02x ",QTemAID[ucI].aucAID[ucJ]);

		util_Printf("\naucAID2:");
		for(ucJ=0;ucJ<QMAXAIDLEN;ucJ++)
			util_Printf("%02x ",AppListTerm[ucI].AID[ucJ]);
		util_Printf("\naucAID3:");
		for(ucJ=0;ucJ<QMAXAIDLEN;ucJ++)
			util_Printf("%02x ",TermAID[ucI].aucAID[ucJ]);
		util_Printf("\nQTemAID[ucI].QReaderParam.bRCFLExist=%02x",QTemAID[ucI].QReaderParam.bRCFLExist);
		util_Printf("\nQTemAID[ucI].QReaderParam.bRCFLExist=%d",QTemAID[ucI].QReaderParam.ulRCFLimit);

		util_Printf("\nQTemAID[ucI].QReaderParam.bRCTLExist=%02x",QTemAID[ucI].QReaderParam.bRCTLExist);
		util_Printf("\nQTemAID[ucI].QReaderParam.ulRCTLimit=%d",QTemAID[ucI].QReaderParam.ulRCTLimit);

		util_Printf("\nQTemAID[ucI].QReaderParam.bRCVReqLExist=%02x",QTemAID[ucI].QReaderParam.bRCVReqLExist);
		util_Printf("\nQTemAID[ucI].QReaderParam.ulTelCVMLimit=%d",QTemAID[ucI].QReaderParam.ulRCVMReqLimit);

		util_Printf("\nQTemAID[ucI].QReaderParam.bStatusCheck=%02x",QTemAID[ucI].QReaderParam.bStatusCheck);
		util_Printf("\naucTermTransQ:");
		for(ucJ=0;ucJ<4;ucJ++)
			util_Printf("%02x ",QTemAID[ucI].QReaderParam.aucTermTransQ[ucJ]);
	}
	util_Printf("\naucTermTransQ:%02x",QEMVConfig.ucCAPKNums);
/*	typedef		struct
	{
		UCHAR		aucRID[QRIDDATALEN];
		UCHAR		ucCAPKI;
		UCHAR 		ucHashInd;
		UCHAR 		ucArithInd;
		UCHAR		ucIndex;
		UCHAR 		ucModulLen;
		UCHAR 		aucModul[QMAXMODULLEN];
		UCHAR 		ucExponentLen;
		UCHAR 		aucExponent[QMAXEXPONENTLEN];
	//	UCHAR 		aucExpireDate[8];
		UCHAR 		aucCheckSum[QHASHDATALEN];
	}QCAPK,*PQCAPK;
*/
}
#endif
