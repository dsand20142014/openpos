            "writable (flags=%lx)\n", vma->vm_flags);
                        ret = -EPERM;
                        goto bail;
                }
                /*
                 * Don't allow permission to later change to writeable
                 * with mprotect.
                 */
                vma->vm_flags &= ~VM_MAYWRITE;
	} else {
		goto bail;
	}
	len = vma->vm_end - vma->vm_start;
	if (len > size) {
		ipath_cdbg(MM, "FAIL: reqlen %lx > %zx\n", len, size);
		ret = -EINVAL;
		goto bail;
	}

	vma->vm_pgoff = (unsigned long) addr >> PAGE_SHIFT;
	vma->vm_ops = &ipath_file_vm_ops;
	vma->vm_flags |= VM_RESERVED | VM_DONTEXPAND;
	ret = 1;

bail:
	return ret;
}

/**
 * ipath_mmap - mmap various structures into user space
 * @fp: the file pointer
 * @vma: the VM area
 *
 * We use this to have a shared buffer between the kernel and the user code
 * for the rcvhdr queue, egr buffers, and the per-port user regs and pio
 * buffers in the chip.  We have the open and close entries so we can bump
 * the ref count and keep the driver from being unloaded while still mapped.
 */
static int ipath_mmap(struct file *fp, struct vm_area_struct *vma)
{
	struct ipath_portdata *pd;
	struct ipath_devdata *dd;
	u64 pgaddr, ureg;
	unsigned piobufs, piocnt;
	int ret;

	pd = port_fp(fp);
	if (!pd) {
		ret = -EINVAL;
		goto bail;
	}
	dd = pd->port_dd;

	/*
	 * This is the ipath_do_user_init() code, mapping the shared buffers
	 * into the user process. The address referred to by vm_pgoff is the
	 * file offset passed via mmap().  For shared ports, this is the
	 * kernel vmalloc() address of the pages to share with the master.
	 * For non-shared or master ports, this is a physical address.
	 * We only do one mmap for each space mapped.
	 */
	pgaddr = vma->vm_pgoff << PAGE_SHIFT;

	/*
	 * Check for 0 in case one of the allocations failed, but user
	 * called mmap anyway.
	 */
	if (!pgaddr)  {
		ret = -EINVAL;
		goto bail;
	}

	ipath_cdbg(MM, "pgaddr %llx vm_start=%lx len %lx port %u:%u:%u\n",
		   (unsigned long long) pgaddr, vma->vm_start,
		   vma->vm_end - vma->vm_start, dd->ipath_unit,
		   pd->port_port, subport_fp(fp));

	/*
	 * Physical addresses must fit in 40 bits for our hardware.
	 * Check for kernel virtual addresses first, anything else must
	 * match a HW or memory address.
	 */
	ret = mmap_kvaddr(vma, pgaddr, pd, subport_fp(fp));
	if (ret) {
		if (ret > 0)
			ret = 0;
		goto bail;
	}

	ureg = dd->ipath_uregbase + dd->ipath_ureg_align * pd->port_port;
	if (!pd->port_subport_cnt) {
		/* port is not shared */
		piocnt = pd->port_piocnt;
		piobufs = pd->port_piobufs;
	} else if (!subport_fp(fp)) {
		/* caller is the master */
		piocnt = (pd->port_piocnt / pd->port_subport_cnt) +
			 (pd->port_piocnt % pd->port_subport_cnt);
		piobufs = pd->port_piobufs +
			dd->ipath_palign * (pd->port_piocnt - piocnt);
	} else {
		unsigned slave = subport_fp(fp) - 1;

		/* caller is a slave */
		piocnt = pd->port_piocnt / pd->port_subport_cnt;
		piobufs = pd->port_piobufs + dd->ipath_palign * piocnt * slave;
	}

	if (pgaddr == ureg)
		ret = mmap_ureg(vma, dd, ureg);
	else if (pgaddr == piobufs)
		ret = mmap_piobufs(vma, dd, pd, piobufs, piocnt);
	else if (pgaddr == dd->ipath_pioavailregs_phys)
		/* in-memory copy of pioavail registers */
		ret = ipath_mmap_mem(vma, pd, PAGE_SIZE, 0,
			      	     (void *) dd->ipath_pioavailregs_dma,
				     "pioavail registers");
	else if (pgaddr == pd->port_rcvegr_phys)
		ret = mmap_rcvegrbufs(vma, pd);
	else if (pgaddr == (u64) pd->port_rcvhdrq_phys)
		/*
		 * The rcvhdrq itself; readonly except on HT (so have
		 * to allow writable mapping), multiple pages, contiguous
		 * from an i/o perspective.
		 */
		ret = ipath_mmap_mem(vma, pd, pd->port_rcvhdrq_size, 1,
				     pd->port_rcvhdrq,
				     "rcvhdrq");
	else if (pgaddr == (u64) pd->port_rcvhdrqtailaddr_phys)
		/* in-memory copy of rcvhdrq tail register */
		ret = ipath_mmap_mem(vma, pd, PAGE_SIZE, 0,
				     pd->port_rcvhdrtail_kvaddr,
				     "rcvhdrq tail");
	else
		ret = -EINVAL;

	vma->vm_private_data = NULL;

	if (ret < 0)
		dev_info(&dd->pcidev->dev,
			 "Failure %d on off %llx len %lx\n",
			 -ret, (unsigned long long)pgaddr,
			 vma->vm_end - vma->vm_start);
bail:
	return ret;
}

