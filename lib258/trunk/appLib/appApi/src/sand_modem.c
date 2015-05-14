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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sched.h>

#include "osmodem.h"
#include "sand_incs.h"
//Uart

#ifdef EN_DBG
#define dbg Uart_Printf
#else
#define dbg
#endif

#define unlikely_zwj(x)	__builtin_expect(!!(x), 0)

static unsigned char modem_or_uart = 0;   //0 Uart 1 Modem

int fd_rs232;
int nread;
char *modem_rs232 ="/dev/s3c2410_serial2";

/*************************************************
  msg section
**************************************************/
static int create_sr_msg_zwj(void)
{
    int rec_queue_id;
    key_t rec_queue_key;

    if ( (rec_queue_key = ftok("/usr/",SEED_SR)) == -1)
    {
        perror("deamon:ftok");
        return -1;
    }

    if ((rec_queue_id = msgget(rec_queue_key,0)) == -1)
    {
        return -1;
    }
    return rec_queue_id;
}

static int create_ss_msg_zwj(void)
{
    int send_queue_id;
    key_t send_queue_key;

    if ( (send_queue_key = ftok("/usr/",SEED_SS)) == -1)
    {
        perror("deamon:ftok");
        return -1;
    }

    if ((send_queue_id = msgget(send_queue_key,0)) == -1)
    {
        return -1;
    }
    return send_queue_id;

}

int msg_send_zwj(int send_queue_id, struct message_type *send_packet,int flag)
{
    return msgsnd(send_queue_id,send_packet,BUFF_SIZE,flag);
}

int msg_rec_zwj(int rec_queue_id,struct message_type *rec_packet,long type,int flag)
{
    return msgrcv(rec_queue_id,rec_packet,sizeof(struct message_type),type,flag);
}

int msg_remove_zwj(int id)
{
    if (msgctl(id,IPC_RMID,NULL) == -1)
        return -1;
    return 0;
}

/*************************************************
  share memory
**************************************************/
int shm_create_zwj(int size)
{
    int shmid;
    key_t shmkey;

    shmkey = ftok("/usr/",SHM_SEED);
    shmid = shmget(shmkey,size,IPC_CREAT | 0666);
    if (shmid == -1)
        perror("shm_create_zwj");

    return shmid;
}

int shm_id_get_zwj(int size)
{
    int shmid;
    key_t shmkey;

    shmkey = ftok("/usr/",SHM_SEED);
    shmid = shmget(shmkey,size,0666);
    if (shmid == -1)
        perror("shm_id_get_zwj");

    return shmid;
}

unsigned char* shm_get_ptr_zwj(int shm_id)
{
    return shmat(shm_id,0,0);
}

void __inline shm_release_ptr_zwj(unsigned char *p)
{
    shmdt(p);
}

void shm_remove_zwj(int shm_id)
{
    shmctl(shm_id, IPC_RMID, NULL);
}

void shm_clear_zwj(int shm_id)
{
    unsigned char *shm_p;

    shm_p = shm_get_ptr_zwj(shm_id);

    memset(shm_p,0x00,(BUFF_SIZE*2));

    shm_release_ptr_zwj(shm_p);
}


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
//  Uart_Printf("fd_switchcom=%d  fd_rs232=%d\n", fd_switchcom, fd_rs232);
    return OK;
}

unsigned char Uart__txcar(unsigned char Charac)
{
    int ret;
    unsigned char tmp;

    tmp = Charac;
    ret=write(fd_rs232, &tmp, 1);

    if (ret!=1)
        return 0x01;
    return 0x00;
}

