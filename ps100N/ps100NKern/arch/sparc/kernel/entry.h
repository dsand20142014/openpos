AHC_PCI_MWI_BUG) != 0
		  && ahc->pci_cachesize != 0) {
			call calc_mwi_residual;
		}

		/* Point to the new next sg in memory */
		call	sg_advance;

sg_load_done:
		if ((ahc->features & AHC_CMD_CHAN) != 0) {
			bmov	STCNT, HCNT, 3;
		} else {
			call	set_stcnt_from_hcnt;
		}

		if ((ahc->flags & AHC_TARGETROLE) != 0) {
			test	SSTAT0, TARGET jnz data_phase_loop;
		}
	}
data_phase_finish:
	/*
	 * If the target has left us in data phase, loop through
	 * the dma code again.  In the case of ULTRA2 adapters,
	 * we should only loop if there is a data overrun.  For
	 * all other adapters, we'll loop after each S/G element
	 * is loaded as well as if there is an overrun.
	 */
	if ((ahc->flags & AHC_TARGETROLE) != 0) {
		test	SSTAT0, TARGET jnz data_phase_done;
	}
	if ((ahc->flags & AHC_INITIATORROLE) != 0) {
		test	SSTAT1, REQINIT jz .;
		if ((ahc->features & AHC_DT) == 0) {
			test	SSTAT1,PHASEMIS	jz data_phase_loop;
		} else {
			test	SCSIPHASE, DATA_PHASE_MASK jnz data_phase_loop;
		}
	}

data_phase_done:
	/*
	 * After a DMA finishes, save the SG and STCNT residuals back into
	 * the SCB.  We use STCNT instead of HCNT, since it's a reflection
	 * of how many bytes were transferred on the SCSI (as opposed to the
	 * host) bus.
	 */
	if ((ahc->features & AHC_CMD_CHAN) != 0) {
		/* Kill off any pending prefetch */
		call	disable_ccsgen;
	}

	if ((ahc->features & AHC_ULTRA2) == 0) {
		/*
		 * Clear the high address byte so that all other DMA
		 * operations, which use 32bit addressing, can assume
		 * HHADDR is 0.
		 */
		if ((ahc->flags & AHC_39BIT_ADDRESSING) != 0) {
			mov	ALLZEROS call set_hhaddr;
		}
	}

	/*
	 * Update our residual information before the information is
	 * lost by some other type of SCSI I/O (e.g. PIO).  If we have
	 * transferred all data, no update is needed.
	 *
	 */
	test	SCB_RESIDUAL_SGPTR, SG_LIST_NULL jnz residual_update_done;
	if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0
	  && ahc->pci_cachesize != 0) {
		if ((ahc->features & AHC_CMD_CHAN) != 0) {
			test	MWI_RESIDUAL, 0xFF jz bmov_resid;
		}
		mov	A, MWI_RESIDUAL;
		add	SCB_RESIDUAL_DATACNT[0], A, STCNT[0];
		clr	A;
		adc	SCB_RESIDUAL_DATACNT[1], A, STCNT[1];
		adc	SCB_RESIDUAL_DATACNT[2], A, STCNT[2];
		clr	MWI_RESIDUAL;
		if ((ahc->features & AHC_CMD_CHAN) != 0) {
			jmp	. + 2;
bmov_resid:
			bmov	SCB_RESIDUAL_DATACNT, STCNT, 3;
		}
	} else if ((ahc->features & AHC_CMD_CHAN) != 0) {
		bmov	SCB_RESIDUAL_DATACNT, STCNT, 3;
	} else {
		mov	SCB_RESIDUAL_DATACNT[0], STCNT[0];
		mov	SCB_RESIDUAL_DATACNT[1], STCNT[1];
		mov	SCB_RESIDUAL_DATACNT[2], STCNT[2];
	}
