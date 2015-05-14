/*
 * (c) Copyright 2009 Freescale Semiconductor
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/mx25.h>
#include <asm/arch/mx25-regs.h>
#include <asm/arch/mx25_pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/gpio.h>
#include <imx_spi.h>

#include <net.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emac_defs.h>

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static u32 system_rev;

u32 get_board_rev(void)
{
    return system_rev;
}

static inline void setup_soc_rev(void)
{
    int reg;
    reg = __REG(IIM_BASE + IIM_SREV);
    if (!reg) {
        reg = __REG(ROMPATCH_REV);
        reg <<= 4;
    } else
        reg += CHIP_REV_1_0;
    system_rev = 0x25000 + (reg & 0xFF);
}

inline int is_soc_rev(int rev)
{
    return (system_rev & 0xFF) - rev;
}

int dram_init(void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#ifdef CONFIG_IPS420
#if (defined(CONFIG_IPS420_64M) ||defined(CONFIG_IPS420_MDDR))
    //Do nothing
#else
    gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
    gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif
#endif
    return 0;
}

#ifdef CONFIG_CMD_MMC

u32 *imx_esdhc_base_addr;

int esdhc_gpio_init(void)
{
    u32 interface_esdhc = 0, val = 0;

    interface_esdhc = (readl(CCM_RCSR) & (0x00300000)) >> 20;

    switch (interface_esdhc) {
        case 0:
            imx_esdhc_base_addr = (u32 *)MMC_SDHC1_BASE;
            /* Pins */
            writel(0x10, IOMUXC_BASE + 0x190);	/* SD1_CMD */
            writel(0x10, IOMUXC_BASE + 0x194);	/* SD1_CLK */
            writel(0x00, IOMUXC_BASE + 0x198);	/* SD1_DATA0 */
            writel(0x00, IOMUXC_BASE + 0x19c);	/* SD1_DATA1 */
            writel(0x00, IOMUXC_BASE + 0x1a0);	/* SD1_DATA2 */
            writel(0x00, IOMUXC_BASE + 0x1a4);	/* SD1_DATA3 */
            writel(0x06, IOMUXC_BASE + 0x094);	/* D12 (SD1_DATA4) */
            writel(0x06, IOMUXC_BASE + 0x090);	/* D13 (SD1_DATA5) */
            writel(0x06, IOMUXC_BASE + 0x08c);	/* D14 (SD1_DATA6) */
            writel(0x06, IOMUXC_BASE + 0x088);	/* D15 (SD1_DATA7) */
            writel(0x05, IOMUXC_BASE + 0x010);	/* A14 (SD1_WP) */
            writel(0x05, IOMUXC_BASE + 0x014);	/* A15 (SD1_DET) */

            /* Pads */
            writel(0xD1, IOMUXC_BASE + 0x388);	/* SD1_CMD */
            writel(0xD1, IOMUXC_BASE + 0x38c);	/* SD1_CLK */
            writel(0xD1, IOMUXC_BASE + 0x390);	/* SD1_DATA0 */
            writel(0xD1, IOMUXC_BASE + 0x394);	/* SD1_DATA1 */
            writel(0xD1, IOMUXC_BASE + 0x398);	/* SD1_DATA2 */
            writel(0xD1, IOMUXC_BASE + 0x39c);	/* SD1_DATA3 */
            writel(0xD1, IOMUXC_BASE + 0x28c);	/* D12 (SD1_DATA4) */
            writel(0xD1, IOMUXC_BASE + 0x288);	/* D13 (SD1_DATA5) */
            writel(0xD1, IOMUXC_BASE + 0x284);	/* D14 (SD1_DATA6) */
            writel(0xD1, IOMUXC_BASE + 0x280);	/* D15 (SD1_DATA7) */
            writel(0xD1, IOMUXC_BASE + 0x230);	/* A14 (SD1_WP) */
            writel(0xD1, IOMUXC_BASE + 0x234);	/* A15 (SD1_DET) */

            /*
             * Set write protect and card detect gpio as inputs
             * A14 (SD1_WP) and A15 (SD1_DET)
             */
            val = ~(3 << 0) & readl(GPIO1_BASE + GPIO_GDIR);
            writel(val, GPIO1_BASE + GPIO_GDIR);
            break;
        case 1:
            imx_esdhc_base_addr = (u32 *)MMC_SDHC2_BASE;
            /* Pins */
            writel(0x16, IOMUXC_BASE + 0x0e8);	/* LD8 (SD1_CMD) */
            writel(0x16, IOMUXC_BASE + 0x0ec);	/* LD9 (SD1_CLK) */
            writel(0x06, IOMUXC_BASE + 0x0f0);	/* LD10 (SD1_DATA0) */
            writel(0x06, IOMUXC_BASE + 0x0f4);	/* LD11 (SD1_DATA1) */
            writel(0x06, IOMUXC_BASE + 0x0f8);	/* LD12 (SD1_DATA2) */
            writel(0x06, IOMUXC_BASE + 0x0fc);	/* LD13 (SD1_DATA3) */
            writel(0x02, IOMUXC_BASE + 0x120);	/* CSI_D2 (SD1_DATA4) */
            writel(0x02, IOMUXC_BASE + 0x124);	/* CSI_D3 (SD1_DATA5) */
            writel(0x02, IOMUXC_BASE + 0x128);	/* CSI_D4 (SD1_DATA6) */
            writel(0x02, IOMUXC_BASE + 0x12c);	/* CSI_D5 (SD1_DATA7) */

            /* Pads */
            writel(0xD1, IOMUXC_BASE + 0x2e0);	/* LD8 (SD1_CMD) */
            writel(0xD1, IOMUXC_BASE + 0x2e4);	/* LD9 (SD1_CLK) */
            writel(0xD1, IOMUXC_BASE + 0x2e8);	/* LD10 (SD1_DATA0) */
            writel(0xD1, IOMUXC_BASE + 0x2ec);	/* LD11 (SD1_DATA1) */
            writel(0xD1, IOMUXC_BASE + 0x2f0);	/* LD12 (SD1_DATA2) */
            writel(0xD1, IOMUXC_BASE + 0x2f4);	/* LD13 (SD1_DATA3) */
            writel(0xD1, IOMUXC_BASE + 0x318);	/* CSI_D2 (SD1_DATA4) */
            writel(0xD1, IOMUXC_BASE + 0x31c);	/* CSI_D3 (SD1_DATA5) */
            writel(0xD1, IOMUXC_BASE + 0x320);	/* CSI_D4 (SD1_DATA6) */
            writel(0xD1, IOMUXC_BASE + 0x324);	/* CSI_D5 (SD1_DATA7) */
            break;
        default:
            break;
    }
    return 0;
}

