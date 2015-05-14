



#ifndef		_TERMCONFIG_H
#define		_TERMCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
#include	<DataAuth.h>
#include  <DomeDataAuth.h>
#include	<AppSelect.h>
#include 	<EMVIcc.h>
#include	<TermActAnalysis.h>
#include	<TermRiskManage.h>
typedef		struct
{
	UCHAR			ucTermType;
	BOOL			bSupportPSE;
	UCHAR			aucTermCapab[TERMCAPABLEN];
	UCHAR			aucTermAddCapab[TERMADDCAPABLEN];
	UCHAR			aucTermCountryCode[TERMCOUNTRYCODELEN];
	UCHAR			aucTransCurrencyCode[TRANSCURRENCYCODELEN];
	UCHAR			ucTransCurrencyExp;
	UCHAR			aucAppVerNumber[APPVERNUMBERLEN];
	UCHAR			ucDefaultDOLLen;
	UCHAR			aucDefaultDOL[MAXDEFAULTDOLLEN];
	UCHAR			ucDefaultTDOLLen;
	UCHAR			aucDefaultTDOL[MAXDEFAULTTDOLLEN];
	UCHAR			ucTermSupportAppNum;
	TERMSUPPORTAPP	*pTermSupportApp;
	UCHAR			ucCAPKNums;
	PCAPK			pTermSupportCAPK;
	UCHAR			ucDCAPKNums;
	DPCAPK		pTermSupportDCAPK;
	UCHAR			ucIPKRevokeNum;
	PIPKREVOKE		pIPKRevoke;
	UCHAR			ucExceptFileNum;
	PEXCEPTPAN		pExceptPAN;
	UCHAR			(*pfnEMVICC)(EMVICCIN *pEMVICCIn ,EMVICCOUT  *pEMVICCOut);
	UCHAR			(*pfnOfflinePIN)(PUCHAR	pucOfflinePin);
	UCHAR			(*pfnOnlinePIN)(PUCHAR pucEnPIN,PUCHAR pucPinLen);
	UCHAR			(*pfnEMVInformMsg)(PUCHAR	pucInformMsg);
	UCHAR			(*pfnEMVIDCardVerify)(void);
	UINT	uiFloorLimit;
	UINT	uiThreshold;
	UCHAR	ucTragetPercent;
	UCHAR	ucMaxTragetPercent;
	TACCODE	TAC;
	UCHAR			ucAccountType;
	BOOL			bPBOC20;
	BOOL			bIssuerReferal;
	UCHAR			(*pfnIssuerReferalProcess)(void);
	UCHAR			aucIFD[MAXIFDLEN];
	BOOL			bECSupport;
	BOOL			bECFLExist;			//EC FLOORLIMIT EXIST INDICATOR
	UINT			uiECFloorLimit;
	BOOL			bSM2;

}TERMCONFIG,*PTERMCONFIG;

#ifdef __cplusplus
}
#endif
#endif

