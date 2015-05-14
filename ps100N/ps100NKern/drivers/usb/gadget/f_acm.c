o make sure the whole TX message (counting
 * from the 'relocated' message header) is aligned to
 * tx_block_size. We assume the _append() code has left enough space
 * in the FIFO for that. If there are no payloads, just pass, as it
 * won't be transferred.
 *
 * The amount of padding bytes depends on how many payloads are in the
 * TX message, as the "msg header and payload descriptors" will be
 * shifted up in the buffer.
 */
static
void i2400m_tx_close(struct i2400m *i2400m)
{
	struct device *dev = i2400m_dev(i2400m);
	struct i2400m_msg_hdr *tx_msg = i2400m->tx_msg;
	struct i2400m_msg_hdr *tx_msg_moved;
	size_t aligned_size, padding, hdr_size;
	void *pad_buf;
	unsigned num_pls;

	if (tx_msg->size & I2400M_TX_SKIP)	/* a skipper? nothing to do */
		goto out;
	num_pls = le16_to_cpu(tx_msg->num_pls);
	/* We can get this situation when a new message was started
	 * and there was no space to add payloads before hitting the
	 tail (and taking padding into consideration). */
	if (num_pls == 0) {
		tx_msg->size |= I2400M_TX_SKIP;
		goto out;
	}
	/* Relocate the message header
	 *
	 * Find the current header size, align it to 16 and if we need
	 * to move it so the tail is next to the payloads, move it and
	 * set the offset.
	 *
	 * If it moved, this header is good only for transmission; the
	 * original one (it is kept if we moved) is still used to
	 * figure out where the next TX message starts (and where the
	 * offset to the moved header is).
	 */
	hdr_size = sizeof(*tx_msg)
		+ le16_to_cpu(tx_msg->num_pls) * sizeof(tx_msg->pld[0]);
	hdr_size = ALIGN(hdr_size, I2400M_PL_ALIGN);
	tx_msg->offset = I2400M_TX_PLD_SIZE - hdr_size;
	tx_msg_moved = (void *) tx_msg + tx_msg->offset;
	memmove(tx_msg_moved, tx_msg, hdr_size);
	tx_msg_moved->size -= tx_msg->offset;
	/*
	 * Now figure out how much we have to add to the (moved!)
	 * message so the size is a multiple of i2400m->bus_tx_block_size.
	 */
	aligned_size = ALIGN(tx_msg_moved->size, i2400m->bus_tx_block_size);
	padding = aligned_size - tx_msg_moved->size;
	if (padding > 0) {
		pad_buf = i2400m_tx_fifo_push(i2400m, padding, 0);
		if (unlikely(WARN_ON(pad_buf == NULL
				     || pad_buf == TAIL_FULL))) {
			/* This should not happen -- append should verify
			 * there is always space left at least to append
			 * tx_block_size */
			dev_err(dev,
				"SW BUG! Possible data leakage from memory the "
				"device should not read for padding - "
				"size %lu aligned_size %zu tx_buf %p in "
				"%zu out %zu\n",
				(unsigned long) tx_msg_moved->size,
				aligned_size, i2400m->tx_buf, i2400m->tx_in,
				i2400m->tx_out);
		} else
			memset(pad_buf, 0xad, padding);
	}
	tx_msg_moved->padding = cpu_to_le16(padding);
	tx_msg_moved->size += padding;
	if (tx_msg != tx_msg_moved)
		tx_msg->size += padding;
out:
	i2400m->tx_msg = NULL;
}


/**
 * i2400m_tx - send the data in a buffer to the device
 *
 * @buf: pointer to the buffer to transmit
 *
 * @buf_len: buffer size
 *
 * @pl_type: type of the payload we are sending.
 *
 * Returns:
 *     0 if ok, < 0 errno code on error (-ENOSPC, if there is no more
 *     room for the message in the queue).
 *
 * Appends the buffer to the TX FIFO and notifies the bus-specific
 * part of the driver that there is new data ready to transmit.
 * Once this function returns, the buffer has been copied, so it can
 * be reused.
 *
 * The steps followed to append are explained in detail in the file
 * header.
 *
 * Whenever we write to a message, we increase msg->size, so it
 * reflects exactly how big the message is. This is needed so that if
 * we concatenate two messages before they can be sent, the code that
 * sends the messages can find the boundaries (and it will replace the
 * size with the real barker before sending).
 *
 * Note:
 *
 *     Cold and warm reset payloads need to be sent as a single
 *     payload, so we handle that.
 */
