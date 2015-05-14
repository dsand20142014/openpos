/*
 * Copyright (c) 2006, 2007, 2008 QLogic Corporation. All rights reserved.
 * Copyright (c) 2003, 2004, 2005, 2006 PathScale, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/spinlock.h>
#include <linux/idr.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>

#include "ipath_kernel.h"
#include "ipath_verbs.h"

static void ipath_update_pio_bufs(struct ipath_devdata *);

const char *ipath_get_unit_name(int unit)
{
	static char iname[16];
	snprintf(iname, sizeof iname, "infinipath%u", unit);
	return iname;
}

#define DRIVER_LOAD_MSG "QLogic " IPATH_DRV_NAME " loaded: "
#define PFX IPATH_DRV_NAME ": "

/*
 * The size has to be longer than this string, so we can append
 * board/chip information to it in the init code.
 */
const char ib_ipath_version[] = IPATH_IDSTR "\n";

static struct idr unit_table;
DEFINE_SPINLOCK(ipath_devs_lock);
LIST_HEAD(ipath_dev_list);

wait_queue_head_t ipath_state_wait;

unsigned ipath_debug = __IPATH_INFO;

module_param_named(debug, ipath_debug, uint, S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(debug, "mask for debug prints");
EXPORT_SYMBOL_GPL(ipath_debug);

unsigned ipath_mtu4096 = 1; /* max 4KB IB mtu by default, if supported */
module_param_named(mtu4096, ipath_mtu4096, uint, S_IRUGO);
MODULE_PARM_DESC(mtu4096, "enable MTU of 4096 bytes, if supported");

static unsigned ipath_hol_timeout_ms = 13000;
module_param_named(hol_timeout_ms, ipath_hol_timeout_ms, uint, S_IRUGO);
MODULE_PARM_DESC(hol_timeout_ms,
	"duration of user app suspension after link failure");

unsigned ipath_linkrecovery = 1;
module_param_named(linkrecovery, ipath_linkrecovery, uint, S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(linkrecovery, "enable workaround for link recovery issue");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("QLogic <support@qlogic.com>");
MODULE_DESCRIPTION("QLogic InfiniPath driver");

/*
 * Table to translate the LINKTRAININGSTATE portion of
 * IBCStatus to a human-readable form.
 */
const char *ipath_ibcstatus_str[] = {
	"Disabled",
	"LinkUp",
	"PollActive",
	"PollQuiet",
	"SleepDelay",
	"SleepQuiet",
	"LState6",		/* unused */
	"LState7",		/* unused */
	"CfgDebounce",
	"CfgRcvfCfg",
	"CfgWaitRmt",
	"CfgIdle",
	"RecovRetrain",
	"CfgTxRevLane",		/* unused before IBA7220 */
	"RecovWaitRmt",
	"RecovIdle",
	/* below were added for IBA7220 */
	"CfgEnhanced",
	"CfgTest",
	"CfgWaitRmtTest",
	"CfgWaitCfgEnhanced",
	"SendTS_T",
	"SendTstIdles",
	"RcvTS_T",
	"SendTst_TS1s",
	"LTState18", "LTState19", "LTState1A", "LTState1B",
	"LTState1C", "LTState1D", "LTState1E", "LTState1F"
};

static void __devexit ipath_remove_one(struct pci_dev *);
static int __devinit ipath_init_one(struct pci_dev *,
				    const struct pci_device_id *);

/* Only needed for registration, nothing else needs this info */
#define PCI_VENDOR_ID_PATHSCALE 0x1fc1
#define PCI_VENDOR_ID_QLOGIC 0x1077
#define PCI_DEVICE_ID_INFINIPATH_HT 0xd
#define PCI_DEVICE_ID_INFINIPATH_PE800 0x10
#define PCI_DEVICE_ID_INFINIPATH_7220 0x7220

/* Number of seconds before our card status check...  */
#define STATUS_TIMEOUT 60

static const struct pci_device_id ipath_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_PATHSCALE, PCI_DEVICE_ID_INFINIPATH_HT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_PATHSCALE, PCI_DEVICE_ID_INFINIPATH_PE800) },
	{ PCI_DEVICE(PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_INFINIPATH_7220) },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, ipath_pci_tbl);

static struct pci_driver ipath_driver = {
	.name = IPATH_DRV_NAME,
	.probe = ipath_init_one,
	.remove = __devexit_p(ipath_remove_one),
	.id_table = ipath_pci_tbl,
	.driver = {
		.groups = ipath_driver_attr_groups,
	},
};

static inline void read_bars(struct ipath_devdata *dd, struct pci_dev *dev,
			     u32 *bar0, u32 *bar1)
{
	int ret;

	ret = pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, bar0);
	if (ret)
		ipath_dev_err(dd, "failed to read bar0 before enable: "
			      "error %d\n", -ret);

	ret = pci_read_config_dword(dev, PCI_BASE_ADDRESS_1, bar1);
	if (ret)
		ipath_dev_err(dd, "failed to read bar1 before enable: "
			      "error %d\n", -ret);

	ipath_dbg("Read bar0 %x bar1 %x\n", *bar0, *bar1);
}

static void ipath_free_devdata(struct pci_dev *pdev,
			       struct ipath_devdata *dd)
{
	unsigned long flags;

	pci_set_drvdata(pdev, NULL);

	if (dd->ipath_unit != -1) {
		spin_lock_irqsave(&ipath_devs_lock, flags);
		idr_remove(&unit_table, dd->ipath_unit);
		list_del(&dd->ipath_list);
		spin_unlock_irqrestore(&ipath_devs_lock, flags);
	}
	vfree(dd);
}

static struct ipath_devdata *ipath_alloc_devdata(struct pci_dev *pdev)
{
	unsigned long flags;
	struct ipath_devdata *dd;
	int ret;

	if (!idr_pre_get(&unit_table, GFP_KERNEL)) {
		dd = ERR_PTR(-ENOMEM);
		goto bail;
	}

	dd = vmalloc(sizeof(*dd));
	if (!dd) {
		dd = ERR_PTR(-ENOMEM);
		goto bail;
	}
	memset(dd, 0, sizeof(*dd));
	dd->ipath_unit = -1;

	spin_lock_irqsave(&ipath_devs_lock, flags);

	ret = idr_get_new(&unit_table, dd, &dd->ipath_unit);
	if (ret < 0) {
		printk(KERN_ERR IPATH_DRV_NAME
		       ": Could not allocate unit ID: error %d\n", -ret);
		ipath_free_devdata(pdev, dd);
		dd = ERR_PTR(ret);
		goto bail_unlock;
	}

	dd->pcidev = pdev;
	pci_set_drvdata(pdev, dd);

	list_add(&dd->ipath_list, &ipath_dev_list);

bail_unlock:
	spin_unlock_irqrestore(&ipath_devs_lock, flags);

bail:
	return dd;
}

static inline struct ipath_devdata *__ipath_lookup(int unit)
{
	return idr_find(&unit_table, unit);
}

struct ipath_devdata *ipath_lookup(int unit)
{
	struct ipath_devdata *dd;
	unsigned long flags;