int board_mmc_init(void)
{
    if (!esdhc_gpio_init())
        return fsl_esdhc_mmc_init(gd->bd);
    else
        return -1;
}
#endif

void spi_io_init(struct imx_spi_dev_t *dev)
{
    switch (dev->base) {
        case CSPI1_BASE:
            writel(0,     IOMUXC_BASE + 0x180);		/* CSPI1 SCLK */
            writel(0x1C0, IOMUXC_BASE + 0x5c4);
            writel(0,     IOMUXC_BASE + 0x184);		/* SPI_RDY */
            writel(0x1E0, IOMUXC_BASE + 0x5c8);
            writel(0,     IOMUXC_BASE + 0x170);		/* MOSI */
            writel(0x1C0, IOMUXC_BASE + 0x5b4);
            writel(0,     IOMUXC_BASE + 0x174);		/* MISO */
            writel(0x1C0, IOMUXC_BASE + 0x5b8);
            writel(0,     IOMUXC_BASE + 0x17C);		/* SS1 */
            writel(0x1E0, IOMUXC_BASE + 0x5C0);
            break;
        default:
            break;
    }
}
#if 1
void mxc_cpld_reset(void)
{
    mxc_request_iomux(MX25_PIN_UPLL_BYPCLK, MUX_CONFIG_GPIO);

    //	gpio_request(IOMUX_TO_GPIO(MX25_PIN_UPLL_BYPCLK),NULL);
    mxc_set_gpio_direction(MX25_PIN_UPLL_BYPCLK, 0);

    mxc_set_gpio_dataout(MX25_PIN_UPLL_BYPCLK, 0);	/* CPLD_RESET */

    udelay(3);
    mxc_set_gpio_dataout(MX25_PIN_UPLL_BYPCLK, 1);	/* CPLD_RESET */
}
#endif

