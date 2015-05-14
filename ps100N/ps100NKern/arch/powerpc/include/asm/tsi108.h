evices below the suspended port are also suspended.
 *
 * Devices leave suspend state when the host wakes them up.  Some devices
 * also support "remote wakeup", where the device can activate the USB
 * tree above them to deliver data, such as a keypress or packet.  In
 * some cases, this wakes the USB host.
 *
 * Suspending OTG devices may trigger HNP, if that's been enabled
 * between a pair of dual-role devices.  That will change roles, such
 * as from A-Host to A-Peripheral or from B-Host back to B-Peripheral.
 *
 * Devices on USB hub ports have only one "suspend" state, corresponding
 * to ACPI D2, "may cause the device to lose some context".
 * State transitions include:
 *
 *   - suspend, resume ... when the VBUS power link stays live
 *   - suspend, disconnect ... VBUS lost
 *
 * Once VBUS drop breaks the circuit, the port it's using has to go through
 * normal re-enumeration procedures, starting with enabling VBUS power.
 * Other than re-initializing the hub (plug/unplug, except for root hubs),
 * Linux (2.6) currently has NO mechanisms to initiate that:  no khubd
 * timer, no SRP, no requests through sysfs.
 *
 * If CONFIG_USB_SUSPEND isn't enabled, devices only really suspend when
 * the root hub for their bus goes into global suspend ... so we don't
 * (falsely) update the device power state to say it suspended.
 *
 * Returns 0 on success, else negative errno.
 */
int usb_port_suspend(struct usb_device *udev, pm_message_t msg)
{
	struct usb_hub	*hub = hdev_to_hub(udev->parent);
	int		port1 = udev->portnum;
	int		status;

	// dev_dbg(hub->intfdev, "suspend port %d\n", port1);

	/* enable remote wakeup when appropriate; this lets the device
	 * wake up the upstream hub (including maybe the root hub).
	 *
	 * NOTE:  OTG devices may issue remote wakeup (or SRP) even when
	 * we don't explicitly enable it here.
	 */
	if (udev->do_remote_wakeup) {
		status = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				USB_REQ_SET_FEATURE, USB_RECIP_DEVICE,
				USB_DEVICE_REMOTE_WAKEUP, 0,
				NULL, 0,
				USB_CTRL_SET_TIMEOUT);
		if (status)
			dev_dbg(&udev->dev, "won't remote wakeup, status %d\n",
					status);
	}

	/* see 7.1.7.6 */
	status = set_port_feature(hub->hdev, port1, USB_PORT_FEAT_SUSPEND);
	if (status) {
		dev_dbg(hub->intfdev, "can't suspend port %d, status %d\n",
				port1, status);
		/* paranoia:  "should not happen" */
		(void) usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RECIP_DEVICE,
				USB_DEVICE_REMOTE_WAKEUP, 0,
				NULL, 0,
				USB_CTRL_SET_TIMEOUT);
	} else {
		/* device has up to 10 msec to fully suspend */
		dev_dbg(&udev->dev, "usb %ssuspend\n",
				(msg.event & PM_EVENT_AUTO ? "auto-" : ""));
		usb_set_device_state(udev, USB_STATE_SUSPENDED);
		msleep(10);
	}
	return status;
}

/*
 * If the USB "suspend" state is in use (rather than "global suspend"),
 * many devices will be individually taken out of suspend state using
 * special "resume" signaling.  This routine kicks in shortly after
 * hardware resume signaling is finished, either because of selective
 * resume (by host) or remote wakeup (by device) ... now see what changed
 * in the tree that's rooted at this device.
 *
 * If @udev->reset_resume is set then the device is reset before the
 * status check is done.
 */
static int finish_port_resume(struct usb_device *udev)
{
	int	status = 0;
	u16	devstatus;

	/* caller owns the udev device lock */
	dev_dbg(&udev->dev, "%s\n",
		udev->reset_resume ? "finish reset-resume" : "finish resume");

	/* usb ch9 identifies four variants of SUSPENDED, based on what
	 * state the device resumes to.  Linux currently won't see the
	 * first two on the host side; they'd be inside