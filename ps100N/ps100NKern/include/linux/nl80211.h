if
			i = 1;	/* Means we have found some */
			break;
		}
		if (!i) {
			/*
			 *  We are done, so we donnot need 
			 *  to synchronize with the SCRIPTS anylonger.
			 *  Remove the SEM flag from the ISTAT.
			 */
			np->istat_sem = 0;
			OUTB(np, nc_istat, SIGP);
			break;
		}
		/*
		 *  Compute index of next position in the start 
		 *  queue the SCRIPTS intends to start and dequeue 
		 *  all CCBs for that device that haven't been started.
		 */
		i = (INL(np, nc_scratcha) - np->squeue_ba) / 4;
		i = sym_dequeue_from_squeue(np, i, cp->target, cp->lun, -1);

		/*
		 *  Make sure at least our IO to abort has been dequeued.
		 */
#ifndef SYM_OPT_HANDLE_DEVICE_QUEUEING
		assert(i && sym_get_cam_status(cp->cmd) == DID_SOFT_ERROR);
#else
		sym_remque(&cp->link_ccbq);
		sym_insque_tail(&cp->link_ccbq, &np->comp_ccbq);
#endif
		/*
		 *  Keep track in cam status of the reason of the abort.
		 */
		if (cp->to_abort == 2)
			sym_set_cam_status(cp->cmd, DID_TIME_OUT);
		else
			sym_set_cam_status(cp->cmd, DID_ABORT);

		/*
		 *  Complete with error everything that we have dequeued.
	 	 */
		sym_flush_comp_queue(np, 0);
		break;
	/*
	 *  The SCRIPTS processor has selected a target 
	 *  we may have some manual recovery to perform for.
	 */
	case SIR_TARGET_SELECTED:
		target = INB(np, nc_sdid) & 0xf;
		tp = &np->target[target];

		np->abrt_tbl.addr = cpu_to_scr(vtobus(np->abrt_msg));

		/*
		 *  If the target is to be reset, prepare a 
		 *  M_RESET message and clear the to_reset flag 
		 *  since we donnot expect this operation to fail.
		 */
		if (tp->to_reset) {
			np->abrt_msg[0] = M_RESET;
			np->abrt_tbl.size = 1;
			tp->to_reset = 0;
			break;
		}

		/*
		 *  Otherwise, look for some logical unit to be cleared.
		 */
		if (tp->lun0p && tp->lun0p->to_clear)
			lun = 0;
		else if (tp->lunmp) {
			for (k = 1 ; k < SYM_CONF_MAX_LUN ; k++) {
				if (tp->lunmp[k] && tp->lunmp[k]->to_clear) {
					lun = k;
					break;
				}
			}
		}

		/*
		 *  If a logical unit is to be cleared, prepare 
		 *  an IDENTIFY(lun) + ABORT MESSAGE.
		 */
		if (lun != -1) {
			struct sym_lcb *lp = sym_lp(tp, lun);
			lp->to_clear = 0; /* We don't expect to fail here */
			np->abrt_msg[0] = IDENTIFY(0, lun);
			np->abrt_msg[1] = M_ABORT;
			np->abrt_tbl.size = 2;
			break;
		}

		/*
		 *  Otherwise, look for some disconnected job to 
		 *  abort for this target.
		 */
		i = 0;
		cp = NULL;
		FOR_EACH_QUEUED_ELEMENT(&np->busy_ccbq, qp) {
			cp = sym_que_entry(qp, struct sym_ccb, link_ccbq);
			if (cp->host_status != HS_DISCONNECT)
				continue;
			if (cp->target != target)
				continue;
			if (!cp->to_abort)
				continue;
			i = 1;	/* Means we have some */
			break;
		}

		/*
		 *  If we have none, probably since the device has 
		 *  completed the command before we won abitration,
		 *  send a M_ABORT message without IDENTIFY.
		 *  According to the specs, the device must just 
		 *  disconnect the BUS and not abort any task.
		 */
		if (!i) {
			np->abrt_msg[0] = M_ABORT;
			np->abrt_tbl.size = 1;
			break;
		}

		/*
		 *  We have some task to abort.
		 *  Set the IDENTIFY(lun)
		 */
		np->abrt_msg[0] = IDENTIFY(0, cp->lun);

		/*
		 *  If we want to abort an untagged command, we 
		 *  will send a IDENTIFY + M_ABORT.
		 *  Otherwise (tagged command), we will send 
		 *  a IDENTITFY + task attributes + ABORT TAG.
		 */
		if (cp->tag == NO_TAG) {
			np->abrt_msg[1] = M_ABORT;
			np->abrt_tbl.size = 2;
		} else {
			np->abrt_msg[1] = cp->scsi_smsg[1];
			np->abrt_msg[2] = cp->scsi_smsg[2];
			np->abrt_msg[3] = M_ABORT_TAG;
			np->abrt_tbl.size = 4;
		}
		/*
		 *  Keep track of software timeout condition, since the 
		 *  peripheral driver may not count retries on abort 
		 *  conditions not due to timeout.
		 */
		if (cp->to_abort == 2)
			sym_set_cam_status(cp->cmd, DID_TIME_OUT);
		cp->to_abort = 0; /* We donnot expect to fail here */
		break;

	/*
	 *  The target has accepted our message and switched 
	 *  to BUS FREE phase as we expected.
	 */
	case SIR_ABORT_SENT:
		target = INB(np, nc_sdid) & 0xf;
		tp = &np->target[target];
		starget = tp->starget;
		
		/*
		**  If we didn't abort anything, leave here.
		*/
		if (np->abrt_msg[0] == M_ABORT)
			break;

		/*
		 *  If we sent a M_RESET, then a hardware reset has 
		 *  been performed by the target.
		 *  - Reset everything to async 8 bit
		 *  - Tell ourself to negotiate next time :-)
		 *  - Prepare to clear all disconnected CCBs for 
		 *    this target from our task list (lun=task=-1)
		 */
		lun = -1;
		task = -1;
		if (np->abrt_msg[0] == M_RESET) {
			tp->head.sval = 0;
			tp->head.wval = np->rv_scntl3;
			tp->head.uval = 0;
			spi_period(starget) = 0;
			spi_offset(starget) = 0;
			spi_width(starget) = 0;
			spi_iu(starget) = 0;
			spi_dt(starget) = 0;
			spi_qas(starget) = 0;
			tp->tgoal.check_nego = 1;
			tp->tgoal.renego = 0;
		}

		/*
		 *  Otherwise, check for the LUN and TASK(s) 
		 *  concerned by the cancelation.
		 *  If it is not ABORT_TAG then it is CLEAR_QUEUE 
		 *  or an ABORT message :-)
		 */
		else {
			lun = np->abrt_msg[0] & 0x3f;
			if (np->abrt_msg[1] == M_ABORT_TAG)
				task = np->abrt_msg[2];
		}

		/*
		 *  Complete all the CCBs the device should have 
		 *  aborted due to our 'kiss of death' message.
		 */
		i = (INL(np, nc_scratcha) - np->squeue_ba) / 4;
		sym_dequeue_from_squeue(np, i, target, lun, -1);
		sym_clear_tasks(np, DID_ABORT, target, lun, task);
		sym_flush_comp_queue(np, 0);

 		/*
		 *  If we sent a BDR, make upper layer aware of that.
 		 */
		if (np->abrt_msg[0] == M_RESET)
			starget_printk(KERN_NOTICE, starget,
							"has been reset\n");
		break;
	}

	/*
	 *  Print to the log the message we intend to send.
	 */
	if (num == SIR_TARGET_SELECTED) {
		dev_info(&tp->starget->dev, "control msgout:");
		sym_printl_hex(np->abrt_msg, np->abrt_tbl.size);
		np->abrt_tbl.size = cpu_to_scr(np->abrt_tbl.size);
	}

	/*
	 *  Let the SCRIPTS processor continue.
	 */
	OUTONB_STD();
}