static unsigned ipath_poll_hdrqfull(struct ipath_portdata *pd)
{
	unsigned pollflag = 0;

	if ((pd->poll_type & IPATH_POLL_TYPE_OVERFLOW) &&
	    pd->port_hdrqfull != pd->port_hdrqfull_poll) {
		pollflag |= POLLIN | POLLRDNORM;
		pd->port_hdrqfull_poll = pd->port_hdrqfull;
	}

	return pollflag;
}

static unsigned int ipath_poll_urgent(struct ipath_portdata *pd,
				      struct file *fp,
				      struct poll_table_struct *pt)
{
	unsigned pollflag = 0;
	struct ipath_devdata *dd;

	dd = pd->port_dd;

	/* variable access in ipath_poll_hdrqfull() needs this */
	rmb();
	pollflag = ipath_poll_hdrqfull(pd);

	if (pd->port_urgent != pd->port_urgent_poll) {
		pollflag |= POLLIN | POLLRDNORM;
		pd->port_urgent_poll = pd->port_urgent;
	}

	if (!pollflag) {
		/* this saves a spin_lock/unlock in interrupt handler... */
		set_bit(IPATH_PORT_WAITING_URG, &pd->port_flag);
		/* flush waiting flag so don't miss an event... */
		wmb();
		poll_wait(fp, &pd->port_wait, pt);
	}

	return pollflag;
}

static unsigned int ipath_poll_next(struct ipath_portdata *pd,
				    struct file *fp,
				    struct poll_table_struct *pt)
{
	u32 head;
	u32 tail;
	unsigned pollflag = 0;
	struct ipath_devdata *dd;

	dd = pd->port_dd;

	/* variable access in ipath_poll_hdrqfull() needs this */
	rmb();
	pollflag = ipath_poll_hdrqfull(pd);

	head = ipath_read_ureg32(dd, ur_rcvhdrhead, pd->port_port);
	if (pd->port_rcvhdrtail_kvaddr)
		tail = ipath_get_rcvhdrtail(pd);
	else
		tail = ipath_read_ureg32(dd, ur_rcvhdrtail, pd->port_port);

	if (head != tail)
		pollflag |= POLLIN | POLLRDNORM;
	else {
		/* this saves a spin_lock/unlock in interrupt handler */
		set_bit(IPATH_PORT_WAITING_RCV, &pd->port_flag);
		/* flush waiting flag so we don't miss an event */
		wmb();

		set_bit(pd->port_port + dd->ipath_r_intravail_shift,
			&dd->ipath_rcvctrl);

		ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvctrl,
				 dd->ipath_rcvctrl);

		if (dd->ipath_rhdrhead_intr_off) /* arm rcv interrupt */
			ipath_write_ureg(dd, ur_rcvhdrhead,
					 dd->ipath_rhdrhead_intr_off | head,
					 pd->port_port);

		poll_wait(fp, &pd->port_wait, pt);
	}

	return pollflag;
}