#define dbg_rx //Uart_Printf
unsigned short Uart__rxcar(unsigned short Timeout)
{
    int ret=0, first_time_flag=1;
    unsigned int to=Timeout, retVal;
    unsigned char hByte=0xff, lByte=0xff;
    unsigned char recv=0x0;

    Os__timer_start((unsigned int*)&to);
    dbg_rx("\n\n************\n%s before while to=%d\n", __func__, to);
    do
    {
        ret=read(fd_rs232, &lByte, 1);
    }
    while (ret<=0 && to>0 );

    dbg_rx("%s out of while to=%d, ret=%d\n***************\n\n", __func__, to, ret);

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

// MODEM FUNCTION
/*************************************************
 		  ioctl command number
**************************************************/
//static int send_queue_id;
//static int rec_queue_id;

static unsigned char send_buff[2048];
static unsigned short send_len = 0;
static unsigned char send_err = 0;

static unsigned char asyn_or_syn = 0;
//	 0 syn
// 	 1 syn

void set_timer()
{
    struct itimerval itv, oldtv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 400000;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 400000;

    setitimer(ITIMER_REAL, &itv, &oldtv);
}

void sigalrm_handler(int sig)
{
    unsigned char len_h;
    unsigned char len_l;
    struct message_type send_message;
    struct message_type rec_message;

    int send_queue_id;
    int rec_queue_id;
	int j=0;

    signal(SIGALRM,SIG_IGN);

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    memset(&send_message,0x00,sizeof(struct message_type));
    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_SEND_I_FRAME;

    len_h =(unsigned char)(send_len>>8);
    len_l =(unsigned char)(send_len);

    send_message.buffer[1] = len_h;
    send_message.buffer[2] = len_l;

    memcpy(&send_message.buffer[3],send_buff,send_len);
#if 1
	for (j=0; j<send_len; j++)
	{
		Uart_Printf("%02x ", send_buff[j]);
	}
	Uart_Printf("\n");
#endif

    send_len = 0;

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("sigalrm_handler:");
        send_err = 1;
        memset(send_buff,0x00,2048);
        return;
    }

    memset(send_buff,0x00,2048);
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
    int send_queue_id;
    int rec_queue_id;
    struct message_type send_message;
    struct message_type rec_message;

    if (ComParameter->com_nr == 1)
        return Uart__init_com(ComParameter);
    if (ComParameter->com_nr == 0)
    {
        asyn_or_syn = 1;
    }
    else
    {
        asyn_or_syn = 0;
    }

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    send_err = 0;
    modem_or_uart = 1;

    if ( ((rec_queue_id) == -1)||(send_queue_id == -1))
    {
        perror("LSI_modem_init:");
        return MODEM_ERR;
    }

    memset(send_message.buffer,0x00,BUFF_SIZE);

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_INIT_COM;
    memcpy(&send_message.buffer[1],ComParameter,sizeof(COM_PARAM));

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        Uart_Printf("send_queue_id :%d\n",send_queue_id);
        perror("msg_send:");
        return MODEM_ERR;
    }

    if (msg_rec_zwj(rec_queue_id,&rec_message,MODEM_INIT_COM,0) == -1)
    {
        perror("Os__check_backgnd_dial:");
        return MODEM_ERR;
    }

    if (rec_message.buffer[0] != 0x00)
    {
        Uart_Printf("%s error\n",__FUNCTION__);
        return MODEM_ERR;
    }

    return MODEM_SUCCESS;
}

unsigned char Os__dial (DIAL_PARAM *DialParameter)
{
    return COM_NOK;
}

unsigned char Os__backgnd_dial (DIAL_PARAM *DialParameter)
{
    int send_queue_id;
    int rec_queue_id;
    struct message_type send_message;

    memset(send_message.buffer,0x00,BUFF_SIZE);

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_BGND_DIAL;
    memcpy(&send_message.buffer[1],DialParameter,sizeof(DIAL_PARAM));

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("msg_send_zwj:");
        return MODEM_ERR;
    }
    return (0x3A);
}

