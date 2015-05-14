/* 
	INGENICO China
--------------------------------------------------------------------------
	FILE  pboc.c								(Copyright INGENICO 2001)       
--------------------------------------------------------------------------
    INTRODUCTION          
    ============                                             
    Created :		2001-08-13		Xiaoxi Jiang
    Last modified :	2001-08-13		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network             
    Purpose :                               
        Source file for pboc smart process. 
                                                             
    List of routines in file :                                            
                                                                          
    File history :                                                        
                                                                          
*/

#include <include.h>
#include <global.h>
#include <xdata.h>
#include <osicc.h>
#include <smart.h>
#include <toolslib.h>

unsigned char PBOC_ISOSelectFilebyAID(unsigned char ucReader,
			unsigned char *pucAID,unsigned char ucAIDLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xA4;
	Iso7816In.ucP1  = 0x04;
	Iso7816In.ucP2  = 00;

	Iso7816In.ucP3 = ucAIDLen;
	memcpy(&Iso7816In.aucDataIn[0],pucAID,ucAIDLen);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)         //old is type4
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOSelectFilebyFID(unsigned char ucReader,
			unsigned char *pucFID)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xA4;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;

	Iso7816In.ucP3 = 0x02;
	memcpy(&Iso7816In.aucDataIn[0],pucFID,2);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)         //old is type4
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOReadBinarybySFI(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xB0;
	Iso7816In.ucP1  = ucSFI | 0x80;
	Iso7816In.ucP2  = ucOffset;
	Iso7816In.ucP3  = ucInLen;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOReadRecordbySFI(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucRecordNumber)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xB2;
	Iso7816In.ucP1  = ucRecordNumber;
	Iso7816In.ucP2  = (ucSFI << 3) | 0x04;
	Iso7816In.ucP3  = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOGetBalance(unsigned char ucReader,
			unsigned char ucType)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x5C;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2	= ucType;			/* 0x01 ED 0x02 EP */
	Iso7816In.ucP3  = 0x04;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOInitForPurchase(unsigned char ucReader,
			unsigned char ucType,unsigned char *pucInData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x50;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2 = ucType;				/* 0x01 ED 0x02 EP */
	Iso7816In.ucP3  = 0x0B;

	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISODebit(unsigned char ucReader,
			unsigned char *pucDebitData,unsigned char ucLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x54;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;
	
	memcpy(&Iso7816In.aucDataIn[0],pucDebitData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x08;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());

}

unsigned char PBOC_ISOVerifyPIN(unsigned char ucReader,
			unsigned char ucInLen,unsigned char *pucPinData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x20;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucInLen;

	memcpy(&Iso7816In.aucDataIn[0],pucPinData,ucInLen);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		{			
			return(ucResult);
		}
		
    return(PBOC_ISOCheckReturn());

}

unsigned char PBOC_ISOGetResponse(unsigned char ucReader,
			unsigned char ucLen)
{
	unsigned char ucResult;
	
	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xC0;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOGetChallenge(unsigned char ucReader)
{
	unsigned char ucResult;
	
	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x84;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x04;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOGetInfo(void)
{
	unsigned char ucResult;
	
	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xE4;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x08;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOSAMComputeMAC1(unsigned char ucReader,
			unsigned char *pucInData)
{
	unsigned char	ucResult;
	
	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x70;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x2C;
	
	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;
	
	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}                                             

unsigned char PBOC_ISOGetTac(unsigned char TranType,
			unsigned char *pucInData)
{
	unsigned char	ucResult;
	
	return SUCCESS;
	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x5A;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = TranType;
	Iso7816In.ucP3  = 0x02;
	
	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x08;
	
	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}                                             

unsigned char PBOC_ISOSAMVerifyMAC2(unsigned char ucReader,
			unsigned char *pucInData)
{
	unsigned char	ucResult;
	
	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x72;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x04; 
	
	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;
	
	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOCheckReturn(void)
{
	
	unsigned char ucI,ucResult,aucBuf[255];
	
	if(Iso7816Out.uiLen<2)
		return SMART_CARD_ERROR;
	if(RunData.ucDxMobile)
	{
		Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-4];
		Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-3];
	}
	else
	{
		Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
		Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];
	}
	Uart_Printf("\n");
	Uart_Printf("ucSW1=%02x\n",Iso7816Out.ucSW1);	
	Uart_Printf("ucSW2=%02x\n",Iso7816Out.ucSW2);	
	if(RunData.ucDxMobile)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,Iso7816Out.aucData,100);
		memset(Iso7816Out.aucData,0,sizeof(Iso7816Out.aucData));
		memcpy(Iso7816Out.aucData,&aucBuf[6],100);
	}
	switch(Iso7816Out.ucSW1)
	{
	case 0x90:
		return(SMART_OK);
	case 0x6C:
		PBOC_SetIso7816Out();
		Iso7816In.ucP3  = Iso7816Out.ucSW2; 
		if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		{
			return(ucResult);
		}	
		return(PBOC_ISOCheckReturn());
	case 0x61:
	case 0x9F:
		return(PBOC_ISOGetResponse(ICC1,Iso7816Out.ucSW2));
	case 0x63:
		if(  (Iso7816Out.ucSW2 >= 0xC0)
		   &&(Iso7816Out.ucSW2 <= 0xCF)
		  )
		{
			RUNDATA_ucPINRetry = Iso7816Out.ucSW2 & 0x0F;
#ifdef TEST
			Uart_Printf("SMART_CARD_ERROR1\n");
#endif
			return(SMART_CARD_ERROR);
		}
#ifdef TEST
			Uart_Printf("SMART_CARD_ERROR2\n");
#endif
		return(SMART_CARD_ERROR);
	default:
#ifdef TEST
			Uart_Printf("SMART_CARD_ERROR3\n");
#endif
		return(SMART_CARD_ERROR);
	}
}

void PBOC_SetIso7816Out(void)
{
	Iso7816Out.uiLen = sizeof(Iso7816Out.aucData);
	memset(Iso7816Out.aucData,0,Iso7816Out.uiLen);
}
unsigned char PBOC_UpdateBinaryFile(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen,unsigned char *DataBuf)
{
	unsigned char	ucResult;
	int i;
	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xD6;
	Iso7816In.ucP1  = ucSFI | 0x80;
	Iso7816In.ucP2  = ucOffset;
	Iso7816In.ucP3  = ucInLen;
	memcpy(&Iso7816In.aucDataIn[0],DataBuf,ucInLen);
	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_UpdateBinaryFileMac(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen,unsigned char *DataBuf)
{
	unsigned char	ucResult;
	int i;
	Iso7816In.ucCla = 0x04;
	Iso7816In.ucIns = 0xD6;
	Iso7816In.ucP1  = ucSFI | 0x80;
	Iso7816In.ucP2  = ucOffset;
	Iso7816In.ucP3  = ucInLen+4;
	memcpy(&Iso7816In.aucDataIn[0],DataBuf,ucInLen+4);
	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_WriteKey_CPU(unsigned char *pucKey,unsigned char keyid)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xD4;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = keyid;

	Iso7816In.ucP3 = 21;
	memcpy(&Iso7816In.aucDataIn[0],pucKey,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)       
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_WritePin_CPU(unsigned char *pucKey,unsigned char keyid)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xD4;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = keyid;

	Iso7816In.ucP3 = 8;
	memcpy(&Iso7816In.aucDataIn[0],pucKey,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)       
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOInitForLoad(unsigned char ucReader,unsigned char *pucInData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x50;
	Iso7816In.ucP1 = 0x00;
	Iso7816In.ucP2 = 0x02;				/* 0x01 ED 0x02 EP */
	Iso7816In.ucP3  = 0x0B;

	memset(Iso7816In.aucDataIn,0,128);
	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x10;
	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOLoad(unsigned char ucReader,
			unsigned char *pucDebitData,unsigned char ucLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x52;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;
	
	memcpy(&Iso7816In.aucDataIn[0],pucDebitData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x08;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());

}

unsigned char PBOC_ISOLock(unsigned char *pucMacData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x84;
	Iso7816In.ucIns = 0xcd;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x04;
	
	memcpy(&Iso7816In.aucDataIn[0],pucMacData,Iso7816In.ucP3);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());

}
unsigned char PBOC_ISOExternalauth(unsigned char ucReader,
			unsigned char *pucInData)
{
	unsigned char	ucResult;
	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x82;
	Iso7816In.ucP1 = 0x00;				
	if(RunData.ucDxMobile&&RunData.aucBakBits[0]!=0)
		Iso7816In.ucP2  = 0x02;
	else	
		Iso7816In.ucP2  = 0x01;
	Iso7816In.ucP3  = 8;

	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOEaserfile(unsigned char ucReader)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x0E;
	Iso7816In.ucP1 = 0x00;				
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0;


	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOCreatefile(unsigned char ucfid,
			unsigned char *pucInData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xE0;
	Iso7816In.ucP1 = 0x00;				
	Iso7816In.ucP2  = ucfid;
	Iso7816In.ucP3  = 7;

	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOCreateDF(unsigned char ucfid,
			unsigned char *pucInData,unsigned char uLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xE0;
	Iso7816In.ucP1 = 0x3F;				
	Iso7816In.ucP2  = ucfid;
	Iso7816In.ucP3  = uLen;

	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOCreateMF(unsigned char ucfid,
			unsigned char *pucInData,unsigned char uLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xE0;
	Iso7816In.ucP1 = 0x3F;				
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 13;

	memset(Iso7816In.aucDataIn,0xFF,Iso7816In.ucP3);
	memcpy(&Iso7816In.aucDataIn[0],"\x38\xFF\xFF\xF1\xF1",5);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_WriteMainKey(unsigned char *pucKey)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xD4;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = 0x00;

	Iso7816In.ucP3 = 0x15;
	memcpy(&Iso7816In.aucDataIn[0],pucKey,0x15);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)      
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_WriteTestKey(unsigned char *pucKey)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xD4;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = 0x00;

	Iso7816In.ucP3 = 0x0D;
	memcpy(&Iso7816In.aucDataIn[0],pucKey,0x0D);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ICC1,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)      
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_Fensan_key(unsigned char *pucCardno,unsigned char ucKeyIndex,unsigned char *Outkey)
{
	unsigned char aucBuf[20],ucKeyArray;
	int i;
		
	switch(NormalTransData.ucPtCode)
	{
		case 0x01:
			if(RunData.ucKeyArrey)
				ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
			else
				ucKeyArray=KEYARRAY_PURSECARD1;
			break;
		case 0x02:ucKeyArray=KEYARRAY_PURSECARD8;break;
		case 0x03:ucKeyArray=KEYARRAY_PURSECARD9+RunData.ucKeyArrey;break;
		default:break;
	}
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucKeyIndex,pucCardno,aucBuf);
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucKeyIndex+1,aucBuf,aucBuf);
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucKeyIndex,aucBuf,Outkey);
	//des(pucCardno,aucBuf,pucKey );
	//desm1(aucBuf,aucBuf,&pucKey[8] );
	//des(aucBuf,Outkey,pucKey );
	for(i=0;i<16;i++)
		aucBuf[i]=pucCardno[i]^0xFF;
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucKeyIndex,aucBuf,aucBuf);
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucKeyIndex+1,aucBuf,aucBuf);
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucKeyIndex,aucBuf,&Outkey[8]);
	//des(aucBuf,aucBuf,pucKey );
	//desm1(aucBuf,aucBuf,&pucKey[8] );
	//des(aucBuf,&Outkey[8],pucKey );
	return SUCCESS;
}
unsigned char PBOC_MAC2(unsigned char *init,unsigned char *key,
	unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucOutMAC)
{
	unsigned char aucHexData[8],ucBackup;
	unsigned char *pucPtr,ucResult=SUCCESS;
	unsigned short uiI,uiJ,uiTTLen;
	unsigned short uiLen;
	int i;
	
	
	uiTTLen=uiInLen;
	pucPtr = pucInData;	
	//ڴıռѾ8Բ
	//Ժ޸עⲻҪ

	memcpy(pucPtr+uiInLen,"\x80",1);
	uiTTLen +=1;
	
			
	memset(aucHexData,0,sizeof(aucHexData));
	uiLen = 8;
	for(uiJ=0;uiJ<uiLen;uiJ++)
	{
		aucHexData[uiJ] ^= *(init+uiJ);
	}
	//pucPtr += 8;
	Uart_Printf("init for mac:\n");
	for(i=0;i<8;i++)
		Uart_Printf("%02x",aucHexData[i]);
	for(uiI=0;uiI<uiTTLen;uiI += 8)
	{
		uiLen = min(8,pucInData+uiTTLen-pucPtr);
		Uart_Printf("\nWaitxor for mac[%d]:\n",uiI/8);
		for(uiJ=0;uiJ<uiLen;uiJ++)
		{
			Uart_Printf("%02x",*(pucPtr+uiJ));
			aucHexData[uiJ] ^= *(pucPtr+uiJ);
		}
		Uart_Printf("\nWaitdes for mac[%d]:\n",uiI/8);
		for(i=0;i<8;i++)
			Uart_Printf("%02x",aucHexData[i]);
		des(aucHexData,aucHexData,key);
		Uart_Printf("\ndesresult for mac[%d]:\n",uiI/8);
		for(i=0;i<8;i++)
			Uart_Printf("%02x",aucHexData[i]);
//		desm1(aucHexData,aucHexData,key+8);
//		des(aucHexData,aucHexData,key);
		if( ucResult != SUCCESS)
        	return(ucResult);
		pucPtr += uiLen;
	}
	Uart_Printf("\nlastresult for mac[%d]:\n",uiI/8);
	for(i=0;i<8;i++)
		Uart_Printf("%02x ",aucHexData[i]);
	memcpy(pucOutMAC,aucHexData,8);

	return(SUCCESS);
}

