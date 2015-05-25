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
#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/irqs.h>

#include <linux/clk.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/signal.h>

#include "iomux.h"
#include "mx25_pins.h"

#include "beep_backlight.h"

#include <mach-mx25/machine.h>

#include <mach/clock.h>

extern void set_driver_status(unsigned int pwr);

int pwm_major = 0;
int pwm_minor = 0;


extern int sand_machine(void);


extern void Buzzer_Contrast_Set(unsigned int contrast);
extern void Buzzer_Freq_Set(unsigned long freq);
extern void Buzzer_On(unsigned long freq);
extern void Buzzer_Off(void);



extern void Backlight_Contrast_Set(unsigned int contrast);
extern void Backlight_Freq_Set(unsigned long freq);
extern void Backlight_On(unsigned long freq);
extern void Backlight_Off(void);



extern void Buzzer_Gate_Set(bool state);


struct pwm_struct {

		struct cdev cdev ;
		struct class *pwm_class;
};

struct pwm_struct *pwmptr;

static int beep_backlight_open(struct inode *inode, struct file *filp)
{
		return 0;
}


static int beep_backlight_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t beep_backlight_ioctl(struct inode *inode, struct file *file,
				                            unsigned int cmd, unsigned long arg)
{
        int ret;
        
        unsigned int temp=0;

        unsigned char level;


        if (_IOC_TYPE(cmd) != BEEP_BACKLIGHT_IOC_NUM) {
                printk(KERN_ERR
                       "device_ioctl() - Error! IOC_TYPE = %d. Expected %d\n",
                       _IOC_TYPE(cmd), BEEP_BACKLIGHT_IOC_NUM);
                return -ENOTTY;
        }

        if (_IOC_NR(cmd) > BEEP_BACKLIGHT_IOC_MAXNUM) {
                printk(KERN_ERR
                       "device_ioctl() - Error!"
                       "IOC_NR = %d greater than max supported(%d)\n",
                       _IOC_NR(cmd), BEEP_BACKLIGHT_IOC_MAXNUM);
                return -ENOTTY;
        }

        switch(cmd)
        {
            case BEEP_ON:
            {
                pr_devel("%s Buzzer_On\n", __func__);
                Buzzer_On(0);
                break;
            }

            case BEEP_OFF:
            {
                pr_devel("%s Buzzer_Off\n", __func__);
                Buzzer_Off();
                break;
            }

            case BACKLIGHT_SET:
            {
                ret = copy_from_user(&temp,(unsigned int *)arg,sizeof(unsigned int));

                level = ((unsigned char)temp) * 100 / 256;               

                pr_devel("%s BACKLIGHT_SET = %d\n", __func__, (unsigned char)temp);
                Backlight_Contrast_Set(level);
                break;
            }

            case  BEEP_GATE:
            {
                ret = copy_from_user(&temp,(unsigned int *)arg,sizeof(unsigned int));
                
                if(temp == BEEP_GATE_CLOSE)
                {
                    pr_devel("%s Buzzer_Gate OFF\n", __func__);
                    Buzzer_Gate_Set(false);
                }
                else
                {
                    pr_devel("%s Buzzer_Gate ON\n", __func__);
                    Buzzer_Gate_Set(true);
                }
                
                break;
            }

            case  BACKLIGHT_GATE:
            {
                ret = copy_from_user(&temp,(unsigned int *)arg,sizeof(unsigned int));
                
                if(temp == BACKLIGHT_GATE_CLOSE)
                {
                    pr_devel("%s Buzzer_Gate OFF\n", __func__);
                    Backlight_Off();
                }
                else
                {
                    pr_devel("%s Buzzer_Gate ON\n", __func__);
                    Backlight_On(0);
                }
                
                break;
            }
            
            case  DRIVER_USED:
            {
                ret = copy_from_user(&temp,(unsigned int *)arg,sizeof(unsigned int));
                set_driver_status(temp);
                
                break;
            }            
            
            default:
            {
                printk("%s error cmd: %d\n", __func__, cmd);
                break;
            }
        }

		return 0;
}


