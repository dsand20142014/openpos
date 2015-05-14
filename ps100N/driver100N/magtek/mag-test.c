/*
 ********************************************************************************
 * Magig card reader Driver for IMX258(hard decode circuit)
 * 磁卡硬解码方式驱动：MagTek 6541
 * Linux 2.6.22,
 *
 * v1.4 增加刷卡超时
 * v1.3 增加定时器测试代码
 * v1.2 支持使用tasklet方式进行中断处理，
 支持标准的read方式输出数据
 * v1.1 支持mmap及直接访问方式与应用通讯，分别配置USE_REAL_MMAP/USE_DIRECT_ACCESS
 * v1.0 支持mmap方式与应用通讯

 * v0.1 新的交叉接线方式，采用中断方式工作
 * GPH10 -- STROBE
 * GPG3/EINT11  -- DATA
 *
 *
 ********************************************************************************
 */
#include <linux/device.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/aio.h>
#include <asm/uaccess.h>
//#include <asm/semaphore.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h>	//for request_irq
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
//#include <asm/arch-s3c2410/gpio.h>
//#include <asm/arch-s3c2410/regs-gpio.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>
//#include <asm/new_fifo.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>

#include <asm/io.h> /* for virt_to_phys */
#include <linux/slab.h> /* for kmalloc and kfree */

//#include <linux/osdriver.h> //sand api

#include <linux/interrupt.h>

#include <linux/timer.h>

#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/irqs.h>

#include "imx258_mag.h"

#include <linux/types.h>
#include <mach/gpio.h>
#include "mx25_pins.h"
#include "osdriver.h"

#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include "iomux.h"

#define SAND_MAG_VERSION "1.4"


//#define tmp_dbg

#define DEBUGGING	1
/*
   struct timer_list {
   unsigned long expires;
   void (*function)(unsigned long);
   unsigned long data;
   };
 */

#if DEBUGGING
#define DPRINTK(n, args...)	do{if(n <= DEBUGGING) printk(args);}while(0);
#else
#define DPRINTK(n, args...)
#endif


#define HIGH 1
#define LOW 0

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

void mag_timer_fn(unsigned long arg);
static int mag_timer_status=0;
volatile unsigned char *addr_mag;

struct magreader_dev {
		struct timer_list mag_timer;
		struct timer_list mag_swiptimeout; //zengshu 20110127
		struct semaphore sem ;     /* Mutual exclusion */
		spinlock_t lock;
		struct cdev cdev ;
		struct class *mag_class;
		struct tasklet_struct tlet;
		wait_queue_head_t wait;
		DRV_OUT *kmalloc_area;
		char data[100];
};
struct magreader_dev *magptr ;
static int flag=0;


//wait_queue_head_t mag_wait;
DECLARE_WAIT_QUEUE_HEAD(mag_wait);

void mag_tasklet_fn(unsigned long data);


#define MAG_DATA IMX258_GPG3
#define MAG_DATA_OFFSET	3
#define MAG_STROBE IMX258_GPH10

#define HARDMAG_SUCCESS        0x00
#define HARDMAG_PAR            0x01
#define HARDMAG_LRC            0x02
#define HARDMAG_NOSTART        0x03
#define HARDMAG_NOEND          0x04
#define HARDMAG_NOTRACK        0x06



/*Timing, ns */
#define Tsu1_rst 180
#define Tsu2_rst (5*1000)
#define TstbL  300
#define TstbH  300
#define TstbH2  300
#define Tsu_mode  40
#define Th_mode  40
#define TstbH_CP 1900 //ns



int mag_major =   240;
int mag_minor =   0;


static int mag_dbg = 0;
module_param(mag_dbg, int, 0);
EXPORT_SYMBOL(mag_dbg);


static int mag_tdelay = 0;
module_param(mag_tdelay, int, 0);
EXPORT_SYMBOL(mag_tdelay);

static int mag_times = 100;
module_param(mag_times, int, 0);
EXPORT_SYMBOL(mag_times);


#define USE_REAL_MMAP //实现mmap方法

#define NPAGES 1
// pointer to the kmalloc'd area, rounded up to a page boundary
static DRV_OUT *kmalloc_area;

// original pointer for kmalloc'd area as returned by kmalloc
static void *kmalloc_ptr;


TK_BUFF	TK_Buff[3];
TK_DATA TK_Data[3];


#define MAX_TK_BUFLen	140			//定义每磁道数据最大长度
#define MGHW_SUCCESS 0
#define MGHW_ODDERR 4
#define MGHW_LRCERR 5


const unsigned char BitMask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

const unsigned char TKBINData[] = {
		0x40, 0x01, 0x02, 0x43, 0x04, 0x45, 0x46, 0x07,
		0x08, 0x49, 0x4A, 0x0b, 0x4c, 0x0d, 0x0e, 0x4f,
		0x10, 0x51, 0x52, 0x13, 0x54, 0x15, 0x16, 0x57,
		0x58, 0x19, 0x1a, 0x5b, 0x1c, 0x5d, 0x5e, 0x1f,
		0x20, 0x61, 0x62, 0x23, 0x64, 0x25, 0x26, 0x67,
		0x68, 0x29, 0x2a, 0x6b, 0x2c, 0x6d, 0x6e, 0x2f,
		0x70, 0x31, 0x32, 0x73, 0x34, 0x75, 0x76, 0x37,
		0x38, 0x79, 0x7a, 0x3b, 0x7c, 0x3d, 0x3e, 0x7f
};
const unsigned char TKASCData[] = {
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
		0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
		0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
};

unsigned char TrackOneData[] = {
		0x40, 0x01, 0x02, 0x43, 0x04, 0x45, 0x46, 0x07, 0x08, 0x49, 0x4A, 0x0b, 0x4c, 0x0d, 0x0e, 0x4f,
		0x10, 0x51, 0x52, 0x13, 0x54, 0x15, 0x16, 0x57, 0x58, 0x19, 0x1a, 0x5b, 0x1c, 0x5d, 0x5e, 0x1f,
		0x20, 0x61, 0x62, 0x23, 0x64, 0x25, 0x26, 0x67, 0x68, 0x29, 0x2a, 0x6b, 0x2c, 0x6d, 0x6e, 0x2f,
		0x70, 0x31, 0x32, 0x73, 0x34, 0x75, 0x76, 0x37, 0x38, 0x79, 0x7a, 0x3b, 0x7c, 0x3d, 0x3e, 0x7f
};

unsigned char TrackOneASCII[] = {
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
};


unsigned char ucBuffer1, ucBuffer2;
unsigned char ucDirection;

static int MSR_reset(struct magreader_dev *dev);
unsigned char MSR_IsReady(void);
//static void MSR_ArmedToRead(void);
void MSR_UpdateBuffer(void);
void TEST_HW_Reverse( unsigned char *pInBuff, unsigned char *pOutBuff );
unsigned char TEST_HW_DirectionJG(unsigned char *pInBuff);
unsigned long TEST_HW_ASCIIConvert1( unsigned char *pInBuff, unsigned char *pOutBuff  );
unsigned long TEST_HW_ASCIIConvert23( unsigned char *pInBuff, unsigned char *pOutBuff  );
void TEST_HW_Convert1( unsigned char *pInBuff, unsigned char *pOutBuff );
void TEST_HW_Convert23( unsigned char *pInBuff, unsigned char *pOutBuff );
unsigned char TEST_HW_Verify1( unsigned char *pInBuff );
unsigned char TEST_HW_Verify23( unsigned char *pInBuff );
static void magnetic_track_gather(void);
#define Delay(x)		{unsigned short uiI=x*2;while(uiI--);}

