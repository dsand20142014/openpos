#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>    
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>   
#include <termios.h> 
#include <errno.h> 
#include <string.h>
#include <sys/ioctl.h>
#include "uart.h"


static int _SerialPortInit(char *dev,int speed,int databits,int parity,int stopbits,int hwf,int swf, int block,struct termios *otty)
{
    int fd,i,ret;
    struct termios	tty;
    int speed_arr[] = {B230400, B115200, B57600, B38400, B19200,
                       B9600, B4800, B2400, B1200, B300,B0
                      };
    int name_arr[] = {230400, 115200, 57600, 38400,  19200,
                      9600, 4800, 2400, 1200,  300, 0
                     };

    if ( block==1 )
        fd=open(dev,O_RDWR|O_NOCTTY);
    else
        fd=open(dev,O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (!fd)
    {
        Uart_Printf("cannot open %s\n",dev);
        return UART_NOK;
    }
   
    ret=tcgetattr(fd, &tty);

	memcpy(otty,&tty,sizeof(tty));/* save the original tty */
	
    if (ret<0)
    {
        Uart_Printf("tcgetattr error\n");
        close(fd);
        return UART_NOK;
    }

    //设置波特率
    for (i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&tty, speed_arr[i]);
            cfsetospeed(&tty, speed_arr[i]);
            break;
        }
        if (name_arr[i] == 0)
        {
            Uart_Printf("speed %d is not support,set to 9600\n",speed);
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
        }
    }
    //设置数据位
    switch (databits)
    {
    case 5:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
        break;
    case 6:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
        break;
    case 7:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
        break;
    case 8:
    default:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        break;
    }

    //设置停止位
    if (stopbits == 2)
        tty.c_cflag |= CSTOPB;
    else
        tty.c_cflag &= ~CSTOPB;
    //设置奇偶校验
    switch (parity)
    {
        //无奇偶校验
    case 'n':
    case 'N':
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
        //奇校验
    case 'o':
    case 'O':
        tty.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        tty.c_iflag |= INPCK;             /* Disable parity checking */
        break;
        //偶校验
    case 'e':
    case 'E':
        tty.c_cflag |= PARENB;    /* Enable parity */
        tty.c_cflag &= ~PARODD;   /* 转换为偶效验*/
        tty.c_iflag |= INPCK;     /* Disable parity checking */
        break;
        //等效于“无奇偶校验”
    case 'S':
    case 's':  /*as no parity*/
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;

        break;
    default:
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;

    }
    //设置硬件流控制
    if (hwf)
        tty.c_cflag |= CRTSCTS;
    else
        tty.c_cflag &= ~CRTSCTS;

    //设置软件流控制
    if (swf)
        tty.c_iflag |= IXON | IXOFF;
    else
        tty.c_iflag &= ~(IXON|IXOFF|IXANY);

    //设置为RAW模式
    tty.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
                     IXANY | IXON | IXOFF | INPCK | ISTRIP);
    tty.c_iflag |= (BRKINT | IGNPAR);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(XCASE|ECHONL|NOFLSH);
    tty.c_lflag &= ~(ICANON | ISIG | ECHO);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cc[VTIME] = 5;
    tty.c_cc[VMIN] = 1;

    //设置串口属性
    ret = tcsetattr(fd, TCSANOW, &tty);
    if (ret < 0)
    {
        close(fd);
        return UART_NOK;
    }
    tcflush(fd,TCIOFLUSH);
    return fd;
}

int UART_Init(char *dev, COM_PARAM *pa)
{
    int retval = 0;
    int fd = 0;

    fd = SerialPortInit(dev,
                        pa->speed,
                        pa->data_bits,
                        pa->parity,
                        pa->stop_bits,
                        0,
                        0,
                        1,//block
                        NULL);//don't save original tty

    if (fd < 0)
    {
        perror("Can't Open Serial Port");
        return UART_NOK;
    }
    return fd;
}

void UartClose(int fd)
{
    close(fd);
}


int UartSend(int fd, unsigned char* inBuf, long inLen)
{
    int retval = UART_OK;
    int i = 0;

    for(i=0; i<inlen; i++)
    {
    	if ((retval=write(fd, inBuf[i],1)) < 0)
    	{
       		fprintf(stderr,"%s\n",__func__);
       		retval = UART_SENDERROR;
       		break;
    	}
    }
    return retval;
}


int UartRecv(int fd,unsigned short timeout, unsigned char *recvBuf, size_t recvLen)
{
		fd_set rfds;
		struct timeval tv;
		unsigned char ch;
		int err_status=0;
		int retval = 0;
		int i = 0;
		int nread = 0;
		int leftLen = recvLen;

    tv.tv_sec = 0;
    tv.tv_usec = timeout*1000;//timeout = n*10ms

    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);

    retval = select(fd+1,&rfds,NULL,NULL,&tv);
    if (retval==-1)
    {
        perror("select()");
        err_status = UART_SELECTERROR;
    }
    else if (retval)
    {
    		while(i<recvLen)
    		{
    				if (nread = read(fd,&recvBuf[i],leftLen)<0)
    				{
								err_status = UART_RECVERROR;
								break;
    				}	
    				else if(nread == 0)		// EOF
    				{	
    						break;		
    				}
    				else
    				{
    						i += nread;
    						leftLen -= nread;		
    				}
    		}
    }
    else // timeout
    {
        err_status = UART_TIMEOUT;
    }

    return err_status;
}

void main(void)
{
		int fd;
		int ret;
		char sendbuf[100];
		
		strcpy(sendbuf,"hello sand");
		
		COM_PARAM cp =
 		{
  			1,
     	 	115200,
      	2,
     	 	8,
     	 	'N'
 	 	};
	
		fd = UART_Init("/dev/ttyGS0", &cp);
		if(fd < 0)
				return;
		
		ret = UartSend(fd,sendbuf,strlen(sendbuf));
		printf("send result:%d\r\n",ret);
		return ;
}