/*
 *  Gerard's alchemy:) that deals with with the data 
 *  pointer for both MDP and the residual calculation.
 *
 *  I didn't want to bloat the code by more than 200 
 *  lines for the handling of both MDP and the residual.
 *  This has been achieved by using a data pointer 
 *  representation consisting in an index in the data 
 *  array (dp_sg) and a negative offset (dp_ofs) that 
 *  have the following meaning:
 *
 *  - dp_sg = SYM_CONF_MAX_SG
 *    we are at the end of the data script.
 *  - dp_sg < SYM_CONF_MAX_SG
 *    dp_sg points to the next entry of the scatter array 
 *    we want to transfer.
 *  - dp_ofs < 0
 *    dp_ofs represents the residual of bytes of the 
 *    previous entry scatter entry we will send first.
 *  - dp_ofs = 0
 *    no residual to send first.
 *
 *  The function sym_evaluate_dp() accepts an arbitray 
 *  offset (basically from the MDP message) and returns 
 *  the corresponding values of dp_sg and dp_ofs.
 */

static int sym_evaluate_dp(struct sym_hcb *np, struct sym_ccb *cp, u32 scr, int *ofs)
{
	u32	dp_scr;
	int	dp_ofs, dp_sg, dp_sgmin;
	int	tmp;
	struct sym_pmc *pm;

	/*
	 *  Compute the resulted data pointer in term of a script 
	 *  address within some DATA script and a signed byte offset.
	 */
	dp_scr = scr;
	dp_ofs = *ofs;
	if	(dp_scr == SCRIPTA_BA(np, pm0_data))
		pm = &cp->phys.pm0;
	else if (dp_scr == SCRIPTA_BA(np, pm1_data))
		pm = &cp->phys.pm1;
	else
		pm = NULL;

	if (pm) {
		dp_scr  = scr_to_cpu(pm->ret);
		dp_ofs -= scr_to_cpu(pm->sg.size) & 0x00ffffff;
	}

	/*
	 *  If we are auto-sensing, then we are done.
	 */
	if (cp->host_flags & HF_SENSE) {
		*ofs = dp_ofs;
		return 0;
	}

	/*
	 *  Deduce the index of the sg entry.
	 *  Keep track of the index of the first valid entry.
	 *  If result is dp_sg = SYM_CONF_MAX_SG, then we are at the 
	 *  end of the data.
	 */
	tmp = scr_to_cpu(cp->goalp);
	dp_sg = SYM_CONF_MAX_SG;
	if (dp_scr != tmp)
		dp_sg -= (tmp - 8 - (int)dp_scr) / (2*4);
	dp_sgmin = SYM_CONF_MAX_SG - cp->segments;

	/*
	 *  Move to the sg entry the data pointer belongs to.
	 *
	 *  If we are inside the data area, we expect result to be:
	 *
	 *  Either,
	 *      dp_ofs = 0 and dp_sg is the index of the sg entry
	 *      the data pointer belongs to (or the end of the data)
	 *  Or,
	 *      dp_ofs < 0 and dp_sg is the index of the sg entry 
	 *      the data pointer belongs to + 1.
	 */
	if (dp_ofs < 0) {
		int n;
		while (dp_sg > dp_sgmin) {
			--dp_sg;
			tmp = scr_to_cpu(cp->phys.data[dp_sg].size);
			n = dp_ofs + (tmp & 0xffffff);
			if (n > 0) {
				++dp_sg;
				break;
			}
			dp_ofs = n;
		}
	}
	else if (dp_ofs > 0) {
		while (dp_sg < SYM_CONF_MAX_SG) {
			tmp = scr_to_cpu(cp->phys.data[dp_sg].size);
			dp_ofs -= (tmp & 0xffffff);
			++dp_sg;
			if (dp_ofs <= 0)
				break;
		}
	}

	/*
	 *  Make sure the data pointer is inside the data area.
	 *  If not, return some error.
	 */
	if	(dp_sg < dp_sgmin || (dp_sg == dp_sgmin && dp_ofs < 0))
		goto out_err;
	else if	(dp_sg > SYM_CONF_MAX_SG ||
		 (dp_sg == SYM_CONF_MAX_SG && dp_ofs > 0))
		goto out_err;

	/*
	 *  Save the extreme pointer if needed.
	 */
	if (dp_sg > cp->ext_sg ||
            (dp_sg == cp->ext_sg && dp_ofs > cp->ext_ofs)) {
		cp->ext_sg  = dp_sg;
		cp->ext_ofs = dp_ofs;
	}

	/*
	 *  Return data.
	 */
	*ofs = dp_ofs;
	return dp_sg;

out_err:
	return -1;
}

/*
 *  chip handler for MODIFY DATA POINTER MESSAGE
 *
 *  We also call this function on IGNORE WIDE RESIDUE 
 *  messages that do not match a SWIDE full condition.
 *  Btw, we assume in that situation that such a message 
 *  is equivalent to a MODIFY DATA POINTER (offset=-1).
 */

static void sym_modify_dp(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp, int ofs)
{
	int dp_ofs	= ofs;
	u32	dp_scr	= sym_get_script_dp (np, cp);
	u32	dp_ret;
	u32	tmp;
	u_char	hflags;
	int	dp_sg;
	struct	sym_pmc *pm;

	/*
	 *  Not supported for auto-sense.
	 */
	if (cp->host_flags & HF_SENSE)
		goto out_reject;

	/*
	 *  Apply our alchemy:) (see comments in sym_evaluate_dp()), 
	 *  to the resulted data pointer.
	 */
	dp_sg = sym_evaluate_dp(np, cp, dp_scr, &dp_ofs);
	if (dp_sg < 0)
		goto out_reject;

	/*
	 *  And our alchemy:) allows to easily calculate the data 
	 *  script address we want to return for the next data phase.
	 */
	dp_ret = cpu_to_scr(cp->goalp);
	dp_ret = dp_ret - 8 - (SYM_CONF_MAX_SG - dp_sg) * (2*4);

	/*
	 *  If offset / scatter entry is zero we donnot need 
	 *  a context for the new current data pointer.
	 */
	if (dp_ofs == 0) {
		dp_scr = dp_ret;
		goto out_ok;
	}

	/*
	 *  Get a context for the new current data pointer.
	 */
	hflags = INB(np, HF_PRT);

	if (hflags & HF_DP_SAVED)
		hflags ^= HF_ACT_PM;

	if (!(hflags & HF_ACT_PM)) {
		pm  = &cp->phys.pm0;
		dp_scr = SCRIPTA_BA(np, pm0_data);
	}
	else {
		pm = &cp->phys.pm1;
		dp_scr = SCRIPTA_BA(np, pm1_data);
	}

	hflags &= ~(HF_DP_SAVED);

	OUTB(np, HF_PRT, hflags);

	/*
	 *  Set up the new current data pointer.
	 *  ofs < 0 there, and for the next data phase, we 
	 *  want to transfer part of the data of the sg entry 
	 *  corresponding to index dp_sg-1 prior to returning 
	 *  to the main data script.
	 */
	pm->ret = cpu_to_scr(dp_ret);
	tmp  = scr_to_cpu(cp->phys.data[dp_sg-1].addr);
	tmp += scr_to_cpu(cp->phys.data[dp_sg-1].size) + dp_ofs;
	pm->sg.addr = cpu_to_scr(tmp);
	pm->sg.size = cpu_to_scr(-dp_ofs);

out_ok:
	sym_set_script_dp (np, cp, dp_scr);
	OUTL_DSP(np, SCRIPTA_BA(np, clrack));
	return;

out_reject:
	OUTL_DSP(np, SCRIPTB_BA(np, msg_bad));
}


/*
 *  chip calculation of the data residual.
 *
 *  As I used to say, the requirement of data residual 
 *  in SCSI is broken, useless and cannot be achieved 
 *  without huge complexity.
 *  But most OSes and even the official CAM require it.
 *  When stupidity happens to be so widely spread inside 
 *  a community, it gets hard to convince.
 *
 *  Anyway, I don't care, since I am not going to use 
 *  any software that considers this data residual as 
 *  a relevant information. :)
 */

