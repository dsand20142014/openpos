/*
 *  sand API for LCD 192x64
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <asm/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>
#include <termios.h>
#include <errno.h>
#include <linux/ioctl.h>


#include "font_8x8.h"

#include "s3c2410_lcd_t6963.h"
#include "sand_incs.h"



#define DEBUG
#ifdef DEBUG
#define dbg Uart_Printf
#else
#define dbg
#endif

#ifndef uchar
#define uchar unsigned char
#endif

int lcd_fd=0;
FILE   *FontFileHandle;
FILE   *FontFileHandle_hf;
const  char FontFilename[]={"/root/JF3S1516.hz"};
const  char FontFilename_hf[]={"./font/jf1s1112.zf"};
const  int  FONTSTARTSECTION = 1;

#define LINENUM 30 //一行16个字符(8*8)
#define COLNUM 16 //总共16列
#define LCD_VRAM_HIGH 128
#define LCD_VRAM_WIDTH 240
unsigned int txthome,grhome;

//显示内存分配
//0000h-67ffh 文本区
//6800h-77ffh 4K 图形区
//7800h-7fffh 2k CGRAM区
#define DISRAM_SIZE 0X7FFF  //设置显示ram的大小
#define TXTSTART 0x0000      //设置文本区的起始地址
#define GRSTART 0x6800       //设置图形区的起始地址
#define CGRAMSTART 0x7800  //设置CGRAM的起始地址

void _nop_(void)
{
    int nms=25;
    int ms=0;

    for (ms=0;ms<nms;ms++)
        {}
}

void delayms(uint nms)
{
    uint ms;
    for (ms=0;ms<nms;ms++)
        {}
}

void wtcom(void)
{
    delayms(250);
}

int OpenFontFile(void)
{
    int err = 0;

    FontFileHandle = fopen( FontFilename, "rb" );
    if (FontFileHandle == NULL)
    {
        Uart_Printf("can not open font filename:%s\n", FontFilename);
        err = 1;
    }

    return err;
}

int LCD_Init(void)
{
    Uart_Printf("open %s in %s\n", DEVICE_NAME, __FUNCTION__);
    lcd_fd=open(DEVICE_NAME, O_RDWR);
    if (lcd_fd== -1)
    {
        Uart_Printf ("Cann't open LCD:%s!\n", DEVICE_NAME);
        return -1;
    }
    Uart_Printf("open %s OK!\n", DEVICE_NAME);
    OpenFontFile();
}

unsigned char HAL240_WriteData(unsigned char ucData)
{
    ioctl(lcd_fd, S3C2410_LCD_DATA, &ucData);
    wtcom();
}

unsigned char HAL240_WriteCmd(unsigned char ucCmd)
{
    ioctl(lcd_fd, S3C2410_LCD_CMD, &ucCmd);
    wtcom();
}

unsigned char HAL240_StarXYAddr(unsigned char *pOffset)
{
    HAL240_WriteData(*pOffset++);
    HAL240_WriteData(*pOffset++);
    HAL240_WriteCmd(*pOffset);
}

void OutPortCom3(uchar twop1,uchar twop2,uchar comd2)
{
    HAL240_WriteData(twop1);
    HAL240_WriteData(twop2);
    HAL240_WriteCmd(comd2);
}

void OutPortCom2(uchar twop1,uchar comd1)
{
    HAL240_WriteData(twop1);
    HAL240_WriteCmd(comd1);
}

void HALDOT_RotateDot(unsigned char *pucDotPtr,	unsigned char ucBitNo,	unsigned char *pucCh)
{
    unsigned char ucCh;

    ucCh = ((*pucDotPtr << ucBitNo) & 0x80) >> 7;
    ucCh |= ((*(pucDotPtr+1) << ucBitNo) & 0x80) >> 6;
    ucCh |= ((*(pucDotPtr+2) << ucBitNo) & 0x80) >> 5;
    ucCh |= ((*(pucDotPtr+3) << ucBitNo) & 0x80) >> 4;
    ucCh |= ((*(pucDotPtr+4) << ucBitNo) & 0x80) >> 3;
    ucCh |= ((*(pucDotPtr+5) << ucBitNo) & 0x80) >> 2;
    ucCh |= ((*(pucDotPtr+6) << ucBitNo) & 0x80) >> 1;
    ucCh |= (*(pucDotPtr+7) << ucBitNo) & 0x80;

    *pucCh = ucCh;
}

unsigned char HAL240_Display8DotInRow_no8(unsigned char ucFlag,
        unsigned char row,unsigned char col,unsigned char ucCh)
{
    unsigned int uiI,i=0, sAddr=0;
    unsigned char aucBuf[8], data;

#if 0
    if (ucFlag)
        ucCh = ~ucCh;
#endif

    sAddr=row*LINENUM+col;
    for (i=0;i<8;i++)
    {
        data=fontdata_8x8[ ucCh*8+i];
        aucBuf[0] = sAddr;
        aucBuf[1] = sAddr>>8;
        aucBuf[2] = 0x24;
        HAL240_StarXYAddr(aucBuf);

        HAL240_WriteData(data);
        HAL240_WriteCmd(0xC0);
        Uart_Printf("* %d * row=%d col=%d,addr=%d ucCh=%c(%d) wr data:%02x\n",i, row, col, sAddr,ucCh, ucCh, data);
        sAddr=sAddr + LINENUM;
    }

    return(LCD240_LCDSUCESS);
}


/*OK*/
unsigned char HAL240_Display8DotInRow(unsigned char ucFlag,
                                      unsigned char row,unsigned char col,unsigned char ucCh)
{
    unsigned int uiI,i=0, sAddr=0;
    unsigned char aucBuf[8], data;

#if 0
    if (ucFlag)
        ucCh = ~ucCh;
#endif

    sAddr=row*LINENUM+col;
    for (i=0;i<8;i++)
    {
        data=fontdata_8x8[ ucCh*8+i];
        aucBuf[0] = sAddr;
        aucBuf[1] = sAddr>>8;
        aucBuf[2] = 0x24;
        HAL240_StarXYAddr(aucBuf);

        HAL240_WriteData(data);
        HAL240_WriteCmd(0xC0);
        Uart_Printf("* %d * row=%d col=%d,addr=%d ucCh=%c(%d) wr data:%02x\n",i, row, col, sAddr,ucCh, ucCh, data);
        sAddr=sAddr + LINENUM;
    }

    return(LCD240_LCDSUCESS);
}


