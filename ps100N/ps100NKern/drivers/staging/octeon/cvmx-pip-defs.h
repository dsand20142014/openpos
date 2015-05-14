/***********************license start***************
 * Author: Cavium Networks
 *
 * Contact: support@caviumnetworks.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium Networks for more information
 ***********************license end**************************************/

#ifndef __CVMX_PIP_DEFS_H__
#define __CVMX_PIP_DEFS_H__

/*
 * Enumeration representing the amount of packet processing
 * and validation performed by the input hardware.
 */
enum cvmx_pip_port_parse_mode {
	/*
	 * Packet input doesn't perform any processing of the input
	 * packet.
	 */
	CVMX_PIP_PORT_CFG_MODE_NONE = 0ull,
	/*
	 * Full packet processing is performed with pointer starting
	 * at the L2 (ethernet MAC) header.
	 */
	CVMX_PIP_PORT_CFG_MODE_SKIPL2 = 1ull,
	/*
	 * Input packets are assumed to be IP.  Results from non IP
	 * packets is undefined. Pointers reference the beginning of
	 * the IP header.
	 */
	CVMX_PIP_PORT_CFG_MODE_SKIPIP = 2ull
};

#define CVMX_PIP_BCK_PRS \
	 CVMX_ADD_IO_SEG(0x00011800A0000038ull)
#define CVMX_PIP_BIST_STATUS \
	 CVMX_ADD_IO_SEG(0x00011800A0000000ull)
#define CVMX_PIP_CRC_CTLX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000040ull + (((offset) & 1) * 8))
#define CVMX_PIP_CRC_IVX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000050ull + (((offset) & 1) * 8))
#define CVMX_PIP_DEC_IPSECX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000080ull + (((offset) & 3) * 8))
#define CVMX_PIP_DSA_SRC_GRP \
	 CVMX_ADD_IO_SEG(0x00011800A0000190ull)
#define CVMX_PIP_DSA_VID_GRP \
	 CVMX_ADD_IO_SEG(0x00011800A0000198ull)
#define CVMX_PIP_FRM_LEN_CHKX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000180ull + (((offset) & 1) * 8))
#define CVMX_PIP_GBL_CFG \
	 CVMX_ADD_IO_SEG(0x00011800A0000028ull)
#define CVMX_PIP_GBL_CTL \
	 CVMX_ADD_IO_SEG(0x00011800A0000020ull)
#define CVMX_PIP_HG_PRI_QOS \
	 CVMX_ADD_IO_SEG(0x00011800A00001A0ull)
#define CVMX_PIP_INT_EN \
	 CVMX_ADD_IO_SEG(0x00011800A0000010ull)
#define CVMX_PIP_INT_REG \
	 CVMX_ADD_IO_SEG(0x00011800A0000008ull)
#define CVMX_PIP_IP_OFFSET \
	 CVMX_ADD_IO_SEG(0x00011800A0000060ull)
#define CVMX_PIP_PRT_CFGX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000200ull + (((offset) & 63) * 8))
#define CVMX_PIP_PRT_TAGX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000400ull + (((offset) & 63) * 8))
#define CVMX_PIP_QOS_DIFFX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000600ull + (((offset) & 63) * 8))
#define CVMX_PIP_QOS_VLANX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A00000C0ull + (((offset) & 7) * 8))
#define CVMX_PIP_QOS_WATCHX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000100ull + (((offset) & 7) * 8))
#define CVMX_PIP_RAW_WORD \
	 CVMX_ADD_IO_SEG(0x00011800A00000B0ull)
#define CVMX_PIP_SFT_RST \
	 CVMX_ADD_IO_SEG(0x00011800A0000030ull)
#define CVMX_PIP_STAT0_PRTX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000800ull + (((offset) & 63) * 80))
#define CVMX_PIP_STAT1_PRTX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000808ull + (((offset) & 63) * 80))
#define CVMX_PIP_STAT2_PRTX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000810ull + (((offset) & 63) * 80))
#define CVMX_PIP_STAT3_PRTX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000818ull + (((offset) & 63) * 80))
#define CVMX_PIP_STAT4_PRTX(offset) \
	 CVMX_ADD_IO_SEG(0x00011800A0000820ull + (((offset) & 63) * 80))
#define CVMX_PIP_STAT5_PRTX(offs 0x3f)
		set_bit(IS_INVREV, &dev->flags);
	else
		clear_bit(IS_INVREV, &dev->flags);
	ix = 1;
	ifno = 1;
	ch = dev->atr[1];
	dev->proto = 0;		/* XXX PROTO */
	any_t1 = any_t0 = done = 0;
	dev->ta1 = 0x11;	/* defaults to 9600 baud */
	do {
		if (ifno == 1 && (ch & 0x10)) {
			/* read first interface byte and TA1 is present */
			dev->ta1 = dev->atr[2];
			DEBUGP(5, dev, "Card says FiDi is 0x%.2x\n", dev->ta1);
			ifno++;
		} else if ((ifno == 2) && (ch & 0x10)) { /* TA(2) */
			dev->ta1 = 0x11;
			ifno++;
		}

		DEBUGP(5, dev, "Yi=%.2x\n", ch & 0xf0);
		ix += ((ch & 0x10) >> 4)	/* no of int.face chars */
		    +((ch & 0x20) >> 5)
		    + ((ch & 0x40) >> 6)
		    + ((ch & 0x80) >> 7);
		/* ATRLENCK(dev,ix); */
		if (ch & 0x80) {	/* TDi */
			ch = dev->atr[ix];
			if ((ch & 0x0f)) {
				any_t1 = 1;
				DEBUGP(5, dev, "card is capable of T=1\n");
			} else {
				any_t0 = 1;
				DEBUGP(5, dev, "card is capable of T=0\n");
			}
		} else
			done = 1;
	} while (!done);

	DEBUGP(5, dev, "ix=%d noHist=%d any_t1=%d\n",
	      ix, dev->atr[1] & 15, any_t1);
	if (ix + 1 + (dev->atr[1] & 0x0f) + any_t1 != dev->atr_len) {
		DEBUGP(5, dev, "length error\n");
		return 0;
	}
	if (any_t0)
		set_bit(IS_ANY_T0, &dev->flags);

	if (any_t1) {		/* compute csum */
		dev->atr_csum = 0;
#ifdef ATR_CSUM
		for (i = 1; i < dev->atr_len; i++)
			dev->atr_csum ^= dev->atr[i];
		if (dev->atr_csum) {
			set_bit(IS_BAD_CSUM, &dev->flags);
			DEBUGP(5, dev, "bad checksum\n");
			goto return_0;
		}
#endif
		if (any_t0 == 0)
			dev->proto = 1;	/* XXX PROTO */
		set_bit(IS_ANY_T1, &dev->flags);
	}

	return 1;
}

