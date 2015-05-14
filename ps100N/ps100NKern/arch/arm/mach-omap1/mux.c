    btv->c.nr,
	       (unsigned long)btv->main.dma,
	       (unsigned long)le32_to_cpu(btv->main.cpu[RISC_SLOT_O_VBI+1]),
	       (unsigned long)le32_to_cpu(btv->main.cpu[RISC_SLOT_O_FIELD+1]),
	       (unsigned long)rc);

	if (0 == (btread(BT848_DSTATUS) & BT848_DSTATUS_HLOC)) {
		printk("bttv%d: Oh, there (temporarely?) is no input signal. "
		       "Ok, then this is harmless, don't worry ;)\n",
		       btv->c.nr);
		return;
	}
	printk("bttv%d: Uhm. Looks like we have unusual high IRQ latencies.\n",
	       btv->c.nr);
	printk("bttv%d: Lets try to catch the culpit red-handed ...\n",
	       btv->c.nr);
	dump_stack();
}

static int
bttv_irq_next_video(struct bttv *btv, struct bttv_buffer_set *set)
{
	struct bttv_buffer *item;

	memset(set,0,sizeof(*set));

	/* capture request ? */
	if (!list_empty(&btv->capture)) {
		set->frame_irq = 1;
		item = list_entry(btv->capture.next, struct bttv_buffer, vb.queue);
		if (V4L2_FIELD_HAS_TOP(item->vb.field))
			set->top    = item;
		if (V4L2_FIELD_HAS_BOTTOM(item->vb.field))
			set->bottom = item;

		/* capture request for other field ? */
		if (!V4L2_FIELD_HAS_BOTH(item->vb.field) &&
		    (item->vb.queue.next != &btv->capture)) {
			item = list_entry(item->vb.queue.next, struct bttv_buffer, vb.queue);
			if (!V4L2_FIELD_HAS_BOTH(item->vb.field)) {
				if (NULL == set->top &&
				    V4L2_FIELD_TOP == item->vb.field) {
					set->top = item;
				}
				if (NULL == set->bottom &&
				    V4L2_FIELD_BOTTOM == item->vb.field) {
					set->bottom = item;
				}
				if (NULL != set->top  &&  NULL != set->bottom)
					set->top_irq = 2;
			}
		}
	}

	/* screen overlay ? */
	if (NULL != btv->screen) {
		if (V4L2_FIELD_HAS_BOTH(btv->screen->vb.field)) {
			if (NULL == set->top && NULL == set->bottom) {
				set->top    = btv->screen;
				set->bottom = btv->screen;
			}
		} else {
			if (V4L2_FIELD_TOP == btv->screen->vb.field &&
			    NULL == set->top) {
				set->top = btv->screen;
			}
			if (V4L2_FIELD_BOTTOM == btv->screen->vb.field &&
			    NULL == set->bottom) {
				set->bottom = btv->screen;
			}
		}
	}

	dprintk("bttv%d: next set: top=%p bottom=%p [screen=%p,irq=%d,%d]\n",
		btv->c.nr,set->top, set->bottom,
		btv->screen,set->frame_irq,set->top_irq);
	return 0;
}

static void
bttv_irq_wakeup_video(struct bttv *btv, struct bttv_buffer_set *wakeup,
		      struct bttv_buffer_set *curr, unsigned int state)
{
	struct timeval ts;

	do_gettimeofday(&ts);

	if (wakeup->top == wakeup->bottom) {
		if (NULL != wakeup->top && curr->top != wakeup->top) {
			if (irq_debug > 1)
				printk("bttv%d: wakeup: both=%p\n",btv->c.nr,wakeup->top);
			wakeup->top->vb.ts = ts;
			wakeup->top->vb.field_count = btv->field_count;
			wakeup->top->vb.state = state;
			wake_up(&wakeup->top->vb.done);
		}
	} else {
		if (NULL != wakeup->top && curr->top != wakeup->top) {
			if (irq_debug > 1)
				printk("bttv%d: wakeup: top=%p\n",btv->c.nr,wakeup->top);
			wakeup->top->vb.ts = ts;
			wakeup->top->vb.field_count = btv->field_count;
			wakeup->top->vb.state = state;
			wake_up(&wakeup->top->vb.done);
		}
		if (NULL != wakeup->bottom && curr->bottom != wakeup->bottom) {
			if (irq_debug > 1)
				printk("bttv%d: wakeup: bottom=%p\n",btv->c.nr,wakeup->bottom);
			wakeup->bottom->vb.ts = ts;
			wakeup->bottom->vb.field_count = btv->field_count;
			wakeup->bottom->vb.state = state;
			wake_up(&wakeup->bottom->vb.done);
		}
	}
}

