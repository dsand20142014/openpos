/*
 * Copyright (c) 2007-2008 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*                                                                      */
/*  Module Name : mm.c                                                  */
/*                                                                      */
/*  Abstract                                                            */
/*      This module contains common functions for handle management     */
/*      frame.                                                          */
/*                                                                      */
/*  NOTES                                                               */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#include "cprecomp.h"
#include "../hal/hpreg.h"

/* TODO : put all constant tables to a file */
const u8_t zg11bRateTbl[4] = {2, 4, 11, 22};
const u8_t zg11gRateTbl[8] = {12, 18, 24, 36, 48, 72, 96, 108};

/* 0xff => element does not exist */
const u8_t zgElementOffsetTable[] =
{
    4,      /*  0 : asoc req */
    6,      /*  1 : asoc rsp */
    10,     /*  2 : reasoc req*/
    6,      /*  3 : reasoc rsp */
    0,      /*  4 : probe req */
    12,     /*  5 : probe rsp */
    0xff,   /*  6 : reserved */
    0xff,   /*  7 : reserved */
    12,     /*  8 : beacon */
    4,      /*  9 : ATIM */
    0xff,   /* 10 : disasoc */
    6,      /* 11 : auth */
    0xff,   /* 12 : deauth */
    4,      /* 13 : action */
    0xff,   /* 14 : reserved */
    0xff,   /* 15 : reserved */
};

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfFindElement               */
/*      Find a specific element in management frame                     */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : management frame buffer                                   */
/*      eid : target element id                                         */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      byte offset of target element                                   */
/*      or 0xffff if not found                                          */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2005.10     */
/*                                                                      */
/************************************************************************/
u16_t zfFindElement(zdev_t* dev, zbuf_t* buf, u8_t eid)
{
    u8_t subType;
    u16_t offset;
    u16_t bufLen;
    u16_t elen;
    u8_t id, HTEid=0;
    u8_t oui[4] = {0x00, 0x50, 0xf2, 0x01};
    u8_t oui11n[3] = {0x00,0x90,0x4C};
    u8_t HTType = 0;

    /* Get offset of first element */
    subType = (zmw_rx_buf_readb(dev, buf, 0) >> 4);
    if ((offset = zgElementOffsetTable[subType]) == 0xff)
    {
        zm_assert(0);
    }

    /* Plus wlan header */
    offset += 24;

    // jhlee HT 0

    if ((eid == ZM_WLAN_EID_HT_CAPABILITY) ||
        (eid == ZM_WLAN_EID_EXTENDED_HT_CAPABILITY))
    {
        HTEid = eid;
        eid = ZM_WLAN_EID_WPA_IE;
        HTType = 1;
    }


    bufLen = zfwBufGetSize(dev, buf);
    /* Search loop */
    while ((offset+2)<bufLen)                   // including element ID and length (2bytes)
    {
        /* Search target element */
        if ((id = zmw_rx_buf_readb(dev, buf, offset)) == eid)
        {
            /* Bingo */
            if ((elen = zmw_rx_buf_readb(dev, buf, offset+1))>(bufLen - offset))
            {
                /* Element length error */
                return 0xffff;
            }

            if ( elen == 0 && eid != ZM_WLAN_EID_SSID)
            {
                /* Element length error */
                return 0xffff;
            }

            if ( eid == ZM_WLAN_EID_WPA_IE )
            {
                /* avoid sta to be thought use 11n when find a WPA_IE */
                if ( (HTType == 0) && zfRxBufferEqualToStr(dev, buf, oui, offset+2, 4) )
                {
                    return offset;
                }

                // jhlee HT 0
                // CWYang(+)

                if ((HTType == 1) && ( zfRxBufferEqualToStr(dev, buf, oui11n, offset+2, 3) ))
                {
                    if ( zmw_rx_buf_readb(dev, buf, offset+5) == HTEid )
                    {
                        return offset + 5;
                    }
                }

            }
            else
            {
                return offset;
            }
        }
        /* Advance to next element */
        #if 1
        elen = zmw_rx_buf_readb(dev, buf, offset+1);
        #else
        if ((elen = zmw_rx_buf_readb(dev, buf, offset+1)) == 0)
        {
            return 0xffff;
        }
        #endif

        offset += (elen+2);
    }
    return 0xffff;
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfFindWifiElement           */
/*      Find a specific Wifi element in management frame                */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : management frame buffer                                   */
/*      type : OUI type                                                 */
/*      subType : OUI subtype                                           */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      byte offset of target element                                   */
/*      or 0xffff if not found                                          */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2006.1      */
/*                                                                      */
/************************************************************************/
u16_t zfFindWifiElement(zdev_t* dev, zbuf_t* buf, u8_t type, u8_t subtype)
{
    u8_t subType;
    u16_t offset;
    u16_t bufLen;
    u16_t elen;
    u8_t id;
    u8_t tmp;

    /* Get offset of first element */
    subType = (zmw_rx_buf_readb(dev, buf, 0) >> 4);

    if ((offset = zgElementOffsetTable[subType]) == 0xff)
    {
        zm_assert(0);
    }

    /* Plus wlan header */
    offset += 24;

    bufLen = zfwBufGetSize(dev, buf);
    /* Search loop */
    while ((offset+2)<bufLen)                   // including element ID and length (2bytes)
    {
        /* Search target element */
        if ((id = zmw_rx_buf_readb(dev, bu the bit order between the docs and
	   the IO8+ card. The new PCI card now conforms to old docs.
	   They changed the PCI docs to reflect the situation on the
	   old card. */
	val2 = (bp->flags & SX_BOARD_IS_PCI)?0x4d : 0xb2;
	if (val1 != val2) {
		printk(KERN_INFO
		  "sx%d: specialix IO8+ ID %02x at 0x%03x not found (%02x).\n",
		       board_No(bp), val2, bp->base, val1);
		sx_release_io_range(bp);
		func_exit();
		return 1;
	}


	/* Reset CD186x again  */
	if (!sx_init_CD186x(bp)) {
		sx_release_io_range(bp);
		func_exit();
		return 1;
	}

	sx_request_io_range(bp);
	bp->flags |= SX_BOARD_PRESENT;

	/* Chip           revcode   pkgtype
			  GFRCR     SRCR bit 7
	   CD180 rev B    0x81      0
	   CD180 rev C    0x82      0
	   CD1864 rev A   0x82      1
	   CD1865 rev A   0x83      1  -- Do not use!!! Does not work.
	   CD1865 rev B   0x84      1
	 -- Thanks to Gwen Wang, Cirrus Logic.
	 */

	switch (sx_in_off(bp, CD186x_GFRCR)) {
	case 0x82:
		chip = 1864;
		rev = 'A';
		break;
	case 0x83:
		chip = 1865;
		rev = 'A';
		break;
	case 0x84:
		chip = 1865;
		rev = 'B';
		break;
	case 0x85:
		chip = 1865;
		rev = 'C';
		break; /* Does not exist at this time */
	default:
		chip = -1;
		rev = 'x';
	}

	dprintk(SX_DEBUG_INIT, " GFCR = 0x%02x\n", sx_in_off(bp, CD186x_GFRCR));

	printk(KERN_INFO
    "sx%d: specialix IO8+ board detected at 0x%03x, IRQ %d, CD%d Rev. %c.\n",
				board_No(bp), bp->base, bp->irq, chip, rev);

	func_exit();
	return 0;
}

/*
 *
 *  Interrupt processing routines.
 * */

static struct specialix_port *sx_get_port(struct specialix_board *bp,
					       unsigned char const *what)
{
	unsigned char channel;
	struct specialix_port *port = NULL;

	channel = sx_in(bp, CD186x_GICR) >> GICR_CHAN_OFF;
	dprintk(SX_DEBUG_CHAN, "channel: %d\n", channel);
	if (channel < CD186x_NCH) {
		port = &sx_port[board_No(bp) * SX_NPORT + channel];
		dprintk(SX_DEBUG_CHAN, "port: %d %p flags: 0x%lx\n",
			board_No(bp) * SX_NPORT + channel,  port,
			port->port.flags & ASYNC_INITIALIZED);

		if (port->port.flags & ASYNC_INITIALIZED) {
			dprintk(SX_DEBUG_CHAN, "port: %d %p\n", channel, port);
			func_exit();
			return port;
		}
	}
	printk(KERN_INFO "sx%d: %s interrupt from invalid port %d\n",
	       board_No(bp), what, channel);
	return NULL;
}


static void sx_receive_exc(struct specialix_board *bp)
{
	struct specialix_port *port;
	struct tty_struct *tty;
	unsigned char status;
	unsigned char ch, flag;

	func_enter();

	port = sx_get_port(bp, "Receive");
	if (!port) {
		dprintk(SX_DEBUG_RX, "Hmm, couldn't find port.\n");
		func_exit();
		return;
	}
	tty = port->port.tty;

	status = sx_in(bp, CD186x_RCSR);

	dprintk(SX_DEBUG_RX, "status: 0x%x\n", status);
	if (status & RCSR_OE) {
		port->overrun++;
		dprintk(SX_DEBUG_FIFO,
			"sx%d: port %d: Overrun. Total %ld overruns.\n",
				board_No(bp), port_No(port), port->overrun);
	}
	status &= port->mark_mask;

	/* This flip buffer check needs to be below the reading of the
	   status register to reset the chip's IRQ.... */
	if (tty_buffer_request_room(tty, 1) == 0) {
		dprintk(SX_DEBUG_FIFO,
		    "sx%d: port %d: Working around flip buffer overflow.\n",
					board_No(bp), port_No(port));
		func_exit();
		return;
	}

	ch = sx_in(bp, CD186x_RDR);
	if (!status) {
		func_exit();
		return;
	}
	if (status & RCSR_TOUT) {
		printk(KERN_INFO
		    "sx%d: port %d: Receiver timeout. Hardware problems ?\n",
					board_No(bp), port_No(port));
		func_exit();
		return;

	} else if (status & RCSR_BREAK) {
		dprintk(SX_DEBUG_RX, "sx%d: port %d: Handling break...\n",
		       board_No(bp), port_No(port));
		flag = TTY_BREAK;
		if (port->port.flags & ASYNC_SAK)
			do_SAK(tty);

	} else if (status & RCSR_PE)
		flag = TTY_PARITY;

	else if (status & RCSR_FE)
		flag = TTY_FRAME;

	else if (status & RCSR_OE)
		flag = TTY_OVERRUN;

	else
		flag = TTY_NORMAL;

	if (tty_insert_flip_char(tty, ch, flag))
		tty_flip_buffer_push(tty);
	func_exit();
}


static void sx_receive(struct specialix_board *bp)
{
	struct specialix_port *port;
	struct tty_struct *tty;
	unsigned char count;

	func_enter();

	port = sx_get_port(bp, "Receive");
	if (port == NULL) {
		dprintk(SX_DEBUG_RX, "Hmm, couldn't find port.\n");
		func_exit();
		return;
	}
	tty = port->port.tty;

	count = sx_in(bp, CD186x_RDCR);
	dprintk(SX_DEBUG_RX, "port: %p: count: %d\n", port, count);
	port->hits[count > 8 ? 9 : count]++;

	tty_buffer_request_room(tty, count);

	while (count--)
		tty_insert_flip_char(tty, sx_in(bp, CD186x_RDR), TTY_NORMAL);
	tty_flip_buffer_push(tty);
	func_exit();
}


static void sx_transmit(struct specialix_board *bp)
{
	struct specialix_port *port;
	struct tty_struct *tty;
	unsigned char count;

	func_enter();
	port = sx_get_port(bp, "Transmit");
	if (port == NULL) {
		func_exit();
		return;
	}
	dprintk(SX_DEBUG_TX, "port: %p\n", port);
	tty = port->port.tty;

	if (port->IER & IER_TXEMPTY) {
		/* FIFO drained */
		sx_out(bp, CD186x_CAR, port_No(port));
		port->IER &= ~IER_TXEMPTY;
		sx_out(bp, CD186x_IER, port->IER);
		func_exit();
		return;
	}

	if ((port->xmit_cnt <= 0 && !port->break_length)
	    || tty->stopped || tty->hw_stopped) {
		sx_out(bp, CD186x_CAR, port_No(port));
		port->IER &= ~IER_TXRDY;
		sx_out(bp, CD186x_IER, port->IER);
		func_exit();
		return;
	}

	if (port->break_length) {
		if (port->break_length > 0) {
			if (port->COR2 & COR2_ETC) {
				sx_out(bp, CD186x_TDR, CD186x_C_ESC);
				sx_out(bp, CD186x_TDR, CD186x_C_SBRK);
				port->COR2 &= ~COR2_ETC;
			}
			count = min_t(int, port->break_length, 0xff);
			sx_out(bp, CD186x_TDR, CD186x_C_ESC);
			sx_out(bp, CD186x_TDR, CD186x_C_DELAY);
			sx_out(bp, CD186x_TDR, count);
			port->break_length -= count;
			if (port->break_length == 0)
				port->break_length--;
		} else {
			sx_out(bp, CD186x_TDR, CD186x_C_ESC);
			sx_out(bp, CD186x_TDR, CD186x_C_EBRK);
			sx_out(bp, CD186x_COR2, port->COR2);
			sx_wait_CCR(bp);
			sx_out(bp, CD186x_CCR, CCR_CORCHG2);
			port->break_length = 0;
		}

		func_exit();
		return;
	}

	count = CD186x_NFIFO;
	do {
		sx_out(bp, CD186x_TDR, port->xmit_buf[port->xmit_tail++]);
		port->xmit_tail = port->xmit_tail & (SERIAL_XMIT_SIZE-1);
		if (--port->xmit_cnt <= 0)
			break;
	} while (--count > 0);

	if (port->xmit_cnt <= 0) {
		sx_out(bp, CD186x_CAR, port_No(port));
		port->IER &= ~IER_TXRDY;
		sx_out(bp, CD186x_IER, port->IER);
	}
	if (port->xmit_cnt <= port->wakeup_chars)
		tty_wakeup(tty);

	func_exit();
}


static void sx_check_modem(struct specialix_board *bp)
{
	struct specialix_port *port;
	struct tty_struct *tty;
	unsigned char mcr;
	int msvr_cd;

	dprintk(SX_DEBUG_SIGNALS, "Modem intr. ");
	port = sx_get_port(bp, "Modem");
	if (port == NULL)
		return;

	tty = port->port.tty;

	mcr = sx_in(bp, CD186x_MCR);

	if ((mcr & MCR_CDCHG)) {
		dprintk(SX_DEBUG_SIGNALS, "CD just changed... ");
		msvr_cd = sx_in(bp, CD186x_MSVR) & MSVR_CD;
		if (msvr_cd) {
			dprintk(SX_DEBUG_SIGNALS, "Waking up guys in open.\n");
			wake_up_interruptible(&port->port.open_wait);
		} else {
			dprintk(SX_DEBUG_SIGNALS, "Sending HUP.\n");
			tty_hangup(tty);
		}
	}

#ifdef SPECIALIX_BRAIN_DAMAGED_CTS
	if (mcr & MCR_CTSCHG) {
		if (sx_in(bp, CD186x_MSVR) & MSVR_CTS) {
			tty->hw_stopped = 0;
			port->IER |= IER_TXRDY;
			if (port->xmit_cnt <= port->wakeup_chars)
				tty_wakeup(tty);
		} else {
			tty->hw_stopped = 1;
			port->IER &= ~IER_TXRDY;
		}
		sx_out(bp, CD186x_IER, port->IER);
	}
	if (mcr & MCR_DSSXHG) {
		if (sx_in(bp, CD186x_MSVR) & MSVR_DSR) {
			tty->hw_stopped = 0;
			port->IER |= IER_TXRDY;
			if (port->xmit_cnt <= port->wakeup_chars)
				tty_wakeup(tty);
		} else {
			tty->hw_stopped = 1;
			port->IER &= ~IER_TXRDY;
		}
		sx_out(bp, CD186x_IER, port->IER);
	}
#endif /* SPECIALIX_BRAIN_DAMAGED_CTS */

	/* Clear change bits */
	sx_out(bp, CD186x_MCR, 0);
}


/* The main interrupt processing routine */
static irqreturn_t sx_interrupt(int dummy, void *dev_id)
{
	unsigned char status;
	unsigned char ack;
	struct specialix_board *bp = dev_id;
	unsigned long loop = 0;
	int saved_reg;
	unsigned long flags;

	func_enter();

	spin_lock_irqsave(&bp->lock, flags);

	dprintk(SX_DEBUG_FLOW, "enter %s port %d room: %ld\n", __func__,
		port_No(sx_get_port(bp, "INT")),
		SERIAL_XMIT_SIZE - sx_get_port(bp, "ITN")->xmit_cnt - 1);
	if (!(bp->flags & SX_BOARD_ACTIVE)) {
		dprintk(SX_DEBUG_IRQ, "sx: False interrupt. irq %d.\n",
								bp->irq);
		spin_unlock_irqrestore(&bp->lock, flags);
		func_exit();
		return IRQ_NONE;
	}

	saved_reg = bp->reg;

	while (++loop < 16) {
		status = sx_in(bp, CD186x_SRSR) &
				(SRSR_RREQint | SRSR_TREQint | SRSR_MREQint);
		if (status == 0)
			break;
		if (status & SRSR_RREQint) {
			ack = sx_in(bp, CD186x_RRAR);

			if (ack == (SX_ID | GIVR_IT_RCV))
				sx_receive(bp);
			else if (ack == (SX_ID | GIVR_IT_REXC))
				sx_receive_exc(bp);
			else
				printk(KERN_ERR
				"sx%d: status: 0x%x Bad receive ack 0x%02x.\n",
						board_No(bp), status, ack);

		} else if (status & SRSR_TREQint) {
			ack = sx_in(bp, CD186x_TRAR);

			if (ack == (SX_ID | GIVR_IT_TX))
				sx_transmit(bp);
			else
				printk(KERN_ERR "sx%d: status: 0x%x Bad transmit ack 0x%02x. port: %d\n",
					board_No(bp), status, ack,
					port_No(sx_get_port(bp, "Int")));
		} else if (status & SRSR_MREQint) {
			ack = sx_in(bp, CD186x_MRAR);

			if (ack == (SX_ID | GIVR_IT_MODEM))
				sx_check_modem(bp);
			else
				printk(KERN_ERR
				  "sx%d: status: 0x%x Bad modem ack 0x%02x.\n",
				       board_No(bp), status, ack);

		}

		sx_out(bp, CD186x_EOIR, 0);   /* Mark end of interrupt */
	}
	bp->reg = saved_reg;
	outb(bp->reg, bp->base + SX_ADDR_REG);
	spin_unlock_irqrestore(&bp->lock, flags);
	func_exit();
	return IRQ_HANDLED;
}


/*
 *  Routines for open & close processing.
 */

static void turn_ints_off(struct specialix_board *bp)
{
	unsigned long flags;

	func_enter();
	spin_lock_irqsave(&bp->lock, flags);
	(void) sx_in_off(bp, 0); /* Turn off interrupts. */
	spin_unlock_irqrestore(&bp->lock, flags);

	func_exit();
}

static void turn_ints_on(struct specialix_board *bp)
{
	unsigned long flags;

	func_enter();

	spin_lock_irqsave(&bp->lock, flags);
	(void) sx_in(bp, 0); /* Turn ON interrupts. */
	spin_unlock_irqrestore(&bp->lock, flags);

	func_exit();
}


/* Called with disabled interrupts */
static int sx_setup_board(struct specialix_board *bp)
{
	int error;

	if (bp->flags & SX_BOARD_ACTIVE)
		return 0;

	if (bp->flags & SX_BOARD_IS_PCI)
		error = request_irq(bp->irq, sx_interrupt,
			IRQF_DISABLED | IRQF_SHARED, "specialix IO8+", bp);
	else
		error = request_irq(bp->irq, sx_interrupt,
			IRQF_DISABLED, "specialix IO8+", bp);

	if (error)
		return error;

	turn_ints_on(bp);
	bp->flags |= SX_BOARD_ACTIVE;

	return 0;
}


/* Called with disabled interrupts */
static void sx_shutdown_board(struct specialix_board *bp)
{
	func_enter();

	if (!(bp->flags & SX_BOARD_ACTIVE)) {
		func_exit();
		return;
	}

	bp->flags &= ~SX_BOARD_ACTIVE;

	dprintk(SX_DEBUG_IRQ, "Freeing IRQ%d for board %d.\n",
		 bp->irq, board_No(bp));
	free_irq(bp->irq, bp);
	turn_ints_off(bp);
	func_exit();
}

static unsigned int sx_crtscts(struct tty_struct *tty)
{
	if (sx_rtscts)
		return C_CRTSCTS(tty);
	return 1;
}

/*
 * Setting up port characteristics.
 * Must be called with disabled interrupts
 */
static void sx_change_speed(struct specialix_board *bp,
						struct specialix_port *port)
{
	struct tty_struct *tty;
	unsigned long baud;
	long tmp;
	unsigned char cor1 = 0, cor3 = 0;
	unsigned char mcor1 = 0, mcor2 = 0;
	static unsigned long again;
	unsigned long flags;

	func_enter();

	tty = port->port.tty;
	if (!tty || !tty->termios) {
		func_exit();
		return;
	}

	port->IER  = 0;
	port->COR2 = 0;
	/* Select port on the board */
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_CAR, port_No(port));

	/* The Specialix board doens't implement the RTS lines.
	   They are used to set the IRQ level. Don't touch them. */
	if (sx_crtscts(tty))
		port->MSVR = MSVR_DTR | (sx_in(bp, CD186x_MSVR) & MSVR_RTS);
	else
		port->MSVR =  (sx_in(bp, CD186x_MSVR) & MSVR_RTS);
	spin_unlock_irqrestore(&bp->lock, flags);
	dprintk(SX_DEBUG_TERMIOS, "sx: got MSVR=%02x.\n", port->MSVR);
	baud = tty_get_baud_rate(tty);

	if (baud == 38400) {
		if ((port->port.flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI)
			baud = 57600;
		if ((port->port.flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI)
			baud = 115200;
	}

	if (!baud) {
		/* Drop DTR & exit */
		dprintk(SX_DEBUG_TERMIOS, "Dropping DTR...  Hmm....\n");
		if (!sx_crtscts(tty)) {
			port->MSVR &= ~MSVR_DTR;
			spin_lock_irqsave(&bp->lock, flags);
			sx_out(bp, CD186x_MSVR, port->MSVR);
			spin_unlock_irqrestore(&bp->lock, flags);
		} else
			dprintk(SX_DEBUG_TERMIOS, "Can't drop DTR: no DTR.\n");
		return;
	} else {
		/* Set DTR on */
		if (!sx_crtscts(tty))
			port->MSVR |= MSVR_DTR;
	}

	/*
	 * Now we must calculate some speed depended things
	 */

	/* Set baud rate for port */
	tmp = port->custom_divisor ;
	if (tmp)
		printk(KERN_INFO
			"sx%d: Using custom baud rate divisor %ld. \n"
			"This is an untested option, please be careful.\n",
							port_No(port), tmp);
	else
		tmp = (((SX_OSCFREQ + baud/2) / baud + CD186x_TPC/2) /
								CD186x_TPC);

	if (tmp < 0x10 && time_before(again, jiffies)) {
		again = jiffies + HZ * 60;
		/* Page 48 of version 2.0 of the CL-CD1865 databook */
		if (tmp >= 12) {
			printk(KERN_INFO "sx%d: Baud rate divisor is %ld. \n"
				"Performance degradation is possible.\n"
				"Read specialix.txt for more info.\n",
						port_No(port), tmp);
		} else {
			printk(KERN_INFO "sx%d: Baud rate divisor is %ld. \n"
		"Warning: overstressing Cirrus chip. This might not work.\n"
		"Read specialix.txt for more info.\n", port_No(port), tmp);
		}
	}
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_RBPRH, (tmp >> 8) & 0xff);
	sx_out(bp, CD186x_TBPRH, (tmp >> 8) & 0xff);
	sx_out(bp, CD186x_RBPRL, tmp & 0xff);
	sx_out(bp, CD186x_TBPRL, tmp & 0xff);
	spin_unlock_irqrestore(&bp->lock, flags);
	if (port->custom_divisor)
		baud = (SX_OSCFREQ + port->custom_divisor/2) /
							port->custom_divisor;
	baud = (baud + 5) / 10;		/* Estimated CPS */

	/* Two timer ticks seems enough to wakeup something like SLIP driver */
	tmp = ((baud + HZ/2) / HZ) * 2 - CD186x_NFIFO;
	port->wakeup_chars = (tmp < 0) ? 0 : ((tmp >= SERIAL_XMIT_SIZE) ?
					      SERIAL_XMIT_SIZE - 1 : tmp);

	/* Receiver timeout will be transmission time for 1.5 chars */
	tmp = (SPECIALIX_TPS + SPECIALIX_TPS/2 + baud/2) / baud;
	tmp = (tmp > 0xff) ? 0xff : tmp;
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_RTPR, tmp);
	spin_unlock_irqrestore(&bp->lock, flags);
	switch (C_CSIZE(tty)) {
	case CS5:
		cor1 |= COR1_5BITS;
		break;
	case CS6:
		cor1 |= COR1_6BITS;
		break;
	case CS7:
		cor1 |= COR1_7BITS;
		break;
	case CS8:
		cor1 |= COR1_8BITS;
		break;
	}

	if (C_CSTOPB(tty))
		cor1 |= COR1_2SB;

	cor1 |= COR1_IGNORE;
	if (C_PARENB(tty)) {
		cor1 |= COR1_NORMPAR;
		if (C_PARODD(tty))
			cor1 |= COR1_ODDP;
		if (I_INPCK(tty))
			cor1 &= ~COR1_IGNORE;
	}
	/* Set marking of some errors */
	port->mark_mask = RCSR_OE | RCSR_TOUT;
	if (I_INPCK(tty))
		port->mark_mask |= RCSR_FE | RCSR_PE;
	if (I_BRKINT(tty) || I_PARMRK(tty))
		port->mark_mask |= RCSR_BREAK;
	if (I_IGNPAR(tty))
		port->mark_mask &= ~(RCSR_FE | RCSR_PE);
	if (I_IGNBRK(tty)) {
		port->mark_mask &= ~RCSR_BREAK;
		if (I_IGNPAR(tty))
			/* Real raw mode. Ignore all */
			port->mark_mask &= ~RCSR_OE;
	}
	/* Enable Hardware Flow Control */
	if (C_CRTSCTS(tty)) {
#ifdef SPECIALIX_BRAIN_DAMAGED_CTS
		port->IER |= IER_DSR | IER_CTS;
		mcor1 |= MCOR1_DSRZD | MCOR1_CTSZD;
		mcor2 |= MCOR2_DSROD | MCOR2_CTSOD;
		spin_lock_irqsave(&bp->lock, flags);
		tty->hw_stopped = !(sx_in(bp, CD186x_MSVR) &
							(MSVR_CTS|MSVR_DSR));
		spin_unlock_irqrestore(&bp->lock, flags);
#else
		port->COR2 |= COR2_CTSAE;
#endif
	}
	/* Enable Software Flow Control. FIXME: I'm not sure about this */
	/* Some people reported that it works, but I still doubt it */
	if (I_IXON(tty)) {
		port->COR2 |= COR2_TXIBE;
		cor3 |= (COR3_FCT | COR3_SCDE);
		if (I_IXANY(tty))
			port->COR2 |= COR2_IXM;
		spin_lock_irqsave(&bp->lock, flags);
		sx_out(bp, CD186x_SCHR1, START_CHAR(tty));
		sx_out(bp, CD186x_SCHR2, STOP_CHAR(tty));
		sx_out(bp, CD186x_SCHR3, START_CHAR(tty));
		sx_out(bp, CD186x_SCHR4, STOP_CHAR(tty));
		spin_unlock_irqrestore(&bp->lock, flags);
	}
	if (!C_CLOCAL(tty)) {
		/* Enable CD check */
		port->IER |= IER_CD;
		mcor1 |= MCOR1_CDZD;
		mcor2 |= MCOR2_CDOD;
	}

	if (C_CREAD(tty))
		/* Enable receiver */
		port->IER |= IER_RXD;

	/* Set input FIFO size (1-8 bytes) */
	cor3 |= sx_rxfifo;
	/* Setting up CD186x channel registers */
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_COR1, cor1);
	sx_out(bp, CD186x_COR2, port->COR2);
	sx_out(bp, CD186x_COR3, cor3);
	spin_unlock_irqrestore(&bp->lock, flags);
	/* Make CD186x know about registers change */
	sx_wait_CCR(bp);
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_CCR, CCR_CORCHG1 | CCR_CORCHG2 | CCR_CORCHG3);
	/* Setting up modem option registers */
	dprintk(SX_DEBUG_TERMIOS, "Mcor1 = %02x, mcor2 = %02x.\n",
								mcor1, mcor2);
	sx_out(bp, CD186x_MCOR1, mcor1);
	sx_out(bp, CD186x_MCOR2, mcor2);
	spin_unlock_irqrestore(&bp->lock, flags);
	/* Enable CD186x transmitter & receiver */
	sx_wait_CCR(bp);
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_CCR, CCR_TXEN | CCR_RXEN);
	/* Enable interrupts */
	sx_out(bp, CD186x_IER, port->IER);
	/* And finally set the modem lines... */
	sx_out(bp, CD186x_MSVR, port->MSVR);
	spin_unlock_irqrestore(&bp->lock, flags);

	func_exit();
}


/* Must be called with interrupts enabled */
static int sx_setup_port(struct specialix_board *bp,
						struct specialix_port *port)
{
	unsigned long flags;

	func_enter();

	if (port->port.flags & ASYNC_INITIALIZED) {
		func_exit();
		return 0;
	}

	if (!port->xmit_buf) {
		/* We may sleep in get_zeroed_page() */
		unsigned long tmp;

		tmp = get_zeroed_page(GFP_KERNEL);
		if (tmp == 0L) {
			func_exit();
			return -ENOMEM;
		}

		if (port->xmit_buf) {
			free_page(tmp);
			func_exit();
			return -ERESTARTSYS;
		}
		port->xmit_buf = (unsigned char *) tmp;
	}

	spin_lock_irqsave(&port->lock, flags);

	if (port->port.tty)
		clear_bit(TTY_IO_ERROR, &port->port.tty->flags);

	port->xmit_cnt = port->xmit_head = port->xmit_tail = 0;
	sx_change_speed(bp, port);
	port->port.flags |= ASYNC_INITIALIZED;

	spin_unlock_irqrestore(&port->lock, flags);


	func_exit();
	return 0;
}


/* Must be called with interrupts disabled */
static void sx_shutdown_port(struct specialix_board *bp,
						struct specialix_port *port)
{
	struct tty_struct *tty;
	int i;
	unsigned long flags;

	func_enter();

	if (!(port->port.flags & ASYNC_INITIALIZED)) {
		func_exit();
		return;
	}

	if (sx_debug & SX_DEBUG_FIFO) {
		dprintk(SX_DEBUG_FIFO,
			"sx%d: port %d: %ld overruns, FIFO hits [ ",
				board_No(bp), port_No(port), port->overrun);
		for (i = 0; i < 10; i++)
			dprintk(SX_DEBUG_FIFO, "%ld ", port->hits[i]);
		dprintk(SX_DEBUG_FIFO, "].\n");
	}

	if (port->xmit_buf) {
		free_page((unsigned long) port->xmit_buf);
		port->xmit_buf = NULL;
	}

	/* Select port */
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_CAR, port_No(port));

	tty = port->port.tty;
	if (tty == NULL || C_HUPCL(tty)) {
		/* Drop DTR */
		sx_out(bp, CD186x_MSVDTR, 0);
	}
	spin_unlock_irqrestore(&bp->lock, flags);
	/* Reset port */
	sx_wait_CCR(bp);
	spin_lock_irqsave(&bp->lock, flags);
	sx_out(bp, CD186x_CCR, CCR_SOFTRESET);
	/* Disable all interrupts from this port */
	port->IER = 0;
	sx_out(bp, CD186x_IER, port->IER);
	spin_unlock_irqrestore(&bp->lock, flags);
	if (tty)
		set_bit(TTY_IO_ERROR, &tty->flags);
	port->port.flags &= ~ASYNC_INITIALIZED;

	if (!bp->count)
		sx_shutdown_board(bp);
	func_exit();
}


