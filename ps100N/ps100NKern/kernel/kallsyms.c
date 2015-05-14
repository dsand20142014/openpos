;
	} else {
		vwrq->flags = IW_RETRY_LIMIT;
		vwrq->value = priv->short_retry;
		if (priv->long_retry != priv->short_retry)
			vwrq->flags |= IW_RETRY_SHORT;
	}

	return 0;
}

static int atmel_set_rts(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq,
			 char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	int rthr = vwrq->value;

	if (vwrq->disabled)
		rthr = 2347;
	if ((rthr < 0) || (rthr > 2347)) {
		return -EINVAL;
	}
	priv->rts_threshold = rthr;

	return -EINPROGRESS;		/* Call commit handler */
}

static int atmel_get_rts(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq,
			 char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);

	vwrq->value = priv->rts_threshold;
	vwrq->disabled = (vwrq->value >= 2347);
	vwrq->fixed = 1;

	return 0;
}

static int atmel_set_frag(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_param *vwrq,
			  char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	int fthr = vwrq->value;

	if (vwrq->disabled)
		fthr = 2346;
	if ((fthr < 256) || (fthr > 2346)) {
		return -EINVAL;
	}
	fthr &= ~0x1;	/* Get an even value - is it really needed ??? */
	priv->frag_threshold = fthr;

	return -EINPROGRESS;		/* Call commit handler */
}

static int atmel_get_frag(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_param *vwrq,
			  char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);

	vwrq->value = priv->frag_threshold;
	vwrq->disabled = (vwrq->value >= 2346);
	vwrq->fixed = 1;

	return 0;
}

static int atmel_set_freq(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_freq *fwrq,
			  char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	int rc = -EINPROGRESS;		/* Call commit handler */

	/* If setting by frequency, convert to a channel */
	if (fwrq->e == 1) {
		int f = fwrq->m / 100000;

		/* Hack to fall through... */
		fwrq->e = 0;
		fwrq->m = ieee80211_freq_to_dsss_chan(f);
	}
	/* Setting by channel number */
	if ((fwrq->m > 1000) || (fwrq->e > 0))
		rc = -EOPNOTSUPP;
	else {
		int channel = fwrq->m;
		if (atmel_validate_channel(priv, channel) == 0) {
			priv->channel = channel;
		} else {
			rc = -EINVAL;
		}
	}
	return rc;
}

static int atmel_get_freq(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_freq *fwrq,
			  char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);

	fwrq->m = priv->channel;
	fwrq->e = 0;
	return 0;
}

static int atmel_set_scan(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *dwrq,
			  char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	unsigned long flags;

	/* Note : you may have realised that, as this is a SET operation,
	 * this is privileged and therefore a normal user can't
	 * perform scanning.
	 * This is not an error, while the device perform scanning,
	 * traffic doesn't flow, so it's a perfect DoS...
	 * Jean II */

	if (priv->station_state == STATION_STATE_DOWN)
		return -EAGAIN;

	/* Timeout old surveys. */
	if (time_after(jiffies, priv->last_survey + 20 * HZ))
		priv->site_survey_state = SITE_SURVEY_IDLE;
	priv->last_survey = jiffies;

	/* Initiate a scan command */
	if (priv->site_survey_state == SITE_SURVEY_IN_PROGRESS)
		return -EBUSY;

	del_timer_sync(&priv->management_timer);
	spin_lock_irqsave(&priv->irqlock, flags);

	priv->site_survey_state = SITE_SURVEY_IN_PROGRESS;
	priv->fast_scan = 0;
	atmel_scan(priv, 0);
	spin_unlock_irqrestore(&priv->irqlock, flags);

	return 0;
}

