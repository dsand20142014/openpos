X_HIGH, 0);
	}

	set_aperture(par, depth);

	mb();
	WAIT_FIFO(par, 19);
	switch (depth) {
	case 8:
		pm2_WR(par, PM2R_FB_READ_PIXEL, 0);
		clrformat = 0x2e;
		break;
	case 16:
		pm2_WR(par, PM2R_FB_READ_PIXEL, 1);
		clrmode |= PM2F_RD_TRUECOLOR | PM2F_RD_PIXELFORMAT_RGB565;
		txtmap = PM2F_TEXTEL_SIZE_16;
		pixsize = 1;
		clrformat = 0x70;
		misc |= 8;
		break;
	case 32:
		pm2_WR(par, PM2R_FB_READ_PIXEL, 2);
		clrmode |= PM2F_RD_TRUECOLOR | PM2F_RD_PIXELFORMAT_RGBA8888;
		txtmap = PM2F_TEXTEL_SIZE_32;
		pixsize = 2;
		clrformat = 0x20;
		misc |= 8;
		break;
	case 24:
		pm2_WR(par, PM2R_FB_READ_PIXEL, 4);
		clrmode |= PM2F_RD_TRUECOLOR | PM2F_RD_PIXELFORMAT_RGB888;
		txtmap = PM2F_TEXTEL_SIZE_24;
		pixsize = 4;
		clrformat = 0x20;
		misc |= 8;
		break;
	}
	pm2_WR(par, PM2R_FB_WRITE_MODE, PM2F_FB_WRITE_ENABLE);
	pm2_WR(par, PM2R_FB_READ_MODE, partprod(xres));
	pm2_WR(par, PM2R_LB_READ_MODE, partprod(xres));
	pm2_WR(par, PM2R_TEXTURE_MAP_FORMAT, txtmap | partprod(xres));
	pm2_WR(par, PM2R_H_TOTAL, htotal);
	pm2_WR(par, PM2R_HS_START, hsstart);
	pm2_WR(par, PM2R_HS_END, hsend);
	pm2_WR(par, PM2R_HG_END, hbend);
	pm2_WR(par, PM2R_HB_END, hbend);
	pm2_WR(par, PM2R_V_TOTAL, vtotal);
	pm2_WR(par, PM2R_VS_START, vsstart);
	pm2_WR(par, PM2R_VS_END, vsend);
	pm2_WR(par, PM2R_VB_END, vbend);
	pm2_WR(par, PM2R_SCREEN_STRIDE, stride);
	wmb();
	pm2_WR(par, PM2R_WINDOW_ORIGIN, 0);
	pm2_WR(par, PM2R_SCREEN_SIZE, (height << 16) | width);
	pm2_WR(par, PM2R_SCISSOR_MODE, PM2F_SCREEN_SCISSOR_ENABLE);
	wmb();
	pm2_WR(par, PM2R_SCREEN_BASE, base);
	wmb();
	set_video(par, video);
	WAIT_FIFO(par, 10);
	switch (par->type) {
	case PM2_TYPE_PERMEDIA2:
		pm2_RDAC_WR(par, PM2I_RD_COLOR_MODE, clrmode);
		pm2_RDAC_WR(par, PM2I_RD_COLOR_KEY_CONTROL,
				(depth == 8) ? 0 : PM2F_COLOR_KEY_TEST_OFF);
		break;
	case PM2_TYPE_PERMEDIA2V:
		pm2v_RDAC_WR(par, PM2VI_RD_DAC_CONTROL, 0);
		pm2v_RDAC_WR(par, PM2VI_RD_PIXEL_SIZE, pixsize);
		pm2v_RDAC_WR(par, PM2VI_RD_COLOR_FORMAT, clrformat);
		pm2v_RDAC_WR(par, PM2VI_RD_MISC_CONTROL, misc);
		pm2v_RDAC_WR(par, PM2VI_RD_OVERLAY_KEY, 0);
		break;
	}
	set_pixclock(par, pixclock);
	DPRINTK("Setting graphics mode at %dx%d depth %d\n",
		info->var.xres, info->var.yres, info->var.bits_per_pixel);
	return 0;
}

