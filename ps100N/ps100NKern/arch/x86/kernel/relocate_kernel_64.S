led_master_disable();
		break;

	case PMIC_BKLIT_ENABLE:
		pmic_bklit_master_enable();
		break;

	case PMIC_BKLIT_DISABLE:
		pmic_bklit_master_disable();
		break;

	case PMIC_SET_BKLIT:
		if ((bklit_setting = kmalloc(sizeof(t_bklit_setting_param),
					     GFP_KERNEL)) == NULL) {
			return -ENOMEM;
		}
		if (copy_from_user(bklit_setting, (t_bklit_setting_param *) arg,
				   sizeof(t_bklit_setting_param))) {
			kfree(bklit_setting);
			return -EFAULT;
		}

		CHECK_ERROR_KFREE(pmic_bklit_set_mode(bklit_setting->channel,
						      bklit_setting->mode),
				  (kfree(bklit_setting)));

		CHECK_ERROR_KFREE(pmic_bklit_set_current(bklit_setting->channel,
							 bklit_setting->
							 current_level),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_set_dutycycle
				  (bklit_setting->channel,
				   bklit_setting->duty_cycle),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_set_cycle_time
				  (bklit_setting->cycle_time),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_set_boost_mode
				  (bklit_setting->en_dis),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_config_boost_mode
				  (bklit_setting->abms, bklit_setting->abr),
				  (kfree(bklit_setting)));
		if (bklit_setting->edge_slow != false) {
			CHECK_ERROR_KFREE(pmic_bklit_enable_edge_slow(),
					  (kfree(bklit_setting)));
		} else {
			CHECK_ERROR_KFREE(pmic_bklit_disable_edge_slow(),
					  (kfree(bklit_setting)));
		}

		kfree(bklit_setting);
		break;

	case PMIC_GET_BKLIT:
		if ((bklit_setting = kmalloc(sizeof(t_bklit_setting_param),
					     GFP_KERNEL)) == NULL) {
			return -ENOMEM;
		}

		if (copy_from_user(bklit_setting, (t_bklit_setting_param *) arg,
				   sizeof(t_bklit_setting_param))) {
			kfree(bklit_setting);
			return -EFAULT;
		}

		CHECK_ERROR_KFREE(pmic_bklit_get_current(bklit_setting->channel,
							 &bklit_setting->
							 current_level),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_get_cycle_time
				  (&bklit_setting->cycle_time),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_get_dutycycle
				  (bklit_setting->channel,
				   &bklit_setting->duty_cycle),
				  (kfree(bklit_setting)));
		bklit_setting->strobe = BACKLIGHT_STROBE_NONE;
		CHECK_ERROR_KFREE(pmic_bklit_get_mode(bklit_setting->channel,
						      &bklit_setting->mode),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_get_edge_slow
				  (&bklit_setting->edge_slow),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_get_boost_mode
				  (&bklit_setting->en_dis),
				  (kfree(bklit_setting)));
		CHECK_ERROR_KFREE(pmic_bklit_gets_boost_mode
				  (&bklit_setting->abms, &bklit_setting->abr),
				  (kfree(bklit_setting)));

		if (copy_to_user((t_bklit_setting_param *) arg, bklit_setting,
				 sizeof(t_bklit_setting_param))) {
			kfree(bklit_setting);
			return -EFAULT;
		}
		kfree(bklit_setting);
		break;

	case PMIC_RAMPUP_BKLIT:
		CHECK_ERROR(pmic_bklit_rampup((t_bklit_channel) arg));
		break;

	case PMIC_RAMPDOWN_BKLIT:
		CHECK_ERROR(pmic_bklit_rampdown((t_bklit_channel) arg));
		break;

	case PMIC_OFF_RAMPUP_BKLIT:
		CHECK_ERROR(pmic_bklit_off_rampup((t_bklit_channel) arg));
		break;

	case PMIC_OFF_RAMPDOWN_BKLIT:
		CHECK_ERROR(pmic_bklit_off_rampdown((t_bklit_channel) arg));
		break;

	case PMIC_TCLED_ENABLE:
		if ((tcled_setting = kmalloc(sizeof(t_tcled_enable_param),
					     GFP_KERNEL))
		    == NULL) {
			return -ENOMEM;
		}

		if (copy_from_user(tcled_setting, (t_tcled_enable_param *) arg,
				   sizeof(t_tcled_enable_param))) {
			kfree(tcled_setting);
			return -EFAULT;
		}
		CHECK_ERROR_KFREE(pmic_tcled_enable(tcled_setting->mode,
						    tcled_setting->bank),
				  (kfree(bklit_setting)));
		break;

	case PMIC_TCLED_DISABLE:
		CHECK_ERROR(pmic_tcled_disable((t_funlight_bank) arg));
		break;

	case PMIC_TCLED_PATTERN:
		if ((fun_param = kmalloc(sizeof(t_fun_param),
					 GFP_KERNEL)) == NULL) {
			return -ENOMEM;
		}
		if (copy_from_user(fun_param,
				   (t_fun_param *) arg, sizeof(t_fun_param))) {
			kfree(fun_param);
			return -EFAULT;
		}

		switch (fun_param->pattern) {
		case BLENDED_RAMPS_SLOW:
			CHECK_ERROR_KFREE(pmic_tcled_fun_blendedramps
					  (fun_param->bank, TC_SLOW),
					  (kfree(fun_param)));
			break;

		case BLENDED_RAMPS_FAST:
			CHECK_ERROR_KFREE(pmic_tcled_fun_blendedramps
					  (fun_param->bank, TC_FAST),
					  (kfree(fun_param)));
			break;

		case SAW_RAMPS_SLOW:
			CHECK_ERROR_KFREE(pmic_tcled_fun_sawramps
					  (fun_param->bank, TC_SLOW),
					  (kfree(fun_param)));
			break;

		case SAW_RAMPS_FAST:
			CHECK_ERROR_KFREE(pmic_tcled_fun_sawramps
					  (fun_param->bank, TC_FAST),
					  (kfree(fun_param)));
			break;

		case BLENDED_BOWTIE_SLOW:
			CHECK_ERROR_KFREE(pmic_tcled_fun_blendedbowtie
					  (fun_param->bank, TC_SLOW),
					  (kfree(fun_param)));
			break;

		case BLENDED_BOWTIE_FAST:
			CHECK_ERROR_KFREE(pmic_tcled_fun_blendedbowtie
					  (fun_param->bank, TC_FAST),
					  (kfree(fun_param)));
			break;

		case STROBE_SLOW:
			CHECK_ERROR_KFREE(pmic_tcled_fun_strobe
					  (fun_param->bank, fun_param->channel,
					   TC_STROBE_SLOW), (kfree(fun_param)));
			break;

		case STROBE_FAST:
			CHECK_ERROR_KFREE(pmic_tcled_fun_strobe
					  (fun_param->bank,
					   fun_param->