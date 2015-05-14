#ifndef __S3C2410_LCD_T6963__
#define __S3C2410_LCD_T6963__

#include <asm/memory.h>

#define DRIVER_VERSION		"0.1"
#define LCD192_MINOR 254
#define DEVICE_NAME	"s3c2410-lcd-yxd"
static int LCD_YXD_Major = 0;


#define S3C2410_LCD_IOCTL_BASE	'Z'

typedef struct
{
    //unsigned char cs;
    //unsigned char val;
    int cs;
    int val;
}lcd192_struct;

#define S3C2410_LCD_CMD_POWER		_IOW(S3C2410_LCD_IOCTL_BASE, 0, sizeof(lcd192_struct))
#define S3C2410_LCD_CMD_STARTLINE		_IOW(S3C2410_LCD_IOCTL_BASE, 1, sizeof(lcd192_struct))
#define S3C2410_LCD_CMD_STARTXADDR		_IOW(S3C2410_LCD_IOCTL_BASE, 2, sizeof(lcd192_struct))
#define S3C2410_LCD_CMD_STARTYADDR		_IOW(S3C2410_LCD_IOCTL_BASE, 3, sizeof(lcd192_struct))
#define S3C2410_LCD_DATA_WR		_IOW(S3C2410_LCD_IOCTL_BASE, 4, sizeof(lcd192_struct))
#define S3C2410_LCD_DATA_RD		_IOR(S3C2410_LCD_IOCTL_BASE, 5, sizeof(lcd192_struct))
#define S3C2410_LCD_STAUTE		_IOR(S3C2410_LCD_IOCTL_BASE, 6, sizeof(lcd192_struct))
#define S3C2410_LCD_DBG			_IOR(S3C2410_LCD_IOCTL_BASE, 7, sizeof(lcd192_struct))




/******** 常量 ***********************************************/
#define LCD192_CHECKBUSY 1000
#define LCD192_LCDSUCESS 0x00
#define LCD192_LCDBUSY	0x01

#define LCD_EN 1
#define LCD_DIS 0

#define LCD_PORT  vLCD_YXD_BASE //		//根据电路图确定 0x10000000的虚拟地址
#define LCD_RD_PORT LCD_PORT & (1<<1)
#define LCD_WR_PORT LCD_PORT

#define LCD192_BASE_CS1 vLCD_YXD_BASE
#define LCD192_BASE_CS2 vLCD_YXD_BASE+0x0400
#define LCD192_BASE_CS3 vLCD_YXD_BASE+0x0800

#define LCD192_RWH_DIH		3	//[1:0]11
#define LCD192_RWH_DIL		2	//10
#define LCD192_RWL_DIH		1	//01
#define LCD192_RWL_DIL		0	//00

#define RW_BITS	0x01
#define RS_BITS	0x00

#define HAL192_POWERON		0x3F		/*打开显示*/
#define HAL192_POWEROFF		0x3E		/*关闭显示*/

#define HAL192_STARTLINE	0xC0	//LCD_STARTLINE + actual line
#define HAL192_STARTPAGE	0xB8	//LCD_STARTPAGE + actual page
#define HAL192_STARTYADDR	0x40	//LCD_STARTYADDR + actual y addr


#define HAL192_BACKLIGHT_OFF 0x01
#define HAL192_BACKLIGHT_ON 0x00




#define STATUSBIT_BUSY(v)	( (v)>>7 ) & 0x1
#define LCD_STATUS_BUSY		0x1		//1 内部操作
#define LCD_STATUS_BUSY_OK	0x0	//0 准备就绪

#define STATUSBIT_ON(v)	( (v)>>5 ) & 0x1
#define LCD_STATUS_ON	0x1		//1 显示开
#define LCD_STATUS_OFF	0x0		//0 显示关

#define STATUSBIT_RST(v)	( (v)>>4 ) & 0x1
#define LCD_STATUS_RST	0x1		//  1复位
#define LCD_STATUS_RST_NORMAL	0x0	//0正常


#endif

