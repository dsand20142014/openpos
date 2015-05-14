 the chip... */
	if (wol->wolopts)
		pegasus->eth_regs[0] |= 0x10;
	else
		pegasus->eth_regs[0] &= ~0x10;
	pegasus->wolopts = wol->wolopts;
	return set_register(pegasus, WakeupControl, reg78);
}

static inline void pegasus_reset_wol(struct net_device *dev)
{
	struct ethtool_wolinfo wol;
	
	memset(&wol, 0, sizeof wol);
	(void) pegasus_set_wol(dev, &wol);
}

static int
pegasus_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
	pegasus_t *pegasus;

	pegasus = netdev_priv(dev);
	mii_ethtool_gset(&pegasus->mii, ecmd);
	return 0;
}

static int
pegasus_set_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
	pegasus_t *pegasus = netdev_priv(dev);
	return mii_ethtool_sset(&pegasus->mii, ecmd);
}

static int pegasus_nway_reset(struct net_device *dev)
{
	pegasus_t *pegasus = netdev_priv(dev);
	return mii_nway_restart(&pegasus->mii);
}

static u32 pegasus_get_link(struct net_device *dev)
{
	pegasus_t *pegasus = netdev_priv(dev);
	return mii_link_ok(&pegasus->mii);
}

static u32 pegasus_get_msglevel(struct net_device *dev)
{
	pegasus_t *pegasus = netdev_priv(dev);
	return pegasus->msg_enable;
}

static void pegasus_set_msglevel(struct net_device *dev, u32 v)
{
	pegasus_t *pegasus = netdev_priv(dev);
	pegasus->msg_enable = v;
}

static struct ethtool_ops ops = {
	.get_drvinfo = pegasus_get_drvinfo,
	.get_settings = pegasus_get_settings,
	.set_settings = pegasus_set_settings,
	.nway_reset = pegasus_nway_reset,
	.get_link = pegasus_get_link,
	.get_msglevel = pegasus_get_msglevel,
	.set_msglevel = pegasus_set_msglevel,
	.get_wol = pegasus_get_wol,
	.set_wol = pegasus_set_wol,
};

static int pegasus_ioctl(struct net_device *net, struct ifreq *rq, int cmd)
{
	__u16 *data = (__u16 *) & rq->ifr_ifru;
	pegasus_t *pegasus = netdev_priv(net);
	int res;

	switch (cmd) {
	case SIOCDEVPRIVATE:
		data[0] = pegasus->phy;
	case SIOCDEVPRIVATE + 1:
		read_mii_word(pegasus, data[0], data[1] & 0x1f, &data[3]);
		res = 0;
		break;
	case SIOCDEVPRIVATE + 2:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		write_mii_word(pegasus, pegasus->phy, data[1] & 0x1f, data[2]);
		res = 0;
		break;
	default:
		res = -EOPNOTSUPP;
	}
	return res;
}

static void pegasus_set_multicast(struct net_device *net)
{
	pegasus_t *pegasus = netdev_priv(net);

	if (net->flags & IFF_PROMISC) {
		pegasus->eth_regs[EthCtrl2] |= RX_PROMISCUOUS;
		if (netif_msg_link(pegasus))
			pr_info("%s: Promiscuous mode enabled.\n", net->name);
	} else if (net->mc_count || (net->flags & IFF_ALLMULTI)) {
		pegasus->eth_regs[EthCtrl0] |= RX_MULTICAST;
		pegasus->eth_regs[EthCtrl2] &= ~RX_PROMISCUOUS;
		if (netif_msg_link(pegasus))
			pr_debug("%s: set allmulti\n", net->name);
	} else {
		pegasus->eth_regs[EthCtrl0] &= ~RX_MULTICAST;
		pegasus->eth_regs[EthCtrl2] &= ~RX_PROMISCUOUS;
	}

	pegasus->ctrl_urb->status = 0;

	pegasus->flags |= ETH_REGS_CHANGE;
	ctrl_callback(pegasus->ctrl_urb);
}

static __u8 mii_phy_probe(pegasus_t * pegasus)
{
	int i;
	__u16 tmp;

	for (i = 0; i < 32; i++) {
		read_mii_word(pegasus, i, MII_BMSR, &tmp);
		if (tmp == 0 || tmp == 0xffff || (tmp & BMSR_MEDIA) == 0)
			continue;
		else
			return i;
	}

	return 0xff;
}

static inline void setup_pegasus_II(pegasus_t * pegasus)
{
	__u8 data = 0xa5;
	
	set_register(pegasus, Reg1d, 0);
	set_register(pegasus, Reg7b, 1);
	mdelay(100);
	if ((pegasus->features & HAS_HOME_PNA) && mii_mode)
		set_register(pegasus, Reg7b, 0);
	else
		set_register(pegasus, Reg7b, 2);

	set_register(pegasus, 0x83, data);
	get_registers(pegasus, 0x83, 1, &data);

	if (data == 0xa5) {
		pegasus->chip = 0x8513;
	} else {
		pegasus->chip = 0;
	}

	set_register(pegasus, 0x80, 0xc0);
	set_register(pegasus, 0x83, 0xff);
	set_register(pegasus, 0x84, 0x01);
	
	if (pegasus->features & HAS_HOME_PNA && mii_mode)
		set_register(pegasus, Reg81, 6);
	else
		set_register(pegasus, Reg81, 2);
}


static int pegasus_count;
static struct workqueue_struct *pegasus_workqueue = NULL;
#define CARRIER_CHECK_DELAY (2 * HZ)

static void check_carrier(struct work_struct *work)
{
	pegasus_t *pegasus = container_of(work, pegasus_t, carrier_check.work);
	set_carrier(pegasus->net);
	if (!(pegasus->flags & PEGASUS_UNPLUG)) {
		queue_delayed_work(pegasus_workqueue, &pegasus->carrier_check,
			CARRIER_CHECK_DELAY);
	}
}

static int pegasus_blacklisted(struct usb_device *udev)
{
	struct usb_device_descriptor *udd = &udev->descriptor;

	/* Special quirk to keep the driver from handling the Belkin Bluetooth
	 * dongle which happens to have the same ID.
	 */
	if ((udd->idVendor == cpu_to_le16(VENDOR_BELKIN)) &&
	    (udd->idProduct == cpu_to_le16(0x0121)) &&
	    (udd->bDeviceClass == USB_CLASS_WIRELESS_CONTROLLER) &&
	    (udd->bDeviceProtocol == 1))
		return 1;

	return 0;
}

/* we rely on probe() and remove() being serialized so we
 * don't need extra locking on pegasus_count.
 */
static void pegasus_dec_workqueue(void)
{
	pegasus_count--;
	if (pegasus_count == 0) {
		destroy_workqueue(pegasus_workqueue);
		pegasus_workqueue = NULL;
	}
}

