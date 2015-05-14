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

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>

#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include "m-u111s2.h"

static ssize_t mu111s2_write (struct file *file ,const char *buf, size_t count, loff_t *f_ops)
{
    return count;
}

static int mu111s2_ioctl (struct inode *inode ,struct file *file, unsigned int cmd, unsigned long data)
{
    return 0;
}

ssize_t mu111s2_open (struct inode *inode ,struct file *file)
{
    return 0;
}

ssize_t mu111s2_release (struct inode  *inode ,struct file *file)
{
    return 0;
}

struct file_operations mu111s2_ops =
{
    .write = mu111s2_write,
    .ioctl = mu111s2_ioctl,
    .open = mu111s2_open,
    .release = mu111s2_release,
};
