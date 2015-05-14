/*
 *  Copyright (c) 1998-2001 Vojtech Pavlik
 */

/*
 * Genius Flight 2000 joystick driver for Linux
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/gameport.h>
#include <linux/jiffies.h>

#define DRIVER_DESC	"Genius Flight 2000 joystick driver"

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define GF2K_START		400	/* The time we wait for the first bit [400 us] */
#define GF2K_STROBE		40	/* The time we wait for the first bit [40 us] */
#define GF2K_TIMEOUT		4	/* Wait for everything to settle [4 ms] */
#define GF2K_LENGTH		80	/* Max number of triplets in a packet */

/*
 * Genius joystick ids ...
 */

#define GF2K_ID_G09		1
#define GF2K_ID_F30D		2
#define GF2K_ID_F30		3
#define GF2K_ID_F31D		4
#define GF2K_ID_F305		5
#define GF2K_ID_F23P		6
#define GF2K_ID_F31		7
#define GF2K_ID_MAX		7

static char gf2k_length[] = { 40, 40, 40, 40, 40, 40, 40, 40 };
static char gf2k_hat_to_axis[][2] = {{ 0, 0}, { 0,-1}, { 1,-1}, { 1, 0}, { 1, 1}, { 0, 1}, {-1, 1}, {-1, 0}, {-1,-1}};

static char *gf2k_names[] = {"", "Genius G-09D", "Genius F-30D", "Genius F-30", "Genius MaxFighter F-31D",
				"Genius F-30-5", "Genius Flight2000 F-23", "Genius F-31"};
static unsigned char gf2k_hats[] = { 0, 2, 0, 0, 2, 0, 2, 0 };
static unsigned char gf2k_axes[] = { 0, 2, 0, 0, 4, 0, 4, 0 };
static unsigned char gf2k_joys[] = { 0, 0, 0, 0,10, 0, 8, 0 };
static unsigned char gf2k_pads[] = { 0, 6, 0, 0, 0, 0, 0, 0 };
static unsigned char gf2k_lens[] = { 0,18, 0, 0,18, 0,18, 0 };

static unsigned char gf2k_abs[] = { ABS_X, ABS_Y, ABS_THROTTLE, ABS_RUDDER, ABS_GAS, ABS_BRAKE };
static short gf2k_btn_joy[] = { BTN_TRIGGER, BTN_THUMB, BTN_TOP, BTN_TOP2, BTN_BASE, BTN_BASE2, BTN_BASE3, BTN_BASE4 };
static short gf2k_btn_pad[] = { BTN_A, BTN_B, BTN_C, BTN_X, BTN_Y, BTN_Z, BTN_TL, BTN_TR, BTN_TL2, BTN_TR2, BTN_START, BTN_SELECT };


static short gf2k_seq_reset[] = { 240, 340, 0 };
static short gf2k_seq_digital[] = { 590, 320, 860, 0 };

struct gf2k {
	struct gameport *gameport;
	struct input_dev *dev;
	int reads;
	int bads;
	unsigned char id;
	unsigned char length;
	char phys[32];
};

/*
 * gf2k_read_packet() reads a Genius Flight2000 packet.
 */

static int gf2k_read_packet(struct gameport *gameport, int length, char *data)
{
	unsigned char u, v;
	int i;
	unsigned int t, p;
	unsigned long flags;

	t = gameport_time(gameport, GF2K_START);
	p = gameport_time(gameport, GF2K_STROBE);

	i = 0;

	local_irq_save(flags);

	gameport_trigger(gameport);
	v = gameport_read(gameport);

	while (t > 0 && i < length) {
		t--; u = v;
		v = gameport_read(gameport);
		if (v & ~u & 0x10) {
			data[i++] = v >> 5;
			t = p;
		}
	}

	local_irq_restore(flags);

	return i;
}

/*
 * gf2k_trigger_seq() initializes a Genius Flight2000 joystick
 * into digital mode.
 */

static void gf2k_trigger_seq(struct gameport *gameport, short *seq)
{

	unsigned long flags;
	int i, t;

        local_irq_save(flags);

	i = 0;
        do {
		gameport_trigger(gameport);
		t = gameport_time(gameport, GF2K_TIMEOUT * 1000);
		while ((gameport_read(gameport) & 1) && t