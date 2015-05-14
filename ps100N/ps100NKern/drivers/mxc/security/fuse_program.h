 mask)) {
			*data = 4;
			break;
		}

		if (!shared_int) {
			/* Disable the other interrupts to be reported in
			 * the cause register and then force the other
			 * interrupts and see if any get posted.  If
			 * an interrupt was posted to the bus, the
			 * test failed.
			 */
			adapter->test_icr = 0;

			/* Flush any pending interrupts */
			wr32(E1000_ICR, ~0);

			wr32(E1000_IMC, ~mask);
			wr32(E1000_ICS, ~mask);
			msleep(10);

			if (adapter->test_icr & mask) {
				*data = 5;
				break;
			}
		}
	}

	/* Disable all the interrupts */
	wr32(E1000_IMC, ~0);
	msleep(10);

	/* Unhook test interrupt handler */
	free_irq(irq, netdev);

	return *data;
}

static void igb_free_desc_rings(struct igb_adapter *adapter)
{
	struct igb_ring *tx_ring = &adapter->test_tx_ring;
	struct igb_ring *rx_ring = &adapter->test_rx_ring;
	struct pci_dev *pdev = adapter->pdev;
	int i;

	if (tx_ring->desc && tx_ring->buffer_info) {
		for (i = 0; i < tx_ring->count; i++) {
			struct igb_buffer *buf = &(tx_ring->buffer_info[i]);
			if (buf->dma)
				pci_unmap_single(pdev, buf->dma, buf->length,
						 PCI_DMA_TODEVICE);
			if (buf->skb)
				dev_kfree_skb(buf->skb);
		}
	}

	if (rx_ring->desc && rx_ring->buf