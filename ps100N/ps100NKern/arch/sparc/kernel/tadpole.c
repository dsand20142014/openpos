iss
		 * that occurs before SCSIEN falls as an incomplete
		 * transfer.
		 */
		test	SSTAT1, PHASEMIS jnz p_command_xfer_failed;
		test	DFCNTRL, SCSIEN jnz . - 1;
	} else {
		test	DFCNTRL, SCSIEN jnz .;
	}
	/*
	 * DMA Channel automatically disabled.
	 * Don't allow a data phase if the command
	 * was not fully transferred.
	 */
	test	SSTAT2, SDONE jnz ITloop;
p_command_xfer_failed:
	or	SEQ_FLAGS, NO_CDB_SENT;
	jmp	ITloop;


/*
 * Status phase.  Wait for the data byte to appear, then read it
 * and store it into the SCB.
 */
SET_SRC_MODE	M_SCSI;
SET_DST_MODE	M_SCSI;
p_status:
	test	SEQ_FLAGS,NOT_IDENTIFIED jnz mesgin_proto_violation;
p_status_okay:
	mov	SCB_SCSI_STATUS, SCSIDAT;
	or	SCB_CONTROL, STATUS_RCVD;
	jmp	ITloop;

/*
 * Message out phase.  If MSG_OUT is MSG_IDENTIFYFLAG, build a full
 * indentify message sequence and send it to the target.  The host may
 * override this behavior by setting the MK_MESSAGE bit in the SCB
 * control byte.  This will cause us to interrupt the host and allow
 * it to handle the message phase completely on its own.  If the bit
 * associated with this target is set, we will also interrupt the host,
 * thereby allowing it to send a message on the next selection regardless
 * of the transaction being sent.
 * 
 * If MSG_OUT is == HOST_MSG, also interrupt the host and take a message.
 * This is done to allow the host to send messages outside of an identify
 * sequence while protecting the seqencer from testing the MK_MESSAGE bit
 * on an SCB that might not be for the current nexus. (For example, a
 * BDR message in responce to a bad reselection would leave us pointed to
 * an SCB that doesn't have anything to do with the current target).
 *
 * Otherwise, treat MSG_OUT as a 1 byte message to send (abort, abort tag,
 * bus device reset).
 *
 * When there are no messages to send, MSG_OUT should be set to MSG_NOOP,
 * in case the target decides to put us in this phase for some strange
 * reason.
 */
p_mesgout_retry:
	/* Turn on ATN for the retry */
	mvi	SCSISIGO, ATNO;
p_mesgout:
	mov	SINDEX, MSG_OUT;
	cmp	SINDEX, MSG_IDENTIFYFLAG jne p_mesgout_from_host;
	test	SCB_CONTROL,MK_MESSAGE	jnz host_message_loop;
p_mesgout_identify:
	or	SINDEX, MSG_IDENTIFYFLAG|DISCENB, SCB_LUN;
	test	SCB_CONTROL, DISCENB jnz . + 2;
	and	SINDEX, ~DISCENB;
/*
 * Send a tag message if TAG_ENB is set in the SCB control block.
 * Use SCB_NONPACKET_TAG as the tag value.
 */
p_mesgout_tag:
	test	SCB_CONTROL,TAG_ENB jz  p_mesgout_onebyte;
	mov	SCSIDAT, SINDEX;	/* Send the identify message */
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	jne p_mesgout_done;
	and	SCSIDAT,TAG_ENB|SCB_TAG_TYPE,SCB_CONTROL;
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	jne p_mesgout_done;
	mov	SCBPTR jmp p_mesgout_onebyte;
/*
 * Interrupt the driver, and allow it to handle this message
 * phase and any required retries.
 */
p_mesgout_from_host:
	cmp	SINDEX, HOST_MSG	jne