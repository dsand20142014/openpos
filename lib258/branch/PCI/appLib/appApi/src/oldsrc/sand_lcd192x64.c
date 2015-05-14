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

#define __SKB25_DISPLAY__

#include "s3c2410_font.h"
#include "s3c2410_lcd_yxd.h"
#include "sand_incs.h"


#define DEBUG
#ifdef DEBUG
#define dbg Uart_Printf
#else
#define dbg
#endif

#define DBG_GB

#define LCD_VRAM_HIGH 64
#define LCD_VRAM_WIDTH 192
//全局变量,用于组织显示点阵 (1 k bytes) 纵向 64 点,横向 192 点
BYTE glb_LCD_vram[LCD_VRAM_HIGH][LCD_VRAM_WIDTH];
lcd192_struct gtmp;
UCHAR col,row,cbyte;  /*列x,行(页)y,输出数据 */
FILE   *FontFileHandle;
const  char *FONTHZK="/root/JF3S1516.hz";
int lcd_fd;


void Os__clr_display ( UCHAR line);
unsigned char Os__GB2312_display(UCHAR row, UCHAR col, UCHAR *text);


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


int locatexy(char y, char x)
{
    lcd192_struct tmp;
    unsigned char ucCS=0, ucCol=0, ucRow=0;
#if 1
    ucCS=y/0x40;
    ucRow=x/8;
    ucCol=y%0x40;
#endif
    //设置Y地址
    tmp.cs=ucCS;
    tmp.val=ucCol;
    ioctl (lcd_fd, S3C2410_LCD_CMD_STARTYADDR, (unsigned long *)&tmp);
    wtcom();

    //设置X地址
    tmp.cs=ucCS;
    tmp.val=ucRow;
    ioctl (lcd_fd, S3C2410_LCD_CMD_STARTXADDR, (unsigned long *)&tmp);
    wtcom();
    dbg("ucCS=%d y=%d x=%d\n", ucCS, ucRow, ucCol);
}

void wrdata(unsigned char y, unsigned char x, unsigned char data)
{
    lcd192_struct tmp;
    unsigned char ucCS=0;

    ucCS=y/0x40;
    tmp.cs=ucCS;
    tmp.val=data;
    locatexy(y, x);
    dbg("%s (y/x)=(%d/%d), data=%02x\n", y, x, data);
    ioctl (lcd_fd, S3C2410_LCD_DATA_WR, (unsigned long *)&tmp);
    wtcom();
}


