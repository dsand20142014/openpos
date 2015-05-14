/************************************************************************************
 * ˾ɼ 
 * ļloadkey.c					
 * ----------------------------------------------------------------------------------
 * ܣÿĹҪʵֶA/BԿĹܵķװ
 *     
 * 
 *
 *
 * 2006/05/12         ļʵԿء
 * 2006/05/25         ˹شԿ
 *
 *
 ***********************************************************************************/




#include <loadkey.h>
#include <include.h>
#include <global.h>
#include <xdata.h>
#include	<new_drv.h>
//#include <rs232.h>

//#define OUTPUT_TEST
//#define USING_TEST_KEY

//Կ洢λ
unsigned char ucLoadKeyDevice;
unsigned char ucLoadKeyPinpadFlag;


ICC_ORDER 				LoadKey_IccOrder;
union ORDER_IN_TYPE 	LoadKey_OrderType;

LOADKEYERRINF   		LoadKeyErrInf;
LOADKEY_ABCARDDATA 		LoadKey_ABCardData;
LOADKEY_ISO7816IN		LoadKey_Iso7816In;
LOADKEY_ISO7816OUT  	LoadKey_Iso7816Out;

DRV_IN  LoadKey_DrvIn;
DRV_OUT	LoadKey_DrvOut;
DRIVER  LoadKey_Drv;

const unsigned char GlbEncryptKey[]=
#ifdef USING_TEST_KEY
	"\xC9\xBC\xB5\xC2\xCE\xA1\xBF\xB5\xBF\xA8\xC0\xEE\xCC\xEC\xB2\xA8";//Կ
#else
	"\xC9\xBC\xB5\xC2\xCE\xA1\xBF\xB5\xA3\xAD\xC0\xEE\xCC\xEC\xB2\xA8";//ʽԿ
#endif	

// chenzx add 2006-12-28 16:36	
const unsigned char GlbExtAuthKey1[]=
	"\xCE\xA1\xC9\xBC\xBF\xB5\xB5\xC2";
const unsigned char GlbExtAuthData1[]=
	"\xCE\xA1\xBF\xB5\xC9\xBC\xB5\xC2";
// end	

/**********************************************************************************
 *	
 *
 *
 **********************************************************************************/
unsigned char LOADKEY_LoadABKey(unsigned char ucDevice,
//								unsigned char ucPinpadFlag,
								unsigned char ucKeyArray,
								unsigned char ucKeyIndex,
								unsigned char ucStartAddr,
								unsigned char ucKeyNumber)
{
	unsigned char ucResult;
	unsigned char ucI,ucJ;
	unsigned char aucTempKey[9];
	
	
	//ʼ
	ucResult = LOADKEY_SUCCESS;
	ucI = ucJ = 0;
	memset(aucTempKey,0,sizeof(aucTempKey));
	
	//ָԿ洢
	//ȡֵ
	//DRV_MMI--Կ;
	//DRV_PAD--Կ
	if( (ucDevice == LOADKEY_DRV_MMI) || (ucDevice == LOADKEY_DRV_PAD) )
	{
		ucLoadKeyDevice = ucDevice;
	}
	else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"豸Ƿ!");
		Os__xget_key();
		return LOADKEY_ERR_CANCEL;
	}
	
	
	//ָʾǷPIN
	//ȡֵ
	//true--PIN; 
	//false--PIN.
#if 0	
	ucLoadKeyPinpadFlag = ucPinpadFlag;
#endif	
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"A!");
	
   	ucResult = LOADKEY_WaitReadCard();
   	
   	if(ucResult != LOADKEY_SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"Aϵʧ");
		return ucResult;
	}
	
	Uart_Printf("Read A Card");
	if( ( ucResult = LOADKEY_ISOReadCard('A') ) != LOADKEY_SUCCESS )
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"Aʧ");
		return ucResult;
	} 
	
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"γA!");
	Os__ICC_remove();
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"B");
	ucResult = LOADKEY_WaitReadCard();
	
	if( ucResult != LOADKEY_SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"Bϵʧ");
		return ucResult;
	}
	if( ( ucResult = LOADKEY_ISOReadCard('B') ) != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"Bʧ");
	   return ucResult;
	}
	
	//УԿ
	ucResult = LOADKEY_CheckKeyMac(ucKeyArray,ucKeyIndex,ucStartAddr,ucKeyNumber);
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char*)"ԿУʧ!");
	   return ucResult;
	}
	
	
	for(ucJ=0;ucJ<ucKeyNumber;ucJ++)
	{
		//洫KEY
		memset(aucTempKey,0,sizeof(aucTempKey));
		ucResult = LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,		
		    		          ucKeyIndex+ucJ*2,&LoadKey_ABCardData.ucRandom[ucStartAddr+ucJ][0],aucTempKey);
		if( ucResult != LOADKEY_SUCCESS )
		{
			return(ucResult);  
		}
		
		ucResult = LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(ucKeyArray,
						               ucKeyIndex+ucJ*2,
						       	       ucKeyIndex+ucJ*2+1,
						       	       aucTempKey);
	
		if (ucResult != LOADKEY_SUCCESS) 
		{
			return(ucResult);  
		}
		
		//*Կ
		
		for(ucI=0;ucI<2;ucI++)
		{			
			ucResult = LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(ucKeyArray,
						               ucKeyIndex+ucJ*2+1,
						       	       ucKeyIndex+ucJ*2+ucI,
						       	       &LoadKey_ABCardData.uc3DKey[ucStartAddr+ucJ][ucI*8]);	
			if (ucResult != LOADKEY_SUCCESS) 
			{
				return (ucResult);  
			}	       	  
		}
	 }	
	 
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"Կسɹ!");
	//Os__GB2312_display(2,0,(unsigned char*)"γB!");
	Os__ICC_remove();
	
	return(LOADKEY_SUCCESS);
}

