#ifndef		_MASAPP_H
#define		_MASAPP_H

#ifndef		MASAPP_DATA
#define		MASAPP_DATA
#endif

#include <include.h>
#include <EMV41.h>
#include <smart.h>


#ifdef __cplusplus
extern "C"
{
#endif


UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);
unsigned char MASAPP_Event(unsigned char ucDispInfoFlag, unsigned char * ucOutKey);
UCHAR	CardHolderConfirmApp(PCANDIDATELIST pCandidateList,PUCHAR pucAppIndex);
UCHAR	EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount);
unsigned char MasAPP_CheckAID(void);
unsigned char HBMasAPP_CheckAID(void);
UCHAR	CheckTransTVR(void);
void	CompleteTrans(void);
unsigned char MASAPP_WaitReadIC(unsigned char ucInputMode);
UCHAR	AddTransLog(unsigned char ucMSGType);
UINT	CalcLogTransAmount(void);
void ProcessMag(UCHAR bFallBack,unsigned char* bufMag);
UCHAR SelectAccountType(void);
UCHAR	TransCapture(TRANSRESULT	enTransResult);
UCHAR	OnlineRespDataProcess(void);
void MasApp_SetEMVTransInfo(void);
unsigned char MasApp_CopytEMVTransInfoToNormalTrans(void);
unsigned char MasApp_CARDPAN(unsigned char* pstrCardNo,unsigned char *pucCardNoLen);
unsigned char EMV_DisplayPKInfo(void);
unsigned char EMV_DisplayAIDParam(void);
#ifdef __cplusplus
}
#endif
#endif
