> 5;

	out_8(&p->valkyrie_regs->msense.r, 7);

	return sense;
}

/*
 * This routine takes a user-supplied var,
 * and picks the best vmode/cmode from it.
 */

/* [bkn] I did a major overhaul of this function.
 *
 * Much of the old code was "swiped by jonh from atyfb.c". Because
 * macmodes has mac_var_to_vmode, I felt that it would be better to
 * rework this function to use that, instead of reinventing the wheel to
 * add support for vmode 17. This was reinforced by the fact that
 * the previously swiped atyfb.c code is no longer there.
 *
 * So, I swiped and adapted platinum_var_to_par (from platinumfb.c), replacing
 * most, but not all, of the old code in the process. One side benefit of
 * swiping the platinumfb code is that we now have more comprehensible error
 * messages when a vmode/cmode switch fails. (Most of the error messages are
 * platinumfb.c, but I added two of my own, and I also changed some commas
 * into colons to make the messages more consistent with other Linux error
 * messages.) In addition, I think the new code *might* fix some vmode-
 * switching oddities, but I'm not sure.
 *
 * There may be some more opportunities for cleanup in here, but this is a
 * good start...
 */

static int valkyrie_var_to_par(struct fb_var_screeninfo *var,
	struct fb_par_valkyrie *par, const struct fb_info *fb_info)
{
	int vmode, cmode;
	struct valkyrie_regvals *init;
	struct fb_info_valkyrie *p = (struct fb_info_valkyrie *) fb_info;

	if (mac_var_to_vmode(var, &vmode, &cmode) != 0) {
		printk(KERN_ERR "valkyriefb: can't do %dx%dx%d.\n",
		       var->xres, var->yres, var->bits_per_pixel);
		return -EINVAL;
	}

	/* Check if we know about the wanted video mode */
	if (vmode < 1 || vmode > VMODE_MAX || !valkyrie_reg_init[vmode-1]) {
		printk(KERN_ERR "valkyriefb: vmode %d not valid.\n", vmode);
		return -EINVAL;
	}
	
	if (cmode != CMODE_8 && cmode != CMODE_16) {
		printk(KERN_ERR "valkyriefb: cmode %d not valid.\n", cmode);
		return -EINVAL;
	}

	if (var->xres_virtual > var->xres || var->yres_virtual > var->yres
	    || var->xoffset != 0 || var->yoffset != 0) {
		return -EINVAL;
	}

	init = valkyrie_reg_init[vmode-1];
	if (init->pitch[cmode] == 0) {
		printk(KERN_ERR "valkyriefb: vmode %d does not support "
		       "cmode %d.\n", vmode, cmode);
		return -EINVAL;
	}

	if (valkyrie_vram_reqd(vmode, cmode) > p->total_vram) {
		printk(KERN_ERR "valkyriefb: not enough ram for vmode %d, "
		       "cmode %d.\n", vmode, cmode);
		return -EINVAL;
	}

	par->vmode = vmode;
	par->cmode = cmode;
	par->init = init;
	par->xres = var->xres;
	par->yres = var->yres;
	par->vxres = par->xres;
	par->vyres = par->yres;

	return 0;
}

static void valkyrie_init_fix(struct fb_fix_screeninfo *fix, struct fb_info_valkyrie *p)
{
	memset(fix, 0, sizeof(*fix));
	strcpy(fix->id, "valkyrie");
	fix->mmio_s