uct timer_list timer;
	struct list_head list_node;
};

static void uwb_rc_neh_timer(unsigned long arg);

static void uwb_rc_neh_release(struct kref *kref)
{
	struct uwb_rc_neh *neh = container_of(kref, struct uwb_rc_neh, kref);

	kfree(neh);
}

static void uwb_rc_neh_get(struct uwb_rc_neh *neh)
{
	kref_get(&neh->kref);
}

/**
 * uwb_rc_neh_put - release reference to a neh
 * @neh: the neh
 */
void uwb_rc_neh_put(struct uwb_rc_neh *neh)
{
	kref_put(&neh->kref, uwb_rc_neh_release);
}


/**
 * Assigns @neh a context id from @rc's pool
 *
 * @rc:	    UWB Radio Controller descriptor; @rc->neh_lock taken
 * @neh:    Notification/Event Handle
 * @returns 0 if context id was assigned ok; < 0 errno on error (if
 *	    all the context IDs are taken).
 *
 * (assumes @wa is locked).
 *
 * NOTE: WUSB spec reserves context ids 0x00 for notifications and
 *	 0xff is invalid, so they must not be used. Initialization
 *	 fills up those two in the bitmap so they are not allocated.
 *
 * We spread the allocation around to reduce the posiblity of two
 * consecutive opened @neh's getting the same context ID assigned (to
 * avoid surprises with late events that timed out long time ago). So
 * first we search from where @rc->ctx_roll is, if not found, we
 * search from zero.
 */
static
int __uwb_rc_ctx_get(struct uwb_rc *rc, struct uwb_rc_neh *neh)
{
	int result;
	result = find_next_zero_bit(rc->ctx_bm, UWB_RC_CTX_MAX,
				    rc->ctx_roll++);
	if (result < UWB_RC_CTX_MAX)
		goto found;
	result = find_first_zero_bit(rc->ctx_bm, UWB_RC_CTX_MAX);
	if (result < UWB_RC_CTX_MAX)
		goto found;
	return -ENFILE;
found:
	set_bit(result, rc->ctx_bm);
	neh->context = result;
	return 0;
}


/** Releases @neh's context ID back to @rc (@rc->neh_lock is locked). */
static
void __uwb_rc_ctx_put(struct uwb_rc *rc, struct uwb_rc_neh *neh)
{
	struct device *dev = &rc->uwb_dev.dev;
	if (neh->context == 0)
		return;
	if (test_bit(neh->context, rc->ctx_bm) == 0) {
		dev_err(dev, "context %u not set in bitmap\n",
			neh->context);
		WARN_ON(1);
	}
	clear_bit(neh->context, rc->ctx_bm);
	neh->context = 0;
}

/**
 * uwb_rc_neh_add - add a neh for a radio controller command
 * @rc:             the radio controller
 * @cmd:            the radio controller command
 * @expected_type:  the type of the expected response event
 * @expected_event: the expected event ID
 * @cb:             callback for when the event is received
 * @arg:            argument for the callback
 *
 * Creates a neh and adds it to the list of those waiting for an
 * event.  A context ID will be assigned to the command.
 */
struct uwb_rc_neh *uwb_rc_neh_add(struct uwb_rc *rc, struct uwb_rccb *cmd,
				  u8 expected_type, u16 expected_event,
				  uwb_rc_cm