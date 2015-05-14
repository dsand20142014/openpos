if (opcode == OP(RDMA_WRITE_MIDDLE) ||
		    opcode == OP(RDMA_WRITE_LAST) ||
		    opcode == OP(RDMA_WRITE_LAST_WITH_IMMEDIATE))
			break;
		goto nack_inv;

	default:
		if (opcode == OP(SEND_MIDDLE) ||
		    opcode == OP(SEND_LAST) ||
		    opcode == OP(SEND_LAST_WITH_IMMEDIATE) ||
		    opcode == OP(RDMA_WRITE_MIDDLE) ||
		    opcode == OP(RDMA_WRITE_LAST) ||
		    opcode == OP(RDMA_WRITE_LAST_WITH_IMMEDIATE))
			goto nack_inv;
		/*
		 * Note that it is up to the requester to not send a new
		 * RDMA read or atomic operation before receiving an ACK
		 * for the previous operation.
		 */
		break;
	}

	memset(&wc, 0, sizeof wc);

	/* OK, process the packet. */
	switch (opcode) {
	case OP(SEND_FIRST):
		if (!ipath_get_rwqe(qp, 0))
			goto rnr_nak;
		qp->r_rcv_len = 0;
		/* FALLTHROUGH */
	case OP(SEND_MIDDLE):
	case OP(RDMA_WRITE_MIDDLE):
	send_middle:
		/* Check for invalid length PMTU or posted rwqe len. */
		if (unlikely(tlen != (hdrsize + pmtu + 4)))
			goto nack_inv;
		qp->r_rcv_len += pmtu;
		if (unlikely(qp->r_rcv_len > qp->r_len))
			goto nack_inv;
		ipath_copy_sge(&qp->r_sge, data, pmtu);
		break;

	case OP(RDMA_WRITE_LAST_WITH_IMMEDIATE):
		/* consume RWQE */
		if (!ipath_get_rwqe(qp, 1))
			goto rnr_nak;
		goto send_last_imm;

	case OP(SEND_ONLY):
	case OP(SEND_ONLY_WITH_IMMEDIATE):
		if (!ipath_get_rwqe(qp, 0))
			goto rnr_nak;
		qp->r_rcv_len = 0;
		if (opcode == OP(SEND_ONLY))
			goto send_last;
		/* FALLTHROUGH */
	case OP(SEND_LAST_WITH_IMMEDIATE):
	send_last_imm:
		if (header_in_data) {
			wc.ex.imm_data = *(__be32 *) data;
			data += sizeof(__be32);
		} else {
			/* Immediate data comes after BTH */
			wc.ex.imm_data = ohdr->u.imm_data;
		}
		hdrsize += 4;
		wc.wc_flags = IB_WC_WITH_IMM;
		/* FALLTHROUGH */
	case OP(SEND_LAST):
	case OP(RDMA_WRITE_LAST):
	send_last:
		/* Get the number of bytes the message was padded by. */
		pad = (be32_to_cpu(ohdr->bth[0]) >> 20) & 3;
		/* Check for invalid length. */
		/* XXX LAST len should be >= 1 */
		if (unlikely(tlen < (hdrsize + pad + 4)))
			goto nack_inv;
		/* Don't count the CRC. */
		tlen -= (hdrsize + pad + 4);
		wc.byte_len = tlen + qp->r_rcv_len;
		if (unlikely(wc.byte_len > qp->r_len))
			goto nack_inv;
		ipath_copy_sge(&qp->r_sge, data, tlen);
		qp->r_msn++;
		if (!test_and_clear_bit(IPATH_R_WRID_VALID, &qp->r_aflags))
			break;
		wc.wr_id = qp->r_wr_id;
		wc.status = IB_WC_SUCCESS;
		if (opcode == OP(RDMA_WRITE_LAST_WITH_IMMEDIATE) ||
		    opcode == OP(RDMA_WRITE_ONLY_WITH_IMMEDIATE))
			wc.opcode = IB_WC_RECV_RDMA_WITH_IMM;
		else
			wc.opcode = IB_WC_RECV;
		wc.qp = &qp->ibqp;
		wc.src_qp = qp->remote_qpn;
		wc.slid = qp->remote_ah_attr.dlid;
		wc.sl = qp->remote_ah_attr.sl;
		/* Signal completion event if the solicited bit is set. */
		ipath_cq_enter(to_icq(qp->ibqp.recv_cq), &wc,
			       (ohdr->bth[0] &
				cpu_to_be32(1 << 23)) != 0);
		break;

	case OP(RDMA_WRITE_FIRST):
	case OP(RDMA_WRITE_ONLY):
	case OP(RDMA_WRITE_ONLY_WITH_IMMEDIATE):
		if (unlikely(!(qp->qp_access_flags &
			       IB_ACCESS_REMOTE_WRITE)))
			goto nack_inv;
		/* consume RWQE */
		/* RETH comes after BTH */
		if (!header_in_data)
			reth = &ohdr->u.rc.reth;
		else {
			reth = (struct ib_reth *)data;
			data += sizeof(*reth);
		}
		hdrsize += sizeof(*reth);
		qp->r_len = be32_to_cpu(reth->length);
		qp->r_rcv_len = 0;
		if (qp->r_len != 0) {
			u32 rkey = be32_to_cpu(reth->rkey);
			u64 vaddr = be64_to_cpu(reth->vaddr);
			int ok;

			/* Check rkey & NAK */
			ok = ipath_rkey_ok(qp, &qp->r_sge,
					   qp->r_len, vaddr, rkey,
					   IB_ACCESS_REMOTE_WRITE);
			if (unlikely(!ok))
				goto nack_acc;
		} else {
			qp->r_sge.sg_list = NULL;
			qp->r_sge.sge.mr = NULL;
			qp->r_sge.sge.vaddr = NULL;
			qp->r_sge.sge.length = 0;
			qp->r_sge.sge.sge_length = 0;
		}
		if (opcode == OP(RDMA_WRITE_FIRST))
			goto send_middle;
		else if (opcode == OP(RDMA_WRITE_ONLY))
			goto send_last;
		if (!ipath_get_rwqe(qp, 1))
			goto rnr_nak;
		goto send_last_imm;

	case OP(RDMA_READ_REQUEST): {
		struct ipath_ack_entry *e;
		u32 len;
		u8 next;

		if (unlikely(!(qp->qp_access_flags &
			       IB_ACCESS_REMOTE_READ)))
			goto nack_inv;
		next = qp->r_head_ack_queue + 1;
		if (next > IPATH_MAX_RDMA_ATOMIC)
			next = 0;
		spin_lock_irqsave(&qp->s_lock, flags);
		/* Double check we can process this while holding the s_lock. */
		if (!(ib_ipath_state_ops[qp->state] & IPATH_PROCESS_RECV_OK))
			goto unlock;
		if (unlikely(next == qp->s_tail_ack_queue)) {
			if (!qp->s_ack_queue[next].sent)
				goto nack_inv_unlck;
			ipath_update_ack_queue(qp, next);
		}
		e = &qp->s_ack_queue[qp->r_head_ack_queue];
		/* RETH comes after BTH */
		if (!header_in_data)
			reth = &ohdr->u.rc.reth;
		else {
			reth = (struct ib_reth *)data;
			data += sizeof(*reth);
		}
		len = be32_to_cpu(reth->length);
		if (len) {
			u32 rkey = be32_to_cpu(reth->rkey);
			u64 vaddr = be64_to_cpu(reth->vaddr);
			int ok;

			/* Check rkey & NAK */
			ok = ipath_rkey_ok(qp, &e->rdma_sge, len, vaddr,
					   rkey, IB_ACCESS_REMOTE_READ);
			if (unlikely(!ok))
				goto nack_acc_unlck;
			/*
			 * Update the next expected PSN.  We add 1 later
			 * below, so only add the remainder here.
			 */
			if (len > pmtu)
				qp->r_psn += (len - 1) / pmtu;
		} else {
			e->rdma_sge.sg_list = NULL;
			e->rdma_sge.num_sge = 0;
			e->rdma_sge.sge.mr = NULL;
			e->rdma_sge.sge.vaddr = NULL;
			e->rdma_sge.sge.length = 0;
			e->rdma_sge.sge.sge_length = 0;
		}
		e->opcode = opcode;
		e->sent = 0;
		e->psn = psn;
		/*
		 * We need to increment the MSN here instead of when we
		 * finish sending the result since a duplicate request would
		 * increment it more than once.
		 */
		qp->r_msn++;
		qp->r_psn++;
		qp->r_state = opcode;
		qp->r_nak_state = 0;
		qp->r_head_ack_queue = next;

		/* Schedule the send tasklet. */
		ipath_schedule_send(qp);

		goto unlock;
	}

	case OP(COMPARE_SWAP):
	case OP(FETCH_ADD): {
		struct ib_atomic_eth *ateth;
		struct ipath_ack_entry *e;
		u64 vaddr;
		atomic64_t *maddr;
		u64 sdata;
		u32 rkey;
		u8 next;

		if (unlikely(!(qp->qp_access_flags &
			       IB_ACCESS_REMOTE_ATOMIC)))
			goto nack_inv;
		next = qp->r_head_ack_queue + 1;
		if (next > IPATH_MAX_RDMA_ATOMIC)
			next = 0;
		spin_lock_irqsave(&qp->s_lock, flags);
		/* Double check we can process this while holding the s_lock. */
		if (!(ib_ipath_state_ops[qp->state] & IPATH_PROCESS_RECV_OK))
			goto unlock;
		if (unlikely(next == qp->s_tail_ack_queue)) {
			if (!qp->s_ack_queue[next].sent)
				goto nack_inv_unlck;
			ipath_update_ack_queue(qp, next);
		}
		if (!header_in_data)
			ateth = &ohdr->u.atomic_eth;
		else
			ateth = (struct ib_atomic_eth *)data;
		vaddr = ((u64) be32_to_cpu(ateth->vaddr[0]) << 32) |
			be32_to_cpu(ateth->vaddr[1]);
		if (unlikely(vaddr & (sizeof(u64) - 1)))
			goto nack_inv_unlck;
		rkey = be32_to_cpu(ateth->rkey);
		/* Check rkey & NAK */
		if (unlikely(!ipath_rkey_ok(qp, &qp->r_sge,
					    sizeof(u64), vaddr, rkey,
					    IB_ACCESS_REMOTE_ATOMIC)))
			goto nack_acc_unlck;
		/* Perform atomic OP and save result. */
		maddr = (atomic64_t *) qp->r_sge.sge.vaddr;
		sdata = be64_to_cpu(ateth->swap_data);
		e = &qp->s_ack_queue[qp->r_head_ack_queue];
		e->atomic_data = (opcode == OP(FETCH_ADD)) ?
			(u64) atomic64_add_return(sdata, maddr) - sdata :
			(u64) cmpxchg((u64 *) qp->r_sge.sge.vaddr,
				      be64_to_cpu(ateth->compare_data),
				      sdata);
		e->opcode = opcode;
		e->sent = 0;
		e->psn = psn & IPATH_PSN_MASK;
		qp->r_msn++;
		qp->r_psn++;
		qp->r_state = opcode;
		qp->r_nak_state = 0;
		qp->r_head_ack_queue = next;

		/* Schedule the send tasklet. */
		ipath_schedule_send(qp);

		goto unlock;
	}

	default:
		/* NAK unknown opcodes. */
		goto nack_inv;
	}
	qp->r_psn++;
	qp->r_state = opcode;
	qp->r_ack_psn = psn;
	qp->r_nak_state = 0;
	/* Send an ACK if requested or required. */
	if (psn & (1 << 31))
		goto send_ack;
	goto done;

rnr_nak:
	qp->r_nak_state = IB_RNR_NAK | qp->r_min_rnr_timer;
	qp->r_ack_psn = qp->r_psn;
	goto send_ack;

nack_inv_unlck:
	spin_unlock_irqrestore(&qp->s_lock, flags);
nack_inv:
	ipath_rc_error(qp, IB_WC_LOC_QP_OP_ERR);
	qp->r_nak_state = IB_NAK_INVALID_REQUEST;
	qp->r_ack_psn = qp->r_psn;
	goto send_ack;

nack_acc_unlck:
	spin_unlock_irqrestore(&qp->s_lock, flags);
