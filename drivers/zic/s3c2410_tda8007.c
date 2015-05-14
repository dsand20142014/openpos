/*****************************************************************************
    Branched from SVN version 1650.
 *******************************************************************************/
#include <linux/config.h>
#include <linux/module.h>	//for min()
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/spinlock.h>	//for request_irq
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/wait.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/types.h>

#include <linux/platform_device.h>
#define HALICCDATA
#include <linux/jiffies.h>

#include "halicc.h" //for most lowlevel functions
#include "haldrv.h"
#include "libicc.h"
#include "osdriver.h"

#include "mach-mx25/board-mx25_3stack.h"
#include "mach-mx25/crm_regs.h"
#include "mach-mx25/iomux.h"

#include "s3c2410_tda8007.h"


#include <linux/types.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/fcntl.h>

#include <mach-mx25/machine.h>

wait_queue_head_t  icc_wait;

extern unsigned char icc_readb(unsigned long addr);

struct sock *nl_sk = NULL;
struct net init_net;
unsigned char show_flag = 1;
unsigned char remove_flag = 1;

struct tasklet_struct tasklet_icc;

static unsigned long tasklet_param_ptr =0x0;
static unsigned long tasklet_param_ptr2 =0x2;

struct completion comp;
EXPORT_SYMBOL(comp);


volatile u32 pid;

struct _data_from_user
{
    unsigned char *pSKB_PTR;
    char flag;
} data_from_user;

struct _data_to_user
{
    int skb_offset;
    int skb_len;
    unsigned char  *pSKB_PTR;
} data_to_user;

#define HAL_CODENOTIMPLEMENT         printk("s3c2410_tda8007.c %d\n",__LINE__);

unsigned long addrN[2][16];

#define MY_TEST //just for my simple test

char erro_flag = 0;
/*
 ʹ��mmap��ʽʵ�������Ӧ�ò�Ĺ���
 */
#define USE_MMAP
static int can_mmap_data_to_user;


wait_queue_head_t my_queue;

spinlock_t icclock=SPIN_LOCK_UNLOCKED;

void icccard_irq_set(void);

int icc_dbg = 0;
module_param(icc_dbg, int, 0);
EXPORT_SYMBOL(icc_dbg);

static int rwtime = 0;
module_param(rwtime, int, 0);
MODULE_PARM_DESC (rwreg, "read/write register times");


#define DEBUGGING	1
#if DEBUGGING
#define DPRINTK(n, args...)	do{ if (n <= DEBUGGING) printk(args); }while(0)
#else
#define DPRINTK(n, args...)
#endif

#define DBG(args...) printk(args)


#define NPAGES 1
// original pointer for kmalloc'd area as returned by kmalloc
static void *kmalloc_ptr;

static DRV          kDRV;
static DRVIN        gDRVIN;
static DRVOUT      *gDRVOUT;


#define PIN_RD S3C2410_GPC7	//GPIO E9Ϊ���������
#define PIN_RD_CFG S3C2410_GPC7_OUTP
//#define TDA8007_INT_GROUP0 IRQ_EINT17	//�ж���� GPG9
//#define TDA8007_INT_GROUP1 IRQ_EINT19 //�ж���� GPG11

static unsigned long TDA8007BASEADDR[MAX_CARDER_GROUP];
static unsigned int TDA8007IRQ[MAX_CARDER_GROUP];

struct ic_device *kICDEV;

volatile unsigned char ucICCPendFlag;
DRVIN ICCLastPendDrvIn;
volatile unsigned char ucICCIFDType;

//int icc_major =   242;
int icc_major =   0;
int icc_minor =   0;

struct icc_dev *iccptr;

unsigned char HALICC_WR_REG(unsigned char ucReader, unsigned char portno, unsigned char val);
unsigned char HALICC_RD_REG(unsigned char ucReader, unsigned char portno);
unsigned char HALICC_SetTimer(unsigned char ucReader,
                              unsigned char ucTimerType,
                              unsigned char ucTimerValue1,
                              unsigned char ucTimerValue2,
                              unsigned char ucTimerValue3,
                              unsigned char ucCheckFlag);
unsigned char TO_Test(void);

unsigned char HALICC_WR_REG(unsigned char ucReader, unsigned char addr, unsigned char val);
unsigned char HALICC_RD_REG(unsigned char ucReader, unsigned char addr);

static irqreturn_t s3c2410_ic_interrupt0(int irq, void *dev_id);
//static irqreturn_t s3c2410_ic_interrupt1(int irq, void *dev_id);
ssize_t s3c2410_ic_write(struct file *file, const char *buf, size_t count, loff_t *ppos);

#ifdef MY_TEST
TDA8007EVENT HSREvent[8] = {
    {HSREvent_PTLProc},
    {HSREvent_INTAUXLProc},
    {HSREvent_PRL1Proc},
    {HSREvent_PRL2Proc},
    {HSREvent_SUPLProc},
    {HSREvent_PRTL1Proc},
    {HSREvent_PRTL2Proc},
    {0},
};

TDA8007EVENT USREvent[8] = {
    {USREvent_TBERBFProc},
    {USREvent_FERProc},
    {USREvent_OVRProc},
    {USREvent_PEProc},
    {USREvent_EAProc},
    {USREvent_TO1Proc},
    {USREvent_TO2Proc},
    {USREvent_TO3Proc},
};
#endif

static unsigned char ic_readb(unsigned long addr)
{
    unsigned char val;
    val = (*(volatile unsigned char *)addr);
    return val;
}

static void ic_writeb(unsigned char data, unsigned long addr)
{
    *(volatile unsigned char *)addr = data;
}

/*���Ĵ���*/
#define REAL_TEST
inline unsigned char readreg(struct ic_device *dev, int portno, unsigned char ucCheckFlag)
{
    struct ic_info *cur = (struct ic_info *) & dev->cur_reader;
    unsigned char cur_reader_id = dev->cur_reader.reader;
    unsigned char ucCh = 0, ucChipIdent;
    unsigned long *pucChipAddr;
    volatile unsigned char ucREGCSR, ucREGMSR;
    int counts = 100;

    ucREGCSR = ucREGCSR;
    ucREGMSR = ucREGMSR;

    pucChipAddr = cur->addr;

    if (icc_dbg > 8)
        printk("cur_reader_id=%d addr=%x\n", cur_reader_id, ( int)pucChipAddr );

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);


    //����Ӧ��Ϊ 0010 / 0011
    ucChipIdent = (ucREGCSR & 0xF0) >> 4;
    if (ucChipIdent != 0x02 && ucChipIdent != 0x03) {
        //printk("ERR!:read ucREGCSR=%x ucChipIdent=%x\n", ucREGCSR, ucChipIdent);
        return 0;
    }
    if (icc_dbg > 8)
        printk("old ucREGCSR=%x\n", ucREGCSR);

    ucREGCSR &= 0xF8;
    ucREGCSR |= cur_reader_id;
    ic_writeb(ucREGCSR, pucChipAddr[REG_CSR]);


    /*********************************************************/
    do {
        ucREGMSR = ic_readb(pucChipAddr[REG_MSR]);
        if (ucREGMSR & 0x10)
            break;
    } while (counts--);
    if (counts == 0) printk("NOT CRED\n");
    /*********************************************************/


    ucCh = ic_readb(pucChipAddr[portno]);

    if (icc_dbg > 8)
        printk("read portno (%x) value=%x\n", portno, ucCh);

    return(ucCh);
}

inline void writereg(struct ic_device *dev, int portno, char value, char ucCheckFlag)
{
    struct ic_info *cur = (struct ic_info *) & dev->cur_reader;
    unsigned char cur_reader_id = dev->cur_reader.reader;

    volatile unsigned char ucREGCSR, ucREGPCR, ucREGMSR, ucChipIdent;
    unsigned long *pucChipAddr = cur->addr;
    int counts = 1000;


    if (icc_dbg > 8)
        printk("cur_reader_id=%d addr=%x\n", cur_reader_id, (unsigned int)pucChipAddr );

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);


    //����Ӧ��Ϊ 0010 / 0011
    ucChipIdent = (ucREGCSR & 0xF0) >> 4;
    if (ucChipIdent != 0x02 && ucChipIdent != 0x03) {
        return;
    }

    if (icc_dbg > 8)
        printk("_read old ucREGCSR=%x\n", ucREGCSR);


    if (  (ucREGCSR & 0x07) != cur_reader_id ) {
        // printk("to change ID, ucREGCSR & 0x07=%x cur_reader_id=%x\n", (ucREGCSR & 0x07), cur_reader_id);
        ucREGCSR &= 0xF8;
        ucREGCSR |= cur_reader_id;
        ic_writeb(ucREGCSR, pucChipAddr[REG_CSR]);
    }


    while (counts--) {
        ucREGPCR = ic_readb(pucChipAddr[REG_PCR]);

        if (icc_dbg > 8)
            printk("to check REG_PCR=%x \n", ucREGPCR);

        if ( ucREGPCR & 0x01 ) {
            ucREGMSR = ic_readb(pucChipAddr[REG_MSR]);
            if (icc_dbg > 8)
                printk("to check REG_MSR=%x\n", ucREGMSR);

            if ( ucREGMSR & 0x10 ) break;
        } else
            break;
    }

    ic_writeb(value, pucChipAddr[portno]);
}

int set_gpg11_output(void)
{
    return 0;
}
int set_gpg11_loop(int i)
{
    return 0;
}
int set_gpg11_h(void)
{
    return 0;
}
int set_gpg11_l(void)
{
    return 0;
}

