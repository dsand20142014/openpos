/*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#include "usbip_common.h"
#include "stub.h"



static int stub_probe(struct usb_interface *interface,
				const struct usb_device_id *id);
static void stub_disconnect(struct usb_interface *interface);


/*
 * Define device IDs here if you want to explicitly limit exportable devices.
 * In the most cases, wild card matching will be ok because driver binding can
 * be changed dynamically by a userland program.
 */
static struct usb_device_id stub_table[] = {
#if 0
	/* just an example */
	{ USB_DEVICE(0x05ac, 0x0301) },   /* Mac 1 button mouse */
	{ USB_DEVICE(0x0430, 0x0009) },   /* Plat Home Keyboard */
	{ USB_DEVICE(0x059b, 0x0001) },   /* Iomega USB Zip 100 */
	{ USB_DEVICE(0x04b3, 0x4427) },   /* IBM USB CD-ROM */
	{ USB_DEVICE(0x05a9, 0xa511) },   /* LifeView USB cam */
	{ USB_DEVICE(0x55aa, 0x0201) },   /* Imation card reader */
	{ USB_DEVICE(0x046d, 0x0870) },   /* Qcam Express(QV-30) */
	{ USB_DEVICE(0x04bb, 0x0101) },   /* IO-DATA HD 120GB */
	{ USB_DEVICE(0x04bb, 0x0904) },   /* IO-DATA USB-ET/TX */
	{ USB_DEVICE(0x04bb, 0x0201) },   /* IO-DATA USB-ET/TX */
	{ USB_DEVICE(0x08bb, 0x2702) },   /* ONKYO USB Speaker */
	{ USB_DEVICE(0x046d, 0x08b2) },   /* Logicool Qcam 4000 Pro */
#endif
	/* magic for wild card */
	{ .driver_info = 1 },
	{ 0, }                                     /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, stub_table);

struct usb_driver stub_driver = {
	.name		= "usbip",
	.probe		= stub_probe,
	.disconnect	= stub_disconnect,
	.id_table	= stub_table,
};


/*-------------------------------------------------------------------------*/

/* Define sysfs entries for a usbip-bound device */


/*
 * usbip_status shows status of usbip as long as this driver is bound to the
 * target device.
 */
static ssize_t show_status(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct stub_device *sdev = dev_get_drvdata(dev);
	int status;

	if (!sdev) {
		dev_err(dev, "sdev is null\n");
		return -ENODEV;
	}

	spin_lock(&sdev->ud.lock);
	status = sdev->ud.status;
	spin_unlock(&sdev->ud.lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", status);
}
static DEVICE_ATTR(usbip_status, S_IRUGO, show_status, NULL);

/*
 * usbip_sockfd gets a socket descriptor of an established TCP connection that
 * is used to transfer usbip requests by kernel threads. -1 is a magic number
 * by which usbip connection is finished.
 */
static ssize_t store_sockfd(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct stub_device *sdev = dev_get_drvdata(dev);
	int sockfd = 0;
	struct socket *socket;

	if (!sdev) {
		dev_err(dev, "sdev is null\n");
		return -ENODEV;
	}

	sscanf(buf, "%d", &sockfd);

	if (sockfd != -1) {
		dev_info(dev, "stub up\n");

		spin_lock(&sdev->ud.lock);

		if (sdev->ud.status != SDEV_ST_AVAILABLE) {
			dev_err(dev, "not ready\n");
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}

		socket = sockfd_to_socket(sockfd);
		if (!socket) {
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}

#if 0
		setnodelay(socket);
		setkeepalive(socket);
		setreuse(socket);
#endif

		sdev->ud.tcp_socket = socket;

		spin_unlock(&sdev->ud.lock);

		usbip_start_threads(&sdev->ud);

		sp