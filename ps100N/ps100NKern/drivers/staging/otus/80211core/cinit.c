00_ICR);
	igb_irq_enable(adapter);

	netif_tx_start_all_queues(adapter->netdev);

	/* Fire a link change interrupt to start the watchdog. */
	wr32(E1000_ICS, E1000_ICS_LSC);
	return 0;
}

void igb_down(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	u32 tctl, rctl;
	int i;

	/* signal that we're down so the interrupt handler does not
	 * reschedule our watchdog timer */
	set_bit(__IGB_DOWN, &adapter->state);

	/* disable receives in the hardware */
	rctl = rd32(E1000_RCTL);
	wr32(E1000_RCTL, rctl & ~E1000_RCTL_EN);
	/* flush and sleep below */

	netif_tx_stop_all_queues(netdev);

	/* disable transmits in the hardware */
	tctl = rd32(E1000_TCTL);
	tctl &= ~E1000_TCTL_EN;
	wr32(E1000_TCTL, tctl);
	/* flush both disables and wait for them to finish */
	wrfl();
	msleep(10);

	for (i = 0; i < adapter->num_rx_queues; i++)
		napi_disable(&adapter->rx_ring[i].napi);

	igb_irq_disable(adapter);

	del_timer_sync(&adapter->watchdog_timer);
	del_timer_sync(&adapter->phy_info_timer);

	netdev->tx_queue_len = adapter->tx_queue_len;
	netif_carrier_off(netdev);

	/* record the stats before reset*/
	igb_update_stats(adapter);

	adapter->link_speed = 0;
	adapter->link_duplex = 0;

	if (!pci_channel_offline(adapter->pdev))
		igb_reset(adapter);
	igb_clean_all_tx_rings(adapter);
	igb_clean_all_rx_rings(adapter);
#ifdef CONFIG_IGB_DCA

	/* since we reset the hardware DCA settings were cleared */
	igb_setup_dca(adapter);
#endif
}

void igb_reinit_locked(struct igb_adapter *adapter)
{
	WARN_ON(in_interrupt());
	while (test_and_set_bit(__IGB_RESETTING, &adapter->state))
		msleep(1);
	igb_down(adapter);
	igb_up(adapter);
	clear_bit(__IGB_RESETTING, &adapter->state);
}

void igb_reset(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct e1000_mac_info *mac = &hw->mac;
	struct e1000_fc_info *fc = &hw->fc;
	u32 pba = 0, tx_space, min_tx_space, min_rx_space;
	u16 hwm;

	/* Repartition Pba for greater than 9k mtu
	 * To take effect CTRL.RST is required.
	 */
	switch (mac->type) {
	case e1000_82576:
		pba = E1000_PBA_64K;
		break;
	case e1000_82575:
	default:
		pba = E1000_PBA_34K;
		break;
	}

	if ((adapter->max_frame_size > ETH_FRAME_LEN + ETH_FCS_LEN) &&
	    (mac->type < e1000_82576)) {
		/* adjust PBA for jumbo frames */
		wr32(E1000_PBA, pba);

		/* To maintain wire speed transmits, the Tx FIFO should be
		 * large enough to accommodate two full transmit packets,
		 * rounded up to the next 1KB and expressed in KB.  Likewise,
		 * the Rx FIFO should be large enough to accommodate at least
		 * one full receive packet and is similarly rounded up and
		 * expressed in KB. */
		pba = rd32(E1000_PBA);
		/* upper 16 bits has Tx packet buffer allocation size in KB */
		tx_space = pba >> 16;
		/* lower 16 bits has Rx packet buffer allocation size in KB */
		pba &= 0xffff;
		/* the tx fifo also stores 16 bytes of information about the tx
		 * but don't include ethernet FCS because hardware appends it */
		min_tx_space = (adapter->max_frame_size +
				sizeof(union e1000_adv_tx_desc) -
				ETH_FCS_LEN) * 2;
		min_tx_space = ALIGN(min_tx_space, 1024);
		min_tx_space >>= 10;
		/* software strips receive CRC, so leave room for it */
		min_rx_space = adapter->max_frame_size;
		min_rx_space = ALIGN(min_rx_space, 1024);
		min_rx_space >>= 10;

		/* If current Tx allocation is less than the min Tx FIFO size,
		 * and the min Tx FIFO size is less than the current Rx FIFO
		 * allocation, take space away from current Rx allocation */
		if (tx_space < min_tx_space &&
		    ((min_tx_space - tx_space) < pba)) {
			pba = pba - (min_tx_space - tx_space);

			/* if short on rx space, rx wins and must trump tx
			 * adjustment */
			if (pba < min_rx_space)
				pba = min_rx_space;
		}
		wr32(E1000_PBA, pba);
	}

	/* flow control settings */
	/* The high water mark must be low enough to fit one full frame
	 * (or the size used for early receive) above it in the Rx FIFO.
	 * Set it to the lower of:
	 * - 90% of the Rx FIFO size, or
	 * - the full Rx FIFO size minus one full frame */
	hwm = min(((pba << 10) * 9 / 10),
			((pba << 10) - 2 * adapter->max_frame_size));

	if (mac->type < e1000_82576) {
		fc->high_water = hwm & 0xFFF8;	/* 8-byte granularity */
		fc->low_water = fc->high_water - 8;
	} else {
		fc->high_water = hwm & 0xFFF0;	/* 16-byte granularity */
		fc->low_water = fc->high_water - 16;
	}
	fc->pause_time = 0xFFFF;
	fc->send_xon = 1;
	fc->type = fc->original_type;

	/* disable receive for all VFs and wait one second */
	if (adapter->vfs_allocated_count) {
		int i;
		for (i = 0 ; i < adapter->vfs_allocated_count; i++)
			adapter->vf_data[i].clear_to_send = false;

		/* ping all the active vfs to let them know we are going down */
			igb_ping_all_vfs(adapter);

		/* disable transmits and receives */
		wr32(E1000_VFRE, 0);
		wr32(E1000_VFTE, 0);
	}

	/* Allow time for pending master requests to run */
	adapter->hw.mac.ops.reset_hw(&adapter->hw);
	wr32(E1000_WUC, 0);

	if (adapter->hw.mac.ops.init_hw(&adapter->hw))
		dev_err(&adapter->pdev->dev, "Hardware Error\n");

	igb_update_mng_vlan(adapter);

	/* Enable h/w to recognize an 802.1Q VLAN Ethernet packet */
	wr32(E1000_VET, ETHERNET_IEEE_VLAN_TYPE);

	igb_reset_adaptive(&adapter->hw);
	igb_get_phy_info(&adapter->hw);
}

static const struct net_device_ops igb_netdev_ops = {
	.ndo_open 		= igb_open,
	.ndo_stop		= igb_close,
	.ndo_start_xmit		= igb_xmit_frame_adv,
	.ndo_get_stats		= igb_get_stats,
	.ndo_set_multicast_list	= igb_set_multi,
	.ndo_set_mac_address	= igb_set_mac,
	.ndo_change_mtu		= igb_change_mtu,
	.ndo_do_ioctl		= igb_ioctl,
	.ndo_tx_timeout		= igb_tx_timeout,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_vlan_rx_register	= igb_vlan_rx_register,
	.ndo_vlan_rx_add_vid	= igb_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= igb_vlan_rx_kill_vid,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= igb_netpoll,
#endif
};

