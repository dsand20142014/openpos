

#ifndef		_QDDFSTACK_H
#define		_QDDFSTACK_H

#ifndef		QDDFSTACK_DATA
#define		QDDFSTACK_DATA		extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include	<QTypeDef.h>
#include	<QDef.h>
//#include	<QAppSelect.h>

/*************const define***********/
#define		QMAXENTRYNUM    			20
#define		QMAXSTACKDEPTH			20


/************error define************/
#define		QDDFSTACKERR_SUCCESS		0
#define		QDDFSTACKERR_ISEMPTY		1	
#define		QDDFSTACKERR_ISFULL		2	

/************struct define************/
typedef struct
{
	UCHAR		ucDDFNameLen;
	UCHAR		aucDDFName[QMAXDDFNAMELEN];
	
}QDDFENTRY;


typedef	 struct
{
	UCHAR		ucADFNameLen;
	UCHAR		aucADFName[QMAXADFNAMELEN];
	UCHAR		ucAppLabelLen;
	UCHAR		aucAppLabel[QMAXAPPLABELLEN];	
	UCHAR		ucAppPreferNameLen;
	UCHAR		aucAppPreferName[QMAXAPPPREFERNAMELEN];
	BOOL		bAppPriorityExist;
	UCHAR		ucAppPriorityIndicator;
	
}QADFENTRY;

typedef		struct
{
	UCHAR		ucADFNameLen;
	UCHAR		aucADFName[QMAXADFNAMELEN];
	UCHAR		ucAppLabelLen;
	UCHAR		aucAppLabel[QMAXAPPLABELLEN];	
	BOOL		bAppPriorityExist;
	UCHAR		ucAppPriorityIndicator;
	USHORT		uiPDOLLen;
	UCHAR		aucPDOL[QMAXPDOLLEN];
	BOOL		bLanguagePreferExist;
	UCHAR		ucLanguagePrefer;
	BOOL		bICTIExist;
	UCHAR		ucICTI;
	UCHAR		ucAppPreferNameLen;
	UCHAR		aucAppPreferName[QMAXAPPPREFERNAMELEN];
	BOOL		bLogEntry;
	UCHAR		aucLogEntry[QLOGENTRYLEN];
	
	
}QADFFCI;
typedef struct
{
	BOOL		bEntryFlag;		//TRUE is QDDFENTRY AND FALSE is QADFENTRY
	union QDIRENTRY
	{
		QDDFENTRY	DDFEntry;
		QADFENTRY	ADFEntry;
	}DirEntry;
}QENTRYITEM;

typedef struct
{
	UCHAR		ucDDFNameLen;
	UCHAR		aucDDFName[QMAXDDFNAMELEN];
	UCHAR		ucSFI;
	UCHAR		ucCurRecordNum;
	UCHAR		ucCurEntryNum;
	UCHAR		ucEntryNum;
	QENTRYITEM	EntryItem[QMAXENTRYNUM];
	
}QDDFSTACKDATA,*PQDDFSTACKDATA;

typedef struct
{
	UCHAR		ucStackDataNum;
	QDDFSTACKDATA QDDFStackData[QMAXSTACKDEPTH];
	
}QDDFSTACK,*PQDDFSTACK;



QDDFSTACK_DATA	QDDFSTACK	QDDFStack;

void	QDDFSTACK_Init(void);
BOOL	QDDFSTACK_IsFull(void);
BOOL	QDDFSTACK_IsEmpty(void);
UCHAR	QDDFSTACK_PushData(PQDDFSTACKDATA pQDDFStackData);
UCHAR	QDDFSTACK_PopData(PQDDFSTACKDATA pQDDFStackData);


#ifdef __cplusplus
}
#endif
#endif
