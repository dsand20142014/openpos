*ttusb, int filter_id,
		     int associated_chan, u8 filter[8], u8 mask[8])
{
	int err;
	/* SetFilter */
	u8 b[] = { 0xaa, 0, 0x24, 0x1a, filter_id, associated_chan,
		filter[0], filter[1], filter[2], filter[3],
		filter[4], filter[5], filter[6], filter[7],
		filter[8], filter[9], filter[10], filter[11],
		mask[0], mask[1], mask[2], mask[3],
		mask[4], mask[5], mask[6], mask[7],
		mask[8], mask[9], mask[10], mask[11]
	};

	err = ttusb_cmd(ttusb, b, sizeof(b), 0);
	return err;
}

static int ttusb_del_filter(struct ttusb *ttusb, int filter_id)
{
	int err;
	/* DelFilter */
	u8 b[] = { 0xaa, ++ttusb->c, 0x25, 1, filter_id };

	err = ttusb_cmd(ttusb, b, sizeof(b), 0);
	return err;
}
#endif

static int ttusb_init_controller(struct ttusb *ttusb)
{
	u8 b0[] = { 0xaa, ++ttusb->c, 0x15, 1, 0 };
	u8 b1[] = { 0xaa, ++ttusb->c, 0x15, 1, 1 };
	u8 b2[] = { 0xaa, ++ttusb->c, 0x32, 1, 0 };
	/* i2c write read: 5 bytes, addr 0x10, 0x02 bytes write, 1 bytes read. */
	u8 b3[] =
	    { 0xaa, ++ttusb->c, 0x31, 5, 0x10, 0x02, 0x01, 0x00, 0x1e };
	u8 b4[] =
	    { 0x55, ttusb->c, 0x31, 4, 0x10, 0x02, 0x01, 0x00, 0x1e };

	u8 get_version[] = { 0xaa, ++ttusb->c, 0x17, 5, 0, 0, 0, 0, 0 };
	u8 get_dsp_version[0x20] =
	    { 0xaa, ++ttusb->c, 0x26, 28, 0, 0, 0, 0, 0 };
	int err;

	/* reset board */
	if ((err = ttusb_cmd(ttusb, b0, sizeof(b0), 0)))
		return err;

	/* reset board (again?) */
	if ((err = ttusb_cmd(ttusb, b1, sizeof(b1), 0)))
		return err;

	ttusb_boot_dsp(ttusb);

	/* set i2c bit rate */
	if ((err = ttusb_cmd(ttusb, b2, sizeof(b2), 0)))
		return err;

	if ((err = ttusb_cmd(ttusb, b3, sizeof(b3), 1)))
		return err;

	err = ttusb_result(ttusb, b4, sizeof(b4));

	if ((err = ttusb_cmd(ttusb, get_version, sizeof(get_version), 1)))
		return err;

	if ((err = ttusb_result(ttusb, get_version, sizeof(get_version))))
		return err;

	dprintk("%s: stc-version: %c%c%c%c%c\n", __func__,
		get_version[4], get_version[5], get_version[6],
		get_version[7], get_version[8]);

	if (memcmp(get_version + 4, "V 0.0", 5) &&
	    memcmp(get_version + 4, "V 1.1", 5) &&
	    memcmp(get_version + 4, "V 2.1", 5) &&
	    memcmp(get_version + 4, "V 2.2", 5)) {
		printk
		    ("%s: unknown STC version %c%c%c%c%c, please report!\n",
		     __func__, get_version[4], get_version[5],
		     get_version[6], get_version[7], get_version[8]);
	}

	ttusb->revision = ((get_version[6] - '0') << 4) |
			   (get_version[8] - '0');

	err =
	    ttusb_cmd(ttusb, get_dsp_version, sizeof(get_dsp_version), 1);
	if (err)
		return err;

	err =
	    ttusb_result(ttusb, get_dsp_version, sizeof(get_dsp_version));
	if (err)
		return err;
	printk("%s: dsp-version: %c%c%c\n", __func__,
	       get_dsp_version[4], get_dsp_version[5], get_dsp_version[6]);
	return 0;
}

