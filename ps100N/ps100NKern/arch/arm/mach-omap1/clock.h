ort data structure.
 * @ndlp: pointer to a node-list data structure.
 * @retry: number of retries to the command IOCB.
 *
 * This routine constructs and issues an ELS Logout (LOGO) iocb command
 * to a remote node, referred by an @ndlp on a @vport. It constructs the
 * payload of the IOCB, properly sets up the @ndlp state, and invokes the
 * lpfc_sli_issue_iocb() routine to send out the LOGO ELS command.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the LOGO ELS command.
 *
 * Return code
 *   0 - successfully issued logo
 *   1 - failed to issue logo
 **/
int
lpfc_issue_els_logo(struct lpfc_vport *vport, struct lpfc_nodelist *ndlp,
		    uint8_t retry)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	IOCB_t *icmd;
	struct lpfc_iocbq *elsiocb;
	uint8_t *pcmd;
	uint16_t cmdsize;
	int rc;

	spin_lock_irq(shost->host_lock);
	if (ndlp->nlp_flag & NLP_LOGO_SND) {
		spin_unlock_irq(shost->host_lock);
		return 0;
	}
	spin_unlock_irq(shost->host_lock);

	cmdsize = (2 * sizeof(uint32_t)) + sizeof(struct lpfc_name);
	elsiocb = lpfc_prep_els_iocb(vport, 1, cmdsize, retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_LOGO);
	if (!elsiocb)
		return 1;

	icmd = &elsiocb->iocb;
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);
	*((uint32_t *) (pcmd)) = ELS_CMD_LOGO;
	pcmd += sizeof(uint32_t);

	/* Fill in LOGO payload */
	*((uint32_t *) (pcmd)) = be32_to_cpu(vport->fc_myDID);
	pcmd += sizeof(uint32_t);
	memcpy(pcmd, &vport->fc_portname, sizeof(struct lpfc_name));

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"Issue LOGO:      did:x%x",
		ndlp->nlp_DID, 0, 0);

	phba->fc_stat.elsXmitLOGO++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_logo;
	spin_lock_irq(shost->host_lock);
	ndlp->nlp_flag |= NLP_LOGO_SND;
	spin_unlock_irq(shost->host_lock);
	rc = lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0);

	if (rc == IOCB_ERROR) {
		spin_lock_irq(shost->host_lock);
		ndlp->nlp_flag &= ~NLP_LOGO_SND;
		spin_unlock_irq(shost->host_lock);
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	return 0;
}

/**
 * lpfc_cmpl_els_cmd - Completion callback function for generic els command
 * @phba: pointer to lpfc hba data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @rspiocb: pointer to lpfc response iocb data structure.
 *
 * This routine is a generic completion callback function for ELS commands.
 * Specifically, it is the callback function which does not need to perform
 * any command specific operations. It is currently used by the ELS command
 * issuing routines for the ELS State Change  Request (SCR),
 * lpfc_issue_els_scr(), and the ELS Fibre Channel Address Resolution
 * Protocol Response (FARPR) routine, lpfc_issue_els_farpr(). Other than
 * certain debug loggings, this callback function simply invokes the
 * lpfc_els_chk_latt() routine to check whether link went down during the
 * discovery process.
 **/
static void
lpfc_cmpl_els_cmd(struct lpfc_hba *phba, struct lpfc_iocbq *cmdiocb,
		  struct lpfc_iocbq *rspiocb)
{
	struct lpfc_vport *vport = cmdiocb->vport;
	IOCB_t *irsp;

	irsp = &rspiocb->iocb;

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"ELS cmd cmpl:    status:x%x/x%x did:x%x",
		irsp->ulpStatus, irsp->un.ulpWord[4],
		irsp->un.elsreq64.remoteID);
	/* ELS cmd tag <ulpIoTag> completes */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0106 ELS cmd tag x%x completes Data: x%x x%x x%x\n",
			 irsp->ulpIoTag, irsp->ulpStatus,
			 irsp->un.ulpWord[4], irsp->ulpTimeout);
	/* Check to see if link went down during discovery */
	lpfc_els_chk_latt(vport);
	lpfc_els_free_iocb(phba, cmdiocb);
	return;
}

