fgdev->buffer_lock, flags);

	if (arg_out && copy_to_user(arg, &p, sizeof(p)))
		r = -EFAULT;

	return r;
}

static unsigned long get_cstate_change(struct b3dfg_dev *fgdev)
{
	unsigned long flags, when;

	spin_lock_irqsave(&fgdev->cstate_lock, flags);
	when = fgdev->cstate_tstamp;
	spin_unlock_irqrestore(&fgdev->cstate_lock, flags);
	return when;
}

static int is_event_ready(struct b3dfg_dev *fgdev, struct b3dfg_buffer *buf,
			  unsigned long when)
{
	int result;
	unsigned long flags;

	spin_lock_irqsave(&fgdev->buffer_lock, flags);
	spin_lock(&fgdev->cstate_lock);
	result = (!fgdev->transmission_enabled ||
		  buf->state == B3DFG_BUFFER_POPULATED ||
		  when != fgdev->cstate_tstamp);
	spin_unlock(&fgdev->cstate_lock);
	spin_unlock_irqrestore(&fgdev->buffer_lock, flags);

	return result;
}

/* sleep until a specific buffer becomes populated */
static int wait_buffer(struct b3dfg_dev *fgdev, void __user *arg)
{
	struct device *dev = &fgdev->pdev->dev;
	struct b3dfg_wait w;
	struct b3dfg_buffer *buf;
	unsigned long flags, when;
	int r;

	if (copy_from_user(&w, arg, sizeof(w)))
		return -EFAULT;

	if (!fgdev->transmission_enabled) {
		dev_dbg(dev, "cannot wait, transmission disabled\n");
		return -EINVAL;
	}

	if (w.buffer_idx < 0 || w.buffer_idx >= b3dfg_nbuf)
		return -ENOENT;

	buf = &fgdev->buffers[w.buffer_idx];

	spin_lock_irqsave(&fgdev->buffer_lock, flags);

	if (buf->state == B3DFG_BUFFER_POPULATED) {
		r = w.timeout;
		goto out_triplets_dropped;
	}

	spin_unlock_irqrestore(&fgdev->buffer_lock, flags);

	when = get_cstate_change(fgdev);
	if (w.timeout > 0) {
		r = wait_event_interruptible_timeout(fgdev->buffer_waitqueue,
			is_event_ready(fgdev, buf, when),
			(w.timeout * HZ) / 1000);

		if (unlikely(r < 0))
			goto out;

		w.timeout = r * 1000 / HZ;
	} else {
		r = wait_event_interruptible(fgdev->buffer_waitqueue,
			is_event_ready(fgdev, buf, when));

		if (unlikely(r)) {
			r = -ERESTARTSYS;
			goto out;
		}
	}

	/* TODO: Inform the user via field(s) in w? */
	if (!fgdev->transmission_enabled || when != get_cstate_change(fgdev)) {
		r = -EINVAL;
		goto out;
	}

	spin_lock_irqsave(&fgdev->buffer_lock, flags);

	if (buf->state != B3DFG_BUFFER_POPULATED) {
		r = -ETIMEDOUT;
		goto out_unlock;
	}

	buf->state = B3DFG_BUFFER_POLLED;

out_triplets_dropped:

	/* IRQs already disabled by spin_lock_irqsave above. */
	spin_lock(&fgdev->triplets_dropped_lock);
	w.triplets_dropped = fgdev->triplets_dropped;
	fgdev->triplets_dropped = 0;
	spin_unlock(&fgdev->triplets_dropped_lock);

out_unlock:
	spin_unlock_irqrestore(&fgdev->buffer_lock, flags);
	if (copy_to_user(arg, &w, sizeof(w)))
		r = -EFAULT;
out:
	return r;
}

/* mmap page fault handler */
static int b3dfg_vma_fault(struct vm_area_struct *vma,
	struct vm_fault *vmf)
{
	struct b3dfg_dev *fgdev = vma->vm_file->private_data;
	unsigned long off = vmf->pgoff << PAGE_SHIFT;
	unsigned int frame_size = fgdev->frame_size;
	unsigned int buf_size = frame_size * B3DFG_FRAMES_PER_BUFFER;
	unsigned char *addr;

	/* determine which buffer the offset lies within */
	unsigned int buf_idx = off / buf_size;
	/* and the offset into the buffer */
	unsigned int buf_off = off % buf_size;

	/* determine which frame inside the buffer the offset lies in */
	unsigned int frm_idx = buf_off / frame_size;
	/* and the offset into the frame */
	unsigned int frm_off = buf_off % frame_size;

	if (unlikely(buf_idx >= b3dfg_nbuf))
		return VM_FAULT_SIGBUS;

	addr = fgdev->buffers[buf_idx].frame[frm_idx] + frm_off;
	vm_insert_pfn(vma, (unsigned long)vmf->virtual_address,
			  virt_to_phys(addr) >> PAGE