	spin_lock_irqsave(&ipath_devs_lock, flags);
	dd = __ipath_lookup(unit);
	spin_unlock_irqrestore(&ipath_devs_lock, flags);

	return dd;
}

int ipath_count_units(int *npresentp, int *nupp, int *maxportsp)
{
	int nunits, npresent, nup;
	struct ipath_devdata *dd;
	unsigned long flags;
	int maxports;

	nunits = npresent = nup = maxports = 0;

	spin_lock_irqsave(&ipath_devs_lock, flags);

	list_for_each_entry(dd, &ipath_dev_list, ipath_list) {
		nunits++;
		if ((dd->ipath_flags & IPATH_PRESENT) && dd->ipath_kregbase)
			npresent++;
		if (dd->ipath_lid &&
		    !(dd->ipath_flags & (IPATH_DISABLED | IPATH_LINKDOWN
					 | IPATH_LINKUNK)))
			nup++;
		if (dd->ipath_cfgports > maxports)
			maxports = dd->ipath_cfgports;
	}

	spin_unlock_irqrestore(&ipath_devs_lock, flags);

	if (npresentp)
		*npresentp = npresent;
	if (nupp)
		*nupp = nup;
	if (maxportsp)
		*maxportsp = maxports;

	return nunits;
}

/*
 * These next two routines are placeholders in case we don't have per-arch
 * code for controlling write combining.  If explicit control of write
 * combining is not available, performance will probably be awful.
 */

int __attribute__((weak)) ipath_enable_wc(struct ipath_devdata *dd)
{
	return -EOPNOTSUPP;
}

void __attribute__((weak)) ipath_disable_wc(struct ipath_devdata *dd)
{
}

/*
 * Perform a PIO buffer bandwidth write test, to verify proper system
 * configuration.  Even when all the setup calls work, occasionally
 * BIOS or other issues can prevent write combining from working, or
 * can cause other bandwidth problems to the chip.
 *
 * This test simply writes the same buffer over and over again, and
 * measures close to the peak bandwidth to the chip (not testing
 * data bandwidth to the wire).   On chips that use an address-based
 * trigger to send packets to the wire, this is easy.  On chips that
 * use a count to trigger, we want to make sure that the packet doesn't
 * go out on the wire, or trigger flow control checks.
 */
static void ipath_verify_pioperf(struct ipath_devdata *dd)
{
	u32 pbnum, cnt, lcnt;
	u32 __iomem *piobuf;
	u32 *addr;
	u64 msecs, emsecs;

	piobuf = ipath_getpiobuf(dd, 0, &pbnum);
	if (!piobuf) {
		dev_info(&dd->pcidev->dev,
			"No PIObufs for checking perf, skipping\n");
		return;
	}

	/*
	 * Enough to give us a reasonable test, less than piobuf size, and
	 * likely multiple of store buffer length.
	 */
	cnt = 1024;

	addr = vmalloc(cnt);
	if (!addr) {
		dev_info(&dd->pcidev->dev,
			"Couldn't get memory for checking PIO perf,"
			" skipping\n");
		goto done;
	}

	preempt_disable();  /* we want reasonably accurate elapsed time */
	msecs = 1 + jiffies_to_msecs(jiffies);
	for (lcnt = 0; lcnt < 10000U; lcnt++) {
		/* wait until we cross msec boundary */
		if (jiffies_to_msecs(jiffies) >= msecs)
			break;
		udelay(1);
	}

	ipath_disable_armlaunch(dd);

	/*
	 * length 0, no dwords actually sent, and mark as VL15
	 * on chips where that may matter (due to IB flowcontrol)
	 */
	if ((dd->ipath_flags & IPATH_HAS_PBC_CNT))
		writeq(1UL << 63, piobuf);
	else
		writeq(0, piobuf);
	ipath_flush_wc();

	/*
	 * this is only roughly accurate, since even with preempt we
	 * still take interrupts that could take a while.   Running for
	 * >= 5 msec seems to get us "close enough" to accurate values
	 */
	msecs = jiffies_to_msecs(jiffies);
	for (emsecs = lcnt = 0; emsecs <= 5UL; lcnt++) {
		__iowrite32_copy(piobuf + 64, addr, cnt >> 2);
		emsecs = jiffies_to_msecs(jiffies) - msecs;
	}

	/* 1 GiB/sec, slightly over IB SDR line rate */
	if (lcnt < (emsecs * 1024U))
		ipath_dev_err(dd,
			"Performance problem: bandwidth to PIO buffers is "
			"only %u MiB/sec\n",
			lcnt / (u32) emsecs);
	else
		ipath_dbg("PIO buffer bandwidth %u MiB/sec is OK\n",
			lcnt / (u32) emsecs);

	preempt_enable();

	vfree(addr);

done:
	/* disarm piobuf, so it's available again */
	ipath_disarm_piobufs(dd, pbnum, 1);
	ipath_enable_armlaunch(dd);
}

