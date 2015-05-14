#include 	<QPBOC.h>
#include	<Util.h>
#include	<include.h>
#include	<Global.h>
#include "new_drv.h"

extern void des(uchar *binput, uchar *boutput, uchar *bkey);
unsigned char UTIL_InputAmount(unsigned char ucLine,
				unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ulMaxAmount)
{
	unsigned char	aucDisp[MAXDISPCHAR+1];		/* Buffer for Display */
	unsigned char	aucInput[MAXAMOUNTLEN+1];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucI;
	unsigned char	ucJ;
	DRV_OUT *pdKey;
    NEW_DRV_TYPE new_drv;
	unsigned int	uiTimeout;
	unsigned long	ulAmount;

	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucDisp[0],' ',MAXDISPCHAR);

	UTIL_Form_Montant(&aucDisp[5],*pulAmount,2);
	memset(&aucInput[0],0,sizeof(aucInput));
    CONV_LongAsc( aucInput,10,pulAmount );
	ucCount = 10;
	while(aucInput[10-ucCount]=='0'&&ucCount>0) ucCount--; 
	for(ucI=0;ucI<ucCount;ucI++)
		aucInput[ucI]=aucInput[ucI+10-ucCount];
	aucInput[ucCount]=0;
	ucRedraw = TRUE;

	do
	{
		if( ucRedraw == TRUE)
        {
            Os__clr_display11(ucLine);
            Os__display11(ucLine,0,&aucDisp[0]);
            printf("333333\n");
			ucRedraw = FALSE;
		}
		
        Os_Wait_Event(KEY_DRV,&new_drv,15000);
        if(new_drv.drv_type == KEY_DRV){
            printf("key drv******\n");
            pdKey = &new_drv.drv_data;
            ucKey = pdKey->xxdata[1];
        }
        else if(new_drv.drv_type == DRV_TIMEOUT)
        {
            Os__abort_drv(drv_mmi);
            Os__xdelay(10);
            return(QERROR_CANCEL);
        }

        printf("uckey===%02x\n",ucKey);

		switch(ucKey)
		{
		case KEY_CLEAR:
			if( ucCount )
			{
				ucCount = 0;
				memset(&aucDisp[0],0,sizeof(aucDisp));
				memset(&aucDisp[0],' ',MAXDISPCHAR);
				aucDisp[MAXDISPCHAR-1] = '0';
				aucDisp[MAXDISPCHAR-2] = '0';
				aucDisp[MAXDISPCHAR-3] = '.';
				aucDisp[MAXDISPCHAR-4] = '0';
				ucRedraw = TRUE;
			}else
			{
				return(QERROR_CANCEL);
			}
			break;
		case KEY_BCKSP:
			if( ucCount )
            {            printf("3666\n");

				ucCount --;
				aucInput[ucCount] = 0;
				ucRedraw = TRUE;
			}
			break;
		case KEY_ENTER:
			ulAmount = asc_long(aucInput,ucCount);
			if( ulAmount < ulMinAmount )
			{
				Os__beep();
			}else
			{
				if( ulAmount > ulMaxAmount )
				{
					Os__beep();					
					
				}else
				{
					*pulAmount = ulAmount;
					return(QERROR_SUCCESS);
				}
			}
			break;
		default :
			if(  (ucKey <= '9') 
			   &&(ucKey >= '0')
			  )
            {            printf("4444444444\n");

				if(ucCount==0&&ucKey=='0')
					break;
				if( ucCount < MAXAMOUNTLEN)
				{
					aucInput[ucCount] = ucKey;
					ucCount ++;
					ucRedraw = TRUE;
				}
			}
			break;
        }            printf("555\n");

		/* Copy data from Input buffer to Display buffer */
		memset(&aucDisp[0],0,sizeof(aucDisp));
		memset(&aucDisp[0],' ',MAXDISPCHAR);
		aucDisp[MAXDISPCHAR-1] = '0';
		aucDisp[MAXDISPCHAR-2] = '0';
		aucDisp[MAXDISPCHAR-3] = '.';
		aucDisp[MAXDISPCHAR-4] = '0';			
		if( ucCount )
		{
			for(ucI=0,ucJ=0;ucI<ucCount;ucI++,ucJ++)
			{
				if( ucJ == 2)
				{
					ucJ ++;
				}
				aucDisp[MAXDISPCHAR-ucJ-1] = aucInput[ucCount-ucI-1];
			}
		}
	}while(1);
}

