 or any of the usage counters
 * for an active interface is greater than 0, no autosuspend request will be
 * queued.  (If an interface driver does not support autosuspend then its
 * usage counter is permanently positive.)  Furthermore, if an interface
 * driver requires remote-wakeup capability during autosuspend but remote
 * wakeup is disabled, the autosuspend will fail.
 *
 * Often the caller will hold @udev's device lock, but this is not
 * necessary.
 *
 * This routine can run only in process context.
 */
void usb_autosuspend_device(struct usb_device *udev)
{
	int	status;

	status = usb_autopm_do_device(udev, -1);
	dev_vdbg(&udev->dev, "%s: cnt %d\n",
			__func__, udev->pm_usage_cnt);
}

/**
 * usb_try_autosuspend_device - attempt an autosuspend of a USB device and its interfaces
 * @udev: the usb_device to autosuspend
 *
 * This routine should be called when a core subsystem thinks @udev may
 * be ready to autosuspend.
 *
 * @udev's usage counter left unchanged.  If it or any of the usage counters
 * for an active interface is greater than 0, or autosuspend is not allowed
 * for any other reason, no autosuspend request will be queued.
 *
 * This routine can run only in process context.
 */
void usb_try_autosuspend_device(struct usb_device *udev)
{
	usb_autopm_do_device(udev, 0);
	dev_vdbg(&udev->dev, "%s: cnt %d\n",
			__func__, udev->pm_usage_cnt);
}

/**
 * usb_autoresume_device - immediately autoresume a USB device and its interfaces
 * @udev: the usb_device to autoresume
 *
 * This routine should be called when a core subsystem wants to use @udev
 * and needs to guarantee that it is not suspended.  No autosuspend will
 * occur until usb_autosuspend_device is called.  (Note that this will not
 * prevent suspend events originating in the PM core.)  Examples would be
 * when @udev's device file in usbfs is opened or when a remote-wakeup
 * request is received.
 *
 * @udev's usage counter is incremented to prevent subsequent autosuspends.
 * However if the autoresume fails then the usage counter is re-decremented.
 *
 * Often the caller will hold @udev's device lock, but this is not
 * necessary (and attempting it might cause deadlock).
 *
 * This routine can run only in process context.
 */
int usb_autoresume_device(struct usb_device *udev)
{
	int	status;

	status = usb_autopm_do_device(udev, 1);
	dev_vdbg(&udev->dev, "%s: status %d cnt %d\n",
			__func__, status, udev->pm_usage_cnt);
	return status;
}

/* Internal routine to adjust an interface's usage counter and change
 * its device's autosuspend state.
 */
static int usb_autopm_do_interface(struct usb_interface *intf,
		int inc_usage_cnt)
{
	struct usb_device	*udev = interface_to_usbdev(intf);
	int			status = 0;

	usb_pm_lock(udev);
	if (intf->condition == USB_INTERFACE_UNBOUND)
		status = -ENODEV;
	else {
		udev->auto_pm = 1;
		intf->pm_usage_cnt += inc_usage_cnt;
		udev->last_busy = jiffies;
		if (inc_usage_cnt >= 0 && intf->pm_usage_cnt > 0) {
			if (udev->state == USB_STATE_SUSPENDED)
				status = usb_resume_both(udev,
						PMSG_AUTO_RESUME);
			if (status != 0)
				intf->pm_usage_cnt -= inc_usage_cnt;
			else
				udev->last_busy = jiffies;
		} else if (inc_usage_cnt <= 0 && intf->pm_usage_cnt <= 0) {
			status = usb_suspend_both(udev, PMSG_AUTO_SUSPEND);
		}
	}
	usb_pm_unlock(udev);
	return status;
}

/**
 * usb_autopm_put_interface - decrement a USB interface's PM-usage counter
 * @intf: the usb_interface whose counter should be decremented
 *
 * This routine should be called by an interface driver when it is
 * finished using @intf and wants to allow it to autosuspend.  A typical
 * example would be a character-device driver when its device file is
 * closed.
 *
 * The routine decrements @intf's usage counter.  When the counter reaches
 * 0, a delayed autosuspend request for @intf's device is queued.  When
 * the delay expires, if @intf->pm_usage_cnt is still <= 0 along with all
 * the other usage counters for the sibling interfaces and @intf's
 * usb_device, the device and all its interfaces will be autosuspended.
 *
 * Note that @intf->pm_usage_cnt is owned by the interface driver.  The
 * core will not change its value other than the increment and decrement
 * in usb_autopm_get_interface and usb_autopm_put_interface.  The driver
 * may use this simple counter-oriented discipline or may set the value
 * any way it likes.
 *
 * If the driver has set @intf->needs_remote_wakeup then autosuspend will
 * take place only if the device's remote-wakeup facility is enabled.
 *
 * Suspend method calls queued by this routine can arrive at any time
 * while @intf is resumed and its usage counter is equal to 0.  They are
 * not protected by the usb_device's lock but only by its pm_mutex.
 * Drivers must provide their own synchronization.
 *
 * This routine can run only in process context.
 */
