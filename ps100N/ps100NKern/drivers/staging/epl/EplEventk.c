 keep track of the current SWQE so that
		 * we don't reset the "furthest progress" state
		 * if we need to back up.
		 */
		newreq = 0;
		if (qp->s_cur == qp->s_tail) {
			/* Check if send work queue is empty. */
			if (qp->s_tail == qp->s_head)
				goto bail;
			/*
			 * If a fence is requested, wait for previous
			 * RDMA read and atomic operations to finish.
			 */
			if ((wqe->wr.send_flags & IB_SEND_FENCE) &&
			    qp->s_num_rd_atomic) {
				qp->s_flags |= IPATH_S_FENCE_PENDING;
				goto bail;
			}
			wqe->psn = qp->s_next_psn;
			newreq = 1;
		}
		/*
		 * Note that we have to be careful not to modify the
		 * original work request since we may need to resend
		 * it.
		 */
		len = wqe->length;
		ss = &qp->s_sge;
		bth2 = 0;
		switch (wqe->wr.opcode) {
		case IB_WR_SEND:
		case IB_WR_SEND_WITH_IMM:
			/* If no credit, return. */
			if (qp->s_lsn != (u32) -1 &&
			    ipath_cmp24(wqe->ssn, qp->s_lsn + 1) > 0) {
				qp->s_flags |= IPATH_S_WAIT_SSN_CREDIT;
				goto bail;
			}
			wqe->lpsn = wqe->psn;
			if (len > pmtu) {
				wqe->lpsn += (len - 1) / pmtu;
				qp->s_state = OP(SEND_FIRST);
				len = pmtu;
				break;
			}
			if (wqe->wr.opcode == IB_WR_SEND)
				qp->s_state = OP(SEND_ONLY);
			else {
				qp->s_state = OP(SEND_ONLY_WITH_IMMEDIATE);
				/* Immediate data comes after the BTH */
				ohdr->u.imm_data = wqe->wr.ex.imm_data;
				hwords += 1;
			}
			if (wqe->wr.send_flags & IB_SEND_SOLICITED)
				bth0 |= 1 << 23;
			bth2 = 1 << 31;	/* Request ACK. */
			if (++qp->s_cur == qp->s_size)
				qp->s_cur = 0;
			break;

		case IB_WR_RDMA_WRITE:
			if (newreq && qp->s_lsn != (u32) -1)
				qp->s_lsn++;
			/* FALLTHROUGH */
		case IB_WR_RDMA_WRITE_WITH_IMM:
			/* If no credit, return. */
			if (qp->s_lsn != (u32) -1 &&
			    ipath_cmp24(wqe->ssn, qp->s_lsn + 1) > 0) {
				qp->s_flags |= IPATH_S_WAIT_SSN_CREDIT;
				goto bail;
			}
			ohdr->u.rc.reth.vaddr =
				cpu_to_be64(wqe->wr.wr.rdma.remote_addr);
			ohdr->u.rc.reth.rkey =
				cpu_to_be32(wqe->wr.wr.rdma.rkey);
			ohdr->u.rc.reth.length = cpu_to_be32(len);
			hwords += sizeof(struct ib_reth) / sizeof(u32);
			wqe->lpsn = wqe->psn;
			if (len > pmtu) {
				wqe->lpsn += (len - 1) / pmtu;
				qp->s_state = OP(RDMA_WRITE_FIRST);
				len = pmtu;
				break;
			}
			if (wqe->wr.opcode == IB_WR_RDMA_WRITE)
				qp->s_state = OP(RDMA_WRITE_ONLY);
			else {
				qp->s_state =
					OP(RDMA_WRITE_ONLY_WITH_IMMEDIATE);
				/* Immediate data comes after RETH */
				ohdr->u.rc.imm_data = wqe->wr.ex.imm_data;
				hwords += 1;
				if (wqe->wr.send_flags & IB_SEND_SOLICITED)
					bth0 |= 1 << 23;
			}
			bth2 = 1 << 31;	/* Request ACK. */
			if (++qp->s_cur == qp->s_size)
				qp->s_cur = 0;
			break;

		case IB_WR_RDMA_READ:
			/*
			 * Don't allow more operations to be started
			 * than the QP limits allow.
			 */
			if (newreq) {
				if (qp->s_num_rd_atomic >=
				    qp->s_max_rd_atomic) {
					qp->s_flags |= IPATH_S_RDMAR_PENDING;
					goto bail;
				}
				qp->s_num_rd_atomic++;
				if (qp->s_lsn != (u32) -1)
					qp->s_lsn++;
				/*
				 * Adjust s_next_psn to count the
				 * expected number of responses.
				 */
				if (len > pmtu)
					qp->s_next_psn += (len - 1) / pmtu;
				wqe->lpsn = qp->s_next_psn++;
			}
			ohdr->u.rc.reth.vaddr =
				cpu_to_be64(wqe->wr.wr.rdma.remote_addr);
			ohdr->u.rc.reth.rkey =
				cpu_to_be32(wqe->wr.wr.rdma.rkey);
			ohdr->u.rc.reth.length = cpu_to_be32(len);
			qp->s_state = OP(RDMA_READ_REQUEST);
			hwords += sizeof(ohdr->u.rc.reth) / sizeof(u32);
			ss = NULL;
			len = 0;
			if (++qp->s_cur == qp->s_size)
				qp->s_cur = 0;
			break;

		case IB_WR_ATOMIC_CMP_AND_SWP:
		case IB_WR_ATOMIC_FETCH_AND_ADD:
			/*
			 * Don't allow more operations to be started
			 * than the QP limits allow.
			 */
			if (newreq) {
				if (qp->s_num_rd_atomic >=
				    qp->s_max_rd_atomic) {
					qp->s_flags |= IPATH_S_RDMAR_PENDING;
					goto bail;
				}
				qp->s_num_rd_atomic++;
				if (qp->s_lsn != (u32) -1)
					qp->s_lsn++;
				wqe->lpsn = wqe->psn;
			}
			if (wqe->wr.opcode == IB_WR_ATOMIC_CMP_AND_SWP) {
				qp->s_state = OP(COMPARE_SWAP);
				ohdr->u.atomic_eth.swap_data = cpu_to_be64(
					wqe->wr.wr.atomic.swap);
				ohdr->u.atomic_eth.compare_data = cpu_to_be64(
					wqe->wr.wr.atomic.compare_add);
			} else {
				qp->s_state = OP(FETCH_ADD);
				ohdr->u.atomic_eth.swap_data = cpu_to_be64(
					wqe->wr.wr.atomic.compare_add);
				ohdr->u.atomic_eth.compare_data = 0;
			}
			ohdr->u.atomic_eth.vaddr[0] = cpu_to_be32(
				wqe->wr.wr.atomic.remote_addr >> 32);
			ohdr->u.atomic_eth.vaddr[1] = cpu_to_be32(
				wqe->wr.wr.atomic.remote_addr);
			ohdr->u.atomic_eth.rkey = cpu_to_be32(
				wqe->wr.wr.atomic.rkey);
			hwords += sizeof(struct ib_atomic_eth) / sizeof(u32);
			ss = NULL;
			len = 0;
			if (++qp->s_cur == qp->s_size)
				qp->s_cur = 0;
			break;

		default:
			goto bail;
		}
		qp->s_sge.sge = wqe->sg_list[0];
		qp->s_sge.sg_list = wqe->sg_list + 1;
		qp->s_sge.num_sge = wqe->wr.num_sge;
		qp->s_len = wqe->length;
		if (newreq) {
			qp->s_tail++;
			if (qp->s_tail >= qp->s_size)
				qp->s_tail = 0;
		}
		bth2 |= qp->s_psn & IPATH_PSN_MASK;
		if (wqe->wr.opcode == IB_WR_RDMA_READ)
			qp->s_psn = wqe->lpsn + 1;
		else {
			qp->s_psn++;
			if (ipath_cmp24(qp->s_psn, qp->s_next_psn) > 0)
				qp->s_next_psn = qp->s_psn;
		}
		/*
		 * Put the QP on the pending list so lost ACKs will cause
		 * a retry.  More than one request can be pending so the
		 * QP may already be on the dev->pending list.
		 */
		spin_lock(&dev->pending_lock);
		if (list_empty(&qp->timerwait))
			list_add_tail(&qp->timerwait,
				      &dev->pending[dev->pending_index]);
		spin_unlock(&dev->pending_lock);
		break;

	case OP(RDMA_READ_RESPONSE_FIRST):
		/*
		 * This case can only happen if a send is restarted.
		 * See ipath_restart_rc().
		 */
		ipath_init_restart(qp, wqe);
		/* FALLTHROUGH */
	case OP(SEND_FIRST):
		qp->s_state = OP(SEND_MIDDLE);
		/* FALLTHROUGH */
	case OP(SEND_MIDDLE):
		bth2 = qp->s_psn++ & IPATH_PSN_MASK;
		if (ipath_cmp24(qp->s_psn, qp->s_next_psn) > 0)
			qp->s_next_psn = qp->s_psn;
		ss = &qp->s_sge;
		len = qp->s_len;
		if (len > pmtu) {
			len = pmtu;
			break;
		}
		if (wqe->wr.opcode == IB_WR_SEND)
			qp->s_state = OP(SEND_LAST);
		else {
			qp->s_state = OP(SEND_LAST_WITH_IMMEDIATE);
			/* Immediate data comes after the BTH */
			ohdr->u.imm_data = wqe->wr.ex.imm_data;
			hwords += 1;
		}
		if (wqe->wr.send_flags & IB_SEND_SOLICITED)
			bth0 |= 1 << 23;
		bth2 |= 1 << 31;	/* Request ACK. */
		qp->s_cur++;
		if (qp->s_cur >= qp->s_size)
			qp->s_cur = 0;
		break;

	case OP(RDMA_READ_RESPONSE_LAST):
		/*
		 * This case can only happen if a RDMA write is restarted.
		 * See ipath_restart_rc().
		 */
		ipath_init_restart(qp, wqe);
		/* FALLTHROUGH */
	case OP(RDMA_WRITE_FIRST):
		qp->s_state = OP(RDMA_WRITE_MIDDLE);
		/* FALLTHROUGH */
	case OP(RDMA_WRITE_MIDDLE):
		bth2 = qp->s_psn++ & IPATH_PSN_MASK;
		if (ipath_cmp24(qp->s_psn, qp->s_next_psn) > 0)
			qp->s_next_psn = qp->s_psn;
		ss = &qp->s_sge;
		len = qp->s_len;
		if (len > pmtu) {
			len = pmtu;
			break;
		}
		if (wqe->wr.opcode == IB_WR_RDMA_WRITE)
			qp->s_state = OP(RDMA_WRITE_LAST);
		else {
			qp->s_state = OP(RDMA_WRITE_LAST_WITH_IMMEDIATE);
			/* Immediate data comes after the BTH */
			ohdr->u.imm_data = wqe->wr.ex.imm_data;
			hwords += 1;
			if (wqe->wr.send_flags & IB_SEND_SOLICITED)
				bth0 |= 1 << 23;
		}
		bth2 |= 1 << 31;	/* Request ACK. */
		qp->s_cur++;
		if (qp->s_cur >= qp->s_size)
			qp->s_cur = 0;
		break;

	case OP(RDMA_READ_RESPONSE_MIDDLE):
		/*
		 * This case can only happen if a RDMA read is restarted.
		 * See ipath_restart_rc().
		 */
		ipath_init_restart(qp, wqe);
		len = ((qp->s_psn - wqe->psn) & IPATH_PSN_MASK) * pmtu;
		ohdr->u.rc.reth.vaddr =
			cpu_to_be64(wqe->wr.wr.rdma.remote_addr + len);
		ohdr->u.rc.reth.rkey =
			cpu_to_be32(wqe->wr.wr.rdma.rkey);
		ohdr->u.rc.reth.length = cpu_to_be32(qp->s_len);
		qp->s_state = OP(RDMA_READ_REQUEST);
		hwords += sizeof(ohdr->u.rc.reth) / sizeof(u32);
		bth2 = qp->s_psn & IPATH_PSN_MASK;
		qp->s_psn = wqe->lpsn + 1;
		ss = NULL;
		len = 0;
		qp->s_cur++;
		if (qp->s_cur == qp->s_size)
			qp->s_cur = 0;
		break;
	}
	if (ipath_cmp24(qp->s_psn, qp->s_last_psn + IPATH_PSN_CREDIT - 1) >= 0)
		bth2 |= 1 << 31;	/* Request ACK. */
	qp->s_len -= len;
	qp->s_hdrwords = hwords;
	qp->s_cur_sge = ss;
	qp->s_cur_size = len;
	ipath_make_ruc_header(dev, qp, ohdr, bth0 | (qp->s_state << 24), bth2);
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
 * send_rc_ack - Construct an ACK packet and send it
 * @qp: a pointer to the QP
 *
 * This is called from ipath_rc_rcv() and only uses the receive
 * side QP state.
 * Note that RDMA reads and atomics are handled in the
 * send side QP state and tasklet.
 */
