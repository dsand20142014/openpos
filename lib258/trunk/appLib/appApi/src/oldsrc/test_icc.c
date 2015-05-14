#include <fcntl.h>
#include "sand_icc.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

typedef struct
{
    unsigned char ucCla;
    unsigned char ucIns;
    unsigned char ucP1;
    unsigned char ucP2;
    unsigned char ucP3;
    unsigned char aucData[128];
}ISO7816IN;

typedef struct
{
    unsigned short	uiLen;
    unsigned char	ucStatus;
    unsigned char	ucSW1;
    unsigned char	ucSW2;
    unsigned char	aucData[255];
}ISO7816OUT;

ISO7816IN	iso7816in;
ISO7816OUT	iso7816out;

ICC_ORDER icc_order;
ICC_ANSWER *picc_answer;

/* Smart Card */
#define SMART_OK				0		/* Return Code Ok  0x00 */
#define SMART_ASYNC				1       /* Synchronous Ok  0x01 */
#define SMART_SYNC				2       /* Asynchronous Ok 0x02 */
#define SMART_DRIVER_ERROR		3       /* Driver Error    0x03 */
#define SMART_CARD_ERROR		4       /* Card Error      0x04 */
#define SMART_OWERFLOW			5       /* Owerflow Buffer 0x05 */
#define SMART_ABORT				6       /* Abort Key       0x06 */
#define SMART_ABSENT			7		/* Smart Absent    0x07 */
#define SMART_PRESENT           8		/* Smart Present   0x08 */


/* Smart Async Order Type */
#define TYPE0					0		/* Order Type0 LgIn,BufIn                     */
#define TYPE1					1		/* Order Type1 CLA,INS,P1,P2                  */
#define TYPE2					2		/* Order Type2 CLA,INS,P1,P2,LgOut            */
#define TYPE3					3		/* Order Type3 CLA,INS,P1,P2,LgIn,BufIn       */
#define TYPE4					4		/* Order Type4 CLA,INS,P1,P2,LgIn,BufIn,LgOut */

unsigned char PBOC_iso(unsigned char ucReader,
                       unsigned char ucCardtype,
                       unsigned char* pucIn,
                       unsigned char* pucOut,
                       unsigned short* pusLengthOut,
                       unsigned char ucOrderType);
unsigned char PBOC_isocheckreturn(unsigned char ucReader);

union ORDER_IN_TYPE order_in_type;
union AS_ORDER_TYPE *pas_order_type;
SYNC_ORDER_TYPE *psync_order_type;

void PBOC_setiso7816out(void)
{
    iso7816out.uiLen = sizeof(iso7816out.aucData);
    memset(iso7816out.aucData,0,sizeof(iso7816out.aucData));
}


unsigned char PBOC_isogetresponse(unsigned char ucReader,
                                  unsigned char ucLen)
{
    unsigned char ucResult;

    iso7816in.ucCla = 0x00;
    iso7816in.ucIns = 0xC0;
    iso7816in.ucP1  = 0x00;
    iso7816in.ucP2  = 0x00;
    iso7816in.ucP3  = ucLen;

    PBOC_setiso7816out();
    if ( (ucResult = PBOC_iso(ucReader,
                              SMART_ASYNC,
                              (unsigned char *)&iso7816in,
                              iso7816out.aucData,
                              &iso7816out.uiLen,
                              TYPE2)) != OK)
    {
        return(ucResult);
    }
    return(PBOC_isocheckreturn(ucReader));
}


unsigned char PBOC_isocheckreturn(unsigned char ucReader)
{
    unsigned char ucResult;

    iso7816out.ucSW1 = iso7816out.aucData[iso7816out.uiLen - 2];
    iso7816out.ucSW2 = iso7816out.aucData[iso7816out.uiLen - 1];

    switch (iso7816out.ucSW1)
    {
    case 0x90:
        return 0;
    case 0x6C:
        PBOC_setiso7816out();
        iso7816in.ucP3  = iso7816out.ucSW2;
        if ( (ucResult = PBOC_iso(ucReader,
                                  SMART_ASYNC,
                                  (unsigned char *)&iso7816in,
                                  iso7816out.aucData,
                                  &iso7816out.uiLen,
                                  TYPE2)) != OK)
        {
            return(ucResult);
        }
        return(PBOC_isocheckreturn(ucReader));
    case 0x61:
    case 0x9F:
        return(PBOC_isogetresponse(ucReader,iso7816out.ucSW2));
    default:
        break;
    }
}

