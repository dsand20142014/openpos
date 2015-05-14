e void ahc_set_transaction_status(struct scb *, uint32_t);
static inline void ahc_cmd_set_scsi_status(struct scsi_cmnd *, uint32_t);
static inline void ahc_set_scsi_status(struct scb *, uint32_t);
static inline uint32_t ahc_cmd_get_transaction_status(struct scsi_cmnd *cmd);
static inline uint32_t ahc_get_transaction_status(struct scb *);
static inline uint32_t ahc_cmd_get_scsi_status(struct scsi_cmnd *cmd);
static inline uint32_t ahc_get_scsi_status(struct scb *);
static inline void ahc_set_transaction_tag(struct scb *, int, u_int);
static inline u_long ahc_get_transfer_length(struct scb *);
static inline int ahc_get_transfer_dir(struct scb *);
static inline void ahc_set_residual(struct scb *, u_long);
static inline void ahc_set_sense_residual(struct scb *scb, u_long resid);
static inline u_long ahc_get_residual(struct scb *);
static inline u_long ahc_get_sense_residual(struct scb *);
static inline int ahc_perform_autosense(struct scb *);
static inline uint32_t ahc_get_sense_bufsize(struct ahc_softc *,
					       struct scb *);
static inline void ahc_notify_xfer_settings_change(struct ahc_softc *,
						     struct ahc_devinfo *);
static inline void ahc_platform_scb_free(struct ahc_softc *ahc,
					   struct scb *scb);
static inline void ahc_freeze_scb(struct scb *scb);

static inline
void ahc_cmd_set_transaction_status(struct scsi_cmnd *cmd, uint32_t status)
{
	cmd->result &= ~(CAM_STATUS_MASK << 16);
	cmd->result |= status << 16;
}

static inline
void ahc_set_transaction_status(struct scb *scb, uint32_t status)
{
	ahc_c