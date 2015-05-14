sc_errs,(int)p->scb->ovrn_errs,(int)p->scb->rus,(int)p->scb->cus);
	}
	old_at_least = at_least_one;
#endif

	if(debuglevel > 0)
		printk("r");
}

/**********************************************************
 * handle 'Receiver went not ready'.
 */

static void sun3_82586_rnr_int(struct net_device *dev)
{
	struct priv *p = netdev_priv(dev);

	p->stats.rx_errors++;

	WAIT_4_SCB_CMD();		/* wait for the last cmd, WAIT_4_FULLSTAT?? */
	p->scb->cmd_ruc = RUC_ABORT; /* usually the RU is in the 'no resource'-state .. abort it now. */
	sun3_attn586();
	WAIT_4_SCB_CMD_RUC();		/* wait for accept cmd. */

	alloc_rfa(dev,(char *)p->rfd_first);
/* maybe add a check here, before restarting the RU */
	startrecv586(dev); /* restart RU */

	printk("%s: Receive-Unit restarted. Status: %04x\n",dev->name,p->scb->rus);

}

/**********************************************************
 * handle xmit - interrupt
 */

static void sun3_82586_xmt_int(struct net_device *dev)
{
	int status;
	struct priv *p = netdev_priv(dev);

	if(debuglevel > 0)
		printk("X");

	status = swab16(p->xmit_cmds[p->xmit_last]->cmd_status);
	if(!(status & STAT_COMPL))
		printk("%s: strange .. xmit-int without a 'COMPLETE'\n",dev->name);

	if(status & STAT_OK)
	{
		p->stats.tx_packets++;
		p->stats.collisions += (status & TCMD_MAXCOLLMASK);
	}
	else
	{
		p->stats.tx_errors++;
		if(status & TCMD_LATECOLL) {
			printk("%s: late collision detected.\n",dev->name);
			p->stats.collisions++;
		}
		else if(status & TCMD_NOCARRIER) {
			p->stats.tx_carrier_errors++;
			printk("%s: no carrier detected.\n",dev->name);
		}
		else if(status & TCMD_LOSTCTS)
			printk("%s: loss of CTS detected.\n",dev->name);
		else if(status & TCMD_UNDERRUN) {
			p->stats.tx_fifo_errors++;
			printk("%s: DMA underrun detected.\n",dev->name);
		}
		else if(status & TCMD_MAXCOLL) {
			printk("%s: Max. collisions exceeded.\n",dev->name);
			p->stats.collisions += 16;
		}
	}

#if (NUM_XMIT_BUFFS > 1)
	if( (++p->xmit_last) == NUM_XMIT_BUFFS)
		p->xmit_last = 0;
#endif
	netif_wake_queue(dev);
}

/***********************************************************
 * (re)start the receiver
 */

static void startrecv586(struct net_device *dev)
{
	struct priv *p = netdev_priv(dev);

	WAIT_4_SCB_CMD();
	WAIT_4_SCB_CMD_RUC();
	p->scb->rfa_offset = make16(p->rfd_first);
	p->scb->cmd_ruc = RUC_START;
	sun3_attn586();		/* start cmd. */
	WAIT_4_SCB_CMD_RUC();	/* wait for accept cmd. (no timeout!!) */
}

static void sun3_82586_timeout(struct net_device *dev)
{
	struct priv *p = netdev_priv(dev);
#ifndef NO_NOPCOMMANDS
	if(p->scb->cus & CU_ACTIVE) /* COMMAND-UNIT active? */
	{
		netif_wake_queue(dev);
#ifdef DEBUG
		printk("%s: strange ... timeout with CU active?!?\n",dev->name);
		printk("%s: X0: %04x N0: %04x N1: %04x %d\n",dev->name,(int)swab16(p->xmit_cmds[0]->cmd_status),(int)swab16(p->nop_cmds[0]->cmd_status),(int)swab16(p->nop_cmds[1]->cmd_status),(int)p->nop_point);
#endif
		p->scb->cmd_cuc = CUC_ABORT;
		sun3_attn586();
		WAIT_4_SCB_CMD();
		p->scb->cbl_offset = make16(p->nop_cmds[p->nop_point]);
		p->scb->cmd_cuc = CUC_START;
		sun3_attn586();
		WAIT_4_SCB_CMD();
		dev->trans_start = jiffies;
		return 0;
	}
#endif
	{
#ifdef DEBUG
		printk("%s: xmitter timed out, try to restart! stat: %02x\n",dev->name,p->scb->cus);
		printk("%s: command-stats: %04x %04x\n",dev->name,swab16(p->xmit_cmds[0]->cmd_status),swab16(p->xmit_cmds[1]->cmd_status));
		printk("%s: check, whether you set the right interrupt number!\n",dev->name);
#endif
		sun3_82586_close(dev);
		sun3_82586_open(dev);
	}
	dev->trans_start = jiffies;
}

