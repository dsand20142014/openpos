SW2A_MODE);
		}
		break;
	case SW_SW2B:
		if (stby) {
			l_mode =
			    BITFEXT(reg_val, MC13783_SWCTRL_SW2B_STBY_MODE);
		} else {
			l_mode = BITFEXT(reg_val, MC13783_SWCTRL_SW2B_MODE);
		}
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	if (l_mode == MC13783_SWCTRL_SW_MODE_SYNC_RECT_EN) {
		*mode = SYNC_RECT;
	} else if (l_mode == MC13783_SWCTRL_SW_MODE_PULSE_NO_SKIP_EN) {
		*mode = NO_PULSE_SKIP;
	} else if (l_mode == MC13783_SWCTRL_SW_MODE_PULSE_SKIP_EN) {
		*mode = PULSE_SKIP;
	} else if (l_mode == MC13783_SWCTRL_SW_MODE_LOW_POWER_EN) {
		*mode = LOW_POWER;
	} else {
		return PMIC_PARAMETER_ERROR;
	}

	return PMIC_SUCCESS;
}

/*!
 * This function sets the switch dvs speed
 *
 * @param        regulator    The regulator to be configured.
 * @param        speed	      The dvs speed.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_power_switcher_set_dvs_speed(t_pmic_regulator regulator,
					      t_switcher_dvs_speed speed)
{
	unsigned int reg_val = 0, reg_mask = 0;
	unsigned int reg;
	if (speed > 3 || speed < 0) {
		return PMIC_PARAMETER_ERROR;
	}

	switch (regulator) {
	case SW_SW1A:
		reg_val = BITFVAL(MC13783_SWCTRL_SW1A_DVS_SPEED, speed);
		reg_mask = BITFMASK(MC13783_SWCTRL_SW1A_DVS_SPEED);
		reg = REG_SWITCHERS_4;
		break;
	case SW_SW1B:
		reg_val = BITFVAL(MC13783_SWCTRL_SW2B_DVS_SPEED, speed);
		reg_mask = BITFMASK(MC13783_SWCTRL_SW1B_DVS_SPEED);
		reg = REG_SWITCHERS_4;
		break;
	case SW_SW2A:
		reg_val = BITFVAL(MC13783_SWCTRL_SW2A_DVS_SPEED, speed);
		reg_mask = BITFMASK(MC13783_SWCTRL_SW2A_DVS_SPEED);
		reg = REG_SWITCHERS_5;
		break;
	case SW_SW2B:
		reg_val = BITFVAL(MC13783_SWCTRL_SW2B_DVS_SPEED, speed);
		reg_mask = BITFMASK(MC13783_SWCTRL_SW2B_DVS_SPEED);
		reg = REG_SWITCHERS_5;
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_write_reg(reg, reg_val, reg_mask));

	return PMIC_SUCCESS;
}

/*!
 * This function gets the switch dvs speed
 *
 * @param        regulator    The regulator to be handled.
 * @param        speed        The dvs speed.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_power_switcher_get_dvs_speed(t_pmic_regulator regulator,
					      t_switcher_dvs_speed * speed)
{
	unsigned int reg_val = 0, reg_mask = 0;
	unsigned int reg;

	switch (regulator) {
	case SW_SW1A:
		reg_mask = BITFMASK(MC13783_SWCTRL_SW1A_DVS_SPEED);
		reg = REG_SWITCHERS_4;
		break;
	case SW_SW1B:
		reg_mask = BITFMASK(MC13783_SWCTRL_SW1B_DVS_SPEED);
		reg = REG_SWITCHERS_4;
		b