cmd_type);
		switch (type) {
		case LPFC_SOL_IOCB:
			spin_unlock_irqrestore(&phba->hbalock, iflag);
			rc = lpfc_sli_process_sol_iocb(phba, pring, saveq);
			spin_lock_irqsave(&phba->hbalock, iflag);
			break;

		case LPFC_UNSOL_IOCB:
			spin_unlock_irqrestore(&phba->hbalock, iflag);
			rc = lpfc_sli_process_unsol_iocb(phba, pring, saveq);
			spin_lock_irqsave(&phba->hbalock, iflag);
			if (!rc)
				free_saveq = 0;
			break;

		case LPFC_ABORT_IOCB:
			cmdiocbp = NULL;
			if (irsp->ulpCommand != CMD_XRI_ABORTED_CX)
				cmdiocbp = lpfc_sli_iocbq_lookup(phba, pring,
								 saveq);
			if (cmdiocbp) {
				/* Call the specified completion routine */
				if (cmdiocbp->iocb_cmpl) {
					spin_unlock_irqrestore(&phba->hbalock,
							       iflag);
					(cmdiocbp->iocb_cmpl)(phba, cmdiocbp,
							      saveq);
					spin_lock_irqsave(&phba->hbalock,
							  iflag);
				} else
					__lpfc_sli_release_iocbq(phba,
								 cmdiocbp);
			}
			break;

		case LPFC_UNKNOWN_IOCB:
			if (irsp->ulpCommand == CMD_ADAPTER_MSG) {
				char adaptermsg[LPFC_MAX_ADPTMSG];
				memset(adaptermsg, 0, LPFC_MAX_ADPTMSG);
				memcpy(&adaptermsg[0], (uint8_t *)irsp,
				       MAX_MSG_DATA);
				dev_warn(&((phba->pcidev)->dev),
					 "lpfc%d: %s\n",
					 phba->brd_no, adaptermsg);
			} else {
				/* Unknown IOCB command */
				lpfc_printf_log(phba, KERN_ERR, LOG_SLI,
						"0335 Unknown IOCB "
						"command Data: x%x "
						"x%x x%x x%x\n",
						irsp->ulpCommand,
						irsp->ulpStatus,
						irsp->ulpIoTag,
						irsp->ulpContext);
			}
			break;
		}

		if (free_saveq) {
			list_for_each_entry_safe(rspiocbp, next_iocb,
						 &saveq->list, list) {
				list_del(&rspiocbp->list);
				__lpfc_sli_release_iocbq(phba, rspiocbp);
			}
			__lpfc_sli_release_iocbq(phba, saveq);
		}
		rspiocbp = NULL;
	}
	spin_unlock_irqrestore(&phba->hbalock, iflag);
	return rspiocbp;
}

/**
 * lpfc_sli_handle_slow_ring_event - Wrapper func for handling slow-path iocbs
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @mask: Host attention register mask for this ring.
 *
 * This routine wraps the actual slow_ring event process routine from the
 * API jump table function pointer from the lpfc_hba struct.
 **/
void
lpfc_sli_handle_slow_ring_event(struct lpfc_hba *phba,
				struct lpfc_sli_ring *pring, uint32_t mask)
{
	phba->lpfc_sli_handle_slow_ring_event(phba, pring, mask);
}

/**
 * lpfc_sli_handle_slow_ring_event_s3 - Handle SLI3 ring event for non-FCP rings
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @mask: Host attention register mask for this ring.
 *
 * This function is called from the worker thread when there is a ring event
 * for non-fcp rings. The caller does not hold any lock. The function will
 * remove each response iocb in the response ring and calls the handle
 * response iocb routine (lpfc_sli_sp_handle_rspiocb) to process it.
 **/
static void
lpfc_sli_handle_slow_ring_event_s3(struct lpfc_hba *phba,
				   struct lpfc_sli_ring *pring, uint32_t mask)
{
	struct lpfc_pgp *pgp;
	IOCB_t *entry;
	IOCB_t *irsp = NULL;
	struct lpfc_iocbq *rspiocbp = NULL;
	uint32_t portRspPut, portRspMax;
	unsigned long iflag;
	uint32_t status;

	pgp = &phba->port_gp[pring->ringno];
	spin_lock_irqsave(&phba->hbalock, iflag);
	pring->stats.iocb_event++;

