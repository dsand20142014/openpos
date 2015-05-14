requests(vhost, DID_ERROR);
			if ((rc = ibmvfc_reset_crq(vhost)) ||
			    (rc = ibmvfc_send_crq_init(vhost))) {
				ibmvfc_link_down(vhost, IBMVFC_LINK_DEAD);
				dev_err(vhost->dev, "Error after reset (rc=%ld)\n", rc);
			} else
				ibmvfc_link_down(vhost, IBMVFC_LINK_DOWN);
		}
		return;
	case IBMVFC_CRQ_CMD_RSP:
		break;
	default:
		dev_err(vhost->dev, "Got an invalid message type 0x%02x\n", crq->valid);
		return;
	}

	if (crq->format == IBMVFC_ASYNC_EVENT)
		return;

	/* The only kind of payload CRQs we should get are responses to
	 * things we send. Make sure this response is to something we
	 * actually sent
	 */
	if (unlikely(!ibmvfc_valid_event(&vhost->pool, evt))) {
		dev_err(vhost->dev, "Returned correlation_token 0x%08llx is invalid!\n",
			crq->ioba);
		return;
	}

	if (unlikely(atomic_read(&evt->free))) {
		dev_err(vhost->dev, "Received duplicate correlation_token 0x%08llx!\n",
			crq->ioba);
		return;
	}

	del_timer(&evt->timer);
	list_del(&evt->queue);
	ibmvfc_trc_end(evt);
	evt->done(evt);
}

/**
 * ibmvfc_scan_finished - Check if the device scan is done.
 * @shost:	scsi host struct
 * @time:	current elapsed time
 *
 * Returns:
 *	0 if scan is not done / 1 if scan is done
 **/
static int ibmvfc_scan_finished(struct Scsi_Host *shost, unsigned long time)
{
	unsigned long flags;
	struct ibmvfc_host *vhost = shost_priv(shost);
	int done = 0;

	spin_lock_irqsave(shost->host_lock, flags);
	if (time >= (init_timeout * HZ)) {
		dev_info(vhost->dev, "Scan taking longer than %d seconds, "
			 "continuing initialization\n", init_timeout);
		done = 1;
	}

	if (vhost->scan_complete)
		done = 1;
	spin_unlock_irqrestore(shost->host_lock, flags);
	return done;
}

/**
 * ibmvfc_slave_alloc - Setup the device's task set value
 * @sdev:	struct scsi_device device to configure
 *
 * Set the device's task set value so that error handling works as
 * expected.
 *
 * Returns:
 *	0 on success / -ENXIO if device does not exist
 **/
static int ibmvfc_slave_alloc(struct scsi_device *sdev)
{
	struct Scsi_Host *shost = sdev->host;
	struct fc_rport *rport = starget_to_rport(scsi_target(sdev));
	struct ibmvfc_host *vhost = shost_priv(shost);
	unsigned long flags = 0;

	if (!rport || fc_remote_port_chkready(rport))
		return -ENXIO;

	spin_lock_irqsave(shost->host_lock, flags);
	sdev->hostdata = (void *)(unsigned long)vhost->task_set++;
	spin_unlock_irqrestore(shost->host_lock, flags);
	return 0;
}

/**
 * ibmvfc_target_alloc - Setup the target's task set value
 * @starget:	struct scsi_target
 *
 * Set the target's task set value so that error handling works as
 * expected.
 *
 * Returns:
 *	0 on success / -ENXIO if device does not exist
 **/
static int ibmvfc_target_alloc(struct scsi_target *starget)
{
	struct Scsi_Host *shost = dev_to_shost(starget->dev.parent);
	struct ibmvfc_host *vhost = shost_priv(shost);
	unsigned long flags = 0;

	spin_lock_irqsave(shost->host_lock, flags);
	starget->hostdata = (void *)(unsigned long)vhost->task_set++;
	spin_unlock_irqrestore(shost->host_lock, flags);
	return 0;
}

/**
 * ibmvfc_slave_configure - Configure the device
 * @sdev:	struct scsi_device device to configure
 *
 * Enable allow_restart for a device if it is a disk. Adjust the
 * queue_depth here also.
 *
 * Returns:
 *	0
 **/
static int ibmvfc_slave_configure(struct scsi_device *sdev)
{
	struct Scsi_Host *shost = sdev->host;
	struct fc_rport *rport = starget_to_rport(sdev->sdev_target);
	unsigned long flags = 0;

	spin_lock_irqsave(shost->host_lock, flags);
	if (sdev->type == TYPE_DISK)
		sdev->allow_restart = 1;

	if (sdev->tagged_supported) {
		scsi_set_tag_type(sdev, MSG_SIMPLE_TAG);
		scsi_activate_tcq(sdev, sdev->queue_depth);
	} else
		scsi_deactivate_tcq(sdev, sdev->queue_depth);

	rport->dev_loss_tmo = dev_loss_tmo;
	spin_unlock_irqrestore(shost->host_lock, flags);
	return 0;
}

/**
 * ibmvfc_change_queue_depth - Change the device's queue depth
 * @sdev:	scsi device struct
 * @qdepth:	depth to set
 *
 * Return value:
 * 	actual depth set
 **/
static int ibmvfc_change_queue_depth(struct scsi_device *sdev, int qdepth)
{
	if (qdepth > IBMVFC_MAX_CMDS_PER_LUN)
		qdepth = IBMVFC_MAX_CMDS_PER_LUN;

	scsi_adjust_queue_depth(sdev, 0, qdepth);
	return sdev->queue_depth;
}

/**
 * ibmvfc_change_queue_type - Change the device's queue type
 * @sdev:		scsi device struct
 * @tag_type:	type of tags to use
 *
 * Return value:
 * 	actual queue type set
 **/