nack_acc:
	ipath_rc_error(qp, IB_WC_LOC_PROT_ERR);
	qp->r_nak_state = IB_NAK_REMOTE_ACCESS_ERROR;
	qp->r_ack_psn = qp->r_psn;
send_ack:
	send_rc_ack(qp);
	goto done;

unlock:
	spin_unlock_irqrestore(&qp->s_lock, flags);
done:
	return;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /*
 * QLogic Fibre Channel HBA Driver
 * Copyright (c)  2003-2008 QLogic Corporation
 *
 * See LICENSE.qla2xxx for copyright and licensing details.
 */
#include "qla_def.h"

#include <linux/blkdev.h>
#include <linux/delay.h>

#include <scsi/scsi_tcq.h>

static request_t *qla2x00_req_pkt(struct scsi_qla_host *, struct req_que *,
							struct rsp_que *rsp);
static void qla2x00_isp_cmd(struct scsi_qla_host *, struct req_que *);

static void qla25xx_set_que(srb_t *, struct rsp_que **);
/**
 * qla2x00_get_cmd_direction() - Determine control_flag data direction.
 * @cmd: SCSI command
 *
 * Returns the proper CF_* direction based on CDB.
 */
static inline uint16_t
qla2x00_get_cmd_direction(srb_t *sp)
{
	uint16_t cflags;

	cflags = 0;

	/* Set transfer direction */
	if (sp->cmd->sc_data_direction == DMA_TO_DEVICE) {
		cflags = CF_WRITE;
		sp->fcport->vha->hw->qla_stats.output_bytes +=
		    scsi_bufflen(sp->cmd);
	} else if (sp->cmd->sc_data_direction == DMA_FROM_DEVICE) {
		cflags = CF_READ;
		sp->fcport->vha->hw->qla_stats.input_bytes +=
		    scsi_bufflen(sp->cmd);
	}
	return (cflags);
}

/**
 * qla2x00_calc_iocbs_32() - Determine number of Command Type 2 and
 * Continuation Type 0 IOCBs to allocate.
 *
 * @dsds: number of data segment decriptors needed
 *
 * Returns the number of IOCB entries needed to store @dsds.
 */
uint16_t
qla2x00_calc_iocbs_32(uint16_t dsds)
{
	uint16_t iocbs;

	iocbs = 1;
	if (dsds > 3) {
		iocbs += (dsds - 3) / 7;
		if ((dsds - 3) % 7)
			iocbs++;
	}
	return (iocbs);
}

/**
 * qla2x00_calc_iocbs_64() - Determine number of Command Type 3 and
 * Continuation Type 1 IOCBs to allocate.
 *
 * @dsds: number of data segment decriptors needed
 *
 * Returns the number of IOCB entries needed to store @dsds.
 */
uint16_t
qla2x00_calc_iocbs_64(uint16_t dsds)
{
	uint16_t iocbs;

	iocbs = 1;
	if (dsds > 2) {
		iocbs += (dsds - 2) / 5;
		if ((dsds - 2) % 5)
			iocbs++;
	}
	return (iocbs);
}

/**
 * qla2x00_prep_cont_type0_iocb() - Initialize a Continuation Type 0 IOCB.
 * @ha: HA context
 *
 * Returns a pointer to the Continuation Type 0 IOCB packet.
 */
static inline cont_entry_t *
qla2x00_prep_cont_type0_iocb(struct scsi_qla_host *vha)
{
	cont_entry_t *cont_pkt;
	struct req_que *req = vha->req;
	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else {
		req->ring_ptr++;
	}

	cont_pkt = (cont_entry_t *)req->ring_ptr;

	/* Load packet defaults. */
	*((uint32_t *)(&cont_pkt->entry_type)) =
	    __constant_cpu_to_le32(CONTINUE_TYPE);

	return (cont_pkt);
}

/**
 * qla2x00_prep_cont_type1_iocb() - Initialize a Continuation Type 1 IOCB.
 * @ha: HA context
 *
 * Returns a pointer to the continuation type 1 IOCB packet.
 */
static inline cont_a64_entry_t *
qla2x00_prep_cont_type1_iocb(scsi_qla_host_t *vha)
{
	cont_a64_entry_t *cont_pkt;

	struct req_que *req = vha->req;
	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else {
		req->ring_ptr++;
	}

	cont_pkt = (cont_a64_entry_t *)req->ring_ptr;

	/* Load packet defaults. */
	*((uint32_t *)(&cont_pkt->entry_type)) =
	    __constant_cpu_to_le32(CONTINUE_A64_TYPE);

	return (cont_pkt);
}

/**
 * qla2x00_build_scsi_iocbs_32() - Build IOCB command utilizing 32bit
 * capable IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 2 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
void qla2x00_build_scsi_iocbs_32(srb_t *sp, cmd_entry_t *cmd_pkt,
    uint16_t tot_dsds)
{
	uint16_t	avail_dsds;
	uint32_t	*cur_dsd;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	struct scatterlist *sg;
	int i;

	cmd = sp->cmd;

	/* Update entry type to indicate Command Type 2 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_TYPE);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return;
	}

	vha = sp->fcport->vha;
	cmd_pkt->control_flags |= cpu_to_le16(qla2x00_get_cmd_direction(sp));

	/* Three DSDs are available in the Command Type 2 IOCB */
	avail_dsds = 3;
	cur_dsd = (uint32_t *)&cmd_pkt->dseg_0_address;

	/* Load data segments */
	scsi_for_each_sg(cmd, sg, tot_dsds, i) {
		cont_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			 * Seven DSDs are available in the Continuation
			 * Type 0 IOCB.
			 */
			cont_pkt = qla2x00_prep_cont_type0_iocb(vha);
			cur_dsd = (uint32_t *)&cont_pkt->dseg_0_address;
			avail_dsds = 7;
		}

		*cur_dsd++ = cpu_to_le32(sg_dma_address(sg));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
}

/**
 * qla2x00_build_scsi_iocbs_64() - Build IOCB command utilizing 64bit
 * capable IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 3 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
void qla2x00_build_scsi_iocbs_64(srb_t *sp, cmd_entry_t *cmd_pkt,
    uint16_t tot_dsds)
{
	uint16_t	avail_dsds;
	uint32_t	*cur_dsd;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	struct scatterlist *sg;
	int i;

	cmd = sp->cmd;

	/* Update entry type to indicate Command Type 3 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_A64_TYPE);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return;
	}

	vha = sp->fcport->vha;
	cmd_pkt->control_flags |= cpu_to_le16(qla2x00_get_cmd_direction(sp));

	/* Two DSDs are available in the Command Type 3 IOCB */
	avail_dsds = 2;
	cur_dsd = (uint32_t *)&cmd_pkt->dseg_0_address;

	/* Load data segments */
	scsi_for_each_sg(cmd, sg, tot_dsds, i) {
		dma_addr_t	sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			 * Five DSDs are available in the Continuation
			 * Type 1 IOCB.
			 */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha);
			cur_dsd = (uint32_t *)cont_pkt->dseg_0_address;
			avail_dsds = 5;
		}

		sle_dma = sg_dma_address(sg);
		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
}

/**
 * qla2x00_start_scsi() - Send a SCSI command to the ISP
 * @sp: command to send to the ISP
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
qla2x00_start_scsi(srb_t *sp)
{
	int		ret, nseg;
	unsigned long   flags;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	uint32_t	*clr_ptr;
	uint32_t        index;
	uint32_t	handle;
	cmd_entry_t	*cmd_pkt;
	uint16_t	cnt;
	uint16_t	req_cnt;
	uint16_t	tot_dsds;
	struct device_reg_2xxx __iomem *reg;
	struct qla_hw_data *ha;
	struct req_que *req;
	struct rsp_que *rsp;

	/* Setup device pointers. */
	ret = 0;
	vha = sp->fcport->vha;
	ha = vha->hw;
	reg = &ha->iobase->isp;
	cmd = sp->cmd;
	req = ha->req_q_map[0];
	rsp = ha->rsp_q_map[0];
	/* So we know we haven't pci_map'ed anything yet */
	tot_dsds = 0;

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req, rsp, 0, 0, MK_SYNC_ALL)
							!= QLA_SUCCESS)
			return (QLA_FUNCTION_FAILED);
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < MAX_OUTSTANDING_COMMANDS; index++) {
		handle++;
		if (handle == MAX_OUTSTANDING_COMMANDS)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}
	if (index == MAX_OUTSTANDING_COMMANDS)
		goto queuing_error;

	/* Map the sg table so we have an accurate count of sg entries needed */
	if (scsi_sg_count(cmd)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd),
		    scsi_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
	} else
		nseg = 0;

	tot_dsds = nseg;

	/* Calculate the number of request entries needed. */
	req_cnt = ha->isp_ops->calc_req_entries(tot_dsds);
	if (req->cnt < (req_cnt + 2)) {
		cnt = RD_REG_WORD_RELAXED(ISP_REQ_Q_OUT(ha, reg));
		if (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
			    (req->ring_index - cnt);
	}
	if (req->cnt < (req_cnt + 2))
		goto queuing_error;

	/* Build command packet */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->cmd->host_scribble = (unsigned char *)(unsigned long)handle;
	req->cnt -= req_cnt;

	cmd_pkt = (cmd_entry_t *)req->ring_ptr;
	cmd_pkt->handle = handle;
	/* Zero out remaining portion of packet. */
	clr_ptr = (uint32_t *)cmd_pkt + 2;
	memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
	cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

	/* Set target ID and LUN number*/
	SET_TARGET_ID(ha, cmd_pkt->target, sp->fcport->loop_id);
	cmd_pkt->lun = cpu_to_le16(sp->cmd->device->lun);

	/* Update tagged queuing modifier */
	cmd_pkt->control_flags = __constant_cpu_to_le16(CF_SIMPLE_TAG);

	/* Load SCSI command packet. */
	memcpy(cmd_pkt->scsi_cdb, cmd->cmnd, cmd->cmd_len);
	cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

	/* Build IOCB segments */
	ha->isp_ops->build_iocbs(sp, cmd_pkt, tot_dsds);

	/* Set total data segment count. */
	cmd_pkt->entry_count = (uint8_t)req_cnt;
	wmb();

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	sp->flags |= SRB_DMA_VALID;

	/* Set chip new ring index. */
	WRT_REG_WORD(ISP_REQ_Q_IN(ha, reg), req->ring_index);
	RD_REG_WORD_RELAXED(ISP_REQ_Q_IN(ha, reg));	/* PCI Posting. */

	/* Manage unprocessed RIO/ZIO commands in response queue. */
	if (vha->flags.process_response_queue &&
	    rsp->ring_ptr->signature != RESPONSE_PROCESSED)
		qla2x00_process_response_queue(rsp);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return (QLA_SUCCESS);

queuing_error:
	if (tot_dsds)
		scsi_dma_unmap(cmd);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return (QLA_FUNCTION_FAILED);
}

/**
 * qla2x00_marker() - Send a marker IOCB to the firmware.
 * @ha: HA context
 * @loop_id: loop ID
 * @lun: LUN
 * @type: marker modifier
 *
 * Can be called from both normal and interrupt context.
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
__qla2x00_marker(struct scsi_qla_host *vha, struct req_que *req,
			struct rsp_que *rsp, uint16_t loop_id,
			uint16_t lun, uint8_t type)
{
	mrk_entry_t *mrk;
	struct mrk_entry_24xx *mrk24;
	struct qla_hw_data *ha = vha->hw;
	scsi_qla_host_t *base_vha = pci_get_drvdata(ha->pdev);

	mrk24 = NULL;
	mrk = (mrk_entry_t *)qla2x00_req_pkt(vha, req, rsp);
	if (mrk == NULL) {
		DEBUG2_3(printk("%s(%ld): failed to allocate Marker IOCB.\n",
		    __func__, base_vha->host_no));

		return (QLA_FUNCTION_FAILED);
	}

	mrk->entry_type = MARKER_TYPE;
	mrk->modifier = type;
	if (type != MK_SYNC_ALL) {
		if (IS_FWI2_CAPABLE(ha)) {
			mrk24 = (struct mrk_entry_24xx *) mrk;
			mrk24->nport_handle = cpu_to_le16(loop_id);
			mrk24->lun[1] = LSB(lun);
			mrk24->lun[2] = MSB(lun);
			host_to_fcp_swap(mrk24->lun, sizeof(mrk24->lun));
			mrk24->vp_index = vha->vp_idx;
			mrk24->handle = MAKE_HANDLE(req->id, mrk24->handle);
		} else {
			SET_TARGET_ID(ha, mrk->target, loop_id);
			mrk->lun = cpu_to_le16(lun);
		}
	}
	wmb();

	qla2x00_isp_cmd(vha, req);

	return (QLA_SUCCESS);
}

int
qla2x00_marker(struct scsi_qla_host *vha, struct req_que *req,
		struct rsp_que *rsp, uint16_t loop_id, uint16_t lun,
		uint8_t type)
{
	int ret;
	unsigned long flags = 0;

	spin_lock_irqsave(&vha->hw->hardware_lock, flags);
	ret = __qla2x00_marker(vha, req, rsp, loop_id, lun, type);
	spin_unlock_irqrestore(&vha->hw->hardware_lock, flags);

	return (ret);
}

/**
 * qla2x00_req_pkt() - Retrieve a request packet from the request ring.
 * @ha: HA context
 *
 * Note: The caller must hold the hardware lock before calling this routine.
 *
 * Returns NULL if function failed, else, a pointer to the request packet.
 */