//#define NOW_DBG
//ucReader: 0-3
unsigned char HALICC_TDA8007ReadREG(unsigned char ucReader,
                                    unsigned char ucREG,
                                    unsigned char *pucCh,
                                    unsigned char ucCheckFlag)
{
    //volatile unsigned char *pucChipAddr;
    unsigned long *pucChipAddr;
    unsigned char ucCh;
    unsigned char ucReaderID;
    unsigned char ucChipIdent;
    unsigned char ucREGCSR;
    unsigned char ucREGMSR;
    ICINFO *cur ;
    int counts=1000;

    switch (ucReader) {
        case HALICC_READER0:
        case HALICC_READER1:
            cur = (struct ic_info *)&kICDEV->ic_reader[0];
            ucReaderID = ucReader + 1;
            break;

        case HALICC_READER2:
        case HALICC_READER3:
            cur = (struct ic_info *)&kICDEV->ic_reader[2];
            ucReaderID = ucReader - 1;
            break;

        default:
            printk("ERR: %s Invalid reader %x\n", __func__, ucReader);
            return(HALICCERR_INVALIDREADER);
    }

    pucChipAddr = cur->addr;


    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    ucChipIdent = (ucREGCSR & 0xF0) >> 4;
    //  printk("%s!:read ucREGCSR=%x ucChipIdent=%x\n", __func__,ucREGCSR, ucChipIdent);
    if (  (ucChipIdent != TDA8007B_C2 ) && (ucChipIdent != TDA8007B_C3 )  ) {
        printk("ERR %s!:read ucREGCSR=%x ucChipIdent=%x\n", __func__,ucREGCSR, ucChipIdent);
        return(HALICCERR_CHIPNOTEXIST);
    }

    //      printk("ERR %s!:read ucREGCSR=%x ucChipIdent=%x\n", __func__,ucREGCSR, ucChipIdent);


    ucREGCSR &= 0xF8;
    ucREGCSR |= ucReaderID;

    ic_writeb(ucREGCSR, pucChipAddr[REG_CSR]);

    if ( ucCheckFlag ) {
        do {
            ucREGMSR = ic_readb(pucChipAddr[REG_MSR]);
            if (ucREGMSR & 0x10)
                break;
        } while (counts--);
        if (counts == 0) printk("NOT CRED\n");
    }

    ucCh = ic_readb(pucChipAddr[ucREG]);
    *pucCh = ucCh;

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_TDA8007WriteREG(unsigned char ucReader,
                                     unsigned char ucREG,
                                     unsigned char ucValue,
                                     unsigned char ucCheckFlag)
{
    unsigned long *pucChipAddr;
    unsigned char ucReaderID;
    unsigned char ucChipIdent;
    unsigned char ucREGCSR;
    unsigned char ucREGPCR;
    unsigned char ucREGMSR;
    struct ic_info *cur ;
    int counts=1000;



    switch (ucReader) {
        case HALICC_READER0:
        case HALICC_READER1:
            cur = (struct ic_info *)&kICDEV->ic_reader[0];
            ucReaderID = ucReader + 1;
            break;

        case HALICC_READER2:
        case HALICC_READER3:
            cur = (struct ic_info *)&kICDEV->ic_reader[2];
            ucReaderID = ucReader - 1;
            break;

        default:
            printk("ERR: %s Invalid reader %x\n", __func__, ucReader);
            return(HALICCERR_INVALIDREADER);
    }

    pucChipAddr = cur->addr;


    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);

    //����Ӧ��Ϊ 0010 / 0011
    ucChipIdent = (ucREGCSR & 0xF0) >> 4;
    if (ucChipIdent != 0x02 && ucChipIdent != 0x03) {
        printk("ERR %s!:read ucREGCSR=%x ucChipIdent=%x, NOT 8007 CHIP???\n", __func__, ucREGCSR, ucChipIdent);
        return (HALICCERR_CHIPNOTEXIST);
    }

    if (  (ucREGCSR & 0x07) != ucReaderID ) {
        ucREGCSR &= 0xF8;
        ucREGCSR |= ucReaderID;
        ic_writeb(ucREGCSR, pucChipAddr[REG_CSR]);
    }

    if ( ucCheckFlag ) {
        while (counts--) {
            ucREGPCR = ic_readb(pucChipAddr[REG_PCR]);
            if ( ucREGPCR & 0x01 ) {
                ucREGMSR = ic_readb(pucChipAddr[REG_MSR]);
                if ( ucREGMSR & 0x10 )
                    break;
            } else
                break;
        }
    }


    ic_writeb(ucValue, pucChipAddr[ucREG]);



    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_TDA8007WriteREG1(unsigned char ucReader,
                                      unsigned char ucREG,
                                      unsigned char ucValue,
                                      unsigned char ucCheckFlag)
{
    //        unsigned long *pucChipAddr;
    ulong *pucChipAddr;
    unsigned char ucReaderID;
    unsigned char ucChipIdent;
    unsigned char ucREGCSR;
    unsigned char ucREGPCR;
    unsigned char ucREGMSR;
    struct ic_info *cur ;
    int counts=1000;



    switch (ucReader) {
        case HALICC_READER0:
        case HALICC_READER1:
            cur = (struct ic_info *)&kICDEV->ic_reader[0];
            ucReaderID = ucReader + 1;
            break;

        case HALICC_READER2:
        case HALICC_READER3:
            cur = (struct ic_info *)&kICDEV->ic_reader[2];
            ucReaderID = ucReader - 1;
            break;

        default:
            printk("%s Invalid reader %x\n", __func__, ucReader);
            return(HALICCERR_INVALIDREADER);
    }

    pucChipAddr = cur->addr;

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    //����Ӧ��Ϊ 0010 / 0011
    ucChipIdent = (ucREGCSR & 0xF0) >> 4;
    if (ucChipIdent != 0x02 && ucChipIdent != 0x03) {
        printk("ERR %s!:read ucREGCSR=%x ucChipIdent=%x, NOT 8007 CHIP???\n", __func__, ucREGCSR, ucChipIdent);
        return (HALICCERR_CHIPNOTEXIST);
    }
    if (  (ucREGCSR & 0x07) != ucReaderID ) {
        ucREGCSR &= 0xF8;
        ucREGCSR |= ucReaderID;
        ic_writeb(ucREGCSR, pucChipAddr[REG_CSR]);
    }
    if ( ucCheckFlag ) {
        while (counts--) {

            ucREGPCR = ic_readb(pucChipAddr[REG_PCR]);
            if ( ucREGPCR & 0x01 ) {
                ucREGMSR = ic_readb(pucChipAddr[REG_MSR]);
                if ( ucREGMSR & 0x10 )
                    break;
            } else
                break;
        }
    }

    set_gpg11_loop(1);
    printk("%s ucValue=%x, addr=%x, ucREG=%x\n", __func__, (int )ucValue, (int )pucChipAddr[ucREG], (int )ucREG);
    ic_writeb(ucValue, pucChipAddr[ucREG]);
    set_gpg11_loop(3);


    return(HALICCERR_SUCCESS);
}

unsigned char TO_Test(void)
{
    volatile unsigned char ucREGUSR = 0, ucREGVAL = 0;
    ucREGUSR = ucREGVAL = 0;


    return 0;
}

#ifdef MY_TEST
/*ucChipID ����ָʾ��0��1оƬ*/
void HSREvent_PTLProc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    ICINFO *cur ;

    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;

    if (icc_dbg)
        printk("%s:%d ucChipID=%d ucReader=%d\n", __func__,__LINE__, ucChipID, ucReader);

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;

    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;
    if ( pfnCallbackFault ) {
        (*pfnCallbackFault)(ucReader);
    }
}

void HSREvent_INTAUXLProc(unsigned char ucChipID)
{
    HAL_CODENOTIMPLEMENT
}

void HSREvent_PRL1Proc(unsigned char ucChipID)
{
    unsigned char ucREGMSR,ucREGHSR;
    unsigned char ucReader, ret;
    ICINFO *cur ;

    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    ucReader = cur->reader;

    erro_flag = 2;

    if (icc_dbg)
        printk("%s:%d ucChipID=%d ucReader=%d\n", __func__,__LINE__, ucChipID, ucReader);
    udelay(300);
    HALICC_TDA8007ReadREG(ucReader, REG_HSR, &ucREGHSR, 0);


    if ( !(ret=HALICC_TDA8007ReadREG(ucReader, REG_MSR, &ucREGMSR, 0) )) {
        if (icc_dbg>=3)
            printk("%s ucChipID=%d ucReader=%d, MSR=%x\n", __func__, ucChipID, ucReader, ucREGMSR);

        if ( ucREGMSR & 0x04 ) {
            /*
             EN: Card insert
             */
            remove_flag = 1;
            //icccard_irq_set();//ZYADD 0510
            if ( pfnCallbackCardInsert ) {
                (*pfnCallbackCardInsert)(ucReader);

            }
        } else {
            /*
             EN: Card remove
             */
            remove_flag = 0;
            if ( pfnCallbackCardRemove ) {
                (*pfnCallbackCardRemove)(ucReader);
                //    icccard_irq_set();//ZYADD

            }
        }
    }
}

void HSREvent_PRL2Proc(unsigned char ucChipID)
{
    unsigned char ucREGMSR;
    unsigned char ucReader;
    ICINFO *cur ;

    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    ucReader = cur->reader+HALICC_READER1;
    if (icc_dbg)
        printk("%s:%d ucChipID=%d ucReader=%d\n", __func__,__LINE__, ucChipID, ucReader);

    if (icc_dbg>=3)
        printk("%s ucChipID=%d ucReader=%d\n", __func__, ucChipID, ucReader);

    if ( !HALICC_TDA8007ReadREG(ucReader, REG_MSR, &ucREGMSR, 0) ) {
        if ( ucREGMSR & 0x04 ) {
            /*
             EN: Card insert
             */
            if ( pfnCallbackCardInsert ) {
                (*pfnCallbackCardInsert)(ucReader);

            }
        } else {
            /*
             EN: Card remove
             */
            if ( pfnCallbackCardRemove ) {
                (*pfnCallbackCardRemove)(ucReader);
                icccard_irq_set();//ZYADD

            }
        }
    }
}

void HSREvent_SUPLProc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    ICINFO *cur ;

    if (icc_dbg)
        printk("%s:%d, REGHSR val=%x\n", __func__, __LINE__, kICDEV->test);


    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;

    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;
    if ( pfnCallbackFault ) {
        (*pfnCallbackFault)(ucReader);
    }
}

void HSREvent_PRTL1Proc(unsigned char ucChipID)
{
    unsigned char ucReader;


    switch (ucChipID) {
        case 0:
            ucReader = HALICC_READER0;
            break;
        case 1:
            ucReader = HALICC_READER2;
            break;
        default:
            printk("%s:%d, NOT right ucChipID(%x)\n", __func__, __LINE__, ucChipID);
            return;
    }
    printk("%s ucChipID=%d ucReader=%d, todo callbackfault!\n", __func__, ucChipID, ucReader);

    if ( pfnCallbackFault ) {
        (*pfnCallbackFault)(ucReader);
    }

}

