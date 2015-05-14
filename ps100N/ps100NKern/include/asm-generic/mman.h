[bit];
				printf(s, ahd_name(ahd), "SG");
			}
		}
	}
	/*
	 * Clear PCI-X status bits.
	 */
	ahd_pci_write_config(ahd->dev_softc, PCIXR_STATUS,
			     pcix_status, /*bytes*/2);
	ahd_outb(ahd, CLRINT, CLRSPLTINT);
	ahd_restore_modes(ahd, saved_modes);
}

static int
ahd_aic7901_setup(struct ahd_softc *ahd)
{

	ahd->chip = AHD_AIC7901;
	ahd->features = AHD_AIC7901_FE;
	return (ahd_aic790X_setup(ahd));
}

static int
ahd_aic7901A_setup(struct ahd_softc *ahd)
{

	ahd->chip = AHD_AIC7901A;
	ahd->features = AHD_AIC7901A_FE;
	return (ahd_aic790X_setup(ahd));
}

static int
ahd_aic7902_setup(struct ahd_softc *ahd)
{
	ahd->chip = AHD_AIC7902;
	ahd->features = AHD_AIC7902_FE;
	return (ahd_aic790X_setup(ahd));
}

static int
ahd_aic790X_set