s), CONTEXT_RUN | extra);
	flush_writes(ohci);
}

static void context_append(struct context *ctx,
			   struct descriptor *d, int z, int extra)
{
	dma_addr_t d_bus;
	struct descriptor_buffer *desc = ctx->buffer_tail;

	d_bus = desc->buffer_bus + (d - desc->buffer) * sizeof(*d);

	desc->used += (z + extra) * sizeof(*d);
	ctx->prev->branch_address = cpu_to_le32(d_bus | z);
	ctx->prev = find_branch_descriptor(d, z);

	reg_write(ctx->ohci, CONTROL_SET(ctx->regs), CONTEXT_WAKE);
	flush_writes(ctx->ohci);
}

static void context_stop(struct context *ctx)
{
	u32 reg;
	int i;

	reg_write(ctx->ohci, CONTROL_CLEAR(ctx->regs), CONTEXT_RUN);
	flush_writes(ctx->ohci);

	for (i = 0; i < 10; i++) {
		reg = reg_read(ctx->ohci, CONTROL_SET(ctx->regs));
		if ((reg & CONTEXT_ACTIVE) == 0)
			return;

		mdelay(1);
	}
	fw_error("Error: DMA context still active (0x%08x)\n", reg);
}

struct driver_data {
	struct fw_packet *packet;
};

/*
 * This function apppends a packet to the DMA queue for transmission.
 * Must always be called with the ochi->lock held to ensure proper
 * generation handling and locking around packet queue manipulation.
 */
static int at_context_queue_packet(struct context *ctx,
				   struct fw_packet *packet)
{
	struct fw_ohci *ohci = ctx->ohci;
	dma_addr_t d_bus, uninitialized_var(payload_bus);
	struct driver_data *driver_data;
	struct descriptor *d, *last;
	__le32 *header;
	int z, tcode;
	u32 reg;

	d = context_get_descriptors(ctx, 4, &d_bus);
	if (d == NULL) {
		packet->ack = RCODE_SEND_ERROR;
		return -1;
	}

	d[0].control   = cpu_to_le16(DESCRIPTOR_KEY_IMMEDIATE);
	d[0].res_count = cpu_to_le16(packet->timestamp);

	/*
	 * The DMA format for asyncronous link packets is different
	 * from the IEEE1394 layout, so shift the fields around
	 * accordingly.  If header_length is 8, it's a PHY packet, to
	 * which we need to prepend an extra quadlet.
	 */

	header = (__le32 *) &d[1];
	switch (packet->header_length) {
	case 16:
	case 12:
		header[0] = cpu_to_le32((packet->header[0] & 0xffff) |
					(packet->speed << 16));
		header[1] = cpu_to_le32((packet->header[1] & 0xffff) |
					(packet->header[0] & 0xffff0000));
		header[2] = cpu_to_le32(packet->header[2]);

		tcode = (packet->header[0] >> 4) & 0x0f;
		if (TCODE_IS_BLOCK_PACKET(tcode))
			header[3] = cpu_to_le32(packet->header[3]);
		else
			header[3] = (__force __le32) packet->header[3];

		d[0].req_count = cpu_to_le16(packet->header_length);
		break;

	case 8:
		header[0] = cpu_to_le32((OHCI1394_phy_tcode << 4) |
					(packet->speed << 16));
		header[1] = cpu_to_le32(packet->header[0]);
		header[2] = cpu_to_le32(packet->header[1]);
		d[0].req_count = cpu_to_le16(12);
		break;

	case 4:
		header[0] = cpu_to_le32((packet->header[0] & 0xffff) |
					(packet->speed << 16));
		header[1] = cpu_to_le32(packet->header[0] & 0xffff0000);
		d[0].req_count = cpu_to_le16(8);
		break;

	default:
		/* BUG(); */
		packet->ack = RCODE_SEND_ERROR;
		return -1;
	}

	driver_data = (struct driver_data *) &d[3];
	driver_data->packet = packet;
	packet->driver_data = driver_data;

