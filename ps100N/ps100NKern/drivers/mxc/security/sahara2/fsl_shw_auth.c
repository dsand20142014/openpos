/*
 * video1394.c - video driver for OHCI 1394 boards
 * Copyright (C)1999,2000 Sebastien Rougeaux <sebastien.rougeaux@anu.edu.au>
 *                        Peter Schlaile <udbz@rz.uni-karlsruhe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * NOTES:
 *
 * ioctl return codes:
 * EFAULT is only for invalid address for the argp
 * EINVAL for out of range values
 * EBUSY when trying to use an already used resource
 * ESRCH when trying to free/stop a not used resource
 * EAGAIN for resource allocation failure that could perhaps succeed later
 * ENOTTY for unsupported ioctl request
 *
 */
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/timex.h>
#include <linux/mm.h>
#include <linux/compat.h>
#include <linux/cdev.h>

#include "dma.h"
#include "highlevel.h"
#include "hosts.h"
#include "ieee1394.h"
#include "ieee1394_core.h"
#include "ieee1394_hotplug.h"
#include "ieee1394_types.h"
#include "nodemgr.h"
#include "ohci1394.h"
#include "video1394.h"

#define ISO_CHANNELS 64

struct it_dma_prg {
	struct dma_cmd begin;
	quadlet_t data[4];
	struct dma_cmd end;
	quadlet_t pad[4]; /* FIXME: quick hack for memory alignment */
};

struct dma_iso_ctx {
	struct ti_ohci *ohci;
	int type; /* OHCI_ISO_TRANSMIT or OHCI_ISO_RECEIVE */
	struct ohci1394_iso_tasklet iso_tasklet;
	int channel;
	int ctx;
	int last_buffer;
	int * next_buffer;  /* For ISO Transmit of video packets
			       to write the correct SYT field
			       into the next block */
	unsigned int num_desc;
	unsigned int buf_size;
	unsigned int frame_size;
	unsigned int packet_size;
	unsigned int left_size;
	unsigned int nb_cmd;

	struct dma_region dma;

	struct dma_prog_region *prg_reg;

        struct dma_cmd **ir_prg;
	struct it_dma_prg **it_prg;

	unsigned int *buffer_status;
	unsigned int *buffer_prg_assignment;
        struct timeval *buffer_time; /* time when the buffer was received */
	unsigned int *last_used_cmd; /* For ISO Transmit with
					variable sized packets only ! */
	int ctrlClear;
	int ctrlSet;
	int cmdPtr;
	int ctxMatch;
	wait_queue_head_t waitq;
	spinlock_t lock;
	unsigned int syt_offset;
	int flags;

	struct list_head link;
};


struct file_ctx {
	struct ti_ohci *ohci;
	struct list_head context_list;
	struct dma_iso_ctx *current_ctx;
};

#ifdef CONFIG_IEEE1394_VERBOSEDEBUG
#define VIDEO1394_DEBUG
#endif

#ifdef DBGMSG
#undef DBGMSG
#endif