	/*
	 * The next available response entry should never exceed the maximum
	 * entries.  If it does, treat it as an adapter hardware error.
	 */
	portRspMax = pring->numRiocb;
	portRspPut = le32_to_cpu(pgp->rspPutInx);
	if (portRspPut >= portRspMax) {
		/*
		 * Ring <ringno> handler: portRspPut <portRspPut> is bigger than
		 * rsp ring <portRspMax>
		 */
		lpfc_printf_log(phba, KERN_ERR, LOG_SLI,
				"0303 Ring %d handler: portRspPut %d "
				"is bigger than rsp ring %d\n",
				pring->ringno, portRspPut, portRspMax);

		phba->link_state = LPFC_HBA_ERROR;
		spin_unlock_irqrestore(&phba->hbalock, iflag);

		phba->work_hs = HS_FFER3;
		lpfc_handle_eratt(phba);

		return;
	}

	rmb();
	while (pring->rspidx != portRspPut) {
		/*
		 * Build a completion list and call the appropriate handler.
		 * The process is to get the next available response iocb, get
		 * a free iocb from the list, copy the response data into the
		 * free iocb, insert to the continuation list, and update the
		 * next response index to slim.  This process makes response
		 * iocb's in the ring available to DMA as fast as possible but
		 * pays a penalty for a copy operation.  Since the iocb is
		 * only 32 bytes, this penalty is considered small relative to
		 * the PCI reads for register values and a slim write.  When
		 * the ulpLe field is set, the entire Command has been
		 * received.
		 */
		entry = lpfc_resp_iocb(phba, pring);

		phba->last_completion_time = jiffies;
		rspiocbp = __lpfc_sli_get_iocbq(phba);
		if (rspiocbp == NULL) {
			printk(KERN_ERR "%s: out of buffers! Failing "
			       "completion.\n", __func__);
			break;
		}

		lpfc_sli_pcimem_bcopy(entry, &rspiocbp->iocb,
				      phba->iocb_rsp_size);
		irsp = &rspiocbp->iocb;

		if (++pring->rspidx >= portRspMax)
			pring->rspidx = 0;

		if (pring->ringno == LPFC_ELS_RING) {
			lpfc_debugfs_slow_ring_trc(phba,
			"IOCB rsp ring:   wd4:x%08x wd6:x%08x wd7:x%08x",
				*(((uint32_t *) irsp) + 4),
				*(((uint32_t *) irsp) + 6),
				*(((uint32_t *) irsp) + 7));
		}

		writel(pring->rspidx, &phba->host_gp[pring->ringno].rspGetInx);

		spin_unlock_irqrestore(&phba->hbalock, iflag);
		/* Handle the response IOCB */
		rspiocbp = lpfc_sli_sp_handle_rspiocb(phba, pring, rspiocbp);
		spin_lock_irqsave(&phba->hbalock, iflag);

		/*
		 * If the port response put pointer has not been updated, sync
		 * the pgp->rspPutInx in the MAILBOX_tand fetch the new port
		 * response put pointer.
		 */
		if (pring->rspidx == portRspPut) {
			portRspPut = le32_to_cpu(pgp->rspPutInx);
		}
	} /* while (pring->rspidx != portRspPut) */

	if ((rspiocbp != NULL) && (mask & HA_R0RE_REQ)) {
		/* At least one response entry has been freed */
		pring->stats.iocb_rsp_full++;
		/* SET RxRE_RSP in Chip Att register */
		status = ((CA_R0ATT | CA_R0RE_RSP) << (pring->ringno * 4));
		writel(status, phba->CAregaddr);
		readl(phba->CAregaddr); /* flush */
	}
	if ((mask & HA_R0CE_RSP) && (pring->flag & LPFC_CALL_RING_AVAILABLE)) {
		pring->flag &= ~LPFC_CALL_RING_AVAILABLE;
		pring->stats.iocb_cmd_empty++;

		/* Force update of the local copy of cmdGetInx */
		pring->local_getidx = le32_to_cpu(pgp->cmdGetInx);
		lpfc_sli_resume_iocb(phba, pring);

		if ((pring->lpfc_sli_cmd_available))
			(pring->lpfc_sli_cmd_available) (phba, pring);

	}

	spin_unlock_irqrestore(&phba->hbalock, iflag);
	return;
}

/**
 * lpfc_sli_handle_slow_ring_event_s4 - Handle SLI4 slow-path els events
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @mask: Host attention register mask for this ring.
 *
 * This function is called from the worker thread when there is a pending
 * ELS response iocb on the driver internal slow-path response iocb worker
 * queue. The caller does not hold any lock. The function will remove each
 * response iocb from the response worker queue and calls the handle
 * response iocb routine (lpfc_sli_sp_handle_rspiocb) to process it.
 **/