void usb_autopm_put_interface(struct usb_interface *intf)
{
	int	status;

	status = usb_autopm_do_interface(intf, -1);
	dev_vdbg(&intf->dev, "%s: status %d cnt %d\n",
			__func__, status, intf->pm_usage_cnt);
}
EXPORT_SYMBOL_GPL(usb_autopm_put_interface);

/**
 * usb_autopm_put_interface_async - decrement a USB interface's PM-usage counter
 * @intf: the usb_interface whose counter should be decremented
 *
 * This routine does essentially the same thing as
 * usb_autopm_put_interface(): it decrements @intf's usage counter and
 * queues a delayed autosuspend request if the counter is <= 0.  The
 * difference is that it does not acquire the device's pm_mutex;
 * callers must handle all synchronization issues themselves.
 *
 * Typically a driver would call this routine during an URB's completion
 * handler, if no more URBs were pending.
 *
 * This routine can run in atomic context.
 */
void usb_autopm_put_interface_async(struct usb_interface *intf)
{
	struct usb_device	*udev = interface_to_usbdev(intf);
	int			status = 0;

	if (intf->condition == USB_INTERFACE_UNBOUND) {
		status = -ENODEV;
	} else {
		udev->last_busy = jiffies;
		--intf->pm_usage_cnt;
		if (udev->autosuspend_disabled || udev->autosuspend_delay < 0)
			status = -EPERM;
		else if (intf->pm_usage_cnt <= 0 &&
				!timer_pending(&udev->autosuspend.timer)) {
			queue_delayed_work(ksuspend_usb_wq, &udev->autosuspend,
					round_jiffies_up_relative(
						udev->autosuspend_delay));
		}
	}
	dev_vdbg(&intf->dev, "%s: status %d cnt %d\n",
			__func__, status, intf->pm_usage_cnt);
}
EXPORT_SYMBOL_GPL(usb_autopm_put_interface_async);

/**
 * usb_autopm_get_interface - increment a USB interface's PM-usage counter
 * @intf: the usb_interface whose counter should be incremented
 *
 * This routine should be called by an interface driver when it wants to
 * use @intf and needs to guarantee that it is not suspended.  In addition,
 * the routine prevents @intf from being autosuspended subsequently.  (Note
 * that this will not prevent suspend events originating in the PM core.)
 * This prevention will persist until usb_autopm_put_interface() is called
 * or @intf is unbound.  A typical example would be a character-device
 * driver when its device file is opened.
 *
 *
 * The routine increments @intf's usage counter.  (However if the
 * autoresume fails then the counter is re-decremented.)  So long as the
 * counter is greater than 0, autosuspend will not be allowed for @intf
 * or its usb_device.  When the driver is finished using @intf it should
 * call usb_autopm_put_interface() to decrement the usage counter and
 * queue a delayed autosuspend request (if the counter is <= 0).
 *
 *
 * Note that @intf->pm_usage_cnt is owned by the interface driver.  The
 * core will not change its value other than the increment and decrement
 * in usb_autopm_get_interface and usb_autopm_put_interface.  The driver
 * may use this simple counter-oriented discipline or may set the value
 * any way it likes.
 *
 * Resume method calls generated by this routine can arrive at any time
 * while @intf is suspended.  They are not protected by the usb_device's
 * lock but only by its pm_mutex.  Drivers must provide their own
 * synchronization.
 *
 * This routine can run only in process context.
 */
int usb_autopm_get_interface(struct usb_interface *intf)
{
	int	status;

	status = usb_autopm_do_interface(intf, 1);
	dev_vdbg(&intf->dev, "%s: status %d cnt %d\n",
			__func__, status, intf->pm_usage_cnt);
	return status;
}
EXPORT_SYMBOL_GPL(usb_autopm_get_interface);

/**
 * usb_autopm_get_interface_async - increment a USB interface's PM-usage counter
 * @intf: the usb_interface whose counter should be incremented
 *
 * This routine does much the same thing as
 * usb_autopm_get_interface(): it increments @intf's usage counter and
 * queues an autoresume request if the result is > 0.  The differences
 * are that it does not acquire the device's pm_mutex (callers must
 * handle all synchronization issues themselves), and it does not
 * autoresume the device directly (it only queues a request).  After a
 * successful call, the device will generally not yet be resumed.
 *
 * This routine can run in atomic context.
 */
