se;
	uint16_t vfi_base;
	unsigned long *vpi_bmask;	/* vpi allocation table */

	/* Data structure used by fabric iocb scheduler */
	struct list_head fabric_iocb_list;
	atomic_t fabric_iocb_count;
	struct timer_list fabric_block_timer;
	unsigned long bit_flags;
#define	FABRIC_COMANDS_BLOCKED	0
	atomic_t num_rsrc_err;
	atomic_t num_cmd_success;
	unsigned long last_rsrc_error_time;
	unsigned long last_ramp_down_time;
	unsigned long last_ramp_up_time;
#ifdef CONFIG_SCSI_LPFC_DEBUG_FS
	struct dentry *hba_debugfs_root;
	atomic_t debugfs_vport_count;
	struct dentry *debug_hbqinfo;
	struct dentry *debug_dumpHostSlim;
	struct dentry *debug_dumpHBASlim;
	struct dentry *debug_dumpData;   /* BlockGuard BPL*/
	struct dentry *debug_dumpD