static int __devinit ipath_init_one(struct pci_dev *pdev,
				    const struct pci_device_id *ent)
{
	int ret, len, j;
	struct ipath_devdata *dd;
	unsigned long long addr;
	u32 bar0 = 0, bar1 = 0;
	u8 rev;

	dd = ipath_alloc_devdata(pdev);
	if (IS_ERR(dd)) {
		ret = PTR_ERR(dd);
		printk(KERN_ERR IPATH_DRV_NAME
		       ": Could not allocate devdata: error %d\n", -ret);
		goto bail;
	}

	ipath_cdbg(VERBOSE, "initializing unit #%u\n", dd->ipath_unit);

	ret = pci_enable_device(pdev);
	if (ret) {
		/* This can happen iff:
		 *
		 * We did a chip reset, and then failed to reprogram the
		 * BAR, or the chip reset due to an internal error.  We then
		 * unloaded the driver and reloaded it.
		 *
		 * Both reset cases set the BAR back to initial state.  For
		 * the latter case, the AER sticky error bit at offset 0x718
		 * should be set, but the Linux kernel doesn't yet know
		 * about that, it appears.  If the original BAR was retained
		 * in the kernel data structures, this may be OK.
		 */
		ipath_dev_err(dd, "enable unit %d failed: error %d\n",
			      dd->ipath_unit, -ret);
		goto bail_devdata;
	}
	addr = pci_resource_start(pdev, 0);
	len = pci_resource_len(pdev, 0);
	ipath_cdbg(VERBOSE, "regbase (0) %llx len %d irq %d, vend %x/%x "
		   "driver_data %lx\n", addr, len, pdev->irq, ent->vendor,
		   ent->device, ent->driver_data);

	read_bars(dd, pdev, &bar0, &bar1);

	if (!bar1 && !(bar0 & ~0xf)) {
		if (addr) {
			dev_info(&pdev->dev, "BAR is 0 (probable RESET), "
				 "rewriting as %llx\n", addr);
			ret = pci_write_config_dword(
				pdev, PCI_BASE_ADDRESS_0, addr);
			if (ret) {
				ipath_dev_err(dd, "rewrite of BAR0 "
					      "failed: err %d\n", -ret);
				goto bail_disable;
			}
			ret = pci_write_config_dword(
				pdev, PCI_BASE_ADDRESS_1, addr >> 32);
			if (ret) {
				ipath_dev_err(dd, "rewrite of BAR1 "
					      "failed: err %d\n", -ret);
				goto bail_disable;
			}
		} else {
			ipath_dev_err(dd, "BAR is 0 (probable RESET), "
				      "not usable until reboot\n");
			ret = -ENODEV;
			goto bail_disable;
		}
	}

	ret = pci_request_regions(pdev, IPATH_DRV_NAME);
	if (ret) {
		dev_info(&pdev->dev, "pci_request_regions unit %u fails: "
			 "err %d\n", dd->ipath_unit, -ret);
		goto bail_disable;
	}

	ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (ret) {
		/*
		 * if the 64 bit setup fails, try 32 bit.  Some systems
		 * do not setup 64 bit maps on systems with 2GB or less
		 * memory installed.
		 */
		ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if (ret) {
			dev_info(&pdev->dev,
				"Unable to set DMA mask for unit %u: %d\n",
				dd->ipath_unit, ret);
			goto bail_regions;
		}
		else {
			ipath_dbg("No 64bit DMA mask, used 32 bit mask\n");
			ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
			if (ret)
				dev_info(&pdev->dev,
					"Unable to set DMA consistent mask "
					"for unit %u: %d\n",
					dd->ipath_unit, ret);

		}
	}
	else {
		ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
		if (ret)
			dev_info(&pdev->dev,
				"Unable to set DMA consistent mask "
				"for unit %u: %d\n",
				dd->ipath_unit, ret);
	}

	pci_set_master(pdev);

	/*
	 * Save BARs to rewrite after device reset.  Save all 64 bits of
	 * BAR, just in case.
	 */
	dd->ipath_pcibar0 = addr;
	dd->ipath_pcibar1 = addr >> 32;
	dd->ipath_deviceid = ent->device;	/* save for later use */
	dd->ipath_vendorid = ent->vendor;

	/* setup the chip-specific functions, as early as possible. */
	switch (ent->device) {
	case PCI_DEVICE_ID_INFINIPATH_HT:
#ifdef CONFIG_HT_IRQ
		ipath_init_iba6110_funcs(dd);
		break;
#else
		ipath_dev_err(dd, "QLogic HT device 0x%x cannot work if "
			      "CONFIG_HT_IRQ is not enabled\n", ent->device);
		return -ENODEV;
#endif
	case PCI_DEVICE_ID_INFINIPATH_PE800:
#ifdef CONFIG_PCI_MSI
		ipath_init_iba6120_funcs(dd);
		break;
#else
		ipath_dev_err(dd, "QLogic PCIE device 0x%x cannot work if "
			      "CONFIG_PCI_MSI is not enabled\n", ent->device);
		return -ENODEV;
#endif
	case PCI_DEVICE_ID_INFINIPATH_7220:
#ifndef CONFIG_PCI_MSI
		ipath_dbg("CONFIG_PCI_MSI is not enabled, "
			  "using INTx for unit %u\n", dd->ipath_unit);
#endif
		ipath_init_iba7220_funcs(dd);
		break;
	default:
		ipath_dev_err(dd, "Found unknown QLogic deviceid 0x%x, "
			      "failing\n", ent->device);
		return -ENODEV;
	}

	for (j = 0; j < 6; j++) {
		if (!pdev->resource[j].start)
			continue;
		ipath_cdbg(VERBOSE, "BAR %d start %llx, end %llx, len %llx\n",
			   j, (unsigned long long)pdev->resource[j].start,
			   (unsigned long long)pdev->resource[j].end,
			   (unsigned long long)pci_resource_len(pdev, j));
	}

	if (!addr) {
		ipath_dev_err(dd, "No valid address in BAR 0!\n");
		ret = -ENODEV;
		goto bail_regions;
	}

	ret = pci_read_config_byte(pdev, PCI_REVISION_ID, &rev);
	if (ret) {
		ipath_dev_err(dd, "Failed to read PCI revision ID unit "
			      "%u: err %d\n", dd->ipath_unit, -ret);
		goto bail_regions;	/* shouldn't ever happen */
	}
	dd->ipath_pcirev = rev;

#if defined(__powerpc__)
	/* There isn't a generic way to specify writethrough mappings */
	dd->ipath_kregbase = __ioremap(addr, len,
		(_PAGE_NO_CACHE|_PAGE_WRITETHRU));
#else
	dd->ipath_kregbase = ioremap_nocache(addr, len);
#endif

	if (!dd->ipath_kregbase) {
		ipath_dbg("Unable to map io addr %llx to kvirt, failing\n",
			  addr);
		ret = -ENOMEM;
		goto bail_iounmap;
	}
	dd->ipath_kregend = (u64 __iomem *)
		((void __iomem *)dd->ipath_kregbase + len);
	dd->ipath_physaddr = addr;	/* used for io_remap, etc. */
	/* for user mmap */
	ipath_cdbg(VERBOSE, "mapped io addr %llx to kregbase %p\n",
		   addr, dd->ipath_kregbase);

	if (dd->ipath_f_bus(dd, pdev))
		ipath_dev_err(dd, "Failed to setup config space; "
			      "continuing anyway\n");

	/*
	 * set up our interrupt handler; IRQF_SHARED probably not needed,
	 * since MSI interrupts shouldn't be shared but won't  hurt for now.
	 * check 0 irq after we return from chip-specific bus setup, since
	 * that can affect this due to setup
	 */
	if (!dd->ipath_irq)
		ipath_dev_err(dd, "irq is 0, BIOS error?  Interrupts won't "
			      "work\n");
	else {
		ret = request_irq(dd->ipath_irq, ipath_intr, IRQF_SHARED,
				  IPATH_DRV_NAME, dd);
		if (ret) {
			ipath_dev_err(dd, "Couldn't setup irq handler, "
				      "irq=%d: %d\n", dd->ipath_irq, ret);
			goto bail_iounmap;
		}
	}

	ret = ipath_init_chip(dd, 0);	/* do the chip-specific init */
	if (ret)
		goto bail_irqsetup;

	ret = ipath_enable_wc(dd);

	if (ret) {
		ipath_dev_err(dd, "Write combining not enabled "
			      "(err %d): performance may be poor\n",
			      -ret);
		ret = 0;
	}

	ipath_verify_pioperf(dd);

	ipath_device_create_group(&pdev->dev, dd);
	ipathfs_add_device(dd);
	ipath_user_add(dd);
	ipath_diag_add(dd);
	ipath_register_ib_device(dd);

	goto bail;

bail_irqsetup:
	if (pdev->irq)
		free_irq(pdev->irq, dd);

bail_iounmap:
	iounmap((volatile void __iomem *) dd->ipath_kregbase);

bail_regions:
	pci_release_regions(pdev);

bail_disable:
	pci_disable_device(pdev);

bail_devdata:
	ipath_free_devdata(pdev, dd);

bail:
	return ret;
}

