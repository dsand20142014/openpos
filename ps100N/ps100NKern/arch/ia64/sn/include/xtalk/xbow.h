t_struct *)crq->IU_data_ptr;
	switch (crq->valid) {
	case 0xC0:		/* initialization */
		switch (crq->format) {
		case 0x01:	/* Initialization message */
			dev_info(hostdata->dev, "partner initialized\n");
			/* Send back a response */
			if ((rc = ibmvscsi_ops->send_crq(hostdata,
							 0xC002000000000000LL, 0)) == 0) {
				/* Now login */
				init_adapter(hostdata);
			} else {
				dev_err(hostdata->dev, "Unable to send init rsp. rc=%ld\n", rc);
			}

			break;
		case 0x02:	/* Initialization response */
			dev_info(hostdata->dev, "partner initialization complete\n");

			/* Now login */
			init_adapter(hostdata);
			break;
		default:
			dev_err(hostdata->dev, "unknown crq message type: %d\n", crq->format);
		}
		return;
	case 0xFF:	/* Hypervisor telling us the connection is closed */
		scsi_block_requests(hostdata->host);
		atomic_set(&hostdata->request_limit, 0);
		if (crq->format == 0x06) {
			/* We need to re-setup the interpartition connection */
			dev_info(hostdata->dev, "Re-enabling adapter!\n");
			hostdata->client_migrated = 1;
			purge_requests(hostdata, DID_REQUEUE);
			if ((ibmvscsi_ops->reenable_crq_queue(&hostdata->queue,
							      hostdata)) ||
			    (ibmvscsi_ops->send_crq(hostdata,
						    0xC001000000000000LL, 0))) {
					atomic_set(&hostdata->request_limit,
						   -1);
					dev_err(hostdata->dev, "error after enable\n");
			}
		} else {
			dev_err(hostdata->dev, "Virtual adapter failed rc %d!\n",
				crq->format);

			purge_requests(hostdata, DID_ERROR);
			if ((ibmvscsi_ops->reset_crq_queue(&hostdata->queue,
							   hostdata)) ||
			    (ibmvscsi_ops->send_crq(hostdata,
						    0xC001000000000000LL, 0))) {
					atomic_set(&hostdata->request_limit,
						   -1);
					dev_err(hostdata->dev, "error after reset\n");
			}
		}
		scsi_unblock_requests(hostdata->host);
		return;
	case 0x80:		/* real payload */
		break;
	default:
		dev_err(hostdata->dev, "got an invalid message type 0x%02x\n",
			crq->valid);
		return;
	}

	/* The only kind of payload CRQs we should get are responses to
	 * things we send. Make sure this response is to something we
	 * actually sent
	 */
	if (!valid_event_struct(&hostdata->pool, evt_struct)) {
		dev_err(hostdata->dev, "returned correlation_token 0x%p is invalid!\n",
		       (void *)crq->IU_data_ptr);
		return;
	}

	if (atomic_read(&evt_struct->free)) {
		dev_err(hostdata->dev, "received duplicate correlation_token 0x%p!\n",
			(void *)crq->IU_data_ptr);
		return;
	}

	if (crq->format == VIOSRP_SRP_FORMAT)
		atomic_add(evt_struct->xfer_iu->srp.rsp.req_lim_delta,
			   &hostdata->request_limit);

	del_timer(&evt_struct->timer);

	if ((crq->status != VIOSRP_OK && crq->status != VIOSRP_OK2) && evt_struct->cmnd)
		evt_struct->cmnd->result = DID_ERROR << 16;
	if (evt_struct->done)
		evt_struct->done(evt_struct);
	else
		dev_err(hostdata->dev, "returned done() is NULL; not running it!\n");

	/*
	 * Lock the host_lock before messing with these structures, since we
	 * are running in a task context
	 */
	spin_lock_irqsave(evt_struct->hostdata->host->host_lock, flags);
	list_del(&evt_struct->list);
	free_event_struct(&evt_struct->hostdata->pool, evt_struct);
	spin_unlock_irqrestore(evt_struct->hostdata->host->host_lock, flags);
}