/**
 *	pm2fb_setcolreg - Sets a color register.
 *	@regno: boolean, 0 copy local, 1 get_user() function
 *	@red: frame buffer colormap structure
 *	@green: The green value which can be up to 16 bits wide
 *	@blue:  The blue value which can be up to 16 bits wide.
 *	@transp: If supported the alpha value which can be up to 16 bits wide.
 *	@info: frame buffer info structure
 *
 *	Set a single color register. The values supplied have a 16 bit
 *	magnitude which needs to be scaled in this function for the hardware.
 *	Pretty much a direct lift from tdfxfb.c.
 *
 *	Returns negative errno on error, or zero on success.
 */
static int pm2fb_setcolreg(unsigned regno, unsigned red, unsigned green,
			   unsigned blue, unsigned transp,
			   struct fb_info *info)
{
	struct pm2fb_par *par = info->par;

	if (regno >= info->cmap.len)  /* no. of hw registers */
		return -EINVAL;
	/*
	 * Program hardware... do anything you want with transp
	 */

	/* grayscale works only partially under directcolor */
	/* grayscale = 0.30*R + 0.59*G + 0.11*B */
	if (info->var.grayscale)
		red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;

	/* Directcolor:
	 *   var->{color}.offset contains start of bitfield
	 *   var->{color}.length contains length of bitfield
	 *   {hardwarespecific} contains width of DAC
	 *   cmap[X] is programmed to
	 *   (X << red.offset) | (X << green.offset) | (X << blue.offset)
	 *   RAMDAC[X] is programmed to (red, green, blue)
	 *
	 * Pseudocolor:
	 *    uses offset = 0 && length = DAC register width.
	 *    var->{color}.offset is 0
	 *    var->{color}.length contains widht of DAC
	 *    cmap is not used
	 *    DAC[X] is programmed to (red, green, blue)
	 * Truecolor:
	 *    does not use RAMDAC (usually has 3 of them).
	 *    var->{color}.offset contains start of bitfield
	 *    var->{color}.length contains length of bitfield
	 *    cmap is programmed to
	 *    (red << red.offset) | (green << green.offset) |
	 *    (blue << blue.offset) | (transp << transp.offset)
	 *    RAMDAC does not exist
	 */
#define CNVT_TOHW(val, width) ((((val) << (width)) + 0x7FFF -(val)) >> 16)
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		red = CNVT_TOHW(red, info->var.red.length);
		green = CNVT_TOHW(green, info->var.green.length);
		blue = CNVT_TOHW(blue, info->var.blue.length);
		transp = CNVT_TOHW(transp, info->var.transp.length);
		break;
	case FB_VISUAL_DIRECTCOLOR:
		/* example here assumes 8 bit DAC. Might be different
		 * for your hardware */
		red = CNVT_TOHW(red, 8);
		green = CNVT_TOHW(green, 8);
		blue = CNVT_TOHW(blue, 8);
		/* hey, there is bug in transp handling... */
		transp = CNVT_TOHW(transp, 8);
		break;
	}
#undef CNVT_TOHW
	/* Truecolor has hardware independent palette */
	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		u32 v;

		if (regno >= 16)
			return -EINVAL;

		v = (red << info->var.red.offset) |
			(green << info->var.green.offset) |
			(blue << info->var.blue.offset) |
			(transp << info->var.transp.offset);

		switch (info->var.bits_per_pixel) {
		case 8:
			break;
		case 16:
		case 24:
		case 32:
			par->palette[regno] = v;
			break;
		}
		return 0;
	} else if (info->fix.visual == FB_VISUAL_PSEUDOCOLOR)
		set_color(par, regno, red, green, blue);

	return 0;
}