static request_t *
qla2x00_req_pkt(struct scsi_qla_host *vha, struct req_que *req,
		struct rsp_que *rsp)
{
	struct qla_hw_data *ha = vha->hw;
	device_reg_t __iomem *reg = ISP_QUE_REG(ha, req->id);
	request_t	*pkt = NULL;
	uint16_t	cnt;
	uint32_t	*dword_ptr;
	uint32_t	timer;
	uint16_t	req_cnt = 1;

	/* Wait 1 second for slot. */
	for (timer = HZ; timer; timer--) {
		if ((req_cnt + 2) >= req->cnt) {
			/* Calculate number of free request entries. */
			if (ha->mqenable)
				cnt = (uint16_t)
					RD_REG_DWORD(&reg->isp25mq.req_q_out);
			else {
				if (IS_FWI2_CAPABLE(ha))
					cnt = (uint16_t)RD_REG_DWORD(
						&reg->isp24.req_q_out);
				else
					cnt = qla2x00_debounce_register(
						ISP_REQ_Q_OUT(ha, &reg->isp));
			}
			if  (req->ring_index < cnt)
				req->cnt = cnt - req->ring_index;
			else
				req->cnt = req->length -
				    (req->ring_index - cnt);
		}
		/* If room for request in request ring. */
		if ((req_cnt + 2) < req->cnt) {
			req->cnt--;
			pkt = req->ring_ptr;

			/* Zero out packet. */
			dword_ptr = (uint32_t *)pkt;
			for (cnt = 0; cnt < REQUEST_ENTRY_SIZE / 4; cnt++)
				*dword_ptr++ = 0;

			/* Set entry count. */
			pkt->entry_count = 1;

			break;
		}

		/* Release ring specific lock */
		spin_unlock_irq(&ha->hardware_lock);

		udelay(2);   /* 2 us */

		/* Check for pending interrupts. */
		/* During init we issue marker directly */
		if (!vha->marker_needed && !vha->flags.init_done)
			qla2x00_poll(rsp);
		spin_lock_irq(&ha->hardware_lock);
	}
	if (!pkt) {
		DEBUG2_3(printk("%s(): **** FAILED ****\n", __func__));
	}

	return (pkt);
}

/**
 * qla2x00_isp_cmd() - Modify the request ring pointer.
 * @ha: HA context
 *
 * Note: The caller must hold the hardware lock before calling this routine.
 */
static void
qla2x00_isp_cmd(struct scsi_qla_host *vha, struct req_que *req)
{
	struct qla_hw_data *ha = vha->hw;
	device_reg_t __iomem *reg = ISP_QUE_REG(ha, req->id);
	struct device_reg_2xxx __iomem *ioreg = &ha->iobase->isp;

	DEBUG5(printk("%s(): IOCB data:\n", __func__));
	DEBUG5(qla2x00_dump_buffer(
	    (uint8_t *)req->ring_ptr, REQUEST_ENTRY_SIZE));

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	/* Set chip new ring index. */
	if (ha->mqenable) {
		WRT_REG_DWORD(&reg->isp25mq.req_q_in, req->ring_index);
		RD_REG_DWORD(&ioreg->hccr);
	}
	else {
		if (IS_FWI2_CAPABLE(ha)) {
			WRT_REG_DWORD(&reg->isp24.req_q_in, req->ring_index);
			RD_REG_DWORD_RELAXED(&reg->isp24.req_q_in);
		} else {
			WRT_REG_WORD(ISP_REQ_Q_IN(ha, &reg->isp),
				req->ring_index);
			RD_REG_WORD_RELAXED(ISP_REQ_Q_IN(ha, &reg->isp));
		}
	}

}

/**
 * qla24xx_calc_iocbs() - Determine number of Command Type 3 and
 * Continuation Type 1 IOCBs to allocate.
 *
 * @dsds: number of data segment decriptors needed
 *
 * Returns the number of IOCB entries needed to store @dsds.
 */
static inline uint16_t
qla24xx_calc_iocbs(uint16_t dsds)
{
	uint16_t iocbs;

	iocbs = 1;
	if (dsds > 1) {
		iocbs += (dsds - 1) / 5;
		if ((dsds - 1) % 5)
			iocbs++;
	}
	return iocbs;
}

/**
 * qla24xx_build_scsi_iocbs() - Build IOCB command utilizing Command Type 7
 * IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 3 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
static inline void
qla24xx_build_scsi_iocbs(srb_t *sp, struct cmd_type_7 *cmd_pkt,
    uint16_t tot_dsds)
{
	uint16_t	avail_dsds;
	uint32_t	*cur_dsd;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	struct scatterlist *sg;
	int i;
	struct req_que *req;

	cmd = sp->cmd;

	/* Update entry type to indicate Command Type 3 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_TYPE_7);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return;
	}

	vha = sp->fcport->vha;
	req = vha->req;

	/* Set transfer direction */
	if (cmd->sc_data_direction == DMA_TO_DEVICE) {
		cmd_pkt->task_mgmt_flags =
		    __constant_cpu_to_le16(TMF_WRITE_DATA);
		sp->fcport->vha->hw->qla_stats.output_bytes +=
		    scsi_bufflen(sp->cmd);
	} else if (cmd->sc_data_direction == DMA_FROM_DEVICE) {
		cmd_pkt->task_mgmt_flags =
		    __constant_cpu_to_le16(TMF_READ_DATA);
		sp->fcport->vha->hw->qla_stats.input_bytes +=
		    scsi_bufflen(sp->cmd);
	}

	/* One DSD is available in the Command Type 3 IOCB */
	avail_dsds = 1;
	cur_dsd = (uint32_t *)&cmd_pkt->dseg_0_address;

	/* Load data segments */

	scsi_for_each_sg(cmd, sg, tot_dsds, i) {
		dma_addr_t	sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			 * Five DSDs are available in the Continuation
			 * Type 1 IOCB.
			 */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha);
			cur_dsd = (uint32_t *)cont_pkt->dseg_0_address;
			avail_dsds = 5;
		}

		sle_dma = sg_dma_address(sg);
		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
}


/**
 * qla24xx_start_scsi() - Send a SCSI command to the ISP
 * @sp: command to send to the ISP
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
qla24xx_start_scsi(srb_t *sp)
{
	int		ret, nseg;
	unsigned long   flags;
	uint32_t	*clr_ptr;
	uint32_t        index;
	uint32_t	handle;
	struct cmd_type_7 *cmd_pkt;
	uint16_t	cnt;
	uint16_t	req_cnt;
	uint16_t	tot_dsds;
	struct req_que *req = NULL;
	struct rsp_que *rsp = NULL;
	struct scsi_cmnd *cmd = sp->cmd;
	struct scsi_qla_host *vha = sp->fcport->vha;
	struct qla_hw_data *ha = vha->hw;

	/* Setup device pointers. */
	ret = 0;

	qla25xx_set_que(sp, &rsp);
	req = vha->req;

	/* So we know we haven't pci_map'ed anything yet */
	tot_dsds = 0;

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req, rsp, 0, 0, MK_SYNC_ALL)
							!= QLA_SUCCESS)
			return QLA_FUNCTION_FAILED;
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < MAX_OUTSTANDING_COMMANDS; index++) {
		handle++;
		if (handle == MAX_OUTSTANDING_COMMANDS)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}
	if (index == MAX_OUTSTANDING_COMMANDS)
		goto queuing_error;

	/* Map the sg table so we have an accurate count of sg entries needed */
	if (scsi_sg_count(cmd)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd),
		    scsi_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
	} else
		nseg = 0;

	tot_dsds = nseg;

	req_cnt = qla24xx_calc_iocbs(tot_dsds);
	if (req->cnt < (req_cnt + 2)) {
		cnt = RD_REG_DWORD_RELAXED(req->req_q_out);

		if (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
				(req->ring_index - cnt);
	}
	if (req->cnt < (req_cnt + 2))
		goto queuing_error;

	/* Build command packet. */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->cmd->host_scribble = (unsigned char *)(unsigned long)handle;
	req->cnt -= req_cnt;

	cmd_pkt = (struct cmd_type_7 *)req->ring_ptr;
	cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

	/* Zero out remaining portion of packet. */
	/*    tagged queuing modifier -- default is TSK_SIMPLE (0). */
	clr_ptr = (uint32_t *)cmd_pkt + 2;
	memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
	cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

	/* Set NPORT-ID and LUN number*/
	cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
	cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
	cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;
	cmd_pkt->vp_index = sp->fcport->vp_idx;

	int_to_scsilun(sp->cmd->device->lun, &cmd_pkt->lun);
	host_to_fcp_swap((uint8_t *)&cmd_pkt->lun, sizeof(cmd_pkt->lun));

	/* Load SCSI command packet. */
	memcpy(cmd_pkt->fcp_cdb, cmd->cmnd, cmd->cmd_len);
	host_to_fcp_swap(cmd_pkt->fcp_cdb, sizeof(cmd_pkt->fcp_cdb));

	cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

	/* Build IOCB segments */
	qla24xx_build_scsi_iocbs(sp, cmd_pkt, tot_dsds);

	/* Set total data segment count. */
	cmd_pkt->entry_count = (uint8_t)req_cnt;
	/* Specify response queue number where completion should happen */
	cmd_pkt->entry_status = (uint8_t) rsp->id;
	wmb();

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	sp->flags |= SRB_DMA_VALID;

	/* Set chip new ring index. */
	WRT_REG_DWORD(req->req_q_in, req->ring_index);
	RD_REG_DWORD_RELAXED(&ha->iobase->isp24.hccr);

	/* Manage unprocessed RIO/ZIO commands in response queue. */
	if (vha->flags.process_response_queue &&
		rsp->ring_ptr->signature != RESPONSE_PROCESSED)
		qla24xx_process_response_queue(vha, rsp);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return QLA_SUCCESS;

queuing_error:
	if (tot_dsds)
		scsi_dma_unmap(cmd);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_FUNCTION_FAILED;
}