static void send_rc_ack(struct ipath_qp *qp)
{
	struct ipath_ibdev *dev = to_idev(qp->ibqp.device);
	struct ipath_devdata *dd;
	u16 lrh0;
	u32 bth0;
	u32 hwords;
	u32 __iomem *piobuf;
	struct ipath_ib_header hdr;
	struct ipath_other_headers *ohdr;
	unsigned long flags;

	spin_lock_irqsave(&qp->s_lock, flags);

	/* Don't send ACK or NAK if a RDMA read or atomic is pending. */
	if (qp->r_head_ack_queue != qp->s_tail_ack_queue ||
	    (qp->s_flags & IPATH_S_ACK_PENDING) ||
	    qp->s_ack_state != OP(ACKNOWLEDGE))
		goto queue_ack;

	spin_unlock_irqrestore(&qp->s_lock, flags);

	/* Don't try to send ACKs if the link isn't ACTIVE */
	dd = dev->dd;
	if (!(dd->ipath_flags & IPATH_LINKACTIVE))
		goto done;

	piobuf = ipath_getpiobuf(dd, 0, NULL);
	if (!piobuf) {
		/*
		 * We are out of PIO buffers at the moment.
		 * Pass responsibility for sending the ACK to the
		 * send tasklet so that when a PIO buffer becomes
		 * available, the ACK is sent ahead of other outgoing
		 * packets.
		 */
		spin_lock_irqsave(&qp->s_lock, flags);
		goto queue_ack;
	}

	/* Construct the header. */
	ohdr = &hdr.u.oth;
	lrh0 = IPATH_LRH_BTH;
	/* header size in 32-bit words LRH+BTH+AETH = (8+12+4)/4. */
	hwords = 6;
	if (unlikely(qp->remote_ah_attr.ah_flags & IB_AH_GRH)) {
		hwords += ipath_make_grh(dev, &hdr.u.l.grh,
					 &qp->remote_ah_attr.grh,
					 hwords, 0);
		ohdr = &hdr.u.l.oth;
		lrh0 = IPATH_LRH_GRH;
	}
	/* read pkey_index w/o lock (its atomic) */
	bth0 = ipath_get_pkey(dd, qp->s_pkey_index) |
		(OP(ACKNOWLEDGE) << 24) | (1 << 22);
	if (qp->r_nak_state)
		ohdr->u.aeth = cpu_to_be32((qp->r_msn & IPATH_MSN_MASK) |
					    (qp->r_nak_state <<
					     IPATH_AETH_CREDIT_SHIFT));
	else
		ohdr->u.aeth = ipath_compute_aeth(qp);
	lrh0 |= qp->remote_ah_attr.sl << 4;
	hdr.lrh[0] = cpu_to_be16(lrh0);
	hdr.lrh[1] = cpu_to_be16(qp->remote_ah_attr.dlid);
	hdr.lrh[2] = cpu_to_be16(hwords + SIZE_OF_CRC);
	hdr.lrh[3] = cpu_to_be16(dd->ipath_lid |
				 qp->remote_ah_attr.src_path_bits);
	ohdr->bth[0] = cpu_to_be32(bth0);
	ohdr->bth[1] = cpu_to_be32(qp->remote_qpn);
	ohdr->bth[2] = cpu_to_be32(qp->r_ack_psn & IPATH_PSN_MASK);

	writeq(hwords + 1, piobuf);

	if (dd->ipath_flags & IPATH_PIO_FLUSH_WC) {
		u32 *hdrp = (u32 *) &hdr;

		ipath_flush_wc();
		__iowrite32_copy(piobuf + 2, hdrp, hwords - 1);
		ipath_flush_wc();
		__raw_writel(hdrp[hwords - 1], piobuf + hwords + 1);
	} else
		__iowrite32_copy(piobuf + 2, (u32 *) &hdr, hwords);

	ipath_flush_wc();

	dev->n_unicast_xmit++;
	goto done;

queue_ack:
	if (ib_ipath_state_ops[qp->state] & IPATH_PROCESS_RECV_OK) {
		dev->n_rc_qacks++;
		qp->s_flags |= IPATH_S_ACK_PENDING;
		qp->s_nak_state = qp->r_nak_state;
		qp->s_ack_psn = qp->r_ack_psn;

		/* Schedule the send tasklet. */
		ipath_schedule_send(qp);
	}
	spin_unlock_irqrestore(&qp->s_lock, flags);
done:
	return;
}

