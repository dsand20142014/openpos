#include	<include.h>
#include	<Global.h>
#include 	<xdata.h>
#include 	<fsync_drvs.h>
#include <menu.h>


void UTIL_IncreaseTraceNumber(void)
{
	unsigned long ulTraceNumber;

	ulTraceNumber = DataSave0.ChangeParamData.ulTraceNumber;
    	ulTraceNumber ++;
	if( ulTraceNumber > 999999 )
	{
		ulTraceNumber = 1;
	}
	DataSave0.ChangeParamData.ulTraceNumber = ulTraceNumber;
	XDATA_Write_Vaild_File(DataSaveChange);
}

unsigned char UTIL_Single_StoreNewKey(void)
{
	unsigned char ucResult;
	unsigned char ucArrayIndex;
	unsigned char aucHexData[25];
	unsigned char aucDataBuf[9];
	unsigned char aucBuf[9];
	unsigned char ucKeyIndex;
	short iLen,usI;



	ucResult = ISO8583_GetBitValue(62,aucHexData,&iLen,sizeof(aucHexData));
	if( ucResult == SUCCESS)
	{
		if( iLen != 24 )
		{
			Uart_Printf("error len \r\n");
			return(ERR_ISO8583_INVALIDLEN);
		}
	}

	memset(aucDataBuf,0,sizeof(aucDataBuf));
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucDataBuf,aucHexData,8);
	memcpy(aucBuf,&aucHexData[12],8);


	ucKeyIndex = KEYINDEX_PURSECARD_MASTERKEY;

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;

	
	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
		ucArrayIndex,ucKeyIndex,
		KEYINDEX_GOLDENCARDSH_PINKEY,aucDataBuf);
	if(ucResult == SUCCESS)
	{
 		ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
				ucArrayIndex,ucKeyIndex,
				KEYINDEX_GOLDENCARDSH_MACKEY,aucBuf);
	}
	if(ucResult == SUCCESS)
	{
		memset(aucDataBuf,0,sizeof(aucDataBuf));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_PINKEY,
						aucDataBuf,aucDataBuf);
	}

	if(ucResult == SUCCESS)
	{
		memcpy(aucBuf,&aucHexData[8],4);
		if(memcmp(aucDataBuf,aucBuf,4) != 0)
		{
			ucResult = ERR_CHECKSECRET;
		}
	}
	if(ucResult == SUCCESS)
	{
		memset(aucDataBuf,0,sizeof(aucDataBuf));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_MACKEY,
						aucDataBuf,aucDataBuf);
	}

	if(ucResult == SUCCESS)
	{
		memcpy(aucBuf,&aucHexData[20],4);
		if(memcmp(aucDataBuf,aucBuf,4) != 0)
		{
			ucResult = ERR_CHECKSECRET;
		}
	}
	
	return(ucResult);
}


unsigned char UTIL_CalcGoldenCardSHMAC_Single(
				unsigned char *pucInData,
				unsigned short uiInLen,
				unsigned char *pucOutMAC)
{
	unsigned char ucResult,ucI;
	unsigned char aucHexData[9];
	unsigned char aucAscData[17];
	unsigned char *pucPtr;
	unsigned short uiI,uiJ;
	unsigned short uiLen;
	unsigned char  ucKeyIndex;

	
	pucPtr = pucInData;


	memset(aucHexData,0,sizeof(aucHexData));

	for(uiI=0;uiI<uiInLen;uiI += 8)
	{
		uiLen = min(8,pucInData+uiInLen-pucPtr);
		for(uiJ=0;uiJ<uiLen;uiJ++)
		{
			aucHexData[uiJ] ^= *(pucPtr+uiJ);
		}
		pucPtr += 8;
	}



	hex_asc(aucAscData,aucHexData,16);
	aucAscData[16] = 0;



	memset(aucHexData,0,sizeof(aucHexData));

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					KEYARRAY_GOLDENCARDSH,
					KEYINDEX_GOLDENCARDSH_MACKEY,
					aucAscData,
					aucHexData);

	if( ucResult != SUCCESS)
		return(ucResult);



	for(uiI=0;uiI<8;uiI++)
	{
		aucHexData[uiI] ^= aucAscData[8+uiI];
	}

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					KEYARRAY_GOLDENCARDSH,
					KEYINDEX_GOLDENCARDSH_MACKEY,
					aucHexData,
					aucHexData);


	if( ucResult != SUCCESS)
		return(ucResult);

	hex_asc(aucAscData,aucHexData,16);


	memcpy(pucOutMAC,aucAscData,8);
	return(SUCCESS);
}