unsigned char HALDOT_ReadDisplayGB18030Dot16(
    unsigned char ucQM,unsigned char ucWM,unsigned char *pucPtr)
{
    unsigned int uiOffset;
    unsigned char ucI, i;
    unsigned char aucDotBuf[32], tmpbuf[32];
    volatile unsigned char *pucGBROMOffset;

    dbg("0 ucQM=%02x ucWM=%02x\n", ucQM, ucWM);
    if (  (ucQM < 0x80) ||(ucWM < 0x40) )
        return -1;


    if ((ucQM >= 0xB0)&&(ucWM >= 0xA1))
    {
        ucQM = ucQM - 0xB0;
        ucWM = ucWM - 0xA1;
        uiOffset = ucQM * 94 + ucWM;
    }
    else if (ucQM <= 0xA0)
    {
        ucQM = ucQM - 0x81;
        ucWM = ucWM - 0x40;
        uiOffset = ucQM * 190 + ucWM + 6768;
        if (ucWM > 0x3E)
            uiOffset --;
    }
    else if ((ucQM >= 0xAA)&&(ucWM <= 0xA0))
    {
        ucQM = ucQM - 0xAA;
        ucWM = ucWM - 0x40;
        uiOffset = ucQM * 96 + ucWM + 12848;
    }
    uiOffset *= 32;

    dbg("1 ucQM=%02x ucWM=%02x uiOffset=%lx\n", ucQM, ucWM, uiOffset);

    fseek(FontFileHandle, uiOffset, SEEK_SET);


    Uart_Printf("tmpbuf data:\n");
    for (i=0; i<32; i++)
    {
        tmpbuf[i]=fgetc(FontFileHandle);
#if 0
        Uart_Printf("%02x ", tmpbuf[i]);
        if (i && (i+1)%16==0) Uart_Printf("\n");
#endif
    }

    if ( pucPtr )
        memcpy(pucPtr,tmpbuf,32);


    return 0;
}


unsigned char HALMMI_DisplayCharGB2312(unsigned char ucY,unsigned char ucX,
                                       unsigned char *pucCh)
{
    unsigned char ucResult;
    unsigned char ucI,ucJ;
    unsigned char ucStartX;
    unsigned char ucStartY;
    unsigned char *pucPtr;
    unsigned char ucCh, i;
    unsigned char aucLineDotBuf[32], aucBuf[4];
    unsigned char ucCS=0, ucCol=0, ucRow=0;
    unsigned int sAddr=0;

    memset(aucLineDotBuf, 0, 32);
    ucResult = HALDOT_ReadDisplayGB18030Dot16(*pucCh,*(pucCh+1),aucLineDotBuf);

    ucStartX = ucX;
    ucStartY = ucY;

    if (!ucResult)
    {
        ucY=ucStartY;
        ucX=ucStartX;//+ucI*8;
        sAddr=ucY*LINENUM+ucX;

        for (ucJ=0;(ucJ<16);ucJ++, ucY++)
        {
            aucBuf[0] = sAddr;
            aucBuf[1] = sAddr>>8;
            aucBuf[2] = 0x24;
            HAL240_StarXYAddr(aucBuf);

            OutPortCom2(aucLineDotBuf[ucJ*2], 0xC0);
            OutPortCom2(aucLineDotBuf[ucJ*2+1], 0xC0);
            wtcom();
            sAddr=sAddr + LINENUM;
        }

    }

    return 0;
}

