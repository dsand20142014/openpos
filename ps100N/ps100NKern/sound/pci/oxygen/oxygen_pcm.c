o lock addr %p, %u pages: "
				 "errno %d\n", (void *) vaddr, cnt, -ret);
			goto done;
		}
	}
	for (i = 0; i < cnt; i++, vaddr += PAGE_SIZE) {
		for (; ntids--; tid++) {
			if (tid == tidcnt)
				tid = 0;
			if (!dd->ipath_pageshadow[porttid + tid])
				break;
		}
		if (ntids < 0) {
			/*
			 * oops, wrapped all the way through their TIDs,
			 * and didn't have enough free; see comments at
			 * start of routine
			 */
			ipath_dbg("Not enough free TIDs for %u pages "
				  "(index %d), failing\n", cnt, i);
			i--;	/* last tidlist[i] not filled in */
			ret = -ENOMEM;
			break;
		}
		tidlist[i] = tid + tidoff;
		ipath_cdbg(VERBOSE, "Updating idx %u to TID %u, "
			   "vaddr %lx\n", i, tid + tidoff, vaddr);
		/* we "know" system pages and TID pages are same size */
		dd->ipath_pageshadow[porttid + tid] = pagep[i];
		dd->ipath_physshadow[porttid + tid] = ipath_map_page(
			dd->pcidev, pagep[i], 0, PAGE_SIZE,
			PCI_DMA_FROMDEVICE);
		/*
		 * don't need atomic or it's overhead
		 */
		__set_bit(tid, tidmap);
		physaddr = dd->ipath_physshadow[porttid + tid];
		ipath_stats.sps_pagelocks++;
		ipath_cdbg(VERBOSE,
			   "TID %u, vaddr %lx, physaddr %llx pgp %p\n",
			   tid, vaddr, (unsigned long long) physaddr,
			   pagep[i]);
		dd->ipath_f_put_tid(dd, &tidbase[tid], RCVHQ_RCV_TYPE_EXPECTED,
				    physaddr);
		/*
		 * don't check this tid in ipath_portshadow, since we
		 * just filled it in; start with the next one.
		 */
		tid++;
	}

	if (ret) {
		u32 limit;
	cleanup:
		/* jump here if copy out of updated info failed... */
		ipath_dbg("After failure (ret=%d), undo %d of %d entries\n",
			  -ret, i, cnt);
		/* same code that's in ipath_free_tid() */
		limit = sizeof(tidmap) * BITS_PER_BYTE;
		if (limit > tidcnt)
			/* just in case size changes in future */
			limit = tidcnt;
		tid = find_first_bit((const unsigned long *)tidmap, limit);
		for (; tid < limit; tid++) {
			if (!test_bit(tid, tidmap))
				continue;
			if (dd->ipath_pageshadow[porttid + tid]) {
				ipath_cdbg(VERBOSE, "Freeing TID %u\n",
					   tid);
				dd->ipath_f_put_tid(dd, &tidbase[tid],
						    RCVHQ_RCV_TYPE_EXPECTED,
						    dd->ipath_tidinvalid);
				pci_unmap_page(dd->pcidev,
					dd->ipath_physshadow[porttid + tid],
					PAGE_SIZE, PCI_DMA_FROMDEVICE);
				dd->ipath_pageshadow[porttid + tid] = NULL;
				ipath_stats.sps_pageunlocks++;
			}
		}
		ipath_release_user_pages(pagep, cnt);
	} else {
		/*
		 * Copy the updated array, with ipath_tid's filled in, back
		 * to user.  Since we did the copy in already, this "should
		 * never fail" If it does, we have to clean up...
		 */
		if (copy_to_user((void __user *)
				 (unsigned long) ti->tidlist,
				 tidlist, cnt * sizeof(*tidlist))) {
			ret = -EFAULT;
			goto cleanup;
		}
		if (copy_to_user((void __user *) (unsigned long) ti->tidmap,
				 tidmap, sizeof tidmap)) {
			ret = -EFAULT;
			goto cleanup;
		}
		if (tid == tidcnt)
			tid = 0;
		if (!pd->port_subport_cnt)
			pd->port_tidcursor = tid;
		else
			tidcursor_fp(fp) = tid;
	}