static int ibmvfc_change_queue_type(struct scsi_device *sdev, int tag_type)
{
	if (sdev->tagged_supported) {
		scsi_set_tag_type(sdev, tag_type);

		if (tag_type)
			scsi_activate_tcq(sdev, sdev->queue_depth);
		else
			scsi_deactivate_tcq(sdev, sdev->queue_depth);
	} else
		tag_type = 0;

	return tag_type;
}

static ssize_t ibmvfc_show_host_partition_name(struct device *dev,
						 struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);

	return snprintf(buf, PAGE_SIZE, "%s\n",
			vhost->login_buf->resp.partition_name);
}

static ssize_t ibmvfc_show_host_device_name(struct device *dev,
					    struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);

	return snprintf(buf, PAGE_SIZE, "%s\n",
			vhost->login_buf->resp.device_name);
}

static ssize_t ibmvfc_show_host_loc_code(struct device *dev,
					 struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);

	return snprintf(buf, PAGE_SIZE, "%s\n",
			vhost->login_buf->resp.port_loc_code);
}

static ssize_t ibmvfc_show_host_drc_name(struct device *dev,
					 struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);

	return snprintf(buf, PAGE_SIZE, "%s\n",
			vhost->login_buf->resp.drc_name);
}

static ssize_t ibmvfc_show_host_npiv_version(struct device *dev,
					     struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);
	return snprintf(buf, PAGE_SIZE, "%d\n", vhost->login_buf->resp.version);
}

static ssize_t ibmvfc_show_host_capabilities(struct device *dev,
					     struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);
	return snprintf(buf, PAGE_SIZE, "%llx\n", vhost->login_buf->resp.capabilities);
}

/**
 * ibmvfc_show_log_level - Show the adapter's error logging level
 * @dev:	class device struct
 * @buf:	buffer
 *
 * Return value:
 * 	number of bytes printed to buffer
 **/
static ssize_t ibmvfc_show_log_level(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);
	unsigned long flags = 0;
	int len;

	spin_lock_irqsave(shost->host_lock, flags);
	len = snprintf(buf, PAGE_SIZE, "%d\n", vhost->log_level);
	spin_unlock_irqrestore(shost->host_lock, flags);
	return len;
}

/**
 * ibmvfc_store_log_level - Change the adapter's error logging level
 * @dev:	class device struct
 * @buf:	buffer
 *
 * Return value:
 * 	number of bytes printed to buffer
 **/
static ssize_t ibmvfc_store_log_level(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);
	unsigned long flags = 0;

	spin_lock_irqsave(shost->host_lock, flags);
	vhost->log_level = simple_strtoul(buf, NULL, 10);
	spin_unlock_irqrestore(shost->host_lock, flags);
	return strlen(buf);
}

static DEVICE_ATTR(partition_name, S_IRUGO, ibmvfc_show_host_partition_name, NULL);
static DEVICE_ATTR(device_name, S_IRUGO, ibmvfc_show_host_device_name, NULL);
static DEVICE_ATTR(port_loc_code, S_IRUGO, ibmvfc_show_host_loc_code, NULL);
static DEVICE_ATTR(drc_name, S_IRUGO, ibmvfc_show_host_drc_name, NULL);
static DEVICE_ATTR(npiv_version, S_IRUGO, ibmvfc_show_host_npiv_version, NULL);
static DEVICE_ATTR(capabilities, S_IRUGO, ibmvfc_show_host_capabilities, NULL);
static DEVICE_ATTR(log_level, S_IRUGO | S_IWUSR,
		   ibmvfc_show_log_level, ibmvfc_store_log_level);

#ifdef CONFIG_SCSI_IBMVFC_TRACE
/**
 * ibmvfc_read_trace - Dump the adapter trace
 * @kobj:		kobject struct
 * @bin_attr:	bin_attribute struct
 * @buf:		buffer
 * @off:		offset
 * @count:		buffer size
 *
 * Return value:
 *	number of bytes printed to buffer
 **/
static ssize_t ibmvfc_read_trace(struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct Scsi_Host *shost = class_to_shost(dev);
	struct ibmvfc_host *vhost = shost_priv(shost);
	unsigned long flags = 0;
	int size = IBMVFC_TRACE_SIZE;
	char *src = (char *)vhost->trace;

	if (off > size)
		return 0;
	if (off + count > size) {
		size -= off;
		count = size;
	}

	spin_lock_irqsave(shost->host_lock, flags);
	memcpy(buf, &src[off], count);
	spin_unlock_irqrestore(shost->host_lock, flags);
	return count;
}

static struct bin_attribute ibmvfc_trace_attr = {
	.attr =	{
		.name = "trace",
		.mode = S_IRUGO,
	},
	.size = 0,
	.read = ibmvfc_read_trace,
};
#endif

static struct device_attribute *ibmvfc_attrs[] = {
	&dev_attr_partition_name,
	&dev_attr_device_name,
	&dev_attr_port_loc_code,
	&dev_attr_drc_name,
	&dev_attr_npiv_version,
	&dev_attr_capabilities,
	&dev_attr_log_level,
	NULL
};

static struct scsi_host_template driver_template = {
	.module = THIS_MODULE,
	.name = "IBM POWER Virtual FC Adapter",
	.proc_name = IBMVFC_NAME,
	.queuecommand = ibmvfc_queuecommand,
	.eh_abort_handler = ibmvfc_eh_abort_handler,
	.eh_device_reset_handler = ibmvfc_eh_device_reset_handler,
	.eh_target_reset_handler = ibmvfc_eh_target_reset_handler,
	.eh_host_reset_handler = ibmvfc_eh_host_reset_handler,
	.slave_alloc = ibmvfc_slave_alloc,
	.slave_configure = ibmvfc_slave_configure,
	.target_alloc = ibmvfc_target_alloc,
	.scan_finished = ibmvfc_scan_finished,
	.change_queue_depth = ibmvfc_change_queue_depth,
	.change_queue_type = ibmvfc_change_queue_type,
	.cmd_per_lun = 16,
	.can_queue = IBMVFC_MAX_REQUESTS_DEFAULT,
	.this_id = -1,
	.sg_tablesize = SG_ALL,
	.max_sectors = IBMVFC_MAX_SECTORS,
	.use_clustering = ENABLE_CLUSTERING,
	.shost_attrs = ibmvfc_attrs,
};

