/* 
	SAND China
--------------------------------------------------------------------------
	FILE  pinpad.c									(Copyright SAND 2001)       
--------------------------------------------------------------------------
    INTRODUCTION          
    ============                                             
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network             
    Purpose :                               
        Source file for pinpad interface. 
                                                             
    List of routines in file :                                            
                                                                          
    File history :                                                        
                                                                          
*/

#include <include.h>
#include <global.h>

unsigned char PINPAD_0D_InitScraCode(unsigned char *pucInData)
{
	DrvIn.type      = 0x0d;
	DrvIn.length	= 16;

	hex_asc(&DrvIn.xxdata[0], pucInData, 16);

	return(SendReceivePinpad());
}

unsigned char PINPAD_30_BackLight(unsigned char ucBackLight)
{
	DrvIn.type      = 0x30;
	DrvIn.length	= 1;

	if( ucBackLight )
	{
		DrvIn.xxdata[0] = 0x31;
	}else
	{
		DrvIn.xxdata[0] = 0x30;
	}	
	return(SendReceivePinpad());
}

unsigned char PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData)
{
	DrvIn.type      = 0x42;
	DrvIn.length	= 19;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucDecryptKeyIndex+0x30;
	DrvIn.xxdata[2] = ucDestKeyIndex+0x30;
	
	hex_asc(&DrvIn.xxdata[3],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			if( DrvOut.xxdata[0] == 0x31 )
				return(ERR_DRIVER);
			else
				return(SUCCESS);
		}		
	}
	return(ERR_DRIVER);
}

unsigned char PINPAD_46_LoadKey(
					unsigned char ucKeyArray,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData)
{
	int i;
	DrvIn.type      = 0x46;
	DrvIn.length	= 43;
	DrvIn.xxdata[0] = 0x30;
	DrvIn.xxdata[1] = 0x00;
	DrvIn.xxdata[2] = APP_ID;
	DrvIn.xxdata[3] = 0xFF;
	DrvIn.xxdata[4] = 0xFF;
	DrvIn.xxdata[5] = 0xFF;
	DrvIn.xxdata[6] = 0xFF;
	DrvIn.xxdata[7] = 0x32;
	DrvIn.xxdata[8] = 0x30;
	DrvIn.xxdata[9] = ucKeyArray+'0';
	DrvIn.xxdata[10] = ucDestKeyIndex/2+'0';
	
	hex_asc(&DrvIn.xxdata[11],pucInData, 32);
	Uart_Printf("DrvIn.xxdata:\n");
	for(i=0;i<50;i++)
		{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
	if( SendReceivePinpad() == SUCCESS)
	{
	Uart_Printf("DrvOut.xxdata:\n");
	for(i=0;i<50;i++)
		{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		if( !DrvOut.drv_status )
		{
			if( DrvOut.xxdata[0] == 0x31 )
				return(ERR_DRIVER);
			else
				return(SUCCESS);
		}		
	}
	return(ERR_DRIVER);
}
/*************系统DES加密*****************/
unsigned char PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	DrvIn.type      = 0x47;
	DrvIn.length	= 18;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucKeyIndex+0x30;
	
	hex_asc(&DrvIn.xxdata[2],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&DrvOut.xxdata[1],16); 
/*
#ifdef TEST
	Uart_Printf("DrvOut.xxdata=%s\n",DrvOut.xxdata);
#endif	
*/
			return(SUCCESS);
		}		
	}
	return(ERR_DRIVER);
}
/*************系统DES解密*****************/
unsigned char PINPAD_47_Crypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData)
{
	DrvIn.type      = 0x3F;
	DrvIn.length	= 18;
	DrvIn.xxdata[0] = ucKeyArray+0x30;
	DrvIn.xxdata[1] = ucKeyIndex+0x30;
	
	hex_asc(&DrvIn.xxdata[2],pucInData, 16);
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			asc_hex(pucOutData,8,&DrvOut.xxdata[1],16); 
/*
#ifdef TEST
	Uart_Printf("DrvOut.xxdata=%s\n",DrvOut.xxdata);
#endif	
*/
			return(SUCCESS);
		}		
	}
	return(ERR_DRIVER);
}