static void qla25xx_set_que(srb_t *sp, struct rsp_que **rsp)
{
	struct scsi_cmnd *cmd = sp->cmd;
	struct qla_hw_data *ha = sp->fcport->vha->hw;
	int affinity = cmd->request->cpu;

	if (ql2xmultique_tag && affinity >= 0 &&
		affinity < ha->max_rsp_queues - 1)
		*rsp = ha->rsp_q_map[affinity + 1];
	 else
		*rsp = ha->rsp_q_map[0];
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
 * Copyright (c) 2001-2002 by David Brownell
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

#ifndef __USB_CORE_HCD_H
#define __USB_CORE_HCD_H

#ifdef __KERNEL__

#include <linux/rwsem.h>

#define MAX_TOPO_LEVEL		6

/* This file contains declarations of usbcore internals that are mostly
 * used or exposed by Host Controller Drivers.
 */

/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_EXT			0xf0	/* USB 2.0 LPM ECN */
#define USB_PID_OUT			0xe1
#define USB_PID_ACK			0xd2
#define USB_PID_DATA0			0xc3
#define USB_PID_PING			0xb4	/* USB 2.0 */
#define USB_PID_SOF			0xa5
#define USB_PID_NYET			0x96	/* USB 2.0 */
#define USB_PID_DATA2			0x87	/* USB 2.0 */
#define USB_PID_SPLIT			0x78	/* USB 2.0 */
#define USB_PID_IN			0x69
#define USB_PID_NAK			0x5a
#define USB_PID_DATA1			0x4b
#define USB_PID_PREAMBLE		0x3c	/* Token mode */
#define USB_PID_ERR			0x3c	/* USB 2.0: handshake mode */
#define USB_PID_SETUP			0x2d
#define USB_PID_STALL			0x1e
#define USB_PID_MDATA			0x0f	/* USB 2.0 */

/*-------------------------------------------------------------------------*/

/*
 * USB Host Controller Driver (usb_hcd) framework
 *
 * Since "struct usb_bus" is so thin, you can't share much code in it.
 * This framework is a layer over that, and should be more sharable.
 *
 * @authorized_default: Specifies if new devices are authorized to
 *                      connect by default or they require explicit
 *                      user space authorization; this bit is settable
 *                      through /sys/class/usb_host/X/authorized_default.
 *                      For the rest is RO, so we don't lock to r/w it.
 */

/*-------------------------------------------------------------------------*/

struct usb_hcd {

	/*
	 * housekeeping
	 */
	struct usb_bus		self;		/* hcd is-a bus */
	struct kref		kref;		/* reference counter */

	const char		*product_desc;	/* product/vendor string */
	char			irq_descr[24];	/* driver + bus # */

	struct timer_list	rh_timer;	/* drives root-hub polling */
	struct urb		*status_urb;	/* the current status urb */
#ifdef CONFIG_PM
	struct work_struct	wakeup_work;	/* for remote wakeup */
#endif

	/*
	 * hardware info/state
	 */
	const struct hc_driver	*driver;	/* hw-specific hooks */

	/* Flags that need to be manipulated atomically */
	unsigned long		flags;
#define HCD_FLAG_HW_ACCESSIBLE	0x00000001
#define HCD_FLAG_SAW_IRQ	0x00000002

	unsigned		rh_registered:1;/* is root hub registered? */

	/* The next flag is a stopgap, to be removed when all the HCDs
	 * support the new root-hub polling mechanism. */
	unsigned		uses_new_polling:1;
	unsigned		poll_rh:1;	/* poll for rh status? */
	unsigned		poll_pending:1;	/* status has changed? */
	unsigned		wireless:1;	/* Wireless USB HCD */
	unsigned		authorized_default:1;
	unsigned		has_tt:1;	/* Integrated TT in root hub */

	int			irq;		/* irq allocated */
	void __iomem		*regs;		/* device memory/io */
	u64			rsrc_start;	/* memory/io resource start */
	u64			rsrc_len;	/* memory/io resource length */
	unsigned		power_budget;	/* in mA, 0 = no limit */

#define HCD_BUFFER_POOLS	4
	struct dma_pool		*pool [HCD_BUFFER_POOLS];

	int			state;
#	define	__ACTIVE		0x01
#	define	__SUSPEND		0x04
#	define	__TRANSIENT		0x80

#	define	HC_STATE_HALT		0
#	define	HC_STATE_RUNNING	(__ACTIVE)
#	define	HC_STATE_QUIESCING	(__SUSPEND|__TRANSIENT|__ACTIVE)
#	define	HC_STATE_RESUMING	(__SUSPEND|__TRANSIENT)
#	define	HC_STATE_SUSPENDED	(__SUSPEND)

#define	HC_IS_RUNNING(state) ((state) & __ACTIVE)
#define	HC_IS_SUSPENDED(state) ((state) & __SUSPEND)

	/* more shared queuing code would be good; it should support
	 * smarter scheduling, handle transaction translators, etc;
	 * input size of periodic table to an interrupt scheduler.
	 * (ohci 32, uhci 1024, ehci 256/512/1024).
	 */

	/* The HC driver's private data is stored at the end of
	 * this structure.
	 */
	unsigned long hcd_priv[0]
			__attribute__ ((aligned(sizeof(unsigned long))));
};

/* 2.4 does this a bit differently ... */
static inline struct usb_bus *hcd_to_bus(struct usb_hcd *hcd)
{
	return &hcd->self;
}

static inline struct usb_hcd *bus_to_hcd(struct usb_bus *bus)
{
	return container_of(bus, struct usb_hcd, self);
}

struct hcd_timeout {	/* timeouts we allocate */
	struct list_head	timeout_list;
	struct timer_list	timer;
};

/*-------------------------------------------------------------------------*/


struct hc_driver {
	const char	*description;	/* "ehci-hcd" etc */
	const char	*product_desc;	/* product/vendor string */
	size_t		hcd_priv_size;	/* size of private data */

	/* irq handler */
	irqreturn_t	(*irq) (struct usb_hcd *hcd);

	int	flags;
#define	HCD_MEMORY	0x0001		/* HC regs use memory (else I/O) */
#define	HCD_LOCAL_MEM	0x0002		/* HC needs local memory */
#define	HCD_USB11	0x0010		/* USB 1.1 */
#define	HCD_USB2	0x0020		/* USB 2.0 */
#define	HCD_USB3	0x0040		/* USB 3.0 */
#define	HCD_MASK	0x0070

	/* called to init HCD and root hub */
	int	(*reset) (struct usb_hcd *hcd);
	int	(*start) (struct usb_hcd *hcd);

	/* NOTE:  these suspend/resume calls relate to the HC as
	 * a whole, not just the root hub; they're for PCI bus glue.
	 */
	/* called after suspending the hub, before entering D3 etc */
	int	(*pci_suspend)(struct usb_hcd *hcd);

	/* called after entering D0 (etc), before resuming the hub */
	int	(*pci_resume)(struct usb_hcd *hcd, bool hibernated);

	/* cleanly make HCD stop writing memory and doing I/O */
	void	(*stop) (struct usb_hcd *hcd);

	/* shutdown HCD */
	void	(*shutdown) (struct usb_hcd *hcd);

	/* return current frame number */
	int	(*get_frame_number) (struct usb_hcd *hcd);

	/* manage i/o requests, device state */
	int	(*urb_enqueue)(struct usb_hcd *hcd,
				struct urb *urb, gfp_t mem_flags);
	int	(*urb_dequeue)(struct usb_hcd *hcd,
				struct urb *urb, int status);

	/* hw synch, freeing endpoint resources that urb_dequeue can't */
	void 	(*endpoint_disable)(struct usb_hcd *hcd,
			struct usb_host_endpoint *ep);

	/* (optional) reset any endpoint state such as sequence number
	   and current window */
	void 	(*endpoint_reset)(struct usb_hcd *hcd,
			struct usb_host_endpoint *ep);

	/* root hub support */
	int	(*hub_status_data) (struct usb_hcd *hcd, char *buf);
	int	(*hub_control) (struct usb_hcd *hcd,
				u16 typeReq, u16 wValue, u16 wIndex,
				char *buf, u16 wLength);
	int	(*bus_suspend)(struct usb_hcd *);
	int	(*bus_resume)(struct usb_hcd *);
	int	(*start_port_reset)(struct usb_hcd *, unsigned port_num);

		/* force handover of high-speed port to full-speed companion */
	void	(*relinquish_port)(struct usb_hcd *, int);
		/* has a port been handed over to a companion? */
	int	(*port_handed_over)(struct usb_hcd *, int);

		/* CLEAR_TT_BUFFER completion callback */
	void	(*clear_tt_buffer_complete)(struct usb_hcd *,
				struct usb_host_endpoint *);

	/* xHCI specific functions */
		/* Called by usb_alloc_dev to alloc HC device structures */
	int	(*alloc_dev)(struct usb_hcd *, struct usb_device *);
		/* Called by usb_release_dev to free HC device structures */
	void	(*free_dev)(struct usb_hcd *, struct usb_device *);

	/* Bandwidth computation functions */
	/* Note that add_endpoint() can only be called once per endpoint before
	 * check_bandwidth() or reset_bandwidth() must be called.
	 * drop_endpoint() can only be called once per endpoint also.
	 * A call to xhci_drop_endpoint() followed by a call to xhci_add_endpoint() will
	 * add the endpoint to the schedule with possibly new parameters denoted by a
	 * different endpoint descriptor in usb_host_endpoint.
	 * A call to xhci_add_endpoint() followed by a call to xhci_drop_endpoint() is
	 * not allowed.
	 */
		/* Allocate endpoint resources and add them to a new schedule */
	int 	(*add_endpoint)(struct usb_hcd *, struct usb_device *, struct usb_host_endpoint *);
		/* Drop an endpoint from a new schedule */
	int 	(*drop_endpoint)(struct usb_hcd *, struct usb_device *, struct usb_host_endpoint *);
		/* Check that a new hardware configuration, set using
		 * endpoint_enable and endpoint_disable, does not exceed bus
		 * bandwidth.  This must be called before any set configuration
		 * or set interface requests are sent to the device.
		 */
	int	(*check_bandwidth)(struct usb_hcd *, struct usb_device *);
		/* Reset the device schedule to the last known good schedule,
		 * which was set from a previous successful call to
		 * check_bandwidth().  This reverts any add_endpoint() and
		 * drop_endpoint() calls since that last successful call.
		 * Used for when a check_bandwidth() call fails due to resource
		 * or bandwidth constraints.
		 */
	void	(*reset_bandwidth)(struct usb_hcd *, struct usb_device *);
		/* Returns the hardware-chosen device address */
	int	(*address_device)(struct usb_hcd *, struct usb_device *udev);
};

extern int usb_hcd_link_urb_to_ep(struct usb_hcd *hcd, struct urb *urb);
extern int usb_hcd_check_unlink_urb(struct usb_hcd *hcd, struct urb *urb,
		int status);
extern void usb_hcd_unlink_urb_from_ep(struct usb_hcd *hcd, struct urb *urb);

extern int usb_hcd_submit_urb(struct urb *urb, gfp_t mem_flags);
extern int usb_hcd_unlink_urb(struct urb *urb, int status);
extern void usb_hcd_giveback_urb(struct usb_hcd *hcd, struct urb *urb,
		int status);
extern void usb_hcd_flush_endpoint(struct usb_device *udev,
		struct usb_host_endpoint *ep);
extern void usb_hcd_disable_endpoint(struct usb_device *udev,
		struct usb_host_endpoint *ep);
extern void usb_hcd_reset_endpoint(struct usb_device *udev,
		struct usb_host_endpoint *ep);
extern void usb_hcd_synchronize_unlinks(struct usb_device *udev);
extern int usb_hcd_check_bandwidth(struct usb_device *udev,
		struct usb_host_config *new_config,
		struct usb_interface *new_intf);
extern int usb_hcd_get_frame_number(struct usb_device *udev);

extern struct usb_hcd *usb_create_hcd(const struct hc_driver *driver,
		struct device *dev, const char *bus_name);
extern struct usb_hcd *usb_get_hcd(struct usb_hcd *hcd);
extern void usb_put_hcd(struct usb_hcd *hcd);
extern int usb_add_hcd(struct usb_hcd *hcd,
		unsigned int irqnum, unsigned long irqflags);
extern void usb_remove_hcd(struct usb_hcd *hcd);

struct platform_device;
extern void usb_hcd_platform_shutdown(struct platform_device *dev);

#ifdef CONFIG_PCI
struct pci_dev;
struct pci_device_id;
extern int usb_hcd_pci_probe(struct pci_dev *dev,
				const struct pci_device_id *id);
extern void usb_hcd_pci_remove(struct pci_dev *dev);
extern void usb_hcd_pci_shutdown(struct pci_dev *dev);

#ifdef CONFIG_PM_SLEEP
extern struct dev_pm_ops	usb_hcd_pci_pm_ops;
#endif
#endif /* CONFIG_PCI */

/* pci-ish (pdev null is ok) buffer alloc/mapping support */
int hcd_buffer_create(struct usb_hcd *hcd);
void hcd_buffer_destroy(struct usb_hcd *hcd);

void *hcd_buffer_alloc(struct usb_bus *bus, size_t size,
	gfp_t mem_flags, dma_addr_t *dma);
void hcd_buffer_free(struct usb_bus *bus, size_t size,
	void *addr, dma_addr_t dma);

/* generic bus glue, needed for host controllers that don't use PCI */
extern irqreturn_t usb_hcd_irq(int irq, void *__hcd);

extern void usb_hc_died(struct usb_hcd *hcd);
extern void usb_hcd_poll_rh_status(struct usb_hcd *hcd);

/* The D0/D1 toggle bits ... USE WITH CAUTION (they're almost hcd-internal) */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> (ep)) & 1)
#define	usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << (ep)))
#define usb_settoggle(dev, ep, out, bit) \
		((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << (ep))) | \
		 ((bit) << (ep)))

