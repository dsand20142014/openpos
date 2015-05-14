#include <include.h>
#include <global.h>
#include <xdata.h>
#include <osicc.h>
#include <smart.h>
#include <toolslib.h>
#include <osmifare.h>
#include <mifare.h>

void MIFARE_SetIso7816Out(void)
{
	Iso7816Out.uiLen = sizeof(Iso7816Out.aucData);
	memset(Iso7816Out.aucData,0,Iso7816Out.uiLen);
}

unsigned char MIFARE_ISOCheckReturn(void)
{
	
	unsigned char ucI,ucResult;
	
	Uart_Printf("Iso7816Out.uiLen=%d\n",Iso7816Out.uiLen);
	if(Iso7816Out.uiLen<2)
		return SMART_CARD_ERROR;
	Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
	Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];
	for(ucI=0;ucI<25;ucI++)
	{
		Uart_Printf("%02x,",Iso7816Out.aucData[ucI]);
	}
	Uart_Printf("\n");
	Uart_Printf("ucSW1=%02x\n",Iso7816Out.ucSW1);	
	Uart_Printf("ucSW2=%02x\n",Iso7816Out.ucSW2);	
	switch(Iso7816Out.ucSW1)
	{
		case 0x90:
			return(SMART_OK);
		case 0x63:
			if(  (Iso7816Out.ucSW2 >= 0xC0)
			   &&(Iso7816Out.ucSW2 <= 0xCF)
			  )
			{
				return(SMART_CARD_ERROR);
			}
			return(SMART_CARD_ERROR);
		default:
			return(SMART_CARD_ERROR);
	}
}

unsigned char MIFARE_ISOSelectFilebyAID(unsigned char *pucAID,unsigned char ucAIDLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xA4;
	Iso7816In.ucP1  = 0x04;
	Iso7816In.ucP2  = 0x00;

	Iso7816In.ucP3 = ucAIDLen;
	memcpy(&Iso7816In.aucDataIn[0],pucAID,ucAIDLen);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE4)) != OK)         //old is type4
		return(ucResult);
	
	return(MIFARE_ISOCheckReturn());
}
unsigned char MIFARE_ISOSelectFilebyFID(unsigned char *pucFID)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xA4;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;

	Iso7816In.ucP3 = 0x02;
	memcpy(&Iso7816In.aucDataIn[0],pucFID,2);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE4)) != OK)         //old is type4
		return(ucResult);
	
	return(MIFARE_ISOCheckReturn());
}
unsigned char MIFARE_ISOReadBinarybySFI(unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xB0;
	Iso7816In.ucP1  = ucSFI | 0x80;
	Iso7816In.ucP2  = ucOffset;
	Iso7816In.ucP3  = ucInLen;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE2)) != OK)
		return(ucResult);
	
	return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISOVerifyPIN(unsigned char ucInLen,unsigned char *pucPinData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x20;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucInLen;

	memcpy(&Iso7816In.aucDataIn[0],pucPinData,ucInLen);

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		{			
			return(ucResult);
		}
		
    return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISOGetBalance(unsigned char ucType)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x5C;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2	= ucType;			/* 0x01 ED 0x02 EP */
	Iso7816In.ucP3  = 0x04;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE2)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISOGetChallenge(void)
{
	unsigned char ucResult;
	
	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x84;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x04;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE2)) != OK)
		return(ucResult);
    return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISOLock(unsigned char *pucMacData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x84;
	Iso7816In.ucIns = 0xcd;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 0x04;
	
	memcpy(&Iso7816In.aucDataIn[0],pucMacData,Iso7816In.ucP3);

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());

}

unsigned char MIFARE_ISOInitForPurchase(unsigned char ucType,unsigned char *pucInData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x50;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2 = ucType;				/* 0x01 ED 0x02 EP */
	Iso7816In.ucP3  = 0x0B;

	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISODebit(unsigned char *pucDebitData,unsigned char ucLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x54;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;
	
	memcpy(&Iso7816In.aucDataIn[0],pucDebitData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x08;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());

}

unsigned char MIFARE_ISOReadRecordbySFI(unsigned char ucSFI,unsigned char ucRecordNumber)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xB2;
	Iso7816In.ucP1  = ucRecordNumber;
	Iso7816In.ucP2  = (ucSFI << 3) | 0x04;
	Iso7816In.ucP3  = 0x00;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE2)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());
}
unsigned char MIFARE_ISOGetTac(unsigned char TranType,
			unsigned char *pucInData)
{
	unsigned char	ucResult;
	
	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x5A;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = TranType;
	Iso7816In.ucP3  = 0x02;
	
	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x08;
	
	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE4)) != OK)
		return(ucResult);
    return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISOInitForLoad(unsigned char *pucInData)
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
	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE4)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());
}

unsigned char MIFARE_ISOLoad(unsigned char *pucDebitData,unsigned char ucLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x80;
	Iso7816In.ucIns = 0x52;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;
	
	memcpy(&Iso7816In.aucDataIn[0],pucDebitData,Iso7816In.ucP3);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x08;

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());

}
unsigned char 	MIFARE_ISOExternalauth(unsigned char *pucInData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x82;
	Iso7816In.ucP1 = 0x00;				
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = 8;

	memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		return(ucResult);
	return(MIFARE_ISOCheckReturn());
}
unsigned char MIFARE_UpdateBinaryFile(unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen,unsigned char *DataBuf)
{
	unsigned char	ucResult;
	int i;
	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xD6;
	Iso7816In.ucP1  = ucSFI | 0x80;
	Iso7816In.ucP2  = ucOffset;
	Iso7816In.ucP3  = ucInLen;
	memcpy(&Iso7816In.aucDataIn[0],DataBuf,ucInLen);
	MIFARE_SetIso7816Out();
	if( (ucResult = MIFARE_ISO(
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						MIFARE_TYPE3)) != OK)
		return(ucResult);
	
	return(MIFARE_ISOCheckReturn());
}