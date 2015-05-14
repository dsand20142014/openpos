/*
 *  	  linux app lib 
 *
 *      Copyright (C) 2008 Sand, Inc.
 *      Author: Ivan Zhang w j
 *      bestzwj@163.com
 *
 *  This file is modem for POS common.c .support sync & async protocol 
 *  Can be use in anyway! any question please ask me . i will give you explain
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/fs.h>

#include "lcd.h"

static unsigned char common_readGB18030Dot16(unsigned char ucQM,unsigned char ucWM,unsigned char *pucPtr)
{
	 int fd_zk; 
	 off_t currpos;
        unsigned int uiOffset;
        unsigned char aucDotBuf[32];

        if((ucQM < 0x80)||(ucWM < 0x40))
           						     return 0;
		
        if((ucQM >= 0xB0)&&(ucWM >= 0xA1)) {
                ucQM = ucQM - 0xB0;
                ucWM = ucWM - 0xA1;
                uiOffset = ucQM * 94 + ucWM;
        }
	 else if(ucQM <= 0xA0) {
                ucQM = ucQM - 0x81;
                ucWM = ucWM - 0x40;
                uiOffset = ucQM * 190 + ucWM + 6768;
                if(ucWM > 0x3E)
                        uiOffset --;
        }
	 else if((ucQM >= 0xAA)&&(ucWM <= 0xA0)) {
                ucQM = ucQM - 0xAA;
                ucWM = ucWM - 0x40;
                uiOffset = ucQM * 96 + ucWM + 12847;
        }

        uiOffset *= 32;
        
        fd_zk = open("/fonts/jf3s1516.hz",O_RDONLY);
	 if(fd_zk<0) {
                   Uart_Printf("no zk\n");
		     return 1;
	 }
	 	           
  	 currpos = lseek(fd_zk,uiOffset,SEEK_SET);
	 if(currpos <0)
	               return 1;
  	 if(read(fd_zk,aucDotBuf,32)<0)
         						return 1;
        if( pucPtr )
                memcpy(pucPtr,aucDotBuf,32);

         return 0;
}

void Os__clr_display(unsigned char line)
{
    int lcd_fd;	
	
    lcd_fd = open("/dev/lcd",O_RDWR);	
    if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    }

    if((line >4)&&(line!=0xFF))
             return;

    ioctl(lcd_fd,LCD_CLR_DISPLAY,&line);

    close(lcd_fd);
	
    return;
}

void Os__display(unsigned char line,unsigned char column,char *Text)
{
    struct display_struct display_p;
    unsigned char len;	
    int lcd_fd;	
	
    lcd_fd = open("/dev/lcd",O_RDWR);	
    if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    }

    len = strlen(Text);

    if((line>4)||(column>16))
                             		return;

    display_p.line = line;
    display_p.column = column;
    display_p.pText = Text;
    display_p.text_len = len;

    ioctl(lcd_fd,LCD_DISPLAY,&display_p);

    close(lcd_fd);
	
    return;
}

static void Os__display_6X8(unsigned char line,unsigned char column,char *Text)
{
    struct display_struct display_p;
    unsigned char len;	
    int lcd_fd;	
	
    lcd_fd = open("/dev/lcd",O_RDWR);	
    if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    }

    len = strlen(Text);

    if((line>8)||(column>22))
                             		return;

    display_p.line = line;
    display_p.column = column;
    display_p.pText = Text;
    display_p.text_len = len;

    ioctl(lcd_fd,LCD_DISPLAY_8X6,&display_p);

    close(lcd_fd);
	
}

unsigned char Os__light_on(void)
{
 
    int lcd_fd;	
	
    lcd_fd = open("/dev/lcd",O_RDWR);	
	
    if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    }	

    ioctl(lcd_fd,LCD_LIGHT_ON);
    close(lcd_fd);

    return 0;
}

unsigned char Os__light_off(void)
{

    int lcd_fd;	
	
    lcd_fd = open("/dev/lcd",O_RDWR);	
    if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    }

   ioctl(lcd_fd,LCD_LIGHT_OFF);
   
   close(lcd_fd);
   
   return 0;
}

unsigned char Os__GB2312_display(unsigned char line,unsigned char column,unsigned char *Text)
{
 	 unsigned char dotbuff[32];
	 unsigned char fuck[2];
        unsigned char high,low;
	 unsigned char *p;
	 unsigned char len,i;  
	
        memset(dotbuff,0x00,32);
		
        p = Text;
        len = strlen(Text);

        if(line>4) {
	           return 1;
        }
        if(2*column+len>17)
		    len = 16-2*column;

        for(i=0;i<len;) {		
        			high =  *p;
					
				if(high<0x80) {
					     fuck[0] =  high;
					     fuck[1] = '\0';
                                        Os__display(line,column*2+i,fuck);
					     p++;
                                        i++;
                                        continue;
				}
					   
	 			low = *(p+1);		
				memset(dotbuff,0x00,32);
        			if(common_readGB18030Dot16(high,low,dotbuff))
													return 0x01;
       			Os__graph_display(line,(i*8+column*16),dotbuff,32);
				p=p+2;
				i +=2;
        }
	
        return 0;
}

unsigned char Os__GB2312_disp(unsigned char Line, unsigned char * Text)
{
 	return Os__GB2312_display(Line,0,Text);
}

unsigned char *Os__conv_ascii_GB2312(unsigned char *ptAscii)
{
   	Uart_Printf("Os__conv_ascii_GB2312 not support ! \n");
   
  	return ptAscii;
}


/* new add, zhangy */
unsigned char *Os__conv_utf8_GB2312(unsigned char *ptUTF8)
{
   	//Uart_Printf("%s not support ! \n", __func__);
	return NULL;
}



