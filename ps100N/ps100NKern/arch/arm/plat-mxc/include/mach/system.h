ptible(frame->wq,
		(frame->grabstate == FRAME_DONE)
		|| (frame->grabstate == FRAME_ERROR));

	if (rc)
		goto error;

	PDEBUG(4, "Got image, frame->grabstate = %d", frame->grabstate);
	PDEBUG(4, "bytes_recvd = %d", frame->bytes_recvd);

	if (frame->grabstate == FRAME_ERROR) {
		frame->bytes_read = 0;
		err("** ick! ** Errored frame %d", ov->curframe);
		if (ov51x_new_frame(ov, frmx)) {
			err("read: ov51x_new_frame error");
			goto error;
		}
		goto restart;
	}


	/* Repeat until we get a snapshot frame */
	if (ov->snap_enabled)
		PDEBUG(4, "Waiting snapshot frame");
	if (ov->snap_enabled && !frame->snapshot) {
		frame->bytes_read = 0;
		if ((rc = ov51x_new_frame(ov, frmx))) {
			err("read: ov51x_new_frame error");
			goto error;
		}
		goto restart;
	}

	/* Clear the snapshot */
	if (ov->snap_enabled && frame->snapshot) {
		frame->snapshot = 0;
		ov51x_clear_snapshot(ov);
	}

	/* Decompression, format conversion, etc... */
	ov51x_postprocess(ov, frame);

	PDEBUG(4, "frmx=%d, bytes_read=%ld, length=%ld", frmx,
		frame->bytes_read,
		get_frame_length