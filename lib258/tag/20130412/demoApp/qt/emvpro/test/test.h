#ifndef _TEST_H
#define _TEST_H

#include "oslib.h"
#include "osicc.h"
//#include	<EMV41.h>
//#include	<include.h>
#include "TypeDef.h"

#define			EMVERROR_SUCCESS				0
#define			EMVERROR_FUNCNOTSUPPORT		1
#define			EMVERROR_CARDBLOCKED			2
#define			EMVERROR_PSENOTFIND			3
#define			EMVERROR_PSEBLOCKED			4
#define         EMVERROR_ICCCOMAND			5
#define			EMVERROR_ICCDATA				6
#define			EMVERROR_PARAM					7
#define			EMVERROR_MISSDATA				8
#define			EMVERROR_RESELECT				9
#define			EMVERROR_DATA					10
#define			EMVERROR_DATAAUTH				11
#define			EMVERROR_CAPKNOTFOUND			12
#define			EMVERROR_CERTIFICATIONMISS   13
#define			EMVERROR_CERTIFICATIONLEN	14
#define			EMVERROR_HASHVALUE			15
#define			EMVERROR_CAPKINDEXNOTFOUND   16
#define			EMVERROR_CANCEL				17
#define			EMVERROR_NEEDARQC				18
#define			EMVERROR_EXTERNALAUTH			19
#define			EMVERROR_ISSUREDATA			20
#define			EMVERROR_OPENCOM				21
#define			EMVERROR_SENDDATA				22
#define			EMVERROR_RECEIVEDATA			23
#define			EMVERROR_TAGNOTFIND			24
#define			EMVERROR_NOSUPPORTAPP			25
#define			EMVERROR_IPKREVOKE			26
#define			EMVERROR_PARAMUPDATE			27
#define			EMVERROR_VELOCITYCHECK		28
#define			EMVERROR_VERIFY				29
#define			EMVERROR_SAVEFILE				30
#define			EMVERROR_READFILE				31
#define			EMVERROR_WRITE					32
#define			EMVERROR_INVALIDSERVICE		33
#define			EMVERROR_SCRIPTLIMIT			34
#define			EMVERROR_CAPKINVALID			35
#define			EMVERROR_OVERFLOW				36
#define			EMVERROR_CID					37
#define			EMVERROR_DATE					38
#define			EMVERROR_RESPFORMAT			39
#define			EMVERROR_BYPASS				40
#define			EMVERROR_FALLBACK				41
#define			EMVERROR_APPBLOCKED			42
#define			EMVERROR_ISSUERREFERAL		43

#ifndef		GLOBAL_DATA
#define		GLOBAL_DATA		extern
#endif

#ifndef		EMVDATA_DATA
#define		EMVDATA_DATA		extern
#endif


#define			TERMINALIDLEN		8
#define			MERCHANTIDLEN		15
#define			MERCHANTNAMELEN		40
#define			AUTHCODELEN		6
#define		TRANSDATELEN				8
#define		TRANSTIMELEN				6
#define		RIDDATALEN					5
#define		CERTSERIALLEN				3
#define		MAXMODULLEN				248
#define		MAXEXPONENTLEN				3
#define		HASHDATALEN				20
#define		SCRIPTIDLEN				4
#define		MAXSCRIPTRESULTNUMS		32

typedef		enum { CASH,GOODS,SERVICES,CASHBACK,INQUIRY,TRANSFER,PAYMENT,ADMIN,DEPOSIT,
                   PREAUTHOR,DEPOSIT_CASH,DEPOSIT_SPECACCOUNT}TRANSTYPE;

typedef		enum {ALLPHASETAG,PSEPHASETAG,ENTRYPHASETAG,ADFPHASETAG,INITAPPPHASETAG,
                  RECORDPHASETAG,DATAAUTHPHASETAG,FIRSTGACPHASETAG,SECONDGACPHASETAG,ISSUREPHASETAG}TAGPHASE;

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
}EMVCONSTPARAM;

typedef struct
{
    UCHAR			aucTerminalID[TERMINALIDLEN];
    UCHAR			aucMerchantID[MERCHANTIDLEN];
    UCHAR			aucMerchantName[MERCHANTNAMELEN];
    TRANSTYPE		enTransType;
    UCHAR			aucTransDate[TRANSDATELEN]; //YYYYMMDD
    UCHAR			aucTransTime[TRANSTIMELEN]; //hhmmss
    UINT			uiRandNum;
    UINT			uiAmount;
    UINT			uiOtherAmount;
    UINT			uiLogAmount;
    UCHAR			ucEntryMode;
    BOOL			bForceOnline;
    UINT			uiTraceNumber;
}TRANSREQINFO,*PTRANSREQINFO;


