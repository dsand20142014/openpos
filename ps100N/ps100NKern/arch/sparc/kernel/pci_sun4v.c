;

	/* PCIe -- adjust Maximum Read Request Size (2048). */
	if (pci_find_capability(ha->pdev, PCI_CAP_ID_EXP))
		pcie_set_readrq(ha->pdev, 2048);

	pci_disable_rom(ha->pdev);

	ha->chip_revision = ha->pdev->revision;

	return QLA_SUCCESS;
}

/**
 * qla2x00_isp_firmware() - Choose firmware image.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
static int
qla2x00_isp_firmware(scsi_qla_host_t *vha)
{
	int  rval;
	uint16_t loop_id, topo, sw_cap;
	uint8_t domain, area, al_pa;
	struct qla_hw_data *ha = vha->hw;

	/* Assume loading risc code */
	rval = QLA_FUNCTION_FAILED;

	if (ha->flags.disable_risc_code_load) {
		DEBUG2(printk("scsi(%ld): RISC CODE NOT loaded\n",
		    vha->host_no));
		qla_printk(KERN_INFO, ha, "RISC CODE NOT loaded\n");

		/* Verify checksum of loaded RISC code. */
		rval = qla2x00_verify_checksum(vha, ha->fw_srisc_address);
		if (rval == QLA_SUCCESS) {
			/* And, verify we are not in ROM code. */
			rval = qla2x00_get_adapter_id(vha, &loop_id, &al_pa,
			    &area, &domain, &topo, &sw_cap);
		}
	}

	if (rval) {
		DEBUG2_3(printk("scsi(%ld): **** Load RISC code ****\n",
		    vha->host_no));
	}

	return (rval);
}

/**
 * qla2x00_reset_chip() - Reset ISP chip.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
void
qla2x00_reset_chip(scsi_qla_host_t *vha)
{
	unsigned long   flags = 0;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_2xxx __iomem *reg = &ha->iobase->isp;
	uint32_t	cnt;
	uint16_t	cmd;

	ha->isp_ops->disable_intrs(ha);

	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Turn off master enable */
	cmd = 0;
	pci_read_config_word(ha->pdev, PCI_COMMAND, &cmd);
	cmd &= ~PCI_COMMAND_MASTER;
	pci_write_config_word(ha->pdev, PCI_COMMAND, cmd);

	if (!IS_QLA2100(ha)) {
		/* Pause RISC. */
		WRT_REG_WORD(&reg->hccr, HCCR_PAUSE_RISC);
		if (IS_QLA2200(ha) || IS_QLA2300(ha)) {
			for (cnt = 0; cnt < 30000; cnt++) {
				if ((RD_REG_WORD(&reg->hccr) &
				    HCCR_RISC_PAUSE) != 0)
					break;
				udelay(100);
			}
		} else {
			RD_REG_WORD(&reg->hccr);	/* PCI Posting. */
			udelay(10);
		}

		/* Select FPM registers. */
		WRT_REG_WORD(&reg->ctrl_status, 0x20);
		RD_REG_WORD(&reg->ctrl_status);		/* PCI Posting. */

		/* FPM Soft Reset. */
		WRT_REG_WORD(&reg->fpm_diag_config, 0x100);
		RD_REG_WORD(&reg->fpm_diag_config);	/* PCI Posting. */

		/* Toggle Fpm Reset. */
		if (!IS_QLA2200(ha)) {
			WRT_REG_WORD(&reg->fpm_diag_config, 0x0);
			RD_REG_WORD(&reg->fpm_diag_config); /* PCI Posting. */
		}

		/* Select frame buffer registers. */
		WRT_REG_WORD(&reg->ctrl_status, 0x10);
		RD_REG_WORD(&reg->ctrl_status);		/* PCI Posting. */

		/* Reset frame buffer FIFOs. */
		if (IS_QLA2200(ha)) {
			WRT_FB_CMD_REG(ha, reg, 0xa000);
			RD_FB_CMD_REG(ha, reg);		/* PCI Posting. */
		} else {
			WRT_FB_CMD_REG(ha, reg, 0x00fc);

			/* Read back fb_cmd until zero or 3 seconds max */
			for (cnt = 0; cnt < 3000; cnt++) {
				if ((RD_FB_CMD_REG(ha, reg) & 0xff) == 0)
					break;
				udelay(100);
			}
		}

		/* Select RISC module registers. */
		WRT_REG_WORD(&reg->ctrl_status, 0);
		RD_REG_WORD(&reg->ctrl_status);		/* PCI Posting. */

		/* Reset RISC processor. */
		WRT_REG_WORD(&reg->hccr, HCCR_RESET_RISC);
		RD_REG_WORD(&reg->hccr);		/* PCI Posting. */

		/* Release RISC processor. */
		WRT_REG_WORD(&reg->hccr, HCCR_RELEASE_RISC);
		RD_REG_WORD(&reg->hccr);		/* PCI Posting. */
	}

	WRT_REG_WORD(&reg->hccr, HCCR_CLR_RISC_INT);
	WRT_REG_WORD(&reg->hccr, HCCR_CLR_HOST_INT);

	/* Reset ISP chip. */
	WRT_REG_WORD(&reg->ctrl_status, CSR_ISP_SOFT_RESET);

	/* Wait for RISC to recover from reset. */
	if (IS_QLA2100(ha) || IS_QLA2200(ha) || IS_QLA2300(ha)) {
		/*
		 * It is necessary to for a delay here since the card doesn't
		 * respond to PCI reads during a reset. On some architectures
		 * this will result in an MCA.
		 */
		udelay(20);
		for (cnt = 30000; cnt; cnt--) {
			if ((RD_REG_WORD(&reg->ctrl_status) &
			    CSR_ISP_SOFT_RESET) == 0)
				break;
			udelay(100);
		}
	} else
		udelay(10);

	/* Reset RISC processor. */
	WRT_REG_WORD(&reg->hccr, HCCR_RESET_RISC);

	WRT_REG_WORD(&reg->semaphore, 0);

	/* Release RISC processor. */
	WRT_REG_WORD(&reg->hccr, HCCR_RELEASE_RISC);
	RD_REG_WORD(&reg->hccr);			/* PCI Posting. */

	if (IS_QLA2100(ha) || IS_QLA2200(ha) || IS_QLA2300(ha)) {
		for (cnt = 0; cnt < 30000; cnt++) {
			if (RD_MAILBOX_REG(ha, reg, 0) != MBS_BUSY)
				break;

			udelay(100);
		}
	} else
		udelay(100);

	/* Turn on master enable */
	cmd |= PCI_COMMAND_MASTER;
	pci_write_config_word(ha->pdev, PCI_COMMAND, cmd);

	/* Disable RISC pause on FPM parity error. */
	if (!IS_QLA2100(ha)) {
		WRT_REG_WORD(&reg->hccr, HCCR_DISABLE_PARITY_PAUSE);
		RD_REG_WORD(&reg->hccr);		/* PCI Posting. */
	}

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
}

