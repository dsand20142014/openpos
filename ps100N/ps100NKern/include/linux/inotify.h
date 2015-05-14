struct pcmcia_socket *skt, int new_cis)
{
	int no_devices = 0;
	int ret = 0;
	unsigned long flags;

	/* must be called with skt_mutex held */
	ds_dev_dbg(0, &skt->dev, "re-scanning socket %d\n", skt->sock);

	spin_lock_irqsave(&pcmcia_dev_list_lock, flags);
	if (list_empty(&skt->devices_list))
		no_devices = 1;
	spin_unlock_irqrestore(&pcmcia_dev_list_lock, flags);

	/* If this is because of a CIS override, start over */
	if (new_cis && !no_devices)
		pcmcia_card_remove(skt, NULL);

	/* if no devices were added for this socket yet because of
	 * missing resource information or other trouble, we need to
	 * do this now. */
	if (no_devices || new_cis) {
		ret = pcmcia_card_add(skt);
		if (ret)
			return;
	}

	/* some device information might have changed because of a CIS
	 * update or because we can finally read it correctly... so
	 * determine it again, overwriting old values if necessary. */
	bus_for_each_dev(&pcmcia_bus_type, NULL, NULL, pcmcia_requery);

	/* we re-scan all devices, not just the ones connected to this
	 * socket. This does not matter, though. */
	ret = bus_rescan_devices(&pcmcia_bus_type);
	if (ret)
		printk(KERN_INFO "pcmcia: bus_rescan_devices failed\n");
}

#ifdef CONFIG_PCMCIA_LOAD_CIS

/**
 * pcmcia_load_firmware - load CIS from userspace if device-provided is broken
 * @dev: the pcmcia device which needs a CIS override
 * @filename: requested filename in /lib/firmware/
 *
 * This uses the in-kernel firmware loading mechanism to use a "fake CIS" if
 * the one provided by the card is broken. The firmware files reside in
 * /lib/firmware/ in userspace.
 */
static int pcmcia_load_firmware(struct pcmcia_device *dev, char * filename)
{
	struct pcmcia_socket *s = dev->socket;
	const struct firmware *fw;
	int ret = -ENOMEM;
	int no_funcs;
	int old_funcs;
	cistpl_longlink_mfc_t mfc;

	if (!filename)
		return -EINVAL;

	ds_dev_dbg(1, &dev->dev, "trying to load CIS file %s\n", filename);

	if (request_firmware(&fw, filename, &dev->dev) == 0) {
		if (fw->size >= CISTPL_MAX_CIS_SIZE) {
			ret = -EINVAL;
			dev_printk(KERN_ERR, &dev->dev,
				   "pcmcia: CIS override is too big\n");
			goto release;
		}

		if (!pcmcia_replace_cis(s, fw->data, fw->size))
			ret = 0;
		else {
			dev_printk(KERN_ERR, &dev->dev,
				   "pcmcia: CIS override failed\n");
			goto release;
		}


		/* update information */
		pcmcia_device_query(dev);

		/* does this cis override add or remove functions? */
		old_funcs = s->functions;

		if (!pccard_read_tuple(s, BIND_FN_ALL, CISTPL_LONGLINK_MFC, &mfc))
			no_funcs = mfc.nfn;
		else
			no_funcs = 1;
		s->functions = no_funcs;

		if (old_funcs > no_funcs)
			pcmcia_card_remove(s, dev);
		else if (no_funcs > old_funcs)
			pcmcia_add_device_later(s, 1);
	}
 release:
	release_firmware(fw);

	return (ret);
}

#else /* !CONFIG_PCMCIA_LOAD_CIS */

static inline int pcmcia_load_firmware(struct pcmcia_device *dev, char * filename)
{
	return -ENODEV;
}

#endif