int sym_compute_residual(struct sym_hcb *np, struct sym_ccb *cp)
{
	int dp_sg, dp_sgmin, resid = 0;
	int dp_ofs = 0;

	/*
	 *  Check for some data lost or just thrown away.
	 *  We are not required to be quite accurate in this 
	 *  situation. Btw, if we are odd for output and the 
	 *  device claims some more data, it may well happen 
	 *  than our residual be zero. :-)
	 */
	if (cp->xerr_status & (XE_EXTRA_DATA|XE_SODL_UNRUN|XE_SWIDE_OVRUN)) {
		if (cp->xerr_status & XE_EXTRA_DATA)
			resid -= cp->extra_bytes;
		if (cp->xerr_status & XE_SODL_UNRUN)
			++resid;
		if (cp->xerr_status & XE_SWIDE_OVRUN)
			--resid;
	}

	/*
	 *  If all data has been transferred,
	 *  there is no residual.
	 */
	if (cp->phys.head.lastp == cp->goalp)
		return resid;

	/*
	 *  If no data transfer occurs, or if the data
	 *  pointer is weird, return full residual.
	 */
	if (cp->startp == cp->phys.head.lastp ||
	    sym_evaluate_dp(np, cp, scr_to_cpu(cp->phys.head.lastp),
			    &dp_ofs) < 0) {
		return cp->data_len - cp->odd_byte_adjustment;
	}

	/*
	 *  If we were auto-sensing, then we are done.
	 */
	if (cp->host_flags & HF_SENSE) {
		return -dp_ofs;
	}

	/*
	 *  We are now full comfortable in the computation 
	 *  of the data residual (2's complement).
	 */
	dp_sgmin = SYM_CONF_MAX_SG - cp->segments;
	resid = -cp->ext_ofs;
	for (dp_sg = cp->ext_sg; dp_sg < SYM_CONF_MAX_SG; ++dp_sg) {
		u_int tmp = scr_to_cpu(cp->phys.data[dp_sg].size);
		resid += (tmp & 0xffffff);
	}

	resid -= cp->odd_byte_adjustment;

	/*
	 *  Hopefully, the result is not too wrong.
	 */
	return resid;
}

/*
 *  Negotiation for WIDE and SYNCHRONOUS DATA TRANSFER.
 *
 *  When we try to negotiate, we append the negotiation message
 *  to the identify and (maybe) simple tag message.
 *  The host status field is set to HS_NEGOTIATE to mark this
 *  situation.
 *
 *  If the target doesn't answer this message immediately
 *  (as required by the standard), the SIR_NEGO_FAILED interrupt
 *  will be raised eventually.
 *  The handler removes the HS_NEGOTIATE status, and sets the
 *  negotiated value to the default (async / nowide).
 *
 *  If we receive a matching answer immediately, we check it
 *  for validity, and set the values.
 *
 *  If we receive a Reject message immediately, we assume the
 *  negotiation has failed, and fall back to standard values.
 *
 *  If we receive a negotiation message while not in HS_NEGOTIATE
 *  state, it's a target initiated negotiation. We prepare a
 *  (hopefully) valid answer, set our parameters, and send back 
 *  this answer to the target.
 *
 *  If the target doesn't fetch the answer (no message out phase),
 *  we assume the negotiation has failed, and fall back to default
 *  settings (SIR_NEGO_PROTO interrupt).
 *
 *  When we set the values, we adjust them in all ccbs belonging 
 *  to this target, in the controller's register, and in the "phys"
 *  field of the controller's struct sym_hcb.
 */

/*
 *  chip handler for SYNCHRONOUS DATA TRANSFER REQUEST (SDTR) message.
 */
static int  
sym_sync_nego_check(struct sym_hcb *np, int req, struct sym_ccb *cp)
{
	int target = cp->target;
	u_char	chg, ofs, per, fak, div;

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_nego_msg(np, target, "sync msgin", np->msgin);
	}

	/*
	 *  Get requested values.
	 */
	chg = 0;
	per = np->msgin[3];
	ofs = np->msgin[4];

	/*
	 *  Check values against our limits.
	 */
	if (ofs) {
		if (ofs > np->maxoffs)
			{chg = 1; ofs = np->maxoffs;}
	}

	if (ofs) {
		if (per < np->minsync)
			{chg = 1; per = np->minsync;}
	}

	/*
	 *  Get new chip synchronous parameters value.
	 */
	div = fak = 0;
	if (ofs && sym_getsync(np, 0, per, &div, &fak) < 0)
		goto reject_it;

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_addr(cp->cmd,
				"sdtr: ofs=%d per=%d div=%d fak=%d chg=%d.\n",
				ofs, per, div, fak, chg);
	}

	/*
	 *  If it was an answer we want to change, 
	 *  then it isn't acceptable. Reject it.
	 */
	if (!req && chg)
		goto reject_it;

	/*
	 *  Apply new values.
	 */
	sym_setsync (np, target, ofs, per, div, fak);

	/*
	 *  It was an answer. We are done.
	 */
	if (!req)
		return 0;

	/*
	 *  It was a request. Prepare an answer message.
	 */
	spi_populate_sync_msg(np->msgout, per, ofs);

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_nego_msg(np, target, "sync msgout", np->msgout);
	}

	np->msgin [0] = M_NOOP;

	return 0;

reject_it:
	sym_setsync (np, target, 0, 0, 0, 0);
	return -1;
}

static void sym_sync_nego(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp)
{
	int req = 1;
	int result;

	/*
	 *  Request or answer ?
	 */
	if (INB(np, HS_PRT) == HS_NEGOTIATE) {
		OUTB(np, HS_PRT, HS_BUSY);
		if (cp->nego_status && cp->nego_status != NS_SYNC)
			goto reject_it;
		req = 0;
	}

	/*
	 *  Check and apply new values.
	 */
	result = sym_sync_nego_check(np, req, cp);
	if (result)	/* Not acceptable, reject it */
		goto reject_it;
	if (req) {	/* Was a request, send response. */
		cp->nego_status = NS_SYNC;
		OUTL_DSP(np, SCRIPTB_BA(np, sdtr_resp));
	}
	else		/* Was a response, we are done. */
		OUTL_DSP(np, SCRIPTA_BA(np, clrack));
	return;

reject_it:
	OUTL_DSP(np, SCRIPTB_BA(np, msg_bad));
}

/*
 *  chip handler for PARALLEL PROTOCOL REQUEST (PPR) message.
 */
static int 
sym_ppr_nego_check(struct sym_hcb *np, int req, int target)
{
	struct sym_tcb *tp = &np->target[target];
	unsigned char fak, div;
	int dt, chg = 0;

	unsigned char per = np->msgin[3];
	unsigned char ofs = np->msgin[5];
	unsigned char wide = np->msgin[6];
	unsigned char opts = np->msgin[7] & PPR_OPT_MASK;

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_nego_msg(np, target, "ppr msgin", np->msgin);
	}

	/*
	 *  Check values against our limits.
	 */
	if (wide > np->maxwide) {
		chg = 1;
		wide = np->maxwide;
	}
	if (!wide || !(np->features & FE_U3EN))
		opts = 0;

	if (opts != (np->msgin[7] & PPR_OPT_MASK))
		chg = 1;

	dt = opts & PPR_OPT_DT;

	if (ofs) {
		unsigned char maxoffs = dt ? np->maxoffs_dt : np->maxoffs;
		if (ofs > maxoffs) {
			chg = 1;
			ofs = maxoffs;
		}
	}

	if (ofs) {
		unsigned char minsync = dt ? np->minsync_dt : np->minsync;
		if (per < minsync) {
			chg = 1;
			per = minsync;
		}
	}

	/*
	 *  Get new chip synchronous parameters value.
	 */
	div = fak = 0;
	if (ofs && sym_getsync(np, dt, per, &div, &fak) < 0)
		goto reject_it;

	/*
	 *  If it was an answer we want to change, 
	 *  then it isn't acceptable. Reject it.
	 */
	if (!req && chg)
		goto reject_it;

	/*
	 *  Apply new values.
	 */
	sym_setpprot(np, target, opts, ofs, per, wide, div, fak);

	/*
	 *  It was an answer. We are done.
	 */
	if (!req)
		return 0;

	/*
	 *  It was a request. Prepare an answer message.
	 */
	spi_populate_ppr_msg(np->msgout, per, ofs, wide, opts);

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_nego_msg(np, target, "ppr msgout", np->msgout);
	}

	np->msgin [0] = M_NOOP;

	return 0;