done:
	if (ret)
		ipath_dbg("Failed to map %u TID pages, failing with %d\n",
			  ti->tidcnt, -ret);
	return ret;
}

/**
 * ipath_tid_free - free a port TID
 * @pd: the port
 * @subport: the subport
 * @ti: the TID info
 *
 * right now we are unlocking one page at a time, but since
 * the intended use of this routine is for a single group of
 * virtually contiguous pages, that should change to improve
 * performance.  We check that the TID is in range for this port
 * but otherwise don't check validity; if user has an error and
 * frees the wrong tid, it's only their own data that can thereby
 * be corrupted.  We do check that the TID was in use, for sanity
 * We always use our idea of the saved address, not the address that
 * they pass in to us.
 */

static int ipath_tid_free(struct ipath_portdata *pd, unsigned subport,
			  const struct ipath_tid_info *ti)
{
	int ret = 0;
	u32 tid, porttid, cnt, limit, tidcnt;
	struct ipath_devdata *dd = pd->port_dd;
	u64 __iomem *tidbase;
	unsigned long tidmap[8];

	if (!dd->ipath_pageshadow) {
		ret = -ENOMEM;
		goto done;
	}

	if (copy_from_user(tidmap, (void __user *)(unsigned long)ti->tidmap,
			   sizeof tidmap)) {
		ret = -EFAULT;
		goto done;
	}

	porttid = pd->port_port * dd->ipath_rcvtidcnt;
	if (!pd->port_subport_cnt)
		tidcnt = dd->ipath_rcvtidcnt;
	else if (!subport) {
		tidcnt = (dd->ipath_rcvtidcnt / pd->port_subport_cnt) +
			 (dd->ipath_rcvtidcnt % pd->port_subport_cnt);
		porttid += dd->ipath_rcvtidcnt - tidcnt;
	} else {
		tidcnt = dd->ipath_rcvtidcnt / pd->port_subport_cnt;
		porttid += tidcnt * (subport - 1);
	}
	tidbase = (u64 __iomem *) ((char __iomem *)(dd->ipath_kregbase) +
				   dd->ipath_rcvtidbase +
				   porttid * sizeof(*tidbase));

	limit = sizeof(tidmap) * BITS_PER_BYTE;
	if (limit > tidcnt)
		/* just in case size changes in future */
		limit = tidcnt;
	tid = find_first_bit(tidmap, limit);
	ipath_cdbg(VERBOSE, "Port%u free %u tids; first bit (max=%d) "
		   "set is %d, porttid %u\n", pd->port_port, ti->tidcnt,
		   limit, tid, porttid);
	for (cnt = 0; tid < limit; tid++) {
		/*
		 * small optimization; if we detect a run of 3 or so without
		 * any set, use find_first_bit again.  That's mainly to
		 * accelerate the case where we wrapped, so we have some at
		 * the beginning, and some at the end, and a big gap
		 * in the middle.
		 */
		if (!test_bit(tid, tidmap))
			continue;
		cnt++;
		if (dd->ipath_pageshadow[porttid + tid]) {
			struct page *p;
			p = dd->ipath_pageshadow[porttid + tid];
			dd->ipath_pageshadow[porttid + tid] = NULL;
			ipath_cdbg(VERBOSE, "PID %u freeing TID %u\n",
				   pid_nr(pd->port_pid), tid);
			dd->ipath_f_put_tid(dd, &tidbase[tid],
					    RCVHQ_RCV_TYPE_EXPECTED,
					    dd->ipath_tidinvalid);
			pci_unmap_page(dd->pcidev,
				dd->ipath_physshadow[porttid + tid],
				PAGE_SIZE, PCI_DMA_FROMDEVICE);
			ipath_release_user_pages(&p, 1);
			ipath_stats.sps_pageunlocks++;
		} else
			ipath_dbg("Unused tid %u, ignoring\n", tid);
	}
	if (cnt != ti->tidcnt)
		ipath_dbg("passed in tidcnt %d, only %d bits set in map\n",
			  ti->tidcnt, cnt);
done:
	if (ret)
		ipath_dbg("Failed to unmap %u TID pages, failing with %d\n",
			  ti->tidcnt, -ret);
	return ret;
}

