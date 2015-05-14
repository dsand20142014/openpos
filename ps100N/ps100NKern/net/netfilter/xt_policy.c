 var->green.offset, var->blue.offset,
		var->transp.offset);

	return 0;
}

/*
 * imxfb_set_par():
 *	Set the user defined part of the display for the specified console
 */
static int imxfb_set_par(struct fb_info *info)
{
	struct imxfb_info *fbi = info->par;
	struct fb_var_screeninfo *var = &info->var;

	if (var->bits_per_pixel == 16 || var->bits_per_pixel == 32)
		info->fix.visual = FB_VISUAL_TRUECOLOR;
	else if (!fbi->cmap_static)
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else {
		/*
		 * Some people have weird ideas about wanting static
		 * pseudocolor maps.  I suspect their user space
		 * applications are broken.
		 */
		info->fix.visual = FB_VISUAL_STATIC_PSEUDOCOLOR;
	}

	info->fix.line_length = var->xres_virtual * var->bits_per_pixel / 8;
	fbi->palette_size = var->bits_per_pixel == 8 ? 256 : 16;

	imxfb_activate_var(var, info);

	return 0;
}

static void imxfb_enable_controller(struct imxfb_info *fbi)
{
	pr_debug("Enabling LCD controller\n");

	writel(fbi->screen_dma, fbi->regs + LCDC_SSA);

	/* physical screen start address	    */
	writel(VPW_VPW(fbi->max_xres * fbi->max_bpp / 8 / 4),
		fbi->regs + LCDC_VPW);

	/* panning offset 0 (0 pixel offset)        */
	writel(0x00000000, fbi->regs + LCDC_POS);

	/* disable hardware cursor */
	writel(readl(fbi->regs + LCDC_CPOS) & ~(CPOS_CC0 | CPOS_CC1),
		fbi->regs + LCDC_CPOS);

	writel(RMCR_LCDC_EN, fbi->regs + LCDC_RMCR);

	clk_enable(fbi->clk);

	if (fbi->backlight_power)
		fbi->backlight_power(1);
	if (fbi->lcd_power)
		fbi->lcd_power(1);
}

static void imxfb_disable_controller(struct imxfb_info *fbi)
{
	pr_debug("Disabling LCD controller\n");

	if (fbi->backlight_power)
		fbi->backlight_power(0);
	if (fbi->lcd_power)
		fbi->lcd_power(0);

	clk_disable(fbi->clk);

	writel(0, fbi->regs + LCDC_RMCR);
}

static int imxfb_blank(int blank, struct fb_info *info)
{
	struct imxfb_info *fbi = info->par;

	pr_debug("imxfb_blank: blank=%d\n", blank);

	switch (blank) {
	case FB_BLANK_POWERDOWN:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_NORMAL:
		imxfb_disable_controller(fbi);
		break;

	case FB_BLANK_UNBLANK:
		imxfb_enable_controller(fbi);
		break;
	}
	return 0;
}

static struct fb_ops imxfb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= imxfb_check_var,
	.fb_set_par	= imxfb_set_par,
	.fb_setcolreg	= imxfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_blank	= imxfb_blank,
};

/*
 * imxfb_activate_var():
 *	Configures LCD Controller based on entries in var parameter.  Settings are
 *	only written to the controller if changes were made.
 */
static int imxfb_activate_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct imxfb_info *fbi = info->par;
	unsigned int pcr, lcd_clk;
	unsigned long long tmp;

	pr_debug("var: xres=%d hslen=%d lm=%d rm=%d\n",
		var->xres, var->hsync_len,
		var->left_margin, var->right_margin);
	pr_debug("var: yres=%d vslen=%d um=%d bm=%d\n",
		var->yres, var->vsync_len,
		var->upper_margin, var->lower_margin);

#if DEBUG_VAR
	if (var->xres < 16        || var->xres > 1024)
		printk(KERN_ERR "%s: invalid xres %d\n",
			info->fix.id, var->xres);
	if (var->hsync_len < 1    || var->hsync_len > 64)
		printk(KERN_ERR "%s: invalid hsync_len %d\n",
			info->fix.id, var->hsync_len);
	if (var->left_margin > 255)
		printk(KERN_ERR "%s: invalid left_margin %d\n",
			info->fix.id, var->left_margin);
	if (var->right_margin > 255)
		printk(KERN_ERR "%s: invalid right_margin %d\n",
			info->fix.id, var->right_margin);
	if (var->yres < 1 || var->yres > 511)
		printk(KERN_ERR "%s: invalid yres %d\n",
			info->fix.id, var->yres);
	if (var->vsync_len > 100)
		printk(KERN_ERR "%s: invalid vsync_len %d\n",
			info->fix.id, var->vsync_len);
	if (var->upper_margin > 63)
		printk(KERN_ERR "%s: invalid upper_margin %d\n",
			info->fix.id, var->upper_margin);
	if (var->lower_margin > 255)
		printk(KERN_ERR "%s: invalid lower_margin %d\n",
			info->fix.id, var->lower_margin);
#endif

	writel(HCR_H_WIDTH(var->hsync_len - 1) |
		HCR_H_WAIT_1(var->right_margin - 1) |
		HCR_H_WAIT_2(var->left_margin - 3),
		fbi->regs + LCDC_HCR);

	writel(VCR_V_WIDTH(var->vsync_len) |
		VCR_V_WAIT_1(var->lower_margin) |
		VCR_V_WAIT_2(var->upper_margin),
		fbi->regs + LCDC_VCR);

	writel(SIZE_XMAX(var->xres) | SIZE_YMAX(var->yres),
			fbi->regs + LCDC_SIZE);

	lcd_clk = clk_get_rate(fbi->clk);
	tmp = var->pixclock * (unsigned long long)lcd_clk;
	do_div(tmp, 1000000);
	if (do_div(tmp, 1000000) > 500000)
		tmp++;
	pcr = (unsigned int)tmp;
	if (--pcr > 0x3F) {
		pcr = 0x3F;
		printk(KERN_WARNING "Must limit pixel clock to %uHz\n",
				lcd_clk / pcr);
	}

	/* add sync polarities */
	pcr |= fbi->pcr & ~0x3F;

	writel(pcr, fbi->regs + LCDC_PCR);
	writel(fbi->pwmr, fbi->regs + LCDC_PWMR);
	writel(fbi->lscr1, fbi->regs + LCDC_LSCR1);
	writel(fbi->dmacr, fbi->regs + LCDC_DMACR);

	return 0;
}

#ifdef CONF