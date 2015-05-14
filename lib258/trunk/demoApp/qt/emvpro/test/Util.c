#include 	<EMV41.h>
#include	<test/Util.h>
#include	<test/include.h>
#include	<test/Global.h>
#include "new_drv.h"
#include "tools.h"
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
    unsigned long	ulAmount;
    NEW_DRV_TYPE new_drv;

	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucDisp[0],' ',MAXDISPCHAR);
	aucDisp[MAXDISPCHAR-1] = '0';
	aucDisp[MAXDISPCHAR-2] = '0';
	aucDisp[MAXDISPCHAR-3] = '.';
	aucDisp[MAXDISPCHAR-4] = '0';
	memset(&aucInput[0],0,sizeof(aucInput));
	ucRedraw = TRUE;
    ucCount = 0;

	do
	{
		if( ucRedraw == TRUE)
		{
            Os__clr_display11(ucLine);
            Os__display11(ucLine,0,&aucDisp[0]);
            emitSignal();
			ucRedraw = FALSE;
		}
		
        unsigned char ret = Os_Wait_Event(KEY_DRV , &new_drv, 15000);
        if(new_drv.drv_type == KEY_DRV){
            pdKey = &new_drv.drv_data;
            ucKey = pdKey->xxdata[1];


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
                    return(EMVERROR_CANCEL);
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
                ulAmount = CONV_AscLong(aucInput,ucCount);
                printf("ulAmount==%d\n",ulAmount);
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
                        printf("pulAmount==%d\n",pulAmount);

                        return(EMVERROR_SUCCESS);
                    }
                }
                break;
            default :
                if(  (ucKey <= '9')
                     &&(ucKey >= '0')
                     )
                {
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
            }
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
        }
        else if(new_drv.drv_type == DRV_TIMEOUT){
            Os__abort_drv(drv_mmi);
            Os__xdelay(10);
            return(EMVERROR_CANCEL);
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
//	static unsigned int	uiTimeout;
//	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
//	uiTimeout = 50*ONESECOND;
//	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
        Os__clr_display_pp11(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
        printf("ucCount  = %d \n",ucCount);
        memcpy(aucInput,pucBuf,ucCount);
        printf("aucInput  = %s \n",aucInput);

	}

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


            printf("Os__display_pp11 aucDisp=%s\n",&aucDisp[0]);
            ucRedraw = FALSE;
		}
//		Uart_Printf("\nOs__get_key_pp");
        //pdKey = Os__get_key_pp();

//		do{
//		}while(  (pdKey->gen_status==DRV_RUNNING)
//			   &&(uiTimeout !=0)
//			   );

        unsigned int rec = Os_Wait_Event(KEY_DRV,&new_drv,15000);
        if(new_drv.drv_type == KEY_DRV)
        {
            pdKey = &new_drv.drv_data;
            ucKey = pdKey->xxdata[1];
        }
        else if(new_drv.drv_type == DRV_TIMEOUT){
            Os__abort_drv(drv_mmi);
//            Os__xdelay(10);
            ucEnd = TRUE;
            ucKey = EMVERROR_CANCEL;
            break;
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
					ucKey = EMVERROR_CANCEL;
					ucEnd = TRUE;
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
				
				if(ucCount == 0) 
				{
//					memset((char *)pucBuf,0x00,sizeof(pucBuf));
					ucEnd = TRUE;
					ucKey =EMVERROR_BYPASS;
					
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
//		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}	
	}while(1);

//	Os__timer_stop(&uiTimeout);
	
		
	return(ucKey);
}

