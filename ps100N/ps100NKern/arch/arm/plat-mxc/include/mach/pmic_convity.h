 target_control *active_tc = host->active_tc;
	struct scsi_ctrl_blk *tmp, *prev;
	u8 tar;

	if (host->phase != MSG_OUT)
		return int_initio_bad_seq(host);	/* Unexpected phase */

	initio_unlink_pend_scb(host, scb);
	initio_release_scb(host, scb);


	tar = scb->target;	/* target                       */
	active_tc->flags &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE | TCF_BUSY);
	/* clr sync. nego & WDTR flags  07/22/98 */

	/* abort all SCB with same target */
	prev = tmp = host->first_busy;	/* Check Busy queue */
	while (tmp != NULL) {
		if (tmp->target == tar) {
			/* unlink it */
			if (tmp == host->first_busy) {
				if ((host->first_busy = tmp->next) == NULL)
					host->last_busy = NULL;
			} else {
				prev->next = tmp->next;
				if (tmp == host->last_busy)
					host->last_busy = prev;
			}
			tmp->hastat = HOST_ABORTED;
			initio_append_done_scb(host, tmp);
		}
		/* Previous haven't change      */
		else {
			prev = tmp;
		}
		tmp = tmp->next;
	}
	outb(MSG_DEVRST, host->addr + TUL_SFifo);
	outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
	return initio_wait_disc(host);

}

static int initio_msgin_accept(struct initio_host * host)
{
	outb(TSC_MSG_ACCEPT, host->addr + TUL_SCmd);
	return wait_tulip(host);
}

static int wait_tulip(struct initio_host * host)
{

	while (!((host->jsstatus0 = inb(host->addr + TUL_SStatus0))
		 & TSS_INT_PENDING))
			cpu_relax();

	host->jsint = inb(host->addr + TUL_SInt);
	host->phase = host->jsstatus0 & TSS_PH_MASK;
	host->jsstatus1 = inb(host->addr + TUL_SStatus1);

	if (host->jsint & TSS_RESEL_INT)	/* if SCSI bus reset detected */
		return int_initio_resel(host);
	if (host->jsint & TSS_SEL_TIMEOUT)	/* if selected/reselected timeout interrupt */
		return int_initio_busfree(host);
	if (host->jsint & TSS_SCSIRST_INT)	/* if SCSI bus reset detected   */
		return int_initio_scsi_rst(host);

	if (host->jsint & TSS_DISC_INT) {	/* BUS disconnection            */
		if (host->flags & HCF_EXPECT_DONE_DISC) {
			outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0); /* Flush SCSI FIFO */
			initio_unlink_busy_scb(host, host->active);
			host->active->hastat = 0;
			initio_append_done_scb(host, host->active);
			host->active = NULL;
			host->active_tc = NULL;
			host->flags &= ~HCF_EXPECT_DONE_DISC;
			outb(TSC_INITDEFAULT, host->addr + TUL_SConfig);
			outb(TSC_HW_RESELECT, host->addr + TUL_SCtrl1);	/* Enable HW reselect */
			return -1;
		}
		if (host->flags & HCF_EXPECT_DISC) {
			outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0); /* Flush SCSI FIFO */
			host->active = NULL;
			host->active_tc = NULL;
			host->flags &= ~HCF_EXPECT_DISC;
			outb(TSC_INITDEFAULT, host->addr + TUL_SConfig);
			outb(TSC_HW_RESELECT, host->addr + TUL_SCtrl1);	/* Enable HW reselect */
			return -1;
		}
		return int_initio_busfree(host);
	}
	/* The old code really does the below. Can probably be removed */
	if (host->jsint & (TSS_FUNC_COMP | TSS_BUS_SERV))
		return host->phase;
	return host->phase;
}

static int initio_wait_disc(struct initio_host * host)
{
	while (!((host->jsstatus0 = inb(host->addr + TUL_SStatus0)) & TSS_INT_PENDING))
		cpu_relax();

	host->jsint = inb(host->addr + TUL_SInt);

	if (host->jsint & TSS_SCSIRST_INT)	/* if SCSI bus reset detected */
		return int_initio_scsi_rst(host);
	if (host->jsint & TSS_DISC_INT) {	/* BUS disconnection */
		outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0); /* Flush SCSI FIFO */
		outb(TSC_INITDEFAULT, host->addr + TUL_SConfig);
		outb(TSC_HW_RESELECT, host->addr + TUL_SCtrl1);	/* Enable HW reselect */
		host->active = NULL;
		return -1;
	}
	return initio_bad_seq(host);
}

