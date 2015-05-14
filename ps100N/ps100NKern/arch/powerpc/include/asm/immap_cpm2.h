se
		CAM_INVALIDATE(config->config_table[1]);
	config->config_table[1].target_table_entry.client_id = 0;
	config->config_table[1].target_table_entry.vlan_id = 0;

	bnx2x_sp_post(bp, RAMROD_CMD_ID_ETH_SET_MAC, 0,
		      U64_HI(bnx2x_sp_mapping(bp, mac_config)),
		      U64_LO(bnx2x_sp_mapping(bp, mac_config)), 0);
}

static void bnx2x_set_mac_addr_e1h(struct bnx2x *bp, int set)
{
	struct mac_configuration_cmd_e1h *config =
		(struct mac_configuration_cmd_e1h *)bnx2x_sp(bp, mac_config);

	if (set && (bp->state != BNX2X_STATE_OPEN)) {
		DP(NETIF_MSG_IFUP, "state is %x, returning\n", bp->state);
		return;
	}

	/* CAM allocation for E1H
	 * unicasts: by func number
	 * multicast: 20+FUNC*20, 20 each
	 */
	config->hdr.length = 1;
	config->hdr.offset = BP_FUNC(bp);
	config->hdr.client_id = bp->fp->cl_id;
	config->hdr.reserved1 = 0;

	/* primary MAC */
	config->config_table[0].msb_mac_addr =
					swab16(*(u16 *)&bp->dev->dev_addr[0]);
	config->config_table[0].middle_mac_addr =
					swab16(*(u16 *)&bp->dev->dev_addr[2]);
	config->config_table[0].lsb_mac_addr =
					swab16(*(u16 *)&bp->dev->dev_addr[4]);
	config->config_table[0].client_id = BP_L_ID(bp);
	config->config_table[0].vlan_id = 0;
	config->config_table[0].e1hov_id = cpu_to_le16(bp->e1hov);
	if (set)
		config->config_table[0].flags = BP_PORT(bp);
	else
		config->config_table[0].flags =
				MAC_CONFIGURATION_ENTRY_E1H_ACTION_TYPE;

	DP(NETIF_MSG_IFUP, "%s MAC (%04x:%04x:%04x)  E1HOV %d  CLID %d\n",
	   (set ? "setting" : "clearing"),
	   config->config_table[0].msb_mac_addr,
	   config->config_table[0].middle_mac_addr,
	   config->config_table[0].lsb_mac_addr, bp->e1hov, BP_L_ID(bp));

	bnx2x_sp_post(bp, RAMROD_CMD_ID_ETH_SET_MAC, 0,
		      U64_HI(bnx2x_sp_mapping(bp, mac_config)),
		      U64_LO(bnx2x_sp_mapping(bp, mac_config)), 0);
}

static int bnx2x_wait_ramrod(struct bnx2x *bp, int state, int idx,
			     int *state_p, int poll)
{
	/* can take a while if any port is running */
	int cnt = 5000;

	DP(NETIF_MSG_IFUP, "%s for state to become %x on IDX [%d]\n",
	   poll ? "polling" : "waiting", state, idx);

	might_sleep();
	while (cnt--) {
		if (poll) {
			bnx2x_rx_int(bp->fp, 10);
			/* if index is different from 0
			 * the reply for some commands will
			 * be on the non default queue
			 */
			if (idx)
				bnx2x_rx_int(&bp->fp[idx], 10);
		}

		mb(); /* state is changed by bnx2x_sp_event() */
		if (*state_p == state) {
#ifdef BNX2X_STOP_ON_ERROR
			DP(NETIF_MSG_IFUP, "exit  (cnt %d)\n", 5000 - cnt);
#endif
			return 0;
		}

		msleep(1);
	}

	/* timeout! */
	BNX2X_ERR("timeout %s for state %x on IDX [%d]\n",
		  poll ? "polling" : "waiting", state, idx);
#ifdef BNX2X_STOP_ON_ERROR
	bnx2x_panic();
#endif

	return -EBUSY;
}

