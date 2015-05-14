r + t + 8 +
					  	cmds.prestringlen / 2);

			iounmap(cmd_ptr0_virtaddr);
			iounmap(cmd_str_virtaddr);
		}
	}

	return 0;
}

static u32 dum_ch_setup(int ch_no, struct dum_ch_setup * ch_setup)
{
	struct cmdstring cmds_c;
	struct cmdstring *cmds = &cmds_c;
	struct disp_window dw;
	int standard;
	u32 orientation = 0;
	struct dumchannel chan = { 0 };
	int ret;

	if ((ch_setup->xmirror) || (ch_setup->ymirror) || (ch_setup->rotate)) {
		standard = 0;

		orientation = BIT(1);	/* always set 9-bit-bus */
		if (ch_setup->xmirror)
			orientation |= BIT(4);
		if (ch_setup->ymirror)
			orientation |= BIT(3);
		if (ch_setup->rotate)
			orientation |= BIT(0);
	} else
		standard = 1;

	cmds->channelnr = ch_no;

	/* build command string header */
	if (standard) {
		cmds->prestringlen = 32;
		cmds->poststringlen = 0;
	} else {
		cmds->prestringlen = 48;
		cmds->poststringlen = 16;
	}

	cmds->format =
	    (u16) ((ch_setup->disp_no << 4) | (BIT(3)) | (ch_setup->format));
	cmds->reserved = 0x0;
	cmds->startaddr_low = (ch_setup->minadr & 0xFFFF);
	cmds->startaddr_high = (ch_setup->minadr >> 16);

	if ((ch_setup->minadr == 0) && (ch_setup->maxadr == 0)
	    && (ch_setup->xmin == 0)
	    && (ch_setup->ymin == 0) && (ch_setup->xmax == 0)
	    && (ch_setup->ymax == 0)) {
		cmds->pixdatlen_low = 0;
		cmds->pixdatlen_high = 0;
	} else {
		u32 nbytes = nof_bytes(ch_setup);
		cmds->pixdatlen_low = (nbytes & 0xFFFF);
		cmds->pixdatlen_high = (nbytes >> 16);
	}

	if (ch_setup->slave_trans)
		cmds->pixdatlen_high |= BIT(15);

	/* build pre-string */
	build_disp_window(ch_setup, &dw);

	if (standard) {
		cmds->precmd[0] =
		    build_command(ch_setup->disp_no, DISP_XMIN_L_REG, 0x99);
		cmds->precmd[1] =
		    build_command(ch_setup->disp_no, DISP_XMIN_L_REG,
				  dw.xmin_l);
		cmds->precmd[2] =
		    build_command(ch_setup->disp_no, DISP_XMIN_H_REG,
				  dw.xmin_h);
		cmds->precmd[3] =
		    build_command(ch_setup->disp_no, DISP_YMIN_REG, dw.ymin);
		cmds->precmd[4] =
		    build_command(ch_setup->disp_no, DISP_XMAX_L_REG,
				  dw.xmax_l);
		cmds->precmd[5] =
		    build_command(ch_setup->disp_no, DISP_XMAX_H_REG,
				  dw.xmax_h);
		cmds->precmd[6] =
		    build_command(ch_setup->disp_no, DISP_YMAX_REG, dw.ymax);
		cmds->precmd[7] =
		    build_double_index(ch_setup->disp_no, DISP_PIXEL_REG);
	} else {
		if (dw.xmin_l == ch_no)
			cmds->precmd[0] =
			    build_command(ch_setup->disp_no, DISP_XMIN_L_REG,
					  0x99);
		else
			cmds->precmd[0] =
			    build_command(ch_setup->disp_no, DISP_XMIN_L_REG,
					  ch_no);

		cmds->precmd[1] =
		    build_command(ch_setup->disp_no, DISP_XMIN_L_REG,
				  dw.xmin_l);
		cmds->precmd[2] =
		    build_command(ch_setup->disp_no, DISP_XMIN_H_REG,
				  dw.xmin_h);
		cmds->precmd[3] =
		    build_command(ch_setup->disp_no, DISP_YMIN_REG, dw.ymin);
		cmds->precmd[4] =
		    build_command(ch_setup->disp_no, DISP_XMAX_L_REG,
				  dw.xmax_l);
		cmds->precmd[5] =
		    build_command(ch_setup->disp_no, DISP_XMAX_H_REG,
				  dw.xmax_h);
		cmds->precmd[6] =
		    build_command(ch_setup->disp_no, DISP_YMAX_REG, dw.ymax);
		cmds->precmd[7] =
		    build_command(ch_setup->disp_no, DISP_1_REG, orientation);
		cmds->precmd[8] =
		    build_double_index(ch_setup->disp_no, DISP_PIXEL_REG);
		cmds->precmd[9] =
		    build_double_index(ch_setup->disp_no, DISP_PIXEL_REG);
		cmds->precmd[0xA] =
		    build_double_index(ch_setup->disp_no, DISP_PIXEL_REG);
		cmds->precmd[0xB] =
		    build_double_index(ch_setup->disp_no, DISP_PIXEL_REG);
		cmds->postcmd[0] =
		    build_command(ch_setup->disp_no, DISP_1_REG, BIT(1));
		cmds->postcmd[1] =
		    build_command(ch_setup->disp_no, DISP_DUMMY1_REG, 1);
		cmds->postcmd[2] =
		    build_command(ch_setup->disp_no, DISP_DUMMY1_REG, 2);
		cmds->postcmd[3] =
		    build_command(ch_setup->disp_no, DISP_DUMMY1_REG, 3);
	}

	if ((ret = put_cmd_string(cmds_c)) != 0) {
		return ret;
	}

	chan.channelnr = cmds->channelnr;
	chan.dum_ch_min = ch_setup->dirtybuffer + ch_setup->minadr;
	chan.dum_ch_max = ch_setup->dirtybuffer + ch_setup->maxadr;
	chan.dum_ch_conf = 0x002;
	chan.dum_ch_ctrl = 0x04;

	put_channel(chan);

	return 0;
}

