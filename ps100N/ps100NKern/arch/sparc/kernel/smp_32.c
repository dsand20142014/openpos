RUN		0x04	/* SCSI Offset overrun detected */
	field	SPIORDY		0x02	/* SCSI PIO Ready */
	field	ARBDO		0x01	/* Arbitration Done Out */
}

/*
 * Clear SCSI Interrupt 0
 * Writing a 1 to a bit clears the associated SCSI Interrupt in SSTAT0.
 */
register CLRSINT0 {
	address			0x04B
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	CLRSELDO	0x40
	field	CLRSELDI	0x20
	field	CLRSELINGO	0x10
	field	CLRIOERR	0x08
	field	CLROVERRUN	0x04
	field	CLRSPIORDY	0x02
	field	CLRARBDO	0x01
	dont_generate_debug_code
}

/*
 * SCSI Interrupt Mode 0
 * Setting any bit will enable the corresponding function
 * in SIMODE0 to interrupt via the IRQ pin.
 */
register SIMODE0 {
	address			0x04B
	access_mode	RW
	modes		M_CFG
	count		8
	field	ENSELDO		0x40
	field	ENSELDI		0x20
	field	ENSELINGO	0x10
	field	ENIOERR		0x08
	field	ENOVERRUN	0x04
	field	ENSPIORDY	0x02
	field	ENARBDO		0x01
}

/*
 * SCSI Status 1
 */
register SSTAT1 {
	address			0x04C
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	SELTO		0x80
	field	ATNTARG 	0x40
	field	SCSIRSTI	0x20
	field	PHASEMIS	0x10
	field	BUSFREE		0x08
	field	SCSIPERR	0x04
	field	STRB2FAST	0x02
	field	REQINIT		0x01
}

/*
 * Clear SCSI Interrupt 1
 * Writing a 1 to a bit clears the associated SCSI Interrupt in SSTAT1.
 */
register CLRSINT1 {
	address			0x04C
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	CLRSELTIMEO	0x80
	field	CLRATNO		0x40
	field	CLRSCSIRSTI	0x20
	field	CLRBUSFREE	0x08
	field	CLRSCSIPERR	0x04
	field	CLRSTRB2FAST	0x02
	field	CLRREQINIT	0x01
	dont_generate_debug_code
}

/*
 * SCSI Status 2
 */
register SSTAT2 {
	address			0x04d
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	BUSFREETIME	0xc0 {
		BUSFREE_LQO	0x40,
		BUSFREE_DFF0	0x80,
		BUSFREE_DFF1	0xC0
	}
	field	NONPACKREQ	0x20
	field	EXP_ACTIVE	0x10	/* SCSI Expander Active */
	field	BSYX		0x08	/* Busy Expander */
	field	WIDE_RES	0x04	/* Modes 0 and 1 only */
	field	SDONE		0x02	/* Modes 0 and 1 only */
	field	DMADONE		0x01	/* Modes 0 and 1 only */
}

/*
 * Clear SCSI Interrupt 2
 */
register CLRSINT2 {
	address			0x04D
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	CLRNONPACKREQ	0x20
	field	CLRWIDE_RES	0x04	/* Modes 0 and 1 only */
	field	CLRSDONE	0x02	/* Modes 0 and 1 only */
	field	CLRDMADONE	0x01	/* Modes 0 and 1 only */
	dont_generate_debug_code
}

/*
 * SCSI Interrupt Mode 2
 */
register SIMODE2 {
	address			0x04D
	access_mode	RW
	modes		M_CFG
	field	ENWIDE_RES	0x04
	field	ENSDONE		0x02
	field	ENDMADONE	0x01
}

/*
 * Physical Error Diagnosis
 */
register PERRDIAG {
	address			0x04E
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		3
	field	HIZERO		0x80
	field	HIPERR		0x40
	field	PREVPHASE	0x20
	field	PARITYERR	0x10
	field	AIPERR		0x08
	field	CRCERR		0x04
	field	DGFORMERR	0x02
	field	DTERR		0x01
}

/*
 * LQI Manager Current State
 */
register LQISTATE {
	address			0x04E
	access_mode	RO
	modes		M_CFG
	count		6
	dont_generate_debug_code
}

/*
 * SCSI Offset Count
 */
register SOFFCNT {
	address			0x04F
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		1
}

/*
 * LQO Manager Current State
 */
register LQOSTATE {
	address			0x04F
	access_mode	RO
	modes		M_CFG
	count		2
	dont_generate_debug_code
}

/*
 * LQI Manager Status
 */
