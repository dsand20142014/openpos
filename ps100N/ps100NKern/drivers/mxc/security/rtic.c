id base (0x%x) for gpio %d\n",
			gpio_base, offset);
		return;
	}

	mask = ASIC3_GPIO_TO_MASK(offset);

	spin_lock_irqsave(&asic->lock, flags);

	out_reg = asic3_read_register(asic, gpio_base + ASIC3_GPIO_OUT);

	if (value)
		out_reg |= mask;
	else
		out_reg &= ~mask;

	asic3_write_register(asic, gpio_base + ASIC3_GPIO_OUT, out_reg);

	spin_unlock_irqrestore(&asic->lock, flags);

	return;
}

static __init int asic3_gpio_probe(struct platform_device *pdev,
				   u16 *gpio_config, int num)
{
	struct asic3 *asic = platform_get_drvdata(pdev);
	u16 alt_reg[ASIC3_NUM_GPIO_BANKS];
	u16 out_reg[ASIC3_NUM_GPIO_BANKS];
	u16 dir_reg[ASIC3_NUM_GPIO_BANKS];
	int i;

	memset(alt_reg, 0, ASIC3_NUM_GPIO_BANKS * sizeof(u16));
	memset(out_reg, 0, ASIC3_NUM_GPIO_BANKS * sizeof(u16));
	memset(dir_reg, 0, ASIC3_NUM_GPIO_BANKS * sizeof(u16));

	/* Enable all GPIOs */
	asic3_write_register(asic, ASIC3_GPIO_OFFSET(A, MASK), 0xffff);
	asic3_write_register(asic, ASIC3_GPIO_OFFSET(B, MASK), 0xffff);
	asic3_write_register(asic, ASIC3_GPIO_OFFSET(C, MASK), 0xffff);
	asic3_write_register(asic, ASIC3_GPIO_OFFSET(D, MASK), 0xffff);

	for (i = 0; i < num; i++) {
		u8 alt, pin, dir, init, bank_num, bit_num;
		u16 config = gpio_config[i];

		pin = ASIC3_CONFIG_GPIO_PIN(config);
		alt = ASIC3_CONFIG_GPIO_ALT(config);
		dir = ASIC3_CONFIG_GPIO_DIR(config);
		init = ASIC3_CONFIG_GPIO_INIT(config);

		bank_num = ASIC3_GPIO_TO_BANK(pin);
		bit_num = ASIC3_GPIO_TO_BIT(pin);

		alt_reg[bank_num] |= (alt << bit_num);
		out_reg[bank_num] |= (init << bit_num);
		dir_reg[bank_num] |= (dir << bit_num);
	}

	for (i = 0; i < ASIC3_NUM_GPIO_BANKS; i++) {
		asic3_write_register(asic,
				     ASIC3_BANK_TO_BASE(i) +
				     ASIC3_GPIO_DIRECTION,
				     dir_reg[i]);
		asic3_write_register(asic,
				     ASIC3_BANK_TO_BASE(i) + ASIC3_GPIO_OUT,
				     out_reg[i]);
		asic3_write_register(asic,
				     ASIC3_BANK_TO_BASE(i) +
				     ASIC3_GPIO_ALT_FUNCTION,
				     alt_reg[i]);
	}

	return gpiochip_add(&asic->gpio);
}

static int asic3_gpio_remove(struct platform_device *pdev)
{
	struct asic3 *asic = platform_get_drvdata(pdev);

	return gpiochip_remove(&asic->gpio);
}

static int asic3_clk_enable(struct asic3 *asic, struct asic3_clk *clk)
{
	unsigned long flags;
	u32 cdex;

	spin_lock_irqsave(&asic->lock, flags);
	if (clk->enabled++ == 0) {
		cdex = asic3_read_register(asic, ASIC3_OFFSET(CLOCK, CDEX));
		cdex |= clk->cdex;
		asic3_write_register(asic, ASIC3_OFFSET(CLOCK, CDEX), cdex);
	}
	spin_unlock_irqrestore(&asic->lock, flags);

	return 0;
}

static void asic3_clk_disable(struct asic3 *asic, struct asic3_clk *clk)
{
	unsigned long flags;
	u32 cdex;

	WARN_ON(clk->enabled == 0);

	spin_lock_irqsave(&asic->lock, flags);
	if (--clk->enabled == 0) {
		cdex = asic3_read_register(asic, ASIC3_OFFSET(CLOCK, CDEX));
		cdex &= ~clk->cdex;
		asic3_write_register(asic, ASIC3_OFFSET(CLOCK, CDEX), cdex);
	}
	spin_unlock_irqrestore(&asic->lock, flags);
}

/* MFD cells (SPI, PWM, LED, DS1WM, MMC) */
static struct ds1wm_driver_data ds1wm_pdata = {
	.active_high = 1,
};

