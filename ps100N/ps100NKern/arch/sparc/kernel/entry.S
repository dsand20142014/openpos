tatus Entry invalid handle.\n");

		set_bit(ISP_ABORT_NEEDED, &vha->dpc_flags);
		qla2xxx_wake_dpc(vha);
		return;
	}
	cp = sp->cmd;
	if (cp == NULL) {
		DEBUG2(printk("scsi(%ld): Command already returned back to OS "
		    "pkt->handle=%d sp=%p.\n", vha->host_no, handle, sp));
		qla_printk(KERN_WARNING, ha,
		    "Command is NULL: already returned to OS (sp=%p)\n", sp);

		return;
	}

  	lscsi_status = scsi_status & STATUS_MASK;

	fcport = sp->fcport;

	sense_len = rsp_info_len = resid_len = fw_resid_len = 0;
	if (IS_FWI2_CAPABLE(ha)) {
		sense_len = le32_to_cpu(sts24->sense_len);
		rsp_info_len = le32_to_cpu(sts24->rsp_data_len);
		resid_len = le32_to_cpu(sts24->rsp_residual_count);
		fw_resid_len = le32_to_cpu(sts24->residual_len);
		rsp_info = sts24->data;
		sense_data = sts24->data;
		host_to_fcp_swap(sts24->data, sizeof(sts24->data));
	} else {
		sense_len = le16_to_cpu(sts->req_sense_length);
		rsp_info_len = le16_to_cpu(sts->rsp_info_len);
		resid_len = le32_to_cpu(sts->residual_length);
		rsp_info = sts->rsp_info;
		sense_data = sts->req_sense_data;
	}

	/* Check for any FCP transport errors. */
	if (scsi_status & SS_RESPONSE_INFO_LEN_VALID) {
		/* Sense data lies beyond any FCP RESPONSE data. */
		if (IS_FWI2_CAPABLE(ha))
			sense_data += rsp_info_len;
		if (rsp_info_len > 3 && rsp_info[3]) {
			DEBUG2(printk("scsi(%ld:%d:%d:%d) FCP I/O protocol "
			    "failure (%x/%02x%02x%02x%02x%02x%02x%02x%02x)..."
			    "retrying command\n", vha->host_no,
			    cp->device->channel, cp->device->id,
			    cp->device->lun, rsp_info_len, rsp_info[0],
			    rsp_info[1], rsp_info[2], rsp_info[3], rsp_info[4],
			    rsp_info[5], rsp_info[6], rsp_info[7]));

			cp->result = DID_BUS_BUSY << 16;
			qla2x00_sp_compl(ha, sp);
			return;
		}
	}

	/* Check for overrun. */
	if (IS_FWI2_CAPABLE(ha) && comp_status == CS_COMPLETE &&
	    scsi_status & SS_RESIDUAL_OVER)
		comp_status = CS_DATA_OVERRUN;

	/*
	 * Based on Host and scsi status generate status code for Linux
	 */
	switch (comp_status) {
	case CS_COMPLETE:
	case CS_QUEUE_FULL:
		if (scsi_status == 0) {
			cp->result = DID_OK << 16;
			break;
		}
		if (scsi_status & (SS_RESIDUAL_UNDER | SS_RESIDUAL_OVER)) {
			resid = resid_len;
			scsi_set_resid(cp, resid);

			if (!lscsi_status &&
			    ((unsigned)(scsi_bufflen(cp) - resid) <
			     cp->underflow)) {
				qla_printk(KERN_INFO, ha,
					   "scsi(%ld:%d:%d:%d): Mid-layer underflow "
					   "detected (%x of %x bytes)...returning "
					   "error status.\n", vha->host_no,
					   cp->device->channel, cp->device->id,
					   cp->device->lun, resid,
					   scsi_bufflen(cp));

				cp->result = DID_ERROR << 16;
				break;
			}
		}
		cp->result = DID_OK << 16 | lscsi_status;

		if (lscsi_status == SAM_STAT_TASK_SET_FULL) {
			DEBUG2(printk(KERN_INFO
			    "scsi(%ld): QUEUE FULL status detected "
			    "0x%x-0x%x.\n", vha->host_no, comp_status,
			    scsi_status));

			/* Adjust queue depth for all luns on the port. */
			if (!ql2xqfulltracking)
				break;
			fcport->last_queue_full = jiffies;
			starget_for_each_device(cp->device->sdev_target,
			    fcport, qla2x00_adjust_sdev_qdepth_down);
			break;
		}
		if (lscsi_status != SS_CHECK_CONDITION)
			break;

		memset(cp->sense_buffer, 0, SCSI_SENSE_BUFFERSIZE);
		if (!(scsi_status & SS_SENSE_LEN_VALID))
			break;

		qla2x00_handle_sense(sp, sense_data, sense_len, rsp);
		break;

	case CS_DATA_UNDERRUN:
		resid = resid_len;
		/* Use F/W calculated residual length. */
		if (IS_FWI2_CAPABLE(ha)) {
			if (!(scsi_status & SS_RESIDUAL_UNDER)) {
				lscsi_status = 0;
			} else if (resid != fw_resid_len) {
				scsi_status &= ~SS_RESIDUAL_UNDER;
				lscsi_status = 0;
			}
			resid = fw_resid_len;
		}

		if (scsi_status & SS_RESIDUAL_UNDER) {
			scsi_set_resid(cp, resid);
		} else {
			DEBUG2(printk(KERN_INFO
			    "scsi(%ld:%d:%d) UNDERRUN status detected "
			    "0x%x-0x%x. resid=0x%x fw_resid=0x%x cdb=0x%x "
			    "os_underflow=0x%x\n", vha->host_no,
			    cp->device->id, cp->device->lun, comp_status,
			    scsi_status, resid_len, resid, cp->cmnd[0],
			    cp->underflow));

		}

		/*
		 * Check to see if SCSI Status is non zero. If so report SCSI
		 * Status.
		 */
		if (lscsi_status != 0) {
			cp->result = DID_OK << 16 | lscsi_status;

			if (lscsi_status == SAM_STAT_TASK_SET_FULL) {
				DEBUG2(printk(KERN_INFO
				    "scsi(%ld): QUEUE FULL status detected "
				    "0x%x-0x%x.\n", vha->host_no, comp_status,
				    scsi_status));

				/*
				 * Adjust queue depth for all luns on the
				 * port.
				 */
				if (!ql2xqfulltracking)
					break;
				fcport->last_queue_full = jiffies;
				starget_for_each_device(
				    cp->device->sdev_target, fcport,
				    qla2x00_adjust_sdev_qdepth_down);
				break;
			}
			if (lscsi_status != SS_CHECK_CONDITION)
				break;

			memset(cp->sense_buffer, 0, SCSI_SENSE_BUFFERSIZE);
			if (!(scsi_status & SS_SENSE_LEN_VALID))
				break;

			qla2x00_handle_sense(sp, sense_data, sense_len, rsp);
		} else {
			/*
			 * If RISC reports underrun and target does not report
			 * it then we must have a lost frame, so tell upper
			 * layer to retry it by reporting an error.
			 */
			if (!(scsi_status & SS_RESIDUAL_UNDER)) {
				DEBUG2(printk("scsi(%ld:%d:%d:%d) Dropped "
					      "frame(s) detected (%x of %x bytes)..."
					      "retrying command.\n",
					vha->host_no, cp->device->channel,
					cp->device->id, cp->device->lun, resid,
					scsi_bufflen(cp)));

				cp->result = DID_ERROR << 16;
				break;
			}

			/* Handle mid-layer underflow */
			if ((unsigned)(scsi_bufflen(cp) - resid) <
			    cp->underflow) {
				qla_printk(KERN_INFO, ha,
					   "scsi(%ld:%d:%d:%d): Mid-layer underflow "
					   "detected (%x of %x bytes)...returning "
					   "error status.\n", vha->host_no,
					   cp->device->channel, cp->device->id,
					   cp->device->lun, resid,
					   scsi_bufflen(cp));

				cp->result = DID_ERROR << 16;
				break;
			}

			/* Everybody online, looking good... */
			cp->result = DID_OK << 16;
		}
		break;

	case CS_DATA_OVERRUN:
		DEBUG2(printk(KERN_INFO
		    "scsi(%ld:%d:%d): OVERRUN status detected 0x%x-0x%x\n",
		    vha->host_no, cp->device->id, cp->device->lun, comp_status,
		    scsi_status));
		DEBUG2(printk(KERN_INFO
		    "CDB: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		    cp->cmnd[0], cp->cmnd[1], cp->cmnd[2], cp->cmnd[3],
		    cp->cmnd[4], cp->cmnd[5]));
		DEBUG2(printk(KERN_INFO
		    "PID=0x%lx req=0x%x xtra=0x%x -- returning DID_ERROR "
		    "status!\n",
		    cp->serial_number, scsi_bufflen(cp), resid_len));

		cp->result = DID_ERROR << 16;
		break;

	case CS_PORT_LOGGED_OUT:
	case CS_PORT_CONFIG_CHG:
	case CS_PORT_BUSY:
	case CS_INCOMPLETE:
	case CS_PORT_UNAVAILABLE:
		/*
		 * If the port is in Target Down state, return all IOs for this
		 * Target with DID_NO_CONNECT ELSE Queue the IOs in the
		 * retry_queue.
		 */
		DEBUG2(printk("scsi(%ld:%d:%d): status_entry: Port Down "
		    "pid=%ld, compl status=0x%x, port state=0x%x\n",
		    vha->host_no, cp->device->id, cp->device->lun,
		    cp->serial_number, comp_status,
		    atomic_read(&fcport->state)));

		/*
		 * We are going to have the fc class block the rport
		 * while we try to recover so instruct the mid layer
		 * to requeue until the class decides how to handle this.
		 */
		cp->result = DID_TRANSPORT_DISRUPTED << 16;
		if (atomic_read(&fcport->state) == FCS_ONLINE)
			qla2x00_mark_device_lost(fcport->vha, fcport, 1, 1);
		break;

	case CS_RESET:
		DEBUG2(printk(KERN_INFO
		    "scsi(%ld): RESET status detected 0x%x-0x%x.\n",
		    vha->host_no, comp_status, scsi_status));

		cp->result = DID_RESET << 16;
		break;

	case CS_ABORTED:
		/*
		 * hv2.19.12 - DID_ABORT does not retry the request if we
		 * aborted this request then abort otherwise it must be a
		 * reset.
		 */
		DEBUG2(printk(KERN_INFO
		    "scsi(%ld): ABORT status detected 0x%x-0x%x.\n",
		    vha->host_no, comp_status, scsi_status));

		cp->result = DID_RESET << 16;
		break;

	case CS_TIMEOUT:
		/*
		 * We are going to have the fc class block the rport
		 * while we try to recover so instruct the mid layer
		 * to requeue until the class decides how to handle this.
		 */
		cp->result = DID_TRANSPORT_DISRUPTED << 16;

		if (IS_FWI2_CAPABLE(ha)) {
			DEBUG2(printk(KERN_INFO
			    "scsi(%ld:%d:%d:%d): TIMEOUT status detected "
			    "0x%x-0x%x\n", vha->host_no, cp->device->channel,
			    cp->device->id, cp->device->lun, comp_status,
			    scsi_status));
			break;
		}
		DEBUG2(printk(KERN_INFO
		    "scsi(%ld:%d:%d:%d): TIMEOUT status detected 0x%x-0x%x "
		    "sflags=%x.\n", vha->host_no, cp->device->channel,
		    cp->device->id, cp->device->lun, comp_status, scsi_status,
		    le16_to_cpu(sts->status_flags)));

		/* Check to see if logout occurred. */
		if ((le16_to_cpu(sts->status_flags) & SF_LOGOUT_SENT))
			qla2x00_mark_device_lost(fcport->vha, fcport, 1, 1);
		break;

	default:
		DEBUG3(printk("scsi(%ld): Error detected (unknown status) "
		    "0x%x-0x%x.\n", vha->host_no, comp_status, scsi_status));
		qla_printk(KERN_INFO, ha,
		    "Unknown status detected 0x%x-0x%x.\n",
		    comp_status, scsi_status);

		cp->result = DID_ERROR << 16;
		break;
	}

	/* Place command on done queue. */
	if (rsp->status_srb == NULL)
		qla2x00_sp_compl(ha, sp);
}