static unsigned int ipath_poll(struct file *fp,
			       struct poll_table_struct *pt)
{
	struct ipath_portdata *pd;
	unsigned pollflag;

	pd = port_fp(fp);
	if (!pd)
		pollflag = 0;
	else if (pd->poll_type & IPATH_POLL_TYPE_URGENT)
		pollflag = ipath_poll_urgent(pd, fp, pt);
	else
		pollflag = ipath_poll_next(pd, fp, pt);

	return pollflag;
}

static int ipath_supports_subports(int user_swmajor, int user_swminor)
{
	/* no subport implementation prior to software version 1.3 */
	return (user_swmajor > 1) || (user_swminor >= 3);
}

static int ipath_compatible_subports(int user_swmajor, int user_swminor)
{
	/* this code is written long-hand for clarity */
	if (IPATH_USER_SWMAJOR != user_swmajor) {
		/* no promise of compatibility if major mismatch */
		return 0;
	}
	if (IPATH_USER_SWMAJOR == 1) {
		switch (IPATH_USER_SWMINOR) {
		case 0:
		case 1:
		case 2:
			/* no subport implementation so cannot be compatible */
			return 0;
		case 3:
			/* 3 is only compatible with itself */
			return user_swminor == 3;
		default:
			/* >= 4 are compatible (or are expected to be) */
			return user_swminor >= 4;
		}
	}
	/* make no promises yet for future major versions */
	return 0;
}

static int init_subports(struct ipath_devdata *dd,
			 struct ipath_portdata *pd,
			 const struct ipath_user_info *uinfo)
{
	int ret = 0;
	unsigned num_subports;
	size_t size;

	/*
	 * If the user is requesting zero subports,
	 * skip the subport allocation.
	 */
	if (uinfo->spu_subport_cnt <= 0)
		goto bail;

	/* Self-consistency check for ipath_compatible_subports() */
	if (ipath_supports_subports(IPATH_USER_SWMAJOR, IPATH_USER_SWMINOR) &&
	    !ipath_compatible_subports(IPATH_USER_SWMAJOR,
				       IPATH_USER_SWMINOR)) {
		dev_info(&dd->pcidev->dev,
			 "Inconsistent ipath_compatible_subports()\n");
		goto bail;
	}

	/* Check for subport compatibility */
	if (!ipath_compatible_subports(uinfo->spu_userversion >> 16,
				       uinfo->spu_userversion & 0xffff)) {
		dev_info(&dd->pcidev->dev,
			 "Mismatched user version (%d.%d) and driver "
			 "version (%d.%d) while port sharing. Ensure "
                         "that driver and library are from the same "
                         "release.\n",
			 (int) (uinfo->spu_userversion >> 16),
                         (int) (uinfo->spu_userversion & 0xffff),
			 IPATH_USER_SWMAJOR,
	                 IPATH_USER_SWMINOR);
		goto bail;
	}
	if (uinfo->spu_subport_cnt > INFINIPATH_MAX_SUBPORT) {
		ret = -EINVAL;
		goto bail;
	}

	num_subports = uinfo->spu_subport_cnt;
	pd->subport_uregbase = vmalloc(PAGE_SIZE * num_subports);
	if (!pd->subport_uregbase) {
		ret = -ENOMEM;
		goto bail;
	}
	/* Note: pd->port_rcvhdrq_size isn't initialized yet. */
	size = ALIGN(dd->ipath_rcvhdrcnt * dd->ipath_rcvhdrentsize *
		     sizeof(u32), PAGE_SIZE) * num_subports;
	pd->subport_rcvhdr_base = vmalloc(size);
	if (!pd->subport_rcvhdr_base) {
		ret = -ENOMEM;
		goto bail_ureg;
	}

	pd->subport_rcvegrbuf = vmalloc(pd->port_rcvegrbuf_chunks *
					pd->port_rcvegrbuf_size *
					num_subports);
	if (!pd->subport_rcvegrbuf) {
		ret = -ENOMEM;
		goto bail_rhdr;
	}

	pd->port_subport_cnt = uinfo->spu_subport_cnt;
	pd->port_subport_id = uinfo->spu_subport_id;
	pd->active_slaves = 1;
	set_bit(IPATH_PORT_MASTER_UNINIT, &pd->port_flag);
	memset(pd->subport_uregbase, 0, PAGE_SIZE * num_subports);
	memset(pd->subport_rcvhdr_base, 0, size);
	memset(pd->subport_rcvegrbuf, 0, pd->port_rcvegrbuf_chunks *
				         pd->port_rcvegrbuf_size *
				         num_subports);
	goto bail;

bail_rhdr:
	vfree(pd->subport_rcvhdr_base);
bail_ureg:
	vfree(pd->subport_uregbase);
	pd->subport_uregbase = NULL;
bail:
	return ret;
}

