/** @defgroup mxc_epit.c
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.0
 *  @date    2011-10-17
 @verbatim

 copyright (C) 2011上海杉德金卡信息科技有限公司

 IMX258RM中描述EPIT的使用步骤：
 1. Disable the EPIT by setting EN=0 in EPITCR.
 2. Program OM=00 in the EPITCR.
 3. Disable the EPIT interrupts.
 4. Program CLKSRC to desired clock source in EPITCR.
 5. Clear the EPIT status register (EPITSR) i.e. (w1c).
 6. Enable the EPIT interrupts.
 7. Set ENMOD= 1 in the EPITCR, to bring the EPIT Counter to defined state (EPITLR value or
 0xFFFF_FFFF).
 8. Enable EPIT (EN=1) in the EPITCR.

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
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/interrupt.h>

#include "mxc_epit.h"

/** EPIT使用步骤
  1. Disable the EPIT by setting EN=0 in EPITCR.
  2. Program OM=00 in the EPITCR.
  3. Disable the EPIT interrupts.
  4. Program CLKSRC to desired clock source in EPITCR.
  5. Clear the EPIT status register (EPITSR) i.e. (w1c).
  6. Enable the EPIT interrupts.
  7. Set ENMOD= 1 in the EPITCR, to bring the EPIT Counter to defined state (EPITLR value or
  0xFFFF_FFFF).
  8. Enable EPIT (EN=1) in the EPITCR.
 */
static struct epit_dev *epit_dev_p;

void epit_set_cmpr(mxc_epit n, u32 val)
{
    switch(n)
    {
    case EPIT1:
        iowrite32(val,epit_dev_p->epit1_cmpr);
        break;
    case EPIT2:
        iowrite32(val,epit_dev_p->epit2_cmpr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_cmpr);

void epit_set_lr(mxc_epit n, u32 val)
{
    switch(n)
    {
    case EPIT1:
        iowrite32(val,epit_dev_p->epit1_lr);
        break;
    case EPIT2:
        iowrite32(val,epit_dev_p->epit2_lr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_lr);

void epit_set_rld(mxc_epit n, u32 val)
{
    if(val>1)
    {
        pr_debug("epit not support rld biger than 1.\n");
    }
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1<<3)),epit_dev_p->epit1_cr);
        iowrite32(ioread32(epit_dev_p->epit1_cr)|(val<<3),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1<<3)),epit_dev_p->epit2_cr);
        iowrite32(ioread32(epit_dev_p->epit2_cr)|(val<<3),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_rld);

void epit_set_enmod(mxc_epit n, u32 val)
{
    if(val>1)
    {
        pr_debug("epit not support enmod biger than 1.\n");
    }
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1<<1)),epit_dev_p->epit1_cr);
        iowrite32(ioread32(epit_dev_p->epit1_cr)|(val<<1),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1<<1)),epit_dev_p->epit2_cr);
        iowrite32(ioread32(epit_dev_p->epit2_cr)|(val<<1),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_enmod);

void epit_set_prescalar(mxc_epit n, u32 val)
{
    if(val>0xFFF)
    {
        pr_debug("epit not support prescalar biger than 0xfff.\n");
    }
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(0xfff<<4)),epit_dev_p->epit1_cr);
        iowrite32(ioread32(epit_dev_p->epit1_cr)|(val<<4),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(0xfff<<4)),epit_dev_p->epit2_cr);
        iowrite32(ioread32(epit_dev_p->epit2_cr)|(val<<4),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_prescalar);

