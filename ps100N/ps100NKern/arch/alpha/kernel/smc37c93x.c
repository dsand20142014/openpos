ic_read_reg(REG_POWER_CONTROL_2, &reg_val, reg_mask));
	*en = BITFEXT(reg_val, MC13783_AUTO_RESTART);
	return PMIC_SUCCESS;
}

/*!
 * This function configures a system reset on a button.
 *
 * @param       bt         type of button.
 * @param       sys_rst    if true, enable the system reset on this button
 * @param       deb_time   sets the debounce time on this button pin
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_power_set_conf_button(t_button bt, bool sys_rst, int deb_time)
{
	int max_val = 0;
	unsigned int reg_val = 0, reg_mask = 0;

	max_val = (1 << MC13783_DEB_BT_ON1B_WID) - 1;
	if (deb_time > max_val) {
		return PMIC_PARAMETER_ERROR;
	}

	switch (bt) {
	case BT_ON1B:
		reg_val = BITFVAL(MC13783_EN_BT_ON1B, sys_rst) |
		    BITFVAL(MC13783_DEB_BT_ON1B, deb_time);
		reg_mask = BITFMASK(MC13783_EN_BT_ON1B) |
		    BITFMASK(MC13783_DEB_BT_ON1B);
		break;
	case BT_ON2B:
		reg_val = BITFVAL(MC13783_EN_BT_ON2B, sys_rst) |
		    BITFVAL(MC13783_DEB_BT_ON2B, deb_time);
		reg_mask = BITFMASK(MC13783_EN_BT_ON2B) |
		    BITFMASK(MC13783_DEB_BT_ON2B);
		break;
	case BT_ON3B:
		reg_val = BITFVAL(MC13783_EN_BT_ON3B, sys_rst) |
		    BITFVAL(MC13783_DEB_BT_ON3B, deb_time);
		reg_mask = BITFMASK(MC13783_EN_BT_ON3B) |
		    BITFMASK(MC13783_DEB_BT_ON3B);
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_write_reg(REG_POWER_CONTROL_2, reg_val, reg_mask));

	return PMIC_SUCCESS;
}

/*!
 * This function gets configuration of a button.
 *
 * @param       bt         type of button.
 * @param       sys_rst    if true, the system reset is enabled on this button
 * @param       deb_time   gets the debounce time on this button pin
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_power_get_conf_button(t_button bt,
				       bool * sys_rst, int *deb_time)
{
	unsigned int reg_val = 0, reg_mask = 0;

	switch (bt) {
	case BT_ON1B:
		reg_mask = BITFMASK(MC13783_EN_BT_ON1B) |
		    BITFMASK(MC13783_DEB_BT_ON1B);
		break;
	case BT_ON2B:
		reg_mask = BITFMASK(MC13783_EN_BT_ON2B) |
		    BITFMASK(MC13783_DEB_BT_ON2B);
		break;
	case BT_ON3B:
		reg_mask = BITFMASK(MC13783_EN_BT_ON3B) |
		    BITFMASK(MC13783_DEB_BT_ON3B);
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}

	CHECK_ERROR(pmic_read_reg(REG_POWER_CONTROL_2, &reg_val, reg_mask));

	switch (bt) {
	case BT_ON1B:
		*sys_rst = BITFEXT(reg_val, MC13783_EN_BT_ON1B);
		*deb_time = BITFEXT(reg_val, MC13783_DEB_BT_ON1B);
		break;
	case BT_ON2B:
		*sys_rst = BITFEXT(reg_val, MC13783_EN_BT_ON2B);
		*deb_time = BITFEXT(reg_val, MC13783_DEB_BT_ON2B);
		break;
	case BT_ON3B:
		*sys_rst = BITFEXT(reg_val, MC13783_EN_BT_ON3B);
		*deb_time = BITFEXT(reg_val, MC13783_DEB_BT_ON3B);
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}
	return PMIC_SUCCESS;
}

/*!
 * This function is used to un/subscribe on power event IT.
 *
 * @param        event          type of event.
 * @param        callback       event callback function.
 * @param        sub            define if Un/subscribe event.
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_power_event(t_pwr_int event, void *callback, bool sub)
{
	pmic_event_callback_t power_callback;
	type_event power_event;

	power_callback.func = callback;
	power_callback.param = NULL;
	switch (event) {
	case PWR_IT_BPONI:
		power_event = EVENT_BPONI;
		break;
	case PWR_IT_LOBATLI:
		power_event = EVENT_LOBATLI;
		break;
	case PWR_IT_LOBATHI:
		power_event = EVENT_LOBATHI;
		break;
	case PWR_IT_ONOFD1I:
		power_event = EVENT_ONOFD1I;
		break;
	case PWR_IT_ONOFD2I:
		power_event = EVENT_ONOFD2I;
		break;
	case PWR_IT_ONOFD3I:
		power_event = EVENT_ONOFD3I;
		break;
	case PWR_IT_SYSRSTI:
		power_event = EVENT_SYSRSTI;
		break;
	case PWR_IT_PWRRDYI:
		power_event = EVENT_PWRRDYI;
		break;
	case PWR_IT_PCI:
		power_event = EVENT_PCI;
		break;
	case PWR_IT_WARMI:
		power_event = EVENT_WARMI;
		break;
	default:
		return PMIC_PARAMETER_ERROR;
	}
	if (sub == true) {
		CHECK_ERROR(pmic_event_subscribe(power_event, power_callback));
	} else {
		CHECK_ERROR(pmic_event_unsubscribe
			    (power_event, power_callback));
	}
	return PMIC_SUCCESS;
}

/*!
 * This function is used to subscribe on power event IT.
 *
 * @param        event          type of event.
 * @param        callback       event callback function.
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_power_event_sub(t_pwr_int event, void *callback)
{
	return pmic_power_event(event, callback, true);
}

/*!
 * This function is used to un subscribe on power event IT.
 *
 * @param        event          type of event.
 * @param        callback       event callback function.
 *
 * @return       This function returns 0 if successful.
 */