/**
 * igb_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in igb_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * igb_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
static int __devinit igb_probe(struct pci_dev *pdev,
			       const struct pci_device_id *ent)
{
	struct net_device *netdev;
	struct igb_adapter *adapter;
	struct e1000_hw *hw;
	const struct e1000_info *ei = igb_info_tbl[ent->driver_data];
	unsigned long mmio_start, mmio_len;
	int err, pci_using_dac;
	u16 eeprom_data = 0;
	u16 eeprom_apme_mask = IGB_EEPROM_APME;
	u32 part_num;

	err = pci_enable_device_mem(pdev);
	if (err)
		return err;

	pci_using_dac = 0;
	err = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (!err) {
		err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
		if (!err)
			pci_using_dac = 1;
	} else {
		err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if (err) {
			err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
			if (err) {
				dev_err(&pdev->dev, "No usable DMA "
					"configuration, aborting\n");
				goto err_dma;
			}
		}
	}

	err = pci_request_selected_regions(pdev, pci_select_bars(pdev,
	                                   IORESOURCE_MEM),
	                                   igb_driver_name);
	if (err)
		goto err_pci_reg;

	err = pci_enable_pcie_error_reporting(pdev);
	if (err) {
		dev_err(&pdev->dev, "pci_enable_pcie_error_reporting failed "
		        "0x%x\n", err);
		/* non-fatal, continue */
	}

	pci_set_master(pdev);
	pci_save_state(pdev);

	err = -ENOMEM;
	netdev = alloc_etherdev_mq(sizeof(struct igb_adapter),
	                           IGB_ABS_MAX_TX_QUEUES);
	if (!netdev)
		goto err_alloc_etherdev;

	SET_NETDEV_DEV(netdev, &pdev->dev);

	pci_set_drvdata(pdev, netdev);
	adapter = netdev_priv(netdev);
	adapter->netdev = netdev;
	adapter->pdev = pdev;
	hw = &adapter->hw;
	hw->back = adapter;
	adapter->msg_enable = NETIF_MSG_DRV | NETIF_MSG_PROBE;

	mmio_start = pci_resource_start(pdev, 0);
	mmio_len = pci_resource_len(pdev, 0);

	err = -EIO;
	hw->hw_addr = ioremap(mmio_start, mmio_len);
	if (!hw->hw_addr)
		goto err_ioremap;

	netdev->netdev_ops = &igb_netdev_ops;
	igb_set_ethtool_ops(netdev);
	netdev->watchdog_timeo = 5 * HZ;

	strncpy(netdev->name, pci_name(pdev), sizeof(netdev->name) - 1);

	netdev->mem_start = mmio_start;
	netdev->mem_end = mmio_start + mmio_len;

	/* PCI config space info */
	hw->vendor_id = pdev->vendor;
	hw->device_id = pdev->device;
	hw->revision_id = pdev->revision;
	hw->subsystem_vendor_id = pdev->subsystem_vendor;
	hw->subsystem_device_id = pdev->subsystem_device;

	/* setup the private structure */
	hw->back = adapter;
	/* Copy the default MAC, PHY and NVM function pointers */
	memcpy(&hw->mac.ops, ei->mac_ops, sizeof(hw->mac.ops));
	memcpy(&hw->phy.ops, ei->phy_ops, sizeof(hw->phy.ops));
	memcpy(&hw->nvm.ops, ei->nvm_ops, sizeof(hw->nvm.ops));
	/* Initialize skew-specific constants */
	err = ei->get_invariants(hw);
	if (err)
		goto err_sw_init;

#ifdef CONFIG_PCI_IOV
	/* since iov functionality isn't critical to base device function we
	 * can accept failure.  If it fails we don't allow iov to be enabled */
	if (hw->mac.type == e1000_82576) {
		/* 82576 supports a maximum of 7 VFs in addition to the PF */
		unsigned int num_vfs = (max_vfs > 7) ? 7 : max_vfs;
		int i;
		unsigned char mac_addr[ETH_ALEN];

		if (num_vfs) {
			adapter->vf_data = kcalloc(num_vfs,
						sizeof(struct vf_data_storage),
						GFP_KERNEL);
			if (!adapter->vf_data) {
				dev_err(&pdev->dev,
				        "Could not allocate VF private data - "
					"IOV enable failed\n");
			} else {
				err = pci_enable_sriov(pdev, num_vfs);
				if (!err) {
					adapter->vfs_allocated_count = num_vfs;
					dev_info(&pdev->dev,
					         "%d vfs allocated\n",
					         num_vfs);
					for (i = 0;
					     i < adapter->vfs_allocated_count;
					     i++) {
						random_ether_addr(mac_addr);
						igb_set_vf_mac(adapter, i,
						               mac_addr);
					}
				} else {
					kfree(adapter->vf_data);
					adapter->vf_data = NULL;
				}
			}
		}
	}

#endif
	/* setup the private structure */
	err = igb_sw_init(adapter);
	if (err)
		goto err_sw_init;

	igb_get_bus_info_pcie(hw);

	/* set flags */
	switch (hw->mac.type) {
	case e1000_82575:
		adapter->flags |= IGB_FLAG_NEED_CTX_IDX;
		break;
	case e1000_82576:
	default:
		break;
	}

	hw->phy.autoneg_wait_to_complete = false;
	hw->mac.adaptive_ifs = true;

	/* Copper options */
	if (hw->phy.media_type == e1000_media_type_copper) {
		hw->phy.mdix = AUTO_ALL_MODES;
		hw->phy.disable_polarity_correction = false;
		hw->phy.ms_type = e1000_ms_hw_default;
	}

	if (igb_check_reset_block(hw))
		dev_info(&pdev->dev,
			"PHY reset is blocked due to SOL/IDER session.\n");

	netdev->features = NETIF_F_SG |
			   NETIF_F_IP_CSUM |
			   NETIF_F_HW_VLAN_TX |
			   NETIF_F_HW_VLAN_RX |
			   NETIF_F_HW_VLAN_FILTER;

	netdev->features |= NETIF_F_IPV6_CSUM;
	netdev->features |= NETIF_F_TSO;
	netdev->features |= NETIF_F_TSO6;

	netdev->features |= NETIF_F_GRO;

	netdev->vlan_features |= NETIF_F_TSO;
	netdev->vlan_features |= NETIF_F_TSO6;
	netdev->vlan_features |= NETIF_F_IP_CSUM;
	netdev->vlan_features |= NETIF_F_SG;

	if (pci_using_dac)
		netdev->features |= NETIF_F_HIGHDMA;

	if (adapter->hw.mac.type == e1000_82576)
		netdev->features |= NETIF_F_SCTP_CSUM;

	adapter->en_mng_pt = igb_enable_mng_pass_thru(&adapter->hw);

	/* before reading the NVM, reset the controller to put the device in a
	 * known good starting state */
	hw->mac.ops.reset_hw(hw);

	/* make sure the NVM is good */
	if (igb_validate_nvm_checksum(hw) < 0) {
		dev_err(&pdev->dev, "The NVM Checksum Is Not Valid\n");
		err = -EIO;
		goto err_eeprom;
	}

	/* copy the MAC address out of the NVM */
	if (hw->mac.ops.read_mac_addr(hw))
		dev_err(&pdev->dev, "NVM Read Error\n");

	memcpy(netdev->dev_addr, hw->mac.addr, netdev->addr_len);
	memcpy(netdev->perm_addr, hw->mac.addr, netdev->addr_len);

	if (!is_valid_ether_addr(netdev->perm_addr)) {
		dev_err(&pdev->dev, "Invalid MAC Address\n");
		err = -EIO;
		goto err_eeprom;
	}

	setup_timer(&adapter->watchdog_timer, &igb_watchdog,
	            (unsigned long) adapter);
	setup_timer(&adapter->phy_info_timer, &igb_update_phy_info,
	            (unsigned long) adapter);

	INIT_WORK(&adapter->reset_task, igb_reset_task);
	INIT_WORK(&adapter->watchdog_task, igb_watchdog_task);

	/* Initialize link properties that are user-changeable */
	adapter->fc_autoneg = true;
	hw->mac.autoneg = true;
	hw->phy.autoneg_advertised = 0x2f;

	hw->fc.original_type = e1000_fc_default;
	hw->fc.type = e1000_fc_default;

	adapter->itr_setting = IGB_DEFAULT_ITR;
	adapter->itr = IGB_START_ITR;

	igb_validate_mdi_setting(hw);

	/* Initial Wake on LAN setting If APM wake is enabled in the EEPROM,
	 * enable the ACPI Magic Packet filter
	 */

	if (hw->bus.func == 0)
		hw->nvm.ops.read(hw, NVM_INIT_CONTROL3_PORT_A, 1, &eeprom_data);
	else if (hw->bus.func == 1)
		hw->nvm.ops.read(hw, NVM_INIT_CONTROL3_PORT_B, 1, &eeprom_data);

	if (eeprom_data & eeprom_apme_mask)
		adapter->eeprom_wol |= E1000_WUFC_MAG;

	/* now that we have the eeprom settings, apply the special cases where
	 * the eeprom may be wrong or the board simply won't support wake on
	 * lan on a particular port */
	switch (pdev->device) {
	case E1000_DEV_ID_82575GB_QUAD_COPPER:
		adapter->eeprom_wol = 0;
		break;
	case E1000_DEV_ID_82575EB_FIBER_SERDES:
	case E1000_DEV_ID_82576_FIBER:
	case E1000_DEV_ID_82576_SERDES:
		/* Wake events only supported on port A for dual fiber
		 * regardless of eeprom setting */
		if (rd32(E1000_STATUS) & E1000_STATUS_FUNC_1)
			adapter->eeprom_wol = 0;
		break;
	case E1000_DEV_ID_82576_QUAD_COPPER:
		/* if quad port adapter, disable WoL on all but port A */
		if (global_quad_port_a != 0)
			adapter->eeprom_wol = 0;
		else
			adapter->flags |= IGB_FLAG_QUAD_PORT_A;
		/* Reset for multiple quad port adapters */
		if (++global_quad_port_a == 4)
			global_quad_port_a = 0;
		break;
	}

	/* initialize the wol settings based on the eeprom settings */
	adapter->wol = adapter->eeprom_wol;
	device_set_wakeup_enable(&adapter->pdev->dev, adapter->wol);

	/* reset the hardware with the new settings */
	igb_reset(adapter);

	/* let the f/w know that the h/w is now under the control of the
	 * driver. */
	igb_get_hw_control(adapter);

	strcpy(netdev->name, "eth%d");
	err = register_netdev(netdev);
	if (err)
		goto err_register;

	/* carrier off reporting is important to ethtool even BEFORE open */
	netif_carrier_off(netdev);