/******************************************************
 * send frame
 */

static int sun3_82586_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	int len,i;
#ifndef NO_NOPCOMMANDS
	int next_nop;
#endif
	struct priv *p = netdev_priv(dev);

	if(skb->len > XMIT_BUFF_SIZE)
	{
		printk("%s: Sorry, max. framelength is %d bytes. The length of your frame is %d bytes.\n",dev->name,XMIT_BUFF_SIZE,skb->len);
		return 0;
	}

	netif_stop_queue(dev);

#if(NUM_XMIT_BUFFS > 1)
	if(test_and_set_bit(0,(void *) &p->lock)) {
		printk("%s: Queue was locked\n",dev->name);
		return NETDEV_TX_BUSY;
	}
	else
#endif
	{
		len = skb->len;
		if (len < ETH_ZLEN) {
			memset((void *)p->xmit_cbuffs[p->xmit_count], 0,
			       ETH_ZLEN);
			len = ETH_ZLEN;
		}
		skb_copy_from_linear_data(skb, (void *)p->xmit_cbuffs[p->xmit_count], skb->len);

#if (NUM_XMIT_BUFFS == 1)
#	ifdef NO_NOPCOMMANDS

#ifdef DEBUG
		if(p->scb->cus & CU_ACTIVE)
		{
			printk("%s: Hmmm .. CU is still running and we wanna send a new packet.\n",dev->name);
			printk("%s: stat: %04x %04x\n",dev->name,p->scb->cus,swab16(p->xmit_cmds[0]->cmd_status));
		}
#endif

		p->xmit_buffs[0]->size = swab16(TBD_LAST | len);
		for(i=0;i<16;i++)
		{
			p->xmit_cmds[0]->cmd_status = 0;
			WAIT_4_SCB_CMD();
			if( (p->scb->cus & CU_STATUS) == CU_SUSPEND)
				p->scb->cmd_cuc = CUC_RESUME;
			else
			{
				p->scb->cbl_offset = make16(p->xmit_cmds[0]);
				p->scb->cmd_cuc = CUC_START;
			}

			sun3_attn586();
			dev->trans_start = jiffies;
			if(!i)
				dev_kfree_skb(skb);
			WAIT_4_SCB_CMD();
			if( (p->scb->cus & CU_ACTIVE)) /* test it, because CU sometimes doesn't start immediately */
				break;
			if(p->xmit_cmds[0]->cmd_status)
				break;
			if(i==15)
				printk("%s: Can't start transmit-command.\n",dev->name);
		}
#	else
		next_nop = (p->nop_point + 1) & 0x1;
		p->xmit_buffs[0]->size = swab16(TBD_LAST | len);

		p->xmit_cmds[0]->cmd_link	 = p->nop_cmds[next_nop]->cmd_link
			= make16((p->nop_cmds[next_nop]));
		p->xmit_cmds[0]->cmd_status = p->nop_cmds[next_nop]->cmd_status = 0;

		p->nop_cmds[p->nop_point]->cmd_link = make16((p->xmit_cmds[0]));
		dev->trans_start = jiffies;
		p->nop_point = next_nop;
		dev_kfree_skb(skb);
#	endif
#else
		p->xmit_buffs[p->xmit_count]->size = swab16(TBD_LAST | len);
		if( (next_nop = p->xmit_count + 1) == NUM_XMIT_BUFFS )
			next_nop = 0;

		p->xmit_cmds[p->xmit_count]->cmd_status	= 0;
		/* linkpointer of xmit-command already points to next nop cmd */
		p->nop_cmds[next_nop]->cmd_link = make16((p->nop_cmds[next_nop]));
		p->nop_cmds[next_nop]->cmd_status = 0;

		p->nop_cmds[p->xmit_count]->cmd_link = make16((p->xmit_cmds[p->xmit_count]));
		dev->trans_start = jiffies;
		p->xmit_count = next_nop;

		{
			unsigned long flags;
			local_irq_save(flags);
			if(p->xmit_count != p->xmit_last)
				netif_wake_queue(dev);
			p->lock = 0;
			local_irq_restore(flags);
		}
		dev_kfree_skb(skb);
#endif
	}
	return 0;
}