/**
 * ipath_set_part_key - set a partition key
 * @pd: the port
 * @key: the key
 *
 * We can have up to 4 active at a time (other than the default, which is
 * always allowed).  This is somewhat tricky, since multiple ports may set
 * the same key, so we reference count them, and clean up at exit.  All 4
 * partition keys are packed into a single infinipath register.  It's an
 * error for a process to set the same pkey multiple times.  We provide no
 * mechanism to de-allocate a pkey at this time, we may eventually need to
 * do that.  I've used the atomic operations, and no locking, and only make
 * a single pass through what's available.  This should be more than
 * adequate for some time. I'll think about spinlocks or the like if and as
 * it's necessary.
 */
static int ipath_set_part_key(struct ipath_portdata *pd, u16 key)
{
	struct ipath_devdata *dd = pd->port_dd;
	int i, any = 0, pidx = -1;
	u16 lkey = key & 0x7FFF;
	int ret;

	if (lkey == (IPATH_DEFAULT_P_KEY & 0x7FFF)) {
		/* nothing to do; this key always valid */
		ret = 0;
		goto bail;
	}

	ipath_cdbg(VERBOSE, "p%u try to set pkey %hx, current keys "
		   "%hx:%x %hx:%x %hx:%x %hx:%x\n",
		   pd->port_port, key, dd->ipath_pkeys[0],
		   atomic_read(&dd->ipath_pkeyrefs[0]), dd->ipath_pkeys[1],
		   atomic_read(&dd->ipath_pkeyrefs[1]), dd->ipath_pkeys[2],
		   atomic_read(&dd->ipath_pkeyrefs[2]), dd->ipath_pkeys[3],
		   atomic_read(&dd->ipath_pkeyrefs[3]));

	if (!lkey) {
		ipath_cdbg(PROC, "p%u tries to set key 0, not allowed\n",
			   pd->port_port);
		ret = -EINVAL;
		goto bail;
	}

	/*
	 * Set the full membership bit, because it has to be
	 * set in the register or the packet, and it seems
	 * cleaner to set in the register than to force all
	 * callers to set it. (see bug 4331)
	 */
	key |= 0x8000;

	for (i = 0; i < ARRAY_SIZE(pd->port_pkeys); i++) {
		if (!pd->port_pkeys[i] && pidx == -1)
			pidx = i;
		if (pd->port_pkeys[i] == key) {
			ipath_cdbg(VERBOSE, "p%u tries to set same pkey "
				   "(%x) more than once\n",
				   pd->port_port, key);
			ret = -EEXIST;
			goto bail;
		}
	}
	if (pidx == -1) {
		ipath_dbg("All pkeys for port %u already in use, "
			  "can't set %x\n", pd->port_port, key);
		ret = -EBUSY;
		goto bail;
	}
	for (any = i = 0; i < ARRAY_SIZE(dd->ipath_pkeys); i++) {
		if (!dd->ipath_pkeys[i]) {
			any++;
			continue;
		}
		if (dd->ipath_pkeys[i] == key) {
			atomic_t *pkrefs = &dd->ipath_pkeyrefs[i];

			if (atomic_inc_return(pkrefs) > 1) {
				pd->port_pkeys[pidx] = key;
				ipath_cdbg(VERBOSE, "p%u set key %x "
					   "matches #%d, count now %d\n",
					   pd->port_port, key, i,
					   atomic_read(pkrefs));
				ret = 0;
				goto bail;
			} else {
				/*
				 * lost race, decrement count, catch below
				 */
				atomic_dec(pkrefs);
				ipath_cdbg(VERBOSE, "Lost race, count was "
					   "0, after dec, it's %d\n",
					   atomic_read(pkrefs));
				any++;
			}
		}
		if ((dd->ipath_pkeys[i] & 0x7FFF) == lkey) {
			/*
			 * It makes no sense to have both the limited and
			 * full membership PKEY set at the same time since
			 * the unlimited one will disable the limited one.
			 */
			ret = -EEXIST;
			goto bail;
		}
	}
	if (!any) {
		ipath_dbg("port %u, all pkeys already in use, "
			  "can't set %x\n", pd->port_port, key);
		ret = -EBUSY;
		goto bail;
	}
	for (any = i = 0; i < ARRAY_SIZE(dd->ipath_pkeys); i++) {
		if (!dd->ipath_pkeys[i] &&
		    atomic_inc_return(&dd->ipath_pkeyrefs[i]) == 1) {
			u64 pkey;

			/* for ipathstats, etc. */
			ipath_stats.sps_pkeys[i] = lkey;
			pd->port_pkeys[pidx] = dd->ipath_pkeys[i] = key;
			pkey =
				(u64) dd->ipath_pkeys[0] |
				((u64) dd->ipath_pkeys[1] << 16) |
				((u64) dd->ipath_pkeys[2] << 32) |
				((u64) dd->ipath_pkeys[3] << 48);
			ipath_cdbg(PROC, "p%u set key %x in #%d, "
				   "portidx %d, new pkey reg %llx\n",
				   pd->port_port, key, i, pidx,
				   (unsigned long long) pkey);
			ipath_write_kreg(
				dd, dd->ipath_kregs->kr_partitionkey, pkey);

			ret = 0;
			goto bail;
		}
	}
	ipath_dbg("port %u, all pkeys already in use 2nd pass, "
		  "can't set %x\n", pd->port_port, key);
	ret = -EBUSY;

bail:
	return ret;
}