static int bnx2x_setup_leading(struct bnx2x *bp)
{
	int rc;

	/* reset IGU state */
	bnx2x_ack_sb(bp, bp->fp[0].sb_id, CSTORM_ID, 0, IGU_INT_ENABLE, 0);

	/* SETUP ramrod */
	bnx2x_sp_post(bp, RAMROD_CMD_ID_ETH_PORT_SETUP, 0, 0, 0, 0);

	/* Wait for completion */
	rc = bnx2x_wait_ramrod(bp, BNX2X_STATE_OPEN, 0, &(bp->state), 0);

	return rc;
}

static int bnx2x_setup_multi(struct bnx2x *bp, int index)
{
	struct bnx2x_fastpath *fp = &bp->fp[index];

	/* reset IGU state */
	bnx2x_ack_sb(bp, fp->sb_id, CSTORM_ID, 0, IGU_INT_ENABLE, 0);

	/* SETUP ramrod */
	fp->state = BNX2X_FP_STATE_OPENING;
	bnx2x_sp_post(bp, RAMROD_CMD_ID_ETH_CLIENT_SETUP, index, 0,
		      fp->cl_id, 0);

	/* Wait for completion */
	return bnx2x_wait_ramrod(bp, BNX2X_FP_STATE_OPEN, index,
				 &(fp->state), 0);
}

static int bnx2x_poll(struct napi_struct *napi, int budget);

static void bnx2x_set_int_mode(struct bnx2x *bp)
{
	int num_queues;

	switch (int_mode) {
	case INT_MODE_INTx:
	case INT_MODE_MSI:
		num_queues = 1;
		bp->num_rx_queues = num_queues;
		bp->num_tx_queues = num_queues;
		DP(NETIF_MSG_IFUP,
		   "set number of queues to %d\n", num_queues);
		break;

	case INT_MODE_MSIX:
	default:
		if (bp->multi_mode == ETH_RSS_MODE_REGULAR)
			num_queues = min_t(u32, num_online_cpus(),
					   BNX2X_MAX_QUEUES(bp));
		else
			num_queues = 1;
		bp->num_rx_queues = num_queues;
		bp->num_tx_queues = num_queues;
		DP(NETIF_MSG_IFUP, "set number of rx queues to %d"
		   "  number of tx queues to %d\n",
		   bp->num_rx_queues, bp->num_tx_queues);
		/* if we can't use MSI-X we only need one fp,
		 * so try to enable MSI-X with the requested number of fp's
		 * and fallback to MSI or legacy INTx with one fp
		 */
		if (bnx2x_enable_msix(bp)) {
			/* failed to enable MSI-X */
			num_queues = 1;
			bp->num_rx_queues = num_queues;
			bp->num_tx_queues = num_queues;
			if (bp->multi_mode)
				BNX2X_ERR("Multi requested but failed to "
					  "enable MSI-X  set number of "
					  "queues to %d\n", num_queues);
		}
		break;
	}
	bp->dev->real_num_tx_queues = bp->num_tx_queues;
}

static void bnx2x_set_rx_mode(struct net_device *dev);

