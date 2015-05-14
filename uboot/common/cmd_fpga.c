/*
 * (C) Copyright 2000, 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/system.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <asm/arch/mx25.h>
#include <asm/arch/mx25-regs.h>
#include <asm/arch/mx25_pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

#if defined(CONFIG_CMD_NET)
#include <net.h>
#endif
#include <fpga.h>
#include <malloc.h>

#if 0
#define	FPGA_DEBUG
#endif

#ifdef	FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#define mdelay(n)  ({unsigned long msec=(n);while(msec--) udelay(1000);})

#define FPGA_ADDR  0x80800000
#define FPGA_SIZE  0x109F8
#define NAND_FPGA  0x00100000

#if (defined(CONFIG_PS4K))
#define FPGA_BASEADDR 0xB2000000
#else
#define FPGA_BASEADDR 0xA0000000
#endif

/* ------------------------------------------------------------------------- */
/* command form:
 *   fpga <op> <device number> <data addr> <datasize>
 * where op is 'load', 'dump', or 'info'
 * If there is no device number field, the fpga environment variable is used.
 * If there is no data addr field, the fpgadata environment variable is used.
 * The info command requires no data address field.
 */

#if defined(CONFIG_PS4K)
/*CDONE MX25_PIN_FEC_MDC GPIO3_5*/
#define set_CDONE_hi()                                     \
		do {                                                         \
				mxc_set_gpio_dataout(MX25_PIN_FEC_MDC, 1);              \
		} while(0)

#define set_CDONE_low()                                    \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_FEC_MDC, 0);              \
		}while(0)

/*CRESET_B MX25_PIN_FEC_MDIO GPIO3_6*/
#define set_CRESET_B_hi()                                        \
		do {                                                         \
				mxc_set_gpio_dataout(MX25_PIN_FEC_MDIO, 1);              \
		} while(0)

#define set_CRESET_B_low()                                       \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_FEC_MDIO, 0);              \
		}while(0)
#elif defined(CONFIG_PS4KV2)
/*CDONE MX25_PIN_FEC_TDATA1 GPIO3_8*/
#define set_CDONE_hi()                                     \
		do {                                                         \
				mxc_set_gpio_dataout(MX25_PIN_FEC_TDATA1, 1);              \
		} while(0)

#define set_CDONE_low()                                    \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_FEC_TDATA1, 0);              \
		}while(0)

/*CRESET_B MX25_PIN_FEC_TX_EN GPIO3_9*/
#define set_CRESET_B_hi()                                        \
		do {                                                         \
				mxc_set_gpio_dataout(MX25_PIN_FEC_TX_EN, 1);              \
		} while(0)

#define set_CRESET_B_low()                                       \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_FEC_TX_EN, 0);              \
		}while(0)

#else
#error //UNKONW BOARD
#endif

/*PIOS/SPI_SS MX25_PIN_CSPI1_SS0 CSPI1_SS0*/
#define set_SPI_SS_hi()                                       \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_CSPI1_SS0, 1);              \
		}while(0)

#define set_SPI_SS_low()                                       \
		do{                                                           \
				mxc_set_gpio_dataout(MX25_PIN_CSPI1_SS0, 0);               \
		}while(0)

/* MX25_PIN_CSPI1_MOSI CSPI1_MOSI*/
#define set_SPI_SI_hi()                                       \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_CSPI1_MOSI, 1);              \
		}while(0)

#define set_SPI_SI_low()                                       \
		do{                                                           \
				mxc_set_gpio_dataout(MX25_PIN_CSPI1_MOSI, 0);               \
		}while(0)

/*PIOS/SPI_SCK MX25_PIN_CSPI1_SCLK CSPI1_SCLK*/
#define set_SPI_SCK_hi()                                       \
		do{                                                          \
				mxc_set_gpio_dataout(MX25_PIN_CSPI1_SCLK, 1);              \
		}while(0)

#define set_SPI_SCK_low()                                       \
		do{                                                           \
				mxc_set_gpio_dataout(MX25_PIN_CSPI1_SCLK, 0);               \
		}while(0)

