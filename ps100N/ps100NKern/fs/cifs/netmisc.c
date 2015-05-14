buf, sizeof(buf), "ieee1394:ven%08Xmo%08Xsp%08Xver%08X",
			ud->vendor_id,
			ud->model_id,
			ud->specifier_id,
			ud->version);
	PUT_ENVP("MODALIAS=%s", buf);

#undef PUT_ENVP

	return 0;
}

#else

static int nodemgr_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return -ENODEV;
}

#endif /* CONFIG_HOTPLUG */


int __hpsb_register_protocol(struct hpsb_protocol_driver *drv,
			     struct module *owner)
{
	int error;

	drv->driver.bus = &ieee1394_bus_type;
	drv->driver.owner = owner;
	drv->driver.name = drv->name;

	/* This will cause a probe for devices */
	error = driver_register(&drv->driver);
	if (!error)
		nodemgr_create_drv_files(drv);
	return error;
}

void hpsb_unregister_protocol(struct hpsb_protocol_driver *driver)
{
	nodemgr_remove_drv_files(driver);
	/* This will subsequently disconnect all devices that our driver
	 * is attached to. */
	driver_unregister(&driver->driver);
}


/*
 * This function updates nodes that were present on the bus before the
 * reset and still are after the reset.  The nodeid and the config rom
 * may have changed, and the drivers managing this device must be
 * informed that this device just went through a bus reset, to allow
 * the to take whatever actions required.
 */
static void nodemgr_update_node(struct node_entry *ne, struct csr1212_csr *csr,
				nodeid_t nodeid, unsigned int generation)
{
	if (ne->nodeid != nodeid) {
		HPSB_DEBUG("Node changed: " NODE_BUS_FMT " -> " NODE_BUS_FMT,
			   NODE_BUS_ARGS(ne->host, ne->nodeid),
			   NODE_BUS_ARGS(ne->host, nodeid));
		ne->nodeid = nodeid;
	}

	if (ne->busopt.generation != ((be32_to_cpu(csr->bus_info_data[2]) >> 4) & 0xf)) {
		kfree(ne->csr->private);
		csr1212_destroy_csr(ne->csr);
		ne->csr = csr;

		/* If the node's configrom generation has changed, we
		 * unregister all the unit directories. */
		nodemgr_remove_uds(ne);

		nodemgr_update_bus_options(ne);

		/* Mark the node as new, so it gets re-probed */
		ne->needs_probe = true;
	} else {
		/* old cache is valid, so update its generation */
		struct nodemgr_csr_info *ci = ne->csr->private;
		ci->generation = generation;
		/* free the partially filled now unneeded new cache */
		kfree(csr->private);
		csr1212_destroy_csr(csr);
	}

	/* Finally, mark the node current */
	smp_wmb();
	ne->generation = generation;

	if (ne->in_limbo) {
		device_remove_file(&ne->device, &dev_attr_ne_in_limbo);
		ne->in_limbo = false;

		HPSB_DEBUG("Node reactivated: "
			   "ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
			   NODE_BUS_ARGS(ne->host, ne->nodeid),
			   (unsigned long long)ne->guid);
	}
}

static void nodemgr_node_scan_one(struct hpsb_host *host,
				  nodeid_t nodeid, int generation)
{
	struct node_entry *ne;
	octlet_t guid;
	struct csr1212_csr *csr;
	struct nodemgr_csr_info *ci;
	u8 *speed;

	ci = kmalloc(sizeof(*ci), GFP_KERNEL);
	kmemcheck_annotate_bitfield(ci, flags);
	if (!ci)
		return;

	ci->host = host;
	ci->nodeid = nodeid;
	ci->generation = generation;

	/* Prepare for speed probe which occurs when reading the ROM */
	speed = &(host->speed[NODEID_TO_NODE(nodeid)]);
	if (*speed > host->csr.lnk_spd)
		*speed = host->csr.lnk_spd;
	ci->speed_unverified = *speed > IEEE1394_SPEED_100;

	/* We need to detect when the ConfigROM's generation has changed,
	 * so we only update the node's info when it needs to be.  */

	csr = csr1212_create_csr(&nodemgr_csr_ops, 5 * sizeof(quadlet_t), ci);
	if (!csr || csr1212_parse_csr(csr) != CSR1212_SUCCESS) {
		HPSB_ERR("Error parsing configrom for node " NODE_BUS_FMT,
			 NODE_BUS_ARGS(host, nodeid));
		if (csr)
			csr1212_destroy_csr(csr);
		kfree(ci);
		return;
	}

	if (csr->bus_info_data[1] != IEEE1394_BUSID_MAGIC) {
		/* This isn't a 1394 device, but we let it slide. There
		 * was a report of a device with broken firmware which
		 * reported '2394' instead of '1394', which is obviously a
		 * mistake. One would hope that a non-1394 device never
		 * gets connected to Firewire bus. If someone does, we
		 * shouldn't be held responsible, so we'll allow it with a
		 * warning.  */
		HPSB_WARN("Node " NODE_BUS_FMT " has invalid busID magic [0x%08x]",
			  NODE_BUS_ARGS(host, nodeid), csr->bus_info_data[1]);
	}

	guid = ((u64)be32_to_cpu(csr->bus_info_data[3]) << 32) | be32_to_cpu(csr->bus_info_data[4]);
	ne = find_entry_by_guid(guid);

	if (ne && ne->host != host && ne->in_limbo) {
		/* Must have moved this device from one host to another */
		nodemgr_remove_ne(ne);
		ne = NULL;
	}

	if (!ne)
		nodemgr_create_node(guid, csr, host, nodeid, generation);
	else
		nodemgr_update_node(ne, csr, nodeid, generation);
}


static void nodemgr_node_scan(struct hpsb_host *host, int generation)
{
	int count;
	struct selfid *sid = (struct selfid *)host->topology_map;
	nodeid_t nodeid = LOCAL_BUS;

	/* Scan each node on the bus */
	for (count = host->selfid_count; count; count--, sid++) {
		if (sid->extended)
			continue;

		if (!sid->link_active) {
			nodeid++;
			continue;
		}
		nodemgr_node_scan_one(host, nodeid++, generation);
	}
}