PMIC_STATUS pmic_power_event_unsub(t_pwr_int event, void *callback)
{
	return pmic_power_event(event, callback, false);
}

void pmic_power_key_callback(void)
{
#ifdef CONFIG_MXC_HWEVENT
	/*read the power key is pressed or up */
	t_sensor_bits sense;
	struct mxc_hw_event event = { HWE_POWER_KEY, 0 };

	pmic_get_sensors(&sense);
	if (sense.sense_onofd1s) {
		pr_debug("PMIC Power key up\n");
		event.args = PWRK_UNPRESS;
	} else {
		pr_debug("PMIC Power key pressed\n");
		event.args = PWRK_PRESS;
	}
	/* send hw event */
	hw_event_send(HWE_DEF_PRIORITY, &event);
#endif
}

static irqreturn_t power_key_int(int irq, void *dev_id)
{
	pr_info(KERN_INFO "on-off key pressed\n");

	return 0;
}

extern void gpio_power_key_active(void);

/*
 * Init and Exit
 */

static int pmic_power_probe(struct platform_device *pdev)
{
	int irq, ret;
	struct pmic_platform_data *ppd;

	/* configure on/off button */
	gpio_power_key_active();

	ppd = pdev->dev.platform_data;
	if (ppd)
		irq = ppd->power_key_irq;
	else
		goto done;

	if (irq == 0) {
		pr_info(KERN_INFO "PMIC Power has no platform data\n");
		goto done;
	}
	set_irq_type(irq, IRQF_TRIGGER_RISING);

	ret = request_irq(irq, power_key_int, 0, "power_key", 0);
	if (ret)
		pr_info(KERN_ERR "register on-off key interrupt failed\n");

	set_irq_wake(irq, 1);

      done:
	pr_info(KERN_INFO "PMIC Power successfully probed\n");
	return 0;
}

static struct platform_driver pmic_power_driver_ldm = {
	.driver = {
		   .name = "pmic_power",
		   },
	.suspend = pmic_power_suspend,
	.resume = pmic_power_resume,
	.probe = pmic_power_probe,
	.remove = NULL,
};

static int