/* -------------------------------------------------------------------------- */

/* Enumeration is only for the hub driver, or HCD virtual root hubs */
extern struct usb_device *usb_alloc_dev(struct usb_device *parent,
					struct usb_bus *, unsigned port);
extern int usb_new_device(struct usb_device *dev);
extern void usb_disconnect(struct usb_device **);

extern int usb_get_configuration(struct usb_device *dev);
extern void usb_destroy_configuration(struct usb_device *dev);

/*-------------------------------------------------------------------------*/

/*
 * HCD Root Hub support
 */

#include "hub.h"

/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE)<<8)
#define DeviceOutRequest \
	((USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_DEVICE)<<8)

#define InterfaceRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)

#define EndpointRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)
#define EndpointOutRequest \
	((USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)

/* class requests from the USB 2.0 hub spec, table 11-15 */
/* GetBusState and SetHubDescriptor are optional, omitted */
#define ClearHubFeature		(0x2000 | USB_REQ_CLEAR_FEATURE)
#define ClearPortFeature	(0x2300 | USB_REQ_CLEAR_FEATURE)
#define GetHubDescriptor	(0xa000 | USB_REQ_GET_DESCRIPTOR)
#define GetHubStatus		(0xa000 | USB_REQ_GET_STATUS)
#define GetPortStatus		(0xa300 | USB_REQ_GET_STATUS)
#define SetHubFeature		(0x2000 | USB_REQ_SET_FEATURE)
#define SetPortFeature		(0x2300 | USB_REQ_SET_FEATURE)


/*-------------------------------------------------------------------------*/

/*
 * Generic bandwidth allocation constants/support
 */
#define FRAME_TIME_USECS	1000L
#define BitTime(bytecount) (7 * 8 * bytecount / 6) /* with integer truncation */
		/* Trying not to use worst-case bit-stuffing
		 * of (7/6 * 8 * bytecount) = 9.33 * bytecount */
		/* bytecount = data payload byte count */

#define NS_TO_US(ns)	((ns + 500L) / 1000L)
			/* convert & round nanoseconds to microseconds */


/*
 * Full/low speed bandwidth allocation constants/support.
 */
#define BW_HOST_DELAY	1000L		/* nanoseconds */
#define BW_HUB_LS_SETUP	333L		/* nanoseconds */
			/* 4 full-speed bit times (est.) */

#define FRAME_TIME_BITS			12000L	/* frame = 1 millisecond */
#define FRAME_TIME_MAX_BITS_ALLOC	(90L * FRAME_TIME_BITS / 100L)
#define FRAME_TIME_MAX_USECS_ALLOC	(90L * FRAME_TIME_USECS / 100L)

/*
 * Ceiling [nano/micro]seconds (typical) for that many bytes at high speed
 * ISO is a bit less, no ACK ... from USB 2.0 spec, 5.11.3 (and needed
 * to preallocate bandwidth)
 */
#define USB2_HOST_DELAY	5	/* nsec, guess */
#define HS_NSECS(bytes) (((55 * 8 * 2083) \
	+ (2083UL * (3 + BitTime(bytes))))/1000 \
	+ USB2_HOST_DELAY)
#define HS_NSECS_ISO(bytes) (((38 * 8 * 2083) \
	+ (2083UL * (3 + BitTime(bytes))))/1000 \
	+ USB2_HOST_DELAY)
#define HS_USECS(bytes) NS_TO_US (HS_NSECS(bytes))
#define HS_USECS_ISO(bytes) NS_TO_US (HS_NSECS_ISO(bytes))

extern long usb_calc_bus_time(int speed, int is_input,
			int isoc, int bytecount);

/*-------------------------------------------------------------------------*/

extern void usb_set_device_state(struct usb_device *udev,
		enum usb_device_state new_state);

/*-------------------------------------------------------------------------*/

/* exported only within usbcore */

extern struct list_head usb_bus_list;
extern struct mutex usb_bus_list_lock;
extern wait_queue_head_t usb_kill_urb_queue;

extern int usb_find_interface_driver(struct usb_device *dev,
	struct usb_interface *interface);

#define usb_endpoint_out(ep_dir)	(!((ep_dir) & USB_DIR_IN))

#ifdef CONFIG_PM
extern void usb_hcd_resume_root_hub(struct usb_hcd *hcd);
extern void usb_root_hub_lost_power(struct usb_device *rhdev);
extern int hcd_bus_suspend(struct usb_device *rhdev, pm_message_t msg);
extern int hcd_bus_resume(struct usb_device *rhdev, pm_message_t msg);
#else
static inline void usb_hcd_resume_root_hub(struct usb_hcd *hcd)
{
	return;
}
#endif /* CONFIG_PM */

/*
 * USB device fs stuff
 */

#ifdef CONFIG_USB_DEVICEFS

/*
 * these are expected to be called from the USB core/hub thread
 * with the kernel lock held
 */
extern void usbfs_update_special(void);
extern int usbfs_init(void);
extern void usbfs_cleanup(void);

#else /* CONFIG_USB_DEVICEFS */

static inline void usbfs_update_special(void) {}
static inline int usbfs_init(void) { return 0; }
static inline void usbfs_cleanup(void) { }

#endif /* CONFIG_USB_DEVICEFS */

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_USB_MON) || defined(CONFIG_USB_MON_MODULE)

struct usb_mon_operations {
	void (*urb_submit)(struct usb_bus *bus, struct urb *urb);
	void (*urb_submit_error)(struct usb_bus *bus, struct urb *urb, int err);
	void (*urb_complete)(struct usb_bus *bus, struct urb *urb, int status);
	/* void (*urb_unlink)(struct usb_bus *bus, struct urb *urb); */
};

extern struct usb_mon_operations *mon_ops;

static inline void usbmon_urb_submit(struct usb_bus *bus, struct urb *urb)
{
	if (bus->monitored)
		(*mon_ops->urb_submit)(bus, urb);
}

static inline void usbmon_urb_submit_error(struct usb_bus *bus, struct urb *urb,
    int error)
{
	if (bus->monitored)
		(*mon_ops->urb_submit_error)(bus, urb, error);
}

static inline void usbmon_urb_complete(struct usb_bus *bus, struct urb *urb,
		int status)
{
	if (bus->monitored)
		(*mon_ops->urb_complete)(bus, urb, status);
}

int usb_mon_register(struct usb_mon_operations *ops);
void usb_mon_deregister(void);

#else

static inline void usbmon_urb_submit(struct usb_bus *bus, struct urb *urb) {}
static inline void usbmon_urb_submit_error(struct usb_bus *bus, struct urb *urb,
    int error) {}
static inline void usbmon_urb_complete(struct usb_bus *bus, struct urb *urb,
		int status) {}

#endif /* CONFIG_USB_MON || CONFIG_USB_MON_MODULE */

/*-------------------------------------------------------------------------*/

/* hub.h ... DeviceRemovable in 2.4.2-ac11, gone in 2.4.10 */
/* bleech -- resurfaced in 2.4.11 or 2.4.12 */
#define bitmap 	DeviceRemovable


/*-------------------------------------------------------------------------*/

/* random stuff */

#define	RUN_CONTEXT (in_irq() ? "in_irq" \
		: (in_interrupt() ? "in_interrupt" : "can sleep"))


/* This rwsem is for use only by the hub driver and ehci-hcd.
 * Nobody else should touch it.
 */
extern struct rw_semaphore ehci_cf_port_reset_rwsem;

/* Keep track of which host controller drivers are loaded */
#define USB_UHCI_LOADED		0
#define USB_OHCI_LOADED		1
#define USB_EHCI_LOADED		2
extern unsigned long usb_hcds_loaded;

#endif /* __KERNEL__ */

#endif /* __USB_CORE_HCD_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /*
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file pmic/core/mc34704.c
 * @brief This file contains MC34704 specific PMIC code.
 *
 * @ingroup PMIC_CORE
 */

/*
 * Includes
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/mfd/mc34704/core.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>

#include "pmic.h"

/*
 * Globals
 */
static pmic_version_t mxc_pmic_version = {
	.id = PMIC_MC34704,
	.revision = 0,
};
static unsigned int events_enabled;
unsigned int active_events[MAX_ACTIVE_EVENTS];
struct i2c_client *mc34704_client;
static void pmic_trigger_poll(void);

#define MAX_MC34704_REG 0x59
static unsigned int mc34704_reg_readonly[MAX_MC34704_REG / 32 + 1] = {
	(1 << 0x03) || (1 << 0x05) || (1 << 0x07) || (1 << 0x09) ||
	    (1 << 0x0B) || (1 << 0x0E) || (1 << 0x11) || (1 << 0x14) ||
	    (1 << 0x17) || (1 << 0x18),
	0,
};
static unsigned int mc34704_reg_written[MAX_MC34704_REG / 32 + 1];
static unsigned char mc34704_shadow_regs[MAX_MC34704_REG - 1];
#define IS_READONLY(r) ((1 << ((r) % 32)) & mc34704_reg_readonly[(r) / 32])
#define WAS_WRITTEN(r) ((1 << ((r) % 32)) & mc34704_reg_written[(r) / 32])
#define MARK_WRITTEN(r) do { \
	mc34704_reg_written[(r) / 32] |= (1 << ((r) % 32)); \
} while (0)

int pmic_read(int reg_nr, unsigned int *reg_val)
{
	int c;

	/*
	 * Use the shadow register if we've written to it
	 */
	if (WAS_WRITTEN(reg_nr)) {
		*reg_val = mc34704_shadow_regs[reg_nr];
		return PMIC_SUCCESS;
	}

	/*
	 * Otherwise, actually read the real register.
	 * Write-only registers will read as zero.
	 */
	c = i2c_smbus_read_byte_data(mc34704_client, reg_nr);
	if (c == -1) {
		pr_debug("mc34704: error reading register 0x%02x\n", reg_nr);
		return PMIC_ERROR;
	} else {
		*reg_val = c;
		return PMIC_SUCCESS;
	}
}

int pmic_write(int reg_nr, const unsigned int reg_val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(mc34704_client, reg_nr, reg_val);
	if (ret == -1) {
		return PMIC_ERROR;
	} else {
		/*
		 * Update our software copy of the register since you
		 * can't read what you wrote.
		 */
		if (!IS_READONLY(reg_nr)) {
			mc34704_shadow_regs[reg_nr] = reg_val;
			MARK_WRITTEN(reg_nr);
		}
		return PMIC_SUCCESS;
	}
}

unsigned int pmic_get_active_events(unsigned int *active_events)
{
	unsigned int count = 0;
	unsigned int faults;
	int bit_set;

	/* Check for any relevant PMIC faults */
	pmic_read(REG_MC34704_FAULTS, &faults);
	faults &= events_enabled;

	/*
	 * Mask all active events, because there is no way to acknowledge
	 * or dismiss them in the PMIC -- they're sticky.
	 */
	events_enabled &= ~faults;

	/* Account for all unmasked faults */
	while (faults) {
		bit_set = ffs(faults) - 1;
		*(active_events + count) = bit_set;
		count++;
		faults ^= (1 << bit_set);
	}
	return count;
}

int pmic_event_unmask(type_event event)
{
	unsigned int event_bit = 0;
	unsigned int prior_events = events_enabled;

	event_bit = (1 << event);
	events_enabled |= event_bit;

	pr_debug("Enable Event : %d\n", event);

	/* start the polling task as needed */
	if (events_enabled && prior_events == 0)
		pmic_trigger_poll();

	return 0;
}

int pmic_event_mask(type_event event)
{
	unsigned int event_bit = 0;

	event_bit = (1 << event);
	events_enabled &= ~event_bit;

	pr_debug("Disable Event : %d\n", event);

	return 0;
}

