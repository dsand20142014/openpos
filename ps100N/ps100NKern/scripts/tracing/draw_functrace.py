		goto error;
	cmd->rccb.bCommandType = UWB_RC_CET_GENERAL;
	cmd->rccb.wCommand = cpu_to_le16(UWB_RC_CMD_SET_DRP_IE);
	cmd->wIELength = num_bytes;
	IEDataptr = (u8 *)&cmd->IEData[0];

	/* FIXME: DRV avail IE is not always needed */
	/* put DRP avail IE first */
	memcpy(IEDataptr, &rc->drp_avail.ie, sizeof(rc->drp_avail.ie));
	IEDataptr += sizeof(struct uwb_ie_drp_avail);

	/* Next traverse all reservations to place IEs in allocated memory. */
	list_for_each_entry(rsv, &rc->reservations, rc_node) {
		if (rsv->drp_ie != NULL) {
			memcpy(IEDataptr, rsv->drp_ie,
			       rsv->drp_ie->hdr.length + 2);
			IEDataptr += rsv->drp_ie->hdr.length + 2;
			
			if (uwb_rsv_has_two_drp_ies(rsv) &&
				(rsv->mv.companion_drp_ie != NULL)) {
				mv = &rsv->mv;
				memcpy(IEDataptr, mv->companion_drp_ie,
				       mv->companion_drp_ie->hdr.length + 2);
				IEDataptr += mv->companion_drp_ie->hdr.length + 2;	
			}
		}
	}

	result = uwb_rc_cmd_async(rc, "SET-DRP-IE", &cmd->rccb, sizeof(*cmd) + num_bytes,
				  UWB_RC_CET_GENERAL, UWB_RC_CMD_SET_DRP_IE,
				  uwb_rc_set_drp_cmd_done, NULL);
	
	rc->set_drp_ie_pending = 1;

	kfree(cmd);
error:
	return result;
}

/*
 * Evaluate the action to perform using conflict resolution rules
 *
 * Return a uwb_drp_conflict_action.
 */
static int evaluate_conflict_action(struct uwb_ie_drp *ext_drp_ie, int ext_beacon_slot,
				    struct uwb_rsv *rsv, int our_status)
{
	int our_tie_breaker = rsv->tiebreaker;
	int our_type        = rsv->type;
	int our_beacon_slot = rsv->rc->uwb_dev.beacon_slot;

	int ext_tie_breaker = uwb_ie_drp_tiebreaker(ext_drp_ie);
	int ext_status      = uwb_ie_drp_status(ext_drp_ie);
	int ext_type        = uwb_ie_drp_type(ext_drp_ie);
	
	
	/* [ECMA-368 2nd Edition] 17.4.6 */
	if (ext_type == UWB_DRP_TYPE_PCA && our_type == UWB_DRP_TYPE_PCA) {
		return UWB_DRP_CONFLICT_MANTAIN;
	}

	/* [ECMA-368 2nd Edition] 17.4.6-1 */
	if (our_type == UWB_DRP_TYPE_ALIEN_BP) {
		return UWB_DRP_CONFLICT_MANTAIN;
	}
	
	/* [ECMA-368 2nd Edition] 17.4.6-2 */
	if (ext_type == UWB_DRP_TYPE_ALIEN_BP) {
		/* here we know our_type != UWB_DRP_TYPE_ALIEN_BP */
		return UWB_DRP_CONFLICT_ACT1;
	}

	/* [ECMA-368 2nd Edition] 17.4.6-3 */
	if (our_status == 0 && ext_status == 1) {
		return UWB_DRP_CONFLICT_ACT2;
	}

	/* [ECMA-368 2nd Edition] 17.4.6-4 */
	if (our_status == 1 && ext_status == 0) {
		return UWB_DRP_CONFLICT_MANTAIN;
	}

	/* [ECMA-368 2nd Edition] 17.4.6-5a */
	if (our_tie_breaker == ext_tie_breaker &&
	    our_beacon_slot <  ext_beacon_slot) {
		return UWB_DRP_CONFLICT_MANTAIN;
	}

	/* [ECMA-368 2nd Edition] 17.4.6-5b */
	if (our_tie_breaker != ext_tie_breaker &&
	    our_beacon_slot >  ext_beacon_slot) {
		return UWB_DRP_CONFLICT_MANTAIN;
	}
	
	if (our_status == 0) {
		if (our_tie_breaker == ext_tie_breaker) {
			/* [ECMA-368 2nd Edition] 17.4.6-6a */
			if (our_beacon_slot > ext_beacon_slot) {
				return UWB_DRP_CONFLICT_ACT2;
			}
		} else  {
			/* [ECMA-368 2nd Edition] 17.4.6-6b */
			if (our_beacon_slot < ext_beacon_slot) {
				return UWB_DRP_CONFLICT_ACT2;
			}
		}
	} else {
		if (our_tie_breaker == ext_tie_breaker) {
			/* [ECMA-368 2nd Edition] 17.4.6-7a */
			if (our_beacon_slot > ext_beacon_slot) {
				return UWB_DRP_CONFLICT_ACT3;
			}
		} else {
			/* [ECMA-368 2nd Edition] 17.4.6-7b */
			if (our_beacon_slot < ext_beacon_slot) {
				return UWB_DRP_CONFLICT_ACT3;
			}
		}
	}
	return UWB_DRP_CONFLICT_MANTAIN;
}

static void handle_conflict_normal(struct uwb_ie_drp *drp_ie, 
				   int ext_beacon_slot, 
				   struct uwb_rsv *rsv, 
				   struct uwb_mas_bm *conflicti