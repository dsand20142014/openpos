d->timer,
			  jiffies + bf54x_kpad->keyup_test_jiffies);
	} else {
		input_report_key(input, key, 0);
		input_sync(input);

		bfin_kpad_clear_irq();
	}

	return IRQ_HANDLED;
}

static int __devinit bfin_kpad_probe(struct platform_device *pdev)
{
	struct bf54x_kpad *bf54x_kpad;
	struct bfin_kpad_platform_data *pdata = pdev->dev.platform_data;
	struct input_dev *input;
	int i, error;

	if (!pdata->rows || !pdata->cols || !pdata->keymap) {
		printk(KERN_ERR DRV_NAME
			": No rows, cols or keymap from pdata\n");
		return -EINVAL;
	}

	if (!pdata->keymapsize ||
	    pdata->keymapsize > (pdata->rows * pdata->cols)) {
		printk(KERN_ERR DRV_NAME ": Invalid keymapsize\n");
		return -EINVAL;
	}

	bf54x_kpad = kzalloc(sizeof(struct bf54x_kpad), GFP_KERNEL);
	if (!bf54x_kpad)
		return -ENOMEM;

	platform_set_drvdata(pdev, bf54x_kpad);

	/* Allocate memory for keymap followed by private LUT */
	bf54x_kpad->key