/*!
 * PMIC event polling task.  This task is called periodically to poll
 * for possible MC34704 events (No interrupt supplied by the hardware).
 */
static void pmic_event_task(struct work_struct *work);
DECLARE_DELAYED_WORK(pmic_ws, pmic_event_task);

static void pmic_trigger_poll(void)
{
	schedule_delayed_work(&pmic_ws, HZ / 10);
}

static void pmic_event_task(struct work_struct *work)
{
	unsigned int count = 0;
	int i;

	count = pmic_get_active_events(active_events);
	pr_debug("active events number %d\n", count);

	/* call handlers for all active events */
	for (i = 0; i < count; i++)
		pmic_event_callback(active_events[i]);

	/* re-trigger this task, but only if somebody is watching */
	if (events_enabled)
		pmic_trigger_poll();

	return;
}

pmic_version_t pmic_get_version(void)
{
	return mxc_pmic_version;
}
EXPORT_SYMBOL(pmic_get_version);

int __devinit pmic_init_registers(void)
{
	/*
	 * Set some registers to what they should be,
	 * if for no other reason than to initialize our
	 * software register copies.
	 */
	CHECK_ERROR(pmic_write(REG_MC34704_GENERAL2, 0x09));
	CHECK_ERROR(pmic_write(REG_MC34704_VGSET1, 0));
	CHECK_ERROR(pmic_write(REG_MC34704_REG2SET1, 0));
	CHECK_ERROR(pmic_write(REG_MC34704_REG3SET1, 0));
	CHECK_ERROR(pmic_write(REG_MC34704_REG4SET1, 0));
	CHECK_ERROR(pmic_write(REG_MC34704_REG5SET1, 0));

	return PMIC_SUCCESS;
}

static int __devinit is_chip_onboard(struct i2c_client *client)
{
	int val;

	/*
	 * This PMIC has no version or ID register, so just see
	 * if it ACK's and returns 0 on some write-only register as
	 * evidence of its presence.
	 */
	val = i2c_smbus_read_byte_data(client, REG_MC34704_GENERAL2);
	if (val != 0)
		return -1;

	return 0;
}

static int __devinit pmic_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int ret = 0;
	struct mc34704 *mc34704;
	struct mc34704_platform_data *plat_data = client->dev.platform_data;

	if (!plat_data || !plat_data->init)
		return -ENODEV;

	ret = is_chip_onboard(client);

	if (ret == -1)
		return -ENODEV;

	mc34704 = kzalloc(sizeof(struct mc34704), GFP_KERNEL);
	if (mc34704 == NULL)
		return -ENOMEM;

	i2c_set_clientdata(client, mc34704);
	mc34704->dev = &client->dev;
	mc34704->i2c_client = client;

	mc34704_client = client;

	/* Initialize the PMIC event handling */
	pmic_event_list_init();

	/* Initialize PMI registers */
	if (pmic_init_registers() != PMIC_SUCCESS)
		return PMIC_ERROR;

	ret = plat_data->init(mc34704);
	if (ret != 0)
		return PMIC_ERROR;

	dev_info(&client->dev, "Loaded\n");

	return PMIC_SUCCESS;
}

static int pmic_remove(struct i2c_client *client)
{
	return 0;
}

static int pmic_suspend(struct i2c_client *client, pm_message_t state)
{
	return 0;
}

