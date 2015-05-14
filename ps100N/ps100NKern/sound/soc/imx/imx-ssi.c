/*
 * imx-ssi.c  --  SSI driver for Freescale IMX
 *
 * Copyright 2006 Wolfson Microelectronics PLC.
 * Author: Liam Girdwood
 *         liam.girdwood@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 * Based on mxc-alsa-mc13783 (C) 2006-2009 Freescale Semiconductor, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    29th Aug 2006   Initial version.
 *
 * TODO:
 *   Need to rework SSI register defs when new defs go into mainline.
 *   Add support for TDM and FIFO 1.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <mach/dma.h>
#include <mach/clock.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>

#include "imx-ssi.h"
#include "imx-pcm.h"

static struct imx_ssi imx_ssi_data[4];

/* debug */
#define IMX_SSI_DEBUG 0
#if IMX_SSI_DEBUG
#define dbg(format, arg...) printk(format, ## arg)
#else
#define dbg(format, arg...)
#endif

#define IMX_SSI_DUMP 0
#if IMX_SSI_DUMP
#define SSI_DUMP() \
	do { \
		printk(KERN_INFO "dump @ %s\n", __func__); \
		printk(KERN_INFO "scr %x\t, %x\n", \
		       __raw_readl(SSI1_SCR), __raw_readl(SSI2_SCR));	\
		printk(KERN_INFO "sisr %x\t, %x\n", \
		       __raw_readl(SSI1_SISR), __raw_readl(SSI2_SISR));	\
		printk(KERN_INFO "stcr %x\t, %x\n", \
		       __raw_readl(SSI1_STCR), __raw_readl(SSI2_STCR)); \
		printk(KERN_INFO "srcr %x\t, %x\n", \
		       __raw_readl(SSI1_SRCR), __raw_readl(SSI2_SRCR)); \
		printk(KERN_INFO "stccr %x\t, %x\n", \
		       __raw_readl(SSI1_STCCR), __raw_readl(SSI2_STCCR)); \
		printk(KERN_INFO "srccr %x\t, %x\n", \
		       __raw_readl(SSI1_SRCCR), __raw_readl(SSI2_SRCCR)); \
		printk(KERN_INFO "sfcsr %x\t, %x\n", \
		       __raw_readl(SSI1_SFCSR), __raw_readl(SSI2_SFCSR)); \
		printk(KERN_INFO "stmsk %x\t, %x\n", \
		       __raw_readl(SSI1_STMSK), __raw_readl(SSI2_STMSK)); \
		printk(KERN_INFO "srmsk %x\t, %x\n", \
		       __raw_readl(SSI1_SRMSK), __raw_readl(SSI2_SRMSK)); \
		printk(KERN_INFO "sier %x\t, %x\n", \
		       __raw_readl(SSI1_SIER), __raw_readl(SSI2_SIER)); \
	} while (0);
#else
#define SSI_DUMP()
#endif

#define SSI1_PORT	0
#define SSI2_PORT	1

static int ssi_active[2] = { 0, 0 };

/*
 * SSI system clock configuration.
 * Should only be called when port is inactive (i.e. SSIEN = 0).
 */
static int imx_ssi_set_dai_sysclk(struct snd_soc_dai *cpu_dai,
				  int clk_id, unsigned int freq, int dir)
{
	u32 scr;

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1)
		scr = __raw_readl(SSI1_SCR);
	else
		scr = __raw_readl(SSI2_SCR);

	if (scr & SSI_SCR_SSIEN)
		return 0;

	switch (clk_id) {
	case IMX_SSP_SYS_CLK:
		if (dir == SND_SOC_CLOCK_OUT)
			scr |= SSI_SCR_SYS_CLK_EN;
		else
			scr &= ~SSI_SCR_SYS_CLK_EN;
		break;
	default:
		return -EINVAL;
	}

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1)
		__raw_writel(scr, SSI1_SCR);
	else
		__raw_writel(scr, SSI2_SCR);

	return 0;
}

/*
 * SSI Clock dividers
 * Should only be called when port is inactive (i.e. SSIEN = 0).
 */