void HSREvent_PRTL2Proc(unsigned char ucChipID)
{
    unsigned char ucReader;



    switch (ucChipID) {
        case 0:
            ucReader = HALICC_READER1;
            break;
        case 1:
            ucReader = HALICC_READER3;
            break;
        default:
            printk("%s:%d, NOT right ucChipID(%x)\n", __func__, __LINE__, ucChipID);
            return;
    }
    printk("%s ucChipID=%d ucReader=%d, todo callbackfault!\n", __func__, ucChipID, ucReader);
    if ( pfnCallbackFault ) {
        (*pfnCallbackFault)(ucReader);
    }
}

void USREvent_TBERBFProc(unsigned char ucChipID)
{
    unsigned char ucREGCSR;
    unsigned char ucREGUCR1 = 0;
    unsigned char ucREGMSR = 0;
    unsigned char ucCh;
    unsigned long *pucChipAddr;
    unsigned char ucReader;
    ICINFO *cur ;

    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;

    ucREGCSR= ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;
    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;

    HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
    //  printk("TBERBF ucREGUCR1:0x%x\n ", ucREGUCR1);
    if ( !(ucREGUCR1 & 0x08) ) {  /* Receive */
        HALICC_TDA8007ReadREG(ucReader, REG_MSR, &ucREGMSR, 0);
        //  printk("TBERBF ucREGMSR:0x%x\n ", ucREGMSR);
        if ( !(ucREGMSR & 0x40) ) {
            if ( !HALICC_RecvChar(ucReader, &ucCh)) {
                //   printk("[%s,%d] Rev: %x\n",__func__,__LINE__, ucCh);
                if ( pfnCallbackRecvChar ) {
                    (*pfnCallbackRecvChar)(ucReader, ucCh);
                }
            }
        }
    } else {  /* Transmit */
        HALICC_TDA8007ReadREG(ucReader, REG_MSR, &ucREGMSR, 0);
        if ( ucREGMSR & 0x01 ) {
            if ( pfnCallbackSendChar ) {
                (*pfnCallbackSendChar)(ucReader);
            }
        }
    }
}

void USREvent_FERProc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader ;
    ICINFO *cur ;



    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;


    ucREGCSR =  ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;
    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;

    if ( pfnCallbackFrameError ) {
        (*pfnCallbackFrameError)(ucReader);
    }
}

void USREvent_OVRProc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    ICINFO *cur ;


    cur = (struct ic_info *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;



    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;
    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;

    if ( pfnCallbackOverrunError ) {
        (*pfnCallbackOverrunError)(ucReader);
    }
}

void USREvent_PEProc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    ICINFO *cur ;


    cur = (ICINFO *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;

    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;

    if ( pfnCallbackParityError ) {
        (*pfnCallbackParityError)(ucReader);
    }
}

void USREvent_EAProc(unsigned char ucChipID)
{
    HAL_CODENOTIMPLEMENT
        HALICC_Deactive(0);

}

void USREvent_TO1Proc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    struct ic_info *cur ;
    cur = (struct ic_info *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;
    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;

    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;

    if ( pfnCallbackTimeout ) {
        (*pfnCallbackTimeout)(ucReader, 1);
    }
}

void USREvent_TO2Proc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    struct ic_info *cur ;

    cur = (struct ic_info *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;


    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;

    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07) - 1;

    if ( pfnCallbackTimeout ) {
        (*pfnCallbackTimeout)(ucReader, 2);
    }
}

void USREvent_TO3Proc(unsigned char ucChipID)
{
    unsigned long *pucChipAddr;
    unsigned char ucREGCSR;
    unsigned char ucReader;
    struct ic_info *cur ;

    cur = (struct ic_info *)&kICDEV->ic_reader[ucChipID*2];
    pucChipAddr = cur->addr;
    ucReader = cur->reader;

    ucREGCSR = ic_readb(pucChipAddr[REG_CSR]);
    if (icc_dbg > 3)     printk("%s ucREGCSR = %x, ucReader=%d\n", __func__, ucREGCSR, ucReader);
    if (  ((ucREGCSR & 0x07) != 0x01)
        && ((ucREGCSR & 0x07) != 0x02)
        )
        return;

    ucReader = 2*ucChipID+ucReader + (ucREGCSR & 0x07)- 1;

    if ( pfnCallbackTimeout ) {
        (*pfnCallbackTimeout)(ucReader, 3);
    }
}

ICCDRVCFG ICCDRVCfg = {
    0x6A,
    0
};

HALDRV HALICCDrv = {
    HALDRVID_ICC,
    &gDRVIN,
    0,
    HALICC_Install,
    HALICC_Uninstall,
    HALICC_Test,
    HALICC_Ioctl,
    HALICC_State,
    HALICC_Run,
    HALICC_Abort,
    HALICC_Reset,
    HALICC_RecvHook,
};