void UTIL_ClearGlobalData(void)
{
	memset((unsigned char *)&NormalTransData,0,sizeof(NormalTransData));
	memset((unsigned char *)&RunData,0,sizeof(RunData));
	memset((unsigned char *)&ISO8583Data,0,sizeof(ISO8583Data));

	memset((unsigned char *)&DataSave0,0,sizeof(DATASAVEPAGE0));
	memset((unsigned char *)&DataSave1,0,sizeof(DATASAVEPAGE1));

	XDATA_Read_Vaild_File(DataSaveConstant);
	XDATA_Read_Vaild_File(DataSaveChange);

	G_NORMALTRANS_ulTraceNumber = DataSave0.ChangeParamData.ulTraceNumber;
	memcpy(G_RUNDATA_aucTerminalID,DataSave0.ConstantParamData.aucTerminalID,
		PARAM_TERMINALIDLEN);
	memcpy(G_RUNDATA_aucMerchantID,DataSave0.ConstantParamData.aucMerchantID,
		PARAM_MERCHANTIDLEN);
}




unsigned char UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
	const unsigned char aucKeyTab[][7]
	={
		{"0 *\0"},
		{"1QZ\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PRS\0"},
		{"8TUV\0"},
		{"9WXY\0"},
		{"0,.-\0"}
	};

	unsigned char	aucDisp[40];	
	unsigned char	aucInput[40];	
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	NEW_DRV_TYPE new_drv;
	DRV_OUT *pKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	unsigned char dbgbuf[900];
	int i;
	int num;
	memset(dbgbuf,0,sizeof(dbgbuf));
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*100;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
		Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}


	do
	{
	       Os__timer_start(&uiTimeout);
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > MAXLINECHARNUM)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',MAXLINECHARNUM);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-MAXLINECHARNUM],MAXLINECHARNUM);
				}
				aucDisp[ucCount] = '_';
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',ucCount);
				}
				else if(  (ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if(ucCount>ucMax)
						memcpy(&aucDisp[0],&aucInput[0],ucCount-1);
					else
						memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				aucDisp[ucCount] = '_';
			}
			Os__clr_display(ucLine);
			Os__display(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
		//pKey = Os__xget_key();
		memset(&new_drv,0,sizeof(new_drv));
		Os_Wait_Event(KEY_DRV, &new_drv, 150000);
		num = sizeof(new_drv);
		memcpy(dbgbuf,&new_drv,num);
		//Uart_Printf("num = %d\r\nnew_drv:\r\n",num);
		//for(i=0;i<num;i++)
		//{
		//	Uart_Printf("%02x ",dbgbuf[i]);
		//}
		

		do{
			//usleep(1);
			//Uart_Printf("wait drive\r\n");
		}while(  (pKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );
		pKey = &(new_drv.drv_data);

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			ucKey = ERR_END;
			break;
		}else
		{
			uiTimeout = 50*100;
			ucKey = pKey->xxdata[1];
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( pucMask )
			{
				if( !strchr((char *)pucMask, ucKey ))
					continue;
			}
			switch(ucType)
			{
            case 'h':
			case 'H':
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
                    if(  (uiLastTime-uiTimeout) < 1*100)
					{
                        if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
                        if(  (ucKey >= '2')
						   &&(ucKey <= '3')
						  )
						{
                            if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset )
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
                        }else
						{
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
                    }else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
                }
				break;
            case 'a':
			case 'A':
				if( ucCount+1> ucMax)
				{
					Os__beep();
					break;
				}

				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{

					if(  (uiLastTime-uiTimeout) < 1*100)
					{
						if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
						if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
						{
							ucKeyTabOffset ++;
							ucRedraw = TRUE;
						}else
						{
							if( ucKeyTabOffset != 0)
							{
								ucKeyTabOffset = 0;
								ucRedraw = TRUE;
							}
						}
						aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
					}else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}

						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
				}

				break;
            default:
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
				break;
			}
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_END;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					if( ucLastKey != ucKey)
					{
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
							aucInput[ucCount++] = '0';
						else
							aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}else
					{
						if(  (uiLastTime-uiTimeout) < 1*100)
						{
							if( ucCount )
							{
								ucOffset = ucCount - 1;
							}else
							{
								ucOffset = 0;
							}
							if( ucKeyTabOffset < strlen((char *)aucKeyTab[10])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset != 0)
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[10][ucKeyTabOffset];
						}else
						{
							if( ucCount >= ucMax)
							{
								Os__beep();
								break;
							}
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
								aucInput[ucCount++] = '0';
							else
								aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
					}
				}else
				{
					if( ucCount > (ucMax-2))
					{
						Os__beep();
						break;
					}
					memcpy(&aucInput[ucCount],"00",2);
					ucCount = ucCount +2;
					ucRedraw = TRUE;
					break;
				}

				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucLastKey = 0;

					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				if(ucCount == 0)
					ucEnd = TRUE;
				if( ucCount < ucMin )
				{
					Os__beep();
				}
				else
				{
					memset((char *)pucBuf,0x00,ucMax);
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
	}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);
	return(ucKey);
}