/*******************************************************************************/
void pull_io(void)
{
   
}
/*设置data引脚高低*/
static  void writeDATA(unsigned int c)
{
////		imx258_gpio_setpin(MAG_DATA, c);
/*	
	
 	if(addr_mag < 0)
        {
//          printk("ioremap(0xb2800000, 8)  failed \n");
        }
	else
	{
//          printk("ioremap(0xb2800000, 8)  success123 \n");
//          *addr_mag |= (1<<4);

			printk(" addr_magx = %X \n", addr_mag);
			printk(" addr_magd = %d \n", addr_mag);
          printk(" *addr_mag = %X \n", *addr_mag);
        }	
*/
/*
addr_mag = (uint8_t *)ioremap_nocache(0xb2800000, 0x01);
printk("wr111 addr_mag : %d\n", addr_mag);
*(uint8_t *) addr_mag |= (1<<7);
//__raw_writeb(addr_mag, __raw_readb(addr_mag)|0x01);
while(1)
printk("writedata\n");
*/

/* 
	if(c == 1)
		*addr_mag |= (1<<4);
	else
		*addr_mag &= (~(1<<4));
*/

	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_GPIO_E), c);



//for(i = 8000; i > 0; i--)
//{
//printk("c %d\n",c);
//}
/*
while(1)
{
	for(i = 50000; i > 0; i--)
	NULL;
	*(int *) addr_mag |= (1<<4);
	for(i = 50000; i > 0; i--)
	NULL;
	*(int *) addr_mag &= (~(1<<4));
}*/
}

/*设置strobe引脚高低*/
static  void writeSTROBE(unsigned int c)
{
////		imx258_gpio_setpin(MAG_STROBE, c);	
int i;
/*
mxc_request_iomux(MX25_PIN_A13, MUX_CONFIG_ALT5);
    mxc_iomux_set_pad(MX25_PIN_A13, PAD_CTL_100K_PU|PAD_CTL_PKE_ENABLE);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_A13), "A13");
*/

//    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A13), c);  
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A13), c);

/*   
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_A13), NULL);
    mxc_free_iomux(MX25_PIN_A13, MUX_CONFIG_GPIO);
*/
}

/*读取data引脚数据*/
static unsigned int readDATA(void)
{
		unsigned int c = 1;

////		c = imx258_gpio_getpin(MAG_DATA) >> MAG_DATA_OFFSET;

//	*addr_mag |= (1<<5);


	if(((*addr_mag)&0x04) == 0)
		c = 0;
	else
		c = 1;
		
		return c;
}
/*******************************************************************************/


unsigned char HARDMARD_TrackGather(void)
{
		unsigned char ucI;
		unsigned char ucBStart;
		unsigned char ucTK_Bit;
		unsigned char ucTK_Byte;
		unsigned char ucChar;
		unsigned short uiI;

		memset(TK_Buff, 0, sizeof(TK_BUFF)*3);
		memset(TK_Data, 0, sizeof(TK_DATA)*3);

		for (ucI = 0; ucI <= 2; ucI++) {
				ucBStart  = 0;
				ucTK_Bit = 0;
				ucTK_Byte = 0;
				for (uiI = 0; uiI < MAX_TK_BITLen; uiI++) {
						if ( ucTK_Bit > 7 ) {
								ucTK_Bit = 0;
								ucTK_Byte++;
						}

						writeSTROBE(HIGH);
						ndelay(TstbH);
						writeSTROBE(LOW);

						if ( readDATA() == 0 ) {
								ucChar = TK_Buff[ucI].aucTKBuf[ucTK_Byte];
								ucChar |= BitMask[7-ucTK_Bit];
								TK_Buff[ucI].aucTKBuf[ucTK_Byte] = ucChar;
								ucBStart = 1;
						}
						if ( ucBStart ) {
								ucTK_Bit++;
						}
				}
		}
		uiI = 0;

		return 0;
}

void HARDMAG_CaclTKLen(int ucTK)
{
		unsigned char ucI;
		TK_BUFF *pTK_Buff;

		pTK_Buff = &TK_Buff[ucTK];

		for ( ucI = 0; ucI < MAX_TK_BUFLen; ucI++) {
				if ( (!pTK_Buff->aucTKBuf[ucI]) && (!pTK_Buff->aucTKBuf[ucI+1]) )
						break;
		}
		pTK_Buff->ucBufLen = ucI;
}

unsigned char HARDMAG_TKArrange(unsigned char ucFlag)
{
		unsigned char ucI, ucK;
		unsigned char ucTK1First;
		unsigned char ucTK2First;
		unsigned char ucTK3First;
		unsigned char ucTK1End;
		unsigned char ucTK2End;
		unsigned char ucTK3End;
		unsigned char aucTKEnd[3];

		ucTK1First = TK_Data[0].aucTKData[0];
		ucTK2First = TK_Data[1].aucTKData[0] & 0x0F;
		ucTK3First = TK_Data[2].aucTKData[0] & 0x0F;

		for (ucI = 0; ucI < 3; ucI++) {
				if (TK_Data[ucI].ucTKLen >= 2) {
						ucK = TK_Data[ucI].ucTKLen - 2;
						aucTKEnd[ucI] = TK_Data[ucI].aucTKData[ucK] & 0x0F;
				}
		}
		ucTK1End = aucTKEnd[0];
		ucTK2End = aucTKEnd[1];
		ucTK3End = aucTKEnd[2];

		if (ucTK2First == 0x0B) {
				if (ucTK2End == 0x0F) {
						return(HARDMAG_SUCCESS);
				} else if ( (ucTK1First == 0x45) || (ucTK3First == 0x0B) ) {
						return(HARDMAG_SUCCESS);
				} else if (ucFlag) {
						return(HARDMAG_SUCCESS);
				}
		} else {
				if (ucTK2End == 0x0F) {
						if ( (ucTK1First == 0x45) || (ucTK3First == 0x0B) )
								return(HARDMAG_SUCCESS);
						if (ucFlag) { /*正刷判断二磁道起始结束均错,反刷判断无起始有结束就认为磁道顺序正确*/
								return(HARDMAG_SUCCESS);
						}
				} else {
						if ( ( (ucTK1First == 0x45) && (ucTK1End == 0x0F) )
										|| ( (ucTK3First == 0x0B) && (ucTK1End == 0x0F) )
						   )
								return(HARDMAG_SUCCESS);
				}
		}
		return(HARDMAG_NOTRACK);
}
void HARDMAG_ResDecode(unsigned char ucLen, unsigned char *pInBuff)
{
		unsigned char ucInByte;
		unsigned char ucOutByte;
		unsigned char ucInBit;
		unsigned char ucOutBit;
		unsigned char ucBStart;
		unsigned char ucChar1 = 0;
		unsigned char ucChar2 = 0;
		unsigned short uiI, uiJ;
		unsigned char aucOutBuf[MAX_TK_BUFLen];

		ucInByte = 0;
		ucOutByte = 0;
		ucInBit = 0;
		ucOutBit = 0;
		ucBStart = 0;
		ucChar2 = 0;
		uiJ = ucLen * 8;
		memset(aucOutBuf, 0, sizeof(aucOutBuf));
		for (uiI = 0; uiI < uiJ; uiI++) {
				if (ucInBit > 7) {
						ucInBit = 0;
						ucLen--;
				}
				if (ucOutBit > 7) {
						ucOutBit = 0;
						ucChar2 = 0;
						ucOutByte++;
				}
				if ( ucInBit == 0 )
						ucChar1 = pInBuff[ucLen-1];
				if ((ucChar1&BitMask[ucInBit])) {
						ucChar2 |= BitMask[ 7-ucOutBit ];
						ucBStart = 1;
				}
				if ( ucOutBit == 7)
						aucOutBuf[ucOutByte] = ucChar2;

				if ( uiI == (uiJ - 1) )
						aucOutBuf[ucOutByte] = ucChar2;  //如果倒序置位，可能出现最后一次ucOutBit计数达不到7，根据循环结束来判断

				ucInBit++;

				if ( ucBStart ) {
						ucOutBit++;
				}
		}
		memcpy(pInBuff, aucOutBuf, MAX_TK_BUFLen);
}

