/*
    Operating System
--------------------------------------------------------------------------
    FILE  osicc.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :

Ӧ�ò�/���ӿ�ʵ��

                [POS APP]
------  sand_icc.c --------------
                osicc.c

                OSDRV.c
      -------------
        HALDRV.c
           |
            _____|_________________
            |											|
s3c2410_tda8007.c    s3c2410_mag.c
    Others
    .......

*/
#include <stdio.h>
#include <stdlib.h>
//#include <sys/fcntl.h>
#include <osicc.h>
#include <osdriver.h>
#include <stdio.h>
#include <haldrv.h>
#include <string.h>
#include <unistd.h>
#include <sand_debug.h>
#include <oslib.h>

#include <stdio.h>
#include <stdlib.h>
#include <asm/fcntl.h>
#include <osicc.h>
#include <osdriver.h>
#include <stdio.h>
#include <haldrv.h>
#include <string.h>
#include <sys/mman.h>

#define TST_PRES    0x00
#define SELECT      0x01
#define WRPROT      0x02
#define ATTPRE      0x04
#define RECART      0x05
#define PREREC      0x07
#define RECIDE      0x08
#define ICC_STD     0x09
#define MHTCAM      0x0a
#define ATTRET      0x0b
#define DECRET      0x0c
#define EXEISO      0x0d
#define EXESYN      0x0e
#define SYNAPP      0x10
#define WARMRST     0x11
#define PROTOCOL    0x12
#define EMVMODUL    0x13
#define INSERTCARD  0x14


#define ICC_DEVICE "/dev/TDA8007"

/*
EN: Internal Function
*/
void OSICC_SetReport(unsigned char val);
unsigned char OSICC_ChangeReader(unsigned char ucReader);
unsigned char OSICC_Activate(unsigned char ucType);
unsigned char OSICC_PowerOn(unsigned char ucType);
unsigned char OSICC_NewPowerOn(unsigned char ucType);
void OSICC_AsyncOrderPrepare(void);
unsigned char OSICC_AsyncOrder(void);
unsigned char OSICC_AsyncProtocol(unsigned char protocol);
unsigned char OSICC_ChangeProtocol(unsigned char protocol);
unsigned char OSICC_SetICCStandard(unsigned char new_standard);
unsigned char OSICC_ExecSyncOrder(void);
unsigned char OSICC_SyncOrder(void);
unsigned char OSICC_NewSyncOrder(void);


/*
EN: Internal variable
*/
static unsigned char ucICCCurrReader = ICC1;
static unsigned char ucDataOffset;
static ICC_ANSWER  ICCReport;
static ICC_ORDER   *pICCOrder;

extern DRV_OUT *ic_drvout_data;
DRVIN	OSICCDrvIn;
DRVOUT	OSICCDrvOut;
DRVOUT	OSICCDrvOut2;
DRV_MMAP *myOSICCDrv;//����ֱ�ӷ���
int fd_icc = -1;

/*******************************************************************************
 * ͨ��IC�����ʵ��
 *
 ********************************************************************************/
void OSICC_Init(void)
{
    /*
        if (ucICCCurrReader ==-1)
            ucICCCurrReader = ICC1;
    */
    munmap(ic_drvout_data, 1024);

    if (fd_icc)
        close(fd_icc);

    if (fd_icc<0)
    {    
//        printf("*****%s %d********\n",  __func__,__LINE__);
        
        fd_icc = open(ICC_DEVICE, O_RDWR);
        if (fd_icc<0)
        {
//            Uart_Printf("open ICC device failed!\n");
            
            printf("### %s open ICC device:%s failed!\n", __func__,ICC_DEVICE);
            
            return;
        }
        
     //   printf("*****%s %d********\n",  __func__,__LINE__);
      /*
        munmap(myOSICCDrv, 1024);
        myOSICCDrv = (DRV_MMAP *)mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd_icc, 0);

        if (myOSICCDrv == MAP_FAILED)
        {
           printf("OSDRV_Active mmap  for ICC\n");
           return ;
        }
        */

        myOSICCDrv = (DRV_MMAP *)malloc(sizeof(DRV_MMAP));
        if(!myOSICCDrv)
        {
            printf("myOSMFDrv malloc failed!\n");
        }
        
//        printf("*****%s %d********\n",  __func__,__LINE__);
    }
}