/**
 * reset_psn - reset the QP state to send starting from PSN
 * @qp: the QP
 * @psn: the packet sequence number to restart at
 *
 * This is called from ipath_rc_rcv() to process an incoming RC ACK
 * for the given QP.
 * Called at interrupt level with the QP s_lock held.
 */
static void reset_psn(struct ipath_qp *qp, u32 psn)
{
	u32 n = qp->s_last;
	struct ipath_swqe *wqe = get_swqe_ptr(qp, n);
	u32 opcode;

	qp->s_cur = n;

	/*
	 * If we are starting the request from the beginning,
	 * let the normal send code handle initialization.
	 */
	if (ipath_cmp24(psn, wqe->psn) <= 0) {
		qp->s_state = OP(SEND_LAST);
		goto done;
	}

	/* Find the work request opcode corresponding to the given PSN. */
	opcode = wqe->wr.opcode;
	for (;;) {
		int diff;

		if (++n == qp->s_size)
			n = 0;
		if (n == qp->s_tail)
			break;
		wqe = get_swqe_ptr(qp, n);
		diff = ipath_cmp24(psn, wqe->psn);
		if (diff < 0)
			break;
		qp->s_cur = n;
		/*
		 * If we are starting the request from the beginning,
		 * let the normal send code handle initialization.
		 */
		if (diff == 0) {
			qp->s_state = OP(SEND_LAST);
			goto done;
		}
		opcode = wqe->wr.opcode;
	}

	/*
	 * Set the state to restart in the middle of a request.
	 * Don't change the s_sge, s_cur_sge, or s_cur_size.
	 * See ipath_make_rc_req().
	 */
	switch (opcode) {
	case IB_WR_SEND:
	case IB_WR_SEND_WITH_IMM:
		qp->s_state = OP(RDMA_READ_RESPONSE_FIRST);
		break;

	case IB_WR_RDMA_WRITE:
	case IB_WR_RDMA_WRITE_WITH_IMM:
		qp->s_state = OP(RDMA_READ_RESPONSE_LAST);
		break;

	case IB_WR_RDMA_READ:
		qp->s_state = OP(RDMA_READ_RESPONSE_MIDDLE);
		break;

	default:
		/*
		 * This case shouldn't happen since its only
		 * one PSN per req.
		 */
		qp->s_state = OP(SEND_LAST);
	}
done:
	qp->s_psn = psn;
}

