/** @defgroup ss205fpga.c
 *  @author  曾舒 (Email:zengshu@gmail.com)
 *  @version 1.2
 *  @date    2012-6-14
 @verbatim

 copyright (C) 2010上海杉德金卡信息科技有限公司

 此文件为ss205热敏打印机FPGA接口源码。
FIXME:现在仅支持一个热敏打印机。

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

/*************************
  Now we support:
#define S3C24XX_PLATFORM
#define MX25X_PLATFORM
#define MX25X_PLATFORM_V2
 ************************/
#define MX25X_PLATFORM_V2

#define SS205_NAME "printer"

#if defined(S3C24XX_PLATFORM)
#include <asm/semaphore.h>
#include <asm/arch-s3c2410/gpio.h>
#include <asm/arch-s3c2410/regs-gpio.h>
#include <asm/arch-s3c2410/regs-fpga.h>
#elif defined(MX25X_PLATFORM)||defined(MX25X_PLATFORM_V2)
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <linux/device.h> 
#include <mach-mx25/board-mx25_3stack.h>
#include <mach-mx25/iomux.h>
#include <mach-mx25/mx25_pins.h>
#include <linux/imx_adc.h>
#endif

int in=0;
int out=0;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define __D_ON

#ifdef __D_ON
#define DEBUGFMT "ss205fpga(%d)-%s: "
#define DEBUGARGS __LINE__, __FUNCTION__
#define __D(fmt, args...) {printk(KERN_ALERT DEBUGFMT fmt, DEBUGARGS, ##args);}
#else
#define __D(fmt, args...) {}
#endif

#if defined(S3C24XX_PLATFORM)
#define ADC_CH	1
#define SS205_CTRL_PHYADDR	0x18000010	///打印状态及控制寄存器的物理地址。
#define SS205_FEEDLINE_PHYADDR	0x18000011	///空走纸行数寄存器的物理地址。
#define SS205_PRINTLINE_PHYADDR	0x18000012	///打印行数寄存器的物理地址。
#define SS205_HEATTIME_PHYADDR	0x18000013	///加热时间寄存器的物理地址。
#define SS205_CHARGEON_PHYADDR	0x18000061	///是否在充电的寄存器。
#define SS205_PRINTDATA_PHYADDR 0x18010000	///打印内容的实际存储区物理地址。
#define SS205_PRINTDATA_SIZE	0x2FD0		///打印内容的存储区大小，由FPGA提供，单位byte。
#elif defined(MX25X_PLATFORM)
#define ADC_CH	0
#define SS205_CTRL_PHYADDR      0xB2000010      ///打印状态及控制寄存器的物理地址。
#define SS205_FEEDLINE_PHYADDR  0xB2000011      ///空走纸行数寄存器的物理地址。
#define SS205_PRINTLINE_PHYADDR 0xB2000012      ///打印行数寄存器的物理地址。
#define SS205_HEATTIME_PHYADDR  0xB2000013      ///加热时间寄存器的物理地址。
#define SS205_BUFFSTAT_PHYADDR	0xB2000014	///打印缓存区状态寄存器。
#define SS205_CHARGEON_PHYADDR  0xB2000061      ///是否在充电的寄存器。
#define SS205_PRINTDATA_PHYADDR 0xB2000130      ///打印内容的实际存储区物理地址。
//#define SS205_PRINTDATA_SIZE	0x17E8		///打印内容的存储区大小，由FPGA提供，单位byte。
#define SS205_PRINTDATA_SIZE	(48*0xA0)		///打印内容的存储区大小，由FPGA提供，单位byte。
#elif defined(MX25X_PLATFORM_V2)
#define ADC_CH	0
#define SS205_CTRL_PHYADDR      0xA0000010      ///打印状态及控制寄存器的物理地址。
#define SS205_FEEDLINE_PHYADDR  0xA0000011      ///空走纸行数寄存器的物理地址。
#define SS205_PRINTLINE_PHYADDR 0xA0000012      ///打印行数寄存器的物理地址。
#define SS205_HEATTIME_PHYADDR  0xA0000013      ///加热时间寄存器的物理地址。
#define SS205_BUFFSTAT_PHYADDR	0xA0000014	///打印缓存区状态寄存器。
#define SS205_CHARGEON_PHYADDR  0xA0000061      ///是否在充电的寄存器。
#define SS205_PRINTDATA_PHYADDR 0xA0000130      ///打印内容的实际存储区物理地址。
#define SS205_PRINTDATA_SIZE	0x1ED0		///打印内容的存储区大小，由FPGA提供，单位byte。

