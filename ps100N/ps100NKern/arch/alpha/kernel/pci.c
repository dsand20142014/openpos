options != 0)
					printf(")\n");
				else
					printf("\n");
			} else {
				printf("%s: target %d using "
				       "asynchronous transfers%s\n",
				       ahd_name(ahd), devinfo->target,
				       (ppr_options & MSG_EXT_PPR_QAS_REQ) != 0
				     ?  "(QAS)" : "");
			}
		}
	}
	/*
	 * Always refresh the neg-table to handle the case of the
	 * sequencer setting the ENATNO bit for a MK_MESSAGE request.
	 * We will always renegotiate in that case if this is a
	 * packetized request.  Also manage the busfree expected flag
	 * from this common routine so that we catch changes due to
	 * WDTR or SDTR messages.
	 */
	if ((type & AHD_TRANS_CUR) != 0) {
		if (!paused)
			ahd_pause(ahd);
		ahd_update_neg_table(ahd, devinfo, &tinfo->curr);
		if (!paused)
			ahd_unpause(ahd);
		if (ahd->msg_type != MSG_TYPE_NONE) {
			if ((old_ppr & MSG_EXT_PPR_IU_REQ)
			 != (ppr_options & MSG_EXT_PPR_IU_REQ)) {
#ifdef AHD_DEBUG
				if ((ahd_debug & AHD_SHOW_MESSAGES) != 0) {
					ahd_print_devinfo(ahd, devinfo);
					printf("Expecting IU Change busfree\n");
				}
#endif
				ahd->msg_flags |= MSG_FLAG_EXPECT_PPR_BUSFREE
					       |  MSG_FLAG_IU_REQ_CHANGED;
			}
			if ((old_ppr & MSG_EXT_PPR_IU_REQ) != 0) {
#ifdef AHD_DEBUG
				if ((ahd_debug & AHD_SHOW_MESSAGES) != 0)
					printf("PPR with IU_REQ outstanding\n");
#endif
				ahd->msg_flags |= MSG_FLAG_EXPECT_PPR_BUSFREE;
			}
		}
	}

	update_needed += ahd_update_neg_request(ahd, devinfo, tstate,
						tinfo, AHD_NEG_TO_GOAL);

	if (update_needed && active)
		ahd_update_pending_scbs(ahd);
}

/*
 * Update the user/goal/curr tables of wide negotiation
 * parameters as well as, in the case of a current or active update,
 * any data structures on the host controller.  In the case of an
 * active update, the specified target is currently talking to us on
 * the bus, so the transfer parameter update must take effect
 * immediately.
 */
void
ahd_set_width(struct ahd_softc *ahd, struct ahd_devinfo *devinfo,
	      u_int width, u_int type, int paused)
{
	struct	ahd_initiator_tinfo *tinfo;
	struct	ahd_tmode_tstate *tstate;
	u_int	oldwidth;
	int	active;
	int	update_needed;

	active = (type & AHD_TRANS_ACTIVE) == AHD_TRANS_ACTIVE;
	update_needed = 0;
	tinfo = ahd_fetch_transinfo(ahd, devinfo->channel, devinfo->our_scsiid,
				    devinfo->target, &tstate);

	if ((type & AHD_TRANS_USER) != 0)
		tinfo->user.width = width;

	if ((type & AHD_TRANS_GOAL) != 0)
		tinfo->goal.width = width;

	oldwidth = tinfo->curr.width;
	if ((type & AHD_TRANS_CUR) != 0 && oldwidth != width) {

		update_needed++;

		tinfo->curr.width = width;
		ahd_send_async(ahd, devinfo->channel, devinfo->target,
			       CAM_LUN_WILDCARD, AC_TRANSFER_NEG);
		if (bootverbose) {
			printf("%s: target %d using %dbit transfers\n",
			       ahd_name(ahd), devinfo->target,
			       8 * (0x01 << width));
		}
	}

	if ((type & AHD_TRANS_CUR) != 0) {
		if (!paused)
			ahd_pause(ahd);
		ahd_update_neg_table(ahd, devinfo, &tinfo->curr);
		if (!paused)
			ahd_unpause(ahd);
	}

	update_needed += ahd_update_neg_request(ahd, devinfo, tstate,
						tinfo, AHD_NEG_TO_GOAL);
	if (update_needed && active)
		ahd_update_pending_scbs(ahd);

}

/*
 * Update the current state of tagged queuing for a given target.
 */
static void
ahd_set_tags(struct ahd_softc *ahd, struct scsi_cmnd *cmd,
	     struct ahd_devinfo *devinfo, ahd_queue_alg alg)
{
	struct scsi_device *sdev = cmd->device;

	ahd_platform_set_tags(ahd, sdev, devinfo, alg);
	ahd_send_async(ahd, devinfo->channel, devinfo->target,
		       devinfo->lun, AC_TRANSFER_NEG);
}

