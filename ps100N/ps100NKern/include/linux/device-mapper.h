				 struct list_head *);
static void qla1280_error_entry(struct scsi_qla_host *, struct response *,
				struct list_head *);
static uint16_t qla1280_get_nvram_word(struct scsi_qla_host *, uint32_t);
static uint16_t qla1280_nvram_request(struct scsi_qla_host *, uint32_t);
static uint16_t qla1280_debounce_register(volatile uint16_t __iomem *);
static request_t *qla1280_req_pkt(struct scsi_qla_host *);
static int qla1280_check_for_dead_scsi_bus(struct scsi_qla_host *,
					   unsigned int);
static void qla1280_get_target_parameters(struct scsi_qla_host *,
					   struct scsi_device *);
static int qla1280_set_target_parameters(struct scsi_qla_host *, int, int);


static struct qla_driver_setup driver_setup;

/*
 * convert scsi data direction to request_t control flags
 */
static inline uint16_t
qla1280_data_direction(struct scsi_cmnd *cmnd)
{
	switch(cmnd->sc_data_direction) {
	case DMA_FROM_DEVICE:
		return BIT_5;
	case DMA_TO_DEVICE:
		return BIT_6;
	case DMA_BIDIRECTIONAL:
		return BIT_5 | BIT_6;
	/*
	 * We could BUG() on default here if one of the four cases aren't
	 * met, but then again if we receive something like that from the
	 * SCSI layer we have more serious problems. This shuts up GCC.
	 */
	case DMA_NONE:
	default:
		return 0;
	}
}
		
#if DEBUG_QLA1280
static void __qla1280_print_scsi_cmd(struct scsi_cmnd * cmd);
static void __qla1280_dump_buffer(char *, int);
#endif


/*
 * insmod needs to find the variable and make it point to something
 */
#ifdef MODULE
static char *qla1280;

/* insmod qla1280 options=verbose" */
module_param(qla1280, charp, 0);
#else
__setup("qla1280=", qla1280_setup);
#endif


/*
 * We use the scsi_pointer structure that's included with each scsi_command
 * to overlay our struct srb over it. qla1280_init() checks that a srb is not
 * bigger than a scsi_pointer.
 */

#define	CMD_SP(Cmnd)		&Cmnd->SCp
#define	CMD_CDBLEN(Cmnd)	Cmnd->cmd_len
#define	CMD_CDBP(Cmnd)		Cmnd->cmnd
#define	CMD_SNSP(Cmnd)		Cmnd->sense_buffer
#define	CMD_SNSLEN(Cmnd)	SCSI_SENSE_BUFFERSIZE
#define	CMD_RESULT(Cmnd)	Cmnd->result
#define	CMD_HANDLE(Cmnd)	Cmnd->host_scribble
#define CMD_REQUEST(Cmnd)	Cmnd->request->cmd

#define CMD_HOST(Cmnd)		Cmnd->device->host
#define SCSI_BUS_32(Cmnd)	Cmnd->device->channel
#define SCSI_TCN_32(Cmnd)	Cmnd->device->id
#define SCSI_LUN_32(Cmnd)	Cmnd->device->lun


/*****************************************/
/*   ISP Boards supported by this driver */
/*****************************************/

struct qla_boards {
	unsigned char name[9];	/* Board ID String */
	int numPorts;		/* Number of SCSI ports */
	char *fwname;		/* firmware name        */
};

/* NOTE: the last argument in each entry is used to index ql1280_board_tbl */
static struct pci_device_id qla1280_pci_tbl[] = {
	{PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP12160,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP1020,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 1},
	{PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP1080,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 2},
	{PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP1240,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 3},
	{PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP1280,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 4},
	{PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP10160,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 5},
	{0,}
};
MODULE_DEVICE_TABLE(pci, qla1280_pci_tbl);

static struct qla_boards ql1280_board_tbl[] = {
	/* Name ,  Number of ports, FW details */
	{"QLA12160",	2, "qlogic/12160.bin"},
	{"QLA1040",	1, "qlogic/1040.bin"},
	{"QLA1080",	1, "qlogic/1280.bin"},
	{"QLA1240",	2, "qlogic/1280.bin"},
	{"QLA1280",	2, "qlogic/1280.bin"},
	{"QLA10160",	1, "qlogic/12160.bin"},
	{"        ",	0, "   "},
};

static int qla1280_verbose = 1;

