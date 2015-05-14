/** @defgroup printer
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.0
 *  @date    2012-10-25
 @verbatim

 copyright (C) 2011上海杉德金卡信息科技有限公司

 @endverbatim
 *  @{
 */
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/aio.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/kfifo.h>

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>

#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include "mx25printer.h"
#include "apselm205.h"
#include "mach-mx25/machine.h"
#include "mach-mx25/mxc_epit.h"

int dbg = 0;
module_param(dbg, int, S_IRUGO);

#define TEN_US_COUNT 665
void set_timer_off(void)
{
    epit_clear_ocif(EPIT1);
    epit_disable(EPIT1);
}

void set_timer_time_again(unsigned int us)
{
    epit_clear_ocif(EPIT1);
    epit_set_lr(EPIT1,0xFFFFFFFF);
    epit_set_cmpr(EPIT1,0xFFFFFFFF-us/10*TEN_US_COUNT);
}

void set_timer_on(unsigned int us)
{
    epit_disable(EPIT1);
    epit_set_om(EPIT1,0);
    epit_set_ocien(EPIT1,0);
    epit_set_iovw(EPIT1,1);
    epit_set_clksrc(EPIT1,2);
    epit_clear_ocif(EPIT1);
    epit_set_lr(EPIT1,0xFFFFFFFF);
    epit_set_cmpr(EPIT1,0xFFFFFFFF-us/10*TEN_US_COUNT);
    //epit_set_rld(EPIT1,1);
    epit_set_enmod(EPIT1,1);
    epit_set_ocien(EPIT1,1);
    epit_enable(EPIT1);
}

static struct printer_t *printer_t_p;

void print_all_reg(void)
{
            pr_alert("printer reg:cs1:0x%02x,cs2:0x%02x,cs3:0x%02x,cs4:0x%02x\n",
             ioread8(printer_t_p->reg_print_cs1),
             ioread8(printer_t_p->reg_print_cs2),
             ioread8(printer_t_p->reg_print_cs3),
             ioread8(printer_t_p->reg_print_cs4));
}

void printer_set_power_on(bool val)
{
    if(val)
    {
        iowrite8(ioread8(printer_t_p->reg_print_cs1)|(1<<7),
                 printer_t_p->reg_print_cs1);
    }
    else
    {
        iowrite8(ioread8(printer_t_p->reg_print_cs1)&(~(1<<7)),
                 printer_t_p->reg_print_cs1);
    }
}

void printer_set_latch(bool val)
{
    if(val)
    {
        iowrite8(ioread8(printer_t_p->reg_print_cs1)|(1<<6),
                 printer_t_p->reg_print_cs1);
    }
    else
    {
        iowrite8(ioread8(printer_t_p->reg_print_cs1)&(~(1<<6)),
                 printer_t_p->reg_print_cs1);
    }
}

void printer_set_paper_detect(bool val)
{
    if(val)
    {
        iowrite8(ioread8(printer_t_p->reg_print_cs1)|(1<<5),
                 printer_t_p->reg_print_cs1);
    }
    else
    {
        iowrite8(ioread8(printer_t_p->reg_print_cs1)&(~(1<<5)),
                 printer_t_p->reg_print_cs1);
    }
}