reject_it:
	sym_setpprot (np, target, 0, 0, 0, 0, 0, 0);
	/*
	 *  If it is a device response that should result in  
	 *  ST, we may want to try a legacy negotiation later.
	 */
	if (!req && !opts) {
		tp->tgoal.period = per;
		tp->tgoal.offset = ofs;
		tp->tgoal.width = wide;
		tp->tgoal.iu = tp->tgoal.dt = tp->tgoal.qas = 0;
		tp->tgoal.check_nego = 1;
	}
	return -1;
}

static void sym_ppr_nego(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp)
{
	int req = 1;
	int result;

	/*
	 *  Request or answer ?
	 */
	if (INB(np, HS_PRT) == HS_NEGOTIATE) {
		OUTB(np, HS_PRT, HS_BUSY);
		if (cp->nego_status && cp->nego_status != NS_PPR)
			goto reject_it;
		req = 0;
	}

	/*
	 *  Check and apply new values.
	 */
	result = sym_ppr_nego_check(np, req, cp->target);
	if (result)	/* Not acceptable, reject it */
		goto reject_it;
	if (req) {	/* Was a request, send response. */
		cp->nego_status = NS_PPR;
		OUTL_DSP(np, SCRIPTB_BA(np, ppr_resp));
	}
	else		/* Was a response, we are done. */
		OUTL_DSP(np, SCRIPTA_BA(np, clrack));
	return;

reject_it:
	OUTL_DSP(np, SCRIPTB_BA(np, msg_bad));
}

/*
 *  chip handler for WIDE DATA TRANSFER REQUEST (WDTR) message.
 */
static int  
sym_wide_nego_check(struct sym_hcb *np, int req, struct sym_ccb *cp)
{
	int target = cp->target;
	u_char	chg, wide;

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_nego_msg(np, target, "wide msgin", np->msgin);
	}

	/*
	 *  Get requested values.
	 */
	chg  = 0;
	wide = np->msgin[3];

	/*
	 *  Check values against our limits.
	 */
	if (wide > np->maxwide) {
		chg = 1;
		wide = np->maxwide;
	}

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_addr(cp->cmd, "wdtr: wide=%d chg=%d.\n",
				wide, chg);
	}

	/*
	 *  If it was an answer we want to change, 
	 *  then it isn't acceptable. Reject it.
	 */
	if (!req && chg)
		goto reject_it;

	/*
	 *  Apply new values.
	 */
	sym_setwide (np, target, wide);

	/*
	 *  It was an answer. We are done.
	 */
	if (!req)
		return 0;

	/*
	 *  It was a request. Prepare an answer message.
	 */
	spi_populate_width_msg(np->msgout, wide);

	np->msgin [0] = M_NOOP;

	if (DEBUG_FLAGS & DEBUG_NEGO) {
		sym_print_nego_msg(np, target, "wide msgout", np->msgout);
	}

	return 0;

reject_it:
	return -1;
}

static void sym_wide_nego(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp)
{
	int req = 1;
	int result;

	/*
	 *  Request or answer ?
	 */
	if (INB(np, HS_PRT) == HS_NEGOTIATE) {
		OUTB(np, HS_PRT, HS_BUSY);
		if (cp->nego_status && cp->nego_status != NS_WIDE)
			goto reject_it;
		req = 0;
	}

	/*
	 *  Check and apply new values.
	 */
	result = sym_wide_nego_check(np, req, cp);
	if (result)	/* Not acceptable, reject it */
		goto reject_it;
	if (req) {	/* Was a request, send response. */
		cp->nego_status = NS_WIDE;
		OUTL_DSP(np, SCRIPTB_BA(np, wdtr_resp));
	} else {		/* Was a response. */
		/*
		 * Negotiate for SYNC immediately after WIDE response.
		 * This allows to negotiate for both WIDE and SYNC on 
		 * a single SCSI command (Suggested by Justin Gibbs).
		 */
		if (tp->tgoal.offset) {
			spi_populate_sync_msg(np->msgout, tp->tgoal.period,
					tp->tgoal.offset);

			if (DEBUG_FLAGS & DEBUG_NEGO) {
				sym_print_nego_msg(np, cp->target,
				                   "sync msgout", np->msgout);
			}

			cp->nego_status = NS_SYNC;
			OUTB(np, HS_PRT, HS_NEGOTIATE);
			OUTL_DSP(np, SCRIPTB_BA(np, sdtr_resp));
			return;
		} else
			OUTL_DSP(np, SCRIPTA_BA(np, clrack));
	}

	return;

reject_it:
	OUTL_DSP(np, SCRIPTB_BA(np, msg_bad));
}

/*
 *  Reset DT, SYNC or WIDE to default settings.
 *
 *  Called when a negotiation does not succeed either 
 *  on rejection or on protocol error.
 *
 *  A target that understands a PPR message should never 
 *  reject it, and messing with it is very unlikely.
 *  So, if a PPR makes problems, we may just want to 
 *  try a legacy negotiation later.
 */
static void sym_nego_default(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp)
{
	switch (cp->nego_status) {
	case NS_PPR:
#if 0
		sym_setpprot (np, cp->target, 0, 0, 0, 0, 0, 0);
#else
		if (tp->tgoal.period < np->minsync)
			tp->tgoal.period = np->minsync;
		if (tp->tgoal.offset > np->maxoffs)
			tp->tgoal.offset = np->maxoffs;
		tp->tgoal.iu = tp->tgoal.dt = tp->tgoal.qas = 0;
		tp->tgoal.check_nego = 1;
#endif
		break;
	case NS_SYNC:
		sym_setsync (np, cp->target, 0, 0, 0, 0);
		break;
	case NS_WIDE:
		sym_setwide (np, cp->target, 0);
		break;
	}
	np->msgin [0] = M_NOOP;
	np->msgout[0] = M_NOOP;
	cp->nego_status = 0;
}

/*
 *  chip handler for MESSAGE REJECT received in response to 
 *  PPR, WIDE or SYNCHRONOUS negotiation.
 */
static void sym_nego_rejected(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp)
{
	sym_nego_default(np, tp, cp);
	OUTB(np, HS_PRT, HS_BUSY);
}

/*
 *  chip exception handler for programmed interrupts.
 */
