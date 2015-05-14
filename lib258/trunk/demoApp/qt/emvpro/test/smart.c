/*
--------------------------------------------------------------------------
	FILE  smart.c								(Copyright INGENICO 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-08-13		Xiaoxi Jiang
    Last modified :	2001-08-13		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for smart general process.
        (WARNING!!! for Terminals MPX M21 M22 M23 M24 M25).

    List of routines in file :

    File history :

*/




#include <test/include.h>
#include <osicc.h>
#include	<EMVIcc.h>
//#include <test/smart.h>
#include "smart.h"


//#define		TEST

ICC_ORDER IccOrder;
union ORDER_IN_TYPE OrderType;
union AS_ORDER_TYPE* pAsType;
SYNC_ORDER_TYPE* pSyncOrder;
ICC_ANSWER* pAnswer;

unsigned char SMART_Detect(unsigned char ucReader)
{
	/* Card Detection */

	if ( Os__ICC_detect(ucReader) != 0 )
    {
    	return (SMART_ABSENT);
    }

	return (SMART_PRESENT);
}

unsigned char SMART_SelectProtocol(
		unsigned char ucReader,
		unsigned char ucProtocol)
{
	/* Protocol Type */
    IccOrder.order = ASYNC_PROTOCOL;
    IccOrder.pt_order_in = &OrderType;
    IccOrder.pt_order_in->async_protocol = 0;
    if ( ucProtocol == PROT_T1 )
    {
    	IccOrder.pt_order_in->async_protocol = 1;
    }

	/* Send Command */
	pAnswer = Os__ICC_command (ucReader, &IccOrder);

	/* Answer Command */
	if ( pAnswer->drv_status != OK )
    {
    	return (SMART_DRIVER_ERROR);
    }
	if ( pAnswer->card_status != ASY_OK )
    {
    	return (SMART_CARD_ERROR);
    }
	return (SMART_OK);
}