static int block_til_ready(struct tty_struct *tty, struct file *filp,
						struct specialix_port *port)
{
	DECLARE_WAITQUEUE(wait,  current);
	struct specialix_board *bp = port_Board(port);
	int    retval;
	int    do_clocal = 0;
	int    CD;
	unsigned long flags;

	func_enter();

	/*
	 * If the device is in the middle of being closed, then block
	 * until it's done, and then try again.
	 */
	if (tty_hung_up_p(filp) || port->port.flags & ASYNC_CLOSING) {
		interruptible_sleep_on(&port->port.close_wait);
		if (port->port.flags & ASYNC_HUP_NOTIFY) {
			func_exit();
			return -EAGAIN;
		} else {
			func_exit();
			return -ERESTARTSYS;
		}
	}

	/*
	 * If non-blocking mode is set, or the port is not enabled,
	 * then make the check up front and then exit.
	 */
	if ((filp->f_flags & O_NONBLOCK) ||
	    (tty->flags & (1 << TTY_IO_ERROR))) {
		port->port.flags |= ASYNC_NORMAL_ACTIVE;
		func_exit();
		return 0;
	}

	if (C_CLOCAL(tty))
		do_clocal = 1;

	/*
	 * Block waiting for the carrier detect and the line to become
	 * free (i.e., not in use by the callout).  While we are in
	 * this loop, info->count is dropped by one, so that
	 * rs_close() knows when to free things.  We restore it upon
	 * exit, either normal or abnormal.
	 */
	retval = 0;
	add_wait_queue(&port->port.open_wait, &wait);
	spin_lock_irqsave(&port->lock, flags);
	if (!tty_hung_up_p(filp))
		port->port.count--;
	spin_unlock_irqrestore(&port->lock, flags);
	port->port.blocked_open++;
	while (1) {
		spin_lock_irqsave(&bp->lock, flags);
		sx_out(bp, CD186x_CAR, port_No(port));
		CD = sx_in(bp, CD186x_MSVR) & MSVR_CD;
		if (sx_crtscts(tty)) {
			/* Activate RTS */
			port->MSVR |= MSVR_DTR;		/* WTF? */
			sx_out(bp, CD186x_MSVR, port->MSVR);
		} else {
			/* Activate DTR */
			port->MSVR |= MSVR_DTR;
			sx_out(bp, CD186x_MSVR, port->MSVR);
		}
		spin_unlock_irqrestore(&bp->lock, flags);
		set_current_state(TASK_INTERRUPTIBLE);
		if (tty_hung_up_p(filp) ||
		    !(port->port.flags & ASYNC_INITIALIZED)) {
			if (port->port.flags & ASYNC_HUP_NOTIFY)
				retval = -EAGAIN;
			else
				retval = -ERESTARTSYS;
			break;
		}
		if (!(port->port.flags & ASYNC_CLOSING) &&
		    (do_clocal || CD))
			break;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		schedule();
	}

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&port->port.open_wait, &wait);
	spin_lock_irqsave(&port->lock, flags);
	if (!tty_hung_up_p(filp))
		port->port.count++;
	port->port.blocked_open--;
	spin_unlock_irqrestore(&port->lock, flags);
	if (retval) {
		func_exit();
		return retval;
	}

	port->port.flags |= ASYNC_NORMAL_ACTIVE;
	func_exit();
	return 0;
}


