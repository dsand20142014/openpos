

	lbs_deb_enter(LBS_DEB_HOST);

	Localpsmode = cpu_to_le32(LBS802_11POWERMODECAM);

	lbs_prepare_and_send_command(priv, CMD_802_11_PS_MODE,
			      CMD_SUBCMD_EXIT_PS,
			      wait_option, 0, &Localpsmode);

	lbs_deb_leave(LBS_DEB_HOST);
}

/**
 *  @brief This function checks condition and prepares to
 *  send sleep confirm command to firmware if ok.
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *  @param psmode  	Power Saving mode
 *  @return 	   	n/a
 */
void lbs_ps_confirm_sleep(struct lbs_private *priv)
{
	unsigned long flags =0;
	int allowed = 1;

	lbs_deb_enter(LBS_DEB_HOST);

	spin_lock_irqsave(&priv->driver_lock, flags);
	if (priv->dnld_sent) {
		allowed = 0;
		lbs_deb_host("dnld_sent was set\n");
	}

	/* In-progress command? */
	if (priv->cur_cmd) {
		allowed = 0;
		lbs_deb_host("cur_cmd was set\n");
	}

	/* Pending events or command responses? */
	if (__kfifo_len(priv->event_fifo) || priv->resp_len[priv->resp_idx]) {
		allowed = 0;
		lbs_deb_host("pending events or command responses\n");
	}
	spin_unlock_irqrestore(&priv->driver_lock, flags);

	if (allowed) {
		lbs_deb_host("sending lbs_ps_confirm_sleep\n");
		lbs_send_confirmsleep(priv);
	} else {
		lbs_deb_host("sleep confirm has been delayed\n");
	}

	lbs_deb_leave(LBS_DEB_HOST);
}


/**
 * @brief Configures the transmission power control functionality.
 *
 * @param priv		A pointer to struct lbs_private structure
 * @param enable	Transmission power control enable
 * @param p0		Power level when link quality is good (dBm).
 * @param p1		Power level when link quality is fair (dBm).
 * @param p2		Power level when link quality is poor (dBm).
 * @param usesnr	Use Signal to Noise Ratio in TPC
 *
 * @return 0 on success
 */
int lbs_set_tpc_cfg(struct lbs_private *priv, int enable, int8_t p0, int8_t p1,
		int8_t p2, int usesnr)
{
	struct cmd_ds_802_11_tpc_cfg cmd;
	int ret;

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.enable = !!enable;
	cmd.usesnr = !!usesnr;
	cmd.P0 = p0;
	cmd.P1 = p1;
	cmd.P2 = p2;

	ret = lbs_cmd_with_response(priv, CMD_802_11_TPC_CFG, &cmd);

	return ret;
}

/**
 * @brief Configures the power adaptation settings.
 *
 * @param priv		A pointer to struct lbs_private structure
 * @param enable	Power adaptation enable
 * @param p0		Power level for 1, 2, 5.5 and 11 Mbps (dBm).
 * @param p1		Power level for 6, 9, 12, 18, 22, 24 and 36 Mbps (dBm).
 * @param p2		Power level for 48 and 54 Mbps (dBm).
 *
 * @return 0 on Success
 */

int lbs_set_power_adapt_cfg(struct lbs_private *priv, int enable, int8_t p0,
		int8_t p1, int8_t p2)
{
	struct cmd_ds_802_11_pa_cfg cmd;
	int ret;

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.enable = !!enable;
	cmd.P0 = p0;
	cmd.P1 = p1;
	cmd.P2 = p2;

	ret = lbs_cmd_with_response(priv, CMD_802_11_PA_CFG , &cmd);

	return ret;
}


static struct cmd_ctrl_node *__lbs_cmd_async(struct lbs_private *priv,
	uint16_t command, struct cmd_header *in_cmd, int in_cmd_size,
	int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	unsigned long callback_arg)
{
	struct cmd_ctrl_node *cmdnode;

	lbs_deb_enter(LBS_DEB_HOST);

	if (priv->surpriseremoved) {
		lbs_deb_host("PREP_CMD: card removed\n");
		cmdnode = ERR_PTR(-ENOENT);
		goto done;
	}

	cmdnode = lbs_get_cmd_ctrl_node(priv);
	if (cmdnode == NULL) {
		lbs_deb_host("PREP_CMD: cmdnode is NULL\n");

		/* Wake up main thread to execute next command */
		wake_up_interruptible(&priv->waitq);
		cmdnode = ERR_PTR(-ENOBUFS);
		goto done;
	}

	cmdnode->callback = callback;
	cmdnode->callback_arg = callback_arg;

	/* Copy the incoming command to the buffer */
	memcpy(cmdnode->cmdbuf, in_cmd, in_cmd_size);

	/* Set sequence number, clean result, move to buffer */
	priv->seqnum++;
	cmdnode->cmdbuf->command = cpu_to_le16(command);
	cmdnode->cmdbuf->size    = cpu_to_le16(in_cmd_size);
	cmdnode->cmdbuf->seqnum  = cpu_to_le16(priv->seqnum);
	cmdnode->cmdbuf->result  = 0;

	lbs_deb_host("PREP_CMD: command 0x%04x\n", command);

	cmdnode->cmdwaitqwoken = 0;
	lbs_queue_cmd(priv, cmdnode);
	wake_up_interruptible(&priv->waitq);

 done:
	lbs_deb_leave_args(LBS_DEB_HOST, "ret %p", cmdnode);
	return cmdnode;
}

void lbs_cmd_async(struct lbs_private *priv, uint16_t command,
	struct cmd_header *in_cmd, int in_cmd_size)
{
	lbs_deb_enter(LBS_DEB_CMD);
	__lbs_cmd_async(priv, command, in_cmd, in_cmd_size,
		lbs_cmd_async_callback, 0);
	lbs_deb_leave(LBS_DEB_CMD);
}

int __lbs_cmd(struct lbs_private *priv, uint16_t command,
	      struct cmd_header *in_cmd, int in_cmd_size,
	      int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	      unsigned long callback_arg)
{
	struct cmd_ctrl_node *cmdnode;
	unsigned long flags;
	int ret = 0;

	lbs_deb_enter(LBS_DEB_HOST);

	cmdnode = __lbs_cmd_async(p