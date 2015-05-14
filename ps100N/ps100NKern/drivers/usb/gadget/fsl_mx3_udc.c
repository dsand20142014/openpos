_update_schib(sch))
		return -ENODEV; 
	if (!sch->schib.pmcw.ena)
		/* Not operational -> done. */
		return 0;
	/* Stage 1: cancel io. */
	if (!(scsw_actl(&sch->schib.scsw) & SCSW_ACTL_HALT_PEND) &&
	    !(scsw_actl(&sch->schib.scsw) & SCSW_ACTL_CLEAR_PEND)) {
		if (!scsw_is_tm(&sch->schib.scsw)) {
			ret = cio_cancel(sch);
			if (ret != -EINVAL)
				return ret;
		}
		/* cancel io unsuccessful or not applicable (transport mode).
		 * Continue with asynchronous instructions. */
		cdev->private->iretry = 3;	/* 3 halt retries. */
	}
	if (!(scsw_actl(&sch->schib.scsw) & SCSW_ACTL_CLEAR_PEND)) {
		/* Stage 2: halt io. */
		if (cdev->private->iretry) {
			cdev->private->iretry--;
			ret = cio_halt(sch);
			if (ret != -EBUSY)
				return (ret == 0) ? -EBUSY : ret;
		}
		/* halt io unsuccessful. */
		cdev->private->iretry = 255;	/* 255 clear retries. */
	}
	/* Stage 3: clear io. */
	if (cdev->private->iretry) {
		cdev->private->iretry--;
		ret = cio_clear (sch);
		return (ret == 0) ? -EBUSY : ret;
	}
	panic("Can't stop i/o on subchannel.\n");
}

void ccw_device_update_sense_data(struct ccw_device *cdev)
{
	memset(&cdev->id, 0, sizeof(cdev->id));
	cdev->id.cu_type   = cdev->private->senseid.cu_type;
	cdev->id.cu_model  = cdev->private->senseid.cu_model;
	cdev->id.dev_type  = cdev->private->senseid.dev_type;
	cdev->id.dev_model = cdev->private->senseid.dev_model;
}

int ccw_device_test_sense_data(struct ccw_device *cdev)
{
	return cdev->id.cu_type == cdev->private->senseid.cu_type &&
		cdev->id.cu_model == cdev->private->senseid.cu_model &&
		cdev->id.dev_type == cdev->private->senseid.dev_type &&
		cdev->id.dev_model == cdev->private->senseid.dev_model;
}

/*
 * The machine won't give us any notification by machine check if a chpid has
 * been varied online on the SE so we have to find out by magic (i. e. driving
 * the channel subsystem to device selection and updating our path masks).
 */
static void
__recover_lost_chpids(struct subchannel *sch, int old_lpm)
{
	int mask, i;
	struct chp_id chpid;

	chp_id_init(&chpid);
	for (i = 0; i<8; i++) {
		mask = 0x80 >> i;
		if (!(sch->lpm & mask))
			continue;
		if (old_lpm & mask)
			continue;
		chpid.id = sch->schib.pmcw.chpid[i];
		if (!chp_is_registered(chpid))
			css_schedule_eval_all();
	}
}