#endif

#define SS205_IOC_MAGIC			'S'
#define SS205_FEED_PAPER		_IOW(SS205_IOC_MAGIC,  1, int)
#define SS205_PRINT_VCC			_IOW(SS205_IOC_MAGIC,  2, int)
#define SS205_PRINT_FONT		_IOW(SS205_IOC_MAGIC,  3, int)
#define SS205_PRINT_TEST		_IOW(SS205_IOC_MAGIC,  4, int)
#define SS205_PRINT_CHECKPAPER	_IOWR(SS205_IOC_MAGIC, 5, int)
#define SS205_PRINT_CFGPARAM	_IOWR(SS205_IOC_MAGIC, 6, int)
#define SS205_PRINT_CLEANBUF	_IOWR(SS205_IOC_MAGIC, 7, int)
#define SS205_PRINT_CHKSTATUS	_IOWR(SS205_IOC_MAGIC, 8, int)
#define SS205_PRINT_SETVF		_IOWR(SS205_IOC_MAGIC, 9, int)
#define SS205_PRINT_END			_IOWR(SS205_IOC_MAGIC,10, int)
#define SS205_IOC_MAXNR			10

/** SS205打印机的数据结构 */
struct ss205_dev_t
{
	struct cdev cdev;									///字符设备数据结构。
	struct class *class;								///设备类结构。
	dev_t devno;										///设备号。

	unsigned int irq_ctrlchanged;						///ctrl寄存器改变中断。

	volatile unsigned char *	ctrl_vaddr;				///打印状态及控制寄存器的虚拟地址。
	volatile unsigned char *	feedline_vaddr;			///空走纸行数寄存器的虚拟地址。
	volatile unsigned char *	printline_vaddr;		///打印行数寄存器的虚拟地址。
	volatile unsigned char *	heattime_vaddr;			///加热时间寄存器虚拟地址。
	volatile unsigned char *	buffstat_vaddr;			///打印缓存区状态寄存器虚拟地址。
	volatile unsigned char *	chargeon_vaddr;			///是否在充电的寄存器。
	unsigned char *	printdata_vaddr;					///打印内容的实际存储区虚拟地址。

	spinlock_t 		kfifo_lock;							///
	unsigned int 	kfifo_order;						///
	struct kfifo *	kfifo;								///
	unsigned char *	kfifo_buffer;						///
	unsigned int	kfifo_maxlen;						///PAGESIZE<<order

	struct workqueue_struct *print_workqueue;
	struct work_struct 	print_workstruct;
	unsigned char is_printing;
}*ss205_dev_t_p;

/** 注意：以下宏请谨慎使用 */
//#define SS205_INIT()				(*(ss205_dev_t_p->ctrl_vaddr)=(0x98))
#define SS205_INIT()				(*(ss205_dev_t_p->ctrl_vaddr)=(0x90))
#define SS205_RESET()				(*(ss205_dev_t_p->ctrl_vaddr)&=(~(0x07)))
#define SS205_CONTINUE_PRINT()			(*(ss205_dev_t_p->ctrl_vaddr)=((*(ss205_dev_t_p->ctrl_vaddr)&(~(0x07)))|0x05))
#define SS205_IS_NORMAL_PRINT			((*(ss205_dev_t_p->ctrl_vaddr)&(0x03))==(1)?1:0)
#define SS205_IS_PREFEED_PRINT			((*(ss205_dev_t_p->ctrl_vaddr)&(0x03))==(2)?1:0)
#define SS205_IS_FEED				((*(ss205_dev_t_p->ctrl_vaddr)&(0x03))==(3)?1:0)
#define SS205_IS_WORK_FINISHED			((*(ss205_dev_t_p->ctrl_vaddr)&(0x04))==(0x04)?1:0)
#define SS205_IS_PRINT_FINISHED			((*(ss205_dev_t_p->ctrl_vaddr)&(0x10))==(0x10)?1:0)
#define SS205_IS_OVERHEAT			((*(ss205_dev_t_p->ctrl_vaddr)&(0x20))==(0x20)?1:0)
#define SS205_IS_NOPAPER			((*(ss205_dev_t_p->ctrl_vaddr)&(0x40))==(0x40)?1:0)
#define SS205_IS_FEED_FINISHED			((*(ss205_dev_t_p->ctrl_vaddr)&(0x80))==(0x80)?1:0)
#define SS205_IS_BUFFEMPTY			((*(ss205_dev_t_p->buffstat_vaddr)&(0x01))==(0x01)?0:1)

