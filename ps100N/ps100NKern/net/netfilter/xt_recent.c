/*
 * Copyright (c) 2006 Patrick McHardy <kaber@trash.net>
 * Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This is a replacement of the old ipt_recent module, which carried the
 * following copyright notice:
 *
 * Author: Stephen Frost <sfrost@snowman.net>
 * Copyright 2002-2003, Stephen Frost, 2.5.x port by laforge@netfilter.org
 */
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/list.h>
#include <linux/random.h>
#include <linux/jhash.h>
#include <linux/bitops.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <net/net_namespace.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_recent.h>

MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_AUTHOR("Jan Engelhardt <jengelh@computergmbh.de>");
MODULE_DESCRIPTION("Xtables: \"recently-seen\" host matching for IPv4");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_recent");
MODULE_ALIAS("ip6t_recent");

static unsigned int ip_list_tot = 100;
static unsigned int ip_pkt_list_tot = 20;
static unsigned int ip_list_hash_size = 0;
static unsigned int ip_list_perms = 0644;
static unsigned int ip_list_uid = 0;
static unsigned int ip_list_gid = 0;
module_param(ip_list_tot, uint, 0400);
module_param(ip_pkt_list_tot, uint, 0400);
module_param(ip_list_hash_size, uint, 0400);
module_param(ip_list_perms, uint, 0400);
module_param(ip_list_uid, uint, 0400);
module_param(ip_list_gid, uint, 0400);
MODULE_PARM_DESC(ip_list_tot, "number of IPs to remember per list");
MODULE_PARM_DESC(ip_pkt_list_tot, "number of packets per IP to remember (max. 255)");
MODULE_PARM_DESC(ip_list_hash_size, "size of hash table used to look up IPs");
MODULE_PARM_DESC(ip_list_perms, "permissions on /proc/net/xt_recent/* files");
MODULE_PARM_DESC(ip_list_uid,"owner of /proc/net/xt_recent/* files");
MODULE_PARM_DESC(ip_list_gid,"owning group of /proc/net/xt_recent/* files");

struct recent_entry {
	struct list_head	list;
	struct list_head	lru_list;
	union nf_inet_addr	addr;
	u_int16_t		family;
	u_int8_t		ttl;
	u_int8_t		index;
	u_int16_t		nstamps;
	unsigned long		stamps[0];
};

struct recent_table {
	struct list_head	list;
	char			name[XT_RECENT_NAME_LEN];
	unsigned int		refcnt;
	unsigned int		entries;
	struct list_head	lru_list;
	struct list_head	iphash[0];
};

static LIST_HEAD(tables);
static DEFINE_SPINLOCK(recent_lock);
static DEFINE_MUTEX(recent_mutex);

#ifdef CONFIG_PROC_FS
#ifdef CONFIG_NETFILTER_XT_MATCH_RECENT_PROC_COMPAT
static struct proc_dir_entry *proc_old_dir;
#endif
static struct proc_dir_entry *recent_proc_dir;
static const struct file_operations recent_old_fops, recent_mt_fops;
#endif

static u_int32_t hash_rnd;
static bool hash_rnd_initted;

static unsigned int recent_entry_hash4(const union nf_inet_addr *addr)
{
	if (!hash_rnd_initted) {
		get_random_bytes(&hash_rnd, sizeof(hash_rnd));
		hash_rnd_initted = true;
	}
	return jhash_1word((__force u32)addr->ip, hash_rnd) &
	       (ip_list_hash_size - 1);
}

static unsigned int recent_entry_hash6(const union nf_inet_addr *addr)
{
	if (!hash_rnd_initted) {
		get_random_bytes(&hash_rnd, sizeof(hash_rnd));
		hash_rnd_initted = true;
	}
	return jhash2((u32 *)addr->ip6, ARRAY_SIZE(addr->ip6), hash_rnd) &
	       (ip_list_hash_size - 1);
}

static struct recent_entry *
recent_entry_lookup(const struct recent_table *table,
		    const union nf_inet_addr *addrp, u_int16_t family,
		    u_int8_t ttl)
{
	struct recent_entry *e;
	unsigned int h;

	if (family == NFPROTO_IPV4)
		h = recent_entry_hash4(addrp);
	else
		h = recent_entry_hash6(addrp);

	list_for_each_entry(e, &table->iphash[h], list)
		if (e->family == family &&
		    memcmp(&e->addr, addrp, sizeof(e->addr)) == 0 &&
		    (ttl == e->ttl || ttl == 0 || e->ttl == 0))
			return e;
	return NULL;
}

