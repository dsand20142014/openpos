imer(&dd->ipath_hol_timer,
			dd->ipath_hol_timer.expires);
	}
}

int ipath_set_rx_pol_inv(struct ipath_devdata *dd, u8 new_pol_inv)
{
	u64 val;

	if (new_pol_inv > INFINIPATH_XGXS_RX_POL_MASK)
		return -1;
	if (dd->ipath_rx_pol_inv != new_pol_inv) {
		dd->ipath_rx_pol_inv = new_pol_inv;
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_xgxsconfig);
		val &= ~(INFINIPATH_XGXS_RX_POL_MASK <<
			 INFINIPATH_XGXS_RX_POL_SHIFT);
		val |= ((u64)dd->ipath_rx_pol_inv) <<
			INFINIPATH_XGXS_RX_POL_SHIFT;
		ipath_write_kreg(dd, dd->ipath_kregs->kr_xgxsconfig, val);
	}
	return 0;
}

/*
 * Disable and enable the armlaunch error.  Used for PIO bandwidth testing on
 * the 7220, which is count-based, rather than trigger-based.  Safe for the
 * driver check, since it's at init.   Not completely safe when used