/** @defgroup camflash.c
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

static int dbg = 0;
module_param(dbg, int, S_IRUGO);

#define FLASH_ON	0
#define FLASH_OFF	1

#define IPS420_PIN_CAMFLASH MX25_PIN_A15

struct camflash_ctl_dev
{
		struct cdev cdev;
		struct class *class;
		dev_t devno;
}*camflash_devp;

struct camflash_ctl_dev  camflash_dev;

static inline void set_camflash(bool val)
{
		if(val)
		{
			gpio_set_value(IOMUX_TO_GPIO(IPS420_PIN_CAMFLASH),1);
		}
		else
		{
			gpio_set_value(IOMUX_TO_GPIO(IPS420_PIN_CAMFLASH),0);
		}
}
EXPORT_SYMBOL(set_camflash);

static int camflash_ioctl (struct inode *inode , struct file *file, unsigned int cmd, long data)
{     
		switch(cmd)
		{
				case FLASH_ON:
						set_camflash(true);
						break;
				case FLASH_OFF:
						set_camflash(false);
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

static ssize_t camflash_write (struct file *file ,const char *buf, size_t count, loff_t *f_ops)
{
	set_camflash(*buf-'0');
	return count;
}

static struct file_operations camflash_ops ={
		.owner = THIS_MODULE,
		.ioctl = camflash_ioctl,
		.write = camflash_write,
};


static int camflash_setup_cdev(struct cdev *dev, int index)
{
		int err=0;

		cdev_init(dev,&camflash_ops);

		dev->owner = THIS_MODULE;
		dev->ops = &camflash_ops;
		err = cdev_add(dev,camflash_dev.devno,1);
		if(err)
		{
				printk(KERN_NOTICE "Error %d adding GZ%d", err, index);
				goto a;
		}

		camflash_dev.class = class_create(THIS_MODULE, "camflash_class");
		if (IS_ERR(camflash_dev.class))
		{
				err = -EINVAL;
				pr_err("camflash: failed in creating class.\n");
				goto b;
		}

		if(NULL==device_create(camflash_dev.class, NULL, camflash_dev.devno, NULL, "camflash"))
		{
				err = -EINVAL;
				pr_err("camflash: failed in creating class device.\n");
				goto c;
		}
		return err;
c:
		device_destroy(camflash_dev.class,camflash_dev.devno);
		class_destroy(camflash_dev.class);
b:
		cdev_del(&camflash_dev.cdev);
a:
		return err;
}


static int __camflash_init(void)
{
		int ret = -ENODEV;

		ret = alloc_chrdev_region(&(camflash_dev.devno), 0, 1, "camflash");
		if(ret < 0)
		{
				pr_alert("S3C2410 init_module failed with %d\n",ret);
				return ret;
		}
		else
		{	if(dbg)
			{
				pr_alert("camflash success!!! major=%d,minor=%d\n",MAJOR(camflash_dev.devno),MINOR(camflash_dev.devno));
			}
		}
	
		camflash_devp = &camflash_dev;
		ret=camflash_setup_cdev(&(camflash_dev.cdev), 0);

		if(sand_machine()==SAND_IPS420)
		{
			mxc_free_iomux(IPS420_PIN_CAMFLASH, MUX_CONFIG_GPIO);
                	mxc_request_iomux(IPS420_PIN_CAMFLASH, MUX_CONFIG_ALT5);
                	gpio_request(IOMUX_TO_GPIO(IPS420_PIN_CAMFLASH), "camflash");
                	gpio_direction_output(IOMUX_TO_GPIO(IPS420_PIN_CAMFLASH), 0);
		}
		else
		{	
			if(dbg)
			{
				pr_alert("This machine is not IPS420!\n");
			}
			ret = -ENODEV;
		}

		return ret;
}


static int __init camflash_init(void) 
{
		int  ret = -ENODEV;
		ret =  __camflash_init();
		if (ret)
		{
				return ret;
		}
		return 0;
}


static void __exit cleanup_camflash_ctl(void)
{
		device_destroy(camflash_dev.class,camflash_dev.devno);
		class_destroy(camflash_dev.class);
		cdev_del(&camflash_devp->cdev);
		unregister_chrdev_region(camflash_dev.devno, 1);
}

MODULE_AUTHOR("zengshu");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("MODULE_RELEASE");

module_init(camflash_init);
module_exit(cleanup_camflash_ctl);
/** @}*/
