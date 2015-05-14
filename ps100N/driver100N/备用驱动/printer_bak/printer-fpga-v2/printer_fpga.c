/** @defgroup printer_fpga.c
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 2.0
 *  @date    2012-6-18
 @verbatim
 此文件为FPGA打印机接口源码。
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

#include <linux/platform_device.h>

#include <mach/gpio.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <mach-mx25/board-mx25_3stack.h>
#include <mach-mx25/iomux.h>
#include <mach-mx25/mx25_pins.h>
#include <linux/imx_adc.h>

#define PRINTER_NAME "printer"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define __D_ON
#ifdef __D_ON
#define DEBUGFMT "printerfpga(%d)-%s: "
#define DEBUGARGS __LINE__, __FUNCTION__
#define __D(fmt, args...) {printk(KERN_ALERT DEBUGFMT fmt, DEBUGARGS, ##args);}
#else
#define __D(fmt, args...) {}
#endif

#define PRINTER_CTRL_PHYADDR		0xA0000010      ///打印控制寄存器
#define PRINTER_FEEDLINE_PHYADDR	0xA0000011      ///空走纸行数寄存器
#define PRINTER_STAT_PHYADDR		0xA0000012	///打印状态寄存器
#define PRINTER_HEATTIME_PHYADDR	0xA0000013      ///加热时间寄存器
#define PRINTER_POWER_PHYADDR		0xA0000014	///电源控制
#define PRINTER_BUFFERLINE_A_PHYADDR	0xA0000015	///BUFFER A 打印行数
#define PRINTER_BUFFERLINE_B_PHYADDR	0xA0000016	///BUFFER B 打印行数
#define PRINTER_BUFFER_A_PHYADDR	0xA0000130	///BUFFER A
#define PRINTER_BUFFER_A_SIZE		(50*48)	///BUFFER A 大小
#define PRINTER_BUFFER_B_PHYADDR	0xA0001030	///BUFFER B
#define PRINTER_BUFFER_B_SIZE		(50*48)	///BUFFER B 大小

#define PRINTER_IOC_MAGIC		'S'
#define PRINTER_FEED_PAPER		_IOW(PRINTER_IOC_MAGIC,  1, int)
#define PRINTER_PRINT_VCC		_IOW(PRINTER_IOC_MAGIC,  2, int)
#define PRINTER_PRINT_FONT		_IOW(PRINTER_IOC_MAGIC,  3, int)
#define PRINTER_PRINT_TEST		_IOW(PRINTER_IOC_MAGIC,  4, int)
#define PRINTER_PRINT_CHECKPAPER	_IOWR(PRINTER_IOC_MAGIC, 5, int)
#define PRINTER_PRINT_CFGPARAM		_IOWR(PRINTER_IOC_MAGIC, 6, int)
#define PRINTER_PRINT_CLEANBUF		_IOWR(PRINTER_IOC_MAGIC, 7, int)
#define PRINTER_PRINT_CHKSTATUS		_IOWR(PRINTER_IOC_MAGIC, 8, int)
#define PRINTER_PRINT_SETVF		_IOWR(PRINTER_IOC_MAGIC, 9, int)
#define PRINTER_PRINT_END		_IOWR(PRINTER_IOC_MAGIC,10, int)
#define PRINTER_IOC_MAXNR		10

/** PRINTER打印机的数据结构 */
struct printer_dev_t
{
	struct cdev cdev;						///字符设备数据结构。
	struct class *class;						///设备类结构。
	dev_t devno;							///设备号。

	unsigned int irq_ctrlchanged;					///ctrl寄存器改变中断。

	volatile unsigned char *ctrl_vaddr;			///打印控制寄存器
	volatile unsigned char *feedline_vaddr;			///空走纸行数寄存器
	volatile unsigned char *stat_vaddr;			///打印状态寄存器
	volatile unsigned char *heattime_vaddr;			///加热时间寄存器虚拟地址。
	volatile unsigned char *power_vaddr;			///电源控制
	volatile unsigned char *bufferline_a_vaddr;
	volatile unsigned char *bufferline_b_vaddr;
	unsigned char *buffer_a;
	unsigned char *buffer_b;

