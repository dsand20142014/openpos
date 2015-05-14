/*
 * Real-Time Scheduling Class (mapped to the SCHED_FIFO and SCHED_RR
 * policies)
 */

static inline struct task_struct *rt_task_of(struct sched_rt_entity *rt_se)
{
	return container_of(rt_se, struct task_struct, rt);
}

#ifdef CONFIG_RT_GROUP_SCHED

#define rt_entity_is_task(rt_se) (!(rt_se)->my_q)

static inline struct rq *rq_of_rt_rq(struct rt_rq *rt_rq)
{
	return rt_rq->rq;
}

static inline struct rt_rq *rt_rq_of_se(struct sched_rt_entity *rt_se)
{
	return rt_se->rt_rq;
}

#else /* CONFIG_RT_GROUP_SCHED */

#define rt_entity_is_task(rt_se) (1)

static inline struct rq *rq_of_rt_rq(struct rt_rq *rt_rq)
{
	return container_of(rt_rq, struct rq, rt);
}

static inline struct rt_rq *rt_rq_of_se(struct sched_rt_entity *rt_se)
{
	struct task_struct *p = rt_task_of(rt_se);
	struct rq *rq = task_rq(p);

	return &rq->rt;
}

#endif /* CONFIG_RT_GROUP_SCHED */

#ifdef CONFIG_SMP

static inline int rt_overloaded(struct rq *rq)
{
	return atomic_read(&rq->rd->rto_count);
}

static inline void rt_set_overload(struct rq *rq)
{
	if (!rq->online)
		return;

	cpumask_set_cpu(rq->cpu, rq->rd->rto_mask);
	/*
	 * Make sure the mask is visible before we set
	 * the overload count. That is checked to determine
	 * if we should look at the mask. It would be a shame
	 * if we looked at the mask, but the mask was not
	 * updated yet.
	 */
	wmb();
	atomic_inc(&rq->rd->rto_count);
}

static inline void rt_clear_overload(struct rq *rq)
{
	if (!rq->online)
		return;

	/* the order here really doesn't matter */
	atomic_dec(&rq->rd->rto_count);
	cpumask_clear_cpu(rq->cpu, rq->rd->rto_mask);
}

static void update_rt_migration(struct rt_rq *rt_rq)
{
	if (rt_rq->rt_nr_migratory && rt_rq->rt_nr_total > 1) {
		if (!rt_rq->overloaded) {
			rt_set_overload(rq_of_rt_rq(rt_rq));
			rt_rq->overloaded = 1;
		}
	} else if (rt_rq->overloaded) {
		rt_clear_overload(rq_of_rt_rq(rt_rq));
		rt_rq->overloaded = 0;
	}
}

static void inc_rt_migration(struct sched_rt_entity *rt_se, struct rt_rq *rt_rq)
{
	if (!rt_entity_is_task(rt_se))
		return;

	rt_rq = &rq_of_rt_rq(rt_rq)->rt;

	rt_rq->rt_nr_total++;
	if (rt_se->nr_cpus_allowed > 1)
		rt_rq->rt_nr_migratory++;

	update_rt_migration(rt_rq);
}

static void dec_rt_migration(struct sched_rt_entity *rt_se, struct rt_rq *rt_rq)
{
	if (!rt_entity_is_task(rt_se))
		return;

	rt_rq = &rq_of_rt_rq(rt_rq)->rt;

	rt_rq->rt_nr_total--;
	if (rt_se->nr_cpus_allowed > 1)
		rt_rq->rt_nr_migratory--;

	update_rt_migration(rt_rq);
}

static void enqueue_pushable_task(struct rq *rq, struct task_struct *p)
{
	plist_del(&p->pushable_tasks, &rq->rt.pushable_tasks);
	plist_node_init(&p->pushable_tasks, p->prio);
	plist_add(&p->pushable_tasks, &rq->rt.pushable_tasks);
}

static void dequeue_pushable_task(struct rq *rq, struct task_struct *p)
{
	plist_del(&p->pushable_tasks, &rq->rt.pushable_tasks);
}

#else

static inline void enqueue_pushable_task(struct rq *rq, struct task_struct *p)
{
}

static inline void dequeue_pushable_task(struct rq *rq, struct task_struct *p)
{
}

static inline
void inc_rt_migration(struct sched_rt_entity *rt_se, struct rt_rq *rt_rq)
{
}

static inline
void dec_rt_migration(struct sched_rt_entity *rt_se, struct rt_rq *rt_rq)
{
}

#endif /* CONFIG_SMP */

static inline int on_rt_rq(struct sched_rt_entity *rt_se)
{
	return !list_empty(&rt_se->run_list);
}

#ifdef CONFIG_RT_GROUP_SCHED

static inline u64 sched_rt_runtime(struct rt_rq *rt_rq)
{
	if (!rt_rq->tg)
		return RUNTIME_INF;

	return rt_rq->rt_runtime;
}

static inline u64 sched_rt_period(struct rt_rq *rt_rq)
{
	return ktime_to_ns(rt_rq->tg->rt_bandwidth.rt_period);
}

#define for_each_leaf_rt_rq(rt_rq, rq) \
	list_for_each_entry_rcu(rt_rq, &rq->leaf_rt_rq_list, leaf_rt_rq_list)

#define for_each_sched_rt_entity(rt_se) \
	for (; rt_se; rt_se = rt_se->parent)

static inline struct rt_rq *group_rt_rq(struct sched_rt_entity *rt_se)
{
	return rt_se->my_q;
}

static void enqueue_rt_entity(struct sched_rt_entity *rt_se);
static void dequeue_rt_entity(struct sched_rt_entity *rt_se);

