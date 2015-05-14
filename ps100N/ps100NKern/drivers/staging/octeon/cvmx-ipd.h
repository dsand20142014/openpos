nsigned short rx_cfg;
	int i;

	if (bp->sleeping)
		return;

	XXDEBUG(("bmac: enter bmac_set_multicast, n_addrs=%d\n", num_addrs));

	if((dev->flags & IFF_ALLMULTI) || (dev->mc_count > 64)) {
		for (i=0; i<4; i++) bp->hash_table_mask[i] = 0xffff;
		bmac_update_hash_table_mask(dev, bp);
		rx_cfg = bmac_rx_on(dev, 1, 0);
		XXDEBUG(("bmac: all multi, rx_cfg=%#08x\n"));
	} else if ((dev->flags & IFF_PROMISC) || (num_addrs < 0)) {
		rx_cfg = bmread(dev, RXCFG);
		rx_cfg |= RxPromiscEnable;
		bmwrite(dev, RXCFG, rx_cfg);
		rx_cfg = bmac_rx_on(dev, 0, 1);
		XXDEBUG(("bmac: promisc mode enabled, rx_cfg=%#08x\n", rx_cfg));
	} else {
		for (i=0; i<4; i++) bp->hash_table_mask[i] = 0;
		for (i=0; i<64; i++) bp->hash_use_count[i] = 0;
		if (num_addrs == 0) {
			rx_cfg = bmac_rx_on(dev, 0, 0);
			XXDEBUG(("bmac: multi disabled, rx_cfg=%#08x\n", rx_cfg));
		} else {
			for (dmi=dev->mc_list; dmi!=NULL; dmi=dmi->next)
				bmac_addhash(bp, dmi->dmi_addr);
			bmac_update_hash_table_mask(dev, bp);
			rx_cfg = bmac_rx_on(dev, 1, 0);
			XXDEBUG(("bmac: multi enabled, rx_cfg=%#08x\n", rx_cfg));
		}
	}
	/* XXDEBUG(("bmac: exit bmac_set_multicast\n")); */
}
#else /* ifdef SUNHME_MULTICAST */

/* The version of set_multicast below was lifted from sunhme.c */

static void bmac_set_multicast(struct net_device *dev)
{
	struct dev_mc_list *dmi = dev->mc_list;
	char *addrs;
	int i;
	unsigned short rx_cfg;
	u32 crc;

	if((dev->flags & IFF_ALLMULTI) || (dev->mc_count > 64)) {
		bmwrite(dev, BHASH0, 0xffff);
		bmwrite(dev, BHASH1, 0xffff);
		bmwrite(dev, BHASH2, 0xffff);
		bmwrite(dev, BHASH3, 0xffff);
	} else if(dev->flags & IFF_PROMISC) {
		rx_cfg = bmread(dev, RXCFG);
		rx_cfg |= RxPromiscEnable;
		bmwrite(dev, RXCFG, rx_cfg);
	} else {
		u16 hash_table[4];

		rx_cfg = bmread(dev, RXCFG);
		rx_cfg &= ~RxPromiscEnable;
		bmwrite(dev, RXCFG, rx_cfg);

		for(i = 0; i < 4; i++) hash_table[i] = 0;

		for(i = 0; i < dev->mc_count; i++) {
			addrs = dmi->dmi_addr;
			dmi = dmi->next;

			if(!(*addrs & 1))
				continue;

			crc = ether_crc_le(6, addrs);
			crc >>= 26;
			hash_table[crc >> 4] |= 1 << (crc & 0xf);
		}
		bmwrite(dev, BHASH0, hash_table[0]);
		bmwrite(dev, BHASH1, hash_table[1]);
		bmwrite(dev, BHASH2, hash_table[2]);
		bmwrite(dev, BHASH3, hash_table[3]);
	}
}
#endif /* SUNHME_MULTICAST */

static int miscintcount;

static irqreturn_t bmac_misc_intr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	unsigned int status = bmread(dev, STATUS);
	if (miscintcount++ < 10) {
		XXDEBUG(("bmac_misc_intr\n"));
	}
	/* XXDEBUG(("bmac_misc_intr, status=%#08x\n", status)); */
	/*     bmac_txdma_intr_inner(irq, dev_id); */
	/*   if (status & FrameReceived) dev->stats.rx_dropped++; */
	if (status & RxErrorMask) dev->stats.rx_errors++;
	if (status & RxCRCCntExp) dev->stats.rx_crc_errors++;
	if (status & RxLenCntExp) dev->stats.rx_length_errors++;
	if (status & RxOverFlow) dev->stats.rx_over_errors++;
	if (status & RxAlignCntExp) dev->stats.rx_frame_errors++;

	/*   if (status & FrameSent) dev->stats.tx_dropped++; */
	if (status & TxErrorMask) dev->stats.tx_errors++;
	if (status & TxUnderrun) dev->stats.tx_fifo_errors++;
	if (status & TxNormalCollExp) dev->stats.collisions++;
	return IRQ_HANDLED;
}

/*
 * Procedure for reading EEPROM
 */
