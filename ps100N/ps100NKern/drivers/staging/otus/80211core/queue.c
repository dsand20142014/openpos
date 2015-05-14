/*
 * Copyright (c) 2006, 2007, 2008 QLogic Corporation. All rights reserved.
 * Copyright (c) 2005, 2006 PathScale, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/io.h>

#include "ipath_verbs.h"
#include "ipath_kernel.h"

/* cut down ridiculously long IB macro names */
#define OP(x) IB_OPCODE_RC_##x

static u32 restart_sge(struct ipath_sge_state *ss, struct ipath_swqe *wqe,
		       u32 psn, u32 pmtu)
{
	u32 len;

	len = ((psn - wqe->psn) & IPATH_PSN_MASK) * pmtu;
	ss->sge = wqe->sg_list[0];
	ss->sg_list = wqe->sg_list + 1;
	ss->num_sge = wqe->wr.num_sge;
	ipath_skip_sge(ss, len);
	return wqe->length - len;
}

/**
 * ipath_init_restart- initialize the qp->s_sge after a restart
 * @qp: the QP who's SGE we're restarting
 * @wqe: the work queue to initialize the QP's SGE from
 *
 * The QP s_lock should be held and interrupts disabled.
 */
static void ipath_init_restart(struct ipath_qp *qp, struct ipath_swqe *wqe)
{
	struct ipath_ibdev *dev;

	qp->s_len = restart_sge(&qp->s_sge, wqe, qp->s_psn,
				ib_mtu_enum_to_int(qp->path_mtu));
	dev = to_idev(qp->ibqp.device);
	spin_lock(&dev->pending_lock);
	if (list_empty(&qp->timerwait))
		list_add_tail(&qp->timerwait,
			      &dev->pending[dev->pending_index]);
	spin_unlock(&dev->pending_lock);
}

/**
 * ipath_make_rc_ack - construct a response packet (ACK, NAK, or RDMA read)
 * @qp: a pointer to the QP
 * @ohdr: a pointer to the IB header being constructed
 * @pmtu: the path MTU
 *
 * Return 1 if constructed; otherwise, return 0.
 * Note that we are in the responder's side of the QP context.
 * Note the QP s_lock must be held.
 */
static int ipath_make_rc_ack(struct ipath_ibdev *dev, struct ipath_qp *qp,
			     struct ipath_other_headers *ohdr, u32 pmtu)
{
	struct ipath_ack_entry *e;
	u32 hwords;
	u32 len;
	u32 bth0;
	u32 bth2;

	/* Don't send an ACK if we aren't supposed to. */
	if (!(ib_ipath_state_ops[qp->state] & IPATH_PROCESS_RECV_OK))
		goto bail;

	/* header size in 32-bit words LRH+BTH = (8+12)/4. */
	hwords = 5;

