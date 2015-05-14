s_status(vha, ms_pkt, ct_rsp,
		    "GNN_ID") != QLA_SUCCESS) {
			rval = QLA_FUNCTION_FAILED;
		} else {
			/* Save nodename */
			memcpy(list[i].node_name,
			    ct_rsp->rsp.gnn_id.node_name, WWN_SIZE);

			DEBUG2_3(printk("scsi(%ld): GID_PT entry - "
			    "nn %02x%02x%02x%02x%02x%02x%02x%02x "
			    "pn %02x%02x%02x%02x%02x%02x%02x%02x "
			    "portid=%02x%02x%02x.\n",
			    vha->host_no,
			    list[i].node_name[0], list[i].node_name[1],
			    list[i].node_name[2], list[i].node_name[3],
			    list[i].node_name[4], list[i].node_name[5],
			    list[i].node_name[6], list[i].node_name[7],
			    list[i].port_name[0], list[i].port_name[1],
			    list[i].port_name[2], list[i].port_name[3],
			    list[i].port_name[4], list[i].port_name[5],
			    list[i].port_name[6], list[i].port_name[7],
			    list[i].d_id.b.domain, list[i].d_id.b.area,
			    list[i].d_id.b.al_pa));
		}

		/* Last device exit. */
		if (list[i].d_id.b.rsvd_1 != 0)
			break;
	}

	return (rval);
}

/**
 * qla2x00_rft_id() - SNS Register FC-4 TYPEs (RFT_ID) supported by the HBA.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla2x00_rft_id(scsi_qla_host_t *vha)
{
	int		rval;
	struct qla_hw_data *ha = vha->hw;
	ms_iocb_entry_t	*ms_pkt;
	struct ct_sns_req	*ct_req;
	struct ct_sns_rsp	*ct_rsp;

	if (IS_QLA2100(ha) || IS_QLA2200(ha))
		return qla2x00_sns_rft_id(vha);

	/* Issue RFT_ID */
	/* Prepare common MS IOCB */
	ms_pkt = ha->isp_ops->prep_ms_iocb(vha, RFT_ID_REQ_SIZE,
	    RFT_ID_RSP_SIZE);

	/* Prepare CT request */
	ct_req = qla2x00_prep_ct_req(&ha->ct_sns->p.req, RFT_ID_CMD,
	    RFT_ID_RSP_SIZE);
	ct_rsp = &ha->ct_sns->p.rsp;

	/* Prepare CT arguments -- port_id, FC-4 types */
	ct_req->req.rft_id.port_id[0] = vha->d_id.b.domain;
	ct_req->req.rft_id.port_id[1] = vha->d_id.b.area;
	ct_req->req.rft_id.port_id[2] = vha->d_id.b.al_pa;

	ct_req->req.rft_id.fc4_types[2] = 0x01;		/* FCP-3 */

	/* Execute MS IOCB */
	rval = qla2x00_issue_iocb(vha, ha->ms_iocb, ha->ms_iocb_dma,
	    sizeof(ms_iocb_entry_t));
	if (rval != QLA_SUCCESS) {
		/*EMPTY*/
		DEBUG2_3(printk("scsi(%ld): RFT_ID issue IOCB failed (%d).\n",
		    vha->host_no, rval));
	} else if (qla2x00_chk_ms_status(vha, ms_pkt, ct_rsp, "RFT_ID") !=
	    QLA_SUCCESS) {
		rval = QLA_FUNCTION_FAILED;
	} else {
		DEBUG2(printk("scsi(%ld): RFT_ID exiting normally.\n",
		    vha->host_no));
	}

	return (rval);
}

/**
 * qla2x00_rff_id() - SNS Register FC-4 Features (RFF_ID) supported by the HBA.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla2x00_rff_id(scsi_qla_host_t *vha)
{
	int		rval;
	struct qla_hw_data *ha = vha->hw;
	ms_iocb_entry_t	*ms_pkt;
	struct ct_sns_req	*ct_req;
	struct ct_sns_rsp	*ct_rsp;

	if (IS_QLA2100(ha) || IS_QLA2200(ha)) {
		DEBUG2(printk("scsi(%ld): RFF_ID call unsupported on "
		    "ISP2100/ISP2200.\n", vha->host_no));
		return (QLA_SUCCESS);
	}

	/* Issue RFF_ID */
	/* Prepare common MS IOCB */
	ms_pkt = ha->isp_ops->prep_ms_iocb(vha, RFF_ID_REQ_SIZE,
	    RFF_ID_RSP_SIZE);

	/* Prepare CT request */
	ct_req = qla2x00_prep_ct_req(&ha->ct_sns->p.req, RFF_ID_CMD,
	    RFF_ID_RSP_SIZE);
	ct_rsp = &ha->ct_sns->p.rsp;

	/* Prepare CT arguments -- port_id, FC-4 feature, FC-4 type */
	ct_req->req.rff_id.port_id[0] = vha->d_id.b.domain;
	ct_req->req.rff_id.port_id[1] = vha->d_id.b.area;
	ct_req->req.rff_id.port_id[2] = vha->d_id.b.al_pa;

	ct_req->req.rff_id.fc4_feature = BIT_1;
	ct_req->req.rff_id.fc4_type = 0x08;		/* SCSI - FCP */

	/* Execute MS IOCB */
	rval = qla2x00_issue_iocb(vha, ha->ms_iocb, ha->ms_iocb_dma,
	    sizeof(ms_iocb_entry_t));
	if (rval != QLA_SUCCESS) {
		/*EMPTY*/
		DEBUG2_3(printk("scsi(%ld): RFF_ID issue IOCB failed (%d).\n",
		    vha->host_no, rval));
	} else if (qla2x00_chk_ms_status(vha, ms_pkt, ct_rsp, "RFF_ID") !=
	    QLA_SUCCESS) {
		rval = QLA_FUNCTION_FAILED;
	} else {
		DEBUG2(printk("scsi(%ld): RFF_ID exiting normally.\n",
		    vha->host_no));
	}

	return (rval);
}

