ardware controlled radio.
	 */
#ifdef CONFIG_RT2X00_LIB_RFKILL
	if (rt2x00_get_field16(eeprom, EEPROM_NIC_HW_RADIO))
		__set_bit(CONFIG_SUPPORT_HW_BUTTON, &rt2x00dev->flags);
#endif /* CONFIG_RT2X00_LIB_RFKILL */

	/*
	 * Store led settings, for correct led behaviour.
	 */
#ifdef CONFIG_RT2X00_LIB_LEDS
	rt2800usb_init_led(rt2x00dev, &rt2x00dev->led_radio, LED_TYPE_RADIO);
	rt2800usb_init_led(rt2x00dev, &rt2x00dev->led_assoc, LED_TYPE_ASSOC);
	rt2800usb_init_led(rt2x00dev, &rt2x00dev->led_qual, LED_TYPE_QUALITY);

	rt2x00_eeprom_read(rt2x00dev, EEPROM_FREQ,
			   &rt2x00dev->led_mcu_reg);
#endif /* CONFIG_RT2X00_LIB_LEDS */

	return 0;
}

/*
 * RF value list for rt2870
 * Supports: 2.4 GHz (all) & 5.2 GHz (RF2850 & RF2750)
 */
static const struct rf_channel rf_vals[] = {
	{ 1,  0x18402ecc, 0x184c0786, 0x1816b455, 0x1800510b },
	{ 2,  0x18402ecc, 0x184c0786, 0x18168a55, 0x1800519f },
	{ 3,  0x18402ecc, 0x184c078a, 0x18168a55, 0x1800518b },
	{ 4,  0x18402ecc, 0x184c078a, 0x18168a55, 0x1800519f },
	{ 5,  0x18402ecc, 0x184c078e, 0x18168a55, 0x1800518b },
	{ 6,  0x18402ecc, 0x184c078e, 0x18168a55, 0x1800519f },
	{ 7,  0x18402ecc, 0x184c0792, 0x18168a55, 0x1800518b },
	{ 8,  0x18402ecc, 0x184c0792, 0x18168a55, 0x1800519f },
	{ 9,  0x18402ecc, 0x184c0796, 0x18168a55, 0x1800518b },
	{ 10, 0x18402ecc, 0x184c0796, 0x18168a55, 0x1800519f },
	{ 11, 0x18402ecc, 0x184c079a, 0x18168a55, 0x1800518b },
	{ 12, 0x18402ecc, 0x184c079a, 0x18168a55, 0x1800519f },
	{ 13, 0x18402ecc, 0x184c079e, 0x18168a55, 0x1800518b },
	{ 14, 0x18402ecc, 0x184c07a2, 0x18168a55, 0x18005193 },

	/* 802.11 UNI / HyperLan 2 */
	{ 36, 0x18402ecc, 0x184c099a, 0x18158a55, 0x180ed1a3 },
	{ 38, 0x18402ecc, 0x184c099e, 0x18158a55, 0x180ed193 },
	{ 40, 0x18402ec8, 0x184c0682, 0x18158a55, 0x180ed183 },
	{ 44, 0x18402ec8, 0x184c0682, 0x18158a55, 0x180ed1a3 },
	{ 46, 0x18402ec8, 0x184c0686, 0x18158a55, 0x180ed18b },
	{ 48, 0x18402ec8, 0x184c0686, 0x18158a55, 0x180ed19b },
	{ 52, 0x18402ec8, 0x184c068a, 0x18158a55, 0x180ed193 },
	{ 54, 0x18402ec8, 0x184c068a, 0x18158a55, 0x180ed1a3 },
	{ 56, 0x18402ec8, 0x184c068e, 0x18158a55, 0x180ed18b },
	{ 60, 0x18402ec8, 0x184c0692, 0x18158a55, 0x180ed183 },
	{ 62, 0x18402ec8, 0x184c0692, 0x18158a55, 0x180ed193 },
	{ 64, 0x18402ec8, 0x184c0692, 0x18158a55, 0x180ed1a3 },

	/* 802.11 HyperLan 2 */
	{ 100, 0x18402ec8, 0x184c06b2, 0x18178a55, 0x180ed783 },
	{ 102, 0x18402ec8, 0x184c06b2, 0x18578a55, 0x180ed793 },
	{ 104, 0x18402ec8, 0x185c06b2, 0x18578a55, 0x180ed1a3 },
	{ 108, 0x18402ecc, 0x185c0a32, 0x18578a55, 0x180ed193 },
	{ 110, 0x18402ecc, 0x184c0a36, 0x18178a55, 0x180ed183 },
	{ 112, 0x18402ecc, 0x184c0a36, 0x18178a55, 0x180ed19b },
	{ 116, 0x18402ecc, 0x184c0a3a, 0x18178a55, 0x180ed1a3 },
	{ 118, 0x18402ecc, 0x184c0a3e, 0x18178a55, 0x180ed193 },
	{ 120, 0x18402ec4, 0x184c0382, 0x18178a55, 0x180ed183 },
	{ 124, 0x18402ec4, 0x184c0382, 0x18178a55, 0x180ed193 },
	{ 126, 0x18402ec4, 0x184c0382, 0x18178a55, 0x180ed15b },
	{ 128, 0x18402ec4, 0x184c0382, 0x18178a55, 0x180ed1a3 },
	{ 132, 0x18402ec4, 0x184c0386, 0x18178a55, 0x180ed18b },
	{ 134, 0x18402ec4, 0x184c0386, 0x18178a55, 0x180ed193 },
	{ 136, 0x18402ec4, 0x184c0386, 0x18178a55, 0x180ed19b },
	{ 140, 0x18402ec4, 0x184c038a, 0x18178a55, 0x180ed183 },

	/* 802.11 UNII */
	{ 149, 0x18402ec4, 0x184c038a, 0x18178a55, 0x180ed1a7 },
	{ 151, 0x18402ec4, 0x184c038e, 0x18178a55, 0x180ed187 },
	{ 153, 0x18402ec4, 0x184c038e, 0x18178a55, 0x180ed18f },
	{ 157, 0x18402ec4, 0x184c038e, 0x18178a55, 0x180ed19f },
	{ 159, 0x18402ec4, 0x184c038e, 0x18178a55, 0x180ed1a7 },
	{ 161, 0x18402ec4, 0x184c0392, 0x18178a55, 0x180ed187 },
	{ 165, 0x18402ec4, 0x184c0392, 0x18178a55, 0x180ed197 },
	{ 167, 0x18402ec4, 0x184c03d2, 0x18179855, 0x1815531f },
	{ 169, 0x18402ec4, 0x184c03d2, 0x18179855, 0x18155327 },
	{ 171, 0x18402ec4, 0x184c03d6, 0x18179855, 0x18155307 },
	{ 173, 0x18402ec4, 0x184c03d6, 0x18179855, 0x1815530f },

	/* 802.11 Japan */
	{ 184, 0x15002ccc, 0x1500491e, 0x1509be55, 0x150c0a0b },
	{ 188, 0x15002ccc, 0x15004922, 0x1509be55, 0x150c0a13 },
	{ 192, 0x15002ccc, 0x15004926, 0x1509be55, 0x150c0a1b },
	{ 196, 0x15002ccc, 0x1500492a, 0x1509be55, 0x150c0a23 },
	{ 208, 0x15002ccc, 0x1500493a, 0x1509be55, 0x150c0a13 },
	{ 212, 0x15002ccc, 0x1500493e, 0x1509be55, 0x150c0a1b },
	{ 216, 0x15002ccc, 0x15004982, 0x1509be55, 0x150c0a23 },
};

