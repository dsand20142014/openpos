/*
 * Copyright (c) 2003+ Evgeniy Polyakov <zbr@ioremap.net>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/if.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/tcp.h>

#include <net/ip.h>
#include <net/tcp.h>

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_log.h>
#include <linux/netfilter/xt_osf.h>

struct xt_osf_finger {
	struct rcu_head			rcu_head;
	struct list_head		finger_entry;
	struct xt_osf_user_finger	finger;
};

enum osf_fmatch_states {
	/* Packet does not match the fingerprint */
	FMATCH_WRONG = 0,
	/* Packet matches the fingerprint */
	FMATCH_OK,
	/* Options do not match the fingerprint, but header does */
	FMATCH_OPT_WRONG,
};

/*
 * Indexed by dont-fragment bit.
 * It is the only constant value in the fingerprint.
 */
static struct list_head xt_osf_fingers[2];

static const struct nla_policy xt_osf_policy[OSF_ATTR_MAX + 1] = {
	[OSF_ATTR_FINGER]	= { .len = sizeof(struct xt_osf_user_finger) },
};

static void xt_osf_finger_free_rcu(struct rcu_head *rcu_head)
{
	struct xt_osf_finger *f = container_of(rcu_head, struct xt_osf_finger, rcu_head);

	kfree(f);
}

static int xt_osf_add_callback(struct sock *ctnl, struct sk_buff *skb,
			struct nlmsghdr *nlh, struct nlattr *osf_attrs[])
{
	struct xt_osf_user_finger *f;
	struct xt_osf_finger *kf = NULL, *sf;
	int err = 0;

	if (!osf_attrs[OSF_ATTR_FINGER])
		return -EINVAL;

	if (!(nlh->nlmsg_flags & NLM_F_CREATE))
		return -EINVAL;

	f = nla_data(osf_attrs[OSF_ATTR_FINGER]);

	kf = kmalloc(sizeof(struct xt_osf_finger), GFP_KERNEL);
	if (!kf)
		return -ENOMEM;

	memcpy(&kf->finger, f, sizeof(struct xt_osf_user_finger));

	list_for_each_entry(sf, &xt_osf_fingers[!!f->df], finger_entry) {
		if (memcmp(&sf->finger, f, sizeof(struct xt_osf_user_finger)))
			continue;

		kfree(kf);
		kf = NULL;

		if (nlh->nlmsg_flags & NLM_F_EXCL)
			err = -EEXIST;
		break;
	}

	/*
	 * We are protected by nfnl mutex.
	 */
	if (kf)
		list_add_tail_rcu(&kf->finger_entry, &xt_osf_fingers[!!f->df]);

	return err;
}

static int xt_osf_remove_callback(struct sock *ctnl, struct sk_buff *skb,
			struct nlmsghdr *nlh, struct nlattr *osf_attrs[])
{
	struct xt_osf_user_finger *f;
	struct xt_osf_finger *sf;
	int err = ENOENT;

	if (!osf_attrs[OSF_ATTR_FINGER])
		return -EINVAL;

	f = nla_data(osf_attrs[OSF_ATTR_FINGER]);

	list_for_each_entry(sf, &xt_osf_fingers[!!f->df], finger_entry) {
		if (memcmp(&sf->finger, f, sizeof(struct xt_osf_user_finger)))
			continue;

		/*
		 * We are protected by nfnl mutex.
		 */
		list_del_rcu(&sf->finger_entry);
		call_rcu(&sf->rcu_head, xt_osf_finger_free_rcu);

		err = 0;
		break;
	}

	return err;
}

static const struct nfnl_callback xt_osf_nfnetlink_callbacks[OSF_MSG_MAX] = {
	[OSF_MSG_ADD]	= {
		.call		= xt_osf_add_callback,
		.attr_count	= OSF_ATTR_MAX,
		.policy		= xt_osf_policy,
	},
	[OSF_MSG_REMOVE]	= {
		.call		= xt_osf_remove_callback,
		.attr_count	= OSF_ATTR_MAX,
		.policy		= xt_osf_policy,
	},
};

static const struct nfnetlink_subsystem xt_osf_nfnetlink = {
	.name			= "osf",
	.subsys_id		= NFNL_SUBSYS_OSF,
	.cb_count		= OSF_MSG_MAX,
	.cb			= xt_osf_nfnetlink_callbacks,
};

static inline int xt_osf_ttl(const struct sk_buff *skb, const struct xt_osf_VATE_BASE + 5)
#define V4L2_CID_PRIVATE_WHITECRUSH_UPPER   (V4L2_CID_PRIVATE_BASE + 6)
#define V4L2_CID_PRIVATE_WHITECRUSH_LOWER   (V4L2_CID_PRIVATE_BASE + 7)
#define V4L2_CID_PRIVATE_UV_RATIO    (V4L2_CID_PRIVATE_BASE + 8)
#define V4L2_CID_PRIVATE_FULL_LUMA_RANGE    (V4L2_CID_PRIVATE_BASE + 9)
#define V4L2_CID_PRIVATE_CORING      (V4L2_CID_PRIVATE_BASE + 10)
#define V4L2_CID_PRIVATE_LASTP1      (V4L2_CID_PRIVATE_BASE + 11)

