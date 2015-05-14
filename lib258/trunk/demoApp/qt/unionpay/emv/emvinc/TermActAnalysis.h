
#ifndef		_TERMACTANALYSIS_H
#define		_TERMACTANALYSIS_H

#ifndef		TERMACTANALYSIS_DATA
#define		TERMACTANALYSIS_DATA		extern
#endif

#include	<TypeDef.h>

#ifdef		__cplusplus
extern		"C"
{
#endif

#define BOOL	UCHAR
typedef		struct
{
	BOOL	bDenialExist;
	UCHAR	aucDenial[5];
	BOOL	bOnlineExist;
	UCHAR	aucOnline[5];
	BOOL	bDefaultExist;
	UCHAR	aucDefault[5];
}TACCODE;

typedef		enum {TC,ARQC,AAR,AAC}CRYPTTYPE;

typedef		enum {OFFLINEAPPROVED,OFFLINEDECLINED,ONLINEAPPROVED,ONLINEDECLINED,
				   UNABLEONLINE_OFFLINEAPPROVED,UNABLEONINE_OFFLINEDECLINED}TRANSRESULT;

TACCODE	TERMACTANALYSIS_GetIACCode();
BOOL	TERMACTANALYSIS_ProcessAct(PUCHAR pTermCode,PUCHAR pIssureCode);
CRYPTTYPE	TERMACTANALYSIS_Process(BOOL bResetThresholdCheck);

#ifdef		__cplusplus
}
#endif
#endif