static void sym_int_sir(struct sym_hcb *np)
{
	u_char	num	= INB(np, nc_dsps);
	u32	dsa	= INL(np, nc_dsa);
	struct sym_ccb *cp	= sym_ccb_from_dsa(np, dsa);
	u_char	target	= INB(np, nc_sdid) & 0x0f;
	struct sym_tcb *tp	= &np->target[target];
	int	tmp;

	if (DEBUG_FLAGS & DEBUG_TINY) printf ("I#%d", num);

	switch (num) {
#if   SYM_CONF_DMA_ADDRESSING_MODE == 2
	/*
	 *  SCRIPTS tell us that we may have to update 
	 *  64 bit DMA segment registers.
	 */
	case SIR_DMAP_DIRTY:
		sym_update_dmap_regs(np);
		goto out;
#endif
	/*
	 *  Command has been completed with error condition 
	 *  or has been auto-sensed.
	 */
	case SIR_COMPLETE_ERROR:
		sym_complete_error(np, cp);
		return;
	/*
	 *  The C code is currently trying to recover from something.
	 *  Typically, user want to abort some command.
	 */
	case SIR_SCRIPT_STOPPED:
	case SIR_TARGET_SELECTED:
	case SIR_ABORT_SENT:
		sym_sir_task_recovery(np, num);
		return;
	/*
	 *  The device didn't go to MSG OUT phase after having 
	 *  been selected with ATN.  We do not want to handle that.
	 */
	case SIR_SEL_ATN_NO_MSG_OUT:
		scmd_printk(KERN_WARNING, cp->cmd,
				"No MSG OUT phase after selection with ATN\n");
		goto out_stuck;
	/*
	 *  The device didn't switch to MSG IN phase after 
	 *  having reselected the initiator.
	 */
	case SIR_RESEL_NO_MSG_IN:
		scmd_printk(KERN_WARNING, cp->cmd,
				"No MSG IN phase after reselection\n");
		goto out_stuck;
	/*
	 *  After reselection, the device sent a message that wasn't 
	 *  an IDENTIFY.
	 */
	case SIR_RESEL_NO_IDENTIFY:
		scmd_printk(KERN_WARNING, cp->cmd,
				"No IDENTIFY after reselection\n");
		goto out_stuck;
	/*
	 *  The device reselected a LUN we do not know about.
	 */
	case SIR_RESEL_BAD_LUN:
		np->msgout[0] = M_RESET;
		goto out;
	/*
	 *  The device reselected for an untagged nexus and we 
	 *  haven't any.
	 */
	case SIR_RESEL_BAD_I_T_L:
		np->msgout[0] = M_ABORT;
		goto out;
	/*
	 * The device reselected for a tagged nexus that we do not have.
	 */
	case SIR_RESEL_BAD_I_T_L_Q:
		np->msgout[0] = M_ABORT_TAG;
		goto out;
	/*
	 *  The SCRIPTS let us know that the device has grabbed 
	 *  our message and will abort the job.
	 */
	case SIR_RESEL_ABORTED:
		np->lastmsg = np->msgout[0];
		np->msgout[0] = M_NOOP;
		scmd_printk(KERN_WARNING, cp->cmd,
			"message %x sent on bad reselection\n", np->lastmsg);
		goto out;
	/*
	 *  The SCRIPTS let us know that a message has been 
	 *  successfully sent to the device.
	 */
	case SIR_MSG_OUT_DONE:
		np->lastmsg = np->msgout[0];
		np->msgout[0] = M_NOOP;
		/* Should we really care of that */
		if (np->lastmsg == M_PARITY || np->lastmsg == M_ID_ERROR) {
			if (cp) {
				cp->xerr_status &= ~XE_PARITY_ERR;
				if (!cp->xerr_status)
					OUTOFFB(np, HF_PRT, HF_EXT_ERR);
			}
		}
		goto out;
	/*
	 *  The device didn't send a GOOD SCSI status.
	 *  We may have some work to do prior to allow 
	 *  the SCRIPTS processor to continue.
	 */
	case SIR_BAD_SCSI_STATUS:
		if (!cp)
			goto out;
		sym_sir_bad_scsi_status(np, num, cp);
		return;
	/*
	 *  We are asked by the SCRIPTS to prepare a 
	 *  REJECT message.
	 */
	case SIR_REJECT_TO_SEND:
		sym_print_msg(cp, "M_REJECT to send for ", np->msgin);
		np->msgout[0] = M_REJECT;
		goto out;
	/*
	 *  We have been ODD at the end of a DATA IN 
	 *  transfer and the device didn't send a 
	 *  IGNORE WIDE RESIDUE message.
	 *  It is a data overrun condition.
	 */
	case SIR_SWIDE_OVERRUN:
		if (cp) {
			OUTONB(np, HF_PRT, HF_EXT_ERR);
			cp->xerr_status |= XE_SWIDE_OVRUN;
		}
		goto out;
	/*
	 *  We have been ODD at the end of a DATA OUT 
	 *  transfer.
	 *  It is a data underrun condition.
	 */
	case SIR_SODL_UNDERRUN:
		if (cp) {
			OUTONB(np, HF_PRT, HF_EXT_ERR);
			cp->xerr_status |= XE_SODL_UNRUN;
		}
		goto out;
	/*
	 *  The device wants us to tranfer more data than 
	 *  expected or in the wrong direction.
	 *  The number of extra bytes is in scratcha.
	 *  It is a data overrun condition.
	 */
	case SIR_DATA_OVERRUN:
		if (cp) {
			OUTONB(np, HF_PRT, HF_EXT_ERR);
			cp->xerr_status |= XE_EXTRA_DATA;
			cp->extra_bytes += INL(np, nc_scratcha);
		}
		goto out;
	/*
	 *  The device switched to an illegal phase (4/5).
	 */
	case SIR_BAD_PHASE:
		if (cp) {
			OUTONB(np, HF_PRT, HF_EXT_ERR);
			cp->xerr_status |= XE_BAD_PHASE;
		}
		goto out;
	/*
	 *  We received a message.
	 */
	case SIR_MSG_RECEIVED:
		if (!cp)
			goto out_stuck;
		switch (np->msgin [0]) {
		/*
		 *  We received an extended message.
		 *  We handle MODIFY DATA POINTER, SDTR, WDTR 
		 *  and reject all other extended messages.
		 */
		case M_EXTENDED:
			switch (np->msgin [2]) {
			case M_X_MODIFY_DP:
				if (DEBUG_FLAGS & DEBUG_POINTER)
					sym_print_msg(cp, NULL, np->msgin);
				tmp = (np->msgin[3]<<24) + (np->msgin[4]<<16) + 
				      (np->msgin[5]<<8)  + (np->msgin[6]);
				sym_modify_dp(np, tp, cp, tmp);
				return;
			case M_X_SYNC_REQ:
				sym_sync_nego(np, tp, cp);
				return;
			case M_X_PPR_REQ:
				sym_ppr_nego(np, tp, cp);
				return;
			case M_X_WIDE_REQ:
				sym_wide_nego(np, tp, cp);
				return;
			default:
				goto out_reject;
			}
			break;
		/*
		 *  We received a 1/2 byte message not handled from SCRIPTS.
		 *  We are only expecting MESSAGE REJECT and IGNORE WIDE 
		 *  RESIDUE messages that haven't been anticipated by 
		 *  SCRIPTS on SWIDE full condition. Unanticipated IGNORE 
		 *  WIDE RESIDUE messages are aliased as MODIFY DP (-1).
		 */
		case M_IGN_RESIDUE:
			if (DEBUG_FLAGS & DEBUG_POINTER)
				sym_print_msg(cp, NULL, np->msgin);
			if (cp->host_flags & HF_SENSE)
				OUTL_DSP(np, SCRIPTA_BA(np, clrack));
			else
				sym_modify_dp(np, tp, cp, -1);
			return;
		case M_REJECT:
			if (INB(np, HS_PRT) == HS_NEGOTIATE)
				sym_nego_rejected(np, tp, cp);
			else {
				sym_print_addr(cp->cmd,
					"M_REJECT received (%x:%x).\n",
					scr_to_cpu(np->lastmsg), np->msgout[0]);
			}
			goto out_clrack;
			break;
		default:
			goto out_reject;
		}
		break;
	/*
	 *  We received an unknown message.
	 *  Ignore all MSG IN phases and reject it.
	 */
	case SIR_MSG_WEIRD:
		sym_print_msg(cp, "WEIRD message received", np->msgin);
		OUTL_DSP(np, SCRIPTB_BA(np, msg_weird));
		return;
	/*
	 *  Negotiation failed.
	 *  Target does not send us the reply.
	 *  Remove the HS_NEGOTIATE status.
	 */
	case SIR_NEGO_FAILED:
		OUTB(np, HS_PRT, HS_BUSY);
	/*
	 *  Negotiation failed.
	 *  Target does not want answer message.
	 */
	case SIR_NEGO_PROTO:
		sym_nego_default(np, tp, cp);
		goto out;
	}

out:
	OUTONB_STD();
	return;
out_reject:
	OUTL_DSP(np, SCRIPTB_BA(np, msg_bad));
	return;
out_clrack:
	OUTL_DSP(np, SCRIPTA_BA(np, clrack));
	return;
out_stuck:
	return;
}

/*
 *  Acquire a control block
 */
struct sym_ccb *sym_get_ccb (struct sym_hcb *np, struct scsi_cmnd *cmd, u_char tag_order)
{
	u_char tn = cmd->device->id;
	u_char ln = cmd->device->lun;
	struct sym_tcb *tp = &np->target[tn];
	struct sym_lcb *lp = sym_lp(tp, ln);
	u_short tag = NO_TAG;
	SYM_QUEHEAD *qp;
	struct sym_ccb *cp = NULL;

	/*
	 *  Look for a free CCB
	 */
	if (sym_que_empty(&np->free_ccbq))
		sym_alloc_ccb(np);
	qp = sym_remque_head(&np->free_ccbq);
	if (!qp)
		goto out;
	cp = sym_que_entry(qp, struct sym_ccb, link_ccbq);