static int initio_wait_done_disc(struct initio_host * host)
{
	while (!((host->jsstatus0 = inb(host->addr + TUL_SStatus0))
		 & TSS_INT_PENDING))
		 cpu_relax();

	host->jsint = inb(host->addr + TUL_SInt);

	if (host->jsint & TSS_SCSIRST_INT)	/* if SCSI bus reset detected */
		return int_initio_scsi_rst(host);
	if (host->jsint & TSS_DISC_INT) {	/* BUS disconnection */
		outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0);		/* Flush SCSI FIFO */
		outb(TSC_INITDEFAULT, host->addr + TUL_SConfig);
		outb(TSC_HW_RESELECT, host->addr + TUL_SCtrl1);		/* Enable HW reselect */
		initio_unlink_busy_scb(host, host->active);

		initio_append_done_scb(host, host->active);
		host->active = NULL;
		return -1;
	}
	return initio_bad_seq(host);
}

/**
 *	i91u_intr		-	IRQ handler
 *	@irqno: IRQ number
 *	@dev_id: IRQ identifier
 *
 *	Take the relevant locks and then invoke the actual isr processing
 *	code under the lock.
 */

static irqreturn_t i91u_intr(int irqno, void *dev_id)
{
	struct Scsi_Host *dev = dev_id;
	unsigned long flags;
	int r;
	
	spin_lock_irqsave(dev->host_lock, flags);
	r = initio_isr((struct initio_host *)dev->hostdata);
	spin_unlock_irqrestore(dev->host_lock, flags);
	if (r)
		return IRQ_HANDLED;
	else
		return IRQ_NONE;
}


/**
 *	initio_build_scb		-	Build the mappings and SCB
 *	@host: InitIO host taking the command
 *	@cblk: Firmware command block
 *	@cmnd: SCSI midlayer command block
 *
 *	Translate the abstract SCSI command into a firmware command block
 *	suitable for feeding to the InitIO host controller. This also requires
 *	we build the scatter gather lists and ensure they are mapped properly.
 */

static void initio_build_scb(struct initio_host * host, struct scsi_ctrl_blk * cblk, struct scsi_cmnd * cmnd)
{				/* Create corresponding SCB     */
	struct scatterlist *sglist;
	struct sg_entry *sg;		/* Pointer to SG list           */
	int i, nseg;
	long total_len;
	dma_addr_t dma_addr;

	/* Fill in the command headers */
	cblk->post = i91uSCBPost;	/* i91u's callback routine      */
	cblk->srb = cmnd;
	cblk->opcode = ExecSCSI;
	cblk->flags = SCF_POST;	/* After SCSI done, call post routine */
	cblk->target = cmnd->device->id;
	cblk->lun = cmnd->device->lun;
	cblk->ident = cmnd->device->lun | DISC_ALLOW;

	cblk->flags |= SCF_SENSE;	/* Turn on auto request sense   */

	/* Map the sense buffer into bus memory */
	dma_addr = dma_map_single(&host->pci_dev->dev, cmnd->sense_buffer,
				  SENSE_SIZE, DMA_FROM_DEVICE);
	cblk->senseptr = (u32)dma_addr;
	cblk->senselen = SENSE_SIZE;
	cmnd->SCp.ptr = (char *)(unsigned long)dma_addr;
	cblk->cdblen = cmnd->cmd_len;

	/* Clear the returned status */
	cblk->hastat = 0;
	cblk->tastat = 0;
	/* Command the command */
	memcpy(cblk->cdb, cmnd->cmnd, cmnd->cmd_len);

	/* Set up tags */
	if (cmnd->device->tagged_supported) {	/* Tag Support                  */
		cblk->tagmsg = SIMPLE_QUEUE_TAG;	/* Do simple tag only   */
	} else {
		cblk->tagmsg = 0;	/* No tag support               */
	}

	/* todo handle map_sg error */
	nseg = scsi_dma_map(cmnd);
	BUG_ON(nseg < 0);
	if (nseg) {
		dma_addr = dma_map_single(&host->pci_dev->dev, &cblk->sglist[0],
					  sizeof(struct sg_entry) * TOTAL_SG_ENTRY,
					  DMA_BIDIRECTIONAL);
		cblk->bufptr = (u32)dma_addr;
		cmnd->SCp.dma_handle = dma_addr;

		cblk->sglen = nseg;

		cblk->flags |= SCF_SG;	/* Turn on SG list flag       */
		total_len = 0;
		sg = &cblk->sglist[0];
		scsi_for_each_sg(cmnd, sglist, cblk->sglen, i) {
			sg->data = cpu_to_le32((u32)sg_dma_address(sglist));
			sg->len = cpu_to_le32((u32)sg_dma_len(sglist));
			total_len += sg_dma_len(sglist);
			++sg;
		}

		cblk->buflen = (scsi_bufflen(cmnd) > total_len) ?
			total_len : scsi_bufflen(cmnd);
	} else {	/* No data transfer required */
		cblk->buflen = 0;
		cblk->sglen = 0;
	}
}

