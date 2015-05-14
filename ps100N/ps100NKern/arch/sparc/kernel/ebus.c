ak;
		}
	} else {
		description = "Unhandled error code";
		action = ACTION_FAIL;
	}

	switch (action) {
	case ACTION_FAIL:
		/* Give up and fail the remainder of the request */
		scsi_release_buffers(cmd);
		if (!(req->cmd_flags & REQ_QUIET)) {
			if (description)
				scmd_printk(KERN_INFO, cmd, "%s\n",
					    description);
			scsi_print_result(cmd);
			if (driver_byte(result) & DRIVER_SENSE)
				scsi_print_sense("", cmd);
		}
		blk_end_request_all(req, -EIO);
		scsi_next_command(cmd);
		break;
	case ACTION_REPREP:
		/* Unprep the request and put it back at the head of the queue.
		 * A new command will be prepared and issued.
		 */
		scsi_release_buffers(cmd);
		scsi_requeue_command(q, cmd);
		break;
	case ACTION_RETRY:
		/* Retry the same command immediately */
		__scsi_queue_insert(cmd, SCSI_MLQUEUE_EH_RETRY, 0);
		break;
	case ACTION_DELAYED_RETRY:
		/* Retry the same command after a delay */
		__scsi_queue_insert(cmd, SCSI_MLQUEUE_DEVICE_BUSY, 0);
		break;
	}
}

static int scsi_init_sgtable(struct request *req, struct scsi_data_buffer *sdb,
			     gfp_t gfp_mask)
{
	int count;

	/*
	 * If sg table allocation fails, requeue request later.
	 */
	if (unlikely(scsi_alloc_sgtable(sdb, req->nr_phys_segments,
					gfp_mask))) {
		return BLKPREP_DEFER;
	}

	req->buffer = NULL;

	/* 
	 * Next, walk the list, and fill in the addresses and sizes of
	 * each segment.
	 */
	count = blk_rq_map_sg(req->q, req, sdb->table.sgl);
	BUG_ON(count > sdb->table.nents);
	sdb->table.nents = count;
	sdb->length = blk_rq_bytes(req);
	return BLKPREP_OK;
}

/*
 * Function:    scsi_init_io()
 *
 * Purpose:     SCSI I/O initialize function.
 *
 * Arguments:   cmd   - Command descriptor we wish to initialize
 *
 * Returns:     0 on success
 *		BLKPREP_DEFER if the failure is retryable
 *		BLKPREP_KILL if the failure is fatal
 */
int scsi_init_io(struct scsi_cmnd *cmd, gfp_t gfp_mask)
{
	int error = scsi_init_sgtable(cmd->request, &cmd->sdb, gfp_mask);
	if (error)
		goto err_exit;

	if (blk_bidi_rq(cmd->request)) {
		struct scsi_data_buffer *bidi_sdb = kmem_cache_zalloc(
			scsi_sdb_cache, GFP_ATOMIC);
		if (!bidi_sdb) {
			error = BLKPREP_DEFER;
			goto err_exit;
		}

		cmd->request->next_rq->special = bidi_sdb;
		error = scsi_init_sgtable(cmd->request->next_rq, bidi_sdb,
								    GFP_ATOMIC);
		if (error)
			goto err_exit;
	}

	if (blk_integrity_rq(cmd->request)) {
		struct scsi_data_buffer *prot_sdb = cmd->prot_sdb;
		int ivecs, count;

		BUG_ON(prot_sdb == NULL);
		ivecs = blk_rq_count_integrity_sg(cmd->request);

		if (scsi_alloc_sgtable(prot_sdb, ivecs, gfp_mask)) {
			error = BLKPREP_DEFER;
			goto err_exit;
		}

		count = blk_rq_map_integrity_sg(cmd->request,
						prot_sdb->table.sgl);
		BUG_ON(unlikely(count > ivecs));

		cmd->prot_sdb = prot_sdb;
		cmd->prot_sdb->table.nents = count;
	}

	return BLKPREP_OK ;

err_exit:
	scsi_release_buffers(cmd);
	if (error == BLKPREP_KILL)
		scsi_put_command(cmd);
	else /* BLKPREP_DEFER */
		scsi_unprep_request(cmd->request);

	return error;
}
EXPORT_SYMBOL(scsi_init_io);