/*******************************************
 * Someone wanna have the statistics
 */

static struct net_device_stats *sun3_82586_get_stats(struct net_device *dev)
{
	struct priv *p = netdev_priv(dev);
	unsigned short crc,aln,rsc,ovrn;

	crc = swab16(p->scb->crc_errs); /* get error-statistic from the ni82586 */
	p->scb->crc_errs = 0;
	aln = swab16(p->scb->aln_errs);
	p->scb->aln_errs = 0;
	rsc = swab16(p->scb->rsc_errs);
	p->scb->rsc_errs = 0;
	ovrn = swab16(p->scb->ovrn_errs);
	p->scb->ovrn_errs = 0;

	p->stats.rx_crc_errors += crc;
	p->stats.rx_fifo_errors += ovrn;
	p->stats.rx_frame_errors += aln;
	p->stats.rx_dropped += rsc;

	return &p->stats;
}

/********************************************************
 * Set MC list ..
 */

static void set_multicast_list(struct net_device *dev)
{
	netif_stop_queue(dev);
	sun3_disint();
	alloc586(dev);
	init586(dev);
	startrecv586(dev);
	sun3_enaint();
	netif_wake_queue(dev);
}

#ifdef MODULE
#error This code is not currently supported as a module
static struct net_device *dev_sun3_82586;

int init_module(void)
{
	dev_sun3_82586 = sun3_82586_probe(-1);
	if (IS_ERR(dev_sun3_82586))
		return PTR_ERR(dev_sun3_82586);
	return 0;
}

void cleanup_module(void)
{
	unsigned long ioaddr = dev_sun3_82586->base_addr;
	unregister_netdev(dev_sun3_82586);
	release_region(ioaddr, SUN3_82586_TOTAL_SIZE);
	iounmap((void *)ioaddr);
	free_netdev(dev_sun3_82586);
}
#endif /* MODULE */

#if 0
/*
 * DUMP .. we expect a not running CMD unit and enough space
 */
void sun3_82586_dump(struct net_device *dev,void *ptr)
{
	struct priv *p = netdev_priv(dev);
	struct dump_cmd_struct *dump_cmd = (struct dump_cmd_struct *) ptr;
	int i;

	p->scb->cmd_cuc = CUC_ABORT;
	sun3_attn586();
	WAIT_4_SCB_CMD();
	WAIT_4_SCB_CMD_RUC();

	dump_cmd->cmd_status = 0;
	dump_cmd->cmd_cmd = CMD_DUMP | CMD_LAST;
	dump_cmd->dump_offset = make16((dump_cmd + 1));
	dump_cmd->cmd_link = 0xffff;

	p->scb->cbl_offset = make16(dump_cmd);
	p->scb->cmd_cuc = CUC_START;
	sun3_attn586();
	WAIT_4_STAT_COMPL(dump_cmd);

	if( (dump_cmd->cmd_status & (STAT_COMPL|STAT_OK)) != (STAT_COMPL|STAT_OK) )
				printk("%s: Can't get dump information.\n",dev->name);

	for(i=0;i<170;i++) {
		printk("%02x ",(int) ((unsigned char *) (dump_cmd + 1))[i]);
		if(i % 24 == 23)
			printk("\n");
	}
	printk("\n");
}
#endif

MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           #ifndef	_CM4040_H_
#define	_CM4040_H_

#define	CM_MAX_DEV		4

#define	DEVICE_NAME		"cmx"
#define	MODULE_NAME		"cm4040_cs"

#define REG_OFFSET_BULK_OUT      0
#define REG_OFFSET_BULK_IN       0
#define REG_OFFSET_BUFFER_STATUS 1
#define REG_OFFSET_SYNC_CONTROL  2

#define BSR_BULK_IN_FULL  0x02
#define BSR_BULK_OUT_FULL 0x01

