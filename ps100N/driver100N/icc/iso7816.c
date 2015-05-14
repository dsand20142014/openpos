/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  iso7816.c
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
#include "libicc.h"
#include <linux/string.h>
//#include <tools.h> //zyadd
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
extern int icc_dbg;
extern int gxd;

unsigned char ISO7816_AsyncReset(unsigned char ucReader)
{
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;
        unsigned char ucPresent;

        pCurrICCInfo = &ICCInfo[ucReader];

        if (icc_dbg>=3)
                printk("%s line=%d ucReader=%d\n", __func__, __LINE__, ucReader);

        ucResult = HALICC_ResetUart(ucReader);
        if ( !ucResult ) {
                ucResult = HALICC_SetMethod(ucReader,ICCMETHOD_ASYNC);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetClock(ucReader,0,1);
        }
        if ( !ucResult ) {
                pCurrICCInfo->ucATRLen = 0;
                memset(pCurrICCInfo->aucATR,0,sizeof(pCurrICCInfo->aucATR));
                ucResult = HALICC_SetBaud(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetDi(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetProtocol(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetConvention(ucReader,1);
        }
        if ( !ucResult ) {
//        ucResult = HALICC_SetPowerType(ucReader,pCurrICCInfo->ucPowerType);
                ucResult = HALICC_SetPowerType(ucReader,HALICC_POWER5V); //ZYTODO
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetPEC(ucReader,0);
        }
        if ( !ucResult ) {
                ucResult = HALICC_Active(ucReader);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetTimer(ucReader,0x61,0x00,0x6C,0x00,0);
        }
        if ( !ucResult ) {
                ucResult = HALICC_Detect(ucReader,&ucPresent);
        }
        if ( !ucResult ) {
                if ( ucPresent ) {
                        pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
                        pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
                        return( ucResult );
                }
        }
      //  printk("!!!!!!!%s:%d detect card failed!\n", __func__, __LINE__);
        HALICC_Deactive(ucReader);
        LIBICC_CallbackCardRemove(ucReader);
           //  pCurrICCInfo->ucSession = ICCSESSION_IDLE;

        return(ucResult);
}

unsigned char ISO7816_WarmReset(unsigned char ucReader)
{
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;
        unsigned char ucPresent;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (icc_dbg)
                printk("%s line=%d ucReader=%d, pCurrICCInfo->ucSession=%x, ucReader=%x\n", __func__, __LINE__, ucReader, pCurrICCInfo->ucSession, ucReader);
        ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
        if ( !ucResult ) {
                pCurrICCInfo->ucATRLen = 0;
                memset(pCurrICCInfo->aucATR,0,sizeof(pCurrICCInfo->aucATR));
                ucResult = HALICC_SetBaud(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetDi(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetProtocol(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetConvention(ucReader,1);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetPEC(ucReader,0);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetTimer(ucReader,0x61,0x00,0x6C,0x00,0);
        }
        if ( !ucResult ) {
                ucResult = HALICC_Detect(ucReader,&ucPresent);
        }
        if ( !ucResult ) {
                if ( ucPresent ) {
                        pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
                        pCurrICCInfo->ucResetType = LIBICC_WARMRESET;
                        pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
                        return( ucResult );
                }
        }

        printk("!!!!!!!%s:%d detect card failed!\n", __func__, __LINE__);
        HALICC_Deactive(ucReader);
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;

        return(ucResult);
}

unsigned char ISO7816_SetAPDUNextCommand(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE2)
                        ||(pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4)
           ) {  //EN: Get Response
                if (  (pCurrICCInfo->aucAPDURecvBuf[0] == 0x61)
                                ||(  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4)
                                     &&(  (pCurrICCInfo->aucAPDURecvBuf[0] == 0x62)
                                          ||(pCurrICCInfo->aucAPDURecvBuf[0] == 0x63)
//                 ||(pCurrICCInfo->aucAPDURecvBuf[0] == 0x90)
                                       )
                                  )
                   ) {
                        pCurrICCInfo->aucAPDUSendBuf[1] = 0xC0;
                        pCurrICCInfo->aucAPDUSendBuf[2] = 0x00;
                        pCurrICCInfo->aucAPDUSendBuf[3] = 0x00;
                        if ( pCurrICCInfo->aucAPDURecvBuf[0] == 0x61 ) {
                                pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[1];
                        } else {
                                if ( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 ) {
                                        pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDUSendBuf[pCurrICCInfo->uiAPDUSendLen];
                                } else {
                                        pCurrICCInfo->aucAPDUSendBuf[4] = 0x00;
                                }
                        }
                        pCurrICCInfo->uiAPDUSendLen = 5;
                        pCurrICCInfo->uiAPDUSentLen = 0;
                        pCurrICCInfo->uiAPDURecvedLen = 0;
                        memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
                        pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
                        return(LIBICCERR_SUCCESS);
                }
        }
        if ( pCurrICCInfo->aucAPDURecvBuf[0] == 0x6C ) {
                pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[1];
                pCurrICCInfo->uiAPDUSentLen = 0;
                pCurrICCInfo->uiAPDURecvedLen = 0;
                memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
                return(LIBICCERR_SUCCESS);
        }
        return(LIBICCERR_APDUNONEXTCOMMAND);
}

unsigned char ISO7816_CheckICCParam(unsigned char ucReader)
{
        return(LIBICCERR_SUCCESS);
}

void ISO7816_RecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (pCurrICCInfo->ucSession) {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ISO7816_ResetRecvCharProc(ucReader,ucCh);
        case ICCSESSION_DATAEXCH:
                ISO7816_APDUExchRecvCharProc(ucReader,ucCh);
                return;
        default:
                break;
        }
}

void ISO7816_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        switch (pCurrICCInfo->ucSessionStep) {
        case ICCSESSIONSTEP_COLDRESET2:
        case ICCSESSIONSTEP_COLDRESET3:
                if ( pCurrICCInfo->ucATRLen < LIBICC_MAXATRBUF) {
                        pCurrICCInfo->aucATR[pCurrICCInfo->ucATRLen] = ucCh;
                        pCurrICCInfo->ucATRLen ++;
                }
                ucResult = ATR_CheckFinished(pCurrICCInfo->aucATR,pCurrICCInfo->ucATRLen);
                if (  (ucResult != LIBICCERR_ATRNOTFINISHED)
                                &&(ucResult != LIBICCERR_ATRHASLASTCHAR)
                   ) {
                    if(gxd)
                	printk("ATR END!!!\n");
                        HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
                        pCurrICCInfo->ucSession = ICCSESSION_IDLE;
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;
                        pCurrICCInfo->ucLastError = ucResult;
                        ATR_Process(ucReader);
                        ATR_FinishedProc(ucReader);
                } else {
                  if (gxd)
                	printk("ATR no END  ucCh = 0x%x!!!\n", ucCh);
                        HALICC_SetTimer(ucReader,0x61,0x00,0x80,0x25,1);
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
                }
                break;
        default:
                LIBICC_CODENOTIMPLEMENT
                break;
        }
}

void ISO7816_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
}