unsigned char HALICC_DRVAbort(void)
{
    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_DRVEnd(unsigned char ucDrvStatus, unsigned char *pucInData, unsigned short uiInLen)
{
    DRVOUT *pDrvOut;


    if (1) {
        pDrvOut = HALICCDrv.pDrvOut;
        if ( pDrvOut ) {
            if ((pucInData) && (uiInLen)) {
                memcpy(&pDrvOut->aucData[0], pucInData, uiInLen);
            }


            pDrvOut->ucDrvStatus = ucDrvStatus;
            ucICCPendFlag = 0;
            pDrvOut->ucGenStatus = HALDRVSTATE_ENDED;


            //    if (iccptr->gMMAP_IC_Drv->pDrvIn.ucType == 0x7)
            {

                wake_up_interruptible(&icc_wait);
                printk("wake up  pDrvOut->ucGenStatus:0x%x  iccptr->card_status:0x%x!!!\r\n",pDrvOut->ucGenStatus,
                       iccptr->card_status);
            }

#if 1
            if (remove_flag)
            {
                if (iccptr->fasync_queue)
                {
                    kill_fasync(&iccptr->fasync_queue, ICC_SIG, POLL_IN);

                }

                if (iccptr->card_status == CARD_EJECTED)
                {
                    iccptr->card_status = CARD_INSERTED;

                }
                remove_flag  = 0;
            }
#endif
#if 0
            int i;
            printk("\n*******%s ************\n", __func__);
            for (i = 0; i <= HALICCDrv.pDrvOut->aucData[0]; i++)
                printk("%02x ", HALICCDrv.pDrvOut->aucData[i]);
            printk("\n");

            for (i = 0; i <= gMMAP_IC_Drv->pDrvOut.aucData[0]; i++)
                printk("%02x ", gMMAP_IC_Drv->pDrvOut.aucData[i]);
            printk("\n*******%s END ************\n", __func__);
#endif

        } else {
            printk("!!! HALICC DRV NULL, Need to copy data to userspace!!!\n");
        }
    } else {
        printk("!!!!!!!!!!!!!!!!!!! %s please run app and then insert card, or I will not gve you any data\n", __func__);
    }



    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Install(void)
{
    return(HALICC_Init());
}

unsigned char HALICC_Uninstall(void)
{

    //	HALICC_Disable();

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Init(void)
{
    void HALICC_Chip1IntProc(void);
    void HALICC_Chip2IntProc(void);
    char tmp;

    if (icc_dbg) printk("this is %s:%d\n", __func__, __LINE__);


    ucICCPendFlag = 0;
    memset(&ICCLastPendDrvIn, 0, sizeof(DRVIN));
    ucICCIFDType = HALICCTYPE_TDA8007;


    ucLastREGHSR = 0x00;
    ucLastREGUSR = 0x00;
    ucChipSELECT = 0x00;

    pfnHALICCCommand = 0;

    pfnCallbackCardInsert = 0;
    pfnCallbackCardRemove = 0;
    pfnCallbackSendChar = 0;
    pfnCallbackRecvChar = 0;
    pfnCallbackFault = 0;
    pfnCallbackTimeout = 0;
    pfnCallbackParityError = 0;
    pfnCallbackOverrunError = 0;
    pfnCallbackFrameError = 0;


    LIBICC_Init();

    HALICC_TDA8007ReadREG(0, REG_HSR, &tmp, 1);
    //printk("%s:%d Read HSR during poweron: %x\n", tmp);

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Test(void)
{
    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Ioctl(unsigned char ucType, unsigned char *pucData)
{
    switch (ucType) {
        case HALDRVCFG_GET:
            if ( pucData ) {
                memcpy(	&ICCDRVCfg, pucData, sizeof(ICCDRVCfg));
            }
            break;
        case HALDRVCFG_SET:
            if ( pucData ) {
                memcpy( pucData, &ICCDRVCfg, sizeof(ICCDRVCfg));
            }
            break;
    }
    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_State(DRVIN *pDrvIn, DRVOUT *pDrvOut)
{
    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Run(DRVIN *pDrvIn, DRVOUT *pDrvOut)
{
    unsigned char ucResult;
    unsigned short uiLen;

    // printk(" GGGGGGGG %s, DRVIN ucType=%x, len=%x\n", __func__, pDrvIn->ucType, pDrvIn->ucLen);

    {
        if ( pfnHALICCCommand ) {
            ucICCPendFlag = 1;
            HALICCDrv.pDrvOut = pDrvOut;

            /*zy:
             *	���Ȳ�ͬ���򴫵ݵ���ݸ�ʽ�б�
             * 0xffʱ��ʵ�ʳ���Ϊ[0]*256+[1]
             * !0xff,ʵ�ʳ���ΪucLen
             */
            if ( pDrvIn->ucLen == 0xFF ) {
                uiLen = pDrvIn->aucData[0] * 256 + pDrvIn->aucData[1];
                ucResult = (*pfnHALICCCommand)(pDrvIn->ucType, &pDrvIn->aucData[2], uiLen,
                                               pDrvOut->aucData);
            } else {
                ucResult = (*pfnHALICCCommand)(pDrvIn->ucType, pDrvIn->aucData, pDrvIn->ucLen,
                                               pDrvOut->aucData);
            }

            if ( ucResult != LIBICCERR_PROCESSPENDING ) {
                ucICCPendFlag = 0;
                //                HALICCDrv.pDrvOut = 0; //zyadd 0804
                pDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
                pDrvOut->ucDrvStatus = ucResult;
            } else { //I am process pending
                pDrvOut->ucGenStatus = HALDRVSTATE_RUNNING;
                pDrvOut->ucDrvStatus = HALDRVSTATE_RUNNING;
            }
            //            printk("********* %s ucResult=%x, pDrvOut->aucData[0-1]=%02x %02x\n", __func__, ucResult, pDrvOut->aucData[0], pDrvOut->aucData[1]);

            return(HALICCERR_SUCCESS);
        } else {
            printk("!!!! NOT implement! %s %d\n", __func__, __LINE__);
        }
        return(HALDRVERR_CMDNOTSUPPORT);
    }
}

unsigned char HALICC_Abort(DRVIN *pDrvIn, DRVOUT *pDrvOut)
{
    {
        printk("[%s,%d]\r\n", __func__,__LINE__);
        pDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
        pDrvOut->ucDrvStatus = HALDRVSTATE_ENDED;

        wake_up_interruptible(&icc_wait);

        return(HALICCERR_SUCCESS);
    }
}

unsigned char HALICC_Enable(void)
{
    unsigned char ucResult =0;

    return( ucResult );
}

void HALICC_Disable(void)
{

}

unsigned char HALICC_Reset(DRVIN *pDrvIn, DRVOUT *pDrvOut)
{
    unsigned char ucResult;
    ucResult = 0;
    printk("[%s,%d]\r\n", __func__, __LINE__);

    {
        pDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
        pDrvOut->ucDrvStatus = HALDRVSTATE_ENDED;

        return(HALICCERR_SUCCESS);
    }
}

unsigned char HALICC_RecvHook(unsigned char ucDevID, unsigned char *pucRecvData, unsigned int uiRecvLen)
{
    unsigned char *pucPtr;
    DRVOUT *pDrvOut;

    pucPtr = pucRecvData;
    //EN: Skip DrvID
    pucPtr ++;
    pDrvOut = HALICCDrv.pDrvOut;
    if ( pDrvOut ) {
        pDrvOut->ucReqNR = *pucPtr;
        pucPtr ++;
        pDrvOut->ucGenStatus = HALDRVSTATE_ENDED;
        pDrvOut->ucDrvStatus = *pucPtr;
        pucPtr ++;
        switch ( ICCLastPendDrvIn.ucType) {
            case 0x10:  //SYNAPP
                pDrvOut->aucData[0] = *pucPtr - 2;
            pucPtr ++;
            pDrvOut->aucData[1] = *pucPtr;
            pucPtr ++;
            if ( uiRecvLen >= 5 ) {
                memcpy(&(pDrvOut->aucData[2]), pucPtr + 2, uiRecvLen - 5);
            }
            //FIXME
            printk("%s pDrvOut->aucData[0] =%x [1]=%x\n",
                   __func__, pDrvOut->aucData[0] ,pDrvOut->aucData[1] );
            break;
            default:
                if ( uiRecvLen >= 3) {
                    memcpy(pDrvOut->aucData, pucPtr, uiRecvLen - 3);
                }
                break;
        }

        printk("[%s,%d]\r\n",__func__, __LINE__ );
        ucICCPendFlag = 0;
    } else {
        printk("NULL RETURN PTR\n");
    }
    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_HandleContact(unsigned char ucReader,
                                   unsigned char ucType,
                                   unsigned char ucID,
                                   unsigned char *pucValue)
{
    unsigned char ucResult;
    unsigned char ucREGUCR1;
    unsigned char ucREGPCR;
    unsigned char ucREGCCR;
    unsigned char ucREGUTR;
    unsigned char ucREGURR = 0;
    unsigned char ucValue=0;
    ucResult = 0;

    if ( !ucResult ) {
        switch (ucID) {
            case HALICCCONTACT_RST:
                ucResult = HALICC_TDA8007ReadREG(ucReader, REG_PCR, &ucREGPCR, 0);
                if ( !ucResult ) {
                    if ( ucType == HALICCCONTACT_CLR ) {
                        ucREGPCR &= 0xFB;
                        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                    } else {
                        if ( ucType == HALICCCONTACT_SET ) {
                            ucREGPCR |= 0x04;//PS100
                            // ucREGPCR |= 0x05;	//zy TODO Pay attention to this!!!
                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                            udelay(10);
                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                            udelay(10);
                        } else {
                            ucValue = ucREGPCR & 0x04;
                        }
                    }
                }
                break;


            case HALICCCONTACT_CLK:
                ucResult = HALICC_TDA8007ReadREG(ucReader, REG_CCR, &ucREGCCR, 0);
                if ( !ucResult ) {
                    if ( ucType == HALICCCONTACT_CLR ) {
                        ucREGCCR &= 0xF7;
                        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_CCR, ucREGCCR, 0);
                    } else {
                        if ( ucType == HALICCCONTACT_SET ) {
                            ucREGCCR |= 0x08;
                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_CCR, ucREGCCR, 0);
                        } else {
                            ucValue = ucREGCCR & 0x08;
                        }
                    }
                }
                break;

            case HALICCCONTACT_IO:
                if ( ucType == HALICCCONTACT_CLR ) {
                    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
                    if (  (!ucResult )
                        && ( !(ucREGUCR1 & 0x08) )
                        ) {

                            ucREGUCR1 |= 0x08;

                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);

                        }
                    if ( !ucResult ) {
                        ucREGUTR = 0x00;
                        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UTR, ucREGUTR, 0);
                    }
                } else {
                    if ( ucType == HALICCCONTACT_SET ) {
                        ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
                        if (  (!ucResult )
                            && ( !(ucREGUCR1 & 0x08) )
                            ) {
                                ucREGUCR1 |= 0x08;
                                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
                            }
                        if ( !ucResult ) {
                            ucREGUTR = 0x01;
                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UTR, ucREGUTR, 0);
                        }
                    } else { //HALICCCONTACT_GET
                        ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
                        if (  (!ucResult )
                            && ( ucREGUCR1 & 0x08 )
                            ) {
                                ucREGUCR1 &= 0xF7;
                                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
                            }
                        if ( !ucResult ) {
                            ucResult = HALICC_TDA8007ReadREG(ucReader, REG_URR, &ucREGURR, 0);
                        }
                        if ( !ucResult ) {
                            ucValue = ucREGURR & 0x01;
                        }
                    }
                }
                break;

            case HALICCCONTACT_C4:
                ucResult = HALICC_TDA8007ReadREG(ucReader, REG_PCR, &ucREGPCR, 0);
                if ( !ucResult ) {
                    if ( ucType == HALICCCONTACT_CLR ) {
                        ucREGPCR &= 0xEF;
                        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                    } else {
                        if ( ucType == HALICCCONTACT_SET ) {
                            ucREGPCR |= 0x10;
                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                        } else {
                            ucValue = ucREGPCR & 0x10;
                        }
                    }
                }
                break;
            case HALICCCONTACT_C8:
                ucResult = HALICC_TDA8007ReadREG(ucReader, REG_PCR, &ucREGPCR, 0);
                if ( !ucResult ) {
                    if ( ucType == HALICCCONTACT_CLR ) {
                        ucREGPCR &= 0xDF;
                        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                    } else {
                        if ( ucType == HALICCCONTACT_SET ) {
                            ucREGPCR |= 0x20;
                            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);
                        } else {
                            ucValue = ucREGPCR & 0x20;
                        }
                    }
                }
                break;

            default:
                return(HALICCERR_INVALIDCONTACT);
        }
    }

    if ( !ucResult ) {
        if ( ucType == HALICCCONTACT_GET ) {
            if ( pucValue ) {
                *pucValue = ucValue;
            }
        }
    }
    return(ucResult);
}
EXPORT_SYMBOL(HALICC_HandleContact);

unsigned char HALICC_SelectReader(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucReaderID;
    unsigned char ucChipIdent;
    unsigned char ucREGCSR;
    struct ic_info *cur ;


    /*ѡ�񿨺ż���ֵַ*/
    cur = (struct ic_info *)&kICDEV->ic_reader[ucReader];
    ucReaderID = cur->reader;


    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_CSR, &ucREGCSR, 0);
    if ( ucResult )
        return(ucResult);

    ucChipIdent = (ucREGCSR & 0xF0) >> 4;
    if (  (ucChipIdent != TDA8007B_C2 )
        && (ucChipIdent != TDA8007B_C3 )
        ) {
            return(HALICCERR_CHIPNOTEXIST);
        }

    ucREGCSR &= 0xF8;
    ucREGCSR |= ucReaderID;
    HALICC_TDA8007WriteREG(ucReader, REG_CSR, ucREGCSR, 0);
    return(HALICCERR_SUCCESS);
}

//0824
unsigned char HALICC_SetMethod(unsigned char ucReader, unsigned char ucMethod)
{
    unsigned char ucResult;
    unsigned char ucREGUCR2;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR2, &ucREGUCR2, 0);
    if ( !ucResult ) {
        if ( !ucMethod ) {
            ucREGUCR2 &= 0xF7;
        } else {
            ucREGUCR2 |= 0x48;
        }
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR2, ucREGUCR2, 0);
    }

    return(ucResult);
}

unsigned char HALICC_GetMethod(unsigned char ucReader, unsigned char *pucMethod)
{
    unsigned char ucResult;
    unsigned char ucREGUCR2;
    unsigned char ucMethod;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR2, &ucREGUCR2, 0);
    if ( !ucResult ) {
        if ( ucREGUCR2 & 0x40 ) {
            ucMethod = 1;
        } else {
            ucMethod = 0;
        }
        if ( pucMethod ) {
            *pucMethod = ucMethod;
        }
    }
    return(ucResult);
}

unsigned char HALICC_SetClock(unsigned char ucReader,
                              unsigned char ucStopFlag, unsigned char ucLevel)
{
    unsigned char ucResult;
    unsigned char ucREGCCR;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_CCR, &ucREGCCR, 0);

    if ( !ucResult ) {
        if ( ucStopFlag ) {
            ucREGCCR |= 0x10;
        } else {
            ucREGCCR &= 0xEF;
        }
        if ( ucLevel ) {
            ucREGCCR |= 0x20;
        } else {
            ucREGCCR &= 0xDF;
        }
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_CCR, ucREGCCR, 0);
    }
    return(ucResult);
}