int i2400m_tx(struct i2400m *i2400m, const void *buf, size_t buf_len,
	      enum i2400m_pt pl_type)
{
	int result = -ENOSPC;
	struct device *dev = i2400m_dev(i2400m);
	unsigned long flags;
	size_t padded_len;
	void *ptr;
	unsigned is_singleton = pl_type == I2400M_PT_RESET_WARM
		|| pl_type == I2400M_PT_RESET_COLD;

	d_fnstart(3, dev, "(i2400m %p skb %p [%zu bytes] pt %u)\n",
		  i2400m, buf, buf_len, pl_type);
	padded_len = ALIGN(buf_len, I2400M_PL_ALIGN);
	d_printf(5, dev, "padded_len %zd buf_len %zd\n", padded_len, buf_len);
	/* If there is no current TX message, create one; if the
	 * current one is out of payload slots or we have a singleton,
	 * close it and start a new one */
	spin_lock_irqsave(&i2400m->tx_lock, flags);
try_new:
	if (unlikely(i2400m->tx_msg == NULL))
		i2400m_tx_new(i2400m);
	else if (unlikely(!i2400m_tx_fits(i2400m)
			  || (is_singleton && i2400m->tx_msg->num_pls != 0))) {
		d_printf(2, dev, "closing TX message (fits %u singleton "
			 "%u num_pls %u)\n", i2400m_tx_fits(i2400m),
			 is_singleton, i2400m->tx_msg->num_pls);
		i2400m_tx_close(i2400m);
		i2400m_tx_new(i2400m);
	}
	if (i2400m->tx_msg == NULL)
		goto error_tx_new;
	if (i2400m->tx_msg->size + padded_len > I2400M_TX_BUF_SIZE / 2) {
		d_printf(2, dev, "TX: message too big, going new\n");
		i2400m_tx_close(i2400m);
		i2400m_tx_new(i2400m);
	}
	if (i2400m->tx_msg == NULL)
		goto error_tx_new;
	/* So we have a current message header; now append space for
	 * the message -- if there is not enough, try the head */
	ptr = i2400m_tx_fifo_push(i2400m, padded_len,
				  i2400m->bus_tx_block_size);
	if (ptr == TAIL_FULL) {	/* Tail is full, try head */
		d_printf(2, dev, "pl append: tail full\n");
		i2400m_tx_close(i2400m);
		i2400m_tx_skip_tail(i2400m);
		goto try_new;
	} else if (ptr == NULL) {	/* All full */
		result = -ENOSPC;
		d_printf(2, dev, "pl append: all full\n");
	} else {			/* Got space, copy it, set padding */
		struct i2400m_msg_hdr *tx_msg = i2400m->tx_msg;
		unsigned num_pls = le16_to_cpu(tx_msg->num_pls);
		memcpy(ptr, buf, buf_len);
		memset(ptr + buf_len, 0xad, padded_len - buf_len);
		i2400m_pld_set(&tx_msg->pld[num_pls], buf_len, pl_type);
		d_printf(3, dev, "pld 0x%08x (type 0x%1x len 0x%04zx\n",
			 le32_to_cpu(tx_msg->pld[num_pls].val),
			 pl_type, buf_len);
		tx_msg->num_pls = le16_to_cpu(num_pls+1);
		tx_msg->size += padded_len;
		d_printf(2, dev, "TX: appended %zu b (up to %u b) pl #%u \n",
			padded_len, tx_msg->size, num_pls+1);
		d_printf(2, dev,
			 "TX: appended hdr @%zu %zu b pl #%u @%zu %zu/%zu b\n",
			 (void *)tx_msg - i2400m->tx_buf, (size_t)tx_msg->size,
			 num_pls+1, ptr - i2400m->tx_buf, buf_len, padded_len);
		result = 0;
		if (is_singleton)
			i2400m_tx_close(i2400m);
	}
error_tx_new:
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	i2400m->bus_tx_kick(i2400m);	/* always kick, might free up space */
	d_fnend(3, dev, "(i2400m %p skb %p [%zu bytes] pt %u) = %d\n",
		i2400m, buf, buf_len, pl_type, result);
	return result;
}
EXPORT_SYMBOL_GPL(i2400m_tx);


