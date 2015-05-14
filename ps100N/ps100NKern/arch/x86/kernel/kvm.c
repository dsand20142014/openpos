gs);
	if (!(req->options & BIT_0)) {
		WRT_REG_DWORD(&reg->req_q_in, 0);
		WRT_REG_DWORD(&reg->req_q_out, 0);
	}
	req->req_q_in = &reg->req_q_in;
	req->req_q_out = &reg->req_q_out;
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	rval = qla2x00_mailbox_command(vha, mcp);
	if (rval != QLA_SUCCESS)
		DEBUG2_3_11(printk(KERN_WARNING "%s(%ld): failed=%x mb0=%x.\n",
			__func__, vha->host_no, rval, mcp->mb[0]));
	return rval;
}

int
qla25xx_init_rsp_que(struct scsi_qla_host *vha, struct rsp_que *rsp)
{
	int rval;
	unsigned long flags;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;
	struct device_reg_25xxmq __iomem *reg;
	struct qla_hw_data *ha = vha->hw;

	mcp->mb[0] = MBC_INITIALIZE_MULTIQ;
	mcp->mb[1] = rsp->options;
	mcp->mb[2] = MSW(LSD(rsp->dma));
	mcp->mb[3] = LSW(LSD(rsp->dma));
	mcp->mb[6] = MSW(MSD(rsp->dma));
	mcp->mb[7] = LSW(MSD(rsp->dma));
	mcp->mb[5] = rsp->length;
	mcp->mb[14] = rsp->msix->entry;
	mcp->mb[13] = rsp->rid;

	reg = (struct device_reg_25xxmq *)((void *)(ha->mqiobase) +
		QLA_QUE_PAGE * rsp->id);

	mcp->mb[4] = rsp->id;
	/* que in ptr index */
	mcp->mb[8] = 0;
	/* que out ptr index */
	mcp->mb[9] = 0;
	mcp->out_mb = MBX_14|MBX_13|MBX_9|MBX_8|MBX_7
			|MBX_6|MBX_5|MBX_4|MBX_3|MBX_2|MBX_1|MBX_0;
	mcp->in_mb = MBX_0;
	mcp->flags = MBX_DMA_OUT;
	mcp->tov = 60;

	spin_lock_irqsave(&ha->hardware_lock, flags);
	if (!(rsp->options & BIT_0)) {
		WRT_REG_DWORD(&reg->rsp_q_out, 0);
		WRT_REG_DWORD(&reg->rsp_q_in, 0);
	}

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	rval = qla2x00_mailbox_command(vha, mcp);
	if (rval != QLA_SUCCESS)
		DEBUG2_3_11(printk(KERN_WARNING "%s(%ld): failed=%x "
			"mb0=%x.\n", __func__,
			vha->host_no, rval, mcp->mb[0]));
	return rval;
}

int
qla81xx_idc_ack(scsi_qla_host_t *vha, uint16_t *mb)
{
	int rval;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;

	DEBUG11(printk("%s(%ld): entered.\n", __func__, vha->host_no));

	mcp->mb[0] = MBC_IDC_ACK;
	memcpy(&mcp->mb[1], mb, QLA_IDC_ACK_REGS * sizeof(uint16_t));
	mcp->out_mb = MBX_7|MBX_6|MBX_5|MBX_4|MBX_3|MBX_2|MBX_1|MBX_0;
	mcp->in_mb = MBX_0;
	mcp->tov = MBX_TOV_SECONDS;
	mcp->flags = 0;
	rval = qla2x00_mailbox_command(vha, mcp);

	if (rval != QLA_SUCCESS) {
		DEBUG2_3_11(printk("%s(%ld): failed=%x (%x).\n", __func__,
		    vha->host_no, rval, mcp->mb[0]));
	} else {
		DEBUG11(printk("%s(%ld): done.\n", __func__, vha->host_no));
	}

	return rval;
}

int
qla81xx_fac_get_sector_size(scsi_qla_host_t *vha, uint32_t *sector_size)
{
	int rval;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;

	if (!IS_QLA81XX(vha->hw))
		return QLA_FUNCTION_FAILED;

	DEBUG11(printk("%s(%ld): entered.\n", __func__, vha->host_no));

	mcp->mb[0] = MBC_FLASH_ACCESS_CTRL;
	mcp->mb[1] = FAC_OPT_CMD_GET_SECTOR_SIZE;
	mcp->out_mb = MBX_1|MBX_0;
	mcp->in_mb = MBX_1|MBX_0;
	mcp->tov = MBX_TOV_SECONDS;
	mcp->flags = 0;
	rval = qla2x00_mailbox_command(vha, mcp);

	if (rval != QLA_SUCCESS) {
		DEBUG2_3_11(printk("%s(%ld): failed=%x mb[0]=%x mb[1]=%x.\n",
		    __func__, vha->host_no, rval, mcp->mb[0], mcp->mb[1]));
	} else {
		DEBUG11(printk("%s(%ld): done.\n", __func__, vha->host_no));
		*sector_size = mcp->mb[1];
	}

	return rval;
}