static void sched_rt_rq_enqueue(struct rt_rq *rt_rq)
{
	struct task_struct *curr = rq->dd->ipath_lid)) {
			/*
			 * If DMAs are in progress, we can't generate
			 * a completion for the loopback packet since
			 * it would be out of order.
			 * XXX Instead of waiting, we could queue a
			 * zero length descriptor so we get a callback.
			 */
			if (atomic_read(&qp->s_dma_busy)) {
				qp->s_flags |= IPATH_S_WAIT_DMA;
				goto bail;
			}
			qp->s_cur = next_cur;
			spin_unlock_irqrestore(&qp->s_lock, flags);
			ipath_ud_loopback(qp, wqe);
			spin_lock_irqsave(&qp->s_lock, flags);
			ipath_send_complete(qp, wqe, IB_WC_SUCCESS);
			goto done;
		}
	}

	qp->s_cur = next_cur;
	extra_bytes = -wqe->length & 3;
	nwords = (wqe->length + extra_bytes) >> 2;

	/* header size in 32-bit words LRH+BTH+DETH = (8+12+8)/4. */
	qp->s_hdrwords = 7;
	qp->s_cur_size = wqe->length;
	qp->s_cur_sge = &qp->s_sge;
	qp->s_dmult = ah_attr->static_rate;
	qp->s_wqe = wqe;
	qp->s_sge.sge = wqe->sg_list[0];
	qp->s_sge.sg_list = wqe->sg_list + 1;
	qp->s_sge.num_sge = wqe->wr.num_sge;

	if (ah_attr->ah_flags & IB_AH_GRH) {
		/* Header size in 32-bit words. */
		qp->s_hdrwords += ipath_make_grh(dev, &qp->s_hdr.u.l.grh,
						 &ah_attr->grh,
						 qp->s_hdrwords, nwords);
		lrh0 = IPATH_LRH_GRH;
		ohdr = &qp->s_hdr.u.l.oth;
		/*
		 * Don't worry about sending to locally attached multicast
		 * QPs.  It is unspecified by the spec. what happens.
		 */
	} else {
		/* Header size in 32-bit words. */
		lrh0 = IPATH_LRH_BTH;
		ohdr = &qp->s_hdr.u.oth;
	}
	if (wqe->wr.opcode == IB_WR_SEND_WITH_IMM) {
		qp->s_hdrwords++;
		ohdr->u.ud.imm_data = wqe->wr.ex.imm_data;
		bth0 = IB_OPCODE_UD_SEND_ONLY_WITH_IMMEDIATE << 24;
	} else
		bth0 = IB_OPCODE_UD_SEND_ONLY << 24;
	lrh0 |= ah_attr->sl << 4;
	if (qp->ibqp.qp_type == IB_QPT_SMI)
		lrh0 |= 0xF000;	/* Set VL (see ch. 13.5.3.1) */
	qp->s_hdr.lrh[0] = cpu_to_be16(lrh0);
	qp->s_hdr.lrh[1] = cpu_to_be16(ah_attr->dlid);	/* DEST LID */
	qp->s_hdr.lrh[2] = cpu_to_be16(qp->s_hdrwords + nwords +
					   SIZE_OF_CRC);
	lid = dev->dd->ipath_lid;
	if (lid) {
		lid |= ah_attr->src_path_bits &
			((1 << dev->dd->ipath_lmc) - 1);
		qp->s_hdr.lrh[3] = cpu_to_be16(lid);
	} else
		qp->s_hdr.lrh[3] = IB_LID_PERMISSIVE;
	if (wqe->wr.send_flags & IB_SEND_SOLICITED)
		bth0 |= 1 << 23;
	bth0 |= extra_bytes << 20;
	bth0 |= qp->ibqp.qp_type == IB_QPT_SMI ? IPATH_DEFAULT_P_KEY :
		ipath_get_pkey(dev->dd, qp->s_pkey_index);
	ohdr->bth[0] = cpu_to_be32(bth0);
	/*
	 * Use the multicast QP if the destination LID is a multicast LID.
	 */
	ohdr->bth[1] = ah_attr->dlid >= IPATH_MULTICAST_LID_BASE &&
		ah_attr->dlid != IPATH_PERMISSIVE_LID ?
		cpu_to_be32(IPATH_MULTICAST_QPN) :
		cpu_to_be32(wqe->wr.wr.ud.remote_qpn);
	ohdr->bth[2] = cpu_to_be32(qp->s_next_psn++ & IPATH_PSN_MASK);
	/*
	 * Qkeys with the high order bit set mean use the
	 * qkey from the QP context instead of the WR (see 10.2.5).
	 */
	ohdr->u.ud.deth[0] = cpu_to_be32((int)wqe->wr.wr.ud.remote_qkey < 0 ?
					 qp->qkey : wqe->wr.wr.ud.remote_qkey);
	ohdr->u.ud.deth[1] = cpu_to_be32(qp->ibqp.qp_num);

done:
	ret = 1;
	goto unlock;

bail:
	qp->s_flags &= ~IPATH_S_BUSY;
unlock:
	spin_unlock_irqrestore(&qp->s_lock, flags);
	return ret;
}

/**
 * ipath_ud_rcv - receive an incoming UD packet
 * @dev: the device the packet came in on
 * @hdr: the packet header
 * @has_grh: true if the packet has a GRH
 * @data: the packet data
 * @tlen: the packet length
 * @qp: the QP the packet came on
 *
 * This is called from ipath_qp_rcv() to process an incoming UD packet
 * for the given QP.
 * Called at interrupt level.
 */
