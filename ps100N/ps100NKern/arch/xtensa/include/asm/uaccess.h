
			 * error when they are not ready and should be returning
			 * Logical Busy. We should delay every time.
			 */
			if (cmd == ELS_CMD_FDISC &&
			    stat.un.b.lsRjtRsnCodeExp == LSEXP_PORT_LOGIN_REQ) {
				maxretry = 3;
				delay = 1000;
				retry = 1;
				break;
			}
		case LSRJT_PROTOCOL_ERR:
			if ((phba->sli3_options & LPFC_SLI3_NPIV_ENABLED) &&
			  (cmd == ELS_CMD_FDISC) &&
			  ((stat.un.b.lsRjtRsnCodeExp == LSEXP_INVALID_PNAME) ||
			  (stat.un.b.lsRjtRsnCodeExp == LSEXP_INVALID_NPORT_ID))
			  ) {
				lpfc_printf_vlog(vport, KERN_ERR, LOG_ELS,
						 "0122 FDISC Failed (x%x). "
						 "Fabric Detected Bad WWN\n",
						 stat.un.lsRjtError);
				lpfc_vport_set_state(vport,
						     FC_VPORT_FABRIC_REJ_WWN);
			}
			break;
		}
		break;

	case IOSTAT_INTERMED_RSP:
	case IOSTAT_BA_RJT:
		break;

	default:
		break;
	}

	if (did == FDMI_DID)
		retry = 1;

	if ((cmd == ELS_CMD_FLOGI) &&
	    (phba->fc_topology != TOPOLOGY_LOOP) &&
	    !lpfc_error_lost_link(irsp)) {
		/* FLOGI retry policy */
		retry = 1;
		maxretry = 48;
		if (cmdiocb->retry >= 32)
			delay = 1000;
	}

	if ((++cmdiocb->retry) >= maxretry) {
		phba->fc_stat.elsRetryExceeded++;
		retry = 0;
	}

	if ((vport->load_flag & FC_UNLOADING) != 0)
		retry = 0;

	if (retry) {

		/* Retry ELS command <elsCmd> to remote NPORT <did> */
		lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
				 "0107 Retry ELS command x%x to remote "
				 "NPORT x%x Data: x%x x%x\n",
				 cmd, did, cmdiocb->retry, delay);

		if (((cmd == ELS_CMD_PLOGI) || (cmd == ELS_CMD_ADISC)) &&
			((irsp->ulpStatus != IOSTAT_LOCAL_REJECT) ||
			((irsp->un.ulpWord[4] & 0xff) != IOERR_NO_RESOURCES))) {
			/* Don't reset timer for no resources */

			/* If discovery / RSCN timer is running, reset it */
			if (timer_pending(&vport->fc_disctmo) ||
			    (vport->fc_flag & FC_RSCN_MODE))
				lpfc_set_disctmo(vport);
		}

		phba->fc_stat.elsXmitRetry++;
		if (ndlp && NLP_CHK_NODE_ACT(ndlp) && delay) {
			phba->fc_stat.elsDelayRetry++;
			ndlp->nlp_retry = cmdiocb->retry;

			/* delay is specified in milliseconds */
			mod_timer(&ndlp->nlp_delayfunc,
				jiffies + msecs_to_jiffies(delay));
			spin_lock_irq(shost->host_lock);
			ndlp->nlp_flag |= NLP_DELAY_TMO;
			spin_unlock_irq(shost->host_lock);

			ndlp->nlp_prev_state = ndlp->nlp_state;
			if (cmd == ELS_CMD_PRLI)
				lpfc_nlp_set_state(vport, ndlp,
					NLP_STE_REG_LOGIN_ISSUE);
			else
				lpfc_nlp_set_state(vport, ndlp,
					NLP_STE_NPR_NODE);
			ndlp->nlp_last_elscmd = cmd;

			return 1;
		}
		switch (cmd) {
		case ELS_CMD_FLOGI:
			lpfc_issue_els_flogi(vport, ndlp, cmdiocb->retry);
			return 1;
		case ELS_CMD_FDISC:
			lpfc_issue_els_fdisc(vport, ndlp, cmdiocb->retry);
			return 1;
		case ELS_CMD_PLOGI:
			if (ndlp && NLP_CHK_NODE_ACT(ndlp)) {
				ndlp->nlp_prev_state = ndlp->nlp_state;
				lpfc_nlp_set_state(vport, ndlp,
						   NLP_STE_PLOGI_ISSUE);
			}
			lpfc_issue_els_plogi(vport, did, cmdiocb->retry);
			return 1;
		case ELS_CMD_ADISC:
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_ADISC_ISSUE);
			lpfc_issue_els_adisc(vport, ndlp, cmdiocb->retry);
			return 1;
		case ELS_CMD_PRLI:
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_PRLI_ISSUE);
			lpfc_issue_els_prli(vport, ndlp, cmdiocb->retry);
			return 1;
		case ELS_CMD_LOGO:
			ndlp->nlp_prev_state = ndlp->nlp_state;
			lpfc_nlp_set_state(vport, ndlp, NLP_STE_NPR_NODE);
			lpfc_issue_els_logo(vport, ndlp, cmdiocb->retry);
			return 1;
		}
	}
	/* No retry ELS command <elsCmd> to remote NPORT <did> */
	if (logerr) {
		lpfc_printf_vlog(vport, KERN_ERR, LOG_ELS,
			 "0137 No retry ELS command x%x to remote "
			 "NPORT x%x: Out of Resources: Error:x%x/%x\n",
			 cmd, did, irsp->ulpStatus,
			 irsp->un.ulpWord[4]);
	}
	else {
		lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0108 No retry ELS command x%x to remote "
			 "NPORT x%x Retried:%d Error:x%x/%x\n",
			 cmd, did, cmdiocb->retry, irsp->ulpStatus,
			 irsp->un.ulpWord[4]);
	}
	return 0;
}

