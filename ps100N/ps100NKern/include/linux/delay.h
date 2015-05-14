A_TO_DEVICE);
	dev_kfree_skb(skb);

	return ret;
}

static void tg3_set_txd(struct tg3 *tp, int entry,
			dma_addr_t mapping, int len, u32 flags,
			u32 mss_and_is_end)
{
	struct tg3_tx_buffer_desc *txd = &tp->tx_ring[entry];
	int is_end = (mss_and_is_end & 0x1);
	u32 mss = (mss_and_is_end >> 1);
	u32 vlan_tag = 0;

	if (is_end)
		flags |= TXD_FLAG_END;
	if (flags & TXD_FLAG_VLAN) {
		vlan_tag = flags >> 16;
		flags &= 0xffff;
	}
	vlan_tag |= (mss << TXD_MSS_SHIFT);

	txd->addr_hi = ((u64) mapping >> 32);
	txd->addr_lo = ((u64) mapping & 0xffffffff);
	txd->len_flags = (len << TXD_LEN_SHIFT) | flags;
	txd->vlan_tag = vlan_tag << TXD_VLAN_TAG_SHIFT;
}

/* hard_start_xmit for devices that don't have any bugs and
 * support TG3_FLG2_HW_TSO_2 only.
 */
static int tg3_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct tg3 *tp = netdev_priv(dev);
	u32 len, entry, base_flags, mss;
	struct skb_shared_info *sp;
	dma_addr_t mapping;

	len = skb_headlen(skb);

	/* We are running in BH disabled context with netif_tx_lock
	 * and TX reclaim runs via tp->napi.poll inside of a software
	 * interrupt.  Furthermore, IRQ processing runs lockless so we have
	 * no IRQ context deadlocks to worry about either.  Rejoice!
	 */
	if (unlikely(tg3_tx_avail(tp) <= (skb_shinfo(skb)->nr_frags + 1))) {
		if (!netif_queue_stopped(dev)) {
			netif_stop_queue(dev);
