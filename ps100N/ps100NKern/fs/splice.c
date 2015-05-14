mask = (u64)0x1<<channel;
		if (!(ohci->ISO_channel_usage & mask)) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Channel %d is not being used", channel);
			return -ESRCH;
		}

		/* Mark this channel as unused */
		ohci->ISO_channel_usage &= ~mask;

		if (cmd == VIDEO1394_IOC_UNLISTEN_CHANNEL)
			d = find_ctx(&ctx->context_list, OHCI_ISO_RECEIVE, channel);
		else
			d = find_ctx(&ctx->context_list, OHCI_ISO_TRANSMIT, channel);

		if (d == NULL) return -ESRCH;
		DBGMSG(ohci->host->id, "Iso context %d "
		      "stop talking on channel %d", d->ctx, channel);
		free_dma_iso_ctx(d);

		return 0;
	}
	case VIDEO1394_IOC_LISTEN_QUEUE_BUFFER:
	{
		struct video1394_wait v;
		struct dma_iso_ctx *d;
		int next_prg;

		if (unlikely(copy_from_user(&v, argp, sizeof(v))))
			return -EFAULT;

		d = find_ctx(&ctx->context_list, OHCI_ISO_RECEIVE, v.channel);
		if (unlikely(d == NULL))
			return -EFAULT;

		if (unlikely(v.buffer >= d->num_desc - 1)) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d out of range",v.buffer);
			return -EINVAL;
		}

		spin_lock_irqsave(&d->lock,flags);

		if (unlikely(d->buffer_status[v.buffer]==VIDEO1394_BUFFER_QUEUED)) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d is already used",v.buffer);
			spin_unlock_irqrestore(&d->lock,flags);
			return -EBUSY;
		}

		d->buffer_status[v.buffer]=VIDEO1394_BUFFER_QUEUED;

		next_prg = (d->last_buffer + 1) % d->num_desc;
		if (d->last_buffer>=0)
			d->ir_prg[d->last_buffer][d->nb_cmd-1].branchAddress =
				cpu_to_le32((dma_prog_region_offset_to_bus(&d->prg_reg[next_prg], 0)
					& 0xfffffff0) | 0x1);

		d->last_buffer = next_prg;
		reprogram_dma_ir_prg(d, d->last_buffer, v.buffer, d->flags);

		d->ir_prg[d->last_buffer][d->nb_cmd-1].branchAddress = 0;

		spin_unlock_irqrestore(&d->lock,flags);

		if (!(reg_read(ohci, d->ctrlSet) & 0x8000))
		{
			DBGMSG(ohci->host->id, "Starting iso DMA ctx=%d",d->ctx);

			/* Tell the controller where the first program is */
			reg_write(ohci, d->cmdPtr,
				  dma_prog_region_offset_to_bus(&d->prg_reg[d->last_buffer], 0) | 0x1);

			/* Run IR context */
			reg_write(ohci, d->ctrlSet, 0x8000);
		}
		else {
			/* Wake up dma context if necessary */
			if (!(reg_read(ohci, d->ctrlSet) & 0x400)) {
				DBGMSG(ohci->host->id,
				      "Waking up iso dma ctx=%d", d->ctx);
				reg_write(ohci, d->ctrlSet, 0x1000);
			}
		}
		return 0;

	}
	case VIDEO1394_IOC_LISTEN_WAIT_BUFFER:
	case VIDEO1394_IOC_LISTEN_POLL_BUFFER:
	{
		struct video1394_wait v;
		struct dma_iso_ctx *d;
		int i = 0;

		if (unlikely(copy_from_user(&v, argp, sizeof(v))))
			return -EFAULT;

		d = find_ctx(&ctx->context_list, OHCI_ISO_RECEIVE, v.channel);
		if (unlikely(d == NULL))
			return -EFAULT;

		if (unlikely(v.buffer > d->num_desc - 1)) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d out of range",v.buffer);
			return -EINVAL;
		}

		/*
		 * I change the way it works so that it returns
		 * the last received frame.
		 */
		spin_lock_irqsave(&d->lock, flags);
		switch(d->buffer_status[v.buffer]) {
		case VIDEO1394_BUFFER_READY:
			d->buffer_status[v.buffer]=VIDEO1394_BUFFER_FREE;
			break;
		case VIDEO1394_BUFFER_QUEUED:
			if (cmd == VIDEO1394_IOC_LISTEN_POLL_BUFFER) {
			    /* for polling, return error code EINTR */
			    spin_unlock_irqrestore(&d->lock, flags);
			    return -EINTR;
			}

			spin_unlock_irqrestore(&d->lock, flags);
			wait_event_interruptible(d->waitq,
					video1394_buffer_state(d, v.buffer) ==
					 VIDEO1394_BUFFER_READY);
			if (signal_pending(current))
                                return -EINTR;
			spin_lock_irqsave(&d->lock, flags);
			d->buffer_status[v.buffer]=VIDEO1394_BUFFER_FREE;
			break;
		default:
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d is not queued",v.buffer);
			spin_unlock_irqrestore(&d->lock, flags);
			return -ESRCH;
		}

		/* set time of buffer */
		v.filltime = d->buffer_time[v.buffer];

		/*
		 * Look ahead to see how many more buffers have been received
		 */
		i=0;
		while (d->buffer_status[(v.buffer+1)%(d->num_desc - 1)]==
		       VIDEO1394_BUFFER_READY) {
			v.buffer=(v.buffer+1)%(d->num_desc - 1);
			i++;
		}
		spin_unlock_irqrestore(&d->lock, flags);

		v.buffer=i;
		if (unlikely(copy_to_user(argp, &v, sizeof(v))))
			return -EFAULT;

		return 0;
	}
	case VIDEO1394_IOC_TALK_QUEUE_BUFFER:
	{
		struct video1394_wait v;
		unsigned int *psizes = NULL;
		struct dma_iso_ctx *d;
		int next_prg;

		if (copy_from_user(&v, argp, sizeof(v)))
			return -EFAULT;

		d = find_ctx(&ctx->context_list, OHCI_ISO_TRANSMIT, v.channel);
		if (d == NULL) return -EFAULT;

		if (v.buffer >= d->num_desc - 1) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d out of range",v.buffer);
			return -EINVAL;
		}

		if (d->flags & VIDEO1394_VARIABLE_PACKET_SIZE) {
			int buf_size = d->nb_cmd * sizeof(*psizes);
			struct video1394_queue_variable __user *p = argp;
			unsigned int __user *qv;

			if (get_user(qv, &p->packet_sizes))
				return -EFAULT;

			psizes = kmalloc(buf_size, GFP_KERNEL);
			if (!psizes)
				return -ENOMEM;

			if (copy_from_user(psizes, qv, buf_size)) {
				kfree(psizes);
				return -EFAULT;
			}
		}

		spin_lock_irqsave(&d->lock,flags);

		/* last_buffer is last_prg */
		next_prg = (d->last_buffer + 1) % d->num_desc;
		if (d->buffer_status[v.buffer]!=VIDEO1394_BUFFER_FREE) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d is already used",v.buffer);
			spin_unlock_irqrestore(&d->lock,flags);
			kfree(psizes);
			return -EBUSY;
		}

		if (d->flags & VIDEO1394_VARIABLE_PACKET_SIZE) {
			initialize_dma_it_prg_var_packet_queue(
				d, next_prg, psizes, ohci);
		}

		d->buffer_status[v.buffer]=VIDEO1394_BUFFER_QUEUED;

		if (d->last_buffer >= 0) {
			d->it_prg[d->last_buffer]
				[ d->last_used_cmd[d->last_buffer] ].end.branchAddress =
					cpu_to_le32((dma_prog_region_offset_to_bus(&d->prg_reg[next_prg],
						0) & 0xfffffff0) | 0x3);

			d->it_prg[d->last_buffer]
				[ d->last_used_cmd[d->last_buffer] ].begin.branchAddress =
					cpu_to_le32((dma_prog_region_offset_to_bus(&d->prg_reg[next_prg],
						0) & 0xfffffff0) | 0x3);
			d->next_buffer[d->last_buffer] = (v.buffer + 1) % (d->num_desc - 1);
		}
		d->last_buffer = next_prg;
		reprogram_dma_it_prg(d, d->last_buffer, v.buffer);
		d->next_buffer[d->last_buffer] = -1;

		d->it_prg[d->last_buffer][d->last_used_cmd[d->last_buffer]].end.branchAddress = 0;

		spin_unlock_irqrestore(&d->lock,flags);

		if (!(reg_read(ohci, d->ctrlSet) & 0x8000))
		{
			DBGMSG(ohci->host->id, "Starting iso transmit DMA ctx=%d",
			       d->ctx);
			put_timestamp(ohci, d, d->last_buffer);
			dma_region_sync_for_device(&d->dma,
				v.buffer * d->buf_size, d->buf_size);

			/* Tell the controller where the first program is */
			reg_write(ohci, d->cmdPtr,
				dma_prog_region_offset_to_bus(&d->prg_reg[next_prg], 0) | 0x3);

			/* Run IT context */
			reg_write(ohci, d->ctrlSet, 0x8000);
		}
		else {
			/* Wake up dma context if necessary */
			if (!(reg_read(ohci, d->ctrlSet) & 0x400)) {
				DBGMSG(ohci->host->id,
				      "Waking up iso transmit dma ctx=%d",
				      d->ctx);
				put_timestamp(ohci, d, d->last_buffer);
				dma_region_sync_for_device(&d->dma,
					v.buffer * d->buf_size, d->buf_size);

				reg_write(ohci, d->ctrlSet, 0x1000);
			}
		}

		kfree(psizes);
		return 0;

	}
	case VIDEO1394_IOC_TALK_WAIT_BUFFER:
	{
		struct video1394_wait v;
		struct dma_iso_ctx *d;

		if (copy_from_user(&v, argp, sizeof(v)))
			return -EFAULT;

		d = find_ctx(&ctx->context_list, OHCI_ISO_TRANSMIT, v.channel);
		if (d == NULL) return -EFAULT;

		if (v.buffer >= d->num_desc - 1) {
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d out of range",v.buffer);
			return -EINVAL;
		}

		switch(d->buffer_status[v.buffer]) {
		case VIDEO1394_BUFFER_READY:
			d->buffer_status[v.buffer]=VIDEO1394_BUFFER_FREE;
			return 0;
		case VIDEO1394_BUFFER_QUEUED:
			wait_event_interruptible(d->waitq,
					(d->buffer_status[v.buffer] == VIDEO1394_BUFFER_READY));
			if (signal_pending(current))
				return -EINTR;
			d->buffer_status[v.buffer]=VIDEO1394_BUFFER_FREE;
			return 0;
		default:
			PRINT(KERN_ERR, ohci->host->id,
			      "Buffer %d is not queued",v.buffer);
			return -ESRCH;
		}
	}
	default:
		return -ENOTTY;
	}
}

