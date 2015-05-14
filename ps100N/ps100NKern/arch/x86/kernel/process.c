a1280: Error Entry invalid handle");
	}
#endif

	LEAVE("qla1280_error_entry");
}

/*
 *  qla1280_abort_isp
 *      Resets ISP and aborts all outstanding commands.
 *
 * Input:
 *      ha           = adapter block pointer.
 *
 * Returns:
 *      0 = success
 */
static int
qla1280_abort_isp(struct scsi_qla_host *ha)
{
	struct device_reg __iomem *reg = ha->iobase;
	struct srb *sp;
	int status = 0;
	int cnt;
	int bus;

	ENTER("qla1280_abort_isp");

	if (ha->flags.abort_isp_active || !ha->flags.online)
		goto out;
	
	ha->flags.abort_isp_active = 1;

	/* Disable ISP interrupts. */
	qla1280_disable_intrs(ha);
	WRT_REG_WORD(&reg->host_cmd, HC_PAUSE_RISC);
	RD_REG_WORD(&reg->id_l);

	printk(KERN_INFO "scsi(%li): dequeuing outstanding commands\n",
	       ha->host_no);
	/* Dequeue all commands in outstanding command list. */
	for (cnt = 0; cnt < MAX_OUTSTANDING_COMMANDS; cnt++) {
		struct scsi_cmnd *cmd;
		sp = ha->outstanding_cmds[cnt];
		if (sp) {
			cmd = sp->cmd;
			CMD_RESULT(cmd) = DID_RESET << 16;
			CMD_HANDLE(cmd) = COMPLETED_HANDLE;
			ha->outstanding_cmds[cnt] = NULL;
			list_add_tail(&sp->list, &ha->done_q);
		}
	}

	qla1280_done(ha);

	status = qla1280_load_firmware(ha);
	if (status)
		goto out;

	/* Setup adapter based on NVRAM parameters. */
	qla1280_nvram_config (ha);

	status = qla1280_init_rings(ha);
	if (status)
		goto out;
		
	/* Issue SCSI reset. */
	for (bus = 0; bus < ha->ports; bus++)
		qla1280_bus_reset(ha, bus);
		
	ha->flags.abort_isp_active = 0;
 out:
	if (status) {
		printk(KERN_WARNING
		       "qla1280: ISP error recovery failed, board disabled");
		qla1280_reset_adapter(ha);
		dprintk(2, "qla1280_abort_isp: **** FAILED ****\n");
	}

	LEAVE("qla1280_abort_isp");
	return status;
}


/*
 * qla1280_debounce_register
 *      Debounce register.
 *
 * Input:
 *      port = register address.
 *
 * Returns:
 *      register value.
 */
static u16
qla1280_debounce_register(volatile u16 __iomem * addr)
{
	volatile u16 ret;
	volatile u16 ret2;

	ret = RD_REG_WORD(addr);
	ret2 = RD_REG_WORD(addr);

	if (ret == ret2)
		return ret;

	do {
		cpu_relax();
		ret = RD_REG_WORD(addr);
		ret2 = RD_REG_WORD(addr);
	} while (ret != ret2);

	return ret;
}


/************************************************************************
 * qla1280_check_for_dead_scsi_bus                                      *
 *                                                                      *
 *    This routine checks for a dead SCSI bus                           *
 ************************************************************************/