/*
 * RF value list for rt3070
 * Supports: 2.4 GHz
 */
static const struct rf_channel rf_vals_3070[] = {
	{1,  241, 2, 2 },
	{2,  241, 2, 7 },
	{3,  242, 2, 2 },
	{4,  242, 2, 7 },
	{5,  243, 2, 2 },
	{6,  243, 2, 7 },
	{7,  244, 2, 2 },
	{8,  244, 2, 7 },
	{9,  245, 2, 2 },
	{10, 245, 2, 7 },
	{11, 246, 2, 2 },
	{12, 246, 2, 7 },
	{13, 247, 2, 2 },
	{14, 248, 2, 4 },
};

static int rt2800usb_probe_hw_mode(struct rt2x00_dev *rt2x00dev)
{
	struct hw_mode_spec *spec = &rt2x00dev->spec;
	struct channel_info *info;
	char *tx_power1;
	char *tx_power2;
	unsigned int i;
	u16 eeprom;

	/*
	 * Initialize all hw fields.
	 */
	rt2x00dev->hw->flags =
	    IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING |
	    IEEE80211_HW_SIGNAL_DBM |
	    IEEE80211_HW_SUPPORTS_PS |
	    IEEE80211_HW_PS_NULLFUNC_STACK;
	rt2x00dev->hw->extra_tx_headroom = TXINFO_DESC_SIZE + TXWI_DESC_SIZE;

	SET_IEEE80211_DEV(rt2x00dev->hw, rt2x00dev->dev);
	SET_IEEE80211_PERM_ADDR(rt2x00dev->hw,
				rt2x00_eeprom_addr(rt2x00dev,
						   EEPROM_MAC_ADDR_0));

	rt2x00_eeprom_read(rt2x00dev, EEPROM_ANTENNA, &eeprom);

	/*
	 * Initialize HT information.
	 */
	spec->ht.ht_supported = true;
	spec->ht.cap =
	    IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
	    IEEE80211_HT_CAP_GRN_FLD |
	    IEEE80211_HT_CAP_SGI_20 |
	    IEEE80211_HT_CAP_SGI_40 |
	    IEEE80211_HT_CAP_TX_STBC |
	    IEEE80211_HT_CAP_RX_STBC |
	    IEEE80211_HT_CAP_PSMP_SUPPORT;
	spec->ht.ampdu_factor = 3;
	spec->ht.ampdu_density = 4;
	spec->ht.mcs.tx_params =
	    IEEE80211_HT_MCS_TX_DEFINED |
	    IEEE80211_HT_MCS_TX_RX_DIFF |
	    ((rt2x00_get_field16(eeprom, EEPROM_ANTENNA_TXPATH) - 1) <<
		IEEE80211_HT_MCS_TX_MAX_STREAMS_SHIFT);

	switch (rt2x00_get_field16(eeprom, EEPROM_ANTENNA_RXPATH)) {
	case 3:
		spec->ht.mcs.rx_mask[2] = 0xff;
	case 2:
		spec->ht.mcs.rx_mask[1] = 0xff;
	case 1:
		spec->ht.mcs.rx_mask[0] = 0xff;
		spec->ht.mcs.rx_mask[4] = 0x1; /* MCS32 */
		break;
	}

	/*
	 * Initialize hw_mode information.
	 */
	spec->supported_bands = SUPPORT_BAND_2GHZ;
	spec->supported_rates = SUPPORT_RATE_CCK | SUPPORT_RATE_OFDM;

	if (rt2x00_rf(&rt2x00dev->chip, RF2820) ||
	    rt2x00_rf(&rt2x00dev->chip, RF2720)) {
		spec->num_channels = 14;
		spec->channels = rf_vals;
	} else if (rt2x00_rf(&rt2x00dev->chip, RF2850) ||
		   rt2x00_rf(&rt2x00dev->chip, RF2750)) {
		spec->supported_bands |= SUPPORT_BAND_5GHZ;
		spec->num_channels = ARRAY_SIZE(rf_vals);
		spec->channels = rf_vals;
	} else if (rt2x00_rf(&rt2x00dev->chip, RF3020) ||
		   rt2x00_rf(&rt2x00dev->chip, RF2020)) {
		spec->num_channels = ARRAY_SIZE(rf_vals_3070);
		spec->channels = rf_vals_3070;
	}

	/*
	 * Create channel information array
	 */
	info = kzalloc(spec->num_channels * sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	spec->channels_info = info;

	tx_power1 = rt2x00_eeprom_addr(rt2x00dev, EEPROM_TXPOWER_BG1);
	tx_power2 = rt2x00_eeprom_addr(rt2x00dev, EEPROM_TXPOWER_BG2);

	for (i = 0; i < 14; i++) {
		info[i].tx_power1 = TXPOWER_G_FROM_DEV(tx_power1[i]);
		info[i].tx_power2 = TXPOWER_G_FROM_DEV(tx_power2[i]);
	}

	if (spec->num_channels > 14) {
		tx_power1 = rt2x00_eeprom_addr(rt2x00dev, EEPROM_TXPOWER_A1);
		tx_power2 = rt2x00_eeprom_addr(rt2x00dev, EEPROM_TXPOWER_A2);

		for (i = 14; i < spec->num_channels; i++) {
			info[i].tx_power1 = TXPOWER_A_FROM_DEV(tx_power1[i]);
			info[i].tx_power2 = TXPOWER_A_FROM_DEV(tx_power2[i]);
		}
	}

	return 0;
}

static int rt2800usb_probe_hw(struct rt2x00_dev *rt2x00dev)
{
	int retval;

	/*
	 * Allocate eeprom data.
	 */
	retval = rt2800usb_validate_eeprom(rt2x00dev);
	if (retval)
		return retval;

	retval = rt2800usb_init_eeprom(rt2x00dev);
	if (retval)
		return retval;

	/*
	 * Initialize hw specifications.
	 */
	retval = rt2800usb_probe_hw_mode(rt2x00dev);
	if (retval)
		return retval;

	/*
	 * This device requires firmware.
	 */
	__set_bit(DRIVER_REQUIRE_FIRMWARE, &rt2x00dev->flags);
	__set_bit(DRIVER_REQUIRE_SCHEDULED, &rt2x00dev->flags);
	__set_bit(DRIVER_REQUIRE_L2PAD, &rt2x00dev->flags);
	if (!modparam_nohwcrypt)
		__set_bit(CONFIG_SUPPORT_HW_CRYPTO, &rt2x00dev->flags);

	/*
	 * Set the rssi offset.
	 */
	rt2x00dev->rssi_offset = DEFAULT_RSSI_OFFSET;

	return 0;
}

/*
 * IEEE80211 stack callback functions.
 */
static void rt2800usb_get_tkip_seq(struct ieee80211_hw *hw, u8 hw_key_idx,
				   u32 *iv32, u16 *iv16)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct mac_iveiv_entry iveiv_entry;
	u32 offset;

	offset = MAC_IVEIV_ENTRY(hw_key_idx);
	rt2x00usb_register_multiread(rt2x00dev, offset,
				      &iveiv_entry, sizeof(iveiv_entry));

	memcpy(&iveiv_entry.iv[0], iv16, sizeof(iv16));
	memcpy(&iveiv_entry.iv[4], iv32, sizeof(iv32));
}