static void nodemgr_pause_ne(struct node_entry *ne)
{
	HPSB_DEBUG("Node paused: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		   NODE_BUS_ARGS(ne->host, ne->nodeid),
		   (unsigned long long)ne->guid);

	ne->in_limbo = true;
	WARN_ON(device_create_file(&ne->device, &dev_attr_ne_in_limbo));
}

static int update_pdrv(struct device *dev, void *data)
{
	struct unit_directory *ud;
	struct device_driver *drv;
	struct hpsb_protocol_driver *pdrv;
	struct node_entry *ne = data;
	int error;

	ud = container_of(dev, struct unit_directory, unit_dev);
	if (ud->ne == ne) {
		drv = get_driver(ud->device.driver);
		if (drv) {
			error = 0;
			pdrv = container_of(drv, struct hpsb_protocol_driver,
					    driver);
			if (pdrv->update) {
				down(&ud->device.sem);
				error = pdrv->update(ud);
				up(&ud->device.sem);
			}
			if (error)
				device_release_driver(&ud->device);
			put_driver(drv);
		}
	}

	return 0;
}

static void nodemgr_update_pdrv(struct node_entry *ne)
{
	class_for_each_device(&nodemgr_ud_class, NULL, ne, update_pdrv);
}

/* Write the BROADCAST_CHANNEL as per IEEE1394a 8.3.2.3.11 and 8.4.2.3.  This
 * seems like an optional service but in the end it is practically mandatory
 * as a consequence of these clauses.
 *
 * Note that we cannot do a broadcast write to all nodes at once because some
 * pre-1394a devices would hang. */
static void nodemgr_irm_write_bc(struct node_entry *ne, int generation)
{
	const u64 bc_addr = (CSR_REGISTER_BASE | CSR_BROADCAST_CHANNEL);
	quadlet_t bc_remote, bc_local;
	int error;

	if (!ne->host->is_irm || ne->generation != generation ||
	    ne->nodeid == ne->host->node_id)
		return;

	bc_local = cpu_to_be32(ne->host->csr.broadcast_channel);

	/* Check if the register is implemented and 1394a compliant. */
	error = hpsb_read(ne->host, ne->nodeid, generation, bc_addr, &bc_remote,
			  sizeof(bc_remote));
	if (!error && bc_remote & cpu_to_be32(0x80000000) &&
	    bc_remote != bc_local)
		hpsb_node_write(ne, bc_addr, &bc_local, sizeof(bc_local));
}


static void nodemgr_probe_ne(struct hpsb_host *host, struct node_entry *ne,
			     int generation)
{
	struct device *dev;

	if (ne->host != host || ne->in_limbo)
		return;

	dev = get_device(&ne->device);
	if (!dev)
		return;

	nodemgr_irm_write_bc(ne, generation);

	/* If "needs_probe", then this is either a new or changed node we
	 * rescan totally. If the generation matches for an existing node
	 * (one that existed prior to the bus reset) we send update calls
	 * down to the drivers. Otherwise, this is a dead node and we
	 * suspend it. */
	if (ne->needs_probe)
		nodemgr_process_root_directory(ne);
	else if (ne->generation == generation)
		nodemgr_update_pdrv(ne);
	else
		nodemgr_pause_ne(ne);

	put_device(dev);
}

struct node_probe_parameter {
	struct hpsb_host *host;
	int generation;
	bool probe_now;
};

static int node_probe(struct device *dev, void *data)
{
	struct node_probe_parameter *p = data;
	struct node_entry *ne;

	if (p->generation != get_hpsb_generation(p->host))
		return -EAGAIN;

	ne = container_of(dev, struct node_entry, node_dev);
	if (ne->needs_probe == p->probe_now)
		nodemgr_probe_ne(p->host, ne, p->generation);
	return 0;
}

static int nodemgr_node_probe(struct hpsb_host *host, int generation)
{
	struct node_probe_parameter p;

	p.host = host;
	p.generation = generation;
	/*
	 * Do some processing of the nodes we've probed. This pulls them
	 * into the sysfs layer if needed, and can result in processing of
	 * unit-directories, or just updating the node and it's
	 * unit-directories.
	 *
	 * Run updates before probes. Usually, updates are time-critical
	 * while probes are time-consuming.
	 *
	 * Meanwhile, another bus reset may have happened. In this case we
	 * skip everything here and let the next bus scan handle it.
	 * Otherwise we may prematurely remove nodes which are still there.
	 */
	p.probe_now = false;
	if (class_for_each_device(&nodemgr_ne_class, NULL, &p, node_probe) != 0)
		return 0;

	p.probe_now = true;
	if (class_for_each_device(&nodemgr_ne_class, NULL, &p, node_probe) != 0)
		return 0;
	/*
	 * Now let's tell the bus to rescan our devices. This may seem
	 * like overhead, but the driver-model core will only scan a
	 * device for a driver when either the device is added, or when a
	 * new driver is added. A bus reset is a good reason to rescan
	 * devices that were there before.  For example, an sbp2 device
	 * may become available for login, if the host that held it was
	 * just removed.
	 */
	if (bus_rescan_devices(&ieee1394_bus_type) != 0)
		HPSB_DEBUG("bus_rescan_devices had an error");

	return 1;
}

static int remove_nodes_in_limbo(struct device *dev, void *data)
{
	struct node_entry *ne;

	if (dev->bus != &ieee1394_bus_type)
		return 0;

	ne = container_of(dev, struct node_entry, device);
	if (ne->in_limbo)
		nodemgr_remove_ne(ne);

	return 0;
}

static void nodemgr_remove_nodes_in_limbo(struct hpsb_host *host)
{
	device_for_each_child(&host->device, NULL, remove_nodes_in_limbo);
}