register LQISTAT0 {
	address			0x050
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		2
	field	LQIATNQAS	0x20
	field	LQICRCT1	0x10
	field	LQICRCT2	0x08
	field	LQIBADLQT	0x04
	field	LQIATNLQ	0x02
	field	LQIATNCMD	0x01
}

/*
 * Clear LQI Interrupts 0
 */
register CLRLQIINT0 {
	address			0x050
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		1
	field	CLRLQIATNQAS	0x20
	field	CLRLQICRCT1	0x10
	field	CLRLQICRCT2	0x08
	field	CLRLQIBADLQT	0x04
	field	CLRLQIATNLQ	0x02
	field	CLRLQIATNCMD	0x01
	dont_generate_debug_code
}

/*
 * LQI Manager Interrupt Mode 0
 */
register LQIMODE0 {
	address			0x050
	access_mode	RW
	modes		M_CFG
	count		3
	field	ENLQIATNQASK	0x20
	field	ENLQICRCT1	0x10
	field	ENLQICRCT2	0x08
	field	ENLQIBADLQT	0x04
	field	ENLQIATNLQ	0x02
	field	ENLQIATNCMD	0x01
	dont_generate_debug_code
}

/*
 * LQI Manager Status 1
 */
register LQISTAT1 {
	address			0x051
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		3
	field	LQIPHASE_LQ	0x80
	field	LQIPHASE_NLQ	0x40
	field	LQIABORT	0x20
	field	LQICRCI_LQ	0x10
	field	LQICRCI_NLQ	0x08
	field	LQIBADLQI	0x04
	field	LQIOVERI_LQ	0x02
	field	LQIOVERI_NLQ	0x01
}

/*
 * Clear LQI Manager Interrupts1
 */
register CLRLQIINT1 {
	address			0x051
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		4
	field	CLRLQIPHASE_LQ	0x80
	field	CLRLQIPHASE_NLQ	0x40
	field	CLRLIQABORT	0x20
	field	CLRLQICRCI_LQ	0x10
	field	CLRLQICRCI_NLQ	0x08
	field	CLRLQIBADLQI	0x04
	field	CLRLQIOVERI_LQ	0x02
	field	CLRLQIOVERI_NLQ	0x01
	dont_generate_debug_code
}

/*
 * LQI Manager Interrupt Mode 1
 */
register LQIMODE1 {
	address			0x051
	access_mode	RW
	modes		M_CFG
	count		4
	field	ENLQIPHASE_LQ	0x80	/* LQIPHASE1 */
	field	ENLQIPHASE_NLQ	0x40	/* LQIPHASE2 */
	field	ENLIQABORT	0x20
	field	ENLQICRCI_LQ	0x10	/* LQICRCI1 */
	field	ENLQICRCI_NLQ	0x08	/* LQICRCI2 */
	field	ENLQIBADLQI	0x04
	field	ENLQIOVERI_LQ	0x02	/* LQIOVERI1 */
	field	ENLQIOVERI_NLQ	0x01	/* LQIOVERI2 */
	dont_generate_debug_code
}

/*
 * LQI Manager Status 2
 */
register LQISTAT2 {
	address			0x052
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	PACKETIZED	0x80
	field	LQIPHASE_OUTPKT	0x40
	field	LQIWORKONLQ	0x20
	field	LQIWAITFIFO	0x10
	field	LQISTOPPKT	0x08
	field	LQISTOPLQ	0x04
	field	LQISTOPCMD	0x02
	field	LQIGSAVAIL	0x01
}

/*
 * SCSI Status 3
 */
register SSTAT3 {
	address			0x053
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		3
	field	NTRAMPERR	0x02
	field	OSRAMPERR	0x01
}

/*
 * Clear SCSI Status 3
 */
register CLRSINT3 {
	address			0x053
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		3
	field	CLRNTRAMPERR	0x02
	field	CLROSRAMPERR	0x01
	dont_generate_debug_code
}

/*
 * SCSI Interrupt Mode 3
 */
register SIMODE3 {
	address			0x053
	access_mode	RW
	modes		M_CFG
	count		4
	field	ENNTRAMPERR	0x02
	field	ENOSRAMPERR	0x01
	dont_generate_debug_code
}

/*
 * LQO Manager Status 0
 */
register LQOSTAT0 {
	address			0x054
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		2
	field	LQOTARGSCBPERR	0x10
	field	LQOSTOPT2	0x08
	field	LQOATNLQ	0x04
	field	LQOATNPKT	0x02
	field	LQOTCRC		0x01
}

/*
 * Clear LQO Manager interrupt 0
 */
register CLRLQOINT0 {
	address			0x054
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		3
	field	CLRLQOTARGSCBPERR	0x10
	field	CLRLQOSTOPT2		0x08
	field	CLRLQOATNLQ		0x04
	field	CLRLQOATNPKT		0x02
	field	CLRLQOTCRC		0x01
	dont_generate_debug_code
}