void HARDMAG_Convert(unsigned char ucTK, unsigned char *pInBuff, unsigned char *pOutBuff )
{
		unsigned char ucInByte;
		unsigned char ucOutByte;
		unsigned char ucInBit;
		unsigned char ucOutBit;
		unsigned char ucBStart;
		unsigned char ucChar1 = 0;
		unsigned char ucChar2 = 0;
		unsigned char ucBitLen;
		unsigned short uiI;

		ucInByte = 0;
		ucOutByte = 0;
		ucInBit = 0;
		ucOutBit = 0;
		ucBStart = 0;
		ucChar2 = 0;

		if (ucTK) {
				ucBitLen = 4;
		} else {
				ucBitLen = 6;
		}
		for ( uiI = 0; uiI < MAX_TK_BUFLen*8; uiI++) {
				if (ucInBit > 7) {
						ucInBit = 0;
						ucInByte++;
				}
				if (ucOutBit > ucBitLen) {
						ucOutBit = 0;
						ucChar2 = 0;
						ucOutByte++;
				}
				if ( ucInBit == 0)
						ucChar1 = pInBuff[ucInByte];
				if ( ucChar1 & BitMask[ 7-ucInBit ] ) {
						ucChar2 |= BitMask[ucOutBit];
						ucBStart = 1;
				}
				if ( ucOutBit == ucBitLen ) {
						pOutBuff[ucOutByte] = ucChar2;
				}
				ucInBit++;
				if (ucBStart) {
						ucOutBit++;
				}
		}
}
unsigned char  HARDMAG_CheckSSES(unsigned char ucTK)
{
		unsigned char ucTKLen;
		unsigned char ucBStart;
		unsigned char ucFirstChar;
		unsigned char ucResult;

		ucResult = HARDMAG_SUCCESS;
		ucTKLen = TK_Data[ucTK].ucTKLen;
		if (ucTK) {
				ucBStart = 0x0B;
				ucFirstChar = TK_Data[ucTK].aucTKData[0] & 0x0F;
		} else {
				ucBStart = 0x45;
				ucFirstChar = TK_Data[ucTK].aucTKData[0];
		}
		if (ucFirstChar != ucBStart) {
				TK_Data[ucTK].ucErr = HARDMAG_NOSTART;
				ucResult = HARDMAG_NOTRACK;
		}
		if (ucTKLen >= 2) {
				if ((TK_Data[ucTK].aucTKData[ucTKLen-2]&0x0F) != 0x0F) {
						TK_Data[ucTK].ucErr = HARDMAG_NOEND;
						ucResult = HARDMAG_NOTRACK;
				}
		} else {
				ucResult = HARDMAG_NOTRACK;
		}
		return(ucResult);
}

unsigned char HARDMAG_TKVerify(unsigned char ucTK)
{
		unsigned char ucI, ucJ;
		unsigned char ucChar1;
		unsigned char ucChar2;
		unsigned char ucBitLen;
		unsigned char ucLRCVal;

		if (ucTK) {
				ucBitLen = 4;
				ucLRCVal = 0x0F;
		} else {
				ucBitLen = 6;
				ucLRCVal = 0x3F;
		}
		for ( ucI = 0; ucI < TK_Data[ucTK].ucTKLen; ucI++) {
				ucChar1 = 0;
				ucChar2 = TK_Data[ucTK].aucTKData[ucI];
				for (ucJ = 0; ucJ < ucBitLen; ucJ++) {
						ucChar1 ^= (ucChar2 & BitMask[ucJ]) >> ucJ;
				}
				ucChar2 = (ucChar2 & BitMask[ucBitLen]) >> ucBitLen;
				ucChar1 = (~ucChar1) & 0x01;
				if (ucChar1 != ucChar2) {
						TK_Data[ucTK].ucErr = HARDMAG_PAR;
						return(HARDMAG_PAR);
				}
		}
		ucChar1 = 0;
		for ( ucI = 0; ucI < TK_Data[ucTK].ucTKLen - 1; ucI++) {
				ucChar2 = TK_Data[ucTK].aucTKData[ucI] & ucLRCVal;
				ucChar1 = ucChar1 ^ ucChar2;
		}
		ucChar2 = TK_Data[ucTK].aucTKData[ucI] & ucLRCVal;
		if ( ucChar1 != ucChar2) {
				TK_Data[ucTK].ucErr = HARDMAG_LRC;
				return(HARDMAG_LRC);
		}
		return(HARDMAG_SUCCESS);
}

void HARDMAG_ConvAscii(unsigned char ucTK)
{
		unsigned char ucI, ucJ;
		unsigned char ucChar;

		if (ucTK) {
				for (ucI = 0; ucI < TK_Data[ucTK].ucTKLen; ucI++)
						TK_Data[ucTK].aucTKData[ucI] &= 0x0F;
		} else {
				for (ucI = 0; ucI < TK_Data[ucTK].ucTKLen; ucI++) {
						ucChar = TK_Data[ucTK].aucTKData[ucI];
						for (ucJ = 0; ucJ < 64; ucJ++) {
								if (ucChar == TKBINData[ucJ]) {
										TK_Data[ucTK].aucTKData[ucI] = TKASCData[ucJ] - 0x20;
								}
						}

				}
		}
}


void HARDMAG_TrackIntProc(void)
{
		unsigned char ucI, i;
		unsigned char ucResult=HARDMAG_SUCCESS;

		magnetic_track_gather();	//use 中断


		for (ucI = 0; ucI < 3; ucI++) {
				HARDMAG_CaclTKLen(ucI);
				if (TK_Buff[ucI].ucBufLen) {
						HARDMAG_Convert(ucI, TK_Buff[ucI].aucTKBuf, TK_Data[ucI].aucTKData);
						TK_Data[ucI].ucTKLen = strlen((char *)TK_Data[ucI].aucTKData);
				}
		}

		if ( (TK_Buff[0].ucBufLen == 0)
						&& (TK_Buff[1].ucBufLen == 0)
						&& (TK_Buff[2].ucBufLen == 0)
		   ) {
				ucResult = HARDMAG_NOTRACK;
		} else {
				if ( HARDMAG_TKArrange(0) ) {
						memset(&TK_Data, 0, sizeof(TK_DATA)*3);
						for (ucI = 0; ucI < 3; ucI++) {
								if (TK_Buff[ucI].ucBufLen) {
										HARDMAG_ResDecode(TK_Buff[ucI].ucBufLen, TK_Buff[ucI].aucTKBuf);
										HARDMAG_Convert(ucI, TK_Buff[ucI].aucTKBuf, TK_Data[ucI].aucTKData);
										TK_Data[ucI].ucTKLen = strlen((char *)TK_Data[ucI].aucTKData);
								}
						}
						ucResult = HARDMAG_TKArrange(1);
				}

				if (!ucResult) {
						for (ucI = 0; ucI < 3; ucI++) {
								if (TK_Data[ucI].ucTKLen) {
										if ( !HARDMAG_CheckSSES(ucI) )
												HARDMAG_TKVerify(ucI);
										HARDMAG_ConvAscii(ucI);
								}
						}
				} else {
						for (ucI=0; ucI<3; ucI++) {
								TK_Data[ucI].ucErr = ucResult;
								TK_Data[ucI].ucTKLen = 0;
						}
				}
		}


		//更新为可读字符
		for (ucI = 1; ucI < 3; ucI++) {
				for (i = 0; i < TK_Data[ucI].ucTKLen; i++)
				{
						printk("ISO %s: data=%d\n", __func__,TK_Data[ucI].aucTKData[i]);
						TK_Data[ucI].aucTKData[i] = TK_Data[ucI].aucTKData[i]  | '0';
				}
				//        printk("ISO %d: len=%d\n", ucI, TK_Data[ucI].ucTKLen);
		}

		printk("ISO %s: TK_Data[0].ucTKLen=%d\n", __func__,TK_Data[0].ucTKLen);

		for (i = 0; i < TK_Data[0].ucTKLen; i++)
		{
				TK_Data[0].aucTKData[i] += 0x20;
				printk("ISO %s: data=%d\n", __func__,TK_Data[0].aucTKData[i]);
		}
}

