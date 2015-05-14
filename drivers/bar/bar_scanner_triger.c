/** @defgroup mx25printer.c
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.0
 *  @date    2011-10-17
 @verbatim

 copyright (C) 2011上海杉德金卡信息科技有限公司

 @endverbatim
 *  @{
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "mach-mx25/board-mx25_3stack.h"
#include "mach-mx25/iomux.h"
#include "mach-mx25/mx25_pins.h"
#include "mach-mx25/machine.h"

#define SCAN_CS_ADDR 0xb2800000
#define IPS420_SCAN_CS_ADDR 0xa0800000
#define PS4KV2_SCAN_CS_ADDR 0xa0000090

/*
 * FIXME:Here the ioctl cmd define is a little too simple.
 */
#define LOW_LEVEL		0   //将trig拉低
#define HIGH_LEVEL		1   //将trig拉高
#define BEEP_ON			2   //NA
#define BEEP_OFF		3   //NA
#define GOODREAD        4   //NA
#define PWR_ON			5   //将pwr和wk同时拉高
#define PWR_OFF			6   //将pwr和wk同时拉低
#define WK_ON			7   //将wk拉高
#define WK_OFF			8   //将wk拉低
#define GET_PW          9   //得到上电状态

static int dbg = 0;
module_param(dbg, int, S_IRUGO);

int* rtsreg;
int* d0reg;
int* ctsreg;
int* d1reg;
int* ucr2;

struct bar_ctl_dev
{
		struct cdev cdev;
		struct class *class;
		dev_t devno;
		volatile uint8_t *reg_scan_cs;
}*bar_devp;

struct bar_ctl_dev  bar_dev;

static inline void set_trig(bool val)
{
		if(val)
		{
				iowrite8(ioread8(bar_dev.reg_scan_cs)|(1<<4),
								bar_dev.reg_scan_cs);
		}
		else
		{
				iowrite8(ioread8(bar_dev.reg_scan_cs)&(~(1<<4)),
								bar_dev.reg_scan_cs);
		}
}

static inline void set_wk(bool val)
{
		if(sand_machine()==SAND_PS4KV2)
		{
			return;
		}
		else
		{
			if(val)
			{
					iowrite8(ioread8(bar_dev.reg_scan_cs)|(1<<5),
									bar_dev.reg_scan_cs);
			}
			else
			{
					iowrite8(ioread8(bar_dev.reg_scan_cs)&(~(1<<5)),
									bar_dev.reg_scan_cs);
			}
		}
}

static inline void set_scanpwr(bool val)
{
		if(sand_machine()==SAND_PS4KV2)
		{
			return;
		}
		else
		{
			if(val)
			{
					iowrite8(ioread8(bar_dev.reg_scan_cs)|(1<<7),
									bar_dev.reg_scan_cs);
			}
			else
			{
					iowrite8(ioread8(bar_dev.reg_scan_cs)&(~(1<<7)),
									bar_dev.reg_scan_cs);
			}
		}
}

static inline void pr_allreg(void)
{
	if(dbg)
	{
		pr_alert("reg_scan_cs=0x%02X\n",ioread8(bar_dev.reg_scan_cs));
	}
}

static int bar_ioctl (struct inode *inode , struct file *file, unsigned int cmd,unsigned long data)
{     
	   u8 pwr_va=0;
		switch(cmd)
		{
				case LOW_LEVEL:
						if(dbg)
						{
							printk(KERN_ALERT "\n\n--------------  SET LOW_LEVEL ------------\n");
						}
						set_trig(false);
						pr_allreg();
						break;
				case HIGH_LEVEL:
						if(dbg)
						{
							printk(KERN_ALERT "\n\n--------------  SET HIGH_LEVEL ------------\n");
						}
						set_trig(true);
						pr_allreg();
						break;
				case BEEP_ON:
						if(dbg)
						{
							printk("BEEP_ON!!!!\n");
						}
						break;
				case BEEP_OFF:
						if(dbg)
						{
							printk("BEEP_OFF!!!!\n");
						}
						break;
				case PWR_ON:
						set_scanpwr(true);
						set_wk(true);
						break;
				case PWR_OFF:
						set_wk(false);
						set_scanpwr(false);
						break;
				case WK_ON:
						set_wk(true);
						break;
				case WK_OFF:
						set_wk(false);
						break;
				case GET_PW:
				pwr_va = (ioread8(bar_dev.reg_scan_cs) >> 7 ) & 0x1;
			//	printk("[%s,%d] pwr_va:0x%x\r\n",__func__,__LINE__, pwr_va);
				copy_to_user((u8*)data,&pwr_va, 1) ;
                break;
				default:
						if(dbg)
						{
							printk("ENTER INTO default!!!\n");
						}
						return 1;
		}
		return 0;
}