/**
 * ibmvfc_next_async_crq - Returns the next entry in async queue
 * @vhost:	ibmvfc host struct
 *
 * Returns:
 *	Pointer to next entry in queue / NULL if empty
 **/
static struct ibmvfc_async_crq *ibmvfc_next_async_crq(struct ibmvfc_host *vhost)
{
	struct ibmvfc_async_crq_queue *async_crq = &vhost->async_crq;
	struct ibmvfc_async_crq *crq;

	crq = &async_crq->msgs[async_crq->cur];
	if (crq->valid & 0x80) {
		if (++async_crq->cur == async_crq->size)
			async_crq->cur = 0;
	} else
		crq = NULL;

	return crq;
}

/**
 * ibmvfc_next_crq - Returns the next entry in message queue
 * @vhost:	ibmvfc host struct
 *
 * Returns:
 *	Pointer to next entry in queue / NULL if empty
 **/
static struct ibmvfc_crq *ibmvfc_next_crq(struct ibmvfc_host *vhost)
{
	struct ibmvfc_crq_queue *queue = &vhost->crq;
	struct ibmvfc_crq *crq;

	crq = &queue->msgs[queue->cur];
	if (crq->valid & 0x80) {
		if (++queue->cur == queue->size)
			queue->cur = 0;
	} else
		crq = NULL;

	return crq;
}

/**
 * ibmvfc_interrupt - Interrupt handler
 * @irq:		number of irq to handle, not used
 * @dev_instance: ibmvfc_host that received interrupt
 *
 * Returns:
 *	IRQ_HANDLED
 **/
static irqreturn_t ibmvfc_interrupt(int irq, void *dev_instance)
{
	struct ibmvfc_host *vhost = (struct ibmvfc_host *)dev_instance;
	unsigned long flags;

	spin_lock_irqsave(vhost->host->host_lock, flags);
	vio_disable_interrupts(to_vio_dev(vhost->dev));
	tasklet_schedule(&vhost->tasklet);
	spin_unlock_irqrestore(vhost->host->host_lock, flags);
	return IRQ_HANDLED;
}

/**
 * ibmvfc_tasklet - Interrupt handler tasklet
 * @data:		ibmvfc host struct
 *
 * Returns:
 *	Nothing
 **/
static void ibmvfc_tasklet(void *data)
{
	struct ibmvfc_host *vhost = data;
	struct vio_dev *vdev = to_vio_dev(vhost->dev);
	struct ibmvfc_crq *crq;
	struct ibmvfc_async_crq *async;
	unsigned long flags;
	int done = 0;

	spin_lock_irqsave(vhost->host->host_lock, flags);
	while (!done) {
		/* Pull all the valid messages off the async CRQ */
		while ((async = ibmvfc_next_async_crq(vhost)) != NULL) {
			ibmvfc_handle_async(async, vhost);
			async->valid = 0;
		}

		/* Pull all the valid messages off the CRQ */
		while ((crq = ibmvfc_next_crq(vhost)) != NULL) {
			ibmvfc_handle_crq(crq, vhost);
			crq->valid = 0;
		}

		vio_enable_interrupts(vdev);
		if ((async = ibmvfc_next_async_crq(vhost)) != NULL) {
			vio_disable_interrupts(vdev);
			ibmvfc_handle_async(async, vhost);
			async->valid = 0;
		} else if ((crq = ibmvfc_next_crq(vhost)) != NULL) {
			vio_disable_interrupts(vdev);
			ibmvfc_handle_crq(crq, vhost);
			crq->valid = 0;
		} else
			done = 1;
	}

	spin_unlock_irqrestore(vhost->host->host_lock, flags);
}

/**
 * ibmvfc_init_tgt - Set the next init job step for the target
 * @tgt:		ibmvfc target struct
 * @job_step:	job step to perform
 *
 **/
static void ibmvfc_init_tgt(struct ibmvfc_target *tgt,
			    void (*job_step) (struct ibmvfc_target *))
{
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_INIT);
	tgt->job_step = job_step;
	wake_up(&tgt->vhost->work_wait_q);
}

/**
 * ibmvfc_retry_tgt_init - Attempt to retry a step in target initialization
 * @tgt:		ibmvfc target struct
 * @job_step:	initialization job step
 *
 * Returns: 1 if step will be retried / 0 if not
 *
 **/
static int ibmvfc_retry_tgt_init(struct ibmvfc_target *tgt,
				  void (*job_step) (struct ibmvfc_target *))
{
	if (++tgt->init_retries > IBMVFC_MAX_TGT_INIT_RETRIES) {
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
		wake_up(&tgt->vhost->work_wait_q);
		return 0;
	} else
		ibmvfc_init_tgt(tgt, job_step);
	return 1;
}

/* Defined in FC-LS */
static const struct {
	int code;
	int retry;
	int logged_in;
} prli_rsp [] = {
	{ 0, 1, 0 },
	{ 1, 0, 1 },
	{ 2, 1, 0 },
	{ 3, 1, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 1 },
	{ 7, 0, 0 },
	{ 8, 1, 0 },
};

/**
 * ibmvfc_get_prli_rsp - Find PRLI response index
 * @flags:	PRLI response flags
 *
 **/
static int ibmvfc_get_prli_rsp(u16 flags)
{
	int i;
	int code = (flags & 0x0f00) >> 8;

	for (i = 0; i < ARRAY_SIZE(prli_rsp); i++)
		if (prli_rsp[i].code == code)
			return i;

	return 0;
}

