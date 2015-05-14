registered);
		return -ENODEV;
	}

	info = framebuffer_alloc(sizeof(struct intelfb_info), &pdev->dev);
	if (!info) {
		ERR_MSG("Could not allocate memory for intelfb_info.\n");
		return -ENODEV;
	}
	if (fb_alloc_cmap(&info->cmap, 256, 1) < 0) {
		ERR_MSG("Could not allocate cmap for intelfb_info.\n");
		goto err_out_cmap;
		return -ENODEV;
	}

	dinfo = info->par;
	dinfo->info  = info;
	dinfo->fbops = &intel_fb_ops;
	dinfo->pdev  = pdev;

	/* Reserve pixmap space. */
	info->pixmap.addr = kzalloc(64 * 1024, GFP_KERNEL);
	if (info->pixmap.addr == NULL) {
		ERR_MSG("Cannot reserve pixmap memory.\n");
		goto err_out_pixmap;
	}

	/* set early this option because it could be changed by tv encoder
	   driver */
	dinfo->fixed_mode = fixed;

	/* Enable device. */
	if ((err = pci_enable_device(pdev))) {
		ERR_MSG("Cannot enable device.\n");
		cleanup(dinfo);
		return -ENODEV;
	}

	/* Set base addresses. */
	if ((ent->device == PCI_DEVICE_ID_INTEL_915G) ||
	    (ent->device == PCI_DEVICE_ID_INTEL_915GM) ||
	    (ent->device == PCI_DEVICE_ID_INTEL_945G)  ||
	    (ent->device == PCI_DEVICE_ID_INTEL_945GM) ||
	    (ent->device == PCI_DEVICE_ID_INTEL_945GME) ||
	    (ent->device == PCI_DEVICE_ID_INTEL_965G) ||
	    (ent->device == PCI_DEVICE_ID_INTEL_965GM)) {

		aperture_bar = 2;
		mmio_bar = 0;
	}
	dinfo->aperture.physical = pci_resource_start(pdev, aperture_bar);
	dinfo->aperture.size     = pci_resource_len(pdev, aperture_bar);
	dinfo->mmio_base_phys    = pci_resource_start(pdev, mmio_bar);
	DBG_MSG("fb aperture: 0x%llx/0x%llx, MMIO region: 0x%llx/0x%llx\n",
		(unsigned long long)pci_resource_start(pdev, aperture_bar),
		(unsigned long long)pci_resource_len(pdev, aperture_bar),
		(unsigned long long)p