static void
bttv_irq_wakeup_vbi(struct bttv *btv, struct bttv_buffer *wakeup,
		    unsigned int state)
{
	struct timeval ts;

	if (NULL == wakeup)
		return;

	do_gettimeofday(&ts);
	wakeup->vb.ts = ts;
	wakeup->vb.field_count = btv->field_count;
	wakeup->vb.state = state;
	wake_up(&wakeup->vb.done);
}

static void bttv_irq_timeout(unsigned long data)
{
	struct bttv *btv = (struct bttv *)data;
	struct bttv_buffer_set old,new;
	struct bttv_buffer *ovbi;
	struct bttv_buffer *item;
	unsigned long flags;

	if (bttv_verbose) {
		printk(KERN_INFO "bttv%d: timeout: drop=%d irq=%d/%d, risc=%08x, ",
		       btv->c.nr, btv->framedrop, btv->irq_me, btv->irq_total,
		       btread(BT848_RISC_COUNT));
		btt"UART3_CLKREQ",		 9,   27,    0,	  2,   5,   0,	 NA,	 0,  0)
MUX_CFG("UART3_BCLK",		 A,    0,    0,	  2,   6,   0,	 NA,	 0,  0)
MUX_CFG("Y15_1610_UART3_RTS",	 A,    0,    1,	  2,   6,   0,	 NA,	 0,  0)

/* PWT & PWL, conflicts with UART3 */
MUX_CFG("PWT",			 6,    0,    2,	  0,  30,   0,	 NA,	 0,  0)
MUX_CFG("PWL",			 6,    3,    1,	  0,  31,   1,	 NA,	 0,  0)

/* USB internal master generic */
MUX_CFG("R18_USB_VBUS",		 7,    9,    2,	  1,  11,   0,	 NA,	 0,  1)
MUX_CFG("R18_1510_USB_GPIO0",	 7,    9,    0,	  1,  11,   1,	 NA,	 0,  1)
/* works around erratum:  W4_USB_PUEN and W4_USB_PUDIS are switched! */
MUX_CFG("W4_USB_PUEN",		 D,    3,    3,	  3,   5,   1,	 NA,	 0,  1)
MUX_CFG("W4_USB_CLKO",		 D,    3,    1,	  3,   5,   0,	 NA,	 0,  1)
MUX_CFG("W4_USB_HIGHZ",		 D,    3,    4,	  3,   5,   0,	  3,	 0,  1)
MUX_CFG("W4_GPIO58",		 D,    3,    7,	  3,   5,   0,	  3,	 0,  1)

/* USB1 master */
MUX_CFG("USB1_SUSP",		 8,   27,    2,	  1,  27,   0,	 NA,	 0,  1)
MUX_CFG("USB1_SE0",		 9,    0,    2,	  1,  28,   0,	 NA,	 0,  1)
MUX_CFG("W13_1610_USB1_SE0",	 9,    0,    4,	  1,  28,   0,	 NA,	 0,  1)
MUX_CFG("USB1_TXEN",		 9,    3,    2,	  1,  29,   0,	 NA,	 0,  1)
MUX_CFG("USB1_TXD",		 9,   24,    1,	  2,   4,   0,	 NA,	 0,  1)
MUX_CFG("USB1_VP",		 A,    3,    1,	  2,   7,   0,	 NA,	 0,  1)
MUX_CFG("USB1_VM",		 A,    6,    1,	  2,   8,   0,	 NA,	 0,  1)
MUX_CFG("USB1_RCV",		 A,    9,    1,	  2,   9,   0,	 NA,	 0,  1)
MUX_CFG("USB1_SPEED",		 A,   12,    2,	  2,  10,   0,	 NA,	 0,  1)
MUX_CFG("R13_1610_USB1_SPEED",	 A,   12,    5,	  2,  10,   0,	 NA,	 0,  1)
MUX_CFG("R13_1710_USB1_SEO",	 A,   12,    5,   2,  10,   0,   NA,     0,  1)

/* USB2 master */
MUX_CFG("USB2_SUSP",		 B,    3,    1,	  2,  17,   0,	 NA,	 0,  1)
MUX_CFG("USB2_VP",		 B,    6,    1,	  2,  18,   0,	 NA,	 0,  1)
MUX_CFG("USB2_TXEN",		 B,    9,    1,	  2,  19,   0,	 NA,	 0,  1)
MUX_CFG("USB2_VM",		 C,   18,    1,	  3,   0,   0,	 NA,	 0,  1)
MUX_CFG("USB2_RCV",		 C,   21,    1,	  3,   1,   0,	 NA,	 0,  1)
MUX_CFG("USB2_SE0",		 C,   24,    2,	  3,   2,   0,	 NA,	 0,  1)
MUX_CFG("USB2_TXD",		 C,   27,    2,	  3,   3,   0,	 NA,	 0,  1)

/* OMAP-1510 GPIO */
MUX_CFG("R18_1510_GPIO0",	 7,    9,    0,   1,  11,   1,    0,     0,  1)
MUX_CFG("R19_1510_GPIO1",	 7,    6,    0,   1,  10,   1,    0,     0,  1)
MUX_CFG("M14_1510_GPIO2",	 7,    3,    0,   1,   9,   1,    0,     0,  1)

/* OMAP1610 GPIO */
MUX_CFG("P18_1610_GPIO3",	 7,    0,    0,   1,   8,   0,   NA,     0,  1)
MUX_CFG("Y15_1610_GPIO17",	 A,    0,    7,   2,   6,   0,   NA,     0,  1)

/* OMAP-1710 GPIO */
MUX_CFG("R18_1710_GPIO0",        7,    9,    0,   1,  11,   1,    1,     1,  1)
MUX_CFG("V2_1710_GPIO10",        F,   27,    1,   4,   3,   1,    4,     1,  1)
MUX_CFG("N21_1710_GPIO14",       6,    9,    0,   1,   1,   1,    1,     1,  1)
MUX_CFG("W15_1710_GPIO40",       9,   27,    7,   2,   5,   1,    2,     1,  1)

/* MPUIO */
MUX_CFG("MPUIO2",		 7,   18,    0,	  1,  14,   1,	 NA,	 0,  1)
MUX_CFG("N15_1610_MPUIO2",	 7,   18,    0,	  1,  14,   1,	  1,	 0,  1)
MUX_CFG("MPUIO4",		 7,   15,    0,	  1,  13,   1,	 NA,	 0,  1)
MUX_CFG("MPUIO5",		 7,   12,    0,	  1,  12,   1,	 NA,	 0,  1)

MUX_CFG("T20_1610_MPUIO5",	 7,   12,    0,	  1,  12,   0,	  3,	 0,  1)
MUX_CFG("W11_1610_MPUIO6",	10,   15,    2,	  3,   8,   0,	  3,	 0,  1)
MUX_CFG("V10_1610_MPUIO7",	 A,   24,    2,	  2,  14,   0,	  2,	 0,  1)
MUX_CFG("W11_1610_MPUIO9",	10,   15,    1,	  3,   8,   0,	  3,	 0,  1)
MUX_CFG("V10_1610_MPUIO10",	 A,   24,    1,	  2,  14,   0,	  2,	 0,  1)
MUX_CFG("W10_1610_MPUIO11",	 A,   18,    2,	  2,  11,   0,	  2,	 0,  1)
MUX_CFG("E20_1610_MPUIO13",	 3,   21,    1,	  0,   7,   0,	  0,	 0,  1)
MUX_CFG("U20_1610_MPUIO14",	 9,    6,    6,	  0,  30,   0,	  0,	 0,  1)
MUX_CFG("E19_1610_MPUIO15",	 3,   18,    1,	  0,   6,   0,	  0,	 0,  1)

/* MCBSP2 */
MUX_CFG("MCBSP2_CLKR",		 C,    6,    0,	  2,  27,   1,	 NA,	 0,  1)
MUX_CFG("MCBSP2_CLKX",		 C,    9,    0,	  2,  29,   1,	 NA,	 0,  1)
MUX_CFG("MCBSP2_DR",		 C,    0,    0,	  2,  26,   1,	 NA,	 0,  1)
MUX_CFG("MCBSP2_DX",		 C,   15,    0,	  2,  31,   1,	 NA,	 0,  1)
MUX_CFG("MCBSP2_FSR",		 C,   12,    0,	  2,  30,   1,	 NA,	 0,  1)
MUX_CFG("MCBSP2_FSX",		 C,    3,    0,	  2,  27,   1,	 NA,	 0,  1)

/* MCBSP3 NOTE: Mode must 1 for clock */
MUX_CFG("MCBSP3_CLKX",		 9,    3,    1,	  1,  29,   0,	 NA,	 0,  1)

/* Misc ballouts */
MUX_CFG("BALLOUT_V8_ARMIO3",	 B,   18,    0,	  2,  25,   1,	 NA,	 0,  1)
MUX_CFG("N20_HDQ",		 6,   18,    1,   1,   4,   0,    1,     4,  0)

/* OMAP-1610 MMC2 */
MUX_CFG("W8_1610_MMC2_DAT0",	 B,   21,    6,	  2,  23,   1,	  2,	 1,  1)
MUX_CFG("V8_1610_MMC2_DAT1",	 B,   27,    6,	  2,  25,   1,	  2,	 1,  1)
MUX_CFG("W15_1610_MMC2_DAT2",	 9,   12,    6,	  2,   5,   1,	  2,	 1,  1)
MUX_CFG("R10_1610_MMC2_DAT3",	 B,   18,    6,	  2,  22,   1,	  2,	 1,  1)
MUX_CFG("Y10_1610_MMC2_CLK",	 B,    3,    6,	  2,  17,   0,	  2,	 0,  1)
MUX_CFG("Y8_1610_MMC2_CMD",	 B,   24,    6,	  2,  24,   1,	  2,	 1,  1)
MUX_CFG("V9_1610_MMC2_CMDDIR",	 B,   12,    6,	  2,  20,   0,	  2,	 1,  1)
MUX_CFG("V5_1610_MMC2_DATDIR0",	 B,   15,    6,	  2,  21,   0,	  2,	 1,  1)
MUX_CFG("W19_1610_MMC2_DATDIR1", 8,   15,    6,	  1,  23,   0,	  1,	 1,  1)
MUX_CFG("R18_1610_MMC2_CLKIN",	 7,    9,    6,	  1,  11,   0,	  1,	11,  1)

/* OMAP-1610 External Trace Interface */
MUX_CFG("M19_1610_ETM_PSTAT0",	 5,   27,    1,	  0,  29,   0,	  0,	 0,  1)
MUX_CFG("L15_1610_ETM_PSTAT1",	 5,   24,    1,	  0,  28,   0,	  0,	 0,  1)
MUX_CFG("L18_1610_ETM_PSTAT2",	 5,   21,    1,	  0,  27,   0,	  0,	 0,  1)
MUX_CFG("L19_1610_ETM_D0",	 5,   18,    1,	  0,  26,   0,	  0,	 0,  1)
MUX_CFG("J19_1610_ETM_D6",	 5,    0,    1,	  0,  20,   0,	  0,	 0,  1)
MUX_CFG("J18_1610_ETM_D7",	 5,   27,    1,	  0,  19,   0,	  0,	 0,  1)

/* OMAP16XX GPIO */
MUX_CFG("P20_1610_GPIO4",	 6,   27,    0,	  1,   7,   0,	  1,	 1,  1)
MUX_CFG("V9_1610_GPIO7",	 B,   12,    1,	  2,  20,   0,	  2,	 1,  1)
MUX_CFG("W8_1610_GPIO9",	 B,   21,    0,	  2,  23,   0,	  2,	 1,  1)
MUX_CFG("N20_1610_GPIO11",       6,   18,    0,   1,   4,   0,    1,     1,  1)
MUX_CFG("N19_1610_GPIO13",	 6,   12,    0,	  1,   2,   0,	  1,	 1,  1)
MUX_CFG("P10_1610_GPIO22",	 C,    0,    7,	  2,  26,   0,	  2,	 1,  1)
MUX_CFG("V5_1610_GPIO24",	 B,   15,    7,	  2,  21,   0,	  2,	 1,  1)
MUX_CFG("AA20_1610_GPIO_41",	 9,    9,    7,	  1,  31,   0,	  1,	 1,  1)
MUX_CFG("W19_1610_GPIO48",	 8,   15,    7,   1,  23,   1,    1,     0,  1)
MUX_CFG("M7_1610_GPIO62",	10,    0,    0,   4,  24,   0,    4,     0,  1)
MUX_CFG("V14_16XX_GPIO37",	 9,   18,    7,	  2,   2,   0,	  2,	 2,  0)
MUX_CFG("R9_16XX_GPIO18",	 C,   18,    7,   3,   0,   0,    3,     0,  0)
MUX_CFG("L14_16XX_GPIO49",	 6,    3,    7,   0,  31,   0,    0,    31,  0)

/* OMAP-1610 uWire */
MUX_CFG("V19_1610_UWIRE_SCLK",	 8,    6,    0,	  1,  20,   0,	  1,	 1,  1)
MUX_CFG("U18_1610_UWIRE_SDI",	 8,    0,    0,	  1,  18,   0,	  1,	 1,  1)
MUX_CFG("W21_1610_UWIRE_SDO",	 8,    3,    0,	  1,  19,   0,	  1,	 1,  1)
MUX_CFG("N14_1610_UWIRE_CS0",	 8,    9,    1,	  1,  21,   0,	  1,	 1,  1)
MUX_CFG("P15_1610_UWIRE_CS3",	 8,   12,    1,	  1,  22,   0,	  1,	 1,  1)
MUX_CFG("N15_1610_UWIRE_CS1",	 7,   18,    2,	  1,  14,   0,	 NA,	 0,  1)

/* OMAP-1610 SPI */
MUX_CFG("U19_1610_SPIF_SCK",	 7,    21,   6,	  1,  15,   0,	  1,	 1,  1)
MUX_CFG("U18_1610_SPIF_DIN",	 8,    0,    6,	  1,  18,   1,	  1,	 0,  1)
MUX_CFG("P20_1610_SPIF_DIN",	 6,    27,   4,   1,   7,   1,    1,     0,  1)
MUX_CFG("W21_1610_SPIF_DOUT",	 8,    3,    6,	  1,  19,   0,	  1,	 0,  1)
MUX_CFG("R18_1610_SPIF_DOUT",	 7,    9,    3,	  1,  11,   0,	  1,	 0,  1)
MUX_CFG("N14_1610_SPIF_CS0",	 8,    9,    6,	  1,  21,   0,	  1,	 1,  1)
MUX_CFG("N15_1610_SPIF_CS1",	 7,    18,   6,	  1,  14,   0,	  1,	 1,  1)
MUX_CFG("T19_1610_SPIF_CS2",	 7,    15,   4,	  1,  13,   0,	  1,	 1,  1)
MUX_CFG("P15_1610_SPIF_CS3",	 8,    12,   3,	  1,  22,   0,	  1,	 1,  1)

/* OMAP-1610 Flash */
MUX_CFG("L3_1610_FLASH_CS2B_OE",10,    6,    1,	 NA,   0,   0,	 NA,	 0,  1)
MUX_CFG("M8_1610_FLASH_CS2B_WE",10,    3,    1,	 NA,   0,   0,	 NA,	 0,  1)

/* First MMC interface, same on 1510, 1610 and 1710 */
MUX_CFG("MMC_CMD",		 A,   27,    0,	  2,  15,   1,	  2,	 1,  1)
MUX_CFG("MMC_DAT1",		 A,   24,    0,	  2,  14,   1,	 n",
	       btv->c.nr, btv->vbi_dev->num);

	if (!btv->has_radio)
		return 0;
	/* radio */
	btv->radio_dev = vdev_init(btv, &radio_template, "radio");
	if (NULL == btv->radio_dev)
		goto err;
	if (video_register_device(btv->radio_dev, VFL_TYPE_RADIO,
				  radio_nr[btv->c.nr]) < 0)
		goto err;
	printk(KERN_INFO "bttv%d: registered device radio%d\n",
	       btv->c.nr, btv->radio_dev->num);

	/* all done */
	return 0;

 err:
	bttv_unregister_video(btv);
	return -1;
}


