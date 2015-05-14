led;
	}
	ha->rsp_q_map[0] = rsp;
	ha->req_q_map[0] = req;
	rsp->req = req;
	req->rsp = rsp;
	set_bit(0, ha->req_qid_map);
	set_bit(0, ha->rsp_qid_map);
	/* FWI2-capable only. */
	req->req_q_in = &ha->iobase->isp24.req_q_in;
	req->req_q_out = &ha->iobase->isp24.req_q_out;
	rsp->rsp_q_in = &ha->iobase->isp24.rsp_q_in;
	rsp->rsp_q_out = &ha->iobase->isp24.rsp_q_out;
	if (ha->mqenable) {
		req->req_q_in = &ha->mqiobase->isp25mq.req_q_in;
		req->req_q_out = &ha->mqiobase->isp25mq.req_q_out;
		rsp->rsp_q_in = &ha->mqiobase->isp25mq.rsp_q_in;
		rsp->rsp_q_out =  &ha->mqiobase->isp25mq.rsp_q_out;
	}

	if (qla2x00_initialize_adapter(base_vha)) {
		qla_printk(KERN_WARNING, ha,
		    "Failed to initialize adapter\n");

		DEBUG2(printk("scsi(%ld): Failed to initialize adapter - "
		    "Adapter flags %x.\n",
		    base_vha->host_no, base_vha->device_flags));

		ret = -ENODEV;
		goto probe_failed;
	}

	if (ha->mqenable)
		if (qla25xx_setup_mode(base_vha))
			qla_printk(KERN_WARNING, ha,
				"Can't create queues, falling back to single"
