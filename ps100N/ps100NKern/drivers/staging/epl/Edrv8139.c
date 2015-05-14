p-level
		 * hwerrmask and in IB uC control reg. uC will allow
		 * it again during startup.
		 */
		u64 val;
		rst_val &= ~(1ULL);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
			dd->ipath_hwerrmask &
			~INFINIPATH_HWE_IB_UC_MEMORYPARITYERR);

		ret = ipath_resync_ibepb(dd);
		if (ret < 0)
			ipath_dev_err(dd, "unable to re-sync IB EPB\n");

		/* set uC control regs to suppress parity errs */
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG5, 1, 1);
		if (ret < 0)
			goto bail;
		/* IB uC code past Version 1.32.17 allow suppression of wdog */
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6, 0x80,
			0x80);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed to set WDOG disable\n");
			goto bail;
		}
		ipath_write_kreg(dd, dd->ipath_kregs->kr_ibserdesctrl, rst_val);
		/* flush write, delay for startup */
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		udelay(1);
		/* clear, then re-enable parity errs */
		ipath_sd7220_clr_ibpar(dd);
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_hwerrstatus);
		if (val & INFINIPATH_HWE_IB_UC_MEMORYPARITYERR) {
			ipath_dev_err(dd, "IBUC Parity still set after RST\n");
			dd->ipath_hwerrmask &=
				~INFINIPATH_HWE_IB_UC_MEMORYPARITYERR;
		}
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
			dd->ipath_hwerrmask);
	}

bail:
	return ret;
}

static void ipath_sd_trimdone_monitor(struct ipath_devdata *dd,
       const char *where)
{
	int ret, chn, baduns;
	u64 val;

	if (!where)
		where = "?";

	/* give time for reset to settle out in EPB */
	udelay(2);

	ret = ipath_resync_ibepb(dd);
	if (ret < 0)
		ipath_dev_err(dd, "not able to re-sync IB EPB (%s)\n", where);

	/* Do "sacrificial read" to get EPB in sane state after reset */
	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_CTRL2(0), 0, 0);
	if (ret < 0)
		ipath_dev_err(dd, "Failed TRIMDONE 1st read, (%s)\n", where);

	/* Check/show "summary" Trim-done bit in IBCStatus */
	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
	if (val & (1ULL << 11))
		ipath_cdbg(VERBOSE, "IBCS TRIMDONE set (%s)\n", where);
	else
		ipath_dev_err(dd, "IBCS TRIMDONE clear (%s)\n", where);

	udelay(2);

	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6, 0x80, 0x80);
	if (ret < 0)
		ipath_dev_err(dd, "Failed Dummy RMW, (%s)\n", where);
	udelay(10);

	baduns = 0;

	for (chn = 3; chn >= 0; --chn) {
		/* Read CTRL reg for each channel to check TRIMDONE */
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
			IB_CTRL2(chn), 0, 0);
		if (ret < 0)
			ipath_dev_err(dd, "Failed checking TRIMDONE, chn %d"
				" (%s)\n", chn, where);

		if (!(ret & 0x10)) {
			int probe;
			baduns |= (1 << chn);
			ipath_dev_err(dd, "TRIMDONE cleared on chn %d (%02X)."
				" (%s)\n", chn, ret, where);
			probe = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_PGUDP(0), 0, 0);
			ipath_dev_err(dd, "probe is %d (%02X)\n",
				probe, probe);
			probe = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_CTRL2(chn), 0, 0);
			ipath_dev_err(dd, "re-read: %d (%02X)\n",
				probe, probe);
			ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_CTRL2(chn), 0x10, 0x10);
			if (ret < 0)
				ipath_dev_err(dd,
					"Err on TRIMDONE rewrite1\n");
		}
	}
	for (chn = 3; chn >= 0; --chn) {
		/* Read CTRL reg for each channel to check TRIMDONE */
		if (baduns & (1 << chn)) {
			ipath_dev_err(dd,
				"Reseting TRIMDONE on chn %d (%s)\n",
				chn, where);
			ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_CTRL2(chn), 0x10, 0x10);
			if (ret < 0)
				ipath_dev_err(dd, "Failed re-setting "
					"TRIMDONE, chn %d (%s)\n",
					chn, where);
		}
	}
}

/*
 * Below is portion of IBA7220-specific bringup_serdes() that actually
 * deals with registers and memory within the SerDes itself.
 * Post IB uC code version 1.32.17, was_reset being 1 is not really
 * informative, so we double-check.
 */
