pnt)[ldn] = IM_LONG_SCB;
		break;
	}
	/*issue scb command, and return */
	if (++disk_rw_in_progress == 1)
		PS2_DISK_LED_ON(shpnt->host_no, target);

	if (last_scsi_type(shpnt)[ldn] == IM_LONG_SCB) {
		issue_cmd(shpnt, isa_virt_to_bus(scb), IM_LONG_SCB | ldn);
		IBM_DS(shpnt).long_scbs++;
	} else {
		issue_cmd(shpnt, isa_virt_to_bus(scb), IM_SCB | ldn);
		IBM_DS(shpnt).scbs++;
	}
	return 0;
}

static int __ibmmca_abort(Scsi_Cmnd * cmd)
{
	/* Abort does not work, as the adapter never generates an interrupt on
	 * whatever situation is simulated, even when really pending commands
	 * are running on the adapters' hardware ! */

	struct Scsi_Host *shpnt;
	unsigned int ldn;
	void (*saved_done) (Scsi_Cmnd *);
	int target;
	int max_pun;
	unsigned long imm_command;

#ifdef IM_DEBUG_PROBE
	printk("IBM MCA SCSI: Abort sub