	{
		/*
		 *  If we have been asked for a tagged command.
		 */
		if (tag_order) {
			/*
			 *  Debugging purpose.
			 */
#ifndef SYM_OPT_HANDLE_DEVICE_QUEUEING
			if (lp->busy_itl != 0)
				goto out_free;
#endif
			/*
			 *  Allocate resources for tags if not yet.
			 */
			if (!lp->cb_tags) {
				sym_alloc_lcb_tags(np, tn, ln);
				if (!lp->cb_tags)
					goto out_free;
			}
			/*
			 *  Get a tag for this SCSI IO and set up
			 *  the CCB bus address for reselection, 
			 *  and count it for this LUN.
			 *  Toggle reselect path to tagged.
			 */
			if (lp->busy_itlq < SYM_CONF_MAX_TASK) {
				tag = lp->cb_tags[lp->ia_tag];
				if (++lp->ia_tag == SYM_CONF_MAX_TASK)
					lp->ia_tag = 0;
				++lp->busy_itlq;
#ifndef SYM_OPT_HANDLE_DEVICE_QUEUEING
				lp->itlq_tbl[tag] = cpu_to_scr(cp->ccb_ba);
				lp->head.resel_sa =
					cpu_to_scr(SCRIPTA_BA(np, resel_tag));
#endif
#ifdef SYM_OPT_LIMIT_COMMAND_REORDERING
				cp->tags_si = lp->tags_si;
				++lp->tags_sum[cp->tags_si];
				++lp->tags_since;
#endif
			}
			else
				goto out_free;
		}
		/*
		 *  This command will not be tagged.
		 *  If we already have either a tagged or untagged 
		 *  one, refuse to overlap this untagged one.
		 */
		else {
			/*
			 *  Debugging purpose.
			 */
#ifndef SYM_OPT_HANDLE_DEVICE_QUEUEING
			if (lp->busy_itl != 0 || lp->busy_itlq != 0)
				goto out_free;
#endif
			/*
			 *  Count this nexus for this LUN.
			 *  Set up the CCB bus address for reselection.
			 *  Toggle reselect path to untagged.
			 */
			++lp->busy_itl;
#ifndef SYM_OPT_HANDLE_DEVICE_QUEUEING
			if (lp->busy_itl == 1) {
				lp->head.itl_task_sa = cpu_to_scr(cp->ccb_ba);
				lp->head.resel_sa =
				      cpu_to_scr(SCRIPTA_BA(np, resel_no_tag));
			}
			else
				goto out_free;
#endif
		}
	}
	/*
	 *  Put the CCB into the busy queue.
	 */
	sym_insque_tail(&cp->link_ccbq, &np->busy_ccbq);
#ifdef SYM_OPT_HANDLE_DEVICE_QUEUEING
	if (lp) {
		sym_remque(&cp->link2_ccbq);
		sym_insque_tail(&cp->link2_ccbq, &lp->waiting_ccbq);
	}

#endif
	cp->to_abort = 0;
	cp->id DSN
 * @NL80211_MPATH_FLAG_FIXED: the mesh path has been manually set
 * @NL80211_MPATH_FLAG_RESOLVED: the mesh path discovery process succeeded
 */
enum nl80211_mpath_flags {
	NL80211_MPATH_FLAG_ACTIVE =	1<<0,
	NL80211_MPATH_FLAG_RESOLVING =	1<<1,
	NL80211_MPATH_FLAG_DSN_VALID =	1<<2,
	NL80211_MPATH_FLAG_FIXED =	1<<3,
	NL80211_MPATH_FLAG_RESOLVED =	1<<4,
};

/**
 * enum nl80211_mpath_info - mesh path information
 *
 * These attribute types are used with %NL80211_ATTR_MPATH_INFO when getting
 * information about a mesh path.
 *
 * @__NL80211_MPATH_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_ATTR_MPATH_FRAME_QLEN: number of queued frames for this destination
 * @NL80211_ATTR_MPATH_DSN: destination sequence number
 * @NL80211_ATTR_MPATH_METRIC: metric (cost) of this mesh path
 * @NL80211_ATTR_MPATH_EXPTIME: expiration time for the path, in msec from now
 * @NL80211_ATTR_MPATH_FLAGS: mesh path flags, enumerated in
 * 	&enum nl80211_mpath_flags;
 * @NL80211_ATTR_MPATH_DISCOVERY_TIMEOUT: total path discovery timeout, in msec
 * @NL80211_ATTR_MPATH_DISCOVERY_RETRIES: mesh path discovery retries
 */
enum nl80211_mpath_info {
	__NL80211_MPATH_INFO_INVALID,
	NL80211_MPATH_INFO_FRAME_QLEN,
	NL80211_MPATH_INFO_DSN,
	NL80211_MPATH_INFO_METRIC,
	NL80211_MPATH_INFO_EXPTIME,
	NL80211_MPATH_INFO_FLAGS,
	NL80211_MPATH_INFO_DISCOVERY_TIMEOUT,
	NL80211_MPATH_INFO_DISCOVERY_RETRIES,

	/* keep last */
	__NL80211_MPATH_INFO_AFTER_LAST,
	NL80211_MPATH_INFO_MAX = __NL80211_MPATH_INFO_AFTER_LAST - 1
};

/**
 * enum nl80211_band_attr - band attributes
 * @__NL80211_BAND_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_BAND_ATTR_FREQS: supported frequencies in this band,
 *	an array of nested frequency attributes
 * @NL80211_BAND_ATTR_RATES: supported bitrates in this band,
 *	an array of nested bitrate attributes
 * @NL80211_BAND_ATTR_HT_MCS_SET: 16-byte attribute containing the MCS set as
 *	defined in 802.11n
 * @NL80211_BAND_ATTR_HT_CAPA: HT capabilities, as in the HT information IE
 * @NL80211_BAND_ATTR_HT_AMPDU_FACTOR: A-MPDU factor, as in 11n
 * @NL80211_BAND_ATTR_HT_AMPDU_DENSITY: A-MPDU density, as in 11n
 */
enum nl80211_band_attr {
	__NL80211_BAND_ATTR_INVALID,
	NL80211_BAND_ATTR_FREQS,
	NL80211_BAND_ATTR_RATES,

	NL80211_BAND_ATTR_HT_MCS_SET,
	NL80211_BAND_ATTR_HT_CAPA,
	NL80211_BAND_ATTR_HT_AMPDU_FACTOR,
	NL80211_BAND_ATTR_HT_AMPDU_DENSITY,

	/* keep last */
	__NL80211_BAND_ATTR_AFTER_LAST,
	NL80211_BAND_ATTR_MAX = __NL80211_BAND_ATTR_AFTER_LAST - 1
};

#define NL80211_BAND_ATTR_HT_CAPA NL80211_BAND_ATTR_HT_CAPA

/**
 * enum nl80211_frequency_attr - frequency attributes
 * @NL80211_FREQUENCY_ATTR_FREQ: Frequency in MHz
 * @NL80211_FREQUENCY_ATTR_DISABLED: Channel is disabled in current
 *	regulatory domain.
 * @NL80211_FREQUENCY_ATTR_PASSIVE_SCAN: Only passive scanning is
 *	permitted on this channel in current regulatory domain.
 * @NL80211_FREQUENCY_ATTR_NO_IBSS: IBSS networks are not permitted
 *	on this channel in current regulatory domain.
 * @NL80211_FREQUENCY_ATTR_RADAR: Radar detection is mandatory
 *	on this channel in current regulatory domain.
 * @NL80211_FREQUENCY_ATTR_MAX_TX_POWER: Maximum transmission power in mBm
 *	(100 * dBm).
 */
enum nl80211_frequency_attr {
	__NL80211_FREQUENCY_ATTR_INVALID,
	NL80211_FREQUENCY_ATTR_FREQ,
	NL80211_FREQUENCY_ATTR_DISABLED,
	NL80211_FREQUENCY_ATTR_PASSIVE_SCAN,
	NL80211_FREQUENCY_ATTR_NO_IBSS,
	NL80211_FREQUENCY_ATTR_RADAR,
	NL80211_FREQUENCY_ATTR_MAX_TX_POWER,

	/* keep last */
	__NL80211_FREQUENCY_ATTR_AFTER_LAST,
	NL80211_FREQUENCY_ATTR_MAX = __NL80211_FREQUENCY_ATTR_AFTER_LAST - 1
};

#define NL80211_FREQUENCY_ATTR_MAX_TX_POWER NL80211_FREQUENCY_ATTR_MAX_TX_POWER

/**
 * enum nl80211_bitrate_attr - bitrate attributes
 * @NL80211_BITRATE_ATTR_RATE: Bitrate in units of 100 kbps
 * @NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE: Short preamble supported
 *	in 2.4 GHz band.
 */
enum nl80211_bitrate_attr {
	__NL80211_BITRATE_ATTR_INVALID,
	NL80211_BITRATE_ATTR_RATE,
	NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE,