struct card_fixup {
	char atr[12];
	u_int8_t atr_len;
	u_int8_t stopbits;
};

static struct card_fixup card_fixups[] = {
	{	/* ACOS */
		.atr = { 0x3b, 0xb3, 0x11, 0x00, 0x00, 0x41, 0x01 },
		.atr_len = 7,
		.stopbits = 0x03,
	},
	{	/* Motorola */
		.atr = {0x3b, 0x76, 0x13, 0x00, 0x00, 0x80, 0x62, 0x07,
			0x41, 0x81, 0x81 },
		.atr_len = 11,
		.stopbits = 0x04,
	},
};

static void set_cardparameter(struct cm4000_dev *dev)
{
	int i;
	unsigned int iobase = dev->p_dev->io.BasePort1;
	u_int8_t stopbits = 0x02; /* ISO default */

	DEBUGP(3, dev, "-> set_cardparameter\n");

	dev->flags1 = dev->flags1 | (((dev->baudv - 1) & 0x0100) >> 8);
	xoutb(dev->flags1, REG_FLAGS1(iobase));
	DEBUGP(5, dev, "flags1 = 0x%02x\n", dev->flags1);

	/* set baudrate */
	xoutb((unsigned char)((dev->baudv - 1) & 0xFF), REG_BAUDRATE(iobase));

	DEBUGP(5, dev, "baudv = %i -> write 0x%02x\n", dev->baudv,
	      ((dev->baudv - 1) & 0xFF));

	/* set stopbits */
	for (i = 0; i < ARRAY_SIZE(card_fixups); i++) {
		if (!memcmp(dev->atr, card_fixups[i].atr,
			    card_fixups[i].atr_len))
			stopbits = card_fixups[i].stopbits;
	}
	xoutb(stopbits, REG_STOPBITS(iobase));

	DEBUGP(3, dev, "<- set_cardparameter\n");
}

static int set_protocol(struct cm4000_dev *dev, struct ptsreq *ptsreq)
{

	unsigned long tmp, i;
	unsigned short num_bytes_read;
	unsigned char pts_reply[4];
	ssize_t rc;
	unsigned int iobase = dev->p_dev->io.BasePort1;

	rc = 0;

	DEBUGP(3, dev, "-> set_protocol\n");
	DEBUGP(5, dev, "ptsreq->Protocol = 0x%.8x, ptsreq->Flags=0x%.8x, "
		 "ptsreq->pts1=0x%.2x, ptsreq->pts2=0x%.2x, "
		 "ptsreq->pts3=0x%.2x\n", (unsigned int)ptsreq->protocol,
		 (unsigned int)ptsreq->flags, ptsreq->pts1, ptsreq->pts2,
		 ptsreq->pts3);

	/* Fill PTS structure */
	dev->pts[0] = 0xff;
	dev->pts[1] = 0x00;
	tmp = ptsreq->protocol;
	while ((tmp = (tmp >> 1)) > 0)
		dev->pts[1]++;
	dev->proto = dev->pts[1];	/* Set new protocol */
	dev->pts[1] = (0x01 << 4) | (dev->pts[1]);

	/* Correct Fi/Di according to CM4000 Fi/Di table */
	DEBUGP(5, dev, "Ta(1) from ATR is 0x%.2x\n", dev->ta1);
	/* set Fi/Di according to ATR TA(1) */
	dev->pts[2] = fi_di_table[dev->ta1 & 0x0F][(dev->ta1 >> 4) & 0x0F];

	/* Calculate PCK character */
	dev->pts[3] = dev->pts[0] ^ dev->pts[1] ^ dev->pts[2];

	DEBUGP(5, dev, "pts0=%.2x, pts1=%.2x, pts2=%.2x, pts3=%.2x\n",
	       dev->pts[0], dev->pts[1], dev->pts[2], dev->pts[3]);

	/* check card convention */
	if (test_bit(IS_INVREV, &dev->flags))
		str_invert_revert(dev->pts, 4);

	/* reset SM */
	xoutb(0x80, REG_FLAGS0(iobase));

	/* Enable access to the message buffer */
	DEBUGP(5, dev, "Enable access to the messages buffer\n");
	dev->flags1 = 0x20	/* T_Active */
	    | (test_bit(IS_INVREV, &dev->flags) ? 0x02 : 0x00) /* inv parity */
	    | ((dev->baudv >> 8) & 0x01);	/* MSB-baud */
	xoutb(dev->flags1, REG_FLAGS1(iobase));

	DEBUGP(5, dev, "Enable message buffer -> flags1 = 0x%.2x\n",
	       dev->flags1);

	/* write challenge to the buffer */
	DEBUGP(5, dev, "Write challenge to buffer: ");
	for (i = 0; i < 4; i++) {
		xoutb(i, REG_BUF_ADDR(iobase));
		xoutb(dev->pts[i], REG_BUF_DATA(iobase));	/* buf data */
#ifdef PCMCIA_DEBUG
		if (pc_debug >= 5)
			printk("0x%.2x ", dev->pts[i]);
	}
	if (pc_debug >= 5)
		printk("\n");
#else
	}
