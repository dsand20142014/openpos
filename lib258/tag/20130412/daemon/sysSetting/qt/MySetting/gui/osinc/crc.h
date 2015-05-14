#ifndef _CRC_H_
#define _CRC_H_ 


unsigned long CRC_Calc32(unsigned long ulCRC,const unsigned char *pucInData,unsigned long ulInLen);
unsigned short CRC_Calc16(unsigned short uiCRC,const unsigned char *pucInData,unsigned long ulInLen);

#endif

