/*
 icc 应用层接口，come from apiing.c

*/
#include <stdio.h>
#include <haldrv.h>
#include <osicc.h>
#include <osdriver.h>
#include "osmifare.h"
#include <string.h>
#include "oslib.h"
#include <unistd.h>


/* Smart cards and SIMs reader-writer */
/* OK 0 , has a card, 1 not*/
unsigned char Os__ICC_detect(unsigned char ucReader)
{
    return(OSICC_Detect(ucReader));
}

ICC_ANSWER *Os__ICC_command(unsigned char ucReader, ICC_ORDER *pOrder)
{
    return(OSICC_Command(ucReader,pOrder));
}
/*
DRV_OUT *Os__ICC_insert(void)
{
    return((DRV_OUT *)OSICC_Insert());
}
*/
DRV_OUT *Os__ICC_Async_order(unsigned char *pucOrder,unsigned short uiLen)
{
    return((DRV_OUT *)OSICC_OrderAsync(pucOrder,uiLen));
}

DRV_OUT *Os__ICC_order(DRV_IN  *pDrvIn)
{
    return((DRV_OUT *)OSICC_Order((DRVIN *)pDrvIn));
}

void Os__ICC_remove(void)
{
    OSICC_Remove();
}

void Os__ICC_off(void)
{
    OSICC_PowerOff();
}


//0717增加
MIFARE_ANSWER * Os__MIFARE_command( MIFIRE_ORDER *pOrder )
{
    return((MIFARE_ANSWER * )OSMIFARE_command(pOrder));
}

//DRV_OUT *Os__MIFARE_Request(unsigned char ucCardType,unsigned char ucReqMode)
//{
//    return ((DRV_OUT *)OSMIFARE_Request(ucCardType, ucReqMode));
//}


unsigned char Os__MIFARE_Request(unsigned char ucCardType,unsigned char ucReqMode)
{
    return (OSMIFARE_Request(ucCardType, ucReqMode));
}

unsigned char Os__MIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,unsigned char *pucCardIdLen,unsigned char *pucCardClass )
{
    return OSMIFARE_Anti( ucAntiEn,pucCardId,pucCardIdLen,pucCardClass );
}

unsigned char Os__MIFARE_Select( unsigned char *pucCardID,unsigned char ucCardIDLen,unsigned char *pucCardClass )
{
    return OSMIFARE_Select( pucCardID,ucCardIDLen,pucCardClass );
}

unsigned char OSMIFARE_Auth( unsigned char ucAuthType,unsigned char ucSectorIndex,unsigned char ucKeyType,
                             unsigned char *pucKey );
unsigned char Os__MIFARE_Auth( unsigned char ucAuthType,unsigned char ucSectorIndex,unsigned char ucKeyType,
                               unsigned char *pucKey )
{
    return OSMIFARE_Auth( ucAuthType,ucSectorIndex,ucKeyType,pucKey );
}

unsigned char Os__MIFARE_SendData( unsigned char ucCheckMode,unsigned char *pucSendData,unsigned int uiSendLen )
{
    return OSMIFARE_SendData( ucCheckMode,pucSendData,uiSendLen );
}

unsigned char Os__MIFARE_SendRevFrame( unsigned char *pucSendFrame,unsigned int uiSendLen,
                                       unsigned char *pucRevFrame,unsigned int *puiRevLen,unsigned int uiTimeDelay )
{
    return OSMIFARE_SendRevFrame( pucSendFrame,uiSendLen,pucRevFrame,puiRevLen,uiTimeDelay );
}

unsigned char Os__MIFARE_DeSelect( void )
{
    return OSMIFARE_DeSelect();
}

unsigned char Os__MIFARE_Halt( void )
{
    return OSMIFARE_Halt();
}

unsigned char Os__MIFARE_Remove( void )
{
    return OSMIFARE_Remove();
}

/*
0 close
1 open
*/
unsigned char Os__MIFARE_PowerRF( unsigned char ucOnOff )
{
    return OSMIFARE_PowerRF( ucOnOff );
}

/*DRV_OUT *Os__MIFARE_Polling(void)
{
    return (DRV_OUT *)OSMIFARE_Polling();
}*/

unsigned char Os__MIFARE_Active(void)
{
    return OSMIFARE_Active();
}