#endif

	/* set number of bytes to write */
	DEBUGP(5, dev, "Set number of bytes to write\n");
	xoutb(0x04, REG_NUM_SEND(iobase));

	/* Trigger CARDMAN CONTROLLER */
	xoutb(0x50, REG_FLAGS0(iobase));

	/* Monitor progress */
	/* wait for xmit done */
	DEBUGP(5, dev, "Waiting for NumRecBytes getting valid\n");

	for (i = 0; i < 100; i++) {
		if (inb(REG_FLAGS0(iobase)) & 0x08) {
			DEBUGP(5, dev, "NumRecBytes is valid\n");
			break;
		}
		mdelay(10);
	}
	if (i == 100) {
		DEBUGP(5, dev, "Timeout waiting for NumRecBytes getting "
		       "valid\n");
		rc = -EIO;
		goto exit_setprotocol;
	}

	DEBUGP(5, dev, "Reading NumRecBytes\n");
	for (i = 0; i < 100; i++) {
		io_read_num_rec_bytes(iobase, &num_bytes_read);
		if (num_bytes_read >= 4) {
			DEBUGP(2, dev, "NumRecBytes = %i\n", num_bytes_read);
			break;
		}
		mdelay(10);
	}

	/* check whether it is a short PTS reply? */
	if (num_bytes_read == 3)
		i = 0;

	if (i == 100) {
		DEBUGP(5, dev, "Timeout reading num_bytes_read\n");
		rc = -EIO;
		goto exit_setprotocol;
	}

	DEBUGP(5, dev, "Reset the CARDMAN CONTROLLER\n");
	xoutb(0x80, REG_FLAGS0(iobase));

	/* Read PPS reply */
	DEBUGP(5, dev, "Read PPS reply\n");
	for (i = 0; i < num_bytes_read; i++) {
		xoutb(i, REG_BUF_ADDR(iobase));
		pts_reply[i] = inb(REG_BUF_DATA(iobase));
	}

#ifdef PCMCIA_DEBUG
	DEBUGP(2, dev, "PTSreply: ");
	for (i = 0; i < num_bytes_read; i++) {
		if (pc_debug >= 5)
			printk("0x%.2x ", pts_reply[i]);
	}
	printk("\n");
#endif	/* PCMCIA_DEBUG */

	DEBUGP(5, dev, "Clear Tactive in Flags1\n");
	xoutb(0x20, REG_FLAGS1(iobase));

	/* Compare ptsreq and ptsreply */
	if ((dev->pts[0] == pts_reply[0]) &&
	    (dev->pts[1] == pts_reply[1]) &&
	    (dev->pts[2] == pts_reply[2]) && (dev->pts[3] == pts_reply[3])) {
		/* setcardparameter according to PPS */
		dev->baudv = calc_baudv(dev->pts[2]);
		set_cardparameter(dev);
	} else if ((dev->pts[0] == pts_reply[0]) &&
		   ((dev->pts[1] & 0xef) == pts_reply[1]) &&
		   ((pts_reply[0] ^ pts_reply[1]) == pts_reply[2])) {
		/* short PTS reply, set card parameter to default values */
		dev->baudv = calc_baudv(0x11);
		set_cardparameter(dev);
	} else
		rc = -EIO;

exit_setprotocol:
	DEBUGP(3, dev, "<- set_protocol\n");
	return rc;
}

static int io_detect_cm4000(unsigned int iobase, struct cm4000_dev *dev)
{

	/* note: statemachine is assumed to be reset */
	if (inb(REG_FLAGS0(iobase)) & 8) {
		clear_bit(IS_ATR_VALID, &dev->flags);
		set_bit(IS_CMM_ABSENT, &dev->flags);
		return 0;	/* detect CMM = 1 -> failure */
	}
	/* xoutb(0x40, REG_FLAGS1(iobase)); detectCMM */
	xoutb(dev->flags1 | 0x40, REG_FLAGS1(iobase));
	if ((inb(REG_FLAGS0(iobase)) & 8) == 0) {
		clear_bit(IS_ATR_VALID, &dev->flags);
		set_bit(IS_CMM_ABSENT, &dev->flags);
		return 0;	/* detect CMM=0 -> failure */
	}
	/* clear detectCMM again by restoring original flags1 */
	xoutb(dev->flags1, REG_FLAGS1(iobase));
	return 1;
}

static void terminate_monitor(struct cm4000_dev *dev)
{

	/* tell the monitor to stop and wait until
	 * it terminates.
	 */
	DEBUGP(3, dev, "-> terminate_monitor\n");
	wait_event_interruptible(dev->devq,
				 test_and_set_bit(LOCK_MONITOR,
						  (void *)&dev->flags));

	/* now, LOCK_MONITOR has been set.
	 * allow a last cycle in the monitor.
	 * the monitor will indicate that it has
	 * finished by clearing this bit.
	 */
	DEBUGP(5, dev, "Now allow last cycle of monitor!\n");
	while (test_bit(LOCK_MONITOR, (void *)&dev->flags))
		msleep(25);

	DEBUGP(5, dev, "Delete timer\n");
	del_timer_sync(&dev->timer);
#ifdef PCMCIA_DEBUG
	dev->monitor_running = 0;
#endif

	DEBUGP(3, dev, "<- terminate_monitor\n");
}

/*
 * monitor the card every 50msec. as a side-effect, retrieve the
 * atr once a card is inserted. another side-effect of retrieving the
 * atr is that the card will be powered on, so there is no need to
 * power on the card explictely from the application: the driver
 * is already doing that for you.
 */