/**
 * ibmvfc_tgt_prli_done - Completion handler for Process Login
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_tgt_prli_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_target *tgt = evt->tgt;
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_process_login *rsp = &evt->xfer_iu->prli;
	struct ibmvfc_prli_svc_parms *parms = &rsp->parms;
	u32 status = rsp->common.status;
	int index, level = IBMVFC_DEFAULT_LOG_LEVEL;

	vhost->discovery_threads--;
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
	switch (status) {
	case IBMVFC_MAD_SUCCESS:
		tgt_dbg(tgt, "Process Login succeeded: %X %02X %04X\n",
			parms->type, parms->flags, parms->service_parms);

		if (parms->type == IBMVFC_SCSI_FCP_TYPE) {
			index = ibmvfc_get_prli_rsp(parms->flags);
			if (prli_rsp[index].logged_in) {
				if (parms->flags & IBMVFC_PRLI_EST_IMG_PAIR) {
					tgt->need_login = 0;
					tgt->ids.roles = 0;
					if (parms->service_parms & IBMVFC_PRLI_TARGET_FUNC)
						tgt->ids.roles |= FC_PORT_ROLE_FCP_TARGET;
					if (parms->service_parms & IBMVFC_PRLI_INITIATOR_FUNC)
						tgt->ids.roles |= FC_PORT_ROLE_FCP_INITIATOR;
					tgt->add_rport = 1;
				} else
					ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
			} else if (prli_rsp[index].retry)
				ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_prli);
			else
				ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
		} else
			ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
		break;
	case IBMVFC_MAD_DRIVER_FAILED:
		break;
	case IBMVFC_MAD_CRQ_ERROR:
		ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_prli);
		break;
	case IBMVFC_MAD_FAILED:
	default:
		if ((rsp->status & IBMVFC_VIOS_FAILURE) && rsp->error == IBMVFC_PLOGI_REQUIRED)
			level += ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_plogi);
		else if (tgt->logo_rcvd)
			level += ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_plogi);
		else if (ibmvfc_retry_cmd(rsp->status, rsp->error))
			level += ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_prli);
		else
			ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);

		tgt_log(tgt, level, "Process Login failed: %s (%x:%x) rc=0x%02X\n",
			ibmvfc_get_cmd_error(rsp->status, rsp->error),
			rsp->status, rsp->error, status);
		break;
	};

	kref_put(&tgt->kref, ibmvfc_release_tgt);
	ibmvfc_free_event(evt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_tgt_send_prli - Send a process login
 * @tgt:	ibmvfc target struct
 *
 **/
static void ibmvfc_tgt_send_prli(struct ibmvfc_target *tgt)
{
	struct ibmvfc_process_login *prli;
	struct ibmvfc_host *vhost = tgt->vhost;
	struct ibmvfc_event *evt;

	if (vhost->discovery_threads >= disc_threads)
		return;

	kref_get(&tgt->kref);
	evt = ibmvfc_get_event(vhost);
	vhost->discovery_threads++;
	ibmvfc_init_event(evt, ibmvfc_tgt_prli_done, IBMVFC_MAD_FORMAT);
	evt->tgt = tgt;
	prli = &evt->iu.prli;
	memset(prli, 0, sizeof(*prli));
	prli->common.version = 1;
	prli->common.opcode = IBMVFC_PROCESS_LOGIN;
	prli->common.length = sizeof(*prli);
	prli->scsi_id = tgt->scsi_id;

	prli->parms.type = IBMVFC_SCSI_FCP_TYPE;
	prli->parms.flags = IBMVFC_PRLI_EST_IMG_PAIR;
	prli->parms.service_parms = IBMVFC_PRLI_INITIATOR_FUNC;

	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_INIT_WAIT);
	if (ibmvfc_send_event(evt, vhost, default_timeout)) {
		vhost->discovery_threads--;
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
		kref_put(&tgt->kref, ibmvfc_release_tgt);
	} else
		tgt_dbg(tgt, "Sent process login\n");
}

/**
 * ibmvfc_tgt_plogi_done - Completion handler for Port Login
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_tgt_plogi_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_target *tgt = evt->tgt;
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_port_login *rsp = &evt->xfer_iu->plogi;
	u32 status = rsp->common.status;
	int level = IBMVFC_DEFAULT_LOG_LEVEL;

	vhost->discovery_threads--;
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
	switch (status) {
	case IBMVFC_MAD_SUCCESS:
		tgt_dbg(tgt, "Port Login succeeded\n");
		if (tgt->ids.port_name &&
		    tgt->ids.port_name != wwn_to_u64(rsp->service_parms.port_name)) {
			vhost->reinit = 1;
			tgt_dbg(tgt, "Port re-init required\n");
			break;
		}
		tgt->ids.node_name = wwn_to_u64(rsp->service_parms.node_name);
		tgt->ids.port_name = wwn_to_u64(rsp->service_parms.port_name);
		tgt->ids.port_id = tgt->scsi_id;
		memcpy(&tgt->service_parms, &rsp->service_parms,
		       sizeof(tgt->service_parms));
		memcpy(&tgt->service_parms_change, &rsp->service_parms_change,
		       sizeof(tgt->service_parms_change));
		ibmvfc_init_tgt(tgt, ibmvfc_tgt_send_prli);
		break;
	case IBMVFC_MAD_DRIVER_FAILED:
		break;
	case IBMVFC_MAD_CRQ_ERROR:
		ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_plogi);
		break;
	case IBMVFC_MAD_FAILED:
	default:
		if (ibmvfc_retry_cmd(rsp->status, rsp->error))
			level += ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_send_plogi);
		else
			ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);

		tgt_log(tgt, level, "Port Login failed: %s (%x:%x) %s (%x) %s (%x) rc=0x%02X\n",
			ibmvfc_get_cmd_error(rsp->status, rsp->error), rsp->status, rsp->error,
			ibmvfc_get_fc_type(rsp->fc_type), rsp->fc_type,
			ibmvfc_get_ls_explain(rsp->fc_explain), rsp->fc_explain, status);
		break;
	};

	kref_put(&tgt->kref, ibmvfc_release_tgt);
	ibmvfc_free_event(evt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_tgt_send_plogi - Send PLOGI to the specified target
 * @tgt:	ibmvfc target struct
 *
 **/