static int try_alloc_port(struct ipath_devdata *dd, int port,
			  struct file *fp,
			  const struct ipath_user_info *uinfo)
{
	struct ipath_portdata *pd;
	int ret;

	if (!(pd = dd->ipath_pd[port])) {
		void *ptmp;

		pd = kzalloc(sizeof(struct ipath_portdata), GFP_KERNEL);

		/*
		 * Allocate memory for use in ipath_tid_update() just once
		 * at open, not per call.  Reduces cost of expected send
		 * setup.
		 */
		ptmp = kmalloc(dd->ipath_rcvtidcnt * sizeof(u16) +
			       dd->ipath_rcvtidcnt * sizeof(struct page **),
			       GFP_KERNEL);
		if (!pd || !ptmp) {
			ipath_dev_err(dd, "Unable to allocate portdata "
				      "memory, failing open\n");
			ret = -ENOMEM;
			kfree(pd);
			kfree(ptmp);
			goto bail;
		}
		dd->ipath_pd[port] = pd;
		dd->ipath_pd[port]->port_port = port;
		dd->ipath_pd[port]->port_dd = dd;
		dd->ipath_pd[port]->port_tid_pg_list = ptmp;
		init_waitqueue_head(&dd->ipath_pd[port]->port_wait);
	}
	if (!pd->port_cnt) {
		pd->userversion = uinfo->spu_userversion;
		init_user_egr_sizes(pd);
		if ((ret = init_subports(dd, pd, uinfo)) != 0)
			goto bail;
		ipath_cdbg(PROC, "%s[%u] opened unit:port %u:%u\n",
			   current->comm, current->pid, dd->ipath_unit,
			   port);
		pd->port_cnt = 1;
		port_fp(fp) = pd;
		pd->port_pid = get_pid(task_pid(current));
		strncpy(pd->port_comm, current->comm, sizeof(pd->port_comm));
		ipath_stats.sps_ports++;
		ret = 0;
	} else
		ret = -EBUSY;

bail:
	return ret;
}

static inline int usable(struct ipath_devdata *dd)
{
	return dd &&
		(dd->ipath_flags & IPATH_PRESENT) &&
		dd->ipath_kregbase &&
		dd->ipath_lid &&
		!(dd->ipath_flags & (IPATH_LINKDOWN | IPATH_DISABLED
				     | IPATH_LINKUNK));
}

static int find_free_port(int unit, struct file *fp,
			  const struct ipath_user_info *uinfo)
{
	struct ipath_devdata *dd = ipath_lookup(unit);
	int ret, i;

	if (!dd) {
		ret = -ENODEV;
		goto bail;
	}

	if (!usable(dd)) {
		ret = -ENETDOWN;
		goto bail;
	}

	for (i = 1; i < dd->ipath_cfgports; i++) {
		ret = try_alloc_port(dd, i, fp, uinfo);
		if (ret != -EBUSY)
			goto bail;
	}
	ret = -EBUSY;

bail:
	return ret;
}