void ipath_ud_rcv(struct ipath_ibdev *dev, struct ipath_ib_header *hdr,
		  int has_grh, void *data, u32 tlen, struct ipath_qp *qp)
{
	struct ipath_other_headers *ohdr;
	int opcode;
	u32 hdrsize;
	u32 pad;
	struct ib_wc wc;
	u32 qkey;
	u32 src_qp;
	u16 dlid;
	int header_in_data;

	/* Check for GRH */
	if (!has_grh) {
		ohdr = &hdr->u.oth;
		hdrsize = 8 + 12 + 8;	/* LRH + BTH + DETH */
		qkey = be32_to_cpu(ohdr->u.ud.deth[0]);
		src_qp = be32_to_cpu(ohdr->u.ud.deth[1]);
		header_in_data = 0;
	} else {
		ohdr = &hdr->u.l.oth;
		hdrsize = 8 + 40 + 12 + 8; /* LRH + GRH + BTH + DETH */
		/*
		 * The header with GRH is 68 bytes and the core driver sets
		 * the eager header buffer size to 56 bytes so the last 12
		 * bytes of the IB header is in the data buffer.
		 */
		header_in_data = dev->dd->ipath_rcvhdrentsize == 16;
		if (header_in_data) {
			qkey = be32_to_cpu(((__be32 *) data)[1]);
			src_qp = be32_to_cpu(((__be32 *) data)[2]);
			data += 12;
		} else {
			qkey = be32_to_cpu(ohdr->u.ud.deth[0]);
			src_qp = be32_to_cpu(ohdr->u.ud.deth[1]);
		}
	}
	src_qp &= IPATH_QPN_MASK;

	/*
	 * Check that the permissive LID is only used on QP0
	 * and the QKEY matches (see 9.6.1.4.1 and 9.6.1.5.1).
	 */
	if (qp->ibqp.qp_num) {
		if (unlikely(hdr->lrh[1] == IB_LID_PERMISSIVE ||
			     hdr->lrh[3] == IB_LID_PERMISSIVE)) {
			dev->n_pkt_drops++;
			goto bail;
		}
		if (unlikely(qkey != qp->qkey)) {
			/* XXX OK to lose a count once in a while. */
			dev->qkey_violations++;
			dev->n_pkt_drops++;
			goto bail;
		}
	} else if (hdr->lrh[1] == IB_LID_PERMISSIVE ||
		   hdr->lrh[3] == IB_LID_PERMISSIVE) {
		struct ib_smp *smp = (struct ib_smp *) data;

		if (smp->mgmt_class != IB_MGMT_CLASS_SUBN_DIRECTED_ROUTE) {
			dev->n_pkt_drops++;
			goto bail;
		}
	}

	/*
	 * The opcode is in the low byte when its in network order
	 * (top byte when in host order).
	 */
	opcode = be32_to_cpu(ohdr->bth[0]) >> 24;
	if (qp->ibqp.qp_num > 1 &&
	    opcode == IB_OPCODE_UD_SEND_ONLY_WITH_IMMEDIATE) {
		if (header_in_data) {
			wc.ex.imm_data = *(__be32 *) data;
			data += sizeof(__be32);
		} else
			wc.ex.imm_data = ohdr->u.ud.imm_data;
		wc.wc_flags = IB_WC_WITH_IMM;
		hdrsize += sizeof(u32);
	} else if (opcode == IB_OPCODE_UD_SEND_ONLY) {
		wc.ex.imm_data = 0;
		wc.wc_flags = 0;
	} else {
		dev->n_pkt_drops++;
		goto bail;
	}

	/* Get the number of bytes the message was padded by. */
	pad = (be32_to_cpu(ohdr->bth[0]) >> 20) & 3;
	if (unlikely(tlen < (hdrsize + pad + 4))) {
		/* Drop incomplete packets. */
		dev->n_pkt_drops++;
		goto bail;
	}
	tlen -= hdrsize + pad + 4;

	/* Drop invalid MAD packets (see 13.5.3.1). */
	if (unlikely((qp->ibqp.qp_num == 0 &&
		      (tlen != 256 ||
		       (be16_to_cpu(hdr->lrh[0]) >> 12) != 15)) ||
		     (qp->ibqp.qp_num == 1 &&
		      (tlen != 256 ||
		       (be16_to_cpu(hdr->lrh[0]) >> 12) == 15)))) {
		dev->n_pkt_drops++;
		goto bail;
	}

	/*
	 * A GRH is expected to preceed the data even if not
	 * present on the wire.
	 */
	wc.byte_len = tlen + sizeof(struct ib_grh);

	/*
	 * Get the next work request entry to find where to put the data.
	 */
	if (qp->r_flags & IPATH_R_REUSE_SGE)
		qp->r_flags &= ~IPATH_R_REUSE_SGE;
	else if (!ipath_get_rwqe(qp, 0)) {
		/*
		 * Count VL15 packets dropped due to no receive buffer.
		 * Otherwise, count them as buffer overruns since usually,
		 * the HW will be able to receive packets even if there are
		 * no QPs with posted receive buffers.
		 */
		if (qp->ibqp.qp_num == 0)
			dev->n_vl15_dropped++;
		else
			dev->rcv_errors++;
		goto bail;
	}
	/* Silently drop packets which are too big. */
	if (wc.byte_len > qp->r_len) {
		qp->r_flags |= IPATH_R_REUSE_SGE;
		dev->n_pkt_drops++;
		goto bail;
	}
	if (has_grh) {
		ipath_copy_sge(&qp->r_sge, &hdr->u.l.grh,
			       sizeof(struct ib_grh));
		wc.wc_flags |= IB_WC_GRH;
	} else
		ipath_skip_sge(&qp->r_sge, sizeof(struct ib_grh));
	ipath_copy_sge(&qp->r_sge, data,
		       wc.byte_len - sizeof(struct ib_grh));
	if (!test_and_clear_bit(IPATH_R_WRID_VALID, &qp->r_aflags))
		goto bail;
	wc.wr_id = qp->r_wr_id;
	wc.status = IB_WC_SUCCESS;
	wc.opcode = IB_WC_RECV;
	wc.vendor_err = 0;
	wc.qp = &qp->ibqp;
	wc.src_qp = src_qp;
	/* XXX do we know which pkey matched? Only needed for GSI. */
	wc.pkey_index = 0;
	wc.slid = be16_to_cpu(hdr->lrh[3]);
	wc.sl = (be16_to_cpu(hdr->lrh[0]) >> 4) & 0xF;
	dlid = be16_to_cpu(hdr->lrh[1]);
	/*
	 * Save the LMC lower bits if the destination LID is a unicast LID.
	 */
	wc.dlid_path_bits = dlid >= IPATH_MULTICAST_LID_BASE ? 0 :
		dlid & ((1 << dev->dd->ipath_lmc) - 1);
	wc.port_num = 1;
	/* Signal completion event if the solicited bit is set. */
	ipath_cq_enter(to_icq(qp->ibqp.recv_cq), &wc,
		       (ohdr->bth[0] &
			cpu_to_be32(1 << 23)) != 0);

bail:;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         /*
 * OmniVision OV511 Camera-to-USB Bridge Driver
 *
 * Copyright (c) 1999-2003 Mark W. McClelland
 * Original decompression code Copyright 1998-2000 OmniVision Technologies
 * Many improvements by Bret Wallach <bwallac1@san.rr.com>
 * Color fixes by by Orion Sky Lawlor <olawlor@acm.org> (2/26/2000)
 * Snapshot code by Kevin Moore
 * OV7620 fixes by Charl P. Botha <cpbotha@ieee.org>
 * Changes by Claudio Matsuoka <claudio@conectiva.com>
 * Original SAA7111A code by Dave Perks <dperks@ibm.net>
 * URB error messages from pwc driver by Nemosoft
 * generic_ioctl() code from videodev.c by Gerd Knorr and Alan Cox
 * Memory management (rvmalloc) code from bttv driver, by Gerd Knorr and others
 *
 * Based on the Linux CPiA driver written by Peter Pregler,
 * Scott J. Bertin and Johannes Erdfelt.
 *
 * Please see the file: Documentation/usb/ov511.txt
 * and the website at:  http://alpha.dyndns.org/ov511
 * for more info.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <asm/processor.h>
#include <linux/mm.h>
#include <linux/device.h>

#if defined (__i386__)
	#include <asm/cpufeature.h>
#endif

#include "ov511.h"

/*
 * Version Information
 */
#define DRIVER_VERSION "v1.64 for Linux 2.5"
#define EMAIL "mark@alpha.dyndns.org"
#define DRIVER_AUTHOR "Mark McClelland <mark@alpha.dyndns.org> & Bret Wallach \
	& Orion Sky Lawlor <olawlor@acm.org> & Kevin Moore & Charl P. Botha \
	<cpbotha@ieee.org> & Claudio Matsuoka <claudio@conectiva.com>"
#define DRIVER_DESC "ov511 USB Camera Driver"

#define OV511_I2C_RETRIES 3
#define ENABLE_Y_QUANTABLE 1
#define ENABLE_UV_QUANTABLE 1

#define OV511_MAX_UNIT_VIDEO 16

/* Pixel count * bytes per YUV420 pixel (1.5) */
#define MAX_FRAME_SIZE(w, h) ((w) * (h) * 3 / 2)

#define MAX_DATA_SIZE(w, h) (MAX_FRAME_SIZE(w, h) + sizeof(struct timeval))

/* Max size * bytes per YUV420 pixel (1.5) + one extra isoc frame for safety */
#define MAX_RAW_DATA_SIZE(w, h) ((w) * (h) * 3 / 2 + 1024)

#define FATAL_ERROR(rc) ((rc) < 0 && (rc) != -EPERM)

/**********************************************************************
 * Module Parameters
 * (See ov511.txt for detailed descriptions of these)
 **********************************************************************/

/* These variables (and all static globals) default to zero */
static int autobright		= 1;
static int autogain		= 1;
static int autoexp		= 1;
static int debug;
static int snapshot;
static int cams			= 1;
static int compress;
static int testpat;
static int dumppix;
static int led 			= 1;
static int dump_bridge;
static int dump_sensor;
static int printph;
static int phy			= 0x1f;
static int phuv			= 0x05;
static int pvy			= 0x06;
static int pvuv			= 0x06;
static int qhy			= 0x14;
static int qhuv			= 0x03;
static int qvy			= 0x04;
static int qvuv			= 0x04;
static int lightfreq;
static int bandingfilter;
static int clockdiv		= -1;
static int packetsize		= -1;
static int framedrop		= -1;
static int fastset;
static int force_palette;
static int backlight;
/* Bitmask marking allocated devices from 0 to OV511_MAX_UNIT_VIDEO */
static unsigned long ov511_devused;
static int unit_video[OV511_MAX_UNIT_VIDEO];
static int remove_zeros;
static int mirror;
static int ov518_color;

module_param(autobright, int, 0);
MODULE_PARM_DESC(autobright, "Sensor automatically changes brightness");
module_param(autogain, int, 0);
MODULE_PARM_DESC(autogain, "Sensor automatically changes gain");
module_param(autoexp, int, 0);
MODULE_PARM_DESC(autoexp, "Sensor automatically changes exposure");
module_param(debug, int, 0);
MODULE_PARM_DESC(debug,
  "Debug level: 0=none, 1=inits, 2=warning, 3=config, 4=functions, 5=max");
module_param(snapshot, int, 0);
MODULE_PARM_DESC(snapshot, "Enable snapshot mode");
module_param(cams, int, 0);
MODULE_PARM_DESC(cams, "Number of simultaneous cameras");
module_param(compress, int, 0);
MODULE_PARM_DESC(compress, "Turn on compression");
module_param(testpat, int, 0);
MODULE_PARM_DESC(testpat,
  "Replace image with vertical bar testpattern (only partially working)");
module_param(dumppix, int, 0);
MODULE_PARM_DESC(dumppix, "Dump raw pixel data");
module_param(led, int, 0);
MODULE_PARM_DESC(led,
  "LED policy (OV511+ or later). 0=off, 1=on (default), 2=auto (on when open)");
module_param(dump_bridge, int, 0);
MODULE_PARM_DESC(dump_bridge, "Dump the bridge registers");
module_param(dump_sensor, int, 0);
MODULE_PARM_DESC(dump_sensor, "Dump the sensor registers");
module_param(printph, int, 0);
MODULE_PARM_DESC(printph, "Print frame start/end headers");
module_param(phy, int, 0);
MODULE_PARM_DESC(phy, "Prediction range (horiz. Y)");
module_param(phuv, int, 0);
MODULE_PARM_DESC(phuv, "Prediction range (horiz. UV)");
module_param(pvy, int, 0);
MODULE_PARM_DESC(pvy, "Prediction range (vert. Y)");
module_param(pvuv, int, 0);
MODULE_PARM_DESC(pvuv, "Prediction range (vert. UV)");
module_param(qhy, int, 0);
MODULE_PARM_DESC(qhy, "Quantization threshold (horiz. Y)");
module_param(qhuv, int, 0);
MODULE_PARM_DESC(qhuv, "Quantization threshold (horiz. UV)");
module_param(qvy, int, 0);
MODULE_PARM_DESC(qvy, "Quantization threshold (vert. Y)");
module_param(qvuv, int, 0);
MODULE_PARM_DESC(qvuv, "Quantization threshold (vert. UV)");
module_param(lightfreq, int, 0);
MODULE_PARM_DESC(lightfreq,
  "Light frequency. Set to 50 or 60 Hz, or zero for default settings");
module_param(bandingfilter, int, 0);
MODULE_PARM_DESC(bandingfilter,
  "Enable banding filter (to reduce effects of fluorescent lighting)");
module_param(clockdiv, int, 0);
MODULE_PARM_DESC(clockdiv, "Force pixel clock divisor to a specific value");
module_param(packetsize, int, 0);
MODULE_PARM_DESC(packetsize, "Force a specific isoc packet size");
module_param(framedrop, int, 0);
MODULE_PARM_DESC(framedrop, "Force a specific frame drop register setting");
module_param(fastset, int, 0);
MODULE_PARM_DESC(fastset, "Allows picture settings to take effect immediately");
module_param(force_palette, int, 0);
MODULE_PARM_DESC(force_palette, "Force the palette to a specific value");
module_param(backlight, int, 0);
MODULE_PARM_DESC(backlight, "For objects that are lit from behind");
static unsigned int num_uv;
module_param_array(unit_video, int, &num_uv, 0);
MODULE_PARM_DESC(unit_video,
  "Force use of specific minor number(s). 0 is not allowed.");
module_param(remove_zeros, int, 0);
MODULE_PARM_DESC(remove_zeros,
  "Remove zero-padding from uncompressed incoming data");
module_param(mirror, int, 0);
MODULE_PARM_DESC(mirror, "Reverse image horizontally");
module_param(ov518_color, int, 0);
MODULE_PARM_DESC(ov518_color, "Enable OV518 color (experimental)");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/**********************************************************************
 * Miscellaneous Globals
 **********************************************************************/

static struct usb_driver ov511_driver;

/* Number of times to retry a failed I2C transaction. Increase this if you
 * are getting "Failed to read sensor ID..." */
static const int i2c_detect_tries = 5;

static struct usb_device_id device_table [] = {
	{ USB_DEVICE(VEND_OMNIVISION, PROD_OV511) },
	{ USB_DEVICE(VEND_OMNIVISION, PROD_OV511PLUS) },
	{ USB_DEVICE(VEND_MATTEL, PROD_ME2CAM) },
	{ }  /* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, device_table);

static unsigned char yQuanTable511[] = OV511_YQUANTABLE;
static unsigned char uvQuanTable511[] = OV511_UVQUANTABLE;
static unsigned char yQuanTable518[] = OV518_YQUANTABLE;
static unsigned char uvQuanTable518[] = OV518_UVQUANTABLE;

/**********************************************************************
 * Symbolic Names
 **********************************************************************/

/* Known OV511-based cameras */
static struct symbolic_list camlist[] = {
	{   0, "Generic Camera (no ID)" },
	{   1, "Mustek WCam 3X" },
	{   3, "D-Link DSB-C300" },
	{   4, "Generic OV511/OV7610" },
	{   5, "Puretek PT-6007" },
	{   6, "Lifeview USB Life TV (NTSC)" },
	{  21, "Creative Labs WebCam 3" },
	{  22, "Lifeview USB Life TV (PAL D/K+B/G)" },
	{  36, "Koala-Cam" },
	{  38, "Lifeview USB Life TV (PAL)" },
	{  41, "Samsung Anycam MPC-M10" },
	{  43, "Mtekvision Zeca MV402" },
	{  46, "Suma eON" },
	{  70, "Lifeview USB Life TV (PAL/SECAM)" },
	{ 100, "Lifeview RoboCam" },
	{ 102, "AverMedia InterCam Elite" },
	{ 112, "MediaForte MV300" },	/* or OV7110 evaluation kit */
	{ 134, "Ezonics EZCam II" },
	{ 192, "Webeye 2000B" },
	{ 253, "Alpha Vision Tech. AlphaCam SE" },
	{  -1, NULL }
};

/* Video4Linux1 Palettes */
static struct symbolic_list v4l1_plist[] = {
	{ VIDEO_PALETTE_GREY,	"GREY" },
	{ VIDEO_PALETTE_HI240,	"HI240" },
	{ VIDEO_PALETTE_RGB565,	"RGB565" },
	{ VIDEO_PALETTE_RGB24,	"RGB24" },
	{ VIDEO_PALETTE_RGB32,	"RGB32" },
	{ VIDEO_PALETTE_RGB555,	"RGB555" },
	{ VIDEO_PALETTE_YUV422,	"YUV422" },
	{ VIDEO_PALETTE_YUYV,	"YUYV" },
	{ VIDEO_PALETTE_UYVY,	"UYVY" },
	{ VIDEO_PALETTE_YUV420,	"YUV420" },
	{ VIDEO_PALETTE_YUV411,	"YUV411" },
	{ VIDEO_PALETTE_RAW,	"RAW" },
	{ VIDEO_PALETTE_YUV422P,"YUV422P" },
	{ VIDEO_PALETTE_YUV411P,"YUV411P" },
	{ VIDEO_PALETTE_YUV420P,"YUV420P" },
	{ VIDEO_PALETTE_YUV410P,"YUV410P" },
	{ -1, NULL }
};

static struct symbolic_list brglist[] = {
	{ BRG_OV511,		"OV511" },
	{ BRG_OV511PLUS,	"OV511+" },
	{ BRG_OV518,		"OV518" },
	{ BRG_OV518PLUS,	"OV518+" },
	{ -1, NULL }
};

static struct symbolic_list senlist[] = {
	{ SEN_OV76BE,	"OV76BE" },
	{ SEN_OV7610,	"OV7610" },
	{ SEN_OV7620,	"OV7620" },
	{ SEN_OV7620AE,	"OV7620AE" },
	{ SEN_OV6620,	"OV6620" },
	{ SEN_OV6630,	"OV6630" },
	{ SEN_OV6630AE,	"OV6630AE" },
	{ SEN_OV6630AF,	"OV6630AF" },
	{ SEN_OV8600,	"OV8600" },
	{ SEN_KS0127,	"KS0127" },
	{ SEN_KS0127B,	"KS0127B" },
	{ SEN_SAA7111A,	"SAA7111A" },
	{ -1, NULL }
};

/* URB error codes: */
static struct symbolic_list urb_errlist[] = {
	{ -ENOSR,	"Buffer error (overrun)" },
	{ -EPIPE,	"Stalled (device not responding)" },
	{ -EOVERFLOW,	"Babble (device sends too much data)" },
	{ -EPROTO,	"Bit-stuff error (bad cable?)" },
	{ -EILSEQ,	"CRC/Timeout (bad cable?)" },
	{ -ETIME,	"Device does not respond to token" },
	{ -ETIMEDOUT,	"Device does not respond to command" },
	{ -1, NULL }
};

/**********************************************************************
 * Memory management
 **********************************************************************/
static void *
rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	memset(mem, 0, size); /* Clear the ram out, no junk to the user */
	adr = (unsigned long) mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return mem;
}

static void
rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long) mem;
	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}