unsigned char LOADKEY_CheckKeyMac(unsigned char ucKeyarray,
								  unsigned char ucKeyindex,
								  unsigned char ucStartAddr,
								  unsigned char ucKeyNumber)
{
	unsigned char ucResult;
	unsigned char ucI;
	unsigned char MAC[9];
	unsigned char aucTempKey[9];
	
	for(ucI=0;ucI<ucKeyNumber;ucI++)
	{
		memset(aucTempKey,0,sizeof(aucTempKey));
		ucResult = LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(ucKeyarray,
													ucKeyindex,
													&LoadKey_ABCardData.uc3DKey[ucStartAddr+ucI][0],
													aucTempKey);
		if( ucResult != LOADKEY_SUCCESS )
		{
			 return (ucResult);  
		}
		ucResult = LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(ucKeyarray,
						               					ucKeyindex,
						       	       					ucKeyindex,
						       	       					aucTempKey);	
		if( ucResult != LOADKEY_SUCCESS ) 
		{
		 return (ucResult);  
		}
		memset(MAC,0,sizeof(MAC));
		ucResult = LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(ucKeyarray,
													ucKeyindex,
													&LoadKey_ABCardData.ucRandom[ucStartAddr+ucI][0],
													MAC);
		if( ucResult != LOADKEY_SUCCESS )
		{
			 return (ucResult);  
		}
		
		if(memcmp(MAC,&LoadKey_ABCardData.ucMAC[ucStartAddr+ucI][0],8))
		{
			return ucI;
		}
	}
	return(LOADKEY_SUCCESS);
}	
/*
unsigned char LOADKEY_WaitReadCard(void)
{
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	unsigned char icc_ATR[100];
	unsigned char ucLen,ucResult;

	ucResult = LOADKEY_SUCCESS;	
	pxIcc = Os__ICC_insert();
	pxKey = Os__get_key();
	while( (pxIcc->gen_status!=DRV_ENDED)
		 &&(pxKey->gen_status!=DRV_ENDED) );
	if( pxIcc->gen_status == DRV_ENDED )	  
	{
		Os__abort_drv(drv_mmi);
		Os__xdelay(10);
		ucLen = pxIcc->xxdata[0];
		memcpy(icc_ATR,pxIcc,ucLen+4);
		
		//if(icc_ATR[4] != ICC_ASY)
		//{
		//	ucResult = ERR_POWERFAIL;
		//}	
	}
	else
	{
		ucResult = LOADKEY_ERR_CANCEL;
		Os__abort_drv(drv_icc);
		Os__xdelay(10);
		Os__xdelay(1);
	}
	return(ucResult);		
}
*/
unsigned int LOADKEY_WaitReadCard(void)
{
	unsigned int ret;            
	unsigned int uiI=0;
	unsigned char ucKey;
	unsigned char aucReq[8],aucCardId[16];
	unsigned char ucLen,ucCardClass;
	unsigned char aucRdInfo[64];
	unsigned char aucWrtInfo[64];
	unsigned char aucData[16];
	NEW_DRV_TYPE  new_drv;                                                    
	unsigned char icc_ATR[300];
	unsigned char  length1,ucResult;

	while(1)
	{
		Uart_Printf("\n1. Os_Wait_Event begin...");
		ret = Os_Wait_Event(KEY_DRV  | ICC_DRV , &new_drv, 5000);
		//Uart_Printf("\n2. Os_Wait_Event:%02x",ret);
		//Uart_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ret == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					ucKey=new_drv.drv_data.xxdata[1];
					Uart_Printf("\n ucKey:%02x.", ucKey);
					if(ucKey==KEY_CLEAR)
					{
						return ERR_CANCEL;
					}
				}
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if( new_drv.drv_data.gen_status == DRV_ENDED )
				{
					length1 = new_drv.drv_data.xxdata[0];
                    memcpy(icc_ATR,(uchar*)&new_drv.drv_data,length1+4);
					/*if(icc_ATR[4]==ICC_ASY)
					{
						RunData.ucInputmode=1;
						NormalTransData.ucCardType=CARD_M1;
					}*/
					break;
				}
			}   
			Uart_Printf("\n OWE_NewDrvtts End.....\n");
		}                                                                         
		else                                                                      
		{                                                                         
			Untouch_OWE_RstErrInfo(ret);                                               
		}                                                                         
	}
	Uart_Printf("\n OWE_NewDrvtts rest:%02x.\n",ret);
	return 0;
}	

unsigned char LOADKEY_ISOReadCard(unsigned char ucCardType)
{
	unsigned char  ucResult;
	unsigned char  ucI;
	unsigned char  ucLen;
	unsigned char  ucLen1;
	unsigned char  ucFileNameLen;
	unsigned char  aucFileName[15];
	unsigned char  aucPin[7];
	unsigned char  aucTransKey[8];
	unsigned char  aucMasterKey[16];
	unsigned char  aucMKeyCheck[8];
	unsigned char  aucTempKey[8];
	unsigned char  aucPrintBuf[40];
	unsigned char  aucPinData[4];
	
	ucResult = LOADKEY_SUCCESS;
	memset(aucFileName,0,sizeof(aucFileName));
	memcpy(aucFileName,"1PAY.SYS.DDF01",14);
	ucFileNameLen = 14;

	ucResult = LOADKEY_ISOSelectFile(ICC1,aucFileName,ucFileNameLen);
	if( ucResult != LOADKEY_SUCCESS )
	{
		Uart_Printf("select mf err");
		return(ucResult);
	}	
	
	if( LOADKEY_ISOReadBinarybySFI(ICC1,0x04,0x00,0x2) != LOADKEY_SUCCESS )
	{
		Uart_Printf("select 04 err");
		return(ucResult);
	}
	if( LoadKey_Iso7816Out.aucData[0] != ucCardType)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"Ͳ");
		Os__xdelay(5);
		return(ucResult);
	}
	
	ucResult = LOADKEY_SUCCESS;
	memset(aucPin,0,sizeof(aucPin));		
	memset(aucPinData,0,sizeof(aucPinData));
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"뿨");
#if 0	
	if( ucLoadKeyPinpadFlag  == true )
	{
		//Os__clr_display_pp(255);
		//Os__GB2312_display_pp(0,0,(unsigned char *)"뿨");
		if( LOADKEY_UTIL_Input_pp(3,true,6,6,'P',aucPin)  == KEY_ENTER )
		{		
		  	memset(aucPinData,0,sizeof(aucPinData));
		  	asc_bcd(aucPinData,3,aucPin,6);
		  	ucLen = 3;	  	
		}		
		else
		{
			ucResult = LOADKEY_ERR_NOPIN;
			LoadKeyErrInf.ErrType = LOADKEY_ERR_CANCEL;
			return(ucResult);
		}
	}
	else
#endif			
	{
		if( LOADKEY_UTIL_Input(1,true,6,6,'P',aucPin,NULL) == KEY_ENTER )
		{		
		  	memset(aucPinData,0,sizeof(aucPinData));
		  	asc_bcd(aucPinData,3,aucPin,6);
		  	ucLen = 3;	  	
		}		
		else
		{
			ucResult = LOADKEY_ERR_NOPIN;
			LoadKeyErrInf.ErrType = LOADKEY_ERR_CANCEL;
			return(ucResult);
		}
	}

	ucResult = LOADKEY_VerifyPin(ICC1,ucLen,aucPinData);
	if( ucResult != LOADKEY_SUCCESS )
	{
		Uart_Printf("verify pin err");
		return(ucResult);
	}
	if(ucCardType == 'A')
	{
		ucLen =2+16*7;   //242
		if( ( ucResult = LOADKEY_ISOReadBinarybySFI(ICC1,0x05,0x00,ucLen) ) !=  OK )
		{
			Uart_Printf("read 0005 err%02x",ucResult);
			return(ucResult);
		}	
		memcpy(&LoadKey_ABCardData.iKeyNumber,&LoadKey_Iso7816Out.aucData[0],ucLen);
		ucLen1 =16*8;
		if( ( ucResult = LOADKEY_ISOReadBinarybySFI(ICC1,0x05,ucLen,ucLen1) ) != OK)
		{
			Uart_Printf("read 0005 err%02x",ucResult);
			return(ucResult);
		}
		memcpy(&LoadKey_ABCardData.uc3DKey[7][0],&LoadKey_Iso7816Out.aucData[0],ucLen1);	
	}
	else
	{
		ucLen = 8*15;    //240
		if( ( ucResult = LOADKEY_ISOReadBinarybySFI(ICC1,0x05,0x00,ucLen) ) != OK)
		{
			Uart_Printf("read 0005 err%02x",ucResult);
			return(ucResult);
		}	
		memcpy(&LoadKey_ABCardData.ucRandom[0][0],&LoadKey_Iso7816Out.aucData[0],ucLen);
		ucLen1 = 8*15;
		if( ( ucResult = LOADKEY_ISOReadBinarybySFI(ICC1,0x05,ucLen,ucLen1) ) != OK)
		{
			Uart_Printf("read 0005 err%02x",ucResult);
			return(ucResult);
		}	
		memcpy(&LoadKey_ABCardData.ucMAC[0][0],&LoadKey_Iso7816Out.aucData[0],ucLen1);
	}
	
	Uart_Printf("ucLen=%02x\n",ucLen);	
	if(ucCardType == 'A')
	{
		{
			unsigned char ucI,disp_buf[33];
			for(ucI=0;ucI<LoadKey_ABCardData.iKeyNumber;ucI++)
			{
				memset(disp_buf,0,sizeof(disp_buf));
				hex_asc(disp_buf,&LoadKey_ABCardData.uc3DKey[ucI][0],32);
				Uart_Printf("%s\n",disp_buf);
			}
		}	 
	}
	else
	{
	 	{
			unsigned char ucI,disp_buf[33];
			for(ucI=0;ucI<LoadKey_ABCardData.iKeyNumber;ucI++)
			{
				memset(disp_buf,0,sizeof(disp_buf));
				hex_asc(disp_buf,&LoadKey_ABCardData.ucRandom[ucI][0],16);
				hex_asc(&disp_buf[16],&LoadKey_ABCardData.ucMAC[ucI][0],16);
				Uart_Printf("%s\n",disp_buf);
			}
		}
	}		
	
	return(ucResult);
} 		