#ifdef VIDEO1394_DEBUG
#define DBGMSG(card, fmt, args...) \
printk(KERN_INFO "video1394_%d: " fmt "\n" , card , ## args)
#else
#define DBGMSG(card, fmt, args...) do {} while (0)
#endif

/* print general (card independent) information */
#define PRINT_G(level, fmt, args...) \
printk(level "video1394: " fmt "\n" , ## args)

/* print card specific information */
#define PRINT(level, card, fmt, args...) \
printk(level "video1394_%d: " fmt "\n" , card , ## args)

static void wakeup_dma_ir_ctx(unsigned long l);
static void wakeup_dma_it_ctx(unsigned long l);

static struct hpsb_highlevel video1394_highlevel;

static int free_dma_iso_ctx(struct dma_iso_ctx *d)
{
	int i;

	DBGMSG(d->ohci->host->id, "Freeing dma_iso_ctx %d", d->ctx);

	ohci1394_stop_context(d->ohci, d->ctrlClear, NULL);
	if (d->iso_tasklet.link.next != NULL)
		ohci1394_unregister_iso_tasklet(d->ohci, &d->iso_tasklet);

	dma_region_free(&d->dma);

	if (d->prg_reg) {
		for (i = 0; i < d->num_desc; i++)
			dma_prog_region_free(&d->prg_reg[i]);
		kfree(d->prg_reg);
	}

	kfree(d->ir_prg);
	kfree(d->it_prg);
	kfree(d->buffer_status);
	kfree(d->buffer_prg_assignment);
	kfree(d->buffer_time);
	kfree(d->last_used_cmd);
	kfree(d->next_buffer);
	list_del(&d->link);
	kfree(d);

	return 0;
}

static struct dma_iso_ctx *
alloc_dma_iso_ctx(struct ti_ohci *ohci, int type, int num_desc,
		  int buf_size, int channel, unsigned int packet_size)
{
	struct dma_iso_ctx *d;
	int i;

	d = kzalloc(sizeof(*d), GFP_KERNEL);
	if (!d) {
		PRINT(KERN_ERR, ohci->host->id, "Failed to allocate dma_iso_ctx");
		return NULL;
	}

	d->ohci = ohci;
	d->type = type;
	d->channel = channel;
	d->num_desc = num_desc;
	d->frame_size = buf_size;
	d->buf_size = PAGE_ALIGN(buf_size);
	d->last_buffer = -1;
	INIT_LIST_HEAD(&d->link);
	init_waitqueue_head(&d->waitq);

	/* Init the regions for easy cleanup */
	dma_region_init(&d->dma);

	if (dma_region_alloc(&d->dma, (d->num_desc - 1) * d->buf_size, ohci->dev,
			     PCI_DMA_BIDIRECTIONAL)) {
		PRINT(KERN_ERR, ohci->host->id, "Failed to allocate dma buffer");
		free_dma_iso_ctx(d);
		return NULL;
	}

	if (type == OHCI_ISO_RECEIVE)
		ohci1394_init_iso_tasklet(&d->iso_tasklet, type,
					  wakeup_dma_ir_ctx,
					  (unsigned long) d);
	else
		ohci1394_init_iso_tasklet(&d->iso_tasklet, type,
					  wakeup_dma_it_ctx,
					  (unsigned long) d);

	if (ohci1394_register_iso_tasklet(ohci, &d->iso_tasklet) < 0) {
		PRINT(KERN_ERR, ohci->host->id, "no free iso %s contexts",
		      type == OHCI_ISO_RECEIVE ? "receive" : "transmit");
		free_dma_iso_ctx(d);
		return NULL;
	}
	d->ctx = d->iso_tasklet.context;

	d->prg_reg = kmalloc(d->num_desc * sizeof(*d->prg_reg), GFP_KERNEL);
	if (!d->prg_reg) {
		PRINT(KERN_ERR, ohci->host->id, "Failed to allocate ir prg regs");
		free_dma_iso_ctx(d);
		return NULL;
	}
	/* Makes for easier cleanup */
	for (i = 0; i < d->num_desc; i++)
		dma_prog_region_init(&d->prg_reg[i]);

	if (type == OHCI_ISO_RECEIVE) {
		d->ctrlSet = OHCI1394_IsoRcvContextControlSet+32*d->ctx;
		d->ctrlClear = OHCI1394_IsoRcvContextControlClear+32*d->ctx;
		d->cmdPtr = OHCI1394_IsoRcvCommandPtr+32*d->ctx;
		d->ctxMatch = OHCI1394_IsoRcvContextMatch+32*d->ctx;

		d->ir_prg = kzalloc(d->num_desc * sizeof(*d->ir_prg),
				    GFP_KERNEL);

		if (!d->ir_prg) {
			PRINT(KERN_ERR, ohci->host->id, "Failed to allocate dma ir prg");
			free_dma_iso_ctx(d);
			return NULL;
		}

		d->nb_cmd = d->buf_size / PAGE_SIZE + 1;
		d->left_size = (d->frame_size % PAGE_SIZE) ?
			d->frame_size % PAGE_SIZE : PAGE_SIZE;

		for (i = 0;i < d->num_desc; i++) {
			if (dma_prog_region_alloc(&d->prg_reg[i], d->nb_cmd *
						  sizeof(struct dma_cmd), ohci->dev)) {
				PRINT(KERN_ERR, ohci->host->id, "Failed to allocate dma ir prg");
				free_dma_iso_ctx(d);
				return NULL;
			}
			d->ir_prg[i] = (struct dma_cmd *)d->prg_reg[i].kvirt;
		}

	} else {  /* OHCI_ISO_TRANSMIT */
		d->ctrlSet = OHCI1394_IsoXmitContextControlSet+16*d->ctx;
		d->ctrlClear = OHCI1394_IsoXmitContextControlClear+16*d->ctx;
		d->cmdPtr = OHCI1394_IsoXmitCommandPtr+16*d->ctx;

		d->it_prg = kzalloc(d->num_desc * sizeof(*d->it_prg),
				    GFP_KERNEL);

		if (!d->it_prg) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Failed to allocate dma it prg");
			free_dma_iso_ctx(d);
			return NULL;
		}

		d->packet_size = packet_size;

		if (PAGE_SIZE % packet_size || packet_size>4096) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Packet size %d (page_size: %ld) "
			      "not yet supported\n",
			      packet_size, PAGE_SIZE);
			free_dma_iso_ctx(d);
			return NULL;
		}

		d->nb_cmd = d->frame_size / d->packet_size;
		if (d->frame_size % d->packet_size) {
			d->nb_cmd++;
			d->left_size = d->frame_size % d->packet_size;
		} else
			d->left_size = d->packet_size;

		for (i = 0; i < d->num_desc; i++) {
			if (dma_prog_region_alloc(&d->prg_reg[i], d->nb_cmd *
						sizeof(struct it_dma_prg), ohci->dev)) {
				PRINT(KERN_ERR, ohci->host->id, "Failed to allocate dma it prg");
				free_dma_iso_ctx(d);
				return NULL;
			}
			d->it_prg[i] = (struct it_dma_prg *)d->prg_reg[i].kvirt;
		}
	}

	d->buffer_status =
	    kzalloc(d->num_desc * sizeof(*d->buffer_status), GFP_KERNEL);
	d->buffer_prg_assignment =
	    kzalloc(d->num_desc * sizeof(*d->buffer_prg_assignment), GFP_KERNEL);
	d->buffer_time =
	    kzalloc(d->num_desc * sizeof(*d->buffer_time), GFP_KERNEL);
	d->last_used_cmd =
	    kzalloc(d->num_desc * sizeof(*d->last_used_cmd), GFP_KERNEL);
	d->next_buffer =
	    kzalloc(d->num_desc * sizeof(*d->next_buffer), GFP_KERNEL);

	if (!d->buffer_status || !d->buffer_prg_assignment || !d->buffer_time ||
	    !d->last_used_cmd || !d->next_buffer) {
		PRINT(KERN_ERR, ohci->host->id,
		      "Failed to allocate dma_iso_ctx member");
		free_dma_iso_ctx(d);
		return NULL;
	}

        spin_lock_init(&d->lock);

	DBGMSG(ohci->host->id, "Iso %s DMA: %d buffers "
	      "of size %d allocated for a frame size %d, each with %d prgs",
	      (type == OHCI_ISO_RECEIVE) ? "receive" : "transmit",
	      d->num_desc - 1, d->buf_size, d->frame_size, d->nb_cmd);

	return d;
}

