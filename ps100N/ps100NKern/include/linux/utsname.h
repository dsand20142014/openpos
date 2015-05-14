R_MASK,
		"Adaptec aic7850 SCSI adapter",
		ahc_aic785X_setup
	},
	{
		ID_AIC7855 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7855 SCSI adapter",
		ahc_aic785X_setup
	},
	{
		ID_AIC7859 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7859 SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AIC7860 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7860 Ultra SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AIC7870 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7870 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AIC7880 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7880 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AIC7890 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7890/91 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AIC7892 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7892 Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AIC7895 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7895 Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AIC7896 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7896/97 Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AIC7899 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7899 Ultra160 SCSI adapter",
		ahc_aic7899_setup
	},
	{
		ID_AIC7810 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7810 RAID memory controller",
		ahc_raid_setup
	},
	{
		ID_AIC7815 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7815 RAID memory controller",
		ahc_raid_setup
	}
};

static const u_int ahc_num_pci_devs = ARRAY_SIZE(ahc_pci_ident_table);
		
#define AHC_394X_SLOT_CHANNEL_A	4
#define AHC_394X_SLOT_CHANNEL_B	5

#define AHC_398X_SLOT_CHANNEL_A	4
#