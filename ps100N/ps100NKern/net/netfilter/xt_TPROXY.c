t input_dev *dev)
{
	/* enable clock */
	return 0;
}

static void stmpkbd_close(struct input_dev *dev)
{
	/* disable clock */
}

static void stmpkbd_hwinit(struct platform_device *pdev)
{
	hw_lradc_init_ladder(LRADC_CH0, LRADC_DELAY_TRIGGER_BUTTON, 200);
	__raw_writel(BM_LRADC_CTRL1_LRADC0_IRQ,
		REGS_LRADC_BASE + HW_LRADC_CTRL1_CLR);
	__raw_writel(BM_LRADC_CTRL1_LRADC0_IRQ_EN,
		REGS_LRADC_BASE + HW_LRADC_CTRL1_SET);
	hw_lradc_set_delay_trigger_kick(LRADC_DELAY_TRIGGER_BUTTON, !0);
}

static int stmpkbd_suspend(struct platform_device *pdev, pm_message_t state)
{
#ifdef CONFIG_PM
	struct input_dev *idev = platform_get_drvdata(pdev);

	hw_lradc_stop_ladder(LRADC_CH0, LRADC_DELAY_TRIGGER_BUTTON);
	hw_lradc_set_delay_trigger_kick(LRADC_DELAY_TRIGGER_BUTTON, 0);
	hw_lradc_unuse_channel(LRADC_CH0);
	__raw_writel(BM_LRADC_CTRL1_LRADC0_IRQ_EN,
		REGS_LRADC_BASE + HW_LRADC_CTRL1_CLR);
	stmpkbd_close(idev);
#endif
	return 0;
}

static int stmpkbd_resume(struct platform_device *pdev)
{
#ifdef CONFIG_PM
	struct input_dev *idev = platform_get_drvdata(pdev);

	__raw_writel(BM_LRADC_CTRL1_LRADC0_IRQ_EN,
		REGS_LRADC_BASE + HW_LRADC_CTRL1_SET);
	stmpkbd_open(idev);
	hw_lradc_use_channel(LRADC_CH0);
	stmpkbd_hwinit(pdev);
#endif
	return 0;
}

static int __devinit stmpkbd_probe(struct platform_device *pdev)
{
	int err = 0;
	int irq = platform_get_irq(pdev, 0);
	struct stmpkbd_data *d;

	/* Create and register the input driver. */
	d = stmpkbd_data_alloc(pdev,
		(struct stmpkbd_keypair *)pdev->dev.platform_data);
	if (!d) {
		dev_err(&pdev->dev, "Cannot allocate driver structures\n");
		err = -ENOMEM;
		goto err_out;
	}

	d->irq = irq;
	err = request_irq(irq, stmpkbd_irq_handler,
		IRQF_DISABLED, pdev->name, pdev);
	if (err) {
		dev_err(&pdev->dev, "Cannot request keypad IRQ\n");
		goto err_free_dev;
	}

	platform_set_drvdata(pdev, d);

	/* Register the input device */
	err = input_register_device(GET_INPUT_DEV(d));
	if (err)
		goto err_free_irq;

	/* these two have to be set after registering the input device */
	d->input->rep[REP_DELAY] = delay1;
	d->input->rep[REP_PERIOD] = delay2;

	hw_lradc_use_channel(LRADC_CH0);
	stmpkbd_hwinit(pdev);

	return 0;

err_free_irq:
	platform_set_drvdata(pdev, NULL);
	free_irq(irq, pdev);
err_free_dev:
	stmpkbd_data_free(d);
err_out:
	return err;
}

static int __devexit stmpkbd_remove(struct platform_device *pdev)
{
	struct stmpkbd_data *d = platform_get_drvdata(pdev);

	hw_lradc_unuse_channel(LRADC_CH0);
	input_unregister_device(GET_INPUT_DEV(d));
	free_irq(d->irq, pdev);
	stmpkbd_data_free(d);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver stmpkbd_driver = {
	.probe		= stmpkbd_probe,
	.remove		= __devexit_p(stmpkbd_remove),
	.suspend	= stmpkbd_suspend,
	.resume		= stmpkbd_resume,
	.driver		= {
		.name	= "stmp3xxx-keyboard",
	},
};

static int __init stmpkbd_init(void)
{
	return platform_driver_register(&stmpkbd_driver);
}

static 