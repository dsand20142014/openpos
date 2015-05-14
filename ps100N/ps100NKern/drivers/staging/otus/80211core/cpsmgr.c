BLE |
		     (((ssb_clockspeed(sdev->bus) + (B44_MDC_RATIO / 2)) / B44_MDC_RATIO)
		     & MDIO_CTRL_MAXF_MASK)));
		break;
	case SSB_BUSTYPE_PCI:
	case SSB_BUSTYPE_PCMCIA:
		bw32(bp, B44_MDIO_CTRL, (MDIO_CTRL_PREAMBLE |
		     (0x0d & MDIO_CTRL_MAXF_MASK)));
		break;
	}

	br32(bp, B44_MDIO_CTRL);

	if (!(br32(bp, B44_DEVCTRL) & DEVCTRL_IPP)) {
		bw32(bp, B44_ENET_CTRL, ENET_CTRL_EPSEL);
		br32(bp, B44_ENET_CTRL);
		bp->flags &= ~B44_FLAG_INTERNAL_PHY;
	} else {
		u32 val = br32(bp, B44_DEVCTRL);

		if (val & DEVCTRL_EPR) {
			bw32(bp, B44_DEVCTRL, (val & ~DEVCTRL_EPR));
			br32(bp, B44_DEVCTRL);
			udelay(100);
		}
		bp->flags |= B44_FLAG_INTERNAL_PHY;
	}
}

/* bp->lock is held. */
static void b44_halt(struct b44 *bp)
{
	b44_disable_ints(bp);
	/* reset PHY */
	b44_phy_reset(bp);
	/* power down PHY */
	printk(KERN_INFO PFX "%s: powering down PHY\n", bp->dev->name);
	bw32(bp, B44_MAC_CTRL, MAC_CTRL_PHY_PDOWN);
	/* now reset the chip, but without enabling the MAC&PHY
	 * part of it. This has to be done _after_ we shut down the PHY */
	b44_chip_reset(bp, B44_CHIP_RESET_PARTIAL);
}

/* bp->lock is held. */
static void __b44_set_mac_addr(struct b44 *bp)
{
	bw32(bp, B44_CAM_CTRL, 0);
	if (!(bp->dev->flags & IFF_PROMISC)) {
		u32 val;

		__b44_cam_write(bp, bp->dev->dev_addr, 0);
		val = br32(bp, B44_CAM_CTRL);
		bw32(bp, B44_CAM_CTRL, val | CAM_CTRL_ENABLE);
	}
}

static int b44_set_mac_addr(struct net_device *dev, void *p)
{
	struct b44 *bp = netdev_priv(dev);
	struct sockaddr *addr = p;
	u32 val;

	if (netif_running(dev))
		return -EBUSY;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EINVAL;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	spin_lock_irq(&bp->lock);

	val = br32(bp, B44_RXCONFIG);
	if (!(val & RXCONFIG_CAM_ABSENT))
		__b44_set_mac_addr(bp);

	spin_unlock_irq(&bp->lock);

	return 0;
}

/* Called at device open time to get the chip ready for
 * packet processing.  Invoked with bp->lock held.
 */