	spinlock_t 	kfifo_lock;					///
	unsigned int 	kfifo_order;					///
	struct kfifo *	kfifo;						///
	unsigned char *	kfifo_buffer;					///
	unsigned int	kfifo_maxlen;					///PAGESIZE<<order

	struct workqueue_struct *print_workqueue;
	struct work_struct 	print_workstruct;
	unsigned char is_printing;
}*printer_dev_t_p;

static void printer_goprint(void)
{
	*(printer_dev_t_p->ctrl_vaddr)=((*(printer_dev_t_p->ctrl_vaddr)&(~(0x07)))|0x01);
}

static void printer_set_feedline(unsigned char line)
{
	*(printer_dev_t_p->feedline_vaddr)=line;
}

static void printer_gofeed(void)
{
	*(printer_dev_t_p->ctrl_vaddr)|=0x03;
}

static void printer_set_bufferline_a(unsigned char line)
{
//	pr_alert("line a:%d\n",line);
	*(printer_dev_t_p->bufferline_a_vaddr)=line;
}

static void printer_set_bufferline_b(unsigned char line)
{
//	pr_alert("line b:%d\n",line);
	*(printer_dev_t_p->bufferline_b_vaddr)=line;
}

static void printer_start(void)
{
	*(printer_dev_t_p->ctrl_vaddr)=(0x00);
}

static void printer_reset(void)
{
	*(printer_dev_t_p->ctrl_vaddr)&=(~(0x07));
}

static void printer_set_heattime(unsigned char time)
{
	*(printer_dev_t_p->heattime_vaddr)=time;
}

static void printer_set_power(int onoff)
{
	if(onoff)
	{
		*(printer_dev_t_p->power_vaddr)&=~(0x01<<7);
	}
	else
	{
		*(printer_dev_t_p->power_vaddr)|=(0x01<<7);
	}
} 

static void pr_allreg(void)
{
	pr_alert("ctrl:%02x\n",*(printer_dev_t_p->ctrl_vaddr));
	pr_alert("feed:%02x\n",*(printer_dev_t_p->feedline_vaddr));
	pr_alert("stat:%02x\n",*(printer_dev_t_p->stat_vaddr));
	pr_alert("heat:%02x\n",*(printer_dev_t_p->heattime_vaddr));
	pr_alert("pwr :%02x\n",*(printer_dev_t_p->power_vaddr));
	pr_alert("a l :%02x\n",*(printer_dev_t_p->bufferline_a_vaddr));
	pr_alert("b l :%02x\n",*(printer_dev_t_p->bufferline_b_vaddr));
}

static bool printer_is_feedend(void)
{
	bool flag;
	flag = ((*(printer_dev_t_p->stat_vaddr)&(0x80))==(0x80)?true:false);
	return flag;
}

static bool printer_is_nopaper(void)
{
	bool flag;
	(*(printer_dev_t_p->power_vaddr)|=(1<<6));
	udelay(50);
	flag = ((*(printer_dev_t_p->stat_vaddr)&(0x40))==(0x40)?true:false);
	(*(printer_dev_t_p->power_vaddr)|=(1<<6));
	return flag;
}

static bool printer_is_overheat(void)
{
	bool flag;
	flag = ((*(printer_dev_t_p->stat_vaddr)&(0x20))==(0x20)?true:false);
	return flag;
}

static unsigned char printer_buffer_stat(void)
{
	return (*(printer_dev_t_p->stat_vaddr)&(0x18))>>3;
}

static int read_temperature(void)
{
	return 300;
}

static inline void printer_autoset_heattime(void)
{
	int temperature;
	unsigned char heattime;

	temperature = read_temperature();
	if(temperature>310)
	{
		heattime=222;
	}
	else if(temperature<260)
	{
		heattime=172;
	}
	else
	{
		heattime=temperature-260+172;
	}

	printer_set_heattime(heattime);
}