#define SET_SXP_BANK            0x0100
#define SCSI_PHASE_INVALID      0x87FF
static int
qla1280_check_for_dead_scsi_bus(struct scsi_qla_host *ha, unsigned int bus)
{
	uint16_t config_reg, scsi_control;
	struct device_reg __iomem *reg = ha->iobase;

	if (ha->bus_settings[bus].scsi_bus_dead) {
		WRT_REG_WORD(&reg->host_cmd, HC_PAUSE_RISC);
		config_reg = RD_REG_WORD(&reg->cfg_1);
		WRT_REG_WORD(&reg->cfg_1, SET_SXP_BANK);
		scsi_control = RD_REG_WORD(&reg->scsiControlPins);
		WRT_REG_WORD(&reg->cfg_1, config_reg);
		WRT_REG_WORD(&reg->host_cmd, HC_RELEASE_RISC);

		if (scsi_control == SCSI_PHASE_INVALID) {
			ha->bus_settings[bus].scsi_bus_dead = 1;
			return 1;	/* bus is dead */
		} else {
			ha->bus_settings[bus].scsi_bus_dead = 0;
			ha->bus_settings[bus].failed_reset_count = 0;
		}
	}
	return 0;		/* bus is not dead */
}

static void
qla1280_get_target_parameters(struct scsi_qla_host *ha,
			      struct scsi_device *device)
{
	uint16_t mb[MAILBOX_REGISTER_COUNT];
	int bus, target, lun;

	bus = device->channel;
	target = device->id;
	lun = device->lun;


	mb[0] = MBC_GET_TARGET_PARAMETERS;
	mb[1] = (uint16_t) (bus ? target | BIT_7 : target);
	mb[1] <<= 8;
	qla1280_mailbox_command(ha, BIT_6 | BIT_3 | BIT_2 | BIT_1 | BIT_0,
				&mb[0]);

	printk(KERN_INFO "scsi(%li:%d:%d:%d):", ha->host_no, bus, target, lun);

	if (mb[3] != 0) {
		printk(" Sync: period %d, offset %d",
		       (mb[3] & 0xff), (mb[3] >> 8));
		if (mb[2] & BIT_13)
			printk(", Wide");
		if ((mb[2] & BIT_5) && ((mb[6] >> 8) & 0xff) >= 2)
			printk(", DT");
	} else
		printk(" Async");

	if (device->simple_tags)
		printk(", Tagged queuing: depth %d", device->queue_depth);
	printk("\n");
}


#if DEBUG_QLA1280
static void
__qla1280_dump_buffer(char *b, int size)
{
	int cnt;
	u8 c;

	printk(KERN_DEBUG " 0   1   2   3   4   5   6   7   8   9   Ah  "
	       "Bh  Ch  Dh  Eh  Fh\n");
	printk(KERN_DEBUG "---------------------------------------------"
	       "------------------\n");

	for (cnt = 0; cnt < size;) {
		c = *b++;

		printk("0x%02x", c);
		cnt++;
		if (!(cnt % 16))
			printk("\n");
		else
			printk(" ");
	}
	if (cnt % 16)
		printk("\n");
}

/**************************************************************************
 *   ql1280_print_scsi_cmd
 *
 **************************************************************************/
static void
__qla1280_print_scsi_cmd(struct scsi_cmnd *cmd)
{
	struct scsi_qla_host *ha;
	struct Scsi_Host *host = CMD_HOST(cmd);
	struct srb *sp;
	/* struct scatterlist *sg; */

	int i;
	ha = (struct scsi_qla_host *)host->hostdata;

	sp = (struct srb *)CMD_SP(cmd);
	printk("SCSI Command @= 0x%p, Handle=0x%p\n", cmd, CMD_HANDLE(cmd));
	printk("  chan=%d, target = 0x%02x, lun = 0x%02x, cmd_len = 0x%02x\n",
	       SCSI_BUS_32(cmd), SCSI_TCN_32(cmd), SCSI_LUN_32(cmd),
	       CMD_CDBLEN(cmd));
	printk(" CDB = ");
	for (i = 0; i < cmd->cmd_len; i++) {
		printk("0x%02x ", cmd->cmnd[i]);
	}
	printk("  seg_cnt =%d\n", scsi_sg_count(cmd));
	printk("  request buffer=0x%p, request buffer len=0x%x\n",
	       scsi_sglist(cmd), scsi_bufflen(cmd));
	/* if (cmd->use_sg)
	   {
	   sg = (struct scatterlist *) cmd->request_buffer;
	   printk("  SG buffer: \n");
	   qla1280_dump_buffer(1, (char *)sg, (cmd->use_sg*sizeof(struct scatterlist)));
	   } */
	printk("  tag=%d, transfersize=0x%x \n",
	       cmd->tag, cmd->transfersize);
	printk("  Pid=%li, SP=0x%p\n", cmd->serial_number, CMD_SP(cmd));
	printk(" underflow size = 0x%x, direction=0x%x\n",
	       cmd->underflow, cmd->sc_data_direction);
}

