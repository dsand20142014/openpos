pin_lock_irqsave(&info->lock,flags);
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
	hdlcdev_init(info);
#endif
}

static void mgslpc_remove_device(MGSLPC_INFO *remove_info)
{
	MGSLPC_INFO *info = mgslpc_device_list;
	MGSLPC_INFO *last = NULL;

	while(info) {
		if (info == remove_info) {
			if (last)
				last->next_device = info->next_device;
			else
				mgslpc_device_list = info->next_device;
#if SYNCLINK_GENERIC_HDLC
			hdlcdev_exit(info);
#endif
			release_resources(info);
			kfree(info);
			mgslpc_device_count--;
			return;
		}
		last = info;
		info = info->next_device;
	}
}

static struct pcmcia_device_id mgslpc_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x02c5, 0x0050),
	PCMCIA_DEVICE_NULL
};
MODULE_DEVICE_TABLE(pcmcia, mgslpc_ids);

static struct pcmcia_driver mgslpc_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "synclink_cs",
	},
	.probe		= mgslpc_probe,
	.remove		= mgslpc_detach,
	.id_table	= mgslpc_ids,
	.suspend	= mgslpc_suspend,
	.resume		= mgslpc_resume,
};

static const struct tty_operations mgslpc_ops = {
	.open = mgslpc_open,
	.close = mgslpc_close,
	.write = mgslpc_write,
	.put_char = mgslpc_put_char,
	.flush_chars = mgslpc_flush_chars,
	.write_room = mgslpc_write_room,
	.chars_in_buffer = mgslpc_chars_in_buffer,
	.flush_buffer = mgslpc_flush_buffer,
	.ioctl = mgslpc_ioctl,
	.throttle = mgslpc_throttle,
	.unthrottle = mgslpc_unthrottle,
	.send_xchar = mgslpc_send_xchar,
	.break_ctl = mgslpc_break,
	.wait_until_sent = mgslpc_wait_until_sent,
	.set_termios = mgslpc_set_termios,
	.stop = tx_pause,
	.start = tx_release,
	.hangup = mgslpc_hangup,
	.tiocmget = tiocmget,
	.tiocmset = tiocmset,
	.proc_fops = &mgslpc_proc_fops,
};

static void synclink_cs_cleanup(void)
{
	int rc;

	printk("Unloading %s: version %s\n", driver_name, driver_version);

	while(mgslpc_device_list)
		mgslpc_remove_device(mgslpc_device_list);

	if (serial_driver) {
		if ((rc = tty_unregister_driver(serial_driver)))
			printk("%s(%d) failed to unregister tty driver err=%d\n",
			       __FILE__,__LINE__,rc);
		put_tty_driver(serial_driver);
	}

	pcmcia_unregister_driver(&mgslpc_driver);
}

static int __init synclink_cs_init(void)
{
    int rc;

    if (break_on_load) {
	    mgslpc_get_text_ptr();
	    BREAKPOINT();
    }

    printk("%s %s\n", driver_name, driver_version);

    if ((rc = pcmcia_register_driver(&mgslpc_driver)) < 0)
	    return rc;

    serial_driver = alloc_tty_driver(MAX_DEVICE_COUNT);
    if (!serial_driver) {
	    rc = -ENOMEM;
	    goto error;
    }

    /* Initialize the tty_driver structure */

    serial_driver->owner = THIS_MODULE;
    serial_driver->driver_name = "synclink_cs";
    serial_driver->name = "ttySLP";
    serial_driver->major = ttymajor;
    serial_driver->minor_start = 64;
    serial_driver->type = TTY_DRIVER_TYPE_SERIAL;
    serial_driver->subtype = SERIAL_TYPE_NORMAL;
    serial_driver->init_termios = tty_std_termios;
    serial_driver->init_termios.c_cflag =
	    B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    serial_driver->flags = TTY_DRIVER_REAL_RAW;
    tty_set_operations(serial_driver, &mgslpc_ops);

    if ((rc = tty_register_driver(serial_driver)) < 0) {
	    printk("%s(%d):Couldn't register serial driver\n",
		   __FILE__,__LINE__);
	    put_tty_driver(serial_driver);
	    serial_driver = NULL;
	    goto error;
    }

    printk("%s %s, tty major#%d\n",
	   driver_name, driver_version,
	   serial_driver->major);

    return 0;

error:
    synclink_cs_cleanup();
    return rc;
}

