atus |= HC_MBINT_ENA | HC_ERINT_ENA | HC_LAINT_ENA;
	if (psli->num_rings > 0)
		status |= HC_R0INT_ENA;
	if (psli->num_rings > 1)
		status |= HC_R1INT_ENA;
	if (psli->num_rings > 2)
		status |= HC_R2INT_ENA;
	if (psli->num_rings > 3)
		status |= HC_R3INT_ENA;

	if ((phba->cfg_poll & ENABLE_FCP_RING_POLLING) &&
	    (phba->cfg_poll & DISABLE_FCP_RING_INT))
		status &= ~(HC_R0INT_ENA);

	writel(status, phba->HCregaddr);
	readl(phba->HCregaddr); /* flush */
	spin_unlock_irq(&phba->hbalock);

	/* Set up ring-0 (ELS) timer */
	timeout = phba->fc_ratov * 2;
	mod_timer(&vport->els_tmofunc, jiffies + HZ * timeout);
	/* Set up heart beat (HB) timer */
	mod_timer(&phba->hb_tmofunc, jiffies + HZ * LPFC_HB_MBOX_INTERVAL);
	phba->hb_outstanding = 0;
	phba->last_completion_time = jiffies;
	/* Set up error attention (ERATT) polling timer */
	mod_timer(&phba->eratt_poll, jiffies + HZ * LPFC_ERATT_POLL_INTERVAL);

	lpfc_init_link(phba, pmb, phba->cfg_topology, phba->cfg_link_speed);
	pmb->mbox_cmpl = lpfc_sli_def_mbox_cmpl;
	lpfc_set_loopback_flag(phba);
	rc = lpfc_sli_issue_mbox(phba, pmb, MBX_NOWAIT);
	if (rc != MBX_SUCCESS) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
				"0454 Adapter failed to init, mbxCmd x%x "
				"INIT_LINK, mbxStatus x%x\n",
				mb->mbxCommand, mb->mbxStatus);

		/* Clear all interrupt enable conditions */
		writel(0, phba->HCregaddr);
		readl(phba->HCregaddr); /* flush */
		/* Clear all pending interrupts */
		writel(0xffffffff, phba->HAregaddr);
		readl(phba->HAregaddr); /* flush */

		phba->link_state = LPFC_HBA_ERROR;
		if (rc != MBX_BUSY)
			mempool_free(pmb, phba->mbox_mem_pool);
		return -EIO;
	}
	/* MBOX buffer will be freed in mbox compl */
	pmb = mempool_alloc(phba->mbox_mem_pool, GFP_KERNEL);
	lpfc_config_async(phba, pmb, LPFC_ELS_RING);
	pmb->mbox_cmpl = lpfc_config_async_cmpl;
	pmb->vport = phba->pport;
	rc = lpfc_sli_issue_mbox(phba, pmb, MBX_NOWAIT);

	if ((rc != MBX_BUSY) && (rc != MBX_SUCCESS)) {
		lpfc_printf_log(phba,
				KERN_ERR,
				LOG_INIT,
				"0456 Adapter failed to issue "
				"ASYNCEVT_ENABLE mbox status x%x \n.",
				rc);
		mempool_free(pmb, phba->mbox_mem_pool);
	}

	/* Get Option rom version */
	pmb = mempool_alloc(phba->mbox_mem_pool, GFP_KERNEL);
	lpfc_dump_wakeup_param(phba, pmb);
	pmb->mbox_cmpl = lpfc_dump_wakeup_param_cmpl;
	pmb->vport = phba->pport;
	rc = lpfc_sli_issue_mbox(phba, pmb, MBX_NOWAIT);

	if ((rc != MBX_BUSY) && (rc != MBX_SUCCESS)) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT, "0435 Adapter failed "
				"to get Option ROM version status x%x\n.", rc);
		mempool_free(pmb, phba->mbox_mem_pool);
	}

	return 0;
}

