/*
 * cmd_testcpld.c
 *
 *  Created on: Sep 7, 2011
 *      Author: root
 */
//
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
 * EINT0  GPF0  CONF_DCLK      GPJ8
 * EINT1  GPF1  CONF_DATA0     GPJ11
 * EINT2  GPF2  CONF_nCONFIG   GPJ10
 * EINT3  GPF3
 * EINT4  GPF4
 * */

/*GPJ11   CONF_DATA0
  GPJ12   CONF_DONE
  GPJ8    CONF_DCLK
  GPJ10   CONF_nCONFIG
  GPJ9    CONF_nSTATUS
  */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/system.h>
//#include <s3c2410.h>

#include <linux/mtd/mtd.h>
#include <nand.h>
#include <asm-arm/io.h>

#include <asm/arch/mx25.h>
#include <asm/arch/mx25-regs.h>
#include <asm/arch/mx25_pins.h>
#include <asm/arch/iomux.h>

#if defined(CONFIG_CMD_NET)
#include <net.h>
#endif
//#include <fpga.h>
#include <malloc.h>

#define TESTCPLD_IO  0xb2e00000
#define TESTLEG_REG  0xb2a00000

#define pSMDK2410_IC1_IO   0xb2e00000
//#define pSMDK2410_IC1_IO   0xa6e00000
//#define pSMDK2410_IC1_IO   0xb2000000

#define mdelay(n)  ({unsigned long msec=(n);while(msec--) udelay(1000);})

#if 0
static void ic_writeb(unsigned char data, unsigned long addr)
{
	*(volatile unsigned char*)addr= data;
}

static unsigned char  ic_readb(unsigned long addr)
{
    unsigned char val;
    val = (*(volatile unsigned char*)addr);
    return val;
}
#endif

void mxc_cpld_reset1(void)
{
     printf("[%s, %d]\n",__func__,__LINE__);
	mxc_request_iomux(MX25_PIN_UPLL_BYPCLK, MUX_CONFIG_GPIO);
	printf("[%s, %d]\n",__func__,__LINE__);
//	gpio_request(IOMUX_TO_GPIO(MX25_PIN_UPLL_BYPCLK),NULL);
	mxc_set_gpio_direction(MX25_PIN_UPLL_BYPCLK, 0);
	printf("[%s, %d]\n",__func__,__LINE__);
	mxc_set_gpio_dataout(MX25_PIN_UPLL_BYPCLK, 0);	/* CPLD_RESET */
	printf("[%s, %d]\n",__func__,__LINE__);
	udelay(3);
	mxc_set_gpio_dataout(MX25_PIN_UPLL_BYPCLK, 1);	/* CPLD_RESET */
	printf("[%s, %d]\n",__func__,__LINE__);

}


