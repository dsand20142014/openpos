 BITFMASK(MC13783_REGGEN_VRFBG);
		break;
	case REGU_VRF1:
		reg_val = BITFVAL(MC13783_REGGEN_VRF1, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_VRF1);
		break;
	case REGU_VRF2:
		reg_val = BITFVAL(MC13783_REGGEN_VRF2, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_VRF2);
		break;
	case REGU_VMMC1:
		reg_val = BITFVAL(MC13783_REGGEN_VMMC1, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_VMMC1);
		break;
	case REGU_VMMC2:
		reg_val = BITFVAL(MC13783_REGGEN_VMMC2, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_VMMC2);
		break;
	case REGU_GPO1:
		reg_val = BITFVAL(MC13783_REGGEN_GPO1, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_GPO1);
		break;
	case REGU_GPO2:
		reg_val = BITFVAL(MC13783_REGGEN_GPO2, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_GPO2);
		break;
	case REGU_GPO3:
		reg_val = BITFVAL(MC13783_REGGEN_GPO3, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_GPO3);
		break;
	case REGU_GPO4:
		reg_val = BITFVAL(MC13783_REGGEN_GPO4, en_dis);
		reg_mask = BITFMASK(MC13783_REGGEN_GPO4);
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_write_reg(REG_REGEN_ASSIGNMENT, reg_val, reg_mask));

	return PMIC_SUCCESS;
}

/*!
 * This function gets the Regen assignment for all regulator
 *
 * @param        regulator      type of regulator
 * @param        en_dis         return value, if true :
 *                              the regulator is enabled by regen.
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_power_get_regen_assig(t_pmic_regulator regulator,
				       bool * en_dis)
{
	unsigned int reg_val = 0, reg_mask = 0;

	switch (regulator) {
	case REGU_VAUDIO:
		reg_mask = BITFMASK(MC13783_REGGEN_VAUDIO);
		break;
	case REGU_VIOHI:
		reg_mask = BITFMASK(MC13783_REGGEN_VIOHI);
		break;
	case REGU_VIOLO:
		reg_mask = BITFMASK(MC13783_REGGEN_VIOLO);
		break;
	case REGU_VDIG:
		reg_mask = BITFMASK(MC13783_REGGEN_VDIG);
		break;
	case REGU_VGEN:
		reg_mask = BITFMASK(MC13783_REGGEN_VGEN);
		break;
	case REGU_VRFDIG:
		reg_mask = BITFMASK(MC13783_REGGEN_VRFDIG);
		break;
	case REGU_VRFREF:
		reg_mask = BITFMASK(MC13783_REGGEN_VRFREF);
		break;
	case REGU_VRFCP:
		reg_mask = BITFMASK(MC13783_REGGEN_VRFCP);
		break;
	case REGU_VCAM:
		reg_mask = BITFMASK(MC13783_REGGEN_VCAM);
		break;
	case REGU_VRFBG:
		reg_mask = BITFMASK(MC13783_REGGEN_VRFBG);
		break;
	case REGU_VRF1:
		reg_mask = BITFMASK(MC13783_REGGEN_VRF1);
		break;
	case REGU_VRF2:
		reg_mask = BITFMASK(MC13783_REGGEN_VRF2);
		break;
	case REGU_VMMC1:
		reg_mask = BITFMASK(MC13783_REGGEN_VMMC1);
		break;
	case REGU_VMMC2:
		reg_mask = BITFMASK(MC13783_REGGEN_VMMC2);
		break;
	case REGU_GPO1:
		reg_mask = BITFMASK(MC13783_REGGEN_GPO1);
		break;
	case REGU_GPO2:
		reg_mask = BITFMASK(MC13783_REGGEN_GPO2);
		break;
	case REGU_GPO3:
		reg_mask = BITFMASK(MC13783_REGGEN_GPO3);
		break;
	case REGU_GPO4:
		reg_mask = BITFMASK(MC13783_REGGEN_GPO4);
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_read_reg(REG_REGEN_ASSIGNMENT, &reg_val, reg_mask));

	switch (regulator) {
	case REGU_VAUDIO:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VAUDIO);
		break;
	case REGU_VIOHI:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VIOHI);
		break;
	case REGU_VIOLO:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VIOLO);
		break;
	case REGU_VDIG:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VDIG);
		break;
	case REGU_VGEN:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VGEN);
		break;
	case REGU_VRFDIG:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VRFDIG);
		break;
	case REGU_VRFREF:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VRFREF);
		break;
	case REGU_VRFCP:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VRFCP);
		break;
	case REGU_VCAM:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VCAM);
		break;
	case REGU_VRFBG:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VRFBG);
		break;
	case REGU_VRF1:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VRF1);
		break;
	case REGU_VRF2:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VRF2);
		break;
	case REGU_VMMC1:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VMMC1);
		break;
	case REGU_VMMC2:
		*en_dis = BITFEXT(reg_val, MC13783_REGGEN_VMMC2