/**********************************************************************
 *
 * Register I/O
 *
 **********************************************************************/

/* Write an OV51x register */
static int
reg_w(struct usb_ov511 *ov, unsigned char reg, unsigned char value)
{
	int rc;

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	mutex_lock(&ov->cbuf_lock);
	ov->cbuf[0] = value;
	rc = usb_control_msg(ov->dev,
			     usb_sndctrlpipe(ov->dev, 0),
			     (ov->bclass == BCL_OV518)?1:2 /* REits affinity changed.
			 * Also make sure that it wasn't scheduled on its rq.
			 */
			if (unlikely(task_rq(task) != rq ||
				     !cpumask_test_cpu(lowest_rq->cpu,
						       &task->cpus_allowed) ||
				     task_running(rq, task) ||
				     !task->se.on_rq)) {

				spin_unlock(&lowest_rq->lock);
				lowest_rq = NULL;
				break;
			}
		}

		/* If this rq is still suitable use it. */
		if (lowest_rq->rt.highest_prio.curr > task->prio)
			break;

		/* try again */
		double_unlock_balance(rq, lowest_rq);
		lowest_rq = NULL;
	}

	return lowest_rq;
}

static inline int has_pushable_tasks(struct rq *rq)
{
	return !plist_head_empty(&rq->rt.pushable_tasks);
}

