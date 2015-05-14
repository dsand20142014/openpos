* N_Port IDs.
 *
 * Return code
 *   0 - Success
 *   -ENXIO - Fail
 **/
static int
lpfc_cmpl_els_flogi_nport(struct lpfc_vport *vport, struct lpfc_nodelist *ndlp,
			  struct serv_parm *sp)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	LPFC_MBOXQ_t *mbox;
	int rc;

	spin_lock_irq(shost->host_lock);
	vport->fc_flag &= ~(FC_FABRIC | FC_PUBLIC_LOOP);
	spin_unlock_irq(shost->host_lock);

	phba->fc_edtov = FF_DEF_EDTOV;
	phba->fc_ratov = FF_DEF_RATOV;
	rc = memcmp(&vport->fc_portname, &sp->portName,
		    sizeof(vport->fc_portname));
	if (rc >= 0) {
		/* This side will initiate the PLOGI */
		spin_lock_irq(shost->host_lock);
		vport->fc_flag |= FC_PT2PT_PLOGI;
		spin_unlock_irq(shost->host_lock);

		/*
		 * N_Port ID cannot be 0, set our to LocalID the other
		 * side will be RemoteID.
		 */

		/* not equal */
		if (rc)
			vport->fc_myDID = PT2PT_LocalID;

		mbox = mempool_alloc(phba->mbox_mem_pool, GFP_KERNEL);
		if (!mbox)
			goto fail;

		lpfc_config_link(phba, mbox);

		mbox->mbox_cmpl = lpfc_sli_def_mbox_cmpl;
		mbox->vport = vport;
		rc = lpfc_sli_issue_mbox(phba, mbox, MBX_NOWAIT);
		if (rc == MBX_NOT_FINISHED) {
			mempool_free(mbox, phba->mbox_mem_pool);
			goto fail;
		}
		/* Decrement ndlp reference count indicating that ndlp can be
		 * safely released when other references to it are done.
		 */
		lpfc_nlp_put(ndlp);

		ndlp = lpfc_findnode_did(vport, PT2PT_RemoteID);
		if (!ndlp) {
			/*
			 * Cannot find existing Fabric ndlp, so allocate a
			 * new one
			 */
			ndlp = mempool_alloc(phba->nlp_mem_pool, GFP_KERNEL);
			if (!ndlp)
				goto fail;
			lpfc_nlp_init(vport, ndlp, PT2PT_RemoteID);
		} else if (!NLP_CHK_NODE_ACT(ndlp)) {
			ndlp = lpfc_enable_node(vport, ndlp,
						NLP_STE_UNUSED_NODE);
			if(!ndlp)
				goto fail;
		}

		memcpy(&ndlp->nlp_portname, &sp->portName,
		       sizeof(struct lpfc_name));
		memcpy(&ndlp->nlp_nodename, &sp->nodeName,
		       sizeof(struct lpfc_name));
		/* Set state will put ndlp onto node list if not already done */
		lpfc_nlp_set_state(vport, ndlp, NLP_STE_NPR_NODE);
		spin_lock_irq(shost->host_lock);
		ndlp->nlp_flag |= NLP_NPR_2B_DISC;
		spin_unlock_irq(shost->host_lock);
	} else
		/* This side will wait for the PLOGI, 