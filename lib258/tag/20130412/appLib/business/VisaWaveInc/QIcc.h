
#ifndef			_QICC_H
#define			_QICC_H

#ifndef			QICC_DATA
#define			QICC_DATA		extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <QTypeDef.h>
#include <QCardSWDef.h>


/************error define************/
#define			QICCERR_SUCCESS				0
#define			QICCERR_COMMAND				1
#define         QICCERR_PARAM				2
#define         QICCERR_ICCDATA				3

#define			MAXQICCDATABUFF				300

typedef		struct
{
	UCHAR		ucCla;
	UCHAR		ucIns;
	UCHAR		ucP1;
	UCHAR 		ucP2;
	USHORT		uiLc;
	UCHAR		aucDataIn[MAXQICCDATABUFF];
	USHORT		uiLe;
	
}QICCIN;


typedef struct{
	USHORT		uiLenOut; 
   	UCHAR		aucDataOut[MAXQICCDATABUFF];
   	UCHAR 		ucSWA;
   	UCHAR 		ucSWB;
	
}QICCOUT;

QICC_DATA		QICCIN	QIccIn;
QICC_DATA		QICCOUT	QIccOut;

void	QICC_Init(void);
USHORT	QICC_SW(void);
UCHAR	QICC_CommandResult(void);
UCHAR QICC_SelectFile(unsigned char *pucAID,unsigned char ucAIDLen,BOOL bNextFlag);
UCHAR  QICC_ReadRecordbySFI(unsigned char ucSFI,unsigned char ucRecordNumber);
UCHAR	QICC_VerifyPIN(UCHAR	ucPINType,PUCHAR pucPinData,UCHAR	ucPinDataLen);
UCHAR	QICC_GetData(PUCHAR pucTag,UCHAR	ucTagLen);
UCHAR	QICC_GetChallenge(void);
UCHAR	QICC_ExternalAuth(PUCHAR pAuthCode,UCHAR	ucAuthCodeLen);

#ifdef __cplusplus
}
#endif
#endif