/**
 * qla2x00_status_cont_entry() - Process a Status Continuations entry.
 * @ha: SCSI driver HA context
 * @pkt: Entry pointer
 *
 * Extended sense data.
 */
static void
qla2x00_status_cont_entry(struct rsp_que *rsp, sts_cont_entry_t *pkt)
{
	uint8_t		sense_sz = 0;
	struct qla_hw_data *ha = rsp->hw;
	srb_t		*sp = rsp->status_srb;
	struct scsi_cmnd *cp;

	if (sp != NULL && sp->request_sense_length != 0) {
		cp = sp->cmd;
		if (cp == NULL) {
			DEBUG2(printk("%s(): Cmd already returned back to OS "
			    "sp=%p.\n", __func__, sp));
			qla_printk(KERN_INFO, ha,
			    "cmd is NULL: already returned to OS (sp=%p)\n",
			    sp);

			rsp->status_srb = NULL;
			return;
		}

		if (sp->request_sense_length > sizeof(pkt->data)) {
			sense_sz = sizeof(pkt->data);
		} else {
			sense_sz = sp->request_sense_length;
		}

		/* Move sense data. */
		if (IS_FWI2_CAPABLE(ha))
			host_to_fcp_swap(pkt->data, sizeof(pkt->data));
		memcpy(sp->request_sense_ptr, pkt->data, sense_sz);
		DEBUG5(qla2x00_dump_buffer(sp->request_sense_ptr, sense_sz));

		sp->request_sense_ptr += sense_sz;
		sp->request_sense_length -= sense_sz;

		/* Place command on done queue. */
		if (sp->request_sense_length == 0) {
			rsp->status_srb = NULL;
			qla2x00_sp_compl(ha, sp);
		}
	}
}

