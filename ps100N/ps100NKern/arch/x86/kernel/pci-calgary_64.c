_ELS,
				"0138 ELS rsp: Cannot issue reg_login for x%x "
				"Data: x%x x%x x%x\n",
				ndlp->nlp_DID, ndlp->nlp_flag, ndlp->nlp_state,
				ndlp->nlp_rpi);

			if (lpfc_nlp_not_used(ndlp)) {
				ndlp = NULL;
				/* Indicate node has already been released,
				 * should not reference to it from within
				 * the routine lpfc_els_free_iocb.
				 */
				cmdiocb->context1 = NULL;
			}
		} else {
			/* Do not drop node for lpfc_els_abort'ed ELS cmds */
			if (!lpfc_error_lost_link(irsp) &&
			    ndlp->nlp_flag & NLP_ACC_REGLOGIN) {
				if (lpfc_nlp_not_used(ndlp)) {
					ndlp = NULL;
					/* Indicate node has already been
					 * released, should not reference
					 * to it from within the routine
					 * lpfc_els_free_iocb.
					 */
					cmdiocb->context1 = NULL;
				}
			}
		}
		mp = (struct lpfc_dmabuf *) mbox->context1;
		if (mp) {
			lpfc_mbuf_free(phba, mp->virt, mp->phys);
			kfree(mp);
		}
		mempool_free(mbox, phba->mbox_mem_pool);
	}
out:
	if (ndlp && NLP_CHK_NODE_ACT(ndlp)) {
		spin_lock_irq(shost->host_lock);
		ndlp->nlp_flag &= ~(NLP_ACC_REGLOGIN | NLP_RM_DFLT_RPI);
		spin_unlock_irq(shost->host_lock);

		/* If the node is not being used by another discovery thread,
		 * and we are sending a reject, we are done with it.
		 * Release driver reference count here and free associated
		 * resources.
		 */
		if (ls_rjt)
			if (lpfc_nlp_not_used(ndlp))
				/* Indicate node has already been released,
				 * should not reference to it from within
				 * the routine lpfc_els_free_iocb.
				 */
				cmdiocb->context1 = NULL;
	}

	lpfc_els_free_iocb(phba, cmdiocb);
	return;
}

/**
 * lpfc_els_rsp_acc - Prepare and issue an acc response iocb command
 * @vport: pointer to a host virtual N_Port data structure.
 * @flag: the els command code to be accepted.
 * @oldiocb: pointer to the original lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 * @mbox: pointer to the driver internal queue element for mailbox command.
 *
 * This routine prepares and issues an Accept (ACC) response IOCB
 * command. It uses the @flag to properly set up the IOCB field for the
 * specific ACC response command to be issued and invokes the
 * lpfc_sli_issue_iocb() routine to send out ACC response IOCB. If a
 * @mbox pointer is passed in, it will be put into the context_un.mbox
 * field of the IOCB for the completion callback function to issue the
 * mailbox command to the HBA later when callback is invoked.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the corresponding response ELS IOCB command.
 *
 * Return code
 *   0 - Successfully issued acc response
 *   1 - Failed to issue acc response
 **/
int
lpfc_els_rsp_acc(struct lpfc_vport *vport, uint32_t flag,
		 struct lpfc_iocbq *oldiocb, struct lpfc_nodelist *ndlp,
		 LPFC_MBOXQ_t *mbox)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	IOCB_t *icmd;
	IOCB_t *oldcmd;
	struct lpfc_iocbq *elsiocb;
	struct lpfc_sli *psli;
	uint8_t *pcmd;
	uint16_t cmdsize;
	int rc;
	ELS_PKT *els_pkt_ptr;

	psli = &phba->sli;
	oldcmd = &oldiocb->iocb;

	switch (flag) {
	case ELS_CMD_ACC:
		cmdsize = sizeof(uint32_t);
		elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry,
					     ndlp, ndlp->nlp_DID, ELS_CMD_ACC);
		if (!elsiocb) {
			spin_lock_irq(shost->host_lock);
			ndlp->nlp_flag &= ~NLP_LOGO_ACC;
			spin_unlock_irq(shost->host_lock);
			return 1;
		}

		icmd = &elsiocb->iocb;
		icmd->ulpContext = oldcmd->ulpContext;	/* Xri */
		pcmd = (((struct lpfc_dmabuf *) elsiocb->context2)->virt);
		*((uint32_t *) (pcmd)) = ELS_CMD_ACC;
		pcmd += sizeof(uint32_t);

		lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
			"Issue ACC:       did:x%x flg:x%x",
			ndlp->nlp_DID, ndlp->nlp_flag, 0);
		break;
	case ELS_CMD_PLOGI:
		cmdsize = (sizeof(struct serv_parm) + sizeof(uint32_t));
		elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry,
					     ndlp, ndlp->nlp_DID, ELS_CMD_ACC);
		if (!elsiocb)
			return 1;

		icmd = &elsiocb->iocb;
		icmd->ulpContext = oldcmd->ulpContext;	/* Xri */
		pcmd = (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

		if (mbox)
			elsiocb->context_un.mbox = mbox;

		*((uint32_t *) (pcmd)) = ELS_CMD_ACC;
		pcmd += sizeof(uint32_t);
		memcpy(pcmd, &vport->fc_sparam, sizeof(struct serv_parm));

		lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
			"Issue ACC PLOGI: did:x%x flg:x%x",
			ndlp->nlp_DID, ndlp->nlp_flag, 0);
		break;
	case ELS_CMD_PRLO:
		cmdsize = sizeof(uint32_t) + sizeof(PRLO);
		elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry,
					     ndlp, ndlp->nlp_DID, ELS_CMD_PRLO);
		if (!elsiocb)
			return 1;

		icmd = &elsiocb->iocb;
		icmd->ulpContext = oldcmd->ulpContext; /* Xri */
		pcmd = (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

		memcpy(pcmd, ((struct lpfc_dmabuf *) oldiocb->context2)->virt,
		       sizeof(uint32_t) + sizeof(PRLO));
		*((uint32_t *) (pcmd)) = ELS_CMD_PRLO_ACC;
		els_pkt_ptr = (ELS_PKT *) pcmd;
		els_pkt_ptr->un.prlo.acceptRspCode = PRLO_REQ_EXECUTED;

		lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
			"Issue ACC PRLO:  did:x%x flg:x%x",
			ndlp->nlp_DID, ndlp->nlp_flag, 0);
		break;
	default:
		return 1;
	}
	/* Xmit ELS ACC response tag <ulpIoTag> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0128 Xmit ELS ACC response tag x%x, XRI: x%x, "
			 "DID: x%x, nlp_flag: x%x nlp_state: x%x RPI: x%x\n",
			 elsiocb->iotag, elsiocb->iocb.ulpContext,
			 ndlp->nlp_DID, ndlp->nlp_flag, ndlp->nlp_state,
			 ndlp->nlp_rpi);
	if (ndlp->nlp_flag & NLP_LOGO_ACC) {
		spin_lock_irq(shost->host_lock);
		ndlp->nlp_flag &= ~NLP_LOGO_ACC;
		spin_unlock_irq(shost->host_lock);
		elsiocb->iocb_cmpl = lpfc_cmpl_els_logo_acc;
	} else {
		elsiocb->iocb_cmpl = lpfc_cmpl_els_rsp;
	}

	phba->fc_stat.elsXmitACC++;
	rc = lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0);
	if (rc == IOCB_ERROR) {
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	return 0;
}

/**
 * lpfc_els_rsp_reject - Propare and issue a rjt response iocb command
 * @vport: pointer to a virtual N_Port data structure.
 * @rejectError:
 * @oldiocb: pointer to the original lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 * @mbox: pointer to the driver internal queue element for mailbox command.
 *
 * This routine prepares and issue an Reject (RJT) response IOCB
 * command. If a @mbox pointer is passed in, it will be put into the
 * context_un.mbox field of the IOCB for the completion callback function
 * to issue to the HBA later.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the reject response ELS IOCB command.
 *
 * Return code
 *   0 - Successfully issued reject response
 *   1 - Failed to issue reject response
 **/
