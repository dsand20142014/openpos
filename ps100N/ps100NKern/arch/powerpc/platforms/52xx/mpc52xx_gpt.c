_CMD_REG(shpnt));
	imm_command &= (unsigned long) (0xffff0000);	/* mask reserved stuff */
	imm_command |= (unsigned long) (IM_RESET_IMM_CMD);
	/* must wait for attention reg not busy */
	while (1) {
		if (!(inb(IM_STAT_REG(shpnt)) & IM_BUSY))
			break;
		spin_unlock_irq(shpnt->host_lock);
		yield();
		spin_lock_irq(shpnt->host_lock);
	}
	/*write registers and enable system interrupts */
	outl(imm_command, IM_CMD_REG(shpnt));
	outb(IM_IMM_CMD | 0xf, IM_ATTN_REG(shpnt));
	/* wait for interrupt finished or intr_stat register to be set, as the
	 * interrupt will not be executed, while we are in here! */
	 
	/* FIXME: This is really really icky we so want a sleeping version of this ! */
	while (reset_status(shpnt) == IM_RESET_IN_PROGRESS && --ticks && ((inb(IM_INTR_REG(shpnt)) & 0x8f) != 0x8f)) {
		udelay((1 + 999 / HZ) * 1000);
		barrier();
	}
	/* if reset did not complete, just return an error */
	if (!ticks) {
		printk(KERN_ERR "IBM MCA SCSI: reset did not complete within %d seconds.\n", IM_RESET_DELAY);
		reset_status(shpnt) = IM_RESET_FINISHED_FAIL;
		return FAILED;
	}

	if ((inb(IM_INTR_REG(shpnt)) & 0x8f) == 0x8f) {
		/* analysis done by this routine and not by the intr-routine */
		if (inb(IM_INTR_REG(shpnt)) == 0xaf)
			reset_status(shpnt) = IM_RESET_FINISHED_OK_NO_INT;
		else if (inb(IM_INTR_REG(shpnt)) == 0xcf)
			reset_status(shpnt) = IM_RESET_FINISHED_FAIL;
		else		/* failed, 4get it */
			reset_status(shpnt) = IM_RESET_NOT_IN_PROGRESS_NO_INT;
		outb(IM_EOI | 0xf, IM_ATTN_REG(shpnt));
	}

	/* if reset failed, just return an error */
	if (reset_status(shpnt) == IM_RESET_FINISHED_FAIL) {
		printk(KERN_ERR "IBM MCA SCSI: reset failed.\n");
		return FAILED;
	}

	/* so reset finished ok - call outstanding done's, and return success */
	printk(KERN_INFO "IBM MCA SCSI: Reset successfully completed.\n");
	for (i = 0; i < MAX_LOG_DEV; i++) {
		cmd_aid = ld(shpnt)[i].cmd;
		if (cmd_aid && cmd_aid->scsi_done) {
			ld(shpnt)[i].cmd = NULL;
			cmd_aid->result = DID_RESET << 16;
		}
	}
	return SUCCESS;
}

static int ibmmca_host_reset(Scsi_Cmnd * cmd)
{
	struct Scsi_Host *shpnt = cmd->device->host;
	int rc;

	spin_lock_irq(shpnt->host_lock);
	rc = __ibmmca_host_reset(cmd);
	spin_unlock_irq(shpnt->host_lock);

	return rc;
}

static int ibmmca_biosparam(struct scsi_device *sdev, struct block_device *bdev, sector_t capacity, int *info)
{
	int size = capacity;
	info[0] = 64;
	info[1] = 32;
	info[2] = size / (info[0] * info[1]);
	if (info[2] >= 1024) {
		info[0] = 128;
		info[1] = 63;
		info[2] = size / (info[0] * info[1]);
		if (info[2] >= 1024) {
			info[0] = 255;
			info[1] = 63;
			info[2] = size / (info[0] * info[1]);
			if (info[2] >= 1024)
				info[2] = 1023;
		}
	}
	return 0;
}

