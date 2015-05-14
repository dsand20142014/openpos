	uint64_t cfg_soft_wwnn;
	uint64_t cfg_soft_wwpn;
	uint32_t cfg_hba_queue_depth;
	uint32_t cfg_enable_hba_reset;
	uint32_t cfg_enable_hba_heartbeat;
	uint32_t cfg_enable_bg;
	uint32_t cfg_enable_fip;
	uint32_t cfg_log_verbose;

	lpfc_vpd_t vpd;		/* vital product data */

	struct pci_dev *pcidev;
	struct list_head      work_list;
	uint32_t              work_ha;      /* Host Attention Bits for WT */
	uint32_t              work_ha_mask; /* HA Bits owned by WT        */
	uint32_t              work_hs;      /* HS stored in case of ERRAT */
	uint32_t              work_status[2]; /* Extra status from SLIM */

	wait_queue_head_t    work_waitq;
	struct task_struct   *worker_thread;
	unsigned long data_flags;

	uint32_t hbq_in_use;		/* HBQs in use flag */
	struct list_head rb_pend_list;  /* Received buffers to be processed */
	uint32_t hbq_count;	        /* Count of configured HBQs */
	struct hbq_s hbqs[LPFC_MAX_HBQS]; /* local copy of hbq indicies  */

	unsigned long pci_bar0_map;     /* Physical address for PCI BAR0 */
	unsigned long pci_bar1_map;     /* Physical address for PCI BAR1 */
	unsigned long pci_bar2_map;     /* Physical address for PCI BAR2 */
	void __iomem *slim_memmap_p;	/* Kernel memory mapped address for
					   PCI BAR0 */
	void __iomem *ctrl_regs_memmap_p;/* Kernel memory mapped address for
					    PCI BAR2 */

	void __iomem *MBslimaddr;	/* virtual address for mbox cmds */
	void __iomem *HAregaddr;	/* virtual address for host attn reg */
	void __iomem *CAregaddr;	/* virtual address for chip attn reg */
	void __iomem *HSregaddr;	/* virtual address for host status
					   reg */
	void __iomem *HCregaddr;	/* virtual address for host ctl reg */

	struct lpfc_hgp __iomem *host_gp; /* Host side get/put pointers */
	struct lpfc_pgp   *port_gp;
	uint32_t __iomem  *hbq_put;     /* Address in SLIM to HBQ put ptrs */
	uint32_t          *hbq_get;     /* Host mem address of HBQ get ptrs */

	int brd_no;			/* FC board number */

	char SerialNumber[32];		/* adapter Serial Number */
	char OptionROMVersion[32];	/* adapter BIOS / Fcode version *