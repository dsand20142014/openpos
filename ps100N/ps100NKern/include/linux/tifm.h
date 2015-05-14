aders/writers/ioctl'ers */
		wake_up_interruptible(&video->waitq);
	}

out:
	spin_unlock(&video->spinlock);
}

static void ir_tasklet_func(unsigned long data)
{
	int wake = 0;
	struct video_card *video = (struct video_card*) data;

	spin_lock(&video->spinlock);

	if (!video->dma_running)
		goto out;

	if ( (video->ohci_ir_ctx != -1) &&
	    (reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) & (1 << 10)) ) {

		int sof=0; /* start-of-frame flag */
		struct frame *f;
		u16 packet_length, packet_time;
		int i, dbc=0;
		struct DMA_descriptor_block *block = NULL;
		u16 xferstatus;

		int next_i, prev_i;
		struct DMA_descriptor_block *next = NULL;
		dma_addr_t next_dma = 0;
		struct DMA_descriptor_block *prev = NULL;

		/* loop over all descriptors in all frames */
		for (i = 0; i < video->n_frames*MAX_PACKETS; i++) {
			struct packet *p = dma_region_i(&video->packet_buf, struct packet, video->current_packet);

			/* make sure we are seeing the latest changes to p */
			dma_region_sync_for_cpu(&video->packet_buf,
						(unsigned long) p - (unsigned long) video->packet_buf.kvirt,
						sizeof(struct packet));

			packet_length = le16_to_cpu(p->data_length);
			packet_time   = le16_to_cpu(p->timestamp);

			irq_printk("received packet %02d, timestamp=%04x, length=%04x, sof=%02x%02x\n", video->current_packet,
				   packet_time, packet_length,
				   p->data[0], p->data[1]);

			/* get the descriptor based on packet_buffer cursor */
			f = video->frames[video->current_packet / MAX_PACKETS];
			block = &(f->descriptor_pool[video->current_packet % MAX_PACKETS]);
			xferstatus = le32_to_cpu(block->u.in.il.q[3]) >> 16;
			xferstatus &= 0x1F;
			irq_printk("ir_tasklet_func: xferStatus/resCount [%d] = 0x%08x\n", i, le32_to_cpu(block->u.in.il.q[3]) );

			/* get the current frame */
			f = video->frames[video->active_frame];

			/* exclude empty packet */
			if (packet_length > 8 && xferstatus == 0x11) {
				/* check for start of frame */
				/* DRD> Changed to check section type ([0]>>5==0)
				   and dif sequence ([1]>>4==0) */
				sof = ( (p->data[0] >> 5) == 0 && (p->data[1] >> 4) == 0);

				dbc = (int) (p->cip_h1 >> 24);
				if ( video->continuity_counter != -1 && dbc > ((video->continuity_counter + 1) % 256) )
				{
					printk(KERN_WARNING "dv1394: discontinuity detected, dropping all frames\n" );
					video->dropped_frames += video->n_clear_frames + 1;
					video->first_frame = 0;
					video->n_clear_frames = 0;
					video->first_clear_frame = -1;
				}
				video->continuity_counter = dbc;

				if (!video->first_frame) {
					if (sof) {
						video->first_frame = 1;
					}

				} else if (sof) {
					/* close current frame */
					frame_reset(f);  /* f->state = STATE_CLEAR */
					video->n_clear_frames++;
					if (video->n_clear_frames > video->n_frames) {
						video->dropped_frames++;
						printk(KERN_WARNING "dv1394: dropped a frame during reception\n" );
						video->n_clear_frames = video->n_frames-1;
						video->first_clear_frame = (video->first_clear_frame + 1) % video->n_frames;
					}
					if (video->first_clear_frame == -1)
						video->first_clear_frame = video->active_frame;

					/* get the next frame */
					video->active_frame = (video->active_frame + 1) % video->n_frames;
					f = video->frames[video->active_frame];
					irq_printk("   frame received, active_frame = %d, n_clear_frames = %d, first_clear_frame = %d\n",
						   video->active_frame, video->n_clear_frames, video->first_clear_frame);
				}
				if (video->first_frame) {
					if (sof) {
						/* open next frame */
						f->state = FRAME_READY;
					}

					/* copy to buffer */
					if (f->n_packets > (video->frame_size / 480)) {
						printk(KERN_ERR "frame buffer overflow during receive\n");
					}

					frame_put_packet(f, p);

				} /* first_frame */
			}

			/* stop, end of ready packets */
			else if (xferstatus == 0) {
				break;
			}

			/* reset xferStatus & resCount */
			block->u.in.il.q[3] = cpu_to_le32(512);

			/* terminate dma chain at this (next) packet */
			next_i = video->current_packet;
			f = video->frames[next_i / MAX_PACKETS];
			next = &(f->descriptor_pool[next_i % MAX_PACKETS]);
			next_dma = ((unsigned long) block - (unsigned long) f->descriptor_pool) + f->descriptor_pool_dma;
			next->u.in.il.q[0] |= cpu_to_le32(3 << 20); /* enable interrupt */
			next->u.in.il.q[2] = cpu_to_le32(0); /* disable branch */

			/* link previous to next */
			prev_i = (next_i == 0) ? (MAX_PACKETS * video->n_frames - 1) : (next_i - 1);
			f = video->frames[prev_i / MAX_PACKETS];
			prev = &(f->descriptor_pool[prev_i % MAX_PACKETS]);
			if (prev_i % (MAX_PACKETS/2)) {
				prev->u.in.il.q[0] &= ~cpu_to_le32(3 << 20); /* no interrupt */
			} else {
				prev->u.in.il.q[0] |= cpu_to_le32(3 << 20); /* enable interrupt */
			}
			prev->u.in.il.q[2] = cpu_to_le32(next_dma | 1); /* set Z=1 */
			wmb();

			/* wake up DMA in case it fell asleep */
			reg_