
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>		/* Async notification */
#include <linux/uaccess.h>	/* for get_user, put_user, access_ok */
#include <linux/sched.h>	/* jiffies */
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <mach/hardware.h>

#include <mach/gpio.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <mach-mx25/mx25_pins.h>
#include <mach-mx25/iomux.h>

static int dbg = 1;
module_param(dbg, int, S_IRUGO);

#define jtag_debug(fmt, arg...)  do{if(dbg)printk(KERN_DEBUG fmt,##arg);}while(0)
#define jtag_info(fmt, arg...)       do{if(dbg)printk(KERN_INFO fmt,##arg);}while(0)

/* The name of the device file */
#define KEYCRYPT_DEVICE_FILE_NAME "cpldjtag"

struct jtagio_struct
{
		dev_t dev_id;
		struct cdev cdev ;
		struct class *cls;	
		struct device *clsdev;   
};

struct jtagio_struct *jtagptr=NULL;
dev_t  imx_jtagio_major;
dev_t  imx_jtagio_minor = 0;

//void config_jtag_gpio(void);
//void set_jtag_gpio(unsigned char ucValue);

#define  	TMS      0x02
#define  	TDI      0x01
#define  	TCK      0x04
#define   	TDO      0x08

static void config_jtag_gpio(void)
{
		/* set D10 as input  for TDO*/
		mxc_free_iomux(MX25_PIN_D10,MUX_CONFIG_ALT5);
		mxc_request_iomux(MX25_PIN_D10, MUX_CONFIG_ALT5);
		mxc_iomux_set_pad(MX25_PIN_D10, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D10), NULL);
		gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D10));	 

		/* set D11 as output  for TCK*/
		mxc_free_iomux(MX25_PIN_D11,MUX_CONFIG_ALT5);
		mxc_request_iomux(MX25_PIN_D11, MUX_CONFIG_ALT5);
		mxc_iomux_set_pad(MX25_PIN_D11, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D11), NULL);
		gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_D11), 0);	 

		/* set D12 as output  for  TDI*/
		mxc_free_iomux(MX25_PIN_D12,MUX_CONFIG_ALT5);
		mxc_request_iomux(MX25_PIN_D12, MUX_CONFIG_ALT5);
		mxc_iomux_set_pad(MX25_PIN_D12, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D12), NULL);
		gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_D12), 0);	 

		/* set D13 as intput for TMS  */
		mxc_free_iomux(MX25_PIN_D13,MUX_CONFIG_ALT5);
		mxc_request_iomux(MX25_PIN_D13, MUX_CONFIG_ALT5);
		mxc_iomux_set_pad(MX25_PIN_D13, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D13), NULL);
		gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_D13),0);	 
}

static void config_jtag_gpio_off(void)
{
		mxc_request_iomux(MX25_PIN_D10, MUX_CONFIG_ALT5);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D10), NULL);
		gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D10));

		mxc_request_iomux(MX25_PIN_D11, MUX_CONFIG_ALT5);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D11), NULL);
		gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D11));
		
		mxc_request_iomux(MX25_PIN_D12, MUX_CONFIG_ALT5);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D12), NULL);
		gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D12));
		
		mxc_request_iomux(MX25_PIN_D13, MUX_CONFIG_ALT5);
		gpio_request(IOMUX_TO_GPIO(MX25_PIN_D13), NULL);
		gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D13));
}

static void set_jtag_gpio(unsigned char ucValue)	
{

		if(ucValue & TMS)
				gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D13),1);
		else
				gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D13),0);

		if(ucValue & TDI)
				gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D12), 1);
		else
				gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D12), 0);

		if(ucValue & TCK)
				gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D11), 1);
		else
				gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D11), 0);

}

static int imx_jtagio_open(struct inode *inode, struct file *filp)
{
		struct jtagio_struct *dev; /* device information */       

		config_jtag_gpio();
		jtag_info("jtagio open.\n");
		dev = container_of(inode->i_cdev, struct jtagio_struct, cdev);
		filp->private_data = dev;

		return 0;
}

