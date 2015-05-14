

#ifndef		_DDFSTACK_H
#define		_DDFSTACK_H

#ifndef		DDFSTACK_DATA
#define		DDFSTACK_DATA		extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include	<TypeDef.h>
#include	<EMVDef.h>
//#include	<AppSelect.h>

/*************const define***********/
#define		MAXENTRYNUM    			20
#define		MAXSTACKDEPTH			20


/************error define************/
#define		DDFSTACKERR_SUCCESS		0
#define		DDFSTACKERR_ISEMPTY		1	
#define		DDFSTACKERR_ISFULL		2	

/************struct define************/
typedef struct
{
	UCHAR		ucDDFNameLen;
	UCHAR		aucDDFName[MAXDDFNAMELEN];
	
}DDFENTRY;


typedef	 struct
{
	UCHAR		ucADFNameLen;
	UCHAR		aucADFName[MAXADFNAMELEN];
	UCHAR		ucAppLabelLen;
	UCHAR		aucAppLabel[MAXAPPLABELLEN];	
	UCHAR		ucAppPreferNameLen;
	UCHAR		aucAppPreferName[MAXAPPPREFERNAMELEN];
	BOOL		bAppPriorityExist;
	UCHAR		ucAppPriorityIndicator;
	
}ADFENTRY;

typedef		struct
{
	UCHAR		ucADFNameLen;
	UCHAR		aucADFName[MAXADFNAMELEN];
	UCHAR		ucAppLabelLen;
	UCHAR		aucAppLabel[MAXAPPLABELLEN];	
	BOOL		bAppPriorityExist;
	UCHAR		ucAppPriorityIndicator;
	USHORT		uiPDOLLen;
	UCHAR		aucPDOL[MAXPDOLLEN];
	BOOL		bLanguagePreferExist;
	UCHAR		aucLanguagePrefer[LANGUAGEPREFERMAXLEN];
	BOOL		bICTIExist;
	UCHAR		ucICTI;
	UCHAR		ucAppPreferNameLen;
	UCHAR		aucAppPreferName[MAXAPPPREFERNAMELEN];
	BOOL		bLogEntry;
	UCHAR		aucLogEntry[LOGENTRYLEN];
	
	
}ADFFCI;
typedef struct
{
	BOOL		bEntryFlag;		//TRUE is DDFENTRY AND FALSE is ADFENTRY
	union DIRENTRY
	{
		DDFENTRY	DDFEntry;
		ADFENTRY	ADFEntry;
	}DirEntry;
}ENTRYITEM;

typedef struct
{
	UCHAR		ucDDFNameLen;
	UCHAR		aucDDFName[MAXDDFNAMELEN];
	UCHAR		ucSFI;
	UCHAR		ucCurRecordNum;
	UCHAR		ucCurEntryNum;
	UCHAR		ucEntryNum;
	ENTRYITEM	EntryItem[MAXENTRYNUM];
	
}DDFSTACKDATA,*PDDFSTACKDATA;

typedef struct
{
	UCHAR		ucStackDataNum;
	DDFSTACKDATA DDFStackData[MAXSTACKDEPTH];
	
}DDFSTACK,*PDDFSTACK;



DDFSTACK_DATA	DDFSTACK	DDFStack;

void	DDFSTACK_Init(void);
BOOL	DDFSTACK_IsFull(void);
BOOL	DDFSTACK_IsEmpty(void);
UCHAR	DDFSTACK_PushData(PDDFSTACKDATA pDDFStackData);
UCHAR	DDFSTACK_PopData(PDDFSTACKDATA pDDFStackData);


#ifdef __cplusplus
}
#endif
#endif