void OSICC_DrvActive(void)
{
	//printf("\r\n %s OSICCDrvIn.ucType:0x%x \r\n",__func__,OSICCDrvIn.ucType);
    OSDRV_Active(HALDRVID_ICC,&OSICCDrvIn,&OSICCDrvOut);
}

void OSICC_DrvRun(void)
{
int i=0; 
 //printf("        \r\nStep1 %s OSICCDrvIn.ucType:0x%x\r\n",__func__,OSICCDrvIn.ucType);
    OSDRV_Active(HALDRVID_ICC,&OSICCDrvIn,&OSICCDrvOut2);
//  printf("2\n");
  //  OSDRV_Wait(&OSICCDrvOut2);
//printf("          \r\nStep3 OSICC_DrvRun\r\n");

#ifdef DBG1
 //   Uart_Printf("\r\n*************** ===== %s -===== ************************\r\n", __func__);
 //   Uart_Printf("\r\n %s OSICCDrvOut2.ucReqNR=%x ucGenStatus=%x  ucDrvStatus=%x\r\n", __func__,
        OSICCDrvOut2.ucReqNR, OSICCDrvOut2.ucGenStatus, OSICCDrvOut2.ucDrvStatus);

    for (i=0; i<OSICCDrvOut2.aucData[0]+1; i++)
        Uart_Printf("%02x ", OSICCDrvOut2.aucData[i]);
   // Uart_Printf("\r\n");
 //   Uart_Printf("\r\n*************** [%s,%d] ************************\r\n", __func__,__LINE__);
#endif
}

DRV_OUT *OSICC_Insert(void)
{
    OSICCDrvIn.ucType = PREREC;
    OSICCDrvIn.ucLen = 1;
    OSICCDrvIn.aucData[0] = ICC1;

    OSICC_DrvActive();
    return (DRV_OUT *)&myOSICCDrv->pDrvOut;
}

void OSICC_Remove(void)
{
    OSICCDrvIn.ucType = DECRET;
    OSICCDrvIn.ucLen=1;
    OSICCDrvIn.aucData[0] = ICC1;
    OSICC_DrvRun();
}

void OSICC_PowerOff(void)
{
    OSICCDrvIn.ucType = MHTCAM;
    OSICCDrvIn.ucLen=1;
    OSICCDrvIn.aucData[0] = ICC1;
    OSICC_DrvRun();
}




/*******************************************************************************
 * ����API�ӿڵ�ʵ��
 *
 ********************************************************************************/
DRVOUT *OSICC_InsertCard(void)
{
    OSICCDrvIn.ucType = INSERTCARD;
    OSICCDrvIn.ucLen = 1;
    OSICCDrvIn.aucData[0] = ICC1;

    OSICC_DrvActive();
//    return(&OSICCDrvOut);
    return (DRVOUT *)&myOSICCDrv->pDrvOut;
}

DRV_OUT *OSICC_OrderAsync(unsigned char *pucInData,unsigned short uiLen)
{
    OSICCDrvIn.ucType = EXEISO;
    OSICCDrvIn.ucLen = uiLen+1;
    OSICCDrvIn.aucData[0] = ICC1;
    memcpy(&OSICCDrvIn.aucData[1],pucInData,uiLen);
    OSICC_DrvRun();
    return((DRV_OUT*)&OSICCDrvOut2);
}

DRV_OUT *OSICC_Order(DRVIN  *pDrvIn)
{
    OSICCDrvIn.ucType = pDrvIn->ucType;
    OSICCDrvIn.ucLen = pDrvIn->ucLen+1;
    OSICCDrvIn.aucData[0] = ICC1;
    memcpy(&OSICCDrvIn.aucData[1],pDrvIn->aucData,pDrvIn->ucLen);
    OSICC_DrvRun();
    return((DRV_OUT*)&OSICCDrvOut2);
}

