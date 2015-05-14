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
#include "libicc.h"
//#include <tools.h> //zy del
/*zy add*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/sched.h>

extern int icc_dbg;
extern unsigned long addrN[2][16];
extern struct completion comp;
extern wait_queue_head_t my_queue;

/*
ICCTRT:
			  DW    TST_PRES          ; (00H) - test pr�sence carte
$IF (MICROSIM)
			  DW    SELECT_ICC
$ELSE
			  DW    RAFICC            ; (01H) - Commande autoris�e sans traitement
$ENDIF
$IF (ICC_PROT)
                    DW    WRPROT            ; (02H) - Change ICC protocol
$ELSE
			  DW    INTICC            ; (02H) - Commande interdite
$ENDIF
			  DW    INTICC            ; (03H) - Commande interdite
			  DW    ATTPRE            ; (04H) - Attente pr�sence carte
			  DW    RECART            ; (05H) - Identification type de carte
			  DW    INTICC            ; (06H) - Commande interdite
			  DW    PREREC            ; (07H) - Attente pr�s et id type de carte
			  DW    RECIDE            ; (08H) - Id et contr�le identite carte
$IF (ICC_STD)
			  DW    STANDARD          ; (09H) - Try a new activation standard
$ELSE
			  DW    RAFICC            ; (09H) - Commande autoris�e sans traitement
$ENDIF
			  DW    DECONN            ; (0AH) - D�connexion interface CAM
			  DW    ATTRET            ; (0BH) - Attente retrait carte
			  DW    DECRET            ; (0CH) - D�connexion, attente retrait carte
			  DW    ISOEXE            ; (0DH) - Ex�cution ordre ISO 7816-3
			  DW    RAFICC            ; (0EH) - Ex-Ex�cution ordre carte synchrone
			  DW    EXEFCT            ; (0FH) - Ex�cution d'une fonction
$IF (SYNCHRONE)
			  DW    SYNAPP            ; (10H) - Nouveau traitement synchrone
$ENDIF						  ;         (IP le 24/07/97)
			  DW    WARMRST           ; (11H) - Warm Reset
			  DW    PROTOCOL          ; (12H) - Returns selected card protocol
			  DW    EMVMODUL          ; (13H) - Identifies EMV module
FICCTRT:
			  DW    INTICC            ; (00H) - Commande interdite
*/

ICCCMD  ICCCMDTable[ICCCMD_MAXNB] = {
        {ICCCMD_Detect},                       //0x00 TST_PRES
        {ICCCMD_Select},                       //0x01 SELECT
        {ICCCMD_ChangeProtocol},        //0x02 WRPROT
        {0},                                           //0x03
        {0},                                          //0x04 ATTPRE
        {0},                                           //0x05 RECART
        {0},                                           //0x06
        {ICCCMD_Insert},                      //0x07 PREREC
        {0},                                          //0x08 RECIDE
        {ICCCMD_SetStandard},           //0x09 ICC_STD
        {ICCCMD_PowerOff},                //0x0A MHTCAM
        {0},                                         //0x0B ATTRET
        {ICCCMD_Remove},                  //0x0C DECRET
        {ICCCMD_AsyncCommand},          //0x0D EXEISO
        {0},                                     //0x0E EXESYN
        {0},                            //0x0F
        {ICCCMD_NewSyncCommand},        //0x10 SYNAPP
        {ICCCMD_WarmReset},             //0x11 WARMRST
        {0},                            //0x12 PROTOCOL
        {0},                            //0x13 EMVMODUL
        {ICCCMD_InsertCard},            //0x14 INSERTCARD
};

unsigned char ICCCMD_Process(unsigned char ucCMD,
                             unsigned char *pucInData,
                             unsigned short uiInLen,
                             unsigned char *pucOutData)
{
        unsigned char ucResult;
        int i;



        if ( ucCMD >= ICCCMD_MAXNB ) {
                return(LIBICCERR_BADCMD);
        }
        if (  (!pucInData)
                        ||(!uiInLen)
           ) {
                return(LIBICCERR_BADCMDINPARAM);
        }
        if (!pucOutData) {
                return(LIBICCERR_BADCMDOUTPARAM);
        }
        if ( !ICCCMDTable[ucCMD].pfnProc ) {
                return(LIBICCERR_CMDNOTIMPLEMENT);
        } else {



      //          if (icc_dbg)
      // 	if (1)
        //                printk("\n%s:%d ucCMD=%x\n", __func__, __LINE__, ucCMD);

                ucResult = (*ICCCMDTable[ucCMD].pfnProc)(pucInData,uiInLen,pucOutData);

                if (icc_dbg) {
                        printk("\n\n*************** %s ******************************\n", __func__);

                        printk("%s DRVIN len=%x\n", __func__,uiInLen);
                        for (i=0; i<uiInLen; i++)
                                printk("%02x ", pucInData[i]);
                        printk("\n");

                        printk("%s DRV_OUT len=%x\n", __func__,pucOutData[0]);
                        for (i=0; i<pucOutData[0]; i++)
                                printk("%02x ", pucOutData[i+1]);
                        printk("\n");
                        printk("*************** %s END******************************\n", __func__);
                }

                return ucResult;


        }
}