/*
 *	This maps the vmalloced and reserved buffer to user space.
 *
 *  FIXME:
 *  - PAGE_READONLY should suffice!?
 *  - remap_pfn_range is kind of inefficient for page by page remapping.
 *    But e.g. pte_alloc() does not work in modules ... :-(
 */

static int video1394_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct file_ctx *ctx = (struct file_ctx *)file->private_data;

	if (ctx->current_ctx == NULL) {
		PRINT(KERN_ERR, ctx->ohci->host->id,
				"Current iso context not set");
		return -EINVAL;
	}

	return dma_region_mmap(&ctx->current_ctx->dma, file, vma);
}

static unsigned int video1394_poll(struct file *file, poll_table *pt)
{
	struct file_ctx *ctx;
	unsigned int mask = 0;
	unsigned long flags;
	struct dma_iso_ctx *d;
	int i;

	ctx = file->private_data;
	d = ctx->current_ctx;
	if (d == NULL) {
		PRINT(KERN_ERR, ctx->ohci->host->id,
				"Current iso context not set");
		return POLLERR;
	}

	poll_wait(file, &d->waitq, pt);

	spin_lock_irqsave(&d->lock, flags);
	for (i = 0; i < d->num_desc; i++) {
		if (d->buffer_status[i] == VIDEO1394_BUFFER_READY) {
			mask |= POLLIN | POLLRDNORM;
			break;
		}
	}
	spin_unlock_irqrestore(&d->lock, flags);

	return mask;
}

