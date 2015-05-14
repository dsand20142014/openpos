/*mifare APIs for india project*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
//#include <linux/ioctl.h>
#include "osmifare.h"
//#include "classic.h"
#include <string.h>
#include "wjc.h"


static int fd_rc531;


/*
*@file CLASSIC_CMD_Open
*@brief 初始化设备
*@param void
*@return >0 OK
*/
int CLASSIC_CMD_Open(void)
{
    fd_rc531 = open("/dev/rc531", O_RDWR) ;

    if (fd_rc531 < 0 )
    {
        printf ("open error.\n") ;
        return -1 ;
    }

    printf("fd_rc531=%d\n", fd_rc531);
    return fd_rc531;
}

/*
*@file CLASSIC_CMD_Close
*@brief 关闭设备
*@param void
*@return >0 OK
*/
int CLASSIC_CMD_Close(void)
{
    if (fd_rc531) close(fd_rc531);
}




void CMD_Power( void )
{
    ioctl(fd_rc531, PCD_IOC_POWERON);
}


void CMD_Reset( void )
{
    ioctl(fd_rc531, PCD_IOC_RESET);

}

void CMD_Wupa( void )
{

}

void CMD_Reqa( void )
{

}

void CMD_Hlta( void )
{

}

void CMD_Wupb( void )
{

}

void CMD_Reqb( void )
{

}

void CMD_Hltb( void )
{

}

void CMD_PollCard( void )
{

}


void CMD_Acta( void )
{

}

void CMD_Actb( void )
{

}

void CMD_Remove( void )
{

}


int  CLASSIC_CMD_PollCard(unsigned char *pucCardType, unsigned char *pucUid,
                          unsigned char *pucUidLen, unsigned char *pucAtq, unsigned char *pucAtqLen )
{
    struct card_attr_info cardAttr;
    int  ucResult;

    memset(&cardAttr, 0, sizeof(struct card_attr_info));
    cardAttr.pucUidLen = *pucUidLen;
    cardAttr.pucAtqLen = *pucAtqLen;
    ucResult = ioctl(fd_rc531, PCD_IOC_POLLCARD, &cardAttr);

    if (!ucResult)
    {
        *pucCardType = cardAttr.pucCardType;

        *pucUidLen = cardAttr.pucUidLen;
        memcpy (pucUid, cardAttr.pucUid, *pucUidLen);

        *pucAtqLen = cardAttr.pucAtqLen;
        memcpy (pucAtq, cardAttr.pucAtq, *pucAtqLen);
    }

    return ucResult;
}



int CLASSIC_CMD_AuthBlock( unsigned char ucAuthType, unsigned char ucSectorIndex,
                           unsigned char ucKeyType, unsigned char *pucKey )
{
    int ucResult;
    struct auth_data_info authData;

    memset( &authData, 0, sizeof(struct auth_data_info) );

    authData.ucAuthType = ucAuthType ;
    authData.ucSectorIndex = ucSectorIndex;
    authData.ucKeyType = ucKeyType ;

    if (strlen(pucKey) != sizeof(authData.pucKey))
    {
        printf("pucKey length is not 6.\n");
        return -1;
    }

    //memcpy (authData.pucKey,pucKey,sizeof(authData.pucKey));
    memcpy (authData.pucKey, pucKey, 6);
    ucResult = ioctl(fd_rc531, PCD_IOC_AUTH, &authData);
    return ucResult;
}

int CLASSIC_CMD_ReadBlock(unsigned char ucBlockIndex, unsigned char *pucBlockData )
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

    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info) );
    sendRevData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN | PCD_CHECK_RXCRCEN;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_READ;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;
    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = 100;

    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( sendRevData.puiRevLen )
        {
            memcpy( pucBlockData, sendRevData.pucRevData, CLASSIC_BLOCK_LEN );
        }
    }

    return ucResult;
}


int CLASSIC_CMD_WriteBlock(unsigned char ucBlockIndex, unsigned char *pucBlockData )
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
    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info) );
    sendRevData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_WRITE;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;
    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
    //ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );
    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( (sendRevData.pucRevData[0] & 0x0F) == CLASSIC_RESPOND_ACK )
        {
            //uiRevLen = 0;
            //ucSendLen = CLASSIC_BLOCK_LEN;
            sendRevData.puiRevLen = 0;
            sendRevData.uiSendLen = CLASSIC_BLOCK_LEN;
            //memcpy( aucSendData,pucBlockData,CLASSIC_BLOCK_LEN );
            memcpy( sendRevData.pucSendData, pucBlockData, CLASSIC_BLOCK_LEN );
            //ucResult = PCD_SendRevDataEx( ucCheckMode,aucSendData,ucSendLen,aucRevData,&uiRevLen,CLASSIC_DELAY_WRITE );
            ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

            if ( ucResult == PCD_ERR_SUCCESS )
            {
                if ( (sendRevData.pucRevData[0] & 0x0F) != CLASSIC_RESPOND_ACK )
                    ucResult = CLASSIC_DELAY_WRITE;
            }
        }
        else
            ucResult = CLASSIC_DELAY_WRITE;
    }

    return ucResult;
}