static int sx_open(struct tty_struct *tty, struct file *filp)
{
	int board;
	int error;
	struct specialix_port *port;
	struct specialix_board *bp;
	int i;
	unsigned long flags;

	func_enter();

	board = SX_BOARD(tty->index);

	if (board >= SX_NBOARD || !(sx_board[board].flags & SX_BOARD_PRESENT)) {
		func_exit();
		return -ENODEV;
	}

	bp = &sx_board[board];
	port = sx_port + board * SX_NPORT + SX_PORT(tty->index);
	port->overrun = 0;
	for (i = 0; i < 10; i++)
		port->hits[i] = 0;

	dprintk(SX_DEBUG_OPEN,
			"Board = %d, bp = %p, port = %p, portno = %d.\n",
				 board, bp, port, SX_PORT(tty->index));

	if (sx_paranoia_check(port, tty->name, "sx_open")) {
		func_enter();
		return -ENODEV;
	}

	error = sx_setup_board(bp);
	if (error) {
		func_exit();
		return error;
	}

	spin_lock_irqsave(&bp->lock, flags);
	port->port.count++;
	bp->count++;
	tty->driver_data = port;
	port->port.tty = tty;
	spin_unlock_irqrestore(&bp->lock, flags);

	error = sx_setup_port(bp, port);
	if (error) {
		func_enter();
		return error;
	}

	error = block_til_ready(tty, filp, port);
	if (error) {
		func_enter();
		return error;
	}

	func_exit();
	return 0;
}

