#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ostools.h"
#include "devSignal.h"
#include "printer.h"
#include "sysparam.h"

#define SIGN_MAXBMP 40560
#define DRVNAME  "/dev/printer"


/** 定义IOCTL命令 */
#define PRINTER_IOC_MAGIC               'S'
#define PRINTER_FEED_PAPER              _IOW(PRINTER_IOC_MAGIC,  1, int)//走纸；
#define PRINTER_PRINT_VCC               _IOW(PRINTER_IOC_MAGIC,  2, int)//未实现；
#define PRINTER_PRINT_FONT              _IOW(PRINTER_IOC_MAGIC,  3, int)//未实现；
#define PRINTER_PRINT_TEST              _IOW(PRINTER_IOC_MAGIC,  4, int)//测试；
#define PRINTER_PRINT_CHECKPAPER        _IOWR(PRINTER_IOC_MAGIC, 5, int)//检纸；
#define PRINTER_PRINT_CFGPARAM          _IOWR(PRINTER_IOC_MAGIC, 6, int)//未实现；
#define PRINTER_PRINT_CLEANBUF          _IOWR(PRINTER_IOC_MAGIC, 7, int)//清缓存；
#define PRINTER_PRINT_CHKSTATUS         _IOWR(PRINTER_IOC_MAGIC, 8, int)//异常状态；
#define PRINTER_PRINT_SETVF             _IOWR(PRINTER_IOC_MAGIC, 9, int)//未实现；
#define PRINTER_PRINT_END               _IOWR(PRINTER_IOC_MAGIC,10, int)//是否打印结束；
#define PRINTER_PRINT_SETMINHEATTIME    _IOWR(PRINTER_IOC_MAGIC,11, int)//最小加热时间，不能低于400，打印机走纸不动，默认535；
#define PRINTER_PRINT_SETMAXHEATTIME    _IOWR(PRINTER_IOC_MAGIC,12, int)//最大加热时间，不高于1200，默认850，过高极易过热；
#define PRINTER_PRINT_SETLINEFEEDTIME   _IOWR(PRINTER_IOC_MAGIC,13, int)//走纸时间，默认400。很少需要改动，影响走纸速度；
#define PRINTER_PRINT_SETADDHEATTIME    _IOWR(PRINTER_IOC_MAGIC,14, int)//加热补偿时间，默认800，在空白行之后自动加在当前加热时间上，一般不用改动；
#define PRINTER_PRINT_SETOVERHEATTEMP   _IOWR(PRINTER_IOC_MAGIC,15, int)//过热温度，默认1500，越大过热温度越低，其值与温度近似线性关系；
#define PRINTER_PRINT_SETRETNORMALTEMP  _IOWR(PRINTER_IOC_MAGIC,16, int)//回复非过热状态温度，默认2000，必须大于过热温度；
#define PRINTER_IOC_MAXNR               16


int dataLength;
unsigned char ucBmpData[SIGN_MAXBMP];
unsigned char ucPrintData[320][48];

static unsigned char  prnBuf[2400][48];/*100 24x24 */
static unsigned char zhbf[72],enbf[72];

extern unsigned char FONT_ASCII1224[][48];
extern unsigned char FONT_ASCII816_2[][16];
extern unsigned char zh_FONT_2424[3][128*72];
extern unsigned char zh_FONT_1616[3][128*32];

typedef int (*prnfunc_t)(unsigned char *,unsigned char *);

/*
typedef struct variablePrinter{
	unsigned short	sum;
	unsigned char	*Text;
	unsigned char	*ucGBFont;
	unsigned char	*ucASCIIFont;
}VARIAPRINTER;
*/

#define  get_byte_bit(uc8,bit)				(((uc8)>>(bit)) & 0x01)


static void colExbit(unsigned char ch,unsigned char rst[])
{
	int i=0;
	for(i=0; i<4; i++)
    {
		rst[0] |= (((ch>>i)&0x01)<<2*i)|(((ch>>i)&0x01)<<(2*i +1));
		rst[1] |= (((ch>>(4+i))&0x01)<<2*i)|(((ch>>(4+i))&0x01)<<(2*i +1));
    }
}

static void Modify24Dot(unsigned char *DotDataBuf)
{
	unsigned int dotbuf[24];
	unsigned int Moddotbuf[24];
	unsigned char i,j;
	for(i=0; i<24; i++)
	{
		Moddotbuf[i] = 0;
		dotbuf[i] = 0;
	}
	//memset(dotbuf,0x00,sizeof(dotbuf))；
	//memset(Moddotbuf,0x00,sizeof(Moddotbuf))；

	for(i=0; i<24; i++)
	{
		dotbuf[i] += *(DotDataBuf+3*i);
		dotbuf[i] <<= 8;
		dotbuf[i] += *(DotDataBuf+3*i+1);
		dotbuf[i] <<= 8;
		dotbuf[i] += *(DotDataBuf+3*i+2);
	}

	for(j=0; j<24; j++)
	{
		for(i=j; i<24; i++)
			Moddotbuf[j] |= (dotbuf[i] & (0x00800000>>j))>>(i-j);
		for (i=0; i<j; i++)
			Moddotbuf[j] |= (dotbuf[i] & (0x00800000>>j))<<(j-i);
	}

	for(i=0; i<24; i++)
	{
		*(DotDataBuf+3*i+2) = (Moddotbuf[i] & 0xff);
		*(DotDataBuf+3*i+1) = ((Moddotbuf[i]>>8) & 0xff);
		*(DotDataBuf+3*i) = ((Moddotbuf[i]>>16) & 0xff);
	}
}