static void
lpfc_sli_handle_slow_ring_event_s4(struct lpfc_hba *phba,
				   struct lpfc_sli_ring *pring, uint32_t mask)
{
	struct lpfc_iocbq *irspiocbq;
	unsigned long iflag;

	while (!list_empty(&phba->sli4_hba.sp_rspiocb_work_queue)) {
		/* Get the response iocb from the head of work queue */
		spin_lock_irqsave(&phba->hbalock, iflag);
		list_remove_head(&phba->sli4_hba.sp_rspiocb_work_queue,
				 irspiocbq, struct lpfc_iocbq, list);
		spin_unlock_irqrestore(&phba->hbalock, iflag);
		/* Process the response iocb */
		lpfc_sli_sp_handle_rspiocb(phba, pring, irspiocbq);
	}
}

/**
 * lpfc_sli_abort_iocb_ring - Abort all iocbs in the ring
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 *
 * This function aborts all iocbs in the given ring and frees all the iocb
 * objects in txq. This function issues an abort iocb for all the iocb commands
 * in txcmplq. The iocbs in the txcmplq is not guaranteed to complete before
 * the return of this function. The caller is not required to hold any locks.
 **/
void
lpfc_sli_abort_iocb_ring(struct lpfc_hba *phba, struct lpfc_sli_ring *pring)
{
	LIST_HEAD(completions);
	struct lpfc_iocbq *iocb, *next_iocb;

	if (pring->ringno == LPFC_ELS_RING) {
		lpfc_fabric_abort_hba(phba);
	}

	/* Error everything on txq and txcmplq
	 * First do the txq.
	 */
	spin_lock_irq(&phba->hbalock);
	list_splice_init(&pring->txq, &completions);
	pring->txq_cnt = 0;

	/* Next issue ABTS for everything on the txcmplq */
	list_for_each_entry_safe(iocb, next_iocb, &pring->txcmplq, list)
		lpfc_sli_issue_abort_iotag(phba, pring, iocb);

	spin_unlock_irq(&phba->hbalock);

	/* Cancel all the IOCBs from the completions list */
	lpfc_sli_cancel_iocbs(phba, &completions, IOSTAT_LOCAL_REJECT,
			      IOERR_SLI_ABORTED);
}

/**
 * lpfc_sli_flush_fcp_rings - flush all iocbs in the fcp ring
 * @phba: Pointer to HBA context object.
 *
 * This function flushes all iocbs in the fcp ring and frees all the iocb
 * objects in txq and txcmplq. This function will not issue abort iocbs
 * for all the iocb commands in txcmplq, they will just be returned with
 * IOERR_SLI_DOWN. This function is invoked with EEH when device's PCI
 * slot has been permanently disabled.
 **/
void
lpfc_sli_flush_fcp_rings(struct lpfc_hba *phba)
{
	LIST_HEAD(txq);
	LIST_HEAD(txcmplq);
	struct lpfc_sli *psli = &phba->sli;
	struct lpfc_sli_ring  *pring;

	/* Currently, only one fcp ring */
	pring = &psli->ring[psli->fcp_ring];

	spin_lock_irq(&phba->hbalock);
	/* Retrieve everything on txq */
	list_splice_init(&pring->txq, &txq);
	pring->txq_cnt = 0;

	/* Retrieve everything on the txcmplq */
	list_splice_init(&pring->txcmplq, &txcmplq);
	pring->txcmplq_cnt = 0;
	spin_unlock_irq(&phba->hbalock);

	/* Flush the txq */
	lpfc_sli_cancel_iocbs(phba, &txq, IOSTAT_LOCAL_REJECT,
			      IOERR_SLI_DOWN);

	/* Flush the txcmpq */
	lpfc_sli_cancel_iocbs(phba, &txcmplq, IOSTAT_LOCAL_REJECT,
			      IOERR_SLI_DOWN);
}

/**
 * lpfc_sli_brdready_s3 - Check for sli3 host ready status
 * @phba: Pointer to HBA context object.
 * @mask: Bit mask to be checked.
 *
 * This function reads the host status register and compares
 * with the provided bit mask to check if HBA completed
 * the restart. This function will wait in a loop for the
 * HBA to complete restart. If the HBA does not restart within
 * 15 iterations, the function will reset the HBA again. The
 * function returns 1 when HBA fail to restart otherwise returns
 * zero.
 **/