static int nodemgr_send_resume_packet(struct hpsb_host *host)
{
	struct hpsb_packet *packet;
	int error = -ENOMEM;

	packet = hpsb_make_phypacket(host,
			EXTPHYPACKET_TYPE_RESUME |
			NODEID_TO_NODE(host->node_id) << PHYPACKET_PORT_SHIFT);
	if (packet) {
		packet->no_waiter = 1;
		packet->generation = get_hpsb_generation(host);
		error = hpsb_send_packet(packet);
	}
	if (error)
		HPSB_WARN("fw-host%d: Failed to broadcast resume packet",
			  host->id);
	return error;
}

/* Perform a few high-level IRM responsibilities. */
static int nodemgr_do_irm_duties(struct hpsb_host *host, int cycles)
{
	quadlet_t bc;

	/* if irm_id == -1 then there is no IRM on this bus */
	if (!host->is_irm || host->irm_id == (nodeid_t)-1)
		return 1;

	/* We are a 1394a-2000 compliant IRM. Set the validity bit. */
	host->csr.broadcast_channel |= 0x40000000;

	/* If there is no bus manager then we should set the root node's
	 * force_root bit to promote bus stability per the 1394
	 * spec. (8.4.2.6) */
	if (host->busmgr_id == 0xffff && host->node_count > 1)
	{
		u16 root_node = host->node_count - 1;

		/* get cycle master capability flag from root node */
		if (host->is_cycmst ||
		    (!hpsb_read(host, LOCAL_BUS | root_node, get_hpsb_generation(host),
				(CSR_REGISTER_BASE + CSR_CONFIG_ROM + 2 * sizeof(quadlet_t)),
				&bc, sizeof(quadlet_t)) &&
		     be32_to_cpu(bc) & 1 << CSR_CMC_SHIFT))
			hpsb_send_phy_config(host, root_node, -1);
		else {
			HPSB_DEBUG("The root node is not cycle master capable; "
				   "selecting a new root node and resetting...");

			if (cycles >= 5) {
				/* Oh screw it! Just leave the bus as it is */
				HPSB_DEBUG("Stopping reset loop for IRM sanity");
				return 1;
			}

			hpsb_send_phy_config(host, NODEID_TO_NODE(host->node_id), -1);
			hpsb_reset_bus(host, LONG_RESET_FORCE_ROOT);

			return 0;
		}
	}

	/* Some devices suspend their ports while being connected to an inactive
	 * host adapter, i.e. if connected before the low-level driver is
	 * loaded.  They become visible either when physically unplugged and
	 * replugged, or when receiving a resume packet.  Send one once. */
	if (!host->resume_packet_sent && !nodemgr_send_resume_packet(host))
		host->resume_packet_sent = 1;

	return 1;
}

/* We need to ensure that if we are not the IRM, that the IRM node is capable of
 * everything we can do, otherwise issue a bus reset and try to become the IRM
 * ourselves. */
static int nodemgr_check_irm_capability(struct hpsb_host *host, int cycles)
{
	quadlet_t bc;
	int status;

	if (hpsb_disable_irm || host->is_irm)
		return 1;

	status = hpsb_read(host, LOCAL_BUS | (host->irm_id),
			   get_hpsb_generation(host),
			   (CSR_REGISTER_BASE | CSR_BROADCAST_CHANNEL),
			   &bc, sizeof(quadlet_t));

	if (status < 0 || !(be32_to_cpu(bc) & 0x80000000)) {
		/* The current irm node does not have a valid BROADCAST_CHANNEL
		 * register and we do, so reset the bus with force_root set */
		HPSB_DEBUG("Current remote IRM is not 1394a-2000 compliant, resetting...");

		if (cycles >= 5) {
			/* Oh screw it! Just leave the bus as it is */
			HPSB_DEBUG("Stopping reset loop for IRM sanity");
			return 1;
		}

		hpsb_send_phy_config(host, NODEID_TO_NODE(host->node_id), -1);
		hpsb_reset_bus(host, LONG_RESET_FORCE_ROOT);

		return 0;
	}

	return 1;
}

static int nodemgr_host_thread(void *data)
{
	struct hpsb_host *host = data;
	unsigned int g, generation = 0;
	int i, reset_cycles = 0;

	set_freezable();
	/* Setup our device-model entries */
	nodemgr_create_host_dev_files(host);

	for (;;) {
		/* Sleep until next bus reset */
		set_current_state(TASK_INTERRUPTIBLE);
		if (get_hpsb_generation(host) == generation &&
		    !kthread_should_stop())
			schedule();
		__set_current_state(TASK_RUNNING);

		/* Thread may have been woken up to freeze or to exit */
		if (try_to_freeze())
			continue;
		if (kthread_should_stop())
			goto exit;

		/* Pause for 1/4 second in 1/16 second intervals,
		 * to make sure things settle down. */
		g = get_hpsb_generation(host);
		for (i = 0; i < 4 ; i++) {
			msleep_interruptible(63);
			try_to_freeze();
			if (kthread_should_stop())
				goto exit;

			/* Now get the generation in which the node ID's we collect
			 * are valid.  During the bus scan we will use this generation
			 * for the read transactions, so that if another reset occurs
			 * during the scan the transactions will fail instead of
			 * returning bogus data. */
			generation = get_hpsb_generation(host);

			/* If we get a reset before we are done waiting, then
			 * start the waiting over again */
			if (generation != g)
				g = generation, i = 0;
		}

		if (!nodemgr_check_irm_capability(host, reset_cycles) ||
		    !nodemgr_do_irm_duties(host, reset_cycles)) {
			reset_cycles++;
			continue;
		}
		reset_cycles = 0;

		/* Scan our nodes to get the bus options and create node
		 * entries. This does not do the sysfs stuff, since that
		 * would trigger uevents and such, which is a bad idea at
		 * this point. */
		nodemgr_node_scan(host, generation);

		/* This actually does the full probe, with sysfs
		 * registration. */
		if (!nodemgr_node_probe(host, generation))
			continue;

		/* Update some of our sysfs symlinks */
		nodemgr_update_host_dev_links(host);

		/* Sleep 3 seconds */
		for (i = 3000/200; i; i--) {
			msleep_interruptible(200);
			try_to_freeze();
			if (kthread_should_stop())
				goto exit;

			if (generation != get_hpsb_generation(host))
				break;
		}
		/* Remove nodes which are gone, unless a bus reset happened */
		if (!i)
			nodemgr_remove_nodes_in_limbo(host);
	}
exit:
	HPSB_VERBOSE("NodeMgr: Exiting thread");
	return 0;
}