/* calculate percentage of total accesses on a ldn */
static int ldn_access_load(struct Scsi_Host *shpnt, int ldn)
{
	if (IBM_DS(shpnt).total_accesses == 0)
		return (0);
	if (IBM_DS(shpnt).ldn_access[ldn] == 0)
		return (0);
	return (IBM_DS(shpnt).ldn_access[ldn] * 100) / IBM_DS(shpnt).total_accesses;
}

/* calculate total amount of r/w-accesses */
static int ldn_access_total_read_write(struct Scsi_Host *shpnt)
{
	int a;
	int i;

	a = 0;
	for (i = 0; i <= MAX_LOG_DEV; i++)
		a += IBM_DS(shpnt).ldn_read_access[i] + IBM_DS(shpnt).ldn_write_access[i];
	return (a);
}

static int ldn_access_total_inquiry(struct Scsi_Host *shpnt)
{
	int a;
	int i;

	a = 0;
	for (i = 0; i <= MAX_LOG_DEV; i++)
		a += IBM_DS(shpnt).ldn_inquiry_access[i];
	return (a);
}

static int ldn_access_total_modeselect(struct Scsi_Host *shpnt)
{
	int a;
	int i;

	a = 0;
	for (i = 0; i <= MAX_LOG_DEV; i++)
		a += IBM_DS(shpnt).ldn_modeselect_access[i];
	return (a);
}

/* routine to display info in the proc-fs-structure (a deluxe feature) */
static int ibmmca_proc_info(struct Scsi_Host *shpnt, char *buffer, char **start, off_t offset, int length, int inout)
{
	int len = 0;
	int i, id, lun;
	unsigned long flags;
	int max_pun;

	
	spin_lock_irqsave(shpnt->host_lock, flags);	/* Check it */

	max_pun = subsystem_maxid(shpnt);

	len += sprintf(buffer + len, "\n             IBM-SCSI-Subsystem-Linux-Driver, Version %s\n\n\n", IBMMCA_SCSI_DRIVER_VERSION);
	len += sprintf(buffer + len, " SCSI Access-Statistics:\n");
	len += sprintf(buffer + len, "               Device Scanning Order....: %s\n", (ibm_ansi_order) ? "IBM/ANSI" : "New Industry Standard");
#ifdef CONFIG_SCSI_MULTI_LUN
	len += sprintf(buffer + len, "               Multiple LUN probing.....: Yes\n");
#else
	len += sprintf(buffer + len, "               Multiple LUN probing.....: No\n");
#endif
	len += sprintf(buffer + len, "               This Hostnumber..........: %d\n", shpnt->host_no);
	len += sprintf(buffer + len, "               Base I/O-Port............: 0x%x\n", (unsigned int) (IM_CMD_REG(shpnt)));
	len += sprintf(buffer + len, "               (Shared) IRQ.............: %d\n", IM_IRQ);
	len += sprintf(buffer + len, "               Total Interrupts.........: %d\n", IBM_DS(shpnt).total_interrupts);
	len += sprintf(buffer + len, "               Total SCSI Accesses......: %d\n", IBM_DS(shpnt).total_accesses);
	len += sprintf(buffer + len, "               Total short SCBs.........: %d\n", IBM_DS(shpnt).scbs);
	len += sprintf(buffer + len, "               Total long SCBs..........: %d\n", IBM_DS(shpnt).long_scbs);
	len += sprintf(buffer + len, "                 Total SCSI READ/WRITE..: %d\n", ldn_access_total_read_write(shpnt));
	len += sprintf(buffer + len, "                 Total SCSI Inquiries...: %d\n", ldn_access_total_inquiry(shpnt));
	len += sprintf(buffer + len, "                 Total SCSI Modeselects.: %d\n", ldn_access_total_modeselect(shpnt));
	len += sprintf(buffer + len, "                 Total SCSI other cmds..: %d\n", IBM_DS(shpnt).total_accesses - ldn_access_total_read_write(shpnt)
		       - ldn_access_total_modeselect(shpnt)
		       - ldn_access_total_inquiry(shpnt));
	len += sprintf(buffer + len, "               Total SCSI command fails.: %d\n\n", IBM_DS(shpnt).total_errors);
	len += sprintf(buffer + len, " Logical-Device-Number (LDN) Access-Statistics:\n");
	len += sprintf(buffer + len, "         LDN | Accesses [%%] |   READ    |   WRITE   | ASSIGNMENTS\n");
	len += sprintf(buffer + len, "        -----|--------------|-----------|-----------|--------------\n");
	for (i = 0; i <= MAX_LOG_DEV; i++)
		len += sprintf(buffer + len, "         %2X  |    %3d       |  %8d |  %8d | %8d\n", i, ldn_access_load(shpnt, i), IBM_DS(shpnt).ldn_read_access[i], IBM_DS(shpnt).ldn_write_access[i], IBM_DS(shpnt).ldn_assignments[i]);
	len += sprintf(buffer + len, "        -----------------------------------------------------------\n\n");
	len += sprintf(buffer + len, " Dynamical-LDN-Assignment-Statistics:\n");
	len += sprintf(buffer + len, "               Number of physical SCSI-devices..: %d (+ Adapter)\n", IBM_DS(shpnt).total_scsi_devices);
	len += sprintf(buffer + len, "               Dynamical Assignment necessary...: %s\n", IBM_DS(shpnt).dyn_flag ? "Yes" : "No ");
	len += sprintf(buffer + len, "               Next LDN to be assigned..........: 0x%x\n", next_ldn(shpnt));
	len += sprintf(buffer + len, "               Dynamical assignments done yet...: %d\n", IBM_DS(shpnt).dynamical_assignments);
	len += sprintf(buffer + len, "\n Current SCSI-Device-Mapping:\n");
	len += sprintf(buffer + len, "        Physical SCSI-Device Map               Logical SCSI-Device Map\n");
	len += sprintf(buffer + len, "    ID\\LUN  0  1  2  3  4  5  6  7       ID\\LUN  0  1  2  3  4  5  6  7\n");
	for (id = 0; id < max_pun; id++) {
		len += sprintf(buffer + len, "    %2d     ", id);
		for (lun = 0; lun < 8; lun++)
			len += sprintf(buffer + len, "%2s ", ti_p(get_scsi(shpnt)[id][lun]));
		len += sprintf(buffer + len, "      %2d     ", id);
		for (lun = 0; lun < 8; lun++)
			len += sprintf(buffer + len, "%2s ", ti_l(get_ldn(shpnt)[id][lun]));
		len += sprintf(buffer + len, "\n");
	}

	len += sprintf(buffer + len, "(A = IBM-Subsystem, D = Harddisk, T = Tapedrive, P = Processor, W = WORM,\n");
	len += sprintf(buffer + len, " R = CD-ROM, S = Scanner, M = MO-Drive, C = Medium-Changer, + = unprovided LUN,\n");
	len += sprintf(buffer + len, " - = nothing found, nothing assigned or unprobed LUN)\n\n");

	*start = buffer + offset;
	len -= offset;
	if (len > length)
		len = length;
	spin_unlock_irqrestore(shpnt->host_lock, flags);
	return len;
}