/**
 * qla2x00_error_entry() - Process an error entry.
 * @ha: SCSI driver HA context
 * @pkt: Entry pointer
 */
static void
qla2x00_error_entry(scsi_qla_host_t *vha, struct rsp_que *rsp, sts_entry_t *pkt)
{
	srb_t *sp;
	struct qla_hw_data *ha = vha->hw;
	uint32_t handle = LSW(pkt->handle);
	uint16_t que = MSW(pkt->handle);
	struct req_que *req = ha->req_q_map[que];
#if defined(QL_DEBUG_LEVEL_2)
	if (pkt->entry_status & RF_INV_E_ORDER)
		qla_printk(KERN_ERR, ha, "%s: Invalid Entry Order\n", __func__);
	else if (pkt->entry_status & RF_INV_E_COUNT)
		qla_printk(KERN_ERR, ha, "%s: Invalid Entry Count\n", __func__);
	else if (pkt->entry_status & RF_INV_E_PARAM)
		qla_printk(KERN_ERR, ha,
		    "%s: Invalid Entry Parameter\n", __func__);
	else if (pkt->entry_status & RF_INV_E_TYPE)
		qla_printk(KERN_ERR, ha, "%s: Invalid Entry Type\n", __func__);
	else if (pkt->entry_status & RF_BUSY)
		qla_printk(KERN_ERR, ha, "%s: Busy\n", __func__);
	else
		qla_printk(KERN_ERR, ha, "%s: UNKNOWN flag error\n", __func__);
#endif

	/* Validate handle. */
	if (handle < MAX_OUTSTANDING_COMMANDS)
		sp = req->outstanding_cmds[handle];
	else
		sp = NULL;

	if (sp) {
		/* Free outstanding command slot. */
		req->outstanding_cmds[handle] = NULL;

		/* Bad payload or header */
		if (pkt->entry_status &
		    (RF_INV_E_ORDER | RF_INV_E_COUNT |
		     RF_INV_E_PARAM | RF_INV_E_TYPE)) {
			sp->cmd->result = DID_ERROR << 16;
		} else if (pkt->entry_status & RF_BUSY) {
			sp->cmd->result = DID_BUS_BUSY << 16;
		} else {
			sp->cmd->result = DID_ERROR << 16;
		}
		qla2x00_sp_compl(ha, sp);

	} else if (pkt->entry_type == COMMAND_A64_TYPE || pkt->entry_type ==
	    COMMAND_TYPE || pkt->entry_type == COMMAND_TYPE_7) {
		DEBUG2(printk("scsi(%ld): Error entry - invalid handle\n",
		    vha->host_no));
		qla_printk(KERN_WARNING, ha,
		    "Error entry - invalid handle\n");

		set_bit(ISP_ABORT_NEEDED, &vha->dpc_flags);
		qla2xxx_wake_dpc(vha);
	}
}

/**
 * qla24xx_mbx_completion() - Process mailbox command completions.
 * @ha: SCSI driver HA context
 * @mb0: Mailbox0 register
 */
static void
qla24xx_mbx_completion(scsi_qla_host_t *vha, uint16_t mb0)
{
	uint16_t	cnt;
	uint16_t __iomem *wptr;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_24xx __iomem *reg = &ha->iobase->isp24;

	/* Load return mailbox registers. */
	ha->flags.mbox_int = 1;
	ha->mailbox_out[0] = mb0;
	wptr = (uint16_t __iomem *)&reg->mailbox1;

	for (cnt = 1; cnt < ha->mbx_count; cnt++) {
		ha->mailbox_out[cnt] = RD_REG_WORD(wptr);
		wptr++;
	}

	if (ha->mcp) {
		DEBUG3(printk("%s(%ld): Got mailbox completion. cmd=%x.\n",
		    __func__, vha->host_no, ha->mcp->mb[0]));
	} else {
		DEBUG2_3(printk("%s(%ld): MBX pointer ERROR!\n",
		    __func__, vha->host_no));
	}
}

/**
 * qla24xx_process_response_queue() - Process response queue entries.
 * @ha: SCSI driver HA context
 */
void qla24xx_process_response_queue(struct scsi_qla_host *vha,
	struct rsp_que *rsp)
{
	struct sts_entry_24xx *pkt;

	if (!vha->flags.online)
		return;

	while (rsp->ring_ptr->signature != RESPONSE_PROCESSED) {
		pkt = (struct sts_entry_24xx *)rsp->ring_ptr;

		rsp->ring_index++;
		if (rsp->ring_index == rsp->length) {
			rsp->ring_index = 0;
			rsp->ring_ptr = rsp->ring;
		} else {
			rsp->ring_ptr++;
		}

		if (pkt->entry_status != 0) {
			DEBUG3(printk(KERN_INFO
			    "scsi(%ld): Process error entry.\n", vha->host_no));

			qla2x00_error_entry(vha, rsp, (sts_entry_t *) pkt);
			((response_t *)pkt)->signature = RESPONSE_PROCESSED;
			wmb();
			continue;
		}

		switch (pkt->entry_type) {
		case STATUS_TYPE:
			qla2x00_status_entry(vha, rsp, pkt);
			break;
		case STATUS_CONT_TYPE:
			qla2x00_status_cont_entry(rsp, (sts_cont_entry_t *)pkt);
			break;
		case VP_RPT_ID_IOCB_TYPE:
			qla24xx_report_id_acquisition(vha,
			    (struct vp_rpt_id_entry_24xx *)pkt);
			break;
		default:
			/* Type Not Supported. */
			DEBUG4(printk(KERN_WARNING
			    "scsi(%ld): Received unknown response pkt type %x "
			    "entry status=%x.\n",
			    vha->host_no, pkt->entry_type, pkt->entry_status));
			break;
		}
		((response_t *)pkt)->signature = RESPONSE_PROCESSED;
		wmb();
	}

	/* Adjust ring index */
	WRT_REG_DWORD(rsp->rsp_q_out, rsp->ring_index);
}