int do_testICC(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if 0
	u8   *dataptr, tmp;
	volatile unsigned char ucREGCSR, ucREGMSR;

	printk("******do_testICC() TTT*********\n");

	//ucREGCSR=ic_readb(pSMDK2410_IC1_IO);
	ucREGCSR= 0xaf;
    writeb(ucREGCSR, TESTCPLD_IO);
//	ucREGCSR = 0xaa;
	//ucREGCSR=readb(0xb2000000);
	ucREGCSR=readb(TESTCPLD_IO);
	printf("%d,TESTCPLD_IO_addr:0x%x, ucREGCSR:0x%x\n",__LINE__,TESTCPLD_IO, ucREGCSR);
	//udelay(10);


        tmp = readb(TESTLEG_REG);
        printk("tmp1=0x%x\n", tmp);
        writeb((tmp & ~(1 << 6)), TESTLEG_REG);
        tmp = readb(TESTLEG_REG);
        printk("tmp2=0x%x\n", tmp);


        tmp = readb(TESTLEG_REG);
               printk("tmp1=0x%x\n", tmp);
               writeb((tmp | (1 << 6)), TESTLEG_REG);
               tmp = readb(TESTLEG_REG);
               printk("tmp2=0x%x\n", tmp);

	//writeb(0x33, pSMDK2410_IC1_IO + 0xf);
	//	udelay(10);
	//	ucREGCSR =readb(pSMDK2410_IC1_IO + 0xf);
	//	printf("%d,CSx_addr:0xb2000000 + 0xF, ucREGHSR:0x%x\n",__LINE__, ucREGCSR);
/*
	writeb(0x33, pSMDK2410_IC1_IO + 0x7);
	udelay(10);
	ucREGCSR =readb(pSMDK2410_IC1_IO + 0x7);
	printf("%d,CSx_addr:0xb2000000 + 0x7, ucREGPCR:0x%x\n",__LINE__, ucREGCSR);
*/
	//ucREGCSR &= 0x0;
//	ucREGCSR = 0xAA;
	//ic_writeb(ucREGCSR, pSMDK2410_IC1_IO);
//	writeb(ucREGCSR, pSMDK2410_IC1_IO);
//	ucREGCSR = 0xad;

	//ucREGCSR=readb(pSMDK2410_IC1_IO);
	//ucREGCSR=readb(pSMDK2410_IC1_IO);

	//printf("%d,ucREGCSR:0x%x\n",__LINE__, ucREGCSR);

	//udelay(10);
  return 0;


	ucREGCSR = 0x3A;
	//ucREGCSR |= ((0x9 << 4) | (0x1 << 1));
	//ic_writeb(ucREGCSR, pSMDK2410_IC1_IO);
	writeb(ucREGCSR,pSMDK2410_IC1_IO);
	ucREGCSR = 0xad;
	udelay(10);
	//ucREGCSR=ic_readb(pSMDK2410_IC1_IO);
	ucREGCSR=readb(pSMDK2410_IC1_IO);
	printf("%d,ucREGCSR:0x%x\n",__LINE__, ucREGCSR);
	udelay(10);



	ucREGCSR = 0x5;
		//ucREGCSR |= ((0x9 << 4) | (0x1 << 1));
		//ic_writeb(ucREGCSR, pSMDK2410_IC1_IO);
		writeb(ucREGCSR,pSMDK2410_IC1_IO);
		ucREGCSR = 0xad;
		udelay(10);
		//ucREGCSR=ic_readb(pSMDK2410_IC1_IO);
		ucREGCSR=readb(pSMDK2410_IC1_IO);
		printf("%d,ucREGCSR:0x%x\n",__LINE__, ucREGCSR);
		udelay(10);

//	writel(0x0000D843,0xB8002030);
//	writel(0x22252321,0xB8002034);
//	writel(0x22220A00,0xB8002038);
#if 0
	writel(0x23424e80,0xB8002030);
	writel(0x10000b43,0xB8002034);
	writel(0x00720900,0xB8002038);

	ucREGCSR =  readl(0xB800304c);

	printf("%d,CSx_addr:0x%x,lock CS3 Register:0x%x\n",__LINE__,pSMDK2410_IC1_IO, ucREGCSR);

	ucREGCSR =  readl(0xB8002060);
		printf("%d,CSx_addr:0x%x,WCR CS3 Register:0x%x\n",__LINE__,pSMDK2410_IC1_IO, ucREGCSR);
		ucREGCSR &= ~(1 << 11);
		writel(ucREGCSR, 0xB8002060);
#endif

#if 0
		unsigned tmp = 0;
		tmp =readl(0xB8002034);
		printf("CS3L:0x%x\n", tmp);

	//	MUX_SDCTL_CSD1_SEL

		/*
#define CS3_ENABLE  1
#define CS3_DISBALE 0
		 * */

		mxc_iomux_set_gpr(MUX_SDCTL_CSD1_SEL, CS3_ENABLE);
		while(0)
		{
	    	mxc_cpld_reset1();
	    	mdelay(1);
		}


  int i;
  //for (i = 0 ; i < 10  ; i++)
  while(1)
  {
	ucREGCSR = 0x5A;
//	ucREGCSR |= ((0x7 << 4) | (0x1 << 2));
//	ic_writeb(ucREGCSR, pSMDK2410_IC1_IO);
	writeb(ucREGCSR, pSMDK2410_IC1_IO);
//	ucREGCSR = 0xad;
	mdelay(100);

	ucREGCSR=readb(pSMDK2410_IC1_IO);
	printf("%d,CSx_addr:0x%x,ucREGCSR:0x%x\n",__LINE__,pSMDK2410_IC1_IO, ucREGCSR);
	mdelay(100);
  }
#endif

#endif
  return 0;
}

U_BOOT_CMD (test_icc, 6, 1, do_testICC,
	    "test_icc",
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
