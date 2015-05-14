_schedule_reset(esp);
	return 0;
}

static int esp_finish_select(struct esp *esp)
{
	struct esp_cmd_entry *ent;
	struct scsi_cmnd *cmd;
	u8 orig_select_state;

	orig_select_state = esp->select_state;

	/* No longer selecting.  */
	esp->select_state = ESP_SELECT_NONE;

	esp->seqreg = esp_read8(ESP_SSTEP) & ESP_STEP_VBITS;
	ent = esp->active_cmd;
	cmd = ent->cmd;

	if (esp->ops->dma_error(esp)) {
		/* If we see a DMA error during or as a result of selection,
		 * all bets are off.
		 */
		esp_schedule_reset(esp);
		esp_cmd_is_done(esp, ent, cmd, (DID_ERROR << 16));
		return 0;
	}

	esp->ops->dma_invalidate(esp);

	if (esp->ireg == (ESP_INTR_RSEL | ESP_INTR_FDONE)) {
		struct esp_target_data *tp = &esp->target[cmd->device->id];

		/* Carefully back out of the selection attempt.  Release
		 * resources (such as DMA mapping & TAG) and reset state (such
		 * as message out and command delivery variables).
		 */
		if (!(ent->flags & ESP_CMD_FLAG_AUTOSENSE)) {
			esp_unmap_dma(esp, cmd);
			esp_free_lun_tag(ent, cmd->device->hostdata);
			tp->flags &= ~(ESP_TGT_NEGO_SYNC | ESP_TGT_NEGO_WIDE);
			esp->flags &= ~ESP_FLAG_DOING_SLOWCMD;
			esp->cmd_bytes_ptr = NULL;
			esp->cmd_bytes_left = 0;
		} else {
			esp->ops->unmap_single(esp, ent->sense_dma,
					       SCSI_SENSE_BUFFERSIZE,
					       DMA_FROM_DEVICE);
			ent->sense_ptr = NULL;
		}

		/* Now that t