#ifdef TTUSB_DISEQC
static int ttusb_send_diseqc(struct dvb_frontend* fe,
			     const struct dvb_diseqc_master_cmd *cmd)
{
	struct ttusb* ttusb = (struct ttusb*) fe->dvb->priv;
	u8 b[12] = { 0xaa, ++ttusb->c, 0x18 };

	int err;

	b[3] = 4 + 2 + cmd->msg_len;
	b[4] = 0xFF;		/* send diseqc master, not burst */
	b[5] = cmd->msg_len;

	memcpy(b + 5, cmd->msg, cmd->msg_len);

	/* Diseqc */
	if ((err = ttusb_cmd(ttusb, b, 4 + b[3], 0))) {
		dprintk("%s: usb_bulk_msg() failed, return value %i!\n",
			__func__, err);
	}

	return err;
}
#endif

static int ttusb_update_lnb(struct ttusb *ttusb)
{
	u8 b[] = { 0xaa, ++ttusb->c, 0x16, 5, /*power: */ 1,
		ttusb->voltage == SEC_VOLTAGE_18 ? 0 : 1,
		ttusb->tone == SEC_TONE_ON ? 1 : 0, 1, 1
	};
	int err;

	/* SetLNB */
	if ((err = ttusb_cmd(ttusb, b, sizeof(b), 0))) {
		dprintk("%s: usb_bulk_msg() failed, return value %i!\n",
			__func__, err);
	}

	return err;
}

static int ttusb_set_voltage(struct dvb_frontend* fe, fe_sec_voltage_t voltage)
{
	struct ttusb* ttusb = (struct ttusb*) fe->dvb->priv;

	ttusb->voltage = voltage;
	return ttusb_update_lnb(ttusb);
}

#ifdef TTUSB_TONE
static int ttusb_set_tone(struct dvb_frontend* fe, fe_sec_tone_mode_t tone)
{
	struct ttusb* ttusb = (struct ttusb*) fe->dvb->priv;

	ttusb->tone = tone;
	return ttusb_update_lnb(ttusb);
}
#endif


#if 0
static void ttusb_set_led_freq(struct ttusb *ttusb, u8 freq)
{
	u8 b[] = { 0xaa, ++ttusb->c, 0x19, 1, freq };
	int err, actual_len;

	err = ttusb_cmd(ttusb, b, sizeof(b), 0);
	if (err) {
		dprintk("%s: usb_bulk_msg() failed, return value %i!\n",
			__func__, err);
	}
}
#endif

/*****************************************************************************/

#ifdef TTUSB_HWSECTIONS
static void ttusb_handle_ts_data(struct ttusb_channel *channel,
				 const u8 * data, int len);
static void ttusb_handle_sec_data(struct ttusb_channel *channel,
				  const u8 * data, int len);
#endif

static int numpkt, numts, numstuff, numsec, numinvalid;
static unsigned long lastj;

