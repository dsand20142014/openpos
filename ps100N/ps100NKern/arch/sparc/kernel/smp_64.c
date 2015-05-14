, u32 *msgbuf, u32 vf)
{
	int add = (msgbuf[0] & E1000_VT_MSGINFO_MASK) >> E1000_VT_MSGINFO_SHIFT;
	int vid = (msgbuf[1] & E1000_VLVF_VLANID_MASK);

	return igb_vlvf_set(adapter, vid, add, vf);
}

static inline void igb_vf_reset_event(struct igb_adapter *adapter, u32 vf)
{
	struct e1000_hw *hw = &adapter->hw;

	/* disable mailbox functionality for vf */
	adapter->vf_data[vf].clear_to_send = false;

	/* reset offloads to defaults */
	igb_set_vmolr(hw, vf);

	/* reset vlans for device */
	igb_clear_vf_vfta(adapter, vf);

	/* reset multicast table array for vf */
	adapter->vf_data[vf].num_vf_mc_hashes = 0;

	/* Flush and reset the mta with the new values */
	igb_set_multi(adapter->netdev);
}

static inline void igb_vf_reset_msg(struct igb_adapter *adapter, u32 vf)
{
	struct e1000_hw *hw = &adapter->hw;
	unsigned char *vf_mac = adapter->vf_data[vf].vf_mac_addresses;
	u32 reg, msgbuf[3];
	u8 *addr = (u8 *)(&msgbuf[1]);

	/* process all the same items cleared in a function level reset */
	igb_vf_reset_event(adapter, vf);

	/* set vf mac address */
	igb_rar_set(hw, vf_mac, vf + 1);
	igb_set_rah_pool(hw, vf, vf + 1);

	/* enable transmit and receive for vf */
	reg = rd32(E1000_VFTE);
	wr32(E1000_VFTE, reg | (1 << vf));
	reg = rd32(E1000_VFRE);
	wr32(E1000_VFRE, reg | (1 << vf));

	/* enable mailbox functionality for vf */
	adapter->vf_data[vf].clear_to_send = true;

	/* reply to reset with ack and vf mac address */
	msgbuf[0] = E1000_VF_RESET | E1000_VT_MSGTYPE_ACK;
	memcpy(addr, vf_mac, 6);
	igb_write_mbx(hw, msgbuf, 3, vf);
}

static int igb_set_vf_mac_addr(struct igb_adapter *adapter, u32 *msg, int vf)
{
		unsigned char *addr = (char *)&msg[1];
		int err = -1;

		if (is_valid_ether_addr(addr))
			err = igb_set_vf_mac(adapter, vf, addr);

		return err;

}

static void igb_rcv_ack_from_vf(struct igb_adapter *adapter, u32 vf)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 msg = E1000_VT_MSGTYPE_NACK;

	/* if device isn't clear to send it shouldn't be reading either */
	if (!adapter->vf_data[vf].clear_to_send)
		igb_write_mbx(hw, &msg, 1, vf);
}


static void igb_msg_task(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	u32 vf;

	for (vf = 0; vf < adapter->vfs_allocated_count; vf++) {
		/* process any reset requests */
		if (!igb_check_for_rst(hw, vf)) {
			adapter->vf_data[vf].clear_to_send = false;
			igb_vf_reset_event(adapter, vf);
		}

		/* process any messages pending */
		if (!igb_check_for_msg(hw, vf))
			igb_rcv_msg_from_vf(adapter, vf);

		/* process any acks */
		if (!igb_check_for_ack(hw, vf))
			igb_rcv_ack_from_vf(adapter, vf);

	}
}

static int igb_rcv_msg_from_vf(struct igb_adapter *adapter, u32 vf)
{
	u32 mbx_size = E1000_VFMAILBOX_SIZE;
	u32 msgbuf[mbx_size];
	struct e1000_hw *hw = &adapter->hw;
	s32 retval;

	retval = igb_read_mbx(hw, msgbuf, mbx_size, vf);

	if (retval)
		dev_err(&adapter->pdev->dev,
		        "Error receiving message from VF\n");

	/* this is a message we already processed, do nothing */
	if (msgbuf[0] & (E1000_VT_MSGTYPE_ACK | E1000_VT_MSGTYPE_NACK))
		return retval;

	/*
	 * until the vf completes a reset it should not be
	 * allowed to start any configuration.
	 */

	if (msgbuf[0] == E1000_VF_RESET) {
		igb_vf_reset_msg(adapter, vf);

		return retval;
	}

	if (!adapter->vf_data[vf].clear_to_send) {
		msgbuf[0] |= E1000_VT_MSGTYPE_NACK;
		igb_write_mbx(hw, msgbuf, 1, vf);
		return retval;
	}

	switch ((msgbuf[0] & 0xFFFF)) {
	case E1000_VF_SET_MAC_ADDR:
		retval = igb_set_vf_mac_addr(adapter, msgbuf, vf);
		break;
	case E1000_VF_SET_MULTICAST:
		retval = igb_set_vf_multicasts(adapter, msgbuf, vf);
		break;
	case E1000_VF_SET_LPE:
		retval = igb_set_vf_rlpml(adapter, msgbuf[1], vf);
		break;
	case E1000_VF_SET_VLAN:
		retval = igb_set_vf_vlan(adapter, msgbuf, vf);
		break;
	default:
		dev_err(&adapter->pdev->dev, "Unhandled Msg %08x\n", msgbuf[0]);
		retval = -1;
		break;
	}

	/* notify the VF of the results of what it sent us */
	if (retval)
		msgbuf[0] |= E1000_VT_MSGTYPE_NACK;
	else
		msgbuf[0] |= E1000_VT_MSGTYPE_ACK;

	msgbuf[0] |= E1000_VT_MSGTYPE_CTS;

	igb_write_mbx(hw, msgbuf, 1, vf);

	return retval;
}