static void
ahd_update_neg_table(struct ahd_softc *ahd, struct ahd_devinfo *devinfo,
		     struct ahd_transinfo *tinfo)
{
	ahd_mode_state	saved_modes;
	u_int		period;
	u_int		ppr_opts;
	u_int		con_opts;
	u_int		offset;
	u_int		saved_negoaddr;
	uint8_t		iocell_opts[sizeof(ahd->iocell_opts)];

	saved_modes = ahd_save_modes(ahd);
	ahd_set_modes(ahd, AHD_MODE_SCSI, AHD_MODE_SCSI);

	saved_negoaddr = ahd_inb(ahd, NEGOADDR);
	ahd_outb(ahd, NEGOADDR, devinfo->target);
	period = tinfo->period;
	offset = tinfo->offset;
	memcpy(iocell_opts, ahd->iocell_opts, sizeof(ahd->iocell_opts)); 
	ppr_opts = tinfo->ppr_options & (MSG_EXT_PPR_QAS_REQ|MSG_EXT_PPR_DT_REQ
					|MSG_EXT_PPR_IU_REQ|MSG_EXT_PPR_RTI);
	con_opts = 0;
	if (period == 0)
		period = AHD_SYNCRATE_ASYNC;
	if (period == AHD_SYNCRATE_160) {

		if ((ahd->bugs & AHD_PACED_NEGTABLE_BUG) != 0) {
			/*
			 * When the SPI4 spec was finalized, PACE transfers
			 * was not made a configurable option in the PPR
			 * message.  Instead it is assumed to be enabled for
			 * any syncrate faster than 80MHz.  Nevertheless,
			 * Harpoon2A4 allows this to be configurable.
			 *
			 * Harpoon2A4 also assumes at most 2 data bytes per
			 * negotiated REQ/ACK offset.  Paced transfers take
			 * 4, so we must adjust our offset.
			 */
			ppr_opts |= PPROPT_PACE;
			offset *= 2;

			/*
			 * Harpoon2A assumed that there would be a
			 * fallback rate between 160MHz and 80MHz,
			 * so 7 is used as the period factor rather
			 * than 8 for 160MHz.
			 */
			period = AHD_SYNCRATE_REVA_160;
		}
		if ((tinfo->ppr_options & MSG_EXT_PPR_PCOMP_EN) == 0)
			iocell_opts[AHD_PRECOMP_SLEW_INDEX] &=
			    ~AHD_PRECOMP_MASK;
	} else {
		/*
		 * Precomp should be disabled for non-paced transfers.
		 */
		iocell_opts[AHD_PRECOMP_SLEW_INDEX] &= ~AHD_PRECOMP_MASK;

		if ((ahd->features & AHD_NEW_IOCELL_OPTS) != 0
		 && (ppr_opts & MSG_EXT_PPR_DT_REQ) != 0
		 && (ppr_opts & MSG_EXT_PPR_IU_REQ) == 0) {
			/*
			 * Slow down our CRC interval to be
			 * compatible with non-packetized
			 * U160 devices that can't handle a
			 * CRC at full speed.
			 */
			con_opts |= ENSLOWCRC;
		}

		if ((ahd->bugs & AHD_PACED_NEGTABLE_BUG) != 0) {
			/*
			 * On H2A4, revert to a slower slewrate
			 * on non-paced transfers.
			 */
			iocell_opts[AHD_PRECOMP_SLEW_INDEX] &=
			    ~AHD_SLEWRATE_MASK;
		}
	}

	ahd_outb(ahd, ANNEXCOL, AHD_ANNEXCOL_PRECOMP_SLEW);
	ahd_outb(ahd, ANNEXDAT, iocell_opts[AHD_PRECOMP_SLEW_INDEX]);
	ahd_outb(ahd, ANNEXCOL, AHD_ANNEXCOL_AMPLITUDE);
	ahd_outb(ahd, ANNEXDAT, iocell_opts[AHD_AMPLITUDE_INDEX]);

	ahd_outb(ahd, NEGPERIOD, period);
	ahd_outb(ahd, NEGPPROPTS, ppr_opts);
	ahd_outb(ahd, NEGOFFSET, offset);

	if (tinfo->width == MSG_EXT_WDTR_BUS_16_BIT)
		con_opts |= WIDEXFER;

	/*
	 * Slow down our CRC interval to be
	 * compatible with packetized U320 devices
	 * that can't handle a CRC at full speed
	 */
	if (ahd->features & AHD_AIC79XXB_SLOWCRC) {
		con_opts |= ENSLOWCRC;
	}