unsigned char HALICC_Detect(unsigned char ucReader, unsigned char *pucPresent)
{
    unsigned char ucResult;
    unsigned char ucREGMSR;
    unsigned char ucPresent = 0;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_MSR, &ucREGMSR, 0);

    if ( !ucResult ) {
        switch (ucReader) {
            case HALICC_READER0:
            case HALICC_READER2:
                ucPresent = ucREGMSR & 0x04;
                break;
            case HALICC_READER1:
            case HALICC_READER3:
                ucPresent = ucREGMSR & 0x08;
                break;
            default:
                return(HALICCERR_INVALIDREADER);
        }

        if ( pucPresent ) {
            *pucPresent = ucPresent;
        }
    }
    return(ucResult);
}


unsigned char HALICC_ResetUart(unsigned char ucReader)
{
    unsigned char ucREGCSR = 0;

    HALICC_TDA8007ReadREG(ucReader, REG_CSR, &ucREGCSR, 0);
    ucREGCSR &= 0xF7;
    HALICC_TDA8007WriteREG(ucReader, REG_CSR, ucREGCSR, 0);
    ucREGCSR |= 0x08;
    HALICC_TDA8007WriteREG(ucReader, REG_CSR, ucREGCSR, 0);


    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Active(unsigned char ucReader)
{
    unsigned char ucREGPCR = 0;

    HALICC_TDA8007ReadREG(ucReader, REG_PCR, &ucREGPCR, 0);
    ucREGPCR &= 0xFB;
    ucREGPCR |= 0x01;
    HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_Deactive(unsigned char ucReader)
{
#if 1
    unsigned char ucREGCSR = 0;
    //   printk("[%s,%d\n]",__func__, __LINE__);

    HALICC_TDA8007WriteREG(ucReader, REG_PCR, 0x00, 0);
    HALICC_TDA8007ReadREG(ucReader, REG_CSR, &ucREGCSR, 0);
    ucREGCSR &= 0xF7;

    ic_readb(addrN[0][15]);
    ndelay(2);
    ic_readb(addrN[1][15]);
    show_flag = 1;

    //  tasklet_kill(&tasklet_icc);
    //  tasklet_disable(&tasklet_icc);

    return(HALICC_TDA8007WriteREG(ucReader, REG_CSR, ucREGCSR, 0));
#else
    return(HALICC_TDA8007WriteREG(ucReader, REG_PCR, 0x00, 0));
#endif
}

unsigned char HALICC_SetPowerType(unsigned char ucReader,
                                  unsigned char ucPowerType)
{
    unsigned char ucREGPCR = 0;

    HALICC_TDA8007ReadREG(ucReader, REG_PCR, &ucREGPCR, 0);
    ucREGPCR &= 0xF5;
    switch (ucPowerType) {
        case HALICC_POWER3V:
            ucREGPCR |= 0x02;
            break;
        case HALICC_POWER18V:
            ucREGPCR |= 0x08;
            break;
        case HALICC_POWER5V:
        default:
            break;
    }
    HALICC_TDA8007WriteREG(ucReader, REG_PCR, ucREGPCR, 0);

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_SetBaud(unsigned char ucReader,
                             unsigned char ucBaudMulti)
{
    unsigned char ucREGCCR = 0;
    HALICC_TDA8007ReadREG(ucReader, REG_CCR, &ucREGCCR, 0);
    switch ( ucBaudMulti ) {
        case 2:
            ucREGCCR = (ucREGCCR & 0xF8) + 0x01;
            break;
        case 4:
            ucREGCCR = ucREGCCR & 0xF8;
            break;
        case 1:
        default:
            //      ucREGCCR = (ucREGCCR & 0xF8) + 0x02;
            ucREGCCR = (ucREGCCR & 0xF8) + 0x03;
            break;
    }
    HALICC_TDA8007WriteREG(ucReader, REG_CCR, ucREGCCR, 0);
    HALICC_TDA8007WriteREG(ucReader, REG_UCR2, 0x00, 0);

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_SetDi(unsigned char ucReader,
                           unsigned char ucDi)
{
    unsigned char ucREGPDR;

    switch (ucDi) {
        case 2:
            ucREGPDR = 0x06;
            break;
        case 4:
            ucREGPDR = 0x03;
            break;
        case 8:
            ucREGPDR = 0x01;
            break;
        case 1:
        default:
            ucREGPDR = 0x0C;
            break;
    }
    HALICC_TDA8007WriteREG(ucReader, REG_PDR, ucREGPDR, 0);

    return(HALICCERR_SUCCESS);
}

unsigned char HALICC_SetProtocol(unsigned char ucReader,
                                 unsigned char ucProtocol)
{
    unsigned char ucResult;
    unsigned char ucREGUCR1;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
    if ( !ucResult ) {
        if ( ucProtocol ) {
            ucREGUCR1 |= 0x10;
        } else {
            ucREGUCR1 &= 0xEF;
        }
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
    }
    return(ucResult);
}

unsigned char HALICC_SetConvention(unsigned char ucReader,
                                   unsigned char ucConvention)
{
    unsigned char ucResult;
    unsigned char ucREGUCR1;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
    if ( !ucResult ) {
        if ( ucConvention ) {
            ucREGUCR1 |= 0x02;
        } else {
            ucREGUCR1 &= 0xFD;
        }
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
    }
    return(ucResult);
}

unsigned char HALICC_SetPEC(unsigned char ucReader,
                            unsigned char ucPEC)
{
    ucPEC = (ucPEC & 0x07) << 4;
    return(HALICC_TDA8007WriteREG(ucReader, REG_FCR, ucPEC, 0));
}

/*
 EN: This function must be called in interrupt routine!!
 */
unsigned char HALICC_EnableSend(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucREGUCR1;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
    if ( !ucResult ) {
        ucREGUCR1 |= 0x08;
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
    }
    return(ucResult);
}

unsigned char HALICC_EnableRecv(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucREGUCR1;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
    if ( !ucResult ) {
        ucREGUCR1 &= 0xF7;
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
    }
    return(ucResult);
}

unsigned char HALICC_EnableRecvAfterSend(unsigned char ucReader)
{
    unsigned char ucResult;
    unsigned char ucREGUCR1;

    ucResult = HALICC_TDA8007ReadREG(ucReader, REG_UCR1, &ucREGUCR1, 0);
    if ( !ucResult ) {
        ucREGUCR1 |= 0x04;
        ucResult = HALICC_TDA8007WriteREG(ucReader, REG_UCR1, ucREGUCR1, 0);
    }
    return(ucResult);
}

unsigned char HALICC_SendChar(unsigned char ucReader,
                              unsigned char ucCh)
{
    //printk("[%02x] ", ucCh);
    unsigned char ret = HALICC_TDA8007WriteREG(ucReader, REG_UTR, ucCh, 1);
    return ret;
}

unsigned char HALICC_RecvChar(unsigned char ucReader,
                              unsigned char *pucCh)
{
    unsigned char  ret = HALICC_TDA8007ReadREG(ucReader, REG_URR, pucCh, 1);
    //printk("(%02x) ", *pucCh);
    return ret;
}

unsigned char HALICC_SetTimer(unsigned char ucReader,
                              unsigned char ucTimerType,
                              unsigned char ucTimerValue1,
                              unsigned char ucTimerValue2,
                              unsigned char ucTimerValue3,
                              unsigned char ucCheckFlag)

{
    unsigned char ucResult;
    unsigned char ucTOC;

    // u8 tmp;
    // HALICC_TDA8007ReadREG(ucReader, REG_CSR, &tmp, 0);
    // printk("%s:%d, CSR val=%x, RIT=%x\n",__func__, __LINE__, tmp, (tmp & 0x08)>>3 );

    switch (ucTimerType) {
        case 0x00:
            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOC, 0x00, ucCheckFlag);
            break;
        case 0x05:
            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOC, ucTimerType, ucCheckFlag);
            break;
        case 0x65:
        case 0xE5:
            ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOR1, ucTimerValue1, ucCheckFlag);
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOR2, ucTimerValue2, ucCheckFlag);
            }
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOR3, ucTimerValue3, ucCheckFlag);
            }
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOC, ucTimerType, ucCheckFlag);
            }
            break;
        default:
            ucResult = HALICC_TDA8007ReadREG(ucReader, REG_TOC, &ucTOC, ucCheckFlag);
            if ( !ucResult ) {
                if ( ucTOC ) {
                    ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOC, 0x00, ucCheckFlag);
                }
            }
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOR1, ucTimerValue1, ucCheckFlag);
            }
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOR2, ucTimerValue2, ucCheckFlag);
            }
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOR3, ucTimerValue3, ucCheckFlag);
            }
            if ( !ucResult ) {
                ucResult = HALICC_TDA8007WriteREG(ucReader, REG_TOC, ucTimerType, ucCheckFlag);
            }
            break;
    }
    //   printk("[%s,%d]\n",__func__, __LINE__);
    return(ucResult);
}

unsigned char HALICC_CheckBGT(unsigned char ucReader)
{
    unsigned char ucREGMSR = 0;

    HALICC_TDA8007ReadREG(ucReader, REG_MSR, &ucREGMSR, 0);

    return( (ucREGMSR & 0x20) );
}

unsigned char HALICC_SetGTR(unsigned char ucReader, unsigned char ucGTR)
{
    return(HALICC_TDA8007WriteREG(ucReader, REG_GTR, ucGTR, 0));
}

void HALICC_TDA8007EnableWrite(void)
{

}

void HALICC_TDA8007DisableWrite(void)
{

}

#endif

void CLKO_set(void)
{
    struct clk *clko_1, *parent;
    unsigned long rate;
    mxc_request_iomux(MX25_PIN_CLKO, MUX_CONFIG_FUNC); //request gpio
    clko_1 = clk_get(NULL, "clko_clk");
    if (IS_ERR(clko_1))  return;
    parent = clk_get(NULL, "ipg_clk");
    if (IS_ERR(parent)) return;

    clk_set_parent(clko_1, parent);
    rate = clk_round_rate(clko_1, 14745600);
    if (rate < 8000000 || rate > 27000000)
    {
        printk("CLKO rate error!!\n");
        clk_put(parent);
        clk_put(clko_1);
        return;
    }
    clk_set_rate(clko_1,rate);
    clk_enable(clko_1);

}