static int gb2312_24x24(unsigned char *chinese,unsigned char *dotdat)
{
	unsigned char qm,wm;
	unsigned long offset;
	FILE *gb2312;

	if((gb2312=fopen("/fonts/GB2312_24x24.bin","rb"))==NULL)
	{
		return -1;
	}

	qm = *(chinese) - 0xa1;
	wm = *(chinese+1)   - 0xa1;

	offset = ( (qm-15)*94 + wm )*72L;

	fseek(gb2312,offset,SEEK_SET);
	fread(dotdat,72,1,gb2312);
	fclose(gb2312);
	
	Modify24Dot(dotdat);
	return 0;
}

static int gb2312_16x16(unsigned char *chinese,unsigned char *dotdat)
{
	unsigned char qm,wm;
	unsigned int offset;
	FILE *gb2312;

	qm = *(chinese);
	wm = *(chinese+1);
	if((qm < 0x80)||(wm < 0x40))
		return 0;
	if((qm >= 0xB0)&&(wm >= 0xA1))
	{
		qm = qm - 0xB0;
		wm = wm - 0xA1;
		offset = qm * 94 + wm;
	}
	else if(qm <= 0xA0)
	{
		qm = qm - 0x81;
		wm = wm - 0x40;
		offset = qm * 190 + wm + 6768;
		if(wm > 0x3E)
			offset --;
	}
	else if((qm >= 0xAA)&&(wm <= 0xA0))
	{
		qm = qm - 0xAA;
		wm = wm - 0x40;
		offset = qm * 96 + wm + 12847;
	} 

	offset *= 32;

	if((gb2312=fopen("/fonts/jf3s1516.hz","rb"))==NULL)
	{
		return -1;
	}
	fseek(gb2312,offset,SEEK_SET);
	fread(dotdat,32,1,gb2312);
	fclose(gb2312);
	return 0;
}

static int ascii12x24(unsigned char *ch,unsigned char *dotdat)
{
	memcpy(dotdat,FONT_ASCII1224[*ch - ' '],48);
}

static int ascii8x16(unsigned char *ch, unsigned char *dotdat)
{
	memcpy(dotdat,FONT_ASCII816_2[*ch],16);
}