static void sx_flush_buffer(struct tty_struct *tty)
{
	struct specialix_port *port = tty->driver_data;
	unsigned long flags;
	struct specialix_board  *bp;

	func_enter();

	if (sx_paranoia_check(port, tty->name, "sx_flush_buffer")) {
		func_exit();
		return;
	}

	bp = port_Board(port);
	spin_lock_irqsave(&port->lock, flags);
	port->xmit_cnt = port->xmit_head = port->xmit_tail = 0;
	spin_unlock_irqrestore(&port->lock, flags);
	tty_wakeup(tty);

	func_exit();
}

static void sx_close(struct tty_struct *tty, struct file *filp)
{
	struct specialix_port *port = tty->driver_data;
	struct specialix_board *bp;
	unsigned long flags;
	unsigned long timeout;

	func_enter();
	if (!port || sx_paranoia_check(port, tty->name, "close")) {
		func_exit();
		return;
	}
	spin_lock_irqsave(&port->lock, flags);

	if (tty_hung_up_p(filp)) {
		spin_unlock_irqrestore(&port->lock, flags);
		func_exit();
		return;
	}

	bp = port_Board(port);
	if (tty->count == 1 && port->port.count != 1) {
		printk(KERN_ERR "sx%d: sx_close: bad port count;"
		       " tty->count is 1, port count is %d\n",
		       board_No(bp), port->port.count);
		port->port.count = 1;
	}

	if (port->port.count > 1) {
		port->port.count--;
		bp->count--;

		spin_unlock_irqrestore(&port->lock, flags);

		func_exit();
		return;
	}
	port->port.flags |= ASYNC_CLOSING;
	/*
	 * Now we wait for the transmit buffer to clear; and we notify
	 * the line discipline to only process XON/XOFF characters.
	 */
	tty->closing = 1;
	spin_unlock_irqrestore(&port->lock, flags);
	dprintk(SX_DEBUG_OPEN, "Closing\n");
	if (port->port.closing_wait != ASYNC_CLOSING_WAIT_NONE)
		tty_wait_until_sent(tty, port->port.closing_wait);
	/*
	 * At this point we stop accepting input.  To do this, we
	 * disable the receive line status interrupts, and tell the
	 * interrupt driver to stop checking the data ready bit in the
	 * line status register.
	 */
	dprintk(SX_DEBUG_OPEN, "Closed\n");
	port->IER &= ~IER_RXD;
	if (port->port.flags & ASYNC_INITIALIZED) {
		port->IER &= ~IER_TXRDY;
		port->IER |= IER_TXEMPTY;
		spin_lock_irqsave(&bp->lock, flags);
		sx_out(bp, CD186x_CAR, port_No(port));
		sx_out(bp, CD186x_IER, port->IER);
		spin_unlock_irqrestore(&bp->lock, flags);
		/*
		 * Before we drop DTR, make sure the UART transmitter
		 * has completely drained; this is especially
		 * important if there is a transmit FIFO!
		 */
		timeout = jiffies+HZ;
		while (port->IER & IER_TXEMPTY) {
			set_current_state(TASK_INTERRUPTIBLE);
			msleep_interruptible(jiffies_to_msecs(port->timeout));
			if (time_after(jiffies, timeout)) {
				printk(KERN_INFO "Timeout waiting for close\n");
				break;
			}
		}

	}

	if (--bp->count < 0) {
		printk(KERN_ERR
		    "sx%d: sx_shutdown_port: bad board count: %d port: %d\n",
				board_No(bp), bp->count, tty->index);
		bp->count = 0;
	}
	if (--port->port.count < 0) {
		printk(KERN_ERR
			"sx%d: sx_close: bad port count for tty%d: %d\n",
				board_No(bp), port_No(port), port->port.count);
		port->port.count = 0;
	}

	sx_shutdown_port(bp, port);
	sx_flush_buffer(tty);
	tty_ldisc_flush(tty);
	spin_lock_irqsave(&port->lock, flags);
	tty->closing = 0;
	port->port.tty = NULL;
	spin_unlock_irqrestore(&port->lock, flags);
	if (port->port.blocked_open) {
		if (port->port.close_delay)
			msleep_interruptible(
				jiffies_to_msecs(port->port.close_delay));
		wake_up_interruptible(&port->port.open_wait);
	}
	port->port.flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CLOSING);
	wake_up_interruptible(&port->port.close_wait);

	func_exit();
}


