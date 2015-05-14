/*
	SAND China
--------------------------------------------------------------------------
	FILE  global.h								(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Include file for the application.

    List of routines in file :

    File history :

*/

#ifndef DATAGLO
#define DATAGLO extern
#endif

#ifdef  TURBOC
extern DRV_IN		drvin_s;
extern DRV_OUT		drvout_s;
#define DrvIn		drvin_s
#define DrvOut		drvout_s
#else
DATAGLO DRV_IN  DrvIn;
DATAGLO DRV_OUT	DrvOut;
#endif
DATAGLO DRIVER  Drv;
DATAGLO ISO7816IN	Iso7816In;
DATAGLO ISO7816OUT	Iso7816Out;
DATAGLO	CARDAPPFILE	CardAppFile;
DATAGLO	SLE4442DATA	SleTransData;
DATAGLO	SLE4442DATASMT	SleTransDataSmt;
DATAGLO	SLE4442DATASP	SleTransDataSp;
DATAGLO	M1DATA18	M1TransData;
DATAGLO	M1DATA19	M1TransData1;
DATAGLO NORMALTRANS	NormalTransData;
DATAGLO 		PAYMENT_TRANS 		PaymentTrans;
DATAGLO	CDBBillInfo oBillInfo;
DATAGLO ISSUETRANSINFO IssueInfo;

DATAGLO RUNDATA	RunData;
DATAGLO EXTRATRANS 	ExtraTransData;
DATAGLO	ISO8583	ISO8583Data;
DATAGLO DIAL_PARAM	DialParam;
DATAGLO MIFARECOMMANDDATA 			MifareCommandData;
//DATAGLO UNTOUCHISSUE IssueData;

DATAGLO COM1_INFO		Com1_Info;
DATAGLO INCEPTCASHTRANSDATA	InceptTransData;
DATAGLO DEMANDDATA	InceptQueryData;
DATAGLO HEADDATA HeadData;
DATAGLO RECEIVETRANS  ReceiveTransData;
DATAGLO RECEIVETRANS_GD ReceiveTransData_Gd;
DATAGLO SENDTRANS     SendTransData;
DATAGLO PORTDATA      PortData;
DATAGLO EXPDATA      ExpTransData;

DATAGLO  unsigned char  PORTAUCBUF[512];
DATAGLO  unsigned char  PORTAUCBUF1[512];
DATAGLO  unsigned char  PORTAUCBUF2[512];

//--
DATAGLO PROUIINTERFACE ProUiFace;


#define NORMALTRANS_aucDateTime               NormalTransData.aucDateTime
#define NORMALTRANS_ucTransType    			NormalTransData.ucTransType
#define NORMALTRANS_ucExpiredDate  			NormalTransData.ucExpiredDate
#define NORMALTRANS_ulAmount       			NormalTransData.ulAmount
#define NORMALTRANS_ulPrevEDBalance			NormalTransData.ulPrevEDBalance
#define NORMALTRANS_ulTraceNumber  			NormalTransData.ulTraceNumber

#define EXTRATRANS_ucISO2Status				ExtraTransData.ucISO2Status
#define EXTRATRANS_uiISO2Len                  ExtraTransData.uiISO2Len
#define EXTRATRANS_aucISO2                    ExtraTransData.aucISO2
#define EXTRATRANS_ucISO3Status               ExtraTransData.ucISO3Status
#define EXTRATRANS_uiISO3Len                  ExtraTransData.uiISO3Len
#define EXTRATRANS_aucISO3                    ExtraTransData.aucISO3

#define MIFARE_ucOutStatus				MifareCommandData.aucCommandOut[0]

#define RUNDATA_ucErrorExtCode				RunData.ucErrorExtCode
#define RUNDATA_ucTerminalType				RunData.ucTerminalType
#define RUNDATA_ucDialRetry               	RunData.ucDialRetry
#define RUNDATA_ucHostConnectFlag				RunData.ucHostConnectFlag
#define RUNDATA_uiTransIndex					RunData.uiTransIndex
#define RUNDATA_uiOldTransIndex				RunData.uiOldTransIndex
#define RUNDATA_ulOldTraceNumber				RunData.ulOldTraceNumber
#define RUNDATA_ulOldFeeAmount				RunData.ulOldFeeAmount
#define RUNDATA_ucPINRetry					RunData.ucPINRetry
#define RUNDATA_aucTerminalID					RunData.aucTerminalID
#define RUNDATA_aucMerchantID					RunData.aucMerchantID
#define RUNDATA_ucReprintFlag					RunData.ucReprintFlag
#define RUNDATA_ucSettleStatus				RunData.ucSettleStatus
//#define RUNDATA_aucDate            RunData.aucDate
//#define RUNDATA_aucTime            RunData.aucTime

#define IPAddress							DataSave0.ConstantParamData.ulHostIPAdd
#define HostPort							DataSave0.ConstantParamData.uiHostPort

#define FONT									DataSave0.ConstantParamData.font
#define LINENUM								DataSave0.ConstantParamData.linenum
#define PRINTTIMES							DataSave0.ConstantParamData.printtimes
#define Bank_flag								DataSave0.ConstantParamData.Bank_flag
#define Printer_flag							DataSave0.ConstantParamData.Printer_flag
#define Pinpad_flag							DataSave0.ConstantParamData.Pinpad_flag
#define terminfoFirst							DataSave0.ConstantParamData.terminfoFirst
#define Commun_flag							DataSave0.ConstantParamData.ucCommunication	
