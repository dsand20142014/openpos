sk[line].imr_offset, sih->bytes_ixr);
		if (status < 0)
			pr_err("twl4030: err %d initializing %s %s\n",
					status, sih->name, "IMR");

		/* Maybe disable "exclusive" mode; buffer second pending irq;
		 * set Clear-On-Read (COR) bit.
		 *
		 * NOTE that sometimes COR polarity is documented as being
		 * inverted:  for MADC and BCI, COR=1 means "clear on write".
		 * And for PWR_INT it's not documented...
		 */
		if (sih->set_cor) {
			status = twl4030_i2c_write_u8(sih->module,
					TWL4030_SIH_CTRL_COR_MASK,
					sih->control_offset);
			if (status < 0)
				pr_err("twl4030: err %d initializing %s %s\n",
						status, sih->name, "SIH_CTRL");
		}
	}

	sih = sih_modules;
	for (i = 0; i < ARRAY_SIZE(sih_modules); i++, sih++) {
		u8 rxbuf[4];
		int j;

		/* skip USB */
		if (!sih->bytes_ixr)
			continue;

		/* Clear pending interrupt status.  Either the read was
		 * enough, or we need to write those bits.  Repeat, in
		 * case an IRQ is pending (PENDDIS=0) ... that's not
		 * uncommon with PWR_INT.PWRON.
		 */
		for (j = 0;