static void ibmvfc_tgt_send_plogi(struct ibmvfc_target *tgt)
{
	struct ibmvfc_port_login *plogi;
	struct ibmvfc_host *vhost = tgt->vhost;
	struct ibmvfc_event *evt;

	if (vhost->discovery_threads >= disc_threads)
		return;

	kref_get(&tgt->kref);
	tgt->logo_rcvd = 0;
	evt = ibmvfc_get_event(vhost);
	vhost->discovery_threads++;
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_INIT_WAIT);
	ibmvfc_init_event(evt, ibmvfc_tgt_plogi_done, IBMVFC_MAD_FORMAT);
	evt->tgt = tgt;
	plogi = &evt->iu.plogi;
	memset(plogi, 0, sizeof(*plogi));
	plogi->common.version = 1;
	plogi->common.opcode = IBMVFC_PORT_LOGIN;
	plogi->common.length = sizeof(*plogi);
	plogi->scsi_id = tgt->scsi_id;

	if (ibmvfc_send_event(evt, vhost, default_timeout)) {
		vhost->discovery_threads--;
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
		kref_put(&tgt->kref, ibmvfc_release_tgt);
	} else
		tgt_dbg(tgt, "Sent port login\n");
}

/**
 * ibmvfc_tgt_implicit_logout_done - Completion handler for Implicit Logout MAD
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_tgt_implicit_logout_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_target *tgt = evt->tgt;
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_implicit_logout *rsp = &evt->xfer_iu->implicit_logout;
	u32 status = rsp->common.status;

	vhost->discovery_threads--;
	ibmvfc_free_event(evt);
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);

	switch (status) {
	case IBMVFC_MAD_SUCCESS:
		tgt_dbg(tgt, "Implicit Logout succeeded\n");
		break;
	case IBMVFC_MAD_DRIVER_FAILED:
		kref_put(&tgt->kref, ibmvfc_release_tgt);
		wake_up(&vhost->work_wait_q);
		return;
	case IBMVFC_MAD_FAILED:
	default:
		tgt_err(tgt, "Implicit Logout failed: rc=0x%02X\n", status);
		break;
	};

	if (vhost->action == IBMVFC_HOST_ACTION_TGT_INIT)
		ibmvfc_init_tgt(tgt, ibmvfc_tgt_send_plogi);
	else if (vhost->action == IBMVFC_HOST_ACTION_QUERY_TGTS &&
		 tgt->scsi_id != tgt->new_scsi_id)
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
	kref_put(&tgt->kref, ibmvfc_release_tgt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_tgt_implicit_logout - Initiate an Implicit Logout for specified target
 * @tgt:		ibmvfc target struct
 *
 **/
static void ibmvfc_tgt_implicit_logout(struct ibmvfc_target *tgt)
{
	struct ibmvfc_implicit_logout *mad;
	struct ibmvfc_host *vhost = tgt->vhost;
	struct ibmvfc_event *evt;

	if (vhost->discovery_threads >= disc_threads)
		return;

	kref_get(&tgt->kref);
	evt = ibmvfc_get_event(vhost);
	vhost->discovery_threads++;
	ibmvfc_init_event(evt, ibmvfc_tgt_implicit_logout_done, IBMVFC_MAD_FORMAT);
	evt->tgt = tgt;
	mad = &evt->iu.implicit_logout;
	memset(mad, 0, sizeof(*mad));
	mad->common.version = 1;
	mad->common.opcode = IBMVFC_IMPLICIT_LOGOUT;
	mad->common.length = sizeof(*mad);
	mad->old_scsi_id = tgt->scsi_id;

	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_INIT_WAIT);
	if (ibmvfc_send_event(evt, vhost, default_timeout)) {
		vhost->discovery_threads--;
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
		kref_put(&tgt->kref, ibmvfc_release_tgt);
	} else
		tgt_dbg(tgt, "Sent Implicit Logout\n");
}

/**
 * ibmvfc_adisc_needs_plogi - Does device need PLOGI?
 * @mad:	ibmvfc passthru mad struct
 * @tgt:	ibmvfc target struct
 *
 * Returns:
 *	1 if PLOGI needed / 0 if PLOGI not needed
 **/
static int ibmvfc_adisc_needs_plogi(struct ibmvfc_passthru_mad *mad,
				    struct ibmvfc_target *tgt)
{
	if (memcmp(&mad->fc_iu.response[2], &tgt->ids.port_name,
		   sizeof(tgt->ids.port_name)))
		return 1;
	if (memcmp(&mad->fc_iu.response[4], &tgt->ids.node_name,
		   sizeof(tgt->ids.node_name)))
		return 1;
	if (mad->fc_iu.response[6] != tgt->scsi_id)
		return 1;
	return 0;
}

/**
 * ibmvfc_tgt_adisc_done - Completion handler for ADISC
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_tgt_adisc_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_target *tgt = evt->tgt;
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_passthru_mad *mad = &evt->xfer_iu->passthru;
	u32 status = mad->common.status;
	u8 fc_reason, fc_explain;

	vhost->discovery_threads--;
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
	del_timer(&tgt->timer);

	switch (status) {
	case IBMVFC_MAD_SUCCESS:
		tgt_dbg(tgt, "ADISC succeeded\n");
		if (ibmvfc_adisc_needs_plogi(mad, tgt))
			ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
		break;
	case IBMVFC_MAD_DRIVER_FAILED:
		break;
	case IBMVFC_MAD_FAILED:
	default:
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
		fc_reason = (mad->fc_iu.response[1] & 0x00ff0000) >> 16;
		fc_explain = (mad->fc_iu.response[1] & 0x0000ff00) >> 8;
		tgt_info(tgt, "ADISC failed: %s (%x:%x) %s (%x) %s (%x) rc=0x%02X\n",
			 ibmvfc_get_cmd_error(mad->iu.status, mad->iu.error),
			 mad->iu.status, mad->iu.error,
			 ibmvfc_get_fc_type(fc_reason), fc_reason,
			 ibmvfc_get_ls_explain(fc_explain), fc_explain, status);
		break;
	};

	kref_put(&tgt->kref, ibmvfc_release_tgt);
	ibmvfc_free_event(evt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_init_passthru - Initialize an event struct for FC passthru
 * @evt:		ibmvfc event struct
 *
 **/