//Configurable Serial Peripheral Interface(CSPI) address
#define CSPI_ADDR			0x43FA4000;
//Receive Data Register (RXDATA) address
#define CSPI_RXDATA_ADDR	0x0000;
//Transmit Data Register (TXDATA) address
#define CSPI_TXDATA_ADDR	0x0004;
//Control Register (CONREG) address
#define CSPI_CONREG_ADDR	0x0008;
//Interrupt Control Register (INTREG) address
#define CSPI_INTREG_ADDR	0x000C;
//DMA Control Register (DMAREG) address
#define CSPI_DMAREG_ADDR	0x0010;
//Status Register (STATREG) address
#define CSPI_STATREG_ADDR	0x0014;
//Sample Period Control Register (PERIODREG) address
#define CSPI_PERIODREG_ADDR	0x0018;
//Test Control Register (TESTREG) address
#define CSPI_TESTREG_ADDR	0x001C;

u32 count;
#if defined(CONFIG_PS4K)
#define Read_nCONF_Done()   mxc_get_gpio_dataout(MX25_PIN_FEC_MDC)
#elif defined(CONFIG_PS4KV2)
#define Read_nCONF_Done()   mxc_get_gpio_dataout(MX25_PIN_FEC_TDATA1)
#else
#error //UNKNOW BOARD
#endif

//internal pre-load command sequence
int const spipre1[6] = {0x7E, 0xAA, 0x99, 0x7E, 0x01, 0x0E};
int const spipre2[5] = {0x83, 0x00, 0x00, 0x26, 0x11};
int const spipre3[5] = {0x83, 0x00, 0x00, 0x27, 0x21};
int const spipre4[1] = {0x81};

/*
	return 	1 exist
		0 not exist
*/
//通过判断写入的数据与读出数据是否相反来看FPGA是否已经配置。
int depend_fpga_exist(void)
{
	volatile int *addr;
	int ret;
	ulong data;
	ulong r1=0x56;// 0x6a 的 bit 位反 ；
	ulong r2=0xab;// 0xd5 的 bit 位反 ；
	addr=(ulong *)(FPGA_BASEADDR+0x68);
	*addr=0xffffff6a;
	data=*addr;
	if(r1==(data&0x000000ff))
	{
		*addr=0xffffffd5;
	    data=*addr;
		if(r2==(data&0x000000ff))
		{
			printf("FPGA has been configed! \n");
			ret=1;
		}
		else
		{
			printf("It will be config FPGA! \n");
			ret = 0;
		}
	}	
	else
	{
		printf("It will be config FPGA! \n");
		ret=0;
	}
	return ret;
}
 
int gpio_init(void)
{
	int ret = 0;
#ifdef CONFIG_PS4K
	//Init GPIO3_5 as a input port
	if((ret = mxc_request_iomux(MX25_PIN_FEC_MDC, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init GPIO3_5 fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_FEC_MDC, 1);
	
	//Init GPIO3_6 as a output port
	if((ret = mxc_request_iomux(MX25_PIN_FEC_MDIO, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init GPIO3_6 fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_FEC_MDIO, 0);
#elif defined(CONFIG_PS4KV2)
	//Init GPIO3_8 as a input port
	if((ret = mxc_request_iomux(MX25_PIN_FEC_TDATA1, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init GPIO3_8 fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_FEC_TDATA1, 1);
	
	//Init GPIO3_9 as a output port
	if((ret = mxc_request_iomux(MX25_PIN_FEC_TX_EN, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init GPIO3_9 fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_FEC_TX_EN, 0);

#else
#error //UNKNOW BOARD
#endif
	//Init GPIO3_7 as a output port
	if((ret = mxc_request_iomux(MX25_PIN_FEC_TDATA0, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init GPIO3_7 fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_FEC_TDATA0, 0);
	//Init CSPI1_MISO as a input port
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_MISO, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init CSPI1_MISO fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_CSPI1_MISO, 1);

	//Init CSPI1_MOSI as a output port
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_MOSI, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init CSPI1_MOSI fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_CSPI1_MOSI, 0);

	//Init CSPI1_SCLK as a output port
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_SCLK, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init CSPI1_SCLK fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_CSPI1_SCLK, 0);

	//Init CSPI1_SS0 as a output port
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_SS0, MUX_CONFIG_GPIO))!=0)
	{
		printf("Init CSPI1_SS0 fail!\n");
		return ret;
	}
	mxc_set_gpio_direction(MX25_PIN_CSPI1_SS0, 0);
	return ret;
}

int cspi_init(void)
{
	int ret = 0;
	//Init CSPI1_MISO as a CSPI
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_MISO, MUX_CONFIG_FUNC))!=0)
	{
		printf("Init CSPI1_MISO fail!\n");
		return ret;
	}

	//Init CSPI1_MOSI as CSPI
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_MOSI, MUX_CONFIG_FUNC))!=0)
	{
		printf("Init CSPI1_MOSI fail!\n");
		return ret;
	}

	//Init CSPI1_SCLK as a SPI
	if((ret=mxc_request_iomux(MX25_PIN_CSPI1_SCLK, MUX_CONFIG_FUNC))!=0)
	{
		printf("Init CSPI1_SCLK fail!\n");
		return ret;
	}
	return ret;
}
 
