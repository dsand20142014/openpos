/*
	Library Function
--------------------------------------------------------------------------
	FILE  tools.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-22		Xiaoxi Jiang
    Last modified :	2002-10-22		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef _TOOLS_H
#define _TOOLS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define OS_ERRMASK					0x08000000

/* date.c */
#define DATE_DATESTRLEN				8
#define DATE_TIMESTRLEN				8
#define DATE_DATETIMESTRLEN			16
#define DATE_YEARSTRLEN				4
#define DATE_MONTHSTRLEN			2
#define DATE_DAYSTRLEN				2

#define DATE_SUCCESS				0
#define DATE_ERRMASK				0xA0

#define DATE_PARAM_ERR				DATE_ERRMASK | 0x01
#define DATE_NOTLEAPYEAR_ERR		DATE_ERRMASK | 0x02
#define DATE_INVALIDCHAR_ERR		DATE_ERRMASK | 0x03
#define DATE_INVALIDYEAR_ERR		DATE_ERRMASK | 0x04
#define DATE_INVALIDMONTH_ERR		DATE_ERRMASK | 0x05
#define DATE_INVALIDDAY_ERR			DATE_ERRMASK | 0x06
#define DATE_INVALIDHOUR_ERR        DATE_ERRMASK | 0x07
#define DATE_INVALIDMIN_ERR         DATE_ERRMASK | 0x08
#define DATE_INVALIDSEC_ERR         DATE_ERRMASK | 0x09
#define DATE_OVERBOUND				DATE_ERRMASK | 0x0A

#define TLVERR_SUCCESS              0x00
#define TLVERR_INVALIDPARAM         0x01
#define TLVERR_TAGNOTFOUND          0x02

/* Macro */
#define min(a, b)           (((a) < (b)) ? (a) : (b))
#define max(a, b)           (((a) > (b)) ? (a) : (b))

/* tlv.c */
typedef struct
{
    unsigned short uiTag;
    unsigned int uiLen;
    unsigned char *pucPtr;
}TLV;

/* conv.c */
void *CONV_AscBcd(unsigned char *pucDest, unsigned int uiDestLen,
				unsigned char *pucSrc, unsigned int uiSrcLen);
void *CONV_AscHex(unsigned char *pucDest, unsigned int uiDestLen,
				unsigned char *pucSrc, unsigned int uiSrcLen);
unsigned long CONV_AscLong(unsigned char *pucSrc,unsigned int iLen);
void *CONV_BcdAsc(unsigned char *pucDest,unsigned char *pucSrc,int uiDestLen);
unsigned long CONV_BcdLong(unsigned char *pucSrc,unsigned int uiSrcLen);
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
void *CONV_HexStr(unsigned char *pucDest,unsigned char *pucSrc,unsigned int uiLen);
unsigned long CONV_HexLong(unsigned char *pucSrc,unsigned int uiLen);
void *CONV_IntAsc(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc);
void *CONV_IntBcd(unsigned char *pucDest, unsigned int uiLen,unsigned int *puiSrc);
void *CONV_IntHex(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc);
void *CONV_IntStr(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc);
void *CONV_LongAsc(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
void *CONV_LongBcd(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
void *CONV_LongHex(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
void *CONV_LongStr(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
void *CONV_ShortAsc(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);
void *CONV_ShortBcd(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);
void *CONV_ShortHex(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);
void *CONV_ShortStr(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);
void *CONV_StrBcd(unsigned char *pucDest,unsigned int uiDestLen,unsigned char *pucSrc);
void *CONV_StrHex(unsigned char *pucDest,unsigned int uiDestLen,unsigned char *pucSrc);
unsigned long CONV_StrLong(unsigned char *pucSrc);
void CONV_StrLowCase(char *pcStr);
void CONV_StrTrimRight(char *pcStr,char cCh);
void CONV_StrTrimLeft(char *pcStr,char cCh);

/* crc.c */
unsigned short CRC_CalcSum16(unsigned char *pucStart,unsigned char *pucEnd,unsigned short uiCRC );
unsigned short CRC_Calc16(unsigned short uiCRC,const unsigned char *pucInData,unsigned long ulInLen);
unsigned long CRC_Calc32(unsigned long ulCRC,const unsigned char *pucInData,unsigned long ulInLen);

/* date.c */
long DATE_LeapYearStr(char *pcDate);
long DATE_LeapYearInt(int iYear);
long DATE_ValideDateStr(char *pcDate);
long DATE_ValideDateInt(int iYear,int iMonth,int iDay);
long DATE_ValideTimeStr(char *pcTime);
long DATE_ValideTimeInt(int iHour,int iMin,int iSec);
long DATE_ConvStrInt(char *pcDate,int *piYear,int *piMonth,int *piDay);
long DATE_ConvIntStr(int iYear,int iMonth,int iDay,char *pcDate);
long DATE_ConvStrLong(char *pcDate,long *plDate);
long DATE_ConvLongStr(long lDate,char *pcDate);
long DATE_ConvDateNameStr(char *pcDate1,char *pcDate2);
long DATE_MonthDaysStr(char *pcDate, int *piDays );
long DATE_MonthDaysInt(int iYear,int iMonth, int *piDays );
long DATE_YearDaysStr(char *pcDate,int *piDays );
long DATE_YearDaysInt(int iYear,int *piDays );
long DATE_OneYearDaysStr(char *pcDate,int *piDays );
long DATE_DiffDaysStr(char *pcDate1,char *pcDate2,long *plDays);
long DATE_WeekDayStr(char *pcDate,int *piWeekDay);
long DATE_YearChangeStr(char *pcDate,int iYear);
long DATE_MonthChangeStr(char *pcDate,int iMonthChange);
long DATE_DayChangeStr(char *pcDate,long lDayChange);

/* trim.c */
void TRIM_StrRight(char *pcStr,char cCh);
void TRIM_StrLeft(char *pcStr,char cCh);
char *TRIM_StrRightSpace(char *pcStr);

/* md5.c */
void MD5_Compute(unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucDigest);

/* misc.c */
unsigned char MISC_GetCharBitNB(unsigned char ucCh,unsigned char ucBit);

/* sha1.c */
void SHA1_Compute(unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucDigest);

/* tlv.c */
unsigned char TLV_DecodeSAND(unsigned char *pucBuf,unsigned int uiLen,
                        TLV *pTLV,unsigned short *puiNbr);
unsigned char TLV_AccessTagSAND(unsigned char *pucInData,unsigned int uiInLen,
                        unsigned short uiTag,TLV *pTLV);

/* zip.c */
int ZIP_NRV2DDecompress(const unsigned char *pucSrc, unsigned int  uiSrcLen,
                        unsigned char *pucDest, unsigned int *puiDestLen);
/* libdes.c */
void des(unsigned char *binput, unsigned char *boutput, unsigned char *bkey);

#ifdef __cplusplus
}
#endif
#endif

