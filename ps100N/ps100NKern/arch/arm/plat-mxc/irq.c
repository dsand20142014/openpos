= u132_read_pcimem(u132, control, &control);
	if (retval)
		return retval;
	mdelay((roothub_a >> 23) & 0x1fe);
	u132_to_hcd(u132)->state = HC_STATE_RUNNING;
	return 0;
}

static void u132_hcd_stop(struct usb_hcd *hcd)
{
	struct u132 *u132 = hcd_to_u132(hcd);
	if (u132->going > 1) {
		dev_err(&u132->platform_dev->dev, "u132 device %p(hcd=%p) has b"
			"een removed %d\n", u132, hcd, u132->going);
	} else if (u132->going > 0) {
		dev_err(&u132->platform_dev->dev, "device hcd=%p is being remov"
			"ed\n", hcd);
	} else {
		mutex_lock(&u132->sw_lock);
		msleep(100);
		u132_power(u132, 0);
		mutex_unlock(&u132->sw_lock);
	}
}

static int u132_hcd_start(struct usb_hcd *hcd)
{
	struct u132 *u132 = hcd_to_u132(hcd);
	if (u132->going > 1) {
		dev_err(&u132->platform_dev->dev, "device has been removed %d\n"
			, u132->going);
		return -ENODEV;
	} else if (u132->going > 0) {
		dev_err(&u132->platform_dev->dev, "device is being removed\n");
		return -ESHUTDOWN;
	} else if (hcd->self.controller) {
		int retval;
		struct platform_device *pdev =
			to_platform_device(hcd->self.controller);
		u16 vendor = ((struct u132_platform_data *)
			(pdev->dev.platform_data))->vendor;
		u16 device = ((struct u132_platform_data *)
			(pdev->dev.platform_data))->device;
		mutex_lock(&u132->sw_lock);
		msleep(10);
		if (vendor == PCI_VENDOR_ID_AMD && device == 0x740c) {
			u132->flags = OHCI_QUIRK_AMD756;
		} else if (vendor == PCI_VENDOR_ID_OPTI && device == 0xc861) {
			dev_err(&u132->platform_dev->dev, "WARNING: OPTi workar"
				"ounds unavailable\n");
		} else if (vendor == PCI_VENDOR_ID_COMPAQ && device == 0xa0f8)
			u132->flags |= OHCI_QUIRK_ZFMICRO;
		retval = u132_run(u132);
		if (retval) {
			u132_disable(u132);
			u132->going = 1;
		}
		msleep(100);
		mutex_unlock(&u132->sw_lock);
		return retval;
	} else {
		dev_err(&u132->platform_dev->dev, "platform_device missing\n");
		return -ENODEV;
	}
}

static int u132_hcd_reset(struct usb_hcd *hcd)
{
	struct u132 *u132 = hcd_to_u132(hcd);
	if (u132->going > 1) {
		dev_err(&u132->platform_dev->dev, "device has been removed %d\n"
			, u132->going);
		return -ENODEV;
	} else if (u132->going > 0) {
		dev_err(&u132->platform_dev->dev, "device is being removed\n");
		return -ESHUTDOWN;
	} else {
		int retval;
		mutex_lock(&u132->sw_lock);
		retval = u132_init(u132);
		if (retval) {
			u132_disable(u132);
			u132->going = 1;
		}
		mutex_unlock(&u132->sw_lock);
		return retval;
	}
}

