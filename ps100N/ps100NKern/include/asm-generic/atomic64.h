		retval = -EBUSY;
		goto err_put;
	}

	clk = clk_get(&dev->dev, "usb-host");
	if (IS_ERR(clk)) {
		dev_err(&dev->dev, "cannot get usb-host clock\n");
		retval = -ENOENT;
		goto err_mem;
	}

	usb_clk = clk_get(&dev->dev, "usb-bus-host");
	if (IS_ERR(usb_clk)) {
		dev_err(&dev->dev, "cannot get usb-bus-host clock\n");
		retval = -ENOENT;
		goto err_clk;
	}

	s3c2410_start_hc(dev, hcd);

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_err(&dev->dev, "ioremap failed\n");
		retval = -ENOMEM;
		goto err_ioremap;
	}

	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, dev->resource[1].start, IRQF_DISABLED);
	if (retval != 0)
		goto err_ioremap;

	return 0;

 err_ioremap:
	s3c2410_stop_hc(dev);
	iounmap(hcd->regs);
	clk_put(usb_clk);

 err_clk:
	clk_put(clk);

 err_mem:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);

 err_put:
	usb_put_hcd(hcd);
	return retval;
}

/*-------------------------------------------------------------------------*/

static int
ohci_s3c2410_start (struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci (hcd);
	int ret;

	if ((ret = ohci_init(ohci)) < 0)
		return ret;

	if ((ret = ohci_run (ohci)) < 0) {
		err ("can't start %s", hcd->self.bus_name);
		ohci_stop (hcd);
		return ret;
	}

	return 0;
}


static const struct hc_driver ohci_s3c2410_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"S3C24XX OHCI",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =		ohci_s3c2410_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_d