int
qla81xx_fac_do_write_enable(scsi_qla_host_t *vha, int enable)
{
	int rval;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;

	if (!IS_QLA81XX(vha->hw))
		return QLA_FUNCTION_FAILED;

	DEBUG11(printk("%s(%ld): entered.\n", __func__, vha->host_no));

	mcp->mb[0] = MBC_FLASH_ACCESS_CTRL;
	mcp->mb[1] = enable ? FAC_OPT_CMD_WRITE_ENABLE :
	    FAC_OPT_CMD_WRITE_PROTECT;
	mcp->out_mb = MBX_1|MBX_0;
	mcp->in_mb = MBX_1|MBX_0;
	mcp->tov = MBX_TOV_SECONDS;
	mcp->flags = 0;
	rval = qla2x00_mailbox_command(vha, mcp);

	if (rval != QLA_SUCCESS) {
		DEBUG2_3_11(printk("%s(%ld): failed=%x mb[0]=%x mb[1]=%x.\n",
		    __func__, vha->host_no, rval, mcp->mb[0], mcp->mb[1]));
	} else {
		DEBUG11(printk("%s(%ld): done.\n", __func__, vha->host_no));
	}

	return rval;
}

int
qla81xx_fac_erase_sector(scsi_qla_host_t *vha, uint32_t start, uint32_t finish)
{
	int rval;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;

	if (!IS_QLA81XX(vha->hw))
		return QLA_FUNCTION_FAILED;

	DEBUG11(printk("%s(%ld): entered.\n", __func__, vha->host_no));

	mcp->mb[0] = MBC_FLASH_ACCESS_CTRL;
	mcp->mb[1] = FAC_OPT_CMD_ERASE_SECTOR;
	mcp->mb[2] = LSW(start);
	mcp->mb[3] = MSW(start);
	mcp->mb[4] = LSW(finish);
	mcp->mb[5] = MSW(finish);
	mcp->out_mb = MBX_5|MBX_4|MBX_3|MBX_2|MBX_1|MBX_0;
	mcp->in_mb = MBX_2|MBX_1|MBX_0;
	mcp->tov = MBX_TOV_SECONDS;
	mcp->flags = 0;
	rval = qla2x00_mailbox_command(vha, mcp);

	if (rval != QLA_SUCCESS) {
		DEBUG2_3_11(printk("%s(%ld): failed=%x mb[0]=%x mb[1]=%x "
		    "mb[2]=%x.\n", __func__, vha->host_no, rval, mcp->mb[0],
		    mcp->mb[1], mcp->mb[2]));
	} else {
		DEBUG11(printk("%s(%ld): done.\n", __func__, vha->host_no));
	}

	return rval;
}

int
qla81xx_restart_mpi_firmware(scsi_qla_host_t *vha)
{
	int rval = 0;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;

	DEBUG11(printk("%s(%ld): entered.\n", __func__, vha->host_no));

	mcp->mb[0] = MBC_RESTART_MPI_FW;
	mcp->out_mb = MBX_0;
	mcp->in_mb = MBX_0|MBX_1;
	mcp->tov = MBX_TOV_SECONDS;
	mcp->flags = 0;
	rval = qla2x00_mailbox_command(vha, mcp);

	if (rval != QLA_SUCCESS) {
		DEBUG2_3_11(printk("%s(%ld): failed=%x mb[0]=0x%x mb[1]=0x%x.\n",
		    __func__, vha->host_no, rval, mcp->mb[0], mcp->mb[1]));
	} else {
		DEBUG11(printk("%s(%ld): done.\n", __func__, vha->host_no));
	}

	return rval;
}

int
qla2x00_read_edc(scsi_qla_host_t *vha, uint16_t dev, uint16_t adr,
    dma_addr_t sfp_dma, uint8_t *sfp, uint16_t len, uint16_t opt)
{
	int rval;
	mbx_cmd_t mc;
	mbx_cmd_t *mcp = &mc;

	DEBUG11(printk("%s(%ld): entered.\n", __func__, vha->host_no));

	mcp->mb[0] = MBC_READ_SFP;
	mcp->mb[1] = dev;
	mcp->mb[2] = MSW(sfp_dma);
	mcp->mb[3] = LSW(sfp_dma);
	mcp->mb[6] = MSW(MSD(sfp_dma));
	mcp->mb[7] = LSW(MSD(sfp_dma));