/**
 * igb_intr_msi - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t igb_intr_msi(int irq, void *data)
{
	struct net_device *netdev = data;
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	/* read ICR disables interrupts using IAM */
	u32 icr = rd32(E1000_ICR);

	igb_write_itr(adapter->rx_ring);

	if(icr & E1000_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & (E1000_ICR_RXSEQ | E1000_ICR_LSC)) {
		hw->mac.get_link_status = 1;
		if (!test_bit(__IGB_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	napi_schedule(&adapter->rx_ring[0].napi);

	return IRQ_HANDLED;
}

/**
 * igb_intr - Legacy Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t igb_intr(int irq, void *data)
{
	struct net_device *netdev = data;
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	/* Interrupt Auto-Mask...upon reading ICR, interrupts are masked.  No
	 * need for the IMC write */
	u32 icr = rd32(E1000_ICR);
	if (!icr)
		return IRQ_NONE;  /* Not our interrupt */

	igb_write_itr(adapter->rx_ring);

	/* IMS will not auto-mask if INT_ASSERTED is not set, and if it is
	 * not set, then the adapter didn't send an interrupt */
	if (!(icr & E1000_ICR_INT_ASSERTED))
		return IRQ_NONE;

	if(icr & E1000_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & (E1000_ICR_RXSEQ | E1000_ICR_LSC)) {
		hw->mac.get_link_status = 1;
		/* guard against interrupt when we're going down */
		if (!test_bit(__IGB_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	napi_schedule(&adapter->rx_ring[0].napi);

	return IRQ_HANDLED;
}

static inline void igb_rx_irq_enable(struct igb_ring *rx_ring)
{
	struct igb_adapter *adapter = rx_ring->adapter;
	struct e1000_hw *hw = &adapter->hw;

	if (adapter->itr_setting & 3) {
		if (adapter->num_rx_queues == 1)
			igb_set_itr(adapter);
		else
			igb_update_ring_itr(rx_ring);
	}

	if (!test_bit(__IGB_DOWN, &adapter->state)) {
		if (adapter->msix_entries)
			wr32(E1000_EIMS, rx_ring->eims_value);
		else
			igb_irq_enable(adapter);
	}
}

/**
 * igb_poll - NAPI Rx polling callback
 * @napi: napi polling structure
 * @budget: count of how many packets we should handle
 **/
static int igb_poll(struct napi_struct *napi, int budget)
{
	struct igb_ring *rx_ring = container_of(napi, struct igb_ring, napi);
	int work_done = 0;

#ifdef CONFIG_IGB_DCA
	if (rx_ring->adapter->flags & IGB_FLAG_DCA_ENABLED)
		igb_update_rx_dca(rx_ring);
#endif
	igb_clean_rx_irq_adv(rx_ring, &work_done, budget);

	if (rx_ring->buddy) {
#ifdef CONFIG_IGB_DCA
		if (rx_ring->adapter->flags & IGB_FLAG_DCA_ENABLED)
			igb_update_tx_dca(rx_ring->buddy);
#endif
		if (!igb_clean_tx_irq(rx_ring->buddy))
			work_done = budget;
	}

	/* If not enough Rx work done, exit the polling mode */
	if (work_done < budget) {
		napi_complete(napi);
		igb_rx_irq_enable(rx_ring);
	}

	return work_done;
}

/**
 * igb_hwtstamp - utility function which checks for TX time stamp
 * @adapter: board private structure
 * @skb: packet that was just sent
 *
 * If we were asked to do hardware stamping and such a time stamp is
 * available, then it must have been for this skb here because we only
 * allow only one such packet into the queue.
 */
static void igb_tx_hwtstamp(struct igb_adapter *adapter, struct sk_buff *skb)
{
	union skb_shared_tx *shtx = skb_tx(skb);
	struct e1000_hw *hw = &adapter->hw;

	if (unlikely(shtx->hardware)) {
		u32 valid = rd32(E1000_TSYNCTXCTL) & E1000_TSYNCTXCTL_VALID;
		if (valid) {
			u64 regval = rd32(E1000_TXSTMPL);
			u64 ns;
			struct skb_shared_hwtstamps shhwtstamps;

			memset(&shhwtstamps, 0, sizeof(shhwtstamps));
			regval |= (u64)rd32(E1000_TXSTMPH) << 32;
			ns = timecounter_cyc2time(&adapter->clock,
						  regval);
			timecompare_update(&adapter->compare, ns);
			shhwtstamps.hwtstamp = ns_to_ktime(ns);
			shhwtstamps.syststamp =
				timecompare_transform(&adapter->compare, ns);
			skb_tstamp_tx(skb, &shhwtstamps);
		}
	}
}

/**
 * igb_clean_tx_irq - Reclaim resources after transmit completes
 * @adapter: board private structure
 * returns true if ring is completely cleaned
 **/
static bool igb_clean_tx_irq(struct igb_ring *tx_ring)
{
	struct igb_adapter *adapter = tx_ring->adapter;
	struct net_device *netdev = adapter->netdev;
	struct e1000_hw *hw = &adapter->hw;
	struct igb_buffer *buffer_info;
	struct sk_buff *skb;
	union e1000_adv_tx_desc *tx_desc, *eop_desc;
	unsigned int total_bytes = 0, total_packets = 0;
	unsigned int i, eop, count = 0;
	bool cleaned = false;

	i = tx_ring->next_to_clean;
	eop = tx_ring->buffer_info[i].next_to_watch;
	eop_desc = E1000_TX_DESC_ADV(*tx_ring, eop);

	while ((eop_desc->wb.status & cpu_to_le32(E1000_TXD_STAT_DD)) &&
	       (count < tx_ring->count)) {
		for (cleaned = false; !cleaned; count++) {
			tx_desc = E1000_TX_DESC_ADV(*tx_ring, i);
			buffer_info = &tx_ring->buffer_info[i];
			cleaned = (i == eop);
			skb = buffer_info->skb;

			if (skb) {
				unsigned int segs, bytecount;
				/* gso_segs is currently only valid for tcp */
				segs = skb_shinfo(skb)->gso_segs ?: 1;
				/* multiply data chunks by size of headers */
				bytecount = ((segs - 1) * skb_headlen(skb)) +
					    skb->len;
				total_packets += segs;
				total_bytes += bytecount;

				igb_tx_hwtstamp(adapter, skb);
			}

			igb_unmap_and_free_tx_resource(adapter, buffer_info);
			tx_desc->wb.status = 0;

			i++;
			if (i == tx_ring->count)
				i = 0;
		}
		eop = tx_ring->buffer_info[i].next_to_watch;
		eop_desc = E1000_TX_DESC_ADV(*tx_ring, eop);
	}

	tx_ring->next_to_clean = i;

	if (unlikely(count &&
		     netif_carrier_ok(netdev) &&
		     igb_desc_unused(tx_ring) >= IGB_TX_QUEUE_WAKE)) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		smp_mb();
		if (__netif_subqueue_stopped(netdev, tx_ring->queue_index) &&
		    !(test_bit(__IGB_DOWN, &adapter->state))) {
			netif_wake_subqueue(netdev, tx_ring->queue_index);
			++adapter->restart_queue;
		}
	}

	if (tx_ring->detect_tx_hung) {
		/* Detect a transmit hang in hardware, this serializes the
		 * check with the clearing of time_stamp and movement of i */
		tx_ring->detect_tx_hung = false;
		if (tx_ring->buffer_info[i].time_stamp &&
		    time_after(jiffies, tx_ring->buffer_info[i].time_stamp +
			       (adapter->tx_timeout_factor * HZ))
		    && !(rd32(E1000_STATUS) &
			 E1000_STATUS_TXOFF)) {

			/* detected Tx unit hang */
			dev_err(&adapter->pdev->dev,
				"Detected Tx Unit Hang\n"
				"  Tx Queue             <%d>\n"
				"  TDH                  <%x>\n"
				"  TDT                  <%x>\n"
				"  next_to_use          <%x>\n"
				"  next_to_clean        <%x>\n"
				"buffer_info[next_to_clean]\n"
				"  time_stamp           <%lx>\n"
				"  next_to_watch        <%x>\n"
				"  jiffies              <%lx>\n"
				"  desc.status          <%x>\n",
				tx_ring->queue_index,
				readl(adapter->hw.hw_addr + tx_ring->head),
				readl(adapter->hw.hw_addr + tx_ring->tail),
				tx_ring->next_to_use,
				tx_ring->next_to_clean,
				tx_ring->buffer_info[i].time_stamp,
				eop,
				jiffies,
				eop_desc->wb.status);
			netif_stop_subqueue(netdev, tx_ring->queue_index);
		}
	}
	tx_ring->total_bytes += total_bytes;
	tx_ring->total_packets += total_packets;
	tx_ring->tx_stats.bytes += total_bytes;
	tx_ring->tx_stats.packets += total_packets;
	adapter->net_stats.tx_bytes += total_bytes;
	adapter->net_stats.tx_packets += total_packets;
	return (count < tx_ring->count);
}

/**
 * igb_receive_skb - helper function to handle rx indications
 * @ring: pointer to receive ring receving this packet
 * @status: descriptor status field as written by hardware
 * @rx_desc: receive descriptor containing vlan and type information.
 * @skb: pointer to sk_buff to be indicated to stack
 **/
static void igb_receive_skb(struct igb_ring *ring, u8 status,
                            union e1000_add = 0;
	busy_mask = 0;
	{
		int i;

		for (i = 0; i < cnt; i++) {
			u64 target, nr;

			nr = cpu_list[i];
			if (nr == 0xffff)
				continue;

			target = (nr << 14) | 0x70;
			if (is_jbus) {
				busy_mask |= (0x1UL << (nr * 2));
			} else {
				target |= (nack_busy_id << 24);
				busy_mask |= (0x1UL <<
					      (nack_busy_id * 2));
			}
			__asm__ __volatile__(
				"stxa	%%g0, [%0] %1\n\t"
				"membar	#Sync\n\t"
				: /* no outputs */
				: "r" (target), "i" (ASI_INTR_W));
			nack_busy_id++;
			if (nack_busy_id == 32) {
				need_more = 1;
				break;
			}
		}
	}

	/* Now, poll for completion. */
	{
		u64 dispatch_stat, nack_mask;
		long stuck;

		stuck = 100000 * nack_busy_id;
		nack_mask = busy_mask << 1;
		do {
			__asm__ __volatile__("ldxa	[%%g0] %1, %0"
					     : "=r" (dispatch_stat)
					     : "i" (ASI_INTR_DISPATCH_STAT));
			if (!(dispatch_stat & (busy_mask | nack_mask))) {
				__asm__ __volatile__("wrpr %0, 0x0, %%pstate"
						     : : "r" (pstate));
				if (unlikely(need_more)) {
					int i, this_cnt = 0;
					for (i = 0; i < cnt; i++) {
						if (cpu_list[i] == 0xffff)
							continue;
						cpu_list[i] = 0xffff;
						this_cnt++;
						if (this_cnt == 32)
							break;
					}
					goto retry;
				}
				return;
			}
			if (!--stuck)
				break;
		} while (dispatch_stat & busy_mask);

		__asm__ __volatile__("wrpr %0, 0x0, %%pstate"
				     : : "r" (pstate));

		if (dispatch_stat & busy_mask) {
			/* Busy bits will not clear, continue instead
			 * of freezing up on this cpu.
			 */
			printk("CPU[%d]: mondo stuckage result[%016llx]\n",
			       smp_processor_id(), dispatch_stat);
		} else {
			int i, this_busy_nack = 0;

			/* Delay some random time with interrupts enabled
			 * to prevent deadlock.
			 */
			udelay(2 * nack_busy_id);

			/* Clear out the mask bits for cpus which did not
			 * NACK us.
			 */
			for (i = 0; i < cnt; i++) {
				u64 check_mask, nr;

				nr = cpu_list[i];
				if (nr == 0xffff)
					continue;

				if (is_jbus)
					check_mask = (0x2UL << (2*nr));
				else
					check_mask = (0x2UL <<
						      this_busy_nack);
				if ((dispatch_stat & check_mask) == 0)
					cpu_list[i] = 0xffff;
				this_busy_nack += 2;
				if (this_busy_nack == 64)
					break;
			}

			goto retry;
		}
	}
}

/* Multi-cpu list version.  */
static void hypervisor_xcall_deliver(struct trap_per_cpu *tb, int cnt)
{
	int retries, this_cpu, prev_sent, i, saw_cpu_error;
	unsigned long status;
	u16 *cpu_list;

	this_cpu = smp_processor_id();

	cpu_list = __va(tb->cpu_list_pa);

	saw_cpu_error = 0;
	retries = 0;
	prev_sent = 0;
	do {
		int forward_progress, n_sent;

		status = sun4v_cpu_mondo_send(cnt,
					      tb->cpu_list_pa,
					      tb->cpu_mondo_block_pa);

		/* HV_EOK means all cpus received the xcall, we're done.  */
		if (likely(status == HV_EOK))
			break;

		/* First, see if we made any forward progress.
		 *
		 * The hypervisor indicates successful sends by setting
		 * cpu list entries to the value 0xffff.
		 */
		n_sent = 0;
		for (i = 0; i < cnt; i++) {
			if (likely(cpu_list[i] == 0xffff))
				n_sent++;
		}

		forward_progress = 0;
		if (n_sent > prev_sent)
			forward_progress = 1;

		prev_sent = n_sent;

		/* If we get a HV_ECPUERROR, then one or more of the cpus
		 * in the list are in error state.  Use the cpu_state()
		 * hypervisor call to find out which cpus are in error state.
		 */
		if (unlikely(status == HV_ECPUERROR)) {
			for (i = 0; i < cnt; i++) {
				long err;
				u16 cpu;

				cpu = cpu_list[i];
				if (cpu == 0xffff)
					continue;

				err = sun4v_cpu_state(cpu);
				if (err == HV_CPU_STATE_ERROR) {
					saw_cpu_error = (cpu + 1);
					cpu_list[i] = 0xffff;
				}
			}
		} else if (unlikely(status != HV_EWOULDBLOCK))
			goto fatal_mondo_error;

		/* Don't bother rewriting the CPU list, just leave the
		 * 0xffff and non-0xffff entries in there and the
		 * hypervisor will do the right thing.
		 *
		 * Only advance timeout state if we didn't make any
		 * forward progress.
		 */
		if (unlikely(!forward_progress)) {
			if (unlikely(++retries > 10000))
				goto fatal_mondo_timeout;

		buffer->skb;
			buffer_info->dma = next_buffer->dma;
			next_buffer->skb = skb;
			next_buffer->dma = 0;
			goto next_desc;
		}
send_up:
		/*
		 * If this bit is set, then the RX registers contain
		 * the time stamp. No other packet will be time
		 * stamped until we read these registers, so read the
		 * registers to make them available again. Because
		 * only one packet can be time stamped at a time, we
		 * know that the register values must belong to this
		 * one here and therefore we don't need to compare
		 * any of the additional attributes stored for it.
		 *
		 * If nothing went wrong, then it should have a
		 * skb_shared_tx that we can turn into a
		 * skb_shared_hwtstamps.
		 *
		 * TODO: can time stamping be triggered (thus locking
		 * the registers) without the packet reaching this point
		 * here? In that case RX time stamping would get stuck.
		 *
		 * TODO: in "time stamp all packets" mode this bit is
		 * not set. Need a global flag for this mode and then
		 * always read the registers. Cannot be done without
		 * a race condition.
		 */
		if (unlikely(staterr & E1000_RXD_STAT_TS)) {
			u64 regval;
			u64 ns;
			struct skb_shared_hwtstamps *shhwtstamps =
				skb_hwtstamps(skb);

			WARN(!(rd32(E1000_TSYNCRXCTL) & E1000_TSYNCRXCTL_VALID),
			     "igb: no RX time stamp available for time stamped packet");
			regval = rd32(E1000_RXSTMPL);
			regval |= (u64)rd32(E1000_RXSTMPH) << 32;
			ns = timecounter_cyc2time(&adapter->clock, regval);
			timecompare_update(&adapter->compare, ns);
			memset(shhwtstamps, 0, sizeof(*shhwtstamps));
			shhwtstamps->hwtstamp = ns_to_ktime(ns);
			shhwtstamps->syststamp =
				timecompare_transform(&adapter->compare, ns);
		}

		if (staterr & E1000_RXDEXT_ERR_FRAME_ERR_MASK) {
			dev_kfree_skb_irq(skb);
			goto next_desc;
		}

		total_bytes += skb->len;
		total_packets++;

		igb_rx_checksum_adv(adapter, staterr, skb);

		skb->protocol = eth_type_trans(skb, netdev);

		igb_receive_skb(rx_ring, staterr, rx_desc, skb);

next_desc:
		rx_desc->wb.upper.status_error = 0;

		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= IGB_RX_BUFFER_WRITE) {
			igb_alloc_rx_buffers_adv(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		/* use prefetched values */
		rx_desc = next_rxd;
		buffer_info = next_buffer;
		staterr = le32_to_cpu(rx_desc->wb.upper.status_error);
	}

	rx_ring->next_to_clean = i;
	cleaned_count = igb_desc_unused(rx_ring);

	if (cleaned_count)
		igb_alloc_rx_buffers_adv(rx_ring, cleaned_count);

	rx_ring->total_packets += total_packets;
	rx_ring->total_bytes += total_bytes;
	rx_ring->rx_stats.packets += total_packets;
	rx_ring->rx_stats.bytes += total_bytes;
	adapter->net_stats.rx_bytes += total_bytes;
	adapter->net_stats.rx_packets += total_packets;
	return cleaned;
}

/**
 * igb_alloc_rx_buffers_adv - Replace used receive buffers; packet split
 * @adapter: address of board private structure
 **/
static void igb_alloc_rx_buffers_adv(struct igb_ring *rx_ring,
				     int cleaned_count)
{
	struct igb_adapter *adapter = rx_ring->adapter;
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	union e1000_adv_rx_desc *rx_desc;
	struct igb_buffer *buffer_info;
	struct sk_buff *skb;
	unsigned int i;
	int bufsz;

	i = rx_ring->next_to_use;
	buffer_info = &rx_ring->buffer_info[i];

	if (adapter->rx_ps_hdr_size)
		bufsz = adapter->rx_ps_hdr_size;
	else
		bufsz = adapter->rx_buffer_len;

	while (cleaned_count--) {
		rx_desc = E1000_RX_DESC_ADV(*rx_ring, i);

		if (adapter->rx_ps_hdr_size && !buffer_info->page_dma) {
			if (!buffer_info->page) {
				buffer_info->page = alloc_page(GFP_ATOMIC);
				if (!buffer_info->page) {
					adapter->alloc_rx_buff_failed++;
					goto no_buffers;
				}
				buffer_info->page_offset = 0;
			} else {
				buffer_info->page_offset ^= PAGE_SIZE / 2;
			}
			buffer_info->page_dma =
				pci_map_page(pdev, buffer_info->page,
					     buffer_info->page_offset,
					     PAGE_SIZE / 2,
					     PCI_DMA_FROMDEVICE);
		}

		if (!buffer_info->skb) {
			skb = netdev_alloc_skb(netdev, bufsz + NET_IP_ALIGN);
			if (!skb) {
				adapter->alloc_rx_buff_failed++;
				goto no_buffers;
			}

			/* Make buffer alignment 2 beyond a 16 byte boundary
			 * this will result in a 16 byte aligned IP header after
			 * the 14 byte MAC header is removed
			 */
			skb_reserve(skb, NET_IP_ALIGN);

			buffer_info->skb = skb;
			buffer_info->dma = pci_map_single(pdev, skb->data,
							  bufsz,
							  PCI_DMA_FROMDEVICE);
		}
		/* Refresh the desc even if buffer_addrs didn't change because
		 * each write-back erases this info. */
		if (adapter->rx_ps_hdr_size) {
			rx_desc->read.pkt_addr =
			     cpu_to_le64(buffer_info->page_dma);
			rx_desc->read.hdr_addr = cpu_to_le64(buffer_info->dma);
		} else {
			rx_desc->read.pkt_addr =
			     cpu_to_le64(buffer_info->dma);
			rx_desc->read.hdr_addr = 0;
		}

		i++;
		if (i == rx_ring->count)
			i = 0;
		buffer_info = &rx_ring->buffer_info[i];
	}

no_buffers:
	if (rx_ring->next_to_use != i) {
		rx_ring->next_to_use = i;
		if (i == 0)
			i = (rx_ring->count - 1);
		else
			i--;

		/* Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.  (Only
		 * applicable for weak-ordered memory model archs,
		 * such as IA-64). */
		wmb();
		writel(i, adapter->hw.hw_addr + rx_ring->tail);
	}
}

/**
 * igb_mii_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/
static int igb_mii_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct mii_ioctl_data *data = if_mii(ifr);

	if (adapter->hw.phy.media_type != e1000_media_type_copper)
		return -EOPNOTSUPP;

	switch (cmd) {
	case SIOCGMIIPHY:
		data->phy_id = adapter->hw.phy.addr;
		break;
	case SIOCGMIIREG:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (igb_read_phy_reg(&adapter->hw, data->reg_num & 0x1F,
		                     &data->val_out))
			return -EIO;
		break;
	case SIOCSMIIREG:
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

/**
 * igb_hwtstamp_ioctl - control hardware time stamping
 * @netdev:
 * @ifreq:
 * @cmd:
 *
 * Outgoing time stamping can be enabled and disabled. Play nice and
 * disable it when requested, although it shouldn't case any overhead
 * when no packet needs it. At most one packet in the queue may be
 * marked for time stamping, otherwise it would be impossible to tell
 * for sure to which packet the hardware time stamp belongs.
 *
 * Incoming time stamping has to be configured via the hardware
 * filters. Not all combinations are supported, in particular event
 * type has to be specified. Matching the kind of event packet is
 * not supported, with the exception of "all V2 events regardless of
 * level 2 or 4".
 *
 **/
static int igb_hwtstamp_ioctl(struct net_device *netdev,
			      struct ifreq *ifr, int cmd)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	struct hwtstamp_config config;
	u32 tsync_tx_ctl_bit = E1000_TSYNCTXCTL_ENABLED;
	u32 tsync_rx_ctl_bit = E1000_TSYNCRXCTL_ENABLED;
	u32 tsync_rx_ctl_type = 0;
	u32 tsync_rx_cfg = 0;
	int is_l4 = 0;
	int is_l2 = 0;
	short port = 319; /* PTP */
	u32 regval;

	if (copy_from_user(&config, ifr->ifr_data, sizeof(config)))
		return -EFAULT;

	/* reserved for future extensions */
	if (config.flags)
		return -EINVAL;

	switch (config.tx_type) {
	case HWTSTAMP_TX_OFF:
		tsync_tx_ctl_bit = 0;
		break;
	case HWTSTAMP_TX_ON:
		tsync_tx_ctl_bit = E1000_TSYNCTXCTL_ENABLED;
		break;
	default:
		return -ERANGE;
	}

	switch (config.rx_filter) {
	case HWTSTAMP_FILTER_NONE:
		tsync_rx_ctl_bit = 0;
		break;
	case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
	case HWTSTAMP_FILTER_ALL:
		/*
		 * register TSYNCRXCFG must be set, therefore it is not
		 * possible to time stamp both Sync and Delay_Req messages
		 * => fall back to time stamping all packets
		 */
		tsync_rx_ctl_type = E1000_TSYNCRXCTL_TYPE_ALL;
		config.rx_filter = HWTSTAMP_FILTER_ALL;
		break;
	case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
		tsync_rx_ctl_type = E1000_TSYNCRXCTL_TYPE_L4_V1;
		tsync_rx_cfg = E1000_TSYNCRXCFG_PTP_V1_SYNC_MESSAGE;
		is_l4 = 1;
		break;
	case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
		tsync_rx_ctl_type = E1000_TSYNCRXCTL_TYPE_L4_V1;
		tsync_rx_cfg = E1000_TSYNCRXCFG_PTP_V1_DELAY_REQ_MESSAGE;
		is_l4 = 1;
		break;
	case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
		tsync_rx_ctl_type = E1000_TSYNCRXCTL_TYPE_L2_L4_V2;
		tsync_rx_cfg = E1000_TSYNCRXCFG_PTP_V2_SYNC_MESSAGE;
		is_l2 = 1;
		is_l4 = 1;
		config.rx_filter = HWTSTAMP_FILTER_SOME;
		break;
	case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
		tsync_rx_ctl_type = E1000_TSYNCRXCTL_TYPE_L2_L4_V2;
		tsync_rx_cfg = E1000_TSYNCRXCFG_PTP_V2_DELAY_REQ_MESSAGE;
		is_l2 = 1;
		is_l4 = 1;
		config.rx_filter = HWTSTAMP_FILTER_SOME;
		break;
	case HWTSTAMP_FILTER_PTP_V2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
		tsync_rx_ctl_type = E1000_TSYNCRXCTL_TYPE_EVENT_V2;
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
		is_l2 = 1;
		break;
	default:
		return -ERANGE;
	}

	/* enable/disable TX */
	regval = rd32(E1000_TSYNCTXCTL);
	regval = (regval & ~E1000_TSYNCTXCTL_ENABLED) | tsync_tx_ctl_bit;
	wr32(E1000_TSYNCTXCTL, regval);

	/* enable/disable RX, define which PTP packets are time stamped */
	regval = rd32(E1000_TSYNCRXCTL);
	regval = (regval & ~E1000_TSYNCRXCTL_ENABLED) | tsync_rx_ctl_bit;
	regval = (regval & ~0xE) | tsync_rx_ctl_type;
	wr32(E1000_TSYNCRXCTL, regval);
	wr32(E1000_TSYNCRXCFG, tsync_rx_cfg);

	/*
	 * Ethertype Filter Queue Filter[0][15:0] = 0x88F7
	 *                                          (Ethertype to filter on)
	 * Ethertype Filter Queue Filter[0][26] = 0x1 (Enable filter)
	 * Ethertype Filter Queue Filter[0][30] = 0x1 (Enable Timestamping)
	 */
	wr32(E1000_ETQF0, is_l2 ? 0x440088f7 : 0);

	/* L4 Queue Filter[0]: only filter by source and destination port */
	wr32(E1000_SPQF0, htons(port));
	wr32(E1000_IMIREXT(0), is_l4 ?
	     ((1<<12) | (1<<19) /* bypass size and control flags */) : 0);
	wr32(E1000_IMIR(0), is_l4 ?
	     (htons(port)
	      | (0<<16) /* immediate interrupt disabled */
	      | 0 /* (1<<17) bit cleared: do not bypass
		     destination port check */)
		: 0);
	wr32(E1000_FTQF0, is_l4 ?
	     (0x11 /* UDP */
	      | (1<<15) /* VF not compared */
	      | (1<<27) /* Enable Timestamping */
	      | (7<<28) /* only source port filter enabled,
			   source/target address and protocol
			   masked */)
	     : ((1<<15) | (15<<28) /* all mask bits set = filter not
				      enabled */));

	wrfl();

	adapter->hwtstamp_config = config;

	/* clear TX/RX time stamp registers, just to be sure */
	regval = rd32(E1000_TXSTMPH);
	regval = rd32(E1000_RXSTMPH);

	return copy_to_user(ifr->ifr_data, &config, sizeof(config)) ?
		-EFAULT : 0;
}

/**
 * igb_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/
static int igb_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		return igb_mii_ioctl(netdev, ifr, cmd);
	case SIOCSHWTSTAMP:
		return igb_hwtstamp_ioctl(netdev, ifr, cmd);
	default:
		return -EOPNOTSUPP;
	}
}

static void igb_vlan_rx_register(struct net_device *netdev,
				 struct vlan_group *grp)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	u32 ctrl, rctl;

	igb_irq_disable(adapter);
	adapter->vlgrp = grp;

	if (grp) {
		/* enable VLAN tag insert/strip */
		ctrl = rd32(E1000_CTRL);
		ctrl |= E1000_CTRL_VME;
		wr32(E1000_CTRL, ctrl);

		/* enable VLAN receive filtering */
		rctl = rd32(E1000_RCTL);
		rctl &= ~E1000_RCTL_CFIEN;
		wr32(E1000_RCTL, rctl);
		igb_update_mng_vlan(adapter);
	} else {
		/* disable VLAN tag insert/strip */
		ctrl = rd32(E1000_CTRL);
		ctrl &= ~E1000_CTRL_VME;
		wr32(E1000_CTRL, ctrl);

		if (adapter->mng_vlan_id != (u16)IGB_MNG_VLAN_NONE) {
			igb_vlan_rx_kill_vid(netdev, adapter->mng_vlan_id);
			adapter->mng_vlan_id = IGB_MNG_VLAN_NONE;
		}
	}

	igb_rlpml_set(adapter);

	if (!test_bit(__IGB_DOWN, &adapter->state))
		igb_irq_enable(adapter);
}

static void igb_vlan_rx_add_vid(struct net_device *netdev, u16 vid)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	int pf_id = adapter->vfs_allocated_count;

	if ((hw->mng_cookie.status &
	     E1000_MNG_DHCP_COOKIE_STATUS_VLAN) &&
	    (vid == adapter->mng_vlan_id))
		return;

	/* add vid to vlvf if sr-iov is enabled,
	 * if that fails add directly to filter table */
	if (igb_vlvf_set(adapter, vid, true, pf_id))
		igb_vfta_set(hw, vid, true);

}

