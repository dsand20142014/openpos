x_init_req_que(vha, req);
	if (ret != QLA_SUCCESS)
		DEBUG2_17(printk(KERN_WARNING "%s failed\n", __func__));
	/* restore options bit */
	req->options &= ~BIT_3;
	return ret;
}


/* Delete all queues for a given vhost */
int
qla25xx_delete_queues(struct scsi_qla_host *vha)
{
	int cnt, ret = 0;
	struct req_que *req = NULL;
	struct rsp_que *rsp = NULL;
	struct qla_hw_data *ha = vha->hw;

	/* Delete request queues */
	for (cnt = 1; cnt < ha->max_req_queues; cnt++) {
		req = ha->req_q_map[cnt];
		if (req) {
			ret = qla25xx_delete_req_que(vha, req);
			if (ret != QLA_SUCCESS) {
				qla_printk(KERN_WARNING, ha,
				"Couldn't delete req que %d\n",
				req->id);
				return ret;
			}
		}
	}

	/* Delete response queues */
	for (cnt = 1; cnt < ha->max_rsp_queues; cnt++) {
		rsp = ha->rsp_q_map[cnt];
		if (rsp) {
			ret = qla25xx_delete_rsp_que(vha, rsp);
			if (ret != QLA_SUCCESS) {
				qla_printk(KERN_WARNING, ha,
				"Couldn't delete rsp que %d\n",
				rsp->id);
				return ret;
			}
		}
	}
	return ret;
}

int
qla25xx_create_req_que(struct qla_hw_data *ha, uint16_t options,
	uint8_t vp_idx, uint16_t rid, int rsp_que, uint8_t qos)
{
	int ret = 0;
	struct req_que *req = NULL;
	struct scsi_qla_host *base_vha = pci_get_drvdata(ha->pdev);
	uint16_t que_id = 0;
	device_reg_t __iomem *reg;
	uint32_t cnt;

	req = kzalloc(sizeof(struct req_que), GFP_KERNEL);
	if (req == NULL) {
		qla_printk(KERN_WARNING, ha, "could not allocate memory"
			"for request que\n");
		goto que_failed;
	}

	req->length = REQUEST_ENTRY_CNT_24XX;
	req->ring = dma_alloc_coherent(&ha->pdev->dev,
			(req->leng