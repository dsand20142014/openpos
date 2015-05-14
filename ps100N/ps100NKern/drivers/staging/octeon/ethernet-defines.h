nt; i++) {
			eq_value = (0x00F8 & mdio_read(net_dev,
					sis_priv->cur_phy, MII_RESV)) >> 3;
			if (i == 0)
				max_value=min_value=eq_value;
			max_value = (eq_value > max_value) ?
						eq_value : max_value;
			min_value = (eq_value < min_value) ?
						eq_value : min_value;
		}
		/* 630E rule to determine the equalizer value */
		if (revision == SIS630E_900_REV || revision == SIS630EA1_900_REV ||
		    revision == SIS630ET_900_REV) {
			if (max_value < 5)
				eq_value = max_value;
			else if (max_value >= 5 && max_value < 15)
				eq_value = (max_value == min_value) ?
						max_value+2 : max_value+1;
			else if (max_value >= 15)
				eq_value=(max_value == min_value) ?
						max_value+6 : max_value+5;
		}
		/* 630B0&B1 rule to determine the equalizer value */
		if (revision == SIS630A_900_REV &&
		    (sis_priv->host_bridge_rev == SIS630B0 ||
		     sis_priv->host_bridge_rev == SIS630B1)) {
			if (max_value == 0)
				eq_value = 3;
			else
				eq_value = (max_value + min_value + 1)/2;
		}
		/* write equalizer value and setting */
		reg14h = mdio_read(net_dev, sis_priv->cur_phy, MII_RESV);
		reg14h = (reg14h & 0xFF07) | ((eq_value << 3) & 0x00F8);
		reg14h = (reg14h | 0x6000) & 0xFDFF;
		mdio_write(net_dev, sis_priv->cur_phy, MII_RESV, reg14h);
	} else {
		reg14h = mdio_read(net_dev, sis_priv->cur_phy, MII_RESV);
		if (revision == SIS630A_900_REV &&
		    (sis_priv->host_bridge_rev == SIS630B0 ||
		     sis_priv->host_bridge_rev == SIS630B1))
			mdio_write(net_dev, sis_priv->cur_phy, MII_RESV,
						(reg14h | 0x2200) & 0xBFFF);
		else
			mdio_write(net_dev, sis_priv->cur_phy, MII_RESV,
						(reg14h | 0x2000) & 0xBFFF);
	}
	return;
}

/**
 *	sis900_timer - sis900 timer routine
 *	@data: pointer to sis900 net device
 *
 *	On each timer ticks we check two things,
 *	link status (ON/OFF) and link mode (10/100/Full/Half)
 */

static void sis900_timer(unsigned long data)
{
	struct net_device *net_dev = (struct net_device *)data;
	struct sis900_private *sis_priv = netdev_priv(net_dev);
	struct mii_phy *mii_phy = sis_priv->mii;
	static const int next_tick = 5*HZ;
	u16 status;

	if (!sis_priv->autong_complete){
		int uninitialized_var(speed), duplex = 0;

		sis900_read_mode(net_dev, &speed, &duplex);
		if (duplex){
			sis900_set_mode(net_dev->base_addr, speed, duplex);
			sis630_set_eq(net_dev, sis_priv->chipset_rev);
			netif_start_queue(net_dev);
		}

		sis_priv->timer.expires = jiffies + HZ;
		add_timer(&sis_priv->timer);
		return;
	}

	status = mdio_read(net_dev, sis_priv->cur_phy, MII_STATUS);
	status = mdio_read(net_dev, sis_priv->cur_phy, MII_STATUS);

	/* Link OFF -> ON */
	if (!netif_carrier_ok(net_dev)) {
	LookForLink:
		/* Search for new PHY */
		status = sis900_default_phy(net_dev);
		mii_phy = sis_priv->mii;

		if (status & MII_STAT_LINK){
			sis900_check_mode(net_dev, mii_phy);
			netif_carrier_on(net_dev);
		}
	} else {
	/* Link ON -> OFF */
                if (!(status & MII_STAT_LINK)){
                	netif_carrier_off(net_dev);
			if(netif_msg_link(sis_priv))
                		printk(KERN_INFO "%s: Media Link Off\n", net_dev->name);

                	/* Change mode issue */
                	if ((mii_phy->phy_id0 == 0x001D) &&
			    ((mii_phy->phy_id1 & 0xFFF0) == 0x8000))
               			sis900_reset_phy(net_dev,  sis_priv->cur_phy);

			sis630_set_eq(net_dev, sis_priv->chipset_rev);

                	goto LookForLink;
                }
	}

	sis_priv->timer.expires = jiffies + next_tick;
	add_timer(&sis_priv->timer);
}

/**
 *	sis900_check_mode - check the media mode for sis900
 *	@net_dev: the net device to be checked
 *	@mii_phy: the mii phy
 *
 *	Older driver gets the media mode from mii status output
 *	register. Now we set our media capability and auto-negotiate
 *	to get the upper bound of speed and duplex between two ends.
 *	If the types of mii phy is HOME, it doesn't need to auto-negotiate
 *	and autong_complete should be set to 1.
 */

static void sis900_check_mode(struct net_device *net_dev, struct mii_phy *mii_phy)
{
	struct sis900_private *sis_priv = netdev_priv(net_dev);
	long ioaddr = net_dev->base_addr;
	int speed, duplex;

	if (mii_phy->phy_types == LAN) {
		outl(~EXD & inl(ioaddr + cfg), ioaddr + cfg);
		sis900_set_capability(net_dev , mii_phy);
		sis900_auto_negotiate(net_dev, sis_priv->cur_phy);
	} else {
		outl(EXD | inl(ioaddr + cfg), ioaddr + cfg);
		speed = HW_SPEED_HOME;
		duplex = FDX_CAPABLE_HALF_SELECTED;
		sis900_set_mode(ioaddr, speed, duplex);
		sis_priv->autong_complete = 1;
	}
}

/**
 *	sis900_set_mode - Set the media mode of mac register.
 *	@ioaddr: the address of the device
 *	@speed : the transmit speed to be determined
 *	@duplex: the duplex mode to be determined
 *
 *	Set the media mode of mac register txcfg/rxcfg according to
 *	speed and duplex of phy. Bit EDB_MASTER_EN indicates the EDB
 *	bus is used instead of PCI bus. When this bit is set 1, the
 *	Max DMA Burst Size for TX/RX DMA should be no larger than 16
 *	double words.
 */

static void sis900_set_mode (long ioaddr, int speed, int duplex)
{
	u32 tx_flags = 0, rx_flags = 0;

	if (inl(ioaddr + cfg) & EDB_MASTER_EN) {
		tx_flags = TxATP | (