static void igb_vlan_rx_kill_vid(struct net_device *netdev, u16 vid)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	int pf_id = adapter->vfs_allocated_count;

	igb_irq_disable(adapter);
	vlan_group_set_device(adapter->vlgrp, vid, NULL);

	if (!test_bit(__IGB_DOWN, &adapter->state))
		igb_irq_enable(adapter);

	if ((adapter->hw.mng_cookie.status &
	     E1000_MNG_DHCP_COOKIE_STATUS_VLAN) &&
	    (vid == adapter->mng_vlan_id)) {
		/* release control to f/w */
		igb_release_hw_control(adapter);
		return;
	}

	/* remove vid from vlvf if sr-iov is enabled,
	 * if not in vlvf remove from vfta */
	if (igb_vlvf_set(adapter, vid, false, pf_id))
		igb_vfta_set(hw, vid, false);
}

static void igb_restore_vlan(struct igb_adapter *adapter)
{
	igb_vlan_rx_register(adapter->netdev, adapter->vlgrp);

	if (adapter->vlgrp) {
		u16 vid;
		for (vid = 0; vid < VLAN_GROUP_ARRAY_LEN; vid++) {
			if (!vlan_group_get_device(adapter->vlgrp, vid))
				continue;
			igb_vlan_rx_add_vid(adapter->netdev, vid);
		}
	}
}