/**
 * qla24xx_reset_risc() - Perform full reset of ISP24xx RISC.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
static inline void
qla24xx_reset_risc(scsi_qla_host_t *vha)
{
	unsigned long flags = 0;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_24xx __iomem *reg = &ha->iobase->isp24;
	uint32_t cnt, d2;
	uint16_t wd;

	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Reset RISC. */
	WRT_REG_DWORD(&reg->ctrl_status, CSRX_DMA_SHUTDOWN|MWB_4096_BYTES);
	for (cnt = 0; cnt < 30000; cnt++) {
		if ((RD_REG_DWORD(&reg->ctrl_status) & CSRX_DMA_ACTIVE) == 0)
			break;

		udelay(10);
	}

	WRT_REG_DWORD(&reg->ctrl_status,
	    CSRX_ISP_SOFT_RESET|CSRX_DMA_SHUTDOWN|MWB_4096_BYTES);
	pci_read_config_word(ha->pdev, PCI_COMMAND, &wd);

	udelay(100);
	/* Wait for firmware to complete NVRAM accesses. */
	d2 = (uint32_t) RD_REG_WORD(&reg->mailbox0);
	for (cnt = 10000 ; cnt && d2; cnt--) {
		udelay(5);
		d2 = (uint32_t) RD_REG_WORD(&reg->mailbox0);
		barrier();
	}

	/* Wait for soft-reset to complete. */
	d2 = RD_REG_DWORD(&reg->ctrl_status);
	for (cnt = 6000000 ; cnt && (d2 & CSRX_ISP_SOFT_RESET); cnt--) {
		udelay(5);
		d2 = RD_REG_DWORD(&reg->ctrl_status);
		barrier();
	}

	WRT_REG_DWORD(&reg->hccr, HCCRX_SET_RISC_RESET);
	RD_REG_DWORD(&reg->hccr);

	WRT_REG_DWORD(&reg->hccr, HCCRX_REL_RISC_PAUSE);
	RD_REG_DWORD(&reg->hccr);

	WRT_REG_DWORD(&reg->hccr, HCCRX_CLR_RISC_RESET);
	RD_REG_DWORD(&reg->hccr);

	d2 = (uint32_t) RD_REG_WORD(&reg->mailbox0);
	for (cnt = 6000000 ; cnt && d2; cnt--) {
		udelay(5);
		d2 = (uint32_t) RD_REG_WORD(&reg->mailbox0);
		barrier();
	}

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	if (IS_NOPOLLING_TYPE(ha))
		ha->isp_ops->enable_intrs(ha);
}