static void monitor_card(unsigned long p)
{
	struct cm4000_dev *dev = (struct cm4000_dev *) p;
	unsigned int iobase = dev->p_dev->io.BasePort1;
	unsigned short s;
	struct ptsreq ptsreq;
	int i, atrc;

	DEBUGP(7, dev, "->  monitor_card\n");

	/* if someone has set the lock for us: we're done! */
	if (test_and_set_bit(LOCK_MONITOR, &dev->flags)) {
		DEBUGP(4, dev, "About to stop monitor\n");
		/* no */
		dev->rlen =
		    dev->rpos =
		    dev->atr_csum = dev->atr_len_retry = dev->cwarn = 0;
		dev->mstate = M_FETCH_ATR;
		clear_bit(LOCK_MONITOR, &dev->flags);
		/* close et al. are sleeping on devq, so wake it */
		wake_up_interruptible(&dev->devq);
		DEBUGP(2, dev, "<- monitor_card (we are done now)\n");
		return;
	}

	/* try to lock io: if it is already locked, just add another timer */
	if (test_and_set_bit(LOCK_IO, (void *)&dev->flags)) {
		DEBUGP(4, dev, "Couldn't get IO lock\n");
		goto return_with_timer;
	}

	/* is a card/a reader inserted at all ? */
	dev->flags0 = xinb(REG_FLAGS0(iobase));
	DEBUGP(7, dev, "dev->flags0 = 0x%2x\n", dev->flags0);
	DEBUGP(7, dev, "smartcard present: %s\n",
	       dev->flags0 & 1 ? "yes" : "no");
	DEBUGP(7, dev, "cardman present: %s\n",
	       dev->flags0 == 0xff ? "no" : "yes");

	if ((dev->flags0 & 1) == 0	/* no smartcard inserted */
	    || dev->flags0 == 0xff) {	/* no cardman inserted */
		/* no */
		dev->rlen =
		    dev->rpos =
		    dev->atr_csum = dev->atr_len_retry = dev->cwarn = 0;
		dev->mstate = M_FETCH_ATR;

		dev->flags &= 0x000000ff; /* only keep IO and MONITOR locks */

		if (dev->flags0 == 0xff) {
			DEBUGP(4, dev, "set IS_CMM_ABSENT bit\n");
			set_bit(IS_CMM_ABSENT, &dev->flags);
		} else if (test_bit(IS_CMM_ABSENT, &dev->flags)) {
			DEBUGP(4, dev, "clear IS_CMM_ABSENT bit "
			       "(card is removed)\n");
			clear_bit(IS_CMM_ABSENT, &dev->flags);
		}

		goto release_io;
	} else if ((dev->flags0 & 1) && test_bit(IS_CMM_ABSENT, &dev->flags)) {
		/* cardman and card present but cardman was absent before
		 * (after suspend with inserted card) */
		DEBUGP(4, dev, "clear IS_CMM_ABSENT bit (card is inserted)\n");
		clear_bit(IS_CMM_ABSENT, &dev->flags);
	}

	if (test_bit(IS_ATR_VALID, &dev->flags) == 1) {
		DEBUGP(7, dev, "believe ATR is already valid (do nothing)\n");
		goto release_io;
	}

	switch (dev->mstate) {
		unsigned char flags0;
	case M_CARDOFF:
		DEBUGP(4, dev, "M_CARDOFF\n");
		flags0 = inb(REG_FLAGS0(iobase));
		if (flags0 & 0x02) {
			/* wait until Flags0 indicate power is off */
			dev->mdelay = T_10MSEC;
		} else {
			/* Flags0 indicate power off and no card inserted now;
			 * Reset CARDMAN CONTROLLER */
			xoutb(0x80, REG_FLAGS0(iobase));

			/* prepare for fetching ATR again: after card off ATR
			 * is read again automatically */
			dev->rlen =
			    dev->rpos =
			    dev->atr_csum =
			    dev->atr_len_retry = dev->cwarn = 0;
			dev->mstate = M_FETCH_ATR;

			/* minimal gap between CARDOFF and read ATR is 50msec */
			dev->mdelay = T_50MSEC;
		}
		break;
	case M_FETCH_ATR:
		DEBUGP(4, dev, "M_FETCH_ATR\n");
		xoutb(0x80, REG_FLAGS0(iobase));
		DEBUGP(4, dev, "Reset BAUDV to 9600\n");
		dev->baudv = 0x173;	/* 9600 */
		xoutb(0x02, REG_STOPBITS(iobase));	/* stopbits=2 */
		xoutb(0x73, REG_BAUDRATE(iobase));	/* baud value */
		xoutb(0x21, REG_FLAGS1(iobase));	/* T_Active=1, baud
							   value */
		/* warm start vs. power on: */
		xoutb(dev->flags0 & 2 ? 0x46 : 0x44, REG_FLAGS0(iobase));
		dev->mdelay = T_40MSEC;
		dev->mstate = M_TIMEOUT_WAIT;
		break;
	case M_TIMEOUT_WAIT:
		DEBUGP(4, dev, "M_TIMEOUT_WAIT\n");
		/* numRecBytes */
		io_read_num_rec_bytes(iobase, &dev->atr_len);
		dev->mdelay = T_10MSEC;
		dev->mstate = M_READ_ATR_LEN;
		break;
	case M_READ_ATR_LEN:
		DEBUGP(4, dev, "M_READ_ATR_LEN\n");
		/* infinite loop possible, since there is no timeout */

#define	MAX_ATR_LEN_RETRY	100

		if (dev->atr_len == io_read_num_rec_bytes(iobase, &s)) {
			if (dev->atr_len_retry++ >= MAX_ATR_LEN_RETRY) {					/* + XX msec */
				dev->mdelay = T_10MSEC;
				dev->mstate = M_READ_ATR;
			}
		} else {
			dev->atr_len = s;
			dev->atr_len_retry = 0;	/* set new timeout */
		}

		DEBUGP(4, dev, "Current ATR_LEN = %i\n", dev->atr_len);
		break;
	case M_READ_ATR:
		DEBUGP(4, dev, "M_READ_ATR\n");
		xoutb(0x80, REG_FLAGS0(iobase));	/* reset SM */
		for (i = 0; i < dev->atr_len; i++) {
			xoutb(i, REG_BUF_ADDR(iobase));
			dev->atr[i] = inb(REG_BUF_DATA(iobase));
		}
		/* Deactivate T_Active flags */
		DEBUGP(4, dev, "Deactivate T_Active flags\n");
		dev->flags1 = 0x01;
		xoutb(dev->flags1, REG_FLAGS1(iobase));

		/* atr is present (which doesnt mean it's valid) */
		set_bit(IS_ATR_PRESENT, &dev->flags);
		if (dev->atr[0] == 0x03)
			str_invert_revert(dev->atr, dev->atr_len);
		atrc = parse_atr(dev);
		if (atrc == 0) {	/* atr invalid */
			dev->mdelay = 0;
			dev->mstate = M_BAD_CARD;
		} else {
			dev->mdelay = T_50MSEC;
			dev->mstate = M_ATR_PRESENT;
			set_bit(IS_ATR_VALID, &dev->flags);
		}

		if (test_bit(IS_ATR_VALID, &dev->flags) == 1) {
			DEBUGP(4, dev, "monitor_card: ATR valid\n");
 			/* if ta1 == 0x11, no PPS necessary (default values) */
			/* do not do PPS with multi protocol cards */
			if ((test_bit(IS_AUTOPPS_ACT, &dev->flags) == 0) &&
			    (dev->ta1 != 0x11) &&
			    !(test_bit(IS_ANY_T0, &dev->flags) &&
			    test_bit(IS_ANY_T1, &dev->flags))) {
				DEBUGP(4, dev, "Perform AUTOPPS\n");
				set_bit(IS_AUTOPPS_ACT, &dev->flags);
				ptsreq.protocol = ptsreq.protocol =
				    (0x01 << dev->proto);
				ptsreq.flags = 0x01;
				ptsreq.pts1 = 0x00;
				ptsreq.pts2 = 0x00;
				ptsreq.pts3 = 0x00;
				if (set_protocol(dev, &ptsreq) == 0) {
					DEBUGP(4, dev, "AUTOPPS ret SUCC\n");
					clear_bit(IS_AUTOPPS_ACT, &dev->flags);
					wake_up_interruptible(&dev->atrq);
				} else {
					DEBUGP(4, dev, "AUTOPPS failed: "
					       "repower using defaults\n");
					/* prepare for repowering  */
					clear_bit(IS_ATR_PRESENT, &dev->flags);
					clear_bit(IS_ATR_VALID, &dev->flags);
					dev->rlen =
					    dev->rpos =
					    dev->atr_csum =
					    dev->atr_len_retry = dev->cwarn = 0;
					dev->mstate = M_FETCH_ATR;

					dev->mdelay = T_50MSEC;
				}
			} else {
				/* for cards which use slightly different
				 * params (extra guard time) */
				set_cardparameter(dev);
				if (test_bit(IS_AUTOPPS_ACT, &dev->flags) == 1)
					DEBUGP(4, dev, "AUTOPPS already active "
					       "2nd try:use default values\n");
				if (dev->ta1 == 0x11)
					DEBUGP(4, dev, "No AUTOPPS necessary "
					       "TA(1)==0x11\n");
				if (test_bit(IS_ANY_T0, &dev->flags)
				    && test_bit(IS_ANY_T1, &dev->flags))
					DEBUGP(4, dev, "Do NOT perform AUTOPPS "
					       "with multiprotocol cards\n");
				clear_bit(IS_AUTOPPS_ACT, &dev->flags);
				wake_up_interruptible(&dev->atrq);
			}
		} else {
			DEBUGP(4, dev, "ATR invalid\n");
			wake_up_interruptible(&dev->atrq);
		}
		break;
	case M_BAD_CARD:
		DEBUGP(4, dev, "M_BAD_CARD\n");
		/* slow down warning, but prompt immediately after insertion */
		if (dev->cwarn == 0 || dev->cwarn == 10) {
			set_bit(IS_BAD_CARD, &dev->flags);
			printk(KERN_WARNING MODULE_NAME ": device %s: ",
			       dev->node.dev_name);
			if (test_bit(IS_BAD_CSUM, &dev->flags)) {
				DEBUGP(4, dev, "ATR checksum (0x%.2x, should "
				       "be zero) failed\n", dev->atr_csum);
			}
#ifdef PCMCIA_DEBUG
			else if (test_bit(IS_BAD_LENGTH, &dev->flags)) {
				DEBUGP(4, dev, "ATR length error\n");
			} else {
				DEBUGP(4, dev, "card damaged or wrong way "
					"inserted\n");
			}
#endif
			dev->cwarn = 0;
			wake_up_interruptible(&dev->atrq);	/* wake open */
		}
		dev->cwarn++;
		dev->mdelay = T_100MSEC;
		dev->mstate = M_FETCH_ATR;
		break;
	default:
		DEBUGP(7, dev, "Unknown action\n");
		break;		/* nothing */
	}

release_io:
	DEBUGP(7, dev, "release_io\n");
	clear_bit(LOCK_IO, &dev->flags);
	wake_up_interruptible(&dev->ioq);	/* whoever needs IO */

return_with_timer:
	DEBUGP(7, dev, "<- monitor_card (returns with timer)\n");
	mod_timer(&dev->timer, jiffies + dev->mdelay);
	clear_bit(LOCK_MONITOR, &dev->flags);
}