static int imx_ssi_set_dai_clkdiv(struct snd_soc_dai *cpu_dai,
				  int div_id, int div)
{
	u32 stccr, srccr;

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1) {
		if (__raw_readl(SSI1_SCR) & SSI_SCR_SSIEN)
			return 0;

		srccr = __raw_readl(SSI1_SRCCR);
		stccr = __raw_readl(SSI1_STCCR);
	} else {
		if (__raw_readl(SSI2_SCR) & SSI_SCR_SSIEN)
			return 0;

		srccr = __raw_readl(SSI2_SRCCR);
		stccr = __raw_readl(SSI2_STCCR);
	}

	switch (div_id) {
	case IMX_SSI_TX_DIV_2:
		stccr &= ~SSI_STCCR_DIV2;
		stccr |= div;
		break;
	case IMX_SSI_TX_DIV_PSR:
		stccr &= ~SSI_STCCR_PSR;
		stccr |= div;
		break;
	case IMX_SSI_TX_DIV_PM:
		stccr &= ~0xff;
		stccr |= SSI_STCCR_PM(div);
		break;
	case IMX_SSI_RX_DIV_2:
		stccr &= ~SSI_STCCR_DIV2;
		stccr |= div;
		break;
	case IMX_SSI_RX_DIV_PSR:
		stccr &= ~SSI_STCCR_PSR;
		stccr |= div;
		break;
	case IMX_SSI_RX_DIV_PM:
		stccr &= ~0xff;
		stccr |= SSI_STCCR_PM(div);
		break;
	default:
		return -EINVAL;
	}

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1) {
		__raw_writel(stccr, SSI1_STCCR);
		__raw_writel(srccr, SSI1_SRCCR);
	} else {
		__raw_writel(stccr, SSI2_STCCR);
		__raw_writel(srccr, SSI2_SRCCR);
	}
	return 0;
}

/*
 * SSI Network Mode or TDM slots configuration.
 * Should only be called when port is inactive (i.e. SSIEN = 0).
 */
static int imx_ssi_set_dai_tdm_slot(struct snd_soc_dai *cpu_dai,
				    unsigned int mask, int slots)
{
	u32 stmsk, srmsk, stccr;

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1) {
		if (__raw_readl(SSI1_SCR) & SSI_SCR_SSIEN)
			return 0;
		stccr = __raw_readl(SSI1_STCCR);
	} else {
		if (__raw_readl(SSI2_SCR) & SSI_SCR_SSIEN)
			return 0;
		stccr = __raw_readl(SSI2_STCCR);
	}

	stmsk = srmsk = mask;
	stccr &= ~SSI_STCCR_DC_MASK;
	stccr |= SSI_STCCR_DC(slots - 1);

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1) {
		__raw_writel(stmsk, SSI1_STMSK);
		__raw_writel(srmsk, SSI1_SRMSK);
		__raw_writel(stccr, SSI1_STCCR);
		__raw_writel(stccr, SSI1_SRCCR);
	} else {
		__raw_writel(stmsk, SSI2_STMSK);
		__raw_writel(srmsk, SSI2_SRMSK);
		__raw_writel(stccr, SSI2_STCCR);
		__raw_writel(stccr, SSI2_SRCCR);
	}

	return 0;
}

/*
 * SSI DAI format configuration.
 * Should only be called when port is inactive (i.e. SSIEN = 0).
 * Note: We don't use the I2S modes but instead manually configure the
 * SSI for I2S.
 */