unsigned char LOADKEY_ISOReadBinarybySFI(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen)
{
	unsigned char	ucResult;

	LoadKey_Iso7816In.ucCla = 0x00;
	LoadKey_Iso7816In.ucIns = 0xB0;
	LoadKey_Iso7816In.ucP1  = ucSFI | 0x80;
	LoadKey_Iso7816In.ucP2  = ucOffset;
	LoadKey_Iso7816In.ucP3  = ucInLen;

	LOADKEY_SetIso7816Out();
	if( (ucResult = LOADKEY_SMART_ISO(ucReader,
						LOADKEY_SMART_ASYNC,
						(unsigned char *)&LoadKey_Iso7816In,
						LoadKey_Iso7816Out.aucData,
						&LoadKey_Iso7816Out.uiLen,
						LOADKEY_TYPE2)) != OK)
		return(ucResult);
	
	return(LOADKEY_ISOCheckReturn(ucReader));	
}	


unsigned char LOADKEY_ISOCheckReturn(unsigned char ucReader)
{
	
	unsigned char ucResult;	
	unsigned char ucI;	
		
	LoadKey_Iso7816Out.ucSW1 = LoadKey_Iso7816Out.aucData[LoadKey_Iso7816Out.uiLen-2];
	LoadKey_Iso7816Out.ucSW2 = LoadKey_Iso7816Out.aucData[LoadKey_Iso7816Out.uiLen-1];
#ifdef OSDEBUG_ENABLE
	for(ucI=0;ucI<LoadKey_Iso7816Out.uiLen;ucI++)
		Uart_Printf("%02x,",LoadKey_Iso7816Out.aucData[ucI]);
	Uart_Printf("\nucSW1=%02x,ucSW2=%02x\n",LoadKey_Iso7816Out.ucSW1,LoadKey_Iso7816Out.ucSW2);	
#endif
    LoadKeyErrInf.PBOCErrCode = LoadKey_Iso7816Out.ucSW1*0x100+LoadKey_Iso7816Out.ucSW2;
    LoadKeyErrInf.ErrType = LOADKEY_ERR_LOAD;
	ucResult = LoadKeyErrInf.ErrType;
	
	switch(LoadKey_Iso7816Out.ucSW1)
	{
	case 0x90:
		return(LOADKEY_SMART_OK);
	case 0x6C:
		LOADKEY_SetIso7816Out();
		LoadKey_Iso7816In.ucP3  = LoadKey_Iso7816Out.ucSW2; 
		if( (ucResult = LOADKEY_SMART_ISO(ucReader,
						LOADKEY_SMART_ASYNC,
						(unsigned char *)&LoadKey_Iso7816In,
						LoadKey_Iso7816Out.aucData,
						&LoadKey_Iso7816Out.uiLen,
						LOADKEY_TYPE2)) != OK)
		{
			 Uart_Printf("hehe9\n");
			return(ucResult);
		}	
		return(LOADKEY_ISOCheckReturn(ucReader));
	case 0x61:
	case 0x9F:
		Uart_Printf("hehe11\n");
		return(LOADKEY_ISOGetResponse(ucReader,LoadKey_Iso7816Out.ucSW2));
	case 0x63:
		if(  (LoadKey_Iso7816Out.ucSW2 >= 0xC0)
		   &&(LoadKey_Iso7816Out.ucSW2 <= 0xCF)
		  )
		{
			Uart_Printf("hehe12\n");
			return(LOADKEY_SMART_CARD_ERROR);
		}
				  Uart_Printf("hehe13\n");
		return(LOADKEY_CheckCardErr(LoadKey_Iso7816Out.ucSW1,LoadKey_Iso7816Out.ucSW2));
	default:
		Uart_Printf("hehe14\n");
		return(LOADKEY_CheckCardErr(LoadKey_Iso7816Out.ucSW1,LoadKey_Iso7816Out.ucSW2));			
	}
}	


unsigned char LOADKEY_ISOGetResponse(unsigned char ucReader,
			unsigned char ucLen)
{
	unsigned char ucResult;

	LoadKey_Iso7816In.ucCla = 0x00;
	LoadKey_Iso7816In.ucIns = 0xC0;
	LoadKey_Iso7816In.ucP1  = 0x00;
	LoadKey_Iso7816In.ucP2  = 0x00;
	LoadKey_Iso7816In.ucP3  = ucLen;

	LOADKEY_SetIso7816Out();
	if( (ucResult = LOADKEY_SMART_ISO(ucReader,
						LOADKEY_SMART_ASYNC,
						(unsigned char *)&LoadKey_Iso7816In,
						LoadKey_Iso7816Out.aucData,
						&LoadKey_Iso7816Out.uiLen,
						LOADKEY_TYPE2)) != OK)
		return(ucResult);

    return(LOADKEY_ISOCheckReturn(ucReader));
}	


unsigned char LOADKEY_ISOSelectFile(unsigned char ucReader,unsigned char *pucFileName,unsigned char ucFileNameLen)
{
	unsigned char	ucResult;

	LoadKey_Iso7816In.ucCla = 0x00;
	LoadKey_Iso7816In.ucIns = 0xA4;
	LoadKey_Iso7816In.ucP1  = 0x04;;
	LoadKey_Iso7816In.ucP2  = 0x00;
	LoadKey_Iso7816In.ucP3  = ucFileNameLen;
	memcpy(&LoadKey_Iso7816In.aucDataIn[0],pucFileName,ucFileNameLen);
    
	LOADKEY_SetIso7816Out();
	Uart_Printf("LoadKey_Iso7816Out.uiLen=%d\n",LoadKey_Iso7816Out.uiLen);
	if( (ucResult = LOADKEY_SMART_ISO(ucReader,
						LOADKEY_SMART_ASYNC,
						(unsigned char *)&LoadKey_Iso7816In,
						LoadKey_Iso7816Out.aucData,
						&LoadKey_Iso7816Out.uiLen,
						LOADKEY_TYPE3)) != OK)
	{
//		Uart_Printf("PBOC ERROR");		
		return(ucResult);
	}
	Uart_Printf("LoadKey_Iso7816Out.uiLen22=%d\n",LoadKey_Iso7816Out.uiLen);
	return(LOADKEY_ISOCheckReturn(ucReader));
}

void LOADKEY_SetIso7816Out(void)
{
	LoadKey_Iso7816Out.uiLen = sizeof(LoadKey_Iso7816Out.aucData);
	memset(LoadKey_Iso7816Out.aucData,0,LoadKey_Iso7816Out.uiLen);
}

unsigned char LOADKEY_VerifyPin(unsigned char ucReader,unsigned char ucLen,unsigned char *Pin_Data)
{
	unsigned char ucResult;
		
	LoadKey_Iso7816In.ucCla = 0x00;
	LoadKey_Iso7816In.ucIns = 0x20;
	LoadKey_Iso7816In.ucP1  = 0x00;
	LoadKey_Iso7816In.ucP2  = 0x00;
	LoadKey_Iso7816In.ucP3  = ucLen; 

	memcpy(&LoadKey_Iso7816In.aucDataIn[0],Pin_Data,ucLen);
	LoadKey_Iso7816In.aucDataIn[LoadKey_Iso7816In.ucP3] = 0x00;
	
	LOADKEY_SetIso7816Out();
	if( (ucResult = LOADKEY_SMART_ISO(ucReader,
						LOADKEY_SMART_ASYNC,
						(unsigned char *)&LoadKey_Iso7816In,
						LoadKey_Iso7816Out.aucData,
						&LoadKey_Iso7816Out.uiLen,
						LOADKEY_TYPE3)) != OK)
		return(ucResult);
		
    return(LOADKEY_ISOCheckReturn(ucReader));
}

