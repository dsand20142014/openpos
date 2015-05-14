
#ifndef __SANDOS_DEF_H__
#define __SANDOS_DEF_H__

#define ZY_TODO Uart_Printf

/*****************************************************************************
* come from oslib.h
*****************************************************************************/
/* File API definement */
#define O_READ              0x01
#define O_WRITE             0x02
#define O_CREATE            0x04

#ifdef O_TRUNC
#undef O_TRUNC	//
#define O_TRUNC             0x08
#endif

#ifndef SEEK_SET
#define SEEK_SET	        0x00
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	        0x01
#endif
#ifndef SEEK_END
#define SEEK_END	        0x02
#endif

#define	FILE_INVMODE		0x01
#define	FILE_NOEXIST		0x02
#define	FILE_EXIST          0x03
#define	FILE_FULL			0x04
#define	FILE_NOSPACE		0x05
#define	FILE_MUCHHALDLE		0x06
/*End of  File API definement */



#if 0
/*
changed by zhangyong 0720
因为与应用程序空间有重复定义，故改成以下结构内容只在相关文件中定义，而不再是全局性的
*/

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
#endif






#endif