static void reset_a(void)
{
	*(printer_dev_t_p->ctrl_vaddr)|=(0x40);
	*(printer_dev_t_p->ctrl_vaddr)&=(~(0x40));
}

static void reset_b(void)
{
	*(printer_dev_t_p->ctrl_vaddr)|=(0x80);
	*(printer_dev_t_p->ctrl_vaddr)&=(~(0x80));
}

static void first_write(void)
{
	int ret_a,ret_b;
        ret_a=0;
        ret_b=0;
	ret_a=kfifo_get(printer_dev_t_p->kfifo,printer_dev_t_p->buffer_a,PRINTER_BUFFER_A_SIZE);
        printer_set_bufferline_a(ret_a/48);
	ret_b=kfifo_get(printer_dev_t_p->kfifo,printer_dev_t_p->buffer_b,PRINTER_BUFFER_B_SIZE);
       	printer_set_bufferline_b(ret_b/48);

	printer_autoset_heattime();
        if(printer_dev_t_p->is_printing==FALSE)
        {
                printer_goprint();
                printer_dev_t_p->is_printing=TRUE;
        }
	
}

static void print_workqueue_func(void* d)
{
	int ret_a,ret_b,stat;
	ret_a=0;
	ret_b=0;
	stat = printer_buffer_stat();
//	pr_allreg();
	switch(stat)
	{
		case 0://all buffers full.
		return;
		case 1://buffer B end.
		ret_b=kfifo_get(printer_dev_t_p->kfifo,printer_dev_t_p->buffer_b,PRINTER_BUFFER_B_SIZE);
        	printer_set_bufferline_b(ret_b/48);
		reset_b();
		break;
		case 2://buffer A end.
		ret_a=kfifo_get(printer_dev_t_p->kfifo,printer_dev_t_p->buffer_a,PRINTER_BUFFER_A_SIZE);
        	printer_set_bufferline_a(ret_a/48);
		reset_a();
		break;
		case 3://all buffers empty.
			printer_reset();
		ret_b=kfifo_get(printer_dev_t_p->kfifo,printer_dev_t_p->buffer_b,PRINTER_BUFFER_B_SIZE);
        	printer_set_bufferline_b(ret_b/48);
		reset_b();
		ret_a=kfifo_get(printer_dev_t_p->kfifo,printer_dev_t_p->buffer_a,PRINTER_BUFFER_A_SIZE);
        	printer_set_bufferline_a(ret_a/48);
		reset_a();
		printer_goprint();
		break;
		default://not into this forever.
		break;
	}
	printer_autoset_heattime();
	if(printer_dev_t_p->is_printing==FALSE)
	{
		printer_goprint();
		printer_dev_t_p->is_printing=TRUE;
	}
	else
	{
		printer_goprint();
	}
}

static irqreturn_t  printer_ctrlchanged_interrupt(int irq, void *dev_id)
{
	if(printer_dev_t_p->is_printing==TRUE && (kfifo_len(printer_dev_t_p->kfifo)>0))
	{
		//queue_work(printer_dev_t_p->print_workqueue,&printer_dev_t_p->print_workstruct);
		print_workqueue_func(0);
	}
	else
	{
		printer_reset();
		printer_dev_t_p->is_printing=FALSE;
		printer_set_power(FALSE);
	}
	return IRQ_HANDLED;
}

