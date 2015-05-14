an be put into the lookup table to obtain the
		 * approximate cable length.
		 */
		cur_agc_index = (phy_data >> IGP02E1000_AGC_LENGTH_SHIFT) &
				IGP02E1000_AGC_LENGTH_MASK;

		/* Array index bound check. */
		if ((cur_agc_index >= IGP02E1000_CABLE_LENGTH_TABLE_SIZE) ||
		    (cur_agc_index == 0)) {
			ret_val = -E1000_ERR_PHY;
			goto out;
		}

		/* Remove min & max AGC values from calculation. */
		if (e1000_igp_2_cable_length_table[min_agc_index] >
		    e1000_igp_2_cable_length_table[cur_agc_index])
			min_agc_index = cur_agc_index;
		if (e1000_igp_2_cable_length_table[max_agc_index] <
		    e1000_igp_2_cable_length_table[cur_agc_index])
			max_agc_index = cur_agc_index;

		agc_value += e1000_igp_2_cable_length_table[cur_agc_index];
	}

	agc_value -= (e1000_igp_2_cable_length_table[min_agc_index] +
		      e1000_igp_2_cable_length_table[max_agc_index]);
	agc_value /= (IGP02E1000_PHY_CHANNEL_NUM - 2);

	/* Calculate cable length with the error range of +/- 10 meters. */
	phy->min_cable_length = ((agc_value - IGP02E1000_AGC_RANGE) > 0) ?
				 (agc_value - IGP02E1000_AGC_RANGE) : 0;
	phy->max_cable_length = agc_value + IGP02E1000_AGC_RANGE;

	phy->cable_length = (phy->min_cable_length + phy->max_cable_length) / 2;

out:
	return ret_val;
}

/**
 *  igb_get_phy_info_m88 - Retrieve PHY information
 *  @hw: pointer to the HW structure
 *
 *  Valid for only copper links.  Read the PHY status register (sticky read)
 *  to verify that link is up.  Read the PHY special control register to
 *  determine the polarity and 10base-T extended distance.  Read the PHY
 *  special status register to determine MDI/MDIx and current speed.  If
 *  speed is 1000, then determine cable length, local and remote receiver.
 **/