static int pmic_resume(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id mc34704_id[] = {
	{"mc34704", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, mc34704_id);

static struct i2c_driver pmic_driver = {
	.driver = {
		   .name = "mc34704",
		   .bus = NULL,
		   },
	.probe = pmic_probe,
	.remove = pmic_remove,
	.suspend = pmic_suspend,
	.resume = pmic_resume,
	.id_table = mc34704_id,
};

static int __init pmic_init(void)
{
	return i2c_add_driver(&pmic_driver);
}

static void __exit pmic_exit(void)
{
	i2c_del_driver(&pmic_driver);
}

/*
 * Module entry points
 */
subsys_initcall_sync(pmic_init);
module_exit(pmic_exit);

MODULE_DESCRIPTION("MC34704 PMIC driver");
MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * File: wpa.h
 *
 * Purpose: Defines the macros, types, and functions for dealing
 *          with WPA informations.
 *
 * Author: Kyle Hsu
 *
 * Date: Jul 14, 2003
 *
 */

#ifndef __WPA_H__
#define __WPA_H__


#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__80211HDR_H__)
#include "80211hdr.h"
#endif


/*---------------------  Export Definitions -------------------------*/

#define WPA_NONE            0
#define WPA_WEP40           1
#define WPA_TKIP            2
#define WPA_AESWRAP         3
#define WPA_AESCCMP         4
#define WPA_WEP104          5
#define WPA_AUTH_IEEE802_1X 1
#define WPA_AUTH_PSK        2

#define WPA_GROUPFLAG       0x02
#define WPA_REPLAYBITSSHIFT 2
#define WPA_REPLAYBITS      0x03

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Types  ------------------------------*/


/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

VOID
WPA_ClearRSN(
    IN PKnownBSS        pBSSList
    );

VOID
WPA_ParseRSN(
    IN PKnownBSS        pBSSList,
    IN PWLAN_IE_RSN_EXT pRSN
    );

BOOL
WPA_SearchRSN(
    BYTE                byCmd,
    BYTE                byEncrypt,
    IN PKnownBSS        pBSSList
    );

BOOL
WPAb_Is_RSN(
    IN PWLAN_IE_RSN_EXT pRSN
    );

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */


#endif // __WPA_H__
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     trên Thêm tùy chọn Xong 
Xem tất cả Chọn một ứng dụng Xóa truy vấn Tìm kiếm truy vấn Tìm kiếm Gửi truy vấn Tìm kiếm bằng giọng nói Chia sẻ với Chia sẻ với %s 	Thu gọn 
Giới thiệu  T.vụ tài khoản là b.buộc ����Cho phép ứng dụng nhận thông tin cập nhật định kỳ về mức hoạt động của bạn từ Google, chẳng hạn như nếu bạn đang đi bộ, lái xe, đạp xe hoặc đứng yên. nhận dạng hoạt động y��Cho phép ứng dụng nhận thông báo khi ID quảng cáo hoặc tùy chọn giới hạn theo dõi quảng cáo của người dùng được cập nhật.  Thông báo về ID quảng cáo Thêm Thêm địa điểm +BKéo bản đồ để xác định vị trí điểm đánh dấu Hủy 	Tiếp theo Chọn ứng dụng Quảng cáo bởi Google /;Chọn không tham gia quảng cáo dựa trên sở thích y��Hướng dẫn các ứng dụng không sử dụng ID quảng cáo của bạn để tạo hồ sơ hoặc hiển thị cho bạn quảng cáo dựa trên sở thích. Đặt lại ID quảng cáo IaThao tác này sẽ thay thế ID quảng cáo của bạn bằng một số ngẫu nhiên mới. Đặt lại ID quảng cáo? ID quảng cáo của bạn: 
Đồng ý Chỉnh sửa bí danh "Chính sách bảo mật Bổ sung Vị trí Mạng Yêu cầu đăng nhập "Yêu cầu đăng nhập bằng %s ����Bạn sắp xóa tất cả dữ liệu ứng dụng bên thứ ba được lưu trữ với tài khoản Google này. Dữ liệu này sẽ không còn khả dụng trên bất kỳ thiết bị nào của bạn. ����Bạn sắp xóa tất cả dữ liệu ứng dụng bên thứ ba được lưu trữ bằng tài khoản Google %1$s của mình. Dữ liệu này sẽ không còn có trên bất kỳ thiết bị nào của bạn. Xóa dữ liệu ứng dụng? !Không xóa được dữ liệu. &Đang xóa dữ liệu ứng dụng… Dữ liệu ứng dụng Tài khoản đã có Gmail. Tài khoản Google 	Quảng cáo 1CHiện có quyền truy cập vào dữ liệu của bạn tại %s %Trình quản lý tài khoản Google g��%s và Google sẽ sử dụng thông tin này theo điều khoản dịch vụ và chính sách bảo mật tương ứng của mình. ỨNG DỤNG NÀY MUỐN ����Bằng cách tiếp tục, bạn cho phép ứng dụng này và Google sử dụng thông tin của bạn theo điều khoản dịch vụ và chính sách bảo mật tương ứng. ����Bằng cách chạm vào Đăng nhập, bạn cho phép ứng dụng này và Google sử dụng thông tin của bạn theo điều khoản dịch vụ và chính sách bảo mật tương ứng. #Hãy đăng nhập %s bằng Google Cho phép đăng nhập Đã phê duyệt tác vụ Đã hủy tác vụ "Được đặt lại mật khẩu Đóng %Đã khởi động lại thông báo 	Cho phép />Cho phép đăng nhập vào tài khoản của bạn từ %s? )Chúng tôi cần xác nhận của bạn 6LSố đã chọn không chính xác. Yêu cầu của bạn đã bị hủy. )chi tiết yêu cầu không xác định (6Thông báo ủy quyền của Google đã hết hạn  +Đã xảy ra lỗi. Vui lòng thử lại. Biểu trưng Google Đã phê duyệt đăng nhập Đã hủy đăng nhập Đăng nhập từ %s? !*Google đã chặn hành vi đăng nhập. Số không đúng ^}Chạm vào số được hiển thị trên màn hình khác. Thao tác này sẽ giúp bạn đăng nhập vào máy tính. k��Ai đó vừa cố đăng nhập vào tài khoản Google của bạn bằng mật khẩu của bạn. Google đã chặn nỗ lực đăng nhập. Chặn đăng nhập Cố gắng đăng nhập? Không QoGoogle sẽ được thông báo rằng có người đã cố gắng truy cập vào tài khoản của bạn. ����Google đã nhận được yêu cầu thay đổi mật khẩu của bạn. Do yêu cầu đó không được gửi từ bạn nên yêu cầu đã bị hủy. Không có thay đổi nào được thực hiện đối với tài khoản của bạn. Ảnh tiểu sử /AChọn tài khoản bạn đang cố gắng đăng nhập bằng: &Bạn đang cố gắng đăng nhập? #-Không tìm thấy thông báo đăng nhập. 
Lỗi nội bộ 2BTrên máy tính của bạn, nhấp vào "Có mã?" rồi nhập: %2Bạn vừa cố gắng đăng nhập vào Google? +Đã phê duyệt đặt lại mật khẩu $Đã hủy đặt lại mật khẩu  ,Cho phép đặt lại mật khẩu từ %s? (6Google đã chặn hành vi đặt lại mật khẩu. Số không chính xác RpChạm vào số hiển thị trên màn hình khác. Thao tác này sẽ đặt lại mật khẩu của bạn. #Cho phép đặt lại mật khẩu s��Một người nào đó đã yêu cầu đặt lại mật khẩu tài khoản Google của bạn. Google đã chặn hành vi đặt lại mật khẩu này. %Đã chặn đặt lại mật khẩu :RBạn đã yêu cầu đặt lại mật khẩu của mình từ %s phải không? 4KBạn đã cố gắng đặt lại mật khẩu của mình phải không? Không thể khóa màn hình q��Hãy đảm bảo rằng đó chính là bạn... <br/>Google sẽ khóa thiết bị của bạn. Mở khóa thiết bị để tiếp tục đăng nhập. Mở khóa để tiếp tục Bảo vệ t.khoản &5Không thể gửi thư trả lời tới máy chủ. &Đang gửi trả lời cho máy chủ $.Đang tìm kiếm thông báo đăng nhập... chưa đầy %s phút trước 
Vừa xong Vui lòng đợi... %Thông báo ủy quyền của Google Có 
Quay lại Sử dụng tên này? (Thêm làm thiết bị đáng tin cậy (Xác nhận màn hình khóa của bạn o��Đã ghép nối thiết bị bluetooth mới.
Thêm %s làm thiết bị đáng tin cậy để bỏ qua khóa màn hình khi được kết nối. x��Đã ghép nối thiết bị bluetooth mới.
Thêm thiết bị đó làm thiết bị đáng tin cậy để bỏ qua khóa màn hình khi được kết nối. #.Đã ghép nối thiết bị bluetooth mới. )Nhập văn bản bạn thấy ở đây: $Cần nhập HÌNH ẢNH XÁC THỰC u��Thiết bị của bạn đang kết nối với Google để lưu thông tin vào tài khoản của bạn. Quá trình này có thể mất tới 5 phút. &Trạng thái thông tin đăng nhập +:Đồng bộ hóa thông tin đăng nhập ngoại tuyến Chạm và Chuyển Khóa và xác minh 8KSao chép tài khoản và dữ liệu vào thiết bị khác của bạn? TqSau khi bạn xác minh, Bluetooth sẽ được bật để các thiết bị của bạn có thể liên lạc. DVSau khi bạn xác minh, các thiết bị của bạn sẽ kết nối qua Bluetooth. Y}Để tiếp tục, màn hình của bạn sẽ bị khóa và bạn sẽ cần phải mở khóa màn hình để xác minh. AUKhông thể Chạm và Chuyển vì không có tài khoản trên thiết bị này. KbKhông thể Chạm và Chuyển vì Bluetooth không khả dụng trên thiết bị của bạn. >XBluetooth sẽ được bật để các thiết bị của bạn có thể liên lạc. "Việc này sử dụng Bluetooth. %Chạm và Chuyển tới hoàn tất $Đang sao chép từ thiết bị... .>Sao chép tài khoản và dữ liệu từ thiết bị này? MkKhông thể sử dụng tính năng Nhấn và khôi phục từ hồ sơ người dùng bị hạn chế. Khôi phục tài khoản  Thiết bị không xác định Đang thêm tài khoản... $3Kết nối với thiết bị khác của bạn... Đã mất kết nối �
�YSao chép nhanh mọi tài khoản Google, ứng dụng và dữ liệu đã sao lưu từ thiết bị Android hiện có của bạn. Để sao chép: 

1. Bạn hãy nhớ bật và mở khóa thiết bị khác của mình. 

2. Đặt nhanh hai thiết bị chạm lưng cho đến khi bạn nghe thấy âm báo, sau đó đặt sang bên. ����Tính năng này sử dụng NFC để chuyển dữ liệu tài khoản. Nếu thiết bị khác của bạn hỗ trợ NFC, đảm bảo thiết bị được bật trong Cài đặt > Không dây & mạng > Khác. Bỏ qua Không bỏ qua d��Chạm và Chuyển sẽ không sao chép tài khoản này hoặc ứng dụng đã sao lưu và dữ liệu vào thiết bị này. 
Vẫn bỏ qua (Bạn muốn bỏ qua tài khoản này? !Rất tiếc, đã xảy ra lỗi :QĐể tiếp tục, hãy chạm vào OK trên thiết bị còn lại của bạn Thử lại )Kiểm tra thiết bị khác của bạn  +Đang tải tài khoản và dữ liệu... p��Tài khoản này đã bị vô hiệu hóa. 

(Để biết chi tiết, truy cập http://google.com/accounts bằng trình duyệt web). ��2Tài khoản này chưa được xác minh. Hãy kiểm tra email của bạn và nhấp vào liên kết được gửi kèm để xác minh tài khoản này. 

(Để khôi phục thông tin tài khoản của bạn, hãy truy cập http://www.google.com/accounts/recovery trên máy tính của bạn). ?XMật khẩu quá yếu hoặc chứa các ký tự không hợp lệ. Hãy thử lại. &Tên người dùng không hợp lệ. H]Đã xảy ra sự cố khi giao tiếp với các máy chủ Google. 

Hãy thử lại sau. YwSử dụng mật khẩu dành riêng cho ứng dụng thay vì mật khẩu tài khoản thông thường của bạn. ����Tên người dùng và mật khẩu của bạn không khớp. Thử lại.

Để khôi phục thông tin tài khoản của bạn, truy cập google.com/accounts/recovery/. l��Để truy cập vào tài khoản, bạn phải đăng nhập trên web. Chạm vào Tiếp theo để bắt đầu đăng nhập Trình duyệt. h��Bạn không có kết nối mạng. 

Đây có thể là sự cố tạm thời. Hãy thử lại hoặc kết nối với mạng Wi-Fi khác. #Người dùng chưa đăng nhập. (Tên người dùng không khả dụng. 1?Đã có một tài khoản ^1 trên thiết bị Android này. Tài khoản đã tồn tại #Thoát đăng nhập trình duyệt  +Ứng dụng này có thể xem những gì ,<Thay đổi những người ứng dụng này có thể xem 1DTính năng bảo vệ thiết bị sẽ không còn hoạt động. Bạn có chắc chắn không? (Trường không được để trống. Chi tiết và cài đặt p��Biết hồ sơ cơ bản của bạn, danh sách mọi người trong vòng kết nối của bạn và quản lý dữ liệu hoạt động trò chơi. =PCung cấp hoạt động trò chơi của bạn qua Google, hiển thị với: \{Không bao giờ bị mất dữ liệu của bạn nữa. Tài khoản Google giúp lưu trữ mọi dữ liệu an toàn. Đặt làm Google  Được cung cấp bởi Google AXBạn có muốn thêm tài khoản hiện tại hoặc tạo tài khoản mới không? WnBạn có tài khoản Google không?

Nếu bạn sử dụng Gmail hoặc Google Apps, hãy trả lời Có. #Bạn đã có tài khoản Google? ����Nếu bạn mất mật khẩu, bạn có thể trả lời câu hỏi bảo mật và nhận liên kết đặt lại mật khẩu được gửi qua email tới một địa chỉ khác. Hoàn tất gia nhập Google+ &2Hoàn tất quá trình thiết lập tài khoản Thêm tài khoản Google )Tên người dùng được đề xuất �@��Sử dụng tài khoản Google của bạn để sao lưu ứng dụng, cài đặt (chẳng hạn như dấu trang và mật khẩu Wi-Fi) và dữ liệu khác của bạn. Nếu trước đây bạn đã sao lưu tài khoản của mình thì bây giờ bạn có thể khôi phục tài khoản đó tới điện thoại này.

Bạn có thể thay đổi cài đặt sao lưu của mình bất kỳ lúc nào trong Cài đặt. �΁Sử dụng tài khoản Google của bạn để sao lưu ứng dụng, cài đặt (chẳng hạn như dấu trang và mật khẩu Wi-Fi) và dữ liệu khác của bạn.

Bạn có thể thay đổi cài đặt sao lưu của mình bất kỳ lúc nào trong Cài đặt. 9ILuôn sao lưu điện thoại này bằng tài khoản Google của tôi &Đang kiểm tra tính khả dụng… ����Điện thoại của bạn đang kết nối với Google để xem tên người dùng mà bạn mong muốn có khả dụng hay không.

Quá trình này có thể mất một vài phút. Mật khẩu Vòng kết nối <QCách dễ nhất để chia sẻ trực tuyến những gì quan trọng nhất. Nhập lại mật khẩu Nhận tài khoản Chọn tên người dùng ��Tài khoản Google của bạn đi cùng với Gmail, email từ Google. Tài khoản hoạt động trên mọi thiết bị, chặn spam và nhiều hơn nữa. Đang lưu tài khoản... &Đang tạo tiểu sử của bạn... Không thể cắt ảnh. ����Quản trị viên tại %s sở hữu và quản lý tất cả các dữ liệu của bạn trên Google+ và có thể xóa hoặc sửa đổi tiểu sử của bạn bất cứ lúc nào mà không cần sự cho phép của bạn. 	Kích hoạt 	Tải xuống /FChạm vào bất kỳ hộp văn bản nào để bắt đầu nhập tối thiểu 8 ký tự Bật lịch sử web p��Để truy cập vào tài khoản, bạn phải đăng nhập trên web. Chạm vào Tiếp theo để bắt đầu đăng nhập vào Trình duyệt. 
Hiện có CTLiên tục cập nhật cho tôi tin tức và các khuyến mãi từ Google Play. CXNhắn tin theo nhóm cho tất cả mọi người trong vòng kết nối của bạn. ?RĐịa chỉ email bạn đã nhập không hợp lệ.
 Ví dụ: joe@example.com 8QĐịa chỉ khôi phục không được giống địa chỉ bạn đang tạo. 'Giới thiệu về tài khoản Google �
�XLịch sử web điều chỉnh kết quả tìm kiếm dựa trên những gì bạn đã tìm kiếm và các trang web mà bạn đã truy cập, cung cấp trải nghiệm Google được cá nhân hóa. 

Để xem và quản lý lịch sử web của bạn, bao gồm cả cách tắt hoặc bật, hãy xem http://www.google.com/history. #Giới thiệu về lịch sử web Đang kiểm tra thông tin… Hãy đợi giây lát… UyTên của bạn được sử dụng để cá nhân hóa điện thoại của bạn và các dịch vụ của Google. >ZĐiện thoại sử dụng tên của bạn để cá nhân hóa một số ứng dụng. $Điện thoại này thuộc về... Tên trên Google+ của bạn 5IKhông thể thiết lập kết nối đáng tin cậy tới máy chủ. ����Đây có thể là sự cố tạm thời hoặc có thể thiết bị Android của bạn không được cấp quyền cho dịch vụ dữ liệu. Nếu sự cố vẫn tiếp diễn, hãy gọi Trung tâm chăm sóc khách hàng. OoĐây có thể là sự cố tạm thời. Hãy thử lại hoặc kết nối với một mạng Wi-Fi khác. Mới )Bạn không có kết nối mạng nào. ���	Đây có thể là sự cố tạm thời hoặc thiết bị Android của bạn có thể không được cấp quyền đối với dịch vụ dữ liệu. Hãy thử lại khi được kết nối với mạng di động hoặc kết nối với mạng Wi-Fi. Không phải bây giờ !-Mật khẩu không khớp. Hãy thử lại. Tạo mật khẩu Chụp ảnh Ảnh của bạn Không thể chạy máy ảnh. l��Ảnh này sẽ được sử dụng trong hồ sơ liên hệ của bạn. Bạn có thể thay đổi ảnh sau này trong ứng dụng Liên hệ. QqChụp ảnh để bạn bè của bạn thấy trên Google+. Bạn có thể chọn một ảnh mới sau đó. Tham gia Google+ ����Tài khoản Google của bạn đã được tạo nhưng yêu cầu tham gia Google+ của bạn chưa được chấp thuận. Hãy thử lại sau từ bất kỳ trình duyệt web nào tại plus.google.com. y��Yêu cầu tham gia Google+ của bạn chưa được chấp thuận. Hãy thử lại sau từ bất kỳ trình duyệt web nào tại plus.google.com. Tham gia Google+ sau �삗Trên Google+, bạn sẽ có tiểu sử công khai để giúp bạn bè của bạn tìm thấy bạn trực tuyến. Bất kỳ ai biết địa chỉ email của bạn cũng có thể tìm thấy và xem tiểu sử của bạn.

Bạn có thể sử dụng nút +1 trên toàn bộ web để đề xuất các nội dung với mọi người. Tiểu sử và +1 của bạn sẽ xuất hiện công khai trong tìm kiếm, trên quảng cáo và trên web.

Từ bây giờ, tiểu sử của bạn sẽ chỉ hiển thị tên bạn (%s %s). Bạn có thể thêm ảnh, tiểu sử và các chi tiết khác sau khi bạn bắt đầu sử dụng Google+. ,6Quá trình này có thể diễn ra trong vài phút. "Đang kết nối với Google+... IZGoogle+ giúp chia sẻ trên web giống với chia sẻ trong cuộc sống thực hơn. Không phải là %s? Xin chào %s! 'Nhập mật khẩu của bạn cho:
%s Thông tin khôi phục AVKhôi phục dữ liệu từ tài khoản Google của tôi sang điện thoại này y��Điện thoại của bạn đang kết nối với Google để khôi phục tài khoản của bạn. Quá trình này có thể mất một khoảng thời gian. Đang khôi phục… Chụp lại ảnh Địa chỉ email khôi phục Câu hỏi bảo mật 1@Đã xảy ra sự cố khi giao tiếp với máy chủ Google. Thử lại sau. 	Rất tiếc… Thay đổi tên người dùng "Chạm để xem các đề xuất ",Đăng nhập vào tài khoản thành công Sao lưu và khôi phục Sao lưu Đồng bộ hóa tài khoản Đồng bộ hóa %s Quá ngắn Đang xác thực… Đang chuẩn bị… Tải lên tức thì >\Dễ dàng đồng bộ hóa tất cả ảnh của bạn từ tất cả các thiết bị. !Xác minh tiểu sử của bạn WrĐang thu thập thông tin về điện thoại của bạn. Quá trình này có thể mất tối đa 30 giây. Thiết lập tài khoản Bảo mật Đăng nhập bằng Google+ (9Chạm để đăng nhập vào tài khoản của bạn. !+Đã yêu cầu quyền
cho tài khoản %s ?PKý tự không hợp lệ. Chỉ cho phép 0-9, a-z, A-Z và dấu chấm câu. (Ký tự mật khẩu không hợp lệ. CZPhạm vi không hợp lệ được sử dụng trong quá trình yêu cầu xác thực. 9NNội dung bạn đã nhập không phải là tên người dùng hợp lệ. ����Cho phép gửi dữ liệu vị trí ẩn danh cho Google. Đồng thời cho phép các ứng dụng được phép sử dụng dữ liệu từ nguồn như Wi-Fi và mạng di động để xác định vị trí gần đúng của bạn. "Dịch vụ vị trí của Google Cài đặt vị trí Đang đăng nhập… 	Đăng nhập Đang đăng nhập Nhập email và mật khẩu Maps và Latitude !Thiết lập tài khoản Google Tên của bạn 5IKhông thể thiết lập kết nối đáng tin cậy với máy chủ. 4GHãy bảo vệ thẻ này. Kẻ tấn công có thể sao chép thẻ. %1$s đã được thêm vào ,?Thiết bị NFC được cung cấp không được hỗ trợ Chỉ bạn 
Mã bảo mật #2Đã xảy ra sự cố khi tạo mã bảo mật. <RKhông có tài khoản nào được định cấu hình trên thiết bị này. Đang tạo mã bảo mật. Mã bảo mật cho %s: Chọn tài khoản "Khi nào tôi sẽ cần mã này?  Google có thể xem những gì ����Tài khoản Google của bạn đã được tạo nhưng yêu cầu tạo tiểu sử trên Google+ của bạn chưa được chấp thuận. Hãy thử lại sau từ bất kỳ trình duyệt web nào tại plus.google.com. #Tạo tiểu sử trên Google+ sau (Chào mừng bạn đến với Google+! Tạo tiểu sử trên Google+ Đã khóa Chromebook Đã khóa Chromebook. Đã mở khóa Chromebook MmChromebook sẽ vẫn được mở khóa chừng nào điện thoại này được mở khóa và ở gần 0@Điện thoại này có thể mở khóa Chromebook của bạn \��Từ bây giờ, Chromebook của bạn sẽ vẫn được mở khóa khi điện thoại này được mở khóa và ở gần. $Smart Lock cho Chrome được bật 6GTìm thấy điện thoại. Tiếp tục trên Chromebook của bạn. u��Sau khi hoàn tất thiết lập trên Chromebook, bạn sẽ sử dụng điện thoại này để mở khóa Chromebook — không cần mật khẩu. #Thiết lập Smart Lock cho Chrome Đang tải... 	Đang gửi… 	Chỉnh sửa 
Biểu tượng 	
Thông tin Thông tin khác Cảnh báo Các dịch vụ của Google ���dThiết bị này cũng có thể nhận và cài đặt bản cập nhật và ứng dụng từ Google, nhà cung cấp dịch vụ và nhà sản xuất thiết bị của bạn. Một số ứng dụng có thể được tải xuống và cài đặt nếu bạn tiếp tục, đồng thời bạn có thể xóa ứng dụng đó bất cứ lúc nào. ��CKhi bạn bật tính năng sao lưu tự động, dữ liệu của thiết bị được định kỳ lưu vào tài khoản Google của bạn. Bản sao lưu có thể bao gồm thông tin trong các thư mục dữ liệu ứng dụng của bạn, như tùy chọn, cài đặt, tin nhắn, ảnh và các tệp khác. OnDữ liệu bao gồm ứng dụng, cài đặt ứng dụng, cài đặt hệ thống và mật khẩu Wi-Fi. Liên lạc ��sDịch vụ vị trí của Google sử dụng Wi-Fi và các tín hiệu khác để xác định vị trí chính xác và nhanh hơn, thường sử dụng ít năng lượng hơn GPS. Một số dữ liệu có thể được lưu trữ trên thiết bị của bạn. Dữ liệu có thể được thu thập ngay cả khi không có ứng dụng nào đang chạy. 	Vị trí ��� Dịch vụ vị trí của Google sử dụng các nguồn như Wi-Fi và mạng di động để giúp ước đoán vị trí thiết bị của bạn nhanh hơn và chính xác hơn. Khi bạn bật dịch vụ vị trí của Google, thiết bị của bạn sẽ dùng chế độ sử dụng Wi-Fi và mạng di động để cung cấp thông tin vị trí. Bạn có thể tắt dịch vụ này trong cài đặt vị trí bằng cách chọn chế độ chỉ sử dụng GPS, đôi khi được gọi chế độ chỉ Thiết bị. ��Đây là thông tin chung về thiết bị của bạn cũng như cách bạn sử dụng thiết bị, chẳng hạn như mức pin, tần suất bạn sử dụng ứng dụng, chất lượng và thời lượng kết nối mạng của bạn (như sóng di động, Wi-Fi và Bluetooth) và báo cáo sự cố khi các tính năng không hoạt động như mong muốn. Thông tin này sẽ được sử dụng để cải thiện các sản phẩm và dịch vụ của Google dành cho tất cả mọi người và sẽ không được liên kết với Tài khoản Google của bạn. Một số thông tin tổng hợp cũng sẽ giúp các đối tác, chẳng hạn như các nhà phát triển Android, cải thiện ứng dụng và sản phẩm của họ.

Bạn có thể bật hoặc tắt tính năng này bất cứ lúc nào. Tính năng này không ảnh hưởng đến khả năng thiết bị gửi thông tin cần thiết để nhận các dịch vụ quan trọng như bảo mật và cập nhật hệ thống. #Trình hướng dẫn thiết lập Không thể đăng nhập Lỗi Đăng nhập trình duyệt (5Đăng nhập bằng tài khoản Google của bạn: &Đã xảy ra lỗi. Hãy thử lại. C\Để thiết lập, hãy nhấn vào thiết bị hoặc thẻ NFC trên thiết bị này. 	Thêm NFC #THÊM THIẾT BỊ ĐÁNG TIN CẬY #Thêm thiết bị đáng tin cậy ,Địa điểm này đã được tin cậy. %Thêm địa điểm đáng tin cậy Cài đặt NFC .=Được kết nối an toàn với thiết bị Bluetooth %s �y�Tính năng này sử dụng gia tốc kế trên thiết bị của bạn để phát hiện xem thiết bị có vẫn đang được mang trên cơ thể hay không. 

Nếu thiết bị của bạn phát hiện ra rằng hiện bạn không còn giữ thiết bị, thiết bị sẽ vẫn không được mở khóa. 

Lưu ý: Nếu bạn mở khóa thiết bị và đưa cho một người khác, thiết bị cũng vẫn được mở khóa miễn là người đó tiếp tục giữ hoặc mang thiết bị. Phát hiện trên cơ thể #Bị tắt bởi quản trị viên &Bật NFC để thêm thiết bị NFC AXtrong khi kết nối với %s mà bạn đã đặt làm thiết bị đáng tin cậy. TuĐể buộc thiết bị của bạn khóa, hãy tắt rồi bật màn hình và chạm vào biểu tượng khóa. .:trong khi thiết bị đã được mang theo bên mình. 7Ltrong khi ở %s mà bạn đã đặt làm địa điểm đáng tin cậy.  1Thiết bị của bạn đã được mở khóa Tên (5Đã tìm thấy NFC
Hãy nhập tên cho thẻ đó. $Bị tắt bởi quản trị viên. ��sChỉ cần mở khóa một lần và thiết bị của bạn sẽ vẫn được mở khóa miễn là bạn luôn giữ hoặc mang theo thiết bị bên mình. Ví dụ:

- Cầm thiết bị trên tay
- Mang thiết bị trong túi hoặc túi xách 

Bất kỳ lúc nào bạn đặt thiết bị xuống, thiết bị sẽ khóa vì không ở bên bạn nữa. &3Giữ thiết bị luôn mở khóa khi bên bạn. Trợ giúp Tắt Bật 'Cải thiện đối sánh khuôn mặt e��Thiết bị này sẽ không nhận dạng được khuôn mặt bạn nữa và tính năng khuôn mặt đáng tin cậy sẽ bị tắt. Đặt lại +Đặt lại khuôn mặt đáng tin cậy? *Đặt lại khuôn mặt đáng tin cậy Khuôn mặt đáng tin cậy "1Chưa được hỗ trợ trên thiết bị này 
Đã kết nối 
Không có Chưa kết nối Thiết bị tin cậy Địa điểm tùy chỉnh Từ Google Maps Màn hình chính *9Để đặt, hãy chuyển tới cài đặt Google Maps +:Đặt địa điểm được tin cậy trong Google Maps Địa điểm tin cậy Cơ quan Giọng nói đáng tin cậy &3Không ghép nối được. Vui lòng thử lại.  Không thể th.lập t.bị NFC ����Ví dụ:

- Đồng hồ Bluetooth của bạn
- Hệ thống Bluetooth trên ô tô của bạn 

Lưu ý: Kết nối Bluetooth có phạm vi tối đa là 100 mét. SpThêm thiết bị Bluetooth đáng tin cậy để thiết bị này luôn mở khóa khi được kết nối. Đã tắt Bluetooth Thêm thiết bị tin cậy Bluetooth đã tắt. Chọn thiết bị Chọn loại thiết bị ����Ví dụ:

- Đồng hồ Bluetooth của bạn
- Hệ thống Bluetooth trên ô tô của bạn 
- Hình dán NFC trên đế điện thoại trong ô tô của bạn 

Lưu ý: Kết nối Bluetooth có phạm vi tối đa là 100 mét. IfThêm thiết bị đáng tin cậy để thiết bị này luôn mở khóa khi được kết nối. +:Xóa thiết bị này khỏi các thiết bị tin cậy? <PVí dụ:

- Hình dán NFC trên đế điện thoại trong ô tô của bạn IeThêm thiết bị NFC đáng tin cậy để mở khóa thiết bị này bằng một lần chạm. %4Bật GPS để truy cập địa điểm tin cậy. 1ABật Internet và GPS để truy cập địa điểm tin cậy. *9Bật Internet để truy cập địa điểm tin cậy. "Chỉnh sửa nhà riêng/cơ quan i��Xin lưu ý rằng không thể đảm bảo độ chính xác của vị trí. Các địa điểm đáng tin cậy có phạm vi là %s mét. Địa chỉ  Tên địa điểm tùy chỉnh "Địa điểm không xác định Tắt vị trí này Bật vị trí này Xóa Đổi tên /@Không có tính năng thiết bị tin cậy để sử dụng. *=Bật GPS để truy cập địa điểm được tin cậy. Đăng nhập Google *Đã xảy ra sự cố khi cấp quyền. WxCho phép Google sử dụng vị trí củand it is zero terminated. */
	u8 ea_value[0];		/* The value of the EA.  Immediately follows
				   the name. */
} __attribute__ ((__packed__)) EA_ATTR;

/*
 * Attribute: Property set (0xf0).
 *
 * Intended to support Native Structure Storage (NSS) - a feature removed from
 * NTFS 3.0 during beta testing.
 */
typedef struct {
	/* Irrelevant as feature unused. */
} __attribute__ ((__packed__)) PROPERTY_SET;

/*
 * Attribute: Logged utility stream (0x100).
 *
 * NOTE: Can be resident or non-resident.
 *
 * Operations on this attribute are logged to the journal ($LogFile) like
 * normal metadata changes.
 *
 * Used by the Encrypting File System (EFS). All encrypted files have this
 * attribute with the name $EFS.
 */
typedef struct {
	/* Can be anything the creator chooses. */
	/* EFS uses it as follows: */
	// FIXME: Type this info, verifying it along the way. (AIA)
} __attribute__ ((__packed__)) LOGGED_UTILITY_STREAM, EFS_ATTR;

#endif /* _LINUX_NTFS_LAYOUT_H */
