#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <oslib.h>


unsigned char Trans_Process(void)
{
     unsigned char ucResult = SUCCESS;

     if(DataSave0.ChangeParamData.ucCashierLogonFlag!=0x55)
	  ucResult =  ERR_CASH_NOTLOGON;

     G_NORMALTRANS_ucTransType = TRANS_PURCHASE;

     if(!ucResult)
     {
          ucResult = MAG_DispCardNo();
     }
     if(!ucResult)
     {
          ucResult = SAV_CheckTransMaxNb();
     }
     if(!ucResult)
     {
          ucResult = UTIL_InputTransAmount();
     }  
      if(!ucResult)
     {
          ucResult = UTIL_InputEncryptPIN();
     }  
       if(!ucResult)
     {
          ucResult = TRANS_Online();
     }  

     if(!ucResult)
     {
           ucResult = SAV_TransSave();
     }
     if(!ucResult)
     {
           ucResult = PRINT_Deal();
     }

     return ucResult;
}

unsigned char TRANS_Online(void)
{
       unsigned char ucResult,ucReFile;
	unsigned char aucBuf[100],aucBuf2[10];
	unsigned char aucTemp[15];
	unsigned char aucDispBuf[15];
	unsigned char date[7];
	unsigned short uiTagLen;
	short iLen;
	unsigned long  ulAmountTemp;
	unsigned int uiI,uiK;
	unsigned long AuthLen;
	unsigned short uiReversalLen;

	ISO8583_Clear();

	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = 0x00;
	aucBuf[1] = aucBuf[1] | (((G_NORMALTRANS_ucSourceAccType - 0x30) << 4 ) & 0xF0);
	ISO8583_SetBitHexValue(3,aucBuf,3);

	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&G_NORMALTRANS_ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);

	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&G_NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber();

	if(memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00",2))
	{
		ISO8583_SetBitHexValue(14,G_NORMALTRANS_ucExpiredDate,2);
	}

	memset(aucBuf,'0',sizeof(aucBuf));
	ISO8583_SetBitValue(25,aucBuf,2);

	if(G_EXTRATRANS_ucInputPINLen != 0)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,"12",2);
		ISO8583_SetBitValue(26,aucBuf,2);

		ISO8583_SetBitHexValue(52,G_EXTRATRANS_aucPINData,8);
		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0] = '2';
              aucBuf[1] = '0';
		memcpy(&aucBuf[2],"00000000000000",14);
		ISO8583_SetBitValue(53,aucBuf,16);
	}

	ISO8583_SetBitValue(41,G_RUNDATA_aucTerminalID,8);

	ISO8583_SetBitValue(42,G_RUNDATA_aucMerchantID,15);

	ISO8583_SetBitValue(49,(uchar *)"156",3);

	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,"22",2);
	long_asc(&aucBuf[2],6,&DataSave0.ChangeParamData.ulBatchNumber);
	memcpy(&aucBuf[8],"000",3);
	aucBuf[11]='5';
	aucBuf[12]='0';
	ISO8583_SetBitValue(60,aucBuf,strlen((char*)aucBuf));

	if( G_EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,G_EXTRATRANS_aucISO2,
    		G_EXTRATRANS_uiISO2Len);
	}
	if( G_EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,G_EXTRATRANS_aucISO3,
    		G_EXTRATRANS_uiISO3Len);
	}

	memcpy(aucBuf,"9210",4);
	ISO8583_SetBitValue(22,aucBuf,4);

	ISO8583_SetMsgID(200);

	ISO8583_SetBitValue(64,(uchar *)"0000000000000000",16);

	ucResult = COMMS_GenSendReceive();

	if( ucResult == SUCCESS )
	{
	       memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(2,aucBuf,&iLen,sizeof(aucBuf));

		if( ucResult == SUCCESS)
		{
			G_NORMALTRANS_ucSourceAccLen = (unsigned char )iLen;
			memset(G_NORMALTRANS_aucSourceAcc,0,sizeof(G_NORMALTRANS_aucSourceAcc));
			memcpy(G_NORMALTRANS_aucSourceAcc,aucBuf,G_NORMALTRANS_ucSourceAccLen);
		}
		
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(12,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult == SUCCESS )
		{
			asc_bcd(G_NORMALTRANS_aucTime,3,aucBuf,6);
		}

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(13,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult == SUCCESS )
		{
			asc_bcd(&G_NORMALTRANS_aucDate[2],2,aucBuf,4);
			Os__read_date(date);
			asc_bcd(&G_NORMALTRANS_aucDate[1],1,&date[4],2);
			G_NORMALTRANS_aucDate[0] = (G_NORMALTRANS_aucDate[1] > 0x90) ?  0x19 :  0x20 ;
		}

	      memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(32,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult == SUCCESS )
		{
			memcpy(G_NORMALTRANS_aucBankID,aucBuf,iLen);
		}

		memset(aucBuf,0,sizeof(aucBuf));
              ucResult = ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult == SUCCESS )
		{
			memcpy(G_NORMALTRANS_ucExpiredDate,aucBuf,4);
		}

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(37,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult == SUCCESS )
		{
			memcpy(G_NORMALTRANS_aucRefNum,aucBuf,TRANS_REFNUMLEN);
		}

		ucResult = ISO8583_GetBitValue(38,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult == SUCCESS)
		{
			memcpy(G_NORMALTRANS_aucAuthCode, aucBuf, TRANS_AUTHCODELEN);
			
		}
		
	}
	
	
	
	
      return ucResult;
}
