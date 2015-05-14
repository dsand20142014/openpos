
#ifndef		_QTERMACTANALYSIS_H
#define		_QTERMACTANALYSIS_H

#ifndef		QTERMACTANALYSIS_DATA
#define		QTERMACTANALYSIS_DATA		extern
#endif

#include	<QTypeDef.h>

#ifdef		__cplusplus
extern		"C"
{
#endif


typedef		struct
{
	BOOL	bDenialExist;
	UCHAR	aucDenial[5];
	BOOL	bOnlineExist;
	UCHAR	aucOnline[5];
	BOOL	bDefaultExist;
	UCHAR	aucDefault[5];
}QTACCODE;

typedef		enum {Q_TC,Q_ARQC,Q_AAR,Q_AAC}QCRYPTTYPE;

typedef		enum {QOFFLINEAPPROVED,QOFFLINEDECLINED,QONLINEAPPROVED,QONLINEDECLINED,
				   QUNABLEONLINE_OFFLINEAPPROVED,QUNABLEONINE_OFFLINEDECLINED}QTRANSRESULT;

QTACCODE	QTERMACTANALYSIS_GetIACCode();
BOOL	QTERMACTANALYSIS_ProcessAct(PUCHAR pTermCode,PUCHAR pIssureCode);
QCRYPTTYPE	QTERMACTANALYSIS_Process(void);

#ifdef		__cplusplus
}
#endif
#endif