#ifdef CONFIG_IGB_DCA
	if (dca_add_requester(&pdev->dev) == 0) {
		adapter->flags |= IGB_FLAG_DCA_ENABLED;
		dev_info(&pdev->dev, "DCA enabled\n");
		igb_setup_dca(adapter);
	}
#endif

	/*
	 * Initialize hardware timer: we keep it running just in case
	 * that some program needs it later on.
	 */
	memset(&adapter->cycles, 0, sizeof(adapter->cycles));
	adapter->cycles.read = igb_read_clock;
	adapter->cycles.mask = CLOCKSOURCE_MASK(64);
	adapter->cycles.mult = 1;
	adapter->cycles.shift = IGB_TSYNC_SHIFT;
	wr32(E1000_TIMINCA,
	     (1<<24) |
	     IGB_TSYNC_CYCLE_TIME_IN_NANOSECONDS * IGB_TSYNC_SCALE);
#if 0
	/*
	 * Avoid rollover while we initialize by resetting the time counter.
	 */
	wr32(E1000_SYSTIML, 0x00000000);
	wr32(E1000_SYSTIMH, 0x00000000);
#else
	/*
	 * Set registers so that rollover occurs soon to test this.
	 */
	wr32(E1000_SYSTIML, 0x00000000);
	wr32(E1000_SYSTIMH, 0xFF800000);
#endif
	wrfl();
	timecounter_init(&adapter->clock,
			 &adapter->cycles,
			 ktime_to_ns(ktime_get_real()));

	/*
	 * Synchronize our NIC clock against system wall clock. NIC
	 * time stamp reading requires ~3us per sample, each sample
	 * was pretty stable even under load => only require 10
	 * samples for each offset comparison.
	 */
	memset(&adapter->compare, 0, sizeof(adapter->compare));
	adapter->compare.source = &adapter->clock;
	adapter->compare.target = ktime_get_real;
	adapter->compare.num_samples = 10;
	timecompare_update(&adapter->compare, 0);

#ifdef DEBUG
	{
		char buffer[160];
		printk(KERN_DEBUG
			"igb: %s: hw %p initialized timer\n",
			igb_get_time_str(adapter, buffer),
			&adapter->hw);
	}
#endif

	dev_info(&pdev->dev, "Intel(R) Gigabit Ethernet Network Connection\n");
	/* print bus type/speed/width info */
	dev_info(&pdev->dev, "%s: (PCIe:%s:%s) %pM\n",
		 netdev->name,
		 ((hw->bus.speed == e1000_bus_speed_2500)
		  ? "2.5Gb/s" : "unknown"),
		 ((hw->bus.width == e1000_bus_width_pcie_x4) ? "Width x4" :
		  (hw->bus.width == e1000_bus_width_pcie_x2) ? "Width x2" :
		  (hw->bus.width == e1000_bus_width_pcie_x1) ? "Width x1" :
		   "unknown"),
		 netdev->dev_addr);

	igb_read_part_num(hw, &part_num);
	dev_info(&pdev->dev, "%s: PBA No: %06x-%03x\n", netdev->name,
		(part_num >> 8), (part_num & 0xff));

	dev_info(&pdev->dev,
		"Using %s interrupts. %d rx queue(s), %d tx queue(s)\n",
		adapter->msix_entries ? "MSI-X" :
		(adapter->flags & IGB_FLAG_HAS_MSI) ? "MSI" : "legacy",
		adapter->num_rx_queues, adapter->num_tx_queues);

	return 0;

err_register:
	igb_release_hw_control(adapter);
err_eeprom:
	if (!igb_check_reset_block(hw))
		igb_reset_phy(hw);

	if (hw->flash_address)
		iounmap(hw->flash_address);

	igb_free_queues(adapter);
err_sw_init:
	iounmap(hw->hw_addr);
err_ioremap:
	free_netdev(netdev);
err_alloc_etherdev:
	pci_release_selected_regions(pdev, pci_select_bars(pdev,
	                             IORESOURCE_MEM));
err_pci_reg:
err_dma:
	pci_disable_device(pdev);
	return err;
}

/**
 * igb_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * igb_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/
static void __devexit igb_remove(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	int err;

	/* flush_scheduled work may reschedule our watchdog task, so
	 * explicitly disable watchdog tasks from being rescheduled  */
	set_bit(__IGB_DOWN, &adapter->state);
	del_timer_sync(&adapter->watchdog_timer);
	del_timer_sync(&adapter->phy_info_timer);

	flush_scheduled_work();

#ifdef CONFIG_IGB_DCA
	if (adapter->flags & IGB_FLAG_DCA_ENABLED) {
		dev_info(&pdev->dev, "DCA disabled\n");
		dca_remove_requester(&pdev->dev);
		adapter->flags &= ~IGB_FLAG_DCA_ENABLED;
		wr32(E1000_DCA_CTRL, E1000_DCA_CTRL_DCA_MODE_DISABLE);
	}
#endif

	/* Release control of h/w to f/w.  If f/w is AMT enabled, this
	 * would have already happened in close and is redundant. */
	igb_release_hw_control(adapter);

	unregister_netdev(netdev);

	if (!igb_check_reset_block(&adapter->hw))
		igb_reset_phy(&adapter->hw);

	igb_reset_interrupt_capability(adapter);

	igb_free_queues(adapter);

#ifdef CONFIG_PCI_IOV
	/* reclaim resources allocated to VFs */
	if (adapter->vf_data) {
		/* disable iov and allow time for transactions to clear */
		pci_disable_sriov(pdev);
		msleep(500);

		kfree(adapter->vf_data);
		adapter->vf_data = NULL;
		wr32(E1000_IOVCTL, E1000_IOVCTL_REUSE_VFQ);
		msleep(100);
		dev_info(&pdev->dev, "IOV Disabled\n");
	}
#endif
	iounmap(hw->hw_addr);
	if (hw->flash_address)
		iounmap(hw->flash_address);
	pci_release_selected_regions(pdev, pci_select_bars(pdev,
	                             IORESOURCE_MEM));

	free_netdev(netdev);

	err = pci_disable_pcie_error_reporting(pdev);
	if (err)
		dev_err(&pdev->dev,
		        "pci_disable_pcie_error_reporting failed 0x%x\n", err);

	pci_disable_device(pdev);
}