static int rt2800usb_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	u32 reg;
	bool enabled = (value < IEEE80211_MAX_RTS_THRESHOLD);

	rt2x00usb_register_read(rt2x00dev, TX_RTS_CFG, &reg);
	rt2x00_set_field32(&reg, TX_RTS_CFG_RTS_THRES, value);
	rt2x00usb_register_write(rt2x00dev, TX_RTS_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, CCK_PROT_CFG, &reg);
	rt2x00_set_field32(&reg, CCK_PROT_CFG_RTS_TH_EN, enabled);
	rt2x00usb_register_write(rt2x00dev, CCK_PROT_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, OFDM_PROT_CFG, &reg);
	rt2x00_set_field32(&reg, OFDM_PROT_CFG_RTS_TH_EN, enabled);
	rt2x00usb_register_write(rt2x00dev, OFDM_PROT_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, MM20_PROT_CFG, &reg);
	rt2x00_set_field32(&reg, MM20_PROT_CFG_RTS_TH_EN, enabled);
	rt2x00usb_register_write(rt2x00dev, MM20_PROT_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, MM40_PROT_CFG, &reg);
	rt2x00_set_field32(&reg, MM40_PROT_CFG_RTS_TH_EN, enabled);
	rt2x00usb_register_write(rt2x00dev, MM40_PROT_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, GF20_PROT_CFG, &reg);
	rt2x00_set_field32(&reg, GF20_PROT_CFG_RTS_TH_EN, enabled);
	rt2x00usb_register_write(rt2x00dev, GF20_PROT_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, GF40_PROT_CFG, &reg);
	rt2x00_set_field32(&reg, GF40_PROT_CFG_RTS_TH_EN, enabled);
	rt2x00usb_register_write(rt2x00dev, GF40_PROT_CFG, reg);

	return 0;
}