static void __exit synclink_cs_exit(void)
{
	synclink_cs_cleanup();
}

module_init(synclink_cs_init);
module_exit(synclink_cs_exit);

static void mgslpc_set_rate(MGSLPC_INFO *info, unsigned char channel, unsigned int rate)
{
	unsigned int M, N;
	unsigned char val;

	/* note:standard BRG mode is broken in V3.2 chip
	 * so enhanced mode is always used
	 */

	if (rate) {
		N = 3686400 / rate;
		if (!N)
			N = 1;
		N >>= 1;
		for (M = 1; N > 64 && M < 16; M++)
			N >>= 1;
		N--;

		/* BGR[5..0] = N
		 * BGR[9..6] = M
		 * BGR[7..0] contained in BGR register
		 * BGR[9..8] contained in CCR2[7..6]
		 * divisor = (N+1)*2^M
		 *
		 * Note: M *must* not be zero (causes asymetric duty cycle)
		 */
		write_reg(info, (unsigned char) (channel + BGR),
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
 * Copyright 2004-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file pmic_core_spi.c
 * @brief This is the main file for the PMIC Core/Protocol driver. SPI
 * should be providing the interface between the PMIC and the MCU.
 *
 * @ingroup PMIC_CORE
 */

/*
 * Includes
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/spi/spi.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>

#include <asm/uaccess.h>

#include "pmic.h"

/*
 * Static functions
 */
static void pmic_pdev_register(void);
static void pmic_pdev_unregister(void);

/*
 * Platform device structure for PMIC client drivers
 */
static struct platform_device adc_ldm = {
	.name = "pmic_adc",
	.id = 1,
};
static struct platform_device battery_ldm = {
	.name = "pmic_battery",
	.id = 1,
};
static struct platform_device power_ldm = {
	.name = "pmic_power",
	.id = 1,
};
static struct platform_device rtc_ldm = {
	.name = "pmic_rtc",
	.id = 1,
};
static struct platform_device light_ldm = {
	.name = "pmic_light",
	.id = 1,
};
static struct platform_device rleds_ldm = {
	.name = "pmic_leds",
	.id = 'r',
};
static struct platform_device gleds_ldm = {
	.name = "pmic_leds",
	.id = 'g',
};
static struct platform_device bleds_ldm = {
	.name = "pmic_leds",
	.id = 'b',
};

/*
 * External functions
 */
extern void pmic_event_list_init(void);
extern void pmic_event_callback(type_event event);
extern void gpio_pmic_active(void);
extern irqreturn_t pmic_irq_handler(int irq, void *dev_id);
extern pmic_version_t mxc_pmic_version;
extern struct workqueue_struct *pmic_event_wq;

/*!
 * This function registers platform device structures for
 * PMIC client drivers.
 */
static void pmic_pdev_register(void)
{
	platform_device_register(&adc_ldm);
	platform_device_register(&battery_ldm);
	platform_device_register(&rtc_ldm);
	platform_device_register(&power_ldm);
	platform_device_register(&light_ldm);
	platform_device_register(&rleds_ldm);
	platform_device_register(&gleds_ldm);
	platform_device_register(&bleds_ldm);
}

/*!
 * This function unregisters platform device structures for
 * PMIC client drivers.
 */
static void pmic_pdev_unregister(void)
{
	platform_device_unregister(&adc_ldm);
	platform_device_unregister(&battery_ldm);
	platform_device_unregister(&rtc_ldm);
	platform_device_unregister(&power_ldm);
	platform_device_unregister(&light_ldm);
}

/*!
 * This function puts the SPI slave device in low-power mode/state.
 *
 * @param	spi	the SPI slave device
 * @param	message	the power state to enter
 *
 * @return 	Returns 0 on SUCCESS and error on FAILURE.
 */
static int pmic_suspend(struct spi_device *spi, pm_message_t message)
{
	return PMIC_SUCCESS;
}

/*!
 * This function brings the SPI slave device back from low-power mode/state.
 *
 * @param	spi	the SPI slave device
 *
 * @return 	Returns 0 on SUCCESS and error on FAILURE.
 */
static int pmic_resume(struct spi_device *spi)
{
	return PMIC_SUCCESS;
}

static struct spi_driver pmic_driver;

/*!
 * This function is called whenever the SPI slave device is detected.
 *
 * @param	spi	the SPI slave device
 *
 * @return 	Returns 0 on SUCCESS and error on FAILURE.
 */
static int __devinit pmic_probe(struct spi_device *spi)
{
	int ret = 0;
	struct pmic_platform_data *plat_data = spi->dev.platform_data;

	/* Initialize the PMIC parameters */
	ret = pmic_spi_setup(spi);
	if (ret != PMIC_SUCCESS) {
		return PMIC_ERROR;
	}

	/* Initialize the PMIC event handling */
	pmic_event_list_init();

	/* Initialize GPIO for PMIC Interrupt */
	gpio_pmic_active();

	/* Get the PMIC Version */
	pmic_get_revision(&mxc_pmic_version);
	if (mxc_pmic_version.revision < 0) {
		dev_err((struct device *)spi,
			"PMIC not detected!!! Access Failed\n");
		return -ENODEV;
	} else {
		dev_dbg((struct device *)spi,
			"Detected pmic core IC version number is %d\n",
			mxc_pmic_version.revision);
	}

	spi_set_drvdata(spi, pmic_alloc_data(&(spi->dev)));

	/* Initialize the PMIC parameters */
	ret = pmic_init_registers();
	if (ret != PMIC_SUCCESS) {
		kfree(spi_get_drvdata(spi));
		spi_set_drvdata(spi, NULL);
		return PMIC_ERROR;
	}

	pmic_event_wq = create_workqueue("pmic_spi");
	if (!pmic_event_wq) {
		pr_err("pmic driver init: fail to create work queue");
		kfree(spi_get_drvdata(spi));
		spi_set_drvdata(spi, NULL);
		return -EFAULT;
	}

	/* Set and install PMIC IRQ handler */
	set_irq_type(spi->irq, IRQF_TRIGGER_RISING);
	ret = request_irq(spi->irq, pmic_irq_handler, 0, "PMIC_IRQ", 0);
	if (ret) {
		kfree(spi_get_drvdata(spi));
		spi_set_drvdata(spi, NULL);
		dev_err((struct device *)spi, "gpio1: irq%d error.", spi->irq);
		return ret;
	}

	enable_irq_wake(spi->irq);

	if (plat_data && plat_data->init) {
		ret = plat_data->init(spi_get_drvdata(spi));
		if (ret != 0) {
			kfree(spi_get_drvdata(spi));
			spi_set_drvdata(spi, NULL);
			return PMIC_ERROR;
		}
	}

	power_ldm.dev.platform_data = spi->dev.platform_data;

	pmic_pdev_register();

	printk(KERN_INFO "Device %s probed\n", dev_name(&spi->dev));

	return PMIC_SUCCESS;
}

/*!
 * This function is called whenever the SPI slave device is removed.
 *
 * @param	spi	the SPI slave device
 *
 * @return 	Returns 0 on SUCCESS and error on FAILURE.
 */
static int __devexit pmic_remove(struct spi_device *spi)
{
	if (pmic_event_wq)
		destroy_workqueue(pmic_event_wq);

	free_irq(spi->irq, 0);

	pmic_pdev_unregister();

	printk(KERN_INFO "Device %s removed\n", dev_name(&spi->dev));

	return PMIC_SUCCESS;
}

/*!
 * This structure contains pointers to the power management callback functions.
 */
static struct spi_driver pmic_driver = {
	.driver = {
		   .name = "pmic_spi",
		   .bus = &spi_bus_type,
		   .owner = THIS_MODULE,
		   },
	.probe = pmic_probe,
	.remove = __devexit_p(pmic_remove),
	.suspend = pmic_suspend,
	.resume = pmic_resume,
};

/*
 * Initialization and Exit
 */

/*!
 * This function implements the init function of the PMIC device.
 * This function is called when the module is loaded. It registers
 * the PMIC Protocol driver.
 *
 * @return       This function returns 0.
 */
static int __init pmic_init(void)
{
	return spi_register_driver(&pmic_driver);
}

/*!
 * This function implements the exit function of the PMIC device.
 * This function is called when the module is unloaded. It unregisters
 * the PMIC Protocol driver.
 *
 */
static void __exit pmic_exit(void)
{
	pr_debug("Unregistering the PMIC Protocol Driver\n");
	spi_unregister_driver(&pmic_driver);
}

/*
 * Module entry points
 */
subsys_initcall_sync(pmic_init);
module_exit(pmic_exit);

MODULE_DESCRIPTION("Core/Protocol driver for PMIC");
MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             volume *vol)
{
	s64 nr_free = vol->nr_clusters;
	u32 *kaddr;
	struct address_space *mapping = vol->lcnbmp_ino->i_mapping;
	struct page *page;
	pgoff_t index, max_index;

	ntfs_debug("Entering.");
	/* Serialize accesses to the cluster bitmap. */
	down_read(&vol->lcnbmp_lock);
	/*
	 * Convert the number of bits into bytes rounded up, then convert into
	 * multiples of PAGE_CACHE_SIZE, rounding up so that if we have one
	 * full and one partial page max_index = 2.
	 */
	max_index = (((vol->nr_clusters + 7) >> 3) + PAGE_CACHE_SIZE - 1) >>
			PAGE_CACHE_SHIFT;
	/* Use multiples of 4 bytes, thus max_size is PAGE_CACHE_SIZE / 4. */
	ntfs_debug("Reading $Bitmap, max_index = 0x%lx, max_size = 0x%lx.",
			max_index, PAGE_CACHE_SIZE / 4);
	for (index = 0; index < max_index; index++) {
		unsigned int i;
		/*
		 * Read the page from page cache, getting it from backing store
		 * if necessary, and increment the use count.
		 */
		page = read_mapping_page(mapping, index, NULL);
		/* Ignore pages which errored synchronously. */
		if (IS_ERR(page)) {
			ntfs_debug("read_mapping_page() error. Skipping "
					"page (index 0x%lx).", index);
			nr_free -= PAGE_CACHE_SIZE * 8;
			continue;
		}
		kaddr = (u32*)kmap_atomic(page, KM_USER0);
		/*
		 * For each 4 bytes, subtract the number of set bits. If this
		 * is the last page and it is partial we don't really care as
		 * it just means we do a little extra work but it won't affect
		 * the result as all out of range bytes are set to zero by
		 * ntfs_readpage().
		 */
	  	for (i = 0; i < PAGE_CACHE_SIZE / 4; i++)
			nr_free -= (s64)hweight32(kaddr[i]);
		kunmap_atomic(kaddr, KM_USER0);
		page_cache_release(page);
	}
	ntfs_debug("Finished reading $Bitmap, last index = 0x%lx.", index - 1);
	/*
	 * Fixup for eventual bits outside logical ntfs volume (see function
	 * description above).
	 */
	if (vol->nr_clusters & 63)
		nr_free += 64 - (vol->nr_clusters & 63);
	up_read(&vol->lcnbmp_lock);
	/* If errors occured we may well have gone below zero, fix this. */
	if (nr_free < 0)
		nr_free = 0;
	ntfs_debug("Exiting.");
	return nr_free;
}

/**
 * __get_nr_free_mft_records - return the number of free inodes on a volume
 * @vol:	ntfs volume for which to obtain free inode count
 * @nr_free:	number of mft records in filesystem
 * @max_index:	maximum number of pages containing set bits
 *
 * Calculate the number of free mft records (inodes) on the mounted NTFS
 * volume @vol. We actually calculate the number of mft records in use instead
 * because this allows us to not care about partial pages as these will be just
 * zero filled and hence not be counted as allocated mft record.
 *
 * If any pages cannot be read we assume all mft records in the erroring pages
 * are in use. This means we return an underestimate on errors which is better
 * than an overestimate.
 *
 * NOTE: Caller must hold mftbmp_lock rw_semaphore for reading or writing.
 */
static unsigned long __get_nr_free_mft_records(ntfs_volume *vol,
		s64 nr_free, const pgoff_t max_index)
{
	u32 *kaddr;
	struct address_space *mapping = vol->mftbmp_ino->i_mapping;
	struct page *page;
	pgoff_t index;

	ntfs_debug("Entering.");
	/* Use multiples of 4 bytes, thus max_size is PAGE_CACHE_SIZE / 4. */
	ntfs_debug("Reading $MFT/$BITMAP, max_index = 0x%lx, max_size = "
			"0x%lx.", max_index, PAGE_CACHE_SIZE / 4);
	for (index = 0; index < max_index; index++) {
		unsigned int i;
		/*
		 * Read the page from page cache, getting it from backing store
		 * if necessary, and increment the use count.
		 */
		page = read_mapping_page(mapping, index, NULL);
		/* Ignore pages which errored synchronously. */
		if (IS_ERR(page)) {
			ntfs_debug("read_mapping_page() error. Skipping "
					"page (index 0x%lx).", index);
			nr_free -= PAGE_CACHE_SIZE * 8;
			continue;
		}
		kaddr = (u32*)kmap_atomic(page, KM_USER0);
		/*
		 * For each 4 bytes, subtract the number of set bits. If this
		 * is the last page and it is partial we don't really care as
		 * it just means we do a little extra work but it won't affect
		 * the result as all out of range bytes are set to zero by
		 * ntfs_readpage().
		 */
	  	for (i = 0; i < PAGE_CACHE_SIZE / 4; i++)
			nr_free -= (s64)hweight32(kaddr[i]);
		kunmap_atomic(kaddr, KM_USER0);
		page_cache_release(page);
	}
	ntfs_debug("Finished reading $MFT/$BITMAP, last index = 0x%lx.",
			index - 1);
	/* If errors occured we may well have gone below zero, fix this. */
	if (nr_free < 0)
		nr_free = 0;
	ntfs_debug("Exiting.");
	return nr_free;
}

/**
 * ntfs_statfs - return information about mounted NTFS volume
 * @dentry:	dentry from mounted volume
 * @sfs:	statfs structure in which to return the information
 *
 * Return information about the mounted NTFS volume @dentry in the statfs structure
 * pointed to by @sfs (this is initialized with zeros before ntfs_statfs is
 * called). We interpret the values to be correct of the moment in time at
 * which we are called. Most values are variable otherwise and this isn't just
 * the free values but the totals as well. For example we can increase the
 * total number of file nodes if we run out and we can keep doing this until
 * there is no more space on the volume left at all.
 *
 * Called from vfs_statfs which is used to handle the statfs, fstatfs, and
 * ustat system calls.
 *
 * Return 0 on success or -errno on error.
 */
static int ntfs_statfs(struct dentry *dentry, struct kstatfs *sfs)
{
	struct super_block *sb = dentry->d_sb;
	s64 size;
	ntfs_volume *vol = NTFS_SB(sb);
	ntfs_inode *mft_ni = NTFS_I(vol->mft_ino);
	pgoff_t max_index;
	unsigned long flags;

	ntfs_debug("Entering.");
	/* Type of filesystem. */
	sfs->f_type   = NTFS_SB_MAGIC;
	/* Optimal transfer block size. */
	sfs->f_bsize  = PAGE_CACHE_SIZE;
	/*
	 * Total data blocks in filesystem in units of f_bsize and since
	 * inodes are also stored in data blocs ($MFT is a file) this is just
	 * the total clusters.
	 */
	sfs->f_blocks = vol->nr_clusters << vol->cluster_size_bits >>
				PAGE_CACHE_SHIFT;
	/* Free data blocks in filesystem in units of f_bsize. */
	size	      = get_nr_free_clusters(vol) << vol->cluster_size_bits >>
				PAGE_CACHE_SHIFT;
	if (size < 0LL)
		size = 0LL;
	/* Free blocks avail to non-superuser, same as above on NTFS. */
	sfs->f_bavail = sfs->f_bfree = size;
	/* Serialize accesses to the inode bitmap. */
	down_read(&vol->mftbmp_lock);
	read_lock_irqsave(&mft_ni->size_lock, flags);
	size = i_size_read(vol->mft_ino) >> vol->mft_record_size_bits;
	/*
	 * Convert the maximum number of set bits into bytes rounded up, then
	 * convert into multiples of PAGE_CACHE_SIZE, rounding up so that if we
	 * have one full and one partial page max_index = 2.
	 */
	max_index = ((((mft_ni->initialized_size >> vol->mft_record_size_bits)
			+ 7) >> 3) + PAGE_CACHE_SIZE - 1) >> PAGE_CACHE_SHIFT;
	read_unlock_irqrestore(&mft_ni->size_lock, flags);
	/* Number of inodes in filesystem (at this point in time). */
	sfs->f_files = size;
	/* Free inodes in fs (based on current total count). */
	sfs->f_ffree = __get_nr_free_mft_records(vol, size, max_index);
	up_read(&vol->mftbmp_lock);
	/*
	 * File system id. This is extremely *nix flavour dependent and even
	 * within Linux itself all fs do their own thing. I interpret this to
	 * mean a unique id associated with the mounted fs and not the id
	 * associated with the filesystem driver, the latter is already given
	 * by the filesystem type in sfs->f_type. Thus we use the 64-bit
	 * volume serial number splitting it into two 32-bit parts. We enter
	 * the least significant 32-bits in f_fsid[0] and the most significant
	 * 32-bits in f_fsid[1].
	 */
	sfs->f_fsid.val[0] = vol->serial_no & 0xffffffff;
	sfs->f_fsid.val[1] = (vol->serial_no >> 32) & 0xffffffff;
	/* Maximum length of filenames. */
	sfs->f_namelen	   = NTFS_MAX_NAME_LEN;
	return 0;
}

/**
 * The complete super operations.
 */
static const struct super_operations ntfs_sops = {
	.alloc_inode	= ntfs_alloc_big_inode,	  /* VFS: Allocate new inode. */
	.destroy_inode	= ntfs_destroy_big_inode, /* VFS: Deallocate inode. */
#ifdef NTFS_RW
	//.dirty_inode	= NULL,			/* VFS: Called from
	//					   __mark_inode_dirty(). */
	.write_inode	= ntfs_write_inode,	/* VFS: Write                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ;
inode_err_out:
	kmem_cache_destroy(ntfs_name_cache);
name_err_out:
	kmem_cache_destroy(ntfs_attr_ctx_cache);
actx_err_out:
	kmem_cache_destroy(ntfs_index_ctx_cache);
ictx_err_out:
	if (!err) {
		printk(KERN_CRIT "NTFS: Aborting NTFS filesystem driver "
				"registration...\n");
		err = -ENOMEM;
	}
	return err;
}

static void __exit exit_ntfs_fs(void)
{
	ntfs_debug("Unregistering NTFS driver.");

	unregister_filesystem(&ntfs_fs_type);
	kmem_cache_destroy(ntfs_big_inode_cache);
	kmem_cache_destroy(ntfs_inode_cache);
	kmem_cache_destroy(ntfs_name_cache);
	kmem_cache_destroy(ntfs_attr_ctx_cache);
	kmem_cache_destroy(ntfs_index_ctx_cache);
	/* Unregister the ntfs sysctls. */
	ntfs_sysctl(0);
}

MODULE_AUTHOR("Anton Altaparmakov <aia21@cantab.net>");
MODULE_DESCRIPTION("NTFS 1.2/3.x driver - Copyright (c) 2001-2007 Anton Altaparmakov");
MODULE_VERSION(NTFS_VERSION);
MODULE_LICENSE("GPL");
#ifdef DEBUG
module_param(debug_msgs, bool, 0);
MODULE_PARM_DESC(debug_msgs, "Enable debug messages.");
#endif

module_init(init_ntfs_fs)
module_exit(exit_ntfs_fs)
