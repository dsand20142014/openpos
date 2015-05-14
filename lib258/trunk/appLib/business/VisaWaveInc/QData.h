
#ifndef		_QDATA_H
#define		_QDATA_H


#ifndef		QDATA_DATA
#define		QDATA_DATA		extern
#endif

#include	<QDataAuth.h>
#include	<QVerify.h>
#include	<QTermConfig.h>
#include	<QLib.h>
#include	<QTransTag.h>
#include	<MSD.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef		struct
{
	QTRANSTYPE	enTransType;
	UCHAR		AIP[2];
	UCHAR		EMVTVR[5];
	UCHAR		EMVTSI[2];
	UCHAR		enAuthStyle;
	BOOL		bAuthFailFlag;
	PQCAPK		pCurCAPK;
	UCHAR		aucTransDate[QTRANSDATELEN]; //YYYYMMDD
	UCHAR		aucTransTime[QTRANSTIMELEN]; //hhmmss
	UINT		uiRandNum;
	BOOL		bCheckSignature;
	UINT		uiTransAmount;
	UINT		uiOtherTransAmount;
	UINT		uiLogSumAmount;
	BOOL		bConnectFlag;
	QCVMRESULT	CVMResult;
	QTRANSRESULT	enTransResult;
	UCHAR		ucScriptResultNum;
	QSCRIPTRESULT ScriptResult[QMAXSCRIPTRESULTNUMS];
	BOOL		bAdvice;
	BOOL		bReversal;
	BOOL		bForceOnline;
	UINT		uiTraceNumber;
	UCHAR		ucRandNum;
	UCHAR	EntryMode;
}QEMVTRANSINFO;

typedef struct
{
	BOOL		bRCFLExist;				//Reader Contactless Floor Limit Exist Flag
	ULONG		ulRCFLimit;				//Reader Contactless Floor Limit
	BOOL		bRCTLExist;				//Reader Contactless Transaction Limit  Exist Flag
	ULONG		ulRCTLimit;				//Reader Contactless Transaction Limit
	BOOL		bRCVReqLExist;				//Reader CVM Required Limit  Exist Flag
	ULONG		ulRCVMReqLimit;			//Reader CVM Required Limit
	UCHAR		aucTermTransQ[4];			//Terminal Transaction Qualifiers
//	ULONG		ulVLPAFunds;				//VLP Available Funds
//	ULONG		ulVLPFLimit;				//VLP Funds Limit
//	ULONG		ulVLPRThreshold;			//VLP Reset Threshold
//     ULONG		ulVLPSTransLimit;			//VLP Single Transaction Limit
	BOOL		bStatusCheck;				//Status Check Flag
}QREADERPARAM1;


QDATA_DATA	QEMVTRANSINFO	QEMVTransInfo;
QDATA_DATA	QTERMCONFIG		QTermConfig;
QDATA_DATA	QREADERPARAM1		QReaderParam;

/*
EMVDATA_DATA	QICDATAINFO		PSEDataInfo;
EMVDATA_DATA	QICDATAINFO		DDFDataInfo;
*/



void	QDATA_TermInit(PQTERMCONFIG pTermConfig);
void    QDATA_TransInit(PQTRANSREQINFO pTransReqInfo);
void	QDATA_TransInfo(QEMVTRANSINFO *pEMVTransInfo);
void	QDATA_KernelVer(PUCHAR pVer);
UCHAR	QDATA_GetTagValue(QTAGPHASE enTagPhase,PUCHAR pucTag,PUCHAR pucBuff,USHORT * puiBuffLen);
UCHAR	QDATA_TagPackProcess(QTAGPHASE enTagPhase,PUCHAR  pucTag,PUCHAR pucBuff,USHORT *puiLen);
UCHAR	QDATA_SetFloorLimit(UINT	uiFloorLimit);
UCHAR	QDATA_SetThreshold(UINT	uiThreshold);
UCHAR	QDATA_SetTragetPercent(UCHAR ucTP,UCHAR	ucMTP);
UCHAR	QDATA_SetAppversion(UCHAR* pucAppVersion);
UCHAR	QDATA_SetTAC(QTACCODE tac);
UCHAR	QDATA_SetDDOL(PUCHAR pucDDol ,UCHAR ucDDolLen);
UCHAR	QDATA_SetDTDOL(PUCHAR pucDTDol ,UCHAR ucDTDolLen);
UCHAR	QDATA_ReaderDParam(QREADERPARAM1 * pQReaderParam);


#ifdef __cplusplus
}
#endif
#endif
