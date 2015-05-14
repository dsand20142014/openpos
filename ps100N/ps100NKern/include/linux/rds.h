iseq;
	uint8_t	sxfrctl0;
	uint8_t	sxfrctl1;
	uint8_t	simode0;
	uint8_t	simode1;
	uint8_t	seltimer;
	uint8_t	seqctl;
};

struct ahd_suspend_pci_state {
	uint32_t  devconfig;
	uint8_t   command;
	uint8_t   csize_lattime;
};

struct ahd_suspend_state {
	struct	ahd_suspend_channel_state channel[2];
	struct  ahd_suspend_pci_state pci_state;
	uint8_t	optionmode;
	uint8_t	dscommand0;
	uint8_t	dspcistatus;
	/* hsmailbox */
	uint8_t	crccontrol1;
	uint8_t	scbbaddr;
	/* Host and sequencer SCB counts */
	uint8_t	dff_thrsh;
	uint8_t	*scratch_ram;
	uint8_t	*btt;
};

typedef void (*ahd_bus_intr_t)(struct ahd_softc *);

typedef enum {
	AHD_MODE_DFF0,
	AHD_MODE_DFF1,
	AHD_MODE_CCHAN,
	AHD_MODE_SCSI,
	AHD_MODE_CFG,
	AHD_MODE_UNKNOWN
} ahd_mode;

#define AHD_MK_MSK(x) (0x01 << (x))
#define AHD_MODE_DFF0_MSK	AHD_MK_MSK(AHD_MODE_DFF0)
#define AHD_MODE_DFF1_MSK	AHD_MK_MSK(AHD_MODE_DFF1)
#define AHD_MODE_CCHAN_MSK	AHD_MK_MSK(AHD_MODE_CCHAN)
#define AHD_MODE_SCSI_MSK	AHD_MK_MSK(AHD_MODE_SCSI)
#define AHD_MODE_CFG_MSK	AHD_MK_MSK(AHD_MODE_CFG)
#define AHD_MODE_UNKNOWN_MSK	AHD_MK_MSK(AHD_MODE_UNKNOWN)
#define AHD_MODE_ANY_MSK (~0)

typedef uint8_t ahd_mode_state;

typedef void ahd_callback_t (void *);

struct ahd_completion
{
	uint16_t	tag;
	uint8_t		sg_status;
	uint8_t		valid_tag;
};

struct ahd_softc {
	bus_space_tag_t           tags[2];
	bus_space_handle_t        bshs[2];
#ifndef __linux__
	bus_dma_tag_t		  buffer_dmat;   /* dmat for buffer I/O */
#endif
	struct scb_data		  scb_data;

	struct hardware_scb	 *next_queued_hscb;
	struct map_node		 *next_queued_hscb_map;

	/*
	 * SCBs that have been sent to the controller
	 */
	LIST_HEAD(, scb)	  pending_scbs;

	/*
	 * Current register window mode information.
	 */
	ahd_mode		  dst_mode;
	ahd_mode		  src_mode;

	/*
	 * Saved register window mode information
	 * used for restore on next unpause.
	 */
	ahd_mode		  saved_dst_mode;
	ahd_mode		  saved_src_mode;

	/*
	 * Platform specific data.
	 */
	struct ahd_platform_data *platform_data;

	/*
	 * Platform specific device information.
	 */
	ahd_dev_softc_t		  dev_softc;

	/*
	 * Bus specific device information.
	 */
	ahd_bus_intr_t		  bus_intr;

	/*
	 * Target mode related state kept on a per enabled lun basis.
	 * Targets that are not enabled will have null entries.
	 * As an initiator, we keep one target entry for our initiator
	 * ID to store our sync/wide transfer settings.
	 */
	struct ahd_tmode_tstate  *enabled_targets[AHD_NUM_TARGETS];

	/*
	 * The black hole device responsible for handling requests for
	 * disabled luns on enabled targets.
	 */
	struct ahd_tmode_lstate  *black_hole;

	/*
	 * Device instance currently on the bus awaiting a continue TIO
	 * for a command that was not given the disconnect priveledge.
	 */
	struct ahd_tmode_lstate  *pending_device;

	/*
	 * Timer handles for timer driven callbacks.
	 */
	ahd_timer_t		  reset_timer;
	ahd_timer_t		  stat_timer;

	/*
	 * Statistics.
	 */
#define	AHD_STAT_UPDATE_US	250000 /* 250ms */
#define	AHD_STAT_BUCKETS	4
	u_int			  cmdcmplt_bucket;
	uint32_t		  cmdcmplt_counts[AHD_STAT_BUCKETS];
	uint32_t		  cmdcmplt_total;

	/*
	 * Card characteristics
	 */
	ahd_chip		  chip;
	ahd_feature		  features;
	ahd_bug			  bugs;
	ahd_flag		  flags;
	struct seeprom_config	 *seep_config;

	/* Command Queues */
	struct ahd_completion	  *qoutfifo;
	uint16_t		  qoutfifonext;
	uint16_t		  qoutfifonext_valid_tag;
	uint16_t		  qinfifonext;
	uint16_t		  qinfifo[AHD_SCB_MAX];