void epit_clear_ocif(mxc_epit n)
{
    switch(n)
    {
    case EPIT1:
        iowrite32(1,epit_dev_p->epit1_sr);
        break;
    case EPIT2:
        iowrite32(1,epit_dev_p->epit2_sr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_clear_ocif);

void epit_set_clksrc(mxc_epit n,u32 val)
{
    if(val>3)
    {
        pr_err("epit%d clksrc can't set to 0x%x", n, val);
    }
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(0x03<<24)),epit_dev_p->epit1_cr);
        iowrite32(ioread32(epit_dev_p->epit1_cr)|(val<<24),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(0x03<<24)),epit_dev_p->epit2_cr);
        iowrite32(ioread32(epit_dev_p->epit2_cr)|(val<<24),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_clksrc);

void epit_set_ocien(mxc_epit n,bool flag)
{
    switch(n)
    {
    case EPIT1:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)|(1<<2),epit_dev_p->epit1_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1<<2)),epit_dev_p->epit1_cr);
        }
        break;
    case EPIT2:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)|(1<<2),epit_dev_p->epit2_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1<<2)),epit_dev_p->epit2_cr);
        }
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_ocien);

void epit_set_iovw(mxc_epit n,bool flag)
{
    switch(n)
    {
    case EPIT1:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)|(1<<17),epit_dev_p->epit1_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1<<17)),epit_dev_p->epit1_cr);
        }
        break;
    case EPIT2:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)|(1<<17),epit_dev_p->epit2_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1<<17)),epit_dev_p->epit2_cr);
        }
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_iovw);

void epit_set_waiten(mxc_epit n,bool flag)
{
    switch(n)
    {
    case EPIT1:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)|(1<<19),epit_dev_p->epit1_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1<<19)),epit_dev_p->epit1_cr);
        }
        break;
    case EPIT2:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)|(1<<19),epit_dev_p->epit2_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1<<19)),epit_dev_p->epit2_cr);
        }
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_waiten);

void epit_set_stopen(mxc_epit n,bool flag)
{
    switch(n)
    {
    case EPIT1:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)|(1<<21),epit_dev_p->epit1_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1<<21)),epit_dev_p->epit1_cr);
        }
        break;
    case EPIT2:
        if(flag==true)
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)|(1<<21),epit_dev_p->epit2_cr);
        }
        else
        {
            iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1<<21)),epit_dev_p->epit2_cr);
        }
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_stopen);

void epit_set_om(mxc_epit n, u32 val)
{
    if(val>3)
    {
        pr_err("epit%d clksrc can't set to 0x%x", n, val);
    }
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(0x03<<22)),epit_dev_p->epit1_cr);
        iowrite32(ioread32(epit_dev_p->epit1_cr)|(val<<22),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(0x03<<22)),epit_dev_p->epit2_cr);
        iowrite32(ioread32(epit_dev_p->epit2_cr)|(val<<22),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_set_om);

void epit_disable(mxc_epit n)
{
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)&(~(1)),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)&(~(1)),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_disable);

void epit_enable(mxc_epit n)
{
    switch(n)
    {
    case EPIT1:
        iowrite32(ioread32(epit_dev_p->epit1_cr)|(1),epit_dev_p->epit1_cr);
        break;
    case EPIT2:
        iowrite32(ioread32(epit_dev_p->epit2_cr)|(1),epit_dev_p->epit2_cr);
        break;
    default:
        break;
    }
}
EXPORT_SYMBOL(epit_enable);

/**
  1. Disable the EPIT by setting EN=0 in EPITCR.
  2. Program OM=00 in the EPITCR.
  3. Disable the EPIT interrupts.
  4. Program CLKSRC to desired clock source in EPITCR.
  5. Clear the EPIT status register (EPITSR) i.e. (w1c).
  6. Enable the EPIT interrupts.
  7. Set ENMOD= 1 in the EPITCR, to bring the EPIT Counter to defined state (EPITLR value or 0xFFFF_FFFF).
  8. Enable EPIT (EN=1) in the EPITCR.
 */