/**
 * igb_sw_init - Initialize general software structures (struct igb_adapter)
 * @adapter: board private structure to initialize
 *
 * igb_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/
static int __devinit igb_sw_init(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;

	pci_read_config_word(pdev, PCI_COMMAND, &hw->bus.pci_cmd_word);

	adapter->tx_ring_count = IGB_DEFAULT_TXD;
	adapter->rx_ring_count = IGB_DEFAULT_RXD;
	adapter->rx_buffer_len = MAXIMUM_ETHERNET_VLAN_SIZE;
	adapter->rx_ps_hdr_size = 0; /* disable packet split */
	adapter->max_frame_size = netdev->mtu + ETH_HLEN + ETH_FCS_LEN;
	adapter->min_frame_size = ETH_ZLEN + ETH_FCS_LEN;

	/* This call may decrease the number of queues depending on
	 * interrupt mode. */
	igb_set_interrupt_capability(adapter);

	if (igb_alloc_queues(adapter)) {
		dev_err(&pdev->dev, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	/* Explicitly disable IRQ since the NIC can be in any state. */
	igb_irq_disable(adapter);

	set_bit(__IGB_DOWN, &adapter->state);
	return 0;
}

/**
 * igb_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 **/
static int igb_open(struct net_device *netdev)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	int err;
	int i;

	/* disallow open during test */
	if (test_bit(__IGB_TESTING, &adapter->state))
		return -EBUSY;

	netif_carrier_off(netdev);

	/* allocate transmit descriptors */
	err = igb_setup_all_tx_resources(adapter);
	if (err)
		goto err_setup_tx;

	/* allocate receive descriptors */
	err = igb_setup_all_rx_resources(adapter);
	if (err)
		goto err_setup_rx;

	/* e1000_power_up_phy(adapter); */

	adapter->mng_vlan_id = IGB_MNG_VLAN_NONE;
	if ((adapter->hw.mng_cookie.status &
	     E1000_MNG_DHCP_COOKIE_STATUS_VLAN))
		igb_update_mng_vlan(adapter);

	/* before we allocate an interrupt, we must be ready to handle it.
	 * Setting DEBUG_SHIRQ in the kernel makes it fire an interrupt
	 * as soon as we call pci_request_irq, so we have to setup our
	 * clean_rx handler before we do so.  */
	igb_configure(adapter);

	igb_vmm_control(adapter);
	igb_set_rah_pool(hw, adapter->vfs_allocated_count, 0);
	igb_set_vmolr(hw, adapter->vfs_allocated_count);

	err = igb_request_irq(adapter);
	if (err)
		goto err_req_irq;

	/* From here on the code is the same as igb_up() */
	clear_bit(__IGB_DOWN, &adapter->state);

	for (i = 0; i < adapter->num_rx_queues; i++)
		napi_enable(&adapter->rx_ring[i].napi);

	/* Clear any pending interrupts. */
	rd32(E1000_ICR);

	igb_irq_enable(adapter);

	netif_tx_start_all_queues(netdev);

	/* Fire a link status change interrupt to start the watchdog. */
	wr32(E1000_ICS, E1000_ICS_LSC);

	return 0;

err_req_irq:
	igb_release_hw_control(adapter);
	/* e1000_power_down_phy(adapter); */
	igb_free_all_rx_resources(adapter);
err_setup_rx:
	igb_free_all_tx_resources(adapter);
err_setup_tx:
	igb_reset(adapter);

	return err;
}

/**
 * igb_close - Disables a network interface
 * @netdev: network interface device structure
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the driver's control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 **/
static int igb_close(struct net_device *netdev)
{
	struct igb_adapter *adapter = netdev_priv(netdev);

	WARN_ON(test_bit(__IGB_RESETTING, &adapter->state));
	igb_down(adapter);

	igb_free_irq(adapter);

	igb_free_all_tx_resources(adapter);
	igb_free_all_rx_resources(adapter);

	/* kill manageability vlan ID if supported, but not if a vlan with
	 * the same ID is registered on the host OS (let 8021q kill it) */
	if ((adapter->hw.mng_cookie.status &
			  E1000_MNG_DHCP_COOKIE_STATUS_VLAN) &&
	     !(adapter->vlgrp &&
	       vlan_group_get_device(adapter->vlgrp, adapter->mng_vlan_id)))
		igb_vlan_rx_kill_vid(netdev, adapter->mng_vlan_id);

	return 0;
}

/**
 * igb_setup_tx_resources - allocate Tx resources (Descriptors)
 * @adapter: board private structure
 * @tx_ring: tx descriptor ring (for a specific queue) to setup
 *
 * Return 0 on success, negative on failure
 **/
int igb_setup_tx_resources(struct igb_adapter *adapter,
			   struct igb_ring *tx_ring)
{
	struct pci_dev *pdev = adapter->pdev;
	int size;

	size = sizeof(struct igb_buffer) * tx_ring->count;
	tx_ring->buffer_info = vmalloc(size);
	if (!tx_ring->buffer_info)
		goto err;
	memset(tx_ring->buffer_info, 0, size);

	/* round up to nearest 4K */
	tx_ring->size = tx_ring->count * sizeof(union e1000_adv_tx_desc);
	tx_ring->size = ALIGN(tx_ring->size, 4096);

	tx_ring->desc = pci_alloc_consistent(pdev, tx_ring->size,
					     &tx_ring->dma);

	if (!tx_ring->desc)
		goto err;

	tx_ring->adapter = adapter;
	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
	return 0;

err:
	vfree(tx_ring->buffer_info);
	dev_err(&adapter->pdev->dev,
		"Unable to allocate memory for the transmit descriptor ring\n");
	return -ENOMEM;
}

/**
 * igb_setup_all_tx_resources - wrapper to allocate Tx resources
 *				  (Descriptors) for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/
static int igb_setup_all_tx_resources(struct igb_adapter *adapter)
{
	int i, err = 0;
	int r_idx;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		err = igb_setup_tx_resources(adapter, &adapter->tx_ring[i]);
		if (err) {
			dev_err(&adapter->pdev->dev,
				"Allocation for Tx Queue %u failed\n", i);
			for (i--; i >= 0; i--)
				igb_free_tx_resources(&adapter->tx_ring[i]);
			break;
		}
	}

	for (i = 0; i < IGB_MAX_TX_QUEUES; i++) {
		r_idx = i % adapter->num_tx_queues;
		adapter->multi_tx_table[i] = &adapter->tx_ring[r_idx];
	}
	return err;
}

/**
 * igb_configure_tx - Configure transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/
static void igb_configure_tx(struct igb_adapter *adapter)
{
	u64 tdba;
	struct e1000_hw *hw = &adapter->hw;
	u32 tctl;
	u32 txdctl, txctrl;
	int i, j;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct igb_ring *ring = &adapter->tx_ring[i];
		j = ring->reg_idx;
		wr32(E1000_TDLEN(j),
		     ring->count * sizeof(union e1000_adv_tx_desc));
		tdba = ring->dma;
		wr32(E1000_TDBAL(j),
		     tdba & 0x00000000ffffffffULL);
		wr32(E1000_TDBAH(j), tdba >> 32);

		ring->head = E1000_TDH(j);
		ring->tail = E1000_TDT(j);
		writel(0, hw->hw_addr + ring->tail);
		writel(0, hw->hw_addr + ring->head);
		txdctl = rd32(E1000_TXDCTL(j));
		txdctl |= E1000_TXDCTL_QUEUE_ENABLE;
		wr32(E1000_TXDCTL(j), txdctl);

		/* Turn off Relaxed Ordering on head write-backs.  The
		 * writebacks MUST be delivered in order or it will
		 * completely screw up our bookeeping.
		 */
		txctrl = rd32(E1000_DCA_TXCTRL(j));
		txctrl &= ~E1000_DCA_TXCTRL_TX_WB_RO_EN;
		wr32(E1000_DCA_TXCTRL(j), txctrl);
	}

	/* disable queue 0 to prevent tail bump w/o re-configuration */
	if (adapter->vfs_allocated_count)
		wr32(E1000_TXDCTL(0), 0);

	/* Program the Transmit Control Register */
	tctl = rd32(E1000_TCTL);
	tctl &= ~E1000_TCTL_CT;
	tctl |= E1000_TCTL_PSP | E1000_TCTL_RTLC |
		(E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);

	igb_config_collision_dist(hw);

	/* Setup Transmit Descriptor Settings for eop descriptor */
	adapter->txd_cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;

	/* Enable transmits */
	tctl |= E1000_TCTL_EN;

	wr32(E1000_TCTL, tctl);
}

/**
 * igb_setup_rx_resources - allocate Rx resources (Descriptors)
 * @adapter: board private structure
 * @rx_ring:    rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 **/
int igb_setup_rx_resources(struct igb_adapter *adapter,
			   struct igb_ring *rx_ring)
{
	struct pci_dev *pdev = adapter->pdev;
	int size, desc_len;

	size = sizeof(struct igb_buffer) * rx_ring->count;
	rx_ring->buffer_info = vmalloc(size);
	if (!rx_ring->buffer_info)
		goto err;
	memset(rx_ring->buffer_info, 0, size);

	desc_len = sizeof(union e1000_adv_rx_desc);

	/* Round up to nearest 4K */
	rx_ring->size = rx_ring->count * desc_len;
	rx_ring->size = ALIGN(rx_ring->size, 4096);

	rx_ring->desc = pci_alloc_consistent(pdev, rx_ring->size,
					     &rx_ring->dma);

	if (!rx_ring->desc)
		goto err;

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;

	rx_ring->adapter = adapter;

	return 0;

err:
	vfree(rx_ring->buffer_info);
	dev_err(&adapter->pdev->dev, "Unable to allocate memory for "
		"the receive descriptor ring\n");
	return -ENOMEM;
}

/**
 * igb_setup_all_rx_resources - wrapper to allocate Rx resources
 *				  (Descriptors) for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/
static int igb_setup_all_rx_resources(struct igb_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_rx_queues; i++) {
		err = igb_setup_rx_resources(adapter, &adapter->rx_ring[i]);
		if (err) {
			dev_err(&adapter->pdev->dev,
				"Allocation for Rx Queue %u failed\n", i);
			for (i--; i >= 0; i--)
				igb_free_rx_resources(&adapter->rx_ring[i]);
			break;
		}
	}

	return err;
}

/**
 * igb_setup_rctl - configure the receive control registers
 * @adapter: Board private structure
 **/
