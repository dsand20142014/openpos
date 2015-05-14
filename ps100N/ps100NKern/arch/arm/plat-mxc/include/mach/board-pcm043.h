	l_mode = BITFEXT(reg_val, MC13783_REGCTRL_VRF1_MODE);
		l_stby = BITFEXT(reg_val, MC13783_REGCTRL_VRF1_STBY);
		break;
	case REGU_VRF2:
		l_mode = BITFEXT(reg_val, MC13783_REGCTRL_VRF2_MODE);
		l_stby = BITFEXT(reg_val, MC13783_REGCTRL_VRF2_STBY);
		break;
	case REGU_VMMC1:
		l_mode = BITFEXT(reg_val, MC13783_REGCTRL_VMMC1_MODE);
		l_stby = BITFEXT(reg_val, MC13783_REGCTRL_VMMC1_STBY);
		break;
	case REGU_VMMC2:
		l_mode = BITFEXT(reg_val, MC13783_REGCTRL_VMMC2_MODE);
		l_stby = BITFEXT(reg_val, MC13783_REGCTRL_VMMC2_STBY);
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	if ((l_mode == MC13783_REGTRL_LP_MODE_DISABLE) &&
	    (l_stby == MC13783_REGTRL_STBY_MODE_DISABLE)) {
		*lp_mode = LOW_POWER_DISABLED;
	} else if ((l_mode == MC13783_REGTRL_LP_MODE_DISABLE) &&
		   (l_stby == MC13783_REGTRL_STBY_MODE_ENABLE)) {
		*lp_mode = LOW_POWER_CTRL_BY_PIN;
	} else if ((l_mode == MC13783_REGTRL_LP_MODE_ENABLE) &&
		   (l_stby == MC13783_REGTRL_STBY_MODE_DISABLE)) {
		*lp_mode = LOW_POWER_EN;
	} else {
		*lp_mode = LOW_POWER_A