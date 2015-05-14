	/* Prepare CT arguments -- port_type */
	ct_req->req.gid_pt.port_type = NS_NX_PORT_TYPE;

	/* Execute MS IOCB */
	rval = qla2x00_issue_iocb(vha, ha->ms_iocb, ha->ms_iocb_dma,
	    sizeof(ms_iocb_entry_t));
	if (rval != QLA_SUCCESS) {
		/*EMPTY*/
		DEBUG2_3(printk("scsi(%ld): GID_PT issue IOCB failed (%d).\n",
		    vha->host_no, rval));
	} else if (qla2x00_chk_ms_status(vha, ms_pkt, ct_rsp, "GID_PT") !=
	    QLA_SUCCESS) {
		rval = QLA_FUNCTION_FAILED;
	} else {
		/* Set port IDs in switch info list. */
		for (i = 0; i < MAX_FIBRE_DEVICES; i++) {
			gid_data = &ct_rsp->rsp.gid_pt.entries[i];
			list[i].d_id.b.domain = gid_data->port_id[0];
			list[i].d_id.b.area = gid_data->port_id[1];
			list[i].d_id.b.al_pa = gid_data->port_id[2];
			memset(list[i].fabric_port_name, 0, WWN_SIZE);
			list[i].fp_speed = PORT_SPEED_UNKNOWN;

			/* Last one exit. */
			if (gid_data->control_byte & BIT_7) {
				list[i].d_id.b.rsvd_1 = gid_data->control_byte;
				break;
			}
		}

		/*
		 * If we've used all available slots, then the switch is
		 * reporting back more devices than we can handle with this
		 * single call.  Return a failed status, and let GA_NXT handle
		 * the overload.
		 */
		if (i == MAX_FIBRE_DEVICES)
			rval = QLA_FUNCTION_FAILED;
	}

	return (rval);
}

/**
 * qla2x00_gpn_id() - SNS Get Port Name (GPN_ID) query.
 * @ha: HA context
 * @list: switch info entries to populate
 *
 * Returns 0 on success.
 */
int
qla2x00_gpn_id(scsi_qla_host_t *vha, sw_info_t *list)
{
	int		rval;
	uint16_t	i;

	ms_iocb_entry_t	*ms_pkt;
	struct ct_sns_req	*ct_req;
	struct ct_sns_rsp	*ct_rsp;
	struct qla_hw_data *ha = vha->hw;

	if (IS_QLA2100(ha) || IS_QLA2200(ha))
		return qla2x00_sns_gpn_id(vha, list);

	for (i = 0; i < MAX_FIBRE_DEVICES; i++) {
		/* Issue GPN_ID */
		/* Prepare common MS IOCB */
		ms_pkt = ha->isp_ops->prep_ms_iocb(vha, GPN_ID_REQ_SIZE,
		    GPN_ID_RSP_SIZE);

		/* Prepare CT request */
		ct_req = qla2x00_prep_ct_req(&ha->ct_sns->p.req, GPN_ID_CMD,
		    GPN_ID_RSP_SIZE);
		ct_rsp = &ha->ct_sns->p.rsp;

		/* Prepare CT arguments -- port_id */
		ct_req->req.port_id.port_id[0] = list[i].d_id.b.domain;
		ct_req->req.port_id.port_id[1] = list[i].d_id.b.area;
		ct_req->req.port_id.port_id[2] = list[i].d_id.b.al_pa;

		/* Execute MS IOCB */
		rval = qla2x00_issue_iocb(vha, ha->ms_iocb, ha->ms_iocb_dma,
		    sizeof(ms_iocb_entry_t));
		if (rval != QLA_SUCCESS) {
			/*EMPTY*/
			DEBUG2_3(printk("scsi(%ld): GPN_ID issue IOCB failed "
			    "(%d).\n", vha->host_no, rval));
		} else if (qla2x00_chk_ms_status(vha, ms_pkt, ct_rsp,
		    "GPN_ID") != QLA_SUCCESS) {
			rval = QLA_FUNCTION_FAILED;
		} else {
			/* Save portname */
			memcpy(list[i].port_name,
			    ct_rsp->rsp.gpn_id.port_name, WWN_SIZE);
		}

		/* L