static int atmel_get_scan(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *dwrq,
			  char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	int i;
	char *current_ev = extra;
	struct iw_event	iwe;

	if (priv->site_survey_state != SITE_SURVEY_COMPLETED)
		return -EAGAIN;

	for (i = 0; i < priv->BSS_list_entries; i++) {
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe.u.ap_addr.sa_data, priv->BSSinfo[i].BSSID, 6);
		current_ev = iwe_stream_add_event(info, current_ev,
						  extra + IW_SCAN_MAX_DATA,
						  &iwe, IW_EV_ADDR_LEN);

		iwe.u.data.length =  priv->BSSinfo[i].SSIDsize;
		if (iwe.u.data.length > 32)
			iwe.u.data.length = 32;
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.flags = 1;
		current_ev = iwe_stream_add_point(info, current_ev,
						  extra + IW_SCAN_MAX_DATA,
						  &iwe, priv->BSSinfo[i].SSID);

		iwe.cmd = SIOCGIWMODE;
		iwe.u.mode = priv->BSSinfo[i].BSStype;
		current_ev = iwe_stream_add_event(info, current_ev,
						  extra + IW_SCAN_MAX_DATA,
						  &iwe, IW_EV_UINT_LEN);

		iwe.cmd = SIOCGIWFREQ;
		iwe.u.freq.m = priv->BSSinfo[i].channel;
		iwe.u.freq.e = 0;
		current_ev = iwe_stream_add_event(info, current_ev,
						  extra + IW_SCAN_MAX_DATA,
						  &iwe, IW_EV_FREQ_LEN);

		/* Add quality statistics */
		iwe.cmd = IWEVQUAL;
		iwe.u.qual.level = priv->BSSinfo[i].RSSI;
		iwe.u.qual.qual  = iwe.u.qual.level;
		/* iwe.u.qual.noise  = SOMETHING */
		current_ev = iwe_stream_add_event(info, current_ev,
						  extra + IW_SCAN_MAX_DATA,
						  &iwe, IW_EV_QUAL_LEN);


		iwe.cmd = SIOCGIWENCODE;
		if (priv->BSSinfo[i].UsingWEP)
			iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;
		iwe.u.data.length = 0;
		current_ev = iwe_stream_add_point(info, current_ev,
						  extra + IW_SCAN_MAX_DATA,
						  &iwe, NULL);
	}

	/* Length of data */
	dwrq->length = (current_ev - extra);
	dwrq->flags = 0;

	return 0;
}

static int atmel_get_range(struct net_device *dev,
			   struct iw_request_info *info,
			   struct iw_point *dwrq,
			   char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	struct iw_range *range = (struct iw_range *) extra;
	int k, i, j;

	dwrq->length = sizeof(struct iw_range);
	memset(range, 0, sizeof(struct iw_range));
	range->min_nwid = 0x0000;
	range->max_nwid = 0x0000;
	range->num_channels = 0;
	for (j = 0; j < ARRAY_SIZE(channel_table); j++)
		if (priv->reg_domain == channel_table[j].reg_domain) {
			range->num_channels = channel_table[j].max - channel_table[j].min + 1;
			break;
		}
	if (range->num_channels != 0) {
		for (k = 0, i = channel_table[j].min; i <= channel_table[j].max; i++) {
			range->freq[k].i = i; /* List index */

			/* Values in MHz -> * 10^5 * 10 */
			range->freq[k].m = (ieee80211_dsss_chan_to_freq(i) *
					    100000);
			range->freq[k++].e = 1;
		}
		range->num_frequency = k;
	}

	range->max_qual.qual = 100;
	range->max_qual.level = 100;
	range->max_qual.noise = 0;
	range->max_qual.updated = IW_QUAL_NOISE_INVALID;

	range->avg_qual.qual = 50;
	range->avg_qual.level = 50;
	range->avg_qual.noise = 0;
	range->avg_qual.updated = IW_QUAL_NOISE_INVALID;

	range->sensitivity = 0;

	range->bitrate[0] =  1000000;
	range->bitrate[1] =  2000000;
	range->bitrate[2] =  5500000;
	range->bitrate[3] = 11000000;
	range->num_bitrates = 4;

	range->min_rts = 0;
	range->max_rts = 2347;
	range->min_frag = 256;
	range->max_frag = 2346;

	range->encoding_size[0] = 5;
	range->encoding_size[1] = 13;
	range->num_encoding_sizes = 2;
	range->max_encoding_tokens = 4;

	range->pmp_flags = IW_POWER_ON;
	range->pmt_flags = IW_POWER_ON;
	range->pm_capa = 0;

	range->we_version_source = WIRELESS_EXT;
	range->we_version_compiled = WIRELESS_EXT;
	range->retry_capa = IW_RETRY_LIMIT ;
	range->retry_flags = IW_RETRY_LIMIT;
	range->r_time_flags = 0;
	range->min_retry = 1;
	range->max_retry = 65535;

	return 0;
}