static struct scsi_cmnd *scsi_get_cmd_from_req(struct scsi_device *sdev,
		struct request *req)
{
	struct scsi_cmnd *cmd;

	if (!req->special) {
		cmd = scsi_get_command(sdev, GFP_ATOMIC);
		if (unlikely(!cmd))
			return NULL;
		req->special = cmd;
	} else {
		cmd = req->special;
	}

	/* pull a tag out of the request if we have one */
	cmd->tag = req->tag;
	cmd->request = req;

	cmd->cmnd = req->cmd;

	return cmd;
}

int scsi_setup_blk_pc_cmnd(struct scsi_device *sdev, struct request *req)
{
	struct scsi_cmnd *cmd;
	int ret = scsi_prep_state_check(sdev, req);

	if (ret != BLKPREP_OK)
		return ret;

	cmd = scsi_get_cmd_from_req(sdev, req);
	if (unlikely(!cmd))
		return BLKPREP_DEFER;

	/*
	 * BLOCK_PC requests may transfer data, in which case they must
	 * a bio attached to them.  Or they might contain a SCSI command
	 * that does not transfer data, in which case they may optionally
	 * submit a request without an attached bio.
	 */
	if (req->bio) {
		int ret;

		BUG_ON(!req->nr_phys_segments);

		ret = scsi_init_io(cmd, GFP_ATOMIC);
		if (unlikely(ret))
			return ret;
	} else {
		BUG_ON(blk_rq_bytes(req));

		memset(&cmd->sdb, 0, sizeof(cmd->sdb));
		req->buffer = NULL;
	}

	cmd->cmd_len = req->cmd_len;
	if (!blk_rq_bytes(req))
		cmd->sc_data_direction = DMA_NONE;
	else if (rq_data_dir(req) == WRITE)
		cmd->sc_data_direction = DMA_TO_DEVICE;
	else
		cmd->sc_data_direction = DMA_FROM_DEVICE;
	
	cmd->transfersize = blk_rq_bytes(req);
	cmd->allowed = req->retries;
	return BLKPREP_OK;
}
EXPORT_SYMBOL(scsi_setup_blk_pc_cmnd);

/*
 * Setup a REQ_TYPE_FS command.  These are simple read/write request
 * from filesystems that still need to be translated to SCSI CDBs from
 * the ULD.
 */
int scsi_setup_fs_cmnd(struct scsi_device *sdev, struct request *req)
{
	struct scsi_cmnd *cmd;
	int ret = scsi_prep_state_check(sdev, req);

	if (ret != BLKPREP_OK)
		return ret;

	if (unlikely(sdev->scsi_dh_data && sdev->scsi_dh_data->scsi_dh
			 && sdev->scsi_dh_data->scsi_dh->prep_fn)) {
		ret = sdev->scsi_dh_data->scsi_dh->prep_fn(sdev, req);
		if (ret != BLKPREP_OK)
			return ret;
	}

	/*
	 * Filesystem requests must transfer data.
	 */
	BUG_ON(!req->nr_phys_segments);

	cmd = scsi_get_cmd_from_req(sdev, req);
	if (unlikely(!cmd))
		return BLKPREP_DEFER;

	memset(cmd->cmnd, 0, BLK_MAX_CDB);
	return scsi_init_io(cmd, GFP_ATOMIC);
}
EXPORT_SYMBOL(scsi_setup_fs_cmnd);

int scsi_prep_state_check(struct scsi_device *sdev, struct request *req)
{
	int ret = BLKPREP_OK;

	/*
	 * If the device is not in running state we will reject some
	 * or all commands.
	 */
	if (unlikely(sdev->sdev_state != SDEV_RUNNING)) {
		switch (sdev->sdev_state) {
		case SDEV_OFFLINE:
			/*
			 * If the device is offline we refuse to process any
			 * commands.  The device must be brought online
			 * before trying any recovery commands.
			 */
			sdev_printk(KERN_ERR, sdev,
				    "rejecting I/O to offline device\n");
			ret = 