unsigned char LOADKEY_UTIL_Input_pp(unsigned char ucLine,unsigned char ucClrFlag,
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
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
	uiTimeout = 50*LOADKEY_ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
		//Os__clr_display_pp(ucLine);
	}	
	//Os__clr_display_pp(1); 
   	//Os__GB2312_display_pp(0,0,(unsigned char *)" 뿨");
	
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);			
	}
	Os__timer_start(&uiTimeout);

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
			//Os__clr_display_pp(ucLine);
			Os__display_pp(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
		Uart_Printf("\nOs__get_key_pp");
		pdKey = Os__get_key_pp();

		do{
		}while(  (pdKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_pad);//drv_mmi
			Os__xdelay(10);
			ucKey = LOADKEY_ERR_CANCEL;
			break;
		}else
		{
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
					ucKey = LOADKEY_ERR_CANCEL;
					ucEnd = TRUE;
				}
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

	Os__timer_stop(&uiTimeout);
	for(uiTimeout=0;uiTimeout<20000;uiTimeout++);
	//Os__clr_display_pp(255);
	//Os__GB2312_display_pp(1,0,(unsigned char *)"   ڴ");
	//Os__GB2312_display_pp(2,0,(unsigned char *)"   Ժ.....");			
	return(ucKey);
}
unsigned char LOADKEY_UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
	const unsigned char aucKeyTab[][7]
	={	{"0 *,\0"},
		{"1#\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PQRS\0"},
		{"8TUV\0"},
		{"9WXYZ\0"}
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
	DRV_OUT *pdKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
	uiTimeout = 50*LOADKEY_ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
		//Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);			
	}
	Os__timer_start(&uiTimeout);

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
					memset(&aucDisp[0],'*',ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				
			}
			//Os__clr_display(ucLine);
			Os__display(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
		
		pdKey = Os__get_key();

		do{
		}while(  (pdKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			Os__xdelay(10);
			ucKey = LOADKEY_ERR_CANCEL;
			break;
		}else
		{
			ucKey = pdKey->xxdata[1];
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
				if( ucCount > ucMax)
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
                    if(  (uiLastTime-uiTimeout) < 1*LOADKEY_ONESECOND)
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
				if( ucCount > ucMax)
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

					if(  (uiLastTime-uiTimeout) < 1*LOADKEY_ONESECOND)
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
					ucKey = LOADKEY_ERR_CANCEL;
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
					if( ucCount < strlen((char *)&aucInput[0]))
						ucCount ++;
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

	Os__timer_stop(&uiTimeout);			
	return(ucKey);
}

unsigned char LOADKEY_SMART_ISO(
			unsigned char ucReader,
			unsigned char ucCardType,
			unsigned char* pucIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut,
            unsigned char ucOrderType)
{
	unsigned char ucLength;
	unsigned short uiAdd;

	// chenzx modify 2006-5-11 17:22
	union AS_ORDER_TYPE* 	pLoadKeyAsType;
	SYNC_ORDER_TYPE* 		pLoadKeySyncOrder;
	ICC_ANSWER* 			pLoadKeyAnswer;
	
	
	/* Order to execute */
	LoadKey_IccOrder.ptout = pucOut;
	LoadKey_IccOrder.pt_order_in = &LoadKey_OrderType;
	if ( ucCardType == LOADKEY_SMART_ASYNC )
    {
	    LoadKey_IccOrder.order = ASYNC_ORDER;
	    LoadKey_IccOrder.pt_order_in->async_order.order_type = ucOrderType;
	    pLoadKeyAsType = &LoadKey_IccOrder.pt_order_in->async_order.as_order;
	    LoadKey_IccOrder.pt_order_in->async_order.NAD = 0x00;
	    switch ( ucOrderType )
	    {
	    case LOADKEY_TYPE0:
	        pLoadKeyAsType->order_type0.Lc = *pucIn;       /* Length of Data transmitted */
	        pLoadKeyAsType->order_type0.ptin = pucIn+1;    /* Data transmitted           */
	        break;
	    case LOADKEY_TYPE1:
	        pLoadKeyAsType->order_type1.CLA = *pucIn;
	        pLoadKeyAsType->order_type1.INS = *(pucIn+1);
	        pLoadKeyAsType->order_type1.P1  = *(pucIn+2);
	        pLoadKeyAsType->order_type1.P2  = *(pucIn+3);
	        break;
	    case LOADKEY_TYPE2:
	        pLoadKeyAsType->order_type2.CLA = *pucIn;
	        pLoadKeyAsType->order_type2.INS = *(pucIn+1);
	        pLoadKeyAsType->order_type2.P1  = *(pucIn+2);
	        pLoadKeyAsType->order_type2.P2  = *(pucIn+3);
	        pLoadKeyAsType->order_type2.Le  = *(pucIn+4);  /* Length of data received   */
	        break;
	    case LOADKEY_TYPE3:
	        pLoadKeyAsType->order_type3.CLA = *pucIn;
	        pLoadKeyAsType->order_type3.INS = *(pucIn+1);
	        pLoadKeyAsType->order_type3.P1  = *(pucIn+2);
	        pLoadKeyAsType->order_type3.P2  = *(pucIn+3);
	        pLoadKeyAsType->order_type3.Lc  = *(pucIn+4);  /* Length of data transmitted*/
	        pLoadKeyAsType->order_type3.ptin = pucIn+5;    /* Data transmitted          */
	        break;
	    case LOADKEY_TYPE4:
	        pLoadKeyAsType->order_type4.CLA = *pucIn;
	        pLoadKeyAsType->order_type4.INS = *(pucIn+1);
	        pLoadKeyAsType->order_type4.P1  = *(pucIn+2);
	        pLoadKeyAsType->order_type4.P2  = *(pucIn+3);
	        pLoadKeyAsType->order_type4.Lc  = *(pucIn+4);  /* Length of data transmitted*/
	        pLoadKeyAsType->order_type4.ptin = pucIn+5;    /* Data transmitted          */
	                                                /* Length of data received   */
	        pLoadKeyAsType->order_type4.Le  = *(pucIn+5+ *(pucIn+4));
	        break;
	    default:
	        return (LOADKEY_SMART_CARD_ERROR);
	    }
	}else
    {
	    LoadKey_IccOrder.order = SYNC_ORDER;
	    pLoadKeySyncOrder = &LoadKey_IccOrder.pt_order_in->sync_order;
	
	    switch ( *pucIn )                               /* Address            */
	    {
	    case 0x05:                                      /* SLE4418 ou SLE4428 */
	    case 0x06:                                      /* SLE4432 ou SLE4442 */
	        uiAdd = (*(pucIn+2) << 8) | (*(pucIn+3) & 0x00FF);
	        uiAdd *= 8;                                 /* Convert to bits    */
	        pLoadKeySyncOrder->ADDH = uiAdd >> 8;             /* P1                 */
	        pLoadKeySyncOrder->ADDL = uiAdd & 0x00FF;         /* P2                 */
	        break;
	    default:                                        /* Other cards        */
	        pLoadKeySyncOrder->ADDH = *(pucIn+2);             /* P1                 */
	        pLoadKeySyncOrder->ADDL = *(pucIn+3);             /* P2                 */
	        break;
	    }
	
	    switch ( *pucIn )                               /* Data lenght        */
	    {
	    case 0x04:                                      /* GFM2K              */
	    case 0x05:                                      /* SLE4418 ou SLE4428 */
	    case 0x06:                                      /* SLE4432 ou SLE4442 */
	        ucLength = *(pucIn+4);
	        ucLength *= 8;                              /* Length * 8         */
	        break;
	    default:                                        /* Other cards        */
	        ucLength = *(pucIn+4);
	        break;
	    }
	
	    switch ( ucOrderType )
	    {
	    case LOADKEY_TYPE2:
	        pLoadKeySyncOrder->card_type = *pucIn;     /* Synchronous Type           */
	        pLoadKeySyncOrder->INS = *(pucIn+1);       /* INS                        */
	        /* P1                         */
	        /* P2                         */
	        pLoadKeySyncOrder->Len = ucLength;         /* Length of data received    */
	        break;
	    case LOADKEY_TYPE3:
	        pLoadKeySyncOrder->card_type = *pucIn;     /* Synchronous Type           */
	        pLoadKeySyncOrder->INS = *(pucIn+1);       /* INS                        */
	        /* P1                         */
	        /* P2                         */
	        pLoadKeySyncOrder->Len = ucLength;         /* Length of data transmitted */
	        pLoadKeySyncOrder->ptin = pucIn+4;         /* Data transmitted           */
	        break;
	    default:
	        return (LOADKEY_SMART_CARD_ERROR);
	    }
	}

	/* Send Command */
	pLoadKeyAnswer = Os__ICC_command (ucReader, &LoadKey_IccOrder);

	/* Answer Command */
	/******************/
	if ( pLoadKeyAnswer->drv_status != OK )
    {
    	return (LOADKEY_SMART_DRIVER_ERROR); 
    }
	if ( pLoadKeyAnswer->card_status != ASY_OK )
	{
		return (LOADKEY_SMART_CARD_ERROR); 
	}

	if ( pLoadKeyAnswer->Len > *puiLengthOut )
	{
		return (LOADKEY_SMART_OWERFLOW);
	}
	*puiLengthOut = pLoadKeyAnswer->Len;
	return (LOADKEY_SMART_OK);
}

unsigned char LOADKEY_CheckCardErr(unsigned char Sw1,unsigned char Sw2)
{
	unsigned short ErrTab;
	unsigned ucResult;
	
	ErrTab = Sw1*0x100+Sw2;
	
	switch(ErrTab)
	{
		case 0x6982:
			ucResult = LOADKEY_ERR_NOTSAFT;
			break;
		case 0x6A82:
			ucResult = LOADKEY_ERR_FILENOTFOUND;
			break;
		case 0x6A83:
			ucResult = LOADKEY_ERR_RECORDNOTFOUND;
			break;
		case 0x6A81:
			ucResult = LOADKEY_ERR_NOTUSEFUNC;
			break;
		case 0x9401:
			ucResult = LOADKEY_ERR_LESSBALANCE;
			break;
		case 0x9402:
			ucResult = LOADKEY_ERR_FULLCOUNT;
			break;
		case 0x9403:
			ucResult = LOADKEY_ERR_NOKEYINDEX;
			break;
		case 0x9302:
			ucResult = LOADKEY_ERR_MACERROR;
			break;
		case 0x9303:
			ucResult = LOADKEY_ERR_CARDLOCK;
			break;
		case 0x6985:
			ucResult = LOADKEY_ERR_CASENOTFULL;
			break;
		case 0x63c1:
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"ʣһУ");
			Os__xdelay(100);
			break;
		case 0x63c2:
		 	//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"ʣУ");
			Os__xdelay(100);
			break;
		default:
			ucResult = LOADKEY_ERR_KNOWERROR;
			break;
	}
	
	LoadKeyErrInf.PBOCErrCode=ErrTab;
//	Uart_Printf("\nucpPErrorCode=%x\n",LOADErrInf.PBOCErrCode);
	LoadKeyErrInf.ErrType  = LOADKEY_ERR_LOAD;
	ucResult = LoadKeyErrInf.ErrType;
	return(ucResult);
}



unsigned char LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData)
{
	LoadKey_DrvIn.type      = 0x42;
	LoadKey_DrvIn.length	= 19;
	LoadKey_DrvIn.xxdata[0] = ucKeyArray+0x30;
	LoadKey_DrvIn.xxdata[1] = ucDecryptKeyIndex+0x30;
	LoadKey_DrvIn.xxdata[2] = ucDestKeyIndex+0x30;
	
	hex_asc(&LoadKey_DrvIn.xxdata[3],pucInData, 16);
	if( LOADKEY_PINPAD_SendReceivePinpad() == LOADKEY_SUCCESS)
	{
		if( !LoadKey_DrvOut.drv_status )
		{
			if( LoadKey_DrvOut.xxdata[0] == 0x31 )
				return(LOADKEY_ERR_DRIVER);
			else
				return(LOADKEY_SUCCESS);
		}		
	}
	return(LOADKEY_ERR_DRIVER);
}

