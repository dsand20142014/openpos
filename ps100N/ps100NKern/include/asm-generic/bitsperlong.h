constant_cpu_to_le16(0xffff))
		goto no_flash_data;
	if (fdt->sig[0] != 'Q' || fdt->sig[1] != 'L' || fdt->sig[2] != 'I' ||
	    fdt->sig[3] != 'D')
		goto no_flash_data;

	for (cnt = 0, chksum = 0; cnt < sizeof(struct qla_fdt_layout) >> 1;
	    cnt++)
		chksum += le16_to_cpu(*wptr++);
	if (chksum) {
		DEBUG2(qla_printk(KERN_INFO, ha, "Inconsistent FDT detected: "
		    "checksum=0x%x id=%c version=0x%x.\n", chksum, fdt->sig[0],
		    le16_to_cpu(fdt->version)));
		DEBUG9(qla2x00_dump_buffer((uint8_t *)fdt, sizeof(*fdt)));
		goto no_flash_data;
	}

	loc = locations[1];
	mid = le16_to_cpu(fdt->man_id);
	fid = le16_to_cpu(fdt->id);
	ha->fdt_wrt_disable = fdt->wrt_disable_bits;
	ha->fdt_erase_cmd = flash_conf_addr(ha, 0x0300 | fdt->erase_cmd);
	ha->fdt_block_size = le32_to_cpu(fdt->block_size);
	if (fdt->unprotect_sec_cmd) {
		ha->fdt_unprotect_sec_cmd = flash_conf_