/**
 * ipath_manage_rcvq - manage a port's receive queue
 * @pd: the port
 * @subport: the subport
 * @start_stop: action to carry out
 *
 * start_stop == 0 disables receive on the port, for use in queue
 * overflow conditions.  start_stop==1 re-enables, to be used to
 * re-init the software copy of the head register
 */
static int ipath_manage_rcvq(struct ipath_portdata *pd, unsigned subport,
			     int start_stop)
{
	struct ipath_devdata *dd = pd->port_dd;

	ipath_cdbg(PROC, "%sabling rcv for unit %u port %u:%u\n",
		   start_stop ? "en" : "dis", dd->ipath_unit,
		   pd->port_port, subport);
	if (subport)
		goto bail;
	/* atomically clear receive enable port. */
	if (start_stop) {
		/*
		 * On enable, force in-memory copy of the tail register to
		 * 0, so that protocol code doesn't have to worry about
		 * whether or not the chip has yet updated the in-memory
		 * copy or not on return from the system call. The chip
		 * always resets it's tail register back to 0 on a
		 * transition from disabled to enabled.  This could cause a
		 * problem if software was broken, and did the enable w/o
		 * the disable, but eventually the in-memory copy will be
		 * updated and correct itself, even in the face of software
		 * bugs.
		 */
		if (pd->port_rcvhdrtail_kvaddr)
			ipath_clear_rcvhdrtail(pd);
		set_bit(dd->ipath_r_portenable_shift + pd->port_port,
			&dd->ipath_rcvctrl);
	} else
		clear_bit(dd->ipath_r_portenable_shift + pd->port_port,
			  &dd->ipath_rcvctrl);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvctrl,
			 dd->ipath_rcvctrl);
	/* now be sure chip saw it before we return */
	ipath_read_kreg64(dd, dd->ipath_kregs->kr_scratch);
	if (start_stop) {
		/*
		 * And try to be sure that tail reg update has happened too.
		 * This should in theory interlock with the RXE changes to
		 * the tail register.  Don't assign it to the tail register
		 * in memory copy, since we could overwrite an update by the
		 * chip if we did.
		 */
		ipath_read_ureg32(dd, ur_rcvhdrtail, pd->port_port);
	}
	/* always; new head should be equal to new tail; see above */
bail:
	return 0;
}

static void ipath_clean_part_key(struct ipath_portdata *pd,
				 struct ipath_devdata *dd)
{
	int i, j, pchanged = 0;
	u64 oldpkey;

	/* for debugging only */
	oldpkey = (u64) dd->ipath_pkeys[0] |
		((u64) dd->ipath_pkeys[1] << 16) |
		((u64) dd->ipath_pkeys[2] << 32) |
		((u64) dd->ipath_pkeys[3] << 48);

