/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  sync.c
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
//#include <haltimer.h>
#include  "libicc.h"
//#include <tools.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/module.h>
extern int icc_dbg;
/*
#define MACRO_CODE		0xff
#define SET_DATA_CODE   0x0f

#define EOC_CODE			0x00
#define CLR_RST_CODE		0x02
#define SET_RST_CODE		0x04
#define RST_10US_CODE	0x06
#define CLR_CLK_CODE		0x08
#define SET_CLK_CODE		0x0a
#define CLK_5US_CODE		0x0c
#define CLK_10US_CODE	0x0e
#define CLR_C4_CODE		0x10
#define SET_C4_CODE		0x12
#define CLR_C8_CODE		0x14
#define SET_C8_CODE		0x16
#define VPP_IDLE_CODE	0x18
#define VPP_12V_CODE		0x1a
#define VPP_15V_CODE		0x1c
#define VPP_21V_CODE		0x1e
#define CLR_IO_CODE		0x20
#define SET_IO_CODE		0x22
#define MSB_TO_IO_CODE	0x24
#define LSB_TO_IO_CODE	0x26
#define IO_TO_MSB_CODE	0x28
#define IO_TO_LSB_CODE	0x2a
#define TST_IO_HI_CODE  0x2c
#define TST_IO_LO_CODE	0x2e
#define EXE_BADLVL_CODE	0x30
#define WT_10US_CODE		0x32
#define WT_200US_CODE	0x34
*/

#define SYNCCMD_MAXNB			0x35
#define SYNCCMD_MACROMAXLEN 	250
#define SYNCCMD_SETDATAMAXLEN	250

#define SYNCCMD_MACRO		0xff
#define SYNCCMD_SETDATA 	0x0f

#define SYNCCMD_EOC			0x00
#define SYNCCMD_CLRRST		0x02
#define SYNCCMD_SETRST		0x04
#define SYNCCMD_RST10US		0x06
#define SYNCCMD_CLRCLK		0x08
#define SYNCCMD_SETCLK		0x0A
#define SYNCCMD_CLK5US		0x0C
#define SYNCCMD_CLK10US		0x0E
#define SYNCCMD_CLRC4		0x10
#define SYNCCMD_SETC4		0x12
#define SYNCCMD_CLRC8		0x14
#define SYNCCMD_SETC8		0x16
#define SYNCCMD_VPPIDLE		0x18
#define SYNCCMD_VPP12V		0x1A
#define SYNCCMD_VPP15V		0x1C
#define SYNCCMD_VPP21V		0x1E
#define SYNCCMD_CLRIO		0x20
#define SYNCCMD_SETIO		0x22
#define SYNCCMD_MSBTOIO		0x24
#define SYNCCMD_LSBTOIO		0x26
#define SYNCCMD_IOTOMSB		0x28
#define SYNCCMD_IOTOLSB		0x2A
#define SYNCCMD_TSTIOHI		0x2C
#define SYNCCMD_TSTIOLO		0x2E
#define SYNCCMD_EXEBADLVL	0x30
#define SYNCCMD_WT10US		0x32
#define SYNCCMD_WT200US		0x34