/**
 * ipath_restart_rc - back up requester to resend the last un-ACKed request
 * @qp: the QP to restart
 * @psn: packet sequence number for the request
 * @wc: the work completion request
 *
 * The QP s_lock should be held and interrupts disabled.
 */
void ipath_restart_rc(struct ipath_qp *qp, u32 psn)
{
	struct ipath_swqe *wqe = get_swqe_ptr(qp, qp->s_last);
	struct ipath_ibdev *dev;

	if (qp->s_retry == 0) {
		ipath_send_complete(qp, wqe, IB_WC_RETRY_EXC_ERR);
		ipath_error_qp(qp, IB_WC_WR_FLUSH_ERR);
		goto bail;
	}
	qp->s_retry--;

	/*
	 * Remove the QP from the timeout queue.
	 * Note: it may already have been removed by ipath_ib_timer().
	 */
	dev = to_idev(qp->ibqp.device);
	spin_lock(&dev->pending_lock);
	if (!list_empty(&qp->timerwait))
		list_del_init(&qp->timerwait);
	if (!list_empty(&qp->piowait))
		list_del_init(&qp->piowait);
	spin_unlock(&dev->pending_lock);

	if (wqe->wr.opcode == IB_WR_RDMA_READ)
		dev->n_rc_resends++;
	else
		dev->n_rc_resends += (qp->s_psn - psn) & IPATH_PSN_MASK;

	reset_psn(qp, psn);
	ipath_schedule_send(qp);

bail:
	return;
}