static u32 display_open(int ch_no, int auto_update, u32 * dirty_buffer,
			u32 * frame_buffer, u32 xpos, u32 ypos, u32 w, u32 h)
{

	struct dum_ch_setup k;
	int ret;

	/* keep width & height within display area */
	if ((xpos + w) > DISP_MAX_X_SIZE)
		w = DISP_MAX_X_SIZE - xpos;

	if ((ypos + h) > DISP_MAX_Y_SIZE)
		h = DISP_MAX_Y_SIZE - ypos;

	/* assume 1 display only */
	k.disp_no = 0;
	k.xmin = xpos;
	k.ymin = ypos;
	k.xmax = xpos + (w - 1);
	k.ymax = ypos + (h - 1);

	/* adjust min and max values if necessary */
	if (k.xmin > DISP_MAX_X_SIZE - 1)
		k.xmin = DISP_MAX_X_SIZE - 1;
	if (k.ymin > DISP_MAX_Y_SIZE - 1)
		k.ymin = DISP_MAX_Y_SIZE - 1;

	if (k.xmax > DISP_MAX_X_SIZE - 1)
		k.xmax = DISP_MAX_X_SIZE - 1;
	if (k.ymax > DISP_MAX_Y_SIZE - 1)
		k.ymax = DISP_MAX_Y_SIZE - 1;

	k.xmirror = 0;
	k.ymirror = 0;
	k.rotate = 0;
	k.minadr = (u32) frame_buffer;
	k.maxadr = (u32) frame_buffer + (((w - 1) << 10) | ((h << 2) - 2));
	k.pad = PAD_1024;
	k.dirtybuffer = (u32) dirty_buffer;
	k.format = RGB888;
	k.hwdirty = 0;
	k.slave_trans = 0;

	ret = dum_ch_setup(ch_no, &k);

	return ret;
}

static void lcd_reset(void)
{
	u32 *dum_pio_base = (u32 *)IO_ADDRESS(PNX4008_PIO_BASE);

	udelay(1);
	iowrite32(BIT(19), &dum_pio_base[2]);
	udelay(1);
	iowrite32(BIT(19), &dum_pio_base[1]);
	udelay(1);
}

static int dum_init(struct platform_device *pdev)
{
	struct clk *clk;

	/* enable DUM clock */
	clk = clk_get(&pdev->dev, "dum_ck");
	if (IS_ERR(clk)) {
		printk(KERN_ERR "pnx4008_dum: Unable to access DUM clock\n");
		return PTR_ERR(clk);
	}

	clk_set_rate(clk, 1);
	clk_put(clk);

	DUM_CTRL = V_DUM_RESET;

	/* set priority to "round-robin". All other params to "false" */
	DUM_CONF = BIT(9);

	/* Display 1 */
	DUM_WTCFG1 = PNX4008_DUM_WT_CFG;
	DUM_RTCFG1 = PNX4008_DUM_RT_CFG;
	DUM_TCFG = PNX4008_DUM_T_CFG;

	return 0;
}