residual_update_done:
	/*
	 * Since we've been through a data phase, the SCB_RESID* fields
	 * are now initialized.  Clear the full residual flag.
	 */
	and	SCB_SGPTR[0], ~SG_FULL_RESID;

	if ((ahc->features & AHC_ULTRA2) != 0) {
		/* Clear the channel in case we return to data phase later */
		or	SXFRCTL0, CLRSTCNT|CLRCHN;
		or	SXFRCTL0, CLRSTCNT|CLRCHN;
	}

	if ((ahc->flags & AHC_TARGETROLE) != 0) {
		test	SEQ_FLAGS, DPHASE_PENDING jz ITloop;
		and	SEQ_FLAGS, ~DPHASE_PENDING;
		/*
		 * For data-in phases, wait for any pending acks from the
		 * initiator before changing phase.  We only need to
		 * send Ignore Wide Residue messages for data-in phases.
		 */
		test	DFCNTRL, DIRECTION jz target_ITloop;
		test	SSTAT1, REQINIT	jnz .;
		test	SCB_LUN, SCB_XFERLEN_ODD jz target_ITloop;
		test	SCSIRATE, WIDEXFER jz target_ITloop;
		/*
		 * Issue an Ignore Wide Residue Message.
		 */
		mvi	P_MESGIN|BSYO call change_phase;
		mvi	MSG_IGN_WIDE_RESIDUE call target_outb;
		mvi	1 call target_outb;
		jmp	target_ITloop;
	} else {
		jmp	ITloop;
	}

