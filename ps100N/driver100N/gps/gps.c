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
#include <linux/i2c.h>

#include <mach-mx25/board-mx25_3stack.h>
#include <mach-mx25/iomux.h>
#include <mach-mx25/mx25_pins.h>
#include <mach-mx25/machine.h>

#define GPS_CS_ADDR 0xA0700000

#define GPS_IOC_MAGIC		'P'
#define GPS_PWR_ON			_IOW(GPS_IOC_MAGIC,1, int)
#define GPS_PWR_OFF			_IOW(GPS_IOC_MAGIC,2, int)
#define GPS_PWR_STATE		_IOW(GPS_IOC_MAGIC,3, int)
#define GPS_IOC_MAXNR		3

static int dbg = 0;
module_param(dbg, int, S_IRUGO);

struct gps_ctl_dev
{
	struct cdev cdev;
	struct class *class;
	dev_t devno;
	struct i2c_client *i2c_client;
	volatile uint8_t *reg_gps_cs;
	bool pwr_state;
}*gps_devp;

struct gps_ctl_dev  gps_dev;

static void gps_set_pwr(int val)
{
	if(val)
	{
		iowrite8(ioread8(gps_devp->reg_gps_cs)|(1<<7),
				gps_devp->reg_gps_cs);
		gps_dev.pwr_state=1;
	}
	else
	{
		iowrite8(ioread8(gps_devp->reg_gps_cs)&(~(1<<7)),
				gps_devp->reg_gps_cs);
		gps_dev.pwr_state=0;
	}
}

static int gps_i2c_read2(u8 reg,u8* pbuf,size_t size)
{
	struct i2c_client *client = gps_dev.i2c_client;
	int i2c_ret;
	u8 buf0[1];
	u16 addr = client->addr;
	u16 flags = client->flags;
	struct i2c_msg msg[2] = {
		{addr, flags, 1, buf0},
		{addr, flags | I2C_M_RD, size, pbuf},
	};

	buf0[0] = (reg);
	i2c_ret = i2c_transfer(client->adapter, msg, 2);
	if (i2c_ret < 0) {
		if(dbg)
			printk("%s: read reg error : Reg 0x%02x\n", __func__, reg);
		return -1;
	}

	return 0;
}

#if 0//not use this function
static u8 gps_i2c_read(u8 reg)
{
	struct i2c_client *client = gps_dev.i2c_client;
	int i2c_ret;
	u8 buf0[1], buf1[1];
	u16 addr = client->addr;
	u16 flags = client->flags;
	struct i2c_msg msg[2] = {
		{addr, flags, 1, buf0},
		{addr, flags | I2C_M_RD, 1, buf1},
	};

	buf0[0] = (reg);
	i2c_ret = i2c_transfer(client->adapter, msg, 2);
	if (i2c_ret < 0) {
		if(dbg)
			printk("%s: read reg error : Reg 0x%02x\n", __func__, reg);
		return 0;
	}

	return buf1[0];
}
#endif

static int gps_i2c_write(const char* buf, size_t count)
{
	struct i2c_client *client = gps_dev.i2c_client;
	u16 addr = client->addr;
	u16 flags = client->flags;
	int i2c_ret;
	char* buffer;
	struct i2c_msg msg;

	buffer=kzalloc(count+1,GFP_KERNEL);
	if(buffer<0)
	{
		return -1;
	}

	msg.addr=addr;
	msg.flags=flags;
	msg.len=count+1;
	msg.buf=buffer;

	buffer[0] = 0xFF;
	memcpy(buffer+1,buf,count);

	i2c_ret = i2c_transfer(client->adapter, &msg, 1);
	kfree(buffer);
	if (i2c_ret < 0) {
		if(dbg)
		{
			pr_err("%s: write reg error : Reg 0x%02x = %s\n",
					__func__, 0xFF, buf);
		}
		return -EIO;
	}
	return i2c_ret;
}


static int gps_ioctl (struct inode *inode , struct file *file, unsigned int cmd,unsigned long data)
{   
	int ret = 0;

        if (_IOC_TYPE(cmd) != GPS_IOC_MAGIC)
        {
                if(dbg)
                        pr_err("IOC_MAGIC not match.\n");
                return -ENOTTY;
        }
        if (_IOC_NR(cmd) > GPS_IOC_MAXNR)
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
		case GPS_PWR_ON:
			gps_set_pwr(1);
			ret=0;
			break;
		case GPS_PWR_OFF:
			gps_set_pwr(0);
			ret=0;
			break;
		case GPS_PWR_STATE:
			ret=gps_dev.pwr_state;
			break;
		default:
			if(dbg)
			{
				printk("\nunsupport ioctl.\n");
			}
			ret=-1;
	}
	return ret;
}

