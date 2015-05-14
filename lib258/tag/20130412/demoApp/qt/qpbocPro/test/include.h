#ifndef		_INCLUDE_H
#define		_INCLUDE_H

#include	<QTypeDef.h>
#include	<oslib.h>
#include	<QDef.h>

#define		CONSTPARAMFILE			"ConstPara"
#define		EMVCONFIGFILE			"QConfig"
#define		CAPKFILE				"QCAPK"
#define		QTERMSUPPORTAPPFILE	"TermApps"
#define		TRANSLOG				"TransLog"
#define		EXCEPTFILE				"Except"
#define		BATCHRECORD			"BatchRec"
#define		IPKREVOKEFILE			"QIPKRevoke"
#define		READERPARAMFILE		"RMFile"

#define			QTERMINALIDLEN		8
#define			QMERCHANTIDLEN		15
#define			QMERCHANTNAMELEN		40
#define			QAUTHCODELEN		6

typedef enum {LEFT_ALIGN,MIDDLE_ALIGN,RIGHT_ALIGN}ALIGN;

#if 0
typedef enum  { ASCII_FONT  , GB2312_FONT} FONT;
#else

#define			ASCII_FONT		0	
#define			GB2312_FONT		1	

#endif
typedef		struct
{
	UCHAR		aucTerminalID[QTERMINALIDLEN];
	UCHAR		aucMerchantID[QMERCHANTIDLEN];
	UCHAR		aucMerchantName[QMERCHANTNAMELEN];
	UCHAR		ucLanguage;
	BOOL		bForceOnline;
	BOOL		bBatchCapture;
	BOOL		bPBOC20;
	BOOL		bSelectAccount;
	UCHAR		ucAccountType;
	BOOL		bShowRandNum;
	BOOL		bDDOL;
	BOOL		bDTOL;
	UCHAR		ucTransType;
	UINT		uiTotalAmount;
	USHORT		uiTotalNums;
	UINT		uiTraceNumber;
	UCHAR		ucCommID;
	UCHAR		ucReaderPort;
	BOOL		bPrintTicket;
	UINT		uiBatchNumber;
	BOOL		bByPassSupport;
	BOOL		bAutoRemove;
	BOOL		bICCDebug;
	BOOL		bDigitalDebug;
}QEMVCONSTPARAM;

typedef		struct
{
	UCHAR		ucTransType;
	UCHAR		ucPANLen;
	UCHAR		aucPAN[QMAXPANDATALEN];
	UCHAR		ucPANSeq;
	UCHAR		aucAuthCode[QAUTHCODELEN];
	UINT		uiAmount;
	UINT		uiOtherAmount;
	UCHAR		aucTransDate[QTRANSDATELEN];
	UCHAR		aucTransTime[QTRANSTIMELEN];
	
}TRANSRECORD;

typedef struct
{
	UCHAR		ucOptionValue;
	UCHAR		szOptionMsg[30];
	
}OPTION,*POPTION;


#endif