static int video1394_open(struct inode *inode, struct file *file)
{
	int i = ieee1394_file_to_instance(file);
	struct ti_ohci *ohci;
	struct file_ctx *ctx;

	ohci = hpsb_get_hostinfo_bykey(&video1394_highlevel, i);
        if (ohci == NULL)
                return -EIO;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)  {
		PRINT(KERN_ERR, ohci->host->id, "Cannot malloc file_ctx");
		return -ENOMEM;
	}

	ctx->ohci = ohci;
	INIT_LIST_HEAD(&ctx->context_list);
	ctx->current_ctx = NULL;
	file->private_data = ctx;

	return 0;
}

static int video1394_release(struct inode *inode, struct file *file)
{
	struct file_ctx *ctx = (struct file_ctx *)file->private_data;
	struct ti_ohci *ohci = ctx->ohci;
	struct list_head *lh, *next;
	u64 mask;

	list_for_each_safe(lh, next, &ctx->context_list) {
		struct dma_iso_ctx *d;
		d = list_entry(lh, struct dma_iso_ctx, link);
		mask = (u64) 1 << d->channel;

		if (!(ohci->ISO_channel_usage & mask))
			PRINT(KERN_ERR, ohci->host->id, "On release: Channel %d "
			      "is not being used", d->channel);
		else
			ohci->ISO_channel_usage &= ~mask;
		DBGMSG(ohci->host->id, "On release: Iso %s context "
		      "%d stop listening on channel %d",
		      d->type == OHCI_ISO_RECEIVE ? "receive" : "transmit",
		      d->ctx, d->channel);
		free_dma_iso_ctx(d);
	}

	kfree(ctx);
	file->private_data = NULL;

	return 0;
}

#ifdef CONFIG_COMPAT
static long video1394_compat_ioctl(struct file *f, unsigned cmd, unsigned long arg);
#endif

static struct cdev video1394_cdev;
static const struct file_operations video1394_fops=
{
	.owner =	THIS_MODULE,
	.unlocked_ioctl = video1394_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = video1394_compat_ioctl,
#endif
	.poll =		video1394_poll,
	.mmap =		video1394_mmap,
	.open =		video1394_open,
	.release =	video1394_release
};

/*** HOTPLUG STUFF **********************************************************/
/*
 * Export information about protocols/devices supported by this driver.
 */
#ifdef MODULE
static const struct ieee1394_device_id video1394_id_table[] = {
	{
		.match_flags	= IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
		.specifier_id	= CAMERA_UNIT_SPEC_ID_ENTRY & 0xffffff,
		.version	= CAMERA_SW_VERSION_ENTRY & 0xffffff
	},
        {
                .match_flags    = IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
                .specifier_id   = CAMERA_UNIT_SPEC_ID_ENTRY & 0xffffff,
                .version        = (CAMERA_SW_VERSION_ENTRY + 1) & 0xffffff
        },
        {
                .match_flags    = IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
                .specifier_id   = CAMERA_UNIT_SPEC_ID_ENTRY & 0xffffff,
                .version        = (CAMERA_SW_VERSION_ENTRY + 2) & 0xffffff
        },
	{ }
};

MODULE_DEVICE_TABLE(ieee1394, video1394_id_table);
#endif /* MODULE */

static struct hpsb_protocol_driver video1394_driver = {
	.name = VIDEO1394_DRIVER_NAME,
};


static void video1394_add_host (struct hpsb_host *host)
{
	struct ti_ohci *ohci;
	int minor;

	/* We only work with the OHCI-1394 driver */
	if (strcmp(host->driver->name, OHCI1394_DRIVER_NAME))
		return;

	ohci = (struct ti_ohci *)host->hostdata;

	if (!hpsb_create_hostinfo(&video1394_highlevel, host, 0)) {
		PRINT(KERN_ERR, ohci->host->id, "Cannot allocate hostinfo");
		return;
	}

	hpsb_set_hostinfo(&video1394_highlevel, host, ohci);
	hpsb_set_hostinfo_key(&video1394_highlevel, host, ohci->host->id);

	minor = IEEE1394_MINOR_BLOCK_VIDEO1394 * 16 + ohci->host->id;
	device_create(hpsb_protocol_class, NULL, MKDEV(IEEE1394_MAJOR, minor),
		      NULL, "%s-%d", VIDEO1394_DRIVER_NAME, ohci->host->id);
}


static void video1394_remove_host (struct hpsb_host *host)
{
	struct ti_ohci *ohci = hpsb_get_hostinfo(&video1394_highlevel, host);

	if (ohci)
		device_destroy(hpsb_protocol_class, MKDEV(IEEE1394_MAJOR,
			       IEEE1394_MINOR_BLOCK_VIDEO1394 * 16 + ohci->host->id));
	return;
}


static struct hpsb_highlevel video1394_highlevel = {
	.name =		VIDEO1394_DRIVER_NAME,
	.add_host =	video1394_add_host,
	.remove_host =	video1394_remove_host,
};

MODULE_AUTHOR("Sebastien Rougeaux <sebastien.rougeaux@anu.edu.au>");
MODULE_DESCRIPTION("driver for digital video on OHCI board");
MODULE_SUPPORTED_DEVICE(VIDEO1394_DRIVER_NAME);
MODULE_LICENSE("GPL");