unsigned char OSICC_Detect(unsigned char ucReader)
{


    if (OSICCDrvOut2.ucGenStatus == HALDRVSTATE_RUNNING)
        OSDRV_Abort(HALDRVID_ICC);

    OSICCDrvIn.ucType = TST_PRES;
    OSICCDrvIn.ucLen = 1;
    OSICCDrvIn.aucData[0] = ucReader;

    OSICC_DrvRun();

#ifdef DBG
    Uart_Printf("%s OSICCDrvOut2.ucGenStatus=%x ucDrvStatus=%x aucData[1]=%x\n",
           __func__, OSICCDrvOut2.ucGenStatus, OSICCDrvOut2.ucDrvStatus, OSICCDrvOut2.aucData[1]);
#endif

    if (  (OSICCDrvOut2.ucGenStatus == HALDRVSTATE_ENDED)
            &&(OSICCDrvOut2.ucDrvStatus == OK)
            &&(OSICCDrvOut2.aucData[1] == 0x30)
       )
    {
        return(OK);
    }

    return(NOICC);
}
ICC_ANSWER *OSICC_Command(unsigned char ucReader, ICC_ORDER *pOrder)
{

    unsigned char ucResult;

    pICCOrder = pOrder;
    ICCReport.drv_status = WRONGORDER;
    ICCReport.card_status = 0;
    ICCReport.Len = 0;
//#ifdef DBG
  //  Uart_Printf("\r\n=============>1 %s pICCOrder->order=%x, ucReader=%x\r\n", __func__, pICCOrder->order, ucReader);
//#endif
    if ( (ucResult = OSICC_ChangeReader(ucReader))!=OK)
    {
        OSICC_SetReport(ucResult);
        return(&ICCReport);
    }
//#ifdef DBG
//    Uart_Printf("\r\n=============>2 %s pICCOrder->order=%x, ucReader=%x\r\n", __func__, pICCOrder->order, ucReader);
//#endif
    if (  (pICCOrder->ptout==NULL)
            &&(pICCOrder->order!=POWER_OFF)
       )
    {
        OSICC_SetReport(NULLPTOUT);
        return(&ICCReport);
    }

//#ifdef DBG
//    Uart_Printf("\r\n=============>3 %s pICCOrder->order=%x, ucReader=%x\r\n", __func__, pICCOrder->order, ucReader);
//#endif

    switch (pICCOrder->order)
    {
    case POWER_ON:
        OSICC_SetReport(OSICC_PowerOn(0));
		printf("finish\n");
        break;

    case NEW_POWER_ON:
        OSICC_SetReport (OSICC_NewPowerOn(0));
        break;

    case POWER_OFF:
        ICCReport.card_status = 0x30;
        OSICCDrvIn.ucType = MHTCAM;
        OSICCDrvIn.ucLen = 1;
        OSICCDrvIn.aucData[0] = ucICCCurrReader;
        OSICC_DrvRun();
        OSICC_SetReport(OK);
        break;

    case SYNC_ORDER:
        OSICC_SetReport(OSICC_SyncOrder());
        break;

    case ASYNC_ORDER:
        OSICC_SetReport(OSICC_AsyncOrder());
        break;

    case ASYNC_PROTOCOL:
        OSICC_SetReport(OSICC_AsyncProtocol(pICCOrder->pt_order_in->async_protocol));
        break;

    case CHANGE_PROTOCOL:
        OSICC_SetReport(OSICC_ChangeProtocol(pICCOrder->pt_order_in->async_protocol));
        break;

    case SET_ICC_STANDARD:
        OSICC_SetReport(OSICC_SetICCStandard(pICCOrder->pt_order_in->icc_standard));
        break;

    case WARM_RESET:
        OSICC_SetReport(OSICC_PowerOn(1));
        break;

    case NEW_WARM_RESET:
        OSICC_SetReport (OSICC_NewPowerOn(1));
        break;

    case NEW_SYNC_ORDER:
        OSICC_SetReport(OSICC_NewSyncOrder());
        break;

    default:
        break;
    }
    return(&ICCReport);
}

