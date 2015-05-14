te->dev_id.ssid,
			      cdev->private->dev_id.devno);
		cdev->private->options.pgroup = 0;
		return;
	}
	if (cdev->private->pgid[last].inf.ps.state1 ==
	    SNID_STATE1_RESET)
		/* No previous pgid found */
		memcpy(&cdev->private->pgid[0],
		       &channel_subsystems[0]->global_pgid,
		       sizeof(struct pgid));
	else
		/* Use existing pgid */
		memcpy(&cdev->private->pgid[0], &cdev->private->pgid[last],
		       sizeof(struct pgid));
}

/*
 * Function called from device_pgid.c after sense path ground has completed.
 */
void
ccw_device_sense_pgid_done(struct ccw_device *cdev, int err)
{
	struct subchannel *sch;

	sch = to_subchannel(cdev->dev.parent);
	switch (err) {
	case -EOPNOTSUPP: /* path grouping not supported, use nop instead. */
		cdev->private->options.pgroup = 0;
		break;
	case 0: /* success */
	case -EACCES: /* partial success, some paths not operational */
		/* Check if all pgids are equal or 0. */
		__ccw_device_get_common_pgid(cdev);
		break;
	case -ETIME:		/* Sense path group id stopped by timeout. */
	case -EUSERS:		/* device is reserved for someone else. */
		ccw_device_done(cdev, DEV_STATE_BOXED);
		return;
	default:
		ccw_device_done(cdev, DEV_STATE_NOT_OPER);
		return;
	}
	/* Start Path Group verification. */
	cdev->private->state = DEV_STATE_VERIFY;
	cdev->private->flags.doverify = 0;
	ccw_device_verify_start(cdev);
}

/*
 * Start device recognition.
 */
int
ccw_device_recognition(struct ccw_device *cdev)
{
	struct subchannel *sch;
	int ret;

	sch = to_subchannel(cdev->dev.parent);
	ret = cio_enable_subchannel(sch, (u32)(addr_t)sch);
	if (ret != 0)
		/* Couldn't enable the subchannel for i/o. Sick device. */
		return ret;

	/* After 60s the device recognition is considered to have failed. */
	ccw_device_set_timeout(cdev, 60*HZ);

	/*
	 * We used to start here with a sense pgid to find out whether a device
	 * is locked by someone else. Unfortunately, the sense pgid command
	 * code has other meanings on devices predating the path grouping
	 * algorithm, so we start with sense id and box the device after an
	 * timeout (or if sense pgid during path verification detects the device
	 * is locked, as may happen on newer devices).
	 */
	cdev->private->flags.recog_done = 0;
	cdev->private->state = DEV_STATE_SENSE_ID;
	ccw_device_sense_id_start(cdev);
	return 0;
}

/*
 * Handle timeout in device recognition.
 */
static void
ccw_device_recog_timeout(struct ccw_device *cdev, enum dev_event dev_event)
{
	int ret;

	ret = ccw_device_cancel_halt_clear(cdev);
	switch (ret) {
	case 0:
		ccw_device_recog_done(cdev, DEV_STATE_BOXED);
		break;
	case -ENODEV:
		ccw_device_recog_done(cdev, DEV_STATE_NOT_OPER);
		break;
	default:
		ccw_device_set_timeout(cdev, 3*HZ);
	}
}


void
ccw_device_verify_done(struct ccw_device *cdev, int err)
{
	struct subchannel *sch;

	sch = to_subchannel(cdev->dev.parent);
	/* Update schib - pom may have changed. */
	if (cio_update_schib(sch)) {
		cdev->private->flags.donotify = 0;
		ccw_device_done(cdev, DEV_STATE_NOT_OPER);
		return;
	}
	/* Update lpm with verified path mask. */
	sch->lpm = sch->vpm;
	/* Repeat path verification? */
	if (cdev->private->flags.doverify) {
		cdev->private->flags.doverify = 0;
		ccw_device_verify_start(cdev);
		return;
	}
	switch (err) {
	case -EOPNOTSUPP: /* path grouping not supported, just set online. */
		cdev->private->options.pgroup = 0;
	case 0:
		ccw_device_done(cdev, DEV_STATE_ONLINE);
		/* Deliver fake irb to device driver, if needed. */
		if (cdev->private->flags.fake_irb) {
			memset(&cdev->private->irb, 0, sizeof(struct irb));
			cdev->private->irb.scsw.cmd.cc = 1;
			cdev->private->irb.scsw.cmd.fctl = SCSW_FCTL_START_FUNC;
			cdev->private->irb.scsw.cmd.actl = SCSW_ACTL_START_PEND;
			cdev->private->irb.scsw.cmd.stctl =
				SCSW_STCTL_STATUS_PEND;
			cdev->private->flags.fake_irb = 0;
			if (cdev->handler)
				cdev->handler(cdev, cdev->private->intparm,
					      &cdev->private->irb);
			memset(&cdev->private->irb, 0, sizeof(struct irb));
		}
		break;
	case -ETIME:
		/* Reset oper notify indication after verify error. */
		cdev->private->flags.donotify = 0;
		ccw_device_done(cdev, DEV_STATE_BOXED);
		break;
	default:
		/* Reset oper notify indication after verify error. */
		cdev->private->flags.donotify = 0;
		if (cdev->online) {
			ccw_device_set_timeout(cdev, 0);
			dev_fsm_event(cdev, DEV_EVENT_NOTOPER);
		} else
			ccw_device_done(cdev, DEV_STATE_NOT_OPER);
		break;
	}
}

/*
 * Get device online.
 */
int
ccw_device_online(struct ccw_device *cdev)
{
	struct subchannel *sch;
	int ret;

	if ((cdev->private->state != DEV_STATE_OFFLINE) &&
	    (cdev->private->state != DEV_STATE_BOXED))
		return -EINVAL;
	sch = to_subchannel(cdev->dev.parent);
	ret = cio_enable_subchannel(sch, (u32)(addr_t)sch);
	if (ret != 0) {
		/* Couldn't enable the subchannel for i/o. Sick device. */
		if (ret == -ENODEV)
			dev_fsm_event(cdev, DEV_EVENT_NOTOPER);
		return ret;
	}
	/* Do we want to do path grouping? */
	if (!cdev->private->options.pgroup) {
		/* Start initial path verification. */
		cdev->private->state = DEV_STATE_VERIFY;
		cdev->private->flags.doverify = 0;
		ccw_device_verify_start(cdev);
		return 0;
	}
	/* Do a SensePGID first. */
	cdev->private->state = DEV_STATE_SENSE_PGID;
	ccw_device_sense_pgid_start(cdev);
	return 0;
}

void
ccw_device_disband_done(struct ccw_device *cdev, int err)
{
	switch (err) {
	case 0:
		ccw_device_done(cdev, DEV_STATE_OFFLINE);
		break;
	case -ETIME:
		ccw_device_done(cdev, DEV_STATE_BOXED);
		break;
	default:
		cdev->private->flags.donotify = 0;
		dev_fsm_event(cdev, DEV_EVENT_NOTOPER);
		ccw_device_done(cdev, DEV_STATE_NOT_OPER);
		break;
	}
}

/*
 * Shutdown device.
 */
int
ccw_device_offline(struct ccw_device *cdev)
{
	struct subchannel *sch;

	/* Allow ccw_device_offline while disconnected. */
	if (cdev->privat