static void test_irq(void)
{
    epit_disable(EPIT2);
    epit_set_om(EPIT2,0);
    epit_set_ocien(EPIT2,0);
    epit_set_iovw(EPIT2,1);
    epit_set_stopen(EPIT2,1);
    epit_set_waiten(EPIT2,1);
    epit_set_clksrc(EPIT2,3);
    epit_clear_ocif(EPIT2);
    epit_set_lr(EPIT2,0xFFFFFFFF);
    epit_set_cmpr(EPIT2,0xFFFFFFFF-(32768*60));
    //epit_set_cmpr(EPIT2,0xFFFFFFFF-0x4280000);
    //epit_set_rld(EPIT1,1);
    epit_set_enmod(EPIT2,1);
    epit_set_ocien(EPIT2,1);
    epit_enable(EPIT2);
}

static irqreturn_t test_int(int irq, void *devid)
{
    epit_pr_allreg(EPIT2);
    epit_clear_ocif(EPIT2);
    epit_set_lr(EPIT2,0xFFFFFFFF);
    test_irq();
    return IRQ_HANDLED;
}
void epit_pr_allreg(mxc_epit n)
{
	switch(n)
	{
		case EPIT1:
			pr_alert("epit1 cr = 0x%08x;sr = 0x%08x;lr = 0x%08x;cmpr = 0x%08x;cnr = 0x%08x\n",
             ioread32(epit_dev_p->epit1_cr),
             ioread32(epit_dev_p->epit1_sr),
             ioread32(epit_dev_p->epit1_lr),
             ioread32(epit_dev_p->epit1_cmpr),
             ioread32(epit_dev_p->epit1_cnr));
		break;
		case EPIT2:
			pr_alert("epit2 cr = 0x%08x;sr = 0x%08x;lr = 0x%08x;cmpr = 0x%08x;cnr = 0x%08x\n",
             ioread32(epit_dev_p->epit2_cr),
             ioread32(epit_dev_p->epit2_sr),
             ioread32(epit_dev_p->epit2_lr),
             ioread32(epit_dev_p->epit2_cmpr),
             ioread32(epit_dev_p->epit2_cnr));
		break;
		default:
		break;
	}
}
EXPORT_SYMBOL(epit_pr_allreg);

static int __init epit_init(void)
{
    epit_dev_p=kzalloc(sizeof(struct epit_dev),GFP_KERNEL);
    if(epit_dev_p==NULL)
    {
        pr_debug("epit nomem.");
        goto nomem;
    }
    epit_dev_p->epit1_cr=ioremap_nocache(EPIT1_CR_PHYADDR,4);
    epit_dev_p->epit1_sr=ioremap_nocache(EPIT1_SR_PHYADDR,4);
    epit_dev_p->epit1_lr=ioremap_nocache(EPIT1_LR_PHYADDR,4);
    epit_dev_p->epit1_cmpr=ioremap_nocache(EPIT1_CMPR_PHYADDR,4);
    epit_dev_p->epit1_cnr=ioremap_nocache(EPIT1_CNR_PHYADDR,4);

    epit_dev_p->epit2_cr=ioremap_nocache(EPIT2_CR_PHYADDR,4);
    epit_dev_p->epit2_sr=ioremap_nocache(EPIT2_SR_PHYADDR,4);
    epit_dev_p->epit2_lr=ioremap_nocache(EPIT2_LR_PHYADDR,4);
    epit_dev_p->epit2_cmpr=ioremap_nocache(EPIT2_CMPR_PHYADDR,4);
    epit_dev_p->epit2_cnr=ioremap_nocache(EPIT2_CNR_PHYADDR,4);

    //request_irq(EPIT2_INT, test_int, 0, 0, 0);
    test_irq();
    
    return 0;
nomem:
    return -ENOMEM;
}

static void __exit epit_exit(void)
{
    free_irq(EPIT1_INT, 0);
    kfree(epit_dev_p);
}

MODULE_AUTHOR("zengshu");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("MODULE_RELEASE");

module_init(epit_init);
module_exit(epit_exit);
/** @}*/