static struct task_struct *pick_next_pushable_task(struct rq *rq)
{
	struct task_struct *p;

	if (!has_pushable_tasks(rq))
		return NULL;

	p = plist_first_entry(&rq->rt.pushable_tasks,
			      struct task_struct, pushable_tasks);

	BUG_ON(rq->cpu != task_cpu(p));
	BUG_ON(task_current(rq, p));
	BUG_ON(p->rt.nr_cpus_allowed <= 1);

	BUG_ON(!p->se.on_rq);
	BUG_ON(!rt_task(p));

	return p;
}

/*
 * If the current CPU has more than one RT task, see if the non
 * running task can migrate over to a CPU that is running a task
 * of lesser priority.
 */
static int push_rt_task(struct rq *rq)
{
	struct task_struct *next_task;
	struct rq *lowest_rq;

	if (!rq->rt.overloaded)
		return 0;

	next_task = pick_next_pushable_task(rq);
	if (!next_task)
		return 0;

 retry:
	if (unlikely(next_task == rq->curr)) {
		WARN_ON(1);
		return 0;
	}

	/*
	 * It's possible that the next_task slipped in of
	 * higher priority than current. If that's the case
	 * just reschedule current.
	 */
	if (unlikely(next_task->prio < rq->curr->prio)) {
		resched_task(rq->curr);
		return 0;
	}

	/* We might release rq lock */
	get_task_struct(next_task);

	/* find_lock_lowest_rq locks the rq if found */
	lowest_rq = find_lock_lowest_rq(next_task, rq);
	if (!lowest_rq) {
		struct task_struct *task;
		/*
		 * find lock_lowest_rq releases rq->lock
		 * so it is possible that next_task has migrated.
		 *
		 * We need to make sure that the task is still on the same
		 * run-queue and is also still the next task eligible for
		 * pushing.
		 */
		task = pick_next_pushable_task(rq);
		if (task_cpu(next_task) == rq->cpu && task == next_task) {
			/*
			 * If we get here, the task hasnt moved at all, but
			 * it has failed to push.  We will not try again,
			 * since the other cpus will pull from us when they
			 * are ready.
			 */
			dequeue_pushable_task(rq, next_task);
			goto out;
		}

		if (!task)
			/* No more tasks, just exit */
			goto out;

		/*
		 * Something has shifted, try again.
		 */
		put_task_struct(next_task);
		next_task = task;
		goto retry;
	}

	deactivate_task(rq, next_task, 0);
	set_task_cpu(next_task, lowest_rq->cpu);
	activate_task(lowest_rq, next_task, 0);

	resched_task(lowest_rq->curr);

	double_unlock_balance(rq, lowest_rq);

out:
	put_task_struct(next_task);

	return 1;
}

