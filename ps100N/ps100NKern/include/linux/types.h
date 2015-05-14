queued_send_wr) {
		ret = ib_post_send(qp_info->qp, &queued_send_wr->send_wr,
				   &bad_send_wr);
		if (ret) {
			printk(KERN_ERR PFX "ib_post_send failed: %d\n", ret);
			mad_send_wr = queued_send_wr;
			wc->status = IB_WC_LOC_QP_OP_ERR;
			goto retry;
		}
	}
}

static void mark_sends_for_retry(struct ib_mad_qp_info *qp_info)
{
	struct ib_mad_send_wr_private *mad_send_wr;
	struct ib_mad_list_head *mad_list;
	unsigned long flags;

	spin_lock_irqsave(&qp_info->send_queue.lock, flags);
	list_for_each_entry(mad_list, &qp_info->send_queue.list, list) {
		mad_send_wr = container_of(mad_list,
					   struct ib_mad_send_wr_private,
					   mad_list);
		mad_send_wr->retry = 1;
	}
	spin_unlock_irqrestore(&qp_info->send_queue.lock, flags);
}

static void mad_error_handler(struct ib_mad_port_private *port_priv,
			      struct ib_wc *wc)
{
	struct ib_mad_list_head *mad_list;
	struct ib_mad_qp_info *qp_info;
	struct ib_mad_send_wr_private *mad_send_wr;
	int ret;

	/* Determine if failure was a send or receive */
	mad_list = (struct ib_mad_list_head *)(unsigned long)wc->wr_id;
	qp_info = mad_list->mad_queue->qp_info;
	if (mad_list->mad_queue == &qp_info->recv_queue)
		/*
		 * Receive errors indicate that the QP has entered the error
		 * state - error handling/shutdown code will cleanup
		 */
		return;

	/*
	 * Send errors will transition the QP to SQE - move
	 * QP to RTS and repost flushed work requests
	 */
	mad_send_wr = container_of(mad_list, struct ib_mad_send_wr_private,
				   mad_list);
	if (wc->status == IB_WC_WR_FLUSH_ERR) {
		if (mad_send_wr->retry) {
			/* Repost send */
			struct ib_send_wr *bad_send_wr;

			mad_send_wr->retry = 0;
			ret = ib_post_send(qp_info->qp, &mad_send_wr->send_wr,
					&bad_send_wr);
			if (ret)
				ib_mad_send_done_handler(port_priv, wc);
		} else
			ib_mad_send_done_handler(port_priv, wc);
	} else {
		struct ib_qp_attr *attr;

		/* Transition QP to RTS and fail offending send */
		attr = kmalloc(sizeof *attr, GFP_KERNEL);
		if (attr) {
			attr->qp_state = IB_QPS_RTS;
			attr->cur_qp_state = IB_QPS_SQE;
			ret = ib_modify_qp(qp_info->qp, attr,
					   IB_QP_STATE | IB_QP_CUR_STATE);
			kfree(attr);
			if (ret)
				printk(KERN_ERR PFX "mad_error_handler - "
				       "ib_modify_qp to RTS : %d\n", ret);
			else
				mark_sends_for_retry(qp_info);
		}
		ib_mad_send_done_handler(port_priv, wc);
	}
}

/*
 * IB MAD completion callback
 */
static void ib_mad_completion_handler(struct work_struct *work)
{
	struct ib_mad_port_private *port_priv;
	struct ib_wc wc;

	port_priv = container_of(work, struct ib_mad_port_private, work);
	ib_req_notify_cq(port_priv->cq, IB_CQ_NEXT_COMP);

	while (ib_poll_cq(port_priv->cq, 1, &wc) == 1) {
		if (wc.status == IB_WC_SUCCESS) {
			switch (wc.opcode) {
			case IB_WC_SEND:
				ib_mad_send_done_handler(port_priv, &wc);
				break;
			case IB_WC_RECV:
				ib_mad_recv_done_handler(port_priv, &wc);
				break;
			default:
				BUG_ON(1);
				break;
			}
		} else
			mad_error_handler(port_priv, &wc);
	}
}

static void cancel_mads(struct ib_mad_agent_private *mad_agent_priv)
{
	unsigned long flags;
	struct ib_mad_send_wr_private *mad_send_wr, *temp_mad_send_wr;
	struct ib_mad_send_wc mad_send_wc;
	struct list_head cancel_list;

	INIT_LIST_HEAD(&cancel_list);

	spin_lock_irqsave(&mad_agent_priv->lock, flags);
	list_for_each_entry_safe(mad_send_wr, temp_mad_send_wr,
				 &mad_agent_priv->send_list, agent_list) {
		if (mad_send_wr->status == IB_WC_SUCCESS) {
			mad_send_wr->status = IB_WC_WR_FLUSH_ERR;
			mad_send_wr->refcount -= (mad_send_wr->timeout > 0);
		}
	}

	/* Empty wait list to prevent receives from finding a request */
	list_splice_init(&mad_agent_priv->wait_list, &cancel_list);
	spin_unlock_irqrestore(&mad_agent_priv->lock, flags);

	/* Report all cancelled requests */
	mad_send_wc.status = IB_WC_WR_FLUSH_ERR;
	mad_send_wc.vendor_err = 0;

	list_for_each_entry_safe(mad_send_wr, temp_mad_send_wr,
				 &cancel_list, agent_list) {
		mad_send_wc.send_buf = &mad_send_wr->send_buf;
		list_del(&mad_send_wr->agent_list);
		mad_agent_priv->agent.send_handler(&mad_agent_priv->agent,
						   &mad_send_wc);
		atomic_dec(&mad_agent_priv->refcount);
	}
}

static struct ib_mad_send_wr_private*
find_send_wr(struct ib_mad_agent_private *mad_agent_priv,
	     struct ib_mad_send_buf *send_buf)
{
	struct ib_mad_send_wr_pri