static void __b44_set_rx_mode(struct net_device *);
static void b44_init_hw(struct b44 *bp, int reset_kind)
{
	u32 val;

	b44_chip_reset(bp, B44_CHIP_RESET_FULL);
	if (reset_kind == B44_FULL_RESET) {
		b44_phy_reset(bp);
		b44_setup_phy(bp);
	}

	/* Enable CRC32, set proper LED modes and power on PHY */
	bw32(bp, B44_MAC_CTRL, MAC_CTRL_CRC32_ENAB | MAC_CTRL_PHY_LEDCTRL);
	bw32(bp, B44_RCV_LAZY, (1 << RCV_LAZY_FC_SHIFT));

	/* This sets the MAC address too.  */
	__b44_set_rx_mode(bp->dev);

	/* MTU + eth header + possible VLAN tag + struct rx_header */
	bw32(bp, B44_RXMAXLEN, bp->dev->mtu + ETH_HLEN + 8 + RX_HEADER_LEN);
	bw32(bp, B44_TXMAXLEN, bp->dev->mtu + ETH_HLEN + 8 + RX_HEADER_LEN);

	bw32(bp, B44_TX_WMARK, 56); /* XXX magic */
	if (reset_kind == B44_PARTIAL_RESET) {
		bw32(bp, B44_DMARX_CTRL, (DMARX_CTRL_ENABLE |
				      (RX_PKT_OFFSET << DMARX_CTRL_ROSHIFT)));
	} else {
		bw32(bp, B44_DMATX_CTRL, DMATX_CTRL_ENABLE);
		bw32(bp, B44_DMATX_ADDR, bp->tx_ring_dma + bp->dma_offset);
		bw32(bp, B44_DMARX_CTRL, (DMARX_CTRL_ENABLE |
				      (RX_PKT_OFFSET << DMARX_CTRL_ROSHIFT)));
		bw32(bp, B44_DMARX_ADDR, bp->rx_ring_dma + bp->dma_offset);

		bw32(bp, B44_DMARX_PTR, bp->rx_pending);
		bp->rx_prod = bp->rx_pending;

		bw32(bp, B44_MIB_CTRL, MIB_CTRL_CLR_ON_READ);
	}

	val = br32(bp, B44_ENET_CTRL);
	bw32(bp, B44_ENET_CTRL, (val | ENET_CTRL_ENABLE));
}

static int b44_open(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);
	int err;

	err = b44_alloc_consistent(bp, GFP_KERNEL);
	if (err)
		goto out;

	napi_enable(&bp->napi);

	b44_init_rings(bp);
	b44_init_hw(bp, B44_FULL_RESET);

	b44_check_phy(bp);

	err = request_irq(dev->irq, b44_interrupt, IRQF_SHARED, dev->name, dev);
	if (unlikely(err < 0)) {
		napi_disable(&bp->napi);
		b44_chip_reset(bp, B44_CHIP_RESET_PARTIAL);
		b44_free_rings(bp);
		b44_free_consistent(bp);
		goto out;
	}

	init_timer(&bp->timer);
	bp->timer.expires = jiffies + HZ;
	bp->timer.data = (unsigned long) bp;
	bp->timer.function = b44_timer;
	add_timer(&bp->timer);

	b44_enable_ints(bp);
	netif_start_queue(dev);
out:
	return err;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling receive - used by netconsole and other diagnostic tools
 * to allow network i/o with interrupts disabled.
 */
static void b44_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
	b44_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif

static void bwfilter_table(struct b44 *bp, u8 *pp, u32 bytes, u32 table_offset)
{
	u32 i;
	u32 *pattern = (u32 *) pp;

	for (i = 0; i < bytes; i += sizeof(u32)) {
		bw32(bp, B44_FILT_ADDR, table_offset + i);
		bw32(bp, B44_FILT_DATA, pattern[i / sizeof(u32)]);
	}
}

static int b44_magic_pattern(u8 *macaddr, u8 *ppattern, u8 *pmask, int offset)
{
	int magicsync = 6;
	int k, j, len = offset;
	int ethaddr_bytes = ETH_ALEN;

	memset(ppattern + offset, 0xff, magicsync);
	for (j = 0; j < magicsync; j++)
		set_bit(len++, (unsigned long *) pmask);

	for (j = 0; j < B44_MAX_PATTERNS; j++) {
		if ((B44_PATTERN_SIZE - len) >= ETH_ALEN)
			ethaddr_bytes = ETH_ALEN;
		else
			ethaddr_bytes = B44_PATTERN_SIZE - len;
		if (ethaddr_bytes <=0)
			break;
		for (k = 0; k< ethaddr_bytes; k++) {
			ppattern[offset + magicsync +
				(j * ETH_ALEN) + k] = macaddr[k];
			len++;
			set_bit(len, (unsigned long *) pmask);
		}
	}
	return len - 1;
}