	for (i = 0; i < ARRAY_SIZE(pd->port_pkeys); i++) {
		if (!pd->port_pkeys[i])
			continue;
		ipath_cdbg(VERBOSE, "look for key[%d] %hx in pkeys\n", i,
			   pd->port_pkeys[i]);
		for (j = 0; j < ARRAY_SIZE(dd->ipath_pkeys); j++) {
			/* check for match independent of the global bit */
			if ((dd->ipath_pkeys[j] & 0x7fff) !=
			    (pd->port_pkeys[i] & 0x7fff))
				continue;
			if (atomic_dec_and_test(&dd->ipath_pkeyrefs[j])) {
				ipath_cdbg(VERBOSE, "p%u clear key "
					   "%x matches #%d\n",
					   pd->port_port,
					   pd->port_pkeys[i], j);
				ipath_stats.sps_pkeys[j] =
					dd->ipath_pkeys[j] = 0;
				pchanged++;
			}
			else ipath_cdbg(
				VERBOSE, "p%u key %x matches #%d, "
				"but ref still %d\n", pd->port_port,
				pd->port_pkeys[i], j,
				atomic_read(&dd->ipath_pkeyrefs[j]));
			break;
		}
		pd->port_pkeys[i] = 0;
	}
	if (pchanged) {
		u64 pkey = (u64) dd->ipath_pkeys[0] |
			((u64) dd->ipath_pkeys[1] << 16) |
			((u64) dd->ipath_pkeys[2] << 32) |
			((u64) dd->ipath_pkeys[3] << 48);
		ipath_cdbg(VERBOSE, "p%u old pkey reg %llx, "
			   "new pkey reg %llx\n", pd->port_port,
			   (unsigned long long) oldpkey,
			   (unsigned long long) pkey);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_partitionkey,
				 pkey);
	}
}

/*
 * Initialize the port data with the receive buffer sizes
 * so this can be done while the master port is locked.
 * Otherwise, there is a race with a slave opening the port
 * and seeing these fields uninitialized.
 */
static void init_user_egr_sizes(struct ipath_portdata *pd)
{
	struct ipath_devdata *dd = pd->port_dd;
	unsigned egrperchunk, egrcnt, size;

	/*
	 * to avoid wasting a lot of memory, we allocate 32KB chunks of
	 * physically contiguous memory, advance through it until used up
	 * and then allocate more.  Of course, we need memory to store those
	 * extra pointers, now.  Started out with 256KB, but under heavy
	 * memory pressure (creating large files and then copying them over
	 * NFS while doing lots of MPI jobs), we hit some allocation
	 * failures, even though we can sleep...  (2.6.10) Still get
	 * failures at 64K.  32K is the lowest we can go without wasting
	 * additional memory.
	 */
	size = 0x8000;
	egrperchunk = size / dd->ipath_rcvegrbufsize;
	egrcnt = dd->ipath_rcvegrcnt;
	pd->port_rcvegrbuf_chunks = (egrcnt + egrperchunk - 1) / egrperchunk;
	pd->port_rcvegrbufs_perchunk = egrperchunk;
	pd->port_rcvegrbuf_size = size;
}

/**
 * ipath_create_user_egr - allocate eager TID buffers
 * @pd: the port to allocate TID buffers for
 *
 * This routine is now quite different for user and kernel, because
 * the kernel uses skb's, for the accelerated network performance
 * This is the user port version
 *
 * Allocate the eager TID buffers and program them into infinipath
 * They are no longer completely contiguous, we do multiple allocation
 * calls.
 */
