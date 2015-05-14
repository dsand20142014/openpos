
#ifndef		_QVSDC_H
#define		_QVSDC_H

#include 	<QTypeDef.h>


#ifndef		QVSDC_DATA
#define		QVSDC_DATA		extern
#endif

#ifdef		__cplusplus
extern	"C"
{
#endif



#define			fDDA_DDOL_VER00			"\x9F\x37\x04"
#define			fDDA_DDOL_VER01			"\x9F\x37\x04\x9F\x02\x06\x5F\x2A\x02"

UCHAR	QVSDC_PreTransProcess(ULONG	ulTransAmount);
UCHAR	QVSDC_ProcessFlow(void);
UCHAR	QVSDC_ReaderOfflineProcess(void);
#ifdef		__cplusplus
}
#endif

#endif
