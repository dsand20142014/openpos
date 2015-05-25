#include "splc501c.h"
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
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h>	//for request_irq
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>

#include <asm/io.h> /* for virt_to_phys */
#include <linux/slab.h> /* for kmalloc and kfree */

#include <linux/interrupt.h>

#include <linux/timer.h>

#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/irqs.h>


#include <linux/types.h>
#include <mach/gpio.h>
#include "../../arch/arm/mach-mx25/mx25_pins.h"
#include "osdriver.h"

#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include "../../arch/arm/mach-mx25/iomux.h"
#include <asm/uaccess.h> 

#define SPLC501C_MAJOR 179
#define SPLC501C_MINOR 3

#define LCD_INITNB      18

char ucLCDUsedFlag;
char *graph_buffer;

extern unsigned int spi_lcd128_write(unsigned int reg_val);
extern struct semaphore sem;
//extern spinlock_t spilock;

#define IMX258_SPLC501_IOCTL_BASE	'C'
#define SPLC501_ZI _IOW(IMX258_SPLC501_IOCTL_BASE, 0, char *)
#define SPLC501_ASCII _IOW(IMX258_SPLC501_IOCTL_BASE, 1, char *)
#define SPLC501_BMP _IOW(IMX258_SPLC501_IOCTL_BASE, 2, char *)
#define SPLC501_CLR _IOW(IMX258_SPLC501_IOCTL_BASE, 3, char *)
#define SPLC501_ASCII5x7 _IOW(IMX258_SPLC501_IOCTL_BASE, 4, char *)
#define SPLC501_LCDON _IOW(IMX258_SPLC501_IOCTL_BASE, 5, char *)
#define SPLC501_LCDOFF _IOW(IMX258_SPLC501_IOCTL_BASE, 6, char *)


const unsigned char aucLCDInitTab[LCD_INITNB]=
{
	0xE2,		//DB	11100010B		;RESET
	0x2C,		//DB	00101100B
	0x2E,
	0x2F,		//DB	00101111B		;SET POWER CONTROL
	0xA3,		//DB	10100011B		;SET LCD BIAS
	0x24,		//DB	00100100B		;SET V5 VOLTAGE RATIO
//	0x23,		//DB	00100100B		;SET V5 VOLTAGE RATIO
	0x81,		//DB	10000001B		;SET ELECTRONIC VOLUME MODE
	0x2C,		//DB	00110010B		;SET ELECTRONIC VOLUME REGISTER
//	0x22,		//DB	00110010B		;SET ELECTRONIC VOLUME REGISTER
	0x00,		//DB	00000000B		;INITIAL DISPLAY LINE
	0xA0,		//DB	10100000B		;ADC SELECT
//123	0xC0,		//DB	11000000B		;COMMON OUTPUT MODE	
	0xC8,		//DB	11000000B		;COMMON OUTPUT MODE
	0xA2,		//DB	10100010B		;DISPLAY ALL POINTS OFF
	0xA6,		//DB	10100110B		;NORMAL DISPLAY
	0xAD,       //                      ;STATIC INDICATOR
	0x00,
//	0xA5,       //                      ;PAGE BLANK
//	0x00,
	0xAF,		//DB	10101111B		;DISPLAY ON
	0xAF,		//DB	10101111B
	0x03		//DB	00000011B
};

ICONDEF IconSingalDef[] =
{
	{0,  	"\x03\x00"},
	{1,  	"\x03\x05\x00"},
	{2,  	"\x03\x05\x07\x00"},
	{3,  	"\x03\x05\x07\x09\x00"},
	{4,  	"\x03\x05\x07\x09\x0B\x00"},
	{5,  	"\x03\x05\x07\x09\x0B\x0D\x00"},
	{0xFF,  0},
};

ICONDEF IconTelephoneDef[] =
{
	{0,  	"\x12\x00"},
	{0xFF,  0},
};

ICONDEF IconMessageDef[] =
{
	{0,  	"\x1C\x00"},
	{0xFF,  0},
};

ICONDEF IconMagCardDef[] =
{
	{0,  	"\x25\x00"},
	{0xFF,  0},
};

ICONDEF IconICCCardDef[] =
{
	{0,  	"\x2F\x00"},
	{0xFF,  0},
};

ICONDEF IconBatteryDef[] =
{
	{0,  	"\x7B\x00"},
	{1,  	"\x7B\x7D\x00"},
	{2,  	"\x7B\x7D\x7C\x00"},
	{3,  	"\x7B\x7D\x7C\x7E\x00"},
	{4,  	"\x7B\x7D\x7C\x7E\x7F\x00"},
	{0xFF,  0},
};

ICONTBL IconTable[LCDICONTYPE_MAXNB] =
{
	{LCDICONTYPE_SIGNAL,		IconSingalDef,		"\x03\x05\x07\x09\x0B\x0D\x00"},
	{LCDICONTYPE_TELEPHONE,		IconTelephoneDef,	"\x12\x00"},
	{LCDICONTYPE_MESSAGE,		IconMessageDef,		"\x1C\x00"},
	{LCDICONTYPE_MAGCARD,		IconMagCardDef,		"\x25\x00"},
	{LCDICONTYPE_ICCCARD,		IconICCCardDef,		"\x2F\x00"},
	{LCDICONTYPE_BATTERY,		IconBatteryDef,		"\x7B\x7D\x7C\x7E\x7F\x00"},
};