/**
 * lpfc_hba_down_prep - Perform lpfc uninitialization prior to HBA reset
 * @phba: pointer to lpfc HBA data structure.
 *
 * This routine will do LPFC uninitialization before the HBA is reset when
 * bringing down the SLI Layer.
 *
 * Return codes
 *   0 - success.
 *   Any other value - error.
 **/
int
lpfc_hba_down_prep(struct lpfc_hba *phba)
{
	struct lpfc_vport **vports;
	int i;

	if (phba->sli_rev <= LPFC_SLI_REV3) {
		/* Disable interrupts */
		writel(0, phba->HCregaddr);
		readl(phba->HCregaddr); /* flush */
	}

	if (phba->pport->load_flag & FC_UNLOADING)
		lpfc_cleanup_discovery_resources(phba->pport);
	else {
		vports = lpfc_create_vport_work_array(phba);
		if (vports != NULL)
			for (i = 0; i <= phba->max_vports &&
				vports[i] != NULL; i++)
				lpfc_cleanup_discovery_resources(vports[i]);
		lpfc_destroy_vport_work_array(phba, vports);
	}
	return 0;
}

/**
 * lpfc_hba_down_post_s3 - Perform lpfc uninitialization after HBA reset
 * @phba: pointer to lpfc HBA data structure.
 *
 * This routine will do uninitialization after the HBA is reset when bring
 * down the SLI Layer.
 *
 * Return codes
 *   0 - sucess.
 *   Any other value - error.
 **/
static int
lpfc_hba_down_post_s3(struct lpfc_hba *phba)
{
	struct lpfc_sli *psli = &phba->sli;
	struct lpfc_sli_ring *pring;
	struct lpfc_dmabuf *mp, *next_mp;
	LIST_HEAD(completions);
	int i;

	if (phba->sli3_options & LPFC_SLI3_HBQ_ENABLED)
		lpfc_sli_hbqbuf_free_all(phba);
	else {
		/* Cleanup preposted buffers on the ELS ring */
		pring = &psli->ring[LPFC_ELS_RING];
		list_for_each_entry_safe(mp, next_mp, &pring->postbufq, list) {
			list_del(&mp->list);
			pring->postbufq_cnt--;
			lpfc_mbuf_free(phba, mp->virt, mp->phys);
			kfree(mp);
		}
	}

	spin_lock_irq(&phba->hbalock);
	for (i = 0; i < psli->num_rings; i++) {
		pring = &psli->ring[i];

		/* At this point in time the HBA is either reset or DOA. Either
		 * way, nothing should be on txcmplq as it will NEVER complete.
		 */
		list_splice_init(&pring->txcmplq, &completions);
		pring->txcmplq_cnt = 0;
		spin_unlock_irq(&phba->hbalock);

		/* Cancel all the IOCBs from the completions list */
		lpfc_sli_cancel_iocbs(phba, &completions, IOSTAT_LOCAL_REJECT,
				      IOERR_SLI_ABORTED);

		lpfc_sli_abort_iocb_ring(phba, pring);
		spin_lock_irq(&phba->hbalock);
	}
	spin_unlock_irq(&phba->hbalock);

	return 0;
}
/**
 * lpfc_hba_down_post_s4 - Perform lpfc uninitialization after HBA reset
 * @phba: pointer to lpfc HBA data structure.
 *
 * This routine will do uninitialization after the HBA is reset when bring
 * down the SLI Layer.
 *
 * Return codes
 *   0 - sucess.
 *   Any other value - error.
 **/
