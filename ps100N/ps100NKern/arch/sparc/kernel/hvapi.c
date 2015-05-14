p_listen(fi, req);
		return 0;

	case RAW1394_REQ_RESET_BUS:
		if (req->req.misc == RAW1394_LONG_RESET) {
			DBGMSG("busreset called (type: LONG)");
			hpsb_reset_bus(fi->host, LONG_RESET);
			free_pending_request(req);	/* we have to free the request, because we queue no response, and therefore nobody will free it */
			return 0;
		}
		if (req->req.misc == RAW1394_SHORT_RESET) {
			DBGMSG("busreset called (type: SHORT)");
			hpsb_reset_bus(fi->host, SHORT_RESET);
			free_pending_request(req);	/* we have to free the request, because we queue no response, and therefore nobody will free it */
			return 0;
		}
		/* error EINVAL (22) invalid argument */
		return (-EINVAL);
	case RAW1394_REQ_GET_ROM:
		return get_config_rom(fi, req);

	case RAW1394_REQ_UPDATE_ROM:
		return update_config_rom(fi, req);

	case RAW1394_REQ_MODIFY_ROM:
		return modify_config_rom(fi, req);
	}

	if (req->req.generation != get_hpsb_generation(fi->host)) {
		req->req.error = RAW1394_ERROR_GENERATION;
		req->req.generation = get_hpsb_generation(fi->host);
		req->req.length = 0;
		queue_complete_req(req);
		return 0;
	}

	switch (req->req.type) {
	case RAW1394_REQ_PHYPACKET:
		return write_phypacket(fi, req);
	case RAW1394_REQ_ASYNC_SEND:
		return handle_async_send(fi, req);
	}

	if (req->req.length == 0) {
		req->req.error = RAW1394_ERROR_INVALID_ARG;
		queue_complete_req(req);
		return 0;
	}

	return handle_async_request(fi, req, node);
}

static ssize_t raw1394_write(struct file *file, const char __user * buffer,
			     size_t count, loff_t * offset_is_ignored)
{
	struct file_info *fi = (struct file_info *)file->private_data;
	struct pending_request *req;
	ssize_t retval = -EBADFD;

#ifdef CONFIG_COMPAT
	if (count == sizeof(struct compat_raw1394_req) &&
   	    sizeof(struct compat_raw1394_req) !=
			sizeof(struct raw1394_request)) {
		buffer = raw1394_compat_write(buffer);
		if (IS_ERR((__force void *)buffer))
			return PTR_ERR((__force void *)buffer);
	} else
#endif
	if (count != sizeof(struct raw1394_request)) {
		return -EINVAL;
	}

	req = alloc_pending_request();
	if (req == NULL) {
		return -ENOMEM;
	}
	req->file_info = fi;

	if (copy_from_user(&req->req, buffer, sizeof(struct raw1394_request))) {
		free_pending_request(req);
		return -EFAULT;
	}

	if (!mutex_trylock(&fi->state_mutex))
		return -EAGAIN;

	switch (fi->state) {
	case opened:
		retval = state_opened(fi, req);
		break;

	case initialized:
		retval = state_initialized(fi, req);
		break;

	case connected:
		retval = state_connected(fi, req);
		break;
	}

	mutex_unlock(&fi->state_mutex);

	if (retval < 0) {
		free_pending_request(req);
	} else {
		BUG_ON(retval);
		retval = count;
	}

	return retval;
}

/* rawiso operations */

/* check if any RAW1394_REQ_RAWISO_ACTIVITY event is already in the
 * completion queue (reqlists_lock must be taken) */
static inline int __rawiso_event_in_queue(struct file_info *fi)
{
	struct pending_request *req;

	list_for_each_entry(req, &fi->req_complete, list)
	    if (req->req.type == RAW1394_REQ_RAWISO_ACTIVITY)
		return 1;

	return 0;
}

/* put a RAWISO_ACTIVITY event in the queue, if one isn't there already */
static void queue_rawiso_event(struct file_info *fi)
{
	unsigned long flags;

	spin_lock_irqsave(&fi->reqlists_lock, flags);

	/* only one ISO activity event may be in the queue */
	if (!__rawiso_event_in_queue(fi)) {
		struct pending_request *req =
		    __alloc_pending_request(GFP_ATOMIC);

		if (req) {
			req->file_info = fi;
			req->req.type = RAW1394_REQ_RAWISO_ACTIVITY;
			req->req.generation = get_hpsb_generation(fi->host);
			__queue_complete_req(req);
		} else {
			/* on allocation failure, signal an overflow */
			if (fi->iso_handle) {
				atomic_inc(&fi->iso_handle->overflows);
			}
		}
	}
	spin_unlock_irqrestore(&fi->reqlists_lock, flags);
}

static void rawiso_activity_cb(struct hpsb_iso *iso)
{
	unsigned long flags;
	struct host_info *hi;
	struct file_info *fi;

	spin_lock_irqsave(&host_info_lock, flags);
	hi = find_host_info(iso->host);

	if (hi != NULL) {
		list_for_each_entry(fi, &hi->file_info_list, list) {
			if (fi->iso_handle == iso)
				queue_rawiso_event(fi);
		}
	}

	spin_unlock_irqrestore(&host_info_lock, flags);
}

/* helper function - gather all the kernel iso status bits for returning to user-space */
static void raw1394_iso_fill_statu