SYNCCMD  SYNCCMDTable[SYNCCMD_MAXNB] =
{
    {SYNC_CMDEOC},			//SYNCCMD_EOC
    {0},
    {SYNC_CMDClrRST},       //SYNCCMD_CLRRST
    {0},
    {SYNC_CMDSetRST},       //SYNCCMD_SETRST
    {0},
    {SYNC_CMDRST10us},      //SYNCCMD_RST10US
    {0},
    {SYNC_CMDClrCLK},       //SYNCCMD_CLRCLK
    {0},
    {SYNC_CMDSetCLK},       //SYNCCMD_SETCLK
    {0},
    {SYNC_CMDCLK5us},       //SYNCCMD_CLK5US
    {0},
    {SYNC_CMDCLK10us},      //SYNCCMD_CLK10US
    {0},
    {SYNC_CMDClrC4},        //SYNCCMD_CLRC4
    {0},
    {SYNC_CMDSetC4},        //SYNCCMD_SETC4
    {0},
    {SYNC_CMDClrC8},        //SYNCCMD_CLRC8
    {0},
    {SYNC_CMDSetC8},        //SYNCCMD_SETC8
    {0},
    {SYNC_CMDVPPIdle},      //SYNCCMD_VPPIDLE
    {0},
    {SYNC_CMDVPP12V},       //SYNCCMD_VPP12V
    {0},
    {SYNC_CMDVPP15V},       //SYNCCMD_VPP15V
    {0},
    {SYNC_CMDVPP21V},       //SYNCCMD_VPP21V
    {0},
    {SYNC_CMDClrIO},        //SYNCCMD_CLRIO
    {0},
    {SYNC_CMDSetIO},        //SYNCCMD_SETIO
    {0},
    {SYNC_CMDMSBToIO},      //SYNCCMD_MSBTOIO
    {0},
    {SYNC_CMDLSBToIO},      //SYNCCMD_LSBTOIO
    {0},
    {SYNC_CMDIOToMSB},      //SYNCCMD_IOTOMSB
    {0},
    {SYNC_CMDIOToLSB},      //SYNCCMD_IOTOLSB
    {0},
    {SYNC_CMDTestIOHigh},   //SYNCCMD_TSTIOHI
    {0},
    {SYNC_CMDTestIOLow},    //SYNCCMD_TSTIOLO
    {0},
    {0},                    //SYNCCMD_EXEBADLVL
    {0},
    {SYNC_CMDWait10us},     //SYNCCMD_WT10US
    {0},
    {SYNC_CMDWait200us}     //SYNCCMD_WT200US
};
//extern int set_gpg11_l();
//extern int set_gpg11_h();
extern int set_gpg11_loop(int i);

#define DBG_SYNC //printk
unsigned char SYNC_RunCommand(unsigned char ucReader,unsigned char *pucInData,unsigned short uiInLen)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;

    ucResult = LIBICCERR_SUCCESS;
    pCurrICCInfo = &ICCInfo[ucReader];
//    ucResult = SYNC_AnalyseCommand(pucInData,uiInLen);
	//ICCInfo[ucReader].ucStandard = 0x1;


    if ( !ucResult )
    {
        ucResult = HALICC_SetMethod(ucReader,ICCMETHOD_SYNC);
       // printk("\n%s:%d, result=%x\n", __func__, __LINE__, ucResult);
    }
    if ( !ucResult )
    {
//		ucResult = HALICC_EnableSend(ucReader);
    }
    if ( !ucResult )
    {
//		ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
    }

    if ( !ucResult )
    {

        ucResult = SYNC_ProcessCommand(ucReader,pucInData,uiInLen);

        //printk("\n%s:%d, result=%x\n", __func__, __LINE__, ucResult);
    }

    pCurrICCInfo->ucLastError = ucResult;
    SYNC_FinishedProc(ucReader);

    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_AnalyseCommand(unsigned char *pucInData,unsigned short uiInLen)
{
    unsigned char *pucSyncCmd;
    unsigned char ucMacroNB;
    unsigned char ucLen, i;

    pucSyncCmd = pucInData;
    ucMacroNB = 0;

	for (i=0; i<uiInLen; i++){
		if (i%20==0) printk("\n");
		printk("%02x ", pucInData[i]);
	}
	printk("\n");
	
    for (pucSyncCmd=pucInData; (pucSyncCmd-pucInData)<uiInLen; pucSyncCmd++)
    {
        switch (*pucSyncCmd)
        {
        
        case SYNCCMD_EOC:
            pucSyncCmd ++;
            if ( (pucSyncCmd-pucInData) != uiInLen )
                return(LIBICCERR_SYNCEOC);
            break;
			
        case SYNCCMD_MACRO:
            ucMacroNB ++;
            if ( !(ucMacroNB%2) )
            {
                pucSyncCmd ++;
            }
            break;
			
        case SYNCCMD_SETDATA:
            pucSyncCmd ++;
            ucLen = *pucSyncCmd;
            pucSyncCmd ++;
            if ( (pucSyncCmd-pucInData+ucLen)>=uiInLen )
                return(LIBICCERR_SYNCSETDATA);
            break;
			
        default:
            if ( *pucSyncCmd < SYNCCMD_MAXNB )
            {
                if ( !SYNCCMDTable[*pucSyncCmd].pfnProc )
                {
                    return(LIBICCERR_BADSYNCCMD);
                }
            }
            else
            {
                return(LIBICCERR_BADSYNCCMD);
            }
            break;
			
        }
    }
    if ( ucMacroNB%2 )
        return(LIBICCERR_SYNCMACRO);
    return(LIBICCERR_SUCCESS);
}