#define SROMAddressLength	5
#define DataInOn		0x0008
#define DataInOff		0x0000
#define Clk			0x0002
#define ChipSelect		0x0001
#define SDIShiftCount		3
#define SD0ShiftCount		2
#define	DelayValue		1000	/* number of microseconds */
#define SROMStartOffset		10	/* this is in words */
#define SROMReadCount		3	/* number of words to read from SROM */
#define SROMAddressBits		6
#define EnetAddressOffset	20

static unsigned char
bmac_clock_out_bit(struct net_device *dev)
{
	unsigned short         data;
	unsigned short         val;

	bmwrite(dev, SROMCSR, ChipSelect | Clk);
	udelay(DelayValue);

	data = bmread(dev, SROMCSR);
	udelay(DelayValue);
	val = (data >> SD0ShiftCount) & 1;

	bmwrite(dev, SROMCSR, ChipSelect);
	udelay(DelayValue);

	return val;
}

static void
bmac_clock_in_bit(struct net_device *dev, unsigned int val)
{
	unsigned short data;

	if (val != 0 && val != 1) return;

	data = (val << SDIShiftCount);
	bmwrite(dev, SROMCSR, data | ChipSelect  );
	udelay(DelayValue);

	bmwrite(dev, SROMCSR, data | ChipSelect | Clk );
	udelay(DelayValue);

	bmwrite(dev, SROMCSR, data | ChipSelect);
	udelay(DelayValue);
}

static void
reset_and_select_srom(struct net_device *dev)
{
	/* first reset */
	bmwrite(dev, SROMCSR, 0);
	udelay(DelayValue);

	/* send it the read command (110) */
	bmac_clock_in_bit(dev, 1);
	bmac_clock_in_bit(dev, 1);
	bmac_clock_in_bit(dev, 0);
}

static unsigned short
read_srom(struct net_device *dev, unsigned int addr, unsigned int addr_len)
{
	unsigned short data, val;
	int i;

	/* send out the address we want to read from */
	for (i = 0; i < addr_len; i++)	{
		val = addr >> (addr_len-i-1);
		bmac_clock_in_bit(dev, val & 1);
	}

	/* Now read in the 16-bit data */
	data = 0;
	for (i = 0; i < 16; i++)	{
		val = bmac_clock_out_bit(dev);
		data <<= 1;
		data |= val;
	}
	bmwrite(dev, SROMCSR, 0);

	return data;
}

/*
 * It looks like Cogent and SMC use different methods for calculating
 * checksums. What a pain..
 */

static int
bmac_verify_checksum(struct net_device *dev)
{
	unsigned short data, storedCS;

	reset_and_select_srom(dev);
	data = read_srom(dev, 3, SROMAddressBits);
	storedCS = ((data >> 8) & 0x0ff) | ((data << 8) & 0xff00);

	return 0;
}


static void
bmac_get_station_address(struct net_device *dev, unsigned char *ea)
{
	int i;
	unsigned short data;

	for (i = 0; i < 6; i++)
		{
			reset_and_select_srom(dev);
			data = read_srom(dev, i + EnetAddressOffset/2, SROMAddressBits);
			ea[2*i]   = bitrev8(data & 0x0ff);
			ea[2*i+1] = bitrev8((data >> 8) & 0x0ff);
		}
}

static void bmac_reset_and_enable(struct net_device *dev)
{
	struct bmac_data *bp = netdev_priv(dev);
	unsigned long flags;
	struct sk_buff *skb;
	unsigned char *data;

	spin_lock_irqsave(&bp->lock, flags);
	bmac_enable_and_reset_chip(dev);
	bmac_init_tx_ring(bp);
	bmac_init_rx_ring(bp);
	bmac_init_chip(dev);
	bmac_start_chip(dev);
	bmwrite(dev, INTDISABLE, EnableNormal);
	bp->sleeping = 0;

	/*
	 * It seems that the bmac can't receive until it's transmitted
	 * a packet.  So we give it a dummy packet to transmit.
	 */
	skb = dev_alloc_skb(ETHERMINPACKET);
	if (skb != NULL) {
		data = skb_put(skb, ETHERMINPACKET);
		memset(data, 0, ETHERMINPACKET);
		memcpy(data, dev->dev_addr, 6);
		memcpy(data+6, dev->dev_addr, 6);
		bmac_transmit_packet(skb, dev);
	}
	spin_unlock_irqrestore(&bp->lock, flags);
}
static void bmac_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct bmac_data *bp = netdev_priv(dev);
	strcpy(info->driver, "bmac");
	strcpy(info->bus_info, dev_name(&bp->mdev->ofdev.dev));
}

static const struct ethtool_ops bmac_ethtool_ops = {
	.get_drvinfo		= bmac_get_drvinfo,
	.get_link		= ethtool_op_get_link,
};

static const struct net_device_ops bmac_netdev_ops = {
	.ndo_open		= bmac_open,
	.ndo_stop		= bmac_close,
	.ndo_start_xmit		= bmac_output,
	.ndo_set_multicast_list	= bmac_set_multicast,
	.ndo_set_mac_address	= bmac_set_address,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
};