static void reset_ir_status(struct dma_iso_ctx *d, int n)
{
	int i;
	d->ir_prg[n][0].status = cpu_to_le32(4);
	d->ir_prg[n][1].status = cpu_to_le32(PAGE_SIZE-4);
	for (i = 2; i < d->nb_cmd - 1; i++)
		d->ir_prg[n][i].status = cpu_to_le32(PAGE_SIZE);
	d->ir_prg[n][i].status = cpu_to_le32(d->left_size);
}

static void reprogram_dma_ir_prg(struct dma_iso_ctx *d, int n, int buffer, int flags)
{
	struct dma_cmd *ir_prg = d->ir_prg[n];
	unsigned long buf = (unsigned long)d->dma.kvirt + buffer * d->buf_size;
	int i;

	d->buffer_prg_assignment[n] = buffer;

	ir_prg[0].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma, buf -
  	                        (unsigned long)d->dma.kvirt));
	ir_prg[1].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma,
				(buf + 4) - (unsigned long)d->dma.kvirt));

	for (i=2;i<d->nb_cmd-1;i++) {
		ir_prg[i].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma,
						(buf+(i-1)*PAGE_SIZE) -
						(unsigned long)d->dma.kvirt));
	}

	ir_prg[i].control = cpu_to_le32(DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE |
				  DMA_CTL_IRQ | DMA_CTL_BRANCH | d->left_size);
	ir_prg[i].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma,
				  (buf+(i-1)*PAGE_SIZE) - (unsigned long)d->dma.kvirt));
}

