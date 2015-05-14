/** @defgroup 
 *  @author  刘一峰 (Email:liuyifeng@hotmail.com)
 *  @version 1.0
 *  @date    2011-10-20
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

#include <mach-mx25/board-mx25_3stack.h>
#include <mach-mx25/iomux.h>
#include <mach-mx25/mx25_pins.h>
#include <mach-mx25/machine.h>

int dbg = 0;
module_param(dbg, int, 0);

/**
  定义	  Bit 7	Bit 6	Bit 5	Bit 4	Bit 3	Bit 2	Bit 1	Bit 0
  R/W	    R/W	R/W	R/W	R/W	R/W	R/W	R/W	R/W
  bit	    GSM_PWR	GSM_RST	GSM_ONOFF	GSM_WAKE	TBD	TBD	TBD	TBD
  描述	   电源控制	复位	开关	唤醒	预留	预留	预留	预留
 */
#define GPRS_CS_ADDR 0xb2700001                    

/** IPS420 */
#define IPS420_GPRS_CS_ADDR1 0xa0700000
#define IPS420_GPRS_CS_ADDR2 0xa0700001
#define IPS420_PIN_GPRS_PWR	MX25_PIN_POWER_FAIL

/** PS4KV2 */
#define PS4KV2_GPRS_CS_ADDR 0xa0000080

#define GSM_IOC_MAGIC		'G'
#define GSM_PWR_ON			_IOW(GSM_IOC_MAGIC,1, int)
#define GSM_PWR_OFF			_IOW(GSM_IOC_MAGIC,2, int)
#define GSM_SOFTONOFF_ON	_IOW(GSM_IOC_MAGIC,3, int)
#define GSM_SOFTONOFF_OFF	_IOW(GSM_IOC_MAGIC,4, int)
#define GSM_RST_ON			_IOW(GSM_IOC_MAGIC,5, int)
#define GSM_RST_OFF			_IOW(GSM_IOC_MAGIC,6, int)
#define GSM_DTR_ON			_IOW(GSM_IOC_MAGIC,7, int)
#define GSM_DTR_OFF			_IOW(GSM_IOC_MAGIC,8, int)
#define GSM_DCD_ON			_IOW(GSM_IOC_MAGIC,9, int)
#define GSM_DCD_OFF			_IOW(GSM_IOC_MAGIC,10, int)
#define GSM_CTS_ON			_IOW(GSM_IOC_MAGIC,11, int)
#define GSM_CTS_OFF			_IOW(GSM_IOC_MAGIC,12, int)
#define GSM_RTS_ON			_IOW(GSM_IOC_MAGIC,13, int)
#define GSM_RTS_OFF			_IOW(GSM_IOC_MAGIC,14, int)
#define GSM_GET_CTS			_IOR(GSM_IOC_MAGIC,15, int)
#define GSM_GET_RTS			_IOR(GSM_IOC_MAGIC,16, int)
#define GSM_IOC_MAXNR		16

struct gprs_ctl_dev
{
		struct cdev cdev;
		struct class *class;
		dev_t devno;
		volatile uint8_t *reg_gprs_cs1; 
		volatile uint8_t *reg_gprs_cs2; 
}*gprs_devp;

struct gprs_ctl_dev  gprs_dev;

static inline void pr_allreg(void);

static inline void set_gprspwr(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
				if(val)
				{
						gpio_set_value(IOMUX_TO_GPIO(IPS420_PIN_GPRS_PWR),1);
				}
				else
				{
						gpio_set_value(IOMUX_TO_GPIO(IPS420_PIN_GPRS_PWR),0);
				}
		}
		else if(sand_machine()==SAND_PS4KV2)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)|(1<<7),
										gprs_dev.reg_gprs_cs1);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)&(~(1<<7)),
										gprs_dev.reg_gprs_cs1);
				}
		}
		else
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)|(1<<4),
										gprs_dev.reg_gprs_cs1);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)&(~(1<<4)),
										gprs_dev.reg_gprs_cs1);
				}
		}
		if(dbg)
				pr_allreg();
}

static inline void set_gprsrst(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)|(1<<5),
										gprs_dev.reg_gprs_cs2);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)&(~(1<<5)),
										gprs_dev.reg_gprs_cs2);
				}
		}
		else if(sand_machine()==SAND_PS4KV2)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)|(1<<6),
										gprs_dev.reg_gprs_cs1);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)&(~(1<<6)),
										gprs_dev.reg_gprs_cs1);
				}
		}
		if(dbg)
				pr_allreg();
}

static inline void set_gprscts(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)|(1<<3),
                                        gprs_dev.reg_gprs_cs2);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)&(~(1<<3)),
                                        gprs_dev.reg_gprs_cs2);
				}
		}
		if(dbg)
                pr_allreg();
}