unsigned char PINPAD_4E_PINInput(
					unsigned char ucRow,unsigned char ucCol,
					unsigned char ucMin,unsigned char ucMax,
					unsigned char ucDispChar,
					unsigned char ucKeyArray,unsigned char ucKeyIndex,
					unsigned char *pucOutKeyNb,
					unsigned char *pucOutData)
{
	DrvIn.type      = 0x4e;
	DrvIn.length	= 9;
	DrvIn.xxdata[0] = ucRow+0x30;
	DrvIn.xxdata[1] = ucCol+0x30;
	DrvIn.xxdata[2] = ucMin+0x30;
	DrvIn.xxdata[3] = ucMax+0x30;
	DrvIn.xxdata[4] = 0x30;
	DrvIn.xxdata[5] = ucKeyArray+0x30;
	DrvIn.xxdata[6] = 0x30;
	DrvIn.xxdata[7] = ucDispChar;
	DrvIn.xxdata[8] = ucKeyIndex+0x30;
	
	if( SendReceivePinpad() == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			//xxdata[1] is number of entered keys
			*pucOutKeyNb = DrvOut.xxdata[1] - '0';
			asc_hex(pucOutData,8,&DrvOut.xxdata[2],16); 
			return(SUCCESS);
		}		
	}
	return(ERR_DRIVER);
}


unsigned char SendReceivePinpad(void)
{
uchar uRst=0;
	Drv.drv_nr = drv_mmi;
	Drv.type = execute;

	memset(&DrvOut,0,sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	Os__call_drv(&Drv);
	do{
	}while( DrvOut.gen_status == DRV_RUNNING );
	
	
	if(  (DrvOut.gen_status == DRV_ABSENT)
       ||(  (DrvOut.gen_status == DRV_ENDED)
          &&(DrvOut.drv_status == DRV_ABSENT)))
		uRst =ERR_DRIVER;
	else
		uRst=OK;

	printf(" \n\n\n 	SendReceivePinpad:%02x\n", uRst);

	return uRst;
}

unsigned char PP_In_Store_Key(
					unsigned char ucRow,
					unsigned char ucCol,
					unsigned char ucKeyIndex,
					unsigned char *aucBuf)

{
    unsigned ucResult;

    ucResult = SUCCESS;

	DrvIn.type = 0x44;
	DrvIn.length = 35;


    DrvIn.xxdata[0]  = (ucRow     + 0x30);
    DrvIn.xxdata[1]  = (ucCol     + 0x30);
    DrvIn.xxdata[2]  = (ucKeyIndex   + 0x30);

	bcd_asc(&DrvIn.xxdata[3], aucBuf, (DrvIn.length-3));

	if( SendReceivePinpad() == SUCCESS)
    {
		if(DrvIn.xxdata[0])
	    	return(DrvIn.xxdata[1]);
    }
    return(ucResult);
}

uchar	Single_Encrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar * Pt_crypt)
{
	uchar	ucResult;

    ucResult = OK;
			
	DrvIn.type   = 0x47;
	DrvIn.length = 18;
	DrvIn.xxdata[0] = (Array    + 0x30);
	DrvIn.xxdata[1] = (Key_used + 0x30);
	hex_asc(&DrvIn.xxdata[2],Pt_data,16);

	if((ucResult = SendReceivePinpad()) == OK)
	{
		if(!DrvOut.drv_status)
		{
			asc_hex(Pt_crypt,8,&DrvOut.xxdata[1],16);
			return(OK);
		}
	}
	return(ERR_DRIVER);
}

uchar	Single_Decrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar *Pt_crypt)
{
	uchar	ucResult;

    ucResult = OK;
			
	DrvIn.type   = 0x3F;
	DrvIn.length = 18;
	DrvIn.xxdata[0] = (Array    + 0x30);
	DrvIn.xxdata[1] = (Key_used + 0x30);
	hex_asc(&DrvIn.xxdata[2],Pt_data,16);

	if((ucResult = SendReceivePinpad()) == OK)
	{
		if(!DrvOut.drv_status)
		{
			asc_hex(Pt_crypt,8,&DrvOut.xxdata[1],16);
			return(OK);
		}
	}
	return(ERR_DRIVER);
}

uchar	Single_Store_Key(uchar Array, uchar Key_used, uchar Key_stored, uchar *Pt_data)
{
	uchar	ucResult;

    ucResult = OK;

	DrvIn.type   = 0x42;
	DrvIn.length = 19;
    DrvIn.xxdata[0] = (Array      + 0x30);
    DrvIn.xxdata[1] = (Key_used   + 0x30);
    DrvIn.xxdata[2] = (Key_stored + 0x30);
	hex_asc(&DrvIn.xxdata[3],Pt_data,(DrvIn.length-3));
	
	if((ucResult = SendReceivePinpad()) == OK)
    {
		if(!DrvOut.drv_status)
		{
			if(DrvOut.xxdata[0] == 0x31)
				return(ERR_DRIVER);
			else
				return(OK);
		}
    }
    return(ERR_DRIVER);
}

