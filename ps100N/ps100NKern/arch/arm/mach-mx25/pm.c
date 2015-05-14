/*
 *  Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <mach/hardware.h>
#include "crm_regs.h"

#include <../mach-mx25/iomux.h>
#include <mach/mx25.h>
#include "iomux.h"

#include <mach-mx25/machine.h>

/*!
 * @defgroup MSL_MX25 i.MX25 Machine Specific Layer (MSL)
 */

/*!
 * @file mach-mx25/pm.c
 * @brief This file contains suspend operations
 *
 * @ingroup MSL_MX25
 */
static unsigned int cgcr0, cgcr1, cgcr2;

#define LCD_SUSPEND_ADDR   0xA0B00000

#define CPLD_SUSPEND_ADDR   0xA0F00000
#define ENTER_SUSPEND       0xAA
#define EXIT_SUSPEND        0x55
static unsigned char *cpld_suspend=NULL;

static unsigned char *lcd_suspend=NULL;


//driver power status variable
static unsigned int driver_active_status;

#define SCAN_CS_ADDR            0xb2800000
#define IPS420_SCAN_CS_ADDR     0xa0800000
#define PS4KV2_SCAN_CS_ADDR     0xa0000090
static volatile uint8_t *scan_cs=0;


#define GPS_CS_ADDR 0xA0700000
static volatile uint8_t *gps_cs=0;


void set_driver_status(unsigned int pwr)
{
    driver_active_status = pwr;
}
EXPORT_SYMBOL(set_driver_status);


static void uart_suspend_enter(void)
{
    //UART1_TXD
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART1_TXD, PAD_CTL_PKE_NONE);

    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_TXD));
    
    
#if     0
    
    //UART1_RXD
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART1_RXD, PAD_CTL_PKE_NONE);
    
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UART1_RXD), 1);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_UART1_RXD), 1);


    //UART1_TXD
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART1_TXD, PAD_CTL_PKE_NONE);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UART1_TXD), 1);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_UART1_TXD), 1);

    //UART1_RTS
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART1_RTS, PAD_CTL_PKE_NONE);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UART1_RTS), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_UART1_RTS), 0);


    //UART1_CTS
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART1_CTS, PAD_CTL_PKE_NONE);

    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_CTS));
    
//    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UART1_CTS), 1);
//    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_UART1_CTS), 1);


    //UART2_RXD
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART2_RXD, PAD_CTL_PKE_NONE);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UART2_RXD), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_UART2_RXD), 0);


    //UART2_TXD
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_GPIO);
    
    mxc_iomux_set_pad(MX25_PIN_UART2_TXD, PAD_CTL_PKE_NONE);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UART2_TXD), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_UART2_TXD), 0);
    
    
#endif
}

static void uart_suspend_exit(void)
{
    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_FUNC);    
    
#if  0
    
    mxc_free_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_FUNC);    

    mxc_free_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_FUNC);

#endif
}


static void lcd_suspend_enter(void)
{
    //LCD HSYNC
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_HSYNC, MUX_CONFIG_GPIO);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_HSYNC), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_HSYNC), 0);

    //LCD VSYNC
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_VSYNC, MUX_CONFIG_GPIO);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSYNC), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSYNC), 0);

    //LSCLK
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_LSCLK, MUX_CONFIG_GPIO);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_LSCLK), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_LSCLK), 0);

    //OE_ACD
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT1);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT2);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT3);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT4);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT5);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT6);
    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_ALT7);

    mxc_request_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_GPIO);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_OE_ACD), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_OE_ACD), 0);
}

static void lcd_suspend_exit(void)
{
    mxc_free_iomux(MX25_PIN_HSYNC, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_HSYNC, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_VSYNC, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_VSYNC, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_LSCLK, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_LSCLK, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_FUNC);
}

