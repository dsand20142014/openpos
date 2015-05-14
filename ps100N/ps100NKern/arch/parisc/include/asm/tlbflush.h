/*
 *  Driver for NEC VR4100 series Serial Interface Unit.
 *
 *  Copyright (C) 2004-2008  Yoichi Yuasa <yuasa@linux-mips.org>
 *
 *  Based on drivers/serial/8250.c, by Russell King.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(CONFIG_SERIAL_VR41XX_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/console.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#include <asm/io.h>
#include <asm/vr41xx/siu.h>
#include <asm/vr41xx/vr41xx.h>

#define SIU_BAUD_BASE	1152000
#define SIU_MAJOR	204
#define SIU_MINOR_BASE	82

#define RX_MAX_COUNT	256
#define TX_MAX_COUNT	15

#define SIUIRSEL	0x08
 #define TMICMODE	0x20
 #define TMICTX		0x10
 #define IRMSEL		0x0c
 #define IRMSEL_HP	0x08
 #define IRMSEL_TEMIC	0x04
 #define IRMSEL_SHARP	0x00
 #define IRUSESEL	0x02
 #define SIRSEL		0x01

static struct uart_port siu_uart_ports[SIU_PORTS_MAX] = {
	[0 ... SIU_PORTS_MAX-1] = {
		.lock	= __SPIN_LOCK_UNLOCKED(siu_uart_ports->lock),
		.irq	= -1,
	},
};

#ifdef CONFIG_SERIAL_VR41XX_CONSOLE
static uint8_t lsr_break_flag[SIU_PORTS_MAX];
#endif

#define siu_read(port, offset)		readb((port)->membase + (offset))
#define siu_write(port, offset, value)	writeb((value), (port)->membase + (offset))

void vr41xx_select_siu_interface(siu_interface_t interface)
{
	struct uart_port *port;
	unsigned long flags;
	uint8_t irsel;

	port = &siu_uart_ports[0];

	sp