static int rt2800usb_conf_tx(struct ieee80211_hw *hw, u16 queue_idx,
			     const struct ieee80211_tx_queue_params *params)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct data_queue *queue;
	struct rt2x00_field32 field;
	int retval;
	u32 reg;
	u32 offset;

	/*
	 * First pass the configuration through rt2x00lib, that will
	 * update the queue settings and validate the input. After that
	 * we are free to update the registers based on the value
	 * in the queue parameter.
	 */
	retval = rt2x00mac_conf_tx(hw, queue_idx, params);
	if (retval)
		return retval;

	/*
	 * We only need to perform additional register initialization
	 * for WMM queues/
	 */
	if (queue_idx >= 4)
		return 0;

	queue = rt2x00queue_get_queue(rt2x00dev, queue_idx);

	/* Update WMM TXOP register */
	offset = WMM_TXOP0_CFG + (sizeof(u32) * (!!(queue_idx & 2)));
	field.bit_offset = (queue_idx & 1) * 16;
	field.bit_mask = 0xffff << field.bit_offset;

	rt2x00usb_register_read(rt2x00dev, offset, &reg);
	rt2x00_set_field32(&reg, field, queue->txop);
	rt2x00usb_register_write(rt2x00dev, offset, reg);

	/* Update WMM registers */
	field.bit_offset = queue_idx * 4;
	field.bit_mask = 0xf << field.bit_offset;

	rt2x00usb_register_read(rt2x00dev, WMM_AIFSN_CFG, &reg);
	rt2x00_set_field32(&reg, field, queue->aifs);
	rt2x00usb_register_write(rt2x00dev, WMM_AIFSN_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, WMM_CWMIN_CFG, &reg);
	rt2x00_set_field32(&reg, field, queue->cw_min);
	rt2x00usb_register_write(rt2x00dev, WMM_CWMIN_CFG, reg);

	rt2x00usb_register_read(rt2x00dev, WMM_CWMAX_CFG, &reg);
	rt2x00_set_field32(&reg, field, queue->cw_max);
	rt2x00usb_register_write(rt2x00dev, WMM_CWMAX_CFG, reg);

	/* Update EDCA registers */
	offset = EDCA_AC0_CFG + (sizeof(u32) * queue_idx);

	rt2x00usb_register_read(rt2x00dev, offset, &reg);
	rt2x00_set_field32(&reg, EDCA_AC0_CFG_TX_OP, queue->txop);
	rt2x00_set_field32(&reg, EDCA_AC0_CFG_AIFSN, queue->aifs);
	rt2x00_set_field32(&reg, EDCA_AC0_CFG_CWMIN, queue->cw_min);
	rt2x00_set_field32(&reg, EDCA_AC0_CFG_CWMAX, queue->cw_max);
	rt2x00usb_register_write(rt2x00dev, offset, reg);

	return 0;
}