unsigned char PBOC_reset(unsigned char  ucReader, unsigned char  ucUsekey)
{
    unsigned char aucBuf[100];
    unsigned char aucTmp[50];
    unsigned short uiLen, i;
    unsigned char ucResult;


    memset(aucBuf, 0, sizeof(aucBuf));
    ucResult = 0;

    while (Os__ICC_detect(ucReader));
    Uart_Printf("****** I got card****** \n\n");

    icc_order.ptout = aucBuf;
    icc_order.order = POWER_ON;

    Uart_Printf("****** todo Os__ICC_command ****** \n\n");
    picc_answer = Os__ICC_command(ucReader, &icc_order);
#if 1
    Uart_Printf("************%s**********\n", __func__);
    Uart_Printf("picc_answer answer len=%x, drv_status=%x, card_status=%x\n", picc_answer->Len, picc_answer->drv_status, picc_answer->card_status);
    Uart_Printf("icc_order pout=\t");
    for (i=0; i<=picc_answer->Len; i++)
        Uart_Printf("%x ", icc_order.ptout[i]);
    Uart_Printf("\n");
#endif

#if 0
    if (picc_answer->drv_status != OK )
    {
        Uart_Printf("Os__ICC_command ANSWER ERROR\n");
        return 1;
    }

    if (!ucResult)
    {
        switch (picc_answer->card_status)
        {
        case ICC_ASY:
            ucResult = 0;
            break;
        case 0x39:
            memset(aucTmp, 0, sizeof(aucTmp));
            uiLen = sizeof(aucTmp);
            ucResult = PBOC_warmreset(ucReader,aucTmp,&uiLen);
            break;
        default:
            ucResult = picc_answer->card_status;
            break;
        }
    }
#endif

    return ucResult;
}


unsigned char PBOC_selectfile(unsigned char ucFlag,
                              unsigned char ucReader,
                              unsigned char *pucData,
                              unsigned char ucDatalen)
{
    unsigned char	ucResult;

    iso7816in.ucCla = 0x00;
    iso7816in.ucIns = 0xA4;
    if (ucFlag)
    {
        iso7816in.ucP1  = 0x04;
    }
    else
    {
        iso7816in.ucP1  = 0x00;
    }
    iso7816in.ucP2  = 00;
    iso7816in.ucP3 = ucDatalen;
    memcpy(&iso7816in.aucData[0],pucData,ucDatalen);
    iso7816in.aucData[iso7816in.ucP3] = 0x00;

    PBOC_setiso7816out();
    if ((ucResult=PBOC_iso(ucReader,
                           SMART_ASYNC,
                           (unsigned char *)&iso7816in,
                           iso7816out.aucData,
                           &iso7816out.uiLen,
                           TYPE4)) != OK)
    {
        return(ucResult);
    }

    return(PBOC_isocheckreturn(ucReader));
}