static int pegasus_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct net_device *net;
	pegasus_t *pegasus;
	int dev_index = id - pegasus_ids;
	int res = -ENOMEM;

	if (pegasus_blacklisted(dev))
		return -ENODEV;

	if (pegasus_count == 0) {
		pegasus_workqueue = create_singlethread_workqueue("pegasus");
		if (!pegasus_workqueue)
			return -ENOMEM;
	}
	pegasus_count++;

	usb_get_dev(dev);

	net = alloc_etherdev(sizeof(struct pegasus));
	if (!net) {
		dev_err(&intf->dev, "can't allocate %s\n", "device");
		goto out;
	}

	pegasus = netdev_priv(net);
	pegasus->dev_index = dev_index;
	init_waitqueue_head(&pegasus->ctrl_wait);

	if (!alloc_urbs(pegasus)) {
		dev_err(&intf->dev, "can't allocate %s\n", "urbs");
		goto out1;
	}

	tasklet_init(&pegasus->rx_tl, rx_fixup, (unsigned long) pegasus);

	INIT_DELAYED_WORK(&pegasus->carrier_check, check_carrier);

	pegasus->intf = intf;
	pegasus->usb = dev;
	pegasus->net = net;


	net->watchdog_timeo = PEGASUS_TX_TIMEOUT;
	net->netdev_ops = &pegasus_netdev_ops;
	SET_ETHTOOL_OPS(net, &ops);
	pegasus->mii.dev = net;
	pegasus->mii.mdio_read = mdio_read;
	pegasus->mii.mdio_write = mdio_write;
	pegasus->mii.phy_id_mask = 0x1f;
	pegasus->mii.reg_num_mask = 0x1f;
	spin_lock_init(&pegasus->rx_pool_lock);
	pegasus->msg_enable = netif_msg_init (msg_level, NETIF_MSG_DRV
				| NETIF_MSG_PROBE | NETIF_MSG_LINK);

	pegasus->features = usb_dev_id[dev_index].private;
	get_interrupt_interval(pegasus);
	if (reset_mac(pegasus)) {
		dev_err(&intf->dev, "can't reset MAC\n");
		res = -EIO;
		goto out2;
	}
	set_ethernet_addr(pegasus);
	fill_skb_pool(pegasus);
	if (pegasus->features & PEGASUS_II) {
		dev_info(&intf->dev, "setup Pegasus II specific registers\n");
		setup_pegasus_II(pegasus);
	}
	pegasus->phy = mii_phy_probe(pegasus);
	if (pegasus->phy == 0xff) {
		dev_warn(&intf->dev, "can't locate MII phy, using default\n");
		pegasus->phy = 1;
	}
	pegasus->mii.phy_id = pegasus->phy;
	usb_set_intfdata(intf, pegasus);
	SET_NETDEV_DEV(net, &intf->dev);
	pegasus_reset_wol(net);
	res = register_netdev(net);
	if (res)
		goto out3;
	queue_delayed_work(pegasus_workqueue, &pegasus->carrier_check,
				CARRIER_CHECK_DELAY);

	dev_info(&intf->dev, "%s, %s, %pM\n",
		 net->name,
		 usb_dev_id[dev_index].name,
		 net->dev_addr);
	return 0;

out3:
	usb_set_intfdata(intf, NULL);
	free_skb_pool(pegasus);
out2:
	free_all_urbs(pegasus);
out1:
	free_netdev(net);
out:
	usb_put_dev(dev);
	pegasus_dec_workqueue();
	return res;
}

static void pegasus_disconnect(struct usb_interface *intf)
{
	struct pegasus *pegasus = usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	if (!pegasus) {
		dev_dbg(&intf->dev, "unregistering non-bound device?\n");
		return;
	}

	pegasus->flags |= PEGASUS_UNPLUG;
	cancel_delayed_work(&pegasus->carrier_check);
	unregister_netdev(pegasus->net);
	usb_put_dev(interface_to_usbdev(intf));
	unlink_all_urbs(pegasus);
	free_all_urbs(pegasus);
	free_skb_pool(pegasus);
	if (pegasus->rx_skb != NULL) {
		dev_kfree_skb(pegasus->rx_skb);
		pegasus->rx_skb = NULL;
	}
	free_netdev(pegasus->net);
	pegasus_dec_workqueue();
}

static int pegasus_suspend (struct usb_interface *intf, pm_message_t message)
{
	struct pegasus *pegasus = usb_get_intfdata(intf);
	
	netif_device_detach (pegasus->net);
	cancel_delayed_work(&pegasus->carrier_check);
	if (netif_running(pegasus->net)) {
		usb_kill_urb(pegasus->rx_urb);
		usb_kill_urb(pegasus->intr_urb);
	}
	return 0;
}

static int pegasus_resume (struct usb_interface *intf)
{
	struct pegasus *pegasus = usb_get_intfdata(intf);

	netif_device_attach (pegasus->net);
	if (netif_running(pegasus->net)) {
		pegasus->rx_urb->status = 0;
		pegasus->rx_urb->actual_length = 0;
		read_bulk_callback(pegasus->rx_urb);

		pegasus->intr_urb->status = 0;
		pegasus->intr_urb->actual_length = 0;
		intr_callback(pegasus->intr_urb);
	}
	queue_delayed_work(pegasus_workqueue, &pegasus->carrier_check,
				CARRIER_CHECK_DELAY);
	return 0;
}

static const struct net_device_ops pegasus_netdev_ops = {
	.ndo_open =			pegasus_open,
	.ndo_stop =			pegasus_close,
	.ndo_do_ioctl =			pegasus_ioctl,
	.ndo_start_xmit =		pegasus_start_xmit,
	.ndo_set_multicast_list =	pegasus_set_multicast,
	.ndo_get_stats =		pegasus_netdev_stats,
	.ndo_tx_timeout =		pegasus_tx_timeout,
	.ndo_change_mtu =		eth_change_mtu,
	.ndo_set_mac_address =		eth_mac_addr,
	.ndo_validate_addr =		eth_validate_addr,
};

static struct usb_driver pegasus_driver = {
	.name = driver_name,
	.probe = pegasus_probe,
	.disconnect = pegasus_disconnect,
	.id_table = pegasus_ids,
	.suspend = pegasus_suspend,
	.resume = pegasus_resume,
};

static void __init parse_id(char *id)
{
	unsigned int vendor_id=0, device_id=0, flags=0, i=0;
	char *token, *name=NULL;

	if ((token = strsep(&id, ":")) != NULL)
		name = token;
	/* name now points to a null terminated string*/
	if ((token = strsep(&id, ":")) != NULL)
		vendor_id = simple_strtoul(token, NULL, 16);
	if ((token = strsep(&id, ":")) != NULL)
		device_id = simple_strtoul(token, NULL, 16);
	flags = simple_strtoul(id, NULL, 16);
	pr_info("%s: new device %s, vendor ID 0x%04x, device ID 0x%04x, flags: 0x%x\n",
	        driver_name, name, vendor_id, device_id, flags);

	if (vendor_id > 0x10000 || vendor_id == 0)
		return;
	if (device_id > 0x10000 || device_id == 0)
		return;

	for (i=0; usb_dev_id[i].name; i++);
	usb_dev_id[i].name = name;
	usb_dev_id[i].vendor = vendor_id;
	usb_dev_id[i].device = device_id;
	usb_dev_id[i].private = flags;
	pegasus_ids[i].match_flags = USB_DEVICE_ID_MATCH_DEVICE;
	pegasus_ids[i].idVendor = vendor_id;
	pegasus_ids[i].idProduct = device_id;
}

static int __init pegasus_init(void)
{
	pr_info("%s: %s, " DRIVER_DESC "\n", driver_name, DRIVER_VERSION);
	if (devid)
		parse_id(devid);
	return usb_register(&pegasus_driver);
}

static void __exit pegasus_exit(void)
{
	usb_deregister(&pegasus_driver);
}

module_init(pegasus_init);
module_exit(pegasus_exit);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               /* DVB USB compliant linux driver for MSI Mega Sky 580 DVB-T USB2.0 receiver
 *
 * Copyright (C) 2006 Aapo Tahkola (aet@rasterburn.org)
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation, version 2.
 *
 * see Documentation/dvb/README.dvb-usb for more information
 */

#include "m920x.h"