static void __devexit cleanup_device(struct ipath_devdata *dd)
{
	int port;
	struct ipath_portdata **tmp;
	unsigned long flags;

	if (*dd->ipath_statusp & IPATH_STATUS_CHIP_PRESENT) {
		/* can't do anything more with chip; needs re-init */
		*dd->ipath_statusp &= ~IPATH_STATUS_CHIP_PRESENT;
		if (dd->ipath_kregbase) {
			/*
			 * if we haven't already cleaned up before these are
			 * to ensure any register reads/writes "fail" until
			 * re-init
			 */
			dd->ipath_kregbase = NULL;
			dd->ipath_uregbase = 0;
			dd->ipath_sregbase = 0;
			dd->ipath_cregbase = 0;
			dd->ipath_kregsize = 0;
		}
		ipath_disable_wc(dd);
	}

	if (dd->ipath_spectriggerhit)
		dev_info(&dd->pcidev->dev, "%lu special trigger hits\n",
			 dd->ipath_spectriggerhit);

	if (dd->ipath_pioavailregs_dma) {
		dma_free_coherent(&dd->pcidev->dev, PAGE_SIZE,
				  (void *) dd->ipath_pioavailregs_dma,
				  dd->ipath_pioavailregs_phys);
		dd->ipath_pioavailregs_dma = NULL;
	}
	if (dd->ipath_dummy_hdrq) {
		dma_free_coherent(&dd->pcidev->dev,
			dd->ipath_pd[0]->port_rcvhdrq_size,
			dd->ipath_dummy_hdrq, dd->ipath_dummy_hdrq_phys);
		dd->ipath_dummy_hdrq = NULL;
	}

	if (dd->ipath_pageshadow) {
		struct page **tmpp = dd->ipath_pageshadow;
		dma_addr_t *tmpd = dd->ipath_physshadow;
		int i, cnt = 0;

		ipath_cdbg(VERBOSE, "Unlocking any expTID pages still "
			   "locked\n");
		for (port = 0; port < dd->ipath_cfgports; port++) {
			int port_tidbase = port * dd->ipath_rcvtidcnt;
			int maxtid = port_tidbase + dd->ipath_rcvtidcnt;
			for (i = port_tidbase; i < maxtid; i++) {
				if (!tmpp[i])
					continue;
				pci_unmap_page(dd->pcidev, tmpd[i],
					PAGE_SIZE, PCI_DMA_FROMDEVICE);
				ipath_release_user_pages(&tmpp[i], 1);
				tmpp[i] = NULL;
				cnt++;
			}
		}
		if (cnt) {
			ipath_stats.sps_pageunlocks += cnt;
			ipath_cdbg(VERBOSE, "There were still %u expTID "
				   "entries locked\n", cnt);
		}
		if (ipath_stats.sps_pagelocks ||
		    ipath_stats.sps_pageunlocks)
			ipath_cdbg(VERBOSE, "%llu pages locked, %llu "
				   "unlocked via ipath_m{un}lock\n",
				   (unsigned long long)
				   ipath_stats.sps_pagelocks,
				   (unsigned long long)
				   ipath_stats.sps_pageunlocks);

		ipath_cdbg(VERBOSE, "Free shadow page tid array at %p\n",
			   dd->ipath_pageshadow);
		tmpp = dd->ipath_pageshadow;
		dd->ipath_pageshadow = NULL;
		vfree(tmpp);

		dd->ipath_egrtidbase = NULL;
	}

	/*
	 * free any resources still in use (usually just kernel ports)
	 * at unload; we do for portcnt, because that's what we allocate.
	 * We acquire lock to be really paranoid that ipath_pd isn't being
	 * accessed from some interrupt-related code (that should not happen,
	 * but best to be sure).
	 */
	spin_lock_irqsave(&dd->ipath_uctxt_lock, flags);
	tmp = dd->ipath_pd;
	dd->ipath_pd = NULL;
	spin_unlock_irqrestore(&dd->ipath_uctxt_lock, flags);
	for (port = 0; port < dd->ipath_portcnt; port++) {
		struct ipath_portdata *pd = tmp[port];
		tmp[port] = NULL; /* debugging paranoia */
		ipath_free_pddata(dd, pd);
	}
	kfree(tmp);
}

static void __devexit ipath_remove_one(struct pci_dev *pdev)
{
	struct ipath_devdata *dd = pci_get_drvdata(pdev);

	ipath_cdbg(VERBOSE, "removing, pdev=%p, dd=%p\n", pdev, dd);

	/*
	 * disable the IB link early, to be sure no new packets arrive, which
	 * complicates the shutdown process
	 */
	ipath_shutdown_device(dd);

	flush_scheduled_work();

	if (dd->verbs_dev)
		ipath_unregister_ib_device(dd->verbs_dev);

	ipath_diag_remove(dd);
	ipath_user_remove(dd);
	ipathfs_remove_device(dd);
	ipath_device_remove_group(&pdev->dev, dd);

	ipath_cdbg(VERBOSE, "Releasing pci memory regions, dd %p, "
		   "unit %u\n", dd, (u32) dd->ipath_unit);

	cleanup_device(dd);

	/*
	 * turn off rcv, send, and interrupts for all ports, all drivers
	 * should also hard reset the chip here?
	 * free up port 0 (kernel) rcvhdr, egr bufs, and eventually tid bufs
	 * for all versions of the driver, if they were allocated
	 */
	if (dd->ipath_irq) {
		ipath_cdbg(VERBOSE, "unit %u free irq %d\n",
			   dd->ipath_unit, dd->ipath_irq);
		dd->ipath_f_free_irq(dd);
	} else
		ipath_dbg("irq is 0, not doing free_irq "
			  "for unit %u\n", dd->ipath_unit);
	/*
	 * we check for NULL here, because it's outside
	 * the kregbase check, and we need to call it
	 * after the free_irq.	Thus it's possible that
	 * the function pointers were never initialized.
	 */
	if (dd->ipath_f_cleanup)
		/* clean up chip-specific stuff */
		dd->ipath_f_cleanup(dd);

	ipath_cdbg(VERBOSE, "Unmapping kregbase %p\n", dd->ipath_kregbase);
	iounmap((volatile void __iomem *) dd->ipath_kregbase);
	pci_release_regions(pdev);
	ipath_cdbg(VERBOSE, "calling pci_disable_device\n");
	pci_disable_device(pdev);

	ipath_free_devdata(pdev, dd);
}

/* general driver use */
DEFINE_MUTEX(ipath_mutex);

static DEFINE_SPINLOCK(ipath_pioavail_lock);

/**
 * ipath_disarm_piobufs - cancel a range of PIO buffers
 * @dd: the infinipath device
 * @first: the first PIO buffer to cancel
 * @cnt: the number of PIO buffers to cancel
 *
 * cancel a range of PIO buffers, used when they might be armed, but
 * not triggered.  Used at init to ensure buffer state, and also user
 * process close, in case it died while writing to a PIO buffer
 * Also after errors.
 */