static inline void set_gprsrts(bool val)
{
        if(sand_machine()==SAND_IPS420)
        {
                if(val)
                {
                        iowrite8(ioread8(gprs_dev.reg_gprs_cs2)|(1<<4),
                                        gprs_dev.reg_gprs_cs2);
                }
                else
                {
                        iowrite8(ioread8(gprs_dev.reg_gprs_cs2)&(~(1<<4)),
                                        gprs_dev.reg_gprs_cs2);
                }
        }
        if(dbg)
                pr_allreg();
}


static inline void set_gprsonoff(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)|(1<<6),
										gprs_dev.reg_gprs_cs2);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)&(~(1<<6)),
										gprs_dev.reg_gprs_cs2);
				}
		}
		else if(sand_machine()==SAND_PS4KV2)
		{
				return;
		}
		else
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)|(1<<6),
										gprs_dev.reg_gprs_cs1);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)&(~(1<<6)),
										gprs_dev.reg_gprs_cs1);
				}
		}
		if(dbg)
				pr_allreg();
}

//not implatment
#if 0
static inline void set_gprswake(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
		}
		else if(sand_machine()==SAND_PS4KV2)
		{
				return;
		}
		else
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)|(1<<7),
										gprs_dev.reg_gprs_cs1);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)&(~(1<<7)),
										gprs_dev.reg_gprs_cs1);
				}
				if(dbg)
						pr_allreg();
		}
}
#endif

static inline void set_gprsdcd(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)|(1<<4),
										gprs_dev.reg_gprs_cs1);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs1)&(~(1<<4)),
										gprs_dev.reg_gprs_cs1);
				}
				if(dbg)
						pr_allreg();
		}
}

static inline void set_gprsdtr(bool val)
{
		if(sand_machine()==SAND_IPS420)
		{
				if(val)
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)|(1<<4),
										gprs_dev.reg_gprs_cs2);
				}
				else
				{
						iowrite8(ioread8(gprs_dev.reg_gprs_cs2)&(~(1<<4)),
										gprs_dev.reg_gprs_cs2);
				}
				if(dbg)
						pr_allreg();
		}
}

static inline void pr_allreg(void)
{
		if(dbg)
		{
				if(sand_machine()==SAND_IPS420)
				{
						pr_alert("reg_gprs_cs2=0x%02X\n",ioread8(gprs_dev.reg_gprs_cs2));
				}
		}
}

static int gprs_ioctl (struct inode *inode , struct file *file, unsigned int cmd, unsigned long data)
{
		int ret = 0;

		if (_IOC_TYPE(cmd) != GSM_IOC_MAGIC)
		{
				if(dbg)
						pr_err("IOC_MAGIC not match.\n");
				return -ENOTTY;
		}
		if (_IOC_NR(cmd) > GSM_IOC_MAXNR)
		{
				if(dbg)
						pr_err("cmd is large than IOC_MAXNR.\n");
				return -ENOTTY;
		}

		if (_IOC_DIR(cmd) & _IOC_READ)
				ret = !access_ok(VERIFY_WRITE, (void __user *)data, _IOC_SIZE(cmd));
		else if (_IOC_DIR(cmd) & _IOC_WRITE)
				ret =  !access_ok(VERIFY_READ, (void __user *)data, _IOC_SIZE(cmd));
		if (ret)
		{
				if(dbg)
						pr_err("READ WRITE check fault.\n");
				return -EFAULT;
		}     
		switch(cmd)
		{
				case GSM_PWR_ON:
						set_gprspwr(true);
						break;
				case GSM_PWR_OFF:
						set_gprspwr(false);
						break;
				case GSM_SOFTONOFF_ON:
						set_gprsonoff(true);
						break;
				case GSM_SOFTONOFF_OFF:
						set_gprsonoff(false);
						break;
				case GSM_RST_ON:
						set_gprsrst(true);
						break;
				case GSM_RST_OFF:
						set_gprsrst(false);
						break;
				case GSM_DTR_ON:
						set_gprsdtr(true);
						break;
				case GSM_DTR_OFF:
						set_gprsdtr(false);
						break;
				case GSM_DCD_ON:
						set_gprsdcd(true);
						break;
				case GSM_DCD_OFF:
						set_gprsdcd(false);
						break;
				case GSM_CTS_ON:
						set_gprscts(true);
						break;
				case GSM_CTS_OFF:
						set_gprscts(false);
						break;
				case GSM_RTS_ON:
						set_gprsrts(true);
						break;
				case GSM_RTS_OFF:
						set_gprsrts(false);
						break;
				case GSM_GET_CTS:
						{
							unsigned int status = (ioread8(gprs_dev.reg_gprs_cs2)>>3)&0x01;
        					ret = copy_to_user((unsigned int *)data,(unsigned int *)&status, sizeof(unsigned int));
						}
						break;
				case GSM_GET_RTS:
						{
							unsigned int status = (ioread8(gprs_dev.reg_gprs_cs2)>>4)&0x01;
        					ret = copy_to_user((unsigned int *)data,(unsigned int *)&status, sizeof(unsigned int));
						}
						break;
				default:
						return -1;
		}
		return 0;
}