static int imx_ssi_set_dai_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct imx_ssi *ssi_mode = (struct imx_ssi *)cpu_dai->private_data;
	u32 stcr = 0, srcr = 0, scr;

	if (cpu_dai->id == IMX_DAI_SSI0 || cpu_dai->id == IMX_DAI_SSI1)
		scr = __raw_readl(SSI1_SCR) & ~(SSI_SCR_SYN | SSI_SCR_NET);
	else
		scr = __raw_readl(SSI2_SCR) & ~(SSI_SCR_SYN | SSI_SCR_NET);

	if (scr & SSI_SCR_SSIEN)
		return 0;

	/* DAI mode */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		/* data on rising edge of bclk, frame low 1clk before data */
		stcr |= SSI_STCR_TFSI | SSI_STCR_TEFS | SSI_STCR_TXBIT0;
		srcr |= SSI_SRCR_RFSI | SSI_SRCR_REFS | SSI_SRCR_RXBIT0;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		/* data on rising edge of bclk, frame high with data */
		stcr |= SSI_STCR_TXBIT0;
		srcr |= SSI_SRCR_RXBIT0;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		/* data on rising edge of bclk, frame high with data */
		stcr |= SSI_STCR_TFSL;
		srcr |= SSI_SRCR_RFSL;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		/* data on rising edge of bclk, frame high 1clk before data */
		stcr |= SSI_STCR_TFSL | SSI_STCR_TEFS;
		srcr |= SSI_SRCR_RFSL | SSI_SRCR_REFS;
		break;
	}

	/* DAI clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_IB_IF:
		stcr &= ~(SSI_STCR_TSCKP | SSI_STCR_TFSI);
		srcr &= ~(SSI_SRCR_RSCKP | SSI_SRCR_RFSI);
		break;
	case SND_SOC_DAIFMT_IB_NF:
		stcr |= SSI_STCR_TFSI;
		stcr &= ~SSI_STCR_TSCKP;
		srcr |= SSI_SRCR_RFSI;
		srcr &= ~SSI_SRCR_RSCKP;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		stcr &= ~SSI_STCR_TFSI;
		stcr |= SSI_STCR_TSCKP;
		srcr &= ~SSI_SRCR_RFSI;
		srcr |= SSI_SRCR_RSCKP;
		break;
	case SND_SOC_DAIFMT_NB_NF:
		stcr |= SSI_STCR_TFSI | SSI_STCR_TSCKP;
		srcr |= SSI_SRCR_RFSI | SSI_SRCR_RSCKP;
		break;
	}

	/* DAI clock master masks */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		stcr |= SSI_STCR_TFDIR | SSI_STCR_TXDIR;
		if (((fmt & SND_SOC_DAIFMT_FORMAT_MASK) == SND_SOC_DAIFMT_I2S)
		    && ssi_mode->network_mode) {
			scr &= ~SSI_SCR_I2S_MODE_MASK;
			scr |= SSI_SCR_I2S_MODE_MSTR;
		}
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
		stcr |= SSI_STCR_TFDIR;
		srcr |= SSI_SRCR_RFDIR;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
		stcr |= SSI_STCR_TXDIR;
		srcr |= SSI_SRCR_RXDIR;
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		if (((fmt & SND_SOC_DAIFMT_FORMAT_MASK) == SND_SOC_DAIFMT_I2S)
		    && ssi_mode->net                                 PCILYNX_MAX_REGISTER);
        lynx->local_ram = ioremap(pci_resource_start(dev,1), PCILYNX_MAX_MEMORY);
        lynx->aux_port  = ioremap(pci_resource_start(dev,2), PCILYNX_MAX_MEMORY);
        lynx->local_rom = ioremap(pci_resource_start(dev,PCI_ROM_RESOURCE),
                                  PCILYNX_MAX_MEMORY);
        lynx->state = have_iomappings;

        if (lynx->registers == NULL) {
                FAIL("failed to remap registers - card not accessible");
        }

        reg_set_bits(lynx, MISC_CONTROL, MISC_CONTROL_SWRESET);
        /* Fix buggy cards with autoboot pin not tied low: */
        reg_write(lynx, DMA0_CHAN_CTRL, 0);

	sprintf (irq_buf, "%d", dev->irq);

        if (!request_irq(dev->irq, lynx_irq_handler, IRQF_SHARED,
                         PCILYNX_DRIVER_NAME, lynx)) {
                PRINT(KERN_INFO, lynx->id, "allocated interrupt %s", irq_buf);
                lynx->state = have_intr;
        } else {
                FAIL("failed to allocate shared interrupt %s", irq_buf);
        }

        /* alloc_pcl return values are not checked, it is expected that the
         * provided PCL space is sufficient for the initial allocations */
        lynx->rcv_pcl = alloc_pcl(lynx);
        lynx->rcv_pcl_start = alloc_pcl(lynx);
        lynx->async.pcl = alloc_pcl(lynx);
        lynx->async.pcl_start = alloc_pcl(lynx);
        lynx->iso_send.pcl = alloc_pcl(lynx);
        lynx->iso_send.pcl_start = alloc_pcl(lynx);

        for (i = 0; i < NUM_ISORCV_PCL; i++) {
                lynx->iso_rcv.pcl[i] = alloc_pcl(lynx);
        }
        lynx->iso_rcv.pcl_start = alloc_pcl(lynx);

        /* all allocations successful - simple init stuff follows */

        reg_write(lynx, PCI_INT_ENABLE, PCI_INT_DMA_ALL);

	tasklet_init(&lynx->iso_rcv.tq, (void (*)(unsigned long))iso_rcv_bh,
		     (unsigned long)lynx);

        spin_lock_init(&lynx->iso_rcv.lock);

        spin_lock_init(&lynx->async.queue_lock);
        lynx->async.channel = CHANNEL_ASYNC_SEND;
        spin_lock_init(&lynx->iso_send.queue_lock);
        lynx->iso_send.channel = CHANNEL_ISO_SEND;

        PRINT(KERN_INFO, lynx->id, "remapped memory spaces reg 0x%p, rom 0x%p, "
              "ram 0x%p, aux 0x%p", lynx->registers, lynx->local_rom,
              lynx->local_ram, lynx->aux_port);

        /* now, looking for PHY register set */
        if ((get_phy_reg(lynx, 2) & 0xe0) == 0xe0) {
                lynx->phyic.reg_1394a = 1;
                PRINT(KERN_INFO, lynx->id,
                      "found 1394a conform PHY (using extended register set)");
                lynx->phyic.vendor = get_phy_vendorid(lynx);
                lynx->phyic.product = get_phy_productid(lynx);
        } else {
                lynx->phyic.reg_1394a = 0;
                PRINT(KERN_INFO, lynx->id, "found old 1394 PHY");
        }

        lynx->selfid_size = -1;
        lynx->phy_reg0 = -1;

	INIT_LIST_HEAD(&lynx->async.queue);
	INIT_LIST_HEAD(&lynx->async.pcl_queue);
	INIT_LIST_HEAD(&lynx->iso_send.queue);
	INIT_LIST_HEAD(&lynx->iso_send.pcl_queue);

        pcl.next = pcl_bus(lynx, lynx->rcv_pcl);
        put_pcl(lynx, lynx->rcv_pcl_start, &pcl);

        pcl.next = PCL_NEXT_INVALID;
        pcl.async_error_next = PCL_NEXT_INVALID;

        pcl.buffer[0].control = PCL_CMD_RCV | 16;
