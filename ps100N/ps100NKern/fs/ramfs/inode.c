t[2] = p->counter_select[2];
		dev->pma_counter_select[3] = p->counter_select[3];
		dev->pma_counter_select[4] = p->counter_select[4];
		if (crp->cr_psstat) {
			ipath_write_creg(dev->dd, crp->cr_psinterval,
					 dev->pma_sample_interval);
			ipath_write_creg(dev->dd, crp->cr_psstart,
					 dev->pma_sample_start);
		} else
			dev->pma_sample_status = IB_PMA_SAMPLE_STATUS_STARTED;
	}
	spin_unlock_irqrestore(&dev->pending_lock, flags);

	ret = recv_pma_get_portsamplescontrol(pmp, ibdev, port);

bail:
	return ret;
}

static u64 get_counter(struct ipath_ibdev *dev,
		       struct ipath_cregs const *crp,
		       __be16 sel)
{
	u64 ret;

	switch (sel) {
	case IB_PMA_PORT_XMIT_DATA:
		ret = (crp->cr_psxmitdatacount) ?
			ipath_read_creg32(dev->dd, crp->cr_psxmitdatacount) :
			dev->ipath_sword;
		break;
	case IB_PMA_PORT_RCV_DATA:
		ret = (crp->cr_psrcvdatacount) ?
			ipath_read_creg32(dev->dd, crp->cr_psrcvdatacount) :
			dev->ipath_rword;
		break;
	case IB_PMA_PORT_XMIT_PKTS:
		ret = (crp->cr_psxmitpktscount) ?
			ipath_read_creg32(dev->dd, crp->cr_psxmitpktscount) :
			dev->ipath_spkts;
		break;
	case IB_PMA_PORT_RCV_PKTS:
		ret = (crp->cr_psrcvpktscount) ?
			ipath_read_creg32(dev->dd, crp->cr_psrcvpktscount) :
			dev->ipath_rpkts;
		break;
	case IB_PMA_PORT_XMIT_WAIT:
		ret = (crp->cr_psxmitwaitcount) ?
			ipath_read_creg32(dev->dd, crp->cr_psxmitwaitcount) :
			dev->ipath_xmit_wait;
		break;
	default:
		ret = 0;
	}

	return ret;
}

static int recv_pma_get_portsamplesresult(struct ib_perf *pmp,
					  struct ib_device *ibdev)
{
	struct ib_pma_portsamplesresult *p =
		(struct ib_pma_portsamplesresult *)pmp->data;
	struct ipath_ibdev *dev = to_idev(ibdev);
	struct ipath_cregs const *crp = dev->dd->ipath_cregs;
	u8 status;
	int i;

	memset(pmp->data, 0, sizeof(pmp->data));
	p->tag = cpu_to_be16(dev->pma_tag);
	if (crp->cr_psstat)
		status = ipath_read_creg32(dev->dd, crp->cr_psstat);
	else
		status = dev->pma_sample_status;
	p->sample_status = cpu_to_be16(status);
	for (i = 0; i < ARRAY_SIZE(dev->pma_counter_select); i++)
		p->counter[i] = (status != IB_PMA_SAMPLE_STATUS_DONE) ? 0 :
		    cpu_to_be32(
			get_counter(dev, crp, dev->pma_counter_select[i]));

	return reply((struct ib_smp *) pmp);
}

static int recv_pma_get_portsamplesresult_ext(struct ib_perf *pmp,
					      struct ib_device *ibdev)
{
	struct ib_pma_portsamplesresult_ext *p =
		(struct ib_pma_portsamplesresult_ext *)pmp->data;
	struct ipath_ibdev *dev = to_idev(ibdev);
	struct ipath_cregs const *crp = dev->dd->ipath_cregs;
	u8 status;
	int i;

	memset(pmp->data, 0, sizeof(pmp->data));
	p->tag = cpu_to_be16(dev->pma_tag);
	if (crp->cr_psstat)
		status = ipath_read_creg32(dev->dd, crp->cr_psstat);
	else
		status = dev->pma_sample_status;
	p->sample_status = cpu_to_be16(status);
	/* 64 bits */
	p->extended_width = cpu_to_be32(0x80000000);
	for (i = 0; i < ARRAY_SIZE(dev->pma_counter_select); i++)
		p->counter[i] = (status != IB_PMA_SAMPLE_STATUS_DONE) ? 0 :
		    cpu_to_be64(
			get_counter(dev, crp, dev->pma_counter_select[i]));

	return reply((struct ib_smp *) pmp);
}

static int recv_pma_get_portcounters(struct ib_perf *pmp,
				     struct ib_device *ibdev, u8 port)
{
	struct ib_pma_portcounters *p = (struct ib_pma_portcounters *)
		pmp->data;
	struct ipath_ibdev *dev = to_idev(ibdev);
	struct ipath_verbs_counters cntrs;
	u8 port_select = p->port_select;

	ipath_get_counters(dev->dd, &cntrs);