/* on OpenFirmware machines (PowerMac at least), PCI memory cycle */
/* response on cards with no firmware is not enabled by OF */
static void pci_set_command(struct pci_dev *dev)
{
#if defined(__powerpc__)
	unsigned int cmd;

	pci_read_config_dword(dev, PCI_COMMAND, &cmd);
	cmd = (cmd | PCI_COMMAND_MEMORY );
	pci_write_config_dword(dev, PCI_COMMAND, cmd);
#endif
}

static int __devinit bttv_probe(struct pci_dev *dev,
				const struct pci_device_id *pci_id)
{
	int result;
	unsigned char lat;
	struct bttv *btv;

	if (bttv_num == BTTV_MAX)
		return -ENOMEM;
	printk(KERN_INFO "bttv: Bt8xx card found (%d).\n", bttv_num);
	bttvs[bttv_num] = btv = kzalloc(sizeof(*btv), GFP_KERNEL);
	if (btv == NULL) {
		printk(KERN_ERR "bttv: out of memory.\n");
		return -ENOMEM;
	}
	btv->c.nr  = bttv_num;
	snprintf(btv->c.v4l2_dev.name, sizeof(btv->c.v4l2_dev.name),
			"bttv%d", btv->c.nr);

	/* initialize structs / fill in defaults */
	mutex_init(&btv->lock);
	spin_lock_init(&btv->s_lock);
	spin_lock_init(&btv->gpio_lock);
	init_waitqueue_head(&btv->gpioq);
	init_waitqueue_head(&btv->i2c_queue);
	INIT_LIST_HEAD(&btv->c.subs);
	INIT_LIST_HEAD(&btv->capture);
	INIT_LIST_HEAD(&btv->vcapture);
	v4l2_prio_init(&btv->prio);

	init_timer(&btv->timeout);
	btv->timeout.function = bttv_irq_timeout;
	btv->timeout.data     = (unsigned long)btv;

	btv->i2c_rc = -1;
	btv->tuner_type  = UNSET;
	btv->new_input   = UNSET;
	btv->has_radio=radio[btv->c.nr];

	/* pci stuff (init, get irq/mmio, ... */
	btv->c.pci = dev;
	btv->id  = dev->device;
	if (pci_enable_device(dev)) {
		printk(KERN_WARNING "bttv%d: Can't enable device.\n",
		       btv->c.nr);
		return -EIO;
	}
	if (pci_set_dma_mask(dev, DMA_BIT_MASK(32))) {
		printk(KERN_WARNING "bttv%d: No suitable DMA available.\n",
		       btv->c.nr);
		return -EIO;
	}
	if (!request_mem_region(pci_resource_start(dev,0),
				pci_resource_len(dev,0),
				btv->c.v4l2_dev.name)) {
		printk(KERN_WARNING "bttv%d: can't request iomem (0x%llx).\n",
		       btv->c.nr,
		       (unsigned long long)pci_resource_start(dev,0));
		return -EBUSY;
	}
	pci_set_master(dev);
	pci_set_command(dev);

	result = v4l2_device_register(&dev->dev, &btv->c.v4l2_dev);
	if (result < 0) {
		printk(KERN_WARNING "bttv%d: v4l2_device_register() failed\n", btv->c.nr);
		goto fail0;
	}

	pci_read_config_byte(dev, PCI_CLASS_REVISION, &btv->revision);
	pci_read_config_byte(dev, PCI_LATENCY_TIMER, &lat);
	printk(KERN_INFO "bttv%d: Bt%d (rev %d) at %s, ",
	       bttv_num,btv->id, btv->revision, pci_name(dev));
	printk("irq: %d, latency: %d, mmio: 0x%llx\n",
	       btv->c.pci->irq, lat,
	       (unsigned long long)pci_resource_start(dev,0));
	schedule();

	btv->bt848_mmio = ioremap(pci_resource_start(dev, 0), 0x1000);
	if (NULL == btv->bt848_mmio) {
		printk("bttv%d: ioremap() failed\n", btv->c.nr);
		result = -EIO;
		goto fail1;
	}

	/* identify card */
	bttv_idcard(btv);

	/* disable irqs, register irq handler */
	btwrite(0, BT848_INT_MASK);
	result = request_irq(btv->c.pci->irq, bttv_irq,
	    IRQF_SHARED | IRQF_DISABLED, btv->c.v4l2_dev.name, (void *)btv);
	if (result < 0) {
		printk(KERN_ERR "bttv%d: can't get IRQ %d\n",
		       bttv_num,btv->c.pci->irq);
		goto fail1;
	}

	if (0 != bttv_handle_chipset(btv)) {
		result = -EIO;
		goto fail2;
	}

	/* init options from insmod args */
	btv->opt_combfilter = combfilter;
	btv->opt_lumafilter = lumafilter;
	btv->opt_automute   = automute;
	btv->opt_chroma_agc = chroma_agc;
	btv->opt_adc_crush  = adc_crush;
	btv->opt_vcr_hack   = vcr_hack;
	btv->opt_whitecrush_upper  = whitecrush_upper;
	btv->opt_whitecrush_lower  = whitecrush_lower;
	btv->op4",	 5,    6,    0,   0,  22,   1,    0,     0,  0)
MUX_CFG("K19_1610_CAM_D3",	 5,    9,    0,   0,  23,   1,    0,     0,  0)
MUX_CFG("K15_1610_CAM_D2",	 5,   12,    0,   0,  24,   1,    0,     0,  0)
MUX_CFG("K14_1610_CAM_D1",	 5,   15,    0,   0,  25,   1,    0,     0,  0)
MUX_CFG("L19_1610_CAM_D0",	 5,   18,    0,   0,  26,   1,    0,     0,  0)
MUX_CFG("L18_1610_CAM_VS",	 5,   21,    0,   0,  27,   1,    0,     0,  0)
MUX_CFG("L15_1610_CAM_HS",	 5,   24,    0,   0,  28,   1,    0,     0,  0)
MUX_CFG("M19_1610_CAM_RSTZ",	 5,   27,    0,   0,  29,   0,    0,     0,  0)
MUX_CFG("Y15_1610_CAM_OUTCLK",	 A,    0,    6,   2,   6,   0,    2,     0,  0)

/* serial camera */
MUX_CFG("H19_1610_CAM_EXCLK",	 4,   21,    0,   0,  17,   0,    0,     0,  0)
	/* REVISIT 5912 spec sez CCP_* can't pullup or pulldown ... ? */
MUX_CFG("Y12_1610_CCP_CLKP",	 8,   18,    6,   1,  24,   1,    1,     0,  0)
MUX_CFG("W13_1610_CCP_CLKM",	 9,    0,    6,   1,  28,   1,    1,     0,  0)
MUX_CFG("W14_1610_CCP_DATAP",	 9,   24,    6,   2,   4,   1,    2,     0,  0)
MUX_CFG("Y14_1610_CCP_DATAM",	 9,   21,    6,   2,   3,   1,    2,     0,  0)
};
#define OMAP1XXX_PINS_SZ	ARRAY_SIZE(omap1xxx_pins)
#else
#define omap1xxx_pins		NULL
#define OMAP1XXX_PINS_SZ	0
#endif	/* CONFIG_ARCH_OMAP15XX || CONFIG_ARCH_OMAP16XX */