/* Setup magic packet patterns in the b44 WOL
 * pattern matching filter.
 */
static void b44_setup_pseudo_magicp(struct b44 *bp)
{

	u32 val;
	int plen0, plen1, plen2;
	u8 *pwol_pattern;
	u8 pwol_mask[B44_PMASK_SIZE];

	pwol_pattern = kzalloc(B44_PATTERN_SIZE, GFP_KERNEL);
	if (!pwol_pattern) {
		printk(KERN_ERR PFX "Memory not available for WOL\n");
		return;
	}

	/* Ipv4 magic packet pattern - pattern 0.*/
	memset(pwol_mask, 0, B44_PMASK_SIZE);
	plen0 = b44_magic_pattern(bp->dev->dev_addr, pwol_pattern, pwol_mask,
				  B44_ETHIPV4UDP_HLEN);

   	bwfilter_table(bp, pwol_pattern, B44_PATTERN_SIZE, B44_PATTERN_BASE);
   	bwfilter_table(bp, pwol_mask, B44_PMASK_SIZE, B44_PMASK_BASE);

	/* Raw ethernet II magic packet pattern - pattern 1 */
	memset(pwol_pattern, 0, B44_PATTERN_SIZE);
	memset(pwol_mask, 0, B44_PMASK_SIZE);
	plen1 = b44_magic_pattern(bp->dev->dev_addr, pwol_pattern, pwol_mask,
				  ETH_HLEN);

   	bwfilter_table(bp, pwol_pattern, B44_PATTERN_SIZE,
		       B44_PATTERN_BASE + B44_PATTERN_SIZE);
  	bwfilter_table(bp, pwol_mask, B44_PMASK_SIZE,
		       B44_PMASK_BASE + B44_PMASK_SIZE);

	/* Ipv6 magic packet pattern - pattern 2 */
	memset(pwol_pattern, 0, B44_PATTERN_SIZE);
	memset(pwol_mask, 0, B44_PMASK_SIZE);
	plen2 = b44_magic_pattern(bp->dev->dev_addr, pwol_pattern, pwol_mask,
				  B44_ETHIPV6UDP_HLEN);

   	bwfilter_table(bp, pwol_pattern, B44_PATTERN_SIZE,
		       B44_PATTERN_BASE + B44_PATTERN_SIZE + B44_PATTERN_SIZE);
  	bwfilter_table(bp, pwol_mask, B44_PMASK_SIZE,
		       B44_PMASK_BASE + B44_PMASK_SIZE + B44_PMASK_SIZE);

	kfree(pwol_pattern);

	/* set these pattern's lengths: one less than each real length */
	val = plen0 | (plen1 << 8) | (plen2 << 16) | WKUP_LEN_ENABLE_THREE;
	bw32(bp, B44_WKUP_LEN, val);

	/* enable wakeup pattern matching */
	val = br32(bp, B44_DEVCTRL);
	bw32(bp, B44_DEVCTRL, val | DEVCTRL_PFE);

}

#ifdef CONFIG_B44_PCI
static void b44_setup_wol_pci(struct b44 *bp)
{
	u16 val;

	if (bp->sdev->bus->bustype != SSB_BUSTYPE_SSB) {
		bw32(bp, SSB_TMSLOW, br32(bp, SSB_TMSLOW) | SSB_TMSLOW_PE);
		pci_read_config_word(bp->sdev->bus->host_pci, SSB_PMCSR, &val);
		pci_write_config_word(bp->sdev->bus->host_pci, SSB_PMCSR, val | SSB_PE);
	}
}
#else
static inline void b44_setup_wol_pci(struct b44 *bp) { }
#endif /* CONFIG_B44_PCI */

