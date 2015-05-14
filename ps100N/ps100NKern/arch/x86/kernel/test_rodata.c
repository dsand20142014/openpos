go(struct sym_hcb *np, struct sym_tcb *tp, struct sym_ccb *cp)
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
	spi_populate_ppr_msg(np->msgout, per, of