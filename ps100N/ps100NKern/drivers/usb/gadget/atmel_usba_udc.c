/*
 * Driver for the Atmel USBA high speed USB device controller
 *
 * Copyright (C) 2005-2007 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/atmel_usba_udc.h>
#include <linux/delay.h>

#include <asm/gpio.h>
#include <mach/board.h>

#include "atmel_usba_udc.h"


static struct usba_udc the_udc;
static struct usba_ep *usba_ep;

#ifdef CONFIG_USB_GADGET_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/uaccess.h>

static int queue_dbg_open(struct inode *inode, struct file *file)
{
	struct usba_ep *ep = inode->i_private;
	struct usba_request *req, *req_copy;
	struct list_head *queue_data;

	queue_data = kmalloc(sizeof(*queue_data), GFP_KERNEL);
	if (!queue_data)
		return -ENOMEM;
	INIT_LIST_HEAD(queue_data);

	spin_lock_irq(&ep->udc->lock);
	list_for_each_entry(req, &ep->queue, queue) {
		req_copy = kmalloc(sizeof(*req_copy), GFP_ATOMIC);
		if (!req_copy)
			goto fail;
		memcpy(req_copy, req, sizeof(*req_copy));
		list_add_tail(&req_copy->queue, queue_data);
	}
	spin_unlock_irq(&ep->udc->lock);

	file->private_data = queue_data;
	return 0;

fail:
	spin_unlock_irq(&ep->udc->lock);
	list_for_each_entry_safe(req, req_copy, queue_data, queue) {
		list_del(&req->queue);
		kfree(req);
	}
	kfree(queue_data);
	return -ENOMEM;
}

/*
 * bbbbbbbb llllllll IZS sssss nnnn FDL\n\0
 *
 * b: buffer address
 * l: buffer length
 * I/i: interrupt/no interrupt
 * Z/z: zero/no zero
 * S/s: short ok/short not ok
 * s: status
 * n: nr_packets
 * F/f: submitted/not submitted to FIFO
 * D/d: using/not using DMA
 * L/l: last transaction/not last transaction
 */
static ssize_t queue_dbg_read(struct file *file, char __user *buf,
		size_t nbytes, loff_t *ppos)
{
	struct list_head *queue = file->private_data;
	struct usba_request *req, *tmp_req;
	size_t len, remaining, actual = 0;
	char tmpbuf[38];

	if (!access_ok(VERIFY_WRITE, buf, nbytes))
		return -EFAULT;

	mutex_lock(&file->f_dentry->d_inode->i_mutex);
	list_for_each_entry_safe(req, tmp_req, queue, queue) {
		len = snprintf(tmpbuf, sizeof(tmpbuf),
				"%8p %08x %c%c%c %5d %c%c%c\n",
				req->req.buf, req->req.length,
				req->req.no_interrupt ? 'i' : 'I',
				req->req.zero ? 'Z' : 'z',
				req->req.short_not_ok ? 's' : 'S',
				req->req.status,
				req->submitted ? 'F' : 'f',
				req->using_dma ? 'D' : 'd',
				req->last_transaction ? 'L' : 'l');
		len = min(len, sizeof(tmpbuf));
		if (len > nbytes)
			break;

		list_del(&req->queue);
		kfree(req);

		remaining = __copy_to_user(buf, tmpbuf, len);
		actual += len - remaining;
		if (remaining)
			break;

		nbytes -= len;
		buf += len;
	}
	mutex_unlock(&file->f_dentry->d_inode->i_mutex);

	return actual;
}

static int queue_dbg_release(struct inode *inode, struct file *file)
{
	struct list_head *queue_data = file->private_data;
	struct usba_request *req, *tmp_req;

	list_for_each_entry_safe(req, tmp_req, queue_data, queue) {
		list_del(&req->queue);
		kfree(req);
	}
	kfree(queue_data);
	return 0;
}

static int regs_dbg_open(struct inode *inode, struct file *file)
{
	struct usba_udc *udc;
	unsigned int i;
	u32 *data;
	int ret = -ENOMEM;

	mutex_lock(&inode->i_mutex);
	udc = inode->i_private;
	data = kmalloc(inode->i_size, GFP_KERNEL);
	if (!data)
		goto out;

	spin_lock_irq(&udc->lock);
	for (i = 0; i < inode->i_size / 4; i++)
		data[i] = __raw_readl(udc->regs + i * 4);
	spin_unlock_irq(&udc->lock);

	file->private_data = data;
	ret = 0;

out:
	mutex_unlock(&inode->i_mutex);

	return ret;
}

static ssize_t regs_dbg_read(struct file *file, char __user *buf,
		size_t nbytes, loff_t *ppos)
{
	struct inode *inode = file->f_dentry->d_inode;
	int ret;

	mutex_lock(&inode->i_mutex);
	q->actual_length < PAGE_SIZE)
			pnd->rx_skb = NULL; /* Last fragment */
		else
			skb = NULL;
		spin_unlock_irqrestore(&pnd->rx_lock, flags);
		if (skb) {
			skb->protocol = htons(ETH_P_PHONET);
			skb_reset_mac_header(skb);
			__skb_pull(skb, 1);
			skb->dev = dev;
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += skb->len;

			netif_rx(skb);
		}
		goto resubmit;

	case -ENOENT:
	case -ECONNRESET:
	case -ESHUTDOWN:
		req = NULL;
		break;

	case -EOVERFLOW:
		dev->stats.rx_over_errors++;
		dev_dbg(&dev->dev, "RX overflow\n");
		break;

	case -EILSEQ:
		dev->stats.rx_crc_errors++;
		break;
	}

	dev->stats.rx_errors++;
resubmit:
	if (page)
		netdev_free_page(dev, page);
	if (req)
		rx_submit(pnd, req, GFP_ATOMIC);
}

static int usbpn_close(struct net_device *dev);