uchar	Double_Encrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar *Pt_crypt)
{
	uchar	Temp1[8],Temp2[8],Temp3[8];

	memset(Temp1, 0, sizeof(Temp1));
	memset(Temp2, 0, sizeof(Temp2));
	memset(Temp3, 0, sizeof(Temp3));

    if(Single_Encrypt_Data(Array, Key_used, Pt_data, Temp1) != OK)
		return(ERR_DRIVER);
	if(Single_Decrypt_Data(Array, Key_used+1, Temp1, Temp2) != OK)
		return(ERR_DRIVER);
    if(Single_Encrypt_Data(Array, Key_used, Temp2, Temp3) != OK)
		return(ERR_DRIVER);
	memcpy(&Pt_crypt[0], Temp3, 8);

	return(OK);
}

uchar	Double_Decrypt_Data(uchar Array, uchar Key_used, uchar *Pt_data, uchar *Pt_crypt)
{
	uchar	Temp1[8],Temp2[8],Temp3[8];

	memset(Temp1, 0, sizeof(Temp1));
	memset(Temp2, 0, sizeof(Temp2));
	memset(Temp3, 0, sizeof(Temp3));

	if(Single_Decrypt_Data(Array, Key_used, Pt_data, Temp1) != OK)
		return(ERR_DRIVER);
    if(Single_Encrypt_Data(Array, Key_used+1, Temp1, Temp2) != OK)
		return(ERR_DRIVER);
	if(Single_Decrypt_Data(Array, Key_used, Temp2, Temp3) != OK)
		return(ERR_DRIVER);
	memcpy(&Pt_crypt[0], Temp3, 8);

	return(OK);
}

uchar	Double_Store_Key(uchar Array, uchar Key_used, uchar Key_stored, uchar DataType, uchar *Key_data)
{
	uchar	Temp1[8],Temp2[8];
	uchar	Temp3[8],Temp4[8];

	memset(Temp1,0,sizeof(Temp1));
	memset(Temp2,0,sizeof(Temp2));
	
	if(DataType == 0)
	{
    	if(  (Single_Encrypt_Data(Array, Key_used, &Key_data[0], Temp1)!= OK)
	   	   ||(Single_Store_Key(Array, Key_used, Key_stored, Temp1) != OK))
	  		return(ERR_DRIVER);
    	if(  (Single_Encrypt_Data(Array, Key_used+1, &Key_data[8], Temp2)!= OK)
	   	   ||(Single_Store_Key(Array, Key_used+1, Key_stored+1, Temp2) != OK))
	  		return(ERR_DRIVER);
	}else
	{
		memset(Temp3,0,sizeof(Temp3));
		memset(Temp4,0,sizeof(Temp4));
        
        if(Double_Decrypt_Data(Array, Key_used, &Key_data[0], Temp1)!= OK)
	  		return(ERR_DRIVER);
        if(Double_Decrypt_Data(Array, Key_used, &Key_data[8], Temp2)!= OK)
	  		return(ERR_DRIVER);
    	if(  (Single_Encrypt_Data(Array, Key_used, &Temp1[0], Temp3)!= OK)
	   	   ||(Single_Store_Key(Array, Key_used, Key_stored, Temp3) != OK))
	  		return(ERR_DRIVER);
    	if(  (Single_Encrypt_Data(Array, Key_used+1, &Temp2[0], Temp4)!= OK)
	   	   ||(Single_Store_Key(Array, Key_used+1, Key_stored+1, Temp4) != OK))
	  		return(ERR_DRIVER);
	}  		
	return(OK);
}

uchar	PINPAD_45_LoadKey(uchar *pucInData)
{
	DrvIn.type      = 0x45;
	DrvIn.length	= 33;
	DrvIn.xxdata[0] = 0x30;

	hex_asc(&DrvIn.xxdata[1],pucInData,32);

	if(SendReceivePinpad() == OK)
	{
		Uart_Printf("DrvOut.drv_status=%02x\n",DrvOut.drv_status);
		if(!DrvOut.drv_status)
		{
			Uart_Printf("DrvOut.xxdata[0]=%02x\n",DrvOut.xxdata[0]);
			if(DrvOut.xxdata[0] == 0x31)
				return(ERR_DRIVER);
			else
				return(OK);
		}		
	}
	return(ERR_DRIVER);
}

