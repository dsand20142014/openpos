
			scb->status &= ~SCB_SELECT;
		host->active = NULL;
	}
	/* --------- get target id---------------------- */
	tar = inb(host->addr + TUL_SBusId);
	/* ------ get LUN from Identify message----------- */
	lun = inb(host->addr + TUL_SIdent) & 0x0F;
	/* 07/22/98 from 0x1F -> 0x0F */
	active_tc = &host->targets[tar];
	host->active_tc = active_tc;
	outb(active_tc->sconfig0, host->addr + TUL_SConfig);
	outb(active_tc->js_period, host->addr + TUL_SPeriod);

	/* ------------- tag queueing ? ------------------- */
	if (active_tc->drv_flags & TCF_DRV_EN_TAG) {
		if ((initio_msgin_accept(host)) == -1)
			return -1;
		if (host->phase != MSG_IN)
			goto no_tag;
		outl(1, host->addr + TUL_SCnt0);
		outb(TSC_XF_FIFO_IN, host->addr + TUL_SCmd);
		if (wait_tulip(host) == -1)
			return -1;
		msg = inb(host->addr + TUL_SFifo);	/* Read Tag Message    */

		if (msg < MSG_STAG || msg > MSG_OTAG)		/* Is simple Tag      */
			goto no_tag;

		if (initio_msgin_accept(host) == -1)
			return -1;

		if (host->phase != MSG_IN)
			goto no_tag;

		outl(1, host->addr + TUL_SCnt0);
		outb(TSC_XF_FIFO_IN, host->addr + TUL_SCmd);
		if (wait_tulip(host) == -1)
			return -1;
		tag = inb(host->addr + TUL_SFifo);	/* Read Tag ID       */
		scb = host->scb + tag;
		if (scb->target != tar || scb->lun != lun) {
			return initio_msgout_abort_tag(host);
		}
		if (scb->status != SCB_BUSY) {	/* 03/24/95             */
			return initio_msgout_abort_tag(host);
		}
		host->active = scb;
		if ((initio_msgin_accept(host)) == -1)
			return -1;
	} else {		/* No tag               */
	      no_tag:
		if ((scb = initio_find_busy_scb(host, tar | (lun << 8))) == NULL) {
			return initio_msgout_abort_targ(host);
		}
		host->active = scb;
		if (!(active_tc->drv_flags & TCF_DRV_EN_TAG)) {
			if ((initio_msgin_accept(host)) == -1)
				return -1;
		}
	}
	return 0;
}

/**
 *	int_initio_bad_seq		-	out of phase
 *	@host: InitIO host flagging event
 *
 *	We have ended up out of phase somehow. Reset the host controller
 *	and throw all our toys out of the pram. Let the midlayer clean up
 */

static int int_initio_bad_seq(struct initio_host * host)
{				/* target wrong phase           */
	struct scsi_ctrl_blk *scb;
	int i;

	initio_reset_scsi(host, 10);

	while ((scb = initio_pop_busy_scb(host)) != NULL) {
		scb->hastat = HOST_BAD_PHAS;
		initio_append_done_scb(host, scb);
	}
	for (i = 0; i < host->max_tar; i++)
		host->targets[i].flags &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
	return -1;
}


/**
 *	initio_msgout_abort_targ		-	abort a tag
 *	@host: InitIO host
 *
 *	Abort when the target/lun does not match or when our SCB is not
 *	busy. Used by untagged commands.
 */

static int initio_msgout_abort_targ(struct initio_host * host)
{

	outb(((inb(host->addr + TUL_SSignal) & (TSC_SET_ACK | 7)) | TSC_SET_ATN), host->addr + TUL_SSignal);
	if (initio_msgin_accept(host) == -1)
		return -1;
	if (host->phase != MSG_OUT)
		return initio_bad_seq(host);

	outb(MSG_ABORT, host->addr + TUL_SFifo);
	outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);

	return initio_wait_disc(host);
}

/**
 *	initio_msgout_abort_tag		-	abort a tag
 *	@host: InitIO host
 *
 *	A