int ipath_sd7220_init(struct ipath_devdata *dd, int was_reset)
{
	int ret = 1; /* default to failure */
	int first_reset;
	int val_stat;

	if (!was_reset) {
		/* entered with reset not asserted, we need to do it */
		ipath_ibsd_reset(dd, 1);
		ipath_sd_trimdone_monitor(dd, "Driver-reload");
	}

	/* Substitute our deduced value for was_reset */
	ret = ipath_ibsd_ucode_loaded(dd);
	if (ret < 0) {
		ret = 1;
		goto done;
	}
	first_reset = !ret; /* First reset if IBSD uCode not yet loaded */

	/*
	 * Alter some regs per vendor latest doc, reset-defaults
	 * are not right for IB.
	 */
	ret = ipath_sd_early(dd);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed to set IB SERDES early defaults\n");
		ret = 1;
		goto done;
	}

	/*
	 * Set DAC manual trim IB.
	 * We only do this once after chip has been reset (usually
	 * same as once per system boot).
	 */
	if (first_reset) {
		ret = ipath_sd_dactrim(dd);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed IB SERDES DAC trim\n");
			ret = 1;
			goto done;
		}
	}

	/*
	 * Set various registers (DDS and RXEQ) that will be
	 * controlled by IBC (in 1.2 mode) to reasonable preset values
	 * Calling the "internal" version avoids the "check for needed"
	 * and "trimdone monitor" that might be counter-productive.
	 */
	ret = ipath_internal_presets(dd);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed to set IB SERDES presets\n");
		ret = 1;
		goto done;
	}
	ret = ipath_sd_trimself(dd, 0x80);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed to set IB SERDES TRIMSELF\n");
		ret = 1;
		goto done;
	}

	/* Load image, then try to verify */
	ret = 0;	/* Assume success */
	if (first_reset) {
		int vfy;
		int trim_done;
		ipath_dbg("SerDes uC was reset, reloading PRAM\n");
		ret = ipath_sd7220_ib_load(dd);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed to load IB SERDES image\n");
			ret = 1;
			goto done;
		}

		/* Loaded image, try to verify */
		vfy = ipath_sd7220_ib_vfy(dd);
		if (vfy != ret) {
			ipath_dev_err(dd, "SERDES PRAM VFY failed\n");
			ret = 1;
			goto done;
		}
		/*
		 * Loaded and verified. Almost good...
		 * hold "success" in ret
		 */
		ret = 0;

		/*
		 * Prev steps all worked, continue bringup
		 * De-assert RESET to uC, only in first reset, to allow
		 * trimming.
		 *
		 * Since our default setup sets START_EQ1 to
		 * PRESET, we need to clear that for this very first run.
		 */
		ret = ibsd_mod_allchnls(dd, START_EQ1(0), 0, 0x38);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed clearing START_EQ1\n");
			ret = 1;
			goto done;
		}

		ipath_ibsd_reset(dd, 0);
		/*
		 * If this is not the first reset, trimdone should be set
		 * already.
		 */
		trim_done = ipath_sd_trimdone_poll(dd);
		/*
		 * Whether or not trimdone succeeded, we need to put the
		 * uC back into reset to avoid a possible fight with the
		 * IBC state-machine.
		 */
		ipath_ibsd_reset(dd, 1);

		if (!trim_done) {
			ipath_dev_err(dd, "No TRIMDONE seen\n");
			ret = 1;
			goto done;
		}

		ipath_sd_trimdone_monitor(dd, "First-reset");
		/* Remember so we do not re-do the load, dactrim, etc. */
		dd->serdes_first_init_done = 1;
	}
	/*
	 * Setup for channel training and load values for
	 * RxEq and DDS in tables used by IBC in IB1.2 mode
	 */

	val_stat = ipath_sd_setvals(dd);
	if (val_stat < 0)
		ret = 1;
done:
	/* start relock timer regardless, but start at 1 second */
	ipath_set_relock_poll(dd, -1);
	return ret;
}

#define EPB_ACC_REQ 1
#define EPB_ACC_GNT 0x100
#define EPB_DATA_MASK 0xFF
#define EPB_RD (1ULL << 24)
#define EPB_TRANS_RDY (1ULL << 31)
#define EPB_TRANS_ERR (1ULL << 30)
#define EPB_TRANS_TRIES 5

/*
 * query, claim, release ownership of the EPB (External Parallel Bus)
 * for a specified SERDES.
 * the "claim" parameter is >0 to claim, <0 to release, 0 to query.
 * Returns <0 for errors, >0 if we had ownership, else 0.
 */
static int epb_access(struct ipath_devdata *dd, int sdnum, int claim)
{
	u16 acc;
	u64 accval;
	int owned = 0;
	u64 oct_sel = 0;

	switch (sdnum) {
	case IB_7220_SERDES :
		/*
		 * The IB SERDES "ownership" is fairly simple. A single each
		 * request/grant.
		 */
		acc = dd->ipath_kregs->kr_ib_epbacc;
		break;
	case PCIE_SERDES0 :
	case PCIE_SERDES1 :
		/* PCIe SERDES has two "octants", need to select which */
		acc = dd->ipath_kregs->kr_pcie_epbacc;
		oct_sel = (2 << (sdnum - PCIE_SERDES0));
		break;
	default :
		return 0;
	}

	/* Make sure any outstanding transaction was seen */
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	udelay(15);

	accval = ipath_read_kreg32(dd, acc);

	owned = !!(accval & EPB_ACC_GNT);
	if (claim < 0) {
		/* Need to release */
		u64 pollval;
		/*
		 * The only writeable bits are the request and CS.
		 * Both should be clear
		 */
		u64 newval = 0;
		ipath_write_kreg(dd, acc, newval);
		/* First read after write is not trustworthy */
		pollval = ipath_read_kreg32(dd, acc);
		udelay(5);
		pollval = ipath_read_kreg32(dd, acc);
		if (pollval & EPB_ACC_GNT)
			owned = -1;
	} else if (claim > 0) {
		/* Need to claim */
		u64 pollval;
		u64 newval = EPB_ACC_REQ | oct_sel;
		ipath_write_kreg(dd, acc, newval);
		/* First read after write is not trustworthy */
		pollval = ipath_read_kreg32(dd, acc);
		udelay(5);
		pollval = ipath_read_kreg32(dd, acc);
		if (!(pollval & EPB_ACC_GNT))
			owned = -1;
	}
	return owned;
}