ICONDEF Date1Def[] =
{
	{0,  	"\x32\x33\x35\x36\x37\x38\x00"},
	{1,  	"\x37\x38\x00"},
	{2,  	"\x32\x38\x34\x35\x36\x00"},
	{3,  	"\x32\x38\x34\x37\x36\x00"},
	{4,  	"\x33\x34\x38\x37\x00"},
	{5,  	"\x32\x33\x34\x37\x36\x00"},
	{6,  	"\x32\x33\x35\x36\x37\x34\x00"},
	{7,  	"\x32\x38\x37\x00"},
	{8,  	"\x32\x33\x34\x37\x36\x35\x38\x00"},
	{9,  	"\x32\x33\x34\x38\x37\x36\x00"},
	{0xFF,  0},
};

ICONDEF Date2Def[] =
{
	{0,  	"\x39\x3A\x3C\x3D\x3E\x3F\x00"},
	{1,  	"\x3E\x3F\x00"},
	{2,  	"\x39\x3F\x3B\x3C\x3D\x00"},
	{3,  	"\x39\x3F\x3B\x3E\x3D\x00"},
	{4,  	"\x3A\x3B\x3F\x3E\x00"},
	{5,  	"\x39\x3A\x3B\x3E\x3D\x00"},
	{6,  	"\x39\x3A\x3C\x3D\x3E\x3B\x00"},
	{7,  	"\x39\x3F\x3E\x00"},
	{8,  	"\x39\x3A\x3B\x3E\x3D\x3C\x3F\x00"},
	{9,  	"\x39\x3A\x3B\x3F\x3E\x3D\x00"},
	{0xFF,  0},
};

ICONDEF Date3Def[] =
{
	{0,  	"\x41\x42\x44\x45\x46\x47\x00"},
	{1,  	"\x46\x47\x00"},
	{2,  	"\x41\x47\x43\x44\x45\x00"},
	{3,  	"\x41\x47\x43\x46\x45\x00"},
	{4,  	"\x42\x43\x47\x46\x00"},
	{5,  	"\x41\x42\x43\x46\x45\x00"},
	{6,  	"\x41\x42\x44\x45\x46\x43\x00"},
	{7,  	"\x41\x47\x46\x00"},
	{8,  	"\x41\x42\x43\x46\x45\x44\x47\x00"},
	{9,  	"\x41\x42\x43\x47\x46\x45\x00"},
	{0xFF,  0},
};

ICONDEF Date4Def[] =
{
	{0,  	"\x48\x49\x4B\x4C\x4D\x4E\x00"},
	{1,  	"\x4D\x4E\x00"},
	{2,  	"\x48\x4E\x4A\x4B\x4C\x00"},
	{3,  	"\x48\x4E\x4A\x4D\x4C\x00"},
	{4,  	"\x49\x4A\x4E\x4D\x00"},
	{5,  	"\x48\x49\x4A\x4D\x4C\x00"},
	{6,  	"\x48\x49\x4B\x4C\x4D\x4A\x00"},
	{7,  	"\x48\x4E\x4D\x00"},
	{8,  	"\x48\x49\x4A\x4D\x4C\x4B\x4E\x00"},
	{9,  	"\x48\x49\x4A\x4E\x4D\x4C\x00"},
	{0xFF,  0},
};

ICONDEF Date5Def[] =
{
	{0,  	"\x50\x51\x53\x54\x55\x56\x00"},
	{1,  	"\x55\x56\x00"},
	{2,  	"\x50\x56\x52\x53\x54\x00"},
	{3,  	"\x50\x56\x52\x55\x54\x00"},
	{4,  	"\x51\x52\x56\x55\x00"},
	{5,  	"\x50\x51\x52\x55\x54\x00"},
	{6,  	"\x50\x51\x53\x54\x55\x52\x00"},
	{7,  	"\x50\x56\x55\x00"},
	{8,  	"\x50\x51\x52\x55\x54\x53\x56\x00"},
	{9,  	"\x50\x51\x52\x56\x55\x54\x00"},
	{0xFF,  0},
};

ICONDEF Date6Def[] =
{
	{0,  	"\x57\x58\x5A\x5B\x5C\x5D\x00"},
	{1,  	"\x5C\x5D\x00"},
	{2,  	"\x57\x5D\x59\x5A\x5B\x00"},
	{3,  	"\x57\x5D\x59\x5C\x5B\x00"},
	{4,  	"\x58\x59\x5D\x5C\x00"},
	{5,  	"\x57\x58\x59\x5C\x5B\x00"},
	{6,  	"\x57\x58\x5A\x5B\x5C\x59\x00"},
	{7,  	"\x57\x5D\x5C\x00"},
	{8,  	"\x57\x58\x59\x5C\x5B\x5A\x5D\x00"},
	{9,  	"\x57\x58\x59\x5D\x5C\x5B\x00"},
	{0xFF,  0},
};

