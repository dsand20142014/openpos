x02
	field	SEQINT    0x01
	mask	BAD_PHASE	SEQINT		/* unknown scsi bus phase */
	mask	SEND_REJECT	0x10|SEQINT	/* sending a message reject */
	mask	PROTO_VIOLATION	0x20|SEQINT	/* SCSI protocol violation */ 
	mask	NO_MATCH	0x30|SEQINT	/* no cmd match for reconnect */
	mask	IGN_WIDE_RES	0x40|SEQINT	/* Complex IGN Wide Res Msg */
	mask	PDATA_REINIT	0x50|SEQINT	/*
						 * Returned to data phase
						 * that requires data
						 * transfer pointers to be
						 * recalculated from the
						 * transfer residual.
						 */
	mask	HOST_MSG_LOOP	0x60|SEQINT	/*
						 * The bus is ready for the
						 * host to perform another
						 * message transaction.  This
						 * mechanism is used for things
						 * like sync/wide negotiation
						 * that require a kernel based
						 * message state engine.
						 */
	mask	BAD_STATUS	0x70|SEQINT	/* Bad status from target */
	mask	PERR_DETECTED	0x80|SEQINT	/*
						 * Either the phase_lock
						 * or inb_next routine has
						 * noticed a