static void igb_setup_rctl(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 rctl;
	u32 srrctl = 0;
	int i;

	rctl = rd32(E1000_RCTL);

	rctl &= ~(3 << E1000_RCTL_MO_SHIFT);
	rctl &= ~(E1000_RCTL_LBM_TCVR | E1000_RCTL_LBM_MAC);

	rctl |= E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_RDMTS_HALF |
		(hw->mac.mc_filter_type << E1000_RCTL_MO_SHIFT);

	/*
	 * enable stripping of CRC. It's unlikely this will break BMC
	 * redirection as it did with e1000. Newer features require
	 * that the HW strips the CRC.
	 */
	rctl |= E1000_RCTL_SECRC;

	/*
	 * disable store bad packets and clear size bits.
	 */
	rctl &= ~(E1000_RCTL_SBP | E1000_RCTL_SZ_256);

	/* enable LPE when to prevent packets larger than max_frame_size */
		rctl |= E1000_RCTL_LPE;

	/* Setup buffer sizes */
	switch (adapter->rx_buffer_len) {
	case IGB_RXBUFFER_256:
		rctl |= E1000_RCTL_SZ_256;
		break;
	case IGB_RXBUFFER_512:
		rctl |= E1000_RCTL_SZ_512;
		break;
	default:
		srrctl = ALIGN(adapter->rx_buffer_len, 1024)
		         >> E1000_SRRCTL_BSIZEPKT_SHIFT;
		break;
	}

	/* 82575 and greater support packet-split where the protocol
	 * header is placed in skb->data and the packet data is
	 * placed in pages hanging off of skb_shinfo(skb)->nr_frags.
	 * In the case of a non-split, skb->data is linearly filled,
	 * followed by the page buffers.  Therefore, skb->data is
	 * sized to hold the largest protocol header.
	 */
	/* allocations using alloc_page take too long for regular MTU
	 * so only enable packet split for jumbo frames */
	if (adapter->netdev->mtu > ETH_DATA_LEN) {
		adapter->rx_ps_hdr_size = IGB_RXBUFFER_128;
		srrctl |= adapter->rx_ps_hdr_size <<
			 E1000_SRRCTL_BSIZEHDRSIZE_SHIFT;
		srrctl |= E1000_SRRCTL_DESCTYPE_HDR_SPLIT_ALWAYS;
	} else {
		adapter->rx_ps_hdr_size = 0;
		srrctl |= E1000_SRRCTL_DESCTYPE_ADV_ONEBUF;
	}

	/* Attention!!!  For SR-IOV PF driver operations you must enable
	 * queue drop for all VF and PF queues to prevent head of line blocking
	 * if an un-trusted VF does not provide descriptors to hardware.
	 */
	if (adapter->vfs_allocated_count) {
		u32 vmolr;

		/* set all queue drop enable bits */
		wr32(E1000_QDE, ALL_QUEUES);
		srrctl |= E1000_SRRCTL_DROP_EN;

		/* disable queue 0 to prevent tail write w/o re-config */
		wr32(E1000_RXDCTL(0), 0);

		vmolr = rd32(E1000_VMOLR(adapter->vfs_allocated_count));
		if (rctl & E1000_RCTL_LPE)
			vmolr |= E1000_VMOLR_LPE;
		if (adapter->num_rx_queues > 1)
			vmolr |= E1000_VMOLR_RSSE;
		wr32(E1000_VMOLR(adapter->vfs_allocated_count), vmolr);
	}

	for (i = 0; i < adapter->num_rx_queues; i++) {
		int j = adapter->rx_ring[i].reg_idx;
		wr32(E1000_SRRCTL(j), srrctl);
	}

	wr32(E1000_RCTL, rctl);
}

/**
 * igb_rlpml_set - set maximum receive packet size
 * @adapter: board private structure
 *
 * Configure maximum receivable packet size.
 **/
static void igb_rlpml_set(struct igb_adapter *adapter)
{
	u32 max_frame_size = adapter->max_frame_size;
	struct e1000_hw *hw = &adapter->hw;
	u16 pf_id = adapter->vfs_allocated_count;

	if (adapter->vlgrp)
		max_frame_size += VLAN_TAG_SIZE;

	/* if vfs are enabled we set RLPML to the largest possible request
	 * size and set the VMOLR RLPML to the size we need */
	if (pf_id) {
		igb_set_vf_rlpml(adapter, max_frame_size, pf_id);
		max_frame_size = MAX_STD_JUMBO_FRAME_SIZE + VLAN_TAG_SIZE;
	}

	wr32(E1000_RLPML, max_frame_size);
}

/**
 * igb_configure_vt_default_pool - Configure VT default pool
 * @adapter: board private structure
 *
 * Configure the default pool
 **/
static void igb_configure_vt_default_pool(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u16 pf_id = adapter->vfs_allocated_count;
	u32 vtctl;

	/* not in sr-iov mode - do nothing */
	if (!pf_id)
		return;

	vtctl = rd32(E1000_VT_CTL);
	vtctl &= ~(E1000_VT_CTL_DEFAULT_POOL_MASK |
		   E1000_VT_CTL_DISABLE_DEF_POOL);
	vtctl |= pf_id << E1000_VT_CTL_DEFAULT_POOL_SHIFT;
	wr32(E1000_VT_CTL, vtctl);
}

/**
 * igb_configure_rx - Configure receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void igb_configure_rx(struct igb_adapter *adapter)
{
	u64 rdba;
	struct e1000_hw *hw = &adapter->hw;
	u32 rctl, rxcsum;
	u32 rxdctl;
	int i;

	/* disable receives while setting up the descriptors */
	rctl = rd32(E1000_RCTL);
	wr32(E1000_RCTL, rctl & ~E1000_RCTL_EN);
	wrfl();
	mdelay(10);

	if (adapter->itr_setting > 3)
		wr32(E1000_ITR, adapter->itr);

	/* Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct igb_ring *ring = &adapter->rx_ring[i];
		int j = ring->reg_idx;
		rdba = ring->dma;
		wr32(E1000_RDBAL(j),
		     rdba & 0x00000000ffffffffULL);
		wr32(E1000_RDBAH(j), rdba >> 32);
		wr32(E1000_RDLEN(j),
		     ring->count * sizeof(union e1000_adv_rx_desc));

		ring->head = E1000_RDH(j);
		ring->tail = E1000_RDT(j);
		writel(0, hw->hw_addr + ring->tail);
		writel(0, hw->hw_addr + ring->head);

		rxdctl = rd32(E1000_RXDCTL(j));
		rxdctl |= E1000_RXDCTL_QUEUE_ENABLE;
		rxdctl &= 0xFFF00000;
		rxdctl |= IGB_RX_PTHRESH;
		rxdctl |= IGB_RX_HTHRESH << 8;
		rxdctl |= IGB_RX_WTHRESH << 16;
		wr32(E1000_RXDCTL(j), rxdctl);
	}

	if (adapter->num_rx_queues > 1) {
		u32 random[10];
		u32 mrqc;
		u32 j, shift;
		union e1000_reta {
			u32 dword;
			u8  bytes[4];
		} reta;

		get_random_bytes(&random[0], 40);

		if (hw->mac.type >= e1000_82576)
			shift = 0;
		else
			shift = 6;
		for (j = 0; j < (32 * 4); j++) {
			reta.bytes[j & 3] =
				adapter->rx_ring[(j % adapter->num_rx_queues)].reg_idx << shift;
			if ((j & 3) == 3)
				writel(reta.dword,
				       hw->hw_addr + E1000_RETA(0) + (j & ~3));
		}
		if (adapter->vfs_allocated_count)
			mrqc = E1000_MRQC_ENABLE_VMDQ_RSS_2Q;
		else
			mrqc = E1000_MRQC_ENABLE_RSS_4Q;

		/* Fill out hash function seeds */
		for (j = 0; j < 10; j++)
			array_wr32(E1000_RSSRK(0), j, random[j]);

		mrqc |= (E1000_MRQC_RSS_FIELD_IPV4 |
			 E1000_MRQC_RSS_FIELD_IPV4_TCP);
		mrqc |= (E1000_MRQC_RSS_FIELD_IPV6 |
			 E1000_MRQC_RSS_FIELD_IPV6_TCP);
		mrqc |= (E1000_MRQC_RSS_FIELD_IPV4_UDP |
			 E1000_MRQC_RSS_FIELD_IPV6_UDP);
		mrqc |= (E1000_MRQC_RSS_FIELD_IPV6_UDP_EX |
			 E1000_MRQC_RSS_FIELD_IPV6_TCP_EX);

		wr32(E1000_MRQC, mrqc);
	} else if (adapter->vfs_allocated_count) {
		/* Enable multi-queue for sr-iov */
		wr32(E1000_MRQC, E1000_MRQC_ENABLE_VMDQ);
	}

	/* Enable Receive Checksum Offload for TCP and UDP */
	rxcsum = rd32(E1000_RXCSUM);
	/* Disable raw packet checksumming */
	rxcsum |= E1000_RXCSUM_PCSD;

	if (adapter->hw.mac.type == e1000_82576)
		/* Enable Receive Checksum Offload for SCTP */
		rxcsum |= E1000_RXCSUM_CRCOFL;

	/* Don't need to set TUOFL or IPOFL, they default to 1 */
	wr32(E1000_RXCSUM, rxcsum);

	/* Set the default pool for the PF's first queue */
	igb_configure_vt_default_pool(adapter);

	igb_rlpml_set(adapter);

	/* Enable Receives */
	wr32(E1000_RCTL, rctl);
}