static void sdhc_suspend_enter(void)
{
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_D8), NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_D9), NULL);

    mxc_free_iomux(MX25_PIN_SD1_CMD, MUX_CONFIG_FUNC | MUX_CONFIG_SION);
    mxc_free_iomux(MX25_PIN_SD1_CLK, MUX_CONFIG_FUNC | MUX_CONFIG_SION);
    mxc_free_iomux(MX25_PIN_SD1_DATA0, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_SD1_DATA1, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_SD1_DATA2, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_SD1_DATA3, MUX_CONFIG_FUNC);
    mxc_free_iomux(MX25_PIN_D8, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_D9, MUX_CONFIG_GPIO);

    mxc_request_iomux(MX25_PIN_SD1_CMD, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_SD1_CLK, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_SD1_DATA0, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_SD1_DATA1, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_SD1_DATA2, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_SD1_DATA3, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_D8, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_D9, MUX_CONFIG_GPIO);

    gpio_request(IOMUX_TO_GPIO(MX25_PIN_SD1_CMD),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_SD1_CLK),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA0),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA1),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA2),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA3),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_D8),NULL);
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_D9),NULL);

    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_SD1_CMD), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_SD1_CLK), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA0), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA1), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA2), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA3), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_D8), 0);
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_D9), 0);

    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_SD1_CMD), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_SD1_CLK), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA0), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA1), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA2), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_SD1_DATA3), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D8), 0);
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D9), 0);
}

static void sdhc_suspend_exit(void)
{
    mxc_free_iomux(MX25_PIN_SD1_CMD, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_SD1_CLK, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_SD1_DATA0, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_SD1_DATA1, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_SD1_DATA2, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_SD1_DATA3, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_D8, MUX_CONFIG_GPIO);
    mxc_free_iomux(MX25_PIN_D9, MUX_CONFIG_GPIO);

    mxc_request_iomux(MX25_PIN_SD1_CMD,MUX_CONFIG_FUNC | MUX_CONFIG_SION);
    mxc_request_iomux(MX25_PIN_SD1_CLK,MUX_CONFIG_FUNC | MUX_CONFIG_SION);
    mxc_request_iomux(MX25_PIN_SD1_DATA0, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_SD1_DATA1, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_SD1_DATA2, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_SD1_DATA3, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_D8, MUX_CONFIG_GPIO);
    mxc_request_iomux(MX25_PIN_D9, MUX_CONFIG_GPIO);


    gpio_request(IOMUX_TO_GPIO(MX25_PIN_D8), "sd_wp");
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_D9), "sd_det");
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D8));
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D9));

}

#if 0

static void test_enter_suspend(void)
{
    //PAGE1:
    mxc_request_iomux(MX25_PIN_A10, MUX_CONFIG_GPIO);//LED_CTROL
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A10), 0);

    mxc_request_iomux(MX25_PIN_A14, MUX_CONFIG_GPIO);//SOFT_PWRON
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A14), 0);

    mxc_request_iomux(MX25_PIN_A16, MUX_CONFIG_GPIO);//RC500_PWR
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A16), 0);

    mxc_request_iomux(MX25_PIN_A17, MUX_CONFIG_GPIO);//WIFI_POWERDOWN
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A17), 0);

    mxc_request_iomux(MX25_PIN_A18, MUX_CONFIG_GPIO);//LED_2
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A18), 0);

    mxc_request_iomux(MX25_PIN_A19, MUX_CONFIG_GPIO);//LED_3
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A19), 0);

    mxc_request_iomux(MX25_PIN_A24, MUX_CONFIG_GPIO);//LED_4
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A24), 0);

    mxc_request_iomux(MX25_PIN_A25, MUX_CONFIG_GPIO);//VBAT_CS
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A25), 0);

    //page2:
    mxc_request_iomux(MX25_PIN_D9, MUX_CONFIG_GPIO);//SD1_DET
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_D9));

    mxc_request_iomux(MX25_PIN_EB1, MUX_CONFIG_GPIO);//FUSE_Power_CTRL
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_EB1), 0);

    mxc_request_iomux(MX25_PIN_ECB, MUX_CONFIG_GPIO);//UART5_TX
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_ECB), 0);

    mxc_request_iomux(MX25_PIN_LBA, MUX_CONFIG_GPIO);//UART5_RX
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_LBA));

    //page3:
    mxc_request_iomux(MX25_PIN_EXT_ARMCLK, MUX_CONFIG_GPIO);//PRINT_DICLK
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_EXT_ARMCLK), 0);

    mxc_request_iomux(MX25_PIN_UPLL_BYPCLK, MUX_CONFIG_GPIO);//PRINT_DI
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_UPLL_BYPCLK), 0);

    mxc_request_iomux(MX25_PIN_VSTBY_REQ, MUX_CONFIG_GPIO);//POWER_EN_VCC
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 0);

    mxc_request_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_GPIO);//LED_1
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);

    mxc_request_iomux(MX25_PIN_POWER_FAIL, MUX_CONFIG_GPIO);//GSM_PWR
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_POWER_FAIL), 0);

    //page4:
	//LCD_PWR_EN:NOGPIO
    mxc_request_iomux(MX25_PIN_PWM, MUX_CONFIG_GPIO);//BUZZ_CTROL
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_PWM), 0);

    mxc_request_iomux(MX25_PIN_OE_ACD, MUX_CONFIG_GPIO);//LCD_EN
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_OE_ACD), 0);

    //page5:
    mxc_request_iomux(MX25_PIN_I2C1_CLK, MUX_CONFIG_GPIO);//IIC_CLK
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_I2C1_CLK), 0);

    mxc_request_iomux(MX25_PIN_I2C1_DAT, MUX_CONFIG_GPIO);//IIC_DATA
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_I2C1_DAT), 0);

    mxc_request_iomux(MX25_PIN_GPIO_F, MUX_CONFIG_GPIO);//CHARGE_INT
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_F));

    mxc_request_iomux(MX25_PIN_GPIO_E, MUX_CONFIG_GPIO);//CPLD_INT
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_E));

    mxc_request_iomux(MX25_PIN_GPIO_D, MUX_CONFIG_GPIO);//8007_INT2
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_D));

    mxc_request_iomux(MX25_PIN_GPIO_B, MUX_CONFIG_GPIO);//8007_INT1
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_B));

    //mxc_request_iomux(MX25_PIN_GPIO_A, MUX_CONFIG_GPIO);//RC500_INT
    //gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_A));

    //USB2-DP,NOGPIO
    //USB2_DM,NOGPIO
