/*
 * Copyright 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright 2008 Juergen Beisert, kernel@pengutronix.de
 *
 * This contains hardware definitions that are common between i.MX21 and
 * i.MX27.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifndef __ASM_ARCH_MXC_MX2x_H__
#define __ASM_ARCH_MXC_MX2x_H__

#ifndef __ASM_ARCH_MXC_HARDWARE_H__
#error "Do not include directly."
#endif

/* The following addresses are common between i.MX21 and i.MX27 */

/* Register offests */
#define AIPI_BASE_ADDR          0x10000000
#define AIPI_BASE_ADDR_VIRT     0xF4000000
#define AIPI_SIZE               SZ_1M

#define DMA_BASE_ADDR           (AIPI_BASE_ADDR + 0x01000)
#define WDOG_BASE_ADDR          (AIPI_BASE_ADDR + 0x02000)
#define GPT1_BASE_ADDR          (AIPI_BASE_ADDR + 0x03000)
#define GPT2_BASE_ADDR          (AIPI_BASE_ADDR + 0x04000)
#define GPT3_BASE_ADDR          (AIPI_BASE_ADDR + 0x05000)
#define PWM_BASE_ADDR           (AIPI_BASE_ADDR + 0x06000)
#define RTC_BASE_ADDR           (AIPI_BASE_ADDR + 0x07000)
#define KPP_BASE_ADDR           (AIPI_BASE_ADDR + 0x08000)
#define OWIRE_BASE_ADDR         (AIPI_BASE_ADDR + 0x09000)
#define UART1_BASE_ADDR         (AIPI_BASE_ADDR + 0x0A000)
#define UART2_BASE_ADDR         (AIPI_BASE_ADDR + 0x0B000)
#define UART3_BASE_ADDR         (AIPI_BASE_ADDR + 0x0C000)
#define UART4_BASE_ADDR         (AIPI_BASE_ADDR + 0x0D000)
#define CSPI1_BASE_ADDR         (AIPI_BASE_ADDR + 0x0E000)
#define CSPI2_BASE_ADDR         (AIPI_BASE_ADDR + 0x0F000)
#define SSI1_BASE_ADDR          (AIPI_BASE_ADDR + 0x10000)
#define SSI2_BASE_ADDR          (AIPI_BASE_ADDR + 0x11000)
#define I2C_BASE_ADDR           (AIPI_BASE_ADDR + 0x12000)
#define SDHC1_BASE_ADDR         (AIPI_BASE_ADDR + 0x13000)
#define SDHC2_BASE_ADDR         (AIPI_BASE_ADDR + 0x14000)
#define GPIO_BASE_ADDR          (AIPI_BASE_ADDR + 0x15000)
#define AUDMUX_BASE_ADDR        (AIPI_BASE_ADDR + 0x16000)
#define CSPI3_BASE_ADDR         (AIPI_BASE_ADDR + 0x17000)
#define LCDC_BASE_ADDR          (AIPI_BASE_ADDR + 0x21000)
#define SLCDC_BASE_ADDR         (AIPI_BASE_ADDR + 0x22000)
#define USBOTG_BASE_ADDR        (AIPI_BASE_ADDR + 0x24000)
#define EMMA_PP_BASE_ADDR       (AIPI_BASE_ADDR + 0x26000)
#define EMMA_PRP_BASE_ADDR      (AIPI_BASE_ADDR + 0x26400)
#define CCM_BASE_ADDR           (AIPI_BASE_ADDR + 0x27000)
#define SYSCTRL_BASE_ADDR       (AIPI_BASE_ADDR + 0x27800)
#define JAM_BASE_ADDR           (AIPI_BASE_ADDR + 0x3E000)
#define MAX_BASE_ADDR           (AIPI_BASE_ADDR + 0x3F000)

#define AVIC_BASE_ADDR          0x10040000

#define SAHB1_BASE_ADDR         0x80000000
#define SAHB1_BASE_ADDR_VIRT    0xF4100000
#define SAHB1_SIZE              SZ_1M

#define CSI_BASE_ADDR           (SAHB1_BASE_ADDR + 0x0000)

