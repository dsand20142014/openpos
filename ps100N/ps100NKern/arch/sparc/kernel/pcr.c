p) & (1 << 7))
/* Max size for Primary Stream Arrays - 2^(n+1), where n is bits 12:15 */
#define HCC_MAX_PSA		(1 << ((((p) >> 12) & 0xf) + 1))
/* Extended Capabilities pointer from PCI base - section 5.3.6 */
#define HCC_EXT_CAPS(p)		XHCI_HCC_EXT_CAPS(p)

/* db_off bitmask - bits 0:1 reserved */
#define	DBOFF_MASK	(~0x3)

/* run_regs_off bitmask - bits 0:4 reserved */
#define	RTSOFF_MASK	(~0x1f)


/* Number of registers per port */
#define	NUM_PORT_REGS	4

/**
 * struct xhci_op_regs - xHCI Host Controller Operational Registers.
 * @command:		USBCMD - xHC command register
 * @status:		USBSTS - xHC status register
 * @page_size:		This indicates the page size that the host controller
 * 			supports.  If bit n is set, the HC supports a page size
 * 			of 2^(n+12), up to a 128MB page size.
 * 			4K is the minimum page size.
 * @cmd_ring:		CRP - 64-bit Command Ring Pointer
 * @dcbaa_ptr:		DCBAAP - 64-bit Device Context Base Address Array Pointer
 * @config_reg:		CONFIG - Configure Register
 * @port_status_base:	PORTSCn - base address for Port Status and Control
 * 			Each port has a Port Status and Control register,
 * 			followed by a Port Power Management Status and Control
 * 			register, a Port Link Info register, and a reserved
 * 			register.
 * @port_power_base:	PORTPMSCn - base address for
 * 			Port Power Management Status and Control
 * @port_link_base:	PORTLIn - base address for Port Link Info (current
 * 			Link PM state and control) for USB 2.1 and USB 3.0
 * 			devices.
 */
struct xhci_op_regs {
	u32	command;
	u32	status;
	u32	page_size;
	u32	reserved1;
	u32	reserved2;
	u32	dev_notification;
	u64	cmd_ring;
	/* rsvd: offset 0x20-2F */
	u32	reserved3[4];
	u64	dcbaa_ptr;
	u32	config_reg;
	/* rsvd: offset 0x3C-3FF */
	u32	reserved4[241];
	/* port 1 registers, which serve as a base address for other ports */
	u32	port_status_base;
	u32	port_power_base;
	u32	port_link_base;
	u32	reserved5;
	/* registers for ports 2-255 */
	u32	reserved6[NUM_PORT_REGS*254];
};

/* USBCMD - USB command - command bitmasks */
/* start/stop HC execution - do not write unless HC is halted*/
#define CMD_RUN		XHCI_CMD_RUN
/* Reset HC - resets internal HC state machine and all registers (except
 * PCI config regs).  HC does NOT drive a USB reset on the downstream ports.
 * The xHCI driver must reinitialize the xHC after setting this bit.
 */
#define CMD_RESET	(1 << 1)
/* Event Interrupt Enable - a '1' allows interrupts from the host controller */
#define CMD_EIE		XHCI_CMD_EIE
/* Host System Error Interrupt Enable - get out-of-band signal for HC errors */
#define CMD_HSEIE	XHCI_CMD_HSEIE
/* bits 4:6 are reserved (and should be preserved on writes). */
/* light reset (port status stays unchanged) - reset completed when this is 0 */
#define CMD_LRESET	(1 << 7)
/* FIXME: ignoring host controller save/restore state for now. */
#define CMD_CSS		(1 << 8)
#define CMD_CRS		(1 << 9)
/* Enable Wrap Event - '1' means xHC generates an event when MFINDEX wraps. */
#define CMD_EWE		XHCI_CMD_EWE
/* MFINDEX power management - '1' means xHC can stop MFINDEX counter if all root
 * hubs are in U3 (selective suspend), disconnect, disabled, or powered-off.
 * '0' means the xHC can po