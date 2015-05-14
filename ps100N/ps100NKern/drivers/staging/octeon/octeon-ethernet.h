eed, so performance is maintained.
		 */
		if (phy->smart_speed == e1000_smart_speed_on) {
			ret_val = phy->ops.read_reg(hw,
						    IGP01E1000_PHY_PORT_CONFIG,
						    &data);
			if (ret_val)
				goto out;

			data |= IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = phy->ops.write_reg(hw,
						     IGP01E1000_PHY_PORT_CONFIG,
						     data);
			if (ret_val)
				goto out;
		} else if (phy->smart_speed == e1000_smart_speed_off) {
			ret_val = phy->ops.read_reg(hw,
						     IGP01E1000_PHY_PORT_CONFIG,
						     &data);
			if (ret_val)
				goto out;

			data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = phy->ops.write_reg(hw,
						     IGP01E1000_PHY_PORT_CONFIG,
						     data);
			if (ret_val)
				goto out;
		}
	} else if ((phy->autoneg_advertised == E1000_ALL_SPEED_DUPLEX) ||
		   (phy->autoneg_advertised == E1000_ALL_NOT_GIG) ||
		   (phy->autoneg_advertised == E1000_ALL_10_SPEED)) {
		data |= IGP02E1000_PM_D3_LPLU;
		ret_val = phy->ops.write_reg(hw, IGP02E1000_PHY_POWER_MGMT,
					      data);
		if (ret_val)
			goto out;

		/* When LPLU is enabled, we should disable SmartSpeed */
		ret_val = phy->ops.read_reg(hw, IGP01E1000_PHY_PORT_CONFIG,
					     &data);
		if (ret_val)
			goto out;

		data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = phy->ops.write_reg(hw, IGP01E1000_PHY_PORT_CONFIG,
					      data);
	}

out:
	return ret_val;
}

/**
 *  igb_check_downshift - Checks whether a downshift in speed occured
 *  @hw: pointer to the HW structure
 *
 *  Success returns 0, Failure returns 1
 *
 *  A downshift is detected by querying the PHY link health.
 **/
s32 igb_check_downshift(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 phy_data, offset, mask;

	switch (phy->type) {
	case e1000_phy_m88:
	case e1000_phy_gg82563:
		offset	= M88E1000_PHY_SPEC_STATUS;
		mask	= M88E1000_PSSR_DOWNSHIFT;
		break;
	case e1000_phy_igp_2:
	case e1000_phy_igp:
	case e1000_phy_igp_3:
		offset	= IGP01E1000_PHY_LINK_HEALTH;
		mask	= IGP01E1000_PLHR_SS_DOWNGRADE;
		break;
	default:
		/* speed downshift not supported */
		phy->speed_downgraded = false;
		ret_val = 0;
		goto out;
	}

	ret_val = phy->ops.read_reg(hw, offset, &phy_data);

	if (!ret_val)
		phy->speed_downgraded = (phy_data & mask) ? true : false;

out:
	return ret_val;
}

/**
 *  igb_check_polarity_m88 - Checks the polarity.
 *  @hw: pointer to the HW structure
 *
 *  Success returns 0, Failure returns -E1000_ERR_PHY (-2)
 *
 *  Polarity is determined based on the PHY specific status register.
 **/
static s32 igb_check_polarity_m88(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data;

	ret_val = phy->ops.read_reg(hw, M88E1000_PHY_SPEC_STATUS, &data);

	if (!ret_val)
		phy->cable_polarity = (data & M88E1000_PSSR_REV_POLARITY)
				      ? e1000_rev_polarity_reversed
				      : e1000_rev_polarity_normal;

	return ret_val;
}

/**
 *  igb_check_polarity_igp - Checks the polarity.
 *  @hw: pointer to the HW structure
 *
 *  Success returns 0, Failure returns -E1000_ERR_PHY (-2)
 *
 *  Polarity is determined based on the PHY port status register, and the
 *  current speed (since there is no polarity at 100Mbps).
 **/
static s32 igb_check_polarity_igp(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data, offset, mask;

	/*
	 * Polarity is determined based on the speed of
	 * our connection.
	 */
	ret_val = phy->ops.read_reg(hw, IGP01E1000_PHY_PORT_STATUS, &data);
	if (ret_val)
		goto out;

	if ((data & IGP01E1000_PSSR_SPEED_MASK) ==
	    IGP01E1000_PSSR_SPEED_1000MBPS) {
		offset	= IGP01E1000_PHY_PCS_INIT_REG;
		mask	= IGP01E1000_PHY_POLARITY_MASK;
	} else {
		/*
		 * This really only applies to 10Mbps since
		 * there is no polarity for 100Mbps (always 0).
		 */
		offset	= IGP01E1000_PHY_PORT_STATUS;
		mask	= IGP01E1000_PSSR_POLARITY_REVERSED;
	}

	ret_val = phy->ops.read_reg(hw, offset, &data);

	if (!ret_val)
		phy->cable_polarity = (data & mask)
				      ? e1000_rev_polarity_reversed
				      : e1000_rev_polarity_normal;

out:
	return ret_val;
}

/**
 *  igb_wait_autoneg - Wait for auto-neg compeletion
 *  @hw: pointer to the HW structure
 *
 *  Waits for auto-negotiation to complete or for the auto-negotiation time
 *  limit to expire, which ever happens first.
 **/
static s32 igb_wait_autoneg(struct e1000_hw *hw)
{
	s32 ret_val = 0;
	u16 i, phy_status;

	/* Break after autoneg completes or PHY_AUTO_NEG_LIMIT expires. */
	for (i = PHY_AUTO_NEG_LIMIT; i > 0; i--) {
		ret_val = hw->phy.ops.read_reg(hw, PHY_STATUS, &phy_status);
		if (ret_val)
			break;
		ret_val = hw->phy.ops.read_reg(hw, PHY_STATUS, &phy_status);
		if (ret_val)
			break;
		if (phy_status & MII_SR_AUTONEG_COMPLETE)
			break;
		msleep(100);
	}

	/*
	 * PHY_AUTO_NEG_TIME expiration doesn't guarantee auto-negotiation
	 * has completed.
	 */
	return ret_val;
}

/**
 *  igb_phy_has_link - Polls PHY for link
 *  @hw: pointer to the HW s