/*
 * Lemma to deal with race condition of write..read to epb regs
 */
static int epb_trans(struct ipath_devdata *dd, u16 reg, u64 i_val, u64 *o_vp)
{
	int tries;
	u64 transval;


	ipath_write_kreg(dd, reg, i_val);
	/* Throw away first read, as RDY bit may be stale */
	transval = ipath_read_kreg64(dd, reg);

	for (tries = EPB_TRANS_TRIES; tries; --tries) {
		transval = ipath_read_kreg32(dd, reg);
		if (transval & EPB_TRANS_RDY)
			break;
		udelay(5);
	}
	if (transval & EPB_TRANS_ERR)
		return -1;
	if (tries > 0 && o_vp)
		*o_vp = transval;
	return tries;
}

/**
 *
 * ipath_sd7220_reg_mod - modify SERDES register
 * @dd: the infinipath device
 * @sdnum: which SERDES to access
 * @loc: location - channel, element, register, as packed by EPB_LOC() macro.
 * @wd: Write Data - value to set in register
 * @mask: ones where data should be spliced into reg.
 *
 * Basic register read/modify/write, with un-needed acesses elided. That is,
 * a mask of zero will prevent write, while a mask of 0xFF will prevent read.
 * returns current (presumed, if a write was done) contents of selected
 * register, or <0 if errors.
 */
static int ipath_sd7220_reg_mod(struct ipath_devdata *dd, int sdnum, u32 loc,
				u32 wd, u32 mask)
{
	u16 trans;
	u64 transval;
	int owned;
	int tries, ret;
	unsigned long flags;

	switch (sdnum) {
	case IB_7220_SERDES :
		trans = dd->ipath_kregs->kr_ib_epbtrans;
		break;
	case PCIE_SERDES0 :
	case PCIE_SERDES1 :
		trans = dd->ipath_kregs->kr_pcie_epbtrans;
		break;
	default :
		return -1;
	}

	/*
	 * All access is locked in software (vs other host threads) and
	 * hardware (vs uC access).
	 */
	spin_lock_irqsave(&dd->ipath_sdepb_lock, flags);

	owned = epb_access(dd, sdnum, 1);
	if (owned < 0) {
		spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
		return -1;
	}
	ret = 0;
	for (tries = EPB_TRANS_TRIES; tries; --tries) {
		transval = ipath_read_kreg32(dd, trans);
		if (transval & EPB_TRANS_RDY)
			break;
		udelay(5);
	}

	if (tries > 0) {
		tries = 1;	/* to make read-skip work */
		if (mask != 0xFF) {
			/*
			 * Not a pure write, so need to read.
			 * loc encodes chip-select as well as address
			 */
			transval = loc | EPB_RD;
			tries = epb_trans(dd, trans, transval, &transval);
		}
		if (tries > 0 && mask != 0) {
			/*
			 * Not a pure read, so need to write.
			 */
			wd = (wd & mask) | (transval & ~mask);
			transval = loc | (wd & EPB_DATA_MASK);
			tries = epb_trans(dd, trans, transval, &transval);
		}
	}
	/* else, failed to see ready, what error-handling? */

	/*
	 * Release bus. Failure is an error.
	 */
	if (epb_access(dd, sdnum, -1) < 0)
		ret = -1;
	else
		ret = transval & EPB_DATA_MASK;

	spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
	if (tries <= 0)
		ret = -1;
	return ret;
}

#define EPB_ROM_R (2)
#define EPB_ROM_W (1)
/*
 * Below, all uC-related, use appropriate UC_CS, depending
 * on which SerDes is used.
 */
#define EPB_UC_CTL EPB_LOC(6, 0, 0)
#define EPB_MADDRL EPB_LOC(6, 0, 2)
#define EPB_MADDRH EPB_LOC(6, 0, 3)
#define EPB_ROMDATA EPB_LOC(6, 0, 4)
#define EPB_RAMDATA EPB_LOC(6, 0, 5)