struct per_host_parameter {
	void *data;
	int (*cb)(struct hpsb_host *, void *);
};

static int per_host(struct device *dev, void *data)
{
	struct hpsb_host *host;
	struct per_host_parameter *p = data;

	host = container_of(dev, struct hpsb_host, host_dev);
	return p->cb(host, p->data);
}

/**
 * nodemgr_for_each_host - call a function for each IEEE 1394 host
 * @data: an address to supply to the callback
 * @cb: function to call for each host
 *
 * Iterate the hosts, calling a given function with supplied data for each host.
 * If the callback fails on a host, i.e. if it returns a non-zero value, the
 * iteration is stopped.
 *
 * Return value: 0 on success, non-zero on failure (same as returned by last run
 * of the callback).
 */
int nodemgr_for_each_host(void *data, int (*cb)(struct hpsb_host *, void *))
{
	struct per_host_parameter p;

	p.cb = cb;
	p.data = data;
	return class_for_each_device(&hpsb_host_class, NULL, &p, per_host);
}

/* The following two convenience functions use a struct node_entry
 * for addressing a node on the bus.  They are intended for use by any
 * process context, not just the nodemgr thread, so we need to be a
 * little careful when reading out the node ID and generation.  The
 * thing that can go wrong is that we get the node ID, then a bus
 * reset occurs, and then we read the generation.  The node ID is
 * possibly invalid, but the generation is current, and we end up
 * sending a packet to a the wrong node.
 *
 * The solution is to make sure we read the generation first, so that
 * if a reset occurs in the process, we end up with a stale generation
 * and the transactions will fail instead of silently using wrong node
 * ID's.
 */

/**
 * hpsb_node_fill_packet - fill some destination information into a packet
 * @ne: destination node
 * @packet: packet to fill in
 *
 * This will fill in the given, pre-initialised hpsb_packet with the current
 * information from the node entry (host, node ID, bus generation number).
 */
void hpsb_node_fill_packet(struct node_entry *ne, struct hpsb_packet *packet)
{
	packet->host = ne->host;
	packet->generation = ne->generation;
	smp_rmb();
	packet->node_id = ne->nodeid;
}

int hpsb_node_write(struct node_entry *ne, u64 addr,
		    quadlet_t *buffer, size_t length)
{
	unsigned int generation = ne->generation;

	smp_rmb();
	return hpsb_write(ne->host, ne->nodeid, generation,
			  addr, buffer, length);
}

static void nodemgr_add_host(struct hpsb_host *host)
{
	struct host_info *hi;

	hi = hpsb_create_hostinfo(&nodemgr_highlevel, host, sizeof(*hi));
	if (!hi) {
		HPSB_ERR("NodeMgr: out of memory in add host");
		return;
	}
	hi->host = host;
	hi->thread = kthread_run(nodemgr_host_thread, host, "knodemgrd_%d",
				 host->id);
	if (IS_ERR(hi->thread)) {
		HPSB_ERR("NodeMgr: cannot start thread for host %d", host->id);
		hpsb_destroy_hostinfo(&nodemgr_highlevel, host);
	}
}

static void nodemgr_host_reset(struct hpsb_host *host)
{
	struct host_info *hi = hpsb_get_hostinfo(&nodemgr_highlevel, host);

	if (hi) {
		HPSB_VERBOSE("NodeMgr: Processing reset for host %d", host->id);
		wake_up_process(hi->thread);
	}
}

static void nodemgr_remove_host(struct hpsb_host *host)
{
	struct host_info *hi = hpsb_get_hostinfo(&nodemgr_highlevel, host);

	if (hi) {
		kthread_stop(hi->thread);
		nodemgr_remove_host_dev(&host->device);
	}
}

static struct hpsb_highlevel nodemgr_highlevel = {
	.name =		"Node manager",
	.add_host =	nodemgr_add_host,
	.host_reset =	nodemgr_host_reset,
	.remove_host =	nodemgr_remove_host,
};

int init_ieee1394_nodemgr(void)
{
	int error;

	error = class_register(&nodemgr_ne_class);
	if (error)
		goto fail_ne;
	error = class_register(&nodemgr_ud_class);
	if (error)
		goto fail_ud;
	error = driver_register(&nodemgr_mid_layer_driver);
	if (error)
		goto fail_ml;
	/* This driver is not used if nodemgr is off (disable_nodemgr=1). */
	nodemgr_dev_template_host.driver = &nodemgr_mid_layer_driver;

	hpsb_register_highlevel(&nodemgr_highlevel);
	return 0;

fail_ml:
	class_unregister(&nodemgr_ud_class);
fail_ud:
	class_unregister(&nodemgr_ne_class);
fail_ne:
	return error;
}

