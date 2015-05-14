

#ifndef _EMVTAG_H
#define _EMVTAG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EMVTAG
#define EMVTAG extern
#endif

#include	<DataElement.h>
//#define EMVTAG_DEBUG

#define  IsTemplate(x)  		(x & 0x20)
#define  IsRefNextByte(x)  	((x & 0x1F) ==0x1F)
#define  TagByteNums(x)	    IsRefNextByte(x)? 2 : 1
#define	 Is80Template(x)		(x == 0x80)
#define	 Is77Template(x)		(x == 0x77)
#define	 Is70Template(x)		(x == 0x70)

/***********	ERROR MSG	*************/
#define EMVTAGERR_SUCCESS				0
#define EMVTAGERR_OVERFLOW			1
#define EMVTAGERR_PARAM				2
#define EMVTAGERR_TAGNOTEXIST			3
#define EMVTAGERR_DATA					4
#define	EMVTAGERR_PROPERTY			5
#define EMVTAGERR_TAGLEN				6
#define	EMVTAGERR_TAGEXIST			7
#define	EMVTAGERR_DUPLICATE			8

#define EMVTAG_NUMMAX		80
#define EMVTAG_BUFFMAX		1024



struct EMVTAGDEF
{
	unsigned char 	aucTag[2];
	unsigned short 	uiLen;
	unsigned char   *pTagValue;
	struct EMVTAGDEF   *pTemplate;
};
typedef struct EMVTAGDEF  EMVTAGINFO;

typedef struct
{
	
	const DATAPROPERTY 	*pDataProperty;
	BOOL				bTagUniqueFlag;
	unsigned char 		ucTagNum;
	EMVTAGINFO			EMVTagInfo[EMVTAG_NUMMAX];
	unsigned short		uiTagBuffLen;
	unsigned char 		aucEMVTagVaule[EMVTAG_BUFFMAX];   //TAG BUFF 
	unsigned short		uiDataInfoLen;
	unsigned char 		*paucDataInfo;		//DataINFO
	
}ICDATAINFO;
									
EMVTAG ICDATAINFO ICDataInfo;
EMVTAG unsigned char ucEMVDebugFlag; 


unsigned char EMVTAG_INIT(DATAPROPERTY *pDataProperty,BOOL bTagUniqueFlag);
EMVTAGINFO * EMVTAG_SearchTag(unsigned char *paucTag);
unsigned char EMVTAG_SetTagValue(unsigned char *paucTag,
								unsigned char *paucTagVaule,unsigned short uiTagVauleLen);
EMVTAGINFO    *EMVTAG_GetNextTemplate(EMVTAGINFO *pLastTemplate,unsigned char *paucTag);
EMVTAGINFO    *EMVTAG_SearchTemplateTag(EMVTAGINFO *pTemplate,unsigned char *paucTag);
unsigned char EMVTAG_GetTagValue(unsigned char *paucTag,
								unsigned char *paucTagValue,unsigned short* uiTagValueLen);
DATAPROPERTY *EMVTAG_GetTagProperty(DATAPROPERTY *pDataProperty,unsigned char *paucTag);
unsigned char EMVTAG_Pack(PUCHAR 	pDataDest,USHORT *puiDataDestLen);						
unsigned char EMVTAG_NestDeal(EMVTAGINFO *pTemplate,unsigned char *pucTreatPos,unsigned short uiTreatLen);
unsigned char EMVTAG_Unpack(PUCHAR pDataSrc,USHORT uiDataLen);
unsigned char EMVTAG_GetTagLenAttr(unsigned char *paucTagData,unsigned short* puiTagLen);
unsigned char EMVTAG_SetTagLenAttr(unsigned short* puiTagLen,unsigned char *paucTagData);
EMVTAGINFO    *EMVTAG_GetTemplateTag(EMVTAGINFO *pTemplate,UCHAR ucIndex);
BOOL	EMVTAG_IsExclusive(unsigned char *paucTag);
void EMVTAG_DataDump();
void EMVTAG_SetbDEBUGValue(unsigned char bDEBUGFlag);

#ifdef __cplusplus
}
#endif

#endif
