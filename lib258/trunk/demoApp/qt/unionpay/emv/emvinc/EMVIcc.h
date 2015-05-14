
#ifndef			_EMVICC_H
#define			_EMVICC_H

#ifndef			EMVICC_DATA
#define			EMVICC_DATA		extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <TypeDef.h>
#include <CardSWDef.h>


/************error define************/
#define			EMVICCERR_SUCCESS				0
#define			EMVICCERR_COMMAND				1
#define         EMVICCERR_PARAM				2
#define         EMVICCERR_ICCDATA				3

#define			MAXEMVICCDATABUFF				300

#define			BOOL UCHAR
typedef		struct
{
	UCHAR		ucCla;
	UCHAR		ucIns;
	UCHAR		ucP1;
	UCHAR 		ucP2;
	USHORT		uiLc;
	UCHAR		aucDataIn[MAXEMVICCDATABUFF];
	USHORT		uiLe;
	
}EMVICCIN;


typedef struct{
	USHORT		uiLenOut; 
   	UCHAR		aucDataOut[MAXEMVICCDATABUFF];
   	UCHAR 		ucSWA;
   	UCHAR 		ucSWB;
	
}EMVICCOUT;

EMVICC_DATA		EMVICCIN	EMVIccIn;
EMVICC_DATA		EMVICCOUT	EMVIccOut;

void	EMVICC_Init(void);
USHORT	EMVICC_SW(void);
UCHAR	EMVICC_CommandResult(void);
UCHAR EMVICC_SelectFile(unsigned char *pucAID,unsigned char ucAIDLen,BOOL bNextFlag);
UCHAR  EMVICC_ReadRecordbySFI(unsigned char ucSFI,unsigned char ucRecordNumber);
UCHAR	EMVICC_VerifyPIN(UCHAR	ucPINType,PUCHAR pucPinData,UCHAR	ucPinDataLen);
UCHAR	EMVICC_GetData(PUCHAR pucTag,UCHAR	ucTagLen);
UCHAR	EMVICC_GetChallenge(void);
UCHAR	EMVICC_ExternalAuth(PUCHAR pAuthCode,UCHAR	ucAuthCodeLen);
void EMVICC_Display(void);

#ifdef __cplusplus
}
#endif
#endif