static void ibmvfc_init_passthru(struct ibmvfc_event *evt)
{
	struct ibmvfc_passthru_mad *mad = &evt->iu.passthru;

	memset(mad, 0, sizeof(*mad));
	mad->common.version = 1;
	mad->common.opcode = IBMVFC_PASSTHRU;
	mad->common.length = sizeof(*mad) - sizeof(mad->fc_iu) - sizeof(mad->iu);
	mad->cmd_ioba.va = (u64)evt->crq.ioba +
		offsetof(struct ibmvfc_passthru_mad, iu);
	mad->cmd_ioba.len = sizeof(mad->iu);
	mad->iu.cmd_len = sizeof(mad->fc_iu.payload);
	mad->iu.rsp_len = sizeof(mad->fc_iu.response);
	mad->iu.cmd.va = (u64)evt->crq.ioba +
		offsetof(struct ibmvfc_passthru_mad, fc_iu) +
		offsetof(struct ibmvfc_passthru_fc_iu, payload);
	mad->iu.cmd.len = sizeof(mad->fc_iu.payload);
	mad->iu.rsp.va = (u64)evt->crq.ioba +
		offsetof(struct ibmvfc_passthru_mad, fc_iu) +
		offsetof(struct ibmvfc_passthru_fc_iu, response);
	mad->iu.rsp.len = sizeof(mad->fc_iu.response);
}

/**
 * ibmvfc_tgt_adisc_cancel_done - Completion handler when cancelling an ADISC
 * @evt:		ibmvfc event struct
 *
 * Just cleanup this event struct. Everything else is handled by
 * the ADISC completion handler. If the ADISC never actually comes
 * back, we still have the timer running on the ADISC event struct
 * which will fire and cause the CRQ to get reset.
 *
 **/
static void ibmvfc_tgt_adisc_cancel_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_target *tgt = evt->tgt;

	tgt_dbg(tgt, "ADISC cancel complete\n");
	vhost->abort_threads--;
	ibmvfc_free_event(evt);
	kref_put(&tgt->kref, ibmvfc_release_tgt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_adisc_timeout - Handle an ADISC timeout
 * @tgt:		ibmvfc target struct
 *
 * If an ADISC times out, send a cancel. If the cancel times
 * out, reset the CRQ. When the ADISC comes back as cancelled,
 * log back into the target.
 **/
static void ibmvfc_adisc_timeout(struct ibmvfc_target *tgt)
{
	struct ibmvfc_host *vhost = tgt->vhost;
	struct ibmvfc_event *evt;
	struct ibmvfc_tmf *tmf;
	unsigned long flags;
	int rc;

	tgt_dbg(tgt, "ADISC timeout\n");
	spin_lock_irqsave(vhost->host->host_lock, flags);
	if (vhost->abort_threads >= disc_threads ||
	    tgt->action != IBMVFC_TGT_ACTION_INIT_WAIT ||
	    vhost->state != IBMVFC_INITIALIZING ||
	    vhost->action != IBMVFC_HOST_ACTION_QUERY_TGTS) {
		spin_unlock_irqrestore(vhost->host->host_lock, flags);
		return;
	}

	vhost->abort_threads++;
	kref_get(&tgt->kref);
	evt = ibmvfc_get_event(vhost);
	ibmvfc_init_event(evt, ibmvfc_tgt_adisc_cancel_done, IBMVFC_MAD_FORMAT);

	evt->tgt = tgt;
	tmf = &evt->iu.tmf;
	memset(tmf, 0, sizeof(*tmf));
	tmf->common.version = 1;
	tmf->common.opcode = IBMVFC_TMF_MAD;
	tmf->common.length = sizeof(*tmf);
	tmf->scsi_id = tgt->scsi_id;
	tmf->cancel_key = tgt->cancel_key;

	rc = ibmvfc_send_event(evt, vhost, default_timeout);

	if (rc) {
		tgt_err(tgt, "Failed to send cancel event for ADISC. rc=%d\n", rc);
		vhost->abort_threads--;
		kref_put(&tgt->kref, ibmvfc_release_tgt);
		__ibmvfc_reset_host(vhost);
	} else
		tgt_dbg(tgt, "Attempting to cancel ADISC\n");
	spin_unlock_irqrestore(vhost->host->host_lock, flags);
}

/**
 * ibmvfc_tgt_adisc - Initiate an ADISC for specified target
 * @tgt:		ibmvfc target struct
 *
 * When sending an ADISC we end up with two timers running. The
 * first timer is the timer in the ibmvfc target struct. If this
 * fires, we send a cancel to the target. The second timer is the
 * timer on the ibmvfc event for the ADISC, which is longer. If that
 * fires, it means the ADISC timed out and our attempt to cancel it
 * also failed, so we need to reset the CRQ.
 **/
static void ibmvfc_tgt_adisc(struct ibmvfc_target *tgt)
{
	struct ibmvfc_passthru_mad *mad;
	struct ibmvfc_host *vhost = tgt->vhost;
	struct ibmvfc_event *evt;

	if (vhost->discovery_threads >= disc_threads)
		return;

	kref_get(&tgt->kref);
	evt = ibmvfc_get_event(vhost);
	vhost->discovery_threads++;
	ibmvfc_init_event(evt, ibmvfc_tgt_adisc_done, IBMVFC_MAD_FORMAT);
	evt->tgt = tgt;

	ibmvfc_init_passthru(evt);
	mad = &evt->iu.passthru;
	mad->iu.flags = IBMVFC_FC_ELS;
	mad->iu.scsi_id = tgt->scsi_id;
	mad->iu.cancel_key = tgt->cancel_key;

	mad->fc_iu.payload[0] = IBMVFC_ADISC;
	memcpy(&mad->fc_iu.payload[2], &vhost->login_buf->resp.port_name,
	       sizeof(vhost->login_buf->resp.port_name));
	memcpy(&mad->fc_iu.payload[4], &vhost->login_buf->resp.node_name,
	       sizeof(vhost->login_buf->resp.node_name));
	mad->fc_iu.payload[6] = vhost->login_buf->resp.scsi_id & 0x00ffffff;

	if (timer_pending(&tgt->timer))
		mod_timer(&tgt->timer, jiffies + (IBMVFC_ADISC_TIMEOUT * HZ));
	else {
		tgt->timer.data = (unsigned long) tgt;
		tgt->timer.expires = jiffies + (IBMVFC_ADISC_TIMEOUT * HZ);
		tgt->timer.function = (void (*)(unsigned long))ibmvfc_adisc_timeout;
		add_timer(&tgt->timer);
	}

	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_INIT_WAIT);
	if (ibmvfc_send_event(evt, vhost, IBMVFC_ADISC_PLUS_CANCEL_TIMEOUT)) {
		vhost->discovery_threads--;
		del_timer(&tgt->timer);
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
		kref_put(&tgt->kref, ibmvfc_release_tgt);
	} else
		tgt_dbg(tgt, "Sent ADISC\n");
}

