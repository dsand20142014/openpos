_device	*p_dev;
	struct pcmcia_device	*tmp;
	unsigned long		flags;

	ds_dev_dbg(2, leftover ? &leftover->dev : &s->dev,
		   "pcmcia_card_remove(%d) %s\n", s->sock,
		   leftover ? leftover->devname : "");

	if (!leftover)
		s->device_count = 0;
	else
		s->device_count = 1;

	/* unregister all pcmcia_devices registered with this socket, except leftover */
	list_for_each_entry_safe(p_dev, tmp, &s->devices_list, socket_device_list) {
		if (p_dev == leftover)
			continue;

		spin_lock_irqsave(&pcmcia_dev_list_lock, flags);
		list_del(&p_dev->socket_device_list);
		p_dev->_removed=1;
		spin_unlock_irqrestore(&pcmcia_dev_list_lock, flags);

		ds_dev_dbg(2, &p_dev->dev, "unregistering device\n");
		device_unregister(&p_dev->dev);
	}

	return;
}

static int pcmcia_device_remove(struct device * dev)
{
	struct pcmcia_device *p_dev;
	struct pcmcia_driver *p_drv;
	struct pcmcia_device_id *did;
	int i;

	p_dev = to_pcmcia_dev(dev);
	p_drv = to_pcmcia_drv(dev->driver);

	ds_dev_dbg(1, dev, "removing device\n");

	/* If we're removing the primary module driving a
	 * pseudo multi-function card, we need to unbind
	 * all devices
	 */
	did = dev_get_drvdata(&p_dev->dev);
	if (did && (did->match_flags & PCMCIA_DEV_ID_MATCH_DEVICE_NO) &&
	    (p_dev->socket->device_count != 0) &&
	    (p_dev->device_no == 0))
		pcmcia_card_remove(p_dev->socket, p_dev);

	/* detach the "instance" */
	if (!p_drv)
		return 0;

	if (p_drv->remove)
	       	p_drv->remove(p_dev);

	p_dev->dev_node = NULL;

	/* check for proper unloading */
	if (p_dev->_irq || p_dev->_io || p_dev->_locked)
		dev_printk(KERN_INFO, dev,
			"pcmcia: driver %s did not release config properly\n",
			p_drv->drv.name);

	for (i = 0; i < MAX_WIN; i++)
		if (p_dev->_win & CLIENT_WIN_REQ(i))
			dev_printk(KERN_INFO, dev,
			  "pcmcia: driver %s did not release window properly\n",
			   p_drv->drv.name);

	/* references from pcmcia_probe_device */
	pcmcia_put_dev(p_dev);
	module_put(p_drv->owner);

	return 0;
}


/*
 * pcmcia_device_query -- determine information about a pcmcia device
 */
static int pcmcia_device_query(struct pcmcia_device *p_dev)
{
	cistpl_manfid_t manf_id;
	cistpl_funcid_t func_id;
	cistpl_vers_1_t	*vers1;
	unsigned int i;

	vers1 = kmalloc(sizeof(*vers1), GFP_KERNEL);
	if (!vers1)
		return -ENOMEM;

	if (!pccard_read_tuple(p_dev->socket, p_dev->func,
			       CISTPL_MANFID, &manf_id)) {
		p_dev->manf_id = manf_id.manf;
		p_dev->card_id = manf_id.card;
		p_dev->has_manf_id = 1;
		p_dev->has_card_id = 1;
	}

	if (!pccard_read_tuple(p_dev->socket, p_dev->func,
			       CISTPL_FUNCID, &func_id)) {
		p_dev->func_id = func_id.func;
		p_dev->has_func_id = 1;
	} else {
		/* rule of thumb: cards with no FUNCID, but with
		 * common memory device geometry information, are
		 * probably memory cards (from pcmcia-cs) */
		cistpl_device_geo_t *devgeo;

		devgeo = kmalloc(sizeof(*devgeo), GFP_KERNEL);
		if (!devgeo) {
			kfree(vers1);
			return -ENOMEM;
		}
		if (!pccard_read_tuple(p_dev->socket, p_dev->func,
				      CISTPL_DEVICE_GEO, devgeo)) {
			ds_dev_dbg(0, &p_dev->dev,
				   "mem device geometry probably means "
				   "FUNCID_MEMORY\n");
			p_dev->func_id = CISTPL_FUNCID_MEMORY;
			p_dev->has_func_id = 1;
		}
		kfree(devgeo);
	}

	if (!pccard_read_tuple(p_dev->socket, p_dev->func, CISTPL_VERS_1,
			       vers1)) {
		for (i=0; i < vers1->ns; i++) {
			char *tmp;
			unsigned int length;

			tmp = vers1->str + vers1->ofs[i];

			length = strlen(tmp) + 1;
			if ((length < 2) || (length > 255))
				continue;

			p_dev->prod_id[i] = kmalloc(sizeof(char) * length,
						    GFP_KERNEL);
			if (!p_dev->prod_id[i])
				continue;

			p_dev->prod_id[i] = strncpy(p_dev->prod_id[i],
						    tmp, length);
		}
	}

	kfree(vers1);
	return 0;
}