#define IMX_OTG_BASE 0x53ff4000

static void imx25_usb_init(void)
{
    unsigned int tmp;

    tmp=readl(IMX_OTG_BASE + 0x600);
    tmp &= ~(3 << 21);
    tmp |= (2 << 21) | (1 << 4) | (1 << 5);
    writel(tmp, IMX_OTG_BASE + 0x600);

    tmp=readl(IMX_OTG_BASE + 0x584);
    tmp |= 3 << 30;
    writel(tmp, IMX_OTG_BASE + 0x584);

    /*Set to Host mode*/
    tmp = readl(IMX_OTG_BASE + 0x5a8);
    writel(tmp | 0x3, IMX_OTG_BASE + 0X5a8);
}

static imx25_usb_host2_init(void)
{
    //MX25_PAD_D9_USBH2_PWR  MX25_PIN_D9
    //#define MX25_PAD_D9__USBH2_PWR		IOMUX_PAD(0x298, 0x0a0, 0x06, 0, 0, PAD_CTL_PKE)
    mxc_request_iomux(MX25_PIN_D9,MUX_CONFIG_ALT6);
    mxc_iomux_set_pad(MX25_PIN_D9,  PAD_CTL_PKE_ENABLE );

    //#define MX25_PAD_D8__USBH2_OC		IOMUX_PAD(0x29c, 0x0a4, 0x06, 0x580, 0, PAD_CTL_PUS_100K_UP)
    //MX25_PIN_D8
    mxc_request_iomux(MX25_PIN_D8,MUX_CONFIG_ALT6);
    mxc_iomux_set_pad(MX25_PIN_D9,  PAD_CTL_100K_PU );

    //#define MX25_PAD_LD0__USBH2_CLK		IOMUX_PAD(0x2c0,  0xc8, 0x06, 0, 0, 0xe0)
    //MX25_PIN_LD0
    mxc_request_iomux(MX25_PIN_LD0,MUX_CONFIG_ALT6);
    mxc_iomux_set_pad(MX25_PIN_LD0,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU ); //1110 0000

    //#define MX25_PAD_LD1__USBH2_DIR		IOMUX_PAD(0x2c4, 0x0cc, 0x06, 0, 0, 0xe0)
    //MX25_PIN_LD1
    mxc_request_iomux(MX25_PIN_LD0,MUX_CONFIG_ALT6);
    mxc_iomux_set_pad(MX25_PIN_LD0,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU ); //1110 0000

    //#define MX25_PAD_LD2__USBH2_STP		IOMUX_PAD(0x2c8, 0x0d0, 0x06, 0, 0, 0xe5)
    //MX25_PIN_LD2
    mxc_request_iomux(MX25_PIN_LD2,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LD2,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_LD3__USBH2_NXT		IOMUX_PAD(0x2cc, 0x0d4, 0x06, 0, 0, 0xe0)
    //MX25_PIN_LD3
    mxc_request_iomux(MX25_PIN_LD3,MUX_CONFIG_ALT6);
    mxc_iomux_set_pad(MX25_PIN_LD3,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU ); //1110 0000

    //#define MX25_PAD_LD4__USBH2_DATA0	IOMUX_PAD(0x2d0, 0x0d8, 0x06, 0, 0, 0xe5)
    //MX25_PIN_LD4
    mxc_request_iomux(MX25_PIN_LD4,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LD4,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_LD5__USBH2_DATA1	IOMUX_PAD(0x2d4, 0x0dc, 0x06, 0, 0, 0xe5)
    //MX25_PIN_LD5
    mxc_request_iomux(MX25_PIN_LD5,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LD5,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_LD6__USBH2_DATA2	IOMUX_PAD(0x2d8, 0x0e0, 0x06, 0, 0, 0xe5)
    //MX25_PIN_LD6
    mxc_request_iomux(MX25_PIN_LD6,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LD6,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_LD7__USBH2_DATA3	IOMUX_PAD(0x2dc, 0x0e4, 0x06, 0, 0, 0xe5)
    //MX25_PIN_LD7
    mxc_request_iomux(MX25_PIN_LD7,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LD7,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_HSYNC__USBH2_DATA4	IOMUX_PAD(0x300, 0x108, 0x06, 0, 0, 0xe5)
    //MX25_PIN_HSYNC
    mxc_request_iomux(MX25_PIN_LD7,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LD7,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_VSYNC__USBH2_DATA5	IOMUX_PAD(0x304, 0x10c, 0x06, 0, 0, 0xe5)
    //MX25_PIN_VSYNC
    mxc_request_iomux(MX25_PIN_VSYNC,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_VSYNC,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_LSCLK__USBH2_DATA6	IOMUX_PAD(0x308, 0x110, 0x06, 0, 0, 0xe5)
    //MX25_PIN_LSCLK
    mxc_request_iomux(MX25_PIN_LSCLK,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_LSCLK,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);

    //#define MX25_PAD_OE_ACD__USBH2_DATA7	IOMUX_PAD(0x30c, 0x114, 0x06, 0, 0, 0xe5)
    //Mx25_PIN_OE_ACD
    mxc_request_iomux(MX25_PIN_OE_ACD,MUX_CONFIG_ALT6);
    //1110 0101
    mxc_iomux_set_pad(MX25_PIN_OE_ACD,   PAD_CTL_PKE_ENABLE |PAD_CTL_PUE_PUD|PAD_CTL_100K_PU
            | PAD_CTL_DRV_MAX |  PAD_CTL_SRE_FAST);
}

int board_init(void)
{
    setup_soc_rev();

    mxc_cpld_reset();

    //ddr 1.8V
    mxc_iomux_set_pad(MX25_PIN_CTL_GRP_DVS_NFC, (1 << 13));

    u32 pad_reg = readl ( 0x43FAC000 + PIN_TO_IOMUX_PAD(MX25_PIN_CTL_GRP_DVS_NFC));

    //imx25_usb_host2_init();

    //imx25_usb_init();

    /* setup pins for UART1 */

    /* UART 1 IOMUX Configs */
    mxc_request_iomux(MX25_PIN_UART1_RXD, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_UART1_TXD, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_UART1_RTS, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_UART1_CTS, MUX_CONFIG_FUNC);
    mxc_iomux_set_pad(MX25_PIN_UART1_RXD,
            PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE |
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PU);
    mxc_iomux_set_pad(MX25_PIN_UART1_TXD,
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PD);
    mxc_iomux_set_pad(MX25_PIN_UART1_RTS,
            PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE |
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PU);
    mxc_iomux_set_pad(MX25_PIN_UART1_CTS,
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PD);
    /* SAND_MACHINE */
#if (defined(CONFIG_PS4K)||defined(CONFIG_PS4KV2))
    mxc_request_iomux(MX25_PIN_CSI_D2, MUX_CONFIG_ALT1);  //RXD
    mxc_request_iomux(MX25_PIN_CSI_D3, MUX_CONFIG_ALT1);  //TXD

    mxc_iomux_set_pad(MX25_PIN_CSI_D2,
            PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE |
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PU);
    mxc_iomux_set_pad(MX25_PIN_CSI_D3,
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PD);
    mxc_iomux_set_input(MUX_IN_UART5_IPP_UART_RXD_MUX,
            INPUT_CTL_PATH1);
#else
    mxc_request_iomux(MX25_PIN_LBA, MUX_CONFIG_ALT3);  //RXD
    mxc_request_iomux(MX25_PIN_ECB, MUX_CONFIG_ALT3);  //TXD

    mxc_iomux_set_pad(MX25_PIN_LBA,
            PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE |
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PU);
    mxc_iomux_set_pad(MX25_PIN_ECB,
            PAD_CTL_PUE_PUD | PAD_CTL_100K_PD);
    mxc_iomux_set_input(MUX_IN_UART5_IPP_UART_RXD_MUX,
            INPUT_CTL_PATH0);
#endif

    /* setup pins for FEC */
    mxc_request_iomux(MX25_PIN_FEC_TX_CLK, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_RX_DV, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_RDATA0, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_TDATA0, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_TX_EN, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_MDC, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_MDIO, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_RDATA1, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_FEC_TDATA1, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_POWER_FAIL, MUX_CONFIG_FUNC); /* PHY INT */

#define FEC_PAD_CTL1 (PAD_CTL_HYS_SCHMITZ | PAD_CTL_PUE_PUD | \
        PAD_CTL_PKE_ENABLE)
#define FEC_PAD_CTL2 (PAD_CTL_PUE_PUD)

    mxc_iomux_set_pad(MX25_PIN_FEC_TX_CLK, FEC_PAD_CTL1);
    mxc_iomux_set_pad(MX25_PIN_FEC_RX_DV,  FEC_PAD_CTL1);
    mxc_iomux_set_pad(MX25_PIN_FEC_RDATA0, FEC_PAD_CTL1);
    mxc_iomux_set_pad(MX25_PIN_FEC_TDATA0, FEC_PAD_CTL2);
    mxc_iomux_set_pad(MX25_PIN_FEC_TX_EN,  FEC_PAD_CTL2);
    mxc_iomux_set_pad(MX25_PIN_FEC_MDC,    FEC_PAD_CTL2);
    mxc_iomux_set_pad(MX25_PIN_FEC_MDIO,   FEC_PAD_CTL1 | PAD_CTL_22K_PU);
    mxc_iomux_set_pad(MX25_PIN_FEC_RDATA1, FEC_PAD_CTL1);
    mxc_iomux_set_pad(MX25_PIN_FEC_TDATA1, FEC_PAD_CTL2);
    mxc_iomux_set_pad(MX25_PIN_POWER_FAIL, FEC_PAD_CTL1);

    /*
     * Set up the FEC_RESET_B and FEC_ENABLE GPIO pins.
     * Assert FEC_RESET_B, then power up the PHY by asserting
     * FEC_ENABLE, at the same time lifting FEC_RESET_B.
     *
     * FEC_RESET_B: gpio2[3] is ALT 5 mode of pin D12
     * FEC_ENABLE_B: gpio4[8] is ALT 5 mode of pin A17
     */
    mxc_request_iomux(MX25_PIN_A24, MUX_CONFIG_ALT5); /* FEC_EN */
    mxc_request_iomux(MX25_PIN_D12, MUX_CONFIG_ALT5); /* FEC_RESET_B */

    mxc_iomux_set_pad(MX25_PIN_A24, PAD_CTL_ODE_OpenDrain);
    mxc_iomux_set_pad(MX25_PIN_D12, 0);

    mxc_set_gpio_direction(MX25_PIN_A24, 0); /* FEC_EN */
    mxc_set_gpio_direction(MX25_PIN_D12, 0); /* FEC_RESET_B */

    /* drop PHY power */
    mxc_set_gpio_dataout(MX25_PIN_A24, 0);	/* FEC_EN */

    /* assert reset */
    mxc_set_gpio_dataout(MX25_PIN_D12, 0);	/* FEC_RESET_B */
    udelay(2000);		/* spec says 1us min */

    /* turn on PHY power and lift reset eeeee*/
    mxc_set_gpio_dataout(MX25_PIN_A24, 1);	/* FEC_EN */
    mxc_set_gpio_dataout(MX25_PIN_D12, 1);	/* FEC_RESET_B */

#define I2C_PAD_CTL (PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE | \
        PAD_CTL_PUE_PUD | PAD_CTL_100K_PU | PAD_CTL_ODE_OpenDrain)

    mxc_request_iomux(MX25_PIN_I2C1_CLK, MUX_CONFIG_SION);
    mxc_request_iomux(MX25_PIN_I2C1_DAT, MUX_CONFIG_SION);
    mxc_iomux_set_pad(MX25_PIN_I2C1_CLK, 0x1E8);
    mxc_iomux_set_pad(MX25_PIN_I2C1_DAT, 0x1E8);

    gd->bd->bi_arch_number = MACH_TYPE_MX25_3DS;    /* board id for linux */
    gd->bd->bi_boot_params = 0x80000100;    /* address of boot parameters */

    /* SAND_MACHINE */
#if (defined(CONFIG_PS400) || defined(CONFIG_IPS420))
    //set 5v by guxudong
    mxc_request_iomux(MX25_PIN_VSTBY_REQ, MUX_CONFIG_ALT5); //request gpio
    mxc_iomux_set_pad(MX25_PIN_VSTBY_REQ, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PUD | PAD_CTL_100K_PU);
    mxc_set_gpio_direction(MX25_PIN_VSTBY_REQ, 0);
    mxc_set_gpio_dataout(MX25_PIN_VSTBY_REQ, 1);
#endif

    return 0;

#undef FEC_PAD_CTL1
#undef FEC_PAD_CTL2
#undef I2C_PAD_CTL
}

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
    u8 reg[4];

    /* Turn PMIC On*/
    reg[0] = 0x09;
    i2c_write(0x54, 0x02, 1, reg, 1);

#ifdef CONFIG_IMX_SPI_CPLD
    mxc_cpld_spi_init();
#endif

    return 0;
}
#endif


int checkboard(void)
{
#ifdef CONFIG_IPS420
    printf("Board: SAND IPS420.\n");
#elif defined(CONFIG_IPS100N)
    printf("Board: SAND IPS100N.\n");
#else
    printf("Board: SAND test board based i.MX25.\n");
#endif

    return 0;
}



int board_eth_init(bd_t *bis)
{
    int rc = -ENODEV;

    //printf("********************board_eth_init()****************************\n");
    // cpu_eth_init()
    //mxc_fec_initialize
    //	davinci_emac_initialize();

    //	return 0;
#if defined(CONFIG_SMC911X)
    rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
    return rc;
}

#if defined(CONFIG_MXC_KPD)
int setup_mx25_kpd(void)
{
    mxc_request_iomux(MX25_PIN_KPP_ROW0, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_ROW1, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_ROW2, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_ROW3, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_COL0, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_COL1, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_COL2, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_KPP_COL3, MUX_CONFIG_FUNC);
    mxc_request_iomux(MX25_PIN_GPIO_C, MUX_CONFIG_ALT3);

#define KPP_PAD_CTL_ROW (PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PUD | \
        PAD_CTL_100K_PU)
#define KPP_PAD_CTL_COL (PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PUD | \
        PAD_CTL_100K_PU | PAD_CTL_ODE_OpenDrain)

    mxc_iomux_set_pad(MX25_PIN_KPP_ROW0, KPP_PAD_CTL_ROW);
    mxc_iomux_set_pad(MX25_PIN_KPP_ROW1, KPP_PAD_CTL_ROW);
    mxc_iomux_set_pad(MX25_PIN_KPP_ROW2, KPP_PAD_CTL_ROW);
    mxc_iomux_set_pad(MX25_PIN_KPP_ROW3, KPP_PAD_CTL_ROW);
    mxc_iomux_set_pad(MX25_PIN_KPP_COL0, KPP_PAD_CTL_COL);
    mxc_iomux_set_pad(MX25_PIN_KPP_COL1, KPP_PAD_CTL_COL);
    mxc_iomux_set_pad(MX25_PIN_KPP_COL2, KPP_PAD_CTL_COL);
    mxc_iomux_set_pad(MX25_PIN_KPP_COL3, KPP_PAD_CTL_COL);
    mxc_iomux_set_pad(MX25_PIN_GPIO_C, KPP_PAD_CTL_COL);

#undef KPP_PAD_CTL_ROW
#undef KPP_PAD_CTL_COL

    return 0;
}
#endif
