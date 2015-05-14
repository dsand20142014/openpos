r =
	    container_of(resource, struct address_handler_resource, resource);

	fw_core_remove_address_handler(&r->handler);
	kfree(r);
}

static int ioctl_allocate(struct client *client, void *buffer)
{
	struct fw_cdev_allocate *request = buffer;
	struct address_handler_resource *r;
	struct fw_address_region region;
	int ret;

	r = kmalloc(sizeof(*r), GFP_KERNEL);
	if (r == NULL)
		return -ENOMEM;

	region.start = request->offset;
	region.end = request->offset + request->length;
	r->handler.length = request->length;
	r->handler.address_callback = handle_request;
	r->handler.callback_data = r;
	r->closure = request->closure;
	r->client = client;

	ret = fw_core_add_address_handler(&r->handler, &region);
	if (ret < 0) {
		kfree(r);
		return ret;
	}

	r->resource.release = release_address_handler;
	ret = add_client_resource(client, &r->resource, GFP_KERNEL);
	if (ret < 0) {
		release_address_handler(client, &r->resource);
		return ret;
	}
	request->handle = r->resource.handle;

	return 0;
}

static int ioctl_deallocate(struct client *client, void *buffer)
{
	struct fw_cdev_deallocate *request = buffer;

	return release_client_resource(client, request->handle,
				       release_address_handler, NULL);
}

static int ioctl_send_response(struct client *client, void *buffer)
{
	struct fw_cdev_send_response *request = buffer;
	struct client_resource *resource;
	struct inbound_transaction_resource *r;

	if (release_client_resource(client, request->handle,
				    release_request, &resource) < 0)
		return -EINVAL;

	r = container_of(resource, struct inbound_transaction_resource,
			 resource);
	if (request->length < r->length)
		r->length = request->length;
	if (copy_from_user(r->data, u64_to_uptr(request->data), r->length))
		return -EFAULT;

	fw_send_response(client->device->card, r->request, request->rcode);
	kfree(r);

	return 0;
}

static int ioctl_initiate_bus_reset(struct client *client, void *buffer)
{
	struct fw_cdev_initiate_bus_reset *request = buffer;
	int short_reset;

	short_reset = (request->type == FW_CDEV_SHORT_RESET);

	return fw_core_initiate_bus_reset(client->device->card, short_reset);
}

static void release_descriptor(struct client *client,
			       struct client_resource *resource)
{
	struct descriptor_resource *r =
		container_of(resource, struct descriptor_resource, resource);

	fw_core_remove_descriptor(&r->descriptor);
	kfree(r);
}

static int ioctl_add_descriptor(struct client *client, void *buffer)
{
	struct fw_cdev_add_descriptor *request = buffer;
	struct descriptor_resource *r;
	int ret;

	/* Access policy: Allow this ioctl only on local nodes' device files. */
	if (!client->device->is_local)
		return -ENOSYS;

	if (request->length > 256)
		return -EINVAL;

	r = kmalloc(sizeof(*r) + request->length * 4, GFP_KERNEL);
	if (r == NULL)
		return -ENOMEM;

	if (copy_from_user(r->data,
			   u64_to_uptr(request->data), request->length * 4)) {
		ret = -EFAULT;
		goto failed;
	}

	r->descriptor.length    = request->length;
	r->descriptor.immediate = request->immediate;
	r->descriptor.key       = request->key;
	r->descriptor.data      = r->data;

	ret = fw_core_add_descriptor(&r->descriptor);
	if (ret < 0)
		goto failed;

	r->resource.release = release_descriptor;
	ret = add_client_resource(client, &r->resource, GFP_KERNEL);
	if (ret < 0) {
		fw_core_remove_descriptor(&r->descriptor);
		goto failed;
	}
	request->handle = r->resource.handle;

	return 0;
 failed:
	kfree(r);

	return ret;
}

static int ioctl_remove_descriptor(struct client *client, void *buffer)
{
	struct fw_cdev_remove_descriptor *request = buffer;

	return release_client_resource(client, request->handle,
				       release_descriptor, NULL);
}

static void iso_callback(struct fw_iso_context *context, u32 cycle,
			 size_t header_length, void *header, void *data)
{
	struct client *client = data;
	struct iso_interrupt_event *e;

	e = kzalloc(sizeof(*e) + header_length, GFP_ATOMIC);
	if (e == NULL)
		return;

	e->interrupt.type      = FW_CDEV_EVENT_ISO_INTERRUPT;
	e->interrupt.closure   = client->iso_closure;
	e->interrupt.cycle     = cycle;
	e->interrupt.header_length = header_length;
	memcpy(e->interrupt.header, header, header_length);
	queue_event(client, &e->event, &e->interrupt,
		    sizeof(e->interrupt) + header_length, NULL, 0);
}

static int ioctl_create_iso_context(struct client *client, void *buffer)
{
	struct fw_cdev_create_iso_context *request = buffer;
	struct fw_iso_context *context;

	/* We only support one context at this time. */
	if (client->iso_context != NULL)
		return -EBUSY;

	if (request->channel > 63)
		return -EINVAL;

	switch (request->type) {
	case FW_ISO_CONTEXT_RECEIVE:
		if (request->header_size < 4 || (request->header_size & 3))
			return -EINVAL;

		break;

	case FW_ISO_CONTEXT_TRANSMIT:
		if (request->speed > SCODE_3200)
			return -EINVAL;

		break;

	default:
		return -EINVAL;
	}

	context =  fw_iso_context_create(client->device->card,
					 request->type,
					 request->channel,
					 request->speed,
					 request->header_size,
					 iso_callback, client);
	if (IS_ERR(context))
		return PTR_ERR(context);

	client->iso_closure = request->closure;
	client->iso_context = context;

	/* We only support one context at this time. */
	request->handle = 0;

	return 0;
}

