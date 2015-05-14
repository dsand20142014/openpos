 \n" \
	     "       jbne 1b            \n" \
	     : "+a" (addr), "+r" (reg1) \
	     : "a" (fifo))

#define MAC_ESP_PIO_FILL(operands, reg1) \
	asm volatile ( \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       moveb " operands " \n" \
	     "       subqw #8,%1        \n" \
	     "       subqw #8,%1        \n" \
	     : "+a" (addr), "+r" (reg1) \
	     : "a" (fifo))

#define MAC_ESP_FIFO_SIZE 16

static void mac_esp_send_pio_cmd(struct esp *esp, u32 addr, u32 esp_count,
				 u32 dma_count, int write, u8 cmd)
{
	unsigned long flags;
	struct mac_esp_priv *mep = MAC_ESP_GET_PRIV(esp);
	u8 *fifo = esp->regs + ESP_FDATA * 16;

	local_irq_save(flags);

	cmd &= ~ESP_CMD_DMA;
	mep->error = 0;

	if (write) {
		scsi_esp_cmd(esp, cmd);

		if (!mac_esp_wait_for_intr(esp)) {
			if (mac_esp_wait_for_fifo(esp))
				esp_count = 0;
		} else {
			esp_count = 0;
		}
	} else {
		scsi_esp_cmd(esp, ESP_CMD_FLUSH);

		if (esp_count >= MAC_ESP_FIFO_SIZE)
			MAC_ESP_PIO_FILL("%0@+,%2@", esp_count);
		else
			MAC_ESP_PIO_LOOP("%0@+,%2@", esp_count);

		scsi_esp_cmd(esp, cmd);
	}

	while (esp_count) {
		unsigned int n;

		if (mac_esp_wait_for_intr(esp)) {
			mep->error = 1;
			break;
		}

		if (esp->sreg & ESP_STAT_SPAM) {
			printk(KERN_ERR PFX "gross error\n");
			mep->error = 1;
			break;
		}

		n =