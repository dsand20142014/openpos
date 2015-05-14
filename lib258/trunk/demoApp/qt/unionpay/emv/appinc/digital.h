
#ifndef		_DIGITAL_H
#define		_DIGITAL_H

#ifndef		DIGITAL_DATA
#define		DIGITAL_DATA	extern
#endif
#include 	<TypeDef.h>
#include	<EMVIcc.h>
#include    <osmodem.h>
#include	<global.h>
#ifdef		__cplusplus
extern	"C"
{
#endif
//#define			DIGITAL_DEBUG

#define			CARDNO_MAXLEN			40
#define STX		0x02
#define ETX		0x03


#define CMD_Analogue	0xff
#define CMD_CatOn		0x00
#define CMD_CatOff		0x10
#define CMD_Reset		0x01
#define CMD_WUPA		0x02
#define CMD_RECA		0x03
#define CMD_HLTA		0x04
#define CMD_WUPB		0x05
#define CMD_RECB		0x06
#define CMD_HLTB		0x07
#define CMD_Poling		0x08
#define CMD_WUPTOTAT	0x09
#define CMD_WUPTOATTR	0x0a
#define CMD_Digital	0x0b
#define	CMD_Remove		0x0c
#define	CMD_DeSelect	0x0d
#define CMD_Light		0x0e
#define	CMD_Beep		0x0f
#define	CMD_InputPIn	0x10

#define	LIGHT_READ			0x02
#define	LIGHT_APPROVED		0x04
#define	LIGHT_DECLINED		0x08
#define	LIGHT_TERMINATED	0x0C
#define	LIGHT_CLEAR		0x00

#define	BEEP_APPROVED		0x01
#define	BEEP_DECLINED		0x02
#define	BEEP_TERMINATED	0x04
typedef		struct
{
	UCHAR		ucCardType;
	UCHAR		ucReqa0;
	UCHAR		ucReqa1;
	UCHAR		ucCardNoLen;
	UCHAR		aucCardNo[CARDNO_MAXLEN];
}POLINGDATA;


UCHAR	DIGITAL_OpenSerial(void);
UCHAR DIGITAL_ComClose(void);
void	DIGITAL_SendData(PUCHAR pucSend,USHORT uiSendLen);
UCHAR DIGITAL_ReceiveByte(PUCHAR pucByte,USHORT uiTimeout);
UCHAR	DIGITAL_RecvData(UCHAR bCheck ,UINT uiTimeout);
UCHAR	DIGITAL_Poling(void);
UCHAR	DIGITAL_Active(void);
#if SANDREADER
extern UCHAR	QICC(QICCIN *pQICCIn ,QICCOUT  *pQICCOut);
#endif
void DIGITAL_Package(PUCHAR  pucData,unsigned int uiLen);
UCHAR DIGITAL_PolingAbort(void);
void DIGITAL_AbortPackage(PUCHAR  pucData,unsigned int uiLen);
UCHAR	DIGITAL_DeSelect(void);
UCHAR	DIGITAL_Light(UCHAR ucLightCtr);
UCHAR	DIGITAL_Remove(void);
UCHAR	DIGITAL_RemoveProcess(void);
UCHAR	DIGITAL_Beep(UCHAR ucNums);
UCHAR	DIGITAL_InputPin(unsigned char* paucPin);



DIGITAL_DATA	UCHAR	aucSendRecvBuf[300];
DIGITAL_DATA	USHORT	uiSendRecvLen;
DIGITAL_DATA	POLINGDATA	PolingData;
DIGITAL_DATA	DRV_OUT		ContactlessDrv;

#ifdef		__cplusplus
}
#endif
#endif
