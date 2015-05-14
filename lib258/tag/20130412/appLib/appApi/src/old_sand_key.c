/*
这里包括了部分加密函数功能
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <termio.h>
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <time.h>       /*时间函数*/
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>


#define __LUXAPP_KBD__
#include "sand_key.h"

#ifdef EN_DBG
#define dbg Uart_Printf
#else
#define dbg
#endif

char *KEY_DEVICE="/dev/ttyS1";
static int fd_comkey=0;
DRV_OUT *kbd=NULL;


/*
 *工作方法：发送命令后等待应答，取键值
 * 返回读到的长度；
 *
 */
int get_key_from_serial(char *dev, unsigned char *data, unsigned char *status)
{
    int fd, ret, i;
    int count=100;


    return ret;
}



unsigned char scancode_to_asc(unsigned char in)
{

#if 0
    unsigned char out;
    dbg("in=0x%x ",  in);
    switch (in)
    {
    case SKBKEY_ENTER1:
    case SKBKEY_ENTER2:
        out=0x0d;
        break;
    case SKBKEY_N0: 	//数字0
        out= '0';
        break;
    case SKBKEY_N1: 	//数字 1
        out= '1';
        break;
    case SKBKEY_N2: 	//数字 2
        out= '2';
        break;
    case SKBKEY_N3: 	//数字 3
        out= '3';
        break;
    case SKBKEY_N4: 	//数字 4
        out= '4';
        break;
    case SKBKEY_N5: 	//数字 5
        out= '5';
        break;
    case SKBKEY_N6: 	//数字 6
        out= '6';
        break;
    case SKBKEY_N7: 	//数字 7
        out= '7';
        break;
    case SKBKEY_N8: 	//数字 8
        out= '8';
        break;
    case SKBKEY_N9: 	//数字 9
        out= '9';
        break;

    case SKBKEY_N00: 	//符号 点 .
        out= KEY_00_PT;
        break;


    case SKBKEY_F1:
        out=KEY_F1;
        break;
    case SKBKEY_F2:
        out=KEY_F2;
        break;
    case SKBKEY_F3:
        out=KEY_F3;
        break;
    case SKBKEY_F4:
        out=KEY_F4;
        break;

    case SKBKEY_CANCEL: 	//取消键,esc?
        out= KEY_CANCEL;
        break;

    case SKBKEY_CLEAR: 	//清除键,backspace?
        out= KEY_BCKSP;
        break;

    case SKBKEY_SUBTOTAL:	// 小计
        out=KEY_SUBTOTAL;
        break;

    case SKBKEY_PAPER_FEED: 	//用于进纸键
        out= KEY_PAPER_FEED;
        break;

    case SKBKEY_PAPER_REVERSEFEED: 	//用于进纸键
        out= KEY_PAPER_REVERSEFEED;
        break;

    case SKBKEY_CASHIER:		//营业员
        out=CASHKEY_CASHIER;
        break;

    case SKBKEY_JUMP:			//jump out
        out=KEY_JUMP;
        break;

    case SKBKEY_ADDING:
        out=KEY_ADDING;
        break;
    case SKBKEY_DECREASE:
        out=KEY_DECREASE;
        break;
    case SKBKEY_MUITIPLY:
        out=KEY_MUITIPLY;
        break;
    case SKBKEY_DIVISION:
        out=KEY_DIVISION;
        break;


        //字母键
    case SKBKEY_A: 	//字母a
        out= 'A';
        break;

    case SKBKEY_B: 	//字母 b
        out= 'B';
        break;

    case SKBKEY_C: 	//字母 c
        out= 'C';
        break;

    case SKBKEY_D: 	//字母 d
        out= 'D';
        break;

    case SKBKEY_E: 	//字母 e
        out= 'E';
        break;

    case SKBKEY_F: 	//字母 f
        out= 'F';
        break;

    case SKBKEY_G: 	//字母 g
        out= 'G';
        break;

    case SKBKEY_H: 	//字母 h
        out= 'H';
        break;

    case SKBKEY_I: 	//字母 i
        out= 'I';
        break;

    case SKBKEY_J: 	//字母 j
        out= 'J';
        break;

    case SKBKEY_K: 	//字母 k
        out= 'K';
        break;

    case SKBKEY_L: 	//字母 l
        out= 'L';
        break;

    case SKBKEY_M: 	//字母 m
        out= 'M';
        break;

    case SKBKEY_N: 	//字母 n
        out= 'N';
        break;

    case SKBKEY_O: 	//字母 o
        out= 'O';
        break;

    case SKBKEY_P: 	//字母 p		//?
        out= 'P';
        break;

    case SKBKEY_Q: 	//字母 q
        out= 'Q';
        break;

    case SKBKEY_R: 	//字母 r
        out= 'R';
        break;

    case SKBKEY_S: 	//字母 s
        out= 'S';
        break;

    case SKBKEY_T: 	//字母 t
        out= 'T';
        break;

    case SKBKEY_U: 	//字母 u
        out= 'U';
        break;

    case SKBKEY_V: 	//字母 v
        out= 'V';
        break;

    case SKBKEY_W: 	//字母 w
        out= 'W';
        break;

    case SKBKEY_X: 	//字母 x
        out= 'X';
        break;

    case SKBKEY_Y: 	//字母 y
        out= 'Y';
        break;

    case SKBKEY_Z: 	//字母 z
        out= 'Z';
        break;

    default:
        out= 0x00;
        break;
    }//switch end
    dbg("out=%c(%02x)\n", out, out);
    return out;
#endif

}


