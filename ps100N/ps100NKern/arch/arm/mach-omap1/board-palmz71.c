/*
 * linux/arch/arm/mach-omap1/board-palmz71.c
 *
 * Modified from board-generic.c
 *
 * Support for the Palm Zire71 PDA.
 *
 * Original version : Laurent Gonzalez
 *
 * Modified for zire71 : Marek Vasut
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <mach/gpio.h>
#include <mach/mux.h>
#include <mach/usb.h>
#include <mach/dma.h>
#include <mach/tc.h>
#include <mach/board.h>
#include <mach/irda.h>
#include <mach/keypad.h>
#include <mach/common.h>
#include <mach/omap-alsa.h>

#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>

#define PALMZ71_USBDETECT_GPIO	0
#define PALMZ71_PENIRQ_GPIO	6
#define PALMZ71_MMC_WP_GPIO	8
#define PALMZ71_HDQ_GPIO 	11

#define PALMZ71_HOTSYNC_GPIO	OMAP_MPUIO(1)
#define PALMZ71_CABLE_GPIO	OMAP_MPUIO(2)
#define PALMZ71_SLIDER_GPIO	OMAP_MPUIO(3)
#define PALMZ71_MMC_IN_GPIO	OMAP_MPUIO(4)

static void __init
omap_palmz71_init_irq(void)
{
	omap1_init_common_hw();
	omap_init_irq();
	omap_gpio_init();
}

static int palmz71_keymap[] = {
	KEY(0, 0, KEY_F1),
	KEY(0, 1, KEY_F2),
	KEY(0, 2, KEY_F3),
	KEY(0, 3, KEY_F4),
	KEY(0, 4, KEY_POWER),
	KEY(1, 0, KEY_LEFT),
	KEY(1, 1, KEY_DOWN),
	KEY(1, 2, KEY_UP),
	KEY(1, 3, KEY_RIGHT),
	KEY(1, 4, KEY_ENTER),
	KEY(2, 0, KEY_CAMERA),
	0,
};

static struct omap_kp_platform_data palmz71_kp_data = {
	.rows	= 8,
	.cols	= 8,
	.keymap	= palmz71_keymap,
	.rep	= 1,
	.delay	= 80,
};

static struct resource palmz71_kp_resources[] = {
	[0] = {
		.start	= INT_KEYBOARD,
		.end	= INT_KEYBOARD,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device palmz71_kp_device = {
	.name	= "omap-keypad",
	.id	= -1,
	.dev	= {
		.platform_data = &palmz71_kp_data,
	},
	.num_resources	= ARRAY_SIZE(palmz71_kp_resources),
	.resource	= palmz71_kp_resources,
};

static struct mtd_partition palmz71_rom_partitions[] = {
	/* PalmOS "Small ROM", contains the bootloader and the debugger */
	{
		.name		= "smallrom",
		.offset		= 0,
		.size		= 0xa000,
		.mask_flags	= MTD_WRITEABLE,
	},
	/* PalmOS "Big ROM", a filesystem with all the OS code and data */
	{
		.name	= "bigrom",
		.offset	= SZ_128K,
		/*
		 * 0x5f0000 bytes big in the multi-language ("EFIGS") version,
		 * 0x7b0000 bytes in the English-only ("enUS") version.
		 */
		.size		= 0x7b0000,
		.mask_flags	= MTD_WRITEABLE,
	},
};

static struct flash_platform_data palmz71_rom_data = {
	.map_name	= "map_rom",
	.name		= "onboardrom",
	.width		= 2,
	.parts		= palmz71_rom_partitions,
	.nr_parts	= ARRAY_SIZE(palmz71_rom_partitions),
};

static struct resource palmz71_rom_resource = {
	.start	= OMAP_CS0_PHYS,
	.end	= OMAP_CS0_PHYS + SZ_8M - 1,
	.flags	= IORESOURCE_MEM,
};

static struct platform_device palmz71_rom_device = {
	.name	= "omapflash",
	.id	= -1,
	.dev = {
		.platform_data = &palmz71_rom_data,
	},
	.num_resources	= 1,
	.resource	= &palmz71_rom_resource,
};

static struct platform_device palmz71_lcd_device = {
	.name	= "lcd_palmz71",
	.id	= -1,
};

static struct omap_irda_config palmz71_irda_config = {
	.transceiver_cap	= IR_SIRMODE,
	.rx_channel		= OMAP_DMA_UART3_RX,
	.tx_channel		= OMAP_DMA_UART3_TX,
	.dest_start		= UART3_THR,
	.src_start		= UART3_RHR,
	.tx_trigger		= 0,
	.rx_trigger		= 0,
};

