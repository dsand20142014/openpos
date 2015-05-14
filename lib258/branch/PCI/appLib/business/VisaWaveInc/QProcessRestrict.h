
#ifndef		_QPROCESSRESTRICT_H
#define		_QPROCESSRESTRICT_H


#ifdef __cplusplus
extern "C" {
#endif

UCHAR	QPROCESSRESTRICT_AppVerNum(void);
UCHAR	QPROCESSRESTRICT_AppUsageControl(void);
UCHAR	QPROCESSRESTRICT_AppExpDateCheck();
UCHAR	QPROCESSRESTRICT_CheckDateValid(UCHAR	aucDate[3]);
#ifdef __cplusplus
}
#endif
#endif
