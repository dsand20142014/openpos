ons.
 **/
static void
lpfc_cmpl_els_flogi(struct lpfc_hba *phba, struct lpfc_iocbq *cmdiocb,
		    struct lpfc_iocbq *rspiocb)
{
	struct lpfc_vport *vport = cmdiocb->vport;
	struct Scsi_Host  *shost = lpfc_shost_from_vport(vport);
	IOCB_t *irsp = &rspiocb->iocb;
	struct lpfc_nodelist *ndlp = cmdiocb->context1;
	struct lpfc_dmabuf *pcmd = cmdiocb->context2, *prsp;
	struct serv_parm *sp;
	int rc;

	/* Check to see if link went down during discovery */
	if (lpfc_els_chk_latt(vport)) {
		/* One additional decrement on node reference count to
		 * trigger the release of the node
		 */
		lpfc_nlp_put(ndlp);
		goto out;
	}

	lpfc_debugfs_disc_trc(vport, LPFC_DISC_TRC_ELS_CMD,
		"FLOGI cmpl:      status:x%x/x%x state:x%x",
		irsp->ulpStatus, irsp->un.ulpWord[4],
		vport->port_state);

	if (irsp->ulpStatus) {
		/* Check for retry */
		if (lpfc_els_retry(phba, cmdiocb, rspiocb))
			goto out;

		/* FLOGI failed, so there is no fabric */
		spin_lock_irq(shost->host_lock);
		vport->fc_flag &= ~(FC_FABRIC | FC_PUBLIC_LOOP);
		spin_unlock_irq(shost->host_lock);

		/* If private loop, then allow max outstanding els to be
		 * LPFC_MAX_DISC_THREADS (32). Scanning in the case of no
		 * alpa map would take too long otherwise.
		 */
		if (phba->alpa_map[0] == 0) {
			vport->cfg_discovery_threads = LPFC_MAX_DISC_THREADS;
		}

		/* FLOGI failure */
		lpfc_printf_vlog(vport, KERN_INFO, LOG_ELS,
				 "0100 FLOGI failure Data: x%x x%x "
				 "x%x\n",
				 irsp->ulpStatus, 