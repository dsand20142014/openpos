e)
 retry_reset_resume:
		status = usb_reset_and_verify_device(udev);

 	/* 10.5.4.5 says be sure devices in the tree are still there.
 	 * For now let's assume the device didn't go crazy on resume,
	 * and device drivers will know about any resume quirks.
	 */
	if (status == 0) {
		devstatus = 0;
		status = usb_get_status(udev, USB_RECIP_DEVICE, 0, &devstatus);
		if (status >= 0)
			status = (status > 0 ? 0 : -ENODEV);

		/* If a normal resume failed, try doing a reset-resume */
		if (status && !udev->reset_resume && udev->persist_enabled) {
			dev_dbg(&udev->dev, "retry with reset-resume\n");
			udev->reset_resume = 1;
			goto retry_reset_resume;
		}
	}

	if (status) {
		dev_dbg(&udev->dev, "gone after usb resume? status %d\n",
				status);
	} else if (udev->actconfig) {
		le16_to_cpus(&devstatus);
		if (devstatus & (1 << USB_DEVICE_REMOTE_WAKEUP)) {
			status = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0),
					USB_REQ_CLEAR_FEATURE,
						USB_RECIP_DEVICE,
					USB_DEVICE_REMOTE_WAKEUP, 0,
					NULL, 0,
					USB_CTRL_SET_TIMEOUT);
			if (status)
				dev_dbg(&udev->dev,
					"disable remote wakeup, status %d\n",
					status);
		}
		status = 0;
	}
	return status;
}

/*
 * usb_port_resume - re-activate a suspended usb device's upstream port
 * @udev: device to re-activate, not a root hub
 * Context: must be able to sleep; device not locked; pm locks held
 *
 * This will re-activate the suspended device, increasing power usage
 * while letting drivers communicate again with its endpoints.
 * USB resume explicitly guarantees that the power session between
 * the host and the device is the same as it was when the device
 * suspended.
 *
 * If @udev->reset_resume is set then this routine won't check that the
 * port is still enabled.  Furthermore, finish_port_resume() above will
 * reset @udev.  The end result is that a broken power session can be
 * recovered and @udev will appear to persist across a loss of VBUS power.
 *
 * For example, if a host controller doesn't maintain VBUS suspend current
 * during a system sleep or is reset when the system wakes up, all the USB
 * power sessions below it will be broken.  This is especially troublesome
 * for mass-storage devices containing mounted filesystems, since the
 * device will appear to have disconnected and all the memory mappings
 * to it will be lost.  Using the USB_PERSIST facility, the device can be
 * made to appear as if it had not disconnected.
 *
 * This facility can be dangerous.  Although usb_reset_and_verify_device() makes
 * every effort to insure that the same device is present after the
 * reset as before, it cannot provide a 100% guarantee.  Furthermore it's
 * quite possible for a device to remain unaltered but its media to be
 * changed.  If the user replaces a flash memory card while the system is
 * asleep, he will have only himself to blame when the filesystem on the
 * new card is corrupted and the system crashes.
 *
 * Returns 0 on success, else negative errno.
 */
int usb_port_resume(struct usb_device *udev, pm_message_t msg)
{
	struct usb_hub	*hub = hdev_to_hub(udev->parent);
	int		port1 = udev->portnum;
	int		status;
	u16		portchange, portstatus;

	/* Skip the initial Clear-Suspend step for a remote wakeup */
	status = hub_port_status(hub, port1, &portstatus, &portchange);
	if (status == 0 && !(portstatus & USB_PORT_STAT_SUSPEND))
		goto SuspendCleared;

	// dev_dbg(hub->intfdev, "resume port %d\n", port1);

	set_bit(port1, hub->busy_bits);

	/* see 7.1.7.7; affects power usage, but not budgeting */
	status = clear_port_feature(hub->hdev,
			port1, USB_PORT_FEAT_SUSPEND);
	if (status) {
		dev_dbg(hub->intfdev, "can't resume port %d, status %d\n",
				port1, status);
	} else {
		/* drive resume for at least 20 msec */
		dev_dbg(&udev->dev, "usb %sresume\n",
				(msg.event & PM_EVENT_AUTO ? "auto-" : ""));
		msleep(25);

		/* Virtual root hubs can trigger on GET_PORT_STATUS to
		 * stop resume signaling.  Then finish the resume
		 * sequence.
		 */
		status = hub_port_status(hub, port1, &portstatus, &portchange);

		/* TRSMRCY = 10 msec */
		msleep(10);
	}

 SuspendCleared:
	if (status == 0) {
		if (portchange & USB_PORT_STAT_C_SUSPEND)
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_SUSPEND);
	}

	clear_bit(port1, hub->busy_bits);

	status = check_port_resume_type(udev,
			hub, port1, status, portchange, portstatus);
	if (status == 0)
		status = finish_port_resume(udev);
	if (status < 0) {
		dev_dbg(&udev->dev, "can't resume, status %d\n", status);
		hub_port_logical_disconnect(hub, port1);
	}
	return status;
}

/* caller has locked udev */
static int remote_wakeup(struct usb_device *udev)
{
	int	status = 0;

	if (udev->state == USB_STATE_SUSPENDED) {
		dev_dbg(&udev->dev, "usb %sresume\n", "wakeup-");
		usb_mark_last_busy(udev);
		status = usb_external_resume_device(udev, PMSG_REMOTE_RESUME);
	}
	return status;
}

#else	/* CONFIG_USB_SUSPEND */

/* When CONFIG_USB_SUSPEND isn't set, we never suspend or resume any ports. */

int usb_port_suspend(struct usb_device *udev, pm_message_t msg)
{
	return 0;
}

