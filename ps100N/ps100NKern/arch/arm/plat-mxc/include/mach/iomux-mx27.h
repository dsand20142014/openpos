us[bus].target[target].flags.flags1x160.sync_offset << 8;
	else
		mb[3] =	nv->bus[bus].target[target].flags.flags1x80.sync_offset << 8;
	mb[3] |= nv->bus[bus].target[target].sync_period;
	status = qla1280_mailbox_command(ha, 0x0f, mb);

	/* Save Tag queuing enable flag. */
	flag = (BIT_0 << target);
	if (nv->bus[bus].target[target].parameter.tag_queuing)
		ha->bus_settings[bus].qtag_enables |= flag;

	/* Save Device enable flag. */
	if (IS_ISP1x160(ha)) {
		if (nv->bus[bus].target[target].flags.flags1x160.device_enable)
			ha->bus_settings[bus].device_enables |= flag;
		ha->bus_settings[bus].lun_disables |= 0;
	} else {
		if (nv->bus[bus].target[target].flags.flags1x80.device_enable)
			ha->bus_settings[bus].device_enables |= flag;
		/* Save LUN disable flag. */
		if (nv->bus[bus].target[target].flags.flags1x80.lun_disable)
			ha->bus_settings[bus].lun_disables |= flag;
	}

	/* Set Device Queue Parameters. */
	for (lun = 0; lun < MAX_LUNS; lun++) {
		mb[0] = MBC_SET_DEVICE_QUEUE;
		mb[1] = (uint16_t)((bus ? target | BIT_7 : target) << 8);
		mb[1] |= lun;
		mb[2] = nv->bus[bus].max_queue_depth;
		mb[3] = nv->bus[bus].target[target].execution_throttle;
		status |= qla1280_mailbox_command(ha, 0x0f, mb);
	}

	return status;
}

static int
qla1280_config_bus(struct scsi_qla_host *ha, int bus)
{
	struct nvram *nv = &ha->nvram;
	uint16_t mb[MAILBOX_REGISTER_COUNT];
	int target, status;

	/* SCSI Reset Disable. */
	ha->bus_settings[bus].disable_scsi_reset =
		nv->bus[bus].config_1.scsi_reset_disable;

	/* Initiator ID. */
	ha->bus_settings[bus].id = nv->bus[bus].config_1.initiator_id;
	mb[0] = MBC_SET_INITIATOR_ID;
	mb[1] = bus ? ha->bus_settings[bus].id | BIT_7 :
		ha->bus_settings[bus].id;
	status = qla1280_mailbox_command(ha, BIT_1 | BIT_0, &mb[0]);

	/* Reset Delay. */
	ha->bus_settings[bus].bus_reset_delay =
		nv->bus[bus].bus_reset_delay;

	/* Command queue depth per device. */
	ha->bus_settings[bus].hiwat = nv->bus[bus].max_queue_depth - 1;

	/* Set target parameters. */
	for (target = 0; target < MAX_TARGETS; target++)
		status |= qla1280_config_target(ha, bus, target);

	return status;
}

