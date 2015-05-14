
#ifndef		_QAPPSELECT_H
#define		_QAPPSELECT_H

#ifndef		QAPPSELECT_DATA
#define		QAPPSELECT_DATA		extern
#endif


#include	<QDef.h>
#include	<QTypeDef.h>
#include	<QTag.h>
#include	<QDDFStack.h>
#include	<QData.h>


#ifdef __cplusplus
extern "C" {
#endif

/************error define************/
#define     QAPPSELECTERR_SUCCESS				0x00
#define		QAPPSELECTERR_OVERFLOW				0x01	
#define		QAPPSELECTERR_DATA					0x02


typedef enum	 {QAPPSELECTBYPSE,QAPPSELECTBYTERMAPP} QAPPSELECTMETHOD;

typedef struct
{
	BOOL		bRCFLExist;				//Reader Contactless Floor Limit Exist Flag
	ULONG		ulRCFLimit;				//Reader Contactless Floor Limit
	BOOL		bRCTLExist;				//Reader Contactless Transaction Limit  Exist Flag
	ULONG		ulRCTLimit;				//Reader Contactless Transaction Limit
	BOOL		bRCVReqLExist;				//Reader CVM Required Limit  Exist Flag
	ULONG		ulRCVMReqLimit;			//Reader CVM Required Limit
	UCHAR		aucTermTransQ[4];			//Terminal Transaction Qualifiers
//	ULONG		ulVLPAFunds;				//VLP Available Funds
//	ULONG		ulVLPFLimit;				//VLP Funds Limit
//	ULONG		ulVLPRThreshold;			//VLP Reset Threshold
//     ULONG		ulVLPSTransLimit;			//VLP Single Transaction Limit
	BOOL		bStatusCheck;				//Status Check Flag
}QREADERPARAM;

typedef 	struct 
{
	UCHAR	ucASI;						//Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
	UCHAR 	ucAIDLen;
	UCHAR 	aucAID[QMAXAIDLEN];			//5-16
	/*
	UCHAR 	ucbLocalName;				//If display app list using local language considerless of info in card.0-use card info;1-use local language.
	UCHAR 	ucAppLocalNameLen;
	UCHAR 	aucAppLocalName[QMAXLOACALNAMELEN];
	*/
	QREADERPARAM	QReaderParam;	
}QTERMSUPPORTAPP,*PQTERMSUPPORTAPP;

typedef		struct
{
	UCHAR		ucADFNameLen;
	UCHAR		aucADFName[QMAXADFNAMELEN];
	UCHAR		ucAppLabelLen;
	UCHAR		aucAppLabel[QMAXAPPLABELLEN];	
	UCHAR		ucAppPreferNameLen;
	UCHAR		aucAppPreferName[QMAXAPPPREFERNAMELEN];
	BOOL		bAppPriorityExist;
	UCHAR		ucAppPriorityIndicator;
	BOOL		bICTIExist;
	UCHAR		ucICTI;
	QREADERPARAM	*pQReaderParam;
}QCANDIDATEAPP,*PQCANDIDATEAPP/*,QADFENTRY,*PADFENTRY*/;


typedef		struct
{
	QAPPSELECTMETHOD enAppSelectMethod;
	UCHAR			CandidateNums;
	QCANDIDATEAPP	CandidateApp[QMAXCANDIDATEAPPNUMS];
	UCHAR			ucSelectedAppIndex;
	
}QCANDIDATELIST,*PQCANDIDATELIST;



QAPPSELECT_DATA QCANDIDATELIST 	QCandidateList;

UCHAR	QAPPSELECT_AddCandidateList(PQCANDIDATEAPP pApp);
BOOL	QAPPSELECT_CheckDDFMandaData(void);
BOOL	QAPPSELECT_CheckPSEMandaData(void);
BOOL	QAPPSELECT_CheckRecordFound(void);
void 	QAPPSELECT_EmptyCandidateList(void);
UCHAR	QAPPSELECT_FinalSelect(UCHAR ucSelectApp,PQCANDIDATELIST pCandidateList);
BOOL	QAPPSELECT_GetADFEntryData(QICDATAINFO *pQICDataInfo,QTAGINFO	*pTemplate ,QADFENTRY *pADFEntry);
UCHAR	QAPPSELECT_CreateAppCandidate(PQCANDIDATELIST pCandidateList);
UCHAR	QAPPSELECT_GetAppCandidate(PQCANDIDATELIST pCandidateList);
BOOL	QAPPSELECT_GetDDFEntryData(QICDATAINFO *pQICDataInfo,struct QTAGDEF *pTemplate ,QDDFENTRY *pDDFEntry);
UCHAR	QAPPSELECT_InterruptCurDIR(PQDDFSTACKDATA QDDFStackData);
UCHAR	QAPPSELECT_MatchTermAID(PUCHAR pucAID,UCHAR ucAIDLen);
UCHAR	QAPPSELECT_SelectAppFromAID(PQCANDIDATELIST pCandidateList);
UCHAR	QAPPSELECT_SelectAppFromPPSE(PQCANDIDATELIST pCandidateList);
void	QAPPSELECT_SortApp(void);
BOOL	QAPPSELECT_GetADFFciData(QICDATAINFO *pQICDataInfo,QTAGINFO	*pTemplate ,QADFFCI *pADFFci);
void	QAPPSELECT_CopyADFFciData(PQCANDIDATEAPP pCandidateApp,QADFFCI *pADFFci);


#ifdef __cplusplus
}
#endif
#endif