static int usbpn_open(struct net_device *dev)
{
	struct usbpn_dev *pnd = netdev_priv(dev);
	int err;
	unsigned i;
	unsigned num = pnd->data_intf->cur_altsetting->desc.bInterfaceNumber;

	err = usb_set_interface(pnd->usb, num, pnd->active_setting);
	if (err)
		return err;

	for (i = 0; i < rxq_size; i++) {
		struct urb *req = usb_alloc_urb(0, GFP_KERNEL);

		if (!req || rx_submit(pnd, req, GFP_KERNEL)) {
			usbpn_close(dev);
			return -ENOMEM;
		}
		pnd->urbs[i] = req;
	}

	netif_wake_queue(dev);
	return 0;
}

static int usbpn_close(struct net_device *dev)
{
	struct usbpn_dev *pnd = netdev_priv(dev);
	unsigned i;
	unsigned num = pnd->data_intf->cur_altsetting->desc.bInterfaceNumber;

	netif_stop_queue(dev);

	for (i = 0; i < rxq_size; i++) {
		struct urb *req = pnd->urbs[i];

		if (!req)
			continue;
		usb_kill_urb(req);
		usb_free_urb(req);
		pnd->urbs[i] = NULL;
	}

	return usb_set_interface(pnd->usb, num, !pnd->active_setting);
}

static int usbpn_set_mtu(struct net_device *dev, int new_mtu)
{
	if ((new_mtu < PHONET_MIN_MTU) || (new_mtu > PHONET_MAX_MTU))
		return -EINVAL;

	dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops usbpn_ops = {
	.ndo_open	= usbpn_open,
	.ndo_stop	= usbpn_close,
	.ndo_start_xmit = usbpn_xmit,
	.ndo_change_mtu = usbpn_set_mtu,
};

static void usbpn_setup(struct net_device *dev)
{
	dev->features		= 0;
	dev->netdev_ops		= &usbpn_ops,
	dev->header_ops		= &phonet_header_ops;
	dev->type		= ARPHRD_PHONET;
	dev->flags		= IFF_POINTOPOINT | IFF_NOARP;
	dev->mtu		= PHONET_MAX_MTU;
	dev->hard_header_len	= 1;
	dev->dev_addr[0]	= PN_MEDIA_USB;
	dev->addr_len		= 1;
	dev->tx_queue_len	= 3;

	dev->destructor		= free_netdev;
}

/*
 * USB driver callbacks
 */
static struct usb_device_id usbpn_ids[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR
			| USB_DEVICE_ID_MATCH_INT_CLASS
			| USB_DEVICE_ID_MATCH_INT_SUBCLASS,
		.idVendor = 0x0421, /* Nokia */
		.bInterfaceClass = USB_CLASS_COMM,
		.bInterfaceSubClass = 0xFE,
	},
	{ },
};

MODULE_DEVICE_TABLE(usb, usbpn_ids);

static struct usb_driver usbpn_driver;