static int find_best_unit(struct file *fp,
			  const struct ipath_user_info *uinfo)
{
	int ret = 0, i, prefunit = -1, devmax;
	int maxofallports, npresentimer(unsigned long data)
{
	struct sock *sk=(struct sock *)data;

	if (!sk_has_allocations(sk)) {
		sk_free(sk);
		return;
	}

	sk->sk_timer.expires = jiffies + 10 * HZ;
	add_timer(&sk->sk_timer);
	printk(KERN_DEBUG "econet socket destroy delayed\n");
}

/*
 *	Close an econet socket.
 */

static int econet_release(struct socket *sock)
{
	struct sock *sk;

	mutex_lock(&econet_mutex);

	sk = sock->sk;
	if (!sk)
		goto out_unlock;

	econet_remove_socket(&econet_sklist, sk);

	/*
	 *	Now the socket is dead. No more input will appear.
	 */

	sk->sk_state_change(sk);	/* It is useless. Just for sanity. */

	sock_orphan(sk);

	/* Purge queues */

	skb_queue_purge(&sk->sk_receive_queue);

	if (sk_has_allocations(sk)) {
		sk->sk_timer.data     = (unsigned long)sk;
		sk->sk_timer.expires  = jiffies + HZ;
		sk->sk_timer.function = econet_destroy_timer;
		add_timer(&sk->sk_timer);

		goto out_unlock;
	}

	sk_free(sk);

out_unlock:
	mutex_unlock(&econet_mutex);
	return 0;
}

static struct proto econet_proto = {
	.name	  = "ECONET",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct econet_sock),
};

/*
 *	Create an Econet socket
 */

static int econet_create(struct net *net, struct socket *sock, int protocol)
{
	struct sock *sk;
	struct econet_sock *eo;
	int err;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	/* Econet only provides datagram services. */
	if (sock->type != SOCK_DGRAM)
		return -ESOCKTNOSUPPORT;

	sock->state = SS_UNCONNECTED;

	err = -ENOBUFS;
	sk = sk_alloc(net, PF_ECONET, GFP_KERNEL, &econet_proto);
	if (sk == NULL)
		goto out;

	sk->sk_reuse = 1;
	sock->ops = &econet_ops;
	sock_init_data(sock, sk);

	eo = ec_sk(sk);
	sock_reset_flag(sk, SOCK_ZAPPED);
	sk->sk_family = PF_ECONET;
	eo->num = protocol;

	econet_insert_socket(&econet_sklist, sk);
	return(0);
out:
	return err;
}

/*
 *	Handle Econet specific ioctls
 */

static int ec_dev_ioctl(struct socket *sock, unsigned int cmd, void __user *arg)
{
	struct ifreq ifr;
	struct ec_device *edev;
	struct net_device *dev;
	struct sockaddr_ec *sec;
	int err;

	/*
	 *	Fetch the caller's info block into kernel space
	 */

	if (copy_from_user(&ifr, arg, sizeof(struct ifreq)))
		return -EFAULT;

	if ((dev = dev_get_by_name(&init_net, ifr.ifr_name)) == NULL)
		return -ENODEV;

	sec = (struct sockaddr_ec *)&ifr.ifr_addr;

	mutex_lock(&econet_mutex);

	err = 0;
	switch (cmd) {
	case SIOCSIFADDR:
		edev = dev->ec_ptr;
		if (edev == NULL) {
			/* Magic up a new one. */
			edev = kzalloc(sizeof(struct ec_device), GFP_KERNEL);
			if (edev == NULL) {
				err = -ENOMEM;
				break;
			}
			dev->ec_ptr = edev;
		} else
			net2dev_map[edev->net] = NULL;
		edev->station = sec->addr.station;
		edev->net = sec->addr.net;
		net2dev_map[sec->addr.net] = dev;
		if (!net2dev_map[0])
			net2dev_map[0] = dev;
		break;

	case SIOCGIFADDR:
		edev = dev->ec_ptr;
		if (edev == NULL) {
			err = -ENODEV;
			break;
		}
		memset(sec, 0, sizeof(struct sockaddr_ec));
		sec->addr.station = edev->station;
		sec->addr.net = edev->net;
		sec->sec_family = AF_ECONET;
		dev_put(dev);
		if (copy_to_user(arg, &ifr, sizeof(struct ifreq)))
			err = -EFAULT;
		break;

	default:
		err = -EINVAL;
		break;
	}

	mutex_unlock(&econet_mutex);

	dev_put(dev);

	return err;
}

/*
 *	Handle generic ioctls
 */

static int econet_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;
	void __user *argp = (void __user *)arg;

	switch(cmd) {
		case SIOCGSTAMP:
			return sock_get_timestamp(sk, argp);

		case SIOCGSTAMPNS:
			return sock_get_timestampns(sk, argp);

		case SIOCSIFADDR:
		case SIOCGIFADDR:
			return ec_dev_ioctl(sock, cmd, argp);
			break;

		default:
			return -ENOIOCTLCMD;
	}
	/*NOTREACHED*/
	return 0;
}

