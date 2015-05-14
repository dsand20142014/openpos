om.htotal = htotal2hw(par->htotal);
			custom.hbstrt = hbstrt2hw(par->hbstrt);
			custom.hbstop = hbstop2hw(par->hbstop);
			custom.hsstrt = hsstrt2hw(par->hsstrt);
			custom.hsstop = hsstop2hw(par->hsstop);
			custom.hcenter = hcenter2hw(par->hcenter);
			custom.vtotal = vtotal2hw(par->vtotal);
			custom.vbstrt = vbstrt2hw(par->vbstrt);
			custom.vbstop = vbstop2hw(par->vbstop);
			custom.vsstrt = vsstrt2hw(par->vsstrt);
			custom.vsstop = vsstop2hw(par->vsstop);
		}
	}
	if (!IS_OCS || par->hsstop)
		custom.beamcon0 = par->beamcon0;
	if (IS_AGA)
		custom.fmode = par->fmode;

	/*
	 * The minimum period for audio depends on htotal
	 */

	amiga_audio_min_period = div16(par->htotal);

	is_lace = par->bplcon0 & BPC0_LACE ? 1 : 0;
#if 1
	if (is_lace) {
		i = custom.vposr >> 15;
	} else {
		custom.vposw = custom.vposr | 0x8000;
		i = 1;
	}
#else
	i = 1;
	custom.vposw = custom.vposr | 0x8000;
#endif
	custom.cop2lc = (u_short *)ZTWO_PADDR(copdisplay.list[currentcop][i]);
}

	/*
	 * (Un)Blank the screen (called by VBlank interrupt)
	 */

static void ami_do_blank(void)
{
	struct amifb_par *par = &currentpar;
#if defined(CONFIG_FB_AMIGA_AGA)
	u_short bplcon3 = par->bplcon3;
#endif
	u_char