//函数功能：刷卡时反刷时用于将反刷的数据逐位倒序
//输入：*pInBuff 输入字符串首地址
//输出：*pOutBuff 输出字符串首地址
void TEST_HW_Reverse( unsigned char *pInBuff, unsigned char *pOutBuff )
{
		unsigned char  ucInByte, ucOutByte, ucInBit, ucOutBit, ucbStart, ucStart;
		unsigned long ulI, ulJ, ulK;
		unsigned int uiBreak;

		uiBreak = 0;
		for ( ulI = 0; ulI < 88; ulI++ ) {
				if ( (!pInBuff[ulI]) && (!pInBuff[ulI+1]) )
						break;
				uiBreak++;
		}

		ucInByte = 0;
		ucOutByte = 0;
		ucInBit = 0;
		ucOutBit = 0;
		ucbStart = 0;
		ucBuffer2 = 0;
		ucStart = 0;
		ulK = uiBreak * 8;

		for ( ulJ = 0; ulJ < ulK; ulJ++ ) {
				if (ucInBit > 7) {
						ucInBit = 0;
						uiBreak--;
				}
				if (ucOutBit > 7) {
						ucOutBit = 0;
						ucBuffer2 = 0;
						ucOutByte++;
				}
				if ( ucInBit == 0 )
						ucBuffer1 = pInBuff[uiBreak-1];
				if ((ucBuffer1&BitMask[ucInBit])) {
						ucBuffer2 |= BitMask[ 7-ucOutBit ];
						ucStart = 1;
				}
				if ( ucOutBit == 7)
						pOutBuff[ucOutByte] = ucBuffer2;

				if ( ulJ == (ulK - 1) )
						pOutBuff[ucOutByte] = ucBuffer2;  //è?1?μ1Dò????￡??é?ü3???×?oóò?′?ucOutBit??êy′?2?μ?7￡??ù?Y?-?·?áê?à′?D??

				ucInBit++;

				if ( ucStart ) {
						ucOutBit++;
				}
		}
}


void TEST_HW_Convert1( unsigned char *pInBuff, unsigned char *pOutBuff )
{
		unsigned char  ucInByte, ucOutByte, ucInBit, ucOutBit, ucbStart;
		unsigned long ulI;

		ucInByte = 0;
		ucOutByte = 0;
		ucInBit = 0;
		ucOutBit = 0;
		ucbStart = 0;
		ucBuffer2 = 0;

		for ( ulI = 0; ulI < (140*8); ulI++) {
				if (ucInBit > 7) {
						ucInBit = 0;
						ucInByte++;
				}
				if (ucOutBit > 6) {
						ucOutBit = 0;
						ucBuffer2 = 0;
						ucOutByte++;
				}

				if ( ucInBit == 0)
						ucBuffer1 = pInBuff[ucInByte];

				if ( ( ucBuffer1 & BitMask[ 7-ucInBit ] ) ) {
						ucBuffer2 |= BitMask[ucOutBit];
						ucbStart = 1;
				}

				if ( ucOutBit == 6 )
						pOutBuff[ucOutByte] = ucBuffer2;

				//printk("%c ", pOutBuff[ucOutByte]);
				if ( (ulI + 1) % 30 == 0 ) printk("\n");

				if ( (!pInBuff[ucInByte]) && (!pInBuff[ucInByte+1]) )//ê?·?ì?3??-?·òa?ú′|àío?êy?Yò?oó?D??￡?·??ò?é?ü?aêy?Y
						break;

				ucInBit++;
				if (ucbStart) {
						ucOutBit++;
				}

		}
}

void TEST_HW_Convert23( unsigned char *pInBuff, unsigned char *pOutBuff )
{
		unsigned char  ucInByte, ucOutByte, ucInBit, ucOutBit, ucbStart;
		unsigned long ulI;

		ucInByte = 0;
		ucOutByte = 0;
		ucInBit = 0;
		ucOutBit = 0;
		ucbStart = 0;
		ucBuffer2 = 0;

		for ( ulI = 0; ulI < (140*8); ulI++) {
				if (ucInBit > 7) {
						ucInBit = 0;
						ucInByte++;
				}
				if (ucOutBit > 4) {
						ucOutBit = 0;
						ucBuffer2 = 0;
						ucOutByte++;
				}

				if ( ucInBit == 0)
						ucBuffer1 = pInBuff[ucInByte];

				if ( ( ucBuffer1 & BitMask[ 7-ucInBit ] ) ) {
						ucBuffer2 |= BitMask[ucOutBit];
						ucbStart = 1;
				}
				if ( ucOutBit == 4 )
						pOutBuff[ucOutByte] = ucBuffer2;

				if ( (!pInBuff[ucInByte]) && (!pInBuff[ucInByte+1]) )//ê?·?ì?3??-?·òa?ú′|àío?êy?Yò?oó?D??￡?·??ò?é?ü?aêy?Y
						break;

				ucInBit++;
				if (ucbStart) {
						ucOutBit++;
				}
		}
}

//oˉêy1|?ü￡o1′?μàD￡?éêy?Yμ??yè·D?
//ê?è?￡o*pInBuff ê?è?×?·?′?ê×μ??·
//·μ??￡oD￡?é?á1?￡oMGHW_SUCCESS￡?MGHW_ODDERR	￡?MGHW_LRCERR
unsigned char TEST_HW_Verify1( unsigned char *pInBuff )
{
		unsigned long ulI, ulNum;

		ulNum = 0;
		for ( ulI = 0; ulI < 140; ulI++) {
				ucBuffer1 = pInBuff[ulI];

				if ( (ulI > 0) && (pInBuff[ulI-2] == 0x1F) )
						break;
				else
						ulNum++;	   //??êyμ?LRC×??ú

				if ( (( ucBuffer1 & BitMask[0] ) ^ (( ucBuffer1 & BitMask[1] ) >> 1) ^ ( (ucBuffer1 & BitMask[2]) >> 2 ) ^
										(( ucBuffer1 & BitMask[3] ) >> 3) ^ (( ucBuffer1 & BitMask[4] ) >> 4) ^
										(( ucBuffer1 & BitMask[5] ) >> 5) ^ (( ucBuffer1 & BitMask[6] ) >> 6)) );
				else
						return MGHW_ODDERR;

		}
		ucBuffer1 = pInBuff[0] & 0x3F;
		for ( ulI = 0; ulI < 140; ulI++) {
				if ( ulI < (ulNum - 2) ) {
						ucBuffer2 = pInBuff[ulI+1] & 0x3F;
						ucBuffer1 = ucBuffer1 ^ ucBuffer2;
				} else {
						if ( ucBuffer1 != (pInBuff[ulNum-1]&0x3F) )
								return 	MGHW_LRCERR;
						else
								break;
				}
		}
		return MGHW_SUCCESS;
}

