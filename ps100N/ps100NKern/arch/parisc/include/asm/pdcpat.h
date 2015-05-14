****************************/
static int initio_bad_seq(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb;

	printk("initio_bad_seg c=%d\n", host->index);

	if ((scb = host->active) != NULL) {
		initio_unlink_busy_scb(host, scb);
		scb->hastat = HOST_BAD_PHAS;
		scb->tastat = 0;
		initio_append_done_scb(host, scb);
	}
	initio_stop_bm(host);
	initio_reset_scsi(host, 8);	/* 7/29/98 */
	return initio_post_scsi_rst(host);
}


/************************************************************************/
static void initio_exec_scb(struct initio_host * host, struct scsi_ctrl_blk * scb)
{
	unsigned long flags;

	scb->mode = 0;

	scb->sgidx = 0;
	scb->sgmax = scb->sglen;

	spin_lock_irqsave(&host->semaph_lock, flags);

	initio_append_pend_scb(host, scb);	/* Append this SCB to Pending queue */

/* VVVVV 07/21/98 */
	if (host->semaph == 1) {
		/* Disable Jasmin SCSI Int */
		outb(0x1F, host->addr + TUL_Mask);
		host->semaph = 0;
		spin_unlock_irqrestore(&host->semaph_lock, flags);

		tulip_main(host);

		spin_lock_irqsave(&host->semaph_lock, flags);
		host->semaph = 1;
		outb(0x0F, host->addr + TUL_Mask);
	}
	spin_unlock_irqrestore(&host->semaph_lock, flags);
	return;
}

/***************************************************************************/
static int initio_isr(struct initio_host * host)
{
	if (inb(host->addr + TUL_Int) & TSS_INT_PENDING) {
		if (host->semaph == 1) {
			outb(0x1F, host->addr + TUL_Mask);
			/* Disable Tulip SCSI Int */
			host->semaph = 0;

			tulip_main(host);

			host->semaph = 1;
			outb(0x0F, host->addr + TUL_Mask);
			return 1;
		}
	}
	return 0;
}

static int tulip_main(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb;

	for (;;) {
		tulip_scsi(host);	/* Call tulip_scsi              */

		/* Walk the list of completed SCBs */
		while ((scb = initio_find_done_scb(host)) != NULL) {	/* find done entry */
			if (scb->tastat == INI_QUEUE_FULL) {
				host->max_tags[scb->target] =
				    host->act_tags[scb->target] - 1;
				scb->tastat = 0;
				initio_append_pend_scb(host, scb);
				continue;
			}
			if (!(scb->mode & SCM_RSENS)) {		/* not in auto req. sense mode */
				if (scb->tastat == 2) {

					/* clr sync. nego flag */

					if (scb->flags & SCF_SENSE) {
						u8 len;
						len = scb->senselen;
						if (len == 0)
							len = 1;
						scb->buflen = scb->senselen;
						scb->bufptr = scb->senseptr;
						scb->flags &= ~(SCF_SG | SCF_DIR);	/* for xfer_data_in */
						/* so, we won't report wrong direction in xfer_data_in,
						   and won't report HOST_DO_DU in state_6 */
						scb->mode = SCM_RSENS;
						scb->ident &= 0xBF;	/* Disable Disconnect */
						scb->tagmsg = 0;
						scb->tastat = 0;
						scb->cdblen = 6;
						scb->cdb[0] = SCSICMD_RequestSense;
						scb->cdb[1] = 0;
						scb->cdb[2] = 0;
						scb->cdb[3] = 0;
						scb->cdb[4] = len;
						scb->cdb[5] = 0;
						initio_push_pend_scb(host, scb);
						break;
					}
				}
			} else {	/* in request sense mode */

				if (scb->tastat == 2) {		/* check contition status again after sending
									   requset sense cmd 0x3 */
					scb->hastat = HOST_BAD_PHAS;
				}
				scb->tastat = 2;
			}
			scb->flags |= SCF_DONE;
			if (scb->flags & SCF_POST) {
				/* FIXME: only one post method and lose casts */
				(*scb->post) ((u8 *) host, (u8 *) scb);
			}
		}		/* while */
		/* find_active: */
		if (inb(host->addr + TUL_SStatus0) & TSS_INT_PENDING)
			continue;
		if (host->active)	/* return to OS and wait for xfer_done_ISR/Selected_ISR */
			return 1;	/* return to OS, enable interrupt */
		/* Check pending SCB            */
		if (initio_find_first_pend_scb(host) == NULL)
			return 1;	/* return to OS, enable interrupt */
	}			/* End of for loop */
	/* statement won't reach here */
}