/**
 * ibmvscsi_get_host_config: Send the command to the server to get host
 * configuration data.  The data is opaque to us.
 */
static int ibmvscsi_do_host_config(struct ibmvscsi_host_data *hostdata,
				   unsigned char *buffer, int length)
{
	struct viosrp_host_config *host_config;
	struct srp_event_struct *evt_struct;
	unsigned long flags;
	dma_addr_t addr;
	int rc;

	evt_struct = get_event_struct(&hostdata->pool);
	if (!evt_struct) {
		dev_err(hostdata->dev, "couldn't allocate event for HOST_CONFIG!\n");
		return -1;
	}

	init_event_struct(evt_struct,
			  sync_completion,
			  VIOSRP_MAD_FORMAT,
			  info_timeout);

	host_config = &evt_struct->iu.mad.host_config;

	/* Set up a lun reset SRP command */
	memset(host_config, 0x00, sizeof(*host_config));
	host_config->common.type = VIOSRP_HOST_CONFIG_TYPE;
	host_config->common.length = length;
	host_config->buffer = addr = dma_map_single(hostdata->dev, buffer,
						    length,
						    DMA_BIDIRECTIONAL);

	if (dma_mapping_error(hostdata->dev, host_config->buffer)) {
		if (!firmware_has_feature(FW_FEATURE_CMO))
			dev_err(hostdata->dev,
			        "dma_mapping error getting host config\n");
		free_event_struct(&hostdata->pool, evt_struct);
		return -1;
	}

	init_completion(&evt_struct->comp);
	spin_lock_irqsave(hostdata->host->host_lock, flags);
	rc = ibmvscsi_send_srp_event(evt_struct, hostdata, info_timeout * 2);
	spin_unlock_irqrestore(hostdata->host->host_lock, flags);
	if (rc == 0)
		wait_for_completion(&evt_struct->comp);
	dma_unmap_single(hostdata->dev, addr, length, DMA_BIDIRECTIONAL);

	return rc;
}

/**
 * ibmvscsi_slave_configure: Set the "allow_restart" flag for each disk.
 * @sdev:	struct scsi_device device to configure
 *
 * Enable allow_restart for a device if it is a disk.  Adjust the
 * queue_depth here also as is required by the documentation for
 * struct scsi_host_template.
 */
static int ibmvscsi_slave_configure(struct scsi_device *sdev)
{
	struct Scsi_Host *shost = sdev->host;
	unsigned long lock_flags = 0;

	spin_lock_irqsave(shost->host_lock, lock_flags);
	if (sdev->type == TYPE_DISK) {
		sdev->allow_restart = 1;
		blk_queue_rq_timeout(sdev->request_queue, 120 * HZ);
	}
	scsi_adjust_queue_depth(sdev, 0, shost->cmd_per_lun);
	spin_unlock_irqrestore(shost->host_lock, lock_flags);
	return 0;
}

/**
 * ibmvscsi_change_queue_depth - Change the device's queue depth
 * @sdev:	scsi device struct
 * @qdepth:	depth to set
 *
 * Return value:
 * 	actual depth set
 **/
static int ibmvscsi_change_queue_depth(struct scsi_device *sdev, int qdepth)
{
	if (qdepth > IBMVSCSI_MAX_CMDS_PER_LUN)
		qdepth = IBMVSCSI_MAX_CMDS_PER_LUN;

	scsi_adjust_queue_depth(sdev, 0, qdepth);
	return sdev->queue_depth;
}

/* ------------------------------------------------------------
 * sysfs attributes
 */
static ssize_t show_host_vhost_loc(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, sizeof(hostdata->caps.loc), "%s\n",
		       hostdata->caps.loc);
	return len;
}