int Os__variable_printf(VARIAPRINTER *vp)
{
	int index,trsflags = 0; /*0:default,
                       		1,twice col,
                       		2,twice row
                       		3,both twice col and row
                       		4,both fourth col and row
                       				*/
	int en = 2; /*1: 8x16 2: 12x24(default)*/
	int zh = 2; /*1: 8x16 2: 12x24(default)*/
	int en_row,en_col,zh_row,zh_col;
	int high;
	int x=0;
	int y=0;
	int m,n;
	int fd,rtn;
	int val;
	int state=0;
	long timeout; 
	int startLine = 0;
	unsigned char print_status;
	unsigned char rst[2];
	unsigned short j,i;
	int temp = 0;

	prnfunc_t zh_func=gb2312_24x24,en_func=ascii12x24;

	if(NULL == vp)
		return -1;

	fd = open(DRVNAME, O_RDWR);
	if(fd < 0 )
	{
		return -1;
	}
	
	Uart_Printf("sum = %d\r\n",vp->sum);
	memset((unsigned char *)prnBuf,0,2400*48);

	//find the highest font in the array.
	high = 16;
	for(index=0;index<vp->sum;index++)
	{
		
		if(vp->ucGBFont[index] == 0x1e ||
		   vp->ucGBFont[index] == 0x0e )	
		{
			high = 32;
			break;
		}
		if(vp->ucGBFont[index] == 0x1b ||
		   vp->ucGBFont[index] == 0x1d ||	
		   vp->ucGBFont[index] == 0x0d ||	
		   vp->ucGBFont[index] == 0x0c)
		{
			temp = 24;
			if(temp > high)
				high = temp;
		}
	}
	Uart_Printf("high = %d\r\n",high);

	for(index=0;index<vp->sum;)
	{
		Uart_Printf("index = %d\r\n",index);
		//Uart_Printf("ucGBFont[%d] = %d\r\n",index,vp->ucGBFont[index]);
		switch(vp->ucGBFont[index])
		{
			case 0x01:/*zh:16X16 en:8*16*/
				trsflags = 0;
				en = 1;
				zh = 1;
				zh_func = gb2312_16x16;
				en_func = ascii8x16;
				break;
			case 0x1c:/*zh:16X16 en:8*16  (both twice COL)*/
				trsflags = 1;
				en = 1;
				zh = 1;
				zh_func = gb2312_16x16;
				en_func = ascii8x16;
				break;
			case 0x1d:/*zh:16X16 en:8*16  (both twice ROW)*/
				trsflags = 2;
				en = 1;
				zh = 1;
				zh_func = gb2312_16x16;
				en_func = ascii8x16;
				break;
			case 0x1e:/*zh:16X16 en:8*16  (both twice COL and ROW)*/
				trsflags = 3;
				en = 1;
				zh = 1;
				zh_func = gb2312_16x16;
				en_func = ascii8x16;
				break;
			case 0x1b:/*zh:16X24 (fourth col and row)*/
				trsflags = 4;
				if (vp->ucASCIIFont[index] == 0x31)
				{
					en = 1;
					en_func = ascii8x16;
				}
				else
				{
					en = 2;
					en_func = ascii12x24;
				}
				zh = 2;
				zh_func = gb2312_24x24;
				break;
			case 0x0c:/*zh:16X24 (twice col)*/
				trsflags = 1;
				if(vp->ucASCIIFont[index] == 0x31)
				{
					en = 1;
					en_func = ascii8x16;
				}
				else
				{
					en = 2;
					en_func = ascii12x24;
				}
				zh = 2;
				zh_func = gb2312_24x24;
				break;
			case 0x0d:/*zh:16X24 (twice  row)*/
				trsflags = 2;
				if(vp->ucASCIIFont[index] == 0x31)
				{
					en = 1;
					en_func = ascii8x16;
				}
				else
				{
					en = 2;
					en_func = ascii12x24;
				}
				zh = 2;
				zh_func = gb2312_24x24;
				break;
			case 0x0e:/*zh:16X24 (twice col and row)*/
				trsflags = 3;
				if(vp->ucASCIIFont[index] == 0x31)
				{
					en = 1;
					en_func = ascii8x16;
				}
				else
				{
					en = 2;
					en_func = ascii12x24;
				}
				zh = 2;
				zh_func = gb2312_24x24;
				break;
			default:  /*zh:16X24*/
				//trsflags = 0;en = 2;zh = 2;
				en = 2;
				zh = 2;
				zh_func = gb2312_24x24;
				en_func = ascii12x24;
				break;
		}
		//Uart_Printf("trsflags = %d\r\n",trsflags);

		if(zh == 2)
		{
			zh_row=24;
			zh_col=3;
		}
		else
		{
			zh_row=16;
			zh_col=2;
		}
		if(en == 2)
		{
			en_row=24;
			en_col=2;
		}
		else
		{
			en_row=16;
			en_col=1;
		}
		//if(high == 24)
		//	zh_col = 3;
	
		//Uart_Printf("zh_row = %d\r\n",zh_row);
		//Uart_Printf("zh_col = %d\r\n",zh_col);
		//Uart_Printf("en_row = %d\r\n",en_row);
		//Uart_Printf("en_col = %d\r\n",en_col);
		//high = zh_row>en_row?zh_row:en_row;
		//x=0;
		//y=0;
		//memset((unsigned char *)prnBuf,0,2400*48);

		if(vp->Text[index]>=0x80)   /*zh*/
		{
			switch (trsflags)
			{
				case 4:   /*both fourth col and row*/
					x+=4*zh_col;		//????
					break;
				case 3:   /*both twice col and row*/
				{
		       		 	if(x>48-2*zh_col)
					{
						x = 0;    /*48-6*/
		          			y += 2;
					}
					memset((unsigned char *)zhbf,0,zh_row*zh_col);
					// zh_func(&Text[index],zhbf);
					//add by garvey at2012.11.26
				
					if(vp->Text[index] >= 0xA1 && vp->Text[index] <= 0xA3)
					{
						int aryRow = vp->Text[index]%0xA1;
						int aryCol = vp->Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_1616[aryRow][aryCol*32],32);
					}
					else
					{
	                			zh_func(&(vp->Text[index]),zhbf);
					}
					for(m = 0; m<zh_row; m++)
						for (n = 0; n<zh_col; n++)
						{
							memset(rst,0,2);
							colExbit(zhbf[m*zh_col+n],rst);
							prnBuf[high*y+2*m][x+2*n] = rst[1];
							prnBuf[high*y+2*m][x+2*n+1] = rst[0];

							prnBuf[high*y+2*m+1][x+2*n] = rst[1];
							prnBuf[high*y+2*m+1][x+2*n+1] = rst[0];
/*
							prnBuf[high*y+2*(startLine+m)][x+2*n] = rst[1];
							prnBuf[high*y+2*(startLine+m)][x+2*n+1] = rst[0];

							prnBuf[high*y+2*(startLine+m+1)][x+2*n] = rst[1];
							prnBuf[high*y+2*(startLine+m+1)][x+2*n+1] = rst[0];
*/
       				             	}
	
					x+=2*zh_col;
					break;
				}
				case 2:/*twice row*/
				{
            				if (x>48-zh_col)
                			{
		       		     		x=0;    /*48-3*/
		            			y += 2;
                			}
					memset((unsigned char *)zhbf,0,zh_row*zh_col);
                	
                			if(vp->Text[index] >= 0xA1 && vp->Text[index] <= 0xA3)
					{
						int aryRow = vp->Text[index]%0xA1;
						int aryCol = vp->Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_1616[aryRow][aryCol*32],32);
						for(m=0;m<zh_row;m++)
							for(n=0;n<zh_col;n++)
								prnBuf[high*y+m][x+n] = zhbf[m*zh_col+n];
					}
					else
					{
	                			zh_func(&(vp->Text[index]),zhbf);
	                
	              				for(m = 0; m<zh_row; m++)
	                			for (n = 0; n<zh_col; n++)
	                    			{
			                		prnBuf[high*y+2*m][x+n] = zhbf[m*zh_col+n];
			                		prnBuf[high*y+2*m+1][x+n] = zhbf[m*zh_col+n];
	                    			}
	                		}
                			x+=zh_col;
                			break;
				}
            			case 1:/*twice col*/
				{
           				if (x>48-2*zh_col)
		            		{
		            			x=0;    /*48-6*/
						if(high > 16)	
							y += 2;
						else 
		            				y++;
		            		}
            				memset((unsigned char *)zhbf,0,zh_row*zh_col);
					if(vp->Text[index] >= 0xA1 && vp->Text[index] <= 0xA3)
					{
						int aryRow = vp->Text[index]%0xA1;
						int aryCol = vp->Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_1616[aryRow][aryCol*32],32);
					}
					else
					{
	                			zh_func(&(vp->Text[index]),zhbf);
                			}

              				for(m = 0; m<zh_row; m++)
                				for(n = 0; n<zh_col; n++)
		                		{
							memset(rst,0,2);
							colExbit(zhbf[m*zh_col+n],rst);
							prnBuf[high*y+m][x+2*n] = rst[1];
							prnBuf[high*y+m][x+2*n+1] = rst[0];
		                		}
                			x+=2*zh_col;
                			break;
				}
				case 0:/**/
				default:
				{
           		 		if(x>48-zh_col)
		       			{
		            			x=0;    /*48-3*/
						if(high > 16)
							y += 2;
						else 
		            				y++;
		            		}
              				memset((unsigned char *)zhbf,0,zh_row*zh_col);
              		
              				if(vp->Text[index] >= 0xA1 && vp->Text[index] <= 0xA3)
					{
						int aryRow = vp->Text[index]%0xA1;
						int aryCol = vp->Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_2424[aryRow][aryCol*72],72);
					}
					else
					{
	               				zh_func(&(vp->Text[index]),zhbf);
	                		}
	              			for(m = 0; m<zh_row; m++)
	              			//for(m = startLine; m<zh_row; m++)
	              				for(n = 0; n<zh_col; n++)
	       		         	   		//prnBuf[high*y+(2*startLine+m)][x+n] = zhbf[m*zh_col+n];
	       		         	   		prnBuf[high*y+m][x+n] = zhbf[m*zh_col+n];
                	
               				x+=zh_col;
             				break;
				}
            		}
        		index+=2;
        	}
		else if(vp->Text[index]>=0x20)   /*en*/
      		{
			Uart_Printf("english\r\n");
			switch(trsflags)
            		{
         			case 4:   /*both fourth col and row*/
           				break;
            			case 3:   /*both twice col and row*/
				{
       			       		if(x>48-2*en_col)
		       			{
		            			x=0;    /*48-2*/
		            			y += 2;
		        		}
					memset((unsigned char *)enbf,0,en_row*en_col);
					en_func(&(vp->Text[index]),enbf);
					for(m = 0; m<en_row; m++)
                 				for(n = 0; n<en_col; n++)
						{
			       		     		memset(rst,0,2);
			       	  	   		colExbit(enbf[m*en_col+n],rst);
			         			prnBuf[high*y+2*m][x+2*n] = rst[1];
						        prnBuf[high*y+2*m][x+2*n+1] = rst[0];
			            			prnBuf[high*y+2*m+1][x+2*n] = rst[1];
			           			prnBuf[high*y+2*m+1][x+2*n+1] = rst[0];
                    				}
		        		x+= 2*en_col;
		       	 		break;
				}
				case 2:   /*twice row*/
				{
           				if(x>48-en_col)
		        		{
						x=0;    /*48-2*/
						y += 2;
		        		}
               				memset((unsigned char *)enbf,0,en_row*en_col);

              				en_func(&(vp->Text[index]),enbf);
              				for(m = 0; m<en_row; m++)
             					for(n = 0; n<en_col; n++)
		            			{
							prnBuf[high*y+2*m][x+n] = enbf[m*en_col+n];
							prnBuf[high*y+2*m+1][x+n] = enbf[m*en_col+n];
					        }
       				         x+=en_col;
                			break;
				}
				case 1:   /*twice col*/
				{
					if (x>48-2*en_col)
					{
						x=0;    /*48-2*/
						if(high > 16)
							y += 4;
						else
							y += 2;
					}
					memset((unsigned char *)enbf,0,en_row*en_col);
					en_func(&(vp->Text[index]),enbf);
					for (m = 0; m<en_row; m++)
						for (n = 0; n<en_col; n++)
						{
							memset(rst,0,2);
							colExbit(enbf[m*en_col+n],rst);
							prnBuf[high*y+m][x+2*n] = rst[1];
							prnBuf[high*y+m][x+2*n+1] = rst[0];
						}
					x+= 2*en_col;
					break;
				}
				case 0:
				default:
				{
        				if(x>48-en_col)
			    		{
						x=0;
						if(y > 16)
							y += 4;
						else
							y += 2;
			    		}
					memset((unsigned char *)enbf,0,en_row*en_col);
					//Uart_Printf("en_row*en_col = %d\r\n",en_row*en_col);
					en_func(&(vp->Text[index]),enbf);
					//Uart_Printf("after en_func\r\n");
					//Uart_Printf("x = %d\r\n",x);
					//Uart_Printf("y = %d\r\n",y);
					for(m = 0; m<en_row; m++)
						for(n = 0; n<en_col; n++)
							prnBuf[high*y+m][x+n] = enbf[m*en_col+n];
					x+=en_col;
					break;
				}
			}
			index++;
		}
      		else
        	{
			index++;
			x+=en_col;
        	}
        	//Uart_Printf("x = %d\r\n",x);
		if(x > 48)
      		break;

	}
	
	if(x != 0)   //half of line -> a line
	{
		switch(trsflags)
        	{
      			case 4://both fourth col and row/
				y +=4;
				break;
    			case 3://both twice col and row/
				y +=2;
				break;
     			case 2://twice row/
         			y += 2;
         			break;
			case 1://twice col/
			case 0:
			default:
				y++;
				break;
        	}
	}
     
	//Uart_Printf("y = %d\r\n",y);
	//Uart_Printf("high = %d\r\n",high);
	rtn = write(fd,prnBuf,y*high*48);
   
	while(rtn < 0)
	{
    		//打印机BUF满
		if(rtn == ENOBUFS)
    		{
    			sleep(3);
		}
		else
		{
			//打印机过热或是缺纸
			while(Os__get_device_signal(DEV_PRINTER_STATE) != 0)
			{
				sleep(1);
			}
		}
		rtn = write(fd,prnBuf,y*high*48);
	}
    
	close(fd);

	return 0;

}