/* However we may need to do a reset-resume */

int usb_port_resume(struct usb_device *udev, pm_message_t msg)
{
	struct usb_hub	*hub = hdev_to_hub(udev->parent);
	int		port1 = udev->portnum;
	int		status;
	u16		portchange, portstatus;

	status = hub_port_status(hub, port1, &portstatus, &portchange);
	status = check_port_resume_type(udev,
			hub, port1, status, portchange, portstatus);

	if (status) {
		dev_dbg(&udev->dev, "can't resume, status %d\n", status);
		hub_port_logical_disconnect(hub, port1);
	} else if (udev->reset_resume) {
		dev_dbg(&udev->dev, "reset-resume\n");
		status = usb_reset_and_verify_device(udev);
	}
	return status;
}

static inline int remote_wakeup(struct usb_device *udev)
{
	return 0;
}

#endif

static int hub_suspend(struct usb_interface *intf, pm_message_t msg)
{
	struct usb_hub		*hub = usb_get_intfdata (intf);
	struct usb_device	*hdev = hub->hdev;
	unsigned		port1;

	/* fail if children aren't already suspended */
	for (port1 = 1; port1 <= hdev->maxchild; port1++) {
		struct usb_device	*udev;

		udev = hdev->children [port1-1];
		if (udev && udev->can_submit) {
			if (!(msg.event & PM_EVENT_AUTO))
				dev_dbg(&intf->dev, "port %d nyet suspended\n",
						port1);
			return -EBUSY;
		}
	}

	dev_dbg(&intf->dev, "%s\n", __func__);

	/* stop khubd and related activity */
	hub_quiesce(hub, HUB_SUSPEND);
	return 0;
}

static int hub_resume(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	dev_dbg(&intf->dev, "%s\n", __func__);
	hub_activate(hub, HUB_RESUME);
	return 0;
}

static int hub_reset_resume(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	dev_dbg(&intf->dev, "%s\n", __func__);
	hub_activate(hub, HUB_RESET_RESUME);
	return 0;
}

/**
 * usb_root_hub_lost_power - called by HCD if the root hub lost Vbus power
 * @rhdev: struct usb_device for the root hub
 *
 * The USB host controller driver calls this function when its root hub
 * is resumed and Vbus power has been interrupted or the controller
 * has been reset.  The routine marks @rhdev as having lost power.
 * When the hub driver is resumed it will take notice and carry out
 * power-session recovery for all the "USB-PERSIST"-enabled child devices;
 * the others will be disconnected.
 */
void usb_root_hub_lost_power(struct usb_device *rhdev)
{
	dev_warn(&rhdev->dev, "root hub lost power or was reset\n");
	rhdev->reset_resume = 1;
}
EXPORT_SYMBOL_GPL(usb_root_hub_lost_power);

#else	/* CONFIG_PM */

static inline int remote_wakeup(struct usb_device *udev)
{
	return 0;
}

#define hub_suspend		NULL
#define hub_resume		NULL
#define hub_reset_resume	NULL
#endif


/* USB 2.0 spec, 7.1.7.3 / fig 7-29:
 *
 * Between connect detection and reset signaling there must be a delay
 * of 100ms at least for debounce and power-settling.  The corresponding
 * timer shall restart whenever the downstream port detects a disconnect.
 * 
 * Apparently there are some bluetooth and irda-dongles and a number of
 * low-speed devices for which this debounce period may last over a second.
 * Not covered by the spec - but easy to deal with.
 *
 * This implementation uses a 1500ms total debounce timeout; if the
 * connection isn't stable by then it returns -ETIMEDOUT.  It checks
 * every 25ms for transient disconnects.  When the port status has been
 * unchanged for 100ms it returns the port status.
 */
static int hub_port_debounce(struct usb_hub *hub, int port1)
{
	int ret;
	int total_time, stable_time = 0;
	u16 portchange, portstatus;
	unsigned connection = 0xffff;

	for (total_time = 0; ; total_time += HUB_DEBOUNCE_STEP) {
		ret = hub_port_status(hub, port1, &portstatus, &portchange);
		if (ret < 0)
			return ret;

		if (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
		     (portstatus & USB_PORT_STAT_CONNECTION) == connection) {
			stable_time += HUB_DEBOUNCE_STEP;
			if (stable_time >= HUB_DEBOUNCE_STABLE)
				break;
		} else {
			stable_time = 0;
			connection = portstatus & USB_PORT_STAT_CONNECTION;
		}

		if (portchange & USB_PORT_STAT_C_CONNECTION) {
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_CONNECTION);
		}

		if (total_time >= HUB_DEBOUNCE_TIMEOUT)
			break;
		msleep(HUB_DEBOUNCE_STEP);
	}

	dev_dbg (hub->intfdev,
		"debounce: port %d: total %dms stable %dms status 0x%x\n",
		port1, total_time, stable_time, portstatus);

	if (stable_time < HUB_DEBOUNCE_STABLE)
		return -ETIMEDOUT;
	return portstatus;
}

void usb_ep0_reinit(struct usb_device *udev)
{
	usb_disable_endpoint(udev, 0 + USB_DIR_IN, true);
	usb_disable_endpoint(udev, 0 + USB_DIR_OUT, true);
	usb_enable_endpoint(udev, &udev->ep0, true);
}
EXPORT_SYMBOL_GPL(usb_ep0_reinit);

#define usb_sndaddr0pipe()	(PIPE_CONTROL << 30)
#define usb_rcvaddr0pipe()	((PIPE_CONTROL << 30) | USB_DIR_IN)

static int hub_set_address(struct usb_de