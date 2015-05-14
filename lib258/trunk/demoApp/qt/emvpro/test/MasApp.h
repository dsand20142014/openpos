#ifndef		_MASAPP_H
#define		_MASAPP_H

#ifndef		MASAPP_DATA
#define		MASAPP_DATA
#endif

#include <test/include.h>
#include <EMV41.h>
#include <smart.h>


#ifdef __cplusplus
extern "C"
{
#endif


UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);
unsigned char MASAPP_Event(void);
UCHAR	CardHolderConfirmApp(PCANDIDATELIST pCandidateList,PUCHAR pucAppIndex);
UCHAR	EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount);
UCHAR	CheckTransTVR(void);
void	CompleteTrans(void);
UCHAR	AddTransLog();
UINT	CalcLogTransAmount(void);
void ProcessMag(BOOL bFallBack,unsigned char* bufMag);
UCHAR SelectAccountType(void);
BOOL	TransCapture(TRANSRESULT	enTransResult);
BOOL	TransCapture(TRANSRESULT	enTransResult);
UCHAR	OnlineRespDataProcess(void);
#ifdef __cplusplus
}
#endif
#endif