int ultimate_print(unsigned char ucASCIIFont, unsigned char ucGBFont, unsigned char *Text,int dlen)
{

	int index,trsflags = 0; /*0:default,
                       		1,twice col,
                       		2,twice row
                       		3,both twice col and row
                       		4,both fourth col and row
                       				*/
	int en = 2; /*1: 8x16 2: 12x24(default)*/
	int zh = 2; /*1: 8x16 2: 12x24(default)*/
	int en_row,en_col,zh_row,zh_col;
	int high,x,y;
	int m,n;
	int fd,rtn;
	int val;
	int state=0;
	long timeout; 
	unsigned char print_status;
	unsigned char rst[2];

	prnfunc_t zh_func=gb2312_24x24,en_func=ascii12x24;

	if(!Text)
	{
		return -1;
	}


	/*open device file.*/
	fd = open(DRVNAME, O_RDWR);
	if(fd < 0 )
	{
		return -1;
	}

	
    /*default*/
	//zh_func = gb2312_24x24; en_func = ascii12x24;
	switch(ucGBFont)
	{
		case 0x01:/*zh:16X16 en:8*16*/
			trsflags = 0;
			en = 1;
			zh = 1;
			zh_func = gb2312_16x16;
			en_func = ascii8x16;
			break;
		case 0x1c:/*zh:16X16 en:8*16  (both twice COL)*/
			trsflags = 1;
			en = 1;
			zh = 1;
			zh_func = gb2312_16x16;
			en_func = ascii8x16;
			break;
		case 0x1d:/*zh:16X16 en:8*16  (both twice ROW)*/
			trsflags = 2;
			en = 1;
			zh = 1;
			zh_func = gb2312_16x16;
			en_func = ascii8x16;
			break;
		case 0x1e:/*zh:16X16 en:8*16  (both twice COL and ROW)*/
			trsflags = 3;
			en = 1;
			zh = 1;
			zh_func = gb2312_16x16;
			en_func = ascii8x16;
			break;
		case 0x1b:/*zh:16X24 (fourth col and row)*/
			trsflags = 6;
			if (ucASCIIFont ==0x31)
			{
				en = 1;
				en_func = ascii8x16;
			}
			else
			{
				en = 2;
				en_func = ascii12x24;
			}
			//		 zh = 2;
			zh_func = gb2312_24x24;
			break;
		case 0x0c:/*zh:16X24 (twice col)*/
			trsflags = 1;
			if(ucASCIIFont ==0x31)
			{
				en = 1;
				en_func = ascii8x16;
			}
			else
			{
				en = 2;
				en_func = ascii12x24;
			}
			//		  zh = 2;
			zh_func = gb2312_24x24;
			break;
		case 0x0d:/*zh:16X24 (twice  row)*/
			trsflags = 2;
			if(ucASCIIFont ==0x31)
			{
				en = 1;
				en_func = ascii8x16;
			}
			else
			{
				en = 2;
				en_func = ascii12x24;
			}
			//		  zh = 2;
			zh_func = gb2312_24x24;
			break;
		case 0x0e:/*zh:16X24 (twice col and row)*/
			trsflags = 3;
			if(ucASCIIFont ==0x31)
			{
				en = 1;
				en_func = ascii8x16;
			}
			else
			{
				en = 2;
				en_func = ascii12x24;
			}
			//		  zh = 2;
			zh_func = gb2312_24x24;
			break;
		default:  /*zh:16X24*/
			//trsflags = 0;en = 2;zh = 2;
			zh_func = gb2312_24x24;
			en_func = ascii12x24;
			break;
    }

	if(zh == 2)
	{
		zh_row=24;
		zh_col=3;
	}
	else
	{
		zh_row=16;
		zh_col=2;
	}
	if(en == 2)
	{
		en_row=24;
		en_col=2;
	}
	else
	{
		en_row=16;
		en_col=1;
	}

	high = zh_row>en_row?zh_row:en_row;
	x=0;
	y=0;
	memset((unsigned char *)prnBuf,0,2400*48);


	for(index = 0; index<dlen;)   /*number of ch.*/
	{
		if(Text[index]>=0x80)   /*zh*/
		{
			switch (trsflags)
			{
				case 4:   /*both fourth col and row*/
					break;
					x+=4*zh_col;
				case 3:   /*both twice col and row*/
		        	if(x>48-2*zh_col)
				{
						x = 0;    /*48-6*/
		          		y += 2;
		            }
                	memset((unsigned char *)zhbf,0,zh_row*zh_col);
               		// zh_func(&Text[index],zhbf);
					//add by garvey at2012.11.26
					
					if(Text[index] >= 0xA1 && Text[index] <= 0xA3)
					{
						int aryRow = Text[index]%0xA1;
						int aryCol = Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_1616[aryRow][aryCol*32],32);
					}
					else
					{
	                	zh_func(&Text[index],zhbf);
					}
               		for(m = 0; m<zh_row; m++)
               			for (n = 0; n<zh_col; n++)
                    	{
							memset(rst,0,2);
							colExbit(zhbf[m*zh_col+n],rst);

							prnBuf[high*y+2*m][x+2*n] = rst[1];
							prnBuf[high*y+2*m][x+2*n+1] = rst[0];

							prnBuf[high*y+2*m+1][x+2*n] = rst[1];
							prnBuf[high*y+2*m+1][x+2*n+1] = rst[0];
                    	}

					x+=2*zh_col;
					break;
				case 2:/*twice row*/
            		if (x>48-zh_col)
                	{
		            	x=0;    /*48-3*/
		            	y += 2;
                	}
					memset((unsigned char *)zhbf,0,zh_row*zh_col);
                	
                	if(Text[index] >= 0xA1 && Text[index] <= 0xA3)
					{
						int aryRow = Text[index]%0xA1;
						int aryCol = Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_1616[aryRow][aryCol*32],32);
						for(m=0;m<zh_row;m++)
							for(n=0;n<zh_col;n++)
								prnBuf[high*y+m][x+n] = zhbf[m*zh_col+n];
					}
					else
					{
	                	zh_func(&Text[index],zhbf);
	                
	              		for(m = 0; m<zh_row; m++)
	                		for (n = 0; n<zh_col; n++)
	                    	{
			                	prnBuf[high*y+2*m][x+n] = zhbf[m*zh_col+n];
			                	prnBuf[high*y+2*m+1][x+n] = zhbf[m*zh_col+n];
	                    	}
	                }
                	x+=zh_col;
                	break;
            	case 1:/*twice col*/

           			if (x>48-2*zh_col)
		            {
		            	x=0;    /*48-6*/
		            	y++;
		            }
            		memset((unsigned char *)zhbf,0,zh_row*zh_col);
					if(Text[index] >= 0xA1 && Text[index] <= 0xA3)
					{
						int aryRow = Text[index]%0xA1;
						int aryCol = Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_1616[aryRow][aryCol*32],32);
					}
					else
					{
	                	zh_func(&Text[index],zhbf);
                	}

              		for(m = 0; m<zh_row; m++)
                		for(n = 0; n<zh_col; n++)
		                {
							memset(rst,0,2);
							colExbit(zhbf[m*zh_col+n],rst);
							prnBuf[high*y+m][x+2*n] = rst[1];
							prnBuf[high*y+m][x+2*n+1] = rst[0];
		                }
                	x+=2*zh_col;
                	break;

				case 0:/**/
				default:
            		if(x>48-zh_col)
		            {
		            	x=0;    /*48-3*/
		            	y++;
		            }
              		memset((unsigned char *)zhbf,0,zh_row*zh_col);
              		
              		if(Text[index] >= 0xA1 && Text[index] <= 0xA3)
					{
						int aryRow = Text[index]%0xA1;
						int aryCol = Text[index+1]%0xA1;
						memcpy(zhbf,&zh_FONT_2424[aryRow][aryCol*72],72);
					}
					else
					{
	               		zh_func(&Text[index],zhbf);
	                }
	              	for(m = 0; m<zh_row; m++)
	              		for(n = 0; n<zh_col; n++)
	                   		prnBuf[high*y+m][x+n] = zhbf[m*zh_col+n];
                	
               		x+=zh_col;
             		break;
            }

        	index+=2;

        }
		else if(Text[index]>=0x20)   /*en*/
      	{
			switch(trsflags)
            {
         		case 4:   /*both fourth col and row*/
           		break;
            case 3:   /*both twice col and row*/
              	if(x>48-2*en_col)
		        {
		            x=0;    /*48-2*/
		            y += 2;
		        }
				memset((unsigned char *)enbf,0,en_row*en_col);
				en_func(&Text[index],enbf);
				for(m = 0; m<en_row; m++)
                 	for(n = 0; n<en_col; n++)
					{
			            memset(rst,0,2);
			            colExbit(enbf[m*en_col+n],rst);
			            prnBuf[high*y+2*m][x+2*n] = rst[1];
			            prnBuf[high*y+2*m][x+2*n+1] = rst[0];
			            prnBuf[high*y+2*m+1][x+2*n] = rst[1];
			            prnBuf[high*y+2*m+1][x+2*n+1] = rst[0];
                    }
		        x+= 2*en_col;
		        break;
			case 2:   /*twice row*/
           		if(x>48-en_col)
		        {
					x=0;    /*48-2*/
					y += 2;
		        }
               	memset((unsigned char *)enbf,0,en_row*en_col);

              	en_func(&Text[index],enbf);
              	for(m = 0; m<en_row; m++)
             		for(n = 0; n<en_col; n++)
		            {
						prnBuf[high*y+2*m][x+n] = enbf[m*en_col+n];
						prnBuf[high*y+2*m+1][x+n] = enbf[m*en_col+n];
		            }
                x+=en_col;
                break;
			case 1:   /*twice col*/
				if (x>48-2*en_col)
				{
					x=0;    /*48-2*/
					y++;
				}
				memset((unsigned char *)enbf,0,en_row*en_col);
				en_func(&Text[index],enbf);
				for (m = 0; m<en_row; m++)
					for (n = 0; n<en_col; n++)
					{
						memset(rst,0,2);
						colExbit(enbf[m*en_col+n],rst);
						prnBuf[high*y+m][x+2*n] = rst[1];
						prnBuf[high*y+m][x+2*n+1] = rst[0];
					}
				x+= 2*en_col;
				break;
			case 0:
			default:
        		if(x>48-en_col)
			    {
					x=0;
					y++;
			    }
				memset((unsigned char *)enbf,0,en_row*en_col);
				en_func(&Text[index],enbf);
				for(m = 0; m<en_row; m++)
					for(n = 0; n<en_col; n++)
						prnBuf[high*y+m][x+n] = enbf[m*en_col+n];
				x+=en_col;
				break;
			}
			index++;
		}
      	else
        {
			index++;
			x+=en_col;
        }
        
		if(x > 48)
      		break;
	}

	if(x != 0)   //half of line -> a line
    {
		switch(trsflags)
        {
      		case 4://both fourth col and row/
				y +=4;
				break;
    		case 3://both twice col and row/
				y +=2;
				break;
     		case 2://twice row/
         		y += 2;
         		break;
			case 1://twice col/
			case 0:
			default:
				y++;
				break;
        }
    }
     
	rtn = write(fd,prnBuf,y*high*48);
   
	while(rtn < 0)
    {
    	//打印机BUF满
    	if(rtn == ENOBUFS)
    	{
    		sleep(3);
		}
		else
		{
			//打印机过热或是缺纸
			while(Os__get_device_signal(DEV_PRINTER_STATE) != 0)
			{
				sleep(1);
			}
		}
		rtn = write(fd,prnBuf,y*high*48);
	}
    
	close(fd);

	return 0;
}


