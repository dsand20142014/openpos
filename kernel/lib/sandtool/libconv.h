#ifndef _LINUX_LIBCONV_H_
#define _LINUX_LIBCONV_H_

/* We don't want strings.h stuff being user by user stuff by accident */

#ifdef __KERNEL__

#include <linux/compiler.h>	/* for inline */
#include <linux/types.h>	/* for size_t */
#include <linux/stddef.h>	/* for NULL */

#ifdef __cplusplus
extern "C" {
#endif


void *CONV_AscBcd(unsigned char *pucDest,unsigned int uiDestLen,
				unsigned char *pucSrc,unsigned int uiSrcLen);

void *CONV_AscHex(unsigned char *pucDest,unsigned int uiDestLen,
				unsigned char *pucSrc,unsigned int uiSrcLen);

unsigned long CONV_AscLong(unsigned char *pucSrc,unsigned int uiLen);

void *CONV_BcdAsc(unsigned char *pucDest,unsigned char *pucSrc, int uiDestLen);
unsigned long CONV_BcdLong(unsigned char *pucSrc, unsigned int uiSrcLen);

void *CONV_BcdStr(unsigned char *pucDest,unsigned char *pucSrc,int uiDestLen);
unsigned char CONV_BitStrCharBit(unsigned char *pucSrc);

void *CONV_CharAsc(unsigned char *pucDest,unsigned char ucLen,unsigned char *pucSrc);
void *CONV_CharBcd(unsigned char *pucDest,unsigned char ucLen,unsigned char *pucSrc);
void CONV_CharBitStr(unsigned char *pucDest,unsigned char ucCh);
unsigned char CONV_CharHex(unsigned char ucCh);
void *CONV_CharStr(unsigned char *pucDest,unsigned char ucLen,unsigned char *pucSrc);
unsigned char CONV_CharLowCase(unsigned char ucCh);
unsigned char CONV_CharUpperCase(unsigned char ucCh);
void *CONV_HexAsc(unsigned char *pucDest,unsigned char *pucSrc,unsigned int uiLen);
unsigned long CONV_HexLong(unsigned char *pucSrc,unsigned int uiLen);
void *CONV_HexStr(unsigned char *pucDest,unsigned char *pucSrc,unsigned int uiLen);
void *CONV_IntAsc(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc);
void *CONV_IntBcd(unsigned char *pucDest, unsigned int uiLen,unsigned int *puiSrc);
void *CONV_IntHex(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc);
void *CONV_IntStr(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc);

void *CONV_LongAsc( unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc );
void *CONV_LongBcd(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
void *CONV_LongHex(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
void *CONV_LongStr(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc );
void *CONV_ShortAsc(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);

void *CONV_ShortBcd(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);
void *CONV_ShortHex(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);
void *CONV_ShortStr(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);

void *CONV_StrBcd(unsigned char *pucDest,unsigned int uiDestLen,unsigned char *pucSrc);
void *CONV_StrHex(unsigned char *pucDest,unsigned int uiDestLen,unsigned char *pucSrc);
unsigned long CONV_StrLong(unsigned char *pucSrc);
void CONV_StrLowCase(char *pcStr);
void CONV_StrUpperCase(char *pcStr);
void CONV_StrTrimRight(char *pcStr,char cCh);
void CONV_StrTrimLeft(char *pcStr,char cCh);
#ifdef __cplusplus
}
#endif

#endif
#endif /* _LINUX_STRING_H_ */