static u64 rt2800usb_get_tsf(struct ieee80211_hw *hw)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	u64 tsf;
	u32 reg;

	rt2x00usb_register_read(rt2x00dev, TSF_TIMER_DW1, &reg);
	tsf = (u64) rt2x00_get_field32(reg, TSF_TIMER_DW1_HIGH_WORD) << 32;
	rt2x00usb_register_read(rt2x00dev, TSF_TIMER_DW0, &reg);
	tsf |= rt2x00_get_field32(reg, TSF_TIMER_DW0_LOW_WORD);

	return tsf;
}

static const struct ieee80211_ops rt2800usb_mac80211_ops = {
	.tx			= rt2x00mac_tx,
	.start			= rt2x00mac_start,
	.stop			= rt2x00mac_stop,
	.add_interface		= rt2x00mac_add_interface,
	.remove_interface	= rt2x00mac_remove_interface,
	.config			= rt2x00mac_config,
	.configure_filter	= rt2x00mac_configure_filter,
	.set_key		= rt2x00mac_set_key,
	.get_stats		= rt2x00mac_get_stats,
	.get_tkip_seq		= rt2800usb_get_tkip_seq,
	.set_rts_threshold	= rt2800usb_set_rts_threshold,
	.bss_info_changed	= rt2x00mac_bss_info_changed,
	.conf_tx		= rt2800usb_conf_tx,
	.get_tx_stats		= rt2x00mac_get_tx_stats,
	.get_tsf		= rt2800usb_get_tsf,
};

