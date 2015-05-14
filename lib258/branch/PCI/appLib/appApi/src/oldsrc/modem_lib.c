/*
 * File: PS4000 linux version static lib for old app
 * Date: 2009.3.30
 * Author: Bestzwj Sand.inc <bestzwj@163.com>
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
#include <signal.h>

#include "osmodem.h"
#include "sand_incs.h"
//Uart


#ifdef EN_DBG
#define dbg Uart_Printf
#else
#define dbg
#endif


static unsigned char modem_or_uart = 0;   //0 Uart 1 Modem

int fd_rs232;
int nread;
char *modem_rs232 ="/dev/ttyS1";

int Uart_SerialPortInit(char *dev,int speed,int databits,int parity,int stopbits,int hwf,int swf, int block)
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

unsigned char Uart__init_com (COM_PARAM *ComParameter)
{
    unsigned int stop_bit, speed, com_nr, data_bit, parity;
    int ret;

    modem_or_uart = 0;

    stop_bit=ComParameter->stop_bits;
    speed=ComParameter->speed;
    com_nr=ComParameter->com_nr;
    data_bit=ComParameter->data_bits;
    parity=ComParameter->parity;

re_open:
    Uart_Printf("speed %d data_bit %d parity = %c stop_bit %d\n",  speed, data_bit, parity, stop_bit);
    fd_rs232=Uart_SerialPortInit(modem_rs232, speed, data_bit, parity, stop_bit, 0, 0, 0);
    if (fd_rs232<0)
    {
        perror("Can't Open Serial Port");
        goto re_open;
    }
//	Uart_Printf("fd_switchcom=%d  fd_rs232=%d\n", fd_switchcom, fd_rs232);
    return OK;
}

unsigned char Uart__txcar(unsigned char Charac)
{
    int ret;
    ret=write(fd_rs232, &Charac, 1);
    dbg("%02x ", Charac);
    if (ret!=1)	 return KO;
    return OK;
}

unsigned short Uart__rxcar(unsigned short Timeout)
{
    int ret=0, first_time_flag=1;
    unsigned int to=Timeout, retVal;
    unsigned char hByte=0xff, lByte=0xff;
    unsigned char recv=0x0;

    Os__timer_start((unsigned int*)&to);
    do
    {
        ret=read(fd_rs232, &lByte, 1);

        to=Os__timer_remain();
    }
    while (ret!=1 && to!=0 );

    Os__timer_stop((unsigned int*)&to);

    if (to==0)
    {
        return COM_ERR_TO;
    }
    else
    {
        hByte=OK;
        retVal= (hByte <<8) | lByte;
    }
    return retVal;
}

// MODEM FUNCTION

/*************************************************
 		  ioctl command number
**************************************************/
#define OS_MODEM_DIAL_INIT 			0x400c4701
#define OS_MODEM_BACKDIAL 				0x40444703
#define OS_MODEM_BACKDIALCHECK 		0x40024704
#define OS_MODEM_READDIALPARAM 		0x40444705
#define OS_MODEM_SETPARAM 			0x40104706
#define OS_MODEM_TXCHAR 				0x40014707
#define OS_MODEM_RXCHAR 				0x40024708
#define OS_MODEM_HANGUP 				0x4709
#define OS_MODEM_SETPARAM_NEW            0x470a

struct rx_struct
{
    unsigned short timeout;
    unsigned char rec_data;
};

static int modem_fd;
static unsigned char send_buff[1024];
static unsigned short send_len = 0;

void set_timer()
{
    struct itimerval itv, oldtv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 100000;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 100000;

    setitimer(ITIMER_REAL, &itv, &oldtv);
}

void sigalrm_handler(int sig)
{
    signal(SIGALRM,SIG_IGN);

    if (write(modem_fd,send_buff,send_len)<0)
    {
        Uart_Printf("send error\n");
        memset(send_buff,0x00,1024);
        send_len = 0;
        close(modem_fd);
        return;
    }
    memset(send_buff,0x00,1024);
    send_len = 0;
    Uart_Printf("send finish\n");
    return;
}

void start_tx_timer(void)
{
    signal(SIGALRM, sigalrm_handler);
    set_timer();
}


unsigned char Os__init_com (COM_PARAM *ComParameter)
{
    unsigned char return_val;
    COM_PARAM *ComParameter_p;

    if (ComParameter->com_nr == 1)
        return Uart__init_com(ComParameter);

    modem_fd = open("/dev/modem",O_RDWR|O_NOCTTY);
    if (modem_fd<0)
    {
        Uart_Printf("open modem error\n");
        return COM_NOK;
    }
    modem_or_uart = 1;
    ComParameter_p = (COM_PARAM *)malloc(sizeof(struct  init_communication)+sizeof(unsigned char));
    if (!ComParameter_p)
    {
        close(modem_fd);
        return COM_NOK;
    }
    ComParameter_p->stop_bits = ComParameter->stop_bits;
    ComParameter_p->speed = ComParameter->speed;
    ComParameter_p->com_nr = ComParameter->com_nr;
    ComParameter_p->data_bits = ComParameter->data_bits;
    ComParameter_p->parity = ComParameter->parity;
    *(unsigned char *)(ComParameter_p+1) = COM_NOK;

    ioctl(modem_fd,OS_MODEM_DIAL_INIT,ComParameter_p);

    return_val = *(unsigned char *)(ComParameter_p+1);

    free(ComParameter_p);

    return return_val;
}

unsigned char Os__dial (DIAL_PARAM *DialParameter)
{
    close(modem_fd);
    return COM_NOK;
}