/**
 * qla24xx_reset_chip() - Reset ISP24xx chip.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
void
qla24xx_reset_chip(scsi_qla_host_t *vha)
{
	struct qla_hw_data *ha = vha->hw;
	ha->isp_ops->disable_intrs(ha);

	/* Perform RISC reset. */
	qla24xx_reset_risc(vha);
}

/**
 * qla2x00_chip_diag() - Test chip for proper operation.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla2x00_chip_diag(scsi_qla_host_t *vha)
{
	int		rval;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_2xxx __iomem *reg = &ha->iobase->isp;
	unsigned long	flags = 0;
	uint16_t	data;
	uint32_t	cnt;
	uint16_t	mb[5];
	struct req_que *req = ha->req_q_map[0];

	/* Assume a failed state */
	rval = QLA_FUNCTION_FAILED;

	DEBUG3(printk("scsi(%ld): Testing device at %lx.\n",
	    vha->host_no, (u_long)&reg->flash_address));

	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Reset ISP chip. */
	WRT_REG_WORD(&reg->ctrl_status, CSR_ISP_SOFT_RESET);

	/*
	 * We need to have a delay here since the card will not respond while
	 * in reset causing an MCA on some architectures.
	 */
	udelay(20);
	data = qla2x00_debounce_register(&reg->ctrl_status);
	for (cnt = 6000000 ; cnt && (data & CSR_ISP_SOFT_RESET); cnt--) {
		udelay(5);
		data = RD_REG_WORD(&reg->ctrl_status);
		barrier();
	}

	if (!cnt)
		goto chip_diag_failed;

	DEBUG3(printk("scsi(%ld): Reset register cleared by chip reset\n",
	    vha->host_no));

	/* Reset RISC processor. */
	WRT_REG_WORD(&reg->hccr, HCCR_RESET_RISC);
	WRT_REG_WORD(&reg->hccr, HCCR_RELEASE_RISC);

	/* Workaround for QLA2312 PCI parity error */
	if (IS_QLA2100(ha) || IS_QLA2200(ha) || IS_QLA2300(ha)) {
		data = qla2x00_debounce_register(MAILBOX_REG(ha, reg, 0));
		for (cnt = 6000000; cnt && (data == MBS_BUSY); cnt--) {
			udelay(5);
			data = RD_MAILBOX_REG(ha, reg, 0);
			barrier();
		}
	} else
		udelay(10);

	if (!cnt)
		goto chip_diag_failed;

	/* Check product ID of chip */
	DEBUG3(printk("scsi(%ld): Checking product ID of chip\n", vha->host_no));

	mb[1] = RD_MAILBOX_REG(ha, reg, 1);
	mb[2] = RD_MAILBOX_REG(ha, reg, 2);
	mb[3] = RD_MAILBOX_REG(ha, reg, 3);
	mb[4] = qla2x00_debounce_register(MAILBOX_REG(ha, reg, 4));
	if (mb[1] != PROD_ID_1 || (mb[2] != PROD_ID_2 && mb[2] != PROD_ID_2a) ||
	    mb[3] != PROD_ID_3) {
		qla_printk(KERN_WARNING, ha,
		    "Wrong product ID = 0x%x,0x%x,0x%x\n", mb[1], mb[2], mb[3]);

		goto chip_diag_failed;
	}
	ha->product_id[0] = mb[1];
	ha->product_id[1] = mb[2];
	ha->product_id[2] = mb[3];
	ha->product_id[3] = mb[4];

	/* Adjust fw RISC transfer size */
	if (req->length > 1024)
		ha->fw_transfer_size = REQUEST_ENTRY_SIZE * 1024;
	else
		ha->fw_transfer_size = REQUEST_ENTRY_SIZE *
		    req->length;

	if (IS_QLA2200(ha) &&
	    RD_MAILBOX_REG(ha, reg, 7) == QLA2200A_RISC_ROM_VER) {
		/* Limit firmware transfer size with a 2200A */
		DEBUG3(printk("scsi(%ld): Found QLA2200A chip.\n",
		    vha->host_no));

		ha->device_type |= DT_ISP2200A;
		ha->fw_transfer_size = 128;
	}

	/* Wrap Incoming Mailboxes Test. */
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	DEBUG3(printk("scsi(%ld): Checking mailboxes.\n", vha->host_no));
	rval = qla2x00_mbx_reg_test(vha);
	if (rval) {
		DEBUG(printk("scsi(%ld): Failed mailbox send register test\n",
		    vha->host_no));
		qla_printk(KERN_WARNING, ha,
		    "Failed mailbox send register test\n");
	}
	else {
		/* Flag a successful rval */
		rval = QLA_SUCCESS;
	}
	spin_lock_irqsave(&ha->hardware_lock, flags);

