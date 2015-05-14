cmnd *cmd, unsigned int result)
{
	struct scsi_device *dev = cmd->device;
	int tgt = dev->id;
	int lun = dev->lun;

	esp->active_cmd = NULL;
	esp_unmap_dma(esp, cmd);
	esp_free_lun_tag(ent, dev->hostdata);
	cmd->result = result;

	if (ent->eh_done) {
		complete(ent->eh_done);
		ent->eh_done = NULL;
	}

	if (ent->flags & ESP_CMD_FLAG_AUTOSENSE) {
		esp->ops->unmap_single(esp, ent->sense_dma,
				       SCSI_SENSE_BUFFERSIZE, DMA_FROM_DEVICE);
		ent->sense_ptr = NULL;

		/* Restore the message/status bytes to what we actually
		 * saw originally.  Also, report that we are providing
		 * the sense data.
		 */
		cmd->result = ((DRIVER_SENSE << 24) |
			       (DID_OK << 16) |
			       (COMMAND_COMPLETE << 8) |
			       (SAM_STAT_CHECK_CONDITION << 0));

		ent->flags &= ~ESP_CMD_FLAG_AUTOSENSE;
		if (esp_debug & ESP_DEBUG_AUTOSENSE) {
			int i;

			printk("esp%d: tgt[%d] lun[%d] AUTO SENSE[ ",
			       esp->host->unique_id, tgt, lun);
			for (i = 0; i < 18; i++)
				printk("%02x ", cmd->sense_buffer[i]);
			printk("]\n");
		}
	}

	cmd->s