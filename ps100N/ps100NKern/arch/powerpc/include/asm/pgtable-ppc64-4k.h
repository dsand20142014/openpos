 boot.
	**
	**---------------------------------------------
	*/
	if ((cmd->cmnd[0] == 0 || cmd->cmnd[0] == 0x12) && 
	    (tp->usrflag & UF_NOSCAN)) {
		tp->usrflag &= ~UF_NOSCAN;
		return DID_BAD_TARGET;
	}

	if (DEBUG_FLAGS & DEBUG_TINY) {
		PRINT_ADDR(cmd, "CMD=%x ", cmd->cmnd[0]);
	}

	/*---------------------------------------------------
	**
	**	Assign a ccb / bind cmd.
	**	If resetting, shorten settle_time if necessary
	**	in order to avoid spurious timeouts.
	**	If resetting or no free ccb,
	**	insert cmd into the waiting list.
	**
	**----------------------------------------------------
	*/
	if (np->settle_time && cmd->request->timeout >= HZ) {
		u_long tlimit = jiffies + cmd->request->timeout - HZ;
		if (time_after(np->settle_time, tlimit))
			np->settle_time = tlimit;
	}

	if (np->settle_time || !(cp=ncr_get_ccb (np, cmd))) {
		insert_into_waiting_list(np, cmd);
		return(DID_OK);
	}
	cp->cmd = cmd;

	/*----------------------------------------------------
	**
	**	Build the identify / tag / sdtr message
	**
	**----------------------------------------------------
	*/

	idmsg = IDENTIFY(0, sdev->lun);

	if (cp ->tag != NO_TAG ||
		(cp != np->ccb && np->disc && !(tp->usrflag & UF_NODISC)))
		idmsg |= 0x40;

	msgptr = cp->scsi_smsg;
	msglen = 0;
	msgptr[msglen++] = idmsg;

	if (cp->tag != NO_TAG) {
		char order = np->order;

		/*
		**	Force ordered tag if necessary to avoid timeouts 
		**	and to preserve interactivity.
		*/
		if (lp && time_after(jiffies, lp->tags_stime)) {
			if (lp->tags_smap) {
				order = ORDERED_QUEUE_TAG;
				if ((DEBUG_FLAGS & DEBUG_TAGS)||bootverbose>2){ 
					PRINT_ADDR(cmd,
						"ordered tag forced.\n");
				}
			}
			lp->tags_stime = jiffies + 3*HZ;
			lp->tags_smap = lp->tags_umap;
		}

		if (order == 0) {
			/*
			**	Ordered write ops, unordered read ops.
			*/
			switch (cmd->cmnd[0]) {
			case 0x08:  /* READ_SMALL (6) */
			case 0x28:  /* READ_BIG  (10) */
			case 0xa8:  /* READ_HUGE (12) */
				order = SIMPLE_QUEUE_TAG;
				break;
			default:
				order = ORDERED_QUEUE_TAG;
			}
		}
		msgptr[msglen++] = order;
		/*
		**	Actual tags are numbered 1,3,5,..2*MAXTAGS+1,
		**	since we may have to deal with devices that have 
		**	problems with #TAG 0 or too great #TAG numbers.
		*/
		msgptr[msglen++] = (cp->tag << 1) + 1;
	}

	/*----------------------------------------------------
	**
	**	Build the data descriptors
	**
	**----------------------------------------------------
	*/

	direction = cmd->sc_data_direction;
	if (direction != DMA_NONE) {
		segments = ncr_scatter(np, cp, cp->cm