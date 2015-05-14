mdStart + 2);
	}
	head = (head + 4) & (cmdMax - cmdStart - 4);
	writew(head, &ch->mailbox->cin);
	count = FEPTIMEOUT;

	for (;;) {
		count--;
		if (count == 0)  {
			printk(KERN_ERR "<Error> - Fep not responding in fepcmd()\n");
			return;
		}
		head = readw(&ch->mailbox->cin);
		cmdTail = readw(&ch->mailbox->cout);
		n = (head - cmdTail) & (cmdMax - cmdStart - 4);
		/*
		 * Basically this will break when the FEP acknowledges the
		 * command by incrementing cmdTail (Making it equal to head).
		 */
		if (n <= ncmds * (sizeof(short) * 4))
			break;
	}
}

/*
 * Digi products use fields in their channels structures that are very similar
 * to the c_cflag and c_iflag fields typically found in UNIX termios
 * structures. The below three routines allow mappings between these hardware
 * "flags" and their respective Linux flags.
 */
static unsigned termios2digi_h(struct channel *ch, unsigned cflag)
{
	unsigned res = 0;

	if (cflag & CRTSCTS) {
		ch->digiext.digi_flags |= (RTSPACE | CTSPACE);
		res |= ((ch->m_cts) | (ch->m_rts));
	}

	if (ch->digiext.digi_flags & RTSPACE)
		res |= ch->m_rts;

	if (ch->digiext.digi_flags & DTRPACE)
		res |= ch->m_dtr;

	if (ch->digiext.digi_flags & CTSPACE)
		res |= ch->m_cts;

	if (ch->digiext.digi_flags & DSRPACE)
		res |= ch->dsr;

	if (ch->digiext.digi_flags & DCDPACE)
		res |= ch->dcd;

	if (res & (ch->m_rts))
		ch->digiext.digi_flags |= RTSPACE;

	if (res & (ch->m_cts))
		ch->digiext.digi_flags |= CTSPACE;

	return res;
}

static unsigned termios2digi_i(struct channel *ch, unsigned iflag)
{
	unsigned res = iflag & (IGNBRK | BRKINT | IGNPAR | PARMRK |
					INPCK | ISTRIP | IXON | IXANY | IXOFF);
	if (ch->digiext.digi_flags & DIGI_AIXON)
		res |= IAIXON;
	return res;
}

static unsigned termios2digi_c(struct channel *ch, unsigned cflag)
{
	unsigned res = 0;
	if (cflag & CBAUDEX) {
		ch->digiext.digi_flags |= DIGI_FAST;
		/*
		 * HUPCL bit is used by FEP to indicate fast baud table is to
		 * be used.
		 */
		res |= FEP_HUPCL;
	} else
		ch->digiext.digi_flags &= ~DIGI_FAST;
	/*
	 * CBAUD has bit position 0x1000 set these days to indicate Linux
	 * baud rate remap. Digi hardware can't handle the bit assignment.
	 * (We use a different bit assignment for high speed.). Clear this
	 * bit out.
	 */
	res |= cflag & ((CBAUD ^ CBAUDEX) | PARODD | PARENB | CSTOPB | CSIZE);
	/*
	 * This gets a little confusing. The Digi cards have their own
	 * representation of c_cflags controlling baud rate. For the most part
	 * this is identical to the Linux implementation. However; Digi
	 * supports one rate (76800) that Linux doesn't. This means that the
	 * c_cflag entry that would normally mean 76800 for Digi actually means
	 * 115200 under Linux. Without the below mapping, a stty 115200 would
	 * only drive the board at 76800. Since the rate 230400 is also found
	 * after 76800, the same problem afflicts us when we choose a rate of
	 * 230400. Without the below modificiation stty 230400 would actually
	 * give us 115200.
	 *
	 * There are two additional differences. The Linux value for CLOCAL
	 * (0x800; 0004000) has no meaning to the Digi hardware. Also in later
	 * releases of Linux; the CBAUD define has CBAUDEX (0x1000; 0010000)
	 * ored into it (CBAUD = 0x100f as opposed to 0xf). CBAUDEX should be
	 * checked for a screened out prior to termios2digi_c returning. Since
	 * CLOCAL isn't used by the board this can be ignored as long as the
	 * returned value is used only by Digi hardware.
	 */
	if (cflag & CBAUDEX) {
		/*
		 * The below code is trying to guarantee that only baud rates
		 * 115200 and 230400 are remapped. We use exclusive or because
		 * the various baud rates share common bit positions and
		 * therefore can't be tested for easily.
		 */
		if ((!((cflag & 0x7) ^ (B115200 & ~CBAUDEX))) ||
		    (!((cflag & 0x7) ^ (B230400 & ~CBAUDEX))))
			res += 1;
	}
	return res;
}

