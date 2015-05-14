/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <mach/hardware.h>
#include <asm/mach/irq.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include "board-mx25_3stack.h"
#include "iomux.h"
#include <linux/semaphore.h>

struct semaphore sem;
EXPORT_SYMBOL(sem);
//volatile spinlock_t spilock=SPIN_LOCK_UNLOCKED;
//EXPORT_SYMBOL(spilock);
/*!
 * @file mach-mx25/mx25_3stack_cpld.c
 *
 * @brief This file contains the board specific initialization routines.
 *
 * @ingroup MSL_MX25
 */

extern int mxc_spi_poll_transfer(struct spi_device *spi,
				 struct spi_transfer *t);
//static int __init mxc_expio_init(void);

struct spi_device *cpld_spi;

/*!
 * This function is used to tranfer data to CPLD regs over CSPI
 */
static inline int mx25_3ds_cpld_rw(u8 *buf, size_t len)
{

	struct spi_transfer t = {
		.tx_buf = (const void *)buf,
		.rx_buf = buf,
		.len = len,
		.cs_change = 0,
		.delay_usecs = 0,
	};

	if (!cpld_spi)
	{
		printk("-------%s---\n", __func__);
		return -1;
	}
	mxc_spi_poll_transfer(cpld_spi, &t);

	//printk("####@@@%%%-------[%s, 0x%02x\, --------\n", __func__,__LINE__);
	return 0;
}

/*!
 * This function is called to read a CPLD register over CSPI.
 *
 * @param        offset    number of the cpld register to be read
 *
 * @return       Returns 0 on success -1 on failure.
 */
unsigned int spi_cpld_read(unsigned int offset)
{
	unsigned char frame[2];
	u8 data;

	frame[0] = offset;
	frame[1] = 0xFF;

	mx25_3ds_cpld_rw((u8 *) frame, 2);

	data = frame[1];
	return data;
}
EXPORT_SYMBOL(spi_cpld_read);

unsigned int spi_single_read(void)
{
	unsigned char frame[2];
	u8 data;

	frame[0] = 0xFF;
	frame[1] = 0xFF;

	mx25_3ds_cpld_rw((u8 *) frame, 1);

	data = frame[0];
	return data;
}
EXPORT_SYMBOL(spi_single_read);

unsigned int spi_lcd128_write(unsigned char dat)
{
	unsigned char frame[1];
	u8 data;

	frame[0] = dat;

	mx25_3ds_cpld_rw((u8 *) frame, 1);

	data = frame[0];
	return data;
}
EXPORT_SYMBOL(spi_lcd128_write);

/*!
 * This function is called to write to a CPLD register over CSPI.
 *
 * @param        offset    number of the cpld register to be written
 * @param        reg_val   value to be written
 *
 * @return       Returns 0 on success -1 on failure.
 */
unsigned int spi_cpld_write(unsigned int offset, unsigned int reg_val)
{
	unsigned char frame[2] = { 0, 0 };

//	frame[0] = (offset << 2) | 0x3;
//	frame[1] = reg_val;

	frame[0] = offset;
	frame[1] = reg_val;

	mx25_3ds_cpld_rw((u8 *) frame, 2);

	return 0;
}
EXPORT_SYMBOL(spi_cpld_write);

static int __init mx25_3ds_cpld_probe(struct spi_device *spi)
{
#if 0

//	printk("---------------%s-----------------\n", __func__);

	spi->bits_per_word = 8;
	spi->max_speed_hz  = 4000000;
	spi->mode          = SPI_MODE_0 | SPI_CS_HIGH;
	spi->chip_select   = 8;

	cpld_spi = spi;
	spi_setup(spi);

//	printk("=====%s====", __func__);
#if 0
	unsigned int i = 0;

	spi->bits_per_word = 46;
	cpld_spi = spi;

	spi_setup(spi);
	i = spi_cpld_read(CPLD_CODE_VER_REG);
	pr_info("3-Stack Debug board detected, rev = 0x%04X\n", i);
	spi_cpld_write(LED_SWITCH_REG, 0xFF);

	/* disable the interrupt and clear the status */
	spi_cpld_write(INTR_MASK_REG, 0);
	spi_cpld_write(INTR_RESET_REG, 0xFFFF);
	spi_cpld_write(INTR_RESET_REG, 0);
	spi_cpld_write(INTR_MASK_REG, 0x1E);

	mxc_expio_init();
#endif
#define  VSTBYACK_4GPIO_CTL ( PAD_CTL_DRV_NORMAL | PAD_CTL_ODE_CMOS | PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE | \
		                      PAD_CTL_PUE_PULL   \
		                     )

	//SPANA    LOW
	mxc_request_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX25_PIN_VSTBY_ACK, VSTBYACK_4GPIO_CTL);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK),NULL);
	gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);

	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);

	spi_cpld_write((0x3 << 2 | 0x3), 0xc8);

	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

