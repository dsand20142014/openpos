R, &q->queue_flags) &&
				!test_bit(QUEUE_FLAG_REENTER,
					&sdev->request_queue->queue_flags);
		if (flagset)
			queue_flag_set(QUEUE_FLAG_REENTER, sdev->request_queue);
		__blk_run_queue(sdev->request_queue);
		if (flagset)
			queue_flag_clear(QUEUE_FLAG_REENTER, sdev->request_queue);
		spin_unlock(sdev->request_queue->queue_lock);

		spin_lock(shost->host_lock);
	}
	/* put any unprocessed entries back */
	list_splice(&starved_list, &shost->starved_list);
	spin_unlock_irqrestore(shost->host_lock, flags);

	blk_run_queue(q);
}

/*
 * Function:	scsi_requeue_command()
 *
 * Purpose:	Handle post-processing of completed commands.
 *
 * Arguments:	q	- queue to operate on
 *		cmd	- command that may need to be requeued.
 *
 * Returns:	Nothing