ICONDEF Date7Def[] =
{
	{0,  	"\x5E\x5F\x61\x62\x63\x64\x00"},
	{1,  	"\x63\x64\x00"},
	{2,  	"\x5E\x64\x60\x61\x62\x00"},
	{3,  	"\x5E\x64\x60\x63\x62\x00"},
	{4,  	"\x5F\x60\x64\x63\x00"},
	{5,  	"\x5E\x5F\x60\x63\x62\x00"},
	{6,  	"\x5E\x5F\x61\x62\x63\x60\x00"},
	{7,  	"\x5E\x64\x63\x00"},
	{8,  	"\x5E\x5F\x60\x63\x62\x61\x64\x00"},
	{9,  	"\x5E\x5F\x60\x64\x63\x62\x00"},
	{0xFF,  0},
};

ICONDEF Date8Def[] =
{
	{0,  	"\x65\x66\x68\x69\x6A\x6B\x00"},
	{1,  	"\x6A\x6B\x00"},
	{2,  	"\x65\x6B\x67\x68\x69\x00"},
	{3,  	"\x65\x6B\x67\x6A\x69\x00"},
	{4,  	"\x66\x67\x6B\x6A\x00"},
	{5,  	"\x65\x66\x67\x6A\x69\x00"},
	{6,  	"\x65\x66\x68\x69\x6A\x67\x00"},
	{7,  	"\x65\x6B\x6A\x00"},
	{8,  	"\x65\x66\x67\x6A\x69\x68\x6B\x00"},
	{9,  	"\x65\x66\x67\x6B\x6A\x69\x00"},
	{0xFF,  0},
};

ICONDEF Date9Def[] =
{
	{0,  	"\x6D\x6E\x70\x71\x72\x73\x00"},
	{1,  	"\x72\x73\x00"},
	{2,  	"\x6D\x73\x6F\x70\x71\x00"},
	{3,  	"\x6D\x73\x6F\x72\x71\x00"},
	{4,  	"\x6E\x6F\x73\x72\x00"},
	{5,  	"\x6D\x6E\x6F\x72\x71\x00"},
	{6,  	"\x6D\x6E\x70\x71\x72\x6F\x00"},
	{7,  	"\x6D\x73\x72\x00"},
	{8,  	"\x6D\x6E\x6F\x72\x71\x70\x73\x00"},
	{9,  	"\x6D\x6E\x6F\x73\x72\x71\x00"},
	{0xFF,  0},
};

ICONDEF Date10Def[] =
{
	{0,  	"\x74\x75\x77\x78\x79\x7A\x00"},
	{1,  	"\x79\x7A\x00"},
	{2,  	"\x74\x7A\x76\x77\x78\x00"},
	{3,  	"\x74\x7A\x76\x79\x78\x00"},
	{4,  	"\x75\x76\x7A\x79\x00"},
	{5,  	"\x74\x75\x76\x79\x78\x00"},
	{6,  	"\x74\x75\x77\x78\x79\x76\x00"},
	{7,  	"\x74\x7A\x79\x00"},
	{8,  	"\x74\x75\x76\x79\x78\x77\x7A\x00"},
	{9,  	"\x74\x75\x76\x7A\x79\x78\x00"},
	{0xFF,  0},
};

ICONDEF DateSep1Def[] =
{
	{0,		"\x40\x00"},
	{0xFF,	0},
};

ICONDEF DateSep2Def[] =
{
	{0,		"\x4F\x00"},
	{0xFF,	0},
};

ICONDEF DateSep3Def[] =
{
	{0,		"\x6C\x00"},
	{0xFF,	0},
};

ICONTBL IconDateTable[LCDDATE_LEN] =
{
	{0,	Date1Def,	"\x32\x33\x34\x35\x36\x37\x38\x00"},
	{1,	Date2Def,	"\x39\x3A\x3B\x3C\x3D\x3E\x3F\x00"},
	{2,	DateSep1Def,"\x40\x00"},
	{3,	Date3Def,	"\x41\x42\x43\x44\x45\x46\x47\x00"},
	{4,	Date4Def,	"\x48\x49\x4A\x4B\x4C\x4D\x4E\x00"},
	{5,	DateSep2Def,"\x4F\x00"},
	{6,	Date5Def,	"\x50\x51\x52\x53\x54\x55\x56\x00"},
	{7,	Date6Def,	"\x57\x58\x59\x5A\x5B\x5C\x5D\x00"},
	{8,	Date7Def,	"\x5E\x5F\x60\x61\x62\x63\x64\x00"},
	{9,	Date8Def,	"\x65\x66\x67\x68\x69\x6A\x6B\x00"},
	{10,DateSep3Def,"\x6C\x00"},
	{11,Date9Def,	"\x6D\x6E\x6F\x70\x71\x72\x73\x00"},
	{12,Date10Def,	"\x74\x75\x76\x77\x78\x79\x7A\x00"},
};

