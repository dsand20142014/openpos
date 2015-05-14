/****************************
create by liuyifeng 2011/12/26 17:58:38

all communication functions 
include serial gprs moden tcp/ip wifi and so on	
*****************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/socket.h>
#include "comm.h"

//串口设备节点
#define COMM_UART1 "/dev/s3c2410_serial0"
#define COMM_UART2 "/mnt/usb"
#define COMM_UART3 "/mnt/usb"

int UART_STATE;         //1-打开 0-关闭 只有串口不可以重复使用
int USEING_MODE=-1;     //当前使用何种通讯方式

void _ttyMode(int fd,int mode);
int _serialPortIni(char *dev,int speed,int databits,char parity,int stopbits);

unsigned char Os__comm_open(int *iHandle,COMM_PARAM cp)
{
  
  unsigned char ucBuf[50];
  P_UART *detail;
  P_TCPIP *serverInfo;
  
  memset(ucBuf,0,sizeof(ucBuf));
   
  if((USEING_MODE!=cp.commMode) //通讯方式切换
    &&(USEING_MODE!=-1))         //不是第一次
    return COMM_REENTRANT; 
     
  USEING_MODE=cp.commMode;
  switch(cp.commMode)
  {
  //串口
  case 1:    
    detail=cp.commParam;
    if(UART_STATE==1)
      return COMM_INUSE;
    if(detail->port==2)
      sprintf(ucBuf,"%s",COMM_UART2);
    else if(detail->port==3)
      sprintf(ucBuf,"%s",COMM_UART3);  
    else
      sprintf(ucBuf,"%s",COMM_UART1); 
      
    iHandle = _serialPortIni(ucBuf,
                             detail->speed,
                             detail->dataBits,
                             detail->parity,
                             detail->stopBits);                        
    if(iHandle<0)
       return COMM_ERR;
    
    _ttyMode(iHandle,0);
    break;
  case 2:
  case 3:
  case 4:
  case 5:    
    serverInfo=cp.commParam;
    iHandle = socket(AF_INET, SOCK_STREAM, 0);
    if(iHandle<0)
       return COMM_ERR;

    _tcpConn(iHandle,);
    break;
  case 6:
    break;
  default:
    return COMM_ERR;
    break;            
  } 
  USEING_MODE=cp.commMode;
  return COMM_OK;
 
}

unsigned char Os__comm_close(int iHandle)
{
  switch(COMM_MODE)
  {
  case 1:
    _ttyMode(iHandle,1);
    UART_STATE=0;
    break;  
  }
  close(iHandle);
}


unsigned char Os__comm_send(int iHandle,unsigned char *pucInbuf,long lInLen)
{
  int ret=0;
  int i=0;
  switch(COMM_MODE)
  {
  case 1:
    for (i = 0; i < lInLen; i++)
		if (write(iHandle, pucInbuf[i], 1) < 0)
		{ret=COMM_SEND;break;}  
    break;  
  }
  return ret; 
}

int _tcpConn(int fd,unsigned long ulPeerAddr,unsigned short uiPeerPort)
{
  struct sockaddr_in dest;
  struct timeval tv;
  int ret=-1,retryTime=0 ;
  
  memset(&dest, 0, sizeof(dest));
  
  tv.tv_sec = 30;  
  tv.tv_usec = 0;
      
  dest.sin_family = AF_INET;
  dest.sin_port = htons(uiPeerPort);
  dest.sin_addr.s_addr = htonl(ulPeerAddr);
  
  setsockopt(iTCPHandle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

  while(ret!=0)
  {
    ret=connect(iTCPHandle, (struct sockaddr*)&dest, sizeof(dest));
    retryTime++;
    sleep(1);
    if(retryTime>5)
      break;
  }
  
  if(ret==0)
    return COMM_OK;
  else
    return COMM_CONN;  
}


//备份或者还原串口原来的设置
//不打算支持多串口同时操作
struct termios oldattr_tty;
void _ttyMode(int fd,int mode)
{
 
  switch (mode)
  {
    case 0://打开串口时备份设置
      (void)tcgetattr(fd, &oldattr_tty);
      break;
    case 1://关闭串口时还原设置
       tcsetattr(fd, TCSADRAIN, &oldattr_tty); 
       break;
  }
}


int _serialPortIni(char *dev,int speed,int databits,char parity,int stopbits)
{

    int fd,i,ret;
    struct termios	tty;
    int speed_arr[] = {B230400, B115200, B57600, B38400, B19200,B9600, B4800, B2400, B1200, B300,B0};
    int name_arr[] = {230400, 115200, 57600, 38400,  19200,9600, 4800, 2400, 1200,  300, 0};
    int hwf=0,swf=0,block=0;
  
    if (block==1)
        fd=open(dev,O_RDWR|O_NOCTTY);
    else
        fd=open(dev,O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (!fd)
        return -1;

    ret=tcgetattr(fd, &tty);
    if (ret<0)
    {
        close(fd);
        return -1;
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
        tty.c_cflag &= ~PARENB;   //Clear parity enable
        tty.c_iflag &= ~INPCK;    //Enable parity checking
        break;
        //奇校验
    case 'o':
    case 'O':
        tty.c_cflag |= (PARODD | PARENB); //设置为奇效验
        tty.c_iflag |= INPCK;             //Disable parity checking
        break;
        //偶校验
    case 'e':
    case 'E':
        tty.c_cflag |= PARENB;    //Enable parity
        tty.c_cflag &= ~PARODD;   //转换为偶效验
        tty.c_iflag |= INPCK;     //Disable parity checking
        break;
        //等效于“无奇偶校验”
    case 'S':
    case 's':  //as no parity
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;

        break;
    default:
        tty.c_cflag &= ~PARENB;   //Clear parity enable
        tty.c_iflag &= ~INPCK;    //Enable parity checking
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
        return -1;
    }
    tcflush(fd,TCIOFLUSH);
    return fd;
}

void main()
{
 
  int ret=0;
  int ihandle;
  unsigned char ucSendBuf[100];
  COMM_PARAM param;
  
  memset(ucSendBuf,0,sizeof(ucSendBuf));
  memcpy(ucSendBuf,"12345678",8);
  
  P_UART p =
	{
	  1,	   	// com 1
	  115200, // Speed 
	  8,     	// Data bit
	  'N',
	  1     	// Stop bit      
	};

  param.commMode=1;   //设置使用串口通讯
  param.commParam=&p;
 
/*  
  P_TCPIP p=
  {
      "172.16.1.1",
      8080
  };
  
  param.commMode=2;   //设置使用网口通讯
  param.commParam=&p;
*/  
  printf("begin\n");
  Os__comm_open(&ihandle,param);
  printf("open\n");
  ret=Os__comm_send(ihandle,ucSendBuf,8);
  printf("send %d\n",ret);
  Os__comm_close(ihandle);
  printf("rev\n");
}

