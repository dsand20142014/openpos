g_bh |= BH_RECEIVE;
		info->rx_overflow = true;
		info->icount.buf_overrun++;
		return;
	}

	if (eom) {
		/* end of frame, get FIFO count from RBCL register */
		if (!(fifo_count = (unsigned char)(read_reg(info, CHA+RBCL) & 0x1f)))
			fifo_count = 32;
	} else
		fifo_count = 32;

	do {
		if (fifo_count == 1) {
			read_count = 1;
			data[0] = read_reg(info, CHA + RXFIFO);
		} else {
			read_count = 2;
			*((unsigned short *) data) = read_reg16(info, CHA + RXFIFO);
		}
		fifo_count -= read_count;
		if (!fifo_count && eom)
			buf->status = data[--read_count];

		for (i = 0; i < read_count; i++) {
			if (buf->count >= info->max_frame_size) {
				/* frame too large, reset receiver and reset current buffer */
				issue_command(info, CHA, CMD_RXRESET);
				buf->count = 0;
				return;
			}
			*(buf->data + buf->count) = data[i];
			buf->count++;
		}
	} while (fifo_count);

	if (eom) {
		info->pending_bh |= BH_RECEIVE;
		info->rx_frame_count++;
		info->rx_put++;
		if (info->rx_put >= info->rx_buf_count)
			info->rx_put = 0;
	}
	issue_command(info, CHA, CMD_RXFIFO);
}

static void rx_ready_async(MGSLPC_INFO *info, int tcd, struct tty_struct *tty)
{
	unsigned char data, status, flag;
	int fifo_count;
	int work = 0;
 	struct mgsl_icount *icount = &info->icount;

	if (tcd) {
		/* early termination, get FIFO count from RBCL register */
		fifo_count = (unsigned char)(read_reg(info, CHA+RBCL) & 0x1f);

		/* Zero fifo count could mean 0 or 32 bytes available.
		 * If BIT5 of STAR is set then at least 1 byte is available.
		 */
		if (!fifo_count && (read_reg(info,CHA+STAR) & BIT5))
			fifo_count = 32;
	} else
		fifo_count = 32;

	tty_buffer_request_room(tty, fifo_count);
	/* Flush received async data to receive data buffer. */
	while (fifo_count) {
		data   = read_reg(info, CHA + RXFIFO);
		status = read_reg(info, CHA + RXFIFO);
		fifo_count -= 2;

		icount->rx++;
		flag = TTY_NORMAL;

		// if no frameing/crc error then save data
		// BIT7:parity error
		// BIT6:framing error

		if (status & (BIT7 + BIT6)) {
			if (status & BIT7)
				icount->parity++;
			else
				icount->frame++;

			/* discard char if tty control flags say so */
			if (status & info->ignore_status_mask)
				continue;

			status &= info->read_status_mask;

			if (status & BIT7)
				flag = TTY_PARITY;
			else if (status & BIT6)
				flag = TTY_FRAME;
		}
		work += tty_insert_flip_char(tty, data, flag);
	}
	issue_command(info, CHA, CMD_RXFIFO);

	if (debug_level >= DEBUG_LEVEL_ISR) {
		printk("%s(%d):rx_ready_async",
			__FILE__,__LINE__);
		printk("%s(%d):rx=%d brk=%d parity=%d frame=%d overrun=%d\n",
			__FILE__,__LINE__,icount->rx,icount->brk,
			icount->parity,icount->frame,icount->overrun);
	}

	if (work)
		tty_flip_buffer_push(tty);
}


static void tx_done(MGSLPC_INFO *info, struct tty_struct *tty)
{
	if (!info->tx_active)
		return;

	info->tx_active = false;
	info->tx_aborting = false;

	if (info->params.mode == MGSL_MODE_ASYNC)
		return;

	info->tx_count = info->tx_put = info->tx_get = 0;
	del_timer(&info->tx_timer);

	if (info->drop_rts_on_tx_done) {
		get_signals(info);
		if (info->serial_signals & SerialSignal_RTS) {
			info->serial_signals &= ~SerialSignal_RTS;
			set_signals(info);
		}
		info->drop_rts_on_tx_done = false;
	}

#if SYNCLINK_GENERIC_HDLC
	if (info->netcount)
		hdlcdev_tx_done(info);
	else
#endif
	{
		if (tty->stopped || tty->hw_stopped) {
			tx_stop(info);
			return;
		}
		info->pending_bh |= BH_TRANSMIT;
	}
}

static void tx_ready(MGSLPC_INFO *info, struct tty_struct *tty)
{
	unsigned char fifo_count = 32;
	int c;

	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):tx_ready(%s)\n", __FILE__,__LINE__,info->device_name);

	if (info->params.mode == MGSL_MODE_HDLC) {
		if (!info->tx_active)
			return;
	} else {
		if (tty->stopped || tty->hw_stopped) {
			tx_stop(info);
			return;
		}
		if (!info->tx_count)
			info->tx_active = false;
	}

	if (!info->tx_count)
		return;

	while (info->tx_count && fifo_count) {
		c = min(2, min_t(int, fifo_count, min(info->tx_count, TXBUFSIZE - info->tx_get)));

		if (c == 1) {
			write_reg(info, CHA + TXFIFO, *(info->tx_buf + info->tx_get));
		} else {
			write_reg16(info, CHA + TXFIFO,
					  *((unsigned short*)(info->tx_buf + info->tx_get)));
		}
		info->tx_count -= c;
		info->tx_get = (info->tx_get + c) & (TXBUFSIZE - 1);
		fifo_count -= c;
	}

	if (info->params.mode == MGSL_MODE_ASYNC) {
		if (info->tx_count < WAKEUP_CHARS)
			info->pending_bh |= BH_TRANSMIT;
		issue_command(info, CHA, CMD_TXFIFO);
	} else {
		if (info->tx_count)
			issue_command(info, CHA, CMD_TXFIFO);
		else
			issue_command(info, CHA, CMD_TXFIFO + CMD_TXEOM);
	}
}

static void cts_change(MGSLPC_INFO *info, struct tty_struct *tty)
{
	get_signals(info);
	if ((info->cts_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		irq_disable(info, CHB, IRQ_CTS);
	info->icount.cts++;
	if (info->serial_signals & SerialSignal_CTS)
		info->input_signal_events.cts_up++;
	else
		info->input_signal_events.cts_down++;
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);

	if (info->port.flags & ASYNC_CTS_FLOW) {
		if (tty->hw_stopped) {
			if (info->serial_signals & SerialSignal_CTS) {
				if (debug_level >= DEBUG_LEVEL_ISR)
					printk("CTS tx start...");
				if (tty)
					tty->hw_stopped = 0;
				tx_start(info, tty);
				info->pending_bh |= BH_TRANSMIT;
				return;
			}
		} else {
			if (!(info->serial_signals & SerialSignal_CTS)) {
				if (debug_level >= DEBUG_LEVEL_ISR)
					printk("CTS tx stop...");
				if (tty)
					tty->hw_stopped = 1;
				tx_stop(info);
			}
		}
	}
	info->pending_bh |= BH_STATUS;
}

static void dcd_change(MGSLPC_INFO *info, struct tty_struct *tty)
{
	get_signals(info);
	if ((info->dcd_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		irq_disable(info, CHB, IRQ_DCD);
	info->icount.dcd++;
	if (info->serial_signals & SerialSignal_DCD) {
		info->input_signal_events.dcd_up++;
	}
	else
		info->input_signal_events.dcd_down++;
#if SYNCLINK_GENERIC_HDLC
	if (info->netcount) {
		if (info->serial_signals & SerialSignal_DCD)
			netif_carrier_on(info->netdev);
		else
			netif_carrier_off(info->netdev);
	}
#endif
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);

	if (info->port.flags & ASYNC_CHECK_CD) {
		if (debug_level >= DEBUG_LEVEL_ISR)
			printk("%s CD now %s...", info->device_name,
			       (info->serial_signals & SerialSignal_DCD) ? "on" : "off");
		if (info->serial_signals & SerialSignal_DCD)
			wake_up_interruptible(&info->port.open_wait);
		else {
			if (debug_level >= DEBUG_LEVEL_ISR)
				printk("doing serial hangup...");
			if (tty)
				tty_hangup(tty);
		}
	}
	info->pending_bh |= BH_STATUS;
}

static void dsr_change(MGSLPC_INFO *info)
{
	get_signals(info);
	if ((info->dsr_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		port_irq_disable(info, PVR_DSR);
	info->icount.dsr++;
	if (info->serial_signals & SerialSignal_DSR)
		info->input_signal_events.dsr_up++;
	else
		info->input_signal_events.dsr_down++;
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);
	info->pending_bh |= BH_STATUS;
}

static void ri_change(MGSLPC_INFO *info)
{
	get_signals(info);
	if ((info->ri_chkcount)++ >= IO_PIN_SHUTDOWN_LIMIT)
		port_irq_disable(info, PVR_RI);
	info->icount.rng++;
	if (info->serial_signals & SerialSignal_RI)
		info->input_signal_events.ri_up++;
	else
		info->input_signal_events.ri_down++;
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);
	info->pending_bh |= BH_STATUS;
}

/* Interrupt service routine entry point.
 *
 * Arguments:
 *
 * irq     interrupt number that caused interrupt
 * dev_id  device ID supplied during interrupt registration
 */
static irqreturn_t mgslpc_isr(int dummy, void *dev_id)
{
	MGSLPC_INFO *info = dev_id;
	struct tty_struct *tty;
	unsigned short isr;
	unsigned char gis, pis;
	int count=0;

	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("mgslpc_isr(%d) entry.\n", info->irq_level);

	if (!(info->p_dev->_locked))
		return IRQ_HANDLED;

	tty = tty_port_tty_get(&info->port);

	spin_lock(&info->lock);

	while ((gis = read_reg(info, CHA + GIS))) {
		if (debug_level >= DEBUG_LEVEL_ISR)
			printk("mgslpc_isr %s gis=%04X\n", info->device_name,gis);

		if ((gis & 0x70) || count > 1000) {
			printk("synclink_cs:hardware failed or ejected\n");
			break;
		}
		count++;

		if (gis & (BIT1 + BIT0)) {
			isr = read_reg16(info, CHB + ISR);
			if (isr & IRQ_DCD)
				dcd_change(info, tty);
			if (isr & IRQ_CTS)
				cts_change(info, tty);
		}
		if (gis & (BIT3 + BIT2))
		{
			isr = read_reg16(info, CHA + ISR);
			if (isr & IRQ_TIMER) {
				info->irq_occurred = true;
				irq_disable(info, CHA, IRQ_TIMER);
			}

			/* receive IRQs */
			if (isr & IRQ_EXITHUNT) {
				info->icount.exithunt++;
				wake_up_interruptible(&info->event_wait_q);
			}
			if (isr & IRQ_BREAK_ON) {
				info->icount.brk++;
				if (info->port.flags & ASYNC_SAK)
					do_SAK(tty);
			}
			if (isr & IRQ_RXTIME) {
				issue_command(info, CHA, CMD_RXFIFO_READ);
			}
			if (isr & (IRQ_RXEOM + IRQ_RXFIFO)) {
				if (info->params.mode == MGSL_MODE_HDLC)
					rx_ready_hdlc(info, isr & IRQ_RXEOM);
				else
					rx_ready_async(info, isr & IRQ_RXEOM, tty);
			}

			/* transmit IRQs */
			if (isr & IRQ_UNDERRUN) {
				if (info->tx_aborting)
					info->icount.txabort++;
				else
					info->icount.txunder++;
				tx_done(info, tty);
			}
			else if (isr & IRQ_ALLSENT) {
				info->icount.txok++;
				tx_done(info, tty);
			}
			else if (isr & IRQ_TXFIFO)
				tx_ready(info, tty);
		}
		if (gis & BIT7) {
			pis = read_reg(info, CHA + PIS);
			if (pis & BIT1)
				dsr_change(info);
			if (pis & BIT2)
				ri_change(info);
		}
	}

	/* Request bottom half processing if there's something
	 * for it to do and the bh is not already running
	 */

	if (info->pending_bh && !info->bh_running && !info->bh_requested) {
		if ( debug_level >= DEBUG_LEVEL_ISR )
			printk("%s(%d):%s queueing bh task.\n",
				__FILE__,__LINE__,info->device_name);
		schedule_work(&info->task);
		info->bh_requested = true;
	}

	spin_unlock(&info->lock);
	tty_kref_put(tty);

	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):mgslpc_isr(%d)exit.\n",
		       __FILE__, __LINE__, info->irq_level);

	return IRQ_HANDLED;
}

/* Initialize and start device.
 */
static int startup(MGSLPC_INFO * info, struct tty_struct *tty)
{
	int retval = 0;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):startup(%s)\n",__FILE__,__LINE__,info->device_name);

	if (info->port.flags & ASYNC_INITIALIZED)
		return 0;

	if (!info->tx_buf) {
		/* allocate a page of memory for a transmit buffer */
		info->tx_buf = (unsigned char *)get_zeroed_page(GFP_KERNEL);
		if (!info->tx_buf) {
			printk(KERN_ERR"%s(%d):%s can't allocate transmit buffer\n",
				__FILE__,__LINE__,info->device_name);
			return -ENOMEM;
		}
	}

	info->pending_bh = 0;

	memset(&info->icount, 0, sizeof(info->icount));

	setup_timer(&info->tx_timer, tx_timeout, (unsigned long)info);

	/* Allocate and claim adapter resources */
	retval = claim_resources(info);

	/* perform existance check and diagnostics */
	if ( !retval )
		retval = adapter_test(info);

	if ( retval ) {
  		if (capable(CAP_SYS_ADMIN) && tty)
			set_bit(TTY_IO_ERROR, &tty->flags);
		release_resources(info);
  		return retval;
  	}

	/* program hardware for current parameters */
	mgslpc_change_params(info, tty);

	if (tty)
		clear_bit(TTY_IO_ERROR, &tty->flags);

	info->port.flags |= ASYNC_INITIALIZED;

	return 0;
}

/* Called by mgslpc_close() and mgslpc_hangup() to shutdown hardware
 */
static void shutdown(MGSLPC_INFO * info, struct tty_struct *tty)
{
	unsigned long flags;

	if (!(info->port.flags & ASYNC_INITIALIZED))
		return;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_shutdown(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	/* clear status wait queue because status changes */
	/* can't happen after shutting down the hardware */
	wake_up_interruptible(&info->status_event_wait_q);
	wake_up_interruptible(&info->event_wait_q);

	del_timer_sync(&info->tx_timer);

	if (info->tx_buf) {
		free_page((unsigned long) info->tx_buf);
		info->tx_buf = NULL;
	}

	spin_lock_irqsave(&info->lock,flags);

	rx_stop(info);
	tx_stop(info);

	/* TODO:disable interrupts instead of reset to preserve signal states */
	reset_device(info);

 	if (!tty || tty->termios->c_cflag & HUPCL) {
 		info->serial_signals &= ~(SerialSignal_DTR + SerialSignal_RTS);
		set_signals(info);
	}

	spin_unlock_irqrestore(&info->lock,flags);

	release_resources(info);

	if (tty)
		set_bit(TTY_IO_ERROR, &tty->flags);

	info->port.flags &= ~ASYNC_INITIALIZED;
}

static void mgslpc_program_hw(MGSLPC_INFO *info, struct tty_struct *tty)
{
	unsigned long flags;

	spin_lock_irqsave(&info->lock,flags);

	rx_stop(info);
	tx_stop(info);
	info->tx_count = info->tx_put = info->tx_get = 0;

	if (info->params.mode == MGSL_MODE_HDLC || info->netcount)
		hdlc_mode(info);
	else
		async_mode(info);

	set_signals(info);

	info->dcd_chkcount = 0;
	info->cts_chkcount = 0;
	info->ri_chkcount = 0;
	info->dsr_chkcount = 0;

	irq_enable(info, CHB, IRQ_DCD | IRQ_CTS);
	port_irq_enable(info, (unsigned char) PVR_DSR | PVR_RI);
	get_signals(info);

	if (info->netcount || (tty && (tty->termios->c_cflag & CREAD)))
		rx_start(info);

	spin_unlock_irqrestore(&info->lock,flags);
}

/* Reconfigure adapter based on new parameters
 */
static void mgslpc_change_params(MGSLPC_INFO *info, struct tty_struct *tty)
{
	unsigned cflag;
	int bits_per_char;

	if (!tty || !tty->termios)
		return;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_change_params(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	cflag = tty->termios->c_cflag;

	/* if B0 rate (hangup) specified then negate DTR and RTS */
	/* otherwise assert DTR and RTS */
 	if (cflag & CBAUD)
		info->serial_signals |= SerialSignal_RTS + SerialSignal_DTR;
	else
		info->serial_signals &= ~(SerialSignal_RTS + SerialSignal_DTR);

	/* byte size and parity */

	switch (cflag & CSIZE) {
	case CS5: info->params.data_bits = 5; break;
	case CS6: info->params.data_bits = 6; break;
	case CS7: info->params.data_bits = 7; break;
	case CS8: info->params.data_bits = 8; break;
	default:  info->params.data_bits = 7; break;
	}

	if (cflag & CSTOPB)
		info->params.stop_bits = 2;
	else
		info->params.stop_bits = 1;

	info->params.parity = ASYNC_PARITY_NONE;
	if (cflag & PARENB) {
		if (cflag & PARODD)
			info->params.parity = ASYNC_PARITY_ODD;
		else
			info->params.parity = ASYNC_PARITY_EVEN;
#ifdef CMSPAR
		if (cflag & CMSPAR)
			info->params.parity = ASYNC_PARITY_SPACE;
#endif
	}

	/* calculate number of jiffies to transmit a full
	 * FIFO (32 bytes) at specified data rate
	 */
	bits_per_char = info->params.data_bits +
			info->params.stop_bits + 1;

	/* if port data rate is set to 460800 or less then
	 * allow tty settings to override, otherwise keep the
	 * current data rate.
	 */
	if (info->params.data_rate <= 460800) {
		info->params.data_rate = tty_get_baud_rate(tty);
	}

	if ( info->params.data_rate ) {
		info->timeout = (32*HZ*bits_per_char) /
				info->params.data_rate;
	}
	info->timeout += HZ/50;		/* Add .02 seconds of slop */

	if (cflag & CRTSCTS)
		info->port.flags |= ASYNC_CTS_FLOW;
	else
		info->port.flags &= ~ASYNC_CTS_FLOW;

	if (cflag & CLOCAL)
		info->port.flags &= ~ASYNC_CHECK_CD;
	else
		info->port.flags |= ASYNC_CHECK_CD;

	/* process tty input control flags */

	info->read_status_mask = 0;
	if (I_INPCK(tty))
		info->read_status_mask |= BIT7 | BIT6;
	if (I_IGNPAR(tty))
		info->ignore_status_mask |= BIT7 | BIT6;

	mgslpc_program_hw(info, tty);
}

/* Add a character to the transmit buffer
 */
static int mgslpc_put_char(struct tty_struct *tty, unsigned char ch)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO) {
		printk( "%s(%d):mgslpc_put_char(%d) on %s\n",
			__FILE__,__LINE__,ch,info->device_name);
	}

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_put_char"))
		return 0;

	if (!info->tx_buf)
		return 0;

	spin_lock_irqsave(&info->lock,flags);

	if (info->params.mode == MGSL_MODE_ASYNC || !info->tx_active) {
		if (info->tx_count < TXBUFSIZE - 1) {
			info->tx_buf[info->tx_put++] = ch;
			info->tx_put &= TXBUFSIZE-1;
			info->tx_count++;
		}
	}

	spin_unlock_irqrestore(&info->lock,flags);
	return 1;
}

/* Enable transmitter so remaining characters in the
 * transmit buffer are sent.
 */
static void mgslpc_flush_chars(struct tty_struct *tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk( "%s(%d):mgslpc_flush_chars() entry on %s tx_count=%d\n",
			__FILE__,__LINE__,info->device_name,info->tx_count);

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_flush_chars"))
		return;

	if (info->tx_count <= 0 || tty->stopped ||
	    tty->hw_stopped || !info->tx_buf)
		return;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk( "%s(%d):mgslpc_flush_chars() entry on %s starting transmitter\n",
			__FILE__,__LINE__,info->device_name);

	spin_lock_irqsave(&info->lock,flags);
	if (!info->tx_active)
	 	tx_start(info, tty);
	spin_unlock_irqrestore(&info->lock,flags);
}

/* Send a block of data
 *
 * Arguments:
 *
 * tty        pointer to tty information structure
 * buf	      pointer to buffer containing send data
 * count      size of send data in bytes
 *
 * Returns: number of characters written
 */
static int mgslpc_write(struct tty_struct * tty,
			const unsigned char *buf, int count)
{
	int c, ret = 0;
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk( "%s(%d):mgslpc_write(%s) count=%d\n",
			__FILE__,__LINE__,info->device_name,count);

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_write") ||
		!info->tx_buf)
		goto cleanup;

	if (info->params.mode == MGSL_MODE_HDLC) {
		if (count > TXBUFSIZE) {
			ret = -EIO;
			goto cleanup;
		}
		if (info->tx_active)
			goto cleanup;
		else if (info->tx_count)
			goto start;
	}

	for (;;) {
		c = min(count,
			min(TXBUFSIZE - info->tx_count - 1,
			    TXBUFSIZE - info->tx_put));
		if (c <= 0)
			break;

		memcpy(info->tx_buf + info->tx_put, buf, c);

		spin_lock_irqsave(&info->lock,flags);
		info->tx_put = (info->tx_put + c) & (TXBUFSIZE-1);
		info->tx_count += c;
		spin_unlock_irqrestore(&info->lock,flags);

		buf += c;
		count -= c;
		ret += c;
	}
start:
 	if (info->tx_count && !tty->stopped && !tty->hw_stopped) {
		spin_lock_irqsave(&info->lock,flags);
		if (!info->tx_active)
		 	tx_start(info, tty);
		spin_unlock_irqrestore(&info->lock,flags);
 	}
cleanup:
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk( "%s(%d):mgslpc_write(%s) returning=%d\n",
			__FILE__,__LINE__,info->device_name,ret);
	return ret;
}

/* Return the count of free bytes in transmit buffer
 */
static int mgslpc_write_room(struct tty_struct *tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	int ret;

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_write_room"))
		return 0;

	if (info->params.mode == MGSL_MODE_HDLC) {
		/* HDLC (frame oriented) mode */
		if (info->tx_active)
			return 0;
		else
			return HDLC_MAX_FRAME_SIZE;
	} else {
		ret = TXBUFSIZE - info->tx_count - 1;
		if (ret < 0)
			ret = 0;
	}

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_write_room(%s)=%d\n",
			 __FILE__,__LINE__, info->device_name, ret);
	return ret;
}

/* Return the count of bytes in transmit buffer
 */
static int mgslpc_chars_in_buffer(struct tty_struct *tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	int rc;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_chars_in_buffer(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_chars_in_buffer"))
		return 0;

	if (info->params.mode == MGSL_MODE_HDLC)
		rc = info->tx_active ? info->max_frame_size : 0;
	else
		rc = info->tx_count;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_chars_in_buffer(%s)=%d\n",
			 __FILE__,__LINE__, info->device_name, rc);

	return rc;
}

/* Discard all data in the send buffer
 */
static void mgslpc_flush_buffer(struct tty_struct *tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_flush_buffer(%s) entry\n",
			 __FILE__,__LINE__, info->device_name );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_flush_buffer"))
		return;

	spin_lock_irqsave(&info->lock,flags);
	info->tx_count = info->tx_put = info->tx_get = 0;
	del_timer(&info->tx_timer);
	spin_unlock_irqrestore(&info->lock,flags);

	wake_up_interruptible(&tty->write_wait);
	tty_wakeup(tty);
}

/* Send a high-priority XON/XOFF character
 */
static void mgslpc_send_xchar(struct tty_struct *tty, char ch)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_send_xchar(%s,%d)\n",
			 __FILE__,__LINE__, info->device_name, ch );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_send_xchar"))
		return;

	info->x_char = ch;
	if (ch) {
		spin_lock_irqsave(&info->lock,flags);
		if (!info->tx_enabled)
		 	tx_start(info, tty);
		spin_unlock_irqrestore(&info->lock,flags);
	}
}

/* Signal remote device to throttle send data (our receive data)
 */
static void mgslpc_throttle(struct tty_struct * tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_throttle(%s) entry\n",
			 __FILE__,__LINE__, info->device_name );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_throttle"))
		return;

	if (I_IXOFF(tty))
		mgslpc_send_xchar(tty, STOP_CHAR(tty));

 	if (tty->termios->c_cflag & CRTSCTS) {
		spin_lock_irqsave(&info->lock,flags);
		info->serial_signals &= ~SerialSignal_RTS;
	 	set_signals(info);
		spin_unlock_irqrestore(&info->lock,flags);
	}
}

/* Signal remote device to stop throttling send data (our receive data)
 */
