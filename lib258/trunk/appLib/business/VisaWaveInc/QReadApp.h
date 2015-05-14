
#ifndef		_QREADAPP_H
#define		_QREADAPP_H

#ifndef		QREADAPP_DATA
#define		QREADAPP_DATA	extern
#endif
#include 	<QTypeDef.h>
#ifdef __cplusplus
extern "C" {
#endif

UCHAR	QREADAPP_ReadData(void);
UCHAR	QREADAPP_CheckIccMandData(void);

#ifdef __cplusplus
}
#endif
#endif