/* Caller must hold the locks */
static void epcaparam(struct tty_struct *tty, struct channel *ch)
{
	unsigned int cmdHead;
	struct ktermios *ts;
	struct board_chan __iomem *bc;
	unsigned mval, hflow, cflag, iflag;

	bc = ch->brdchan;
	epcaassert(bc != NULL, "bc out of range");

	assertgwinon(ch);
	ts = tty->termios;
	if ((ts->c_cflag & CBAUD) == 0)  { /* Begin CBAUD detected */
		cmdHead = readw(&bc->rin);
		writew(cmdHead, &bc->rout);
		cmdHead = readw(&bc->tin);
		/* Changing baud in mid-stream transmission can be wonderful */
		/*
		 * Flush current transmit buffer by setting cmdTail pointer
		 * (tout) to cmdHead pointer (tin). Hopefully the transmit
		 * buffer is empty.
		 */
		fepcmd(ch, STOUT, (unsigned) cmdHead, 0, 0, 0);
		mval = 0;
	} else { /* Begin CBAUD not detected */
		/*
		 * c_cflags have changed but that change had nothing to do with
		 * BAUD. Propagate the change to the card.
		 */
		cflag = termios2digi_c(ch, ts->c_cflag);
		if (cflag != ch->fepcflag)  {
			ch->fepcflag = cflag;
			/* Set baud rate, char size, stop bits, parity */
			fepcmd(ch, SETCTRLFLAGS, (unsigned) cflag, 0, 0, 0);
		}
		/*
		 * If the user has not forced CLOCAL and if the device is not a
		 * CALLOUT device (Which is always CLOCAL) we set flags such
		 * that the driver will wait on carrier detect.
		 */
		if (ts->c_cflag & CLOCAL)
			clear_bit(ASYNCB_CHECK_CD, &ch->port.flags);
		else
			set_bit(ASYNCB_CHECK_CD, &ch->port.flags);
		mval = ch->m_dtr | ch->m_rts;
	} /* End CBAUD not detected */
	iflag = termios2digi_i(ch, ts->c_iflag);
	/* Check input mode flags */
	if (iflag != ch->fepiflag)  {
		ch->fepiflag = iflag;
		/*
		 * Command sets channels iflag structure on the board. Such
		 * things as input soft flow control, handling of parity
		 * errors, and break handling are all set here.
		 *
		 * break handling, parity handling, input stripping,
		 * flow control chars
		 */
		fepcmd(ch, SETIFLAGS, (unsigned int) ch->fepiflag, 0, 0, 0);
	}
	/*
	 * Set the board mint value for this channel. This will cause hardware
	 * events to be generated each time the DCD signal (Described in mint)
	 * changes.
	 */
	writeb(ch->dcd, &bc->mint);
	if ((ts->c_cflag & CLOCAL) || (ch->digiext.digi_flags & DIGI_FORCEDCD))
		if (ch->digiext.digi_flags & DIGI_FORCEDCD)
			writeb(0, &bc->mint);
	ch->imodem = readb(&bc->mstat);
	hflow = termios2digi_h(ch, ts->c_cflag);
	if (hflow != ch->hflow)  {
		ch->hflow = hflow;
		/*
		 * Hard flow control has been selected but the board is not
		 * using it. Activate hard flow control now.
		 */
		fepcmd(ch, SETHFLOW, hflow, 0xff, 0, 1);
	}
	mval ^= ch->modemfake & (mval ^ ch->modem);

	if (ch->omodem ^ mval)  {
		ch->omodem = mval;
		/*
		 * The below command sets the DTR and RTS mstat structure. If
		 * hard flow control is NOT active these changes will drive the
		 * output of the actual DTR and RTS lines. If hard flow control
		 * is active, the changes will be saved in the mstat structure
		 * and only asserted when hard flow control is turned off.
		 */

		/* First reset DTR & RTS; then set them */
		fepcmd(ch, SETMODEM, 0, ((ch->m_dtr)|(ch->m_rts)), 0, 1);
		fepcmd(ch, SETMODEM, mval, 0, 0, 1);
	}
	if (ch->startc != ch->fepstartc || ch->stopc != ch->fepstopc)  {
		ch->fepstartc = ch->startc;
		ch->fepstopc = ch->stopc;
		/*
		 * The XON / XOFF characters have changed; propagate these
		 * changes to the card.
		 */
		fepcmd(ch, SONOFFC, ch->fepstartc, ch->fepstopc, 0, 1);
	}
	if (ch->startca != ch->fepstartca || ch->stopca != ch->fepstopca)  {
		ch->fepstartca = ch->startca;
		ch->fepstopca = ch->stopca;
		/*
		 * Similar to the above, this time the auxilarly XON / XOFF
		 * characters have changed; propagate these changes to the card.
		 */
		fepcmd(ch, SAUXONOFFC, ch->fepstartca, ch->fepstopca, 0, 1);
	}
}

/* Caller holds lock */
static void receive_data(struct channel *ch, struct tty_struct *tty)
{
	unchar *rptr;
	struct ktermios *ts = NULL;
	struct board_chan __iomem *bc;
	int dataToRead, wrapgap, bytesAvailable;
	unsigned int tail, head;
	unsigned int wrapmask;

	/*
	 * This routine is called by doint when a receive data event has taken
	 * place.
	 */
	globalwinon(ch);
	if (ch->statusflags & RXSTOPPED)
		return;
	if (tty)
		ts = tty->termios;
	bc = ch->brdchan;
	BUG_ON(!bc);
	wrapmask = ch->rxbufsize - 1;

	/*
	 * Get the head and tail pointers to the receiver queue. Wrap the head
	 * pointer if it has rea