	/* keep last */
	__NL80211_BITRATE_ATTR_AFTER_LAST,
	NL8021this checking looks paranoid. */
	/*
	 *  Check some alignments required by the chip.
	 */	
	assert (((offsetof(struct sym_reg, nc_sxfer) ^
		offsetof(struct sym_tcb, head.sval)) &3) == 0);
	assert (((offsetof(struct sym_reg, nc_scntl3) ^
		offsetof(struct sym_tcb, head.wval)) &3) == 0);
#endif
}

/*
 *  Lun control block allocation and initialization.
 */
struct sym_lcb *sym_alloc_lcb (struct sym_hcb *np, u_char tn, u_char ln)
{
	struct sym_tcb *tp = &np->target[tn];
	struct sym_lcb *lp = NULL;

	/*
	 *  Initialize the target control block if not yet.
	 */
	sym_init_tcb (np, tn);

	/*
	 *  Allocate the LCB bus address array.
	 *  Compute the bus address of this table.
	 */
	if (ln && !tp->luntbl) {
		int i;

		tp->luntbl = sym_calloc_dma(256, "LUNTBL");
		if (!tp->luntbl)
			goto fail;
		for (i = 0 ; i < 64 ; i++)
			tp->luntbl[i] = cpu_to_scr(vtobus(&np->badlun_sa));
		tp->head.luntbl_sa = cpu_to_scr(vtobus(tp->luntbl));
	}

	/*
	 *  Allocate the table of pointers for LUN(s) > 0, if needed.
	 */
	if (ln && !tp->lunmp) {
		tp->lunmp = kcalloc(SYM_CONF_MAX_LUN, sizeof(struct sym_lcb *),
				GFP_ATOMIC);
		if (!tp->lunmp)
			goto fail;
	}

	/*
	 *  Allocate the lcb.
	 *  Make it available to the chip.
	 */
	lp = sym_calloc_dma(sizeof(struct sym_lcb), "LCB");
	if (!lp)
		goto fail;
	if (ln) {
		tp->lunmp[ln] = lp;
		tp->luntbl[ln] = cpu_to_scr(vtobus(lp));
	}
	else {
		tp->lun0p = lp;
		tp->head.lun0_sa = cpu_to_scr(vtobus(lp));
	}
	tp->nlcb++;

	/*
	 *  Let the itl task point to error handling.
	 */
	lp->head.itl_task_sa = cpu_to_scr(np->bad_itl_ba);

	/*
	 *  Set the reselect pattern to our default. :)
	 */
	lp->head.resel_sa = cpu_to_scr(SCRIPTB_BA(np, resel_bad_lun));

