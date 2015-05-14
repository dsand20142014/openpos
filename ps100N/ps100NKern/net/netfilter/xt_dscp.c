/************************************************************************
 * Copyright 2003 Digi International (www.digi.com)
 *
 * Copyright (C) 2004 IBM Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 * Temple Place - Suite 330, Boston,
 * MA  02111-1307, USA.
 *
 * Contact Information:
 * Scott H Kilau <Scott_Kilau@digi.com>
 * Wendy Xiong   <wendyx@us.ibm.com>
 *
 ***********************************************************************/
#include <linux/delay.h>	/* For udelay */
#include <linux/serial_reg.h>	/* For the various UART offsets */
#include <linux/tty.h>
#include <linux/pci.h>
#include <asm/io.h>

#include "jsm.h"		/* Driver main header file */

static u32 jsm_offset_table[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

/*
 * This function allows calls to ensure that all outstanding
 * PCI writes have been completed, by doing a PCI read against
 * a non-destructive, read-only location on the Neo card.
 *
 * In this case, we are reading the DVID (Read-only Device Identification)
 * value of the Neo card.
 */
static inline void neo_pci_posting_flush(struct jsm_board *bd)
{
      readb(bd->re_map_membase + 0x8D);
}

static void neo_set_cts_flow_control(struct jsm_channel *ch)
{
	u8 ier, efr;
	ier = readb(&ch->ch_neo_uart->ier);
	efr = readb(&ch->ch_neo_uart->efr);

	jsm_printk(PARAM, INFO, &ch->ch_bd->pci_dev, "Setting CTSFLOW\n");

	/* Turn on auto CTS flow control */
	ier |= (UART_17158_IER_CTSDSR);
	efr |= (UART_17158_EFR_ECB | UART_17158_EFR_CTSDSR);

	/* Turn off auto Xon flow control */
	efr &= ~(UART_17158_EFR_IXON);

	/* Why? Becuz Exar's spec says we have to zero it out before setting it */
	writeb(0, &ch->ch_neo_uart->efr);

	/* Turn on UART enhanced bits */
	writeb(efr, &ch->ch_neo_uart->efr);

	/* Turn on table D, with 8 char hi/low watermarks */
	writeb((UART_17158_FCTR_TRGD | UART_17158_FCTR_RTS_4DELAY), &ch->ch_neo_uart->fctr);

	/* Feed the UART our trigger levels */
	writeb(8, &ch->ch_neo_uart->tfifo);
	ch->ch_t_tlevel = 8;

	writeb(ier, &ch->ch_neo_uart->ier);
}

static void neo_set_rts_flow_control(struct jsm_channel *ch)
{
	u8 ier, efr;
	ier = readb(&ch->ch_neo_uart->ier);
	efr = readb(&ch->ch_neo_uart->efr);

	jsm_printk(PARAM, INFO, &ch->ch_bd->pci_dev, "Setting RTSFLOW\n");

	/* Turn on auto RTS flow control */
	ier |= (UART_17158_IER_RTSDTR);
	efr |= (UART_17158_EFR_ECB | UART_17158_EFR_RTSDTR);

	/* Turn off auto Xoff flow control */
	ier &= ~(UART_17158_IER_XOFF);
	efr &= ~(UART_17158_EFR_IXOFF);

	/* Why? Becuz Exar's spec says we have to zero it out before setting it */
	writeb(0, &ch->ch_neo_uart->efr);

	/* Turn on UART enhanced bits */