chip_diag_failed:
	if (rval)
		DEBUG2_3(printk("scsi(%ld): Chip diagnostics **** FAILED "
		    "****\n", vha->host_no));

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return (rval);
}

/**
 * qla24xx_chip_diag() - Test ISP24xx for proper operation.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla24xx_chip_diag(scsi_qla_host_t *vha)
{
	int rval;
	struct qla_hw_data *ha = vha->hw;
	struct req_que *req = ha->req_q_map[0];

	ha->fw_transfer_size = REQUEST_ENTRY_SIZE * req->length;

	rval = qla2x00_mbx_reg_test(vha);
	if (rval) {
		DEBUG(printk("scsi(%ld): Failed mailbox send register test\n",
		    vha->host_no));
		qla_printk(KERN_WARNING, ha,
		    "Failed mailbox send register test\n");
	} else {
		/* Flag a successful rval */
		rval = QLA_SUCCESS;
	}

	return rval;
}

void
qla2x00_alloc_fw_dump(scsi_qla_host_t *vha)
{
	int rval;
	uint32_t dump_size, fixed_size, mem_size, req_q_size, rsp_q_size,
	    eft_size, fce_size, mq_size;
	dma_addr_t tc_dma;
	void *tc;
	struct qla_hw_data *ha = vha->hw;
	struct req_que *req = ha->req_q_map[0];
	struct rsp_que *rsp = ha->rsp_q_map[0];

	if (ha->fw_dump) {
		qla_printk(KERN_WARNING, ha,
		    "Firmware dump previously allocated.\n");
		return;
	}

	ha->fw_dumped = 0;
	fixed_size = mem_size = eft_size = fce_size = mq_size = 0;
	if (IS_QLA2100(ha) || IS_QLA2200(ha)) {
		fixed_size = sizeof(struct qla2100_fw_dump);
	} else if (IS_QLA23XX(ha)) {
		fixed_size = offsetof(struct qla2300_fw_dump, data_ram);
		mem_size = (ha->fw_memory_size - 0x11000 + 1) *
		    sizeof(uint16_t);
	} else if (IS_FWI2_CAPABLE(ha)) {
		if (IS_QLA81XX(ha))
			fixed_size = offsetof(struct qla81xx_fw_dump, ext_mem);
		else if (IS_QLA25XX(ha))
			fixed_size = offsetof(struct qla25xx_fw_dump, ext_mem);
		else
			fixed_size = offsetof(struct qla24xx_fw_dump, ext_mem);
		mem_size = (ha->fw_memory_size - 0x100000 + 1) *
		    sizeof(uint32_t);
		if (ha->mqenable)
			mq_size = sizeof(struct qla2xxx_mq_chain);
		/* Allocate memory for Fibre Channel Event Buffer. */
		if (!IS_QLA25XX(ha) && !IS_QLA81XX(ha))
			goto try_eft;

		tc = dma_alloc_coherent(&ha->pdev->dev, FCE_SIZE, &tc_dma,
		    GFP_KERNEL);
		if (!tc) {
			qla_printk(KERN_WARNING, ha, "Unable to allocate "
			    "(%d KB) for FCE.\n", FCE_SIZE / 1024);
			goto try_eft;
		}

		memset(tc, 0, FCE_SIZE);
		rval = qla2x00_enable_fce_trace(vha, tc_dma, FCE_NUM_BUFFERS,
		    ha->fce_mb, &ha->fce_bufs);
		if (rval) {
			qla_printk(KERN_WARNING, ha, "Unable to initialize "
			    "FCE (%d).\n", rval);
			dma_free_coherent(&ha->pdev->dev, FCE_SIZE, tc,
			    tc_dma);
			ha->flags.fce_enabled = 0;
			goto try_eft;
		}

		qla_printk(KERN_INFO, ha, "Allocated (%d KB) for FCE...\n",
		    FCE_SIZE / 1024);

		fce_size = sizeof(struct qla2xxx_fce_chain) + FCE_SIZE;
		ha->flags.fce_enabled = 1;
		ha->fce_dma = tc_dma;
		ha->fce = tc;
try_eft:
		/* Allocate memory for Extended Trace Buffer. */
		tc = dma_alloc_coherent(&ha->pdev->dev, EFT_SIZE, &tc_dma,
		    GFP_KERNEL);
		if (!tc) {
			qla_printk(KERN_WARNING, ha, "Unable to allocate "
			    "(%d KB) for EFT.\n", EFT_SIZE / 1024);
			goto cont_alloc;
		}

		memset(tc, 0, EFT_SIZE);
		rval = qla2x00_enable_eft_trace(vha, tc_dma, EFT_NUM_BUFFERS);
		if (rval) {
			qla_printk(KERN_WARNING, ha, "Unable to initialize "
			    "EFT (%d).\n", rval);
			dma_free_coherent(&ha->pdev->dev, EFT_SIZE, tc,
			    tc_dma);
			goto cont_alloc;
		}

		qla_printk(KERN_INFO, ha, "Allocated (%d KB) for EFT...\n",
		    EFT_SIZE / 1024);

		eft_size = EFT_SIZE;
		ha->eft_dma = tc_dma;
		ha->eft = tc;
	}
cont_alloc:
	req_q_size = req->length * sizeof(request_t);
	rsp_q_size = rsp->length * sizeof(response_t);

	dump_size = offsetof(struct qla2xxx_fw_dump, isp);
	dump_size += fixed_size + mem_size + req_q_size + rsp_q_size + eft_size;
	ha->chain_offset = dump_size;
	dump_size += mq_size + fce_size;

	ha->fw_dump = vmalloc(dump_size);
	if (!ha->fw_dump) {
		qla_printk(KERN_WARNING, ha, "Unable to allocate (%d KB) for "
		    "firmware dump!!!\n", dump_size / 1024);

		if (ha->eft) {
			dma_free_coherent(&ha->pdev->dev, eft_size, ha->eft,
			    ha->eft_dma);
			ha->eft = NULL;
			ha->eft_dma = 0;
		}
		return;
	}
	qla_printk(KERN_INFO, ha, "Allocated (%d KB) for firmware dump...\n",
	    dump_size / 1024);

	ha->fw_dump_len = dump_size;
	ha->fw_dump->signature[0] = 'Q';
	ha->fw_dump->signature[1] = 'L';
	ha->fw_dump->signature[2] = 'G';
	ha->fw_dump->signature[3] = 'C';
	ha->fw_dump->version = __constant_htonl(1);

	ha->fw_dump->fixed_size = htonl(fixed_size);
	ha->fw_dump->mem_size = htonl(mem_size);
	ha->fw_dump->req_q_size = htonl(req_q_size);
	ha->fw_dump->rsp_q_size = htonl(rsp_q_size);

	ha->fw_dump->eft_size = htonl(eft_size);
	ha->fw_dump->eft_addr_l = htonl(LSD(ha->eft_dma));
	ha->fw_dump->eft_addr_h = htonl(MSD(ha->eft_dma));

	ha->fw_dump->header_size =
	    htonl(offsetof(struct qla2xxx_fw_dump, isp));
}