#include "mt352.h"
#include "mt352_priv.h"
#include "qt1010.h"
#include "tda1004x.h"
#include "tda827x.h"
#include <asm/unaligned.h>

/* debug */
static int dvb_usb_m920x_debug;
module_param_named(debug,dvb_usb_m920x_debug, int, 0644);
MODULE_PARM_DESC(debug, "set debugging level (1=rc (or-able))." DVB_USB_DEBUG_STATUS);

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static int m920x_set_filter(struct dvb_usb_device *d, int type, int idx, int pid);

static inline int m920x_read(struct usb_device *udev, u8 request, u16 value,
			     u16 index, void *data, int size)
{
	int ret;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      request, USB_TYPE_VENDOR | USB_DIR_IN,
			      value, index, data, size, 2000);
	if (ret < 0) {
		printk(KERN_INFO "m920x_read = error: %d\n", ret);
		return ret;
	}

	if (ret != size) {
		deb("m920x_read = no data\n");
		return -EIO;
	}

	return 0;
}

static inline int m920x_write(struct usb_device *udev, u8 request,
			      u16 value, u16 index)
{
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      request, USB_TYPE_VENDOR | USB_DIR_OUT,
			      value, index, NULL, 0, 2000);

	return ret;
}

static int m920x_init(struct dvb_usb_device *d, struct m920x_inits *rc_seq)
{
	int ret = 0, i, epi, flags = 0;
	int adap_enabled[M9206_MAX_ADAPTERS] = { 0 };

	/* Remote controller init. */
	if (d->props.rc_query) {
		deb("Initialising remote control\n");
		while (rc_seq->address) {
			if ((ret = m920x_write(d->udev, M9206_CORE,
					       rc_seq->data,
					       rc_seq->address)) != 0) {
				deb("Initialising remote control failed\n");
				return ret;
			}

			rc_seq++;
		}

		deb("Initialising remote control success\n");
	}

	for (i = 0; i < d->props.num_adapters; i++)
		flags |= d->adapter[i].props.caps;

	/* Some devices(Dposh) might crash if we attempt touch at all. */
	if (flags & DVB_USB_ADAP_HAS_PID_FILTER) {
		for (i = 0; i < d->props.num_adapters; i++) {
			epi = d->adapter[i].props.stream.endpoint - 0x81;

			if (epi < 0 || epi >= M9206_MAX_ADAPTERS) {
				printk(KERN_INFO "m920x: Unexpected adapter endpoint!\n");
				return -EINVAL;
			}

			adap_enabled[epi] = 1;
		}

		for (i = 0; i < M9206_MAX_ADAPTERS; i++) {
			if (adap_enabled[i])
				continue;

			if ((ret = m920x_set_filter(d, 0x81 + i, 0, 0x0)) != 0)
				return ret;

			if ((ret = m920x_set_filter(d, 0x81 + i, 0, 0x02f5)) != 0)
				return ret;
		}
	}

	return ret;
}

static int m920x_init_ep(struct usb_interface *intf)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_host_interface *alt;

	if ((alt = usb_altnum_to_altsetting(intf, 1)) == NULL) {
		deb("No alt found!\n");
		return -ENODEV;
	}

	return usb_set_interface(udev, alt->desc.bInterfaceNumber,
				 alt->desc.bAlternateSetting);
}

static int m920x_rc_query(struct dvb_usb_device *d, u32 *event, int *state)
{
	struct m920x_state *m = d->priv;
	int i, ret = 0;
	u8 rc_state[2];

	if ((ret = m920x_read(d->udev, M9206_CORE, 0x0, M9206_RC_STATE, rc_state, 1)) != 0)
		goto unlock;

	if ((ret = m920x_read(d->udev, M9206_CORE, 0x0, M9206_RC_KEY, rc_state + 1, 1)) != 0)
		goto unlock;

	for (i = 0; i < d->props.rc_key_map_size; i++)
		if (d->props.rc_key_map[i].data == rc_state[1]) {
			*event = d->props.rc_key_map[i].event;

			switch(rc_state[0]) {
			case 0x80:
				*state = REMOTE_NO_KEY_PRESSED;
				goto unlock;

			case 0x88: /* framing error or "invalid code" */
			case 0x99:
			case 0xc0:
			case 0xd8:
				*state = REMOTE_NO_KEY_PRESSED;
				m->rep_count = 0;
				goto unlock;

			case 0x93:
			case 0x92:
				m->rep_count = 0;
				*state = REMOTE_KEY_PRESSED;
				goto unlock;

			case 0x91:
				/* prevent immediate auto-repeat */
				if (++m->rep_count > 2)
					*state = REMOTE_KEY_REPEAT;
				else
					*state = REMOTE_NO_KEY_PRESSED;
				goto unlock;

			default:
				deb("Unexpected rc state %02x\n", rc_state[0]);
				*state = REMOTE_NO_KEY_PRESSED;
				goto unlock;
			}
		}

	if (rc_state[1] != 0)
		deb("Unknown rc key %02x\n", rc_state[1]);

	*state = REMOTE_NO_KEY_PRESSED;

 unlock:

	return ret;
}

/* I2C */
static int m920x_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msg[], int num)
{
	struct dvb_usb_device *d = i2c_get_adapdata(adap);
	int i, j;
	int ret = 0;

	if (!num)
		return -EINVAL;

	if (mutex_lock_interruptible(&d->i2c_mutex) < 0)
		return -EAGAIN;

	for (i = 0; i < num; i++) {
		if (msg[i].flags & (I2C_M_NO_RD_ACK | I2C_M_IGNORE_NAK | I2C_M_TEN) || msg[i].len == 0) {
			/* For a 0 byte message, I think sending the address
			 * to index 0x80|0x40 would be the correct thing to
			 * do.  However, zero byte messages are only used for
			 * probing, and since we don't know how to get the
			 * slave's ack, we can't probe. */
			ret = -ENOTSUPP;
			goto unlock;
		}
		/* Send START & address/RW bit */
		if (!(msg[i].flags & I2C_M_NOSTART)) {
			if ((ret = m920x_write(d->udev, M9206_I2C,
					(msg[i].addr << 1) |
					(msg[i].flags & I2C_M_RD ? 0x01 : 0), 0x80)) != 0)
				goto unlock;
			/* Should check for ack here, if we knew how. */
		}
		if (msg[i].flags & I2C_M_RD) {
			for (j = 0; j < msg[i].len; j++) {
				/* Last byte of transaction?
				 * Send STOP, otherwise send ACK. */
				int stop = (i+1 == num && j+1 == msg[i].len) ? 0x40 : 0x01;

				if ((ret = m920x_read(d->udev, M9206_I2C, 0x0,
						      0x20 | stop,
						      &msg[i].buf[j], 1)) != 0)
					goto unlock;
			}
		} else {
			for (j = 0; j < msg[i].len; j++) {
				/* Last byte of transaction? Then send STOP. */
				int stop = (i+1 == num && j+1 == msg[i].len) ? 0x40 : 0x00;

				if ((ret = m920x_write(d->udev, M9206_I2C, msg[i].buf[j], stop)) != 0)
					goto unlock;
				/* Should check for ack here too. */
			}
		}
	}
	ret = num;

 unlock:
	mutex_unlock(&d->i2c_mutex);

	return ret;
}

static u32 m920x_i2c_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C;
}

static struct i2c_algorithm m920x_i2c_algo = {
	.master_xfer   = m920x_i2c_xfer,
	.functionality = m920x_i2c_func,
};