static void mgslpc_unthrottle(struct tty_struct * tty)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_unthrottle(%s) entry\n",
			 __FILE__,__LINE__, info->device_name );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_unthrottle"))
		return;

	if (I_IXOFF(tty)) {
		if (info->x_char)
			info->x_char = 0;
		else
			mgslpc_send_xchar(tty, START_CHAR(tty));
	}

 	if (tty->termios->c_cflag & CRTSCTS) {
		spin_lock_irqsave(&info->lock,flags);
		info->serial_signals |= SerialSignal_RTS;
	 	set_signals(info);
		spin_unlock_irqrestore(&info->lock,flags);
	}
}

/* get the current serial statistics
 */
static int get_stats(MGSLPC_INFO * info, struct mgsl_icount __user *user_icount)
{
	int err;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("get_params(%s)\n", info->device_name);
	if (!user_icount) {
		memset(&info->icount, 0, sizeof(info->icount));
	} else {
		COPY_TO_USER(err, user_icount, &info->icount, sizeof(struct mgsl_icount));
		if (err)
			return -EFAULT;
	}
	return 0;
}

/* get the current serial parameters
 */
static int get_params(MGSLPC_INFO * info, MGSL_PARAMS __user *user_params)
{
	int err;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("get_params(%s)\n", info->device_name);
	COPY_TO_USER(err,user_params, &info->params, sizeof(MGSL_PARAMS));
	if (err)
		return -EFAULT;
	return 0;
}

/* set the serial parameters
 *
 * Arguments:
 *
 * 	info		pointer to device instance data
 * 	new_params	user buffer containing new serial params
 *
 * Returns:	0 if success, otherwise error code
 */
static int set_params(MGSLPC_INFO * info, MGSL_PARAMS __user *new_params, struct tty_struct *tty)
{
 	unsigned long flags;
	MGSL_PARAMS tmp_params;
	int err;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):set_params %s\n", __FILE__,__LINE__,
			info->device_name );
	COPY_FROM_USER(err,&tmp_params, new_params, sizeof(MGSL_PARAMS));
	if (err) {
		if ( debug_level >= DEBUG_LEVEL_INFO )
			printk( "%s(%d):set_params(%s) user buffer copy failed\n",
				__FILE__,__LINE__,info->device_name);
		return -EFAULT;
	}

	spin_lock_irqsave(&info->lock,flags);
	memcpy(&info->params,&tmp_params,sizeof(MGSL_PARAMS));
	spin_unlock_irqrestore(&info->lock,flags);

 	mgslpc_change_params(info, tty);

	return 0;
}

static int get_txidle(MGSLPC_INFO * info, int __user *idle_mode)
{
	int err;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("get_txidle(%s)=%d\n", info->device_name, info->idle_mode);
	COPY_TO_USER(err,idle_mode, &info->idle_mode, sizeof(int));
	if (err)
		return -EFAULT;
	return 0;
}

static int set_txidle(MGSLPC_INFO * info, int idle_mode)
{
 	unsigned long flags;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("set_txidle(%s,%d)\n", info->device_name, idle_mode);
	spin_lock_irqsave(&info->lock,flags);
	info->idle_mode = idle_mode;
	tx_set_idle(info);
	spin_unlock_irqrestore(&info->lock,flags);
	return 0;
}

static int get_interface(MGSLPC_INFO * info, int __user *if_mode)
{
	int err;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("get_interface(%s)=%d\n", info->device_name, info->if_mode);
	COPY_TO_USER(err,if_mode, &info->if_mode, sizeof(int));
	if (err)
		return -EFAULT;
	return 0;
}

static int set_interface(MGSLPC_INFO * info, int if_mode)
{
 	unsigned long flags;
	unsigned char val;
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("set_interface(%s,%d)\n", info->device_name, if_mode);
	spin_lock_irqsave(&info->lock,flags);
	info->if_mode = if_mode;

	val = read_reg(info, PVR) & 0x0f;
	switch (info->if_mode)
	{
	case MGSL_INTERFACE_RS232: val |= PVR_RS232; break;
	case MGSL_INTERFACE_V35:   val |= PVR_V35;   break;
	case MGSL_INTERFACE_RS422: val |= PVR_RS422; break;
	}
	write_reg(info, PVR, val);

	spin_unlock_irqrestore(&info->lock,flags);
	return 0;
}

static int set_txenable(MGSLPC_INFO * info, int enable, struct tty_struct *tty)
{
 	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("set_txenable(%s,%d)\n", info->device_name, enable);

	spin_lock_irqsave(&info->lock,flags);
	if (enable) {
		if (!info->tx_enabled)
			tx_start(info, tty);
	} else {
		if (info->tx_enabled)
			tx_stop(info);
	}
	spin_unlock_irqrestore(&info->lock,flags);
	return 0;
}

static int tx_abort(MGSLPC_INFO * info)
{
 	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("tx_abort(%s)\n", info->device_name);

	spin_lock_irqsave(&info->lock,flags);
	if (info->tx_active && info->tx_count &&
	    info->params.mode == MGSL_MODE_HDLC) {
		/* clear data count so FIFO is not filled on next IRQ.
		 * This results in underrun and abort transmission.
		 */
		info->tx_count = info->tx_put = info->tx_get = 0;
		info->tx_aborting = true;
	}
	spin_unlock_irqrestore(&info->lock,flags);
	return 0;
}

static int set_rxenable(MGSLPC_INFO * info, int enable)
{
 	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("set_rxenable(%s,%d)\n", info->device_name, enable);

	spin_lock_irqsave(&info->lock,flags);
	if (enable) {
		if (!info->rx_enabled)
			rx_start(info);
	} else {
		if (info->rx_enabled)
			rx_stop(info);
	}
	spin_unlock_irqrestore(&info->lock,flags);
	return 0;
}

/* wait for specified event to occur
 *
 * Arguments:	 	info	pointer to device instance data
 * 			mask	pointer to bitmask of events to wait for
 * Return Value:	0 	if successful and bit mask updated with
 *				of events triggerred,
 * 			otherwise error code
 */
static int wait_events(MGSLPC_INFO * info, int __user *mask_ptr)
{
 	unsigned long flags;
	int s;
	int rc=0;
	struct mgsl_icount cprev, cnow;
	int events;
	int mask;
	struct	_input_signal_events oldsigs, newsigs;
	DECLARE_WAITQUEUE(wait, current);

	COPY_FROM_USER(rc,&mask, mask_ptr, sizeof(int));
	if (rc)
		return  -EFAULT;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("wait_events(%s,%d)\n", info->device_name, mask);

	spin_lock_irqsave(&info->lock,flags);

	/* return immediately if state matches requested events */
	get_signals(info);
	s = info->serial_signals;
	events = mask &
		( ((s & SerialSignal_DSR) ? MgslEvent_DsrActive:MgslEvent_DsrInactive) +
 		  ((s & SerialSignal_DCD) ? MgslEvent_DcdActive:MgslEvent_DcdInactive) +
		  ((s & SerialSignal_CTS) ? MgslEvent_CtsActive:MgslEvent_CtsInactive) +
		  ((s & SerialSignal_RI)  ? MgslEvent_RiActive :MgslEvent_RiInactive) );
	if (events) {
		spin_unlock_irqrestore(&info->lock,flags);
		goto exit;
	}

	/* save current irq counts */
	cprev = info->icount;
	oldsigs = info->input_signal_events;

	if ((info->params.mode == MGSL_MODE_HDLC) &&
	    (mask & MgslEvent_ExitHuntMode))
		irq_enable(info, CHA, IRQ_EXITHUNT);

	set_current_state(TASK_INTERRUPTIBLE);
	add_wait_queue(&info->event_wait_q, &wait);

	spin_unlock_irqrestore(&info->lock,flags);


	for(;;) {
		schedule();
		if (signal_pending(current)) {
			rc = -ERESTARTSYS;
			break;
		}

		/* get current irq counts */
		spin_lock_irqsave(&info->lock,flags);
		cnow = info->icount;
		newsigs = info->input_signal_events;
		set_current_state(TASK_INTERRUPTIBLE);
		spin_unlock_irqrestore(&info->lock,flags);

		/* if no change, wait aborted for some reason */
		if (newsigs.dsr_up   == oldsigs.dsr_up   &&
		    newsigs.dsr_down == oldsigs.dsr_down &&
		    newsigs.dcd_up   == oldsigs.dcd_up   &&
		    newsigs.dcd_down == oldsigs.dcd_down &&
		    newsigs.cts_up   == oldsigs.cts_up   &&
		    newsigs.cts_down == oldsigs.cts_down &&
		    newsigs.ri_up    == oldsigs.ri_up    &&
		    newsigs.ri_down  == oldsigs.ri_down  &&
		    cnow.exithunt    == cprev.exithunt   &&
		    cnow.rxidle      == cprev.rxidle) {
			rc = -EIO;
			break;
		}

		events = mask &
			( (newsigs.dsr_up   != oldsigs.dsr_up   ? MgslEvent_DsrActive:0)   +
			  (newsigs.dsr_down != oldsigs.dsr_down ? MgslEvent_DsrInactive:0) +
			  (newsigs.dcd_up   != oldsigs.dcd_up   ? MgslEvent_DcdActive:0)   +
			  (newsigs.dcd_down != oldsigs.dcd_down ? MgslEvent_DcdInactive:0) +
			  (newsigs.cts_up   != oldsigs.cts_up   ? MgslEvent_CtsActive:0)   +
			  (newsigs.cts_down != oldsigs.cts_down ? MgslEvent_CtsInactive:0) +
			  (newsigs.ri_up    != oldsigs.ri_up    ? MgslEvent_RiActive:0)    +
			  (newsigs.ri_down  != oldsigs.ri_down  ? MgslEvent_RiInactive:0)  +
			  (cnow.exithunt    != cprev.exithunt   ? MgslEvent_ExitHuntMode:0) +
			  (cnow.rxidle      != cprev.rxidle     ? MgslEvent_IdleReceived:0) );
		if (events)
			break;

		cprev = cnow;
		oldsigs = newsigs;
	}

	remove_wait_queue(&info->event_wait_q, &wait);
	set_current_state(TASK_RUNNING);

	if (mask & MgslEvent_ExitHuntMode) {
		spin_lock_irqsave(&info->lock,flags);
		if (!waitqueue_active(&info->event_wait_q))
			irq_disable(info, CHA, IRQ_EXITHUNT);
		spin_unlock_irqrestore(&info->lock,flags);
	}
exit:
	if (rc == 0)
		PUT_USER(rc, events, mask_ptr);
	return rc;
}

static int modem_input_wait(MGSLPC_INFO *info,int arg)
{
 	unsigned long flags;
	int rc;
	struct mgsl_icount cprev, cnow;
	DECLARE_WAITQUEUE(wait, current);

	/* save current irq counts */
	spin_lock_irqsave(&info->lock,flags);
	cprev = info->icount;
	add_wait_queue(&info->status_event_wait_q, &wait);
	set_current_state(TASK_INTERRUPTIBLE);
	spin_unlock_irqrestore(&info->lock,flags);

	for(;;) {
		schedule();
		if (signal_pending(current)) {
			rc = -ERESTARTSYS;
			break;
		}

		/* get new irq counts */
		spin_lock_irqsave(&info->lock,flags);
		cnow = info->icount;
		set_current_state(TASK_INTERRUPTIBLE);
		spin_unlock_irqrestore(&info->lock,flags);

		/* if no change, wait aborted for some reason */
		if (cnow.rng == cprev.rng && cnow.dsr == cprev.dsr &&
		    cnow.dcd == cprev.dcd && cnow.cts == cprev.cts) {
			rc = -EIO;
			break;
		}

		/* check for change in caller specified modem input */
		if ((arg & TIOCM_RNG && cnow.rng != cprev.rng) ||
		    (arg & TIOCM_DSR && cnow.dsr != cprev.dsr) ||
		    (arg & TIOCM_CD  && cnow.dcd != cprev.dcd) ||
		    (arg & TIOCM_CTS && cnow.cts != cprev.cts)) {
			rc = 0;
			break;
		}

		cprev = cnow;
	}
	remove_wait_queue(&info->status_event_wait_q, &wait);
	set_current_state(TASK_RUNNING);
	return rc;
}

/* return the state of the serial control and status signals
 */
static int tiocmget(struct tty_struct *tty, struct file *file)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned int result;
 	unsigned long flags;

	spin_lock_irqsave(&info->lock,flags);
 	get_signals(info);
	spin_unlock_irqrestore(&info->lock,flags);

	result = ((info->serial_signals & SerialSignal_RTS) ? TIOCM_RTS:0) +
		((info->serial_signals & SerialSignal_DTR) ? TIOCM_DTR:0) +
		((info->serial_signals & SerialSignal_DCD) ? TIOCM_CAR:0) +
		((info->serial_signals & SerialSignal_RI)  ? TIOCM_RNG:0) +
		((info->serial_signals & SerialSignal_DSR) ? TIOCM_DSR:0) +
		((info->serial_signals & SerialSignal_CTS) ? TIOCM_CTS:0);

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):%s tiocmget() value=%08X\n",
			 __FILE__,__LINE__, info->device_name, result );
	return result;
}

/* set modem control signals (DTR/RTS)
 */
static int tiocmset(struct tty_struct *tty, struct file *file,
		    unsigned int set, unsigned int clear)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
 	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):%s tiocmset(%x,%x)\n",
			__FILE__,__LINE__,info->device_name, set, clear);

	if (set & TIOCM_RTS)
		info->serial_signals |= SerialSignal_RTS;
	if (set & TIOCM_DTR)
		info->serial_signals |= SerialSignal_DTR;
	if (clear & TIOCM_RTS)
		info->serial_signals &= ~SerialSignal_RTS;
	if (clear & TIOCM_DTR)
		info->serial_signals &= ~SerialSignal_DTR;

	spin_lock_irqsave(&info->lock,flags);
 	set_signals(info);
	spin_unlock_irqrestore(&info->lock,flags);

	return 0;
}

/* Set or clear transmit break condition
 *
 * Arguments:		tty		pointer to tty instance data
 *			break_state	-1=set break condition, 0=clear
 */
static int mgslpc_break(struct tty_struct *tty, int break_state)
{
	MGSLPC_INFO * info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_break(%s,%d)\n",
			 __FILE__,__LINE__, info->device_name, break_state);

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_break"))
		return -EINVAL;

	spin_lock_irqsave(&info->lock,flags);
 	if (break_state == -1)
		set_reg_bits(info, CHA+DAFO, BIT6);
	else
		clear_reg_bits(info, CHA+DAFO, BIT6);
	spin_unlock_irqrestore(&info->lock,flags);
	return 0;
}

/* Service an IOCTL request
 *
 * Arguments:
 *
 * 	tty	pointer to tty instance data
 * 	file	pointer to associated file object for device
 * 	cmd	IOCTL command code
 * 	arg	command argument/context
 *
 * Return Value:	0 if success, otherwise error code
 */
static int mgslpc_ioctl(struct tty_struct *tty, struct file * file,
			unsigned int cmd, unsigned long arg)
{
	MGSLPC_INFO * info = (MGSLPC_INFO *)tty->driver_data;
	int error;
	struct mgsl_icount cnow;	/* kernel counter temps */
	struct serial_icounter_struct __user *p_cuser;	/* user space */
	void __user *argp = (void __user *)arg;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_ioctl %s cmd=%08X\n", __FILE__,__LINE__,
			info->device_name, cmd );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_ioctl"))
		return -ENODEV;

	if ((cmd != TIOCGSERIAL) && (cmd != TIOCSSERIAL) &&
	    (cmd != TIOCMIWAIT) && (cmd != TIOCGICOUNT)) {
		if (tty->flags & (1 << TTY_IO_ERROR))
		    return -EIO;
	}

	switch (cmd) {
	case MGSL_IOCGPARAMS:
		return get_params(info, argp);
	case MGSL_IOCSPARAMS:
		return set_params(info, argp, tty);
	case MGSL_IOCGTXIDLE:
		return get_txidle(info, argp);
	case MGSL_IOCSTXIDLE:
		return set_txidle(info, (int)arg);
	case MGSL_IOCGIF:
		return get_interface(info, argp);
	case MGSL_IOCSIF:
		return set_interface(info,(int)arg);
	case MGSL_IOCTXENABLE:
		return set_txenable(info,(int)arg, tty);
	case MGSL_IOCRXENABLE:
		return set_rxenable(info,(int)arg);
	case MGSL_IOCTXABORT:
		return tx_abort(info);
	case MGSL_IOCGSTATS:
		return get_stats(info, argp);
	case MGSL_IOCWAITEVENT:
		return wait_events(info, argp);
	case TIOCMIWAIT:
		return modem_input_wait(info,(int)arg);
	case TIOCGICOUNT:
		spin_lock_irqsave(&info->lock,flags);
		cnow = info->icount;
		spin_unlock_irqrestore(&info->lock,flags);
		p_cuser = argp;
		PUT_USER(error,cnow.cts, &p_cuser->cts);
		if (error) return error;
		PUT_USER(error,cnow.dsr, &p_cuser->dsr);
		if (error) return error;
		PUT_USER(error,cnow.rng, &p_cuser->rng);
		if (error) return error;
		PUT_USER(error,cnow.dcd, &p_cuser->dcd);
		if (error) return error;
		PUT_USER(error,cnow.rx, &p_cuser->rx);
		if (error) return error;
		PUT_USER(error,cnow.tx, &p_cuser->tx);
		if (error) return error;
		PUT_USER(error,cnow.frame, &p_cuser->frame);
		if (error) return error;
		PUT_USER(error,cnow.overrun, &p_cuser->overrun);
		if (error) return error;
		PUT_USER(error,cnow.parity, &p_cuser->parity);
		if (error) return error;
		PUT_USER(error,cnow.brk, &p_cuser->brk);
		if (error) return error;
		PUT_USER(error,cnow.buf_overrun, &p_cuser->buf_overrun);
		if (error) return error;
		return 0;
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

/* Set new termios settings
 *
 * Arguments:
 *
 * 	tty		pointer to tty structure
 * 	termios		pointer to buffer to hold returned old termios
 */
static void mgslpc_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
{
	MGSLPC_INFO *info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_set_termios %s\n", __FILE__,__LINE__,
			tty->driver->name );

	/* just return if nothing has changed */
	if ((tty->termios->c_cflag == old_termios->c_cflag)
	    && (RELEVANT_IFLAG(tty->termios->c_iflag)
		== RELEVANT_IFLAG(old_termios->c_iflag)))
	  return;

	mgslpc_change_params(info, tty);

	/* Handle transition to B0 status */
	if (old_termios->c_cflag & CBAUD &&
	    !(tty->termios->c_cflag & CBAUD)) {
		info->serial_signals &= ~(SerialSignal_RTS + SerialSignal_DTR);
		spin_lock_irqsave(&info->lock,flags);
	 	set_signals(info);
		spin_unlock_irqrestore(&info->lock,flags);
	}

	/* Handle transition away from B0 status */
	if (!(old_termios->c_cflag & CBAUD) &&
	    tty->termios->c_cflag & CBAUD) {
		info->serial_signals |= SerialSignal_DTR;
 		if (!(tty->termios->c_cflag & CRTSCTS) ||
 		    !test_bit(TTY_THROTTLED, &tty->flags)) {
			info->serial_signals |= SerialSignal_RTS;
 		}
		spin_lock_irqsave(&info->lock,flags);
	 	set_signals(info);
		spin_unlock_irqrestore(&info->lock,flags);
	}

	/* Handle turning off CRTSCTS */
	if (old_termios->c_cflag & CRTSCTS &&
	    !(tty->termios->c_cflag & CRTSCTS)) {
		tty->hw_stopped = 0;
		tx_release(tty);
	}
}

static void mgslpc_close(struct tty_struct *tty, struct file * filp)
{
	MGSLPC_INFO * info = (MGSLPC_INFO *)tty->driver_data;
	struct tty_port *port = &info->port;

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_close"))
		return;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_close(%s) entry, count=%d\n",
			 __FILE__,__LINE__, info->device_name, port->count);

	WARN_ON(!port->count);

	if (tty_port_close_start(port, tty, filp) == 0)
		goto cleanup;

 	if (port->flags & ASYNC_INITIALIZED)
 		mgslpc_wait_until_sent(tty, info->timeout);

	mgslpc_flush_buffer(tty);

	tty_ldisc_flush(tty);
	shutdown(info, tty);
	
	tty_port_close_end(port, tty);
	tty_port_tty_set(port, NULL);
cleanup:
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_close(%s) exit, count=%d\n", __FILE__,__LINE__,
			tty->driver->name, port->count);
}

/* Wait until the transmitter is empty.
 */
static void mgslpc_wait_until_sent(struct tty_struct *tty, int timeout)
{
	MGSLPC_INFO * info = (MGSLPC_INFO *)tty->driver_data;
	unsigned long orig_jiffies, char_time;

	if (!info )
		return;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_wait_until_sent(%s) entry\n",
			 __FILE__,__LINE__, info->device_name );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_wait_until_sent"))
		return;

	if (!(info->port.flags & ASYNC_INITIALIZED))
		goto exit;

	orig_jiffies = jiffies;

	/* Set check interval to 1/5 of estimated time to
	 * send a character, and make it at least 1. The check
	 * interval should also be less than the timeout.
	 * Note: use tight timings here to satisfy the NIST-PCTS.
	 */

	if ( info->params.data_rate ) {
	       	char_time = info->timeout/(32 * 5);
		if (!char_time)
			char_time++;
	} else
		char_time = 1;

	if (timeout)
		char_time = min_t(unsigned long, char_time, timeout);

	if (info->params.mode == MGSL_MODE_HDLC) {
		while (info->tx_active) {
			msleep_interruptible(jiffies_to_msecs(char_time));
			if (signal_pending(current))
				break;
			if (timeout && time_after(jiffies, orig_jiffies + timeout))
				break;
		}
	} else {
		while ((info->tx_count || info->tx_active) &&
			info->tx_enabled) {
			msleep_interruptible(jiffies_to_msecs(char_time));
			if (signal_pending(current))
				break;
			if (timeout && time_after(jiffies, orig_jiffies + timeout))
				break;
		}
	}

exit:
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_wait_until_sent(%s) exit\n",
			 __FILE__,__LINE__, info->device_name );
}

/* Called by tty_hangup() when a hangup is signaled.
 * This is the same as closing all open files for the port.
 */
static void mgslpc_hangup(struct tty_struct *tty)
{
	MGSLPC_INFO * info = (MGSLPC_INFO *)tty->driver_data;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_hangup(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_hangup"))
		return;

	mgslpc_flush_buffer(tty);
	shutdown(info, tty);
	tty_port_hangup(&info->port);
}

static int carrier_raised(struct tty_port *port)
{
	MGSLPC_INFO *info = container_of(port, MGSLPC_INFO, port);
	unsigned long flags;

	spin_lock_irqsave(&info->lock,flags);
 	get_signals(info);
	spin_unlock_irqrestore(&info->lock,flags);

	if (info->serial_signals & SerialSignal_DCD)
		return 1;
	return 0;
}

static void dtr_rts(struct tty_port *port, int onoff)
{
	MGSLPC_INFO *info = container_of(port, MGSLPC_INFO, port);
	unsigned long flags;

	spin_lock_irqsave(&info->lock,flags);
	if (onoff)
		info->serial_signals |= SerialSignal_RTS + SerialSignal_DTR;
	else
		info->serial_signals &= ~SerialSignal_RTS + SerialSignal_DTR;
	set_signals(info);
	spin_unlock_irqrestore(&info->lock,flags);
}


static int mgslpc_open(struct tty_struct *tty, struct file * filp)
{
	MGSLPC_INFO	*info;
	struct tty_port *port;
	int 			retval, line;
	unsigned long flags;

	/* verify range of specified line number */
	line = tty->index;
	if ((line < 0) || (line >= mgslpc_device_count)) {
		printk("%s(%d):mgslpc_open with invalid line #%d.\n",
			__FILE__,__LINE__,line);
		return -ENODEV;
	}

	/* find the info structure for the specified line */
	info = mgslpc_device_list;
	while(info && info->line != line)
		info = info->next_device;
	if (mgslpc_paranoia_check(info, tty->name, "mgslpc_open"))
		return -ENODEV;

	port = &info->port;
	tty->driver_data = info;
	tty_port_tty_set(port, tty);

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_open(%s), old ref count = %d\n",
			 __FILE__,__LINE__,tty->driver->name, port->count);

	/* If port is closing, signal caller to try again */
	if (tty_hung_up_p(filp) || port->flags & ASYNC_CLOSING){
		if (port->flags & ASYNC_CLOSING)
			interruptible_sleep_on(&port->close_wait);
		retval = ((port->flags & ASYNC_HUP_NOTIFY) ?
			-EAGAIN : -ERESTARTSYS);
		goto cleanup;
	}

	tty->low_latency = (port->flags & ASYNC_LOW_LATENCY) ? 1 : 0;

	spin_lock_irqsave(&info->netlock, flags);
	if (info->netcount) {
		retval = -EBUSY;
		spin_unlock_irqrestore(&info->netlock, flags);
		goto cleanup;
	}
	spin_lock(&port->lock);
	port->count++;
	spin_unlock(&port->lock);
	spin_unlock_irqrestore(&info->netlock, flags);

	if (port->count == 1) {
		/* 1st open on this device, init hardware */
		retval = startup(info, tty);
		if (retval < 0)
			goto cleanup;
	}

	retval = tty_port_block_til_ready(&info->port, tty, filp);
	if (retval) {
		if (debug_level >= DEBUG_LEVEL_INFO)
			printk("%s(%d):block_til_ready(%s) returned %d\n",
				 __FILE__,__LINE__, info->device_name, retval);
		goto cleanup;
	}

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):mgslpc_open(%s) success\n",
			 __FILE__,__LINE__, info->device_name);
	retval = 0;