static void tulip_scsi(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb;
	struct target_control *active_tc;

	/* make sure to service interrupt asap */
	if ((host->jsstatus0 = inb(host->addr + TUL_SStatus0)) & TSS_INT_PENDING) {
		host->phase = host->jsstatus0 & TSS_PH_MASK;
		host->jsstatus1 = inb(host->addr + TUL_SStatus1);
		host->jsint = inb(host->addr + TUL_SInt);
		if (host->jsint & TSS_SCSIRST_INT) {	/* SCSI bus reset detected      */
			int_initio_scsi_rst(host);
			return;
		}
		if (host->jsint & TSS_RESEL_INT) {	/* if selected/reselected interrupt */
			if (int_initio_resel(host) == 0)
				initio_next_state(host);
			return;
		}
		if (host->jsint & TSS_SEL_TIMEOUT) {
			int_initio_busfree(host);
			return;
		}
		if (host->jsint & TSS_DISC_INT) {	/* BUS disconnection            */
			int_initio_busfree(host);	/* unexpected bus free or sel timeout */
			return;
		}
		if (host->jsint & (TSS_FUNC_COMP | TSS_BUS_SERV)) {	/* func complete or Bus service */
			if ((scb = host->active) != NULL)
				initio_next_state(host);
			return;
		}
	}
	if (host->active != NULL)
		return;

	if ((scb = initio_find_first_pend_scb(host)) == NULL)
		return;

	/* program HBA's SCSI ID & target SCSI ID */
	outb((host->scsi_id << 4) | (scb->target & 0x0F),
		host->addr + TUL_SScsiId);
	if (scb->opcode == ExecSCSI) {
		active_tc = &host->targets[scb->target];

		if (scb->tagmsg)
			active_tc->drv_flags |= TCF_DRV_EN_TAG;
		else
			active_tc->drv_flags &= ~TCF_DRV_EN_TAG;

		outb(active_tc->js_period, host->addr + TUL_SPeriod);
		if ((active_tc->flags & (TCF_WDTR_DONE | TCF_NO_WDTR)) == 0) {	/* do wdtr negotiation          */
			initio_select_atn_stop(host, scb);
		} else {
			if ((active_tc->flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) == 0) {	/* do sync negotiation          */
				initio_select_atn_stop(host, scb);
			} else {
				if (scb->tagmsg)
					initio_select_atn3(host, scb);
				else
					initio_select_atn(host, scb);
			}
		}
		if (scb->flags & SCF_POLL) {
			while (wait_tulip(host) != -1) {
				if (initio_next_state(host) == -1)
					break;
			}
		}
	} else if (scb->opcode == BusDevRst) {
		initio_select_atn_stop(host, scb);
		scb->next_state = 8;
		if (scb->flags & SCF_POLL) {
			while (wait_tulip(host) != -1) {
				if (initio_next_state(host) == -1)
					break;
			}
		}
	} else if (scb->opcode == AbortCmd) {
		if (initio_abort_srb(host, scb->srb) != 0) {
			initio_unlink_pend_scb(host, scb);
			initio_release_scb(host, scb);
		} else {
			scb->opcode = BusDevRst;
			initio_select_atn_stop(host, scb);
			scb->next_state = 8;
		}
	} else {
		initio_unlink_pend_scb(host, scb);
		scb->hastat = 0x16;	/* bad command */
		initio_append_done_scb(host, scb);
	}
	return;
}

/**
 *	initio_next_state		-	Next SCSI state
 *	@host: InitIO host we are processing
 *
 *	Progress the active command block along the state machine
 *	until we hit a state which we must wait for activity to occur.
 *
 *	Returns zero or a negative code.
 */

