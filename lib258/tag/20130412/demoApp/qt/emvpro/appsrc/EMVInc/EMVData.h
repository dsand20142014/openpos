
#ifndef		_EMVDATA_H
#define		_EMVDATA_H


#ifndef		EMVDATA_DATA
#define		EMVDATA_DATA		extern
#endif

#include	<DataAuth.h>
#include	<EMVVerify.h>
#include	<TermConfig.h>
#include	<EMVLib.h>
#include	<EMVTransTag.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef		struct
{
	UINT		uiBalance;							//EC Balance
	UINT		uiNBalance;							//EC Balance after transaction
	UINT		uiBLimit;							//EC Balance Limit
	UCHAR		aucECICode[ECISSUERCODELEN];		//EC Issuer Authorization Code
	UINT		uiSTLimit;							//EC Single Transaction Limit
	UINT		uiResetThreshold;					//EC Reset Threshold
	
}ECTRANSINFO;
typedef		struct
{
	TRANSTYPE	enTransType;
	UCHAR		AIP[2];
	UCHAR		EMVTVR[5];
	UCHAR		EMVTSI[2];
	UCHAR		enAuthStyle;
	BOOL		bAuthFailFlag;
	PCAPK		pCurCAPK;
	DPCAPK	DpCurCAPK;
	UCHAR		aucTransDate[TRANSDATELEN]; //YYYYMMDD
	UCHAR		aucTransTime[TRANSTIMELEN]; //hhmmss
	UINT		uiRandNum;
	BOOL		bCheckSignature;
	UINT		uiTransAmount;
	UINT		uiOtherTransAmount;
	UINT		uiLogSumAmount;
	BOOL		bConnectFlag;
	CVMRESULT	CVMResult;
	TRANSRESULT	enTransResult;
	UCHAR		ucScriptResultNum;
	SCRIPTRESULT ScriptResult[MAXSCRIPTRESULTNUMS];
	BOOL		bAdvice;
	BOOL		bReversal;
	BOOL		bForceOnline;
	UINT		uiTraceNumber;
	UCHAR		ucRandNum;
	BOOL		bECTrans;
	UCHAR  	Authalgorithm;
	
}EMVTRANSINFO;

EMVDATA_DATA	EMVTRANSINFO	EMVTransInfo;
EMVDATA_DATA	TERMCONFIG		TermConfig;
EMVDATA_DATA	ECTRANSINFO		ECTransInfo;

/*
EMVDATA_DATA	ICDATAINFO		PSEDataInfo;
EMVDATA_DATA	ICDATAINFO		DDFDataInfo;
*/


void	EMVDATA_Init(void);
void	EMVDATA_TermInit(PTERMCONFIG pTermConfig);
void    EMVDATA_TransInit(PTRANSREQINFO pTransReqInfo);
void	EMVDATA_TransInfo(EMVTRANSINFO *pEMVTransInfo);
void	EMVDATA_KernelVer(PUCHAR pVer);
UCHAR	EMVDATA_GetTagValue(TAGPHASE enTagPhase,PUCHAR pucTag,PUCHAR pucBuff,USHORT * puiBuffLen);
UCHAR	EMVDATA_TagPackProcess(TAGPHASE enTagPhase,PUCHAR  pucTag,PUCHAR pucBuff,USHORT *puiLen);
UCHAR	EMVDATA_SetFloorLimit(UINT	uiFloorLimit);
UCHAR	EMVDATA_SetThreshold(UINT	uiThreshold);
UCHAR	EMVDATA_SetTragetPercent(UCHAR ucTP,UCHAR	ucMTP);
UCHAR	EMVDATA_SetAppversion(UCHAR* pucAppVersion);
UCHAR	EMVDATA_SetTAC(TACCODE tac);
UCHAR	EMVDATA_SetDDOL(PUCHAR pucDDol ,UCHAR ucDDolLen);
UCHAR	EMVDATA_SetDTDOL(PUCHAR pucDTDol ,UCHAR ucDTDolLen);
UCHAR	EMVDATA_SetTermCapab(PUCHAR pucTermCapab);
UCHAR	EMVDATA_SetTermAddCapab(PUCHAR pucTermAddCapab);
UCHAR	EMVDATA_SetCountryCode(PUCHAR pucCountryCode);
UCHAR	EMVDATA_SetTransCurrencyCode(PUCHAR pucTransCurrencyCode);
UCHAR	EMVDATA_ChangeTransAmount(UINT uiTransAmount,UINT uiOtherTransAmount);
UCHAR	EMVDATA_SetECFloorLimit(BOOL bExistFlag,UINT	uiFloorLimit);
BOOL	EMVDATA_CheckCardSupportEC(void);
void	EMVDATA_TerminalSupportEC(BOOL bSupport);

#ifdef __cplusplus
}
#endif
#endif