static void b44_setup_wol(struct b44 *bp)
{
	u32 val;

	bw32(bp, B44_RXCONFIG, RXCONFIG_ALLMULTI);

	if (bp->flags & B44_FLAG_B0_ANDLATER) {

		bw32(bp, B44_WKUP_LEN, WKUP_LEN_DISABLE);

		val = bp->dev->dev_addr[2] << 24 |
			bp->dev->dev_addr[3] << 16 |
			bp->dev->dev_addr[4] << 8 |
			bp->dev->dev_addr[5];
		bw32(bp, B44_ADDR_LO, val);

		val = bp->dev->dev_addr[0] << 8 |
			bp->dev->dev_addr[1];
		bw32(bp, B44_ADDR_HI, val);

		val = br32(bp, B44_DEVCTRL);
		bw32(bp, B44_DEVCTRL, val | DEVCTRL_MPM | DEVCTRL_PFE);

 	} else {
 		b44_setup_pseudo_magicp(bp);
 	}
	b44_setup_wol_pci(bp);
}

static int b44_close(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);

	netif_stop_queue(dev);

	napi_disable(&bp->napi);

	del_timer_sync(&bp->timer);

	spin_lock_irq(&bp->lock);

	b44_halt(bp);
	b44_free_rings(bp);
	netif_carrier_off(dev);

	spin_unlock_irq(&bp->lock);

	free_irq(dev->irq, dev);

	if (bp->flags & B44_FLAG_WOL_ENABLE) {
		b44_init_hw(bp, B44_PARTIAL_RESET);
		b44_setup_wol(bp);
	}

	b44_free_consistent(bp);

	return 0;
}

static struct net_device_stats *b44_get_stats(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);
	struct net_device_stats *nstat = &dev->stats;
	struct b44_hw_stats *hwstat = &bp->hw_stats;

	/* Convert HW stats into netdevice stats. */
	nstat->rx_packets = hwstat->rx_pkts;
	nstat->tx_packets = hwstat->tx_pkts;
	nstat->rx_bytes   = hwstat->rx_octets;
	nstat->tx_bytes   = hwstat->tx_octets;
	nstat->tx_errors  = (hwstat->tx_jabber_pkts +
			     hwstat->tx_oversize_pkts +
			     hwstat->tx_underruns +
			     hwstat->tx_excessive_cols +
			     hwstat->tx_late_cols);
	nstat->multicast  = hwstat->tx_multicast_pkts;
	nstat->collisions = hwstat->tx_total_cols;

	nstat->rx_length_errors = (hwstat->rx_oversize_pkts +
				   hwstat->rx_undersize);
	nstat->rx_over_errors   = hwstat->rx_missed_pkts;
	nstat->rx_frame_errors  = hwstat->rx_align_errs;
	nstat->rx_crc_errors    = hwstat->rx_crc_errs;
	nstat->rx_errors        = (hwstat->rx_jabber_pkts +
				   hwstat->rx_oversize_pkts +
				   hwstat->rx_missed_pkts +
				   hwstat->rx_crc_align_errs +
				   hwstat->rx_undersize +
				   hwstat->rx_crc_errs +
				   hwstat->rx_align_errs +
				   hwstat->rx_symbol_errs);

	nstat->tx_aborted_errors = hwstat->tx_underruns;
#if 0
	/* Carrier lost counter seems to be broken for some devices */
	nstat->tx_carrier_errors = hwstat->tx_carrier_lost;
#endif

	return nstat;
}

static int __b44_load_mcast(struct b44 *bp, struct net_device *dev)
{
	struct dev_mc_list *mclist;
	int i, num_ents;

	num_ents = min_t(int, dev->mc_count, B44_MCAST_TABLE_SIZE);
	mclist = dev->mc_list;
	for (i = 0; mclist && i < num_ents; i++, mclist = mclist->next) {
		__b44_cam_write(bp, mclist->dmi_addr, i + 1);
	}
	return i+1;
}