/**
 * lpfc_els_free_data - Free lpfc dma buffer and data structure with an iocb
 * @phba: pointer to lpfc hba data structure.
 * @buf_ptr1: pointer to the lpfc DMA buffer data structure.
 *
 * This routine releases the lpfc DMA (Direct Memory Access) buffer(s)
 * associated with a command IOCB back to the lpfc DMA buffer pool. It first
 * checks to see whether there is a lpfc DMA buffer associated with the
 * response of the command IOCB. If so, it will be released before releasing
 * the lpfc DMA buffer associated with the IOCB itself.
 *
 * Return code
 *   0 - Successfully released lpfc DMA buffer (currently, always return 0)
 **/
static int
lpfc_els_free_data(struct lpfc_hba *phba, struct lpfc_dmabuf *buf_ptr1)
{
	struct lpfc_dmabuf *buf_ptr;

	/* Free the response before processing the command. */
	if (!list_empty(&buf_ptr1->list)) {
		list_remove_head(&buf_ptr1->list, buf_ptr,
				 struct lpfc_dmabuf,
				 list);
		lpfc_mbuf_free(phba, buf_ptr->virt, buf_ptr->phys);
		kfree(buf_ptr);
	}
	lpfc_mbuf_free(phba, buf_ptr1->virt, buf_ptr1->phys);
	kfree(buf_ptr1);
	return 0;
}

/**
 * lpfc_els_free_bpl - Free lpfc dma buffer and data structure with bpl
 * @phba: pointer to lpfc hba data structure.
 * @buf_ptr: pointer to the lpfc dma buffer data structure.
 *
 * This routine releases the lpfc Direct Memory Access (DMA) buffer
 * associated with a Buffer Pointer List (BPL) back to the lpfc DMA buffer
 * pool.
 *
 * Return code
 *   0 - Successfully released lpfc DMA buffer (currently, always return 0)
 **/
static int
lpfc_els_free_bpl(struct lpfc_hba *phba, struct lpfc_dmabuf *buf_ptr)
{
	lpfc_mbuf_free(phba, buf_ptr->virt, buf_ptr->phys);
	kfree(buf_ptr);
	return 0;
}

/**
 * lpfc_els_free_iocb - Free a command iocb and its associated resources
 * @phba: pointer to lpfc hba data structure.
 * @elsiocb: pointer to lpfc els command iocb data structure.
 *
 * This routine frees a command IOCB and its associated resources. The
 * command IOCB data structure contains the reference to various associated
 * resources, these fields must be set to NULL if the associated reference
 * not present:
 *   context1 - reference to ndlp
 *   context2 - reference to cmd
 *   context2->next - reference to rsp
 *   context3 - reference to bpl
 *
 * It first properly decrements the reference count held on ndlp for the
 * IOCB completion callback function. If LPFC_DELAY_MEM_FREE flag is not
 * set, it invokes the lpfc_els_free_data() routine to release the Direct
 * Memory Access (DMA) buffers associated with the IOCB. Otherwise, it
 * adds the DMA buffer the @phba data structure for the delayed release.
 * If reference to the Buffer Pointer List (BPL) is present, the
 * lpfc_els_free_bpl() routine is invoked to release the DMA memory
 * associated with BPL. Finally, the lpfc_sli_release_iocbq() routine is
 * invoked to release the IOCB data structure back to @phba IOCBQ list.
 *
 * Return code
 *   0 - Success (currently, always return 0)
 **/