static int sx_write(struct tty_struct *tty,
					const unsigned char *buf, int count)
{
	struct specialix_port *port = tty->driver_data;
	struct specialix_board *bp;
	int c, total = 0;
	unsigned long flags;

	func_enter();
	if (sx_paranoia_check(port, tty->name, "sx_write")) {
		func_exit();
		return 0;
	}

	bp = port_Board(port);

	if (!port->xmit_buf) {
		func_exit();
		return 0;
	}

	while (1) {
		spin_lock_irqsave(&port->lock, flags);
		c = min_t(int, count, min(SERIAL_XMIT_SIZE - port->xmit_cnt - 1,
				   SERIAL_XMIT_SIZE - port->xmit_head));
		if (c <= 0) {
			spin_unlock_irqrestore(&port->lock, flags);
			break;
		}
		memcpy(port->xmit_buf + port->xmit_head, buf, c);
		port->xmit_head = (port->xmit_head + c) & (SERIAL_XMIT_SIZE-1);
		port->xmit_cnt += c;
		spin_unlock_irqrestore(&port->lock, flags);

		buf += c;
		count -= c;
		total += c;
	}

	spin_lock_irqsave(&bp->lock, flags);
	if (port->xmit_cnt && !tty->stopped && !tty->hw_stopped &&
	    !(port->IER & IER_TXRDY)) {
		port->IER |= IER_TXRDY;
		sx_out(bp, CD186x_CAR, port_No(port));
		sx_out(bp, CD186x_IER, port->IER);
	}
	spin_unlock_irqrestore(&bp->lock, flags);
	func_exit();

	return total;
}


static int sx_put_char(struct tty_struct *tty, unsigned char ch)
{
	struct specialix_port *port = tty->driver_data;
	unsigned long flags;
	struct specialix_board  *bp;

	func_enter();

	if (sx_paranoia_check(port, tty->name, "sx_put_char")) {
		func_exit();
		return 0;
	}
	dprintk(SX_DEBUG_TX, "check tty: %p %p\n", tty, port->xmit_buf);
	if (!port->xmit_buf) {
		func_exit();
		return 0;
	}
	bp = port_Board(port);
	spin_lock_irqsave(&port->lock, flags);

	dprTYPE_AUTH :
            if (p1 == 0x30001)
            {
                hlen += 4;
                offset += 4;        // for reserving wep header
                encrypt = 1;
            }

            /* Algotrithm Number */
            zmw_tx_buf_writeh(dev, buf, offset, (u16_t)(p1&0xffff));
            offset+=2;

            /* Transaction Number */
            zmw_tx_buf_writeh(dev, buf, offset, (u16_t)(p1>>16));
            offset+=2;

            /* Status Code */
            zmw_tx_buf_writeh(dev, buf, offset, (u16_t)p2);
            offset+=2;

            if (wd->wlanMode == ZM_MODE_AP)
            {
                vap = (u16_t) p3;
            }

            /* Challenge Text => share-2 or share-3 */
            if (p1 == 0x20001)
            {
                if (p2 == 0) //Status == success
                {
                    zmw_buf_writeh(dev, buf, offset, 0x8010);
                    offset+=2;
                    /* share-2 : AP generate challenge text */
                    for (i=0; i<128; i++)
                    {
                        wd->ap.challengeText[i] = (u8_t)zfGetRandomNumber(dev, 0);
                    }
                    zfCopyToIntTxBuffer(dev, buf, wd->ap.challengeText, offset, 128);
                    offset += 128;
                }
            }
            else if (p1 == 0x30001)
            {
                /* share-3 : STA return challenge Text */
                zfCopyToIntTxBuffer(dev, buf, wd->sta.challengeText, offset, wd->sta.challengeText[1]+2);
                offset += (wd->sta.challengeText[1]+2);
            }

            break;

        case ZM_WLAN_FRAME_TYPE_ASOCREQ :
        case ZM_WLAN_FRAME_TYPE_REASOCREQ :
            /* Capability */
            zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.capability[0]);
            zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.capability[1]);

            /* Listen Interval */
            zmw_tx_buf_writeh(dev, buf, offset, 0x0005);
            offset+=2;

            /* Reassocaited Request : Current AP address */
            if (frameType == ZM_WLAN_FRAME_TYPE_REASOCREQ)
            {
            zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[0]);
                offset+=2;
            zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[1]);
                offset+=2;
            zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[2]);
                offset+=2;
            }

            /* SSID */
            offset = zfStaAddIeSsid(dev, buf, offset);


            if ( wd->sta.currentFrequency < 3000 )
            {
                /* Support Rate */
                offset = zfMmAddIeSupportRate(dev, buf, offset, ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_CCK);
            }
            else
            {
                /* Support Rate */
                offset = zfMmAddIeSupportRate(dev, buf, offset, ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_OFDM);
            }

            if ((wd->sta.capability[1] & ZM_BIT_0) == 1)
            {   //spectrum managment flag enable
                offset = zfStaAddIePowerCap(dev, buf, offset);
                offset = zfStaAddIeSupportCh(dev, buf, offset);
            }

            if (wd->sta.currentFrequency < 3000)
            {
                /* Extended Supported Rates */
                if (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N))
                {
                    offset = zfMmAddIeSupportRate(dev, buf, offset, ZM_WLAN_EID_EXTENDED_RATE, ZM_RATE_SET_OFDM);
                }
            }


            //offset = zfStaAddIeWpaRsn(dev, buf, offset, frameType);
            //Move to wrapper function, for OS difference--CWYang(m)
            //for windows wrapper, zfwStaAddIeWpaRsn() should be below:
            //u16_t zfwStaAddIeWpaRsn(zdev_t* dev, zbuf_t* buf, u16_t offset, u8_t frameType)
            //{
            //    return zfStaAddIeWpaRsn(dev, buf, offset, frameType);
            //}
            offset = zfwStaAddIeWpaRsn(dev, buf, offset, frameType);

#ifdef ZM_ENABLE_CENC
            /* CENC */
            //if (wd->sta.encryMode == ZM_CENC)
            offset = zfStaAddIeCenc(dev, buf, offset);