unsigned char SMART_Reset(
        unsigned char ucReader,
        unsigned char * pucReset,
        unsigned short * puiLength)
{
    DRV_OUT* pdIcc;
    DRV_OUT* pdKey;

    unsigned char ucI,ucCardStatus;
    unsigned char ucResult;
    //unsigned int uiTimeOut;
    //unsigned char ucCardType;
    unsigned char ucBuf[30];
    printf("~~~~~~~SMART_Reset \n");
    while(1)
    {
        //sleep(sleepTime);
        pdIcc = OSICC_Insert();
        if(pdIcc->gen_status==DRV_ENDED)
        {
            ucCardStatus = pdIcc->xxdata[1];
            *puiLength = pdIcc->xxdata[0]-1;
            printf("ucCardStatus=%02x\n",ucCardStatus);
            for(ucI=0;ucI<pdIcc->xxdata[0]+1;ucI++)
                printf("%02x,",pdIcc->xxdata[ucI]);
            switch(ucCardStatus)
            {
            case ICC_ASY:
                memcpy(pucReset,&pdIcc->xxdata[2],*puiLength);
                return(SMART_OK);//
                /*
            case 0x39:
                *puiLength = 39;
                ucResult = SMART_WarmReset(ucReader,pucReset,puiLength);
                for(ucI=0;ucI<*puiLength;ucI++)
                    printf("%02x,",*(pucReset+ucI));
                printf("\n");
                return(ucResult);
                */
            default:
            {
                printf("123456 Os__ICC_off\n");
                Os__ICC_off();
                continue;
            }
            }
        }
    }


}
unsigned char SMART_ISO(
			unsigned char ucReader,
			unsigned char ucCardType,
			unsigned char* pucIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut,
            unsigned char ucOrderType)
{
	unsigned char ucLength,ucLLI;
	unsigned short uiAdd;
	/* Order to execute */
	IccOrder.ptout = pucOut;
	IccOrder.pt_order_in = &OrderType;
	if ( ucCardType == SMART_ASYNC )
    {
	    IccOrder.order = ASYNC_ORDER;
	    IccOrder.pt_order_in->async_order.order_type = ucOrderType;
	    pAsType = &IccOrder.pt_order_in->async_order.as_order;
	    IccOrder.pt_order_in->async_order.NAD = 0x00;
	    switch ( ucOrderType )
	    {
	    case TYPE0:
	        pAsType->order_type0.Lc = *pucIn;       /* Length of Data transmitted */
	        pAsType->order_type0.ptin = pucIn+1;    /* Data transmitted           */
	        break;
	    case TYPE1:
	        pAsType->order_type1.CLA = *pucIn;
	        pAsType->order_type1.INS = *(pucIn+1);
	        pAsType->order_type1.P1  = *(pucIn+2);
	        pAsType->order_type1.P2  = *(pucIn+3);
	        break;
	    case TYPE2:
	        pAsType->order_type2.CLA = *pucIn;
	        pAsType->order_type2.INS = *(pucIn+1);
	        pAsType->order_type2.P1  = *(pucIn+2);
	        pAsType->order_type2.P2  = *(pucIn+3);
	        pAsType->order_type2.Le  = *(pucIn+4);  /* Length of data received   */
	        break;
	    case TYPE3:
	        pAsType->order_type3.CLA = *pucIn;
	        pAsType->order_type3.INS = *(pucIn+1);
	        pAsType->order_type3.P1  = *(pucIn+2);
	        pAsType->order_type3.P2  = *(pucIn+3);
	        pAsType->order_type3.Lc  = *(pucIn+4);  /* Length of data transmitted*/
	        pAsType->order_type3.ptin = pucIn+5;    /* Data transmitted          */
	        break;
	    case TYPE4:
	        pAsType->order_type4.CLA = *pucIn;
	        pAsType->order_type4.INS = *(pucIn+1);
	        pAsType->order_type4.P1  = *(pucIn+2);
	        pAsType->order_type4.P2  = *(pucIn+3);
	        pAsType->order_type4.Lc  = *(pucIn+4);  /* Length of data transmitted*/
	        pAsType->order_type4.ptin = pucIn+5;    /* Data transmitted          */
	                                                /* Length of data received   */
	        pAsType->order_type4.Le  = *(pucIn+5+ *(pucIn+4));
	        break;
	    default:
	        return (SMART_CARD_ERROR);
	    }

#ifdef TEST
			if(ucOrderType == TYPE1)
				ucLLI = 4;
			if(ucOrderType == TYPE2)
				ucLLI =	5;
			if(ucOrderType == TYPE3)
				ucLLI =	pAsType->order_type3.Lc+4+1;
			if(ucOrderType == TYPE4)
				ucLLI =	pAsType->order_type4.Lc+4+2;
#if 0
			Uart_Printf("\nSend to card: \n");
			for(ucLength=0;ucLength<ucLLI;ucLength++)
				Uart_Printf("%02X ",*(pucIn+ucLength));
			Uart_Printf("\n");
#endif
#endif

	}else
    {
	    IccOrder.order = SYNC_ORDER;
	    pSyncOrder = &IccOrder.pt_order_in->sync_order;

	    switch ( *pucIn )                               /* Address            */
	    {
	    case 0x05:                                      /* SLE4418 ou SLE4428 */
	    case 0x06:                                      /* SLE4432 ou SLE4442 */
	        uiAdd = (*(pucIn+2) << 8) | (*(pucIn+3) & 0x00FF);
	        uiAdd *= 8;                                 /* Convert to bits    */
	        pSyncOrder->ADDH = uiAdd >> 8;             /* P1                 */
	        pSyncOrder->ADDL = uiAdd & 0x00FF;         /* P2                 */
	        break;
	    default:                                        /* Other cards        */
	        pSyncOrder->ADDH = *(pucIn+2);             /* P1                 */
	        pSyncOrder->ADDL = *(pucIn+3);             /* P2                 */
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
	    case TYPE2:
	        pSyncOrder->card_type = *pucIn;     /* Synchronous Type           */
	        pSyncOrder->INS = *(pucIn+1);       /* INS                        */
	        /* P1                         */
	        /* P2                         */
	        pSyncOrder->Len = ucLength;         /* Length of data received    */
	        break;
	    case TYPE3:
	        pSyncOrder->card_type = *pucIn;     /* Synchronous Type           */
	        pSyncOrder->INS = *(pucIn+1);       /* INS                        */
	        /* P1                         */
	        /* P2                         */
	        pSyncOrder->Len = ucLength;         /* Length of data transmitted */
	        pSyncOrder->ptin = pucIn+4;         /* Data transmitted           */
	        break;
	    default:
	        return (SMART_CARD_ERROR);
	    }
	}

	/* Send Command */
	pAnswer = Os__ICC_command (ucReader, &IccOrder);

	/* Answer Command */
	/******************/

	if ( pAnswer->drv_status != OK )
    {
    	return(SMART_DRIVER_ERROR);
    }
	if ( pAnswer->card_status != ASY_OK )
	{
		return(SMART_CARD_ERROR);
	}
	if ( pAnswer->Len > *puiLengthOut )
	{
		return(SMART_OWERFLOW);
	}
	*puiLengthOut = pAnswer->Len;
	return (SMART_OK);
}

