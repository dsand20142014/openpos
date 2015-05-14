ATT | HA_MBATT | HA_LATT);
	phba->work_ha_mask |= (HA_RXMASK << (LPFC_ELS_RING * 4));

	/* Get all the module params for configuring this host */
	lpfc_get_cfgparam(phba);
	/*
	 * Since the sg_tablesize is module parameter, the sg_dma_buf_size
	 * used to create the sg_dma_buf_pool must be dynamically calculated.
	 * 2 segments are added since the IOCB needs a command and response bde.
	 */
	phba->cfg_sg_dma_buf_size = sizeof(struct fcp_cmnd) +
		sizeof(struct fcp_rsp) +
			((phba->cfg_sg_seg_cnt + 2) * sizeof(struct ulp_bde64));

	if (phba->cfg_enable_bg) {
		phba->cfg_sg_seg_cnt = LPFC_MAX_SG_SEG_CNT;
		phba->cfg_sg_dma_buf_size +=
			phba->cfg_prot_sg_seg_cnt * sizeof(struct ulp_bde64);
	}

	/* Also reinitialize the host templates with new values. */
	lpfc_vport_template.sg_tablesize = phba->cfg_sg_seg_cnt;
	lpfc_template.sg_tablesize = phba->cfg_sg_seg_cnt;

	phba->max_vpi = LPFC_MAX_VPI;
	/* This will be set to correct value after config_port mbox */
	phba->max_vports = 0;

	/*
	 * Initialize the SLI Layer to run with lpfc HBAs.
	 */
	lpfc_sli_setup(phba);
	lpfc_sli_queue_setup(phba);

	/* Allocate device driver memory */
	if (lpfc_mem_alloc(phba, BPL_ALIGN_SZ))
		return -ENOMEM;

	return 0;
}

/**
 * lpfc_sli_driver_resource_unset - Unset drvr internal resources for SLI3 dev
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to unset the driver internal resources set up
 * specific for supporting the SLI-3 HBA device it attached to.
 **/
static void
lpfc_sli_driver_resource_unset(struct lpfc_hba *phba)
{
	/* Free device driver memory allocated */
	lpfc_mem_free_all(phba);

	return;
}

/**
 * lpfc_sli4_driver_resource_setup - Setup drvr internal resources for SLI4 dev
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to set up the driver internal resources specific to
 * support the SLI-4 HBA device it attached to.
 *
 * Return codes
 * 	0 - sucessful
 * 	other values - error
 **/
