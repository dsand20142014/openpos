1:
		mask = LEDR1RAMPDOWN;
		break;
	case TCLED_FUN_BANK2:
		mask = LEDR2RAMPDOWN;
		break;
	case TCLED_FUN_BANK3:
		mask = LEDR3RAMPDOWN;
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	switch (channel) {
	case TCLED_FUN_CHANNEL1:
		mask = mask;
		break;
	case TCLED_FUN_CHANNEL2:
		mask = mask * 2;
		break;
	case TCLED_FUN_CHANNEL3:
		mask = mask * 4;
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_read_reg(LREG_1, &value, mask));
	if (value) {
		*rampdown = true;
	} else {
		*rampdown = false;
	}
	return PMIC_SUCCESS;
}

/*!
 * This function enables a Tri-color channel triode mode.
 *
 * @param        bank         Tri-color LED bank
 * @param        channel      Tri-color LED channel.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_tcled_fun_triode_on(t_funlight_bank bank,
				     t_funlight_channel channel)
{
	unsigned int mask = 0;
	unsigned int value = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	switch (bank) {
	case TCLED_FUN_BANK1:
		mask = MASK_BK1_FL;
		value = ENABLE_BK1_FL;
		break;
	case TCLED_FUN_BANK2:
		mask = MASK_BK2_FL;
		value = ENABLE_BK2_FL;
		break;
	case TCLED_FUN_BANK3:
		mask = MASK_BK3_FL;
		value = ENABLE_BK2_FL;
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_write_reg(LREG_0, value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function disables a Tri-color LED channel triode mode.
 *
 * @param        bank         Tri-color LED bank
 * @param        channel      Tri-color LED channel.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_tcled_fun_triode_off(t_funlight_bank bank,
				      t_funlight_channel channel)
{
	unsigned int mask = 0;
	unsigned int value = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	switch (bank) {
	case TCLED_FUN_BANK1:
		mask = MASK_BK1_FL;
		break;
	case TCLED_FUN_BANK2:
		mask = MASK_BK2_FL;
		break;
	case TCLED_FUN_BANK3:
		mask = MASK_BK3_FL;
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_write_reg(LREG_0, value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function enables Tri-color LED edge slowing.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_tcled_enable_edge_slow(void)
{
	unsigned int mask = 0;
	unsigned int value = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	value = BITFVAL(BIT_SLEWLIMTC, 1);
	mask = BITFMASK(BIT_SLEWLIMTC);

	CHECK_ERROR(pmic_write_reg(LREG_1, value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function disables Tri-color LED edge slowing.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_tcled_disable_edge_slow(void)
{
	unsigned int mask = 0;
	unsigned int value = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	value = BITFVAL(BIT_SLEWLIMTC, 0);
	mask = BITFMASK(BIT_SLEWLIMTC);

	CHECK_ERROR(pmic_write_reg(LREG_1, value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function enables Tri-color LED half current mode.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_tcled_enable_half_current(void)
{
	unsigned int mask = 0;
	unsigned int value = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	value = BITFVAL(BIT_TC1HALF, 1);
	mask = BITFMASK(BIT_TC1HALF);

	CHECK_ERROR(pmic_write_reg(LREG_1, value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function disables Tri-color LED half current mode.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_tcled_disable_half_current(void)
{
	unsigned int mask = 0;
	unsigned int value = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	value = BITFVAL(BIT_TC1HALF, 0);
	mask = BITFMASK(BIT_TC1HALF);

	CHECK_ERROR(pmic_write_reg(LREG_1, value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function enables backlight or Tri-color LED audio modulation.
 *
 * @return       This function returns PMIC_NOT_SUPPORTED.
 */
PMIC_STATUS pmic_tcled_enable_audio_modulation(t_led_channel channel,
					       t_aud_path path,
					       t_aud_gain gain, bool lpf_bypass)
{
	return PMIC_NOT_SUPPORTED;
}

/*!
 * This function disables backlight or Tri-color LED audio modulation.
 *
 * @return       This function returns PMIC_NOT_SUPPORTED.
 */
PMIC_STATUS pmic_tcled_disable_audio_modulation(void)
{
	return PMIC_NOT_SUPPORTED;
}

/*!
 * This function enables the boost mode.
 * Only on mc13783 2.0 or higher
 *
 * @param       en_dis   Enable or disable the boost mode
 *
 * @return      This function returns 0 if successful.
 */
