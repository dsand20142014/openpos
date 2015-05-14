/*
--------------------------------------------------------------------------
	FILE  classic.c
--------------------------------------------------------------------------
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
//#include <linux/ioctl.h>
#include "osmifare.h"
#include "classic.h"
#include <string.h>

#define dbg 0

static int fd_rc531;

int Os_open_rf()
{
    fd_rc531 = open("/dev/rc531", O_RDWR) ;
    if (fd_rc531 < 0 )
    {
        printf ("open error.\n") ;
        return -1 ;
    }
}


int Os_close_rf()
{
    if (fd_rc531) close(fd_rc531);
}

#define CLASSIC_ORDER_READ	    	0x30
#define CLASSIC_ORDER_WRITE	      	0xA0
#define CLASSIC_ORDER_INC	      	0xC1
#define CLASSIC_ORDER_DEC			0xC0
#define CLASSIC_ORDER_RESTORE		0xC2
#define CLASSIC_ORDER_TRANSFER		0xB0
#define CLASSIC_ORDER_AUTHA			0x60
#define CLASSIC_ORDER_AUTHB			0x61

CLASSIC_ORDER_TYPE *pClassicOrder;
unsigned char CLASSIC_AuthBlock_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );
unsigned char CLASSIC_ReadBlock_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );
unsigned char CLASSIC_WriteBlock_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );
unsigned char CLASSIC_Increment_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );
unsigned char CLASSIC_Decrement_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );
unsigned char CLASSIC_Restore_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );
unsigned char CLASSIC_Transfer_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );

unsigned char CLASSIC_Process( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char ucResult;

    pClassicOrder = &pOrder->pt_order_in->classic_order;
//	if (dbg) ("%s %d, pClassicOrder->order_type=%#02x\n", __func__, __LINE__,pClassicOrder->order_type);
    switch ( pClassicOrder->order_type )
    {
    case CLASSIC_ORDER_AUTHA:
    case CLASSIC_ORDER_AUTHB:
        ucResult = CLASSIC_AuthBlock_lib( pAnswer,pOrder );
        break;
    case CLASSIC_ORDER_READ:
        if (dbg) ("%s %d Os__MIFARE_command\n", __func__, __LINE__);
        ucResult = CLASSIC_ReadBlock_lib( pAnswer,pOrder );
        break;
    case CLASSIC_ORDER_WRITE:
        ucResult = CLASSIC_WriteBlock_lib( pAnswer,pOrder );
        break;
    case CLASSIC_ORDER_INC:
        ucResult = CLASSIC_Increment_lib( pAnswer,pOrder );
        break;
    case CLASSIC_ORDER_DEC:
        ucResult = CLASSIC_Decrement_lib( pAnswer,pOrder );
        break;
    case CLASSIC_ORDER_RESTORE:
        ucResult = CLASSIC_Restore_lib( pAnswer,pOrder );
        break;
    case CLASSIC_ORDER_TRANSFER:
        ucResult = CLASSIC_Transfer_lib( pAnswer,pOrder );
        break;
    }
    return ucResult;
}

unsigned char CLASSIC_AuthBlock_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char ucAuthType,ucKeyType,ucBlockIndex,aucKey[OSMIFARE_KEY_LEN];
    unsigned char ucResult;

    ucKeyType = 0;
    if ( pClassicOrder->order_type==CLASSIC_ORDER_AUTHB )
        ucKeyType = 1;
    ucBlockIndex = pClassicOrder->ucBlockIndex;
    ucAuthType = pClassicOrder->ucAuthType;
    memcpy( aucKey,pClassicOrder->aucInData,sizeof(aucKey) );
    ucResult = OSMIFARE_Auth( ucAuthType,ucBlockIndex,ucKeyType,aucKey );
    pAnswer->card_status = ucResult;
    return ucResult;
}

unsigned char CLASSIC_ReadBlock_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult, i=0;

    if (dbg) ("%s %d 1111111\n", __func__, __LINE__);

    pAnswer->card_status = CLASSIC_ERR_READ;
    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = pClassicOrder->order_type;
    aucSendData[ucSendLen++] = pClassicOrder->ucBlockIndex;

    if (dbg) ("%s %d 2222222\n", __func__, __LINE__);

    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_RXTXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_READ );
    if (dbg) ("\n\nCLASSIC_ReadBlock:ucRevLen=%d\n", ucRevLen);
    for (i=0; i<ucRevLen; i++)
        if (dbg) ("%02x ", aucRevData[i]);
    if (dbg) ("\n\n\n");
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
        if ( ucRevLen )
        {
            pAnswer->Len = ucRevLen;
            memcpy( pOrder->ptout,aucRevData,ucRevLen );
        }
        pAnswer->card_status = CLASSIC_ERR_SUCCESS;
    }
    return ucResult;
}

unsigned char CLASSIC_WriteBlock_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult;

    pAnswer->card_status = CLASSIC_ERR_WRITE;
    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = pClassicOrder->order_type;
    aucSendData[ucSendLen++] = pClassicOrder->ucBlockIndex;
    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
        if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
        {
            ucRevLen = 0;
            ucSendLen = CLASSIC_BLOCK_LEN;
            memcpy( aucSendData,pClassicOrder->aucInData,CLASSIC_BLOCK_LEN );
            ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                             aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
            if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
                pAnswer->card_status = CLASSIC_ERR_SUCCESS;
        }
    }
    return ucResult;
}

unsigned char CLASSIC_TransferBlock_lib( unsigned char ucBlockIndex )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult;

    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = 0xB0;
    aucSendData[ucSendLen++] = ucBlockIndex;
    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
        if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
        {
            ucResult = OSMIFARE_ERR_SUCCESS;
        }
        else
            ucResult = OSMIFARE_ERR_TRANS;
    }
    return ucResult;
}

unsigned char CLASSIC_Increment_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult;

    pAnswer->card_status = CLASSIC_ERR_INC;
    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = pClassicOrder->order_type;
    aucSendData[ucSendLen++] = pClassicOrder->ucBlockIndex;
    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
        if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
        {
            ucRevLen = 0;
            ucSendLen = CLASSIC_VALUE_LEN;
            memcpy( aucSendData,pClassicOrder->aucInData,CLASSIC_BLOCK_LEN );
            ucResult = OSMIFARE_SendData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen );
            if ( ucResult==OSMIFARE_ERR_SUCCESS )
            {
//				ucResult = CLASSIC_TransferBlock(pClassicOrder->ucBlockIndex );
//				if( ucResult==OSMIFARE_ERR_SUCCESS )
                pAnswer->card_status = CLASSIC_ERR_SUCCESS;
            }
        }
    }
    return ucResult;
}

unsigned char CLASSIC_Decrement_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult;

    pAnswer->card_status = CLASSIC_ERR_DEC;
    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = pClassicOrder->order_type;
    aucSendData[ucSendLen++] = pClassicOrder->ucBlockIndex;
    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
        if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
        {
            ucRevLen = 0;
            ucSendLen = CLASSIC_VALUE_LEN;
            memcpy( aucSendData,pClassicOrder->aucInData,CLASSIC_BLOCK_LEN );
            ucResult = OSMIFARE_SendData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen );
            if ( ucResult==OSMIFARE_ERR_SUCCESS )
            {
//				ucResult = CLASSIC_TransferBlock(pClassicOrder->ucBlockIndex );
//			if( ucResult==OSMIFARE_ERR_SUCCESS )
                pAnswer->card_status = CLASSIC_ERR_SUCCESS;
            }
        }
    }
    return ucResult;
}

unsigned char CLASSIC_Transfer_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult;

    pAnswer->card_status = CLASSIC_ERR_TRANSFER;
    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = pClassicOrder->order_type;
    aucSendData[ucSendLen++] = pClassicOrder->ucBlockIndex;
    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
        if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
        {
            pAnswer->card_status = CLASSIC_ERR_SUCCESS;
        }
    }
    return ucResult;
}

unsigned char CLASSIC_Restore_lib( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
    unsigned char aucSendData[CLASSIC_SEND_MAX_LEN],ucSendLen;
    unsigned char aucRevData[CLASSIC_REV_MAX_LEN],ucRevLen;
    unsigned char ucResult;

    if (dbg) ("%s %d\n", __func__, __LINE__);
    pAnswer->card_status = CLASSIC_ERR_RESTORE;
    memset( aucSendData,0,sizeof(aucSendData) );
    memset( aucRevData,0,sizeof(aucRevData) );
    ucRevLen = 0;
    ucSendLen = 0;
    aucSendData[ucSendLen++] = pClassicOrder->order_type;
    aucSendData[ucSendLen++] = pClassicOrder->ucBlockIndex;
    if (dbg) ("%s %d\n", __func__, __LINE__);
    ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
                                     aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
    if (dbg) ("%s %d, ucResult=%x,aucRevData[0]=%x\n", __func__, __LINE__, ucResult, aucRevData[0]);
    if ( ucResult==OSMIFARE_ERR_SUCCESS )
    {
                if (dbg) ("%s %d\n", __func__, __LINE__);
        if ( (aucRevData[0]&0x0F)==CLASSIC_RESPOND_ACK )
        {
            if (dbg) ("%s %d\n", __func__, __LINE__);
            ucRevLen = 0;
            ucSendLen = CLASSIC_VALUE_LEN;
            memset( aucSendData,0,sizeof(CLASSIC_VALUE_LEN) );
//			ucResult = OSMIFARE_SendRevData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen,
//											aucRevData,&ucRevLen,CLASSIC_DELAY_WRITE );
            if (dbg) ("%s %d\n", __func__, __LINE__);

            ucResult = OSMIFARE_SendData( OSMIFARE_CHECK_TXPARITYEN,aucSendData,ucSendLen );
            if (dbg) ("%s %d ucResult=%x\n", __func__, __LINE__,ucResult);

            if ( ucResult==OSMIFARE_ERR_SUCCESS )
            {
                pAnswer->card_status = CLASSIC_ERR_SUCCESS;
            }
        }
    }
            if (dbg) ("%s %d\n", __func__, __LINE__);
	
    return ucResult;
}


#define PCD_ERR_SUCCESS 		0 	//ÕýÈ·

int classic_test(char argc, char **argv)
{
    unsigned char ucStatus,ucCardType,aucUid[32],ucUidLen,aucAtq[2],ucAtqLen,ucI,ucSak;
    unsigned char aucData[32], i=10,ii=0, val=0x0;
    unsigned char data[64], ret, blockID=0;


    ret=Os_open_rf();
    if (ret<0)
    {
        printf("%s open rf device failed!\n", __func__);
        return -1;
    }




#if 0
    ret=CLASSIC_Reset(1000);
    ucUidLen = sizeof(aucUid);
    ucAtqLen = sizeof(aucAtq);
    do
    {
        memset(aucUid, 0, 32);
        ucStatus = CLASSIC_PollCard( &ucCardType,aucUid,&ucUidLen,aucAtq,&ucAtqLen );
        if ( ucStatus!=PCD_ERR_SUCCESS)
            CLASSIC_Reset(1000);
    }
    while ( ucStatus!=PCD_ERR_SUCCESS );

    if ( ucStatus==PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_Wupa( aucAtq,&ucAtqLen );
    printf("PCD_Wupa ucStatus=%x\n\r\r\r", ucStatus);

    if ( ucStatus==PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_Select( aucUid,ucUidLen,&ucSak );
    printf("PCD_Select ucStatus=%x, ucSak=%x\n\r", ucStatus, ucSak);



    if ( ucStatus==PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_AuthBlock_lib( 0x01,7,0x01,(unsigned char *)"\xff\xff\xff\xff\xff\xff" );
    printf("CLASSIC_AuthBlock_lib ucStatus=%x\n\r", ucStatus);


    blockID = 7*4;
    if ( ucStatus==PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_ReadBlock( blockID,aucData );
    printf("CLASSIC_ReadBlock ucStatus=%x\n\r", ucStatus);
    for (i=0; i<16; i++)
        printf("block%d: %d %x \n\r", blockID, i, aucData[i]);


    for (i=0; i<16; i++)
        aucData[i]=i;
    //blockID++;
    if ( ucStatus==PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_WriteBlock( blockID,aucData );
    printf("CLASSIC_WriteBlock ucStatus=%x\n\r", ucStatus);
    for (i=0; i<16; i++)
        if (dbg) ("block%d: %d %x \n\r", blockID, i, aucData[i]);


    if ( ucStatus==PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_ReadBlock( blockID,aucData );
    if (dbg) ("CLASSIC_ReadBlock ucStatus=%x\n\r", ucStatus);
    for (i=0; i<16; i++)
        if (dbg) ("block%d: %d %x \n\r", blockID, i, aucData[i]);
#endif

    Os_close_rf();

}