static void dum_chan_init(void)
{
	int i = 0, ch = 0;
	u32 *cmdptrs;
	u32 *cmdstrings;

	DUM_COM_BASE =
		CMDSTRING_BASEADDR + BYTES_PER_CMDSTRING * NR_OF_CMDSTRINGS;

	if ((cmdptrs =
	     (u32 *) ioremap_nocache(DUM_COM_BASE,
				     sizeof(u32) * NR_OF_CMDSTRINGS)) == NULL)
		return;

	for (ch = 0; ch < NR_OF_CMDSTRINGS; ch++)
		iowrite32(CMDSTRING_BASEADDR + BYTES_PER_CMDSTRING * ch,
			  cmdptrs + ch);

	for (ch = 0; ch < MAX_DUM_CHANNELS; ch++)
		clear_channel(ch);

	/* Clear the cmdstrings */
	cmdstrings =
	    (u32 *)ioremap_nocache(*cmdptrs,
				   BYTES_PER_CMDSTRING * NR_OF_CMDSTRINGS);

	if (!cmdstrings)
		goto out;

	for (i = 0; i < NR_OF_CMDSTRINGS * BYTES_PER_CMDSTRING / sizeof(u32);
	     i++)
		iowrite32(0, cmdstrings + i);

	iounmap((u32 *)cmdstrings);

out:
	iounmap((u32 *)cmdptrs);
}

static void lcd_init(void)
{
	lcd_reset();

	DUM_OUTP_FORMAT1 = 0; /* RGB666 */

	udelay(1);
	iowrite32(V_LCD_STANDBY_OFF, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_USE_9BIT_BUS, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_SYNC_RISE_L, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_SYNC_RISE_H, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_SYNC_FALL_L, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_SYNC_FALL_H, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_SYNC_ENABLE, dum_data.slave_virt_base);
	udelay(1);
	iowrite32(V_LCD_DISPLAY_ON, dum_data.slave_virt_base);
	udelay(1);
}

/* Interface exported to framebuffer drivers */

int pnx4008_get_fb_addresses(int fb_type, void **virt_addr,
			     dma_addr_t *phys_addr, int *fb_length)
{
	int i;
	int ret = -1;
	for (i = 0; i < ARRAY_SIZE(fb_addr); i++)
		if (fb_addr[i].fb_type == fb_type) {
			*virt_addr = (void *)(dum_data.lcd_virt_start +
					fb_addr[i].addr_offset);
			*phys_addr =
			    dum_data.lcd_phys_start + fb_addr[i].addr_offset;
			*fb_length = fb_addr[i].fb_length;
			ret = 0;
			break;
		}

	return ret;
}

EXPORT_SYMBOL(pnx4008_get_fb_addresses);

int pnx4008_alloc_dum_channel(int dev_id)
{
	int i = 0;

	while ((i < MAX_DUM_CHANNELS) && (dum_data.fb_owning_channel[i] != -1))
		i++;

	if (i == MAX_DUM_CHANNELS)
		return -ENORESOURCESLEFT;
	else {
		dum_data.fb_owning_channel[i] = dev_id;
		return i;
	}
}

EXPORT_SYMBOL(pnx4008_alloc_dum_channel);

int pnx4008_free_dum_channel(int channr, int dev_id)
{
	if (channr < 0 || channr > MAX_DUM_CHANNELS)
		return -EINVAL;
	else if (dum_data.fb_owning_channel[channr] != dev_id)
		return -EFBNOTOWNER;
	else {
		clear_channel(channr);
		dum_data.fb_owning_channel[channr] = -1;
	}

	return 0;
}

EXPORT_SYMBOL(pnx4008_free_dum_channel);

int pnx4008_put_dum_channel_uf(struct dumchannel_uf chan_uf, int dev_id)
{
	int i = chan_uf.channelnr;
	int ret;

	if (i < 0 || i > MAX_DUM_CHANNELS)
		return -EINVAL;
	else if (dum_data.fb_owning_channel[i] != dev_id)
		return -EFBNOTOWNER;
	else if ((ret =
		  display_open(chan_uf.channelnr, 0, chan_uf.dirty,
			       chan_uf.source, chan_uf.y_offset,
			       chan_uf.x_offset, chan_uf.height,
			       chan_uf.width)) != 0)
		return ret;
	else {
		dum_data.chan_uf_store[i].dirty = chan_uf.dirty;
		dum_data.chan_uf_store[i].source = chan_uf.source;
		dum_data.chan_uf_store[i].x_offset = chan_uf.x_offset;
		dum_data.chan_uf_store[i].y_offset = chan_uf.y_offset;
		dum_data.chan_uf_store[i].width = chan_uf.width;
		dum_data.chan_uf_store[i].height = chan_uf.height;
	}

	return 0;
}

EXPORT_SYMBOL(pnx4008_put_dum_channel_uf);

