PDC_TCR)
#define UART_GET_TCR(port)	__raw_readl((port)->membase + ATMEL_PDC_TCR)

static int (*atmel_open_hook)(struct uart_port *);
static void (*atmel_close_hook)(struct uart_port *);

struct atmel_dma_buffer {
	unsigned char	*buf;
	dma_addr_t	dma_addr;
	unsigned int	dma_size;
	unsigned int	ofs;
};

struct atmel_uart_char {
	u16		status;
	u16		ch;
};

#define ATMEL_SERIAL_RINGSIZE 1024

/*
 * We wrap our port structure around the generic uart_port.
 */
struct atmel_uart_port {
	struct uart_port	uart;		/* uart */
	struct clk		*clk;		/* uart clock */
	int			may_wakeup;	/* cached value of device_may_wakeup for times we need to disable it */
	u32			backup_imr;	/* IMR saved during suspend */
	int			break_active;	/* break being received */

	short			use_dma_rx;	/* enable PDC receiver */
	short			pdc_rx_idx;	/* current PDC RX buffer */
	struct atmel_dma_buffer	pdc_rx[2];	/* PDC receier */

	short			use_dma_tx;	/* enable PDC transmitter */
	struct atmel_dma_buffer	pdc_tx;		/* PDC transmitter */

	struct tasklet_struct	tasklet;
	unsigned int		irq_status;
	unsigned int		irq_status_prev;

	struct circ_buf		rx_ring;
};

static struct atmel_uart_port atmel_ports[ATMEL_MAX_UART];

#ifdef SUPPORT_SYSRQ
static struct console atmel_console;
#endif

static inline struct atmel_uart_port *
to_atmel_uart_port(struct uart_port *uart)
{
	return container_of(uart, struct atmel_uart_port, uart);
}

#ifdef CONFIG_SERIAL_ATMEL_PDC
static bool atmel_use_dma_rx(struct uart_port *port)
{
	struct atmel_uart_port *atmel_port = to_atmel_uart_port(port);

	return atmel_port->use_dma_rx;
}

static bool atmel_use_dma_tx(struct uart_port *port)
{
	struct atmel_uart_port *atmel_port = to_atmel_uart_port(port);

	return atmel_port->use_dma_tx;
}
#else
static bool atmel_use_dma_rx(struct uart_port *port)
{
	return false;
}

static bool atmel_use_dma_tx(struct uart_port *port)
{
	return false;
}
#endif

/*
 * Return TIOCSER_TEMT when transmitter FIFO and Shift register is empty.
 */
static u_int atmel_tx_empty(struct uart_port *port)
{
	return (UART_GET_CSR(port) & ATMEL_US_TXEMPTY) ? TIOCSER_TEMT : 0;
}

/*
 * Set state of the modem control output lines
 */
static void atmel_set_mctrl(struct uart_port *port, u_int mctrl)
{
	unsigned int control = 0;
	unsigned int mode;

#ifdef CONFIG_ARCH_AT91RM9200
	if (cpu_is_at91rm9200()) {
		/*
		 * AT91RM9200 Errata #39: RTS0 is not internally connected
		 * to PA21. We need to drive the pin manually.
		 */
		if (port->mapbase == AT91RM9200_BASE_US0) {
			if (mctrl & TIOCM_RTS)
				at91_set_gpio_value(AT91_PIN_PA21, 0);
			else
				at91_set_gpio_value(AT91_PIN_PA21, 1);
		}
	}
#endif

	if (mctrl & TIOCM_RTS)
		control |= ATMEL_US_RTSEN;
	else
		control |= ATMEL_US_RTSDIS;

	if (mctrl & TIOCM_DTR)
		control |= ATMEL_US_DTREN;
	else
		control |= ATMEL_US_DTRDIS;

	UART_PUT_CR(port, control);

	/* Local loopback mode? */
	mode = UART_GET_MR(port) & ~ATMEL_US_CHMODE;
	if (mctrl & TIOCM_LOOP)
		mode |= ATMEL_US_CHMODE_LOC_LOOP;
	else
		mode |= ATMEL_US_CHMODE_NORMAL;
	UART_PUT_MR(port, mode);
}

/*
 * Get state of the modem control input lines
 */
static u_int atmel_get_mctrl(struct uart_port *port)
{
	unsigned int status, ret = 0;

	status = UART_GET_CSR(port);

	/*
	 * The control signals are active low.
	 */
	if (!(status & ATMEL_US_DCD))
		ret |= TIOCM_CD;
	if (!(status & ATMEL_US_CTS))
		ret |= T