#if DEBUG_QLA1280
static int ql_debug_level = 1;
#define dprintk(level, format, a...)	\
	do { if (ql_debug_level >= level) printk(KERN_ERR format, ##a); } while(0)
#define qla1280_dump_buffer(level, buf, size)	\
	if (ql_debug_level >= level) __qla1280_dump_buffer(buf, size)
#define qla1280_print_scsi_cmd(level, cmd)	\
	if (ql_debug_level >= level) __qla1280_print_scsi_cmd(cmd)
#else
#define ql_debug_level			0
#define dprintk(level, format, a...)	do{}while(0)
#define qla1280_dump_buffer(a, b, c)	do{}while(0)
#define qla1280_print_scsi_cmd(a, b)	do{}while(0)
#endif

#define ENTER(x)		dprintk(3, "qla1280 : Entering %s()\n", x);
#define LEAVE(x)		dprintk(3, "qla1280 : Leaving %s()\n", x);
#define ENTER_INTR(x)		dprintk(4, "qla1280 : Entering %s()\n", x);
#define LEAVE_INTR(x)		dprintk(4, "qla1280 : Leaving %s()\n", x);


static int qla1280_read_nvram(struct scsi_qla_host *ha)
{
	uint16_t *wptr;
	uint8_t chksum;
	int cnt, i;
	struct nvram *nv;

	ENTER("qla1280_read_nvram");

	if (driver_setup.no_nvram)
		return 1;

	printk(KERN_INFO "scsi(%ld): Reading NVRAM\n", ha->host_no);

	wptr = (uint16_t *)&ha->nvram;
	nv = &ha->nvram;
	chksum = 0;
	for (cnt = 0; cnt < 3; cnt++) {
		*wptr = qla1280_get_nvram_word(ha, cnt);
		chksum += *wptr & 0xff;
		chksum += (*wptr >> 8) & 0xff;
		wptr++;
	}

	if (nv->id0 != 'I' || nv->id1 != 'S' ||
	    nv->id2 != 'P' || nv->id3 != ' ' || nv->version < 1) {
		dprintk(2, "Invalid nvram ID or version!\n");
		chksum = 1;
	} else {
		for (; cnt < sizeof(struct nvram); cnt++) {
			*wptr = qla1280_get_nvram_word(ha, cnt);
			chksum += *wptr & 0xff;
			chksum += (*wptr >> 8) & 0xff;
			wptr++;
		}
	}

	dprintk(3, "qla1280_read_nvram: NVRAM Magic ID= %c %c %c %02x"
	       " version %i\n", nv->id0, nv->id1, nv->id2, nv->id3,
	       nv->version);


	if (chksum) {
		if (!driver_setup.no_nvram)
			printk(KERN_WARNING "scsi(%ld): Unable to identify or "
			       "validate NVRAM checksum, using default "
			       "settings\n", ha->host_no);
		ha->nvram_valid = 0;
	} else
		ha->nvram_valid = 1;

	/* The firmware interface is, um, interesting, in that the
	 * actual firmware image on the chip is little endian, thus,
	 * the process of taking that image to the CPU would end up
	 * little endian.  However, the firmware interface requires it
	 * to be read a word (two bytes) at a time.
	 *
	 * The net result of this would be that the word (and
	 * doubleword) quantites in the firmware would be correct, but
	 * the bytes would be pairwise reversed.  Since most of the
	 * firmware quantites are, in fact, bytes, we do an extra
	 * le16_to_cpu() in the firmware read routine.
	 *
	 * The upshot of all this is that the bytes in the firmware
	 * are in the correct places, but the 16 and 32 bit quantites
	 * are still in little endian format.  We fix that up below by
	 * doing extra reverses on them */
	nv->isp_parameter = cpu_to_le16(nv->isp_parameter);
	nv->firmware_feature.w = cpu_to_le16(nv->firmware_feature.w);
	for(i = 0; i < MAX_BUSES; i++) {
		nv->bus[i].selection_timeout = cpu_to_le16(nv->bus[i].selection_timeout);
		nv->bus[i].max_queue_depth = cpu_to_le16(nv->bus[i].max_queue_depth);
	}
	dprintk(1, "qla1280_read_nvram: Completed Reading NVRAM\n");
	LEAVE("qla1280_read_nvram");

	return chksum;
}

/**************************************************************************
 *   qla1280_info
 *     Return a string describing the driver.
 **************************************************************************/
static const char *
qla1280_info(struct Scsi_Host *host)
{
	static char qla1280_scsi_name_buffer[125];
	char *bp;
	struct scsi_qla_host *ha;
	struct qla_boards *bdp;

	bp = &qla1280_scsi_name_buffer[0];
	ha = (struct scsi_qla_host *)host->hostdata;
	bdp = &ql1280_board_tbl[ha->devnum];
	memset(bp, 0, sizeof(qla1280_scsi_name_buffer));

	sprintf (bp,
		 "QLogic %s PCI to SCSI Host Adapter\n"
		 "       Firmware version: %2d.%02d.%02d, Driver version %s",
		 &bdp->name[0], ha->fwver1, ha->fwver2, ha->fwver3,
		 QLA1280_VERSION);
	return bp;
}

/**************************************************************************
 *   qla1280_queuecommand
 *     Queue a command to the controller.
 *
 * Note:
 * The mid-level driver tries to ensures that queuecommand never gets invoked
 * concurrently with itself or the interrupt handler (although the
 * interrupt handler may call this routine as part of request-completion
 * handling).   Unfortunely, it sometimes calls the scheduler in interrupt
 * context which is a big NO! NO!.
 **************************************************************************/
static int
qla1280_queuecommand(struct scsi_cmnd *cmd, void (*fn)(struct scsi_cmnd *))
{
	struct Scsi_Host *host = cmd->device->host;
	struct scsi_qla_host *ha = (struct scsi_qla_host *)host->hostdata;
	struct srb *sp = (struct srb *)CMD_SP(cmd);
	int status;

	cmd->scsi_done = fn;
	sp->cmd = cmd;
	sp->flags = 0;
	sp->wait = NULL;
	CMD_HANDLE(cmd) = (unsigned char *)NULL;

	qla1280_print_scsi_cmd(5, cmd);

#ifdef QLA_64BIT_PTR
	/*
	 * Using 64 bit commands if the PCI bridge doesn't support it is a
	 * bit wasteful, however this should really only happen if one's
	 * PCI controller is completely broken, like the BCM1250. For
	 * sane hardware this is not an issue.
	 */
	status = qla1280_64bit_start_scsi(ha, sp);
#else
	status = qla1280_32bit_start_scsi(ha, sp);
#endif
	return status;
}

enum action {
	ABORT_COMMAND,
	DEVICE_RESET,
	BUS_RESET,
	ADAPTER_RESET,
};


static void qla1280_mailbox_timeout(unsigned long __data)
{
	struct scsi_qla_host *ha = (struct scsi_qla_host *)__data;
	struct device_reg __iomem *reg;
	reg = ha->iobase;

	ha->mailbox_out[0] = RD_REG_WORD(&reg->mailbox0);
	printk(KERN_ERR "scsi(%ld): mailbox timed out, mailbox0 %04x, "
	       "ictrl %04x, istatus %04x\n", ha->host_no, ha->mailbox_out[0],
	       RD_REG_WORD(&reg->ictrl), RD_REG_WORD(&reg->istatus));
	complete(ha->mailbox_wait);
}

static int
_qla1280_wait_for_single_command(struct scsi_qla_host *ha, struct srb *sp,
				 struct completion *wait)
{
	int	status = FAILED;
	struct scsi_cmnd *cmd = sp->cmd;

	spin_unlock_irq(ha->host->host_lock);
	wait_for_completion_timeout(wait, 4*HZ);
	spin_lock_irq(ha->host->host_lock);
	sp->wait = NULL;
	if(CMD_HANDLE(cmd) == COMPLETED_HANDLE) {
		status = SUCCESS;
		(*cmd->scsi_done)(cmd);
	}
	return status;
}

static int
qla1280_wait_for_single_command(struct scsi_qla_host *ha, struct srb *sp)
{
	DECLARE_COMPLETION_ONSTACK(wait);

	sp->wait = &wait;
	return _qla1280_wait_for_single_command(ha, sp, &wait);
}

static int
qla1280_wait_for_pending_commands(struct scsi_qla_host *ha, int bus, int target)
{
	int		cnt;
	int		status;
	struct srb	*sp;
	struct scsi_cmnd *cmd;

	status = SUCCESS;

	/*
	 * Wait for all commands with the designated bus/target
	 * to be completed by the firmware
	 */
	for (cnt = 0; cnt < MAX_OUTSTANDING_COMMANDS; cnt++) {
		sp = ha->outstanding_cmds[cnt];
		if (sp) {
			cmd = sp->cmd;

			if (bus >= 0 && SCSI_BUS_32(cmd) != bus)
				continue;
			if (target >= 0 && SCSI_TCN_32(cmd) != target)
				continue;

			status = qla1280_wait_for_single_command(ha, sp);
			if (status == FAILED)
				break;
		}
	}
	return status;
}

/**************************************************************************
 * qla1280_error_action
 *    The function will attempt to perform a specified error action and
 *    wai