void fpga_poweron(void)
{
	volatile int *addr1;
	volatile int *addr2;
    ulong data;
    addr1=(ulong *)(FPGA_BASEADDR+0x6C);
    *addr1=0xffffffff;
	*addr1=0xffffff5a;
	*addr1=0xffffffff;
	addr2=(ulong *)(FPGA_BASEADDR+0x70);
	*addr2=0xffffffe0;
}


void read_fpga_ver(void)
{

	volatile int *addr;
	ulong data;
	addr=(ulong *)(FPGA_BASEADDR+0x64);
	data=*addr;
	printf("%s %1x.%1x\n","FPGA VER ",(data&0xf0) >> 4,data&0x0000000f);
}


/*dir: 1 -outPut
 *     0 -input
 */

int detect_flag(void)
{
	int flag = 0 ;
	volatile int *addr;
	addr=(ulong *)0x43fa4014;
	flag=(*addr)&0x000080;
	if(flag != 0){
	return 1;
	}
	else
		return 0 ;
}
void clear_flag(void)
{
	volatile int *addr;
	addr=(ulong *)0x43fa4014;
	*addr |=0x00000080;
} 
void fpga_init(void)
{
	mxc_set_gpio_dataout(MX25_PIN_FEC_TDATA0, 0);  
	mxc_set_gpio_dataout(MX25_PIN_FEC_TDATA0, 1);  
	udelay(4);
	mxc_set_gpio_dataout(MX25_PIN_FEC_TDATA0, 0);  
}	
 