void EatAnyKey()
{

}



DRV_OUT * OSMMI_GetKey()
{
    DRV_OUT *tmp;
    tmp=Os__get_key();
    return tmp;
}


/*
作用：产生一个按键事件, 并返回键值。
函数： DRV_OUT * Os__get_key () ;
等待：否
说明：Os__get_key将激活终端键盘, 同时不等待按键输入, 由应用控制终端按键返回。
返回值： 指向 DRV_OUT 的数据结构指针。
         结构定义如下：
		reqnr： 暂不使用.
		gen_status： 系统模块状态。
		drv_status： 系统执行状态
		xxdata[0]        :：返回数据长度, 长度位一字节。
		xxdata[1]        ：返回键值, 此为asc码。
*/
DRV_OUT * Os__get_key ()
{
    unsigned char data, status;
    char *dev_path=KEY_DEVICE;
    int ret;

    kbd=(DRV_OUT *)malloc(sizeof(DRV_OUT));
    memset(kbd, 0, sizeof(kbd));

    ret=get_key_from_serial(dev_path, &data, &status);
    if (ret<0) return NULL;

    kbd->xxdata[0]=1;
    kbd->xxdata[1]=data;
    kbd->gen_status=status;
    return kbd;
}

DRV_OUT * Os__get_key_pp(void)
{
}

unsigned char   Os__xget_key_pp(void)
{
}




/**
 *获取按键状态及键值 OK
 */
unsigned char  Os__get_key_status()
{
    unsigned int tmp;
    DRV_OUT tmp_drv;
    unsigned char data=0, status;
    char *dev_path=KEY_DEVICE;
    int ret;

    ret=get_key_from_serial(dev_path, &data, &status);

    if (status==DRV_ENDED)
    {
        kbd->xxdata[0]=0;
        kbd->xxdata[1]=scancode_to_asc(data);	/*asc码*/
        dbg("xxdata1=%c tmp_data=%x\n", kbd->xxdata[1], data);
    }
    kbd->gen_status=status;

    return status;
}


/*
 *关闭串口，释放指针
 */
/*
void Os__stop_key()
{
	dbg("fd_comkey=%d\n", fd_comkey);

	if (fd_comkey>0)
	{
		ioctl(fd_comkey, CMD_TIMER_KBD_STOP);
		close(fd_comkey);
	}
	if (kbd!=NULL) {
		dbg("kbd is not NULL\n");
//		kbd=NULL;
		free(kbd);
	}
	else
		dbg("kbd is NULL\n");
}
*/


/*
作用	： 得到一按键信息。
函数	： unsigned char Os__xget_key () ;
等待	：是
说明	：等待终端按键, 如终端无按键返回, 系统将一直等待, 它不同于函数Os__get_key()。
返回值	：键值。
ok
*/
unsigned char Os__xget_key ()
{
    unsigned char data, status;
    char *dev_path=KEY_DEVICE;
    int ret;

    while (1)
    {
        ret=get_key_from_serial(dev_path, &data, &status);
        if (ret<0)
        {
            Uart_Printf("%s get key failed:%d \n", ret);
            return -1;
        }

        if (status==KEY_REP_OK)
        {
            return data;
        }
    }
}

unsigned char Os__read_eeprom(unsigned short uiStartAddr, unsigned char ucLen, unsigned char *pucPtr)
{
}

void Os__crypt(unsigned char *pucCryptKey)
{

}
unsigned char *Os__xget_pin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{

}

DRV_OUT *Os__get_fixkey(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB)
{

}


DRV_OUT *Os__get_pin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB)
{
}
int key_main(char argc, char *argv[])
{
    int ret;
    char *dev_path=KEY_DEVICE;
    unsigned char status, data;



    return 0;
}