/**
 * qla2x00_rnn_id() - SNS Register Node Name (RNN_ID) of the HBA.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla2x00_rnn_id(scsi_qla_host_t *vha)
{
	int		rval;
	struct qla_hw_data *ha = vha->hw;
	ms_iocb_entry_t	*ms_pkt;
	struct ct_sns_req	*ct_req;
	struct ct_sns_rsp	*ct_rsp;

	if (IS_QLA2100(ha) || IS_QLA2200(ha))
		return qla2x00_sns_rnn_id(vha);

	/* Issue RNN_ID */
	/* Prepare common MS IOCB */
	ms_pkt = ha->isp_ops->prep_ms_iocb(vha, RNN_ID_REQ_SIZE,
	    RNN_ID_RSP_SIZE);

	/* Prepare CT request */
	ct_req = qla2x00_prep_ct_req(&ha->ct_sns->p.req, RNN_ID_CMD,
	    RNN_ID_RSP_SIZE);
	ct_rsp = &ha->ct_sns->p.rsp;

	/* Prepare CT arguments -- port_id, node_name */
	ct_req->req.rnn_id.port_id[0] = vha->d_id.b.domain;
	ct_req->req.rnn_id.port_id[1] = vha->d_id.b.area;
	ct_req->req.rnn_id.port_id[2] = vha->d_id.b.al_pa;

	memcpy(ct_req->req.rnn_id.node_name, vha->node_name, WWN_SIZE);

	/* Execute MS IOCB */
	rval = qla2x00_issue_iocb(vha, ha->ms_iocb, ha->ms_iocb_dma,
	    sizeof(ms_iocb_entry_t));
	if (rval != QLA_SUCCESS) {
		/*EMPTY*/
		DEBUG2_3(printk("scsi(%ld): RNN_ID issue IOCB failed (%d).\n",
		    vha->host_no, rval));
	} else if (qla2x00_chk_ms_status(vha, ms_pkt, ct_rsp, "RNN_ID") !=
	    QLA_SUCCESS) {
		rval = QLA_FUNCTION_FAILED;
	} else {
		DEBUG2(printk("scsi(%ld): RNN_ID exiting normally.\n",
		    vha->host_no));
	}

	return (rval);
}

void
qla2x00_get_sym_node_name(scsi_qla_host_t *vha, uint8_t *snn)
{
	struct qla_hw_data *ha = vha->hw;
	sprintf(snn, "%s FW:v%d.%02d.%02d DVR:v%s",ha->model_number,
	    ha->fw_major_version, ha->fw_minor_version,
	    ha->fw_subminor_version, qla2x00_version_str);
}

/**
 * qla2x00_rsnn_nn() - SNS Register Symbolic Node Name (RSNN_NN) of the HBA.
 * @ha: HA context
 *
 * Returns 0 on success.
 */
int
qla2x00_rsnn_nn(scsi_qla_host_t *vha)
{
	int		rval;
	struct qla_hw_data *ha = vha->hw;
	ms_iocb_entry_t	*ms_pkt;
	struct ct_sns_req	*ct_req;
	struct ct_sns_rsp	*ct_rsp;

	if (IS_QLA2100(ha) || IS_QLA2200(ha)) {
		DEBUG2(printk("scsi(%ld): RSNN_ID call unsupported on "
		    "ISP2100/ISP2200.\n", vha->host_no));
		return (QLA_SUCCESS);
	}

	/* Issue RSNN_NN */
	/* Prepare common MS IOCB */
	/*   Request size adjusted after CT preparation */
	ms_pkt = ha->isp_ops->prep_ms_iocb(vha, 0, RSNN_NN_RSP_SIZE);

	/* Prepare CT request */
	ct_req = qla2x00_prep_ct_req(&ha->ct_sns->p.req, RSNN_NN_CMD,
	    RSNN_NN_RSP_SIZE);
	ct_rsp = &ha->ct_sns->p.rsp;

	/* Prepare CT arguments -- node_name, symbolic node_name, size */
	memcpy(ct_req->req.rsnn_nn.node_name, vha->node_name, WWN_SIZE);

	/* Prepare the Symbolic Node Name */
	qla2x00_get_sym_node_name(vha, ct_req->req.rsnn_nn.sym_node_name);

	/* Calculate SNN length */
	ct_req->req.rsnn_nn.name_len =
	    (uint8_t)strlen(ct_req->req.rsnn_nn.sym_node_name);

	/* Update MS IOCB request */
	ms_pkt->req_bytecount =
	    cpu_to_le32(24 + 1 + ct_req->req.rsnn_nn.name_len);
	ms_pkt->dseg_req_length = ms_pkt->req_bytecount;

	/* Execute MS IOCB */
	rval = qla2x00_issue_iocb(vha, ha->ms_iocb, ha->ms_iocb_dma,
	    sizeof(ms_iocb_entry_t));
	if (rval != QLA_SUCCESS) {
		/*EMPTY*/
		DEBUG2_3(printk("scsi(%ld): RSNN_NN issue IOCB failed (%d).\n",
		    vha->host_no, rval));
	} else if (qla2x00_chk_ms_status(v