cleanup:
	return retval;
}

/*
 * /proc fs routines....
 */

static inline void line_info(struct seq_file *m, MGSLPC_INFO *info)
{
	char	stat_buf[30];
	unsigned long flags;

	seq_printf(m, "%s:io:%04X irq:%d",
		      info->device_name, info->io_base, info->irq_level);

	/* output current serial signal states */
	spin_lock_irqsave(&info->lock,flags);
 	get_signals(info);
	spin_unlock_irqrestore(&info->lock,flags);

	stat_buf[0] = 0;
	stat_buf[1] = 0;
	if (info->serial_signals & SerialSignal_RTS)
		strcat(stat_buf, "|RTS");
	if (info->serial_signals & SerialSignal_CTS)
		strcat(stat_buf, "|CTS");
	if (info->serial_signals & SerialSignal_DTR)
		strcat(stat_buf, "|DTR");
	if (info->serial_signals & SerialSignal_DSR)
		strcat(stat_buf, "|DSR");
	if (info->serial_signals & SerialSignal_DCD)
		strcat(stat_buf, "|CD");
	if (info->serial_signals & SerialSignal_RI)
		strcat(stat_buf, "|RI");

	if (info->params.mode == MGSL_MODE_HDLC) {
		seq_printf(m, " HDLC txok:%d rxok:%d",
			      info->icount.txok, info->icount.rxok);
		if (info->icount.txunder)
			seq_printf(m, " txunder:%d", info->icount.txunder);
		if (info->icount.txabort)
			seq_printf(m, " txabort:%d", info->icount.txabort);
		if (info->icount.rxshort)
			seq_printf(m, " rxshort:%d", info->icount.rxshort);
		if (info->icount.rxlong)
			seq_printf(m, " rxlong:%d", info->icount.rxlong);
		if (info->icount.rxover)
			seq_printf(m, " rxover:%d", info->icount.rxover);
		if (info->icount.rxcrc)
			seq_printf(m, " rxcrc:%d", info->icount.rxcrc);
	} else {
		seq_printf(m, " ASYNC tx:%d rx:%d",
			      info->icount.tx, info->icount.rx);
		if (info->icount.frame)
			seq_printf(m, " fe:%d", info->icount.frame);
		if (info->icount.parity)
			seq_printf(m, " pe:%d", info->icount.parity);
		if (info->icount.brk)
			seq_printf(m, " brk:%d", info->icount.brk);
		if (info->icount.overrun)
			seq_printf(m, " oe:%d", info->icount.overrun);
	}

	/* Append serial signal status to end */
	seq_printf(m, " %s\n", stat_buf+1);

	seq_printf(m, "txactive=%d bh_req=%d bh_run=%d pending_bh=%x\n",
		       info->tx_active,info->bh_requested,info->bh_running,
		       info->pending_bh);
}

/* Called to print information about devices
 */
static int mgslpc_proc_show(struct seq_file *m, void *v)
{
	MGSLPC_INFO *info;

	seq_printf(m, "synclink driver:%s\n", driver_version);

	info = mgslpc_device_list;
	while( info ) {
		line_info(m, info);
		info = info->next_device;
	}
	return 0;
}

static int mgslpc_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgslpc_proc_show, NULL);
}

static const struct file_operations mgslpc_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= mgslpc_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int rx_alloc_buffers(MGSLPC_INFO *info)
{
	/* each buffer has header and data */
	info->rx_buf_size = sizeof(RXBUF) + info->max_frame_size;

	/* calculate total allocation size for 8 buffers */
	info->rx_buf_total_size = info->rx_buf_size * 8;

	/* limit total allocated memory */
	if (info->rx_buf_total_size > 0x10000)
		info->rx_buf_total_size = 0x10000;

	/* calculate number of buffers */
	info->rx_buf_count = info->rx_buf_total_size / info->rx_buf_size;

	info->rx_buf = kmalloc(info->rx_buf_total_size, GFP_KERNEL);
	if (info->rx_buf == NULL)
		return -ENOMEM;

	rx_reset_buffers(info);
	return 0;
}

static void rx_free_buffers(MGSLPC_INFO *info)
{
	kfree(info->rx_buf);
	info->rx_buf = NULL;
}

static int claim_resources(MGSLPC_INFO *info)
{
	if (rx_alloc_buffers(info) < 0 ) {
		printk( "Cant allocate rx buffer %s\n", info->device_name);
		release_resources(info);
		return -ENODEV;
	}
	return 0;
}

static void release_resources(MGSLPC_INFO *info)
{
	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("release_resources(%s)\n", info->device_name);
	rx_free_buffers(info);
}

/* Add the specified device instance data structure to the
 * global linked list of devices and increment the device count.
 *
 * Arguments:		info	pointer to device instance data
 */
static void mgslpc_add_device(MGSLPC_INFO *info)
{
	info->next_device = NULL;
	info->line = mgslpc_device_count;
	sprintf(info->device_name,"ttySLP%d",info->line);

	if (info->line < MAX_DEVICE_COUNT) {
		if (maxframe[info->line])
			info->max_frame_size = maxframe[info->line];
	}

	mgslpc_device_count++;

	if (!mgslpc_device_list)
		mgslpc_device_list = info;
	else {
		MGSLPC_INFO *current_dev = mgslpc_device_list;
		while( current_dev->next_device )
			current_dev = current_dev->next_device;
		current_dev->next_device = info;
	}

	if (info->max_frame_size < 4096)
		info->max_frame_size = 4096;
	else if (info->max_frame_size > 65535)
		info->max_frame_size = 65535;

	printk( "SyncLink PC Card %s:IO=%04X IRQ=%d\n",
		info->device_name, info->io_base, info->irq_level);

#if SYNCLINK_GENERIC_HDLC
	heast one
 *				context switch.
 *
 * @p must not be current.
 */
void wait_task_context_switch(struct task_struct *p)
{
	unsigned long nvcsw, nivcsw, flags;
	int running;
	struct rq *rq;

	nvcsw	= p->nvcsw;
	nivcsw	= p->nivcsw;
	for (;;) {
		/*
		 * The runqueue is assigned before the actual context
		 * switch. We need to take the runqueue lock.
		 *
		 * We could check initially without the lock but it is
		 * very likely that we need to take the lock in every
		 * iteration.
		 */
		rq = task_rq_lock(p, &flags);
		running = task_running(rq, p);
		task_rq_unlock(rq, &flags);

		if (likely(!running))
			break;
		/*
		 * The switch count is incremented before the actual
		 * context switch. We thus wait for two switches to be
		 * sure at least one completed.
		 */
		if ((p->nvcsw - nvcsw) > 1)
			break;
		if ((p->nivcsw - nivcsw) > 1)
			break;

		cpu_relax();
	}
}

/*
 * wait_task_inactive - wait for a thread to unschedule.
 *
 * If @match_state is nonzero, it's the @p->state value just checked and
 * not expected to change.  If it changes, i.e. @p might have woken up,
 * then return zero.  When we succeed in waiting for @p to be off its CPU,
 * we return a positive number (its total switch count).  If a second call
 * a short while later returns the same number, the caller can be sure that
 * @p has remained unscheduled the whole time.
 *
 * The caller must ensure that the task *will* unschedule sometime soon,
 * else this function might spin for a *long* time. This function can't
 * be called with interrupts off, or it may introduce deadlock with
 * smp_call_function() if an IPI is sent by the same process we are
 * waiting to become inactive.
 */
unsigned long wait_task_inactive(struct task_struct *p, long match_state)
{
	unsigned long flags;
	int running, on_rq;
	unsigned long ncsw;
	struct rq *rq;

	for (;;) {
		/*
		 * We do the initial early heuristics without holding
		 * any task-queue locks at all. We'll only try to get
		 * the runqueue lock when things look like they will
		 * work out!
		 */
		rq = task_rq(p);

		/*
		 * If the task is actively running on another CPU
		 * still, just relax and busy-wait without holding
		 * any locks.
		 *
		 * NOTE! Since we don't hold any locks, it's not
		 * even sure that "rq" stays as the right runqueue!
		 * But we don't care, since "task_running()" will
		 * return false if the runqueue has changed and p
		 * is actually now running somewhere else!
		 */
		while (task_running(rq, p)) {
			if (match_state && unlikely(p->state != match_state))
				return 0;
			cpu_relax();
		}

		/*
		 * Ok, time to look more closely! We need the rq
		 * lock now, to be *sure*. If we're wrong, we'll
		 * just go back and repeat.
		 */
		rq = task_rq_lock(p, &flags);
		trace_sched_wait_task(rq, p);
		running = task_running(rq, p);
		on_rq = p->se.on_rq;
		ncsw = 0;
		if (!match_state || p->state == match_state)
			ncsw = p->nvcsw | LONG_MIN; /* sets MSB */
		task_rq_unlock(rq, &flags);

		/*
		 * If it changed from the expected state, bail out now.
		 */
		if (unlikely(!ncsw))
			break;

		/*
		 * Was it really running after all now that we
		 * checked with the proper locks actually held?
		 *
		 * Oops. Go back and try again..
		 */
		if (unlikely(running)) {
			cpu_relax();
			continue;
		}

		/*
		 * It's not enough that it's not actively running,
		 * it must be off the runqueue _entirely_, and not
		 * preempted!
		 *
		 * So if it was still runnable (but just not actively
		 * running right now), it's preempted, and we should
		 * yield - it could be a while.
		 */
		if (unlikely(on_rq)) {
			schedule_timeout_uninterruptible(1);
			continue;
		}

		/*
		 * Ahh, all good. It wasn't running, and it wasn't
		 * runnable, which means that it will never become
		 * running in the future either. We're all done!
		 */
		break;
	}

	return ncsw;
}

/***
 * kick_process - kick a running thread to enter/exit the kernel
 * @p: the to-be-kicked thread
 *
 * Cause a process which is running on another CPU to enter
 * kernel-mode, without any delay. (to get signals handled.)
 *
 * NOTE: this o, (unsigned char) (channel + BGR),
				  (unsigned char) ((M << 6) + N));
		val = read_reg(info, (unsigned char) (channel + CCR2)) & 0x3f;
		val |= ((M << 4) & 0xc0);
		write_reg(info, (unsigned char) (channel + CCR2), val);
	}
}

/* Enabled the AUX clock output at the specified frequency.
 */
static void enable_auxclk(MGSLPC_INFO *info)
{
	unsigned char val;

	/* MODE
	 *
	 * 07..06  MDS[1..0] 10 = transparent HDLC mode
	 * 05      ADM Address Mode, 0 = no addr recognition
	 * 04      TMD Timer Mode, 0 = external
	 * 03      RAC Receiver Active, 0 = inactive
	 * 02      RTS 0=RTS active during xmit, 1=RTS always active
	 * 01      TRS Timer Resolution, 1=512
	 * 00      TLP Test Loop, 0 = no loop
	 *
	 * 1000 0010
	 */
	val = 0x82;

	/* channel B RTS is used to enable AUXCLK driver on SP505 */
	if (info->params.mode == MGSL_MODE_HDLC && info->params.clock_speed)
		val |= BIT2;
	write_reg(info, CHB + MODE, val);

	/* CCR0
	 *
	 * 07      PU Power Up, 1=active, 0=power down
	 * 06      MCE Master Clock Enable, 1=enabled
	 * 05      Reserved, 0
	 * 04..02  SC[2..0] Encoding
	 * 01..00  SM[1..0] Serial Mode, 00=HDLC
	 *
	 * 11000000
	 */
	write_reg(info, CHB + CCR0, 0xc0);

	/* CCR1
	 *
	 * 07      SFLG Shared Flag, 0 = disable shared flags
	 * 06      GALP Go Active On Loop, 0 = not used
	 * 05      GLP Go On Loop, 0 = not used
	 * 04      ODS Output Driver Select, 1=TxD is push-pull output
	 * 03      ITF Interframe Time Fill, 0=mark, 1=flag
	 * 02..00  CM[2..0] Clock Mode
	 *
	 * 0001 0111
	 */
	write_reg(info, CHB + CCR1, 0x17);

	/* CCR2 (Channel B)
	 *
	 * 07..06  BGR[9..8] Baud rate bits 9..8
	 * 05      BDF Baud rate divisor factor, 0=1, 1=BGR value
	 * 04      SSEL Clock source select, 1=submode b
	 * 03      TOE 0=TxCLK is input, 1=TxCLK is output
	 * 02      RWX Read/Write Exchange 0=disabled
	 * 01      C32, CRC select, 0=CRC-16, 1=CRC-32
	 * 00      DIV, data inversion 0=disabled, 1=enabled
	 *
	 * 0011 1000
	 */
	if (info->params.mode == MGSL_MODE_HDLC && info->params.clock_speed)
		write_reg(info, CHB + CCR2, 0x38);
	else
		write_reg(info, CHB + CCR2, 0x30);

	/* CCR4
	 *
	 * 07      MCK4 Master Clock Divide by 4, 1=enabled
	 * 06      EBRG Enhanced Baud Rate Generator Mode, 1=enabled
	 * 05      TST1 Test Pin, 0=normal operation
	 * 04      ICD Ivert Carrier Detect, 1=enabled (active low)
	 * 03..02  Reserved, must be 0
	 * 01..00  RFT[1..0] RxFIFO Threshold 00=32 bytes
	 *
	 * 0101 0000
	 */
	write_reg(info, CHB + CCR4, 0x50);

	/* if auxclk not enabled, set internal BRG so
	 * CTS transitions can be detected (requires TxC)
	 */
	if (info->params.mode == MGSL_MODE_HDLC && info->params.clock_speed)
		mgslpc_set_rate(info, CHB, info->params.clock_speed);
	else
		mgslpc_set_rate(info, CHB, 921600);
}

static void loopback_enable(MGSLPC_INFO *info)
{
	unsigned char val;

	/* CCR1:02..00  CM[2..0] Clock Mode = 111 (clock mode 7) */
	val = read_reg(info, CHA + CCR1) | (BIT2 + BIT1 + BIT0);
	write_reg(info, CHA + CCR1, val);

	/* CCR2:04 SSEL Clock source select, 1=submode b */
	val = read_reg(info, CHA + CCR2) | (BIT4 + BIT5);
	write_reg(info, CHA + CCR2, val);

	/* set LinkSpeed if available, otherwise default to 2Mbps */
	if (info->params.clock_speed)
		mgslpc_set_rate(info, CHA, info->params.clock_speed);
	else
		mgslpc_set_rate(info, CHA, 1843200);

	/* MODE:00 TLP Test Loop, 1=loopback enabled */
	val = read_reg(info, CHA + MODE) | BIT0;
	write_reg(info, CHA + MODE, val);
}

static void hdlc_mode(MGSLPC_INFO *info)
{
	unsigned char val;
	unsigned char clkmode, clksubmode;

	/* disable all interrupts */
	irq_disable(info, CHA, 0xffff);
	irq_disable(info, CHB, 0xffff);
	port_irq_disable(info, 0xff);

	/* assume clock mode 0a, rcv=RxC xmt=TxC */
	clkmode = clksubmode = 0;
	if (info->params.flags & HDLC_FLAG_RXC_DPLL
	    && info->params.flags & HDLC_FLAG_TXC_DPLL) {
		/* clock mode 7a, rcv = DPLL, xmt = DPLL */
		clkmode = 7;
	} else if (info->params.flags & HDLC_FLAG_RXC_BRG
		 && info->params.flags & HDLC_FLAG_TXC_BRG) {
		/* clock mode 7b, rcv = BRG, xmt = BRG */
		clkmode = 7;
		clksubmode = 1;
	} else if (info->params.flags & HDLC_FLAG_RXC_DPLL) {
		if (info->params.flags & HDLC_FLAG_TXC_BRG) {
			/* clock mode 6b, rcv = DPLL, xmt = BRG/16 */
			clkmode = 6;
			clksubmode = 1;
		} else {
			/* clock mode 6a, rcv = DPLL, xmt = TxC */
			clkmode = 6;
		}
	} else if (info->params.flags & HDLC_FLAG_TXC_BRG) {
		/* clock mode 0b, rcv = RxC, xmt = BRG */
		clksubmode = 1;
	}

	/* MODE
	 *
	 * 07..06  MDS[1..0] 10 = transparent HDLC mode
	 * 05      ADM Address Mode, 0 = no addr recognition
	 * 04      TMD Timer Mode, 0 = external
	 * 03      RAC Receiver Active, 0 = inactive
	 * 02      RTS 0=RTS active during xmit, 1=RTS always active
	 * 01      TRS Timer Resolution, 1=512
	 * 00      TLP Test Loop, 0 = no loop
	 *
	 * 1000 0010
	 */
	val = 0x82;
	if (info->params.loopback)
		val |= BIT0;

	/* preserve RTS state */
	if (info->serial_signals & SerialSignal_RTS)
		val |= BIT2;
	write_reg(info, CHA + MODE, val);

	/* CCR0
	 *
	 * 07      PU Power Up, 1=active, 0=power down
	 * 06      MCE Master Clock Enable, 1=enabled
	 * 05      Reserved, 0
	 * 04..02  SC[2..0] Encoding
	 * 01..00  SM[1..0] Serial Mode, 00=HDLC
	 *
	 * 11000000
	 */
	val = 0xc0;
	switch (info->params.encoding)
	{
	case HDLC_ENCODING_NRZI:
		val |= BIT3;
		break;
	case HDLC_ENCODING_BIPHASE_SPACE:
		val |= BIT4;
		break;		// FM0
	case HDLC_ENCODING_BIPHASE_MARK:
		val |= BIT4 + BIT2;
		break;		// FM1
	case HDLC_ENCODING_BIPHASE_LEVEL:
		val |= BIT4 + BIT3;
		break;		// Manchester
	}
	write_reg(info, CHA + CCR0, val);

	/* CCR1
	 *
	 * 07      SFLG Shared Flag, 0 = disable shared flags
	 * 06      GALP Go Active On Loop, 0 = not used
	 * 05      GLP Go On Loop, 0 = not used
	 * 04      ODS Output Driver Select, 1=TxD is push-pull output
	 * 03      ITF Interframe Time Fill, 0=mark, 1=flag
	 * 02..00  CM[2..0] Clock Mode
	 *
	 * 0001 0000
	 */
	val = 0x10 + clkmode;
	write_reg(info, CHA + CCR1, val);

	/* CCR2
	 *
	 * 07..06  BGR[9..8] Baud rate bits 9..8
	 * 05      BDF Baud rate divisor factor, 0=1, 1=BGR value
	 * 04      SSEL Clock source select, 1=submode b
	 * 03      TOE 0=TxCLK is input, 0=TxCLK is input
	 * 02      RWX Read/Write Exchange 0=disabled
	 * 01      C32, CRC select, 0=CRC-16, 1=CRC-32
	 * 00      DIV, data inversion 0=disabled, 1=enabled
	 *
	 * 0000 0000
	 */
	val = 0x00;
	if (clkmode == 2 || clkmode == 3 || clkmode == 6
	    || clkmode == 7 || (clkmode == 0 && clksubmode == 1))
		val |= BIT5;
	if (clksubmode)
		val |= BIT4;
	if (info->params.crc_type == HDLC_CRC_32_CCITT)
		val |= BIT1;
	if (info->params.encoding == HDLC_ENCODING_NRZB)
		val |= BIT0;
	write_reg(info, CHA + CCR2, val);

	/* CCR3
	 *
	 * 07..06  PRE[1..0] Preamble count 00=1, 01=2, 10=4, 11=8
	 * 05      EPT Enable preamble transmission, 1=enabled
	 * 04      RADD Receive address pushed to FIFO, 0=disabled
	 * 03      CRL CRC Reset Level, 0=FFFF
	 * 02      RCRC Rx CRC 0=On 1=Off
	 * 01      TCRC Tx CRC 0=On 1=Off
	 * 00      PSD DPLL Phase Shift Disable
	 *
	 * 0000 0000
	 */
	val = 0x00;
	if (info->params.crc_type == HDLC_CRC_NONE)
		val |= BIT2 + BIT1;
	if (info->params.preamble != HDLC_PREAMBLE_PATTERN_NONE)
		val |= BIT5;
	switch (info->params.preamble_length)
	{
	case HDLC_PREAMBLE_LENGTH_16BITS:
		val |= BIT6;
		break;
	case HDLC_PREAMBLE_LENGTH_32BITS:
		val |= BIT6;
		break;
	case HDLC_PREAMBLE_LENGTH_64BITS:
		val |= BIT7 + BIT6;
		break;
	}
	write_reg(info, CHA + CCR3, val);

	/* PRE - Preamble pattern */
	val = 0;
	switch (info->params.preamble)
	{
	case HDLC_PREAMBLE_PATTERN_FLAGS: val = 0x7e; break;
	case HDLC_PREAMBLE_PATTERN_10:    val = 0xaa; break;
	case HDLC_PREAMBLE_PATTERN_01:    val = 0x55; break;
	case HDLC_PREAMBLE_PATTERN_ONES:  val = 0xff; break;
	}
	write_reg(info, CHA + PRE, val);

	/* CCR4
	 *
	 * 07      MCK4 Master Clock Divide by 4, 1=enabled
	 * 06      EBRG Enhanced Baud Rate Generator Mode, 1=enabled
	 * 05      TST1 Test Pin, 0=normal operation
	 * 04      ICD Ivert Carrier Detect, 1=enabled (active low)
	 * 03..02  Reserved, must be 0
	 * 01..00  RFT[1..0] RxFIFO Threshold 00=32 bytes
	 *
	 * 0101 0000
	 */
	val = 0x50;
	write_reg(info, CHA + CCR4, val);
	if (info->params.flags & HDLC_FLAG_RXC_DPLL)
		mgslpc_set_rate(info, CHA, info->params.clock_speed * 16);
	else
		mgslpc_set_rate(info, CHA, info->params.clock_speed);

	/* RLCR Receive length check register
	 *
	 * 7     1=enable receive length check
	 * 6..0  Max frame length = (RL + 1) * 32
	 */
	write_reg(info, CHA + RLCR, 0);

	/* XBCH Transmit Byte Count High
	 *
	 * 07      DMA mode, 0 = interrupt driven
	 * 06      NRM, 0=ABM (ignored)
	 * 05      CAS Carrier Auto Start
	 * 04      XC Transmit Continuously (ignored)
	 * 03..00  XBC[10..8] Transmit byte count bits 10..8
	 *
	 * 0000 0000
	 */
	val = 0x00;
	if (info->params.flags & HDLC_FLAG_AUTO_DCD)
		val |= BIT5;
	write_reg(info, CHA + XBCH, val);
	enable_auxclk(info);
	if (info->params.loopback || info->testing_irq)
		loopback_enable(info);
	if (info->params.flags & HDLC_FLAG_AUTO_CTS)
	{
		irq_enable(info, CHB, IRQ_CTS);
		/* PVR[3] 1=AUTO CTS active */
		set_reg_bits(info, CHA + PVR, BIT3);
	} else
		clear_reg_bits(info, CHA + PVR, BIT3);

	irq_enable(info, CHA,
			 IRQ_RXEOM + IRQ_RXFIFO + IRQ_ALLSENT +
			 IRQ_UNDERRUN + IRQ_TXFIFO);
	issue_command(info, CHA, CMD_TXRESET + CMD_RXRESET);
	wait_command_complete(info, CHA);
	read_reg16(info, CHA + ISR);	/* clear pending IRQs */

	/* Master clock mode enabled above to allow reset commands
	 * to complete even if no data clocks are present.
	 *
	 * Disable master clock mode for normal communications because
	 * V3.2 of the ESCC2 has a bug that prevents the transmit all sent
	 * IRQ when in master clock mode.
	 *
	 * Leave master clock mode enabled for IRQ test because the
	 * timer IRQ used by the test can only happen in master clock mode.
	 */
	if (!info->testing_irq)
		clear_reg_bits(info, CHA + CCR0, BIT6);

	tx_set_idle(info);

	tx_stop(info);
	rx_stop(info);
}

static void rx_stop(MGSLPC_INFO *info)
{
	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):rx_stop(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	/* MODE:03 RAC Receiver Active, 0=inactive */
	clear_reg_bits(info, CHA + MODE, BIT3);

	info->rx_enabled = false;
	info->rx_overflow = false;
}

static void rx_start(MGSLPC_INFO *info)
{
	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):rx_start(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	rx_reset_buffers(info);
	info->rx_enabled = false;
	info->rx_overflow = false;

	/* MODE:03 RAC Receiver Active, 1=active */
	set_reg_bits(info, CHA + MODE, BIT3);

	info->rx_enabled = true;
}

static void tx_start(MGSLPC_INFO *info, struct tty_struct *tty)
{
	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):tx_start(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	if (info->tx_count) {
		/* If auto RTS enabled and RTS is inactive, then assert */
		/* RTS and set a flag indicating that the driver should */
		/* negate RTS when the transmission completes. */
		info->drop_rts_on_tx_done = false;

		if (info->params.flags & HDLC_FLAG_AUTO_RTS) {
			get_signals(info);
			if (!(info->serial_signals & SerialSignal_RTS)) {
				info->serial_signals |= SerialSignal_RTS;
				set_signals(info);
				info->drop_rts_on_tx_done = true;
			}
		}

		if (info->params.mode == MGSL_MODE_ASYNC) {
			if (!info->tx_active) {
				info->tx_active = true;
				tx_ready(info, tty);
			}
		} else {
			info->tx_active = true;
			tx_ready(info, tty);
			mod_timer(&info->tx_timer, jiffies +
					msecs_to_jiffies(5000));
		}
	}

	if (!info->tx_enabled)
		info->tx_enabled = true;
}

static void tx_stop(MGSLPC_INFO *info)
{
	if (debug_level >= DEBUG_LEVEL_ISR)
		printk("%s(%d):tx_stop(%s)\n",
			 __FILE__,__LINE__, info->device_name );

	del_timer(&info->tx_timer);

	info->tx_enabled = false;
	info->tx_active = false;
}

/* Reset the adapter to a known state and prepare it for further use.
 */
static void reset_device(MGSLPC_INFO *info)
{
	/* power up both channels (set BIT7) */
	write_reg(info, CHA + CCR0, 0x80);
	write_reg(info, CHB + CCR0, 0x80);
	write_reg(info, CHA + MODE, 0);
	write_reg(info, CHB + MODE, 0);

	/* disable all interrupts */
	irq_disable(info, CHA, 0xffff);
	irq_disable(info, CHB, 0xffff);
	port_irq_disable(info, 0xff);

	/* PCR Port Configuration Register
	 *
	 * 07..04  DEC[3..0] Serial I/F select outputs
	 * 03      output, 1=AUTO CTS control enabled
	 * 02      RI Ring Indicator input 0=active
	 * 01      DSR input 0=active
	 * 00      DTR output 0=active
	 *
	 * 0000 0110
	 */
	write_reg(info, PCR, 0x06);

	/* PVR Port Value Register
	 *
	 * 07..04  DEC[3..0] Serial I/F select (0000=disabled)
	 * 03      AUTO CTS output 1=enabled
	 * 02      RI Ring Indicator input
	 * 01      DSR input
	 * 00      DTR output (1=inactive)
	 *
	 * 0000 0001
	 */
//	write_reg(info, PVR, PVR_DTR);

	/* IPC Interrupt Port Configuration
	 *
	 * 07      VIS 1=Masked interrupts visible
	 * 06..05  Reserved, 0
	 * 04..03  SLA Slave address, 00 ignored
	 * 02      CASM Cascading Mode, 1=daisy chain
	 * 01..00  IC[1..0] Interrupt Config, 01=push-pull output, active low
	 *
	 * 0000 0101
	 */
	write_reg(info, IPC, 0x05);
}

static void async_mode(MGSLPC_INFO *info)
{
	unsigned char val;

	/* disable all interrupts */
	irq_disable(info, CHA, 0xffff);
	irq_disable(info, CHB, 0xffff);
	port_irq_disable(info, 0xff);

	/* MODE
	 *
	 * 07      Reserved, 0
	 * 06      FRTS RTS State, 0=active
	 * 05      FCTS Flow Control on CTS
	 * 04      FLON Flow Control Enable
	 * 03      RAC Receiver Active, 0 = inactive
	 * 02      RTS 0=Auto RTS, 1=manual RTS
	 * 01      TRS Timer Resolution, 1=512
	 * 00      TLP Test Loop, 0 = no loop
	 *
	 * 0000 0110
	 */
	val = 0x06;
	if (info->params.loopback)
		val |= BIT0;

	/* preserve RTS state */
	if (!(info->serial_signals & SerialSignal_RTS))
		val |= BIT6;
	write_reg(info, CHA + MODE, val);

	/* CCR0
	 *
	 * 07      PU Power Up, 1=active, 0=power down
	 * 06      MCE Master Clock Enable, 1=enabled
	 * 05      Reserved, 0
	 * 04..02  SC[2..0] Encoding, 000=NRZ
	 * 01..00  SM[1..0] Serial Mode, 11=Async
	 *
	 * 1000 0011
	 */
	write_reg(info, CHA + CCR0, 0x83);

	/* CCR1
	 *
	 * 07..05  Reserved, 0
	 * 04      ODS Output Driver Select, 1=TxD is push-pull output
	 * 03      BCR Bit Clock Rate, 1=16x
	 * 02..00  CM[2..0] Clock Mode, 111=BRG
	 *
	 * 0001 1111
	 */
	write_reg(info, CHA + CCR1, 0x1f);

	/* CCR2 (channel A)
	 *
	 * 07..06  BGR[9..8] Baud rate bits 9..8
	 * 05      BDF Baud rate divisor factor, 0=1, 1=BGR value
	 * 04      SSEL Clock source select, 1=submode b
	 * 03      TOE 0=TxCLK is input, 0=TxCLK is input
	 * 02      RWX Read/Write Exchange 0=disabled
	 * 01      Reserved, 0
	 * 00      DIV, data inversion 0=disabled, 1=enabled
	 *
	 * 0001 0000
	 */
	write_reg(info, CHA + CCR2, 0x10);

	/* CCR3
	 *
	 * 07..01  Reserved, 0
	 * 00      PSD DPLL Phase Shift Disable
	 *
	 * 0000 0000
	 */
	write_reg(info, CHA + CCR3, 0);

	/* CCR4
	 *
	 * 07      MCK4 Master Clock Divide by 4, 1=enabled
	 * 06      EBRG Enhanced Baud Rate Generator Mode, 1=enabled
	 * 05      TST1 Test Pin, 0=normal operation
	 * 04      ICD Ivert Carrier Detect, 1=enabled (active low)
	 * 03..00  Reserved, must be 0
	 *
	 * 0101 0000
	 */
	write_reg(info, CHA + CCR4, 0x50);
	mgslpc_set_rate(info, CHA, info->params.data_rate * 16);

	/* DAFO Data Format
	 *
	 * 07      Reserved, 0
	 * 06      XBRK transmit break, 0=normal operation
	 * 05      Stop bits (0=1, 1=2)
	 * 04..03  PAR[1..0] Parity (01=odd, 10=even)
	 * 02      PAREN Parity Enable
	 * 01..00  CHL[1..0] Character Length (00=8, 01=7)
	 *
	 */
	val = 0x00;
	if (info->params.data_bits != 8)
		val |= BIT0;	/* 7 bits */
	if (info->params.stop_bits != 1)
		val |= BIT5;
	if (info->params.parity != ASYNC_PARITY_NONE)
	{
		val |= BIT2;	/* Parity enable */
		if (info->params.parity == ASYNC_PARITY_ODD)
			val |= BIT3;
		else
			val |= BIT4;
	}
	write_reg(info, CHA + DAFO, val);

	/* RFC Rx FIFO Control
	 *
	 * 07      Reserved, 0
	 * 06      DPS, 1=parity bit not stored in data byte
	 * 05      DXS, 0=all data stored in FIFO (including XON/XOFF)
	 * 04      RFDF Rx FIFO Data Format, 1=status byte stored in FIFO
	 * 03..02  RFTH[1..0], rx threshold, 11=16 status + 16 data byte
	 * 01      Reserved, 0
	 * 00      TCDE Terminate Char Detect Enable, 0=disabled
	 *
	 * 0101 1100
	 */
	write_reg(info, CHA + RFC, 0x5c);

	/* RLCR Receive length check register
	 *
	 * Max frame length = (RL + 1) * 32
	 */
	write_reg(info, CHA + RLCR, 0);

	/* XBCH Transmit Byte Count High
	 *
	 * 07      DMA mode, 0 = interrupt driven
	 * 06      NRM, 0=ABM (ignored)
	 * 05      CAS Carrier Auto Start
	 * 04      XC Transmit Continuously (ignored)
	 * 03..00  XBC[10..8] Transmit byte count bits 10..8
	 *
	 * 0000 0000
	 */
	val = 0x00;
	if (info->params.flags & HDLC_FLAG_AUTO_DCD)
		val |= BIT5;
	write_reg(info, CHA + XBCH, val);
	if (info->params.flags & HDLC_FLAG_AUTO_CTS)
		irq_enable(info, CHA, IRQ_CTS);

	/* MODE:03 RAC Receiver Active, 1=active */
	set_reg_bits(info, CHA + MODE, BIT3);
	enable_auxclk(info);
	if (info->params.flags & HDLC_FLAG_AUTO_CTS) {
		irq_enable(info, CHB, IRQ_CTS);
		/* PVR[3] 1=AUTO CTS active */
		set_reg_bits(info, CHA + PVR, BIT3);
	} else
		clear_reg_bits(info, CHA + PVR, BIT3);
	irq_enable(info, CHA,
			  IRQ_RXEOM + IRQ_RXFIFO + IRQ_BREAK_ON + IRQ_RXTIME +
			  IRQ_ALLSENT + IRQ_TXFIFO);
	issue_command(info, CHA, CMD_TXRESET + CMD_RXRESET);
	wait_command_complete(info, CHA);
	read_reg16(info, CHA + ISR);	/* clear pending IRQs */
}

/* Set the HDLC idle mode for the transmitter.
 */
static void tx_set_idle(MGSLPC_INFO *info)
{
	/* Note: ESCC2 only supports flags and one idle modes */
	if (info->idle_mode == HDLC_TXIDLE_FLAGS)
		set_reg_bits(info, CHA + CCR1, BIT3);
	else
		clear_reg_bits(info, CHA + CCR1, BIT3);
}

/* get state of the V24 status (input) signals.
 */
static void get_signals(MGSLPC_INFO *info)
{
	unsigned char status = 0;

	/* preserve DTR and RTS */
	info->serial_signals &= SerialSignal_DTR + SerialSignal_RTS;

	if (read_reg(info, CHB + VSTR) & BIT7)
		info->serial_signals |= SerialSignal_DCD;
	if (read_reg(info, CHB + STAR) & BIT1)
		info->serial_signals |= SerialSignal_CTS;

	status = read_reg(info, CHA + PVR);
	if (!(status & PVR_RI))
		info->serial_signals |= SerialSignal_RI;
	if (!(status & PVR_DSR))
		info->serial_signals |= SerialSignal_DSR;
}

/* Set the state of DTR and RTS based on contents of
 * serial_signals member of device extension.
 */
static void set_signals(MGSLPC_INFO *info)
{
	unsigned char val;

	val = read_reg(info, CHA + MODE);
	if (info->params.mode == MGSL_MODE_ASYNC) {
		if (info->serial_signals & SerialSignal_RTS)
			val &= ~BIT6;
		else
			val |= BIT6;
	} else {
		if (info->serial_signals & SerialSignal_RTS)
			val |= BIT2;
		else
			val &= ~BIT2;
	}
	write_reg(info, CHA + MODE, val);

	if (info->serial_signals & SerialSignal_DTR)
		clear_reg_bits(info, CHA + PVR, PVR_DTR);
	else
		set_reg_bits(info, CHA + PVR, PVR_DTR);
}

static void rx_reset_buffers(MGSLPC_INFO *info)
{
	RXBUF *buf;
	int i;

	info->rx_put = 0;
	info->rx_get = 0;
	info->rx_frame_count = 0;
	for (i=0 ; i < info->rx_buf_count ; i++) {
		buf = (RXBUF*)(info->rx_buf + (i * info->rx_buf_size));
		buf->status = buf->count = 0;
	}
}

/* Attempt to return a received HDLC frame
 * Only frames received without errors are returned.
 *
 * Returns true if frame returned, otherwise false
 */
static bool rx_get_frame(MGSLPC_INFO *info, struct tty_struct *tty)
{
	unsigned short status;
	RXBUF *buf;
	unsigned int framesize = 0;
	unsigned long flags;
	bool return_frame = false;

	if (info->rx_frame_count == 0)
		return false;

	buf = (RXBUF*)(info->rx_buf + (info->rx_get * info->rx_buf_size));

	status = buf->status;

	/* 07  VFR  1=valid frame
	 * 06  RDO  1=data overrun
	 * 05  CRC  1=OK, 0=error
	 * 04  RAB  1=frame aborted
	 */
	if ((status & 0xf0) != 0xA0) {
		if (!(status & BIT7) || (status & BIT4))
			info->icount.rxabort++;
		else if (status & BIT6)
			info->icount.rxover++;
		else if (!(status & BIT5)) {
			info->icount.rxcrc++;
			if (info->params.crc_type & HDLC_CRC_RETURN_EX)
				return_frame = true;
		}
		framesize = 0;
#if SYNCLINK_GENERIC_HDLC
		{
			info->netdev->stats.rx_errors++;
			info->netdev->stats.rx_frame_errors++;
		}
#endif
	} else
		return_frame = true;

	if (return_frame)
		framesize = buf->count;

	if (debug_level >= DEBUG_LEVEL_BH)
		printk("%s(%d):rx_get_frame(%s) status=%04X size=%d\n",
			__FILE__,__LINE__,info->device_name,status,framesize);

	if (debug_level >= DEBUG_LEVEL_DATA)
		trace_block(info, buf->data, framesize, 0);

	if (framesize) {
		if ((info->params.crc_type & HDLC_CRC_RETURN_EX &&
		      framesize+1 > info->max_frame_size) ||
		    framesize > info->max_frame_size)
			info->icount.rxlong++;
		else {
			if (status & BIT5)
				info->icount.rxok++;

			if (info->params.crc_type & HDLC_CRC_RETURN_EX) {
				*(buf->data + framesize) = status & BIT5 ? RX_OK:RX_CRC_ERROR;
				++framesize;
			}

#if SYNCLINK_GENERIC_HDLC
			if (info->netcount)
				hdlcdev_rx(info, buf->data, framesize);
			else
#endif
				ldisc_receive_buf(tty, buf->data, info->flag_buf, framesize);
		}
	}

	spin_lock_irqsave(&info->lock,flags);
	buf->status = buf->count = 0;
	info->rx_frame_count--;
	info->rx_get++;
	if (info->rx_get >= info->rx_buf_count)
		info->rx_get = 0;
	spin_unlock_irqrestore(&info->lock,flags);

	return true;
}

static bool register_test(MGSLPC_INFO *info)
{
	static unsigned char patterns[] =
	    { 0x00, 0xff, 0xaa, 0x55, 0x69, 0x96, 0x0f };
	static unsigned int count = ARRAY_SIZE(patterns);
	unsigned int i;
	bool rc = true;
	unsigned long flags;

	spin_lock_irqsave(&info->lock,flags);
	reset_device(info);

	for (i = 0; i < count; i++) {
		write_reg(info, XAD1, patterns[i]);
		write_reg(info, XAD2, patterns[(i + 1) % count]);
		if ((read_reg(info, XAD1) != patterns[i]) ||
		    (read_reg(info, XAD2) != patterns[(i + 1) % count])) {
			rc = false;
			break;
		}
	}

	spin_unlock_irqrestore(&info->lock,flags);
	return rc;
}

static bool irq_test(MGSLPC_INFO *info)
{
	unsigned long end_time;
	unsigned long flags;

	spin_lock_irqsave(&info->lock,flags);
	reset_device(info);

	info->testing_irq = true;
	hdlc_mode(info);

	info->irq_occurred = false;

	/* init hdlc mode */

	irq_enable(info, CHA, IRQ_TIMER);
	write_reg(info, CHA + TIMR, 0);	/* 512 cycles */
	issue_command(info, CHA, CMD_START_TIMER);

	spin_unlock_irqrestore(&info->lock,flags);

	end_time=100;
	while(end_time-- && !info->irq_occurred) {
		msleep_interruptible(10);
	}

	info->testing_irq = false;

	spin_lock_irqsave(&info->lock,flags);
	reset_device(info);
	spin_unlock_irqrestore(&info->lock,flags);

	return info->irq_occurred;
}

static int adapter_test(MGSLPC_INFO *info)
{
	if (!register_test(info)) {
		info->init_error = DiagStatus_AddressFailure;
		printk( "%s(%d):Register test failure for device %s Addr=%04X\n",
			__FILE__,__LINE__,info->device_name, (unsigned short)(info->io_base) );
		return -ENODEV;
	}

	if (!irq_test(info)) {
		info->init_error = DiagStatus_IrqFailure;
		printk( "%s(%d):Interrupt test failure for device %s IRQ=%d\n",
			__FILE__,__LINE__,info->device_name, (unsigned short)(info->irq_level) );
		return -ENODEV;
	}

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s(%d):device %s passed diagnostics\n",
			__FILE__,__LINE__,info->device_name);
	return 0;
}

