#ifndef __CVMX_CONFIG_H__
#define __CVMX_CONFIG_H__

/************************* Config Specific Defines ************************/
#define CVMX_LLM_NUM_PORTS 1
#define CVMX_NULL_POINTER_PROTECT 1
#define CVMX_ENABLE_DEBUG_PRINTS 1
/* PKO queues per port for interface 0 (ports 0-15) */
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 1
/* PKO queues per port for interface 1 (ports 16-31) */
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE1 1
/* Limit on the number of PKO ports enabled for interface 0 */
#define CVMX_PKO_MAX_PORTS_INTERFACE0 CVMX_HELPER_PKO_MAX_PORTS_INTERFACE0
/* Limit on the number of PKO ports enabled for interface 1 */
#define CVMX_PKO_MAX_PORTS_INTERFACE1 CVMX_HELPER_PKO_MAX_PORTS_INTERFACE1
/* PKO queues per port for PCI (ports 32-35) */
#define CVMX_PKO_QUEUES_PER_PORT_PCI 1
/* PKO queues per port for Loop devices (ports 36-39) */
#define CVMX_PKO_QUEUES_PER_PORT_LOOP 1

/************************* FPA allocation *********************************/
/* Pool sizes in bytes, must be multiple of a cache line */
#define CVMX_FPA_POOL_0_SIZE (16 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_1_SIZE (1 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_2_SIZE (8 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_3_SIZE (0 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_4_SIZE (0 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_5_SIZE (0 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_6_SIZE (0 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_7_SIZE (0 * CVMX_CACHE_LINE_SIZE)

/* Pools in use */
/* Packet buffers */
#define CVMX_FPA_PACKET_POOL                (0)
#define CVMX_FPA_PACKET_POOL_SIZE           CVMX_FPA_POOL_0_SIZE
/* Work queue entrys */
#define CVMX_FPA_WQE_POOL                   (1)
#define CVMX_FPA_WQE_POOL_SIZE              CVMX_FPA_POOL_1_SIZE
/* PKO queue command buffers */
#define CVMX_FPA_OUTPUT_BUFFER_POOL         (2)
#define CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE    CVMX_FPA_POOL_2_SIZE

/*************************  FAU allocation ********************************/
/* The fetch and add registers are allocated here.  They are arranged
 * in order of descending size so that all alignment constraints are
 * automatically met.  The enums are linked so that the following enum
 * continues allocating where the previous one left off, so the
 * numbering within each enum always starts with zero.  The macros
 * take care of the address increment size, so the values entered
 * always increase by 1.  FAU registers are accessed with byte
 * addresses.
 */

#define CVMX_FAU_REG_64_ADDR(x) ((x << 3) + CVMX_FAU_REG_64_START)
typedef enum {
	CVMX_FAU_REG_64_START	= 0,
	CVMX_FAU_REG_64_END	= CVMX_FAU_REG_64_ADDR(0),
} cvmx_fau_reg_64_t;

#define CVMX_FAU_REG_32_ADDR(x) ((x << 2) + CVMX_FAU_REG_32_START)
typedef enum {
	CVMX_FAU_REG_32_START	= CVMX_FAU_REG_64_END,
	CVMX_FAU_REG_32_END	= CVMX_FAU_REG_32_ADDR(0),
} cvmx_fau_reg_32_t;

#define CVMX_FAU_REG_16_ADDR(x) ((x << 1) + CVMX_FAU_REG_16_START)
typedef enum {
	CVMX_FAU_REG_16_START	= CVMX_FAU_REG_32_END,
	CVMX_FAU_REG_16_END	= CVMX_FAU_REG_16_ADDR(0),
} cvmx_fau_reg_16_t;

#define CVMX_FAU_REG_8_ADDR(x) ((x) + CVMX_FAU_REG_8_START)
typedef enum {
	CVMX_FAU_REG_8_START	= CVMX_FAU_REG_16_END,
	CVMX_FAU_REG_8_END	= CVMX_FAU_REG_8_ADDR(0),
} cvmx_fau_reg_8_t;

