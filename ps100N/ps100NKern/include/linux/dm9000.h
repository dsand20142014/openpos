delay(10);
		else
			rval = QLA_FUNCTION_TIMEOUT;
		cond_resched();
	}
	return rval;
}

static void
qla24xx_get_flash_manufacturer(struct qla_hw_data *ha, uint8_t *man_id,
    uint8_t *flash_id)
{
	uint32_t ids;

	ids = qla24xx_read_flash_dword(ha, flash_conf_addr(ha, 0x03ab));
	*man_id = LSB(ids);
	*flash_id = MSB(ids);

	/* Check if man_id and flash_id are valid. */
	if (ids != 0xDEADDEAD && (*man_id == 0 || *flash_id == 0)) {
		/* Read information using 0x9f opcode
		 * Device ID, Mfg ID would be read in the format:
		 *   <Ext Dev Info><Device ID Part2><Device ID Part 1><Mfg ID>
		 * Example: ATMEL 0x00 01 45 1F
		 * Extract MFG and Dev ID from last two bytes.
		 */
		ids = qla24xx_read_flash_dword(ha, flash_conf_addr(ha, 0x009f));
		*man_id = LSB(ids);
		*flash_id = MSB(ids);
	}
}

static int
qla2xxx_find_flt_start(scsi_qla_host_t *vha, uint32_t *start)
{
	const char *loc, *locations[] = { "DEF", "PCI" };
	uint32_t pcihdr, pcids;
	uint32_t *dcode;
	uint8_t *buf, *bcode, last_image;
	uint16_t cnt, chksum, *wptr;
	struct qla_flt_location *fltl;
	struct qla_hw_data *ha = vha->hw;
	