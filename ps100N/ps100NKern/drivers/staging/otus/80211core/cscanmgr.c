                                          E1000_ADVTXD_DTYP_DATA |
		                                          E1000_ADVTXD_DCMD_DEXT);
	}

	/* Setup Rx descriptor ring and Rx buffers */

	if (!rx_ring->count)
		rx_ring->count = IGB_DEFAULT_RXD;

	rx_ring->buffer_info = kcalloc(rx_ring->count,
				       sizeof(struct igb_buffer),
				       GFP_KERNEL);
	if (!rx_ring->buffer_info) {
		ret_val = 4;
		goto err_nomem;
	}

	rx_ring->size = rx_ring->count * sizeof(union e1000_adv_rx_desc);
	rx_ring->desc = pci_alloc_consistent(pdev, rx_ring->size,
					     &rx_ring->dma);
	if (!rx_ring->desc) {
		ret_val = 5;
		goto err_nomem;
	}
	rx_ring->next_to_use = rx_ring->next_to_clean = 0;

	rctl = rd32(E1000_RCTL);
	wr32(E1000_RCTL, rctl & ~E1000_RCTL_EN);
	wr32(E1000_RDBAL(0),
			((u64) rx_ring->dma & 0xFFFFFFFF));
	wr32(E1000_RDBAH(0),
			((u64) rx_ring->dma >> 32));
	wr32(E1000_RDLEN(0), rx_ring->size);
	wr32(E1000_RDH(0), 0);
	wr32(E1000_RDT(0), 0);
	rctl &= ~(E1000_RCTL_LBM_TCVR | E1000_RCTL_LBM_MAC);
	rctl = E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_RDMTS_HALF |
		(adapter->hw.mac.mc_filter_type << E1000_RCTL_MO_SHIFT);
	wr32(E1000_RCTL, rctl);
	wr32(E1000_SRRCTL(0), E1000_SRRCTL_DESCTYPE_ADV_ONEBUF);

	for (i = 0; i < rx_ring->count; i++) {
		union e1000_adv_rx_desc *rx_desc;
		struct sk_buff *skb;

		buffer_info = &rx_ring->buffer_info[i];
		rx_desc = E1000_RX_DESC_ADV(*rx_ring, i);
		skb = alloc_skb(IGB_RXBUFFER_2048 + NET_IP_ALIGN,
				GFP_KERNEL);
		if (!skb) {
			ret_val = 6;
			goto err_nomem;
		}
		skb_reserve(skb, NET_IP_ALIGN);
		buffer_info->skb = skb;
		buffer_info->dma = pci_map_single(pdev, skb->data,
		                                  IGB_RXBUFFER_2048,
		                                  PCI_DMA_FROMDEVICE);
		rx_desc->read.pkt_addr = cpu_to_le64(buffer_info->dma);
		memset(skb->data, 0x00, skb->len);
	}

	return 0;

err_nomem:
	igb_free_desc_rings(adapter);
	return ret_val;
}

static void igb_phy_disable_receiver(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;

	/* Write out to PHY registers 29 and 30 to disable the Receiver. */
	igb_write_phy_reg(hw, 29, 0x001F);
	igb_write_phy_reg(hw, 30, 0x8FFC);
	igb_write_phy_reg(hw, 29, 0x001A);
	igb_write_phy_reg(hw, 30, 0x8FF0);
}

static int igb_integrated_phy_loopback(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 ctrl_reg = 0;

	hw->mac.autoneg = false;

	if (hw->phy.type == e1000_phy_m88) {
		/* Auto-MDI/MDIX Off */
		igb_write_phy_reg(hw, M88E1000_PHY_SPEC_CTRL, 0x0808);
		/* reset to update Auto-MDI/MDIX */
		igb_write_phy_reg(hw, PHY_CONTROL, 0x9140);
		/* autoneg off */
		igb_write_phy_reg(hw, PHY_CONTROL, 0x8140);
	}

	ctrl_reg = rd32(E1000_CTRL);

	/* force 1000, set loopback */
	igb_write_phy_reg(hw, PHY_CONTROL, 0x4140);

	/* Now set up the MAC to the same speed/duplex as the PHY. */
	ctrl_reg = rd32(E1000_CTRL);
	ctrl_reg &= ~E1000_CTRL_SPD_SEL; /* Clear the speed sel bits */
	ctrl_reg |= (E1000_CTRL_FRCSPD | /* Set the Force Speed Bit */
		     E1000_CTRL_FRCDPX | /* Set the Force Duplex Bit */
		     E1000_CTRL_SPD_1000 |/* Force Speed to 1000 */
		     E1000_CTRL_FD |	 /* Force Duplex to FULL */
		     E1000_CTRL_SLU);	 /* Set link up enable bit */

	if (hw->phy.type == e1000_phy_m88)
		ctrl_reg |= E1000_CTRL_ILOS; /* Invert Loss of Signal */

	wr32(E1000_CTRL, ctrl_reg);

	/* Disable the receiver on the PHY so when a cable is plugged in, the
	 * PHY does not begin to autoneg when a cable is reconnected to the NIC.
	 */
	if (hw->phy.type == e1000_phy_m88)
		igb_phy_disable_receiver(adapter);

	udelay(500);

	return 0;
}