ICONTBL IconDateTable2[LCDDATE_LEN2] =
{
	{0,	Date1Def,	"\x32\x33\x34\x35\x36\x37\x38\x00"},
	{1,	Date2Def,	"\x39\x3A\x3B\x3C\x3D\x3E\x3F\x00"},
	{2,	Date3Def,	"\x41\x42\x43\x44\x45\x46\x47\x00"},
	{3,	Date4Def,	"\x48\x49\x4A\x4B\x4C\x4D\x4E\x00"},
	{4,	Date5Def,	"\x50\x51\x52\x53\x54\x55\x56\x00"},
	{5,	Date6Def,	"\x57\x58\x59\x5A\x5B\x5C\x5D\x00"},
	{6,	Date7Def,	"\x5E\x5F\x60\x61\x62\x63\x64\x00"},
	{7,	Date8Def,	"\x65\x66\x67\x68\x69\x6A\x6B\x00"},
	{8, Date9Def,	"\x6D\x6E\x6F\x70\x71\x72\x73\x00"},
	{9, Date10Def,	"\x74\x75\x76\x77\x78\x79\x7A\x00"},
};

unsigned char aucOldDate[LCDDATE_LEN2];


unsigned char HALLCD_WriteCMDChip(unsigned char ucCmd)
{
	down(&sem);
	//spin_lock(&spilock);

	//LCD_EN
	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);

	//CSPI1_SS0	
	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), 0);	
	
	spi_lcd128_write(ucCmd);
	
	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), 1);
    up(&sem);
	//spin_unlock(&spilock);
}

#define isdigit(c)	('0' <= (c) && (c) <= '9')

unsigned char HALLCD_Init(void)
{
	unsigned char ucI;
	
	int i=0;
	
	//258
	//LCD_PWR_EN
	mxc_free_iomux(MX25_PIN_A14, MUX_CONFIG_ALT5);
	mxc_request_iomux(MX25_PIN_A14, MUX_CONFIG_ALT5);
	mxc_iomux_set_pad(MX25_PIN_A14, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_A14), "A14");
	gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A14), 0);
	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 0);
	
	
	//CSPI1_SS0
	mxc_free_iomux(MX25_PIN_CSPI1_SS0, MUX_CONFIG_ALT5);
	mxc_request_iomux(MX25_PIN_CSPI1_SS0, MUX_CONFIG_ALT5);
	mxc_iomux_set_pad(MX25_PIN_CSPI1_SS0, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), "ss0");
	gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), 1);
	
	//LCD_EN
	mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT5);
	mxc_request_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT5);
	mxc_iomux_set_pad(MX25_PIN_VSTBY_ACK, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), "ack");
	gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

	ucLCDUsedFlag = 0;

	HALLCD_WriteCMDChip(aucLCDInitTab[0]);
	for(ucI=0;ucI<18;ucI++)
	{
		HALLCD_WriteCMDChip(aucLCDInitTab[ucI]);
	}
	
	memset(aucOldDate,0xFF,sizeof(aucOldDate));

	return(HALLCD_ClearAddon());
}



unsigned char HALLCD_ClearAll(void)
{
	unsigned char ucI;

	for(ucI=0;ucI<8;ucI++)
	{
		HALLCD_ClearLine(ucI);
	}
	return(HALLCD_SUCCESS);
}


unsigned char HALLCD_Displayascii(unsigned char ucFlag,unsigned char ucY,unsigned char ucX,unsigned char *aucLineDotBuf)
{
    
    int i = 0;
	unsigned char ucRow;
	unsigned char ucOldCh;
	unsigned char ucMask;
	unsigned char ucDot;
	unsigned char ucI,ucJ;
	unsigned char ucStartX;
	unsigned char ucCh;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	unsigned char *pucPtr;
        
	ucY = (4 - 1) - ucY;
	ucX = ucX*8;
	ucY = ucY*16;
	
    ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;

	HALLCD_WriteCMDChip(ucRow);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
	
	
	pucPtr = aucLineDotBuf+8;
   
	for(i = 0 ;i < 8; i++)
	{
		HALLCD_WriteData(*(pucPtr));
		pucPtr++;
	}
	
	ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;
	
	HALLCD_WriteCMDChip(ucRow - 1);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
	pucPtr = aucLineDotBuf;
	for(i = 0 ;i < 8; i++)
	{
		HALLCD_WriteData(*(pucPtr));
		pucPtr++;
	}
	
	return(HALLCD_SUCCESS);
}

unsigned char HALLCD_Displayascii5x7(unsigned char ucFlag,unsigned char ucY,unsigned char ucX,unsigned char *aucLineDotBuf)
{
    
    int i = 0;
	unsigned char ucRow;
	unsigned char ucOldCh;
	unsigned char ucMask;
	unsigned char ucDot;
	unsigned char ucI,ucJ;
	unsigned char ucStartX;
	unsigned char ucCh;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	unsigned char *pucPtr;

	ucY = (8 - 1) - ucY;
	ucX = ucX*6;
	ucY = ucY*8;
	
    ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;
	HALLCD_WriteCMDChip(ucRow);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
		
	pucPtr = aucLineDotBuf;
   
	for(i = 0 ;i < 5; i++)
	{
		HALLCD_WriteData(*(pucPtr));
		pucPtr++;
	}
	
	return(HALLCD_SUCCESS);
}