#ifndef __BIG_ENDIAN
	pcl.buffer[0].control |= PCL_BIGENDIAN;
#endif
	pcl.buffer[1].control = PCL_LAST_BUFF | 4080;

        pcl.buffer[0].pointer = lynx->rcv_page_dma;
        pcl.buffer[1].pointer = lynx->rcv_page_dma + 16;
        put_pcl(lynx, lynx->rcv_pcl, &pcl);

        pcl.next = pcl_bus(lynx, lynx->async.pcl);
        pcl.async_error_next = pcl_bus(lynx, lynx->async.pcl);
        put_pcl(lynx, lynx->async.pcl_start, &pcl);

        pcl.next = pcl_bus(lynx, lynx->iso_send.pcl);
        pcl.async_error_next = PCL_NEXT_INVALID;
        put_pcl(lynx, lynx->iso_send.pcl_start, &pcl);

        pcl.next = PCL_NEXT_INVALID;
        pcl.async_error_next = PCL_NEXT_INVALID;
        pcl.buffer[0].control = PCL_CMD_RCV | 4;
#ifndef __BIG_ENDIAN
        pcl.buffer[0].control |= PCL_BIGENDIAN;
#endif
        pcl.buffer[1].control = PCL_LAST_BUFF | 2044;

        for (i = 0; i < NUM_ISORCV_PCL; i++) {
                int page = i / ISORCV_PER_PAGE;
                int sec = i % ISORCV_PER_PAGE;

                pcl.buffer[0].pointer = lynx->iso_rcv.page_dma[page]
                        + sec * MAX_ISORCV_SIZE;
                pcl.buffer[1].pointer = pcl.buffer[0].pointer + 4;
                put_pcl(lynx, lynx->iso_rcv.pcl[i], &pcl);
        }

        pcli = (u32 *)&pcl;
        for (i = 0; i < NUM_ISORCV_PCL; i++) {
                pcli[i] = pcl_bus(lynx, lynx->iso_rcv.pcl[i]);
        }
        put_pcl(lynx, lynx->iso_rcv.pcl_start, &pcl);

        /* FIFO sizes from left to right: ITF=48 ATF=48 GRF=160 */
        reg_write(lynx, FIFO_SIZES, 0x003030a0);
        /* 20 byte threshold before triggering PCI transfer */
        reg_write(lynx, DMA_GLOBAL_REGISTER, 0x2<<24);
        /* threshold on both send FIFOs before transmitting:
           FIFO size - cache line size - 1 */
        i = reg_read(lynx, PCI_LATENCY_CACHELINE) & 0xff;
        i = 0x30 - i - 1;
        reg_write(lynx, FIFO_XMIT_THRESHOLD, (i << 8) | i);

        reg_set_bits(lynx, PCI_INT_ENABLE, PCI_INT_1394);

        reg_write(lynx, LINK_INT_ENABLE, LINK_INT_PHY_TIMEOUT
                  | LINK_INT_PHY_REG_RCVD  | LINK_INT_PHY_BUSRESET
                  | LINK_INT_ISO_STUCK     | LINK_INT_ASYNC_STUCK
                  | LINK_INT_SENT_REJECT   | LINK_INT_TX_INVALID_TC
                  | LINK_INT_GRF_OVERFLOW  | LINK_INT_ITF_UNDERFLOW
                  | LINK_INT_ATF_UNDERFLOW);

        reg_write(lynx, DMA_WORD0_CMP_VALUE(CHANNEL_ASYNC_RCV), 0);
        reg_write(lynx, DMA_WORD0_CMP_ENABLE(CHANNEL_ASYNC_RCV), 0xa<<4);
        reg_write(lynx, DMA_WORD1_CMP_VALUE(CHANNEL_ASYNC_RCV), 0);
        reg_write(lynx, DMA_WORD1_CMP_ENABLE(CHANNEL_ASYNC_RCV),
                  DMA_WORD1_CMP_MATCH_LOCAL_NODE | DMA_WORD1_CMP_MATCH_BROADCAST
                  | DMA_WORD1_CMP_MATCH_EXACT    | DMA_WORD1_CMP_MATCH_BUS_BCAST
                  | DMA_WORD1_CMP_ENABLE_SELF_ID | DMA_WORD1_CMP_ENABLE_MASTER);

        run_pcl(lynx, lynx->rcv_pcl_start, CHANNEL_ASYNC_RCV);

        reg_write(lynx, DMA_WORD0_CMP_VALUE(CHANNEL_ISO_RCV), 0);
        reg_write(lynx, DMA_WORD0_CMP_ENABLE(CHANNEL_ISO_RCV), 0x9<<4);
        reg_write(lynx, DMA_WORD1_CMP_VALUE(CHANNEL_ISO_RCV), 0);
        reg_write(lynx, DMA_WORD1_CMP_ENABLE(CHANNEL_ISO_RCV), 0);

        run_sub_pcl(lynx, lynx->iso_rcv.pcl_start, 0, CHANNEL_ISO_RCV);

        reg_write(lynx, LINK_CONTROL, LINK_CONTROL_RCV_CMP_VALID
                  | LINK_CONTROL_TX_ISO_EN   | LINK_CONTROL_RX_ISO_EN
                  | LINK_CONTROL_TX_ASYNC_EN | LINK_CONTROL_RX_ASYNC_EN
                  | LINK_CONTROL_RESET_TX    | LINK_CONTROL_RESET_RX);

	if (!lynx->phyic.reg_1394a) {
		if (!hpsb_disable_irm) {
			/* attempt to enable contender bit -FIXME- would this
			 * work elsewhere? */
			reg_set_bits(lynx, GPIO_CTRL_A, 0x1);
			reg_write(lynx, GPIO_DATA_BASE + 0x3c, 0x1);
		}
	} else {
		/* set the contender (if appropriate) and LCtrl bit in the
		 * extended PHY register set. (Should check that PHY_02_EXTENDED
		 * is set in register 2?)
		 */
		i = get_phy_reg(lynx, 4);
		i |= PHY_04_LCTRL;
		if (hpsb_disable_irm)
			i &= ~PHY_04_CONTENDER;
		else
			i |= PHY_04_CONTENDER;
		if (i != -1) set_phy_reg(lynx, 4, i);
	}
	
        if (!skip_eeprom)
        {
        	/* needed for i2c communication with serial eeprom */
        	struct i2c_adapter *i2c_ad;
        	struct i2c_algo_bit_data i2c_adapter_data;

        	error = -ENOMEM;
		i2c_ad = kzalloc(sizeof(*i2c_ad), GFP_KERNEL);
        	if (!i2c_ad) FAIL("failed to allocate I2C adapter memory");

		strlcpy(i2c_ad->name, "PCILynx I2C", sizeof(i2c_ad->name));
                i2c_adapter_data = bit_data;
                i2c_ad->algo_data = &i2c_adapter_data;
                i2c_adapter_data.data = lynx;
		i2c_ad->dev.parent = &dev->dev;

		PRINTD(KERN_DEBUG, lynx->id,"original eeprom control: %d",
		       reg_read(lynx, SERIAL_EEPROM_CONTROL));

        	/* reset hardware to sane state */
        	lynx->i2c_driven_state = 0x00000070;
        	reg_write(lynx, SERIAL_EEPROM_CONTROL, lynx->i2c_driven_state);

        	if (i2c_bit_add_bus(i2c_ad) < 0)
        	{
			kfree(i2c_ad);
			error = -ENXIO;
			FAIL("unable to register i2c");
        	}
        	else
        	{
                        /* do i2c stuff */
                        unsigned char i2c_cmd = 0x10;
                        struct i2c_msg msg[2] = { { 0x50, 0, 1, &i2c_cmd },
                                                  { 0x50, I2C_M_RD, 20, (unsigned char*) lynx->bus_info_block }
                                                };

			/* we use i2c_transfer because we have no i2c_client
			   at hand */
                        if (i2c_transfer(i2c_ad, msg, 2) < 0) {
                                PRINT(KERN_ERR, lynx->id, "unable to read bus info block from i2c");
                        } else {
                                PRINT(KERN_INFO, lynx->id, "got bus info block from serial eeprom");
				/* FIXME: probably we shoud rewrite the max_rec, max_ROM(1394a),
				 * generation(1394a) and link_spd(1394a) field and recalculate
				 * the CRC */

                                for (i = 0; i < 5 ; i++)
                                        PRINTD(KERN_DEBUG, lynx->id, "Businfo block quadlet %i: %08x",
					       i, be32_to_cpu(lynx->bus_info_block[i]));

                                /* info_length, crc_length and 1394 magic number to check, if it is really a bus info block */
				if (((be32_to_cpu(lynx->bus_info_block[0]) & 0xffff0000) == 0x04040000) &&
				    (lynx->bus_info_block[1] == IEEE1394_BUSID_MAGIC))
                                {
                                        PRINT(KERN_DEBUG, lynx->id, "read a valid bus info block from");
                                } else {
					kfree(i2c_ad);
					error = -ENXIO;
					FAIL("read something from serial eeprom, but it does not seem to be a valid bus info block");
                                }

                        }

			i2c_del_adapter(i2c_ad);
			kfree(i2c_ad);
                }
        }

	host->csr.guid_hi = be32_to_cpu(lynx->bus_info_block[3]);
	host->csr.guid_lo = be32_to_cpu(lynx->bus_info_block[4]);
	host->csr.cyc_clk_acc = (be32_to_cpu(lynx->bus_info_block[2]) >> 16) & 0xff;
	host->csr.max_rec = (be32_to_cpu(lynx->bus_info_block[2]) >> 12) & 0xf;
	if (!lynx->phyic.reg_1394a)
		host->csr.lnk_spd = (get_phy_reg(lynx, 2) & 0xc0) >> 6;
	else
		host->csr.lnk_spd = be32_to_cpu(lynx->bus_info_block[2]) & 0x7;

	if (hpsb_add_host(host)) {
		error = -ENOMEM;
		FAIL("Failed to register host with highlevel");
	}

	lynx->state = is_host;

	return 0;