static int
qla1280_nvram_config(struct scsi_qla_host *ha)
{
	struct device_reg __iomem *reg = ha->iobase;
	struct nvram *nv = &ha->nvram;
	int bus, target, status = 0;
	uint16_t mb[MAILBOX_REGISTER_COUNT];

	ENTER("qla1280_nvram_config");

	if (ha->nvram_valid) {
		/* Always force AUTO sense for LINUX SCSI */
		for (bus = 0; bus < MAX_BUSES; bus++)
			for (target = 0; target < MAX_TARGETS; target++) {
				nv->bus[bus].target[target].parameter.
					auto_request_sense = 1;
			}
	} else {
		qla1280_set_defaults(ha);
	}

	qla1280_print_settings(nv);

	/* Disable RISC load of firmware. */
	ha->flags.disable_risc_code_load =
		nv->cntr_flags_1.disable_loading_risc_code;

	if (IS_ISP1040(ha)) {
		uint16_t hwrev, cfg1, cdma_conf, ddma_conf;

		hwrev = RD_REG_WORD(&reg->cfg_0) & ISP_CFG0_HWMSK;

		cfg1 = RD_REG_WORD(&reg->cfg_1) & ~(BIT_4 | BIT_5 | BIT_6);
		cdma_conf = RD_REG_WORD(&reg->cdma_cfg);
		ddma_conf = RD_REG_WORD(&reg->ddma_cfg);

		/* Busted fifo, says mjacob. */
		if (hwrev != ISP_CFG0_1040A)
			cfg1 |= nv->isp_config.fifo_threshold << 4;

		cfg1 |= nv->isp_config.burst_enable << 2;
		WRT_REG_WORD(&reg->cfg_1, cfg1);

		WRT_REG_WORD(&reg->cdma_cfg, cdma_conf | CDMA_CONF_BENAB);
		WRT_REG_WORD(&reg->ddma_cfg, cdma_conf | DDMA_CONF_BENAB);
	} else {
		uint16_t cfg1, term;

		/* Set ISP hardware DMA burst */
		cfg1 = nv->isp_config.fifo_threshold << 4;
		cfg1 |= nv->isp_config.burst_enable << 2;
		/* Enable DMA arbitration on dual channel controllers */
		if (ha->ports > 1)
			cfg1 |= BIT_13;
		WRT_REG_WORD(&reg->cfg_1, cfg1);

		/* Set SCSI termination. */
		WRT_REG_WORD(&reg->gpio_enable,
			     BIT_7 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
		term = nv->termination.scsi_bus_1_control;
		term |= nv->termination.scsi_bus_0_control << 2;
		term |= nv->termination.auto_term_support << 7;
		RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
		WRT_REG_WORD(&reg->gpio_data, term);
	}
	RD_REG_WORD(&reg->id_l);	/* Flush PCI write */

	/* ISP parameter word. */
	mb[0] = MBC_SET_SYSTEM_PARAMETER;
	mb[1] = nv->isp_parameter;
	status |= qla1280_mailbox_command(ha, BIT_1 | BIT_0, &mb[0]);

	if (IS_ISP1x40(ha)) {
		/* clock rate - for qla1240 and older, only */
		mb[0] = MBC_SET_CLOCK_RATE;
		mb[1] = 40;
	 	status |= qla1280_mailbox_command(ha, BIT_1 | BIT_0, mb);
	}

	/* Firmware feature word. */
	mb[0] = MBC_SET_FIRMWARE_FEATURES;
	mb[1] = nv->firmware_feature.f.enable_fast_posting;
	mb[1] |= nv->firmware_feature.f.report_lvd_bus_transition << 1;
	mb[1] |= nv->firmware_feature.f.disable_synchronous_backoff << 5;
#if defined(CONFIG_IA64_GENERIC) || defined (CONFIG_IA64_SGI_SN2)
	if (ia64_platform_is("sn2")) {
		printk(KERN_INFO "scsi(%li): Enabling SN2 PCI DMA "
		       "workaround\n", ha->host_no);
		mb[1] |= nv->firmware_feature.f.unused_9 << 9; /* XXX */
	}
#endif
	status |= qla1280_mailbox_command(ha, BIT_1 | BIT_0, mb);

	/* Retry count and delay. */
	mb[0] = MBC_SET_RETRY_COUNT;
	mb[1] = nv->bus[0].retry_count;
	mb[2] = nv->bus[0].retry_delay;
	mb[6] = nv->bus[1].retry_count;
	mb[7] = nv->bus[1].retry_delay;
	status |= qla1280_mailbox_command(ha, BIT_7 | BIT_6 | BIT_2 |
					  BIT_1 | BIT_0, &mb[0]);

	/* ASYNC data setup time. */
	mb[0] = MBC_SET_ASYNC_DATA_SETUP;
	mb[1] = nv->bus[0].config_2.async_data_setup_time;
	mb[2] = nv->bus[1].config_2.async_data_setup_time;
	status |= qla1280_mailbox_command(ha, BIT_2 | BIT_1 | BIT_0, &mb[0]);

	/* Active negation states. */
	mb[0] = MBC_SET_ACTIVE_NEGATION;
	mb[1] = 0;
	if (nv->bus[0].config_2.req_ack_active_negation)
		mb[1] |= BIT_5;
	if (nv->bus[0].config_2.data_line_active_negation)
		mb[1] |= BIT_4;
	mb[2] = 0;
	if (nv->bus[1].config_2.req_ack_active_negation)
		mb[2] |= BIT_5;
	if (nv->bus[1].config_2.data_line_active_negation)
		mb[2] |= BIT_4;
	status |= qla1280_mailbox_command(ha, BIT_2 | BIT_1 | BIT_0, mb);

	mb[0] = MBC_SET_DATA_OVERRUN_RECOVERY;
	mb[1] = 2;	/* Reset SCSI bus and return all outstanding IO */
	status |= qla1280_mailbox_command(ha, BIT_1 | BIT_0, mb);

	/* thingy */
	mb[0] = MBC_SET_PCI_CONTROL;
	mb[1] = BIT_1;	/* Data DMA Channel Burst Enable */
	mb[2] = BIT_1;	/* Command DMA Channel Burst Enable */
	status |= qla1280_mailbox_command(ha, BIT_2 | BIT_1 | BIT_0, mb);

	mb[0] = MBC_SET_TAG_AGE_LIMIT;
	mb[1] = 8;
	status |= qla1280_mailbox_command(ha, BIT_1 | BIT_0, mb);

	/* Selection timeout. */
	mb[0] = MBC_SET_SELECTION_TIMEOUT;
	mb[1] = nv->bus[0].selection_timeout;
	mb[2] = nv->bus[1].selection_timeout;
	status |= qla1280_mailbox_command(ha, BIT_2 | BIT_1 | BIT_0, mb);

	for (bus = 0; bus < ha->ports; bus++)
		status |= qla1280_config_bus(ha, bus);

	if (status)
		dprintk(2, "qla1280_nvram_config: **** FAILED ****\n");

	LEAVE("qla1280_nvram_config");
	return status;
}

/*
 * Get NVRAM data word
 *      Calculates word position in NVRAM and calls request routine to
 *      get the word from NVRAM.
 *
 * Input:
 *      ha      = adapter block pointer.
 *      address = NVRAM word address.
 *
 * Returns:
 *      data word.
 */
static uint16_t
qla1280_get_nvram_word(struct scsi_qla_host *ha, uint32_t address)
{
	uint32_t nv_cmd;
	uint16_t data;

	nv_cmd = address << 16;
	nv_cmd |= NV_READ_OP;

	data = le16_to_cpu(qla1280_nvram_request(ha, nv_cmd));

	dprintk(8, "qla1280_get_nvram_word: exiting normally NVRAM data = "
		"0x%x", data);

	return data;
}

/*
 * NVRAM request
 *      Sends read command to NVRAM and gets data from NVRAM.
 *
 * Input:
 *      ha     = adapter block pointer.
 *      nv_cmd = Bit 26     = start bit
 *               Bit 25, 24 = opcode
 *               Bit 23-16  = address
 *               Bit 15-0   = write data
 *
 * Returns:
 *      data word.
 */
static uint16_t
qla1280_nvram_request(struct scsi_qla_host *ha, uint32_t nv_cmd)
{
	struct device_reg __iomem *reg = ha->iobase;
	int cnt;
	uint16_t data = 0;
	uint16_t reg_data;

	/* Send command to NVRAM. */

	nv_cmd <<= 5;
	for (cnt = 0; cnt < 11; cnt++) {
		if (nv_cmd & BIT_31)
			qla1280_nv_write(ha, NV_DATA_OUT);
		else
			qla1280_nv_write(ha, 0);
		nv_cmd <<= 1;
	}

	/* Read data from NVRAM. */

	for (cnt = 0; cnt < 16; cnt++) {
		WRT_REG_WORD(&reg->nvram, (NV_SELECT | NV_CLOCK));
		RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
		NVRAM_DELAY();
		data <<= 1;
		reg_data = RD_REG_WORD(&reg->nvram);
		if (reg_data & NV_DATA_IN)
			data |= BIT_0;
		WRT_REG_WORD(&reg->nvram, NV_SELECT);
		RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
		NVRAM_DELAY();
	}

	/* Deselect chip. */

	WRT_REG_WORD(&reg->nvram, NV_DESELECT);
	RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
	NVRAM_DELAY();

	return data;
}

static void
qla1280_nv_write(struct scsi_qla_host *ha, uint16_t data)
{
	struct device_reg __iomem *reg = ha->iobase;

	WRT_REG_WORD(&reg->nvram, data | NV_SELECT);
	RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
	NVRAM_DELAY();
	WRT_REG_WORD(&reg->nvram, data | NV_SELECT | NV_CLOCK);
	RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
	NVRAM_DELAY();
	WRT_REG_WORD(&reg->nvram, data | NV_SELECT);
	RD_REG_WORD(&reg->id_l);	/* Flush PCI write */
	NVRAM_DELAY();
}

/*
 * Mailbox Command
 *      Issue mailbox command and waits for completion.
 *
 * Input:
 *      ha = adapter block pointer.
 *      mr = mailbox registers to load.
 *      mb = data pointer for mailbox registers.
 *
 * Output:
 *      mb[MAILBOX_REGISTER_COUNT] = returned mailbox data.
 *
 * Returns:
 *      0 = success
 */
static int
qla1280_mailbox_command(struct scsi_qla_host *ha, uint8_t mr, uint16_t *mb)
{
	struct device_reg __iomem *reg = ha->iobase;
	int status = 0;
	int cnt;
	uint16_t *optr, *iptr;
	uint16_t __iomem *mptr;
	uint16_t data;
	DECLARE_COMPLETION_ONSTACK(wait);
	struct timer_list timer;

	ENTER("qla1280_mailbox_command");

	if (ha->mailbox_wait) {
		printk(KERN_ERR "Warning mailbox wait already in use!\n");
	}
	ha->mailbox_wait = &wait;

	/*
	 * We really should start out by verifying that the mailbox is
	 * available before starting sending the command data
	 */
	/* Load mailbox registers. */
	mptr = (uint16_t __iomem *) &reg->mailbox0;
	iptr = mb;
	for (cnt = 0; cnt < MAILBOX_REGISTER_COUNT; cnt++) {
		if (mr & BIT_0) {
			WRT_REG_WORD(mptr, (*iptr));
		}

		mr >>= 1;
		mptr++;
		iptr++;
	}

	/* Issue set host interrupt command. */

	/* set up a timer just in case we're really jammed */
	init_timer(&timer);
	timer.expires = jiffies + 20*HZ;
	timer.data = (unsigned long)ha;
	timer.function = qla1280_mailbox_timeout;
	add_timer(&timer);

	spin_unlock_irq(ha->host->host_lock);
	WRT_REG_WORD(&reg->host_cmd, HC_SET_HOST_INT);
	data = qla1280_debounce_register(&reg->istatus);

	wait_for_completion(&wait);
	del_timer_sync(&timer);

	spin_lock_irq(ha->host->host_lock);

	ha->mailbox_wait = NULL;

	/* Check for mailbox command timeout. */
	if (ha->mailbox_out[0] != MBS_CMD_CMP) {
		printk(KERN_WARNING "qla1280_mailbox_command: Command failed, "
		       "mailbox0 = 0x%04x, mailbox_out0 = 0x%04x, istatus = "
		       "0x%04x\n", 
		       mb[0], ha->mailbox_out[0], RD_REG_WORD(&reg->istatus));
		printk(KERN_WARNING "m0 %04x, m1 %04x, m2 %04x, m3 %04x\n",
		       RD_REG_WORD(&reg->mailbox0), RD_REG_WORD(&reg->mailbox1),
		       RD_REG_WORD(&reg->mailbox2), RD_REG_WORD(&reg->mailbox3));
		printk(KERN_WARNING "m4 %04x, m5 %04x, m6 %04x, m7 %04x\n",
		       RD_REG_WORD(&reg->mailbox4), RD_REG_WORD(&reg->mailbox5),
		       RD_REG_WORD(&reg->mailbox6), RD_REG_WORD(&reg->mailbox7));
		status = 1;
	}

	/* Load return mailbox registers. */
	optr = mb;
	iptr = (uint16_t *) &ha->mailbox_out[0];
	