/* Transfer date to/from uC Program RAM of IB or PCIe SerDes */
static int ipath_sd7220_ram_xfer(struct ipath_devdaOINT(14)
#define EDRV_COUNT_RX_ERR               TGT_DBG_SIGNAL_TRACE_POINT(15)
#define EDRV_COUNT_RX_FOVW              TGT_DBG_SIGNAL_TRACE_POINT(16)
#define EDRV_COUNT_RX_PUN               TGT_DBG_SIGNAL_TRACE_POINT(17)
#define EDRV_COUNT_RX_FAE               TGT_DBG_SIGNAL_TRACE_POINT(18)
#define EDRV_COUNT_RX_OVW               TGT_DBG_SIGNAL_TRACE_POINT(19)

#define EDRV_TRACE_CAPR(x)              TGT_DBG_POST_TRACE_VALUE(((x) & 0xFFFF) | 0x06000000)
#define EDRV_TRACE_RX_CRC(x)            TGT_DBG_POST_TRACE_VALUE(((x) & 0xFFFF) | 0x0E000000)
#define EDRV_TRACE_RX_ERR(x)            TGT_DBG_POST_TRACE_VALUE(((x) & 0xFFFF) | 0x0F000000)
#define EDRV_TRACE_RX_PUN(x)            TGT_DBG_POST_TRACE_VALUE(((x) & 0xFFFF) | 0x11000000)
#define EDRV_TRACE(x)                   TGT_DBG_POST_TRACE_VALUE(((x) & 0xFFFF0000) | 0x0000FEC0)

//---------------------------------------------------------------------------
// local types
//---------------------------------------------------------------------------
/*
typedef struct
{
    BOOL            m_fUsed;
    unsigned int    m_uiSize;
    MCD_bufDescFec *m_pBufDescr;

} tEdrvTxBufferIntern;
*/

// Private structure
typedef struct {
	struct pci_dev *m_pPciDev;	// pointer to PCI device structure
	void *m_pIoAddr;	// pointer to register space of Ethernet controller
	u8 *m_pbRxBuf;	// pointer to Rx buffer
	dma_addr_t m_pRxBufDma;
	u8 *m_pbTxBuf;	// pointer to Tx buffer
	dma_addr_t m_pTxBufDma;
	BOOL m_afTxBufUsed[EDRV_MAX_TX_BUFFERS];
	unsigned int m_uiCurTxDesc;

	tEdrvInitParam m_InitParam;
	tEdrvTxBuffer *m_pLastTransmittedTxBuffer;

} tEdrvInstance;

//---------------------------------------------------------------------------
// local function prototypes
//---------------------------------------------------------------------------

static int EdrvInitOne(struct pci_dev *pPciDev,
		       const struct pci_device_id *pId);

static void EdrvRemoveOne(struct pci_dev *pPciDev);

//---------------------------------------------------------------------------
// modul globale vars
//---------------------------------------------------------------------------
// buffers and buffer descriptors and pointers

static struct pci_device_id aEdrvPciTbl[] = {
	{0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0,}
};

MODULE_DEVICE_TABLE(pci, aEdrvPciTbl);

static tEdrvInstance EdrvInstance_l;

static struct pci_driver EdrvDriver = {
	.name = DRV_NAME,
	.id_table = aEdrvPciTbl,
	.probe = EdrvInitOne,
	.remove = EdrvRemoveOne,
};

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*          C L A S S  <edrv>                                              */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
//
// Description:
//
//
/***************************************************************************/

//=========================================================================//
//                                                                         //
//          P R I V A T E   D E F I N I T I O N S                          //
//                                                                         //
//=========================================================================//

//---------------------------------------------------------------------------
// const defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// local types
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// local vars
//---------------------------------------------------------------------------

//---------------------------------------, sofar + offset,
					  readback, req, 1);
		if (cnt < req) {
			/* failed in read itself */
			sofar = -1;
			break;
		}
		for (idx = 0; idx < cnt; ++idx) {
			if (readback[idx] != img[idx+sofar])
				++errors;
		}
		sofar += cnt;
	}
	return errors ? -errors : sofar;
}

/* IRQ not set up at this point in init, so we poll. */
#define IB_SERDES_TRIM_DONE (1ULL << 11)
#define TRIM_TMO (30)

static int ipath_sd_trimdone_poll(struct ipath_devdata *dd)
{
	int trim_tmo, ret;
	uint64_t val;

	/*
	 * Default to failure, so IBC will not start
	 * without IB_SERDES_TRIM_DONE.
	 */
	ret = 0;
	for (trim_tmo = 0; trim_tmo < TRIM_TMO; ++trim_tmo) {
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
		if (val & IB_SERDES_TRIM_DONE) {
			ipath_cdbg(VERBOSE, "TRIMDONE after %d\n", trim_tmo);
			ret = 1;
			break;
		}
		msleep(10);
	}
	if (trim_tmo >= TRIM_TMO) {
		ipath_dev_err(dd, "No TRIMDONE in %d tries\n", trim_tmo);
		ret = 0;
	}
	return ret;
}

#define TX_FAST_ELT (9)

/*
 * Set the "negotiation" values for SERDES. These are used by the IB1.2
 * link negotiation. Macros below are attempt to keep the values a
 * little more human-editable.
 * First, values related to Drive De-emphasis Settings.
 */

#define NUM_DDS_REGS 6
#define DDS_REG_MAP 0x76A910 /* LSB-first list of regs (in elt 9) to mod */

#define DDS_VAL(amp_d, main_d, ipst_d, ipre_d, amp_s, main_s, ipst_s, ipre_s) \
	{ { ((amp_d & 0x1F) << 1) | 1, ((amp_s & 0x1F) << 1) | 1, \
	  (main_d << 3) | 4 | (ipre_d >> 2), \
	  (main_s << 3) | 4 | (ipre_s >> 2), \
	  ((ipst_d & 0xF) << 1) | ((ipre_d & 3) << 6) | 0x21, \
	  ((ipst_s & 0xF) << 1) | ((ipre_s & 3) << 6) | 0x21 } }