void icccard_irq_set(void)
{
    //����GPG9,GPG11Ϊ�жϹ��ܣ�֧���������裬�½��ش���

    int ret;
    //	CLKO_set();
    /*ICC request OK*/
    mxc_request_iomux(MX25_PIN_GPIO_B, MUX_CONFIG_FUNC); //request gpio
    mxc_iomux_set_pad(MX25_PIN_GPIO_B, INT_4GPIO_B_CTL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_GPIO_B),NULL);
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_B));

    //  irq= IOMUX_TO_IRQ(MX25_PIN_GPIO_B);
    //    printk("irq num:0x%x\n", irq);
    set_irq_type(TDA8007_INT_GROUP0,IRQF_TRIGGER_LOW);
    //   ret = request_irq(irq, s3c2410_ic_interrupt0,IRQF_DISABLED, DEVICE_NAME,NULL);
    ret = request_irq(TDA8007_INT_GROUP0, s3c2410_ic_interrupt0,IRQF_DISABLED, DEVICE_NAME,(void*)(&tasklet_param_ptr));
    if (ret)
    {
        unregister_chrdev(icc_major, DEVICE_NAME);
        printk("register IC Card interrupt failed...\n");
    }  else
    {
        //   enable_irq(irq);
        //    printk("register IC Card interrupt succeed!!!\n");
    }

    /*sim 3, 4   PIN IOMUXC_SW_MUX_CTL_PAD_GPIO_D  MX25_PIN_GPIO_D*/
    mxc_request_iomux(MX25_PIN_GPIO_D, MUX_CONFIG_FUNC); //request gpio
    mxc_iomux_set_pad(MX25_PIN_GPIO_D, INT_4GPIO_B_CTL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_GPIO_D),NULL);
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_D));

    //  irq= IOMUX_TO_IRQ(MX25_PIN_GPIO_D);
    //    printk("irq num:0x%x\n", irq);
    set_irq_type(TDA8007_INT_GROUP1,IRQF_TRIGGER_LOW);

    ret = request_irq(TDA8007_INT_GROUP1, s3c2410_ic_interrupt0,IRQF_DISABLED, DEVICE_NAME,(void*)(&tasklet_param_ptr2));
    // ret = request_irq(TDA8007_INT_GROUP1, s3c2410_ic_interrupt0,IRQF_SHARED, DEVICE_NAME,(void*)(&irq_nu2));
    if (ret)
    {
        unregister_chrdev(icc_major, DEVICE_NAME);
        printk("register IC Card interrupt failed...\n");
    }  else
    {
        //  enable_irq(irq);
        //    printk("register IC Card interrupt succeed!!!\n");
    }

    TDA8007IRQ[0] = TDA8007_INT_GROUP0;
    TDA8007IRQ[1] = TDA8007_INT_GROUP1;
}
/***************************************************
 * IC����λ,
 * type ��λ��ʽ��
 *    �临λ HALICC_RESET_COLD
 *    �ȸ�λ HALICC_RESET_WARM
 ***************************************************/

static int iccard_init(void)
{
    int i, j;
    unsigned int irq;
    //    unsigned long addrN[2][16];



    //������Ӧ�жϵ�ַ
    TDA8007BASEADDR[0] = pSMDK2410_IC1_IO;
    TDA8007BASEADDR[1] = pSMDK2410_IC2_IO;


    for (i = 0; i < MAX_CARDER_GROUP; i++) {
        for (j = 0; j < MAX_REG_NB; j++) {
#ifdef HV4
            addrN[i][j] = (unsigned long )ioremap((TDA8007BASEADDR[i] + j), 1);
#else
            addrN[i][j] = (unsigned long )ioremap(ADDR_FPGA_CONV(TDA8007BASEADDR[i] + j), 1);
#endif
            if (icc_dbg>=2)
                printk("addr[%d/%d]=%x\n", i, j, (unsigned int)addrN[i][j]);
        }
    }

    kICDEV = kzalloc(sizeof(struct ic_device), GFP_KERNEL);
    if (kICDEV == 0) {
        printk("!!!!!!!!Out of memory for ICDEV!\n");
        return -ENOMEM;
    }
    //      memset(kICDEV, 0, sizeof(struct ic_device));
    //        printk("virtual addr=%lx %lx\n", TDA8007BASEADDR[0], TDA8007BASEADDR[1]);

    //�Է���TDA8007оƬ�ṹ���г�ʼ��
    for (i = 0; i < MAX_CARDER_GROUP; i++) {
        irq = TDA8007IRQ[i];

        kICDEV->ic_reader[i*2].chipid= i;
        kICDEV->ic_reader[i*2+1].chipid= i;

        kICDEV->ic_reader[i*2].reader = HALICC_READER0;
        for (j = 0; j < MAX_REG_NB; j++)
            kICDEV->ic_reader[i*2].addr[j] = addrN[i][j];
        kICDEV->ic_reader[i*2].irq = irq;

        kICDEV->ic_reader[i*2+1].reader = HALICC_READER2;
        for (j = 0; j < MAX_REG_NB; j++)
            kICDEV->ic_reader[i*2+1].addr[j] = addrN[i][j];
        kICDEV->ic_reader[i*2+1].irq = irq;
    }


    memset(kICDEV->pin, 0, ICBUF_MAX_LEN);
    memset(kICDEV->pout, 0, ICBUF_MAX_LEN);
    kICDEV->cur_reader_id = 1;	//���õ�ǰ����ͷΪ1,��1��ĵ�2����ͷ
    kICDEV->cur_reader = kICDEV->ic_reader[kICDEV->cur_reader_id];


    init_waitqueue_head(&my_queue);


    /*
     * ʹ��mmap ��ʽʵ��ӳ��
     */
    /* allocate a memory area with kmalloc. Will be rounded up to a page boundary */

    //if ((kmalloc_ptr = kmalloc((NPAGES + 2) * PAGE_SIZE, GFP_KERNEL)) == NULL) {
    iccptr->gMMAP_IC_Drv= (DRV_MMAP *)kmalloc(sizeof(DRV_MMAP),GFP_KERNEL);
    gDRVOUT = (DRVOUT *)&iccptr->gMMAP_IC_Drv->pDrvOut;

    gDRVOUT->ucGenStatus=0xaa;
    HALICC_Install();

    ic_readb(addrN[0][15]);
    ndelay(2);
    ic_readb(addrN[1][15]);
    icccard_irq_set();

#ifdef MY_TEST
    LIBICC_Init();
#endif

    return 0;
}

/***********************************************************************
 * halicc.c ��ʵ��
 ***********************************************************************/
#ifdef MY_TEST
unsigned char HALDRV_ICC_Abort(DRV *pDrv)
{
    volatile HALDRV *pHALDrv;

    if ( pDrv->ucDrvID >= HALDRV_MAXNB) {
        return(HALDRVERR_BADDRVID);
    }
    if ( !apHALDrv[pDrv->ucDrvID] ) {
        return(HALDRVERR_NOTEXIST);
    }
    pHALDrv = apHALDrv[pDrv->ucDrvID];
    if ( pHALDrv->pfnAbort ) {
        return((*(pHALDrv->pfnAbort))(pDrv->pDrvIn, pDrv->pDrvOut));
    } else {
        return(HALDRVERR_NOHANDLE);
    }
}

unsigned char HALDRV_ICC_Run(DRV *pDrv)
{
    volatile HALDRV *pHALDrv;

    if ( pDrv->ucDrvID >= HALDRV_MAXNB) {
        return(HALDRVERR_BADDRVID);
    }
    if ( !apHALDrv[pDrv->ucDrvID] ) {
        return(HALDRVERR_NOTEXIST);
    }

    pHALDrv = apHALDrv[pDrv->ucDrvID];
    if ( pHALDrv->pfnRun ) {
        return((*(pHALDrv->pfnRun))(pDrv->pDrvIn, pDrv->pDrvOut));
    } else {
        return(HALDRVERR_NOHANDLE);
    }
}


unsigned char HALDRV_ICC_State(DRV *pDrv)
{
    volatile HALDRV *pHALDrv;

    if ( pDrv->ucDrvID >= HALDRV_MAXNB) {
        return(HALDRVERR_BADDRVID);
    }
    if ( !apHALDrv[pDrv->ucDrvID] ) {
        return(HALDRVERR_NOTEXIST);
    }
    pHALDrv = apHALDrv[pDrv->ucDrvID];
    if ( pHALDrv->pfnState ) {
        return((*(pHALDrv->pfnState))(pDrv->pDrvIn, pDrv->pDrvOut));
    } else {
        return(HALDRVERR_NOHANDLE);
    }
}

unsigned char HALDRV_ICC_Reset(DRV *pDrv)
{
    volatile HALDRV *pHALDrv;

    if ( pDrv->ucDrvID >= HALDRV_MAXNB) {
        return(HALDRVERR_BADDRVID);
    }
    if ( !apHALDrv[pDrv->ucDrvID] ) {
        return(HALDRVERR_NOTEXIST);
    }
    pHALDrv = apHALDrv[pDrv->ucDrvID];
    if ( pHALDrv->pfnReset ) {
        return((*(pHALDrv->pfnReset))(pDrv->pDrvIn, pDrv->pDrvOut));
    } else {
        return(HALDRVERR_NOHANDLE);
    }
}

unsigned char HALDRV_ICC_Ioctl(unsigned char ucDrvID, unsigned char ucType, unsigned char *pucData)
{
    volatile HALDRV *pHALDrv;

    if ( ucDrvID > HALDRV_MAXNB )
        return(HALDRVERR_NOTEXIST);
    if ( !apHALDrv[ucDrvID] ) {
        return(HALDRVERR_NOTEXIST);
    }
    pHALDrv = apHALDrv[ucDrvID];
    if ( pHALDrv->pfnIoctl ) {
        return((*(pHALDrv->pfnIoctl))(ucType, pucData));
    } else {
        return(HALDRVERR_NOHANDLE);
    }
}