//#define SS205_SET_BUFFREFULL()			(*(ss205_dev_t_p->buffstat_vaddr)|=0x01)
#define SS205_SET_PRINTLINE(x)			(*(ss205_dev_t_p->printline_vaddr)=(x))

#define SS205_SET_PRINTER_POWERON()		(*(ss205_dev_t_p->buffstat_vaddr)&=~(0x01<<7))
#define SS205_SET_PRINTER_POWEROFF()		(*(ss205_dev_t_p->buffstat_vaddr)|=(0x01<<7))

#define SS205_GOPRINT()				(*(ss205_dev_t_p->ctrl_vaddr)=((*(ss205_dev_t_p->ctrl_vaddr)&(~(0x07)))|0x01))
#define SS205_GOPRINT_PREFEED()    		(*(ss205_dev_t_p->ctrl_vaddr)=((*(ss205_dev_t_p->ctrl_vaddr)&(~(0x07)))|0x02))

#define SS205_SET_FEEDLINE(x)			(*(ss205_dev_t_p->feedline_vaddr)=(x))
#define SS205_GOFEED()				(*(ss205_dev_t_p->ctrl_vaddr)|=0x03)

#define SS205_SET_HEATTIME(x)			(*(ss205_dev_t_p->heattime_vaddr)=(x))

#ifdef S3C24XX_PLATFORM
extern ssize_t my_adc_read(u32 adc_ch);
#elif defined(MX25X_PLATFORM) || defined(MX25X_PLATFORM_V2)
static ssize_t my_adc_read(u32 adc_ch)
{
#if 0
	int ret=0;
	unsigned short result=0;
	switch(adc_ch)
	{
		case 0:
			ret=imx_adc_convert(GER_PURPOSE_ADC0,&result);
		break;
		case 1:
			ret=imx_adc_convert(GER_PURPOSE_ADC1,&result);
		break;
		case 2:
			ret=imx_adc_convert(GER_PURPOSE_ADC2,&result);
		break;
		default:
		break;	
	}
	if(ret<0)
	{
		__D("Error in my_adc_read!\n");
		return ret;
	}
	return result;
#endif
	return 300;
}
#endif
static inline void ss205_autoset_heattime(void)
{
	int temp,voltage;
	unsigned char heattime;

	temp = my_adc_read(0);
	if(temp>310)
	{
		heattime=222;
	}
	else if(temp<260)
	{
		heattime=172;
	}
	else
	{
		heattime=temp-260+172;
	}

	if((*ss205_dev_t_p->chargeon_vaddr)&1)
	{
		voltage=my_adc_read(1);
	}
	else
	{
		voltage=0;
	}

	//__D("temprature=%d,voltage=%d,nCHARGE=%x\n",temp,voltage,*ss205_dev_t_p->chargeon_vaddr);

	SS205_SET_HEATTIME(heattime);
}