void cleanup_ieee1394_nodemgr(void)
{
	hpsb_unregister_highlevel(&nodemgr_highlevel);
	driver_unregister(&nodemgr_mid_layer_driver);
	class_unregister(&nodemgr_ud_class);
	class_unregister(&nodemgr_ne_class);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /******************************************************************************
**  Device driver for the PCI-SCSI NCR538XX controller family.
**
**  Copyright (C) 1994  Wolfgang Stanglmeier
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**-----------------------------------------------------------------------------
**
**  This driver has been ported to Linux from the FreeBSD NCR53C8XX driver
**  and is currently maintained by
**
**          Gerard Roudier              <groudier@free.fr>
**
**  Being given that this driver originates from the FreeBSD version, and
**  in order to keep synergy on both, any suggested enhancements and corrections
**  received on Linux are automatically a potential candidate for the FreeBSD 
**  version.
**
**  The original driver has been written for 386bsd and FreeBSD by
**          Wolfgang Stanglmeier        <wolf@cologne.de>
**          Stefan Esser                <se@mi.Uni-Koeln.de>
**
**  And has been ported to NetBSD by
**          Charles M. Hannum           <mycroft@gnu.ai.mit.edu>
**
**-----------------------------------------------------------------------------
**
**                     Brief history
**
**  December 10 1995 by Gerard Roudier:
**     Initial port to Linux.
**
**  June 23 1996 by Gerard Roudier:
**     Support for 64 bits architectures (Alpha).
**
**  November 30 1996 by Gerard Roudier:
**     Support for Fast-20 scsi.
**     Support for large DMA fifo and 128 dwords bursting.
**
**  February 27 1997 by Gerard Roudier:
**     Support for Fast-40 scsi.
**     Support for on-Board RAM.
**
**  May 3 1997 by Gerard Roudier:
**     Full support for scsi scripts instructions pre-fetching.
**
**  May 19 1997 by Richard Waltham <dormouse@farsrobt.demon.co.uk>:
**     Support for NvRAM detection and reading.
**
**  August 18 1997 by Cort <cort@cs.nmt.edu>:
**     Support for Power/PC (Big Endian).
**
**  June 20 1998 by Gerard Roudier
**     Support for up to 64 tags per lun.
**     O(1) everywhere (C and SCRIPTS) for normal cases.
**     Low PCI traffic for command handling when on-chip RAM is present.
**     Aggressive SCSI SCRIPTS optimizations.
**
**  2005 by Matthew Wilcox and James Bottomley
**     PCI-ectomy.  This driver now supports only the 720 chip (see the
**     NCR_Q720 and zalon drivers for the bus probe logic).
**
*******************************************************************************
*/

/*
**	Supported SCSI-II features:
**	    Synchronous negotiation
**	    Wide negotiation        (depends on the NCR Chip)
**	    Enable disconnection
**	    Tagged command queuing
**	    Parity checking
**	    Etc...
**
**	Supported NCR/SYMBIOS chips:
**		53C720		(Wide,   Fast SCSI-2, intfly problems)
*/

/* Name and version of the driver */
#define SCSI_NCR_DRIVER_NAME	"ncr53c8xx-3.4.3g"

#define SCSI_NCR_DEBUG_FLAGS	(0)

#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/spinlock.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/types.h>

#include <asm/dma.h>
#include <asm/io.h>
#include <asm/system.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_dbg.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_spi.h>

#include "ncr53c8xx.h"

#define NAME53C8XX		"ncr53c8xx"

/*==========================================================
**
**	Debugging tags
**
**==========================================================
*/

#define DEBUG_ALLOC    (0x0001)
#define DEBUG_PHASE    (0x0002)
#define DEBUG_QUEUE    (0x0008)
#define DEBUG_RESULT   (0x0010)
#define DEBUG_POINTER  (0x0020)
#define DEBUG_SCRIPT   (0x0040)
#define DEBUG_TINY     (0x0080)
#define DEBUG_TIMING   (0x0100)
#define DEBUG_NEGO     (0x0200)
#define DEBUG_TAGS     (0x0400)
#define DEBUG_SCATTER  (0x0800)
#define DEBUG_IC        (0x1000)

/*
**    Enable/Disable debug messages.
**    Can be changed at runtime too.
*/

#ifdef SCSI_NCR_DEBUG_INFO_SUPPORT
static int ncr_debug = SCSI_NCR_DEBUG_FLAGS;
	#define DEBUG_FLAGS ncr_debug
#else
	#define DEBUG_FLAGS	SCSI_NCR_DEBUG_FLAGS
#endif

static inline struct list_head *ncr_list_pop(struct list_head *head)
{
	if (!list_empty(head)) {
		struct list_head *elem = head->next;

		list_del(elem);
		return elem;
	}

	return NULL;
}

/*==========================================================
**
**	Simple power of two buddy-like allocator.
**
**	This simple code is not intended to be fast, but to 
**	provide power of 2 aligned memory allocations.
**	Since the SCRIPTS processor only supplies 8 bit 
**	arithmetic, this allocator allows simple and fast 
**	address calculations  from the SCRIPTS code.
**	In addition, cache line alignment is guaranteed for 
**	power of 2 cache line size.
**	Enhanced in linux-2.3.44 to provide a memory pool 
**	per pcidev to support dynamic dma mapping. (I would 
**	have preferred a real bus abstraction, btw).
**
**==========================================================
*/

#define MEMO_SHIFT	4	/* 16 bytes minimum memory chunk */
#if PAGE_SIZE >= 8192
#define MEMO_PAGE_ORDER	0	/* 1 PAGE  maximum */
#else
#define MEMO_PAGE_ORDER	1	/* 2 PAGES maximum */
#endif
#define MEMO_FREE_UNUSED	/* Free unused pages immediately */
#define MEMO_WARN	1
#define MEMO_GFP_FLAGS	GFP_ATOMIC
#define MEMO_CLUSTER_SHIFT	(PAGE_SHIFT+MEMO_PAGE_ORDER)
#define MEMO_CLUSTER_SIZE	(1UL << MEMO_CLUSTER_SHIFT)
#define MEMO_CLUSTER_MASK	(MEMO_CLUSTER_SIZE-1)

typedef u_long m_addr_t;	/* Enough bits to bit-hack addresses */
typedef struct device *m_bush_t;	/* Something that addresses DMAable */

typedef struct m_link {		/* Link between free memory chunks */
	struct m_link *next;
} m_link_s;

typedef struct m_vtob {		/* Virtual to Bus address translation */
	struct m_vtob *next;
	m_addr_t vaddr;
	m_addr_t baddr;
} m_vtob_s;
#define VTOB_HASH_SHIFT		5
#define VTOB_HASH_SIZE		(1UL << VTOB_HASH_SHIFT)
#define VTOB_HASH_MASK		(VTOB_HASH_SIZE-1)
#define VTOB_HASH_CODE(m)	\
	((((m_addr_t) (m)) >> MEMO_CLUSTER_SHIFT) & VTOB_HASH_MASK)

typedef struct m_pool {		/* Memory pool of a given kind */
	m_bush_t bush;
	m_addr_t (*getp)(struct m_pool *);
	void (*freep)(struct m_pool *, m_addr_t);
	int nump;
	m_vtob_s *(vtob[VTOB_HASH_SIZE]);
	struct m_pool *next;
	struct m_link h[PAGE_SHIFT-MEMO_SHIFT+MEMO_PAGE_ORDER+1];
} m_pool_s;

static void *___m_alloc(m_pool_s *mp, int size)
{
	int i = 0;
	int s = (1 << MEMO_SHIFT);
	int j;
	m_addr_t a;
	m_link_s *h = mp->h;

	if (size > (PAGE_SIZE << MEMO_PAGE_ORDER))
		return NULL;

	while (size > s) {
		s <<= 1;
		++i;
	}

	j = i;
	while (!h[j].next) {
		if (s == (PAGE_SIZE << MEMO_PAGE_ORDER)) {
			h[j].next = (m_link_s *)mp->getp(mp);
			if (h[j].next)
				h[j].next->next = NULL;
			break;
		}
		++j;
		s <<= 1;
	}
	a = (m_addr_t) h[j].next;
	if (a) {
		h[j].next = h[j].next->next;
		while (j > i) {
			j -= 1;
			s >>= 1;
			h[j].next = (m_link_s *) (a+s);
			h[j].next->next = NULL;
		}
	}
#ifdef DEBUG
	printk("___m_alloc(%d) = %p\n", size, (void *) a);
#endif
	return (void *) a;
}

static void ___m_free(m_pool_s *mp, void *ptr, int size)
{
	int i = 0;
	int s = (1 << MEMO_SHIFT);
	m_link_s *q;
	m_addr_t a, b;
	m_link_s *h = mp->h;

#ifdef DEBUG
	printk("___m_free(%p, %d)\n", ptr, size);
#endif

	if (size > (PAGE_SIZE << MEMO_PAGE_ORDER))
		return;

	while (size > s) {
		s <<= 1;
		++i;
	}

	a = (m_addr_t) ptr;

	while (1) {
#ifdef MEMO_FREE_UNUSED
		if (s == (PAGE_SIZE << MEMO_PAGE_ORDER)) {
			mp->freep(mp, a);
			break;
		}
#endif
		b = a ^ s;
		q = &h[i];
		while (q->next && q->next != (m_link_s *) b) {
			q = q->next;
		}
		if (!q->next) {
			((m_link_s *) a)->next = h[i].next;
			h[i].next = (m_link_s *) a;
			break;
		}
		q->next = q->next->next;
		a = a & b;
		s <<= 1;
		++i;
	}
}

static DEFINE_SPINLOCK(ncr53c8xx_lock);

static void *__m_calloc2(m_pool_s *mp, int size, char *name, int uflags)
{
	void *p;

	p = ___m_alloc(mp, size);

	if (DEBUG_FLAGS & DEBUG_ALLOC)
		printk ("new %-10s[%4d] @%p.\n", name, size, p);

	if (p)
		memset(p, 0, size);
	else if (uflags & MEMO_WARN)
		printk (NAME53C8XX ": failed to allocate %s[%d]\n", name, size);

	return p;
}

#define __m_calloc(mp, s, n)	__m_calloc2(mp, s, n, MEMO_WARN)

static void __m_free(m_pool_s *mp, void *ptr, int size, char *name)
{
	if (DEBUG_FLAGS & DEBUG_ALLOC)
		printk ("freeing %-10s[%4d] @%p.\n", name, size, ptr);

	___m_free(mp, ptr, size);

}

/*
 * With pci bus iommu support, we use a default pool of unmapped memory 
 * for memory we donnot need to DMA from/to and one pool per pcidev for 
 * memory accessed by the PCI chip. `mp0' is the default not DMAable pool.
 */

static m_addr_t ___mp0_getp(m_pool_s *mp)
{
	m_addr_t m = __get_free_pages(MEMO_GFP_FLAGS, MEMO_PAGE_ORDER);
	if (m)
		++mp->nump;
	return m;
}

static void ___mp0_freep(m_pool_s *mp, m_addr_t m)
{
	free_pages(m, MEMO_PAGE_ORDER);
	--mp->nump;
}

static m_pool_s mp0 = {NULL, ___mp0_getp, ___mp0_freep};

/*
 * DMAable pools.
 */

/*
 * With pci bus iommu support, we maintain one pool per pcidev and a 
 * hashed reverse table for virtual to bus physical address translations.
 */
static m_addr_t ___dma_getp(m_pool_s *mp)
{
	m_addr_t vp;
	m_vtob_s *vbp;

	vbp = __m_calloc(&mp0, sizeof(*vbp), "VTOB");
	if (vbp) {
		dma_addr_t daddr;
		vp = (m_addr_t) dma_alloc_coherent(mp->bush,
						PAGE_SIZE<<MEMO_PAGE_ORDER,
						&daddr, GFP_ATOMIC);
		if (vp) {
			int hc = VTOB_HASH_CODE(vp);
			vbp->vaddr = vp;
			vbp->baddr = daddr;
			vbp->next = mp->vtob[hc];
			mp->vtob[hc] = vbp;
			++mp->nump;
			return vp;
		}
	}
	if (vbp)
		__m_free(&mp0, vbp, sizeof(*vbp), "VTOB");
	return 0;
}

static void ___dma_freep(m_pool_s *mp, m_addr_t m)
{
	m_vtob_s **vbpp, *vbp;
	int hc = VTOB_HASH_CODE(m);

	vbpp = &mp->vtob[hc];
	while (*vbpp && (*vbpp)->vaddr != m)
		vbpp = &(*vbpp)->next;
	if (*vbpp) {
		vbp = *vbpp;
		*vbpp = (*vbpp)->next;
		dma_free_coherent(mp->bush, PAGE_SIZE<<MEMO_PAGE_ORDER,
				  (void *)vbp->vaddr, (dma_addr_t)vbp->baddr);
		__m_free(&mp0, vbp, sizeof(*vbp), "VTOB");
		--mp->nump;
	}
}

static inline m_pool_s *___get_dma_pool(m_bush_t bush)
{
	m_pool_s *mp;
	for (mp = mp0.next; mp && mp->bush != bush; mp = mp->next);
	return mp;
}

static m_pool_s *___cre_dma_pool(m_bush_t bush)
{
	m_pool_s *mp;
	mp = __m_calloc(&mp0, sizeof(*mp), "MPOOL");
	if (mp) {
		memset(mp, 0, sizeof(*mp));
		mp->bush = bush;
		mp->getp = ___dma_getp;
		mp->freep = ___dma_freep;
		mp->next = mp0.next;
		mp0.next = mp;
	}
	return mp;
}

static void ___del_dma_pool(m_pool_s *p)
{
	struct m_pool **pp = &mp0.next;

	while (*pp && *pp != p)
		pp = &(*pp)->next;
	if (*pp) {
		*pp = (*pp)->next;
		__m_free(&mp0, p, sizeof(*p), "MPOOL");
	}
}

static void *__m_calloc_dma(m_bush_t bush, int size, char *name)
{
	u_long flags;
	struct m_pool *mp;
	void *m = NULL;

	spin_lock_irqsave(&ncr53c8xx_lock, flags);
	mp = ___get_dma_pool(bush);
	if (!mp)
		mp = ___cre_dma_pool(bush);
	if (mp)
		m = __m_calloc(mp, size, name);
	if (mp && !mp->nump)
		___del_dma_pool(mp);
	spin_unlock_irqrestore(&ncr53c8xx_lock, flags);

	return m;
}

static void __m_free_dma(m_bush_t bush, void *m, int size, char *name)
{
	u_long flags;
	struct m_pool *mp;

	spin_lock_irqsave(&ncr53c8xx_lock, flags);
	mp = ___get_dma_pool(bush);
	if (mp)
		__m_free(mp, m, size, name);
	if (mp && !mp->nump)
		___del_dma_pool(mp);
	spin_unlock_irqrestore(&ncr53c8xx_lock, flags);
}

static m_addr_t __vtobus(m_bush_t bush, void *m)
{
	u_long flags;
	m_pool_s *mp;
	int hc = VTOB_HASH_CODE(m);
	m_vtob_s *vp = NULL;
	m_addr_t a = ((m_addr_t) m) & ~MEMO_CLUSTER_MASK;

	spin_lock_irqsave(&ncr53c8xx_lock, flags);
	mp = ___get_dma_pool(bush);
	if (mp) {
		vp = mp->vtob[hc];
		while (vp && (m_addr_t) vp->vaddr != a)
			vp = vp->next;
	}
	spin_unlock_irqrestore(&ncr53c8xx_lock, flags);
	return vp ? vp->baddr + (((m_addr_t) m) - a) : 0;
}

#define _m_calloc_dma(np, s, n)		__m_calloc_dma(np->dev, s, n)
#define _m_free_dma(np, p, s, n)	__m_free_dma(np->dev, p, s, n)
#define m_calloc_dma(s, n)		_m_calloc_dma(np, s, n)
#define m_free_dma(p, s, n)		_m_free_dma(np, p, s, n)
#define _vtobus(np, p)			__vtobus(np->dev, p)
#define vtobus(p)			_vtobus(np, p)

/*
 *  Deal with DMA mapping/unmapping.
 */

/* To keep track of the dma mapping (sg/single) that has been set */
#define __data_mapped	SCp.phase
#define __data_mapping	SCp.have_data_in

static void __unmap_scsi_data(struct device *dev, struct scsi_cmnd *cmd)
{
	switch(cmd->__data_mapped) {
	case 2:
		scsi_dma_unmap(cmd);
		break;
	}
	cmd->__data_mapped = 0;
}

static int __map_scsi_sg_data(struct device *dev, struct scsi_cmnd *cmd)
{
	int use_sg;

	use_sg = scsi_dma_map(cmd);
	if (!use_sg)
		return 0;

	cmd->__data_mapped = 2;
	cmd->__data_mapping = use_sg;

	return use_sg;
}

#define unmap_scsi_data(np, cmd)	__unmap_scsi_data(np->dev, cmd)
#define map_scsi_sg_data(np, cmd)	__map_scsi_sg_data(np->dev, cmd)

/*==========================================================
**
**	Driver setup.
**
**	This structure is initialized from linux config 
**	options. It can be overridden at boot-up by the boot 
**	command line.
**
**==========================================================
*/
static struct ncr_driver_setup
	driver_setup			= SCSI_NCR_DRIVER_SETUP;

#ifndef MODULE
#ifdef	SCSI_NCR_BOOT_COMMAND_LINE_SUPPORT
static struct ncr_driver_setup
	driver_safe_setup __initdata	= SCSI_NCR_DRIVER_SAFE_SETUP;
#endif
#endif /* !MODULE */

#define initverbose (driver_setup.verbose)
#define bootverbose (np->verbose)


/*===================================================================
**
**	Driver setup from the boot command line
**
**===================================================================
*/

#ifdef MODULE
#define	ARG_SEP	' '
#else
#define	ARG_SEP	','
#endif

#define OPT_TAGS		1
#define OPT_MASTER_PARITY	2
#define OPT_SCSI_PARITY		3
#define OPT_DISCONNECTION	4
#define OPT_SPECIAL_FEATURES	5
#define OPT_UNUSED_1		6
#define OPT_FORCE_SYNC_NEGO	7
#define OPT_REVERSE_PROBE	8
#define OPT_DEFAULT_SYNC	9
#define OPT_VERBOSE		10
#define OPT_DEBUG		11
#define OPT_BURST_MAX		12
#define OPT_LED_PIN		13
#define OPT_MAX_WIDE		14
#define OPT_SETTLE_DELAY	15
#define OPT_DIFF_SUPPORT	16
#define OPT_IRQM		17
#define OPT_PCI_FIX_UP		18
#define OPT_BUS_CHECK		19
#define OPT_OPTIMIZE		20
#define OPT_RECOVERY		21
#define OPT_SAFE_SETUP		22
#define OPT_USE_NVRAM		23
#define OPT_EXCLUDE		24
#define OPT_HOST_ID		25

#ifdef SCSI_NCR_IARB_SUPPORT
#define OPT_IARB		26
#endif

#ifdef MODULE
#define	ARG_SEP	' '
#else
#define	ARG_SEP	','
#endif

#ifndef MODULE
static char setup_token[] __initdata = 
	"tags:"   "mpar:"
	"spar:"   "disc:"
	"specf:"  "ultra:"
	"fsn:"    "revprob:"
	"sync:"   "verb:"
	"debug:"  "burst:"
	"led:"    "wide:"
	"settle:" "diff:"
	"irqm:"   "pcifix:"
	"buschk:" "optim:"
	"recovery:"
	"safe:"   "nvram:"
	"excl:"   "hostid:"
#ifdef SCSI_NCR_IARB_SUPPORT
	"iarb:"
#endif
	;	/* DONNOT REMOVE THIS ';' */

static int __init get_setup_token(char *p)
{
	char *cur = setup_token;
	char *pc;
	int i = 0;

	while (cur != NULL && (pc = strchr(cur, ':')) != NULL) {
		++pc;
		++i;
		if (!strncmp(p, cur, pc - cur))
			return i;
		cur = pc;
	}
	return 0;
}

static int __init sym53c8xx__setup(char *str)
{
#ifdef SCSI_NCR_BOOT_COMMAND_LINE_SUPPORT
	char *cur = str;
	char *pc, *pv;
	int i, val, c;
	int xi = 0;

	while (cur != NULL && (pc = strchr(cur, ':')) != NULL) {
		char *pe;

		val = 0;
		pv = pc;
		c = *++pv;

		if	(c == 'n')
			val = 0;
		else if	(c == 'y')
			val = 1;
		else
			val = (int) simple_strtoul(pv, &pe, 0);

		switch (get_setup_token(cur)) {
		case OPT_TAGS:
			driver_setup.default_tags = val;
			if (pe && *pe == '/') {
				i = 0;
				while (*pe && *pe != ARG_SEP && 
					i < sizeof(driver_setup.tag_ctrl)-1) {
					driver_setup.tag_ctrl[i++] = *pe++;
				}
				driver_setup.tag_ctrl[i] = '\0';
			}
			break;
		case OPT_MASTER_PARITY:
			driver_setup.master_parity = val;
			break;
		case OPT_SCSI_PARITY:
			driver_setup.scsi_parity = val;
			break;
		case OPT_DISCONNECTION:
			driver_setup.disconnection = val;
			break;
		case OPT_SPECIAL_FEATURES:
			driver_setup.special_features = val;
			break;
		case OPT_FORCE_SYNC_NEGO:
			driver_setup.force_sync_nego = val;
			break;
		case OPT_REVERSE_PROBE:
			driver_setup.reverse_probe = val;
			break;
		case OPT_DEFAULT_SYNC:
			driver_setup.default_sync = val;
			break;
		case OPT_VERBOSE:
			driver_setup.verbose = val;
			break;
		case OPT_DEBUG:
			driver_setup.debug = val;
			break;
		case OPT_BURST_MAX:
			driver_setup.burst_max = val;
			break;
		case OPT_LED_PIN:
			driver_setup.led_pin = val;
			break;
		case OPT_MAX_WIDE:
			driver_setup.max_wide = val? 1:0;
			break;
		case OPT_SETTLE_DELAY:
			driver_setup.settle_delay = val;
			break;
		case OPT_DIFF_SUPPORT:
			driver_setup.diff_support = val;
			break;
		case OPT_IRQM:
			driver_setup.irqm = val;
			break;
		case OPT_PCI_FIX_UP:
			driver_setup.pci_fix_up	= val;
			break;
		case OPT_BUS_CHECK:
			driver_setup.bus_check = val;
			break;
		case OPT_OPTIMIZE:
			driver_setup.optimize = val;
			break;
		case OPT_RECOVERY:
			driver_setup.recovery = val;
			break;
		case OPT_USE_NVRAM:
			driver_setup.use_nvram = val;
			break;
		case OPT_SAFE_SETUP:
			memcpy(&driver_setup, &driver_safe_setup,
				sizeof(driver_setup));
			break;
		case OPT_EXCLUDE:
			if (xi < SCSI_NCR_MAX_EXCLUDES)
				driver_setup.excludes[xi++] = val;
			break;
		case OPT_HOST_ID:
			driver_setup.host_id = val;
			break;
#ifdef SCSI_NCR_IARB_SUPPORT
		case OPT_IARB:
			driver_setup.iarb = val;
			break;
#endif
		default:
			printk("sym53c8xx_setup: unexpected boot option '%.*s' ignored\n", (int)(pc-cur+1), cur);
			break;
		}

		if ((cur = strchr(cur, ARG_SEP)) != NULL)
			++cur;
	}
#endif /* SCSI_NCR_BOOT_COMMAND_LINE_SUPPORT */
	return 1;
}
#endif /* !MODULE */

/*===================================================================
**
**	Get device queue depth from boot command line.
**
**===================================================================
*/
#define DEF_DEPTH	(driver_setup.default_tags)
#define ALL_TARGETS	-2
#define NO_TARGET	-1
#define ALL_LUNS	-2
#define NO_LUN		-1

static int device_queue_depth(int unit, int target, int lun)
{
	int c, h, t, u, v;
	char *p = driver_setup.tag_ctrl;