/* pid filter */
static int m920x_set_filter(struct dvb_usb_device *d, int type, int idx, int pid)
{
	int ret = 0;

	if (pid >= 0x8000)
		return -EINVAL;

	pid |= 0x8000;

	if ((ret = m920x_write(d->udev, M9206_FILTER, pid, (type << 8) | (idx * 4) )) != 0)
		return ret;

	if ((ret = m920x_write(d->udev, M9206_FILTER, 0, (type << 8) | (idx * 4) )) != 0)
		return ret;

	return ret;
}

static int m920x_update_filters(struct dvb_usb_adapter *adap)
{
	struct m920x_state *m = adap->dev->priv;
	int enabled = m->filtering_enabled[adap->id];
	int i, ret = 0, filter = 0;
	int ep = adap->props.stream.endpoint;

	for (i = 0; i < M9206_MAX_FILTERS; i++)
		if (m->filters[adap->id][i] == 8192)
			enabled = 0;

	/* Disable all filters */
	if ((ret = m920x_set_filter(adap->dev, ep, 1, enabled)) != 0)
		return ret;

	for (i = 0; i < M9206_MAX_FILTERS; i++)
		if ((ret = m920x_set_filter(adap->dev, ep, i + 2, 0)) != 0)
			return ret;

	/* Set */
	if (enabled) {
		for (i = 0; i < M9206_MAX_FILTERS; i++) {
			if (m->filters[adap->id][i] == 0)
				continue;

			if ((ret = m920x_set_filter(adap->dev, ep, filter + 2, m->filters[adap->id][i])) != 0)
				return ret;

			filter++;
		}
	}

	return ret;
}

static int m920x_pid_filter_ctrl(struct dvb_usb_adapter *adap, int onoff)
{
	struct m920x_state *m = adap->dev->priv;

	m->filtering_enabled[adap->id] = onoff ? 1 : 0;

	return m920x_update_filters(adap);
}

static int m920x_pid_filter(struct dvb_usb_adapter *adap, int index, u16 pid, int onoff)
{
	struct m920x_state *m = adap->dev->priv;

	m->filters[adap->id][index] = onoff ? pid : 0;

	return m920x_update_filters(adap);
}

static int m920x_firmware_download(struct usb_device *udev, const struct firmware *fw)
{
	u16 value, index, size;
	u8 read[4], *buff;
	int i, pass, ret = 0;

	buff = kmalloc(65536, GFP_KERNEL);

	if ((ret = m920x_read(udev, M9206_FILTER, 0x0, 0x8000, read, 4)) != 0)
		goto done;
	deb("%x %x %x %x\n", read[0], read[1], read[2], read[3]);

	if ((ret = m920x_read(udev, M9206_FW, 0x0, 0x0, read, 1)) != 0)
		goto done;
	deb("%x\n", read[0]);

	for (pass = 0; pass < 2; pass++) {
		for (i = 0; i + (sizeof(u16) * 3) < fw->size;) {
			value = get_unaligned_le16(fw->data + i);
			i += sizeof(u16);

			index = get_unaligned_le16(fw->data + i);
			i += sizeof(u16);

			size = get_unaligned_le16(fw->data + i);
			i += sizeof(u16);

			if (pass == 1) {
				/* Will stall if using fw->data ... */
				memcpy(buff, fw->data + i, size);

				ret = usb_control_msg(udev, usb_sndctrlpipe(udev,0),
						      M9206_FW,
						      USB_TYPE_VENDOR | USB_DIR_OUT,
						      value, index, buff, size, 20);
				if (ret != size) {
					deb("error while uploading fw!\n");
					ret = -EIO;
					goto done;
				}
				msleep(3);
			}
			i += size;
		}
		if (i != fw->size) {
			deb("bad firmware file!\n");
			ret = -EINVAL;
			goto done;
		}
	}

	msleep(36);

	/* m920x will disconnect itself from the bus after this. */
	(void) m920x_write(udev, M9206_CORE, 0x01, M9206_FW_GO);
	deb("firmware uploaded!\n");

 done:
	kfree(buff);

	return ret;
}

/* Callbacks for DVB USB */
static int m920x_identify_state(struct usb_device *udev,
				struct dvb_usb_device_properties *props,
				struct dvb_usb_device_description **desc,
				int *cold)
{
	struct usb_host_interface *alt;

	alt = usb_altnum_to_altsetting(usb_ifnum_to_if(udev, 0), 1);
	*cold = (alt == NULL) ? 1 : 0;

	return 0;
}

/* demod configurations */
static int m920x_mt352_demod_init(struct dvb_frontend *fe)
{
	int ret;
	u8 config[] = { CONFIG, 0x3d };
	u8 clock[] = { CLOCK_CTL, 0x30 };
	u8 reset[] = { RESET, 0x80 };
	u8 adc_ctl[] = { ADC_CTL_1, 0x40 };
	u8 agc[] = { AGC_TARGET, 0x1c, 0x20 };
	u8 sec_agc[] = { 0x69, 0x00, 0xff, 0xff, 0x40, 0xff, 0x00, 0x40, 0x40 };
	u8 unk1[] = { 0x93, 0x1a };
	u8 unk2[] = { 0xb5, 0x7a };

	deb("Demod init!\n");

	if ((ret = mt352_write(fe, config, ARRAY_SIZE(config))) != 0)
		return ret;
	if ((ret = mt352_write(fe, clock, ARRAY_SIZE(clock))) != 0)
		return ret;
	if ((ret = mt352_write(fe, reset, ARRAY_SIZE(reset))) != 0)
		return ret;
	if ((ret = mt352_write(fe, adc_ctl, ARRAY_SIZE(adc_ctl))) != 0)
		return ret;
	if ((ret = mt352_write(fe, agc, ARRAY_SIZE(agc))) != 0)
		return ret;
	if ((ret = mt352_write(fe, sec_agc, ARRAY_SIZE(sec_agc))) != 0)
		return ret;
	if ((ret = mt352_write(fe, unk1, ARRAY_SIZE(unk1))) != 0)
		return ret;
	if ((ret = mt352_write(fe, unk2, ARRAY_SIZE(unk2))) != 0)
		return ret;

	return 0;
}

static struct mt352_config m920x_mt352_config = {
	.demod_address = 0x0f,
	.no_tuner = 1,
	.demod_init = m920x_mt352_demod_init,
};

static struct tda1004x_config m920x_tda10046_08_config = {
	.demod_address = 0x08,
	.invert = 0,
	.invert_oclk = 0,
	.ts_mode = TDA10046_TS_SERIAL,
	.xtal_freq = TDA10046_XTAL_16M,
	.if_freq = TDA10046_FREQ_045,
	.agc_config = TDA10046_AGC_TDA827X,
	.gpio_config = TDA10046_GPTRI,
	.request_firmware = NULL,
};

static struct tda1004x_config m920x_tda10046_0b_config = {
	.demod_address = 0x0b,
	.invert = 0,
	.invert_oclk = 0,
	.ts_mode = TDA10046_TS_SERIAL,
	.xtal_freq = TDA10046_XTAL_16M,
	.if_freq = TDA10046_FREQ_045,
	.agc_config = TDA10046_AGC_TDA827X,
	.gpio_config = TDA10046_GPTRI,
	.request_firmware = NULL, /* uses firmware EEPROM */
};

/* tuner configurations */
static struct qt1010_config m920x_qt1010_config = {
	.i2c_address = 0x62
};

/* Callbacks for DVB USB */
static int m920x_mt352_frontend_attach(struct dvb_usb_adapter *adap)
{
	deb("%s\n",__func__);

	if ((adap->fe = dvb_attach(mt352_attach,
				   &m920x_mt352_config,
				   &adap->dev->i2c_adap)) == NULL)
		return -EIO;

	return 0;
}

