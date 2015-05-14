
#ifndef		_READAPP_H
#define		_READAPP_H

#ifndef		READAPP_DATA
#define		READAPP_DATA	extern
#endif
#ifdef __cplusplus
extern "C" {
#endif

UCHAR	READAPP_ReadData(void);
UCHAR	READAPP_CheckIccMandData(void);
UCHAR	READAPP_PANIdentical(void);
unsigned char READAPP_ECData(void);

#ifdef __cplusplus
}
#endif
#endif
