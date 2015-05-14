

#ifndef _QTAG_H
#define _QTAG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef QTAG
#define QTAG extern
#endif

#include	<QDataElement.h>
//#define QTAG_DEBUG

#define  IsTemplate(x)  		(x & 0x20)
#define  IsRefNextByte(x)  	((x & 0x1F) ==0x1F)
#define  TagByteNums(x)	    IsRefNextByte(x)? 2 : 1
#define	 Is80Template(x)		(x == 0x80)
#define	 Is77Template(x)		(x == 0x77)
#define	 Is70Template(x)		(x == 0x70)

/***********	ERROR MSG	*************/
#define QTAGERR_SUCCESS				0
#define QTAGERR_OVERFLOW			1
#define QTAGERR_PARAM				2
#define QTAGERR_TAGNOTEXIST			3
#define QTAGERR_DATA					4
#define	QTAGERR_PROPERTY			5
#define QTAGERR_TAGLEN				6
#define	QTAGERR_TAGEXIST			7
#define	QTAGERR_DUPLICATE			8

#define QTAG_NUMMAX		40
#define QTAG_BUFFMAX		1024



struct QTAGDEF
{
	unsigned char 	aucTag[2];
	unsigned short 	uiLen;
	unsigned char   *pTagValue;
	struct QTAGDEF   *pTemplate;
};
typedef struct QTAGDEF  QTAGINFO;

typedef struct
{
	
	const QDATAPROPERTY 	*pDataProperty;
	BOOL				bTagUniqueFlag;
	unsigned char 		ucTagNum;
	QTAGINFO			EMVTagInfo[QTAG_NUMMAX];
	unsigned short		uiTagBuffLen;
	unsigned char 		aucEMVTagVaule[QTAG_BUFFMAX];   //TAG BUFF 
	unsigned short		uiDataInfoLen;
	unsigned char 		*paucDataInfo;		//DataINFO
	
}QICDATAINFO;
									
QTAG QICDATAINFO QICDataInfo;


unsigned char QTAG_INIT(QDATAPROPERTY *pDataProperty,BOOL bTagUniqueFlag);
QTAGINFO * QTAG_SearchTag(unsigned char *paucTag);
unsigned char QTAG_SetTagValue(unsigned char *paucTag,
								unsigned char *paucTagVaule,unsigned short uiTagVauleLen);
QTAGINFO    *QTAG_GetNextTemplate(QTAGINFO *pLastTemplate,unsigned char *paucTag);
QTAGINFO    *QTAG_SearchTemplateTag(QTAGINFO *pTemplate,unsigned char *paucTag);
unsigned char QTAG_GetTagValue(unsigned char *paucTag,
								unsigned char *paucTagValue,unsigned short* uiTagValueLen);
QDATAPROPERTY *QTAG_GetTagProperty(QDATAPROPERTY *pDataProperty,unsigned char *paucTag);
unsigned char QTAG_Pack(PUCHAR 	pDataDest,USHORT *puiDataDestLen);						
unsigned char QTAG_NestDeal(QTAGINFO *pTemplate,unsigned char *pucTreatPos,unsigned short uiTreatLen);
unsigned char QTAG_Unpack(PUCHAR pDataSrc,USHORT uiDataLen);
unsigned char QTAG_GetTagLenAttr(unsigned char *paucTagData,unsigned short* puiTagLen);
unsigned char QTAG_SetTagLenAttr(unsigned short* puiTagLen,unsigned char *paucTagData);
QTAGINFO    *QTAG_GetTemplateTag(QTAGINFO *pTemplate,UCHAR ucIndex);
BOOL	QTAG_IsExclusive(unsigned char *paucTag);
BOOL	QTAG_CheckTagInDOL(PUCHAR pDOL,UCHAR ucDOLLen,PUCHAR pTag);
void QTAG_DataDump();

#ifdef __cplusplus
}
#endif

#endif