unsigned char flip(unsigned char x)
{  
	x = (x & 0x0f) << 4 | (x & 0xf0) >>4;  x = (x & 0x33) << 2 | (x & 0xcc) >>2;  
	x = (x & 0x55) << 1 | (x & 0xaa) >>1;  
	return x;
}

void HALDOT_RotateDot(
		unsigned char *pucDotPtr,
		unsigned char ucBitNo,
		unsigned char *pucCh)
{
	unsigned char ucCh;

	ucCh = ((*pucDotPtr << ucBitNo) & 0x80) >> 7;
	ucCh |= ((*(pucDotPtr+1) << ucBitNo) & 0x80) >> 6;
	ucCh |= ((*(pucDotPtr+2) << ucBitNo) & 0x80) >> 5;
	ucCh |= ((*(pucDotPtr+3) << ucBitNo) & 0x80) >> 4;
	ucCh |= ((*(pucDotPtr+4) << ucBitNo) & 0x80) >> 3;
	ucCh |= ((*(pucDotPtr+5) << ucBitNo) & 0x80) >> 2;
	ucCh |= ((*(pucDotPtr+6) << ucBitNo) & 0x80) >> 1;
	ucCh |= (*(pucDotPtr+7) << ucBitNo) & 0x80;

	*pucCh = ucCh;
}

unsigned char HALLCD_Displaybmp(unsigned char ucFlag,unsigned char ucY,unsigned char ucX,unsigned char *aucLineDotBuf1)
{    
    int i = 0;
    int j = 0;
	unsigned char ucRow;
	unsigned char ucOldCh;
	unsigned char ucMask;
	unsigned char ucDot;
	unsigned char ucI,ucJ;
	unsigned char ucStartX;
	unsigned char ucCh;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	unsigned char *pucPtr;
	unsigned char aucLineDotBuf[][16] = 
{
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}
	};
	int lie, hang;
	int k;
	char disc;
	unsigned char rotate;
	int m = 0;

	ucX = 0;
	ucY = 0;
	ucX = ucX*16;
	ucY = ucY*16;
printk("tu-----------\n");
	ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;


#if 1//横向取模1维点阵
//aucLineDotBuf[8*((8 - 1 - j)) +  0到7][0] 组成i  0到7
//aucLineDotBuf[8*((8 - 1 - j)) + 0到7][1] 组成i  8到15
				
	for(j = 0; j < 8; j++)
	{
	
		HALLCD_WriteCMDChip(ucRow - 1 + j - 6);
		HALLCD_WriteCMDChip(ucColHigh);
		HALLCD_WriteCMDChip(ucColLow);
		pucPtr = aucLineDotBuf;   
		
		pucPtr = aucLineDotBuf+128*j;   

		unsigned char temp;
		unsigned char temp2;
		for(i = 0; i < 128; i++)
		{			
			rotate = 0;
			temp = 0;

		 	for(m = 0; m < 8; m++)
		 	{
		 	temp = aucLineDotBuf[(8*(j) +  m)*16 + i/8];
			temp2 = ((temp&(0x01<<(7 - (i%8))))<<(i%8))>>(7 - m);
			rotate =rotate + temp2;
			}

			HALLCD_WriteData(rotate);
		}
	}
	return(HALLCD_SUCCESS);
#endif

#if 1//纵向取模
	for(j = 0; j < 8; j++)
	{
	
		HALLCD_WriteCMDChip(ucRow - 1 + j - 4);
		HALLCD_WriteCMDChip(ucColHigh);
		HALLCD_WriteCMDChip(ucColLow);
		pucPtr = aucLineDotBuf;   
		
		pucPtr = aucLineDotBuf+128*(8 - 1 - j);   
		for(i = 0; i < 128; i++)
		{
			HALLCD_WriteData(*(pucPtr+i));
		}
	
	}
	return(HALLCD_SUCCESS);
#endif
    ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;

	HALLCD_WriteCMDChip(ucRow - 1);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
	pucPtr = aucLineDotBuf+31;   
	for(i = 0 ;i < 16; i++)
	{
		HALLCD_WriteData(*(pucPtr-15));
		pucPtr++;
	}
	
	pucPtr = aucLineDotBuf+15;
	ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;

	HALLCD_WriteCMDChip(ucRow);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
	for(i = 0 ;i < 16; i++)
	{
		HALLCD_WriteData(*(pucPtr-15));
		pucPtr++;
	}
	return(HALLCD_SUCCESS);
}