static int SING_BMP_read(unsigned char *ucPath)
{
	int fd,ret,ip=0,iheight=0;
	unsigned char ucBuf[SIGN_MAXBMP];
	FILE* FD = NULL;
	unsigned char shellBuf[50];
	unsigned char *ptr;

	if(ucPath[0] != '/')		//non_root path
	{
		FD = popen("pwd","r");
		if(NULL == FD)
			return -1;
		fread(shellBuf,1,sizeof(shellBuf),FD);
		pclose(FD);
		Uart_Printf("pwd = %s\r\n",shellBuf);
	
		ptr = strstr(ucPath,".");
		if(NULL == ptr)
		{
			shellBuf[strlen(shellBuf)] = '/';
			shellBuf[strlen(shellBuf)+1] = 0;
			strcat(ucPath,shellBuf);
			Uart_Printf("path = %s\r\n",shellBuf);
		}
		else
		{
			
		}
	}

	dataLength=0;
	memset(ucBmpData,0,SIGN_MAXBMP);
	memset(ucBuf,0,SIGN_MAXBMP);
	fd=open(ucPath,O_RDONLY );
	if(fd<0)
		return -1;
	ret=read(fd,ucBuf,sizeof(ucBuf));
	//判断bmp是否超过20k
	if(ret>SIGN_MAXBMP)
		ret=-2;
	//判断是否是bmp文件
	if(ret>0)
		if(ucBuf[0]!=0x42||ucBuf[1]!=0x4d)
			ret=-3;
	//判断是否是单色图
	if(ret>0)
		if(ucBuf[28]!=0x01||ucBuf[29]!=0x00)
			ret=-4;
	//判断宽度是否为384
	if(ret>0)
		if(ucBuf[18]!=0x80||ucBuf[19]!=0x01)
			ret=-5;
	//判度高度是否大于320
	if(ret>0)
    {
		iheight+=ucBuf[22];
		iheight+=ucBuf[23]<<8;
		iheight+=ucBuf[24]<<16;
		iheight+=ucBuf[25]<<24;
		if(iheight>320)
			ret=-6;
    }
	//取偏移量
	if(ret>0)
    {
		ip+=ucBuf[10];
		ip+=ucBuf[11]<<8;
		ip+=ucBuf[11]<<16;
		ip+=ucBuf[12]<<24;
		dataLength=ret-ip;
		memcpy(ucBmpData,&ucBuf[ip],dataLength);
		ret=0;
    }
	close(fd);
	return ret;
}
//组装出打印数据
static void SIGN_BMG_getPrintData(void)
{
	int line=dataLength/48;
	int i=0,j=0,p;

	memset(ucPrintData,0,sizeof(ucPrintData));

	for(i=0; i<line; i++)
		for(j=0; j<48; j++)
        {
			p=(line-i-1)*48;
			ucPrintData[i][j]=~ucBmpData[p+j]&0xff;
        }
}

