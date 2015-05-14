i = 0; i < TG3_RX_RING_SIZE; i++) {
		struct tg3_rx_buffer_desc *rxd;

		rxd = &tp->rx_std[i];
		rxd->idx_len = (tp->rx_pkt_buf_sz - tp->rx_offset - 64)
			<< RXD_LEN_SHIFT;
		rxd->type_flags = (RXD_FLAG_END << RXD_FLAGS_SHIFT);
		rxd->opaque = (RXD_OPAQUE_RING_STD |
			       (i << RXD_OPAQUE_INDEX_SHIFT));
	}

	if (tp->tg3_flags & TG3_FLAG_JUMBO_RING_ENABLE) {
		for (i = 0; i < TG3_RX_JUMBO_RING_SIZE; i++) {
			struct tg3_rx_buffer_desc *rxd;

			rxd = &tp->rx_jumbo[i];
			rxd->idx_len = (RX_JUMBO_PKT_BUF_SZ - tp->rx_offset - 64)
				<< RXD_LEN_SHIFT;
			rxd->type_flags = (RXD_FLAG_END << RXD_FLAGS_SHIFT) |
				RXD_FLAG_JUMBO;
			rxd->opaque = (RXD_OPAQUE_RING_JUMBO |
			       (i << RXD_OPAQUE_INDEX_SHIFT));
		}
	}

	/* Now allocate fresh SKBs for each rx ring. */
	for (i = 0; i < tp->rx_pending; i++) {
		if (tg3_alloc_rx_skb(tp, RXD_OPAQUE_RING_STD, -1, i) < 0) {
			printk(KERN_WARNING PFX
			       "%s: Using a smaller RX standard ring, "
			       "only %d out of %d buffers were allocated "
			       "successfully.\n",
			       tp->dev->name, i, tp->rx_pending);
			if (i == 0)
				return -ENOMEM;
			tp->rx_pending = i;
			break;
		}
	}

	if (tp->tg3_flags & TG3_FLAG_JUMBO_RING_ENABLE) {
		for (i = 0; i < tp->rx_jumbo_pending; i++) {
			if (tg3_alloc_rx_skb(tp, RXD_OPAQUE_RING_JUMBO,
					     -1, i) < 0) {
				printk(KERN_WARNING PFX
				       "%s: Using a smaller RX jumbo ring, "
				       "only %d out of %d buffers were "
				       "allocated successfully.\n",
				       tp->dev->name, i, tp->rx_jumbo_pending);
				if (i == 0) {
					tg3_free_rings(tp);
					return -ENOMEM;
				}
				tp->rx_jumbo_pending = i;
				break;
			}
		}
	}
	return 0;
}

/*
 * Must not be invoked with interrupt sources disabled and
 * the hardware shutdown down.
 */
static void tg3_free_consistent(struct tg3 *tp)
{
	kfree(tp->rx_std_buffers);
	tp->rx_std_buffers = NULL;
	if (tp->rx_std) {
		pci_free_consistent(tp->pdev, TG3_RX_RING_BYTES,
				    tp->rx_std, tp->rx_std_mapping);
		tp->rx_std = NULL;
	}
	if (tp->rx_jumbo) {
		pci_free_consistent(tp->pdev, TG3_RX_JUMBO_RING_BYTES,
				    tp->rx_jumbo, tp->rx_jumbo_mapping);
		tp->rx_jumbo = NULL;
	}
	if (tp->rx_rcb) {
		pci_free_consistent(tp->pdev, TG3_RX_RCB_RING_BYTES(tp),
				    tp->rx_rcb, tp->rx_rcb_mapping);
		tp->rx_rcb = NULL;
	}
	if (tp->tx_ring) {
		pci_free_consistent(tp->pdev, TG3_TX_RING_BYTES,
			tp->tx_ring, tp->tx_desc_mapping);
		tp->tx_ring = NULL;
	}
	if (tp->hw_status) {
		pci_free_consistent(tp->pdev, TG3_HW_STATUS_SIZE,
				    tp->hw_status, tp->status_mapping);
		tp->hw_status = NULL;
	}
	if (tp->hw_stats) {
		pci_free_consistent(tp->pdev, sizeof(struct tg3_hw_stats),
				    tp->hw_stats, tp->stats_mapping);
		tp->hw_stats = NULL;
	}
}

/*
 * Must not be invoked with interrupt sources disabled and
 * the hardware shutdown down.  Can sleep.
 */