//oˉêy1|?ü￡o2￡?3′?μàD￡?éêy?Yμ??yè·D?
//ê?è?￡o*pInBuff ê?è?×?·?′?ê×μ??·
//·μ??￡oD￡?é?á1?￡oMGHW_SUCCESS￡?MGHW_ODDERR	￡?MGHW_LRCERR
unsigned char TEST_HW_Verify23( unsigned char *pInBuff )
{
		unsigned long ulI, ulNum;

		ulNum = 0;
		for ( ulI = 0; ulI < 140; ulI++) {
				ucBuffer1 = pInBuff[ulI];
				if ( (ulI > 0) && (pInBuff[ulI-2] == 0x1F) )
						break;
				else
						ulNum++;       //??êyμ?LRC×??ú
				if ( (( ucBuffer1 & BitMask[0] ) ^ (( ucBuffer1 & BitMask[1] ) >> 1) ^ ( (ucBuffer1 & BitMask[2]) >> 2 ) ^
										(( ucBuffer1 & BitMask[3] ) >> 3) ^ (( ucBuffer1 & BitMask[4] ) >> 4)) );
				else
						return MGHW_ODDERR;

		}
		ucBuffer1 = pInBuff[0] & 0x0F;
		for ( ulI = 0; ulI < 140; ulI++) {
				if ( ulI < (ulNum - 2) ) {
						ucBuffer2 = pInBuff[ulI+1] & 0x0F;
						ucBuffer1 = ucBuffer1 ^ ucBuffer2;
				} else {
						if ( ucBuffer1 != (pInBuff[ulNum-1]&0x0F) )
								return 	MGHW_LRCERR;
						else
								break;
				}
		}
		return MGHW_SUCCESS;
}

//1′?μà×aASCII??
unsigned long TEST_HW_ASCIIConvert1( unsigned char *pInBuff, unsigned char *pOutBuff  )
{
		unsigned long ulI, ulJ, ulLen;

		ulLen = 0;
		for ( ulI = 0; ulI < 140; ulI++) {
				ucBuffer1 = pInBuff[ulI];
				if ( (ulI > 0) && (pInBuff[ulI-1] == 0x1F) ) {
						pOutBuff[ulI] = pInBuff[ulI];
						ulLen++;
						break;
				} else {
						for ( ulJ = 0; ulJ < 64; ulJ++) {
								if ( ucBuffer1 == TrackOneData[ulJ] ) {
										pOutBuff[ulI] = TrackOneASCII[ulJ];
										break;
								}
						}
						ulLen++;
				}
		}
		return ulLen;
}

//23′?μà×aASCII??
unsigned long TEST_HW_ASCIIConvert23( unsigned char *pInBuff, unsigned char *pOutBuff )
{
		unsigned long ulI, ulLen;

		ulLen = 0;
		for ( ulI = 0; ulI < 140; ulI++) {
				ucBuffer1 = pInBuff[ulI];
				if ( (ulI > 0) && (pInBuff[ulI-1] == 0x1F) ) {
						pOutBuff[ulI] = pInBuff[ulI];
						ulLen++;
						break;
				} else {
						ucBuffer1 &= 0x0F;
						ucBuffer1 |= 0x30;
						pOutBuff[ulI] = ucBuffer1;
						ulLen++;
				}
		}
		return ulLen;
}




/********************************************************************************
 * 			MAG设备 标准操作
 ********************************************************************************/
static int imx258_mag_open(struct inode *inode, struct file *filp)
{

		struct magreader_dev *dev; /* device information */

		dev = container_of(inode->i_cdev, struct magreader_dev, cdev);
		filp->private_data = dev; /* for other methods */

#ifdef tmp_dbg
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
		memset(dev->data, 0, 100);
		sprintf(dev->data, "%s", __func__);
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
#endif

#if 0
		/* now trim to 0 the length of the device if open was write-only */
		if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
				if (down_interruptible(&dev->sem))
						return -ERESTARTSYS;
				up(&dev->sem);
		}
#endif
		//        printk("%s data val=%s\n", __func__, dev->data);

		return 0;          /* success */
}

static int imx258_mag_release(struct inode *inode, struct file *file)
{

		return 0;
}


ssize_t imx258_mag_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
{
		struct magreader_dev *dev = filp->private_data;
		int retval = 0;

		/*
		   if (down_interruptible(&dev->sem))
		   return -ERESTARTSYS;
		 */

#ifdef tmp_dbg
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
		memset(dev->data, 0, 100);
		sprintf(dev->data, "%s", __func__);
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);        //printk("You can read data from me 1!dev->kmalloc_area->gen_status=%x\n", dev->kmalloc_area->gen_status);
#endif

		wait_event_interruptible(dev->wait, dev->kmalloc_area->gen_status == DRV_ENDED);

#ifdef tmp_dbg
		printk("You can read data from me 2!dev->kmalloc_area->gen_status=%x\n", dev->kmalloc_area->gen_status);


		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
		memset(dev->data, 0, 100);
		sprintf(dev->data, "%s", __func__);
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
#endif

		if (copy_to_user(buf, dev->kmalloc_area, sizeof(DRV_OUT))) {
				printk("failed to do copy_to_user!\n");
				retval = -EFAULT;
				goto nothing;
		}

		dev->kmalloc_area->gen_status = DRV_RUNNING;
nothing:
		//up (&dev->sem);
		return retval;
}

/*kernel<---user*/
ssize_t imx258_mag_write(struct file *filp, const char *buf, size_t count, loff_t *ppos)
{
		struct magreader_dev *dev = filp->private_data;

		wake_up_interruptible(&dev->wait);

		return 0;
}

static int imx258_mag_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
		int err = 0, ret, ucI, i, len;
		ucI = i = len = 0;

		/*
		 * extract the type and number bitfields, and don't decode
		 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
		 */
		if (_IOC_TYPE(cmd) != IMX258_MAG_IOCTL_BASE) return -ENOTTY;
		if (_IOC_NR(cmd) > IMX258_MAG_IOC_MAXNR) return -ENOTTY;

		/*
		 * the direction is a bitmask, and VERIFY_WRITE catches R/W
		 * transfers. `Type' is user-oriented, while
		 * access_ok is kernel-oriented, so the concept of "read" and
		 * "write" is reversed
		 */
		if (_IOC_DIR(cmd) & _IOC_READ)
				err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
		else if (_IOC_DIR(cmd) & _IOC_WRITE)
				err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
		if (err) return -EFAULT;

		switch (cmd) {
				case MAG_SET_CFG:
						ret = copy_from_user((OSMAGCFG *)&MagCfg, (OSMAGCFG *)arg, sizeof(OSMAGCFG));

						if (mag_dbg) {
								printk("MAG_SET_CFG info:\n");
								printk("dbg: %x\n", MagCfg.ucMagDebug);
								printk("ver: %s\n", MagCfg.ucMagVer);
						}
						mag_dbg = MagCfg.ucMagDebug;

						memset(MagCfg.ucMagVer, 0, 5);
						memcpy(MagCfg.ucMagVer, SAND_MAG_VERSION, strlen(SAND_MAG_VERSION));

						ret = copy_to_user( (OSMAGCFG *)arg, (OSMAGCFG *)&MagCfg,  sizeof(OSMAGCFG));
						break;

				default:
						return -EINVAL;
		}

		return 0;
}



static void magnetic_track_gather(void)
{
		char i = 0;
		long timeout = 90000;
reset:
		writeSTROBE(HIGH);
		while ( !readDATA() && timeout-- ) {
				i++;
		};

		writeSTROBE(LOW);
		timeout = 90000;
		i = 0;

		while ( readDATA() && timeout-- ) {
				i++;
				if (timeout==1) goto reset;	//timeout?
		};

		for (i = 0; i < 16; i++) {
				writeSTROBE(HIGH);
				udelay(TstbH);
				writeSTROBE(LOW);
				udelay(TstbL);
		}

		printk("-------%s--------\n", __func__);

		HARDMARD_TrackGather();

}


