/*******************************************************************
 * This file is part of the Emulex Linux Device Driver for         *
 * Fibre Channel Host Bus Adapters.                                *
 * Copyright (C) 2004-2009 Emulex.  All rights reserved.           *
 * EMULEX and SLI are trademarks of Emulex.                        *
 * www.emulex.com                                                  *
 * Portions Copyright (C) 2004-2005 Christoph Hellwig              *
 *                                                                 *
 * This program is free software; you can redistribute it and/or   *
 * modify it under the terms of version 2 of the GNU General       *
 * Public License as published by the Free Software Foundation.    *
 * This program is distributed in the hope that it will be useful. *
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND          *
 * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,  *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE      *
 * DISCLAIMED, EXCEPT TO THE EXTENT THAT SUCH DISCLAIMERS ARE HELD *
 * TO BE LEGALLY INVALID.  See the GNU General Public License for  *
 * more details, a copy of which can be found in the file COPYING  *
 * included with this package.                                     *
 *******************************************************************/

#include <linux/blkdev.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport_fc.h>
#include <scsi/fc/fc_fs.h>

#include "lpfc_hw4.h"
#include "lpfc_hw.h"
#include "lpfc_sli.h"
#include "lpfc_sli4.h"
#include "lpfc_nl.h"
#include "lpfc_disc.h"
#include "lpfc_scsi.h"
#include "lpfc.h"
#include "lpfc_crtn.h"
#include "lpfc_logmsg.h"
#include "lpfc_compat.h"
#include "lpfc_debugfs.h"
#include "lpfc_vport.h"

/* There are only four IOCB completion types. */
typedef enum _lpfc_iocb_type {
	LPFC_UNKNOWN_IOCB,
	LPFC_UNSOL_IOCB,
	LPFC_SOL_IOCB,
	LPFC_ABORT_IOCB
} lpfc_iocb_type;


/* Provide function prototypes local to this module. */
static int lpfc_sli_issue_mbox_s4(struct lpfc_hba *, LPFC_MBOXQ_t *,
				  uint32_t);
static int lpfc_sli4_read_rev(struct lpfc_hba *, LPFC_MBOXQ_t *,
			    uint8_t *, uint32_t *);

static IOCB_t *
lpfc_get_iocb_from_iocbq(struct lpfc_iocbq *iocbq)
{
	return &iocbq->iocb;
}

/**
 * lpfc_sli4_wq_put - Put a Work Queue Entry on an Work Queue
 * @q: The Work Queue to operate on.
 * @wqe: The work Queue Entry to put on the Work queue.
 *
 * This routine will copy the contents of @wqe to the next available entry on
 * the @q. This function will then ring the Work Queue Doorbell to signal the
 * HBA to start processing the Work Queue Entry. This function returns 0 if
 * successful. If no entries are available on @q then this function will return
 * -ENOMEM.
 * The caller is expected to hold the hbalock when calling this routine.
 **/
static uint32_t
lpfc_sli4_wq_put(struct lpfc_queue *q, union lpfc_wqe *wqe)
{
	union lpfc_wqe *temp_wqe = q->qe[q->host_index].wqe;
	struct lpfc_register doorbell;
	uint32_t host_index;

	/* If the host has not yet processed the next entry then we are done */
	if (((q->host_index + 1) % q->entry_count) == q->hba_index)
		return -ENOMEM;
	/* set consumption flag every once in a while */
	if (!((q->host_index + 1) % LPFC_RELEASE_NOTIFICATION_INTERVAL))
		bf_set(lpfc_wqe_gen_wqec, &wqe->generic, 1);

	lpfc_sli_pcimem_bcopy(wqe, temp_wqe, q->entry_size);

	/* Update the host index before invoking device */
	host_index = q->host_index;
	q->host_index = ((q->host_index + 1) % q->entry_count);

	/* Ring Doorbell */
	doorbell.word0 = 0;
	bf_set(lpfc_wq_doorbell_num_posted, &doorbell, 1);
	bf_set(lpfc_wq_doorbell_index, &doorbell, host_index);
	bf_set(lpfc_wq_doorbell_id, &doorbell, q->queue_id);
	writel(doorbell.word0, q->phba->sli4_hba.WQDBregaddr);
	readl(q->phba->sli4_hba.WQDBregaddr); /* Flush */

	return 0;
}

