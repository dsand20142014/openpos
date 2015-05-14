gnal states */
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
		printk("%s(%d):mgslpc_chars_in_buffer(%s)\n"