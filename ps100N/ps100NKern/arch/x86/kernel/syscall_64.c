entries\n");
		goto out_free_fcp_eq_hdl;
	}

	return rc;

out_free_fcp_eq_hdl:
	kfree(phba->sli4_hba.fcp_eq_hdl);
out_remove_rpi_hdrs:
	lpfc_sli4_remove_rpi_hdrs(phba);
out_free_active_sgl:
	lpfc_free_active_sgl(phba);
out_free_sgl_list:
	lpfc_free_sgl_list(phba);
out_destroy_cq_event_pool:
	lpfc_sli4_cq_event_pool_destroy(phba);
out_destroy_queue:
	lpfc_sli4_queue_destroy(phba);
out_free_bsmbx:
	lpfc_destroy_bootstrap_mbox(phba);
out_free_mem:
	lpfc_mem_free(phba);
	return rc;
}

/**
 * lpfc_sli4_driver_resource_unset - Unset drvr internal resources for SLI4 dev
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to unset the driver in