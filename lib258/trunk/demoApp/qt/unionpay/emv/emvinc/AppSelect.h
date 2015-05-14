


#ifndef		_APPSELECT_H
#define		_APPSELECT_H

#ifndef		APPSELECT_DATA
#define		APPSELECT_DATA		extern
#endif

#include	<EMVDef.h>
#include	<TypeDef.h>
#include	<EMVTag.h>
#include	<DDFStack.h>


#ifdef __cplusplus
extern "C" {
#endif

/************error define************/
#define     APPSELECTERR_SUCCESS				0x00
#define		APPSELECTERR_OVERFLOW				0x01	
#define		APPSELECTERR_DATA					0x02


typedef enum	 {APPSELECTBYPSE,APPSELECTBYTERMAPP} APPSELECTMETHOD;
typedef 	struct 
{
	UCHAR	ucASI;            			//Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
	UCHAR 	ucAIDLen;
	UCHAR 	aucAID[MAXAIDLEN];        	//5-16
	/*
	UCHAR 	ucbLocalName;				//If display app list using local language considerless of info in card.0-use card info;1-use local language.
	UCHAR 	ucAppLocalNameLen;
	UCHAR 	aucAppLocalName[MAXLOACALNAMELEN];
	*/
}TERMSUPPORTAPP;

typedef		struct
{
	UCHAR		ucADFNameLen;
	UCHAR		aucADFName[MAXADFNAMELEN];
	UCHAR		ucAppLabelLen;
	UCHAR		aucAppLabel[MAXAPPLABELLEN];	
	UCHAR		ucAppPreferNameLen;
	UCHAR		aucAppPreferName[MAXAPPPREFERNAMELEN];
	BOOL		bAppPriorityExist;
	UCHAR		ucAppPriorityIndicator;
	BOOL		bICTIExist;
	UCHAR		ucICTI;
}CANDIDATEAPP,*PCANDIDATEAPP/*,ADFENTRY,*PADFENTRY*/;


typedef		struct
{
	APPSELECTMETHOD enAppSelectMethod;
	UCHAR			CandidateNums;
	CANDIDATEAPP	CandidateApp[MAXCANDIDATEAPPNUMS];
	UCHAR			ucSelectedAppIndex;
}CANDIDATELIST,*PCANDIDATELIST;



APPSELECT_DATA CANDIDATELIST 	CandidateList;

UCHAR	APPSELECT_AddCandidateList(PCANDIDATEAPP pApp);
BOOL	APPSELECT_CheckDDFMandaData(void);
BOOL	APPSELECT_CheckPSEMandaData(void);
BOOL	APPSELECT_CheckRecordFound(void);
void 	APPSELECT_EmptyCandidateList(void);
UCHAR	APPSELECT_FinalSelect(UCHAR ucSelectApp,PCANDIDATELIST pCandidateList);
BOOL	APPSELECT_GetADFEntryData(ICDATAINFO *pICDataInfo,EMVTAGINFO	*pTemplate ,ADFENTRY *pADFEntry);
UCHAR	APPSELECT_CreateAppCandidate(PCANDIDATELIST pCandidateList);
UCHAR	APPSELECT_GetAppCandidate(PCANDIDATELIST pCandidateList);
BOOL	APPSELECT_GetDDFEntryData(ICDATAINFO *pICDataInfo,struct EMVTAGDEF *pTemplate ,DDFENTRY *pDDFEntry);
UCHAR	APPSELECT_InterruptCurDIR(PDDFSTACKDATA DDFStackData);
BOOL    APPSELECT_MatchTermAID(PUCHAR pucAID,UCHAR ucAIDLen);
UCHAR	APPSELECT_SelectAppFromAID(PCANDIDATELIST pCandidateList);
UCHAR	APPSELECT_SelectAppFromPSE(PCANDIDATELIST pCandidateList);
void	APPSELECT_SortApp(void);
BOOL	APPSELECT_GetADFFciData(ICDATAINFO *pICDataInfo,EMVTAGINFO	*pTemplate ,ADFFCI *pADFFci);
void	APPSELECT_CopyADFFciData(PCANDIDATEAPP pCandidateApp,ADFFCI *pADFFci);
BOOL	APPSELECT_GetFinFciData(ICDATAINFO *pICDataInfo,EMVTAGINFO	*pTemplate ,ADFFCI *pADFFci);


#ifdef __cplusplus
}
#endif
#endif