/* Interface to userland (file_operations) */

static ssize_t cmm_read(struct file *filp, __user char *buf, size_t count,
			loff_t *ppos)
{
	struct cm4000_dev *dev = filp->private_data;
	unsigned int iobase = dev->p_dev->io.BasePort1;
	ssize_t rc;
	int i, j, k;

	DEBUGP(2, dev, "-> cmm_read(%s,%d)\n", current->comm, current->pid);

	if (count == 0)		/* according to manpage */
		return 0;

	if (!pcmcia_dev_present(dev->p_dev) || /* device removed */
	    test_bit(IS_CMM_ABSENT, &dev->flags))
		return -ENODEV;

	if (test_bit(IS_BAD_CSUM, &dev->flags))
		return -EIO;

	/* also see the note about this in cmm_write */
	if (wait_event_interruptible
	    (dev->atrq,
	     ((filp->f_flags & O_NONBLOCK)
	      || (test_bit(IS_ATR_PRESENT, (void *)&dev->flags) != 0)))) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		return -ERESTARTSYS;
	}

	if (test_bit(IS_ATR_VALID, &dev->flags) == 0)
		return -EIO;

	/* this one implements blocking IO */
	if (wait_event_interruptible
	    (dev->readq,
	     ((filp->f_flags & O_NONBLOCK) || (dev->rpos < dev->rlen)))) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		return -ERESTARTSYS;
	}

	/* lock io */
	if (wait_event_interruptible
	    (dev->ioq,
	     ((filp->f_flags & O_NONBLOCK)
	      || (test_and_set_bit(LOCK_IO, (void *)&dev->flags) == 0)))) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		return -ERESTARTSYS;
	}

	rc = 0;
	dev->flags0 = inb(REG_FLAGS0(iobase));
	if ((dev->flags0 & 1) == 0	/* no smartcard inserted */
	    || dev->flags0 == 0xff) {	/* no cardman inserted */
		clear_bit(IS_ATR_VALID, &dev->flags);
		if (dev->flags0 & 1) {
			set_bit(IS_CMM_ABSENT, &dev->flags);
			rc = -ENODEV;
		}
		rc = -EIO;
		goto release_io;
	}

	DEBUGP(4, dev, "begin read answer\n");
	j = min(count, (size_t)(dev->rlen - dev->rpos));
	k = dev->rpos;
	if (k + j > 255)
		j = 256 - k;
	DEBUGP(4, dev, "read1 j=%d\n", j);
	for (i = 0; i < j; i++) {
		xoutb(k++, REG_BUF_ADDR(iobase));
		dev->rbuf[i] = xinb(REG_BUF_DATA(iobase));
	}
	j = min(count, (size_t)(dev->rlen - dev->rpos));
	if (k + j > 255) {
		DEBUGP(4, dev, "read2 j=%d\n", j);
		dev->flags1 |= 0x10;	/* MSB buf addr set */
		xoutb(dev->flags1, REG_FLAGS1(iobase));
		for (; i < j; i++) {
			xoutb(k++, REG_BUF_ADDR(iobase));
			dev->rbuf[i] = xinb(REG_BUF_DATA(iobase));
		}
	}

	if (dev->proto == 0 && count > dev->rlen - dev->rpos) {
		DEBUGP(4, dev, "T=0 and count > buffer\n");
		dev->rbuf[i] = dev->rbuf[i - 1];
		dev->rbuf[i - 1] = dev->procbyte;
		j++;
	}
	count = j;

	dev->rpos = dev->rlen + 1;

	/* Clear T1Active */
	DEBUGP(4, dev, "Clear T1Active\n");
	dev->flags1 &= 0xdf;
	xoutb(dev->flags1, REG_FLAGS1(iobase));

	xoutb(0, REG_FLAGS1(iobase));	/* clear detectCMM */
	/* last check before exit */
	if (!io_detect_cm4000(iobase, dev))
		count = -ENODEV;

	if (test_bit(IS_INVREV, &dev->flags) && count > 0)
		str_invert_revert(dev->rbuf, count);

	if (copy_to_user(buf, dev->rbuf, count))
		return -EFAULT;