static struct net_proto_family econet_family_ops = {
	.family =	PF_ECONET,
	.create =	econet_create,
	.owner	=	THIS_MODULE,
};

static const struct proto_ops econet_ops = {
	.family =	PF_ECONET,
	.owner =	THIS_MODULE,
	.release =	econet_release,
	.bind =		econet_bind,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	eco_pid),
				   dd->ipath_unit, pd->port_port);
			ret = 1;
			goto done;
		}
	}

done:
	return ret;
}

static int ipath_open(struct inode *in, struct file *fp)
{
	/* The real work is performed later in ipath_assign_port() */
	cycle_kernel_lock();
	fp->private_data = kzalloc(sizeof(struct ipath_filedata), GFP_KERNEL);
	return fp->private_data ? 0 : -ENOMEM;
}

/* Get port early, so can set affinity prior to memory allocation */
static int ipath_assign_port(struct file *fp,
			      const struct ipath_user_info *uinfo)
{
	int ret;
	int i_minor;
	unsigned swmajor, swminor;

	/* Check to be sure we haven't already initialized this file */
	if (port_fp(fp)) {
		ret = -EINVAL;
		goto done;
	}

	/* for now, if major version is different, bail */
	swmajor = uinfo->spu_userversion >> 16;
	if (swmajor != IPATH_USER_SWMAJOR) {
		ipath_dbg("User major version %d not same as driver "
			  "major %d\n", uinfo->spu_userversion >> 16,
			  IPATH_USER_SWMAJOR);
		ret = -ENODEV;
		goto done;
	}

	swminor = uinfo->spu_userversion & 0xffff;
	if (swminor != IPATH_USER_SWMINOR)
		ipath_dbg("User minor version %d not same as driver "
			  "minor %d\n", swminor, IPATH_USER_SWMINOR);

	mutex_lock(&ipath_mutex);

	if (ipath_compatible_subports(swmajor, swminor) &&
	    uinfo->spu_subport_cnt &&
	    (ret = find_shared_port(fp, uinfo))) {
		if (ret > 0)
			ret = 0;
		goto done_chk_sdma;
	}

	i_minor = iminor(fp->f_path.dentry->d_inode) - IPATH_USER_MINOR_BASE;
	ipath_cdbg(VERBOSE, "open on dev %lx (minor %d)\n",
		   (long)fp->f_path.dentry->d_inode->i_rdev, i_minor);

	if (i_minor)
		ret = find_free_port(i_minor - 1, fp, uinfo);
	else
		ret = find_best_unit(fp, uinfo);

done_chk_sdma:
	if (!ret) {
		struct ipath_filedata *fd = fp->private_data;
		const struct ipath_portdata *pd = fd->pd;
		const struct ipath_devdata *dd = pd->port_dd;

		fd->pq = ipath_user_sdma_queue_create(&dd->pcidev->dev,
						      dd->ipath_unit,
						      pd->port_port,
						      fd->subport);

		if (!fd->pq)
			ret = -ENOMEM;
	}

	mutex_unlock(&ipath_mutex);

done:
	return ret;
}