unsigned char ICCCMD_Detect(unsigned char *pucInData,
                            unsigned short uiInLen,
                            unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned short uiOffset;
        unsigned char ucPresent;

        uiOffset = 0;
        ucReader = *(pucInData+uiOffset);

        ucResult = HALICC_Detect(ucReader,&ucPresent);

        if ( !ucResult ) {
                if ( ucPresent ) {
                        ucPresent = 0x30;
                }
                *(pucOutData++) = 1;
                *(pucOutData++) = ucPresent;
        }
        return(ucResult);
}

unsigned char ICCCMD_Select(unsigned char *pucInData,
                            unsigned short uiInLen,
                            unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned short uiOffset;

        uiOffset = 0;
        ucReader = *(pucInData+uiOffset);

        ucResult = HALICC_SelectReader(ucReader);
        if ( !ucResult ) {
                *(pucOutData++) = 1;
                *(pucOutData++) = 0x30;
        }
        return(ucResult);
}

unsigned char ICCCMD_ChangeProtocol(unsigned char *pucInData,
                                    unsigned short uiInLen,
                                    unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned char ucProtocol;
        unsigned short uiOffset;

        uiOffset = 0;
        ucReader = *(pucInData+uiOffset);
        uiOffset ++;
        ucProtocol = *(pucInData+uiOffset);

        ucResult = HALICC_SetProtocol(ucReader,ucProtocol);
        if ( !ucResult ) {
                *(pucOutData++) = 1;
                *(pucOutData++) = 1;
        }
        return(ucResult);
}

unsigned char ICCCMD_Insert(unsigned char *pucInData,
                            unsigned short uiInLen,
                            unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned char ucPresent;
        volatile ICCINFO *pCurrICCInfo;
        ucReader = *(pucInData);
        ucResult = HALICC_Detect(ucReader,&ucPresent);
        ICCInfo[ucReader].ucStandard =(*(pucInData+1))&0x7F;
        if ( !ucResult ) {
            //    printk("%s:%d\n", __func__, __LINE__);
                pCurrICCInfo = &ICCInfo[ucReader];
                if ( !ucPresent ) {
                        //printk("%s:%d\n", __func__, __LINE__);
                        pCurrICCInfo->ucAutoPower = 1;
                        return(LIBICCERR_PROCESSPENDING);
                } else {
                        //printk("%s:%d\n", __func__, __LINE__);
                        if ( uiInLen==3 )
                                pCurrICCInfo->ucStandard = (*(pucInData+1))&0x7F;
                        ucResult = ATR_PowerOn(ucReader);
                        //   printk("%s:%d, ucResult=%x\n", __func__, __LINE__, ucResult);
                        if ( !ucResult )
                                return(LIBICCERR_PROCESSPENDING);
                        else
                                return(ucResult);
                }
        }
        return(ucResult);
}

unsigned char ICCCMD_SetStandard(unsigned char *pucInData,
                                 unsigned short uiInLen,
                                 unsigned char *pucOutData)
{
        unsigned char ucReader;
        unsigned char ucStandard;

        ucReader = *(pucInData);
        ucStandard = *(pucInData+1);
        ICCInfo[ucReader].ucStandard =ucStandard;

        *(pucOutData++) = 1;
        switch (ucStandard) {
        case ICCSTANDARD_EMV:
        case ICCSTANDARD_ISO7816:
        case ICCSTANDARD_MPEV5:
        case ICCSTANDARD_MONDEX:
        case ICCSTANDARD_GJISAM:
        case ICCSTANDARD_GJPSAM:
        case ICCSTANDARD_SHEBAO:
                *(pucOutData++) = 1;
                break;
        default:
                *(pucOutData++) = 0;
                break;
        }
        return(HALICCERR_SUCCESS);
}

unsigned char ICCCMD_PowerOff(unsigned char *pucInData,
                              unsigned short uiInLen,
                              unsigned char *pucOutData)
{
        unsigned char ucReader;

        ucReader = *(pucInData);
    //    udelay(10);

        return(HALICC_Deactive(ucReader));
}