/*
 * This macro defines the physical to virtual address mapping for all the
 * peripheral modules. It is used by passing in the physical address as x
 * and returning the virtual address. If the physical address is not mapped,
 * it returns 0xDEADBEEF
 */
#define IO_ADDRESS(x)   \
	(void __force __iomem *) \
	(((x >= AIPI_BASE_ADDR) && (x < (AIPI_BASE_ADDR + AIPI_SIZE))) ? \
		AIPI_IO_ADDRESS(x) : \
	((x >= SAHB1_BASE_ADDR) && (x < (SAHB1_BASE_ADDR + SAHB1_SIZE))) ? \
		SAHB1_IO_ADDRESS(x) : \
	((x >= X_MEMC_BASE_ADDR) && (x < (X_MEMC_BASE_ADDR + X_MEMC_SIZE))) ? \
		X_MEMC_IO_ADDRESS(x) : 0xDEADBEEF)

/* define the address mapping macros: in physical address or request and an interrupt event).
 *
 * We need to use a spinlock whenever we do need to provide mutual
 * exclusion while possibly executing in a hardware interrupt context.
 * Spinlocks should be held for the minimum time that is necessary
 * because hardware interrupts are disabled while a spinlock is held.
 *
 */

static spinlock_t lock = SPIN_LOCK_UNLOCKED;
/*!
 * @brief This mutex is used to provide mutual exclusion.
 *
 * Create a mutex that can be used to provide mutually exclusive
 * read/write access to the globally accessible data structures
 * that were defined above. Mutually exclusive access is required to
 * ensure that the audio data structures are consistent at all times
 * when possibly accessed by multiple threads of execution.
 *
 * Note that we use a mutex instead of the spinlock whenever disabling
 * interrupts while in the critical section is not required. This helps
 * to minimize kernel interrupt handling latency.
 */
static DECLARE_MUTEX(mutex);

/*!
 * @brief Global variable to track currently active interrupt events.
 *
 * This global variable is used to keep track of all of the currently
 * active interrupt events for the audio driver. Note that access to this
 * variable may occur while within an interrupt context and, therefore,
 * must be guarded by using a spinlock.
 */
/* static PMIC_CORE_EVENT eventID = 0; */

/* Prototypes for all static audio driver functions. */
/*
static PMIC_STATUS pmic_audio_mic_boost_enable(void);
static PMIC_STATUS pmic_audio_mic_boost_disable(void);*/
static PMIC_STATUS pmic_audio_close_handle(const PMIC_AUDIO_HANDLE handle);
static PMIC_STATUS pmic_audio_reset_device(const PMIC_AUDIO_HANDLE handle);

static PMIC_STATUS pmic_audio_deregister(void *callback,
					 PMIC_AUDIO_EVENTS * const eventMask);

/*************************************************************************
 * Audio device access APIs.
 *************************************************************************
 */

/*!
 * @name General Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Audio
 * hardware.
 */
/*@{*/

PMIC_STATUS pmic_audio_set_autodetect(int val)
{
	PMIC_STATUS status;
	unsigned int reg_mask = 0, reg_write = 0;
	reg_mask = SET_BITS(regAUDIO_RX_0, VAUDIOON, 1);
	status = pmic_write_reg(REG_AUDIO_RX_0, reg_mask, reg_mask);
	if (status != PMIC_SUCCESS)
		return status;
	reg_mask = 0;
	if (val == 1) {
		reg_write = SET_BITS(regAUDIO_RX_0, HSDETEN, 1) |
		    SET_BITS(regAUDIO_RX_0, HSDETAUTOB, 1);
	} else {
		reg_write = 0;
	}
	reg_mask =
	    SET_BITS(regAUDIO_RX_0, HSDETEN, 1) | SET_BITS(regAUDIO_RX_0,
							   HSDETAUTOB, 1);
	status = pmic_write_reg(REG_AUDIO_RX_0, reg_write, reg_mask);

	return status;
}

/*!
 * @brief Request exclusive access to the PMIC Audio hardware.
 *
 * Attempt to open and gain exclusive access to a key PMIC audio hardware
 * component (e.g., the Stereo DAC or the Voice CODEC). Depending upon the
 * type of audio operation that is desired and the nature of the