static void __b44_set_rx_mode(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);
	u32 val;

	val = br32(bp, B44_RXCONFIG);
	val &= ~(RXCONFIG_PROMISC | RXCONFIG_ALLMULTI);
	if ((dev->flags & IFF_PROMISC) || (val & RXCONFIG_CAM_ABSENT)) {
		val |= RXCONFIG_PROMISC;
		bw32(bp, B44_RXCONFIG, val);
	} else {
		unsigned char zero[6] = {0, 0, 0, 0, 0, 0};
		int i = 1;

		__b44_set_mac_addr(bp);

		if ((dev->flags & IFF_ALLMULTI) ||
		    (dev->mc_count > B44_MCAST_TABLE_SIZE))
			val |= RXCONFIG_ALLMULTI;
		else
			i = __b44_load_mcast(bp, dev);

		for (; i < 64; i++)
			__b44_cam_write(bp, zero, i);

		bw32(bp, B44_RXCONFIG, val);
        	val = br32(bp, B44_CAM_CTRL);
	        bw32(bp, B44_CAM_CTRL, val | CAM_CTRL_ENABLE);
	}
}

static void b44_set_rx_mode(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);

	spin_lock_irq(&bp->lock);
	__b44_set_rx_mode(dev);
	spin_unlock_irq(&bp->lock);
}

static u32 b44_get_msglevel(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);
	return bp->msg_enable;
}

static void b44_set_msglevel(struct net_device *dev, u32 value)
{
	struct b44 *bp = netdev_priv(dev);
	bp->msg_enable = value;
}

static void b44_get_drvinfo (struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct b44 *bp = netdev_priv(dev);
	struct ssb_bus *bus = bp->sdev->bus;

	strncpy(info->driver, DRV_MODULE_NAME, sizeof(info->driver));
	strncpy(info->version, DRV_MODULE_VERSION, sizeof(info->driver));
	switch (bus->bustype) {
	case SSB_BUSTYPE_PCI:
		strncpy(info->bus_info, pci_name(bus->host_pci), sizeof(info->bus_info));
		break;
	case SSB_BUSTYPE_PCMCIA:
	case SSB_BUSTYPE_SSB:
		strncpy(info->bus_info, "SSB", sizeof(info->bus_info));
		break;
	}
}

static int b44_nway_reset(struct net_device *dev)
{
	struct b44 *bp = netdev_priv(dev);
	u32 bmcr;
	int r;

	spin_lock_irq(&bp->lock);
	b44_readphy(bp, MII_BMCR, &bmcr);
	b44_readphy(bp, MII_BMCR, &bmcr);
	r = -EINVAL;
	if (bmcr & BMCR_ANENABLE) {
		b44_writephy(bp, MII_BMCR,
			     bmcr | BMCR_ANRESTART);
		r = 0;
	}
	spin_unlock_irq(&bp->lock);

	return r;
}