void    OSICC_SetReport(unsigned char ucICCStatus)
{
    ICCReport.drv_status = ucICCStatus;
}

unsigned char OSICC_ChangeReader(unsigned char ucReader)
{
#ifdef DBG
    Uart_Printf("\n\n!!!!!%s0 %d ucReader=%x ucICCCurrReader=%x\n", __func__, __LINE__, ucReader, ucICCCurrReader);
#endif

    if (ucReader == ucICCCurrReader)
        return (OK);

    OSICCDrvIn.ucType = SELECT;
    OSICCDrvIn.ucLen = 1;
    OSICCDrvIn.aucData[0] = ucReader;

    OSICC_DrvRun();

    if (  (OSICCDrvOut2.ucGenStatus == HALDRVSTATE_ENDED)
            &&(OSICCDrvOut2.ucDrvStatus == OK)
            &&(OSICCDrvOut2.aucData[1] == 0x30)
       )
    {
        ucICCCurrReader=ucReader;
#ifdef DBG
        Uart_Printf("!!!!!%s1 %d ucReader=%x ucICCCurrReader=%x\n\n\n", __func__, __LINE__,ucReader, ucICCCurrReader);
#endif
        return(OK);
    }
    return(WRONGREADER);
}

unsigned char   OSICC_Activate(unsigned char ucResetType)
{
    if ( ucResetType == 0)
    {
        /*
        EN: Cold Reset
        */
        OSICCDrvIn.ucType = PREREC;
    }
    else
    {
        /*
        EN: Warm Reset
        */
        OSICCDrvIn.ucType = WARMRST;
    }

   // printf("\r\n[%s,%d]\r\n",__func__,__LINE__);
    OSICC_DrvRun();

   // printf("\r\n[%s,%d]  OSICCDrvOut2.ucGenStatus:0x%x OSICCDrvOut2.ucDrvStatus:0x%x\r\n ",
    //		  __func__,__LINE__,OSICCDrvOut2.ucGenStatus,OSICCDrvOut2.ucDrvStatus);

    if (  (OSICCDrvOut2.ucGenStatus == HALDRVSTATE_ENDED)
            &&(OSICCDrvOut2.ucDrvStatus == OK)
       )
    {
        ICCReport.card_status = OSICCDrvOut2.aucData[1];
        if (OSICCDrvOut2.aucData[0] > 1)
        {
            ICCReport.Len = OSICCDrvOut2.aucData[0]-2;
            memcpy(pICCOrder->ptout,&OSICCDrvOut2.aucData[3],ICCReport.Len);
        }
        else
            ICCReport.Len = 0;
    }
    if (OSICCDrvOut2.ucGenStatus == HALDRVSTATE_ENDED)
        return(OSICCDrvOut2.ucDrvStatus);
    else
        return(OSICCDrvOut2.ucGenStatus);
}

unsigned char   OSICC_PowerOn(unsigned char ucResetType)
{
 //printf("\r\n------>[%s,%d]\r\n",__func__,__LINE__);
 	// *(int*)0 = 0;

    if (OSICC_Detect(ucICCCurrReader))
    {
        Uart_Printf("!!! there is no card! %s:%x\n", __func__, ucICCCurrReader);
        return(NOICC);
    }

    OSICCDrvIn.ucLen = 1;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    return (OSICC_Activate(ucResetType));
}