static int
qla81xx_mpi_sync(scsi_qla_host_t *vha)
{
#define MPS_MASK	0xe0
	int rval;
	uint16_t dc;
	uint32_t dw;
	struct qla_hw_data *ha = vha->hw;

	if (!IS_QLA81XX(vha->hw))
		return QLA_SUCCESS;

	rval = qla2x00_write_ram_word(vha, 0x7c00, 1);
	if (rval != QLA_SUCCESS) {
		DEBUG2(qla_printk(KERN_WARNING, ha,
		    "Sync-MPI: Unable to acquire semaphore.\n"));
		goto done;
	}

	pci_read_config_word(vha->hw->pdev, 0x54, &dc);
	rval = qla2x00_read_ram_word(vha, 0x7a15, &dw);
	if (rval != QLA_SUCCESS) {
		DEBUG2(qla_printk(KERN_WARNING, ha,
		    "Sync-MPI: Unable to read sync.\n"));
		goto done_release;
	}

	dc &= MPS_MASK;
	if (dc == (dw & MPS_MASK))
		goto done_release;

	dw &= ~MPS_MASK;
	dw |= dc;
	rval = qla2x00_write_ram_word(vha, 0x7a15, dw);
	if (rval != QLA_SUCCESS) {
		DEBUG2(qla_printk(KERN_WARNING, ha,
		    "Sync-MPI: Unable to gain sync.\n"));
	}

done_release:
	rval = qla2x00_write_ram_word(vha, 0x7c00, 0);
	if (rval != QLA_SUCCESS) {
		DEBUG2(qla_printk(KERN_WARNING, ha,
		    "Sync-MPI: Unable to release semaphore.\n"));
	}

done:
	return rval;
}