int
lpfc_els_rsp_reject(struct lpfc_vport *vport, uint32_t rejectError,
		    struct lpfc_iocbq *oldiocb, struct lpfc_nodelist *ndlp,
		    LPFC_MBOXQ_t *mbox)
{
	struct lpfc_hba  *phba = vport->phba;
	IOCB_t *icmd;
	IOCB_t *oldcmd;
	struct lpfc_iocbq *elsiocb;
	struct lpfc_sli *psli;
	uint8_t *pcmd;
	uint16_t cmdsize;
	int rc;

	psli = &phba->sli;
	cmdsize = 2 * sizeof(uint32_t);
	elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_LS_RJT);
	if (!elsiocb)
		return 1;

	icmd = &elsiocb->iocb;
	oldcmd = &oldiocb->iocb;
	icmd->ulpContext = oldcmd->ulpContext;	/* Xri */
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

	*((uint32_t *) (pcmd)) = ELS_CMD_LS_RJT;
	pcmd += sizeof(uint32_t);
	*((uint32_t *) (pcmd)) = rejectError;

	if (mbox)
		elsiocb->context_un.mbox = mbox;

	/* Xmit ELS RJT <err> response tag <ulpIoTag> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0129 Xmit ELS RJT x%x response tag x%x "
			 "xri x%x, did x%x, nlp_flag x%x, nlp_state x%x, "
			 "rpi x%x\n",
			 rejectError, elsiocb->iotag,
			 elsiocb->iocb.ulpContext, ndlp->nlp_DID,
			 ndlp->nlp_flag, ndlp->nlp_state, ndlp->nlp_rpi);
	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
		"Issue LS_RJT:    did:x%x flg:x%x err:x%x",
		ndlp->nlp_DID, ndlp->nlp_flag, rejectError);

	phba->fc_stat.elsXmitLSRJT++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_rsp;
	rc = lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0);

	if (rc == IOCB_ERROR) {
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	return 0;
}

/**
 * lpfc_els_rsp_adisc_acc - Prepare and issue acc response to adisc iocb cmd
 * @vport: pointer to a virtual N_Port data structure.
 * @oldiocb: pointer to the original lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine prepares and issues an Accept (ACC) response to Address
 * Discover (ADISC) ELS command. It simply prepares the payload of the IOCB
 * and invokes the lpfc_sli_issue_iocb() routine to send out the command.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the ADISC Accept response ELS IOCB command.
 *
 * Return code
 *   0 - Successfully issued acc adisc response
 *   1 - Failed to issue adisc acc response
 **/
int
lpfc_els_rsp_adisc_acc(struct lpfc_vport *vport, struct lpfc_iocbq *oldiocb,
		       struct lpfc_nodelist *ndlp)
{
	struct lpfc_hba  *phba = vport->phba;
	ADISC *ap;
	IOCB_t *icmd, *oldcmd;
	struct lpfc_iocbq *elsiocb;
	uint8_t *pcmd;
	uint16_t cmdsize;
	int rc;

	cmdsize = sizeof(uint32_t) + sizeof(ADISC);
	elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_ACC);
	if (!elsiocb)
		return 1;

	icmd = &elsiocb->iocb;
	oldcmd = &oldiocb->iocb;
	icmd->ulpContext = oldcmd->ulpContext;	/* Xri */

	/* Xmit ADISC ACC response tag <ulpIoTag> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0130 Xmit ADISC ACC response iotag x%x xri: "
			 "x%x, did x%x, nlp_flag x%x, nlp_state x%x rpi x%x\n",
			 elsiocb->iotag, elsiocb->iocb.ulpContext,
			 ndlp->nlp_DID, ndlp->nlp_flag, ndlp->nlp_state,
			 ndlp->nlp_rpi);
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

	*((uint32_t *) (pcmd)) = ELS_CMD_ACC;
	pcmd += sizeof(uint32_t);

	ap = (ADISC *) (pcmd);
	ap->hardAL_PA = phba->fc_pref_ALPA;
	memcpy(&ap->portName, &vport->fc_portname, sizeof(struct lpfc_name));
	memcpy(&ap->nodeName, &vport->fc_nodename, sizeof(struct lpfc_name));
	ap->DID = be32_to_cpu(vport->fc_myDID);

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
		"Issue ACC ADISC: did:x%x flg:x%x",
		ndlp->nlp_DID, ndlp->nlp_flag, 0);

	phba->fc_stat.elsXmitACC++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_rsp;
	rc = lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0);
	if (rc == IOCB_ERROR) {
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	return 0;
}

/**
 * lpfc_els_rsp_prli_acc - Prepare and issue acc response to prli iocb cmd
 * @vport: pointer to a virtual N_Port data structure.
 * @oldiocb: pointer to the original lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine prepares and issues an Accept (ACC) response to Process
 * Login (PRLI) ELS command. It simply prepares the payload of the IOCB
 * and invokes the lpfc_sli_issue_iocb() routine to send out the command.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the PRLI Accept response ELS IOCB command.
 *
 * Return code
 *   0 - Successfully issued acc prli response
 *   1 - Failed to issue acc prli response
 **/