PMIC_STATUS pmic_bklit_set_boost_mode(bool en_dis)
{

	pmic_version_t mc13783_ver;
	unsigned int mask;
	unsigned int value;
	mc13783_ver = pmic_get_version();
	if (mc13783_ver.revision >= 20) {

		if (suspend_flag == 1) {
			return -EBUSY;
		}

		value = BITFVAL(BIT_BOOSTEN, en_dis);
		mask = BITFMASK(BIT_BOOSTEN);
		CHECK_ERROR(pmic_write_reg(LREG_0, value, mask));
		return PMIC_SUCCESS;
	} else {
		return PMIC_NOT_SUPPORTED;
	}
}

/*!
 * This function gets the boost mode.
 * Only on mc13783 2.0 or higher
 *
 * @param       en_dis   Enable or disable the boost mode
 *
 * @return      This function returns 0 if successful.
 */
PMIC_STATUS pmic_bklit_get_boost_mode(bool * en_dis)
{
	pmic_version_t mc13783_ver;
	unsigned int mask;
	unsigned int value;
	mc13783_ver = pmic_get_version();
	if (mc13783_ver.revision >= 20) {

		if (suspend_flag == 1) {
			return -EBUSY;
		}
		mask = BITFMASK(BIT_BOOSTEN);
		CHECK_ERROR(pmic_read_reg(LREG_0, &value, mask));
		*en_dis = BITFEXT(value, BIT_BOOSTEN);
		return PMIC_SUCCESS;
	} else {
		return PMIC_NOT_SUPPORTED;
	}
}

/*!
 * This function sets boost mode configuration
 * Only on mc13783 2.0 or higher
 *
 * @param    abms      Define adaptive boost mode selection
 * @param    abr       Define adaptive boost reference
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_bklit_config_boost_mode(unsigned int abms, unsigned int abr)
{
	unsigned int conf_boost = 0;
	unsigned int mask;
	unsigned int value;
	pmic_version_t mc13783_ver;

	mc13783_ver = pmic_get_version();
	if (mc13783_ver.revision >= 20) {
		if (suspend_flag == 1) {
			return -EBUSY;
		}

		if (abms > MAX_BOOST_ABMS) {
			return PMIC_PARAMETER_ERROR;
		}

		if (abr > MAX_BOOST_ABR) {
			return PMIC_PARAMETER_ERROR;
		}

		conf_boost = abms | (abr << 3);

		value = BITFVAL(BITS_BOOST, conf_boost);
		mask = BITFMASK(BITS_BOOST);
		CHECK_ERROR(pmic_write_reg(LREG_0, value, mask));

		return PMIC_SUCCESS;
	} else {
		return PMIC_NOT_SUPPORTED;
	}
}

/*!
 * This function gets boost mode configuration
 * Only on mc13783 2.0 or higher
 *
 * @param    abms      Define adaptive boost mode selection
 * @param    abr       Define adaptive boost reference
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_bklit_gets_boost_mode(unsigned int *abms, unsigned int *abr)
{
	unsigned int mask;
	unsigned int value;
	pmic_version_t mc13783_ver;
	mc13783_ver = pmic_get_version();
	if (mc13783_ver.revision >= 20) {
		if (suspend_flag == 1) {
			return -EBUSY;
		}

		mask = BITFMASK(BITS_BOOST_ABMS);
		CHECK_ERROR(pmic_read_reg(LREG_0, &value, mask));
		*abms = BITFEXT(value, BITS_BOOST_ABMS);

		mask = BITFMASK(BITS_BOOST_ABR);
		CHECK_ERROR(pmic_read_reg(LREG_0, &value, mask));
		*abr = BITFEXT(value, BITS_BOOST_ABR);
		return PMIC_SUCCESS;
	} else {
		return PMIC_NOT_SUPPORTED;
	}
}

/*!
 * This function implements IOCTL controls on a PMIC Light device.
 *

 * @param        inode       pointer on the node
 * @param        file        pointer on the file
 * @param        cmd         the command
 * @param        arg         the parameter
 * @return       This function returns 0 if successful.
 */