static void push_rt_tasks(struct rq *rq)
{
	/* push_rt_task will return true if it moved an RT */
	while (push_rt_task(rq))
		;
}

static int pull_rt_task(struct rq *this_rq)
{
	int this_cpu = this_rq->cpu, ret = 0, cpu;
	struct task_struct *p;
	struct rq *src_rq;

	if (likely(!rt_overloaded(this_rq)))
		return 0;

	for_each_cpu(cpu, this_rq->rd->rto_mask) {
		if (this_cpu == cpu)
			continue;

		src_rq = cpu_rq(cpu);

		/*
		 * Don't bother taking the src_rq->lock if the next highest
		 * task is known to be lower-priority than our current task.
		 * This may look racy, but if this value is about to go
		 * logically higher, the src_rq will push this task away.
		 * And if its going logically lower, we do not care
		 */
		if (src_rq->rt.highest_prio.next >=
		    this_rq->rt.highest_prio.curr)
			continue;

		/*
		 * We can potentially drop this_rq's lock in
		 * double_lock_balance, and another CPU could
		 * alter this_rq
		 */
		double_lock_balance(this_rq, src_rq);

		/*
		 * Are there still pullable RT tasks?
		 */
		if (src_rq->rt.rt_nr_running <= 1)
			goto skip;

		p = pick_next_highest_task_rt(src_rq, treset: command failed");

	return rc;
}

/**********************************************************************
 *
 * Low-level I2C I/O functions
 *
 **********************************************************************/

/* NOTE: Do not call this function directly!
 * The OV518 I2C I/O procedure is different, hence, this function.
 * This is normally only called from i2c_w(). Note that this function
 * always succeeds regardless of whether the sensor is present and working.
 */
static int
ov518_i2c_write_internal(struct usb_ov511 *ov,
			 unsigned char reg,
			 unsigned char value)
{
	int rc;

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	/* Select camera register */
	rc = reg_w(ov, R51x_I2C_SADDR_3, reg);
	if (rc < 0)
		return rc;

	/* Write "value" to I2C data port of OV511 */
	rc = reg_w(ov, R51x_I2C_DATA, value);
	if (rc < 0)
		return rc;

	/* Initiate 3-byte write cycle */
	rc = reg_w(ov, R518_I2C_CTL, 0x01);
	if (rc < 0)
		return rc;

	return 0;
}

/* NOTE: Do not call this function directly! */
static int
ov511_i2c_write_internal(struct usb_ov511 *ov,
			 unsigned char reg,
			 unsigned char value)
{
	int rc, retries;

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	/* Three byte write cycle */
	for (retries = OV511_I2C_RETRIES; ; ) {
		/* Select camera register */
		rc = reg_w(ov, R51x_I2C_SADDR_3, reg);
		if (rc < 0)
			break;

		/* Write "value" to I2C data port of OV511 */
		rc = reg_w(ov, R51x_I2C_DATA, value);
		if (rc < 0)
			break;

		/* Initiate 3-byte write cycle */
		rc = reg_w(ov, R511_I2C_CTL, 0x01);
		if (rc < 0)
			break;

		/* Retry until idle */
		do {
			rc = reg_r(ov, R511_I2C_CTL);
		} while (rc > 0 && ((rc&1) == 0));
		if (rc < 0)
			break;

		/* Ack? */
		if ((rc&2) == 0) {
			rc = 0;
			break;
		}
#if 0
		/* I2C abort */
		reg_w(ov, R511_I2C_CTL, 0x10);
#endif
		if (--retries < 0) {
			err("i2c write retries exhausted");
			rc = -1;
			break;
		}
	}

	return rc;
}

/* NOTE: Do not call this function directly!
 * The OV518 I2C I/O procedure is different, hence, this function.
 * This is normally only called from i2c_r(). Note that this function
 * always succeeds regardless of whether the sensor is present and working.
 */
static int
ov518_i2c_read_internal(struct usb_ov511 *ov, unsigned char reg)
{
	int rc, value;

	/* Select camera register */
	rc = reg_w(ov, R51x_I2C_SADDR_2, reg);
	if (rc < 0)
		return rc;

	/* Initiate 2-byte write cycle */
	rc = reg_w(ov, R518_I2C_CTL, 0x03);
	if (rc < 0)
		return rc;

	/* Initiate 2-byte read cycle */
	rc = reg_w(ov, R518_I2C_CTL, 0x05);
	if (rc < 0)
		return rc;

	value = reg_r(ov, R51x_I2C_DATA);

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	return value;
}

/* NOTE: Do not call this function directly!
 * returns: negative is error, pos or zero is data */
static int
ov511_i2c_read_internal(struct usb_ov511 *ov, unsigned char reg)
{
	int rc, value, retries;

	/* Two byte write cycle */
	for (retries = OV511_I2C_RETRIES; ; ) {
		/* Select camera register */
		rc = reg_w(ov, R51x_I2C_SADDR_2, reg);
		if (rc < 0)
			return rc;

		/* Initiate 2-byte write cycle */
		rc = reg_w(ov, R511_I2C_CTL, 0x03);
		if (rc < 0)
			return rc;

		/* Retry until idle */
		do {
			rc = reg_r(ov, R511_I2C_CTL);
		} while (rc > 0 && ((rc & 1) == 0));
		if (rc < 0)
			return rc;

		if ((rc&2) == 0) /* Ack? */
			break;

		/* I2C abort */
		reg_w(ov, R511_I2C_CTL, 0x10);

		if (--retries < 0) {
			err("i2c write retries exhausted");
			return -1;
		}
	}

	/* Two byte read cycle */
	for (retries = OV511_I2C_RETRIES; ; ) {
		/* Initiate 2-byte read cycle */
		rc = reg_w(ov, R511_I2C_CTL, 0x05);
		if (rc < 0)
			return rc;

		/* Retry until idle */
		do {
			rc = reg_r(ov, R511_I2C_CTL);
		} while (rc > 0 && ((rc&1) == 0));
		if (rc < 0)
			return rc;

		if ((rc&2) == 0) /* Ack? */
			break;

		/* I2C abort */
		rc = reg_w(ov, R511_I2C_CTL, 0x10);
		if (rc < 0)
			return rc;

		if (--retries < 0) {
			err("i2c read retries exhausted");
			return -1;
		}
	}

	value = reg_r(ov, R51x_I2C_DATA);

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	/* This is needed to make i2c_w() work */
	rc = reg_w(ov, R511_I2C_CTL, 0x05);
	if (rc < 0)
		return rc;

	return value;
}

/* returns: negative is error, pos or zero is data */
static int
i2c_r(struct usb_ov511 *ov, unsigned char reg)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	if (ov->bclass == BCL_OV518)
		rc = ov518_i2c_read_internal(ov, reg);
	else
		rc = ov511_i2c_read_internal(ov, reg);

	mutex_unlock(&ov->i2c_lock);

	return rc;
}

static int
i2c_w(struct usb_ov511 *ov, unsigned char reg, unsigned char value)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	if (ov->bclass == BCL_OV518)
		rc = ov518_i2c_write_internal(ov, reg, value);
	else
		rc = ov511_i2c_write_internal(ov, reg, value);

	mutex_unlock(&ov->i2c_lock);

	return rc;
}

