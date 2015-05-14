already allocated. Bad\n");
	else
		dev->isobufs = kzalloc(MAX_ISO_BUFS * sizeof(*dev->isobufs),
					GFP_KERNEL);
	if (dev->isobufs == NULL) {
		STK_ERROR("Unable to allocate iso buffers\n");
		return -ENOMEM;
	}
	for (i = 0; i < MAX_ISO_BUFS; i++) {
		if (dev->isobufs[i].data == NULL) {
			kbuf = kzalloc(ISO_BUFFER_SIZE, GFP_KERNEL);
			if (kbuf == NULL) {
				STK_ERROR("Failed to allocate iso buffer %d\n",
					i);
				goto isobufs_out;
			}
			dev->isobufs[i].data = kbuf;
		} else
			STK_ERROR("isobuf data already allocated\n");
		if (dev->isobufs[i].urb == NULL) {
			urb = usb_alloc_urb(ISO_FRAMES_PER_DESC, GFP_KERNEL);
			if (urb == NULL) {
				STK_ERROR("Failed to allocate URB %d\n", i);
				goto isobufs_out;
			}
			dev->isobufs[i].urb = urb;
		} else {
			STK_ERROR("Killing URB\n");
			usb_kill_urb(dev->isobufs[i].urb);
			urb = dev->isobufs[i].urb;
		}
		urb->interval = 1;
		urb->dev = udev;
		urb->pipe = usb_rcvisocpipe(udev, dev->isoc_ep);
		urb->transfer_flags = URB_ISO_ASAP;
		urb->transfer_buffer = dev->isobufs[i].data;
		urb->transfer_buffer_length = ISO_BUFFER_SIZE;
		urb->complete = stk_isoc_handler;
		urb->context = dev;
		urb->start_frame = 0;
		urb->number_of_packets = ISO_FRAMES_PER_DESC;

		for (j = 0; j < ISO_FRAMES_PER_DESC; j++) {
			urb->iso_frame_desc[j].offset = j * ISO_MAX_FRAME_SIZE;
			urb->iso_frame_desc[j].length = ISO_MAX_FRAME_SIZE;
		}
	}
	set_memallocd(dev);
	return 0;

isobufs_out:
	for (i = 0; i < MAX_ISO_BUFS && dev->isobufs[i].data; i++)
		kfree(dev->isobufs[i].data);
	for (i = 0; i < MAX_ISO_BUFS && dev->isobufs[i].urb; i++)
		usb_free_urb(dev->isobufs[i].urb);
	kfree(dev->isobufs);
	dev->isobufs = NULL;
	return -ENOMEM;
}

static void stk_clean_iso(struct stk_camera *dev)
{
	int i;

	if (dev == NULL || dev->isobufs == NULL)
		return;

	for (i = 0; i < MAX_ISO_BUFS; i++) {
		struct urb *urb;

		urb = dev->isobufs[i].urb;
		if (urb) {
			if (atomic_read(&dev->urbs_used) && is_present(dev))
				usb_kill_urb(urb);
			usb_free_urb(urb);
		}
		kfree(dev->isobufs[i].data);
	}
	kfree(dev->isobufs);
	dev->isobufs = NULL;
	unset_memallocd(dev);
}

static int stk_setup_siobuf(struct stk_camera *dev, int index)
{
	struct stk_sio_buffer *buf = dev->sio_bufs + index;
	INIT_LIST_HEAD(&buf->list);
	buf->v4lbuf.length = PAGE_ALIGN(dev->frame_size);
	buf->buffer = vmalloc_user(buf->v4lbuf.length);
	if (buf->buffer == NULL)
		return -ENOMEM;
	buf->mapcount = 0;
	buf->dev = dev;
	buf->v4lbuf.index = index;
	buf->v4lbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->v4lbuf.field = V4L2_FIELD_NONE;
	buf->v4lbuf.memory = V4L2_MEMORY_MMAP;
	buf->v4lbuf.m.offset = 2*index*buf->v4lbuf.length;
	return 0;
}

static int stk_free_sio_buffers(struct stk_camera *dev)
{
	int i;
	int nbufs;
	unsigned long flags;
	if (dev->n_sbufs == 0 || dev->sio_bufs == NULL)
		return 0;
	/*
	* If any buffers are mapped, we cannot free them at all.
	*/
	for (i = 0; i < dev->n_sbufs; i++) {
		if (dev->sio_bufs[i].mapcount > 0)
			return -EBUSY;
	}
	/*
	* OK, let's do it.
	*/
	spin_lock_irqsave(&dev->spinlock, flags);
	INIT_LIST_HEAD(&dev->sio_avail);
	INIT_LIST_HEAD(&dev->sio_full);
	nbufs = dev->n_sbufs;
	dev->n_sbufs = 0;
	spin_unlock_irqrestore(&dev->spinlock, flags);
	for (i = 0; i < nbufs; i++) {
		if (dev->sio_bufs[i].buffer != NULL)
			vfree(dev->sio_bufs[i].buffer);
	}
	kfree(dev->sio_bufs);
	dev->sio_bufs = NULL;
	return 0;
}