if ((ahc->flags & AHC_INITIATORROLE) != 0) {
/*
 * Command phase.  Set up the DMA registers and let 'er rip.
 */
p_command:
	test	SEQ_FLAGS, NOT_IDENTIFIED jz p_command_okay;
	mvi	PROTO_VIOLATION call set_seqint;
p_command_okay:

	if ((ahc->features & AHC_ULTRA2) != 0) {
		bmov	HCNT[0], SCB_CDB_LEN,  1;
		bmov	HCNT[1], ALLZEROS, 2;
		mvi	SG_CACHE_PRE, LAST_SEG;
	} else if ((ahc->features & AHC_CMD_CHAN) != 0) {
		bmov	STCNT[0], SCB_CDB_LEN, 1;
		bmov	STCNT[1], ALLZEROS, 2;
	} else {
		mov	STCNT[0], SCB_CDB_LEN;
		clr	STCNT[1];
		clr	STCNT[2];
	}
	add	NONE, -13, SCB_CDB_LEN;
	mvi	SCB_CDB_STORE jnc p_command_embedded;
p_command_from_host:
	if ((ahc->features & AHC_ULTRA2) != 0) {
		bmov	HADDR[0], SCB_CDB_PTR, 4;
		mvi	DFCNTRL, (PRELOADEN|SCSIEN|HDMAEN|DIRECTION);
	} else {
		if ((ahc->features & AHC_CMD_CHAN) != 0) {
			bmov	HADDR[0], SCB_CDB_PTR, 4;
			bmov	HCNT, STCNT, 3;
		} else {
			mvi	DINDEX, HADDR;
			mvi	SCB_CDB_PTR call bcopy_4;
			mov	SCB_CDB_LEN call set_hcnt;
		}
		mvi	DFCNTRL, (SCSIEN|SDMAEN|HDMAEN|DIRECTION|FIFORESET);
	}
	jmp	p_command_xfer;
p_command_embedded:
	/*
	 * The data fifo seems to require 4 byte aligned
	 * transfers from the sequencer.  Force this to
	 * be the case by clearing HADDR[0] even though
	 * we aren't going to touch host memory.
	 */
	clr	HADDR[0];
	if ((ahc->features & AHC_ULTRA2) != 0) {
		mvi	DFCNTRL, (PRELOADEN|SCSIEN|DIRECTION);
		bmov	DFDAT, SCB_CDB_STORE, 12; 
	} else if ((ahc->features & AHC_CMD_CHAN) != 0) {
		if ((ahc->flags & AHC_SCB_BTT) != 0) {
			/*
			 * On the 7895 the data FIFO will
			 * get corrupted if you try to dump
			 * data from external SCB memory into
			 * the FIFO while it is enabled.  So,
			 * fill the fifo and then enable SCSI
			 * transfers.
			 */
			mvi	DFCNTRL, (DIRECTION|FIFORESET);
		} else {
			mvi	DFCNTRL, (SCSIEN|SDMAEN|DIRECTION|FIFORESET);
		}
		bmov	DFDAT, SCB_CDB_STORE, 12; 
		if ((ahc->flags & AHC_SCB_BTT) != 0) {
			mvi	DFCNTRL, (SCSIEN|SDMAEN|DIRECTION|FIFOFLUSH);
		} else {
			or	DFCNTRL, FIFOFLUSH;
		}
	} else {
		mvi	DFCNTRL, (SCSIEN|SDMAEN|DIRECTION|FIFORESET);
		call	copy_to_fifo_6;
		call	copy_to_fifo_6;
		or	DFCNTRL, FIFOFLUSH;
	}
p_command_xfer:
	and	SEQ_FLAGS, ~NO_CDB_SENT;
	if ((ahc->features & AHC_DT) == 0) {
		test	SSTAT0, SDONE jnz . + 2;
		test    SSTAT1, PHASEMIS jz . - 1;
		/*
		 * Wait for our ACK to go-away on it's own
		 * instead of being killed by SCSIEN getting cleared.
		 */
		test	SCSISIGI, ACKI jnz .;
	} else {
		test	DFCNTRL, SCSIEN jnz .;
	}
	test	SSTAT0, SDONE jnz p_command_successful;
	/*
	 * Don't allow a data phase if the command
	 * was not fully transferred.
	 */
	or	SEQ_FLAGS, NO_CDB_SENT;
p_command_successful:
	and	DFCNTRL, ~(SCSIEN|SDMAEN|HDMAEN);
	test	DFCNTRL, (SCSIEN|SDMAEN|HDMAEN) jnz .;
	jmp	ITloop;

/*
 * Status phase.  Wait for the data byte to appear, then read it
 * and store it into the SCB.
 */
p_status:
	test	SEQ_FLAGS, NOT_IDENTIFIED jnz mesgin_proto_violation;
p_status_okay:
	mov	SCB_SCSI_STATUS, SCSIDATL;
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
	if ((ahc->features & AHC_DT) == 0) {
		or	SCSISIGO, ATNO, LASTPHASE;
	} else {
		mvi	SCSISIGO, ATNO;
	}
p_mesgout:
	mov	SINDEX, MSG_OUT;
	cmp	SINDEX, MSG_IDENTIFYFLAG jne p_mesgout_from_host;
	test	SCB_CONTROL,MK_MESSAGE	jnz host_message_loop;
p_mesgout_identify:
	or	SINDEX, MSG_IDENTIFYFLAG|DISCENB, SAVED_LUN;
	test	SCB_CONTROL, DISCENB jnz . + 2;
	and	SINDEX, ~DISCENB;
/*
 * Send a tag message if TAG_ENB is set in the SCB control block.
 * Use SCB_TAG (the position in the kernel's SCB array) as the tag value.
 */
p_mesgout_tag:
	test	SCB_CONTROL,TAG_ENB jz  p_mesgout_onebyte;
	mov	SCSIDATL, SINDEX;	/* Send the identify message */
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	jne p_mesgout_done;
	and	SCSIDATL,TAG_ENB|SCB_TAG_TYPE,SCB_CONTROL;
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	jne p_mesgout_done;
	mov	SCB_TAG	jmp p_mesgout_onebyte;
/*
 * Interrupt the driver, and allow it to handle this message
 * phase and any required retries.
 */
p_mesgout_from_host:
	cmp	SINDEX, HOST_MSG	jne p_mesgout_oneb