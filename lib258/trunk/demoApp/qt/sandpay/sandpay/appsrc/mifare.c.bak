#include <include.h>
#include <global.h>
#include <xdata.h>

#include <osicc.h>
#include <smart.h>
#include <osmifare.h>
#include <mifare.h>

MIFIRE_ORDER 			  MIFARE_Order;
MIFARE_ANSWER 			*MIFARE_pAnswer;
MIFARE_ORDER_IN_TYPE 	  MIFARE_Type;

unsigned char MIFARE_ISO( 	unsigned char* pucIn,unsigned char* pucOut,
            							unsigned short* puiLengthOut,unsigned char ucOrderType )
{
	unsigned char ucI,ucJ;
	DICARD_ORDER_TYPE *pDiCardType;
	
	/* Order to execute */
	ucI = 0;
	ucJ = 0;
	MIFARE_Order.ptout = pucOut;
	MIFARE_Order.pt_order_in = &MIFARE_Type;
	pDiCardType = &MIFARE_Type.dicard_order;
    	MIFARE_Order.order = OSMIFARE_DICARD_ORDER;
		
    switch ( ucOrderType )
    {
	    case MIFARE_TYPE1:
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = 0;
	        break;
	    case MIFARE_TYPE2:
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        break;
	    case MIFARE_TYPE3:
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];	/* Length of data transmitted*/
	        memcpy( &pDiCardType->aucInData[ucI],pucIn+ucJ,*(pucIn+4) ); /* Data transmitted          */
	        ucI += *(pucIn+4);
	        pDiCardType->aucInData[ucI++] = 0;
	        break;
	    case MIFARE_TYPE4:
	    	pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];	/* Length of data transmitted*/
	        memcpy( &pDiCardType->aucInData[ucI],pucIn+ucJ,*(pucIn+4) ); /* Data transmitted          */
	        ucI += *(pucIn+4);
	        ucJ += *(pucIn+4);
	        pDiCardType->aucInData[ucI++] = pucIn[ucJ++];
	        break;
	    default:
	        return (MIFARE_CARD_ERROR);
    }
    pDiCardType->uiInDataLen = ucI;
	/* Send Command */
	MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );

	/* Answer Command */
	/******************/
	if ( MIFARE_pAnswer->drv_status != 0 )
    {
    	return MIFARE_DRIVER_ERROR;
    }
	if ( MIFARE_pAnswer->card_status==OSMIFARE_ERR_NOTAG )
	{
//		MifareDebitFlag=1;
		return MIFARE_ABSENT;
	}
	if ( MIFARE_pAnswer->Len > *puiLengthOut )
	{
		return MIFARE_OWERFLOW;
	}
	*puiLengthOut = MIFARE_pAnswer->Len;
	return MIFARE_OK;
}

