/*
appinc/osutil.h
*/




#ifndef __OSUTIL_H
#define __OSUTIL_H

#ifdef __cplusplus
extern "C" {
#endif


unsigned char OSUTIL_CheckIPAddrField(
                unsigned char *pucIP,
                unsigned char ucIPLen,
                unsigned char *pucIPField);

unsigned char OSUTIL_WaitKey(unsigned char ucFlag,unsigned int uiTimeout);


unsigned char OSUTIL_Input(
                unsigned int uiTimeout,
                unsigned char ucFont,
                unsigned char ucLine,
                unsigned char ucMin,
                unsigned char ucMax,
                unsigned char ucType,
                unsigned char *pucBuf,
                char *pcMask);

unsigned char OSUTIL_InputInsertChar(
                unsigned int uiTimeout,
                unsigned char ucFont,
                unsigned char ucRow,
                unsigned char ucCharNB,
                unsigned char ucInsertChar,
                unsigned char ucStep,
                unsigned char *pucBuf);
unsigned char OSUTIL_InputIPV4Addr(
                unsigned int uiTimeout,
                unsigned char ucFont,
                unsigned char ucRow,
                unsigned char *pucIPV4Addr);

unsigned char OSUTIL_CheckIPAddrField(
                unsigned char *pucIP,
                unsigned char ucIPLen,
                unsigned char *pucIPField);

unsigned char OSUTIL_FullInput( unsigned char ucFont,
                                unsigned char ucLine,
                                unsigned char ucMin,
                                unsigned char ucMax,
                                unsigned char *pucBuf);

									                						                

#ifdef __cplusplus
}
#endif

#endif