static int printer_all_irq_request(void)
{
	int err;
	mxc_free_iomux(MX25_PIN_CSI_D7, MUX_CONFIG_GPIO);
	mxc_request_iomux(MX25_PIN_CSI_D7, MUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX25_PIN_CSI_D7, PAD_CTL_PKE_NONE);
	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_CSI_D7));
	set_irq_type(IOMUX_TO_IRQ(MX25_PIN_CSI_D7), IRQF_TRIGGER_HIGH);
	err = request_irq(IOMUX_TO_IRQ(MX25_PIN_CSI_D7), printer_ctrlchanged_interrupt, IRQF_TRIGGER_HIGH, "ctrl changed",printer_dev_t_p);
	enable_irq_wake(IOMUX_TO_IRQ(MX25_PIN_CSI_D7));
	if (err) {
		printk("printer driver: %s request_irq failed, err=%d\n", __func__, err);
		return 0;
	}
	return 0;
}

static void printer_all_irq_free(void)
{
	free_irq(IOMUX_TO_IRQ(MX25_PIN_CSI_D7), printer_dev_t_p);
}

static ssize_t printer_write (struct file *file ,const char *buf, size_t count, loff_t *f_ops)
{
	int ret=0;
	if(count%48 != 0)
	{
		__D("printer printer only accept data size integer multiple of 48.\n");
		ret=0;
	}
	else if (printer_dev_t_p->kfifo_maxlen-kfifo_len(printer_dev_t_p->kfifo)<count)
	{
		__D("printer kfifo is no space.\n");
		ret=-EBUSY;
	}
	else
	{
		ret = kfifo_put(printer_dev_t_p->kfifo,(unsigned char *)buf, count);
		if(printer_dev_t_p->is_printing==FALSE)
		{
			printer_set_power(TRUE);
			printer_reset();
			first_write();
		}

	}
	return ret;
}

static unsigned int printer_poll (struct file *file, struct poll_table_struct *poll_t_p)
{
	__D("not support poll yet.\n");
	return 0;
}