unsigned char MIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,unsigned char *pucCardIdLen,unsigned char *pucCardClass )
{
	unsigned char aucRevData[16];
	
	MIFARE_Order.order = OSMIFARE_ANTI_ORDER;
	MIFARE_Order.ptout = aucRevData;
	MIFARE_Order.pt_order_in = &MIFARE_Type;
	MIFARE_Type.base_order.ucInDataLen = 1;
	MIFARE_Type.base_order.aucInData[0] = ucAntiEn;
	MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
	if( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
	{
	    return (MIFARE_pAnswer->drv_status);
	}
	if( MIFARE_pAnswer->Len )
	{
		*pucCardClass = aucRevData[0];
		memcpy( pucCardId,&aucRevData[1],MIFARE_pAnswer->Len-1 );
		*pucCardIdLen = MIFARE_pAnswer->Len-1;		
	}else
		*pucCardIdLen = 0;		
	return( MIFARE_pAnswer->card_status );
}

unsigned char MIFARE_Select( unsigned char *pucCardId,unsigned char ucCardIdLen,unsigned char *pucCardClass )
{
	unsigned char aucRevData[16];
	
	MIFARE_Order.order = OSMIFARE_SELECT_ORDER;
	MIFARE_Order.ptout = aucRevData;
	MIFARE_Order.pt_order_in = &MIFARE_Type;
	MIFARE_Type.base_order.ucInDataLen = ucCardIdLen;
	memcpy( MIFARE_Type.base_order.aucInData,pucCardId,ucCardIdLen );
	MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
	if( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
	{
	    return (MIFARE_pAnswer->drv_status);
	}
	*pucCardClass = aucRevData[0];
	return( MIFARE_pAnswer->card_status );
}

unsigned char MIFARE_GetAts( unsigned char *pucAts,unsigned char *pucAtsLen )
{
	unsigned char aucRevData[OSMIFARE_ATS_MAX_LEN];
	
	MIFARE_Order.order = OSMIFARE_RATS_ORDER;
	MIFARE_Order.ptout = aucRevData;
	MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
	if( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
	{
	    return (MIFARE_pAnswer->drv_status);
	}
	if( MIFARE_pAnswer->Len )
		memcpy( pucAts,aucRevData,MIFARE_pAnswer->Len );
	*pucAtsLen = MIFARE_pAnswer->Len;		
	return( MIFARE_pAnswer->card_status );
}

unsigned char MIFARE_Halt( void )
{
	unsigned char aucRevData[16];
	
	MIFARE_Order.order = OSMIFARE_HALT_ORDER;
	MIFARE_Order.ptout = aucRevData;
	MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
	if( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
	{
	    return (MIFARE_pAnswer->drv_status);
	}
	return( MIFARE_pAnswer->card_status );
}

unsigned char MIFARE_CloseRF( void )
{
	unsigned char aucRevData[16];
	
	MIFARE_Order.order = OSMIFARE_CLOSERF_ORDER;
	MIFARE_Order.ptout = aucRevData;
	MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
	if( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
	{
	    return (MIFARE_pAnswer->drv_status);
	}
	return( MIFARE_pAnswer->card_status );
}


unsigned char MIFARE_GetCardInfo(void)
{
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));

	
	memcpy(&MifareCommandData.aucCommandIn[0],"\xA0\xA1\xA2\xA3\xA4\xA5",6);//Կ
	MifareCommandData.ucInLen = 6;
	MifareCommandData.ucOutLen = 255;
	CMDWK_ReadCard(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
						MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);
	Uart_Printf("\n 1. MIFARE_ucOutStatus:%02x",MIFARE_ucOutStatus);
	Uart_Printf("\n 2. MIFARE_FindErrCode:%02x\n",MIFARE_FindErrCode(MIFARE_ucOutStatus));
		
	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}

unsigned char MIFARE_GetCardDetailInfo(void)
{
	unsigned char	ucKeyArray;
	
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
	//memset(&WKMifCardInfo,0,sizeof(WKDETAILCARDINFO));
	
	memcpy(&MifareCommandData.aucCommandIn[0],RunData.aucKeyA,6);
	//memcpy(&G_WKMIFARE_aucWKCommandIn[6],RunData.aucKeyB,6);
	memcpy(&MifareCommandData.aucCommandIn[6],RunData.aucPurchaseKey,6);
	
	switch(NormalTransData.ucPtCode)
	{
		case 0x01:ucKeyArray=KEYARRAY_PURSECARD1;break;
		case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
		case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
		default:break;
	}
	MifareCommandData.aucCommandIn[12]=ucKeyArray;
	MifareCommandData.aucCommandIn[13]=KEYINDEX_PURSECARD1_LOADMACKEY;
	MifareCommandData.aucCommandIn[14]=ucKeyArray;
	MifareCommandData.aucCommandIn[15]=KEYINDEX_PURSECARD1_LOADMACKEY+1;
	
	MifareCommandData.ucInLen = 16;
	MifareCommandData.ucOutLen = 255;
	CMDWK_GetBalance(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
						MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);

	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}

unsigned char MIFARE_ReadCardDetail(void)
{
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));

	MifareCommandData.aucCommandIn[0] = NormalTransData.ucTransPoint;
	MifareCommandData.ucInLen = 1;
	MifareCommandData.ucOutLen = 255;
	CMDWK_Getlog(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
						MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);

	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}

unsigned char MIFARE_RemoveCard(void)
{
	unsigned char ucResult;
//	//Os__clr_display(1);
//	//Os__GB2312_display(1,0,(unsigned char*)"ȡ>>>");
	while(1)
	{
		memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
		ucResult = CMD_AntiRFcard(MifareCommandData.aucCommandOut,&MifareCommandData.ucOutLen);
		if(MifareCommandData.aucCommandOut[0] == 0x04 )
		{
			return(SUCCESS);
		}
	}
}

unsigned char MIFARE_ReadCardIssueInfo(void)
{
	unsigned char ucResult;
	
	ucResult = SUCCESS;
	
	if( ucResult == SUCCESS )
	{
		MIFARE_GetCardDetailInfo();
		if( MIFARE_ucOutStatus == 0x00 )
		{
			/*if( G_WKMIFARE_ucOutLen <= sizeof(WKDETAILCARDINFO) )
			{
				memcpy(&WKMifCardInfo,&G_WKMIFARE_aucWKCommandOut[1],G_WKMIFARE_ucOutLen-1);
			}
			else*/
			/*{
				memcpy(&WKMifCardInfo,&G_WKMIFARE_aucWKCommandOut[1],sizeof(WKDETAILCARDINFO));
			}
			
			NormalTransData.ucMemFlag=WKMifCardInfo.ucMemberFlag;;
			NormalTransData.ucHopassFlag=WKMifCardInfo.ucWanTongFlag;
			NormalTransData.ucTransPoint=WKMifCardInfo.ucCardPoint;
			memcpy(NormalTransData.aucSamTace,&WKMifCardInfo.aucCount,4);
			bcd_asc(NormalTransData.aucLoadTrace,WKMifCardInfo.aucDepositTrace,16);
			memcpy(NormalTransData.aucZoneCode,WKMifCardInfo.aucDeliverCode,3);
			memcpy(NormalTransData.aucExpiredDate,WKMifCardInfo.aucExpiredDate,4);
		  NormalTransData.ulPrevEDBalance = tab_long(WKMifCardInfo.aucBalance, 4);
			NormalTransData.ucCardStatus=WKMifCardInfo.ucCardStatus;*/
			
		}
		else
		{
			return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
		}
	}
	return(ucResult);
}

unsigned char MIFARE_FindErrCode(unsigned char ucStatus)
{
	unsigned char ucResult;
	switch(ucStatus)
	{
		case WK_MIFARE_CARDERR_BADPARA:
		case WK_MIFARE_CARDERR_NOCARD:
		case WK_MIFARE_CARDERR_DECFAIL:
		case WK_MIFARE_CARDERR_KEYI1:
		case WK_MIFARE_CARDERR_BADBACKUPEP:
		case WK_MIFARE_CARDERR_INVALIDCARD:
		case WK_MIFARE_CARDERR_DEPFAIL:
		case WK_MIFARE_CARDERR_BADTACPARAM:
		case WK_MIFARE_CARDERR_UNACCEPT:
		case WK_MIFARE_CARDERR_KEYI2:
		case WK_MIFARE_CARDERR_AUTHFAIL:
		case WK_MIFARE_CARDERR_CARDLOCK:
			RUNDATA_ucErrorExtCode = ucStatus;
			ucResult = ERR_WK_MIFARE;
			break;
		default:
			ucResult = ucStatus;
			break;
	}
	return(ucResult);
}

unsigned char MIFARE_WaitCard(void)
{
	unsigned char ucResult;
	
	////Os__clr_display(255);
	////Os__GB2312_display(0,0,(unsigned char*)"ſ>>>");
	//while(1)
	{
		memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
		ucResult = CMD_AntiRFcard(MifareCommandData.aucCommandOut,&MifareCommandData.ucOutLen);
		if( MifareCommandData.aucCommandOut[0] == 0x00 )
		{
			return(SUCCESS);
		}
		else
		{
			return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
		}
	}
}

unsigned char MIFARE_WaitCard1(void)
{
	unsigned char ucResult;
	
	////Os__clr_display(255);
	////Os__GB2312_display(0,0,(unsigned char*)"ſ>>>");
	//while(1)
	{
		memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
		ucResult = CMD_AntiRFcard(MifareCommandData.aucCommandOut,&MifareCommandData.ucOutLen);
		if( MifareCommandData.aucCommandOut[0] == 0x00 )
		{
			return(SUCCESS);
		}
		else
		{
			return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
		}
	}
}

unsigned char MIFARE_GetTAC(void)
{
	unsigned char ucResult;
	
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));

	MifareCommandData.aucCommandIn[0] = 0x01;
	long_tab(&MifareCommandData.aucCommandIn[1],4,&NormalTransData.ulAmount);
	MifareCommandData.aucCommandIn[5] = 0x21;
	asc_bcd(&MifareCommandData.aucCommandIn[6],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
	long_bcd(&MifareCommandData.aucCommandIn[10],4,&NormalTransData.ulTraceNumber);
	memcpy(&MifareCommandData.aucCommandIn[14],NormalTransData.aucDateTime,7);
	MifareCommandData.ucInLen = 21;
	MifareCommandData.ucOutLen = 255;
	
	ucResult = CMDWK_CalcTAC(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
							MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);

	return(ucResult);
}
unsigned char MIFARE_DecreaseBlance(void)
{
	unsigned char ucResult;
	unsigned long ulAmount;
	
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
	
	if(RunData.aucFunFlag[1])
		ulAmount=min(NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount);
	else
		ulAmount=min(NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance);

	memcpy(&MifareCommandData.aucCommandIn[0],NormalTransData.aucDateTime,7);
	long_tab(&MifareCommandData.aucCommandIn[7],4,&ulAmount);

	if((NormalTransData.aucUserCardNo[0]>>4)==0x09)
		memcpy(&MifareCommandData.aucCommandIn[11],&NormalTransData.aucPsamTerminal[2],4);
	else
		asc_bcd(&MifareCommandData.aucCommandIn[11],4,DataSave0.ConstantParamData.aucOnlytermcode,8);

	long_bcd(&MifareCommandData.aucCommandIn[15],4,&NormalTransData.ulTraceNumber);
	MifareCommandData.aucCommandIn[19]=0x21;
	if(NormalTransData.ucReturnflag)
		MifareCommandData.aucCommandIn[20]=0x03;
	else
		MifareCommandData.aucCommandIn[20]=0x01;

	MifareCommandData.ucInLen = 21;
	MifareCommandData.ucOutLen = 255;
	
	CMDWK_Debit(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
								MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);

	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}

