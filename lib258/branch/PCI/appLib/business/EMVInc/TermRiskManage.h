

#ifndef		_TERMRISKMANAGE_H
#define		_TERMRISKMANAGE_H

#ifndef		TERMRISKMANAGE_DATA
#define		TERMRISKMANAGE_DATA	extern
#endif

#include	<TypeDef.h>
#include	<EMVDef.h>


#ifdef		__cplusplus
extern	"C"
{
#endif

typedef struct 
{
	UCHAR		aucPAN[MAXPANDATALEN];
	UCHAR		ucPANSeq;
}EXCEPTPAN,*PEXCEPTPAN;

UCHAR	TERMRISKMANAGE_CheckFloorLimits(UINT uiLogSumAmount);
UCHAR	TERMRISKMANAGE_RandomSelect(void);
UCHAR	TERMRISKMANAGE_CheckVelocity(void);
UCHAR	TERMRISKMANAGE_Process(UINT uiLogSumAmount);
UCHAR	TERMRISKMANAGE_CheckExceptFile(void);
void	TERMSETTING_LoadDefaultSetting(void);

#ifdef		__cplusplus
}
#endif

#endif