static int
lpfc_sli4_driver_resource_setup(struct lpfc_hba *phba)
{
	struct lpfc_sli *psli;
	int rc;
	int i, hbq_count;

	/* Before proceed, wait for POST done and device ready */
	rc = lpfc_sli4_post_status_check(phba);
	if (rc)
		return -ENODEV;

	/*
	 * Initialize timers used by driver
	 */

	/* Heartbeat timer */
	init_timer(&phba->hb_tmofunc);
	phba->hb_tmofunc.function = lpfc_hb_timeout;
	phba->hb_tmofunc.data = (unsigned long)phba;

	psli = &phba->sli;
	/* MBOX heartbeat timer */
	init_timer(&psli->mbox_tmo);
	psli->mbox_tmo.function = lpfc_mbox_timeout;
	psli->mbox_tmo.data = (unsigned long) phba;
	/* Fabric block timer */
	init_timer(&phba->fabric_block_timer);
	phba->fabric_block_timer.function = lpfc_fabric_block_timeout;
	phba->fabric_block_timer.data = (unsigned long) phba;
	/* EA polling mode timer */
	init_timer(&phba->eratt_poll);
	phba->eratt_poll.function = lpfc_poll_eratt;
	phba->eratt_poll.data = (unsigned long) phba;
	/*
	 * We need to do a READ_CONFIG mailbox command here before
	 * calling lpfc_get_cfgparam. For VFs this will report the
	 * MAX_XRI, MAX_VPI, MAX_RPI, MAX_IOCB, and MAX_VFI settings.
	 * All of the resources allocated
	 * for this Port are tied to these values.
	 */
	/* Get all the module params for configuring this host */
	lpfc_get_cfgparam(phba);
	phba->max_vpi = LPFC_MAX_VPI;
	/* This will be set to correct value after the read_config mbox */
	phba->max_vports = 0;

	/* Program the default value of vlan_id and fc_map */
	phba->valid_vlan = 0;
	phba->fc_map[0] = LPFC_FCOE_FCF_MAP0;
	phba->fc_map[1] = LPFC_FCOE_FCF_MAP1;
	phba->fc_map[2] = LPFC_FCOE_FCF_MAP2;

	/*
	 * Since the sg_tablesize is module parameter, the sg_dma_buf_size
	 * used to create the sg_dma_buf_pool must be dynamically calculated.
	 * 2 segments are added since the IOCB needs a command and response bde.
	 * To insure that the scsi sgl does not cross a 4k page boundary only
	 * sgl sizes of 1k, 2k, 4k, and 8k are supported.
	 * Table of sgl sizes and seg_cnt:
	 * sgl size, 	sg_seg_cnt	total seg
	 * 1k		50		52
	 * 2k		114		116
	 * 4k		242		244
	 * 8k		498		500
	 * cmd(32) + rsp(160) + (52 * sizeof(sli4_sge)) = 1024
	 * cmd(32) + rsp(160) + (116 * sizeof(sli4_sge)) = 2048
	 * cmd(32) + rsp(160) + (244 * sizeof(sli4_sge)) = 4096
	 * cmd(32) + rsp(160) + (500 * sizeof(sli4_sge)) = 8192
	 */
	if (phba->cfg_sg_seg_cnt <= LPFC_DEFAULT_SG_SEG_CNT)
		phba->cfg_sg_seg_cnt = 50;
	else if (phba->cfg_sg_seg_cnt <= 114)
		phba->cfg_sg_seg_cnt = 114;
	else if (phba->cfg_sg_seg_cnt <= 242)
		phba->cfg_sg_seg_cnt = 242;
	else
		phba->cfg_sg_seg_cnt = 498;

	phba->cfg_sg_dma_buf_size = sizeof(struct fcp_cmnd)
					+ sizeof(struct fcp_rsp);
	phba->cfg_sg_dma_buf_size +=
		((phba->cfg_sg_seg_cnt + 2) * sizeof(struct sli4_sge));

	/* Initialize buffer queue management fields */
	hbq_count = lpfc_sli_hbq_count();
	for (i = 0; i < hbq_count; ++i)
		INIT_LIST_HEAD(&phba->hbqs[i].hbq_buffer_list);
	INIT_LIST_HEAD(&phba->rb_pend_list);
	phba->hbqs[LPFC_ELS_HBQ].hbq_alloc_buffer = lpfc_sli4_rb_alloc;
	phba->hbqs[LPFC_ELS_HBQ].hbq_free_buffer = lpfc_sli4_rb_free;

	/*
	 * Initialize the SLI Layer to run with lpfc SLI4 HBAs.
	 */
	/* Initialize the Abort scsi buffer list used by driver */
	spin_lock_init(&phba->sli4_hba.abts_scsi_buf_list_lock);
	INIT_LIST_HEAD(&phba->sli4_hba.lpfc_abts_scsi_buf_list);
	/* This abort list used by worker thread */
	spin_lock_init(&phba->sli4_hba.abts_sgl_list_lock);

	/*
	 * Initialize dirver internal slow-path work queues
	 */

	/* Driver internel slow-path CQ Event pool */
	INIT_LIST_HEAD(&phba->sli4_hba.sp_cqe_event_pool);
	/* Response IOCB work queue list */
	INIT_LIST_HEAD(&phba->sli4_hba.sp_rspiocb_work_queue);
	/* Asynchronous event CQ Event work queue list */
	INIT_LIST_HEAD(&phba->sli4_hba.sp_asynce_work_queue);
	/* Fast-path XRI aborted CQ Event work queue list */
	INIT_LIST_HEAD(&phba->sli4_hba.sp_fcp_xri_aborted_work_queue);
	/* Slow-path XRI aborted CQ Event work queue list */
	INIT_LIST_HEAD(&phba->sli4_hba.sp_els_xri_aborted_work_queue);
	/* Receive queue CQ Event work queue list */
	INIT_LIST_HEAD(&phba->sli4_hba.sp_unsol_work_queue);

	/* Initialize the driver internal SLI layer lists. */
	lpfc_sli_setup(phba);
	lpfc_sli_queue_setup(phba);

	/* Allocate device driver memory */
	rc = lpfc_mem_alloc(phba, SGL_ALIGN_SZ);
	if (rc)
		return -ENOMEM;

	/* Create the bootstrap mailbox command */
	rc = lpfc_create_bootstrap_mbox(phba);
	if (unlikely(rc))
		goto out_free_mem;

	/* Set up the host's endian order with the device. */
	rc = lpfc_setup_endian_order(phba);
	if (unlikely(rc))
		goto out_free_bsmbx;

	/* Set up the hba's configuration parameters. */
	rc = lpfc_sli4_read_config(phba);
	if (unlikely(rc))
		goto out_free_bsmbx;

	/* Perform a function reset */
	rc = lpfc_pci_function_reset(phba);
	if (unlikely(rc))
		goto out_free_bsmbx;

	/* Create all the SLI4 queues */
	rc = lpfc_sli4_queue_create(phba);
	if (rc)
		goto out_free_bsmbx;

	/* Create driver internal CQE event pool */
	rc = lpfc_sli4_cq_event_pool_create(phba);
	if (rc)
		goto out_destroy_queue;

	/* Initialize and populate the iocb list per host */
	rc = lpfc_init_sgl_list(phba);
	if (rc) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
				"1400 Failed to initialize sgl list.\n");
		goto out_destroy_cq_event_pool;
	}
	rc = lpfc_init_active_sgl_array(phba);
	if (rc) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
				"1430 Failed to initialize sgl list.\n");
		goto out_free_sgl_list;
	}

	rc = lpfc_sli4_init_rpi_hdrs(phba);
	if (rc) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
				"1432 Failed to initialize rpi headers.\n");
		goto out_free_active_sgl;
	}

	phba->sli4_hba.fcp_eq_hdl = kzalloc((sizeof(struct lpfc_fcp_eq_hdl) *
				    phba->cfg_fcp_eq_count), GFP_KERNEL);
	if (!phba->sli4_hba.fcp_eq_hdl) {
		lpfc