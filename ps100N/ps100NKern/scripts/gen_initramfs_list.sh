);

	i810fb_getcolreg(fg, &red, &green, &blue, &trans, info);
	temp = i810_readb(PIXCONF1, mmio);
	i810_writeb(PIXCONF1, mmio, temp | EXTENDED_PALETTE);

	i810_write_dac(5, red, green, blue, mmio);

	i810_writeb(PIXCONF1, mmio, temp);
}

/**
 * i810_init_cursor - initializes the cursor
 * @par: pointer to i810fb_par structure
 *
 * DESCRIPTION:
 * Initializes the cursor registers
 */
static void i810_init_cursor(struct i810fb_par *par)
{
	u8 __iomem *mmio = par->mmio_start_virtual;

	i810_enable_cursor(mmio, OFF);
	i810_writel(CURBASE, mmio, par->cursor_heap.physical);
	i810_writew(CURCNTR, mmio, COORD_ACTIVE | CURSOR_MODE_64_XOR);
}	

/*********************************************************************
 *                    Framebuffer hook helpers                       *
 *********************************************************************/
/**
 * i810_round_off -  Round off values to capability of hardware
 * @var: pointer to fb_var_screeninfo structure
 *
 * DESCRIPTION:
 * @var contains user-defined information for the mode to be set.
 * This will try modify those values to ones nearest the
 * capability of the hardware
 */
static void i810_round_off(struct fb_var_screeninfo *var)
{
	u32 xres, yres, vxres, vyres;

	/*
	 *  Presently supports only these configurations 
	 */

	xres = var->xres;
	yres = var->yres;
	vxres = var->xres_virtual;
	vyres = var->yres_virtual;

	var->bits_per_pixel += 7;
	var->bits_per_pixel &= ~7;
	
	if (var->bits_per_pixel < 8)
		var->bits_per_pixel = 8;
	if (var->bits_per_pixel > 32) 
		var->bits_per_pixel = 32;

	round_off_xres(&xres);
	if (xres < 40)
		xres = 40;
	if (xres > 2048) 
		xres = 2048;
	xres = (xres + 7) & ~7;

	if (vxres < xres) 
		vxres = xres;

	round_off_yres(&xres, &yres);
	if (yres < 1)
		yres = 1;
	if (yres >= 2048)
		yres = 2048;

	if (vyres < yres) 
		vyres = yres;

	if (var->bits_per_pixel == 32)
		var->accel_flags = 0;

	/* round of horizontal timings to nearest 8 pixels */
	var->left_margin = (var->left_margin + 4) & ~7;
	var->right_margin = (var->right_margin + 4) & ~7;
	var->hsync_len = (var->hsync_len + 4) & ~7;

	if (var->vmode & FB_VMODE_INTERLACED) {
		if (!((yres + var->upper_margin + var->vsync_len + 
		       var->lower_margin) & 1))
			var->upper_margin++;
	}
	
	var->xres = xres;
	var->yres = yres;
	var->xres_virtual = vxres;
	var->yres_virtual = vyres;
}	

/**
 * set_color_bitfields - sets rgba fields
 * @var: pointer to fb_var_screeninfo
 *
 * DESCRIPTION:
 * The length, offset and ordering  for each color field 
 * (red, green, blue)  will be set as specified 
 * by the hardware
 */  