/**
 *	pm2fb_pan_display - Pans the display.
 *	@var: frame buffer variable screen structure
 *	@info: frame buffer structure that represents a single frame buffer
 *
 *	Pan (or wrap, depending on the `vmode' field) the display using the
 *	`xoffset' and `yoffset' fields of the `var' structure.
 *	If the values don't fit, return -EINVAL.
 *
 *	Returns negative errno on error, or zero on success.
 *
 */
static int pm2fb_pan_display(struct fb_var_screeninfo *var,
			     struct fb_info *info)
{
	struct pm2fb_par *p = info->par;
	u32 base;
	u32 depth = (var->bits_per_pixel + 7) & ~7;
	u32 xres = (var->xres + 31) & ~31;

	depth = (depth > 32) ? 32 : depth;
	base = to3264(var->yoffset * xres + var->xoffset, depth, 1);
	WAIT_FIFO(p, 1);
	pm2_WR(p, PM2R_SCREEN_BASE, base);
	return 0;
}

/**
 *	pm2fb_blank - Blanks the display.
 *	@blank_mode: the blank mode we want.
 *	@info: frame buffer structure that represents a single frame buffer
 *
 *	Blank the screen if blank_mode != 0, else unblank. Return 0 if
 *	blanking succeeded, != 0 if un-/blanking failed due to e.g. a
 *	video mode which doesn't support it. Implements VESA suspend
 *	and powerdown modes on hardware that supports disabling hsync/vsync:
 *	blank_mode == 2: suspend vsync
 *	blank_mode == 3: suspend hsync
 *	blank_mode == 4: powerdown
 *
 *	Returns negative errno on error, or zero on success.
 *
 */
static int pm2fb_blank(int blank_mode, struct fb_info *info)
{
	struct pm2fb_par *par = info->par;
	u32 video = par->video;

	DPRINTK("blank_mode %d\n", blank_mode);

	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
		/* Screen: On */
		video |= PM2F_VIDEO_ENABLE;
		break;
	case FB_BLANK_NORMAL:
		/* Screen: Off */
		video &= ~PM2F_VIDEO_ENABLE;
		break;
	case FB_BLANK_VSYNC_SUSPEND:
		/* VSync: Off */
		video &= ~(PM2F_VSYNC_MASK | PM2F_BLANK_LOW);
		break;
	case FB_BLANK_HSYNC_SUSPEND:
		/* HSync: Off */
		video &= ~(PM2F_HSYNC_MASK | PM2F_BLANK_LOW);
		break;
	case FB_BLANK_POWERDOWN:
		/* HSync: Off, VSync: Off */
		video &= ~(PM2F_VSYNC_MASK | PM2F_HSYNC_MASK | PM2F_BLANK_LOW);
		break;
	}
	set_video(par, video);
	return 0;
}

static int pm2fb_sync(struct fb_info *info)
{
	struct pm2fb_par *par = info->par;

	WAIT_FIFO(par, 1);
	pm2_WR(par, PM2R_SYNC, 0);
	mb();
	do {
		while (pm2_RD(par, PM2R_OUT_FIFO_WORDS) == 0)
			cpu_relax();
	} while (pm2_RD(par, PM2R_OUT_FIFO) != PM2TAG(PM2R_SYNC));

	return 0;
}