void LCD_ClearOneLine(int row,int col,char SorD,char WorB)
{
    int i, y=0, times=0;
    unsigned int sAddr=0;
    unsigned char aucBuf[8], data;


    WorB=WorB;

    if (row>LCD_VRAM_HIGH/8)
    {
        Uart_Printf("ERR: Row=%d\n", row);
        return ;
    }

    if (SorD=='D')
        times=2;
    else
        times=1;

    sAddr=row*LINENUM+col;
    for (i=0; i<times; i++)
    {
        for (y=col; y<LCD_VRAM_WIDTH; y++)
        {
            aucBuf[0] = sAddr;
            aucBuf[1] = sAddr>>8;
            aucBuf[2] = 0x24;
            HAL240_StarXYAddr(aucBuf);

            HAL240_WriteData(0x00);
            HAL240_WriteCmd(0xC0);
            wtcom();
        }
        sAddr+=LINENUM;
    }
}


void LCD_DisplayOneLine( int row, int col, BYTE *text,char SorD, char WorB)
{
    int i, len;
    UCHAR data[64];


    if (row>LCD_VRAM_HIGH || col>LCD_VRAM_WIDTH)
    {
        Uart_Printf("ERR: row=%d col=%d\n", row, col);
        return ;
    }
    dbg("%s:row=%d col=%d text=%s Sord=%c WorB=%c\n",
        __FUNCTION__, row, col, text, SorD, WorB);

    LCD_ClearOneLine(row, col, SorD, WorB);

    len=strlen(text);
    memcpy(data, text, len);
    data[len+1]='\n';

    i=0;
    while (*text)
    {
        if (row>LCD_VRAM_HIGH || col >LCD_VRAM_WIDTH)
        {
            dbg("%s: I will break!!!  row=%d col=%d\n", __FUNCTION__, row, col);
            break;
        }

        if (SorD=='D' || SorD=='d' )
        {
            HALMMI_DisplayCharGB2312(row, col, text);
            col+=2;
            text+=2;
            i++;
        }
        else
        {
            HAL240_Display8DotInRow(0, row, col, *text);
            col+=1;
            text+=1;
            i++;
        }
    }
}


unsigned char Os__GB2312_display(UCHAR Line, UCHAR Column, UCHAR *Text)
{
    if (Text!=NULL)
        LCD_DisplayOneLine(Line, Column, Text, 'D', 'W');
    return 0;
}

int main()
{
    int fd, status=0, i=1, ret;
    char choice[2], data[2], c;
    int yaddr=0, xaddr=0, x1, x2, y1=0;
    unsigned char *pData="上海杉德金卡信息系统科技公司上", str[31];
    unsigned char val[10], val1;
    char flag=0;


    ret=LCD_Init();
    if (ret<0)
    {
        Uart_Printf("open LCD device failed!\n");
        return -1;
    }

    while (1)
    {
        Uart_Printf("LCD Test Demo\n");
        Uart_Printf("1. clear screen\n");
        Uart_Printf("2. PowerSwitch OK\n");
        Uart_Printf("3. show chinese\n");
        Uart_Printf("4. show english\n");
        Uart_Printf("5. show many char\n");
        Uart_Printf("6. no /8 \n");
        Uart_Printf("0. exit\n");

        Uart_Printf("please enter your choice:\n");
        scanf("%s", choice);

        switch (choice[0])
        {
        case '1':
            Uart_Printf("1 clear screen\n");
            ioctl (lcd_fd, S3C2410_LCD_CLS, (unsigned char *)&val[0]);
            break;

        case '2':
            Uart_Printf("2 please enter status(0/1)\n");
            scanf("%s", val);
            Uart_Printf("status data: %s\n", val);
            ioctl (lcd_fd, S3C2410_LCD_STAUTE, (unsigned char *)&val[0]);
            break;

        case '3':
            Uart_Printf("3 show many chinese(y/x)\n");
            scanf("%d,%d", &yaddr, &xaddr);
            Os__GB2312_display(yaddr, xaddr, pData);
            break;

        case '4':
            Uart_Printf("4 show mang english(y/x)\n");
            scanf("%d,%d", &yaddr, &xaddr);
            strcpy(str, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123");
            for (i=0; i<strlen(str); i++)
            {
                HAL240_Display8DotInRow(0, yaddr, xaddr+i, str[i]);
            }
            break;

        case '5':
            memset(str, 0, 30);
            Uart_Printf("5 show string:(y,x,str)\n");
            scanf("%d,%d, %s", &yaddr, &xaddr, str);
            //strcpy(str, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            for (i=0; i<strlen(str); i++)
            {
                HAL240_Display8DotInRow(0, yaddr, xaddr+i, str[i]);
            }
            break;

        case '6':
#if 0
            Uart_Printf("draw a line: x1,y1,x2\n");
            scanf("%d,%d,%d", &x1, &y1, &x2);
            draw_line(x1, y1, x2);
#endif
            Uart_Printf("4 show english 'S' no /8 at (y,x)\n");
            scanf("%d,%d", &yaddr, &xaddr);
            HAL240_Display8DotInRow_no8(0, yaddr, xaddr, 'S');

            break;

        case '0':
            goto OUT_LINE;
        default:
            break;
        }
    }

OUT_LINE:
    Uart_Printf("app exit\n");
    close(lcd_fd);
    Uart_Printf("app exit 0\n");
    fclose(FontFileHandle);
    Uart_Printf("app exit 1\n");
    return 0;
}