static void
qla2xxx_check_risc_status(scsi_qla_host_t *vha)
{
	int rval;
	uint32_t cnt;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_24xx __iomem *reg = &ha->iobase->isp24;

	if (!IS_QLA25XX(ha) && !IS_QLA81XX(ha))
		return;

	rval = QLA_SUCCESS;
	WRT_REG_DWORD(&reg->iobase_addr, 0x7C00);
	RD_REG_DWORD(&reg->iobase_addr);
	WRT_REG_DWORD(&reg->iobase_window, 0x0001);
	for (cnt = 10000; (RD_REG_DWORD(&reg->iobase_window) & BIT_0) == 0 &&
	    rval == QLA_SUCCESS; cnt--) {
		if (cnt) {
			WRT_REG_DWORD(&reg->iobase_window, 0x0001);
			udelay(10);
		} else
			rval = QLA_FUNCTION_TIMEOUT;
	}
	if (rval == QLA_SUCCESS)
		goto next_test;

	WRT_REG_DWORD(&reg->iobase_window, 0x0003);
	for (cnt = 100; (RD_REG_DWORD(&reg->iobase_window) & BIT_0) == 0 &&
	    rval == QLA_SUCCESS; cnt--) {
		if (cnt) {
			WRT_REG_DWORD(&reg->iobase_window, 0x0003);
			udelay(10);
		} else
			rval = QLA_FUNCTION_TIMEOUT;
	}
	if (rval != QLA_SUCCESS)
		goto done;

next_test:
	if (RD_REG_DWORD(&reg->iobase_c8) & BIT_3)
		qla_printk(KERN_INFO, ha, "Additional code -- 0x55AA.\n");

done:
	WRT_REG_DWORD(&reg->iobase_window, 0x0000);
	RD_REG_DWORD(&reg->iobase_window);
}

/**
 * qla24xx_intr_handler() - Process interrupts for the ISP23xx and ISP63xx.
 * @irq:
 * @dev_id: SCSI driver HA context
 *
 * Called by system whenever the host adapter generates an interrupt.
 *
 * Returns handled flag.
 */
irqreturn_t
qla24xx_intr_handler(int irq, void *dev_id)
{
	scsi_qla_host_t	*vha;
	struct qla_hw_data *ha;
	struct device_reg_24xx __iomem *reg;
	int		status;
	unsigned long	iter;
	uint32_t	stat;
	uint32_t	hccr;
	uint16_t	mb[4];
	struct rsp_que *rsp;
	unsigned long	flags;

	rsp = (struct rsp_que *) dev_id;
	if (!rsp) {
		printk(KERN_INFO
		    "%s(): NULL response queue pointer\n", __func__);
		return IRQ_NONE;
	}

	ha = rsp->hw;
	reg = &ha->iobase->isp24;
	status = 0;

	spin_lock_irqsave(&ha->hardware_lock, flags);
	vha = pci_get_drvdata(ha->pdev);
	for (iter = 50; iter--; ) {
		stat = RD_REG_DWORD(&reg->host_status);
		if (stat & HSRX_RISC_PAUSED) {
			if (pci_channel_offline(ha->pdev))
				break;

			hccr = RD_REG_DWORD(&reg->hccr);

			qla_printk(KERN_INFO, ha, "RISC paused -- HCCR=%x, "
			    "Dumping firmware!\n", hccr);

			qla2xxx_check_risc_status(vha);

			ha->isp_ops->fw_dump(vha, 1);
			set_bit(ISP_ABORT_NEEDED, &vha->dpc_flags);
			break;
		} else if ((stat & HSRX_RISC_INT) == 0)
			break;

		switch (stat & 0xff) {
		case 0x1:
		case 0x2:
		case 0x10:
		case 0x11:
			qla24xx_mbx_completion(vha, MSW(stat));
			status |= MBX_INTERRUPT;

			break;
		case 0x12:
			mb[0] = MSW(stat);
			mb[1] = RD_REG_WORD(&reg->mailbox1);
			mb[2] = RD_REG_WORD(&reg->mailbox2);
			mb[3] = RD_REG_WORD(&reg->mailbox3);
			qla2x00_async_event(vha, rsp, mb);
			break;
		case 0x13:
		case 0x14:
			qla24xx_process_response_queue(vha, rsp);
			break;
		default:
			DEBUG2(printk("scsi(%ld): Unrecognized interrupt type "
			    "(%d).\n",
			    vha->host_no, stat & 0xff));
			break;
		}
		WRT_REG_DWORD(&reg->hccr, HCCRX_CLR_RISC_INT);
		RD_REG_DWORD_RELAXED(&reg->hccr);
	}
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	if (test_bit(MBX_INTR_WAIT, &ha->mbx_cmd_flags) &&
	    (status & MBX_INTERRUPT) && ha->flags.mbox_int) {
		set_bit(MBX_INTERRUPT, &ha->mbx_cmd_flags);
		complete(&ha->mbx_intr_comp);
	}

	return IRQ_HANDLED;
}

static irqreturn_t
qla24xx_msix_rsp_q(int irq, void *dev_id)
{
	struct qla_hw_data *ha;
	struct rsp_que *rsp;
	struct device_reg_24xx __iomem *reg;
	struct scsi_qla_host *vha;

	rsp = (struct rsp_que *) dev_id;
	if (!rsp) {
		printk(KERN_INFO
		"%s(): NULL response queue pointer\n", __func__);
		return IRQ_NONE;
	}
	ha = rsp->hw;
	reg = &ha->iobase->isp24;

	spin_lock_irq(&ha->hardware_lock);

	vha = qla25xx_get_host(rsp);
	qla24xx_process_response_queue(vha, rsp);
	WRT_REG_DWORD(&reg->hccr, HCCRX_CLR_RISC_INT);

	spin_unlock_irq(&ha->hardware_lock);

	return IRQ_HANDLED;
}

static irqreturn_t
qla25xx_msix_rsp_q(int irq, void *dev_id)
{
	struct qla_hw_data *ha;
	struct rsp_que *rsp;

	rsp = (struct rsp_que *) dev_id;
	if (!rsp) {
		printk(KERN_INFO
			"%s(): NULL response queue pointer\n", __func__);
		return IRQ_NONE;
	}
	ha = rsp->hw;

	queue_work_on((int) (rsp->id - 1), ha->wq, &rsp->q_work);

	return IRQ_HANDLED;
}

