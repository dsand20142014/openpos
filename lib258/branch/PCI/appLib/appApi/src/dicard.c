/*
--------------------------------------------------------------------------
	FILE  mcard.c
--------------------------------------------------------------------------
*/

//#include <os.h>
#include <osmifare.h>
#include <dicard.h>
#include <string.h>

unsigned char OSMIFARE_SendRevFrame( unsigned char *pucSendFrame,unsigned int uiSendLen,
								unsigned char *pucRevFrame,unsigned int *puiRevLen,unsigned int uiTimeDelay );

unsigned char DICARD_Process( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
	unsigned char aucSendData[DICARD_SEND_MAX_LEN];
	unsigned char aucRevData[DICARD_REV_MAX_LEN];
	unsigned char ucResult;
	unsigned int uiSendLen,uiRevLen;
	unsigned int uiTimeDelay;
	
	memset( aucSendData,0,sizeof(aucSendData) );
	memset( aucRevData,0,sizeof(aucRevData) );
	uiRevLen = 0;
	uiSendLen = pOrder->pt_order_in->dicard_order.uiInDataLen;
	memcpy( aucSendData,pOrder->pt_order_in->dicard_order.aucInData,uiSendLen );
	if( (aucSendData[0]==0x80)&&(aucSendData[1]==0x54) )
	{
		uiTimeDelay =DICARD_DELAY_PURCHASE;
	}else if( (aucSendData[0]==0x80)&&(aucSendData[1]==0x52) )
	{
		uiTimeDelay =DICARD_DELAY_CREDITLOAD;
	}else if( (aucSendData[0]==0x04)&&(aucSendData[1]==0xE2) )
	{
		uiTimeDelay =DICARD_DELAY_APPENDRECORD;
	}else if( (aucSendData[0]==0x04)&&(aucSendData[1]==0xDC) )
	{
		uiTimeDelay =DICARD_DELAY_UPDATAERECORD;
	}else if( (aucSendData[0]==0x04)&&(aucSendData[1]==0xD6) )
	{
		uiTimeDelay =DICARD_DELAY_UPDATAEBINARY;
	}else
		uiTimeDelay = 20;
	
	ucResult = OSMIFARE_SendRevFrame( aucSendData,uiSendLen,aucRevData,&uiRevLen,uiTimeDelay );
	if( ucResult==OSMIFARE_ERR_SUCCESS )
	{
		if( uiRevLen )
		{
			pAnswer->Len = uiRevLen;
			memcpy( pOrder->ptout,aucRevData,uiRevLen );
		}
	}
	pAnswer->card_status = ucResult;
	return OSMIFARE_ERR_SUCCESS;
}
