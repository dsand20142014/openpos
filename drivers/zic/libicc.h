/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  halicc.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-02-22		Xiaoxi Jiang
    Last modified :	2003-02-22		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __LIBICC_H__
#define __LIBICC_H__

#ifndef LIBICCDATA
#define LIBICCDATA extern
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include "halicc.h"

#define LIBICC_DEBUG


#define LIBICC_CODENOTIMPLEMENT         printk("%s %d:CODE_NOT_IMPLEMENT!\n",__FILE__,__LINE__)


#define LIBICCERR_SUCCESS               0x00
#define LIBICCERR_NOICC                 0x01
#define LIBICCERR_NOREADER              0x02
#define LIBICCERR_OVERRECVBUF           0x03
#define LIBICCERR_BADID                 0x04
#define LIBICCERR_CMDNOTIMPLEMENT       0x0B
#define LIBICCERR_BADCMDOUTPARAM        0x0C

#define LIBICCERR_BADTS					0x11
#define LIBICCERR_ATRTOOLONG            0x12
#define LIBICCERR_CARDFAULT             0x15
#define LIBICCERR_BADCMD                0x16
#define LIBICCERR_BADCMDINPARAM         0x17
#define LIBICCERR_UNKNOWNPROTOCOL       0x18
#define LIBICCERR_PROTOCOLPARAM         0x19
#define LIBICCERR_APDUSENDTIMEOUT       0x1A
#define LIBICCERR_APDURECVPROCTIMEOUT   0x1B
#define LIBICCERR_APDURECVTIMEOUT1      0x1C
#define LIBICCERR_APDURECVTIMEOUT2      0x1D
#define LIBICCERR_APDUEXCHUNKNOWN       0x1E
#define LIBICCERR_UNKNOWNSTATUS         0x1F
#define LIBICCERR_NOTPOWERED            0x20
#define LIBICCERR_ATRPARITYERROR		0x21
#define LIBICCERR_APDUPARITYERROR		0x22
#define LIBICCERR_APDUTIMEOUT			0x23
#define LIBICCERR_BADTA1				0x24
#define LIBICCERR_BADTB1                0x25
#define LIBICCERR_BADTD1                0x26
#define LIBICCERR_BADTA2                0x27
#define LIBICCERR_BADTB2                0x28
#define LIBICCERR_BADTC2				0x29
#define LIBICCERR_BADTD2                0x2A
#define LIBICCERR_BADTA3                0x2B
#define LIBICCERR_BADTB3				0x2C
#define LIBICCERR_BADTC3                0x2D
#define LIBICCERR_BADTCK                0x2E
#define LIBICCERR_BADPROCBYTE			0x2F

#define LIBICCERR_ISASYNCCARD           0x35
#define LIBICCERR_EXECUTEOK             0x30
#define LIBICCERR_CARDREMOVE            0x31
#define LIBICCERR_VPPPROBLEM            0x32
#define LIBICCERR_VCCPROBLEM            0x33
#define LIBICCERR_ISSYNCCARD			0x37
#define LIBICCERR_CARDMUTE              0x38
#define LIBICCERR_NEEDWARMRESET         0x39

#define LIBICCERR_BADSYNCCMD			0x40
#define LIBICCERR_SYNCEOC				0x41
#define LIBICCERR_SYNCSETDATA			0x42
#define LIBICCERR_SYNCMACRO				0x43

#define LIBICCERR_T1BADEDC				0x50
#define LIBICCERR_T1BADPCB				0x51
#define LIBICCERR_T1BADRECVSEQ			0x52
#define LIBICCERR_T1UNKNOWNERR			0x53
#define LIBICCERR_T1BADNAD              0x54
#define LIBICCERR_T1SABORTREQUEST       0x55
#define LIBICCERR_T1BADIFRAMELEN        0x56
#define LIBICCERR_T1BADRFRAMELEN        0x57
#define LIBICCERR_T1BADSFRAMELEN        0x58
#define LIBICCERR_T1BADPROTOCOL         0x59
#define LIBICCERR_T1INVALIDSEQ          0x5A
#define LIBICCERR_T1RFRAMEERR           0x5B
#define LIBICCERR_T1SFRAMEERR           0x5C
#define LIBICCERR_T1ILINKERR            0x5D
#define LIBICCERR_T1RFRAMEBADEDC        0x5E
#define LIBICCERR_T1INVALIDBLOCK        0x5F