static void set_color_bitfields(struct fb_var_screeninfo *var)
{
	switch (var->bits_per_pixel) {
	case 8:       
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 0;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 16:
		var->green.length = (var->green.length == 5) ? 5 : 6;
		var->red.length = 5;
		var->blue.length = 5;
		var->transp.length = 6 - var->green.length;
		var->blue.offset = 0;
		var->green.offset = 5;
		var->red.offset = 5 + var->green.length;
		var->transp.offset =  (5 + var->red.offset) & 15;
		break;
	case 24:	/* RGB 888   */
	case 32:	/* RGBA 8888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.length = var->bits_per_pixel - 24;
		var->transp.offset = (var->transp.length) ? 24 : 0;
		break;
	}
	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;
}

/**
 * i810_check_params - check if contents in var are valid
 * @var: pointer to fb_var_screeninfo
 * @info: pointer to fb_info
 *
 * DESCRIPTION:
 * This will check if the framebuffer size is sufficient 
 * for the current mode and if the user's monitor has the 
 * required specifications to display the current mode.
 */
static int i810_check_params(struct fb_var_screeninfo *var, 
			     struct fb_info *info)
{
	struct i810fb_par *par = info->par;
	int line_length, vidmem, mode_valid = 0, retval = 0;
	u32 vyres = var->yres_virtual, vxres = var->xres_virtual;

	/*
	 *  Memory limit
	 */
	line_length = get_line_length(par, vxres, var->bits_per_pixel);
	vidmem = line_length*vyres;

	if (vidmem > par->fb.size) {
		vyres = par->fb.size/line_length;
		if (vyres < var->yres) {
			vyres = info->var.yres;
			vxres = par->fb.size/vyres;
			vxres /= var->bits_per_pixel >> 3;
			line_length = get_line_length(par, vxres, 
						      var->bits_per_pixel);
			vidmem = line_length * info->var.yres;
			if (vxres < var->xres) {
				printk("i810fb: required video memory, "
				       "%d bytes, for %dx%d-%d (virtual) "
				       "is out of range\n", 
				       vidmem, vxres, vyres, 
				       var->bits_per_pixel);
				return -ENOMEM;
			}
		}
	}

	var->xres_virtual = vxres;
	var->yres_virtual = vyres;

	/*
	 * Monitor limit
	 */
	switch (var->bits_per_pixel) {
	case 8:
		info->monspecs.dclkmax = 234000000;
		break;
	case 16:
		info->monspecs.dclkmax = 229000000;
		break;
	case 24:
	case 32:
		info->monspecs.dclkmax = 204000000;
		break;
	}

	info->monspecs.dclkmin = 15000000;

	if (!fb_validate_mode(var, info))
		mode_valid = 1;

#ifdef CONFIG_FB_I810_I2C
	if (!mode_valid && info->monspecs.gtf &&
	    !fb_get_mode(FB_MAXTIMINGS, 0, var, info))
		mode_valid = 1;

	if (!mode_valid && info->monspecs.modedb_len) {
		const struct fb_videomode *mode;

		mode = fb_find_best_mode(var, &info->modelist);
		if (mode) {
			fb_videomode_to_var(var, mode);
			mode_valid = 1;
		}
	}
#endif
	if (!mode_valid && info->monspecs.modedb_len == 0) {
		if (fb_get_mode(FB_MAXTIMINGS, 0, var, info)) {
			int default_sync = (info->monspecs.hfmin-HFMIN)
				|(info->monspecs.hfmax-HFMAX)
				|(info->monspecs.vfmin-VFMIN)
				|(info->monspecs.vfmax-VFMAX);
			printk("i810fb: invalid video mode%s\n",
			       default_sync ? "" : ". Specifying "
			       "vsyncN/hsyncN parameters may help");
			retval = -EINVAL;
		}
	}

	return retval;
}	

/**
 * encode_fix - fill up fb_fix_screeninfo structure
 * @fix: pointer to fb_fix_screeninfo
 * @info: pointer to fb_info
 *
 * DESCRIPTION:
 * This will set up parameters that are unmodifiable by the user.
 */
static int encode_fix(struct fb_fix_screeninfo *fix, struct fb_info *info)
{
	struct i810fb_par *par = info->par;

    	memset(fix, 0, sizeof(struct fb_fix_screeninfo));

    	strcpy(fix->id, "I810");
	mutex_lock(&info->mm_lock);
    	fix->smem_start = par->fb.physical;
    	fix->smem_len = par->fb.size;
	mutex_unlock(&info->mm_lock);
    	fix->type = FB_TYPE_PACKED_PIXELS;
    	fix->type_aux = 0;
	fix->xpanstep = 8;
	fix->ypanstep = 1;

    	switch (info->var.bits_per_pixel) {
	case 8:
	    	fix->visual = FB_VISUAL_PSEUDOCOLOR;
	    	break;
	case 16:
	case 24:
	case 32:
		if (info->var.nonstd)
			fix->visual = FB_VISUAL_DIRECTCOLOR;
		else
			fix->visual = FB_VISUAL_TRUECOLOR;
	    	break;
	default:
		return -EINVAL;
	}
    	fix->ywrapstep = 0;
	fix->line_length = par->pitch;
	fix->mmio_start = par->mmio_start_phys;
	fix->mmio_len = MMIO