unsigned char UTIL_Input_pp(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf)
				
{
	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pdKey;
    NEW_DRV_TYPE new_drv;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{

        Os__clr_display_pp11(ucLine);
         emitSignal();
	}
//	UTIL_display_amount(0,G_NORMALTRANS_ulAmount);
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);			
	}
    //Os__timer_start(&uiTimeout);

	do
	{
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > 15)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',15);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-15],15);
				}
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					if(ucCount == 0)
					{
						memset(aucDisp,' ',sizeof(aucDisp));
						aucDisp[15] = 0x00;
					}
					else
						memset(&aucDisp[0],'*',ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
			}

            Os__clr_display_pp11(ucLine);
            Os__display_pp11(ucLine,0,&aucDisp[0]);
            emitSignal();


			ucRedraw = FALSE;
		}
//		Uart_Printf("\nOs__get_key_pp");
//		pdKey = Os__get_key_pp();
        Os_Wait_Event(KEY_DRV,&new_drv,15000);


        if (new_drv.drv_type == DRV_TIMEOUT)
		{
			Os__abort_drv(drv_mmi);
			Os__xdelay(10);
			ucEnd = TRUE;
			ucKey = QERROR_CANCEL;
			break;
        }
        else if(new_drv.drv_type == KEY_DRV)
		{
            pdKey = &new_drv.drv_data;
			ucKey = pdKey->xxdata[1];
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( ucCount >= ucMax)
			{
				Os__beep();
				continue;
			}
			aucInput[ucCount++] = ucKey;
			ucRedraw = TRUE;
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
					ucKey = QERROR_CANCEL;
					ucEnd = TRUE;
				}
				break;
			case KEY_ENTER:
				
				if(ucCount == 0) 
				{
//					memset((char *)pucBuf,0x00,sizeof(pucBuf));
					ucEnd = TRUE;
					ucKey =QERROR_BYPASS;
					
				}
				else if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					memset((char *)pucBuf,0x00,sizeof(pucBuf));
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

    //Os__timer_stop(&uiTimeout);
	
		
	return(ucKey);
}