#undef FAIL
}


static struct pci_device_id pci_table[] = {
	{
                .vendor =    PCI_VENDOR_ID_TI,
                .device =    PCI_DEVICE_ID_TI_PCILYNX,
                .subvendor = PCI_ANY_ID,
                .subdevice = PCI_ANY_ID,
	},
	{ }			/* Terminating entry */
};

static struct pci_driver lynx_pci_driver = {
        .name =     PCILYNX_DRIVER_NAME,
        .id_table = pci_table,
        .probe =    add_card,
        .remove =   remove_card,
};

static struct hpsb_host_driver lynx_driver = {
	.owner =	   THIS_MODULE,
	.name =		   PCILYNX_DRIVER_NAME,
	.set_hw_config_rom = NULL,
        .transmit_packet = lynx_transmit,
        .devctl =          lynx_devctl,
	.isoctl =          NULL,
};

MODULE_AUTHOR("Andreas E. Bombe <andreas.bombe@munich.netsurf.de>");
MODULE_DESCRIPTION("driver for Texas Instruments PCI Lynx IEEE-1394 controller");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("pcilynx");
MODULE_DEVICE_TABLE(pci, pci_table);

static int __init pcilynx_init(void)
{
        int ret;

        ret = pci_register_driver(&lynx_pci_driver);
        if (ret < 0) {
                PRINT_G(KERN_ERR, "PCI module init failed");
                return ret;
        }

        return 0;
}

