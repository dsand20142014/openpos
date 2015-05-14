
/*****************************************************************************
*	IC card reader Driver for S3C2410 SandBoard(TDA8007A circuit)
* author: zhangyong
*
*******************************************************************************/



/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  halicc.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-01-18		Xiaoxi Jiang
    Last modified :	2003-01-18		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __HALICC_H__
#define __HALICC_H__

#ifndef HALICCDATA
#define HALICCDATA extern
#endif

//#include <haldrv.h>
#ifdef __cplusplus
extern "C"
{
#endif


#define S3C2410_IC_IOCTL_BASE	'I'
#define DRIVER_VERSION "0.1"
#define DEVICE_NAME	"s3c2410_tda8007"


#define IC_HARDRESET		_IOW(S3C2410_IC_IOCTL_BASE, 0, int)//用于出错强制复位

#define IC_RESET _IOW(S3C2410_IC_IOCTL_BASE, 1, int)//初始化（包括冷复位和热复位）
#define IC_READ		_IOR(S3C2410_IC_IOCTL_BASE, 2, int)
#define IC_WRITE		_IOR(S3C2410_IC_IOCTL_BASE, 3, int)
#define IC_CMD		_IOWR(S3C2410_IC_IOCTL_BASE, 4, int)

    /**************************************************
    *关键数据
    **************************************************/

#define MAX_CARDER_GROUP 2
#define MAX_CARDER_NB MAX_CARDER_GROUP*2
#define MAX_REG_NB 16
#define ICBUF_MAX_LEN 256

    struct ic_info
    {
        //unsigned char group;//确定芯片的分组
        char reader;//分组内的编号，1，2号卡头
        unsigned long addr[16];//基地址,寄存器映射后的地址0615
        unsigned int irq;//中断
    };

    struct ic_device
    {
        struct ic_info ic_reader[MAX_CARDER_NB];// 0-3
        struct ic_info cur_reader;//当前读卡头

        unsigned char cur_reader_id;//0-3 // 定义当前的卡号,0-3
        unsigned char cur_cmd;	//定义当前的命令
        unsigned char pin[ICBUF_MAX_LEN]; //输入缓冲区,USR-->KERNEL-->CARD
        unsigned char pout[ICBUF_MAX_LEN];//输出缓冲区,USR<--KERNEL<--CARD
        int pin_len, pout_len;//数据的实际长度
        int sent_len, rcv_len;//发送接收到的长度，临时变量

        char flag_snd_finished;//是否发送完毕
        char flag_timeout;//1,2,3
        char flag_removed;
        char flag_send;//0,1
        char flag_err;

        //全局变量, 080716
        unsigned char bSendByte;
//				unsigned char ucAPDUOneByteFlag;
        unsigned char uiAPDUSentLen, uiAPDUSendLen;
        unsigned char bFinished;
        unsigned char uiRecvedLen;//已接收到的IC数据
        unsigned char uiRecvLen;//应用已经读取的数据

    };

//IC卡需要实现的命令列表
    enum CMD_TYPE
    {
        CMD_SELECTREADER=1,
        CMD_DETECT,
        CMD_RESET_UART,
        CMD_SET_CLOCK,

        CMD_SET_METHOD,//5
        CMD_SetBaud,
        CMD_SetDi,
        CMD_SetProtocol,
        CMD_SetConvention,

        CMD_SetPowerType,//10
        CMD_SetPEC,
        CMD_SetWaitTime,
        CMD_GET_METHOD,
        CMD_ACTIVE,

        CMD_DEACTIVE,//15
        CMD_READ_REG,
        CMD_WRITE_REG,
        CMD_TEST,//测试专用，现用于测试寄存器读写功能
        CMD_RD_INT_STATUS,

        CMD_RESETUAR,//20
        CMD_HANDLECONTACT,
        CMD_SetTimer,
        CMD_SETGUARDTIME,
        CMD_CHECK_BGT,

        CMD_EXEISO,//25 异步执行 EXEISO
        CMD_EN_RCV_AFTER_SEND,
        CMD_SENDCHAR,
        CMD_RCVCHAR,
        CMD_EN_RCVCHAR,

        CMD_EN_SENDCHAR,//3//30
        CMD_RD_GLOBAL_VAR,//读取全局变量值
        CMD_WR_GLOBAL_VAR //设置全局变量值

    };


// execution status from ICC and SIM driver (drv_status)
#define OK              0
#define KO              1
#define NOICC           1
#define NOREADER        2
#define OVERRUN         6
#define DRVUNKNOWN      7
#define WRONGREADER     10
#define WRONGORDER      11
#define NULLPTOUT       12

    /*****************************************************************************/
//0615
    enum ic_regs
    {
//Card Select Register
        REG_CSR=0,
        REG_CCR,//Clock Configuration Register1
        REG_PDR,//Programmable Divider Register
        //UART Configuration Register2
        REG_UCR2=3,

        //Guard Time Register
        REG_GTR=5,

        //UART Configuration Register1
        REG_UCR1,

        //Power Control Register
        REG_PCR,

        //Timeout Counter Register1
        REG_TOC,

        REG_TOR1,//9
        REG_TOR2,//10
        REG_TOR3,//11

        REG_MSR=12,//12
        REG_FCR=12,//12

        REG_URR=13,//13
        REG_UTR=13,//13

        REG_USR=14,
        REG_HSR=15//0x0f, 15
    };

#define UCR2_PSC		(0x1<<0)//prescale
#define UCR2_CKU		(0x1<<1) //CKU
#define UCR2_AUTOC 	(0x1<<2)// /AUTOC
#define UCR2_SAN		(0x1<<3)//SAN
#define UCR2_PDWN		(0x1<<4)//PDWN
#define UCR2_DISAUX (0x1<<5) //DISAUX
#define UCR2_DISTBE_RBF (0x1<<6) //DISTBE_/RBF

//#define REG_UCR1            0x06
#define UCR1_CONV		(0x1<<0)//CONV
#define UCR1_SS		(0x1<<1) //CKU
#define UCR1_LCT 	(0x1<<2)// /AUTOC
#define UCR1_T_R		(0x1<<3)//SAN
#define UCR1_PROT		(0x1<<4)//PDWN
#define UCR1_FC 		(0x1<<5) //DISAUX
#define UCR1_FIP 	(0x1<<6) //DISTBE_/RBF


//Power Control Register
//#define REG_PCR             0x07
#define PCR_START						(0x1<<0)
#define PCR_3V_5V						(0x1<<1)
#define PCR_RSTIN						(0x1<<2)
#define PCR_1V8						(0x1<<3)
#define PCR_C4						(0x1<<4)
#define PCR_C8						(0x1<<5)


//Timeout Counter Register1
//#define REG_TOC             0x08
#define TOC_TOC0 0x01
#define TOC_TOC1 0x02
#define TOC_TOC2 0x04
#define TOC_TOC3 0x08
#define TOC_TOC4 0x10
#define TOC_TOC5 0x20
#define TOC_TOC6 0x40
#define TOC_TOC7 0x80

//#define REG_TOR1            0x09
//#define REG_TOR2            0x0A
//#define REG_TOR3            0x0B

//Mixed Receive Register1
//#define REG_MSR             0x0C
#define MSR_TBE_RF 					(0x1<<0)
#define MSR_INTAUX 					(0x1<<1)
#define MSR_PR1							(0x1<<2)
#define MSR_PR2							(0x1<<3)
#define MSR_BGT							(0x1<<5)
#define MSR_FE							(0x1<<6)


//FIFO Control Register
//#define REG_FCR             0x0C
//UART Receive Register1
//#define REG_URR             0x0D
//UART Transmit Register1
//#define REG_UTR             0x0D


//UART Status Register1
//#define REG_USR             0x0E
#define USR_TBF_RBF 	0x01
#define USR_FER 0x02
#define USR_OVR 0x04
#define USR_PE 0x08
#define USR_EA 0x10
#define USR_TO1 0x20
#define USR_TO2 0x40//not used
#define USR_TO3 0x80

//Hardware Status Register
//#define REG_HSR             0x0F
#define HSR_PTL 0x01
#define HSR_INTAUXL 0x02
#define HSR_PRL1 0x04
#define HSR_PRL2 0x08
#define HSR_SUPL 0x10
#define HSR_PRTL1 0x20
#define HSR_PRTL2 0x40





    /*读,写取寄存器信息*/
    typedef struct
    {
        unsigned char ucReader;
        unsigned char ucREG;
        unsigned char val[2];
        unsigned char wr_check;	// 写寄存器，检测
    }REG_INFO;


#define LIBICC_READERNB 4





#define HALICCERR_SUCCESS           0x00
#define HALICCERR_CHIPNOTEXIST      0x03
#define HALICCERR_INVALIDREADER     0x04
#define HALICCERR_NOTPOWER          0x07
#define HALICCERR_TOOLONGSENDDATA   0x08
#define HALICCERR_INVALIDCONTACT    0x0A
#define HALICCERR_CHIP1NOTEXIST     0x0B
#define HALICCERR_CHIP2NOTEXIST     0x0C
#define HALICCERR_RECVNODATA		0x0D

#define HALICCTYPE_TDA8002          0x01
#define HALICCTYPE_TDA8007          0x02


#define HALICCCONTACT_GET       0x00
#define HALICCCONTACT_CLR       0x01
#define HALICCCONTACT_SET       0x02

#define HALICCCONTACT_C1        0x01
#define HALICCCONTACT_C2        0x02
#define HALICCCONTACT_C3        0x03
#define HALICCCONTACT_C4        0x04
#define HALICCCONTACT_C5        0x05
#define HALICCCONTACT_C6        0x06
#define HALICCCONTACT_C7        0x07
#define HALICCCONTACT_C8        0x08
#define HALICCCONTACT_VCC       0x01
#define HALICCCONTACT_RST       0x02
#define HALICCCONTACT_CLK       0x03
#define HALICCCONTACT_IO        0x07

#define ICCTIMERTYPE_STOP       0x00
#define ICCTIMERTYPE_RSTKEEP    0x01
#define ICCTIMERTYPE_ISORST1    0x02
#define ICCTIMERTYPE_ISORST2    0x03

#define HALICC_READERNB         4
#define HALICC_READER0          0x00
#define HALICC_READER1          0x01
#define HALICC_READER2          0x02
#define HALICC_READER3          0x03

#define TDA8007B_C2             0x02
#define TDA8007B_C3             0x03
#define TDA8007BASEADDR1        0x4200000
#define TDA8007BASEADDR2        0x4300000




#define HALICC_POWER5V      0x00
#define HALICC_POWER3V      0x01
#define HALICC_POWER18V     0x02

#define HALICCSTEP_IDLE         0x00
#define HALICCSTEP_POWERED      0x04
#define HALICCSTEP_APDUEXCHSEND 0x05
#define HALICCSTEP_APDUEXCHRECV 0x06

#define HALICCAPDUSTEP_T0SEND   0x01


#ifdef __cplusplus
}
#endif

#endif