/**
 * igb_free_tx_resources - Free Tx Resources per Queue
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 **/
void igb_free_tx_resources(struct igb_ring *tx_ring)
{
	struct pci_dev *pdev = tx_ring->adapter->pdev;

	igb_clean_tx_ring(tx_ring);

	vfree(tx_         hlen += 2;
                icvLen = 4;
                macCtrl |= 0x40;
                break;

            case ZM_TKIP:
            {
                wd->sta.iv16++;
                if ( wd->sta.iv16 == 0 )
                {
                    wd->sta.iv32++;
                }

                b1 = (u8_t) (wd->sta.iv16 >> 8);
                b2 = (b1 | 0x20) & 0x7f;
                header[hlen] = ((u16_t)b2 << 8) + b1;
                b1 = (u8_t) wd->sta.iv16;
                b2 = 0x20;
                header[hlen+1] = ((u16_t)b2 << 8) + b1;
                header[hlen+2] = (u16_t) wd->sta.iv32;
                header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);

                //macCtrl |= 0x80;
                macCtrl |= 0x40;
                icvLen = 4;

                /* set hardware MIC */
                if ( (!(seq & 0xf))&&(!(flag & 0x4)) )
                {
                    macCtrl |= 0x100;
                    plusLen += 8;
                    *micLen = 8;
                }

                header[4] |= 0x4000;
                hlen += 4;
            }/* end of PSEUDO TKIP */
                break;

            case ZM_AES:
            {
                wd->sta.iv16++;
                if ( wd->sta.iv16 == 0 )
                {
                    wd->sta.iv32++;
                }

                b1 = (u8_t) wd->sta.iv16;
                b2 = (u8_t) (wd->sta.iv16 >> 8);
                header[hlen] = ((u16_t)b2 << 8) + b1;
                header[hlen+1] = 0x2000;
                header[hlen+2] = (u16_t) (wd->sta.iv32);
                header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);
                macCtrl |= 0xc0;
                icvLen = 8;  /* MIC */
                header[4] |= 0x4000;
                hlen += 4;
            }/* end of PSEUDO AES */
                    break;

              #ifdef ZM_ENABLE_CENC
              case ZM_CENC:
                    /* Accumlate the PN sequence */
                    wd->sta.txiv[0] += 2;

                    if (wd->sta.txiv[0] == 0 || wd->sta.txiv[0] == 1)
                    {
                        wd->sta.txiv[1]++;
                    }

                    if (wd->sta.txiv[1] == 0)
                    {
                        wd->sta.txiv[2]++;
                    }

                    if (wd->sta.txiv[2] == 0)
                    {
                        wd->sta.txiv[3]++;
                    }

                    if (wd->sta.txiv[3] == 0)
                    {
                        wd->sta.txiv[0] = 0;
                        wd->sta.txiv[1] = 0;
                        wd->sta.txiv[2] = 0;
                    }

                    header[hlen] = 0;
                    header[hlen+1] = (u16_t) wd->sta.txiv[0];
                    header[hlen+2] = (u16_t) (wd->sta.txiv[0] >> 16);
                    header[hlen+3] = (u16_t) wd->sta.txiv[1];
                    header[hlen+4] = (u16_t) (wd->sta.txiv[1] >> 16);
                    header[hlen+5] = (u16_t) wd->sta.txiv[2];
                    header[hlen+6] = (u16_t) (wd->sta.txiv[2] >> 16);
                    header[hlen+7] = (u16_t) wd->sta.txiv[3];
                    header[hlen+8] = (u16_t) (wd->sta.txiv[3] >> 16);

                    macCtrl |= 0x80;
                    icvLen = 16;  /* MIC */

                    header[4] |= 0x4000;
                    hlen += 9;
				break;
		    #endif //ZM_ENABLE_CENC
			}/* end of switch */
		}

        /* Generate control setting */

        /* protection mode */
        if (wd->enableProtectionMode)
        {
            if (wd->enableProtectionMode==2)
            {
                /* Force enable protection: self cts  */
                macCtrl &= 0xFFFC;
                macCtrl |= 2;
            }
            /* if wd->enableProtectionMode=1 => force disable */
            /* if wd->enableProtectionMode=0 => auto */
        }
        else
        {

            /* protection mode */
            if (wd->sta.bProtectionMode == TRUE)
            {
                /* Enable Self-CTS */
                macCtrl &= 0xFFFC;
                macCtrl |=ean = 0;
	rx_ring->next_to_use = 0;

	writel(0, adapter->hw.hw_addr + rx_ring->head);
	writel(0, adapter->hw.hw_addr + rx_ring->tail);
}

/**
 * igb_clean_all_rx_rings - Free Rx Buffers for all queues
 * @adapter: board private structure
 **/
static void igb_clean_all_rx_rings(struct igb_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		igb_clean_rx_ring(&adapter->rx_ring[i]);
}

/**
 * igb_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 **/
static int igb_set_mac(struct net_device *netdev, void *p)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	memcpy(hw->mac.addr, addr->sa_data, netdev->addr_len);

	hw->mac.ops.rar_set(hw, hw->mac.addr, 0);

	igb_set_rah_pool(hw, adapter->vfs_allocated_count, 0);

	return 0;
}

/**
 * igb_set_multi - Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_multi entry point is called whenever the multicast address
 * list or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper multicast,
 * promiscuous mode, and all-multi behavior.
 **/
static void igb_set_multi(struct net_device *netdev)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	struct e1000_mac_info *mac = &hw->mac;
	struct dev_mc_list *mc_ptr;
	u8  *mta_list = NULL;
	u32 rctl;
	int i;

	/* Check for Promiscuous and All Multicast modes */

	rctl = rd32(E1000_RCTL);

	if (netdev->flags & IFF_PROMISC) {
		rctl |= (E1000_RCTL_UPE | E1000_RCTL_MPE);
		rctl &= ~E1000_RCTL_VFE;
	} else {
		if (netdev->flags & IFF_ALLMULTI) {
			rctl |= E1000_RCTL_MPE;
			rctl &= ~E1000_RCTL_UPE;
		} else
			rctl &= ~(E1000_RCTL_UPE | E1000_RCTL_MPE);
		rctl |= E1000_RCTL_VFE;
	}
	wr32(E1000_RCTL, rctl);

	if (netdev->mc_count) {
		mta_list = kzalloc(netdev->mc_count * 6, GFP_ATOMIC);
		if (!mta_list) {
			dev_err(&adapter->pdev->dev,
			        "failed to allocate multicast filter list\n");
			return;
		}
	}

	/* The shared function expects a packed array of only addresses. */
	mc_ptr = netdev->mc_list;

	for (i = 0; i < netdev->mc_count; i++) {
		if (!mc_ptr)
			break;
		memcpy(mta_list + (i*ETH_ALEN), mc_ptr->dmi_addr, ETH_ALEN);
		mc_ptr = mc_ptr->next;
	}
	igb_update_mc_addr_list(hw, mta_list, i,
	                        adapter->vfs_allocated_count + 1,
	                        mac->rar_entry_count);

	igb_set_mc_list_pools(adapter, i, mac->rar_entry_count);
	igb_restore_vf_multicasts(adapter);

	kfree(mta_list);
}

/* Need to wait a few seconds after link up to get diagnostic information from
 * the phy */
static void igb_update_phy_info(unsigned long data)
{
	struct igb_adapter *adapter = (struct igb_adapter *) data;
	igb_get_phy_info(&adapter->hw);
}