unsigned char   OSICC_NewPowerOn(unsigned char ucType)
{
    if (OSICC_Detect(ucICCCurrReader))
        return(NOICC);

    OSICCDrvIn.ucLen = 3;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    OSICCDrvIn.aucData[1] = pICCOrder->pt_order_in->new_power.card_standards;

    if (pICCOrder->pt_order_in->new_power.GR_class_byte_00 == 0)
    {
        OSICCDrvIn.aucData[1] &= 0x7f;
    }
    else
    {
        OSICCDrvIn.aucData[1] |= 0x80;
    }
    OSICCDrvIn.aucData[2] = pICCOrder->pt_order_in->new_power.preferred_protocol;

    return (OSICC_Activate(ucType));
}

void    OSICC_AsyncOrderPrepare(void)
{
    unsigned char ucCaseType4;
    unsigned short uiLen;

    OSICCDrvIn.ucType = EXEISO;
    if (pICCOrder->pt_order_in->async_order.order_type == 5)
        pICCOrder->pt_order_in->async_order.order_type = 3;

    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    OSICCDrvIn.aucData[1] = pICCOrder->pt_order_in->async_order.order_type;
    memcpy (&OSICCDrvIn.aucData[2],
            &pICCOrder->pt_order_in->async_order.as_order.order_type1.CLA,
            5);

    ucCaseType4 = 0;
    switch (OSICCDrvIn.aucData[1])
    {
        /*
        EN: direct data => impossible in T=0 protocol
        */
    case 0:
        OSICCDrvIn.ucLen = pICCOrder->pt_order_in->async_order.as_order.order_type0.Lc+2;
        memcpy(&OSICCDrvIn.aucData[2],
               pICCOrder->pt_order_in->async_order.as_order.order_type0.ptin,
               (OSICCDrvIn.ucLen-2));
        break;
        /*
        EN: type1 : CLA,INS,P1,P2
        */
    case 1:
        OSICCDrvIn.ucLen = 6;
        break;
        /*
        EN:  type2 : CLA,INS,P1,P2,Le
        */
    case 2:
        OSICCDrvIn.ucLen = 7;
        break;
        /*
        EN:  type3 : CLA,INS,P1,P2,Lc,data...
        */
    case 3:
        /*
        EN:  type4 : CLA,INS,P1,P2,Lc,data...,Le
        */
    case 4:
        if (OSICCDrvIn.aucData[1] == 4)
            ucCaseType4 = 1;

        if (pICCOrder->pt_order_in->async_order.as_order.order_type3.Lc < (249 - ucCaseType4))
        {
            OSICCDrvIn.ucLen = pICCOrder->pt_order_in->async_order.as_order.order_type3.Lc + 7 + ucCaseType4;
            memcpy(&OSICCDrvIn.aucData[7],
                   pICCOrder->pt_order_in->async_order.as_order.order_type3.ptin,
                   (OSICCDrvIn.ucLen-7));
        }
        else
        {
            OSICCDrvIn.ucLen = 255;
            OSICCDrvIn.aucData[3] = OSICCDrvIn.aucData[1];
            uiLen = pICCOrder->pt_order_in->async_order.as_order.order_type3.Lc + 7 + ucCaseType4;
            OSICCDrvIn.aucData[0] = uiLen/256;
            OSICCDrvIn.aucData[1] = uiLen%256;
            OSICCDrvIn.aucData[2] = ucICCCurrReader;
            memcpy (&OSICCDrvIn.aucData[4], &pICCOrder->pt_order_in->
                    async_order.as_order.order_type3.CLA, 5);
            memcpy(&OSICCDrvIn.aucData[9],  pICCOrder->pt_order_in->
                   async_order.as_order.order_type3.ptin, 256);
        }
        break;
        /*
        EN: direct data > 255  => impossible in T=0 protocol
        */
    case 6:
        OSICCDrvIn.ucLen = 255;
        OSICCDrvIn.aucData[3] = OSICCDrvIn.aucData[1];
        *((unsigned short *)OSICCDrvIn.aucData) = (unsigned short)(pICCOrder->pt_order_in->
                async_order.as_order.order_type6.Lc) + 1;
        memcpy( &OSICCDrvIn.aucData[4],
                pICCOrder->pt_order_in->async_order.as_order.order_type6.ptin,
                pICCOrder->pt_order_in->async_order.as_order.order_type6.Lc);
        break;
        /*
        EN: S-IFSD to card
        */
    case 7:
        OSICCDrvIn.ucLen = 255;
        OSICCDrvIn.aucData[3] = 1;
        *((unsigned short *)OSICCDrvIn.aucData) = 6000;
        OSICCDrvIn.aucData[9] = pICCOrder->pt_order_in->async_order.NAD;
        return;
    }
    if (OSICCDrvIn.ucLen != 255)
    {
        if (ucCaseType4 == 1)  OSICCDrvIn.aucData[OSICCDrvIn.ucLen-1] =
                pICCOrder->pt_order_in->async_order.as_order.order_type4.Le;
        OSICCDrvIn.aucData[OSICCDrvIn.ucLen] =
            pICCOrder->pt_order_in->async_order.NAD;
    }
    /*
        else
        {

            if (ucCaseType4 == 1)
                OSICCDrvIn.aucData[*((unsigned short *)OSICCDrvIn.aucData) + 2 - 1]
                = pICCOrder->pt_order_in->async_order.as_order.order_type4.Le;
            OSICCDrvIn.aucData[*((unsigned short *)OSICCDrvIn.aucData) + 2] =
                pICCOrder->pt_order_in->async_order.NAD;
        }
    */
}