unsigned char SYNC_ProcessCommand(unsigned char ucReader,unsigned char *pucInData,unsigned short uiInLen)
{
    unsigned char ucResult;
    unsigned short uiI;
    unsigned char ucJ;
    ICCINFO *pCurrICCInfo;
    unsigned char *pucSyncCmd;
    unsigned char ucMacroFlag;
    unsigned short uiMacroNB;
    unsigned char ucMacroRunFlag;
    unsigned char ucMacroLen = 0;
    unsigned char aucMacro[SYNCCMD_MACROMAXLEN];
    unsigned char ucSetDataFlag;
    unsigned char ucSetDataNB;
    unsigned char ucSetDataRunFlag;
    unsigned char ucSetDataLen = 0;
    unsigned char aucSetData[SYNCCMD_SETDATAMAXLEN];

    ucResult = LIBICCERR_SUCCESS;
    pCurrICCInfo = &ICCInfo[ucReader];
    pCurrICCInfo->ucSYNCSentBit = 0;
    pCurrICCInfo->ucSYNCSendByte = 0;
    pCurrICCInfo->ucSYNCRecvedBit = 0;
    pCurrICCInfo->ucSYNCRecvedByte = 0;
    pCurrICCInfo->uiSYNCRecvedLen = 0;
    memset(pCurrICCInfo->aucSYNCRecvBuf,0,sizeof(pCurrICCInfo->aucSYNCRecvBuf));

    ucMacroFlag = 0;
    uiMacroNB = 0;
    ucMacroRunFlag = 0;
    ucSetDataFlag = 0;
    ucSetDataNB = 0;
    ucSetDataRunFlag = 0;
    pucSyncCmd = pucInData;

    for (pucSyncCmd=pucInData; (pucSyncCmd-pucInData)<uiInLen; pucSyncCmd++)
    {
        switch (*pucSyncCmd)
        {
        case SYNCCMD_EOC://0x00
            break;

        case SYNCCMD_MACRO://0xff
            if ( ucMacroFlag )
            {
                ucMacroFlag = 0;
                pucSyncCmd ++;
                ucMacroRunFlag = 1;
                uiMacroNB = *pucSyncCmd;
            }
            else
            {
                ucMacroFlag = 1;
                ucMacroLen = 0;
                memset(aucMacro,0,sizeof(aucMacro));
            }
            break;

        case SYNCCMD_SETDATA://0x0f
            ucSetDataFlag = 1;
            pucSyncCmd ++;
            ucSetDataNB = *pucSyncCmd;
            ucSetDataLen = 0;
            memset(aucSetData,0,sizeof(aucSetData));
            break;

        default:
            if (icc_dbg)
                printk("%s deflaut *pucSyncCmd=%x\n", __func__, *pucSyncCmd);
            if ( ucSetDataFlag )
            {
                if ( ucSetDataLen < SYNCCMD_SETDATAMAXLEN )
                    aucSetData[ucSetDataLen++] = *pucSyncCmd;
                if ( ucSetDataLen >= ucSetDataNB )
                {
                    ucSetDataRunFlag = 1;
                    break;
                }
                else
                    continue;
            }

            if ( ucMacroFlag )
            {
                if ( ucMacroLen < SYNCCMD_MACROMAXLEN )
                    aucMacro[ucMacroLen++] = *pucSyncCmd;
                continue;
            }

            if ( *pucSyncCmd < SYNCCMD_MAXNB )
            {
                if (icc_dbg)
                printk("%s 0 funtion id %02x, ucReader=%x\n", __func__, *pucSyncCmd, ucReader);
                if ( SYNCCMDTable[*pucSyncCmd].pfnProc )
                {
//set_gpg11_loop(2);                      
                    ucResult = (*SYNCCMDTable[*pucSyncCmd].pfnProc)(ucReader);
//set_gpg11_loop(2);      					
                }
                else
                    ucResult = LIBICCERR_BADSYNCCMD;
            }
            else
            {
                ucResult = LIBICCERR_BADSYNCCMD;
            }
            break;

        }


        if ( !ucResult )
        {
            if ( ucMacroRunFlag )
            {
                if ( !uiMacroNB )
                    uiMacroNB = 256;
                for (uiI=0; uiI<uiMacroNB; uiI++)
                {
                    for (ucJ=0; ucJ<ucMacroLen; ucJ++)
                    {
                        if (icc_dbg)
                        printk("%s 1 funtion id: %02x\n", __func__, aucMacro[ucJ]);
                        if ( SYNCCMDTable[aucMacro[ucJ]].pfnProc )
                        {
//set_gpg11_loop(3);                      
                            ucResult = (*SYNCCMDTable[aucMacro[ucJ]].pfnProc)(ucReader);
//set_gpg11_loop(3);                      
                        }
                        if ( ucResult )
                            break;
                    }
                    if ( ucResult )
                        break;
                }
                ucMacroRunFlag = 0;
                uiMacroNB = 0;
            }
        }

        if ( !ucResult )
        {
            if ( ucSetDataRunFlag )
            {
                for (uiI=0; uiI<ucSetDataLen; uiI++)
                {

                }
                ucSetDataRunFlag = 0;
            }
        }

        if ( ucResult )
            break;
    }
    return(ucResult);
}