static int initio_next_state(struct initio_host * host)
{
	int next;

	next = host->active->next_state;
	for (;;) {
		switch (next) {
		case 1:
			next = initio_state_1(host);
			break;
		case 2:
			next = initio_state_2(host);
			break;
		case 3:
			next = initio_state_3(host);
			break;
		case 4:
			next = initio_state_4(host);
			break;
		case 5:
			next = initio_state_5(host);
			break;
		case 6:
			next = initio_state_6(host);
			break;
		case 7:
			next = initio_state_7(host);
			break;
		case 8:
			return initio_bus_device_reset(host);
		default:
			return initio_bad_seq(host);
		}
		if (next <= 0)
			return next;
	}
}


/**
 *	initio_state_1		-	SCSI state machine
 *	@host: InitIO host we are controlling
 *
 *	Perform SCSI state processing for Select/Attention/Stop
 */

static int initio_state_1(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb = host->active;
	struct target_control *active_tc = host->active_tc;
#if DEBUG_STATE
	printk("-s1-");
#endif

	/* Move the SCB from pending to busy */
	initio_unlink_pend_scb(host, scb);
	initio_append_busy_scb(host, scb);

	outb(active_tc->sconfig0, host->addr + TUL_SConfig );
	/* ATN on */
	if (host->phase == MSG_OUT) {
		outb(TSC_EN_BUS_IN | TSC_HW_RESELECT, host->addr + TUL_SCtrl1);
		outb(scb->ident, host->addr + TUL_SFifo);

		if (scb->tagmsg) {
			outb(scb->tagmsg, host->addr + TUL_SFifo);
			outb(scb->tagid, host->addr + TUL_SFifo);
		}
		if ((active_tc->flags & (TCF_WDTR_DONE | TCF_NO_WDTR)) == 0) {
			active_tc->flags |= TCF_WDTR_DONE;
			outb(MSG_EXTEND, host->addr + TUL_SFifo);
			outb(2, host->addr + TUL_SFifo);	/* Extended msg length */
			outb(3, host->addr + TUL_SFifo);	/* Sync request */
			outb(1, host->addr + TUL_SFifo);	/* Start from 16 bits */
		} else if ((active_tc->flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) == 0) {
			active_tc->flags |= TCF_SYNC_DONE;
			outb(MSG_EXTEND, host->addr + TUL_SFifo);
			outb(3, host->addr + TUL_SFifo);	/* extended msg length */
			outb(1, host->addr + TUL_SFifo);	/* sync request */
			outb(initio_rate_tbl[active_tc->flags & TCF_SCSI_RATE], host->addr + TUL_SFifo);
			outb(MAX_OFFSET, host->addr + TUL_SFifo);	/* REQ/ACK offset */
		}
		outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
		if (wait_tulip(host) == -1)
			return -1;
	}
	outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0);
	outb((inb(host->addr + TUL_SSignal) & (TSC_SET_ACK | 7)), host->addr + TUL_SSignal);
	/* Into before CDB xfer */
	return 3;
}


/**
 *	initio_state_2		-	SCSI state machine
 *	@host: InitIO host we are controlling
 *
 * state after selection with attention
 * state after selection with attention3
 */

static int initio_state_2(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb = host->active;
	struct target_control *active_tc = host->active_tc;
#if DEBUG_STATE
	printk("-s2-");
#endif

	initio_unlink_pend_scb(host, scb);
	initio_append_busy_scb(host, scb);

	outb(active_tc->sconfig0, host->addr + TUL_SConfig);

	if (host->jsstatus1 & TSS_CMD_PH_CMP)
		return 4;

	outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0);
	outb((inb(host->addr + TUL_SSignal) & (TSC_SET_ACK | 7)), host->addr + TUL_SSignal);
	/* Into before CDB xfer */
	return 3;
}

/**
 *	initio_state_3		-	SCSI state machine
 *	@host: InitIO host we are controlling
 *
 * state before CDB xfer is done
 */