/**
 * lpfc_issue_els_scr - Issue a scr to an node on a vport
 * @vport: pointer to a host virtual N_Port data structure.
 * @nportid: N_Port identifier to the remote node.
 * @retry: number of retries to the command IOCB.
 *
 * This routine issues a State Change Request (SCR) to a fabric node
 * on a @vport. The remote node @nportid is passed into the function. It
 * first search the @vport node list to find the matching ndlp. If no such
 * ndlp is found, a new ndlp shall be created for this (SCR) purpose. An
 * IOCB is allocated, payload prepared, and the lpfc_sli_issue_iocb()
 * routine is invoked to send the SCR IOCB.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the SCR ELS command.
 *
 * Return code
 *   0 - Successfully issued scr command
 *   1 - Failed to issue scr command
 **/
int
lpfc_issue_els_scr(struct lpfc_vport *vport, uint32_t nportid, uint8_t retry)
{
	struct lpfc_hba  *phba = vport->phba;
	IOCB_t *icmd;
	struct lpfc_iocbq *elsiocb;
	struct lpfc_sli *psli;
	uint8_t *pcmd;
	uint16_t cmdsize;
	struct lpfc_nodelist *ndlp;

	psli = &phba->sli;
	cmdsize = (sizeof(uint32_t) + sizeof(SCR));

	ndlp = lpfc_findnode_did(vport, nportid);
	if (!ndlp) {
		ndlp = mempool_alloc(phba->nlp_mem_pool, GFP_KERNEL);
		if (!ndlp)
			return 1;
		lpfc_nlp_init(vport, ndlp, nportid);
		lpfc_enqueue_node(vport, ndlp);
	} else if (!NLP_CHK_NODE_ACT(ndlp)) {
		ndlp = lpfc_enable_node(vport, ndlp, NLP_STE_UNUSED_NODE);
		if (!ndlp)
			return 1;
	}

	elsiocb = lpfc_prep_els_iocb(vport, 1, cmdsize, retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_SCR);

	if (!elsiocb) {
		/* This will trigger the release of the node just
		 * allocated
		 */
		lpfc_nlp_put(ndlp);
		return 1;
	}

	icmd = &elsiocb->iocb;
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

	*((uint32_t *) (pcmd)) = ELS_CMD_SCR;
	pcmd += sizeof(uint32_t);

	/* For SCR, remainder of payload is SCR parameter page */
	memset(pcmd, 0, sizeof(SCR));
	((SCR *) pcmd)->Function = SCR_FUNC_FULL;

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"Issue SCR:       did:x%x",
		ndlp->nlp_DID, 0, 0);

	phba->fc_stat.elsXmitSCR++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_cmd;
	if (lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0) ==
	    IOCB_ERROR) {
		/* The additional lpfc_nlp_put will cause the following
		 * lpfc_els_free_iocb routine to trigger the rlease of
		 * the node.
		 */
		lpfc_nlp_put(ndlp);
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	/* This will cause the callback-function lpfc_cmpl_els_cmd to
	 * trigger the release of node.
	 */
	lpfc_nlp_put(ndlp);
	return 0;
}

/**
 * lpfc_issue_els_farpr - Issue a farp to an node on a vport
 * @vport: pointer to a host virtual N_Port data structure.
 * @nportid: N_Port identifier to the remote node.
 * @retry: number of retries to the command IOCB.
 *
 * This routine issues a Fibre Channel Address Resolution Response
 * (FARPR) to a node on a vport. The remote node N_Port identifier (@nportid)
 * is passed into the function. It first search the @vport node list to find
 * the matching ndlp. If no such ndlp is found, a new ndlp shall be created
 * for this (FARPR) purpose. An IOCB is allocated, payload prepared, and the
 * lpfc_sli_issue_iocb() routine is invoked to send the FARPR ELS command.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the PARPR ELS command.
 *
 * Return code
 *   0 - Successfully issued farpr command
 *   1 - Failed to issue farpr command
 **/