/**
 * lpfc_sli4_wq_release - Updates internal hba index for WQ
 * @q: The Work Queue to operate on.
 * @index: The index to advance the hba index to.
 *
 * This routine will update the HBA index of a queue to reflect consumption of
 * Work Queue Entries by the HBA. When the HBA indicates that it has consumed
 * an entry the host calls this function to update the queue's internal
 * pointers. This routine returns the number of entries that were consumed by
 * the HBA.
 **/
static uint32_t
lpfc_sli4_wq_release(struct lpfc_queue *q, uint32_t index)
{
	uint32_t released = 0;

	if (q->hba_index == index)
		return 0;
	do {
		q->hba_index = ((q->hba_index + 1) % q->entry_count);
		released++;
	} while (q->hba_index != index);
	return released;
}

/**
 * lpfc_sli4_mq_put - Put a Mailbox Queue Entry on an Mailbox Queue
 * @q: The Mailbox Queue to operate on.
 * @wqe: The Mailbox Queue Entry to put on the Work queue.
 *
 * This routine will copy the contents of @mqe to the next available entry on
 * the @q. This function will then ring the Work Queue Doorbell to signal the
 * HBA to start processing the Work Queue Entry. This function returns 0 if
 * successful. If no entries are available on @q then this function will return
 * -ENOMEM.
 * The caller is expected to hold the hbalock when calling this routine.
 **/
static uint32_t
lpfc_sli4_mq_put(struct lpfc_queue *q, struct lpfc_mqe *mqe)
{
	struct lpfc_mqe *temp_mqe = q->qe[q->host_index].mqe;
	struct lpfc_register doorbell;
	uint32_t host_index;

	/* If the host has not yet processed the next entry then we are done */
	if (((q->host_index + 1) % q->entry_count) == q->hba_index)
		return -ENOMEM;
	lpfc_sli_pcimem_bcopy(mqe, temp_mqe, q->entry_size);
	/* Save off the mailbox pointer for completion */
	q->phba->mbox = (MAILBOX_t *)temp_mqe;

	/* Update the host index before invoking device */
	host_index = q->host_index;
	q->host_index = ((q->host_index + 1) % q->entry_count);

	/* Ring Doorbell */
	doorbell.word0 = 0;
	bf_set(lpfc_mq_doorbell_num_posted, &doorbell, 1);
	bf_set(lpfc_mq_doorbell_id, &doorbell, q->queue_id);
	writel(doorbell.word0, q->phba->sli4_hba.MQDBregaddr);
	readl(q->phba->sli4_hba.MQDBregaddr); /* Flush */
	return 0;
}

/**
 * lpfc_sli4_mq_release - Updates internal hba index for MQ
 * @q: The Mailbox Queue to operate on.
 *
 * This routine will update the HBA index of a queue to reflect consumption of
 * a Mailbox Queue Entry by the HBA. When the HBA indicates that it has consumed
 * an entry the host calls this function to update the queue's internal
 * pointers. This routine returns the number of entries that were consumed by
 * the HBA.
 **/
static uint32_t
lpfc_sli4_mq_release(struct lpfc_queue *q)
{
	/* Clear the mailbox pointer for completion */
	q->phba->mbox = NULL;
	q->hba_index = ((q->hba_index + 1) % q->entry_count);
	return 1;
}

/**
 * lpfc_sli4_eq_get - Gets the next valid EQE from a EQ
 * @q: The Event Queue to get the first valid EQE from
 *
 * This routine will get the first valid Event Queue Entry from @q, update
 * the queue's internal hba index, and return the EQE. If no valid EQEs are in
 * the Queue (no more work to do), or the Queue is full of EQEs that have been
 * processed, but not popped back to the HBA then this routine will return NULL.
 **/
static struct lpfc_eqe *
lpfc_sli4_eq_get(struct lpfc_queue *q)
{
	struct lpfc_eqe *eqe = q->qe[q->hba_index].eqe;

	/* If the next EQE is not valid then we are done */
	if (!bf_get(lpfc_eqe_valid, eqe))
		return NULL;
	/* If the host has not yet processed the next entry then we are done */
	if (((q->hba_index + 1) % q->entry_count) == q->host_index)
		return NULL;

	q->hba_index = ((q->hba_index + 1) % q->entry_count);
	return eqe;
}

/**
 * lpfc_sli4_eq_release - Indicates the host has finished processing an EQ
 * @q: The Event Queue that the host has completed processing for.
 * @arm: Indicates whether the host wants to arms this CQ.
 *
 * This routine will mark all Event Queue Entries on @q, from the last
 * known comp