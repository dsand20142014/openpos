;
	if (!entry)
		goto fail_ssid;
	entry->uid = proc_uid;
	entry->gid = proc_gid;

	/* Setup the APList */
	entry = proc_create_data("APList",
				 S_IFREG | proc_perm,
				 apriv->proc_entry, &proc_APList_ops, dev);
	if (!entry)
		goto fail_aplist;
	entry->uid = proc_uid;
	entry->gid = proc_gid;

	/* Setup the BSSList */
	entry = proc_create_data("BSSList",
				 S_IFREG | proc_perm,
				 apriv->proc_entry, &proc_BSSList_ops, dev);
	if (!entry)
		goto fail_bsslist;
	entry->uid = proc_uid;
	entry->gid = proc_gid;

	/* Setup the WepKey */
	entry = proc_create_data("WepKey",
				 S_IFREG | proc_perm,
				 apriv->proc_entry, &proc_wepkey_ops, dev);
	if (!entry)
		goto fail_wepkey;
	entry->uid = proc_uid;
	entry->gid = proc_gid;

	return 0;

fail_wepkey:
	remove_proc_entry("BSSList", apriv->proc_entry);
fail_bsslist:
	remove_proc_entry("APList", apriv->proc_entry);
fail_aplist:
	remove_proc_entry("SSID", apriv->proc_entry);
fail_ssid:
	remove_proc_entry("Config", apriv->proc_entry);
fail_config:
	remove_proc_entry("Status", apriv->proc_entry);
fail_status:
	remove_proc_entry("Stats", apriv->proc_entry);
fail_stats:
	remove_proc_entry("StatsDelta", apriv->proc_entry);
fail_stats_delta:
	remove_proc_entry(apriv->proc_name, airo_entry);
fail:
	return -ENOMEM;
}

static int takedown_proc_entry( struct net_device *dev,
				struct airo_info *apriv ) {
	if ( !apriv->proc_entry->namelen ) return 0;
	remove_proc_entry("Stats",apriv->proc_entry);
	remove_proc_entry("StatsDelta",apriv->proc_entry);
	remove_proc_entry("Status",apriv->proc_entry);
	remove_proc_entry("Config",apriv->proc_entry);
	remove_proc_entry("SSID",apriv->proc_entry);
	remove_proc_entry("APList",apriv->proc_entry);
	remove_proc_entry("BSSList",apriv->proc_entry);
	remove_proc_entry("WepKey",apriv->proc_entry);
	remove_proc_entry(apriv->proc_name,airo_entry);
	return 0;
}

/*
 *  What we want from the proc_fs is to be able to efficiently read
 *  and write the configuration.  To do this, we want to read the
 *  configuration when the file is opened and write it when the file is
 *  closed.  So basically we allocate a read buffer at open and fill it
 *  with data, and allocate a write buffer and read it at close.
 */

/*
 *  The read routine is generic, it relies on the preallocated rbuffer
 *  to supply the data.
 */
static ssize_t proc_read( struct file *file,
			  char __user *buffer,
			  size_t len,
			  loff_t *offset )
{
	struct proc_data *priv = file->private_data;

	if (!priv->rbuffer)
		return -EINVAL;

	return simple_read_from_buffer(buffer, len, offset, priv->rbuffer,
					priv->readlen);
}

/*
 *  The write routine is generic, it fills in a preallocated rbuffer
 *  to supply the data.
 */
static ssize_t proc_write( struct file *file,
			   const char __user *buffer,
			   size_t len,
			   loff_t *offset )
{
	loff_t pos = *offset;
	struct proc_data *priv = (struct proc_data*)file->private_data;

	if (!priv->wbuffer)
		return -EINVAL;

	if (pos < 0)
		return -EINVAL;
	if (pos >= priv->maxwritelen)
		return 0;
	if (len > priv->maxwritelen - pos)
		len = priv->maxwritelen - pos;
	if (copy_from_user(priv->wbuffer + pos, buffer, len))
		return -EFAULT;
	if ( pos + len > priv->writelen )
		priv->writelen = len + file->f_pos;
	*offset = pos + len;
	return len;
}

static int proc_status_open(struct inode *inode, struct file *file)
{
	struct proc_data *data;
	struct proc_dir_entry *dp = PDE(inode);
	struct net_device *dev = dp->data;
	struct airo_info *apriv = dev->ml_priv;
	CapabilityRid cap_rid;
	StatusRid status_rid;
	u16 mode;
	int i;

	if ((file->private_data = kzalloc(sizeof(struct proc_data ), GFP_KERNEL)) == NULL)
		return -ENOMEM;
	data = (struct proc_data *)file->private_data;
	if ((data->rbuffer = kmalloc( 2048, GFP_KERNEL )) == NULL) {
		kfree (file->private_data);
		return -ENOMEM;
	}

	readStatusRid(apriv, &status_rid, 1);
	readCapabilityRid(apriv, &cap_rid, 1);

	mode = le16_to_cpu(status_rid.mode);

        i = sprintf(data->rbuffer, "Status: %s%s%s%s%s%s%s%s%s\n",
                    mode & 1 ? "CFG ": "",
                    mode & 2 ? "ACT ": "",
                    mode & 0x10 ? "SYN ": "",
                    mode & 0x20 ? "LNK ": "",
                    mode & 0x40 ? "LEAP ": "",
                    mode & 0x80 ? "PRIV ": "",
                    mode & 0x100 ? "KEY ": "",
                    mode & 0x200 ? "WEP ": "",
                    mode & 0x8000 ? "ERR ": "");
	sprintf( data->rbuffer+i, "Mode: %x\n"
		 "Signal Strength: %d\n"
		 "Signal Quality: %d\n"
		 "SSID: %-.*s\n"
		 "AP: %-.16s\n"
		 "Freq: %d\n"
		 "BitRate: %dmbs\n"
		 "Driver Version: %s\n"
		 "Device: %s\nManufacturer: %s\nFirmware Version: %s\n"
		 "Radio type: %x\nCountry: %x\nHardware Version: %x\n"
		 "Software Version: %x\nSoftware Subversion: %x\n"
		 "Boot block version: %x\n",
		 le16_to_cpu(status_rid.mode),
		 le16_to_cpu(status_rid.normalizedSignalStrength),
		 le16_to_cpu(status_rid.signalQuality),
		 le16_to_cpu(status_rid.SSIDlen),
		 status_rid.SSID,
		 status_rid.apName,
		 le16_to_cpu(status_rid.channel),
		 le16_to_cpu(status_rid.currentXmitRate) / 2,
		 version,
		 cap_rid.prodName,
		 cap_rid.manName,
		 cap_rid.prodVer,
		 le16_to_cpu(cap_rid.radioType),
		 le16_to_cpu(cap_rid.country),
		 le16_to_cpu(cap_rid.hardVer),
		 le16_to_cpu(cap_rid.softVer),
		 le16_to_cpu(cap_rid.softSubVer),
		 le16_to_cpu(cap_rid.bootBlockVer));
	data->readlen = strlen( data->rbuffer );
	return 0;
}