int usbpn_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	static const char ifname[] = "usbpn%d";
	const struct usb_cdc_union_desc *union_header = NULL;
	const struct usb_cdc_header_desc *phonet_header = NULL;
	const struct usb_host_interface *data_desc;
	struct usb_interface *data_intf;
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct net_device *dev;
	struct usbpn_dev *pnd;
	u8 *data;
	int len, err;

	data = intf->altsetting->extra;
	len = intf->altsetting->extralen;
	while (len >= 3) {
		u8 dlen = data[0];
		if (dlen < 3)
			return -EINVAL;

		/* bDescriptorType */
		if (data[1] == USB_DT_CS_INTERFACE) {
			/* bDescriptorSubType */
			switch (data[2]) {
			case USB_CDC_UNION_TYPE:
				if (union_header || dlen < 5)
					break;
				union_header =
					(struct usb_cdc_union_desc *)data;
				break;
			case 0xAB:
				if (phonet_header || dlen < 5)
					break;
				phonet_header =
					(struct usb_cdc_header_desc *)data;
				break;
			}
		}
		data += dlen;
		len -= dlen;
	}

	if (!union_header || !phonet_header)
		return -EINVAL;

	data_intf = usb_ifnum_to_if(usbdev, union_header->bSlaveInterface0);
	if (data_intf == NULL)
		return -ENODEV;
	/* Data interface has one inactive and one active s>req.zero)
		req->last_transaction = 0;

	DBG(DBG_QUEUE, "%s: submit_transaction, req %p (length %d)%s\n",
		ep->ep.name, req, transaction_len,
		req->last_transaction ? ", done" : "");

	memcpy_toio(ep->fifo, req->req.buf + req->req.actual, transaction_len);
	usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
	req->req.actual += transaction_len;
}

static void submit_request(struct usba_ep *ep, struct usba_request *req)
{
	DBG(DBG_QUEUE, "%s: submit_request: req %p (length %d)\n",
		ep->ep.name, req, req->req.length);

	req->req.actual = 0;
	req->submitted = 1;

	if (req->using_dma) {
		if (req->req.length == 0) {
			usba_ep_writel(ep, CTL_ENB, USBA_TX_PK_RDY);
			return;
		}

		if (req->req.zero)
			usba_ep_writel(ep, CTL_ENB, USBA_SHORT_PACKET);
		else
			usba_ep_writel(ep, CTL_DIS, USBA_SHORT_PACKET);

		usba_dma_writel(ep, ADDRESS, req->req.dma);
		usba_dma_writel(ep, CONTROL, req->ctrl);
	} else {
		next_fifo_transaction(ep, req);
		if (req->last_transaction) {
			usba_ep_writel(ep, CTL_DIS, USBA_TX_PK_RDY);
			usba_ep_writel(ep, CTL_ENB, USBA_TX_COMPLETE);
		} else {
			usba_ep_writel(ep, CTL_DIS, USBA_TX_COMPLETE);
			usba_ep_writel(ep, CTL_ENB, USBA_TX_PK_RDY);
		}
	}
}

static void submit_next_request(struct usba_ep *ep)
{
	struct usba_request *req;

	if (list_empty(&ep->queue)) {
		usba_ep_writel(ep, CTL_DIS, USBA_TX_PK_RDY | USBA_RX_BK_RDY);
		return;
	}

	req = list_entry(ep->queue.next, struct usba_request, queue);
	if (!req->submitted)
		submit_request(ep, req);
}

static void send_status(struct usba_udc *udc, struct usba_ep *ep)
{
	ep->state = STATUS_STAGE_IN;
	usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
	usba_ep_writel(ep, CTL_ENB, USBA_TX_COMPLETE);
}

static void receive_data(struct usba_ep *ep)
{
	struct usba_udc *udc = ep->udc;
	struct usba_request *req;
	unsigned long status;
	unsigned int bytecount, nr_busy;
	int is_complete = 0;

	status = usba_ep_readl(ep, STA);
	nr_busy = USBA_BFEXT(BUSY_BANKS, status);

	DBG(DBG_QUEUE, "receive data: nr_busy=%u\n", nr_busy);

	while (nr_busy > 0) {
		if (list_empty(&ep->queue)) {
			usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);
			break;
		}
		req = list_entry(ep->queue.next,
				 struct usba_request, queue);

		bytecount = USBA_BFEXT(BYTE_COUNT, status);

		if (status & (1 << 31))
			is_complete = 1;
		if (req->req.actual + bytecount >= req->req.length) {
			is_complete = 1;
			bytecount = req->req.length - req->req.actual;
		}

		memcpy_fromio(req->req.buf + req->req.actual,
				ep->fifo, bytecount);
		req->req.actual += bytecount;

		usba_ep_writel(ep, CLR_STA, USBA_RX_BK_RDY);

		if (is_complete) {
			DBG(DBG_QUEUE, "%s: request done\n", ep->ep.name);
			req->req.status = 0;
			list_del_init(&req->queue);
			usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);
			spin_unlock(&udc->lock);
			req->req.complete(&ep->ep, &req->req);
			spin_lock(&udc->lock);
		}

		status = usba_ep_readl(ep, STA);
		nr_busy = USBA_BFEXT(BUSY_BANKS, status);

		if (is_complete && ep_is_control(ep)) {
			send_status(udc, ep);
			break;
		}
	}
}

static void
request_complete(struct usba_ep *ep, struct usba_request *req, int status)
{
	struct usba_udc *udc = ep->udc;

	WARN_ON(!list_empty(&req->queue));

	if (req->req.status == -EINPROGRESS)
		req->req.status = status;

	if (req->mapped) {
		dma_unmap_single(
			&udc->pdev->dev, req->req.dma, req->req.length,
			ep->is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		req->req.dma = DMA_ADDR_INVALID;
		req->mapped = 0;
	}

	DBG(DBG_GADGET | DBG_REQ,
		"%s: req %p complete: status %d, actual %u\n",
		ep->ep.name, req, req->req.status, req->req.actual);

	spin_unlock(&udc->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&udc->lock);
}

static void
request_complete_list(struct usba_ep *ep, struct list_head *list, int status)
{
	struct usba_request *req, *tmp_req;

	list_for_each_entry_safe(req, tmp_req, list, queue) {
		list_del_init(&req->queue);
		request_complete(ep, req, status);
	}
}

static int
usba_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct usba_ep *ep = to_usba_ep(_ep);
	struct usba_udc *udc = ep->u/*
 * wm8350-i2c.c  --  Generic I2C driver for Wolfson WM8350 PMIC
 *
 * Copyright 2007, 2008 Wolfson Microelectronics PLC.
 *
 * Author: Liam Girdwood
 *         linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/mfd/wm8350/core.h>

static int wm8350_i2c_read_device(struct wm8350 *wm8350, char reg,
				  int bytes, void *dest)
{
	int ret;

	ret = i2c_master_send(wm8350->i2c_client, &reg, 1);
	if (ret < 0)
		return ret;
	ret = i2c_master_recv(wm8350->i2c_client, dest, bytes);
	if (ret < 0)
		return ret;
	if (ret != bytes)
		return -EIO;
	return 0;
}

static int wm8350_i2c_write_device(struct wm8350 *wm8350, char reg,
				   int bytes, void *src)
{
	/* we add 1 byte for device register */
	u8 msg[(WM8350_MAX_REGISTER << 1) + 1];
	int ret;

	if (bytes > ((WM8350_MAX_REGISTER << 1) + 1))
		return -EINVAL;

	msg[0] = reg;
	memcpy(&msg[1], src, bytes);
	ret = i2c_master_send(wm8350->i2c_client, msg, bytes + 1);
	if (ret < 0)
		return ret;
	if (ret != bytes + 1)
		return -EIO;
	return 0;
}

static int wm8350_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct wm8350 *wm8350;
	int ret = 0;

	wm8350 = kzalloc(sizeof(struct wm8350), GFP_KERNEL);
	if (wm8350 == NULL) {
		kfree(i2c);
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c, wm8350);
	wm8350->dev = &i2c->dev;
	wm8350->i2c_client = i2c;
	wm8350->read_dev = wm8350_i2c_read_device;
	wm8350->write_dev = wm8350_i2c_write_device;

	ret = wm8350_device_init(wm8350, i2c->irq, i2c->dev.platform_data);
	if (ret < 0)
		goto err;

	return ret;

err:
	kfree(wm8350);
	return ret;
}

static int wm8350_i2c_remove(struct i2c_client *i2c)
{
	struct wm8350 *wm8350 = i2c_get_clientdata(i2c);

	wm8350_device_exit(wm8350);
	kfree(wm8350);

	return 0;
}

static const struct i2c_device_id wm8350_i2c_id[] = {
       { "wm8350", 0 },
       { "wm8351", 0 },
       { "wm8352", 0 },
       { }
};
MODULE_DEVICE_TABLE(i2c, wm8350_i2c_id);