int igb_set_spd_dplx(struct igb_adapter *adapter, u16 spddplx)
{
	struct e1000_mac_info *mac = &adapter->hw.mac;

	mac->autoneg = 0;

	/* Fiber NICs only allow 1000 gbps Full duplex */
	if ((adapter->hw.phy.media_type == e1000_media_type_fiber) &&
		spddplx != (SPEED_1000 + DUPLEX_FULL)) {
		dev_err(&adapter->pdev->dev,
			"Unsupported Speed/Duplex configuration\n");
		return -EINVAL;
	}

	switch (spddplx) {
	case SPEED_10 + DUPLEX_HALF:
		mac->forced_speed_duplex = ADVERTISE_10_HALF;
		break;
	case SPEED_10 + DUPLEX_FULL:
		mac->forced_speed_duplex = ADVERTISE_10_FULL;
		break;
	case SPEED_100 + DUPLEX_HALF:
		mac->forced_speed_duplex = ADVERTISE_100_HALF;
		break;
	case SPEED_100 + DUPLEX_FULL:
		mac->forced_speed_duplex = ADVERTISE_100_FULL;
		break;
	case SPEED_1000 + DUPLEX_FULL:
		mac->autoneg = 1;
		adapter->hw.phy.autoneg_advertised = ADVERTISE_1000_FULL;
		break;
	case SPEED_1000 + DUPLEX_HALF: /* not supported */
	default:
		dev_err(&adapter->pdev->dev,
			"Unsupported Speed/Duplex configuration\n");
		return -EINVAL;
	}
	return 0;
}