int Os__sign_xprint(unsigned char *ucPath)
{
	unsigned char ucBuf[49];
	unsigned char ucTmp[SIGN_MAXBMP];
	int ret=-1, i,j;
	FILE *pfd = NULL;
	int len;

	//added by lichao for relative path
	memset(ucBuf,0,sizeof(ucBuf));
	if('/' != ucPath[0])
	{
		pfd = popen("pwd","r");
		if(NULL == pfd)
			return -1;
		fread(ucBuf,1,sizeof(ucBuf),pfd);
		pclose(pfd);
		len = strlen(ucBuf);
		//ucBuf[len] = '/';
		ucBuf[len-1] = 0x00;
		strcat(ucBuf,"/image/");
		strcat(ucBuf,ucPath);
	}
	else
		strncpy(ucBuf,ucPath,sizeof(ucBuf)-1);

	ret = SING_BMP_read(ucBuf);
	if(ret < 0)
	{
		return ret;
	}

	if(ret==0)
		SIGN_BMG_getPrintData();

	int fd = open(DRVNAME, O_RDWR) ;
	if(fd < 0)
	{
		Uart_Printf("%s:%d--can't open file \n",__FUNCTION__,__LINE__);
		ret = -1;
		return ret;
	}

  	memset(ucTmp,0,sizeof(ucTmp));
  	
  	for(i = 0; i < dataLength/48; i++)
		memcpy(&ucTmp[i*48],&ucPrintData[i],48);
 
	//ret = Os__graph_xprint(ucTmp,dataLength);

	ret = write(fd,ucTmp,dataLength);
	
	while(ret < 0)
	{
    	//打印机BUF满
    	if(ret == ENOBUFS)
    	{
    		sleep(3);
		}
		else
		{
			//打印机过热或是缺纸
			while(Os__get_device_signal(DEV_PRINTER_STATE) != 0)
			{
				sleep(1);
			}
		}
		ret = write(fd,ucTmp,dataLength);
	}
       
	ret = 0;

    close(fd);
    return ret;
}

