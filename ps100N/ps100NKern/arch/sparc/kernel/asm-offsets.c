drp *drp_ie)
{
	struct uwb_rsv *rsv;
	struct uwb_pal *pal;
	enum uwb_rsv_state state;

	rsv = uwb_rsv_alloc(rc);
	if (!rsv)
		return NULL;

	rsv->rc          = rc;
	rsv->owner       = src;
	uwb_dev_get(rsv->owner);
	rsv->target.type = UWB_RSV_TARGET_DEV;
	rsv->target.dev  = &rc->uwb_dev;
	uwb_dev_get(&rc->uwb_dev);
	rsv->type        = uwb_ie_drp_type(drp_ie);
	rsv->stream      = uwb_ie_drp_stream_index(drp_ie);
	uwb_drp_ie_to_bm(&rsv->mas, drp_ie);

	/*
	 * See if any PALs are interested in this reservation. If not,
	 * deny the request.
	 */
	rsv->state = UWB_RSV_STATE_T_DENIED;
	mutex_lock(&rc->uwb_dev.mutex);
	list_for_each_entry(pal, &rc->pals, node) {
		if (pal->new_rsv)
			pal->new_rsv(pal, rsv);
		if (rsv->state == UWB_RSV_STATE_T_ACCEPTED)
			break;
	}
	mutex_unlock(&rc->uwb_dev.mutex);

	list_add_tail(&rsv->rc_node, &rc->reservations);
	state = rsv->state;
	rsv->s