unsigned char HALLCD_Display8DotInCol(unsigned char ucFlag,unsigned char ucY,unsigned char ucX,unsigned char *aucLineDotBuf)
{    
    int i = 0;
	unsigned char ucRow;
	unsigned char ucOldCh;
	unsigned char ucMask;
	unsigned char ucDot;
	unsigned char ucI,ucJ;
	unsigned char ucStartX;
	unsigned char ucCh;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	unsigned char *pucPtr;
	
	ucX = ucX*8;
	ucY = ucY*16;
    ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;

	HALLCD_WriteCMDChip(ucRow);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
	pucPtr = aucLineDotBuf+31;   
	for(i = 0 ;i < 16; i++)
	{
		HALLCD_WriteData(*(pucPtr-15));
		pucPtr++;
	}
	
	pucPtr = aucLineDotBuf+15;
	ucRow = ucY/8;
	ucRow &= 0x07;
	ucRow = 0x07-ucRow;
	ucRow |= 0xB0;
	ucJ = ucY%8;
	ucColHigh = ((ucX & 0xF0) >> 4)+0x10;
	ucColLow = ucX & 0x0F;

	HALLCD_WriteCMDChip(ucRow - 1);
	HALLCD_WriteCMDChip(ucColHigh);
	HALLCD_WriteCMDChip(ucColLow);
	for(i = 0 ;i < 16; i++)
	{
		HALLCD_WriteData(*(pucPtr-15));
		pucPtr++;
	}
	return(HALLCD_SUCCESS);
}
unsigned char HALLCD_DisplayDateIcon(unsigned char clean)
{
	unsigned char ucI;
	unsigned char ucCh;
	unsigned char ucData;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	
	if(clean)
		ucData = 0x00;
	else
		ucData = 0xff;
		
	for(ucI=0;DateSep3Def[0].aucSEG[ucI];ucI++)
	{
		ucCh = DateSep3Def[0].aucSEG[ucI];
		ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
		ucColLow = ucCh & 0x0F;
		HALLCD_WriteCMDChip(0xB8);
		HALLCD_WriteCMDChip(ucColHigh);
		HALLCD_WriteCMDChip(ucColLow);
		HALLCD_WriteData(ucData);
	}	
}

unsigned char HALLCD_DisplayDate(unsigned char *pucDate)
{
	unsigned char ucI;
	unsigned char ucCh;
	unsigned char ucData;
	unsigned char ucColHigh;
	unsigned char ucColLow;

	if( !pucDate )
		return(LCDDRVERR_BADPARAM);

	HALLCD_Lock(1);
	ucData = 0xFF;
	for(ucI=0;DateSep1Def[0].aucSEG[ucI];ucI++)
	{
		ucCh = DateSep1Def[0].aucSEG[ucI];
		ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
		ucColLow = ucCh & 0x0F;
		HALLCD_WriteCMDChip(0xB8);
		HALLCD_WriteCMDChip(ucColHigh);
		HALLCD_WriteCMDChip(ucColLow);
		HALLCD_WriteData(ucData);
	}
	for(ucI=0;DateSep2Def[0].aucSEG[ucI];ucI++)
	{
		ucCh = DateSep2Def[0].aucSEG[ucI];
		ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
		ucColLow = ucCh & 0x0F;
		HALLCD_WriteCMDChip(0xB8);
		HALLCD_WriteCMDChip(ucColHigh);
		HALLCD_WriteCMDChip(ucColLow);
		HALLCD_WriteData(ucData);
	}
	for(ucI=0;DateSep3Def[0].aucSEG[ucI];ucI++)
	{
		ucCh = DateSep3Def[0].aucSEG[ucI];
		ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
		ucColLow = ucCh & 0x0F;
		HALLCD_WriteCMDChip(0xB8);
		HALLCD_WriteCMDChip(ucColHigh);
		HALLCD_WriteCMDChip(ucColLow);
		HALLCD_WriteData(ucData);
	}
	HALLCD_DisplayNewDate(pucDate);
	HALLCD_Unlock();
	return(HALLCD_SUCCESS);
}
unsigned char HALLCD_DisplayNewDate(unsigned char *pucDate)
{
	unsigned char ucI,ucJ;
	unsigned char ucCh;
	unsigned char ucData;
	ICONDEF *pIconPtr;
	unsigned char *pucIconSeg;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	unsigned char aucDate[LCDDATE_LEN2];

	if( !pucDate )
		return(LCDDRVERR_BADPARAM);

	memcpy(aucDate,pucDate,LCDDATE_LEN2);
	if( aucDate[6] == 0)
		aucDate[6] = 0xFF;
	if( !memcmp(aucOldDate,aucDate,LCDDATE_LEN2) )
		return(HALLCD_SUCCESS);

	for( ucI=0;ucI<LCDDATE_LEN2;ucI++)
	{
		if( aucOldDate[ucI]== aucDate[ucI] )
			continue;

		pIconPtr = IconDateTable2[ucI].pIconDef;
		pucIconSeg = IconDateTable2[ucI].aucIconSEG;
		if( !pIconPtr )
			continue;
		ucData = 0x00;
		if( pucIconSeg )
		{
			for(ucJ=0;*(pucIconSeg+ucJ);ucJ++)
			{
				ucCh = *(pucIconSeg+ucJ);
				ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
				ucColLow = ucCh & 0x0F;
				HALLCD_WriteCMDChip(0xB8);
				HALLCD_WriteCMDChip(ucColHigh);
				HALLCD_WriteCMDChip(ucColLow);
				HALLCD_WriteData(ucData);
			}
		}
		ucData = 0xFF;
		ucCh = aucDate[ucI];
		for( ucJ=0;(pIconPtr+ucJ)->ucIconCmd != 0xFF;ucJ++)
		{
			if( (pIconPtr+ucJ)->ucIconCmd == ucCh )
			{
				pucIconSeg = (pIconPtr+ucJ)->aucSEG;
				if( pucIconSeg )
				{
					for(ucJ=0;*(pucIconSeg+ucJ);ucJ++)
					{
						ucCh = *(pucIconSeg+ucJ);
						ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
						ucColLow = ucCh & 0x0F;
						HALLCD_WriteCMDChip(0xB8);
						HALLCD_WriteCMDChip(ucColHigh);
						HALLCD_WriteCMDChip(ucColLow);
						HALLCD_WriteData(ucData);
					}
				}
				break;
			}
		}
		aucOldDate[ucI] = aucDate[ucI];
	}

	return(HALLCD_SUCCESS);
}

