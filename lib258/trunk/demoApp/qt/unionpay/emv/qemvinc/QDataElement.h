

#ifndef			_QDATAELEMENT_H
#define			_QDATAELEMENT_H

#ifndef			QDATAELEMENT_DATA
#define			QDATAELEMENT_DATA		extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include		<QTypeDef.h>

/*************const define***********/
#define QATTR_A				0x01
#define QATTR_AN				0x02
#define QATTR_ANS			0x03
#define QATTR_BIN			0x04
#define QATTR_CN				0x05     //Left justified and padded with tailing hexadecimal 'F's.
#define QATTR_N				0x06     //right justified and padded with leading hexadecimal zeroes.
#define QATTR_VAR            0x07
#define QATTR_OR				0x08     



typedef		struct
{
	UCHAR		aucTag[2];
	UCHAR		ucTagAttr;
	USHORT		uiMinTagLengh;
	USHORT		uiMaxTagLengh;
}QDATAPROPERTY;

typedef		struct
{
	UCHAR		aucTag[2];
}QMANDDATAOBJ;

/*

DATAELEMENT_DATA  QDATAPROPERTY	QTermDataProperty[];
DATAELEMENT_DATA  QDATAPROPERTY	QICCardProperty[];
DATAELEMENT_DATA  QDATAPROPERTY	QIssuerProperty[];
*/

#ifdef __cplusplus
}
#endif
#endif