static void mxc_pwm_release(struct device * dev)
{
    return ;
}


static struct platform_device pwm_device ={
                .name = PLATFORM_NAME,
                .id   = 0,
                .dev  = {
                           .release  =  mxc_pwm_release,
                        },
};



static struct file_operations beep_backlight_fops = {
		.owner      =       THIS_MODULE,
		.ioctl      =       beep_backlight_ioctl,
		.open       =       beep_backlight_open,
		.release    =       beep_backlight_release,
};


static int pwm_probe (struct platform_device *pdev)
{
		int ret;
		dev_t dev = 0;

		/*
		 * Get a range of minor numbers to work with, asking for a dynamic
		 * major unless directed otherwise at load time.
		 */
		if (pwm_major) {
				dev = MKDEV(pwm_major, pwm_minor);
				ret = register_chrdev_region(dev, 1, DEVICE_NAME);
		} else {
				ret = alloc_chrdev_region(&dev, pwm_minor, 1, DEVICE_NAME);
				pwm_major = MAJOR(dev);
		}

		if (ret < 0) {
				printk(KERN_WARNING "beep_backlight driver: can't get major %d\n", pwm_major);
				return ret;
		}

		pwmptr = (struct pwm_struct *)kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!pwmptr) {
				ret = -ENOMEM;
				goto fail_malloc;
		}

		memset(pwmptr, 0, sizeof(struct pwm_struct) );


		cdev_init(&pwmptr->cdev, &beep_backlight_fops);
		pwmptr->cdev.owner = THIS_MODULE;
		pwmptr->cdev.ops = &beep_backlight_fops;
		ret = cdev_add(&pwmptr->cdev, dev, 1);
		if (ret) {
				printk(KERN_NOTICE "beep_backlight cdev error.\n");
				goto fail_add;
		}

		pwmptr->pwm_class=class_create(THIS_MODULE, "beep_backlight");
		if (IS_ERR(pwmptr->pwm_class)) {
				printk("Err: failed in creating class.\n");
				return -1;
		}

		/* register your own device in sysfs, and this will cause udevd to create corresponding device node */
		device_create(pwmptr->pwm_class, NULL, MKDEV(pwm_major, pwm_minor), NULL, DEVICE_NAME );

		printk("beep and keyboard backlight probe ok\n");

		return 0 ;


fail_add:
		unregister_chrdev_region(dev, 1);

fail_malloc:
		return ret;
}

static int  pwm_remove(struct platform_device *pdev)
{
		cdev_del(&pwmptr->cdev);
		unregister_chrdev_region(MKDEV (pwm_major, pwm_minor), 1);
		device_destroy(pwmptr->pwm_class, MKDEV(pwm_major, pwm_minor));
		class_destroy(pwmptr->pwm_class);
		kfree(pwmptr);

		return 0;
}
static struct platform_driver  pwm_driver ={
                .probe     = pwm_probe,
                .remove    = pwm_remove,
                .driver    = {
                             .name   = PLATFORM_NAME,
                             .owner  = THIS_MODULE,
                          },
};




static int __init beep_backlight_init(void)
{
    int ret;

    ret = platform_device_register(&pwm_device);

    if(ret != 0)
    {
        printk("%s:platform_device_register result: %d\n", __func__, ret);
        return ret;
    }

    ret = platform_driver_register(&pwm_driver);

    if(ret != 0)
    {
        printk("%s:platform_driver_register result: %d\n",  __func__, ret);
        platform_device_unregister(&pwm_device);
    }

    return ret;
}

static void __exit beep_backlight_exit(void)
{
    platform_driver_unregister(&pwm_driver );

    platform_device_unregister(&pwm_device);
}

module_init(beep_backlight_init);
module_exit(beep_backlight_exit);

MODULE_AUTHOR("xuxiaobo <xuxiaobo@sand.com.cn>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("pwm beep and keyboard backlight for imx258");