static irqreturn_t  imx258_mag_interrupt(int irq, void *dev_id)
{
int i;
	
//	long	flag;

printk("in  interrupt\n");
		struct magreader_dev *dev=(struct magreader_dev *)dev_id;

		spin_lock(&dev->lock);



////		imx258_gpio_cfgpin(MAG_DATA, IMX258_GPG3_INP);
////		imx258_gpio_pullup(MAG_DATA, 0);
	
//		*addr_mag &= (~(1<<4));

		//free_irq(IOMUX_TO_IRQ(MX25_PIN_GPIO_E),dev);
		mxc_iomux_set_pad(MX25_PIN_GPIO_E, PAD_CTL_PKE_NONE);

//free_irq(IOMUX_TO_IRQ(MX25_PIN_GPIO_E), magptr);   
    
    


HARDMAG_TrackIntProc();
//#ifdef tmp_dbg
printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
memset(dev->data, 0, 100);
sprintf(dev->data, "%s", __func__);
printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
//#endif

		tasklet_schedule(&dev->tlet);

		spin_unlock(&dev->lock);
printk("out  interrupt\n");
		return IRQ_HANDLED;
}

void mag_swiptimeout_fn(unsigned long arg)
{
		struct magreader_dev *dev=(struct magreader_dev *)arg;
		dev->kmalloc_area->gen_status = DRV_ABSENT;
		DRV_OUT* mag=dev->kmalloc_area;
		unsigned char ucmaglen2 = mag->xxdata[0];
    	unsigned char ucmaglen3 = mag->xxdata[ucmaglen2+2];
    	unsigned char ucmaglen1 = mag->xxdata[ucmaglen2+2+ucmaglen3+2];

    	mag->drv_status=DRV_ABSENT;
    	mag->xxdata[ucmaglen2+1]=DRV_ABSENT;
    	mag->xxdata[ucmaglen2+2+ucmaglen3+1]=DRV_ABSENT;
		
		flag=1;
		wake_up_interruptible(&dev->wait);
		del_timer(&dev->mag_swiptimeout);
}

