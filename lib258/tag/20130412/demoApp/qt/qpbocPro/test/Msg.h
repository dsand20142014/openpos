
#ifndef		_MSG_H
#define		_MSG_H

#ifndef		MSG_DATA
#define		MSG_DATA	extern
#endif

#include	<QTypeDef.h>
#include	<osmodem.h>
#include	<QError.h>
#include	<Global.h>

#ifdef		__cplusplus
extern "C"
{
#endif

#define CHAR_STX				0x02
#define CHAR_ETX				0x03

#define Msg_EMV_AuthRQ          	0x01
#define Msg_EMV_FinaRQ				0x02
#define Msg_EMV_AuthRP				0x03
#define Msg_EMV_Confirm			0x04
#define	Msg_EMV_TransResult		0x05
#define Msg_EMV_BatchAdvice		0x51
#define Msg_EMV_BatchTrans		0x52
#define Msg_EMV_Reconci			0x06
#define Msg_EMV_OnlineAdvice		0x07
#define Msg_EMV_Reversal        	0x08

/*
#define Msg_EMV_DownloadKey     	9

#define Msg_EMV_BatchOnlineApprove  10
#define Msg_EMV_BatchOnlineDecline  11
#define Msg_EMV_OnlineForceApprove  12
#define Msg_EMV_OfflineForceApprove 13
*/
#define Msg_EMV_UpdatePara      0x0E


enum {BASEPARAM=0x01,CAPKPARAM,AIDPARAM,EXCEPTFILEPARAM,IPKREVOKEPARAM};
enum {ADDACTION =0x01,DELETEACTION,MODIFYACTION};
typedef		struct
{
	UCHAR	ucParamIndex;
	USHORT	uiParamLen;       // 999 is variable len
	PUCHAR	pParamLenIndicate;
	PUCHAR	pParamIndicate;
}PARAMINDICATE;



UCHAR	MSG_OpenCom(void);
UCHAR MSG_ComSendData(PUCHAR pucBuff, USHORT uiLen);
UCHAR MSG_ComRecvData(PUCHAR pBuff, USHORT *puiLen);
UCHAR MSG_ComSendByte(UCHAR	 ucSendChar);
UCHAR MSG_ComClose();
UCHAR	MSG_PackTagData(PUCHAR pucTag,PUCHAR pucPackBuff,UINT *puiLen);
UCHAR	MSG_FinaTransRequest(PUCHAR pRecvBuff,USHORT *puiLen);
UCHAR	MSG_CAPKAdd(PQCAPK	pCapk);
UCHAR	MSG_CAPKModify(PQCAPK 	pCapk);
UCHAR	MSG_CAPKDelete(PQCAPK 	pCapk);
UCHAR	MSG_ProcessCAPKUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen);
UCHAR	MSG_ProcessBaseParmUpdate(PUCHAR	pBaseParam,USHORT uiParamLen);
UCHAR	MSG_ProcessParamUpdate(UCHAR	*pucParamData,USHORT uiParamLen);
UCHAR	MSG_UpdateParam(void);
UCHAR	MSG_IPKRevokeAdd(PQIPKREVOKE	pIPKRevoke);
UCHAR	MSG_IPKRevokeModify(PQIPKREVOKE 	pIPKRevoke);
UCHAR	MSG_IPKRevokeDelete(PQIPKREVOKE		pIPKRevoke);
UCHAR	MSG_ProcessIPKRevokeUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen);
UCHAR	MSG_BatchUp(void);
UCHAR	MSG_Reconciliation(void);
UCHAR	MSG_TransConfirm(void);
UCHAR	MSG_AIDAdd(QTERMSUPPORTAPP* pAID);
UCHAR	MSG_AIDModify(QTERMSUPPORTAPP* pAID);
UCHAR	MSG_AIDDelete(QTERMSUPPORTAPP* pAID);
UCHAR	MSG_ProcessAIDUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen);
UCHAR	MSG_ExceptFileAdd(PQEXCEPTPAN	pFile);
UCHAR	MSG_ExceptFileModify(PQEXCEPTPAN	pFile);
UCHAR	MSG_ExceptFileDelete(PQEXCEPTPAN pFile);
UCHAR	MSG_ProcessExceptFileUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen);
UCHAR	MSG_ProcessIccFailBack(void);
UCHAR	MSG_TransResult(void);

#ifdef		__cplusplus
}
#endif
#endif
