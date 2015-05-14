/*****************************************************************
*  UART API
******************************************************************
*/
//#include <oslib.h>
#include <fcntl.h>
#include "sand_com.h"
#include "sand_incs.h"

struct _uart
{
    char *u_dev;
    int   u_fd;
	struct termios *otty;	
};

struct termios	ori_tty[3];

static struct _uart uart_tbl[] =
{
    {"/dev/ttymxc4",-1,&ori_tty[0]},
    {"/dev/s3c2410_serial1",-1,&ori_tty[1]},
    {"/dev/ttyGS0",-1,&ori_tty[2]},
};



static unsigned char map_data_bits[]={5,6,7,8};
static unsigned char map_parity[]={'e','o','n','n'};
static unsigned char map_stop_bits[]={1,2};
static unsigned long map_speed[]=
{
    50,   200, 1050,  7200, 38400,
    110,  135,  300,   600,  1200,
    2400,4800, 9600,    75,   150,
    2000,1800, 19200,38400, 57600,
    115200
};

/*
 * 串口初始化函数。
 * dev：串口设备名, 串口1是"/dev/ttyS0", 串口2是"/dev/ttyS1", 依此类推。
 * USB转串口的设备名是"/dev/ttyUSB0"、"/dev/ttyUSB1"等等。
 * speed：串口波特率, 可以是230400, 115200, 57600, 38400,  19200,  9600, 4800, 2400, 1200,  300, 0。
 * databits：数据位, 值是5、6、7或者8。
 * parity：奇偶校验。值为'N','E','O','S'。
 * stopbits：停止位, 值是1或者2。
 * hwf：硬件流控制。1为打开, 0为关闭。
 * swf：软件流控制。1为打开, 0为关闭。
 * noblock 是否堵塞方式 1 堵塞 0 不堵塞
 * example: fd=SerialPortInit("/dev/ttyS0",9600,8,'N',1,0,0,0);
 */
int SerialPortInit(char *dev,int speed,int databits,int parity,int stopbits,int hwf,int swf, int block,struct termios *otty)
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
        return -1;
    }
   
    ret=tcgetattr(fd, &tty);

	memcpy(otty,&tty,sizeof(tty));/* reserve the original tty */
	
    if (ret<0)
    {
        Uart_Printf("tcgetattr error\n");
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
        return -1;
    }
    tcflush(fd,TCIOFLUSH);
    return fd;
}

int init_com_param(COM_PARAM *cparam,unsigned short uiParam1,unsigned short uiParam2,unsigned char ucParam3)
{
    int retval = 0;

    unsigned char R7,R6,R5,R4,R3;//from sandhelp

    R7 = uiParam1&0x00ff;       // data bits
    R6 = (uiParam1>>8)&0x00ff;  // parity bits
    R5 = uiParam2&0x00ff;       //stop bits
    R3 = ucParam3;


    cparam->data_bits = (R7<4)? map_data_bits[R7]:8;
    cparam->parity    = (R6<4)? map_parity[R6]:'n';
    cparam->stop_bits = (R5<2)? map_stop_bits[R5]:0;
    cparam->speed     = (R3<20)?map_speed[R3]:115200;



	return retval;
}
#if 1 
unsigned short OSUART_read_char(int fd,unsigned short timeout)
{
    fd_set rfds;
    struct timeval tv;
    unsigned char ch;
    unsigned short err_status=0;
	int retval;

    if (fd < 0)
    {
        fprintf(stderr,"Have not opened !!!\n",__func__);
        return -1;
    }
    tv.tv_sec = 0;
    tv.tv_usec = timeout*1000;//timeout = n*10ms

    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);

    retval = select(fd+1,&rfds,NULL,NULL,&tv);
    if (retval==-1)
    {
        perror("select()");
        err_status = 0x3<<8;
    }
    else if (retval)
    {
        if (read(fd,&ch,1)<0)
			err_status = 0x2<<8;	
		else
			err_status = ch;
    }
    else // timeout
    {
        err_status = 0x1<<8;
    }

    return err_status;

}
#else

unsigned short OSUART_read_char(int fd,unsigned short timeout)
{
    int ret=0, first_time_flag=1;
    unsigned int to=timeout, retVal;
    unsigned char hByte=0xff, lByte=0xff;
    unsigned char recv=0x0;

    Os__timer_start((unsigned int*)&to);
    do
    {
        ret=read(fd, &lByte, 1);
    }
    while (ret<=0 && to>0 );
    Os__timer_stop((unsigned int*)&to);

    if (to==0)
    {
        return COM_ERR_TO*256;
    }
    else
    {
        hByte=OK;
        retVal= (hByte <<8) | lByte;
    }
    return retVal;
}
#endif

/* 1st RS232 */
unsigned char OSUART_Init1(COM_PARAM *pa)
{
    int retval = 0;
    struct _uart *puart = &uart_tbl[0] ;

    puart->u_fd = SerialPortInit(puart->u_dev,
                                 pa->speed,
                                 pa->data_bits,
                                 pa->parity,
                                 pa->stop_bits,
                                 0,
                                 0,
                                 1,//block
                                 puart->otty);

    if (puart->u_fd < 0)
    {
        perror("Can't Open Serial Port");
        return -1;
    }
    return retval;
}
void OSUART_TxChar1(unsigned char ucCh)
{
    int retval;
    struct _uart *puart = &uart_tbl[0] ;

    if (puart->u_fd < 0)
    {
        fprintf(stderr,"Have not opened !!!\n",__func__);
        return ;
    }
    if ((retval=write(puart->u_fd, &ucCh,1)) < 0)
    {
        fprintf(stderr,"%s\n",__func__);
        return ;
    }
    return ;
}

