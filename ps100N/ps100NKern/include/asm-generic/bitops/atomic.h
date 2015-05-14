= NULL)
			host->last_avail = NULL;
		scb->next = NULL;
		scb->status = SCB_RENT;
	}
	spin_unlock_irqrestore(&host->avail_lock, flags);
	return scb;
}

/**
 *	initio_release_scb		-	Release an SCB
 *	@host: InitIO host that owns the SCB
 *	@cmnd: SCB command block being returned
 *
 *	Return an allocated SCB to the host free list
 */

static void initio_release_scb(struct initio_host * host, struct scsi_ctrl_blk * cmnd)
{
	unsigned long flags;

#if DEBUG_QUEUE
	printk("Release SCB %p; ", cmnd);
#endif
	spin_lock_irqsave(&(host->avail_lock), flags);
	cmnd->srb = NULL;
	cmnd->status = 0;
	cmnd->next = NULL;
	if (host->last_avail != NULL) {
		host->last_avail->next = cmnd;
		host->last_avail = cmnd;
	} else {
		host->first_avail = cmnd;
		host->last_avail = cmnd;
	}
	spin_unlock_irqrestore(&(host->avail_lock), flags);
}

/***************************************************************************/
static void initio_append_pend_scb(struct initio_host * host, struct scsi_ctrl_blk * scbp)
{

#if DEBUG_QUEUE
	printk("Append pend SCB %p; ", scbp);
#endif
	scbp->status = SCB_PEND;
	scbp->next = NULL;
	if (host->last_pending != NULL) {
		host->last_pending->next = scbp;
		host->last_pending = scbp;
	} else {
		host->first_pending = scbp;
		host->last_pending = scbp;
	}
}

/***************************************************************************/
static void initio_push_pend_scb(struct initio_host * host, struct scsi_ctrl_blk * scbp)
{

#if DEBUG_QUEUE
	printk("Push pend SCB %p; ", scbp);
#endif
	scbp->status = SCB_PEND;
	if ((scbp->next = host->first_pending) != NULL) {
		host->first_pending = scbp;
	} else {
		host->first_pending = scbp;
		host->last_pending = scbp;
	}
}

static struct scsi_ctrl_blk *initio_find_first_pend_scb(struct initio_host * host)
{
	struct scsi_ctrl_blk *first;


	first = host->first_pending;
	while (first != NULL) {
		if (first->opcode != ExecSCSI)
			return first;
		if (first->tagmsg == 0) {
			if ((host->act_tags[first->target] == 0) &&
			    !(host->targets[first->target].flags & TCF_BUSY))
				return first;
		} else {
			if ((host->act_tags[first->target] >=
			  host->max_tags[first->target]) |
			    (host->targets[first->target].flags & TCF_BUSY)) {
				first = first->next;
				continue;
			}
			return first;
		}
		first = first->next;
	}
	return first;
}

static void initio_unlink_pend_scb(struct initio_host * host, struct scsi_ctrl_blk * scb)
{
	struct scsi_ctrl_blk *tmp, *prev;

#if DEBUG_QUEUE
	printk("unlink pend SCB %p; ", scb);
#endif

	prev = tmp = host->first_pending;
	while (tmp != NULL) {
		if (scb == tmp) {	/* Unlink this SCB              */
			if (tmp == host->first_pending) {
				if ((host->first_pending = tmp->next) == NULL)
					host->last_pending = NULL;
			} else {
				prev->next = tmp->next;
				if (tmp == host->last_pending)
					host->last_pending = prev;
			}
			tmp->next = NULL;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
}

static void initio_append_busy_scb(struct initio_host * host, struct scsi_ctrl_blk * scbp)
{

#if DEBUG_QUEUE
	printk("append busy SCB %p; ", scbp);
#endif
	if (scbp->tagmsg)
		host->act_tags[scbp->target]++;
	else
		host->targets[scbp->target].flags |= TCF_BUSY;
	scbp->status = SCB_BUSY;
	scbp->next = NULL;
	if (host->last_busy != NULL) {
		host->last_busy->next = scbp;
		host->last_busy = scbp;
	} else {
		host->first_busy = scbp;
		host->last_busy = scbp;
	}
}

/***************************************************************************/
static struct scsi_ctrl_blk *initio_pop_busy_scb(struct initio_host * host)
{
	struct scsi_ctrl_blk *tmp;


	if ((tmp = host->first_busy) != NULL) {
		if ((host->first_busy = tmp->next) == NULL)
			host->last_busy = NULL;
		tmp->next = NULL;
		if (tmp->tagmsg)
			host->act_tags[tmp->target]--;
		else
			host->targets[tmp->target].flags &= ~TCF_BUSY;
	}
#if DEBUG_QUEUE
	printk("Pop busy SCB %p; ", tmp);
#endif
	return tmp;
}

/***************************************************************************/
static void initio_unlink_busy_scb(struct initio_host * host, struct scsi_ctrl_blk * scb)
{
	struct scsi_ctrl_blk *tmp, *prev;

#if DEBUG_QUEUE
	printk("unlink busy SCB %p; ", scb);
#endif

	prev = tmp = host->first_busy;
	while (tmp != NULL) {
		if (scb == tmp) {	/* Unlink this SCB              */
			if (tmp == host->first_busy) {
				if ((host->first_busy = tmp->next) == NULL)
					host->last_busy = NULL;
			} else {
				prev->next = tmp->next;
				if (tmp == host->last_busy)
					host->last_busy = prev;
			}
			tmp->next = NULL;
			if (tmp->tagmsg)
				host->act_tags[tmp->target]--;
			else
				host->targets[tmp->target].flags &= ~TCF_BUSY;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	return;
}

struct scsi_ctrl_blk *initio_find_busy_scb(struct initio_host * host, u16 tarlun)
{
	struct scsi_ctrl_blk *tmp, *prev;
	u16 scbp_tarlun;


	prev = tmp = host->first_busy;
	while (tmp != NULL) {
		scbp_tarlun = (tmp->lun << 8) | (tmp->target);
		if (scbp_tarlun == tarlun) {	/* Unlink this SCB              */
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
#if DEBUG_QUEUE
	printk("find busy SCB %p; ", tmp);
#endif
	return tmp;
}

static void initio_append_done_scb(struct initio_host * host, struct scsi_ctrl_blk * scbp)
{
#if DEBUG_QUEUE
	printk("append done SCB %p; ", scbp);
#endif

	scbp->status = SCB_DONE;
	scbp->next = NULL;
	if (host->last_done != NULL) {
		host->last_done->next = scbp;
		host->last_done = scbp;
	} else {
		host->first_done = scbp;
		host->last_done = scbp;
	}
}

struct scsi_ctrl_blk *initio_find_done_scb(struct initio_host * host)
{
	struct scsi_ctrl_blk 