static inline int pcmcia_devmatch(struct pcmcia_device *dev,
				  struct pcmcia_device_id *did)
{
	if (did->match_flags & PCMCIA_DEV_ID_MATCH_MANF_ID) {
		if ((!dev->has_manf_id) || (dev->manf_id != did->manf_id))
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_CARD_ID) {
		if ((!dev->has_card_id) || (dev->card_id != did->card_id))
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_FUNCTION) {
		if (dev->func != did->function)
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_PROD_ID1) {
		if (!dev->prod_id[0])
			return 0;
		if (strcmp(did->prod_id[0], dev->prod_id[0]))
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_PROD_ID2) {
		if (!dev->prod_id[1])
			return 0;
		if (strcmp(did->prod_id[1], dev->prod_id[1]))
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_PROD_ID3) {
		if (!dev->prod_id[2])
			return 0;
		if (strcmp(did->prod_id[2], dev->prod_id[2]))
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_PROD_ID4) {
		if (!dev->prod_id[3])
			return 0;
		if (strcmp(did->prod_id[3], dev->prod_id[3]))
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_DEVICE_NO) {
		if (dev->device_no != did->device_no)
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_FUNC_ID) {
		if ((!dev->has_func_id) || (dev->func_id != did->func_id))
			return 0;

		/* if this is a pseudo-multi-function device,
		 * we need explicit matches */
		if (did->match_flags & PCMCIA_DEV_ID_MATCH_DEVICE_NO)
			return 0;
		if (dev->device_no)
			return 0;

		/* also, FUNC_ID matching needs to be activated by userspace
		 * after it has re-checked that there is no possible module
		 * with a prod_id/manf_id/card_id match.
		 */
		ds_dev_dbg(0, &dev->dev,
			"skipping FUNC_ID match until userspace interaction\n");
		if (!dev->allow_func_id_match)
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_FAKE_CIS) {
		ds_dev_dbg(0, &dev->dev, "device needs a fake CIS\n");
		if (!dev->socket->fake_cis)
			pcmcia_load_firmware(dev, did->cisfile);

		if (!dev->socket->fake_cis)
			return 0;
	}

	if (did->match_flags & PCMCIA_DEV_ID_MATCH_ANONYMOUS) {
		int i;
		for (i=0; i<4; i++)
			if (dev->prod_id[i])
				return 0;
		if (dev->has_manf_id || dev->has_card_id || dev->has_func_id)
			return 0;
	}

	dev_set_drvdata(&dev->dev, did);

	return 1;
}


static int pcmcia_bus_match(struct device * dev, struct device_driver * drv) {
	struct pcmcia_device * p_dev = to_pcmcia_dev(dev);
	struct pcmcia_driver * p_drv = to_pcmcia_drv(drv);
	struct pcmcia_device_id *did = p_drv->id_table;
	struct pcmcia_dynid *dynid;

	/* match dynamic devices first */
	spin_lock(&p_drv->dynids.lock);
	list_for_each_entry(dynid, &p_drv->dynids.list, node) {
		ds_dev_dbg(3, dev, "trying to match to %s\n", drv->name);
		if (pcmcia_devmatch(p_dev, &dynid->id)) {
			ds_dev_dbg(0, dev, "matched to %s\n", drv->name);
			spin_unlock(&p_drv->dynids.lock);
			return 1;
		}
	}
	spin_unlock(&p_drv->dynids.lock);

#ifdef CONFIG_PCMCIA_IOCTL
	/* matching by cardmgr */
	if (p_dev->cardmgr == p_drv) {
		ds_dev_dbg(0, dev, "cardmgr matched to %s\n", drv->name);
		return 1;
	}
#endif

	while (did && did->match_flags) {
		ds_dev_dbg(3, dev, "trying to match to %s\n", drv->name);
		if (pcmcia_devmatch(p_dev, did)) {
			ds_dev_dbg(0, dev, "matched to %s\n", drv->name);
			return 1;
		}
		did++;
	}

	return 0;
}

#ifdef CONFIG_HOTPLUG

static int pcmcia_bus_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct pcmcia_device *p_dev;
	int i;
	u32 hash[4] = { 0, 0, 0, 0};

	if (!dev)
		return -ENODEV;

	p_dev = to_pcmcia_dev(dev);

	/* calculate hashes */
	for (i=0; i<4; i++) {
		if (!p_dev->prod_id[i])
			continue;
		hash[i] = crc32(0, p_dev->prod_id[i], strlen(p_dev->prod_id[i]));
	}

	if (add_uevent_var(env, "SOCKET_NO=%u", p_dev->socket->sock))
		return -ENOMEM;

	if (add_uevent_var(env, "DEVICE_NO=%02X", p_dev->device_no))
		return -ENOMEM;

	if (add_uevent_var(env, "MODALIAS=pcmcia:m%04Xc%04Xf%02Xfn%02Xpfn%02X"
			   "pa%08Xpb%08Xpc%08Xpd%08X",
			   p_dev->has_manf_id ? p_dev->manf_id : 0,
			   p_dev->has_card_id ? p_dev->card_id : 0,
			   p_dev->has_func_id ? p_dev->func_id : 0,
			   p_dev->func,
			   p_dev->device_no,
			   hash[0],
			   hash[1],
			   hash[2],
			   hash[3]))
		return -ENOMEM;

	return 0;
}

#else

static int pcmcia_bus_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return -ENODEV;
}

#endif

/************************ runtime PM support ***************************/

static int pcmcia_dev_suspend(struct device *dev, pm_message_t state);
static int pcmcia_dev_resume(struct device *dev);

static int runtime_suspend(struct device *dev)
{
	int rc;

	down(&dev->sem);
	rc = pcmcia_dev_suspend(dev, PMSG_SUSPEND);
	up(&de