	/*
	 * Our qfreeze count.  The sequencer compares
	 * this value with its own counter to determine
	 * whether to allow selections to occur.
	 */
	uint16_t		  qfreeze_cnt;

	/* Values to store in the SEQCTL register for pause and unpause */
	uint8_t			  unpause;
	uint8_t			  pause;

	/* Critical Section Data */
	struct cs		 *critical_sections;
	u_int			  num_critical_sections;

	/* Buffer for handling packetized bitbucket. */
	uint8_t			 *overrun_buf;

	/* Links for chaining softcs */
	TAILQ_ENTRY(ahd_softc)	  links;

	/* Channel Names ('A', 'B', etc.) */
	char			  channel;

	/* Initiator Bus ID */
	uint8_t			  our_id;

	/*
	 * Target incoming command FIFO.
	 */
	struct target_cmd	 *targetcmds;
	uint8_t			  tqinfifonext;

	/*
	 * Cached verson of the hs_mailbox so we can avoid
	 * pausing the sequencer during mailbox updates.
	 */
	uint8_t			  hs_mailbox;

	/*
	 * Incoming and outgoing message handling.
	 */
	uint8_t			  send_msg_perror;
	ahd_msg_flags		  msg_flags;
	ahd_msg_type		  msg_type;
	uint8_t			  msgout_buf[12];/* Message we are sending */
	uint8_t			  msgin_buf[12];/* Message we are receiving */
	u_int			  msgout_len;	/* Length of message to send */
	u_int			  msgout_index;	/* Current index in msgout */
	u_int			  msgin_index;	/* Current index in msgin */

	/*
	 * Mapping information for data structures shared
	 * between the sequencer and kernel.
	 */
	bus_dma_tag_t		  parent_dmat;
	bus_dma_tag_t		  shared_data_dmat;
	struct map_node		  shared_data_map;

	/* Information saved through suspend/resume cycles */
	struct ahd_suspend_state  suspend_state;

	/* Number of enabled target mode device on this card */
	u_int			  enabled_luns;

	/* Initialization level of this data structure */
	u_int			  init_level;

	/* PCI cacheline size. */
	u_int			  pci_cachesize;

	/* IO Cell Parameters */
	uint8_t			  iocell_opts[AHD_NUM_PER_DEV_ANNEXCOLS];

	u_int			  stack_size;
	uint16_t		 *saved_stack;

	/* Per-Unit descriptive information */
	const char		 *description;
	const char		 *bus_description;
	char			 *name;
	int			  unit;

	/* Selection Timer settings */
	int			  seltime;

	/*
	 * Interrupt coalescing settings.
	 */
#define	AHD_INT_COALESCING_TIMER_DEFAULT		250 /*us*/
#define	AHD_INT_COALESCING_MAXCMDS_DEFAULT		10
#define	AHD_INT_COALESCING_MAXCMDS_MAX			127
#define	AHD_INT_COALESCING_MINCMDS_DEFAULT		5
#define	AHD_INT_COALESCING_MINCMDS_MAX			127
#define	AHD_INT_COALESCING_THRESHOLD_DEFAULT		2000
#define	AHD_INT_COALESCING_STOP_THRESHOLD_DEFAULT	1000
	u_int			  int_coalescing_timer;
	u_int			  int_coalescing_maxcmds;
	u_int			  int_coalescing_mincmds;
	u_int			  int_coalescing_threshold;
	u_int			  int_coalescing_stop_threshold;

	uint16_t	 	  user_discenable;/* Disconnection allowed  */
	uint16_t		  user_tagenable;/* Tagged Queuing allowed */
};

/*************************** IO Cell Configuration ****************************/
#define	AHD_PRECOMP_SLEW_INDEX						\
    (AHD_ANNEXCOL_PRECOMP_SLEW - AHD_ANNEXCOL_PER_DEV0)

#define	AHD_AMPLITUDE_INDEX						\
    (AHD_ANNEXCOL_AMPLITUDE - AHD_ANNEXCOL_PER_DEV0)

#define AHD_SET_SLEWRATE(ahd, new_slew)					\
do {									\
    (ahd)->iocell_opts[AHD_PRECOMP_SLEW_INDEX] &= ~AHD_SLEWRATE_MASK;	\
    (ahd)->iocell_opts[AHD_PRECOMP_SLEW_INDEX] |=			\
	(((new_slew) << AHD_SLEWRATE_SHIFT) & AHD_SLEWRATE_MASK);	\
} while (0)

#define AHD_SET_PRECOMP(ahd, new_pcomp)					\
do {									\
    (ahd)->iocell_opts[AHD_PRECOMP_SLEW_INDEX] &= ~AHD_PRECOMP_MASK;	\
    (ahd)->iocell_opts[AHD_PRECOMP_SLEW_INDEX] |=			\
	(((new_pcomp) << AHD_PRECOMP_SHIFT) & AHD_PRECOMP_MASK);	\
} while (0)

#define AHD_SET_AMPLITUDE(ahd, new_amp)					\
do {									\
    (ahd)->iocell_opts[AHD_AMPLITUDE_INDEX] &= ~AHD_AM