typedef		struct
{
    UCHAR		aucRID[RIDDATALEN];
    UCHAR		ucCAPKI;
    UCHAR 		ucHashInd;
    UCHAR 		ucArithInd;
    UCHAR		ucIndex;
    UCHAR 		ucModulLen;
    UCHAR 		aucModul[MAXMODULLEN];
    UCHAR 		ucExponentLen;
    UCHAR 		aucExponent[MAXEXPONENTLEN];
    UCHAR 		aucExpireDate[8];
    UCHAR 		aucCheckSum[HASHDATALEN];
}CAPK,*PCAPK;

typedef		struct
{
    UCHAR	ucCVMCode;
    UCHAR	ucConditionCode;
    UCHAR	ucCVMResult;

}CVMRESULT;

typedef		struct
{
    UCHAR  	ucSeqNumber : 4,
            ucScriptResult : 4;
    UCHAR	aucScriptID[SCRIPTIDLEN];
}SCRIPTRESULT;

typedef		enum {OFFLINEAPPROVED,OFFLINEDECLINED,ONLINEAPPROVED,ONLINEDECLINED,
                   UNABLEONLINE_OFFLINEAPPROVED,UNABLEONINE_OFFLINEDECLINED}TRANSRESULT;

typedef		struct
{
    TRANSTYPE	enTransType;
    UCHAR		AIP[2];
    UCHAR		EMVTVR[5];
    UCHAR		EMVTSI[2];
    UCHAR		enAuthStyle;
    BOOL		bAuthFailFlag;
    PCAPK		pCurCAPK;
    UCHAR		aucTransDate[TRANSDATELEN]; //YYYYMMDD
    UCHAR		aucTransTime[TRANSTIMELEN]; //hhmmss
    UINT		uiRandNum;
    BOOL		bCheckSignature;
    UINT		uiTransAmount;
    UINT		uiOtherTransAmount;
    UINT		uiLogSumAmount;
    BOOL		bConnectFlag;
    CVMRESULT	CVMResult;
    TRANSRESULT	enTransResult;
    UCHAR		ucScriptResultNum;
    SCRIPTRESULT ScriptResult[MAXSCRIPTRESULTNUMS];
    BOOL		bAdvice;
    BOOL		bReversal;
    BOOL		bForceOnline;
    UINT		uiTraceNumber;
    UCHAR		ucRandNum;
    BOOL		bECTrans;

}EMVTRANSINFO;

GLOBAL_DATA	 EMVCONSTPARAM		ConstParam;
GLOBAL_DATA	 TRANSREQINFO TransReqInfo;

#define SMART_OK				0		/* Return Code Ok  0x00 */
#define SMART_ASYNC				1       /* Synchronous Ok  0x01 */
#define SMART_SYNC				2       /* Asynchronous Ok 0x02 */
#define SMART_DRIVER_ERROR		3       /* Driver Error    0x03 */
#define SMART_CARD_ERROR		4       /* Card Error      0x04 */
#define SMART_OWERFLOW			5       /* Owerflow Buffer 0x05 */
#define SMART_ABORT				6       /* Abort Key       0x06 */
#define SMART_ABSENT			7		/* Smart Absent    0x07 */
#define SMART_PRESENT           8		/* Smart Present   0x08 */

/* Smart Async Order Type */
#define TYPE0					0		/* Order Type0 LgIn,BufIn                     */
#define TYPE1					1		/* Order Type1 CLA,INS,P1,P2                  */
#define TYPE2					2		/* Order Type2 CLA,INS,P1,P2,LgOut            */
#define TYPE3					3		/* Order Type3 CLA,INS,P1,P2,LgIn,BufIn       */
#define TYPE4					4		/* Order Type4 CLA,INS,P1,P2,LgIn,BufIn,LgOut */

#define SALEERR_SUCCESS         0
#define SALEERR_SEEKFILE		1
#define	SALEERR_READFILE        2
#define SALEERR_OPENFILE        4
#define SALEERR_WRITEFILE		5
#define SALEERR_DELETERECORD    6




typedef		struct
{
    unsigned char 	aucTag[2];
    unsigned short 	uiLen;
    unsigned char   *pTagValue;
    TAGPHASE		enTagPhase;
}EMVTRANSTAGDATA;

unsigned char OnEve_power_on(void);

unsigned char SMART_WarmReset(
        unsigned char ucReader,
        unsigned char ucCardStandard,
        unsigned char * pucReset,
        unsigned short * puiLength);



unsigned char UTIL_InputAmount(unsigned char ucLine,
                unsigned long *pulAmount,
                unsigned long ulMinAmount, unsigned long ulMaxAmount);

UCHAR UTIL_WriteConstParamFile(EMVCONSTPARAM * pConstParam);

UCHAR	MSG_ProcessIccFailBack(void);

EMVTRANSTAGDATA * EMVTRANSTAG_SearchTag(TAGPHASE enTagPhase,unsigned char *paucTag);
EMVDATA_DATA	EMVTRANSINFO	EMVTransInfo;
UCHAR	FILE_ReadRecordNumByFileName(UCHAR* pucFileName,UINT *puiRecordNum);
UCHAR	EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount);



#endif