static int ipath_do_user_init(struct file *fp,
			      const struct ipath_user_info *uinfo)
{
	int ret;
	struct ipath_portdata *pd = port_fp(fp);
	struct ipath_devdata *dd;
	u32 head32;

	/* Subports don't need to initialize anything since master did it. */
	if (subport_fp(fp)) {
		ret = wait_event_interruptible(pd->port_wait,
			!test_bit(IPATH_PORT_MASTER_UNINIT, &pd->port_flag));
		goto done;
	}

	dd = pd->port_dd;

	if (uinfo->spu_rcvhdrsize) {
		ret = ipath_setrcvhdrsize(dd, uinfo->spu_rcvhdrsize);
		if (ret)
			goto done;
	}

	/* for now we do nothing with rcvhdrcnt: uinfo->spu_rcvhdrcnt */

	/* some ports may get extra buffers, calculate that here */
	if (pd->port_port <= dd->ipath_ports_extrabuf)
		pd->port_piocnt = dd->ipath_pbufsport + 1;
	else
		pd->port_piocnt = dd->ipath_pbufsport;

	/* for right now, kernel piobufs are at end, so port 1 is at 0 */
	if (pd->port_port <= dd->ipath_ports_extrabuf)
		pd->port_pio_base = (dd->ipath_pbufsport + 1)
			* (pd->port_port - 1);
	else
		pd->port_pio_base = dd->ipath_ports_extrabuf +
			dd->ipath_pbufsport * (pd->port_port - 1);
	pd->port_piobufs = dd->ipath_piobufbase +
		pd->port_pio_base * dd->ipath_palign;
	ipath_cdbg(VERBOSE, "piobuf base for port %u is 0x%x, piocnt %u,"
		" first pio %u\n", pd->port_port, pd->port_piobufs,
		pd->port_piocnt, pd->port_pio_base);
	ipath_chg_pioavailkernel(dd, pd->port_pio_base, pd->port_piocnt, 0);

	/*
	 * Now allocate the rcvhdr Q and eager TIDs; skip the TID
	 * array for time being.  If pd->port_port > chip-supported,
	 * we need to do extra stuff here to handle by handling overflow
	 * through port 0, someday
	 */
	ret = ipath_create_rcvhdrq(dd, pd);
	if (!ret)
		ret = ipath_create_user_egr(pd);
	if (ret)
		goto done;

	/*
	 * set the eager head register for this port to the current values
	 * of the tail pointers, since we don't know if they were
	 * updated on last use of the port.
	 */
	head32 = ipath_read_ureg32(dd, ur_rcvegrindextail, pd->port_port);
	ipath_write_ureg(dd, ur_rcvegrindexhead, head32, pd->port_port);
	pd->port_lastrcvhdrqtail = -1;
	ipath_cdbg(VERBOSE, "Wrote port%d egrhead %x from tail regs\n",
		pd->port_port, head32);
	pd->port_tidcursor = 0;	/* start at beginning after open */

	/* initialize poll variables... */
	pd->port_urgent = 0;
	pd->port_urgent_poll = 0;
	pd->port_hdrqfull_poll = pd->port_hdrqfull;

	/*
	 * Now enable the port for receive.
	 * For chips that are set to DMA the tail register to memory
	 * when they change (and when the update bit transitions from
	 * 0 to 1.  So for those chips, we turn it off and then back on.
	 * This will (very briefly) affect any other open ports, but the
	 * duration is very short, and therefore isn't an issue.  We
	 * explictly set the in-memory tail copy to 0 beforehand, so we
	 * don't have to wait to be sure the DMA update has happened
	 * (chip resets head/tail to 0 on transition to enable).
	 */
	set_bit(dd->ipath_r_portenable_shift + pd->port_port,
		&dd->ipath_rcvctrl);
	if (!(dd->ipath_flags & IPATH_NODMA_RTAIL)) {
		if (pd->port_rcvhdrtail_kvaddr)
			ipath_clear_rcvhdrtail(pd);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvctrl,
			dd->ipath_rcvctrl &
			~(1ULL << dd->ipath_r_tailupd_shift));
	}
	ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvctrl,
			 dd->ipath_rcvctrl);
	/* Notify any waiting slaves */
	if (pd->port_subport_cnt) {
		clear_bit(IPATH_PORT_MASTER_UNINIT, &pd->port_flag);
		wake_up(&pd->port_wait);
	}
done:
	return ret;
}

/**
 * unlock_exptid - unlock any expected TID entries port still had in use
 * @pd: port
 *
 * We don't actually update the chip here, because we do a bulk update
 * below, using ipath_f_clear_tids.
 */
static void unlock_expected_tids(struct ipath_portdata *pd)
{
	struct ipath_devdata *dd = pd->port_dd;
	int port_tidbase = pd->port_port * dd->ipath_rcvtidcnt;
	int i, cnt = 0, maxtid = port_tidbase + dd->ipath_rcvtidcnt;

	ipath_cdbg(VERBOSE, "Port %u unlocking any locked expTID pages\n",
		   pd->port_port);
	for (i = port_tidbase; i < maxtid; i++) {
		struct page *ps = dd->ipath_pageshadow[i];

		if (!ps)
			continue;

		dd->ipath_pageshadow[i] = NULL;
		pci_unmap_page(dd->pcidev, dd->ipath_physshadow[i],
			PAGE_SIZE, PCI_DMA_FROMDEVICE);
		ipath_release_user_pages_on_close(&ps, 1);
		cnt++;
		ipath_stats.sps_pageunlocks++;
	}
	if (cnt)
		ipath_cdbg(VERBOSE, "Port %u locked %u expTID entries\n",
			   pd->port_port, cnt);

	if (ipath_stats.sps_pagelocks || ipath_stats.sps_pageunlocks)
		ipath_cdbg(VERBOSE, "%llu pages locked, %llu unlocked\n",
			   (unsigned long long) ipath_stats.sps_pagelocks,
			   (unsigned long long)
			   ipath_stats.sps_pageunlocks);
}