static void __exit pcilynx_cleanup(void)
{
        pci_unregister_driver(&lynx_pci_driver);
}


module_init(pcilynx_init);
module_exit(pcilynx_cleanup);
                                      .channels_max = 2,
		     .rates = IMX_SSI_RATES,
		     .formats = IMX_SSI_FORMATS,
		     },
	 .ops = &imx_ssi_dai_ops,
	 .private_data = &imx_ssi_data[IMX_DAI_SSI2],
	 },
	{
	 .name = "imx-ssi-2-1",
	 .id = IMX_DAI_SSI3,
	 .probe = imx_ssi_probe,
	 .suspend = imx_ssi_suspend,
	 .remove = imx_ssi_remove,
	 .resume = imx_ssi_resume,
	 .playback = {
		      .channels_min = 1,
		      .channels_max = 2,
		      .rates = IMX_SSI_RATES,
		      .formats = IMX_SSI_FORMATS,
		      },
	 .capture = {
		     .channels_min = 1,
		     .channels_max = 2,
		     .rates = IMX_SSI_RATES,
		     .formats = IMX_SSI_FORMATS,
		     },
	 .ops = &imx_ssi_dai_ops,
	 .private_data = &imx_ssi_data[IMX_DAI_SSI3],
	 },
};

EXPORT_SYMBOL_GPL(imx_ssi_dai);

static int __init imx_ssi_init(void)
{
	return snd_soc_register_dais(imx_ssi_dai, ARRAY_SIZE(imx_ssi_dai));
}

static void __exit imx_ssi_exit(void)
{
	snd_soc_unregister_dais(imx_ssi_dai, ARRAY_SIZE(imx_ssi_dai));
}

module_init(imx_ssi_init);
module_exit(imx_ssi_exit);
MODULE_AUTHOR
    ("Liam Girdwood, liam.girdwood@wolfsonmicro.com, www.wolfsonmicro.com");
MODULE_DESCRIPTION("i.MX ASoC I2S driver");
MODULE_LICENSE("GPL");