#endif //ZM_ENABLE_CENC
            if (((wd->sta.wmeEnabled & ZM_STA_WME_ENABLE_BIT) != 0) //WME enabled
              && ((wd->sta.apWmeCapability & 0x1) != 0)) //WME AP
            {
                if (((wd->sta.apWmeCapability & 0x80) != 0) //UAPSD AP
                 && ((wd->sta.wmeEnabled & ZM_STA_UAPSD_ENABLE_BIT) != 0)) //UAPSD enabled
                {
                    offset = zfStaAddIeWmeInfo(dev, buf, offset, wd->sta.wmeQosInfo);
                }
                else
                {
                    offset = zfStaAddIeWmeInfo(dev, buf, offset, 0);
                }
            }
            // jhlee HT 0
            //CWYang(+)
            if (wd->sta.EnableHT != 0)
            {
                #ifndef ZM_DISABLE_AMSDU8K_SUPPORT
                    //Support 8K A-MSDU
                    if (wd->sta.wepStatus == ZM_ENCRYPTION_WEP_DISABLED)
                    {
                        wd->sta.HTCap.Data.HtCapInfo |= HTCAP_MaxAMSDULength;
                    }
                    else
                    {
                        wd->sta.HTCap.Data.HtCapInfo &= (~HTCAP_MaxAMSDULength);
                    }
                #else
                    //Support 4K A-MSDU
                    wd->sta.HTCap.Data.HtCapInfo &= (~HTCAP_MaxAMSDULength);
                #endif

                /* HT Capabilities Info */
                if (wd->BandWidth40 == 1) {
                    wd->sta.HTCap.Data.HtCapInfo |= HTCAP_SupChannelWidthSet;
                }
                else {
                    wd->sta.HTCap.Data.HtCapInfo &= ~HTCAP_SupChannelWidthSet;
                    //wd->sta.HTCap.Data.HtCapInfo |= HTCAP_SupChannelWidthSet;
                }

                wd->sta.HTCap.Data.AMPDUParam &= ~HTCAP_MaxRxAMPDU3;
                wd->sta.HTCap.Data.AMPDUParam |= HTCAP_MaxRxAMPDU3;
                wd->sta.HTCap.Data.MCSSet[1] = 0xFF; // MCS 8 ~ 15
                offset = zfMmAddHTCapability(dev, buf, offset);
                offset = zfMmAddPreNHTCapability(dev, buf, offset);
                //CWYang(+)
                /* Extended HT Capabilities Info */
                //offset = zfMmAddExtendedHTCapability(dev, buf, offset);
            }


            //Store asoc request frame body, for VISTA only
            wd->sta.asocReqFrameBodySize = ((offset - hlen) >
                    ZM_CACHED_FRAMEBODY_SIZE)?
                    ZM_CACHED_FRAMEBODY_SIZE:(offset - hlen);
            for (i=0; i<wd->sta.asocReqFrameBodySize; i++)
            {
                wd->sta.asocReqFrameBody[i] = zmw_tx_buf_readb(dev, buf, i + hlen);
            }
            break;

        case ZM_WLAN_FRAME_TYPE_ASOCRSP :
        case ZM_WLAN_FRAME_TYPE_REASOCRSP :
            vap = (u16_t) p3;

            /* Capability */
            zmw_tx_buf_writeh(dev, buf, offset, wd->ap.capab[vap]);
            offset+=2;

            /* Status Code */
            zmw_tx_buf_writeh(dev, buf, offset, (u16_t)p1);
            offset+=2;

            /* AID */
            zmw_tx_buf_writeh(dev, buf, offset, (u16_t)(p2|0xc000));
            offset+=2;


            if ( wd->frequency < 3000 )
            {
            /* Support Rate */
            offset = zfMmAddIeSupportRate(dev, buf, offset, ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_CCK);

            /* Extended Supported Rates */
            offset = zfMmAddIeSupportRate(dev, buf, offset, ZM_WLAN_EID_EXTENDED_RATE, ZM_RATE_SET_OFDM);
            }
            else
            {
                /* Support Rate */
                offset = zfMmAddIeSupportRate(dev, buf, offset, ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_OFDM);
            }



            /* WME Parameters */
            if (wd->wlanMode == ZM_MODE_AP)
            {
                /* TODO : if WME STA then send WME parameter element */
                if (wd->ap.qosMode == 1)
                {
                    offset = zfApAddIeWmePara(dev, buf, offset, vap);
                }
            }
            // jhlee HT 0
            //CWYang(+)
            /* HT Capabilities Info */
            offset = zfMmAddHTCapability(dev, buf, offset);
            //CWYang(+)
            /* Extended HT Capabilities Info */
            offset = zfMmAddExtendedHTCapability(dev, buf, offset);
            break;

        case ZM_WLAN_FRAME_TYPE_ATIM :
            /* NULL frame */
            /* TODO : add two dumb bytes temporarily */
            offset += 2;
            break;

        case ZM_WLAN_FRAME_TYPE_QOS_NULL :
            zmw_buf_writeh(dev, buf, offset, 0x0010);
            offset += 2;
            break;

        case ZM_WLAN_DATA_FRAME :
            break;

        case ZM_WLAN_FRAME_TYPE_DISASOC :
        case ZM_WLAN_FRAME_TYPE_DEAUTH :
            if (wd->wlanMode == ZM_MODE_AP)
            {
              vap = (u16_t) p3;

              if ((aid = zfApFindSta(dev, dst)) != 0xffff)
              {
                  zmw_enter_critical_section(dev);
                  /* Clear STA table */
                  wd->ap.staTable[aid].valid = 0;

                  zmw_leave_critical_section(dev);

                  if (wd->zfcbDisAsocNotify != NULL)
                  {
                      wd->zfcbDisAsocNotify(dev, (u8_t*)dst, vap);
                  }
              }
            }
            /* Reason Code */
            zmw_tx_buf_writeh(dev, buf, offset, (u16_t)p1);
            offset+=2;
            break;
    }

    zfwBufSetSize(dev, buf, offset);

    zm_msg2_mm(ZM_LV_2, "management frame body size=", offset-hlen);

    //Copy wlan header
    zfTxGenMmHeader(dev, frameType, dst, header, offset-hlen, buf, vap, encrypt);
    for (i=0; i<(hlen>>1); i++)
    {
        zmw_tx_buf_writeh(dev, buf, i*2, header[i]);
    }

    /* Get buffer DMA address */
    //if ((addrTblSize = zfwBufMapDma(dev, buf, &addrTbl)) == 0)
    //if ((addrTblSize = zfwMapTxDma(dev, buf, &addrTbl)) == 0)
    //{
    //    goto zlError;
    //}

    zm_msg2_mm(ZM_LV_2, "offset=", offset);
    zm_msg2_mm(ZM_LV_2, "hlen=", hlen);
    //zm_msg2_mm(ZM_LV_2, "addrTblSize=", addrTblSize);
    //zm_msg2_mm(ZM_LV_2, "addrTbl.len[0]=", addrTbl.len[0]);
    //zm_msg2_mm(ZM_LV_2, "addrTbl.physAddrl[0]=", addrTbl.physAddrl[0]);
    //zm_msg2_mm(ZM_LV_2, "buf->data=", buf->data);

    #if 0
    if ((err = zfHpSend(dev, NULL, 0, NULL, 0, NULL, 0, buf, 0,
            ZM_INTERNAL_ALLOC_BUF, 0, 0xff)) != ZM_SUCCESS)
    {
        goto zlError;
    }
    #else
    zfPutVmmq(dev, buf);
    zfPushVtxq(dev);
    #endif

    return;
#if 0
zlError:

    zfwBufFree(dev, buf, 0);
    return;
#endif
}