unsigned char Os__backgnd_dial (DIAL_PARAM *DialParameter)
{
    unsigned char return_val;
    DIAL_PARAM *DialParameter_p;

    DialParameter_p = (DIAL_PARAM *)malloc(sizeof(DIAL_PARAM)+sizeof(unsigned char));
    if (!DialParameter_p)
    {
        close(modem_fd);
        return COM_NOK;
    }
    memcpy(DialParameter_p,DialParameter,sizeof(DIAL_PARAM));
    *(unsigned char *)(DialParameter_p+1) = COM_NOK;

    ioctl(modem_fd,OS_MODEM_BACKDIAL,DialParameter_p);

    return_val = *(unsigned char *)(DialParameter+1);

    free(DialParameter_p);

    return 0x3a;
}

unsigned char Os__check_backgnd_dial (unsigned short TimeOut)
{
    unsigned char return_val;
    unsigned short *timeout_p;

    timeout_p = (unsigned short *)malloc(sizeof(unsigned short)+sizeof(unsigned char));
    if (!timeout_p)
    {
        close(modem_fd);
        return COM_NOK;
    }
    memcpy(timeout_p,&TimeOut,sizeof(unsigned short));

    ioctl(modem_fd,OS_MODEM_BACKDIALCHECK,timeout_p);

    return_val = *(unsigned char *)(timeout_p+1);

    free(timeout_p);

    return return_val;
}

void Os__read_dial_param (DIAL_PARAM * DialParameter)
{
    /* This api has not be used yet */
    memset(DialParameter,0x00,sizeof(DIAL_PARAM));
    return;
}

unsigned char Os__set_modem_parameter (unsigned char ParamNb, unsigned char NewValue)
{
    /* This api has not be used yet !  compatible return OK */
    return COM_OK;
}

unsigned char Os__txcar (unsigned char Charac)
{
    if (!modem_or_uart)
        return Uart__txcar(Charac);

    send_buff[send_len] = Charac;
    send_len++;
    if (send_len>=1024)
    {
        Uart_Printf("send buff full\n");
        close(modem_fd);
        return COM_NOK;
    }

    start_tx_timer();

    return COM_OK;

}

unsigned short Os__rxcar (ushort TimeOut)
{
    struct rx_struct *p;
    unsigned char retval;
    unsigned short rec_data = 0;
    int result;

    if (!modem_or_uart)
        return Uart__rxcar(TimeOut);

    p = (struct rx_struct *)malloc(sizeof(struct rx_struct)+sizeof(unsigned char));
    if (!p)
        return(COM_ERR_TO*256);

    p->timeout = TimeOut;

read_again:
    result = ioctl(modem_fd,OS_MODEM_RXCHAR,p);
    if (result <0)
        goto read_again;

    retval = *(unsigned char *)(p+1);

    if (retval == 0xFF)
        return(COM_ERR_TO*256);

    rec_data = (unsigned short)(p->rec_data);

    free(p);

    return rec_data;

}

void Os__hang_up (void)
{
    if (!modem_or_uart)
    {
        close(fd_rs232);
        return;
    }
    ioctl(modem_fd,OS_MODEM_HANGUP);
    close(modem_fd);
}

void Os__set_modem_parameter_new(unsigned char index)
{
    ioctl(modem_fd,OS_MODEM_SETPARAM_NEW,&index);
}

int main1(int argc,char* argv[])
{
    int retval;
    unsigned short uiStatus;
    unsigned int uiJ;
    unsigned char aucBuf[512];
    unsigned char write_data1[] = "123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567";
    unsigned char write_data2[] = "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg";
    int i,ii;
    unsigned char ret;

    COM_PARAM ComParameter =
    {
        1,
        115200,
        2,
        8,
        'N'
    };

    DIAL_PARAM  DialParameter;

    retval = Os__init_com(&ComParameter);
    if (retval)
    {
        Uart_Printf("Os__init_com error\n");
        Os__hang_up();
        return -1;
    }

    Os__read_dial_param(&DialParameter);

    DialParameter.prefix[0] = 0;
    strcpy(&DialParameter.phone_nr[0], "8276");
    DialParameter.auto_man = MANUAL_CALL;

    if (Os__backgnd_dial(&DialParameter) == STILL_DIALING)
    {
        Uart_Printf("Dialing\n");
    }

    if (Os__check_backgnd_dial(3000) == DIAL_CONNECT )
    {
        for (i=0;;i++)
        {
            Uart_Printf("\n*********send********\n");
            for (ii=0;ii<strlen(write_data1);ii++)
            {
                ret = Os__txcar(write_data1[ii]);
                if (ret == COM_NOK)
                    goto end;
            }
            Uart_Printf("%d\n",strlen(write_data1));
            Uart_Printf("\n*********begin rec:**********\n");
            uiStatus = Os__rxcar(3000);
            if ((uiStatus/256) == 0x00)
            {
                uiJ = 0;
                aucBuf[uiJ++] = (uiStatus%256);
                Uart_Printf(" %02x ",aucBuf[uiJ-1]);
                do
                {
                    uiStatus = Os__rxcar(10);
                    if (  (uiStatus/256) == 0x00)
                    {
                        aucBuf[uiJ++] = (uiStatus%256);
                        Uart_Printf(" %02x ",aucBuf[uiJ-1]);
                    }
                }
                while (  (uiStatus/256) == 0x00);
                Uart_Printf("\n\r");
            }
            else
            {
                Uart_Printf("rec faild\n");
                Os__hang_up();
            }
        }
    }
    else
        Uart_Printf("Not connected\n");

    sleep(10);
end:
    return 0;
}