static int initio_state_3(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb = host->active;
	struct target_control *active_tc = host->active_tc;
	int i;

#if DEBUG_STATE
	printk("-s3-");
#endif
	for (;;) {
		switch (host->phase) {
		case CMD_OUT:	/* Command out phase            */
			for (i = 0; i < (int) scb->cdblen; i++)
				outb(scb->cdb[i], host->addr + TUL_SFifo);
			outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
			if (wait_tulip(host) == -1)
				return -1;
			if (host->phase == CMD_OUT)
				return initio_bad_seq(host);
			return 4;

		case MSG_IN:	/* Message in phase             */
			scb->next_state = 3;
			if (initio_msgin(host) == -1)
				return -1;
			break;

		case STATUS_IN:	/* Status phase                 */
			if (initio_status_msg(host) == -1)
				return -1;
			break;

		case MSG_OUT:	/* Message out phase            */
			if (active_tc->flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) {
				outb(MSG_NOP, host->addr + TUL_SFifo);		/* msg nop */
				outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
				if (wait_tulip(host) == -1)
					return -1;
			} else {
				active_tc->flags |= TCF_SYNC_DONE;

				outb(MSG_EXTEND, host->addr + TUL_SFifo);
				outb(3, host->addr + TUL_SFifo);	/* ext. msg len */
				outb(1, host->addr + TUL_SFifo);	/* sync request */
				outb(initio_rate_tbl[active_tc->flags & TCF_SCSI_RATE], host->addr + TUL_SFifo);
				outb(MAX_OFFSET, host->addr + TUL_SFifo);	/* REQ/ACK offset */
				outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
				if (wait_tulip(host) == -1)
					return -1;
				outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0);
				outb(inb(host->addr + TUL_SSignal) & (TSC_SET_ACK | 7), host->addr + TUL_SSignal);

			}
			break;
		default:
			return initio_bad_seq(host);
		}
	}
}

/**
 *	initio_state_4		-	SCSI state machine
 *	@host: InitIO host we are controlling
 *
 *	SCSI state machine. State 4
 */

static int initio_state_4(struct initio_host * host)
{
	struct scsi_ctrl_blk *scb = host->active;

#if DEBUG_STATE
	printk("-s4-");
#endif
	if ((scb->flags & SCF_DIR) == SCF_NO_XF) {
		return 6;	/* Go to state 6 (After data) */
	}
	for (;;) {
		if (scb->buflen == 0)
			return 6;

		switch (host->phase) {

		case STATUS_IN:	/* Status phase                 */
			if ((scb->flags & SCF_DIR) != 0)	/* if direction bit set  mod_info;		/* module information */
	unsigned long mod_location;	/* physical location of the module */
	struct hardware_path mod_path;	/* module path (device path - layers) */
	unsigned long mod[508];		/* PAT cell module components */
} __attribute__((aligned(8))) ;

typedef struct pdc_pat_cell_mod_maddr_block pdc_pat_cell_mod_maddr_block_t;


extern int pdc_pat_chassis_send_log(unsigned long status, unsigned long data);
extern int pdc_pat_cell_get_number(struct pdc_pat_cell_num *cell_info);
extern int pdc_pat_cell_module(unsigned long *actcnt, unsigned long ploc, unsigned long mod, unsigned long view_type, void *mem_addr);
extern int pdc_pat_cell_num_to_loc(void *, unsigned long);

extern int pdc_pat_cpu_get_number(struct pdc_pat_cpu_num *cpu_info, void *hpa);

extern int pdc_pat_pd_get_addr_map(unsigned long *actual_len, void *mem_addr, unsigned long count, unsigned long offset);


extern int pdc_pat_io_pci_cfg_read(unsigned long pci_addr, int pci_size, u32 *val); 
extern int pdc_pat_io_pci_cfg_write(unsigned long pci_addr, int pci_size, u32 val); 


/* Flag to indicate this is a PAT box...don't use this unless you
** really have to...it might go away some day.
*/
extern int pdc_pat;     /* arch/parisc/kernel/inventory.c */

#endif /* __ASSEMBLY__ */

#endif /* ! __PARISC_PATPDC_H */