unsigned char Os__graph_display(unsigned char line,unsigned char column,unsigned char *Drawing,unsigned char size)
{
        struct graph_display_struct graph_display_p;
   	  int lcd_fd;	
	
   	  lcd_fd = open("/dev/lcd",O_RDWR);	
   	  if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return 1;
    	  }	

  	 if((line>4)||(column>128))
                             		return;
	 graph_display_p.line = line;
	 graph_display_p.column = column;
	 graph_display_p.Drawing = Drawing;
        graph_display_p.size = size;

        ioctl(lcd_fd,LCD_GRAPH_DISPLAY,&graph_display_p);

        close(lcd_fd);

        return 0;
}

void Os__set_contrast(unsigned char value)
{
 	 int lcd_fd;	
	 
	 lcd_fd = open("/dev/lcd",O_RDWR);	
	  
   	 if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    	 }	
	  
  	 ioctl(lcd_fd,LCD_SET_CONTRAST,&value);

  	 close(lcd_fd);

        return;
}


void OSMMI_clr_Display(unsigned char ucFont,unsigned char line)
{
   	  int lcd_fd;	
	
   	  lcd_fd = open("/dev/lcd",O_RDWR);	
   	  if(lcd_fd<0) {
                 Uart_Printf("open lcd error\n");
		   return;
    	  }	

 	 if((line >8)&&(line!=0xFF)) {
	 					 close(lcd_fd);
            					 return;
 	 }

	 switch(ucFont) {
			case 0x31:
   				 ioctl(lcd_fd,LCD_CLR_DISPLAY,&line);
 				 break; 
                     case 0x30:
				 ioctl(lcd_fd,LCD_CLR_ONE_LINE,&line);
                             break;
			default:
				 close(lcd_fd);
				 return;
        }

	 close(lcd_fd);
	 
   	 return;
}

unsigned char OSMMI_DisplayASCII (unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * Text)
{	
     switch(ucFont) {
			case 0x31:
				 Os__display(ucRow,ucCol,Text);
 				 break; 
                     case 0x30:
     				 Os__display_6X8(ucRow,ucCol,Text);
                             break;
			default:
				 return 1;
     }
	 
     return 0;
}


int main(int argc,char *argv[])
{
       unsigned char i;
       unsigned char err;

       Os__light_on();
       sleep(3);
	Os__light_off();
       sleep(3);
       Os__light_on();
       sleep(3);
   
	Os__clr_display(0xFF); 
	
	Os__display(3,0,"Author:");
       Os__display(3,8,"Bestzwj");
	Os__GB2312_display(1,0,"A:ÉÏº£É¼µÂbestzwj:$");  
	OSMMI_DisplayASCII(0x30,0,0,"abcdefghijklmnopqrstu");

       return 0;
}










