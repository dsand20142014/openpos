
		info->stop_rx = 0;
		info->ldisc_stop_rx = 0;

		/* Enhance mode enabled here */
		if (brd->chip_flag != MOXA_OTHER_UART)
			mxser_enable_must_enchance_mode(info->ioaddr);

		info->port.flags = ASYNC_SHARE_IRQ;
		info->type = brd->uart_type;

		process_txrx_fifo(info);

		info->custom_divisor = info->baud_base * 16;
		info->port.close_delay = 5 * HZ / 10;
		info->port.closing_wait = 30 * HZ;
		info->normal_termios = mxvar_sdriver->init_termios;
		init_waitqueue_head(&info->delta_msr_wait);
		memset(&info->mon_data, 0, sizeof(struct mxser_mon));
		info->err_shadow = 0;
		spin_lock_init(&info->slock);

		/* before set INT ISR, disable all int */
		outb(inb(info->ioaddr + UART_IER) & 0xf0,
			info->ioaddr + UART_IER);
	}

	retval = request_irq(brd->irq, mxser_interrupt, IRQF_SHARED, "mxser",
			brd);
	if (retval) {
		printk(KERN_ERR "Board %s: Request irq failed, IRQ (%d) may "
			"conflict with another device.\n",
			brd->info->name, brd->irq);
		/* We hold resources, we need to release them. */
		mxser_release_res(brd, pdev, 0);
	}
	return retval;
}

static int __init mxser_get_ISA_conf(int cap, struct mxser_board *brd)
{
	int id, i, bits;
	unsigned short regs[16], irq;
	unsigned char scratch, scratch2;

	brd->chip_flag = MOXA_OTHER_UART;

	id = mxser_read_register(cap, regs);
	switch (id) {
	case C168_ASIC_ID:
		brd->info = &mxser_cards[0];
		break;
	case C104_ASIC_ID:
		brd->info = &mxser_cards[1];
		break;
	case CI104J_ASIC_ID:
		brd->info = &mxser_cards[2];
		break;
	case C102_ASIC_ID:
		brd->info = &mxser_cards[5];
		break;
	case CI132_ASIC_ID:
		brd->info = &mxser_cards[6];
		break;
	case CI134_ASIC_ID:
		brd->info = &mxser_cards[7];
		break;
	default:
		return 0;
	}

	irq = 0;
	/* some ISA cards have 2 ports, but we want to see them as 4-port (why?)
	   Flag-hack checks if configuration should be read as 2-port here. */
	if (brd->info->nports == 2 || (brd->info->flags & MXSER_HAS2)) {
		irq = regs[9] & 0xF000;
		irq = irq | (irq >> 4);
		if (irq != (regs[9] & 0xFF00))
			goto err_irqconflict;
	} else if (brd->info->nports == 4) {
		irq = regs[9] & 0xF000;
		irq = irq | (irq >> 4);
		irq = irq | (irq >> 8);
		if (irq != regs[9])
			goto err_irqconflict;
	} else if (brd->info->nports == 8) {
		irq = regs[9] & 0xF000;
		irq = irq | (irq >> 4);
		irq = irq | (irq >> 8);
		if ((irq != regs[9]) || (irq != regs[10]))
			goto err_irqconflict;
	}

	if (!irq) {
		printk(KERN_ERR "mxser: interrupt number unset\n");
		return -EIO;
	}
	brd->irq = ((int)(irq & 0xF000) >> 12);
	for (i = 0; i < 8; i++)
		brd->ports[i].ioaddr = (int) regs[i + 1] & 0xFFF8;
	if ((regs[12] & 0x80) == 0) {
		printk(KERN_ERR "mxser: invalid interrupt vector\n");
		return -EIO;
	}
	brd->vector = (int)regs[11];	/* interrupt vector */
	if (id == 1)
		brd->vector_mask = 0x00FF;
	else
		brd->vector_mask = 0x000F;
	for (i = 7, bits = 0x0100; i >= 0; i--, bits <<= 1) {
		if (regs[12] & bits) {
			brd->ports[i].baud_base = 921600;
			brd->ports[i].max_baud = 921600;
		} else {
			brd->ports[i].baud_base = 115200;
			brd->ports[i].max_baud = 115200;
		}
	}
	scratch2 = inb(cap + UART_LCR) & (~UART_LCR_DLAB);
	outb(scratch2 | UART_LCR_DLAB, cap + UART_LCR);
	outb(0, cap + UART_EFR);	/* EFR is the same as FCR */
	outb(scratch2, cap + UART_LCR);
	outb(UART_FCR_ENABLE_FIFO, cap + UART_FCR);
	scratch = inb(cap + UART_IIR);

	if (scratch & 0xC0)
		brd->uart_type = PORT_16550A;
	else
		brd->uart_type = PORT_16450;
	if (!request_region(brd->ports[0].ioaddr, 8 * brd->info->nports,
			"mxser(IO)")) {
		printk(KERN_E