unsigned char SYNC_FinishedProc(unsigned char ucReader)
{
    unsigned char ucLen;
    unsigned char aucResult[LIBICC_MAXCOMMBUF];
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    memset(aucResult,0,sizeof(aucResult));
    ucLen = 1;
    if ( !pCurrICCInfo->ucLastError )
    {
        aucResult[ucLen] = LIBICCERR_EXECUTEOK;
    }
    else
    {
        aucResult[ucLen] = pCurrICCInfo->ucLastError;
    }
    ucLen ++;
    if ( pCurrICCInfo->uiSYNCRecvedLen <= LIBICC_MAXCOMMBUF-2 )
    {
        memcpy( &aucResult[ucLen],pCurrICCInfo->aucSYNCRecvBuf,
                pCurrICCInfo->uiSYNCRecvedLen);
        ucLen += pCurrICCInfo->uiSYNCRecvedLen;
    }
    aucResult[0] = ucLen-1;

#if 0
 //   printk("\n********** %s **********************\n", __func__ );
    for (i=0; i<pCurrICCInfo->uiSYNCRecvedLen; i++)  printk("");
       // printk("%02x ", pCurrICCInfo->aucSYNCRecvBuf[i]);
  //  printk("\n");
  //  printk("\n********** %s END**********************\n", __func__ );
#endif

    HALICC_DRVEnd(LIBICCERR_SUCCESS,aucResult,ucLen);

    return(pCurrICCInfo->ucLastError);
}

unsigned char SYNC_CMDEOC(unsigned char ucReader)
{
    HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_CLK,0);
    HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0);
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0));
}

unsigned char SYNC_CMDClrRST(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0));
}

unsigned char SYNC_CMDSetRST(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0));
}

unsigned char SYNC_CMDRST10us(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucValue;

    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_GET,HALICCCONTACT_RST,&ucValue);
    if ( !ucResult )
    {
        if ( ucValue )
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
        }
        else
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0);
        }
    }
    if ( !ucResult )
    {
        udelay(10);
        if ( ucValue )
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0);
        }
        else
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
        }
    }
    return(ucResult);
}

unsigned char SYNC_CMDClrCLK(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_CLK,0));
}

unsigned char SYNC_CMDSetCLK(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_CLK,0));
}
/* �ı�ʱ��״̬��������5us*/
unsigned char SYNC_CMDCLK5us(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucValue;

    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_GET,HALICCCONTACT_CLK,&ucValue);
    if ( !ucResult )
    {
        if ( ucValue )
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_CLK,0);
        }
        else
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_CLK,0);
        }
    }
    if ( !ucResult )
    {
        udelay(5);
        if ( ucValue )
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_CLK,0);
        }
        else
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_CLK,0);
        }
    }
    return(ucResult);
}



