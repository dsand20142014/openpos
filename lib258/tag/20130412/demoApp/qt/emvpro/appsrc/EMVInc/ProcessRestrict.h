
#ifndef		_PROCESSRESTRICT_H
#define		_PROCESSRESTRICT_H


#ifdef __cplusplus
extern "C" {
#endif

UCHAR	PROCESSRESTRICT_AppVerNum(void);
UCHAR	PROCESSRESTRICT_AppUsageControl(void);
UCHAR	PROCESSRESTRICT_AppExpDateCheck();
UCHAR	PROCESSRESTRICT_CheckDateValid(UCHAR	aucDate[3]);
#ifdef __cplusplus
}
#endif
#endif