#endif

     //SPANA   HIGH
	return 0;
}

/*!
 * This structure contains pointers to the CPLD callback functions.
 */
static struct spi_driver mx25_3ds_cpld_driver = {
	.driver = {
		   .name = "LCD_SPI",
		   .bus = &spi_bus_type,
		   .owner = THIS_MODULE,
		   },
	.probe = mx25_3ds_cpld_probe,
};

static int __init mx25_3ds_cpld_init(void)
{
   int ret;

	sema_init(&sem, 1);

	ret= spi_register_driver(&mx25_3ds_cpld_driver);
	pr_info("Registering the LCD_SPI Driver  Result=%d\n",ret);
	return ret;

#if 0
	pr_info("Registering the CPLD Driver\n");
	return spi_register_driver(&mx25_3ds_cpld_driver);
#endif
	return 0;
}
device_initcall(mx25_3ds_cpld_init);

static int __initdata is_dbg_removed = { 0 };
static int __init remove_dbg_setup(char *__unused)
{
	is_dbg_removed = 1;
	return 0;
}
__setup("remove_dbg", remove_dbg_setup);

static void mxc_expio_irq_handler(u32 irq, struct irq_desc *desc)
{
	u32 expio_irq;
	struct irq_desc *d;

	desc->chip->mask(irq);	/* irq = gpio irq number */

	expio_irq = MXC_BOARD_IRQ_START;

	d = irq_desc + expio_irq;
	if (unlikely(!(d->handle_irq))) {
		printk(KERN_ERR "\nEXPIO irq: %d unhandled\n", expio_irq);
		BUG();		/* oops */
	}
	d->handle_irq(expio_irq, d);

	desc->chip->ack(irq);
	desc->chip->unmask(irq);
}

/*
 * Disable an expio pin's interrupt by setting the bit in the imr.
 * @param irq		an expio virtual irq number
 */
static void expio_mask_irq(u32 irq)
{
}

/*
 * Acknowledge an expanded io pin's interrupt by clearing the bit in the isr.
 * @param irq		an expanded io virtual irq number
 */
static void expio_ack_irq(u32 irq)
{
	/* clear the interrupt status */
	spi_cpld_write(INTR_RESET_REG, 1);
	spi_cpld_write(INTR_RESET_REG, 0);
}

/*
 * Enable a expio pin's interrupt by clearing the bit in the imr.
 * @param irq		a expio virtual irq number
 */
static void expio_unmask_irq(u32 irq)
{
}

static struct irq_chip expio_irq_chip = {
	.ack = expio_ack_irq,
	.mask = expio_mask_irq,
	.unmask = expio_unmask_irq,
};

static int __init mxc_expio_init(void)
{
#if 0
	int i;

	if (is_dbg_removed)
		return 0;

	/*
	 * Configure INT line as GPIO input
	 */
	mxc_request_iomux(MX25_PIN_PWM, MUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX25_PIN_PWM, PAD_CTL_PUE_PUD);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_PWM), NULL);
	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_PWM));

	for (i = MXC_BOARD_IRQ_START; i < (MXC_BOARD_IRQ_START + MXC_BOARD_IRQS);
	     i++) {
		set_irq_chip(i, &expio_irq_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
	set_irq_type(IOMUX_TO_IRQ(MX25_PIN_PWM), IRQF_TRIGGER_LOW);
	set_irq_chained_handler(IOMUX_TO_IRQ(MX25_PIN_PWM),
				mxc_expio_irq_handler);
#endif
	return 0;
}