#if 0
unsigned char ICCCMD_Remove(unsigned char *pucInData,
                            unsigned short uiInLen,
                            unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned char ucPresent;
        volatile ICCINFO *pCurrICCInfo;
        unsigned long timeout;

        printk("ICCCMD_Remove(): 0xC\n");

        ucReader = *(pucInData);
        ucResult = HALICC_Deactive(ucReader);

        readb(addrN[0][15]);
        ndelay(2);
        readb(addrN[1][15]);

        return(ucResult);
}
#endif


#if 1
unsigned char ICCCMD_Remove(unsigned char *pucInData,
                            unsigned short uiInLen,
                            unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned char ucPresent;
        volatile ICCINFO *pCurrICCInfo;
        unsigned long timeout;

       // printk("ICCCMD_Remove(): 0xC\n");

        ucReader = *(pucInData);
        ucResult = HALICC_Detect(ucReader,&ucPresent);
        if ( !ucResult ) {
                pCurrICCInfo = &ICCInfo[ucReader];
                if ( !ucPresent ) {
                        return(ucResult);
                } else {
                        HALICC_Deactive(ucReader);

                     //   while ( pCurrICCInfo->ucStatus != ICCSTATUS_REMOVE)
		//	printk("ucStatus:0x%xd\r\n", pCurrICCInfo->ucStatus);
                while ( pCurrICCInfo->ucStatus != ICCSTATUS_REMOVE)
                {

                        //	timeout = wait_for_completion_interruptible_timeout(&comp, msecs_to_jiffies(2000));
                        HALICC_Detect(ucReader,&ucPresent);
                        //	printk("%s,line:%d ucPresent:0x%x\n", __func__, __LINE__, ucPresent);
                       if(!ucPresent)   return(ucResult);
                         	//msleep(500);
                         	wait_for_completion_timeout(&comp,msecs_to_jiffies(200));
					//	if (wait_event_interruptible_timeout(my_queue,1,msecs_to_jiffies(500))>0)
					//		break;

						//wait_event_interruptible(removecard_wait, 1);
                 }
                 return(ucResult);
                }
        }
        return(ucResult);
}
#endif

unsigned char ICCCMD_AsyncCommand(unsigned char *pucInData,
                                  unsigned short uiInLen,
                                  unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;

        ucReader = *(pucInData);
        ucResult = PROT_AsyncCommand(ucReader,pucInData+1,uiInLen-1);

        if ( !ucResult ) {
                return(LIBICCERR_PROCESSPENDING);
        }
        if ( ucResult == LIBICCERR_PROCESSEND ) {
                return(LIBICCERR_SUCCESS);
        }
        return(ucResult);
}

unsigned char ICCCMD_NewSyncCommand(unsigned char *pucInData,
                                    unsigned short uiInLen,
                                    unsigned char *pucOutData)
{
        unsigned char ucReader;

        ucReader = *(pucInData);
        pucInData ++;
        uiInLen --;

        return(SYNC_RunCommand(ucReader,pucInData,uiInLen));
}

unsigned char ICCCMD_WarmReset(unsigned char *pucInData,
                               unsigned short uiInLen,
                               unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned char ucPresent;
        volatile ICCINFO *pCurrICCInfo;

        ucReader = *(pucInData);
        ucResult = HALICC_Detect(ucReader,&ucPresent);
        if ( !ucResult ) {
                pCurrICCInfo = &ICCInfo[ucReader];
                if ( !ucPresent ) {
                        return(LIBICCERR_NOICC);
                } else {
                 //       if ( uiInLen==3 )
                   //             pCurrICCInfo->ucStandard = (*(pucInData+1))&0x7F;
                        ucResult = ATR_WarmReset(ucReader);
                        if ( !ucResult )
                                return(LIBICCERR_PROCESSPENDING);
                        else
                                return(ucResult);
                }
        }
        return(ucResult);
}

unsigned char ICCCMD_InsertCard(unsigned char *pucInData,
                                unsigned short uiInLen,
                                unsigned char *pucOutData)
{
        unsigned char ucResult;
        unsigned char ucReader;
        unsigned char ucPresent;
        volatile ICCINFO *pCurrICCInfo;

        ucReader = *(pucInData);
        ucResult = HALICC_Detect(ucReader,&ucPresent);
        if ( !ucResult ) {
                pCurrICCInfo = &ICCInfo[ucReader];
                if ( !ucPresent ) {
                        pCurrICCInfo->ucAutoPower = 0;
                        return(LIBICCERR_PROCESSPENDING);
                } else {
                        return(LIBICCERR_SUCCESS);
                }
        }
        return(ucResult);
}