static int pmic_light_ioctl(struct inode *inode, struct file *file,
			    unsigned int cmd, unsigned long arg)
{
	t_bklit_setting_param *bklit_setting = NULL;
	t_tcled_enable_param *tcled_setting;
	t_fun_param *fun_param;
	t_tcled_ind_param *tcled_ind;

	if (_IOC_TYPE(cmd) != 'p')
		return -ENOTTY;

	switch (cmd) {
	case PMIC_BKLIT_TCLED_ENABLE:
		pmic_bklit_tcled_master_enable();
		break;

	case PMIC_BKLIT_TCLED_DISABLE:
		pmic_bklit_tcled_master_disable();
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
					   fun_param->channel, TC_STROBE_SLOW),
					  (kfree(fun_param)));
			break;

		case CHASING_LIGHT_RGB_SLOW:
			CHECK_ERROR_KFREE(pmic_tcled_fun_chasinglightspattern
					  (fun_param->bank, PMIC_RGB, TC_SLOW),
					  (kfree(fun_param)));
			break;

		case CHASING_LIGHT_RGB_FAST:
			CHECK_ERROR_KFREE(pmic_tcled_fun_chasinglightspattern
					  (fun_param->bank, PMIC_RGB, TC_FAST),
					  (kfree(fun_param)));
			break;

		case CHASING_LIGHT_BGR_SLOW:
			CHECK_ERROR_KFREE(pmic_tcled_fun_chasinglightspattern
					  (fun_param->bank, BGR, TC_SLOW),
					  (kfree(fun_param)));
			break;

		case CHASING_LIGHT_BGR_FAST:
			CHECK_ERROR_KFREE(pmic_tcled_fun_chasinglightspattern
					  (fun_param->bank, BGR, TC_FAST),
					  (kfree(fun_param)));
			break;
		}

		kfree(fun_param);
		break;

	case PMIC_SET_TCLED:
		if ((tcled_ind = kmalloc(sizeof(t_tcled_ind_param), GFP_KERNEL))
		    == NULL) {
			return -ENOMEM;
		}

		if (copy_from_user(tcled_ind, (t_tcled_ind_param *) arg,
				   sizeof(t_tcled_ind_param))) {
			kfree(tcled_ind);
			return -EFAULT;
		}
		CHECK_ERROR_KFREE(pmic_tcled_ind_set_current(tcled_ind->channel,
							     tcled_ind->level,
							     tcled_ind->bank),
				  (kfree(tcled_ind)));
		CHECK_ERROR_KFREE(pmic_tcled_ind_set_blink_pattern
				  (tcled_ind->channel, tcled_ind->pattern,
				   tcled_ind->skip, tcled_ind->bank),
				  (kfree(tcled_ind)));
		CHECK_ERROR_KFREE(pmic_tcled_fun_rampup
				  (tcled_ind->bank, tcled_ind->channel,
				   tcled_ind->rampup), (kfree(tcled_ind)));
		CHECK_ERROR_KFREE(pmic_tcled_fun_rampdown
				  (tcled_ind->bank, tcled_ind->channel,
				   tcled_ind->rampdown), (kfree(tcled_ind)));
		if (tcled_ind->half_current) {
			CHECK_ERROR_KFREE(pmic_tcled_enable_half_current(),
					  (kfree(tcled_ind)));
		} else {
			CHECK_ERROR_KFREE(pmic_tcled_disable_half_current(),
					  (kfree(tcled_ind)));
		}

		kfree(tcled_ind);
		break;

	case PMIC_GET_TCLED:
		if ((tcled_ind = kmalloc(sizeof(t_tcled_ind_param), GFP_KERNEL))
		    == NULL) {
			return -ENOMEM;
		}
		if (copy_from_user(tcled_ind, (t_tcled_ind_param *) arg,
				   sizeof(t_tcled_ind_param))) {
			kfree(tcled_ind);
			return -EFAULT;
		}
		CHECK_ERROR_KFREE(pmic_tcled_ind_get_current(tcled_ind->channel,
							     &tcled_ind->level,
							     tcled_ind->bank),
				  (kfree(tcled_ind)));
		CHECK_ERROR_KFREE(pmic_tcled_ind_get_blink_pattern
				  (tcled_ind->channel, &tcled_ind->pattern,
				   &tcled_ind->skip, tcled_ind->bank),
				  (kfree(tcled_ind)));
		CHECK_ERROR_KFREE(pmic_tcled_get_fun_rampup
				  (tcled_ind->bank, tcled_ind->channel,
				   &tcled_ind->rampup), (kfree(tcled_ind)));
		CHECK_ERROR_KFREE(pmic_tcled_get_fun_rampdown
				  (tcled_ind->bank, tcled_ind->channel,
				   &tcled_ind->rampdown), (kfree(tcled_ind)));
		if (copy_to_user
		    ((t_tcled_ind_param *) arg, tcled_ind,
		     sizeof(t_tcled_ind_param))) {
			return -EFAULT;
		}
		kfree(tcled_ind);

		break;

	default:
		return -EINVAL;
	}
	return 0;
}

/*!
 * This function initialize Light registers of mc13783 with 0.
 *
 * @return       This function returns 0 if successful.
 */