void mag_tasklet_fn(unsigned long arg)
{
		int i, len=0;
		unsigned short uiOffset;
		struct magreader_dev *dev=(struct magreader_dev *)arg;
		unsigned long j = jiffies;

		printk("123magptr addr=%p, %s,%d\n", dev, __func__, __LINE__);
////		flash_led(0, 0x02);

		if (mag_dbg>1) {
				printk("********* TRACK 1 ucErr=%x ucLen=%d****************\n",   TK_Data[0].ucErr,TK_Data[0].ucTKLen );
				for (i = 0; i <= TK_Data[0].ucTKLen - 3; i++) {
						printk("%c ", TK_Data[0].aucTKData[i]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");

				printk("********* TRACK 2 ucErr=%x ucLen=%d****************\n",   TK_Data[1].ucErr,TK_Data[1].ucTKLen );
				for (i = 0; i <= TK_Data[1].ucTKLen - 3; i++) {
						printk("%c ", TK_Data[1].aucTKData[i]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");

				printk("********* TRACK 3 ucErr=%x ucLen=%d****************\n",   TK_Data[2].ucErr,TK_Data[2].ucTKLen );
				for (i = 0; i <= TK_Data[2].ucTKLen - 3; i++) {
						printk("%c ", TK_Data[2].aucTKData[i]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");
		}

#if 1
		/*
		 *[	for direct access between app and kernal space. ]
		 *1. failed with write directly.
		 *2. try use new_fifo function
		 *3. try use real map
		 */
		//memset(kmalloc_area, 0, (NPAGES + 2) * PAGE_SIZE);
		memset(dev->kmalloc_area, 0, sizeof(DRV_OUT));

		dev->kmalloc_area->gen_status = DRV_RUNNING;
		dev->kmalloc_area->reqnr = 0;

		/*
		 * 如果存在3/1磁道，则赋值；不存在，则设置长度为0
		 * 磁道顺序为 2/3/1
		 */

		dev->kmalloc_area->drv_status = TK_Data[1].ucErr;
		uiOffset = 0;
		if (TK_Data[1].ucTKLen >= 3) {
				dev->kmalloc_area->xxdata[uiOffset++] =  TK_Data[1].ucTKLen - 3;
				memcpy(&dev->kmalloc_area->xxdata[uiOffset], &TK_Data[1].aucTKData[1], TK_Data[1].ucTKLen-3);
				uiOffset +=  TK_Data[1].ucTKLen - 3;
		} else {
				dev->kmalloc_area->xxdata[uiOffset++] =  0;
		}



		dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[2].ucErr; //3    // 3磁道状态
		if (TK_Data[2].ucTKLen >= 3) {
				dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[2].ucTKLen - 3; //3    // 3磁道长度
				memcpy(&dev->kmalloc_area->xxdata[uiOffset], &TK_Data[2].aucTKData[1], TK_Data[2].ucTKLen - 3);
				uiOffset += TK_Data[2].ucTKLen-3;
		} else {
				dev->kmalloc_area->xxdata[uiOffset++]=0; //3    // 3磁道长度
		}



		dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[0].ucErr; //1    // 1磁道状态
		if (TK_Data[0].ucTKLen >= 3) {
				dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[0].ucTKLen - 3;; //1    // 1磁道长度
				memcpy(&dev->kmalloc_area->xxdata[uiOffset], &TK_Data[0].aucTKData[1], TK_Data[0].ucTKLen - 3);
		} else {
				dev->kmalloc_area->xxdata[uiOffset++]=0; //3    // 3磁道长度
		}




		if (mag_dbg) {
				uiOffset=0;
				len= dev->kmalloc_area->xxdata[uiOffset++];
				printk("************ TRACK 2(%d, uiOffset=%d)***************\n",len, uiOffset);
				for (i = 1; i <=len ; i++) {
						printk("%c ", dev->kmalloc_area->xxdata[uiOffset++]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");

				printk("************ TRACK 3( x uiOffset=%d err:%d, len:%d, )***************\n",
								uiOffset, dev->kmalloc_area->xxdata[uiOffset++], dev->kmalloc_area->xxdata[uiOffset++]);
				len= dev->kmalloc_area->xxdata[uiOffset-1];

				for (i = 1; i <= len; i++) {
						printk("%c ", dev->kmalloc_area->xxdata[uiOffset++]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");


				printk("************ TRACK 1(uiOffset=%d err:%d,len: %d, )***************\n",
								uiOffset, dev->kmalloc_area->xxdata[uiOffset++], dev->kmalloc_area->xxdata[uiOffset++]);
				len= dev->kmalloc_area->xxdata[uiOffset-1];
				for (i = 1; i <= len; i++) {
						printk("%c ", dev->kmalloc_area->xxdata[uiOffset++]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");

		}
		//zengshu 20110127
		del_timer(&dev->mag_swiptimeout);
		dev->mag_swiptimeout.expires=j+60;
		dev->mag_swiptimeout.function = mag_swiptimeout_fn;
		dev->mag_swiptimeout.data = (struct magreader_dev *)dev;
		add_timer(&dev->mag_swiptimeout);

		//zy 20100428
		//to remove static induction
		if ( (TK_Data[1].ucTKLen>2 && TK_Data[1].ucErr==0x00)
						||  (TK_Data[2].ucTKLen>2 && TK_Data[2].ucErr==0x00)
						|| (TK_Data[0].ucTKLen>2 && TK_Data[0].ucErr==0x00)) {
				del_timer(&dev->mag_swiptimeout);
				dev->kmalloc_area->gen_status = DRV_ENDED;
				flag=1;
				wake_up_interruptible(&dev->wait);
		} 

#endif

		if (mag_dbg)
				printk("************ Len/Err = %02x/%02x  %02x/%02x %02x/%02x gen_status=%x****************\n",
								TK_Data[0].ucTKLen,TK_Data[0].ucErr, TK_Data[1].ucTKLen,TK_Data[1].ucErr, TK_Data[2].ucTKLen,TK_Data[2].ucErr,
								dev->kmalloc_area->gen_status);
		MSR_reset(dev);
#if 0
		if (mag_timer_status==0) {
				dev->mag_timer.expires=j+mag_tdelay;
				dev->mag_timer.function = mag_timer_fn;
				add_timer(&dev->mag_timer);
		}
#endif
////		flash_led(1, 0x02);
}


/*****************************************************************************
  how about real mmap to share var between kernal and app space?
  zy 0720
 *****************************************************************************/
void simple_vma_open(struct vm_area_struct *vma)
{
		memset(magptr->kmalloc_area, 0, (((unsigned long)kmalloc_ptr) + PAGE_SIZE - 1) & PAGE_MASK);
		//printk("%s\n", __func__);
}

void simple_vma_close(struct vm_area_struct *vma)
{
		//printk("%s\n", __func__);
}


/*
 * The remap_pfn_range version of mmap.  This one is heavily borrowed
 * from drivers/char/mem.c.
 */

static struct vm_operations_struct simple_remap_vm_ops = {
		.open =  simple_vma_open,
		.close = simple_vma_close,
};

static int imx258_mag_mmap(struct file *filp, struct vm_area_struct *vma)
{
		struct magreader_dev *dev=filp->private_data;
		int ret;
		long length = vma->vm_end - vma->vm_start;


		/* check length - do not allow larger mappings than the number of
		   pages allocated */
		if (length > NPAGES * PAGE_SIZE)
				return -EIO;

		vma->vm_ops = &simple_remap_vm_ops;

		/* map the whole physically contiguous area in one piece */
		if ((ret = remap_pfn_range(vma,
										vma->vm_start,
										virt_to_phys((void *)dev->kmalloc_area) >> PAGE_SHIFT,
										length,
										vma->vm_page_prot)) < 0) {
				return ret;
		}


		simple_vma_open(vma);
		return 0;

}
/****************************************************************************/

static struct file_operations imx258_mag_fops = {
		.owner =     THIS_MODULE,
		.llseek =    no_llseek,
		.read =imx258_mag_read,
		.write = imx258_mag_write,
		.ioctl =     imx258_mag_ioctl,
		.open =	     imx258_mag_open,
		.release =   imx258_mag_release,
		.mmap = imx258_mag_mmap,
};


static int MSR_reset(struct magreader_dev *dev)
{
		int err;
int i;
printk("reset 123\n");
////		imx258_gpio_cfgpin(MAG_STROBE, IMX258_GPH10_OUTP);
////		imx258_gpio_cfgpin(MAG_DATA, IMX258_GPG3_OUTP);


////		imx258_gpio_pullup(MAG_DATA, 1);
////		imx258_gpio_pullup(MAG_STROBE, 1);


    
//    gpio_request(IOMUX_TO_GPIO(MX25_PIN_A13), NULL);
//    mxc_free_iomux(MX25_PIN_A13, MUX_CONFIG_GPIO);
		
//		*addr_mag |= (1<<4);
	
		//OE = ENABLE;
		*addr_mag |= (1<<5);

		ndelay(100);


		/*初始化硬解码电路，注意时序*/
		writeSTROBE(HIGH);
		writeDATA(HIGH);
		ndelay(Tsu1_rst);

		writeDATA(LOW);
		ndelay(Tsu2_rst);

		writeSTROBE(LOW);//数据低
		ndelay(TstbL);

		writeSTROBE(HIGH);
		ndelay(TstbH);


		writeSTROBE(LOW);
		ndelay(Tsu_mode);

		writeDATA(HIGH);
		ndelay(Th_mode);

		ndelay(TstbL-Th_mode-Tsu_mode);

		writeSTROBE(HIGH);
		ndelay(TstbH2);

		writeSTROBE(LOW);
		ndelay(TstbL);

		writeSTROBE(HIGH);
		ndelay(TstbH);

		writeSTROBE(LOW);
		ndelay(100);//Tsu2_rst 4u


		//设置数据端口为中断方式(IMX258_GPG3)
////		imx258_gpio_cfgpin(MAG_DATA, IMX258_GPG3_EINT11);

////		imx258_gpio_pullup(MAG_DATA, 0);


//		*addr_mag &= (~(1<<4));

		//OE = DISABLE;
		*addr_mag &= (~(1<<5));

////		set_irq_type(IRQ_EINT11, IRQT_FALLING );	//IRQT_LOW, IRQT_FALLING,IRQF_SHARED,SA_INTERRUPT
//free_irq(IOMUX_TO_IRQ(MX25_PIN_GPIO_E),dev);
  
	  //gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_CSPI1_MOSI), 0);     

	  mxc_iomux_set_pad(MX25_PIN_GPIO_E, PAD_CTL_PKE_NONE);
	  gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_E));  
      set_irq_type(IOMUX_TO_IRQ(MX25_PIN_GPIO_E), IRQF_TRIGGER_LOW);

#ifdef tmp_dbg
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
		memset(dev->data, 0, 100);
		sprintf(dev->data, "%s", __func__);
		printk("magptr addr=%p, %s,%d, data=%s\n", dev, __func__, __LINE__, dev->data);
#endif
		
		err = request_irq(IOMUX_TO_IRQ(MX25_PIN_GPIO_E), imx258_mag_interrupt, 0, DEVICE_NAME,dev); 
////		err = request_irq(IRQ_EINT11, imx258_mag_interrupt, 0, DEVICE_NAME, dev);
		if (err ) {
				printk("mag driver: %s request_irq failed, err=%d\n", __func__, err); //why always show this message?
				return 0;
		}

		return 0;
}

void mag_timer_fn(unsigned long arg)
{
		struct magreader_dev *dev=(struct magreader_dev *)arg;
		int uiOffset=0, len, i=0;
		unsigned long j = jiffies;

#if 0
		if (dev->kmalloc_area->gen_status != DRV_ENDED)
				return;
#endif

		if (mag_timer_status++>mag_times /*&& dev->kmalloc_area->gen_status == DRV_ENDED*/) {
				printk("remove timer!!!!!!!!!!!!!\n");
				del_timer(&dev->mag_timer);
				mag_timer_status=0;
				// dev->kmalloc_area->gen_status = DRV_RUNNING;
				return;
		}

		//		if (mag_dbg)
		printk("************ %s %p times=%d***************\n",__func__, dev, mag_timer_status);


		dev->kmalloc_area->gen_status = DRV_ENDED;
		dev->kmalloc_area->reqnr = 0;

		/*
		 * 如果存在3/1磁道，则赋值；不存在，则设置长度为0
		 * 磁道顺序为 2/3/1
		 */

		dev->kmalloc_area->drv_status = TK_Data[1].ucErr;
		uiOffset = 0;
		if (TK_Data[1].ucTKLen >= 3) {
				dev->kmalloc_area->xxdata[uiOffset++] =  TK_Data[1].ucTKLen - 3;
				memcpy(&dev->kmalloc_area->xxdata[uiOffset], &TK_Data[1].aucTKData[1], TK_Data[1].ucTKLen-3);
				uiOffset +=  TK_Data[1].ucTKLen - 3;
		} else {
				dev->kmalloc_area->xxdata[uiOffset++] =  0;
		}



		dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[2].ucErr; //3    // 3磁道状态
		if (TK_Data[2].ucTKLen >= 3) {
				dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[2].ucTKLen - 3; //3    // 3磁道长度
				memcpy(&dev->kmalloc_area->xxdata[uiOffset], &TK_Data[2].aucTKData[1], TK_Data[2].ucTKLen - 3);
				uiOffset += TK_Data[2].ucTKLen-3;
		} else {
				dev->kmalloc_area->xxdata[uiOffset++]=0; //3    // 3磁道长度
		}



		dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[0].ucErr; //1    // 1磁道状态
		if (TK_Data[0].ucTKLen >= 3) {
				dev->kmalloc_area->xxdata[uiOffset++]=TK_Data[0].ucTKLen - 3;; //1    // 1磁道长度
				memcpy(&dev->kmalloc_area->xxdata[uiOffset], &TK_Data[0].aucTKData[1], TK_Data[0].ucTKLen - 3);
		} else {
				dev->kmalloc_area->xxdata[uiOffset++]=0; //3    // 3磁道长度
		}




		if (mag_dbg) {
				uiOffset=0;
				len= dev->kmalloc_area->xxdata[uiOffset++];

				printk("************ TRACK 2(%d, uiOffset=%d)***************\n",len, uiOffset);
				for (i = 1; i <=len ; i++) {
						printk("%c ", dev->kmalloc_area->xxdata[uiOffset++]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");

				printk("************ TRACK 3( x uiOffset=%d err:%d, len:%d, )***************\n",
								uiOffset, dev->kmalloc_area->xxdata[uiOffset++], dev->kmalloc_area->xxdata[uiOffset++]);
				len= dev->kmalloc_area->xxdata[uiOffset-1];

				for (i = 1; i <= len; i++) {
						printk("%c ", dev->kmalloc_area->xxdata[uiOffset++]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");


				printk("************ TRACK 1(uiOffset=%d err:%d,len: %d, )***************\n",
								uiOffset, dev->kmalloc_area->xxdata[uiOffset++], dev->kmalloc_area->xxdata[uiOffset++]);
				len= dev->kmalloc_area->xxdata[uiOffset-1];
				for (i = 1; i <= len; i++) {
						printk("%c ", dev->kmalloc_area->xxdata[uiOffset++]);
						if ((i+1)%20==0) printk("\n");
				}
				printk("\n");

		}


		dev->mag_timer.expires=j+mag_tdelay;
		dev->mag_timer.function = mag_timer_fn;
		add_timer(&dev->mag_timer);
}



static int __init imx258_mag_init(void)
{
		int ret, i;
		dev_t dev = 0;
		unsigned long j = jiffies;

//		addr_mag = (unsigned long )ioremap(0xb2800000, 1);
		addr_mag = (unsigned char *)ioremap(0xb2800000, 1);

		*addr_mag = 0;
		printk(" 0*addr_mag = %x \n", *addr_mag);

		mdelay(100);

		*addr_mag |= (1<<7);
		printk(" 1*addr_mag = %x \n", *addr_mag);
		
		*addr_mag &= (0xFC);
		printk(" 2*addr_mag = %X \n", *addr_mag);


		/*
		 * Get a range of minor numbers to work with, asking for a dynamic
		 * major unless directed otherwise at load time.
		 */
		if (mag_major) {
				dev = MKDEV(mag_major, mag_minor);
				ret = register_chrdev_region(dev, 1, DEVICE_NAME);
		} else {
				ret = alloc_chrdev_region(&dev, mag_minor, 1, DEVICE_NAME);
				mag_major = MAJOR(dev);
		}

		if (ret < 0) {
				printk(KERN_WARNING "!!!!!! mag driver: can't get major %d\n", mag_major);
				return ret;
		}

		magptr = (struct magreader_dev *)kmalloc( (sizeof(struct magreader_dev)+sizeof(DRV_OUT))*PAGE_SIZE, GFP_KERNEL);
		//magptr = kmalloc( (sizeof(struct magreader_dev)+1024), GFP_KERNEL);
		if (!magptr) {
				ret = -ENOMEM;
				goto fail_malloc;
		}
		memset(magptr, 0, sizeof(struct magreader_dev) );        
		memset(magptr->data, 0, 100);
		memcpy(magptr->data, "mag test data", strlen("mag test data"));


		cdev_init(&magptr->cdev, &imx258_mag_fops);
		magptr->cdev.owner = THIS_MODULE;
		magptr->cdev.ops = &imx258_mag_fops;
		ret = cdev_add(&magptr->cdev, dev, 1);
		if (ret) {
				printk(KERN_NOTICE "add cdev error.\n");
				goto fail_add;
		}


		/* allocate a memory area with kmalloc. Will be rounded up to a page boundary */
		if ((kmalloc_ptr = kmalloc((NPAGES + 2) * PAGE_SIZE, GFP_KERNEL)) == NULL) {
				ret = -ENOMEM;
				goto fail_add;
		}
		/* round it up to the page bondary */
		magptr->kmalloc_area = (DRV_OUT *)((((unsigned long)kmalloc_ptr) + PAGE_SIZE - 1) & PAGE_MASK);

		/* mark the pages reserved */
		for (i = 0; i < NPAGES * PAGE_SIZE; i+= PAGE_SIZE) {
				SetPageReserved(virt_to_page(((DRV_OUT  *)magptr->kmalloc_area) + i));
		}

		magptr->mag_class=class_create(THIS_MODULE, "mag");
		if (IS_ERR(magptr->mag_class)) {
				printk("Err: failed in creating class.\n");
				return -1;
		}

		/* register your own device in sysfs, and this will cause udevd to create corresponding device node */
		device_create(magptr->mag_class, NULL, MKDEV(mag_major, mag_minor), NULL, DEVICE_NAME );



		init_waitqueue_head (&magptr->wait);
		tasklet_init(&magptr->tlet, mag_tasklet_fn, (unsigned long)magptr);
		spin_lock_init(&magptr->lock);

		printk("<1>complex IMX258 MAG 6541  Driver %d:%d v" SAND_MAG_VERSION " mag_dbg=%d mag_tdelay=%d, mag_times=%d\n",
						mag_major, mag_minor, mag_dbg, mag_tdelay,mag_times);

		//config CPLD_INT
	mxc_free_iomux(MX25_PIN_GPIO_E, MUX_CONFIG_GPIO);
	  mxc_request_iomux(MX25_PIN_GPIO_E, MUX_CONFIG_FUNC);
      mxc_iomux_set_pad(MX25_PIN_GPIO_E, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU);
    //  gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_E));  
	gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_GPIO_E), 1);

	printk("----------%s,%d----------\n",__func__,__LINE__);  

	//config STROBE
	mxc_free_iomux(MX25_PIN_A13, MUX_CONFIG_GPIO);
	mxc_request_iomux(MX25_PIN_A13, MUX_CONFIG_ALT5);
    mxc_iomux_set_pad(MX25_PIN_A13, PAD_CTL_100K_PU|PAD_CTL_PKE_ENABLE);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_A13), "A13");
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A13), 1);  
	

	printk("----------%s,%d----------\n",__func__,__LINE__);

	return 0 ;

		MSR_reset(magptr);

		init_timer(&(magptr->mag_swiptimeout));		//zengshu 20110127

		if (mag_tdelay) {
				init_timer(&magptr->mag_timer);
				magptr->mag_timer.expires=j+mag_tdelay;
				magptr->mag_timer.data = (struct magreader_dev *)magptr;
				magptr->mag_timer.function = mag_timer_fn;
				add_timer(&magptr->mag_timer);
		}

		return 0 ;


fail_add:
		unregister_chrdev_region(dev, 1);

fail_malloc:
		return ret;

		return 0;
}

static void __exit imx258_mag_exit(void)
{
////		free_irq(IRQ_EINT11, magptr);
		free_irq(IOMUX_TO_IRQ(MX25_PIN_GPIO_E), magptr);
		cdev_del(&magptr->cdev);
		unregister_chrdev_region(MKDEV (mag_major, mag_minor), 1);
		device_destroy(magptr->mag_class, MKDEV(mag_major, mag_minor));
		class_destroy(magptr->mag_class);
		kfree(magptr);
}

module_init(imx258_mag_init);
module_exit(imx258_mag_exit);

MODULE_AUTHOR("zhangyong <zhangyong@sand.cn>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MagCard 6541 driver for Samsung IMX258 MPU");