static struct resource ds1wm_resources[] = {
	{
		.start = ASIC3_OWM_BASE,
		.end   = ASIC3_OWM_BASE + 0x13,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = ASIC3_IRQ_OWM,
		.start = ASIC3_IRQ_OWM,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static int ds1wm_enable(struct platform_device *pdev)
{
	struct asic3 *asic = dev_get_drvdata(pdev->dev.parent);

	/* Turn on external clocks and the OWM clock */
	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_EX0]);
	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_EX1]);
	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_OWM]);
	msleep(1);

	/* Reset and enable DS1WM */
	asic3_set_register(asic, ASIC3_OFFSET(EXTCF, RESET),
			   ASIC3_EXTCF_OWM_RESET, 1);
	msleep(1);
	asic3_set_register(asic, ASIC3_OFFSET(EXTCF, RESET),
			   ASIC3_EXTCF_OWM_RESET, 0);
	msleep(1);
	asic3_set_register(asic, ASIC3_OFFSET(EXTCF, SELECT),
			   ASIC3_EXTCF_OWM_EN, 1);
	msleep(1);

	return 0;
}

static int ds1wm_disable(struct platform_device *pdev)
{
	struct asic3 *asic = dev_get_drvdata(pdev->dev.parent);

	asic3_set_register(asic, ASIC3_OFFSET(EXTCF, SELECT),
			   ASIC3_EXTCF_OWM_EN, 0);

	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_OWM]);
	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_EX0]);
	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_EX1]);

	return 0;
}

static struct mfd_cell asic3_cell_ds1wm = {
	.name          = "ds1wm",
	.enable        = ds1wm_enable,
	.disable       = ds1wm_disable,
	.driver_data   = &ds1wm_pdata,
	.num_resources = ARRAY_SIZE(ds1wm_resources),
	.resources     = ds1wm_resources,
};

static struct tmio_mmc_data asic3_mmc_data = {
	.hclk = 24576000,
};

static struct resource asic3_mmc_resources[] = {
	{
		.start = ASIC3_SD_CTRL_BASE,
		.end   = ASIC3_SD_CTRL_BASE + 0x3ff,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = ASIC3_SD_CONFIG_BASE,
		.end   = ASIC3_SD_CONFIG_BASE + 0x1ff,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,
	},
};

static int asic3_mmc_enable(struct platform_device *pdev)
{
	struct asic3 *asic = dev_get_drvdata(pdev->dev.parent);

	/* Not sure if it must be done bit by bit, but leaving as-is */
	asic3_set_register(asic, ASIC3_OFFSET(SDHWCTRL, SDCONF),
			   ASIC3_SDHWCTRL_LEVCD, 1);
	asic3_set_register(asic, ASIC3_OFFSET(SDHWCTRL, SDCONF),
			   ASIC3_SDHWCTRL_LEVWP, 1);
	asic3_set_register(asic, ASIC3_OFFSET(SDHWCTRL, SDCONF),
			   ASIC3_SDHWCTRL_SUSPEND, 0);
	asic3_set_register(asic, ASIC3_OFFSET(SDHWCTRL, SDCONF),
			   ASIC3_SDHWCTRL_PCLR, 0);

	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_EX0]);
	/* CLK32 used for card detection and for interruption detection
	 * when HCLK is stopped.
	 */
	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_EX1]);
	msleep(1);

	/* HCLK 24.576 MHz, BCLK 12.288 MHz: */
	asic3_write_register(asic, ASIC3_OFFSET(CLOCK, SEL),
		CLOCK_SEL_CX | CLOCK_SEL_SD_HCLK_SEL);

	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_SD_HOST]);
	asic3_clk_enable(asic, &asic->clocks[ASIC3_CLOCK_SD_BUS]);
	msleep(1);

	asic3_set_register(asic, ASIC3_OFFSET(EXTCF, SELECT),
			   ASIC3_EXTCF_SD_MEM_ENABLE, 1);

	/* Enable SD card slot 3.3V power supply */
	asic3_set_register(asic, ASIC3_OFFSET(SDHWCTRL, SDCONF),
			   ASIC3_SDHWCTRL_SDPWR, 1);

	return 0;
}

static int asic3_mmc_disable(struct platform_device *pdev)
{
	struct asic3 *asic = dev_get_drvdata(pdev->dev.parent);

	/* Put in suspend mode */
	asic3_set_register(asic, ASIC3_OFFSET(SDHWCTRL, SDCONF),
			   ASIC3_SDHWCTRL_SUSPEND, 1);

	/* Disable clocks */
	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_SD_HOST]);
	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_SD_BUS]);
	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_EX0]);
	asic3_clk_disable(asic, &asic->clocks[ASIC3_CLOCK_EX1]);
	return 0;
}

static struct mfd_cell asic3_cell_mmc = {
	.name          = "tmio-mmc",
	.enable        = asic3_mmc_enable,
	.disable       = asic3_mmc_disable,
	.driver_data   = &asic3_mmc_data,
	.num_resources = ARRAY_SIZE(asic3_mmc_resources),
	.resources     = asic3_mmc_resources,
};

static int __init asic3_mfd_probe(struct platform_device *pdev,
				  struct resource *mem)
{
	struct asic3 *asic = platform_get_drvdata(pdev);
	struct resource *mem_sdio;
	int irq, ret;