	/*
	 *  Set user capabilities.
	 */
	lp->user_flags = tp->usrflags & (SYM_DISC_ENABLED | SYM_TAGS_ENABLED);

#ifdef SYM_OPT_HANDLE_DEVICE_QUEUEING
	/*
	 *  Initialize device queueing.
	 */
	sym_que_init(&lp->waiting_ccbq);
	sym_que_init(&lp->started_ccbq);
	lp->started_max   = SYM_CONF_MAX_TASK;
	lp->started_limit = SYM_CONF_MAX_TASK;
#endif

fail:
	return lp;
}

/*
 *  Allocate LCB resources for tagged command queuing.
 */
static void sym_alloc_lcb_tags (struct sym_hcb *np, u_char tn, u_char ln)
{
	struct sym_tcb *tp = &np->target[tn];
	struct sym_lcb *lp = sym_lp(tp, ln);
	int i;

	/*
	 *  Allocate the task table and and the tag allocation 
	 *  circular buffer. We want both or none.
	 */
	lp->itlq_tbl = sym_calloc_dma(SYM_CONF_MAX_TASK*4, "ITLQ_TBL");
	if (!lp->itlq_tbl)
		goto fail;
	lp->cb_tags = kcalloc(SYM_CONF_MAX_TASK, 1, GFP_ATOMIC);
	if (!lp->cb_tags) {
		sym_mfree_dma(lp->itlq_tbl, SYM_CONF_MAX_TASK*4, "ITLQ_TBL");
		lp->itlq_tbl = NULL;
		goto fail;
	}

	/*
	 *  Initialize the task table with invalid entries.
	 */
	for (i = 0 ; i < SYM_CONF_MAX_TASK ; i++)
		lp->itlq_tbl[i] = cpu_to_scr(np->notask_ba);

	/*
	 *  Fill up the tag buffer with tag numbers.
	 */
	for (i = 0 ; i < SYM_CONF_MAX_TASK ; i++)
		lp->cb_tags[i] = i;

	/*
	 *  Make the task table available to SCRIPTS, 
	 *  And accept tagged commands now.
	 */
	lp->head.itlq_tbl_sa = cpu_to_scr(vtobus(lp->itlq_tbl));

	return;
fail:
	return;
}

/*
 *  Lun control block deallocation. Returns the number of valid remaing LCBs
 *  for the target.
 */
int sym_free_lcb(struct sym_hcb *np, u_char tn, u_char ln)
{
	struct sym_tcb *tp = &np->target[tn];
	struct sym_lcb *lp = sym_lp(tp, ln);

	tp->nlcb--;

	if (ln) {
		if (!tp->nlcb) {
			kfree(tp->lunmp);
			sym_mfree_dma(tp->luntbl, 256, "LUNTBL");
			tp->lunmp = NULL;
			tp->luntbl = NULL;
			tp->head.luntbl_sa = cpu_to_scr(vtobus(np->badluntbl));
		} else {
			tp->luntbl[ln] = cpu_to_scr(vtobus(&np->badlun_sa));
			tp->lunmp[ln] = NULL;
		}
	} else {
		tp->lun0p = NULL;
		tp->head.lun0_sa = cpu_to_scr(vtobus(&np->badlun_sa));
	}

	if (lp->itlq_tbl) {
		sym_mfree_dma(lp->itlq_tbl, SYM_CONF_MAX_TASK*4, "ITLQ_TBL");
		kfree(lp->cb_tags);
	}

	sym_mfree_dma(lp, sizeof(*lp), "LCB");

	return tp->nlcb;
}

/*
 *  Queue a SCSI IO to the controller.
 */
int sym_queue_scsiio(struct sym_hcb *np, struct scsi_cmnd *cmd, struct sym_ccb *cp)
{
	struct scsi_device *sdev = cmd->device;
	struct sym_tcb *tp;
	struct sym_lcb *lp;
	u_char	*msgptr;
	u_int   msglen;
	int can_disconnect;

	/*
	 *  Keep track of the IO in our CCB.
	 */
	cp->cmd = cmd;

	/*
	 *  Retrieve the target descriptor.
	 */
	tp = &np->target[cp->target];

	/*
	 *  Retrieve the lun descriptor.
	 */
	lp = sym_lp(tp, sdev->lun);

	can_disconnect = (cp->tag != NO_TAG) ||
		(lp && (lp->curr_flags & SYM_DISC_ENABLED));

	msgptr = cp->scsi_smsg;
	msglen = 0;
	msgptr[msglen++] = IDENTIFY(can_disconnect, sdev->lun);

	/*
	 *  Build the tag message if present.
	 */
	if (cp->tag != NO_TAG) {
		u_char order = cp->order;

		switch(order) {
		case M_ORDERED_TAG:
			break;
		case M_HEAD_TAG:
			break;
		default:
			order = M_SIMPLE_TAG;
		}
#ifdef SYM_OPT_LIMIT_COMMAND_REORDERING
		/*
		 *  Avoid too much reordering of SCSI commands.
		 *  The algorithm tries to prevent completion of any 
		 *  tagged command from being delayed against more 
		 *  than 3 times the max number of queued commands.
		 */
		if (lp && lp->tags_since > 3*SYM_CONF_MAX_TAG) {
			lp->tags_si = !(lp->tags_si);
			if (lp->tags_sum[lp->tags_si]) {
				order = M_ORDERED_TAG;
				if ((DEBUG_FLAGS & DEBUG_TAGS)||sym_verbose>1) {
					sym_print_addr(cmd,
						"ordered tag forced.\n");
				}
			}
			lp->tags_since = 0;
		}
#endif
		msgptr[msglen++] = order;

		/*
		 *  For less than 128 tags, actual tags are numbered 
		 *  1,3,5,..2*MAXTAGS+1,since we may have to deal 
		 *  with devices that have problems with #TAG 0 or too 
		 *  great #TAG numbers. For more tags (up to 256), 
		 *  we use directly our tag number.
		 */
#if SYM_CONF_MAX_TASK > (512/4)
		msgptr[msglen++] = cp->tag;
#else
		msgptr[msglen++] = (cp->tag << 1) + 1;
#endif
	}

	/*
	 *  Build a negotiation message if needed.
	 *  (nego_status is filled by sym_prepare_nego())
	 *
	 *  Always negotiate on INQUIRY and REQUEST SENSE.
	 *
	 */
	cp->nego_status = 0;
	if ((tp->tgoal.check_nego ||
	     cmd->cmnd[0] == INQUIRY || cmd->cmnd[0] == REQUEST_SENSE) &&
	    !tp->nego_cp && lp) {
		msglen += sym_prepare_nego(np, cp, msgptr + msglen);
	}

	/*
	 *  Startqueue
	 */
	cp->phys.head.go.start   = cpu_to_scr(SCRIPTA_BA(np, select));
	cp->phys.head.go.restart = cpu_to_scr(SCRIPTA_BA(np, resel_dsa));

	/*
	 *  select
	 */
	cp->phys.select.sel_id		= cp->target;
	cp->phys.select.sel_scntl3	= tp->head.wval;
	cp->phys.select.sel_sxfer	= tp->head.sval;
	cp->phys.select.sel_scntl4	= tp->head.uval;

	/*
	 *  message
	 */
	cp->phys.smsg.addr	= CCB_BA(cp, scsi_smsg);
	cp->phys.smsg.size	= cpu_to_scr(msglen);

	/*
	 *  status
	 */
	cp->host_xflags		= 0;
	cp->host_status		= cp->nego_status ? HS_NEGOTIATE : HS_BUSY;
	cp->ssss_status		= S_ILLEGAL;
	cp->xerr_status		= 0;
	cp->host_flags		= 0;
	cp->extra_bytes		= 0;

	/*
	 *  extreme data pointer.
	 *  shall be positive, so -1 is lower than lowest.:)
	 */
	cp->ext_sg  = -1;
	cp->ext_ofs = 0;

	/*
	 *  Build the CDB and DATA descriptor block 
	 *  and start the IO.
	 */
	return sym_setup_data_and_start(np, cmd, cp);
}

/*
 *  Reset a SCSI target (all LUNs of this target).
 */
int sym_reset_scsi_target(struct sym_hcb *np, int target)
{
	struct sym_tcb *tp;

	if (target == np->myaddr || (u_int)target >= SYM_CONF_MAX_TARGET)
		return -1;

	tp = &np->target[target];
	tp->to_reset = 1;

	np->istat_sem = SEM;
	OUTB(np, nc_istat, SIGP|SEM);

	return 0;
}

/*
 *  Abort a SCSI IO.
 */
static int sym_abort_ccb(struct sym_hcb *np, struct sym_ccb *cp, int timed_out)
{
	/*
	 *  Check that the IO is active.
	 */
	if (!cp || !cp->host_status || cp->host_status == HS_WAIT)
		return -1;

	/*
	 *  If a previous abort didn't succeed in time,
	 *  perform a BUS reset.
	 */
	if (cp->to_abort) {
		sym_reset_scsi_bus(np, 1);
		return 0;
	}

	/*
	 *  Mark the CCB for abort and allow time for.
	 */
	cp->to_abort = timed_out ? 2 : 1;

	/*
	 *  Tell the SCRIPTS processor to stop and synchronize with us.
	 */
	np->istat_sem = SEM;
	OUTB(np, nc_istat, SIGP|SEM);
	return 0;
}

int sym_abort_scsiio(struct sym_hcb *np, struct scsi_cmnd *cmd, int timed_out)
{
	struct sym_ccb *cp;
	SYM_QUEHEAD *qp;

	/*
	 *  Look up our CCB control block.
	 */
	cp = NULL;
	FOR_EACH_QUEUED_ELEMENT(&np->busy_ccbq, qp) {
		struct sym_ccb *cp2 = sym_que_entry(qp, struct sym_ccb, link_ccbq);
		if (cp2->cmd == cmd) {
			cp = cp2;
			break;
		}
	}

	return sym_abort_ccb(np, cp, timed_out);
}

/*
 *  Complete execution of a SCSI command with extended 
 *  error, SCSI status error, or having been auto-sensed.
 *
 *  The SCRIPTS processor is not running there, so we 
 *  can safely access IO registers and remove JOBs from  
 *  the START queue.
 *  SCRATCHA is assumed to have been loaded with STARTPOS 
 *  before the SCRIPTS called the C code.
 */
void sym_complete_error(struct sym_hcb *np, struct sym_ccb *cp)
{
	struct scsi_device *sdev;
	struct scsi_cmnd *cmd;
	struct sym_tcb *tp;
	struct sym_lcb *lp;
	int resid;
	int i;

	/*
	 *  Paranoid check. :)
	 */
	if (!cp || !cp->cmd)
		return;

	cmd = cp->cmd;
	sdev = cmd->device;
	if (DEBUG_FLAGS & (DEBUG_TINY|DEBUG_RESULT)) {
		dev_info(&sdev->sdev_gendev, "CCB=%p STAT=%x/%x/%x\n", cp,
			cp->host_status, cp->ssss_status, cp->host_flags);
	}

	/*
	 *  Get target and lun pointers.
	 */
	tp = &np->target[cp->target];
	lp = sym_lp(tp, sdev->lun);

	/*
	 *  Check for extended errors.
	 */
	if (cp->xerr_status) {
		if (sym_verbose)
			sym_print_xerr(cmd, cp->xerr_status);
		if (cp->host_status == HS_COMPLETE)
			cp->host_status = HS_COMP_ERR;
	}

	/*
	 *  Calculate the residual.
	 */
	resid = sym_compute_residual(np, cp);

	if (!SYM_SETUP_RESIDUAL_SUPPORT) {/* If user does not want residuals */
		resid  = 0;		 /* throw them away. :)		    */
		cp->sv_resid = 0;
	}
#ifdef DEBUG_2_0_X
if (resid)
	printf("XXXX RESID= %d - 0x%x\n", resid, resid);
#endif

	/*
	 *  Dequeue all queued CCBs for that device 
	 *  not yet started by SCRIPTS.
	 */
	i = (INL(np, nc_scratcha) - np->squeue_ba) / 4;
	i = sym_dequeue_from_squeue(np, i, cp->target, sdev->lun, -1);

	/*
	 *  Restart the SCRIPTS processor.
	 */
	OUTL_DSP(np, SCRIPTA_BA(np, start));

#ifdef SYM_OPT_HANDLE_DEVICE_QUEUEING
	if (cp->host_status == HS_COMPLETE &&
	    cp->ssss_status == S_QUEUE_FULL) {
		if (!lp || lp->started_tags - i < 2)
			goto weirdness;
		/*
		 *  Decrease queue depth as needed.
		 */
		lp->started_max = lp->started_tags - i - 1;
		lp->num_sgood = 0;

		if (sym_verbose >= 2) {
			sym_print_addr(cmd, " queue depth is now %d\n",
					lp->started_max);
		}

		/*
		 *  Repair the CCB.
		 */
		cp->host_status = HS_BUSY;
		cp->ssss_status = S_ILLEGAL;

		/*
		 *  Let's requeue it to device.
		 */
		sym_set_cam_status(cmd, DID_SOFT_ERROR);
		goto finish;
	}
weirdness:
#endif
	/*
	 *  Build result in CAM ccb.
	 */
	sym_set_cam_result_error(np, cp, resid);

#ifdef SYM_OPT_HANDLE_DEVICE_QUEUEING
finish:
#endif
	/*
	 *  Add this one to the COMP queue.
	 */
	sym_remque(&cp->link_ccbq);
	sym_insque_head(&cp->link_ccbq, &np->comp_ccbq);

	/*
	 *  Complete all those commands with either error 
	 *  or re