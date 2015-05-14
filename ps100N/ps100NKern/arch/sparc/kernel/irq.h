bled()) {
		err("USB is disabled");
		ret = -ENODEV;
		goto out;
	}

	if (pdev->num_resources != 2
	    || pdev->resource[0].flags != IORESOURCE_MEM
	    || pdev->resource[1].flags != IORESOURCE_IRQ) {
		err("Invalid resource configuration");
		ret = -ENODEV;
		goto out;
	}

	/* Enable AHB slave USB clock, needed for further USB clock control */
	__raw_writel(USB_SLAVE_HCLK_EN | (1 << 19), USB_CTRL);

	ret = i2c_add_driver(&isp1301_driver);
	if (ret < 0) {
		err("failed to add ISP1301 driver");
		goto out;
	}
	i2c_adap = i2c_get_adapter(2);
	memset(&i2c_info, 0, sizeof(struct i2c_board_info));
	strlcpy(i2c_info.name, "isp1301_pnx", I2C_NAME_SIZE);
	isp1301_i2c_client = i2c_new_probed_device(i2c_adap, &i2c_info,
						   normal_i2c);
	i2c_put_adapter(i2c_adap);
	if (!isp1301_i2c_client) {
		err("failed to connect I2C to ISP1301 USB Transceiver");
		ret = -ENODEV;
		goto out_i2c_driver;
	}

	isp1301_configure();

	/* Enable USB PLL */
	usb_clk = clk_get(&pdev->dev, "ck_pll5");
	if (IS_ERR(usb_clk)) {
		err("failed to acquire USB PLL");
		ret = PTR_ERR(usb_clk);
		goto out1;
	}

	ret = clk_enable(usb_clk);
	if (ret < 0) {
		err("failed to start USB PLL");
		goto out2;
	}

	ret = clk_set_rate(usb_clk, 48000);
	if (ret < 0) {
		err("failed to set USB clock rate");
		goto out3;
	}

	__raw_writel(__raw_readl(USB_CTRL) | USB_HOST_NEED_CLK_EN, USB_CTRL);

	/* Set to enable all needed USB clocks */
	__raw_writel(USB_CLOCK_MASK, USB_OTG_CLK_CTRL);

	while ((__raw_readl(USB_OTG_CLK_STAT) & USB_CLOCK_MASK) !=
	       USB_CLOCK_MASK) ;

	hcd = usb_create_hcd (driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		err("Failed to allocate HC buffer");
		ret = -ENOMEM;
		goto out3;
	}

	/* Set all USB bits in the Start Enable register