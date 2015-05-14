HANDLED;
}

static void igb_write_itr(struct igb_ring *ring)
{
	struct e1000_hw *hw = &ring->adapter->hw;
	if ((ring->adapter->itr_setting & 3) && ring->set_itr) {
		switch (hw->mac.type) {
		case e1000_82576:
			wr32(ring->itr_register, ring->itr_val |
			     0x80000000);
			break;
		default:
			wr32(ring->itr_register, ring->itr_val |
			     (ring->itr_val << 16));
			break;
		}
		ring->set_itr = 0;
	}
}

static irqreturn_t igb_msix_rx(int irq, void *data)
{
	struct igb_ring *rx_ring = data;

	/* Write the ITR value calculated at the end of the
	 * previous interrupt.
	 */

	igb_write_itr(rx_ring);

	if (napi_schedule_prep(&rx_ring->napi))
		__napi_schedule(&rx_ring->napi);

#ifdef CONFIG_IGB_DCA
	if (rx_ring->adapter->flags & IGB_FLAG_DCA_ENABLED)
		igb_update_rx_dca(rx_ring);
#endif
		return IRQ_HANDLED;
}

#ifdef CONFIG_IGB_DCA
static void igb_update_rx_dca(struct igb_ring *rx_ring)
{
	u32 dca_rxctrl;
	struct igb_adapter *adapter = rx_ring->adapter;
	struct e1000_hw *hw = &adapter->hw;
	int cpu = get_cpu();
	int q = rx_ring->reg_idx;

	if (rx_ring->cpu != cpu) {
		dca_rxctrl = rd32(E1000_DCA_RXCTRL(q));
		if (hw->mac.type == e1000_82576) {
			dca_rxctrl &= ~E1000_DCA_RXCTRL_CPUID_MASK_82576;
			dca_rxctrl |= dca3_get_tag(&adapter->pdev->dev, cpu) <<
			              E1000_DCA_RXCTRL_CPUID_SHIFT;
		} else {
			dca_rxctrl &= ~E1000_DCA_RXCTRL_CPUID_MASK;
			dca_rxctrl |= dca3_get_tag(&adapter->pdev->dev, cpu);
		}
		dca_rxctrl |= E1000_DCA_RXCTRL_DESC_DCA_EN;
		dca_rxctrl |= E1000_DCA_RXCTRL_HEAD_DCA_EN;
		dca_rxctrl |= E1000_DCA_RXCTRL_DATA_DCA_EN;
		wr32(E1000_DCA_RXCTRL(q), dca_rxctrl);
		rx_ring->cpu = cpu;
	}
	put_cpu();
}

static void igb_update_tx_dca(struct igb_ring *tx_ring)
{
	u32 dca_txctrl;
	struct igb_adapter *adapter = tx_ring->adapter;
	struct e1000_hw *hw = &adapter->hw;
	int cpu = get_cpu();
	int q = tx_ring->reg_idx;

	if (tx_ring->cpu != cpu) {
		dca_txctrl = rd32(E1000_DCA_TXCTRL(q));
		if (hw->mac.type == e1000_82576) {
			dca_txctrl &= ~E1000_DCA_TXCTRL_CPUID_MASK_82576;
			dca_txctrl |= dca3_get_tag(&adapter->pdev->dev, cpu) <<
			              E1000_DCA_TXCTRL_CPUID_SHIFT;
		} else {
			dca_txctrl &= ~E1000_DCA_TXCTRL_CPUID_MASK;
			dca_txctrl |= dca3_get_tag(&adapter->pdev->dev, cpu);
		}
		dca_txctrl |= E1000_DCA_TXCTRL_DESC_DCA_EN;
		wr32(E1000_DCA_TXCTRL(q), dca_txctrl);
		tx_ring->cpu = cpu;
	}
	put_cpu();
}

static void igb_setup_dca(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	int i;

	if (!(adapter->flags & IGB_FLAG_DCA_ENABLED))
		return;

	/* Always use CB2 mode, difference is masked in the CB driver. */
	wr32(E1000_DCA_CTRL, E1000_DCA_CTRL_DCA_MODE_CB2);

	for (i = 0; i < adapter->num_tx_queues; i++) {
		adapter->tx_ring[i].cpu = -1;
		igb_update_tx_dca(&adapter->tx_ring[i]);
	}
	for (i = 0; i < adapter->num_rx_queues; i++) {
		adapter->rx_ring[i].cpu = -1;
		igb_update_rx_dca(&adapter->rx_ring[i]);
	}
}

static int __igb_notify_dca(struct device *dev, void *data)
{
	struct net_device *netdev = dev_get_drvdata(dev);
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	unsigned long event = *(unsigned long *)data;

	switch (event) {
	case DCA_PROVIDER_ADD:
		/* if already enabled, don't do it again */
		if (adapter->flags & IGB_FLAG_DCA_ENABLED)
			break;
		/* Always use CB2 mode, difference is masked
		 * in the CB driver. */
		wr32(E1000_DCA_CTRL, E1000_DCA_CTRL_DCA_MODE_CB2);
		if (dca_add_requester(dev) == 0) {
			adapter->flags |= IGB_FLAG_DCA_ENABLED;
			dev_info(&adapter->pdev->dev, "DCA enabled\n");
			igb_setup_dca(adapter);
			break;
		}
		/* Fall Through since DCA is disabled. */
	case DCA_PROV