/*****************************************************************
功能:保护密钥解密存传输密钥
参数:
	1.ucDecryptTransmitKeyType  	解密传输密钥方式
	2.uiStoringAppNum               保存传输密钥应用号
	3.uiDecryptTransKeyAppNum  		指定解密传输密钥应用号
	4.pucDecryptKeyArray           	解密密钥行列号
	5.ucDecryptTye                  解密方式
	6.ucProctKeyIndex               保护密钥索引
	7.pucStoringArray               存密钥行列号
	8.pucInData                     待解密传输密钥
******************************************************************/
uchar	PINPAD_46_LoadKey1(uchar ucDecryptTransmitKeyType,uint  uiStoringAppNum,
						  uint  uiDecryptTransKeyAppNum, uchar *pucDecryptKeyArray,
						  uchar ucDecryptTye,            uchar ucProctKeyIndex,
					      uchar *pucStoringArray,		 uchar *pucInData)
{
	uint	i,uiI;
	uint 	uiOffset;

	uiOffset = 0;
	DrvIn.type = 0x46;
	
	DrvIn.xxdata[uiOffset] = ucDecryptTransmitKeyType+0x30;
	uiOffset++;
	int_tab(&DrvIn.xxdata[uiOffset],2,&uiStoringAppNum);
	uiOffset +=2;
	int_tab(&DrvIn.xxdata[uiOffset],2,&uiDecryptTransKeyAppNum);
	uiOffset +=2;
	if(pucDecryptKeyArray[0] == 0xFF)
		DrvIn.xxdata[uiOffset] = pucDecryptKeyArray[0];
	else	
		DrvIn.xxdata[uiOffset] = pucDecryptKeyArray[0]+0x30;
	uiOffset++;
	if(pucDecryptKeyArray[1] == 0xFF)
		DrvIn.xxdata[uiOffset] = pucDecryptKeyArray[1];
	else
		DrvIn.xxdata[uiOffset] = pucDecryptKeyArray[1]+0x30;
	uiOffset++;
	DrvIn.xxdata[uiOffset] = ucDecryptTye;
	uiOffset++;
	DrvIn.xxdata[uiOffset] = ucProctKeyIndex;
	uiOffset++;
	if(pucStoringArray[0] == 0xFF)
		DrvIn.xxdata[uiOffset] = pucStoringArray[0];
	else
		DrvIn.xxdata[uiOffset] = pucStoringArray[0]+0x30;
	uiOffset++;
	if(pucStoringArray[0] == 0xFF)
		DrvIn.xxdata[uiOffset] = pucStoringArray[1];
	else
		DrvIn.xxdata[uiOffset] = pucStoringArray[1]+0x30;
	uiOffset++;
#ifdef PS100
	hex_asc(&DrvIn.xxdata[uiOffset],pucInData,32);
	uiOffset +=32;
#else
	hex_asc(&DrvIn.xxdata[uiOffset],pucInData,16);
	uiOffset +=16;
#endif	
	DrvIn.length = uiOffset;

	if(SendReceivePinpad() == OK)
	{
		if( !DrvOut.drv_status )
		{
			if( DrvOut.xxdata[0] == 0x31 )
				return(ERR_DRIVER);
			else
				return(OK);
		}	
	}
	return(ERR_DRIVER);
}
unsigned char Load_Default_Keys(void)
{
    unsigned char ucResult;
    unsigned char ucKey[17];
	
	printf("\n.1.Load_Default_Keys----------------------------------\n");
	memset(ucKey,0,17);
	memcpy(ucKey,(uchar *)"\x01\x23\x45\x67\x89\xAB\xCD\xEF\xFE\xDC\xBA\x98\x76\x54\x32\x10",16);
	ucResult = PINPAD_45_LoadKey(ucKey);
	printf("\n.1.Load_Default_Keys result----------------------------------[%02x]\n",ucResult);

	
    return(OK);
}
unsigned char	Load_Default_Keys_Check(void)
{
	unsigned char ucResult;
	static	uchar  buf[17],tbuf[17],sbuf[17];
	static	uchar  Array[3],DecryArray[3];
	uchar  Buffer[9],Sbuf[9],Temp[20];

	printf("\n\n\n.1.Load_Default_Keys_Check----------------------------------\n");

	memset(tbuf,0,17);
	memcpy(tbuf,(uchar *)"\x9B\x3A\x7B\x88\x3A\x10\x0F\x73",8);
	memset(Array,0,sizeof(Array));
	memset(DecryArray,0,sizeof(DecryArray));
	Array[0] = 0x00;DecryArray[0] = 0xFF;
	Array[1] = 0x00;DecryArray[1] = 0xFF;

	ucResult = PINPAD_46_LoadKey1(0,APP_ID,0xFF,DecryArray,0x31,0x30,Array,tbuf);
    Uart_Printf("PINPAD_46_LoadKey1 === %02x\n",ucResult);
	if(ucResult == SUCCESS)
	{
		memset(tbuf,0,17);
		memset(sbuf,0,17);
		if(Single_Encrypt_Data(0,0,tbuf,sbuf)!= OK)
		return(ERR_DRIVER);
		hex_str(tbuf,&sbuf[0],16);
		Os__xprint(tbuf);
		printf("22222222222222::::::%d   |          %s@@@@@@@@@@@@@@@@@@@@@@@@@@\n",tbuf,tbuf);

		memset(tbuf,0,17);
		memcpy(tbuf,(uchar *)"\xB4\xAB\xA2\xBB\x79\x1C\x50\xE7",8);
		memset(Array,0,sizeof(Array));
		memset(DecryArray,0,sizeof(DecryArray));
		Array[0] = 0x00;DecryArray[0] = 0xFF;
		Array[1] = 0x01;DecryArray[1] = 0xFF;
		printf("+++++++++++1\n");
		if(PINPAD_46_LoadKey1(0,APP_ID,0xFF,DecryArray,0x31,0x30,Array,tbuf)!= OK)
		return(ERR_DRIVER);
		printf("+++++++++++2\n");
		memset(tbuf,0,17);
		memset(sbuf,0,17);
		if(Single_Encrypt_Data(0,1,tbuf,sbuf)!= OK)
		return(ERR_DRIVER);
		printf("+++++++++++3\n");
		hex_str(tbuf,&sbuf[0],16);
		Os__xprint(tbuf);
		printf("33333333333:::::::::::;%d   |          %s@@@@@@@@@@@@@@@@@@@@@@@@@@\n",tbuf,tbuf);

		return(OK);
	}else
	{
		memset(tbuf,0,17);memset(Buffer,0,sizeof(Buffer));
		memcpy(tbuf,(uchar *)"\x9B\x3A\x7B\x88\x3A\x10\x0F\x73",8);
		if(Double_Decrypt_Data(15,36,tbuf,Buffer)!= OK)
		return(ERR_DRIVER);

		memset(Sbuf,0,sizeof(Sbuf));
		if(Single_Encrypt_Data(15,0,Buffer,Sbuf)!= OK)
		return(ERR_DRIVER);
		if(Single_Store_Key(15,0,0,Sbuf)!= OK)
		return(ERR_DRIVER);

		memset(tbuf,0x00,8);memset(sbuf,0x00,8);
		if(Single_Encrypt_Data(15,0,tbuf,sbuf)!= OK)
		return(ERR_DRIVER);
		memset(tbuf,0,16);
		hex_str(tbuf,&sbuf[0],16);
		Os__xprint(tbuf);
		printf("00000000000%x@@@@@@@@@@@@@@@@@@@@@@@@@@",tbuf);

		memset(tbuf,0,17);memset(Buffer,0,sizeof(Buffer));
		memcpy(tbuf,(uchar *)"\xB4\xAB\xA2\xBB\x79\x1C\x50\xE7",8);
		if(Double_Decrypt_Data(15,36,tbuf,Buffer)!= OK)
		return(ERR_DRIVER);

		memset(Sbuf,0,sizeof(Sbuf));
		if(Single_Encrypt_Data(15,1,Buffer,Sbuf)!= OK)
		return(ERR_DRIVER);
		if(Single_Store_Key(15,1,1,Sbuf)!= OK)
		return(ERR_DRIVER);

		memset(tbuf,0x00,8);memset(sbuf,0x00,8);
		if(Single_Encrypt_Data(15,1,tbuf,sbuf)!= OK)
		return(ERR_DRIVER);
		memset(tbuf,0,16);
		hex_str(tbuf,&sbuf[0],16);
		Os__xprint(tbuf);
		printf("11111111111111%x@@@@@@@@@@@@@@@@@@@@@@@@@@",tbuf);

		return(OK);
	}
	printf("\n\n\n.1.Load_Default_Keys_Check result----------------------------------[%02x]\n\n",ucResult);

	return 0;
}

