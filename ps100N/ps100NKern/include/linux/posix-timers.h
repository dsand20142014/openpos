DRP IEs.
 */
static void uwb_rsv_state_update(struct uwb_rsv *rsv,
				 enum uwb_rsv_state new_state)
{
	rsv->state = new_state;
	rsv->ie_valid = false;

	uwb_rsv_dump("SU", rsv);

	uwb_rsv_stroke_timer(rsv);
	uwb_rsv_sched_update(rsv->rc);
}

static void uwb_rsv_callback(struct uwb_rsv *rsv)
{
	if (rsv->callback)
		rsv->callback(rsv);
}

void uwb_rsv_set_state(struct uwb_rsv *rsv, enum uwb_rsv_state new_state)
{
	struct uwb_rsv_move *mv = &rsv->mv;

	if (rsv->state == new_state) {
		switch (rsv->state) {
		case UWB_RSV_STATE_O_ESTABLISHED:
		case UWB_RSV_STATE_O_MOVE_EXPANDING:
		case UWB_RSV_STATE_O_MOVE_COMBINING:
		case UWB_RSV_STATE_O_MOVE_REDUCING:
		case UWB_RSV_STATE_T_ACCEPTED:
		case UWB_RSV_STATE_T_EXPANDING_ACCEPTED:
		case UWB_RSV_STATE_T_RESIZED:
		case UWB_RSV_STATE_NONE:
			uwb_rsv_stroke_timer(rsv);
			break;
		default:
			/* Expecting a state transition so leave timer
			   as-is. */
			break;
		}
		return;
	}

	uwb_rsv_dump("SC", rsv);

	switch (new_state) {
	case UWB_RSV_STATE_NONE:
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_NONE);
		uwb_rsv_callback(rsv);
		break;
	case UWB_RSV_STATE_O_INITIATED:
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_INITIATED);
		break;
	case UWB_RSV_STATE_O_PENDING:
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_PENDING);
		break;
	case UWB_RSV_STATE_O_MODIFIED:
		/* in the companion there are the MASes to drop */
		bitmap_andnot(rsv->mas.bm, rsv->mas.bm, mv->companion_mas.bm, UWB_NUM_MAS);
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_MODIFIED);
		break;
	case UWB_RSV_STATE_O_ESTABLISHED:
		if (rsv->state == UWB_RSV_STATE_O_MODIFIED
		    || rsv->state == UWB_RSV_STATE_O_MOVE_REDUCING) {
			uwb_drp_avail_release(rsv->rc, &mv->companion_mas);
			rsv->needs_release_companion_mas = false;
		}
		uwb_drp_avail_reserve(rsv->rc, &rsv->mas);
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_ESTABLISHED);
		uwb_rsv_callback(rsv);
		break;
	case UWB_RSV_STATE_O_MOVE_EXPANDING:
		rsv->needs_release_companion_mas = true;
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_MOVE_EXPANDING);
		break;
	case UWB_RSV_STATE_O_MOVE_COMBINING:
		rsv->needs_release_companion_mas = false;
		uwb_drp_avail_reserve(rsv->rc, &mv->companion_mas);
		bitmap_or(rsv->mas.bm, rsv->mas.bm, mv->companion_mas.bm, UWB_NUM_MAS);
		rsv->mas.safe   += mv->companion_mas.safe;
		rsv->mas.unsafe += mv->companion_mas.unsafe;
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_MOVE_COMBINING);
		break;
	case UWB_RSV_STATE_O_MOVE_REDUCING:
		bitmap_andnot(mv->companion_mas.bm, rsv->mas.bm, mv->final_mas.bm, UWB_NUM_MAS);
		rsv->needs_release_companion_mas = true;
		rsv->mas.safe   = mv->final_mas.safe;
		rsv->mas.unsafe = mv->final_mas.unsafe;
		bitmap_copy(rsv->mas.bm, mv->final_mas.bm, UWB_NUM_MAS);
		bitmap_copy(rsv->mas.unsafe_bm, mv->final_mas.unsafe_bm, UWB_NUM_MAS);
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_O_MOVE_REDUCING);
		break;
	case UWB_RSV_STATE_T_ACCEPTED:
	case UWB_RSV_STATE_T_RESIZED:
		rsv->needs_release_companion_mas = false;
		uwb_drp_avail_reserve(rsv->rc, &rsv->mas);
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_T_ACCEPTED);
		uwb_rsv_callback(rsv);
		break;
	case UWB_RSV_STATE_T_DENIED:
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_T_DENIED);
		break;
	case UWB_RSV_STATE_T_CONFLICT:
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_T_CONFLICT);
		break;
	case UWB_RSV_STATE_T_PENDING:
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_T_PENDING);
		break;
	case UWB_RSV_STATE_T_EXPANDING_ACCEPTED:
		rsv->needs_release_companion_mas = true;
		uwb_drp_avail_reserve(rsv->rc, &mv->companion_mas);
		uwb_rsv_state_update(rsv, UWB_RSV_STATE_T_EXPANDING_ACCEPTED);
		break;
	default:
		dev_err(&rsv->rc->uwb_dev.dev, "unhandled state: %s (%d)\n",
			uwb_rsv_state_str(new_state), new_state);
	}
}

static void uwb_rsv_handle_timeout_work(struct work_struct *work)
{
	struct uwb_rsv *rsv = container_of(work, struct uwb_rsv,
					   handle_timeout_work);
	struct uwb_rc *rc = rsv->rc;

	mutex_lock(&rc->rsvs_mutex);

	uwb_rsv_dump("TO", rsv);

	switch (rsv->state) {
	case UWB_RSV_STATE_O_INITIATED:
		if (rsv->is_multicast) {
			uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_ESTABLISHED);
			goto unlock;
		}
		break;
	case UWB_RSV_STATE_O_MOVE_EXPANDING:
		if (rsv->is_multicast) {
			uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MOVE_COMBINING);
			go