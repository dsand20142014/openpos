, WAITING_TID_TAIL);
			if (waiting_t == waiting_h) {
				ahd_outw(ahd, WAITING_TID_TAIL, scbid);
				next = SCB_LIST_NULL;
			} else {
				ahd_set_scbptr(ahd, waiting_h);
				next = ahd_inw_scbram(ahd, SCB_NEXT2);
			}
			ahd_set_scbptr(ahd, scbid);
			ahd_outw(ahd, SCB_NEXT2, next);
		}
		ahd_set_scbptr(ahd, saved_scbptr);
		if (scb->crc_retry_count < AHD_MAX_LQ_CRC_ERRORS) {
			if (SCB_IS_SILENT(scb) == FALSE) {
				ahd_print_path(ahd, scb);
				printf("Probable outgoing LQ CRC error.  "
				       "Retrying command\n");
			}
			scb->crc_retry_count++;
		} else {
			ahd_set_transaction_status(scb, CAM_UNCOR_PARITY);
			ahd_freeze_scb(scb);
			ahd_freeze_devq(ahd, scb);
		}
		/* Return unpausing the sequencer. */
		return (0);
	} else if ((ahd_inb(ahd, PERRDIAG) & PARITYERR) != 0) {
		/*
		 * Ignore what are really parity errors that
		 * occur on the last REQ of a free running
		 * clock prior to going busfree.  Some drives
		 * do not properly active negate just before
		 * going busfree resulting in a parity glitch.
		 */
		ahd_outb(ahd, CLRSINT1, CLRSCSIPERR|CLRBUSF