 pmic_light_resume(struct platform_device *pdev)
{
	suspend_flag = 0;
	while (swait > 0) {
		swait--;
		wake_up_interruptible(&suspendq);
	}

	return 0;
};

/*!
 * This function enables backlight & tcled.
 *
 * @return       This function returns PMIC_SUCCESS if successful.
 */
PMIC_STATUS pmic_bklit_tcled_master_enable(void)
{
	unsigned int reg_value = 0;
	unsigned int mask = 0;

	if (suspend_flag == 1) {
		return -EBUSY;
	}

	reg_value = BITFVAL(BIT_LEDEN, 1);
	mask = BITFMASK(BIT_LEDEN);
	CHECK_ERROR(pmic_write_reg(LREG_0, reg_value, mask));

	return PMIC_SUCCESS;
}

/*!
 * This function disables backlight & tcled.
 *
 * @return       This function returns PMIC_SUCCESS if successful
 */
PMIC_STATUS pmic_bklit_tcled_master_disable(void)
{
	unsigned int reg_value = 0;
	unsigned int mask = 0;

	if 