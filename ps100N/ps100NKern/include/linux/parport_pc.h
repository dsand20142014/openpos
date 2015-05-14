ct sl811		*sl811,
	struct sl811h_ep	*ep,
	struct urb		*urb,
	u8			bank,
	u8			control
)
{
	u8			addr;
	u8			len;
	void __iomem		*data_reg;

	addr = SL811HS_PACKET_BUF(bank == 0);
	len = sizeof(struct usb_ctrlrequest);
	data_reg = sl811->data_reg;
	sl811_write_buf(sl811, addr, urb->setup_packet, len);

	/* autoincrementing */
	sl811_write(sl811, bank + SL11H_BUFADDRREG, addr);
	writeb(len, data_reg);
	writeb(SL_SETUP /* | ep->epnum */, data_reg);
	writeb(usb_pipedevice(urb->pipe), data_reg);

	/* always OUT/data0 */ ;
	sl811_write(sl811, bank + SL11H_HOSTCTLREG,
			control | SL11H_HCTLMASK_OUT);
	ep->length = 0;
	PACKET("SETUP qh%p\n", ep);
}

/* STATUS finishes control requests, often after IN or OUT data packets */
static void status_packet(
	struct sl811		*sl811,
	struct sl811h_ep	*ep,
	struct urb		*urb,
	u8			bank,
	u8			control
)
{
	int			do_out;
	void __iomem		*data_reg;

	do_out = urb->transfer_buffer_length && usb_pipein(urb->pipe);
	data_reg = sl811->data_reg;

	/* autoincrementing */
	sl811_write(sl811, bank + SL11H_BUFADDRREG, 0);
	writeb(0, data_reg);
	writeb((do_out ? SL_OUT : SL_IN) /* | ep->epnum */, data_reg);
	writeb(usb_pipedevice(urb->pipe), data_reg);

	/* always data1; sometimes IN */
	control |= SL11H_HCTLMASK_TOGGLE;
	if (do_out)
		control |= SL11H_HCTLMASK_OUT;
	sl811_write(sl811, bank + SL11H_HOSTCTLREG, control);
	ep->length = 0;
	PACKET("STATUS%s/%s qh%p\n", ep->nak_count ? "/retry" : "",
			do_out ? "out" : "in", ep);
}

/* IN packets can be used with any type of endpoint. here we just
 * start the transfer, data from the peripheral may arrive later.
 * urb->iso_frame_desc is currently ignored here...
 */
static void in_packet(
	struct sl811		*sl811,
	struct sl811h_ep	*ep,
	struct urb		*urb,
	u8			bank,
	u8			control
)
{
	u8			addr;
	u8			len;
	void __iomem		*data_reg;

	/* avoid losing data on overflow */
	len = ep->maxpacket;
	addr = SL811HS_PACKET_BUF(bank == 0);
	if (!(control & SL11H_HCTLMASK_ISOCH)
			&& usb_gettoggle(urb->dev, ep->epnum, 0))
		control |= SL11H_HCTLMASK_TOGGLE;
	data_reg = sl811->data_reg;

	/* autoincrementing */
	sl811_write(sl811, bank + SL11H_BUFADDRREG, addr);
	writeb(len, data_reg);
	writeb(SL_IN | ep->epnum, data_reg);
	writeb(usb_pipedevice(urb->pipe), data_reg);

	sl811_write(sl811, bank + SL11H_HOSTCTLREG, control);
	ep->length = min_t(u32, len,
			urb->transfer_buffer_length - urb->actual_length);
	PACKET("IN%s/%d qh%p len%d\n", ep->nak_count ? "/retry" : "",
			!!usb_gettoggle(urb->dev, ep->epnum, 0), ep, len);
}

/* OUT packets can be used with any type of endpoint.
 * urb->iso_frame_desc is currently ignored here...
 */
