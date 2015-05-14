SB_BLOCK;
				if (pdword[1] >= MAX_CHANNELS)
					break;
				cc = G_chnmap[pdword[1]];
				if (!(cc >= 0 && cc < MAX_CHANNELS))
					break;
				switch (pdword[2]) {
				case S2255_RESPONSE_SETMODE:
					/* check if channel valid */
					/* set mode ready */
					dev->setmode_ready[cc] = 1;
					wake_up(&dev->wait_setmode[cc]);
					dprintk(5, "setmode ready %d\n", cc);
					break;
				case S2255_RESPONSE_FW:

					dev->chn_ready |= (1 << cc);
					if ((dev->chn_ready & 0x0f) != 0x0f)
						break;
					/* all channels ready */
					printk(KERN_INFO "s2255: fw loaded\n");
					atomic_set(&dev->fw_data->fw_state,
						   S2255_FW_SUCCESS);
					wake_up(&dev->fw_data->wait_fw);
					break;
				default:
					printk(KERN_INFO "s2255 unknwn resp\n");
				}
			default:
				pdata++;
				break;
			}
			if (bframe)
				break;
		} /* for */
		if (!bframe)
			return -EINVAL;
	}


	idx = dev->cur_frame[dev->cc];
	frm = &dev->buffer[dev->cc].frame[idx];

	/* search done.  now find out if should be acquiring on this channel */
	if (!dev->b_acquire[dev->cc]) {
		/* we found a frame, but this channel is turned off */
		frm->ulState = S2255_READ_IDLE;
		return -EINVAL;
	}

	if (frm->ulState == S2255_READ_IDLE) {
		frm->ulState = S2255_READ_FRAME;
		frm->cur_size = 0;
	}

	/* skip the marker 512 bytes (and offset if out of sync) */
	psrc = (u8 *)pipe_info->transfer_buffer + offset;


	if (frm->lpvbits == NULL) {
		dprintk(1, "s2255 frame buffer == NULL.%p %p %d %d",
			frm, dev, dev->cc, idx);
		return -ENOMEM;
	}

	pdest = frm->lpvbits + frm->cur_size;

	copy_size = (pipe_info->cur_transfer_size - offset);

	size = dev->pkt_size[dev->cc] - PREFIX_SIZE;

	/* sanity check on pdest */
	if ((copy_size + frm->cur_size) < dev->req_image_size[dev->cc])