/* device_add_lock is needed to avoid double registration by cardmgr and kernel.
 * Serializes pcmcia_device_add; will most likely be removed in future.
 *
 * While it has the caveat that adding new PCMCIA devices inside(!) device_register()
 * won't work, this doesn't matter much at the moment: the driver core doesn't
 * support it either.
 */
static DEFINE_MUTEX(device_add_lock);

struct pcmcia_device * pcmcia_device_add(struct pcmcia_socket *s, unsigned int function)
{
	struct pcmcia_device *p_dev, *tmp_dev;
	unsigned long flags;

	s = pcmcia_get_socket(s);
	if (!s)
		return NULL;

	mutex_lock(&device_add_lock);

	ds_dbg(3, "adding device to %d, function %d\n", s->sock, function);

	/* max of 4 devices per card */
	if (s->device_count == 4)
		goto err_put;

	p_dev = kzalloc(sizeof(struct pcmcia_device), GFP_KERNEL);
	if (!p_dev)
		goto err_put;

	p_dev->socket = s;
	p_dev->device_no = (s->device_count++);
	p_dev->func   = function;

	p_dev->dev.bus = &pcmcia_bus_type;
	p_dev->dev.parent = s->dev.parent;
	p_dev->dev.release = pcmcia_release_dev;
	/* by default don't allow DMA */
	p_dev->dma_mask = DMA_MASK_NONE;
	p_dev->dev.dma_mask = &p_dev->dma_mask;
	dev_set_name(&p_dev->dev, "%d.%d", p_dev->socket->sock, p_dev->device_no);
	if (!dev_name(&p_dev->dev))
		goto err_free;
	p_dev->devname = kasprintf(GFP_KERNEL, "pcmcia%s", dev_name(&p_dev->dev));
	if (!p_dev->devname)
		goto err_free;
	ds_dev_dbg(3, &p_dev->dev, "devname is %s\n", p_dev->devname);

	spin_lock_irqsave(&pcmcia_dev_list_lock, flags);

	/*
	 * p_dev->function_config must be the same for all card functions.
	 * Note that this is serialized by the device_add_lock, so that
	 * only one such struct will be created.
	 */
        list_for_each_entry(tmp_dev, &s->devices_list, socket_device_list)
                if (p_dev->func == tmp_dev->func) {
			p_dev->function_config = tmp_dev->function_config;
			p_dev->io = tmp_dev->io;
			p_dev->irq = tmp_dev->irq;
			kref_get(&p_dev->function_config->ref);
		}

	/* Add to the list in pcmcia_bus_socket */
	list_add(&p_dev->socket_device_list, &s->devices_list);

	spin_unlock_irqrestore(&pcmcia_dev_list_lock, flags);

	if (!p_dev->function_config) {
		ds_dev_dbg(3, &p_dev->dev, "creating config_t\n");
		p_dev->function_config = kzalloc(sizeof(struct config_t),
						 GFP_KERNEL);
		if (!p_dev->function_config)
			goto err_unreg;
		kref_init(&p_dev->function_config->ref);
	}

	dev_printk(KERN_NOTICE, &p_dev->dev,
		   "pcmcia: registering new device %s\n",
		   p_dev->devname);

	pcmcia_device_query(p_dev);

	if (device_register(&p_dev->dev))
		goto err_unreg;

	mutex_unlock(&device_add_lock);

	return p_dev;

 err_unreg:
	spin_lock_irqsave(&pcmcia_dev_list_lock, flags);
	list_del(&p_dev->socket_device_list);
	spin_unlock_irqrestore(&pcmcia_dev_list_lock, flags);

 err_free:
	kfree(p_dev->devname);
	kfree(p_dev);
	s->d