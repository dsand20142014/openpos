
	int visual;
	int cmap_len;
	unsigned int ydstorg;
	MINFO_FROM_INFO(info);

	if (ACCESS_FBINFO(dead)) {
		return -ENXIO;
	}
	if ((err = matroxfb_decode_var(PMINFO var, &visual, &cmap_len, &ydstorg)) != 0)
		return err;
	return 0;
}

static int matroxfb_set_par(struct fb_info *info)
{
	int err;
	int visual;
	int cmap_len;
	unsigned int ydstorg;
	struct fb_var_screeninfo *var;
	MINFO_FROM_INFO(info);

	DBG(__func__)

	if (ACCESS_FBINFO(dead)) {
		return -ENXIO;
	}

	var = &info->var;
	if ((err = matroxfb_decode_var(PMINFO var, &visual, &cmap_len, &ydstorg)) != 0)
		return err;
	ACCESS_FBINFO(fbcon.screen_base) = vaddr_va(ACCESS_FBINFO(video.vbase)) + ydstorg;
	matroxfb_update_fix(PMINFO2);
	ACCESS_FBINFO(fbcon).fix.visual = visual;
	ACCESS_FBINFO(fbcon).fix.type = FB_TYPE_PACKED_PIXELS;
	ACCESS_FBINFO(fbcon).fix.type_aux = 0;
	ACCESS_FBINFO(fbcon).fix.line_length = (var->xres_virtual * var->bits_per_pixel) >> 3;
	{
		unsigned int pos;

		ACCESS_FBINFO(curr.cmap_len) = cmap_len;
		ydstorg += ACCESS_FBINFO(devflags.ydstorg);
		ACCESS_FBINFO(curr.ydstorg.bytes) = ydstorg;
		ACCESS_FBINFO(curr.ydstorg.chunks) = ydstorg >> (isInterleave(MINFO)?3:2);
		if (var->bits_per_pixel == 4)
			ACCESS_FBINFO(curr.ydstorg.pixels) = ydstorg;
		else
			ACCESS_FBINFO(curr.ydstorg.pixels) = (ydstorg * 8) / var->bits_per_pixel;
		ACCESS_FBINFO(curr.final_bppShift) = matroxfb_get_final_bppShift(PMINFO var->bits_per_pixel);
		{	struct my_timming mt;
			struct matrox_hw_state* hw;
			int out;

			matroxfb_var2my(var, &mt);
			mt.crtc = MATROXFB_SRC_CRTC1;
			/* CRTC1 delays */
			switch (var->bits_per_pixel) {
				case  0:	mt.delay = 31 + 0; break;
				case 16:	mt.delay = 21 + 8; break;
				case 24:	mt.delay = 17 + 8; break;
				case 32:	mt.delay = 16 + 8; break;
				default:	mt.delay = 31 + 8; break;
			}

			hw = &ACCESS_FBINFO(hw);

			down_read(&ACCESS_FBINFO(altout).lock);
			for (out = 0; out < MATROXFB_MAX_OUTPUTS; out++) {
				if (ACCESS_FBINFO(outputs[out]).src == MATROXFB_SRC_CRTC1 &&
				    ACCESS_FBINFO(outputs[out]).output->compute) {
					ACCESS_FBINFO(outputs[out]).output->compute(ACCESS_FBINFO(outputs[out]).data, &mt);
				}
			}
			up_read(&ACCESS_FBINFO(altout).lock);
			ACCESS_FBINFO(crtc1).pixclock = mt.pixclock;
			ACCESS_FBINFO(crtc1).mnp = mt.mnp;
			ACCESS_FBINFO(hw_switch->init(PMINFO &mt));
			pos = (var->yoffset * var->xres_virtual + var->xoffset) * ACCESS_FBINFO(curr.final_bppShift) / 32;
			pos += ACCESS_FBINFO(curr.ydstorg.chunks);

			hw->CRTC[0x0D] = pos & 0xFF;
			hw->CRTC[0x0C] = (pos & 0xFF00) >> 8;
			hw->CRTCEXT[0] = (hw->CRTCEXT[0] & 0xF0) | ((pos >> 16) & 0x0F) | ((pos >> 14) & 0x40);
			hw->CRTCEXT[8] = pos >> 21;
			ACCESS_FBINFO(hw_switch->restore(PMINFO2));
			update_crtc2(PMINFO pos);
			down_read(&ACCESS_FBINFO(altout).lock);
			for (out = 0; out < MATROXFB_MAX_OUTPUTS; out++) {
				if (ACCESS_FBINFO(outputs[out]).src == MATROXFB_SRC_CRTC1 &&
				    ACCESS_FBINFO(outputs[out]).output->program) {
					ACCESS_FBINFO(outputs[out]).output->program(ACCESS_FBINFO(outputs[out]).data);
				}
			}
			for (out = 0; out < MATROXFB_MAX_OUTPUTS; out++) {
				if (ACCESS_FBINFO(outputs[out]).src == MATROXFB_SRC_CRTC1 &&
				    ACCESS_FBINFO(outputs[out]).output->start) {
					ACCESS_FBINFO(outputs[out]).output->start(ACCESS_FBINFO(outputs[out]).data);
				}
			}
			up_read(&ACCESS_FBINFO(altout).lock);
			matrox_cfbX_init(PMINFO2);
		}
	}
	ACCESS_FBINFO(initialized) = 1;
	return 0;
}

static int matroxfb_get_vblank(WPMINFO struct fb_vblank *vblank)
{
	unsigned int sts1;

	matroxfb_enable_irq(PMINFO 0);
	memset(vblank, 0, sizeof(*vblank));
	vblank->flags = FB_VBLANK_HAVE_VCOUNT | FB_VBLANK_HAVE_VSYNC |
			FB_VBLANK_HAVE_VBLANK | FB_VBLANK_HAVE_HBLANK;
	sts1 = mga_inb(M_INSTS1);
	vblank->vcount = mga_inl(M_VCOUNT);
	/* BTW, on my PIII/450 with G400, reading M_INSTS1
	   byte makes this call about 12% slower (1.70 vs. 2.05 us
	   per ioctl()) */
	if (sts1 & 1)
		vblank->flags |= FB_VBLANK_HBLANKING;
	if (sts1 & 8)
		vblank->flags |= FB_VBLANK_VSYNCING;
	if (vblank->vcount >= ACCESS_FBINFO(fbcon).var.yres)
		vblank->flags |= FB_VBLANK_VBLANKING;
	if (test_bit(0, &ACCESS_FBINFO(irq_flags))) {
		vblank->flags |= FB_VBLANK_HAVE_COUNT;
		/* Only one writer, aligned int value...
		   it should work without lock and without atomic_t */
		vblank->count = ACCESS_FBINFO(crtc1).vsync.cnt;
	}
	return 0;
}

static struct matrox_altout panellink_output = {
	.name	 = "Panellink output",
};

static int matroxfb_ioctl(struct fb_info *info,
			  unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	MINFO_FROM