int fpga_load(u8 *ptr, u32 fpga_size)
{
	u8 *fpga_buffer = ptr;
	u32  i;
	u8 delay_times = 0;

	volatile  int *SPI_Control_Reg;
	volatile  int *SPI_Txdata_Reg;

	SPI_Control_Reg = (ulong *)0x43fa4008 ;
	SPI_Txdata_Reg = (ulong *)0x43fa4004 ;
	
	//Init CSPI
	if(cspi_init())
	{
		printf("Init CSPI failed!\n");
		return -1;
	}
	
	//配置SPI
	*SPI_Control_Reg = 0x0072003b;


//	printf("fpga_test 3 !!\n");	
	
	//Index 1: reset FPGA Configuration State Machine : > 200ns
	set_CRESET_B_low();
	set_SPI_SS_low();
	udelay(2);
	
	//Index 2: force FPGA Configuration Slave Mode : >= 800us for L01, L04, and P04; >= 1200us for L08
	set_CRESET_B_hi();
	udelay(1200);
	
	//Begin pre-load sequence
   //Index 4: Set SPI_SS_B high for 8 clocks
	set_SPI_SS_hi();
	*SPI_Txdata_Reg = 0x00;
	while(detect_flag()==0){;}
  clear_flag();


	//Index 5: Sent first pre-load commands
	set_SPI_SS_low();
	for(i=0;i<6;i++)
	{
		*SPI_Txdata_Reg = (volatile unsigned int)spipre1[i];
			while(detect_flag()==0){;}
			clear_flag();
	}

	//Index 6: Set SPI_SS_B high for 13000 clocks
	set_SPI_SS_hi();
	for(i=0;i<13000/8;i++)
	{
		*SPI_Txdata_Reg = (volatile unsigned int)0x0;
			while(detect_flag()==0){;}
			clear_flag();
	}

	//Index 7: Sent second pre-load commands
  set_SPI_SS_low();
	for(i=0;i<5;i++)
	{
		*SPI_Txdata_Reg = (volatile unsigned int)spipre2[i];
		while(detect_flag()==0){;}
    clear_flag();
	}

	//Index 8: Set SPI_SS_B high for 8 clocks
	set_SPI_SS_hi();
	*SPI_Txdata_Reg = (volatile unsigned int)0x0;
  while(detect_flag()==0){;}
  clear_flag();

	//Index 9: Sent third pre-load commands
		set_SPI_SS_low();
	for(i=0;i<5;i++)
	{
	*SPI_Txdata_Reg = (volatile unsigned int)spipre3[i];	
		while(detect_flag()==0){;}
   clear_flag();
	}


	//Index 10: Set SPI_SS_B high for 8 clocks
	set_SPI_SS_hi();
	*SPI_Txdata_Reg = (volatile unsigned int)0x0;
  while(detect_flag()==0){;}
  clear_flag();
  
	//Index 11: Sent fourth pre-load commands
	set_SPI_SS_low();
	*SPI_Txdata_Reg = (volatile unsigned int)spipre4[0];
	while(detect_flag()==0){;}
	clear_flag();
	
	//Index 12: Set SPI_SS_B high for 8 clocks
	set_SPI_SS_hi();
	*SPI_Txdata_Reg = (volatile unsigned int)0x0;
	while(detect_flag()==0){;}
	clear_flag();

	//Index 13:
	/*write data to FPGA port*/
	set_SPI_SS_low();
	count = 0 ;
	while(count < fpga_size )
	{
		static u8 configbyte;
		configbyte = *(fpga_buffer+count);
		*SPI_Txdata_Reg = (volatile unsigned int)configbyte;
		while(detect_flag()==0){;}
    clear_flag();
		count++ ;
	}
	
//Index 14: Shifting 100 clocks
	set_SPI_SS_hi();
	for(i=0;i<100/8+1;i++)
	{
		*SPI_Txdata_Reg = (volatile unsigned int)0x0;
		while(detect_flag()==0){;}
		clear_flag();
	}
//Index 15 :Check CDONE Hi
	if(Read_nCONF_Done() == 0)
	{
			printf("FPGA config error.\n");
			run_command("menu;", 0);
	}
	set_SPI_SS_hi();
	*SPI_Control_Reg = 0x00000000;
	return 0;
}
 
int do_fpga (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	
	u8   *dataptr;

	size_t cnt = (size_t)FPGA_SIZE;
	if(gpio_init())
	{
		printf("gpio_init error!!\n");
		return -1;
	}
	if(depend_fpga_exist())
	{
		fpga_init();
	}
	else
	{
		ulong offset = NAND_FPGA;
		nand_info_t *nand = &nand_info[nand_curr_device];
		if(nand_read_skip_bad(nand, offset, &cnt, (u8*)FPGA_ADDR))
		{
			printf("load fpga image from nand error!!\n");
			return -1;
		}
		dataptr = (u8 *)FPGA_ADDR;
		fpga_load(dataptr,cnt);
		fpga_init();
		fpga_poweron();		
	}
	read_fpga_ver();
/*	
	if(gpio_init())
	{
		printf("gpio_init error!!\n");
		return -1;
	}
	*/
}

#if (defined(CONFIG_PS4K)||defined(CONFIG_PS4KV2))
U_BOOT_CMD (fpga_config, 6, 1, do_fpga,
	    "loadable FPGA image support",
	    "fpga [operation type] [device number] [image address] [image size]\n"
	    "fpga operations:\n"
	    "\tinfo\tlist known device information\n"
	    "\tload\tLoad device from memory buffer\n"
	    "\tloadb\tLoad device from bitstream buffer (Xilinx devices only)\n"
	    "\tloadmk\tLoad device generated with mkimage\n"
	    "\tdump\tLoad device to memory buffer"
#if defined(CONFIG_FIT)
	    "\n"
	    "\tFor loadmk operating on FIT format uImage address must include\n"
	    "\tsubimage unit name in the form of addr:<subimg_uname>"
#endif
);
#endif //#ifdef CONFIG_PS4K