static void recent_entry_remove(struct recent_table *t, struct recent_entry *e)
{
	list_del(&e->list);
	list_del(&e->lru_list);
	kfree(e);
	t->entries--;
}

static struct recent_entry *
recent_entry_init(struct recent_table *t, const union nf_inet_addr *addr,
		  u_int16_t family, u_int8_t ttl)
{
	struct recent_entry *e;

	if (t->entries >= ip_list_tot) {
		e = list_entry(t->lru_list.next, struct recent_entry, lru_list);
		recent_entry_remove(t, e);
	}
	e = kmalloc(sizeof(*e) + sizeof(e->stamps[0]) * ip_pkt_list_tot,
		    GFP_ATOMIC);
	if (e == NULL)
		return NULL;
	memcpy(&e->addr, addr, sizeof(e->addr));
	e->ttl       = ttl;
	e->stamps[0] = jiffies;
	e->nstamps   = 1;
	e->index     = 1;
	e->family    = family;
	if (family == NFPROTO_IPV4)
		list_add_tail(&e->list, &t->iphash[recent_entry_hash4(addr)]);
	else
		list_add_tail(&e->list, &t->iphash[recent_entry_hash6(addr)]);
	list_add_tail(&e->lru_list, &t->lru_list);
	t->entries++;
	return e;
}

static void recent_entry_update(struct recent_table *t, struct recent_entry *e)
{
	e->stamps[e->index++] = jiffies;
	if (e->index > e->nstamps)
		e->nstamps = e->index;
	e->index %= ip_pkt_list_tot;
	list_move_tail(&e->lru_list, &t->lru_list);
}

static struct recent_table *recent_table_lookup(const char *name)
{
	struct recent_table *t;

	list_for_each_entry(t, &tables, list)
		if (!strcmp(t->name, name))
			return t;
	return NULL;
}

static void recent_table_flush(struct recent_table *t)
{
	struct recent_entry *e, *next;
	unsigned int i;

	for (i = 0; i < ip_list_hash_size; i++)
		list_for_each_entry_safe(e, next, &t->iphash[i], list)
			recent_entry_remove(t, e);
}

static bool
recent_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_recent_mtinfo *info = par->matchinfo;
	struct recent_table *t;
	struct recent_entry *e;
	union nf_inet_addr addr = {};
	u_int8_t ttl;
	bool ret = info->invert;

	if (par->match->family == NFPROTO_IPV4) {
		const struct iphdr *iph = ip_hdr(skb);

		if (info->side == XT_RECENT_DEST)
			addr.ip = iph->daddr;
		else
			addr.ip = iph->saddr;

		ttl = iph->ttl;
	} else {
		const struct ipv6hdr *iph = ipv6_hdr(skb);

		if (info->side == XT_RECENT_DEST)
			memcpy(&addr.in6, &iph->daddr, sizeof(addr.in6));
		else
			memcpy(&addr.in6, &iph->saddr, sizeof(addr.in6));

		ttl = iph->hop_limit;
	}

	/* use TTL as seen before forwarding */
	if (par->out != NULL && skb->sk == NULL)
		ttl++;

	spin_lock_bh(&recent_lock);
	t = recent_table_lookup(info->name);
	e = recent_entry_lookup(t, &addr, par->match->family,
				(info->check_set & XT_RECENT_TTL) ? ttl : 0);
	if (e == NULL) {
		if (!(info->check_set & XT_RECENT_SET))
			goto out;
		e = recent_entry_init(t, &addr, par->match->family, ttl);
		if (e == NULL)
			*par->hotdrop = true;
		ret = !ret;
		goto out;
	}

	if (info->check_set & XT_RECENT_SET)
		ret = !ret;
	else if (info->check_set & XT_RECENT_REMOVE) {
		recent_entry_remove(t, e);
		ret = !ret;
	} else if (info->check_set & (XT_RECENT_CHECK | XT_RECENT_UPDATE)) {
		unsigned long time = jiffies - info->seconds * HZ;
		unsigned int i, hits = 0;

		for (i = 0; i < e->nstamps; i++) {
			if (info->seconds && time_after(time, e->stamps[i]))
				continue;
			if (++hits >= info->hit_count) {
				ret = !ret;
				break;
			}
		}
	}

	if (info->check_set & XT_RECENT_SET ||
	    (info->check_set & XT_RECENT_UPDATE && ret)) {
		recent_entry_update(t, e);
		e->ttl = ttl;
	}