unsigned char OSICC_AsyncOrder(void)
{
    if (pICCOrder->pt_order_in->async_order.order_type > 7)
        return(WRONGORDER);

    OSICC_AsyncOrderPrepare();

    OSICC_DrvRun();

    if ( OSICCDrvOut2.ucGenStatus != HALDRVSTATE_ENDED )
        return(OSICCDrvOut2.ucGenStatus);
    if ( OSICCDrvOut2.ucDrvStatus != OK )
        return(OSICCDrvOut2.ucDrvStatus);

    ICCReport.card_status = OSICCDrvOut2.aucData[1];

    if (OSICCDrvOut2.aucData[0] < 2)
    {
        ICCReport.Len = 0;
        return(OK);
    }

    if (OSICCDrvOut2.aucData[0] < 3)
    {
        ICCReport.Len = OSICCDrvOut2.aucData[0]-1;
        memcpy(pICCOrder->ptout,&OSICCDrvOut2.aucData[4], ICCReport.Len + 2);
        return(OK);
    }

    if (OSICCDrvOut2.aucData[0] != 0xff)
    {
        ICCReport.Len = OSICCDrvOut2.aucData[0]-3;
        ucDataOffset = 0;
    }
    else
    {
        ICCReport.Len = OSICCDrvOut2.aucData[1]*256 + OSICCDrvOut2.aucData[2] - 3;
        ICCReport.card_status = OSICCDrvOut2.aucData[3];
        ucDataOffset = 2;
    }

//    uiNADPos = 4 + ucDataOffset + ICCReport.Len + 2;

    if (  (pICCOrder->pt_order_in->async_order.order_type == 2)
            &&(pICCOrder->pt_order_in->async_order.as_order.order_type2.Le != 0)
            &&(pICCOrder->pt_order_in->async_order.as_order.order_type2.Le < ICCReport.Len)
       )
        ICCReport.Len = pICCOrder->pt_order_in->async_order.as_order.order_type2.Le;

    if (  (pICCOrder->pt_order_in->async_order.order_type == 4)
            &&(pICCOrder->pt_order_in->async_order.as_order.order_type4.Le != 0)
            &&(pICCOrder->pt_order_in->async_order.as_order.order_type4.Le < ICCReport.Len)
       )
        ICCReport.Len = pICCOrder->pt_order_in->async_order.as_order.order_type4.Le;

    memcpy(pICCOrder->ptout, &OSICCDrvOut2.aucData[4+ucDataOffset], ICCReport.Len);

    memcpy(&pICCOrder->ptout[ICCReport.Len],&OSICCDrvOut2.aucData[2+ucDataOffset],2);

    /*
        if (OSICCDrvOut2.aucData[uiNADPos+1] != 0xff)
            memcpy(&pICCOrder->ptout[ICCReport.Len+2],&OSICCDrvOut2.aucData[uiNADPos],2);
    */

    ICCReport.Len += 2;
    return(OK);
}