static inline void update_last_psn(struct ipath_qp *qp, u32 psn)
{
	qp->s_last_psn = psn;
}

/**
 * do_rc_ack - process an incoming RC ACK
 * @qp: the QP the ACK came in on
 * @psn: the packet sequence number of the ACK
 * @opcode: the opcode of the request that resulted in the ACK
 *
 * This is called from ipath_rc_rcv_resp() to process an incoming RC ACK
 * for the given QP.
 * Called at interrupt level with the QP s_lock held and interrupts disabled.
 * Returns 1 if OK, 0 if current operation should be aborted (NAK).
 */
static int do_rc_ack(struct ipath_qp *qp, u32 aeth, u32 psn, int opcode,
		     u64 val)
{
	struct ipath_ibdev *dev = to_idev(qp->ibqp.device);
	struct ib_wc wc;
	enum ib_wc_status status;
	struct ipath_swqe *wqe;
	int ret = 0;
	u32 ack_psn;
	int diff;

	/*
	 * Remove the QP from the timeout queue (or RNR timeout queue).
	 * If ipath_ib_timer() has already removed it,
	 * it's OK since we hold the QP s_lock and ipath_restart_rc()
	 * just won't find anything to restart if we ACK everything.
	 */
	spin_lock(&dev->pending_lock);
	if (!list_empty(&qp->timerwait))
		list_del_init(&qp->timerwait);
	spin_unlock(&dev->pending_lock);

	/*
	 * Note that NAKs implicitly ACK outstanding SEND and RDMA write
	 * requests and implicitly NAK RDMA read and atomic requests issued
	 * before the NAK'ed request.  The MSN won't include the NAK'ed
	 * request but will include an ACK'ed request(s).
	 */
	ack_psn = psn;
	if (aeth >> 29)
		ack_psn--;
	wqe = get_swqe_ptr(qp, qp->s_last);

	/*
	 * The MSN might be for a later WQE than the PSN indicates so
	 * only complete WQEs that the PSN finishes.
	 */
	while ((diff = ipath_cmp24(ack_psn, wqe->lpsn)) >= 0) {
		/*
		 * RDMA_READ_RESPONSE_ONLY is a special case since
		 * we want to generate completion events for everything
		 * before the RDMA read, copy the data, then generate
		 * the completion for the read.
		 */
		if (wqe->wr.opcode == IB_WR_RDMA_READ &&
		    opcode == OP(RDMA_READ_RESPONSE_ONLY) &&
		    diff == 0) {
			ret = 1;
			goto bail;
		}
		/*
		 * If this request is a RDMA read or atomic, and the ACK is
		 * for a later operation, this ACK NAKs the RDMA read or
		 * atomic.  In other words, only a RDMA_READ_LAST or ONLY
		 * can ACK a RDMA read and likewise for atomic ops.  Note
		 * that the NAK case can only happen if relaxed ordering is
		 * used and requests are sent after an RDMA read or atomic
		 * is sent but before the response is received.
		 */
		if ((wqe->wr.opcode == IB_WR_RDMA_READ &&
		     (opcode != OP(RDMA_READ_RESPONSE_LAST) || diff != 0)) ||
		    ((wqe->wr.opcode == IB_WR_ATOMIC_CMP_AND_SWP ||
		      wqe->wr.opcode == IB_WR_ATOMIC_FETCH_AND_ADD) &&
		     (opcode != OP(ATOMIC_ACKNOWLEDGE) || diff != 0))) {
			/*
			 * The last valid PSN seen is the previous
			 * request's.
			 */
			update_last_psn(qp, wqe->psn - 1);
			/* Retry this request. */
			ipath_restart_rc(qp, wqe->psn);
			/*
			 * No need to process the ACK/NAK since we are
			 * restarting an earlier request.
			 */
			goto bail;
		}
		if (wqe->wr.opcode == IB_WR_ATOMIC_CMP_AND_SWP ||
		    wqe->wr.opcode == IB_WR_ATOMIC_FETCH_AND_ADD)
			*(u64 *) wqe->sg_list[0].vaddr = val;
		if (qp->s_num_rd_atomic &&
		    (wqe->wr.opcode == IB_WR_RDMA_READ ||
		     wqe->wr.opcode == IB_WR_ATOMIC_CMP_AND_SWP ||
		     wqe->wr.opcode == IB_WR_ATOMIC_FETCH_AND_ADD)) {
			qp->s_num_rd_atomic--;
			/* Restart sending task if fence is complete */
			if (((qp->s_flags & IPATH_S_FENCE_PENDING) &&
			     !qp->s_num_rd_atomic) ||
			    qp->s_flags & IPATH_S_RDMAR_PENDING)
				ipath_schedule_send(qp);
		}
		/* Post a send completion queue entry if requested. */
		if (!(qp->s_flags & IPATH_S_SIGNAL_REQ_WR) ||
		    (wqe->wr.send_flags & IB_SEND_SIGNALED)) {
			memset(&wc, 0, sizeof wc);
			wc.wr_id = wqe->wr.wr_id;
			wc.status = IB_WC_SUCCESS;
			wc.opcode = ib_ipath_wc_opcode[wqe->wr.opcode];
			wc.byte_len = wqe->length;
			wc.qp = &qp->ibqp;
			wc.src_qp = qp->remote_qpn;
			wc.slid = qp->remote_ah_attr.dlid;
			wc.sl = qp->remote_ah_attr.sl;
			ipath_cq_enter(to_icq(qp->ibqp.send_cq), &wc, 0);
		}
		qp->s_retry = qp->s_retry_cnt;
		/*
		 * If we are completing a request which is in the process of
		 * being resent, we can stop resending it since we know the
		 * responder has already seen it.
		 */
		if (qp->s_last == qp->s_cur) {
			if (++qp->s_cur >= qp->s_size)
				qp->s_cur = 0;
			qp->s_last = qp->s_cur;
			if (qp->s_last == qp->s_tail)
				break;
			wqe = get_swqe_ptr(qp, qp->s_cur);
			qp->s_state = OP(SEND_LAST);
			qp->s_psn = wqe->psn;
		} else {
			if (++qp->s_last >= qp->s_size)
				qp->s_last = 0;
			if (qp->state == IB_QPS_SQD && qp->s_last == qp->s_cur)
				qp->s_draining = 0;
			if (qp->s_last == qp->s_tail)
				break;
			wqe = get_swqe_ptr(qp, qp->s_last);
		}
	}

	switch (aeth >> 29) {
	case 0:		/* ACK */
		dev->n_rc_acks++;
		/* If this is a partial ACK, reset the retransmit timer. */
		if (qp->s_last != qp->s_tail) {
			spin_lock(&dev->pending_lock);
			if (list_empty(&qp->timerwait))
				list_add_tail(&qp->timerwait,
					&dev->pending[dev->pending_index]);
			spin_unlock(&dev->pending_lock);
			/*
			 * If we get a partial ACK for a resent operation,
			 * we can stop resending the earlier packets and
			 * continue with the next packet the receiver wants.
			 */
			if (ipath_cmp24(qp->s_psn, psn) <= 0) {
				reset_psn(qp, psn + 1);
				ipath_schedule_send(qp);
			}
		} else if (ipath_cmp24(qp->s_psn, psn) <= 0) {
			qp->s_state = OP(SEND_LAST);
			qp->s_psn = psn + 1;
		}
		ipath_get_credit(qp, aeth);
		qp->s_rnr_retry = qp->s_rnr_retry_cnt;
		qp->s_retry = qp->s_retry_cnt;
		update_last_psn(qp, psn);
		ret = 1;
		goto bail;

	case 1:		/* RNR NAK */
		dev->n_rnr_naks++;
		if (qp->s_last == qp->s_tail)
			goto bail;
		if (qp->s_rnr_retry == 0) {
			status = IB_WC_RNR_RETRY_EXC_ERR;
			goto class_b;
		}
		if (qp->s_rnr_retry_cnt < 7)
			qp->s_rnr_retry--;

		/* The last valid PSN is the previous PSN. */
		update_last_psn(qp, psn - 1);

		if (wqe->wr.opcode == IB_WR_RDMA_READ)
			dev->n_rc_resends++;
		else
			dev->n_rc_resends +=
				(qp->s_psn - psn) & IPATH_PSN_MASK;

		reset_psn(qp, psn);

		qp->s_rnr_timeout =
			ib_ipath_rnr_table[(aeth >> IPATH_AETH_CREDIT_SHIFT) &
					   IPATH_AETH_CREDIT_MASK];
		ipath_insert_rnr_queue(qp);
		ipath_schedule_send(qp);
		goto bail;

	case 3:		/* NAK */
		if (qp->s_last == qp->s_tail)
			goto bail;
		/* The last valid PSN is the previous PSN. */
		update_last_psn(qp, psn - 1);
		switch ((aeth >> IPATH_AETH_CREDIT_SHIFT) &
			IPATH_AETH_CREDIT_MASK) {
		case 0:	/* PSN sequence error */
			dev->n_seq_naks++;
			/*
			 * Back up to the responder's expected PSN.
			 * Note that we might get a NAK in the middle of an
			 * RDMA READ response which terminates the RDMA
			 * READ.
			 */
			ipath_restart_rc(qp, psn);
			break;

		case 1:	/* Invalid Request */
			status = IB_WC_REM_INV_REQ_ERR;
			dev->n_other_naks++;
			goto class_b;

		case 2:	/* Remote Access Error */
			status = IB_WC_REM_ACCESS_ERR;
			dev->n_other_naks++;
			goto class_b;

		case 3:	/* Remote Operation Error */
			status = IB_WC_REM_OP_ERR;
			dev->n_other_naks++;
		class_b:
			ipath_send_complete(qp, wqe, status);
			ipath_error_qp(qp, IB_WC_WR_FLUSH_ERR);
			break;

		default:
			/* Ignore other reserved NAK error codes */
			goto reserved;
		}
		qp->s_rnr_retry = qp->s_rnr_retry_cnt;
		goto bail;

	default:		/* 2: reserved */
	reserved:
		/* Ignore reserved NAK codes. */
		goto bail;
	}

bail:
	return ret;
}