/**
 * qla2x00_setup_chip() - Load and start RISC firmware.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
static int
qla2x00_setup_chip(scsi_qla_host_t *vha)
{
	int rval;
	uint32_t srisc_address = 0;
	struct qla_hw_data *ha = vha->hw;
	struct device_reg_2xxx __iomem *reg = &ha->iobase->isp;
	unsigned long flags;
	uint16_t fw_major_version;

	if (!IS_FWI2_CAPABLE(ha) && !IS_QLA2100(ha) && !IS_QLA2200(ha)) {
		/* Disable SRAM, Instruction RAM and GP RAM parity.  */
		spin_lock_irqsave(&ha->hardware_lock, flags);
		WRT_REG_WORD(&reg->hccr, (HCCR_ENABLE_PARITY + 0x0));
		RD_REG_WORD(&reg->hccr);
		spin_unlock_irqrestore(&ha->hardware_lock, flags);
	}

	qla81xx_mpi_sync(vha);

	/* Load firmware sequences */
	rval = ha->isp_ops->load_risc(vha, &srisc_address);
	if (rval == QLA_SUCCESS) {
		DEBUG(printk("scsi(%ld): Verifying Checksum of loaded RISC "
		    "code.\n", vha->host_no));

		rval = qla2x00_verify_checksum(vha, srisc_address);
		if (rval == QLA_SUCCESS) {
			/* Start firmware execution. */
			DEBUG(printk("scsi(%ld): Checksum OK, start "
			    "firmware.\n", vha->host_no));

			rval = qla2x00_execute_fw(vha, srisc_address);
			/* Retrieve firmware information. */
			if (rval == QLA_SUCCESS) {
				fw_major_version = ha->fw_major_version;
				rval = qla2x00_get_fw_version(vha,
				    &ha->fw_major_version,
				    &ha->fw_minor_version,
				    &ha->fw_subminor_version,
				    &ha->fw_attributes, &ha->fw_memory_size,
				    ha->mpi_version, &ha->mpi_capabilities,
				    ha->phy_version);
				if (rval != QLA_SUCCESS)
					goto failed;

				ha->flags.npiv_supported = 0;
				if (IS_QLA2XXX_MIDTYPE(ha) &&
					 (ha->fw_attributes & BIT_2)) {
					ha->flags.npiv_supported = 1;
					if ((!ha->max_npiv_vports) ||
					    ((ha->max_npiv_vports + 1) %
					    MIN_MULTI_ID_FABRIC))
						ha->max_npiv_vports =
						    MIN_MULTI_ID_FABRIC - 1;
				}
				qla2x00_get_resource_cnts(vha, NULL,
				    &ha->fw_xcb_count, NULL, NULL,
				    &ha->max_npiv_vports);

				if (!fw_major_version && ql2xallocfwdump)
					qla2x00_alloc_fw_dump(vha);
			}
		} else {
			DEBUG2(printk(KERN_INFO
			    "scsi(%ld): ISP Firmware failed checksum.\n",
			    vha->host_no));
		}
	}

	if (!IS_FWI2_CAPABLE(ha) && !IS_QLA2100(ha) && !IS_QLA2200(ha)) {
		/* Enable proper parity. */
		spin_lock_irqsave(&ha->hardware_lock, flags);
		if (IS_QLA2300(ha))
			/* SRAM parity */
			WRT_REG_WORD(&reg->hccr, HCCR_ENABLE_PARITY + 0x1);
		else
			/* SRAM, Instruction RAM and GP RAM parity */
			WRT_REG_WORD(&reg->hccr, HCCR_ENABLE_PARITY + 0x7);
		RD_REG_WORD(&reg->hccr);
		spin_unlock_irqrestore(&ha->hardware_lock, flags);
	}

	if (rval == QLA_SUCCESS && IS_FAC_REQUIRED(ha)) {
		uint32_t size;

		rval = qla81xx_fac_get_sector_size(vha, &size);
		if (rval == QLA_SUCCESS) {
			ha->flags.fac_supported = 1;
			ha->fdt_block_size = size << 2;
		} else {
			qla_printk(KERN_ERR, ha,
			    "Unsupported FAC firmware (%d.%02d.%02d).\n",
			    ha->fw_major_version, ha->fw_minor_version,
			    ha->fw_subminor_version);
		}
	}