#ifdef CONFIG_COMPAT

#define VIDEO1394_IOC32_LISTEN_QUEUE_BUFFER     \
	_IOW ('#', 0x12, struct video1394_wait32)
#define VIDEO1394_IOC32_LISTEN_WAIT_BUFFER      \
	_IOWR('#', 0x13, struct video1394_wait32)
#define VIDEO1394_IOC32_TALK_WAIT_BUFFER        \
	_IOW ('#', 0x17, struct video1394_wait32)
#define VIDEO1394_IOC32_LISTEN_POLL_BUFFER      \
	_IOWR('#', 0x18, struct video1394_wait32)

struct video1394_wait32 {
	u32 channel;
	u32 buffer;
	struct compat_timeval filltime;
};

static int video1394_wr_wait32(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct video1394_wait32 __user *argp = (void __user *)arg;
        struct video1394_wait32 wait32;
        struct video1394_wait wait;
        mm_segment_t old_fs;
        int ret;

        if (copy_from_user(&wait32, argp, sizeof(wait32)))
                return -EFAULT;

        wait.channel = wait32.channel;
        wait.buffer = wait32.buffer;
        wait.filltime.tv_sec = (time_t)wait32.filltime.tv_sec;
        wait.filltime.tv_usec = (suseconds_t)wait32.filltime.tv_usec;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        if (cmd == VIDEO1394_IOC32_LISTEN_WAIT_BUFFER)
		ret = video1394_ioctl(file,
				      VIDEO1394_IOC_LISTEN_WAIT_BUFFER,
				      (unsigned long) &wait);
        else
		ret = video1394_ioctl(file,
				      VIDEO1394_IOC_LISTEN_POLL_BUFFER,
				      (unsigned long) &wait);
        set_fs(old_fs);

        if (!ret) {
                wait32.channel = wait.channel;
                wait32.buffer = wait.buffer;
                wait32.filltime.tv_sec = (int)wait.filltime.tv_sec;
                wait32.filltime.tv_usec = (int)wait.filltime.tv_usec;

                if (copy_to_user(argp, &wait32, sizeof(wait32)))
                        ret = -EFAULT;
        }

        return ret;
}

static int video1394_w_wait32(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct video1394_wait32 wait32;
        struct video1394_wait wait;
        mm_segment_t old_fs;
        int ret;

        if (copy_from_user(&wait32, (void __user *)arg, sizeof(wait32)))
                return -EFAULT;

        wait.channel = wait32.channel;
        wait.buffer = wait32.buffer;
        wait.filltime.tv_sec = (time_t)wait32.filltime.tv_sec;
        wait.filltime.tv_usec = (suseconds_t)wait32.filltime.tv_usec;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        if (cmd == VIDEO1394_IOC32_LISTEN_QUEUE_BUFFER)
		ret = video1394_ioctl(file,
				      VIDEO1394_IOC_LISTEN_QUEUE_BUFFER,
				      (unsigned long) &wait);
        else
		ret = video1394_ioctl(file,
				      VIDEO1394_IOC_TALK_WAIT_BUFFER,
				      (unsigned long) &wait);
        set_fs(old_fs);

        return ret;
}

static int video1394_queue_buf32(struct file *file, unsigned int cmd, unsigned long arg)
{
        return -EFAULT;   /* ??? was there before. */

	return video1394_ioctl(file,
				VIDEO1394_IOC_TALK_QUEUE_BUFFER, arg);
}

static long video1394_compat_ioctl(struct file *f, unsigned cmd, unsigned long arg)
{
	switch (cmd) {
	case VIDEO1394_IOC_LISTEN_CHANNEL:
	case VIDEO1394_IOC_UNLISTEN_CHANNEL:
	case VIDEO1394_IOC_TALK_CHANNEL:
	case VIDEO1394_IOC_UNTALK_CHANNEL:
		return video1394_ioctl(f, cmd, arg);

	case VIDEO1394_IOC32_LISTEN_QUEUE_BUFFER:
		return video1394_w_wait32(f, cmd, arg);
	case VIDEO1394_IOC32_LISTEN_WAIT_BUFFER:
		return video1394_wr_wait32(f, cmd, arg);
	case VIDEO1394_IOC_TALK_QUEUE_BUFFER:
		return video1394_queue_buf32(f, cmd, arg);
	case VIDEO1394_IOC32_TALK_WAIT_BUFFER:
		return video1394_w_wait32(f, cmd, arg);
	case VIDEO1394_IOC32_LISTEN_POLL_BUFFER:
		return video1394_wr_wait32(f, cmd, arg);
	default:
		return -ENOIOCTLCMD;
	}
}

#endif /* CONFIG_COMPAT */

static void __exit video1394_exit_module (void)
{
	hpsb_unregister_protocol(&video1394_driver);
	hpsb_unregister_highlevel(&video1394_highlevel);
	cdev_del(&video1394_cdev);
	PRINT_G(KERN_INFO, "Removed " VIDEO1394_DRIVER_NAME " module");
}

static int __init video1394_init_module (void)
{
	int ret;

	hpsb_init_highlevel(&video1394_highlevel);

	cdev_init(&video1394_cdev, &video1394_fops);
	video1394_cdev.owner = THIS_MODULE;
	ret = cdev_add(&video1394_cdev, IEEE1394_VIDEO1394_DEV, 16);
	if (ret) {
		PRINT_G(KERN_ERR, "video1394: unable to get minor device block");
		return ret;
        }

	hpsb_register_highlevel(&video1394_highlevel);

	ret = hpsb_register_protocol(&video1394_driver);
	if (ret) {
		PRINT_G(KERN_ERR, "video1394: failed to register protocol");
		hpsb_unregister_highlevel(&video1394_highlevel);
		cdev_del(&video1394_cdev);
		return ret;
	}

	PRINT_G(KERN_INFO, "Installed " VIDEO1394_DRIVER_NAME " module");
	return 0;
}