unsigned char UTIL_GetKey(unsigned char wait_sec)
{
	unsigned int Timeout;
	DRV_OUT *KBoard;
	NEW_DRV_TYPE new_drv;

	Timeout=wait_sec*100;
	Os__timer_start(&Timeout);
	Os__xdelay(1);
	//KBoard=Os__get_key();
	
	Os_Wait_Event(KEY_DRV, &new_drv, 150000);
	KBoard = &(new_drv.drv_data);
	while ((Timeout!=0)&&(KBoard->gen_status==DRV_RUNNING));
	Os__timer_stop(&Timeout);
	if(Timeout==0)
	{
		Os__abort_drv(drv_mmi);
		Os__xdelay(10);
		return  99;
	}
	return KBoard->xxdata[1];
}

unsigned char UTIL_InputTransAmount(void)
{
      unsigned char ucResult=SUCCESS,aucAmountBuf[40],ucKey;
	unsigned long ulAmount,ulAmountLimit;
	unsigned char tmpBuf[17];


	clear_display();
	Os__GB2312_display(1, 0, (uchar *) "please input amount:");
	ulAmount=0;
	ucResult=UTIL_InputAmount(2,&ulAmount,1,9999999); 
	if(!ucResult)
	     G_NORMALTRANS_ulAmount = ulAmount;
	return ucResult;
}