release_io:
	clear_bit(LOCK_IO, &dev->flags);
	wake_up_interruptible(&dev->ioq);

	DEBUGP(2, dev, "<- cmm_read returns: rc = %Zi\n",
	       (rc < 0 ? rc : count));
	return rc < 0 ? rc : count;
}

static ssize_t cmm_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	struct cm4000_dev *dev = (struct cm4000_dev *) filp->private_data;
	unsigned int iobase = dev->p_dev->io.BasePort1;
	unsigned short s;
	unsigned char tmp;
	unsigned char infolen;
	unsigned char sendT0;
	unsigned short nsend;
	unsigned short nr;
	ssize_t rc;
	int i;

	DEBUGP(2, dev, "-> cmm_write(%s,%d)\n", current->comm, current->pid);

	if (count == 0)		/* according to manpage */
		return 0;

	if (dev->proto == 0 && count < 4) {
		/* T0 must have at least 4 bytes */
		DEBUGP(4, dev, "T0 short write\n");
		return -EIO;
	}

	nr = count & 0x1ff;	/* max bytes to write */

	sendT0 = dev->proto ? 0 : nr > 5 ? 0x08 : 0;

	if (!pcmcia_dev_present(dev->p_dev) || /* device removed */
	    test_bit(IS_CMM_ABSENT, &dev->flags))
		return -ENODEV;

	if (test_bit(IS_BAD_CSUM, &dev->flags)) {
		DEBUGP(4, dev, "bad csum\n");
		return -EIO;
	}

	/*
	 * wait for atr to become valid.
	 * note: it is important to lock this code. if we dont, the monitor
	 * could be run between test_bit and the call to sleep on the
	 * atr-queue.  if *then* the monitor detects atr valid, it will wake up
	 * any process on the atr-queue, *but* since we have been interrupted,
	 * we do not yet sleep on this queue. this would result in a missed
	 * wake_up and the calling process would sleep forever (until
	 * interrupted).  also, do *not* restore_flags before sleep_on, because
	 * this could result in the same situation!
	 */
	if (wait_event_interruptible
	    (dev->atrq,
	     ((filp->f_flags & O_NONBLOCK)
	      || (test_bit(IS_ATR_PRESENT, (void *)&dev->flags) != 0)))) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		return -ERESTARTSYS;
	}

	if (test_bit(IS_ATR_VALID, &dev->flags) == 0) {	/* invalid atr */
		DEBUGP(4, dev, "invalid ATR\n");
		return -EIO;
	}

	/* lock io */
	if (wait_event_interruptible
	    (dev->ioq,
	     ((filp->f_flags & O_NONBLOCK)
	      || (test_and_set_bit(LOCK_IO, (void *)&dev->flags) == 0)))) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		return -ERESTARTSYS;
	}

	if (copy_from_user(dev->sbuf, buf, ((count > 512) ? 512 : count)))
		return -EFAULT;

	rc = 0;
	dev->flags0 = inb(REG_FLAGS0(iobase));
	if ((dev->flags0 & 1) == 0	/* no smartcard inserted */
	    || dev->flags0 == 0xff) {	/* no cardman inserted */
		clear_bit(IS_ATR_VALID, &dev->flags);
		if (dev->flags0 & 1) {
			set_bit(IS_CMM_ABSENT, &dev->flags);
			rc = -ENODEV;
		} else {
			DEBUGP(4, dev, "IO error\n");
			rc = -EIO;
		}
		goto release_io;
	}

	xoutb(0x80, REG_FLAGS0(iobase));	/* reset SM  */

	if (!io_detect_cm4000(iobase, dev)) {
		rc = -ENODEV;
		goto release_io;
	}

	/* reflect T=0 send/read mode in flags1 */
	dev->flags1 |= (sendT0);

	set_cardparameter(dev);

	/* dummy read, reset flag procedure received */
	tmp = inb(REG_FLAGS1(iobase));

	dev->flags1 = 0x20	/* T_Active */
	    | (sendT0)
	    | (test_bit(IS_INVREV, &dev->flags) ? 2 : 0)/* inverse parity  */
	    | (((dev->baudv - 1) & 0x0100) >> 8);	/* MSB-Baud */
	DEBUGP(1, dev, "set dev->flags1 = 0x%.2x\n", dev->flags1);
	xoutb(dev->flags1, REG_FLAGS1(iobase));

	/* xmit data */
	DEBUGP(4, dev, "Xmit data\n");
	for (i = 0; i < nr; i++) {
		if (i >= 256) {
			dev->flags1 = 0x20	/* T_Active */
			    | (sendT0)	/* SendT0 */
				/* inverse parity: */
			    | (test_bit(IS_INVREV, &dev->flags) ? 2 : 0)
			    | (((dev->baudv - 1) & 0x0100) >> 8) /* MSB-Baud */
			    | 0x10;	/* set address high */
			DEBUGP(4, dev, "dev->flags = 0x%.2x - set address "
			       "high\n", dev->flags1);
			xoutb(dev->flags1, REG_FLAGS1(iobase));
		}
		if (test_bit(IS_INVREV, &dev->flags)) {
			DEBUGP(4, dev, "Apply inverse convention for 0x%.2x "
				"-> 0x%.2x\n", (unsigned char)dev->sbuf[i],
			      invert_revert(dev->sbuf[i]));
			xoutb(i, REG_BUF_ADDR(iobase));
			xoutb(invert_revert(dev->sbuf[i]),
			      REG_BUF_DATA(iobase));
		} else {
			xoutb(i, REG_BUF_ADDR(iobase));
			xoutb(dev->sbuf[i], REG_BUF_DATA(iobase));
		}
	}
	DEBUGP(4, dev, "Xmit done\n");

	if (dev->proto == 0) {
		/* T=0 proto: 0 byte reply  */
		if (nr == 4) {
			DEBUGP(4, dev, "T=0 assumes 0 byte reply\n");
			xoutb(i, REG_BUF_ADDR(iobase));
			if (test_bit(IS_INVREV, &dev->flags))
				xoutb(0xff, REG_BUF_DATA(iobase));
			else
				xoutb(0x00, REG_BUF_DATA(iobase));
		}

		/* numSendBytes */
		if (sendT0)
			nsend = nr;
		else {
			if (nr == 4)
				nsend = 5;
			else {
				nsend = 5 + (unsigned char)dev->sbuf[4];
				if (dev->sbuf[4] == 0)
					nsend += 0x100;
			}
		}
	} else
		nsend = nr;

	/* T0: output procedure byte */
	if (test_bit(IS_INVREV, &dev->flags)) {
		DEBUGP(4, dev, "T=0 set Procedure byte (inverse-reverse) "
		       "0x%.2x\n", invert_revert(dev->sbuf[1]));
		xoutb(invert_revert(dev->sbuf[1]), REG_NUM_BYTES(iobase));
	} else {
		DEBUGP(4, dev, "T=0 set Procedure byte 0x%.2x\n", dev->sbuf[1]);
		xoutb(dev->sbuf[1], REG_NUM_BYTES(iobase));
	}

	DEBUGP(1, dev, "set NumSendBytes = 0x%.2x\n",
	       (unsigned char)(nsend & 0xff));
	xoutb((unsigned char)(nsend & 0xff), REG_NUM_SEND(iobase));

	DEBUGP(1, dev, "Trigger CARDMAN CONTROLLER (0x%.2x)\n",
	       0x40	/* SM_Active */
	      | (dev->flags0 & 2 ? 0 : 4)	/* power on if needed */
	      |(dev->proto ? 0x10 : 0x08)	/* T=1/T=0 */
	      |(nsend & 0x100) >> 8 /* MSB numSendBytes */ );
	xoutb(0x40		/* SM_Active */
	      | (dev->flags0 & 2 ? 0 : 4)	/* power on if needed */
	      |(dev->proto ? 0x10 : 0x08)	/* T=1/T=0 */
	      |(nsend & 0x100) >> 8,	/* MSB numSendBytes */
	      REG_FLAGS0(iobase));

	/* wait for xmit done */
	if (dev->proto == 1) {
		DEBUGP(4, dev, "Wait for xmit done\n");
		for (i = 0; i < 1000; i++) {
			if (inb(REG_FLAGS0(iobase)) & 0x08)
				break;
			msleep_interruptible(10);
		}
		if (i == 1000) {
			DEBUGP(4, dev, "timeout waiting for xmit done\n");
			rc = -EIO;
			goto release_io;
		}
	}

	/* T=1: wait for infoLen */

	infolen = 0;
	if (dev->proto) {
		/* wait until infoLen is valid */
		for (i = 0; i < 6000; i++) {	/* max waiting time of 1 min */
			io_read_num_rec_bytes(iobase, &s);
			if (s >= 3) {
				infolen = inb(REG_FLAGS1(iobase));
				DEBUGP(4, dev, "infolen=%d\n", infolen);
				break;
			}
			msleep_interruptible(10);
		}
		if (i == 6000) {
			DEBUGP(4, dev, "timeout waiting for infoLen\n");
			rc = -EIO;
			goto release_io;
		}
	} else
		clear_bit(IS_PROCBYTE_PRESENT, &dev->flags);

	/* numRecBytes | bit9 of numRecytes */
	io_read_num_rec_bytes(iobase, &dev->rlen);
	for (i = 0; i < 600; i++) {	/* max waiting time of 2 sec */
		if (dev->proto) {
			if (dev->rlen >= infolen + 4)
				break;
		}
		msleep_interruptible(10);
		/* numRecBytes | bit9 of numRecytes */
		io_read_num_rec_bytes(iobase, &s);
		if (s > dev->rlen) {
			DEBUGP(1, dev, "NumRecBytes inc (reset timeout)\n");
			i = 0;	/* reset timeout */
			dev->rlen = s;
		}
		/* T=0: we are done when numRecBytes doesn't
		 *      increment any more and NoProcedureByte
		 *      is set and numRecBytes == bytes sent + 6
		 *      (header bytes + data + 1 for sw2)
		 *      except when the card replies an error
		 *      which means, no data will be sent back.
		 */
		else if (dev->proto == 0) {
			if ((inb(REG_BUF_ADDR(iobase)) & 0x80)) {
				/* no procedure byte received since last read */
				DEBUGP(1, dev, "NoProcedure byte set\n");
				/* i=0; */
			} else {
				/* procedure byte received since last read */
				DEBUGP(1, dev, "NoProcedure byte unset "
					"(reset timeout)\n");
				dev->procbyte = inb(REG_FLAGS1(iobase));
				DEBUGP(1, dev, "Read procedure byte 0x%.2x\n",
				      dev->procbyte);
				i = 0;	/* resettimeout */
			}
			if (inb(REG_FLAGS0(iobase)) & 0x08) {
				DEBUGP(1, dev, "T0Done flag (read reply)\n");
				break;
			}
		}
		if (dev->proto)
			infolen = inb(REG_FLAGS1(iobase));
	}
	if (i == 600) {
		DEBUGP(1, dev, "timeout waiting for numRecBytes\n");
		rc = -EIO;
		goto release_io;
	} else {
		if (dev->proto == 0) {
			DEBUGP(1, dev, "Wait for T0Done bit to be  set\n");
			for (i = 0; i < 1000; i++) {
				if (inb(REG_FLAGS0(iobase)) & 0x08)
					break;
				msleep_interruptible(10);
			}
			if (i == 1000) {
				DEBUGP(1, dev, "timeout waiting for T0Done\n");
				rc = -EIO;
				goto release_io;
			}

			dev->procbyte = inb(REG_FLAGS1(iobase));
			DEBUGP(4, dev, "Read procedure byte 0x%.2x\n",
			      dev->procbyte);

			io_read_num_rec_bytes(iobase, &dev->rlen);
			DEBUGP(4, dev, "Read NumRecBytes = %i\n", dev->rlen);

		}
	}
	/* T=1: read offset=zero, T=0: read offset=after challenge */
	dev->rpos = dev->proto ? 0 : nr == 4 ? 5 : nr > dev->rlen ? 5 : nr;
	DEBUGP(4, dev, "dev->rlen = %i,  dev->rpos = %i, nr = %i\n",
	      dev->rlen, dev->rpos, nr);