module_init(video1394_init_module);
module_exit(video1394_exit_module);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ize the necessary fields of @sd.
 */
void splice_from_pipe_begin(struct splice_desc *sd)
{
	sd->num_spliced = 0;
	sd->need_wakeup = false;
}
EXPORT_SYMBOL(splice_from_pipe_begin);

/**
 * splice_from_pipe_end - finish splicing from pipe
 * @pipe:	pipe to splice from
 * @sd:		information about the splice operation
 *
 * Description:
 *    This function will wake up pipe writers if necessary.  It should
 *    be called after a loop containing splice_from_pipe_next() and
 *    splice_from_pipe_feed().
 */
void splice_from_pipe_end(struct pipe_inode_info *pipe, struct splice_desc *sd)
{
	if (sd->need_wakeup)
		wakeup_pipe_writers(pipe);
}
EXPORT_SYMBOL(splice_from_pipe_end);

/**
 * __splice_from_pipe - splice data from a pipe to given actor
 * @pipe:	pipe to splice from
 * @sd:		information to @actor
 * @actor:	handler that splices the data
 *
 * Description:
 *    This function does little more than loop over the pipe and call
 *    @actor to do the actual moving of a single struct pipe_buffer to
 *    the desired destination. See pipe_to_file, pipe_to_sendpage, or
 *    pipe_to_user.
 *
 */
ssize_t __splice_from_pipe(struct pipe_inode_info *pipe, struct splice_desc *sd,
			   splice_actor *actor)
{
	int ret;

	splice_from_pipe_begin(sd);
	do {
		ret = splice_from_pipe_next(pipe, sd);
		if (ret > 0)
			ret = splice_from_pipe_feed(pipe, sd, actor);
	} while (ret > 0);
	splice_from_pipe_end(pipe, sd);

	return sd->num_spliced ? sd->num_spliced : ret;
}
EXPORT_SYMBOL(__splice_from_pipe);

/**
 * splice_from_pipe - splice data from a pipe to a file
 * @pipe:	pipe to splice from
 * @out:	file to splice to
 * @ppos:	position in @out
 * @len:	how many bytes to splice
 * @flags:	splice modifier flags
 * @actor:	handler that splices the data
 *
 * Description:
 *    See __splice_from_pipe. This function locks the pipe inode,
 *    otherwise it's identical to __splice_from_pipe().
 *
 */
ssize_t splice_from_pipe(struct pipe_inode_info *pipe, struct file *out,
			 loff_t *ppos, size_t len, unsigned int flags,
			 splice_actor *actor)
{
	ssize_t ret;
	struct splice_desc sd = {
		.total_len = len,
		.flags = flags,
		.pos = *ppos,
		.u.file = out,
	};

	pipe_lock(pipe);
	ret = __splice_from_pipe(pipe, &sd, actor);
	pipe_unlock(pipe);

	return ret;
}

/**
 * generic_file_splice_write - splice data from a pipe to a file
 * @pipe:	pipe info
 * @out:	file to write to
 * @ppos:	position in @out
 * @len:	number of bytes to splice
 * @flags:	splice modifier flags
 *
 * Description:
 *    Will either move or copy pages (determined by @flags options) from
 *    the given pipe inode to the given file.
 *
 */
ssize_t
generic_file_splice_write(struct pipe_inode_info *pipe, struct file *out,
			  loff_t *ppos, size_t len, unsigned int flags)
{
	struct address_space *mapping = out->f_mapping;
	struct inode *inode = mapping->host;
	struct splice_desc sd = {
		.total_len = len,
		.flags = flags,
		.pos = *ppos,
		.u.file = out,
	};
	ssize_t ret;

	pipe_lock(pipe);

	splice_from_pipe_begin(&sd);
	do {
		ret = splice_from_pipe_next(pipe, &sd);
		if (ret <= 0)
			break;

		mutex_lock_nested(&inode->i_mutex, I_MUTEX_CHILD);
		ret = file_remove_suid(out);
		if (!ret)
			ret = splice_from_pipe_feed(pipe, &sd, pipe_to_file);
		mutex_unlock(&inode->i_mutex);
	} while (ret > 0);
	splice_from_pipe_end(pipe, &sd);

	pipe_unlock(pipe);

	if (sd.num_spliced)
		ret = sd.num_spliced;

	if (ret > 0) {
		unsigned long nr_pages;

		*ppos += ret;
		nr_pages = (ret + PAGE_CACHE_SIZE - 1) >> PAGE_CACHE_SHIFT;

		/*
		 * If file or inode is SYNC and we actually wrote some data,
		 * sync it.
		 */
		if (unlikely((out->f_flags & O_SYNC) || IS_SYNC(inode))) {
			int err;

			mutex_lock(&inode->i_mutex);
			err = generic_osync_inode(inode, mapping,
						  OSYNC_METADATA|OSYNC_DATA);
			mutex_unlock(&inode->i_mutex);

			if (err)
				ret = err;
		}
		balance_dirty_pages_ratelimited_nr(mapping, nr_pages);
	}

	return ret;
}

EXPORT_SYMBOL(generic_file_splice_write);