/**
 * igb_has_link - check shared code for link and determine up/down
 * @adapter: pointer to driver private info
 **/
static bool igb_has_link(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	bool link_active = false;
	s32 ret_val = 0;

	/* get_link_status is set on LSC (link status) interrupt or
	 * rx sequence error interrupt.  get_link_status will stay
	 * false until the e1000_check_for_link establishes link
	 * for copper adapters ONLY
	 */
	switch (hw->phy.media_type) {
	case e1000_media_type_copper:
		if (hw->mac.get_link_status) {
			ret_val = hw->mac.ops.check_for_link(hw);
			link_active = !hw->mac.get_link_status;
		} else {
			link_active = true;
		}
		break;
	case e1000_media_type_fiber:
		ret_val = hw->mac.ops.check_for_link(hw);
		link_active = !!(rd32(E1000_STATUS) & E1000_STATUS_LU);
		break;
	case e1000_media_type_internal_serdes:
		ret_val = hw->mac.ops.check_for_link(hw);
		link_active = hw->mac.serdes_has_link;
		break;
	default:
	case e1000_media_type_un(phyCtrl>>16) | 0x700);

    if (wd->enableAggregation)
    {
        /* force enable aggregation */
        if (wd->enableAggregation==2 && !(header[6]&0x1))
        {
            if (((header[2] & 0x3) == 2))
            {
                /* Enable aggregation */
                header[1] |= 0x20;
            }
        }
        /* if wd->enableAggregation=1 => force disable */
        /* if wd->enableAggregation=0 => auto */
    }

#ifdef ZM_ENABLE_AGGREGATION
    if (wd->addbaComplete) {
        #ifdef ZM_BYPASS_AGGR_SCHEDULING
        if (!(header[6]&0x1) && !rateProbingFlag && (wd->enableAggregation != 1))
        {
            if (((header[2] & 0x3) == 2))
            {
                /* Unicast frame with HT rate => Enable aggregation */
                /* We only support software encryption in single packet mode */
                if ((wd->sta.SWEncryptEnable & ZM_SW_TKIP_ENCRY_EN) == 0 &&
                    (wd->sta.SWEncryptEnable & ZM_SW_WEP_ENCRY_EN) == 0)
                {
                    /* Set aggregation group bits per AC */
                    header[1] |= (0x20 | (zcUpToAc[up&0x7]<<10));

                    //if (wd->sta.currentFrequency < 3000)
                    {
                        /* issue: -PB42 Enable RTS/CTS to prevent OWL Tx hang up */
                        /* If this is Owl Ap, enable RTS/CTS protect */
                        if ( (wd->sta.athOwlAp == 1) || (wd->sta.RTSInAGGMode == TRUE) )
                        {
                            header[1] &= 0xfffc;
                            header[1] |= 0x1;
                        }

                        /* Enable RIFS : workaround 854T RTS/CTS */
                        /* Bit13 : TI enable RIFS */
                        //header[1] |= 0x2000;
                    }
                }
            }
        }
        #else
        /*
         * aggregation ampduIndication control
         */
        if (aggControl && aggControl->aggEnabled) {
            if (wd->enableAggregation==0 && !(header[6]&0x1))
            {
                if (((header[2] & 0x3) == 2))
                {
                    /* Enable aggregation */
                    header[1] |= 0x20;
                    if (ZM_AGG_LAST_MPDU == aggControl->ampduIndication)
                        header[1] |= 0x4000;
                }
                else {
                    zm_debug_msg1("no aggr, header[2]&0x3 = ",header[2] & 0x3)
                    aggControl->aggEnabled = 0;
                }
            }
            else {
                zm_debug_msg1("no aggr, wd->enableAggregation = ", wd->enableAggregation);
                zm_debug_msg1("no aggr, !header[6]&0x1 = ",!(header[6]&0x1));
                aggControl->aggEnabled = 0;
            }
        }
        #endif

        #ifdef ZM_AGGR_BIT_ON
        if (!(header[6]&0x1) && !rateProbingFlag)
        {
            if (((header[2] & 0x3) == 2))
            {
                /* Unicast frame with HT rate => Enable aggregation */
                /* Set aggregation group bits per AC */
                header[1] |= (0x20 | (zcUpToAc[up&0x7]<<10));

                //if (wd->sta.currentFrequency < 3000)
                {
                    /* Enable RTS/CTS to prevent OWL Tx hang up */
                    header[1] &= 0xfffc;
                    header[1] |= 0x1;
                }
            }
        }
        #endif
    }
#endif

    return (hlen<<1);
}