#if 1
    mxc_request_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_GPIO);//UART1_RX
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_RXD));

    mxc_request_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_GPIO);//UART1_TX
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_TXD));

    mxc_request_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_GPIO);//UART1_RTS
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_RTS));

    mxc_request_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_GPIO);//UART1_CTS
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_CTS));

    mxc_request_iomux(MX25_PIN_UART2_RXD, MUX_CONFIG_GPIO);//UART2_RX
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART2_RXD));

    mxc_request_iomux(MX25_PIN_UART2_TXD, MUX_CONFIG_GPIO);//UART2_TX
    gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_UART1_TXD));
#endif
    unsigned char* a0200008;
    a0200008 = (unsigned char *)ioremap_nocache(0xa0200008, 1);
    iowrite8(ioread8(a0200008) & (~(1<<6)), a0200008);//RC500_RST
    //iounmap(a0200008);

    unsigned char* a0700001;
    a0700001 = (unsigned char *)ioremap_nocache(0xa0700001, 1);
    iowrite8(ioread8(a0700001) & (~(1<<4)), a0700001);//GSM_DTR
    iowrite8(ioread8(a0700001) & (~(1<<5)), a0700001);//GSM_RST
    iowrite8(ioread8(a0700001) & (~(1<<6)), a0700001);//GSM_ON/OFF
    //iounmap(a0700001);

    unsigned char* a0800000;
    a0800000 = (unsigned char *)ioremap_nocache(0xa0800000, 1);
    iowrite8(ioread8(a0800000) & (~(1<<7)), a0800000);//SCAN_PWR
    iowrite8(ioread8(a0800000) & (~(1<<5)), a0800000);//SCAN_AIE_WK
    iowrite8(ioread8(a0800000) & (~(1<<4)), a0800000);//SCAN_TRIG
    //iounmap(a0800000);

    unsigned char* a0500001;
    a0500001 = (unsigned char *)ioremap_nocache(0xa0500001, 1);
    iowrite8(0, a0500001);//SB0~SB5
    //iounmap(a0500001);

    unsigned char* a0500002;
    a0500002 = (unsigned char *)ioremap_nocache(0xa0500002, 1);
    iowrite8(0, a0500002);//MOTO0~3
    //iounmap(a0500002);

    unsigned char* a0700000;
    a0700000 = (unsigned char *)ioremap_nocache(0xa0700000, 1);
    iowrite8(ioread8(a0700000) & (~(1<<4)), a0700000);//GSM_DCD
    iowrite8(ioread8(a0700000) & (~(1<<7)), a0700000);//GPS_POWERDOWN
    //iounmap(a0700000);

    unsigned char* a0500000;
    a0500000 = (unsigned char *)ioremap_nocache(0xa0500000, 1);
    iowrite8(ioread8(a0500000) & (~(1<<5)), a0500000);//PHOTO_ON
    iowrite8(ioread8(a0500000) & (~(1<<6)), a0500000);//LATCH
    iowrite8(ioread8(a0500000) & (~(1<<7)), a0500000);//PRINT_ON
    //iounmap(a0500000);

    unsigned char* a0b00000;
    a0b00000 = (unsigned char *)ioremap_nocache(0xa0b00000, 1);
    iowrite8(ioread8(a0b00000) & (~(1<<6)), a0b00000);//LCD_RST
    //iounmap(a0b00000);

    unsigned char* a0900000;
    a0900000 = (unsigned char *)ioremap_nocache(0xa0900000, 1);
    iowrite8(ioread8(a0900000) & (~(1<<6)), a0900000);//CAM_RST
    iowrite8(ioread8(a0900000) & (~(1<<7)), a0900000);//CAM_RST
    //iounmap(a0900000);
}

