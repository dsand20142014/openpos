	 * is inverted,  because it is normally used to indicate
		 * a hardware fault at reset, if there were errors
		 */
		extctl = (dd->ipath_extctrl & ~INFINIPATH_EXTC_LEDGBLOK_ON)
			| INFINIPATH_EXTC_LEDGBLERR_OFF;
		if (ltst == INFINIPATH_IBCS_LT_STATE_LINKUP)
			extctl &= ~INFINIPATH_EXTC_LEDGBLERR_OFF;
		if (lst == INFINIPATH_IBCS_L_STATE_ACTIVE)
			extctl |= INFINIPATH_EXTC_LEDGBLOK_ON;
	}
	else {
		extctl = dd->ipath_extctrl &
			~(INFINIPATH_EXTC_LED1PRIPORT_ON |
			  INFINIPATH_EXTC_LED2PRIPORT_ON);
		if (ltst == INFINIPATH_IBCS_LT_STATE_LINKUP)
			extctl |= INFINIPATH_EXTC_LED1PRIPORT_ON;
		if (lst == INFINIPATH_IBCS_L_STATE_ACTIVE)
			extctl |= INFINIPATH_EXTC_LED2PRIPORT_ON;
	}
	dd->ipath_extctrl = extctl;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_extctrl, extctl);
	spin_unlock_irqrestore(&dd->ipath_gpio_lock, flags);
}

static void ipath_init_ht_variables(struct ipath_devdata *dd)
{
	/*
	 * setup the register offsets, since they are different for each
	 * chip
	 */
	dd->ipath_kregs = &ipath_ht_kregs;
	dd->ipath_cregs = &ipath_ht_cregs;

	dd->ipath_gpio_sda_num = _IPATH_GPIO_SDA_NUM;
	dd->ipath_gpio_scl_num = _IPATH_GPIO_SCL_NUM;
	dd->ipath_gpio_sda = IPATH_GPIO_SDA;
	dd->ipath_gpio_scl = IPATH_GPIO_SCL;

	/*
	 * Fill in data for field-values that change in newer chips.
	 * We dynamically specify only the mask for LINKTRAININGSTATE
	 * and only the shift for LINKSTATE, as they are the only ones
	 * that change.  Also precalculate the 3 link states of interest
	 * and the combined mask.
	 */
	dd->ibcs_ls_shift = IBA6110_IBCS_LINKSTATE_SHIFT;
	dd->ibcs_lts_mask = IBA6110_IBCS_LINKTRAININGSTATE_MASK;
	dd->ibcs_mask = (INFINIPATH_IBCS_LINKSTATE_MASK <<
		dd->ibcs_ls_shift) | dd->ibcs_lts_mask;
	dd->ib_init = (INFINIPATH_IBCS_LT_STATE_LINKUP <<
		INFINIPATH_IBCS_LINKTRAININGSTATE_SHIFT) |
		(INFINIPATH_IBCS_L_STATE_INIT << dd->ibcs_ls_shift);
	dd->ib_arm = (INFINIPATH_IBCS_LT_STATE_LINKUP <<
		INFINIPATH_IBCS_LINKTRAININGSTATE_SHIFT) |
		(INFINIPATH_IBCS_L_STATE_ARM << dd->ibcs_ls_shift);
	dd->ib_active = (INFINIPATH_IBCS_LT_STATE_LINKUP <<
		INFINIPATH_IBCS_LINKTRAININGSTATE_SHIFT) |
		(INFINIPATH_IBCS_L_STATE_ACTIVE << dd->ibcs_ls_shift);

	/*
	 * Fill in data for ibcc field-values that change in newer chips.
	 * We dynamically specify only the mask for LINKINITCMD
	 * and only the shift for LINKCMD and MAXPKTLEN, as they are
	 * the only ones that change.
	 */
	dd->ibcc_lic_mask = INFINIPATH_IBCC_LINKINITCMD_MASK;
	dd->ibcc_lc_shift = INFINIPATH_IBCC_LINKCMD_SHIFT;
	dd->ibcc_mpl_shift = INFINIPATH_IBCC_MAXPKTLEN_SHIFT;

	/* Fill in shifts for RcvCtrl. */
	dd->ipath_r_portenable_shift = INFINIPATH_R_PORTENABLE_SHIFT;
	dd->ipath_r_intravail_shift = INFINIPATH_R_INTRAVAIL_SHIFT;
	dd->ipath_r_tailupd_shift = INFINIPATH_R_TAILUPD_SHIFT;
	dd->ipath_r_portcfg_shift = 0; /* Not on IBA6110 */

	dd->ipath_i_bitsextant =
		(INFINIPATH_I_RCVURG_MASK << INFINIPATH_I_RCVURG_SHIFT) |
		(INFINIPATH_I_RCVAVAIL_MASK <<
		 INFINIPATH_I_RCVAVAIL_SHIFT) |
		INFINIPATH_I_ERROR | INFINIPATH_I_SPIOSENT |
		INFINIPATH_I_SPIOBUFAVAIL | INFINIPATH_I_GPIO;

	dd->ipath_e_bitsextant =
		INFINIPATH_E_RFORMATERR | INFINIPATH_E_RVCRC |
		INFINIPATH_E_RICRC |