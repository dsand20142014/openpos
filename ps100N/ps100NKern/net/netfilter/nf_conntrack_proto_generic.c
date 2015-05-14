hipset = INTEL_965G;
		dinfo->mobile = 0;
		dinfo->pll_index = PLLS_I9xx;
		return 0;
	case PCI_DEVICE_ID_INTEL_965GM:
		dinfo->name = "Intel(R) 965GM";
		dinfo->chipset = INTEL_965GM;
		dinfo->mobile = 1;
		dinfo->pll_index = PLLS_I9xx;
		return 0;
	default:
		return 1;
	}
}

int intelfbhw_get_memory(struct pci_dev *pdev, int *aperture_size,
			 int *stolen_size)
{
	struct pci_dev *bridge_dev;
	u16 tmp;
	int stolen_overhead;

	if (!pdev || !aperture_size || !stolen_size)
		return 1;

	/* Find the bridge device.  It is always 0:0.0 */
	if (!(bridge_dev = pci_get_bus_and_slot(0, PCI_DEVFN(0, 0)))) {
		ERR_MSG("cannot find bridge device\n");
		return 1;
	}

	/* Get the fb aperture size and "stolen" memory amount. */
	tmp = 0;
	pci_read_config_word(bridge_dev, INTEL_GMCH_CTRL, &tmp);
	pci_dev_put(bridge_dev);

	switch (pdev->device) {
	case PCI_DEVICE_ID_INTEL_915G:
	case PCI_DEVICE_ID_INTEL_915GM:
	case PCI_DEVICE_ID_INTEL_945G:
	case PCI_DEVICE_ID_INTEL_945GM:
	case PCI_DEVICE_ID_INTEL_945GME:
	case PCI_DEVICE_ID_INTEL_965G:
	case PCI_DEVICE_ID_INTEL_965GM:
		/* 915, 945 and 965 chipsets support a 256MB aperture.
		   Aperture size is determined by inspected the
		   base address of the aperture. */
		if (pci_resource_start(pdev, 2) & 0x08000000)
			*aperture_size = MB(128);
		else
			*aperture_size = MB(256);
		break;
	default:
		if ((tmp & INTEL_GMCH_MEM_MASK) == INTEL_GMCH_MEM_64M)
			*aperture_size = MB(64);
		else
			*aperture_size = MB(128);
		break;
	}

	/* Stolen memory size is reduced by the GTT and the popup.
	   GTT is 1K per MB of aperture size, and popup is 4K. */
	stolen_overhead = (*aperture_size / MB(1)) + 4;
	switch(pdev->device) {
	case PCI_DEVICE_ID_INTEL_830M:
	case PCI_DEVICE_ID_INTEL_845G:
		switch (tmp & INTEL_830_GMCH_GMS_MASK) {
		case INTEL_830_GMCH_GMS_STOLEN_512:
			*stolen_size = KB(512) - KB(stolen_overhead);
			return 0;
		case INTEL_830_GMCH_GMS_STOLEN_1024:
			*stolen_size = MB(1) - KB(stolen_overhead);
			return 0;
		case INTEL_830_GMCH_GMS_STOLEN_8192:
			*stolen_size = MB(8) - KB(stolen_overhead);
			return 0;
		case INTEL_830_GMCH_GMS_LOCAL:
			ERR_MSG("only local memory found\n");
			return 1;
		case INTEL_830_GMCH_GMS_DISABLED:
			ERR_MSG("video memory is disabled\n");
			return 1;
		default:
			ERR_MSG("unexpected GMCH_GMS value: 0x%02x\n",
				tmp & INTEL_830_GMCH_GMS_MASK);
			return 1;
		}
		break;
	default:
		switch (tmp & INTEL_855_GMCH_GMS_MASK) {
		case INTEL_855_GMCH_GMS_STOLEN_1M:
			*stolen_size = MB(1) - KB(stolen_overhead);
			return 0;
		case INTEL_855_GMCH_GMS_STOLEN_4M:
			*stolen_size = MB(4) - KB(stolen_overhead);
			return 0;
		case INTEL_855_GMCH_GMS_STOLEN_8M:
			*stolen_size = MB(8) - KB(stolen_overhead);
			return 0;
		case INTEL_855_