/**************************************************************************
 *   ql1280_dump_device
 *
 **************************************************************************/
static void
ql1280_dump_device(struct scsi_qla_host *ha)
{

	struct scsi_cmnd *cp;
	struct srb *sp;
	int i;

	printk(KERN_DEBUG "Outstanding Commands on controller:\n");

	for (i = 0; i < MAX_OUTSTANDING_COMMANDS; i++) {
		if ((sp = ha->outstanding_cmds[i]) == NULL)
			continue;
		if ((cp = sp->cmd) == NULL)
			continue;
		qla1280_print_scsi_cmd(1, cp);
	}
}
#endif


enum tokens {
	TOKEN_NVRAM,
	TOKEN_SYNC,
	TOKEN_WIDE,
	TOKEN_PPR,
	TOKEN_VERBOSE,
	TOKEN_DEBUG,
};

struct setup_tokens {
	char *token;
	int val;
};

static struct setup_tokens setup_token[] __initdata = 
{
	{ "nvram", TOKEN_NVRAM },
	{ "sync", TOKEN_SYNC },
	{ "wide", TOKEN_WIDE },
	{ "ppr", TOKEN_PPR },
	{ "verbose", TOKEN_VERBOSE },
	{ "debug", TOKEN_DEBUG },
};


/**************************************************************************
 *   qla1280_setup
 *
 *   Handle boot parameters. This really needs to be changed so one
 *   can specify per adapter parameters.
 **************************************************************************/
static int __init
qla1280_setup(char *s)
{
	char *cp, *ptr;
	unsigned long val;
	int toke;

	cp = s;

	while (cp && (ptr = strchr(cp, ':'))) {
		ptr++;
		if (!strcmp(ptr, "yes")) {
			val = 0x10000;
			ptr += 3;
		} else if (!strcmp(ptr, "no")) {
 			val = 0;
			ptr += 2;
		} else
			val = simple_strtoul(ptr, &ptr, 0);

		switch ((toke = qla1280_get_token(cp))) {
		case TOKEN_NVRAM:
			if (!val)
				driver_setup.no_nvram = 1;
			break;
		case TOKEN_SYNC:
			if (!val)
				driver_setup.no_sync = 1;
			else if (val != 0x10000)
				driver_setup.sync_mask = val;
			break;
		case TOKEN_WIDE:
			if (!val)
				driver_setup.no_wide = 1;
			else if (val != 0x10000)
				driver_setup.wide_mask = val;
			break;
		case TOKEN_PPR:
			if (!val)
				driver_setup.no_ppr = 1;
			else if (val != 0x10000)
				driver_setup.ppr_mask = val;
			break;
		case TOKEN_VERBOSE:
			qla1280_verbose = val;
			break;
		default:
			printk(KERN_INFO "qla1280: unknown boot option %s\n",
			       cp);
		}

		cp = strchr(ptr, ';');
		if (cp)
			cp++;
		else {
			break;
		}
	}
	return 1;
}


static int __init
qla1280_get_token(char *str)
{
	char *sep;
	long ret = -1;
	int i;

	sep = strchr(str, ':');

	if (sep) {
		for (i = 0; i < ARRAY_SIZE(setup_token); i++) {
			if (!strncmp(setup_token[i].token, str, (sep - str))) {
				ret =  setup_token[i].val;
				break;
			}
		}
	}

	return ret;
}