static void initialize_dma_ir_prg(struct dma_iso_ctx *d, int n, int flags)
{
	struct dma_cmd *ir_prg = d->ir_prg[n];
	struct dma_prog_region *ir_reg = &d->prg_reg[n];
	unsigned long buf = (unsigned long)d->dma.kvirt;
	int i;

	/* the first descriptor will read only 4 bytes */
	ir_prg[0].control = cpu_to_le32(DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE |
		DMA_CTL_BRANCH | 4);

	/* set the sync flag */
	if (flags & VIDEO1394_SYNC_FRAMES)
		ir_prg[0].control |= cpu_to_le32(DMA_CTL_WAIT);

	ir_prg[0].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma, buf -
				(unsigned long)d->dma.kvirt));
	ir_prg[0].branchAddress = cpu_to_le32((dma_prog_region_offset_to_bus(ir_reg,
					1 * sizeof(struct dma_cmd)) & 0xfffffff0) | 0x1);

	/* If there is *not* only one DMA page per frame (hence, d->nb_cmd==2) */
	if (d->nb_cmd > 2) {
		/* The second descriptor will read PAGE_SIZE-4 bytes */
		ir_prg[1].control = cpu_to_le32(DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE |
						DMA_CTL_BRANCH | (PAGE_SIZE-4));
		ir_prg[1].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma, (buf + 4) -
						(unsigned long)d->dma.kvirt));
		ir_prg[1].branchAddress = cpu_to_le32((dma_prog_region_offset_to_bus(ir_reg,
						      2 * sizeof(struct dma_cmd)) & 0xfffffff0) | 0x1);

		for (i = 2; i < d->nb_cmd - 1; i++) {
			ir_prg[i].control = cpu_to_le32(DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE |
							DMA_CTL_BRANCH | PAGE_SIZE);
			ir_prg[i].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma,
							(buf+(i-1)*PAGE_SIZE) -
							(unsigned long)d->dma.kvirt));

			ir_prg[i].branchAddress =
				cpu_to_le32((dma_prog_region_offset_to_bus(ir_reg,
					    (i + 1) * sizeof(struct dma_cmd)) & 0xfffffff0) | 0x1);
		}

		/* The last descriptor will generate an interrupt */
		ir_prg[i].control = cpu_to_le32(DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE |
						DMA_CTL_IRQ | DMA_CTL_BRANCH | d->left_size);
		ir_prg[i].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma,
						(buf+(i-1)*PAGE_SIZE) -
						(unsigned long)d->dma.kvirt));
	} else {
		/* Only one DMA page is used. Read d->left_size immediately and */
		/* generate an interrupt as this is also the last page. */
		ir_prg[1].control = cpu_to_le32(DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE |
						DMA_CTL_IRQ | DMA_CTL_BRANCH | (d->left_size-4));
		ir_prg[1].address = cpu_to_le32(dma_region_offset_to_bus(&d->dma,
						(buf + 4) - (unsigned long)d->dma.kvirt));
	}
}