uchar PBOC_Create_MAC1(uchar *Data,uchar Len)
{
    	uchar data_in[20],aucBuf[20];
    	uchar data_out[10],aucFensan[10]; 
    	uchar key[9],key1[9];
    	uchar data_mac[20];
    	uint  Length;     
    	uchar ucResult;   
     
     	memset(data_mac,0,sizeof(data_mac));
     	memset(key,0,sizeof(key));
     	memcpy(data_mac,Data,6);     
	long_bcd(aucBuf,4,&NormalTransData.ulTraceNumber);
     	memcpy(&data_mac[6],&aucBuf[2],2);  /*նˮλڲSAMԾ\X00*/
     	
     	{ 
     		uchar i;
     		Uart_Printf("input data for SESLK is:");
     		for (i=0; i<8;i++)
     			Uart_Printf("%02x",data_mac[i]);
     		Uart_Printf("\n");
     	}
     	{ 
     		uchar i;
     		Uart_Printf("DataSave0.ConstantParamData.Cpukey[0] is:");
     		for (i=0; i<16;i++)
     			Uart_Printf("%02x",DataSave0.ConstantParamData.Cpukey[0][i]);
     		Uart_Printf("\n");
     	}
     
     	/*Կ 3DES encrypt */       
	memset(aucFensan,0,sizeof(aucFensan));
	memcpy(aucFensan,NormalTransData.aucCardSerial,4);
	memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
	PBOC_Fensan_key(aucFensan,KEYINDEX_PURSECARD0_PURCHASEKEY,aucBuf);
	//memcpy(aucBuf,"\xc7\x2c\xf8\x2b\x89\x9d\x93\x54\xde\x4c\x2e\xa9\xda\x50\x49\x17",16);
	{ 
     		uchar i;
     		Uart_Printf("aucBuf:");
     		for (i=0; i<16;i++)
     			Uart_Printf("%02x",aucBuf[i]);
     		Uart_Printf("\n");
     	}          
	des(data_mac,key,aucBuf);            
	desm1(key,key,&aucBuf[8]);            
	des(key,key,aucBuf);            
	memcpy(key1,key,8);     //dudu   
	{ 
     		uchar i;
     		Uart_Printf("SESLK:");
     		for (i=0; i<8;i++)
     			Uart_Printf("%02x",key[i]);
     		Uart_Printf("\n");
     	}          
     	memset(data_in,0,sizeof(data_in));
     	memset(data_out,0,sizeof(data_out));
     	memcpy(data_in,&Data[6],5);  Length = 5;  /*  &  */
     	asc_bcd(&data_in[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8);Length += 4; /* ն˺ */
     	memset(&data_in[9],0x00,2);Length +=2;
     	memcpy(&data_in[Length],&Data[11],7); Length += 7; /* ʱ */
         
        /*ùԿMAC1*/  
        { 
     		uchar i;
     		Uart_Printf("input data for MAC1:");
     		for (i=0; i<18;i++)
     			Uart_Printf("%02x",data_in[i]);
     		Uart_Printf("\n");
     	}       
     	//macgen(data_in,Length,key,data_out);     	
     	memset(aucBuf,0,sizeof(aucBuf));
	PBOC_MAC2(aucBuf,key,data_in, Length, data_out);
     	memcpy(&Iso7816Out.aucData[0],data_out,4); 
  
        /*ùԿMAC2   
     	memset(data_mac,0,sizeof(data_mac));
	memcpy(data_mac,&data_in[0],4);  Length=4; // 
     	macgen(data_mac,Length,key1,data_out);
     	memcpy(RunData.Mac2,data_out,4);*/    
     	
     	return(SUCCESS);
}	
unsigned char PBOC_CardReput(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20],aucBuf1[20];
	unsigned long ulNowAmount;
	while(1)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"Żԭ");
		ucResult = MIFARE_WaitCard();
		if(ucResult==ERR_CANCEL)
			return ucResult;
		if(ucResult==SUCCESS)
		{
			////Os__clr_display(255);
			////Os__GB2312_display(0,0,(uchar *)"Ժ...");
	   		memset(aucOutData,0,sizeof(aucOutData));
		 	ucResult = Untouch_ReadCpu_tmp(&ucStatus,aucOutData,&ucLen);
		 	//ucResult = Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult == SUCCESS)
			{
				memcpy(NormalTransData.aucCardSerial,aucOutData,4);
				ucResult=PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x00");
				if(ucResult == SUCCESS)
				{
					ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"SANDPAY",7);
				}
				if(ucResult != SUCCESS)
				{
					ucResult =  PBOC_ISOSelectFilebyAID(ICC1, (unsigned char*)"1PAY.SYS.DDF01",14);		
					if(ucResult == SUCCESS)
						ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"\xA0\x00\x00\x00\x03\x86\x98\x07\x02",9);
					if(ucResult == SUCCESS)
						ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"\x10\x00\x00\x00\x00\x00\x10\x00\x03",9);
				}
				if(ucResult == SUCCESS)
				{
					ucResult=PBOC_ISOReadBinarybySFI(ICC1, 0x15, 0, 30);
				}
				if(ucResult == SUCCESS)
				{
					if(memcmp(NormalTransData.aucUserCardNo,&Iso7816Out.aucData[10],8))
					{
						continue;
					}
				}
				if(ucResult==SUCCESS)
				{
					Uart_Printf("֤pin\n");
					ucResult = PBOC_ISOVerifyPIN(ICC1,3,(uchar*)"\x88\x88\x88");
				}
				if(ucResult == SUCCESS)
				{
					ucResult=PBOC_ISOGetBalance(ICC1, 0x02);
				}
				if(ucResult == SUCCESS)
				{
					ulNowAmount=tab_long(Iso7816Out.aucData,4);
					if(ulNowAmount==NormalTransData.ulPrevEDBalance-NormalTransData.ulAmount)
					{
						ucResult=PBOC_ISOReadRecordbySFI(ICC1, 0x18, 1);
						if(ucResult==SUCCESS)
						{
							asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucOnlytermcode,8);
							long_tab(&aucBuf[4],4,&DataSave0.RedoTrans.Tran.ulAmount);
							if(memcmp(aucBuf,&Iso7816Out.aucData[10],4)==0
								&&memcmp(&aucBuf[4],&Iso7816Out.aucData[5],4)==0
								&&memcmp(DataSave0.RedoTrans.Tran.aucDateTime,&Iso7816Out.aucData[16],7)==0)
							{
								ucResult=PBOC_ISOGetTac(0x06,&NormalTransData.aucSamTace[2]);
								if(ucResult==SUCCESS)
								{
									memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);
									return SUCCESS;
								}
							}
							else
							{
								ucResult=SAV_KyTran(1);
								return ERR_DETAILDIF;
							}
								
						}
					}else
					if(ulNowAmount==NormalTransData.ulPrevEDBalance)
					{
						return 0xFF;
					}else
					{
						ucResult=SAV_KyTran(1);
						return ERR_DIFFERENT_AMOUNT;
					}
				}
			}
		}
	}
}
unsigned char PBOC_DebitPart1(void)
{
	uchar ucResult=SUCCESS,ucLen,aucBuf[20],aucFensan[10],aucUseKey[20];
	uchar Mac_Data[100],Offline_Cnt[2],Issuer_Key_Version,Card_Random[4];
	ulong ulAmount,Prev_ED_Balance,ulamount;
	
	if(ucResult!=SUCCESS)
		return(ucResult);

	Uart_Printf("Start debit\n");	
	if(RunData.aucFunFlag[1])
		ulamount=min(NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount);
	else
		ulamount=min(NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance);
	memset(Mac_Data,0,sizeof(Mac_Data));	
	/*if(RunData.ucDxMobile)
		Mac_Data[0] = 0x01;
	else*/
		Mac_Data[0] = 0x00;
	long_tab(&Mac_Data[1],4,&ulamount);
	asc_bcd(&Mac_Data[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8); 
	memset(&Mac_Data[9],0x00,2);
	{ 
     		uchar i;
     		Uart_Printf("input data for init for purchase:");
     		for (i=0; i<11;i++)
     			Uart_Printf("%02x",Mac_Data[i]);
     		Uart_Printf("\n");
     		}   
	/*ѳʼ*/
	ucResult = PBOC_ISOInitForPurchase(ICC1,2,Mac_Data);
	if(ucResult!=SUCCESS)
	{
		Uart_Printf("init debit err\n");
		return(ucResult);
	}
	
	Prev_ED_Balance = tab_long(Iso7816Out.aucData,4);   /*ǰ*/
	memcpy(Offline_Cnt,&Iso7816Out.aucData[4],2);	  /*ѻ*/
	Issuer_Key_Version = Iso7816Out.aucData[9];	  /*Կ汾*/
	memcpy(Card_Random,&Iso7816Out.aucData[11],4);	  /**/
	
	memset(Mac_Data,0,sizeof(Mac_Data));	
		
	memcpy(&Mac_Data[0],Card_Random,4); ucLen = 4;	
	memcpy(&Mac_Data[ucLen],Offline_Cnt,2); ucLen += 2;
	
	long_tab(&Mac_Data[ucLen],4,&ulamount);
	ucLen += 4;	

 	Mac_Data[ucLen] = 0x06;
	ucLen += 1;
	 
	memcpy(&Mac_Data[ucLen],NormalTransData.aucDateTime,7); ucLen += 7;	
	Mac_Data[ucLen] = Issuer_Key_Version; ucLen += 1;	
	if(RunData.ucDxMobile)
		Mac_Data[ucLen] = 0x00; 
	else
		Mac_Data[ucLen] = 0x01;   ucLen += 1; /* 㷨ʶ */
	memcpy(&Mac_Data[ucLen],NormalTransData.aucCardSerial,4); ucLen += 4;
	memcpy(&Mac_Data[ucLen],&NormalTransData.aucUserCardNo[4],4); ucLen += 4;
	
	
	ucResult = PBOC_Create_MAC1(Mac_Data,ucLen);
	if(ucResult!=SUCCESS)
	{
		Uart_Printf("create mac1 err in purchase \n");
		DataSave0.RedoTrans.ucRedoFlag=0;
		XDATA_Write_Vaild_File(DataSaveRedoTrans);
		return(ucResult);
	}	
	Uart_Printf("create mac1 success in purchase\n");
	{ 
 		uchar i;
 		Uart_Printf("MAC1:");
 		for (i=0; i<4;i++)
 			Uart_Printf("%02x",Iso7816Out.aucData[i]);
 		Uart_Printf("\n");
 		}   
		
	memset(Mac_Data,0,sizeof(Mac_Data));		
	long_bcd(&Mac_Data[0],4,&NormalTransData.ulTraceNumber); ucLen  = 4;
	memcpy(&Mac_Data[ucLen],NormalTransData.aucDateTime,7); ucLen += 7;
	memcpy(&Mac_Data[ucLen],&Iso7816Out.aucData[0],4); ucLen += 4;
	
	{ 
     		uchar i;
     		Uart_Printf("input data PBOC_DebitProcess:");
     		for (i=0; i<ucLen;i++)
     			Uart_Printf("%02x",Mac_Data[i]);
     		Uart_Printf("\n");
     	}       
	memcpy(&NormalTransData.aucSamTace[2],Offline_Cnt,2);
	memcpy(&NormalTransData.aucCardRandom,Card_Random,4);
	
	memcpy(&DataSave0.RedoTrans.Tran,&NormalTransData,sizeof(NORMALTRANS));
	DataSave0.RedoTrans.ucRedoFlag=1;
	XDATA_Write_Vaild_File(DataSaveRedoTrans);
	if ((ucResult = PBOC_ISODebit(ICC1,Mac_Data,ucLen)) != SUCCESS)
	{
		Uart_Printf("PBOC_DebitProcess error \n");
		/*if(ucResult==0x40)
		{
			ucResult=PBOC_CardReput();
		}*/
		if(ucResult!=SUCCESS)
		{
			if(RunData.ucNeedRedo==0&&ucResult!=0x04)
			{
				DataSave0.RedoTrans.ucRedoFlag=0;
				XDATA_Write_Vaild_File(DataSaveRedoTrans);
			}
			return(ucResult);
		}
	}	
	
	/* store Tac */
	memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);   /*TAC*/
	Uart_Printf("init debit success\n");
	if(NormalTransData.ucReturnflag==true)
	{
		if(ucResult==SUCCESS)
		{
			ucResult=PBOC_ISOGetChallenge(ICC1);
		}
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,Iso7816Out.aucData,4);
			memset(aucFensan,0,sizeof(aucFensan));
			memcpy(aucFensan,NormalTransData.aucCardSerial,4);
			memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
			PBOC_Fensan_key(aucFensan,
				KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
			des(aucBuf,aucBuf,aucUseKey);
			desm1(aucBuf,aucBuf,&aucUseKey[8]);
			des(aucBuf,aucBuf,aucUseKey);
			ucResult=PBOC_ISOExternalauth(ICC1, aucBuf);
		}
		if(ucResult==SUCCESS)
		{
			aucBuf[0]=CARD_RECYCLED;
			ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
		}
	}
	DataSave0.RedoTrans.ucRedoFlag=2;
	XDATA_Write_Vaild_File(DataSaveRedoTrans);
	Uart_Printf("PBOC_DebitProcess success \n");
	return(SUCCESS);
}
unsigned char PBOC_MAC(unsigned char *init,unsigned char *key,
	unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucOutMAC)
{
	unsigned char aucHexData[8],ucBackup;
	unsigned char *pucPtr,ucResult=SUCCESS;
	unsigned short uiI,uiJ,uiTTLen;
	unsigned short uiLen;
	int i;
	
	Uart_Printf("key for mac:\n");
	for(i=0;i<16;i++)
		Uart_Printf("%02x",key[i]);
	
	uiTTLen=uiInLen;
	pucPtr = pucInData;	
	//ڴıռѾ8Բ
	//Ժ޸עⲻҪ

	memcpy(pucPtr+uiInLen,"\x80",1);
	uiTTLen +=1;
	
			
	memset(aucHexData,0,sizeof(aucHexData));
	uiLen = 8;
	for(uiJ=0;uiJ<uiLen;uiJ++)
	{
		aucHexData[uiJ] ^= *(init+uiJ);
	}
	//pucPtr += 8;
	Uart_Printf("init for mac:\n");
	for(i=0;i<8;i++)
		Uart_Printf("%02x",aucHexData[i]);
	for(uiI=0;uiI<uiTTLen;uiI += 8)
	{
		uiLen = min(8,pucInData+uiTTLen-pucPtr);
		Uart_Printf("\nWaitxor for mac[%d]:\n",uiI/8);
		for(uiJ=0;uiJ<uiLen;uiJ++)
		{
			Uart_Printf("%02x",*(pucPtr+uiJ));
			aucHexData[uiJ] ^= *(pucPtr+uiJ);
		}
		Uart_Printf("\nWaitdes for mac[%d]:\n",uiI/8);
		for(i=0;i<8;i++)
			Uart_Printf("%02x",aucHexData[i]);
		des(aucHexData,aucHexData,key);
		Uart_Printf("\ndesresult for mac[%d]:\n",uiI/8);
		for(i=0;i<8;i++)
			Uart_Printf("%02x",aucHexData[i]);
//		desm1(aucHexData,aucHexData,key+8);
//		des(aucHexData,aucHexData,key);
		if( ucResult != SUCCESS)
        	return(ucResult);
		pucPtr += uiLen;
	}
	desm1(aucHexData,aucHexData,key+8);
	des(aucHexData,aucHexData,key);
	Uart_Printf("\nlastresult for mac[%d]:\n",uiI/8);
	for(i=0;i<8;i++)
		Uart_Printf("%02x ",aucHexData[i]);
	memcpy(pucOutMAC,aucHexData,8);

	return(SUCCESS);
}
unsigned char PBOC_ISOSelectFilebyFID_Bak(unsigned char ucReader,
			unsigned char *pucFID)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xA4;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;

	Iso7816In.ucP3 = 0x02;
	memcpy(&Iso7816In.aucDataIn[0],pucFID,2);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO_bak(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)         //old is type4
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOReadBinarybySFI_Bak(unsigned char ucReader,
			unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xB0;
	Iso7816In.ucP1  = ucSFI | 0x80;
	Iso7816In.ucP2  = ucOffset;
	Iso7816In.ucP3  = ucInLen;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO_bak(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOSpecialDes(unsigned char ucReader,
			unsigned char *pucIndata)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0xFC;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x01;

	Iso7816In.ucP3 = 0x0D;
	memcpy(&Iso7816In.aucDataIn[0],pucIndata,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)         //old is type4
		return(ucResult);

	return(PBOC_ISOCheckReturn());
	
}