void ipath_disarm_piobufs(struct ipath_devdata *dd, unsigned first,
			  unsigned cnt)
{
	unsigned i, last = first + cnt;
	unsigned long flags;

	ipath_cdbg(PKT, "disarm %u PIObufs first=%u\n", cnt, first);
	for (i = first; i < last; i++) {
		spin_lock_irqsave(&dd->ipath_sendctrl_lock, flags);
		/*
		 * The disarm-related bits are write-only, so it
		 * is ok to OR them in with our copy of sendctrl
		 * while we hold the lock.
		 */
		ipath_write_kreg(dd, dd->ipath_kregs->kr_sendctrl,
			dd->ipath_sendctrl | INFINIPATH_S_DISARM |
			(i << INFINIPATH_S_DISARMPIOBUF_SHIFT));
		/* can't disarm bufs back-to-back per iba7220 spec */
		ipath_read_kreg64(dd, dd->ipath_kregs->kr_scratch);
		spin_unlock_irqrestore(&dd->ipath_sendctrl_lock, flags);
	}
	/* on some older chips, update may not happen after cancel */
	ipath_force_pio_avail_update(dd);
}

/**
 * ipath_wait_linkstate - wait for an IB link state change to occur
 * @dd: the infinipath device
 * @state: the state to wait for
 * @msecs: the number of milliseconds to wait
 *
 * wait up to msecs milliseconds for IB link state change to occur for
 * now, take the easy polling route.  Currently used only by
 * ipath_set_linkstate.  Returns 0 if state reached, otherwise
 * -ETIMEDOUT state can have multiple states set, for any of several
 * transitions.
 */
int ipath_wait_linkstate(struct ipath_devdata *dd, u32 state, int msecs)
{
	dd->ipath_state_wanted = state;
	wait_event_interruptible_timeout(ipath_state_wait,
					 (dd->ipath_flags & state),
					 msecs_to_jiffies(msecs));
	dd->ipath_state_wanted = 0;

	if (!(dd->ipath_flags & state)) {
		u64 val;
		ipath_cdbg(VERBOSE, "Didn't reach linkstate %s within %u"
			   " ms\n",
			   /* test INIT ahead of DOWN, both can be set */
			   (state & IPATH_LINKINIT) ? "INIT" :
			   ((state & IPATH_LINKDOWN) ? "DOWN" :
			    ((state & IPATH_LINKARMED) ? "ARM" : "ACTIVE")),
			   msecs);
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
		ipath_cdbg(VERBOSE, "ibcc=%llx ibcstatus=%llx (%s)\n",
			   (unsigned long long) ipath_read_kreg64(
				   dd, dd->ipath_kregs->kr_ibcctrl),
			   (unsigned long long) val,
			   ipath_ibcstatus_str[val & dd->ibcs_lts_mask]);
	}
	return (dd->ipath_flags & state) ? 0 : -ETIMEDOUT;
}

static void decode_sdma_errs(struct ipath_devdata *dd, ipath_err_t err,
	char *buf, size_t blen)
{
	static const struct {
		ipath_err_t err;
		const char *msg;
	} errs[] = {
		{ INFINIPATH_E_SDMAGENMISMATCH, "SDmaGenMismatch" },
		{ INFINIPATH_E_SDMAOUTOFBOUND, "SDmaOutOfBound" },
		{ INFINIPATH_E_SDMATAILOUTOFBOUND, "SDmaTailOutOfBound" },
		{ INFINIPATH_E_SDMABASE, "SDmaBase" },
		{ INFINIPATH_E_SDMA1STDESC, "SDma1stDesc" },
		{ INFINIPATH_E_SDMARPYTAG, "SDmaRpyTag" },
		{ INFINIPATH_E_SDMADWEN, "SDmaDwEn" },
		{ INFINIPATH_E_SDMAMISSINGDW, "SDmaMissingDw" },
		{ INFINIPATH_E_SDMAUNEXPDATA, "SDmaUnexpData" },
		{ INFINIPATH_E_SDMADESCADDRMISALIGN, "SDmaDescAddrMisalign" },
		{ INFINIPATH_E_SENDBUFMISUSE, "SendBufMisuse" },
		{ INFINIPATH_E_SDMADISABLED, "SDmaDisabled" },
	};
	int i;
	int expected;
	size_t bidx = 0;

	for (i = 0; i < ARRAY_SIZE(errs); i++) {
		expected = (errs[i].err != INFINIPATH_E_SDMADISABLED) ? 0 :
			test_bit(IPATH_SDMA_ABORTING, &dd->ipath_sdma_status);
		if ((err & errs[i].err) && !expected)
			bidx += snprintf(buf + bidx, blen - bidx,
					 "%s ", errs[i].msg);
	}
}

/*
 * Decode the error status into strings, deciding whether to always
 * print * it or not depending on "normal packet errors" vs everything
 * else.   Return 1 if "real" errors, otherwise 0 if only packet
 * errors, so caller can decide what to print with the string.
 */
int ipath_decode_err(struct ipath_devdata *dd, char *buf, size_t blen,
	ipath_err_t err)
{
	int iserr = 1;
	*buf = '\0';
	if (err & INFINIPATH_E_PKTERRS) {
		if (!(err & ~INFINIPATH_E_PKTERRS))
			iserr = 0; // if only packet errors.
		if (ipath_debug & __IPATH_ERRPKTDBG) {
			if (err & INFINIPATH_E_REBP)
				strlcat(buf, "EBP ", blen);
			if (err & INFINIPATH_E_RVCRC)
				strlcat(buf, "VCRC ", blen);
			if (err & INFINIPATH_E_RICRC) {
				strlcat(buf, "CRC ", blen);
				// clear for check below, so only once
				err &= INFINIPATH_E_RICRC;
			}
			if (err & INFINIPATH_E_RSHORTPKTLEN)
				strlcat(buf, "rshortpktlen ", blen);
			if (err & INFINIPATH_E_SDROPPEDDATAPKT)
				strlcat(buf, "sdroppeddatapkt ", blen);
			if (err & INFINIPATH_E_SPKTLEN)
				strlcat(buf, "spktlen ", blen);
		}
		if ((err & INFINIPATH_E_RICRC) &&
			!(err&(INFINIPATH_E_RVCRC|INFINIPATH_E_REBP)))
			strlcat(buf, "CRC ", blen);
		if (!iserr)
			goto done;
	}
	if (err & INFINIPATH_E_RHDRLEN)
		strlcat(buf, "rhdrlen ", blen);
	if (err & INFINIPATH_E_RBADTID)
		strlcat(buf, "rbadtid ", blen);
	if (err & INFINIPATH_E_RBADVERSION)
		strlcat(buf, "rbadversion ", blen);
	if (err & INFINIPATH_E_RHDR)
		strlcat(buf, "rhdr ", blen);
	if (err & INFINIPATH_E_SENDSPECIALTRIGGER)
		strlcat(buf, "sendspecialtrigger ", blen);
	if (err & INFINIPATH_E_RLONGPKTLEN)
		strlcat(buf, "rlongpktlen ", blen);
	if (err & INFINIPATH_E_RMAXPKTLEN)
		strlcat(buf, "rmaxpktlen ", blen);
	if (err & INFINIPATH_E_RMINPKTLEN)
		strlcat(buf, "rminpktlen ", blen);
	if (err & INFINIPATH_E_SMINPKTLEN)
		strlcat(buf, "sminpktlen ", blen);
	if (err & INFINIPATH_E_RFORMATERR)
		strlcat(buf, "rformaterr ", blen);
	if (err & INFINIPATH_E_RUNSUPVL)
		strlcat(buf, "runsupvl ", blen);
	if (err & INFINIPATH_E_RUNEXPCHAR)
		strlcat(buf, "runexpchar ", blen);
	if (err & INFINIPATH_E_RIBFLOW)
		strlcat(buf, "ribflow ", blen);
	if (err & INFINIPATH_E_SUNDERRUN)
		strlcat(buf, "sunderrun ", blen);
	if (err & INFINIPATH_E_SPIOARMLAUNCH)
		strlcat(buf, "spioarmlaunch ", blen);
	if (err & INFINIPATH_E_SUNEXPERRPKTNUM)
		strlcat(buf, "sunexperrpktnum ", blen);
	if (err & INFINIPATH_E_SDROPPEDSMPPKT)
		strlcat(buf, "sdroppedsmppkt ", blen);
	if (err & INFINIPATH_E_SMAXPKTLEN)
		strlcat(buf, "smaxpktlen ", blen);
	if (err & INFINIPATH_E_SUNSUPVL)
		strlcat(buf, "sunsupVL ", blen);
	if (err & INFINIPATH_E_INVALIDADDR)
		strlcat(buf, "invalidaddr ", blen);
	if (err & INFINIPATH_E_RRCVEGRFULL)
		strlcat(buf, "rcvegrfull ", blen);
	if (err & INFINIPATH_E_RRCVHDRFULL)
		strlcat(buf, "rcvhdrfull ", blen);
	if (err & INFINIPATH_E_IBSTATUSCHANGED)
		strlcat(buf, "ibcstatuschg ", blen);
	if (err & INFINIPATH_E_RIBLOSTLINK)
		strlcat(buf, "riblostlink ", blen);
	if (err & INFINIPATH_E_HARDWARE)
		strlcat(buf, "hardware ", blen);
	if (err & INFINIPATH_E_RESET)
		strlcat(buf, "reset ", blen);
	if (err & INFINIPATH_E_SDMAERRS)
		decode_sdma_errs(dd, err, buf, blen);
	if (err & INFINIPATH_E_INVALIDEEPCMD)
		strlcat(buf, "invalideepromcmd ", blen);
done:
	return iserr;
}