static int ipath_create_user_egr(struct ipath_portdata *pd)
{
	struct ipath_devdata *dd = pd->port_dd;
	unsigned e, egrcnt, egrperchunk, chunk, egrsize, egroff;
	size_t size;
	int ret;
	gfp_t gfp_flags;

	/*
	 * GFP_USER, but without GFP_FS, so buffer cache can be
	 * coalesced (we hope); otherwise, even at order 4,
	 * heavy filesystem activity makes these fail, and we can
	 * use compound pages.
	 */
	gfp_flags = __GFP_WAIT | __GFP_IO | __GFP_COMP;

	egrcnt = dd->ipath_rcvegrcnt;
	/* TID number offset for this port */
	egroff = (pd->port_port - 1) * egrcnt + dd->ipath_p0_rcvegrcnt;
	egrsize = dd->ipath_rcvegrbufsize;
	ipath_cdbg(VERBOSE, "Allocating %d egr buffers, at egrtid "
		   "offset %x, egrsize %u\n", egrcnt, egroff, egrsize);

	chunk = pd->port_rcvegrbuf_chunks;
	egrperchunk = pd->port_rcvegrbufs_perchunk;
	size = pd->port_rcvegrbuf_size;
	pd->port_rcvegrbuf = kmalloc(chunk * sizeof(pd->port_rcvegrbuf[0]),
				     GFP_KERNEL);
	if (!pd->port_rcvegrbuf) {
		ret = -ENOMEM;
		goto bail;
	}
	pd->port_rcvegrbuf_phys =
		kmalloc(chunk * sizeof(pd->port_rcvegrbuf_phys[0]),
			GFP_KERNEL);
	if (!pd->port_rcvegrbuf_phys) {
		ret = -ENOMEM;
		goto bail_rcvegrbuf;
	}
	for (e = 0; e < pd->port_rcvegrbuf_chunks; e++) {

		pd->port_rcvegrbuf[e] = dma_alloc_coherent(
			&dd->pcidev->dev, size, &pd->port_rcvegrbuf_phys[e],
			gfp_flags);

		if (!pd->port_rcvegrbuf[e]) {
			ret = -ENOMEM;
			goto bail_rcvegrbuf_phys;
		}
	}

	pd->port_rcvegr_phys = pd->port_rcvegrbuf_phys[0];

	for (e = chunk = 0; chunk < pd->port_rcvegrbuf_chunks; chunk++) {
		dma_addr_t pa = pd->port_rcvegrbuf_phys[chunk];
		unsigned i;

		for (i = 0; e < egrcnt && i < egrperchunk; e++, i++) {
			dd->ipath_f_put_tid(dd, e + egroff +
					    (u64 __iomem *)
					    ((char __iomem *)
					     dd->ipath_kregbase +
					     dd->ipath_rcvegrbase),
					    RCVHQ_RCV_TYPE_EAGER, pa);
			pa += egrsize;
		}
		cond_resched();	/* don't hog the cpu */
	}

	ret = 0;
	goto bail;

bail_rcvegrbuf_phys:
	for (e = 0; e < pd->port_rcvegrbuf_chunks &&
		pd->port_rcvegrbuf[e]; e++) {
		dma_free_coherent(&dd->pcidev->dev, size,
				  pd->port_rcvegrbuf[e],
				  pd->port_rcvegrbuf_phys[e]);

	}
	kfree(pd->port_rcvegrbuf_phys);
	pd->port_rcvegrbuf_phys = NULL;
bail_rcvegrbuf:
	kfree(pd->port_rcvegrbuf);
	pd->port_rcvegrbuf = NULL;
bail:
	return ret;
}


/* common code for the mappings on dma_alloc_coherent mem */
static int ipath_mmap_mem(struct vm_area_struct *vma,
	struct ipath_portdata *pd, unsigned len, int write_ok,
	void *kvaddr, char *what)
{
	struct ipath_devdata *dd = pd->port_dd;
	unsigned long pfn;
	int ret;

	if ((vma->vm_end - vma->vm_start) > len) {
		dev_info(&dd->pcidev->dev,
		         "FAIL on %s: len %lx > %x\n", what,
			 vma->vm_end - vma->vm_start, len);
		ret = -EFAULT;
		goto bail;
	}

	if (!write_ok) {
		if (vma->vm_flags & VM_WRITE) {
			dev_info(&dd->pcidev->dev,
				 "%s must be mapped readonly\n", what);
			ret = -EPERM;
			goto bail;
		}

		/* don't allow them to later change with mprotect */
		vma->vm_flags &= ~VM_MAYWRITE;
	}

	pfn = virt_to_phys(kvaddr) >> PAGE_SHIFT;
	ret = remap_pfn_range(vma, vma->vm_start, pfn,
			      len, vma->vm_page_prot);
	if (ret)
		dev_info(&dd->pcidev->dev, "%s port%u mmap of %lx, %x "
			 "bytes r%c failed: %d\n", what, pd->port_port,
			 pfn, len, write_ok?'w':'o', ret);
	else
		ipath_cdbg(VERBOSE, "%s port%u mmaped %lx, %x bytes "
			   "r%c\n", what, pd->port_port, pfn, len,
			   write_ok?'w':'o');
bail:
	return ret;
}

