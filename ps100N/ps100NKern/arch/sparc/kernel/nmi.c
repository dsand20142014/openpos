eturn;
			}

			lpfc_heart_beat(phba, pmboxq);
			pmboxq->mbox_cmpl = lpfc_hb_mbox_cmpl;
			pmboxq->vport = phba->pport;
			retval = lpfc_sli_issue_mbox(phba, pmboxq, MBX_NOWAIT);

			if (retval != MBX_BUSY && retval != MBX_SUCCESS) {
				mempool_free(pmboxq, phba->mbox_mem_pool);
				mod_timer(&phba->hb_tmofunc,
					  jiffies + HZ * LPFC_HB_MBOX_INTERVAL);
				return;
			}
			mod_timer(&phba->hb_tmofunc,
				  jiffies + HZ * LPFC_HB_MBOX_TIMEOUT);
			phba->hb_outstanding = 1;
			return;
		} else {
			/*
			* If heart beat timeout called with hb_outstanding set
			* we need to take the HBA offline.
			*/
			lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
					"0459 Adapter heartbeat failure, "
					"taking this port offline.\n");

			spin_lock_irq(&phba->hbalock);
			psli->sli_flag &= ~LPFC_SLI_ACTIVE;
			spin_unlock_irq(&phba->hbalock);

			lpfc_offline_prep(phba);
			lpfc_offline(phba);
			lpfc_unblock_mgmt_io(phba);
			phba->link_state = LPFC_HBA_ERROR;
			lpfc_hba_down_post(phba);
		}
	}
}

/**
 * lpfc_offline_eratt - Bring lpfc offline on hardware error attention
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is called to bring the HBA offline when HBA hardware error
 * other than Port Error 6 has been detected.
 **/
static void
lpfc_offline_eratt(struct lpfc_hba *phba)
{
	struct lpfc_sli   *psli = &phba->sli;

	spin_lock_irq(&phba->hbalock);
	psli->sli_flag &= ~LPFC_SLI_ACTIVE;
	spin_unlock_irq(&phba->hbalock);
	lpfc_offline_prep(phba);

	lpfc_offline(phba);
	lpfc_reset_barrier(phba);
	spin_lock_irq(&phba->hbalock);
	lpfc_sli_brdreset(phba);
	spin_unlock_irq(&phba->hbalock);
	lpfc_hba_down_post(phba);
	lpfc_sli_brdready(phba, HS_MBRDY);
	lpfc_unblock_mgmt_io(phba);
	phba->link_state = LPFC_HBA_ERROR;
	return;
}

/**
 * lpfc_sli4_offline_eratt - Bring lpfc offline on SLI4 hardware error attention
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is called to bring a SLI4 HBA offline when HBA hardware error
 * other than Port Error 6 has been detected.
 **/
static void
lpfc_sli4_offline_eratt(struct lpfc_hba *phba)
{
	lpfc_offline_prep(phba);
	lpfc_offline(phba);
	lpfc_sli4_brdreset(phba);
	lpfc_hba_down_post(phba);
	lpfc_sli4_post_status_check(phba);
	lpfc_unblock_mgmt_io(phba);
	phba->link_state = LPFC_HBA_ERROR;
}

/**
 * lpfc_handle_deferred_eratt - The HBA hardware deferred error handler
 * @phba: pointer to lpfc hba data structure.
 *
 * This routine is invoked to handle the deferred HBA hardware error
 * conditions. This type of error is indicated by HBA by setting ER1
 * and another ER bit in the host status register. The driver will
 * wait until the ER1 bit clears before handling the error condition.
 **/
static void
lpfc_handle_deferred_eratt(struct lpfc_hba *phba)
{
	uint32_t old_host_status = phba->work_hs;
	struct lpfc_sli_ring  *pring;
	struct lpfc_sli *psli = &phba->sli;

	/* If the pci channel is offline, ignore possible errors,
	 * since we cannot communicate with the pci card anyway.
	 */
	if (pci_channel_offline(phba->pcidev)) {
		spin_lock_irq(&phba->hbalock);
		phba->hba_flag &= ~DEFER_ERATT;
		spin_unlock_irq(&phba->hbalock);
		return;
	}

	lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
		"0479 Deferred Adapter Hardware Error "
		"Data: x%x x%x x%x\n",
		phba->work_hs,
		phba->work_status[0], phba->work_status[1]);

	spin_lock_irq(&phba->hbalock);
	psli->sli_flag &= ~LPFC_SLI_ACTIVE;
	spin_unlock_irq(&phba->hbalock);


	/*
	 * Firmware stops when it triggred erratt. That could cause the I/Os
	 * dropped by the firmware. Error iocb (I/O) on txcmplq and let the
	 * SCSI layer retry it after re-establishing link.
	 */
	pring = &psli->ring[psli->fcp_ring];
	lpfc_sli_abort_iocb_ring(phba, pring);

	/*
	 * There was a firmware error. Take the hba offline and then
	 * attempt to restart it.
	 */
	lpfc_offline_prep(phba);
	lpfc_offline(phba);

	/* Wait for the ER1 bit to clear.*/
	while (phba->work_hs & HS_FFER1) {
		msleep(100);
		phba->work_hs = readl(phba->HSregaddr);
		/* If driver is unloading let the worker thread continue */
		if (phba->pport->load_flag & FC_UNLOADING) {
	_usable) {
		kfree(prev_nmi_count);
		err = -ENODEV;
		goto error;
	}
	printk("OK.\n");

	nmi_hz = 1;

	kfree(prev_nmi_count);
	return 0;
error:
	on_each_cpu(stop_watchdog, NULL, 1);
	return err;
}

static void start_watchdog(void *unused)
{
	pcr_ops->write(PCR_PIC_PRIV);
	write_pic(picl_value(nmi_hz));

	pcr_ops->write(pcr_enable);
}

void nmi_adjust_hz(unsigned int new_hz)
{
	nmi_hz = new_hz;
	on_each_cpu(start_watchdog, NULL, 1);
}
EXPORT_SYMBOL_GPL(nmi_adjust_hz);

static int nmi_shutdown(struct notifier_block *nb, unsigned long cmd, void *p)
{
	on_each_cpu(stop_watchdog, NULL, 1);
	return 0;
}

static struct notifier_block nmi_reboot_notifier = {
	.notifier_call = nmi_shutdown,
};

int __init nmi_init(void)
{
	int err;

	nmi_usable = 1;

	on_each_cpu(start_watchdog, NULL, 1);

	err = check_nmi_watchdog();
	if (!err) {
		err = register_reboot_notifier(&nmi_reboot_notifier);
		if (err) {
			nmi_usable = 0;
			on_each_cpu(stop_watchdog, NULL, 1);
		}
	}
	return err;
}

static int __init setup_nmi_watchdog(char *str)
{
	if (!strncmp(str, "panic", 5))
		panic_on_timeout = 1;

	return 0;
}
__setup("nmi_watchdog=", setup_nmi_watchdog);
