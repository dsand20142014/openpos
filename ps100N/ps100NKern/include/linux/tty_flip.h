k(&ha->vport_lock);
	que_id = find_first_zero_bit(ha->rsp_qid_map, ha->max_rsp_queues);
	if (que_id >= ha->max_rsp_queues) {
		mutex_unlock(&ha->vport_lock);
		qla_printk(KERN_INFO, ha, "No resources to create "
			 "additional response queue\n");
		goto que_failed;
	}
	set_bit(que_id, ha->rsp_qid_map);

	if (ha->flags.msix_enabled)
		rsp->msix = &ha->msix_entries[que_id + 1];
	else
		qla_printk(KERN_WARNING, ha, "msix not enabled\n");

	ha->rsp_q_map[que_id] = rsp;
	rsp->rid = rid;
	rsp->vp_idx = vp_idx;
	rsp->hw = ha;
	/* Use alternate PCI bus number */
	if (MSB(rsp->rid))
		options |= BIT_4;
	/* Use alternate PCI devfn */
	if (LSB(rsp->rid))
		options |= BIT_5;
	rsp->options = options;
	rsp->id = que_id;
	reg = ISP_QUE_REG(ha, que_id);
	rsp->rsp_q_in = &reg->isp25mq.rsp_q_in;
	rsp->rsp_q_out = &reg->isp25mq.rsp_q_out;
	mutex_unlock(&ha->vport_lock);

	ret = qla25xx_request_irq(rsp);
	if (ret)
		goto que_failed;

	ret = qla25xx_init_rsp_que(base_vha,