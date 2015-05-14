/** @defgroup modem.c
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.0
 *  @date    2011-10-17
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
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include "mach-mx25/board-mx25_3stack.h"
#include "mach-mx25/iomux.h"
#include "mach-mx25/mx25_pins.h"
#include "mach-mx25/machine.h"

#define MODEM_CS_ADDR 0xb2500000

#define IOCTL_RESET 	0
#define IOCTL_POWER 	1

struct modem_ctl_dev
{
		struct cdev cdev;
		struct class *class;
		dev_t devno;
		volatile uint8_t *reg_modem_cs1;
}*modem_devp;

struct modem_ctl_dev  modem_dev;

static inline void set_reset(bool val)
{
		if(val)
		{
				iowrite8(ioread8(modem_dev.reg_modem_cs1)|(1<<6),
								modem_dev.reg_modem_cs1);
		}
		else
		{
				iowrite8(ioread8(modem_dev.reg_modem_cs1)&(~(1<<6)),
								modem_dev.reg_modem_cs1);
		}
}

static inline void set_power(bool val)
{
		if(val)
		{
				iowrite8(ioread8(modem_dev.reg_modem_cs1)|(1<<7),
								modem_dev.reg_modem_cs1);
		}
		else
		{
				iowrite8(ioread8(modem_dev.reg_modem_cs1)&(~(1<<7)),
								modem_dev.reg_modem_cs1);
		}
		
}

static int modem_ioctl (struct inode *inode , struct file *file, unsigned int cmd,unsigned long data)
{     
		switch(cmd)
		{
				case IOCTL_RESET:
						set_reset(data);
						break;
				case IOCTL_POWER:
						set_power(data);
						break;
				default:
						printk("ENTER INTO default!!!\n");
						return 1;
		}
		return 0;
}

static struct file_operations modem_ops ={
		.owner = THIS_MODULE,
		.ioctl = modem_ioctl,
};


static int modem_setup_cdev(struct cdev *dev, int index)
{
		int err=0;

		cdev_init(dev,&modem_ops);

		dev->owner = THIS_MODULE;
		dev->ops = &modem_ops;
		err = cdev_add(dev,modem_dev.devno,1);
		if(err)
		{
				printk(KERN_NOTICE "Error %d adding GZ%d", err, index);
				goto a;
		}

		modem_dev.class = class_create(THIS_MODULE, "modem_class");
		if (IS_ERR(modem_dev.class))
		{
				err = -EINVAL;
				pr_err("modem: failed in creating class.\n");
				goto b;
		}

		if(NULL==device_create(modem_dev.class, NULL, modem_dev.devno, NULL, "modem"))
		{
				err = -EINVAL;
				pr_err("modem: failed in creating class device.\n");
				goto c;
		}
		return err;
c:
		device_destroy(modem_dev.class,modem_dev.devno);
		class_destroy(modem_dev.class);
b:
		cdev_del(&modem_dev.cdev);
a:
		return err;
}


static int __init __modem_init(void)
{
		int ret = -ENODEV;

		ret = alloc_chrdev_region(&(modem_dev.devno), 0, 1, "modem");
		if(ret < 0)
		{
				pr_alert("S3C2410 init_module failed with %d\n",ret);
				return ret;
		}
		else
		{
				pr_alert("modem success!!! major=%d,minor=%d\n",MAJOR(modem_dev.devno),MINOR(modem_dev.devno));
		}
		
		modem_dev.reg_modem_cs1 = ioremap_nocache(MODEM_CS_ADDR,1);

		set_power(true);

		set_reset(false);
		msleep(1);
		set_reset(true);

		modem_devp = &modem_dev;
		ret=modem_setup_cdev(&(modem_dev.cdev), 0);

		return ret;
}


static int __init modem_init(void) 
{
		int  ret = -ENODEV;
		ret =  __modem_init();
		if (ret)
		{
				return ret;
		}
		return 0;
}


static void __exit cleanup_modem_ctl(void)
{
		device_destroy(modem_dev.class,modem_dev.devno);
		class_destroy(modem_dev.class);
		iounmap(modem_dev.reg_modem_cs1);
		iounmap(modem_dev.reg_modem_cs1);
		cdev_del(&modem_devp->cdev);
		unregister_chrdev_region(modem_dev.devno, 1);
}

MODULE_AUTHOR("zengshu");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("MODULE_RELEASE");

module_init(modem_init);
module_exit(cleanup_modem_ctl);
/** @}*/
