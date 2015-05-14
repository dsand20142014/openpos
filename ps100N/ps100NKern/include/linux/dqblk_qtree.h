_CONF1_ADDR_24, FA_NPIV_CONF1_ADDR,
			FA_NPIV_CONF1_ADDR_81 };
	uint32_t def;
	uint16_t *wptr;
	uint16_t cnt, chksum;
	uint32_t start;
	struct qla_flt_header *flt;
	struct qla_flt_region *region;
	struct qla_hw_data *ha = vha->hw;
	struct req_que *req = ha->req_q_map[0];

	ha->flt_region_flt = flt_addr;
	wptr = (uint16_t *)req->ring;
	flt = (struct qla_flt_header *)req->ring;
	region = (struct qla_flt_region *)&flt[1];
	ha->isp_ops->read_optrom(vha, (uint8_t *)req->ring,
	    flt_addr << 2, OPTROM_BURST_SIZE);
	if (*wptr == __constant_cpu_to_le16(0xffff))
		goto no_flash_data;
	if (flt->version != __constant_cpu_to_le16(1)) {
		DEBUG2(qla_printk(KERN_INFO, ha, "Unsupported FLT detected: "
		    "version=0x%x length=0x%x checksum=0x%x.\n",
		    le16_to_cpu(flt->version), le16_to_cpu(flt->length),
		    le16_to_cpu(flt->checksum)));
		goto no_flash_data;
	}

	cnt = (sizeof(struct qla_flt_header) + le16_to_cpu(flt->length)) >> 1;
	for (chksum = 0; cnt; cnt--)
		chksum += le16_to_cpu(*wptr++);
	if (chksum) {
		DEBUG2(qla_printk(KERN_INFO, ha, "Inconsistent FLT detected: "
		    "version=0x%x length=0x%x checksum=0x%x.\n",
		    le16_to_cpu(flt->version), le16_to_cpu(flt->length),
		    chksum));
		goto no_flash_data;
	}

	loc = locations[1];
	cnt = le16_to_cpu(flt->length) / sizeof(struct qla_flt_region);
	for ( ; cnt; cnt--, region++) {
		/* Store addresses as DWORD offsets. */
		start = le32_to_cpu(region->start) >> 2;

		DEBUG3(qla_printk(KERN_DEBUG, ha, "FLT[%02x]: start=0x%x "
		    "end=0x%x size=0x%x.\n", le32_to_cpu(region->code), start,
		    le32_to_cpu(region->end) >> 2, le32_to_cpu(region->size)));

		switch (le32_to_cpu(region->code) & 0xff) {
		case FLT_REG_FW:
			ha->flt_region_fw = start;
			break;
		case FLT_REG_BOOT_CODE:
			ha->flt_region_boot = start;
			break;
		case FLT_REG_VPD_0:
			ha->flt_region_vpd_nvram = start;
			if (ha->flags.port0)
				ha->flt_region_vpd = start;
			break;
		case FLT_REG_VPD_1:
			if (!ha->flags.port0)
				ha->flt_region_vpd = start;
			break;
		case FLT_REG_NVRAM_0:
			if (ha->flags.port0)
				ha->flt_region_nvram = start;
		