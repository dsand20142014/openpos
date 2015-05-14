/*
*pinpad.c
*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ospad.h>
#include <osdriver.h>
#include <osdrv.h>

unsigned char gaucSend[UARTPADMAXLEN];
unsigned int  guiSendLen;

unsigned char gaucRecv[UARTPADMAXLEN];
unsigned int  guiRecvLen;
unsigned int  guiDataLen;

unsigned char gaucRecvACK[5];
unsigned char gucRecvACKLen;

unsigned char gaucSendACK[5];
unsigned char gucSendACKLen;

unsigned char  gucNum;
unsigned char  gucBNum;



extern unsigned char FONT_ASCII57[][5];
extern unsigned char FONT_ASCII816_2[][16];


int OSPAD_Pos_ask(DRVIN *pDrvin)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
int OSPAD_Pinpad_answer(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
int OSPAD_Pos_answer(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
int OSPAD_Proc(DRV *pDrv)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
void OSPAD_ClrDisplay(unsigned char ucFont, unsigned char ucRow)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return ;
}

void OSPAD_Beep(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

void OSPAD_DisplayASCII(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

DRVOUT *OSPAD_GetKey(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}


unsigned char OSPAD_XGetKey(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}


DRV_OUT *OSPAD_GetKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}


unsigned char *OSPAD_XGetKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}


DRVOUT *OSPAD_GetFixKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

unsigned char *OSPAD_XGetFixKeys(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

DRVOUT *OSPAD_GetPin(unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
unsigned char *OSPAD_XGetPin(unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
void OSPAD_Crypt(unsigned char *pucCryptKey)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

unsigned char OSPAD_BackLightCtl(unsigned char ucFlag)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

unsigned char OSPAD_GB2312Display(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
unsigned char OSPAD_LCDGB2312Col(DRV *pDrv)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
unsigned char OSPAD_ReadDisplayGB2312Dot16(unsigned char ucQM,
        unsigned char ucWM,
        unsigned char *pucPtr)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

void OSPAD_RotateDot(unsigned char *pucDotPtr,
                     unsigned char ucBitNo,
                     unsigned char *pucCh)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

unsigned char OSPAD_ArabicDisplay(unsigned char ucFont, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}
unsigned char OSPAD_GraphDisplay(unsigned char ucType, unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr, unsigned char ucLen)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
	return 0;
}

