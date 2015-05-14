/* MU110的实现方式 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <stdio.h>

#define __LUXAPP_PRINTER__

#include <sand_print.h>
#include <s3c2410_mu110.h>
#include <mu110_font.h>

#define DRVNAME  "/dev/s3c2410_mu110"
#define FILENAME  "/dev/s3c2410_mu110"

//typedef enum { md_5x7en,md_7x9en,md_7x9gb,md_gb2312 }	fmode_t ;
#define  get_byte_bit(uc8,bit)				(((uc8)>>(bit)) & 0x01)

void compress_y(unsigned char *);
void compress_x(unsigned char *);
void compress(unsigned char *);

static int gb2312_16x16rd(unsigned char *chinese,unsigned char buff[])
{
    unsigned char qm,wm;
    //    unsigned char ucQM,ucWM;
    unsigned long offset;
//    unsigned char tbuff[32];
    FILE *gb2312;
    int i ;

    /*打开字库文件hzk16*/
    if ((gb2312=fopen("GB2312.bin","rb"))==NULL)
    {
        Uart_Printf("Can't open GB2312.bin,Please add it?");
    }
    /*区码=内码(高字节)-160  位码=内码(低字节)-160*/

//    qm  = *(chinese)   - 0xa0;           /*10进制的160等于16进制的A0*/
    qm  = *(chinese)   - 0xa0 -3;          /*10进制的160等于16进制的A0*/
    wm  = *(chinese+1) - 0xa0;             /*获得区码与位码*/
    offset = (94*(qm-1)+(wm-1))*32L; /*计算该汉字在字库中偏移量*/
    fseek(gb2312,offset,SEEK_SET);   /*将文件指针移动到偏移量的位置*/
    fread(buff,32,1,gb2312);         /*从偏移量的位置读取32个字节*/
    fclose(gb2312);

    /*
       for (i = 0; i<32; i++) {
         if (i%8 ==0) Uart_Printf("\n");
       	Uart_Printf ("buff[%d]=%02x  ",i,buff[i]);
       }
    */
    Uart_Printf("\n qm:%d,wm:%d,offset:%ld\n\r",qm,wm,offset);
}

/*
description : printer goes forward n_steps.
variable :
				n_steps: steps.
 return :
 				0: ok
 				-1: error.
*/
unsigned char Os__prn_forward (unsigned char n_steps)
{

    int  rtn = -1;
    int fd ;
    fd = open(DRVNAME, O_RDWR) ;

    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    rtn = ioctl(fd,MU110_FEED_PAPER,n_steps);

    close(fd);
    return rtn ;

}
/*
the same as Os__prn_xforward
*/
unsigned char Os__prn_xforward (unsigned char n_steps)
{

    return Os__prn_forward(n_steps);
}

/*
description : printer goes backword n_steps.
variable :
				n_steps: steps.
 return :
 				0: ok
 				-1: error.
*/

unsigned char Os__prn_backward (unsigned char n_steps)
{

    int  rtn = -1;
    int fd ;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    rtn = ioctl(fd,MU110_BACK_PAPER,n_steps);
    close(fd);
    return rtn ;
}
/*
the same as Os__prn_backward
*/

unsigned char OSPRN_XReverseFeed (unsigned char n_steps)
{
    return Os__prn_backward(n_steps);
}

/*
description :printer feed paper n lines
variable :
				line:lines
Return :
				0: ok
			 -1: error
*/
DRV_OUT  *Os__linefeed (unsigned  char line)
{

    int  rtn = -1;
    int fd ;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return NULL ;
    }
    rtn = ioctl(fd,MU110_FEED_PAPER,line*8); //??
    close(fd);

    return NULL;

}
/*
the same as Os__linefeed
*/
unsigned char Os__xlinefeed (unsigned char line)
{
    Os__linefeed(line) ;
    return 0;
}