static ssize_t gps_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
{
	int retval = 0;
	u8* data;
	u8 datasize_buf[2]={0};
	u16 datasize=0;
	gps_i2c_read2(0xFD,((u8*)datasize_buf),2);
	datasize = datasize_buf[0]<<8 | datasize_buf[1];
	pr_alert("datasize=0x%x:%d\n",datasize,datasize);
	pr_alert("gps_devp->reg_gps_cs=0x%02x\n",*gps_devp->reg_gps_cs);

	if(datasize == 0)
	{
		retval = -EFAULT;
		goto nothing;
	}

	if(datasize > count)
	{
		datasize = count;
	}

	data=kzalloc(datasize,GFP_KERNEL);

	gps_i2c_read2(0xFF,((u8*)data),datasize);

	if (copy_to_user(buf, data, datasize)) {
		if(dbg)
			printk("failed to do copy_to_user!\n");
		retval = -EFAULT;
		goto nothing;
	}
	kfree(data);



	return datasize;

nothing:
	return retval;
}

static ssize_t gps_write (struct file *file ,const char *buf, size_t count, loff_t *f_ops)
{
	int ret=gps_i2c_write(buf, count);//If right return = count+1;

    return ret<0?ret:count;
}

static int gps_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int gps_release(struct inode *inode, struct file *filp)
{
	return 0;	
}

static struct file_operations gps_ops ={
	.owner = THIS_MODULE,
	.ioctl = gps_ioctl,
	.read = gps_read,
	.write = gps_write,
	.open = gps_open,
	.release = gps_release,
};

static int gps_setup_cdev(struct cdev *dev, int index)
{
	int err=0;

	cdev_init(dev,&gps_ops);

	dev->owner = THIS_MODULE;
	dev->ops = &gps_ops;
	err = cdev_add(dev,gps_dev.devno,1);
	if(err)
	{
		if(dbg)
			printk(KERN_NOTICE "Error %d adding GZ%d", err, index);
		goto a;
	}

	gps_dev.class = class_create(THIS_MODULE, "gps_class");
	if (IS_ERR(gps_dev.class))
	{
		err = -EINVAL;
		if(dbg)
		{
			pr_err("gps: failed in creating class.\n");
		}
		goto b;
	}

	if(NULL==device_create(gps_dev.class, NULL, gps_dev.devno, NULL, "gps"))
	{
		err = -EINVAL;
		if(dbg)
		{
			pr_err("gps: failed in creating class device.\n");
		}
		goto c;
	}
	return err;
c:
	class_destroy(gps_dev.class);
b:
	cdev_del(&gps_dev.cdev);
a:
	return err;
}

static int gps_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = -ENODEV;
	memset(&gps_dev, 0, sizeof(gps_dev));
	gps_dev.i2c_client = client;

	ret = alloc_chrdev_region(&(gps_dev.devno), 0, 1, "gps");

	if(ret < 0)
	{
		if(dbg)
		{
			pr_alert("imx258 init_module failed with %d\n",ret);
		}
		return ret;
	}
	else
	{	
		if(dbg)
		{
			pr_alert("gps success!!!\n");
		}
	}

	gps_devp = &gps_dev;
	ret=gps_setup_cdev(&(gps_dev.cdev), 0);
	gps_devp->reg_gps_cs = ioremap_nocache(GPS_CS_ADDR,1);

	gps_set_pwr(0);
	return ret;
}

static int gps_remove(struct i2c_client *client)
{
	device_destroy(gps_dev.class,gps_dev.devno);
	class_destroy(gps_dev.class);	
	cdev_del(&gps_devp->cdev);
	unregister_chrdev_region(MKDEV(gps_dev.devno,0), 1);

	return 0;
}

static const struct i2c_device_id gps_id[] = {
	{"gps", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, gps_id);

static struct i2c_driver gps_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "gps",
	},
	.probe = gps_probe,
	.remove = gps_remove,
	.id_table = gps_id,
};

static __init int gps_init(void)
{
	u8 err;

	if(dbg)
		pr_alert("In gps_init\n");

	err = i2c_add_driver(&gps_i2c_driver);
	if (err != 0)
	{
		if(dbg)
		{
			pr_err("%s:driver registration failed, error=%d \n",
					__func__, err);
		}
	}
	return err;
}

static void __exit gps_clean(void)
{
	if(dbg)
	{
		pr_alert("In gps_clean\n");
	}
	i2c_del_driver(&gps_i2c_driver);
}

module_init(gps_init);
module_exit(gps_clean);

MODULE_AUTHOR("zengshu");
MODULE_DESCRIPTION("gps Driver");
MODULE_LICENSE("GPL");
