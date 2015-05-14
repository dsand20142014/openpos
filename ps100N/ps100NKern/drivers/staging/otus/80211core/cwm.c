complete = 1;

	/* Workaround for Realtek RTL8201 PHY issue */
	if ((phy->phy_id0 == 0x0000) && ((phy->phy_id1 & 0xFFF0) == 0x8200)) {
		if (mdio_read(net_dev, phy_addr, MII_CONTROL) & MII_CNTL_FDX)
			*duplex = FDX_CAPABLE_FULL_SELECTED;
		if (mdio_read(net_dev, phy_addr, 0x0019) & 0x01)
			*speed = HW_SPEED_100_MBPS;
	}

	if(netif_msg_link(sis_priv))
		printk(KERN_INFO "%s: Media Link On %s %s-duplex \n",
	       				net_dev->name,
	       				*speed == HW_SPEED_100_MBPS ?
	       					"100mbps" : "10mbps",
	       				*duplex == FDX_CAPABLE_FULL_SELECTED ?
	       					"full" : "half");
}

/**
 *	sis900_tx_timeout - sis900 transmit timeout routine
 *	@net_dev: the net device to transmit
 *
 *	print transmit timeout status
 *	disable interrupts and do some tasks
 */

static void sis900_tx_timeout(struct net_device *net_dev)
{
	struct sis900_private *sis_priv = netdev_priv(net_dev);
	long ioaddr = net_dev->base_addr;
	unsigned long flags;
	int i;

	if(netif_msg_tx_err(sis_priv))
		printk(KERN_INFO "%s: Transmit timeout, status %8.8x %8.8x \n",
	       		net_dev->name, inl(ioaddr + cr), inl(ioaddr + isr));

	/* Disable interrupts by clearing the interrupt mask. */
	outl(0x0000, ioaddr + imr);

	/* use spinlock to prevent interrupt handler accessing buffer ring */
	spin_lock_irqsave(&sis_priv->lock, flags);

	/* discard unsent packets */
	sis_priv->dirty_tx = sis_priv->cur_tx = 0;
	for (i = 0; i < NUM_TX_DESC; i++) {
		struct sk_buff *skb = sis_priv->tx_skbuff[i];

		if (skb) {
			pci_unmap_single(sis_priv->pci_dev,
				sis_priv->tx_ring[i].bufptr, skb->len,
				PCI_DMA_TODEVICE);
			dev_kfree_skb_irq(skb);
			sis_priv->tx_skbuff[i] = NULL;
			sis_priv->tx_ring[i].cmdsts = 0;
			sis_priv->tx_ring[i].bufptr = 0;
			net_dev->stats.tx_dropped++;
		}
	}
	sis_priv->tx_full = 0;
	netif_wake_queue(net_dev);

	spin_unlock_irqrestore(&sis_priv->lock, flags);

	net_dev->trans_start = jiffies;

	/* load Transmit Descriptor Register */
	outl(sis_priv->tx_ring_dma, ioaddr + txdp);

	/* Enable all known interrupts by setting the interrupt mask. */
	outl((RxSOVR|RxORN|RxERR|RxOK|TxURN|TxERR|TxIDLE), ioaddr + imr);
	return;
}

/**
 *	sis900_start_xmit - sis900 start transmit routine
 *	@skb: socket buffer pointer to put the data being transmitted
 *	@net_dev: the net device to transmit with
 *
 *	Set the transmit buffer descriptor,
 *	and write TxENA to enable transmit state machine.
 *	tell upper layer if the buffer is full
 */

static int
sis900_start_xmit(struct sk_buff *skb, struct net_device *net_dev)
{
	struct sis900_private *sis_priv = netdev_priv(net_dev);
	long ioaddr = net_dev->base_addr;
	unsigned int  entry;
	unsigned long flags;
	unsigned int  index_cur_tx, index_dirty_tx;
	unsigned int  count_dirty_tx;

	/* Don't transmit data before the complete of auto-negotiation */
	if(!sis_priv->autong_complete){
		netif_stop_queue(net_dev);
		return NETDEV_TX_BUSY;
	}

	spin_lock_irqsave(&sis_priv->lock, flags);

	/* Calculate the next Tx descriptor entry. */
	entry = sis_priv->cur_tx % NUM_TX_DESC;
	sis_priv->tx_skbuff[entry] = skb;

	/* set the transmit buffer descriptor and enable Transmit State Machine */
	sis_priv->tx_ring[entry].bufptr = pci_map_single(sis_priv->pci_dev,
		skb->data, skb->len, PCI_DMA_TODEVICE);
	sis_priv->tx_ring[entry].cmdsts = (OWN | skb->len);
	outl(TxENA | inl(ioaddr + cr), ioaddr + cr);

	sis_priv->cur_tx ++;
	index_cur_tx = sis_priv->cur_tx;
	index_dirty_tx = sis_priv->dirty_tx;

	for (count_dirty_tx = 0; index_cur_tx != index_dirty_tx; index_dirty_tx++)
		count_dirty_tx ++;

	if (index_cur_tx == index_dirty_tx) {
		/* dirty_tx is met in the cycle of cur_tx, buffer full */
		sis_priv->tx_full = 1;
		netif_stop_queue(net_dev);
	} else if (count_dirty_tx < NUM_TX_DESC) {
		/* Typical path, tell upper lay