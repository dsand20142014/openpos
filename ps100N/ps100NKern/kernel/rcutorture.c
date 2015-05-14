/*
 * Read-Copy Update module-based torture test facility
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2005, 2006
 *
 * Authors: Paul E. McKenney <paulmck@us.ibm.com>
 *          Josh Triplett <josh@freedesktop.org>
 *
 * See also:  Documentation/RCU/torture.txt
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/rcupdate.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/moduleparam.h>
#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/freezer.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/srcu.h>
#include <linux/slab.h>
#include <asm/byteorder.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul E. McKenney <paulmck@us.ibm.com> and "
              "Josh Triplett <josh@freedesktop.org>");

static int nreaders = -1;	/* # reader threads, defaults to 2*ncpus */
static int nfakewriters = 4;	/* # fake writer threads */
static int stat_interval;	/* Interval between stats, in seconds. */
				/*  Defaults to "only at end of test". */
static int verbose;		/* Print more debug info. */
static int test_no_idle_hz;	/* Test RCU's support for tickless idle CPUs. */
static int shuffle_interval = 3; /* Interval between shuffles (in sec)*/
static int stutter = 5;		/* Start/stop testing interval (in sec) */
static int irqreader = 1;	/* RCU readers from irq (timers). */
static char *torture_type = "rcu"; /* What RCU implementation to torture. */

module_param(nreaders, int, 0444);
MODULE_PARM_DESC(nreaders, "Number of RCU reader threads");
module_param(nfakewriters, int, 0444);
MODULE_PARM_DESC(nfakewriters, "Number of RCU fake writer threads");
module_param(stat_interval, int, 0444);
MODULE_PARM_DESC(stat_interval, "Number of seconds between stats printk()s");
module_param(verbose, bool, 0444);
MODULE_PARM_DESC(verbose, "Enable verbose debugging printk()s");
module_param(test_no_idle_hz, bool, 0444);
MODULE_PARM_DESC(test_no_idle_hz, "Test support for tickless idle CPUs");
module_param(shuffle_interval, int, 0444);
MODULE_PARM_DESC(shuffle_interval, "Number of seconds between shuffles");
module_param(stutter, int, 0444);
MODULE_PARM_DESC(stutter, "Number of seconds to run/halt test");
module_param(irqreader, int, 0444);
MODULE_PARM_DESC(irqreader, "Allow RCU readers from irq handlers");
module_param(torture_type, charp, 0444);
MODULE_PARM_DESC(torture_type, "Type of RCU to torture (rcu, rcu_bh, srcu)");

#define TORTURE_FLAG "-torture:"
#define PRINTK_STRING(s) \
	do { printk(KERN_ALERT "%s" TORTURE_FLAG s "\n", torture_type); } while (0)
#define VERBOSE_PRINTK_STRING(s) \
	do { if (verbose) printk(KERN_ALERT "%s" TORTURE_FLAG s "\n", torture_type); } while (0)
#define VERBOSE_PRINTK_ERRSTRING(s) \
	do { if (verbose) printk(KERN_ALERT "%s" TORTURE_FLAG "!!! " s "\n", torture_type); } while (0)

static char printk_buf[4096];

static int nrealreaders;
static struct task_struct *writer_task;
static struct task_struct **fakewriter_tasks;
static struct task_struct **reader_tasks;
static struct task_struct *stats_task;
static struct task_struct *shuffler_task;
static struct task_struct *stutter_task;
/*

    bttv-gpio.c  --  gpio sub drivers

    sysfs-based sub driver interface for bttv
    mainly intented for gpio access


    Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
			   & Marcus Metzler (mocm@thp.uni-koeln.de)
    (c) 1999-2003 Gerd Knorr <kraxel@bytesex.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/io.h>

#include "bttvp.h"

/* ----------------------------------------------------------------------- */
/* internal: the bttv "bus"                                                */

static int bttv_sub_bus_match(struct device *dev, struct device_driver *drv)
{
	struct bttv_sub_driver *sub = to_bttv_sub_drv(drv);
	int len = strlen(sub->wanted);

	if (0 == strncmp(dev_name(dev), sub->wanted, len))
		return 1;
	return 0;
}

static int bttv_sub_probe(struct device *dev)
{
	struct bttv_sub_device *sdev = to_bttv_sub_dev(dev);
	struct bttv_sub_driver *sub = to_bttv_sub_drv(dev->driver);

	return sub->probe ? sub->probe(sdev) : -ENODEV;
}