static struct file_operations gprs_ops ={
		.owner = THIS_MODULE,
		.ioctl = gprs_ioctl,
};


static int gprs_setup_cdev(struct cdev *dev, int index)
{
		int err=0;

		cdev_init(dev,&gprs_ops);

		dev->owner = THIS_MODULE;
		dev->ops = &gprs_ops;
		err = cdev_add(dev,gprs_dev.devno,1);
		if(err)
		{
				if(dbg)
						printk(KERN_NOTICE "Error %d adding GZ%d", err, index);
				goto a;
		}

		gprs_dev.class = class_create(THIS_MODULE, "gprs_class");
		if (IS_ERR(gprs_dev.class))
		{
				err = -EINVAL;
				if(dbg)
						pr_err("gprs: failed in creating class.\n");
				goto b;
		}

		if(NULL==device_create(gprs_dev.class, NULL, gprs_dev.devno, NULL, "gprs"))
		{
				err = -EINVAL;
				if(dbg)
						pr_err("gprs: failed in creating class device.\n");
				goto c;
		}
		return err;
c:
		class_destroy(gprs_dev.class);
b:
		cdev_del(&gprs_dev.cdev);
a:
		set_gprspwr(false);
		return err;
}


static int __gprs_init(void)
{
		int ret = -ENODEV;

		ret = alloc_chrdev_region(&(gprs_dev.devno), 0, 1, "gprs");

		if(ret < 0)
		{
				if(dbg)
						pr_alert("imx258 init_module failed with %d\n",ret);
				return ret;
		}

		if(sand_machine()==SAND_IPS420)
		{
				gprs_dev.reg_gprs_cs1 = ioremap_nocache(IPS420_GPRS_CS_ADDR1,1);
				gprs_dev.reg_gprs_cs2 = ioremap_nocache(IPS420_GPRS_CS_ADDR2,1);
				mxc_free_iomux(IPS420_PIN_GPRS_PWR, MUX_CONFIG_GPIO);			
				mxc_request_iomux(IPS420_PIN_GPRS_PWR, MUX_CONFIG_ALT5);
				gpio_request(IOMUX_TO_GPIO(IPS420_PIN_GPRS_PWR), "gprs_pwr");
				gpio_direction_output(IOMUX_TO_GPIO(IPS420_PIN_GPRS_PWR), 0);	
		}
		else if(sand_machine()==SAND_PS4KV2)
		{
				gprs_dev.reg_gprs_cs1 = ioremap_nocache(PS4KV2_GPRS_CS_ADDR,1);
		}
		else
		{
				gprs_dev.reg_gprs_cs1 = ioremap_nocache(GPRS_CS_ADDR,1);
		}

		set_gprscts(false);
		set_gprsrts(false);
		set_gprsdcd(false);
		set_gprspwr(true);
		set_gprsonoff(true);
		set_gprsrst(true);
		msleep(300);
		set_gprsrst(false);

		gprs_devp = &gprs_dev;
		ret=gprs_setup_cdev(&(gprs_dev.cdev), 0);

		return ret;
}


static int __init gprs_init(void) 
{
		int  ret = -ENODEV;
		ret =  __gprs_init();
		if (ret)
		{
				return ret;
		}
		return 0;
}


static void __exit cleanup_gprs_ctl(void)
{
		set_gprspwr(false);

		device_destroy(gprs_dev.class,gprs_dev.devno);
		class_destroy(gprs_dev.class);	

		if(sand_machine()==SAND_IPS420)
		{
				gpio_request(IOMUX_TO_GPIO(IPS420_PIN_GPRS_PWR), NULL);
				mxc_free_iomux(IPS420_PIN_GPRS_PWR, MUX_CONFIG_GPIO);
				iounmap(gprs_dev.reg_gprs_cs1);
				iounmap(gprs_dev.reg_gprs_cs2);
		}
		else
		{
				iounmap(gprs_dev.reg_gprs_cs1);
		}
		cdev_del(&gprs_devp->cdev);
		unregister_chrdev_region(MKDEV(gprs_dev.devno,0), 1);
}

MODULE_AUTHOR("liuyifeng");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("MODULE_RELEASE");

module_init(gprs_init);
module_exit(cleanup_gprs_ctl);
/** @}*/