/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfProcessManagement         */
/*      Process received management frame.                              */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : received management frame buffer                           */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      none                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        ZyDAS Technology Corporation    2005.10     */
/*                                                                      */
/************************************************************************/
void zfProcessManagement(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* AddInfo) //CWYang(m)
{
    u8_t frameType;
    u16_t ta[3];
    u16_t ra[3];
    u16_t vap = 0, index = 0;
    //u16_t i;

    zmw_get_wlan_dev(dev);

    ra[0] = zmw_rx_buf_readh(dev, buf, 4);
    ra[1] = zmw_rx_buf_readh(dev, buf, 6);
    ra[2] = zmw_rx_buf_readh(dev, buf, 8);

    ta[0] = zmw_rx_buf_readh(dev, buf, 10);
    ta[1] = zmw_rx_buf_readh(dev, buf, 12);
    ta[2] = zmw_rx_buf_readh(dev, buf, 14);

    frameType = zmw_rx_buf_readb(dev, buf, 0);

    if (wd->wlanMode == ZM_MODE_AP)
    {
#if 1
        vap = 0;
        if ((ra[0] & 0x1) != 1)
        {
            /* AP : Find virtual AP */
            if ((index = zfApFindSta(dev, ta)) != 0xffff)
            {
                vap = wd->ap.staTable[index].vap;
            }
        }
        zm_msg2_mm(ZM_LV_2, "vap=", vap);
#endif

        /* Dispatch by frame type */
        switch (frameType)
        {
                /* Beacon */
            case ZM_WLAN_FRAME_TYPE_BEACON :
                zfApProcessBeacon(dev, buf);
                break;
                /* Authentication */
            case ZM_WLAN_FRAME_TYPE_AUTH :
                zfApProcessAuth(dev, buf, ta, vap);
                break;
                /* Association request */
            case ZM_WLAN_FRAME_TYPE_ASOCREQ :
                /* Reassociation request */
            case ZM_WLAN_FRAME_TYPE_REASOCREQ :
                zfApProcessAsocReq(dev, buf, ta, vap);
                break;
                /* Association response */
            case ZM_WLAN_FRAME_TYPE_ASOCRSP :
                //zfApProcessAsocRsp(dev, buf);
                break;
                /* Deauthentication */
            case ZM_WLAN_FRAME_TYPE_DEAUTH :
                zfApProcessDeauth(dev, buf, ta, vap);
                break;
                /* Disassociation */
            case ZM_WLAN_FRAME_TYPE_DISASOC :
                zfApProcessDisasoc(dev, buf, ta, vap);
                break;
                /* Probe request */
            case ZM_WLAN_FRAME_TYPE_PROBEREQ :
                zfProcessProbeReq(dev, buf, ta);
                break;
                /* Probe response */
            case ZM_WLAN_FRAME_TYPE_PROBERSP :
                zfApProcessProbeRsp(dev, buf, AddInfo);
                break;
                /* Action */
            case ZM_WLAN_FRAME_TYPE_ACTION :
                zfApProcessAction(dev, buf);
                break;
        }
    }
    else //if ((wd->wlanMode == ZM_MODE_INFRASTRUCTURE) || (wd->wlanMode == ZM_MODE_IBSS))
    {
        /* Dispatch by frame type */
        switch (frameType)
        {
                /* Beacon */
            case ZM_WLAN_FRAME_TYPE_BEACON :
                /* if enable 802.11h and current chanel is silent but receive beacon from other AP */
                if (((wd->regulationTable.allowChannel[wd->regulationTable.CurChIndex].channelFlags
                        & ZM_REG_FLAG_CHANNEL_CSA) != 0) && wd->sta.DFSEnable)
                {
                    wd->regulationTable.allowChannel[wd->regulationTable.CurChIndex].channelFlags
                            &= ~(ZM_REG_FLAG_CHANNEL_CSA & ZM_REG_FLAG_CHANNEL_PASSIVE);
                }
                zfStaProcessBeacon(dev, buf, AddInfo); //CWYang(m)
                break;
                /* Authentication */
            case ZM_WLAN_FRAME_TYPE_AUTH :
                /* TODO : vap parameter is useless in STA mode, get rid of it */
                zfStaProcessAuth(dev, buf, ta, 0);
                break;
                /* Association request */
            case ZM_WLAN_FRAME_TYPE_ASOCREQ :
                /* TODO : vap parameter is useless in STA mode, get rid of it */
                zfStaProcessAsocReq(dev, buf, ta, 0);
                break;
                /* Association response */
            case ZM_WLAN_FRAME_TYPE_ASOCRSP :
                /* Reassociation request */
            case ZM_WLAN_FRAME_TYPE_REASOCRSP :
                zfStaProcessAsocRsp(dev, buf);
                break;
                /* Deauthentication */
            case ZM_WLAN_FRAME_TYPE_DEAUTH :
                zm_debug_msg0("Deauthentication received");
                zfStaProcessDeauth(dev, buf);
                break;
                /* Disassociation */
            case ZM_WLAN_FRAME_TYPE_DISASOC :
                zm_debug_msg0("Disassociation received");
                zfStaProcessDisasoc(dev, buf);
                break;
ORMAL_MAJOR);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ag = 1;
                }
            }
            if (sendFlag == 1)
            {
                /* Send probe response */
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_PROBERSP, src, i, 0, i);
            }
        }
    }
}

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfProcessProbeRsp           */
/*      Process probe response management frame.                        */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*      buf : auth frame buffer                                         */
/*  AddInfo : Rx Header and Rx Mac Status                               */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      none                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Aress Yang          ZyDAS Technology Corporation    2006.11     */
/*                                                                      */
/************************************************************************/
void zfProcessProbeRsp(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* AddInfo)
{
    /* Gather scan result */
    /* Parse TIM and send PS-POLL in power saving mode */
    struct zsWlanProbeRspFrameHeader*  pProbeRspHeader;
    struct zsBssInfo* pBssInfo;
    u8_t   pBuf[sizeof(struct zsWlanProbeRspFrameHeader)];
    int    res;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zfCopyFromRxBuffer(dev, buf, pBuf, 0,
                       sizeof(struct zsWlanProbeRspFrameHeader));
    pProbeRspHeader = (struct zsWlanProbeRspFrameHeader*) pBuf;

    zmw_enter_critical_section(dev);

    //zm_debug_msg1("bss count = ", wd->sta.bssList.bssCount);

    pBssInfo = zfStaFindBssInfo(dev, buf, pProbeRspHeader);

    //if ( i == wd->sta.bssList.bssCount )
    if ( pBssInfo == NULL )
    {
        /* Allocate a new entry if BSS not in the scan list */
        pBssInfo = zfBssInfoAllocate(dev);
        if (pBssInfo != NULL)
        {
            res = zfStaInitBssInfo(dev, buf, pProbeRspHeader, pBssInfo, AddInfo, 0);
            //zfDumpSSID(pBssInfo->ssid[1], &(pBssInfo->ssid[2]));
            if ( res != 0 )
            {
                zfBssInfoFree(dev, pBssInfo);
            }
            else
            {
                zfBssInfoInsertToList(dev, pBssInfo);
            }
        }
    }
    else
    {
        res = zfStaInitBssInfo(dev, buf, pProbeRspHeader, pBssInfo, AddInfo, 1);
        if (res == 2)
        {
            zfBssInfoRemoveFromList(dev, pBssInfo);
            zfBssInfoFree(dev, pBssInfo);
        }
        else if ( wd->wlanMode == ZM_MODE_IBSS )
        {
            int idx;

            // It would reset the alive counter if the peer station is found!
            zfStaFindFreeOpposite(dev, (u16_t *)pBssInfo->macaddr, &idx);
        }
    }

    zmw_leave_critical_section(dev);

    return;
}

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfSendProbeReq              */
/*      Send probe request management frame.                            */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*                                                                      */
/*                                                                      */
/*    OUTPUTS                            1[32];  /* Color selection plane */
	u8 ccr;	/* Cursor Control Reg */
	u8 xxx0[3];
	u16 cursx;	/* Cursor x,y position */
	u16 cursy;	/* Cursor x,y position */
	u32 color0;
	u32 color1;
	u32 xxx1[0x1bc];
	u32 cpl0i[32];	/* Enable plane 0 autoinc */
	u32 cpl1i[32]; /* Color selection autoinc */
};

struct cg14_dac {
	u8 addr;	/* Address Register */
	u8 xxx0[255];
	u8 glut;	/* Gamma table */
	u8 xxx1[255];
	u8 select;	/* Register Select */
	u8 xxx2[255];
	u8 mode;	/* Mode Register */
};

struct cg14_xlut{
	u8 x_xlut [256];
	u8 x_xlutd [256];
	u8 xxx0[0x600];
	u8 x_xlut_inc [256];
	u8 x_xlutd_inc [256];
};

/* Color look up table (clut) */
/* Each one of these arrays hold the color lookup table (for 256
 * colors) for each MDI page (I assume then there should be 4 MDI
 * pages, I still wonder what they are.  I have seen NeXTStep split
 * the screen in four parts, while operating in 24 bits mode.  Each
 * integer holds 4 values: alpha value (transparency channel, thanks
 * go to John Stone (johns@umr.edu) from OpenBSD), red, green and blue
 *
 * I currently use the clut instead of the Xlut
 */
struct cg14_clut {
	u32 c_clut [256];
	u32 c_clutd [256];    /* i wonder what the 'd' is for */
	u32 c_clut_inc [256];
	u32 c_clutd_inc [256];
};

#define CG14_MMAP_ENTRIES	16

struct cg14_par {
	spinlock_t		lock;
	struct cg14_regs	__iomem *regs;
	struct cg14_clut	__iomem *clut;
	struct cg14_cursor	__iomem *cursor;

	u32			flags;
#define CG14_FLAG_BLANKED	0x00000001

	unsigned long		iospace;

	struct sbus_mmap_map	mmap_map[CG14_MMAP_ENTRIES];

	int			mode;
	int			ramsize;
};

static void __cg14_reset(struct cg14_par *par)
{
	struct cg14_regs __iomem *regs = par->regs;
	u8 val;

	val = sbus_readb(&regs->mcr);
	val &= ~(CG14_MCR_PIXMODE_MASK);
	sbus_writeb(val, &regs->mcr);
}

static int cg14_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct cg14_par *par = (struct cg14_par *) info->par;
	unsigned long flags;

	/* We just use this to catch switches out of
	 * graphics mode.
	 */
	spin_lock_irqsave(&par->lock, flags);
	__cg14_reset(par);
	spin_unlock_irqrestore(&par->lock, flags);

	if (var->xoffset || var->yoffset || var->vmode)
		return -EINVAL;
	return 0;
}

/**
 *      cg14_setcolreg - Optional function. Sets a color register.
 *      @regno: boolean, 0 copy local, 1 get_user() function
 *      @red: frame buffer colormap structure
 *      @green: The green value which can be up to 16 bits wide
 *      @blue:  The blue value which can be up to 16 bits wide.
 *      @transp: If supported the alpha value which can be up to 16 bits wide.
 *      @info: frame buffer info structure
 */
static int cg14_setcolreg(unsigned regno,
			  unsigned red, unsigned green, unsigned blue,
			  unsigned transp, struct fb_info *info)
{
	struct cg14_par *par = (struct cg14_par *) info->par;
	struct cg14_clut __iomem *clut = par->clut;
	unsigned long flags;
	u32 val;

	if (regno >= 256)
		return 1;

	red >>= 8;
	green >>= 8;
	blue >>= 8;
	val = (red | (green << 8) | (blue << 16));

	spin_lock_irqsave(&par->lock, flags);
	sbus_writel(val, &clut->c_clut[regno]);
	spin_unlock_irqrestore(&par->lock, flags);

	return 0;
}

static int cg14_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	struct cg14_par *par = (struct cg14_par *) info->par;

	return sbusfb_mmap_helper(par->mmap_map,
				  info->fix.smem_start, info->fix.smem_len,
				  par->iospace, vma);
}

