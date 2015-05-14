c_iocb_list);

	/* Initialize list to save ELS buffers */
	INIT_LIST_HEAD(&phba->elsbuf);

	/* Initialize FCF connection rec list */
	INIT_LIST_HEAD(&phba->fcf_conn_rec_list);

	return 0;
}

/**
 * lpfc_setup_driver_resource_phase2 - Phase2 setup driver internal resources.
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to set up the driver internal resources after the
 * device specific resource setup to support the HBA device it attached to.
 *
 * Return codes
 * 	0 - sucessful
 * 	other values - error
 **/
static int
lpfc_setup_driver_resource_phase2(struct lpfc_hba *phba)
{
	int error;

	/* Startup the kernel thread for this host adapter. */
	phba->worker_thread = kthread_run(lpfc_do_work, phba,
					  "lpfc_worker_%d", phba->brd_no);
	if (IS_ERR(phba->worker_thread)) {
		error = PTR_ERR(phba->worker_thread);
		return error;
	}

	return 0;
}

/**
 * lpfc_unset_driver_resource_phase2 - Phase2 unset driver internal resources.
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to unset the driver internal resources set up after
 * the device specific resource setup for supporting the HBA device it
 * attached to.
 **/
static void
lpfc_unset_driver_resource_phase2(struct lpfc_hba *phba)
{
	/* Stop kernel worker thread */
	kthread_stop(phba->worker_thread);
}

/**
 * lpfc_free_iocb_list - Free iocb list.
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to free the driver's IOCB list and memory.
 **/
static void
lpfc_free_iocb_list(struct lpfc_hba *phba)
{
	struct lpfc_iocbq *iocbq_entry = NULL, *iocbq_next = NULL;

	spin_lock_irq(&phba->hbalock);
	list_for_each_entry_safe(iocbq_entry, iocbq_next,
				 &phba->lpfc_iocb_list, list) {
		list_del(&iocbq_entry->list);
		kfree(iocbq_entry);
		phba->total_iocbq_bufs--;
	}
	spin_unlock_irq(&phba->hbalock);

	return;
}

/**
 * lpfc_init_iocb_list - Allocate and initialize iocb list.
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to allocate and initizlize the driver's IOCB
 