static void print_workqueue_func(unsigned long data)
{
	int ret;
	ret=kfifo_get(ss205_dev_t_p->kfifo,ss205_dev_t_p->printdata_vaddr,SS205_PRINTDATA_SIZE);
	SS205_SET_PRINTLINE(ret/48);
	ss205_autoset_heattime();
	if(ss205_dev_t_p->is_printing==FALSE)
	{
		SS205_GOPRINT_PREFEED();
		ss205_dev_t_p->is_printing=TRUE;
	}
	else
	{
		SS205_GOPRINT();
	}
	out+=ret/48;
	//pr_alert("workqueue:nCTRL=0x%x\n",*ss205_dev_t_p->ctrl_vaddr);
	//pr_alert("%s:send %d line data\n",__func__,out);
}

static inline void ss205_enable_checkpaper(int x)
{
	/* BIT[3] 0:开 1:关 */
	if(x)
	{
		*(ss205_dev_t_p->ctrl_vaddr)&=~(0x01<<3);
	}
	else
	{
		*(ss205_dev_t_p->ctrl_vaddr)|=(0x01<<3);
	}
}
static irqreturn_t  ss205_ctrlchanged_interrupt(int irq, void *dev_id)
{
//	pr_alert("irq:nCTRL=0x%x\n",*ss205_dev_t_p->ctrl_vaddr);
#if 0
	if(SS205_IS_BUFFEMPTY)
	{
		SS205_SET_PRINTLINE(kfifo_get(ss205_dev_t_p->kfifo,ss205_dev_t_p->printdata_vaddr,SS205_PRINTDATA_SIZE)/48);
		ss205_autoset_heattime();
		if(SS205_IS_PRINT_FINISHED)
		{
			SS205_RESET();
			SS205_GOPRINT();
		}
		else
		{
			SS205_SET_BUFFREFULL();
		}
		return IRQ_HANDLED;
	}
	else
#endif
#if 0
	if(SS205_IS_BUFFEMPTY)
	{
		int ret;
		ret=kfifo_get(ss205_dev_t_p->kfifo,ss205_dev_t_p->printdata_vaddr,SS205_PRINTDATA_SIZE);
		SS205_SET_PRINTLINE(ret/48);
		SS205_SET_BUFFREFULL();
		ss205_dev_t_p->is_printing=TRUE;
		out+=ret/48;
		pr_alert("%s:send %d line data.\n",__func__,out);
		return IRQ_HANDLED;
	} 
	else 
#endif
	if(SS205_IS_PRINT_FINISHED)
	{
		if(ss205_dev_t_p->is_printing==TRUE && (kfifo_len(ss205_dev_t_p->kfifo)>0))
		{
			SS205_CONTINUE_PRINT();
			//queue_work(ss205_dev_t_p->print_workqueue,&ss205_dev_t_p->print_workstruct);
			print_workqueue_func(NULL);
		}
		else
		{
			SS205_RESET();
			ss205_dev_t_p->is_printing=FALSE;
			SS205_SET_PRINTER_POWEROFF();
		}
		//pr_alert("%d\n",__LINE__);
	}
	else if(SS205_IS_FEED_FINISHED)
	{
		SS205_RESET();
		ss205_dev_t_p->is_printing=FALSE;
		pr_alert("%d\n",__LINE__);
	}
	else if(SS205_IS_OVERHEAT)
	{
		SS205_RESET();
		ss205_dev_t_p->is_printing=FALSE;
		pr_alert("%d\n",__LINE__);
	}
	else if(SS205_IS_NOPAPER)
	{
		SS205_RESET();
		ss205_dev_t_p->is_printing=FALSE;
		pr_alert("%d:ctrl1 is 0x%02x,ctrl4 is 0x%02x \n",__LINE__, *(ss205_dev_t_p->ctrl_vaddr),*(ss205_dev_t_p->buffstat_vaddr));
	}
	
	return IRQ_HANDLED;
}