int
lpfc_els_free_iocb(struct lpfc_hba *phba, struct lpfc_iocbq *elsiocb)
{
	struct lpfc_dmabuf *buf_ptr, *buf_ptr1;
	struct lpfc_nodelist *ndlp;

	ndlp = (struct lpfc_nodelist *)elsiocb->context1;
	if (ndlp) {
		if (ndlp->nlp_flag & NLP_DEFER_RM) {
			lpfc_nlp_put(ndlp);

			/* If the ndlp is not being used by another discovery
			 * thread, free it.
			 */
			if (!lpfc_nlp_not_used(ndlp)) {
				/* If ndlp is being used by another discovery
				 * thread, just clear NLP_DEFER_RM
				 */
				ndlp->nlp_flag &= ~NLP_DEFER_RM;
			}
		}
		else
			lpfc_nlp_put(ndlp);
		elsiocb->context1 = NULL;
	}
	/* context2  = cmd,  context2->next = rsp, context3 = bpl */
	if (elsiocb->context2) {
		if (elsiocb->iocb_flag & LPFC_DELAY_MEM_FREE) {
			/* Firmware could still be in progress of DMAing
			 * payload, so don't free data buffer till after
			 * a hbeat.
			 */
			elsiocb->iocb_flag &= ~LPFC_DELAY_MEM_FREE;
			buf_ptr = elsiocb->context2;
			elsiocb->context2 = NULL;
			if (buf_ptr) {
				buf_ptr1 = NULL;
				spin_lock_irq(&phba->hbalock);
				if (!list_empty(&buf_ptr->list)) {
					list_remove_head(&buf_ptr->list,
						buf_ptr1, struct lpfc_dmabuf,
						list);
					INIT_LIST_HEAD(&buf_ptr1->list);
					list_add_tail(&buf_ptr1->list,
						&phba->elsbuf);
					phba->elsbuf_cnt++;
				}
				INIT_LIST_HEAD(&buf_ptr->list);
				list_add_tail(&buf_ptr->list, &phba->elsbuf);
				phba->elsbuf_cnt++;
				spin_unlock_irq(&phba->hbalock);
			}
		} else {
			buf_ptr1 = (struct lpfc_dmabuf *) elsiocb->context2;
			lpfc_els_free_data(phba, buf_ptr1);
		}
	}

	if (elsiocb->context3) {
		buf_ptr = (struct lpfc_dmabuf *) elsiocb->context3;
		lpfc_els_free_bpl(phba, buf_ptr);
	}
	lpfc_sli_release_iocbq(phba, elsiocb);
	return 0;
}

/**
 * lpfc_cmpl_els_logo_acc - Completion callback function to logo acc response
 * @phba: pointer to lpfc hba data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @rspiocb: pointer to lpfc response iocb data structure.
 *
 * This routine is the completion callback function to the Logout (LOGO)
 * Accept (ACC) Response ELS command. This routine is invoked to indicate
 * the completion of the LOGO process. It invokes the lpfc_nlp_not_used() to
 * release the ndlp if it has the last reference remaining (reference count
 * is 1). If succeeded (meaning ndlp released), it sets the IOCB context1
 * field to NULL to inform the following lpfc_els_free_iocb() routine no
 * ndlp reference count needs to be decremented. Otherwise, the ndlp
 * reference use-count shall be decremented by the lpfc_els_free_iocb()
 * routine. Finally, the lpfc_els_free_iocb() is invoked to release the
 * IOCB data structure.
 **/
static void
lpfc_cmpl_els_logo_acc(struct lpfc_hba *phba, struct lpfc_iocbq *cmdiocb,
		       struct lpfc_iocbq *rspiocb)
{
	struct lpfc_nodelist *ndlp = (struct lpfc_nodelist *) cmdiocb->context1;
	struct lpfc_vport *vport = cmdiocb->vport;
	IOCB_t *irsp;

	irsp = &rspiocb->iocb;
	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_RSP,
		"ACC LOGO cmpl:   status:x%x/x%x did:x%x",
		irsp->ulpStatus, irsp->un.ulpWord[4], ndlp->nlp_DID);
	/* ACC to LOGO completes to NPort <nlp_DID> */
	lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
			 "0109 ACC to LOGO completes to NPort x%x "
			 "Data: x%x x%x x%x\n",
			 ndlp->nlp_DID, ndlp->nlp_flag, ndlp->nlp_state,
			 ndlp->nlp_rpi);

	if (ndlp->nlp_state == NLP_STE_NPR_NODE) {
		/* NPort Recovery mode or node is just allocated */
		if (!lpfc_nlp_not_used(ndlp)) {
			/* If the ndlp is being used by another discovery
			 * thread, just unregister the RPI.
			 */
			lpfc_unreg_rpi(vport, ndlp);
		} else {
			/* Indicate the node has already released, should
			 * not reference to it from within lpfc_els_free_iocb.
			 */
			cmdiocb->context1 = NULL;
		}
	}
	lpfc_els_free_iocb(phba, cmdiocb);
	return;
}