static int bttv_sub_remove(struct device *dev)
{
	struct bttv_sub_device *sdev = to_bttv_sub_dev(dev);
	struct bttv_sub_driver *sub = to_bttv_sub_drv(dev->driver);

	if (sub->remove)
		sub->remove(sdev);
	return 0;
}

struct bus_type bttv_sub_bus_type = {
	.name   = "bttv-sub",
	.match  = &bttv_sub_bus_match,
	.probe  = bttv_sub_probe,
	.remove = bttv_sub_remove,
};

static void release_sub_device(struct device *dev)
{
	struct bttv_sub_device *sub = to_bttv_sub_dev(dev);
	kfree(sub);
}

int bttv_sub_add_device(struct bttv_core *core, char *name)
{
	struct bttv_sub_device *sub;
	int err;

	sub = kzalloc(sizeof(*sub),GFP_KERNEL);
	if (NULL == sub)
		return -ENOMEM;

	sub->core        = core;
	sub->dev.parent  = &core->pci->dev;
	sub->dev.bus     = &bttv_sub_bus_type;
	sub->dev.release = release_sub_device;
	dev_set_name(&sub->dev, "%s%d", name, core->nr);

	err = device_register(&sub->dev);
	if (0 != err) {
		kfree(sub);
		return err;
	}
	printk("bttv%d: add subdevice \"%s\"\n", core->nr, dev_name(&sub->dev));
	list_add_tail(&sub->list,&core->subs);
	return 0;
}

int bttv_sub_del_devices(struct bttv_core *core)
{
	struct bttv_sub_device *sub, *save;

	list_for_each_entry_safe(sub, save, &core->subs, list) {
		list_del(&sub->list);
		device_unregister(&sub->dev);
	}
	return 0;
}

/* ----------------------------------------------------------------------- */
/* external: sub-driver register/unregister                                */

int bttv_sub_register(struct bttv_sub_driver *sub, char *wanted)
{
	sub->drv.bus = &bttv_sub_bus_type;
	snprintf(sub->wanted,sizeof(sub->wanted),"%s",wanted);
	return driver_register(&sub->drv);
}
EXPORT_SYMBOL(bttv_sub_register);

int bttv_sub_unregister(struct bttv_sub_driver *sub)
{
	driver_unregister(&sub->drv);
	return 0;
}
EXPORT_SYMBOL(bttv_sub_unregister);

/* ----------------------------------------------------------------------- */
/* external: gpio access functions                                         */

void bttv_gpio_inout(struct bttv_core *core, u32 mask, u32 outbits)
{
	struct bttv *btv = container_of(core, struct bttv, c);
	unsigned long flags;
	u32 data;

	spin_lock_irqsave(&btv->gpio_lock,flags);
	data = btread(BT848_GPIO_OUT_EN);
	data = data & ~mask;
	data = data | (mask & outbits);
	btwrite(data,BT848_GPIO_OUT_EN);
	spin_unlock_irqrestore(&btv->gpio_lock,flags);
}

u32 bttv_gpio_read(struct bttv_core *core)
{
	struct bttv *btv = container_of(core, struct bttv, c);
	u32 value;

	value = btread(BT848_GPIO_DATA);
	return value;
}

void bttv_gpio_write(struct bttv_core *core, u32 value)
{
	struct bttv *btv = container_of(core, struct bttv, c);

	btwrite(value,BT848_GPIO_DATA);
}

void bttv_gpio_bits(struct bttv_core *core, u32 mask, u32 bits)
{
	struct bttv *btv = container_of(core, struct bttv, c);
	unsigned long flags;
	u32 data;

	spin_lock_irqsave(&btv->gpio_lock,flags);
	data = btread(BT848_GPIO_DATA);
	data = data & ~mask;
	data = data | (mask & bits);
	btwrite(data,BT848_GPIO_DATA);
	spin_unlock_irqrestore(&btv->gpio_lock,flags);
}

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      /* OmniVision* camera chip driver private definitions for core code and
 * chip-specific code
 *
 * Copyright (c) 1999-2004 Mark McClelland
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. NO WARRANTY OF ANY KIND is expressed or implied.
 *
 * * OmniVision is a trademark of OmniVision Technologies, Inc. This driver
 * is not sponsored or developed by them.
 */

#ifndef __LINUX_OVCAMCHIP_PRIV_H
#define __LINUX_OVCAMCHIP_PRIV_H

#include <linux/i2c.h>
#include <media/v4l2-subdev.h>
#include <media/ovcamchip.h>

#ifdef DEBUG
extern int ovcamchip_debug;
#endif