int __init_or_module omap1_cfg_reg(const struct pin_config *cfg)
{
	static DEFINE_SPINLOCK(mux_spin_lock);
	unsigned long flags;
	unsigned int reg_orig = 0, reg = 0, pu_pd_orig = 0, pu_pd = 0,
		pull_orig = 0, pull = 0;
	unsigned int mask, warn = 0;

	/* Check the mux register in question */
	if (cfg->mux_reg) {
		unsigned	tmp1, tmp2;

		spin_lock_irqsave(&mux_spin_lock, flags);
		reg_orig = omap_readl(cfg->mux_reg);

		/* The mux registers always seem to be 3 bits long */
		mask = (0x7 << cfg->mask_offset);
		tmp1 = reg_orig & mask;
		reg = reg_orig & ~mask;

		tmp2 = (cfg->mask << cfg->mask_offset);
		reg |= tmp2;

		if (tmp1 != tmp2)
			warn = 1;

		omap_writel(reg, cfg->mux_reg);
		spin_unlock_irqrestore(&mux_spin_lock, flags);
	}

	/* Check for pull up or pull down selection on 1610 */
	if (!cpu_is_omap15xx()) {
		if (cfg->pu_pd_reg && cfg->pull_val) {
			spin_lock_irqsave(&mux_spin_lock, flags);
			pu_pd_orig = omap_readl(cfg->pu_pd_reg);
			mask = 1 << cfg->pull_bit;

			if (cfg->pu_pd_val) {
				if (!(pu_pd_orig & mask))
					warn = 1;
				/* Use pull up */
				pu_pd = pu_pd_orig | mask;
			} else {
				if (pu_pd_orig & mask)
					warn = 1;
				/* Use pull down */
				pu_pd = pu_pd_orig & ~mask;
			}
			omap_writel(pu_pd, cfg->pu_pd_reg);
			spin_unlock_irqrestore(&mux_spin_lock, flags);
		}
	}

	/* Check for an associated pull down register */
	if (cfg->pull_reg) {
		spin_lock_irqsave(&mux_spin_lock, flags);
		pull_orig = omap_readl(cfg->pull_reg);
		mask = 1 << cfg->pull_bit;

		if (cfg->pull_val) {
			if (pull_orig & mask)
				warn = 1;
			/* Low bit = pull enabled */
			pull = pull_orig & ~mask;
		} else {
			if (!(pull_orig & mask))
				warn = 1;
			/* High bit = pull disabled */
			pull = pull_orig | mask;
		}

		omap_writel(pull, cfg->pull_reg);
		spin_unlock_irqrestore(&mux_spin_lock, flags);
	}

	if (warn) {
#ifdef CONFIG_OMAP_MUX_WARNINGS
		printk(KERN_WARNING "MUX: initialized %s\n", cfg->name);
#endif
	}

#ifdef CONFIG_OMAP_MUX_DEBUG
	if (cfg->debug || warn) {
		printk("MUX: Setting register %s\n", cfg->name);
		printk("      %s (0x%08x) = 0x%08x -> 0x%08x\n",
		       cfg->mux_reg_name, cfg->mux_reg, reg_orig, reg);

		if (!cpu_is_omap15xx()) {
			if (cfg->pu_pd_reg && cfg->pull_val) {
				printk("      %s (0x%08x) = 0x%08x -> 0x%08x\n",
				       cfg->pu_pd_name, cfg->pu_pd_reg,
				       pu_pd_orig, pu_pd);
			}
		}

		if (cfg->pull_reg)
			printk("      %s (0x%08x) = 0x%08x -> 0x%08x\n",
			       cfg->pull_name, cfg->pull_reg, pull_orig, pull);
	}

#ifdef CONFIG_ARCH_OMAP850
	omap_mux_register(omap850_pins, ARRAY_SIZE(omap850_pins));
#endif

#endif

#ifdef CONFIG_OMAP_MUX_ERRORS
	return warn ? -ETXTBSY : 0;
#else
	return 0;
#endif
}

int __init omap1_mux_init(void)
{
	if (cpu_is_omap730()) {
		arch_mux_cfg.pins	=btv->c.nr);

	/* restore pci state */
	if (btv->state.disabled) {
		err=pci_enable_device(pci_dev);
		if (err) {
			printk(KERN_WARNING "bttv%d: Can't enable device.\n",
								btv->c.nr);
			return err;
		}
		btv->state.disabled = 0;
	}
	err=pci_set_power_state(pci_dev, PCI_D0);
	if (err) {
		pci_disable_device(pci