/*
 * IEEE 1394 for Linux
 *
 * Transaction support.
 *
 * Copyright (C) 1999 Andreas E. Bombe
 *
 * This code is licensed under the GPL.  See the file COPYING in the root
 * directory of the kernel sources for details.
 */

#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/hardirq.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/sched.h>  /* because linux/wait.h is broken if CONFIG_SMP=n */
#include <linux/wait.h>

#include <asm/bug.h>
#include <asm/errno.h>
#include <asm/system.h>

#include "ieee1394.h"
#include "ieee1394_types.h"
#include "hosts.h"
#include "ieee1394_core.h"
#include "ieee1394_transactions.h"

#define PREP_ASYNC_HEAD_ADDRESS(tc) \
        packet->tcode = tc; \
        packet->header[0] = (packet->node_id << 16) | (packet->tlabel << 10) \
                | (1 << 8) | (tc << 4); \
        packet->header[1] = (packet->host->node_id << 16) | (addr >> 32); \
        packet->header[2] = addr & 0xffffffff

#ifndef HPSB_DEBUG_TLABELS
static
#endif
DEFINE_SPINLOCK(hpsb_tlabel_lock);

static DECLARE_WAIT_QUEUE_HEAD(tlabel_wq);

static void fill_async_readquad(struct hpsb_packet *packet, u64 addr)
{
	PREP_ASYNC_HEAD_ADDRESS(TCODE_READQ);
	packet->header_size = 12;
	packet->data_size = 0;
	packet->expect_response = 1;
}

static void fill_async_readblock(struct hpsb_packet *packet, u64 addr,
				 int length)
{
	PREP_ASYNC_HEAD_ADDRESS(TCODE_READB);
	packet->header[3] = length << 16;
	packet->header_size = 16;
	packet->data_size = 0;
	packet->expect_response = 1;
}

static void fill_async_writequad(struct hpsb_packet *packet, u64 addr,
				 quadlet_t data)
{
	PREP_ASYNC_HEAD_ADDRESS(TCODE_WRITEQ);
	packet->header[3] = data;
	packet->header_size = 16;
	packet->data_size = 0;
	packet->expect_response = 1;
}

static void fill_async_writeblock(struct hpsb_packet *packet, u64 addr,
				  int length)
{
	PREP_ASYNC_HEAD_ADDRESS(TCODE_WRITEB);
	packet->header[3] = length << 16;
	packet->header_size = 16;
	packet->expect_response = 1;
	packet->data_size = length + (length % 4 ? 4 - (length % 4) : 0);
}

static void fill_async_lock(struct hpsb_packet *packet, u64 addr, int extcode,
			    in