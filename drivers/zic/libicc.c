/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  libicc.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-02-22		Xiaoxi Jiang
    Last modified :	2003-02-22		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include "halicc.h"
#define LIBICCDATA
#include "libicc.h"
#undef LIBICCDATA
//#include <tools.h>
#include <linux/module.h>
#include <asm/string.h>
#include <linux/sched.h>
extern struct completion comp;
extern int icc_dbg;
extern wait_queue_head_t  icc_wait;
extern wait_queue_head_t my_queue;

unsigned char ICCCMD_Process(unsigned char,  unsigned char *,unsigned short, unsigned char *);
void LIBICC_CallbackCardInsert(unsigned char);
void LIBICC_CallbackCardRemove(unsigned char);
void LIBICC_CallbackSendChar(unsigned char);
void LIBICC_CallbackRecvChar(unsigned char,unsigned char);
void LIBICC_CallbackFault(unsigned char);
void LIBICC_CallbackTimeout(unsigned char,unsigned char);
void LIBICC_CallbackParityError(unsigned char);
void LIBICC_CallbackOverrunError(unsigned char);
void LIBICC_CallbackFrameError(unsigned char);


void LIBICC_Init(void)
{
        unsigned char ICCCMD_Process(unsigned char,
                                     unsigned char *,unsigned short,
                                     unsigned char *);
        void LIBICC_CallbackCardInsert(unsigned char);
        void LIBICC_CallbackCardRemove(unsigned char);
        void LIBICC_CallbackSendChar(unsigned char);
        void LIBICC_CallbackRecvChar(unsigned char,unsigned char);
        void LIBICC_CallbackFault(unsigned char);
        void LIBICC_CallbackTimeout(unsigned char,unsigned char);
        void LIBICC_CallbackParityError(unsigned char);
        void LIBICC_CallbackOverrunError(unsigned char);
        void LIBICC_CallbackFrameError(unsigned char);

        unsigned char ucI;


        for (ucI=0; ucI<HALICC_READERNB; ucI++) {
                memset(&ICCInfo[ucI],0,sizeof(ICCINFO));
                ICCInfo[ucI].ucAutoPower = 1;
                ICCInfo[ucI].ucProcWord = 0;
                ICCInfo[ucI].ucSession = ICCSESSIONSTEP_IDLE; //ICCSESSION_COLDRESET;
                //gxd 2012.2.16
                ICCInfo[ucI].ucStandard =ICCSTANDARD_EMV;
               // ICCInfo[ucI].ucStandard =ICCSTANDARD_ISO7816;
        }

        pfnHALICCCommand = ICCCMD_Process;
        pfnCallbackCardInsert = LIBICC_CallbackCardInsert;
        pfnCallbackCardRemove = LIBICC_CallbackCardRemove;
        pfnCallbackSendChar = LIBICC_CallbackSendChar;
        pfnCallbackRecvChar = LIBICC_CallbackRecvChar;
        pfnCallbackFault = LIBICC_CallbackFault;
        pfnCallbackTimeout = LIBICC_CallbackTimeout;
        pfnCallbackParityError = LIBICC_CallbackParityError;
        pfnCallbackOverrunError = LIBICC_CallbackOverrunError;
        pfnCallbackFrameError = LIBICC_CallbackFrameError;
}

void LIBICC_CallbackCardInsert(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;


        pCurrICCInfo = &ICCInfo[ucReader];
        pCurrICCInfo->ucStatus = ICCSTATUS_INSERT;

      //  pCurrICCInfo->ucAutoPower =1;
     //   if (icc_dbg)
       //         printk("%s:%d\n", __func__, __LINE__);

      //  if (icc_dbg>=3)
     //           printk("%s %d, ucAutoPower=%d\n", __func__, ucReader, pCurrICCInfo->ucAutoPower);

        if ( pCurrICCInfo->ucAutoPower ) {
                ATR_PowerOn(ucReader);
        } else {
                HALICC_DRVEnd(LIBICCERR_SUCCESS,0,0);
        }

}

void LIBICC_CallbackCardRemove(unsigned char ucReader)
{
#if 1
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        switch ( pCurrICCInfo->ucSession) {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ATR_CallbackCardRemove(ucReader);
                break;
        case ICCSESSION_DATAEXCH:
                PROT_CallbackCardRemove(ucReader);
                break;
        default:
                if ( pCurrICCInfo->ucSession ) {
                        LIBICC_CODENOTIMPLEMENT;
                }
                break;
        }
        HALICC_Deactive(ucReader);
        pCurrICCInfo->ucStatus = ICCSTATUS_REMOVE;
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;
	 // init_waitqueue_head(&icc_wait);
    //	printk("Befor complete\r\n");
	    complete(&comp);
        wake_up_interruptible(&my_queue);
#else
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        HALICC_Deactive(ucReader);
        if (  (pCurrICCInfo->ucStatus == ICCSTATUS_POWERED )
                        &&(pCurrICCInfo->ucSession != ICCSESSION_IDLE )
                        &&(pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE)
           ) {
                pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
        }
        pCurrICCInfo->ucStatus = ICCSTATUS_REMOVE;
	complete(&comp);
#endif
}

void LIBICC_CallbackSendChar(unsigned char ucReader)
{
        PROT_CallbackSendChar(ucReader);
}

void LIBICC_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (icc_dbg>=3) printk("%s r:%x\n", __func__, ucCh);
        switch ( pCurrICCInfo->ucSession) {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ATR_CallbackRecvChar(ucReader,ucCh);
                break;
        case ICCSESSION_DATAEXCH:
                PROT_CallbackRecvChar(ucReader,ucCh);
                break;
        }
}

void LIBICC_CallbackFault(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        switch ( pCurrICCInfo->ucSession) {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ATR_CallbackFault(ucReader);
                break;
        case ICCSESSION_DATAEXCH:
                PROT_CallbackFault(ucReader);
                break;
        default:
                if ( pCurrICCInfo->ucSession ) {
                        LIBICC_CODENOTIMPLEMENT;
                }
                break;
        }
//    HALICC_Deactive(ucReader);
//    pCurrICCInfo->ucStatus = ICCSTATUS_REMOVE;
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;
}

void LIBICC_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        if (icc_dbg>=3)
                printk("\nTTTTTT %s %d pCurrICCInfo->ucSession=%x,ucTimeoutType=%x NOT be ZERO!!!!!!\n",
                       __func__, __LINE__, pCurrICCInfo->ucSession,ucTimeoutType);//0819

        switch ( pCurrICCInfo->ucSession) {
        case 0x0://zy 0504
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ATR_CallbackTimeout(ucReader,ucTimeoutType);
                break;
        case ICCSESSION_DATAEXCH:
                PROT_CallbackTimeout(ucReader,ucTimeoutType);
                break;
        }
}

void LIBICC_CallbackParityError(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch ( pCurrICCInfo->ucSession) {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ATR_CallbackParityError(ucReader);
                break;
        case ICCSESSION_DATAEXCH:
                PROT_CallbackParityError(ucReader);
                break;
        }
}

void LIBICC_CallbackOverrunError(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        pCurrICCInfo->ucLastError = LIBICCERR_OVERRECVBUF;
}

void LIBICC_CallbackFrameError(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch ( pCurrICCInfo->ucStandard ) {
        case ICCSTANDARD_ISO7816:
        case ICCSTANDARD_EMV:
        case ICCSTANDARD_GJISAM:
        case ICCSTANDARD_GJPSAM:
                pCurrICCInfo->ucLastError = LIBICCERR_BADID;
                break;
        case ICCSTANDARD_SHEBAO:
                break;
        default:
                break;
        }
}