static struct i2c_driver wm8350_i2c_driver = {
	.driver = {
		   .name = "wm8350",
		   .owner = THIS_MODULE,
	},
	.probe = wm8350_i2c_probe,
	.remove = wm8350_i2c_remove,
	.id_table = wm8350_i2c_id,
};

static int __init wm8350_i2c_init(void)
{
	return i2c_add_driver(&wm8350_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(wm8350_i2c_init);

static void __exit wm8350_i2c_exit(void)
{
	i2c_del_driver(&wm8350_i2c_driver);
}
module_exit(wm8350_i2c_exit);

MODULE_DESCRIPTION("I2C support for the WM8350 AudioPlus PMIC");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
/******************************************************************************
 *
 * Name: hwsleep.c - ACPI Hardware Sleep/Wake Interface
 *
 *****************************************************************************/

/*
 * Copyright (C) 2000 - 2008, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#include <acpi/acpi.h>
#include "accommon.h"
#include "actables.h"

#define _COMPONENT          ACPI_HARDWARE
ACPI_MODULE_NAME("hwsleep")

/*******************************************************************************
 *
 * FUNCTION:    acpi_set_firmware_waking_vector
 *
 * PARAMETERS:  physical_address    - 32-bit physical address of ACPI real mode
 *                                    entry point.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Sets the 32-bit firmware_waking_vector field of the FACS
 *
 ******************************************************************************/
acpi_status
acpi_set_firmware_waking_vector(u32 physical_address)
{
	ACPI_FUNCTION_TRACE(acpi_set_firmware_waking_vector);


	/*
	 * According to the ACPI specification 2.0c and later, the 64-bit
	 * waking vector should be cleared and the 32-bit waking vector should
	 * be used, unless we want the wake-up code to be called by the BIOS in
	 * Protected Mode.  Some systems (for example HP dv5-1004nr) are known
	 * to fail to resume if the 64-bit vector is used.
	 */

	/* Set the 32-bit vector */

	acpi_gbl_FACS->firmware_waking_vector = physical_address;

	/* Clear the 64-bit vector if it exists */

	if ((acpi_gbl_FACS->length > 32) && (acpi_gbl_FACS->version >= 1)) {
		acpi_gbl_FACS->xfirmware_waking_vector = 0;
	}

	return_ACPI_STATUS(AE_OK);
}

ACPI_EXPORT_SYMBOL(acpi_set_firmware_waking_vector)

#if ACPI_MACHINE_WIDTH == 64
/*******************************************************************************
 *
 * FUNCTION:    acpi_set_firmware_waking_vector64
 *
 * PARAMETERS:  physical_address    - 64-bit physical address of ACPI protected
 *                                    mode entry point.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Sets the 64-bit X_firmware_waking_vector field of the FACS, if
 *              it exists in the table. This function is intended for use with
 *              64-bit host operating systems.
 *
 ******************************************************************************/
acpi_status
acpi_set_firmware_waking_vector64(u64 physical_address)
{
	ACPI_FUNCTION_TRACE(acpi_set_firmware_waking_vector64);


	/* Determine if the 64-bit vector actually exists */

	if ((acpi_gbl_FACS->length <= 32) || (acpi_gbl_FACS->version < 1)) {
		return_ACPI_STATUS(AE_NOT_EXIST);
	}

	/* Clear 32-bit vector, set the 64-bit X_ vector */

	acpi_gbl_FACS->firmware_waking_vector = 0;
	acpi_gbl_FACS->xfirmware_waking_vector = physical_address;

	return_ACPI_STATUS(AE_OK);
}

ACPI_EXPORT_SYMBOL(acpi_set_firmware_waking_vector64)
#endif

/*******************************************************************************
 *
 * FUNCTION:    acpi_enter_sleep_state_prep
 *
 * PARAMETERS:  sleep_state         - Which sleep state to enter
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Prepare to enter a system sleep state (see ACPI 2.0 spec p 231)
 *              This function must execute with interrupts enabled.
 *              We break sleeping into 2 stages so that OSPM can handle
 *              various OS-specific tasks between the two steps.
 *
 ******************************************************************************/
acpi_status acpi_enter_sleep_state_prep(u8 sleep_state)
{
	acpi_status status;
	struct acpi_object_list arg_list;
	union acpi_object arg;

	ACPI_FUNCTION_TRACE(acpi_enter_sleep_state_prep);

	/* _PSW methods could be run here to enable wake-on keyboard, LAN, etc. */

	status = acpi_get_sleep_type_data(sleep_state,
					  &acpi_gbl_sleep_type_a,
					  &acpi_gbl_sleep_type_b);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Setup parameter object */

	arg_list.count = 1;
	arg_list.pointer = &arg;

	arg.type = ACPI_TYPE_INTEGER;
	arg.integer.value = sleep_state;

	/* Run the _PTS method */

	status = acpi_evaluate_object(NULL, METHOD_NAME__PTS, &arg_list, NULL);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		return_ACPI_STATUS(status);
	}

	/* Setup the argument to _SST */

	switch (sleep_state) {
	case ACPI_STATE_S0:
		arg.integer.value = ACPI_SST_WORKING;
		break;

	case ACPI_STATE_S1:
	case ACPI_STATE_S2:
	case ACPI_STATE_S3:
		arg.integer.value = ACPI_SST_SLEEPING;
		break;

	case ACPI_STATE_S4:
		arg.integer.value = ACPI_SST_SLEEP_CONTEXT;
		break;

	default:
		arg.integer.value = ACPI_SST_INDICATOR_OFF;	/* Default is off */
		break;
	}

	/*
	 * Set the system indicators to show the desired sleep state.
	 * _SST is an optional method (return no error if not found)
	 */
	status = acpi_evaluate_object(NULL, METHOD_NAME__SST, &arg_list, NULL);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		ACPI_EXCEPTION((AE_INFO, status,
				"While executing method _SST"));
	}

	return_ACPI_STATUS(AE_OK);
}