static irqreturn_t
qla24xx_msix_default(int irq, void *dev_id)
{
	scsi_qla_host_t	*vha;
	struct qla_hw_data *ha;
	struct rsp_que *rsp;
	struct device_reg_24xx __iomem *reg;
	int		status;
	uint32_t	stat;
	uint32_t	hccr;
	uint16_t	mb[4];

	rsp = (struct rsp_que *) dev_id;
	if (!rsp) {
		DEBUG(printk(
		"%s(): NULL response queue pointer\n", __func__));
		return IRQ_NONE;
	}
	ha = rsp->hw;
	reg = &ha->iobase->isp24;
	status = 0;

	spin_lock_irq(&ha->hardware_lock);
	vha = pci_get_drvdata(ha->pdev);
	do {
		stat = RD_REG_DWORD(&reg->host_status);
		if (stat & HSRX_RISC_PAUSED) {
			if (pci_channel_offline(ha->pdev))
				break;

			hccr = RD_REG_DWORD(&reg->hccr);

			qla_printk(KERN_INFO, ha, "RISC paused -- HCCR=%x, "
			    "Dumping firmware!\n", hccr);

			qla2xxx_check_risc_status(vha);

			ha->isp_ops->fw_dump(vha, 1);
			set_bit(ISP_ABORT_NEEDED, &vha->dpc_flags);
			break;
		} else if ((stat & HSRX_RISC_INT) == 0)
			break;

		switch (stat & 0xff) {
		case 0x1:
		case 0x2:
		case 0x10:
		case 0x11:
			qla24xx_mbx_completion(vha, MSW(stat));
			status |= MBX_INTERRUPT;

			break;
		case 0x12:
			mb[0] = MSW(stat);
			mb[1] = RD_REG_WORD(&reg->mailbox1);
			mb[2] = RD_REG_WORD(&reg->mailbox2);
			mb[3] = RD_REG_WORD(&reg->mailbox3);
			qla2x00_async_event(vha, rsp, mb);
			break;
		case 0x13:
		case 0x14:
			qla24xx_process_response_queue(vha, rsp);
			break;
		default:
			DEBUG2(printk("scsi(%ld): Unrecognized interrupt type "
			    "(%d).\n",
			    vha->host_no, stat & 0xff));
			break;
		}
		WRT_REG_DWORD(&reg->hccr, HCCRX_CLR_RISC_INT);
	} while (0);
	spin_unlock_irq(&ha->hardware_lock);

	if (test_bit(MBX_INTR_WAIT, &ha->mbx_cmd_flags) &&
	    (status & MBX_INTERRUPT) && ha->flags.mbox_int) {
		set_bit(MBX_INTERRUPT, &ha->mbx_cmd_flags);
		complete(&ha->mbx_intr_comp);
	}

	return IRQ_HANDLED;
}

/* Interrupt handling helpers. */

struct qla_init_msix_entry {
	const char *name;
	irq_handler_t handler;
};

static struct qla_init_msix_entry msix_entries[3] = {
	{ "qla2xxx (default)", qla24xx_msix_default },
	{ "qla2xxx (rsp_q)", qla24xx_msix_rsp_q },
	{ "qla2xxx (multiq)", qla25xx_msix_rsp_q },
};

static void
qla24xx_disable_msix(struct qla_hw_data *ha)
{
	int i;
	struct qla_msix_entry *qentry;

	for (i = 0; i < ha->msix_count; i++) {
		qentry = &ha->msix_entries[i];
		if (qentry->have_irq)
			free_irq(qentry->vector, qentry->rsp);
	}
	pci_disable_msix(ha->pdev);
	kfree(ha->msix_entries);
	ha->msix_entries = NULL;
	ha->flags.msix_enabled = 0;
}

static int
qla24xx_enable_msix(struct qla_hw_data *ha, struct rsp_que *rsp)
{
#define MIN_MSIX_COUNT	2
	int i, ret;
	struct msix_entry *entries;
	struct qla_msix_entry *qentry;

	entries = kzalloc(sizeof(struct msix_entry) * ha->msix_count,
					GFP_KERNEL);
	if (!entries)
		return -ENOMEM;

	for (i = 0; i < ha->msix_count; i++)
		entries[i].entry = i;

	ret = pci_enable_msix(ha->pdev, entries, ha->msix_count);
	if (ret) {
		if (ret < MIN_MSIX_COUNT)
			goto msix_failed;

		qla_printk(KERN_WARNING, ha,
			"MSI-X: Failed to enable support -- %d/%d\n"
			" Retry with %d vectors\n", ha->msix_count, ret, ret);
		ha->msix_count = ret;
		ret = pci_enable_msix(ha->pdev, entries, ha->msix_count);
		if (ret) {
msix_failed:
			qla_printk(KERN_WARNING, ha, "MSI-X: Failed to enable"
				" support, giving up -- %d/%d\n",
				ha->msix_count, ret);
			goto msix_out;
		}
		ha->max_rsp_queues = ha->msix_count - 1;
	}
	ha->msix_entries = kzalloc(sizeof(struct qla_msix_entry) *
				ha->msix_count, GFP_KERNEL);
	if (!ha->msix_entries) {
		ret = -ENOMEM;
		goto msix_out;
	}
	ha->flags.msix_enabled = 1;

	for (i = 0; i < ha->msix_count; i++) {
		qentry = &ha->msix_entries[i];
		qentry->vector = entries[i].vector;
		qentry->entry = entries[i].entry;
		qentry->have_irq = 0;
		qentry->rsp = NULL;
	}

	/* Enable MSI-X vectors for the base queue */
	for (i = 0; i < 2; i++) {
		qentry = &ha->msix_entries[i];
		ret = request_irq(qentry->vector, msix_entries[i].handler,
					0, msix_entries[i].name, rsp);
		if (ret) {
			qla_printk(KERN_WARNING, ha,
			"MSI-X: Unable to register handler -- %x/%d.\n",
			qentry->vector, ret);
			qla24xx_disable_msix(ha);
			ha->mqenable = 0;
			goto msix_out;
		}
		qentry->have_irq = 1;
		qentry->rsp = rsp;
		rsp->msix = qentry;
	}

	/* Enable MSI-X vector for response queue update for queue 0 */
	if (ha->mqiobase &&  (ha->max_rsp_queues > 1 || ha->max_req_queues > 1))
		ha->mqenable = 1;

msix_out:
	kfree(entries);
	return ret;
}