unsigned char MIFARE_LoadBlance(void)
{
	unsigned char ucResult;
	unsigned long ulAmount;
	
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));

	memcpy(&MifareCommandData.aucCommandIn[0],NormalTransData.aucDateTime,7);
	long_tab(&MifareCommandData.aucCommandIn[7],4,&NormalTransData.ulAmount);
	
	if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
		memcpy(&MifareCommandData.aucCommandIn[11],&NormalTransData.aucPsamTerminal[2],4);
	else
		asc_bcd(&MifareCommandData.aucCommandIn[11],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
	long_bcd(&MifareCommandData.aucCommandIn[15],4,&NormalTransData.ulTraceNumber);
	MifareCommandData.aucCommandIn[19]=0x62;
	memcpy(&MifareCommandData.aucCommandIn[20],NormalTransData.aucExpiredDate,4);
	asc_bcd(&MifareCommandData.aucCommandIn[24],8,NormalTransData.aucLoadTrace,16);
	memcpy(&MifareCommandData.aucCommandIn[32],RunData.aucLoadKey,6);
	asc_bcd(&MifareCommandData.aucCommandIn[39],3,DataSave0.ChangeParamData.aucCashierNo,6);
	memcpy(&MifareCommandData.aucCommandIn[42],RunData.aucTotalLoad,4);
	memcpy(&MifareCommandData.aucCommandIn[46],RunData.aucLoadMac,4);
	memcpy(&MifareCommandData.aucCommandIn[50],RunData.aucWriteKey1,6);

	MifareCommandData.ucInLen = 56;
	MifareCommandData.ucOutLen = 255;
	
	CMDWK_Credit(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
								MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);

	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}
