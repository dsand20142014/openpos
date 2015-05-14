/* 
	INGENICO China
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

#include <include.h>
#include <global.h>
#include <osicc.h>
#include <smart.h>

ICC_ORDER IccOrder;
union ORDER_IN_TYPE OrderType;
union AS_ORDER_TYPE* pAsType;
SYNC_ORDER_TYPE* pSyncOrder;
ICC_ANSWER* pAnswer;

unsigned char SMART_Reset(
		unsigned char ucReader,
		unsigned char * pucReset,
		unsigned short * puiLength)
{
	DRV_OUT* pdKey;
	unsigned char ucCardType;

	/* Wait for a Card */
	//pdKey = Os__get_key();
	while(  SMART_Detect(ucReader) == SMART_ABSENT)
	{
		if(pdKey->gen_status == DRV_ENDED)
		{
			if ( pdKey->xxdata[1] == KEY_CLEAR )
			{
				return (SMART_ABORT);
			}
			pdKey = Os__get_key();
		}
     }

	/* Power On */
	IccOrder.ptout = pucReset;
	IccOrder.order = POWER_ON;

	/* Send Command */
	pAnswer = Os__ICC_command (ucReader, &IccOrder);
	/* Answer Command */
	if ( pAnswer->drv_status != OK )
    { 
    	return (SMART_DRIVER_ERROR); 
    }
	switch ( pAnswer->card_status )
    {
	case ICC_ASY:
	    ucCardType = SMART_ASYNC;
	    break;
	case ICC_SYN:
	    ucCardType = SMART_SYNC;
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

unsigned char SMART_ISO_bak(
			unsigned char ucReader,
			unsigned char ucCardType,
			unsigned char* pucIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut,
            unsigned char ucOrderType)
{
	unsigned char ucLength;
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
	for(ucLength=0;ucLength<35;ucLength++)
	Uart_Printf("%02x",*(pucIn+ucLength));
	Uart_Printf("\n");

	/* Send Command */
	pAnswer = Os__ICC_command (ucReader, &IccOrder);

	/* Answer Command */
	/******************/
	if ( pAnswer->drv_status != OK )
    {
    	return (SMART_DRIVER_ERROR); 
    }
	if ( pAnswer->card_status != ASY_OK )
	{
		return (SMART_CARD_ERROR); 
	}

	if ( pAnswer->Len > *puiLengthOut )
	{
		return (SMART_OWERFLOW);
	}
	*puiLengthOut = pAnswer->Len;
	return (SMART_OK);
}

unsigned char SMART_ISO_DX(
			unsigned char ucReader,
			unsigned char ucCardType,
			unsigned char* pucIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut,
            unsigned char ucOrderType)
{
	unsigned char ucResult = SUCCESS,ucStatus;
	DrvIn.type      = 0xa2;
	switch(ucOrderType)
	{
		case TYPE1: DrvIn.length=5+4;break;
		case TYPE2: DrvIn.length=6+4;break;
		case TYPE3: 
		case TYPE4: DrvIn.length=*(pucIn+4)+6+4;break;
		default: DrvIn.length=135;break;
	}
	DrvIn.xxdata[1]=200;
	DrvIn.xxdata[2]=DrvIn.length-3;
	DrvIn.xxdata[3]=0xa0;
	DrvIn.xxdata[4]=DrvIn.length-5;
	memcpy(&DrvIn.xxdata[5],pucIn,DrvIn.length-5);
	{
		unsigned char  i;
		Uart_Printf("\nThis IsPinPad Send Data [ %d]\n",DrvIn.length);
		for(i = 0; i <50; i++)
		{
			Uart_Printf("%02x ",DrvIn.xxdata[i]);
		}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
		unsigned char  i;
		Uart_Printf("\nThis Is PinPad Receive Data [ DrvOut.xxdata ]\n");
		for(i = 0; i <  65; i++)
		{
			Uart_Printf("%02x ",DrvOut.xxdata[i]);
		}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		*puiLengthOut=DrvOut.xxdata[3];
		memcpy(pucOut,&DrvOut.xxdata[4],*puiLengthOut);
		ucResult=Untouch_Check_Err(ucStatus);
		
	}
	return(ucResult);
}

unsigned char SMART_ISO(
			unsigned char ucReader,
			unsigned char ucCardType,
			unsigned char* pucIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut,
            unsigned char ucOrderType)
{
	unsigned char ucResult = SUCCESS,ucStatus;
	if(RunData.ucDxMobile==1)
	{
		ucResult = SMART_ISO_DX(ucReader,
							ucCardType,
							pucIn,
							pucOut,
							puiLengthOut,
							ucOrderType);  
		return(ucResult);
	}
	if(RunData.ucInputmode==1||ucReader!=ICC1)
	{
		ucResult = SMART_ISO_bak(ucReader,
							ucCardType,
							pucIn,
							pucOut,
							puiLengthOut,
							ucOrderType);  
		return(ucResult);
	}
	{
		unsigned char  i;
		Uart_Printf("\nThis IsPinPad Send Data \n");
		for(i = 0; i <50; i++)
		{
			Uart_Printf("%02x ",pucIn[i]);
		}
		Uart_Printf("\n");
	}
	
	ucResult = MIFARE_ISO(pucIn,
						pucOut,
						puiLengthOut,
						ucOrderType);  
	{
		unsigned char  i;
		Uart_Printf("\nThis Is PinPad Receive Data [ DrvOut.xxdata ]\n");
		for(i = 0; i <  50; i++)
		{
			Uart_Printf("%02x ",pucOut[i]);
		}
		Uart_Printf("\n");
	}
	return(ucResult);

}
unsigned char SMART_Seimma(
			unsigned char* pucIn,
			unsigned short uiLengthIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut)
{
	unsigned char ucResult = SUCCESS,ucStatus,ucLrc;
	int i,iLen;
	ucLrc=0;
	DrvIn.type      = 0xa3;
	DrvIn.length = uiLengthIn+7;
	DrvIn.xxdata[0]=0x02;
	DrvIn.xxdata[1]=0x01;
	DrvIn.xxdata[2]=uiLengthIn/100;
	DrvIn.xxdata[3]=uiLengthIn%100/10;
	DrvIn.xxdata[4]=uiLengthIn%100%10;
	for(i=0;i<uiLengthIn;i++)
	{
		DrvIn.xxdata[5+i]=pucIn[i];
		ucLrc^=pucIn[i];
	}
	DrvIn.xxdata[5+uiLengthIn]=0x03;
	DrvIn.xxdata[6+uiLengthIn]=ucLrc;
	{
		unsigned char  i;
		Uart_Printf("\nThis IsPinPad Send Data [ %d]\n",DrvIn.length);
		for(i = 0; i <50; i++)
		{
			Uart_Printf("%02x ",DrvIn.xxdata[i]);
		}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_WaitCard();
	{
		unsigned char  i;
		Uart_Printf("\nThis Is PinPad Receive Data [ DrvOut.xxdata ]\n");
		for(i = 0; i <  80; i++)
		{
			Uart_Printf("%02x ",DrvOut.xxdata[i]);
		}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		if(DrvOut.xxdata[1]!=0x02)
			return 0x04;
		iLen=DrvOut.xxdata[3]*100+DrvOut.xxdata[4]*10+DrvOut.xxdata[5];
		Uart_Printf("iLen=%d\n",iLen);
		memcpy(pucOut,&DrvOut.xxdata[1],iLen+7);
		ucLrc=0;
		for(i=0;i<iLen;i++)
		{
			ucLrc^=DrvOut.xxdata[6+i];
		}
		Uart_Printf("DrvOut.xxdata[6+iLen]=%02x\n",DrvOut.xxdata[6+iLen]);
		Uart_Printf("DrvOut.xxdata[7+iLen]=%02x\n",DrvOut.xxdata[7+iLen]);
		Uart_Printf("ucLrc=%d\n",ucLrc);
		if(DrvOut.xxdata[6+iLen]!=0x03)
			return 0x04;
		if(DrvOut.xxdata[7+iLen]!=ucLrc)
			return 0x04;
		ucResult=Untouch_Check_Err(ucStatus);
		
	}
	return(ucResult);
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
unsigned char SMART_Elitel(
			unsigned char* pucIn,
			unsigned short uiLengthIn,
            unsigned char* pucOut,
            unsigned short* puiLengthOut)
{
	unsigned char ucResult = SUCCESS,ucStatus;
	DrvIn.type      = 0xa2;
	DrvIn.length = uiLengthIn+3;
	DrvIn.xxdata[1]=200;
	DrvIn.xxdata[2]=DrvIn.length-3;
	memcpy(&DrvIn.xxdata[3],pucIn,uiLengthIn);
	{
		unsigned char  i;
		Uart_Printf("\nThis IsPinPad Send Data [ %d]\n",DrvIn.length);
		for(i = 0; i <50; i++)
		{
			Uart_Printf("%02x ",DrvIn.xxdata[i]);
		}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_WaitCard();
	{
		unsigned char  i;
		Uart_Printf("\nThis Is PinPad Receive Data [ DrvOut.xxdata ]\n");
		for(i = 0; i <  80; i++)
		{
			Uart_Printf("%02x ",DrvOut.xxdata[i]);
		}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		*puiLengthOut=DrvOut.xxdata[1];
		memcpy(pucOut,&DrvOut.xxdata[2],*puiLengthOut);
		ucResult=Untouch_Check_Err(ucStatus);
		
	}
	return(ucResult);
}

	