static void initialize_dma_ir_ctx(struct dma_iso_ctx *d, int tag, int flags)
{
	struct ti_ohci *ohci = (struct ti_ohci *)d->ohci;
	int i;

	d->flags = flags;

	ohci1394_stop_context(ohci, d->ctrlClear, NULL);

	for (i=0;i<d->num_desc;i++) {
		initialize_dma_ir_prg(d, i, flags);
		reset_ir_status(d, i);
	}

	/* reset the ctrl register */
	reg_write(ohci, d->ctrlClear, 0xf0000000);

	/* Set bufferFill */
	reg_write(ohci, d->ctrlSet, 0x80000000);

	/* Set isoch header */
	if (flags & VIDEO1394_INCLUDE_ISO_HEADERS)
		reg_write(ohci, d->ctrlSet, 0x40000000);

	/* Set the context match register to match on all tags,
	   sync for sync tag, and listen to d->channel */
	reg_write(ohci, d->ctxMatch, 0xf0000000|((tag&0xf)<<8)|d->channel);

	/* Set up isoRecvIntMask to generate interrupts */
	reg_write(ohci, OHCI1394_IsoRecvIntMaskSet, 1<<d->ctx);
}

/* find which context is listening to this channel */
static struct dma_iso_ctx *
find_ctx(struct list_head *list, int type, int channel)
{
	struct dma_iso_ctx *ctx;

	list_for_each_entry(ctx, list, link) {
		if (ctx->type == type && ctx->channel == channel)
			return ctx;
	}

	return NULL;
}

static void wakeup_dma_ir_ctx(unsigned long l)
{
	struct dma_iso_ctx *d = (struct dma_iso_ctx *) l;
	int i;

	spin_lock(&d->lock);

	for (i = 0; i < d->num_desc; i++) {
		if (d->ir_prg[i][d->nb_cmd-1].status & cpu_to_le32(0xFFFF0000)) {
			reset_ir_status(d, i);
			d->buffer_status[d->buffer_prg_assignment[i]] = VIDEO1394_BUFFER_READY;
			do_gettimeofday(&d->buffer_time[d->buffer_prg_assignment[i]]);
			dma_region_sync_for_cpu(&d->dma,
				d->buffer_prg_assignment[i] * d->buf_size,
				d->buf_size);
		}
	}

	spin_unlock(&d->lock);

	if (waitqueue_active(&d->waitq))
		wake_up_interruptible(&d->waitq);
}

static inline void put_timestamp(struct ti_ohci *ohci, struct dma_iso_ctx * d,
				 int n)
{
	unsigned char* buf = d->dma.kvirt + n * d->buf_size;
	u32 cycleTimer;
	u32 timeStamp;

	if (n == -1) {
	  return;
	}

	cycleTimer = reg_read(ohci, OHCI1394_IsochronousCycleTimer);

	timeStamp = ((cycleTimer & 0x0fff) + d->syt_offset); /* 11059 = 450 us */
	timeStamp = (timeStamp % 3072 + ((timeStamp / 3072) << 12)
		+ (cycleTimer & 0xf000)) & 0xffff;

	buf[6] = timeStamp >> 8;
	buf[7] = timeStamp & 0xff;

    /* if first packet is empty packet, then put timestamp into the next full one too */
    if ( (le32_to_cpu(d->it_prg[n][0].data[1]) >>16) == 0x008) {
   	    buf += d->packet_size;
    	buf[6] = timeStamp >> 8;
	    buf[7] = timeStamp & 0xff;
	}

    /* do the next buffer frame too in case of irq latency */
	n = d->next_buffer[n];
	if (n == -1) {
	  return;
	}
	buf = d->dma.kvirt + n * d->buf_size;

	timeStamp += (d->last_used_cmd[n] << 12) & 0xffff;

	buf[6] = timeStamp >> 8;
	buf[7] = timeStamp & 0xff;

    /* if first packet is empty packet, then put timestamp into the next full one too */
    if ( (le32_to_cpu(d->it_prg[n][0].data[1]) >>16) == 0x008) {
   	    buf += d->packet_size;
    	buf[6] = timeStamp >> 8;
	    buf[7] = timeStamp & 0xff;
	}

#if 0
	printk("curr: %d, next: %d, cycleTimer: %08x timeStamp: %08x\n",
	       curr, n, cycleTimer, timeStamp);
#endif
}