unsigned short OSUART_RxChar1(unsigned short uiTimeout)
{
    struct _uart *puart = &uart_tbl[0];
	
	return OSUART_read_char(puart->u_fd,uiTimeout);
}



void OSUART_Close1(void)
{
    struct _uart *puart = &uart_tbl[0];

	if (puart->u_fd>=0){
		tcsetattr(puart->u_fd, TCSANOW, puart->otty);
	}
    close(puart->u_fd);
    puart->u_fd = -1;
}
void OSUART_Flush1(unsigned short uiRecvNB)
{
}

/* 2nd RS232 */
unsigned char OSUART_Init2(unsigned short uiParam1,unsigned short uiParam2,unsigned char ucParam3)
{
    int retval = 0;
    struct _uart *puart ;
    COM_PARAM cparam;
	
    init_com_param(&cparam,uiParam1,uiParam2,ucParam3);

    puart = &uart_tbl[1] ;
    puart->u_fd = SerialPortInit(puart->u_dev,
                                 cparam.speed,
                                 cparam.data_bits,
                                 cparam.parity,
                                 cparam.stop_bits,
                                 0,
                                 0,
                                 1,//block
                                 puart->otty);

    if (puart->u_fd < 0)
    {
        fprintf(stderr,"%s\n",__func__);
        return -1;
    }
    return retval;

}
void OSUART_TxChar2(unsigned char ucCh)
{
    int retval;
    struct _uart *puart = &uart_tbl[1] ;

    if (puart->u_fd < 0)
    {
        fprintf(stderr,"Have not opened !!!\n",__func__);
        return ;
    }
    if ((retval=write(puart->u_fd, &ucCh,1)) < 0)
    {
        fprintf(stderr,"%s\n",__func__);
        return ;
    }
    return ;
}
unsigned short OSUART_RxChar2(unsigned short uiTimeout)
{
    struct _uart *puart = &uart_tbl[1];
	
	return OSUART_read_char(puart->u_fd,uiTimeout);
}
void OSUART_Flush2(unsigned short uiRecvNB)
{
}

void OSUART_Close2(void)
{
    struct _uart *puart = &uart_tbl[1];

	if (puart->u_fd>=0){
		tcsetattr(puart->u_fd, TCSANOW, puart->otty);
	}
    close(puart->u_fd);
    puart->u_fd = -1;
}

/* 3nd RS232 */
unsigned char OSUART_Init3(COM_PARAM *pa)
{
    int retval = 0;
    struct _uart *puart = &uart_tbl[2] ;

    puart->u_fd = SerialPortInit(puart->u_dev,
                                 pa->speed,
                                 pa->data_bits,
                                 pa->parity,
                                 pa->stop_bits,
                                 0,
                                 0,
                                 1,//block
                                 puart->otty);

    if (puart->u_fd < 0)
    {
        perror("Can't Open Serial Port");
        return -1;
    }
    return retval;
}

unsigned char OSUART_Init3_Rx(unsigned short uiParam1,unsigned short uiParam2,unsigned char ucParam3)
{
    int retval = 0;
    struct _uart *puart ;
    COM_PARAM cparam;

	init_com_param(&cparam,uiParam1,uiParam2,ucParam3);

    puart = &uart_tbl[2] ;
    puart->u_fd = SerialPortInit(puart->u_dev,
                                 cparam.speed,
                                 cparam.data_bits,
                                 cparam.parity,
                                 cparam.stop_bits,
                                 0,
                                 0,
                              //   1,//block
                                 0,//2013 0302 edit by liuyifeng otg口有问题
                                 puart->otty);
                                 
    if (puart->u_fd < 0)
    {
        fprintf(stderr,"%s\n",__func__);
        return -1;
    }
	_g_com2_openned = 1;
    return retval;
}

void OSUART_TxChar3(unsigned char ucCh)
{
    int retval;
    struct _uart *puart = &uart_tbl[2] ;

    if (puart->u_fd < 0)
    {
        fprintf(stderr,"Have not opened !!!\n",__func__);
        return ;
    }
    if ((retval=write(puart->u_fd, &ucCh,1)) < 0)
    {
        fprintf(stderr,"%s\n",__func__);
        return ;
    }
    return ;
}
unsigned short OSUART_RxChar3(unsigned short uiTimeout)
{
    struct _uart *puart = &uart_tbl[2];
	
	return OSUART_read_char(puart->u_fd,uiTimeout);

}


void OSUART_Close3(void)
{
    struct _uart *puart = &uart_tbl[2];

	if (puart->u_fd>=0){
		tcsetattr(puart->u_fd, TCSANOW, puart->otty);
	}
    close(puart->u_fd);
    puart->u_fd = -1;
	_g_com2_openned = 0;
}

unsigned char Os__com_flush1(void)
{
    Uart_Printf("%s:Do Nothing \n",__func__);
    return 0;
}
unsigned char Os__com_flush2(void)
{
    Uart_Printf("%s:Do Nothing \n",__func__);
    return 0;
}
unsigned char Os__com_flush3(void)
{
    Uart_Printf("%s:Do Nothing \n",__func__);
    return 0;
}

unsigned char Os__init_com3(unsigned short uiParam1,unsigned short uiParam2,unsigned char ucParam3)
{
    return OSUART_Init3_Rx(uiParam1,uiParam2,ucParam3);
}

void Os__txcar3(unsigned char ucCh)
{
    return OSUART_TxChar3(ucCh);
}

unsigned short Os__rxcar3(unsigned short uiTimeout)
{
    return OSUART_RxChar3(uiTimeout);
  
}
void Os__end_com3(void)
{
    return OSUART_Close3();
}
