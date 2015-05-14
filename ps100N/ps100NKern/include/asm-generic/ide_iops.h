ctl(struct ahc_softc *ahc);
static void ahc_pci_intr(struct ahc_softc *ahc);
static int  ahc_pci_chip_init(struct ahc_softc *ahc);

static int
ahc_9005_subdevinfo_valid(uint16_t device, uint16_t vendor,
			  uint16_t subdevice, uint16_t subvendor)
{
	int result;

	/* Default to invalid. */
	result = 0;
	if (vendor == 0x9005
	 && subvendor == 0x9005
         && subdevice != device
         && SUBID_9005_TYPE_KNOWN(subdevice) != 0) {

		switch (SUBID_9005_TYPE(subdevice)) {
		case SUBID_9005_TYPE_MB:
			break;
		case SUBID_9005_TYPE_CARD:
		case SUBID_9005_TYPE_LCCARD:
			/*
			 * Currently only trust Adaptec cards to
			 * get the sub device info correct.
			 */
			if (DEVID_9005_TYPE(device) == DEVID_9005_TYPE_HBA)
				result = 1;
			break;
	