static void ttusb_process_muxpack(struct ttusb *ttusb, const u8 * muxpack,
			   int len)
{
	u16 csum = 0, cc;
	int i;
	for (i = 0; i < len; i += 2)
		csum ^= le16_to_cpup((__le16 *) (muxpack + i));
	if (csum) {
		printk("%s: muxpack with incorrect checksum, ignoring\n",
		       __func__);
		numinvalid++;
		return;
	}

	cc = (muxpack[len - 4] << 8) | muxpack[len - 3];
	cc &= 0x7FFF;
	if ((cc != ttusb->cc) && (ttusb->cc != -1))
		printk("%s: cc discontinuity (%d frames missing)\n",
		       __func__, (cc - ttusb->cc) & 0x7FFF);
	ttusb->cc = (cc + 1) & 0x7FFF;
	if (muxpack[0] & 0x80) {
#ifdef TTUSB_HWSECTIONS
		/* section data */
		int pusi = muxpack[0] & 0x40;
		int channel = muxpack[0] & 0x1F;
		int payload = muxpack[1];
		const u8 *data = muxpack + 2;
		/* check offset flag */
		if (muxpack[0] & 0x20)
			data++;

		ttusb_handle_sec_data(ttusb->channel + channel, data,
				      payload);
		data += payload;

		if ((!!(ttusb->muxpack[0] & 0x20)) ^
		    !!(ttusb->muxpack[1] & 1))
			data++;
#warning TODO: pusi
		printk("cc: %04x\n", (data[0] << 8) | data[1]);
#endif
		numsec++;
	} else if (muxpack[0] == 0x47) {
#ifdef TTUSB_HWSECTIONS
		/* we have TS data here! */
		int pid = ((muxpack[1] & 0x0F) << 8) | muxpack[2];
		int channel;
		for (channel = 0; channel < TTUSB_MAXCHANNEL; ++channel)
			if (ttusb->channel[channel].active
			    && (pid == ttusb->channel[channel].pid))
				ttusb_handle_ts_data(ttusb->channel +
						     channel, muxpack,
						     188);
#endif
		numts++;
		dvb_dmx_swfilter_packets(&ttusb->dvb_demux, muxpack, 1);
	} else if (muxpack[0] != 0) {
		numinvalid++;
		printk("illegal muxpack type %02x\n", muxpack[0]);
	} else
		numstuff++;
}

static void ttusb_process_frame(struct ttusb *ttusb, u8 * data, int len)
{
	int maxwork = 1024;
	while (len) {
		if (!(maxwork--)) {
			printk("%s: too much work\n", __func__);
			break;
		}

		switch (ttusb->mux_state) {
		case 0:
		case 1:
		case 2:
			len--;
			if (*data++ == 0xAA)
				++ttusb->mux_state;
			else {
				ttusb->mux_state = 0;
#if DEBUG > 3
				if (ttusb->insync)
					printk("%02x ", data[-1]);
#else
				if (ttusb->insync) {
					printk("%s: lost sync.\n",
					       __func__);
					ttusb->insync = 0;
				}
#endif
			}
			break;
		case 3:
			ttusb->insync = 1;
			len--;
			ttusb->mux_npacks = *data++;
			++ttusb->mux_state;
			ttusb->muxpack_ptr = 0;
			/* maximum bytes, until we know the length */
			ttusb->muxpack_len = 2;
			break;
		case 4:
			{
				int avail;
				avail = len;
				if (avail >
				    (ttusb->muxpack_len -
				     ttusb->muxpack_ptr))
					avail =
					    ttusb->muxpack_len -
					    ttusb->muxpack_ptr;
				memcpy(ttusb->muxpack + ttusb->muxpack_ptr,
				       data, avail);
				ttusb->muxpack_ptr += avail;
				BUG_ON(ttusb->muxpack_ptr > 264);
				data += avail;
				len -= avail;
				/* determine length */
				if (ttusb->muxpack_ptr == 2) {
					if (ttusb->muxpack[0] & 0x80) {
						ttusb->muxpack_len =
						    ttusb->muxpack[1] + 2;
						if (ttusb->
						    muxpack[0] & 0x20)
							ttusb->
							    muxpack_len++;
						if ((!!
						     (ttusb->
						      muxpack[0] & 0x20)) ^
						    !!(ttusb->
						       muxpack[1] & 1))
							ttusb->
							    muxpack_len++;
						ttusb->muxpack_len += 4;
					} else if (ttusb->muxpack[0] ==
						   0x47)
						ttusb->muxpack_len =
						    188 + 4;
					else if (ttusb->muxpack[0] == 0x00)
						ttusb->muxpack_len =
						    ttu(capable);