unsigned char UTIL_Input(ALIGN enAlign,unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
    const unsigned char aucKeyTab[][7]
	={
//		{"0 *\0"},
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
		{"0,.\0"}
	};

	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pKey;
    NEW_DRV_TYPE new_drv;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;
	if( ucClrFlag )
	{
        Os__clr_display11(ucLine);
        emitSignal();

	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}

    //Os__timer_start(&uiTimeout);

	do
	{
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
				}else if(  (ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if(ucCount>=ucMax)
						memcpy(&aucDisp[0],&aucInput[0],ucCount-1);
					else
						memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				aucDisp[ucCount] = '_';
			}


            Os__clr_display11(ucLine);
            emitSignal();
			switch(enAlign)
			{
			case LEFT_ALIGN:
                Os__display11(ucLine,0,&aucDisp[0]);
				break;
			case MIDDLE_ALIGN:
                Os__display11(ucLine,(MAXLINECHARNUM-strlen((char*)aucDisp)+1)/2,&aucDisp[0]);
				break;
			case RIGHT_ALIGN:
                Os__display11(ucLine,MAXLINECHARNUM-strlen((char*)aucDisp)+1,&aucDisp[0]);
				break;
			}
           emitSignal();

			ucRedraw = FALSE;
		}

        Os_Wait_Event(KEY_DRV,&new_drv,15000);
//		pKey = Os__get_key();

//		do{
//		}while(  (pKey->gen_status==DRV_RUNNING)
//			   &&(uiTimeout !=0)
//			   );

        if (new_drv.drv_type==DRV_TIMEOUT)
		{
			Os__abort_drv(drv_mmi);
			ucKey = QERROR_CANCEL;
			break;
        }else if(new_drv.drv_type== KEY_DRV)
		{
            pKey = &new_drv.drv_data;
			ucKey = pKey->xxdata[1];
//			Uart_Printf("\n Key Value %02x",ucKey);
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
				if( ucLastKey != ucKey)
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
				}else
				{
                    if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
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
							ucLastKey = ucKey;
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
				if( ucLastKey != ucKey)
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
				}else
				{
					if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
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
						if( ucCount >=ucMax)
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
				if( ucCount >=ucMax)
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
					ucKey = QERROR_CANCEL;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
//			case 0x00:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
//					if( ucCount < strlen((char *)&aucInput[0]))
//						ucCount ++;
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

						if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
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
//					aucInput[ucCount] = ucKey;
					memcpy(&aucInput[ucCount],"00",2);
					ucCount = ucCount +2;
					ucRedraw = TRUE;
					break;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				ucLastKey = 0;
				break;
			case KEY_ENTER:
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					memset((char *)pucBuf,0x00,sizeof(pucBuf));
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

    //Os__timer_stop(&uiTimeout);
	return(ucKey);
}



void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule)
{
	uchar   i;
	uchar   j;

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

POPTION UTIL_SelectOption(PUCHAR szOptionTitle,UCHAR ucFont,POPTION pOption ,UCHAR ucOptionNums)
{
	UCHAR	ucI,ucMaxRows,ucCurIndex,ucDispRows;
	POPTION pCurOption;
	UCHAR	ucKey;
	
	ucCurIndex=0;
	while(1)
	{
        Os__clr_display11(255);
		if(ucFont==ASCII_FONT)
		{
			ucMaxRows=8;
            OSMMI_DisplayASCII11(0x30|0x80,0,0,szOptionTitle);
		}
		else
		{
			ucMaxRows=4;

            OSMMI_GB2312Display11(0x31|0x80,0,0,szOptionTitle);
		}
        emitSignal();

		ucDispRows=ucCurIndex+ucMaxRows-1<ucOptionNums-1? ucMaxRows-1:ucOptionNums-ucCurIndex;
		
		for(ucI=0;ucI<ucDispRows;ucI++)
		{
			
			if(ucFont==ASCII_FONT)
                OSMMI_DisplayASCII11(0x30,ucI+1,0,(pOption+ucI+ucCurIndex)->szOptionMsg);
			else
                OSMMI_GB2312Display11(0x31,ucI+1,0,(pOption+ucI+ucCurIndex)->szOptionMsg);

          emitSignal();

		}
        ucKey=Os__xget_key11();
		switch(ucKey)
		{
			case KEY_F3:
				if(ucCurIndex+ucMaxRows-1<ucOptionNums-1)
					ucCurIndex+=ucMaxRows-1;
				break;
			case KEY_F4:
				if(ucCurIndex+1>ucMaxRows-1)
					ucCurIndex-=ucMaxRows-1;
				break;
			case KEY_ENTER:
				return pOption+ucCurIndex;
				 
			case KEY_CLEAR:
				return NULL;
				break;
			default:
				for(ucI=0;ucI<ucOptionNums;ucI++)
				{
					if((pOption+ucI)->ucOptionValue==ucKey-0x30)
						return pOption+ucI;
				}
		}
		
	}
	
}

UCHAR	UTIL_OfflinePIN(PUCHAR	pucOfflinePin)
{
	unsigned char ucResult;
	
    Os__clr_display_pp11(255);
    Os__GB2312_display_pp11(0,0,(PUCHAR)"Offline PIN pls");

    emitSignal();
    ucResult = UTIL_Input_pp(1,TRUE,4,12,'P',pucOfflinePin);
	if(ucResult ==KEY_ENTER)
		return QERROR_SUCCESS;
	else if(ucResult == QERROR_BYPASS)
		return QERROR_BYPASS;
	else
		return QERROR_CANCEL;
	
}

UCHAR	UTIL_OnlinePIN(PUCHAR pucEnPIN,PUCHAR pucPinLen)
{
	UCHAR	aucPinData[20],ucPinLen,ucResult,aucBuff[20];
	const UCHAR  PINKEY[] ="12345678";


    Os__clr_display_pp11(255);
    Os__GB2312_display_pp11(0,0,(PUCHAR)"Online PIN pls");
    emitSignal();

	memset(aucPinData,0x00,sizeof(aucPinData));
	ucResult = UTIL_Input_pp(1,TRUE,4,12,'P',aucPinData);
	if(ucResult ==KEY_ENTER)
	{
		memset(aucBuff,0xFF,sizeof(aucBuff));
		ucPinLen = strlen((char*)aucPinData);
		aucBuff[0] = 0x00 + ucPinLen;
		if(ucPinLen%2)
		{
			aucPinData[ucPinLen] =0x3F;
			ucPinLen++;
		}
		CONV_AscBcd(&aucBuff[1],ucPinLen/2,aucPinData,ucPinLen);
		des(aucBuff,pucEnPIN,(PUCHAR)PINKEY);
		*pucPinLen =8;
		return QERROR_SUCCESS;
		
		
	}
	else if(ucResult == QERROR_BYPASS)
		return QERROR_BYPASS;
	return 
		QERROR_CANCEL;
	
}

UCHAR UTIL_WriteConstParamFile(QEMVCONSTPARAM * pConstParam)
{
    printf("~~~~~~~~0000 ucResult  \n");

	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;
	
	iHandle = OSAPP_OpenFile((char *)CONSTPARAMFILE,O_WRITE);
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pConstParam,sizeof(QEMVCONSTPARAM));
	}
	OSAPP_FileClose(iHandle);
	ucResult = OSAPP_FileGetLastError();
    printf("~~~~~~~~11111 ucResult  = %02x\n",ucResult);
    if(ucResult)
		ucResult =QERROR_SAVEFILE;
	return ucResult;
}

