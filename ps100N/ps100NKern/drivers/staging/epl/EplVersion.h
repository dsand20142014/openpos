its1, newbits2;

		if (GET_ASIC_REV(tp->pci_chip_rev_id) == ASIC_REV_5700 ||
		    GET_ASIC_REV(tp->pci_chip_rev_id) == ASIC_REV_5701) {
			newbits1 = (CLOCK_CTRL_RXCLK_DISABLE |
				    CLOCK_CTRL_TXCLK_DISABLE |
				    CLOCK_CTRL_ALTCLK);
			newbits2 = newbits1 | CLOCK_CTRL_44MHZ_CORE;
		} else if (tp->tg3_flags2 & TG3_FLG2_5705_PLUS) {
			newbits1 = CLOCK_CTRL_625_CORE;
			newbits2 = newbits1 | CLOCK_CTRL_ALTCLK;
		} else {
			newbits1 = CLOCK_CTRL_ALTCLK;
			newbits2 = newbits1 | CLOCK_CTRL_44MHZ_CORE;
		}

		tw32_wait_f(TG3PCI_CLOCK_CTRL, tp->pci_clock_ctrl | newbits1,
			    40);

		tw32_wait_f(TG3PCI_CLOCK_CTRL, tp->pci_clock_ctrl | newbits2,
			    40);

		if (!(tp->tg3_flags2 & TG3_FLG2_5705_PLUS)) {
			u32 newbits3;

			if (GET_ASIC_REV(tp->pci_chip_rev_id) == ASIC_REV_5700 ||
			    GET_ASIC_REV(tp->pci_chip_rev_id) == ASIC_REV_5701) {
				newbits3 = (CLOCK_CTRL_RXCLK_DISABLE |
					    CLOCK_CTRL_TXCLK_DISABLE |
					    CLOCK_CTRL_44MHZ_CORE);
			} else {
				newbits3 = CLOCK_CTRL_44MHZ_CORE;
			}

			tw32_wait_f(TG3PCI_CLOCK_CTRL,
				    tp->pci_clock_ctrl | newbits3, 40);
		}
	}

	if (!(device_should_wake) &&
	    !(tp->tg3_flags & TG3_FLAG_ENABLE_ASF))
		tg3_power_down_phy(tp, do_low_power);

	tg3_frob_aux_power(tp);

	/* Workaround for unstable PLL clock */
	if ((GET_CHIP_REV(tp->pci_chip_rev_id) == CHIPREV_5750_AX) ||
	    (GET_CHIP_REV(tp->pci_chip_rev_id) == CHIPREV_5750_BX)) {
		u32 val = tr32(0x7d00);

		val &= ~((1 << 16) | (1 << 4) | (1 << 2) | (1 << 1) | 1);
		tw32(0x7d00, val);
		if (!(tp->tg3_flags & TG3_FLAG_ENABLE_ASF)) {
			int err;

			err = tg3_nvram_lock(tp);
			tg3_halt_cpu(tp, RX_CPU_BASE);
			if (!err)
				tg3_nvram_unlock(tp);
		}
	}

	tg3_write_sig_post_reset(tp, RESET_KIND_SHUTDOWN);

	if (device_should_wake)
		pci_enable_wake(tp->pdev, state, true);

	/* Finally, set the new power state. */
	pci_set_power_state(tp->pdev, state);

	return 0;
}

static void tg3_aux_stat_to_speed_duplex(struct tg3 *tp, u32 val, u16 *speed, u8 *duplex)
{
	switch (val & MII_TG3_AUX_STAT_SPDMASK) {
	case MII_TG3_AUX_STAT_10HALF:
		*speed = SPEED_10;
		*duplex = DUPLEX_HALF;
		break;

	case MII_TG3_AUX_STAT_10FULL:
		*speed = SPEED_10;
		*duplex = DUPLEX_FULL;
		break;

	case MII_TG3_AUX_STAT_100HALF:
		*speed = SPEED_100;
		*duplex = DUPLEX_HALF;
		break;

	case MII_TG3_AUX_STAT_100FULL:
		*speed = SPEED_100;
		*duplex = DUPLEX_FULL;
		break;

	case MII_TG3_AUX_STAT_1000HALF:
		*speed = SPEED_1000;
		*duplex = DUPLEX_HALF;
		break;

	case MII_TG3_AUX_STAT_1000FULL:
		*speed = SPEED_1000;
		*duplex = DUPLEX_FULL;
		break;

	default:
		if (GET_ASIC_REV(tp->pci_chip_rev_id) == ASIC_REV_5906) {
			*speed = (val & MII_TG3_AUX_STAT_100) ? SPEED_100 :
				 SPEED_10;
			*duplex = (val & MII_TG3_AUX_STAT_FULL) ? DUPLEX_FULL :
				  DUPLEX_HALF;
			break;
		}
		*speed = SPEED_INVALID;
		*duplex = DUPLEX_INVALID;
		break;
	}
}

static void tg3_phy_copper_begin(struct tg3 *tp)
{
	u32 new_adv;
	int i;

	if (tp->link_config.phy_is_low_power) {
		/* Entering low power mode.  Disable gigabit and
		 * 100baseT advertisements.
		 */
		tg3_writephy(tp, MII_TG3_CTRL, 0);

		new_adv = (ADVERTISE_10HALF | ADVERTISE_10FULL |
			   ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP);
		if (tp->tg3_flags & TG3_FLAG_WOL_SPEED_100MB)
			new_adv |= (ADVERTISE_100HALF | ADVERTISE_100FULL);

		tg3_writephy(tp, MII_ADVERTISE, new_adv);
	} else if (tp->link_config.speed == SPEED_INVALID) {
		if (tp->tg3_flags & TG3_FLAG_10_100_ONLY)
			tp->link_config.advertising &=
				~(ADVERTISED_1000baseT_Half |
				  ADVERTISED_1000baseT_Full);

		new_adv = ADVERTISE_CSMA;
		if (tp->link_config.advertising & ADVERTISED_10baseT_Half)
			new_adv |= ADVERTISE_10HALF;
		if (tp->link_config.advertising & ADVERTISED_10baseT_Full)
			new_adv |= ADVERTISE_10FULL;
		if (tp->link_config.advertising & ADVERTISED_100baseT_Half)
			new_adv |= ADVERTISE_100HALF;
		if (tp->link_config.advertising & ADVERTISED_100baseT_Full)
			new_adv |= ADVERTISE_100FULL;

		new_adv |= tg3_advert_flowctrl_1000T(tp->link_config.flowctrl);

		tg3_writephy(tp, MII_ADVERTISE, new_adv);

		if (tp->link_config.advertising &
		    (A