static int __igb_shutdown(struct pci_dev *pdev, bool *enable_wake)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	u32 ctrl, rctl, status;
	u32 wufc = adapter->wol;
#ifdef CONFIG_PM
	int retval = 0;
#endif

	netif_device_detach(netdev);

	if (netif_running(netdev))
		igb_close(netdev);

	igb_reset_interrupt_capability(adapter);

	igb_free_queues(adapter);

#ifdef CONFIG_PM
	retval = pci_save_state(pdev);
	if (retval)
		return retval;
#endif

	status = rd32(E1000_STATUS);
	if (status & E1000_STATUS_LU)
		wufc &= ~E1000_WUFC_LNKC;

	if (wufc) {
		igb_setup_rctl(adapter);
		igb_set_multi(netdev);

		/* turn on all-multi mode if wake on multicast is enabled */
		if (wufc & E1000_WUFC_MC) {
			rctl = rd32(E1000_RCTL);
			rctl |= E1000_RCTL_MPE;
			wr32(E1000_RCTL, rctl);
		}

		ctrl = rd32(E1000_CTRL);
		/* advertise wake from D3Cold */
		#define E1000_CTRL_ADVD3WUC 0x00100000
		/* phy power management enable */
		#define E1000_CTRL_EN_PHY_PWR_MGMT 0x00200000
		ctrl |= E1000_CTRL_ADVD3WUC;
		wr32(E1000_CTRL, ctrl);

		/* Allow time for pending master requests to run */
		igb_disable_pcie_master(&adapter->hw);

		wr32(E1000_WUC, E1000_WUC_PME_EN);
		wr32(E1000_WUFC, wufc);
	} else {
		wr32(E1000_WUC, 0);
		wr32(E1000_WUFC, 0);
	}

	*enable_wake = wufc || adapter->en_mng_pt;
	if (!*enable_wake)
		igb_shutdown_fiber_serdes_link_82575(hw);

	/* Release control of h/w to f/w.  If f/w is AMT enabled, this
	 * would have already happened in close and is redundant. */
	igb_release_hw_control(adapter);

	pci_disable_device(pdev);

	return 0;
}

