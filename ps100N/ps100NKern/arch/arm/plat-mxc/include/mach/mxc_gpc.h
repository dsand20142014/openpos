st rounding up. It makes sense to optimize for
	 * memory footprint over access speed since the amount
	 * of memory available for dma may be limited.
	 */
	put_unaligned((unsigned long)*vaddr_handle,
		      (unsigned long *)(vaddr + size));

	if (dir == DMA_TO_DEVICE)
		memcpy(vaddr, *vaddr_handle, size);

	*vaddr_handle = vaddr;
	return 0;
}

static void hcd_free_coherent(struct usb_bus *bus, dma_addr_t *dma_handle,
			      void **vaddr_handle, size_t size,
			      enum dma_data_direction dir)
{
	unsigned char *vaddr = *vaddr_handle;

	vaddr = (void *)get_unaligned((unsigned long *)(vaddr + size));

	if (dir == DMA_FROM_DEVICE)
		memcpy(vaddr, *vaddr_handle, size);

	hcd_buffer_free(bus, size + sizeof(vaddr), *vaddr_handle, *dma_handle);

	*vaddr_handle = vaddr;
	*dma_handle = 0;
}

static int map_urb_for_dma(struct usb_hcd *hcd, struct urb *urb,
			   gfp_t mem_flags)
{
	enum dma_data_direction dir;
	int ret = 0;

	/* Map the URB's buffers for DMA access.
	 * Lower level HCD code should use *_dma exclusively,
	 * unless it uses pio or talks to another transport,
	 * or uses the provided scatter gather list for bulk.
	 */
	if (is_root_hub(urb->dev))
		return 0;

	if (usb_endpoint_xfer_control(&urb->ep->desc)
	    && !(urb->transfer_flags & URB_NO_SETUP_DMA_MAP)) {
		if (hcd->self.uses_dma)
			urb->setup_dma = dma_map_single(
					hcd->self.controller,
					urb->setup_packet,
					sizeof(struct usb_ctrlrequest),
					DMA_TO_DEVICE);
		else if (hcd->driver->flags & HCD_LOCAL_MEM)
			ret = hcd_alloc_coherent(
					urb->dev->bus, mem_flags,
					&urb->setup_dma,
					(void **)&urb->setup_packet,
					sizeof(struct usb_ctrlrequest),
					DMA_TO_DEVICE);
	}

	dir = usb_urb_dir_in(urb) ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
	if (ret == 0 && urb->transfer_buffer_length != 0
	    && !(urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP)) {
		if (hcd->self.uses_dma)
			urb->transfer_dma = dma_map_single (
					hcd->self.controller,
					urb->transfer_buffer,
					urb->transfer_buffer_length,
					dir);
		e