unsigned char LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	LoadKey_DrvIn.type      = 0x47;
	LoadKey_DrvIn.length	= 18;
	LoadKey_DrvIn.xxdata[0] = ucKeyArray+0x30;
	LoadKey_DrvIn.xxdata[1] = ucKeyIndex+0x30;
	
	hex_asc(&LoadKey_DrvIn.xxdata[2],pucInData, 16);
	if( LOADKEY_PINPAD_SendReceivePinpad() == LOADKEY_SUCCESS)
	{
		if( !LoadKey_DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&LoadKey_DrvOut.xxdata[1],16); 
/*
#ifdef TEST
	Uart_Printf("DrvOut.xxdata=%s\n",LoadKey_DrvOut.xxdata);
#endif	
*/
			return(LOADKEY_SUCCESS);
		}		
	}
	return(LOADKEY_ERR_DRIVER);
}


unsigned char LOADKEY_PINPAD_47_Crypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	LoadKey_DrvIn.type      = 0x3F;
	LoadKey_DrvIn.length	= 18;
	LoadKey_DrvIn.xxdata[0] = ucKeyArray+0x30;
	LoadKey_DrvIn.xxdata[1] = ucKeyIndex+0x30;
	
	hex_asc(&LoadKey_DrvIn.xxdata[2],pucInData, 16);
	if( LOADKEY_PINPAD_SendReceivePinpad() == LOADKEY_SUCCESS)
	{
		if( !LoadKey_DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&LoadKey_DrvOut.xxdata[1],16); 
/*
#ifdef TEST
	Uart_Printf("DrvOut.xxdata=%s\n",DrvOut.xxdata);
#endif	
*/
			return(LOADKEY_SUCCESS);
		}		
	}
	return(LOADKEY_ERR_DRIVER);
}

unsigned char LOADKEY_PINPAD_SendReceivePinpad(void)
{
	//if( ucLoadKeyDevice == LOADKEY_DRV_PAD )
	{
	//	LoadKey_Drv.drv_nr = drv_pad;
	}
	//else
	{
		LoadKey_Drv.drv_nr = drv_mmi;
	}
	
	LoadKey_Drv.type = execute;

	memset(&LoadKey_DrvOut, 0, sizeof(DRV_OUT));
	LoadKey_Drv.pt_drv_in = &LoadKey_DrvIn;
	LoadKey_Drv.pt_drv_out = &LoadKey_DrvOut;
	Os__call_drv(&LoadKey_Drv);
	do{
	}while( LoadKey_DrvOut.gen_status == DRV_RUNNING );
	if(  ( LoadKey_DrvOut.gen_status == DRV_ABSENT )
       ||(  ( LoadKey_DrvOut.gen_status == DRV_ENDED ) 
          &&( LoadKey_DrvOut.drv_status == DRV_ABSENT ) 
         )
      )
	{
/*
#ifdef TEST
		Uart_Printf("DrvOut.drv_status=%02x\n",LoadKey_DrvOut.drv_status);
#endif
*/
		return(LOADKEY_ERR_DRIVER);
	}else
	{
		return(LOADKEY_SUCCESS);
	}
}