static const struct v4l2_queryctrl no_ctl = {
	.name  = "42",
	.flags = V4L2_CTRL_FLAG_DISABLED,
};
static const struct v4l2_queryctrl bttv_ctls[] = {
	/* --- video --- */
	{
		.id            = V4L2_CID_BRIGHTNESS,
		.name          = "Brightness",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 256,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_CONTRAST,
		.name          = "Contrast",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 128,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_SATURATION,
		.name          = "Saturation",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 128,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_HUE,
		.name          = "Hue",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 256,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},
	/* --- audio --- */
	{
		.id            = V4L2_CID_AUDIO_MUTE,
		.name          = "Mute",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_AUDIO_VOLUME,
		.name          = "Volume",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 65535/100,
		.default_value = 65535,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_AUDIO_BALANCE,
		.name          = "Balance",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 65535/100,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_AUDIO_BASS,
		.name          = "Bass",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 65535/100,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_AUDIO_TREBLE,
		.name          = "Treble",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 65535/100,
		.default_value = 32768,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},
	/* --- private --- */
	{
		.id            = V4L2_CID_PRIVATE_CHROMA_AGC,
		.name          = "chroma agc",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_PRIVATE_COMBFILTER,
		.name          = "combfilter",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_PRIVATE_AUTOMUTE,
		.name          = "automute",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_PRIVATE_LUMAFILTER,
		.name          = "luma decimation filter",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_PRIVATE_AGC_CRUSH,
		.name          = "agc crush",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_PRIVATE_VCR_HACK,
		.name          = "vcr hack",
		.minimum       = 0,
		.maximum       = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	},{
		.id            = V4L2_CID_PRIVATE_WHITECRUSH_UPPER,
		.name          = "whitecrush upper",
		.minimum       = 0,
		.maximum       = 255,
		.step          = 1,
		.default_value = 0xCF,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_PRIVATE_WHITECRUSH_LOWER,
		.name          = "whitecrush lower",
		.minimum       = 0,
		.maximum       = 255,
		.step          = 1,
		.default_value = 0x7F,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_PT_OSF_LOG) &&
			    info->loglevel == XT_OSF_LOGLEVEL_FIRST)
				break;
		}
	}
	rcu_read_unlock();

	if (!fcount && (info->flags & XT_OSF_LOG))
		nf_log_packet(p->family, p->hooknum, skb, p->in, p->out, NULL,
			"Remote OS is not known: %pi4:%u -> %pi4:%u\n",
				&ip->saddr, ntohs(tcp->source),
				&ip->daddr, ntohs(tcp->dest));

	if (fcount)
		fmatch = FMATCH_OK;

	return fmatch == FMATCH_OK;
}

static struct xt_match xt_osf_match = {
	.name 		= "osf",
	.revision	= 0,
	.family		= NFPROTO_IPV4,
	.proto		= IPPROTO_TCP,
	.hooks      	= (1 << NF_INET_LOCAL_IN) |
				(1 << NF_INET_PRE_ROUTING) |
				(1 << NF_INET_FORWARD),
	.match 		= xt_osf_match_packet,
	.matchsize	= sizeof(struct xt_osf_info),
	.me		= THIS_MODULE,
};

static int __init xt_osf_init(void)
{
	int err = -EINVAL;
	int i;

	for (i=0; i<ARRAY_SIZE(xt_osf_fingers); ++i)
		INIT_LIST_HEAD(&xt_osf_fingers[i]);

	err = nfnetlink_subsys_register(&xt_osf_nfnetlink);
	if (err < 0) {
		printk(KERN_ERR "Failed (%d) to register OSF nsfnetlink helper.\n", err);
		goto err_out_exit;
	}

	err = xt_register_match(&xt_osf_match);
	if (err) {
		printk(KERN_ERR "Failed (%d) to register OS fingerprint "
				"matching module.\n", err);
		goto err_out_remove;
	}

	return 0;

err_out_remove:
	nfnetlink_subsys_unregister(&xt_osf_nfnetlink);
err_out_exit:
	return err;
}

static void __exit xt_osf_fini(void)
{
	struct xt_osf_finger *f;
	int i;

	nfnetlink_subsys_unregister(&xt_osf_nfnetlink);
	xt_unregister_match(&xt_osf_match);

	rcu_read_lock();
	for (i=0; i<ARRAY_SIZE(xt_osf_fingers); ++i) {

		list_for_each_entry_rcu(f, &xt_osf_fingers[i], finger_entry) {
			list_del_rcu(&f->finger_entry);
			call_rcu(&f->rcu_head, xt_osf_finger_free_rcu);
		}
	}
	rcu_read_unlock();

	rcu_barrier();
}

module_init(xt_osf_init);
module_exit(xt_osf_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evgeniy Polyakov <zbr@ioremap.net>");
MODULE_DESCRIPTION("Passive OS fingerprint matching.");
MODULE_ALIAS_NFNL_SUBSYS(NFNL_SUBSYS_OSF);