/**
 *	i91u_queuecommand	-	Queue a new command if possible
 *	@cmd: SCSI command block from the mid layer
 *	@done: Completion handler
 *
 *	Attempts to queue a new command with the host adapter. Will return
 *	zero if successful or indicate a host busy condition if not (which
 *	will cause the mid layer to call us again later with the command)
 */

static int i91u_queuecommand(struct scsi_cmnd *cmd,
		void (*done)(struct scsi_cmnd *))
{
	struct initio_host *host = (struct initio_host *) cmd->device->host->hostdata;
	struct scsi_ctrl_blk *cmnd;

	cmd->scsi_done = done;

	cmnd = initio_alloc_scb(host);
	if (!cmnd)
		return SCSI_MLQUEUE_HOST_BUSY;

	initio_build_scb(host, cmnd, cmd);
	initio_exec_scb(host, cmnd);
	return 0;
}

/**
 *	i91u_bus_reset		-	reset the SCSI bus
 *	@cmnd: Command block we want to trigger the reset for
 *
 *	Initiate a SCSI bus reset sequence
 */

static int i91u_bus_reset(struct scsi_cmnd * cmnd)
{
	struct initio_host *host;

	host = (struct initio_host *) cmnd->device->host->hostdata;

	spin_lock_irq(cmnd->device->host->host_lock);
	initio_reset_scsi(host, 0);
	spin_unlock_irq(cmnd->device->host->host_lock);

	return SUCCESS;
}

/**
 *	i91u_biospararm			-	return the "logical geometry
 *	@sdev: SCSI device
 *	@dev; Matching block device
 *	@capacity: Sector size of drive
 *	@info_array: Return space for BIOS geometry
 *
 *	Map the device geometry in a manner compatible with the host
 *	controller BIOS behaviour.
 *
 *	FIXME: limited to 2^32 sector devices.
 */

static int i91u_biosparam(struct scsi_device *sdev, struct block_device *dev,
		sector_t capacity, int *info_array)
{
	struct initio_host *host;		/* Point to Host adapter control block */
	struct target_control *tc;

	host = (struct initio_host *) sdev->host->hostdata;
	tc = &host->targets[sdev->id];

	if (tc->heads) {
		info_array[0] = tc->heads;
		info_array[1] = tc->sectors;
		info_array[2] = (unsigned long)capacity / tc->heads / tc->sectors;
	} else {
		if (tc->drv_flags & TCF_DRV_255_63) {
			info_array[0] = 255;
			info_array[1] = 63;
			info_array[2] = (unsigned long)capacity / 255 / 63;
		} else {
			info_array[0] = 64;
			info_array[1] = 32;
			info_array[2] = (unsigned long)capacity >> 11;
		}
	}

#if defined(DEBUG_BIOSPARAM)
	if (i91u_debug & debug_biosparam) {
		printk("bios geometry: head=%d, sec=%d, cyl=%d\n",
		       info_array[0], info_array[1], info_array[2]);
		printk("WARNING: check, if the bios geometry is correct.\n");
	}
#endif

	return 0;
}

/**
 *	i91u_unmap_scb		-	Unmap a command
 *	@pci_dev: PCI device the command is for
 *	@cmnd: The command itself
 *
 *	Unmap any PCI mapping/IOMMU resources allocated when the command
 *	was mapped originally as part of initio_build_scb
 */

static void i91u_unmap_scb(struct pci_dev *pci_dev, struct scsi_cmnd *cmnd)
{
	/* auto sense buffer */
	if (cmnd->SCp.ptr) {
		dma_unmap_single(&pci_dev->dev,
				 (dma_addr_t)((unsigned long)cmnd->SCp.ptr),
				 SENSE_SIZE, DMA_FROM_DEVICE);
		cmnd->SCp.ptr = NULL;
	}

	/* request buffer */
	if (scsi_sg_count(cmnd)) {
		dma_unmap_single(&pci_dev->dev, cmnd->SCp.dma_handle,
				 sizeof(struct sg_entry) * TOTAL_SG_ENTRY,
				 DMA_BIDIRECTIONAL);

		scsi_dma_unmap(cmnd);
	}
}