static int create_endpoint_and_queue_int(struct u132 *u132,
	struct u132_udev *udev, struct urb *urb,
	struct usb_device *usb_dev, u8 usb_addr, u8 usb_endp, u8 address,
	gfp_t mem_flags)
{
	struct u132_ring *ring;
	unsigned long irqs;
	int rc;
	u8 endp_number;
	struct u132_endp *endp = kmalloc(sizeof(struct u132_endp), mem_flags);

	if (!endp)
		return -ENOMEM;

	spin_lock_init(&endp->queue_lock.slock);
	spin_lock_irqsave(&endp->queue_lock.slock, irqs);
	rc = usb_hcd_link_urb_to_ep(u132_to_hcd(u132), urb);
	if (rc) {
		spin_unlock_irqrestore(&endp->queue_lock.slock, irqs);
		kfree(endp);
		return rc;
	}

	endp_number = ++u132->num_endpoints;
	urb->ep->hcpriv = u132->endp[endp_number - 1] = endp;
	INIT_DELAYED_WORK(&endp->scheduler, u132_hcd_endp_work_scheduler);
	INIT_LIST_HEAD(&endp->urb_more);
	ring = endp->ring = &u132->ring[0];
	if (ring->curr_endp) {
		list_add_tail(&endp->endp_ring, &ring->curr_endp->endp_ring);
	} else {
		INIT_LIST_HEAD(&endp->endp_ring);
		ring->curr_endp = endp;
	}
	ring->length += 1;
	endp->dequeueing = 0;
	endp->edset_flush = 0;
	endp->active = 0;
	endp->delayed = 0;
	endp->endp_number = endp_number;
	endp->u132 = u132;
	endp->hep = urb->ep;
	endp->pipetype = usb_pipetype(urb->pipe);
	u132_endp_init_kref(u132, endp);
	if (usb_pipein(urb->pipe)) {
		endp->toggle_bits = 0x2;
		usb_settoggle(udev->usb_device, usb_endp, 0, 0);
		endp->input = 1;
		endp->output = 0;
		udev->endp_number_in[usb_endp] = endp_number;
		u132_udev_get_kref(u132, udev);
	} else {
		endp->toggle_bits = 0x2;
		usb_settoggle(udev->usb_device, usb_endp, 1, 0);
		endp->input = 0;
		endp->output = 1;
		udev->endp_number_out[usb_endp] = endp_number;
		u132_udev_get_kref(u132, udev);
	}
	urb->hcpriv = u132;
	endp->delayed = 1;
	endp->jiffies = jiffies + msecs_to_jiffies(urb->interval);
	endp->udev_number = address;
	endp->usb_addr = usb_addr;
	endp->usb_endp = usb_endp;
	endp->queue_size = 1;
	endp->queue_last = 0;
	endp->queue_next = 0;
	endp->urb_list[ENDP_QUEUE_MASK & endp->queue_last++] = urb;
	spin_unlock_irqrestore(&endp->queue_lock.slock, irqs);
	u132_endp_queue_work(u132, endp, msecs_to_jiffies(urb->interval));
	return 0;
}

static int queue_int_on_old_endpoint(struct u132 *u132,
	struct u132_udev *udev, struct urb *urb,
	struct usb_device *usb_dev, struct u132_endp *endp, u8 usb_addr,
	u8 usb_endp, u8 address)
{
	urb->hcpriv = u132;
	endp->delayed = 1;
	endp->jiffies = jiffies + msecs_to_jiffies(urb->interval);
	if (endp->queue_size++ < ENDP_QUEUE_SIZE) {
		endp->urb_list[ENDP_QUEUE_MASK & endp->queue_last++] = urb;
	} else {
		struct u132_urbq *urbq = kmalloc(sizeof(struct u132_urbq),
			GFP_ATOMIC);
		if (urbq == NULL) {
			endp->queue_size -= 1;
			return -ENOMEM;
		} else {
			list_add_tail(&urbq->urb_more, &endp->urb_more);
			urbq->urb = urb;
		}
	}
	return 0;
}