#if defined(S3C24XX_PLATFORM)
static int ss205_all_irq_request(void)
{
	s3c2410_gpio_cfgpin(S3C2410_GPF6,S3C2410_GPF6_EINT6);
	ss205_dev_t_p->irq_ctrlchanged = IRQ_EINT6;
	return request_irq(ss205_dev_t_p->irq_ctrlchanged, ss205_ctrlchanged_interrupt,
			IRQF_TRIGGER_RISING,"ctrl changed",ss205_dev_t_p);
}
static void ss205_all_irq_free(void)
{
	free_irq(ss205_dev_t_p->irq_ctrlchanged, ss205_dev_t_p);
}
#elif defined(MX25X_PLATFORM) || defined(MX25X_PLATFORM_V2)
static int ss205_all_irq_request(void)
{
	int err;
	mxc_free_iomux(MX25_PIN_CSI_D7, MUX_CONFIG_GPIO);
	mxc_request_iomux(MX25_PIN_CSI_D7, MUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX25_PIN_CSI_D7, PAD_CTL_PKE_NONE);
	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_CSI_D7));
	set_irq_type(IOMUX_TO_IRQ(MX25_PIN_CSI_D7), IRQF_TRIGGER_RISING);
        err = request_irq(IOMUX_TO_IRQ(MX25_PIN_CSI_D7), ss205_ctrlchanged_interrupt, IRQF_TRIGGER_RISING, "ctrl changed",ss205_dev_t_p);
	enable_irq_wake(IOMUX_TO_IRQ(MX25_PIN_CSI_D7));
	if (err) {
		printk("printer driver: %s request_irq failed, err=%d\n", __func__, err);
		return 0;
	}
	return 0;
}
static void ss205_all_irq_free(void)
{
	free_irq(IOMUX_TO_IRQ(MX25_PIN_CSI_D7), ss205_dev_t_p);
}
#endif

static ssize_t ss205_write (struct file *file ,const char *buf, size_t count, loff_t *f_ops)
{
	int ret=0;
	ss205_enable_checkpaper(TRUE);
	udelay(100);
	if(count%48 != 0)
	{
		__D("ss205 printer only accept data size integer multiple of 48.\n");
		ret=0;
	}
	else if(SS205_IS_NOPAPER || SS205_IS_OVERHEAT)
	{
		__D("ss205 printer no paper or overheat.\n");
		SS205_RESET();
		ss205_dev_t_p->is_printing=FALSE;
		ret = -(SS205_IS_OVERHEAT<<1|SS205_IS_NOPAPER);
	}
	else if (ss205_dev_t_p->kfifo_maxlen-kfifo_len(ss205_dev_t_p->kfifo)<count)
	{
		__D("ss205 kfifo is no space.\n");
		ret=-EBUSY;
	}
	else
	{
		ret = kfifo_put(ss205_dev_t_p->kfifo,(unsigned char *)buf, count);
		in+=count/48;
		if(ss205_dev_t_p->is_printing==FALSE)
		{
			SS205_SET_PRINTER_POWERON();
			//queue_work(ss205_dev_t_p->print_workqueue,&ss205_dev_t_p->print_workstruct);
			print_workqueue_func(NULL);
		}

	}
	ss205_enable_checkpaper(FALSE);
	return ret;
}

static unsigned int ss205_poll (struct file *file, struct poll_table_struct *poll_t_p)
{
	return 0;
}