static void test_exit_suspend(void)
{
#if 1
    unsigned char* a0200008;
    a0200008 = (unsigned char *)ioremap_nocache(0xa0200008, 1);
    iowrite8(ioread8(a0200008) | ((1<<6)), a0200008);//RC500_RST
    iounmap(a0200008);

    mxc_request_iomux(MX25_PIN_A16, MUX_CONFIG_GPIO);//RC500_PWR
    gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_A16), 1);

    mxc_free_iomux(MX25_PIN_ECB, MUX_CONFIG_GPIO);//UART5_TX
    mxc_request_iomux(MX25_PIN_ECB, MUX_CONFIG_FUNC);

    mxc_free_iomux(MX25_PIN_LBA, MUX_CONFIG_GPIO);//UART5_RX
    mxc_request_iomux(MX25_PIN_ECB, MUX_CONFIG_FUNC);
#endif
}

#endif

static int mx25_suspend_enter(suspend_state_t state)
{
    unsigned int reg;

    uart_suspend_enter();

    //GSM POWER OFF
//    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_POWER_FAIL), 0);    

    //GPS POWER OFF
    if(gps_cs == 0)
    {
        gps_cs = ioremap_nocache(GPS_CS_ADDR,1);
    }
    if(gps_cs != 0)
    {
        iowrite8(ioread8(gps_cs) & (~(1<<7)), gps_cs);
    }

    //SCANNER POWER OFF
    if(scan_cs == 0)
    {
        if(sand_machine()==SAND_IPS420)
        {
            scan_cs = ioremap_nocache(IPS420_SCAN_CS_ADDR,1);
        }
        else if(sand_machine()==SAND_PS4KV2)
        {
            scan_cs = ioremap_nocache(PS4KV2_SCAN_CS_ADDR,1);
        }
        else
        {
            scan_cs = ioremap_nocache(SCAN_CS_ADDR,1);
        }
    }
    if(scan_cs != 0)
    {
        if(sand_machine() != SAND_PS4KV2)
        {
            iowrite8(ioread8(scan_cs) & (~(1<<7)), scan_cs);
        }
    }

    //WIFI POWER OFF
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A17), 0);

    //LED1 POWER OFF
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);

    //LED2 POWER OFF
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A18), 0);

    //LED3 POWER OFF
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A19), 0);

    //LED4 POWER OFF
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A24), 0);

    sdhc_suspend_enter();    
    
    lcd_suspend_enter();

    // 5V POWER OFF
//    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 0);

//    test_enter_suspend();