ACPI_EXPORT_SYMBOL(acpi_enter_sleep_state_prep)

static unsigned int gts, bfs;
module_param(gts, uint, 0644);
module_param(bfs, uint, 0644);
MODULE_PARM_DESC(gts, "Enable evaluation of _GTS on suspend.");
MODULE_PARM_DESC(bfs, "Enable evaluation of _BFS on resume".);

/*******************************************************************************
 *
 * FUNCTION:    acpi_enter_sleep_state
 *
 * PARAMETERS:  sleep_state         - Which sleep state to enter
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Enter a system sleep state (see ACPI 2.0 spec p 231)
 *              THIS FUNCTION MUST BE CALLED WITH INTERRUPTS DISABLED
 *
 ******************************************************************************/
acpi_status asmlinkage acpi_enter_sleep_state(u8 sleep_state)
{
	u32 pm1a_control;
	u32 pm1b_control;
	struct acpi_bit_register_info *sleep_type_reg_info;
	struct acpi_bit_register_info *sleep_enable_reg_info;
	u32 in_value;
	struct acpi_object_list arg_list;
	union acpi_object arg;
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_enter_sleep_state);

	if ((acpi_gbl_sleep_type_a > ACPI_SLEEP_TYPE_MAX) ||
	    (acpi_gbl_sleep_type_b > ACPI_SLEEP_TYPE_MAX)) {
		ACPI_ERROR((AE_INFO, "Sleep values out of range: A=%X B=%X",
			    acpi_gbl_sleep_type_a, acpi_gbl_sleep_type_b));
		return_ACPI_STATUS(AE_AML_OPERAND_VALUE);
	}

	sleep_type_reg_info =
	    acpi_hw_get_bit_register_info(ACPI_BITREG_SLEEP_TYPE);
	sleep_enable_reg_info =
	    acpi_hw_get_bit_register_info(ACPI_BITREG_SLEEP_ENABLE);

	/* Clear wake status */

	status =
	    acpi_write_bit_register(ACPI_BITREG_WAKE_STATUS, ACPI_CLEAR_STATUS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Clear all fixed and general purpose status bits */

	status = acpi_hw_clear_acpi_status();
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/*
	 * 1) Disable/Clear all GPEs
	 * 2) Enable all wakeup GPEs
	 */
	status = acpi_hw_disable_all_gpes();
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}
	acpi_gbl_system_awake_and_running = FALSE;

	status = acpi_hw_enable_all_wakeup_gpes();
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	if (gts) {
		/* Execute the _GTS method */

		arg_list.count = 1;
		arg_list.pointer = &arg;
		arg.type = ACPI_TYPE_INTEGER;
		arg.integer.value = sleep_state;

		status = acpi_evaluate_object(NULL, METHOD_NAME__GTS, &arg_list, NULL);
		if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
			return_ACPI_STATUS(status);
		}
	}

	/* Get current value of PM1A control */

	status = acpi_hw_register_read(ACPI_REGISTER_PM1_CONTROL,
				       &pm1a_control);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}
	ACPI_DEBUG_PRINT((ACPI_DB_INIT,
			  "Entering sleep state [S%d]\n", sleep_state));

	/* Clear the SLP_EN and SLP_TYP fields */

	pm1a_control &= ~(sleep_type_reg_info->access_bit_mask |
			  sleep_enable_reg_info->access_bit_mask);
	pm1b_control = pm1a_control;

	/* Insert the SLP_TYP bits */

	pm1a_control |=
	    (acpi_gbl_sleep_type_a << sleep_type_reg_info->bit_position);
	pm1b_control |=
	    (acpi_gbl_sleep_type_b << sleep_type_reg_info->bit_position);

	/*
	 * We split the writes of SLP_TYP and SLP_EN to workaround
	 * poorly implemented hardware.
	 */

	/* Write #1: write the SLP_TYP data to the PM1 Control registers */

	status = acpi_hw_write_pm1_control(pm1a_control, pm1b_control);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Insert the sleep enable (SLP_EN) bit */

	pm1a_control |= sleep_enable_reg_info->access_bit_mask;
	pm1b_control |= sleep_enable_reg_info->access_bit_mask;

	/* Flush caches, as per ACPI specification */

	ACPI_FLUSH_CPU_CACHE();

	/* Write #2: Write both SLP_TYP + SLP_EN */

	status = acpi_hw_write_pm1_control(pm1a_control, pm1b_control);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	if (sleep_state > ACPI_STATE_S3) {
		/*
		 * We wanted to sleep > S3, but it didn't happen (by virtue of the
		 * fact that we are still executing!)
		 *
		 * Wait ten seconds, then try again. This is to get S4/S5 to work on
		 * all machines.
		 *
		 * We wait so long to allow chipsets that poll this reg very slowly
		 * to still read the right value. Ideally, this block would go
		 * away entirely.
		 */
		acpi_os_stall(10000000);

		status = acpi_hw_register_write(ACPI_REGISTER_PM1_CONTROL,
						sleep_enable_reg_info->
						access_bit_mask);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}
	}

	/* Wait until we enter sleep state */

	do {
		status = acpi_read_bit_register(ACPI_BITREG_WAKE_STATUS,
						    &in_value);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		/* Spin until we wake */

	} while (!in_value);

	return_ACPI_STATUS(AE_OK);
}

ACPI_EXPORT_SYMBOL(acpi_enter_sleep_state)

/*******************************************************************************
 *
 * FUNCTION:    acpi_enter_sleep_state_s4bios
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Perform a S4 bios request.
 *              THIS FUNCTION MUST BE CALLED WITH INTERRUPTS DISABLED
 *
 ******************************************************************************/