static int mmap_ureg(struct vm_area_struct *vma, struct ipath_devdata *dd,
		     u64 ureg)
{
	unsigned long phys;
	int ret;

	/*
	 * This is real hardware, so use io_remap.  This is the mechanism
	 * for the user process to update the head registers for their port
	 * in the chip.
	 */
	if ((vma->vm_end - vma->vm_start) > PAGE_SIZE) {
		dev_info(&dd->pcidev->dev, "FAIL mmap userreg: reqlen "
			 "%lx > PAGE\n", vma->vm_end - vma->vm_start);
		ret = -EFAULT;
	} else {
		phys = dd->ipath_physaddr + ureg;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

		vma->vm_flags |= VM_DONTCOPY | VM_DONTEXPAND;
		ret = io_remap_pfn_range(vma, vma->vm_start,
					 phys >> PAGE_SHIFT,
					 vma->vm_end - vma->vm_start,
					 vma->vm_page_prot);
	}
	return ret;
}

static int mmap_piobufs(struct vm_area_struct *vma,
			struct ipath_devdata *dd,
			struct ipath_portdata *pd,
			unsigned piobufs, unsigned piocnt)
{
	unsigned long phys;
	int ret;

	/*
	 * When we map the PIO buffers in the chip, we want to map them as
	 * writeonly, no read possible.   This prevents access to previous
	 * process data, and catches users who might try to read the i/o
	 * space due to a bug.
	 */
	if ((vma->vm_end - vma->vm_start) > (piocnt * dd->ipath_palign)) {
		dev_info(&dd->pcidev->dev, "FAIL mmap piobufs: "
			 "reqlen %lx > PAGE\n",
			 vma->vm_end - vma->vm_start);
		ret = -EINVAL;
		goto bail;
	}

	phys = dd->ipath_physaddr + piobufs;

#if defined(__powerpc__)
	/* There isn't a generic way to specify writethrough mappings */
	pgprot_val(vma->vm_page_prot) |= _PAGE_NO_CACHE;
	pgprot_val(vma->vm_page_prot) |= _PAGE_WRITETHRU;
	pgprot_val(vma->vm_page_prot) &= ~_PAGE_GUARDED;
#endif

	/*
	 * don't allow them to later change to readable with mprotect (for when
	 * not initially mapped readable, as is normally the case)
	 */
	vma->vm_flags &= ~VM_MAYREAD;
	vma->vm_flags |= VM_DONTCOPY | VM_DONTEXPAND;

	ret = io_remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT,
				 vma->vm_end - vma->vm_start,
				 vma->vm_page_prot);
bail:
	return ret;
}

static int mmap_rcvegrbufs(struct vm_area_struct *vma,
			   struct ipath_portdata *pd)
{
	struct ipath_devdata *dd = pd->port_dd;
	unsigned long start, size;
	size_t total_size, i;
	unsigned long pfn;
	int ret;

	size = pd->port_rcvegrbuf_size;
	total_size = pd->port_rcvegrbuf_chunks * size;
	if ((vma->vm_end - vma->vm_start) > total_size) {
		dev_info(&dd->pcidev->dev, "FAIL on egr bufs: "
			 "reqlen %lx > actual %lx\n",
			 vma->vm_end - vma->vm_start,
			 (unsigned long) total_size);
		ret = -EINVAL;
		goto bail;
	}

	if (vma->vm_flags & VM_WRITE) {
		dev_info(&dd->pcidev->dev, "Can't map eager buffers as "
			 "writable (flags=%lx)\n", vma->vm_flags);
		ret = -EPERM;
		goto bail;
	}
	/* don't allow them to later change to writeable w