static int
lpfc_issue_els_farpr(struct lpfc_vport *vport, uint32_t nportid, uint8_t retry)
{
	struct lpfc_hba  *phba = vport->phba;
	IOCB_t *icmd;
	struct lpfc_iocbq *elsiocb;
	struct lpfc_sli *psli;
	FARP *fp;
	uint8_t *pcmd;
	uint32_t *lp;
	uint16_t cmdsize;
	struct lpfc_nodelist *ondlp;
	struct lpfc_nodelist *ndlp;

	psli = &phba->sli;
	cmdsize = (sizeof(uint32_t) + sizeof(FARP));

	ndlp = lpfc_findnode_did(vport, nportid);
	if (!ndlp) {
		ndlp = mempool_alloc(phba->nlp_mem_pool, GFP_KERNEL);
		if (!ndlp)
			return 1;
		lpfc_nlp_init(vport, ndlp, nportid);
		lpfc_enqueue_node(vport, ndlp);
	} else if (!NLP_CHK_NODE_ACT(ndlp)) {
		ndlp = lpfc_enable_node(vport, ndlp, NLP_STE_UNUSED_NODE);
		if (!ndlp)
			return 1;
	}

	elsiocb = lpfc_prep_els_iocb(vport, 1, cmdsize, retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_RNID);
	if (!elsiocb) {
		/* This will trigger the release of the node just
		 * allocated
		 */
		lpfc_nlp_put(ndlp);
		return 1;
	}

	icmd = &elsiocb->iocb;
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

	*((uint32_t *) (pcmd)) = ELS_CMD_FARPR;
	pcmd += sizeof(uint32_t);

	/* Fill in FARPR payload */
	fp = (FARP *) (pcmd);
	memset(fp, 0, sizeof(FARP));
	lp = (uint32_t *) pcmd;
	*lp++ = be32_to_cpu(nportid);
	*lp++ = be32_to_cpu(vport->fc_myDID);
	fp->Rflags = 0;
	fp->Mflags = (FARP_MATCH_PORT | FARP_MATCH_NODE);

	memcpy(&fp->RportName, &vport->fc_portname, sizeof(struct lpfc_name));
	memcpy(&fp->RnodeName, &vport->fc_nodename, sizeof(struct lpfc_name));
	ondlp = lpfc_findnode_did(vport, nportid);
	if (ondlp && NLP_CHK_NODE_ACT(ondlp)) {
		memcpy(&fp->OportName, &ondlp->nlp_portname,
		       sizeof(struct lpfc_name));
		memcpy(&fp->OnodeName, &ondlp->nlp_nodename,
		       sizeof(struct lpfc_name));
	}

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"Issue FARPR:     did:x%x",
		ndlp->nlp_DID, 0, 0);

	phba->fc_stat.elsXmitFARPR++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_cmd;
	if (lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0) ==
	    IOCB_ERROR) {
		/* The additional lpfc_nlp_put will cause the following
		 * lpfc_els_free_iocb routine to trigger the release of
		 * the node.
		 */
		lpfc_nlp_put(ndlp);
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	/* This will cause the callback-function lpfc_cmpl_els_cmd to
	 * trigger the release of the node.
	 */
	lpfc_nlp_put(ndlp);
	return 0;
}

/**
 * lpfc_cancel_retry_delay_tmo - Cancel the timer with delayed iocb-cmd retry
 * @vport: pointer to a host virtual N_Port data structure.
 * @nlp: pointer to a node-list data structure.
 *
 * This routine cancels the timer with a delayed IOCB-command retry for
 * a @vport's @ndlp. It stops the timer for the delayed function retrial and
 * removes the ELS retry event if it presents. In addition, if the
 * NLP_NPR_2B_DISC bit is set in the @nlp's nlp_flag bitmap, ADISC IOCB
 * commands are sent for the @vport's nodes that require issuing discovery
 * ADISC.
 **/
void
lpfc_cancel_retry_delay_tmo(struct lpfc_vport *vport, struct lpfc_nodelist *nlp)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_work_evt *evtp;

	if (!(nlp->nlp_flag & NLP_DELAY_TMO))
		return;
	spin_lock_irq(shost->host_lock);
	nlp->nlp_flag &= ~NLP_DELAY_TMO;
	spin_unlock_irq(shost->host_lock);
	del_timer_sync(&nlp->nlp_delayfunc);
	nlp->nlp_last_elscmd = 0;
	if (!list_empty(&nlp->els_retry_evt.evt_listp)) {
		list_del_init(&nlp->els_retry_evt.evt_listp);
		/* Decrement nlp reference count held for the delayed retry */
		evtp = &nlp->els_retry_evt;
		lpfc_nlp_put((struct lpfc_nodelist *)evtp->evt_arg1);
	}
	if (nlp->nlp_flag & NLP_NPR_2B_DISC) {
		spin_lock_irq(shost->host_lock);
		nlp->nlp_flag &= ~NLP_NPR_2B_DISC;
		spin_unlock_irq(shost->host_lock);
		if (vport->num_disc_nodes) {
			if (vport->port_state < LPFC_VPORT_READY) {
				/* Check if there are more ADISCs to be sent */
				lpfc_more_adisc(vport);
			} else {
				/* Check if there are more PLOGIs to be sent */
				lpfc_more_plogi(vport);
				if (vport->num_disc_nodes == 0) {
					spin_lock_irq(shost->host_lock);
					vport->fc_flag &= ~FC_NDISC_ACTIVE;
					spin_unlock_irq(shost->host_lock);
					lpfc_can_disctmo(vport);
					lpfc_end_rscn(vport);
				}
			}
		}
	}
	return;
}