static struct dds_init {
	uint8_t reg_vals[NUM_DDS_REGS];
} dds_init_vals[] = {
	/*       DDR(FDR)       SDR(HDR)   */
	/* Vendor recommends below for 3m cable */
#define DDS_3M 0
	DDS_VAL(31, 19, 12, 0, 29, 22,  9, 0),
	DDS_VAL(31, 12, 15, 4, 31, 15, 15, 1),
	DDS_VAL(31, 13, 15, 3, 31, 16, 15, 0),
	DDS_VAL(31, 14, 15, 2, 31, 17, 14, 0),
	DDS_VAL(31, 15, 15, 1, 31, 18, 13, 0),
	DDS_VAL(31, 16, 15, 0, 31, 19, 12, 0),
	DDS_VAL(31, 17, 14, 0, 31, 20, 11, 0),
	DDS_VAL(31, 18, 13, 0, 30, 21, 10, 0),
	DDS_VAL(31, 20, 11, 0, 28, 23,  8, 0),
	DDS_VAL(31, 21, 10, 0, 27, 24,  7, 0),
	DDS_VAL(31, 22,  9, 0, 26, 25,  6, 0),
	DDS_VAL(30, 23,  8, 0, 25, 26,  5, 0),
	DDS_VAL(29, 24,  7, 0, 23, 27,  4, 0),
	/* Vendor recommends below for 1m cable */
#define DDS_1M 13
	DDS_VAL(28, 25,  6, 0, 21, 28,  3, 0),
	DDS_VAL(27, 26,  5, 0, 19, 29,  2, 0),
	DDS_VAL(25, 27,  4, 0, 17, 30,  1, 0)
};

/*
 * Next, values related to Receive Equalization.
 * In comments, FDR (Full) is IB DDR, HDR (Half) is IB SDR
 */
/* Hardware packs an element number and register address thus: */
#define RXEQ_INIT_RDESC(elt, addr) (((elt) & 0xF) | ((addr) << 4))
#define RXEQ_VAL(elt, adr, val0, val1, val2, val3) \
	{RXEQ_INIT_RDESC((elt), (adr)), {(val0), (val1), (val2), (val3)} }

#define RXEQ_VAL_ALL(elt, adr, val)  \
	{RXEQ_INIT_RDESC((elt), (adr)), {(val), (val), (val), (val)} }

#define RXEQ_SDR_DFELTH 0
#define RXEQ_SDR_TLTH 0
#define RXEQ_SDR_G1CNT_Z1CNT 0x11
#define RXEQ_SDR_ZCNT 23

static struct rxeq_init {
	u16 rdesc;	/* in form used in SerDesDDSRXEQ */
	u8  rdata[4];
} rxeq_init_vals[] = {
	/* Set Rcv Eq. to Preset node */
	RXEQ_VAL_ALL(7, 0x27, 0x10),
	/* Set DFELTHFDR/HDR thresholds */
	RXEQ_VAL(7, 8,    0, 0, 0, 0), /* FDR */
	RXEQ_VAL(7, 0x21, 0, 0, 0, 0), /* HDR */
	/* Set TLTHFDR/HDR theshold */
	RXEQ_VAL(7, 9,    2, 2, 2, 2), /* FDR */
	RXEQ_VAL(7, 0x23, 2, 2, 2, 2), /* HDR */
	/* Set Preamp setting 2 (ZFR/ZCNT) */
	RXEQ_VAL(7, 0x1B, 12, 12, 12, 12), /* FDR */
	RXEQ_VAL(7, 0x1C, 12, 12, 12, 12), /* HDR */
	/* Set Preamp DC gain and Setting 1 (GFR/GHR) */
	RXEQ_VAL(7, 0x1E, 0x10, 0x10, 0x10, 0x10), /* FDR */
	RXEQ_VAL(7, 0x1F, 0x10, 0x10, 0x10, 0x10), /* HDR */
	/* Toggle RELOCK (in VCDL_CTRL0) to lock to data */
	RXEQ_VAL_ALL(6, 6, 0x20), /* Set D5 High */
	RXEQ_VAL_ALL(6, 6, 0), /* Set D5 Low */
};

/* There are 17 values from vendor, but IBC only accesses the first 16 */
#define DDS_ROWS (16)
#define RXEQ_ROWS ARRAY_SIZE(rxeq_init_vals)