static int b44_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct b44 *bp = netdev_priv(dev);

	cmd->supported = (SUPPORTED_Autoneg);
	cmd->supported |= (SUPPORTED_100baseT_Half |
			  SUPPORTED_100baseT_Full |
			  SUPPORTED_10baseT_Half |
			  SUPPORTED_10baseT_Full |
			  SUPPORTED_MII);

	cmd->advertising = 0;
	if (bp->flags & B44_FLAG_ADV_10HALF)
		cmd->advertising |= ADVERTISED_10baseT_Half;
	if (bp->flags & B44_FLAG_ADV_10FULL)
		cmd->advertising |= ADVERTISED_10baseT_Full;
	if (bp->flags & B44_FLAG_ADV_100HALF)
		cmd->advertising |= ADVERTISED_100baseT_Half;
	if (bp->flags & B44_FLAG_ADV_100FULL)
		cmd->advertising |= ADVERTISED_100baseT_Full;
	cmd->advertising |= ADVERTISED_Pause | ADVERTISED_Asym_Pause;
	cmd->speed = (bp->flags & B44_FLAG_100_BASE_T) ?
		SPEED_100 : SPEED_10;
	cmd->duplex = (bp->flags & B44_FLAG_FULL_DUPLEX) ?
		DUPLEX_FULL : DUPLEX_HALF;
	cmd->port = 0;
	cmd->phy_address = bp->phy_addr;
	cmd->transceiver = (bp->flags & B44_FLAG_INTERNAL_PHY) ?
		XCVR_INTERNAL : XCVR_EXTERNAL;
	cmd->autoneg = (bp->flags & B44_FLAG_FORCE_LINK) ?
		AUTONEG_DISABLE : AUTONEG_ENABLE;
	if (cmd->autoneg == AUTONEG_ENABLE)
		cmd->advertising |= ADVERTISED_Autoneg;
	if (!netif_running(dev)){
		cmd->speed = 0;
		cmd->duplex = 0xff;
	}
	cmd->maxtxpkt = 0;
	cmd->maxrxpkt = 0;
	return 0;
}

static int b44_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct b44 *bp = netdev_priv(dev);

	/* We do not support gigabit. */
	if (cmd->autoneg == AUTONEG_ENABLE) {
		if (cmd->advertising &
		    (ADVERTISED_1000baseT_Half |
		     ADVERTISED_1000baseT_Full))
			return -EINVAL;
	} else if ((cmd->speed != SPEED_100 &&
		    cmd->speed != SPEED_10) ||
		   (cmd->duplex != DUPLEX_HALF &&
		    cmd->duplex != DUPLEX_FULL)) {
			return -EINVAL;
	}

	spin_lock_irq(&bp->lock);

	if (cmd->autoneg == AUTONEG_ENABLE) {
		bp->flags &= ~(B44_FLAG_FORCE_LINK |
			       B44_FLAG_100_BASE_T |
			       B44_FLAG_FULL_DUPLEX |
			       B44_FLAG_ADV_10HALF |
			       B44_FLAG_ADV_10FULL |
			       B44_FLAG_ADV_100HALF |
			       B44_FLAG_ADV_100FULL);
		if (cmd->advertising == 0) {
			bp->flags |= (B44_FLAG_ADV_10HALF |
				      B44_FLAG_ADV_10FULL |
				      B44_FLAG_ADV_100HALF |
				      B44_FLAG_ADV_100FULL);
		} else {
			if (cmd->advertising & ADVERTISED_10baseT_Half)
				bp->flags |= B44_FLAG_ADV_10HALF;
			if (cmd->advertising & ADVERTISED_10baseT_Full)
				bp->flags |= B44_FLAG_ADV_10FULL;
			if (cmd->advertising & ADVERTISED_100baseT_Half)
				bp->flags |= B44_FLAG_ADV_100HALF;
			if (cmd->advertising & ADVERTISED_100baseT_Full)
				bp->flags |= B44_FLAG_ADV_100FULL;
		}
	} else {
		bp->flags |= B44_FLAG_FORCE_LINK;
		bp->flags &= ~(B44_FLAG_100_BASE_T | B44_FLAG_FULL_DUPLEX);
		if (cmd->speed == SPEED_100)
			bp->flags |= B44_FLAG_100_BASE_T;
		if (cmd->duplex == DUPLEX_FULL)
			bp->flags |= B44_FLAG_FULL_DUPLEX;
	}

	if (netif_running(dev))
		b44_setup_phy(bp);

	spin_unlock_irq(&bp->lock);

	return 0;
}

static void b44_get_ringparam(struct net_device *dev,
			      struct ethtool_ringparam *ering)
{
	struct b44 *bp = netdev_priv(dev);

	ering->rx_max_pending = B44_RX_RING_SIZE - 1;
	ering->rx_pending = bp->rx_pending;