unsigned char HALLCD_ClearLine(unsigned char ucLineNb)
{
	unsigned char ucI;

	ucLineNb &= 0x07;
	ucLineNb = 0x07-ucLineNb;
	ucLineNb |= 0xB0;

	HALLCD_Lock(1);
	//CN: 设置页地址
	HALLCD_WriteCMDChip(ucLineNb);
	//CN: 设置列地址
	HALLCD_WriteCMDChip(0x10);
	HALLCD_WriteCMDChip(0x00);

	for(ucI=0;ucI<128;ucI++)
	{
		HALLCD_WriteData(0x00);
	}
	HALLCD_Unlock();

	return(HALLCD_SUCCESS);
}



unsigned char HALLCD_ClearAddon(void)
{
	unsigned char ucI;

	HALLCD_Lock(1);
	//CN: 设置页地址
	HALLCD_WriteCMDChip(0xB8);
	//CN: 设置列地址
	HALLCD_WriteCMDChip(0x10);
	HALLCD_WriteCMDChip(0x00);

	for(ucI=0;ucI<128;ucI++)
	{
		HALLCD_WriteData(0x00);
	}

	HALLCD_Unlock();
	
	return(HALLCD_SUCCESS);
}


unsigned char HALLCD_ReadData(unsigned char *pucData)
{

}

unsigned char HALLCD_WriteData(unsigned char ucData)
{
	down(&sem);
	//spin_lock(&spilock);

	//LCD_EN
	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
	
	//CSPI1_SS0
	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), 0);	
	
	spi_lcd128_write(ucData);
	
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), 1);
	up(&sem);  
	//spin_unlock(&spilock);
	return(HALLCD_CheckBusy());
}

unsigned char HALLCD_CheckBusy(void)
{
	return HALLCD_SUCCESS;
}

unsigned char HALLCD_Lock(unsigned char ucWaitFlag)
{
	return(HALLCD_SUCCESS);
}

unsigned char HALLCD_Unlock(void)
{
	return(HALLCD_SUCCESS);
}

unsigned char HALLCD_DisplayIcon(unsigned char ucIconType,unsigned char *pucIconData)
{
	unsigned char ucI;
	unsigned char ucCh;
	unsigned char ucData;
	ICONDEF *pIconPtr;
	unsigned char *pucIconSeg;
	unsigned char ucColHigh;
	unsigned char ucColLow;

	if( ucIconType >= LCDICONTYPE_MAXNB)
		return(LCDDRVERR_UNSUPPORTICON);
	if( !pucIconData )
		return(LCDDRVERR_BADPARAM);

	pIconPtr = IconTable[ucIconType].pIconDef;
	pucIconSeg = IconTable[ucIconType].aucIconSEG;
	if( !pIconPtr )
		return(LCDDRVERR_UNSUPPORTICON);

	ucData = 0x00;
	if( pucIconSeg )
	{
		for(ucI=0;*(pucIconSeg+ucI);ucI++)
		{
			ucCh = *(pucIconSeg+ucI);
			ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
			ucColLow = ucCh & 0x0F;
			HALLCD_WriteCMDChip(0xB8);
			HALLCD_WriteCMDChip(ucColHigh);
			HALLCD_WriteCMDChip(ucColLow);
			HALLCD_WriteData(ucData);
		}
	}
	if( *pucIconData == 0xFF )
	{
		return(HALLCD_SUCCESS);
	}
	ucData = 0xFF;
	for( ucI=0;(pIconPtr+ucI)->ucIconCmd != 0xFF;ucI++)
	{
		if( (pIconPtr+ucI)->ucIconCmd == *pucIconData )
		{
			pucIconSeg = (pIconPtr+ucI)->aucSEG;
			if( pucIconSeg )
			{
				for(ucI=0;*(pucIconSeg+ucI);ucI++)
				{
					ucCh = *(pucIconSeg+ucI);
					ucColHigh = ((ucCh & 0xF0) >> 4)+0x10;
					ucColLow = ucCh & 0x0F;
					HALLCD_WriteCMDChip(0xB8);
					HALLCD_WriteCMDChip(ucColHigh);
					HALLCD_WriteCMDChip(ucColLow);
					HALLCD_WriteData(ucData);
				}
			}
			return(HALLCD_SUCCESS);
		}
	}
	return(LCDDRVERR_UNKNOWICONCMD);
}

unsigned char HALLCD_ClearIcon(unsigned char ucIconType)
{
	unsigned char ucData;

	ucData = 0xFF;
	HALLCD_Lock(1);

	HALLCD_DisplayIcon(ucIconType,&ucData);
	HALLCD_Unlock();

	return(HALLCD_SUCCESS);
}