static void trace_block(MGSLPC_INFO *info,const char* data, int count, int xmit)
{
	int i;
	int linecount;
	if (xmit)
		printk("%s tx data:\n",info->device_name);
	else
		printk("%s rx data:\n",info->device_name);

	while(count) {
		if (count > 16)
			linecount = 16;
		else
			linecount = count;

		for(i=0;i<linecount;i++)
			printk("%02X ",(unsigned char)data[i]);
		for(;i<17;i++)
			printk("   ");
		for(i=0;i<linecount;i++) {
			if (data[i]>=040 && data[i]<=0176)
				printk("%c",data[i]);
			else
				printk(".");
		}
		printk("\n");

		data  += linecount;
		count -= linecount;
	}
}

/* HDLC frame time out
 * update stats and do tx completion processing
 */
static void tx_timeout(unsigned long context)
{
	MGSLPC_INFO *info = (MGSLPC_INFO*)context;
	unsigned long flags;

	if ( debug_level >= DEBUG_LEVEL_INFO )
		printk( "%s(%d):tx_timeout(%s)\n",
			__FILE__,__LINE__,info->device_name);
	if(info->tx_active &&
	   info->params.mode == MGSL_MODE_HDLC) {
		info->icount.txtimeout++;
	}
	spin_lock_irqsave(&info->lock,flags);
	info->tx_active = false;
	info->tx_count = info->tx_put = info->tx_get = 0;

	spin_unlock_irqrestore(&info->lock,flags);

#if SYNCLINK_GENERIC_HDLC
	if (info->netcount)
		hdlcdev_tx_done(info);
	else
#endif
	{
		struct tty_struct *tty = tty_port_tty_get(&info->port);
		bh_transmit(info, tty);
		tty_kref_put(tty);
	}
}

#if SYNCLINK_GENERIC_HDLC

/**
 * called by generic HDLC layer when protocol selected (PPP, frame relay, etc.)
 * set encoding and frame check sequence (FCS) options
 *
 * dev       pointer to network device structure
 * encoding  serial encoding setting
 * parity    FCS setting
 *
 * returns 0 if success, otherwise error code
 */
static int hdlcdev_attach(struct net_device *dev, unsigned short encoding,
			  unsigned short parity)
{
	MGSLPC_INFO *info = dev_to_port(dev);
	struct tty_struct *tty;
	unsigned char  new_encoding;
	unsigned short new_crctype;

	/* return error if TTY interface open */
	if (info->port.count)
		return -EBUSY;

	switch (encoding)
	{
	case ENCODING_NRZ:        new_encoding = HDLC_ENCODING_NRZ; break;
	case ENCODING_NRZI:       new_encoding = HDLC_ENCODING_NRZI_SPACE; break;
	case ENCODING_FM_MARK:    new_encoding = HDLC_ENCODING_BIPHASE_MARK; break;
	case ENCODING_FM_SPACE:   new_encoding = HDLC_ENCODING_BIPHASE_SPACE; break;
	case ENCODING_MANCHESTER: new_encoding = HDLC_ENCODING_BIPHASE_LEVEL; break;
	default: return -EINVAL;
	}

	switch (parity)
	{
	case PARITY_NONE:            new_crctype = HDLC_CRC_NONE; break;
	case PARITY_CRC16_PR1_CCITT: new_crctype = HDLC_CRC_16_CCITT; break;
	case PARITY_CRC32_PR1_CCITT: new_crctype = HDLC_CRC_32_CCITT; break;
	default: return -EINVAL;
	}

	info->params.encoding = new_encoding;
	info->params.crc_type = new_crctype;

	/* if network interface up, reprogram hardware */
	if (info->netcount) {
		tty = tty_port_tty_get(&info->port);
		mgslpc_program_hw(info, tty);
		tty_kref_put(tty);
	}

	return 0;
}

/**
 * called by generic HDLC layer to send frame
 *
 * skb  socket buffer containing HDLC frame
 * dev  pointer to network device structure
 *
 * returns 0 if success, otherwise error code
 */
static int hdlcdev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	MGSLPC_INFO *info = dev_to_port(dev);
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk(KERN_INFO "%s:hdlc_xmit(%s)\n",__FILE__,dev->name);

	/* stop sending until this frame completes */
	netif_stop_queue(dev);

	/* copy data to device buffers */
	skb_copy_from_linear_data(skb, info->tx_buf, skb->len);
	info->tx_get = 0;
	info->tx_put = info->tx_count = skb->len;

	/* update network statistics */
	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	/* done with socket buffer, so free it */
	dev_kfree_skb(skb);

	/* save start time for transmit timeout detection */
	dev->trans_start = jiffies;

	/* start hardware transmitter if necessary */
	spin_lock_irqsave(&info->lock,flags);
	if (!info->tx_active) {
		struct tty_struct *tty = tty_port_tty_get(&info->port);
	 	tx_start(info, tty);
	 	tty_kref_put(tty);
	}
	spin_unlock_irqrestore(&info->lock,flags);

	return 0;
}

/**
 * called by network layer when interface enabled
 * claim resources and initialize hardware
 *
 * dev  pointer to network device structure
 *
 * returns 0 if success, otherwise error code
 */
static int hdlcdev_open(struct net_device *dev)
{
	MGSLPC_INFO *info = dev_to_port(dev);
	struct tty_struct *tty;
	int rc;
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s:hdlcdev_open(%s)\n",__FILE__,dev->name);

	/* generic HDLC layer open processing */
	if ((rc = hdlc_open(dev)))
		return rc;

	/* arbitrate between network and tty opens */
	spin_lock_irqsave(&info->netlock, flags);
	if (info->port.count != 0 || info->netcount != 0) {
		printk(KERN_WARNING "%s: hdlc_open returning busy\n", dev->name);
		spin_unlock_irqrestore(&info->netlock, flags);
		return -EBUSY;
	}
	info->netcount=1;
	spin_unlock_irqrestore(&info->netlock, flags);

	tty = tty_port_tty_get(&info->port);
	/* claim resources and init adapter */
	if ((rc = startup(info, tty)) != 0) {
		tty_kref_put(tty);
		spin_lock_irqsave(&info->netlock, flags);
		info->netcount=0;
		spin_unlock_irqrestore(&info->netlock, flags);
		return rc;
	}
	/* assert DTR and RTS, apply hardware settings */
	info->serial_signals |= SerialSignal_RTS + SerialSignal_DTR;
	mgslpc_program_hw(info, tty);
	tty_kref_put(tty);

	/* enable network layer transmit */
	dev->trans_start = jiffies;
	netif_start_queue(dev);

	/* inform generic HDLC layer of current DCD status */
	spin_lock_irqsave(&info->lock, flags);
	get_signals(info);
	spin_unlock_irqrestore(&info->lock, flags);
	if (info->serial_signals & SerialSignal_DCD)
		netif_carrier_on(dev);
	else
		netif_carrier_off(dev);
	return 0;
}

/**
 * called by network layer when interface is disabled
 * shutdown hardware and release resources
 *
 * dev  pointer to network device structure
 *
 * returns 0 if success, otherwise error code
 */
static int hdlcdev_close(struct net_device *dev)
{
	MGSLPC_INFO *info = dev_to_port(dev);
	struct tty_struct *tty = tty_port_tty_get(&info->port);
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s:hdlcdev_close(%s)\n",__FILE__,dev->name);

	netif_stop_queue(dev);

	/* shutdown adapter and release resources */
	shutdown(info, tty);
	tty_kref_put(tty);
	hdlc_close(dev);

	spin_lock_irqsave(&info->netlock, flags);
	info->netcount=0;
	spin_unlock_irqrestore(&info->netlock, flags);

	return 0;
}

/**
 * called by network layer to process IOCTL call to network device
 *
 * dev  pointer to network device structure
 * ifr  pointer to network interface request structure
 * cmd  IOCTL command code
 *
 * returns 0 if success, otherwise error code
 */