UCHAR UTIL_WriteEMVConfigFile(QTERMCONFIG * pEMVConfig)
{
	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;
	
	iHandle = OSAPP_OpenFile((char *)EMVCONFIGFILE,O_WRITE);
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pEMVConfig,sizeof(QTERMCONFIG));
	}
	OSAPP_FileClose(iHandle);
	ucResult = OSAPP_FileGetLastError();
	if(ucResult)
		ucResult =QERROR_SAVEFILE;
	return ucResult;
}

UCHAR UTIL_WriteReaderParamFile(QREADERPARAM *pRM)
{
	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;
	
	iHandle = OSAPP_OpenFile((char *)READERPARAMFILE,O_WRITE);
	if(iHandle>=0)
	{
		OSAPP_FileTrunc(iHandle,0);
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pRM,sizeof(QREADERPARAM));
	}
	OSAPP_FileClose(iHandle);
	ucResult = OSAPP_FileGetLastError();
	if(ucResult)
		ucResult =QERROR_SAVEFILE;
	return ucResult;
}


UCHAR UTIL_SaveCAPKFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	ucResult =UTIL_DeleteAllData((PUCHAR)CAPKFILE);
	for(ucI=0;!ucResult && ucI<ucQTermCAPKNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(CAPKFILE,&QTermCAPK[ucI],sizeof(QCAPK));
		if(uiRecordNum==0) ucResult =QERROR_SAVEFILE;
#if 0
		Uart_Printf("\n RID:");
		for(ucJ =0;ucJ<5;ucJ++) Uart_Printf("%02x ",QTermCAPK[ucI].aucRID[ucJ]);
		Uart_Printf(" CAPKI:%02x",QTermCAPK[ucI].ucCAPKI);
#endif
	}
	return ucResult;
	
}

UCHAR UTIL_SaveAIDFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	
	ucResult =UTIL_DeleteAllData((PUCHAR)QTERMSUPPORTAPPFILE);
	for(ucI=0;!ucResult && ucI<ucQTermAIDNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(QTERMSUPPORTAPPFILE,&QTermAID[ucI],sizeof(QTERMSUPPORTAPP));
		if(uiRecordNum==0) ucResult =QERROR_SAVEFILE;
#if 0
		Uart_Printf("\nASI: %02X",QTermAID[ucI].ucASI);
		Uart_Printf("AID:");
		for(ucJ=0;ucJ<QTermAID[ucI].ucAIDLen;ucJ++)
			Uart_Printf("%02X ",QTermAID[ucI].aucAID[ucJ]);
#endif
		
	}
	return ucResult;
	
}

UCHAR UTIL_SaveExceptFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	
	ucResult =UTIL_DeleteAllData((PUCHAR)EXCEPTFILE);
	for(ucI=0;!ucResult && ucI<ucQExceptFileNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(EXCEPTFILE,&QExceptFile[ucI],sizeof(QEXCEPTPAN));
		if(uiRecordNum==0) ucResult =QERROR_SAVEFILE;
#if 0
		Uart_Printf("\nPAN:");
		for(ucJ=0; ucJ<QMAXPANDATALEN;ucJ++)
			Uart_Printf("%02X ",QExceptFile[ucI].aucPAN[ucJ]);
		Uart_Printf(" %02X",QExceptFile[ucI].ucPANSeq);
#endif
	}
	return ucResult;
	
}

