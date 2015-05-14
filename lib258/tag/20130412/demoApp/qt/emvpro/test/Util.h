
#ifndef		_UTIL_H
#define		_UTIL_H

#ifndef		UTIL_DATA
#define		UTIL_DATA		extern
#endif

#include	<TypeDef.h>
#include	<EMV41.h>
#include "test/include.h"
#ifdef	__cplusplus
extern "C"
{
#endif

#define 	MAXDISPCHAR					16
#define 	ONESECOND						100
#define 	MAXAMOUNTLEN					10
#define 	MAXLINECHARNUM					16
unsigned char UTIL_InputAmount(unsigned char ucLine,
				unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ulMaxAmount);
unsigned char UTIL_Input_pp(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf);
unsigned char UTIL_Input_PP_Uart(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr,
									 char ucMinNB,unsigned char ucMaxNB,unsigned char *PinData);
unsigned char UTIL_Input(ALIGN enAlign,unsigned char ucLine,unsigned char ucClrFlag,unsigned char ucMin, unsigned char ucMax,
                unsigned char ucType,
                unsigned char *pucBuf,
                unsigned char *pucMask);

UCHAR UTIL_WriteConstParamFile(EMVCONSTPARAM * pConstParam);
POPTION UTIL_SelectOption(PUCHAR szOptionTitle,FONT ucFont,POPTION pOption ,UCHAR ucOptionNums);


UCHAR	UTIL_OfflinePIN(PUCHAR	pucOfflinePin);
UCHAR	UTIL_OnlinePIN(PUCHAR pucEnPIN,PUCHAR pucPinLen);
UCHAR UTIL_WriteEMVConfigFile(TERMCONFIG * pEMVConfig);
UCHAR	TERMSETTING_Func(void);
UCHAR	TERMSETTING_TransParam(void);
UCHAR UTIL_SaveCAPKFile(void);
UCHAR UTIL_SaveAIDFile(void);
UCHAR UTIL_SaveExceptFile(void);
UCHAR UTIL_SaveIPKRevokeFile(void);
unsigned char UTIL_GetKey(unsigned char wait_sec);
UCHAR	UTIL_DeleteAllData(UCHAR* pucFileName);
UCHAR	UTIL_EMVInformMsg(PUCHAR	pucInformMsg);
UCHAR	UTIL_IDCardVerify(void);
UCHAR	UTIL_IssuerReferalProcess(void);
void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule);

#ifdef	__cplusplus
}
#endif

#endif