static int
lpfc_sli_brdready_s3(struct lpfc_hba *phba, uint32_t mask)
{
	uint32_t status;
	int i = 0;
	int retval = 0;

	/* Read the HBA Host Status Register */
	status = readl(phba->HSregaddr);

	/*
	 * Check status register every 100ms for 5 retries, then every
	 * 500ms for 5, then every 2.5 sec for 5, then reset board and
	 * every 2.5 sec for 4.
	 * Break our of the loop if errors occurred during init.
	 */
	while (((status & mask) != mask) &&
	       !(status & HS_FFERM) &&
	       i++ < 20) {

		if (i <= 5)
			msleep(10);
		else if (i <= 10)
			msleep(500);
		else
			msleep(2500);

		if (i == 15) {
				/* Do post */
			phba->pport->port_state = LPFC_VPORT_UNKNOWN;
			lpfc_sli_brdrestart(phba);
		}
		/* Read the HBA Host Status Register */
		status = readl(phba->HSregaddr);
	}

	/* Check to see if any errors occurred during init */
	if ((status & HS_FFERM) || (i >= 20)) {
		phba->link_state = LPFC_HBA_ERROR;
		retval = 1;
	}

	return retval;
}

/**
 * lpfc_sli_brdready_s4 - Check for sli4 host ready status
 * @phba: Pointer to HBA context object.
 * @mask: Bit mask to be checked.
 *
 * This function checks the host status register to check if HBA is
 * ready. This function will wait in a loop for the HBA to be ready
 * If the HBA is not ready , the function will will reset the HBA PCI
 * function again. The function returns 1 when HBA fail to be ready
 * otherwise returns zero.
 **/
static int
lpfc_sli_brdready_s4(struct lpfc_hba *phba, uint32_t mask)
{
	uint32_t status;
	int retval = 0;

	/* Read the HBA Host Status Register */
	status = lpfc_sli4_post_status_check(phba);

	if (status) {
		phba->pport->port_state = LPFC_VPORT_UNKNOWN;
		lpfc_sli_brdrestart(phba);
		status = lpfc_sli4_post_status_check(phba);
	}

	/* Check to see if any errors occurred during init */
	if (status) {
		phba->link_state = LPFC_HBA_ERROR;
		retval = 1;
	} else
		phba->sli4_hba.intr_enable = 0;

	return retval;
}

/**
 * lpfc_sli_brdready - Wrapper func for checking the hba readyness
 * @phba: Pointer to HBA context object.
 * @mask: Bit mask to be checked.
 *
 * This routine wraps the actual SLI3 or SLI4 hba readyness check routine
 * from the API jump table function pointer from the lpfc_hba struct.
 **/
int
lpfc_sli_brdready(struct lpfc_hba *phba, uint32_t mask)
{
	return phba->lpfc_sli_brdready(phba, mask);
}

#define BARRIER_TEST_PATTERN (0xdeadbeef)

/**
 * lpfc_reset_barrier - Make HBA ready for HBA reset
 * @phba: Pointer to HBA context object.
 *
 * This function is called before resetting an HBA. This
 * function requests HBA to quiesce DMAs before a reset.
 **/