/**
 * get_rhf_errstring - decode RHF errors
 * @err: the err number
 * @msg: the output buffer
 * @len: the length of the output buffer
 *
 * only used one place now, may want more later
 */
static void get_rhf_errstring(u32 err, char *msg, size_t len)
{
	/* if no errors, and so don't need to check what's first */
	*msg = '\0';

	if (err & INFINIPATH_RHF_H_ICRCERR)
		strlcat(msg, "icrcerr ", len);
	if (err & INFINIPATH_RHF_H_VCRCERR)
		strlcat(msg, "vcrcerr ", len);
	if (err & INFINIPATH_RHF_H_PARITYERR)
		strlcat(msg, "parityerr ", len);
	if (err & INFINIPATH_RHF_H_LENERR)
		strlcat(msg, "lenerr ", len);
	if (err & INFINIPATH_RHF_H_MTUERR)
		strlcat(msg, "mtuerr ", len);
	if (err & INFINIPATH_RHF_H_IHDRERR)
		/* infinipath hdr checksum error */
		strlcat(msg, "ipathhdrerr ", len);
	if (err & INFINIPATH_RHF_H_TIDERR)
		strlcat(msg, "tiderr ", len);
	if (err & INFINIPATH_RHF_H_MKERR)
		/* bad port, offset, etc. */
		strlcat(msg, "invalid ipathhdr ", len);
	if (err & INFINIPATH_RHF_H_IBERR)
		strlcat(msg, "iberr ", len);
	if (err & INFINIPATH_RHF_L_SWA)
		strlcat(msg, "swA ", len);
	if (err & INFINIPATH_RHF_L_SWB)
		strlcat(msg, "swB ", len);
}

/**
 * ipath_get_egrbuf - get an eager buffer
 * @dd: the infinipath device
 * @bufnum: the eager buffer to get
 *
 * must only be called if ipath_pd[port] is known to be allocated
 */
static inline void *ipath_get_egrbuf(struct ipath_devdata *dd, u32 bufnum)
{
	return dd->ipath_port0_skbinfo ?
		(void *) dd->ipath_port0_skbinfo[bufnum].skb->data : NULL;
}

/**
 * ipath_alloc_skb - allocate an skb and buffer with possible constraints
 * @dd: the infinipath device
 * @gfp_mask: the sk_buff SFP mask
 */
struct sk_buff *ipath_alloc_skb(struct ipath_devdata *dd,
				gfp_t gfp_mask)
{
	struct sk_buff *skb;
	u32 len;

	/*
	 * Only fully supported way to handle this is to allocate lots
	 * extra, align as needed, and then do skb_reserve().  That wastes
	 * a lot of memory...  I'll have to hack this into infinipath_copy
	 * also.
	 */

	/*
	 * We need 2 extra bytes for ipath_ether data sent in the
	 * key header.  In order to keep everything dword aligned,
	 * we'll reserve 4 bytes.
	 */
	len = dd->ipath_ibmaxlen + 4;

	if (dd->ipath_flags & IPATH_4BYTE_TID) {
		/* We need a 2KB multiple alignment, and there is no way
		 * to do it except to allocate extra and then skb_reserve
		 * enough to bring it up to the right alignment.
		 */
		len += 2047;
	}

	skb = __dev_alloc_skb(len, gfp_mask);
	if (!skb) {
		ipath_dev_err(dd, "Failed to allocate skbuff, length %u\n",
			      len);
		goto bail;
	}

	skb_reserve(skb, 4);

	if (dd->ipath_flags & IPATH_4BYTE_TID) {
		u32 una = (unsigned long)skb->data & 2047;
		if (una)
			skb_reserve(skb, 2048 - una);
	}

bail:
	return skb;
}

static void ipath_rcv_hdrerr(struct ipath_devdata *dd,
			     u32 eflags,
			     u32 l,
			     u32 etail,
			     __le32 *rhf_addr,
			     struct ipath_message_header *hdr)
{
	char emsg[128];

	get_rhf_errstring(eflags, emsg, sizeof emsg);
	ipath_cdbg(PKT, "RHFerrs %x hdrqtail=%x typ=%u "
		   "tlen=%x opcode=%x egridx=%x: %s\n",
		   eflags, l,
		   ipath_hdrget_rcv_type(rhf_addr),
		   ipath_hdrget_length_in_bytes(rhf_addr),
		   be32_to_cpu(hdr->bth[0]) >> 24,
		   etail, emsg);

	/* Count local link integrity errors. */
	if (eflags & (INFINIPATH_RHF_H_ICRCERR | INFINIPATH_RHF_H_VCRCERR)) {
		u8 n = (dd->ipath_ibcctrl >>
			INFINIPATH_IBCC_PHYERRTHRESHOLD_SHIFT) &
			INFINIPATH_IBCC_PHYERRTHRESHOLD_MASK;

		if (++dd->ipath_lli_counter > n) {
			dd->ipath_lli_counter = 0;
			dd->ipath_lli_errors++;
		}
	}
}

/*
 * ipath_kreceive - receive a packet
 * @pd: the infinipath port
 *
 * called from interrupt handler for errors or receive interrupt
 */
