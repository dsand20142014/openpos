->first_line;
	tmp.irq = cinfo->irq;
	tmp.flags = info->port.flags;
	tmp.close_delay = info->port.close_delay;
	tmp.closing_wait = info->port.closing_wait;
	tmp.baud_base = info->baud;
	tmp.custom_divisor = info->custom_divisor;
	tmp.hub6 = 0;		/*!!! */
	return copy_to_user(retinfo, &tmp, sizeof(*retinfo)) ? -EFAULT : 0;
}				/* get_serial_info */

static int
set_serial_info(struct cyclades_port *info,
		struct serial_struct __user *new_info)
{
	struct serial_struct new_serial;
	struct cyclades_port old_info;

	if (copy_from_user(&new_serial, new_info, sizeof(new_serial)))
		return -EFAULT;
	old_info = *info;

	if (!capable(CAP_SYS_ADMIN)) {
		if (new_serial.close_delay != info->port.close_delay ||
				new_serial.baud_base != info->baud ||
				(new_serial.flags & ASYNC_FLAGS &
					~ASYNC_USR_MASK) !=
				(info->port.flags & ASYNC_FLAGS & ~ASYNC_USR_MASK))
			return -EPERM;
		info->port.flags = (info->port.flags & ~ASYNC_USR_MASK) |
				(new_serial.flags & ASYNC_USR_MASK);
		info->baud = new_serial.baud_base;
		info->custom_divisor = new_serial.custom_divisor;
		goto check_and_exit;
	}

	/*
	 * OK, past this point, all the error checking has been done.
	 * At this point, we start making changes.....
	 */

	info->baud = new_serial.baud_base;
	info->custom_divisor = new_serial.custom_divisor;
	info->port.flags = (info->port.flags & ~ASYNC_FLAGS) |
			(new_serial.flags & ASYNC_FLAGS);
	info->port.close_delay = new_serial.close_delay * HZ / 100;
	info->port.closing_wait = new_serial.closing_wait * HZ / 100;

check_and_exit:
	if (info->port.flags & ASYNC_INITIALIZED) {
		set_line_char(info);
		return 0;
	} else {
		return startup(info);
	}
}				/* set_serial_info */

/*
 * get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 *	    is emptied.  On bus types like RS485, the transmitter must
 *	    release the bus after transmitting. This must be done when
 *	    the transmit shift register is empty, not be done when the
 *	    transmit holding register is empty.  This functionality
 *	    allows an RS485 driver to be written in user space.
 */
static int get_lsr_info(struct cyclades_port *info, unsigned int __user *value)
{
	struct cyclades_card *card;
	int chip, channel, index;
	unsigned char status;
	unsigned int result;
	unsigned long flags;
	void __iomem *base_addr;

	card = info->card;
	channel = (info->line) - (card->first_line);
	if (!cy_is_Z(card)) {
		chip = channel >> 2;
		channel &= 0x03;
		index = card->bus_index;
		base_addr = card->base_addr + (cy_chip_offset[chip] << index);

		spin_lock_irqsave(&card->card_lock, flags);
		status = readb(base_addr + (CySRER << index)) &
				(CyTxRdy | CyTxMpty);
		spin_unlock_irqrestore(&card->card_lock, flags);
		result = (status ? 0 : TIOCSER_TEMT);
	} else {
		/* Not supported yet */
		return -EINVAL;
	}
	return put_user(result, (unsigned long __user *)value);
}

static int cy_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct cyclades_port *info = tty->driver_data;
	struct cyclades_card *card;
	int chip, channel, index;
	void __iomem *base_addr;
	unsigned long flags;
	unsigned char status;
	unsigned long lstatus;
	unsigned int result;
	struct FIRM_ID __iomem *firm_id;
	struct ZFW_CTRL __iomem *zfw_ctrl;
	struct BOARD_CTRL __iomem *board_ctrl;
	struct CH_CTRL __iomem *ch_ctrl;

	if (serial_paranoia_check(info, tty->name, __func__))
		return -ENODEV;

	lock_kernel();

	card = info->card;
	channel = info->line - card->first_line;
	if (!cy_is_Z(card)) {
		chip = channel >> 2;
		channel &= 0x03;
		index = card->bus_index;
		base_addr = card->base_addr + (cy_chip_offset[chip] << index);

		spin_lock_irqsave(&card->card_lock, flags);
		cy_writeb(base_addr + (CyCAR << index), (u_char) channel);
		status = readb(base_addr + (CyMSVR1 << index));
		status |= readb(base_addr + (CyMSVR2 << index));
		spin_unlock_irqrestore(&card->card_lock, flags);

		if (info->rtsdtr_inv) {
			result = ((status & CyRTS) ? TIOCM_DTR : 0) |
				((status & CyDTR) ? TIOCM_RTS : 0);
		} else {
			result = ((status & CyRTS) ? TIOCM_RTS : 0) |
				((status 