extern DRVOUT	myOSICCDrv;//用于直接访问
extern void OSICC_Init(void);
extern unsigned char Os__ICC_detect(unsigned char ucReader);
extern void OSDRV_Init(void);
/*
int icc_main()
{
    DRV_OUT *ptIcc;

    printf("come from cj\n");
//    ptIcc = Os__ICC_insert();
    while (ptIcc->gen_status!=DRV_ENDED);

    if ( ptIcc->gen_status==DRV_ENDED )
    {
        Uart_Printf("\n***ptIcc->gen_status=%x***ptIcc->xxdata[1]=%x*\n",ptIcc->gen_status,ptIcc->xxdata[1]);

        Os__xprint((unsigned char*)"Async. ICC ok");
    }
    return 0;
}
*/
/*
unsigned char TESTPAD_TestMFCCard(void)
{
    unsigned char ucFlag,ucResult;
    unsigned char ucKey;
    unsigned char ucCardLen;
    unsigned char ucCardClass;
    unsigned char aucBuf[21];
    unsigned char aucCardId[64];

    DRVOUT *pKeyDrvOut;
    DRVOUT *pMFCDrvOut;
    DRVOUT *pPCIDrvOut;

    HALDRV_Install(&HALPADDrv);
    HALDRV_Install(&HALMIFAREDrv);
    OSMMI_ClrDisplay(ASCIIFONT57,255);
    OSMMI_DisplayASCII(ASCIIFONT57,0,0,OSMSG_TestPutMFCCard);
    do
    {
        pKeyDrvOut = OSMMI_GetKey();
        pMFCDrvOut = OSPAD_MFCQuest();
        ucFlag = 0;
        do
        {
            pPCIDrvOut = OSMIFARE_Request(OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_REQA);
            if (!ucFlag)
            {
                ucFlag = 1;
            }
            if (!pPCIDrvOut->aucData[0])
                pPCIDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
            else
                pPCIDrvOut->ucGenStatus = HALDRVSTATE_RUNNING;
        }
        while ( (pKeyDrvOut->ucGenStatus != HALDRVSTATE_ENDED )
                &&(pMFCDrvOut->ucGenStatus != HALDRVSTATE_ENDED )
                &&(pPCIDrvOut->ucGenStatus != HALDRVSTATE_ENDED )
              );


        if ( (pMFCDrvOut->ucGenStatus == HALDRVSTATE_ENDED)
                ||(pPCIDrvOut->ucGenStatus == HALDRVSTATE_ENDED)
           )
        {
            OSDRV_Abort(HALDRVID_MMI);
            if (pPCIDrvOut->ucGenStatus == HALDRVSTATE_ENDED)
            {
                OSDRV_Abort(HALDRVID_PAD);
                ucResult = OSMIFARE_Anti( 1,aucCardId,&ucCardLen,&ucCardClass );
            }
            else
            {
                ucResult = pMFCDrvOut->aucData[0];
                memcpy( aucCardId,&pMFCDrvOut->aucData[1],4 );
                ucCardLen = 4;
            }
        }
    }
    while (1);
}

*/

#if 1
/*
mifare API Test
code come from sandpay & libVikang
*/
MIFIRE_ORDER 			  MIFARE_Order;
MIFARE_ANSWER 			*MIFARE_pAnswer;
MIFARE_ORDER_IN_TYPE 	  MIFARE_Type;