int CLASSIC_CMD_TransferBlock(unsigned char ucBlockIndex )
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
    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info) );
    sendRevData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;

    sendRevData.pucSendData[sendRevData.uiSendLen++] = 0xB0;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( (sendRevData.pucRevData[0] & 0x0F) != CLASSIC_RESPOND_ACK )
            ucResult = CLASSIC_ERR_TRANSFER;
    }

    return ucResult;
}
#if 0
int CLASSIC_CMD_Increment(unsigned char ucBlockIndex, unsigned long ulValue )
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
    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info));
    memset( &sendData, 0, sizeof(struct sendData_info));
    sendRevData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;

    sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_INC;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( (sendRevData.pucRevData[0] & 0x0F) == CLASSIC_RESPOND_ACK )
        {
            //uiRevLen = 0;
            //ucSendLen = CLASSIC_VALUE_LEN;

            //memcpy( aucSendData,&ulValue,CLASSIC_VALUE_LEN );
            //ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
            sendData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;
            memcpy( sendData.pucSendData , &ulValue, CLASSIC_VALUE_LEN );
            sendData.uiSendLen = CLASSIC_VALUE_LEN;

            //ucResult = PCD_SendData( ucCheckMode,aucSendData,ucSendLen );
            ucResult = ioctl(fd_rc531, PCD_IOC_SENDDATA, &sendData);


            if ( ucResult == PCD_ERR_SUCCESS )
            {
                ucResult = CLASSIC_TransferBlock( ucBlockIndex );
            }
        }
        else
            ucResult = CLASSIC_ERR_INC;
    }
    return ucResult;

}

int CLASSIC_CMD_Decrement(unsigned char ucBlockIndex, unsigned long ulValue )
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

    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info));
    memset( &sendData, 0, sizeof(struct sendData_info));
    sendRevData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;

    sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_DEC;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( (sendRevData.pucRevData[0] & 0x0F) == CLASSIC_RESPOND_ACK )
        {
            //uiRevLen = 0;
            //ucSendLen = CLASSIC_VALUE_LEN;
            //memcpy( aucSendData,&ulValue,CLASSIC_VALUE_LEN );
            //ucResult = PCD_SendData( ucCheckMode,aucSendData,ucSendLen );

            sendData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;
            sendData.uiSendLen = CLASSIC_VALUE_LEN;
            memcpy( sendData.pucSendData, &ulValue, CLASSIC_VALUE_LEN );
            ucResult = ioctl(fd_rc531, PCD_IOC_SENDDATA, &sendData);

            if ( ucResult == PCD_ERR_SUCCESS )
            {
                ucResult = CLASSIC_TransferBlock( ucBlockIndex );
            }
        }
        else
            ucResult = CLASSIC_ERR_DEC;
    }

    return ucResult;
}

int CLASSIC_CMD_Transfer(unsigned char ucBlockIndex )
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
    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info));
    sendRevData.ucCheckMode =  PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;

    sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_TRANSFER;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( (sendRevData.pucRevData[0] & 0x0F) != CLASSIC_RESPOND_ACK )
            ucResult = CLASSIC_ERR_TRANSFER;
    }

    return ucResult;
}

int CLASSIC_CMD_Restore(unsigned char ucBlockIndex )
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
    memset( &sendRevData, 0, sizeof(struct sendRevDataEx_info));
    memset( &sendData, 0, sizeof(struct sendData_info));
    sendRevData.ucCheckMode =  PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;

    sendRevData.pucSendData[sendRevData.uiSendLen++] = CLASSIC_ORDER_RESTORE;
    sendRevData.pucSendData[sendRevData.uiSendLen++] = ucBlockIndex;

    sendRevData.puiRevLen = sizeof(sendRevData.pucRevData);
    sendRevData.uiTimeDelay = CLASSIC_DELAY_WRITE;
    ucResult = ioctl(fd_rc531, PCD_IOC_SENDREVDATAEX, &sendRevData);

    if ( ucResult == PCD_ERR_SUCCESS )
    {
        if ( (sendRevData.pucRevData[0] & 0x0F) == CLASSIC_RESPOND_ACK )
        {
            //uiRevLen = 0;
            //ucSendLen = CLASSIC_VALUE_LEN;
            //memset( aucSendData,0,CLASSIC_VALUE_LEN );
            //ucCheckMode = PCD_CHECK_PARITYODD|PCD_CHECK_PARITYEN|PCD_CHECK_TXCRCEN;
            //ucResult = PCD_SendData( ucCheckMode,aucSendData,ucSendLen );
            sendData.ucCheckMode = PCD_CHECK_PARITYODD | PCD_CHECK_PARITYEN | PCD_CHECK_TXCRCEN;
            sendData.uiSendLen = CLASSIC_VALUE_LEN;
            memcpy( sendData.pucSendData, 0, CLASSIC_VALUE_LEN );
            ucResult = ioctl(fd_rc531, PCD_IOC_SENDDATA, &sendData);
        }
        else
            ucResult = CLASSIC_ERR_RESTORE;
    }

    return ucResult;
}
#endif
int CLASSIC_CMD_Reset(int delay)
{
    return ioctl(fd_rc531, PCD_IOC_RESET, &delay);
}



