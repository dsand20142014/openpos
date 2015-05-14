/****************************
create by liuyifeng 2011/12/26 17:58:38

all communication functions 
include serial gprs moden tcp/ip wifi and so on	
*****************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if.h>
#include <netdb.h>
#include "comm.h"
//#include "sand_network.h"

//串口设备节点
#define COMM_UART1 "/dev/ttyGS0"
#define COMM_UART2 "/mnt/usb"
#define COMM_UART3 "/mnt/usb"
#define MAX_RCV_LEN 1500
#define bbzero(a, b)             memset(a, 0, b)
#define ONLINE_MASK 0x01
#define RX_MASK 0x02
#define TX_MASK 0x04


int UART_STATE;         	//1-打开 0-关闭 只有串口不可以重复使用
int USEING_MODE=-1;     	//当前使用何种通讯方式
//struct termios oldattr_tty;
P_UART 	g_uart_param;
P_TCPIP	g_tcpip_param;
extern int errno;

int _ttyMode(int fd,int mode);
int _serialPortIni(char *dev,int speed,int databits,char parity,int stopbits);
int _tcpConn(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);
unsigned long get_ipaddr();
int TCP_send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
int TCP_recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
void Os__keyled_disp(unsigned char status, unsigned char led){}
char * istrstr(const char *str, const char * substr);

int Os__comm_init(COMM_PARAM *cp)
{
	int ret; 
	P_UART *detail;
	P_TCPIP *serverInfo;
   
//	if((USEING_MODE!=cp->commMode) //通讯方式切换
//			&&(USEING_MODE!=-1))         //不是第一次
//		return COMM_REENTRANT; 
     
	USEING_MODE=cp->commMode;
	switch(cp->commMode)
	{
		case MODE_UART:		//UART
		case MODE_MODEM:	//MODEM
			detail=cp->commParam;
			memcpy(&g_uart_param,detail,sizeof(P_UART));
			ret = COMM_OK;
			break;
		case MODE_TCPIP:	//TCP/IP
		case MODE_GSM:		//GSM
		case MODE_WIFI:		//WIFI
		case MODE_CDMA:    	//CDMA
			serverInfo=cp->commParam;
			memcpy(&g_tcpip_param,serverInfo,sizeof(P_TCPIP));
			ret = COMM_OK;
			break;
			//ihandle = socket(AF_INET, SOCK_STREAM, 0);
			//g_ip_addr = serverInfo->
			//if(ihandle<0)
			//return COMM_ERR;
		default:
			return COMM_ERR;
	} 
	return ret;
}

int Os_comm_connect()
{
	int i;
	int ihandle = COMM_ERR;
	unsigned char ucBuf[50];
	unsigned long addr;
	struct sockaddr_in sin;
	int ret = COMM_CONN;
	struct hostent *hptr = NULL;
	char **pptr = NULL;
	char *ptr = NULL;
	char ip[32];
	int count = 1;
	memset(ucBuf,0,sizeof(ucBuf));
	switch(USEING_MODE)
	{
		case MODE_UART:		//UART
			if(UART_STATE==1)
      				return COMM_INUSE;
    			if(g_uart_param.port==2)
      				sprintf(ucBuf,"%s",COMM_UART2);
    			else if(g_uart_param.port==3)
      				sprintf(ucBuf,"%s",COMM_UART3);  
    			else
      				sprintf(ucBuf,"%s",COMM_UART1); 
      
			ihandle = _serialPortIni(ucBuf,
                             g_uart_param.speed,
                             g_uart_param.dataBits,
                             g_uart_param.parity,
                             g_uart_param.stopBits);                        
			if(ihandle<0)
				return COMM_ERR;
			//_ttyMode(ihandle,0);	//back up tty.
			break;
		case MODE_TCPIP:	//TCP/IP
		case MODE_GSM:		//GSM
		case MODE_WIFI:		//WIFI
		case MODE_CDMA:		//CDMA
			ihandle = socket(AF_INET, SOCK_STREAM, 0);
			Uart_Printf("socket ihandle = %d\r\n",ihandle);
			if(ihandle<0)
				return COMM_ERR;
			Uart_Printf("g_tcpip_param.linkdescribe = %s\r\n",g_tcpip_param.linkdescribe);
			if(istrstr(g_tcpip_param.linkdescribe,"http://") == NULL)	//ip
			{
				ret = inet_addr(g_tcpip_param.linkdescribe);
				Uart_Printf("addr = %d\r\n",ret);
				
				if(ret == INADDR_NONE)
				{
					Uart_Printf("wrong ip address\r\n");
					return COMM_CONN;
				}
				else
				{
					addr = (unsigned long)ret;
				}
				//Uart_Printf("addr = %04x\r\n",addr);
				//Uart_Printf("port = %d\r\n",g_tcpip_param.port);

				sin.sin_family = AF_INET;
				sin.sin_addr.s_addr = get_ipaddr();
				sin.sin_port = htons(g_tcpip_param.port);
				ret = bind(ihandle,(struct sockaddr*)&sin,sizeof(sin));
				//Uart_Printf("tcpconnect bind return = %d\r\n",ret);

				ret = _tcpConn(ihandle,addr, htons(g_tcpip_param.port));
				//Uart_Printf("tcpconnect _tcpConn return = %d\r\n",ret);
				if(ret < 0)
					return ret;

			}
			else							//url
			{
				ptr = istrstr(g_tcpip_param.linkdescribe,"http://");
				ptr += 7;
				//strcpy(ip,ptr);
				
				Uart_Printf("url = %s\r\n",ip);
				hptr = gethostbyname(ptr);
				if(hptr == NULL)
				{
					Uart_Printf("can't access the url\r\n");
					return 0;
				}	
				pptr = hptr->h_addr_list;

				for(;*pptr!=NULL;pptr++)
				{
					if(count > 3)
						return COMM_CONN;
					inet_ntop(hptr->h_addrtype,*pptr,ip,sizeof(ip));
					//Uart_Printf("ip = %s\r\n",ip);
					addr = inet_addr(ip);
					//Uart_Printf("addr = %04x\r\n",addr);
					//Uart_Printf("port = %d\r\n",g_tcpip_param.port);
					count ++;

					ret = _tcpConn(ihandle,addr, htons(g_tcpip_param.port));
					//Uart_Printf("tcpconnect _tcpConn return = %d\r\n",ret);
					if(ret == COMM_OK)
						break;
					if(ret < 0)
						return ret;
				}
			}
		case MODE_MODEM:		//MODEM
			break;
		default:
			break;	
	}

	return ihandle;
}

int Os__comm_close(int iHandle)
{
	int ret = -1;
	switch(USEING_MODE)
	{
	case MODE_UART:
		//ret = _ttyMode(iHandle,1);	//restore old tty.
		USEING_MODE = -1;
		UART_STATE=0;
		ret = close(iHandle);
		break;  
	case MODE_GSM:
	case MODE_WIFI:
	case MODE_TCPIP:
	case MODE_CDMA:
		//ret = TCP_CLose(iHandle);
		USEING_MODE = -1;
		ret = close(iHandle);
		break;
	default:
		break;
	}
	return ret;
}

int Os__comm_recv(int iHandle,unsigned char *pucInbuf,unsigned short *pucInLen,int iTimeout)
{
	int ret=0;
	switch(USEING_MODE)
	{
		case MODE_UART:
			ret = read(iHandle, pucInbuf, (int)*pucInLen);
			if(ret < 0)
				return COMM_RECV;
    		break;  
		case MODE_GSM:
		case MODE_WIFI:
		case MODE_TCPIP:
		case MODE_CDMA:
			ret = TCP_recv(iHandle, pucInbuf,pucInLen, iTimeout);
			//Uart_Printf("TCP_recv return %d\r\n",ret);
			if(ret < 0)
				return COMM_RECV;
			break;
		case MODE_MODEM:
			break;
		default:
			return COMM_RECV;
	}
	return ret; 

}

int Os__comm_send(int iHandle,unsigned char *pucInbuf,long lInLen)
{
	int ret=0;
	int i=0;
	switch(USEING_MODE)
	{
		case MODE_UART:
			ret = write(iHandle, pucInbuf, lInLen);
			if(ret < 0)
				return COMM_SEND;
    		break;  
		case MODE_GSM:
		case MODE_WIFI:
		case MODE_TCPIP:
		case MODE_CDMA:
			ret = TCP_send(iHandle, pucInbuf, lInLen);
			if(ret < 0)
				return COMM_SEND;
			break;
		case MODE_MODEM:
			break;
		default:
			return COMM_SEND;
	}
	return ret; 
}

int _tcpConn(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort)
{
	struct sockaddr_in dest;
	struct timeval tv;
	int ret=-1,retryTime=0 ;
  
	memset(&dest, 0, sizeof(dest));
  
	tv.tv_sec = 30;  
	tv.tv_usec = 0;
      
	dest.sin_family = AF_INET;
	//dest.sin_port = htons(uiPeerPort);
	dest.sin_port = uiPeerPort;
	//dest.sin_addr.s_addr = htonl(ulPeerAddr);
	dest.sin_addr.s_addr = ulPeerAddr;
  
	Uart_Printf("in _tcpConn,dest.sin_port = %d\r\n",dest.sin_port);
	Uart_Printf("in _tcpConn,dest.sin_addr.s_addr= %04x\r\n",dest.sin_addr.s_addr);

	setsockopt(iTCPHandle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

	while(ret!=0)
	{
		ret=connect(iTCPHandle, (struct sockaddr*)&dest, sizeof(dest));
		Uart_Printf("in _tcpConn,connect result = %d\r\n",ret);
		if(ret == -1)
		{
			Uart_Printf("errno = %02x\r\n",errno);
			Uart_Printf("connection error:->  %s\r\n",strerror(errno));
		}
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
/*
int _ttyMode(int fd,int mode)
{
	int ret = 0;
	switch (mode)
	{
		case 0://打开串口时备份设置
			ret = tcgetattr(fd, &oldattr_tty);
			break;
		case 1://关闭串口时还原设置
			ret = tcsetattr(fd, TCSANOW, &oldattr_tty); 
			break;
	}
	return ret;
}
*/

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
	//_ttyMode(fd,0);	//back up tty.
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

