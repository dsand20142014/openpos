#ifndef		_QTERMCONFIG_H
#define		_QTERMCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


#include	<QAppSelect.h>
#include	<QDataAuth.h>
#include 	<QIcc.h>
#include	<QTermActAnalysis.h>
#include	<QTermRiskManage.h>


typedef		struct
{
	UCHAR			ucTermType;
	BOOL			bSupportPSE;
	UCHAR			aucTermCapab[QTERMCAPABLEN];
	UCHAR			aucTermAddCapab[QTERMADDCAPABLEN];
	UCHAR			aucTermCountryCode[QTERMCOUNTRYCODELEN];
	UCHAR			aucTransCurrencyCode[QTRANSCURRENCYCODELEN];
	UCHAR			ucTransCurrencyExp;
	UCHAR			aucAppVerNumber[QAPPVERNUMBERLEN];
	UCHAR			ucDefaultDOLLen;
	UCHAR			aucDefaultDOL[QMAXDEFAULTDOLLEN];
	UCHAR			ucDefaultTDOLLen;
	UCHAR			aucDefaultTDOL[QMAXDEFAULTTDOLLEN];
	UCHAR			ucTermSupportAppNum;
	PQTERMSUPPORTAPP pTermSupportApp;
	UCHAR			ucCAPKNums;
	PQCAPK			pTermSupportCAPK;
	UCHAR			ucQIPKRevokeNum;
	PQIPKREVOKE		pIPKRevoke;
	UCHAR			ucQExceptFileNum;
	PQEXCEPTPAN		pExceptPAN;
	UCHAR			(*pfnQICC)(QICCIN *pQICCIn ,QICCOUT  *pQICCOut);
	UCHAR			(*pfnOfflinePIN)(PUCHAR	pucOfflinePin);
	UCHAR			(*pfnOnlinePIN)(PUCHAR pucEnPIN,PUCHAR pucPinLen);
	UCHAR			(*pfnEMVInformMsg)(PUCHAR	pucInformMsg);
	UCHAR			(*pfnEMVIDCardVerify)(void);
	UINT	uiFloorLimit;
	UINT	uiThreshold;
	UCHAR	ucTragetPercent;
	UCHAR	ucMaxTragetPercent;
	QTACCODE	TAC;
	UCHAR			ucAccountType;
	BOOL			bPBOC20;
	
}QTERMCONFIG,*PQTERMCONFIG;

#ifdef __cplusplus
}
#endif
#endif