void ipath_kreceive(struct ipath_portdata *pd)
{
	struct ipath_devdata *dd = pd->port_dd;
	__le32 *rhf_addr;
	void *ebuf;
	const u32 rsize = dd->ipath_rcvhdrentsize;	/* words */
	const u32 maxcnt = dd->ipath_rcvhdrcnt * rsize;	/* words */
	u32 etail = -1, l, hdrqtail;
	struct ipath_message_header *hdr;
	u32 eflags, i, etype, tlen, pkttot = 0, updegr = 0, reloop = 0;
	static u64 totcalls;	/* stats, may eventually remove */
	int last;

	l = pd->port_head;
	rhf_addr = (__le32 *) pd->port_rcvhdrq + l + dd->ipath_rhf_offset;
	if (dd->ipath_flags & IPATH_NODMA_RTAIL) {
		u32 seq = ipath_hdrget_seq(rhf_addr);

		if (seq != pd->port_seq_cnt)
			goto bail;
		hdrqtail = 0;
	} else {
		hdrqtail = ipath_get_rcvhdrtail(pd);
		if (l == hdrqtail)
			goto bail;
		smp_rmb();
	}

reloop:
	for (last = 0, i = 1; !last; i += !last) {
		hdr = dd->ipath_f_get_msgheader(dd, rhf_addr);
		eflags = ipath_hdrget_err_flags(rhf_addr);
		etype = ipath_hdrget_rcv_type(rhf_addr);
		/* total length */
		tlen = ipath_hdrget_length_in_bytes(rhf_addr);
		ebuf = NULL;
		if ((dd->ipath_flags & IPATH_NODMA_RTAIL) ?
		    ipath_hdrget_use_egr_buf(rhf_addr) :
		    (etype != RCVHQ_RCV_TYPE_EXPECTED)) {
			/*
			 * It turns out that the chip uses an eager buffer
			 * for all non-expected packets, whether it "needs"
			 * one or not.  So always get the index, but don't
			 * set ebuf (so we try to copy data) unless the
			 * length requires it.
			 */
			etail = ipath_hdrget_index(rhf_addr);
			updegr = 1;
			if (tlen > sizeof(*hdr) ||
			    etype == RCVHQ_RCV_TYPE_NON_KD)
				ebuf = ipath_get_egrbuf(dd, etail);
		}

		/*
		 * both tiderr and ipathhdrerr are set for all plain IB
		 * packets; only ipathhdrerr should be set.
		 */

		if (etype != RCVHQ_RCV_TYPE_NON_KD &&
		    etype != RCVHQ_RCV_TYPE_ERROR &&
		    ipath_hdrget_ipath_ver(hdr->iph.ver_port_tid_offset) !=
		    IPS_PROTO_VERSION)
			ipath_cdbg(PKT, "Bad InfiniPath protocol version "
				   "%x\n", etype);

		if (unlikely(eflags))
			ipath_rcv_hdrerr(dd, eflags, l, etail, rhf_addr, hdr);
		else if (etype == RCVHQ_RCV_TYPE_NON_KD) {
			ipath_ib_rcv(dd->verbs_dev, (u32 *)hdr, ebuf, tlen);
			if (dd->ipath_lli_counter)
				dd->ipath_lli_counter--;
		} else if (etype == RCVHQ_RCV_TYPE_EAGER) {
			u8 opcode = be32_to_cpu(hdr->bth[0]) >> 24;
			u32 qp = be32_to_cpu(hdr->bth[1]) & 0xffffff;
			ipath_cdbg(PKT, "typ %x, opcode %x (eager, "
				   "qp=%x), len %x; ignored\n",
				   etype, opcode, qp, tlen);
		}
		else if (etype == RCVHQ_RCV_TYPE_EXPECTED)
			ipath_dbg("Bug: Expected TID, opcode %x; ignored\n",
				  be32_to_cpu(hdr->bth[0]) >> 24);
		else {
			/*
			 * error packet, type of error unknown.
			 * Probably type 3, but we don't know, so don't
			 * even try to print the opcode, etc.
			 * Usually caused by a "bad packet", that has no
			 * BTH, when the LRH says it should.
			 */
			ipath_cdbg(ERRPKT, "Error Pkt, but no eflags! egrbuf"
				  " %x, len %x hdrq+%x rhf: %Lx\n",
				  etail, tlen, l, (unsigned long long)
				  le64_to_cpu(*(__le64 *) rhf_addr));
			if (ipath_debug & __IPATH_ERRPKTDBG) {
				u32 j, *d, dw = rsize-2;
				if (rsize > (tlen>>2))
					dw = tlen>>2;
				d = (u32 *)hdr;
				printk(KERN_DEBUG "EPkt rcvhdr(%x dw):\n",
					dw);
				for (j = 0; j < dw; j++)
					printk(KERN_DEBUG "%8x%s", d[j],
						(j%8) == 7 ? "\n" : " ");
				printk(KERN_DEBUG ".\n");
			}
		}
		l += rsize;
		if (l >= maxcnt)
			l = 0;
		rhf_addr = (__le32 *) pd->port_rcvhdrq +
			l + dd->ipath_rhf_offset;
		if (dd->ipath_flags & IPATH_NODMA_RTAIL) {
			u32 seq = ipath_hdrget_seq(rhf_addr);

			if (++pd->port_seq_cnt > 13)
				pd->port_seq_cnt = 1;
			if (seq != pd->port_seq_cnt)
				last = 1;
		} else if (l == hdrqtail)
			last = 1;
		/*
		 * update head regs on last packet, and every 16 packets.
		 * Reduce bus traffic, while still trying to prevent
		 * rcvhdrq overflows, for when the queue is nearly full
		 */
		if (last || !(i & 0xf)) {
			u64 lval = l;

			/* request IBA6120 and 7220 interrupt only on last */
			if (last)
				lval |= dd->ipath_rhdrhead_intr_off;
			ipath_write_ureg(dd, ur_rcvhdrhead, lval,
				pd->port_port);
			if (updegr) {
				ipath_write_ureg(dd, ur_rcvegrindexhead,
						 etail, pd->port_port);
				updegr = 0;
			}
		}
	}

	if (!dd->ipath_rhdrhead_intr_off && !reloop &&
	    !(dd->ipath_flags & IPATH_NODMA_RTAIL)) {
		/* IBA6110 workaround; we can have a race clearing chip
		 * interrupt with another interrupt about to be delivered,
		 * and can clear it before it is delivered on the GPIO
		 * workaround.  By doing the extra check here for the
		 * in-memory tail register updating while we were doing
		 * earlier packets, we "almost" guarantee we have covered
		 * that case.
		 */
		u32 hqtail = ipath_get_rcvhdrtail(pd);
		if (hqtail != hdrqtail) {
			hdrqtail = hqtail;
			reloop = 1; /* loop 1 extra time at most */
			goto reloop;
		}
	}

	pkttot += i;

	pd->port_head = l;

	if (pkttot > ipath_stats.sps_maxpkts_call)
		ipath_stats.sps_maxpkts_call = pkttot;
	ipath_stats.sps_port0pkts += pkttot;
	ipath_stats.sps_avgpkts_call =
		ipath_stats.sps_port0pkts / ++totcalls;

bail:;
}