	/* Adjust counters for any resets done. */
	cntrs.symbol_error_counter -= dev->z_symbol_error_counter;
	cntrs.link_error_recovery_counter -=
		dev->z_link_error_recovery_counter;
	cntrs.link_downed_counter -= dev->z_link_downed_counter;
	cntrs.port_rcv_errors += dev->rcv_errors;
	cntrs.port_rcv_errors -= dev->z_port_rcv_errors;
	cntrs.port_rcv_remphys_errors -= dev->z_port_rcv_remphys_errors;
	cntrs.port_xmit_discards -= dev->z_port_xmit_discards;
	cntrs.port_xmit_data -= dev->z_port_xmit_data;
	cntrs.port_rcv_data -= dev->z_port_rcv_data;
	cntrs.port_xmit_packets -= dev->z_port_xmit_packets;
	cntrs.port_rcv_packets -= dev->z_port_rcv_packets;
	cntrs.local_link_integrity_errors -=
		dev->z_local_link_integrity_errors;
	cntrs.excessive_buffer_overrun_errors -=
		dev->z_excessive_buffer_overrun_errors;
	cntrs.vl15_dropped -= dev->z_vl15_dropped;
	cntrs.vl15_dropped += dev->n_vl15_dropped;

	memset(pmp->data, 0, sizeof(pmp->data));

	p->port_select = port_select;
	if (pmp->attr_mod != 0 ||
	    (port_select != port && port_select != 0xFF))
		pmp->status |= IB_SMP_INVALID_FIELD;

	if (cntrs.symbol_error_counter > 0xFFFFUL)
		p->symbol_error_counter = cpu_to_be16(0xFFFF);
	else
		p->symbol_error_counter =
			cpu_to_be16((u16)cntrs.symbol_error_counter);
	if (cntrs.link_error_recovery_counter > 0xFFUL)
		p->link_error_recovery_counter = 0xFF;
	else
		p->link_error_recovery_counter =
			(u8)cntrs.link_error_recovery_counter;
	if (cntrs.link_downed_counter > 0xFFUL)
		p->link_downed_counter = 0xFF;
	else
		p->link_downed_counter = (u8)cntrs.link_downed_counter;
	if (cntrs.port_rcv_errors > 0xFFFFUL)
		p->port_rcv_errors = cpu_to_be16(0xFFFF);
	else
		p->port_rcv_errors =
			cpu_to_be16((u16) cntrs.port_rcv_errors);
	if (cntrs.port_rcv_remphys_errors > 0xFFFFUL)
		p->port_rcv_remphys_errors = cpu_to_be16(0xFFFF);
	else
		p->port_rcv_remphys_errors =
			cpu_to_be16((u16)cntrs.port_rcv_remphys_errors);
	if (cntrs.port_xmit_discards > 0xFFFFUL)
		p->port_xmit_discards = cpu_to_be16(0xFFFF);
	else
		p->port_xmit_discards =
			cpu_to_be16((u16)cntrs.port_xmit_discards);
	if (cntrs.local_link_integrity_errors > 0xFUL)
		cntrs.local_link_integrity_errors = 0xFUL;
	if (cntrs.excessive_buffer_overrun_errors > 0xFUL)
		cntrs.excessive_buffer_overrun_errors = 0xFUL;
	p->lli_ebor_errors = (cntrs.local_link_integrity_errors << 4) |
		cntrs.excessive_buffer_overrun_errors;
	if (cntrs.vl15_dropped > 0xFFFFUL)
		p->vl15_dropped = cpu_to_be16(0xFFFF);
	else
		p->vl15_dropped = cpu_to_be16((u16)cntrs.vl15_dropped);
	if (cntrs.port_xmit_data > 0xFFFFFFFFUL)
		p->port_xmit_data = cpu_to_be32(0xFFFFFFFF);
	else
		p->port_xmit_data = cpu_to_be32((u32)cntrs.port_xmit_data);
	if (cntrs.port_rcv_data > 0xFFFFFFFFUL)
		p->port_rcv_data = cpu_to_be32(0xFFFFFFFF);
	else
		p->port_rcv_data = cpu_to_be32((u32)cntrs.port_rcv_data);
	if (cntrs.port_xmit_packets > 0xFFFFFFFFUL)
		p->port_xmit_packets = cpu_to_be32(0xFFFFFFFF);
	else
		p->port_xmit_packets =
			cpu_to_be32((u32)cntrs.port_xmit_packets);
	if (cntrs.port_rcv_packets > 0xFFFFFFFFUL)
		p->port_rcv_packets = cpu_to_be32(0xFFFFFFFF);
	else
		p->port_rcv_packets =
			cpu_to_be32((u32) cntrs.port_rcv_packets);

	return reply((struct ib_smp *) pmp);
}

static int recv_pma_get_portcounters_ext(struct ib_perf *pmp,
					 struct ib_device *ibdev, u8 port)
{
	struct ib_pma_portcounters_ext *p =
		(struct ib_pma_portcounters_ext *)pmp->data;
	struct ipath_ibdev *dev = to_idev(ibdev);
	u64 swords, rwords, spkts, rpkts, xwait;
	u8 port_select = p->port_select;

	ipath_snapshot_counters(dev->dd, &swords, &rwords, &spkts,
				&rpkts, &xwait);

	/* Adjust counters for any resets done. */
	swords -= dev->z_port_xmit_data;
	rwords -= dev->z_port_rcv_data;
	spkts -= dev->z_port_xmit_packets;
	rpkts -= dev->z_port_rcv_packets;

	memset(pmp->data, 0, sizeof(pmp->data));

	p->port_select = port_select;
	if (pmp->attr_mod != 0 ||
	    (port_select != port && port_select != 0xFF))
		pmp->status |= IB_SMP_INVALID_FIELD;

	p->port