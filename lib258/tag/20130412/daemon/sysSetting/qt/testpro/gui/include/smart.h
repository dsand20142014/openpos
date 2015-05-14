/*
修改原因见sandos_def.h
*/
#ifndef _SMART_H__
#define _SMART_H__

#ifdef __cplusplus 
extern "C" { 
#endif

//#include "sandos_def.h"
/*****************************************************************************
* come from smart.h
*****************************************************************************/
/* Smart Card */
#define SMART_OK				0		/* Return Code Ok  0x00 */
#define SMART_ASYNC				1       /* Synchronous Ok  0x01 */
#define SMART_SYNC				2       /* Asynchronous Ok 0x02 */
#define SMART_DRIVER_ERROR		3       /* Driver Error    0x03 */
#define SMART_CARD_ERROR		4       /* Card Error      0x04 */
#define SMART_OWERFLOW			5       /* Owerflow Buffer 0x05 */
#define SMART_ABORT				6       /* Abort Key       0x06 */
#define SMART_ABSENT			7		/* Smart Absent    0x07 */
#define SMART_PRESENT           8		/* Smart Present   0x08 */


/* Smart Async Order Type */
#define TYPE0					0		/* Order Type0 LgIn,BufIn                     */
#define TYPE1					1		/* Order Type1 CLA,INS,P1,P2                  */
#define TYPE2					2		/* Order Type2 CLA,INS,P1,P2,LgOut            */
#define TYPE3					3		/* Order Type3 CLA,INS,P1,P2,LgIn,BufIn       */
#define TYPE4					4		/* Order Type4 CLA,INS,P1,P2,LgIn,BufIn,LgOut */

typedef struct
{
    unsigned char ucCla;
    unsigned char ucIns;
    unsigned char ucP1;
    unsigned char ucP2;
    unsigned char ucP3;
    unsigned char aucData[128];
}ISO7816IN;

typedef struct
{
    unsigned short	uiLen;
    unsigned char	ucStatus;
    unsigned char	ucSW1;
    unsigned char	ucSW2;
    unsigned char	aucData[255];
}ISO7816OUT;


unsigned char SMART_Reset(unsigned char ucReader,unsigned char *pucReset,
                          unsigned char *pucLen,unsigned char ucRate);
unsigned char SMART_Detect(unsigned char ucReader);
unsigned char SMART_SelectProtocol(
    unsigned char ucReader,
    unsigned char ucProtocol);
unsigned char SMART_ISO(
    unsigned char ucReader,
    unsigned char ucCardType,
    unsigned char* pucIn,
    unsigned char* pucOut,
    unsigned short* puiLengthOut,
    unsigned char ucOrderType);
unsigned char SMART_PowerOff (unsigned char ucReader);
unsigned char SMART_Remove(unsigned char ucReader);

#ifdef __cplusplus 
}
#endif


#endif
//EOF