unsigned char LOADKEY_LoadTransKey(unsigned char ucDevice,
                                   unsigned char ucKeyarray,
                                   unsigned char ucKeyindex)
{
	unsigned char ucResult;
	unsigned char aucKeyData[40];
	
	//ʼ
	ucResult = LOADKEY_SUCCESS;
	
	//ָԿ洢
	//ȡֵ
	//DRV_MMI--Կ;
	//DRV_PAD--Կ
	if( (ucDevice == LOADKEY_DRV_MMI) || (ucDevice == LOADKEY_DRV_PAD) )
	{
		ucLoadKeyDevice = ucDevice;
	}
	else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"豸Ƿ!");
		Os__xget_key();
		return LOADKEY_ERR_CANCEL;
	}
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"崫Կ");
    
    ucResult = LOADKEY_WaitReadCard();
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ϵʧ");
	   return ucResult;
	}
	Uart_Printf("read  card");
	memset(aucKeyData,0,sizeof(aucKeyData));
	ucResult = LOADKEY_ReadTransKeyCard(aucKeyData);
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ʧ");
	   return ucResult;
	} 
#if 0	
    {
       	unsigned char i,buf[100];
    	for(i=0;i<32;i++)
    	{
    	   Uart_Printf("%02x",aucKeyData[i]);
    	   if(i%10 == 0x00)
    	   {
    	   	Uart_Printf("\n");
    	   }
    	}
	}
#endif	
	ucResult = LOADKEY_LoadingKey(aucKeyData,ucKeyarray,ucKeyindex);
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ʧ");
	   return ucResult;
	} 
	return LOADKEY_SUCCESS;
}	

unsigned char LOADKEY_ReadTransKeyCard(unsigned char *pucOutData )
{
	unsigned char  ucResult,ucI,ucLen,ucLen1;
	unsigned char  filename[15],filenamelen;
	unsigned char  aucPin[7];
	unsigned char  trans_key[8],master_key[16],mkey_check[8],temp_key[8];
	unsigned char  aucPrintBuf[40];
	unsigned char  aucPinData[4];
	
	
	ucResult = LOADKEY_SUCCESS;
	memset(filename,0,sizeof(filename));
	memcpy(filename,"1PAY.SYS.DDF01",14);
	filenamelen = 14;

	ucResult = LOADKEY_ISOSelectFile(ICC1,filename,filenamelen);
	if( ucResult != LOADKEY_SUCCESS)
	{
		Uart_Printf("select mf err");
		return(ucResult);
	}	
	
	//Os__clr_display(255);
	
	//Os__GB2312_display(0,0,(unsigned char *)"뿨");
	
	memset(aucPin,0,sizeof(aucPin));		
	memset(aucPinData,0,sizeof(aucPinData));
	

   if( LOADKEY_UTIL_Input(1,true,6,6,'P',aucPin,NULL) == KEY_ENTER )
	{		
	  	memset(aucPinData,0,sizeof(aucPinData));
	  	asc_bcd(aucPinData,3,aucPin,6);
	  	ucLen = 3;	  	
	}		
	else
	{
		ucResult = LOADKEY_ERR_NOPIN;
		LoadKeyErrInf.ErrType = LOADKEY_ERR_CANCEL;
		return(ucResult);
	}
	ucResult = LOADKEY_VerifyPin(ICC1,ucLen, aucPinData);
  	if( ucResult != LOADKEY_SUCCESS )
    {
        Uart_Printf("verify pin err%02x",ucResult);
        return LOADKEY_SUCCESS;
    }
    ucLen = 32;
    if( (ucResult = LOADKEY_ISOReadBinarybySFI(ICC1,0x06,0x00,ucLen)) != OK )
	{
		Uart_Printf("read 0006 err%02x",ucResult);
		return(ucResult);
	}	
	memcpy(pucOutData,&LoadKey_Iso7816Out.aucData[0],ucLen);
	
	return LOADKEY_SUCCESS;
}

unsigned char LOADKEY_LoadingKey(unsigned char *pucInData,
                              unsigned char ucKeyarray,
                              unsigned char ucKeyindex)
{
	unsigned char ucResult;
	unsigned char aucRandomKey[9],aucKeyData[17],aucMac[9];
	unsigned char aucTempKey[9];
	
	memset(aucRandomKey,0,sizeof(aucRandomKey));
	memcpy(aucRandomKey,pucInData,8);
	memset(aucKeyData,0,sizeof(aucKeyData));
	memcpy(aucKeyData,&pucInData[8],16);
	memset(aucMac,0,sizeof(aucMac));
	memcpy(aucMac,&pucInData[24],8);
	
	ucResult = LOADKEY_SUCCESS;
	/*洫Կ*/
	memset(aucTempKey,0,sizeof(aucTempKey));
	ucResult = LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(ucKeyarray,     
						ucKeyindex,aucRandomKey,aucTempKey);
	if( ucResult != LOADKEY_SUCCESS )
	{
		 return (ucResult);  
	}
	ucResult = LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(ucKeyarray,
					               ucKeyindex,
					       	       ucKeyindex+1,
					       	       aucTempKey);	
	if( ucResult != LOADKEY_SUCCESS ) 
	{
	    return (ucResult);  
	}
	/*Կ*/
	#if 1	
    {
       	unsigned char i;
       	Uart_Printf("aucKeyData Step0:");
    	for(i=0;i<16;i++)
    	{
    	   Uart_Printf("%02x",aucKeyData[i]);
    	}
    	Uart_Printf("\n");
	}
	#endif
	ucResult = LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(ucKeyarray,
					               ucKeyindex+1,
					       	       ucKeyindex,
					       	       aucKeyData);	
	if( ucResult != LOADKEY_SUCCESS ) 
	{
	    return (ucResult);  
	}
	ucResult = LOADKEY_PINPAD_42_LoadSingleKeyUseSingleKey(ucKeyarray,
					               ucKeyindex+1,
					       	       ucKeyindex+1,
					       	       &aucKeyData[8]);	
	if( ucResult != LOADKEY_SUCCESS ) 
	{
	    return (ucResult);  
	}
	/*УԿ*/
	memset(aucTempKey,0,sizeof(aucTempKey));
	ucResult = LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(ucKeyarray, 
							ucKeyindex,
							aucRandomKey,
							aucTempKey);
	if( ucResult != LOADKEY_SUCCESS )
	{
		 return (ucResult);  
	}
	#if 1	
    {
       	unsigned char i;
       	Uart_Printf("aucTempKey Step1:");
    	for(i=0;i<8;i++)
    	{
    	   Uart_Printf("%02x",aucTempKey[i]);
    	}
    	Uart_Printf("\n");
	}
	#endif
	
	ucResult = LOADKEY_PINPAD_47_Crypt8ByteSingleKey(ucKeyarray,
					              ucKeyindex+1,
					              aucTempKey,
					              aucTempKey);
	
	if( ucResult != LOADKEY_SUCCESS ) 
	{
	    return (ucResult);  
	}
	
	#if 1	
    {
       	unsigned char i;
       	Uart_Printf("aucTempKey Step2:");
    	for(i=0;i<8;i++)
    	{
    	   Uart_Printf("%02x",aucTempKey[i]);
    	}
    	Uart_Printf("\n");
	}
	#endif
	 
	ucResult = LOADKEY_PINPAD_47_Encrypt8ByteSingleKey(ucKeyarray,	
							ucKeyindex,
							aucTempKey,
							aucTempKey);
	if( ucResult != LOADKEY_SUCCESS ) 
	{
	    return (ucResult);  
	}
	
	#if 1	
    {
       	unsigned char i;
       	Uart_Printf("aucTempKey Step3:");
    	for(i=0;i<8;i++)
    	{
    	   Uart_Printf("%02x",aucTempKey[i]);
    	}
    	Uart_Printf("\n");
	}
	#endif
	
	#if 1	
    {
       	unsigned char i;
       	Uart_Printf("aucTempKey:");
    	for(i=0;i<8;i++)
    	{
    	   Uart_Printf("%02x",aucTempKey[i]);
    	}
    	Uart_Printf("\n");
    	Uart_Printf("aucMac:");
    	for(i=0;i<8;i++)
    	{
    	   Uart_Printf("%02x",aucMac[i]);
    	}
    	Uart_Printf("\n");
	}
	#endif	

	if( memcmp(aucTempKey,aucMac,8) != 0 )
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,"ԿУʧ!!");
	MSG_WaitKey(3);
        return  2;
	}
	return LOADKEY_SUCCESS;
} 