char Os__GB2312_xprint(unsigned char * pdata, unsigned char ucFont)
{
	int retval ;
	unsigned char tmpBuf[1024];
	memset(tmpBuf,0,sizeof(1024));
	utf8_gb2312(pdata,strlen(pdata),tmpBuf,sizeof(tmpBuf));

	switch (ucFont)
    {
		case 0x00:
			retval = ultimate_print(0,0,tmpBuf,strlen(tmpBuf));
			break;
		case 0x01:
			retval = ultimate_print(0,0x01,tmpBuf,strlen(tmpBuf));
			break;
		case 0x1c:
			retval = ultimate_print(0,0x1c,tmpBuf,strlen(tmpBuf));
			break;
		case 0x1d:
			retval = ultimate_print(0,0x1d,tmpBuf,strlen(tmpBuf));
			break;
		case 0x1e:
			retval = ultimate_print(0,0x1e,tmpBuf,strlen(tmpBuf));
			break;
		default:
			retval = ultimate_print(0,0,tmpBuf,strlen(tmpBuf));
			break;
    }

	return retval;
}

char Os__xprint(unsigned char *data)
{
	ultimate_print(0,0,data,strlen(data));
	return strlen(data);
}

//打印走纸
int Os__linefeed(unsigned int line)
{
	int rtn = -1;
	int fd ;
	long timeout;
	unsigned char print_status;

	fd = open(DRVNAME, O_RDWR) ;
	if(fd <= 0 )
	{
		return -1 ;
	}

	rtn =  ioctl(fd,PRINTER_FEED_PAPER,&line) ;
	close(fd);

	return rtn ;
}

