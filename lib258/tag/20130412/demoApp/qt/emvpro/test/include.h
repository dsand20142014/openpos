#ifndef		_INCLUDE_H
#define		_INCLUDE_H

#include	<TypeDef.h>
#include	<oslib.h>
#include	<EMVDef.h>
#include "new_drv.h"
#define		CONSTPARAMFILE			"ConstPara"
#define		EMVCONFIGFILE			"EMVConfig"
#define		CAPKFILE				"CAPK"
#define		TERMSUPPORTAPPFILE	"TermApps"
#define		TRANSLOG				"TransLog"
#define		EXCEPTFILE				"Except"
#define		BATCHRECORD			"BatchRec"
#define		IPKREVOKEFILE			"IPKRevoke"

#define			TERMINALIDLEN		8
#define			MERCHANTIDLEN		15
#define			MERCHANTNAMELEN		40
#define			AUTHCODELEN		6
typedef enum {LEFT_ALIGN,MIDDLE_ALIGN,RIGHT_ALIGN}ALIGN;


typedef struct
{
    //���Դ���: progress ��UI����
    unsigned char ucClsFlag;	// 0xAA ������Ļ
    bool clearScreenFlag;

    unsigned int uiLines;		//��ʾ����
    unsigned char aucLine1[64];
    unsigned char aucLine2[64];
    unsigned char aucLine3[64];
    unsigned char aucLine4[64];
    unsigned char aucLine5[64];
    unsigned char aucLine6[64];
    unsigned char aucLine7[64];
    unsigned char aucLine8[64];
    unsigned char aucLine9[64];

    unsigned char aucLine10[64];
    unsigned char aucLine11[64];
    unsigned char aucLine12[64];
    unsigned char aucLine13[64];
    unsigned char aucLine14[64];
    unsigned char aucLine15[64];
    unsigned char aucLine16[64];
    unsigned char aucLine17[64];
    unsigned char aucLine18[64];
    unsigned char aucLine19[64];

    unsigned char aucLine20[64];
    unsigned char aucLine21[64];
    unsigned char aucLine22[64];
    unsigned char aucLine23[64];
    unsigned char aucLine24[64];
    unsigned char aucLine25[64];
    unsigned char aucLine26[64];
    unsigned char aucLine27[64];
    unsigned char aucLine28[64];
    unsigned char aucLine29[64];

    unsigned char aucLine30[64];
    unsigned char aucLine31[64];
    unsigned char aucLine32[64];
    unsigned char aucLine33[64];
    unsigned char aucLine34[64];
    unsigned char aucLine35[64];
    unsigned char aucLine36[64];
    unsigned char aucLine37[64];
    unsigned char aucLine38[64];
    unsigned char aucLine39[64];
    unsigned char aucLine40[64];

}FACEPROTOUI;

typedef struct
{
    unsigned char ucProBar;//�Ƿ��н�����
    unsigned int  uiProgress;//����������

    unsigned int  uiTimes;//����ʱ

    NEW_DRV_TYPE OweDrv;
    FACEPROTOUI ProToUi;

    //FACEUITOPRO UiToPro;
}PROUIINTERFACE;

typedef		struct
{
	UCHAR		aucTerminalID[TERMINALIDLEN];
	UCHAR		aucMerchantID[MERCHANTIDLEN];
	UCHAR		aucMerchantName[MERCHANTNAMELEN];
	UCHAR		ucLanguage;
	BOOL		bForceOnline;
	BOOL		bBatchCapture;
	BOOL		bPBOC20;
	BOOL		bSelectAccount;
	UCHAR		ucAccountType;
	BOOL		bShowRandNum;
	BOOL		bDDOL;
	BOOL		bDTOL;
	UCHAR		ucTransType;
	UINT		uiTotalAmount;
	USHORT		uiTotalNums;
	UINT		uiTraceNumber;
	UCHAR		ucCommID;
	BOOL		bPrintTicket;
    UINT		uiBatchNumber;
	BOOL		bIssuerReferal;
	BOOL		bECSupport;
    BOOL 		bDEBUG;
    BOOL        bAutoSupport;
    UINT        uiDelayTime;
    UINT        uiTransAmount;
    UINT        uiOtherAmount;
    UCHAR       ucPinNum[6];

}EMVCONSTPARAM;


typedef		struct
{
	UCHAR		ucTransType;
	UCHAR		ucPANLen;
	UCHAR		aucPAN[MAXPANDATALEN];
	UCHAR		ucPANSeq;
	UCHAR		aucAuthCode[AUTHCODELEN];
	UINT		uiAmount;
	UINT		uiOtherAmount;
	UCHAR		aucTransDate[TRANSDATELEN];
	UCHAR		aucTransTime[TRANSTIMELEN];
	
}TRANSRECORD;

typedef struct
{
	UCHAR		ucOptionValue;
	UCHAR		szOptionMsg[30];
	
}OPTION,*POPTION;


#endif