void HALDRV_ICC_Init(void)
{
    unsigned char ucI;
    for (ucI = 0; ucI < HALDRV_MAXNB; ucI++) {
        apHALDrv[ucI] = 0;
    }

}

unsigned char HALDRV_ICC_Install(HALDRV *pHALDrv)
{
    unsigned char ucResult;
    unsigned char ucDrvID;

    ucDrvID = pHALDrv->ucDrvID;
    if ( ucDrvID < HALDRVID_RESERVNB ) {
        if ( apHALDrv[ucDrvID] )
            return(HALDRVERR_SUCCESS);
        else {
            if ( pHALDrv->pfnInstall ) {
                pHALDrv->pDrvIn = (DRVIN *)&gDRVIN;
                pHALDrv->pDrvOut =gDRVOUT;								
                apHALDrv[ucDrvID] = pHALDrv;
                ucResult = (*(pHALDrv->pfnInstall))();

                return(ucResult);
            }
            return(HALDRVERR_NOHANDLE);
        }
    }



    return(HALDRVERR_BADDRVID);
}

unsigned char HALDRV_ICC_Uninstall(unsigned char ucDrvID)
{
    unsigned char ucResult;
    volatile HALDRV *pHALDrv;

    ucResult = HALDRVERR_SUCCESS;
    if ( ucDrvID < HALDRVID_RESERVNB ) {
        if ( !apHALDrv[ucDrvID] )
            return(HALDRVERR_NOTINSTALL);
        else {
            pHALDrv = apHALDrv[ucDrvID];
            if ( pHALDrv->pfnUninstall ) {
                ucResult = (*(pHALDrv->pfnUninstall))();
            }
            apHALDrv[ucDrvID] = 0;
            return(ucResult);
        }
    }
    return(HALDRVERR_BADDRVID);
}

unsigned char HALDRV_ICC_Process(DRV *pDrv)
{
    switch ( pDrv->ucDrvCMD  ) {
        case HALDRVCMD_ABORT:
            return(HALDRV_ICC_Abort(pDrv));
        case HALDRVCMD_EXECUTE:
            return(HALDRV_ICC_Run(pDrv));
        case HALDRVCMD_STATEREQ:
            return(HALDRV_ICC_State(pDrv));
        case HALDRVCMD_RESET:
            return(HALDRV_ICC_Reset(pDrv));
        default:
            return(HALDRVERR_BADCMD);
    }
}
#endif


static inline void iccTaskle_callbak(unsigned long data)
{

    unsigned long *pucChipAddr;
    unsigned char ucEvent;
    unsigned char ucREGUSR;
    unsigned char ucREGHSR;
    unsigned char ucMask = 0;
    unsigned char ucIndex = 0;

    unsigned char ucReader;
    unsigned char ucREGCSR;

    ICINFO *dev;
    ICCINFO *pCurrICCInfo;

    u8 chipID = 0;
    u8 idx_8007 = 10;
    idx_8007 =*((u8*)data);

    chipID =  idx_8007 >> 1;

    dev = (ICINFO *)&kICDEV->ic_reader[idx_8007];

    pucChipAddr = dev->addr;
    ucReader    = dev->reader;
    ucREGCSR    = ic_readb(pucChipAddr[REG_CSR]);
    ucReader    = 2 * chipID + ucReader + (ucREGCSR & 0x07) - 1;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucREGUSR = ic_readb(pucChipAddr[REG_USR]);
    ucREGHSR = ic_readb(pucChipAddr[REG_HSR]);
    ucEvent  = ucREGHSR;


    if ( ucEvent ) {
        ucMask = 0x01;
        ucIndex = 0;
        do {

            if ( ucEvent & ucMask ) {
                if ( HSREvent[ucIndex].pfnEvent ) {
                    (*(HSREvent[ucIndex].pfnEvent))(chipID);
                }
                ucEvent = ucEvent & (~ucMask);
            }
            ucMask = ucMask << 1;
            ucIndex ++;
        } while (  (ucMask) && (ucEvent) );
    }
    ucEvent = ucREGUSR;
    if ( ucEvent ) {
        ucMask = 0x01;
        ucIndex = 0;
        do {
            if ( ucEvent & ucMask ) {
                if ( USREvent[ucIndex].pfnEvent ) {
                    (*(USREvent[ucIndex].pfnEvent))(chipID);
                }
                ucEvent = ucEvent & (~ucMask);
            }
            ucMask = ucMask << 1;
            ucIndex ++;
        } while (  (ucMask) && (ucEvent) );
    }

    ic_readb(addrN[0][15]);
    ic_readb(addrN[1][15]);
}
unsigned int atr_flag = 1;

static irqreturn_t s3c2410_ic_interrupt0(int irq, void *dev_id)
{
    tasklet_icc.data = (unsigned long)dev_id;
    iccTaskle_callbak(tasklet_icc.data);

    return IRQ_HANDLED;
}
ssize_t s3c2410_ic_open(struct inode *inode, struct file *file)
{
    struct icc_dev *dev;

    dev = container_of(inode->i_cdev, struct icc_dev, cdev);        
    file->private_data = dev;
    return 0;
}


static unsigned int icc_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    //struct icc_dev *dev = filp->private_data;
    printk("%d iccptr->card_status:0x%x\r\n",__LINE__,iccptr->card_status);
    poll_wait(filp, &icc_wait, wait);
    printk("%d iccptr->card_status:0x%x\r\n",__LINE__,iccptr->card_status);
    if (iccptr->card_status == CARD_INSERTED)
    {
        mask  |= POLLIN | POLLRDNORM;
        iccptr->card_status = 0;
    }
    return mask;
}

static int icc_fasync(int fd, struct file * filp, int on)
{
    int retval;
    struct icc_dev *dev = filp->private_data;


    retval=fasync_helper(fd,filp,on,&dev->fasync_queue);

    if(retval<0)
    {
        printk("%s, fasync_helper failed\n", __func__);
        return retval;
    }

    return 0;
}

ssize_t s3c2410_ic_release(struct inode *inode, struct file *file)
{
    if (icc_dbg>=2)
        printk("%s can copy=%x\n", __func__, can_mmap_data_to_user);

    icc_fasync(-1, file, 0);

    file->private_data = NULL;

    return 0;
}

/*kernel--->user*/
ssize_t s3c2410_ic_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    int ret;
    int i = 0;

    struct icc_dev *dev;

    dev = file->private_data;

    printk("[%s,%d]\r\n",__func__,__LINE__);


    ret = copy_to_user(buf, &dev->gMMAP_IC_Drv->pDrvOut, sizeof(DRVOUT));
    printk("[%s,%d]\r\n",__func__,__LINE__);
    printk("\r\n 0x%x\n\r\n",  buf[0]);
    for (i = 0; i <= buf[0]; i++)
        printk("(%02x) ", buf[i]);

    return sizeof(DRVOUT);

}

//static ssize_t cs8900_eeprom_fwrite(struct file *file, const char *buf, size_t count, loff_t *f_pos);
/*kernel<---user*/
ssize_t s3c2410_ic_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    int ret;
    ret = copy_from_user(kICDEV->pin + kICDEV->pin_len, buf, count);
    kICDEV->pin_len += count;
    return count;
}

int copy_flag = 1;