/**
 * ipath_rc_rcv_resp - process an incoming RC response packet
 * @dev: the device this packet came in on
 * @ohdr: the other headers for this packet
 * @data: the packet data
 * @tlen: the packet length
 * @qp: the QP for this packet
 * @opcode: the opcode for this packet
 * @psn: the packet sequence number for this packet
 * @hdrsize: the header length
 * @pmtu: the path MTU
 * @header_in_data: true if part of the header data is in the data buffer
 *
 * This is called from ipath_rc_rcv() to process an incoming RC response
 * packet for the given QP.
 * Called at interrupt level.
 */
static inline void ipath_rc_rcv_resp(struct ipath_ibdev *dev,
				     struct ipath_other_headers *ohdr,
				     void *data, u32 tlen,
				     struct ipath_qp *qp,
				     u32 opcode,
				     u32 psn, u32 hdrsize, u32 pmtu,
				     int header_in_data)
{
	struct ipath_swqe *wqe;
	enum ib_wc_status status;
	unsigned long flags;
	int diff;
	u32 pad;
	u32 aeth;
	u64 val;

	spin_lock_irqsave(&qp->s_lock, flags);

	/* Double check we can process this now that we hold the s_lock. */
	if (!(ib_ipath_state_ops[qp->state] & IPATH_PROCESS_RECV_OK))
		goto ack_done;

	/* Ignore invalid responses. */
	if (ipath_cmp24(psn, qp->s_next_psn) >= 0)
		goto ack_done;

	/* Ignore duplicate responses. */
	diff = ipath_cmp24(psn, qp->s_last_psn);
	if (unlikely(diff <= 0)) {
		/* Update credits for "ghost" ACKs */
		if (diff == 0 && opcode == OP(ACKNOWLEDGE)) {
			if (!header_in_data)
				aeth = be32_to_cpu(ohdr->u.aeth);
			else {
				aeth = be32_to_cpu(((__be32 *) data)[0]);
				data += sizeof(__be32);
			}
			if ((aeth >> 29) == 0)
				ipath_get_credit(qp, aeth);
		}
		goto ack_done;
	}

	if (unlikely(qp->s_last == qp->s_tail))
		goto ack_done;
	wqe = get_swqe_ptr(qp, qp->s_last);
	status = IB_WC_SUCCESS;

	switch (opcode) {
	case OP(ACKNOWLEDGE):
	case OP(ATOMIC_ACKNOWLEDGE):
	case OP(RDMA_READ_RESPONSE_FIRST):
		if (!header_in_data)
			aeth = be32_to_cpu(ohdr->u.aeth);
		else {
			aeth = be32_to_cpu(((__be32 *) data)[0]);
			data += sizeof(__be32);
		}
		if (opcode == OP(ATOMIC_ACKNOWLEDGE)) {
			if (!header_in_data) {
				__be32 *p = ohdr->u.at.atomic_ack_eth;

				val = ((u64) be32_to_cpu(p[0]) << 32) |
			