static int stk_prepare_sio_buffers(struct stk_camera *dev, unsigned n_sbufs)
{
	int i;
	if (dev->sio_bufs != NULL)
		STK_ERROR("sio_bufs already allocated\n");
	else {
		dev->sio_bufs = kzalloc(n_sbufs * sizeof(struct stk_sio_buffer),
				GFP_KERNEL);
		if (dev->sio_bufs == NULL)
			return -ENOMEM;
		for (i = 0; i < n_sbufs; i++) {
			if (stk_setup_siobuf(dev, i))
				return (dev->n_sbufs > 1)? 0 : -ENOMEM;
			dev->n_sbufs = i+1;
		}
	}
	return 0;
}

static int stk_allocate_buffers(struct stk_camera *dev, unsigned n_sbufs)
{
	int err;
	err = stk_prepare_iso(dev);
	if (err) {
		stk_clean_iso(dev);
		return err;
	}
	err = stk_prepare_sio_buffers(dev, n_sbufs);
	if (err) {
		stk_free_sio_buffers(dev);
		return err;
	}
	return 0;
}

static void stk_free_buffers(struct stk_camera *dev)
{
	stk_clean_iso(dev);
	stk_free_sio_buffers(dev);
}
/* -------------------------------------------- */

/* v4l file operations */

static int v4l_stk_open(struct file *fp)
{
	struct stk_camera *dev;
	struct video_device *vdev;

	vdev = video_devdata(fp);
	dev = vdev_to_camera(vdev);

	lock_kernel();
	if (dev == NULL || !is_present(dev)) {
		unlock_kernel();
		return -ENXIO;
	}
	fp->private_data = dev;
	usb_autopm_get_interface(dev->interface);
	unlock_kernel();

	return 0;
}

static int v4l_stk_release(struct file *fp)
{
	struct stk_camera *dev = fp->private_data;

	if (dev->owner == fp) {
		stk_stop_stream(dev);
		stk_free_buffers(dev);
		dev->owner = NULL;
	}

	if(is_present(dev))
		usb_autopm_put_interface(dev->interface);

	return 0;
}

static ssize_t v4l_stk_read(struct file *fp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	int i;
	int ret;
	unsigned long flags;
	struct stk_sio_buffer *sbuf;
	struct stk_camera *dev = fp->private_data;

	if (!is_present(dev))
		return -EIO;
	if (dev->owner && dev->owner != fp)
		return -EBUSY;
	dev->owner = fp;
	if (!is_streaming(dev)) {
		if (stk_initialise(dev)
			|| stk_allocate_buffers(dev, 3)
			|| stk_start_stream(dev))
			return -ENOMEM;
		spin_lock_irqsave(&dev->spinlock, flags);
		for (i = 0; i < dev->n_sbufs; i++) {
			list_add_tail(&dev->sio_bufs[i].list, &dev->sio_avail);
			dev->sio_bufs[i].v4lbuf.flags = V4L2_BUF_FLAG_QUEUED;
		}
		spin_unlock_irqrestore(&dev->spinlock, flags);
	}
	if (*f_pos == 0) {
		if (fp->f_flags & O_NONBLOCK && list_empty(&dev->sio_full))
			return -EWOULDBLOCK;
		ret = wait_event_interruptible(dev->wait_frame,
			!list_empty(&dev->sio_full) || !is_present(dev));
		if (ret)
			return ret;
		if (!is_present(dev))
			return -EIO;
	}
	if (count + *f_pos > dev->frame_size)
		count = dev->frame_size - *f_pos;
	spin_lock_irqsave(&dev->spinlock, flags);
	if (list_empty(&dev->sio_full)) {
		spin_unlock_irqrestore(&dev->spinlock, flags);
		STK_ERROR("BUG: No siobufs ready\n");
		return 0;
	}
	sbuf = list_first_entry(&dev->sio_full, struct stk_sio_buffer, list);
	spin_unlock_irqrestore(&dev->spinlock, flags);

	if (copy_to_user(buf, sbuf->buffer + *f_pos, count))
		return -EFAULT;

	*f_pos += count;

	if (*f_pos >= dev->frame_size) {
		*f_pos = 0;
		spin_lock_irqsave(&dev->spinlock, flags);
		list_move_tail(&sbuf->list, &dev->sio_avail);
		spin_unlock_irqrestore(&dev->spinlock, flags);
	}
	return count;
}

static unsigned int v4l_stk_poll(struct file *fp, poll_table *wait)
{
	struct stk_camera *dev = fp->private_data;

	poll_wait(fp, &dev->wait_frame, wait);

	if (!is_present(dev))
		return POLLERR;

	if (!list_empty(&dev->sio_full))
		return (P