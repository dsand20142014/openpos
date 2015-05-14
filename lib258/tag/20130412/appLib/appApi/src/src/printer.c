#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ostools.h"
#include "devSignal.h"
#include "printer.h"

#define SIGN_MAXBMP 40560
#define DRVNAME  "/dev/printer"


/** 定义IOCTL命令 */
#define PRINTER_IOC_MAGIC       'S'
#define PRINTER_FEED_PAPER      _IOW(PRINTER_IOC_MAGIC,  1, int)
#define PRINTER_PRINT_VCC       _IOW(PRINTER_IOC_MAGIC,  2, int)
#define PRINTER_PRINT_FONT      _IOW(PRINTER_IOC_MAGIC,  3, int)
#define PRINTER_PRINT_TEST      _IOW(PRINTER_IOC_MAGIC,  4, int)
#define PRINTER_PRINT_CHECKPAPER    _IOWR(PRINTER_IOC_MAGIC, 5, int)
#define PRINTER_PRINT_CFGPARAM  _IOWR(PRINTER_IOC_MAGIC, 6, int)
#define PRINTER_PRINT_CLEANBUF  _IOWR(PRINTER_IOC_MAGIC, 7, int)
#define PRINTER_PRINT_CHKSTATUS _IOWR(PRINTER_IOC_MAGIC, 8, int)
#define PRINTER_PRINT_SETVF     _IOWR(PRINTER_IOC_MAGIC, 9, int)
#define PRINTER_PRINT_END       _IOWR(PRINTER_IOC_MAGIC,10, int)
#define PRINTER_PRINT_SETMINHEATTIME _IOWR(PRINTER_IOC_MAGIC,11, int)
#define PRINTER_PRINT_SETMAXHEATTIME _IOWR(PRINTER_IOC_MAGIC,12, int)
#define PRINTER_IOC_MAXNR       12


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

static int ultimate_print(unsigned char ucASCIIFont, unsigned char ucGBFont, unsigned char *Text,int dlen)
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

	ret = SING_BMP_read(ucPath);
	if(ret < 0)
	{
		return ret;
    }

	if(ret==0)
		SIGN_BMG_getPrintData();

	int fd = open(DRVNAME, O_RDWR) ;
	if(fd < 0)
	{
		printf("%s:%d--can't open file \n",__FUNCTION__,__LINE__);
		ret = -1;
		return ret;
	}

  	memset(ucTmp,0,sizeof(ucTmp));
  	
  	for(i = 0; i < dataLength/48; i++)
		memcpy(&ucTmp[i*48],&ucPrintData[i],48);
 
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
	    return -1;
	}
	ret = write(fd,pixel_line,size);
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
