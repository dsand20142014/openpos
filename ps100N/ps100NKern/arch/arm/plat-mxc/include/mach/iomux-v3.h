spin_lock_init(&fgdev->triplets_dropped_lock);
	return 0;

err_no_mem:
	free_all_frame_buffers(fgdev);
err_no_buf:
	return -ENOMEM;
}

/* find next free minor number, returns -1 if none are availabile */
static int get_free_minor(void)
{
	int i;
	for (i = 0; i < B3DFG_MAX_DEVS; i++) {
		if (b3dfg_devices[i] == 0)
			return i;
	}
	return -1;
}

static int __devinit b3dfg_probe(struct pci_dev *pdev,
	const struct pci_device_id *id)
{
	struct b3dfg_dev *fgdev = kzalloc(sizeof(*fgdev), GFP_KERNEL);
	int r = 0;
	int minor = get_free_minor();
	dev_t devno = MKDEV(MAJOR(b3dfg_devt), minor);
	unsigned long res_len;
	resource_size_t res_base;

	if (fgdev == NULL)
		return -ENOMEM;

	if (minor < 0) {
		dev_err(&pdev->dev, "too many devices found!\n");
		r = -EIO;
		goto err_free;
	}

	b3dfg_devices[minor] = 1;
	dev_info(&pdev->dev, "probe device with IRQ %d\n", pdev->irq);

	cdev_init(&fgdev->chardev, &b3dfg_fops);
	fgdev->chardev.owner = THIS_MODULE;

	r = cdev_add(&fgdev->chardev, devno, 1);
	if (r) {
		dev_err(&pdev->dev, "cannot add char device\n");
		goto err_release_minor;
	}

	fgdev->dev = device_create(
		b3dfg_class,
		&pdev->dev,
		devno,
		dev_get_drvdata(&pdev->dev),
		DRIVER_NAME "%d", minor);

	if (IS_ERR(fgdev->dev)) {
		dev_err(&pdev->dev, "cannot create device\n");
		r = PTR_ERR(fgdev->dev);
		goto err_del_cdev;
	}

	r = pci_enable_device(pdev);
	if (r) {
		dev_err(&pdev->dev, "cannot enable PCI device\n");
		goto err_dev_unreg;
	}

	res_len = pci_resource_len(pdev, B3DFG_BAR_REGS);
	if (res_len != B3DFG_REGS_LENGTH) {
		dev_err(&pdev->dev, "invalid register resource size\n");
		r = -EIO;
		goto err_disable;
	}

	if (pci_resource_flags(pdev, B3DFG_BAR_REGS)
				!= (IORESOURCE_MEM | IORESOURCE_SIZEALIGN)) {
		dev_err(&pdev->dev, "invalid resource flags\n");
		r = -EIO;
		goto err_disable;
	}
	r = pci_request_regions(pdev, DRIVER_NAME);
	if (r) {
		dev_err(&pdev->dev, "cannot obtain PCI resources\n");
		goto err_disable;
	}

	pci_set_master(pdev);

	r = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (r) {
		dev_err(&pdev->dev, "no usable DMA configuration\n");
		goto err_free_res;
	}

	res_base = pci_resource_start(pdev, B3DFG_BAR_REGS);
	fgdev->regs = ioremap_nocache(res_base, res_len);
	if (!fgdev->regs) {
		dev_err(&pdev->dev, "regs ioremap failed\n");
		r = -EIO;
		goto err_free_res;
	}

	fgdev->pdev = pdev;
	pci_set_drvdata(pdev, fgdev);
	r = b3dfg_init_dev(fgdev);
	if (r < 0) {
		dev_err(&pdev->dev, "failed to initalize device\n");
		goto err_unmap;
	}

	r = request_irq(pdev->irq, b3dfg_intr, IRQF_SHARED, DRIVER_NAME, fgdev);
	if (r) {
		dev_err(&pdev->dev, "couldn't request irq %d\n", pdev->irq);
		goto err_free_bufs;
	}

	return 0;

err_free_bufs:
	free_all_frame_buffers(fgdev);
err_unmap:
	iounmap(fgdev->regs);
err_free_res:
	pci_release_regions(pdev);
err_disable:
	pci_disable_device(pdev);
err_dev_unreg:
	device_destroy(b3dfg_class, devno);
err_del_cdev:
	cdev_del(&fgdev->chardev);
err_release_minor:
	b3dfg_devices[minor] = 0;
err_free:
	kfree(fgdev);
	return r;
}

static void __devexit b3dfg_remove(struct pci_dev *pdev)
{
	struct b3dfg_dev *fgdev = pci_get_drvdata(pdev);
	unsigned int minor = MINOR(fgdev->chardev.dev);

	dev_dbg(&pdev->dev, "remove\n");

	free_irq(pdev->irq, fgdev);
	iounmap(fgdev->regs);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	device_destroy(b3dfg_class, MKDEV(MAJOR(b3dfg_devt), minor));
	cdev_del(&fgdev->chardev);
	free_all_frame_buffers(fgdev);
	kfree(fgdev);
	b3dfg_devices[minor] = 0;
}

static struct pci_driver b3dfg_driver = {
	.name = DRIVER_NAME,
	.id_table = b3dfg_ids,
	.probe = b3dfg_probe,
	.remove = __devexit_p(b3dfg_remove),
};

static int __init b3dfg_module_init(void)
{
	int r;

	if (b3dfg_nbuf < 2) {
		printk(KERN_ERR DRIVER_NAME
			   ": buffer_count is out of range (must be >= 2)");
		return -EINVAL;
	