/**
 * lpfc_mbx_cmpl_dflt_rpi - Completion callbk func for unreg dflt rpi mbox cmd
 * @phba: pointer to lpfc hba data structure.
 * @pmb: pointer to the driver internal queue element for mailbox command.
 *
 * This routine is the completion callback function for unregister default
 * RPI (Remote Port Index) mailbox command to the @phba. It simply releases
 * the associated lpfc Direct Memory Access (DMA) buffer back to the pool and
 * decrements the ndlp reference count held for this completion callback
 * function. After that, it invokes the lpfc_nlp_not_used() to check
 * whether there is only one reference left on the ndlp. If so, it will
 * perform one more decrement and trigger the release of the ndlp.
 **/
void
lpfc_mbx_cmpl_dflt_rpi(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb)
{
	struct lpfc_dmabuf *mp = (struct lpfc_dmabuf *) (pmb->context1);
	struct lpfc_nodelist *ndlp = (struct lpfc_nodelist *) pmb->context2;

	/*
	 * This routine is used to register and unregister in previous SLI
	 * modes.
	 */
	if ((pmb->u.mb.mbxCommand == MBX_UNREG_LOGIN) &&
	    (phba->sli_rev == LPFC_SLI_REV4))
		lpfc_sli4_free_rpi(phba, pmb->u.mb.un.varUnregLogin.rpi);

	pmb->context1 = NULL;
	lpfc_mbuf_free(phba, mp->virt, mp->phys);
	kfree(mp);
	mempool_free(pmb, phba->mbox_mem_pool);
	if (ndlp && NLP_CHK_NODE_ACT(ndlp)) {
		lpfc_nlp_put(ndlp);
		/* This is the end of the default RPI cleanup logic for this
		 * ndlp. If no other discovery threads are using this ndlp.
		 * we should free all resources associated with it.
		 */
		lpfc_nlp_not_used(ndlp);
	}

	return;
}

/**
 * lpfc_cmpl_els_rsp - Completion callback function for els response iocb cmd
 * @phba: pointer to lpfc hba data structure.
 * @cmdiocb: pointer to lpfc command iocb data structure.
 * @rspiocb: pointer to lpfc response iocb data structure.
 *
 * This routine is the completion callback function for ELS Response IOCB
 * command. In normal case, this callback function just properly sets the
 * nlp_flag bitmap in the ndlp data structure, if the mbox command reference
 * field in the command IOCB is not NULL, the referred mailbox command will
 * be send out, and then invokes the lpfc_els_free_iocb() routine to release
 * the IOCB. Under error conditions, such as when a LS_RJT is returned or a
 * link down event occurred during the discovery, the lpfc_nlp_not_used()
 * routine shall be invoked trying to release the ndlp if no other threads
 * are currently referring it.
 **/
static void
lpfc_cmpl_els_rsp(struct lpfc_hba *phba, struct lpfc_iocbq *cmdiocb,
		  struct lpfc_iocbq *rspiocb)
{
	struct lpfc_nodelist *ndlp = (struct lpfc_nodelist *) cmdiocb->context1;
	struct lpfc_vport *vport = ndlp ? ndlp->vport : NULL;
	struct Scsi_Host  *shost = vport ? lpfc_shost_from_vport(vport) : NULL;
	IOCB_t  *irsp;
	uint8_t *pcmd;
	LPFC_MBOXQ_t *mbox = NULL;
	struct lpfc_dmabuf *mp = NULL;
	uint32_t ls_rjt = 0;

	irsp = &rspiocb->iocb;

	if (cmdiocb->context_un.mbox)
		mbox = cmdiocb->context_un.mbox;

	/* First determine if this is a LS_RJT cmpl. Note, this callback
	 * function can have cmdiocb->contest1 (ndlp) field set to NULL.
	 */
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) cmdiocb->context2)->virt);
	if (ndlp && NLP_CHK_NODE_ACT(ndlp) &&
	    (*((uint32_t *) (pcmd)) == ELS_CMD_LS_RJT)) {
		/* A LS_RJT associated with Default RPI cleanup has its own
		 * seperate code path.
		 */
		if (!(ndlp->nlp_flag & NLP_RM_DFLT_RPI))
			ls_rjt = 1;
	}

	/* Check to see 