ssize_t s3c2410_ic_ioctl(struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg)
{
    int ret, i;
    unsigned char ucPresent = 0;

    ICCINFO *pCurrICCInfo = &ICCInfo[0];

    printk("\r\n^^^^^^^^^[%s,%d]\r\n", __func__,__LINE__);

    if (_IOC_TYPE(cmd) != S3C2410_IC_IOCTL_BASE) {
        printk("ERROR: not matched device\n");
        return -EINVAL;
    }
    switch (cmd) {
        case IC_CMD_PROCESS://ic��������ܳ���,
            ret = copy_from_user( (DRV_MMAP *)iccptr->gMMAP_IC_Drv, (DRV_MMAP *)arg, sizeof(DRV_MMAP) );
        kDRV.ucDrvCMD = iccptr->gMMAP_IC_Drv->ucDrvCMD;

        kDRV.ucDrvID = iccptr->gMMAP_IC_Drv->ucDrvID;
        kDRV.pDrvIn = (DRVIN *)&iccptr->gMMAP_IC_Drv->pDrvIn;
        kDRV.pDrvOut= (DRVOUT *)&iccptr->gMMAP_IC_Drv->pDrvOut;

        if (kDRV.pDrvIn->ucLen == 0)
        {
            iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus = 1;
            iccptr->gMMAP_IC_Drv->pDrvOut.aucData[0] = 0x1;
            iccptr->gMMAP_IC_Drv->pDrvOut.aucData[1] = 0x37;
            ret=copy_to_user((DRVOUT *)arg,  (DRVOUT *) &iccptr->gMMAP_IC_Drv->pDrvOut, sizeof(DRVOUT) );
            if(ret!=0)
            {
                printk("%s:%d:copy_to_user Error.",__FILE__,__LINE__);
            }
            printk(" ucLen 0 error return -44\n");
            return -1;
        }
        //if (0xD == kDRV.pDrvIn->ucType)  msleep(80);
        if (kDRV.pDrvIn->aucData[0] == 0  && kDRV.pDrvIn->ucType != 0x0C && kDRV.pDrvIn->ucType != 0x0A)
        {
            HALICC_Detect(0,&ucPresent);
            if (!(ucPresent & 0x04))
            {
                iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus = 1;
                iccptr->gMMAP_IC_Drv->pDrvOut.aucData[0] = 0x1;
                iccptr->gMMAP_IC_Drv->pDrvOut.aucData[1] = 0x37;
                ret=copy_to_user((DRVOUT *)arg,  (DRVOUT *) &iccptr->gMMAP_IC_Drv->pDrvOut, sizeof(DRVOUT) );
                if(ret!=0)
                {
                    printk("%s:%d:copy_to_user Error.",__FILE__,__LINE__);
                }
                return -1;
            }
        }

        if (dbg)	printk("\r\nucType:0x%x\r\n",kDRV.pDrvIn->ucType);
        HALDRV_ICC_Process(&kDRV);
        //   if (0x7 == kDRV.pDrvIn->ucType)
        {
            if (wait_event_interruptible_timeout(icc_wait,iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus == HALDRVSTATE_ENDED,msecs_to_jiffies(10000)) >0)
                //if (wait_event_interruptible(icc_wait,iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus == HALDRVSTATE_ENDED) == 0)
            {
                atr_flag = 1;
                if (dbg)
                {
                    printk("\r\n^Data Recv len:0x%x\n\r\n",  iccptr->gMMAP_IC_Drv->pDrvOut.aucData[0]);
                    for (i = 0; i <= iccptr->gMMAP_IC_Drv->pDrvOut.aucData[0]; i++)
                        printk("(%02x) ", iccptr->gMMAP_IC_Drv->pDrvOut.aucData[i]);
                }
            }else
            {
                printk("\r\n^^^^^^^^^[%s,%d]^^^^^^^^^iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus:0x%x\r\n", __func__,__LINE__,iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus);
                iccptr->gMMAP_IC_Drv->pDrvOut.ucDrvStatus = HALDRVERR_NOTRESPONSE;
                iccptr->gMMAP_IC_Drv->pDrvOut.ucGenStatus = HALDRVSTATE_ENDED;
                iccptr->gMMAP_IC_Drv->pDrvOut.aucData[0]  = HALDRVERR_NOTRESPONSE;
            }

        }

        ret = copy_to_user((DRVOUT *)arg,  (DRVOUT *)&iccptr->gMMAP_IC_Drv->pDrvOut, sizeof(DRVOUT) );
        if(ret!=0)
        {
            printk("%s:%d:copy_to_user Error.",__FILE__,__LINE__);
        }
        copy_flag = 1;
        break;

        case  IC_CMD_POWUP:
            printk("\r\n*******next is gMMAP_IC_Drv->pDrvOut.aucData************\r\n");
            for (i = 0; i <= iccptr->gMMAP_IC_Drv->pDrvOut.aucData[0]; i++)
                printk("%02x ", iccptr->gMMAP_IC_Drv->pDrvOut.aucData[i]);

            kDRV.pDrvOut= (DRVOUT *)&iccptr->gMMAP_IC_Drv->pDrvOut;
            // ret = cn_msg_sender( &iccptr->gMMAP_IC_Drv->pDrvOut, 11);
            memset(&iccptr->gMMAP_IC_Drv->pDrvOut, 0, sizeof(DRVOUT));
            break;


        case IC_CMD_POWDOWN:
            ret = copy_from_user( (DRV *)arg, (DRV *) & kDRV,  sizeof(DRV) );
            printk("%s IC_CMD_GETSTATUS ok\n", __func__);
            break;

        case ICC_IOC_POLLING_START:

            printk("\r\n^^^^^^^^^[%s,%d]\r\n", __func__,__LINE__);
            //	 HALICC_Deactive(0);
            // mdelay(10);
            kDRV.ucDrvCMD = iccptr->gMMAP_IC_Drv->ucDrvCMD;

            kDRV.ucDrvID = iccptr->gMMAP_IC_Drv->ucDrvID;
            kDRV.pDrvIn = (DRVIN *)&iccptr->gMMAP_IC_Drv->pDrvIn;
            kDRV.pDrvOut= (DRVOUT *)&iccptr->gMMAP_IC_Drv->pDrvOut;

            iccptr->card_status = CARD_EJECTED;
            pCurrICCInfo->ucAutoPower =1;

            HALICC_Detect(0,&ucPresent);
            printk("\r\n^^^^^^^^^[%s,%d] kDRV.pDrvIn->aucData[0]:0x%x \r\n", __func__,__LINE__,
                   kDRV.pDrvIn->aucData[0] );
            //     	  if (kDRV.pDrvIn->aucData[0] == 0)
        {

            char inData[3]={0};
            char outData[3] = {0};
            int inLen;

            inData[0] = 0x0;
            inData[1] = 0x0;
            inLen = 3;

            printk("\r\n^^^^^^^^^[%s,%d] ucPresent:0x%x \r\n", __func__,__LINE__,
                   ucPresent	);
            if (ucPresent & 0x04)
            {
                printk("\r\n^^^[%s,%d] \r\n", __func__,__LINE__);
                remove_flag = 1;
                ICCCMD_Insert(inData,inLen,outData);
                printk("\r\n^^^^^^^^^[%s,%d] 0x%x:0x%x:0x%x \r\n", __func__,__LINE__,
                       outData[0],outData[1],outData[2]);


            }
        }
            break;

        case ICC_IOC_POLLING_ABORT:
            iccptr->card_status = 0;
            pCurrICCInfo->ucAutoPower =0;

            break;

        default:
            printk("\n!!!!!!!!!!!!!!!!!Unknown command:%x\n", cmd);
            break;
    }

    return 0;
}

void s3c2410_ic_vma_open(struct vm_area_struct *vma)
{
    memset(iccptr->gMMAP_IC_Drv, 0, PAGE_SIZE);

    can_mmap_data_to_user = 1;
}

void s3c2410_ic_vma_close(struct vm_area_struct *vma)
{
    can_mmap_data_to_user = 0;
}

static const struct file_operations s3c2410_icc_fops = {
    .owner =    THIS_MODULE,
    .read = s3c2410_ic_read,
    .write = s3c2410_ic_write,
    .ioctl = s3c2410_ic_ioctl,
    .open = s3c2410_ic_open,
    .release =  s3c2410_ic_release,
    .llseek = no_llseek,
    .fasync     =   icc_fasync,
    .poll      = icc_poll,
};
static  DEFINE_MUTEX(icc_genl_mutex);

static int  gxdicc_probe(struct platform_device *pdev)
{

    int ret;
    dev_t dev = 0;

    iccptr = kzalloc(sizeof(struct icc_dev), GFP_KERNEL);
    if (!iccptr) {
        printk("%s malloc for iccptr failed!\n", __func__);
        return -ENOMEM ;
    }

    init_MUTEX(&iccptr->sem);

    if (icc_major) {
        dev = MKDEV(icc_major, icc_minor);
        ret = register_chrdev_region(dev, 1, DEVICE_NAME);
    } else {
        ret = alloc_chrdev_region(&dev, icc_minor, 1, DEVICE_NAME);
        icc_major = MAJOR(dev);
        icc_minor = MINOR(dev);
    }
    if (ret < 0) {
        printk(KERN_WARNING "icc driver: can't get major %d\n", icc_major);
        goto failed_register;
    }

    cdev_init(&iccptr->cdev, &s3c2410_icc_fops);
    iccptr->cdev.owner = THIS_MODULE ;
    iccptr->cdev.ops = &s3c2410_icc_fops;

    ret = cdev_add(&iccptr->cdev, dev, 1);
    if (ret) {
        printk(KERN_NOTICE "add cdev error.\n");
        goto failed_add;
    }

    iccptr->ic_class=class_create(THIS_MODULE, "icc");
    if (IS_ERR(iccptr->ic_class)) {
        printk("Err: failed in creating class.\n");
        return -1;
    }

    /* register your own device in sysfs, and this will cause udevd to create corresponding device node */
    device_create(iccptr->ic_class, NULL, MKDEV(icc_major, icc_minor), NULL, DEVICE_NAME );

    init_waitqueue_head(&icc_wait);
    iccptr->card_status = 0;


    iccard_init();
    init_completion(&comp);

    ic_readb(addrN[0][15]);
    ndelay(2);
    ic_readb(addrN[1][15]);

    HALDRV_ICC_Install(&HALICCDrv);

    printk("<1>S3C2410 IC TDA8007A  Driver v" DRIVER_VERSION "	%d:%d with\
    %s, icc_dbg=%d\n", icc_major, icc_minor, DEVICE_NAME, icc_dbg);
    return 0;

    failed_add:
    failed_register:
    kfree(iccptr);
    unregister_chrdev_region(dev, 1);

    return -1;
}

static int  gxdicc_remove(struct platform_device *pdev);
static int  tda8007_suspend(struct platform_device *dev, pm_message_t state)
{
    disable_irq(TDA8007_INT_GROUP0);
    return 0;
}

static int  tda8007_resume(struct platform_device *dev)
{
    enable_irq(TDA8007_INT_GROUP0);
    return 0;
}
static struct platform_driver tda8007_driver ={
    .probe           = gxdicc_probe,
    .remove          = gxdicc_remove,
    .suspend         = tda8007_suspend,
    .resume          = tda8007_resume,
    .driver = {
        .name 	= PROBE_NAME,
        .owner 	= THIS_MODULE,
    },
};

static void mxc_tda8007_release(struct device *dev)
{
    printk("[%s,%d]\n", __func__, __LINE__);
}

static struct platform_device tda8007_device ={
    .name = PROBE_NAME,//"TDA_8007"
    .id   = -1,
    .dev  = {
        .release  =  mxc_tda8007_release,
    },
};

static int __init s3c2410_drv_init(void)
{
    tasklet_init( &tasklet_icc, iccTaskle_callbak, tasklet_param_ptr);
    
    platform_device_register( &tda8007_device);

    return platform_driver_register( &tda8007_driver);
}

static int  gxdicc_remove(struct platform_device *pdev)
{
    free_irq(kICDEV->ic_reader[0].irq , (void*)(&tasklet_param_ptr));
    free_irq(kICDEV->ic_reader[2].irq , (void*)(&tasklet_param_ptr2));

    if (! kICDEV) kfree(kICDEV);
    if (! kmalloc_ptr)  kfree(kmalloc_ptr);

    cdev_del(&iccptr->cdev);
    unregister_chrdev_region(MKDEV(icc_major, icc_minor), 1);

    device_destroy(iccptr->ic_class, MKDEV(icc_major, icc_minor));
    class_destroy(iccptr->ic_class);
    kfree(iccptr);
    return 0;
}

static void __exit s3c2410_drv_exit(void)
{
    platform_driver_unregister(&tda8007_driver);
    platform_device_unregister(&tda8007_device);
    tasklet_kill(&tasklet_icc);
}

module_init(s3c2410_drv_init);
module_exit(s3c2410_drv_exit);

module_param(dbg,int, S_IRUGO);

MODULE_AUTHOR("GXD <gu.xd@sand.cn>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TDA8007 IC Card driver for I.MX258");