static int __devinit bmac_probe(struct macio_dev *mdev, const struct of_device_id *match)
{
	int j, rev, ret;
	struct bmac_data *bp;
	const unsigned char *prop_addr;
	unsigned char addr[6];
	struct net_device *dev;
	int is_bmac_plus = ((int)match->data) != 0;

	if (macio_resource_count(mdev) != 3 || macio_irq_count(mdev) != 3) {
		printk(KERN_ERR "BMAC: can't use, need 3 addrs and 3 intrs\n");
		return -ENODEV;
	}
	prop_addr = of_get_property(macio_get_of_node(mdev),
			"mac-address", NULL);
	if (prop_addr == NULL) {
		prop_addr = of_get_property(macio_get_of_node(mdev),
				"local-mac-address", NULL);
		if (prop_addr == NULL) {
			printk(KERN_ERR "BMAC: Can't get mac-address\n");
			return -ENODEV;
		}
	}
	memcpy(addr, prop_addr, sizeof(addr));

	dev = alloc_etherdev(PRIV_BYTES);
	if (!dev) {
		printk(KERN_ERR "BMAC: alloc_etherdev failed, out of memory\n");
		return -ENOMEM;
	}

	bp = netdev_priv(dev);
	SET_NETDEV_DEV(dev, &mdev->ofdev.dev);
	macio_set_drvdata(mdev, dev);

	bp->mdev = mdev;
	spin_lock_init(&bp->lock);

	if (macio_request_resources(mdev, "bmac")) {
		printk(KERN_ERR "BMAC: can't request IO resource !\n");
		goto out_free;
	}

	dev->base_addr = (unsigned long)
		ioremap(macio_resource_start(mdev, 0), macio_resource_len(mdev, 0));
	if (dev->base_addr == 0)
		goto out_release;

	dev->irq = macio_irq(mdev, 0);

	bmac_enable_and_reset_chip(dev);
	bmwrite(dev, INTDISABLE, DisableAll);

	rev = addr[0] == 0 && addr[1] == 0xA0;
	for (j = 0; j < 6; ++j)
		dev->dev_addr[j] = rev ? bitrev8(addr[j]): addr[j];

	/* Enable chip without interrupts for now */
	bmac_enable_and_reset_chip(dev);
	bmwrite(dev, INTDISABLE, DisableAll);

	dev->netdev_ops = &bmac_netdev_ops;
	dev->ethtool_ops = &bmac_ethtool_ops;

	bmac_get_station_address(dev, addr);
	if (bmac_verify_checksum(dev) != 0)
		goto err_out_iounmap;

	bp->is_bmac_plus = is_bmac_plus;
	bp->tx_dma = ioremap(macio_resource_start(mdev, 1), macio_resource_len(mdev, 1));
	if (!bp->tx_dma)
		goto err_out_iounmap;
	bp->tx_dma_intr = macio_irq(mdev, 1);
	bp->rx_dma = ioremap(macio_resource_start(mdev, 2), macio_resource_len(mdev, 2));
	if (!bp->rx_dma)
		goto err_out_iounmap_tx;
	bp->rx_dma_intr = macio_irq(mdev, 2);

	bp->tx_cmds = (volatile struct dbdma_cmd *) DBDMA_ALIGN(bp + 1);
	bp->rx_cmds = bp->tx_cmds + N_TX_RING + 1;

	bp->queue = (struct sk_buff_head *)(bp->rx_cmds + N_RX_RING + 1);
	skb_queue_head_init(bp->queue);

	init_timer(&bp->tx_timeout);

	ret = request_irq(dev->irq, bmac_misc_intr, 0, "BMAC-misc", dev);
	if (ret) {
		printk(KERN_ERR "BMAC: can't get irq %d\n", dev->irq);
		goto err_out_iounmap_rx;
	}
	ret = request_irq(bp->tx_dma_intr, bmac_txdma_intr, 0, "BMAC-txdma", dev);
	if (ret) {
		printk(KERN_ERR "BMAC: can't get irq %d\n", bp->tx_dma_intr);
		goto err_out_irq0;
	}
	ret = request_irq(bp->rx_dma_intr, bmac_rxdma_intr, 0, "BMAC-rxdma", dev);
	if (ret) {
		printk(KERN_ERR "BMAC: can't get irq %d\n", bp->rx_dma_intr);
		goto err_out_irq1;
	}

	/* Mask chip interrupts and disable chip, will be
	 * re-enabled on open()
	 */
	disable_irq(dev->irq);
	pmac_call_feature(PMAC_FTR_BMAC_ENABLE, macio_get_of_node(bp->mdev), 0, 0);

	if (register_netdev(dev) != 0) {
		printk(KERN_ERR "BMAC: Ethernet registration failed\n");
		goto err_out_irq2;
	}

	printk(KERN_INFO "%s: BMAC%s at %pM",
	       dev->name, (is_bmac_plus ? "+" : ""), dev->dev_addr);
	XXDEBUG((", base_addr=%#0lx", dev->base_addr));
	printk("\n");

	return 0;

err_out_irq2:
	free_irq(bp->rx_dma_i