int
lpfc_els_rsp_prli_acc(struct lpfc_vport *vport, struct lpfc_iocbq *oldiocb,
		      struct lpfc_nodelist *ndlp)
{
	struct lpfc_hba  *phba = vport->phba;
	PRLI *npr;
	lpfc_vpd_t *vpd;
	IOCB_t *icmd;
	IOCB_t *oldcmd;
	struct lpfc_iocbq *elsiocb;
	struct lpfc_sli *psli;
	uint8_t *pcmd;
	uint16_t cmdsize;
	int rc;

	psli = &phba->sli;

	cmdsize = sizeof(uint32_t) + sizeof(PRLI);
	elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry, ndlp,
		ndlp->nlp_DID, (ELS_CMD_ACC | (ELS_CMD_PRLI & ~ELS_RSP_MASK)));
	if (!elsiocb)
		return 1;

	icmd = &elsiocb->iocb;
	oldcmd = &oldiocb->iocb;
	icmd->ulpContext = oldcmd->ulpContext;	/* Xri */
	/* Xmit PRLI ACC response tag <ulpIoTag> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0131 Xmit PRLI ACC response tag x%x xri x%x, "
			 "did x%x, nlp_flag x%x, nlp_state x%x, rpi x%x\n",
			 elsiocb->iotag, elsiocb->iocb.ulpContext,
			 ndlp->nlp_DID, ndlp->nlp_flag, ndlp->nlp_state,
			 ndlp->nlp_rpi);
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

	*((uint32_t *) (pcmd)) = (ELS_CMD_ACC | (ELS_CMD_PRLI & ~ELS_RSP_MASK));
	pcmd += sizeof(uint32_t);

	/* For PRLI, remainder of payload is PRLI parameter page */
	memset(pcmd, 0, sizeof(PRLI));

	npr = (PRLI *) pcmd;
	vpd = &phba->vpd;
	/*
	 * If the remote port is a target and our firmware version is 3.20 or
	 * later, set the following bits for FC-TAPE support.
	 */
	if ((ndlp->nlp_type & NLP_FCP_TARGET) &&
	    (vpd->rev.feaLevelHigh >= 0x02)) {
		npr->ConfmComplAllowed = 1;
		npr->Retry = 1;
		npr->TaskRetryIdReq = 1;
	}

	npr->acceptRspCode = PRLI_REQ_EXECUTED;
	npr->estabImagePair = 1;
	npr->readXferRdyDis = 1;
	npr->ConfmComplAllowed = 1;

	npr->prliType = PRLI_FCP_TYPE;
	npr->initiatorFunc = 1;

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
		"Issue ACC PRLI:  did:x%x flg:x%x",
		ndlp->nlp_DID, ndlp->nlp_flag, 0);

	phba->fc_stat.elsXmitACC++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_rsp;

	rc = lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0);
	if (rc == IOCB_ERROR) {
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	return 0;
}

/**
 * lpfc_els_rsp_rnid_acc - Issue rnid acc response iocb command
 * @vport: pointer to a virtual N_Port data structure.
 * @format: rnid command format.
 * @oldiocb: pointer to the original lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine issues a Request Node Identification Data (RNID) Accept
 * (ACC) response. It constructs the RNID ACC response command according to
 * the proper @format and then calls the lpfc_sli_issue_iocb() routine to
 * issue the response. Note that this command does not need to hold the ndlp
 * reference count for the callback. So, the ndlp reference count taken by
 * the lpfc_prep_els_iocb() routine is put back and the context1 field of
 * IOCB is set to NULL to indicate to the lpfc_els_free_iocb() routine that
 * there is no ndlp reference available.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function. However, for the RNID Accept Response ELS command,
 * this is undone later by this routine after the IOCB is allocated.
 *
 * Return code
 *   0 - Successfully issued acc rnid response
 *   1 - Failed to issue acc rnid response
 **/
static int
lpfc_els_rsp_rnid_acc(struct lpfc_vport *vport, uint8_t format,
		      struct lpfc_iocbq *oldiocb, struct lpfc_nodelist *ndlp)
{
	struct lpfc_hba  *phba = vport->phba;
	RNID *rn;
	IOCB_t *icmd, *oldcmd;
	struct lpfc_iocbq *elsiocb;
	struct lpfc_sli *psli;
	uint8_t *pcmd;
	uint16_t cmdsize;
	int rc;

	psli = &phba->sli;
	cmdsize = sizeof(uint32_t) + sizeof(uint32_t)
					+ (2 * sizeof(struct lpfc_name));
	if (format)
		cmdsize += sizeof(RNID_TOP_DISC);

	elsiocb = lpfc_prep_els_iocb(vport, 0, cmdsize, oldiocb->retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_ACC);
	if (!elsiocb)
		return 1;