static int atmel_set_wap(struct net_device *dev,
			 struct iw_request_info *info,
			 struct sockaddr *awrq,
			 char *extra)
{
	struct atmel_private *priv = netdev_priv(dev);
	int i;
	static const u8 any[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	static const u8 off[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned long flags;

	if (awrq->sa_family != ARPHRD_ETHER)
		return -EINVAL;

	if (!memcmp(any, awrq->sa_data, 6) ||
	    !memcmp(off, awrq->sa_data, 6)) {
		del_timer_sync(&priv->management_timer);
		spin_lock_irqsave(&priv->irqlock, flags);
		atmel_scan(priv, 1);
		spin_unlock_irqrestore(&priv->irqlock, flags);
		return 0;
	}

	for (i = 0; i < priv->BSS_list_entries; i++) {
		if (memcmp(priv->BSSinfo[i].BSSID, awrq->sa_data, 6) == 0) {
			if (!priv->wep_is_on && priv->BSSinfo[i].UsingWEP) {
				return -EINVAL;
			} else if  (priv->wep_is_on && !priv->BSSinfo[i].UsingWEP) {
				return -EINVAL;
			} else {
				del_timer_sync(&priv->management_timer);
				spin_lock_irqsave(&priv->irqlock, flags);
				atmel_join_bss(priv, i);
				spin_unlock_irqrestore(&priv->irqlock, flags);
				return 0;
			}
		}
	}

	return -EINVAL;
}

static int atmel_config_commit(struct net_device *dev,
			       struct iw_request_info *info,	/* NULL */
			       void *zwrq,			/* NULL */
			       char *extra)			/* NULL */
{
	return atmel_open(dev);
}

static const iw_handler atmel_handler[] =
{
	(iw_handler) atmel_config_commit,	/* SIOCSIWCOMMIT */
	(iw_handler) atmel_get_name,		/* SIOCGIWNAME */
	(iw_handler) NULL,			/* SIOCSIWNWID */
	(iw_handler) NULL,			/* SIOCGIWNWID */
	(iw_handler) atmel_set_freq,		/* SIOCSIWFREQ */
	(iw_handler) atmel_get_freq,		/* SIOCGIWFREQ */
	(iw_handler) atmel_set_mode,		/* SIOCSIWMODE */
	(iw_handler) atmel_get_mode,		/* SIOCGIWMODE */
	(iw_handler) NULL,			/* SIOCSIWSENS */
	(iw_handler) NULL,			/* SIOCGIWSENS */
	(iw_handler) NULL,			/* SIOCSIWRANGE */
	(iw_handler) atmel_get_range,           /* SIOCGIWRANGE */
	(iw_handler) NULL,			/* SIOCSIWPRIV */
	(iw_handler) NULL,			/* SIOCGIWPRIV */
	(iw_handler) NULL,			/* SIOCSIWSTATS */
	(iw_handler) NULL,			/* SIOCGIWSTATS */
	(iw_handler) NULL,			/* SIOCSIWSPY */
	(iw_handler) NULL,			/* SIOCGIWSPY */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) atmel_set_wap,		/* SIOCSIWAP */
	(iw_handler) atmel_get_wap,		/* SIOCGIWAP */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* SIOCGIWAPLIST */
	(iw_handler) atmel_set_scan,		/* SIOCSIWSCAN */
	(iw_handler) atmel_get_scan,		/* SIOCGIWSCAN */
	(iw_handler) atmel_set_essid,		/* SIOCSIWESSID */
	(iw_handler) atmel_get_essid,		/* SIOCGIWESSID */
	(iw_handler) NULL,			/* SIOCSIWNICKN */
	(iw_handler) NULL,			/* SIOCGIWNICKN */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) atmel_set_rate,		/* SIOCSIWRATE */
	(iw_handler) atmel_get_rate,		/* SIOCGIWRATE */
	(iw_handler) atmel_set_rts,		/* SIOCSIWRTS */
	(iw_handler) atmel_get_rts,		/* SIOCGIWRTS */
	(iw_handler) atmel_set_frag,		/* SIOCSIWFRAG */
	(iw_handler) atmel_get_frag,		/* SIOCGIWFRAG */
	(iw_handler) NULL,			/* SIOCSIWTXPOW */
	(iw_handler) NULL,			/* SIOCGIWTXPOW */
	(iw_handler) atmel_set_retry,		/* SIOCSIWRETRY */
	(iw_handler) atmel_get_retry,		/* SIOCGIWRETRY */
	(iw_handler) atmel_set_encode,		/* SIOCSIWENCODE */
	(iw_handler) atmel_get_encode,		/* SIOCGIWENCODE */
	(iw_handler) atmel_set_power,		/* SIOCSIWPOWER */
	(iw_handler) atmel_get_power,		/* SIOCGIWPOWER */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* SIOCSIWGENIE */
	(iw_handler) NULL,			/* SIOCGIWGENIE */
	(iw_handler) atmel_set_auth,		/* SIOCSIWAUTH */
	(iw_handler) atmel_get_auth,		/* SIOCGIWAUTH */
	(iw_handler) atmel_set_encodeext,	/* SIOCSIWENCODEEXT */
	(iw_handler) atmel_get_encodeext,	/* SIOCGIWENCODEEXT */
	(iw_handler) NULL,			/* SIOCSIWPMKSA */
};