release_io:
	DEBUGP(4, dev, "Reset SM\n");
	xoutb(0x80, REG_FLAGS0(iobase));	/* reset SM */

	if (rc < 0) {
		DEBUGP(4, dev, "Write failed but clear T_Active\n");
		dev->flags1 &= 0xdf;
		xoutb(dev->flags1, REG_FLAGS1(iobase));
	}

	clear_bit(LOCK_IO, &dev->flags);
	wake_up_interruptible(&dev->ioq);
	wake_up_interruptible(&dev->readq);	/* tell read we have data */

	/* ITSEC E2: clear write buffer */
	memset((char *)dev->sbuf, 0, 512);

	/* return error or actually written bytes */
	DEBUGP(2, dev, "<- cmm_write\n");
	return rc < 0 ? rc : nr;
}

static void start_monitor(struct cm4000_dev *dev)
{
	DEBUGP(3, dev, "-> start_monitor\n");
	if (!dev->monitor_running) {
		DEBUGP(5, dev, "create, init and add timer\n");
		setup_timer(&dev->timer, monitor_card, (unsigned long)dev);
		dev->monitor_running = 1;
		mod_timer(&dev->timer, jiffies);
	} else
		DEBUGP(5, dev, "monitor already running\n");
	DEBUGP(3, dev, "<- start_monitor\n");
}