/*
description :according to font,print data in the pdata on the line.
variable :
				font,pdata
return :
				0: ok
				-1:error
*/
#if 1
DRV_OUT *Os__print_EX(fmode_t font,unsigned char *pdata)
{

//	fmode_t font = md_gb2312 ;
    unsigned char kdata[2][210] ;
    unsigned char ddata[32];
    unsigned char i ;
    unsigned char j ;
//	unsigned char k ;
//	unsigned char test ;
    unsigned char len ;
    int rtn = -1 ;
    int fd ;
    Uart_Printf("Os__print,%s\n",pdata);
    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return NULL;
    }

    switch (font)
    {
    case FONT_5X7_EN:
        i = 0 ; //0
        while (*(pdata+i))
        {
//			rd_dat_5x7en(pdata[i],&kdata[0][i*6]);
            rd_dat_5x7en(pdata[i],ddata);
            memcpy(&kdata[0][1+i*6],ddata,6);
            i++ ;
        }
        kdata[0][0] = 6*i ;
        ioctl(fd,MU110_PRINT_STRING,kdata[0]);
        rtn = 0;
        break ;

    case FONT_7X9_EN:
    case FONT_7X9_GB:
        i = 0; //0
        while ( *(pdata+i) )
        {
            if (font == FONT_7X9_EN)
                rd_dat_7x9en(pdata[i],ddata); //16
            else
                rd_dat_7x9gb(pdata[i],ddata);

            memcpy (&kdata[0][1+i*8],ddata,8);
            memcpy (&kdata[1][1+i*8],ddata+8,8);
            i++;
        }
        kdata[0][0] = 8*i ;
        kdata[1][0] = 8*i ;
        ioctl(fd,MU110_PRINT_STRING,kdata[0]); //??
        ioctl(fd,MU110_PRINT_STRING,kdata[1]);//??
        rtn = 0;
        break ;
    case FONT_16X16_GB2312:
        i = 0;
        j = 0;
        while ( *(pdata+i) )
        {
            //				Uart_Printf("*(pdata+0)=%x,*(pdata+1)=%x\n",*(pdata+i),*(pdata+i+1));
            gb2312_16x16rd((pdata+i),ddata);

            memcpy (&kdata[0][1+j*16],ddata,16);
            memcpy (&kdata[1][1+j*16],ddata+16,16);
            i += 2;
            j++ ;
        }
        kdata[0][0] = 16*j ;
        kdata[1][0] = kdata[0][0] ;
        ioctl(fd,MU110_PRINT_STRING,kdata[0]);
        ioctl(fd,MU110_PRINT_STRING,kdata[1]);

        break ;
    case FONT_8X8_GB2312 :
        i = 0;
        j = 0;
        while ( *(pdata+i) )
        {
            //		Uart_Printf("*(pdata+0)=%x,*(pdata+1)=%x\n",*(pdata+i),*(pdata+i+1));
            gb2312_16x16rd((pdata+i),ddata);
//				halfstep_modify(ddata);
//				compress_y(ddata);
//				compress_x(ddata);
            compress(ddata);

            //			for (k = 0 ;k<8; k++)
            {
                //			kdata[0][1+j*8+k] = ddata[2*k] | ddata[2*k+1];
                //		kdata[1][1+j*8+k] = ddata[16+2*k] | ddata[16+2*k+1];

                //			kdata[0][1+j*8+k] |= kdata[1][1+j*8+k] ;
                //			Uart_Printf("ddata[2*k] = %#x  ddata[2*k+1] = %#x test = %#x\n",ddata[2*k],ddata[2*k+1],test);
                //			kdata[0][1+j*8+k] = ddata[2*k];
                //			kdata[1][1+j*8+k] = ddata[2*k+1];
            }
            memcpy (&kdata[0][1+j*16],ddata,16);
            i += 2;
            j++ ;
        }

        //		kdata[0][0] = 8*j ;
        kdata[0][0] = 16*j ;
        //		kdata[1][0] = kdata[0][0];
        ioctl(fd,MU110_PRINT_STRING_CH8X8,kdata[0]);
        //		ioctl(fd,MU110_PRINT_STRING,kdata[1]);

        break ;
    default :
        break ;
    }
    close (fd);
    return NULL ;
}


DRV_OUT *Os__print(unsigned char *pdata)
{
    fmode_t font;
    font = FONT_16X16_GB2312;
    Os__print_EX(font,pdata);
}