static int ipath_sd_setvals(struct ipath_devdata *dd)
{
	int idx, midx;
	int min_idx;	 /* Minimum index for this portion of table */
	uint32_t dds_reg_map;
	u64 __iomem *taddr, *iaddr;
	uint64_t data;
	uint64_t sdctl;

	taddr = dd->ipath_kregbase + KR_IBSerDesMappTable;
	iaddr = dd->ipath_kregbase + dd->ipath_kregs->kr_ib_ddsrxeq;

	/*
	 * Init the DDS section of the table.
	 * Each "row" of the table provokes NUM_DDS_REG writes, to the
	 * registers indicated in DDS_REG_MAP.
	 */
	sdctl = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibserdesctrl);
	sdctl = (sdctl & ~(0x1f << 8)) | (NUM_DDS_REGS << 8);
	sdctl = (sdctl & ~(0x1f << 13)) | (RXEQ_ROWS << 13);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_ibserdesctrl, sdctl);

	/*
	 * Iterate down table within loop for each register to store.
	 */
	dds_reg_map = DDS_REG_MAP;
	for (idx = 0; idx < NUM_DDS_REGS; ++idx) {
		data = ((dds_reg_map & 0xF) << 4) | TX_FAST_ELT;
		writeq(data, iaddr + idx);
		mmiowb();
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		dds_reg_map >>= 4;
		for (midx = 0; midx < DDS_ROWS; ++midx) {
			u64 __iomem *daddr = taddr + ((midx << 4) + idx);
			data = dds_init_vals[midx].reg_vals[idx];
			writeq(data, daddr);
			mmiowb();
			ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		} /* End inner for (vals for this reg, each row) */
	} /* end outer for (regs to be stored) */

	/*
	 * Init the RXEQ section of the table. As explained above the table
	 * rxeq_init_vals[], this runs in a different order, as the pattern
	 * of register references is more complex, but there are only
	 * four "data" values per register.
	 */
	min_idx = idx; /* RXEQ indices pick up where DDS left off */
	taddr += 0x100; /* RXEQ data is in second half of table */
	/* Iterate through RXEQ register addresses */
	for (idx = 0; idx < RXEQ_ROWS; ++idx) {
		int didx; /* "destination" */
		int vidx;

		/* didx is offset by min_idx to address RXEQ range of regs */
		didx = idx + min_idx;
		/* Store the next RXEQ register address */
		writeq(rxeq_init_vals[idx].rdesc, iaddr + didx);
		mmiowb();
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		/* Iterate through RXEQ values */
		for (vidx = 0; vidx < 4; vidx++) {
			data = rxeq_init_vals[idx].rdata[vidx];
			writeq(data, taddr + (vidx << 6) + idx);
			mmiowb();
			ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		}
	} /* end outer for (Reg-writes for RXEQ) */
	return 0;
}

#define CMUCTRL5 EPB_LOC(7, 0, 0x15)
#define RXHSCTRL0(chan) EPB_LOC(chan, 6, 0)
#define VCDL_DAC2(chan) EPB_LOC(chan, 6, 5)
#define VCDL_CTRL0(chan) EPB_LOC(chan, 6, 6)
#define VCDL_CTRL2(chan) EPB_LOC(chan, 6, 8)
#define START_EQ2(chan) EPB_LOC(chan, 7, 0x28)

static int ibsd_sto_noisy(struct ipath_devdata *dd, int loc, int val, int mask)
{
	int ret = -1;
	int sloc; /* shifted loc, for messages */

	loc |= (1U << EPB_IB_QUAD0_CS_SHF);
	sloc = loc >> EPB_ADDR_SHF;

	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, val, mask);
	if (ret < 0)
		ipath_dev_err(dd, "Write failed: elt %d,"
			" addr 0x%X, chnl %d, val 0x%02X, mask 0x%02X\n",
			(sloc & 0xF), (sloc >> 9) & 0x3f, (sloc >> 4) & 7,
			val & 0xFF, mask & 0xFF);
	return ret;
}

/*
 * Repeat a "store" across all channels of the IB SerDes.
 * Although nominally it inherits the "read value" of the last
 * channel it modified, the only really useful return is <0 for
 * failure, >= 0 for success. The parameter 'loc' is assumed to
 * be the location for the channel-0 copy of the register to
 * be modified.
 */
static int ibsd_mod_allchnls(struct ipath_devdata *dd, int loc, int val,
	int mask)
{
	int ret = -1;
	int chnl;

	if (loc & EPB_GLOBAL_WR) {
		/*
		 * Our caller has assured us that we can set all four
		 * channels at once. Trust that. If mask is not 0xFF,
		 * we will read the _specified_ channel for our starting
		 * value.
		 */
		loc |= (1U << EPB_IB_QUAD0_CS_SHF);
		chnl = (loc >> (4 + EPB_ADDR_SHF)) & 7;
		if (mask != 0xFF) {
			ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				loc & ~EPB_GLOBAL_WR, 0, 0);
			if (ret < 0) {
				int sloc = loc >> EPB_ADDR_SHF;
				ipath_dev_err(dd, "pre-read failed: elt %d,"
					" addr 0x%X, chnl %d\n", (sloc & 0xF),
					(sloc >> 9) & 0x3f, chnl);
				return ret;
			}
			val = (ret & ~mask) | (val & mask);
		}
		loc &=  ~(7 << (4+EPB_ADDR_SHF));
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, val, 0xFF);
		if (ret < 0) {
			int sloc = loc >> EPB_ADDR_SHF;
			ipath_dev_err(dd, "Global WR failed: elt %d,"
				" addr 0x%X, val %02X\n",
				(sloc & 0xF), (sloc >> 9) & 0x3f, val);
		}
		return ret;
	}
	/* Clear "channel" and set CS so we can simply iterate */
	loc &=  ~(7 << (4+EPB_ADDR_SHF));
	loc |= (1U << EPB_IB_QUAD0_CS_SHF);
	for (chnl = 0; chnl < 4; ++chnl) {
		int cloc;
		cloc = loc | (chnl << (4+EPB_ADDR_SHF));
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, cloc, val, mask);
		if (ret < 0) {
			int sloc = loc >> EPB_ADDR_SHF;
			ipath_dev_err(dd, "Write failed: elt %d,"
				" addr 0x%X, chnl %d, val 0x%02X,"
				" mask 0x%02X\n",
				(sloc & 0xF), (sloc >> 9) & 0x3f, chnl,
				val & 0xFF, mask & 0xFF);
			break;
		}
	}
	return ret;
}