void ISO7816_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (icc_dbg>=3)
                printk("%s:%d ucSession:%x\n", __func__, __LINE__, pCurrICCInfo->ucSession);
        switch (pCurrICCInfo->ucSession) {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                ISO7816_ResetTimeoutProc(ucReader,ucTimeoutType);
                break;
        default:
                break;
        }
}
int gxd = 0;
void ISO7816_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        if (gxd)
        printk("%s:%d, pCurrICCInfo->ucSessionStep=%x, ucReader=%x\n", __func__, __LINE__, pCurrICCInfo->ucSessionStep, ucReader);


        if (icc_dbg>=2) //zy tmp 0414
                printk("%s:%d, pCurrICCInfo->ucSessionStep=%x, ucReader=%x\n", __func__, __LINE__, pCurrICCInfo->ucSessionStep, ucReader);

        switch (pCurrICCInfo->ucSessionStep) {
        case ICCSESSIONSTEP_COLDRESET1:
                HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0);

                HALICC_SetTimer(ucReader,0x61,0x00,0x78,0x00,1);
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET2;
                if (gxd)
                       printk("%s:%d, pCurrICCInfo->ucSessionStep=%x, ucReader=%x\n", __func__, __LINE__, pCurrICCInfo->ucSessionStep, ucReader);
                return;
        case ICCSESSIONSTEP_COLDRESET2:
                HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,0);
                HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
                pCurrICCInfo->ucLastError = LIBICCERR_ISSYNCCARD;
                break;
        case ICCSESSIONSTEP_COLDRESET3:
                HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,0);
                HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
                pCurrICCInfo->ucLastError = LIBICCERR_CARDMUTE;
//        printk("TimeoutType=%02x\n",ucTimeoutType);
                break;
        default:
                break;
        }

     //   printk("%s, line:%d,pCurrICCInfo->ucLastError :0x%x\n",__func__, __LINE__,pCurrICCInfo->ucLastError);
        ATR_FinishedProc(ucReader);
}