unsigned char UTIL_Input_PP_Uart(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr,
									 char ucMinNB,unsigned char ucMaxNB,unsigned char *PinData)
{
	unsigned char ucResult,ucKey;
	
	DRV_OUT *pxPINData;

	ucResult = EMVERROR_SUCCESS;
#if 0
	pxPINData = OSPAD_GetPinKey(0,0,pucPtr,ucMinNB,ucMaxNB);

	while(pxPINData->gen_status!=DRV_ENDED);

	if(pxPINData->gen_status==DRV_ENDED)
	{
		if(!pxPINData->drv_status)
		{
			if(pxPINData->xxdata[0] == 0)
			{
				return EMVERROR_BYPASS;	
					
			}else
			{
				memset((char *)PinData,0x00,sizeof(PinData));
				memcpy(PinData,&pxPINData->xxdata[1],pxPINData->xxdata[0]);
				return EMVERROR_SUCCESS;
			}
		}
		else
		{	
			Os__abort_drv(drv_pad);
			return EMVERROR_CANCEL;
		}
	}
#endif
	return EMVERROR_FUNCNOTSUPPORT;
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
    NEW_DRV_TYPE new_drv;

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
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
//	ucRedraw = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;
	if( ucClrFlag )
	{
        Os__clr_display11(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);

        //printf("aucInput===%s\n",aucInput);
        //printf("ucCount===%d\n",ucCount);

        //printf("pucBuf===%s\n",pucBuf);

	}


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
                    //printf("111aucInput===%s\n",aucInput);

					memcpy(&aucDisp[0],&aucInput[0],ucCount);
                    //printf("111aucDisp===%s\n",aucDisp);

				}
				aucDisp[ucCount] = '_';
			}
            Os__clr_display11(ucLine);
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



        unsigned int ret = Os_Wait_Event(KEY_DRV,&new_drv,45000);

       if(new_drv.drv_type == KEY_DRV){
            pKey = &new_drv.drv_data;
            ucKey = pKey->xxdata[1];

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
                //printf("ss  ucKey==%02x\n",ucKey);
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
					ucKey = EMVERROR_CANCEL;
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
                    //printf("~~~~~pucBuf==%s\n",pucBuf);
                    //printf("~~~~~aucInput==%s\n",aucInput);

					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
	}
        }
        else if(new_drv.drv_type == DRV_TIMEOUT)
         {
             Os__abort_drv(drv_mmi);
             ucKey = EMVERROR_CANCEL;
             //printf("timeout~~~~~~~~~~~uckey  = %02x \ n",ucKey);

            break;
         }

        //uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);


    //printf("~~~~~~~~~~~uckey  = %02x \n",ucKey);
	return(ucKey);
}



void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule)
{
	uchar   i;
	uchar   j;

    printf("\n!!!!!!!!!! Montant==%d\n",Montant);

        CONV_LongStr(Mnt_Fmt,10,&Montant);
        printf("@@@@@@ Mnt_Fmt==%s\n",Mnt_Fmt);
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

POPTION UTIL_SelectOption(PUCHAR szOptionTitle,FONT ucFont,POPTION pOption ,UCHAR ucOptionNums)
{

	UCHAR	ucI,ucMaxRows,ucCurIndex,ucDispRows;
	POPTION pCurOption;
	UCHAR	ucKey;
	
	ucCurIndex=0;
	while(1)
	{
        Os__clr_display11(255);
       printf("~~~~~~~~szOptionTitle===%s\n",szOptionTitle);
		if(ucFont==ASCII_FONT)
		{
			ucMaxRows=8;

            printf("~~~~~~~~sizeof(szOptionTitle)===%d\n",sizeof(szOptionTitle));
            OSMMI_DisplayASCII11(0x30,0,0,szOptionTitle);

		}
		else
		{
			ucMaxRows=4;

            OSMMI_GB2312Display11(0x31,0,0,szOptionTitle);
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

    if(ConstParam.bPBOC20==false)
	{
        Os__GB2312_display_pp(0,0,(PUCHAR)"Please Enter PIN");

	}
	else
	{
        Os__GB2312_display_pp(0,0,(PUCHAR)"请输入密码");
	}

    emitSignal();


    printf("~~~~~~~~~pucOfflinePin == %s \n",pucOfflinePin);

    if(!ConstParam.bAutoSupport)
          ucResult = UTIL_Input_pp(1,TRUE,4,12,'P',pucOfflinePin);
    else {
         Os__GB2312_display_pp(1,0,(PUCHAR)"******");
         printf("ConstParam.ucPinNum =%s\n",ConstParam.ucPinNum);
         pucOfflinePin = ConstParam.ucPinNum;
         printf("pucOfflinePin =%s\n",pucOfflinePin);

         emitSignal();


         sleep(2);
         return EMVERROR_SUCCESS;
    }


    printf("~~~~~~~~~UTIL_Input_pp ucResult == %02x \n",ucResult);
	if(ucResult ==KEY_ENTER)
		return EMVERROR_SUCCESS;
	else if(ucResult == EMVERROR_BYPASS)
		return EMVERROR_BYPASS;
	else
		return EMVERROR_CANCEL;

}

UCHAR	UTIL_OnlinePIN(PUCHAR pucEnPIN,PUCHAR pucPinLen)
{

	UCHAR	aucPinData[20],ucPinLen,ucResult,aucBuff[20];
	const UCHAR  PINKEY[] ="12345678";
	
    Os__clr_display_pp11(255);
	if(ConstParam.bPBOC20==FALSE)
	{
        Os__GB2312_display_pp(0,0,(PUCHAR)"Please Enter PIN");
	}
	else
	{
        Os__GB2312_display_pp(0,0,(PUCHAR)"请输入密码");
	}
    emitSignal();


	memset(aucPinData,0x00,sizeof(aucPinData));

    if(!ConstParam.bAutoSupport)
          ucResult = UTIL_Input_pp(1,TRUE,4,12,'P',aucPinData);
    else {
         Os__GB2312_display_pp(1,0,(PUCHAR)"******");
         printf("ConstParam.ucPinNum =%s\n",ConstParam.ucPinNum);
         pucEnPIN = ConstParam.ucPinNum;
         printf("pucOfflinePin =%s\n",pucEnPIN);
         pucPinLen = 6;
         emitSignal();

         sleep(2);
         return EMVERROR_SUCCESS;

    }

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
		return EMVERROR_SUCCESS;
		
		
	}
	else if(ucResult == EMVERROR_BYPASS)
		return EMVERROR_BYPASS;
	return 
		EMVERROR_CANCEL;

}



UCHAR UTIL_WriteConstParamFile(EMVCONSTPARAM * pConstParam)
{

	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;
	
	iHandle = OSAPP_OpenFile((char *)CONSTPARAMFILE,O_WRITE);
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pConstParam,sizeof(EMVCONSTPARAM));
	}
	OSAPP_FileClose(iHandle);
	ucResult = OSAPP_FileGetLastError();
	if(ucResult)
		ucResult =EMVERROR_SAVEFILE;
	return ucResult;

}

