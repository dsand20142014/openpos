e->isp;

	pci_set_master(ha->pdev);
	pci_try_set_mwi(ha->pdev);

	pci_read_config_word(ha->pdev, PCI_COMMAND, &w);
	w |= (PCI_COMMAND_PARITY | PCI_COMMAND_SERR);
	pci_write_config_word(ha->pdev, PCI_COMMAND, w);

	pci_disable_rom(ha->pdev);

	/* Get PCI bus information. */
	spin_lock_irqsave(&ha->hardware_lock, flags);
	ha->pci_attr = RD_REG_WORD(&reg->ctrl_status);
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_SUCCESS;
}

/**
 * qla2300_pci_config() - Setup ISP23xx PCI configuration registers.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla2300_pci_config(scsi_qla_host_t *vha)
{
	uint16_t	w;
	unsigned long   flags = 0;
	uint32_t	cnt;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_2xxx __iomem *reg = &ha->iobase->isp;

	pci_set_master(ha->pdev);
	pci_try_set_mwi(ha->pdev);

	pci_read_config_word(ha->pdev, PCI_COMMAND, &w);
	w |= (PCI_COMMAND_PARITY | PCI_COMMAND_SERR);

	if (IS_QLA2322(ha) || IS_QLA6322(ha))
		w &= ~PCI_COMMAND_INTX_DISABLE;
	pci_write_config_word(ha->pdev, PCI_COMMAND, w);

	/*
	 * If this is a 2300 card and not 2312, reset the
	 * COMMAND_INVALIDATE due to a bug in the 2300. Unfortunately,
	 * the 2310 also reports itself as a 2300 so we need to get the
	 * fb revision level -- a 6 indicates it really is a 2300 and
	 * not a 2310.
	 */
	if (IS_QLA2300(ha)) {
		spin_lock_irqsave(&ha->hardware_lock, flags);

		/* Pause RISC. */
		WRT_REG_WORD(&reg->hccr, HCCR_PAUSE_RISC);
		for (cnt = 0; cnt < 30000; cnt++) {
			if ((RD_REG_WORD(&reg->hccr) & HCCR_RISC_PAUSE) != 0)
				break;

			udelay(10);
		}

		/* Select FPM registers. */
		WRT_REG_WORD(&reg->ctrl_status, 0x20);
		RD_REG_WORD(&reg->ctrl_status);

		/* Get the fb rev level */
		ha->fb_rev = RD_FB_CMD_REG(ha, reg);

		if (ha->fb_rev == FPM_2300)
			pci_clear_mwi(ha->pdev);

		/* Deselect FPM registers. */
		WRT_REG_WORD(&reg->ctrl_status, 0x0);
		RD_REG_WORD(&reg->ctrl_status);

		/* Release RISC module. */
		WRT_REG_WORD(&reg->hccr, HCCR_RELEASE_RISC);
		for (cnt = 0; cnt < 30000; cnt++) {
			if ((RD_REG_WORD(&reg->hccr) & HCCR_RISC_PAUSE) == 0)
				break;

			udelay(10);
		}

		spin_unlock_irqrestore(&ha->hardware_lock, flags);
	}

	pci_write_config_byte(ha->pdev, PCI_LATENCY_TIMER, 0x80);

	pci_disable_rom(ha->pdev);

	/* Get PCI bus information. */
	spin_lock_irqsave(&ha->hardware_lock, flags);
	ha->pci_attr = RD_REG_WORD(&reg->ctrl_status);
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_SUCCESS;
}

/**
 * qla24xx_pci_config() - Setup ISP24xx PCI configuration registers.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla24xx_pci_config(scsi_qla_host_t *vha)
{
	uint16_t w;
	unsigned long flags = 0;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_24xx __iomem *reg = &ha->iobase->isp24;

	pci_set_master(ha->pdev);
	pci_try_set_mwi(ha->pdev);

	pci_read_config_word(ha->pdev, PCI_COMMAND, &w);
	w |= (PCI_COMMAND_PARITY | PCI_COMMAND_SERR);
	w &= ~PCI_COMMAND_INTX_DISABLE;
	pci_write_config_word(ha->pdev, PCI_COMMAND, w);

	pci_write_config_byte(ha->pdev, PCI_LATENCY_TIMER, 0x80);

	/* PCI-X -- adjust Maximum Memory Read Byte Count (2048). */
	if (pci_find_capability(ha->pdev, PCI_CAP_ID_PCIX))
		pcix_set_mmrbc(ha->pdev, 2048);

	/* PCIe -- adjust Maximum Read Request Size (2048). */
	if (pci_find_capability(ha->pdev, PCI_CAP_ID_EXP))
		pcie_set_readrq(ha->pdev, 2048);

	pci_disable_rom(ha->pdev);

	ha->chip_revision = ha->pdev->revision;

	/* Get PCI bus information. */
	spin_lock_irqsave(&ha->hardware_lock, flags);
	ha->pci_attr = RD_REG_DWORD(&reg->ctrl_status);
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_SUCCESS;
}

/**
 * qla25xx_pci_config() - Setup ISP25xx PCI configuration registers.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla25xx_pci_config(scsi_qla_host_t *vha)
{
	uint16_t w;
	struct qla_hw_data *ha = vha->hw;

	pci_set_master(ha->pdev);
	pci_try_set_mwi(ha->pdev);

	pci_read_config_word(ha->pdev, PCI_COMMAND, &w);
	w |= (PCI_COMMAND_PARITY | PCI_COMMAND_SERR);
	w &= ~PCI_COMMAND_INTX_