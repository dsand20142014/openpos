#ifndef		_MASAPP_H
#define		_MASAPP_H

#ifndef		MASAPP_DATA
#define		MASAPP_DATA
#endif

#include <include.h>
#include <QPBOC.h>
#include <smart.h>


#ifdef __cplusplus
extern "C"
{
#endif


UCHAR	QICC(QICCIN *pQICCIn ,QICCOUT *pQICCOut);
unsigned char MASAPP_Event(void);
UCHAR	CardHolderConfirmApp(PQCANDIDATELIST pCandidateList,PUCHAR pucAppIndex);
UCHAR	EMVTransProcess(QTRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount);
UCHAR	CheckTransTVR(void);
void	CompleteTrans(void);
UCHAR	AddTransLog();
UINT	CalcLogTransAmount(void);
void ProcessMag(BOOL bFallBack,unsigned char* bufMag);
UCHAR SelectAccountType(void);
BOOL	TransCapture(QTRANSRESULT	enTransResult);
BOOL	TransCapture(QTRANSRESULT	enTransResult);
UCHAR	OnlineRespDataProcess(void);

unsigned char Os__xget_key1111();
#ifdef __cplusplus
}
#endif
#endif