static int hdlcdev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	const size_t size = sizeof(sync_serial_settings);
	sync_serial_settings new_line;
	sync_serial_settings __user *line = ifr->ifr_settings.ifs_ifsu.sync;
	MGSLPC_INFO *info = dev_to_port(dev);
	unsigned int flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("%s:hdlcdev_ioctl(%s)\n",__FILE__,dev->name);

	/* return error if TTY interface open */
	if (info->port.count)
		return -EBUSY;

	if (cmd != SIOCWANDEV)
		return hdlc_ioctl(dev, ifr, cmd);

	switch(ifr->ifr_settings.type) {
	case IF_GET_IFACE: /* return current sync_serial_settings */

		ifr->ifr_settings.type = IF_IFACE_SYNC_SERIAL;
		if (ifr->ifr_settings.size < size) {
			ifr->ifr_settings.size = size; /* data size wanted */
			return -ENOBUFS;
		}

		flags = info->params.flags & (HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_RXC_DPLL |
					      HDLC_FLAG_RXC_BRG    | HDLC_FLAG_RXC_TXCPIN |
					      HDLC_FLAG_TXC_TXCPIN | HDLC_FLAG_TXC_DPLL |
					      HDLC_FLAG_TXC_BRG    | HDLC_FLAG_TXC_RXCPIN);

		switch (flags){
		case (HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_TXC_TXCPIN): new_line.clock_type = CLOCK_EXT; break;
		case (HDLC_FLAG_RXC_BRG    | HDLC_FLAG_TXC_BRG):    new_line.clock_type = CLOCK_INT; break;
		case (HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_TXC_BRG):    new_line.clock_type = CLOCK_TXINT; break;
		case (HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_TXC_RXCPIN): new_line.clock_type = CLOCK_TXFROMRX; break;
		default: new_line.clock_type = CLOCK_DEFAULT;
		}

		new_line.clock_rate = info->params.clock_speed;
		new_line.loopback   = info->params.loopback ? 1:0;

		if (copy_to_user(line, &new_line, size))
			return -EFAULT;
		return 0;

	case IF_IFACE_SYNC_SERIAL: /* set sync_serial_settings */

		if(!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (copy_from_user(&new_line, line, size))
			return -EFAULT;

		switch (new_line.clock_type)
		{
		case CLOCK_EXT:      flags = HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_TXC_TXCPIN; break;
		case CLOCK_TXFROMRX: flags = HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_TXC_RXCPIN; break;
		case CLOCK_INT:      flags = HDLC_FLAG_RXC_BRG    | HDLC_FLAG_TXC_BRG;    break;
		case CLOCK_TXINT:    flags = HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_TXC_BRG;    break;
		case CLOCK_DEFAULT:  flags = info->params.flags &
					     (HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_RXC_DPLL |
					      HDLC_FLAG_RXC_BRG    | HDLC_FLAG_RXC_TXCPIN |
					      HDLC_FLAG_TXC_TXCPIN | HDLC_FLAG_TXC_DPLL |
					      HDLC_FLAG_TXC_BRG    | HDLC_FLAG_TXC_RXCPIN); break;
		default: return -EINVAL;
		}

		if (new_line.loopback != 0 && new_line.loopback != 1)
			return -EINVAL;

		info->params.flags &= ~(HDLC_FLAG_RXC_RXCPIN | HDLC_FLAG_RXC_DPLL |
					HDLC_FLAG_RXC_BRG    | HDLC_FLAG_RXC_TXCPIN |
					HDLC_FLAG_TXC_TXCPIN | HDLC_FLAG_TXC_DPLL |
					HDLC_FLAG_TXC_BRG    | HDLC_FLAG_TXC_RXCPIN);
		info->params.flags |= flags;

		info->params.loopback = new_line.loopback;

		if (flags & (HDLC_FLAG_RXC_BRG | HDLC_FLAG_TXC_BRG))
			info->params.clock_speed = new_line.clock_rate;
		else
			info->params.clock_speed = 0;

		/* if network interface up, reprogram hardware */
		if (info->netcount) {
			struct tty_struct *tty = tty_port_tty_get(&info->port);
			mgslpc_program_hw(info, tty);
			tty_kref_put(tty);
		}
		return 0;

	default:
		return hdlc_ioctl(dev, ifr, cmd);
	}
}

/**
 * called by network layer when transmit timeout is detected
 *
 * dev  pointer to network device structure
 */
static void hdlcdev_tx_timeout(struct net_device *dev)
{
	MGSLPC_INFO *info = dev_to_port(dev);
	unsigned long flags;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("hdlcdev_tx_timeout(%s)\n",dev->name);

	dev->stats.tx_errors++;
	dev->stats.tx_aborted_errors++;

	spin_lock_irqsave(&info->lock,flags);
	tx_stop(info);
	spin_unlock_irqrestore(&info->lock,flags);

	netif_wake_queue(dev);
}

/**
 * called by device driver when transmit completes
 * reenable network layer transmit if stopped
 *
 * info  pointer to device instance information
 */
static void hdlcdev_tx_done(MGSLPC_INFO *info)
{
	if (netif_queue_stopped(info->netdev))
		netif_wake_queue(info->netdev);
}

/**
 * called by device driver when frame received
 * pass frame to network layer
 *
 * info  pointer to device instance information
 * buf   pointer to buffer contianing frame data
 * size  count of data bytes in buf
 */
static void hdlcdev_rx(MGSLPC_INFO *info, char *buf, int size)
{
	struct sk_buff *skb = dev_alloc_skb(size);
	struct net_device *dev = info->netdev;

	if (debug_level >= DEBUG_LEVEL_INFO)
		printk("hdlcdev_rx(%s)\n",dev->name);

	if (skb == NULL) {
		printk(KERN_NOTICE "%s: can't alloc skb, dropping packet\n", dev->name);
		dev->stats.rx_dropped++;
		return;
	}

	memcpy(skb_put(skb, size), buf, size);

	skb->protocol = hdlc_type_trans(skb, dev);

	dev->stats.rx_packets++;
	dev->stats.rx_bytes += size;

	netif_rx(skb);
}

static const struct net_device_ops hdlcdev_ops = {
	.ndo_open       = hdlcdev_open,
	.ndo_stop       = hdlcdev_close,
	.ndo_change_mtu = hdlc_change_mtu,
	.ndo_start_xmit = hdlc_start_xmit,
	.ndo_do_ioctl   = hdlcdev_ioctl,
	.ndo_tx_timeout = hdlcdev_tx_timeout,
};

/**
 * called by device driver when adding device instance
 * do generic HDLC initialization
 *
 * info  pointer to device instance information
 *
 * returns 0 if success, otherwise error code
 */
static int hdlcdev_init(MGSLPC_INFO *info)
{
	int rc;
	struct net_device *dev;
	hdlc_device *hdlc;

	/* allocate and initialize network and HDLC layer objects */

	if (!(dev = alloc_hdlcdev(info))) {
		printk(KERN_ERR "%s:hdlc device allocation failure\n",__FILE__);
		return -ENOMEM;
	}

	/* for network layer reporting purposes only */
	dev->base_addr = info->io_base;
	dev->irq       = info->irq_level;

	/* network layer callbacks and settings */
	dev->netdev_ops	    = &hdlcdev_ops;
	dev->watchdog_timeo = 10 * HZ;
	dev->tx_queue_len   = 50;

	/* generic HDLC layer callbacks and settings */
	hdlc         = dev_to_hdlc(dev);
	hdlc->attach = hdlcdev_attach;
	hdlc->xmit   = hdlcdev_xmit;

	/* register objects with HDLC layer */
	if ((rc = register_hdlc_device(dev))) {
		printk(KERN_WARNING "%s:unable to register hdlc device\n",__FILE__);
		free_netdev(dev);
		return rc;
	}

	info->netdev = dev;
	return 0;
}

/**
 * called by device driver when removing device instance
 * do generic HDLC cleanup
 *
 * info  pointer to device instance information
 */
static void hdlcdev_exit(MGSLPC_INFO *info)
{
	unregister_hdlc_device(info->netdev);
	free_netdev(info->netdev);
	info->netdev = NULL;
}

#endif /* CONFIG_HDLC */

                                                                                                                         /*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#include "usbip_common.h"

static int event_handler(struct usbip_device *ud)
{
	dbg_eh("enter\n");

	/*
	 * Events are handled by only this thread.
	 */
	while (usbip_event_happend(ud)) {
		dbg_eh("pending event %lx\n", ud->event);

		/*
		 * NOTE: shutdown must come first.
		 * Shutdown the device.
		 */
		if (ud->event & USBIP_EH_SHUTDOWN) {
			ud->eh_ops.shutdown(ud);

			ud->event &= ~USBIP_EH_SHUTDOWN;

			break;
		}

		/* Stop the error handler. */
		if (ud->event & USBIP_EH_BYE)
			return -1;

		/* Reset the device. */
		if (ud->event & USBIP_EH_RESET) {
			ud->eh_ops.reset(ud);

			ud->event &= ~USBIP_EH_RESET;

			break;
		}

		/* Mark the device as unusable. */
		if (ud->event & USBIP_EH_UNUSABLE) {
			ud->eh_ops.unusable(ud);

			ud->event &= ~USBIP_EH_UNUSABLE;

			break;
		}

		/* NOTREACHED */
		printk(KERN_ERR "%s: unknown event\n", __func__);
		return -1;
	}

	return 0;
}

static void event_handler_loop(struct usbip_task *ut)
{
	struct usbip_device *ud = container_of(ut, struct usbip_device, eh);

	while (1) {
		if (signal_pending(current)) {
			dbg_eh("signal catched!\n");
			break;
		}

		if (event_handler(ud) < 0)
			break;

		wait_event_interruptible(ud->eh_waitq, usbip_event_happend(ud));
		dbg_eh("wakeup\n");
	}
}

void usbip_start_eh(struct usbip_device *ud)
{
	struct usbip_task *eh = &ud->eh;

	init_waitqueue_head(&ud->eh_waitq);
	ud->event = 0;

	usbip_task_init(eh, "usbip_eh", event_handler_loop);

	kernel_thread(usbip_thread, (void *)eh, 0);

	wait_for_completion(&eh->thread_done);
}
EXPORT_SYMBOL_GPL(usbip_start_eh);

void usbip_stop_eh(struct usbip_device *ud)
{
	struct usbip_task *eh = &ud->eh;

	wait_for_completion(&eh->thread_done);
	dbg_eh("usbip_eh has finished\n");
}
EXPORT_SYMBOL_GPL(usbip_stop_eh);

void usbip_event_add(struct usbip_device *ud, unsigned long event)
{
	spin_lock(&ud->lock);

	ud->event |= event;

	wake_up(&ud->eh_waitq);

	spin_unlock(&ud->lock);
}
EXPORT_SYMBOL_GPL(usbip_event_add);

int usbip_event_happend(struct usbip_device *ud)
{
	int happend = 0;

	spin_lock(&ud->lock);

	if (ud->event != 0)
		happend = 1;

	spin_unlock(&ud->lock);

	return happend;
}
EXPORT_SYMBOL_GPL(usbip_event_happend);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  щ    Н      ъ    О      ы    П      ь    Р      э    С      ю    Т      я    У           Ф          Х          Ц          Ч          Ш          Щ          Ъ          Ы          Ь      	    Э      
    Ю          Я          а          б          в          г          д          е          ж          4"         з          и          й          ъ         к          л          м          н          о          п          р          с           т      !    у      "    ф      #    х      $    ц      %    ч      &    ш      '    щ      (    ъ      )    ы      *    ь      +    э      ,    ю      -    я      .     !     /    !     0    !     1    !     2    !     3    !     4    !     5    !     6    !     7    !     8    	!     9    
!     :    !     ;    !     <    !     =    !     >    !     ?    !     @    !     A    !     B    !     C    !     D    a     E    !     F    !     G    !     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P         Q    €	     R    v     S    !     T    !     U    !     V    !     W    !     X    !     Y    !     Z    b     [    !     \     !     ]    !!     ^    "!     `    #!     a    $!     b    %!     c    &!     d    '!     e    (!     f    )!     g    В      h    з      i    *!     j    y     k    +!     l    ,!     m    -!     n    .!     o    /!     p    0!     q    /!     r    0!     s    1!     t    2!     u    3!     v    с      w    1!     x    2!     y    4!     z    5!     {    4!     |    5!     }    6!     ~    7!         8!     Ђ    9!     Ѓ    :!     ‚    ;!     ѓ    :!     „    ;!     …    <!     †    =!     ‡    >!     €    ?!     ‰    @!     Љ    A!     ‹    B!     Њ    C!     Ќ    D!     Ћ    E!     Џ    F!     ђ    G!     ‘    H!     ’    I!     “    J!     ”    K!     •    L!     –    M!     —    N!         O!     ™    P!     љ    Q!     ›    R!     њ    S!     ќ    2"     ћ    1"     џ    T!          †     Ў    x     ў    Ж      Ј    U!     ¤    V!     Ґ    s     ¦    W!     §    X!     Ё    Y!     ©    Z!     Є    [!     «    \!     ¬    “     ­    ]!     ®    ^!     Ї    _!     °    `!     ±    a!     І    t     і    b!     ґ    В      µ    3"     ¶    5"     ·    c!     ё    d!     №    v     є    e!     »    f!     ј    g!     Ѕ    h!     ѕ    i!     ї    j!     А    k!     Б    l!     В    m!     Г    Z!     Д    n!     Е    o!     Ж    p!     З    q!     И    r!     Й    s!     К    t!     Л    ‡     М    Ѓ     Н    »      О    u!     П    v!     Р    w!     С    x!     Т    y!     У    z!     Ф    {!     Х    |!     Ц    ·      Ч    }!     Ш    ~!     Щ    !     Ъ    Ђ!     Ы    Ѓ!     Ь    ‚!     Э    ѓ!     Ю    „!     а    …!     б    †!     в    ‡!     г    €!     д    ‰!     е    Љ!     ж    ‹!     з    Њ!     и    Ќ!     й    Ћ!     к    h     л    Џ!     м    ђ!     н    ‘!     о    ’!     п    “!     р    ”!     с    •!     т    –!     у    —!     ф    !     х    ™!     ц    љ!     ч    ›!     ш    d!     щ    њ!     ъ    ќ!     ы    ћ!     ь    џ!     э     !     ю    Ў!     я    ў!          Ј!         ¤!         Ґ!         ¦!         §!         Ё!         ©!         Є!         z     	    «!     
    ¬!         ­!         ®!         Ї!         №          °!         ±!         І!         і!         ґ!         µ!         ¶!         ·!         ё!         №!         є!         »!         ј!         Ѕ!         ѕ!         ї!         А!          Б!     !    В!     "    Г!     #    Д!     $    Е!     %    Ж!     &    З!     '    И!     (    w     3    Й!     4    К!     5    Л!     6    М!     7    Н!     8          9    О!     :    П!     ;    Р!     <    С!     =    Т!     >    V!     ?    ‚!     @    У!     A    Ф!     B    Х!     C    Ц!     D    Ч!     E    Ш!     F    Щ!     G    Ъ!     H    Ы!     I    Ь!     J    Э!     K    Ю!     N    Я!     O    а!     P    б!     Q    в!     R    г!     S    ”     T    д!     U    й      V    е!     W    ж!     X    з!     Y    и!     Z    й!     [    к!     \    л!     ]    м!     ^    н!     _    о!     `    п!     a    р!     b    с!     c    т!     d    у!     e    ф!     f    х!     g    ц!     h    ч!     i    ш!     j    щ!     k    ъ!     l    ы!     m    ь!     n    э!     o    l     p    u     q    r     r    q     s    ю!     t    ґ      u    r!     v    я!     w    |     x    ѓ     y     "     z    й      {    "     |    "     }    "     ~    "         "     Ђ    "     Ѓ    "     ‚    "     ѓ    	"     „    
"     …    "     †    "     ‡    "     €    К!     ‰    "     Љ    "     ‹    "     Њ    "     Ќ    "     Ћ    "     Џ    "     ђ    "     ‘    "     ’    "     “    "     ”    "     •    "     –    "     —    "         <      ™    "     љ    "     ›    "     њ     "     ќ    !"     ћ    ""     џ    #"          {     Ў    $"     ў    %"     Ј    &"     ¤    '"     Ґ    ("     ¦    )"     §    с!     Ё    т!     ©    у!     Є    ф!     «    х!     ¬    *"  D      й  и  0       ro                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    6"     *    7"     +    8"  D ш     й  и  0       ro                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    1"  D ш     й  и  0       ro                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    2"  D ($     й  и  0       ar                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяА  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяяяяяP  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя   Ъ    9"     Ы    :"     Ь    ;"     Э    <"     °    ="     Ю    >"     Я    ?"     а    @"     б    A"     в    B"     г    C"     д    D"     е    E"     ж    F"     з    G"     и    H"     й    I"     к    J"     л    K"     м    H"     н    v     о    L"     п    M"     р    N"     с    O"     т    A"     х    P"     ц    Q"     ч    R"     ш    S"     щ    T"     ъ    U"     ы    V"     ь    W"     э    X"     ю    Y"     я    Z"          ["         \"         ]"         ^"         _"         `"         a"         a"         b"     	    c"     
    d"         e"         f"         g"         h"         i"         j"         k"         l"         m"         n"         o"         p"         q"         r"         r"         s"         t"         u"         v"         w"         x"          y"     !    z"     "    {"     #    |"     $    }"     %    ~"     &    "     '    Ђ"     (    Ѓ"     )    ‚"     *    ѓ"     +    „"     ,    …"     -    †"     .    ‡"     /    €"     0    ‰"     1    Љ"     2    ‹"     3    Њ"     4    Ќ"     5    Ћ"     6    Џ"     7    Џ"     8    ђ"     9    ‘"     :    ’"     ;    “"     <    ”"     =    •"     >    –"     ?    —"     @    "     A    ™"     B    љ"     C    ›"     D    њ"     E    ќ"     F    ћ"     G    џ"     H    i     I     "     J    m     K    Ў"     L    ў"     M    Ј"     N    j     O    k     P    ¤"     Q    ~     R    v     S    Ґ"     T    ¦"     U    §"     V    Ё"     W    ©"     X    Є"     Y    «"     Z    ¬"     [    ­"     \    ®"     ]    Ї"     ^    °"     `    ±"     a    І"     b    і"     c    ґ"     d    µ"     e    ¶"     f    ·"     g    H"     h    ё"     i    №"     j    y     k    є"     l    »"     m    ј"     n    Ѕ"     o    ѕ"     p    ї"     q    А"     r    Б"     s    В"     t    Г"     u    Д"     v    x"     w    Е"     x    Ж"     y    З"     z    И"     {    Й"     |    К"     }    Л"     ~    М"         Н"     Ђ    О"     Ѓ    П"     ‚    Р"     ѓ    С"     „    Т"     …    У"     †    Ф"     ‡    Х"     €    Ц"     ‰    Ч"     Љ    Ш"     ‹    Щ"     Њ    Ъ"     Ќ    Ы"     Ћ    Ь"     Џ    Э"     ђ    Ю"     ‘    Я"     ’    а"     “    б"     ”    в"     •    г"     –    д"     —    е"         ж"     ™    з"     љ    и"     ›    й"     њ    к"     ќ    л"     ћ    м"     џ    н"          о"     Ў    x     ў    L"     Ј    п"     ¤    р"     Ґ    с"     ¦    т"     §    у"     Ё    ф"     ©    х"     Є    ц"     «    ч"     ¬    ш"     ­    щ"     ®    ъ"     Ї    ы"     °    ь"     ±    э"     І    t     і    ю"     ґ    H"     µ    я"     ¶     #     ·    #     ё    #     №    v     є    #     »    #     ј    #     Ѕ    #     ѕ    #     ї    #     А    	#     Б    
#     В    #     Г    х"     Д    O"     Е    #     Ж    #     З    #     И    #     Й    #     К    #     Л    #     М    #     Н    A"     О    #     П    #     Р    #     С    #     Т    #     У    #     Ф    #     Х    #     Ц    ="     Ч    <"     Ш    #     Щ    o"     Ъ    #     Ы    р"     Ь    #     Э    #     Ю     #     а    !#     б    o"     в    "#     г    ##     д    $#     е    %#     ж    &#     з    '#     и    '#     й    (#     к    h     л    )#     м    *#     н    +#     о    ,#     п    -#     р    .#     с    /#     т    0#     у    1#     ф    2#     х    3#     ц    4#     ч    5#     ш    #     щ    6#     ъ    7#     ы    8#     ь    9#     э    :#     ю    ;#     я    <#          =#         >#         ?#         @#         A#         B#         C#         D#         z     	    E#     
    F#         G#         М#         H#         ?"         I#         J#         K#         L#         M#         N#         O#         P#         Q#         R#         S#         T#         U#         V#         W#         X#         Y#          Z#     !    [#     "    \#     #    ]#     $    ^#     %    _#     &    `#     '    a#     (    w     3    …     4    b#     5    c#     6    d#     7    e#     8          9    f#     :    g#     ;    h#     <    i#     =    j#     >    р"     ?    #     @    k#     A    l#     B    m#     C    n#     D    o#     E    p#     F    q#     G    r#     H    s#     I    t#     J    u#     K    v#     N    w#     O    x#     P    y#     Q    z#     R    {#     S    |#     T    }#     U    p"     V    ~#     W    #     X    Ђ#     Y    Ѓ#     Z    ‚#     [    ѓ#     \    „#     ]    …#     ^    †#     _    ‡#     `    €#     a    ‰#     b    Љ#     c    ‹#     d    Њ#     e    Ќ#     f    Ћ#     g    Џ#     h    ђ#     i    ‘#     j    ’#     k    “#     l    ”#     m    •#     n    –#     o    l     p    —#     q    r     r    q     s    #     t    ™#     u    #     v    љ#     w    ›#     x    ѓ     y    њ#     z    p"     {    ќ#     |    ћ#     }    џ#     ~     #         Ў#     Ђ    ў#     Ѓ    Ј#     ‚    ¤#     ѓ    Ґ#     „    ¦#     …    §#     †    Ё#     ‡    ©#     €    Є#     ‰    «#     Љ    ¬#     ‹    ­#     Њ    ®#     Ќ    v#     Ћ    Ї#     Џ    °#     ђ    ±#     ‘    І#     ’    і#     “    ґ#     ”    µ#     •    ¶#     –    ·#     —    ё#         =      ™    №#     љ    є#     ›    »#     њ    ј#     ќ    Ѕ#     ћ    ѕ#     џ    ї#          {     Ў    ;#     ў    А#     Ј    Б#     ¤    В#     Ґ    Г#     ¦    Д#     §    Љ#     Ё    ‹#     ©    Њ#     Є    Ќ#     «    Ћ#     ¬    Е#  D      й  и  0       ar                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    М#     *    Н#     +    О#  D ш     й  и  0       ar                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    П#  D ш     й  и  0       ar                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    Р#  D $     й  и  0       fr                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяя°  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р       яяяя   0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          яяяяяяяя   Ъ    С#     Ы    g$     Ь    h$     Э    i$     °    j$     Ю    k$     Я    l$     а    m$     б    ¤     в    Т#     г    У#     д    Ф#     е    Х#     ж    Ц#     з    Ч#     и    n$     й    Ш#     к    o$     л    Щ#     м    n$     н    v     о    p$     п    Ъ#     р    q$     с    Ђ     т    ¤     х    s$     ц    t$     ч    u$     ш    Ы#     щ    Ь#     ъ    Э#     ы    Ю#     ь    v$     э    w$     ю    x$     я    y$          z$         Я#         {$         |$         }$         ~$         $         а#         Ђ$     	    Ѓ$     
    ‚$         ѓ$         б#         „$         в#         г#         …$         д#         е#         ‚         †$         ‡$         €$         ж#         ‰$         Љ$         з#         и#         ‹$         Њ$         Ќ$         Ћ$          Џ$     !    ђ$     "    ‘$     #    ’$     $    “$     %    ”$     &    •$     '    –$     (    —$     )    $     *    ™$     +    •$     ,    љ$     -    ›$     .    њ$     /    ™$     0    ќ$     1    ћ$     2    џ$     3     $     4    Ў$     5    ў$     6    Ј$     7    Ј$     8    ¤$     9    Ґ$     :    ¦$     ;    §$     <    Ё$     =    •$     >    ©$     ?    •$     @    љ$     A    Є$     B    «$     C    ™$     D    ¬$     E    ­$     F    ®$     G    Ї$     H    i     I    °$     J    m     K    n     L    ±$     M    o     N    j     O    k     P    І$     Q    €	     R    v     S    і$     T    ґ$     U    p$     W    µ$     X    ¤     Y    ¶$     Z    ·$     [    ё$     \    №$     ]    є$     ^    »$     `    й#     a    ј$     b    к#     c    Ѕ$     d    ѕ$     e    ї$     f    А$     g    n$     h    †$     i    л#     j    †	     k    м#     l    Б$     m    В$     n    Г$     o    н#     p    о#     q    Д$     r    Е$     s    Ж$     t    З$     u    И$     v    Й$     w    Ж$     x    З$     y    п#     z    р#     {    п#     |    р#     }    К$     ~    с#         Л$     Ђ    т#     Ѓ    М$     ‚    Н$     ѓ    М$     „    Н$     …    О$     †    П$     ‡    Р$     €    С$     ‰    у#     Љ    ф#     ‹    х#     Њ    Т$     Ќ    ц#     Ћ    У$     Џ    ч#     ђ    Ф$     ‘    Х$     ’    Ц$     “    Ч$     ”    Ш$     •    Щ$     –    Ъ$     —    Ы$         Ь$     ™    Э$     љ    Ю$     ›    Я$     њ    ш#     ќ    щ#     ћ    ъ#     џ    а$          ы#     Ў    x     ў    ь#     Ј    б$     ¤    в$     Ґ    s     ¦    г$     §    э#     Ё    ю#     ©    д$     Є    е$     «    ж$     ¬    ‘     ­    з$     ®    и$     Ї    я#     °    й$     ±    к$     І    t     і     $     ґ    n$     µ    $     ¶    л$     ·    м$     ё    н$     №    v     є    ©$     »    о$     ј    ќ$     Ѕ    $     ѕ    $     ї    п$     А    р$     Б    $     В    $     Г    д$     Д    r$     Е    с$     Ж    т$     З    у$     И    $     Й    ®$     К    $     Л    х$     М    $     Н    ¤     О    ц$     П    	$     Р    