static void stop_monitor(struct cm4000_dev *dev)
{
	DEBUGP(3, dev, "-> stop_monitor\n");
	if (dev->monitor_running) {
		DEBUGP(5, dev, "stopping monitor\n");
		terminate_monitor(dev);
		/* reset monitor SM */
		clear_bit(IS_ATR_VALID, &dev->flags);
		clear_bit(IS_ATR_PRESENT, &dev->flags);
	} else
		DEBUGP(5, dev, "monitor already stopped\n");
	DEBUGP(3, dev, "<- stop_monitor\n");
}

static long cmm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct cm4000_dev *dev = filp->private_data;
	unsigned int iobase = dev->p_dev->io.BasePort1;
	struct inode *inode = filp->f_path.dentry->d_inode;
	struct pcmcia_device *link;
	int size;
	int rc;
	void __user *argp = (void __user *)arg;
#ifdef PCMCIA_DEBUG
	char *ioctl_names[CM_IOC_MAXNR + 1] = {
		[_IOC_NR(CM_IOCGSTATUS)] "CM_IOCGSTATUS",
		[_IOC_NR(CM_IOCGATR)] "CM_IOCGATR",
		[_IOC_NR(CM_IOCARDOFF)] "CM_IOCARDOFF",
		[_IOC_NR(CM_IOCSPTS)] "CM_IOCSPTS",
		[_IOC_NR(CM_IOSDBGLVL)] "CM4000_DBGLVL",
	};
#endif
	DEBUGP(3, dev, "cmm_ioctl(device=%d.%d) %s\n", imajor(inode),
	       iminor(inode), ioctl_names[_IOC_NR(cmd)]);

	lock_kernel();
	rc = -ENODEV;
	link = dev_table[iminor(inode)];
	if (!pcmcia_dev_present(link)) {
		DEBUGP(4, dev, "DEV_OK false\n");
		goto out;
	}

	if (test_bit(IS_CMM_ABSENT, &dev->flags)) 