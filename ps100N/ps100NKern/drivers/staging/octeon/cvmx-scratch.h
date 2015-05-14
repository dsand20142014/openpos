/*
 * Node information (ConfigROM) collection and management.
 *
 * Copyright (C) 2000		Andreas E. Bombe
 *               2001-2003	Ben Collins <bcollins@debian.net>
 *
 * This code is licensed under the GPL.  See the file COPYING in the root
 * directory of the kernel sources for details.
 */

#include <linux/bitmap.h>
#include <linux/kernel.h>
#include <linux/kmemcheck.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/freezer.h>
#include <linux/semaphore.h>
#include <asm/atomic.h>

#include "csr.h"
#include "highlevel.h"
#include "hosts.h"
#include "ieee1394.h"
#include "ieee1394_core.h"
#include "ieee1394_hotplug.h"
#include "ieee1394_types.h"
#include "ieee1394_transactions.h"
#include "nodemgr.h"

static int ignore_drivers;
module_param(ignore_drivers, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ignore_drivers, "Disable automatic probing for drivers.");

struct nodemgr_csr_info {
	struct hpsb_host *host;
	nodeid_t nodeid;
	unsigned int generation;

	kmemcheck_bitfield_begin(flags);
	unsigned int speed_unverified:1;
	kmemcheck_bitfield_end(flags);
};


/*
 * Correct the speed map entry.  This is necessary
 *  - for nodes with link speed < phy speed,
 *  - for 1394b nodes with negotiated phy port speed < IEEE1394_SPEED_MAX.
 * A possible speed is determined by trial and error, using quadlet reads.
 */
static int nodemgr_check_speed(struct nodemgr_csr_info *ci, u64 addr,
			       quadlet_t *buffer)
{
	quadlet_t q;
	u8 i, *speed, old_speed, good_speed;
	int error;

	speed = &(ci->host->speed[NODEID_TO_NODE(ci->nodeid)]);
	old_speed = *speed;
	good_speed = IEEE1394_SPEED_MAX + 1;

	/* Try every speed from S100 to old_speed.
	 * If we did it the other way around, a too low speed could be caught
	 * if the retry succeeded for some other reason, e.g. because the link
	 * just finished its initialization. */
	for (i = IEEE1394_SPEED_100; i <= old_speed; i++) {
		*speed = i;
		error = hpsb_read(ci->host, ci->nodeid, ci->generation, addr,
				  &q, 4);
		if (error)
			break;
		*buffer = q;
		good_speed = i;
	}
	if (good_speed <= IEEE1394_SPEED_MAX) {
		HPSB_DEBUG("Speed probe of node " NODE_BUS_FMT " yields %s",
			   NODE_BUS_ARGS(ci->host, ci->nodeid),
			   hpsb_speedto_str[good_speed]);
		*speed = good_speed;
		ci->speed_unverified = 0;
		return 0;
	}
	*speed = old_speed;
	return error;
}

static int nodemgr_bus_read(struct csr1212_csr *csr, u64 addr,
			    void *buffer, void *__ci)
{
	struct nodemgr_csr_info *ci = (struct nodemgr_csr_info*)__ci;
	int i, error;

	for (i = 1; ; i++) {
		error = hpsb_read(ci->host, ci->nodeid, ci->generation, addr,
				  buffer, 4);
		if (!error) {
			ci->speed_unverified = 0;
			break;
		}
		/* Give up after 3rd failure. */
		if (i == 3)
			break;

		/* The ieee1394_core guessed the node's speed capability from
		 * the self ID.  Check whether a lower speed works. */
		if (ci->speed_unverified) {
			error = nodemgr_check_speed(ci, addr, buffer);
			if (!error)
				break;
		}
		if (msleep_interruptible(334))
			return -EINTR;
	}
	return error;
}

static struct csr1212_bus_ops nodemgr_csr_ops = {
	.bus_read =	nodemgr_bus_read,
};


/*
 * Basically what we do here is start off retrieving the bus_info block.
 * From there will fill in some info about the node, verify it is of IEEE
 * 1394 type, and that the crc checks out ok. After that we start off with
 * the root directory, and subdirectories. To do this, we retrieve the
 * quadlet header for a directory, find out the length, and retrieve the
 * complete directory entry (be it a leaf or a directory). We then process
 * it and add the info to our structure for that particular node.
 *
 * We verify CRC's along the way for each directory/block/leaf. The entire
 * node structure i