e */
	if (dev->mii.mdio_read)
		return mii_link_ok(&dev->mii);

	/* Otherwise, dtrt for drivers calling netif_carrier_{on,off} */
	return ethtool_op_get_link(net);
}
EXPORT_SYMBOL_GPL(usbnet_get_link);

int usbnet_nway_reset(struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);

	if (!dev->mii.mdio_write)
		return -EOPNOTSUPP;

	return mii_nway_restart(&dev->mii);
}
EXPORT_SYMBOL_GPL(usbnet_nway_reset);

void usbnet_get_drvinfo (struct net_device *net, struct ethtool_drvinfo *info)
{
	struct usbnet *dev = netdev_priv(net);

	strncpy (info->driver, dev->driver_name, sizeof info->driver);
	strncpy (info->version, DRIVER_VERSION, sizeof info->version);
	strncpy (info->fw_version, dev->driver_info->description,
		sizeof info->fw_version);
	usb_make_path (dev->udev, info->bus_info, sizeof info->bus_info);
}
EXPORT_SYMBOL_GPL(usbnet_get_drvinfo);

u32 usbnet_get_msglevel (struct net_device *net)
{
	struct usbnet *dev = netdev_priv(net);

	return dev->msg_enable;
}
EXPORT_SYMBOL_GPL(usbnet_get_msglevel);

void usbnet_set_msglevel (struct net_device *net, u32 level)
{
	struct usbnet *dev = netdev_priv(net);

	dev->msg_enable = level;
}
EXPORT_SYMBOL_GPL(usbnet_set_msglevel);

/* drivers may override default ethtool_ops in their bind() routine */
static struct ethtool_ops usbnet_ethtool_ops = {
	.get_settings		= usbnet_get_settings,
	.set_settings		= usbnet_set_settings,
	.get_link		= usbnet_get_link,
	.nway_reset		= usbnet_nway_reset,
	.get_drvinfo		= usbnet_get_drvinfo,
	.get_msglevel		= usbnet_get_msglevel,
	.set_msglevel		= usbnet_set_msglevel,
};

/*-------------------------------------------------------------------------*/

/* work that cannot be done in interrupt context uses keventd.
 *
 * NOTE:  with 2.5 we could do more of this using completion callbacks,
 * especially now that control transfers can be queued.
 */
static void
kevent (struct work_struct *work)
{
	struct usbnet		*dev =
		container_of(work, struct usbnet, kevent);
	int			status;

	/* usb_clear_halt() needs a thread context */
	if (test_bit (EVENT_TX_HALT, &dev->flags)) {
		unlink_urbs (dev, &dev->txq);
		status = usb_clear_halt (dev->udev, dev->out);
		if (status < 0
				&& status != -EPIPE
				&& status != -ESHUTDOWN) {
			if (netif_msg_tx_err (dev))
				deverr (dev, "can't clear tx halt, status %d",
					status);
		} else {
			clear_bit (EVENT_TX_HALT, &dev->flags);
			if (status != -ESHUTDOWN)
				netif_wake_queue (dev->net);
		}
	}
	if (test_bit (EVENT_RX_HALT, &dev->flags)) {
		unlink_urbs (dev, &dev->rxq);
		status = usb_clear_halt (dev->udev, dev->in);
		if (status < 0
				&& status != -EPIPE
				&& status != -ESHUTDOWN) {
			if (netif_msg_rx_err (dev))
				deverr (dev, "can't clear rx halt, status %d",
					status);
		} else {
			clear_bit (EVENT_RX_HALT, &dev->flags);
			tasklet_schedule (&dev->bh);
		}
	}

	/* tasklet could resubmit itself forever if memory is tight */
	if (test_bit (EVENT_RX_MEMORY, &dev->flags)) {
		struct urb	*urb = NULL;

		if (netif_running (dev->net))
			urb = usb_alloc_urb (0, GFP_KERNEL);
		else
			clear_bit (EVENT_RX_MEMORY, &dev->flags);
		if (urb != NULL) {
			clear_bit (EVENT_RX_MEMORY, &dev->flags);
			rx_submit (dev, urb, GFP_KERNEL);
			tasklet_schedule (&dev->bh);
		}
	}

	if (test_bit (EVENT_LINK_RESET, &dev->flags)) {
		struct driver_info	*info = dev->driver_info;
		int			retval = 0;

		clear_bit (EVENT_LINK_RESET, &dev->flags);
		if(info->link_reset && (retval = info->link_reset(dev)) < 0) {
			devinfo(dev, "link reset failed (%d) usbnet usb-%s-%s, %s",
				retval,
				dev->udev->bus->bus_name, dev->udev->devpath,
				info->description);
		}
	}

	if (dev->flags)
		devdbg (dev, "kevent done, flags = 0x%lx",
			dev->flags);
}