static int m920x_tda10046_08_frontend_attach(struct dvb_usb_adapter *adap)
{
	deb("%s\n",__func__);

	if ((adap->fe = dvb_attach(tda10046_attach,
				   &m920x_tda10046_08_config,
				   &adap->dev->i2c_adap)) == NULL)
		return -EIO;

	return 0;
}

static int m920x_tda10046_0b_frontend_attach(struct dvb_usb_adapter *adap)
{
	deb("%s\n",__func__);

	if ((adap->fe = dvb_attach(tda10046_attach,
				   &m920x_tda10046_0b_config,
				   &adap->dev->i2c_adap)) == NULL)
		return -EIO;

	return 0;
}

static int m920x_qt1010_tuner_attach(struct dvb_usb_adapter *adap)
{
	deb("%s\n",__func__);

	if (dvb_attach(qt1010_attach, adap->fe, &adap->dev->i2c_adap, &m920x_qt1010_config) == NULL)
		return -ENODEV;

	return 0;
}

static int m920x_tda8275_60_tuner_attach(struct dvb_usb_adapter *adap)
{
	deb("%s\n",__func__);

	if (dvb_attach(tda827x_attach, adap->fe, 0x60, &adap->dev->i2c_adap, NULL) == NULL)
		return -ENODEV;

	return 0;
}

static int m920x_tda8275_61_tuner_attach(struct dvb_usb_adapter *adap)
{
	deb("%s\n",__func__);

	if (dvb_attach(tda827x_attach, adap->fe, 0x61, &adap->dev->i2c_adap, NULL) == NULL)
		return -ENODEV;

	return 0;
}

/* device-specific initialization */
static struct m920x_inits megasky_rc_init [] = {
	{ M9206_RC_INIT2, 0xa8 },
	{ M9206_RC_INIT1, 0x51 },
	{ } /* terminating entry */
};

static struct m920x_inits tvwalkertwin_rc_init [] = {
	{ M9206_RC_INIT2, 0x00 },
	{ M9206_RC_INIT1, 0xef },
	{ 0xff28,         0x00 },
	{ 0xff23,         0x00 },
	{ 0xff21,         0x30 },
	{ } /* terminating entry */
};

/* ir keymaps */
static struct dvb_usb_rc_key megasky_rc_keys [] = {
	{ 0x0, 0x12, KEY_POWER },
	{ 0x0, 0x1e, KEY_CYCLEWINDOWS }, /* min/max */
	{ 0x0, 0x02, KEY_CHANNELUP },
	{ 0x0, 0x05, KEY_CHANNELDOWN },
	{ 0x0, 0x03, KEY_VOLUMEUP },
	{ 0x0, 0x06, KEY_VOLUMEDOWN },
	{ 0x0, 0x04, KEY_MUTE },
	{ 0x0, 0x07, KEY_OK }, /* TS */
	{ 0x0, 0x08, KEY_STOP },
	{ 0x0, 0x09, KEY_MENU }, /* swap */
	{ 0x0, 0x0a, KEY_REWIND },
	{ 0x0, 0x1b, KEY_PAUSE },
	{ 0x0, 0x1f, KEY_FASTFORWARD },
	{ 0x0, 0x0c, KEY_RECORD },
	{ 0x0, 0x0d, KEY_CAMERA }, /* screenshot */
	{ 0x0, 0x0e, KEY_COFFEE }, /* "MTS" */
};

static struct dvb_usb_rc_key tvwalkertwin_rc_keys [] = {
	{ 0x0, 0x01, KEY_ZOOM }, /* Full Screen */
	{ 0x0, 0x02, KEY_CAMERA }, /* snapshot */
	{ 0x0, 0x03, KEY_MUTE },
	{ 0x0, 0x04, KEY_REWIND },
	{ 0x0, 0x05, KEY_PLAYPAUSE }, /* Play/Pause */
	{ 0x0, 0x06, KEY_FASTFORWARD },
	{ 0x0, 0x07, KEY_RECORD },
	{ 0x0, 0x08, KEY_STOP },
	{ 0x0, 0x09, KEY_TIME }, /* Timeshift */
	{ 0x0, 0x0c, KEY_COFFEE }, /* Recall */
	{ 0x0, 0x0e, KEY_CHANNELUP },
	{ 0x0, 0x12, KEY_POWER },
	{ 0x0, 0x15, KEY_MENU }, /* source */
	{ 0x0, 0x18, KEY_CYCLEWINDOWS }, /* TWIN PIP */
	{ 0x0, 0x1a, KEY_CHANNELDOWN },
	{ 0x0, 0x1b, KEY_VOLUMEDOWN },
	{ 0x0, 0x1e, KEY_VOLUMEUP },
};

/* DVB USB Driver stuff */
static struct dvb_usb_device_properties megasky_properties;
static struct dvb_usb_device_properties digivox_mini_ii_properties;
static struct dvb_usb_device_properties tvwalkertwin_properties;
static struct dvb_usb_device_properties dposh_properties;

static int m920x_probe(struct usb_interface *intf,
		       const struct usb_device_id *id)
{
	struct dvb_usb_device *d = NULL;
	int ret;
	struct m920x_inits *rc_init_seq = NULL;
	int bInterfaceNumber = intf->cur_altsetting->desc.bInterfaceNumber;

	deb("Probing for m920x device at interface %d\n", bInterfaceNumber);

	if (bInterfaceNumber == 0) {
		/* Single-tuner device, or first interface on
		 * multi-tuner device
		 */

		ret = dvb_usb_device_init(intf, &megasky_properties,
					  THIS_MODULE, &d, adapter_nr);
		if (ret == 0) {
			rc_init_seq = megasky_rc_init;
			goto found;
		}

		ret = dvb_usb_device_init(intf, &digivox_mini_ii_properties,
					  THIS_MODULE, &d, adapter_nr);
		if (ret == 0) {
			/* No remote control, so no rc_init_seq */
			goto found;
		}

		/* This configures both tuners on the TV Walker Twin */
		ret = dvb_usb_device_init(intf, &tvwalkertwin_properties,
					  THIS_MODULE, &d, adapter_nr);
		if (ret == 0) {
			rc_init_seq = tvwalkertwin_rc_init;
			goto found;
		}

		ret = dvb_usb_device_init(intf, &dposh_properties,
					  THIS_MODULE, &d, adapter_nr);
		if (ret == 0) {
			/* Remote controller not supported yet. */
			goto found;
		}

		return ret;
	} else {
		/* Another interface on a multi-tuner device */

		/* The LifeView TV Walker Twin gets here, but struct
		 * tvwalkertwin_properties already configured both
		 * tuners, so there is nothing for us to do here
		 */
	}

 found:
	if ((ret = m920x_init_ep(intf)) < 0)
		return ret;

	if (d && (ret = m920x_init(d, rc_init_seq)) != 0)
		return ret;

	return ret;
}