void lpfc_reset_barrier(struct lpfc_hba *phba)
{
	uint32_t __iomem *resp_buf;
	uint32_t __iomem *mbox_buf;
	volatile uint32_t mbox;
	uint32_t hc_copy;
	int  i;
	uint8_t hdrtype;

	pci_read_config_byte(phba->pcidev, PCI_HEADER_TYPE, &hdrtype);
	if (hdrtype != 0x80 ||
	    (FC_JEDEC_ID(phba->vpd.rev.biuRev) != HELIOS_JEDEC_ID &&
	     FC_JEDEC_ID(phba->vpd.rev.biuRev) != THOR_JEDEC_ID))
		return;

	/*
	 * Tell the other part of the chip to suspend temporarily all
	 * its DMA activity.
	 */
	resp_buf = phba->MBslimaddr;

	/* Disable the error attention */
	hc_copy = readl(phba->HCregaddr);
	writel((hc_copy & ~HC_ERINT_ENA), phba->HCregaddr);
	readl(phba->HCregaddr); /* flush */
	phba->link_flag |= LS_IGNORE_ERATT;

	if (readl(phba->HAregaddr) & HA_ERATT) {
		/* Clear Chip error bit */
		writel(HA_ERATT, phba->HAregaddr);
		phba->pport->stopped = 1;
	}

	mbox = 0;
	((MAILBOX_t *)&mbox)->mbxCommand = MBX_KILL_BOARD;
	((MAILBOX_t *)&mbox)->mbxOwner = OWN_CHIP;

	writel(BARRIER_TEST_PATTERN, (resp_buf + 1));
	mbox_buf = phba->MBslimaddr;
	writel(mbox, mbox_buf);

	for (i = 0;
	     readl(resp_buf + 1) != ~(BARRIER_TEST_PATTERN) && i < 50; i++)
		mdelay(1);

	if (readl(resp_buf + 1) != ~(BARRIER_TEST_PATTERN)) {
		if (phba->sli.sli_flag & LPFC_SLI_ACTIVE ||
		    phba->pport->stopped)
			goto restore_hc;
		else
			goto clear_errat;
	}

	((MAILBOX_t *)&mbox)->mbxOwner = OWN_HOST;
	for (i = 0; readl(resp_buf) != mbox &&  i < 500; i++)
		mdelay(1);

clear_errat:

	while (!(readl(phba->HAregaddr) & HA_ERATT) && ++i < 500)
		mdelay(1);

	if (readl(phba->HAregaddr) & HA_ERATT) {
		writel(HA_ERATT, phba->HAregaddr);
		phba->pport->stopped = 1;
	}

restore_hc:
	phba->link_flag &= ~LS_IGNORE_ERATT;
	writel(hc_copy, phba->HCregaddr);
	readl(phba->HCregaddr); /* flush */
}

/**
 * lpfc_sli_brdkill - Issue a kill_board mailbox command
 * @phba: Pointer to HBA context object.
 *
 * This function issues a kill_board mailbox command and waits for
 * the error attention interrupt. This function is called for stopping
 * the firmware processing. The caller is not required to hold any
 * locks. This function calls lpfc_hba_down_post function to free
 * any pending commands after the kill. The function will return 1 when it
 * fails to kill the board else will return 0.
 **/
int
lpfc_sli_brdkill(struct lpfc_hba *phba)
{
	struct lpfc_sli *psli;
	LPFC_MBOXQ_t *pmb;
	uint32_t status;
	uint32_t ha_copy;
	int retval;
	int i = 0;

	psli = &phba->sli;

	/* Kill HBA */
	lpfc_printf_log(phba, KERN_INFO, LOG_SLI,
			"0329 Kill HBA Data: x%x x%x\n",
			phba->pport->port_state, psli->sli_flag);

	pmb = (LPFC_MBOXQ_t *) mempool_alloc(phba->mbox_mem_pool, GFP_KERNEL);
	if (!pmb)
		return 1;

	/* Disable the error attention */
	spin_lock_irq(&phba->hbalock);
	status = readl(phba->HCregaddr);
	status &= ~HC_ERINT_ENA;
	writel(status, phba->HCregaddr);
	readl(phba->HCregaddr); /* flush */
	phba->link_flag |= LS_IGNORE_ERATT;
	spin_unlock_irq(&phba->hbalock);

	lpfc_kill_board(phba, pmb);
	pmb->mbox_cmpl = lpfc_sli_def_mbox_cmpl;
	retval = lpfc_sli_issue_mbox(phba, pmb, MBX_NOWAIT);

	if (retval != MBX_SUCCESS) {
		if (retval != MBX_BUSY)
			mempool_free(pmb, phba->mbox_mem_pool);
		spin_lock_irq(&phba->hbalock);
		phba->link_flag &= ~LS_IGNORE_ERATT;
		spin_unlock_irq(&phba->hbalock);
		return 1;
	}

	spin_lock_irq(&phba->hbalock);
	psli->sli_flag &= ~LPFC_SLI_ACTIVE;
	spin_unlock_irq(&phba->hbalock);

	mempool_free(pmb, phba->mbox_mem_pool);

	/* There is no completion for a KILL_BOARD mbox cmd. Check for an error
	 * attention every 100ms for 3 seconds. If we don't get ERATT after
	 * 3 seconds we still set HBA_ERROR state because the status of the
	 * board is now undefined.
	 */
	ha_copy = readl(phba->HAregaddr);

	while ((i++ < 30) && !(ha_copy & HA_ERATT)) {
		mdelay(100);
		ha_copy = readl(phba->HAregaddr);
	}

	del_timer_sync(&psli->mbox_tmo);
	if (ha_