//failed return 0 succeed return ip address
unsigned long get_ipaddr()
{
	struct ifreq ifr;
	int fd = 0;
	struct sockaddr_in *pAddr;
	unsigned long result = 0;
	char* dev = NULL;
 
	switch(USEING_MODE)
	{
		case MODE_TCPIP:	//TCP/IP
			break;
		case MODE_GSM:		//GSM
			dev = "ppp0";
			break;
		case MODE_WIFI:		//WIFI
			dev = "wlan0";
			break;
		case MODE_CDMA:		//CDMA
			break;
		default:
			return 0;
	}
	if(dev == NULL)
		return 0;
 
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		Uart_Printf("open socket failed \n");
		return 0;
	}
 
	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_name, dev);
 
	if(ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		Uart_Printf("SIOCGIFADDR socket failed \n");
		close(fd);
		return 0;
	}
 
	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
 
	result = pAddr->sin_addr.s_addr;
     
	close(fd);
     
	return result;
}

//if success return 0,else return -1.
int TCP_send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen)
{
    int ret;
    struct timeval tv;    
    int err_again_cnt = 0;

    tv.tv_sec = 20;// modified by liugm, 2010-11-12 
    tv.tv_usec = 0;    

    setsockopt(iTCPHandle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

    /* modified by liugm, 2010-11-12 */

    Os__keyled_disp(0, RX_MASK);

again:   
    Os__keyled_disp(1, TX_MASK);

    ret = send(iTCPHandle, pucInBuf, uiInLen, 0);

    Os__keyled_disp(0, TX_MASK);

    	
    if (ret == -1)
    {
        perror("TCP_send");
        Uart_Printf("errno = %d\n",errno);

        if (errno == EAGAIN || errno == EWOULDBLOCK || errno==EINTR) {
            if (err_again_cnt++ > 5){
                //set_ppp_restart();
                return -1;
            }
            goto again;
        }

        return(errno);
    }

    return(0);
}


//if success return 0,else return -1.
int TCP_recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout)
{
	int ret;
	struct timeval tv;
	int err_again_cnt = 0;  // liugm 2010-11-12

	tv.tv_sec = uiTimeout;
	tv.tv_usec = 0;

	/* modified by liugm, 2010-10-12  */
	if (setsockopt(iTCPHandle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		return -1;

	Os__keyled_disp(0, TX_MASK);

again:
	Os__keyled_disp(1, RX_MASK);

	ret = recv(iTCPHandle, pucInBuf, MAX_RCV_LEN, 0);

	Os__keyled_disp(0, RX_MASK);
	
	if(ret==-1)
	{
		*puiInLen=0;
		return -1;
	}
	else  
		*puiInLen = ret;

	return 0;
}

//compare string ignore case
char * istrstr(const char *str, const char * substr)
{
	int len;
	char *ptr;
	//Uart_Printf("str = %s\r\n",str);
	//Uart_Printf("substr = %s\r\n",substr);
	if((str == NULL) || (substr == NULL))
	{
		return NULL;
	}
	len = strlen(substr);
	if(0 == len)
	{
		return NULL;
	}

	ptr = str;
	while(*ptr)
	{
		if(strncasecmp(ptr,substr,len) == 0)
			return ptr;	
		ptr ++;
	}
	return NULL;
}

int Os__comm_check()
{

}

#if 0
int main()
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
#endif