static struct usb_device_id m920x_table [] = {
		{ USB_DEVICE(USB_VID_MSI, USB_PID_MSI_MEGASKY580) },
		{ USB_DEVICE(USB_VID_ANUBIS_ELECTRONIC,
			     USB_PID_MSI_DIGI_VOX_MINI_II) },
		{ USB_DEVICE(USB_VID_ANUBIS_ELECTRONIC,
			     USB_PID_LIFEVIEW_TV_WALKER_TWIN_COLD) },
		{ USB_DEVICE(USB_VID_ANUBIS_ELECTRONIC,
			     USB_PID_LIFEVIEW_TV_WALKER_TWIN_WARM) },
		{ USB_DEVICE(USB_VID_DPOSH, USB_PID_DPOSH_M9206_COLD) },
		{ USB_DEVICE(USB_VID_DPOSH, USB_PID_DPOSH_M9206_WARM) },
		{ }		/* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, m920x_table);

static struct dvb_usb_device_properties megasky_properties = {
	.caps = DVB_USB_IS_AN_I2C_ADAPTER,

	.usb_ctrl = DEVICE_SPECIFIC,
	.firmware = "dvb-usb-megasky-02.fw",
	.download_firmware = m920x_firmware_download,

	.rc_interval      = 100,
	.rc_key_map       = megasky_rc_keys,
	.rc_key_map_size  = ARRAY_SIZE(megasky_rc_keys),
	.rc_query         = m920x_rc_query,

	.size_of_priv     = sizeof(struct m920x_state),

	.identify_state   = m920x_identify_state,
	.num_adapters = 1,
	.adapter = {{
		.caps = DVB_USB_ADAP_HAS_PID_FILTER |
			DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF,

		.pid_filter_count = 8,
		.pid_filter       = m920x_pid_filter,
		.pid_filter_ctrl  = m920x_pid_filter_ctrl,

		.frontend_attach  = m920x_mt352_frontend_attach,
		.tuner_attach     = m920x_qt1010_tuner_attach,

		.stream = {
			.type = USB_BULK,
			.count = 8,
			.endpoint = 0x81,
			.u = {
				.bulk = {
					.buffersize = 512,
				}
			}
		},
	}},
	.i2c_algo         = &m920x_i2c_algo,

	.num_device_descs = 1,
	.devices = {
		{   "MSI Mega Sky 580 DVB-T USB2.0",
			{ &m920x_table[0], NULL },
			{ NULL },
		}
	}
};

static struct dvb_usb_device_properties digivox_mini_ii_properties = {
	.caps = DVB_USB_IS_AN_I2C_ADAPTER,

	.usb_ctrl = DEVICE_SPECIFIC,
	.firmware = "dvb-usb-digivox-02.fw",
	.download_firmware = m920x_firmware_download,

	.size_of_priv     = sizeof(struct m920x_state),

	.identify_state   = m920x_identify_state,
	.num_adapters = 1,
	.adapter = {{
		.caps = DVB_USB_ADAP_HAS_PID_FILTER |
			DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF,

		.pid_filter_count = 8,
		.pid_filter       = m920x_pid_filter,
		.pid_filter_ctrl  = m920x_pid_filter_ctrl,

		.frontend_attach  = m920x_tda10046_08_frontend_attach,
		.tuner_attach     = m920x_tda8275_60_tuner_attach,

		.stream = {
			.type = USB_BULK,
			.count = 8,
			.endpoint = 0x81,
			.u = {
				.bulk = {
					.buffersize = 0x4000,
				}
			}
		},
	}},
	.i2c_algo         = &m920x_i2c_algo,

	.num_device_descs = 1,
	.devices = {
		{   "MSI DIGI VOX mini II DVB-T USB2.0",
			{ &m920x_table[1], NULL },
			{ NULL },
		},
	}
};

/* LifeView TV Walker Twin support by Nick Andrew <nick@nick-andrew.net>
 *
 * LifeView TV Walker Twin has 1 x M9206, 2 x TDA10046, 2 x TDA8275A
 * TDA10046 #0 is located at i2c address 0x08
 * TDA10046 #1 is located at i2c address 0x0b
 * TDA8275A #0 is located at i2c address 0x60
 * TDA8275A #1 is located at i2c address 0x61
 */
static struct dvb_usb_device_properties tvwalkertwin_properties = {
	.caps = DVB_USB_IS_AN_I2C_ADAPTER,

	.usb_ctrl = DEVICE_SPECIFIC,
	.firmware = "dvb-usb-tvwalkert.fw",
	.download_firmware = m920x_firmware_download,

	.rc_interval      = 100,
	.rc_key_map       = tvwalkertwin_rc_keys,
	.rc_key_map_size  = ARRAY_SIZE(tvwalkertwin_rc_keys),
	.rc_query         = m920x_rc_query,

	.size_of_priv     = sizeof(struct m920x_state),

	.identify_state   = m920x_identify_state,
	.num_adapters = 2,
	.adapter = {{
		.caps = DVB_USB_ADAP_HAS_PID_FILTER |
			DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF,

		.pid_filter_count = 8,
		.pid_filter       = m920x_pid_filter,
		.pid_filter_ctrl  = m920x_pid_filter_ctrl,

		.frontend_attach  = m920x_tda10046_08_frontend_attach,
		.tuner_attach     = m920x_tda8275_60_tuner_attach,

		.stream = {
			.type = USB_BULK,
			.count = 8,
			.endpoint = 0x81,
			.u = {
				 .bulk = {
					 .buffersize = 512,
				 }
			}
		}},{
		.caps = DVB_USB_ADAP_HAS_PID_FILTER |
			DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF,

		.pid_filter_count = 8,
		.pid_filter       = m920x_pid_filter,
		.pid_filter_ctrl  = m920x_pid_filter_ctrl,

		.frontend_attach  = m920x_tda10046_0b_frontend_attach,
		.tuner_attach     = m920x_tda8275_61_tuner_attach,

		.stream = {
			.type = USB_BULK,
			.count = 8,
			.endpoint = 0x82,
			.u = {
				 .bulk = {
					 .buffersize = 512,
				 }
			}
		},
	}},
	.i2c_algo         = &m920x_i2c_algo,

	.num_device_descs = 1,
	.devices = {
		{   .name = "LifeView TV Walker Twin DVB-T USB2.0",
		    .cold_ids = { &m920x_table[2], NULL },
		    .warm_ids = { &m920x_table[3], NULL },
		},
	}
};

static struct dvb_usb_device_properties dposh_properties = {
	.caps = DVB_USB_IS_AN_I2C_ADAPTER,

	.usb_ctrl = DEVICE_SPECIFIC,
	.firmware = "dvb-usb-dposh-01.fw",
	.download_firmware = m920x_firmware_download,

	.size_of_priv     = sizeof(struct m920x_state),

	.identify_state   = m920x_identify_state,
	.num_adapters = 1,
	.adapter = {{
		/* Hardware pid filters don't work with this device/firmware */

		.frontend_attach  = m920x_mt352_frontend_attach,
		.tuner_attach     = m920x_qt1010_tuner_attach,

		.stream = {
			.type = USB_BULK,
			.count = 8,
			.endpoint = 0x81,
			.u = {
				 .bulk = {
					 .buffersize = 512,
				 }
			}
		},
	}},
	.i2c_algo         = &m920x_i2c_algo,

	.num_device_descs = 1,
	.devices = {
		 {   .name = "Dposh DVB-T USB2.0",
		     .cold_ids = { &m920x_table[4], NULL },
		     .warm_ids = { &m920x_table[5], NULL },
		 },
	 }
};

static struct usb_driver m920x_driver = {
	.name		= "dvb_usb_m920x",
	.probe		= m920x_probe,
	.disconnect	= dvb_usb_device_exit,
	.id_table	= m920x_table,
};

/* module stuff */
static int __init m920x_module_init(void)
{
	int ret;

	if ((ret = usb_register(&m920x_driver))) {
		err("usb_register failed. Error number %d", ret);
		return ret;
	}

	return 0;
}

static void __exit m920x_module_exit(void)
{
	/* deregister this driver from the USB subsystem */
	usb_deregister(&m920x_driver);
}

module_init (m920x_module_init);
module_exit (m920x_module_exit);

MODULE_AUTHOR("Aapo Tahkola <aet@rasterburn.org>");
MODULE_DESCRIPTION("DVB Driver for ULI M920x");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

/*
 * Local variables:
 * c-basic-offset: 8
 */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               R);
	zfTimerCancel(dev, ZM_EVENT_CM_DISCONNECT);

	wd->sta.connectState = ZM_STA_CONN_STATE_NONE;
	wd->sta.connectByReasso = FALSE;
	wd->sta.cmDisallowSsidLength = 0;
	wd->sta.bAutoReconnect = 0;
	wd->sta.InternalScanReq = 0;
	wd->sta.encryMode = ZM_NO_WEP;
	wd->sta.wepStatus = ZM_ENCRYPTION_WEP_DISABLED;
	wd->sta.wpaState = ZM_STA_WPA_STATE_INIT;
	wd->sta.cmMicFailureCount = 0;
	wd->sta.ibssBssIsCreator = 0;
