RspPut = le32_to_cpu(pgp->rspPutInx);
	}

	if ((rsp_cmpl > 0) && (mask & HA_R0RE_REQ)) {
		pring->stats.iocb_rsp_full++;
		status = ((CA_R0ATT | CA_R0RE_RSP) << (pring->ringno * 4));
		writel(status, phba->CAregaddr);
		readl(phba->CAregaddr);
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
	return rc;
}

/**
 * lpfc_sli_sp_handle_rspiocb - Handle slow-path response iocb
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @rspiocbp: Pointer to driver response IOCB object.
 *
 * This function is called from the worker thread when there is a slow-path
 * response IOCB to process. This function chains all the response iocbs until
 * seeing the iocb with the LE bit set. The function will call
 * lpfc_sli_process_sol_iocb function if the response iocb indicates a
 * completion of a command iocb. The function will call the
 * lpfc_sli_process_unsol_iocb function if this is an unsolicited iocb.
 * The function frees the resources or calls the completion handler if this
 * iocb is an abort completion. The function returns NULL when the response
 * iocb has the LE bit set and all the chained iocbs are processed, otherwise
 * this function shall chain the iocb on to the iocb_continueq and return the
 * response iocb passed in.
 **/
static struct lpfc_iocbq *
lpfc_sli_sp_handle_rspiocb(struct lpfc_hba *phba, struct lpfc_sli_ring *pring,
			struct lpfc_iocbq *rspiocbp)
{
	struct lpfc_iocbq *saveq;
	struct lpfc_iocbq *cmdiocbp;
	struct lpfc_iocbq *next_iocb;
	IOCB_t *irsp = NULL;
	uint32_t free_saveq;
	uint8_t iocb_cmd_type;
	lpfc_iocb_type type;
	unsigned long iflag;
	int rc;

	spin_lock_irqsave(&phba->hbalock, iflag);
	/* First add the response iocb to the countinueq list */
	list_add_tail(&rspiocbp->list, &(pring->iocb_continueq));
	pring->iocb_continueq_cnt++;

	/* Now, determine whetehr the list is completed for processing */
	irsp = &rspiocbp->iocb;
	if (irsp->ulpLe) {
		/*
		 * By default, the driver expects to free all resources
		 * associated with this iocb completion.
		 */
		free_saveq = 1;
		saveq = list_get_first(&pring->iocb_continueq,
				       struct lpfc_iocbq, list);
		irsp = &(saveq->iocb);
		list_del_init(&pring->iocb_continueq);
		pring->iocb_continueq_cnt = 0;

		pring->stats.iocb_rsp++;

		/*
		 * If resource errors reported from HBA, reduce
		 * queuedepths of the SCSI device.
		 */
		if ((irsp->ulpStatus == IOSTAT_LOCAL_REJECT) &&
		    (irsp->un.ulpWord[4] == IOERR_NO_RESOURCES)) {
			spin_unlock_irqrestore(&phba->hbalock, iflag);
			ph