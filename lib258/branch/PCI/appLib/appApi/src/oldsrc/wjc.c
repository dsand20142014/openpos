
#include <stdio.h>
#include <string.h>

#include "wjc.h"

int CLASSIC_AuthBlock(int fd, unsigned char ucAuthType,unsigned char ucSectorIndex,
					unsigned char ucKeyType,unsigned char *pucKey )
{						
		int ucResult;
	 	struct auth_data_info authData;
			
		memset( &authData,0,sizeof(struct auth_data_info) );
		
   	    authData.ucAuthType = ucAuthType ;
   	    authData.ucSectorIndex = ucSectorIndex;
   	    authData.ucKeyType = ucKeyType ;	
		
		if (strlen(pucKey) != sizeof(authData.pucKey)) {
 		  	printf("pucKey length is not 6.\n");
 		  	return -1;
 		}			  
 		//memcpy (authData.pucKey,pucKey,sizeof(authData.pucKey));
		memcpy (authData.pucKey,pucKey,6);
		ucResult = ioctl(fd,PCD_IOC_AUTH,&authData);
	   return ucResult;									
}
			
int CLASSIC_ReadBlock(int fd,  unsigned char ucBlockIndex,unsigned char *pucBlockData )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned int uiRevLen,ucCheckMode;
	struct sendRevDataEx_info sendRevData;
	int ucResult;
	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	
	//uiRevLen = sizeof(aucRevData);
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN|PCD_CHECK_RXCRCEN;
	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,100 );
	
	 memset( &sendRevData,0,sizeof(struct sendRevDataEx_info) );
	 sendRevData.ucCheckMode= PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN|PCD_CHECK_RXCRCEN;	 
	 sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_READ;
	 sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;	  
	 sendRevData.puiRevLen= sizeof(sendRevData.pucRevData);
	 sendRevData.uiTimeDelay = 100;
	 
   ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	
	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( sendRevData.puiRevLen )
		{
			memcpy( pucBlockData,sendRevData.pucRevData,CLASSIC_BLOCK_LEN );
		}
	}
	return ucResult;
}		


int CLASSIC_WriteBlock(int fd,  unsigned char ucBlockIndex,unsigned char *pucBlockData )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned char ucResult,ucCheckMode;
	//unsigned int uiRevLen;
	
	struct sendRevDataEx_info sendRevData;
	int ucResult;
	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	//aucSendData[ucSendLen++] = CLASSIC_ORDER_WRITE;
	//aucSendData[ucSendLen++] = ucBlockIndex;
	//uiRevLen = sizeof(aucRevData);
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;		
	memset( &sendRevData,0,sizeof(struct sendRevDataEx_info) );		
	 sendRevData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;	
	 sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_WRITE;
	 sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;	 
	 sendRevData.puiRevLen= sizeof(sendRevData.pucRevData); 
	 sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );			
	 ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	
	 
	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( (sendRevData.pucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
		{
			//uiRevLen = 0;
			//ucSendLen = CLASSIC_BLOCK_LEN;
			sendRevData.puiRevLen = 0;
			sendRevData.uiSendLen = CLASSIC_BLOCK_LEN;
			//memcpy( aucSendData,pucBlockData,CLASSIC_BLOCK_LEN );
			memcpy( sendRevData.pucSendData,pucBlockData,CLASSIC_BLOCK_LEN );
			//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );
			ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	
			if( ucResult==PCD_ERR_SUCCESS )
			{
				if( (sendRevData.pucRevData[0]&0x0F)!=CLASSIC_RESPOND_ACK )
					ucResult = CLASSIC_DELAY_WRITE;
			}
		}else
			ucResult = CLASSIC_DELAY_WRITE;
	}
	return ucResult;
}
	
	int CLASSIC_TransferBlock(int fd, unsigned char ucBlockIndex )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned char ucResult,ucCheckMode;
	//unsigned int uiRevLen;

	struct sendRevDataEx_info sendRevData;
	int ucResult;
	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	//aucSendData[ucSendLen++] = 0xB0;
	//aucSendData[ucSendLen++] = ucBlockIndex;
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
	//uiRevLen = sizeof(aucRevData);	
	

	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );			 			 
	memset( &sendRevData,0,sizeof(struct sendRevDataEx_info) );	
	sendRevData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
	
	sendRevData.pucSendData[sendRevData.uiSendLen++] = 0xB0;
	sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;
	 
	sendRevData.puiRevLen= sizeof(sendRevData.pucRevData); 
	sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
	ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	
	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( (sendRevData.pucRevData[0]&0x0F)!=CLASSIC_RESPOND_ACK )
			ucResult = CLASSIC_ERR_TRANSFER;
	}
	return ucResult;
}