/*
 * Set the Tx values normally modified by IBC in IB1.2 mode to default
 * values, as gotten from first row of init table.
 */
static int set_dds_vals(struct ipath_devdata *dd, struct dds_init *ddi)
{
	int ret;
	int idx, reg, data;
	uint32_t regmap;

	regmap = DDS_REG_MAP;
	for (idx = 0; idx < NUM_DDS_REGS; ++idx) {
		reg = (regmap & 0xF);
		regmap >>= 4;
		data = ddi->reg_vals[idx];
		/* Vendor says RMW not needed for these regs, use 0xFF mask */
		ret = ibsd_mod_allchnls(dd, EPB_LOC(0, 9, reg), data, 0xFF);
		if (ret < 0)
			break;
	}
	return ret;
}

/*
 * Set the Rx values normally modified by IBC in IB1.2 mode to default
 * values, as gotten from selected column of init table.
 */
static int set_rxeq_vals(struct ipath_devdata *dd, int vsel)
{
	int ret;
	int ridx;
	int cnt = ARRAY_SIZE(rxeq_init_vals);

	for (ridx = 0; ridx < cnt; ++ridx) {
		int elt, reg, val, loc;
		elt = rxeq_init_vals[ridx].rdesc & 0xF;
		reg = rxeq_init_vals[ridx].rdesc >> 4;
		loc = EPB_LOC(0, elt, reg);
		val = rxeq_init_vals[ridx].rdata[vsel];
		/* mask of 0xFF, because hardware does full-byte store. */
		ret = ibsd_mod_allchnls(dd, loc, val, 0xFF);
		if (ret < 0)
			break;
	}
	return ret;
}

/*
 * Set the default values (row 0) for DDR Driver Demphasis.
 * we do this initially and whenever we turn off IB-1.2
 * The "default" values for Rx equalization are also stored to
 * SerDes registers. Formerly (and still default), we used set 2.
 * For experimenting with cables and link-partners, we allow changing
 * that via a module parameter.
 */
static unsigned ipath_rxeq_set = 2;
module_param_named(rxeq_default_set, ipath_rxeq_set, uint,
	S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(rxeq_default_set,
	"Which set [0..3] of Rx Equalization values is default");

static int ipath_internal_presets(struct ipath_devdata *dd)
{
	int ret = 0;

	ret = set_dds_vals(dd, dds_init_vals + DDS_3M);

	if (ret < 0)
		ipath_dev_err(dd, "Failed to set default DDS values\n");
	ret = set_rxeq_vals(dd, ipath_rxeq_set & 3);
	if (ret < 0)
		ipath_dev_err(dd, "Failed to set default RXEQ values\n");
	return ret;
}

int ipath_sd7220_presets(struct ipath_devdata *dd)
{
	int ret = 0;

	if (!dd->ipath_presets_needed)
		return ret;
	dd->ipath_presets_needed = 0;
	/* Assert uC reset, so we don't clash with it. */
	ipath_ibsd_reset(dd, 1);
	udelay(2);
	ipath_sd_trimdone_monitor(dd, "link-down");

	ret = ipath_internal_presets(dd);
return ret;
}

static int ipath_sd_trimself(struct ipath_devdata *dd, int val)
{
	return ibsd_sto_noisy(dd, CMUCTRL5, val, 0xFF);
}

static int ipath_sd_early(struct ipath_devdata *dd)
{
	int ret = -1; /* Default failed */
	int chnl;

	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, RXHSCTRL0(chnl), 0xD4, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, VCDL_DAC2(chnl), 0x2D, 0xFF);
		if (ret < 0)
			goto bail;
	}
	/* more fine-tuning of what will be default */
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, VCDL_CTRL2(chnl), 3, 0xF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, START_EQ1(chnl), 0x10, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, START_EQ2(chnl), 0x30, 0xFF);
		if (ret < 0)
			goto bail;
	}
bail:
	return ret;
}

#define BACTRL(chnl) EPB_LOC(chnl, 6, 0x0E)
#define LDOUTCTRL1(chnl) EPB_LOC(chnl, 7, 6)
#define RXHSSTATUS(chnl) EPB_LOC(chnl, 6, 0xF)

static int ipath_sd_dactrim(struct ipath_devdata *dd)
{
	int ret = -1; /* Default failed */
	int chnl;

	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, BACTRL(chnl), 0x40, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, LDOUTCTRL1(chnl), 0x04, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, RXHSSTATUS(chnl), 0x04, 0xFF);
		if (ret < 0)
			goto bail;
	}
	/*
	 * delay for max possible number of steps, with slop.
	 * Each step is about 4usec.
	 */
	udelay(415);
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, LDOUTCTRL1(chnl), 0x00, 0xFF);
		if (ret < 0)
			goto bail;
	}
bail:
	return ret;
}