#define LIBICCERR_BADT0_20   			0x60
#define LIBICCERR_BADT0_40   			0x61

#define LIBICCERR_T1IFRAMERESEND        0xFA
#define LIBICCERR_APDUNONEXTCOMMAND     0xFB
#define LIBICCERR_ATRHASLASTCHAR		0xFC
#define LIBICCERR_ATRNOTFINISHED        0xFD
#define LIBICCERR_PROCESSEND            0xFE
#define LIBICCERR_PROCESSPENDING        0xFF

// execution status from ICC and SIM driver (drv_status)
    /*
    #define OK              0
    #define KO              1
    #define NOICC           1
    #define NOREADER        2
    #define OVERRUN         6
    #define DRVUNKNOWN      7
    #define WRONGREADER     10
    #define WRONGORDER      11
    #define NULLPTOUT       12
    */
#define ICCMETHOD_ASYNC					0x00
#define ICCMETHOD_SYNC					0x01

#define ICCSTANDARD_EMV                 0x00
#define ICCSTANDARD_ISO7816             0x01
#define ICCSTANDARD_MPEV5               0x02
#define ICCSTANDARD_MONDEX              0x03
#define ICCSTANDARD_GJISAM				0x04	//����ISAM�淶��
#define ICCSTANDARD_GJPSAM				0x05	//����PSAM�淶��
#define ICCSTANDARD_SHEBAO				0x06	//�籣�淶��
#define ICCSTANDARD_QINGD               0x07
#define ICCSTANDARD_NEW                 0x08

#define ICCPROTOCOL_T0                  0x00
#define ICCPROTOCOL_T1                  0x01

#define ICCAPDU_CASE0                   0x00
#define ICCAPDU_CASE1                   0x01
#define ICCAPDU_CASE2                   0x02
#define ICCAPDU_CASE3                   0x03
#define ICCAPDU_CASE4                   0x04
#define ICCAPDU_CASE5                   0x05
#define ICCAPDU_CASE6                   0x06
#define ICCAPDU_CASE7                   0x07


#define ICCSTATUS_INSERT                0x01
#define ICCSTATUS_REMOVE                0x02
#define ICCSTATUS_POWERED               0x03
#define ICCSTATUS_ASYNCCARD             0x35

#define ICCSESSION_IDLE                 0x00
#define ICCSESSION_COLDRESET            0x01
#define ICCSESSION_WARMRESET            0x02
#define ICCSESSION_DATAEXCH             0x03
    /*
    #define ICCSESSION_IFSEXCH				0x04
    */

#define ICCSESSIONSTEP_IDLE             0x00

#define ICCSESSIONSTEP_COLDRESET1       0x01
#define ICCSESSIONSTEP_COLDRESET2       0x02
#define ICCSESSIONSTEP_COLDRESET3       0x03

#define ICCSESSIONSTEP_DATAEXCHSEND1    0x01
#define ICCSESSIONSTEP_DATAEXCHSEND2    0x02
#define ICCSESSIONSTEP_DATAEXCHSEND3    0x03
#define ICCSESSIONSTEP_DATAEXCHSEND4    0x04

#define ICCSESSIONSTEP_DATAEXCHRECV1    0x08
#define ICCSESSIONSTEP_DATAEXCHRECV2    0x09
#define ICCSESSIONSTEP_DATAEXCHRECV3    0x0A
#define ICCSESSIONSTEP_DATAEXCHRECV4	0x0B

    /*
    #define ICCSESSIONSTEP_IFSEXCHSEND1		0x01
    #define ICCSESSIONSTEP_IFSEXCHSEND2		0x02

    #define ICCSESSIONSTEP_IFSEXCHRECV		0x08
    */

#define ICCCMD_MAXNB                    0x15
#define LIBICC_MAXATRBUF                512
#define LIBICC_MAXCOMMBUF	        512