static int ss205_ioctl (struct inode *inode ,struct file *file, unsigned int cmd, unsigned long data)
{
	int ret = 0;
	unsigned int status;

	if (_IOC_TYPE(cmd) != SS205_IOC_MAGIC)
	{
		__D("IOC_MAGIC not match.\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > SS205_IOC_MAXNR)
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
		case SS205_PRINT_VCC:
			__D("Not support SS205_PRINT_VCC yet.\n");
			ret=-ENOTTY;
			break;
		case SS205_PRINT_FONT:
			__D("Not support SS205_PRINT_FONT yet.\n");
			ret=-ENOTTY;
			break;
		case SS205_PRINT_CFGPARAM:
			__D("Not support SS205_PRINT_CFGPARAM yet.\n");
			ret=-ENOTTY;
			break;
		case SS205_PRINT_CLEANBUF:
			kfifo_reset(ss205_dev_t_p->kfifo);
			break;
		case SS205_PRINT_CHKSTATUS:
			{
				//ss205_enable_checkpaper(TRUE);
				udelay(100);
				status = (SS205_IS_OVERHEAT<<1|SS205_IS_NOPAPER);
				ret = copy_to_user((unsigned int *)data,
						(unsigned int *)&status, sizeof(unsigned int));
				//ss205_enable_checkpaper(FALSE);

			}
			break;
		case SS205_PRINT_SETVF:		///FIXME:无作用
			ret = 0;
			break;
		case SS205_PRINT_END:		///FIXME:
			{
				unsigned int status = 0;

				//xu.xb
				if(kfifo_len(ss205_dev_t_p->kfifo) == 0)
					status = 1;
				else    
					status = 0;
				ret = copy_to_user((unsigned int *)data,
						(unsigned int *)&status, sizeof(unsigned int));

			}
			break;
		case SS205_PRINT_CHECKPAPER:
			{
				//ss205_enable_checkpaper(TRUE);
				udelay(100);
				ret = SS205_IS_NOPAPER;
				//ss205_enable_checkpaper(FALSE);
			}
			break;
		case SS205_FEED_PAPER:
			{
				unsigned int line;
				ret = copy_from_user(&line,(unsigned int *)data,sizeof(unsigned int));
				int i=0;
				unsigned char buf[48] = {0};
				if(!ss205_dev_t_p->is_printing)
				{
					if(!ret)
					{
						SS205_RESET();
						SS205_SET_FEEDLINE((unsigned char)line*2);
						SS205_GOFEED();
					}
				}
				else
				{
					for(i=0;i<line*16;i++)
					{
						kfifo_put(ss205_dev_t_p->kfifo, buf, 48);
					}
				}
				
			}
			break;
		case SS205_PRINT_TEST:
			{
				unsigned int cnt=0;
				SS205_RESET();

#if 1
				for(cnt=0;cnt<SS205_PRINTDATA_SIZE;cnt++)
				{
					if((cnt/(48*8))%2==0)
					{
						if(cnt%2==0)
							*(ss205_dev_t_p->printdata_vaddr+cnt)=0xff;
						else
							*(ss205_dev_t_p->printdata_vaddr+cnt)=0x00;
					}
					else
					{
						if(cnt%2==0)
							*(ss205_dev_t_p->printdata_vaddr+cnt)=0x00;
						else
							*(ss205_dev_t_p->printdata_vaddr+cnt)=0xff;
					}
				}
#else
				for(cnt=0;cnt<SS205_PRINTDATA_SIZE;cnt++)
				{
						if(cnt%2==0)
							*(ss205_dev_t_p->printdata_vaddr+cnt)=0xff;
						else
							*(ss205_dev_t_p->printdata_vaddr+cnt)=0x00;
				}
#endif
				SS205_SET_PRINTLINE(SS205_PRINTDATA_SIZE/48);
				SS205_GOPRINT();
			}
			break;
		default:
			printk(KERN_ALERT "ss205fpga doesn't support this cmd yet.\n");
			ret=-ENOTTY;
			break;
	}
	return ret;
}

ssize_t ss205_open (struct inode *inode ,struct file *file)
{
	return 0;
}

ssize_t ss205_release (struct inode  *inode ,struct file *file)
{
	return 0;
}

static struct file_operations ss205_ops =
{
	.poll = ss205_poll,
	.write = ss205_write,
	.ioctl = ss205_ioctl,
	.open = ss205_open,
	.release = ss205_release,
};

static int __init ss205_setup_cdev(struct cdev *dev,int index)
{
	int ret=0;
	cdev_init(dev,&ss205_ops);
	dev->owner=THIS_MODULE;
	dev->ops=&ss205_ops;
	ret=cdev_add(dev,ss205_dev_t_p->devno,1);
	if (ret)
	{
		__D("Error %d adding GZ%d\n",ret,index);
		goto seterr_a;
	}
	ss205_dev_t_p->class = class_create(THIS_MODULE, "ss205_class");
	if (IS_ERR(ss205_dev_t_p->class))
	{
		ret = -EINVAL;
		__D("ss205: failed in creating class.\n");
		goto seterr_b;
	}
#ifdef S3C24XX_PLATFORM
	if(NULL==class_device_create(ss205_dev_t_p->class, NULL, ss205_dev_t_p->devno, NULL, "printer"))
#else
	if(NULL==device_create(ss205_dev_t_p->class, NULL, ss205_dev_t_p->devno, NULL, "printer"))
#endif
	{
		ret = -EINVAL;
		__D("ss205: failed in creating class device.\n");
		goto seterr_c;
	}
	return ret;
seterr_c:
	class_destroy(ss205_dev_t_p->class);
seterr_b:
	cdev_del(&ss205_dev_t_p->cdev);
seterr_a:
	return ret;
}

static int __init ss205_init_register(void)
{
	int ret = -ENODEV;
	/** 为ss205设备结构分配空间 */
	ss205_dev_t_p=kzalloc(sizeof(struct ss205_dev_t),GFP_KERNEL);
	if(ss205_dev_t_p == NULL)
	{
		ret = -ENOMEM;
		goto regerr_a;
	}

	/** 将寄存器的物理地址重新映射 */
	ss205_dev_t_p->ctrl_vaddr = ioremap_nocache(SS205_CTRL_PHYADDR, 1);
	ss205_dev_t_p->feedline_vaddr = ioremap_nocache(SS205_FEEDLINE_PHYADDR, 1);
	ss205_dev_t_p->printline_vaddr = ioremap_nocache(SS205_PRINTLINE_PHYADDR, 1);
	ss205_dev_t_p->heattime_vaddr = ioremap_nocache(SS205_HEATTIME_PHYADDR, 1);
	ss205_dev_t_p->buffstat_vaddr = ioremap_nocache(SS205_BUFFSTAT_PHYADDR, 1);
	ss205_dev_t_p->chargeon_vaddr = ioremap_nocache(SS205_CHARGEON_PHYADDR, 1);
	ss205_dev_t_p->printdata_vaddr = ioremap_nocache(SS205_PRINTDATA_PHYADDR, SS205_PRINTDATA_SIZE); 


	pr_alert("%d:ctrl is %02x \n",__LINE__, *(ss205_dev_t_p->ctrl_vaddr));
	/** 重置SS205并初始化加热时间寄存器 */
	SS205_INIT();
	SS205_SET_HEATTIME(192);

	/** 初始化写入缓冲 */
	spin_lock_init(&ss205_dev_t_p->kfifo_lock);
	ss205_dev_t_p->kfifo_order = 6;
	ss205_dev_t_p->kfifo_buffer = (unsigned char *) __get_free_pages(GFP_KERNEL, ss205_dev_t_p->kfifo_order);
	if (!ss205_dev_t_p->kfifo_buffer)
	{
		__D("kfifo_buffer malloc error \n");
		ret =  -ENOMEM ;
		goto  regerr_b;
	}
	ss205_dev_t_p->kfifo_maxlen = PAGE_SIZE<<ss205_dev_t_p->kfifo_order;
	memset(ss205_dev_t_p->kfifo_buffer,0,ss205_dev_t_p->kfifo_maxlen);

	ss205_dev_t_p->kfifo = kfifo_init(
			ss205_dev_t_p->kfifo_buffer, 
			ss205_dev_t_p->kfifo_maxlen,
			GFP_KERNEL, 
			&(ss205_dev_t_p->kfifo_lock));

	/** 注册中断 */
	ret = ss205_all_irq_request();
	if (ret) {
		__D("request irq_ctrlchanged error\n");
		ret = -EINVAL;
		goto regerr_c;
	}

	/** 由系统分配设备号 */
	ret=alloc_chrdev_region(&(ss205_dev_t_p->devno),0,1,"printer");
	if (ret<0)
	{
		__D("ss205fpga init_module failed with %d.\n",ret);
		goto regerr_d;
	}
	else
	{
		__D("ss205fpga init_module success, major=%d, minor=%d.\n", 
				MAJOR(ss205_dev_t_p->devno), MINOR(ss205_dev_t_p->devno));
	}

	/** 注册设备并建立设备节点 */
	ret=ss205_setup_cdev(&(ss205_dev_t_p->cdev),0);
	if(ret)
	{
		ret = -ENODEV;
		goto regerr_e;
	};

	/** 注册workqueue */
	ss205_dev_t_p->print_workqueue = create_singlethread_workqueue("print_queue");
	INIT_WORK(&ss205_dev_t_p->print_workstruct,print_workqueue_func);

	ss205_dev_t_p->is_printing=FALSE;

	//ss205_enable_checkpaper(FALSE);

	pr_alert("%d:ctrl is %02x \n",__LINE__, *(ss205_dev_t_p->ctrl_vaddr));
	return 0;
regerr_e:
	unregister_chrdev_region(ss205_dev_t_p->devno, 1);
regerr_d:
	ss205_all_irq_free();
regerr_c:
	free_pages((unsigned long)ss205_dev_t_p->kfifo_buffer,ss205_dev_t_p->kfifo_order);
regerr_b:
	iounmap(ss205_dev_t_p->ctrl_vaddr);
	iounmap(ss205_dev_t_p->feedline_vaddr);
	iounmap(ss205_dev_t_p->printline_vaddr);
	iounmap(ss205_dev_t_p->heattime_vaddr);
	iounmap(ss205_dev_t_p->buffstat_vaddr);
	iounmap(ss205_dev_t_p->chargeon_vaddr);
	iounmap(ss205_dev_t_p->printdata_vaddr);
	kfree(ss205_dev_t_p);
regerr_a:
	return ret;
}

static int __init ss205_probe(void)
{
	int  ret;
	ret = ss205_init_register();
	return ret;
}

static void __exit ss205_remove(void)
{
#ifdef S3C24XX_PLATFORM
	class_device_destroy(ss205_dev_t_p->class,ss205_dev_t_p->devno);
#elif defined(MX25X_PLATFORM)
	device_destroy(ss205_dev_t_p->class,ss205_dev_t_p->devno);
#endif
	class_destroy(ss205_dev_t_p->class);
	cdev_del(&ss205_dev_t_p->cdev);
	iounmap(ss205_dev_t_p->ctrl_vaddr);
	iounmap(ss205_dev_t_p->feedline_vaddr);
	iounmap(ss205_dev_t_p->printline_vaddr);
	iounmap(ss205_dev_t_p->heattime_vaddr);
	iounmap(ss205_dev_t_p->buffstat_vaddr);
	iounmap(ss205_dev_t_p->chargeon_vaddr);
	iounmap(ss205_dev_t_p->printdata_vaddr);
	unregister_chrdev_region(ss205_dev_t_p->devno, 1);
	ss205_all_irq_free();	
	kfree(ss205_dev_t_p);
}

static int  ss205_suspend(struct platform_device *dev, pm_message_t state)
{   
    return 0;
}

static int  ss205_resume(struct platform_device *dev, pm_message_t state)
{   
    return 0;
}


static struct platform_device ss205_device ={
                .name = SS205_NAME,
                .id   = -1,
};

static struct platform_driver  ss205_driver ={
                .probe     = ss205_probe,
                .remove    = ss205_remove,
                .suspend   = ss205_suspend,
                .resume    = ss205_resume,
                .driver    = {
                             .name   = SS205_NAME,
                             .owner  = THIS_MODULE,
                          },
};

static int __init ss205_init(void)
{
    int ret;
    
    ret = platform_device_register(&ss205_device);
    
    if(ret != 0)
    {
        printk("%s:platform_device_register result: %d\n", __func__, ret);
        return ret;
    }

    ret = platform_driver_register(&ss205_driver);

    if(ret != 0)
    {
        printk("%s:platform_driver_register result: %d\n",  __func__, ret);
        platform_device_unregister(&ss205_device);
    }

    return ret;        
}    

static void __exit ss205_exit(void)
{
    platform_driver_unregister(&ss205_driver );
    platform_device_unregister(&ss205_device);    
}

module_init(ss205_init);
module_exit(ss205_exit);

MODULE_AUTHOR("zengshu@gmail.com");
MODULE_LICENSE("Dual BSD/GPL");
/** @}*/ //注释文件结束