/**
 * lpfc_els_retry_delay - Timer function with a ndlp delayed function timer
 * @ptr: holder for the pointer to the timer function associated data (ndlp).
 *
 * This routine is invoked by the ndlp delayed-function timer to check
 * whether there is any pending ELS retry event(s) with the node. If not, it
 * simply returns. Otherwise, if there is at least one ELS delayed event, it
 * adds the delayed events to the HBA work list and invokes the
 * lpfc_worker_wake_up() routine to wake up worker thread to process the
 * event. Note that lpfc_nlp_get() is called before posting the event to
 * the work list to hold reference count of ndlp so that it guarantees the
 * reference to ndlp will still be available when the worker thread gets
 * to the event associated with the ndlp.
 **/
void
lpfc_els_retry_delay(unsigned long ptr)
{
	struct lpfc_nodelist *ndlp = (struct lpfc_nodelist *) ptr;
	struct lpfc_vport *vport = ndlp->vport;
	struct lpfc_hba   *phba = vport->phba;
	unsigned long flags;
	struct lpfc_work_evt  *evtp = &ndlp->els_retry_evt;

	spin_lock_irqsave(&phba->hbalock, flags);
	if (!list_empty(&evtp->evt_listp)) {
		spin_unlock_irqrestore(&phba->hbalock, flags);
		return;
	}

	/* We need to hold the node by incrementing the reference
	 * count until the queued work is done
	 */
	evtp->evt_arg1  = lpfc_nlp_get(ndlp);
	if (evtp->evt_arg1) {
		evtp->evt = LPFC_EVT_ELS_RETRY;
		list_add_tail(&evtp->evt_listp, &phba->work_list);
		lpfc_worker_wake_up(phba);
	}
	spin_unlock_irqrestore(&phba->hbalock, flags);
	return;
}

/**
 * lpfc_els_retry_delay_handler - Work thread handler for ndlp delayed function
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine is the worker-thread handler for processing the @ndlp delayed
 * event(s), posted by the lpfc_els_retry_delay() routine. It simply retrieves
 * the last ELS command from the associated ndlp and invokes the proper ELS
 * function according to the delayed ELS command to retry the command.
 **/
void
lpfc_els_retry_delay_handler(struct lpfc_nodelist *ndlp)
{
	struct lpfc_vport *vport = ndlp->vport;
	struct Scsi_Host  *shost = lpfc_shost_from_vport(vport);
	uint32_t cmd, did, retry;

	spin_lock_irq(shost->host_lock);
	did = ndlp->nlp_DID;
	cmd = ndlp->nlp_last_elscmd;
	ndlp->nlp_last_elscmd = 0;

	if (!(ndlp->nlp_flag & NLP_DELAY_TMO)) {
		spin_unlock_irq(shost->host_lock);
		return;
	}

	ndlp->nlp_flag &= ~NLP_DELAY_TMO;
	spin_unlock_irq(shost->host_lock);
	/*
	 * If a discovery event readded nlp_delayfunc after timer
	 * firing and before processing the timer, cancel the
	 * nlp_delayfunc.
	 */
	del_timer_sync(&ndlp->nlp_delayfunc);
	retry = ndlp->nlp_retry;

	switch (cmd) {
	case ELS_CMD_FLOGI:
		lpfc_issue_els_flogi(vport, ndlp, retry);
		break;
	case ELS_CMD_PLOGI:
		if (!lpfc_issue_els_plogi(vport, ndlp->nlp_DID, retry)) {
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_PLOGI_ISSUE);
		}
		break;
	case ELS_CMD_ADISC:
		if (!lpfc_issue_els_adisc(vport, ndlp, retry)) {
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_ADISC_ISSUE);
		}
		break;
	case ELS_CMD_PRLI:
		if (!lpfc_issue_els_prli(vport, ndlp, retry)) {
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_PRLI_ISSUE);
		}
		break;
	case ELS_CMD_LOGO:
		if (!lpfc_issue_els_logo(vport, ndlp, retry)) {
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_NPR_NODE);
		}
		break;
	case ELS_CMD_FDISC:
		lpfc_issue_els_fdisc(vport, ndlp, retry);
		break;
	}
	return;
}