static struct resource palmz71_irda_resources[] = {
	[0] = {
		.start	= INT_UART3,
		.end	= INT_UART3,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device palmz71_irda_device = {
	.name	= "omapirda",
	.id	= -1,
	.dev = {
		.platform_data = &palmz71_irda_config,
	},
	.num_resources	= ARRAY_SIZE(palmz71_irda_resources),
	.resource	= palmz71_irda_reASK;
        DEVICE_PRT(MSG_LEVEL_DEBUG, KERN_INFO "pDevice->byRFType = %x\n", pDevice->byRFType);

        if (pDevice->bZoneRegExist == FALSE) {
            pDevice->byZoneType = pDevice->abyEEPROM[EEP_OFS_ZONETYPE];
        }
        DEVICE_PRT(MSG_LEVEL_DEBUG, KERN_INFO "pDevice->byZoneType = %x\n", pDevice->byZoneType);

        //Init RF module
        RFbInit(pDevice);

        //Get Desire Power Value
        pDevice->byCurPwr = 0xFF;
        pDevice->byCCKPwr = SROMbyReadEmbedded(pDevice->PortOffset, EEP_OFS_PWR_CCK);
        pDevice->byOFDMPwrG = SROMbyReadEmbedded(pDevice->PortOffset, EEP_OFS_PWR_OFDMG);
        //byCCKPwrdBm = SROMbyReadEmbedded(pDevice->PortOffset, EEP_OFS_CCK_PWR_dBm);

	//byOFDMPwrdBm = SROMbyReadEmbedded(pDevice->PortOffset, EEP_OFS_OFDM_PWR_dBm);
//printk("CCKPwrdBm is 0x%x,byOFDMPwrdBm is 0x%x\n",byCCKPwrdBm,byOFDMPwrdBm);
		// Load power Table


        for (ii=0;ii<CB_MAX_CHANNEL_24G;ii++) {
            pDevice->abyCCKPwrTbl[ii+1] = SROMbyReadEmbedded(pDevice->PortOffset, (BYTE)(ii + EEP_OFS_CCK_PWR_TBL));
            if (pDevice->abyCCKPwrTbl[ii+1] == 0) {
                pDevice->abyCCKPwrTbl[ii+1] = pDevice->byCCKPwr;
            }
            pDevice->abyOFDMPwrTbl[ii+1] = SROMbyReadEmbedded(pDevice->PortOffset, (BYTE)(ii + EEP_OFS_OFDM_PWR_TBL));
            if (pDevice->abyOFDMPwrTbl[ii+1] == 0) {
                pDevice->abyOFDMPwrTbl[ii+1] = pDevice->byOFDMPwrG;
            }
            pDevice->abyCCKDefaultPwr[ii+1] = byCCKPwrdBm;
            pDevice->abyOFDMDefaultPwr[ii+1] = byOFDMPwrdBm;
        }
		//2008-8-4 <add> by chester
	  //recover 12,13 ,14channel for EUROPE by 11 channel
          if(((pDevice->abyEEPROM[EEP_OFS_ZONETYPE] == ZoneType_Japan) ||
	        (pDevice->abyEEPROM[EEP_OFS_ZONETYPE] == ZoneType_Europe))&&
	     (pDevice->byOriginalZonetype == ZoneType_USA)) {
	    for(ii=11;ii<14;ii++) {
                pDevice->abyCCKPwrTbl[ii] = pDevice->abyCCKPwrTbl[10];
	       pDevice->abyOFDMPwrTbl[ii] = pDevice->abyOFDMPwrTbl[10];

	    }
	  }


        // Load OFDM A Power Table
        for (ii=0;ii<CB_MAX_CHANNEL_5G;ii++) { //RobertYu:20041224, bug using CB_MAX_CHANNEL
            pDevice->abyOFDMPwrTbl[ii+CB_MAX_CHANNEL_24G+1] = SROMbyReadEmbedded(pDevice->PortOffset, (BYTE)(ii + EEP_OFS_OFDMA_PWR_TBL));
            pDevice->abyOFDMDefaultPwr[ii+CB_MAX_CHANNEL_24G+1] = SROMbyReadEmbedded(pDevice->PortOffset, (BYTE)(ii + EEP_OFS_OFDMA_PWR_dBm));
        }
        CARDvInitChannelTable((PVOID)pDevice);


        if (pDevice->byLocalID > REV_ID_VT3253_B1) {
            MACvSelectPage1(pDevice->PortOffset);
            VNSvOutPortB(pDevice->PortOffset + MAC_REG_MSRCTL + 1, (MSRCTL1_TXPWR | MSRCTL1_CSAPAREN));
            MACvSelectPage0(pDevice->PortOffset);
        }


         // use relative tx timeout and 802.11i D4
        MACvWordRegBitsOn(pDevice->PortOffset, MAC_REG_CFG, (CFG_TKIPOPT | CFG_NOTXTIMEOUT));

        // set performance parameter by registry
        MACvSetShortRetryLimit(pDevice->PortOffset, pDevice->byShortRetryLimit);
        MACvSetLongRetryLimit(pDevice->PortOffset, pDevice->byLongRetryLimit);

        // reset TSF counter
        VNSvOutPortB(pDevice->PortOffset + MAC_REG_TFTCTL, TFTCTL_TSFCNTRST);
        // enable TSF counter
        VNSvOutPortB(pDevice->PortOffset + MAC_REG_TFTCTL, TFTCTL_TSFCNTREN);

        // initialize BBP registers
        BBbVT3253Init(pDevice);

        if (pDevice->bUpdateBBVGA) {
            pDevice->byBBVGACurrent = pDevice->abyBBVGA[0];
            pDevice->byBBVGANew = pDevice->byBBVGACurrent;
            BBvSetVGAGainOffset(pDevice, pDevice->abyBBVGA[0]);
        }
#ifdef	PLICE_DEBUG
	//printk("init registers:RxAntennaMode is %x,TxAntennaMode is %x\n",pDevice->byRxAntennaMode,pDevice->byTxAntennaMode);
#endif
        BBvSetRxAntennaMode(pDevice->PortOffset, pDevice->byRxAntennaMode);
        BBvSetTxAntennaMode(pDevice->PortOffset, pDevice->byTxAntennaMode);

        pDevice->byCurrentCh = 0;

        //pDevice->NetworkType = Ndis802_11Automode;
        