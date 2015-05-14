info, width, AHC_TRANS_GOAL, FALSE);
	ahc_unlock(ahc, &flags);
}

static void ahc_linux_set_period(struct scsi_target *starget, int period)
{
	struct Scsi_Host *shost = dev_to_shost(starget->dev.parent);
	struct ahc_softc *ahc = *((struct ahc_softc **)shost->hostdata);
	struct ahc_tmode_tstate *tstate;
	struct ahc_initiator_tinfo *tinfo 
		= ahc_fetch_transinfo(ahc,
				      starget->channel + 'A',
				      shost->this_id, starget->id, &tstate);
	struct ahc_devinfo devinfo;
	unsigned int ppr_options = tinfo->goal.ppr_options;
	unsigned long flags;
	unsigned long offset = tinfo->goal.offset;
	const struct ahc_syncrate *syncrate;

	if (offset == 0)
		offset = MAX_OFFSET;

	if (period < 9)
		period = 9;	/* 12.5ns is our minimum */
	if (period == 9) {
		if (spi_max_width(starget))
			ppr_options |= MSG_EXT_PPR_DT_REQ;
		else
			/* need wide for DT and need DT for 12.5 ns */
			period = 10;
	}

	ahc_compile_devinfo(&devinfo, shost->this_id, starget->id, 0,
			    starget->channel + 'A', ROLE_INITIATOR);

	/* all PPR requests apar