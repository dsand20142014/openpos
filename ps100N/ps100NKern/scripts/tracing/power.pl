/*
 * OHCI HCD(Host Controller Driver) for USB.
 *
 *(C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *(C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 *(C) Copyright 2002 Hewlett-Packard Company
 *
 * Bus glue for Toshiba Mobile IO(TMIO) Controller's OHCI core
 * (C) Copyright 2005 Chris Humbert <mahadri-usb@drigon.com>
 * (C) Copyright 2007, 2008 Dmitry Baryshkov <dbaryshkov@gmail.com>
 *
 * This is known to work with the following variants:
 *	TC6393XB revision 3	(32kB SRAM)
 *
 * The TMIO's OHCI core DMAs through a small internal buffer that
 * is directly addressable by the CPU.
 *
 * Written from sparse documentation from Toshiba and Sharp's driver
 * for the 2.4 kernel,
 *	usb-ohci-tc6393.c(C) Copyright 2004 Lineo Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/sched.h>*/
#include <linux/platform_device.h>
#include <linux/mfd/core.h>
#include <linux/mfd/tmio.h>
#include <linux/dma-mapping.h>

/*-------------------------------------------------------------------------*/

/*
 * USB Host Controller Configuration Register
 */
#define CCR_REVID	0x08	/* b Revision ID				*/
#define CCR_BASE	0x10	/* l USB Control Register Base Address Low	*/
#define CCR_ILME	0x40	/* b Internal Local Memory Enable		*/
#define CCR_PM		0x4c	/* w Power Management			*/
#define CCR_INTC	0x50	/* b INT Control				*/
#define CCR_LMW1L	0x54	/* w Local Memory Window 1 LMADRS Low	*/
#define CCR_LMW1H	0x56	/* w Local Memory Window 1 LMADRS High	*/
#define CCR_LMW1BL	0x58	/* w Local Memory Window 1 Base Address Low	*/
#define CCR_LMW1BH	0x5A	/* w Local Memory Window 1 Base Address High	*/
#define CCR_LMW2L	0x5C	/* w Local Memory Window 2 LMADRS Low	*/
#define CCR_LMW2H	0x5E	/* w Local Memory Window 2 LMADRS High	*/
#define CCR_LMW2BL	0x60	/* w Local Memory Window 2 Base Address Low	*/
#define CCR_LMW2BH	0x62	/* w Local Memory Window 2 Base Address High	*/
#define CCR_MISC	0xFC	/* b MISC					*/

#define CCR_PM_GKEN      0x0001
#define CCR_PM_CKRNEN    0x0002
#define CCR_PM_USBPW1    0x0004
#define CCR_PM_USBPW2    0x0008
#define CCR_PM_USBPW3    0x0008
#define CCR_PM_PMEE      0x0100
#define CCR_PM_PMES      0x8000

/*-------------------------------------------------------------------------*/

struct tmio_hcd {
	void __iomem		*ccr;
	spinlock_t		lock; /* protects RMW cycles */
};

#define hcd_to_tmio(hcd)	((struct tmio_hcd *)(hcd_to_ohci(hcd) + 1))

/*-------------------------------------------------------------------------*/

static void tmio_write_pm(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct tmio_hcd *tmio = hcd_to_tmio(hcd);
	u16 pm;
	unsigned long flags;

	spin_lock_irqsave(&tmio->lock, flags);

	pm = CCR_PM_GKEN | CCR_PM_CKRNEN |
	     CCR_PM_PMEE | CCR_PM_PMES;

	tmio_iowrite16(pm, tmio->ccr + CCR_PM);
	spin_unlock_irqrestore(&tmio->lock, flags);
}

static void tmio_stop_hc(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	struct tmio_hcd *tmio = hcd_to_tmio(hcd);
	u16 pm;

	pm = CCR_PM_GKEN | CCR_PM_CKRNEN;
	switch