/**
 * ibmvfc_tgt_query_target_done - Completion handler for Query Target MAD
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_tgt_query_target_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_target *tgt = evt->tgt;
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_query_tgt *rsp = &evt->xfer_iu->query_tgt;
	u32 status = rsp->common.status;
	int level = IBMVFC_DEFAULT_LOG_LEVEL;

	vhost->discovery_threads--;
	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
	switch (status) {
	case IBMVFC_MAD_SUCCESS:
		tgt_dbg(tgt, "Query Target succeeded\n");
		tgt->new_scsi_id = rsp->scsi_id;
		if (rsp->scsi_id != tgt->scsi_id)
			ibmvfc_init_tgt(tgt, ibmvfc_tgt_implicit_logout);
		else
			ibmvfc_init_tgt(tgt, ibmvfc_tgt_adisc);
		break;
	case IBMVFC_MAD_DRIVER_FAILED:
		break;
	case IBMVFC_MAD_CRQ_ERROR:
		ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_query_target);
		break;
	case IBMVFC_MAD_FAILED:
	default:
		if ((rsp->status & IBMVFC_FABRIC_MAPPED) == IBMVFC_FABRIC_MAPPED &&
		    rsp->error == IBMVFC_UNABLE_TO_PERFORM_REQ &&
		    rsp->fc_explain == IBMVFC_PORT_NAME_NOT_REG)
			ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);
		else if (ibmvfc_retry_cmd(rsp->status, rsp->error))
			level += ibmvfc_retry_tgt_init(tgt, ibmvfc_tgt_query_target);
		else
			ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_DEL_RPORT);

		tgt_log(tgt, level, "Query Target failed: %s (%x:%x) %s (%x) %s (%x) rc=0x%02X\n",
			ibmvfc_get_cmd_error(rsp->status, rsp->error), rsp->status, rsp->error,
			ibmvfc_get_fc_type(rsp->fc_type), rsp->fc_type,
			ibmvfc_get_gs_explain(rsp->fc_explain), rsp->fc_explain, status);
		break;
	};

	kref_put(&tgt->kref, ibmvfc_release_tgt);
	ibmvfc_free_event(evt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_tgt_query_target - Initiate a Query Target for specified target
 * @tgt:	ibmvfc target struct
 *
 **/
static void ibmvfc_tgt_query_target(struct ibmvfc_target *tgt)
{
	struct ibmvfc_query_tgt *query_tgt;
	struct ibmvfc_host *vhost = tgt->vhost;
	struct ibmvfc_event *evt;

	if (vhost->discovery_threads >= disc_threads)
		return;

	kref_get(&tgt->kref);
	evt = ibmvfc_get_event(vhost);
	vhost->discovery_threads++;
	evt->tgt = tgt;
	ibmvfc_init_event(evt, ibmvfc_tgt_query_target_done, IBMVFC_MAD_FORMAT);
	query_tgt = &evt->iu.query_tgt;
	memset(query_tgt, 0, sizeof(*query_tgt));
	query_tgt->common.version = 1;
	query_tgt->common.opcode = IBMVFC_QUERY_TARGET;
	query_tgt->common.length = sizeof(*query_tgt);
	query_tgt->wwpn = tgt->ids.port_name;

	ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_INIT_WAIT);
	if (ibmvfc_send_event(evt, vhost, default_timeout)) {
		vhost->discovery_threads--;
		ibmvfc_set_tgt_action(tgt, IBMVFC_TGT_ACTION_NONE);
		kref_put(&tgt->kref, ibmvfc_release_tgt);
	} else
		tgt_dbg(tgt, "Sent Query Target\n");
}

/**
 * ibmvfc_alloc_target - Allocate and initialize an ibmvfc target
 * @vhost:		ibmvfc host struct
 * @scsi_id:	SCSI ID to allocate target for
 *
 * Returns:
 *	0 on success / other on failure
 **/