/**
 *	i91uSCBPost		-	SCSI callback
 *	@host: Pointer to host adapter control block.
 *	@cmnd: Pointer to SCSI control block.
 *
 *	This is callback routine be called when tulip finish one
 *	SCSI command.
 */

static void i91uSCBPost(u8 * host_mem, u8 * cblk_mem)
{
	struct scsi_cmnd *cmnd;	/* Pointer to SCSI request block */
	struct initio_host *host;
	struct scsi_ctrl_blk *cblk;

	host = (struct initio_host *) host_mem;
	cblk = (struct scsi_ctrl_blk *) cblk_mem;
	if ((cmnd = cblk->srb) == NULL) {
		printk(KERN_ERR "i91uSCBPost: SRB pointer is empty\n");
		WARN_ON(1);
		initio_release_scb(host, cblk);	/* Release SCB for current channel */
		return;
	}

	/*
	 *	Remap the firmware error status into a mid layer one
	 */
	switch (cblk->hastat) {
	case 0x0:
	case 0xa:		/* Linked command complete without error and linked normally */
	case 0xb:		/* Linked command complete without error interrupt generated */
		cblk->hastat = 0;
		break;

	case 0x11:		/* Selection time out-The initiator selection or target
				   reselection was not complete within the SCSI Time out period */
		cblk->hastat = DID_TIME_OUT;
		break;

	case 0x14:		/* Target bus phase sequence failure-An invalid bus phase or bus
				   phase sequence was requested by the target. The host adapter
				   will generate a SCSI Reset Condition, notifying the host with
				   a SCRD interrupt */
		cblk->hastat = DID_RESET;
		break;

	case 0x1a:		/* SCB Aborted. 07/21/98 */
		cblk->hastat = DID_ABORT;
		break;

	case 0x12:		/* Data overrun/underrun-The target attempted to transfer more data
				   than was allocated by the Data Length field or the sum of the
				   Scatter / Gather Data Length fields. */
	case 0x13:		/* Unexpected bus free-The target dropped the SCSI BSY at an unexpected time. */
	case 0x16:		/* Invalid SCB Operation Code. */

	default:
		printk("ini9100u: %x %x\n", cblk->hastat, cblk->tastat);
		cblk->hastat = DID_ERROR;	/* Couldn't find any better */
		break;
	}

	cmnd->result = cblk->tastat | (cblk->hastat << 16);
	WARN_ON(cmnd == NULL);
	i91u_unmap_scb(host->pci_dev, cmnd);
	cmnd->scsi_done(cmnd);	/* Notify system DONE           */
	initio_release_scb(host, cblk);	/* Release SCB for current channel */
}

static struct scsi_host_template initio_template = {
	.proc_name		= "INI9100U",
	.name			= "Initio INI-9X00U/UW SCSI device driver",
	.queuecommand		= i91u_queuecommand,
	.eh_bus_reset_handler	= i91u_bus_reset,
	.bios_param		= i91u_biosparam,
	.can_queue		= MAX_TARGETS * i91u_MAXQUEUE,
	.this_id		= 1,
	.sg_tablesize		= SG_ALL,
	.cmd_per_lun		= 1,
	.use_clustering		= ENABLE_CLUSTERING,
};