static int tg3_alloc_consistent(struct tg3 *tp)
{
	tp->rx_std_buffers = kzalloc((sizeof(struct ring_info) *
				      (TG3_RX_RING_SIZE +
				       TG3_RX_JUMBO_RING_SIZE)) +
				     (sizeof(struct tx_ring_info) *
				      TG3_TX_RING_SIZE),
				     GFP_KERNEL);
	if (!tp->rx_std_buffers)
		return -ENOMEM;

	tp->rx_jumbo_buffers = &tp->rx_std_buffers[TG3_RX_RING_SIZE];
	tp->tx_buffers = (struct tx_ring_info *)
		&tp->rx_jumbo_buffers[TG3_RX_JUMBO_RING_SIZE];

	tp->rx_std = pci_alloc_consistent(tp->pdev, TG3_RX_RING_BYTES,
					  &tp->rx_std_mapping);
	if (!tp->rx_std)
		goto err_out;

	tp->rx_jumbo = pci_alloc_consistent(tp->pdev, TG3_RX_JUMBO_RING_BYTES,
					    &tp->rx_jumbo_mapping);

	if (!tp->rx_jumbo)
		goto err_out;

	tp->rx_rcb = pci_alloc_consistent(tp->pdev, TG3_RX_RCB_RING_BYTES(tp),
					  &tp->rx_rcb_mapping);
	if (!tp->rx_rcb)
		goto err_out;

	tp->tx_ring = pci_alloc_consistent(tp->pdev, TG3_TX_RING_BYTES,
					   &tp->tx_desc_mapping);
	if (!tp->tx_ring)
		goto err_out;

	tp->hw_status = pci_alloc_consistent(tp->pdev,
					     TG3_HW_STATUS_SIZE,
					     &tp->status_mapping);
	if (!tp->hw_status)
		goto err_out;

	tp->hw_stats = pci_alloc_consistent(tp->pdev,
					    sizeof(struct tg3_hw_stats),
					    &tp->stats_mapping);
	if (!tp->hw_stats)
		goto err_out;

	memset(tp->hw_status, 0, TG3_HW_STATUS_SIZE);
	memset(tp->hw_stats, 0, sizeof(struct tg3_hw_stats));

	return 0;

err_out:
	tg3_free_consistent(tp);
	return -ENOMEM;
}

#define MAX_WAIT_CNT 1000

/* To stop a block, clear the enable bit and poll till it
 * clears.  tp->lock is held.
 */
static int tg3_stop_block(struct tg3 *tp, unsigned long ofs, u32 enable_bit, int silent)
{
	unsigned int i;
	u32 val;

	if (tp->tg3_flags2 & TG3_FLG2_5705_PLUS) {
		switch (ofs) {
		case RCVLSC_MODE:
		case DMAC_MODE:
		case MBFREE_MODE:
		case BUFMGR_MODE:
		case MEMARB_MODE:
			/* We can't enable/disable these bits of the
			 * 5705/5750, just say success.
			 */
			return 0;

		default:
			break;
		}
	}

	val = tr32(ofs);
	val &= ~enable_bit;
	tw32_f(ofs, val);

	for (i = 0; i < MAX_WAIT_CNT; i++) {
		udelay(100);
		val = tr32(ofs);
		if ((val & enable_bit) == 0)
			break;
	}

	if (i == MAX_WAIT_CNT && !silent) {
		printk(KERN_ERR PFX "tg3_stop_block timed out, "
		       "ofs=%lx enable_bit=%x\n",
		       ofs, enable_bit);
		return -ENODEV;
	}

	return 0;
}

/* tp->lock is held. */
static int tg3_abort_hw(struct tg3 *tp, int silent)
{
	int i, err;

	tg3_disable_ints(tp);

	tp->rx_mode &= ~RX_MODE_ENABLE;
	tw32_f(MAC_RX_MODE, tp->rx_mode);
	udelay(10);

	err  = tg3_stop_block(tp, RCVBDI_MODE, RCVBDI_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, RCVLPC_MODE, RCVLPC_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, RCVLSC_MODE, RCVLSC_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, RCVDBDI_MODE, RCVDBDI_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, RCVDCC_MODE, RCVDCC_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, RCVCC_MODE, RCVCC_MODE_ENABLE, silent);

	err |= tg3_stop_block(tp, SNDBDS_MODE, SNDBDS_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, SNDBDI_MODE, SNDBDI_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, SNDDATAI_MODE, SNDDATAI_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, RDMAC_MODE, RDMAC_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, SNDDATAC_MODE, SNDDATAC_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, DMAC_MODE, DMAC_MODE_ENABLE, silent);
	err |= tg3_stop_block(tp, SNDBDC_MODE, SNDBDC_MODE_ENABLE, silent);

	tp->mac_mod