#ifdef CONFIG_PM
static int igb_suspend(struct pci_dev *pdev, pm_message_t state)
{
	int retval;
	bool wake;

	retval = __igb_shutdown(pdev, &wake);
	if (retval)
		return retval;

	if (wake) {
		pci_prepare_to_sleep(pdev);
	} else {
		pci_wake_from_d3(pdev, false);
		pci_set_power_state(pdev, PCI_D3hot);
	}

	return 0;
}

static int igb_resume(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	u32 err;

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);

	err = pci_enable_device_mem(pdev);
	if (err) {
		dev_err(&pdev->dev,
			"igb: Cannot enable PCI device from suspend\n");
		return err;
	}
	pci_set_master(pdev);

	pci_enable_wake(pdev, PCI_D3hot, 0);
	pci_enable_wake(pdev, PCI_D3cold, 0);

	igb_set_interrupt_capability(adapter);

	if (igb_alloc_queues(adapter)) {
		dev_err(&pdev->dev, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	/* e1000_power_up_phy(adapter); */

	igb_reset(adapter);

	/* let the f/w know that the h/w is now under the control of the
	 * driver. */
	igb_get_hw_control(adapter);

	wr32(E1000_WUS, ~0);

	if (netif_running(netdev)) {
		err = igb_open(netdev);
		if (err)
			return err;
	}

	netif_device_attach(netdev);

	return 0;
}
#endif

static void igb_shutdown(struct pci_dev *pdev)
{
	bool wake;

	__igb_shutdown(pdev, &wake);

	if (system_state == SYSTEM_POWER_OFF) {
		pci_wake_from_d3(pdev, wake);
		pci_set_power_state(pdev, PCI_D3hot);
	}
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void igb_netpoll(struct net_device *netdev)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	int i;

	if (!adapter->msix_entries) {
		igb_irq_disable(adapter);
		napi_schedule(&adapter->rx_ring[0].napi);
		return;
	}

	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct igb_ring *tx_ring = &adapter->tx_ring[i];
		wr32(E1000_EIMC, tx_ring->eims_value);
		igb_clean_tx_irq(tx_ring);
		wr32(E1000_EIMS, tx_ring->eims_value);
	}

	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct igb_ring *rx_ring = &adapter->rx_ring[i];
		wr32(E1000_EIMC, rx_ring->eims_value);
		napi_schedule(&rx_ring->napi);
	}
}
#endif /* CONFIG_NET_POLL_CONTROLLER */