#define PDEBUG(level, fmt, args...) \
	if (ovcamchip_debug >= (level))	pr_debug("[%s:%d] " fmt "\n", \
		__func__, __LINE__ , ## args)

#define DDEBUG(level, dev, fmt, args...) \
	if (ovcamchip_debug >= (level))	dev_dbg(dev, "[%s:%d] " fmt "\n", \
		__func__, __LINE__ , ## args)

/* Number of times to retry chip detection. Increase this if you are getting
 * "Failed to init camera chip" */
#define I2C_DETECT_RETRIES	10

struct ovcamchip_regvals {
	unsigned char reg;
	unsigned char val;
};

struct ovcamchip_ops {
	int (*init)(struct i2c_client *);
	int (*free)(struct i2c_client *);
	int (*command)(struct i2c_client *, unsigned int, void *);
};

struct ovcamchip {
	struct v4l2_subdev sd;
	struct ovcamchip_ops *sops;
	void *spriv;               /* Private data for OV7x10.c etc... */
	int subtype;               /* = SEN_OV7610 etc... */
	int mono;                  /* Monochrome chip? (invalid until init) */
	int initialized;           /* OVCAMCHIP_CMD_INITIALIZE was successful */
};

static inline struct ovcamchip *to_ovcamchip(struct v4l2_subdev *sd)
{
	return container_of(sd, struct ovcamchip, sd);
}

extern struct ovcamchip_ops ov6x20_ops;
extern struct ovcamchip_ops ov6x30_ops;
extern struct ovcamchip_ops ov7x10_ops;
extern struct ovcamchip_ops ov7x20_ops;
extern struct ovcamchip_ops ov76be_ops;

/* --------------------------------- */
/*              I2C I/O              */
/* --------------------------------- */

static inline int ov_read(struct i2c_client *c, unsigned char reg,
			  unsigned char *value)
{
	int rc;

	rc = i2c_smbus_read_byte_data(c, reg);
	*value = (unsigned char) rc;
	return rc;
}

static inline int ov_write(struct i2c_client *c, unsigned char reg,
			   unsigned char value )
{
	return i2c_smbus_write_byte_data(c, reg, value);
}

/* --------------------------------- */
/*        FUNCTION PROTOTYPES        */
/* --------------------------------- */

/* Functions in ovcamchip_core.c */

extern int ov_write_regvals(struct i2c_client *c,
			    struct ovcamchip_regvals *rvals);

extern int ov_write_mask(struct i2c_client *c, unsigned char reg,
			 unsigned char value, unsigned char mask);

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       INDX( 	                 (   °   è      ¥       v y c .         Ä;     x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   Ä;     x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z    ¥          l m 8 3 2 3 . c . s v n - b a s e                   Ä;     x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z            ¥  l m 8 3 2 3 . c . s v n - b a s e                   x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   Ä;     x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z            ¥  l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s v n - b a s e                   x d     ®;     ·‰<<ŽÐÓÊû!
£ÎPŠ<<ŽÐÒËqº‚Ð `      `Z               l m 8 3 2 3 . c . s ¥ n - b a s e                   ˆ r     ®;     ¤ƒ<<ŽÐKó!
£ÎÓƒ<<ŽÐf¼ÿ·‚Ð 0      ÷,               m a t r i x _ k e y p a d . c . s v n - b a s e       À;     ˆ v     ®;     ,›‡<<ŽÐ+Êò!
£Î Â‡<<ŽÐÒþ·‚Ð        ø               m c 9 s 0 8 d z 6 0 _ k e y b . c . s v n - b a s e   »;     x d     ®;     Nž…<<ŽÐxþõ!
£ÎÎÍ…<<ŽÐßG¸‚Ð 0      ï/               m p r 0 8 4 . c . s v n - b a s e     ³;     x h     ®;     r<<ŽÐ^÷!
£Î‚È<<ŽÐcHs¹‚Ð €      Ùx               m x c _ k e ¥ b . c . s v n - b a s e ±;     x h     ®;     ¶³€<<ŽÐ…÷!
£ÎbÕ€<<ŽÐâ€t¹‚Ð       ´               m x c _ k e y b . h . s v n - b a s e Ë;     € j     ®;     ¤<<ŽÐCÜü!
£Î¨-<<ŽÐ°—çº‚Ð        h               n e w t o n k b d . c . s v n - b a s e       Á;     € n     ®;     Úö‡<<ŽÐGú÷!
£Îè)ˆ<<ŽÐ1Du¹‚Ð 0      ì/               o m a p - k e y p a d . c . s v n - b a s e   ¶;     ˆ r     ®;     ¤1ƒ<<ŽÐ%ú!
£Îèoƒ<<ŽÐ¹î¹‚Ð @      Ð>               p x a 2 7 x _ k e y p a d . ¥ . s v n - b a s e       Â;     ˆ r     ®;      ]ˆ<<ŽÐùÅô!
£Î`„ˆ<<ŽÐ‘¸‚Ð                       p x a 9 3 0 _ r o t a r y . c . s v n - b a s e       Í;     x h     ®;     ¿<<ŽÐ#Žü!
£ÎNú<<ŽÐÄÜâº‚Ð @      :               s p i t z k b d . c . s v n - b a s e É;     € p     ®;     Œ@Œ<<ŽÐšÛó!
£Î$gŒ<<ŽÐƒ{¸‚Ð                       s t m p 3 x x x - k b d . c . s v n - b a s e Å;     x h     ®;      6Š<<ŽÐÚwô!
£Î*cŠ<<ŽÐ2)¸‚Ð        ¦               s t o w a w a y . c ¥ s v n - b a s e È;     x d     ®;     Ôb‹<<ŽÐå€ù!
£Î¢”‹<<ŽÐÿy¹‚Ð 0      ‹%               s u n k b d . c . s v n - b a s e     ²;     x f     ®;      
<<ŽÐEkú!
£ÎÖ=<<ŽÐEûmº‚Ð 0      *               t o s a k b d . c . s v n - b a s e   ´;     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                                                                                                                                           ¥ INDX( 	                (   °   è      ! k c c c               ´;     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t ! b d . c . s v n - b a s e                     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ´;     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . ! . s v n - b a s e                     ´;     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . ! . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ´;     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ´;     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . ! . s v n - b a s e                     x b     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                     ®;     X÷<<ŽÐÆ2ù!
£Î´•‚<<ŽÐ¾y¹‚Ð        «               x t k b d . c . s v n - b a s e                                                                                                                                                                                                                                                   !                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               !                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               !                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ! /*
 * linux/drivers/video/n411.c -- Platform device for N411 EPD kit
 *
 * Copyright (C) 2008, Jaya Kumar
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive for
 * more details.
 *
 * Layout is based on skeletonfb.c by James Simmons and Geert Uytterhoeven.
 *
 * This driver is written to be used with the Hecuba display controller
 * board, and tested with the EInk 800x600 display in 1 bit mode.
 * The interface between Hecuba and the host is TTL based GPIO. The
 * GPIO requirements are 8 writable data lines and 6 lines for control.
 * Only 4 of the controls are actually used here but 6 for future use.
 * The driver requires the IO addresses for data and control GPIO at
 * load time. It is also possible to use this display with a standard
 * PC parallel port.
 *
 * General notes:
 * - User must set dio_addr=0xIOADDR cio_addr=0xIOADDR c2io_addr=0xIOADDR
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/irq.h>

#include <video/hecubafb.h>

static unsigned long dio_addr;
static unsigned long cio_addr;
static unsigned long c2io_addr;
static unsigned long splashval;
static unsigned int nosplash;
static unsigned char ctl;

static void n411_set_ctl(struct hecubafb_par *par, unsigned char bit, unsigned
							char state)
{
	switch (bit) {
	case HCB_CD_BIT:
		if (state)
			ctl &= ~(HCB_CD_BIT);
		else
			ctl |= HCB_CD_BIT;
		break;
	case HCB_DS_BIT:
		if (state)
			ctl &= ~(HCB_DS_BIT);
		else
			ctl |= HCB_DS_BIT;
		break;
	}
	outb(ctl, cio_addr);
}

static unsigned char n411_get_ctl(struct hecubafb_par *par)
{
	return inb(c2io_addr);
}

static void n411_set_data(struct hecubafb_par *par, unsigned char value)
{
	outb(value, dio_addr);
}

static void n411_wait_for_ack(struct hecubafb_par *par, int clear)
{
	int timeout;
	unsigned char tmp;

	timeout = 500;
	do {
		tmp = n411_get_ctl(par);
		if ((tmp & HCB_ACK_BIT) && (!clear))
			return;
		else if (!(tmp & HCB_ACK_BIT) && (clear))
			return;
		udelay(1);
	} while (timeout--);
	printk(KERN_ERR "timed out waiting for ack\n");
}

static int n411_init_control(struct hecubafb_par *par)
{
	unsigned char tmp;
	/* for init, we want the following setup to be set:
	WUP = lo
	ACK = hi
	DS = hi
	RW = hi
	CD = lo
	*/

	/* write WUP to lo, DS to hi, RW to hi, CD to lo */
	ctl = HCB_WUP_BIT | HCB_RW_BIT | HCB_CD_BIT ;
	n411_set_ctl(par, HCB_DS_BIT, 1);

	/* check ACK is not lo */
	tmp = n411_get_ctl(par);
	if (tmp & HCB_ACK_BIT) {
		printk(KERN_ERR "Fail because ACK is already low\n");
		return -ENXIO;
	}

	return 0;
}


static int n411_init_board(struct hecubafb_par *par)
{
	int retval;

	retval = n411_init_control(par);
	if (retval)
		return retval;

	par->send_command(par, APOLLO_INIT_DISPLAY);
	par->send_data(par, 0x81);

	/* have to wait while display resets */
	udelay(1000);

	/* if we were told to splash the screen, we just clear it */
	if (!nosplash) {
		par->send_command(par, APOLLO_ERASE_DISPLAY);
		par->send_data(par, splashval);
	}

	return 0;
}

static struct hecuba_board n411_board = {
	.owner			= THIS_MODULE,
	.init			= n411_init_board,
	.set_ctl		= n411_set_ctl,
	.set_data		= n411_set_data,
	.wait_for_ack		= n411_wait_for_ack,
};

static struct platform_device *n411_device;
static int __init n411_init(void)
{
	int ret;
	if (!dio_addr || !cio_addr || !c2io_addr) {
		printk(KERN_WARNING "no IO addresses supplied\n");
		return -EINVAL;
	}

	/* request our platform independent driver */
	request_module("hecubafb");

	n411_device = platform_device_alloc("hecubafb", -1);
	if (!n411_device)
		return -ENOMEM;

	platform_device_add_data(n411_device, &n411_board, sizeof(n411_board));

	/* this _add binds hecubafb to n411. hecubafb refcounts n411 */
	ret = platform_device_add(n411_device);

	if (ret)
		platform_device_put(n411_device);

	return ret;

}

static void __exit n411_exit(void)
{
	platform_device_unregister(n411_device);
}

module_init(n411_init);
module_exit(n411_exit);

module_param(nosplash, uint, 0);
MODULE_PARM_DESC(nosplash, "Disable doing the splash screen");
module_param(dio_addr, ulong, 0);
MODULE_PARM_DESC(dio_addr, "IO address for data, eg: 0x480");
module_param(cio_addr, ulong, 0);
MODULE_PARM_DESC(cio_addr, "IO address for control, eg: 0x400");
module_param(c2io_addr, ulong, 0);
MODULE_PARM_DESC(c2io_addr, "IO address for secondary control, eg: 0x408");
module_param(splashval, ulong, 0);
MODULE_PARM_DESC(splashval, "Splash pattern: 0x00 is black, 0x01 is white");

MODULE_DESCRIPTION("board driver for n411 hecuba/apollo epd kit");
MODULE_AUTHOR("Jaya Kumar");
MODULE_LICENSE("GPL");

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*
 * Adaptec AIC7xxx device driver for Linux.
 *
 * $Id: //depot/aic7xxx/linux/drivers/scsi/aic7xxx/aic7xxx_osm.c#235 $
 *
 * Copyright (c) 1994 John Aycock
 *   The University of Calgary Department of Computer Science.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Sources include the Adaptec 1740 driver (aha1740.c), the Ultrastor 24F
 * driver (ultrastor.c), various Linux kernel source, the Adaptec EISA
 * config file (!adp7771.cfg), the Adaptec AHA-2740A Series User's Guide,
 * the Linux Kernel Hacker's Guide, Writing a SCSI Device Driver for Linux,
 * the Adaptec 1542 driver (aha1542.c), the Adaptec EISA overlay file
 * (adp7770.ovl), the Adaptec AHA-2740 Series Technical Reference Manual,
 * the Adaptec AIC-7770 Data Book, the ANSI SCSI specification, the
 * ANSI SCSI-2 specification (draft 10c), ...
 *
 * --------------------------------------------------------------------------
 *
 *  Modifications by Daniel M. Eischen (deischen@iworks.InterWorks.org):
 *
 *  Substantially modified to include support for wide and twin bus
 *  adapters, DMAing of SCBs, tagged queueing, IRQ sharing, bug fixes,
 *  SCB paging, and other rework of the code.
 *
 * -----------