static int cg14_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	struct cg14_par *par = (struct cg14_par *) info->par;
	struct cg14_regs __iomem *regs = par->regs;
	struct mdi_cfginfo kmdi, __user *mdii;
	unsigned long flags;
	int cur_mode, mode, ret = 0;

	switch (cmd) {
	case MDI_RESET:
		spin_lock_irqsave(&par->lock, flags);
		__cg14_reset(par);
		spin_unlock_irqrestore(&par->lock, flags);
		break;

	case MDI_GET_CFGINFO:
		memset(&kmdi, 0, sizeof(kmdi));

		spin_lock_irqsave(&par->lock, flags);
		kmdi.mdi_type = FBTYPE_MDICOLOR;
		kmdi.mdi_height = info->var.yres;
		kmdi.mdi_width = info->var.xres;
		kmdi.mdi_mode = par->mode;
		kmdi.mdi_pixfreq = 72; /] = 1023;
    aifs[0] = 3 * 9 + 10;
    txop[0] = 0;
    /* Back Ground */
    cwmin[1] = 15;
    cwmax[1] = 1023;
    aifs[1] = 7 * 9 + 10;
    txop[1] = 0;
    /* VIDEO */
    cwmin[2] = 7;
    cwmax[2] = 15;
    aifs[2] = 2 * 9 + 10;
    txop[2] = 94;
    /* VOICE */
    cwmin[3] = 3;
    cwmax[3] = 7;
    aifs[3] = 2 * 9 + 10;
    txop[3] = 47;
    /* Special TxQ */
    cwmin[4] = 3;
    cwmax[4] = 7;
    aifs[4] = 2 * 9 + 10;
    txop[4] = 0;

    /* WMM parameter for AP */
    if (mode == 1)
    {
        cwmax[0] = 63;
        aifs[3] = 1 * 9 + 10;
        aifs[4] = 1 * 9 + 10;
    }
    zfHpUpdateQosParameter(dev, cwmin, cwmax, aifs, txop);
}

u16_t zfFindATHExtCap(zdev_t* dev, zbuf_t* buf, u8_t type, u8_t subtype)
{
    u8_t subType;
    u16_t offset;
    u16_t bufLen;
    u16_t elen;
    u8_t id;
    u8_t tmp;

    /* Get offset of first element */
    subType = (zmw_rx_buf_readb(dev, buf, 0) >> 4);

    if ((offset = zgElementOffsetTable[subType]) == 0xff)
    {
        zm_assert(0);
    }

    /* Plus wlan header */
    offset += 24;

    bufLen = zfwBufGetSize(dev, buf);

    /* Search loop */
    while ((offset+2)<bufLen)                   // including element ID and length (2bytes)
    {
        /* Search target element */
        if ((id = zmw_rx_buf_readb(dev, buf, offset)) == ZM_WLAN_EID_WIFI_IE)
        {
            /* Bingo */
            if ((elen = zmw_rx_buf_readb(dev, buf, offset+1))>(bufLen - offset))
            {
                /* Element length error */
                return 0xffff;
            }

            if ( elen == 0 )
            {
                return 0xffff;
            }

            if (((tmp = zmw_rx_buf_readb(dev, buf, offset+2)) == 0x00)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+3)) == 0x03)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+4)) == 0x7f)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+5)) == type))

            {
                if ( subtype != 0xff )
                {
                    if ( (tmp = zmw_rx_buf_readb(dev, buf, offset+6)) == subtype  )
                    {
                        return offset;
                    }
                }
                else
                {
                    return offset;
                }
            }
        }

        /* Advance to next element */
        if ((elen = zmw_rx_buf_readb(dev, buf, offset+1)) == 0)
        {
            return 0xffff;
        }
        offset += (elen+2);
    }
    return 0xffff;
}

u16_t zfFindBrdcmMrvlRlnkExtCap(zdev_t* dev, zbuf_t* buf)
{
    u8_t subType;
    u16_t offset;
    u16_t bufLen;
    u16_t elen;
    u8_t id;
    u8_t tmp;

    /* Get offset of first element */
    subType = (zmw_rx_buf_readb(dev, buf, 0) >> 4);

    if ((offset = zgElementOffsetTable[subType]) == 0xff)
    {
        zm_assert(0);
    }

    /* Plus wlan header */
    offset += 24;

    bufLen = zfwBufGetSize(dev, buf);

    /* Search loop */
    while ((offset+2)<bufLen)                   // including element ID and length (2bytes)
    {
        /* Search target element */
        if ((id = zmw_rx_buf_readb(dev, buf, offset)) == ZM_WLAN_EID_WIFI_IE)
        {
            /* Bingo */
            if ((elen = zmw_rx_buf_readb(dev, buf, offset+1))>(bufLen - offset))
            {
                /* Element length error */
                return 0xffff;
            }

            if ( elen == 0 )
            {
                return 0xffff;
            }

            if (((tmp = zmw_rx_buf_readb(dev, buf, offset+2)) == 0x00)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+3)) == 0x10)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+4)) == 0x18))

            {
                return offset;
            }
            else if (((tmp = zmw_rx_buf_readb(dev, buf, offset+2)) == 0x00)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+3)) == 0x50)
                    && ((tmp = zmw_rx_buf_readb(dev, buf, offset+4)) == 0x43))

            {
                return offset;
            }
        }
        ecg14 regs");
	par->clut = of_ioremap(&op->resource[0], CG14_CLUT1,
			       sizeof(struct cg14_clut), "cg14 clut");
	par->cursor = of_ioremap(&op->resource[0], CG14_CURSORREGS,
				 sizeof(struct cg14_cursor), "cg14 cursor");

	info->screen_base = of_ioremap(&op->resource[1], 0,
				       info->fix.smem_len, "cg14 ram");

	if (!par->regs || !par->clut || !par->cursor || !info->screen_base)
		goto out_unmap_regs;

	is_8mb = (((op->resource[1].end - op->resource[1].start) + 1) ==
		  (8 * 1024 * 1024));

	BUILD_BUG_ON(sizeof(par->mmap_map) != sizeof(__cg14_mmap_map));
		
	memcpy(&par->mmap_map, &__cg14_mmap_map, sizeof(par->mmap_map));

	for (i = 0; i < CG14_MMAP_ENTRIES; i++) {
		struct sbus_mmap_map *map = &par->mmap_map[i];

		if (!map->size)
			break;
		if (map->poff & 0x80000000)
			map->poff = (map->poff & 0x7fffffff) +
				(op->resource[0].start -
				 op->resource[1].start);
		if (is_8mb &&
		    map->size >= 0x100000 &&
		    map->size <= 0x400000)
			map->size *= 2;
	}

	par->mode = MDI_8_PIX;
	par->ramsize = (is_8mb ? 0x800000 : 0x400000);

	info->flags = FBINFO_DEFAULT | FBINFO_HWACCEL_YPAN;
	info->fbops = &cg14_ops;

	__cg14_reset(par);

	if (fb_alloc_cmap(&info->cmap, 256, 0))
		goto out_unmap_regs;

	fb_set_cmap(&info->cmap, info);

	cg14_init_fix(info, linebytes, dp);

	err = register_framebuffer(info);
	if (err < 0)
		goto out_dealloc_cmap;

	dev_set_drvdata(&op->dev, info);

	printk(KERN_INFO "%s: cgfourteen at %lx:%lx, %dMB\n",
	       dp->full_name,
	       par->iospace, info->fix.smem_start,
	       par->ramsize >> 20);

	return 0;

out_dealloc_cmap:
	fb_dealloc_cmap(&info->cmap);

out_unmap_regs:
	cg14_unmap_regs(op, info, par);

out_err:
	return err;
}

static int __devexit cg14_remove(struct of_device *op)
{
	struct fb_info *info = dev_get_drvdata(&op->dev);
	struct cg14_par *par = info->par;

	unregister_framebuffer(info);
	fb_dealloc_cmap(&info->cmap);

	cg14_unmap_regs(op, info, par);

	framebuffer_release(info);

	dev_set_drvdata(&op->dev, NULL);

	return 0;
}

static const struct of_device_id cg14_match[] = {
	{
		.name = "cgfourteen",
	},
	{},
};
MODULE_DEVICE_TABLE(of, cg14_match);

static struct of_platform_driver cg14_driver = {
	.name		= "cg14",
	.match_table	= cg14_match,
	.probe		= cg14_probe,
	.remove		= __devexit_p(cg14_remove),
};

static int __init cg14_init(void)
{
	if (fb_get_options("cg14fb", NULL))
		return -ENODEV;

	return of_register_driver(&cg14_driver, &of_bus_type);
}

static void __exit cg14_exit(void)
{
	of_unregister_driver(&cg14_driver);
}

module_init(cg14_init);
module_exit(cg14_exit);

MODULE_DESCRIPTION("framebuffer driver for CGfourteen chipsets");
MODULE_AUTHOR("David S. Miller <davem@davemloft.net>");
MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            INDX( 	                 (   ®   Ë      õ c e –c   –          H     p `     ÔG     ä!„<é–k¸'4
£Œﬁ¿!„<é–k\nkìç–        ∏               w h c i . c . s v n - b a s e               p `     ÔG     ä!„<é–k¸'4
£Œﬁ¿!„<é–k\nkìç–        ∏               w h c i . c . s v n - b a s e               ÔG     ä!„<é–k¸'4
£Œﬁ¿!„<é–k\nkìç–        ∏               w h c i . c . s v n - b a s e               H     p `     ÔG     ä!„<é–k¸'4
£Œﬁ¿!„<é–k\nkìç–        ∏               w h õ i . c . s v n - b a s e               H     p `     ÔG     ä!„<é–k¸'4
£Œﬁ¿!„<é–k\nkìç–        ∏               w h c i . c . s v n - b a s e               p `     ÔG     ä!„<é–k¸'4
£Œﬁ¿!„<é–k\nkìç–        ∏               w h c i . c . s v n - b a s e               p `     ÔG     ä!„<é–k¸'4
£Œﬁ¿!„