/* Do not call this function directly! */
static int
ov51x_i2c_write_mask_internal(struct usb_ov511 *ov,
			      unsigned char reg,
			      unsigned char value,
			      unsigned char mask)
{
	int rc;
	unsigned char oldval, newval;

	if (mask == 0xff) {
		newval = value;
	} else {
		if (ov->bclass == BCL_OV518)
			rc = ov518_i2c_read_internal(ov, reg);
		else
			rc = ov511_i2c_read_internal(ov, reg);
		if (rc < 0)
			return rc;

		oldval = (unsigned char) rc;
		oldval &= (~mask);		/* Clear the masked bits */
		value &= mask;			/* Enforce mask on value */
		newval = oldval | value;	/* Set the desired bits */
	}

	if (ov->bclass == BCL_OV518)
		return (ov518_i2c_write_internal(ov, reg, newval));
	else
		return (ov511_i2c_write_internal(ov, reg, newval));
}

/* Writes bits at positions specified by mask to an I2C reg. Bits that are in
 * the same position as 1's in "mask" are cleared and set to "value". Bits
 * that are in the same position as 0's in "mask" are preserved, regardless
 * of their respective state in "value".
 */
static int
i2c_w_mask(struct usb_ov511 *ov,
	   unsigned char reg,
	   unsigned char value,
	   unsigned char mask)
{
	int rc;

	mutex_lock(&ov->i2c_lock);
	rc = ov51x_i2c_write_mask_internal(ov, reg, value, mask);
	mutex_unlock(&ov->i2c_lock);

	return rc;
}