	/* XXX ethtool lacks a tx_max_pending, oops... */
}

static int b44_set_ringparam(struct net_device *dev,
			     struct ethtool_ringparam *ering)
{
	struct b44 *bp = netdev_priv(dev);

	if ((ering->rx_pending > B44_RX_RING_SIZE - 1) ||
	    (ering->rx_mini_pending != 0) ||
	    (ering->rx_jumbo_pending != 0) ||
	    (ering->tx_pending > B44_TX_RING_SIZE - 1))
		return -EINVAL;

	spin_lock_irq(&bp->lock);

	bp->rx_pending = ering->rx_pending;
	bp->tx_pending = ering->tx_pending;

	b44_halt(bp);
	b44_init_rings(bp);
	b44_init_hw(bp, B44_FULL_RESET);
	netif_wake_queue(bp->dev);
	spin_unlock_irq(&bp->lock);

	b44_enable_ints(bp);

	return 0;
}

static void b44_get_pauseparam(struct net_device *dev,
				struct ethtool_pauseparam *epause)
{
	struct b44 *bp = netdev_priv(dev);

	epause->autoneg =
		(bp->flags & B44_FLAG_PAUSE_AUTO) != 0;
	epause->rx_pause =
		(bp->flags & B44_FLAG_RX_PAUSE) != 0;
	epause->tx_pause =
		(bp->flags & B44_FLAG_TX_PAUSE) != 0;
}

static int b44_set_pauseparam(struct net_device *dev,
				struct ethtool_pauseparam *epause)
{
	struct b44 *bp = netdev_priv(dev);

	spin_lock_irq(&bp->lock);
	if (epause->autoneg)
		bp->flags |= B44_FLAG_PAUSE_AUTO;
	else
		bp->flags &= ~B44_FLAG_PAUSE_AUTO;
	if (epause->rx_pause)
		bp->flags |= B44_FLAG_RX_PAUSE;
	else
		bp->flags &= ~B44_FLAG_RX_PAUSE;
	if (epause->tx_pause)
		bp->flags |= B44_FLAG_TX_PAUSE;
	else
		bp->flags &= ~B44_FLAG_TX_PAUSE;
	if (bp->flags & B44_FLAG_PAUSE_AUTO) {
		b44_halt(bp);
		b44_init_rings(bp);
		b44_init_hw(bp, B44_FULL_RESET);
	} else {
		__b44_set_flow_ctrl(bp, bp->flags);
	}
	spin_unlock_irq(&bp->lock);

	b44_enable_ints(bp);

	return 0;
}

static void b44_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	switch(stringset) {
	case ETH_SS_STATS:
		memcpy(data, *b44_gstrings, sizeof(b44_gstrings));
		break;
	}
}

static int b44_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(b44_gstrings);
	default:
		return -EOPNOTSUPP;
	}
}

static void b44_get_ethtool_stats(struct net_device *dev,
				  struct ethtool_stats *stats, u64 *data)
{
	struct b44 *bp = netdev_priv(dev);
	u32 *val = &bp->hw_stats.tx_good_octets;
	u32 i;

	spin_lock_irq(&bp->lock);

	b44_stats_update(bp);

	for (i = 0; i < ARRAY_SIZE(b44_gstrings); i++)
		*data++ = *val++;

	spin_unlock_irq(&bp->lock);
}

static void b44_get_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	struct b44 *bp = netdev_priv(dev);

	wol->supported = WAKE_MAGIC;
	if (bp->flags & B44_FLAG_WOL_ENABLE)
		wol->wolopts = WAKE_MAGIC;
	else
		wol->wolopts = 0;
	memset(&wol->sopass, 0, sizeof(wol->sopass));
}

static int b44_set_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	struct b44 *bp = netdev_priv(dev);

	spin_lock_irq(&bp->lock);
	if (wol->wolopts & WAKE_MAGIC)
		bp->flags |= B44