/**
 * ipath_update_pio_bufs - update shadow copy of the PIO availability map
 * @dd: the infinipath device
 *
 * called whenever our local copy indicates we have run out of send buffers
 * NOTE: This can be called from interrupt context by some code
 * and from non-interrupt context by ipath_getpiobuf().
 */

static void ipath_update_pio_bufs(struct ipath_devdata *dd)
{
	unsigned long flags;
	int i;
	const unsigned piobregs = (unsigned)dd->ipath_pioavregs;

	/* If the generation (check) bits have changed, then we update the
	 * busy bit for the corresponding PIO buffer.  This algorithm will
	 * modify positions to the value they already have in some cases
	 * (i.e., no change), but it's faster than changing only the bits
	 * that have changed.
	 *
	 * We would like to do this atomicly, to avoid spinlocks in the
	 * critical send path, but that's not really possible, given the
	 * type of changes, and that this routine could be called on
	 * multiple cpu's simultaneously, so we lock in this routine only,
	 * to avoid conflicting updates; all we change is the shadow, and
	 * it's a single 64 bit memory location, so by definition the update
	 * is atomic in terms of what other cpu's can see in testing the
	 * bits.  The spin_lock overhead isn't too bad, since it only
	 * happens when all buffers are in use, so only cpu overhead, not
	 * latency or bandwidth is affected.
	 */
	if (!dd->ipath_pioavailregs_dma) {
		ipath_dbg("Update shadow pioavail, but regs_dma NULL!\n");
		return;
	}
	if (ipath_debug & __IPATH_VERBDBG) {
		/* only if packet debug and verbose */
		volatile __le64 *dma = dd->ipath_pioavailregs_dma;
		unsigned long *shadow = dd->ipath_pioavailshadow;

		ipath_cdbg(PKT, "Refill avail, dma0=%llx shad0=%lx, "
			   "d1=%llx s1=%lx, d2=%llx s2=%lx, d3=%llx "
			   "s3=%lx\n",
			   (unsigned long long) le64_to_cpu(dma[0]),
			   shadow[0],
			   (unsigned long long) le64_to_cpu(dma[1]),
			   shadow[1],
			   (unsigned long long) le64_to_cpu(dma[2]),
			   shadow[2],
			   (unsigned long long) le64_to_cpu(dma[3]),
			   shadow[3]);
		if (piobregs > 4)
			ipath_cdbg(
				PKT, "2nd group, dma4=%llx shad4=%lx, "
				"d5=%llx s5=%lx, d6=%llx s6=%lx, "
				"d7=%llx s7=%lx\n",
				(unsigned long long) le64_to_cpu(dma[4]),
				shadow[4],
				(unsigned long long) le64_to_cpu(dma[5]),
				shadow[5],
				(unsigned long long) le64_to_cpu(dma[6]),
				shadow[6],
				(unsigned long long) le64_to_cpu(dma[7]),
				shadow[7]);
	}
	spin_lock_irqsave(&ipath_pioavail_lock, flags);
	for (i = 0; i < piobregs; i++) {
		u64 pchbusy, pchg, piov, pnew;
		/*
		 * Chip Errata: bug 6641; even and odd qwords>3 are swapped
		 */
		if (i > 3 && (dd->ipath_flags & IPATH_SWAP_PIOBUFS))
			piov = le64_to_cpu(dd->ipath_pioavailregs_dma[i ^ 1]);
		else
			piov = le64_to_cpu(dd->ipath_pioavailregs_dma[i]);
		pchg = dd->ipath_pioavailkernel[i] &
			~(dd->ipath_pioavailshadow[i] ^ piov);
		pchbusy = pchg << INFINIPATH_SENDPIOAVAIL_BUSY_SHIFT;
		if (pchg && (pchbusy & dd->ipath_pioavailshadow[i])) {
			pnew = dd->ipath_pioavailshadow[i] & ~pchbusy;
			pnew |= piov & pchbusy;
			dd->ipath_pioavailshadow[i] = pnew;
		}
	}
	spin_unlock_irqrestore(&ipath_pioavail_lock, flags);
}

/*
 * used to force update of pioavailshadow if we can't get a pio buffer.
 * Needed primarily due to exitting freeze mode after recovering
 * from errors.  Done lazily, because it's safer (known to not
 * be writing pio buffers).
 */
static void ipath_reset_availshadow(struct ipath_devdata *dd)
{
	int i, im;
	unsigned long flags;

	spin_lock_irqsave(&ipath_pioavail_lock, flags);
	for (i = 0; i < dd->ipath_pioavregs; i++) {
		u64 val, oldval;
		/* deal with 6110 chip bug on high register #s */
		im = (i > 3 && (dd->ipath_flags & IPATH_SWAP_PIOBUFS)) ?
			i ^ 1 : i;
		val = le64_to_cpu(dd->ipath_pioavailregs_dma[im]);
		/*
		 * busy out the buffers not in the kernel avail list,
		 * without changing the generation bits.
		 */
		oldval = dd->ipath_pioavailshadow[i];
		dd->ipath_pioavailshadow[i] = val |
			((~dd->ipath_pioavailkernel[i] <<
			INFINIPATH_SENDPIOAVAIL_BUSY_SHIFT) &
			0xaaaaaaaaaaaaaaaaULL); /* All BUSY bits in qword */
		if (oldval != dd->ipath_pioavailshadow[i])
			ipath_dbg("shadow[%d] was %Lx, now %lx\n",
				i, (unsigned long long) oldval,
				dd->ipath_pioavailshadow[i]);
	}
	spin_unlock_irqrestore(&ipath_pioavail_lock, flags);
}

/**
 * ipath_setrcvhdrsize - set the receive header size
 * @dd: the infinipath device
 * @rhdrsize: the receive header size
 *
 * called from user init code, and also layered driver init
 */
int ipath_setrcvhdrsize(struct ipath_devdata *dd, unsigned rhdrsize)
{
	int ret = 0;

	if (dd->ipath_flags & IPATH_RCVHDRSZ_SET) {
		if (dd->ipath_rcvhdrsize != rhdrsize) {
			dev_info(&dd->pcidev->dev,
				 "Error: can't set protocol header "
				 "size %u, already %u\n",
				 rhdrsize, dd->ipath_rcvhdrsize);
			ret = -EAGAIN;
		} else
			ipath_cdbg(VERBOSE, "Reuse same protocol header "
				   "size %u\n", dd->ipath_rcvhdrsize);
	} else if (rhdrsize > (dd->ipath_rcvhdrentsize -
			       (sizeof(u64) / sizeof(u32)))) {
		ipath_dbg("Error: can't set protocol header size %u "
			  "(> max %u)\n", rhdrsize,
			  dd->ipath_rcvhdrentsize -
			  (u32) (sizeof(u64) / sizeof(u32)));
		ret = -EOVERFLOW;
	} else {
		dd->ipath_flags |= IPATH_RCVHDRSZ_SET;
		dd->ipath_rcvhdrsize = rhdrsize;
		ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvhdrsize,
				 dd->ipath_rcvhdrsize);
		ipath_cdbg(VERBOSE, "Set protocol header si