UCHAR UTIL_SaveIPKRevokeFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	
	ucResult =UTIL_DeleteAllData((PUCHAR)IPKREVOKEFILE);
	for(ucI=0;!ucResult && ucI<ucQIPKRevokeNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(IPKREVOKEFILE,&QIPKRevoke[ucI],sizeof(QIPKREVOKE));
		if(uiRecordNum==0) ucResult =QERROR_SAVEFILE;
#if 0
		Uart_Printf("\n RID:");
		for(ucJ =0;ucJ<5;ucJ++) Uart_Printf("%02x ",QIPKRevoke[ucI].aucRID[ucJ]);
		Uart_Printf(" CAPKI:%02x",QIPKRevoke[ucI].ucCAPKI);
		for(ucJ =0;ucJ<3;ucJ++) Uart_Printf("%02x ",QIPKRevoke[ucI].aucCertSerial[ucJ]);
#endif
	}
	return ucResult;
	
}

unsigned char UTIL_GetKey(unsigned char wait_sec)
{  
	unsigned int Timeout;
	DRV_OUT *KBoard;
    NEW_DRV_TYPE new_drv;

//    Os__xdelay(1);

    Os_Wait_Event(KEY_DRV,&new_drv,wait_sec*1000);

    if(new_drv.drv_type==KEY_DRV){
        KBoard = &new_drv.drv_data;
        return KBoard->xxdata[1];
    }
   else if(new_drv.drv_type==DRV_TIMEOUT)
    {
        Os__abort_drv(drv_mmi);
//        Os__xdelay(10);
        return  99;
    }
}

UCHAR	UTIL_DeleteAllData(UCHAR* pucFileName)
{
	int		iHandle;
	long	uiFileSize;
	UCHAR	ucResult;

	
	uiFileSize = OSAPP_FileSize((char*)pucFileName);
	if(uiFileSize>0)
	{
		iHandle = OSAPP_OpenFile((char*)pucFileName,O_WRITE);
		if(iHandle>=0)
		{
			OSAPP_FileTrunc(iHandle,0);
			OSAPP_FileClose(iHandle);
		}
	}
	
	return (QERROR_SUCCESS);

}

UCHAR	UTIL_EMVInformMsg(PUCHAR	pucInformMsg)
{
    Os__clr_display11(255);
    Os__GB2312_display11(2,0,pucInformMsg);
    emitSignal();
	UTIL_GetKey(3);
	return QERROR_SUCCESS;
}


UCHAR	UTIL_IDCardVerify(void)
{
	UCHAR	ucResult,ucIDType,aucID[30],aucBuff[40],ucKey;
	USHORT	uiIDLen,uiLen;

	
	ucResult=QERROR_SUCCESS;

	if(!ucResult)
	{
		memset(aucID,0x00,sizeof(aucID));
		uiIDLen=sizeof(aucID);
		ucResult=QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x9F\x61",aucID,&uiIDLen);
	}
	if(!ucResult)
	{
		uiLen =sizeof(UCHAR);
		ucResult=QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x9F\x62",&ucIDType,&uiLen);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"ID:");
		memcpy(aucBuff+strlen((char*)aucBuff),aucID,uiIDLen);
		

        Os__clr_display11(255);
        OSMMI_DisplayASCII11(0x30,0,0,aucBuff);
		
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"ÍĢ");
		switch(ucIDType)
		{
		case 0x00:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"Éí·ÝÖĪ");
			break;
		case 0x01:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"ūüđŲÖĪ");
			break;
		case 0x02:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"ŧĪÕÕ");
			break;
		case 0x03:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"ČëūģÖĪ");
			break;
		case 0x04:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"ÁŲĘąÉí·ÝÖĪ");
			break;
		default://05
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"ÆäËü");
			break;
		
		}


        Os__GB2312_display11(1,0,aucBuff);

        OSMMI_DisplayASCII11(0x30,4,0,(PUCHAR)"1.ID Verify Pass");
        OSMMI_DisplayASCII11(0x30,5,0,(PUCHAR)"2.ID Verify Fail");
        OSMMI_DisplayASCII11(0x30,7,0,(PUCHAR)"Pls Confirm");
        emitSignal();
		while(1)
		{
            ucKey=Os__xget_key11();
			if(ucKey=='1')
			{
				ucResult=QERROR_SUCCESS;
				break;
			}
			else if(ucKey=='2')
			{
				ucResult=QERROR_CANCEL;
				break;
			}
			else
				continue;
		}

	
		
	}
	return ucResult;
}