/* Set the read and write slave IDs. The "slave" argument is the write slave,
 * and the read slave will be set to (slave + 1). ov->i2c_lock should be held
 * when calling this. This should not be called from outside the i2c I/O
 * functions.
 */
static int
i2c_set_slave_internal(struct usb_ov511 *ov, unsigned char slave)
{
	int rc;

	rc = reg_w(ov, R51x_I2C_W_SID, slave);
	if (rc < 0)
		return rc;

	rc = reg_w(ov, R51x_I2C_R_SID, slave + 1);
	if (rc < 0)
		return rc;

	return 0;
}

/* Write to a specific I2C slave ID and register, using the specified mask */
static int
i2c_w_slave(struct usb_ov511 *ov,
	    unsigned char slave,
	    unsigned char reg,
	    unsigned char value,
	    unsigned char mask)
{
	int rc = 0;

	mutex_lock(&ov->i2c_lock);

	/* Set new slave IDs */
	rc = i2c_set_slave_internal(ov, slave);
	if (rc < 0)
		goto out;

	rc = ov51x_i2c_write_mask_internal(ov, reg, value, mask);

out:
	/* Restore primary IDs */
	if (i2c_set_slave_internal(ov, ov->primary_i2c_slave) < 0)
		err("Couldn't restore primary I2C slave");

	mutex_unlock(&ov->i2c_lock);
	return rc;
}

/* Read from a specific I2C slave ID and register */
static int
i2c_r_slave(struct usb_ov511 *ov,
	    unsigned char slave,
	    unsigned char reg)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	/* Set new slave IDs */
	rc = i2c_set_slave_internal(ov, slave);
	if (rc < 0)
		goto out;

	if (ov->bclass == BCL_OV518)
		rc = ov518_i2c_read_internal(ov, reg);
	else
		rc = ov511_i2c_read_internal(ov, reg);

out:
	/* Restore primary IDs */
	if (i2c_set_slave_internal(ov, ov->primary_i2c_slave) < 0)
		err("Couldn't restore primary I2C slave");

	mutex_unlock(&ov->i2c_lock);
	return rc;
}

/* Sets I2C read and write slave IDs. Returns <0 for error */
static int
ov51x_set_slave_ids(struct usb_ov511 *ov, unsigned char sid)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	rc = i2c_set_slave_internal(ov, sid);
	if (rc < 0)
		goto out;

	// FIXME: Is this actually necessary?
	rc = ov51x_reset(ov, OV511_RESET_NOREGS);
out:
	mutex_unlock(&ov->i2c_lock);
	return rc;
}

static int
write_regvals(struct usb_ov511 *ov, struct ov511_regvals * pRegvals)
{
	int rc;

	while (pRegvals->bus != OV511_DONE_BUS) {
		if (pRegvals->bus == OV511_REG_BUS) {
			if ((rc = reg_w(ov, pRegvals->reg, pRegvals->val)) < 0)
				return rc;
		} else if (pRegvals->bus == OV511_I2C_BUS) {
			if ((rc = i2c_w(ov, pRegvals->reg, pRegvals->val)) < 0)
				return rc;
		} else {
			err("Bad regval array");
			return -1;
		}
		pRegvals++;
	}
	return 0;
}

#ifdef OV511_DEBUG
static void
dump_i2c_range(struct usb_ov511 *ov, int reg1, int regn)
{
	int i, rc;

	for (i = reg1; i <= regn; i++) {
		rc =