	icmd = &elsiocb->iocb;
	oldcmd = &oldiocb->iocb;
	icmd->ulpContext = oldcmd->ulpContext;	/* Xri */
	/* Xmit RNID ACC response tag <ulpIoTag> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0132 Xmit RNID ACC response tag x%x xri x%x\n",
			 elsiocb->iotag, elsiocb->iocb.ulpContext);
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);
	*((uint32_t *) (pcmd)) = ELS_CMD_ACC;
	pcmd += sizeof(uint32_t);

	memset(pcmd, 0, sizeof(RNID));
	rn = (RNID *) (pcmd);
	rn->Format = format;
	rn->CommonLen = (2 * sizeof(struct lpfc_name));
	memcpy(&rn->portName, &vport->fc_portname, sizeof(struct lpfc_name));
	memcpy(&rn->nodeName, &vport->fc_nodename, sizeof(struct lpfc_name));
	switch (format) {
	case 0:
		rn->SpecificLen = 0;
		break;
	case RNID_TOPOLOGY_DISC:
		rn->SpecificLen = sizeof(RNID_TOP_DISC);
		memcpy(&rn->un.topologyDisc.portName,
		       &vport->fc_portname, sizeof(struct lpfc_name));
		rn->un.topologyDisc.unitType = RNID_HBA;
		rn->un.topologyDisc.physPort = 0;
		rn->un.topologyDisc.attachedNodes = 0;
		break;
	default:
		rn->CommonLen = 0;
		rn->SpecificLen = 0;
		break;
	}

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
		"Issue ACC RNID:  did:x%x flg:x%x",
		ndlp->nlp_DID, ndlp->nlp_flag, 0);

	phba->fc_stat.elsXmitACC++;
	elsiocb->iocb_cmpl = lpfc_cmpl_els_rsp;
	lpfc_nlp_put(ndlp);
	elsiocb->context1 = NULL;  /* Don't need ndlp for cmpl,
				    * it could be freed */

	rc = lpfc_sli_issue_iocb(phba, LPFC_ELS_RING, elsiocb, 0);
	if (rc == IOCB_ERROR) {
		lpfc_els_free_iocb(phba, elsiocb);
		return 1;
	}
	return 0;
}

/**
 * lpfc_els_disc_adisc - Issue remaining adisc iocbs to npr nodes of a vport
 * @vport: pointer to a host virtual N_Port data structure.
 *
 * This routine issues Address Discover (ADISC) ELS commands to those
 * N_Ports which are in node port recovery state and ADISC has not been issued
 * for the @vport. Each time an ELS ADISC IOCB is issued by invoking the
 * lpfc_issue_els_adisc() routine, the per @vport number of discover count
 * (num_disc_nodes) shall be incremented. If the num_disc_nodes reaches a
 * pre-configured threshold (cfg_discovery_threads), the @vport fc_flag will
 * be marked with FC_NLP_MORE bit and the process of issuing remaining ADISC
 * IOCBs quit for later pick up. On the other hand, after walking through
 * all the ndlps with the @vport and there is none ADISC IOCB issued, the
 * @vport fc_flag shall be cleared with FC_NLP_MORE bit indicating there is
 * no more ADISC need to be sent.
 *
 * Return code
 *    The number of N_Ports with adisc issued.
 **/
int
lpfc_els_disc_adisc(struct lpfc_vport *vport)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_nodelist *ndlp, *next_ndlp;
	int sentadisc = 0;

	/* go thru NPR nodes and issue any remaining ELS ADISCs */
	list_for_each_entry_safe(ndlp, next_ndlp, &vport->fc_nodes, nlp_listp) {
		if (!NLP_CHK_NODE_ACT(ndlp))
			continue;
		if (ndlp->nlp_state == NLP_STE_NPR_NODE &&
		    (ndlp->nlp_flag & NLP_NPR_2B_DISC) != 0 &&
		    (ndlp->nlp_flag & NLP_NPR_ADISC) != 0) {
			spin_lock_irq(shost->host_lock);
			ndlp->nlp_flag &= ~NLP_NPR_ADISC;
			spin_unlock_irq(shost->host_lock);
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_ADISC_ISSUE);
			lpfc_issue_els_adisc(vport, ndlp, 0);
			sentadisc++;
			vport->num_disc_nodes++;
			if (vport->num_disc_nodes >=
			    vport->cfg_discovery_threads) {
				spin_lock_irq(shost->host_lock);
				vport->fc_flag |= FC_NLP_MORE;
				spin_unlock_irq(shost->host_lock);
				break;
			}
		}
	}
	if (sentadisc == 0) {
		spin_lock_irq(shost->host_lock);
		vport->fc_flag &= ~FC_NLP_MORE;
		spin_unlock_irq(shost->host_lock);
	}
	return sentadisc;
}

/**
 * lpfc_els_disc_plogi - Issue plogi for all npr nodes of a vport before adisc
 * @vport: pointer to a host virtual N_Port data structure.
 *
 * This routine issues Port Login (PLOGI) ELS commands to all the N_Ports
 * which are in node port recovery state, with a @vport. Each time an ELS
 * ADISC PLOGI IOCB is issued by invoking the lpfc_issue_els_plogi() routine,
 * the per @vport number of discover count (num_disc_nodes) shall be
 * incremented. If the num_disc_nodes reaches a pre-configured threshold
 * (cfg_discovery_threads), the @vport fc_flag will be marked with FC_NLP_MORE
 * bit set and quit the process of issuing remaining ADISC PLOGIN IOCBs for
 * later pick up. On the other hand, after walking through all the ndlps with
 * the @vport and there is none ADISC PLOGI IOCB issued, the @vport fc_flag
 * shall be cleared with the FC_NLP_MORE bit indicating there is no more ADISC
 * PLOGI need to be sent.
 *
 * Return code
 *   The number of N_Ports with plogi issued.
 **/
int
lpfc_els_disc_plogi(struct lpfc_vport *vport)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_nodelist *ndlp, *next_ndlp;
	int sentplogi = 0;

	/* go thru NPR nodes and issue any remaining ELS PLOGIs */
	list_for_each_entry_safe(ndlp, next_ndlp, &vport->fc_nodes, nlp_listp) {
		if (!NLP_CHK_NODE_ACT(ndlp))
			continue;
		if (ndlp->nlp_state == NLP_STE_NPR_NODE &&
		    (ndlp->nlp_flag & NLP_NPR_2B_DISC) != 0 &&
		    (ndlp->nlp_flag & NLP_DELAY_TMO) == 0 &&
		    (ndlp->nlp_flag & NLP_NPR_ADISC) == 0) {
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_PLOGI_ISSUE);
			lpfc_issue_els_plogi(vport, ndlp->nlp_DID, 0);
			sentplogi++;
			vport->num_disc_nodes++;
			if (vport->num_disc_nodes >=
			    vport->cfg_discovery_threads) {
				spin_lock_irq(shost->host_lock);
				vport->fc_flag |= FC_NLP_MORE;
				spin_unlock_irq(shost->host_lock);
				break;
			}
		}
	}
	if (sentplogi) {
		lpfc_set_disctmo(vport);
	}
	else {
		spin_lock_irq(shost->host_lock);
		vport->fc_flag &= ~FC_NLP_MORE;
		spin_unlock_irq(shost->host_lock);
	}
	return sentplogi;
}