$     С    ч$     Т    ш$     У    $     Ф    $     Х    $     Ц    j$     Ч    i$     Ш    щ$     Щ    ъ$     Ъ    ы$     Ы    ь$     Ь    $     Э    $     Ю    э$     а    ю$     б    ‡$     в    $     г    $     д    $     е    $     ж    $     з    $     и    $     й    $     к    h     л    $     м    $     н    $     о    $     п    $     р    $     с    $     т    $     у    $     ф    я$     х     $     ц     %     ч    %     ш    н$     щ    %     ъ    %     ы    %     ь    !$     э    %     ю    %     я    %          "$         #$         %         $$         	%         %$         &$         '$         €     	    
%     
    ($         %         f$         )$         l$         %         *$         +$         ,$         -$         .$         /$         0$         1$         2$         3$         4$         5$         6$         7$         8$         5$          9$     !    :$     "    ;$     #    <$     $    %     %    %     &    %     '    =$     (    w     3    >$     4    ?$     5    @$     6    A$     7    B$     8          9    %     :    C$     ;    D$     <    E$     =    %     >    ь$     ?    F$     @    %     A    %     B    %     C    і$     D    %     E    %     F    %     G    %     H    %     I    %     J    %     K    %     N    %     O    %     P    %     Q    G$     R    H$     S    ‰     T     %     U    !%     V    "%     W    #%     X    I$     Y    $%     Z    %%     [    &%     \    '%     ]    (%     ^    J$     _    )%     `    *%     a    +%     b    ,%     c    -%     d    .%     e    /%     f    0%     g    1%     h    2%     i    3%     j    K$     k    L$     l    M$     m    N$     n    O$     o    l     p    u     q    r     r    q     s    4%     t    %     u    ф$     v    5%     w    c     x    6%     y    ‰$     z    €$     {    7%     |    8%     }    P$     ~    Q$         9%     Ђ    :%     Ѓ    ;%     ‚    <%     ѓ    =%     „    R$     …    >%     †    ?%     ‡    S$     €    T$     ‰    @%     Љ    A%     ‹    U$     Њ    V$     Ќ    B%     Ћ    W$     Џ    C%     ђ    D%     ‘    E%     ’    F%     “    X$     ”    Y$     •    G%     –    H%     —    I%         >      ™    J%     љ    Z$     ›    K%     њ    L%     ќ    M%     ћ    N%     џ    O%          {     Ў    %     ў    [$     Ј    \$     ¤    ]$     Ґ    ^$     ¦    _$     §    ,%     Ё    -%     ©    .%     Є    /%     «    0%     ¬    P%  D      й  и  0       fr                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    f$     *    Q%     +    R%  D ш     й  и  0       fr                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя;
	if (irqs_disabled())
		print_irqtrace_events(prev);

	if (regs)
		show_regs(regs);
	else
		dump_stack();
}

/*
 * Various schedule()-time debugging checks and statistics:
 */
static inline void schedule_debug(struct task_struct *prev)
{
	/*
	 * Test if we are atomic. Since do_exit() needs to call into
	 * schedule() atomically, we ignore that path for now.
	 * Otherwise, whine if we are scheduling when we should not be.
	 */
	if (unlikely(in_atomic_preempt_off() && !prev->exit_state))
		__schedule_bug(prev);

	profile_hit(SCHED_PROFILING, __builtin_return_address(0));

	schedstat_inc(this_rq(), sched_count);
#ifdef CONFIG_SCHEDSTATS
	if (unlikely(prev->lock_depth >= 0)) {
		schedstat_inc(this_rq(), bkl_count);
		schedstat_inc(prev, sched_info.bkl_count);
	}
#endif
}

static void put_prev_task(struct rq *rq, struct task_struct *prev)
{
	if (prev->state == TASK_RUNNING) {
		u64 runtime = prev->se.sum_exec_runtime;

		runtime -= prev->se.prev_sum_exec_runtime;
		runtime = min_t(u64, runtime, 2*sysctl_sched_migration_cost);

		/*
		 * In order to avoid avg_overlap growing stale when we are
		 * indeed overlapping and hence not getting put to sleep, grow
		 * the avg_overlap on preemption.
		 *
		 * We use the average preemption runtime because that
		 * correlates to the amount of cache footprint a task can
		 * build up.
		 */
		update_avg(&prev->se.avg_overlap, runtime);
	}
	prev->sched_class->put_prev_task(rq, prev);
}

/*
 * Pick up the highest-prio task:
 */
static inline struct task_struct *
pick_next_task(struct rq *rq)
{
	const struct sched_class *class;
	struct task_struct *p;

	/*
	 * Optimization: we know that if all tasks are in
	 * the fair class we can call that function directly:
	 */
	if (likely(rq->nr_running == rq->cfs.nr_running)) {
		p = fair_sched_class.pick_next_task(rq);
		if (likely(p))
			return p;
	}

	class = sched_class_highest;
	for ( ; ; ) {
		p = class->pick_next_task(rq);
		if (p)
			return p;
		/*
		 * Will never be NULL as the idle class always
		 * returns a non-NULL p:
		 */
		class = class->next;
	}
}

/*
 * schedule() is the main scheduler function.
 */
asmlinkage void __sched schedule(void)
{
	struct task_struct *prev, *next;
	unsigned long *switch_count;
	struct rq *rq;
	int cpu;

need_resched:
	preempt_disable();
	cpu = smp_processor_id();
	rq = cpu_rq(cpu);
	rcu_qsctr_inc(cpu);
	prev = rq->curr;
	switch_count = &prev->nivcsw;

	release_kernel_lock(prev);
need_resched_nonpreemptible:

	schedule_debug(prev);

	if (sched_feat(HRTICK))
		hrtick_clear(rq);

	spin_lock_irq(&rq->lock);
	update_rq_clock(rq);
	clear_tsk_need_resched(prev);

	if (prev->state && !(preempt_count() & PREEMPT_ACTIVE)) {
		if (unlikely(signal_pending_state(prev->state, prev)))
			prev->state = TASK_RUNNING;
		else
			deactivate_task(rq, prev, 1);
		switch_count = &prev->nvcsw;
	}

#ifdef CONFIG_SMP
	if (prev->sched_class->pre_schedule)
		prev->sched_class->pre_schedule(rq, prev);
#endif

	if (unlikely(!rq->nr_running))
		idle_balance(cpu, rq);

	put_prev_task(rq, prev);
	next = pick_next_task(rq);

	if (likely(prev != next)) {
		sched_info_switch(prev, next);
		perf_counter_task_sched_out(prev, next, cpu);

		rq->nr_switches++;
		rq->curr = next;
		++*switch_count;

		context_switch(rq, prev, next); /* unlocks the rq */
		/*
		 * the context switch might have flipped the stack from under
		 * us, hence refresh the local variables.
		 */
		cpu = smp_processor_id();
		rq = cpu_rq(cpu);
	} else
		spin_unlock_irq(&rq->lock);

	if (unlikely(reacquire_kernel_lock(current) < 0))
		goto need_resched_nonpreemptible;

	preempt_enable_no_resched();
	if (need_resched())
		goto need_resched;
}
EXPORT_SYMBOL(schedule);

#ifdef CONFIG_SMP
/*
 * Look out! "owner" is an entirely speculative pointer
 * access and not reliable.
 */
int mutex_spin_on_owner(struct mutex *lock, struct thread_info *owner)
{
	unsigned int cpu;
	struct rq *rq;

	if (!sched_feat(OWNER_SPIN))
		return 0;

#ifdef CONFIG_DEBUG_PAGEALLOC
	/*
	 * Need to access the cpu field knowing that
	 * DEBUG_PAGEALLOC could have unmapped it if
	 * the mutex owner just r  џ%     1    џ%     2     %     3    Ў%     4    ў%     5    Ј%     6    ¤%     7    Ґ%     8    ¦%     9    §%     :    Ё%     ;    ©%     <    Є%     =    «%     >    ¬%     ?    ­%     @    ®%     A    Ї%     B    °%     C    ±%     D    І%     E    і%     F    ґ%     G    µ%     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P         Q    ~     R    v     S    ¶%     T    ·%     U    f%     V    ё%     W    №%     X    є%     Y    »%     Z    ј%     [    Ѕ%     \    ѕ%     ]    ї%     ^    А%     `    Б%     a    В%     b    Г%     c    Д%     d    Е%     e    Ж%     f    З%     g    b%     h    ¤     i    И%     j    y     k    Й%     l    К%     m    Л%     n    М%     o    Н%     p    О%     q    Н%     r    О%     s    Џ%     t    П%     u    Р%     v    Ћ%     w    Џ%     x    П%     y    С%     z    Т%     {    С%     |    Т%     }    У%     ~    Ф%         Х%     Ђ    Ц%     Ѓ    Ч%     ‚    Ш%     ѓ    Ч%     „    Ш%     …    Щ%     †    Ъ%     ‡    Ы%     €    Ь%     ‰    Э%     Љ    Ю%     ‹    Я%     Њ    а%     Ќ    б%     Ћ    в%     Џ    г%     ђ    д%     ‘    е%     ’    Ґ     “    ж%     ”    з%     •    и%     –    й%     —    к%         л%     ™    м%     љ    н%     ›    о%     њ    п%     ќ    р%     ћ    с%     џ    т%          у%     Ў    x     ў    f%     Ј    ф%     ¤    х%     Ґ    s     ¦    ц%     §    ч%     Ё    ш%     ©    щ%     Є    ъ%     «    ы%     ¬    ь%     ­    э%     ®    ю%     Ї    ¦     °    я%     ±     &     І    t     і    &     ґ    b%     µ    &     ¶    Ь     ·    &     ё    &     №    v     є    &     »    &     ј    &     Ѕ    &     ѕ    	&     ї    
&     А    &     Б    &     В    &     Г    щ%     Д    &     Е    &     Ж    &     З    &     И    &     Й    &     К    &     Л    &     М    &     Н    \%     О    &     П    &     Р    &     С    &     Т    &     У    &     Ф    &     Х    &     Ц    X%     Ч    W%     Ш    &     Щ     &     Ъ    !&     Ы    "&     Ь    #&     Э    $&     Ю    %&     а    &&     б    '&     в    (&     г    )&     д    *&     е    +&     ж    ,&     з    -&     и    -&     й    .&     к    h     л    /&     м    0&     н    1&     о    2&     п    3&     р    4&     с    т     т    5&     у    6&     ф    7&     х    8&     ц    §     ч    9&     ш    &     щ    Ё     ъ    :&     ы    ;&     ь    <&     э    =&     ю    >&     я    ?&          @&         A&         B&         C&         D&         E&         F&         G&         z     	    H&     
    I&         J&         K&         L&         M&         N&         O&         P&         Q&         R&         S&         T&         U&         V&         W&         X&         Y&         Z&         [&         \&         ]&         ^&          _&     !    `&     "    a&     #    b&     $    c&     %    ©     &    d&     '    e&     (    w     3    f&     4    g&     5    h&     6    i&     7    j&     8          9    k&     :    l&     ;    m&     <    n&     =    o&     >    "&     ?    #&     @    p&     A    q&     B    r&     C    s&     D    t&     E    u&     F    v&     G    w&     H    x&     I    y&     J    z&     K    {&     N    |&     O    }&     P    Є     Q    ~&     R    &     S    ’     T    Ђ&     U    Ј     V    Ѓ&     W    ‚&     X    ѓ&     Y    „&     Z    …&     [    †&     \    ‡&     ]    €&     ^    ‰&     _    Љ&     `    ‹&     a    Њ&     b    Ќ&     c    Ћ&     d    Џ&     e    ђ&     f    ‘&     g    ’&     h    “&     i    «     j    ”&     k    •&     l    –&     m    —&     n    &     o    l     p    u     q    r     r    q     s    ™&     t    љ&     u    &     v    ›&     w    |     x    ѓ     y    њ&     z    Ј     {    ќ&     |    ћ&     }    џ&     ~     &         Ў&     Ђ    ў&     Ѓ    Ј&     ‚    ¤&     ѓ    Ґ&     „    ¦&     …    §&     †    Ё&     ‡    ©&     €    Є&     ‰    «&     Љ    ¬&     ‹    ­&     Њ    ®&     Ќ    {&     Ћ    Ї&     Џ    °&     ђ    ±&     ‘    І&     ’    і&     “    ґ&     ”    µ&     •    ¶&     –    ¬     —    ·&         ?      ™    ё&     љ    №&     ›    є&     њ    »&     ќ    ј&     ћ    Ѕ&     џ    ѕ&          щ     Ў    >&     ў    ї&     Ј    А&     ¤    Б&     Ґ    В&     ¦    Г&     §    Ќ&     Ё    Ћ&     ©    Џ&     Є    ђ&     «    ‘&     ¬    Д&  D      й  и  0       hr                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    Л&     *    М&     +    Н&  D ш     й  и  0       hr                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    О&  D ш     й  и  0       hr                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    П&  D ($     й  и  0       sr                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяА  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяяяяяP  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя   Ъ    Р&     Ы    С&     Ь    Т&     Э    У&     °    Ф&     Ю    Х&     Я    Ц&     а    Ч&     б    Ш&     в    Щ&     г    Ъ&     д    Ы&     е    Ь&     ж    Э&     з    Ю&     и    Я&     й    а&     к    б&     л    в&     м    Я&     н    v     о    г&     п    д&     р    е&     с    ж&     т    Ш&     х    з&     ц    и&     ч    й&     ш    к&     щ    л&     ъ    м&     ы    н&     ь    о&     э    п&     ю    р&     я    с&          т&         у&         ф&         х&         ц&         ч&         ш&         ш&         щ&     	    ъ&     
    ы&         ь&         э&         ю&         я&          '         '                  '         '                  '                           '         '         '         '         	'         
'         '         '          '     !    '     "    '     #    '     $    '     %    '     &    '     '    '     (    '     )    '     *    '     +    '     ,    '     -    '     .    '     /    '     0    '     1    '     2    '     3     '     4    !'     5    "'     6    #'     7    #'     8    $'     9    %'     :    &'     ;    ''     <    ('     =    )'     >    *'     ?    +'     @    ,'     A    -'     B    .'     C    /'     D    0'     E    1'     F    2'     G    3'     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P    4'     Q    ~     R    v     S    5'     T    6'     U    7'     V    8'     W    9'     X    :'     Y    ;'     Z    <'     [    ='     \    >'     ]    ?'     ^    @'     `    A'     a    B'     b    C'     c    D'     d    E'     e    F'     f    G'     g    Я&     h    H'     i    I'     j    y     k    '     l    J'     m    K'     n  .
 * @p: the task in question.
 */
int task_nice(const struct task_struct *p)
{
	return TASK_NICE(p);
}
EXPORT_SYMBOL(task_nice);

/**
 * idle_cpu - is a given cpu idle currently?
 * @cpu: the processor in question.
 */
int idle_cpu(int cpu)
{
	return cpu_curr(cpu) == cpu_rq(cpu)->idle;
}

/**
 * idle_task - return the idle task for a given cpu.
 * @cpu: the processor in question.
 */
struct task_struct *idle_task(int cpu)
{
	return cpu_rq(cpu)->idle;
}

/**
 * find_process_by_pid - find a process with a matching PID value.
 * @pid: the pid in question.
 */
static struct task_struct *find_process_by_pid(pid_t pid)
{
	return pid ? find_task_by_vpid(pid) : current;
}

/* Actually do priority change: must hold rq lock. */
static void
__setscheduler(struct rq *rq, struct task_struct *p, int policy, int prio)
{
	BUG_ON(p->se.on_rq);

	p->policy = policy;
	switch (p->policy) {
	case SCHED_NORMAL:
	case SCHED_BATCH:
	case SCHED_IDLE:
		p->sched_class = &fair_sched_class;
		break;
	case SCHED_FIFO:
	case SCHED_RR:
		p->sched_class = &rt_sched_class;
		break;
	}

	p->rt_priority = prio;
	p->normal_prio = normal_prio(p);
	/* we are holding p->pi_lock already */
	p->prio = rt_mutex_getprio(p);
	set_load_weight(p);
}

/*
 * check the target process has a UID that matches the current process's
 */
static bool check_same_owner(struct task_struct *p)
{
	const struct cred *cred = current_cred(), *pcred;
	bool match;

	rcu_read_lock();
	pcred = __task_cred(p);
	match = (cred->euid == pcred->euid ||
		 cred->euid == pcred->uid);
	rcu_read_unlock();
	return match;
}

static int __sched_setscheduler(struct task_struct *p, int policy,
				struct sched_param *param, bool user)
{
	int retval, oldprio, oldpolicy = -1, on_rq, running;
	unsigned long flags;
	const struct sched_class *prev_class = p->sched_class;
	struct rq *rq;

	/* may grab non-irq protected spin_locks */
	BUG_ON(in_interrupt());
recheck:
	/* double check policy once rq lock held */
	if (policy < 0)
		policy = oldpolicy = p->policy;
	else if (policy != SCHED_FIFO && policy != SCHED_RR &&
			policy != SCHED_NORMAL && policy != SCHED_BATCH &&
			policy != SCHED_IDLE)
		return -EINVAL;
	/*
	 * Valid priorities for SCHED_FIFO and SCHED_RR are
	 * 1..MAX_USER_RT_PRIO-1, valid priority for SCHED_NORMAL,
	 * SCHED_BATCH and SCHED_IDLE is 0.
	 */
	if (param->sched_priority < 0 ||
	    (p->mm && param->sched_priority > MAX_USER_RT_PRIO-1) ||
	    (!p->mm && param->sched_priority > MAX_RT_PRIO-1))
		return -EINVAL;
	if (rt_policy(policy) != (param->sched_priority != 0))
		return -EINVAL;

	/*
	 * Allow unprivileged RT tasks to decrease priority:
	 */
	if (user && !capable(CAP_SYS_NICE)) {
		if (rt_policy(policy)) {
			unsigned long rlim_rtprio;

			if (!lock_task_sighand(p, &flags))
				return -ESRCH;
			rlim_rtprio = p->signal->rlim[RLIMIT_RTPRIO].rlim_cur;
			unlock_task_sighand(p, &flags);

			/* can't set/change the rt policy */
			if (policy != p->policy && !rlim_rtprio)
				return -EPERM;

			/* can't increase priority */
			if (param->sched_priority > p->rt_priority &&
			    param->sched_priority > rlim_rtprio)
				return -EPERM;
		}
		/*
		 * Like positive nice levels, dont allow tasks to
		 * move out of SCHED_IDLE either:
		 */
		if (p->policy == SCHED_IDLE && policy != SCHED_IDLE)
			return -EPERM;

		/* can't change other user's priorities */
		if (!check_same_owner(p))
			return -EPERM;
	}

	if (user) {
#ifdef CONFIG_RT_GROUP_SCHED
		/*
		 * Do not allow realtime tasks into groups that have no runtime
		 * assigned.
		 */
		if (rt_bandwidth_enabled() && rt_policy(policy) &&
				task_group(p)->rt_bandwidth.rt_runtime == 0)
			return -EPERM;
#endif

		retval = security_task_setscheduler(p, policy, param);
		if (retval)
			return retval;
	}

	/*
	 * make sure no PI-waiters arrive (or leave) while we are
	 * changing the priority of the task:
	 */
	spin_lock_irqsave(&p->pi_lock, flags);
	/*
	 * To be able to change p->policy safely, the apropriate
	 * runqueue lock must be held.
	 */
	rq = __task_rq_lock(p);
	/* recheck policy now with rq lock held */
	if (unlikely(oldpolicy !=   !(     |    "(     }    #(     ~    $(         %(     Ђ    &(     Ѓ    '(     ‚    ((     ѓ    )(     „    *(     …    +(     †    ,(     ‡    -(     €    .(     ‰    /(     Љ    0(     ‹    1(     Њ    2(     Ќ    э'     Ћ    3(     Џ    4(     ђ    5(     ‘    6(     ’    7(     “    8(     ”    9(     •    :(     –    ;(     —    <(         @      ™    =(     љ    >(     ›    ?(     њ    @(     ќ    A(     ћ    B(     џ    C(          щ     Ў    Б'     ў    D(     Ј    E(     ¤    F(     Ґ    G(     ¦    H(     §    (     Ё    (     ©    (     Є    (     «    (     ¬    I(  D      й  и  0       sr                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    P(     *    S(     +    T(  D ш     й  и  0       sr                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    Q(  D ш     й  и  0       sr                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    R(  D 8$     й  и  0       tr                                      яяяяяяяяяяяяяяяяяяяяяяяя    яяяя       0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ     яяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяP  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяР  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  яяяяяяяя`  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяяяяя   Щ    ‡	     Ъ    U(     Ы    V(     Ь    W(     Э    X(     °    Y(     Ю    Z(     Я    [(     а    \(     б    ](     в    ^(     г    _(     д    `(     е    a(     ж    b(     з    c(     и    d(     й    e(     к    f(     л    g(     м    d(     н    h(     о    i(     п    j(     р    k(     с    l(     т    ](     х    m(     ц    n(     ч    o(     ш    p(     щ    q(     ъ    r(     ы    s(     ь    t(     э    u(     ю    v(     я    w(          x(         y(         z(         {(         |(         }(         ~(         ~(         (     	    Ђ(     
    Ѓ(         ‚(         ѓ(         „(         …(         †(         ‡(         €(         ‰(         Љ         Љ(         ‹(         п         ъ         Њ(         Ќ(         Ћ(         Џ(         ђ(         ‘(         ’(         “(          ”(     !    •(     "    –(     #    —(     $    (     %    ™(     &    љ(     '    ›(     (    њ(     )    ќ(     *    ћ(     +    џ(     ,     (     -    Ў(     .    ў(     /    Ј(     0    ¤(     1    Ґ(     2    ¦(     3    §(     4    Ё(     5    ©(     6    Є(     7    Є(     8    «(     9    ¬(     :    ­(     ;    ®(     <    Ї(     =    °(     >    ±(     ?    І(     @    і(     A    ґ(     B    µ(     C    ¶(     D    ·(     E    ё(     F    №(     G    є(     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P    »(     Q    ~     R    h(     S    ј(     T    Ѕ(     U    ѕ(     V    ї(     W    А(     X    Б(     Y    В(     Z    Г(     [    Д(     \    Е(     ]    Ж(     ^    З(     `    И(     a    Й(     b    К(     c    Л(     d    М(     e    Н(     f    О(     g    d(     h    П(     i    Р(     j    y     k    С(     l    Т(     m    ы     n    У(     o    Ф(     p    Х(     q    Ц(     r    Ч(     s    Ш(     t    Щ(     u    Ъ(     v    “(     w    Ы(     x    Ь(     y    Э(     z    Ю(     {    Я(     |    а(     }    б(     ~    в(         г(     Ђ    д(     Ѓ    е(     ‚    ж(     ѓ    з(     „    и(     …    й(     †    к(     ‡    л(     €    м(     ‰    н(     Љ    о(     ‹    п(     Њ    р(     Ќ    с(     Ћ    т(     Џ    у(     ђ    ф(     ‘    х(     ’    ц(     “    ч(     ”    ш(     •    щ(     –    ъ(     —    ы(         ь(     ™    э(     љ    ю(     ›    я(     њ     )     ќ    )     ћ    )     џ    )          )     Ў    x     ў    i(     Ј    )     ¤    )     Ґ    s     ¦    )     §    )     Ё    	)     ©    
)     Є    )     «    )     ¬    )     ­    )     ®    )     Ї    )     °    )     ±    )     І    t     і    )     ґ    d(     µ    )     ¶    )     ·    )     ё    )     №    h(     є    )     »    )     ј    )     Ѕ    )     ѕ    )     ї    )     А    )     Б    )     В     )     Г    
)     Д    !)     Е    ")     Ж    #)     З    $)     И    %)     Й    &)     К    ')     Л    ‡     М    Ѓ     Н    ](     О    ()     П    ))     Р    *)     С    +)     Т    ,)     У    -)     Ф    .)     Х    /)     Ц    Y(     Ч    0)     Ш    1)     Щ    2)     Ъ    3)     Ы    4)     Ь    5)     Э    6)     Ю    7)     а    8)     б    ‹(     в    9)     г    :)     д    ;)     е    <)     ж    =)     з    >)     и    ?)     й    @)     к    h     л    A)     м    B)     н    C)     о    D)     п    E)     р    F)     с    G)     т    H)     у    I)     ф    J)     х    K)     ц    L)     ч    M)     ш    )     щ    N)     ъ    O)     ы    P)     ь    Q)     э    R)     ю    S)     я    T)          U)         V)         W)         X)         Y)         Z)         [)         \)         ])     	    ^)     
    _)         `)         a)         b)         [(         c)         d)         e)         f)         g)         h)         i)         j)         k)         l)         m)         n)         o)         p)         q)         r)         s)          t)     !    u)     "    v)     #    w)     $    x)     %    y)     &    z)     '    {)     (    w     3    …     4    |)     5    })     6    ~)     7    )     8          9    Ђ)     :    Ѓ)     ;    ‚)     <    ѓ)     =    „)     >    )     ?    5)     @    …)     A    †)     B    ‡)     C    €)     D    ‰)     E    Љ)     F    ‹)     G    Њ)     H    Ќ)     I    Ћ)     J    Џ)     K    ђ)     N    ‘)     O    ’)     P    “)     Q    ”)     R    •)     S    –)     T    —)     U    п     V    )     W    ™)     X    љ)     Y    ›)     Z    њ)     [    ќ)     \    ћ)     ]    џ)     ^     )     _    Ў)     `    ў)     a    Ј)     b    ¤)     c    Ґ)     d    ¦)     e    §)     f    Ё)     g    ©)     h    Є)     i    «)     j    ¬)     k    ­)     l    ®)     m    Ї)     n    °)     o    l     p    u     q    r     r    q     s    ±)     t    І)     u    %)     v    і)     w    ґ)     x    µ)     y    Њ(     z    п     {    ¶)     |    ·)     }    ё)     ~    №)         є)     Ђ    »)     Ѓ    ј)     ‚    Ѕ)     ѓ    ѕ)     „    ї)     …    А)     †    Б)     ‡    В)     €    Г)     ‰    Д)     Љ    Е)     ‹    Ж)     Њ    З)     Ќ    ђ)     Ћ    И)     Џ    Й)     ђ    К)     ‘    Л)     ’    М)     “    Н)     ”    О)     •    П)     –    Р)     —    С)         A      ™    Т)     љ    У)     ›    Ф)     њ    Х)     ќ    Ц)     ћ    Ч)     џ    Ш)          {     Ў    S)     ў    Щ)     Ј    Ъ)     ¤    Ы)     Ґ    Ь)     ¦    Э)     §    ¤)     Ё    Ґ)     ©    ¦)     Є    §)     «    Ё)     ¬    Ю)  D      й  и  0       tr                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    д)     *    е)     +    ж)  D ш     й  и  0       tr                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    з)  D $     й  и  0       cs                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяя°  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р       яяяя   0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          яяяяяяяя   Ъ    Ю     Ы    и)     Ь    й)     Э    к)     °    л)     Ю    м)     Я    н)     а    о)     б    п)     в    р)     г    с)     д    т)     е    у)     ж    ф)     з    х)     и    ц)     й    ч)     к    ш)     л    щ)     м    ц)     н    v     о    ъ)     п    ы)     р    ь)     с          т    п)     х    э)     ц    ю)     ч    я)     ш     *     щ    *     ъ    *     ы    *     ь    *     э    *     ю    *     я    *          *         	*         