UCHAR UTIL_WriteEMVConfigFile(TERMCONFIG * pEMVConfig)
{

	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;
	
	iHandle = OSAPP_OpenFile((char *)EMVCONFIGFILE,O_WRITE);
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);
		
		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pEMVConfig,sizeof(TERMCONFIG));
	}
	OSAPP_FileClose(iHandle);
	ucResult = OSAPP_FileGetLastError();
	if(ucResult)
		ucResult =EMVERROR_SAVEFILE;
	return ucResult;

}

UCHAR UTIL_SaveCAPKFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	ucResult =UTIL_DeleteAllData((PUCHAR)CAPKFILE);
	for(ucI=0;!ucResult && ucI<ucTermCAPKNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(CAPKFILE,&TermCAPK[ucI],sizeof(CAPK));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 0
		Uart_Printf("\n RID:");
		for(ucJ =0;ucJ<5;ucJ++) Uart_Printf("%02x ",TermCAPK[ucI].aucRID[ucJ]);
		Uart_Printf(" CAPKI:%02x",TermCAPK[ucI].ucCAPKI);
#endif
	}
	return ucResult;
	
}

UCHAR UTIL_SaveAIDFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	
	ucResult =UTIL_DeleteAllData((PUCHAR)TERMSUPPORTAPPFILE);
	for(ucI=0;!ucResult && ucI<ucTermAIDNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(TERMSUPPORTAPPFILE,&TermAID[ucI],sizeof(TERMSUPPORTAPP));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 0
		Uart_Printf("\nASI: %02X",TermAID[ucI].ucASI);
		Uart_Printf("AID:");
		for(ucJ=0;ucJ<TermAID[ucI].ucAIDLen;ucJ++)
			Uart_Printf("%02X ",TermAID[ucI].aucAID[ucJ]);
#endif
		
	}
	return ucResult;
	
}

UCHAR UTIL_SaveExceptFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	
	ucResult =UTIL_DeleteAllData((PUCHAR)EXCEPTFILE);
	for(ucI=0;!ucResult && ucI<ucExceptFileNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(EXCEPTFILE,&ExceptFile[ucI],sizeof(EXCEPTPAN));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 0
		Uart_Printf("\nPAN:");
		for(ucJ=0; ucJ<MAXPANDATALEN;ucJ++)
			Uart_Printf("%02X ",ExceptFile[ucI].aucPAN[ucJ]);
		Uart_Printf(" %02X",ExceptFile[ucI].ucPANSeq);
#endif
	}
	return ucResult;
	
}

UCHAR UTIL_SaveIPKRevokeFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	
	ucResult =UTIL_DeleteAllData((PUCHAR)IPKREVOKEFILE);
	for(ucI=0;!ucResult && ucI<ucIPKRevokeNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(IPKREVOKEFILE,&IPKRevoke[ucI],sizeof(IPKREVOKE));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 0
		Uart_Printf("\n RID:");
		for(ucJ =0;ucJ<5;ucJ++) Uart_Printf("%02x ",IPKRevoke[ucI].aucRID[ucJ]);
		Uart_Printf(" CAPKI:%02x",IPKRevoke[ucI].ucCAPKI);
		for(ucJ =0;ucJ<3;ucJ++) Uart_Printf("%02x ",IPKRevoke[ucI].aucCertSerial[ucJ]);
#endif
	}
	return ucResult;
	
}

