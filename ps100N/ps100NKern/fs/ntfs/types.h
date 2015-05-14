 >>= 1;
	} while (sendcode);

	return IO_GOT_PACKET | IO_MODE_FAST;
}

/*
 * Disables and restores interrupts for mp_io(), which does the actual I/O.
 */

static int multiport_io(struct gameport* gameport, int sendflags, int sendcode, u32 *packet)
{
	int status;
	unsigned long flags;

	local_irq_save(flags);
	status = mp_io(gameport, sendflags, sendcode, packet);
	local_irq_restore(flags);

	return status;
}

/*
 * Puts multiport into digital mode.  Multiport LED turns green.
 *
 * Returns true if a valid digital packet was received, false otherwise.
 */

static int dig_mode_start(struct gameport *gameport, u32 *packet)
{
	int i;
	int flags, tries = 0, bads = 0;

	for (i = 0; i < ARRAY_SIZE(init_seq); i++) {     /* Send magic sequence */
		if (init_seq[i])
			gameport_trigger(gameport);
		udelay(GRIP_INIT_DELAY);
	}

	for (i = 0; i < 16; i++)            /* Wait for multiport to settle */
		udelay(GRIP_INIT_DELAY);

	while (tries < 64 && bads < 8) {    /* Reset multiport and try getting a packet */

		flags = multiport_io(gameport, IO_RESET, 0x27, packet);

		if (flags & IO_MODE_FAST)
			return 1;

		if (flags & IO_RETRY)
			tries++;
		else
			bads++;
	}
	return 0;
}

/*
 * Packet structure: B0-B15   => gamepad state
 *                   B16-B20  => gamepad device type
 *                   B21-B24  => multiport slot index (1-4)
 *
 * Known device types: 0x1f (grip pad), 0x0 (no device).  Others may exist.
 *
 * Returns the packet status.
 */

static int get_and_decode_packet(struct grip_mp *grip, int flags)
{
	struct grip_port *port;
	u32 packet;
	int joytype = 0;
	int slot;

	/* Get a packet and check for validity */

	flags &= IO_RESET | IO_RETRY;
	flags = multiport_io(grip->gameport, flags, 0, &packet);
	grip->reads++;

	if (packet & PACKET_MP_DONE)
		flags |= IO_DONE;

	if (flags && !(flags & IO_GOT_PACKET)) {
		grip->bads++;
		return flags;
	}

	/* Ignore non-gamepad packets, e.g. multiport hardware version */

	slot = ((packet >> 21) & 0xf) - 1;
	if ((slot < 0) || (slot > 3))
		return flags;

	port = grip->port[slot];

	/*
	 * Handle "reset" packe