static struct file_operations bar_ops ={
		.owner = THIS_MODULE,
		.ioctl = bar_ioctl,
};


static int bar_setup_cdev(struct cdev *dev, int index)
{
		int err=0;

		cdev_init(dev,&bar_ops);

		dev->owner = THIS_MODULE;
		dev->ops = &bar_ops;
		err = cdev_add(dev,bar_dev.devno,1);
		if(err)
		{
				printk(KERN_ALERT "Error %d adding GZ%d", err, index);
				goto a;
		}

		bar_dev.class = class_create(THIS_MODULE, "bar_class");
		if (IS_ERR(bar_dev.class))
		{
				err = -EINVAL;
				pr_err("bar: failed in creating class.\n");
				goto b;
		}

		if(NULL==device_create(bar_dev.class, NULL, bar_dev.devno, NULL, "bar"))
		{
				err = -EINVAL;
				pr_err("bar: failed in creating class device.\n");
				goto c;
		}
		return err;
c:
		device_destroy(bar_dev.class,bar_dev.devno);
		class_destroy(bar_dev.class);
b:
		cdev_del(&bar_dev.cdev);
a:
		set_scanpwr(false);
		return err;
}


static int __bar_init(void)
{
		int ret = -ENODEV;

		ret = alloc_chrdev_region(&(bar_dev.devno), 0, 1, "bar");
		if(ret < 0)
		{
				pr_alert("S3C2410 init_module failed with %d\n",ret);
				return ret;
		}
		else
		{	if(dbg)
			{
				pr_alert("bar success!!! major=%d,minor=%d\n",MAJOR(bar_dev.devno),MINOR(bar_dev.devno));
			}
		}
		
		if(sand_machine()==SAND_IPS420)
		{
			bar_dev.reg_scan_cs = ioremap_nocache(IPS420_SCAN_CS_ADDR,1);
		}
		else if(sand_machine()==SAND_PS4KV2)
		{
			bar_dev.reg_scan_cs = ioremap_nocache(PS4KV2_SCAN_CS_ADDR,1);
		}
		else
		{
			bar_dev.reg_scan_cs = ioremap_nocache(SCAN_CS_ADDR,1);
		}
		set_trig(true);
		set_scanpwr(false);
		set_wk(false);
		pr_allreg();

		bar_devp = &bar_dev;
		ret=bar_setup_cdev(&(bar_dev.cdev), 0);

		return ret;
}


static int __init bar_init(void) 
{
		int  ret = -ENODEV;
        rtsreg=ioremap(0x43FAC178,4);
        d0reg=ioremap(0x43FAC488,4);
        ctsreg=ioremap(0x43FAC17C,4);
        d1reg=ioremap(0x43FAC48C,4);
        ucr2=ioremap(0x43F90084,4);

        ret =  __bar_init();
		if (ret)
		{
				return ret;
		}
		return 0;
}


static void __exit cleanup_bar_ctl(void)
{
        iounmap(rtsreg);
        iounmap(d0reg);
        iounmap(ctsreg);
        iounmap(d1reg);
        iounmap(ucr2);
		set_scanpwr(false);
		device_destroy(bar_dev.class,bar_dev.devno);
		class_destroy(bar_dev.class);
		iounmap(bar_dev.reg_scan_cs);
		cdev_del(&bar_devp->cdev);
		unregister_chrdev_region(bar_dev.devno, 1);
}

MODULE_AUTHOR("zengshu");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("MODULE_RELEASE");

module_init(bar_init);
module_exit(cleanup_bar_ctl);
/** @}*/
