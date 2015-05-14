ow
		 */
		if ((dd->ipath_piobcnt2k + dd->ipath_piobcnt4k) >
		    (sizeof(shadow[0]) * 4 * 4))
			ipath_dbg("2nd group: dmacopy: "
				  "%llx %llx %llx %llx\n"
				  "ipath  shadow:  %lx %lx %lx %lx\n",
				  (unsigned long long)le64_to_cpu(dma[4]),
				  (unsigned long long)le64_to_cpu(dma[5]),
				  (unsigned long long)le64_to_cpu(dma[6]),
				  (unsigned long long)le64_to_cpu(dma[7]),
				  shadow[4], shadow[5], shadow[6], shadow[7]);

		/* at end, so update likely happened */
		ipath_reset_availshadow(dd);
	}
}

/*
 * common code for normal driver pio buffer allocation, and reserved
 * allocation.
 *
 * do appropriate marking as busy, etc.
 * returns buffer number if one found (>=0), negative number is error.
 */
static u32 __iomem *ipath_getpiobuf_range(struct ipath_devdata *dd,
	u32 *pbufnum, u32 first, u32 last, u32 firsti)
{
	int i, j, updated = 0;
	unsigned piobcnt;
	unsigned long flags;
	unsigned long *shadow = dd->ipath_pioavailshadow;
	u32 __iomem *buf;

	piobcnt = last - first;
	if (dd->ipath_upd_pio_shadow) {
		/*
		 * Minor optimization.  If we had no buffers on last call,
		 * start out by doing the update; continue and do scan even
		 * if no buffers were updated, to be paranoid
		 */
		ipath_update_pio_bufs(dd);
		updated++;
		i = first;
	} else
		i = firsti;
rescan:
	/*
	 * while test_and_set_bit() is atomic, we do that and then the
	 * change_bit(), and the pair is not.  See if this is the cause
	 * of the remaining armlaunch errors.
	 */
	spin_lock_irqsave(&ipath_pioavail_lock, flags);
	for (j = 0; j < piobcnt; j++, i++) {
		if (i >= last)
			i = first;
		if (__test_and_set_bit((2 * i) + 1, shadow))
			continue;
		/* flip generation bit */
		__change_bit(2 * i, shadow);
		break;
	}
	spin_unlock_irqrestore(&ipath_pioavail_lock, flags);

	if (j == piobcnt) {
		if (!updated) {
			/*
			 * first time through; shadow exhausted, but may be
			 * buffers available, try an update and then rescan.
			 */
			ipath_update_pio_bufs(dd);
			updated++;
			i = first;
			goto rescan;
		} else if (updated == 1 && piobcnt <=
			((dd->ipath_sendctrl
			>> INFINIPATH_S_UPDTHRESH_SHIFT) &
			INFINIPATH_S_UPDTHRESH_MASK)) {
			/*
			 * for chips supporting and using the update
			 * threshold we need to force an update of the
			 * in-memory copy if the count is less than the
			 * thershold, then check one more time.
			 */
			ipath_force_pio_avail_update(dd);
			ipath_update_pio_bufs(dd);
			updated++;
			i = first;
			goto rescan;
		}

		no_pio_bufs(dd);
		buf = NULL;
	} else {
		if (i < dd->ipath_piobcnt2k)
			buf = (u32 __iomem *) (dd->ipath_pio2kbase +
					       i * dd->ipath_palign);
		else
			buf = (u32 __iomem *)
				(dd->ipath_pio4kbase +
				 (i - dd->ipath_piobcnt2k) * dd->ipath_4kalign);
		if (pbufnum)
			*pbufnum = i;
	}

	return buf;
}

/**
 * ipath_getpiobuf - find an available pio buffer
 * @dd: the infinipath device
 * @plen: the size of the PIO buffer needed in 32-bit words
 * @pbufnum: the buffer number is placed here
 */
u32 __iomem *ipath_getpiobuf(struct ipath_devdata *dd, u32 plen, u32 *pbufnum)
{
	u32 __iomem *buf;
	u32 pnum, nbufs;
	u32 first, lasti;

	if (plen + 1 >= IPATH_SMALLBUF_DWORDS) {
		first = dd->ipath_piobcnt2k;
		lasti = dd->ipath_lastpioindexl;
	} else {
		first = 0;
		lasti = dd->ipath_lastpioindex;
	}
	nbufs = dd->ipath_piobcnt2k + dd->ipath_piobcnt4k;
	buf = ipath_getpiobuf_range(dd, &pnum, first, nbufs, lasti);

	if (buf) {
		/*
		 * Set next starting place.  It's just an optimization,
		 * it doesn't matter who wins on this, so no locking
		 */
		if (plen + 1 >= IPATH_SMALLBUF_DWORDS)
			dd->ipath_lastpioindexl = pnum + 1;
		else
			dd->ipath_lastpioindex = pnum + 1;
		if (dd->ipath_upd_pio_shadow)
			dd->ipath_upd_pio_shadow = 0;
		if (dd->ipath_consec_nopiobuf)
			dd->ipath_consec_nopiobuf = 0;
		ipath_cdbg(VERBOSE, "Return piobuf%u %uk @ %p\n",
			   pnum, (pnum < dd->ipath_piobcnt2k) ? 2 : 4, buf);
		if (pbufnum)
			*pbufnum = pnum;

	}
	return buf;
}

/**
 * ipath_chg_pioavailkernel - change which send buffers are available for kernel
 * @dd: the infinipath device
 * @start: the starting send buffer number
 * @len: the number of send buffers
 * @avail: true if the buffers are available for kernel use, false otherwise
 */
void ipath_chg_pioavailkernel(struct ipath_devdata *dd, unsigned start,
			      unsigned len, int avail)
{
	unsigned long flags;
	unsigned end, cnt = 0, next;

	/* There are two bits per send buffer (busy and generation) */
	start *= 2;
	end = start + len * 2;

	spin_lock_irqsave(&ipath_pioavail_lock, flags);
	/* Set or clear the busy bit in the shadow. */
	while (start < end) {
		if (avail) {
			unsigned long dma;
			int i, im;
			/*
			 * the BUSY bit will never be set, because we disarm
			 * the user buffers before we hand them back to the
			 * kernel.  We do have to make sure the generation
			 * bit is set correctly in shadow, since i