#else
int gb232_16x16_print(unsigned char *pdata)
{

    unsigned char kdata[2][210] ;
    unsigned char ddata[32];
    unsigned char i ;
    unsigned char j ;
    int fd ;

    Uart_Printf("gb232_16x16_print:%s\n",pdata);
    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    i = 0;
    j = 0;
    while ( *(pdata+i) )
    {
        //		Uart_Printf("*(pdata+0)=%x,*(pdata+1)=%x\n",*(pdata+i),*(pdata+i+1));
        gb2312_16x16rd((pdata+i),ddata);

        memcpy (&kdata[0][1+j*16],ddata,16);
        memcpy (&kdata[1][1+j*16],ddata+16,16);

        i += 2;
        j++ ;
    }

    kdata[0][0] = 16*j ;
    kdata[1][0] = kdata[0][0] ;
    ioctl(fd,MU110_PRINT_STRING,kdata[0]);
    ioctl(fd,MU110_PRINT_STRING,kdata[1]);

    close(fd);
    return 0 ;
}

int gb232_8x8_print(unsigned char *pdata)
{

    unsigned char kdata[210];
    unsigned char ddata[32];
    unsigned char i ;
    unsigned char j ;

    int fd ;

    Uart_Printf("gb232_8x8_print,%s\n",pdata);

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    i = 0;
    j = 0;
    while ( *(pdata+i) )
    {
        //		Uart_Printf("*(pdata+0)=%x,*(pdata+1)=%x\n",*(pdata+i),*(pdata+i+1));
        gb2312_16x16rd((pdata+i),ddata);
        compress(ddata);
        memcpy (&kdata[1+j*16],ddata,16);
        i += 2;
        j++ ;
    }

    kdata[0] = 16*j ;

    ioctl(fd,MU110_PRINT_STRING_CH8X8,kdata);

    close(fd);
    return 0 ;

}
int en_5x7_print(unsigned char *pdata)
{
    unsigned char kdata[210] ;
    unsigned char ddata[32];
    unsigned char i ;
    int fd ;

    Uart_Printf("en_5x7_print,%s\n",pdata);

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("en_5x7_print:open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    i = 0 ;
    while (*(pdata+i))
    {

        rd_dat_5x7en(pdata[i],ddata);
        memcpy(&kdata[1+i*6],ddata,6);
        i++ ;
    }
    kdata[0] = 6*i ;
    ioctl(fd,MU110_PRINT_STRING,kdata);

    close(fd) ;
    return 0 ;
}

int en_7x9_print(unsigned char *pdata)
{
    unsigned char kdata[2][210] ;
    unsigned char ddata[32];
    unsigned char i ;
    unsigned char j ;
    int fd ;

    Uart_Printf("en_7x9_print,%s\n",pdata);

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    i = 0;
    while ( *(pdata+i) )
    {
        rd_dat_7x9en(pdata[i],ddata);
        memcpy (&kdata[0][1+i*8],ddata,8);
        memcpy (&kdata[1][1+i*8],ddata+8,8);
        i++;
    }

    kdata[0][0] = 8*i ;
    kdata[1][0] = 8*i ;

    ioctl(fd,MU110_PRINT_STRING,kdata[0]);
    ioctl(fd,MU110_PRINT_STRING,kdata[1]);

    close(fd);
    return 0;
}

int gb_7x9_print(unsigned char *pdata)
{
    unsigned char kdata[2][210] ;
    unsigned char ddata[32];
    unsigned char i ;
    unsigned char j ;
    int fd ;

    Uart_Printf("gb_7x9_print,%s\n",pdata);

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    i = 0;
    while ( *(pdata+i) )
    {
        rd_dat_7x9gb(pdata[i],ddata);
        memcpy (&kdata[0][1+i*8],ddata,8);
        memcpy (&kdata[1][1+i*8],ddata+8,8);
        i++;
    }

    kdata[0][0] = 8*i ;
    kdata[1][0] = 8*i ;

    ioctl(fd,MU110_PRINT_STRING,kdata[0]);
    ioctl(fd,MU110_PRINT_STRING,kdata[1]);

    close(fd);
    return 0;
}

int Os__print( fmode_t font,unsigned char *data)
{
    switch (font)
    {
    case FONT_5X7_EN:
        en_5x7_print(data);
        break;
    case FONT_7X9_EN:
        en_7x9_print(data);
        break;
    case FONT_7X9_GB:
        gb_7x9_print(data);
        break;
    case FONT_16X16_GB2312:
        gb232_16x16_print(data);
        break;
    case FONT_8X8_GB2312:
        gb232_8x8_print(data);
        break;
    default :
        break ;
    }
    return 0 ;
}
#endif


/*
The same as Os__print.
*/

unsigned char Os__xprint (unsigned char *pdata)
{

//		return Os__print(pdata) ;

}
/*
 description: printting chinese in terminal graph.
 variable :
 			pdata :data
 			size :the length of data.
 return :
 				0: ok
 				-1: error.
*/
unsigned char Os__GB2312_xprint (unsigned char * pdata, unsigned char size)
{

    return -1;
}
/*
	description : printting acsii string in GB2312 way.
	variable :
				ptAscii: ascii string.
 return :
 				0: ok
 				-1: error.
*/

unsigned char *Os__conv_ascii_GB2312 (unsigned char * ptAscii)
{

    return NULL;
}

/*
description : printting graph.
variable :
				pixel_line :the print pixel
				size : length of pixel_line.
 return :
 				0: ok
 				-1: error.
*/

unsigned char Os__graph_xprint (unsigned char * pixel_line, unsigned char size)
{

    return -1;
}

unsigned char Os__black_mark(void)
{

    int fd ;
    int rtn = -1 ;
    int data = 0;
    fd = open(DRVNAME, O_RDWR);
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    rtn = ioctl(fd,MU110_BLACK_MASK,&data);
    return rtn ;
    close(fd);
}


#if 0
int halfstep_modify(unsigned char *dotbuf)
{
    unsigned char i;
    unsigned char temp1,temp2;
    unsigned char pdata1,pdata2;
    for (i=0;i<16;i++)
    {
        temp1 = *(dotbuf+i);
        temp2 = *(dotbuf+i+16);
        pdata1 = temp1&0x01;
        pdata1 |= ((temp1&0x04)>>1);
        pdata1 |= ((temp1&0x10)>>2);
        pdata1 |= ((temp1&0x40)>>3);

        pdata1 |= ((temp2&0x01)<<4);
        pdata1 |= ((temp2&0x04)<<3);
        pdata1 |= ((temp2&0x10)<<2);
        pdata1 |= ((temp2&0x40)<<1);

        pdata2  = ((temp1&0x02)>>1);
        pdata2 |= ((temp1&0x08)>>2);
        pdata2 |= ((temp1&0x20)>>3);
        pdata2 |= ((temp1&0x80)>>4);

        pdata2 |= ((temp2&0x02)<<3);
        pdata2 |= ((temp2&0x08)<<2);
        pdata2 |= ((temp2&0x20)<<1);
        pdata2 |=  (temp2&0x80);

        *(dotbuf+i) = pdata1;
        *(dotbuf+i+16) = pdata2;
    }

    for (i=1;i<15;i++)
    {
        *(dotbuf+i) = (~((*(dotbuf+i-1))&(*(dotbuf+i))))&(*(dotbuf+i));
        *(dotbuf+i+16) = (~((*(dotbuf+i+15))&(*(dotbuf+i+16))))&(*(dotbuf+i+16));
    }
    return 0;
}

#else

void compress_y(unsigned char *dotbuf)
{

    int i ;
    int j ;
    unsigned char datup;
    unsigned char datdn;
    unsigned char tfw;
    unsigned char tdn;
    unsigned char odd ;
    unsigned char even ;

    for (i = 0; i<16; i++ )
    {
        datup = *(dotbuf+i);
        datdn = *(dotbuf+i+16);

        odd = 0;
        even = 0;
        for (j = 0 ;j<4; j++)
        {
            //		compress  |= ( get_byte_bit(datup,2*j) | get_byte_bit(datup,2*j+1) )<< j;
            //		compress  |= ( get_byte_bit(datdn,2*j) | get_byte_bit(datdn,2*j+1) )<< (j+4);

            odd |= get_byte_bit(datup,2*j) << j ;
            odd |= get_byte_bit(datdn,2*j) << (4+j) ;

            even |= get_byte_bit(datup,2*j+1) << j ;
            even |= get_byte_bit(datdn,2*j+1) << (4+j) ;
        }
//		Uart_Printf ("datup = %#x,datdn = %#x,odd[%d] = %#x,even[%d] = %#x\n",datup,datdn,i,odd,i,even);
        *(dotbuf+i) = odd ;
        *(dotbuf+16+i) = even ;
    }
//	return 0 ;
}

void compress_x(unsigned char *data_x)
{
    unsigned char i ;
    for (i=0;i<16;i++)
    {
        if (i%2)
        {
//		Uart_Printf("data_x[%d] =%#x,data_x[%d] =%#x \n",i-1,data_x[i-1],i,data_x[i]);
//		Uart_Printf("data_x[%d] =%#x,data_x[%d] =%#x \n",16+i-1,data_x[16+i-1],16+i,data_x[16+i]);
            data_x[i-1] = data_x[i-1]|data_x[i];
            //			data_x[i] = 0;
            data_x[i]	= data_x[16+i-1]|data_x[16+i];

            //			data_x[16+i-1] = data_x[16+i-1]|data_x[16+i];
            //			data_x[16+i] = 0;
            //	Uart_Printf("data_x[%d] =%#x,data_x[%d] =%#x\n\n",i-1,data_x[i-1],i,data_x[i]);
        }
    }
}


void compress(unsigned char *data)
{
    compress_y(data);
    compress_x(data);
}

#endif

/*
字模：横向->纵向,对8个字节操作。
*/

void dot_switch (unsigned char *uch)
{
    int i ;
    int j ;
    unsigned char rst[8];

    memset (rst,0,8);
    for (i=0; i<8; i++)
    {
        /*
        		rst[i] |= get_byte_bit(*(uch+0),i)<< 7 ;
        		rst[i] |= get_byte_bit(*(uch+1),i)<< 6 ;
        		rst[i] |= get_byte_bit(*(uch+2),i)<< 5 ;
        		rst[i] |= get_byte_bit(*(uch+3),i)<< 4 ;
        		rst[i] |= get_byte_bit(*(uch+4),i)<< 3 ;
        		rst[i] |= get_byte_bit(*(uch+5),i)<< 2 ;
        		rst[i] |= get_byte_bit(*(uch+6),i)<< 1 ;
        		rst[i] |= get_byte_bit(*(uch+7),i)<< 0 ;
        */
        for (j=0; j<8 ;j++)
            //		rst[i] |= get_byte_bit(*(uch+j),i) << 7-j ;		//right ->left  up:high bit
            //		rst[i] |= get_byte_bit(*(uch+j),7-i) << 7-j ;	//left ->right  up:high bit
            rst[i] |= get_byte_bit(*(uch+j),7-i) << j ;		  //left ->right  up:low bit
    }
    for (i =0 ;i<8; i++)
        Uart_Printf ("rst[%d]=%#x\n",i,rst[i]);
    memcpy(uch,rst,8);
}

void dot16_switch (unsigned char *dot32)
{
    int i ;
    int j ;
    unsigned char rst[32];

    memset (rst,0,32);
    for (i=0; i<8; i++)
    {
        rst[i] 	= *(dot32 + 2*i);
        rst[8+i] = *(dot32 + 2*i+1);
        rst[16+i] = *(dot32+16 + 2*i);
        rst[24+i] = *(dot32+16 + 2*i+1);
    }
    dot_switch(rst);
    dot_switch(rst+8);
    dot_switch(rst+16);
    dot_switch(rst+24);

    memcpy(dot32,rst,32);
}
int gb18030_16x16_rd (unsigned char *chinese,unsigned char *dotdat)
{
    unsigned char qm,wm;
    unsigned int offset ;
    FILE *gb18030;


    qm  = *(chinese) ;
    wm  = *(chinese+1) ;
    if ( (qm < 0x80)||(wm < 0x40))  return -1;

    if ((qm >= 0xB0)&&(wm >= 0xA1))
    {
        qm = qm - 0xB0;
        wm = wm - 0xA1;
        offset = qm * 94 + wm;
    }
    else if (qm <= 0xA0)
    {
        qm = qm - 0x81;
        wm = wm - 0x40;
        offset = qm * 190 + wm + 6768;
        if (wm > 0x3E)
            offset --;
    }
    else if ((qm >= 0xAA)&&(wm <= 0xA0))
    {
        qm = qm - 0xAA;
        wm = wm - 0x40;
        offset = qm * 96 + wm + 12848;
    }

    offset *= 32;

    if ((gb18030=fopen("GB18030.bin","rb"))==NULL)
        Uart_Printf("Can't open GB18030.bin.");

    fseek(gb18030,offset,SEEK_SET);
    fread(dotdat,32,1,gb18030);
    fclose(gb18030);

}

int wprint(fmode_t en_font,fmode_t ch_font,unsigned char *src)
{
    int rtn = 0;
    unsigned char i,j,k;
    unsigned char pnt = 1;
    unsigned char kdata[2][420];
    unsigned char rddata[32];
    unsigned char ucdata ;
//	fmode_t en_font;
//	fmode_t ch_font;
    int fd ;

//	en_font = FONT_5X7_EN ;
//	ch_font = FONT_16X16_GB2312 ;

    Uart_Printf("wprint:%s\n",src);
    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",DRVNAME) ;
        return -1 ;
    }

    i = 0;
    j = 0;
    k = 0;
    while ( ucdata = *(src+i) )
    {

        if (ucdata < 0x20)   //control char
        {


        }
        else if (ucdata >= 0x20 && ucdata < 0x80)
        {

            if (en_font == FONT_5X7_EN)
            {
                rd_dat_5x7en(*(src+i),rddata);
                //		memcpy(&kdata[0][pnt],rddata,6);

                memset(&kdata[0][pnt],0,6);
                memcpy(&kdata[1][pnt],rddata,6);
                pnt += 6;
                i++;
            }
            else if (en_font == FONT_7X9_EN)
            {
                rd_dat_7x9en(*(src+i),rddata);
                memcpy (&kdata[0][pnt],rddata,8);
                memcpy (&kdata[1][pnt],rddata+8,8);
                pnt += 8 ;
                i++;
            }
            else{}

        }
        else
        {

            if (ch_font == FONT_16X16_GB2312)
            {
                gb2312_16x16rd(src+i,rddata) ;
                memcpy (&kdata[0][pnt],rddata,16);
                memcpy (&kdata[1][pnt],rddata+16,16);
                pnt += 16 ;
            }
            else if (ch_font == FONT_8X8_GB2312)
            {
                gb2312_16x16rd(src+i,rddata) ;
                compress(rddata);
                //	memcpy (&kdata[0][pnt],rddata,16);
                memset (&kdata[0][pnt],0,16);
                memcpy (&kdata[1][pnt],rddata,16);
                pnt += 16 ;
            }
            else if (ch_font == FONT_16X16_GB18030 )
            {
                gb18030_16x16_rd(src+i,rddata);
                dot16_switch(rddata);
                memcpy (&kdata[0][pnt],rddata,16);
                memcpy (&kdata[1][pnt],rddata+16,16);
                pnt += 16 ;
            }
            else {}
            i += 2;
        }
    }

    kdata[0][0]=pnt;
    kdata[1][0]=pnt;
    if (ch_font == FONT_8X8_GB2312 && en_font == FONT_5X7_EN )
    {
// 		ioctl(fd,MU110_PRINT_STRING_CH8X8,kdata[0]);
        ioctl(fd,MU110_PRINT_STRING_CH8X8,kdata[1]);
    }
    else
    {
        ioctl(fd,MU110_PRINT_STRING,kdata[0]);
        ioctl(fd,MU110_PRINT_STRING,kdata[1]);
    }
    close(fd);
    return rtn;
}


