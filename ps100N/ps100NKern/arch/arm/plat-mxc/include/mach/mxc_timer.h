(*hostdata));
	INIT_LIST_HEAD(&hostdata->sent);
	hostdata->host = host;
	hostdata->dev = dev;
	atomic_set(&hostdata->request_limit, -1);
	hostdata->host->max_sectors = IBMVSCSI_MAX_SECTORS_DEFAULT;

	if (map_persist_bufs(hostdata)) {
		dev_err(&vdev->dev, "couldn't map persistent buffers\n");
		goto persist_bufs_failed;
	}

	rc = ibmvscsi_ops->init_crq_queue(&hostdata->queue, hostdata, max_events);
	if (rc != 0 && rc != H_RESOURCE) {
		dev_err(&vdev->dev, "couldn't initialize crq. rc=%d\n", rc);
		goto init_crq_failed;
	}
	if (initialize_event_pool(&hostdata->pool, max_events, hostdata) != 0) {
		dev_err(&vdev->dev, "couldn't initialize event pool\n");
		goto init_pool_failed;
	}

	host->max_lun = 8;
	host->max_id = max_id;
	host->max_channel = max_channel;
	host->max_cmd_len = 16;

	if (scsi_add_host(hostdata->host, hostdata->dev))
		goto add_host_failed;

	/* we don't have a proper target_port_id so let's use the fake one */
	memcpy(ids.port_id, hostdata->madapter_info.partition_name,
	       sizeof(ids.port_id));
	ids.roles = SRP_RPORT_ROLE_TARGET;
	rport = srp_rport_add(host, &ids);
	if (IS_ERR(rport))
		goto add_srp_port_failed;

	/* Try to send an initialization message.  Note that this is allowed
	 * to fail if the other end is not acive.  In that case we don't
	 * want to scan
	 */
	if (ibmvscsi_ops->send_crq(hostdata, 0xC001000000000000LL, 0) == 0
	    || rc == H_RESOURCE) {
		/*
		 * Wait around max init_timeout secs for the adapter to finish
		 * initializing. When we are done initializing, we will have a
		 * valid request_limit.  We don't want Linux scanning before
		 * we are ready.
		 */
		for (wait_switch = jiffies + (init_timeout * HZ);
		     time_before(jiffies, wait_switch) &&
		     atomic_read(&hostdata->request_limit) < 2;) {

			msleep(10);
		}

		/* if we now have a valid request_limit, initiate a scan */
		if (atomic_read(&hostdata->request_limit) > 0)
			scsi_scan_host(host);
	}

	dev_set_drvdata(&vdev->dev, hostdata);
	return 0;

      add_srp_port_failed:
	scsi_remove_host(hostdata->host);
      add_host_failed:
	release_event_pool(&hostdata->pool, hostdata);
      init_pool_failed:
	ibmvscsi_ops->release_crq_queue(&hostdata->queue, hostdata, max_events);
      init_crq_failed:
	unmap_persist_bufs(hostdata);
      persist_bufs_failed:
	scsi_host_put(host);
      scsi_host_alloc_failed:
	return -1;
}

static int ibmvscsi_remove(struct vio_dev *vdev)
{
	struct ibmvscsi_host_data *hostdata = dev_get_drvdata(&vdev->dev);
	unmap_persist_bufs(hostdata);
	release_event_pool(&hostdata->pool, hostdata);
	ibmvscsi_ops->release_crq_queue(&hostdata->queue, hostdata,
					max_events);

	srp_remove_host(hostdata->host);
	scsi_remove_host(hostdata->host);
	scsi_host_put(hostdata->host);

	return 0;
}

/**
 * ibmvscsi_device_table: Used by vio.c to match devices in the device tree we 
 * support.
 */
static struct vio_device_id ibmvscsi_device_table[] __devinitdata = {
	{"vscsi", "IBM,v-scsi"},
	{ "", "" }
};
MODULE_DEVICE_TABLE(vio, ibmvscsi_device_table);

static struct vio_driver ibmvscsi_driver = {
	.id_table = ibmvscsi_device_table,
	.probe = ibmvscsi_probe,
	.remove = ibmvscsi_remove,
	.get_desired_dma = ibmvscsi_get_desired_dma,
	.driver = {
		.name = "ibmvscsi",
		.owner = THIS_MODULE,
	}
};

static struct srp_function_template ibmvscsi_transport_functions = {
};

int __init ibmvscsi_module_init(void)
{
	int ret;

	/* Ensure we have two requests to do error recovery */
	driver_template.can_queue = max_requests;
	max_events = max_requests + 2;

	if (firmware_has_feature(FW_FEATURE_ISERIES))
		ibmvscsi_ops = &iseriesvscsi_ops;
	else if (firmware_has_feature(FW_FEATURE_VIO))
		ibmvscsi_ops = &rpavscsi_ops;
	else
		return -ENODEV;

	ibmvscsi_transport_template =
		srp_attach_transport(&ibmvscsi_transport_functions);
	if (!ibmvscsi_transport_template)
		return -ENOMEM;

	ret = vio_register_driver(&ibmvscsi_driver);
	if (ret)
		srp_release_tra