 any */

	/* Some USB-IDE converter chips need a 100us delay between the
	 * command phase and the data phase.  Some devices need a little
	 * more than that, probably because of clock rate inaccuracies. */
	if (unlikely(us->fflags & US_FL_GO_SLOW))
		udelay(125);

	if (transfer_length) {
		unsigned int pipe = srb->sc_data_direction == DMA_FROM_DEVICE ? 
				us->recv_bulk_pipe : us->send_bulk_pipe;
		result = usb_stor_bulk_srb(us, pipe, srb);
		US_DEBUGP("Bulk data transfer result 0x%x\n", result);
		if (result == USB_STOR_XFER_ERROR)
			return USB_STOR_TRANSPORT_ERROR;

		/* If the device tried to send back more data than the
		 * amount requested, the spec requires us to transfer
		 * the CSW anyway.  Since there's no point retrying the
		 * the command, we'll return fake sense data indicating
		 * Illegal Request, Invalid Field in CDB.
		 */
		if (result == USB_STOR_XFER_LONG)
			fake_sense = 1;
	}

	/* See flow chart on pg 15 of the Bulk Only Transport spec for
	 * an explanation of how this code works.
	 */

	/* get CSW for device status */
	US_DEBUGP("Attempting to get CSW...\n");
	result = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe,
				bcs, US_BULK_CS_WRAP_LEN, &cswlen);

	/* Some broken devices add unnecessary zero-length packets to the
	 * end of their data transfers.  Such packets show up as 0-length
	 * CSWs.  If we encounter such a thing, try to read the CSW again.
	 */
	if (result == USB_STOR_XFER_SHORT && cswlen == 0) {
		US_DEBUGP("Received 0-length CSW; retrying...\n");
		result = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe,
				bcs, US_BULK_CS_WRAP_LEN, &cswlen);
	}

	/* did the attempt to read the CSW fail? */
	if (result == USB_STOR_XFER_STALLED) {

		/* get the status again */
		US_DEBUGP("Attempting to get CSW (2nd try)...\n");
		result = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe,
				bcs, US_BULK_CS_WRAP_LEN, NULL);
	}

	/* if we still have a failure at this point, we're in trouble */
	US_DEBUGP("Bulk status result = %d\n", result);
	if (result != USB_STOR_XFER_GOOD)
		return USB_STOR_TRANSPORT_ERROR;

	/* check bulk status */
	residue = le32_to_cpu(bcs->Residue);
	US_DEBUGP("Bulk Status S 0x%x T 0x%x R %u Stat 0x%x\n",
			le32_to_cpu(bcs->Signature), bcs->Tag, 
			residue, bcs->Status);
	if (!(bcs->Tag == us->tag || (us->fflags & US_FL_BULK_IGNORE_TAG)) ||
		bcs->Status > US_BULK_STAT_PHASE) {
		US_DEBUGP("Bulk logical error\n");
		return USB_STOR_TRANSPORT_ERROR;
	}

	/* Some broken devices report odd signatures, so we do not check them
	 * for validity against the spec. We store the first one we see,
	 * and check subsequent transfers for validity against this signature.
	 */
	if (!us->bcs_signature) {
		us->bcs_signature = bcs->Signature;
		if (us->bcs_signature != cpu_to_le32(US_BULK_CS_SIGN))
			US_DEBUGP("Learnt BCS signature 0x%08X\n",
					le32_to_cpu(us->bcs_sig