/*
 * The name CVMX_FAU_REG_AVAIL_BASE is provided to indicate the first
 * available FAU address that is not allocated in cvmx-config.h. This
 * is 64 bit aligned.
 */
#define CVMX_FAU_REG_AVAIL_BASE ((CVMX_FAU_REG_8_END + 0x7) & (~0x7ULL))
#define CVMX_FAU_REG_END (2048)

/********************** scratch memory allocation *************************/
/* Scratchpad memory allocation.  Note that these are byte memory
 * addresses.  Some uses of scratchpad (IOBDMA for example) require
 * the use of 8-byte aligned addresses, so proper alignment needs to
 * be taken into account.
 */
/* Generic scratch iobdma area */
#define CVMX_SCR_SCRATCH               (0)
/* First location available after cvmx-config.h allocated region. */
#define CVMX_SCR_REG_AVAIL_BASE        (8)

/*
 * CVMX_HELPER_FIRST_MBUFF_SKIP is the number of bytes to reserve
 * befus = 0;
			break;
		}
	}
	if (status < 0)
		return status;

	agent = kzalloc(sizeof *agent, GFP_KERNEL);
	if (!agent)
		return -ENOMEM;

	status = 0;

	agent->irq_base = irq_base;
	agent->sih = sih;
	agent->imr = ~0;
	INIT_WORK(&agent->mask_work, twl4030_sih_do_mask);
	INIT_WORK(&agent->edge_work, twl4030_sih_do_edge);

	for (i = 0; i < sih->bits; i++) {
		irq = irq_base + i;

		set_irq_chip_and_handler(irq, &twl4030_sih_irq_chip,
				handle_edge_irq);
		set_irq_chip_data(irq, agent);
		activate_irq(irq);
	}

	status = irq_base;
	twl4030_irq_next += i;

	/* replace generic PIH handler (handle_simple_irq) */
	irq = sih_mod + twl4030_irq_base;
	set_irq_data(irq, agent);
	set_irq_chained_handler(irq, handle_twl4030_sih);

	pr_info("twl4030: %s (irq %d) chaining IRQs %d..%d\n", sih->name,
			irq, irq_base, twl4030_irq_next - 1);

	return status;
}

/* FIXME need a call to reverse twl4030_sih_setup() ... */


/*----------------------------------------------------------------------*/

/* FIXME pass in which interrupt line we'll use ... */
#define twl_irq_line	0

int twl_init_irq(int irq_num, unsigned irq_base, unsigned irq_end)
{
	static struct irq_chip	twl4030_irq_chip;

	int			status;
	int			i;
	struct task_struct	*task;

	/*
	 * Mask and clear all TWL4030 interrupts since initially we do
	 * not have any TWL4030 module interrupt handlers present
	 */
	status = twl4030_init_sih_modules(twl_irq_line);
	if (status < 0)
		return status;

	wq = create_singlethread_workqueue("twl4030-irqchip");
	if (!wq) {
		pr_err("twl4030: workqueue FAIL\n");
		return -ESRCH;
	}

	twl4030_irq_base = irq_base;

	/* install an irq handler for each of the SIH modules;
	 * clone dummy irq_chip since PIH can't *do* anything
	 */
	twl4030_irq_chip = dummy_irq_chip;
	twl4030_irq_chip.name = "twl4030";

	twl4030_sih_irq_chip.ack = dummy_irq_chip.ack;

	for (i = irq_base; i < irq_end; i++) {
		set_irq_chip_and_handler(i, &twl4030_irq_chip,
				handle_simple_irq);
		activate_irq(i);
	}
	twl4030_irq_next = i;
	pr_info("twl4030: %s (irq %d) chaining IRQs %d..%d\n", "PIH",
			irq_num, irq_base, twl4030_irq_next - 1);

	/* ... and the PWR_INT module ... */
	status = twl4030_sih_setup(TWL4030_MODULE_INT);
	if (status < 0) {
		pr_err("twl4030: sih_setup PWR INT --> %d\n", status);
		goto fail;
	}

	/* install an irq handler to demultiplex the TWL4030 interrupt */


	init_completion(&irq_