static const struct rt2x00lib_ops rt2800usb_rt2x00_ops = {
	.probe_hw		= rt2800usb_probe_hw,
	.get_firmware_name	= rt2800usb_get_firmware_name,
	.check_firmware		= rt2800usb_check_firmware,
	.load_firmware		= rt2800usb_load_firmware,
	.initialize		= rt2x00usb_initialize,
	.uninitialize		= rt2x00usb_uninitialize,
	.clear_entry		= rt2x00usb_clear_entry,
	.set_device_state	= rt2800usb_set_device_state,
	.rfkill_poll		= rt2800usb_rfkill_poll,
	.link_stats		= rt2800usb_link_stats,
	.reset_tuner		= rt2800usb_reset_tuner,
	.link_tuner		= rt2800usb_link_tuner,
	.write_tx_desc		= rt2800usb_write_tx_desc,
	.write_tx_data		= rt2x00usb_write_tx_data,
	.write_beacon		= rt2800usb_write_beacon,
	.get_tx_data_len	= rt2800usb_get_tx_data_len,
	.kick_tx_queue		= rt2800usb_kick_tx_queue,
	.kill_tx_queue		= rt2x00usb_kill_tx_queue,
	.fill_rxdone		= rt2800usb_fill_rxdone,
	.config_shared_key	= rt2800usb_config_shared_key,
	.config_pairwise_key	= rt2800usb_config_pairwise_key,
	.config_filter		= rt2800usb_config_filter,
	.config_intf		= rt2800usb_config_intf,
	.config_erp		= rt2800usb_config_erp,
	.config_ant		= rt2800usb_config_ant,
	.config			= rt2800usb_config,
};

static const struct data_queue_desc rt2800usb_queue_rx = {
	.entry_num		= RX_ENTRIES,
	.data_size		= AGGREGATION_SIZE,
	.desc_size		= RXD_DESC_SIZE + RXWI_DESC_SIZE,
	.priv_size		= sizeof(struct queue_entry_priv_usb),
};

static const struct data_queue_desc rt2800usb_queue_tx = {
	.entry_num		= TX_ENTRIES,
	.data_size		= AGGREGATION_SIZE,
	.desc_size		= TXINFO_DESC_SIZE + TXWI_DESC_SIZE,
	.priv_size		= sizeof(struct queue_entry_priv_usb),