/**
 * igb_io_error_detected - called when PCI error is detected
 * @pdev: Pointer to PCI device
 * @state: The current pci connection state
 *
 * This function is called after a PCI bus error affecting
 * this device has been detected.
 */
static pci_ers_result_t igb_io_error_detected(struct pci_dev *pdev,
					      pci_channel_state_t state)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igb_adapter *adapter = netdev_priv(netdev);

	netif_device_detach(netdev);

	if (state == pci_channel_io_perm_failure)
		return PCI_ERS_RESULT_DISCONNECT;

	if (netif_running(netdev))
		igb_down(adapter);
	pci_disable_device(pdev);

	/* Request a slot slot reset. */
	return PCI_ERS_RESULT_NEED_RESET;
}

/**
 * igb_io_slot_reset - called after the pci bus has been reset.
 * @pdev: Pointer to PCI device
 *
 * Restart the card from scratch, as if from a cold-boot. Implementation
 * resembles the first-half of the igb_resume routine.
 */
static pci_ers_result_t igb_io_slot_reset(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igb_adapter *adapter = netdev_priv(netdev);
	struct e1000_hw *hw = &adapter->hw;
	pci_ers_result_t result;
	int err;

	if (pci_enable_device_mem(pdev)) {
		dev_err(&pdev->dev,
			"Cannot re-enable PCI device after reset.\n");
		result = PCI_ERS_RESULT_DISCONNECT;
	} else {
		pci_set_master(pdev);
		pci_restore_state(pdev);

		pci_enable_wake(pdev, PCI_D3hot, 0);
		pci_enable_wake(pdev, PCI_D3cold, 0);

		igb_reset(adapter);
		wr32(E1000_WUS, ~0);
		result = PCI_ERS_RESULT_RECOVERED;
	}

	err = pci_cleanup_aer_uncorrect_error_status(p