*         *         *         *         *         *         *     	    *     
    *         *         *         *         *         *         *         *         *         ‚         *         *         Я         ъ         *         *         *         *          *         !*         "*         #*          $*     !    %*     "    &*     #    '*     $    (*     %    )*     &    **     '    +*     (    ,*     )    -*     *    .*     +    /*     ,    0*     -    1*     .    2*     /    3*     0    4*     1    5*     2    6*     3    7*     4    8*     5    9*     6    :*     7    :*     8    ;*     9    <*     :    =*     ;    >*     <    ?*     =    @*     >    A*     ?    B*     @    C*     A    D*     B    E*     C    а     D    F*     E    G*     F    H*     G    I*     H    i     I    p     J    m     K    n     L    `     M    o     N    j     O    k     P         Q    ~     R    v     S    J*     T    K*     U    L*     W    M*     X    N*     Y    O*     Z    P*     [    Q*     \    R*     ]    S*     ^    T*     `    U*     a    V*     b    W*     c    X*     d    Y*     e    Z*     f    [*     g    ц)     h    *     i    \*     j    †	     k    ]*     l    ^*     m    б     n    _*     o    `*     p    a*     q    `*     r    a*     s    b*     t    c*     u    d*     v    #*     w    b*     x    c*     y    e*     z    f*     {    e*     |    f*     }    g*     ~    h*         i*     Ђ    j*     Ѓ    k*     ‚    l*     ѓ    k*     „    l*     …    m*     †    n*     ‡    o*     €    p*     ‰    q*     Љ    r*     ‹    s*     Њ    t*     Ќ    u*     Ћ    v*     Џ    w*     ђ    x*     ‘    y*     ’    z*     “    {*     ”    |*     •    }*     –    ~*     —    *         Ђ*     ™    Ѓ*     љ    ‚*     ›    ѓ*     њ    „*     ќ    ]+     ћ    …*     џ    †*          ‡*     Ў    x     ў    ъ)     Ј    €*     ¤    ‰*     Ґ    s     ¦    Љ*     §    ‹*     Ё    Њ*     ©    в     Є    Ќ*     «    Ћ*     ¬    г     ­    Џ*     ®    ђ*     Ї    д     °    ‘*     ±    ’*     І    t     і    “*     ґ    ц)     µ    ”*     ¶    •*     ·    –*     ё    —*     №    v     є    *     »    ™*     ј    љ*     Ѕ    ›*     ѕ    њ*     ї    ќ*     А    ћ*     Б    џ*     В     *     Г    в     Д    е     Е    Ў*     Ж    ў*     З    Ј*     И    ж     Й    ¤*     К    Ґ*     Л    ¦*     М    Ѓ     Н    п)     О    §*     П    Ё*     Р    ©*     С    Є*     Т    «*     У    ¬*     Ф    з     Х    ­*     Ц    л)     Ч    к)     Ш    ®*     Щ    Ї*     Ъ    °*     Ы    ‰*     Ь    ±*     Э    І*     Ю    і*     а    ґ*     б    µ*     в    ¶*     г    ·*     д    ё*     е    №*     ж    є*     з    »*     и    ј*     й    Ѕ*     к    h     л    ѕ*     м    ї*     н    А*     о    Б*     п    В*     р    Г*     с    Д*     т    Е*     у    Ж*     ф    З*     х    И*     ц    Й*     ч    К*     ш    —*     щ    Л*     ъ    М*     ы    Н*     ь    О*     э    П*     ю    Р*     я    С*          Т*         У*         Ф*         Х*         Ц*         Ч*         Ш*         Щ*         €     	    и     
    Ъ*         Ы*         \+         Ь*         н)         Э*         Ю*         Я*         а*         б*         в*         г*         д*         е*         ж*         з*         и*         й*         к*         л*         м*         н*          о*     !    п*     "    р*     #    с*     $    т*     %    у*     &    ф*     '    х*     (    w     3    ц*     4    ч*     5    ш*     6    щ*     7    ъ*     8          9    ы*     :    ь*     ;    э*     <    ю*     =    я*     >    ‰*     ?    ±*     @     +     A    +     B    +     C    +     D    +     E    +     F    +     G    +     H    +     I    	+     J    
+     K    +     N    +     O    +     P    +     Q    +     R    +     S    ‰     T    +     U    Я     V    +     W    +     X    +     Y    +     Z    +     [    +     \    +     ]    +     ^    +     _    +     `    +     a    +     b    +     c    +     d     +     e    !+     f    "+     g    #+     h    $+     i    %+     j    &+     k    '+     l    (+     m    )+     n    *+     o    l     p    u     q    q     r    q     s    ++     t    ,+     u    ж     v    -+     w    |     x    ѓ     y    *     z    Я     {    й     |    .+     }    /+     ~    0+         1+     Ђ    2+     Ѓ    3+     ‚    4+     ѓ    5+     „    6+     …    7+     †    8+     ‡    9+     €    ч*     ‰    :+     Љ    ;+     ‹    <+     Њ    =+     Ќ    >+     Ћ    ?+     Џ    @+     ђ    A+     ‘    B+     ’    C+     “    D+     ”    E+     •    к     –    F+     —    G+         B      ™    H+     љ    I+     ›    J+     њ    K+     ќ    L+     ћ    M+     џ    N+          щ     Ў    O+     ў    P+     Ј    Q+     ¤    R+     Ґ    S+     ¦    T+     §    +     Ё    +     ©     +     Є    !+     «    "+     ¬    U+  D      й  и  0       cs                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    \+     *    ^+     +    _+  D ш     й  и  0       cs                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    ]+  D ($     й  и  0       es                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяА  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяяяяяP  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя   Ъ    `+     Ы    a+     Ь    M,     Э    N,     °    b+     Ю    c+     Я    d+     а    e+     б    P,     в    f+     г    K,     д    g+     е    h+     ж    Q,     з    i+     и    j+     й    R,     к    S,     л    k+     м    j+     н    v     о    T,     п    l+     р    U,     с    Ђ     т    P,     х    V,     ц    9,     ч    E,     ш    m+     щ    n+     ъ    o+     ы    p+     ь    q+     э    W,     ю    Ю,     я    r+          s+         t+         X,         u+         v+         w+         x+         x+         Y,     	    y+     
    z+         {+         |+         Z,         [,         \,         }+         ],         ~+         e         +         ^,         _,         `,         F,         G,         a,         b,         c,         d,         e,         f,          g,     !    Ђ+     "    h,     #  l = 0;

	if (!sd) {
		printk(KERN_DEBUG "CPU%d attaching NULL sched-domain.\n", cpu);
		return;
	}

	printk(KERN_DEBUG "CPU%d attaching sched-domain:\n", cpu);

	if (!alloc_cpumask_var(&groupmask, GFP_KERNEL)) {
		printk(KERN_DEBUG "Cannot load-balance (out of memory)\n");
		return;
	}

	for (;;) {
		if (sched_domain_debug_one(sd, cpu, level, groupmask))
			break;
		level++;
		sd = sd->parent;
		if (!sd)
			break;
	}
	free_cpumask_var(groupmask);
}
#else /* !CONFIG_SCHED_DEBUG */
# define sched_domain_debug(sd, cpu) do { } while (0)
#endif /* CONFIG_SCHED_DEBUG */

static int sd_degenerate(struct sched_domain *sd)
{
	if (cpumask_weight(sched_domain_span(sd)) == 1)
		return 1;

	/* Following flags need at least 2 groups */
	if (sd->flags & (SD_LOAD_BALANCE |
			 SD_BALANCE_NEWIDLE |
			 SD_BALANCE_FORK |
			 SD_BALANCE_EXEC |
			 SD_SHARE_CPUPOWER |
			 SD_SHARE_PKG_RESOURCES)) {
		if (sd->groups != sd->groups->next)
			return 0;
	}

	/* Following flags don't use groups */
	if (sd->flags & (SD_WAKE_IDLE |
			 SD_WAKE_AFFINE |
			 SD_WAKE_BALANCE))
		return 0;

	return 1;
}

static int
sd_parent_degenerate(struct sched_domain *sd, struct sched_domain *parent)
{
	unsigned long cflags = sd->flags, pflags = parent->flags;

	if (sd_degenerate(parent))
		return 1;

	if (!cpumask_equal(sched_domain_span(sd), sched_domain_span(parent)))
		return 0;

	/* Does parent contain flags not in child? */
	/* WAKE_BALANCE is a subset of WAKE_AFFINE */
	if (cflags & SD_WAKE_AFFINE)
		pflags &= ~SD_WAKE_BALANCE;
	/* Flags needing groups don't count if only 1 group in parent */
	if (parent->groups == parent->groups->next) {
		pflags &= ~(SD_LOAD_BALANCE |
				SD_BALANCE_NEWIDLE |
				SD_BALANCE_FORK |
				SD_BALANCE_EXEC |
				SD_SHARE_CPUPOWER |
				SD_SHARE_PKG_RESOURCES);
		if (nr_node_ids == 1)
			pflags &= ~SD_SERIALIZE;
	}
	if (~cflags & pflags)
		return 0;

	return 1;
}

static void free_rootdomain(struct root_domain *rd)
{
	cpupri_cleanup(&rd->cpupri);

	free_cpumask_var(rd->rto_mask);
	free_cpumask_var(rd->online);
	free_cpumask_var(rd->span);
	kfree(rd);
}

static void rq_attach_root(struct rq *rq, struct root_domain *rd)
{
	struct root_domain *old_rd = NULL;
	unsigned long flags;

	spin_lock_irqsave(&rq->lock, flags);

	if (rq->rd) {
		old_rd = rq->rd;

		if (cpumask_test_cpu(rq->cpu, old_rd->online))
			set_rq_offline(rq);

		cpumask_clear_cpu(rq->cpu, old_rd->span);

		/*
		 * If we dont want to free the old_rt yet then
		 * set old_rd to NULL to skip the freeing later
		 * in this function:
		 */
		if (!atomic_dec_and_test(&old_rd->refcount))
			old_rd = NULL;
	}

	atomic_inc(&rd->refcount);
	rq->rd = rd;

	cpumask_set_cpu(rq->cpu, rd->span);
	if (cpumask_test_cpu(rq->cpu, cpu_online_mask))
		set_rq_online(rq);

	spin_unlock_irqrestore(&rq->lock, flags);

	if (old_rd)
		free_rootdomain(old_rd);
}

static int init_rootdomain(struct root_domain *rd, bool bootmem)
{
	gfp_t gfp = GFP_KERNEL;

	memset(rd, 0, sizeof(*rd));

	if (bootmem)
		gfp = GFP_NOWAIT;

	if (!alloc_cpumask_var(&rd->span, gfp))
		goto out;
	if (!alloc_cpumask_var(&rd->online, gfp))
		goto free_span;
	if (!alloc_cpumask_var(&rd->rto_mask, gfp))
		goto free_online;

	if (cpupri_init(&rd->cpupri, bootmem) != 0)
		goto free_rto_mask;
	return 0;

free_rto_mask:
	free_cpumask_var(rd->rto_mask);
free_online:
	free_cpumask_var(rd->online);
free_span:
	free_cpumask_var(rd->span);
out:
	return -ENOMEM;
}

static void init_defrootdomain(void)
{
	init_rootdomain(&def_root_domain, true);

	atomic_set(&def_root_domain.refcount, 1);
}

static struct root_domain *alloc_rootdomain(void)
{
	struct root_domain *rd;

	rd = kmalloc(sizeof(*rd), GFP_KERNEL);
	if (!rd)
		return NULL;

	if (init_rootdomain(rd, false) != 0) {
		kfree(rd);
		return NULL;
	}

	return rd;
}

/*
 * Attach the domain 'sd' to 'cpu' as its base domain. Callers must
 * hold the hotplug lock.
 */
static void
cpu_attach_domain(struct sched_domain *sd, struct root_domain *rd, int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	struct sched_domain *tmp;

	/* Remove the sched domains which do not contribute to scheduling. */
	f  п+     &    I,     '    р+     (    w     3    с+     4    т+     5    у+     6    ѕ,     7    ф+     8          9    х+     :    ц+     ;    ї,     <    ч+     =    ш+     >    ђ,     ?    Ё,     @    щ+     A    ъ+     B    А,     C    ы+     D    ь+     E    Б,     F    э+     G    В,     H    ю+     I    Г,     J    я+     K    Д,     N    Е,     O     ,     P    Ж,     Q    ,     R    ,     S    ,     T    З,     U    _,     V    ,     W    ,     X    ,     Y    И,     Z    ,     [    ,     \    Й,     ]    К,     ^    Л,     _    М,     `    Н,     a    	,     b    
,     c    ,     d    О,     e    П,     f    ,     g    ,     h    ,     i    Р,     j    б,     k    ,     l    С,     m    ,     n    ,     o    l     p    u     q    r     r    q     s    J,     t    L,     u    ћ,     v    Т,     w    ,     x    ,     y    F,     z    _,     {    ,     |    У,     }    Ф,     ~    Х,         Ц,     Ђ    ,     Ѓ    ,     ‚    ,     ѓ    Ч,     „    Ш,     …    ,     †    ,     ‡    ,     €    ,     ‰    ,     Љ    ,     ‹    ,     Њ    ,     Ќ    Д,     Ћ     ,     Џ    Щ,     ђ    !,     ‘    Ъ,     ’    ",     “    #,     ”    $,     •    Ы,     –    %,     —    &,         C      ™    ',     љ    (,     ›    ),     њ    *,     ќ    +,     ћ    ,,     џ    -,          {     Ў    L,     ў    .,     Ј    /,     ¤    0,     Ґ    1,     ¦    Ь,     §    
,     Ё    ,     ©    О,     Є    П,     «    ,     ¬    Э,  D      й  и  0       es                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    8,     *    в,     +    г,  D ш     й  и  0       es                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    д,  D $     й  и  0       it                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяя°  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р       яяяя   0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          яяяяяяяя   Ъ    е,     Ы    d.     Ь    ж,     Э    з,     °    и,     Ю    й,     Я    к,     а    л,     б    м,     в    н,     г    о,     д    п,     е    р,     ж    с,     з    т,     и    у,     й    ф,     к    х,     л    ц,     м    у,     н    v     о    ч,     п    ш,     р    “     с    Ђ     т    м,     х    щ,     ц    ъ,     ч    ы,     ш    ь,     щ    э,     ъ    ю,     ы    я,     ь     -     э    -     ю    -     я    -          -         -         -         -         -         	-         
-         
-         -     	    -     
    -         -         -         -         -         -         -         -         ”         ‚         -         i         -         -         -         -         -         -         -         -         -         -           -     !    !-     "    "-     #    #-     $    $-     %    %-     &    &-     '    '-     (    (-     )    )-     *    *-     +    +-     ,    ,-     -    --     .    .-     /    /-     0    0-     1    1-     2    2-     3    3-     4    4-     5    5-     6    6-     7    6-     8    7-     9    8-     :    9-     ;    :-     <    ;-     =    <-     >    =-     ?    >-     @    j     A    ?-     B    @-     C    A-     D    ‰	     E    Љ	     F    B-     G    C-     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P         Q    ~     R    v     S    D-     T    f.     U    E-     W    F-     X    G-     Y    H-     Z    I-     [    J-     \    K-     ]    L-     ^    M-     `    N-     a    O-     b    P-     c    Q-     d    R-     e    S-     f    T-     g    у,     h    U-     i    V-     j    y     k    W-     l    X-     m    Y-     n    Z-     o    [-     p    \-     q    ]-     r    ^-     s    _-     t    `-     u    a-     v    -     w    b-     x    c-     y    d-     z    e-     {    f-     |    g-     }    h-     ~    i-         j-     Ђ    k-     Ѓ    l-     ‚    m-     ѓ    n-     „    o-     …    p-     †    q-     ‡    r-     €    s-     ‰    t-     Љ    u-     ‹    v-     Њ    w-     Ќ    x-     Ћ    y-     Џ    z-     ђ    {-     ‘    |-     ’    }-     “    ~-     ”    -     •    Ђ-     –    Ѓ-     —    ‚-         ѓ-     ™    „-     љ    e.     ›    …-     њ    †-     ќ    ‡-     ћ    €-     џ    ‰-          †     Ў    x     ў    ч,     Ј    Љ-     ¤    ‹-     Ґ    s     ¦    Њ-     §    Ќ-     Ё    Ћ-     ©    Џ-     Є    ђ-     «    ‘-     ¬    ’-     ­    “-     ®    ”-     Ї    •-     °    –-     ±    —-     І    t     і    -     ґ    у,     µ    3"     ¶    ™-     ·    љ-     ё    ›-     №    v     є    њ-     »    ќ-     ј    ћ-     Ѕ    џ-     ѕ     -     ї    Ў-     А    ў-     Б    Ј-     В    ¤-     Г    Џ-     Д    Ђ     Е    Ґ-     Ж    ¦-     З    §-     И    Ё-     Й    ©-     К    Є-     Л    ‡     М    Ѓ     Н    м,     О    «-     П    ¬-     Р    ­-     С    ®-     Т    Ї-     У    °-     Ф    ±-     Х    І-     Ц    и,     Ч    і-     Ш    ґ-     Щ    k     Ъ    µ-     Ы    ‹-     Ь    ¶-     Э    ·-     Ю    ё-     а    №-     б    є-     в    »-     г    ј-     д    Ѕ-     е    ѕ-     ж    ї-     з    А-     и    Б-     й    В-     к    h     л    Г-     м    Д-     н    Е-     о    Ж-     п    З-     р    И-     с    Й-     т    К-     у    Л-     ф    М-     х    Н-     ц    О-     ч    П-     ш    ›-     щ    Р-     ъ    С-     ы    Т-     ь    У-     э    Ф-     ю    Х-     я    Ц-          Ч-         Ш-         Щ-         Ъ-         Ы-         Ь-         Э-         Ю-         z     	    Я-     
    а-         б-         в-         г-         к,         д-         е-         ж-         з-         и-         й-         к-         л-         м-         н-         о-         п-         р-         с-         т-         у-         ф-          х-     !    ц-     "    ч-     #    ш-     $    щ-     %    ъ-     &    l     '    ы-     (    w     3    …     4    ь-     5    э-     6    ю-     7    я-     8          9     .     :    .     ;    .     <    .     =    .     >    .     ?    ¶-     @    .     A    .     B    .     C    	.     D    
.     E    .     F    .     G    .     H    .     I    .     J    .     K    .     N    .     O    .     P    .     Q    .     R    .     S    ’     T    .     U    -     V    .     W    .     X    .     Y    .     Z    .     [    .     \    .     ]    .     ^     .     _    !.     `    ".     a    #.     b    $.     c    %.     d    &.     e    '.     f    (.     g    ).     h    *.     i    +.     j    ,.     k    -.     l    ..     m    /.     n    0.     o    l     p    u     q    r     r    q     s    1.     t    2.     u    Ё-     v    3.     w    4.     x    5.     y    -     z    -     {    6.     |    7.     }    8.     ~    9.         :.     Ђ    ;.     Ѓ    <.     ‚    =.     ѓ    >.     „    ?.     …    @.     †    A.     ‡    B.     €    C.     ‰    D.     Љ    E.     ‹    F.     Њ    G.     Ќ    .     Ћ    H.     Џ    I.     ђ    J.     ‘    K.     ’    L.     “    M.     ”    N.     •    •     –    O.     —    P.         D      ™    Q.     љ    R.     ›    S.     њ    T.     ќ    U.     ћ    V.     џ    W.          щ     Ў    Х-     ў    X.     Ј    Y.     ¤    Z.     Ґ    [.     ¦    \.     §    $.     Ё    %.     ©    &.     Є    '.     «    (.     ¬    ].  D      й  и  0       it                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    d.     *    g.     +    h.  D ш     й  и  0       it                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    e.  D 8$     й  и  0       lt                                      яяяяяяяяяяяяяяяяяяяяяяяя    яяяя       0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ     яяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяP  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяР  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  яяяяяяяя`  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяяяяя   Щ    ‡	     Ъ    i.     Ы    j.     Ь    k.     Э    l.     °    m.     Ю    n.     Я    o.     а    p.     б    q.     в    r.     г    s.     д    t.     е    u.     ж    v.     з    w.     и    x.     й    y.     к    z.     л    {.     м    x.     н    v     о    |.     п    }.     р    ~.     с          т    q.     х    .     ц    Ђ.     ч    Ѓ.     ш    ‚.     щ    ѓ.     ъ    „.     ы    ….     ь    †.     э    ‡.     ю    €.     я    ‰.          Љ.         ‹.         Њ.         Ќ.         Ћ.         Џ.         ђ.         ђ.         ‘.     	    ’.     
    “.         ”.         •.         –.         —.         .         ™.         љ.         ›.         4"         њ.         ќ.         ћ.         џ.          .         Ў.         ў.         Ј.         ¤.         Ґ.         ¦.         §.          Ё.     !    ©.     "    Є.     #    «.     $    ¬.     %    ­.     &    ®.     '    Ї.     (    °.     )    ±.     *    І.     +    і.     ,    ґ.     -    µ.     .    ¶.     /    ·.     0    ё.     1    №.     2    є.     3    ».     4    ј.     5    Ѕ.     6    ѕ.     7    ѕ.     8    ї.     9    А.     :    Б.     ;    В.     <    Г.     =    Д.     >    Е.     ?    Ж.     @    З.     A    И.     B    Й.     C    К.     D    Л.     E    М.     F    Н.     G    О.     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P    П.     Q    Р.     R    v     S    С.     T    Т.     U    У.     V    Ф.     W    Х.     X    Ц.     Y    Ч.     Z    Ш.     [    Щ.     \    Ъ.     ]    Ы.     ^    Ь.     `    Э.     a    Ю.     b    Я.     c    а.     d    б.     e    в.     f    г.     g    x.     h    д.     i    е.     j    y     k    ж.     l    з.     m    и.     n    й.     o    к.     p    л.     q    к.     r    л.     s    Ё.     t    м.     u    н.     v    §.     w    Ё.     x    м.     y    о.     z    п.     {    о.     |    п.     }    р.     ~    с.         т.     Ђ    у.     Ѓ    ф.     ‚    х.     ѓ    ф.     „    х.     …    ц.     †    ч.     ‡    ш.     €    щ.     ‰    ъ.     Љ    ы.     ‹    ь.     Њ    э.     Ќ    ю.     Ћ    я.     Џ     /     ђ    /     ‘    /     ’    /     “    /     ”    /     •    /     –    /     —    /         	/     ™    