int CLASSIC_CMD_Wupa(unsigned char *pucRevData, unsigned char *pucRevLen )
{
    struct wakeup_info wakeupa;
    int  ucResult;

    memset(&wakeupa, 0, sizeof(struct wakeup_info));
    wakeupa.pucRevLen = *pucRevLen;

    ucResult = ioctl(fd_rc531, PCD_IOC_WAKEUPA, &wakeupa);
    *pucRevLen = wakeupa.pucRevLen ;

    memcpy (pucRevData, wakeupa.pucRevData, *pucRevLen);
    return ucResult;
}

int CLASSIC_CMD_Select(unsigned char *pucUid,
                       unsigned char ucUidLen, unsigned char *pucSak )
{

    struct card_id_info cardInfo;
    int  ucResult;
    memset(&cardInfo, 0, sizeof(struct card_id_info));

    cardInfo.ucUidLen = ucUidLen;
    memcpy(cardInfo.pucUid, pucUid, ucUidLen);
    ucResult = ioctl(fd_rc531, PCD_IOC_SELECT, &cardInfo);
    *pucSak = cardInfo.pucSak;

    return ucResult;
}


unsigned char cmd_rf_test()
{
    unsigned char ucStatus, ucCardType, aucUid[32], ucUidLen, aucAtq[2], ucAtqLen, ucSak;
    unsigned char aucData[32];
    unsigned char xaucData[32];

    int i;
    ucUidLen = sizeof(aucUid);
    ucAtqLen = sizeof(aucAtq);

    if (!CLASSIC_CMD_Open()) return -1;

    CLASSIC_CMD_Reset(1000);
    do
    {
        ucStatus = CLASSIC_CMD_PollCard( &ucCardType, aucUid, &ucUidLen, aucAtq, &ucAtqLen);
        if (ucStatus!=PCD_ERR_SUCCESS)
            CLASSIC_CMD_Reset(1000);
    }
    while (ucStatus != PCD_ERR_SUCCESS);

    printf("******************** card info: len=%d, ucCardType=%#x *******************\n",
           ucUidLen, ucCardType);
    for (i = 0; i < ucUidLen; i++)
        printf("%02x", aucUid[i]);
    printf("\n");

    if ( ucStatus == PCD_ERR_SUCCESS )
        CLASSIC_CMD_Wupa(aucAtq, &ucAtqLen);


    if ( ucStatus == PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_CMD_Select( aucUid, ucUidLen, &ucSak );

    if ( ucStatus == PCD_ERR_SUCCESS )
        ucStatus = CLASSIC_CMD_AuthBlock(0x01, 7, 0x01, (unsigned char *)"\xff\xff\xff\xff\xff\xff" );

    if ( ucStatus == PCD_ERR_SUCCESS )
    {
        ucStatus = CLASSIC_CMD_ReadBlock( 28, aucData );
        printf("CLASSIC_ReadBlock 28 ucStatus %d\n", ucStatus);

        for (i = 0; i < 16; i++) printf("%#x(%d) ", aucData[i], i);
        printf("\n");
    }

    for (i = 0; i < 16; i++) aucData[i] = i;

    if ( ucStatus == PCD_ERR_SUCCESS )
    {
        ucStatus = CLASSIC_CMD_WriteBlock( 28, aucData );
        printf("\n\nCLASSIC_WriteBlock 28 ucStatus %d\n", ucStatus);

        for (i = 0; i < 16; i++) printf("%#x(%d) ", aucData[i], i);
        printf("\n");
    }

    if ( ucStatus == PCD_ERR_SUCCESS )
    {
        ucStatus = CLASSIC_CMD_ReadBlock( 28, aucData );
        printf("\n\nCLASSIC_ReadBlock 28 ucStatus %d\n", ucStatus);

        for (i = 0; i < 16; i++) printf("%#x(%d) ", aucData[i], i);
        printf("\n");
    }


    CLASSIC_CMD_Close();
}


