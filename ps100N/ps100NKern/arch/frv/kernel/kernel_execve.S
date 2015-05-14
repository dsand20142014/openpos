	outl(1, host->addr + TUL_SCnt0);

		outb(0, host->addr + TUL_SFifo);
		outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
		if ((wait_tulip(host)) == -1)
			return -1;
		if (host->phase != DATA_OUT) {	/* Disable wide CPU to allow read 16 bits */
			outb(TSC_HW_RESELECT, host->addr + TUL_SCtrl1);
			outb(TSC_FLUSH_FIFO, host->addr + TUL_SCtrl0);
			return 6;
		}
	}
}

int initio_status_msg(struct initio_host * host)
{				/* status & MSG_IN */
	struct scsi_ctrl_blk *scb = host->active;
	u8 msg;

	outb(TSC_CMD_COMP, host->addr + TUL_SCmd);
	if (wait_tulip(host) == -1)
		return -1;

	/* get status */
	scb->tastat = inb(host->addr + TUL_SFifo);

	if (host->phase == MSG_OUT) {
		if (host->jsstatus0 & TSS_PAR_ERROR)
			outb(MSG_PARITY, host->addr + TUL_SFifo);
		else
			outb(MSG_NOP, host->addr + TUL_SFifo);
		outb(TSC_XF_FIFO_OUT, host->addr + TUL_SCmd);
		return wait_tulip(host);
	}
	if (host->phase == MSG_IN) {
		msg = inb(host->addr + TUL_SFifo);
		if (host->jsstatus0 & TSS_PAR_ERROR) {	/* Parity error                 */
			if (