/**
 * lpfc_els_flush_rscn - Clean up any rscn activities with a vport
 * @vport: pointer to a host virtual N_Port data structure.
 *
 * This routine cleans up any Registration State Change Notification
 * (RSCN) activity with a @vport. Note that the fc_rscn_flush flag of the
 * @vport together with the host_lock is used to prevent multiple thread
 * trying to access the RSCN array on a same @vport at the same time.
 **/
void
lpfc_els_flush_rscn(struct lpfc_vport *vport)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	int i;

	spin_lock_irq(shost->host_lock);
	if (vport->fc_rscn_flush) {
		/* Another thread is walking fc_rscn_id_list on this vport */
		spin_unlock_irq(shost->host_lock);
		return;
	}
	/* Indicate we are walking lpfc_els_flush_rscn on this vport */
	vport->fc_rscn_flush = 1;
	spin_unlock_irq(shost->host_lock);

	for (i = 0; i < vport->fc_rscn_id_cnt; i++) {
		lpfc_in_buf_free(phba, vport->fc_rscn_id_list[i]);
		vport->fc_rscn_id_list[i] = NULL;
	}
	spin_lock_irq(shost->host_lock);
	vport->fc_rscn_id_cnt = 0;
	vport->fc_flag &= ~(FC_RSCN_MODE | FC_RSCN_DISCOVERY);
	spin_unlock_irq(shost->host_lock);
	lpfc_can_disctmo(vport);
	/* Indicate we are done walking this fc_rscn_id_list */
	vport->fc_rscn_flush = 0;
}

/**
 * lpfc_rscn_payload_check - Check whether there is a pending rscn to a did
 * @vport: pointer to a host virtual N_Port data structure.
 * @did: remote destination port identifier.
 *
 * This routine checks whether there is any pending Registration State
 * Configuration Notification (RSCN) to a @did on @vport.
 *
 * Return code
 *   None zero - The @did matched with a pending rscn
 *   0 - not able to match @did with a pending rscn
 **/
int
lpfc_rscn_payload_check(struct lpfc_vport *vport, uint32_t did)
{
	D_ID ns_did;
	D_ID rscn_did;
	uint32_t *lp;
	uint32_t payload_len, i;
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);

	ns_did.un.word = did;

	/* Never match fabric nodes for RSCNs */
	if ((did & Fabric_DID_MASK) == Fabric_DID_MASK)
		return 0;

	/* If we are doing a FULL RSCN rediscovery, match everything */
	if (vport->fc_flag & FC_RSCN_DISCOVERY)
		return did;

	spin_lock_irq(shost->host_lock);
	if (vport->fc_rscn_flush) {
		/* Another thread is walking fc_rscn_id_list on this vport */
		spin_unlock_irq(shost->host_lock);
		return 0;
	}
	/* Indicate we are walking fc_rscn_id_list on this vport */
	vport->fc_rscn_flush = 1;
	spin_unlock_irq(shost->host_lock);
	for (i = 0; i < vport->fc_rscn_id_cnt; i++) {
		lp = vport->fc_rscn_id_list[i]->virt;
		payload_len = be32_to_cpu(*lp++ & ~ELS_CMD_MASK);
		payload_len -= sizeof(uint32_t);	/* take off word 0 */
		while (payload_len) {
			rscn_did.un.word = be32_to_cpu(*lp++);
			payload_len -= sizeof(uint32_t);
			switch (rscn_did.un.b.resv & RSCN_ADDRESS_FORMAT_MASK) {
			case RSCN_ADDRESS_FORMAT_PORT:
				if ((ns_did.un.b.domain == rscn_did.un.b.domain)
				    && (ns_did.un.b.area == rscn_did.un.b.area)
				    && (ns_did.un.b.id == rscn_did.un.b.id))
					goto return_did_out;
				break;
			case RSCN_ADDRESS_FORMAT_AREA:
				if ((ns_did.un.b.domain == rscn_did.un.b.domain)
				    && (ns_did.un.b.area == rscn_did.un.b.area))
					goto return_did_out;
				break;
			case RSCN_ADDRESS_FORMAT_DOMAIN:
				if (ns_did.un.b.domain == rscn_did.un.b.domain)
					goto return_did_out;
				break;
			case RSCN_ADDRESS_FORMAT_FABRIC:
				goto return_did_out;
			}
		}
	}
	/* Indicate we are done with walking fc_rscn_id_list on this vport */
	vport->fc_rscn_flush = 0;
	return 0;
return_did_out:
	/* Indicate we are done with walking fc_rscn_id_list on this vport */
	vport->fc_rscn_flush = 0;
	return did;
}

/**
 * lpfc_rscn_recovery_check - Send recovery event to vport nodes matching rscn
 * @vport: pointer to a host virtual N_Port data structure.
 *
 * This routine sends recovery (NLP_EVT_DEVICE_RECOVERY) event to the
 * state machine for a @vport's nodes that are with pending RSCN (Registration
 * State Change Notification).
 *
 * Return code
 *   0 - Successful (currently alway return 0)
 **/
static int
lpfc_rscn_recovery_check(struct lpfc_vport *vport)
{
	struct lpfc_nodelist *ndlp = NULL;

	/* Move all affected nodes by pending RSCNs to NPR state. */
	list_for_each_entry(ndlp, &vport->fc_nodes, nlp_listp) {
		if (!NLP_CHK_NODE_ACT(ndlp) ||
		    (ndlp->nlp_state == NLP_STE_UNUSED_NODE) ||
		    !lpfc_rscn_payload_check(vport, ndlp->nlp_DID))
			continue;
		lpfc_disc_state_machine(vport, ndlp, NULL,
					NLP_EVT_DEVICE_RECOVERY);
		lpfc_cancel_retry_delay_tmo(vport, ndlp);
	}
	return 0;
}

/**
 * lpfc_send_rscn_event - Send an RSCN event to management application
 * @vport: pointer to a host virtual N_Port data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 *
 * lpfc_send_rscn_event sends an RSCN netlink event to management
 * applications.
 */