/* �ı�ʱ��״̬��������10us*/
unsigned char SYNC_CMDCLK10us(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucValue;

    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_GET,HALICCCONTACT_CLK,&ucValue);
    if ( !ucResult )
    {
        if ( ucValue )
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_CLK,0);
        }
        else
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_CLK,0);
        }
    }
    if ( !ucResult )
    {
        udelay(10);
        if ( ucValue )
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_CLK,0);
        }
        else
        {
            ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_CLK,0);
        }
    }
    return(ucResult);
}

unsigned char SYNC_CMDClrC4(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_C4,0));
}

unsigned char SYNC_CMDSetC4(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_C4,0));
}

unsigned char SYNC_CMDClrC8(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_C8,0));
}

unsigned char SYNC_CMDSetC8(unsigned char ucReader)
{
    return(HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_C8,0));
}

unsigned char SYNC_CMDVPPIdle(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDVPP5V(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDVPP12V(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDVPP15V(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDVPP21V(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDClrIO(unsigned char ucReader)
{

    return(HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_IO,0));
}

unsigned char SYNC_CMDSetIO(unsigned char ucReader)
{

//set_gpg11_loop(2);                      
//HALICC_SelectReader(ucReader);
//set_gpg11_loop(1);  

    return(HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_IO,0));
}

unsigned char SYNC_CMDMSBToIO(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDLSBToIO(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

/*

*/
unsigned char SYNC_CMDIOToMSB(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucValue;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_GET,HALICCCONTACT_IO,&ucValue);


    if ( !ucResult )
    {

        pCurrICCInfo->ucSYNCRecvedByte <<= 1;


        if ( ucValue )
            pCurrICCInfo->ucSYNCRecvedByte |= 0x01;

        pCurrICCInfo->ucSYNCRecvedBit ++;

        pCurrICCInfo->aucSYNCRecvBuf[pCurrICCInfo->uiSYNCRecvedLen] = pCurrICCInfo->ucSYNCRecvedByte;
        if ( pCurrICCInfo->ucSYNCRecvedBit >= 8)
        {
            pCurrICCInfo->ucSYNCRecvedBit = 0;
            pCurrICCInfo->ucSYNCRecvedByte = 0;
            pCurrICCInfo->uiSYNCRecvedLen ++;
        }
    }

    return(ucResult);
}

unsigned char SYNC_CMDIOToLSB(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucValue;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_GET,HALICCCONTACT_IO,&ucValue);

    if ( !ucResult )
    {

        pCurrICCInfo->ucSYNCRecvedByte >>= 1;

        if ( ucValue )
            pCurrICCInfo->ucSYNCRecvedByte |= 0x80;
        pCurrICCInfo->ucSYNCRecvedBit ++;
        pCurrICCInfo->aucSYNCRecvBuf[pCurrICCInfo->uiSYNCRecvedLen] = pCurrICCInfo->ucSYNCRecvedByte;
        if ( pCurrICCInfo->ucSYNCRecvedBit >= 8)
        {
            pCurrICCInfo->ucSYNCRecvedBit = 0;
            pCurrICCInfo->ucSYNCRecvedByte = 0;
            pCurrICCInfo->uiSYNCRecvedLen ++;
        }

//printk("%s pCurrICCInfo->ucSYNCRecvedByte=%x, ucValue=%x\n",
//	__func__, pCurrICCInfo->aucSYNCRecvBuf[pCurrICCInfo->uiSYNCRecvedLen], ucValue);
    }

    return(ucResult);
}

unsigned char SYNC_CMDTestIOHigh(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDTestIOLow(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDWait10us(unsigned char ucReader)
{
    udelay(10);
    return(LIBICCERR_SUCCESS);
}

unsigned char SYNC_CMDWait200us(unsigned char ucReader)
{
    udelay(200);
    return(LIBICCERR_SUCCESS);
}