int Os__clear_printBuf(void)
{
	int fd, ret = -1, value;

	fd = open(DRVNAME, O_RDWR) ;
	if(fd < 0 )
	{
		return -1 ;
	}
	ret = ioctl(fd,PRINTER_PRINT_CLEANBUF,NULL);
	close(fd);
	return ret;
}

int Os__graph_xprint(unsigned char *pixel_line, int size)
{
	int fd, ret;
	fd = open(DRVNAME, O_RDWR) ;
	if(fd < 0)
	{
		Uart_Printf("open error\r\n");
	    return -1;
	}
	ret = write(fd,pixel_line,size);
	Uart_Printf("fist write return %d\r\n",ret);
	while(ret < 0)
    	{
    		//打印机BUF满
    		if(ret == ENOBUFS)
    		{
    			sleep(3);
		}
		else
		{
			//打印机过热或是缺纸
			while(Os__get_device_signal(DEV_PRINTER_STATE) != 0)
			{
				sleep(1);
			}
		}
		ret = write(fd,pixel_line,size);
		Uart_Printf("next write return %d\r\n",ret);
	}

	close(fd);
	return 0;
}

int Os__checkpaper(void)
{
	int ret;
	ret = Os__get_device_signal(DEV_PRINTER_STATE);
	ret &= 0x01;
	return ret;
}

int Os__check_printBuf(void)
{
	int fd, value = 0;

	fd = open(DRVNAME, O_RDWR);
	if(fd < 0 )
	{
		return -1 ;
	}
	ioctl(fd,PRINTER_PRINT_END,&value);
	close(fd);
	return value;
}

int _print_heattime(int min, int max)
{
	int fd = 0;
	
	
	fd = open(DRVNAME, O_RDWR) ;
	if(fd < 0 )
	{
		return -1;
	}
	ioctl(fd,PRINTER_PRINT_SETMINHEATTIME,&min);
	ioctl(fd,PRINTER_PRINT_SETMAXHEATTIME,&max);
	close(fd);
	return 0;
}

int PRINTER_set_printlevel(int level)
{
	int ret = 0;
	SYS_SETTING setting;
	
	switch(level)
	{
		case PRINTER_LIGHT_LEVEL:
			ret = _print_heattime(535-150,850-150);
			break;
		case PRINTER_NORMAL_LEVEL:
			ret = _print_heattime(535,850);
			break;
		case PRINTER_DARK_LEVEL:
			ret = _print_heattime(535+150,850+150);
			break;
		default:
			ret = -1;
			break;
	}
	if(ret == 0)
	{
		memset(&setting,0,sizeof(SYS_SETTING));
 		strcpy(setting.section,"PRINTER_SETTING");
 		strcpy(setting.key,"PRINT_LEVEL");
 		setting.value[0] = level + '0';
 		PARAM_setting_set(&setting);
	}
	return ret;
}

int PRINTER_get_printlevel(void)
{
	SYS_SETTING setting;
	int b=0;
	
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"PRINTER_SETTING");
 	strcpy(setting.key,"PRINT_LEVEL");
 	PARAM_setting_get(&setting);	
 	b=atoi(setting.value);
 	return b;
}