#define LIBICC_COLDRESET                0x00
#define LIBICC_WARMRESET                0x01

    typedef struct
    {
        volatile unsigned char ucStatus;
        volatile unsigned char ucSession;
        volatile unsigned char ucSessionStep;
        volatile unsigned char ucLastError;
        volatile unsigned char ucStandard;
        volatile unsigned char ucPowerType;
        volatile unsigned char ucResetType;
        volatile unsigned char ucAutoPower;

        volatile unsigned char ucATR_Finished;//ATR ��λOK? 1 OK 0 No

        volatile unsigned char ucATRLen;
        volatile unsigned char ucProcWord;
        unsigned char aucATR[LIBICC_MAXATRBUF];
        volatile unsigned char ucProtocol;
        volatile unsigned char ucAPDUCase;
        volatile unsigned short uiRecvLen;
        volatile unsigned short uiRecvedLen;
        unsigned char aucRecvBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned char ucAPDUOneByteFlag;
        volatile unsigned short uiAPDUSendLen;
        unsigned char aucAPDUSendBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiAPDUSentLen;
        volatile unsigned short uiAPDURecvLen;
        unsigned char aucAPDURecvBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiAPDURecvedLen;
        volatile unsigned short uiT1FrameSendLen;
        unsigned char aucT1FrameSendBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiT1FrameSentLen;
        volatile unsigned short uiT1FrameLastSendLen;
        unsigned char aucT1FrameLastSendBuf[LIBICC_MAXCOMMBUF];
        unsigned char aucSYNCSendBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiSYNCSendLen;
        unsigned char ucSYNCSendByte;
        unsigned char ucSYNCSentBit;
        unsigned char aucSYNCRecvBuf[LIBICC_MAXCOMMBUF];
        volatile unsigned short uiSYNCRecvedLen;
        unsigned char ucSYNCRecvedByte;
        unsigned char ucSYNCRecvedBit;
        unsigned char ucFi;
        unsigned char ucDi;
        unsigned char ucGTR;
        unsigned long ulWWT;
        unsigned short uiCWT;
        unsigned char ucWTX;
        unsigned long ulBWT;
        unsigned char ucT1Status;
        unsigned char ucT1ErrorCount;
        unsigned char ucT1IFSReqFlag;
        unsigned char ucT1IFSC;
        unsigned char ucT1NAD;
        unsigned char ucT1SendSeq;
        unsigned char ucT1RecvSeq;
        unsigned char ucT1EDCMode;
    }ICCINFO;

    typedef struct
    {
        unsigned char (*pfnProc)(unsigned char *pucInData,
                                 unsigned short uiInLen,
                                 unsigned char *pucOutData);
    }ICCCMD;

    typedef struct
    {
        unsigned char (*pfnProc)(unsigned char ucReader);
    }SYNCCMD;

    LIBICCDATA ICCINFO ICCInfo[HALICC_READERNB];

    /* libicc.c */
    void LIBICC_Init(void);
    unsigned char LIBICC_ProcessATR(unsigned char ucReader);
    void LIBICC_CallbackCardInsert(unsigned char ucReader);
    void LIBICC_CallbackCardRemove(unsigned char ucReader);
    void LIBICC_CallbackSendChar(unsigned char ucReader);
    void LIBICC_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh);
    void LIBICC_CallbackFault(unsigned char ucReader);
    void LIBICC_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void LIBICC_CallbackParityError(unsigned char ucReader);
    void LIBICC_CallbackOverrunError(unsigned char ucReader);
    void LIBICC_CallbackFrameError(unsigned char ucReader);

    /* icccmd.c */
    unsigned char ICCCMD_Process(unsigned char ucCMD,
                                 unsigned char *pucInData,
                                 unsigned short uiInLen,
                                 unsigned char *pucOutData);
    unsigned char ICCCMD_Detect(unsigned char *pucInData,
                                unsigned short uiInLen,
                                unsigned char *pucOutData);
    unsigned char ICCCMD_Select(unsigned char *pucInData,
                                unsigned short uiInLen,
                                unsigned char *pucOutData);
    unsigned char ICCCMD_ChangeProtocol(unsigned char *pucInData,
                                        unsigned short uiInLen,
                                        unsigned char *pucOutData);
    unsigned char ICCCMD_Insert(unsigned char *pucInData,
                                unsigned short uiInLen,
                                unsigned char *pucOutData);
    unsigned char ICCCMD_SetStandard(unsigned char *pucInData,
                                     unsigned short uiInLen,
                                     unsigned char *pucOutData);
    unsigned char ICCCMD_PowerOff(unsigned char *pucInData,
                                  unsigned short uiInLen,
                                  unsigned char *pucOutData);
    unsigned char ICCCMD_Remove(unsigned char *pucInData,
                                unsigned short uiInLen,
                                unsigned char *pucOutData);
    unsigned char ICCCMD_AsyncCommand(unsigned char *pucInData,
                                      unsigned short uiInLen,
                                      unsigned char *pucOutData);
    unsigned char ICCCMD_NewSyncCommand(unsigned char *pucInData,
                                        unsigned short uiInLen,
                                        unsigned char *pucOutData);
    unsigned char ICCCMD_WarmReset(unsigned char *pucInData,
                                   unsigned short uiInLen,
                                   unsigned char *pucOutData);
    unsigned char ICCCMD_InsertCard(unsigned char *pucInData,
                                    unsigned short uiInLen,
                                    unsigned char *pucOutData);

    /* iso7816.c */
    unsigned char ISO7816_AsyncReset(unsigned char ucReader);
    unsigned char ISO7816_WarmReset(unsigned char ucReader);
    unsigned char ISO7816_SetAPDUNextCommand(unsigned char ucReader);
    unsigned char ISO7816_CheckICCParam(unsigned char ucReader);
    void ISO7816_RecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void ISO7816_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void ISO7816_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void ISO7816_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    void ISO7816_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);

    /* emv.c */
    unsigned char EMV_AsyncReset(unsigned char ucReader);
    unsigned char EMV_WarmReset(unsigned char ucReader);
    unsigned char EMV_SetAPDUNextCommand(unsigned char ucReader);
    unsigned char EMV_CheckICCParam(unsigned char ucReader);
    void EMV_RecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void EMV_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void EMV_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void EMV_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    void EMV_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);

    /* gjisam.c */
    unsigned char GJISAM_AsyncReset(unsigned char ucReader);
    unsigned char GJISAM_WarmReset(unsigned char ucReader);
    unsigned char GJISAM_SetAPDUNextCommand(unsigned char ucReader);
    unsigned char GJISAM_CheckICCParam(unsigned char ucReader);
    void GJISAM_RecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void GJISAM_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void GJISAM_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void GJISAM_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    void GJISAM_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);

    /* gjpsam.c */
    unsigned char GJPSAM_AsyncReset(unsigned char ucReader);
    unsigned char GJPSAM_WarmReset(unsigned char ucReader);
    unsigned char GJPSAM_SetAPDUNextCommand(unsigned char ucReader);
    unsigned char GJPSAM_CheckICCParam(unsigned char ucReader);
    void GJPSAM_RecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void GJPSAM_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void GJPSAM_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void GJPSAM_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    void GJPSAM_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    unsigned char GJPSAM_T0SendReceive(unsigned char ucReader);

    /* shebao.c */
    unsigned char SHEBAO_AsyncReset(unsigned char ucReader);
    unsigned char SHEBAO_WarmReset(unsigned char ucReader);
    unsigned char SHEBAO_SetAPDUNextCommand(unsigned char ucReader);
    unsigned char SHEBAO_CheckICCParam(unsigned char ucReader);
    void SHEBAO_RecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void SHEBAO_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void SHEBAO_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
    void SHEBAO_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    void SHEBAO_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
    unsigned char SHEBAO_T0SendReceive(unsigned char ucReader);


    /* atr.c */
    unsigned char ATR_PowerOn(unsigned char ucReader);
    unsigned char ATR_WarmReset(unsigned char ucReader);
    unsigned char ATR_CheckFinished(unsigned char *pucATR,unsigned short uiATRLen);
    unsigned char ATR_Process(unsigned char ucReader);
    unsigned char ATR_FinishedProc(unsigned char ucReader);
    void ATR_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh);
    void ATR_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void ATR_CallbackCardRemove(unsigned char ucReader);
    void ATR_CallbackFault(unsigned char ucReader);
    void ATR_CallbackParityError(unsigned char ucReader);

    /* protocol.c */
    unsigned char PROT_AsyncCommand(unsigned char ucReader,
                                    unsigned char *pucInData,
                                    unsigned short uiInLen);
    unsigned char PROT_T0Command(unsigned char ucReader);
    unsigned char PROT_T0SendReceive(unsigned char ucReader);
    unsigned char PROT_T0RecvChar(unsigned char ucReader,unsigned char *pucCh);
    unsigned char PROT_T1Command(unsigned char ucReader);
    unsigned char PROT_T1SendReceiveFrame(unsigned char ucReader,unsigned char ucFrameType,unsigned char ucLastFrameType);
    unsigned char PROT_T1ReceiveFrame(unsigned char ucReader);
    unsigned char PROT_T0FinishedProc(unsigned char ucReader);
    unsigned char PROT_T1FinishedProc(unsigned char ucReader);
    unsigned char PROT_T1ComputeEDC(unsigned char ucMode,unsigned char *pucInData,unsigned short uiInLen);
    unsigned char PROT_T1PackIFrame(unsigned char ucReader);
    unsigned char PROT_T1PackRFrame(unsigned char ucReader,unsigned char ucSEQ,unsigned char ucError);
    unsigned char PROT_T1PackSFrameIFSReq(unsigned char ucReader);
    unsigned char PROT_T1PackSFrameSyncReply(unsigned char ucReader);
    unsigned char PROT_T1PackSFrameIFSReply(unsigned char ucReader);
    unsigned char PROT_T1PackSFrameBWTReply(unsigned char ucReader);
    unsigned char PROT_T1PackFrame(unsigned char ucReader,unsigned char ucPCB,
                                   unsigned char *pucInData,
                                   unsigned char ucInLen);
    void PROT_CallbackSendChar(unsigned char ucReader);
    void PROT_CallbackT0SendChar(unsigned char ucReader);
    void PROT_CallbackT1SendChar(unsigned char ucReader);
    void PROT_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh);
    void PROT_CallbackT0RecvChar(unsigned char ucReader,unsigned char ucCh);
    void PROT_CallbackT1RecvChar(unsigned char ucReader,unsigned char ucCh);
    void PROT_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void PROT_CallbackT0Timeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void PROT_CallbackT1Timeout(unsigned char ucReader,unsigned char ucTimeoutType);
    void PROT_CallbackCardRemove(unsigned char ucReader);
    void PROT_CallbackT0CardRemove(unsigned char ucReader);
    void PROT_CallbackT1CardRemove(unsigned char ucReader);
    void PROT_CallbackFault(unsigned char ucReader);
    void PROT_CallbackParityError(unsigned char ucReader);
    void PROT_CallbackT0ParityError(unsigned char ucReader);
    void PROT_CallbackT1ParityError(unsigned char ucReader);

    /* sync.c */
    unsigned char SYNC_RunCommand(unsigned char ucReader,unsigned char *pucInData,unsigned short uiInLen);
    unsigned char SYNC_AnalyseCommand(unsigned char *pucInData,unsigned short uiInLen);
    unsigned char SYNC_ProcessCommand(unsigned char ucReader,unsigned char *pucInData,unsigned short uiInLen);
    unsigned char SYNC_FinishedProc(unsigned char ucReader);
    unsigned char SYNC_CMDEOC(unsigned char ucReader);
    unsigned char SYNC_CMDClrRST(unsigned char ucReader);
    unsigned char SYNC_CMDSetRST(unsigned char ucReader);
    unsigned char SYNC_CMDRST10us(unsigned char ucReader);
    unsigned char SYNC_CMDClrCLK(unsigned char ucReader);
    unsigned char SYNC_CMDSetCLK(unsigned char ucReader);
    unsigned char SYNC_CMDCLK5us(unsigned char ucReader);
    unsigned char SYNC_CMDCLK10us(unsigned char ucReader);
    unsigned char SYNC_CMDClrC4(unsigned char ucReader);
    unsigned char SYNC_CMDSetC4(unsigned char ucReader);
    unsigned char SYNC_CMDClrC8(unsigned char ucReader);
    unsigned char SYNC_CMDSetC8(unsigned char ucReader);
    unsigned char SYNC_CMDVPPIdle(unsigned char ucReader);
    unsigned char SYNC_CMDVPP5V(unsigned char ucReader);
    unsigned char SYNC_CMDVPP12V(unsigned char ucReader);
    unsigned char SYNC_CMDVPP15V(unsigned char ucReader);
    unsigned char SYNC_CMDVPP21V(unsigned char ucReader);
    unsigned char SYNC_CMDClrIO(unsigned char ucReader);
    unsigned char SYNC_CMDSetIO(unsigned char ucReader);
    unsigned char SYNC_CMDMSBToIO(unsigned char ucReader);
    unsigned char SYNC_CMDLSBToIO(unsigned char ucReader);
    unsigned char SYNC_CMDIOToMSB(unsigned char ucReader);
    unsigned char SYNC_CMDIOToLSB(unsigned char ucReader);
    unsigned char SYNC_CMDTestIOHigh(unsigned char ucReader);
    unsigned char SYNC_CMDTestIOLow(unsigned char ucReader);
    unsigned char SYNC_CMDWait10us(unsigned char ucReader);
    unsigned char SYNC_CMDWait200us(unsigned char ucReader);

#ifdef __cplusplus
}
#endif

#endif
