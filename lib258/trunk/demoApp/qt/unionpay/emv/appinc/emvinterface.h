//  Copyright(c) Gu Bohua, 2001 - 2002.
/*  
  The program is written by Gu Bohua for EMV Level2 test.   
  The copyright declaration should be kept unchanged if you 
  have the right to use this program.
*/

#include	<EMV41.h>
#ifndef			_EMVINTERFACE_H
#define			_EMVINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
#ifndef EMVINTERFACEDATA
#define EMVINTERFACEDATA extern
#endif

#ifndef BYTE 
#define BYTE unsigned char
#endif

#define EMVERR_SUCCESS				0
#define ERR_EMV_ScriptLen		22
#define ERR_EMV_ScriptFormat    24
#define ERR_EMV_TermDataMissing 25
#define ERR_EMV_IccDataFormat   34
#define ERR_EMV_IccCommand      35
#define ERR_EMV_IccReturn		36
#define ERR_EMV_IccDataMissing  38
#define ERR_EMV_IccDataRedund   39
#define ERR_EMV_AppReselect     40
#define ERR_EMV_GetChallenge    41
#define ERR_EMV_CVRFail         42
#define ERR_EMV_CVRNoSupport    43
#define ERR_EMV_InterAuth       44
#define ERR_EMV_NotAccept		48
#define ERR_EMV_EmvDataFormat   49
#define ERR_EMV_CancelTrans     50
#define ERR_EMV_CardBlock       51 
#define ERR_EMV_CDADifferCID    52 
#define ERR_EMV_AppBlock         53
#define ERR_EMV_FALLBACK         54
#define EMVERR_CAPKNUMOVERFLOW	55
#define EMVERR_CAPKEXISTED		56
#define EMVERR_CAPKNOEXIST		57
#define EMVERR_APPNUMOVERFLOW		58
#define EMVERR_TERMAPPEXISTED		59
#define EMVERR_TERMAPPNOEXIST		60
#define EMVERR_OVERFLOW			61
#define EMVERR_DDOLNOTEXIST		62


#define EMVRIDMAXNUM		100
#define EMVMAXAPPNUM		32
#define MAXCAPKNUM			32
#define MAXAPPNUM			16

//option codes
#define OP_ADD				0
#define OP_DELETE			1
#define OP_MODIFY			2
#define OP_MODIFYEXPIRE       3

#define EMVTAB_BUFFMAX		1024



typedef struct {
	BYTE RID[RIDDATALEN];
	BYTE CAPKI;
	BYTE HashInd;
	BYTE Index;
	BYTE ArithInd;
	BYTE ModulLen;
	BYTE Modul[MAXMODULLEN];
	BYTE ExponentLen;
	BYTE Exponent[MAXEXPONENTLEN];
	BYTE ExpireDate[8];//add by yj
	BYTE CheckSum[HASHDATALEN];
}CAPK_STRUCT;


typedef struct {
	BYTE ASI;  //Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
	BYTE AIDLen;
	BYTE AID[MAXAIDLEN];//5-16
//	BYTE bLocalName;//If display app list using local language considerless of info in card.0-use card info;1-use local language.
//	BYTE AppLocalNameLen;
//	BYTE AppLocalName[16];
}TERMAPP;

typedef struct {
	BYTE RID[5];
	BYTE CAPKI;
	BYTE ExpireDate[8];//add by yj
	
}CAPKINFO;

typedef struct
{
	BYTE ucAIDLen;
	BYTE AID[EMVAIDLEN];
}SUPPORTAID;

typedef struct
{
	unsigned char	aucTag[3];
//	unsigned char   ucLenAttr;
}EMVTAGATTR;

typedef struct
{
	
	const EMVTAGATTR *pEMVTagAttr;
	unsigned char 	ucTagNum;
	EMVTAGINFO		EMVTagInfo[EMVTAG_NUMMAX];
	unsigned short	uiTagBuffLen;
	unsigned char 	aucEMVTagVaule[EMVTAB_BUFFMAX];   //TAG BUFF 
	unsigned short	uiTransInfoLen;
	unsigned char 	aucTransInfo[EMVTAB_BUFFMAX];		//TRANS INFO
		
}ICTRANSINFO;


EMVINTERFACEDATA ICTRANSINFO ICTransInfo;
EMVINTERFACEDATA EMVAIDPARAM AIDParam_LoadBuf[EMVMAXAPPNUM];

//EMVINTERFACEDATA BYTE ucTermCAPKNum;
//EMVINTERFACEDATA CAPK_STRUCT TermCAPK[MAXCAPKNUM];

//EMVINTERFACEDATA BYTE ucTermAIDNum;
EMVINTERFACEDATA TERMAPP AppListTerm[MAXAPPNUM];

EMVINTERFACEDATA BYTE ucCAPKNum;
EMVINTERFACEDATA CAPKINFO	CAPKInfo[EMVRIDMAXNUM];

EMVINTERFACEDATA BYTE SupportAIDNum;
EMVINTERFACEDATA SUPPORTAID SupportAID[EMVMAXAPPNUM];


unsigned char CAPK_TermCAPKNum(void);
unsigned char TERMAPP_TermAppNum(void);
unsigned char TERMAPP_DeleteAll(void);
EMVTAGINFO * EMVINTERFACE_SearchTag(ICTRANSINFO *pICTransInfo,unsigned char *paucTag);
unsigned char EMVINTERFACE_GetTagValue(ICTRANSINFO *pICTransInfo,unsigned char *paucTag,
								unsigned char *paucTagValue,unsigned short* uiTagValueLen);
unsigned char EMVINTERFACE_Unpack(ICTRANSINFO *pICTransInfo);
unsigned char EMVINTERFACE_INIT(ICTRANSINFO *pICTransInfo,const EMVTAGATTR  EMVTagAttrInfo[],unsigned char ucArrayNum);
unsigned char EMV_LoadCAPK(void);
unsigned char EMV_SetEMVConfig(void);
unsigned char EMV_LoadAIDParam(void);
unsigned char EMVONLINE_CAPKProcess(unsigned char ucProcessFlag,unsigned char* paucParam,unsigned short uiLen);
unsigned char EMVONLINE_AddRID(unsigned char* paucParam,unsigned short uiLen);
unsigned char EMVONLINE_AddAID(unsigned char* paucParam,unsigned short uiLen);
unsigned char TERMAPP_Insert(TERMAPP TermApp,EMVAIDPARAM AIDParamNew);
unsigned char EMVONLINE_AIDParamProcess(unsigned char ucProcessFlag,unsigned char* paucParam,unsigned short uiLen);
BYTE VerifyCAPK(CAPK* capk);
unsigned char VerifyReaderCAPK(unsigned char ucFlag);
#ifdef __cplusplus
}
#endif
//  Copyright(c) Gu Bohua(Birch Gu), 2001 - 2002.
#endif


