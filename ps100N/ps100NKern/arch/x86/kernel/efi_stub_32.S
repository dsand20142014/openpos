hing */
	  return;
	}

	rc=bttv_I2CRead(btv,(PX_I2C_PIC<<1),NULL);
	if (!(rc & PX_CFG_PXC200F)) {
	  printk(KERN_DEBUG "bttv%d: PXC200_muxsel: not PXC200F rc:%d \n", btv->c.nr,rc);
	  return;
	}


	/* The multiplexer in the 200F is handled by the GPIO port */
	/* get correct mapping between inputs  */
	/*  mux = bttv_tvcards[btv->type].muxsel[input] & 3; */
	/* ** not needed!?   */
	mux = input;

	/* make sure output pins are enabled */
	/* bitmask=0x30f; */
	bitmask=0x302;
	/* check whether we have a PXC200A */
	if (btv->cardid == PX_PXC200A_CARDID)  {
	   bitmask ^= 0x180; /* use 7 and 9, not 8 and 9 */
	   bitmask |= 7<<4; /* the DAC */
	}
	btwrite(bitmask, BT848_GPIO_OUT_EN);

	bitmask = btread(BT848_GPIO_DATA);
	if (btv->cardid == PX_PXC200A_CARDID)
	  bitmask = (bitmask & ~0x280) | ((mux & 2) << 8) | ((mux & 1) << 7);
	else /* older device */
	  bitmask = (bitmask & ~0x300) | ((mux & 3) << 8);
	btwrite(bitmask,BT848_GPIO_DATA);

	/*
	 * Was "to be safe, set the bt848 to input 0"
	 * Actually, since it's ok at load time, better not messing
	 * with these bits (on PXC200AF you need to set mux 2 here)
	 *
	 * needed because bttv-driver sets mux before calling this function
	 */
	if (btv->cardid == PX_PXC200A_CARDID)
	  btaor(2<<5, ~BT848_IFORM_MUXSEL, BT848_IFORM);
	else /* older device */
	  btand(~BT848_IFORM_MUXSEL,BT848_IFORM);

	printk(KERN_DEBUG "bttv%d: setting input channel to:%d\n", btv->c.nr,(int)mux);
}

static void phytec_muxsel(struct bttv *btv, unsigned int input)
{
	unsigned int mux = input % 4;

	if (input == btv->svhs)
		mux = 0;

	gpio_bits(0x3, mux);
}

/*
 * GeoVision GV-800(S) functions
 * Bruno Christo <bchristo@inf.ufsm.br>
*/

/* This is a function to control the analog switch, which determines which
 * camera is routed to which controller.  The switch comprises an X-address
 * (gpio bits 0-3, representing the camera, ranging from 0-15), and a
 * Y-address (gpio bits 4-6, representing the controller, ranging from 0-3).
 * A data value (gpio bit 18) of '1' enables the switch, and '0' disables
 * the switch.  A STROBE bit (gpio bit 17) latches the data value into the
 * specified address. There is also a chip select (gpio bit 16).
 * The idea is to set the address and chip select together, bring
 * STROBE high, write the data, and finally bring STROBE back to low.
 */
static void gv800s_write(struct bttv *btv,
			 unsigned char xaddr,
			 unsigned char yaddr,
			 unsigned char data) {
	/* On the "master" 878A:
	* GPIO bits 0-9 are used for the analog switch:
	*   00 - 03:	camera selector
	*   04 - 06:	878A (controller) selector
	*   16: 	cselect
	*   17:		strobe
	*   18: 	data (1->on, 0->off)
	*   19:		reset
	*/
	const u32 ADDRESS = ((xaddr&0xf) | (yaddr&3)<<4);
	const u32 CSELECT = 1<<16;
	const u32 STROBE = 