/     љ    /     ›    /     њ    /     ќ    /     ћ    /     џ    /          /     Ў    /     ў    |.     Ј    /     ¤    /     Ґ    s     ¦    /     §    /     Ё    /     ©    /     Є    /     «    /     ¬    /     ­    /     ®    /     Ї    /     °    /     ±     /     І    t     і    !/     ґ    x.     µ    "/     ¶    #/     ·    $/     ё    %/     №    v     є    &/     »    '/     ј    (/     Ѕ    )/     ѕ    */     ї    +/     А    ,/     Б    -/     В    ./     Г    /     Д    //     Е    0/     Ж    1/     З    2/     И    3/     Й    4/     К    5/     Л    6/     М    7/     Н    q.     О    –     П    8/     Р    9/     С    :/     Т    ;/     У    </     Ф    =/     Х    >/     Ц    m.     Ч    l.     Ш    ?/     Щ    @/     Ъ    A/     Ы    B/     Ь    C/     Э    D/     Ю    E/     а    F/     б    G/     в    H/     г    I/     д    J/     е    K/     ж    L/     з    M/     и    M/     й    N/     к    h     л    O/     м    P/     н    Q/     о    R/     п    S/     р    T/     с    U/     т    с/     у    V/     ф    W/     х    X/     ц    Y/     ч    Z/     ш    %/     щ    [/     ъ    \/     ы    ]/     ь    ^/     э    _/     ю    `/     я    a/          b/         c/         d/         e/         f/         g/         h/         i/         с     	    j/     
    k/         l/         m/         n/         o.         o/         p/         q/         r/         s/         t/         u/         v/         w/         x/         y/         z/         {/         |/         }/         ~/         /          Ђ/     !    Ѓ/     "    ‚/     #    ѓ/     $    „/     %    …/     &    †/     '    ‡/     (    w     3    €/     4    ‰/     5    Љ/     6    ‹/     7    Њ/     8          9    Ќ/     :    Ћ/     ;    Џ/     <    ђ/     =    ‘/     >    /     ?    ’/     @    “/     A    ”/     B    •/     C    –/     D    —/     E    /     F    ™/     G    љ/     H    ›/     I    њ/     J    ќ/     K    ћ/     N    џ/     O     /     P    Ў/     Q    ў/     R    Ј/     S    ¤/     T    Ґ/     U    ћ.     V    ¦/     W    §/     X    Ё/     Y    ©/     Z    Є/     [    «/     \    ¬/     ]    ­/     ^    ®/     _    Ї/     `    °/     a    ±/     b    І/     c    і/     d    ґ/     e    µ/     f    ¶/     g    ·/     h    ё/     i    №/     j    є/     k    »/     l    ј/     m    Ѕ/     n    ѕ/     o    l     p    u     q    r     r    q     s    ї/     t    А/     u    3/     v    Б/     w    |     x    В/     y     .     z    Г/     {    Д/     |    Е/     }    Ж/     ~    З/         И/     Ђ    Й/     Ѓ    К/     ‚    Л/     ѓ    М/     „    Н/     …    О/     †    П/     ‡    Р/     €    ‰/     ‰    С/     Љ    Т/     ‹    У/     Њ    Ф/     Ќ    ћ/     Ћ    Х/     Џ    Ц/     ђ    Ч/     ‘    Ш/     ’    Щ/     “    Ъ/     ”    Ы/     •    Ь/     –    Э/     —    Ю/         E      ™    Я/     љ    а/     ›    б/     њ    в/     ќ    г/     ћ    д/     џ    е/          щ     Ў    `/     ў    ж/     Ј    з/     ¤    и/     Ґ    й/     ¦    к/     §    І/     Ё    і/     ©    ґ/     Є   sysctl_timer_migration = 1;

int in_sched_functions(unsigned long addr)
{
	return in_lock_functions(addr) ||
		(addr >= (unsigned long)__sched_text_start
		&& addr < (unsigned long)__sched_text_end);
}

static void init_cfs_rq(struct cfs_rq *cfs_rq, struct rq *rq)
{
	cfs_rq->tasks_timeline = RB_ROOT;
	INIT_LIST_HEAD(&cfs_rq->tasks);
#ifdef CONFIG_FAIR_GROUP_SCHED
	cfs_rq->rq = rq;
#endif
	cfs_rq->min_vruntime = (u64)(-(1LL << 20));
}

static void init_rt_rq(struct rt_rq *rt_rq, struct rq *rq)
{
	struct rt_prio_array *array;
	int i;

	array = &rt_rq->active;
	for (i = 0; i < MAX_RT_PRIO; i++) {
		INIT_LIST_HEAD(array->queue + i);
		__clear_bit(i, array->bitmap);
	}
	/* delimiter for bitsearch: */
	__set_bit(MAX_RT_PRIO, array->bitmap);

#if defined CONFIG_SMP || defined CONFIG_RT_GROUP_SCHED
	rt_rq->highest_prio.curr = MAX_RT_PRIO;
#ifdef CONFIG_SMP
	rt_rq->highest_prio.next = MAX_RT_PRIO;
#endif
#endif
#ifdef CONFIG_SMP
	rt_rq->rt_nr_migratory = 0;
	rt_rq->overloaded = 0;
	plist_head_init(&rt_rq->pushable_tasks, &rq->lock);
#endif

	rt_rq->rt_time = 0;
	rt_rq->rt_throttled = 0;
	rt_rq->rt_runtime = 0;
	spin_lock_init(&rt_rq->rt_runtime_lock);

#ifdef CONFIG_RT_GROUP_SCHED
	rt_rq->rt_nr_boosted = 0;
	rt_rq->rq = rq;
#endif
}

#ifdef CONFIG_FAIR_GROUP_SCHED
static void init_tg_cfs_entry(struct task_group *tg, struct cfs_rq *cfs_rq,
				struct sched_entity *se, int cpu, int add,
				struct sched_entity *parent)
{
	struct rq *rq = cpu_rq(cpu);
	tg->cfs_rq[cpu] = cfs_rq;
	init_cfs_rq(cfs_rq, rq);
	cfs_rq->tg = tg;
	if (add)
		list_add(&cfs_rq->leaf_cfs_rq_list, &rq->leaf_cfs_rq_list);

	tg->se[cpu] = se;
	/* se could be NULL for init_task_group */
	if (!se)
		return;

	if (!parent)
		se->cfs_rq = &rq->cfs;
	else
		se->cfs_rq = parent->my_q;

	se->my_q = cfs_rq;
	se->load.weight = tg->shares;
	se->load.inv_weight = 0;
	se->parent = parent;
}
#endif

#ifdef CONFIG_RT_GROUP_SCHED
static void init_tg_rt_entry(struct task_group *tg, struct rt_rq *rt_rq,
		struct sched_rt_entity *rt_se, int cpu, int add,
		struct sched_rt_entity *parent)
{
	struct rq *rq = cpu_rq(cpu);

	tg->rt_rq[cpu] = rt_rq;
	init_rt_rq(rt_rq, rq);
	rt_rq->tg = tg;
	rt_rq->rt_se = rt_se;
	rt_rq->rt_runtime = tg->rt_bandwidth.rt_runtime;
	if (add)
		list_add(&rt_rq->leaf_rt_rq_list, &rq->leaf_rt_rq_list);

	tg->rt_se[cpu] = rt_se;
	if (!rt_se)
		return;

	if (!parent)
		rt_se->rt_rq = &rq->rt;
	else
		rt_se->rt_rq = parent->my_q;

	rt_se->my_q = rt_rq;
	rt_se->parent = parent;
	INIT_LIST_HEAD(&rt_se->run_list);
}
#endif

void __init sched_init(void)
{
	int i, j;
	unsigned long alloc_size = 0, ptr;

#ifdef CONFIG_FAIR_GROUP_SCHED
	alloc_size += 2 * nr_cpu_ids * sizeof(void **);
#endif
#ifdef CONFIG_RT_GROUP_SCHED
	alloc_size += 2 * nr_cpu_ids * sizeof(void **);
#endif
#ifdef CONFIG_USER_SCHED
	alloc_size *= 2;
#endif
#ifdef CONFIG_CPUMASK_OFFSTACK
	alloc_size += num_possible_cpus() * cpumask_size();
#endif
	/*
	 * As sched_init() is called before page_alloc is setup,
	 * we use alloc_bootmem().
	 */
	if (alloc_size) {
		ptr = (unsigned long)kzalloc(alloc_size, GFP_NOWAIT);

#ifdef CONFIG_FAIR_GROUP_SCHED
		init_task_group.se = (struct sched_entity **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);

		init_task_group.cfs_rq = (struct cfs_rq **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);

#ifdef CONFIG_USER_SCHED
		root_task_group.se = (struct sched_entity **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);

		root_task_group.cfs_rq = (struct cfs_rq **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);
#endif /* CONFIG_USER_SCHED */
#endif /* CONFIG_FAIR_GROUP_SCHED */
#ifdef CONFIG_RT_GROUP_SCHED
		init_task_group.rt_se = (struct sched_rt_entity **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);

		init_task_group.rt_rq = (struct rt_rq **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);

#ifdef CONFIG_USER_SCHED
		root_task_group.rt_se = (struct sched_rt_entity **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);

		root_task_group.rt_rq = (struct rt_rq **)ptr;
		ptr += nr_cpu_ids * sizeof(void **);
#endif /* CONFIG_USER_SCHED */
#endif /* CONFIG_RT_GROUP_SCHED */
#ifdef CONFIG_CPUMASK_OFFSTACKяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    х/  D ш     й  и  0       lt                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    ц/  D $     й  и  0       pt                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяя°  А  Р  а  р          яяяя0  @  P  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р       яяяя   0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяяяяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  open_softirq(SCHED_SOFTIRQ, run_rebalance_domains);
#endif

#ifdef CONFIG_RT_MUTEXES
	plist_head_init(&init_task.pi_waiters, &init_task.pi_lock);
#endif

	/*
	 * The boot idle thread does lazy MMU switching as well:
	 */
	atomic_inc(&init_mm.mm_count);
	enter_lazy_tlb(&init_mm, current);

	/*
	 * Make us the idle thread. Technically, schedule() should not be
	 * called from this thread, however somewhere below it might be,
	 * but because we are the idle thread, we just pick up running again
	 * when this runqueue becomes "idle".
	 */
	init_idle(current, smp_processor_id());

	calc_load_update = jiffies + LOAD_FREQ;

	/*
	 * During early bootup we pretend to be a normal task:
	 */
	current->sched_class = &fair_sched_class;

	/* Allocate the nohz_cpu_mask if CONFIG_CPUMASK_OFFSTACK */
	alloc_cpumask_var(&nohz_cpu_mask, GFP_NOWAIT);
#ifdef CONFIG_SMP
#ifdef CONFIG_NO_HZ
	alloc_cpumask_var(&nohz.cpu_mask, GFP_NOWAIT);
	alloc_cpumask_var(&nohz.ilb_grp_nohz_mask, GFP_NOWAIT);
#endif
	alloc_cpumask_var(&cpu_isolated_map, GFP_NOWAIT);
#endif /* SMP */

	perf_counter_init();

	scheduler_running = 1;
}

#ifdef CONFIG_DEBUG_SPINLOCK_SLEEP
void __might_sleep(char *file, int line)
{
#ifdef in_atomic
	static unsigned long prev_jiffy;	/* ratelimiting */

	if ((!in_atomic() && !irqs_disabled()) ||
		    system_state != SYSTEM_RUNNING || oops_in_progress)
		return;
	if (time_before(jiffies, prev_jiffy + HZ) && prev_jiffy)
		return;
	prev_jiffy = jiffies;

	printk(KERN_ERR
		"BUG: sleeping function called from invalid context at %s:%d\n",
			file, line);
	printk(KERN_ERR
		"in_atomic(): %d, irqs_disabled(): %d, pid: %d, name: %s\n",
			in_atomic(), irqs_disabled(),
			current->pid, current->comm);

	debug_show_held_locks(current);
	if (irqs_disabled())
		print_irqtrace_events(current);
	dump_stack();
#endif
}
EXPORT_SYMBOL(__might_sleep);
#endif

#ifdef CONFIG_MAGIC_SYSRQ
static void normalize_task(struct rq *rq, struct task_struct *p)
{
	int on_rq;

	update_rq_clock(rq);
	on_rq = p->se.on_rq;
	if (on_rq)
		deactivate_task(rq, p, 0);
	__setscheduler(rq, p, SCHED_NORMAL, 0);
	if (on_rq) {
		activate_task(rq, p, 0);
		resched_task(rq->curr);
	}
}

void normalize_rt_tasks(void)
{
	struct task_struct *g, *p;
	unsigned long flags;
	struct rq *rq;

	read_lock_irqsave(&tasklist_lock, flags);
	do_each_thread(g, p) {
		/*
		 * Only normalize user tasks:
		 */
		if (!p->mm)
			continue;

		p->se.exec_start		= 0;
#ifdef CONFIG_SCHEDSTATS
		p->se.wait_start		= 0;
		p->se.sleep_start		= 0;
		p->se.block_start		= 0;
#endif

		if (!rt_task(p)) {
			/*
			 * Renice negative nice level userspace
			 * tasks back to 0:
			 */
			if (TASK_NICE(p) < 0 && p->mm)
				set_user_nice(p, 0);
			continue;
		}

		spin_lock(&p->pi_lock);
		rq = __task_rq_lock(p);

		normalize_task(rq, p);

		__task_rq_unlock(rq);
		spin_unlock(&p->pi_lock);
	} while_each_thread(g, p);

	read_unlock_irqrestore(&tasklist_lock, flags);
}

#endif /* CONFIG_MAGIC_SYSRQ */

#ifdef CONFIG_IA64
/*
 * These functions are only useful for the IA64 MCA handling.
 *
 * They can only be called when the whole system has been
 * stopped - every CPU needs to be quiescent, and no scheduling
 * activity can take place. Using them for anything else would
 * be a serious bug, and as a result, they aren't even visible
 * under any other configuration.
 */

/**
 * curr_task - return the current task for a given cpu.
 * @cpu: the processor in question.
 *
 * ONLY VALID WHEN THE WHOLE SYSTEM IS STOPPED!
 */
struct task_struct *curr_task(int cpu)
{
	return cpu_curr(cpu);
}

/**
 * set_curr_task - set the current task for a given cpu.
 * @cpu: the processor in question.
 * @p: the task pointer to set.
 *
 * Description: This function must only be used when non-maskable interrupts
 * are serviced on a separate stack. It allows the architecture to switch the
 * notion of the current task on a cpu in a non-blocking manner. This function
 * must be called with all CPU's synchronized, and interrupts disabled, the
 * and caller must save the original value of the current task (see
 * curr_task() above) and restore   o     Ь    Ћ0     Э    Џ0     Ю    ђ0     а    ‘0     б    #1     в    ’0     г    “0     д    I1     е    J1     ж    ”0     з    •0     и    –0     й    —0     к    h     л    K1     м    0     н    ™0     о    љ0     п    ›0     р    L1     с    M1     т    N1     у    њ0     ф    ќ0     х    ћ0     ц    H,     ч    џ0     ш    }0     щ    O1     ъ    P1     ы    Q1     ь     0     э    Ў0     ю    R1     я    ў0          Ј0         S1         T1         ¤0         Ґ0         ¦0         §0         Ё0         z     	    U1     
    ©0         Є0         1         «0         1         ¬0         ­0         ®0         Ї0         °0         ±0         І0         D,         і0         ґ0         µ0         ¶0         ·0         ё0         №0         є0         »0          ј0     !    Ѕ0     "    ѕ0     #    ї0     $    А0     %    V1     &    I,     '    W1     (    w     3    Б0     4    В0     5    Г0     6    Д0     7    Е0     8          9    Ж0     :    З0     ;    X1     <    И0     =    Й0     >    91     ?    Ћ0     @    К0     A    Л0     B    Y1     C    М0     D    Н0     E    Z1     F    О0     G    [1     H    П0     I    \1     J    Р0     K    С0     N    p     O    Т0     P    ]1     Q    ^1     R    У0     S    m     T    Ф0     U    0     V    Х0     W    _1     X    `1     Y    Ц0     Z    Ч0     [    Ш0     \    Щ0     ]    Ъ0     ^    Ы0     _    Ь0     `    Э0     a    a1     b    Ю0     c    Я0     d    а0     e    б0     f    в0     g    г0     h    д0     i    b1     j    е0     k    ж0     l    з0     m    и0     n    й0     o    l     p    u     q    r     r    q     s    J,     t    к0     u    D1     v    c1     w    |     x    ѓ     y    F,     z    0     {    1     |    d1     }    л0     ~    м0         н0     Ђ    e1     Ѓ    о0     ‚    п0     ѓ    р0     „    с0     …    т0     †    у0     ‡    ф0     €    х0     ‰    ц0     Љ    ч0     ‹    ш0     Њ    щ0     Ќ    С0     Ћ    ъ0     Џ    ы0     ђ    ь0     ‘    э0     ’    ю0     “    я0     ”     1     •    1     –    1     —    1         F      ™    1     љ    1     ›    1     њ    1     ќ    1     ћ    	1     џ    
1          {     Ў    R1     ў    1     Ј    1     ¤    1     Ґ    1     ¦    1     §    Ю0     Ё    Я0     ©    а0     Є    б0     «    в0     ¬    f1  D      й  и  0       pt                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    1     *    h1     +    g1  D ш     й  и  0       pt                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    1  D 8$     й  и  0       hu                                      яяяяяяяяяяяяяяяяяяяяяяяя    яяяя       0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ     яяяяяяяя°  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяP  `  p  Ђ  ђ     °  А  Р  а  р   	  	   	  0	  @	  P	  `	  p	  Ђ	  ђ	   	  °	  А	  Р	  а	  р	   
  
   
  0
  @
  P
  `
  p
  Ђ
  ђ
   
  °
  А
  Р
  а
  р
          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  яяяя@  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяР  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  яяяяяяяя`  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  яяяяяяяя   Щ    ‡	     Ъ    i1     Ы    j1     Ь    k1     Э    l1     °    m1     Ю    n1     Я    o1     а    p1     б    q1     в    r1     г    s1     д    t1     е    u1     ж    v1     з    w1     и    x1     й    y1     к    z1     л    {1     м    x1     н    v     о    |1     п    }1     р    ~1     с    Ђ     т    q1     х    1     ц    Ђ1     ч    Ѓ1     ш    ‚1     щ    ѓ1     ъ    „1     ы    …1     ь    †1     э    ‡1     ю    €1     я    ‰1          Љ1         ‹1         Њ1         Ќ1         Ћ1         Џ1         ђ1         ђ1         ‘1     	    ’1     
    “1         ”1         •1         –1         —1         1         ™1         љ1         ›1         Љ         њ1         ќ1         п         ъ         ћ1         џ1          1         Ў1         ў1         Ј1         ¤1         Ґ1          ¦1     !    §1     "    Ё1     #    ©1     $    Є1     %    «1     &    ¬1     '    ­1     (    ®1     )    Ї1     *    °1     +    ±1     ,    І1     -    і1     .    ґ1     /    µ1     0    ¶1     1    ·1     2    ё1     3    №1     4    є1     5    »1     6    ј1     7    ј1     8    Ѕ1     9    ѕ1     :    ї1     ;    А1     <    Б1     =    В1     >    Г1     ?    Д1     @    Е1     A    Ж1     B    З1     C    И1     D    Й1     E    К1     F    Л1     G    М1     H    i     I    p     J    m     K    n     L    }     M    o     N    j     O    k     P    Н1     Q    ~     R    v     S    О1     T    П1     U    Р1     V    С1     W    Т1     X    q1     Y    У1     Z    Ф1     [    Х1     \    Ц1     ]    Ч1     ^    Ш1     `    Щ1     a    Ъ1     b    Ы1     c    Ь1     d    Э1     e    Ю1     f    Я1     g    x1     h    а1     i    б1     j    y     k    ¶1     l    в1     m    ы     n    г1     o    д1     p    е1     q    д1     r    е1     s    ж1     t    з1     u    и1     v    Ґ1     w    ж1     x    з1     y    й1     z    к1     {    л1     |    м1     }    н1     ~    о1         п1     Ђ    р1     Ѓ    с1     ‚    т1     ѓ    с1     „    т1     …    у1     †    ф1     ‡    х1     €    ц1     ‰    ч1     Љ    ш1     ‹    щ1     Њ    ъ1     Ќ    ы1     Ћ    ь1     Џ    э1     ђ    ю1     ‘    я1     ’     2     “    2     ”    2     •    2     –    2     —    2         2     ™    2     љ    и2     ›    з2     њ    2     ќ    и2     ћ    з2     џ    —          	2     Ў    x     ў    |1     Ј    
2     ¤    2     Ґ    s     ¦    2     §    2     Ё    2     ©    2     Є    2     «    2     ¬    2     ­    2     ®    2     Ї    2     °    2     ±    2     І    t     і    2     ґ    x1     µ    2     ¶    2     ·    2     ё    2     №    v     є    2     »    2     ј    2     Ѕ     2     ѕ    !2     ї    "2     А    #2     Б    $2     В    %2     Г    2     Д    &2     Е    '2     Ж    (2     З    )2     И    *2     Й    +2     К    ,2     Л    -2     М    Ѓ     Н    q1     О    .2     П    /2     Р    02     С    12     Т    22     У    32     Ф    42     Х    52     Ц    m1     Ч    62     Ш    72     Щ    82     Ъ    92     Ы    2     Ь    :2     Э    ;2     Ю    <2     а    =2     б    ќ1     в    >2     г    ?2     д    @2     е    A2     ж    B2     з    C2     и    D2     й    E2     к    h     л    F2     м    G2     н    H2     о    I2     п    J2     р    K2     с    L2     т    M2     у    N2     ф    O2     х    P2     ц    Q2     ч    R2     ш    2     щ    S2     ъ    T2     ы    U2     ь    V2     э    W2     ю    X2     я    Y2          Z2         [2         \2         ]2         ^2         _2         `2         a2         b2     	    c2     
    d2         e2         f2         g2         o1         h2         i2         j2         k2         l2         m2         n2         o2         p2         q2         r2         s2         t2         u2         v2         w2         x2          y2     !    z2     "    {2     #    |2     $    }2     %    ~2     &    2     '    Ђ2     (    w     3    Ѓ2     4    ‚2     5    ѓ2     6    „2     7    …2     8          9    †2     :    ‡2     ;    €2     <    ‰2     =    Љ2     >    2     ?    :2     @    ‹2     A    Њ2     B    Ќ2     C    Ћ2     D    Џ2     E    ђ2     F    ‘2     G    ’2     H    “2     I    ”2     J    •2     K    –2     N    —2     O    2     P    ™2     Q    љ2     R    ›2     S    њ2     T    ќ2     U    п     V    ћ2     W    џ2     X     2     Y    Ў2     Z    ў2     [    Ј2     \    ¤2     ]    Ґ2     ^    ¦2     _    §2     `    Ё2     a    ©2     b    Є2     c    «2     d    ¬2     e    ­2     f    ®2     g    Ї2     h    °2     i    ±2     j    І2     k    і2     l    ґ2     m    µ2     n    ¶2     o    l     p    u     q    r     r    q     s    ·2     t    ‰     u    *2     v    ё2     w    |     x    ѓ     y    ћ1     z    п     {    №2     |    є2     }    »2     ~    ј2         Ѕ2     Ђ    ѕ2     Ѓ    ї2     ‚    А2     ѓ    Б2     „    В2     …    Г2     †    Д2     ‡    Е2     €    Ж2     ‰    З2     Љ    И2     ‹    Й2     Њ    К2     Ќ    –2     Ћ    Л2     Џ    М2     ђ    Н2     ‘    О2     ’    П2     “    Р2     ”    С2     •    Т2     –    У2     —    Ф2         G      ™    Х2     љ    Ц2     ›    Ч2     њ    Ш2     ќ    Щ2     ћ    Ъ2     џ    Ы2          щ     Ў    X2     ў    Ь2     Ј    Э2     ¤    Ю2     Ґ    Я2     ¦    а2     §    Є2     Ё    «2     ©    ¬2     Є    ­2     «    ®2     ¬    б2  D      й  и  0       hu                                    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   Ы    й2     *    к2     +    л2  D ш     й  и  0       hu                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    з2  D ш     й  и  0       hu                                     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя   )    и2  D $     й  и  0       ru                                      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя           0   @   P   `   p   Ђ   ђ       °   А   Р   а   р           0  @  P  `  p  Ђ  ђ  яяяяяяяя   °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p  Ђ  ђ     °  А  Р  а  р          0  @  P  `  p