static struct scsi_host_template qla1280_driver_template = {
	.module			= THIS_MODULE,
	.proc_name		= "qla1280",
	.name			= "Qlogic ISP 1280/12160",
	.info			= qla1280_info,
	.slave_configure	= qla1280_slave_configure,
	.queuecommand		= qla1280_queuecommand,
	.eh_abort_handler	= qla1280_eh_abort,
	.eh_device_reset_handler= qla1280_eh_device_reset,
	.eh_bus_reset_handler	= qla1280_eh_bus_reset,
	.eh_host_reset_handler	= qla1280_eh_adapter_reset,
	.bios_param		= qla1280_biosparam,
	.can_queue		= 0xfffff,
	.this_id		= -1,
	.sg_tablesize		= SG_ALL,
	.cmd_per_lun		= 1,
	.use_clustering		= ENABLE_CLUSTERING,
};


static int __devinit
qla1280_probe_one(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int devnum = id->driver_data;
	struct qla_boards *bdp = &ql1280_board_tbl[devnum];
	struct Scsi_Host *host;
	struct scsi_qla_host *ha;
	int error = -ENODEV;

	/* Bypass all AMI SUBSYS VENDOR IDs */
	if (pdev->subsystem_vendor == PCI_VENDOR_ID_AMI) {
		printk(KERN_INFO
		       "qla1280: Skipping AMI SubSys Vendor ID Chip\n");
		goto error;
	}

	printk(KERN_INFO "qla1280: %s found on PCI bus %i, dev %i\n",
	       bdp->name, pdev->bus->number, PCI_SLOT(pdev->devfn));
	
	if (pci_enable_device(pdev)) {
		printk(KERN_WARNING
		       "qla1280: Failed to enabled pci device, aborting.\n");
		goto error;
	}

	pci_set_master(pdev);

	error = -ENOMEM;
	host = scsi_host_alloc(&qla1280_driver_template, sizeof(*ha));
	if (!host) {
		printk(KERN_WARNING
		       "qla1280: Failed to register host, aborting.\n");
		goto error_disable_device;
	}

	ha = (struct scsi_qla_host *)host->hostdata;
	memset(ha, 0, sizeof(struct scsi_qla_host));

	ha->pdev = pdev;
	ha->devnum = devnum;	/* specifies microcode load address */

#ifdef QLA_64BIT_PTR
	if (pci_set_dma_mask(ha->pdev, DMA_BIT_MASK(64))) {
		if (pci_set_dma_mask(ha->pdev, DMA_BIT_MASK(32))) {
			printk(KERN_WARNING "scsi(%li): Unable to set a "
			       "suitable DMA mask - aborting\n", ha->host_no);
			error = -ENODEV;
			goto error_put_host;
		}
	} else
		dprintk(2, "scsi(%li): 64 Bit PCI Addressing Enabled\n",
			ha->host_no);
#else
	if (pci_set_dma_mask(ha->pdev, DMA_BIT_MASK(32))) {
		printk(KERN_WARNING "scsi(%li): Unable to set a "
		       "suitable DMA mask - aborting\n", ha->host_no);
		error = -ENODEV;
		goto error_put_host;
	}
#endif

	ha->request_ring = pci_alloc_consistent(ha->pdev,
			((REQUEST_ENTRY_CNT + 1) * sizeof(request_t)),
			&ha->request_dma);
	if (!ha->request_ring) {
		printk(KERN_INFO "qla1280: Failed to get request memory\n");
		goto error_put_host;
	}

	ha->response_ring = pci_alloc_consistent(ha->pdev,
			((RESPONSE_ENTRY_CNT + 1) * sizeof(struct response)),
			&ha->response_dma);
	if (!ha->response_ring) {
		printk(KERN_INFO "qla1280: Failed to get response memory\n");
		goto error_free_request_ring;
	}

	ha->ports = bdp->numPorts;

	ha->host = host;
	ha->host_no = host->host_no;

	host->irq = pdev->irq;
	host->max_channel = bdp->numPorts - 1;
	host->max_lun = MAX_LUNS - 1;
	host->max_id = MAX_TARGETS;
	host->max_sectors = 1024;
	host->unique_id = host->host_no;

	error = -ENODEV;

#if MEMORY_MAPPED_IO
	ha->mmpbase = pci_ioremap_bar(ha->pdev, 1);
	if (!ha->mmpbase) {
		printk(KERN_INFO "qla1280: Unable to map I/O memory\n");
		goto error_free_response_ring;
	}

	host->base = (unsigned long)ha->mmpbase;
	ha->iobase = (struct device_reg __iomem *)ha->mmpbase;
#else
	host->io_port = pci_resource_start(ha->pdev, 0);
	if (!request_region(host->io_port, 0xff, "qla1280")) {
		printk(KERN_INFO "qla1280: Failed to reserve i/o region "
				 "0x%04lx-0x%04lx - already in use\n",
		       host->io_port, host->io_port + 0xff);
		goto error_free_response_ring;
	}

	ha->iobase = (struct device_reg *)host->io_port;
#endif

	INIT_LIST_HEAD(&ha->done_q);

	/* Disable ISP interrupts. */
	qla1280_disable_intrs(ha);

	if (request_irq(pdev->irq, qla1280_intr_handler, IRQF_SHARED,
				"qla1280", ha)) {
		printk("qla1280 : Failed to reserve interrupt %d already "
		       "in use\n", pdev->irq);
		goto error_release_region;
	}

	/* load the F/W, read paramaters, and init the H/W */
	if (qla1280_initialize_adapter(ha)) {
		printk(KERN_INFO "qla1x160: Failed to initialize adapter\n");
		goto error_free_irq;
	}

	/* set our host ID  (need to do something about our two IDs) */
	host->this_id = ha->bus_settings[0].id;

	pci_set_drvdata(pdev, host);

	error = scsi_add_host(host, &pdev->dev);
	if (error)
		goto error_disable_adapter;
	scsi_scan_host(host);

	return 0;

 error_disable_adapter:
	qla1280_disable_intrs(ha);
 error_free_irq:
	free_irq(pdev->irq, ha);
 error_release_region:
#if MEMORY_MAPPED_IO
	iounmap(ha->mmpbase);
#else
	release_region(host->io_port, 0xff);
#endif
 error_free_response_ring:
	pci_free_consistent(ha->pdev,
			((RESPONSE_ENTRY_CNT + 1) * sizeof(struct response)),
			ha->response_ring, ha->response_dma);
 error_free_request_ring:
	pci_free_consistent(ha->pdev,
			((REQUEST_ENTRY_CNT + 1) * sizeof(request_t)),
			ha->request_ring, ha->request_dma);
 error_put_host:
	scsi_host_put(host);
 error_disable_device:
	pci_disable_device(pdev);
 error:
	return error;
}


static void __devexit
qla1280_remove_one(struct pci_dev *pdev)
{
	struct Scsi_Host *host = pci_get_drvdata(pdev);
	struct scsi_qla_host *ha = (struct scsi_qla_host *)host->hostdata;

	scsi_remove_host(host);

	qla1280_disable_intrs(ha);

	free_irq(pdev->irq, ha);

#if MEMORY_MAPPED_IO
	iounmap(ha->mmpbase);
#else
	release_region(host->io_port, 0xff);
#endif

	pci_free_consistent(ha->pdev,
			((REQUEST_ENTRY_CNT + 1) * (sizeof(request_t))),
			ha->request_ring, ha->request_dma);
	pci_free_consistent(ha->pdev,
			((RESPONSE_ENTRY_CNT + 1) * (sizeof(struct response))),
			ha->response_ring, ha->response_dma);

	pci_disable_device(pdev);

	scsi_host_put(host);
}

static struct pci_driver qla1280_pci_driver = {
	.name		= "qla1280",
	.id_table	= qla1280_pci_tbl,
	.probe		= qla1280_probe_one,
	.remo