/**
 * i2400m_tx_msg_get - Get the first TX message in the FIFO to start sending it
 *
 * @i2400m: device descriptors
 * @bus_size: where to place the size of the TX message
 *
 * Called by the bus-specific driver to get the first TX message at
 * the FIF that is ready for transmission.
 *
 * It sets the state in @i2400m to indicate the bus-specific driver is
 * transfering that message (i2400m->tx_msg_size).
 *
 * Once the transfer is completed, call i2400m_tx_msg_sent().
 *
 * Notes:
 *
 *     The size of the TX message to be transmitted might be smaller than
 *     that of the TX message in the FIFO (in case the header was
 *     shorter). Hence, we copy it in @bus_size, for the bus layer to
 *     use. We keep the message's size in i2400m->tx_msg_size so that
 *     when the bus later is done transferring we know how much to
 *     advance the fifo.
 *
 *     We collect statistics here as all the data is available and we
 *     assume it is going to work [see i2400m_tx_msg_sent()].
 */
struct i2400m_msg_hdr *i2400m_tx_msg_get(struct i2400m *i2400m,
					 size_t *bus_size)
{
	struct device *dev = i2400m_dev(i2400m);
	struct i2400m_msg_hdr *tx_msg, *tx_msg_moved;
	unsigned long flags, pls;

	d_fnstart(3, dev, "(i2400m %p bus_size %p)\n", i2400m, bus_size);
	spin_lock_irqsave(&i2400m->tx_lock, flags);
skip:
	tx_msg_moved = NULL;
	if (i2400m->tx_in == i2400m->tx_out) {	/* Empty FIFO? */
		i2400m->tx_in = 0;
		i2400m->tx_out = 0;
		d_printf(2, dev, "TX: FIFO empty: resetting\n");
		goto out_unlock;
	}
	tx_msg = i2400m->tx_buf + i2400m->tx_out % I2400M_TX_BUF_SIZE;
	if (tx_msg->size & I2400M_TX_SKIP) {	/* skip? */
		d_printf(2, dev, "TX: skip: msg @%zu (%zu b)\n",
			 i2400m->tx_out % I2400M_TX_BUF_SIZE,
			 (size_t) tx_msg->size & ~I2400M_TX_SKIP);
		i2400m->tx_out += tx_msg->size & ~I2400M_TX_SKIP;
		goto skip;
	}

	if (tx_msg->num_pls == 0) {		/* No payloads? */
		if (tx_msg == i2400m->tx_msg) {	/* open, we are done */
			d_printf(2, dev,
				 "TX: FIFO empty: open msg w/o payloads @%zu\n",
				 (void *) tx_msg - i2400m->tx_buf);
			tx_msg = NULL;
			goto out_unlock;
		} else {			/* closed, skip it */
			d_printf(2, dev,
				 "TX: skip msg w/o payloads @%zu (%zu b)\n",
				 (void *) tx_msg - i2400m->tx_buf,
				 (size_t) tx_msg->size);
			i2400m->tx_out += tx_msg->size & ~I2400M_TX_SKIP;
			goto skip;
		}
	}
	if (tx_msg == i2400m->tx_msg)		/* open msg? */
		i2400m_tx_close(i2400m);

	/* Now we have a valid TX message (with payloads) to TX */
	tx_msg_moved = (void *) tx_msg + tx_msg->offset;
	i2400m->tx_msg_size = tx_msg->size;
	*bus_size = tx_msg_moved->size;
	d_printf(2, dev, "TX: pid %d msg hdr at @%zu offset +@%zu "
		 "size %zu bus_size %zu\n",
		 current->pid, (void *) tx_msg - i2400m->tx_buf,
		 (size_t) tx_msg->offset, (size_t) tx_msg->size,
		 (size_t) tx_msg_moved->size);
	tx_msg_moved->barker = le32_to_cpu(I2400M_H2D_PREVIEW_BARKER);
	tx_msg_moved->sequence = le32_to_cpu(i2400m->tx_sequence++);

	pls = le32_to_cpu(tx_msg_moved->num_pls);
	i2400m->tx_pl_num += pls;		/* Update stats */
	if (pls > i2400m->tx_pl_max)
		i2400m->tx_pl_max = pls;
	if (pls < i2400m->tx_pl_min)
		i2400m->tx_pl_min = pls;
	i2400m->tx_num++;
	i2400m->tx_size_acc += *bus_size;
	if (*bus_size < i2400m->tx_size_min)
		i2400m->tx_size_min = *bus_size;
	if (*bus_size > i2400m->tx_size_max)
		i2400m->tx_size_max = *bus_size;
out_unlock:
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	d_fnstart(3, dev, "(i2400m %p bus_size %p [%zu]) = %p\n",
		  i2400m, bus_size, *bus_size, tx_msg_moved);
	return tx_msg_moved;
}
EXPORT_SYMBOL_GPL(i2400m_tx_msg_get);