/*-------------------------------------------------------------------------*/

static void tx_complete (struct urb *urb)
{
	struct sk_buff		*skb = (struct sk_buff *) urb->context;
	struct skb_data		*entry = (struct skb_data *) skb->cb;
	struct usbnet		*dev = entry->dev;

	if (urb->status == 0) {
		dev->net->stats.tx_packets++;
		dev->net->stats.tx_bytes += entry->length;
	} else {
		dev->net->stats.tx_errors++;

		switch (urb->status) {
		case -EPIPE:
			usbnet_defer_kevent (dev, EVENT_TX_HALT);
			break;

		/* software-driven interface shutdown */
		case -ECONNRESET:		// async unlink
		case -ESHUTDOWN:		// hardware gone
			break;

		// like rx, tx gets controller i/o faults during khubd delays
		// and so it uses the same throttling mechanism.
		case -EPROTO:
		case -ETIME:
		case -EILSEQ:
			if (!timer_pending (&dev->delay)) {
				mod_timer (&dev->delay,
					jiffies + THROTTLE_JIFFIES);
				if (netif_msg_link (dev))
					devdbg (dev, "tx throttle %d",
							urb->status);
			}
			netif_stop_queue (dev->net);
			break;
		default:
			if (netif_msg_tx_err (dev))
				devdbg (dev, "tx err %d", entry->urb->status);
			break;
		}
	}

	urb->dev = NULL;
	entry->state = tx_done;
	defer_bh(dev, skb, &dev->txq);
}

/*-------------------------------------------------------------------------*/

void usbnet_tx_timeout (struct net_device *net)
{
	struct usbnet		*dev = netdev_priv(net);

	unlink_urbs (dev, &dev->txq);
	tasklet_schedule (&dev->bh);

	// FIXME: device recovery -- reset?
}
EXPORT_SYMBOL_GPL(usbnet_tx_timeout);

/*-------------------------------------------------------------------------*/