unsigned char OSPRN_XPrintASCII(unsigned char ucFont,unsigned char *pucPtr)
{
    Os__print_EX(ucFont, pucPtr);
}






int print_main(int argc,char *argv[])
{

    int fd ;
    unsigned long paper = 15;
    unsigned char data ;
    unsigned char data_en[] = "ABCDEFGHPLAKDI  12345678" ;
    unsigned char data_gb[] = "上海杉德金卡科技有限公司" ;
    unsigned char data_chen[] ="中华人民共和国12345 chinese" ;
    unsigned char data_tst[] = {0x55,0x55,0x55,0x55,0xaa,0xaa,0xaa,0xaa};
//		unsigned char data_gb[] = "德" ;
    int rst ;
    int status ;
    int i ;

    if (argc <2)
    {
        Uart_Printf ("input error.\n");
        return -1 ;
    }
    status = atoi(argv[1]);
    Uart_Printf("status = %d\n",status);
    fd = open(FILENAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("open %s error.\n",FILENAME) ;
        return -1 ;
    }

    switch (status)
    {

    case 1 :	/*设置数据总线为0x55*/
        data = 0x55 ;
        while (1)
        {
            ioctl (fd,MU110_DATA_TEST,data) ;
            //	sleep(1);
        }
        break ;
    case 2 :	/*测试使能信号ST*/
        data = 0x00 ;
        while (1)
        {
            ioctl (fd,MU110_ST_TEST,data) ;
//		sleep(1);
        }
        break ;
    case 3 :	/*设置马达控制信号为0x55,测试控制，数据线用*/
        data = 0x55 ;
        while (1)
        {
            ioctl (fd,MU110_MOTO_CTL_TEST,data) ;
            sleep(1);
        }
        break ;

    case 4 :	/*读数据测试,正确值为0xe2*/
        while (1)
        {
            ioctl (fd,MU110_SYSCS_TEST,data) ;
            sleep(1);
        }
        Uart_Printf ("the test data sys_cs :0x%2x\n",data);
        break ;

    case 5 :	/*打印*/
        //	while (1) {
        rst = write(fd, data, sizeof(data) );
        if (rst)
            Uart_Printf("print %d characters \n") ;
        //		sleep(1);
//		}
        break ;

    case 6 :		/*读打印机状态*/
        while (1)
        {
            ioctl (fd,MU110_STATE_CTL,data) ;
            sleep(1);
        }
        Uart_Printf ("the print control status :0x%2x\n",data);
        break;

    case 7 :	/*走纸马达测试，向前走到黑标处。*/
        Uart_Printf("feed paper...\n");
        //	while (1){
        ioctl(fd,MU110_FEED_PAPER,data);
        //		sleep(1);
        //	}
        break ;
    case 8 :/*carriage马达向左运动*/
        ioctl(fd,MU110_CARRIAGE_TOLEFT_TEST,data);
        break;
        /*carriage马达向右运动*/
    case 9 :
        ioctl(fd,MU110_CARRIAGE_TORIGHT_TEST,data);
        break;

    case 10 ://到达打印开始位置
        ioctl(fd,MU110_CARRIAGE_START_POS,data);
        break ;
    case 11 ://走纸
        //	ioctl(fd,MU110_FEED_PAPER,paper);
        Os__prn_forward(15);
        break ;
    case 12 ://退纸
        Os__prn_backward(15);
        //	ioctl(fd,MU110_BACK_PAPER,paper);
        break ;
    case 13 ://黑标
        Os__black_mark();
        break ;
    case 14 :
        Os__print_EX(FONT_5X7_EN,data_en);
        break ;
    case 15 :
        Os__print_EX( FONT_7X9_EN ,data_en);
        break ;
    case 16 :
        Os__print_EX(FONT_16X16_GB2312 ,data_gb);
        break ;
    case 17 :
        Os__print_EX(FONT_8X8_GB2312 ,data_gb);
        break ;
    case 18 :
        dot_switch(data_tst);
        break ;
    case 19 :
        wprint(FONT_7X9_EN,FONT_16X16_GB2312,data_chen);
        wprint(FONT_5X7_EN,FONT_16X16_GB2312,data_chen);
        wprint(FONT_5X7_EN,FONT_8X8_GB2312,data_chen);
//			wprint(FONT_7X9_EN,FONT_8X8_GB2312,data_chen);
        break ;
    default :
        break ;
    }
    close (fd);
    return 0 ;
}