static int igb_set_phy_loopback(struct igb_adapter *adapter)
{
	return igb_integrated_phy_loopback(adapter);
}

static int igb_setup_loopback_test(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 reg;

	if (hw->phy.media_type == e1000_media_type_fiber ||
	    hw->phy.media_type == e1000_media_type_internal_serdes) {
		reg = rd32(E1000_RCTL);
		reg |= E1000_RCTL_LBM_TCVR;
		wr32(E1000_RCTL, reg);

		wr32(E1000_SCTL, E1000_ENABLE_SERDES_LOOPBACK);

		reg = rd32(E1000_CTRL);
		reg &= ~(E1000_CTRL_RFCE |
			 E1000_CTRL_TFCE |
			 E1000_CTRL_LRST);
		reg |= E1000_CTRL_SLU |
		       E1000_CTRL_FD;
		wr32(E1000_CTRL, reg);

		/* Unset switch control to serdes energy detect */
		reg = rd32(E1000_CONNSW);
		reg &= ~E1000_CONNSW_ENRGSRC;
		wr32(E1000_CONNSW, reg);

		/* Set PCS register for forced speed */
		reg = rd32(E1000_PCS_LCTL);
		reg &= ~E1000_PCS_LCTL_AN_ENABLE;     /* Disable Autoneg*/
		reg |= E1000_PCS_LCTL_FLV_LINK_UP |   /* Force link up */
		       E1000_PCS_LCTL_FSV_1000 |      /* Force 1000    */
		       E1000_PCS_LCTL_FDV_FULL |      /* SerDes Full duplex */
		       E1000_PCS_LCTL_FSD |           /* Force Speed */
		       E1000_PCS_LCTL_FORCE_LINK;     /* Force Link */
		wr32(E1000_PCS_LCTL, reg);

		return 0;
	} else if (hw->phy.media_type == e1000_media_type_copper) {
		return igb_set_phy_loopback(adapter);
	}

	return 7;
}

static void igb_loopback_cleanup(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 rctl;
	u16 phy_reg;

	rctl = rd32(E1000_RCTL);
	rctl &= ~(E1000_RCTL_LBM_TCVR | E1000_RCTL_LBM_MAC);
	wr32(E1000_RCTL, rctl);

	hw->mac.autoneg = true;
	igb_read_phy_reg(hw, PHY_CONTROL, &phy_reg);
	if (phy_reg & MII_CR_LOOPBACK) {
		phy_reg &= ~MII_CR_LOOPBACK;
		igb_write_phy_reg(hw, PHY_CONTROL, phy_reg);
		igb_phy_sw_reset(hw);
	}
}

static void igb_create_lbtest_frame(struct sk_buff *skb,
				    unsigned int frame_size)
{
	memset(skb->data, 0xFF, frame_size);
	frame_size &= ~1;
	memset(&skb->data[frame_size / 2], 0xAA, frame_size / 2 - 1);
	memset(&skb->data[frame_size / 2 + 10], 0xBE, 1);
	memset(&skb->data[frame_size / 2 + 12], 0xAF, 1);
}

static int igb_check_lbtest_frame(struct sk_buff *skb, unsigned int frame_size)
{
	frame_size &= ~1;
	if (*(skb->data + 3) == 0xFF)
		if ((*(skb->data + frame_size / 2 + 10) == 0xBE) &&
		   (*(skb->data + frame_size / 2 + 12) == 0xAF))
			return 0;
	return 13;
}