int usbnet_start_xmit (struct sk_buff *skb, struct net_device *net)
{
	struct usbnet		*dev = netdev_priv(net);
	int			length;
	int			retval = NET_XMIT_SUCCESS;
	struct urb		*urb = NULL;
	struct skb_data		*entry;
	struct driver_info	*info = dev->driver_info;
	unsigned long		flags;

	// some devices want funky USB-level framing, for
	// win32 driver (usually) and/or hardware quirks
	if (info->tx_fixup) {
		skb = info->tx_fixup (dev, skb, GFP_ATOMIC);
		if (!skb) {
			if (netif_msg_tx_err (dev))
				devdbg (dev, "can't tx_fixup skb");
			goto drop;
		}
	}
	length = skb->len;

	if (!(urb = usb_alloc_urb (0, GFP_ATOMIC))) {
		if (netif_msg_tx_err (dev))
			devdbg (dev, "no urb");
		goto drop;
	}

	entry = (struct skb_data *) skb->cb;
	entry->urb = urb;
	entry->dev = dev;
	entry->state = tx_start;
	entry->length = length;

	usb_fill_bulk_urb (urb, dev->udev, dev->out,
			skb->data, skb->len, tx_complete, skb);

	/* don't assume the hardware handles USB_ZERO_PACKET
	 * NOTE:  strictly conforming cdc-ether devices should expect
	 * the ZLP here, but ignore the one-byte packet.
	 */
	if ((length % dev->maxpacket) == 0) {
		urb->transfer_buffer_length++;
		if (skb_tailroom(skb)) {
			skb->data[skb->len] = 0;
			__skb_put(skb, 1);
		}
	}

	spin_lock_irqsave (&dev->txq.lock, flags);

	switch ((retval = usb_submit_urb (urb, GFP_ATOMIC))) {
	case -EPIPE:
		netif_stop_queue (net);
		usbnet_defer_kevent (dev, EVENT_TX_HALT);
		break;
	default:
		if (netif_msg_tx_err (dev))
			devdbg (dev, "tx: submit urb err %d", retval);
		break;
	case 0:
		net->trans_start = jiffies;
		__skb_queue_tail (&dev->txq, skb);
		if (dev->txq.qlen >= TX_QLEN (dev))
			netif_stop_queue (net);
	}
	spin_unlock_irqrestore (&dev->txq.lock, flags);

	if (retval) {
		if (netif_msg_tx_err (dev))
			devdbg (dev, "drop, code %d", retval);
drop:
		retval = NET_XMIT_SUCCESS;
		dev->net->stats.tx_dropped++;
		if (skb)
			dev_kfree_skb_any (skb);
		usb_free_urb (urb);
	} else if (netif_msg_tx_queued (dev)) {
		devdbg (dev, "> tx, len %d, type 0x%x",
			length, skb->protocol);
	}
	return retval;
}
EXPORT_SYMBOL_GPL(usbnet_start_xmit);

/*-------------------------------------------------------------------------*/

// tasklet (work deferred from completions, in_irq) or timer

static void usbnet_bh (unsigned long param)
{
	struct usbnet		*dev = (struct usbnet *) param;
	struct sk_buff		*skb;
	struct skb_data		*entry;

	while ((skb = skb_dequeue (&dev->done))) {
		entry = (struct skb_data *) skb->cb;
		switch (entry->state) {
		case rx_done:
			entry->state = rx_cleanup;
			rx_process (dev, skb);
			continue;
		case tx_done:
		case rx_cleanup:
			usb_free_urb (entry->urb);
			dev_kfree_skb (skb);
			continue;
		default:
			devdbg (dev, "bogus skb state %d", entry->state);
		}
	}

	// waiting for all pending urbs to complete?
	if (dev->wait) {
		if ((dev->txq.qlen + dev->rxq.qlen + dev->done.qlen) == 0) {
			wake_up (dev->wait);
		}

	// or are we maybe short a few urbs?
	} else if (netif_running (dev->net)
			&& netif_device_present (dev->net)
			&& !timer_pending (&dev->delay)
			&& !test_bit (EVENT_RX_HALT, &dev->flags)) {
		int	temp = dev->rxq.qlen;
		int	qlen = RX_QLEN (dev);

		if (temp < qlen) {
			struct urb	*urb;
			int		i;

			// don't refill the queue all at once
			for (i = 0; i < 10 && dev->rxq.qlen < qlen; i++) {
				urb = usb_alloc_urb (0, GFP_ATOMIC);
				if (urb != NULL)
					rx_submit (dev, urb, GFP_ATOMIC);
			}
			if (temp != dev->rxq.qlen && netif_msg_link (dev))
				devdbg (dev, "rxqlen %d --> %d",
						temp, dev->rxq.qlen);
			if (dev->rxq.qlen < qlen)
				tasklet_schedule (&dev->bh);
		}
		if (dev->txq.qlen < TX_QLEN (dev))
			netif_wake_queue (dev->net);
	}
}



/*-------------------------------------------------------------------------
 *
 * USB Device Driver support
 *
 *-------------------------------------------------------------------------*/

// precondition: never called in_interrupt

