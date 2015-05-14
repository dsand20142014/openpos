r->bits_per_pixel);
		return -EINVAL;
	}

	if (PICOS2KHZ(var->pixclock) > PM2_MAX_PIXCLOCK) {
		DPRINTK("pixclock too high (%ldKHz)\n",
			PICOS2KHZ(var->pixclock));
		return -EINVAL;
	}

	var->transp.offset = 0;
	var->transp.length = 0;
	switch (var->bits_per_pixel) {
	case 8:
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		break;
	case 16:
		var->red.offset   = 11;
		var->red.length   = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.offset  = 0;
		var->blue.length  = 5;
		break;
	case 32:
		var->transp.offset = 24;
		var->transp.length = 8;
		var->red.offset	  = 16;
		var->green.offset = 8;
		var->blue.offset  = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		break;
	case 24:
#ifdef __BIG_ENDIAN
		var->red.offset   = 0;
		var->blue.offset  = 16;
#else
		var->red.offset   = 16;
		var->blue.offset  = 0;
#endif
		var->green.offset = 8;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		break;
	}
	var->height = -1;
	var->width = -1;

	var->accel_flags = 0;	/* Can't mmap if this is on */

	DPRINTK("Checking graphics mode at %dx%d depth %d\n",
		var->xres, var->yres, var->bits_per_pixel);
	return 0;
}

/**
 *	pm2fb_set_par - Alters the hardware state.
 *	@info: frame buffer structure that represents a single frame buffer
 *
 *	Using the fb_var_screeninfo in fb_info we set the resolution of the
 *	this particular framebuffer.
 */
static int pm2fb_set_par(struct fb_info *info)
{
	struct pm2fb_par *par = info->par;
	u32 pixclock;
	u32 width = (info->var.xres_virtual + 7) & ~7;
	u32 height = info->var.yres_virtual;
	u32 depth = (info->var.bits_per_pixel + 7) & ~7;
	u32 hsstart, hsend, hbend, htotal;
	u32 vsstart, vsend, vbend, vtotal;
	u32 stride;
	u32 base;
	u32 video = 0;
	u32 clrmode = PM2F_RD_COLOR_MODE_RGB | PM2F_RD_GUI_ACTIVE;
	u32 txtmap = 0;
	u32 pixsize = 0;
	u32 clrformat = 0;
	u32 misc = 1; /* 8-bit DAC */
	u32 xres = (info->var.xres + 31) & ~31;
	int data64;

	reset_card(par);
	reset_config(par);
	clear_palette(par);
	if (par->memclock)
		set_memclock(par, par->memclock);

	depth = (depth > 32) ? 32 : depth;
	data64 = depth > 8 || par->type == PM2_TYPE_PERMEDIA2V;

	pixclock = PICOS2KHZ(info->var.pixclock);
	if (pixclock > PM2_MAX_PIXCLOCK) {
		DPRINTK("pixclock too high (%uKHz)\n", pixclock);
		return -EINVAL;
	}

	hsstart = to3264(info->var.right_margin, depth, data64);
	hsend = hsstart + to3264(info->var.hsync_len, depth, data64);
	hbend = hsend + to3264(info->var.left_margin, depth, data64);
	htotal = to3264(xres, depth, data64) + hbend - 1;
	vsstart = (info->var.lower_margin)
		? info->var.lower_margin - 1
		: 0;	/* FIXME! */
	vsend = info->var.lower_margin + info->var.vsync_len - 1;
	vbend = info->var.lower_margin + info->var.vsync_len +
		info->var.upper_margin;
	vtotal = info->var.yres + vbend - 1;
	stride = to3264(width, depth, 1);
	base = to3264(info->var.yoffset * xres + info->var.xoffset, depth, 1);
	if (data64)
		video |= PM2F_DATA_64_ENABLE;

	if (info->var.sync & FB_SYNC_HOR_HIGH_ACT) {
		if (lowhsync) {
			DPRINTK("ignoring +hsync, using -hsync.\n");
			video |= PM2F_HSYNC_ACT_LOW;
		} else
			video |= PM2F_HSYNC_ACT_HIGH;
	} else
		video |= PM2F_HSYNC_ACT_LOW;

	if (info->var.sync & FB_SYNC_VERT_HIGH_ACT) {
		if (lowvsync) {
			DPRINTK("ignoring +vsync, using -vsync.\n");
			video |= PM2F_VSYNC_ACT_LOW;
		} else
			video |= PM2F_VSYNC_ACT_HIGH;
	} else
		video |= PM2F_VSYNC_ACT_LOW;

	if ((info->var.vmode & FB_VMODE_MASK) == FB_VMODE_INTERLACED) {
		DPRINTK("interlaced not supported\n");
		return -EIN