//chenzx add 2006-7-4 17:11
//ΡشԿ贫
unsigned char LOADKEY_LoadWeiKangTransKey(unsigned char ucDevice,
                                   unsigned char ucKeyarray,
                                   unsigned char ucKeyindex,
                                   unsigned char ucArrayNum)
{
	unsigned char ucResult;
	unsigned char ucI;
	unsigned char aucKeyData[40];
	unsigned char aucBuf[50],aucBufout[50];
	int iLen;
	
	//ʼ
	ucResult = LOADKEY_SUCCESS;
	
	//ָԿ洢
	//ȡֵ
	//DRV_MMI--Կ;
	//DRV_PAD--Կ
	if( (ucDevice == LOADKEY_DRV_MMI) || (ucDevice == LOADKEY_DRV_PAD) )
	{
		ucLoadKeyDevice = ucDevice;
	}
	else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"豸Ƿ!");
		Os__xget_key();
		return LOADKEY_ERR_CANCEL;
	}
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"崫Կ");
    
    ucResult = LOADKEY_WaitReadCard();
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ϵʧ");
	   return ucResult;
	}
	Uart_Printf("read  card");
	memset(aucKeyData,0,sizeof(aucKeyData));
	ucResult = LOADKEY_ReadTransKeyCard(aucKeyData);
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ʧ");
	   return ucResult;
	} 
#if 1	
    {
       	unsigned char i,buf[100];
    	for(i=0;i<32;i++)
    	{
    	   Uart_Printf("%02x",aucKeyData[i]);
    	   if(i%10 == 0x00)
    	   {
    	   	Uart_Printf("\n");
    	   }
    	}
	}
#endif	
    if( ucArrayNum > 0 )
    {
        /*
        switch(ucDevice)
        {
        case LOADKEY_DRV_MMI:
            if( (ucKeyarray + ucArrayNum) > 5 )
            {
            }
            break;
        case LOADKEY_DRV_PAD:
            if( (ucKeyarray + ucArrayNum) > 6 )
            {
            }
            break;
        }*/
        for(ucI=0;ucI<ucArrayNum;ucI++)
        {
            ucResult = LOADKEY_LoadingKey(aucKeyData,ucKeyarray+ucI,ucKeyindex);
            if( ucResult != LOADKEY_SUCCESS )
            {
                break;
            }
        }
       /*    
	if(DataSave0.ConstantParamData.ucComType)
	{
		rs232_InitCOM();
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,"\x00\x24",2);
		memcpy(&aucBuf[2],aucKeyData,24);
		iLen=rs232_PackData_Send(aucBuf,aucBufout);
		ucResult = rs232_SendData(aucBufout, iLen);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"ͳɹ!");
		//Os__GB2312_display(1,0,(unsigned char*)"װؽն");
		//Os__GB2312_display(2,0,(unsigned char*)"γ!");
		Os__ICC_remove();
	}
	else
	{
	        for(ucI=0;ucI<ucArrayNum;ucI++)
	        {
	            ucResult = LOADKEY_LoadingKey(aucKeyData,ucKeyarray+ucI,ucKeyindex);
	            if( ucResult != LOADKEY_SUCCESS )
	            {
	                break;
	            }
	        }
        }*/
    }
    else
    {
        ucResult = LOADKEY_ERR_CANCEL;
    }
	
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ʧ");
	   return ucResult;
	} 
	return LOADKEY_SUCCESS;
}

#if 0
unsigned char LOADKEY_LoadWeiKangTransKeyEx(void)
{
	unsigned char ucResult = LOADKEY_SUCCESS;
	unsigned char aucKeyData[40];
	unsigned char aucTransKey[17];
	unsigned char aucSend[256];
	unsigned char aucReceive[256];
	unsigned char aucExtAuthData[9];
	unsigned int  uiSendLen;
	unsigned int  uiRevLen;
	unsigned int  uiTimesOut;
	unsigned char ucStatus;

	
	
	memset(aucKeyData,0,sizeof(aucKeyData));
	memset(aucTransKey,0,sizeof(aucTransKey));
	
	memset(aucSend,0,sizeof(aucSend));
	memset(aucReceive,0,sizeof(aucReceive));
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"崫Կ");
    
    ucResult = LOADKEY_WaitReadCard();
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ϵʧ");
	   return ucResult;
	}
	Uart_Printf("read  card");
	memset(aucKeyData,0,sizeof(aucKeyData));
	ucResult = LOADKEY_ReadTransKeyCard(aucKeyData);
	if( ucResult != LOADKEY_SUCCESS )
	{
	   //Os__clr_display(255);
	   //Os__GB2312_display(0,0,(unsigned char *)"ʧ");
	   return ucResult;
	} 
#ifdef OUTPUT_TEST
    {
       	unsigned char i,buf[100];
    	for(i=0;i<32;i++)
    	{
    		if(i%8 == 0x00)
    	    {
    	   		Uart_Printf("\n");
    	    }
    	    Uart_Printf("%02x",aucKeyData[i]);
    	   
    	}
	}
#endif	
	// chenzx add 2006-12-28 16:29
	// Ӽ֤
	memset(aucExtAuthData,0,sizeof(aucExtAuthData));
	des(GlbExtAuthData1,aucExtAuthData,GlbExtAuthKey1);
#ifdef OUTPUT_TEST
    {
       	unsigned char i;
    	for(i=0;i<8;i++)
    	{
    		if(i%8 == 0x00)
    	    {
    	   		Uart_Printf("\n");
    	    }
    	    Uart_Printf("%02x",aucExtAuthData[i]);
    	   
    	}
	}