//    if(!cpld_suspend)
//    {
//        cpld_suspend = (unsigned char *)ioremap_nocache(CPLD_SUSPEND_ADDR, 1);
//    }
//
//    iowrite8(ioread8(lcd_suspend) & (~(1<<6)), lcd_suspend);
//
//    iowrite8(ENTER_SUSPEND, cpld_suspend);

    //CPLD RESET LOW IN SUSPEND
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_BCLK), 0);

	switch (state) {
	case PM_SUSPEND_MEM:
		mxc_cpu_lp_set(STOP_POWER_OFF);
		break;
	case PM_SUSPEND_STANDBY:
		mxc_cpu_lp_set(WAIT_UNCLOCKED_POWER_OFF);
		break;
	default:
		return -EINVAL;
	}

	/* Executing CP15 (Wait-for-Interrupt) Instruction */
	cpu_do_idle();

	reg = (__raw_readl(MXC_CCM_CGCR0) & ~MXC_CCM_CGCR0_STOP_MODE_MASK) |
	    cgcr0;
	__raw_writel(reg, MXC_CCM_CGCR0);

	reg = (__raw_readl(MXC_CCM_CGCR1) & ~MXC_CCM_CGCR1_STOP_MODE_MASK) |
	    cgcr1;
	__raw_writel(reg, MXC_CCM_CGCR1);

	reg = (__raw_readl(MXC_CCM_CGCR2) & ~MXC_CCM_CGCR2_STOP_MODE_MASK) |
	    cgcr2;
	__raw_writel(reg, MXC_CCM_CGCR2);

	// 5V POWER ON
//	gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_REQ), 1);

	//ENABLE PWM
	reg = __raw_readl(MXC_CCM_LPIMR0);
	reg &= (~(1<<26));
    __raw_writel(reg , MXC_CCM_LPIMR0);

	reg = __raw_readl(MXC_CCM_LPIMR1);
	reg &= (~(1<<(42-32)));
    __raw_writel(reg , MXC_CCM_LPIMR1);

//    //cpld reset
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_BCLK), 1);
//    mdelay(5);
//
//    mdelay(5);
//    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_BCLK), 1);
//
//    iowrite8(EXIT_SUSPEND, cpld_suspend);

//    iowrite8(ioread8(lcd_suspend) | (1<<6), lcd_suspend);

    //CPLD RESET LOW OUT SUSPEND
    //gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_BCLK), 1);
//    test_exit_suspend();

    uart_suspend_exit();

    lcd_suspend_exit();        

    sdhc_suspend_exit();

    if(driver_active_status & GPRS_PWR)
    {
        //GSM POWER ON
//        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_POWER_FAIL), 1);
    }

    if(driver_active_status & GPS_PWR)
    {
        //GPS POWER ON
        if(gps_cs != 0)
        {
            iowrite8(ioread8(gps_cs) | (1<<7), gps_cs);
        }
    }

    if(driver_active_status & BAR_PWR)
    {
        //SCANNER POWER ON
        if(scan_cs != 0)
        {
            if(sand_machine() != SAND_PS4KV2)
            {
                iowrite8(ioread8(scan_cs) | (1<<7), scan_cs);
            }
        }
    }

    if(driver_active_status & WIFI_PWR)
    {
        //WIFI POWER ON
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A17), 1);
    }

    if(driver_active_status & LED1_PWR)
    {
        //LED1 POWER ON
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
    }

    if(driver_active_status & LED2_PWR)
    {
        //LED2 POWER ON
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A18), 1);
    }

    if(driver_active_status & LED3_PWR)
    {
        //LED3 POWER ON
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A19), 1);
    }

    if(driver_active_status & LED4_PWR)
    {
        //LED4 POWER ON
        gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_A24), 1);
    }

	return 0;
}

/*
 * Called after processes are frozen, but before we shut down devices.
 */
static int mx25_suspend_prepare(void)
{
	cgcr0 = __raw_readl(MXC_CCM_CGCR0) & MXC_CCM_CGCR0_STOP_MODE_MASK;
	cgcr1 = __raw_readl(MXC_CCM_CGCR1) & MXC_CCM_CGCR1_STOP_MODE_MASK;
	cgcr2 = __raw_readl(MXC_CCM_CGCR2) & MXC_CCM_CGCR2_STOP_MODE_MASK;

	return 0;
}

/*
 * Called after devices are re-setup, but before processes are thawed.
 */
static void mx25_suspend_finish(void)
{
}

static int mx25_pm_valid(suspend_state_t state)
{
	return state > PM_SUSPEND_ON && state <= PM_SUSPEND_MAX;
}

struct platform_suspend_ops mx25_suspend_ops = {
	.valid = mx25_pm_valid,
	.prepare = mx25_suspend_prepare,
	.enter = mx25_suspend_enter,
	.finish = mx25_suspend_finish,
};

static int __init mx25_pm_init(void)
{
	pr_info("Static Power Management for Freescale i.MX25\n");
	suspend_set_ops(&mx25_suspend_ops);

	return 0;
}

late_initcall(mx25_pm_init);
