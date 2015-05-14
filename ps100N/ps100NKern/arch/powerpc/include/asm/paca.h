fig->interface[i];
		struct usb_host_interface *alt;

		alt = usb_altnum_to_altsetting(intf, 0);

		/* No altsetting 0?  We'll assume the first altsetting.
		 * We could use a GetInterface call, but if a device is
		 * so non-compliant that it doesn't have altsetting 0
		 * then I wouldn't trust its reply anyway.
		 */
		if (!alt)
			alt = &intf->altsetting[0];

		if (alt != intf->cur_altsetting) {
			remove_intf_ep_devs(intf);
			usb_remove_sysfs_intf_files(intf);
		}
		intf->cur_altsetting = alt;
		usb_enable_interface(dev, intf, true);
		if (device_is_registered(&intf->dev)) {
			usb_create_sysfs_intf_files(intf);
			create_intf_ep_devs(intf);
		}
	}
	return 0;
}
EXPORT_SYMBOL_GPL(usb_reset_configuration);

static void usb_release_interface(struct device *dev)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usb_interface_cache *intfc =
			altsetting_to_usb_interface_cache(intf->altsetting);

	kref_put(&intfc->ref, usb_release_interface_cache);
	kfree(intf);
}

#ifdef	CONFIG_HOTPLUG
static int usb_if_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct usb_device *usb_dev;
	struct usb_interface *intf;
	struct usb_host_interface *alt;

	intf = to_usb_interface(dev);
	usb_dev = interface_to_usbdev(intf);
	alt = intf->cur_altsetting;

	if (add_uevent_var(env, "INTERFACE=%d/%d/%d",
		   alt->desc.bInterfaceClass,
		   alt->desc.bInterfaceSubClass,
		   alt->desc.bInterfaceProtocol))
		return -ENOMEM;

	if (add_uevent_var(env,
		   "MODALIAS=usb:"
		   "v%04Xp%04Xd%04Xdc%02Xdsc%02Xdp%02Xic%02Xisc%02Xip%02X",
		   le16_to_cpu(usb_dev->descriptor.idVendor),
		   le16_to_cpu(usb_dev->descriptor.idProduct),
		   le16_to_cpu(usb_dev->descriptor.bcdDevice),
		   usb_dev->descriptor.bDeviceClass,
		   usb_dev->descriptor.bDeviceSubClass,
		   usb_dev->descriptor.bDeviceProtocol,
		   alt->desc.bInterfaceClass,
		   alt->desc.bInterfaceSubClass,
		   alt->desc.bInterfaceProtocol))
		return -ENOMEM;

	return 0;
}

#else

static int usb_if_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return -ENODEV;
}
#endif	/* CONFIG_HOTPLUG */

struct device_type usb_if_device_type = {
	.name =		"usb_interface",
	.release =	usb_release_interface,
	.uevent =	usb_if_uevent,
};

static struct usb_interface_assoc_descriptor *find_iad(struct usb_device *dev,
						struct usb_host_config *config,
						u8 inum)
{
	struct usb_interface_assoc_descriptor *retval = NULL;
	struct usb_interface_assoc_descriptor *intf_assoc;
	int first_intf;
	int last_intf;
	int i;

	for (i = 0; (i < USB_MAXIADS && config->intf_assoc[i]); i++) {
		intf_assoc = config->intf_assoc[i];
		if (intf_assoc->bInterfaceCount == 0)
			continue;

		first_intf = intf_assoc->bFirstInterface;
		last_intf = first_intf + (intf_assoc->bInterfaceCount - 1);
		if (inum >= first_intf && inum <= last_intf) {
			if (!retval)
				retval = intf_assoc;
			else
				dev_err(&dev->dev, "Interface #%d referenced"
					" by multiple IADs\n", inum);
		}
	}

	return retval;
}


/*
 * Internal function to queue a device reset
 *
 * This is initialized into the workstruct in 'struct
 * usb_device->reset_ws' that is launched by
 * message.c:usb_set_configuration() when initializing each 'struct
 * usb_interface'.
 *
 * It is safe to get the USB device without reference counts because
 * the life cycle of @iface is bound to the life cycle of @udev. Then,
 * this function will be ran only if @iface is alive (and before
 * freeing it any scheduled instances of it will have been cancelled).
 *
 * We need to set a flag (usb_dev->reset_running) because when we call
 * the reset, the interfaces might be unbound. The current interface
 * cannot try to remove the queued work as it would cause a deadlock
 * (you cannot remove your work from within your executing
 * workqueue). This flag lets it know, so that
 * usb_cancel_queued_reset() doesn't try to do it.
 *
 * See usb_queue_reset_device() for more details
 */
void __usb_queue_reset_device(struct work_struct *ws)
{
	int rc;
	struct usb_interface *iface =
		container_of(ws