unsigned char UTIL_GetKey(unsigned char wait_sec)
{  
	unsigned int Timeout;
	DRV_OUT *KBoard;

    NEW_DRV_TYPE new_drv;

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

//    unsigned int ret =Os_Wait_Event(KEY_DRV,&new_drv,wait_sec*1000);
//    if(ret==0)
//    {
//        KBoard = &new_drv.drv_data;
//        return KBoard->xxdata[1];
//    }
//    else
//    {
//        Os__abort_drv(drv_mmi);
//        Os__xdelay(10);
//        return  99;
//    }
//	Timeout=wait_sec*100;
//	Os__timer_start(&Timeout);
//	Os__xdelay(1);
//    	KBoard=Os__get_key();
//    	while ((Timeout!=0)&&(KBoard->gen_status==DRV_RUNNING));
//    	Os__timer_stop(&Timeout);
//    	if(Timeout==0)
//    	{
//    		Os__abort_drv(drv_mmi);
//		Os__xdelay(10);
//    		return  99;
//    	}

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
	
	return (EMVERROR_SUCCESS);

}

UCHAR	UTIL_EMVInformMsg(PUCHAR pucInformMsg)
{
    Os__clr_display11(255);
    printf("\n~~~~~~~~pucInformMsg==%s\n",pucInformMsg);
    Os__GB2312_display11(2,0,pucInformMsg);
    emitSignal();

	UTIL_GetKey(3);
	return EMVERROR_SUCCESS;
}


UCHAR	UTIL_IDCardVerify(void)
{

	UCHAR	ucResult,ucIDType,aucID[30],aucBuff[40],ucKey;
	USHORT	uiIDLen,uiLen;

	
	ucResult=EMVERROR_SUCCESS;

	if(!ucResult)
	{
		memset(aucID,0x00,sizeof(aucID));
		uiIDLen=sizeof(aucID);
		ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x61",aucID,&uiIDLen);
	}
	if(!ucResult)
	{
		uiLen =sizeof(UCHAR);
		ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x62",&ucIDType,&uiLen);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"ID:");
		memcpy(aucBuff+strlen((char*)aucBuff),aucID,uiIDLen);
		
        Os__clr_display11(255);
        OSMMI_DisplayASCII11(0x30,0,0,aucBuff);
        emitSignal();


		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"Í£");
		switch(ucIDType)
		{
		case 0x00:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"Ö¤");
			break;
		case 0x01:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"Ö¤");
			break;
		case 0x02:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"");
			break;
		case 0x03:
            strcpy((char*)aucBuff+strlen((char*)aucBuff),"ë¾³Ö?");
			break;
		case 0x04:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"Ê±Ö¤");
			break;
		default://05
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"");
			break;
		
		}

        Os__GB2312_display11(1,0,aucBuff);
        Os__GB2312_display11(2,0,"1.IDÈÏÖ¤Í¨¹ý");
        Os__GB2312_display11(3,0,"2.IDÈÏÖ¤Ê§°Ü");

        emitSignal();



		
		while(1)
		{
            ucKey=Os__xget_key11();
			if(ucKey=='1')
			{
				ucResult=EMVERROR_SUCCESS;
				break;
			}
			else if(ucKey=='2')
			{
				ucResult=EMVERROR_CANCEL;
				break;
			}
			else
				continue;
		}

	
		
	}
	return ucResult;


}

UCHAR	UTIL_IssuerReferalProcess(void)
{

	UCHAR	aucPAN[20],ucI,ucKey,aucBuff[40];
	USHORT	uiPANLen;


	Os__light_on();
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiPANLen);
	CONV_HexAsc(aucBuff,aucPAN,uiPANLen*2);
	for(ucI=0;ucI<uiPANLen*2;ucI++)
	{
		if(aucBuff[+ucI]=='F')
		{
			aucBuff[+ucI] =0x00;
			break;
		}
	}
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,aucPAN);
    Os__GB2312_display11(1,0,(PUCHAR)"Call Your Bank");
    Os__GB2312_display11(2,0,(PUCHAR)"1.[ACCECPT TRANS]");
    Os__GB2312_display11(3,0,(PUCHAR)"2.[DECLINE TRANS]");
    emitSignal();

	do
	{
        ucKey=Os__xget_key11();
		if(ucKey=='1'||ucKey=='2'||ucKey==KEY_ENTER)
			break;
	}
	while(1);

	if(ucKey=='1'||ucKey=='2')
		return EMVERROR_SUCCESS;
	else 
		return EMVERROR_ISSUERREFERAL;

	
}