int
qla2x00_request_irqs(struct qla_hw_data *ha, struct rsp_que *rsp)
{
	int ret;
	device_reg_t __iomem *reg = ha->iobase;

	/* If possible, enable MSI-X. */
	if (!IS_QLA2432(ha) && !IS_QLA2532(ha) &&
	    !IS_QLA8432(ha) && !IS_QLA8001(ha))
		goto skip_msix;

	if (IS_QLA2432(ha) && (ha->pdev->revision < QLA_MSIX_CHIP_REV_24XX ||
		!QLA_MSIX_FW_MODE_1(ha->fw_attributes))) {
		DEBUG2(qla_printk(KERN_WARNING, ha,
		"MSI-X: Unsupported ISP2432 (0x%X, 0x%X).\n",
			ha->pdev->revision, ha->fw_attributes));

		goto skip_msix;
	}

	if (ha->pdev->subsystem_vendor == PCI_VENDOR_ID_HP &&
	    (ha->pdev->subsystem_device == 0x7040 ||
		ha->pdev->subsystem_device == 0x7041 ||
		ha->pdev->subsystem_device == 0x1705)) {
		DEBUG2(qla_printk(KERN_WARNING, ha,
		    "MSI-X: Unsupported ISP2432 SSVID/SSDID (0x%X, 0x%X).\n",
		    ha->pdev->subsystem_vendor,
		    ha->pdev->subsystem_device));

		goto skip_msi;
	}

	ret = qla24xx_enable_msix(ha, rsp);
	if (!ret) {
		DEBUG2(qla_printk(KERN_INFO, ha,
		    "MSI-X: Enabled (0x%X, 0x%X).\n", ha->chip_revision,
		    ha->fw_attributes));
		goto cleork_kpsr]
	add	%sp, STACKFRAME_SZ, %o2		! arg2:	pt_regs ptr
	mov	0, %o3
	call	sparc_do_fork
	 mov	%l5, %o7

	/* Whee, kernel threads! */
	.globl	sys_clone, flush_patch_three
sys_clone:
	mov	%o7, %l5
flush_patch_three:
	FLUSH_ALL_KERNEL_WINDOWS;
	ld	[%curptr + TI_TASK], %o4
	rd	%psr, %g4
	WRITE_PAUSE

	/* arg0,1: flags,usp  -- loaded already */
	cmp	%o1, 0x0			! Is new_usp NULL?
	rd	%wim, %g5
	WRITE_PAUSE
	be,a	1f
	 mov	%fp, %o1			! yes, use callers usp
	andn	%o1, 7, %o1			! no, align to 8 bytes
1:
	std	%g4, [%o4 + AOFF_task_thread + AOFF_thread_fork_kpsr]
	add	%sp, STACKFRAME_SZ, %o2		! arg2:	pt_regs ptr
	mov	0, %o3
	call	sparc_do_fork
	 mov	%l5, %o7

	/* Whee, real vfork! */
	.globl	sys_vfork, flush_patch_four
sys_vfork:
flush_patch_four:
	FLUSH_ALL_KERNEL_WINDOWS;
	ld	[%curptr + TI_TASK], %o4
	rd	%psr, %g4
	WRITE_PAUSE
	rd	%wim, %g5
	WRITE_PAUSE
	std	%g4, [%o4 + AOFF_task_thread + AOFF_thread_fork_kpsr]
	sethi	%hi(0x4000 | 0x0100 | SIGCHLD), %o0
	mov	%fp, %o1
	or	%o0, %lo(0x4000 | 0x0100 | SIGCHLD), %o0
	sethi	%hi(sparc_do_fork), %l1
	mov	0, %o3
	jmpl	%l1 + %lo(sparc_do_fork), %g0
	 add	%sp, STACKFRAME_SZ, %o2

        .align  4
linux_sparc_ni_syscall:
	sethi   %hi(sys_ni_syscall), %l7
	b       syscall_is_too_hard
	 or     %l7, %lo(sys_ni_syscall), %l7

linux_fast_syscall:
	andn	%l7, 3, %l7
	mov	%i0, %o0
	mov	%i1, %o1
	mov 	%i2, %o2
	jmpl	%l7 + %g0, %g0
	 mov	%i3, %o3

linux_syscall_trace:
	add	%sp, STACKFRAME_SZ, %o0
	call	syscall_trace
	 mov	0, %o1
	cmp	%o0, 0
	bne	3f
	 mov	-ENOSYS, %o0
	mov	%i0, %o0
	mov	%i1, %o1
	mov	%i2, %o2
	mov	%i3, %o3
	b	2f
	 mov	%i4, %o4

	.globl	ret_from_fork
ret_from_fork:
	call	schedule_tail
	 mov	%g3, %o0
	b	ret_sys_call
	 ld	[%sp + STACKFRAME_SZ + PT_I0], %o0

	/* Linux native system calls enter here... */
	.align	4
	.globl	linux_sparc_syscall
linux_sparc_syscall:
	sethi	%hi(PSR_SYSCALL), %l4
	or	%l0, %l4, %l0
	/* Direct access to user regs, must faster. */
	cmp	%g1, NR_SYSCALLS
	bgeu	linux_sparc_ni_syscall
	 sll	%g1, 2, %l4
	ld	[%l7 + %l4], %l7
	andcc	%l7, 1, %g0
	bne	linux_fast_syscall
	 /* Just do first insn from SAVE_ALL in the delay slot */

syscall_is_too_hard:
	SAVE_ALL_HEAD
	 rd	%wim, %l3

	wr	%l0, PSR_ET, %psr
	mov	%i0, %o0
	mov	%i1, %o1
	mov	%i2, %o2

	ld	[%curptr + TI_FLAGS], %l5
	mov	%i3, %o3
	andcc	%l5, _TIF_SYSCALL_TRACE, %g0
	mov	%i4, %o4
	bne	linux_syscall_trace
	 mov	%i0, %l5
2:
	call	%l7
	 mov	%i5, %o5

3:
	st	%o0, [%sp + STACKFRAME_SZ + PT_I0]

ret_sys_call:
	ld	[%curptr + TI_FLAGS], %l6
	cmp	%o0, -ERESTART_RESTARTBLOCK
	ld	[%sp + STACKFRAME_SZ + PT_PSR], %g3
	set	PSR_C, %g2
	bgeu	1f
	 andcc	%l6, _TIF_SYSCALL_TRACE, %g0

	/* System call success, clear Carry condition code. */
	andn	%g3, %g2, %g3
	clr	%l6
	st	%g3, [%sp + STACKFRAME_SZ + PT_PSR]	
	bne	linux_syscall_trace2
	 ld	[%sp + STACKFRAME_SZ + PT_NPC], %l1 /* pc = npc */
	add	%l1, 0x4, %l2			/* npc = npc+4 */
	st	%l1, [%sp + STACKFRAME_SZ + PT_PC]
	b	ret_trap_entry
	 st	%l2, [%sp + STACKFRAME_SZ + PT_NPC]
1:
	/* System call failure, set Carry condition code.
	 * Also, get abs(errno) to return to the process.
	 */
	sub	%g0, %o0, %o0
	or	%g3, %g2, %g3
	st	%o0, [%sp + STACKFRAME_SZ + PT_I0]
	mov	1, %l6
	st	%g3, [%sp + STACKFRAME_SZ + PT_PSR]
	bne	linux_syscall_trace2
	 ld	[%sp + STACKFRAME_SZ + PT_NPC], %l1 /* pc = npc */
	add	%l1, 0x4, %l2			/* npc = npc+4 */
	st	%l1, [%sp + STACKFRAME_SZ + PT_PC]
	b	ret_trap_entry
	 st	%l2, [%sp + STACKFRAME_SZ + PT_NPC]

linux_syscall_trace2:
	add	%sp, STACKFRAME_SZ, %o0
	mov	1, %o1
	call	syscall_trace
	 add	%l1, 0x4, %l2			/* npc = npc+4 */
	st	%l1, [%sp + STACKFRAME_SZ + PT_PC]
	b	ret_trap_entry
	 st	%l2, [%sp + STACKFRAME_SZ + PT_NPC]


/* Saving and restoring the FPU state is best done from lowlevel code.
 *
 * void fpsave(unsigned long *fpregs, unsigned long *fsr,
 *             void *fpqueue, unsigned long *fpqdepth)
 */

	.globl	fpsave
fpsave:
	st	%fsr, [%o1]	! this can trap on us if fpu is in bogon state
	ld	[%o1], %g1
	set	0x2000, %g4
	andcc	%g1, %g4, %g0
	be	2f
	 mov	0, %g2

	/* We have an fpqueue to save. */
1:
	std	%fq, [%o2]
fpsave_magic:
	st	%fsr, [%o1]
	ld	[%o1], %g3o $Àao  ¡Mq  ¿vo   ¿Çu  ¿b !√ 0  ¡œf  ¡á  ¡Ωk! )¿Lo  ¡8q  ¿b(C  ¡A  ¿˝u ¡Îu  ¡œÉ  ¡#q  ¡S   √ÃÉ  ¡M$  ¡qv  ¡x   ¿%x   ¿ÙE`¡Äk# !¡:x  ¡ë  ¡Ox  ¡ !¡÷u   ¿dx   ¿∏x  ¡Õx  ¡‚x   ¿ıx   À|  ¡)|  ¡ !¡  !Àï  ¡õJ  ¡	  !ÀR  ¡&A  ¡1? 0√PN  ¡R  ¡–-B  ÀMN  ¡⁄VÉ  ¡Ω  ¿Ä  √û  ¡(C  ¡_ !√Ç;  ¡Ù  !ÀÏVÉ  ¡˚c  √ìk# !¡≥  ¡†o  ¡ÎpC  ¡}$  ¡cÉ  √ó;  ¿òE ¡â   ¡<  ¡	YÉ !¡}  √·c  ¡Ç# $Àµo@  ¿q   ¿V? 0¡YÉ !¡.u  √á0  ¡Mh  ¡ÖV  √   ¡,(C  ¡ !¡Ë  !Àá$  ¡Ò-C  Àm? 0√Œ)C"√¥E`¡ER  √àV  √ê0  ¡í   ¡e  √≈)C"√∏;  ¡‰J  ¡? 0√\  √%N  ¡ë !√-C  ¡7Ç  ¡7Ç  ¡d    Àp? 0√°E ¡€J  ¡ú$  ¿HR  √öV  ¡H;  ¡-C  ¡0É  √  !ÀkA  ¡h  ¡-É  √Øc  ¡s !¡oR  ¡hh  ¡È)C"√D  ¡qh  ¡Ê)C"√ 7É  √t  ¡∆J  √ão  ¡"-C  ¡√J  √rR  ¡Gf  ¡å> 0√OE  ¡   ÀÙoC  ¡Ò %√Ω$  ¡Ó %√â> 0√É  ¡πB  √TYÅ !¡EE  ¡WYÅ !¡/K  ¡∂B  √m]  ¿É  ¡æ> 0¡âB  ¡éc  ¿	h  ¡ oC  ¡¨pA  √≥g  ¡W  ¡¡pA  √x;  ¡É  ¡÷pA  ¡Z  ¡ù !…ﬂoC  ¡ÇpB  ¡˛pB  À     Àu  ¡u  ¡Xu  ¡mu  ¬¡u  ¡–  ¡Œ %√eE  ¡H7Ç  √K7Ç  √	;  ¡,    Àyc  ¡  ¡‚Ç  ¡ÂÇ  ¡⁄  ¡Ô$  ¡≠)C"¿R-B  ¡;  ¡g !√  ¡ÑYÅ !√© !¡áYÅ !√E  ¡¨ !¡B]  ¡oc  ¡gK  ¡]]  ¡™g  ¡l !¡mpB  ¿ópB  ¡E  ¡;l  !¿∞t 0√Cu  ¿%  ¡”  …jK  ¡p !√(;  ¿˚> 0√ÛB  √O !¡A)C  ¡™N  ¡˛> 0√%  ¡≠N  ¡ég  ¡U3  ¡„> 0¡CpC"¬Ô  ¿&l  !¿Üt 0¡GK  √˘YÉ  √éx  ¡£x   ¿P)B ¿$E  ¿B{  √W{   ¿ΩÉ  ¡> 1¡?c  ¡Y)B ¿—:Ç  √ÉN  ¡> 1¡Pl !¡¯ !¡‘:Ç  √‚   ÀˆYÉ  √Ìg  ¡ !ÀÄ  ¿è7É  ¡ B  ¡PK  √?V  √∑7É  √ŒYÉ  ¡z3  ¡Ô !¡2V  ¡ !À| !ÀÇ   À.pB"¿XpB"¡õt 0¡zl !¡Ôt   ¿∫7É  √˙F  ¡{  ¡ﬂ !¡c  ¿Y%  ¡y)B ¿áÇ  ¡ƒF  ¡ c  ¡Ñ   Àøh  ¿{  ¿æK  √∞h  ¡°  ¡
c  ¡Ó]  √B  ¡Ò]  √äÇ  ¡ !√O%  ¡
B  ¡3  √µK  √FO  ¡ !¡X !À£  À !¡CO  ¡üQ  ¡¢Q  ¡ŸF  ¿el !¡/g  ¡!# )¡Di   ¿")C  ¡	pC  ¡≈t 0√äÉ ¡XÇ  ¡*# )¡ä:É  √ùK  ¡pB"¿è  ¡n%  ¿+)C  ¡nB  ¡èl !¡K   ÀÎ !À¥   …XÇ  ¡◊]  ¡Ö  ¡Yi  ¡⁄t  1¿∫F  ¡-{  ¡l{  ¡¿{  ¡≤l !À8)C  ¡"3  √>|   ¿2t 1¿S|  ¡¥.  ¿ÉF`¡h|  ¡}|  √’{  ¿ÜF`¡qXÇ  ¡ni   ¡€3  √¢É √§:Ç  ¡k  ¡C!   ÀÌ7É  ¡•É √# )¡§  ¿°:Ç  ¡a> 0¡ﬂÉ √O  ¡a  ¡ÌQ  ¡tXÇ  ¡≤]  √é%  ¡˜`  ¡¢l  !À’l !À
t 0¡t 0¡Ö]  ¡Í{  ¿ˇ{  ¿[B  ¡ï.  ¡B:Ç  ¡í|  √d!  Àæ%  ¡˚3  √ü.  ¡¯3  √gF`¡É  ¡÷K  ¡ŸK  ¡≈l  !À)6É  ¡ß|   ¿©`  ¡ÏO  √~! )¡Å{  ¡ÈO  √≈  ¡u:É  ¡⁄  ¡ª !¡˜U  ¡Sg  ¡JXÉ  ¡>6É  √º`  ¡Jg  ¡i  ¡¯l !À›%  ¿ !   À/i   ¿qt  1¿º|  ¡—|   ¿)É  ¡H# )¡à !¡,É  ¡ã !¡ÍU  ¡î3  √!   À“U  ¡ë3  √„  ¡:É  ¡ïC  ¡ÌU  ¡»%  ¡Q  √ÆXÄ  ¿!  Àñd  √Ël  !Àm  !ÀX6Ç  ¡Gt 1√œi  ¡?   À˛%  ¡Q  √U`  ¡\t 1¡ñ{  √_ !…’.  ¡¯C  ¡≈U  ¡U6Ç  ¡˚C  ¡U\  ¡X`  ¡ôd  √m !À› !¡ŒsÇ  ¡´{  ¿ﬁ|  ¡Û|  ¿2}  ¿G}  ¿Á !¡  ¡\}  ¿i}   ¿¿= 1¡?:Ç  ¡¿  ¡wQ  ¡n`  ¡#É  ¡√  ¡£3  √':É  √Å! )¡‡C  √πO  ¡ºO  ¡˙XÉ !¡4\  ¿·iÉ  ¡¯s  ¡   ¡¸ !¿Û  ¡0.  ¡KzÅ !¡√!  À0m %√Ÿ= 1¡uzÄ  ¿~}   ¿Ãd  √F`¡\É  ¡Ä  ¡5  ¿€*C  ¡&.  ¡Z2  ¡ã6É  ¡‚9Ç  ¡,= 1¡/= 1¡GÉ  √«C  ¡F`¡Ì!   ÀˆG  √ëO  ¡AQ  ¡\  ¡çU †√î   ¿g2  √JÉ  √DQ  ¡†6É  √\  ¡  !À·!   À%`  √H  ¡Êd  √Éi   ¿ü  √òi  ¡ÙiÉ  ¡
= 1¡Em !À«9É  √©   ¿xÉ ¡» !¡U  ¡á  ¡î!  …	  u 0
 
 ∏guâ´Õs  ˘ û p 8–¸Ó  8—˝t  –';ê¸π8–Ã/8–¸   ¿A ÙÑ*ê‰I;—˝ö;—˝Ó;—˝8êÃ+ÄÙ*;ê¸~;ê¸á;ê¸À†Ùa+ÄÙl;ê¸I8–¸ó9—˝§8–Ï•8êÃ˜  É  ¿28–¸@8–¸8—Ìö9—˝À;—˝o;ê¸ò†ÙÓ	†‘gÄÙ‘;—˝∑	 –…	 Ù‹	 Ù
;ê¸(
8–‹}†ÙoÄÙÌkÄÙÂ	†‘;ê¸Ω8–Ì;
;–ÏI
8–ÌY
8—˝Ó+ÄÙb†ÙÓ	ÄÙL8ê¸è†Ù_	 ÙÄ†Ù∫;—˝†ÙÉ
;—˝c;–¸Ω;—˝ò
;—˝≠
8ê‹¬
kÄÙâÄÙ$+ÄÙJ8—˝‘;—˝ç	 –ˇ8–¸¡;êÏÔ8–Ìº;—˝ÄÙ≤†Ù˛	+ÄÙ˘
 Ù^+ÄÙp;–ÏŸ;ê¸≠;ê‰;—˝h;—˝π;—˝;—˝ø+ÄÙ≈  –>+ÄÙ3†Ù‰
   À;—˝n
9—˝  ¿\;ê¸B;ê¸e;—˝8ê‹ó;—˝h9—˝°;ê¸◊
†ÙÛ;—˝+ÄÙe9—˝–8–‹¢	  ¿[ÄÙ	+ÄÙß+ÄÙ9;ê¸
  í 	b 	∂ 	‹ 	Î 	ø 	  Q â 	k 	
  ;#D#† "m &ó "q ƒ "  d &µ "‘ "â  Î #	  y É	  @XÖ   P]d Ä| Å´ÅI Ä{  ‘Å8ÅVÅ  Åø ÅÀÅR Ä_Å› Å∂ Å)6C
  ò É ÄY Ä∑ ÉÅ Ç 	  tBãC@» N@	  x¢RÅüœ    Î†G+ DM†Gô†C›†CŸ @˚†C  @n  @÷†C!†E\†AŒ†D{†Gê†GÜ  @F†@r†E‰†E`†GÎ†C!†GW†Ec†C†C††C†E=†GY  @†A(†GUÄ 	  { S	  5] 
 	 j M  Oö OÛ M2MGOR L LT 
  R g ˛ ì I   	  íú N	  P°‡G|Ì 	  Y¢¨ 	  u 9
  ] › ñ ´ ≈ Q  i ‘ Ò P¬ 	  GP}  @ Lx_x6x\Ñë  ¥x/xÃ∑x£xxx†x].xAxÿ xQxfxs -8àMà¬mxÃ ùt≤  xÎ âx/x‰H8”x xv÷xyˆø Qx	xÔ™ 8x, xàx–xÙ ”xÖx/•·
 ‡ ∑ 	:"	Ω	&%	%	H%	]%	 Õ	3		Ì	S	c	≤!	"	Ã	⁄	2	=#	ë$	j%	®	s 	p	L!	%	Â%	û$	Ú%	•!	˙		"	(#	&	&	7	ﬂ	Ô	&		ˇ	
	#À	u	 	>"		 	¸		a!	n!	 	5`	Í	‰"	˘"	ÿ		,	#	#			≥$	›		©	…	i	!	¬"	/ 	Ù#	 v		¿$	÷	Ï	∂	!	’$	å%	°%	»	 < 	◊"	G	>	Á#	˜	$	≠	#		ª	á	€	Î	ƒ 	{	?!	1	†"	f	T	∫	0	¶	∆	î	Ÿ 	¯	*!	µ"	≈#	“#	$	‚$	˜$	ã	%	5	ø	Ö	E	+$	ô	e	‘!	Ê 	Æ%	Y	˝	∞#	™	D	R	ò	Ñ	í	q"	 	˚ 	8$	Ë	«!	£#	ì"	r	~"	%	;%	≤	ó	ù	ß	g	W	â	w	ˆ!	Ä 	C	ﬁO"	M$	à	¥	√%	˛	B	t	Ç	È!	é#	ñ	Z$	–%	Ò	b	P	ï 	\"	œ	Ûl#	Å#	N	õ	´	!	U	.	y	A	¢ 	π	ê!	Ó	-"	§	‹	_#	∏	Q 	d	o$	^ 	Ü	É!	J#	@	|$	·		  0@s	  P	sM	õ 	  X{ 	  <≈   êì<ë¸ ì•ìì≤  • ì êY êÿ ìq ë{ë 4 C]B°¬Ï¬º:Ñ%∫  € ÄVÇ…∫≥:ú:ı∫“ Äi;(∫Õ¬@˘  ∫ Ä:;É∫  wö÷¬π¬ôB§
ﬁ∫± Än∫ã¬Q∫º¬⁄ä
j∫:ÄÑZ∫F£∫+U:¯∫rC∫@∫O	  _á 9\	  Dè gA	  J88S8o:§ Z:	  0J0S0ÿ2Z2É2Œ 
   ¿¬~ ¿ª ¬ ä ¿ê ¬
  l8w3∂7…7è 9÷Q 0  h 9Å 1Ö 1Æ ;8?;?ò 9X 1À 3A 0~ 1• ;!?a8b 97$?¬ 3 ( ÑÑ6´≈2˛Á3|  6*Á7RÂ6}Á3  0’Â6Ä