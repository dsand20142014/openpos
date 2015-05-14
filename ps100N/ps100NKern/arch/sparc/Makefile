/* suncore.c
 *
 * Common SUN serial routines.  Based entirely
 * upon drivers/sbus/char/sunserial.c which is:
 *
 * Copyright (C) 1997  Eddie C. Dost  (ecd@skynet.be)
 *
 * Adaptation to new UART layer is:
 *
 * Copyright (C) 2002 David S. Miller (davem@redhat.com)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/serial_core.h>
#include <linux/init.h>

#include <asm/prom.h>

#include "suncore.h"

static int sunserial_current_minor = 64;

int sunserial_register_minors(struct uart_driver *drv, int count)
{
	int err = 0;

	drv->minor = sunserial_current_minor;
	drv->nr += count;
	/* Register the driver on the first call */
	if (drv->nr == count)
		err = uart_register_driver(drv);
	if (err == 0) {
		sunserial_current_minor += count;
		drv->tty_driver->name_base = drv->minor - 64;
	}
	return err;
}
EXPORT_SYMBOL(sunserial_register_minors);

void sunserial_unregister_minors(struct uart_driver *drv, int count)
{
	drv->nr -= count;
	sunserial_current_minor -= count;

	if (drv->nr == 0)
		uart_unregister_driver(drv);
}
EXPORT_SYMBOL(sunserial_unregister_minors);

int sunserial_console_match(struct console *con, struct device_node *dp,
			    struct uart_driver *drv, int line)
{
	int off;

	if (!con || of_console_device != dp)
		return 0;

	off = 0;
	if (of_console_options &&
	    *of_console_options == 'b')
		off = 1;

	if ((line & 1) != off)
		return 0;

	con->index = line;
	drv->cons = con;
	add_preferred_console(con->name, line, NULL);

	return 1;
}
EXPORT_SYMBOL(sunserial_console_match);

void
sunserial_console_termios(struct console *con)
{
	struct device_node *dp;
	const char *od, *mode, *s;
	char mode_prop[] = "ttyX-mode";
	int baud, bits, stop, cflag;
	char parity;

	dp = of_find_node_by_path("/options");
	od = of_get_property(dp, "output-device", NULL);
	if (!strcmp(od, "rsc")) {
		mode = of_get_property(of_console_device,
				       "ssp-console-modes", NULL);
		if (!mode)
			mode = "115200,8,n,1,-";
	} else {
		char c;

		c = 'a';
		if (of_console_options)
			c = *of_console_options;

		mode_prop[3] = c;

		mode = of_get_property(dp, mode_prop, NULL);
		if (!mode)
			mode = "9600,8,n,1,-";
	}

	cflag = CREAD | HUPCL | CLOCAL;

	s = mode;
	baud = simple_strtoul(s, NULL, 0);
	s = strchr(s, ',');
	bits = simple_strtoul(++s, NULL, 0);
	s = strchr(s, ',');
	parity = *(++s);
	s = strchr(s, ',');
	stop = simple_strtoul(++s, NULL, 0);
	s = strchr(s, ',');
	/* XXX handshake is not handled here. */

	switch (baud) {
		case 150: cflag |= B150; break;
		case 300: cflag |= B300; break;
		case 600: cflag |= B600; break;
		case 1200: cflag |= B1200; break;
		case 2400: cflag |= B2400; break;
		case 4800: cflag |= B4800; break;
		case 9600: cflag |= B9600; break;
		case 19200: cflag |= B19200; break;
		case 38400: cflag |= B38400; break;
		case 57600: cflag |= B57600; break;
		case 115200: cflag |= B115200; break;
		case 230400: cflag |= B230400; break;
		case 460800: cflag |= B460800; break;
		default: baud = 9600; cflag |= B9600; break;
	}

	switch (bits) {
		case 5: cflag |= CS5; b