int pnx4008_set_dum_channel_sync(int channr, int val, int dev_id)
{
	if (channr < 0 || channr > MAX_DUM_CHANNELS)
		return -EINVAL;
	else if (dum_data.fb_owning_channel[channr] != dev_id)
		return -EFBNOTOWNER;
	else {
		if (val == CONF_SYNC_ON) {
			DUM_CH_CONF(channr) |= CONF_SYNCENABLE;
			DUM_CH_CONF(channr) |= DUM_CHANNEL_CFG_SYNC_MASK |
				DUM_CHANNEL_CFG_SYNC_MASK_SET;
		} else if (val == CONF_SYNC_OFF)
			DUM_CH_CONF(channr) &= ~CONF_SYNCENABLE;
		else
			return -EINVAL;
	}

	return 0;
}

EXPORT_SYMBOL(pnx4008_set_dum_channel_sync);

int pnx4008_set_dum_channel_dirty_detect(int channr, int val, int dev_id)
{
	if (channr < 0 || channr > MAX_DUM_CHANNELS)
		return -EINVAL;
	else if (dum_data.fb_owning_channel[channr] != dev_id)
		return -EFBNOTOWNER;
	else {
		if (val == CONF_DIRTYDETECTION_ON)
			DUM_CH_CONF(channr) |= CONF_DIRTYENABLE;
		else if (val == CONF_DIRTYDETECTION_OFF)
			DUM_CH_CONF(channr) &= ~CONF_DIRTYENABLE;
		else
			return -EINVAL;
	}

	return 0;
}

EXPORT_SYMBOL(pnx4008_set_dum_channel_dirty_detect);

#if 0 /* Functions not used currently, but likely to be used in future */

static int get_channel(struct dumchannel *p_chan)
{
	int i = p_chan->channelnr;

	if (i < 0 || i > MAX_DUM_CHANNELS)
		return -EINVAL;
	else {
		p_chan->dum_ch_min = DUM_CH_MIN(i);
		p_chan->dum_ch_max = DUM_CH_MAX(i);
		p_chan->dum_ch_conf = DUM_CH_CONF(i);
		p_chan->dum_ch_stat = DUM_CH_STAT(i);
		p_chan->dum_ch_ctrl = 0;	/* WriteOnly control register */
	}

	return 0;
}

int pnx4008_get_dum_channel_uf(struct dumchannel_uf *p_chan_uf, int dev_id)
{
	int i = p_chan_uf->channelnr;

	if (i < 0 || i > MAX_DUM_CHANNELS)
		return -EINVAL;
	else if (dum_data.fb_owning_channel[i] != dev_id)
		return -EFBNOTOWNER;
	else {
		p_chan_uf->dirty = dum_data.chan_uf_store[i].dirty;
		p_chan_uf->source = dum_data.chan_uf_store[i].source;
		p_chan_uf->x_offset = dum_data.chan_uf_store[i].x_offset;
		p_chan_uf->y_offset = dum_data.chan_uf_store[i].y_offset;
		p_chan_uf->width = dum_data.chan_uf_store[i].width;
		p_chan_uf->height = dum_data.chan_uf_store[i].height;
	}

	return 0;
}

EXPORT_SYMBOL(pnx4008_get_dum_channel_uf);

int pnx4008_get_dum_channel_config(int channr, int dev_id)
{
	int ret;
	struct dumchannel chan;

	if (channr < 0 || channr > MAX_DUM_CHANNELS)
		return -EINVAL;
	else if (dum_data.fb_owning_channel[channr] != dev_id)
		return -EFBNOTOWNER;
	else {
		chan.channelnr = channr;
		if ((ret = get_channel(&chan)) != 0)
			return ret;
	}

	return (chan.dum_ch_conf & DUM_CHANNEL_CFG_MASK);
}

EXPORT_SYMBOL(pnx4008_get_dum_channel_config);

int pnx4008_force_update_dum_channel(int channr, int dev_id)
{
	if (channr < 0 || channr > MAX_DUM_CHANNELS)
		return -EINVAL;

	else if (dum_data.fb_owning_channel[channr] != dev_id)
		return -EFBNOTOWNER;
	else
		DUM_CH_CTRL(channr) = CTRL_SETDIRTY;

	return 0;
}

EXPORT_SYMBOL(pnx4008_force_update_dum_channel);

#endif

int pnx4008_sdum_mmap(struct fb_info *info, struct vm_area_struct *vma,
		      struct device *dev)
{
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;

	if (off < info->fix.smem_len) {
		vma->vm_pgoff += 1;
		return dma_mmap_writecombine(dev, vma,
				(void *)dum_data.lcd_virt_start,
				dum_data.lcd_phys_start,
				FB_DMA_SIZE);
	}
	return -EINVAL;
}

EXPORT_SYMBOL(pnx4008_sdum_mmap);

int pnx4008_set_dum_exit_notification(