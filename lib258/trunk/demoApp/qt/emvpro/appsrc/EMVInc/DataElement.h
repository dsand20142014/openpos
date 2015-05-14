

#ifndef			_DATAELEMENT_H
#define			_DATAELEMENT_H

#ifndef			DATAELEMENT_DATA
#define			DATAELEMENT_DATA		extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include		<TypeDef.h>

/*************const define***********/
#define EMVATTR_A				0x01
#define EMVATTR_AN				0x02
#define EMVATTR_ANS			0x03
#define EMVATTR_BIN			0x04
#define EMVATTR_CN				0x05     //Left justified and padded with tailing hexadecimal 'F's.
#define EMVATTR_N				0x06     //right justified and padded with leading hexadecimal zeroes.
#define EMVATTR_VAR            0x07
#define EMVATTR_OR				0x08     



typedef		struct
{
	UCHAR		aucTag[2];
	UCHAR		ucTagAttr;
	USHORT		uiMinTagLengh;
	USHORT		uiMaxTagLengh;
}DATAPROPERTY;

typedef		struct
{
	UCHAR		aucTag[2];
}MANDDATAOBJ;
/*

DATAELEMENT_DATA  DATAPROPERTY	TermDataProperty[];
DATAELEMENT_DATA  DATAPROPERTY	ICCardProperty[];
DATAELEMENT_DATA  DATAPROPERTY	IssuerProperty[];
*/

#ifdef __cplusplus
}
#endif
#endif