acpi_status asmlinkage acpi_enter_sleep_state_s4bios(void)
{
	u32 in_value;
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_enter_sleep_state_s4bios);

	/* Clear the wake status bit (PM1) */

	status =
	    acpi_write_bit_register(ACPI_BITREG_WAKE_STATUS, ACPI_CLEAR_STATUS);
	if (ACPI_: ep0 not mapped\n");
		} else {
			usba_ep_writel(ep, CTL_ENB, USBA_EPT_ENABLE);
			usba_writel(udc, TST, USBA_TST_PKT_MODE);
			memcpy_toio(ep->fifo, test_packet_buffer,
					sizeof(test_packet_buffer));
			usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
			dev_info(dev, "Entering Test_Packet mode...\n");
		}
		break;
	default:
		dev_err(dev, "Invalid test mode: 0x%04x\n", test_mode);
		return -EINVAL;
	}

	return 0;
}

/* Avoid overly long expressions */
static inline bool feature_is_dev_remote_wakeup(struct usb_ctrlrequest *crq)
{
	if (crq->wValue == cpu_to_le16(USB_DEVICE_REMOTE_WAKEUP))
		return true;
	return false;
}

static inline bool feature_is_dev_test_mode(struct usb_ctrlrequest *crq)
{
	if (crq->wValue == cpu_to_le16(USB_DEVICE_TEST_MODE))
		return true;
	return false;
}

static inline bool feature_is_ep_halt(struct usb_ctrlrequest *crq)
{
	if (crq->wValue == cpu_to_le16(USB_ENDPOINT_HALT))
		return true;
	return false;
}

static int handle_ep0_setup(struct usba_udc *udc, struct usba_ep *ep,
		struct usb_ctrlrequest *crq)
{
	int retval = 0;

	switch (crq->bRequest) {
	case USB_REQ_GET_STATUS: {
		u16 status;

		if (crq->bRequestType == (USB_DIR_IN | USB_RECIP_DEVICE)) {
			status = cpu_to_le16(udc->devstatus);
		} else if (crq->bRequestType
				== (USB_DIR_IN | USB_RECIP_INTERFACE)) {
			status = cpu_to_le16(0);
		} else if (crq->bRequestType
				== (USB_DIR_IN | USB_RECIP_ENDPOINT)) {
			struct usba_ep *target;

			target = get_ep_by_addr(udc, le16_to_cpu(crq->wIndex));
			if (!target)
				goto stall;

			status = 0;
			if (is_stalled(udc, target))
				status |= cpu_to_le16(1);
		} else
			goto delegate;

		/* Write directly to the FIFO. No queueing is done. */
		if (crq->wLength != cpu_to_le16(sizeof(status)))
			goto stall;
		ep->state = DATA_STAGE_IN;
		__raw_writew(status, ep->fifo);
		usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
		break;
	}

	case USB_REQ_CLEAR_FEATURE: {
		if (crq->bRequestType == USB_RECIP_DEVICE) {
			if (feature_is_dev_remote_wakeup(crq))
				udc->devstatus
					&= ~(1 << USB_DEVICE_REMOTE_WAKEUP);
			else
				/* Can't CLEAR_FEATURE TEST_MODE */
				goto stall;
		} else if (crq->bRequestType == USB_RECIP_ENDPOINT) {
			struct usba_ep *target;

			if (crq->wLength != cpu_to_le16(0)
					|| !feature_is_ep_halt(crq))
				goto stall;
			target = get_ep_by_addr(udc, le16_to_cpu(crq->wIndex));
			if (!target)
				goto stall;

			usba_ep_writel(target, CLR_STA, USBA_FORCE_STALL);
			if (target->index != 0)
				usba_ep_writel(target, CLR_STA,
						USBA_TOGGLE_CLR);
		} else {
			goto delegate;
		}

		send_status(udc, ep);
		break;
	}

	case USB_REQ_SET_FEATURE: {
		if (crq->bRequestType == USB_RECIP_DEVICE) {
			if (feature_is_dev_test_mode(crq)) {
				send_status(udc, ep);
				ep->state = STATUS_STAGE_TEST;
				udc->test_mode = le16_to_cpu(crq->wIndex);
				return 0;
			} else if (feature_is_dev_remote_wakeup(crq)) {
				udc->devstatus |= 1 << USB_DEVICE_REMOTE_WAKEUP;
			} else {
				goto stall;
			}
		} else if (crq->bRequestType == USB_RECIP_ENDPOINT) {
			struct usba_ep *target;

			if (crq->wLength != cpu_to_le16(0)
					|| !feature_is_ep_halt(crq))
				goto stall;

			target = get_ep_by_addr(udc, le16_to_cpu(crq->wIndex));
			if (!target)
				goto stall;

			usba_ep_writel(target, SET_STA, USBA_FORCE_STALL);
		} else
			goto delegate;

		send_status(udc, ep);
		break;
	}

	case USB_REQ_SET_ADDRESS:
		if (crq->bRequestType != (USB_DIR_OUT | USB_RECIP_DEVICE))
			goto delegate;

		set_address(udc, le16_to_cpu(crq->wValue));
		send_status(udc, ep);
		ep->state = STATUS_STAGE_ADDR;
		break;

	default:
delegate:
		spin_unlock(&udc->lock);
		retval = udc->driver->setup(&udc->gadget, crq);
		spin_lock(&udc->lock);
	}

	return retval;

stall:
	pr_err("udc: %s: Invalid setup request: %02x.%02x v%04x i%04x l%d, "
		"halting endpoint...\n",
		ep->ep.name, crq->bRequestType, crq->bRequest,
		le16_to_cpu(crq->wValue), le16_to_cpu(crq->wIndex),
		le16_to_cpu(crq->wLength));
	set_protocol_stall(udc, ep);
	return -1;
}