	/*
	 * During packetized transfers, the target will
	 * give us the oportunity to send command packets
	 * without us asserting attention.
	 */
	if ((tinfo->ppr_options & MSG_EXT_PPR_IU_REQ) == 0)
		con_opts |= ENAUTOATNO;
	ahd_outb(ahd, NEGCONOPTS, con_opts);
	ahd_outb(ahd, NEGOADDR, saved_negoaddr);
	ahd_restore_modes(ahd, saved_modes);
}

/*
 * When the transfer settings for a connection change, setup for
 * negotiation in pending SCBs to effect the change as quickly as
 * possible.  We also cancel any negotiations that are scheduled
 * for inflight SCBs that have not been started yet.
 */
static void
ahd_update_pending_scbs(struct ahd_softc *ahd)
{
	struct		scb *pending_scb;
	int		pending_scb_count;
	int		paused;
	u_int		saved_scbptr;
	ahd_mode_state	saved_modes;

	/*
	 * Traverse the pending SCB list and ensure that all of the
	 * SCBs there have the proper settings.  We can only safely
	 * clear the negotiation required flag (setting requires the
	 * execution queue to be modified) and this is only possible
	 * if we are not already attempting to select out for this
	 * SCB.  For this reason, all callers only call this routine
	 * if we are changing the negotiation settings for the currently
	 * active transaction on the bus.
	 */
	pending_scb_count = 0;
	LIST_FOREACH(pending_scb, &ahd->pending_scbs, pending_links) {
		struct ahd_devinfo devinfo;
		struct ahd_initiator_tinfo *tinfo;
		struct ahd_tmode_tstate *tstate;

		ahd_scb_devinfo(ahd, &devinfo, pending_scb);
		tinfo = ahd_fetch_transinfo(ahd, devinfo.channel,
					    devinfo.our_scsiid,
					    devinfo.target, &tstate);
		if ((tstate->auto_negotiate & devinfo.target_mask) == 0
		 && (pending_scb->flags & SCB_AUTO_NEGOTIATE) != 0) {
			pending_scb->flags &= ~SCB_AUTO_NEGOTIATE;
			pending_scb->hscb->control &= ~MK_MESSAGE;
		}
		ahd_sync_scb(ahd, pending_scb,
			     BUS_DMASYNC_PREREAD|BUS_DMASYNC_PREWRITE);
		pending_scb_count++;
	}

	if (pending_scb_count == 0)
		return;

	if (ahd_is_paused(ahd)) {
		paused = 1;
	} else {
		paused = 0;
		ahd_pause(ahd);
	}

	/*
	 * Force the sequencer to reinitialize the selection for
	 * the command at the head of the execution queue if it
	 * has already been setup.  The negotiation changes may
	 * effect whether we select-out with ATN.  It is only
	 * safe to clear ENSELO when the bus is not free and no
	 * selection is in progres or completed.
	 */
	saved_modes = ahd_save_modes(ahd);
	ahd_set_modes(ahd, AHD_MODE_SCSI, AHD_MODE_SCSI);
	if ((ahd_inb(ahd, SCSISIGI) & BSYI) != 0
	 && (ahd_inb(ahd, SSTAT0) & (SELDO|SELINGO)) == 0)
		ahd_outb(ahd, SCSISEQ0, ahd_inb(ahd, SCSISEQ0) & ~ENSELO);
	saved_scbptr = ahd_get_scbptr(ahd);
	/* Ensure that the hscbs down on the card match the new information */
	LIST_FOREACH(pending_scb, &ahd->pending_scbs, pending_links) {
		u_int	scb_tag;
		u_int	control;

		scb_tag = SCB_GET_TAG(pending_scb);
		ahd_set_scbptr(ahd, scb_tag);
		control = ahd_inb_scbram(ahd, SCB_CONTROL);
		control &= ~MK_MESSAGE;
		control |= pending_scb->hscb->control & MK_MESSAGE;
		ahd_outb(ahd, SCB_CONTROL, control);
	}
	ahd_set_scbptr(ahd, saved_scbptr);
	ahd_restore_modes(ahd, saved_modes);

	if (paused == 0)
		ahd_unpause(ahd);
}

/**************************** Pathing Information *****************************/
static void
ahd_fetch_devinfo(struct ahd_softc *ahd, struct ahd_devinfo *devinfo)
{
	ahd_mode_state	saved_modes;
	u_int		saved_scsiid;
	role_t		role;
	int		our_id;

	saved_modes = ahd_save_modes(ahd);
	ahd_set_modes(ahd, AHD_MODE_SCSI, AHD_MODE_SCSI);

	if (ahd_inb(ahd, SSTAT0) & TARGET)
		role = ROLE_TARGET;
	else
		role = ROLE_INITIATOR;

	if (role == ROLE_TARGET
	 && (ahd_inb(ahd, SEQ_FLAGS) & CMDPHASE_PENDING) != 0) {
		/* We were selected, so pull our id from TARGIDIN */
		our_id = ahd_inb(ahd, TARGIDIN) & OID;
	} else if (role == ROLE_TARGET)
		our_id = ahd_inb(ahd, TOWNID);
	else
		our_id = ahd_inb(ahd, IOWNID);

	saved_scsiid = ahd_inb(ahd, SAVED_SCSIID);
	ahd_compile_devinfo(devinfo,
			    our_id,
			    SCSIID_TARGET(ahd, saved_scsiid),
			    ahd_inb(ahd, SAVED_LUN),
			    SCSIID_CHANNEL(ahd, saved_scsiid),
			    role);
	ahd_restore_modes(ahd, saved_modes);
}

