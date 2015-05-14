iocbq *elsiocb;
	struct lpfc_sli_ring *pring;
	uint8_t *pcmd;
	uint16_t cmdsize;
	uint32_t tmo;
	int rc;

	pring = &phba->sli.ring[LPFC_ELS_RING];

	cmdsize = (sizeof(uint32_t) + sizeof(struct serv_parm));
	elsiocb = lpfc_prep_els_iocb(vport, 1, cmdsize, retry, ndlp,
				     ndlp->nlp_DID, ELS_CMD_FLOGI);

	if (!elsiocb)
		return 1;

	icmd = &elsiocb->iocb;
	pcmd = (uint8_t *) (((struct lpfc_dmabuf *) elsiocb->context2)->virt);

	/* For FLOGI request, remainder of payload is service parameters */
	*((uint32_t *) (pcmd)) = ELS_CMD_FLOGI;
	pcmd += sizeof(uint32_t);
	memcpy(pcmd, &vport->fc_sparam, sizeof(struct serv_parm));
	sp = (struct serv_parm *) pcmd;

	/* Setup CSPs accordingly for Fabric