/*
功能描述：根据短文件标志符读取文件
入口参数：ucReader为读卡位置，ucSFI为短文件标志符，ucOffset为偏移量，ucInLen为读取的数据长度
返回值：函数执行状态
*/
unsigned char PBOC_ISOReadBinarybySFI(unsigned char ucReader,
                                      unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen)
{
    unsigned char	ucResult;
    unsigned char ucI;

    iso7816in.ucCla = 0x00;
    iso7816in.ucIns = 0xB0;
    iso7816in.ucP1  = ucSFI; //zy
    //iso7816in.ucP1  = ucSFI | 0x80;
    iso7816in.ucP2  = ucOffset;
    iso7816in.ucP3  = ucInLen;

    PBOC_setiso7816out();
    if ( (ucResult = PBOC_iso(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&iso7816in,
                               iso7816out.aucData,
                               &iso7816out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);


    return(PBOC_isocheckreturn(ucReader));
}

/*
功能描述：根据短文件标志符读取文件记录
入口参数：ucReader为读卡位置，ucSFI为短文件标志符，ucRecordNumber为记录号
返回值：函数执行状态
*/
unsigned char PBOC_ISOReadRecordbySFI(unsigned char ucReader,
                                      unsigned char ucSFI,unsigned char ucRecordNumber)
{
    unsigned char	ucResult;


    iso7816in.ucCla = 0x00;
    iso7816in.ucIns = 0xB2;
    iso7816in.ucP1  = ucRecordNumber;
    iso7816in.ucP2  = (ucSFI << 3) | 0x04;
    iso7816in.ucP3  = 0x00;

    PBOC_setiso7816out();
    if ( (ucResult = PBOC_iso(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&iso7816in,
                               iso7816out.aucData,
                               &iso7816out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);



    return(PBOC_isocheckreturn(ucReader));
}


unsigned char PBOC_iso(unsigned char ucReader,
                       unsigned char ucCardtype,
                       unsigned char* pucIn,
                       unsigned char* pucOut,
                       unsigned short* pusLengthOut,
                       unsigned char ucOrderType)
{
    unsigned char ucLength;
    unsigned short uiAdd;
	int i;


    /* Order to execute */
    icc_order.ptout = pucOut;
    icc_order.pt_order_in = &order_in_type;
    if ( ucCardtype == SMART_ASYNC )
    {
        icc_order.order = ASYNC_ORDER;
        icc_order.pt_order_in->async_order.order_type = ucOrderType;
        pas_order_type = &icc_order.pt_order_in->async_order.as_order;
        icc_order.pt_order_in->async_order.NAD = 0x00;
        switch ( ucOrderType )
        {
        case TYPE0:
            pas_order_type->order_type0.Lc = *pucIn;       /* Length of Data transmitted */
            pas_order_type->order_type0.ptin = pucIn+1;    /* Data transmitted           */
            break;
        case TYPE1:
            pas_order_type->order_type1.CLA = *pucIn;
            pas_order_type->order_type1.INS = *(pucIn+1);
            pas_order_type->order_type1.P1  = *(pucIn+2);
            pas_order_type->order_type1.P2  = *(pucIn+3);
            break;
        case TYPE2:
            pas_order_type->order_type2.CLA = *pucIn;
            pas_order_type->order_type2.INS = *(pucIn+1);
            pas_order_type->order_type2.P1  = *(pucIn+2);
            pas_order_type->order_type2.P2  = *(pucIn+3);
            pas_order_type->order_type2.Le  = *(pucIn+4);  /* Length of data received   */
            break;
        case TYPE3:
            pas_order_type->order_type3.CLA = *pucIn;
            pas_order_type->order_type3.INS = *(pucIn+1);
            pas_order_type->order_type3.P1  = *(pucIn+2);
            pas_order_type->order_type3.P2  = *(pucIn+3);
            pas_order_type->order_type3.Lc  = *(pucIn+4);  /* Length of data transmitted*/
            pas_order_type->order_type3.ptin = pucIn+5;    /* Data transmitted          */
            break;
        case TYPE4:
            pas_order_type->order_type4.CLA = *pucIn;
            pas_order_type->order_type4.INS = *(pucIn+1);
            pas_order_type->order_type4.P1  = *(pucIn+2);
            pas_order_type->order_type4.P2  = *(pucIn+3);
            pas_order_type->order_type4.Lc  = *(pucIn+4);  /* Length of data transmitted*/
            pas_order_type->order_type4.ptin = pucIn+5;    /* Data transmitted          */
            /* Length of data received   */
            pas_order_type->order_type4.Le  = *(pucIn+5+ *(pucIn+4));
            break;
        default:
            return (SMART_CARD_ERROR);
        }
    }
    else
    {
        icc_order.order = SYNC_ORDER;
        psync_order_type = &icc_order.pt_order_in->sync_order;

        switch ( *pucIn )                               /* Address            */
        {
        case 0x05:                                      /* SLE4418 ou SLE4428 */
        case 0x06:                                      /* SLE4432 ou SLE4442 */
            uiAdd = (*(pucIn+2) << 8) | (*(pucIn+3) & 0x00FF);
            uiAdd *= 8;                                 /* Convert to bits    */
            psync_order_type->ADDH = uiAdd >> 8;             /* P1                 */
            psync_order_type->ADDL = uiAdd & 0x00FF;         /* P2                 */
            break;
        default:                                        /* Other cards        */
            psync_order_type->ADDH = *(pucIn+2);             /* P1                 */
            psync_order_type->ADDL = *(pucIn+3);             /* P2                 */
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
            psync_order_type->card_type = *pucIn;     /* Synchronous Type           */
            psync_order_type->INS = *(pucIn+1);       /* INS                        */
            /* P1                         */
            /* P2                         */
            psync_order_type->Len = ucLength;         /* Length of data received    */
            break;
        case TYPE3:
            psync_order_type->card_type = *pucIn;     /* Synchronous Type           */
            psync_order_type->INS = *(pucIn+1);       /* INS                        */
            /* P1                         */
            /* P2                         */
            psync_order_type->Len = ucLength;         /* Length of data transmitted */
            psync_order_type->ptin = pucIn+4;         /* Data transmitted           */
            break;
        default:
            return (SMART_CARD_ERROR);
        }
    }

    picc_answer=Os__ICC_command(ucReader,&icc_order);

#ifdef DBG
	Uart_Printf("______________%s(%x, %x):_______________\n", __func__, icc_order.ptout[0], picc_answer->Len);
	for (i=0; i<=icc_order.ptout[0];i++)
		Uart_Printf("%02x ", icc_order.ptout[i]);
	Uart_Printf("\n");
	Uart_Printf("______________%s THE END_______________\n", __func__);
#endif
    if ( picc_answer->drv_status != OK )
    {
        if ((picc_answer->drv_status == 0x1C)||(picc_answer->drv_status == 0x2f))
        {
            Uart_Printf("Os__ICC_command ANSWER ERROR\n");
            return 1;
        }
        else
        {
            Uart_Printf("Os__ICC_command DRIVER ERROR\n");
            return 1;
        }
    }

    if ( picc_answer->card_status != ASY_OK )
    {
        Uart_Printf("Os__ICC_command CARD ERROR\n");
        return 1;
    }

    if ( picc_answer->Len > *pusLengthOut )
    {
        Uart_Printf("Os__ICC_command LENGTH ERROR\n");
        return 1;
    }


    *pusLengthOut = picc_answer->Len;

    return (SMART_OK);
}


int main(char argc, char *argv[])
{
	unsigned char readerID=0x0, ret=0;
	DRV_OUT *pxIcc;
	
	Uart_Printf("ICC Test demo\n");

	PBOC_reset(0, 0);
	sleep(2);
	Uart_Printf("**********PBOC_reset OK *******************\n\n\n");

#if 1
  //TID01 用户卡
  Uart_Printf("\n\n\nto select TID01\n");        
  ret=PBOC_selectfile(1, readerID, "TID01", 5);
  Uart_Printf("SelectFile ret=%d\n", ret);
	sleep(1);
#endif

  Uart_Printf("\n\n\nto select EF02\n");
  ret=PBOC_selectfile(0, readerID, "\xEF\x02", 2);
  Uart_Printf("SelectFile ret=%d\n", ret);
	sleep(1);

  Uart_Printf("\n\n\nto read EF02\n");
  ret = PBOC_ISOReadBinarybySFI(readerID, 0x00, 0x00, 0xdd);
  Uart_Printf("PBOC_ISOReadBinarybySFI ret=%d\n", ret);
	sleep(1);

#if 0
	do{
		ret=Os__ICC_detect(ICC1);
		if (ret){
			Uart_Printf("Got0 card: %d\n", ret);
			break;
		}
	}while(1);
	
	Os__ICC_off();
	do{
		ret=Os__ICC_detect(ICC1);
		if (ret){
			Uart_Printf("Got1 card: %d\n", ret);
			break;
		}
	}while(1);
#endif
/*
	pxIcc = Os__ICC_insert();
	
	//Os__ICC_off();
	Os__ICC_remove();
*/
//	Os__close();
out:
	return 0;
}