static const iw_handler atmel_private_handler[] =
{
	NULL,				/* SIOCIWFIRSTPRIV */
};

typedef struct atmel_priv_ioctl {
	char id[32];
	unsigned char __user *data;
	unsigned short len;
} atmel_priv_ioctl;

#define ATMELFWL	SIOCIWFIRSTPRIV
#define ATMELIDIFC	ATMELFWL + 1
#define ATMELRD		ATMELFWL + 2
#define ATMELMAGIC 0x51807
#define REGDOMAINSZ 20

static const struct iw_priv_args atmel_private_args[] = {
	{
		.cmd = ATMELFWL,
		.set_args = IW_PRIV_TYPE_BYTE
				| IW_PRIV_SIZE_FIXED
				| sizeof (atmel_priv_ioctl),
		.get_args = IW_PRIV_TYPE_NONE,
		.name = "atmelfwl"
	}, {
		.cmd = ATMELIDIFC,
		.set_args = IW_PRIV_TYPE_NONE,
		.get_args = IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		.name = "atmelidifc"
	}, {
		.cmd = ATMELRD,
		.set_args = IW_PRIV_TYPE_CHAR | REGDOMAINSZ,
		.get_args = IW_PRIV_TYPE_NONE,
		.name = "regdomain"
	},
};

static const struct iw_handler_def atmel_handler_def = {
	.num_standard	= ARRAY_SIZE(atmel_handler),
	.num_private	= ARRAY_SIZE(atmel_private_handler),
	.num_private_args = ARRAY_SIZE(atmel_private_args),
	.standard	= (iw_handler *) atmel_handler,
	.private	= (iw_handler *) atmel_private_handler,
	.private_args	= (struct iw_priv_args *) atmel_private_args,