	switch (qp->s_ack_state) {
	case OP(RDMA_READ_RESPONSE_LAST):
	case OP(RDMA_READ_RESPONSE_ONLY):
	case OP(ATOMIC_ACKNOWLEDGE):
		/*
		 * We can increment the tail pointer now that the last
		 * response has been sent instead of only being
		 * constructed.
		 */
		if (++qp->s_tail_ack_queue > IPATH_MAX_RDMA_ATOMIC)
			qp->s_tail_ack_queue = 0;
		/* FALLTHROUGH */
	case OP(SEND_ONLY):
	case OP(ACKNOWLEDGE):
		/* Check for no next entry in the queue. */
		if (qp->r_head_ack_queue == qp->s_tail_ack_queue) {
			if (qp->s_flags & IPATH_S_ACK_PENDING)
				goto normal;
			qp->s_ack_state = OP(ACKNOWLEDGE);
			goto bail;
		}

		e = &qp->s_ack_queue[qp->s_tail_ack_queue];
		if (e->opcode == OP(RDMA_READ_REQUEST)) {
			/* Copy SGE state      q->cell[index].buf = q->cell[next].buf;
                    q->cell[index].tick = q->cell[next].tick;

                    if ((*mb == 0) && (zfCompareDstwithBuf(dev,
                            q->cell[next].buf, addr) == 0))
                    {
                        *mb = 1;
                    }

                    index = next;
                }
                q->tail = (q->tail-1) & q->sizeMask;

                zmw_leave_critical_section(dev);
                return retBuf;
            }
            index = (index + 1) & q->sizeMask;
        } //if (index != q->tail)
        else
        {
            break;
        }
    }

    zmw_leave_critical_section(dev);

    return retBuf;

}

void zfQueueFlush(zdev_t* dev, struct zsQueue* q)
{
    zbuf_t* buf;

    while ((buf = zfQueueGet(dev, q)) != NULL)
    {
        zfwBufFree(dev, buf, 0);
    }

    return;
}

void zfQueueAge(zdev_t* dev, struct zsQueue* q, u32_t tick, u32_t msAge)
{
    zbuf_t* buf;
    u32_t   buftick;
    zmw_declare_for_critical_section();

    while (1)
    {
        buf = NULL;
        zmw_enter_critical_section(dev);

        if (q->head != q->tail)
        {
            buftick = q->cell[q->head].tick;
            if (((tick - buftick)*ZM_MS_PER_TICK) > msAge)
            {
                buf = q->cell[q->head].buf;
                q->head = (q->head+1) & q->sizeMask;
            }
        }

        zmw_leave_critical_section(dev);

        if (buf != NULL)
        {
            zm_msg0_mm(ZM_LV_0, "Age frame in queue!");
            zfwBufFree(dev, buf, 0);
        }
        else
        {
            break;
        }
    }
    return;
}


u8_t zfQueueRemovewithIndex(zdev_t* dev, struct zsQueue* q, u16_t index, u8_t* addr)
{
    u16_t next;
    u8_t mb = 0;

    //trace the whole queue to calculate more bit
    while ((next =((index+1)&q->sizeMask)) != q->tail)
    {
        q->cell[index].buf = q->cell[next].buf;
        q->cell[index].tick = q->cell[next].tick;

        if ((mb == 0) && (zfCompareDstwithBuf(dev,
                q->cell[next].buf, addr) == 0))
        {
            mb = 1;
        }

        index = next;
    }
    q->tail = (q->tail-1) & q->sizeMask;

    return mb;

}

void zfQueueGenerateUapsdTim(zdev_t* dev, struct zsQueue* q,
        u8_t* uniBitMap, u16_t* highestByte)
{
    zbuf_t* psBuf;
    u8_t dst[6];
    u16_t id, aid, index, i;
    u16_t bitPosition;
    u16_t bytePosition;
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    index = q->head;

    while (index != q->tail)
    {
        psBuf = q->cell[index].buf;
        for (i=0; i<6; i++)
        {
            dst[i] = zmw_buf_readb(dev, psBuf, i);
        }
        /* TODO : use u8_t* fot MAC address */
        if (((id = zfApFindSta(dev, (u16_t*)dst)) != 0xffff)
                && (wd->ap.staTable[id].psMode != 0))
        {
            /* Calculate PVB only when all AC are delivery-enabled */
            if ((wd->ap.staTable[id].qosInfo & 0xf) == 0xf)
            {
                aid = id + 1;
                bitPosition = (1 << (aid & 0x7));
                bytePosition = (aid >> 3);
                uniBitMap[bytePosition] |= bitPosition;

                if (bytePosition>*highestByte)
                {
                    *highestByte = bytePosition;
                }
            }
            index = (index+1) & q->sizeMask;
        }
        else
        {
            /* Free garbage UAPSD frame */
            zfQueueRemovewithIndex(dev, q, index, dst);
            zfwBufFree(dev, psBuf, 0);
        }
    }
    zmw_leave_critical_section(dev);

    return;
}