#ifdef ZM_ENABLE_IBSS_WPA2PSK
	wd->sta.ibssWpa2Psk = 0;
#endif
	/* reset connect timeout counter */
	wd->sta.connectTimeoutCount = 0;

	/* reset leap enable variable */
	wd->sta.leapEnabled = 0;

	/* Reset the RIFS Status / RIFS-like frame count / RIFS count */
	if (wd->sta.rifsState == ZM_RIFS_STATE_DETECTED)
		zfHpDisableRifs(dev);
	wd->sta.rifsState = ZM_RIFS_STATE_DETECTING;
	wd->sta.rifsLikeFrameCnt = 0;
	wd->sta.rifsCount = 0;

	wd->sta.osRxFilter = 0;
	wd->sta.bSafeMode = 0;

	/* 	Clear the information for the peer
		stations of IBSS or AP of Station mode
	*/
	zfZeroMemory((u8_t *)wd->sta.oppositeInfo,
			sizeof(struct zsOppositeInfo) * ZM_MAX_OPPOSITE_COUNT);

	zmw_leave_critical_section(dev);

	zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
	zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_EXTERNAL);

	/* Turn off Software WEP/TKIP */
	if (wd->sta.SWEncryptEnable != 0) {
		zm_debug_msg0("Disable software encryption");
		zfStaDisableSWEncryption(dev);
	}

	/* 	Improve WEP/TKIP performace with HT AP,
		detail information please look bug#32495
	*/
	/* zfHpSetTTSIFSTime(dev, 0x8); */

	/* Keep Pseudo mode */
	if (wd->wlanMode != ZM_MODE_PSEUDO)
		wd->wlanMode = ZM_MODE_INFRASTRUCTURE;

	return 0;
}

/* Deauthenticate a STA */
u16_t zfiWlanDeauth(zdev_t *dev, u16_t *macAddr, u16_t reason)
{
	zmw_get_wlan_dev(dev);

	if (wd->wlanMode == ZM_MODE_AP) {
		/* u16_t id; */

		/*
		* we will reset all key in zfHpResetKeyCache() when call
		* zfiWlanDisable(), if we want to reset PairwiseKey for each
		* sta, need to use a nullAddr to let keyindex not match.
		* otherwise hardware will still find PairwiseKey when AP change
		* encryption mode from WPA to WEP
		*/

		/*
		if ((id = zfApFindSta(dev, macAddr)) != 0xffff)
		{
			u32_t key[8];
			u16_t nullAddr[3] = { 0x0, 0x0, 0x0 };

			if (wd->ap.staTable[i].encryMode != ZM_NO_WEP)
			{
				zfHpSetApPairwiseKey(dev, nullAddr,
				ZM_NO_WEP, &key[0], &key[4], i+1);
			}
			//zfHpSetApPairwiseKey(dev, (u16_t *)macAddr,
			//        ZM_NO_WEP, &key[0], &key[4], id+1);
		wd->ap.staTable[id].encryMode = ZM_NO_WEP;
		wd->ap.staTable[id].keyIdx = 0xff;
		}
		*/

		zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH, macAddr,
								reason, 0, 0);
	} else
		zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH,
						wd->sta.bssid, 3, 0, 0);

	/* Issue DEAUTH command to FW */
	return 0;
}


/* XP packet filter feature : */
/* 1=>enable: All multicast address packets, not just the ones */
/* enumerated in the multicast address list. */
/* 0=>disable */
void zfiWlanSetAllMulticast(zdev_t *dev, u32_t setting)
{
	zmw_get_wlan_dev(dev);
	zm_msg1_mm(ZM_LV_0, "sta.bAllMulticast = ", setting);
	wd->sta.bAllMulticast = (u8_t)setting;
}


/* HT configure API */
void zfiWlanSetHTCtrl(zdev_t *dev, u32_t *setting, u32_t forceTxTPC)
{
	zmw_get_wlan_dev(dev);

	wd->preambleType        = (u8_t)setting[0];
	wd->sta.preambleTypeHT  = (u8_t)setting[1];
	wd->sta.htCtrlBandwidth = (u8_t)setting[2];
	wd->sta.htCtrlSTBC      = (u8_t)setting[3];
	wd->sta.htCtrlSG        = (u8_t)setting[4];
	wd->sta.defaultTA       = (u8_t)setting[5];
	wd->enableAggregation   = (u8_t)setting[6];
	wd->enableWDS           = (u8_t)setting[7];

	wd->forceTxTPC          = forceTxTPC;
}

/* FB50 in OS XP, RD private test code */
void zfiWlanQueryHTCtrl(zdev_t *dev, u32_t *setting, u32_t *forceTxTPC)
{
	zmw_get_wlan_dev(dev);

	setting[0] = wd->preambleType;
	setting[1] = wd->sta.preambleTypeHT;
	setting[2] = wd->sta.htCtrlBandwidth;
	setting[3] = wd->sta.htCtrlSTBC;
	setting[4] = wd->sta.htCtrlSG;
	setting[5] = wd->sta.defaultTA;
	setting[6] = wd->enableAggregation;
	setting[7] = wd->enableWDS;

	*forceTxTPC = wd->forceTxTPC;
}

void zfiWlanDbg(zdev_t *dev, u8_t setting)
{
	zmw_get_wlan_dev(dev);

	wd->enableHALDbgInfo = setting;
}

/* FB50 in OS XP, RD private test code */
void zfiWlanSetRxPacketDump(zdev_t *dev, u32_t setting)
{
	zmw_get_wlan_dev(dev);
	if (setting)
		wd->rxPacketDump = 1;   /* enable */
	else
		wd->rxPacketDump = 0;   /* disable */
}