static int proc_stats_rid_open(struct inode*, struct file*, u16);
static int proc_statsdelta_open( struct inode *inode,
				 struct file *file ) {
	if (file->f_mode&FMODE_WRITE) {
		return proc_stats_rid_open(inode, file, RID_STATSDELTACLEAR);
	}
	return proc_stats_rid_open(inode, file, RID_STATSDELTA);
}

static int proc_stats_open( struct inode *inode, struct file *file ) {
	return proc_stats_rid_open(inode, file, RID_STATS);
}

static int proc_stats_rid_open( struct inode *inode,
				struct file *file,
				u16 rid )
{
	struct proc_data *data;
	struct proc_dir_entry *dp = PDE(inode);
	struct net_device *dev = dp->data;
	struct airo_info *apriv = dev->ml_priv;
	StatsRid stats;
	int i, j;
	__le32 *vals = stats.vals;
	int len;

	if ((file->private_data = kzalloc(sizeof(struct proc_data ), GFP_KERNEL)) == NULL)
		return -ENOMEM;
	data = (struct proc_data *)file->private_data;
	if ((data->rbuffer = kmalloc( 4096, GFP_KERNEL )) == NULL) {
		kfree (file->private_data);
		return -ENOMEM;
	}

	readStatsRid(apriv, &stats, rid, 1);
	len = le16_to_cpu(stats.len);

        j = 0;
	for(i=0; statsLabels[i]!=(char *)-1 && i*4<len; i++) {
		if (!statsLabels[i]) continue;
		if (j+strlen(statsLabels[i])+16>4096) {
			airo_print_warn(apriv->dev->name,
			       "Potentially disasterous buffer overflow averted!");
			break;
		}
		j+=sprintf(data->rbuffer+j, "%s: %u\n", statsLabels[i],
				le32_to_cpu(vals[i]));
	}
	if (i*4 >= len) {
		airo_print_warn(apriv->dev->name, "Got a short rid");
	}
	data->readlen = j;
	return 0;
}

static int get_dec_u16( char *buffer, int *start, int limit ) {
	u16 value;
	int valid = 0;
	for( value = 0; buffer[*start] >= '0' &&
		     buffer[*start] <= '9' &&
		     *start < limit; (*start)++ ) {
		valid = 1;
		value *= 10;
		value += buffer[*start] - '0';
	}
	if ( !valid ) return -1;
	return value;
}

static int airo_config_commit(struct net_device *dev,
			      struct iw_request_info *info, void *zwrq,
			      char *extra);

static inline int sniffing_mode(struct airo_info *ai)
{
	return le16_to_cpu(ai->config.rmode & RXMODE_MASK) >=
		le16_to_cpu(RXMODE_RFMON);
}

static void proc_config_on_close(struct inode *inode, struct file *file)
{
	struct proc_data *data = file->private_data;
	struct proc_dir_entry *dp = PDE(inode);
	struct net_device *dev = dp->data;
	struct airo_info *ai = dev->ml_priv;
	char *line;

	if ( !data->writelen ) return;

	readConfigRid(ai, 1);
	set_bit (FLAG_COMMIT, &ai->flags);

	line = data->wbuffer;
	while( line[0] ) {
/*** Mode processing */
		if ( !strncmp( line, "Mode: ", 6 ) ) {
			line += 6;
			if (sniffing_mode(ai))
				set_bit (FLAG_RESET, &ai->flags);
			ai->config.rmode &= ~RXMODE_FULL_MASK;
			clear_bit (FLAG_802_11, &ai->flags);
			ai->config.opmode &= ~MODE_CFG_MASK;
			ai->config.scanMode = SCANMODE_ACTIVE;
			if ( line[0] == 'a' ) {
				ai->config.opmode |= MODE_STA_IBSS;
			} else {
				ai->config.opmode |= MODE_STA_ESS;
				if ( line[0] == 'r' ) {
					ai->config.rmode |= RXMODE_RFMON | RXMODE_DISABLE_802_3_HEADER;
					ai->config.scanMode = SCANMODE_PASSIVE;
					set_bit (FLAG_802_11, &ai->flags);
				} else if ( line[0] == 'y' ) {
					ai->config.r