static void usba_control_irq(struct usba_udc *udc, struct usba_ep *ep)
{
	struct  methods */

	arg.integer.value = ACPI_SST_WAKING;
	status = acpi_evaluate_object(NULL, METHOD_NAME__SST, &arg_list, NULL);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		ACPI_EXCEPTION((AE_INFO, status, "During Method _SST"));
	}

	/*
	 * GPEs must be enabled before _WAK is called as GPEs
	 * might get fired there
	 *
	 * Restore the GPEs:
	 * 1) Disable/Clear all GPEs
	 * 2) Enable all runtime GPEs
	 */
	status = acpi_hw_disable_all_gpes();
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}
	status = acpi_hw_enable_all_runtime_gpes();
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	arg.integer.value = sleep_state;
	status = acpi_evaluate_object(NULL, METHOD_NAME__WAK, &arg_list, NULL);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		ACPI_EXCEPTION((AE_INFO, status, "During Method _WAK"));
	}
	/* TBD: _WAK "sometimes" returns stuff - do we want to look at it? */

	/*
	 * Some BIOSes assume that WAK_STS will be cleared on resume and use
	 * it to determine whether the system is rebooting or resuming. Clear
	 * it for compatibility.
	 */
	acpi_write_bit_register(ACPI_BITREG_WAKE_STATUS, 1);

	acpi_gbl_system_awake_and_running = TRUE;

	/* Enable power button */

	(void)
	    acpi_write_bit_register(acpi_gbl_fixed_event_info
			      [ACPI_EVENT_POWER_BUTTON].
			      enable_register_id, ACPI_ENABLE_EVENT);

	(void)
	    acpi_write_bit_register(acpi_gbl_fixed_event_info
			      [ACPI_EVENT_POWER_BUTTON].
			      status_register_id, ACPI_CLEAR_STATUS);

	arg.integer.value = ACPI_SST_WORKING;
	status = acpi_evaluate_object(NULL, METHOD_NAME__SST, &arg_list, NULL);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		ACPI_EXCEPTION((AE_INFO, status, "During Method _SST"));
	}

	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_leave_sleep_state)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /*
 *  Driver for DVB-T lgdt3304 demodulator
 *
 *  Copyright (C) 2008 Markus Rechberger <mrechberger@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.=
 */

#ifndef LGDT3304_H
#define LGDT3304_H

#include <linux/dvb/frontend.h>

struct lgdt3304_config
{
	/* demodulator's I2C address */
	u8 i2c_address;
};

#if defined(CONFIG_DVB_LGDT3304) || (defined(CONFIG_DVB_LGDT3304_MODULE) && defined(MODULE))
extern struct dvb_frontend* lgdt3304_attach(const struct lgdt3304_config *config,
					   struct i2c_adapter *i2c);