void usbnet_disconnect (struct usb_interface *intf)
{
	struct usbnet		*dev;
	struct usb_device	*xdev;
	struct net_device	*net;

	dev = usb_get_intfdata(intf);
	usb_set_intfdata(intf, NULL);
	if (!dev)
		return;

	xdev = interface_to_usbdev (intf);

	if (netif_msg_probe (dev))
		devinfo (dev, "unregister '%s' usb-%s-%s, %s",
			intf->dev.driver->name,
			xdev->bus->bus_name, xdev->devpath,
			dev->driver_info->description);

	net = dev->net;
	unregister_netdev (net);

	/* we don't hold rtnl here ... */
	flush_scheduled_work ();

	if (dev->driver_info->unbind)
		dev->driver_info->unbind (dev, intf);

	free_netdev(net);
	usb_put_dev (xdev);
}
EXPORT_SYMBOL_GPL(usbnet_disconnect);

static const struct net_device_ops usbnet_netdev_ops = {
	.ndo_open		= usbnet_open,
	.ndo_stop		= usbnet_stop,
	.ndo_start_xmit		= usbnet_start_xmit,
	.ndo_tx_timeout		= usbnet_tx_timeout,
	.ndo_change_mtu		= usbnet_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

/*-------------------------------------------------------------------------*/

// precondition: never called in_interrupt

int
usbnet_probe (struct usb_interface *udev, const struct usb_device_id *prod)
{
	struct usbnet			*dev;
	struct net_device		*net;
	struct usb_host_interface	*interface;
	struct driver_info		*info;
	struct usb_device		*xdev;
	int				status;
	const char			*name;

	name = udev->dev.driver->name;
	info = (struct driver_info *) prod->driver_info;
	if (!info) {
		dev_dbg (&udev->dev, "blacklisted by %s\n", name);
		return -ENODEV;
	}
	xdev = interface_to_usbdev (udev);
	interface = udev->cur_altsetting;

	usb_get_dev (xdev);

	status = -ENOMEM;

	// set up our own records
	net = alloc_etherdev(sizeof(*dev));
	if (!net) {
		dbg ("can't kmalloc dev");
		goto out;
	}

	dev = netdev_priv(net);
	dev->udev = xdev;
	dev->intf = udev;
	dev->driver_info = info;
	dev->driver_name = name;
	dev->msg_enable = netif_msg_init (msg_level, NETIF_MSG_DRV
				| NETIF_MSG_PROBE | NETIF_MSG_LINK);
	skb_queue_head_init (&dev->rxq);
	skb_queue_head_init (&dev->txq);
	skb_queue_head_init (&dev->done);
	dev->bh.func = usbnet_bh;
	dev->bh.data = (unsigned long) dev;
	INIT_WORK (&dev->kevent, kevent);
	dev->delay.function = usbnet_bh;
	dev->delay.data = (unsigned long) dev;
	init_timer (&dev->delay);
	mutex_init (&dev->phy_mutex);

	dev->net = net;
	strcpy (net->name, "usb%d");
	memcpy (net->dev_addr, node_id, sizeof node_id);

	/* rx and tx sides can use different message sizes;
	 * bind() should set rx_urb_size in that case.
	 */
	dev->hard_mtu = net->mtu + net->hard_header_len;
#if 0
// dma_supported() is deeply broken on almost all architectures
	// possible with some EHCI controllers
	if (dma_supported (&udev->dev, DMA_BIT_MASK(64)))
		net->features |= NETIF_F_HIGHDMA;
#endif

	net->netdev_ops = &usbnet_netdev_ops;
	net->watchdog_timeo = TX_TIMEOUT_JIFFIES;
	net->ethtool_ops = &usbnet_ethtool_ops;

	// allow device-specific bind/init procedures
	// NOTE net->name still not usable ...
	if (info->bind) {
		status = info->bind (dev, udev);
		if (status < 0)
			goto out1;

		// heuristic:  "usb%d" for links we know are two-host,
		// else "eth%d" when there's reasonable doubt.  userspace
		// can rename the link if it knows better.
		if ((dev->driver_info->flags & FLAG_ETHER) != 0
				&& (net->dev_addr [0] & 0x02) == 0)
			strcpy (net->name, "eth%d");
		/* WLAN devices should always be named "wlan%d" */
		if ((dev->driver_info->flags & FLAG_WLAN) != 0)
			strcpy(net->name, "wlan%d");

		/* maybe the remote can't receive an Ethernet MTU */
		if (net->mtu > (dev->hard_mtu - net->hard_header_len))
			net->mtu = dev->hard_mtu - net->hard_header_len;
	} else if (!info->in || !info->out)
		status = usbnet_get_endpoints (dev, udev);
	else {
		dev->in = usb_rcvbulkpipe (xdev, info->in);
		dev->out = usb_sndbulkpipe (xdev, info->out);
		if (!(info->flags & FLAG_NO_SETINT))
			status = usb_set_interface (xdev,
				interface->desc.bInterfaceNumber,
				interface->desc.bAlternateSetting);
		else
			status = 0;

	}
	if (status >= 0 && dev->status)
		status = init_status (dev, udev);
	if (status < 0)
		goto out3;

	if (!dev->rx_urb_size)
		dev->rx_urb_size = dev->hard_mtu;
	dev->maxpacket = usb_maxpacket (dev->udev, dev->out, 1);

	SET_NETDEV_DEV(net, &udev->dev);
	status = register_netdev (net);
	if (status)
		goto out3;
	if (netif_msg_probe (dev))
		devinfo (dev, "register '%s' at usb-%s-%s, %s, %pM",
			udev->dev.driver->name,
			xdev->bus->bus_name, xdev->devpath,
			dev->driver_info->description,
			net->dev_addr);

	// ok, it's ready to go.
	usb_set_intfdata (udev, dev);

	// start as if the link is up
	netif_device_attach (net);

	return 0;

out3:
	if (info->unbind)
		info->unbind (dev, udev);
out1:
	free_netdev(net);
out:
	usb_put_dev(xdev);
	return status;
}
EXPORT_SYMBOL_GPL(usbnet_probe);

/*-------------------------------------------------------------------------*/

/*
 * suspend the whole driver as soon as the first interface is suspended
 * resume only when the last interface is resumed
 */

int usbnet_suspend (struct usb_interface *intf, pm_message_t message)
{
	struct usbnet		*dev = usb_get_intfdata(intf);

	if (!dev->suspend_count++) {
		/*
		 * accelerate emptying of the rx and queues, to avoid
		 * having everything error out.
		 */
		netif_device_detach (dev->net);
		(void) unlink_urbs (dev, &dev->rxq);
		(void) unlink_urbs (dev, &dev->txq);
		/*
		 * reattach so runtime management can use and
		 * wake the device
		 */
		netif_device_attach (dev->net);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(usbnet_suspend);

int usbnet_resume (struct usb_interface *intf)
{
	struct usbnet		*dev = usb_get_intfdata(intf);

	if (!--dev->suspend_count)
		tasklet_schedule (&dev->bh);

	return 0;
}
EXPORT_SYMBOL_GPL(usbnet_resume);


/*-------------------------------------------------------------------------*/

static int __init usbnet_init(void)
{
	/* compiler should optimize this out */
	BUILD_BUG_ON (sizeof (((struct sk_buff *)0)->cb)
			< sizeof (struct skb_data));

	random_ether_addr(node_id);
	return 0;
}
module_init(usbnet_init);

static void __exit usbnet_exit(void)
{
}
module_exit(usbnet_exit);

MODULE_AUTHOR("David Brownell");
MODULE_DESCRIPTION("USB network driver framework");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /*
    TDA10023  - DVB-C decoder
    (as used in Philips CU1216-3 NIM and the Reelbox DVB-C tuner card)

    Copyright (C) 2005 Georg Acher, BayCom GmbH (acher at baycom dot de)
    Copyright (c)