unsigned char Os__check_backgnd_dial (unsigned short TimeOut)
{
    unsigned char *p;
    unsigned char len_h;
    unsigned char len_l;
    unsigned short timeout;
    int send_queue_id;
    int rec_queue_id;

    struct message_type send_message;
    struct message_type rec_message;

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();


    memset(send_message.buffer,0x00,BUFF_SIZE);
    memset(rec_message.buffer,0x00,BUFF_SIZE);

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_CHECK_DIAL;

    timeout = TimeOut;

    len_h = (unsigned char)(timeout >>8);
    len_l = (unsigned char)timeout;

    send_message.buffer[1] = len_h;
    send_message.buffer[2] = len_l;

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("msg_send_zwj:");
        return MODEM_ERR;
    }

    if (msg_rec_zwj(rec_queue_id,&rec_message,MODEM_CHECK_DIAL,0) == -1)
    {
        perror("Os__check_backgnd_dial:");
        return MODEM_ERR;
    }

    if (rec_message.buffer[0] == 0xFF)
        rec_message.buffer[0] = 0x09;

    return rec_message.buffer[0];
}

void Os__read_dial_param (DIAL_PARAM * DialParameter)
{
    /* This api has not be used yet */
    return;
}

unsigned char Os__set_modem_parameter (unsigned char ParamNb, unsigned char NewValue)
{
    /* This api has not be used yet !  compatible return OK */
    // return Os__set_modem_parameter(ParamNb,NewValue);
    return COM_OK;
}

void Os__hang_up (void)
{
    int send_queue_id;
    int rec_queue_id;

    struct message_type send_message;

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    memset(&send_message,0x00,sizeof(struct message_type));

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_HANGUP;

    Uart_Printf(" %s send id=%d, hangup to be....", __func__, send_queue_id);
    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("msg_send_zwj:");
        return;
    }
    Uart_Printf("%s OK\n", __func__);
    if (asyn_or_syn)
    {
        sleep(4);
    }
    else
    {
        sleep(1);
    }

    return;
}

unsigned char Os__txcar (unsigned char Charac)
{
    if (!modem_or_uart)
        return Uart__txcar(Charac);

    if (send_err)
        return COM_NOK;

    send_buff[send_len] = Charac;
    send_len++;
    if (send_len>=2048)
    {
        Uart_Printf("send buff full\n");
        return COM_NOK;
    }

    start_tx_timer();

    return COM_OK;
}

unsigned short Os__rxcar (unsigned short TimeOut)
{
    unsigned char rec_data = 0;
    unsigned short retval = 0;
    unsigned long check_times;
    int result;
    int shm_id;
    int send_queue_id;
    int rec_queue_id;

    struct shm_struct *shm_p;

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    if (!modem_or_uart)
        return Uart__rxcar(TimeOut);

    check_times = TimeOut;

    shm_id = shm_id_get_zwj(2*BUFF_SIZE);
    shm_p = (struct shm_struct *)shm_get_ptr_zwj(shm_id);

    while (check_times --)
    {
        if (shm_p->connect_status != 0x26)
            goto error_end;

        if (shm_p->data_in > shm_p->data_out)
        {
            retval = shm_p->info_data[shm_p->data_out];
            shm_p->data_out++;

            if (shm_p->data_in == shm_p->data_out)
            {
                shm_p->data_in = 0;
                shm_p->data_out = 0;
            }

            shm_release_ptr_zwj((unsigned char *)shm_p);
            //Uart_Printf("0x%02x\n",retval);
            return retval;
        }
        usleep(5000);
    }
	
error_end:
    Uart_Printf("has error\n");
    shm_release_ptr_zwj((unsigned char *)shm_p);
    return(COM_ERR_TO*256);
}

void Os_write_data_to_modem(unsigned char *data,unsigned short len)
{
    int send_queue_id;
    int rec_queue_id;
    unsigned char len_h;
    unsigned char len_l;
    struct message_type send_message;

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    memset(&send_message,0x00,sizeof(struct message_type));

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_SEND_I_FRAME;

    len_h =(unsigned char)(len>>8);
    len_l =(unsigned char)(len);

    send_message.buffer[1] = len_h;
    send_message.buffer[2] = len_l;

    memcpy(&send_message.buffer[3],data,len);

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("msg_send_zwj:");
        return;
    }
    return;
}