unsigned char MIFARE_ChangeStatus(void)
{
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
	
	MifareCommandData.aucCommandIn[0]=RunData.ucTagetStatus;
	MifareCommandData.aucCommandIn[1]=NormalTransData.ucMemFlag;
	MifareCommandData.aucCommandIn[2]=NormalTransData.ucHopassFlag;
	
	MifareCommandData.ucInLen = 3;
	MifareCommandData.ucOutLen = 255;
	CMDWK_RenewCardInfo(MifareCommandData.aucCommandIn,MifareCommandData.aucCommandOut,
						MifareCommandData.ucInLen,&MifareCommandData.ucOutLen);
	
	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}
#if 1
unsigned char MIFARE_GetVersion(void)
{
	memset(MifareCommandData.aucCommandIn,0,sizeof(MifareCommandData.aucCommandIn));
	memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
	
	MifareCommandData.ucOutLen = 255;
	CMDVikang_CheckAppVer(MifareCommandData.aucCommandOut,&MifareCommandData.ucOutLen);
	
	if(memcmp(&MifareCommandData.aucCommandOut[1],"WK",2))
		MIFARE_ucOutStatus=0x04;
	return(MIFARE_FindErrCode(MIFARE_ucOutStatus));
}
#endif

unsigned char MifareCard_Init( unsigned char *pucCardNo,unsigned char ucLen )
{
	unsigned char ucResult,ucCardClass;
	DRV_OUT *pxReuslt;

	MIFARE_CloseRF();
	MIFARE_Halt();

#ifdef GUI_PROJECT
	ucResult = Os__MIFARE_Request( OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_REQA );
	if( ucResult==OSMIFARE_ERR_SUCCESS )
	{
		ucResult = MIFARE_Anti( 1,pucCardNo,&ucLen,&ucCardClass );
	}else
		ucResult = 0x01;
#else
	pxReuslt = Os__MIFARE_Request( OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_REQA );
	if( pxReuslt->xxdata[0]==OSMIFARE_ERR_SUCCESS )
	{
		ucResult = MIFARE_Anti( 1,pucCardNo,&ucLen,&ucCardClass );
	}else
		ucResult = 0x01;
#endif	
	return ucResult;
}