/*
 * LQO Manager Interrupt Mode 0
 */
register LQOMODE0 {
	address			0x054
	access_mode	RW
	modes		M_CFG
	count		4
	field	ENLQOTARGSCBPERR	0x10
	field	ENLQOSTOPT2		0x08
	field	ENLQOATNLQ		0x04
	field	ENLQOATNPKT		0x02
	field	ENLQOTCRC		0x01
	dont_generate_debug_code
}

/*
 * LQO Manager Status 1
 */
register LQOSTAT1 {
	address			0x055
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	LQOINITSCBPERR	0x10
	field	LQOSTOPI2	0x08
	field	LQOBADQAS	0x04
	field	LQOBUSFREE	0x02
	field	LQOPHACHGINPKT	0x01
}

/*
 * Clear LOQ Interrupt 1
 */
register CLRLQOINT1 {
	address			0x055
	access_mode	WO
	modes		M_DFF0, M_DFF1, M_SCSI
	count		7
	field	CLRLQOINITSCBPERR	0x10
	field	CLRLQOSTOPI2		0x08
	field	CLRLQOBADQAS		0x04
	field	CLRLQOBUSFREE		0x02
	field	CLRLQOPHACHGINPKT	0x01
	dont_generate_debug_code
}

/*
 * LQO Manager Interrupt Mode 1
 */
register LQOMODE1 {
	address			0x055
	access_mode	RW
	modes		M_CFG
	count		4
	field	ENLQOINITSCBPERR	0x10
	field	ENLQOSTOPI2		0x08
	field	ENLQOBADQAS		0x04
	field	ENLQOBUSFREE		0x02
	field	ENLQOPHACHGINPKT	0x01
	dont_generate_debug_code
}

/*
 * LQO Manager Status 2
 */
register LQOSTAT2 {
	address			0x056
	access_mode	RO
	modes		M_DFF0, M_DFF1, M_SCSI
	field	LQOPKT		0xE0
	field	LQOWAITFIFO	0x10
	field	LQOPHACHGOUTPKT	0x02	/* outside of packet boundaries. */
	field	LQOSTOP0	0x01	/* Stopped after sending all packets */
}

/*
 * Output Synchronizer Space Count
 */
register OS_SPACE_CNT {
	address			0x056
	access_mode	RO
	modes		M_CFG
	count		2
	dont_generate_debug_code
}

/*
 * SCSI Interrupt Mode 1
 * Setting any bit will enable the corresponding function
 * in SIMODE1 to interrupt via the IRQ pin.
 */
register SIMODE1 {
	address			0x057
	access_mode	RW
	modes		M_DFF0, M_DFF1, M_SCSI
	field	ENSELTIMO	0x80
	field	ENATNTARG	0x40
	field	ENSCSIRST	0x20
	field	ENPHASEMIS	0x10
	field	ENBUSFREE	0x08
	field	ENSCSIPERR	0x04
	field	ENSTRB2FAST	0x02
	field	ENREQINIT	0x01
}

/*
 * Good Status FIFO
 */
register GSFIFO {
	address			0x058
	access_mode	RO
	size		2
	modes		M_DFF0, M_DFF1, M_SCSI
	dont_generate_debug_code
}

/*
 * Data FIFO SCSI Transfer Control
 */
register DFFSXFRCTL {
	address			0x05A
	access_mode	RW
	modes		M_DFF0, M_DFF1
	field	DFFBITBUCKET	0x08
	field	CLRSHCNT	0x04
	field	CLRCHN		0x02
	field	RSTCHN		0x01
}

/*
 * Next SCSI Control Block
 */
register NEXTSCB {
	address			0x05A
	access_mode	RW
	size		2
	modes		M_SCSI
	dont_generate_debug_code
}

/*
 * LQO SCSI Control
 * (Rev B only.)
 */
register LQOSCSCTL {
	address			0x05A
	access_mode	RW
	size		1
	modes		M_CFG
	count		1
	field		LQOH2A_VERSION	0x80
	field		LQOBUSETDLY	0x40
	field		LQONOHOLDLACK	0x02
	field		LQONOCHKOVER	0x01
	dont_generate_debug_code
}

/*
 * SEQ Interrupts
 */
register SEQINTSRC {
	address			0x05B
	access_mode	RO
	modes		M_DFF0, M_DFF1
	field	CTXTDONE	0x40
	field	SAVEPTRS	0x20
	field	CFG4DATA	0x10
	field	CFG4ISTAT	0x08
	field	CFG4TSTAT	0x04
	field	CFG4ICMD	0x02
	fi