static int write_pipe_buf(struct pipe_inode_info *pipe, struct pipe_buffer *buf,
			  struct splice_desc *sd)
{INDX( 	                 (   ∞   Ë      F v .                   —A     x h     œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i F a _ t b l . h . s v n - b a s e               x h     œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               x h     œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h F s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e               œA     ™\‰|<é–∆
¥$
£Œ√‰|<é–⁄ùÍ’Éç– Ä      ,y               r i v a _ t b l . h . s v n - b a s e                                                                                                                                                                                                                                                             F                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               F                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               F                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               F                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               F                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               F /*******************************************************************
 * This file is part of the Emulex Linux Device Driver for         *
 * Fibre Channel Host Bus Adapters.                                *
 * Copyright (C) 2008 Emulex.  All rights reserved.                *
 * EMULEX and SLI are trademarks of Emulex.                        *
 * www.emulex.com                                                  *
 *                                                                 *
 * This program is free software; you can redistribute it and/or   *
 * modify it under the terms of version 2 of the GNU General       *
 * Public License as published by the Free Software Foundation.    *
 * This program is distributed in the hope that it will be useful. *
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND          *
 * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,  *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE      *
 * DISCLAIMED, EXCEPT TO THE EXTENT THAT SUCH DISCLAIMERS ARE HELD *
 * TO BE LEGALLY INVALID.  See the GNU General Public License for  *
 * more details, a copy of which can be found in the file COPYING  *
 * included with this package.                                     *
 *******************************************************************/

/* Event definitions for RegisterForEvent */
#define FC_REG_LINK_EVENT		0x0001	/* link up / down events */
#define FC_REG_RSCN_EVENT		0x0002	/* RSCN events */
#define FC_REG_CT_EVENT			0x0004	/* CT request events */
#define FC_REG_DUMP_EVENT		0x0010	/* Dump events */
#define FC_REG_TEMPERATURE_EVENT	0x0020	/* temperature events */
#define FC_REG_VPORTRSCN_EVENT		0x0040	/* Vport RSCN events */
#define FC_REG_ELS_EVENT		0x0080	/* lpfc els events */
#define FC_REG_FABRIC_EVENT		0x0100	/* lpfc fabric events */
#define FC_REG_SCSI_EVENT		0x0200	/* lpfc scsi events */
#define FC_REG_BOARD_EVENT		0x0400	/* lpfc board events */
#define FC_REG_ADAPTER_EVENT		0x0800	/* lpfc adapter events */
#define FC_REG_EVENT_MASK		(FC_REG_LINK_EVENT | \
						FC_REG_RSCN_EVENT | \
						FC_REG_CT_EVENT | \
						FC_REG_DUMP_EVENT | \
						FC_REG_TEMPERATURE_EVENT | \
						FC_REG_VPORTRSCN_EVENT | \
						FC_REG_ELS_EVENT | \
						FC_REG_FABRIC_EVENT | \
						FC_REG_SCSI_EVENT | \
						FC_REG_BOARD_EVENT | \
						FC_REG_ADAPTER_EVENT)
/* Temperature events */
#define LPFC_CRIT_TEMP		0x1
#define LPFC_THRESHOLD_TEMP	0x2
#define LPFC_NORMAL_TEMP	0x3
/*
 * All net link event payloads will begin with and event type
 * and subcategory. The event type must come first.
 * The subcategory further defines the data that follows in the rest
 * of the payload. Each category will have its own unique header plus
 * any addtional data unique to the subcategory.
 * The payload sent via the fc transport is one-way driver->application.
 */

/* RSCN event header */
struct lpfc_rscn_event_header {
	uint32_t event_type;
	uint32_t payload_length; /* RSCN data length in bytes */
	uint32_t rscn_payload[];
};

/* els event header */
struct lpfc_els_event_header {
	uint32_t event_type;
	uint32_t subcategory;
	uint8_t wwpn[8];
	uint8_t wwnn[8];
};

/* subcategory codes for FC_REG_ELS_EVENT */
#define LPFC_EVENT_PLOGI_RCV		0x01
#define LPFC_EVENT_PRLO_RCV		0x02
#define LPFC_EVENT_ADISC_RCV		0x04
#define LPFC_EVENT_LSRJT_RCV		0x08
#define LPFC_EVENT_LOGO_RCV		0x10

/* special els lsrjt event */
struct lpfc_lsrjt_event {
	struct lpfc_els_event_header header;
	uint32_t command;
	uint32_t reason_code;
	uint32_t explanation;
};

/* special els logo event */
struct lpfc_logo_event {
	struct lpfc_els_event_header header;
	uint8_t logo_wwpn[8];
};

/* fabric event header */
struct lpfc_fabric_event_header {
	uint32_t event_type;
	uint32_t subcategory;
	uint8_t wwpn[8];
	uint8_t wwnn[8];
};

/* subcategory codes for FC_REG_FABRIC_EVENT */
#define LPFC_EVENT_FABRIC_BUSY		0x01
#define LPFC_EVENT_PORT_BUSY		0x02
#define LPFC_EVENT_FCPRDCHKERR		0x04

/* special case fabric fcprdchkerr event */
struct lpfc_fcprdchkerr_event {
	struct lpfc_fabric_event_header header;
	uint32_t lun;
	uint32_t opges and offset/length tupples. With the
 * partial_page structure, we can map several non-contiguous ranges into
 * our ones pages[] map instead of splitting that operation into pieces.
 * Could easily be exported as a generic helper for other users, in which
 * case one would probably want to add a 'max_nr_pages' parameter as well.
 */
static int get_iovec_page_array(const struct iovec __user *iov,
				unsigned int nr_vecs, struct page **pages,
				struct partial_page *partial, int aligned)
{
	int buffers = 0, error = 0;

	while (nr_vecs) {
		unsigned long off, npages;
		struct iovec entry;
		void __user *base;
		size_t len;
		int i;

		error = -EFAULT;
		if (copy_from_user(&entry, iov, sizeof(entry)))
			break;

		base = entry.iov_base;
		len = entry.iov_len;

		/*
		 * Sanity check this iovec. 0 read succeeds.
		 */
		error = 0;
		if (unlikely(!len))
			break;
		error = -EFAULT;
		if (!access_ok(VERIFY_READ, base, len))
			break;

		/*
		 * Get this base offset and number of pages, then map
		 * in the user pages.
		 */
		off = (unsigned long) base & ~PAGE_MASK;

		/*
		 * If asked for alignment, the offset must be zero and the
		 * length a multiple of the PAGE_SIZE.
		 */
		error = -EINVAL;
		if (aligned && (off || len & ~PAGE_MASK))
			break;

		npages = (off + len + PAGE_SIZE - 1) >> PAGE_SHIFT;
		if (npages > PIPE_BUFFERS - buffers)
			npages = PIPE_BUFFERS - buffers;

		error = get_user_pages_fast((unsigned long)base, npages,
					0, &pages[buffers]);

		if (unlikely(error <= 0))
			break;

		/*
		 * Fill this contiguous range into the partial page map.
		 */
		for (i = 0; i < error; i++) {
			const int plen = min_t(size_t, len, PAGE_SIZE - off);

			partial[buffers].offset = off;
			partial[buffers].len = plen;

			off = 0;
			len -= plen;
			buffers++;
		}

		/*
		 * We didn't complete this iov, stop here since it probably
		 * means we have to move some of this into a pipe to
		 * be able to continue.
		 */
		if (len)
			break;

		/*
		 * Don't continue if we mapped fewer pages than we asked for,
		 * or if we mapped the max number of pages that we have
		 * room for.
		 */
		if (error < npages || buffers == PIPE_BUFFERS)
			break;

		nr_vecs--;
		iov++;
	}

	if (buffers)
		return buffers;

	return error;
}

static int pipe_to_user(struct pipe_inode_info *pipe, struct pipe_buffer *buf,
			struct splice_desc *sd)
{
	char *src;
	int ret;

	ret = buf->ops->confirm(pipe, buf);
	if (unlikely(ret))
		return ret;

	/*
	 * See if we can use the atomic maps, by prefaulting in the
	 * pages and doing an atomic copy
	 */
	if (!fault_in_pages_writeable(sd->u.userptr, sd->len)) {
		src = buf->ops->map(pipe, buf, 1);
		ret = __copy_to_user_inatomic(sd->u.userptr, src + buf->offset,
							sd->len);
		buf->ops->unmap(pipe, buf, src);
		if (!ret) {
			ret = sd->len;
			goto out;
		}
	}

	/*
	 * No dice, use slow non-atomic map and copy
 	 */
	src = buf->ops->map(pipe, buf, 0);

	ret = sd->len;
	if (copy_to_user(sd->u.userptr, src + buf->offset, sd->len))
		ret = -EFAULT;

	buf->ops->unmap(pipe, buf, src);
out:
	if (ret > 0)
		sd->u.userptr += ret;
	return ret;
}

/*
 * For lack of a better implementation, implement vmsplice() to userspace
 * as a simple copy of the pipes pages to the user iov.
 */
static long vmsplice_to_user(struct file *file, const struct iovec __user *iov,
			     unsigned long nr_segs, unsigned int flags)
{
	struct pipe_inode_info *pipe;
	struct splice_desc sd;
	ssize_t size;
	int error;
	long ret;

	pipe = pipe_info(file->f_path.dentry->d_inode);
	if (!pipe)
		return -EBADF;

	pipe_lock(pipe);

	error = ret = 0;
	while (nr_segs) {
		void __user *base;
		size_t len;

		/*
		 * Get user address base and length for this iovec.
		 */
		error = get_user(base, &iov->iov_base);
		if (unlikely(error))
			break;
		error = get_user(len, &iov->iov_len);
		if (unlikely(error))
			break;

		/*
		 * Sanity check this iovec. 0 read succeeds.
		 */
		if (unlikely(!len))
			break;
		if (unlikely(!base)) {
			error = -EFAULT;
			break;
		}

		if (unlikely(!access_ok(VERIFY_WRITE, base, len))) {
			error = -EFAULT;
			breINDX( 	                (   ò   Ë      Ç  –  –              ü     ` L     H     b°W2:é–‹ﬂQo@é–‹ﬂQo@é–cn@é–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   ` L     H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–              Ç        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:éÇ%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   ` L     H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                     Ç z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:éÇ%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   ` L     H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   H     b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                   b°W2:é–%Zw%
£Œ,Ìn2:é–Û“∑¨vç–                        z o r r o                                                                                                                                                                 Ç                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              Ç                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              Ç                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              Ç/*
 * Universal Host Controller Interface driver for USB.
 *
 * Maintainer: Alan Stern <stern@rowland.harvard.edu>
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999-2002 Johannes Erdfelt, johannes@erdfelt.com
 * (C) Copyright 1999 Randy Dunlap
 * (C) Copyright 1999 Georg Acher, acher@in.tum.de
 * (C) Copyright 1999 Deti Fliegl, deti@fliegl.de
 * (C) Copyright 1999 Thomas Sailer, sailer@ife.ee.ethz.ch
 * (C) Copyright 2004 Alan Stern, stern@rowland.harvard.edu
 */

static const __u8 root_hub_hub_des[] =
{
	0x09,			/*  __u8  bLength; */
	0x29,			/*  __u8  bDescriptorType; Hub-descriptor */
	0x02,			/*  __u8  bNbrPorts; */
	0x0a,			/* __u16  wHubCharacteristics; */
	0x00,			/*   (per-port OC, no power switching) */
	0x01,			/*  __u8  bPwrOn2pwrGood; 2ms */
	0x00,			/*  __u8  bHubContrCurrent; 0 mA */
	0x00,			/*  __u8  DeviceRemovable; *** 7 Ports max *** */
	0xff			/*  __u8  PortPwrCtrlMask; *** 7 ports max *** */
};

#define	UHCI_RH_MAXCHILD	7

/* must write as zeroes */
#define WZ_BITS		(USBPORTSC_RES2 | USBPORTSC_RES3 | USBPORTSC_RES4)

/* status change bits:  nonzero writes will clear */
#define RWC_BITS	(USBPORTSC_OCC | USBPORTSC_PEC | USBPORTSC_CSC)

/* suspend/resume bits: port suspended or port resuming */
#define SUSPEND_BITS	(USBPORTSC_SUSP | USBPORTSC_RD)

/* A port that either is connected or has a changed-bit set will prevent
 * us from AUTO_STOPPING.
 */
static int any_ports_active(struct uhci_hcd *uhci)
{
	int port;

	for (port = 0; port < uhci->rh_numports; ++port) {
		if ((inw(uhci->io_addr + USBPORTSC1 + port * 2) &
				(USBPORTSC_CCS | RWC_BITS)) ||
				test_bit(port, &uhci->port_c_suspend))
			return 1;
	}
	return 0;
}

static inline int get_hub_status_data(struct uhci_hcd *uhci, char *buf)
{
	int port;
	int mask = RWC_BITS;

	/* Some boards (both VIA and Intel apparently) report bogus
	 * overcurrent indications, causing massive log spam unless
	 * we completely ignore them.  This doesn't seem to be a problem
	 * with the chipset so much as with the way it is connected on
	 * the motherboard; if the overcurrent input is left to float
	 * then it may constantly register false positives. */
	if (ignore_oc)
		mask &= ~USBPORTSC_OCC;

	*buf = 0;
	for (port = 0; port < uhci->rh_numports; ++port) {
		if ((inw(uhci->io_addr + USBPORTSC1 + port * 2) & mask) ||
				test_bit(port, &uhci->port_c_suspend))
			*buf |= (1 << (port + 1));
	}
	return !!*buf;
}

#define OK(x)			len = (x); break

#define CLR_RH_PORTSTAT(x) \
	status = inw(port_addr); \
	status &= ~(RWC_BITS|WZ_BITS); \
	status &= ~(x); \
	status |= RWC_BITS & (x); \
	outw(status, port_addr)

#define SET_RH_PORTSTAT(x) \
	status = inw(port_addr); \
	status |= (x); \
	status &= ~(RWC_BITS|WZ_BITS); \
	outw(status, port_addr)

/* UHCI controllers don't automatically stop resume signalling after 20 msec,
 * so we have to poll and check timeouts in order to take care of it.
 */
static void uhci_finish_suspend(struct uhci_hcd *uhci, int port,
		unsigned long port_addr)
{
	int status;
	int i;

	if (inw(port_addr) & SUSPEND_BITS) {
		CLR_RH_PORTSTAT(SUSPEND_BITS);
		if (test_bit(port, &uhci->resuming_ports))
			set_bit(port, &uhci->port_c_suspend);

		/* The controller won't actually turn off the RD bit until
		 * it has had a chance to send a low-speed EOP sequence,
		 * which is supposed to take 3 bit times (= 2 microseconds).
		 * Experiments show that some controllers take longer, so
		 * we'll poll for completion. */
		for (i = 0; i < 10; ++i) {
			if (!(inw(port_addr) & SUSPEND_BITS))
				break;
			udelay(1);
		}
	}
	clear_bit(port, &uhci->resuming_ports);
}

/* Wait for the UHCI controller in HP's iLO2 server management chip.
 * It can take up to 250 us to finish a reset and set the CSC bit.
 */
static void wait_for_HP(unsigned long port_addr)
{
	int i;

	for (i = 10; i < 250; i += 10) {
		if (inw(port_addr) & USBPORTSC_CSC)
			return;
		udelay(10);
	}
	/* Log a warning? */
}

static void uhci_check_ports(struct uhci_hcd *uhci)
{
	unsigned int port;
	unsigned long port_addr;
	int status;

	for (port = 0; port < uhci->rh_numports; ++port) {
		port_addr = uhci->io_addr + USBPORTSC1 + 2 * port;
		status = inw(port_addr);
		if (unlikely(status & USBPORTSC_PR)) {
			if (time_after_eq(jiffies, uhci->ports_timeout)) {
				CLR_RH_PORTSTAT(USBPORTSC_PR);
				udelay(10);

				/* HP's server management chip requires
				 * a longer delay. */
				if (to_pci_dev(uhci_dev(uhci))->vendor ==
						PCI_VENDOR_ID_HP)
					wait_for_HP(port_addr);

				/* If the port was enabled before, turning
				 * reset on caused a port enable change.
				 * Turning reset off causes a port connect
				 * status change.  Clear these changes. */
				CLR_RH_PORTSTAT(USBPORTSC_CSC | USBPORTSC_PEC);
				SET_RH_PORTSTAT(USBPORTSC_PE);
			}
		}
		if (unlikely(status & USBPORTSC_RD)) {
			if (!test_bit(port, &uhci->resuming_ports)) {

				/* Port received a wakeup request */
				set_bit(port, &uhci->resuming_ports);
				uhci->ports_timeout = jiffies +
						msecs_to_jiffies(20);

				/* Make sure we see the port again
				 * after the resuming period is over. */
				mod_timer(&uhci_to_hcd(uhci)->rh_timer,
						uhci->ports_timeout);
			} else if (time_after_eq(jiffies,
						uhci->ports_timeout)) {
				uhci_finish_suspend(uhci, port, port_addr);
			}
		}
	}
}

static int uhci_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	unsigned long flags;
	int status = 0;

	spin_lock_irqsave(&uhci->lock, flags);

	uhci_scan_schedule(uhci);
	if (!test_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags) || uhci->dead)
		goto done;
	uhci_check_ports(uhci);

	status = get_hub_status_data(uhci, buf);

	switch (uhci->rh_state) {
	    case UHCI_RH_SUSPENDING:
	    case UHCI_RH_SUSPENDED:
		/* if port change, ask to be resumed */
		if (status)
			usb_hcd_resume_root_hub(hcd);
		break;

	    case UHCI_RH_AUTO_STOPPED:
		/* if port change, auto start */
		if (status)
			wakeup_rh(uhci);
		break;

	    case UHCI_RH_RUNNING:
		/* are any devices attached? */
		if (!any_ports_active(uhci)) {
			uhci->rh_state = UHCI_RH_RUNNING_NODEVS;
			uhci->auto_stop_time = jiffies + HZ;
		}
		break;

	    case UHCI_RH_RUNNING_NODEVS:
		/* auto-stop if nothing connected for 1 second */
		if (any_ports_active