	if (packet->payload_length > 0) {
		payload_bus =
			dma_map_single(ohci->card.device, packet->payload,
				       packet->payload_length, DMA_TO_DEVICE);
		if (dma_mapping_error(ohci->card.device, payload_bus)) {
			packet->ack = RCODE_SEND_ERROR;
			return -1;
		}
		packet->payload_bus = payload_bus;

		d[2].req_count    = cpu_to_le16(packet->payload_length);
		d[2].data_address = cpu_to_le32(payload_bus);
		last = &d[2];
		z = 3;
	} else {
		last = &d[0];
		z = 2;
	}

	last->control |= cpu_to_le16(DESCRIPTOR_OUTPUT_LAST |
				     DESCRIPTOR_IRQ_ALWAYS |
				     DESCRIPTOR_BRANCH_ALWAYS);

	/*
	 * If the controller and packet generations don't match, we need to
	 * bail out and try again.  If IntEvent.busReset is set, the AT context
	 * is halted, so appending to the context and trying to run it is
	 * futile.  Most controllers do the right thing and just flush the AT
	 * queue (per section 7.2.3.2 of the OHCI 1.1 specification), but
	 * some controllers (like a JMicron JMB381 PCI-e) misbehave and wind
	 * up stalling out.  So we just bail out in software and try again
	 * later, and everyone is happy.
	 * FIXME: Document how the locking works.
	 */
	if (ohci->generation != packet->generation ||
	    reg_read(ohci, OHCI1394_IntEventSet) & OHCI1394_busReset) {
		if (packet->payload_length > 0)
			dma_unmap_single(ohci->card.device, payload_bus,
					 packet->payload_length, DMA_TO_DEVICE);
		packet->ack = RCODE_GENERATION;
		return -1;
	}

	context_append(ctx, d, z, 4 - z);

	/* If the context isn't already running, start it up. */
	reg = reg_read(ctx->ohci, CONTROL_SET(ctx->regs));
	if ((reg & CONTEXT_RUN) == 0)
		context_run(ctx, 0);

	return 0;
}

static int handle_at_packet(struct context *context,
			    struct descriptor *d,
			    struct descriptor *last)
{
	struct driver_data *driver_data;
	struct fw_packet *packet;
	struct fw_ohci *ohci = context->ohci;
	int evt;

	if (last->transfer_status == 0)
		/* This descriptor isn't done yet, stop iteration. */
		return 0;

	driver_data = (struct driver_data *) &d[3];
	packet = driver_data->packet;
	if (packet == NULL)
		/* This packet was cancelled, just continue. */
		return 1;

	if (packet->payload_bus)
		dma_unmap_single(ohci->card.device, packet->payload_bus,
				 packet->payload_length, DMA_TO_DEVICE);

	evt = le16_to_cpu(last->transfer_status) & 0x1f;
	packet->timestamp = le16_to_cpu(last->res_count);

	log_ar_at_event('T', packet->speed, packet->header, evt);

	switch (evt) {
	case OHCI1394_evt_timeout:
		/* Async response transmit timed out. */
		packet->ack = RCODE_CANCELLED;
		break;

	case OHCI1394_evt_flushed:
		/*
		 * The packet was flushed should give same error as
		 * when we try to use a stale generation count.
		 */
		packet->ack = RCODE_GENERATION;
		break;

	case OHCI1394_evt_missing_ack:
		/*
		 * Using a valid (current) generation count, but the
		 * node is not on the bus or not sending acks.
		 */
		packet->ack = RCODE_NO_ACK;
		break;

	case ACK_COMPLETE + 0x10:
	case ACK_PENDING + 0x10:
	case ACK_BUSY_X + 0x10:
	case ACK_BUSY_A + 0x10:
	case ACK_BUSY_B + 0x10:
	case ACK_DATA_ERROR + 0x10:
	case ACK_TYPE_ERROR + 0x10:
		packet->ack = evt - 0x10;
		break;

	default:
		packet->ack = RCODE_SEND_ERROR;
		break;
	}

	packet->callback(packet, &ohci->card, packet->ack);

	return 1;
}