unsigned char   OSICC_AsyncProtocol(unsigned char protocol)
{
    OSICCDrvIn.ucType = PROTOCOL;
    OSICCDrvIn.ucLen = 1;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;

    OSICC_DrvRun();

    ICCReport.card_status = '0';

    if (OSICCDrvOut2.aucData[1] == protocol)
        return(OK);
    else
        return(KO);
}

unsigned char   OSICC_ChangeProtocol(unsigned char protocol)
{
    OSICCDrvIn.ucType = WRPROT;
    OSICCDrvIn.ucLen = 2;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    OSICCDrvIn.aucData[1] = protocol;
    OSICC_DrvRun();

    ICCReport.card_status = '0';

    if (OSICCDrvOut2.aucData[1] == 1)
        return(OK);
    else
        return(KO);
}

unsigned char   OSICC_SetICCStandard(unsigned char new_standard)
{
    OSICCDrvIn.ucType = ICC_STD;
    OSICCDrvIn.ucLen = 2;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    OSICCDrvIn.aucData[1] = new_standard;

    OSICC_DrvRun();

    ICCReport.card_status = '0';

    if (OSICCDrvOut2.aucData[1] == 1)
        return(OK);
    else
        return(KO);
}

unsigned char OSICC_ExecSyncOrder(void)
{
    OSICC_DrvRun();

    if ( OSICCDrvOut2.ucGenStatus!=HALDRVSTATE_ENDED )
    {
        return(OSICCDrvOut2.ucGenStatus);
    }
    if (OSICCDrvOut2.ucDrvStatus!=OK)
    {
        return(OSICCDrvOut2.ucDrvStatus);
    }
    ICCReport.card_status=OSICCDrvOut2.aucData[1];
    if (OSICCDrvOut2.aucData[0]<2)
    {
        ICCReport.Len=0;
        return(OK);
    }

    ICCReport.Len=OSICCDrvOut2.aucData[0]-1;
    memcpy(pICCOrder->ptout,&OSICCDrvOut2.aucData[2],ICCReport.Len);

    return(OK);
}

unsigned char OSICC_SyncOrder(void)
{
    OSICCDrvIn.ucType = 0x0E;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    OSICCDrvIn.aucData[1] = pICCOrder->pt_order_in->sync_order.card_type;
    OSICCDrvIn.aucData[2] = pICCOrder->pt_order_in->sync_order.INS;
    OSICCDrvIn.aucData[3] = pICCOrder->pt_order_in->sync_order.ADDH;
    OSICCDrvIn.aucData[4] = pICCOrder->pt_order_in->sync_order.ADDL;
    OSICCDrvIn.aucData[5] = pICCOrder->pt_order_in->sync_order.Len;
    OSICCDrvIn.ucLen = 6+(pICCOrder->pt_order_in->sync_order.Len/8);
    if (pICCOrder->pt_order_in->sync_order.Len%8)
        OSICCDrvIn.ucLen++;
    memcpy(&OSICCDrvIn.aucData[5],
           pICCOrder->pt_order_in->sync_order.ptin,
           190);
    return(OSICC_ExecSyncOrder());
}

unsigned char OSICC_NewSyncOrder(void)
{
    OSICCDrvIn.ucType = SYNAPP;
    OSICCDrvIn.ucLen=pICCOrder->pt_order_in->new_sync_order.Len+1;
    OSICCDrvIn.aucData[0] = ucICCCurrReader;
    memcpy(&OSICCDrvIn.aucData[1],
           pICCOrder->pt_order_in->new_sync_order.ptin,
           OSICCDrvIn.ucLen-1);

    return(OSICC_ExecSyncOrder());
}

