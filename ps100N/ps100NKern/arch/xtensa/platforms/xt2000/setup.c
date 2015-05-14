= ASIC_REV_5714)) {
		u32 tmp;

		tmp = tr32(SERDES_RX_CTRL);
		tw32(SERDES_RX_CTRL, tmp | SERDES_RX_SIG_DETECT);
		tp->grc_local_ctrl &= ~GRC_LCLCTRL_USE_EXT_SIG_DETECT;
		tp->grc_local_ctrl |= GRC_LCLCTRL_USE_SIG_DETECT;
		tw32(GRC_LOCAL_CTRL, tp->grc_local_ctrl);
	}

	if (!(tp->tg3_flags3 & TG3_FLG3_USE_PHYLIB)) {
		if (tp->link_config.phy_is_low_power) {
			tp->link_config.phy_is_low_power = 0;
			tp->link_config.speed = tp->link_config.orig_speed;
			tp->link_config.duplex = tp->link_config.orig_duplex;
			tp->link_config.autoneg = tp->link_config.orig_autoneg;
		}

		err = tg3_setup_phy(tp, 0);
		if (err)
			return err;

		if (!(tp->tg3_flags2 & TG3_FLG2_PHY_SERDES) &&
		    GET_ASIC_REV(tp->pci_chip_rev_id) != ASIC_REV_5906) {
			u32 tmp;

			/* Clear CRC stats. */
			if (!tg3_readphy(tp, MII_TG3_TEST1, &tmp)) {
				tg3_writephy(tp, MII_TG3_TEST1,
					     tmp | MII_TG3_TEST1_CRC_EN);
				tg3_readphy(tp, 0x14, &tmp);
			}
		}
	}

	__tg3_set_rx_mode(tp->dev);

	/* Initialize receive rules. */
	tw32(MAC_RCV_RULE_0,  0xc2000000 & RCV_RULE_DISABLE_MASK);
	tw32(MAC_RCV_VALUE_0, 0xffffffff & RCV_RULE_DISABLE_MASK);
	tw32(MAC_RCV_RULE_1,  0x86000004 & RCV_RULE_DISABLE_MASK);
	tw32(MAC_RCV_VALUE_1, 0xffffffff & RCV_RULE_DISABLE_MASK);

	if ((tp->tg3_flags2 & TG3_FLG2_5705_PLUS) &&
	    !(tp->tg3_flags2 & TG3_FLG2_5780_CLASS))
		limit = 8;
	else
		limit = 16;
	if (tp->tg3_flags & TG3_FLAG_ENABLE_ASF)
		limit -= 4;
	switch (limit) {
	case 16:
		tw32(MAC_RCV_RULE_15,  0); tw32(MAC_RCV_VALUE_15,  0);
	case 15:
		tw32(MAC_RCV_RULE_14,  0); tw32(MAC_RCV_VALUE_14,  0);
	case 14:
		tw32(MAC_RCV_RULE_13,  0); tw32(MAC_RCV_VALUE_13,  0);
	case 13:
		tw32(MAC_RCV_RULE_12,  0); tw32(MAC_RCV_VALUE_12,  0);
	case 12:
		tw32(MAC_RCV_RULE_11,  0); tw32(MAC_RCV_VALUE_11,  0);
	case 11:
		tw32(MAC_RCV_RULE_10,  0); tw32(MAC_RCV_VALUE_10,  0);
	case 10:
		tw32(MAC_RCV_RULE_9,  0); tw32(MAC_RCV_VALUE_9,  0);
	case 9:
		tw32(MAC_RCV_RULE_8,  0); tw32(MAC_RCV_VALUE_8,  0);
	case 8:
		tw32(MAC_RCV_RULE_7,  0); tw32(MAC_RCV_VALUE_7,  0);
	case 7:
		tw32(MAC_RCV_RULE_6,  0); tw32(MAC_RCV_VALUE_6,  0);
	case 6:
		tw32(MAC_RCV_RULE_5,  0); tw32(MAC_RCV_VALUE_5,  0);
	case 5:
		tw32(MAC_RCV_RULE_4,  0); tw32(MAC_RCV_VALUE_4,  0);
	case 4:
		/* tw32(MAC_RCV_RULE_3,  0); tw32(MAC_RCV_VALUE_3,  0); */
	case 3:
		/* tw32(MAC_RCV_RULE_2,  0); tw32(MAC_RCV_VALUE_2,  0); */
	case 2:
	case 1:

	default:
		break;
	}

	if (tp->tg3_flags3 & TG3_FLG3_ENABLE_APE)
		/* Write our heartbeat update interval to APE. */
		tg3_ape_write32(tp, TG3_APE_HOST_HEARTBEAT_INT_MS,
				APE_HOST_HEARTBEAT_INT_DISABLE);

	tg3_write_sig_post_reset(tp, RESET_KIND_INIT);

	return 0;
}

/* Called at device open time to get the chip ready for
 * packet processing.  Invoked with tp->lock held.
 */
static int tg3_init_hw(struct tg3 *tp, int reset_phy)
{
	tg3_switch_clocks(tp);

	tw32(TG3PCI_MEM_WIN_BASE_ADDR, 0);

	return tg3_reset_hw(tp, reset_phy);
}

#define TG3_STAT_ADD32(PSTAT, REG) \
do {	u32 __val = tr32(REG); \
	(PSTAT)->low += __val; \
	if ((PSTAT)->low < __val) \
		(PSTAT)->high += 1; \
} while (0)

static void tg3_periodic_fetch_stats(struct tg3 *tp)
{
	struct tg3_hw_stats *sp = tp->hw_stats;

	if (!netif_carrier_ok(tp->dev))
		return;

	TG3_STAT_ADD32(&sp->tx_octets, MAC_TX_STATS_OCTETS);
	TG3_STAT_ADD32(&sp->tx_collisions, MAC_TX_STATS_COLLISIONS);
	TG3_STAT_ADD32(&sp->tx_xon_sent, MAC_TX_STATS_XON_SENT);
	TG3_STAT_ADD32(&sp->tx_xoff_sent, MAC_TX_STATS_XOFF_SENT);
	TG3_STAT_ADD32(&sp->tx_mac_errors, MAC_TX_STATS_MAC_ERRORS);
	TG3_STAT_ADD32(&sp->tx_single_collisions, MAC_TX_STATS_SINGLE_COLLISIONS);
	TG3_STAT_ADD32(&sp->tx_mult_collisions, MAC_TX_STATS_MULT_COLLISIONS);
	TG3_STAT_ADD32(&sp->tx_deferred, MAC_TX_STATS_DEFERRED);
	TG3_STAT_ADD32(&sp->tx_excessive_collisions, MAC_TX_STATS_EXCESSIVE_COL);
	TG3_STAT_ADD32(&sp->tx_late_collisions, MAC_TX_STATS_LATE_COL);
	TG3_STAT_ADD32(&sp->tx_ucast_packets, MAC_TX_STATS_UCAST);
	TG3_STAT_ADD32(&sp->tx_mcast_packets, MAC_TX_STATS_MCAST);
	TG3_STAT_ADD32(&sp->tx_bcast_packets, MAC_TX_STATS_BCAST);

	TG3_STAT_ADD32(&sp->rx_octets, MAC_RX_STATS_OCTETS);
	TG3_STAT_ADD32(&sp->rx_fragments, MAC