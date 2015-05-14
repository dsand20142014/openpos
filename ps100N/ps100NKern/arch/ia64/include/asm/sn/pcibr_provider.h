hba, KERN_INFO, LOG_MBOX | LOG_SLI,
				"(%d):0307 Mailbox cmd x%x (x%x) Cmpl x%p "
				"Data: x%x x%x x%x x%x x%x x%x x%x x%x x%x\n",
				pmb->vport ? pmb->vport->vpi : 0,
				pmbox->mbxCommand,
				lpfc_sli4_mbox_opcode_get(phba, pmb),
				pmb->mbox_cmpl,
				*((uint32_t *) pmbox),
				pmbox->un.varWords[0],
				pmbox->un.varWords[1],
				pmbox->un.varWords[2],
				pmbox->un.varWords[3],
				pmbox->un.varWords[4],
				pmbox->un.varWords[5],
				pmbox->un.varWords[6],
				pmbox->un.varWords[7]);

		if (pmb->mbox_cmpl)
			pmb->mbox_cmpl(phba,pmb);
	} while (1);
	return 0;
}

/**
 * lpfc_sli_get_buff - Get the buffer associated with the buffer tag
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @tag: buffer tag.
 *
 * This function is called with no lock held. When QUE_BUFTAG_BIT bit
 * is set in the tag the buffer is posted for a particular exchange,
 * the function will return the buffer without replacing the buffer.
 * If the buffer is for unsolicited ELS or CT traffic, this function
 * returns the buffer and also posts another buffer to the firmware.
 **/
static struct lpfc_dmabuf *
lpfc_sli_get_buff(struct lpfc_hba *phba,
		  struct lpfc_sli_ring *pring,
		  uint32_t tag)
{
	struct hbq_dmabuf *hbq_entry;

	if (tag & QUE_BUFTAG_BIT)
		return lpfc_sli_ring_taggedbuf_get(phba, pring, tag);
	hbq_entry = lpfc_sli_hbqbuf_find(phba, tag);
	if (!hbq_entry)
		return NULL;
	return &hbq_entry->dbuf;
}

/**
 * lpfc_complete_unsol_iocb - Complete an unsolicited sequence
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @saveq: Pointer to the iocbq struct representing the sequence starting frame.
 * @fch_r_ctl: the r_ctl for the first frame of the sequence.
 * @fch_type: the type for the first frame of the sequence.
 *
 * This function is called with no lock held. This function uses the r_ctl and
 * type of the received sequence to find the correct callback function to call
 * to process the sequence.
 **/
static int
lpfc_complete_unsol_iocb(struct lpfc_hba *phba, struct lpfc_sli_ring *pring,
			 struct lpfc_iocbq *saveq, uint32_t fch_r_ctl,
			 uint32_t fch_type)
{
	int i;

	/* unSolicited Responses */
	if (pring->prt[0].profile) {
		if (pring->prt[0].lpfc_sli_rcv_unsol_event)
			(pring->prt[0].lpfc_sli_rcv_unsol_event) (phba, pring,
									saveq);
		return 1;
	}
	/* We must search, based on rctl / type
	   for the right routine */
	for (i = 0; i < pring->num_mask; i++) {
		if ((pring->prt[i].rctl == fch_r_ctl) &&
		    (pring->prt[i].type == fch_type)) {
			if (pring->prt[i].lpfc_sli_rcv_unsol_event)
				(pring->prt[i].lpfc_sli_rcv_unsol_event)
						(phba, pring, saveq);
			return 1;
		}
	}
	return 0;
}

/**
 * lpfc_sli_process_unsol_iocb - Unsolicited iocb handler
 * @phba: Pointer to HBA context object.
 * @pring: Pointer to driver SLI ring object.
 * @saveq: Pointer to the unsolicited iocb.
 *
 * This function is called with no lock held by the ring event handler
 * when there is an unsolicited iocb posted to the response ring by the
 * firmware. This function gets the buffer associated with the iocbs
 * and calls the event handler for the ring. This function handles both
 * qring buffers and hbq buffers.
 * When the function returns 1 the caller can free the iocb object otherwise
 * upper layer functions will free the iocb objects.
 **/
static int
lpfc_sli_process_unsol_iocb(struct lpfc_hba *phba, struct lpfc_sli_ring *pring,
			    struct lpfc_iocbq *saveq)
{
	IOCB_t           * irsp;
	WORD5            * w5p;
	uint32_t           Rctl, Type;
	uint32_t           match;
	struct lpfc_iocbq *iocbq;
	struct lpfc_dmabuf *dmzbuf;

	match = 0;
	irsp = &(saveq->iocb);

	if (irsp->ulpCommand == CMD_ASYNC_STATUS) {
		if (pring->lpfc_sli_rcv_async_status)
			pring->lpfc_sli_rcv_async_status(phba, pring, saveq);
		else
			lpfc_printf_log(phba,
					KERN_WARNING,
					LOG_SLI,
					"0316 Ring %d handler: unexpected "
					"ASYNC_STATUS iocb received evt_code "
					"0x%x\n",
					pring->ringno,
					irsp->un.asyncstat.evt_code);
		return 1;
	}

	if ((irsp->ulpCommand == CMD_IOCB_RET_XRI64_CX) &&
		(phba->sli3_options & LPFC_SLI3_HBQ_ENABLED)) {
		if (irsp->ulpBdeCount > 0) {
			dmzbuf = lpfc_sli_get_buff(phba, pring,
					irsp->un.ulpWord[3]);
			lpfc_in_buf_free(phba, dmzbuf);
		}

		if (irsp->ulpBdeCount > 1) {
			dmzbuf = lpfc_sli_get_buff(phba, pring,
					irsp->unsli3.sli3Words[3]);
			lpfc_in_buf_free(phba, dmzbuf);
		}

		if (irsp->ulpBdeCount > 2) {
			dmzbuf = lpfc_sli_get_buff(phba, pring,
				irsp->unsli3.sli3Words[7]);
			lpfc_in_buf_free(phba, dmzbuf);
		}

		return 1;
	}

	if (phba->sli3_options & LPFC_SLI3_HBQ_ENABLED) {
		if (irsp->ulpBdeCount != 0) {
			saveq->context2 = lpfc_sli_get_buff(phba, pring,
						irsp->un.ulpWord[3]);
			if (!saveq->context2)
				lpfc_printf_log(phba,
					KERN_ERR,
					LOG_SLI,
					"0341 Ring %d Cannot find buffer for "
					"an unsolicited iocb. tag 0x%x\n",
					pring->ringno,
					irsp->un.ulpWord[3]);
		}
		if (irsp->ulpBdeCount == 2) {
			saveq->context3 = lpfc_sli_get_buff(phba, pring,
						irsp->unsli3.sli3Words[7]);
			if (!saveq->context3)
				lpfc_printf_log(phba,
					KERN_ERR,
					LOG_SLI,
					"0342 Ring %d Cannot find buffer for an"
					" unsolicited iocb. tag 0x%x\n",
					pring->ringno,
					irsp->unsli3.sli3Words[7]);
		}
		list_for_each_entry(iocbq, &saveq->list, list) {
			irsp = &(iocbq->iocb);
			if (irsp->ulpBdeCount != 0) {
				iocbq->context2 = lpfc_sli_get_buff(phba, pring,
							irsp->un.ulpWord[3]);
				if (!iocbq->context2)
					lpfc_printf_log(phba,
						KERN_ERR,
						LOG_SLI,
						"0343 Ring %d