s32 igb_get_phy_info_m88(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32  ret_val;
	u16 phy_data;
	bool link;

	if (phy->media_type != e1000_media_type_copper) {
		hw_dbg("Phy info is only valid for copper media\n");
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

	ret_val = igb_phy_has_link(hw, 1, 0, &link);
	if (ret_val)
		goto out;

	if (!link) {
		hw_dbg("Phy info is only valid if link is up\n");
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

	ret_val = phy->ops.read_reg(hw, M88E1000_PHY_SPEC_CTRL, &phy_data);
	if (ret_val)
		goto out;

	phy->polarity_correction = (phy_data & M88E1000_PSCR_POLARITY_REVERSAL)
				   ? true : false;

	ret_val = igb_check_polarity_m88(hw);
	if (ret_val)
		goto out;

	ret_val = phy->ops.read_reg(hw, M88E1000_PHY_SPEC_STATUS, &phy_data);
	if (ret_val)
		goto out;

	phy->is_mdix = (phy_data & M88E1000_PSSR_MDIX) ? true : false;

	if ((phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_1000MBS) {
		ret_val = phy->ops.get_cable_length(hw);
		if (ret_val)
			goto out;

		ret_val = phy->ops.read_reg(hw, PHY_1000T_STATUS, &phy_data);
		if (ret_val)
			goto out;

		phy->local_rx = (phy_data & SR_1000T_LOCAL_RX_STATUS)
				? e1000_1000t_rx_status_ok
				: e1000_1000t_rx_status_not_ok;

		phy->remote_rx = (phy_data & SR_1000T_REMOTE_RX_STATUS)
				 ? e1000_1000t_rx_status_ok
				 : e1000_1000t_rx_status_not_ok;
	} else {
		/* Set values to "undefined" */
		phy->cable_length = E1000_CABLE_LENGTH_UNDEFINED;
		phy->local_rx = e1000_1000t_rx_status_undefined;
		phy->remote_rx = e1000_1000t_rx_status_undefined;
	}

out:
	return ret_val;
}

/**
 *  igb_get_phy_info_igp - Retrieve igp PHY information
 *  @hw: pointer to the HW structure
 *
 *  Read PHY status to determine if link is up.  If link is up, then
 *  set/determine 10base-T extended distance and polarity correction.  Read
 *  PHY port status to determine MDI/MDIx and speed.  Based on the speed,
 *  determine on the cable length, local and remote receiver.
 **/
s32 igb_get_phy_info_igp(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data;
	bool link;

	ret_val = igb_phy_has_link(hw, 1, 0, &link);
	if (ret_val)
		goto out;

	if (!link) {
		hw_dbg("Phy info is only valid if link is up\n");
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

	phy->polarity_correction = true;

	ret_val = igb_check_polarity_igp(hw);
	if (ret_val)
		goto out;

	ret_val = phy->ops.read_reg(hw, IGP01E1000_PHY_PORT_STATUS, &data);
	if (ret_val)
		goto out;

	phy->is_mdix = (data & IGP01E1000_PSSR_MDIX) ? true : false;

	if ((data & IGP01E1000_PSSR_SPEED_MASK) ==
	    IGP01E1000_PSSR_SPEED_1000MBPS) {
		ret_val = phy->ops.get_cable_length(hw);
		if (ret_val)
			goto out;

		ret_val = phy->ops.read_reg(hw, PHY_1000T_STATUS, &data);
		if (ret_val)
			goto out;

		phy->local_rx = (data & SR_1000T_LOCAL_RX_STATUS)
				? e1000_1000t_rx_status_ok
				: e1000_1000t_rx_status_not_ok;

		phy->remote_rx = (data & SR_1000T_REMOTE_RX_STATUS)
				 ? e1000_1000t_rx_status_ok
				 : e1000_1000t_rx_status_not_ok;
	} else {
		phy->cable_length = E1000_CABLE_LENGTH_UNDEFINED;
		phy->local_rx = e1000_1000t_rx_status_undefined;
		phy->remote_rx = e1000_1000t_rx_status_undefined;
	}

out:
	return ret_val;
}

/**
 *  igb_phy_sw_reset - PHY software reset
 *  @hw: pointer to the HW structure
 *
 *  Does a software reset of the PHY by reading the PHY control register and
 *  setting/write the control register reset bit to the PHY.
 **/
s32 igb_phy_sw_reset(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 phy_ctrl;

	ret_val = hw->phy.ops.read_reg(hw, PHY_CONTROL, &phy_ctrl);
	if (ret_val)
		goto out;

	phy_ctrl |= MII_CR_RESET;
	ret_val = hw->phy.ops.write_reg(hw, PHY_CONTROL, phy_ctrl);
	if (ret_val)
		goto out;

	udelay(1);

out:
	return ret_val;
}

/**
 *  igb_phy_hw_reset - PHY hardware reset
 *  @hw: pointer to the HW structure
 *
 *  Verify the reset block is not blocking us from resetting.  Acquire
 *  semaphore (if necessary) and read/set/write the device control reset
 *  bit in the PHY.  Wait the appropriate delay time for the device to
 *  reset and relase the semaphore (if necessary).
 **/
s32 igb_phy_hw_reset(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32  ret_val;
	u32 ctrl;

	ret_val = igb_check_reset_block(hw);
	if (ret_val) {
		ret_val = 0;
		goto out;
	}

	ret_val = phy->ops.acquire(hw);
	if (ret_val)
		goto out;

	ctrl = rd32(E1000_CTRL);
	wr32(E1000_CTRL, ctrl | E1000_CTRL_PHY_RST);
	wrfl();

	udelay(phy->reset_delay_us);

	wr32(E1000_CTRL, ctrl);
	wrfl();

	udelay(150);

	phy->ops.release(hw);

	ret_val = phy->ops.get_cfg_done(hw);

out:
	return ret_val;
}

/**
 *  igb_phy_init_script_igp3 - Inits the IGP3 PHY
 *  @hw: pointer to the HW structure
 *
 *  Initializes a Intel Gigabit PHY3 when an EEPROM is not present.
 **/
s32 igb_phy_init_script_igp3(struct e1000_hw *hw)
{
	hw_dbg("Running IGP 3 PHY init script\n");

	/* PHY init IGP 3 */
	/* Enable rise/fall, 10-mode work in class-A */
	hw->phy.ops.write_reg(hw, 0x2F5B, 0x9018);
	/* Remove all caps from Replica path filter */
	hw->phy.ops.write_reg(hw, 0x2F52, 0x0000);
	/* Bias trimming for ADC, AFE and Driver (Default) */
	hw->phy.ops.write_reg(hw, 0x2FB1, 0x8B24);
	/* Increase Hybrid poly bias */
	hw->phy.ops.write_reg(hw, 0x2FB2, 0xF8F0);
	/* Add 4% to TX amplitude in Giga mode */
	hw->phy.ops.write_reg(hw, 0x2010, 0x10B0);
	/* Disable trimming (TTT) */
	hw->phy.ops.write_reg(hw, 0x2011, 0x0000);
	/* Poly DC correction to 94.6% + 2% for all channels */
	hw->phy.ops.write_reg(hw, 0x20DD, 0x249A);
	/* ABS DC correction to 95.9% */
	hw->phy.ops.write_reg(hw, 0x20DE, 0x00D3);
	/* BG temp curve trim */
	hw->phy.ops.write_reg(hw, 0x28B4, 0x04CE);
	/* Increasing ADC OPAMP stage 1 currents to max */
	hw->phy.ops.write_reg(hw, 0x2F70, 0x29E4);
	/* Force 1000 ( required for enabling PHY regs configuration) */
	hw->phy.ops.write_reg(hw, 0x0000, 0x0140);
	/* Set upd_freq to 6 */
	hw->phy.ops.write_reg(hw, 0x1F30, 0x1606);
	/* Disable NPDFE */
	hw->phy.ops.write_reg(hw, 0x1F31, 0xB814);
	/* Disable adaptive fixed FFE (Default) */
	hw->phy.ops.write_reg(hw, 0x1F35, 0x002A);
	/* Enable FFE hysteresis */
	hw->phy.ops.write_reg(hw, 0x1F3E, 0x0067);
	/* 