#define SCR_HOST_TO_READER_START 0x80
#define SCR_ABORT                0x40
#define SCR_EN_NOTIFY            0x20
#define SCR_ACK_NOTIFY           0x10
#define SCR_READER_TO_HOST_DONE  0x08
#define SCR_HOST_TO_READER_DONE  0x04
#define SCR_PULSE_INTERRUPT      0x02
#define SCR_POWER_DOWN           0x01


#define  CMD_PC_TO_RDR_ICCPOWERON       0x62
#define  CMD_PC_TO_RDR_GETSLOTSTATUS    0x65
#define  CMD_PC_TO_RDR_ICCPOWEROFF      0x63
#define  CMD_PC_TO_RDR_SECURE           0x69
#define  CMD_PC_TO_RDR_GETPARAMETERS    0x6C
#define  CMD_PC_TO_RDR_RESETPARAMETERS  0x6D
#define  CMD_PC_TO_RDR_SETPARAMETERS    0x61
#define  CMD_PC_TO_RDR_XFRBLOCK         0x6F
#define  CMD_PC_TO_RDR_ESCAPE           0x6B
#define  CMD_PC_TO_RDR_ICCCLOCK         0x6E
#define  CMD_PC_TO_RDR_TEST_SECURE      0x74
#define  CMD_PC_TO_RDR_OK_SECURE        0x89


#define  CMD_RDR_TO_PC_SLOTSTATUS         0x81
#define  CMD_RDR_TO_PC_DATABLOCK          0x80
#define  CMD_RDR_TO_PC_PARAMETERS         0x82
#define  CMD_RDR_TO_PC_ESCAPE             0x83
#define  CMD_RDR_TO_PC_OK_SECURE          0x89

#endif	/* _CM4040_H_ */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     /*
 * Copyright (c) 2006, 2007 QLogic Corporation. All rights reserved.
 * Copyright (c) 2003, 2004, 2005, 2006 PathScale, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _IPATH_REGISTERS_H
#define _IPATH_REGISTERS_H

/*
 * This file should only be included by kernel source, and by the diags.  It
 * defines the registers, and their contents, for InfiniPath chips.
 */

/*
 * These are the InfiniPath register and buffer bit definitions,
 * that are visible to software, and needed only by the kernel
 * and diag code.  A few, that are visible to protocol and user
 * code are in ipath_common.h.  Some bits are specific
 * to a given chip implementation, and have been moved to the
 * chip-specific source file
 */

/* kr_revision bits */
#define INFINIPATH_R_CHIPREVMINOR_MASK 0xFF
#define INFINIPATH_R_CHIPREVMINOR_SHIFT 0
#define INFINIPATH_R_CHIPREVMAJOR_MASK 0xFF
#define INFINIPATH_R_CHIPREVMAJOR_SHIFT 8
#define INFINIPATH_R_ARCH_MASK 0xFF
#define INFINIPATH_R_ARCH_SHIFT 16
#define INFINIPATH_R_SOFTWARE_MASK 0xFF
#define INFINIPATH_R_SOFTWARE_SHIFT 24
#define INFINIPATH_R_BOARDID_MASK 0xFF
#define INFINIPATH_R_BOARDID_SHIFT 32

/* kr_control bits */
#define INFINIPATH_C_FREEZEMODE 0x00000002
#define INFINIPATH_C_LINKENABLE 0x00000004

/* kr_sendctrl bits */
#define INFINIPATH_S_DISARMPIOBUF_SHIFT 16
#define INFINIPATH_S_UPDTHRESH_SHIFT 24
#define INFINIPATH_S_UPDTHRESH_MASK 0x1f

#define IPATH_S_ABORT		0
#define IPATH_S_PIOINTBUFAVAIL	1
#define IPATH_S_PIOBUFAVAILUPD	2
#define IPATH_S_PIOENABLE	3
#define IPATH_S_SDMAINTENABLE	9
#define IPATH_S_SDMASINGLEDESCRIPTOR	10
#define IPATH_S_SDMAENABLE	11
#define IPATH_S_SDMAHALT	12
#define IPATH_S_DISARM		31

