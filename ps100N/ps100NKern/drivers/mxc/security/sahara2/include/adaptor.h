ype, cdev->id.dev_model);
			break;
		}
		cdev->private->state = DEV_STATE_OFFLINE;
		cdev->private->flags.recog_done = 1;
		if (ccw_device_test_sense_data(cdev)) {
			cdev->private->flags.donotify = 1;
			ccw_device_online(cdev);
			wake_up(&cdev->private->wait_q);
		} else {
			ccw_device_update_sense_data(cdev);
			PREPARE_WORK(&cdev->private->kick_work,
				     ccw_device_do_unbind_bind);
			queue_work(ccw_device_work, &cdev->private->kick_work);
		}
		return;
	case DEV_STATE_BOXED:
		CIO_MSG_EVENT(0, "SenseID : boxed device %04x on "
			      " subchannel 0.%x.%04x\n",
			      cdev->private->dev_id.devno,
			      sch->schid.ssid, sch->schid.sch_no);
		if (cdev->id.cu_type != 0) { /* device was recognized before */
			cdev->private->flags.recog_done = 1;
			cdev->private->state = DEV_STATE_BOXED;
			wake_up(&cdev->private->wait_q);
			return;
		}
		break;
	}
	cdev->private->state = state;
	io_subchannel_recog_done(cdev);
	wake_up(&cdev->private->wait_q);
}

/*
 * Function called from device_id.c after sense id has completed.
 */
void
ccw_device_sense_id_done(struct ccw_device *cdev, int err)
{
	switch (err) {
	case 0:
		ccw_device_recog_done(cdev, DEV_STATE_OFFLINE);
		break;
	case -ETIME:		/* Sense id stopped by timeout. */
		ccw_device_recog_done(cdev, DEV_STATE_BOXED);
		break;
	default:
		ccw_device_recog_done(cdev, DEV_STATE_NOT_OPER);
		break;
	}
}

int ccw_device_notify(struct ccw_device *cdev, int event)
{
	if (!cdev->drv)
		return 0;
	if (!cdev->online)
		return 0;
	CIO_MSG_EVENT(2, "notify called for 0.%x.%04x, event=%d\n",
		      cdev->private->dev_id.ssid, cdev->private->dev_id.devno,
		      event);
	return cdev->drv->notify ? cdev->drv->notify(cdev, event) : 0;
}

static void cmf_reenable_delayed(struct work_struct *work)
{
	struct ccw_device_private *priv;
	struct ccw_device *cdev;

	priv = container_of(work, struct ccw_device_private, kick_work);
	cdev = priv->cdev;
	cmf_reenable(cdev);
}

static void ccw_device_oper_notify(struct ccw_device *cdev)
{
	if (ccw_device_notify(cdev, CIO_OPER)) {
		/* Reenable channel measurements, if needed. */
		PREPARE_WORK(&cdev->private->kick_work, cmf_reenable_delayed);
		queue_work(ccw_device_work, &cdev->private->kick_work);
		return;
	}
	/* Driver doesn't want device back. */
	ccw_device_set_notoper(cdev);
	PREPARE_WORK(&cdev->private->kick_work, ccw_device_do_unbind_bind);
	queue_work(ccw_device_work, &cdev->private->kick_work);
}

/*
 * Finished with online/offline processing.
 */
static void
ccw_device_done(struct ccw_device *cdev, int state)
{
	struct subchannel *sch;

	sch = to_subchannel(cdev->dev.parent);

	ccw_device_set_timeout(cdev, 0);

	if (state != DEV_STATE_ONLINE)
		cio_disable_subchannel(sch);

	/* Reset device status. */
	memset(&cdev->private->irb, 0, sizeof(struct irb));

	cdev->private->state = state;

	if (state == DEV_STATE_BOXED) {
		CIO_MSG_EVENT(0, "Boxed device %04x on subchannel %04x\n",
			      cdev->private->dev_id.devno, sch->schid.sch_no);
		if (cdev->online && !ccw_device_notify(cdev, CIO_BOXED))
			ccw_device_schedule_sch_unregister(cdev);
		cdev->private->flags.donotify = 0;
	}

	if (cdev->private->flags.donotify) {
		cdev->private->flags.donotify = 0;
		ccw_device_oper_notify(cdev);
	}
	wake_up(&cdev->private->wait_q);
}

static int cmp_pgid(struct pgid *p1, struct pgid *p2)
{
	char *c1;
	char *c2;

	c1 = (char *)p1;
	c2 = (char *)p2;

	return memcmp(c1 + 1, c2 + 1, sizeof(struct pgid) - 1);
}

static void __ccw_device_get_common_pgid(struct ccw_device *cdev)
{
	int i;
	int last;

	last = 0;
	for (i = 0; i < 8; i++) {
		if (cdev->private->pgid[i].inf.ps.state1 == SNID_STATE1_RESET)
			/* No PGID yet */
			continue;
		if (cdev->private->pgid[last].inf.ps.state1 ==
		    SNID_STATE1_RESET) {
			/*