static int initio_probe_one(struct pci_dev *pdev,
	const struct pci_device_id *id)
{
	struct Scsi_Host *shost;
	struct initio_host *host;
	u32 reg;
	u16 bios_seg;
	struct scsi_ctrl_blk *scb, *tmp, *prev = NULL /* silence gcc */;
	int num_scb, i, error;

	error = pci_enable_device(pdev);
	if (error)
		return error;

	pci_read_config_dword(pdev, 0x44, (u32 *) & reg);
	bios_seg = (u16) (reg & 0xFF);
	if (((reg & 0xFF00) >> 8) == 0xFF)
		reg = 0;
	bios_seg = (bios_seg << 8) + ((u16) ((reg & 0xFF00) >> 8));

	if (pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		printk(KERN_WARNING  "i91u: Could not set 32 bit DMA mask\n");
		error = -ENODEV;
		goto out_disable_device;
	}
	shost = scsi_host_alloc(&initio_template, sizeof(struct initio_host));
	if (!shost) {
		printk(KERN_WARNING "initio: Could not allocate host structure.\n");
		error = -ENOMEM;
		goto out_disable_device;
	}
	host = (struct initio_host *)shost->hostdata;
	memset(host, 0, sizeof(struct initio_host));
	host->addr = pci_resource_start(pdev, 0);
	host->bios_addr = bios_seg;

	if (!request_region(host->addr, 256, "i91u")) {
		printk(KERN_WARNING "initio: I/O port range 0x%x is busy.\n", host->addr);
		error = -ENODEV;
		goto out_host_put;
	}

	if (initio_tag_enable)	/* 1.01i */
		num_scb = MAX_TARGETS * i91u_MAXQUEUE;
	else
		num_scb = MAX_TARGETS + 3;	/* 1-tape, 1-CD_ROM, 1- extra */

	for (; num_scb >= MAX_TARGETS + 3; num_scb--) {
		i = num_scb * sizeof(struct scsi_ctrl_blk);
		if ((scb = kzalloc(i, GFP_DMA)) != NULL)
			break;
	}

	if (!scb) {
		printk(KERN_WARNING "initio: Cannot allocate SCB array.\n");
		error = -ENOMEM;
		goto out_release_region;
	}

	host->pci_dev = pdev;

	host->semaph = 1;
	spin_lock_init(&host->semaph_lock);
	host->num_scbs = num_scb;
	host->scb = scb;
	host->next_pending = scb;
	host->next_avail = scb;
	for (i = 0, tmp = scb; i < num_scb; i++, tmp++) {
		tmp->tagid = i;
		if (i != 0)
			prev->next = tmp;
		prev = tmp;
	}
	prev->next = NULL;
	host->scb_end = tmp;
	host->first_avail = scb;
	host->last_avail = prev;
	spin_lock_init(&host->avail_lock);

	initio_init(host, phys_to_virt(((u32)bios_seg << 4)));

	host->jsstatus0 = 0;

	shost->io_port = host->addr;
	shost->n_io_port = 0xff;
	shost->can_queue = num_scb;		/* 03/05/98                      */
	shost->unique_id = host->addr;
	shost->max_id = host->max_tar;
	shost->max_lun = 32;	/* 10/21/97                     */
	shost->irq = pdev->irq;
	shost->this_id = host->scsi_id;	/* Assign HCS index           */
	shost->base = host->addr;
	shost->sg_tablesize = TOTAL_SG_ENTRY;

	error = request_irq(pdev->irq, i91u_intr, IRQF_DISABLED|IRQF_SHARED, "i91u", shost);
	if (error < 0) {
		printk(KERN_WARNING "initio: Unable to request IRQ %d\n", pdev->irq);
		goto out_free_scbs;
	}

	pci_set_drvdata(pdev, shost);

	error = scsi_add_host(shost, &pdev->dev);
	if (error)
		goto out_free_irq;
	scsi_scan_host(shost);
	return 0;
out_free_irq:
	free_irq(pdev->irq, shost);
out_free_scbs:
	kfree(host->scb);
out_release_region:
	release_region(host->addr, 256);
out_host_put:
	scsi_host_put(shost);
out_disable_device:
	pci_disable_device(pdev);
	return error;
}

/**
 *	initio_remove_one	-	control shutdown
 *	@pdev:	PCI device being released
 *
 *	Release the resources assigned to this adapter after it has
 *	finished being used.
 */

static void initio_remove_one(struct pci_dev *pdev)
{
	struct Scsi_Host *host = pci_get_drvdata(pdev);
	struct initio_host *s = (struct initio_host *)host->hostdata;
	scsi_remove_host(host);
	free_irq(pdev->irq, host);
	release_region(s->addr, 256);
	scsi_host_put(host);
	pci_disable_device(pdev);
}

MODULE_LICENSE("GPL");

static struct pci_device_id initio_pci_tbl[] = {
	{PCI_VENDOR_ID_INIT, 0x9500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_INIT, 0x9400, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_INIT, 0x9401, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_INIT, 0x0002, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_DOMEX, 0x0002, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0,}
};
MODULE_DEVICE_TABLE(pci, initio_pci_tbl);

static struct pci_driver initio_pci_driver = {
	.name		= "initio",
	.id_table	= initio_pci_tbl,
	.probe		= initio_probe_one,
	.remove		= __devexit_p(initio_remove_one),
};

static int __init initio_init_driver(void)
{
	return pci_register_driver(&initio_pci_driver);
}

static void __exit initio_exit_driver(void)
{
	pci_unregister_driver(&initio_pci_driver);
}

MODULE_DESCRIPTION("Initio INI-9X00U/UW SCSI device driver");
MODULE_AUTHOR("Initio Corporation");
MODULE_LICENSE("GPL");

module_init(initio_init_driver);
module_exit(initio_exit_driver);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  