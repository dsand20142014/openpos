	reg_val &= 0xff00;
	__raw_writew(reg_val, KDDR);

	reg_val = __raw_readw(KPSR);
	reg_val &= ~(KBD_STAT_KPKR | KBD_STAT_KPKD);
	reg_val |= KBD_STAT_KPKD;
	reg_val |= KBD_STAT_KRSS | KBD_STAT_KDSC;
	__raw_writew(reg_val, KPSR);
	reg_val |= KBD_STAT_KDIE;
	reg_val &= ~KBD_STAT_KRIE;
	__raw_writew(reg_val, KPSR);

	has_leaning_key = keypad->learning;
	mxckpd_keycodes = keypad->matrix;
	mxckpd_keycodes_size = keypad->rowmax * keypad->colmax;

	if ((keypad->matrix == (void *)0)
	    || (mxckpd_keycodes_size == 0)) {
		retval = -ENODEV;
		goto err1;
	}

	mxckbd_dev = input_allocate_device();
	if (!mxckbd_dev) {
		printk(KERN_ERR
		       "mxckbd_dev: not enough memory for input device\n");
		retval = -ENOMEM;
		goto err1;
	}

	mxckbd_dev->keycode = (void *)mxckpd_keycodes;
	mxckbd_dev->keycodesize = sizeof(mxckpd_keycodes[0]);
	mxckbd_dev->keycodemax = mxckpd_keycodes_size;
	mxckbd_dev->name = "mxckpd";
	mxckbd_dev->id.bustype = BUS_HOST;
	mxckbd_dev->open = mxc_kpp_open;
	mxckbd_dev->close = mxc_kpp_close;

	retval = input_register_device(mxckbd_dev);
	if (retval < 0) {
		printk(KERN_ERR
		       "mxckbd_dev: failed to register input device\n");
		goto err2;
	}

	pr_alert("$$$$$$$$$$$$$$$$$$$$$$ mxckbd_dev=%p",mxckbd_dev);

	/* allocate required memory */
	press_scancode = kmalloc(kpp_dev.kpp_rows * sizeof(press_scancode[0]),
				 GFP_KERNEL);
	release_scancode =
	    kmalloc(kpp_dev.kpp_rows * sizeof(release_scancode[0]), GFP_KERNEL);

	if (!press_scancode || !release_scancode) {
		retval = -ENOMEM;
		goto err3;
	}

	for (i = 0; i < kpp_dev.kpp_rows; i++) {
		press_scancode[i] = kmalloc(kpp_dev.kpp_cols
					    * sizeof(press_scancode[0][0]),
					    GFP_KERNEL);
		release_scancode[i] =
		    kmalloc(kpp_dev.kpp_cols * sizeof(release_scancode[0][0]),
			    GFP_KERNEL);

		if (!press_scancode[i] || !release_scancode[i]) {
			retval = -ENOMEM;
			goto err3;
		}
	}

	cur_rcmap =
	    kmalloc(kpp_dev.kpp_rows * sizeof(cur_rcmap[0]), GFP_KERNEL);
	prev_rcmap =
	    kmalloc(kpp_dev.kpp_rows * sizeof(prev_rcmap[0]), GFP_KERNEL);

	if (!cur_rcmap || !prev_rcmap) {
		retval = -ENOMEM;
		goto err3;
	}

	__set_bit(EV_KEY, mxckbd_dev->evbit);

	for (i = 0; i < mxckpd_keycodes_size; i++)
		__set_bit(mxckpd_keycodes[i], mxckbd_dev->keybit);

	__set_bit(KEY_POWER,mxckbd_dev->keybit);

	for (i = 0; i < kpp_dev.kpp_rows; i++) {
		memset(press_scancode[i], -1,
		       sizeof(press_scancode[0][0]) * kpp_dev.kpp_cols);
		memset(release_scancode[i], -1,
		       sizeof(release_scancode[0][0]) * kpp_dev.kpp_cols);
	}
	memset(cur_rcmap, 0, kpp_dev.kpp_rows * sizeof(cur_rcmap[0]));
	memset(prev_rcmap, 0, kpp_dev.kpp_rows * sizeof(prev_rcmap[0]));

	key_pad_enabled = 1;
	/* Initialize the polling timer */
	init_timer(&kpp_dev.poll_timer);

	/*
	 * Request for IRQ number for keypad port. The Interrupt handler
	 * function (mxc_kpp_interrupt) is called when ever interrupt occurs on
	 * keypad port.
	 */
	retval = request_irq(irq, mxc_kpp_interrupt, 0, MOD_NAME, MOD_NAME);
	if (retval) {
		pr_debug("KPP: request_irq(%d) returned error %d\n",
			 MXC_INT_KPP, retval);
		goto err3;
	}

	/* By default, devices should wakeup if they can */
	/* So keypad is set as "should wakeup" as it can */
	//device_init_wakeup(&pdev->dev, 1);
	device_init_wakeup(&pdev->dev, 0); //xu.xb disable keyboard wakeup

	return 0;

      err3:
	mxc_kpp_free_allocated();
      err2:
	input_free_device(mxckbd_dev);
      err1:
	free_irq(irq, MOD_NAME);
	clk_disable(kpp_clk);
	clk_put(kpp_clk);
	return retval;
}

/*!
 * Dissociates the driver from the kpp device.
 *
 * @param   pdev  the device structure used to give information on which SDHC
 *                to remove
 *
 * @return  The function always returns 0.
 */
static int mxc_kpp_remove(struct platform_device *pdev)
{
	unsigned short reg_val;

	/*
	 * Clear the KPKD status flag (write 1 to it) and synchronizer chain.
	 * Set KDIE control bit, clear KRIE control bit (avoid false release
	 * events. Disable the keypad GPIO pins.
	 */
	__raw_writew(0x00, KPCR);
	__raw_writew(0x00, KPDR);
	__raw_writew(0x00, KDDR);

	reg_val = __raw_readw(KPSR);
	reg_val |= KBD_STAT_KPKD;
	reg_val &= ~KBD_STAT_KRSS;
	reg_val |= KBD_STAT_KDIE;
	reg_val &= ~KBD_STAT_KRIE;
	__raw_writew(reg_val, KPSR);

	gpio_keypad_inactive();
	clk_disable(kpp_clk);
	clk_put(kpp_clk);

	KPress = 0;

	del_timer(&kpp_dev.poll_timer);

	free_irq(keypad->irq, MOD_NAME);
	input_unregister_device(mxckbd_dev);

	mxc_kpp_free_allocated();

	return 0;
}

/*!
 * This structure contains pointers to the power management callback functions.
 */
static struct platform_driver mxc_kpd_driver = {
	.driver = {
		   .name = "mxc_keypad",
		   .bus = &platform_bus_type,
		   },
	.suspend = mxc_kpp_suspend,
	.resume = mxc_kpp_resume,
	.probe = mxc_kpp_probe,
	.remove = mxc_kpp_remove,
};

/*!
 * This function is called for module initialization.
 * It registers keypad char driver and requests for KPP irq number. This
 * function does the initialization of the keypad device.
 *
 * The following steps are used for keypad configuration,\n
 * -# Enable number of rows in the keypad control register (KPCR[7:0}).\n
 * -# Write 0's to KPDR[15:8]\n
 * -# Configure keypad columns as open-drain (KPCR[15:8])\n
 * -# Configure columns as output, rows as input (KDDR[15:0])\n
 * -# Clear the KPKD status flag (write 1 to it) and synchronizer chain\n
 * -# Set KDIE control b