static void out_packet(
	struct sl811		*sl811,
	struct sl811h_ep	*ep,
	struct urb		*urb,
	u8			bank,
	u8			control
)
{
	void			*buf;
	u8			addr;
	u8			len;
	void __iomem		*data_reg;

	buf = urb->transfer_buffer + urb->actual_length;
	prefetch(buf);

	len = min_t(u32, ep->maxpacket,
			urb->transfer_buffer_length - urb->actual_length);

	if (!(control & SL11H_HCTLMASK_ISOCH)
			&& usb_gettoggle(urb->dev, ep->epnum, 1))
		control |= SL11H_HCTLMASK_TOGGLE;
	addr = SL811HS_PACKET_BUF(bank == 0);
	data_reg = sl811->data_reg;

	sl811_write_buf(sl811, addr, buf, len);

	/* autoincrementing */
	sl811_write(sl811, bank + SL11H_BUFADDRREG, addr);
	writeb(len, data_reg);
	writeb(SL_OUT | ep->epnum, data_reg);
	writeb(usb_pipedevice(urb->pipe), data_reg);

	sl811_write(sl811, bank + SL11H_HOSTCTLREG,
			control | SL11H_HCTLMASK_OUT);
	ep->length = len;
	PACKET("OUT%s/%d qh%p len%d\n", ep->nak_count ? "/retry" : "",
			!!usb_gettoggle(urb->dev, ep->epnum, 1), ep, len);
}

/*-------------------------------------------------------------------------*/

/* caller updates on-chip enables later */

static inline void sofirq_on(struct sl811 *sl811)
{
	if (sl811->irq_enable & SL11H_INTMASK_SOFINTR)
		return;
	VDBG("sof irq on\n");
	sl811->irq_enable |= SL11H_INTMASK_SOFINTR;
}

static inline void sofirq_off(struct sl811 *sl811)
{
	if (!(sl811->irq_enable & SL11H_INTMASK_SOFINTR))
		return;
	VDBG("sof irq off\n");
	sl811->irq_enable &= ~SL11H_INTMASK_SOFINTR;
}

/*-------------------------------------------------------------------------*/

/* pick the next endpoint for a transaction, and issue it.
 * frames start with periodic transfers (after whatever is pending
 * from the previous frame), and the rest of the time is async
 * transfers, scheduled round-robin.
 */
static struct sl811h_ep	*start(struct sl811 *sl811, u8 bank)
{
	struct sl811h_ep	*ep;
	struct urb		*urb;
	int			fclock;
	u8			control;

	/* use endpoint at schedule head */
	if (sl811->next_periodic) {
		ep = sl811->next_periodic;
		sl811->next_periodic = ep->next;
	} else {
		if (sl811->next_async)
			ep = sl811->next_async;
		else if (!list_empty(&sl811->async))
			ep = container_of(sl811->async.next,
					struct sl811h_ep, schedule);
		else {
			/* could set up the first fullspeed periodic
			 * transfer for the next frame ...
			 */
			return NULL;
		}

#ifdef USE_B
		if ((bank && sl811->active_b == ep) || sl811->active_a == ep)
			return NULL;
#endif

		if (ep->schedule.next == &sl811->async)
			sl811->next_async = NULL;
		else
			sl811->next_async = container_of(ep->schedule.next,
					struct sl811h_ep, schedule);
	}

	if (unlikely(list_empty(&ep->hep->urb_list))) {
		DBG("empty %p queue?\n", ep);
		return NULL;
	}

	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);
	control = ep->defctrl;

	/* if this frame doesn't have enough time left to transfer this
	 * packet, wait till the next frame.  too-simple algorithm...
	 */
	fclock = sl811_read(sl811, SL11H_SOFTMRREG) << 6;
	fclock -= 100;		/* setup takes not much time */
	if (urb->dev->speed == USB_SPEED_LOW) {
		if (control & SL11H_HCTLMASK_PREAMBLE) {
			/* also note erratum 1: some hubs won't work */
			fclock -= 800;
		}
		fclock -= ep->maxpacket << 8;

		/* erratum 2: AFTERSOF only works for fullspeed */
		if (fclock < 0) {
			if (ep->period)
				sl811->stat_overrun++;
			sofirq_on(sl811);
			return NULL;
		}
	} else {
		fclock -= 12000 / 19;	/* 19 64byte packets/msec */
		if (fclock < 0) {
			if (ep->period)
				sl811->stat_overrun++;
			control |= SL11H_HCTLMASK_AFTERSOF;

		/* throttle bulk/control irq noise */
		} else if (ep->nak_count)
			control |= SL11H_HCTLMASK_AFTERSOF;
	}


	switch (ep->nextpid) {
	case USB_PID_IN:
		in_packet(sl811, ep, urb, bank, control);
		break;
	case USB_PID_OUT:
		out_packet(sl811, ep, urb, bank, control);
		break;
	case USB_PID_SETUP:
		setup_packet(sl811, ep, urb, bank, control);
		break;
	case USB_PID_ACK:		/* for control status */
		status_packet(sl811, ep, urb, bank, control);
		break;
	default:
		DBG("bad ep%p pid %