unsigned char MIFARE_Halt( void )
{
    unsigned char aucRevData[16];

    MIFARE_Order.order = OSMIFARE_HALT_ORDER;
    MIFARE_Order.ptout = aucRevData;
    MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
    if ( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
    {
        return (MIFARE_pAnswer->drv_status);
    }
    return( MIFARE_pAnswer->card_status );
}

unsigned char MIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,
                           unsigned char *pucCardIdLen,unsigned char *pucCardClass )
{
    unsigned char aucRevData[16];

    MIFARE_Order.order = OSMIFARE_ANTI_ORDER;
    MIFARE_Order.ptout = aucRevData;
    MIFARE_Order.pt_order_in = &MIFARE_Type;
    MIFARE_Type.base_order.ucInDataLen = 1;
    MIFARE_Type.base_order.aucInData[0] = ucAntiEn;
    MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
    if ( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
    {
        return (MIFARE_pAnswer->drv_status);
    }
    if ( MIFARE_pAnswer->Len )
    {
        *pucCardClass = aucRevData[0];
        memcpy( pucCardId,&aucRevData[1],MIFARE_pAnswer->Len-1 );
        *pucCardIdLen = MIFARE_pAnswer->Len-1;
    }
    else
        *pucCardIdLen = 0;
    return( MIFARE_pAnswer->card_status );
}

#if 0
unsigned char MifareCard_GetUID( unsigned char *pucOutData )
{
    unsigned char aucCardId[32],ucLen,ucResult,ucCardClass;
    DRV_OUT *pxReuslt;

    MIFARE_Halt();
    pxReuslt = Os__MIFARE_Request( OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_WUPA );
    if ( pxReuslt->xxdata[0]==OSMIFARE_ERR_SUCCESS )
    {
        ucResult = MIFARE_Anti( 1,aucCardId,&ucLen,&ucCardClass );
    }
    else
        ucResult = 0x01;

    memcpy(pucOutData, aucCardId, ucLen);
    pucOutData[ucLen]=0x0;

    return ucResult;
}
#endif



unsigned char TESTPAD_TestMFCCard(void)
{
    unsigned char ucFlag,ucResult;
    unsigned char ucCardLen;
    unsigned char ucCardClass;
    unsigned char aucCardId[64], i;

    DRVOUT *pPCIDrvOut;


    do
    {
        ucFlag = 0;
        do
        {
            pPCIDrvOut = (DRVOUT *)OSMIFARE_Request(OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_REQA);
            if (!ucFlag)
            {
                ucFlag = 1;
            }
            if (!pPCIDrvOut->aucData[0])
                pPCIDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
            else
                pPCIDrvOut->ucGenStatus = HALDRVSTATE_RUNNING;
        }
        while ((pPCIDrvOut->ucGenStatus != HALDRVSTATE_ENDED )      );



        if (pPCIDrvOut->ucGenStatus == HALDRVSTATE_ENDED)
        {
            ucResult = OSMIFARE_Anti( 1,aucCardId,&ucCardLen,&ucCardClass );
        }
        Os__beep();
        for (i=0; i<ucCardLen; i++)
            printf("%02x ", aucCardId[i]);
        printf("\n");

    }
    while (1);
}


unsigned char TESTPAD_TestMFCWRCard(char select)
{
    unsigned char ucFlag,ucResult;
    unsigned char ucCardLen;
    unsigned char ucCardClass;
    unsigned char aucCardId[64], i;
    DRVOUT *pPCIDrvOut;

    ucFlag = 0;
    do
    {
        pPCIDrvOut = (DRVOUT *)Os__MIFARE_Request(OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_REQA);
        if (!ucFlag)
        {
            ucFlag = 1;
        }

        //printf("pPCIDrvOut->aucData[0,1,2]=%#x %#x %#x\n",
        //       pPCIDrvOut->aucData[0],pPCIDrvOut->aucData[1],pPCIDrvOut->aucData[2]);

        if (!pPCIDrvOut->aucData[0])
            pPCIDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
        else
            pPCIDrvOut->ucGenStatus = HALDRVSTATE_RUNNING;
    }
    while (pPCIDrvOut->ucGenStatus != HALDRVSTATE_ENDED );
    printf("~~~~~~~~~~~~~~~ OSMIFARE_Request OK!, pPCIDrvOut->ucGenStatus=%d, select=%d\n",
           pPCIDrvOut->ucGenStatus, select);
    // return 0;

    if (pPCIDrvOut->ucGenStatus == HALDRVSTATE_ENDED)
    {
        ucResult = OSMIFARE_Anti( 1,aucCardId,&ucCardLen,&ucCardClass );
        if (!ucResult)
        {
            for (i=0; i<ucCardLen; i++)
                printf("%02x ", aucCardId[i]);
            printf("\n");
            if (select==0) return 0;

            sleep(1);
            ucResult = OSMIFARE_Halt();
            printf("~~~~~~~~~~~~~~~ OSMIFARE_Halt ret=%d\n", ucResult);

            if ( !ucResult )
            {
                do
                {
                    printf("-------------------> to do OSMIFARE_Request  with OSMIFARE_REQUEST_WUPA\n");
                    pPCIDrvOut =(DRVOUT *)OSMIFARE_Request(OSMIFARE_CARD_TYPE_A,OSMIFARE_REQUEST_WUPA);
                    if (!pPCIDrvOut->aucData[0])
                        pPCIDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
                    else
                        pPCIDrvOut->ucGenStatus = HALDRVSTATE_RUNNING;
                }
                while ((pPCIDrvOut->ucGenStatus != HALDRVSTATE_ENDED ));

                printf("~~~~~~~~~~~~~~~OSMIFARE_Request WUPA!, ucCardLen=%d cardid=\n", ucCardLen);
                for (i=0; i<ucCardLen; i++)
                    printf("%02x ", aucCardId[i]);
                printf("\n");

            }

            if ( !ucResult && ucCardLen)
            {
                ucResult = OSMIFARE_Select( aucCardId,ucCardLen,&ucCardClass );
                printf("~~~~~~~~~~~~~~~ OSMIFARE_Select ret=%d!\n",ucResult);
            }

            if ( !ucResult )
            {
                ucResult = OSMIFARE_TestWRBlock(select);
            }
        }
    }

    return ucResult;
}
#endif

//PCD test
int mifare_test(char argc, char **argv)
{
    unsigned char ret;
    int r_time=0, x_time=0;
    char select=0;

    if (argc==2 && strcmp(argv[1], "-h")==0)
    {
        printf("sand_icc 0, read UID\n");
        printf("sand_icc 1, read blockID 28,29\n");
        printf("		blockID 28:1122..88\n");
        printf("		blockID 29:8877..11\n");
        printf("sand_icc 2, write blockID 28,29\n");
        printf("sand_icc 3, read/write blockID 28,29\n");
        return 0;
    }

    if (argc==1)
        select=2;	//read
    else
        select = atoi(argv[1]);

    OSMIFARE_Open();
    ret = TESTPAD_TestMFCWRCard(select);

    if (ret) x_time++;
    else r_time++;
    printf("x_time=%d, r_time=%d\n", x_time, r_time);
    OSMIFARE_Close();

    return 0;
}
//end