out:
	spin_unlock_bh(&recent_lock);
	return ret;
}

static bool recent_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_recent_mtinfo *info = par->matchinfo;
	struct recent_table *t;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *pde;
#endif
	unsigned i;
	bool ret = false;

	if (hweight8(info->check_set &
		     (XT_RECENT_SET | XT_RECENT_REMOVE |
		      XT_RECENT_CHECK | XT_RECENT_UPDATE)) != 1)
		return false;
	if ((info->check_set & (XT_RECENT_SET | XT_RECENT_REMOVE)) &&
	    (info->seconds || info->hit_count))
		return false;
	if (info->hit_count > ip_pkt_list_tot)
		return false;
	if (info->name[0] == '\0' ||
	    strnlen(infomand(lu, scsi_status, SCpnt,
					  cmd->Current_done);
	return RCODE_COMPLETE;
}

/**************************************
 * SCSI interface related section
 **************************************/

static int sbp2scsi_queuecommand(struct scsi_cmnd *SCpnt,
				 void (*done)(struct scsi_cmnd *))
{
	struct sbp2_lu *lu = (struct sbp2_lu *)SCpnt->device->host->hostdata[0];
	struct sbp2_fwhost_info *hi;
	int result = DID_NO_CONNECT << 16;

	if (unlikely(!sbp2util_node_is_available(lu)))
		goto done;

	hi = lu->hi;

	if (unlikely(!hi)) {
		SBP2_ERR("sbp2_fwhost_info is NULL - this is bad!");
		goto done;
	}

	/* Multiple units are currently represented to the SCSI core as separate
	 * targets, not as one target with multiple LUs. Therefore return
	 * selection time-out to any IO directed at non-zero LUNs. */
	if (unlikely(SCpnt->device->lun))
		goto done;

	if (unlikely(!hpsb_node_entry_valid(lu->ne))) {
		SBP2_ERR("Bus reset in progress - rejecting command");
		result = DID_BUS_BUSY << 16;
		goto done;
	}

	/* Bidirectional commands are not yet implemented,
	 * and unknown transfer direction not handled. */
	if (unlikely(SCpnt->sc_data_direction == DMA_BIDIRECTIONAL)) {
		SBP2_ERR("Cannot handle DMA_BIDIRECTIONAL - rejecting command");
		result = DID_ERROR << 16;
		goto done;
	}

	if (sbp2_send_command(lu, SCpnt, done)) {
		SBP2_ERR("Error sending SCSI command");
		sbp2scsi_complete_command(lu,
					  SBP2_SCSI_STATUS_SELECTION_TIMEOUT,
					  SCpnt, done);
	}
	return 0;

done:
	SCpnt->result = result;
	done(SCpnt);
	return 0;
}

static void sbp2scsi_complete_all_commands(struct sbp2_lu *lu, u32 status)
{
	struct list_head *lh;
	struct sbp2_command_info *cmd;
	unsigned long flags;

	spin_lock_irqsave(&lu->cmd_orb_lock, flags);
	while (!list_empty(&lu->cmd_orb_inuse)) {
		lh = lu->cmd_orb_inuse.next;
		cmd = list_entry(lh, struct sbp2_command_info, list);
		sbp2util_mark_command_completed(lu, cmd);
		if (cmd->Current_SCpnt) {
			cmd->Current_SCpnt->result = status << 16;
			cmd->Current_done(cmd->Current_SCpnt);
		}
	}
	spin_unlock_irqrestore(&lu->cmd_orb_lock, flags);

	return;
}

/*
 * Complete a regular SCSI command. Can be called in atomic context.
 */
static void sbp2scsi_complete_command(struct sbp2_lu *lu, u32 scsi_status,
				      struct scsi_cmnd *SCpnt,
				      void (*done)(struct scsi_cmnd *))
{
	if (!SCpnt) {
		SBP2_ERR("SCpnt is NULL");
		return;
	}

	switch (scsi_status) {
	case SBP2_SCSI_STATUS_GOOD:
		SCpnt->result = DID_OK << 16;
		break;

	case SBP2_SCSI_STATUS_BUSY:
		SBP2_ERR("SBP2_SCSI_STATUS_BUSY");
		SCpnt->result = DID_BUS_BUSY << 16;
		break;

	case SBP2_SCSI_STATUS_CHECK_CONDITION:
		SCpnt->result = CHECK_CONDITION << 1 | DID_OK << 16;
		break;

	case SBP2_SCSI_STATUS_SELECTION_TIMEOUT:
		SBP2_ERR("SBP2_SCSI_STATUS_SELECTION_TIMEOUT");
		SCpnt->result = DID_NO_CONNECT << 16;
		scsi_print_command(SCpnt);
		break;

	case SBP2_SCSI_STATUS_CONDITION_MET:
	case SBP2_SCSI_STATUS_RESERVATION_CONFLICT:
	case SBP2_SCSI_STATUS_COMMAND_TERMINATED:
		SBP2_ERR("Bad SCSI status = %x", scsi_status);
		SCpnt->result = DID_ERROR << 16;
		scsi_print_command(SCpnt);
		break;

	default:
		SBP2_ERR("Unsupported SCSI status = %x", scsi_status);
		SCpnt->result = DID_ERROR << 16;
	}

	/* If a bus reset is in progress and there was an error, complete
	 * the command as busy so that it will get retried. */
	if (!hpsb_node_entry_valid(lu->ne)
	    && (scsi_status != SBP2_SCSI_STATUS_GOOD)) {
		SBP2_ERR("Completing command with busy (bus reset)");
		SCpnt->result = DID_BUS_BUSY << 16;
	}

	/* Tell the SCSI stack that we're done with this command. */
	done(SCpnt);
}

static int sbp2scsi_slave_alloc(struct scsi_device *sdev)
{
	struct sbp2_lu *lu = (struct sbp2_lu *)sdev->host->hostdata[0];

	if (sdev->lun != 0 || sdev->id != lu->ud->id || sdev->channel != 0)
		return -ENODEV;

	lu->sdev = sdev;
	sdev->allow_restart = 1;

	/* SBP-2 requires quadlet alignment of the data buffers. */
	blk_queue_update_dma_alignment(sdev->request_queue, 4 - 1);

	if (lu->workarounds & SBP2_WORKAROUND_INQUIRY_36)
		sdev->inquiry_len = 36;
	return 0;
}

static int sbp2scsi_slave_configure(struct scsi_device *sdev)
{
	struct sbp2_lu *lu = (struct sbp2_lu *)sdev->host->hostdata[0];

	sdev->use_10_for_rw = 1;

	if (sbp2_exclusive_login)
		sdev->manage_start_stop = 1;
	if (sdev->type == TYPE_ROM)
		sdev->use_10_for_ms = 1;
	if (sdev->type == TYPE_DISK &&
	    lu->workarounds & SBP2_WORKAROUND_MODE_SENSE_8)
		sdev->skip_ms_page_8 = 1;
	if (lu->workarounds & SBP2_WORKAROUND_FIX_CAPACITY)
		sdev->fix_capacity = 1;
	if (lu->workarounds & SBP2_WORKAROUND_POWER_CONDITION)
		sdev->start_stop_pwr_cond = 1;
	if (lu->workarounds & SBP2_WORKAROUND_128K_MAX_TRANS)
		blk_queue_max_sectors(sdev->request_queue, 128 * 1024 / 512);

	blk_queue_max_segment_size(sdev->request_queue, SBP2_MAX_SEG_SIZE);
	return 0;
}

static void sbp2scsi_slave_destroy(struct scsi_device *sdev)
{
	((struct sbp2_lu *)sdev->host->hostdata[0])->sdev = NULL;
	return;
}

/*
 * Called by scsi stack when something has really gone wrong.
 * Usually called when a command has timed-out for some reason.
 */
static int sbp2scsi_abort(struct scsi_cmnd *SCpnt)
{
	struct sbp2_lu *lu = (struct sbp2_lu *)SCpnt->device->host->hostdata[0];
	struct sbp2_command_info *cmd;
	unsigned long flags;

	SBP2_INFO("aborting sbp2 command");
	scsi_print_command(SCpnt);

	if (sbp2util_node_is_available(lu)) {
		sbp2_agent_reset(lu, 1);

		/* Return a matching command structure to the free pool. */
		spin_lock_irqsave(&lu->cmd_orb_lock, flags);
		cmd = sbp2util_find_command_for_SCpnt(lu, SCpnt);
		if (cmd) {
			sbp2util_mark_command_completed(lu, cmd);
			if (cmd->Current_SCpnt) {
				cmd->Current_SCpnt->result = DID_ABORT << 16;
				cmd->Current_done(cmd->Current_SCpnt);
			}
		}
		spin_unlock_irqrestore(&lu->cmd_orb_lock, flags);

		sbp2scsi_complete_all_commands(lu, DID_BUS_BUSY);
	}

	return SUCCESS;
}

/*
 * Called by scsi stack when something has really gone wrong.
 */
static int sbp2scsi_reset(struct scsi_cmnd *SCpnt)
{
	struct sbp2_lu *lu = (struct sbp2_lu *)SCpnt->device->host->hostdata[0];

	SBP2_INFO("reset requested");

	if (sbp2util_node_is_available(lu)) {
		SBP2_INFO("generating sbp2 fetch agent reset");
		sbp2_agent_reset(lu, 1);
	}

	return SUCCESS;
}

static ssize_t sbp2_sysfs_ieee1394_id_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct scsi_device *sdev;
	struct sbp2_lu *lu;

	if (!(sdev = to_scsi_device(dev)))
		return 0;

	if (!(lu = (struct sbp2_lu *)sdev->host->hostdata[0]))
		return 0;

	if (sbp2_long_sysfs_ieee1394_id)
		return sprintf(buf, "%016Lx:%06x:%04x\n",
				(unsigned long long)lu->ne->guid,
				lu->ud->directory_id, ORB_SET_LUN(lu->lun));
	else
		return sprintf(buf, "%016Lx:%d:%d\n",
				(unsigned long long)lu->ne->guid,
				lu->ud->id, ORB_SET_LUN(lu->lun));
}