static int create_endpoint_and_queue_bulk(struct u132 *u132,
	struct u132_udev *udev, struct urb *urb,
	struct usb_device *usb_dev, u8 usb_addr, u8 usb_endp, u8 address,
	gfp_t mem_flags)
{
	int ring_number;
	struct u132_ring *ring;
	unsigned long irqs;
	int rc;
	u8 endp_number;
	struct u132_endp *endp = kmalloc(sizeof(struct u132_endp), mem_flags);

	if (!endp)
		return -ENOMEM;

	spin_lock_init(&endp->queue_lock.slock);
	spin_lock_irqsave(&endp->queue_lock.slock, irqs);
	rc = usb_hcd_link_urb_to_ep(u132_to_hcd(u132), urb);
	if (rc) {
		spin_unlock_irqrestore(&endp->queue_lock.slock, irqs);
		kfree(endp);
		return rc;
	}

	endp_number = ++u132->num_endpoints;
	urb->ep->hcpriv = u132->endp[endp_number - 1] = endp;
	INIT_DELAYED_WORK(&endp->scheduler, u132_hcd_endp_work_scheduler);
	INIT_LIST_HEAD(&endp->urb_more);
	endp->dequeueing = 0;
	endp->edset_flush = 0;
	endp->active = 0;
	endp->delayed = 0;
	endp->endp_number = endp_number;
	endp->u132 = u132;
	endp->hep = urb->ep;
	endp->pipetype = usb_pipetype(urb->pipe);
	u132_endp_init_kref(u132, endp);
	if (usb_pipein(urb->pipe)) {
		endp->toggle_bits = 0x2;
		usb_settoggle(udev->usb_device, usb_endp, 0, 0);
		ring_number = 3;
		endp->input = 1;
		endp->output = 0;
		udev->endp_number_in[usb_endp] = endp_number;
		u132_udev_get_kref(u132, udev);
	} else {
		endp->toggle_bits = 0x2;
		usb_settoggle(udev->usb_device, usb_endp, 1, 0);
		ring_number = 2;
		endp->input = 0;
		endp->output = 1;
		udev->endp_number_out[usb_endp] = endp_number;
		u132_udev_get_kref(u132, udev);
	}
	ring = endp->ring = &u132->ring[ring_number - 1];
	if (ring->curr_endp) {
		list_add_tail(&endp->endp_ring, &ring->curr_endp->endp_ring);
	} else {
		INIT_LIST_HEAD(&endp->endp_ring);
		ring->curr_endp = endp;
	}
	ring->length += 1;
	urb->hcpriv = u132;
	endp->udev_number = address;
	endp->usb_addr = usb_addr;
	endp->usb_endp = usb_endp;
	endp->queue_size = 1;
	endp->queue_last = 0;
	endp->queue_next = 0;
	endp->urb_list[ENDP_QUEUE_MASK & endp->queue_last++] = urb;
	spin_unlock_irqrestore(&endp->queue_lock.slock, irqs);
	u132_endp_queue_work(u132, endp, 0);
	return 0;
}

static int queue_bulk_on_old_endpoint(struct u132 *u132, struct u132_udev *udev,
	struct urb *urb,
	struct usb_device *usb_dev, struct u132_endp *endp, u8 usb_addr,
	u8 usb_endp, u8 address)
{
	urb->hcpriv = u132;
	if (endp->queue_size++ < ENDP_QUEUE_SIZE) {
		endp->urb_list[ENDP_QUEUE_MASK & endp->queue_last++] = urb;
	} else {
		struct u132_urbq *urbq = kmalloc(sizeof(struct u132_urbq),
			GFP_ATOMIC);
		if (urbq == NULL) {
			endp->queue_size -= 1;
			return -ENOMEM;
		} else {
			list_add_tail(&urbq->urb_more, &endp->urb_more);
			urbq->urb = urb;
		}
	}
	return 0;
}

static int create_endpoint_and_queue_control(struct u132 *u132,
	struct urb *urb,
	struct usb_device *usb_dev, u8 usb_addr, u8 usb_endp,
	gfp_t mem_flags)
{
	struct u132_ring *ring;
	unsigned long irqs;
	int rc;
	u8 endp_number;
	struct u132_endp *endp = kmalloc(sizeof(struct u132_endp), mem_flags);

	if (!endp)
		return -ENOMEM;

	spin_lock_init(&endp->queue_lock.slock);
	spin_lock_irqsave(&endp->queue_lock.slock, irqs);
	rc = usb_hcd_link_urb_to_ep(u132_to_hcd(u132), urb);
	if (rc) {
		spin_unlock_irqrestore(&endp->queue_lock.slo