int usb_autopm_get_interface_async(struct usb_interface *intf)
{
	struct usb_device	*udev = interface_to_usbdev(intf);
	int			status = 0;

	if (intf->condition == USB_INTERFACE_UNBOUND)
		status = -ENODEV;
	else if (udev->autoresume_disabled)
		status = -EPERM;
	else if (++intf->pm_usage_cnt > 0 && udev->state == USB_STATE_SUSPENDED)
		queue_work(ksuspend_usb_wq, &udev->autoresume);
	dev_vdbg(&intf->dev, "%s: status %d cnt %d\n",
			__func__, status, intf->pm_usage_cnt);
	return status;
}
EXPORT_SYMBOL_GPL(usb_autopm_get_interface_async);

/**
 * usb_autopm_set_interface - set a USB interface's autosuspend state
 * @intf: the usb_interface whose state should be set
 *
 * This routine sets the autosuspend state of @intf's device according
 * to @intf's usage counter, which the caller must have set previously.
 * If the counter is <= 0, the device is autosuspended (if it isn't
 * already suspended and if nothing else prevents the autosuspend).  If
 * the counter is > 0, the device is autoresumed (if it isn't already
 * awake).
 */
int usb_autopm_set_interface(struct usb_interface *intf)
{
	int	status;

	status = usb_autopm_do_interface(intf, 0);
	dev_vdbg(&intf->dev, "%s: status %d cnt %d\n",
			__func__, status, intf->pm_usage_cnt);
	return status;
}
EXPORT_SYMBOL_GPL(usb_autopm_set_interface);

#else

void usb_autosuspend_work(struct work_struct *work)
{}

void usb_autoresume_work(struct work_struct *work)
{}

#endif /* CONFIG_USB_SUSPEND */

/**
 * usb_external_suspend_device - external suspend of a USB device and its interfaces
 * @udev: the usb_device to suspend
 * @msg: Power Management message describing this state transition
 *
 * This routine handles external suspend requests: ones not generated
 * internally by a USB driver (autosuspend) but rather coming from the user
 * (via sysfs) or the PM core (system sleep).  The suspend will be carried
 * out regardless of @udev's usage counter or those of its interfaces,
 * and regardless of whether or not remote wakeup is enabled.  Of course,
 * interface drivers still have the option of failing the suspend (if
 * there are unsuspended children, for example).
 *
 * The caller must hold @udev's device lock.
 */
int usb_external_suspend_device(struct usb_device *udev, pm_message_t msg)
{
	int	status;

	do_unbind_rebind(udev, DO_UNBIND);
	usb_pm_lock(udev);
	udev->auto_pm = 0;
	status = usb_suspend_both(udev, msg);
	usb_pm_unlock(udev);
	return status;
}

/**
 * usb_external_resume_device - external resume of a USB device and its interfaces
 * @udev: the usb_device to resume
 * @msg: Power Management message describing this state transition
 *
 * This routine handles external resume requests: ones not generated
 * internally by a USB driver (autoresume) but rather coming from the user
 * (via sysfs), the PM core (system resume), or the device itself (remote
 * wakeup).  @udev's usage counter is unaffected.
 *
 * The caller must hold @udev's device lock.
 */
int usb_external_resume_device(struct usb_device *udev, pm_message_t msg)
{
	int	status;

	usb_pm_lock(udev);
	udev->auto_pm = 0;
	status = usb_resume_both(udev, msg);
	udev->last_busy = jiffies;
	usb_pm_unlock(udev);
	if (status == 0)
		do_unbind_rebind(udev, DO_REBIND);

	/* Now that the device is awake, we can start trying to autosuspend
	 * it again. */
	if (status == 0)
		usb_try_autosuspend_device(udev);
	return status;
}

int usb_suspend(struct device *dev, pm_message_t msg)
{
	struct usb_device	*udev;

	udev = to_usb_device(dev);

	/* If udev is already suspended, we can skip this suspend and
	 * we should also skip the upcoming system resume.  High-speed
	 * root hubs are an exception; they need to resume whenever the
	 * system wakes up in order for USB-PERSIST port handover to work
	 * properly.
	 */
	if (udev->state == USB_STATE_SUSPENDED) {
		if (udev->parent || udev->speed != USB_SPEED_HIGH)
			udev->skip_sys_resume = 1;
		return 0;
	}

	udev->skip_sys_resume = 0;
	return usb_external_suspend_device(udev, msg);
}

int usb_resume(struct device *dev, pm_message_t msg)
{
	struct usb_device	*udev;
	int			status;

	udev = to_usb_device(dev);

	/* If udev->skip_sys_resume is set then udev was already suspended
	 * when the system sleep started, so we don't want to resume it
	 * during this system wakeup.
	 */
	if (udev->skip_sys_resume)
		return 0;
	status = usb_external_resume_device(udev, msg);

	/* Avoid PM error messages for devices disconnected while suspended
	 * as we'll display regular disconnect messages just a bit later.
	 */
	if (status == -ENODEV)
		return 0;
	return status;
}

#endif /* CONFIG_PM */

struct bus_type usb_bus_type = {
	.name =		"usb",
	.match =	usb_device_match,
	.uevent =	usb_uevent,
};
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       