static int option_setup(char *str)
{
	int ints[IM_MAX_HOSTS];
	char *cur = str;
	int i = 1;

	while (cur && isdigit(*cur) && i <= IM_MAX_HOSTS) {
		ints[i++] = simple_strtoul(cur, NULL, 0);
		if ((cur = strchr(cur, ',')) != NULL)
			cur++;
	}
	ints[0] = i - 1;
	internal_ibmmca_scsi_setup(cur, ints);
	return 1;
}

__setup("ibmmcascsi=", option_setup);

static struct mca_driver ibmmca_driver = {
	.id_table = ibmmca_id_table,
	.driver = {
		.name	= "ibmmca",
		.bus	= &mca_bus_type,
		.probe	= ibmmca_probe,
		.remove	= __devexit_p(ibmmca_remove),
	},
};

static int __init ibmmca_init(void)
{
#ifdef MODULE
	/* If the driver is run as module, read from conf.modules or cmd-line */
	if (boot_options)
		option_setup(boot_options);
#endif

	return mca_register_driver_integrated(&ibmmca_driver, MCA_INTEGSCSI);
}

static void __exit ibmmca_exit(void)
{
	mca_unregister_driver(&ibmmca_driver);
}

module_init(ibmmca_init);
module_exit(ibmmca_exit);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              