unsigned char SMART_PowerOff (unsigned char ucReader)
{
	/* Power Off */
	IccOrder.order = POWER_OFF;

	/* Send Command */
	pAnswer = Os__ICC_command (ucReader, &IccOrder);

	/* Answer Command */
	if ( pAnswer->drv_status != OK )
    {
    	return (SMART_DRIVER_ERROR);
    }
	if ( pAnswer->card_status != ASY_OK )
    {
    	return (SMART_CARD_ERROR);
    }
	return (SMART_OK);
}

unsigned char SMART_Remove(unsigned char ucReader)
{
	unsigned char ucRet;

	/* Remove Card */
	if( (ucRet=SMART_PowerOff(ucReader)) == SMART_OK )
    {
    	while ( SMART_Detect(ucReader) == SMART_PRESENT )
		{ /*NOP*/;
		}
	}
	return (ucRet);
}

unsigned char IccIsoCommand(unsigned char slot, EMVICCIN *pEMVICCIn, EMVICCOUT *pEMVICCOut)
{
	unsigned char ucReader,ucCase,ucResult;
	unsigned short uiLen;
	unsigned char aucInBuf[300];
	unsigned char aucOutBuf[300];

	int i;

	ucReader = slot;
	memset(aucOutBuf,0,sizeof(aucOutBuf));
	memset(aucInBuf,0,sizeof(aucInBuf));
	memcpy(aucInBuf,&pEMVICCIn->ucCla,4);
	if(pEMVICCIn->uiLc)
	{
		aucInBuf[4] = pEMVICCIn->uiLc;
		memcpy(&aucInBuf[5],pEMVICCIn->aucDataIn,pEMVICCIn->uiLc);
		if(pEMVICCIn->uiLe)
			aucInBuf[5+pEMVICCIn->uiLc] = pEMVICCIn->uiLe%256;
	}else 
	{
		aucInBuf[4] = pEMVICCIn->uiLe%256;
	}
	if(pEMVICCIn->uiLc)
	{
		if(pEMVICCIn->uiLe)
			ucCase = TYPE4;
		else 
			ucCase = TYPE3;	
	}else
	{
		if(pEMVICCIn->uiLe)
			ucCase = TYPE2;
		else 
			ucCase = TYPE1;	
	}
	uiLen = sizeof(aucOutBuf);
	ucResult = SMART_ISO(ucReader,SMART_ASYNC,aucInBuf,aucOutBuf,&uiLen,ucCase);
#if 0
	uiSendLen = pEMVICCIn->uiLc+4+2;
	uiLen = sizeof(aucOutBuf);

	ucResult = Os__MIFARE_SendRevFrame( aucInBuf,uiSendLen,aucOutBuf,&uiLen,0 );
#endif

#ifdef TEST
	Uart_Printf("ICC Comm ret: %02X",ucResult);
#endif
	if(ucResult!= SMART_OK)
		return(ucResult);
	memset(pEMVICCOut->aucDataOut,0,300);
	pEMVICCOut->uiLenOut= uiLen-2;
	memcpy(pEMVICCOut->aucDataOut,aucOutBuf,pEMVICCOut->uiLenOut);
	pEMVICCOut->ucSWA = aucOutBuf[uiLen-2];
	pEMVICCOut->ucSWB = aucOutBuf[uiLen-1];
#ifdef TEST
	Uart_Printf("\nCard return Data: \n");
	for(i=0;i<uiLen;i++)
		Uart_Printf("%02X ",aucOutBuf[i]);
	Uart_Printf("\n");
#endif
	return(SMART_OK);
}

unsigned char SMART_WarmReset(
		unsigned char ucReader,
		unsigned char ucCardStandard,
		unsigned char * pucReset,
		unsigned short * puiLength)
{
	unsigned char ucCardType;
	/* Power On */
	

	/* Wait for a Card */
	
	
    IccOrder.pt_order_in = &OrderType;
	IccOrder.ptout = pucReset;
	IccOrder.order = NEW_WARM_RESET;
    IccOrder.pt_order_in->new_power.card_standards = ucCardStandard;
    IccOrder.pt_order_in->new_power.GR_class_byte_00 = 1;
    IccOrder.pt_order_in->new_power.preferred_protocol = 0xFF;

	/* Send Command */
	pAnswer = Os__ICC_command(ucReader, &IccOrder);
	/* Answer Command */
	if ( pAnswer->drv_status != OK )
    {
    	return (SMART_DRIVER_ERROR);
    }
	switch ( pAnswer->card_status )
    {
	case ICC_ASY:
	case ICC_SYN:
	    ucCardType = pAnswer->card_status;
	    break;
	default:
    	return (SMART_CARD_ERROR);
    }
	if ( pAnswer->Len > *puiLength )
    {
    	return (SMART_OWERFLOW);
    }
	*puiLength = pAnswer->Len;
	return (ucCardType);
}