static int igb_run_loopback_test(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct igb_ring *tx_ring = &adapter->test_tx_ring;
	struct igb_ring *rx_ring = &adapter->test_rx_ring;
	struct pci_dev *pdev = adapter->pdev;
	int i, j, k, l, lc, good_cnt;
	int ret_val = 0;
	unsigned long time;

	wr32(E1000_RDT(0), rx_ring->count - 1);

	/* Calculate the loop count based on the largest descriptor ring
	 * The idea is to wrap the largest ring a number of times using 64
	 * send/receive pairs during each loop
	 */

	if (rx_ring->count <= tx_ring->count)
		lc = ((tx_ring->count / 64) * 2) + 1;
	else
		lc = ((rx_ring->count / 64) * 2) + 1;

	k = l = 0;
	for (j = 0; j <= lc; j++) { /* loop count loop */
		for (i = 0; i < 64; i++) { /* send the packets */
			igb_create_lbtest_frame(tx_ring->buffer_info[k].skb,
						1024);
			pci_dma_sync_single_for_device(pdev,
				tx_ring->buffer_info[k].dma,
				tx_ring->buffer_info[k].length,
				PCI_DMA_TODEVICE);
			k++;
			if (k == tx_ring->count)
				k = 0;
		}
		wr32(E1000_TDT(0), k);
		msleep(200);
		time = jiffies; /* set the start time for the receive */
		good_cnt = 0;
		do { /* receive the sent packets */
			pci_dma_sync_single_for_cpu(pdev,
					rx_ring->buffer_info[l].dma,
					IGB_RXBUFFER_2048,
					PCI_DMA_FROMDEVICE);

			ret_val = igb_check_lbtest_frame(
					     rx_ring->buffer_info[l].skb, 1024);
			if (!ret_val)
				good_cnt++;
			l++;
			if (l == rx_ring->count)
				l = 0;
			/* time + 20 msecs (200 msecs on 2.4) is more than
			 * enough time to complete the receives, if it's
			 * exceeded, break and error off
			 */
		} while (good_cnt < 64 && jiffies < (time + 20));
		if (good_cnt != 64) {
			ret_val = 13; /* ret_val is the same as mis-compare */
			break;
		}
		if (jiffies >= (time + 20)) {
			ret_val = 14; /* error code for time out error */
			break;
		}
	} /* end loop count loop */
	return ret_val;
}

static int igb_loopback_test(struct igb_adapter *adapter, u64 *data)
{
	/* PHY loopback cannot be performed if SoL/IDER
	 * sessions are active */
	if (igb_check_reset_block(&adapter->hw)) {
		dev_err(&adapter->pdev->dev,
			"Cannot do PHY loopback test "
			"when SoL/IDER is active.\n");
		*data = 0;
		goto out;
	}
	*data = igb_setup_desc_rings(adapter);
	if (*data)
		goto out;
	*data = igb_setup_loopback_test(adapter);
	if (*data)
		goto err_loopback;
	*data = igb_run_loopback_test(adapter);
	igb_loopback_cleanup(adapter);

err_loopback:
	igb_free_desc_rings(adapter);
out:
	return *data;
}

static int igb_link_test(struct igb_adapter *adapter, u64 *data)
{
	struct e1000_hw *hw = &adapter->hw;
	*data = 0;
	if (hw->phy.media_type == e1000_media_type_internal_serdes) {
		int i = 0;
		hw->mac.serdes_has_link = false;

		/* On some blade server designs, link establishment
		 * could take as long as 2-3 minutes */
		do {
			hw->mac.ops.check_for_link(&adapter->hw);
			if (hw->mac.serdes_has_link)
				return *data;
			msleep(20);
		} while (i++ < 3750);

		*data = 1;
	} else {
		hw->mac.ops.check_for_link(&adapter->hw);
		if (hw->mac.autoneg)
			msleep(4000);

		if (!(rd32(E1000_STATUS) &
		      E1000_STATUS_LU))
			*data = 1;
	}
	return *data;
}