void
ahd_print_devinfo(struct ahd_softc *ahd, struct ahd_devinfo *devinfo)
{
	printf("%s:%c:%d:%d: ", ahd_name(ahd), 'A',
	       devinfo->target, devinfo->lun);
}

static const struct ahd_phase_table_entry*
ahd_lookup_phase_entry(int phase)
{
	const struct ahd_phase_table_entry *entry;
	const struct ahd_phase_table_entry *last_entry;

	/*
	 * num_phases doesn't include the default entry which
	 * will be returned if the phase doesn't match.
	 */
	last_entry = &ahd_phase_table[num_phases];
	for (entry = ahd_phase_table; entry < last_entry; entry++) {
		if (phase == entry->phase)
			break;
	}
	return (entry);
}

void
ahd_compile_devinfo(struct ahd_devinfo *devinfo, u_int our_id, u_int target,
		    u_int lun, char channel, role_t role)
{
	devinfo->our_scsiid = our_id;
	devinfo->target = target;
	devinfo->lun = lun;
	devinfo->target_offset = target;
	devinfo->channel = channel;
	devinfo->role = role;
	if (channel == 'B')
		devinfo->target_offset += 8;
	devinfo->target_mask = (0x01 << devinfo->target_offset);
}

static void
ahd_scb_devinfo(struct ahd_softc *ahd, struct ahd_devinfo *devinfo,
		struct scb *scb)
{
	role_t	role;
	int	our_id;

	our_id = SCSIID_OUR_ID(scb->hscb->scsiid);
	role = ROLE_INITIATOR;
	if ((scb->hscb->control & TARGET_SCB) != 0)
		role = ROLE_TARGET;
	ahd_compile_devinfo(devinfo, our_id, SCB_GET_TARGET(ahd, scb),
			    SCB_GET_LUN(scb), SCB_GET_CHANNEL(ahd, scb), role);
}


/************************ Message Phase Processing ****************************/
/*
 * When an initiator transaction with the MK_MESSAGE flag either reconnects
 * or enters the initial message out phase, we are interrupted.  Fill our
 * outgoing message buffer with the appropriate message and beging handing
 * the message phase(s) manually.
 */
static void
ahd_setup_initiator_msgout(struct ahd_softc *ahd, struct ahd_devinfo *devinfo,
			   struct scb *scb)
{
	/*
	 * To facilitate adding multiple messages together,
	 * each routine should increment the index and len
	 * variables instead of setting them explicitly.
	 */
	ahd->msgout_index = 0;
	ahd->msgout_len = 0;

	if (ahd_currently_packetized(ahd))
		ahd->msg_flags |= MSG_FLAG_PACKETIZED;

	if (ahd->send_msg_perror
	 && ahd_inb(ahd, MSG_OUT) == HOST_MSG) {
		ahd->msgout_buf[ahd->msgout_index++] = ahd->send_msg_perror;
		ahd->msgout_len++;
		ahd->msg_type = MSG_TYPE_INITIATOR_MSGOUT;
#ifdef AHD_DEBUG
		if ((ahd_debug & AHD_SHOW_MESSAGES) != 0)
			printf("Setting up for Parity Error delivery\n");
#endif
		return;
	} else if (scb == NULL) {
		printf("%s: WARNING. No pending message for "
		       "I_T msgin.  Issuing NO-OP\n", ahd_name(ahd));
		ahd->msgout_buf[ahd->msgout_index++] = MSG_NOOP;
		ahd->msgout_len++;
		ahd->msg_type = MSG_TYPE_INITIATOR_MSGOUT;
		return;
	}

	if ((scb->flags & SCB_DEVICE_RESET) == 0
	 && (scb->flags & SCB_PACKETIZED) == 0
	 && ahd_inb(ahd, MSG_OUT) == MSG_IDENTIFYFLAG) {
		u_int identify_msg;

		identify_msg = MSG_IDENTIFYFLAG | SCB_GET_LUN(scb);
		if ((scb->hscb->control & DISCENB) != 0)
			identify_msg |= MSG_IDENTIFY_DISCFLAG;
		ahd->msgout_buf[ahd->msgout_index++] = identify_msg;
		ahd->msg