#define HEADER_GET_DESTINATION(q)	(((q) >> 16) & 0xffff)
#define HEADER_GET_TCODE(q)		(((q) >> 4) & 0x0f)
#define HEADER_GET_OFFSET_HIGH(q)	(((q) >> 0) & 0xffff)
#define HEADER_GET_DATA_LENGTH(q)	(((q) >> 16) & 0xffff)
#define HEADER_GET_EXTENDED_TCODE(q)	(((q) >> 0) & 0xffff)

static void handle_local_rom(struct fw_ohci *ohci,
			     struct fw_packet *packet, u32 csr)
{
	struct fw_packet response;
	int tcode, length, i;

	tcode = HEADER_GET_TCODE(packet->header[0]);
	if (TCODE_IS_BLOCK_PACKET(tcode))
		length = HEADER_GET_DATA_LENGTH(packet->header[3]);
	else
		length = 4;

	i = csr - CSR_CONFIG_ROM;
	if (i + length > CONFIG_ROM_SIZE) {
		fw_fill_response(&response, packet->header,
				 RCODE_ADDRESS_ERROR, NULL, 0);
	} else if (!TCODE_IS_READ_REQUEST(tcode)) {
		fw_fill_response(&response, packet->header,
				 RCODE_TYPE_ERROR, NULL, 0);
	} else {
		fw_fill_response(&response, packet->header, RCODE_COMPLETE,
				 (void *) ohci->config_rom + i, length);
	}

	fw_core_handle_response(&ohci->card, &response);
}

static void handle_local_lock(struct fw_ohci *ohci,
			      struct fw_packet *packet, u32 csr)
{
	struct fw_packet response;
	int tcode, length, ext_tcode, sel;
	__be32 *payload, lock_old;
	u32 lock_arg, lock_data;

	tcode = HEADER_GET_TCODE(packet->header[0]);
	length = HEADER_GET_DATA_LENGTH(packet->header[3]);
	payload = packet->payload;
	ext_tcode = HEADER_GET_EXTENDED_TCODE(packet->header[3]);

	if (tcode == TCODE_LOCK_REQUEST &&
	    ext_tcode == EXTCODE_COMPARE_SWAP && length == 8) {
		lock_arg = be32_to_cpu(payload[0]);
		lock_data = be32_to_cpu(payload[1]);
	} else if (tcode == TCODE_READ_QUADLET_REQUEST) {
		lock_arg = 0;
		lock_data = 0;
	} else {
		fw_fill_response(&response, packet->header,
				 RCODE_TYPE_ERROR, NULL, 0);
		goto out;
	}

	sel = (csr - CSR_BUS_MANAGER_ID) / 4;
	reg_write(ohci, OHCI1394_CSRData, lock_data);
	reg_write(ohci, OHCI1394_CSRCompareData, lock_arg);
	reg_write(ohci, OHCI1394_CSRControl, sel);

	if (reg_read(ohci, OHCI1394_CSRControl) & 0x80000000)
		lock_old = cpu_to_be32(reg_read(ohci, OHCI1394_CSRData));
	else
		fw_notify("swap not done yet\n");

	fw_fill_response(&response, packet->header,
			 RCODE_COMPLETE, &lock_old, sizeof(lock_old));
 out:
	fw_core_handle_response(&ohci->card, &response);
}

static void handle_local_request(struct context *ctx, struct fw_packet *packet)
{
	u64 offset;
	u32 csr;

	if (ctx == &ctx->ohci->at_request_ctx) {
		packet->ack = ACK_PENDING;
		packet->callback(packet, &ctx->ohci->card, packet->ack);
	}

	offset =
		((unsigned long long)
		 HEADER_GET_OFFSET_HIGH(packet->header[1]) << 32) |
		packet->header[2];
	csr = offset - CSR_REGISTER_BASE;

	/* Handle config rom reads. */
	if (csr >= CSR_CONFIG_ROM && csr < CSR_CONFIG_ROM_END)
		handle_local_rom(ctx->ohci, packet, csr);
	else switch (csr) {
	case CSR_