static int ipath_close(struct inode *in, struct file *fp)
{
	int ret = 0;
	struct ipath_filedata *fd;
	struct ipath_portdata *pd;
	struct ipath_devdata *dd;
	unsigned long flags;
	unsigned port;
	struct pid *pid;

	ipath_cdbg(VERBOSE, "close on dev %lx, private data %p\n",
		   (long)in->i_rdev, fp->private_data);

	mutex_lock(&ipath_mutex);

	fd = (struct ipath_filedata *) fp->private_data;
	fp->private_data = NULL;
	pd = fd->pd;
	if (!pd) {
		mutex_unlock(&ipath_mutex);
		goto bail;
	}

	dd = pd->port_dd;

	/* drain user sdma queue */
	ipath_user_sdma_queue_drain(dd, fd->pq);
	ipath_user_sdma_queue_destroy(fd->pq);

	if (--pd->port_cnt) {
		/*
		 * XXX If the master closes the port before the slave(s),
		 * revoke the mmap for the eager receive queue so
		 * the slave(s) don't wait for receive data forever.
		 */
		pd->active_slaves &= ~(1 << fd->subport);
		put_pid(pd->port_subpid[fd->subport]);
		pd->port_subpid[fd->subport] = NULL;
		mutex_unlock(&ipath_mutex);
		goto bail;
	}
	/* early; no interrupt users after this */
	spin_lock_irqsave(&dd->ipath_uctxt_lock, flags);
	port = pd->port_port;
	dd->ipath_pd[port] = NULL;
	pid = pd->port_pid;
	pd->port_pid = NULL;
	spin_unlock_irqrestore(&dd->ipath_uctxt_lock, flags);

	if (pd->port_rcvwait_to || pd->port_piowait_to
	    || pd->port_rcvnowait || pd->port_pionowait) {
		ipath_cdbg(VERBOSE, "port%u, %u rcv, %u pio wait timeo; "
			   "%u rcv %u, pio already\n",
			   pd->port_port, pd->port_rcvwait_to,
			   pd->port_piowait_to, pd->port_rcvnowait,
			   pd->port_pionowait);
		pd->port_rcvwait_to = pd->port_piowait_to =
			pd->port_rcvnowait = pd->port_pionowait = 0;
	}
	if (pd->port_flag) {
		ipath_cdbg(PROC, "port %u port_flag set: 0x%lx\n",
			  pd->port_port, pd->port_flag);
		pd->port_flag = 0;
	}

	if (dd->ipath_kregbase) {
		/* atomically clear receive enable port and intr avail. */
		clear_bit(dd->ipath_r_portenable_shift + port,
			  &dd->ipath_rcvctrl);
		clear_bit(pd->port_port + dd->ipath_r_intravail_shift,
			  &dd->ipath_rcvctrl);
		ipath_write_kreg( dd, dd->ipath_kregs->kr_rcvctrl,
			dd->ipath_rcvctrl);
		/* and read back from chip to be sure that nothing
		 * else is in flight when we do the rest */
		(void)ipath_read_kreg64(dd, dd->ipath_kregs->kr_scratch);

		/* clean up the pkeys for this port user */
		ipath_clean_part_key(pd, dd);
		/*
		 * be paranoid, and never write 0's to these, just use an
		 * unused part of the port 0 tail page.  Of course,
		 * rcvhdraddr points to a large chunk of memory, so this
		 * could still trash things, but at least it won't trash
		 * page 0, and by disabling the port, it should stop "soon",
		 * even if a packet or two is in already in flight after we
		 * disabled the port.
		 */
		ipath_write_kreg_port(dd,
		        d