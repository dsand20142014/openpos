/*
--------------------------------------------------------------------------
	FILE  desfire.c
--------------------------------------------------------------------------
*/

//#include <os.h>
#include <osmifare.h>
#define DESFIREDATA
#include <desfire.h>
#undef DESFIREDATA

unsigned char OSMIFARE_SendRevFrame( unsigned char *pucSendFrame,unsigned int uiSendLen,
								unsigned char *pucRevFrame,unsigned int *puiRevLen,unsigned int uiTimeDelay );

unsigned char aucErrCode[][2] = 
{ 
	{0x01,0x0C},        //DESFIRE_ERR_NOCHANGE		
	{0x02,0x0E},        //DESFIRE_ERR_OUTOFMEM		
	{0x03,0x1C},        //DESFIRE_ERR_CMDCODE			
	{0x04,0x1E},        //DESFIRE_ERR_CRCMAC			
	{0x05,0x40},        //DESFIRE_ERR_NOSUCHKEY		
	{0x06,0x7E},        //DESFIRE_ERR_CMDLENGH		
	{0x07,0x9D},        //DESFIRE_ERR_DENIED			
	{0x08,0x9E},        //DESFIRE_ERR_PARAMETER		
	{0x09,0xA0},        //DESFIRE_ERR_APPNOTFOUND		
	{0x0A,0xA1},        //DESFIRE_ERR_APPINTEGRITY	
	{0x0B,0xAE},        //DESFIRE_ERR_AUTHENTICATION	
	{0x0C,0xAF},        //DESFIRE_ERR_NEEDFRAME		
	{0x0D,0xBE},        //DESFIRE_ERR_BOUNDARY		
	{0x0E,0xC1},        //DESFIRE_ERR_PICCINTEGRITY	
	{0x0F,0xCA},        //DESFIRE_ERR_CMDABORTED		
	{0x10,0xCD},        //DESFIRE_ERR_PICCDISABLE		
	{0x11,0xCE},        //DESFIRE_ERR_APPCOUNT		
	{0x12,0xDE},        //DESFIRE_ERR_DUPLICATE		
	{0x13,0xEE},        //DESFIRE_ERR_EEPROM			
	{0x14,0xF0},        //DESFIRE_ERR_FILENOTFOUND	
	{0x15,0xF1},        //DESFIRE_ERR_FILEINTEGRITY
	{0,0}	
};
#define DESFIRE_ORDER_GETKEYVERSION	    	0x64

DESFIRE_ORDER_TYPE *pDesfireOrder;
unsigned char DESFIRE_GetKeyVersion( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );

unsigned char DESFIRE_ConvErrCode( unsigned char ucCode )
{
	unsigned char ucI;
	for( ucI=0;aucErrCode[ucI][0]||aucErrCode[ucI][1];ucI++ )
	{
		if( aucErrCode[ucI][1]==ucCode )
			return aucErrCode[ucI][0];
	}
	return DESFIRE_ERR_UNKNOWN;
}

unsigned char DESFIRE_Process( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
	unsigned char ucResult;
	
	pDesfireOrder = &pOrder->pt_order_in->desfire_order;
	switch( pDesfireOrder->order_type )
	{
		case DESFIRE_ORDER_GETKEYVERSION:
			ucResult = DESFIRE_GetKeyVersion( pAnswer,pOrder );
			break;
	}
	return ucResult;
}

/*unsigned char void DESFIRE_SendRevCmd( aucSendData,uiSendLen,aucRevData,&uiRevLen )
{
	return 0;
}*/

unsigned char DESFIRE_GetKeyVersion( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
	unsigned char aucSendData[DESFIRE_SEND_MAX_LEN];
	unsigned char aucRevData[DESFIRE_REV_MAX_LEN];
	unsigned char ucResult;
	unsigned int uiSendLen,uiRevLen;
	unsigned int uiTimeDelay;
	
	memset( aucSendData,0,sizeof(aucSendData) );
	memset( aucRevData,0,sizeof(aucRevData) );
	uiSendLen = 0;
	uiTimeDelay = 0;
	aucSendData[uiSendLen++] = pDesfireOrder->order_type;
	aucSendData[uiSendLen++] = pDesfireOrder->aucInData[0];
	uiRevLen = 0;
	ucResult = OSMIFARE_SendRevFrame( aucSendData,uiSendLen,aucRevData,&uiRevLen,uiTimeDelay );
	if( ucResult==OSMIFARE_ERR_SUCCESS )
	{
		if( uiRevLen==2 )
		{
			pOrder->ptout[0] = aucRevData[1];
			pAnswer->card_status = aucRevData[0];
		}else
			ucResult = OSMIFARE_ERR_BYTECOUNT;
	}
	return ucResult;
}