unsigned char UTIL_InputAmount(unsigned char ucLine,unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ulMaxAmount)
{
	 unsigned char aucDisp[CFG_MAXDISPCHAR+1];  		/* Buffer for Display */
	 unsigned char aucInput[CFG_MAXAMOUNTLEN+1]; 		/* Buffer for Key Input */
	 unsigned char aucInputTmp1[CFG_MAXAMOUNTLEN+1],aucInputTmp2[CFG_MAXAMOUNTLEN+1],aucInputTmp3[CFG_MAXAMOUNTLEN+1]; /* Buffer for Key Input */
	unsigned char ucKey;
	unsigned char ucRedraw;
	unsigned char ucCount;
	unsigned char ucI;
	unsigned char ucJ;
	DRV_OUT *pdKey;
	NEW_DRV_TYPE new_drv;
	unsigned int uiTimeout;


	Os__GB2312_display(3, 0, (uchar *)"wrong input pls ");
	Os__GB2312_display(4, 0, (uchar *)"press clear key");
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucDisp[0],' ',CFG_MAXDISPCHAR);
	UTIL_Form_Montant(&aucDisp[5],*pulAmount,2);

	memset(&aucInput[0],0,sizeof(aucInput));
	long_asc( aucInput,10,pulAmount );
	ucRedraw = TRUE;
	uiTimeout = 50*100;
	ucCount = 10;
	 while(aucInput[10-ucCount]=='0'&&ucCount>0) ucCount--;
	 for(ucI=0;ucI<ucCount;ucI++)
	  aucInput[ucI]=aucInput[ucI+10-ucCount];
	 aucInput[ucCount]=0;


	 do
	 {
	 Os__timer_start(&uiTimeout);
		  if( ucRedraw == TRUE)
		  {
		   Os__clr_display(ucLine);
		   Os__display(ucLine,0,&aucDisp[0]);
		   ucRedraw = FALSE;
		  }
		//  pdKey = Os__get_key();
		 Os_Wait_Event(KEY_DRV, &new_drv, 150000);
        	 pdKey = &(new_drv.drv_data);
		  do{
		  }while(  (pdKey->gen_status==DRV_RUNNING) 
			  &&(uiTimeout !=0)
			  );
		 if(uiTimeout == 0)
		  {
		   Os__abort_drv(drv_mmi);
		   return(ERR_CANCEL);
		  }else
		  {		   
		   ucKey = pdKey->xxdata[1];
		  }
		  Os__timer_stop(&uiTimeout);
		  switch(ucKey)
		  {
		  case KEY_CLEAR:
			   if( ucCount )
			   {
					ucCount = 0;
					memset(&aucDisp[0],0,sizeof(aucDisp));
					memset(&aucDisp[0],' ',CFG_MAXDISPCHAR);
					aucDisp[CFG_MAXDISPCHAR-1] = '0';
					aucDisp[CFG_MAXDISPCHAR-2] = '0';
					aucDisp[CFG_MAXDISPCHAR-3] = '.';
					aucDisp[CFG_MAXDISPCHAR-4] = '0';
					ucRedraw = TRUE;
			   }else
			   {
					return(ERR_CANCEL);
			   }
			   break;
		  case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				memset(aucInputTmp1,0,sizeof(aucInputTmp1));
				long_asc(aucInputTmp1,CFG_MAXAMOUNTLEN,&ulMaxAmount);
				memset(aucInputTmp2,0,sizeof(aucInputTmp2));
				memset(aucInputTmp2,'0',CFG_MAXAMOUNTLEN);
				memcpy(&aucInputTmp2[CFG_MAXAMOUNTLEN-ucCount],aucInput,ucCount);
				memset(aucInputTmp3,0,sizeof(aucInputTmp3));
				long_asc(aucInputTmp3,CFG_MAXAMOUNTLEN,&ulMinAmount);
				if(memcmp(aucInputTmp2,aucInputTmp3,CFG_MAXAMOUNTLEN)<0)
				{
					Os__beep();
				}else
				{
					if(memcmp(aucInputTmp1,aucInputTmp2,CFG_MAXAMOUNTLEN)<0)
					{
		 				Os__beep();
					}else
					{
						 *pulAmount = asc_long(aucInput,ucCount);
						 return(SUCCESS);
					}
				}
				break;
		  case KEY_00_PT:
	  			if(ucCount ==0 ) continue;
				if( ucCount < CFG_MAXAMOUNTLEN)
				{
					 aucInput[ucCount] = '0';
					 ucCount ++;
					 if( ucCount < CFG_MAXAMOUNTLEN)
					 {
						  aucInput[ucCount] = '0';
						  ucCount ++;
					 }
					 ucRedraw = TRUE;
				}
				break;
		  default :
			   if(  (ucKey <= '9')
				  &&(ucKey >= '0')
				 )
			   {
					if(ucCount ==0 &&ucKey=='0') continue;
					if( ucCount < CFG_MAXAMOUNTLEN)
					{
						 aucInput[ucCount] = ucKey;
						 ucCount ++;
						 ucRedraw = TRUE;
					}
			   }
		   break;
		  }
		  memset(&aucDisp[0],0,sizeof(aucDisp));
		  memset(&aucDisp[0],' ',CFG_MAXDISPCHAR);
		  aucDisp[CFG_MAXDISPCHAR-1] = '0';
		  aucDisp[CFG_MAXDISPCHAR-2] = '0';
		  aucDisp[CFG_MAXDISPCHAR-3] = '.';
		  aucDisp[CFG_MAXDISPCHAR-4] = '0';
		  if( ucCount )
		  {
			   for(ucI=0,ucJ=0;ucI<ucCount;ucI++,ucJ++)
			   {
					if( ucJ == 2)
					{
						ucJ ++;
					}
					aucDisp[CFG_MAXDISPCHAR-ucJ-1] = aucInput[ucCount-ucI-1];
				}
		  }
	 }while(1);
}