/* must be called with rtnl_lock */
static int bnx2x_nic_load(struct bnx2x *bp, int load_mode)
{
	u32 load_code;
	int i, rc = 0;
#ifdef BNX2X_STOP_ON_ERROR
	DP(NETIF_MSG_IFUP, "enter  load_mode %d\n", load_mode);
	if (unlikely(bp->panic))
		return -EPERM;
#endif

	bp->state = BNX2X_STATE_OPENING_WAIT4_LOAD;

	bnx2x_set_int_mode(bp);

	if (bnx2x_alloc_mem(bp))
		return -ENOMEM;

	for_each_rx_queue(bp, i)
		bnx2x_fp(bp, i, disable_tpa) =
					((bp->flags & TPA_ENABLE_FLAG) == 0);

	for_each_rx_queue(bp, i)
		netif_napi_add(bp->dev, &bnx2x_fp(bp, i, napi),
			       bnx2x_poll, 128);

#ifdef BNX2X_STOP_ON_ERROR
	for_each_rx_queue(bp, i) {
		struct bnx2x_fastpath *fp = &bp->fp[i];

		fp->poll_no_work = 0;
		fp->poll_calls = 0;
		fp->poll_max_calls = 0;
		fp->poll_complete = 0;
		fp->poll_exit = 0;
	}
#endif
	bnx2x_napi_enable(bp);

	if (bp->flags & USING_MSIX_FLAG) {
		rc = bnx2x_req_msix_irqs(bp);
		if (rc) {
			pci_disable_msix(bp->pdev);
			goto load_error1;
		}
	} else {
		if ((rc != -ENOMEM) && (int_mode != INT_MODE_INTx))
			bnx2x_enable_msi(bp);
		bnx2x_ack_int(bp);
		rc = bnx2x_req_irq(bp);
		if (rc) {
			BNX2X_ERR("IRQ request failed  rc %d, aborting\n", rc);
			if (bp->flags & USING_MSI_FLAG)
				pci_disable_msi(bp->pdev);
			goto load_error1;
		}
		if (bp->flags & USING_MSI_FLAG) {
			bp->dev->irq = bp->pdev->irq;
			printk(KERN_INFO PFX "%s: using MSI  IRQ %d\n",
			       bp->dev->name, bp->pdev->irq);
		}
	}

	/* Send LOAD_REQUEST command to MCP
	   Returns the type of LOAD command:
	   if it is the first port to be initialized
	   common blocks should be initialized, otherwise - not
	*/
	if (!BP_NOMCP(bp)) {
		load_code = bnx2x_fw_command(bp, DRV_MSG_CODE_LOAD_REQ);
		if (!load_code) {
			BNX2X_ERR("MCP response failure, aborting\n");
			rc = -EBUSY;
			goto load_error2;
		}
		if (load_code == FW_MSG_CODE_DRV_LOAD_REFUSED) {
			rc = -EBUSY; /* other port in diagnostic mode */
			goto load_error2;
		}

	} else {
		int port = BP_PORT(bp);

		DP(NETIF_MSG_IFUP, "NO MCP - load counts      %d, %d, %d\n",
		   load_count[0], load_count[1], load_count[2]);
		load_count[0]++;
		load_count[1 + port]++;
		DP(NETIF_MSG_IFUP, "NO MCP - new load counts  %d, %d, %d\n",
		   load_count[0], load_count[1], load_count[2]);
		if (load_count[0] == 1)
			load_code = FW_MSG_CODE_DRV_LOAD_COMMON;
		else if (load_count[1 + port] == 1)
			load_code = FW_MSG_CODE_DRV_LOAD_PORT;
		else
			load_code = FW_MSG_CODE_DRV_LOAD_FUNCTION;
	}

	if ((load_code == FW_MSG_CODE_DRV_LOAD_COMMON) ||
	    (load_code == FW_MSG_CODE_DRV_LOAD_PORT))
		bp->port.pmf = 1;
	else
		bp->port.pmf = 0;
	DP(NETIF_MSG_LINK, "pmf %d\n", bp->port.pmf);

	/* Initialize HW */
	rc = bnx2x_init_hw(bp, load_code);
	if (rc) {
		BNX2X_ERR("HW init failed, aborting\n");
		goto load_error2;
	}

	/* Setup NIC internals and enable interrupts */
	bnx2x_nic_init(bp, load_code);

	/* Send LOAD_DONE command to MCP */
	if (!BP_NOMCP(bp)) {
		load_code = bnx2x_fw_command(bp, DRV_MSG_CODE_LOAD_DONE);
		if (!load_code) {
			BNX2X_ERR("MCP response failure, aborting\n");
			rc = -EBUSY;
			goto load_error3;
		}
	}

	bp->state = BNX2X_STATE_OPENING_WAIT4_PORT;

	rc = bnx2x_setup_leading(bp);
	if (rc) {
		BNX2X_ERR("Setup leading failed!\n");
		goto load_error3;
	}

	if (CHIP_IS_E1H(bp))
		if (bp->mf_config & FUNC_MF_CFG_FUNC_DISABLED) {
			DP(N {
	u16	mcc_mcce;
	u8	res1[2];
	u16	mcc_mccm;
	u8	res2[2];
	u8	mcc_mccf;
	u8	res3[7];
} mcc_t;

typedef struct comm_proc {
	u32	cp_cpcr;
	u32	cp_rccr;
	u8	res1[14];
	u16	cp_rter;
	u8	res2[2];
	u16	cp_rtmr;
	u16	cp_rtscr;
	u8	res3[2];
	u32	cp_rtsr;
	u8	res4[12];
} cpm_cpm2_t;

/* USB Controller.
*/
typedef struct usb_ctlr {
	u8	usb_usmod;
	u8	usb_usadr;
	u8	usb_uscom;
	u8	res1[1];
	__be16  usb_usep[4];
	u8	res2[4];
	__be16  usb_usber;
	u8	res3[2];
	__be16  usb_usbmr;
	u8	usb_usbs;
	u8	res4[7];
} usb_cpm2_t;

/* ...and the whole thing wrapped up....
*/

typedef struct immap {
	/* Some references are into the unique and known dpram spaces,
	 * others are from the generic base.
	 */
#define im_dprambase	im_dpram1
	u8		im_dpram1[16*1024];
	u8		res1[16*1024];
	u8		im_dpram2[4*1024];
	u8		res2[8*1024];
	u8		im_dpram3[4*1024];
	u8		res3[16*1024];

	sysconf_cpm2_t	im_siu_conf;	/* SIU Configuration */
	memctl_cpm2_t	im_memctl;	/* Memory Controller */
	sit_cpm2_t	im_sit;		/* System Integration Timers */
	pci_cpm2_t	im_pci;		/* PCI Controller */
	intctl_cpm2_t	im_intctl;	/* Interrupt Controller */
	car_cpm2_t	im_clkrst;	/* Clocks and reset */
	iop_cpm2_t	im_ioport;	/* IO Port control/status */
	cpmtimer_cpm2_t	im_cpmtimer;	/* CPM timers */
	sdma_cpm2_t	im_sdma;	/* SDMA control/status */

	fcc_t		im_fcc[3];	/* Three FCCs */
	u8		res4z[32];
	fcc_c_t		im_fcc_c[3];	/* Continued FCCs */

	u8		res4[32];

	tclayer_t	im_tclayer[8];	/* Eight TCLayers */
	u16		tc_tcgsr;
	u16		tc_tcger;

	/* First set of baud rate generators.
	*/
	u8		res[236];
	u32		im_brgc5;
	u32		im_brgc6;
	u32		im_brgc7;
	u32		im_brgc8;

	u8		res5[608];

	i2c_cpm2_t	im_i2c;		/* I2C control/status */
	cpm_cpm2_t	im_cpm;		/* Communication processor */

	/* Second set of baud rate generators.
	*/
	u32		im_brgc1;
	u32		im_brgc2;
	u32		im_brgc3;
	u32		im_brgc4;

	scc_t		im_scc[4];	/* Four SCCs */
	smc_t		im_smc[2];	/* Couple of SMCs */
	spictl_cpm2_t	im_spi;		/* A SPI */
	cpmux_t		im_cpmux;	/* CPM clock route mux */
	siramctl_t	im_siramctl1;	/* First SI RAM Control */
	mcc_t		im_mcc1;	/* First MCC */
	siramctl_t	im_siramctl2;	/* Second SI RAM Control */
	mcc_t		im_mcc2;	/* Second MCC */
	usb_cpm2_t	im_usb;		/* USB Controller */

	u8		res6[1153];

	u16		im_si1txram[256];
	u8		res7[512];
	u16		im_si1rxram[256];
	u8		res8[512];
	u16		im_si2txram[256];
	u8		res9[512];
	u16		im_si2rxram[256];
	u8		res10[512];
	u8		res11[4096];
} cpm2_map_t;

extern cpm2_map_t __iomem *cpm2_immr;

#endif /* __IMMAP_CPM2__ */
#endif /* __KERNEL__ */