void Os__modem_wild_adjust(unsigned char index)
{
    int send_queue_id;
    int rec_queue_id;

    struct message_type send_message;

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    memset(&send_message,0x00,sizeof(struct message_type));

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_SET_PARAM;
    send_message.buffer[1] = index;

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("msg_send_zwj:");
        return;
    }
    return;
}

void Os__modem_small_adjust(struct osdialparam *osdialparam_p)
{
    int send_queue_id;
    int rec_queue_id;

    struct message_type send_message;

    send_queue_id = create_sr_msg_zwj();
    rec_queue_id = create_ss_msg_zwj();

    memset(&send_message,0x00,sizeof(struct message_type));

    send_message.type = CLIENT_TO_SERVER_MSG;
    send_message.buffer[0] = MODEM_SMALL_ADJUST;

    memcpy(&send_message.buffer[1],osdialparam_p,sizeof(struct osdialparam));

    if (msg_send_zwj(send_queue_id,&send_message,0) == -1)
    {
        perror("msg_send_zwj:");
        return;
    }

    return;
}

/*************************************************************************
*
* Sand API for OSMODEM_
* added by liu, 2010-01-19
**************************************************************************/
unsigned char OSMODEM_Init(void)
{
    return 0;
}

unsigned char OSMODEM_Test(void)
{
    return(DIAL_NO_MODEM);
}

unsigned char OSMODEM_DialInit(COM_PARAM *pComParam)
{
    return Os__init_com(pComParam);
}

unsigned char OSMODEM_Dial(DIAL_PARAM * pDialParam)
{
    return Os__dial(pDialParam);
}

unsigned char OSMODEM_BackDial(DIAL_PARAM * pDialParam)
{
    return Os__backgnd_dial(pDialParam);
}

unsigned char OSMODEM_CheckBackDial(unsigned short uiTimeout)
{
    return Os__check_backgnd_dial(uiTimeout);
}

void OSMODEM_ReadDialParam(DIAL_PARAM *pDialParam)
{
    return Os__read_dial_param(pDialParam);
}

unsigned char OSMODEM_SetParam(unsigned char ucParamNb, unsigned char ucValue)
{
    return Os__set_modem_parameter(ucParamNb,ucValue);
}

unsigned char OSMODEM_GetRegister(unsigned short uiParamNb, unsigned char *pucValue)
{
    return(DIAL_NO_MODEM);
}

unsigned char OSMODEM_SetRegister(unsigned short uiParamNb, unsigned char ucValue)
{
    return(DIAL_NO_MODEM);
}

unsigned char OSMODEM_TxChar(unsigned char ucCh)
{
    return Os__txcar(ucCh);
}

unsigned short OSMODEM_RxChar(unsigned short uiTimeout)
{
    return Os__rxcar(uiTimeout);
}

unsigned char OSMODEM_Hangup(void)
{
    Os__hang_up();
    return MODEM_SUCCESS;
}

unsigned char OSMODEM_ParamDefault(void)
{
    return (0);
}
unsigned char OSMODEM_SetDialParam(/*OSDIALPARAM*/void *pDialParam )
{
    return (0);
}

//unsigned char OSMODEM_SetSyncParam(OSSYNCPARAM * pSyncParam )
unsigned char OSMODEM_SetSyncParam(void * pSyncParam )
{
    return (0);
}

unsigned char OSMODEM_CheckVoltage(void)
{
    return(0);
}

unsigned char OSMODEM_SendFrame(unsigned char *pucFrame,unsigned short uiFrameLen,unsigned short uiTimeout)
{
    Os_write_data_to_modem(pucFrame,uiFrameLen);
    return (0);
}

unsigned char OSMODEM_RecvFrame(unsigned char *pucFrame,unsigned short *puiFrameLen,unsigned short uiTimeout)
{
    return(DIAL_NO_MODEM*256);
}

unsigned char	OSMODEMPPP_Open(unsigned char* paucUserName,unsigned char *paucPassword)
{
    Uart_Printf("\nneed %s \n",__func__);
}
////////////////////////// END //////////////////////