static void igb_diag_test(struct net_device *netdev,
			  struct ethtool_test *eth_test, u64 *data)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	u16 autoneg_advertised;
	u8 forced_speed_duplex, autoneg;
	bool if_running = netif_running(netdev);

	set_bit(__IGB_TESTING, &adapter->state);
	if (eth_test->flags == ETH_TEST_FL_OFFLINE) {
		/* Offline tests */

		/* save speed, duplex, autoneg settings */
		autoneg_advertised = adapter->hw.phy.autoneg_advertised;
		forced_speed_duplex = adapter->hw.mac.forced_speed_duplex;
		autoneg = adapter->hw.mac.autoneg;

		dev_info(&adapter->pdev->dev, "offline testing starting\n");

		/* Link test performed before hardware reset so autoneg doesn't
		 * interfere with test result */
		if (igb_link_test(adapter, &data[4]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		if (if_running)
			/* indicate we're in test mode */
			dev_close(netdev);
		else
			igb_reset(adapter);

		if (igb_reg_test(adapter, &data[0]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		igb_reset(adapter);
		if (igb_eeprom_test(adapter, &data[1]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		igb_reset(adapter);
		if (igb_intr_test(adapter, &data[2]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		igb_reset(adapter);
		if (igb_loopback_test(adapter, &data[3]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		/* restore speed, duplex, autoneg settings */
		adapter->hw.phy.autoneg_advertised = autoneg_advertised;
		adapter->hw.mac.forced_speed_duplex = forced_speed_duplex;
		adapter->hw.mac.autoneg = autoneg;

		/* force this routine to wait until autoneg complete/timeout */
		adapter->hw.phy.autoneg_wait_to_complete = true;
		igb_reset(adapter);
		adapter->hw.phy.autoneg_wait_to_complete = false;

		clear_bit(__IGB_TESTING, &adapter->state);
		if (if_running)
			dev_open(netdev);
	} else {
		dev_info(&adapter->pdev->dev, "online testing starting\n");
		/* Online tests */
		if (igb_link_test(adapter, &data[4]))
			eth_test->flags |= ETH_TEST_FL_FAILED;

		/* Online tests aren't run; pass by default */
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;

		clear_bit(__IGB_TESTING, &adapter->state);
	}
	msleep_interruptible(4 * 1000);
}

static int igb_wol_exclusion(struct igb_adapter *adapter,
			     struct ethtool_wolinfo *wol)
{
	struct e1000_hw *hw = &adapter->hw;
	int retval = 1; /* fail by default */

	switch (hw->device_id) {
	case E1000_DEV_ID_82575GB_QUAD_COPPER:
		/* WoL not supported */
		wol->supported = 0;
		break;
	case E1000_DEV_ID_82575EB_FIBER_SERDES:
	case E1000_DEV_ID_82576_FIBER:
	case E1000_DEV_ID_82576_SERDES:
		/* Wake events not supported on port B */
		if (rd32(E1000_STATUS) & E1000_STATUS_FUNC_1) {
			wol->supported = 0;
			break;
		}
		/* return success for non excluded adapter ports */
		retval = 0;
		break;
	case E1000_DEV_ID_82576_QUAD_COPPER:
		/* quad port adapters only support WoL on port A */
		if (!(adapter->flags & IGB_FLAG_QUAD_PORT_A)) {
			wol->supported = 0;
			break;
		}
		/* return success for non excluded adapter ports */
		retval = 0;
		break;
	default:
		/* dual port cards only support WoL on port A from now on
		 * unless it was enabled in the eeprom for port B
		 * so exclude FUNC_1 ports from having WoL enabled */
		if (rd32(E1000_STATUS) & E1000_STATUS_FUNC_1 &&
		    !adapter->eeprom_wol) {
			wol->supported = 0;
			break;
		}

		retval = 0;
	}

	return retval;
}

static void igb_get_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
	struct igb_adapter *adapter = netdev_priv(netdev);

	wol->supported = WAKE_UCAST | WAKE_MCAST |
			 WAKE_BCAST | WAKE_MAGIC;
	wol->wolopts = 0;

	/* this function will set ->supported = 0 and return 1 if wol is not
	 * supported by this hardware */
	if (igb_wol_exclusion(adapter, wol) ||
	    !device_can_wakeup(&adapter->pdev->dev))
		return;

	/* apply any specific unsupported masks here */
	switch (adapter->hw.device_id) {
	default:
		break;
	}

	if (adapter->wol & E1000_WUFC_EX)
		wol->wolopts |= WAKE_UCAST;
	if (adapter->wol & E1000_WUFC_MC)
		wol->wolopts |= WAKE_MCAST;
	if (adapter->wol & E1000_WUFC_BC)
		wol->wolopts |= WAKE_BCAST;
	if (adapter->wol & E1000_WUFC_MAG)
		wol->wolopts |= WAKE_MAGIC;

	return;
}

static int igb_set_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;

	if (wol->wolopts & (WAKE_PHY | WAKE_ARP | WAKE_MAGICSECURE))
		return -EOPNOTSUPP;

	if (igb_wol_exclusion(adapter, wol) ||
	    !device_can_wakeup(&adapter->pdev->dev))
		return wol->wolopts ? -EOPNOTSUPP : 0;

	switch (hw->device_id) {
	default:
		break;
	}

	/* these settings will always override what we currently have */
	adapter->wol = 0;

	if (wol->wolopts & WAKE_UCAST)
		adapter->wol |= E1000_WUFC_EX;
	if (wol->wolopts & WAKE_MCAST)
		adapter->wo