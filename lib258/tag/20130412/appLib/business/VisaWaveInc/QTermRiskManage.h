

#ifndef		_QTERMRISKMANAGE_H
#define		_QTERMRISKMANAGE_H

#ifndef		QTERMRISKMANAGE_DATA
#define		QTERMRISKMANAGE_DATA	extern
#endif

#include	<QTypeDef.h>
#include	<QDef.h>


#ifdef		__cplusplus
extern	"C"
{
#endif

typedef struct 
{
	UCHAR		aucPAN[QMAXPANDATALEN];
	UCHAR		ucPANSeq;
}QEXCEPTPAN,*PQEXCEPTPAN;

UCHAR	QTERMRISKMANAGE_CheckFloorLimits(UINT uiLogSumAmount);
UCHAR	QTERMRISKMANAGE_RandomSelect(void);
UCHAR	QTERMRISKMANAGE_CheckVelocity(void);
UCHAR	QTERMRISKMANAGE_Process(UINT uiLogSumAmount);
UCHAR	QTERMRISKMANAGE_CheckExceptFile(void);
void	TERMSETTING_LoadDefaultSetting(void);

#ifdef		__cplusplus
}
#endif

#endif
