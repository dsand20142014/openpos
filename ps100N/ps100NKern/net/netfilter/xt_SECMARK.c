 void ntty_close(struct tty_struct *tty, struct file *file)
{
	struct nozomi *dc = get_dc_by_tty(tty);
	struct port *nport = tty->driver_data;
	struct tty_port *port = &nport->port;
	unsigned long flags;

	if (!dc || !nport)
		return;

	/* Users cannot interrupt a close */
	mutex_lock(&nport->tty_sem);

	WARN_ON(!port->count);

	dc->open_ttys--;
	port->count--;
	tty_port_tty_set(port, NULL);

	if (port->count == 0) {
		DBG1("close: %d", nport->token_dl);
		spin_lock_irqsave(&dc->spin_mutex, flags);
		dc->last_ier &= ~(nport->token_dl);
		writew(dc->last_ier, dc->reg_ier);
		spin_unlock_irqrestore(&dc->spin_mutex, flags);
	}
	mutex_unlock(&nport->tty_sem);
}

/*
 * called when the userspace process writes to the tty (/dev/noz*).
 * Data is inserted into a fifo, which is then read and transfered to the modem.
 */
static int ntty_write(struct tty_struct *tty, const unsigned char *buffer,
		      int count)
{
	int rval = -EINVAL;
	struct nozomi *dc = get_dc_by_tty(tty);
	struct port *port = tty->driver_data;
	unsigned long flags;

	/* DBG1( "WRITEx: %d, index = %d", count, index); */

	if (!dc || !port)
		return -ENODEV;

	if (unlikely(!mutex_trylock(&port->tty_sem))) {
		/*
		 * must test lock as tty layer wraps calls
		 * to this function with BKL
		 */
		dev_err(&dc->pdev->dev, "Would have deadlocked - "
			"return EAGAIN\n");
		return -EAGAIN;
	}

	if (unlikely(!port->port.count)) {
		DBG1(" ");
		goto exit;
	}

	rval = __kfifo_put(port->fifo_ul, (unsigned char *)buffer, count);

	/* notify card */
	if (unlikely(dc == NULL)) {
		DBG1("No device context?");
		goto exit;
	}

	spin_lock_irqsave(&dc->spin_mutex, flags);
	/* CTS is only valid on the modem channel */
	if (port == &(dc->port[PORT_MDM])) {
		if (port->ctrl_dl.CTS) {
			DBG4("Enable interrupt");
			enable_transmit_ul(tty->index % MAX_PORT, dc);
		} else {
			dev_err(&dc->pdev->dev,
				"CTS not active on modem port?\n");
		}
	} else {
		enable_transmit_ul(tty->index % MAX_PORT, dc);
	}
	spin_unlock_irqrestore(&dc->spin_mutex, flags);

exit:
	mutex_unlock(&port->tty_sem);
	return rval;
}

/*
 * Calculate how much is left in device
 * This method is called by the upper tty layer.
 *   #according to sources N_TTY.c it expects a value >= 0 and
 *    does not check for negative values.
 */
static int ntty_write_room(struct tty_struct *tty)
{
	struct port *port = tty->driver_data;
	int room = 0;
	const struct nozomi *dc = get_dc_by_tty(tty);

	if (!dc || !port)
		return 0;
	if (!mutex_trylock(&port->tty_sem))
		return 0;

	if (!port->port.count)
		goto exit;

	room = port->fifo_ul->size - __kfifo_len(port->fifo_ul);

exit:
	mutex_unlock(&port->tty_sem);
	return room;
}

/* Gets io control parameters */
static int ntty_tiocmget(struct tty_struct *tty, struct file *file)
{
	const struct port *port = tty->driver_data;
	const struct ctrl_dl *ctrl_dl = &port->ctrl_dl;
	const struct ctrl_ul *ctrl_ul = &port->ctrl_ul;

	/* Note: these could change under us but it is not clear this
	   matters if so */
	return	(ctrl_ul->RTS ? TIOCM_RTS : 0) |
		(ctrl_ul->DTR ? TIOCM_DTR : 0) |
		(ctrl_dl->DCD ? TIOCM_CAR : 0) |
		(ctrl_dl->RI  ? TIOCM_RNG : 0) |
		(ctrl_dl->DSR ? TIOCM_DSR : 0) |
		(ctrl_dl->CTS ? TIOCM_CTS : 0);
}

/* Sets io controls parameters */
static int ntty_tiocmset(struct tty_struct *tty, struct file *file,
	unsigned int set, unsigned int clear)
{
	struct nozomi *dc = ge