/**
 * i2400m_tx_msg_sent - indicate the transmission of a TX message
 *
 * @i2400m: device descriptor
 *
 * Called by the bus-specific driver when a message has been sent;
 * this pops it from the FIFO; and as there is space, start the queue
 * in case it was stopped.
 *
 * Should be called even if the message send failed and we are
 * dropping this TX message.
 */
void i2400m_tx_msg_sent(struct i2400m *i2400m)
{
	unsigned n;
	unsigned long flags;
	struct device *dev = i2400m_dev(i2400m);

	d_fnstart(3, dev, "(i2400m %p)\n", i2400m);
	spin_lock_irqsave(&i2400m->tx_lock, flags);
	i2400m->tx_out += i2400m->tx_msg_size;
	d_printf(2, dev, "TX: sent %zu b\n", (size_t) i2400m->tx_msg_size);
	i2400m->tx_msg_size = 0;
	BUG_ON(i2400m->tx_out > i2400m->tx_in);
	/* level them FIFO markers off */
	n = i2400m->tx_out / I2400M_TX_BUF_SIZE;
	i2400m->tx_out %= I2400M_TX_BUF_SIZE;
	i2400m->tx_in -= n * I2400M_TX_BUF_SIZE;
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	d_fnend(3, dev, "(i2400m %p) = void\n", i2400m);
}
EXPORT_SYMBOL_GPL(i2400m_tx_msg_sent);


/**
 * i2400m_tx_setup - Initialize the TX queue and infrastructure
 *
 * Make sure we reset the TX sequence to zero, as when this function
 * is called, the firmware has been just restarted.
 */
int i2400m_tx_setup(struct i2400m *i2400m)
{
	int result;

	/* Do this here only once -- can't do on
	 * i2400m_hard_start_xmit() as we'll cause race conditions if
	 * the WS was scheduled on another CPU */
	INIT_WORK(&i2400m->wake_tx_ws, i2400m_wake_tx_work);

	i2400m->tx_sequence = 0;
	i2400m->tx_buf = kmalloc(I2400M_TX_BUF_SIZE, GFP_KERNEL);
	if (i2400m->tx_buf == NULL)
		result = -ENOMEM;
	else
		result = 0;
	/* Huh? the bus layer has to define this... */
	BUG_ON(i2400m->bus_tx_block_size == 0);
	return result;

}


/**
 * i2400m_tx_release - Tear down the TX queue and infrastructure
 */
void i2400m_tx_release(struct i2400m *i2400m)
{
	kfree(i2400m->tx_buf);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    te |= ACM_CTRL_BRK;

	acm->serial_state = state;
	return acm_notify_serial_state(acm);
}

/*-------------------------------------------------------------------------*/

/* ACM function driver setup/binding */
static int __init
acm_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_acm		*acm = func_to_acm(f);
	int			status;
	struct usb_ep		*ep;

	/* allocate instance-specific interface IDs, and patch descriptors */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	acm->ctrl_id = status;

	acm_control_interface_desc.bInterfaceNumber = status;
	acm_union_desc .bMasterInterface0 = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	acm->data_id = status;

	acm_data_interface_desc.bInterfaceNumber = status;
	acm_union_desc.bSlaveInterface0 = status;
	acm_call_mgmt_descriptor.bDataInterface = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &acm_fs_in_desc);
	if (!ep)
		goto fail;
	acm->port.in = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &acm_fs_out_desc);
	if (!ep)
		goto fail;
	acm->port.out = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &acm_fs_notify_desc);
	if (!ep)
		goto fail;
	acm->notify = ep;
	ep->driver_data = cdev;	/* claim */

	/* allocate notification */
	acm->notify_req = gs_alloc_req(ep,
			sizeof(struct usb_cdc_notification) + 2,
			GFP_KERNEL);
	if (!acm->notify_req)
		goto fail;

	acm->notify_req->complete = acm_cdc_notify_complete;
	acm->notify_req->context = acm;

	/* copy descriptors, and track endpoint copies */
	f->descriptors = usb_copy_descriptors(acm_fs_function);
	if (!f->descriptors)
		goto fail;

	acm->fs.in = usb_find_endpoint(acm_fs_function,
			f->descriptors, &acm_fs_in_desc);
	acm->fs.out = usb_find_endpoint(acm_fs_function,
			f->descriptors, &acm_fs_out_desc);
	acm->fs.notify = usb_find_endpoint(acm_fs_function,
			f->descriptors, &acm_fs_notify_desc);

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		acm_hs_in_desc.bEndpointAddress =
				acm_fs_in_desc.bEndpointAddress;
		acm_hs_out_desc.bEndpointAddress =
				acm_fs_out_desc.bEndpointAddress;
		acm_hs_notify_desc.bEndpointAddress =
				acm_fs_notify_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(acm_hs_function);

		acm->hs.in = usb_find_endpoint(acm_hs_function,
				f->hs_descriptors, &acm_hs_in_desc);
		acm->hs.out = usb_find_endpoint(acm_hs_function,
				f->hs_descriptors, &acm_hs_out_desc);
		acm->hs.notify = usb_find_endpoint(acm_hs_function,
				f->hs_descriptors, &acm_hs_notify_desc);
	}

	DBG(cdev, "acm ttyGS%d: %s speed IN/%s OUT/%s NOTIFY/%s\n",
			acm->port_num,
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			acm->port.in->name, acm->port.out->name,
			acm->notify->name);
	return 0;