static struct device_attribute ibmvscsi_host_vhost_loc = {
	.attr = {
		 .name = "vhost_loc",
		 .mode = S_IRUGO,
		 },
	.show = show_host_vhost_loc,
};

static ssize_t show_host_vhost_name(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, sizeof(hostdata->caps.name), "%s\n",
		       hostdata->caps.name);
	return len;
}

static struct device_attribute ibmvscsi_host_vhost_name = {
	.attr = {
		 .name = "vhost_name",
		 .mode = S_IRUGO,
		 },
	.show = show_host_vhost_name,
};

static ssize_t show_host_srp_version(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, PAGE_SIZE, "%s\n",
		       hostdata->madapter_info.srp_version);
	return len;
}

static struct device_attribute ibmvscsi_host_srp_version = {
	.attr = {
		 .name = "srp_version",
		 .mode = S_IRUGO,
		 },
	.show = show_host_srp_version,
};

static ssize_t show_host_partition_name(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, PAGE_SIZE, "%s\n",
		       hostdata->madapter_info.partition_name);
	return len;
}

static struct device_attribute ibmvscsi_host_partition_name = {
	.attr = {
		 .name = "partition_name",
		 .mode = S_IRUGO,
		 },
	.show = show_host_partition_name,
};

static ssize_t show_host_partition_number(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, PAGE_SIZE, "%d\n",
		       hostdata->madapter_info.partition_number);
	return len;
}

static struct device_attribute ibmvscsi_host_partition_number = {
	.attr = {
		 .name = "partition_number",
		 .mode = S_IRUGO,
		 },
	.show = show_host_partition_number,
};

static ssize_t show_host_mad_version(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, PAGE_SIZE, "%d\n",
		       hostdata->madapter_info.mad_version);
	return len;
}

static struct device_attribute ibmvscsi_host_mad_version = {
	.attr = {
		 .name = "mad_version",
		 .mode = S_IRUGO,
		 },
	.show = show_host_mad_version,
};

static ssize_t show_host_os_type(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);
	int len;

	len = snprintf(buf, PAGE_SIZE, "%d\n", hostdata->madapter_info.os_type);
	return len;
}

static struct device_attribute ibmvscsi_host_os_type = {
	.attr = {
		 .name = "os_type",
		 .mode = S_IRUGO,
		 },
	.show = show_host_os_type,
};

static ssize_t show_host_config(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvscsi_host_data *hostdata = shost_priv(shost);

	/* returns null-terminated host config data */
	if (ibmvscsi_do_host_config(hostdata, buf, PAGE_SIZE) == 0)
		return strlen(buf);
	else
		return 0;
}

static struct device_attribute ibmvscsi_host_config = {
	.attr = {
		 .name = "config",
		 .mode = S_IRUGO,
		 },
	.show = show_host_config,
};

static struct device_attribute *ibmvscsi_attrs[] = {
	&ibmvscsi_host_vhost_loc,
	&ibmvscsi_host_vhost_name,
	&ibmvscsi_host_srp_version,
	&ibmvscsi_host_partition_name,
	&ibmvscsi_host_partition_number,
	&ibmvscsi_host_mad_version,
	&ibmvscsi_host_os_type,
	&ibmvscsi_host_config,
	NULL
};

/* ------------------------------------------------------------
 * SCSI driver registration
 */
static struct scsi_host_template driver_template = {
	.module = THIS_MODULE,
	.name = "IBM POWER Virtual SCSI Adapter " IBMVSCSI_VERSION,
	.proc_name = "ibmvscsi",
	.queuecommand = ibmvscsi_queuecommand,
	.eh_abort_handler = ibmvscsi_eh_abort_handler,
	.eh_device_reset_handler = ibmvscsi_eh_device_reset_handler,
	.eh_host_reset_handler = ibmvscsi_eh_host_reset_handler,
	.slave_configure = ibmvscsi_slave_configure,
	.change_queue_depth = ibmvscsi_