/**
 * lpfc_els_retry - Make retry decision on an els command iocb
 * @phba: pointer to lpfc hba data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @rspiocb: pointer to lpfc response iocb data structure.
 *
 * This routine makes a retry decision on an ELS command IOCB, which has
 * failed. The following ELS IOCBs use this function for retrying the command
 * when previously issued command responsed with error status: FLOGI, PLOGI,
 * PRLI, ADISC, LOGO, and FDISC. Based on the ELS command type and the
 * returned error status, it makes the decision whether a retry shall be
 * issued for the command, and whether a retry shall be made immediately or
 * delayed. In the former case, the corresponding ELS command issuing-function
 * is called to retry the command. In the later case, the ELS command shall
 * be posted to the ndlp delayed event and delayed function timer set to the
 * ndlp for the delayed command issusing.
 *
 * Return code
 *   0 - No retry of els command is made
 *   1 - Immediate or delayed retry of els command is made
 **/
static int
lpfc_els_retry(struct lpfc_hba *phba, struct lpfc_iocbq *cmdiocb,
	       struct lpfc_iocbq *rspiocb)
{
	struct lpfc_vport *vport = cmdiocb->vport;
	struct Scsi_Host  *shost = lpfc_shost_from_vport(vport);
	IOCB_t *irsp = &rspiocb->iocb;
	struct lpfc_nodelist *ndlp = (struct lpfc_nodelist *) cmdiocb->context1;
	struct lpfc_dmabuf *pcmd = (struct lpfc_dmabuf *) cmdiocb->context2;
	uint32_t *elscmd;
	struct ls_rjt stat;
	int retry = 0, maxretry = lpfc_max_els_tries, delay = 0;
	int logerr = 0;
	uint32_t cmd = 0;
	uint32_t did;


	/* Note: context2 may be 0 for internal driver abort
	 * of delays ELS command.
	 */

	if (pcmd && pcmd->virt) {
		elscmd = (uint32_t *) (pcmd->virt);
		cmd = *elscmd++;
	}

	if (ndlp && NLP_CHK_NODE_ACT(ndlp))
		did = ndlp->nlp_DID;
	else {
		/* We should only hit this case for retrying PLOGI */
		did = irsp->un.elsreq64.remoteID;
		ndlp = lpfc_findnode_did(vport, did);
		if ((!ndlp || !NLP_CHK_NODE_ACT(ndlp))
		    && (cmd != ELS_CMD_PLOGI))
			return 1;
	}

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"Retry ELS:       wd7:x%x wd4:x%x did:x%x",
		*(((uint32_t *) irsp) + 7), irsp->un.ulpWord[4], ndlp->nlp_DID);

	switch (irsp->ulpStatus) {
	case IOSTAT_FCP_RSP_ERROR:
	case IOSTAT_REMOTE_STOP:
		break;

	case IOSTAT_LOCAL_REJECT:
		switch ((irsp->un.ulpWord[4] & 0xff)) {
		case IOERR_LOOP_OPEN_FAILURE:
			if (cmd == ELS_CMD_FLOGI) {
				if (PCI_DEVICE_ID_HORNET ==
					phba->pcidev->device) {
					phba->fc_topology = TOPOLOGY_LOOP;
					phba->pport->fc_myDID = 0;
					phba->alpa_map[0] = 0;
					phba->alpa_map[1] = 0;
				}
			}
			if (cmd == ELS_CMD_PLOGI && cmdiocb->retry == 0)
				delay = 1000;
			retry = 1;
			break;

		case IOERR_ILLEGAL_COMMAND:
			lpfc_printf_vlog(vport, KERN_ERR, LOG_ELS,
					 "0124 Retry illegal cmd x%x "
					 "retry:x%x delay:x%x\n",
					 cmd, cmdiocb->retry, delay);
			retry = 1;
			/* All command's retry policy */
			maxretry = 8;
			if (cmdiocb->retry > 2)
				delay = 1000;
			break;

		case IOERR_NO_RESO