static int printer_ioctl (struct inode *inode ,struct file *file, unsigned int cmd, unsigned long data)
{
	int ret = 0;

	if (_IOC_TYPE(cmd) != PRINTER_IOC_MAGIC)
	{
		__D("IOC_MAGIC not match.\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > PRINTER_IOC_MAXNR)
	{
		__D("cmd is large than IOC_MAXNR.\n");
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void __user *)data, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret =  !access_ok(VERIFY_READ, (void __user *)data, _IOC_SIZE(cmd));
	if (ret)
	{
		__D("READ WRITE check fault.\n");
		return -EFAULT;
	}

	switch(cmd)
	{
		case PRINTER_PRINT_VCC:
			break;
		case PRINTER_PRINT_FONT:
			break;
		case PRINTER_PRINT_CFGPARAM:
			break;
		case PRINTER_PRINT_CLEANBUF:
			break;
		case PRINTER_PRINT_CHKSTATUS:
			break;
		case PRINTER_PRINT_SETVF:
			break;
		case PRINTER_PRINT_END:
			break;
		case PRINTER_PRINT_CHECKPAPER:
			break;
		case PRINTER_FEED_PAPER:
			break;
		case PRINTER_PRINT_TEST:
			break;
		default:
			pr_alert("printerfpga doesn't support this cmd yet.\n");
			ret=-ENOTTY;
			break;
	}
	return ret;
}

ssize_t printer_open (struct inode *inode ,struct file *file)
{
	return 0;
}

ssize_t printer_release (struct inode  *inode ,struct file *file)
{
	return 0;
}

static struct file_operations printer_ops =
{
	.poll = printer_poll,
	.write = printer_write,
	.ioctl = printer_ioctl,
	.open = printer_open,
	.release = printer_release,
};

static int __init printer_setup_cdev(struct cdev *dev,int index)
{
	int ret=0;
	cdev_init(dev,&printer_ops);
	dev->owner=THIS_MODULE;
	dev->ops=&printer_ops;
	ret=cdev_add(dev,printer_dev_t_p->devno,1);
	if (ret)
	{
		__D("Error %d adding GZ%d\n",ret,index);
		goto seterr_a;
	}
	printer_dev_t_p->class = class_create(THIS_MODULE, "printer_class");
	if (IS_ERR(printer_dev_t_p->class))
	{
		ret = -EINVAL;
		__D("printer: failed in creating class.\n");
		goto seterr_b;
	}
	if(NULL==device_create(printer_dev_t_p->class, NULL, printer_dev_t_p->devno, NULL, "printer"))
	{
		ret = -EINVAL;
		__D("printer: failed in creating class device.\n");
		goto seterr_c;
	}
	return ret;
seterr_c:
	class_destroy(printer_dev_t_p->class);
seterr_b:
	cdev_del(&printer_dev_t_p->cdev);
seterr_a:
	return ret;
}

static int __init printer_init_register(void)
{
	int ret = -ENODEV;
	/** 为printer设备结构分配空间 */
	printer_dev_t_p=kzalloc(sizeof(struct printer_dev_t),GFP_KERNEL);
	if(printer_dev_t_p == NULL)
	{
		ret = -ENOMEM;
		goto regerr_a;
	}

	/** 将寄存器的物理地址重新映射 */
	printer_dev_t_p->ctrl_vaddr	=	ioremap_nocache(PRINTER_CTRL_PHYADDR, 1);
	printer_dev_t_p->feedline_vaddr	=	ioremap_nocache(PRINTER_FEEDLINE_PHYADDR, 1);
	printer_dev_t_p->stat_vaddr	=	ioremap_nocache(PRINTER_STAT_PHYADDR, 1);
	printer_dev_t_p->heattime_vaddr	=	ioremap_nocache(PRINTER_HEATTIME_PHYADDR, 1);
	printer_dev_t_p->power_vaddr	=	ioremap_nocache(PRINTER_POWER_PHYADDR, 1);
	printer_dev_t_p->bufferline_a_vaddr =	ioremap_nocache(PRINTER_BUFFERLINE_A_PHYADDR, 1);
	printer_dev_t_p->bufferline_b_vaddr =	ioremap_nocache(PRINTER_BUFFERLINE_B_PHYADDR, 1);
	printer_dev_t_p->buffer_a	=	ioremap_nocache(PRINTER_BUFFER_A_PHYADDR, PRINTER_BUFFER_A_SIZE);
	printer_dev_t_p->buffer_b	=	ioremap_nocache(PRINTER_BUFFER_B_PHYADDR, PRINTER_BUFFER_B_SIZE);

	/** 重置PRINTER并初始化加热时间寄存器 */
	printer_start();
	printer_set_heattime(192);

	/** 初始化写入缓冲 */
	spin_lock_init(&printer_dev_t_p->kfifo_lock);
	printer_dev_t_p->kfifo_order = 6;
	printer_dev_t_p->kfifo_buffer = (unsigned char *) __get_free_pages(GFP_KERNEL, printer_dev_t_p->kfifo_order);
	if (!printer_dev_t_p->kfifo_buffer)
	{
		__D("kfifo_buffer malloc error \n");
		ret =  -ENOMEM ;
		goto  regerr_b;
	}
	printer_dev_t_p->kfifo_maxlen = PAGE_SIZE<<printer_dev_t_p->kfifo_order;
	memset(printer_dev_t_p->kfifo_buffer,0,printer_dev_t_p->kfifo_maxlen);

	printer_dev_t_p->kfifo = kfifo_init(
			printer_dev_t_p->kfifo_buffer,
			printer_dev_t_p->kfifo_maxlen,
			GFP_KERNEL,
			&(printer_dev_t_p->kfifo_lock));

	/** 注册中断 */
	ret = printer_all_irq_request();
	if (ret) {
		__D("request irq_ctrlchanged error\n");
		ret = -EINVAL;
		goto regerr_c;
	}

	/** 由系统分配设备号 */
	ret=alloc_chrdev_region(&(printer_dev_t_p->devno),0,1,"printer");
	if (ret<0)
	{
		__D("printerfpga init_module failed with %d.\n",ret);
		goto regerr_d;
	}
	else
	{
		__D("printerfpga init_module success, major=%d, minor=%d.\n",
				MAJOR(printer_dev_t_p->devno), MINOR(printer_dev_t_p->devno));
	}

	/* 注册设备并建立设备节点 */
	ret=printer_setup_cdev(&(printer_dev_t_p->cdev),0);
	if(ret)
	{
		ret = -ENODEV;
		goto regerr_e;
	};

	/* 注册workqueue */
	printer_dev_t_p->print_workqueue = create_singlethread_workqueue("print_queue");
	INIT_WORK(&printer_dev_t_p->print_workstruct,print_workqueue_func);

	printer_dev_t_p->is_printing=FALSE;
	return 0;
regerr_e:
	unregister_chrdev_region(printer_dev_t_p->devno, 1);
regerr_d:
	printer_all_irq_free();
regerr_c:
	free_pages((unsigned long)printer_dev_t_p->kfifo_buffer,printer_dev_t_p->kfifo_order);
regerr_b:
	iounmap(printer_dev_t_p->ctrl_vaddr);
	iounmap(printer_dev_t_p->feedline_vaddr);
	iounmap(printer_dev_t_p->stat_vaddr);
	iounmap(printer_dev_t_p->heattime_vaddr);
	iounmap(printer_dev_t_p->power_vaddr);
	iounmap(printer_dev_t_p->bufferline_a_vaddr);
	iounmap(printer_dev_t_p->bufferline_b_vaddr);
	iounmap(printer_dev_t_p->buffer_a);
	iounmap(printer_dev_t_p->buffer_b);
	kfree(printer_dev_t_p);
regerr_a:
	return ret;
}

static int __init printer_probe(void)
{
	int  ret;
	ret = printer_init_register();
	return ret;
}

static void __exit printer_remove(void)
{
	device_destroy(printer_dev_t_p->class,printer_dev_t_p->devno);
	class_destroy(printer_dev_t_p->class);
	cdev_del(&printer_dev_t_p->cdev);
	iounmap(printer_dev_t_p->ctrl_vaddr);
	iounmap(printer_dev_t_p->feedline_vaddr);
	iounmap(printer_dev_t_p->stat_vaddr);
	iounmap(printer_dev_t_p->heattime_vaddr);
	iounmap(printer_dev_t_p->power_vaddr);
	iounmap(printer_dev_t_p->bufferline_a_vaddr);
	iounmap(printer_dev_t_p->bufferline_b_vaddr);
	iounmap(printer_dev_t_p->buffer_a);
	iounmap(printer_dev_t_p->buffer_b);
	unregister_chrdev_region(printer_dev_t_p->devno, 1);
	printer_all_irq_free();
	kfree(printer_dev_t_p);
}

static int  printer_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

static int  printer_resume(struct platform_device *dev, pm_message_t state)
{
	return 0;
}


static struct platform_device printer_device = {
	.name = PRINTER_NAME,
	.id   = -1,
};

static struct platform_driver  printer_driver = {
	.probe     = printer_probe,
	.remove    = printer_remove,
	.suspend   = printer_suspend,
	.resume    = printer_resume,
	.driver    = {
		.name   = PRINTER_NAME,
		.owner  = THIS_MODULE,
	},
};

static int __init printer_init(void)
{
	int ret;

	ret = platform_device_register(&printer_device);

	if(ret != 0)
	{
		printk("%s:platform_device_register result: %d\n", __func__, ret);
		return ret;
	}

	ret = platform_driver_register(&printer_driver);

	if(ret != 0)
	{
		printk("%s:platform_driver_register result: %d\n",  __func__, ret);
		platform_device_unregister(&printer_device);
	}

	return ret;
}

static void __exit printer_exit(void)
{
	platform_driver_unregister(&printer_driver);
	platform_device_unregister(&printer_device);
}

module_init(printer_init);
module_exit(printer_exit);

MODULE_AUTHOR("zengshu@gmail.com");
MODULE_LICENSE("Dual BSD/GPL");
/** @}*/ //注释文件结束