void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule)
{
	unsigned char   i,j;

	long_str(Mnt_Fmt,10,&Montant);
	j = Pos_Virgule;
	for (i=9; j ; i--, j--)
		Mnt_Fmt[i+1] = Mnt_Fmt[i];
	if (Pos_Virgule)
	{
		Mnt_Fmt[i+1] ='.';
		Mnt_Fmt[i+1+Pos_Virgule+1] = 0;
	}
	for(j=0 ; ((j < i) && (Mnt_Fmt[j]=='0')) ; Mnt_Fmt[j++]=' ');
}

unsigned char UTIL_InputEncryptPIN(void)
{
	unsigned char ucResult;
	unsigned char aucPIN[13];
	unsigned char aucAmountBuf[20];
	unsigned char aucDispYUAN[15];
	unsigned char aucPINBuf[16];
	unsigned char aucCardPan[9];
	
	memset( aucAmountBuf, 0, sizeof(aucAmountBuf));
	memset( aucDispYUAN, 0, sizeof(aucDispYUAN));
	UTIL_Form_Montant(&aucAmountBuf[5],G_NORMALTRANS_ulAmount,2);
	memcpy(aucAmountBuf,"amount",6);
	
  
	memset(aucPIN,0,sizeof(aucPIN));
	clear_display();
	Os__GB2312_display(0,0,(unsigned char *)aucAmountBuf);
	Os__GB2312_display(1,0,(unsigned char *)"please input password");
	Os__GB2312_display(3,0,(unsigned char *)"without password ");
	Os__GB2312_display(4,0,(unsigned char *)"please press Enter");
    
      
	ucResult = UTIL_Input(2,true,0,6,'P',aucPIN,NULL);  
	if(ucResult == KEY_ENTER)
	{
		G_EXTRATRANS_ucInputPINLen = strlen((char *)aucPIN);
		ucResult = SUCCESS;
		if(G_EXTRATRANS_ucInputPINLen != 0)
		{
			memset(aucPINBuf,'F',sizeof(aucPINBuf));
			aucPINBuf[0] = '0';
			aucPINBuf[1] = G_EXTRATRANS_ucInputPINLen+'0';
			memcpy(&aucPINBuf[2],aucPIN,G_EXTRATRANS_ucInputPINLen);
			asc_hex(aucPIN,8,aucPINBuf,16);
			memset(aucCardPan,0,sizeof(aucCardPan));
			memcpy(&aucCardPan[2],G_NORMALTRANS_aucCardPan,6);
			gz_xor(aucCardPan,aucPIN,8);
			ucResult = PINPAD_47_Encrypt8ByteSingleKey(KEYARRAY_GOLDENCARDSH,
			KEYINDEX_GOLDENCARDSH_PINKEY,aucPIN,G_EXTRATRANS_aucPINData);
		}
	}
	return(ucResult);
}

void gz_xor(unsigned char *a, unsigned char *b, char lg)
{
	while (lg--)
		*(b++) ^= *(a++);
}

unsigned char UTIL_format_time_bcd_str( unsigned char *Ptd, unsigned char *Pts )
{
	unsigned char i ;

	for ( i = 0 ; i < 2 ; i++, Pts++)
	{
		*Ptd++ = (*Pts >> 4) | 0x30;
		*Ptd++ = (*Pts & 0x0F) | 0x30;
		if (!i)
			*Ptd++ = ':';
	}
	*Ptd = 0;
	return(SUCCESS);
}

unsigned char UTIL_format_date_bcd_str( unsigned char *Ptd, unsigned char *Pts )
{
    unsigned char i ;

    for ( i = 0 ; i < 4; i++,Pts++)  
    {
    *Ptd++ = (*Pts >> 4) | 0x30;
    *Ptd++ = (*Pts & 0x0F) | 0x30;
    if ( ( i == 1) || ( i == 2) ) *Ptd++ = '/';
    }
    return(SUCCESS);
}

void UTIL_Beep(void)
{
    char i;
    for(i=0;i<7;i++)
        Os__beep();
    return ;
}