/* FB50 in OS XP, RD private test code */
/* Tally */
void zfiWlanResetTally(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);

	wd->commTally.txUnicastFrm = 0;		/* txUnicastFrames */
	wd->commTally.txMulticastFrm = 0;	/* txMulticastFrames */
	wd->commTally.txUnicastOctets = 0;	/* txUniOctets  byte size */
	wd->commTally.txMulticastOctets = 0;	/* txMultiOctets  byte size */
	wd->commTally.txFrmUpperNDIS = 0;
	wd->commTally.txFrmDrvMgt = 0;
	wd->commTally.RetryFailCnt = 0;
	wd->commTally.Hw_TotalTxFrm = 0;	/* Hardware total Tx Frame */
	wd->commTally.Hw_RetryCnt = 0;		/* txMultipleRetriesFrames */
	wd->commTally.Hw_UnderrunCnt = 0;
	wd->commTally.DriverRxFrmCnt = 0;
	wd->commTally.rxUnicastFrm = 0;		/* rxUnicastFrames */
	wd->commTally.rxMulticastFrm = 0;	/* rxMulticastFrames */
	wd->commTally.NotifyNDISRxFrmCnt = 0;
	wd->commTally.rxUnicastOctets = 0;	/* rxUniOctets  byte size */
	wd->commTally.rxMulticastOctets = 0;	/* rxMultiOctets  byte size */
	wd->commTally.DriverDiscardedFrm = 0;	/* Discard by ValidateFrame */
	wd->commTally.LessThanDataMinLen = 0;
	wd->commTally.GreaterThanMaxLen = 0;
	wd->commTally.DriverDiscardedFrmCauseByMulticastList = 0;
	wd->commTally.DriverDiscardedFrmCauseByFrmCtrl = 0;
	wd->commTally.rxNeedFrgFrm = 0;		/* need more frg frm */
	wd->commTally.DriverRxMgtFrmCnt = 0;
	wd->commTally.rxBroadcastFrm = 0;/* Receive broadcast frame count */
	wd->commTally.rxBroadcastOctets = 0;/*Receive broadcast framebyte size*/
	wd->commTally.Hw_TotalRxFrm = 0;
	wd->commTally.Hw_CRC16Cnt = 0;		/* rxPLCPCRCErrCnt */
	wd->commTally.Hw_CRC32Cnt = 0;		/* rxCRC32ErrCnt */
	wd->commTally.Hw_DecrypErr_UNI = 0;
	wd->commTally.Hw_DecrypErr_Mul = 0;
	wd->commTally.Hw_RxFIFOOverrun = 0;
	wd->commTally.Hw_RxTimeOut = 0;
	wd->commTally.LossAP = 0;

	wd->commTally.Tx_MPDU = 0;
	wd->commTally.BA_Fail = 0;
	wd->commTally.Hw_Tx_AMPDU = 0;
	wd->commTally.Hw_Tx_MPDU = 0;

	wd->commTally.txQosDropCount[0] = 0;
	wd->commTally.txQosDropCount[1] = 0;
	wd->commTally.txQosDropCount[2] = 0;
	wd->commTally.txQosDropCount[3] = 0;
	wd->commTally.txQosDropCount[4] = 0;

	wd->commTally.Hw_RxMPDU = 0;
	wd->commTally.Hw_RxDropMPDU = 0;
	wd->commTally.Hw_RxDelMPDU = 0;

	wd->commTally.Hw_RxPhyMiscError = 0;
	wd->commTally.Hw_RxPhyXRError = 0;
	wd->commTally.Hw_RxPhyOFDMError = 0;
	wd->commTally.Hw_RxPhyCCKError = 0;
	wd->commTally.Hw_RxPhyHTError = 0;
	wd->commTally.Hw_RxPhyTotalCount = 0;

#if (defined(GCCK) && defined(OFDM))
	wd->commTally.rx11bDataFrame = 0;
	wd->commTally.rxOFDMDataFrame = 0;
#endif

	zmw_leave_critical_section(dev);
}

/* FB50 in OS XP, RD private test code */
void zfiWlanQueryTally(zdev_t *dev, struct zsCommTally *tally)
{
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);
	zfMemoryCopy((u8_t *)tally, (u8_t *)&wd->commTally,
						sizeof(struct zsCommTally));
	zmw_leave_critical_section(dev);
}

void zfiWlanQueryTrafTally(zdev_t *dev, struct zsTrafTally *tally)
{
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);
	zfMemoryCopy((u8_t *)tally, (u8_t *)&wd->trafTally,
						sizeof(struct zsTrafTally));
	zmw_leave_critical_section(dev);
}

void zfiWlanQueryMonHalRxInfo(zdev_t *dev, struct zsMonHalRxInfo *monHalRxInfo)
{
	zfHpQueryMonHalRxInfo(dev, (u8_t *)monHalRxInfo);
}

/* parse the modeMDKEnable to DrvCore */
void zfiDKEnable(zdev_t *dev, u32_t enable)
{
	zmw_get_wlan_dev(dev);

	wd->modeMDKEnable = enable;
	zm_debug_msg1("modeMDKEnable = ", wd->modeMDKEnable);
}

/* airoPeek */
u32_t zfiWlanQueryPacketTypePromiscuous(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	return wd->swSniffer;
}

/* airoPeek */
void zfiWlanSetPacketTypePromiscuous(zdev_t *dev, u32_t setValue)
{
	zmw_get_wlan_dev(dev);

	wd->swSniffer = setValue;
	zm_msg1_mm(ZM_LV_0, "wd->swSniffer ", wd->swSniffer);
	if (setValue) {
		/* write register for sniffer mode */
		zfHpSetSnifferMode(dev, 1);
		zm_msg0_mm(ZM_LV_1, "enalbe sniffer mode");
	} else {
		zfHpSetSnifferMode(dev, 0);
		zm_msg0_mm(ZM_LV_0, "disalbe sniffer mode");
	}
}

void zfiWlanSetXLinkMode(zdev_t *dev, u32_t setValue)
{
	zmw_get_wlan_dev(dev);

	wd->XLinkMode = setValue;
	if (setValue) {
		/* write register for sniffer mode */
		zfHpSetSnifferMode(dev, 1);
	} else
		zfHpSetSnifferMode(dev, 0);
}

extern void zfStaChannelManagement(zdev_t *dev, u8_t scan);

void zfiSetChannelManagement(zdev_t *dev, u32_t setting)
{
	zmw_get_wlan_dev(dev);

	switch (setting) {
	case 1:
		wd->sta.EnableHT = 1;
		wd->BandWidth40 = 1;
		wd->ExtOffset   = 1;
		break;
	case 3:
		wd->sta.EnableHT = 1;
		wd->BandWidth40 = 1;
		wd->ExtOffset   = 3;
		break;
	case 0:
		wd->sta.EnableHT = 1;
		wd->BandWidth40 = 0;
		wd->ExtOffset   = 0;
		break;
	default:
		wd->BandWidth40 = 0;
		wd->ExtOffset   = 0;
		break;
	}

	zfCoreSetFrequencyEx(dev, wd->frequency, wd->BandWidth40,
							wd->ExtOffset, NULL);
}

void zfiSetRifs(zdev_t *dev, u16_t setting)
{
	zmw_get_wlan_dev(dev);

	wd->sta.ie.HtInfo.ChannelInfo |= ExtHtCap_RIFSMode;
	wd->sta.EnableHT = 1;

	switch (setting) {
	case 0:
		wd->sta.HT2040 = 0;
		/* zfHpSetRifs(dev, 1, 0,
		*  (wd->sta.currentFrequency < 3000)? 1:0);
		*/
		break;
	case 1:
		wd->sta.HT2040 = 1;
		/* zfHpSetRifs(dev, 1, 1,
		*  (wd->sta.currentFrequency < 3000)? 1:0);
		*/
		break;
	default:
		wd->sta.HT2040 = 0;
		/* zfHpSetRifs(dev, 1, 0,
		*  (wd->sta.currentFrequency < 3000)? 1:0);
		*/
		break;
	}
}

void zfiCheckRifs(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	if (wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_RIFSMode)
		;
		/* zfHpSetRifs(dev, wd->sta.EnableHT, wd->sta.HT2040,
		*  (wd->sta.currentFrequency < 3000)? 1:0);
		*/
}

void zfiSetReorder(zdev_t *dev, u16_t value)
{
	zmw_get_wlan_dev(dev);

	wd->reorder = value;
}

void zfiSetSeqDebug(zdev_t *dev, u16_t value)
{
	zmw_get_wlan_dev(dev);

	wd->seq_debug = value;
}