#endif
	// end
	ucResult = LOADKEY_GetTransKey(aucKeyData,aucTransKey);
	if( ucResult == LOADKEY_SUCCESS )
	{
		RS232_SetProtocol(0x02,2,2,0x03,1);
		ucResult = RS232_InitComm();
		Uart_Printf("RS232_InitComm Out ucResult = %02x\n",ucResult);
		if( ucResult == RS232_SUCCESS )
		{
			//Os__clr_display(255);
    		//Os__GB2312_display(0,0,(unsigned char *)"ݷ...");
			aucSend[0] = 0x21;
			memcpy(&aucSend[1],aucExtAuthData,4);
			memcpy(&aucSend[5],aucTransKey,16);
			memcpy(&aucSend[21],&aucExtAuthData[4],4);
			uiSendLen = 1 + 4 + 16 + 4;
#ifdef OUTPUT_TEST
    {
       	unsigned char i;
    	for(i=0;i<uiSendLen;i++)
    	{
    		if(i%8 == 0x00)
    	    {
    	   		Uart_Printf("\n");
    	    }
    	    Uart_Printf("%02x",aucSend[i]);
    	   
    	}
	}
#endif			
			ucResult = RS232_Send(aucSend,uiSendLen);
			Uart_Printf("RS232_Send Out ucResult = %02x\n",ucResult);
		}
		
		if( ucResult == RS232_SUCCESS )
		{
			//Os__clr_display(255);
    		//Os__GB2312_display(0,0,(unsigned char *)"ݽ...");
			uiTimesOut = 60*100;
			Os__timer_start(&uiTimesOut);
			do{
				ucStatus = RS232_WaitForReceive();
			}while( ( ucStatus != RS232_OK )
					&&(uiTimesOut) );
			Os__timer_stop(&uiTimesOut);
			if( uiTimesOut == 0 )
			{
				//Os__clr_display(255);
    			//Os__GB2312_display(0,0,(unsigned char *)"ճʱ");
    			//Os__GB2312_display(2,0,(unsigned char*)"γ!");
				Os__ICC_remove();
    			return RS232_ERR_CANCEL;
			}
			else if( ucStatus == RS232_OK )
			{
				uiRevLen = 0;
				ucResult = RS232_Receive(aucReceive,&uiRevLen);
				Uart_Printf("RS232_Receive Out ucResult = %02x\n",ucResult);
				if( ucResult == RS232_SUCCESS )
				{
					if( aucReceive[0] != 0x21 )
					{
						RS232_DisplayMsg(RS232_ERR_COM_PROTOCOL);
						return RS232_ERR_CANCEL;
					}
					if( aucReceive[1] != 0x00 )
					{
						//Os__clr_display(255);
		    			//Os__GB2312_display(0,0,(unsigned char *)"ײ㺯ʧ");
		    			//Os__GB2312_display(2,0,(unsigned char*)"γ!");
						Os__ICC_remove();
		    			return RS232_ERR_CANCEL;
					}
					if( aucReceive[2] != 0x00 )
					{
						//Os__clr_display(255);
		    			//Os__GB2312_display(0,0,(unsigned char *)"ʧ");
		    			//Os__GB2312_display(2,0,(unsigned char*)"γ!");
						Os__ICC_remove();
		    			return RS232_ERR_CANCEL;
					}
				}
			}
				
		}
		
		if( ucResult != RS232_SUCCESS )
		{
			RS232_DisplayMsg(ucResult);
			//Os__GB2312_display(2,0,(unsigned char*)"γ!");
			Os__ICC_remove();
			return RS232_ERR_CANCEL;
		}
	}
	else
	{
		//Os__clr_display(255);
	    //Os__GB2312_display(0,0,(unsigned char *)"ʧ");
	    //Os__GB2312_display(2,0,(unsigned char*)"γ!");
		Os__ICC_remove();
	    return ucResult;
	}
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"Կسɹ!");
	//Os__GB2312_display(2,0,(unsigned char*)"γ!");
	Os__ICC_remove();
	return LOADKEY_SUCCESS;
}
#endif

unsigned char LOADKEY_GetTransKey(unsigned char *pucKeyData,unsigned char *pucTransKey)
{
	unsigned char aucRandomKey[9];
	unsigned char aucKeyA[9];
	unsigned char aucKeyB[9];
	unsigned char aucMacKey[9];
	unsigned char aucTempKey[9];
	int i;
	
	memset(aucRandomKey,0,sizeof(aucRandomKey));
	memset(aucKeyA,0,sizeof(aucKeyA));
	memset(aucKeyB,0,sizeof(aucKeyB));
	memset(aucMacKey,0,sizeof(aucMacKey));
	memset(aucTempKey,0,sizeof(aucTempKey));
	
	memcpy(aucRandomKey,pucKeyData,8);
	memcpy(aucMacKey,&pucKeyData[24],8);
	
	memcpy(aucTempKey,&pucKeyData[8],8);
	desm1(aucTempKey,aucKeyA,aucRandomKey);
	
#ifdef OUTPUT_TEST	
	Uart_Printf("\n");
	for(i=0;i<8;i++)
	{
		Uart_Printf("%02x",aucKeyA[i]);
	}
	Uart_Printf("\n");
#endif	

	memcpy(aucTempKey,&pucKeyData[16],8);
	desm1(aucTempKey,aucKeyB,aucRandomKey);
	
#ifdef OUTPUT_TEST	
	Uart_Printf("\n");
	for(i=0;i<8;i++)
	{
		Uart_Printf("%02x",aucKeyB[i]);
	}
	Uart_Printf("\n");
#endif
	
	memset(aucTempKey,0,sizeof(aucTempKey));
	des(aucRandomKey,aucTempKey,aucKeyA);
	desm1(aucTempKey,aucTempKey,aucKeyB);
	des(aucTempKey,aucTempKey,aucKeyA);
	
	if( memcmp(aucTempKey,aucMacKey,8) != 0 )
	{
		//Os__clr_display(255);
        //Os__GB2312_display(0,0,"ԿУʧ");
        //Os__GB2312_display(2,0,(unsigned char*)"γ!");
        Os__ICC_remove();
        return  2;
	}
	
	des(aucKeyA,pucTransKey,(uchar *)GlbEncryptKey);
#ifdef OUTPUT_TEST	
	Uart_Printf("\n");
	for(i=0;i<8;i++)
	{
		Uart_Printf("%02x",pucTransKey[i]);
	}
	Uart_Printf("\n");
#endif
	des(aucKeyB,&pucTransKey[8],(uchar *)&GlbEncryptKey[8]);
#ifdef OUTPUT_TEST	
	Uart_Printf("\n");
	for(i=0;i<8;i++)
	{
		Uart_Printf("%02x",pucTransKey[i+8]);
	}
	Uart_Printf("\n");
#endif

	return LOADKEY_SUCCESS;
}



unsigned char DownProtectKey(unsigned char ucKeyarray,unsigned char ucKeyindex,unsigned char ucArrayNum)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucI;
	unsigned char aucKeyData[40],ucChar;
	unsigned char aucBuf[50],aucBufout[50];
	unsigned int iLen,uiTimeout;
	
	DRV_OUT *pxKey;

	rs232_InitCOM();
	ucChar=0;
	uiTimeout = 60*100;
	do
	{
		pxKey = Os__get_key();
	//	pxKey = OSMMI_GetKey();
		ucResult = rs232_ComRecvByte(&ucChar,100);
	}
	while(pxKey->xxdata[1]!=KEY_CLEAR&&ucChar!=STX_CHAR);
	if(!uiTimeout)
		return ERR_CANCEL;
	if(!ucResult)
	{
		OSDRV_Abort(drv_mmi);
		while(ucResult == OK)
		{
			if(ucChar == STX_CHAR)
				break;
			else
			      Os__xdelay(50);
		      ucResult = rs232_ComRecvByte(&ucChar,1);
		}
		if(ucResult == SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult = rs232_InceptData(aucBuf);
		}
		else if(ucResult == ERR_COMMS_RECVTIMEOUT)
		{
			ucResult = ERR_COMMS_PROTOCOL;
		}		
		//ն
	      /*  while(1)
	   	{
	   		if(rs232_ComRecvByte(&ucChar,10) != SUCCESS)
	   			break;
	   	}*/
		if(ucResult == SUCCESS)
		{
			for(ucI=0;ucI<ucArrayNum;ucI++)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"װԿ");	
				ucResult = LOADKEY_LoadingKey(&aucBuf[2],ucKeyarray+ucI,ucKeyindex);
				if( ucResult != SUCCESS )
				{
					break;
				}
			}
		}
		if(ucResult != SUCCESS)
		{
		        //Os__clr_display(255);
		        //Os__GB2312_display(0,0,"Կʧ!!");
			MSG_WaitKey(3);
		}
		else
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char*)"Կسɹ");
			MSG_WaitKey(3);
		}
		return ucResult;
	}
	return ucResult;
}




