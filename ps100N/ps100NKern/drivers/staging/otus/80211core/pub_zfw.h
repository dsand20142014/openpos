w(chsc_debug_msg_id, &debug_sprintf_view);
	debug_set_level(chsc_debug_msg_id, 2);
	chsc_debug_log_id = debug_register("chsc_log", 16, 1, 16);
	if (!chsc_debug_log_id)
		goto out;
	debug_register_view(chsc_debug_log_id, &debug_hex_ascii_view);
	debug_set_level(chsc_debug_log_id, 2);
	return 0;
out:
	if (chsc_debug_msg_id)
		debug_unregister(chsc_debug_msg_id);
	return -ENOMEM;
}

static void chsc_remove_dbfs(void)
{
	debug_unregister(chsc_debug_log_id);
	debug_unregister(chsc_debug_msg_id);
}

static int __init chsc_init_sch_driver(void)
{
	return css_driver_register(&chsc_subchannel_driver);
}

static void chsc_cleanup_sch_driver(void)
{
	css_driver_unregister(&chsc_subchannel_driver);
}

static DEFINE_SPINLOCK(chsc_lock);

static int chsc_subchannel_match_next_free(struct device *dev, void *data)
{
	struct subchannel *sch = to_subchannel(dev);

	return sch->schib.pmcw.ena && !scsw_fctl(&sch->schib.scsw);
}

static struct subchannel *chsc_get_next_subchannel(struct subchannel *sch)
{
	struct device *dev;

	dev = driver_find_device(&chsc_subchannel_driver.drv,
				 sch ? &sch->dev : NULL, NULL,
				 chsc_subchannel_match_next_free);
	return dev ? to_subchannel(dev) : NULL;
}

/**
 * chsc_async() - try to start a chsc request asynchronously
 * @chsc_area: request to be started
 * @request: request structure to associate
 *
 * Tries to start a chsc request on one of the existing chsc subchannels.
 * Returns:
 *  %0 if the request was performed synchronously
 *  %-EINPROGRESS if the request was successfully started
 *  %-EBUSY if all chsc subchannels are busy
 *  %-ENODEV if no chsc subchannels are available
 * Context:
 *  interrupts disabled, chsc_lock held
 */
static int chsc_async(struct chsc_async_area *chsc_area,
		      struct chsc_request *request)
{
	int cc;
	struct chsc_private *private;
	struct subchannel *sch = NULL;
	int ret = -ENODEV;
	char dbf[10];

	chsc_area->header.key = PAGE_DEFAULT_KEY;
	while ((sch = chsc_get_next_subchannel(sch))) {
		spin_lock(sch->lock);
		private = sch->private;
		if (private->request) {
			spin_unlock(sch->lock);
			ret = -EBUSY;
			continue;
		}
		chsc_area->header.sid = sch->schid;
		CHSC_LOG(2, "schid");
		CHSC_LOG_HEX(2, &sch->schid, sizeof(sch->schid));
		cc = chsc(chsc_area);
		sprintf(dbf, "cc:%d", cc);
		CHSC_LOG(2, dbf);
		switch (cc) {
		case 0:
			ret = 0;
			break;
		case 1:
			sch->schib.scsw.cmd.fctl |= SCSW_FCTL_START_FUNC;
			ret = -EINPROGRESS;
			private->request = request;
			break;
		case 2:
			ret = -EBUSY;
			break;
		default:
			ret = -ENODEV;
		}
		spin_unlock(sch->lock);
		CHSC_MSG(2, "chsc on 0.%x.%04x returned cc=%d\n",
			 sch->schid.ssid, sch->schid.sch_no, cc);
		if (ret == -EINPROGRESS)
			return -EINPROGRESS;
		put_device(&sch->dev);
		if (ret == 0)
			return 0;
	}
	return ret;
}

static void chsc_log_command(struct chsc_async_area *chsc_area)
{
	char dbf[10];

	sprintf(dbf, "CHSC:%x", chsc_area->header.code);
	CHSC_LOG(0, dbf);
	CHSC_LOG_HEX(0, chsc_area, 32);
}

static int chsc_examine_irb(struct chsc_request *request)
{
	int backed_up;

	if (!(scsw_stctl(&request->irb.scsw) & SCSW_STCTL_STATUS_PEND))
		return -EIO;
	backed_up = scsw_cstat(&request->irb.scsw) & SCHN_STAT_CHAIN_CHECK;
	request->irb.scsw.cmd.cstat &= ~SCHN_STAT_CHAIN_CHECK;
	if (scsw_cstat(&request->irb.scsw) == 0)
		return 0;
	if (!backed_up)
		return 0;
	if (scsw_cstat(&request->irb.scsw) & SCHN_STAT_PROG_CHECK)
		return -EIO;
	if (scsw_cstat(&request->irb.scsw) & SCHN_STAT_PROT_CHECK)
		return -EPERM;
	if (scsw_cstat(&request->irb.scsw) & SCHN_STAT_CHN_DATA_CHK)
		return -EAGAIN;
	if (scsw_cstat(&request->irb.scsw) & SCHN_STAT_CHN_CTRL_CHK)
		return -EAGAIN;
	return -EIO;
}

static int chsc_ioctl_start(void __user *user_area)
{
	struct chsc_request *request;
	struct chsc_async_area *chsc_area;
	int ret;
	char dbf[10];

	if (!css_general_characteristics.dynio)
		/* It makes no sense to try. */
		return -EOPNOTSUPP;
	chsc_area = (void *)get_zeroed_page(GFP_DMA | GFP_KERNEL);
	if (!chsc_area)
		return -ENOMEM;
	request = kzalloc(sizeof(*request), GFP_KERNEL);
	if (!request) {
		ret = -ENOMEM;
		goto out_free;
	}
	init_completion(&request->completion);
	if (copy_from_user(chsc_area, user_area, PAGE_SIZE)) {
		ret = -EFAULT;
		goto out_free;
	}
	chsc_log_command(chsc_area);
	spin_lock_irq(&chsc_lock);
	ret = chsc_async(chsc_area, request);
	spin_unlock_irq(&chsc_lock);
	if (ret == -E