failed:
	if (rval) {
		DEBUG2_3(printk("scsi(%ld): Setup chip **** FAILED ****.\n",
		    vha->host_no));
	}

	return (rval);
}

/**
 * qla2x00_init_response_q_entries() - Initializes response queue entries.
 * @ha: HA context
 *
 * Beginning of request ring has initialization control block already built
 * by nvram config routine.
 *
 * Returns 0 on success.
 */
void
qla2x00_init_response_q_entries(struct rsp_que *rsp)
{
	uint16_t cnt;
	response_t *pkt;

	rsp->ring_ptr = rsp->ring;
	rsp->ring_index    = 0;
	rsp->status_srb = NULL;
	pkt = rsp->ring_ptr;
	for (cnt = 0; cnt < rsp->length; cnt++) {
		pkt->signature = RESPONSE_PROCESSED;
		pkt++;
	}
}

/**
 * qla2x00_update_fw_options() - Read and process firmware options.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
void
qla2x00_update_fw_options(scsi_qla_host_t *vha)
{
	uint16_t swing, emphasis, tx_sens, rx_sens;
	struct qla_hw_data *ha = vha->hw;

	memset(ha->fw_options, 0, sizeof(ha->fw_options));
	qla2x00_get_fw_options(vha, ha->fw_options);

	if (IS_QLA2100(ha) || IS_QLA2200(ha))
		return;

	/* Serial Link options. */
	DEBUG3(printk("scsi(%ld): Serial link options:\n",
	    vha->host_no));
	DEBUG3(qla2x00_dump_buffer((uint8_t *)&ha->fw_seriallink_options,
	    sizeof(ha->fw_seriallink_options)));

	ha->fw_options[1] &= ~FO1_SET_EMPHASIS_SWING;
	if (ha->fw_seriallink_options[3] & BIT_2) {
		ha->fw_options[1] |= FO1_SET_EMPHASIS_SWING;

		/*  1G settings */
		swing = ha->fw_seriallink_options[2] & (BIT_2 | BIT_1 | BIT_0);
		emphasis = (ha->fw_seriallink_options[2] &
		    (um; i++) {
		unsigned long msiqid = pbm->msiq_first + i;

		(void) pci_sun4v_msiq_conf(pbm->devhandle, msiqid, 0UL, 0);
	}

	q_size = pbm->msiq_ent_count * sizeof(struct pci_sun4v_msiq_entry);
	alloc_size = (pbm->msiq_num * q_size);
	order = get_order(alloc_size);

	pages = (unsigned long) pbm->msi_queues;

	free_pages(pages, order);

	pbm->msi_queues = NULL;
}

static int pci_sun4v_msiq_build_irq(struct pci_pbm_info *pbm,
				    unsigned long msiqid,
				    unsigned long devino)
{
	unsigned int virt_irq = sun4v_build_irq(pbm->devhandle, devino);

	if (!virt_irq)
		return -ENOMEM;

	if (pci_sun4v_msiq_setstate(pbm->devhandle, msiqid, HV_MSIQSTATE_IDLE))
		return -EINVAL;
	if (pci_sun4v_msiq_setvalid(pbm->devhandle, msiqid, HV_MSIQ_VALID))
		return -EINVAL;

	return virt_irq;
}

static const struct sparc64_msiq_ops pci_sun4v_msiq_ops = {
	.get_head	=	pci_sun4v_get_head,
	.dequeue_msi	=	pci_sun4v_dequeue_msi,
	.set_head	=	pci_sun4v_set_head,
	.msi_setup	=	pci_sun4v_msi_setup,
	.msi_teardown	=	pci_sun4v_msi_teardown,
	.msiq_alloc	=	pci_sun4v_msiq_alloc,
	.msiq_free	=	pci_sun4v_msiq_free,
	.msiq_build_irq	=	pci_sun4v_msiq_build_irq,
};

static void pci_sun4v_msi_init(struct pci_pbm_info *pbm)
{
	sparc64_pbm_msi_init(pbm, &pci_sun4v_msiq_ops);
}
#else /* CONFIG_PCI_MSI */
static void pci_sun4v_msi_init(struct pci_pbm_info *pbm)
{
}
#endif /* !(CONFIG_PCI_MSI) */

static int __devinit pci_sun4v_pbm_init(struct pci_pbm_info *pbm,
					struct of_device *op, u32 devhandle)
{
	struct device_node *dp = op->node;
	int err;

	pbm->numa_node = of_node_to_nid(dp);

	pbm->pci_ops = &sun4v_pci_ops;
	pbm->config_space_reg_bits = 12;

	pbm->index = pci_num_pbms++;

	pbm->op = op;

	pbm->devhandle = devhandle;

	pbm->name = dp->full_name;

	printk("%s: SUN4V PCI Bus Module\n", pbm->name);
	printk("%s: On NUMA node %d\n", pbm->name, pbm->numa_node);

	pci_determine_mem_io_space(pbm);

	pci_get_pbm_props(pbm);

	err = pci_sun4v_iommu_init(pbm);
	if (err)
		return err;

	pci_sun4v_msi_init(pbm);

	pci_sun4v_scan_bus(pbm, &op->dev);

	pbm->next = pci_pbm_root;
	pci_pbm_root = pbm;

	return 0;
}

static int __devinit pci_sun4v_probe(struct of_device *op,
				     const struct of_device_id *match)
{
	const struct linux_prom64_registers *regs;
	static int hvapi_negotiated = 0;
	struct pci_pbm_info *pbm;
	struct device_node *dp;
	struct iommu *iommu;
	u32 devhandle;
	int i, err;

	dp = op->node;

	if (!hvapi_negotiated++) {
		err = sun4v_hvapi_register(HV_GRP_PCI,
					   vpci_major,
					   &vpci_minor);

		if (err) {
			printk(KERN_ERR PFX "Could not register hvapi, "
			       "err=%d\n", err);
			return err;
		}
		printk(KERN_INFO PFX "Registered hvapi major[%lu] minor[%lu]\n",
		       vpci_major, vpci_minor);

		dma_ops = &sun4v_dma_ops;
	}

	regs = of_get_property(dp, "reg", NULL);
	err = -ENODEV;
	if (!regs) {
		printk(KERN_ERR PFX "Could not find config registers\n");
		goto out_err;
	}
	devhandle = (regs->phys_addr >> 32UL) & 0x0fffffff;

	err = -ENOMEM;
	if (!iommu_batch_initialized) {
		for_each_possible_cpu(i) {
			unsigned long page = get_zeroed_page(GFP_KERNEL);

			if (!page)
				goto out_err;

			per_cpu(iommu_batch, i).pglist = (u64 *) page;
		}
		iommu_batch_initialized = 1;
	}

	pbm = kzalloc(sizeof(*pbm), GFP_KERNEL);
	if (!pbm) {
		printk(KERN_ERR PFX "Could not allocate pci_pbm_info\n");
		goto out_err;
	}

	iommu = kzalloc(sizeof(struct iommu), GFP_KERNEL);
	if (!iommu) {
		printk(KERN_ERR PFX "Could not allocate pbm iommu\n");
		goto out_free_controller;
	}

	pbm->iommu = iommu;

	err = pci_sun4v_pbm_init(pbm, op, devhandle);
	if (err)
		goto out_free_iommu;

	dev_set_drvdata(&op->dev, pbm);

	return 0;

out_free_iommu:
	kfree(pbm->iommu);

out_free_controller:
	kfree(pbm);

out_err:
	return err;
}

static struct of_device_id __initdata pci_sun4v_match[] = {
	{
		.name = "pci",
		.compatible = "SUNW,sun4v-pci",
	},
	{},
};

static struct of_platform_driver pci_sun4v_driver = {
	.name		= DRIVER_NAME,
	.match_table	= pci_sun4v_match,
	.probe		= pci_sun4v_probe,
};

static int __init pci_sun4v_init(void)
{
	return of_registe
	if (ha->mqenable) {
		icb->qos = __constant_cpu_to_le16(QLA_DEFAULT_QUE_QOS);