void lcmcls(unsigned char WorB)
{
    int row, col;
    unsigned char data;

    if (WorB=='W')
        data=0xFF;
    else
        data=0x0;


    for (row=0;row<LCD_VRAM_HIGH/8;row+=8)
    {
        for (col=0;col<LCD_VRAM_WIDTH;col++)
        {
            wrdata(col, row, data);
        }
    }
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


unsigned char HALDOT_ReadDisplayGB18030Dot16(
    unsigned char ucQM,unsigned char ucWM,unsigned char *pucPtr)
{
    unsigned int uiOffset;
    unsigned char ucI, i;
    unsigned char aucDotBuf[32], tmpbuf[32];
    volatile unsigned char *pucGBROMOffset;
#ifdef DBG_GB
    Uart_Printf("0 ucQM=%02x ucWM=%02x\n", ucQM, ucWM);
#endif
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

#ifdef DBG_GB
    Uart_Printf("1 ucQM=%02x ucWM=%02x uiOffset=%lx\n", ucQM, ucWM, uiOffset);
#endif

    fseek(FontFileHandle, uiOffset, SEEK_SET);

#ifdef DBG_GB
    Uart_Printf("tmpbuf data:\n");
    for (i=0; i<32; i++)
    {
        tmpbuf[i]=fgetc(FontFileHandle);
        Uart_Printf("%02x ", tmpbuf[i]);
        if ((i-1)%16==0) Uart_Printf("\n");
    }
    Uart_Printf("\n");
#endif
    pucGBROMOffset=tmpbuf;

    for (ucI=0;ucI<8;ucI++)
    {
        aucDotBuf[ucI] = *pucGBROMOffset;
        pucGBROMOffset ++;
        aucDotBuf[8+ucI] = *pucGBROMOffset;
        pucGBROMOffset ++;
    }
    for (ucI=0;ucI<8;ucI++)
    {
        aucDotBuf[16+ucI] = *pucGBROMOffset;
        pucGBROMOffset ++;
        aucDotBuf[24+ucI] = *pucGBROMOffset;
        pucGBROMOffset ++;
    }
    if ( pucPtr )
        memcpy(pucPtr,aucDotBuf,32);

#ifdef DBG_GB
    Uart_Printf("Dotbuf data:\n");
    for (i=0; i<32; i++)
    {
        Uart_Printf("%02x ", aucDotBuf[i]);
        if ((i-1)%16==0) Uart_Printf("\n");
    }
    Uart_Printf("\n");
#endif
    return 0;
}


/*
unsigned char HALMMI_DisplayCharASCII86(
unsigned char ucY,// 0-63 0-63 0-63 X3
unsigned char ucX,// 0-7 0-7 ... 0-7 X8
unsigned char ucCh)*/
unsigned char HALMMI_DisplayCharASCII86(unsigned char ucY, unsigned char ucX, unsigned char ucCh)
{
    unsigned char ucI,ucJ, ucCS=0, ucCol=0, ucRow=0;
    unsigned char ucStartX;
    unsigned char ucStartY;
    unsigned char *pucPtr, i=0;
    lcd192_struct tmp;

    pucPtr=&fontdata_6x8[ucCh*8];

    Uart_Printf("%s, ucY=%d ucX=%d, ucCh=%c\n", __FUNCTION__, ucY, ucX, ucCh);
#if 0
    for (i=0; i<8; i++)
    {
        Uart_Printf("%02x ", pucPtr[i]);
    }
    Uart_Printf("\n");
#endif

    //for (i=ucY; ucY<192; )
    {
        locatexy(ucY, ucX);
        for (ucJ=0; ucJ<8; ucJ++)
        {

            HALDOT_RotateDot(pucPtr,ucJ,&ucCh);
            ucCS=ucY/0x40;
            tmp.cs=ucCS;
            tmp.val=ucCh;

            Uart_Printf("%02x\n", tmp.val);
            ioctl (lcd_fd, S3C2410_LCD_DATA_WR, (unsigned long *)&tmp);
            wtcom();
        }
        //	ucY+=8;
    }

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
    unsigned char aucLineDotBuf[32];
    unsigned char ucCS=0, ucCol=0, ucRow=0;
    lcd192_struct tmp;

    ucResult = HALDOT_ReadDisplayGB18030Dot16(*pucCh,*(pucCh+1),aucLineDotBuf);
#ifdef DBG_GB
    Uart_Printf("input hz: (%d/%d) %02x %02x\n", ucY, ucX, *pucCh, *(pucCh+1));
    Uart_Printf("aucLineDotBuf data:\n");
    for (i=0; i<32; i++)
    {
        Uart_Printf("%02x ", aucLineDotBuf[i]);
        if ((i-1)%16==0) Uart_Printf("\n");
    }
    Uart_Printf("\n");
#endif

    ucStartX = ucX;
    ucStartY = ucY;

    if (!ucResult)
    {
        pucPtr = aucLineDotBuf;
        for (ucI=0;ucI<2;ucI++)
        {
            ucY=ucStartY;
            ucX=ucStartX+ucI*8;
            locatexy(ucY, ucX);
            for (ucJ=0;(ucJ<8);ucJ++, ucY++)
            {
                HALDOT_RotateDot(pucPtr,ucJ,&ucCh);
                ucCS=ucY/0x40;
                tmp.cs=ucCS;
                tmp.val=ucCh;
                ioctl (lcd_fd, S3C2410_LCD_DATA_WR, (unsigned long *)&tmp);
                wtcom();
            }
            pucPtr += 8;

            for (ucJ=0;(ucJ<8);ucJ++, ucY++)
            {
                HALDOT_RotateDot(pucPtr,ucJ,&ucCh);
                ucCS=ucY/0x40;
                tmp.cs=ucCS;
                tmp.val=ucCh;
//				Uart_Printf("ucCh=%02x\n", ucCh);
                ioctl (lcd_fd, S3C2410_LCD_DATA_WR, (unsigned long *)&tmp);
                wtcom();
            }
            pucPtr += 8;
        }
    }
    return 0;
}



int OpenFontFile(void)
{
    int err = 0;

    FontFileHandle = fopen(FONTHZK, "rb" );
    if (FontFileHandle == NULL)
    {
        Uart_Printf("can not open font filename: %s\n", FONTHZK);
        err = -1;
    }

    return err;
}

int LCD_Init(void)
{
    lcd_fd=open("/dev/s3c2410-lcd-yxd", O_RDWR);
    if (lcd_fd== -1)
    {
        Uart_Printf ("Cann't open LCD!\n");
        return -1;
    }
    OpenFontFile();
}

/**********************************************************
* Row 起始行 Col 起始列
* SorD Single/Double //单行/双行 'S'/'D'
* WorB  White/Black   //'W'->白底黑字/'B'->黑底白字
**********************************************************/
void LCD_ClearOneLine(int row,int col,char SorD,char WorB)
{
    int i, y=0, times=0;
    lcd192_struct tmp;
    WorB=WorB;

    Uart_Printf("%s row=%d col=%d\n", __FUNCTION__, row, col);
    if (row>LCD_VRAM_HIGH)
    {
        Uart_Printf("ERR: Row=%d\n", row);
        return ;
    }

    if (SorD=='D')
        times=2;
    else
        times=1;

    for (i=0; i<times; i++)
    {
        for (y=col; y<LCD_VRAM_WIDTH; y++)
        {
            if (y%0x40==0)
            {
                locatexy(y, (row+i*8));
            }
            tmp.cs=y/0x40;
            tmp.val=0x00;
            ioctl (lcd_fd, S3C2410_LCD_DATA_WR, (unsigned long *)&tmp);
            wtcom();
        }
    }
}


void LCD_DisplayOneLine( int row, int col, BYTE *text,char SorD, char WorB)
{
    int i, len;
    UCHAR data[64];
    lcd192_struct tmp;


    if (row>LCD_VRAM_HIGH || col>LCD_VRAM_WIDTH)
    {
        Uart_Printf("ERR: row=%d col=%d\n", row, col);
        return ;
    }

//    LCD_ClearOneLine(row, col, SorD, WorB);

    len=strlen(text);
    memcpy(data, text, len);
    data[len+1]='\n';

    i=0;
    while (*text)
    {
        dbg("%s:row=%d col=%d text=%s Sord=%c WorB=%c\n", __FUNCTION__, row, col, text, SorD, WorB);
        if (col>=LCD_VRAM_HIGH || row >=LCD_VRAM_WIDTH)
        {
            dbg("%s: I will break!!!  row=%d col=%d\n", __FUNCTION__, row, col);
            break;
        }

        if (SorD=='D' || SorD=='d' )
        {
            HALMMI_DisplayCharGB2312(row, col, text);
            row+=2*8;
            text+=2;
            i++;
        }
        else
        {
            HALMMI_DisplayCharASCII86(row, col, *text);
            row+=1*8;
            text+=1;
            i++;
        }
    }
}

void LCD_Clear(char WorB)
{
    lcmcls(WorB);
}


/******************************************************************************
 *
 *	显示接口函数部分
 *
 ******************************************************************************/
/*
作用     ： 清除一个或EFT终端一行或多行显示。
语法     ： void Os__clr_display (UCHAR line) ；
等待     ： 是
说明     ： Os__clr_display清除行, 须代入行数；
首行的数陈列是0, 当代入为 LCD_VRAM_HIGH,  则将清除所有行。
返回值：无；
*/
void Os__clr_display ( UCHAR line )
{
    if ( line ==LCD_VRAM_HIGH )
        lcmcls('W');
    else
        LCD_ClearOneLine(line, 0, 'D', 'W');
}

/*
作用     ： 将文本内容显示在LCD上；
语法     ： void Os__display (UCHAR Line, UCHAR Column, UCHAR *Text) ;
等待     ： 是
说明     ： “文本”文本内容显示在指定行列(Line, Column)；
         那儿有没有溢出检查, 文本过长将自动截断；
         Line = 0是首行显示；Column = 0是首列显示；
         “Text”是指向显示内容的指针；
返回值： 无.
*/
void Os__display(UCHAR Line, UCHAR Column, UCHAR *Text)
{
    if (Text!=NULL)
        LCD_DisplayOneLine(Line, Column, Text, 'S', 'W');	//显示字母则 S W
    //LCD_DisplayOneLine(Line, Column, Text, 'D', 'W');	//显示汉字
}

unsigned char Os__display1(UCHAR row, UCHAR col, UCHAR *text)
{
    int i, len;
    UCHAR data[64];
    lcd192_struct tmp;


    if (row>64 || col>192)
    {
        Uart_Printf("ERR: row=%d col=%d\n", row, col);
        return -1;
    }

    Os__clr_display(row);

    len=strlen(text);
    memcpy(data, text, len);
    data[len+1]='\n';

    i=0;
    while (*text)
    {
        HALMMI_DisplayCharGB2312(row, col, text);
        row+=16;
        text+=2;
        i++;
    }

    return 0;
}

unsigned char Os__GB2312_display(UCHAR Line, UCHAR Column, UCHAR *Text)
{
    if (Text!=NULL)
        LCD_DisplayOneLine(Line, Column, Text, 'D', 'W');
    return 0;
}

#if 0
extern UCHAR respone_data[256];
void Os__display_recvdata(UCHAR Line, UCHAR Column, UCHAR *rec, int len)
{
    int off_lines=0, msg_len;
    UCHAR msg[513], msg1[60];
    int dsp_len=40;
    Line=Line*2;
    Os__clr_display ( Line );
    str_from_bytes(msg, rec, len);
    msg_len=strlen(msg);
    while (msg_len>dsp_len)
    {
        memcpy(msg1, msg+dsp_len*off_lines, dsp_len);
        Os__display(Line+off_lines*2,Column,(unsigned char *)(msg1));
        off_lines++;
        msg_len=msg_len-dsp_len;
    }
    Os__display(Line+off_lines*2,Column,(unsigned char *)(msg+dsp_len*off_lines));
}
#endif

/*
	背光, 未实现！
*/
UCHAR Os__light_on ()
{
    return 0;
}
UCHAR Os__light_off ()
{
    return 0;
}

/*
OSMMI 显示部分, 现在是否使用 ? API上未实现
*/
void OSMMI_ClrDisplay(unsigned char ucFont, unsigned char ucRow)
{
    Os__clr_display(ucRow);
}

void OSMMI_DisplayASCII(unsigned char ucFont,unsigned char ucRow,
                        unsigned char ucCol,unsigned char *aucDisp)
{
    Os__GB2312_display(ucRow, ucCol, aucDisp);
}

unsigned char OSMMI_GB2312Display(unsigned char ucFont,unsigned char ucRow,
                                  unsigned char ucCol, unsigned char *pucPtr)
{
    if (pucPtr!=NULL) Os__GB2312_display(ucRow, ucCol, pucPtr);
    else
        Uart_Printf("ERROR! OSMMI_GB2312Display canNOT display NULL string!\n");
    return 0;
}

/***********************************************************
*				for Pinpad display
***********************************************************/
void Os__light_off_pp()
{
}

void Os__light_on_pp()
{
}

void Os__clr_display_pp(UCHAR line)
{
}

void Os__display_pp(UCHAR line,UCHAR col,UCHAR *text)
{
}

UCHAR Os__GB2312_display_pp(UCHAR line,UCHAR col,UCHAR * pt_msg)
{
}


/**************************************************************
 *                          客显程序
 *************************************************************/
#if 0
static char buf_dfj[20];
void change_to_3byte(char *str)
{
    int len;
    char new_str[4];
    memset(new_str, 0, 4);

    len=strlen(str);

    if (len>=3) return;

    if (len==0)
        memcpy(new_str, "000", 3);
    else if (len==1)
    {
        memcpy(new_str, "00", 2);
        memcpy(new_str+2, (void *)&str[0], 1);
    }
    else if (len==2)
    {
        memcpy(new_str, "0", 1);
        memcpy(new_str+1, (void *)&str[0], 2);
    }
    memmove(str, new_str, 3);

    return;
}


char  intTostraddPo(long int *a )
{
    char buf[16], buf2[20], buf3[3];
    int i=0, j=0, stlong=0, tmp;

    memset(buf, 0,16);
    memset(buf_dfj, 0,20);
    memset(buf2,0,20);

    sprintf(buf,"%ld",a);

    change_to_3byte(buf);
    tmp=strlen(buf);


    if (tmp>0)
    {
        while (i< (strlen(buf)-2))
        {
            memmove((void *)&buf2[i],(void *)&buf[i],1);
            i++;
        }
    }

    memmove((void *)&buf2[i],".",1);
    memmove((void *)&buf2[i+1],(void *)&buf[i],2);
    memmove((void *)&buf2[i+3], "J", 1);
    memmove((void *)&buf_dfj[0],"K",1);

    stlong = strlen(buf2);
    sprintf(buf3,"%d", stlong-1);

    if ((stlong-1) >9)
    {
        memmove((void *)&buf_dfj[1], &buf3, 2);
        memmove((void *)&buf_dfj[3],&buf2,stlong);
    }
    else
    {
        memmove((void *)&buf_dfj[1], buf3, 1);
        memmove((void *)&buf_dfj[2],buf2, stlong);
    }
}

//用于客显切换
extern int fd_switchcom;
void kx_je_dfj(long int *b)
{
    int ret, fd, d1, d2,i=0;
    char buf_dfj2[20];

    memset(buf_dfj2,0,20);

    //将整形转换成 Kn123...nJ格式的字符串,并且保存在buf_dfj中。
    intTostraddPo(b);

    ret = ioctl(fd_switchcom,4,NULL); //打开客显串口

    usleep(100000); //用于稳定串口读写
    fd=SerialPortInit("/dev/ttyS1",9600,8,'N',1,0,0,0);

    d1 = write(fd, buf_dfj, strlen(buf_dfj)); //0926
    usleep(10000);
    d2 = read(fd, buf_dfj2,strlen(buf_dfj) );

    while (d1 != d2 )
    {
        usleep(10000);
        d1 = write(fd, buf_dfj, strlen(buf_dfj)); //0926
        usleep(10000);
        d2 = read(fd, buf_dfj2,strlen(buf_dfj) );

        i++;
        if (i>20)    break;
    }
    close(fd);
}

void OSCST_CustDisplay(unsigned long ulLong)
{
    long int ulRealAmount_for_kx;
    ulRealAmount_for_kx=(long int)ulLong;
    kx_je_dfj(&ulRealAmount_for_kx);
}
#endif


int main()
{
    int fd, status=0, i=1, c, ret;
    lcd192_struct tmp;
    char choice[2], data[2], str[30];
    int yaddr=0, xaddr=0;
    unsigned char *pData="上海杉德";
    unsigned char *pEn;

    while (1)
    {
        Uart_Printf("1\n");
        ret=LCD_Init();
        if (ret<0)
        {
            Uart_Printf("open LCD device failed!\n");
            return -1;
        }
        Uart_Printf("2\n");

        Uart_Printf("LCD Test Demo\n");
        Uart_Printf("1. clear\n");
        Uart_Printf("2. HAL192_PowerSwitch\n");
        Uart_Printf("3. HAL192_StarLine\n");
        Uart_Printf("4. HAL192_StarXAddr\n");
        Uart_Printf("5. HAL192_StarYAddr\n");
        Uart_Printf("6. HAL192_WriteData\n");
        Uart_Printf("7. HAL192_ReadData\n");
        Uart_Printf("8. write en\n");
        Uart_Printf("9. write chinese\n");
        Uart_Printf("0. exit\n");

        Uart_Printf("please enter your choice:\n");
        scanf("%s", choice);
        switch (choice[0])
        {
        case '9':
            Uart_Printf("9 write many Chinese  please enter X, Y\n");
            scanf("%d,%d", &xaddr, &yaddr);
            Uart_Printf("x=%d y=%d\n", xaddr, yaddr);
            LCD_DisplayOneLine(yaddr, xaddr, "上海杉德金卡信息系统科技", 'D', 'W');

            break;

        case '8':
            Uart_Printf("8 write char  please enter X, Y\n");
            Uart_Printf("4 show mang english(x/y)\n");
            scanf("%d,%d", &xaddr, &yaddr);
            memset(str, 0, 30);
            strcpy(str, "ABCDEFGHIJKLMNOPQRSTUVWX");
            LCD_DisplayOneLine(yaddr, xaddr, str, 'S', 'W');

            break;


        case '1':
            Uart_Printf("clear one line\n");
            scanf("%d", &yaddr);
            Os__clr_display(yaddr);
            break;

        case '2':
            Uart_Printf("2 please enter cs,state(0/1)\n");
            scanf("%d,%d", &tmp.cs, &tmp.val);
            Uart_Printf("tmp.val=%x\n", tmp.val);
            Uart_Printf("tmp.cs=%x\n", tmp.cs);
            ioctl (lcd_fd, S3C2410_LCD_CMD_POWER, (unsigned long *)&tmp);
            wtcom();
            break;

        case '3':
            Uart_Printf("3 please enter cs, Z\n");
            scanf("%d,%d", &tmp.cs, &tmp.val);
            ioctl (lcd_fd, S3C2410_LCD_CMD_STARTLINE, (unsigned long *)&tmp);
            wtcom();
            break;

        case '4':
            Uart_Printf("4 please enter cs, X\n");
            scanf("%d,%d", &tmp.cs, &tmp.val);
            ioctl (lcd_fd, S3C2410_LCD_CMD_STARTXADDR, (unsigned long *)&tmp);
            wtcom();
            break;

        case '5':
            Uart_Printf("5 please enter cs, Y\n");
            scanf("%d,%d", &tmp.cs, &tmp.val);
            ioctl (lcd_fd, S3C2410_LCD_CMD_STARTYADDR, (unsigned long *)&tmp);
            wtcom();
            break;

        case '6':
            Uart_Printf("6 please enter cs, data\n");
            scanf("%d,%d", &tmp.cs, &tmp.val);
            tmp.val=~tmp.val;
            Uart_Printf("to write tmp.val=%d\n", tmp.val);
            ioctl (lcd_fd, S3C2410_LCD_DATA_WR, (unsigned long *)&tmp);
            wtcom();
            break;

        case '7':
            Uart_Printf("7 please enter cs\n");
            tmp.val=0;
            scanf("%d", &tmp.cs);
            ioctl (lcd_fd, S3C2410_LCD_DATA_RD, (unsigned long *)&tmp);
            wtcom();
            Uart_Printf("read data: %x\n", tmp.val);
            break;




        case '0':
            goto OUT_LINE;

        default:
            break;
        }
        close(lcd_fd);
        fclose(FontFileHandle);
    }

OUT_LINE:
    Uart_Printf("app exit\n");
    close(lcd_fd);
    return 0;
}
