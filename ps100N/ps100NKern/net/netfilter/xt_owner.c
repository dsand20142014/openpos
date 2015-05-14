 unsigned int code, int value)
{
	struct sunkbd *sunkbd = input_get_drvdata(dev);

	switch (type) {

		case EV_LED:

			sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_SETLED);
			sunkbd->serio->write(sunkbd->serio,
				(!!test_bit(LED_CAPSL, dev->led) << 3) | (!!test_bit(LED_SCROLLL, dev->led) << 2) |
				(!!test_bit(LED_COMPOSE, dev->led) << 1) | !!test_bit(LED_NUML, dev->led));
			return 0;

		case EV_SND:

			switch (code) {

				case SND_CLICK:
					sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_NOCLICK - value);
					return 0;

				case SND_BELL:
					sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_BELLOFF - value);
					return 0;
			}

			break;
	}

	return -1;
}

/*
 * sunkbd_initialize() checks for a Sun keyboard attached, and determines
 * its type.
 */

static int sunkbd_initialize(struct sunkbd *sunkbd)
{
	sunkbd->reset = -2;
	sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_RESET);
	wait_event_interruptible_timeout(sunkbd->wait, sunkbd->reset >= 0, HZ);
	if (sunkbd->reset < 0)
		return -1;

	sunkbd->type = sunkbd->reset;

	if (sunkbd->type == 4) {	/* Type 4 keyboard */
		sunkbd->layout = -2;
		sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_LAYOUT);
		wait_event_interruptible_timeout(sunkbd->wait, sunkbd->layout >= 0, HZ/4);
		if (sunkbd->layout < 0) return -1;
		if (sunkbd->layout & SUNKBD_LAYOUT_5_MASK) sunkbd->type = 5;
	}

	return 0;
}

/*
 * sunkbd_reinit() sets leds and beeps to a state the computer remembers they
 * were in.
 */

static void sunkbd_reinit(struct work_struct *work)
{
	struct sunkbd *sunkbd = container_of(work, struct sunkbd, tq);

	wait_event_interruptible_timeout(sunkbd->wait, sunkbd->reset >= 0, HZ);

	sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_SETLED);
	sunkbd->serio->write(sunkbd->serio,
		(!!test_bit(LED_CAPSL, sunkbd->dev->led) << 3) | (!!test_bit(LED_SCROLLL, sunkbd->dev->led) << 2) |
		(!!test_bit(LED_COMPOSE, sunkbd->dev->led) << 1) | !!test_bit(LED_NUML, sunkbd->dev->led));
	sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_NOCLICK - !!test_bit(SND_CLICK, sunkbd->dev->snd));
	sunkbd->serio->write(sunkbd->serio, SUNKBD_CMD_BELLOFF - !!test_bit(SND_BELL, sunkbd->dev->snd));
}

static void sunkbd_enable(struct sunkbd *sunkbd, int enable)
{
	serio_pause_rx(sunkbd->serio);
	sunkbd->enabled = enable;
	serio_continue_rx(sunkbd->serio);
}

/*
 * sunkbd_connect() probes for a Sun keyboard and fills the necessary structures.
 */

static int sunkbd_connect(struct serio *serio, struct serio_driver *drv)
{
	struct sunkbd *sunkbd;
	struct input_dev *input_dev;
	int err = -ENOMEM;
	int i;

	sunkbd = kzalloc(sizeof(struct sunkbd), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!sunkbd || !input_dev)
		goto fail1;

	sunkbd->serio = serio;
	sunkbd->dev = input_dev;
	init_waitqueue_head(&sunkbd->wait);
	INIT_WORK(&sunkbd->tq, sunkbd_reinit);
	snprintf(sunkbd->phys, sizeof(sunkbd->phys), "%s/input0", serio->phys);

	serio_set_drvdata(serio, sunkbd);

	err = serio_open(serio, drv);
	if (err)
		goto fail2;

	if (sunkbd_initialize(sunkbd) < 0) {
		err = -ENODEV;
		goto fail3;
	}

	snprintf(sunkbd->name, sizeof(sunkbd->name), "Sun Type %d keyboard", sunkbd->type);
	memcpy(sunkbd->keycode, sunkbd_keycode, sizeof(sunkbd->keycode));

	input_dev->name = sunkbd->name;
	input_dev->phys = sunkbd->phys;
	input_dev->id.bustype = BUS_RS232;
	input_dev->id.vendor  = SERIO_SUNKBD;
	input_dev->id.product = sunkbd->type;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &serio->dev;

	input_set_drvdata(input_dev, sunkbd);

	input_dev->event = sunkbd_event;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) |
		BIT_MASK(EV_SND) | BIT_MASK(EV_REP);
	input_dev->ledbit[0] = BIT_MASK(LED_CAPSL) | BIT_MASK(LED_COMPOSE) |
		BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_NUML);
	input_dev->sndbit[0] = BIT_MASK(SND_CLICK) | BIT_MASK(SND_BELL);

	input_dev->keycode = sunkbd->keycode;
	input_dev->keycodesize = sizeof(unsigned char);
	input_dev->keycodemax = ARRAY_SIZE(sunkbd_keycode);
	for (i = 0; i < 128; i++)
		set_bit(sunkbd->keycode[i], input_dev->keybit);
	clear_bit(0, input_dev->keybit);

	sunkbd_enable(sunkbd, 1);

	err = input_register_device(sunkbd->dev);
	if (err)
		goto fail4;

	return 0;

 fail4:	sunkbd_enable(sunkbd, 0);
 fail3:	serio_close(serio);
 fail2:	serio_set_drvdata(serio, NULL);
 fail1:	input_free_device(input_dev);
	kfree(sunkbd);
	return err;
}

/*
 * sunkbd_disconnect() unregisters and closes behind us.
 */

static void sunkbd_disconnect(struct serio *serio)
{
	struct sunkbd *sunkbd = serio_get_drvdata(serio);

	sunkbd_enable(sunkbd, 0);
	input_unregister_device(sunkbd->dev);
	serio_close(serio);
	serio_set_drvdata(serio, NULL);
	kfree(sunkbd);
}

static struct serio_device_id sunkbd_serio_ids[] = {
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_SUNKBD,
		.id	= SERIO_ANY,
		.extra	= SERIO_ANY,
	},
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_UNKNOWN, /* sunkbd does probe */
		.id	= SERIO_ANY,
		.extra	= SERIO_ANY,
	},
	{ 0 }
};