#define RELOCK_FIRST_MS 3
#define RXLSPPM(chan) EPB_LOC(chan, 0, 2)
void ipath_toggle_rclkrls(struct ipath_devdata *dd)
{
	int loc = RXLSPPM(0) | EPB_GLOBAL_WR;
	int ret;

	ret = ibsd_mod_allchnls(dd, loc, 0, 0x80);
	if (ret < 0)
		ipath_dev_err(dd, "RCLKRLS failed to clear D7\n");
	else {
		udelay(1);
		ibsd_mod_allchnls(dd, loc, 0x80, 0x80);
	}
	/* And again for good measure */
	udelay(1);
	ret = ibsd_mod_allchnls(dd, loc, 0, 0x80);
	if (ret < 0)
		ipath_dev_err(dd, "RCLKRLS failed to clear D7\n");
	else {
		udelay(1);
		ibsd_mod_allchnls(dd, loc, 0x80, 0x80);
	}
	/* Now reset xgxs and IBC to complete the recovery */
	dd->ipath_f_xgxs_reset(dd);
}

/*
 * Shut down the timer that polls for relock occasions, if needed
 * this is "hooked" from ipath_7220_quiet_serdes(), which is called
 * just before ipath_shutdown_device() in ipath_driver.c shuts down all
 * the other timers
 */
void ipath_shutdown_relock_poll(struct ipath_devdata *dd)
{
	struct ipath_relock *irp = &dd->ipath_relock_singleton;
	if (atomic_read(&irp->ipath_relock_timer_active)) {
		del_timer_sync(&irp->ipath_relock_timer);
		atomic_set(&irp->ipath_relock_timer_active, 0);
	}
}

static unsigned ipath_relock_by_timer = 1;
module_param_named(relock_by_timer, ipath_relock_by_timer, uint,
	S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(relock_by_timer, "Allow relock attempt if link not up");

static void ipath_run_relock(unsigned long opaque)
{
	struct ipath_devdata *dd = (struct ipath_devdata *)opaque;
	struct ipath_relock *irp = &dd->ipath_relock_singleton;
	u64 val, ltstate;

	if (!(dd->ipath_flags & IPATH_INITTED)) {
		/* Not yet up, just reenable the timer for later */
		irp->ipath_relock_interval = HZ;
		mod_timer(&irp->ipath_relock_timer, jiffies + HZ);
		return;
	}

	/*
	 * Check link-training state for "stuck" state.
	 * if found, try relock and schedule another try at
	 * exponentially growing delay, maxed at one second.
	 * if not stuck, our work is done.
	 */
	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
	ltstate = ipath_ib_linktrstate(dd, val);

	if (ltstate <= INFINIPATH_IBCS_LT_STATE_CFGWAITRMT
		&& ltstate != INFINIPATH_IBCS_LT_STATE_LINKUP) {
		int timeoff;
		/* Not up yet. Try again, if allowed by module-param */
		if (ipath_relock_by_timer) {
			if (dd->ipath_flags & IPATH_IB_AUTONEG_INPROG)
				ipath_cdbg(VERBOSE, "Skip RELOCK in AUTONEG\n");
			else if (!(dd->ipath_flags & IPATH_IB_LINK_DISABLED)) {
				ipath_cdbg(VERBOSE, "RELOCK\n");
				ipath_toggle_rclkrls(dd);
			}
		}
		/* re-set timer for next check */
		timeoff = irp->ipath_relock_interval << 1;
		if (timeoff > HZ)
			timeoff = HZ;
		irp->ipath_relock_interval = timeoff;

		mod_timer(&irp->ipath_relock_timer, jiffies + timeoff);
	} else {
		/* Up, so no more need to check so often */
		mod_timer(&irp->ipath_relock_timer, jiffies + HZ);
	}
}

void ipath_set_relock_poll(struct ipath_devdata *dd, int ibup)
{
	struct ipath_relock *irp = &dd->ipath_relock_singleton;

	if (ibup > 0) {
		/* we are now up, so relax timer to 1 second interval */
		if (atomic_read(&irp->ipath_relock_timer_active))
			mod_timer(&irp->ipath_relock_timer, jiffies + HZ);
	} else {
		/* Transition to down, (re-)set timer to short interval. */
		int timeout;
		timeout = (HZ * ((ibup == -1) ? 1000 : RELOCK_FIRST_MS))/1000;
		if (timeout == 0)
			timeout = 1;
		/* If timer has not yet been started, do so. */
		if (atomic_inc_return(&irp->ipath_relock_timer_active) == 1) {
			init_timer(&irp->ipath_relock_timer);
			irp->ipath_relock_timer.function = ipath_run_relock;
			irp->ipath_relock_timer.data = (unsigned long) dd;
			irp->ipath_relock_interval = timeout;
			irp->ipath_relock_timer.expires = jiffies + timeout;
			add_timer(&irp->ipath_relock_timer);
		} else {
			irp->ipath_relock_interval = timeout;
			mod_timer(&irp->ipath_relock_timer, jiffies + timeout);
			atomic_dec(&irp->ipath_relock_timer_active);
		}
	}
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    K 25
svn:wc:ra_dav:version-url
V 73
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/video/vermilion
END
vermilion.c
K 25
svn:wc:ra_dav:version-url
V 85
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/video/vermilion/vermilion.c
END
vermilion.h
K 25
svn:wc:ra_dav:version-url
V 85
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/video/vermilion/vermilion.h
END
cr_pll.c
K 25
svn:wc:ra_dav:version-url
V 82
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/video/vermilion/cr_pll.c
END
Makefile
K 25
svn:wc:ra_dav:version-url
V 82
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/video/vermilion/Makefile
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        