static void
lpfc_send_rscn_event(struct lpfc_vport *vport,
		struct lpfc_iocbq *cmdiocb)
{
	struct lpfc_dmabuf *pcmd;
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	uint32_t *payload_ptr;
	uint32_t payload_len;
	struct lpfc_rscn_event_header *rscn_event_data;

	pcmd = (struct lpfc_dmabuf *) cmdiocb->context2;
	payload_ptr = (uint32_t *) pcmd->virt;
	payload_len = be32_to_cpu(*payload_ptr & ~ELS_CMD_MASK);

	rscn_event_data = kmalloc(sizeof(struct lpfc_rscn_event_header) +
		payload_len, GFP_KERNEL);
	if (!rscn_event_data) {
		lpfc_printf_vlog(vport, KERN_ERR, LOG_ELS,
			"0147 Failed to allocate memory for RSCN event\n");
		return;
	}
	rscn_event_data->event_type = FC_REG_RSCN_EVENT;
	rscn_event_data->payload_length = payload_len;
	memcpy(rscn_event_data->rscn_payload, payload_ptr,
		payload_len);

	fc_host_post_vendor_event(shost,
		fc_get_event_number(),
		sizeof(struct lpfc_els_event_header) + payload_len,
		(char *)rscn_event_data,
		LPFC_NL_VENDOR_ID);

	kfree(rscn_event_data);
}

/**
 * lpfc_els_rcv_rscn - Process an unsolicited rscn iocb
 * @vport: pointer to a host virtual N_Port data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine processes an unsolicited RSCN (Registration State Change
 * Notification) IOCB. First, the payload of the unsolicited RSCN is walked
 * to invoke fc_host_post_event() routine to the FC transport layer. If the
 * discover state machine is about to begin discovery, it just accepts the
 * RSCN and the discovery process will satisfy the RSCN. If this RSCN only
 * contains N_Port IDs for other vports on this HBA, it just accepts the
 * RSCN and ignore processing it. If the state machine is in the recovery
 * state, the fc_rscn_id_list of this @vport is walked and the
 * lpfc_rscn_recovery_check() routine is invoked to send recovery event for
 * all nodes that match RSCN payload. Otherwise, the lpfc_els_handle_rscn()
 * routine is invoked to handle the RSCN event.
 *
 * Return code
 *   0 - Just sent the acc response
 *   1 - Sent the acc response and waited for name server completion
 **/