int pmic_light_init_reg(void)
{
	CHECK_ERROR(pmic_write_reg(LREG_0, 0, PMIC_ALL_BITS));
	CHECK_ERROR(pmic_write_reg(LREG_1, 0, PMIC_ALL_BITS));
	CHECK_ERROR(pmic_write_reg(LREG_2, 0, PMIC_ALL_BITS));
	CHECK_ERROR(pmic_write_reg(LREG_3, 0, PMIC_ALL_BITS));
	CHECK_ERROR(pmic_write_reg(LREG_4, 0, PMIC_ALL_BITS));
	CHECK_ERROR(pmic_write_reg(LREG_5, 0, PMIC_ALL_BITS));
	return 0;
}

/*!
 * This function implements the open method on a mc13783 light device.
 *
 * @param        inode       pointer on the node
 * @param        file        pointer on the file
 * @return       This function returns 0.
 */
static int pmic_light_open(struct inode *inode, struct file *file)
{
	while (suspend_flag == 1) {
		swait++;
		/* Block if the device is suspended */
		if (wait_event_interruptible(suspendq, (suspend_flag == 0))) {
			return -ERESTARTSYS;
		}
	}
	return 0;
}

/*!
 * This function implements the release method on a mc13783 light device.
 *
 * @param        inode       pointer on the node
 * @param        file        pointer on the file
 * @return       This function returns 0.
 */
static int pmic_light_release(struct inode *inode, struct file *file)
{
	while (suspend_flag == 1) {
		swait++;
		/* Block if the device is suspended */
		if (wait_event_interruptible(suspendq, (suspend_flag == 0))) {
			return -ERESTARTSYS;
		}
	}
	return 0;
}

static struct file_operations pmic_light_fops = {
	.owner = THIS_MODULE,
	.ioctl = pmic_light_ioctl,
	.open = pmic_light_open,
	.release = pmic_light_release,
};

static int pmic_light_remove(struct platform_device *pdev)
{
	device_destroy(pmic_light_class, MKDEV(pmic_light_major, 0));
	class_destroy(pmic_light_class);
	unregister_chrdev(pmic_light_major, "pmic_light");
	return 0;
}

static int pmic_light_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *temp_class;

	while (suspend_flag == 1) {
		swait++;
		/* Block if the device is suspended */
		if (wait_event_interruptible(suspendq, (suspend_flag == 0))) {
			return -ERESTARTSYS;
		}
	}
	pmic_light_major = register_chrdev(0, "pmic_light", &pmic_light_fops);

	if (pmic_light_major < 0) {
		printk(KERN_ERR "Unable to get a major for pmic_light\n");
		return pmic_light_major;
	}
	init_waitqueue_head(&suspendq);

	pmic_light_class = class_create(THIS_MODULE, "pmic_light");
	if (IS_ERR(pmic_light_class)) {
		printk(KERN_ERR "Error creating pmic_light class.\n");
		ret = PTR_ERR(pmic_light_class);
		goto err_out1;
	}

	temp_class = device_create(pmic_light_class, NULL,
				   MKDEV(pmic_light_major, 0), NULL,
				   "pmic_light");
	if (IS_ERR(temp_class)) {
		printk(KERN_ERR "Error creating pmic_light class device.\n");
		ret = PTR_ERR(temp_class);
		goto err_out2;
	}

	ret = pmic_light_init_reg();
	if (ret != PMIC_SUCCESS) {
		goto err_out3;
	}

	printk(KERN_INFO "PMIC Light successfully loaded\n");
	return ret;

      err_out3:
	device_destroy(pmic_light_class, MKDEV(pmic_light_major, 0));
      err_out2:
	class_destroy(pmic_light_class);
      err_out1:
	unregister_chrdev(pmic_light_major, "pmic_light");
	return ret;
}

static struct platform_driver pmic_light_driver_ldm = {
	.driver = {
		   .name = "pmic_light",
		   },
	.suspend = pmic_light_suspend,
	.resume = pmic_light_resume,
	.probe = pmic_light_probe,
	.remove = pmic_light_remove,
};

/*
 * Initialization and Exit
 */

static int __init pmic_light_init(void)
{
	pr_debug("PMIC Light driver loading...\n");
	return platform_driver_register(&pmic_light_driver_ldm);
}
static void __exit pmic_light_exit(void)
{
	platform_driver_unregister(&pmic_light_driver_ldm);
	pr_debug("PMIC Light driver successfully unloaded\n");
}

/*
 * Module entry points
 */

subsys_initcall(pmic_light_init);
module_exit(pmic_light_exit);

MODULE_DESCRIPTION("PMIC_LIGHT");
MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              