/* Macros for decoding the iso packet control header. */
#define GET_PAYLOAD_LENGTH(v)	((v) & 0xffff)
#define GET_INTERRUPT(v)	(((v) >> 16) & 0x01)
#define GET_SKIP(v)		(((v) >> 17) & 0x01)
#define GET_TAG(v)		(((v) >> 18) & 0x03)
#define GET_SY(v)		(((v) >> 20) & 0x0f)
#define GET_HEADER_LENGTH(v)	(((v) >> 24) & 0xff)

static int ioctl_queue_iso(struct client *client, void *buffer)
{
	struct fw_cdev_queue_iso *request = buffer;
	struct fw_cdev_iso_packet __user *p, *end, *next;
	struct fw_iso_context *ctx = client->iso_context;
	unsigned long payload, buffer_end, header_length;
	u32 control;
	int count;
	struct {
		struct fw_iso_packet packet;
		u8 header[256];
	} u;

	if (ctx == NULL || request->handle != 0)
		return -EINVAL;

	/*
	 * If the user passes a non-NULL data pointer, has mmap()'ed
	 * the iso buffer, and the pointer points inside the buffer,
	 * we setup the payload pointers accordingly.  Otherwise we
	 * set them both to 0, which will still let packets with
	 * payload_length == 0 through.  In other words, if no packets
	 * use the indirect payload, the iso buffer need not be mapped
	 * and the request->data pointer is ignored.
	 */

	payload = (unsigned long)request->data - client->vm_start;
	buffer_end = client->buffer.page_count << PAGE_SHIFT;
	if (request->data == 0 || client->buffer.pages == NULL ||
	    payload >= buffer_end) {
		payload = 0;
		buffer_end = 0;
	}

	p = (struct fw_cdev_iso_packet __user *)u64_to_uptr(request->packets);

	if (!access_ok(VERIFY_READ, p, request->size))
		return -EFAULT;

	end = (void __user *)p + request->size;
	count = 0;
	while (p < end) {
		if (get_user(control, &p->control))
			return -EFAULT;
		u.packet.payload_length = GET_PAYLOAD_LENGTH(control);
		u.packet.interrupt = GET_INTERRUPT(control);
		u.packet.skip = GET_SKIP(control);
		u.packet.tag = GET_TAG(control);
		u.packet.sy = GET_SY(control);
		u.packet.header_length = GET_HEADER_LENGTH(control);

		if (ctx->type == FW_ISO_CONTEXT_TRANSMIT) {
			header_length = u.packet.header_length;
		} else {
			/*
			 * We require that header_length is a multiple of
			 * the fixed header size, ctx->header_size.
			 */
			if (ctx->header_size == 0) {
				if (u.packet.header_length > 0)
					return -EINVAL;
			} else if (u.packet.header_length % ctx->header_size != 0) {
				return -EINVAL;
			}
			header_length = 0;
		}

		next = (struct fw_cdev_iso_packet __user *)
			&p->header[header_length / 4];
		if (next > end)
			return -EINVAL;
		if (__copy_from_user
		    (u.packet.header, p->header, header_length))
			return -EFAULT;
		if (u.packet.skip && ctx->type == FW_ISO_CONTEXT_TRANSMIT &&
		    u.packet.header_length + u.packet.payload_length > 0)
			return -EINVAL;
		if (payload + u.packet.payload_length > buffer_end)
			return -EINVAL;

		if (fw_iso_context_queue(ctx, &u.packet,
					 &client->buffer, payload))
			break;

		p = next;
		payload += u.packet.payload_length;
		count++;
	}

	request->size    -= uptr_to_u64(p) - request->packets;
	request->packets  = uptr_to_u64(p);
	request->data     = client->vm_start + payload;

	return count;
}

static int ioctl_start_iso(struct client *client, void *buffer)
{
	struct fw_cdev_start_iso *request = buffer;

	if (client->iso_context == NULL || request->handle != 0)
		return -EINVAL;

	if (client->iso_context->type == FW_ISO_CONTEXT_RECEIVE) {
		if (request->tags == 0 || request->tags > 15)
			return -EINVAL;

		if (request->sync > 15)
			return -EINVAL;
	}

	return fw_iso_context_start(client->iso_context, request->cycle,
				    request->sync, request->tags);
}

static int ioctl_stop_iso(struct client *client, void *buffer)
{
	struct fw_cdev_stop_iso *request = buffer;

	if (client->iso_context == NULL || request->handle != 0)
		return -EINVAL;

	return fw_iso_context_stop(client->iso_context);
}

static int ioctl_get_cycle_timer(struct client *client, void *buffer)
{
	struct fw_cdev_get_cycle_timer *request = buffer;
	struct fw_card *card = client->device->card;
	unsigned long long bus_time;
	struct timeval tv;
	unsigned long flags;

	preempt_disable();
	local_irq_save(flags);

	bus_time = card->driver->get_