static void wakeup_dma_it_ctx(unsigned long l)
{
	struct dma_iso_ctx *d = (struct dma_iso_ctx *) l;
	struct ti_ohci *ohci = d->ohci;
	int i;

	spin_lock(&d->lock);

	for (i = 0; i < d->num_desc; i++) {
		if (d->it_prg[i][d->last_used_cmd[i]].end.status &
		    cpu_to_le32(0xFFFF0000)) {
			int next = d->next_buffer[i];
			put_timestamp(ohci, d, next);
			d->it_prg[i][d->last_used_cmd[i]].end.status = 0;
			d->buffer_status[d->buffer_prg_assignment[i]] = VIDEO1394_BUFFER_READY;
		}
	}

	spin_unlock(&d->lock);

	if (waitqueue_active(&d->waitq))
		wake_up_interruptible(&d->waitq);
}

static void reprogram_dma_it_prg(struct dma_iso_ctx  *d, int n, int buffer)
{
	struct it_dma_prg *it_prg = d->it_prg[n];
	unsigned long buf = (unsigned long)d->dma.kvirt + buffer * d->buf_size;
	int i;

	d->buffer_prg_assignment[n] = buffer;
	for (i=0;i<d->nb_cmd;i++) {
	  it_prg[i].end.address =
		cpu_to_le32(dma_region_offset_to_bus(&d->dma,
			(buf+i*d->packet_size) - (unsigned long)d->dma.kvirt));
	}
}

static void initialize_dma_it_prg(struct dma_iso_ctx *d, int n, int sync_tag)
{
	struct it_dma_prg *it_prg = d->it_prg[n];
	struct dma_prog_region *it_reg = &d->prg_reg[n];
	unsigned long buf = (unsigned long)d->dma.kvirt;
	int i;
	d->last_used_cmd[n] = d->nb_cmd - 1;
	for (i=0;i<d->nb_cmd;i++) {

		it_prg[i].begin.control = cpu_to_le32(DMA_CTL_OUTPUT_MORE |
			DMA_CTL_IMMEDIATE | 8) ;
		it_prg[i].begin.address = 0;

		it_prg[i].begin.status = 0;

		it_prg[i].data[0] = cpu_to_le32(
			(IEEE1394_SPEED_100 << 16)
			| (/* tag */ 1 << 14)
			| (d->channel << 8)
			| (TCODE_ISO_DATA << 4));
		if (i==0) it_prg[i].data[0] |= cpu_to_le32(sync_tag);
		it_prg[i].data[1] = cpu_to_le32(d->packet_size << 16);
		it_prg[i].data[2] = 0;
		it_prg[i].data[3] = 0;

		it_prg[i].end.control = cpu_to_le32(DMA_CTL_OUTPUT_LAST |
			    	    	     DMA_CTL_BRANCH);
		it_prg[i].end.address =
			cpu_to_le32(dma_region_offset_to_bus(&d->dma, (buf+i*d->packet_size) -
						(unsigned long)d->dma.kvirt));

		if (i<d->nb_cmd-1) {
			it_prg[i].end.control |= cpu_to_le32(d->packet_size);
			it_prg[i].begin.branchAddress =
				cpu_to_le32((dma_prog_region_offset_to_bus(it_reg, (i + 1) *
					sizeof(struct it_dma_prg)) & 0xfffffff0) | 0x3);
			it_prg[i].end.branchAddress =
				cpu_to_le32((dma_prog_region_offset_to_bus(it_reg, (i + 1) *
					sizeof(struct it_dma_prg)) & 0xfffffff0) | 0x3);
		} else {
			/* the last prg generates an interrupt */
			it_prg[i].end.control |= cpu_to_le32(DMA_CTL_UPDATE |
				DMA_CTL_IRQ | d->left_size);
			/* the last prg doesn't branch */
			it_prg[i].begin.branchAddress = 0;
			it_prg[i].end.branchAddress = 0;
		}
		it_prg[i].end.status = 0;
	}
}

static void initialize_dma_it_prg_var_packet_queue(
	struct dma_iso_ctx *d, int n, unsigned int * packet_sizes,
	struct ti_ohci *ohci)
{
	struct it_dma_prg *it_prg = d->it_prg[n];
	struct dma_prog_region *it_reg = &d->prg_reg[n];
	int i;

#if 0
	if (n != -1) {
		put_timestamp(ohci, d, n);
	}
#endif
	d->last_used_cmd[n] = d->nb_cmd - 1;