bool is_printer_paper_detect_enable(void)
{
    if((ioread8(printer_t_p->reg_print_cs1)&(1<<5))!=0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void printer_set_sb(uint8_t val)
{
    iowrite8(val,printer_t_p->reg_print_cs2);
}

uint8_t printer_read_sb(void)
{
    return ioread8(printer_t_p->reg_print_cs2);
}

void printer_set_moto(uint8_t val)
{
    iowrite8(val,printer_t_p->reg_print_cs3);
}

uint8_t printer_read_moto(void)
{
    return ioread8(printer_t_p->reg_print_cs3);
}

bool printer_is_overheat(void)
{
    return (ioread8(printer_t_p->reg_print_cs4)&(1<<3))?true:false;
}

bool printer_is_havepaper(void)
{
    bool paper_stat;
    if(is_printer_paper_detect_enable()==true)
    {
        paper_stat=(ioread8(printer_t_p->reg_print_cs4)&(1<<1))?true:false;
    }
	else
	{
        printer_set_paper_detect(true);
        udelay(500);
        paper_stat=(ioread8(printer_t_p->reg_print_cs4)&(1<<1))?true:false;
        printer_set_paper_detect(false);
	}

    return paper_stat;
}

struct printer_t* printer_t_init(void)
{
    struct printer_t *p;
    p=kzalloc(sizeof(struct printer_t),GFP_KERNEL);
    if(p==NULL)
    {
        if(dbg)
            pr_alert("kzalloc struct printer_t faild!\n");
        return p;
    }

	if(SAND_DEMOBOARD==sand_machine())
	{
		p->reg_print_cs1=ioremap_nocache(DEMO_REG_PRINT_CS1,1);
		p->reg_print_cs2=ioremap_nocache(DEMO_REG_PRINT_CS2,1);
		p->reg_print_cs3=ioremap_nocache(DEMO_REG_PRINT_CS3,1);
		p->reg_print_cs4=ioremap_nocache(DEMO_REG_PRINT_CS4,1);
		p->reg_cpld_dbg=ioremap_nocache(DEMO_REG_CPLD_DBG,1);
	}
	else if(SAND_IPS420==sand_machine())
	{
		p->reg_print_cs1=ioremap_nocache(IPS420_REG_PRINT_CS1,1);
		p->reg_print_cs2=ioremap_nocache(IPS420_REG_PRINT_CS2,1);
		p->reg_print_cs3=ioremap_nocache(IPS420_REG_PRINT_CS3,1);
		p->reg_print_cs4=ioremap_nocache(IPS420_REG_PRINT_CS4,1);
		p->reg_cpld_dbg=ioremap_nocache(IPS420_REG_CPLD_DBG,1);
	}
	else
	{
		p->reg_print_cs1=ioremap_nocache(PS400_REG_PRINT_CS1,1);
		p->reg_print_cs2=ioremap_nocache(PS400_REG_PRINT_CS2,1);
		p->reg_print_cs3=ioremap_nocache(PS400_REG_PRINT_CS3,1);
		p->reg_print_cs4=ioremap_nocache(PS400_REG_PRINT_CS4,1);
		p->reg_cpld_dbg=ioremap_nocache(PS400_REG_CPLD_DBG,1);
	}

    p->machine_type=(ioread8(p->reg_print_cs1))&0x07;

    return p;
}

void printer_t_exit(void)
{
    iounmap(printer_t_p->reg_print_cs1);
    iounmap(printer_t_p->reg_print_cs2);
    iounmap(printer_t_p->reg_print_cs3);
    iounmap(printer_t_p->reg_print_cs4);
    iounmap(printer_t_p->reg_cpld_dbg);

    kfree(printer_t_p);
}

/**
* @brief 打印机的初始化函数，此函数仅作为一个接口，
* 判断打印机类型,然后加载相应的驱动程序。
*/
static int __init printer_init(void)
{
    int ret=0;
    printer_t_p=printer_t_init();
    if(printer_t_p==NULL)
    {
        return -ENOMEM;
    }
    switch(printer_t_p->machine_type)
    {
        case PRINTER_TYPE_APSELM205:
            ret=apselm205_init(printer_t_p);
        break;
        case PRINTER_TYPE_MU111SII:
            ret=apselm205_init(printer_t_p);
            if(dbg)
                pr_err("Not support M-U111SII printer yet.\n");
        break;
        default:
        	ret=apselm205_init(printer_t_p);
        	if(dbg)
                pr_err("Unknow printer..\n");
            //ret=-EINVAL;
        break;
    }
    return ret;
}

/**
* @brief 打印机的退出函数
*/
static void __exit printer_exit(void)
{
    switch(printer_t_p->machine_type)
    {
        case PRINTER_TYPE_APSELM205:
            apselm205_exit();
        break;
        case PRINTER_TYPE_MU111SII:
            apselm205_exit();
            if(dbg)
                pr_err("Not support M-U111SII printer yet.\n");
        break;
        default:
            if(dbg)
                pr_err("Unknow printer.\n");
        break;
    }
    printer_t_exit();
}

MODULE_AUTHOR("zengshu@gmail.com");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Sand printer module driver.");

module_init(printer_init);
module_exit(printer_exit);
/** @} */