#define INFINIPATH_S_ABORT		(1U << IPATH_S_ABORT)
#define INFINIPATH_S_PIOINTBUFAVAIL	(1U << IPATH_S_PIOINTBUFAVAIL)
#define INFINIPATH_S_PIOBUFAVAILUPD	(1U << IPATH_S_PIOBUFAVAILUPD)
#define INFINIPATH_S_PIOENABLE		(1U << IPATH_S_PIOENABLE)
#define INFINIPATH_S_SDMAINTENABLE	(1U << IPATH_S_SDMAINTENABLE)
#define INFINIPATH_S_SDMASINGLEDESCRIPTOR \
					(1U << IPATH_S_SDMASINGLEDESCRIPTOR)
#define INFINIPATH_S_SDMAENABLE		(1U << IPATH_S_SDMAENABLE)
#define INFINIPATH_S_SDMAHALT		(1U << IPATH_S_SDMAHALT)
#define INFINIPATH_S_DISARM		(1U << IPATH_S_DISARM)

/* kr_rcvctrl bits that are the same on multiple chips */
#define INFINIPATH_R_PORTENABLE_SHIFT 0
#define INFINIPATH_R_QPMAP_ENABLE (1ULL << 38)

/* kr_intstatus, kr_intclear, kr_intmask bits */
#define INFINIPATH_I_SDMAINT		0x8000000000000000ULL
#define INFINIPATH_I_SDMADISABLED	0x4000000000000000ULL
#define INFINIPATH_I_ERROR		0x0000000080000000ULL
#define INFINIPATH_I_SPIOSENT		0x0000000040000000ULL
#define INFINIPATH_I_SPIOBUFAVAIL	0x0000000020000000ULL
#define INFINIPATH_I_GPIO		0x0000000010000000ULL
#define INFINIPATH_I_JINT		0x0000000004000000ULL

/* kr_errorstatus, kr_errorclear, kr_errormask bits */
#define INFINIPATH_E_RFORMATERR			0x0000000000000001ULL
#define INFINIPATH_E_RVCRC			0x0000000000000002ULL
#define INFINIPATH_E_RICRC			0x0000000000000004ULL
#define INFINIPATH_E_RMINPKTLEN			0x0000000000000008ULL
#define INFINIPATH_E_RMAXPKTLEN			0x0000000000000010ULL
#define INFINIPATH_E_RLONGPKTLEN		0x0000000000000020ULL
#define INFINIPATH_E_RSHORTPKTLEN		0x0000000000000040ULL
#define INFINIPATH_E_RUNEXPCHAR			0x0000000000000080ULL
#define INFINIPATH_E_RUNSUPVL			0x0000000000000100ULL
#define INFINIPATH_E_REBP			0x0000000000000200ULL
#define INFINIPATH_E_RIBFLOW			0x0000000000000400ULL
#define INFINIPATH_E_RBADVERSION		0x0000000000000800ULL
#define INFINIPATH_E_RRCVEGRFULL		0x0000000000001000ULL
#define INFINIPATH_E_RRCVHDRFULL		0x0000000000002000ULL
#define INFINIPATH_E_RBADTID			0x0000000000004000ULL
#define INFINIPATH_E_RHDRLEN			0x0000000000008000ULL
#define INFINIPATH_E_RHDR			0x0000000000010000ULL
#define INFINIPATH_E_RIBLOSTLINK		0x0000000000020000ULL
#define INFINIPATH_E_SENDSPECIALTRIGGER		0x0000000008000000ULL
#define INFINIPATH_E_SDMADISABLED		0x0000000010000000ULL
#define INFINIPATH_E_SMINPKTLEN			0x0000000020000000ULL
#define INFINIPATH_E_SMAXPKTLEN			0x0000000040000000ULL
#define INFINIPATH_E_SUNDERRUN			0x0000000080000000ULL
#define INFINIPATH_E_SPKTLEN			0x0000000100000000ULL
#define INFINIPATH_E_SDROPPEDSMPPKT		0x0000000200000000ULL
#define INFINIPATH_E_SDROPPEDDATAPKT		0x0000000400000000ULL
#define INFINIPATH_E_SPIOARMLAUNCH		0x0000000800000000ULL
#define INFINIPATH_E_SUNEXPERRPKTNUM		0x0000001000000000ULL
#define INFINIPATH_E_SUNSUPVL			0x0000002000000000ULL
#define INFINIPATH_E_SENDBUFMISUSE		0x0000004000000000ULL
#define INFINIPATH_E_SDMAGENMISMATCH		0x0000008000000000ULL
#define INFINIPATH_E_SDMAOUTOFB