int CLASSIC_Increment(int fd, unsigned char ucBlockIndex,unsigned long ulValue )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned char ucResult,ucCheckMode;
	//unsigned int uiRevLen;

	struct sendRevDataEx_info sendRevData;
	struct sendData_info sendData;
	int ucResult;
	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	//aucSendData[ucSendLen++] = CLASSIC_ORDER_INC;
	//aucSendData[ucSendLen++] = ucBlockIndex;
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
	//uiRevLen = sizeof(aucRevData);			 
	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );
	memset( &sendRevData,0,sizeof(struct sendRevDataEx_info));		
	memset( &sendData,0,sizeof(struct sendData_info));	
	sendRevData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;

	sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_INC;
	sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;
	
	sendRevData.puiRevLen= sizeof(sendRevData.pucRevData); 
	sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
	ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	
	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( (sendRevData.pucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
		{
			//uiRevLen = 0;
			//ucSendLen = CLASSIC_VALUE_LEN;
			
			//memcpy( aucSendData,&ulValue,CLASSIC_VALUE_LEN );
			//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
			sendData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
			memcpy( sendData.pucSendData ,&ulValue,CLASSIC_VALUE_LEN );
			sendData.uiSendLen = CLASSIC_VALUE_LEN;

			//ucResult = PCD_SendData( ucCheckMode,aucSendData,ucSendLen );
			ucResult = ioctl(fd,PCD_IOC_SENDDATA,&sendData);

			
			if( ucResult==PCD_ERR_SUCCESS )							
			{
				ucResult = CLASSIC_TransferBlock(fd, ucBlockIndex );
			}
		}else
			ucResult = CLASSIC_ERR_INC;
	}
	return ucResult;
}

int CLASSIC_Decrement(int fd, unsigned char ucBlockIndex,unsigned long ulValue )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned char ucResult,ucCheckMode;
	//unsigned int uiRevLen;

	struct sendRevDataEx_info sendRevData;
	struct sendData_info sendData;
	int ucResult;
	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	//aucSendData[ucSendLen++] = CLASSIC_ORDER_DEC;
	//aucSendData[ucSendLen++] = ucBlockIndex;
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
	//uiRevLen = sizeof(aucRevData);	
	
	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );			 

	memset( &sendRevData,0,sizeof(struct sendRevDataEx_info));		
	memset( &sendData,0,sizeof(struct sendData_info));	
	sendRevData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;

	sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_DEC;
	sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

	sendRevData.puiRevLen= sizeof(sendRevData.pucRevData); 
	sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
	ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	

	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( (sendRevData.pucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
		{
			//uiRevLen = 0;
			//ucSendLen = CLASSIC_VALUE_LEN;
			//memcpy( aucSendData,&ulValue,CLASSIC_VALUE_LEN );
			//ucResult = PCD_SendData( ucCheckMode,aucSendData,ucSendLen );
			
			sendData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
			sendData.uiSendLen = CLASSIC_VALUE_LEN;
			memcpy( sendData.pucSendData,&ulValue,CLASSIC_VALUE_LEN );
			ucResult = ioctl(fd,PCD_IOC_SENDDATA,&sendData);
			if( ucResult==PCD_ERR_SUCCESS )							
			{
				ucResult = CLASSIC_TransferBlock(fd, ucBlockIndex );
			}
		}else
			ucResult = CLASSIC_ERR_DEC;
	}
	return ucResult;
}

int CLASSIC_Transfer( int fd,unsigned char ucBlockIndex )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned char ucResult,ucCheckMode;
	//unsigned int uiRevLen;

	struct sendRevDataEx_info sendRevData;
	int ucResult;
	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	//aucSendData[ucSendLen++] = CLASSIC_ORDER_TRANSFER;
	//aucSendData[ucSendLen++] = ucBlockIndex;
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
	//uiRevLen = sizeof(aucRevData);			 
	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );
	memset( &sendRevData,0,sizeof(struct sendRevDataEx_info));		
	sendRevData.ucCheckMode =  PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;

	sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_TRANSFER;
	sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

	sendRevData.puiRevLen= sizeof(sendRevData.pucRevData); 
	sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
	ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	

	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( (sendRevData.pucRevData[0]&0x0F)!=CLASSIC_RESPOND_ACK )
			ucResult = CLASSIC_ERR_TRANSFER;
	}
	return ucResult;
}