static int
lpfc_hba_down_post_s4(struct lpfc_hba *phba)
{
	struct lpfc_scsi_buf *psb, *psb_next;
	LIST_HEAD(aborts);
	int ret;
	unsigned long iflag = 0;
	ret = lpfc_hba_down_post_s3(phba);
	if (ret)
		return ret;
	/* At this point in time the HBA is either reset or DOA. Either
	 * way, nothing should be on lpfc_abts_els_sgl_list, it needs to be
	 * on the lpfc_sgl_list so that it can either be freed if the
	 * driver is unloading or reposted if the driver is restarting
	 * the port.
	 */
	spin_lock_irq(&phba->hbalock);  /* required for lpfc_sgl_list and */
					/* scsl_buf_list */
	/* abts_sgl_list_lock required because worker thread uses this
	 * list.
	 */
	spin_lock(&phba->sli4_hba.abts_sgl_list_lock);
	list_splice_init(&phba->sli4_hba.lpfc_abts_els_sgl_list,
			&phba->sli4_hba.lpfc_sgl_list);
	spin_unlock(&phba->sli4_hba.abts_sgl_list_lock);
	/* abts_scsi_buf_list_lock required because worker thread uses this
	 * list.
	 */
	spin_lock(&phba->sli4_hba.abts_scsi_buf_list_lock);
	list_splice_init(&phba->sli4_hba.lpfc_abts_scsi_buf_list,
			&aborts);
	spin_unlock(&phba->sli4_hba.abts_scsi_buf_list_lock);
	spin_unlock_irq(&phba->hbalock);

	list_for_each_entry_safe(psb, psb_next, &aborts, list) {
		psb->pCmd = NULL;
		psb->status = IOSTAT_SUCCESS;
	}
	spin_lock_irqsave(&phba->scsi_buf_list_lock, iflag);
	list_splice(&aborts, &phba->lpfc_scsi_buf_list);
	spin_unlock_irqrestore(&phba->scsi_buf_list_lock, iflag);
	return 0;
}

/**
 * lpfc_hba_down_post - Wrapper func for hba down post routine
 * @phba: pointer to lpfc HBA data structure.
 *
 * This routine wraps the actual SLI3 or SLI4 routine for performing
 * uninitialization after the HBA is reset when bring down the SLI Layer.
 *
 * Return codes
 *   0 - sucess.
 *   Any other value - error.
 **/
int
lpfc_hba_down_post(struct lpfc_hba *phba)
{
	return (*phba->lpfc_hba_down_post)(phba);
}

/**
 * lpfc_hb_timeout - The HBA-timer timeout handler
 * @ptr: unsigned long holds the pointer to lpfc hba data structure.
 *
 * This is the HBA-timer timeout handler registered to the lpfc driver. When
 * this timer fires, a HBA timeout event shall be posted to the lpfc driver
 * work-port-events bitmap and the worker thread is notified. This timeout
 * event will be used by the worker thread to invoke the actual timeout
 * handler routine, lpfc_hb_timeout_handler. Any periodical operations will
 * be performed in the timeout handler and the HBA timeout event bit shall
 * be cleared by the worker thread after it has taken the event bitmap out.
 **/
static void
lpfc_hb_timeout(unsigned long ptr)
{
	struct lpfc_hba *phba;
	uint32_t tmo_posted;
	unsigned long iflag;

	phba = (struct lpfc_hba *)ptr;

	/* Check for heart beat timeout conditions */
	spin_lock_irqsave(&phba->pport->work_port_lock, iflag);
	tmo_posted = phba->pport->work_port_events & WORKER_HB_TMO;
	if (!tmo_posted)
		phba->pport->work_port_events |= WORKER_HB_TMO;
	spin_unlock_irqrestore(&phba->pport->work_port_lock, iflag);

	/* Tell the worker thread there is work to do */
	if (!tmo_posted)
		lpfc_worker_wake_up(phba);
	return;
}

/**
 * lpfc_hb_mbox_cmpl - The lpfc heart-beat mailbox command callback function
 * @phba: pointer to lpfc hba data structure.
 * @pmboxq: pointer to the driver internal queue element for mailbox command.
 *
 * This is the callback function to the lpfc heart-beat mailbox command.
 * If configured, the lpfc driver issues the heart-beat mailbox command to
 * the HBA every LPFC_HB_MBOX_INTERVAL (current 5) seconds. At the time the
 * heart-beat mailbox command is issued, the drive