static int ibmvfc_alloc_target(struct ibmvfc_host *vhost, u64 scsi_id)
{
	struct ibmvfc_target *tgt;
	unsigned long flags;

	spin_lock_irqsave(vhost->host->host_lock, flags);
	list_for_each_entry(tgt, &vhost->targets, queue) {
		if (tgt->scsi_id == scsi_id) {
			if (tgt->need_login)
				ibmvfc_init_tgt(tgt, ibmvfc_tgt_implicit_logout);
			goto unlock_out;
		}
	}
	spin_unlock_irqrestore(vhost->host->host_lock, flags);

	tgt = mempool_alloc(vhost->tgt_pool, GFP_NOIO);
	if (!tgt) {
		dev_err(vhost->dev, "Target allocation failure for scsi id %08llx\n",
			scsi_id);
		return -ENOMEM;
	}

	memset(tgt, 0, sizeof(*tgt));
	tgt->scsi_id = scsi_id;
	tgt->new_scsi_id = scsi_id;
	tgt->vhost = vhost;
	tgt->need_login = 1;
	tgt->cancel_key = vhost->task_set++;
	init_timer(&tgt->timer);
	kref_init(&tgt->kref);
	ibmvfc_init_tgt(tgt, ibmvfc_tgt_implicit_logout);
	spin_lock_irqsave(vhost->host->host_lock, flags);
	list_add_tail(&tgt->queue, &vhost->targets);

unlock_out:
	spin_unlock_irqrestore(vhost->host->host_lock, flags);
	return 0;
}

/**
 * ibmvfc_alloc_targets - Allocate and initialize ibmvfc targets
 * @vhost:		ibmvfc host struct
 *
 * Returns:
 *	0 on success / other on failure
 **/
static int ibmvfc_alloc_targets(struct ibmvfc_host *vhost)
{
	int i, rc;

	for (i = 0, rc = 0; !rc && i < vhost->num_targets; i++)
		rc = ibmvfc_alloc_target(vhost,
					 vhost->disc_buf->scsi_id[i] & IBMVFC_DISC_TGT_SCSI_ID_MASK);

	return rc;
}

/**
 * ibmvfc_discover_targets_done - Completion handler for discover targets MAD
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_discover_targets_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_host *vhost = evt->vhost;
	struct ibmvfc_discover_targets *rsp = &evt->xfer_iu->discover_targets;
	u32 mad_status = rsp->common.status;
	int level = IBMVFC_DEFAULT_LOG_LEVEL;

	switch (mad_status) {
	case IBMVFC_MAD_SUCCESS:
		ibmvfc_dbg(vhost, "Discover Targets succeeded\n");
		vhost->num_targets = rsp->num_written;
		ibmvfc_set_host_action(vhost, IBMVFC_HOST_ACTION_ALLOC_TGTS);
		break;
	case IBMVFC_MAD_FAILED:
		level += ibmvfc_retry_host_init(vhost);
		ibmvfc_log(vhost, level, "Discover Targets failed: %s (%x:%x)\n",
			   ibmvfc_get_cmd_error(rsp->status, rsp->error), rsp->status, rsp->error);
		break;
	case IBMVFC_MAD_DRIVER_FAILED:
		break;
	default:
		dev_err(vhost->dev, "Invalid Discover Targets response: 0x%x\n", mad_status);
		ibmvfc_link_down(vhost, IBMVFC_LINK_DEAD);
		break;
	}

	ibmvfc_free_event(evt);
	wake_up(&vhost->work_wait_q);
}

/**
 * ibmvfc_discover_targets - Send Discover Targets MAD
 * @vhost:	ibmvfc host struct
 *
 **/
static void ibmvfc_discover_targets(struct ibmvfc_host *vhost)
{
	struct ibmvfc_discover_targets *mad;
	struct ibmvfc_event *evt = ibmvfc_get_event(vhost);

	ibmvfc_init_event(evt, ibmvfc_discover_targets_done, IBMVFC_MAD_FORMAT);
	mad = &evt->iu.discover_targets;
	memset(mad, 0, sizeof(*mad));
	mad->common.version = 1;
	mad->common.opcode = IBMVFC_DISC_TARGETS;
	mad->common.length = sizeof(*mad);
	mad->bufflen = vhost->disc_buf_sz;
	mad->buffer.va = vhost->disc_buf_dma;
	mad->buffer.len = vhost->disc_buf_sz;
	ibmvfc_set_host_action(vhost, IBMVFC_HOST_ACTION_INIT_WAIT);

	if (!ibmvfc_send_event(evt, vhost, default_timeout))
		ibmvfc_dbg(vhost, "Sent discover targets\n");
	else
		ibmvfc_link_down(vhost, IBMVFC_LINK_DEAD);
}

/**
 * ibmvfc_npiv_login_done - Completion handler for NPIV Login
 * @evt:	ibmvfc event struct
 *
 **/
static void ibmvfc_npiv_login_done(struct ibmvfc_event *evt)
{
	struct ibmvfc_host *vhost = evt->vhost;
	u32 mad_status = evt->xfer_iu->npiv_login.common.status;
	struct ibmvfc_npiv_login_resp *rsp = &vhost->login_buf->resp;
	unsigned int npiv_max_sectors;
	int level = IBMVFC_DEFAULT_LOG_LEVEL;

	switch (mad_status) {
	case IBMVFC_MAD_SUCCESS:
		ibmvfc_free_event(evt);
		break;
	case IBMVFC_MAD_FAILED:
		if (ibmvfc_retry_cmd(rsp->status, rsp->error))
			level += ibmvfc_retry_host_init(vhost);
		else
			ibmvfc_link_down(vhost, IBMVFC_LINK_DEAD);
		ibmvfc_log(vhost, level, "NPIV Login failed: %s (%x:%x)\n",
			   ibmvfc_get_cmd_error(rsp->status, rsp->error), rsp->status, rsp->error);
		ibmvfc_free_event(evt);
		return;
	case IBMVFC_MAD_CRQ_ERROR:
		ibmvfc_retry_host_init(vhost);
	case IBMVFC_MAD_DRIVER_FAILED:
		ibmvfc_free_event(evt);
		re