int CLASSIC_Restore(int fd, unsigned char ucBlockIndex )
{
	//unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
	//unsigned char aucRevData[CLASSIC_REV_MAX_LEN];
	//unsigned char ucResult,ucCheckMode;
	//unsigned int uiRevLen;

	struct sendRevDataEx_info sendRevData;
	struct sendData_info sendData;
	int ucResult;

	
	//memset( aucSendData,0,sizeof(aucSendData) );
	//memset( aucRevData,0,sizeof(aucRevData) );
	//uiRevLen = 0;
	//ucSendLen = 0;
	//aucSendData[ucSendLen++] = CLASSIC_ORDER_RESTORE;
	//aucSendData[ucSendLen++] = ucBlockIndex;
	//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
	//uiRevLen = sizeof(aucRevData);			 
	//ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );
	memset( &sendRevData,0,sizeof(struct sendRevDataEx_info));		
	memset( &sendData,0,sizeof(struct sendData_info));	
	sendRevData.ucCheckMode =  PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;

	sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_RESTORE;
	sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

	sendRevData.puiRevLen= sizeof(sendRevData.pucRevData); 
	sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
	ucResult = ioctl(fd,PCD_IOC_SENDREVDATAEX,&sendRevData);	

	if( ucResult==PCD_ERR_SUCCESS )
	{
		if( (sendRevData.pucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
		{
			//uiRevLen = 0;
			//ucSendLen = CLASSIC_VALUE_LEN;
			//memset( aucSendData,0,CLASSIC_VALUE_LEN );
			//ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
			//ucResult = PCD_SendData( ucCheckMode,aucSendData,ucSendLen );
			sendData.ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
			sendData.uiSendLen = CLASSIC_VALUE_LEN;
			memcpy( sendData.pucSendData,0,CLASSIC_VALUE_LEN );
			ucResult = ioctl(fd,PCD_IOC_SENDDATA,&sendData);
		}else
			ucResult = CLASSIC_ERR_RESTORE;
	}
	return ucResult;
}

int CLASSIC_Reset(int fd,int reset) 
{
    return ioctl(fd,PCD_IOC_RESET,&reset);
}

int  CLASSIC_PollCard(int fd,unsigned char *pucCardType,unsigned char *pucUid,
                           unsigned char *pucUidLen,unsigned char *pucAtq,unsigned char *pucAtqLen )
{
	struct card_attr_info cardAttr;
    int  ucResult;
	
	memset(&cardAttr,0,sizeof(struct card_attr_info));
	cardAttr.pucUidLen = *pucUidLen;
	cardAttr.pucAtqLen = *pucAtqLen;	
	ucResult = ioctl(fd,PCD_IOC_POLLCARD,&cardAttr);
	
	if (!ucResult) {
		*pucCardType = cardAttr.pucCardType;

		*pucUidLen = cardAttr.pucUidLen;
		memcpy (pucUid,cardAttr.pucUid,*pucUidLen);
		
		*pucAtqLen = cardAttr.pucAtqLen;
		memcpy (pucAtq,cardAttr.pucAtq,*pucAtqLen);
	}
	return ucResult;
}

int CLASSIC_Wupa(int fd, unsigned char *pucRevData,unsigned char *pucRevLen )
{
	struct wakeup_info wakeupa;
	int  ucResult;
	
	memset(&wakeupa,0,sizeof(struct wakeup_info));	
	wakeupa.pucRevLen = *pucRevLen;
	
	ucResult = ioctl(fd,PCD_IOC_WAKEUPA,&wakeupa);
	*pucRevLen = wakeupa.pucRevLen ;

    memcpy (pucRevData,wakeupa.pucRevData,*pucRevLen);
	return ucResult;
}

int CLASSIC_Select(int fd, unsigned char *pucUid,
	                         unsigned char ucUidLen,unsigned char *pucSak )
{

	struct card_id_info cardInfo;
	int  ucResult;
	memset(&cardInfo,0,sizeof(struct card_id_info));	
	
	cardInfo.ucUidLen = ucUidLen;
	memcpy(cardInfo.pucUid,pucUid,ucUidLen);
	ucResult = ioctl(fd,PCD_IOC_SELECT,&cardInfo);
	*pucSak = cardInfo.pucSak;

	return ucResult;
}
	