	for (i = 0; i < d->nb_cmd; i++) {
		unsigned int size;
		if (packet_sizes[i] > d->packet_size) {
			size = d->packet_size;
		} else {
			size = packet_sizes[i];
		}
		it_prg[i].data[1] = cpu_to_le32(size << 16);
		it_prg[i].end.control = cpu_to_le32(DMA_CTL_OUTPUT_LAST | DMA_CTL_BRANCH);

		if (i < d->nb_cmd-1 && packet_sizes[i+1] != 0) {
			it_prg[i].end.control |= cpu_to_le32(size);
			it_prg[i].begin.branchAddress =
				cpu_to_le32((dma_prog_region_offset_to_bus(it_reg, (i + 1) *
					sizeof(struct it_dma_prg)) & 0xfffffff0) | 0x3);
			it_prg[i].end.branchAddress =
				cpu_to_le32((dma_prog_region_offset_to_bus(it_reg, (i + 1) *
					sizeof(struct it_dma_prg)) & 0xfffffff0) | 0x3);
		} else {
			/* the last prg generates an interrupt */
			it_prg[i].end.control |= cpu_to_le32(DMA_CTL_UPDATE |
				DMA_CTL_IRQ | size);
			/* the last prg doesn't branch */
			it_prg[i].begin.branchAddress = 0;
			it_prg[i].end.branchAddress = 0;
			d->last_used_cmd[n] = i;
			break;
		}
	}
}

static void initialize_dma_it_ctx(struct dma_iso_ctx *d, int sync_tag,
				  unsigned int syt_offset, int flags)
{
	struct ti_ohci *ohci = (struct ti_ohci *)d->ohci;
	int i;

	d->flags = flags;
	d->syt_offset = (syt_offset == 0 ? 11000 : syt_offset);

	ohci1394_stop_context(ohci, d->ctrlClear, NULL);

	for (i=0;i<d->num_desc;i++)
		initialize_dma_it_prg(d, i, sync_tag);

	/* Set up isoRecvIntMask to generate interrupts */
	reg_write(ohci, OHCI1394_IsoXmitIntMaskSet, 1<<d->ctx);
}

static inline unsigned video1394_buffer_state(struct dma_iso_ctx *d,
					      unsigned int buffer)
{
	unsigned long flags;
	unsigned int ret;
	spin_lock_irqsave(&d->lock, flags);
	ret = d->buffer_status[buffer];
	spin_unlock_irqrestore(&d->lock, flags);
	return ret;
}

static long video1394_ioctl(struct file *file,
			    unsigned int cmd, unsigned long arg)
{
	struct file_ctx *ctx = (struct file_ctx *)file->private_data;
	struct ti_ohci *ohci = ctx->ohci;
	unsigned long flags;
	void __user *argp = (void __user *)arg;

	switch(cmd)
	{
	case VIDEO1394_IOC_LISTEN_CHANNEL:
	case VIDEO1394_IOC_TALK_CHANNEL:
	{
		struct video1394_mmap v;
		u64 mask;
		struct dma_iso_ctx *d;
		int i;

		if (copy_from_user(&v, argp, sizeof(v)))
			return -EFAULT;

		/* if channel < 0, find lowest available one */
		if (v.channel < 0) {
		    mask = (u64)0x1;
		    for (i=0; ; i++) {
			if (i == ISO_CHANNELS) {
			    PRINT(KERN_ERR, ohci->host->id, 
				  "No free channel found");
			    return -EAGAIN;
			}
			if (!(ohci->ISO_channel_usage & mask)) {
			    v.channel = i;
			    PRINT(KERN_INFO, ohci->host->id, "Found free channel %d", i);
			    break;
			}
			mask = mask << 1;
		    }
		} else if (v.channel >= ISO_CHANNELS) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Iso channel %d out of bounds", v.channel);
			return -EINVAL;
		} else {
			mask = (u64)0x1<<v.channel;
		}
		DBGMSG(ohci->host->id, "mask: %08X%08X usage: %08X%08X\n",
			(u32)(mask>>32),(u32)(mask&0xffffffff),
			(u32)(ohci->ISO_channel_usage>>32),
			(u32)(ohci->ISO_channel_usage&0xffffffff));
		if (ohci->ISO