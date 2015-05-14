#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <errno.h>
#include<sys/ioctl.h>

#define PRINTER "/dev/printer"

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
#define PRINTER_PRINT_SETMINHEATTIME    _IOWR(PRINTER_IOC_MAGIC,11, int)//最小加热时间，不能低于400，打印>机走纸不动，默认535；
#define PRINTER_PRINT_SETMAXHEATTIME    _IOWR(PRINTER_IOC_MAGIC,12, int)//最大加热时间，不高于1200，默认850，过高极易过热；
#define PRINTER_PRINT_SETLINEFEEDTIME   _IOWR(PRINTER_IOC_MAGIC,13, int)//走纸时间，默认400。很少需要改动>，影响走纸速度；
#define PRINTER_PRINT_SETADDHEATTIME    _IOWR(PRINTER_IOC_MAGIC,14, int)//加热补偿时间，默认800，在空白行>之后自动加在当前加热时间上，一般不用改动；
#define PRINTER_PRINT_SETOVERHEATTEMP   _IOWR(PRINTER_IOC_MAGIC,15, int)//过热温度，默认1500，越大过热温度越低，其值与温度近似线性关系；
#define PRINTER_PRINT_SETRETNORMALTEMP  _IOWR(PRINTER_IOC_MAGIC,16, int)//回复非过热状态温度，默认2000，必须大于过热温度；
#define PRINTER_IOC_MAXNR               16


static int print_square(int fd)
{
		int i,j;
		int ret;
		char a[48*8];
		for(i=0;i<40;i++)
		{
				if(i%2==0)
				{
						for(j=0;j<sizeof(a);j++)
						{
								if(j%48%2==0)
										a[j]=0xFF;
								else
										a[j]=0x00;
						}
				}
				else
				{
						for(j=0;j<sizeof(a);j++)
						{
								if(j%48%2==0)
										a[j]=0x00;
								else
										a[j]=0xFF;
						}
				}
				ret=write(fd,a,sizeof(a));
				if(ret!=sizeof(a))
				{
						printf("%d\n",ret);
				}
		}
		return ret;

}

static int print_num(int fd)
{
		int i;
		int ret;
		unsigned char a[48*6]={0};
		for(i=0;i<256;i++)
		{
				a[i]=i;
		}
		ret=write(fd,a,sizeof(a));
		if(ret!=sizeof(a))
		{
				printf("%d\n",ret);
		}
		return ret;	
}

static int print_black(int fd)
{
		int i;
		int ret;
		unsigned char a[48]={0};
		for(i=0;i<48;i++)
		{
				a[i]=0xFF;
		}
		for(i=0;i<8*100;i++)
		{
				ret=write(fd,a,sizeof(a));
		}
		return 0;

}

static void print_help(void)
{
	printf("print_test [-b][-s][-c]");
}

static void print_checkpaper(int fd)
{
	int ret=ioctl(fd, PRINTER_PRINT_CHECKPAPER);
	if(ret)
	{
		printf("%d not have paper.\n",ret);
	}
	else
	{
		printf("%d have paper.\n",ret);
	}
}

int main(int argc,char** argv)
{
		int fd;
		fd=open(PRINTER,O_RDWR);
		if(fd<0)
		{
				perror("open printer");
				return -1;
		}
		if(argc==1)
		{
			print_black(fd);
			print_square(fd);
			//print_num(fd);
		}
		else if(argc==2)
		{
			if(strcmp(argv[1],"-c")==0)
			{
				print_checkpaper(fd);
			}
			else if(strcmp(argv[1],"-b")==0)
			{
				print_black(fd);
			}
			else if(strcmp(argv[1],"-s")==0)
			{
				print_square(fd);
			}
			else
			{
				print_help();
			}
		}
		else
		{
			print_help();			
		}
		close(fd);
		return 0;
}