fail:
	if (acm->notify_req)
		gs_free_req(acm->notify, acm->notify_req);

	/* we might as well release our claims on endpoints */
	if (acm->notify)
		acm->notify->driver_data = NULL;
	if (acm->port.out)
		acm->port.out->driver_data = NULL;
	if (acm->port.in)
		acm->port.in->driver_data = NULL;

	ERROR(cdev, "%s/%p: can't bind, err %d\n", f->name, f, status);

	return status;
}

static void
acm_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_acm		*acm = func_to_acm(f);

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);
	gs_free_req(acm->notify, acm->notify_req);
	kfree(acm);
}

/* Some controllers can't support CDC ACM ... */
static inline bool can_support_cdc(struct usb_configuration *c)
{
	/* SH3 doesn't support multiple interfaces */
	if (gadget_is_sh(c->cdev->gadget))
		return false;

	/* sa1100 doesn't have a third interrupt endpoint */
	if (gadget_is_sa1100(c->cdev->gadget))
		return falseibsd_ucode_loaded(struct ipath_devdata *dd)
{
	if (!dd->serdes_first_init_done && (ipath_sd7220_ib_vfy(dd) > 0))
		dd->serdes_first_init_done = 1;
	return dd->serdes_first_init_done;
}

/* repeat #define for local use. "Real" #define is in ipath_iba7220.c */
#define INFINIPATH_HWE_IB_UC_MEMORYPARITYERR      0x0000004000000000ULL
#define IB_MPREG5 (EPB_LOC(6, 0, 0xE) | (1L << EPB_IB_UC_CS_SHF))
#define IB_MPREG6 (EPB_LOC(6, 0, 0xF) | (1U << EPB_IB_UC_CS_SHF))
#define UC_PAR_CLR_D 8
#define UC_PAR_CLR_M 0xC
#define IB_CTRL2(chn) (EPB_LOC(chn, 7, 3) | EPB_IB_QUAD0_CS)
#define START_EQ1(chan) EPB_LOC(chan, 7, 0x27)

void ipath_sd7220_clr_ibpar(struct ipath_devdata *dd)
{
	int ret;

	/* clear, then re-enable parity errs */
	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6,
		UC_PAR_CLR_D, UC_PAR_CLR_M);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed clearing IBSerDes Parity err\n");
		goto bail;
	}
	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6, 0,
		UC_PAR_CLR_M);

	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	udelay(4);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrclear,
		INFINIPATH_HWE_IB_UC_MEMORYPARITYERR);
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
bail:
	return;
}

/*
 * After a reset or other unusual event, the epb interface may need
 * to be re-synchronized, between the host and the uC.
 * returns <0 for failure to resync within IBSD_RESYNC_TRIES (not expected)
 */
#define IBSD_RESYNC_TRIES 3
#define IB_PGUDP(chn) (EPB_LOC((chn), 2, 1) | EPB_IB_QUAD0_CS)
#define IB_CMUDONE(chn) (EPB_LOC((chn), 7, 0xF) | EPB_IB_QUAD0_CS)

static int ipath_resync_ibepb(struct ipath_devdata *dd)
{
	int ret, pat, tries, chn;
	u32 loc;

	ret = -1;
	chn = 0;
	for (tries = 0; tries < (4 * IBSD_RESYNC_TRIES); ++tries) {
		loc = IB_PGUDP(chn);
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, 0, 0);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed read in resync\