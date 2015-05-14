
#ifndef _MAGCOM_H
#define _MAGCOM_H
#include "ostools.h"
#include "oslib.h"

unsigned char MAGCOM_PORT_PreCom2(void);

unsigned char MAGCOM_SendData(const unsigned char *pucInData,unsigned short uiInLen );

unsigned char MAGCOM_GetIsoTrack(uchar ucLen1,uchar ucStatu1,uchar* puciso1,
                              uchar ucLen2,uchar ucStatu2,uchar* puciso2,
                              uchar ucLen3,uchar ucStatu3,uchar* puciso3);

#endif