static void pm2fb_fillrect(struct fb_info *info,
				const struct fb_fillrect *region)
{
	struct pm2fb_par *par = info->par;
	struct fb_fillrect modded;
	int vxres, vyres;
	u32 color = (info->fix.visual == FB_VISUAL_TRUECOLOR) ?
		((u32 *)info->pseudo_palette)[region->color] : region->color;

	if (info->state != FBINFO_STATE_RUNNING)
		return;
	if ((info->flags & FBINFO_HWACCEL_DISABLED) ||
		region->rop != ROP_COPY ) {
		cfb_fillrect(info, region);
		return;
	}

	vxres = info->var.xres_virtual;
	vyres = info->var.yres_virtual;

	memcpy(&modded, region, sizeof(struct fb_fillrect));

	if (!modded.width || !modded.height ||
	    modded.dx >= vxres || modded.dy >= vyres)
		return;

	if (modded.dx + modded.width  > vxres)
		modded.width  = vxres - modded.dx;
	if (modded.dy + modded.height > vyres)
		modded.height = vyres - modded.dy;

	if (info->var.bits_per_pixel == 8)
		color |= color << 8;
	if (info->var.bits_per_pixel <= 16)
		color |= color << 16;

	WAIT_FIFO(par, 3);
	pm2_WR(par, PM2R_CONFIG, PM2F_CONFIG_FB_WRITE_ENABLE);
	pm2_WR(par, PM2R_RECTANGLE_ORIGIN, (modded.dy << 16) | modded.dx);
	pm2_WR(par, PM2R_RECTANGLE_SIZE, (modded.height << 16) | modded.width);
	if (info->var.bits_per_pixel != 24) {
		WAIT_FIFO(par, 2);
		pm2_WR(par, PM2R_FB_BLOCK_COLOR, color);
		wmb();
		pm2_WR(par, PM2R_RENDER,
				PM2F_RENDER_RECTANGLE | PM2F_RENDER_FASTFILL);
	} else {
		WAIT_FIFO(par, 4);
		pm2_WR(par, PM2R_COLOR_DDA_MODE, 1);
		pm2_WR(par, PM2R_CONSTANT_COLOR, color);
		wmb();
		pm2_WR(par, PM2R_RENDER,
				PM2F_RENDER_RECTANGLE |
				PM2F_INCREASE_X | PM2F_INCREASE_Y );
		pm2_WR(par, PM2R_COLOR_DDA_MODE, 0);
	}
}

static void pm2fb_copyarea(struct fb_info *info,
				const struct fb_copyarea *area)
{
	struct pm2fb_par *par = info->par;
	struct fb_copyarea modded;
	u32 vxres, vyres;

	if (info->state != FBINFO_STATE_RUNNING)
		return;
	if (info->flags & FBINFO_HWACCEL_DISABLED) {
		cfb_copyarea(info, area);
		return;
	}

	memcpy(&modded, area, sizeof(struct fb_copyarea));

	vxres = info->var.xres_virtual;
	vyres = info->var.yres_virtual;

	if (!modded.width || !modded.height ||
	    modded.sx >= vxres || modded.sy >= vyres ||
	    modded.dx >= vxres || modded.dy >= vyres)
		return;

	if (modded.sx + modded.width > vxres)
		modded.width = vxres - modded.sx;
	if (modded.dx + modded.width > vxres)
		modded.width = vxres - modded.dx;
	if (modded.sy + modded.height > vyres)
		modded.height = vyres - modded.sy;
	if (modded.dy + modded.height > vyres)
		modded.height = vyres - modded.dy;

	WAIT_FIFO(par, 5);
	pm2_WR(par, PM2R_CONFIG, PM2F_CONFIG_FB_WRITE_ENABLE |
		PM2F_CONFIG_FB_READ_SOURCE_ENABLE);
	pm2_WR(par, PM2R_FB_SOURCE_DELTA,
			((modded.sy - modded.dy) & 0xfff) << 16 |
			((modded.sx - modded.dx) & 0xfff));
	pm2_WR(par, PM2R_RECTANGLE_ORIGIN, (modded.dy << 16) | modded.dx);
	pm2_WR(par, PM2R_RECTANGLE_SIZE, (modded.height << 16) | modded.width);
	wmb();
	pm2_WR(par, PM2R_RENDER, PM2F_RENDER_RECTANGLE |
				(modded.dx < modded.sx ? PM2F_INCREASE_X : 0) |
				(m