static int
lpfc_els_rcv_rscn(struct lpfc_vport *vport, struct lpfc_iocbq *cmdiocb,
		  struct lpfc_nodelist *ndlp)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	struct lpfc_dmabuf *pcmd;
	uint32_t *lp, *datap;
	IOCB_t *icmd;
	uint32_t payload_len, length, nportid, *cmd;
	int rscn_cnt;
	int rscn_id = 0, hba_id = 0;
	int i;

	icmd = &cmdiocb->iocb;
	pcmd = (struct lpfc_dmabuf *) cmdiocb->context2;
	lp = (uint32_t *) pcmd->virt;

	payload_len = be32_to_cpu(*lp++ & ~ELS_CMD_MASK);
	payload_len -= sizeof(uint32_t);	/* take off word 0 */
	/* RSCN received */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_DISCOVERY,
			 "0214 RSCN received Data: x%x x%x x%x x%x\n",
			 vport->fc_flag, payload_len, *lp,
			 vport->fc_rscn_id_cnt);

	/* Send an RSCN event to the management application */
	lpfc_send_rscn_event(vport, cmdiocb);

	for (i = 0; i < payload_len/sizeof(uint32_t); i++)
		fc_host_post_event(shost, fc_get_event_number(),
			FCH_EVT_RSCN, lp[i]);

	/* If we are about to begin discovery, just ACC the RSCN.
	 * Discovery processing will satisfy it.
	 */
	if (vport->port_state <= LPFC_NS_QRY) {
		lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_UNSOL,
			"RCV RSCN ignore: did:x%x/ste:x%x flg:x%x",
			ndlp->nlp_DID, vport->port_state, ndlp->nlp_flag);

		lpfc_els_rsp_acc(vport, ELS_CMD_ACC, cmdiocb, ndlp, NULL);
		return 0;
	}

	/* If this RSCN just contains NPortIDs for other vports on this HBA,
	 * just ACC and ignore it.
	 */
	if ((phba->sli3_options & LPFC_SLI3_NPIV_ENABLED) &&
		!(vport->cfg_peer_port_login)) {
		i = payload_len;
		datap = lp;
		while (i > 0) {
			nportid = *datap++;
			nportid = ((be32_to_cpu(nportid)) & Mask_DID);
			i -= sizeof(uint32_t);
			rscn_id++;
			if (lpfc_find_vport_by_did(phba, nportid))
				hba_id++;
		}
		if (rscn_id == hba_id) {
			/* ALL NPortIDs in RSCN are on HBA */
			lpfc_printf_vlog(vport, KERN_INFO, LOG_DISCOVERY,
					 "0219 Ignore RSCN "
					 "Data: x%x x%x x%x x%x\n",
					 vport->fc_flag, payload_len,
					 *lp, vport->fc_rscn_id_cnt);
			lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_UNSOL,
				"RCV RSCN vport:  did:x%x/ste:x%x flg:x%x",
				ndlp->nlp_DID, vport->port_state,
				ndlp->nlp_flag);

			lpfc_els_rsp_acc(vport, ELS_CMD_ACC, cmdiocb,
				ndlp, NULL);
			return 0;
		}
	}

	spin_lock_irq(shost->host_lock);
	if (vport->fc_rscn_flush) {
		/* Another thread is walking fc_rscn_id_list on this vport */
		spin_unlock_irq(shost->host_lock);
		vport->fc_flag |= FC_RSCN_DISCOVERY;
		/* Send back ACC */
		lpfc_els_rsp_acc(vport, ELS_CMD_ACC, cmdiocb, ndlp, NULL);
		return 0;
	}
	/* Indicate we are walking fc_rscn_id_list on this vport */
	vport->fc_rscn_flush = 1;
	spin_unlock_irq(shost->host_lock);
	/* Get the array count after sucessfully have the token */
	rscn_cnt = vport->fc_rscn_id_cnt;
	/* If we are already processing an RSCN, save the received
	 * RSCN payload buffer, cmdiocb->context2 to process later.
	 */
	if (vport->fc_flag & (FC_RSCN_MODE | FC_NDISC_ACTIVE)) {
		lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_UNSOL,
			"RCV RSCN defer:  did:x%x/ste:x%x flg:x%x",
			ndlp->nlp_DID, vport->port_state, ndlp->nlp_flag);

		spin_lock_irq(shost->host_lock);
		vport->fc_flag |= FC_RSCN_DEFERRED;
		if ((rscn_cnt < FC_MAX_HOLD_RSCN) &&
		    !(vport->fc_flag & FC_RSCN_DISCOVERY)) {
			vport->fc_flag |= FC_RSCN_MODE;
			spin_unlock_irq(shost->host_lock);
			if (rscn_cnt) {
				cmd = vport->fc_rscn_id_list[rscn_cnt-1]->virt;
				length = be32_to_cpu(*cmd & ~ELS_CMD_MASK);
			}
			if ((rscn_cnt) &&
			    (payload_len + length <= LPFC_BPL_SIZE)) {
				*cmd &= ELS_CMD_MASK;
				*cmd |= cpu_to_be32(payload_len + length);
				memcpy(((uint8_t *)cmd) + length, lp,
				       payload_len);
			} else {
				vport->fc_rscn_id_list[rscn_cnt] = pcmd;
				vport->fc_rscn_id_cnt++;
				/* If we zero, cmdiocb->context2, the calling
				 * routine will not try to free it.
				 */
				cmdiocb->context2 = NULL;
			}
			/* Deferred RSCN */
			lpfc_printf_vlog(vport, KERN_INFO, LOG_DISCOVERY,
					 "0235 Deferred RSCN "
					 "Data: x%x x%x x%x\n",
					 vport->fc_rscn_id_cnt, vport->fc_flag,
					 vport->port_state);
		} else {
			vport->fc_flag |= FC_RSCN_DISCOVERY;
			spin_unlock_irq(shost->host_lock);
			/* ReDiscovery RSCN */
			lpfc_printf_vlog(vport, KERN_INFO, LOG_DISCOVERY,
					 "0234 ReDiscovery RSCN "
					 "Data: x%x x%x x%x\n",
					 vport->fc_rscn_id_cnt, vport->fc_flag,
					 vport->port_state);
		}
		/* Indicate we are done walking fc_rscn_id_list on this vport */
		vport->fc_rscn_flush = 0;
		/* Send back ACC */
		lpfc_els_rsp_acc(vport, ELS_CMD_ACC, cmdiocb, ndlp, NULL);
		/* send RECOVERY event for ALL nodes that match RSCN payload */
		lpfc_rscn_recovery_check(vport);
		spin_lock_irq(shost->host_lock);
		vport->fc_flag &= ~FC_RSCN_DEFERRED;
		spin_unlock_irq(shost->host_lock);
		return 0;
	}
	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_UNSOL,
		"RCV RSCN:        did:x%x/ste:x%x flg:x%x",
		ndlp->nlp_DID, vport->port_state, ndlp->nlp_flag);

	spin_lock_irq(shost->host_lock);
	vport->fc_flag |= FC_RSCN_MODE;
	spin_unlock_irq(shost->host_lock);
	vport->fc_rscn_id_list[vport->fc_rscn_id_cnt++] = pcmd;
	/* Indicate we are done walking fc_rscn_id_list on this vport */
	vport->fc_rscn_flush = 0;
	/*
	 * If we zero, cmdiocb->context2, the calling routine will
	 * not try to free it.
	 */
	cmdiocb->context2 = NULL;
	lpfc_set_disctmo(vport);
	/* Send back ACC */
	lpfc_els_rsp_acc(vport, ELS_CMD_ACC, cmdiocb, ndlp, NULL);
	/* send RECOVERY event for ALL nodes that match RSCN payload */
	lpfc_rscn_recovery_check(vport);
	return lpfc_els_handle_rscn(vport);
}

/**
 * lpfc_els_handle_rscn - Handle rscn for a vport
 * @vport: pointer to a host virtual N_Port data structure.
 *
 * This routine handles the Registration State Configuration Notification
 * (RSCN) for a @vport. If login to NameServer does not exist, a new ndlp shall
 * be created and a Port Login (PLOGI) to the NameServer is issued. Otherwise,
 * if the ndlp to NameServer exists, a Common Transport (CT) command to the
 * NameServer shall be issued. If CT command to the NameServer fails to be
 * issued, the lpfc_els_flush_rscn() routine shall be invoked to clean up any
 * RSCN activities with the @vport.
 *
 * Return code
 *   0 - Cleaned up rscn on the @vport
 *   1 - Wait for plogi to name server before proceed
 **/
int
lpfc_els_handle_rscn(struct lpfc_vport *vport)
{
	struct lpfc_nodelist *ndlp;
	struct lpfc_hba *phba = vport->phba;

	/* Ignore RSCN if the port is being torn down. */
	if (vport->load_flag & FC_UNLOADING) {
		lpfc_els_flush_rscn(vport);
		return 0;
	}

	/* Start timer for RSCN processing */
	lpfc_set_disctmo(vport);

	/* RSCN processed */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_DISCOVERY,
			 "0215 RSCN processed Data: x%x x%x x%x x%x\n",
			 vport->fc_flag, 0, vport->fc_rscn_id_cnt,
			 vport->port_state);

	/* To process RSCN, first compare RSCN data with NameServer */
	vport->fc_ns_retry = 0;
	vport->num_disc_nodes = 0;

	ndlp = lpfc_findnode_did(vport, NameServer_DID);
	if (ndlp && NLP_CHK_NODE_ACT(ndlp)
	    && ndlp->nlp_state == NLP_STE_UNMAPPED_NODE) {
		/* Good ndlp, issue CT Request to NameServer */
		if (lpfc_ns_cmd(vport, SLI_CTNS_GID_FT, 0, 0) == 0)
			/* Wait for NameServer query cmpl before we can
			   continue */
			return 1;
	} else {
		/* If login to NameServer does not exist, issue one */
		/* Good status, issue PLOGI to NameServer */
		ndlp = lpfc_findnode_did(vport, NameServer_DID);
		if (ndlp && NLP_CHK_NODE_ACT(ndlp))
			/* Wait for NameServer login cmpl before we can
			   continue */
			return 1;

		if (ndlp) {
			ndlp = lpfc_enable_node(vport, ndlp,
						NLP_STE_PLOGI_ISSUE);
			if (!ndlp) {
				lpfc_els_flush_rscn(vport);
				return 0;
			}
			ndlp->nlp_prev_state = NLP_STE_UNUSED_NODE;
		} else {
			ndlp = mempool_alloc(phba->nlp_mem_pool, GFP_KERNEL);
			if (!ndlp) {
				lpfc_els_flush_rscn(vport);
				return 0;
			}
			lpfc_nlp_init(vport, ndlp, NameServer_DID);
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_PLOGI_ISSUE);
		}
		ndlp->nlp_type |= NLP_FABRIC;
		lpfc_issue_els_plogi(vport, NameServer_DID, 0);
		/* Wait for NameServer login cmpl before we can
		 * continue
		 */
		return 1;
	}

	lpfc_els_flush_rscn(vport);
	return 0;
}

