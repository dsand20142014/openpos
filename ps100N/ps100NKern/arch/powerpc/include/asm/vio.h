freq);
	tea5757_write(btv, 5 * freq + 0x358); /* add 10.7MHz (see docs) */
}

/* RemoteVision MX (rv605) muxsel helper [Miguel Freitas]
 *
 * This is needed because rv605 don't use a normal multiplex, but a crosspoint
 * switch instead (CD22M3494E). This IC can have multiple active connections
 * between Xn (input) and Yn (output) pins. We need to clear any existing
 * connection prior to establish a new one, pulsing the STROBE pin.
 *
 * The board hardwire Y0 (xpoint) to MUX1 and MUXOUT to Yin.
 * GPIO pins are wired as:
 *  GPIO[0:3] - AX[0:3] (xpoint) - P1[0:3] (microcontroller)
 *  GPIO[4:6] - AY[0:2] (xpoint) - P1[4:6] (microcontroller)
 *  GPIO[7]   - DATA (xpoint)    - P1[7] (microcontroller)
 *  GPIO[8]   -                  - P3[5] (microcontroller)
 *  GPIO[9]   - RESET (xpoint)   - P3[6] (microcontroller)
 *  GPIO[10]  - STROBE (xpoint)  - P3[7] (microcontroller)
 *  GPINTR    -                  - P3[4] (microcontroller)
 *
 * The microcontroller is a 80C32 like. It should be possible to change xpoint
 * configuration either directly (as we are doing) or using the microcontroller
 * which is also wired to I2C interface. I have no further info on the
 * microcontroller features, one would need to disassembly the firmware.
 * note: the vendor refused to give any information on this product, all
 *       that stuff was found using a multimeter! :)
 */
static void rv605_muxsel(struct bttv *btv, unsigned int input)
{
	static const u8 muxgpio[] = { 0x3, 0x1, 0x2, 0x4, 0xf, 0x7, 0xe, 0x0,
				      0xd, 0xb, 0xc, 0x6, 0x9, 0x5, 0x8, 0xa };

	gpio_bits(0x07f, muxgpio[input]);

	/* reset all conections */
	gpio_bits(0x200,0x200);
	mdelay(1);
	gpio_bits(0x200,0x000);
	mdelay(1);

	/* create a new connection */
	gpio_bits(0x480,0x480);
	mdelay(1);
	gpio_bits(0x480,0x080);
	mdelay(1);
}

/* Tibet Systems 'Progress DVR' CS16 muxsel helper [Chris Fanning]
 *
 * The CS16 (available on eBay cheap) is a PCI board with four Fusion
 * 878A chips, a PCI bridge, an Atmel microcontroller, four sync seperator
 * chips, ten eight input analog multiplexors, a not chip and a few
 * other components.
 *
 * 16 inputs on a secondary bracket are provided and can be selected
 * from each of the four capture chips.  Two of the eight input
 * multiplexors are used to select from any of the 16 input signals.
 *
 * Unsupported hardware capabilities:
 *  . A video output monitor on the secondary bracket can be selected from
 *    one of the 878A chips.
 *  . Another passthrough but I haven't spent any time investigating it.
 *  . Digital I/O (logic level connected to GPIO) is available from an
 *    onboard header.
 *
 * The on chip input mux should always be set to 2.
 * GPIO[16:19] - Video input selection
 * GPIO[0:3]   - Video output monitor select (only available from one 878A)
 * GPIO[?:?]   - Digital I/O.
 *
 * There is an ATMEL microcontroller with an 8031 core on board.  I have not
 * determined what function (if any) it provides.  With the microcontroller
 * and sync seperator chips a guess is that it might have to do with video
 * switching and maybe some digital I/O.
 */
static void tibetCS16_muxsel(struct bttv *btv, unsigned int input)
{
	/* video mux */
	gpio_bits(0x0f0000, input << 16);
}

static void tibetCS16_init(s