#else
static inline struct dvb_frontend* lgdt3304_attach(const struct lgdt3304_config *config,
					   struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif /* CONFIG_DVB_LGDT */

#endif /* LGDT3304_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           config DVB_TTUSB_BUDGET
	tristate "Technotrend/Hauppauge Nova-USB devices"
	depends on DVB_CORE && USB && I2C && PCI
	select DVB_CX22700 if !DVB_FE_CUSTOMISE
	select DVB_TDA1004X if !DVB_FE_CUSTOMISE
	select DVB_VES1820 if !DVB_FE_CUSTOMISE
	select DVB_TDA8083 if !DVB_FE_CUSTOMISE
	select DVB_STV0299 if !DVB_FE_CUSTOMISE
	select DVB_STV0297 if !DVB_FE_CUSTOMISE
	select DVB_LNBP21 if !DVB_FE_CUSTOMISE
	help
	  Support for external USB adapters designed by Technotrend and
	  produced by Hauppauge, shipped under the brand name 'Nova-USB'.

	  These devices don't have a MPEG decoder built in, so you need
	  an external software decoder to watch TV.

	  Say Y if you own such a device and want to use it.
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            /*
    hexium_orion.c - v4l2 driver for the Hexium Orion frame grabber cards

    Visit http://www.mihu.de/linux/saa7146/ and follow the link
    to "hexium" for further details about this card.

    Copyright (C) 2003 Michael Hunold <michael@mihu.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define DEBUG_VARIABLE debug

#include <media/saa7146_vv.h>

static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debug verbosity");

/* global variables */
static int hexium_num;

#define HEXIUM_HV_PCI6_ORION		1
#define HEXIUM_ORION_1SVHS_3BNC		2
#define HEXIUM_ORION_4BNC		3

#define HEXIUM_INPUTS	9
static struct v4l2_input hexium_inputs[HEXIUM_INPUTS] = {
	{ 0, "CVBS 1",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 1, "CVBS 2",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 2, "CVBS 3",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 3, "CVBS 4",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 4, "CVBS 5",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 5, "CVBS 6",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 6, "Y/C 1",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 7, "Y/C 2",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
	{ 8, "Y/C 3",	V4L2_INPUT_TYPE_CAMERA,	2, 0, V4L2_STD_PAL_BG|V4L2_STD_NTSC_M, 0 },
};

#define HEXIUM_AUDIOS	0

struct hexium_data
{
	s8 adr;
	u8 byte;
};

struct hexium
{
	int type;
	struct video_device	*video_dev;
	struct i2c_adapter	i2c_adapter;

	int cur_input;	/* current input */
};

/* Philips SAA7110 decoder default registers */
static u8 hexium_saa7110[53]={
/*00*/ 0x4C,0x3C,0x0D,0xEF,0xBD,0xF0,0x00,0x00,
/*08*/ 0xF8,0xF8,0x60,0x60,0x40,0x86,0x18,0x90,
/*10*/ 0x00,0x2C,0x40,0x46,0x42,0x1A,0xFF,0xDA,
/*18*/ 0xF0,0x8B,0x00,0x00,0x00,0x00,0x00,0x00,
/*20*/ 0xD9,0x17,0x40,0x41,0x80,0x41,0x80,0x4F,
/*28*/ 0xFE,0x01,0x0F,0x0F,0x03,0x01,0x81,0x03,
/*30*/ 0x44,0x75,0x01,0x8C,0x03
};

static struct {
	struct hexium_data data[8];
} hexium_input_select[] = {
{
	{ /* cvbs 1 */
		{ 0x06, 0x00 },
		{ 0x20, 0xD9 },
		{ 0x21, 0x17 }, // 0x16,
		{ 0x22, 0x40 },
		{ 0x2C, 0x03 },
		{ 0x30, 0x44 },
		{ 0x31, 0x75 }, // ??
		{ 0x21, 0x16 }, // 0x03,
	}
}, {
	{ /* cvbs 2 */
		{ 0x06, 0x00 },
		{ 0x20, 0x78 },
		{ 0x21, 0x07 }, // 0x03,
		{ 0x22, 0xD2 },
		{ 0x2C, 0x83 },
		{ 0x30, 0x60 },
		{ 0x31, 0xB5 }, // ?
		{ 0x21, 0x03 },
	}
}, {
	{ /* cvbs 3 */
		{ 0x06, 0x00 },
		{ 0x20, 0xBA },
		{ 0x21, 0x07 }, // 0x05,
		{ 0x22, 0x91 },
		{ 0x2C, 0x03 },
		{ 0x30, 0x60 },
		{ 0x31, 0xB5 }, // ??
		{ 0x21, 0x05 }, // 0x03,
	}
}, {
	{ /* cvbs 4 */
		{ 0x06, 0x00 },
		{ 0x20, 0xD8 },
		{ 0x21, 0x17 }, // 0x16,
		{ 0x22, 0x40 },
		{ 0x2C, 0x03 },
		{ 0x30, 0x44 },
		{ 0x31, 0x75 }, // ??
		{ 0x21, 0x16 }, // 0x03,
	}
}, {
	{ /* cvbs 5 */
		{ 0x06, 0x00 },
		{ 0x20, 0xB8 },
		{ 0x21, 0x07 }, // 0x05,
		{ 0x22, 0x91 },
		{ 0x2C, 0x03 },
		{ 0x30, 0x60 },
		{ 0x31, 0xB5 }, // ??
		{ 0x21, 0x05 }, // 0x03,
	}
}, {
	{ /* cvbs 6 */
		{ 0x06, 0x00 },
		{ 0x20, 0x7C },
		{ 0x21, 0x07 }, // 0x03
		{ 0x22, 0xD2 },
		{ 0x2C, 0x83 },
		{ 0x30, 0x60 },
		{ 0x31, 0xB5 }, // ??
		{ 0x21, 0x03 },
	}
}, {
	{ /* y/c 1 */
		{ 0x06, 0x80 },
		{ 0x20, 0x59 },
		{ 0x21, 0x17 },
		{ 0x22, 0x42 },
		{ 0x2C, 0xA3 },
		{ 0x30, 0x44 },
		{ 0x31, 0x75 },
		{ 0x21, 0x12 },
	}
}, {
	{ /* y/c 2 */
		{ 0x06, 0x80 },
		{ 0x20, 0x9A },
		{ 0x21, 0x17 },
		{ 0x22, 0xB1 },
		{ 0x2C, 0x13 },
		{ 0x30, 0x60 },
		{ 0x31, 0xB5 },
		{ 0x21, 0x14 },
	}
}, {
	{ /* y/c 3 */
		{ 0x06, 0x80 },
		{ 0x20, 0x3C },
		{ 0x21, 0x27 },
		{ 0x22, 0xC1 },
		{ 0x2C, 0x23 },
		{ 0x30, 0x44 },
		{ 0x31, 0x75 },
		{ 0x21, 0x21 },
	}
}
};

static struct saa7146_standard hexium_standards[] = {
	{
		.name	= "PAL", 	.id	= V4L2_STD_PAL,
		.v_offset	= 16,	.v_field 	= 288,
		.h_offset	= 1,	.h_pixels 	= 680,
		.v_max_out	= 576,	.h_max_out	= 768,
	}, {
		.name	= "NTSC", 	.id	= V4L2_STD_NTSC,
		.v_offset	= 16,	.v_field 	= 240,
		.h_offset	= 1,	.h_pixels 	= 640,
		.v_max_out	= 480,	.h_max_out	= 640,
	}, {
		.name	= "SECAM", 	.id	= V4L2_STD_SECAM,
		.v_offset	= 16,	.v_field 	= 288,
		.h_offset	= 1,	.h_pixels 	= 720,
		.v_max_out	= 576,	.h_max_out	= 768,
	}
};

/* this is only called for old HV-PCI6/Orion cards
   without eeprom */
static int hexium_probe(struct saa7146_dev *dev)
{
	struct hexium *hexium = NULL;
	union i2c_smbus_data data;
	int err = 0;

	DEB_EE((".\n"));

	/* there are no hexium orion cards with revision 0 saa7146s */
	if (0 == dev->revision) {
		return -EFAULT;
	}

	hexium = kzalloc(sizeof(struct hexium), GFP_KERNEL);
	if (NULL == hexium) {
		printk("hexium_orion: hexium_probe: not enough kernel memory.\n");
		return -ENOMEM;
	}

	/* enable i2c-port pins */
	saa7146_write(dev, MC1, (MASK_08 | MASK_24 | MASK_10 | MASK_26));

	saa7146_write(dev, DD1_INIT, 0x01000100);
	saa7146_write(dev, DD1_STREAM_B, 0x00000000);
	saa7146_write(dev, MC2, (MASK_09 | MASK_25 | MASK_10 | MASK_26));

	hexium->i2c_adapter = (struct i2c_adapter) {
		.class = I2C_CLASS_TV_ANALOG,
		.name = "hexium orion",
	};
	saa7146_i2c_adapter_prepare(dev, &hexium->i2c_adapter, SAA7146_I2C_BUS_BIT_RATE_480);
	if (i