/**
 * lpfc_els_rcv_flogi - Process an unsolicited flogi iocb
 * @vport: pointer to a host virtual N_Port data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine processes Fabric Login (FLOGI) IOCB received as an ELS
 * unsolicited event. An unsolicited FLOGI can be received in a point-to-
 * point topology. As an unsolicited FLOGI should not be received in a loop
 * mode, any unsolicited FLOGI received in loop mode shall be ignored. The
 * lpfc_check_sparm() routine is invoked to check the parameters in the
 * unsolicited FLOGI. If parameters validation failed, the routine
 * lpfc_els_rsp_reject() shall be called with reject reason code set to
 * LSEXP_SPARM_OPTIONS to reject the FLOGI. Otherwise, the Port WWN in the
 * FLOGI shall be compared with the Port WWN of the @vport to determine who
 * will initiate PLOGI. The higher lexicographical value party shall has
 * higher priority (as the winning port) and will initiate PLOGI and
 * communicate Port_IDs (Addresses) for both nodes in PLOGI. The result
 * of this will be marked in the @vport fc_flag field with FC_PT2PT_PLOGI
 * and then the lpfc_els_rsp_acc() routine is invoked to accept the FLOGI.
 *
 * Return code
 *   0 - Successfully processed the unsolicited flogi
 *   1 - Failed to process the unsolicited flogi
 **/
static int
lpfc_els_rcv_flogi(struct lpfc_vport *vport, struct lpfc_iocbq *cmdiocb,
		   struct lpfc_nodelist *ndlp)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	struct lpfc_dmabuf *pcmd = (struct lpfc_dmabuf *) cmdiocb->context2;
	uint32_t *lp = (uint32_t *) pcmd->virt;
	IOCB_t *icmd = &cmdiocb->iocb;
	struct serv_parm *sp;
	LPFC_MBOXQ_t *mbox;
	struct ls_rjt stat;
	uint32_t cmd, did;
	int rc;

	cmd = *lp++;
	sp = (struct serv_parm *) lp;

	/* FLOGI received */

	lpfc_set_disctmo(vport);

	if (phba->fc_topology == TOPOLOGY_LOOP) {
		/* We should never receive a FLOGI in loop mode, ignore it */
		did = icmd->un.elsreq64.remoteID;

		/* An FLOGI ELS command <elsCmd> was received from DID <did> in
		   Loop Mode */
		lpfc_printf_vlog(vport, KERN_ERR, LOG_ELS,
				 "0113 An FLOGI ELS command x%x was "
				 "received from DID x%x in Loop Mode\n",
				 cmd, did);
		return 1;
	}

	did = Fabric_DID;

	if ((lpfc_check_sparm(vport, ndlp, sp, CLASS3))) {
		/* For a FLOGI we accept, then if our portname is greater
		 * then the remote portname we initiate Nport login.
		 */

		rc = memcmp(&vport->fc_portname, &sp->portName,
			    sizeof(struct lpfc_name));

		if (!rc) {
			mbox = mempool_alloc(phba->mbox_mem_pool, GFP_KERNEL);
			if (!mbox)
				return 1;

			lpfc_linkdown(phba);
			lpfc_init_link(phba, mbox,
				       phba->cfg_topology,
				       phba->cfg_link_speed);
			mbox->u.mb.un.varInitLnk.lipsr_AL_PA = 0;
			mbox->mbox_cmpl = lpfc_sli_def_mbox_cmpl;
			mbox->vport = vport;
			rc = lpfc_sli_issue_mbox(phba, mbox, MBX_NOWAIT);
			lpfc_set_loopback_flag(phba);
			if (rc == MBX_NOT_FINISHED) {
				mempool_free(mbox, phba->mbox_mem_pool);
			}
			return 1;
		} else if (rc > 0) {	/* greater than */
			spin_lock_irq(shost->host_lock);
			vport->fc_flag |= FC_PT2PT_PLOGI;
			spin_unlock_irq(shost->host_lock);
		}
		spin_lock_irq(shost->host_lock);
		vport->fc_flag |= FC_PT2PT;
		vport->fc_flag &= ~(FC_FABRIC | FC_PUBLIC_LOOP);
		spin_unlock_irq(shost->host_lock);
	} else {
		/* Reject this request because invalid parameters */
		stat.un.b.lsRjtRsvd0 = 0;
		stat.un.b.lsRjtRsnCode = LSRJT_UNABLE_TPC;
		stat.un.b.lsRjtRsnCodeExp = LSEXP_SPARM_OPTIONS;
		stat.un.b.vendorUnique = 0;
		lpfc_els_rsp_reject(vport, stat.un.lsRjtError, cmdiocb, ndlp,
			NULL);
		return 1;
	}

	/* Send back ACC */
	lpfc_els_rsp_acc(vport, ELS_CMD_PLOGI, cmdiocb, ndlp, NULL);

	return 0;
}

/**
 * lpfc_els_rcv_rnid - Process an unsolicited rnid iocb
 * @vport: pointer to a host virtual N_Port data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @ndlp: pointer to a node-list data structure.
 *
 * This routine processes Request Node Identification Data (RNID) IOCB
 * re