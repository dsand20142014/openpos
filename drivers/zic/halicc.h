/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  halicc.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-01-18		Xiaoxi Jiang
    Last modified :	2003-01-18		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __HALICC_H__
#define __HALICC_H__

#ifndef HALICCDATA
#define HALICCDATA extern
#endif

#include "haldrv.h"
#ifdef __cplusplus
extern "C" {
#endif

#define HALICCERR_SUCCESS           0x00
#define HALICCERR_CHIPNOTEXIST      0x03
#define HALICCERR_INVALIDREADER     0x04
#define HALICCERR_NOTPOWER          0x07
#define HALICCERR_TOOLONGSENDDATA   0x08
#define HALICCERR_INVALIDCONTACT    0x0A
#define HALICCERR_CHIP1NOTEXIST     0x0B
#define HALICCERR_CHIP2NOTEXIST     0x0C
#define HALICCERR_RECVNODATA		0x0D

#define HALICCTYPE_TDA8002          0x01
#define HALICCTYPE_TDA8007          0x02


#define HALICCCONTACT_GET       0x00
#define HALICCCONTACT_CLR       0x01
#define HALICCCONTACT_SET       0x02

#define HALICCCONTACT_C1        0x01
#define HALICCCONTACT_C2        0x02
#define HALICCCONTACT_C3        0x03
#define HALICCCONTACT_C4        0x04
#define HALICCCONTACT_C5        0x05
#define HALICCCONTACT_C6        0x06
#define HALICCCONTACT_C7        0x07
#define HALICCCONTACT_C8        0x08
#define HALICCCONTACT_VCC       0x01
#define HALICCCONTACT_RST       0x02
#define HALICCCONTACT_CLK       0x03
#define HALICCCONTACT_IO        0x07

#define ICCTIMERTYPE_STOP       0x00
#define ICCTIMERTYPE_RSTKEEP    0x01
#define ICCTIMERTYPE_ISORST1    0x02
#define ICCTIMERTYPE_ISORST2    0x03

#define HALICC_READERNB         4
#define HALICC_READER0          0x00
#define HALICC_READER1          0x01
#define HALICC_READER2          0x02
#define HALICC_READER3          0x03

#define TDA8007B_C2             0x02
#define TDA8007B_C3             0x03
#define TDA8007BASEADDR1        0x4200000
#define TDA8007BASEADDR2        0x4300000


   enum ic_regs
    {
//Card Select Register
        REG_CSR=0,
        REG_CCR,//Clock Configuration Register1
        REG_PDR,//Programmable Divider Register
        //UART Configuration Register2
        REG_UCR2=3,

        //Guard Time Register
        REG_GTR=5,

        //UART Configuration Register1
        REG_UCR1,

        //Power Control Register
        REG_PCR,

        //Timeout Counter Register1
        REG_TOC,

        REG_TOR1,//9
        REG_TOR2,//10
        REG_TOR3,//11

        REG_MSR=12,//12
        REG_FCR=12,//12

        REG_URR=13,//13
        REG_UTR=13,//13

        REG_USR=14,
        REG_HSR=15//0x0f, 15
    };

#define HALICC_POWER5V      0x00
#define HALICC_POWER3V      0x01
#define HALICC_POWER18V     0x02

#define HALICCSTEP_IDLE         0x00
#define HALICCSTEP_POWERED      0x04
#define HALICCSTEP_APDUEXCHSEND 0x05
#define HALICCSTEP_APDUEXCHRECV 0x06

#define HALICCAPDUSTEP_T0SEND   0x01

typedef struct
{
	unsigned char ucIICAddr;
	unsigned char ucIICProcFlag;
}ICCDRVCFG;

typedef struct
{
    void    (*pfnEvent)(unsigned char ucChipID);
}TDA8007EVENT;

unsigned char HALICC_Install(void);
unsigned char HALICC_Uninstall(void);
unsigned char HALICC_Init(void);
unsigned char HALICC_Test(void);
unsigned char HALICC_Ioctl(unsigned char ucType,unsigned char *pucData);
unsigned char HALICC_State(DRVIN *pDrvIn,DRVOUT *pDrvOut);
unsigned char HALICC_Run(DRVIN *pDrvIn,DRVOUT *pDrvOut);
unsigned char HALICC_Abort(DRVIN *pDrvIn,DRVOUT *pDrvOut);
unsigned char HALICC_Reset(DRVIN *pDrvIn,DRVOUT *pDrvOut);
unsigned char HALICC_IICProc(DRVIN *pDrvIn,DRVOUT *pDrvOut);
unsigned char HALICC_RecvHook(unsigned char ucDevID,unsigned char *pucRecvData,unsigned int uiRecvLen);

unsigned char HALICC_Enable(void);
void HALICC_Disable(void);
unsigned char HALICC_DRVExecute(unsigned char ucCMD,
        unsigned char *pucInData,
        unsigned char ucInLen);
unsigned char HALICC_DRVAbort(void);
unsigned char HALICC_DRVEnd(unsigned char ucDrvStatus,
        unsigned char *pucInData,
        unsigned short uiInLen);
unsigned char HALICC_SelectReader(unsigned char ucReader);
unsigned char HALICC_SetMethod(unsigned char ucReader,
		unsigned char ucMethod);
unsigned char HALICC_GetMethod(unsigned char ucReader,
		unsigned char *pucMethod);
unsigned char HALICC_SetClock(unsigned char ucReader,
		unsigned char ucStopFlag,
		unsigned char ucLevel);
unsigned char HALICC_Detect(unsigned char ucReader,
        unsigned char *pucPresent);
unsigned char HALICC_HandleContact(unsigned char ucReader,
        unsigned char ucType,
        unsigned char ucID,
        unsigned char *pucValue);
unsigned char HALICC_ResetUart(unsigned char ucReader);
unsigned char HALICC_Active(unsigned char ucReader);
unsigned char HALICC_Deactive(unsigned char ucReader);
unsigned char HALICC_SetPowerType(unsigned char ucReader,
        unsigned char ucPowerType);
unsigned char HALICC_SetBaud(unsigned char ucReader,
        unsigned char ucBaudMulti);
unsigned char HALICC_SetDi(unsigned char ucReader,
        unsigned char ucDi);
unsigned char HALICC_SetProtocol(unsigned char ucReader,
        unsigned char ucProtocol);
unsigned char HALICC_SetConvention(unsigned char ucReader,
        unsigned char ucConvention);
unsigned char HALICC_SetPEC(unsigned char ucReader,
        unsigned char ucPEC);
unsigned char HALICC_EnableSend(unsigned char ucReader);
unsigned char HALICC_EnableRecv(unsigned char ucReader);
unsigned char HALICC_EnableRecvAfterSend(unsigned char ucReader);
unsigned char HALICC_SendChar(unsigned char ucReader,
        unsigned char ucCh);
unsigned char HALICC_RecvChar(unsigned char ucReader,
        unsigned char *pucCh);
unsigned char HALICC_SetTimer(unsigned char ucReader,
        unsigned char ucTimerType,
        unsigned char ucTimerValue1,
        unsigned char ucTimerValue2,
        unsigned char ucTimerValue3,
        unsigned char ucCheckFlag);
unsigned char HALICC_CheckBGT(unsigned char ucReader);
unsigned char HALICC_SetGTR(unsigned char ucReader,
        unsigned char ucGTR);
unsigned char HALICC_ReadTDA8007REG(unsigned char *pucBaseAddr,
        unsigned char ucREG,
        unsigned char *pucCh,
        unsigned char ucCheckFlag);
unsigned char HALICC_WriteTDA8007REG(unsigned char *pucBaseAddr,
        unsigned char ucREG,
        unsigned char ucValue,
        unsigned char ucCheckFlag);
void HALICC_TDA8007EnableWrite(void);
void HALICC_TDA8007DisableWrite(void);
unsigned char HALICC_TDA8007ReadREG(unsigned char ucReader,
        unsigned char ucREG,
        unsigned char *pucCh,
        unsigned char ucCheckFlag);
unsigned char HALICC_TDA8007WriteREG(unsigned char ucReader,
        unsigned char ucREG,
        unsigned char ucValue,
        unsigned char ucCheckFlag);
void HSREvent_PTLProc(unsigned char ucChipID);
void HSREvent_INTAUXLProc(unsigned char ucChipID);
void HSREvent_PRL1Proc(unsigned char ucChipID);
void HSREvent_PRL2Proc(unsigned char ucChipID);
void HSREvent_SUPLProc(unsigned char ucChipID);
void HSREvent_PRTL1Proc(unsigned char ucChipID);
void HSREvent_PRTL2Proc(unsigned char ucChipID);
void USREvent_TBERBFProc(unsigned char ucChipID);
void USREvent_FERProc(unsigned char ucChipID);
void USREvent_OVRProc(unsigned char ucChipID);
void USREvent_PEProc(unsigned char ucChipID);
void USREvent_EAProc(unsigned char ucChipID);
void USREvent_TO1Proc(unsigned char ucChipID);
void USREvent_TO2Proc(unsigned char ucChipID);
void USREvent_TO3Proc(unsigned char ucChipID);
void HALICC_Chip1IntProc(void);
void HALICC_Chip2IntProc(void);

HALICCDATA unsigned char ucLastREGHSR;
HALICCDATA unsigned char ucLastREGUSR;		/*fante 新修改*/
HALICCDATA unsigned char ucChipSELECT;		/*fante 新修改*/


HALICCDATA unsigned char (*pfnHALICCCommand)(unsigned char ucCMD,
        unsigned char *pucInData,
        unsigned short ucInLen,
        unsigned char *pucOutData);
HALICCDATA void (*pfnCallbackCardInsert)(unsigned char ucReader);
HALICCDATA void (*pfnCallbackCardRemove)(unsigned char ucReader);
HALICCDATA void (*pfnCallbackSendChar)(unsigned char ucReader);
HALICCDATA void (*pfnCallbackRecvChar)(unsigned char ucReader,unsigned char ucCh);
HALICCDATA void (*pfnCallbackFault)(unsigned char ucReader);
HALICCDATA void (*pfnCallbackTimeout)(unsigned char ucReader,unsigned char ucTimeoutType);
HALICCDATA void (*pfnCallbackParityError)(unsigned char ucReader);
HALICCDATA void (*pfnCallbackOverrunError)(unsigned char ucReader);
HALICCDATA void (*pfnCallbackFrameError)(unsigned char ucReader);

#ifdef __cplusplus
}
#endif

#endif
