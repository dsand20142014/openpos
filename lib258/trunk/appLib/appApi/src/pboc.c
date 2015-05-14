/*
appsrc/pboc.c
*/

#include "osicc.h"
#include "smart.h"
#include "libicc.h"
#include <string.h>
#include <stdio.h>


#include <haldrv.h>
#include <osicc.h>
#include <osdriver.h>
//#include "menu.h"

ISO7816IN iso7816in;
ISO7816OUT iso7816out;
ICC_ORDER icc_order;
ICC_ANSWER *picc_answer;

union ORDER_IN_TYPE order_in_type;
union AS_ORDER_TYPE *pas_order_type;
SYNC_ORDER_TYPE *psync_order_type;

unsigned char Os__ICC_detect(unsigned char ucReader);



unsigned char PBOC_reset(unsigned char  ucReader, unsigned char  ucUsekey);
void PBOC_setiso7816out(void);
unsigned char PBOC_warmreset(unsigned char ucReader,unsigned char * pucReset,unsigned short * pusLength);
unsigned char PBOC_reset(unsigned char  ucReader, unsigned char  ucUsekey);
unsigned char PBOC_isocheckreturn(unsigned char ucReader);
unsigned char PBOC_isogetresponse(unsigned char ucReader, unsigned char ucLen);
unsigned char PBOC_selectfile(unsigned char ucFlag, unsigned char ucReader, unsigned char * pucData, unsigned char ucDatalen);
unsigned char PBOC_isoreadrecordbysfi(unsigned char ucReader,unsigned char ucSfi,unsigned char ucRecordnumber);
//extern unsigned char SMART_Reset1(unsigned char ucReader,unsigned char *pucReset,unsigned char *pucLen,unsigned char ucRate);
#define Uart_Printf Uart_Printf

unsigned char PBOC_card(void)
{
        unsigned char ucResult;
        unsigned int uiLoop;

        Uart_Printf("Please insert pboc card\n");
        ucResult = PBOC_reset(0, 1);

        if (!ucResult) {
                Uart_Printf("Reading ...\n");
                ucResult = PBOC_selectfile(1, 0, (unsigned char *)"TID01", 5);

                for (uiLoop = 1; !ucResult; uiLoop++) {
                        ucResult = PBOC_isoreadrecordbysfi(0, 0x01, uiLoop);
                }

                Uart_Printf("card id: %s\n", iso7816out.aucData);
        }

        return ucResult;
}

void PBOC_setiso7816out(void)
{
        iso7816out.uiLen = sizeof(iso7816out.aucData);
        memset(iso7816out.aucData,0,sizeof(iso7816out.aucData));
}

unsigned char PBOC_warmreset(unsigned char ucReader,unsigned char * pucReset,unsigned short * pusLength)
{
        unsigned char ucResult;

        icc_order.ptout = pucReset;
        icc_order.order = WARM_RESET;

        picc_answer = Os__ICC_command(ucReader, &icc_order);

#if 1
        int i;
    Uart_Printf("************%s**********\n", __func__);
    Uart_Printf("picc_answer answer len=%x, drv_status=%x, card_status=%x\n", picc_answer->Len, picc_answer->drv_status, picc_answer->card_status);
    Uart_Printf("icc_order pout=\t");
    for (i=0; i<=picc_answer->Len; i++)
        Uart_Printf("%x ", icc_order.ptout[i]);
    Uart_Printf("\n");
#endif


        if ( picc_answer->drv_status != OK ) {
                Uart_Printf("%d,Os__ICC_command ANSWER ERROR\n",__LINE__);
                return 1;
        }

        switch (picc_answer->card_status) {
        case ICC_ASY:
                ucResult = SMART_OK;
                break;
        default:
                ucResult = picc_answer->card_status;
                break;
        }
        if (picc_answer->Len > *pusLength) {
                ucResult = SMART_OWERFLOW;
        }
        *pusLength = picc_answer->Len;

        return ucResult;
}

