oop_id = SNS_LAST_LOOP_ID_2100;
		ha->gid_list_info_size = 4;
		ha->flash_conf_off = ~0;
		ha->flash_data_off = ~0;
		ha->nvram_conf_off = ~0;
		ha->nvram_data_off = ~0;
		ha->isp_ops = &qla2100_isp_ops;
	} else if (IS_QLA23XX(ha)) {
		ha->mbx_count = MAILBOX_REGISTER_COUNT;
		req_length = REQUEST_ENTRY_CNT_2200;
		rsp_length = RESPONSE_ENTRY_CNT_2300;
		ha->max_loop_id = SNS_LAST_LOOP_ID_2300;
		ha->gid_list_info_size = 6;
		if (IS_QLA2322(ha) || IS_QLA6322(ha))
			ha->optrom_size = OPTROM_SIZE_2322;
		ha->flash_conf_off = ~0;
		ha->flash_data_off = ~0;
		ha->nvram_conf_off = ~0;
		ha->nvram_data_off = ~0;
		ha->isp_ops = &qla2300_isp_ops;
	} else if (IS_QLA24XX_TYPE(ha)) {
		ha->mbx_count = MAILBOX_REGISTER_COUNT;
		req_length = REQUEST_ENTRY_CNT_24XX;
		rsp_length = RESPONSE_ENTRY_CNT_2300;
		ha->max_loop_id = SNS_LAST_LOOP_ID_2300;
		ha->init_cb_size = sizeof(struct mid_init_cb_24xx);
		ha->gid_list_info_size = 8;
		ha->optrom_size = OPTROM_SIZE_24XX;
		ha->nvram_npiv_size = QLA_MAX_VPORTS_QLA24XX;
		ha->isp_ops = &qla24xx_isp_ops;
		ha->flash_conf_off = FARX_ACCESS_FLASH_CONF;
		ha->flash_data_off = FARX_ACCESS_FLASH_DATA;
		ha->nvram_conf_off = FARX_ACCESS_NVRAM_CONF;
		ha->nvram_data_off = FARX_ACCESS_NVRAM_DATA;
	} else if (IS_QLA25XX(ha)) {
		ha->mbx_count = MAILBOX_REGISTER_COUNT;
		req_length = REQUEST_ENTRY_CNT_24XX;
		rsp_length = RESPONSE_ENTRY_CNT_2300;
		ha->max_loop_id = SNS_LAST_LOOP_ID_2300;
		ha->init_cb_size = sizeof(struct mid_init_cb_24xx);
		ha->gid_list_info_size = 8;
		ha->optrom_size = OPTROM_SIZE_25XX;
		ha->nvram_npiv_size = QLA_MAX_VPORTS_QLA25XX;
		ha->isp_ops = &qla25xx_isp_ops;
		ha->flash_conf_off = FARX_AC