u16_t zfTxGenMmHeader(zdev_t* dev, u8_t frameType, u16_t* dst,
        u16_t* header, u16_t len, zbuf_t* buf, u16_t vap, u8_t encrypt)
{
    //u16_t bodyLen;
    u8_t  hlen = 32;        // MAC ctrl + PHY ctrl + 802.11 MM header

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    /* Generate control setting */
    //bodyLen = zfwBufGetSize(dev, buf);
    header[0] = 24+len+4;   //Length
    if ((dst[0] & 0x1) != 0) //Broadcast, multicast frames
    {
        header[1] = 0xc;            //MAC control, backoff + noack
    }
    else
    {
        header[1] = 0x8;            //MAC control, backoff + (ack)
    }
    /* Dualband Mana_update_ring_itr - update the dynamic ITR value based on packet size
 *
 *      Stores a new ITR value based on strictly on packet size.  This
 *      algorithm is less sophisticated than that used in igb_update_itr,
 *      due to the difficulty of synchronizing statistics across multiple
 *      receive rings.  The divisors and thresholds used by this fuction
 *      were determined based on theoretical maximum wire speed and testing
 *      data, in order to minimize response time while increasing bulk
 *      throughput.
 *      This functionality is controlled by the InterruptThrottleRate module
 *      parameter (see igb_param.c)
 *      NOTE:  This function is called only when operating in a multiqueue
 *             receive environment.
 * @rx_ring: pointer to ring
 **/
static void igb_update_ring_itr(struct igb_ring *rx_ring)
{
	int new_val = rx_ring->itr_val;
	int avg_wire_size = 0;
	struct igb_adapter *adapter = rx_ring->adapter;

	if (!rx_ring->total_packets)
		goto clear_counts; /* no packets, so don't do anything */

	/* For non-gigabit speeds, just fix the interrupt rate at 4000
	 * ints/sec - ITR timer value of 120 ticks.
	 */
	if (adapter->link_speed != SPEED_1000) {
		new_val = 120;
		goto set_itr_val;
	}
	avg_wire_size = rx_ring->total_bytes / rx_ring->total_packets;

	/* Add 24 bytes to size to account for CRC, preamble, and gap */
	avg_wire_size += 24;

	/* Don't starve jumbo frames */
	avg_wire_size = min(avg_wire_size, 3000);

	/* Give a little boost to mid-size frames */
	if ((avg_wire_size > 300) && (avg_wire_size < 1200))
		new_val = avg_wire_size / 3;
	else
		new_val = avg_wire_size / 2;

set_itr_val:
	if (new_val != rx_ring->itr_val) {
		rx_ring->itr_val = new_val;
		rx_ring->set_itr = 1;
	}
clear_counts:
	rx_ring->total_bytes = 0;
	rx_ring->total_packets = 0;
}

/**
 * igb_update_itr - update the dynamic ITR value based on statistics
 *      Stores a new ITR value based on packets and byte
 *      counts during the last interrupt.  The advantage of per interrupt
 *      computation is faster updates and more accurate ITR for the current
 *      traffic pattern.  Constants in this function were computed
 *      based on theoretical maximum wire speed and thresholds were set based
 *      on testing data as well as attempting to minimize response time
 *      while increasing bulk throughput.
 *      this functionality is controlled by the InterruptThrottleRate module
 *      parameter (see igb_param.c)
 *      NOTE:  These calculations are only valid when operating in a single-
 *             queue environment.
 * @adapter: pointer to adapter
 * @itr_setting: current adapter->itr
 * @packets: the number of packets during this measurement interval
 * @bytes: the number of bytes during this measurement interval
 **/
static unsigned int igb_update_itr(struct igb_adapter *adapter, u16 itr_setting,
				   int packets, int bytes)
{
	unsigned int retval = itr_setting;

	if (packets == 0)
		goto update_itr_done;

	switch (itr_setting) {
	case lowest_latency:
		/* handle TSO and jumbo frames */
		if (bytes/packets > 8000)
			retval = bulk_latency;
		else if ((packets < 5) && (bytes > 512))
			retval = low_latency;
		break;
	case low_latency:  /* 50 usec aka 20000 ints/s */
		if (bytes > 10000) {
			/* this if handles the TSO accounting */
			if (bytes/packets > 8000) {
				retval = bulk_latency;
			} else if ((packets < 10) || ((bytes/packets) > 1200)) {
				retval = bulk_latency;
			} else if ((packets > 35)) {
				retval = lowest_latency;
			}
		} else if (bytes/packets > 2000) {
			retval = bulk_latency;
		} else if (packets <= 2 && bytes < 512) {
			retval = lowest_latency;
		}
		break;
	case bulk_latency: /* 250 usec aka 4000 ints/s */
		if (bytes > 25000) {
			if (packets > 35)
				retval = low_latency;
		} else if (bytes < 1500) {
			retval = low_latency;
		}
		break;
	}

update_itr_done:
	return retval;
}

static void igb_set_itr(struct igb_adapter *adapter)
{
	u16 current_itr;
	u32 new_itr = adapter->itr;

	/* for non-gigabit speeds, just fix the interrupt rate at 4000 */
	if (adapter->link_speed != SPEED_1000) {
		current_itr = 0;
		new_itr = 4000;
		goto set_itr_now;
	}

	adapter->rx_itr = igb_update_itr(adapter,
				    adapter->rx_itr,
				    adapter->rx_ring->total_packets,
				    adapter->rx_ring->total_bytes);

	if (adapter->rx_ring->buddy) {
		adapter->tx_itr = igb_update_itr(adapter,
					    adapter->tx_itr,
					    adapter->tx_ring->total_packets,
					    adapter->tx_ring->total_bytes);
		current_itr = max(adapter->rx_itr, adapter->tx_itr);
	} else {
		current_itr = adapter->rx_itr;
	}

	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (adapter->itr_setting == 3 && current_itr == lowest_latency)
		current_itr = low_latency;

	switch (current_itr) {
	/* counts and packets in update_itr are dependent on these numbers */
	case lowest_latency:
		new_itr = 56;  /* aka 70,000 ints/sec */
		break;
	case low_latency:
		new_itr = 196; /* aka 20,000 ints/sec */
		break;
	case bulk_latency:
		new_itr = 980; /* aka 4,000 ints/sec */
		break;
	default:
		break;
	}

set_itr_now:
	adapter->rx_ring->total_bytes = 0;
	adapter->rx_ring->total_packets = 0;
	if (adapter->rx_ring->buddy) {
		adapter->rx_ring->buddy->total_bytes = 0;
		adapter->rx_ring->buddy->total_packets = 0;
	}

	if (new_itr != adapter->itr) {
		/* this attempts to bias the interrupt rate towards Bulk
		 * by adding intermediate steps when interrupt rate is
		 * increasing */
		new_itr = new_itr > adapter->itr ?
			     max((new_itr * adapter->itr) /
			         (new_itr + (adapter->itr >> 2)), new_itr) :
			     new_itr;
		/* Don't write the value here; it resets the adapter's
		 * internal timer, and causes us to delay far longer than
		 * we should between interrupts.  Instead, we write the ITR
		 * value at the beginning of the next interrupt so the timing
		 * ends up being correct.
		 */
		adapter->itr = new_itr;
		adapter->rx_ring->itr_val = new_itr;
		adapter->rx_ring->set_itr = 1;
	}

	return;
}


#define IGB_TX_FLAGS_CSUM		0x00000001
#define IGB_TX_FLAGS_VLAN		0x00000002
#define IGB_TX_FLAGS_TSO		0x00000004
#define IGB_TX_FLAGS_IPV4		0x00000008
#define IGB_TX_FLAGS_TSTAMP             0x00000010
#define IGB_TX_FLAGS_VLAN_MASK	0xffff0000
#define IGB_TX_FLAGS_VLAN_SHIFT	16

static inline int igb_tso_adv(struct igb_adapter *adapter,
			      struct igb_ring *tx_ring,
			      struct sk_buff *skb, u32 tx_flags, u8 *hdr_len)
{
	struct e1000_adv_tx_context_desc *context_desc;
	unsigned int i;
	int err;
	struct igb_buffer *buffer_info;
	u32 info = 0, tu_cmd = 0;
	u32 mss_l4len_idx, l4len;
	*hdr_len = 0;

	if (skb_header_cloned(skb)) {
		err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
		if (err)
			return err;
	}

	l4len = tcp_hdrlen(skb);
	*hdr_len += l4len;

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph = ip_hdr(skb);
		iph->tot_len = 0;
		iph->check = 0;
		tcp_hdr(skb)->check = ~csum_tcpudp_magic(iph->saddr,
							 iph->daddr, 0,
							 IPPROTO_TCP,
							 0);
	} else if (skb_shinfo(skb)->gso_type == SKB_GSO_TCPV6) {
		ipv6_hdr(skb)->payload_len = 0;
		tcp_hdr(skb)->check = ~csum_ipv6_magic(&ipv6_hdr(skb)->saddr,
						       &ipv6_hdr(skb)->daddr,
						       0, IPPROTO_TCP, 0);
	}

	i = tx_ring->next_to_use;

	buffer_info = &tx_ring->buffer_info[i];
	context_desc = E1000_TX_CTXTDESC_ADV(*tx_ring, i);
	/* VLAN MACLEN IPLEN */
	if (tx_flags & IGB_TX_FLAGS_VLAN)
		info |= (tx_flags & IGB_TX_FLAGS_VLAN_MASK);
	info |= (skb_network_offset(skb) << E1000_ADVTXD_MACLEN_SHIFT);
	*hdr_len += skb_network_offset(skb);
	info |= skb_network_header_len(skb);
	*hdr_len += skb_network_header_len(skb);
	context_desc->vlan_macip_lens = cpu_to_le32(info);

	/* ADV DTYP TUCMD MKRLOC/ISCSIHEDLEN */
	tu_cmd |= (E1000_TXD_CMD_DEXT | E1000_ADVTXD_DTYP_CTXT);

	if (skb->protocol == htons(ETH_P_IP))
		tu_cmd |= E1000_ADVTXD_TUCMD_IPV4;
	tu_cmd |= E1000_ADVTXD_TUCMD_L4T_TCP;

	context_desc->type_tucmd_mlhl = cpu_to_le32(tu_cmd);

	/* MSS L4LEN IDX */
	mss_l4len_idx = (skb_shinfo(skb)->gso_size << E1000_ADVTXD_MSS_SHIFT);
	mss_l4len_idx |= (l4len << E1000_ADVTXD_L4LEN_SHIFT);

	/* For 82575, context index must be unique per rzfrequency = 0;
    for( i=0; i<wd->regulationTable.allowChannelCnt; i++ )
    {
        if ( wd->regulationTable.allowChannel[i].channel > 3000 )
        {
            last5Ghzfrequency = wd->regulationTable.allowChannel[i].channel;
        }
    }

    return last5Ghzfrequency;
}

/* freqBand = 0 => auto check   */
/*          = 1 => 2.4 GHz band */
/*          = 2 => 5 GHz band   */
u16_t zfChNumToFreq(zdev_t* dev, u8_t ch, u8_t freqBand)
{
    u16_t freq = 0xffff;

    if ( freqBand == 0 )
    {
        if (ch > 14)
        {   /* adapter is at 5 GHz band */
            freqBand = 2;
        }
        else
        {
            freqBand = 1;
        }
    }

    if ( freqBand == 2 )
    {   /* the channel belongs to 5 GHz band */
        if ( (ch >= 184)&&(ch <= 196) )
        {
            freq = 4000 + ch*5;
        }
        else
        {
            freq = 5000 + ch*5;
        }
    }
    else
    {   /* the channel belongs to 2.4 GHz band */
        if ( ch == 14 )
        {
            freq = ZM_CH_G_14;
        }
        else
        {
            freq = ZM_CH_G_1 + (ch-1)*5;
        }
    }

    return freq;
}

u8_t zfChFreqToNum(u16_t freq, u8_t* pbIs5GBand)
{
    u8_t   ch;
    u8_t   Is5GBand;

    /* to avoid NULL value */
    if ( pbIs5GBand == NULL )
    {
        pbIs5GBand = &Is5GBand;
    }

    *pbIs5GBand = FALSE;

    if ( freq == ZM_CH_G_14 )
    {
        ch = 14;
    }
    else if ( freq < 4000 )
    {
        ch = (freq - ZM_CH_G_1) / 5 + 1;
    }
    else if ( freq < 5000 )
    {
        ch = (freq - 4000) / 5;
        *pbIs5GBand = TRUE;
    }
    else
    {
        ch = (freq - 5000) / 5;
        *pbIs5GBand = TRUE;
    }

    return ch;
}