static int imx_jtagio_release(struct inode *inode, struct file *filp)
{
		config_jtag_gpio_off();
		filp->private_data = NULL;
		return 0;
}


static  ssize_t imx_jtagio_read(struct file *filep, char __user *buf, size_t count, loff_t *offset)
{
		char  ucValue;	
		unsigned int   uiCount;
		ucValue = (unsigned char )(gpio_get_value(IOMUX_TO_GPIO(MX25_PIN_D10)));  //get TDO	
		uiCount = copy_to_user((char *)buf,&ucValue,1);
		*buf = ucValue;
		return(1);	  
}


static ssize_t imx_jtagio_write(struct file *filp, const char *buf, size_t count, loff_t *ppos)
{		
		unsigned char ucValue;

		ucValue = *buf;
		set_jtag_gpio(ucValue);	
		return 0;
}

struct file_operations jtagio_Fops = {
		.owner = THIS_MODULE,
		.read =imx_jtagio_read,		
		.write = imx_jtagio_write,
		.open = imx_jtagio_open,
		.release =  imx_jtagio_release,	
};


/*!
 * Entry point for keycrpt module.
 *
 */
static int __init mx258_jtagio_init(void)
{       
		int ret;

		jtag_info("JTAG device enter  info.\n");
		config_jtag_gpio_off();
		jtagptr = kmalloc(sizeof(struct jtagio_struct), GFP_KERNEL);
		if (!jtagptr) {
				printk(KERN_ERR"jtagio device kmalloc error.\n");
				return -ENOMEM;
		}
		memset(jtagptr,0,sizeof(struct jtagio_struct));

		/* Register character device */
		ret = alloc_chrdev_region(&jtagptr->dev_id,imx_jtagio_minor, 1,"cpld_jtag");	
		if (ret < 0) {
				printk(KERN_ERR"can't get jtagio major %d\n",imx_jtagio_major);
				goto err3;
		}
		imx_jtagio_major = MAJOR(jtagptr->dev_id);
		cdev_init(&jtagptr->cdev, &jtagio_Fops);
		jtagptr->cdev.owner = THIS_MODULE;

		ret = cdev_add(&jtagptr->cdev, jtagptr->dev_id, 1);
		if (ret) {
				printk(KERN_ERR"can't add cpldjtag cdev.\n");
				goto err2;
		}	

		/* create class and device for udev information */
		jtagptr->cls= class_create(THIS_MODULE, "cpldjtag");
		if (IS_ERR(jtagptr->cls)) {
				printk(KERN_ERR"Err!failed to create cpldjtag class\n");
				ret = -ENOMEM;
				goto err1;
		}

		jtagptr->clsdev= device_create(jtagptr->cls, NULL, jtagptr->dev_id, NULL,
						KEYCRYPT_DEVICE_FILE_NAME); //MKDEV(imx_keycrypt_major, imx_jtagio_minor)
		if (IS_ERR(jtagptr->clsdev)) {
				printk(KERN_ERR"Err!failed to create gps cpldjtag class device\n");
				ret = -ENOMEM;
				goto err0;
		}
		return ret;
err0:
		class_destroy(jtagptr->cls);
err1:
		cdev_del(&jtagptr->cdev);
err2:
		unregister_chrdev_region(jtagptr->dev_id, 1);
err3:	
		return ret;

}


/*!
 * unloading module.
 *
 */
static void __exit mx258_jtagio_exit(void)
{
		device_destroy(jtagptr->cls, jtagptr->dev_id); 
		class_destroy(jtagptr->cls);
		/* Unregister the device */
		cdev_del(&jtagptr->cdev);
		unregister_chrdev_region(jtagptr->dev_id, 1);
		kfree(jtagptr);
}

module_init(mx258_jtagio_init);
module_exit(mx258_jtagio_exit);
MODULE_DESCRIPTION("MX258 GPIO as JTAG for CPLD program loading");
MODULE_AUTHOR("SAND<lijinmei>");
MODULE_LICENSE("GPL");