static int fops_splc501c_ioctl(struct inode *inode, struct file *file, unsigned int cmd,unsigned char *arg)
{
	char str[34];
	int i = 0;
	char dat[10];
	for(i = 0; i < 10; i++)
	dat[i] = i;


	switch(cmd) 
	{
		case SPLC501_ZI:
			copy_from_user(str,arg, 32 + 2);
			HALLCD_Display8DotInCol(0, str[0], str[1], &(str[2]));
			HALLCD_DisplayIcon(LCDICONTYPE_SIGNAL, IconSingalDef);
			HALLCD_DisplayIcon(LCDICONTYPE_TELEPHONE, IconTelephoneDef);
			HALLCD_DisplayIcon(LCDICONTYPE_MESSAGE, IconMessageDef);
			HALLCD_DisplayIcon(LCDICONTYPE_MAGCARD, IconMagCardDef);
			HALLCD_DisplayIcon(LCDICONTYPE_ICCCARD, IconICCCardDef);
			HALLCD_DisplayIcon(LCDICONTYPE_BATTERY, IconBatteryDef);
			HALLCD_DisplayDate(dat);
			HALLCD_DisplayNewDate(dat);
		break;
		case SPLC501_BMP:
			copy_from_user(graph_buffer,arg, 16*64);
			HALLCD_Displaybmp(1,1,1,graph_buffer);
			HALLCD_DisplayIcon(LCDICONTYPE_SIGNAL, &(IconSingalDef[5]));
			HALLCD_DisplayIcon(LCDICONTYPE_TELEPHONE, IconTelephoneDef);
			HALLCD_DisplayIcon(LCDICONTYPE_MESSAGE, IconMessageDef);
			HALLCD_DisplayIcon(LCDICONTYPE_MAGCARD, IconMagCardDef);
			HALLCD_DisplayIcon(LCDICONTYPE_ICCCARD, IconICCCardDef);
			HALLCD_DisplayIcon(LCDICONTYPE_BATTERY, &(IconBatteryDef[3]));
			HALLCD_ClearIcon(LCDICONTYPE_TELEPHONE);
			HALLCD_DisplayDate(dat);			
			break;
		case SPLC501_ASCII:
			copy_from_user(str,arg, 16 + 2);
			HALLCD_Displayascii(0, str[0], str[1], &(str[2]));
		break;
		case SPLC501_ASCII5x7:
			copy_from_user(str,arg, 6 + 2);
			HALLCD_Displayascii5x7(0, str[0], str[1], &(str[2]));
		break;
		case SPLC501_LCDON:
			gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 0);
			break;
		case SPLC501_LCDOFF:
			gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 1);
			break;
		case SPLC501_CLR:
			copy_from_user(&(str[0]),arg, 1);
			HALLCD_ClearLine(str[0]);
			break;
		default:
		break;
	}
}

static int fops_splc501c_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int fops_splc501c_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations splc501c_lcd_ops = {
	.owner 	= THIS_MODULE,
	.open 	= fops_splc501c_open,
	.ioctl	 	= fops_splc501c_ioctl,
	.release	= fops_splc501c_release,       	                              
};

static int __init proc_register_chrdev_for_splc501c(struct basic_char_dev_info *p)
{
	int ret;

	ret = register_chrdev(SPLC501C_MAJOR,"lcd",&splc501c_lcd_ops);
	if(ret <0) 
	{
		printk(" can't get major number\n");
		return ret;
	}
	return 0;
}

static struct class *splc501c_class;
static int __init splc501c_init()
{
	int i = 0;
	int j = 0;
	unsigned char ucI,ucJ;
	unsigned char ucStartX;
	unsigned char *pucPtr;
	unsigned char ucCh;
	unsigned char ucColHigh;
	unsigned char ucColLow;
	int retval;

	struct basic_char_dev_info *dev_info;

	graph_buffer = (char *)kmalloc(16*64,GFP_USER);
	splc501c_class = class_create(THIS_MODULE, "splc501c");
	device_create(splc501c_class, NULL, MKDEV(SPLC501C_MAJOR,SPLC501C_MINOR), NULL, "splc501c" );
	HALLCD_Init();


	retval = proc_register_chrdev_for_splc501c(dev_info);

	HALLCD_ClearAll();
	return 0;
}
		
static void __exit splc501c_exit()
{
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), NULL);
	mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT5);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0), NULL);
    mxc_free_iomux(MX25_PIN_CSPI1_SS0, MUX_CONFIG_GPIO);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A14), 1);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_A14), NULL);
    mxc_free_iomux(MX25_PIN_A14, MUX_CONFIG_GPIO);    
    
	kfree(graph_buffer);
	device_destroy(splc501c_class, MKDEV(SPLC501C_MAJOR,SPLC501C_MINOR));
	unregister_chrdev(SPLC501C_MAJOR,"lcd");
    class_destroy(splc501c_class) ;     
	printk("exit\n");
}

module_init(splc501c_init);
module_exit(splc501c_exit);

MODULE_AUTHOR("wangyoulin<wang.youlin@sand.com.cn>");
MODULE_LICENSE("Dual BSD/GPL");
