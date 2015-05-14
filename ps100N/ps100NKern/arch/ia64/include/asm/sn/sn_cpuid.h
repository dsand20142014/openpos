 remaining ELS ADISCs */
		sentadisc = lpfc_els_disc_adisc(vport);
	}
	if (!vport->num_disc_nodes)
		lpfc_adisc_done(vport);
	return;
}

/**
 * lpfc_cmpl_els_adisc - Completion callback function for adisc
 * @phba: pointer to lpfc hba data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @rspiocb: pointer to lpfc response iocb data structure.
 *
 * This routine is the completion function for issuing the Address Discover
 * (ADISC) command. It first checks to see whether link went down during
 * the discovery process. If so, the node will be marked as node port
 * recovery for issuing discover IOCB by the link attention handler and
 * exit. Otherwise, the response status is checked. If error was reported
 * in the response status, the ADISC command shall be retried by invoking
 * the lpfc_els_retry() routine. Otherwise, if no error was reported in
 * the response status, the state machine is invoked to set transition
 * with respect to NLP_EVT_CMPL_ADISC event.
 **/
static void
lpfc_cmpl_els_adisc(struct lpfc_hba *phba, struct lpfc_iocbq *cmdiocb,
		    struct lpfc_iocbq *rspiocb)
{
	struct lpfc_vport *vport = cmdiocb->vport;
	struct Scsi_Host  *shost = lpfc_shost_from_vport(vport);
	IOCB_t *irsp;
	struct lpfc_nodelist *ndlp;
	int  disc;

	/* we pass cmdiocb to state machine which needs rspiocb as well */
	cmdiocb->context_un.rsp_iocb = rspiocb;

	irsp = &(rspiocb->iocb);
	ndlp = (struct lpfc_nodelist *) cmdiocb->context1;

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"ADISC cmpl:      status:x%x/x%x did:x%x",
		irsp->ulpStatus, irsp->un.ulpWord[4],
		ndlp->nlp_DID);

	/* Since ndlp can be freed in the disc state machine, note if this node
	 * is being used during discovery.
	 */
	spin_lock_irq(shost->host_lock);
	disc = (ndlp->nlp_flag & NLP_NPR_2B_DISC);
	ndlp->nlp_flag &= ~(NLP_ADISC_SND | NLP_NPR_2B_DISC);
	spin_unlock_irq(shost->host_lock);
	/* ADISC completes to NPort <nlp_DID> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0104 ADISC completes to NPort x%x "
			 "Data: x%x x%x x%x x%x x%x\n",
			 ndlp->nlp_DID, irsp->ulpStatus, irsp->un.ulpWord[4],
			 irsp->ulpTimeout, disc, vport->num_disc_nodes);
	/* Check to see if link went down during discovery */
	if (lpfc_els_chk_latt(vport)) {
		spin_lock_irq(shost->host_lock);
		ndlp->nlp_flag |= NLP_NPR_2B_DISC;
		spin_unlock_irq(shost->host_lock);
		goto out;
	}

	if (irsp->ulpStatus) {
		/* Check for retry */
		if (lpfc_els_retry(phba, cmdiocb, rspiocb)) {
			/* ELS command is being retried */
			if (disc) {
				spin_lock_irq(shost->host_lock);
				ndlp->nlp_flag |= NLP_NPR_2B_DISC;
				spin_unlock_irq(shost->host_lock);
				lpfc_set_disctmo(vport);
			}
			goto out;
		}
		/* ADISC failed */
		/* Do not call DSM for lpfc_els_abort'ed ELS cmds */
		if (!lpfc_error_lost_link(irsp))
			lpfc_disc_state_machine(vport, ndlp, cmdiocb,
						NLP_EVT_CMPL_ADISC);
	} else
		/* Good status, call state machine */
		lpfc_disc_state_machine(vport, ndlp, cmdiocb,
					NLP_EVT_CMPL_ADISC);

	/* Check to see if there are more ADISCs to be sent */
	if (disc && vport->num_disc_nodes)
		lpfc_more_adisc(vport);
out:
	lpfc_els_free_iocb(phba, cmdiocb);
	return;
}

/**
 * lpfc_issue_els_adisc - Issue an address discover iocb to an node on a vport
 * @vport: pointer to a virtual N_Port data structure.
 * @ndlp: pointer to a node-list data structure.
 * @retry: number of retries to the command IOCB.
 *
 * This routine issues an Address Discover (ADISC) for an @ndlp on a
 * @vport. It prepares the payload of the ADISC ELS command, updates the
 * and states of the ndlp, and invokes the lpfc_sli_issue_iocb() routine
 * to issue the ADISC ELS command.
 *
 * Note that, in lpfc_prep_els_iocb() routine, the reference count of ndlp
 * will be incremented by 1 for holding the ndlp and the reference to ndlp
 * will be stored into the context1 field of the IOCB for the completion
 * callback function to the ADISC ELS command.
 *
 * Return code
 *   0 - successfully issued adisc
 *   1 - failed to issue adisc
 **/
int
lpfc_issue_els_adisc(struct lpfc_vport *vport, struct lpfc_nodelist *ndlp,
		     uint8_t retry)
{
	struct Scsi_Host *shost = lpfc_shost_from_vport(vport);
	struct lpfc_hba  *phba = vport->phba;
	ADISC *ap;
	IOCB_t *icmd;
	struct lpfc_iocbq *elsiocb;
	uint8_t *pcmd;
	uint16_t cmdsize;

	cmdsize = 