	mem_sdio = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem_sdio)
		dev_dbg(asic->dev, "no SDIO MEM resource\n");

	irq = platform_get_irq(pdev, 1);
	if (irq < 0)
		dev_dbg(asic->dev, "no SDIO IRQ resource\n");

	/* DS1WM */
	asic3_set_register(asic, ASIC3_OFFSET(EXTCF, SELECT),
			   ASIC3_EXTCF_OWM_SMB, 0);

	ds1wm_resources[0].start >>= asic->bus_shift;
	ds1wm_resources[0].end   >>= asic->bus_shift;

	asic3_cell_ds1wm.platform_data = &asic3_cell_ds1wm;
	asic3_cell_ds1wm.data_size = sizeof(asic3_cell_ds1wm);

	/* MMC */
	asic3_mmc_resources[0].start >>= asic->bus_shift;
	asic3_mmc_resources[0].end   >>= asic->bus_shift;
	asic3_mmc_resources[1].start >>= asic->bus_shift;
	asic3_mmc_resources[1].end   >>= asic->bus_shift;

	asic3_cell_mmc.platform_data = &asic3_cell_mmc;
	asic3_cell_mmc.data_size = sizeof(asic3_cell_mmc);

	ret = mfd_add_devices(&pdev->dev, pdev->id,
			&asic3_cell_ds1wm, 1, mem, asic->irq_base);
	if (ret < 0)
		goto out;

	if (mem_sdio && (irq >= 0))
		ret = mfd_add_devices(&pdev->dev, pdev->id,
			&asic3_cell_mmc, 1, mem_sdio, irq);

 out:
	return ret;
}

static void asic3_mfd_remove(struct platform_device *pdev)
{
	mfd_remove_devices(&pdev->dev);
}

/* Core */
static int __init asic3_probe(struct platform_device *pdev)
{
	struct asic3_platform_data *pdata = pdev->dev.platform_data;
	struct asic3 *asic;
	struct resource *mem;
	unsigned long clksel;
	int ret = 0;

	asic = kzalloc(sizeof(struct asic3), GFP_KERNEL);
	if (asic == NULL) {
		printk(KERN_ERR "kzalloc failed\n");
		return -ENOMEM;
	}

	spin_lock_init(&asic->lock);
	platform_set_drvdata(pdev, asic);
	asic->dev = &pdev->dev;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		ret = -ENOMEM;
		dev_err(asic->dev, "no MEM resource\n");
		goto out_free;
	}

	asic->mapping = ioremap(mem->start, resource_size(mem));
	if (!asic->mapping) {
		ret = -ENOMEM;
		dev_err(asic->dev, "Couldn't ioremap\n");
		goto out_free;
	}

	asic->irq_base = pdata->irq_base;

	/* calculate bus shift from mem resource */
	asic->bus_shift = 2 - (resource_size(mem) >> 12);

	clksel = 0;
	asic3_write_register(asic, ASIC3_OFFSET(CLOCK, SEL), clksel);

	ret = asic3_irq_probe(pdev);
	if (ret < 0) {
		dev_err(asic->dev, "Couldn't probe IRQs\n");
		goto out_unmap;
	}

	asic->gpio.base = pdata->gpio_base;
	asic->gpio.ngpio = ASIC3_NUM_GPIOS;
	asic->gpio.get = asic3_gpio_get;
	asic->gpio.set = asic3_gpio_set;
	asic->gpio.direction_input = asic3_gpio_direction_input;
	asic->gpio.direction_output = asic3_gpio_direction_output;

	ret = asic3_gpio_probe(pdev,
			       pdata->gpio_config,
			       pdata->gpio_config_num);
	if (ret < 0) {
		dev_err(asic->dev, "GPIO probe failed\n");
		goto out_irq;
	}

	/* Making a per-device copy is only needed for the
	 * theoretical case of multiple ASIC3s on one board:
	 */
	memcpy(asic->clocks, asic3_clk_init, sizeof(asic3_clk_init));

	asic3_mfd_probe(pdev, mem);

	dev_info(asic->dev, "ASIC3 Core driver\n");

	return 0;

 out_irq:
	asic3_irq_remove(pdev);

 out_unmap:
	iounmap(asic->mapping);

 out_free:
	kfree(asic);

	return ret;
}

static int asic3_remove(struct platform_device *pdev)
{
	int ret;
	struct asic3 *asic = platform_get_drvdata(pdev);

	asic3_mfd_remove(pdev);

	ret = asic3_gpio_remove(pdev);
	if (ret < 0)
		return ret;
	asic3_irq_remove(pdev);

	asic3_write_register(asic, ASIC3_OFFSET(CLOCK, SEL), 0);

	iounmap(asic->mapping);

	kfree(asic);

	return 0;
}

static void asic3_shutdown(struct platform_device *pdev)
{
}

static struct platform_driver asic3_device_driver = {
	.driver		= {
		.name	= "asic3",
	},
	.remove		= __devexit_p(asic3_remove),
	.shutdown	= asic3_shutdown,
};

static int __init asic3_init(void)
{
	int retval = 0;
	retval = platform_driver_probe(&asic3_device_driver, asic3_probe);
	return retval;
}

subsys_initcall(asic3_init);
                                                                                                                        