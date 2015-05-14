ideo->first_run = 1;

		/* wait until DMA really stops */
		i = 0;
		while (i < 1000) {

			/* wait 0.1 millisecond */
			udelay(100);

			if ( (reg_read(video->ohci, video->ohci_IsoXmitContextControlClear) & (1 << 10)) ||
			    (reg_read(video->ohci, video->ohci_IsoRcvContextControlClear)  & (1 << 10)) ) {
				/* still active */
				debug_printk("dv1394: stop_dma: DMA not stopped yet\n" );
				mb();
			} else {
				debug_printk("dv1394: stop_dma: DMA stopped safely after %d ms\n", i/10);
				break;
			}

			i++;
		}

		if (i == 1000) {
			printk(KERN_ERR "dv1394: stop_dma: DMA still going after %d ms!\n", i/10);
		}
	}
	else
		debug_printk("dv1394: stop_dma: already stopped.\n");

out:
	spin_unlock_irqrestore(&video->spinlock, flags);
}



static void do_dv1394_shutdown(struct video_card *video, int free_dv_buf)
{
	int i;

	debug_printk("dv1394: shutdown...\n");

	/* stop DMA if in progress */
	stop_dma(video);

	/* release the DMA contexts */
	if (video->ohci_it_ctx != -1) {
		video->ohci_IsoXmitContextControlSet = 0;
		video->ohci_IsoXmit