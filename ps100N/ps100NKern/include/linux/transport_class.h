 unknown URB status %d\n", urb->status);
		if (edc_inc(&i1480u->tx_errors, EDC_MAX_ERRORS,
					EDC_ERROR_TIMEFRAME)) {
			dev_err(dev, "TX: max acceptable errors exceeded."
					"Reset device.\n");
			netif_stop_queue(net_dev);
			i1480u_tx_unlink_urbs(i1480u);
			wlp_reset_all(&i1480u->wlp);
		}
		break;
	}
	i1480u_tx_destroy(i1480u, wtx);
	if (atomic_dec_return(&i1480u->tx_inflight.count)
	    <= i1480u->tx_inflight.threshold
	    && netif_queue_stopped(net_dev)
	    && i1480u->tx_inflight.threshold != 0) {
		netif_start_queue(net_dev);
		atomic_inc(&i1480u->tx_inflight.restart_count);
	}
	return;
}


/*
 * Given a buffer that doesn't fit in a single fragment, create an
 * scatter/gather structure for delivery to the USB pipe.
 *
 * Implements functionality of i1480u_tx_create().
 *
 * @wtx:	tx descriptor
 * @skb:	skb to send
 * @gfp_mask:	gfp allocation mask
 * @returns:    Pointer to @wtx if ok, NULL on error.
 *
 * Sorry, TOO LONG a function, but breaking it up is kind of hard
 *
 * This will break the buffer in chunks smaller than
 * i1480u_MAX_FRG_SIZE (including the header) and add proper headers
 * to each:
 *
 *   1st header           \
 *   i1480 tx header      |  fragment 1
 *   fragment data        /
 *   nxt header           \  fragment 2
 *   fragment data        /
 *   ..
 *   ..
 *   last header          \  fragment 3
 *   last fragment data   /
 *
 * This does not fill the i1480 TX header, it is left up to the
 * caller to do that; you can get it from @wtx->wlp_tx_hdr.
 *
 * This function consumes the skb unless there is an error.
 */
static
int i1480u_tx_create_n(struct i1480u_tx *wtx, struct sk_buff *skb,
		       gfp_t gfp_mask)
{
	int result;
	void *pl;
	size_t pl_size;

	void *pl_itr, *buf_itr;
	size_t pl_size_left, frgs, pl_size_1st, frg_pl_size = 0;
	struct untd_hdr_1st *untd_hdr_1st;
	struct wlp_tx_hdr *wlp_tx_hdr;
	struct untd_hdr_rst *untd_hdr_rst;

	wtx->skb = NULL;
	pl = skb->data;
	pl_itr = pl;
	pl_size = skb->len;
	pl_size_left = pl_size;	/* payload size */
	/* First fragment; fits as much as i1480u_MAX_FRG_SIZE minus
	 * the headers */
	pl_size_1st = i1480u_MAX_FRG_SIZE
		- sizeof(struct untd_hdr_1st) - sizeof(struct wlp_tx_hdr);
	BUG_ON(pl_size_1st > pl_size);
	pl_size_left -= pl_size_1st;
	/* The rest have an smaller header (no i1480 TX header). We
	 * need to break up the payload in blocks smaller than
	 * i1480u_MAX_PL_SIZE (payload excluding header). */
	frgs = (pl_size_left + i1480u_MAX_PL_SIZE - 1) / i1480u_MAX_PL_SIZE;
	/* Allocate space for the new 