
/*****************************************************************************
*	IC card reader Driver for S3C2410 SandBoard(TDA8007A circuit)
* author: zhangyong
*
*******************************************************************************/

#ifndef __TDA8007_ICC_H__
#define __TDA8007_ICC_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <linux/irq.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/mutex.h>
#include <linux/connector.h>
#include "osicc.h"

#include "haldrv.h"

int dbg = 0;

#define CN_IDX_ICC  11
#define CN_VAL_ICC  0x456

#define CARD_EJECTED  0xAAAA
#define CARD_INSERTED 0xFFFF

#define NETLINK_GXDICC		21

#define  KEY_SIG  (SIGIO)
#define  MAG_SIG  (SIGRTMIN+6)
#define  MFR_SIG  (SIGRTMIN+7)
#define  ICC_SIG  (SIGRTMIN+8)



#define CONTROL_MSG     0x10
#define NOT_MIRROR      0
#define DO_MIRROR       0x01
#define DATA_MSG        0x11

//#define NETLINK_GXDICC		21

extern int packet_transmit(struct sk_buff*);


#define S3C2410_IC_IOCTL_BASE	'I'
#define DRIVER_VERSION "1.1"
#define DEVICE_NAME	"TDA8007"
#define PROBE_NAME "GXD_ICC"

#define HV4

#define pSMDK2410_IC1_IO  0xa0000000
#define pSMDK2410_IC2_IO  0xa0100000

#define TDA8007_INT_GROUP0 IOMUX_TO_IRQ(MX25_PIN_GPIO_B)	//INT MX25_PIN_GPIO_B
#define TDA8007_INT_GROUP1 IOMUX_TO_IRQ(MX25_PIN_GPIO_D)    //�ж���� MX25_PIN_GPIO_D

//#define INT_4GPIO_B_CTL  (PAD_CTL_DRV_NORMAL |  PAD_CTL_ODE_CMOS |  PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE |  PAD_CTL_PUE_PULL)
#define INT_4GPIO_B_CTL  (PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE |  PAD_CTL_PUE_PULL)


#define IC_CMD_PROCESS 	 _IOWR(S3C2410_IC_IOCTL_BASE, 0, DRV)
#define IC_CMD_POWUP _IOWR(S3C2410_IC_IOCTL_BASE, 1, DRVOUT)
#define IC_CMD_POWDOWN _IOWR(S3C2410_IC_IOCTL_BASE, 2, DRVOUT)

#define ICC_IOC_POLLING_START    _IO('I', 16)
#define ICC_IOC_POLLING_ABORT    _IO('I', 17)

    /**************************************************
    *            �ؼ����
    **************************************************/
#define MAX_CARDER_GROUP 2
#define MAX_CARDER_NB MAX_CARDER_GROUP*2
#define MAX_REG_NB 16
#define ICBUF_MAX_LEN 256



#pragma pack(push)
#pragma pack(4)

typedef struct {
	unsigned char ucType;
	unsigned char ucLen;
	unsigned char pad;
	unsigned char pad2;
	unsigned int  ioctl_cmd;
}DRVIN258;

typedef struct{
	unsigned char ucReqNR;
	unsigned char ucGenStatus;
	unsigned char ucDrvStatus;
	unsigned char pad;
}DRVOUT258;

typedef struct
{
	unsigned char ucDrvID;
	unsigned char ucDrvCMD;
	unsigned char pad;
	unsigned char pad2;
	DRVIN258      pDrvIn;
	DRVOUT258     pDrvOut;
	unsigned char sand_data[400];
//	unsigned char* sand_data;
}  DRV258;

#pragma pack(pop)


//*************************************************
// used for exchange for IC_CMD
//************************************************
    struct OS_CMD_INFO
    {
        char reader;
        ICC_ORDER pOrder;
    };

    struct ic_info
    {
        unsigned char chipid;//ȷ��оƬ�ķ���, [0,1]
        char reader;//�����ڵı�ţ�1��2�ſ�ͷ[1,2]
        unsigned long addr[16];//���ַ,�Ĵ���ӳ���ĵ�ַ0615
        unsigned int irq;//�ж�
    };
    typedef struct ic_info ICINFO;

struct icc_dev {
        struct semaphore sem ;     /* Mutual exclusion */
        struct cdev cdev ;
        struct class *ic_class;
	DRV_MMAP    *gMMAP_IC_Drv;
	struct fasync_struct *fasync_queue;
	unsigned int  card_status;
};

    struct ic_device
    {
        //struct ic_info ic_reader[MAX_CARDER_NB];// 0-3
        ICINFO ic_reader[MAX_CARDER_NB];

        struct ic_info cur_reader;//��ǰ����ͷ, not use it!!!
        /******************************************************************
        *usage:
        *    ic_reader[cur_reader_id]
        *******************************************************************/
        unsigned char cur_reader_id;//0-3 // ���嵱ǰ�Ŀ���,0-3

        unsigned char cur_cmd;	//���嵱ǰ������
        /*
         *   pin[0] �����
         *			[1] order_type,eg:
        		 *				0 direct data => impossible in T=0 protocol
         *				type1 : CLA,INS,P1,P2
         *				type2 : CLA,INS,P1,P2,Le
         *	      type3 : CLA,INS,P1,P2,Lc,data...
         *	      type4 : CLA,INS,P1,P2,Lc,data...,Le
         *  			5:     XXXXXXX
         *				6:direct data > 255  => impossible in T=0 protocol
         *				7:S-IFSD to card
         *				8:
         *
         */
        unsigned char pin[ICBUF_MAX_LEN]; //���뻺����,USR-->KERNEL-->CARD
        unsigned char pout[ICBUF_MAX_LEN];//���������,USR<--KERNEL<--CARD
        int pin_len, pout_len;//��ݵ�ʵ�ʳ���
        int sent_len, rcv_len;//���ͽ��յ��ĳ��ȣ���ʱ����

		spinlock_t lock; //zy for lock 0527
        wait_queue_head_t wq;//wait queue, add by zhangy 0706

        char flag_snd_finished;//�Ƿ������
        char flag_timeout;//1,2,3
        char flag_removed;
        char flag_send;//0,1
        char flag_err;

        //ȫ�ֱ���, 080716
        unsigned char bSendByte;
//				unsigned char ucAPDUOneByteFlag;
        unsigned char uiAPDUSentLen, uiAPDUSendLen;
        unsigned char bFinished;
        unsigned char uiRecvedLen;//�ѽ��յ���IC���
        unsigned char uiRecvLen;//Ӧ���Ѿ���ȡ�����
	 u8 test;
    };

int cn_msg_sender( DRVOUT *icc_drvp, unsigned int forwoard);

inline unsigned char ic_read(unsigned long addr) 
{  
        udelay(100);      
	printk("");    
	return 0; 
}


inline unsigned char icc_read(unsigned long addr) 
{  
        udelay(100);      
	     
	return 0; 
}


#ifdef __cplusplus
}
#endif

#endif
//END