MODULE_AUTHOR("Ben Collins <bcollins@debian.org>");
MODULE_DESCRIPTION("IEEE-1394 SBP-2 protocol driver");
MODULE_SUPPORTED_DEVICE(SBP2_DEVICE_NAME);
MODULE_LICENSE("GPL");

static int sbp2_module_init(void)
{
	int ret;

	if (sbp2_serialize_io) {
		sbp2_shost_template.can_queue = 1;
		sbp2_shost_template.cmd_per_lun = 1;
	}

	sbp2_shost_template.max_sectors = sbp2_max_sectors;

	hpsb_register_highlevel(&sbp2_highlevel);
	ret = hpsb_register_protocol(&sbp2_driver);
	if (ret) {
		SBP2_ERR("Failed to register protocol");
		hpsb_unregister_highlevel(&sbp2_highlevel);
		return ret;
	}
	return 0;
}

static void __exit sbp2_module_exit(void)
{
	hpsb_unregister_protocol(&sbp2_driver);
	hpsb_unregister_highlevel(&sbp2_highlevel);
}

module_init(sbp2_module_init);
module_exit(sbp2_module_exit);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                /* linux/drivers/serial/s3c24a0.c
 *
 * Driver for Samsung S3C24A0 SoC onboard UARTs.
 *
 * Based on drivers/serial/s3c2410.c
 *
 * Author: Sandeep Patil <sandeep.patil@azingo.com>
 *
 * Ben Dooks, Copyright (c) 2003-2005,2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/io.h>
#include <linux/irq.h>

#include <mach/hardware.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>

#include "samsung.h"

static int s3c24a0_serial_setsource(struct uart_port *port,
				    struct s3c24xx_uart_clksrc *clk)
{
	unsigned long ucon = rd_regl(port, S3C2410_UCON);

	if (strcmp(clk->name, "uclk") =