unsigned char PBOC_reset(unsigned char  ucReader, unsigned char  ucUsekey)
{
        unsigned char aucBuf[100];
        unsigned char aucTmp[50];
        unsigned short uiLen, i;
        unsigned char ucResult;


        memset(aucBuf, 0, sizeof(aucBuf));
        ucResult = 0;
        do {
                ucResult=Os__ICC_detect(ucReader);
                Uart_Printf("%s %d\n", __func__, ucResult);
        } while (0);

        icc_order.ptout = aucBuf;
        icc_order.order = POWER_ON;
        picc_answer = Os__ICC_command(ucReader, &icc_order);

#if 0

        Uart_Printf("\n\n************%s**********\n", __func__);
        Uart_Printf("answer len=%x, drv_status=%x, card_status=%x\n", picc_answer->Len, picc_answer->drv_status, picc_answer->card_status);
        Uart_Printf("icc_order pout=\t");
        for (i=0; i<picc_answer->Len; i++)
                Uart_Printf("%x ", icc_order.ptout[i]);
        Uart_Printf("\n");
#endif

        if (picc_answer->drv_status != OK ) {
                Uart_Printf("%d, Os__ICC_command ANSWER ERROR!!!\n",__LINE__);
                return 1;
        }

        if (!ucResult) {
                switch (picc_answer->card_status) {
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
        Uart_Printf("%s picc_answer->card_status=%x ucResult=%x\n", __func__, picc_answer->card_status, ucResult);
        return ucResult;
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


        /* Order to execute */
        icc_order.ptout = pucOut;
        icc_order.pt_order_in = &order_in_type;
        if ( ucCardtype == SMART_ASYNC ) {
                icc_order.order = ASYNC_ORDER;
                icc_order.pt_order_in->async_order.order_type = ucOrderType;
                pas_order_type = &icc_order.pt_order_in->async_order.as_order;
                icc_order.pt_order_in->async_order.NAD = 0x00;
                switch ( ucOrderType ) {
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
			//	printf("type 4444\n\n");
					
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
        } else {
                icc_order.order = SYNC_ORDER;
                psync_order_type = &icc_order.pt_order_in->sync_order;

                switch ( *pucIn ) {                             /* Address            */
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

                switch ( *pucIn ) {                             /* Data lenght        */
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

                switch ( ucOrderType ) {
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

#if 0
        int i;
    Uart_Printf("************%s**********\n", __func__);
    Uart_Printf("picc_answer answer len=%x, drv_status=%x, card_status=%x\n", picc_answer->Len, picc_answer->drv_status, picc_answer->card_status);
    Uart_Printf("icc_order pout=\t");
    for (i=0; i<=picc_answer->Len; i++)
        Uart_Printf("%x ", icc_order.ptout[i]);
    Uart_Printf("\n");
#endif

		
	//Uart_Printf("********************[drv_status %02x ][card_status %02x]\n", picc_answer->drv_status,picc_answer->card_status);

        if ( picc_answer->drv_status != OK ) {
                if ((picc_answer->drv_status == 0x1C)||(picc_answer->drv_status == 0x2f)) {
                        Uart_Printf("%d Os__ICC_command ANSWER ERROR\n",__LINE__);
                        return 1;
                } else {
                        Uart_Printf("Os__ICC_command DRIVER ERROR\n");
                        return 1;
                }
        }

		

        if ( picc_answer->card_status != ASY_OK ) {
                Uart_Printf("Os__ICC_command CARD ERROR\n");
                return 1;
        }

        if ( picc_answer->Len > *pusLengthOut ) {
                Uart_Printf("Os__ICC_command LENGTH ERROR\n");
                return 1;
        }


        *pusLengthOut = picc_answer->Len;

        return (SMART_OK);
}

unsigned char PBOC_isocheckreturn(unsigned char ucReader)
{
        unsigned char ucResult;

        iso7816out.ucSW1 = iso7816out.aucData[iso7816out.uiLen - 2];
        iso7816out.ucSW2 = iso7816out.aucData[iso7816out.uiLen - 1];

        switch (iso7816out.ucSW1) {
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
                                          TYPE2)) != OK) {
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
                                  TYPE2)) != OK) {
                return(ucResult);
        }
        return(PBOC_isocheckreturn(ucReader));
}


unsigned char PBOC_selectfile(unsigned char ucFlag,
                              unsigned char ucReader,
                              unsigned char *pucData,
                              unsigned char ucDatalen)
{
        unsigned char	ucResult;

        iso7816in.ucCla = 0x00;
        iso7816in.ucIns = 0xA4;
        if (ucFlag) {
                iso7816in.ucP1  = 0x04;
        } else {
                iso7816in.ucP1  = 0x00;
        }
        iso7816in.ucP2  = 00;
        iso7816in.ucP3 = ucDatalen;
        memcpy(&iso7816in.aucData[0],pucData,ucDatalen);
        iso7816in.aucData[iso7816in.ucP3] = 0xfe;

        PBOC_setiso7816out();

	{
    int i=0;
	char * p=(unsigned char *)&iso7816in;
    printf("\n\n[iso7816in]\n");
    for(i=0;i<ucDatalen+5;i++)
    printf("[%02x] ",*(p+i));
    printf("\n\n[end]\n");
 } 
		
        if ((ucResult=PBOC_iso(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&iso7816in,
                               iso7816out.aucData,
                               &iso7816out.uiLen,
                               TYPE4)) != OK) {
                return(ucResult);
        }

        return(PBOC_isocheckreturn(ucReader));
}

unsigned char PBOC_isoreadrecordbysfi(unsigned char ucReader,
                                      unsigned char ucSfi,
                                      unsigned char ucRecordnumber)
{
        unsigned char	ucResult;

        iso7816in.ucCla = 0x00;
        iso7816in.ucIns = 0xB2;
        iso7816in.ucP1  = ucRecordnumber;
        iso7816in.ucP2  = (ucSfi << 3)|0x04;
        iso7816in.ucP3  = 0x00;

        PBOC_setiso7816out();
        if ( (ucResult = PBOC_iso(ucReader,
                                  SMART_ASYNC,
                                  (unsigned char *)&iso7816in,
                                  iso7816out.aucData,
                                  &iso7816out.uiLen,
                                  TYPE2)) != OK) {
                return(ucResult);
        }

        return(PBOC_isocheckreturn(ucReader));
}


extern DRVOUT	myOSICCDrv;//����ֱ�ӷ���
extern void OSICC_Init(void);
extern unsigned char Os__ICC_detect(unsigned char ucReader);
extern void OSDRV_Init(void);

extern unsigned char OSICC_Detect(unsigned char ucReader);

/*
unsigned char LOADKEY_WaitReadCard_ZY(void)
{
        DRV_OUT *pxIcc;
        DRV_OUT *pxKey;
        unsigned char icc_ATR[100], i;
        unsigned char ucLen,ucResult;

        pxIcc = (DRV_OUT *)Os__ICC_insert();
        Uart_Printf("before wait status\n");
        while ( pxIcc->gen_status!=DRV_ENDED);
        Uart_Printf("I got gen_status=%x\n", pxIcc->gen_status);
        if ( pxIcc->gen_status == DRV_ENDED ) {
                ucLen = pxIcc->xxdata[0];
                memcpy(icc_ATR,pxIcc,ucLen+4);


                for (i=0; i<ucLen+4; i++) {
                        Uart_Printf("%02x ", icc_ATR[i]);
                }
                Uart_Printf("\n");
        }


        return(ucResult);
}
*/
/*
int linuxpos_test()
{
        unsigned char icc_ATR[100], i;
        DRVOUT *pIccDrvOut, *pMagDrvOut, *pKeyDrvOut;
        unsigned char ucLen,ucResult;


        pIccDrvOut = (DRVOUT*)Os__ICC_insert();
        Uart_Printf("\nbefore wait status, pIccDrvOut->ucGenStatus=%x\n", pIccDrvOut->ucGenStatus);
        while (pIccDrvOut->ucGenStatus != DRV_ENDED );
        Uart_Printf("\nI got gen_status=%x\n", pIccDrvOut->ucGenStatus);

        if ( pIccDrvOut->ucGenStatus == DRV_ENDED ) {
                ucLen = pIccDrvOut->aucData[0];
                memcpy(icc_ATR,pIccDrvOut->aucData,ucLen+1);

                printf("\nAPP data0**********************\n");
                for (i=0; i<ucLen+1; i++) {
                        Uart_Printf("%02x ", icc_ATR[i]);
                }
                Uart_Printf("\n");
                printf("\nAPP data1**********************\n");
        }

}
*/
unsigned char PBOC_poweroff_test(unsigned char  ucReader)
{
        unsigned char aucBuf[100];
        unsigned char aucTmp[50];
        unsigned short uiLen, i;
        unsigned char ucResult;


        memset(aucBuf, 0, sizeof(aucBuf));
        ucResult = 0;
        do {
                ucResult=Os__ICC_detect(ucReader);
                Uart_Printf("%s %d\n", __func__, ucResult);
        } while (0);

        icc_order.ptout = aucBuf;
        icc_order.order = POWER_OFF;
        picc_answer = Os__ICC_command(ucReader, &icc_order);

#if 1
        Uart_Printf("\n\n************%s**********\n", __func__);
        Uart_Printf("answer len=%x, drv_status=%x, card_status=%x\n", picc_answer->Len, picc_answer->drv_status, picc_answer->card_status);
        Uart_Printf("icc_order pout=\t");
        for (i=0; i<picc_answer->Len; i++)
                Uart_Printf("%x ", icc_order.ptout[i]);
        Uart_Printf("\n");
#endif

        Uart_Printf("%s picc_answer->card_status=%x ucResult=%x\n", __func__, picc_answer->card_status, ucResult);
        return ucResult;
}

