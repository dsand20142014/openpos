;

	if (info->var.bits_per_pixel > 8) return 1; /* failsafe */

	local_irq_save(flags);
	
	/* From the VideoToolbox driver.  Seems to be saying that
	 * regno #254 and #255 are the important ones for 1-bit color,
	 * regno #252-255 are the important ones for 2-bit color, etc.
	 */
	_regno = regno + (256-(1 << info->var.bits_per_pixel));

	/* reset clut? (VideoToolbox sez "not necessary") */
	nubus_writeb(0xFF, &rbv_cmap_regs->cntl); nop();
	
	/* tell clut which address to use. */
	nubus_writeb(_regno, &rbv_cmap_regs->addr); nop();
	
	/* send one color channel at a time. */
	nubus_writeb(_red,   &rbv_cmap_regs->lut); nop();
	nubus_writeb(_green, &rbv_cmap_regs->lut); nop();
	nubus_writeb(_blue,  &rbv_cmap_regs->lut);
	
	local_irq_restore(flags); /* done. */
	return 0;
}

/* Macintosh Display Card (8x24) */
static int mdc_setpalette(unsigned int regno, unsigned int red,
			  unsigned int green, unsigned int blue,
			  struct fb_info *info)
{
	volatile struct mdc_cmap_regs *cmap_regs =
		nubus_slot_addr(video_slot);
	/* use MSBs */
	unsigned char _red  =red>>8;
	unsigned char _green=green>>8;
	unsigned char _blue =blue>>8;
	unsigned char _regno=regno;
	unsigned long flags;

	local_irq_save(flags);
	
	/* the nop's are there to order writes. */
	nubus_writeb(_regno, &cmap_regs->addr); nop();
	nubus_writeb(_red, &cmap_regs->lut);    nop();
	nubus_writeb(_green, &cmap_regs->lut);  nop();
	nubus_writeb(_blue, &cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}

/* Toby frame buffer */
static int toby_setpalette(unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *info) 
{
	volatile struct toby_cmap_regs *cmap_regs =
		nubus_slot_addr(video_slot);
	unsigned int bpp = info->var.bits_per_pixel;
	/* use MSBs */
	unsigned char _red  =~(red>>8);
	unsigned char _green=~(green>>8);
	unsigned char _blue =~(blue>>8);
	unsigned char _regno = (regno << (8 - bpp)) | (0xFF >> bpp);
	unsigned long flags;

	local_irq_save(flags);
		
	nubus_writeb(_regno, &cmap_regs->addr); nop();
	nubus_writeb(_red, &cmap_regs->lut);    nop();
	nubus_writeb(_green, &cmap_regs->lut);  nop();
	nubus_writeb(_blue, &cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}

/* Jet frame buffer */
static int jet_setpalette(unsigned int regno, unsigned int red,
			  unsigned int green, unsigned int blue,
			  struct fb_info *info)
{
	volatile struct jet_cmap_regs *cmap_regs =
		nubus_slot_addr(video_slot);
	/* use MSBs */
	unsigned char _red   = (red>>8);
	unsigned char _green = (green>>8);
	unsigned char _blue  = (blue>>8);
	unsigned long flags;

	local_irq_save(flags);
	
	nubus_writeb(regno, &cmap_regs->addr); nop();
	nubus_writeb(_red, &cmap_regs->lut); nop();
	nubus_writeb(_green, &cmap_regs->lut); nop();
	nubus_writeb(_blue, &cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}

/*
 * Civic framebuffer -- Quadra AV built-in video.  A chip
 * called Sebastian holds the actual color palettes, and
 * apparently, there are two different banks of 512K RAM 
 * which can act as separate framebuffers for doing video
 * input and viewing the screen at the same time!  The 840AV
 * Can add another 1MB RAM to give the two framebuffers 
 * 1MB RAM apiece.
 *
 * FIXME: this doesn't seem to work anymore.
 */
static int civic_setpalette (unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     struct fb_info *info)
{
	static int lastreg = -1;
	unsigned long flags;
	int clut_status;
	
	if (info->var.bits_per_pixel > 8) return 1; /* failsafe */

	red   >>= 8;
	green >>= 8;
	blue  >>= 8;

	local_irq_save(flags);
	
	/*
	 * Set the register address
	 */
	nubus_writeb(regno, &civic_cmap_regs->addr); nop();

	/*
	 * Wait for VBL interrupt here;
	 * They're usually not enabled from Penguin, so we won't check
	 */
#if 0
	{
#define CIVIC_VBL_OFFSET	0x120
		volatile unsigned long *vbl = nubus_readl(civic_cmap_regs->vbl_addr + CIVIC_VBL_OFFSET);
		/* do interrupt setup stuff here? */
		*vbl = 0L; nop();	/* clear */
		*vbl = 1L; nop();	/* set */
		while (*vbl != 0L)	/* wait for next vbl */
		{
			usleep(10);	/* needed? */
		}
		/* do interrupt shutdown stuff here? */
	}
#endif

	/*
	 * Grab a status word and do some checking;
	 * Then finally write the clut!
	 */
	clut_status =  nubus_readb(&civic_cmap_regs->status2);

	if ((clut_status & 0x0008) == 0)
	{
#if 0
		if ((clut_status & 0x000D) != 0)
		{
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
		}
#endif

		nubus_writeb(  red, &civic_cmap_regs->lut); nop();
		nubus_writeb(green, &civic_cmap_regs->lut); nop();
		nubus_writeb( blue, &civic_cmap_regs->lut); nop();
		nubus_writeb( 0x00, &civic_cmap_regs->lut); nop();
	}
	else
	{
		unsigned char junk;

		junk = nubus_readb(&civic_cmap_regs->lut); nop();
		junk = nubus_readb(&civic_cmap_regs->lut); nop();
		junk = nubus_readb(&civic_cmap_regs->lut); nop();
		junk = nubus_readb(&civic_cmap_regs->lut); nop();

		if ((clut_status & 0x000D) != 0)
		{
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
		}

		nubus_writeb(  red, &civic_cmap_regs->lut); nop();
		nubus_writeb(green, &civic_cmap_regs->lut); nop();
		nubus_writeb( blue, &civic_cmap_regs->lut); nop();
		nubus_writeb( junk, &civic_cmap_regs->lut); nop();
	}

	local_irq_restore(flags);
	lastreg = regno;
	return 0;
}

/*
 * The CSC is the framebuffer on the PowerBook 190 series
 * (and the 5300 too, but that's a PowerMac). This function
 * brought to you in part by the ECSC driver for MkLinux.
 */

static int csc_setpalette (unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *info)
{
	mdelay(1);
	nubus_writeb(regno, &csc_cmap_regs->clut_waddr);
	nubus_writeb(red,   &csc_cmap_regs->clut_data);
	nubus_writeb(green, &csc_cmap_regs->clut_data);
	nubus_writeb(blue,  &csc_cmap_regs->clut_data);
	return 0;
}

static int macfb_setcolreg(unsigned regno, unsigned red, unsigned green,
			   unsigned blue, unsigned transp,
			   struct fb_info *fb_info)
{
	/*
	 *  Set a single color register. The values supplied are
	 *  already rounded down to the hardware's capabilities
	 *  (according to the entries in the `var' structure). Return
	 *  != 0 for invalid regno.
	 */
	
	if (regno >= fb_info->cmap.len)
		return 1;

	if (fb_info->var.bits_per_pixel <= 8) {
		switch (fb_info->var.bits_per_pixel) {
		case 1:
			/* We shouldn't get here */
			break;
		case 2:
		case 4:
		case 8:
			if (macfb_setpalette)
				macfb_setpalette(regno, red, green, blue,
						 fb_info);
			else
				return 1;
			break;
		}
	} else if (regno < 16) {
		switch (fb_info->var.bits_per_pixel) {
		case 16:
			if (fb_info->var.red.offset == 10) {
				/* 1:5:5:5 */
				((u32*) (fb_info->pseudo_palette))[regno] =
					((red   & 0xf800) >>  1) |
					((green & 0xf800) >>  6) |
					((blue  & 0xf800) >> 11) |
					((transp != 0) << 15);
			} else {
				/* 0:5:6:5 */
				((u32*) (fb_info->pseudo_palette))[regno] =
					((red   & 0xf800)      ) |
					((green & 0xfc00) >>  5) |
					((blue  & 0xf800) >> 11);
			}
			break;
			/* I'm pretty sure that one or the other of these
			   doesn't exist on 68k Macs */
		case 24:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((u32 *)(fb_info->pseudo_palette))[regno] =
				(red   << fb_info->var.red.offset)   |
				(green << fb_info->var.green.offset) |
				(blue  << fb_info->var.blue.offset);
			break;
		case 32:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((u32 *)(fb_info->pseudo_palette))[regno] =
				(red   << fb_info->var.red.offset)   |
				(green << fb_info->var.green.offset) |
				(blue  << fb_info->var.blue.offset);
			break;
		}
	}

	return 0;
}

static struct fb_ops macfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= macfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static void __init macfb_setup(char *options)
{
	char *this_opt;
	
	if (!options || !*options)
		return;
	
	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt) continue;
		
		if (! strcmp(this_opt, "inverse"))
			inverse=1;
		/* This means "turn on experimental CLUT code" */
		else if (!strcmp(this_opt, "vidtest"))
			vidtest=1;
	}
}

static void __init iounmap_macfb(void)
{
	if (valkyrie_cmap_regs)
		iounmap(valkyrie_cmap_regs);
	if (dafb_cmap_regs)
		iounmap(dafb_cmap_regs);
	if (v8_brazil_cmap_regs)
		iounmap(v8_brazil_cmap_regs);
	if (rbv_cmap_regs)
		iounmap(rbv_cmap_regs);
	if (civic_cmap_regs)
		iounmap(civic_cmap_regs);
	if (csc_cmap_regs)
		iounmap(csc_cmap_regs);
}

static int __init macfb_init(void)
{
	int video_cmap_len, video_is_nubus = 0;
	struct nubus_dev* ndev = NULL;
	char *option = NULL;
	int err;

	if (fb_get_options("macfb", &option))
		return -ENODEV;
	macfb_setup(option);

	if (!MACH_IS_MAC) 
		return -ENODEV;

	/* There can only be one internal video controller anyway so
	   we're not too worried about this */
	macfb_defined.xres = mac_bi_data.dimensions & 0xFFFF;
	macfb_defined.yres = mac_bi_data.dimensions >> 16;
	macfb_defined.bits_per_pixel = mac_bi_data.videodepth;
	macfb_fix.line_length = mac_bi_data.videorow;
	macfb_fix.smem_len = macfb_fix.line_length * macfb_defined.yres;
	/* Note: physical address (since 2.1.127) */
	macfb_fix.smem_start = mac_bi_data.videoaddr;
	/* This is actually redundant with the initial mappings.
	   However, there are some non-obvious aspects to the way
	   those mappings are set up, so this is in fact the safest
	   way to ensure that this driver will work on every possible
	   Mac */
	fb_info.screen_base = ioremap(mac_bi_data.videoaddr, macfb_fix.smem_len);
	
	printk("macfb: framebuffer at 0x%08lx, mapped to 0x%p, size %dk\n",
	       macfb_fix.smem_start, fb_info.screen_base, macfb_fix.smem_len/1024);
	printk("macfb: mode is %dx%dx%d, linelength=%d\n",
	       macfb_defined.xres, macfb_defined.yres, macfb_defined.bits_per_pixel, macfb_fix.line_length);
	
	/*
	 *	Fill in the available video resolution
	 */
	 
	macfb_defined.xres_virtual   = macfb_defined.xres;
	macfb_defined.yres_virtual   = macfb_defined.yres;
	macfb_defined.height = PIXEL_TO_MM(macfb_defined.yres);
	macfb_defined.width  = PIXEL_TO_MM(macfb_defined.xres);	 

	printk("macfb: scrolling: redraw\n");
	macfb_defined.yres_virtual = macfb_defined.yres;

	/* some dummy values for timing to make fbset happy */
	macfb_defined.pixclock     = 10000000 / macfb_defined.xres * 1000 / macfb_defined.yres;
	macfb_defined.left_margin  = (macfb_defined.xres / 8) & 0xf8;
	macfb_defined.hsync_len    = (macfb_defined.xres / 8) & 0xf8;

	switch (macfb_defined.bits_per_pixel) {
	case 1:
		/* XXX: I think this will catch any program that tries
		   to do FBIO_PUTCMAP when the visual is monochrome */
		macfb_defined.red.length = macfb_defined.bits_per_pixel;
		macfb_defined.green.length = macfb_defined.bits_per_pixel;
		macfb_defined.blue.length = macfb_defined.bits_per_pixel;
		video_cmap_len = 0;
		macfb_fix.visual = FB_VISUAL_MONO01;
		break;
	case 2:
	case 4:
	case 8:
		macfb_defined.red.length = macfb_defined.bits_per_pixel;
		macfb_defined.green.length = macfb_defined.bits_per_pixel;
		macfb_defined.blue.length = macfb_defined.bits_per_pixel;
		video_cmap_len = 1 << macfb_defined.bits_per_pixel;
		macfb_fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	case 16:
		macfb_defined.transp.offset = 15;
		macfb_defined.transp.length = 1;
		macfb_defined.red.offset = 10;
		macfb_defined.red.length = 5;
		macfb_defined.green.offset = 5;
		macfb_defined.green.length = 5;
		macfb_defined.blue.offset = 0;
		macfb_defined.blue.length = 5;
		printk("macfb: directcolor: "
		       "size=1:5:5:5, shift=15:10:5:0\n");
		video_cmap_len = 16;
		/* Should actually be FB_VISUAL_DIRECTCOLOR, but this
		   works too */
		macfb_fix.visual = FB_VISUAL_TRUECOLOR;
		break;
	case 24:
	case 32:
		/* XXX: have to test these... can any 68k Macs
		   actually do this on internal video? */
		macfb_defined.red.offset = 16;
		macfb_defined.red.length = 8;
		macfb_defined.green.offset = 8;
		macfb_defined.green.length = 8;
		macfb_defined.blue.offset = 0;
		macfb_defined.blue.length = 8;
		printk("macfb: truecolor: "
		       "size=0:8:8:8, shift=0:16:8:0\n");
		video_cmap_len = 16;
		macfb_age.  On failure, *@lsn is undefined.
 *
 * The following error codes are defined:
 *	-EINVAL	- The restart page is inconsistent.
 *	-ENOMEM	- Not enough memory to load the restart page.
 *	-EIO	- Failed to reading from $LogFile.
 */
static int ntfs_check_and_load_restart_page(struct inode *vi,
		RESTART_PAGE_HEADER *rp, s64 pos, RESTART_PAGE_HEADER **wrp,
		LSN *lsn)
{
	RESTART_AREA *ra;
	RESTART_PAGE_HEADER *trp;
	int size, err;

	ntfs_debug("Entering.");
	/* Check the restart page header for consistency. */
	if (!ntfs_check_restart_page_header(vi, rp, pos)) {
		/* Error output already done inside the function. */
		return -EINVAL;
	}
	/* Check the restart area for consistency. */
	if (!ntfs_check_restart_area(vi, rp)) {
		/* Error output already done inside the function. */
		return -EINVAL;
	}
	ra = (RESTART_AREA*)((u8*)rp + le16_to_cpu(rp->restart_area_offset));
	/*
	 * Allocate a buffer to store the whole restart page so we can multi
	 * sector transfer deprotect it.
	 */
	trp = ntfs_malloc_nofs(le32_to_cpu(rp->system_page_size));
	if (!trp) {
		ntfs_error(vi->i_sb, "Failed to allocate memory for $LogFile "
				"restart page buffer.");
		return -ENOMEM;
	}
	/*
	 * Read the whole of the restart page into the buffer.  If it fits
	 * completely inside @rp, just copy it from there.  Otherwise map all
	 * the required pages and copy the data from them.
	 */
	size = PAGE_CACHE_SIZE - (pos & ~PAGE_CACHE_MASK);
	if (size >= le32_to_cpu(rp->system_page_size)) {
		memcpy(trp, rp, le32_to_cpu(rp->system_page_size));
	} else {
		pgoff_t idx;
		struct page *page;
		int have_read, to_read;

		/* First copy what we already have in @rp. */
		memcpy(trp, rp, size);
		/* Copy the remaining data one page at a time. */
		have_read = size;
		to_read = le32_to_cpu(rp->system_page_size) - size;
		idx = (pos + size) >> PAGE_CACHE_SHIFT;
		BUG_ON((pos + size) & ~PAGE_CACHE_MASK);
		do {
			page = ntfs_map_page(vi->i_mapping, idx);
			if (IS_ERR(page)) {
				ntfs_error(vi->i_sb, "Error mapping $LogFile "
						"page (index %lu).", idx);
				err = PTR_ERR(page);
				if (err != -EIO && err != -ENOMEM)
					err = -EIO;
				goto err_out;
			}
			size = min_t(int, to_read, PAGE_CACHE_SIZE);
			memcpy((u8*)trp + have_read, page_address(page), size);
			ntfs_unmap_page(page);
			have_read += size;
			to_read -= size;
			idx++;
		} while (to_read > 0);
	}
	/*
	 * Perform the multi sector transfer deprotection on the buffer if the
	 * restart page is protected.
	 */
	if ((!ntfs_is_chkd_record(trp->magic) || le16_to_cpu(trp->usa_count))
			&& post_read_mst_fixup((NTFS_RECORD*)trp,
			le32_to_cpu(rp->system_page_size))) {
		/*
		 * A multi sector tranfer error was detected.  We only need to
		 * abort if the restart page contents exceed the multi sector
		 * transfer fixup of the first sector.
		 */
		if (le16_to_cpu(rp->restart_area_offset) +
				le16_to_cpu(ra->restart_area_length) >
				NTFS_BLOCK_SIZE - sizeof(u16)) {
			ntfs_error(vi->i_sb, "Multi sector transfer error "
					"detected in $LogFile restart page.");
			err = -EINVAL;
			goto err_out;
		}
	}
	/*
	 * If the restart page is modified by chkdsk or there are no active
	 * logfile clients, the logfile is consistent.  Otherwise, need to
	 * check the log client records for consistency, too.
	 */
	err = 0;
	if (ntfs_is_rstr_record(rp->magic) &&
			ra->client_in_use_list != LOGFILE_NO_CLIENT) {
		if (!ntfs_check_log_client_array(vi, trp)) {
			err = -EINVAL;
			goto err_out;
		}
	}
	if (lsn) {
		if (ntfs_is_rstr_record(rp->magic))
			*lsn = sle64_to_cpu(ra->current_lsn);
		else /* if (ntfs_is_chkd_record(rp->magic)) */
			*lsn = sle64_to_cpu(rp->chkdsk_lsn);
	}
	ntfs_debug("Done.");
	if (wrp)
		*wrp = trp;
	else {
err_out:
		ntfs_free(trp);
	}
	return err;
}

/**
 * ntfs_check_logfile - check the journal for consistency
 * @log_vi:	struct inode of loaded journal $LogFile to check
 * @rp:		[OUT] on success this is a copy of the current restart page
 *
 * Check the $LogFile journal for consistency and return 'true' if it is
 * consistent and 'false' if not.  On success, the current restart pagv_cmap_regs = ioremap(DAC_BASE, 0x1000);
			break;

			/* AVs use the Civic framebuffer */
		case MAC_MODEL_Q840:
		case MAC_MODEL_C660:
			macfb_setpalette = civic_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			strcpy(macfb_fix.id, "Civic");
			civic_cmap_regs = ioremap(CIVIC_BASE, 0x1000);
			break;

		
			/* Write a setpalette function for your machine, then
			   you can add something similar here.  These are
			   grouped by classes of video chipsets.  Some of this
			   information is from the VideoToolbox "Bugs" web
			   page at
			   http://rajsky.psych.nyu.edu/Tips/VideoBugs.html */

			/* Assorted weirdos */
			/* We think this may be like the LC II */
		case MAC_MODEL_LC:
			if (vidtest) {
				macfb_setpalette = v8_brazil_setpalette;
				macfb_defined.activate = FB_ACTIVATE_NOW;
				v8_brazil_cmap_regs =
					ioremap(DAC_BASE, 0x1000);
			}
			strcpy(macfb_fix.id, "LC");
			break;
			/* We think this may be like the LC II */
		case MAC_MODEL_CCL:
			if (vidtest) {
				macfb_setpalette = v8_brazil_setpalette;
				macfb_defined.activate = FB_ACTIVATE_NOW;
				v8_brazil_cmap_regs =
					ioremap(DAC_BASE, 0x1000);
			}
			strcpy(macfb_fix.id, "Color Classic");
			break;

			/* And we *do* mean "weirdos" */
		case MAC_MODEL_TV:
			strcpy(macfb_fix.id, "Mac TV");
			break;

			/* These don't have colour, so no need to worry */
		case MAC_MODEL_SE30:
		case MAC_MODEL_CLII:
			strcpy(macfb_fix.id, "Monochrome");
			break;

			/* Powerbooks are particularly difficult.  Many of
			   them have separate framebuffers for external and
			   internal video, which is admittedly pretty cool,
			   but will be a bit of a headache to support here.
			   Also, many of them are grayscale, and we don't
			   really support that. */

		case MAC_MODEL_PB140:
		case MAC_MODEL_PB145:
		case MAC_MODEL_PB170:
			strcpy(macfb_fix.id, "DDC");
			break;

			/* Internal is GSC, External (if present) is ViSC */
		case MAC_MODEL_PB150:	/* no external video */
		case MAC_MODEL_PB160:
		case MAC_MODEL_PB165:
		case MAC_MODEL_PB180:
		case MAC_MODEL_PB210:
		case MAC_MODEL_PB230:
			strcpy(macfb_fix.id, "GSC");
			break;

			/* Internal is TIM, External is ViSC */
		case MAC_MODEL_PB165C:
		case MAC_MODEL_PB180C:
			strcpy(macfb_fix.id, "TIM");
			break;

			/* Internal is CSC, External is Keystone+Ariel. */
		case MAC_MODEL_PB190:	/* external video is optional */
		case MAC_MODEL_PB520:
		case MAC_MODEL_PB250:
		case MAC_MODEL_PB270C:
		case MAC_MODEL_PB280:
		case MAC_MODEL_PB280C:
			macfb_setpalette = csc_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			strcpy(macfb_fix.id, "CSC");
			csc_cmap_regs = ioremap(CSC_BASE, 0x1000);
			break;
		
		default:
			strcpy(macfb_fix.id, "Unknown");
			break;
		}

	fb_info.fbops		= &macfb_ops;
	fb_info.var		= macfb_defined;
	fb_info.fix		= macfb_fix;
	fb_info.pseudo_palette	= pseudo_palette;
	fb_info.flags		= FBINFO_DEFAULT;

	err = fb_alloc_cmap(&fb_info.cmap, video_cmap_len, 0);
	if (err)
		goto fail_unmap;
	
	err = register_framebuffer(&fb_info);
	if (err)
		goto fail_dealloc;

	printk("fb%d: %s frame buffer device\n",
	       fb_info.node, fb_info.fix.id);
	return 0;

fail_dealloc:
	fb_dealloc_cmap(&fb_info.cmap);
fail_unmap:
	iounmap(fb_info.screen_base);
	iounmap_macfb();
	return err;
}

module_init(macfb_init);
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  s) {
				pos = NTFS_BLOCK_SIZE >> 1;
				continue;
			}
			/*
			 * We have now found the second (modified by chkdsk)
			 * restart page, so we can stop looking.
			 */
			break;
		}
		/*
		 * Error output already done inside the function.  Note, we do
		 * not abort if the restart page was invalid as we might still
		 * find a valid one further in the file.
		 */
		if (err != -EINVAL) {
			ntfs_unmap_page(page);
			goto err_out;
		}
		/* Continue looking. */
		if (!pos)
			pos = NTFS_BLOCK_SIZE >> 1;
	}
	if (page)
		ntfs_unmap_page(page);
	if (logfile_is_empty) {
		NVolSetLogFileEmpty(vol);
is_empty:
		ntfs_debug("Done.  ($LogFile is empty.)");
		return true;
	}
	if (!rstr1_ph) {
		BUG_ON(rstr2_ph);
		ntfs_error(vol->sb, "Did not find any restart pages in "
				"$LogFile and it was not empty.");
		return false;
	}
	/* If both restart pages were found, use the more recent one. */
	if (rstr2_ph) {
		/*
		 * If the second restart area is more recent, switch to it.
		 * Otherwise just throw it away.
		 */
		if (rstr2_lsn > rstr1_lsn) {
			ntfs_debug("Using second restart page as it is more "
					"recent.");
			ntfs_free(rstr1_ph);
			rstr1_ph = rstr2_ph;
			/* rstr1_lsn = rstr2_lsn; */
		} else {
			ntfs_debug("Using first restart page as it is more "
					"recent.");
			ntfs_free(rstr2_ph);
		}
		rstr2_ph = NULL;
	}
	/* All consistency checks passed. */
	if (rp)
		*rp = rstr1_ph;
	else
		ntfs_free(rstr1_ph);
	ntfs_debug("Done.");
	return true;
err_out:
	if (rstr1_ph)
		ntfs_free(rstr1_ph);
	return false;
}

/**
 * ntfs_is_logfile_clean - check in the journal if the volume is clean
 * @log_vi:	struct inode of loaded journal $LogFile to check
 * @rp:		copy of the current restart page
 *
 * Analyze the $LogFile journal and return 'true' if it indicates the volume was
 * shutdown cleanly and 'false' if not.
 *
 * At present we only look at the two restart pages and ignore the log record
 * pages.  This is a little bit crude in that there will be a very small number
 * of cases where we think that a volume is dirty when in fact it is clean.
 * This should only affect volumes that have not been shutdown cleanly but did
 * not have any pending, non-check-pointed i/o, i.e. they were completely idle
 * at least for the five seconds preceeding the unclean shutdown.
 *
 * This function assumes that the $LogFile journal has already been consistency
 * checked by a call to ntfs_check_logfile() and in particular if the $LogFile
 * is empty this function requires that NVolLogFileEmpty() is true otherwise an
 * empty volume will be reported as dirty.
 */
bool ntfs_is_logfile_clean(struct inode *log_vi, const RESTART_PAGE_HEADER *rp)
{
	ntfs_volume *vol = NTFS_SB(log_vi->i_sb);
	RESTART_AREA *ra;

	ntfs_debug("Entering.");
	/* An empty $LogFile must have been clean before it got emptied. */
	if (NVolLogFileEmpty(vol)) {
		ntfs_debug("Done.  ($LogFile is empty.)");
		return true;
	}
	BUG_ON(!rp);
	if (!ntfs_is_rstr_record(rp->magic) &&
			!ntfs_is_chkd_record(rp->magic)) {
		ntfs_error(vol->sb, "Restart page buffer is invalid.  This is "
				"probably a bug in that the $LogFile should "
				"have been consistency checked before calling "
				"this function.");
		return false;
	}
	ra = (RESTART_AREA*)((u8*)rp + le16_to_cpu(rp->restart_area_offset));
	/*
	 * If the $LogFile has active clients, i.e. it is open, and we do not
	 * have the RESTART_VOLUME_IS_CLEAN bit set in the restart area flags,
	 * we assume there was an unclean shutdown.
	 */
	if (ra->client_in_use_list != LOGFILE_NO_CLIENT &&
			!(ra->flags & RESTART_VOLUME_IS_CLEAN)) {
		ntfs_debug("Done.  $LogFile indicates a dirty shutdown.");
		return false;
	}
	/* $LogFile indicates a clean shutdown. */
	ntfs_debug("Done.  $LogFile indicates a clean shutdown.");
	return true;
}

/**
 * ntfs_empty_logfile - empty the contents of the $LogFile journal
 * @log_vi:	struct inode of loaded journal $LogFile to empty
 *
 * Empty the contents of the $LogFile journal @log_vi and return 'true' on
 * success and 'false' on error.
 *
 * This function assumes that the $LogFile journal has already been consistency
 * checked by a call to ntfs_check_logfile() and that ntfs_is_logfile_clean()
 * has been used to ensure that the $LogFile is clean.
 */
bool ntfs_empty_logfile(struct inode *log_vi)
{
	VCN vcn, end_vcn;
	ntfs_inode *log_ni = NTFS_I(log_vi);
	ntfs_volume *vol = log_ni->vol;
	struct super_block *sb = vol->sb;
	runlist_element *rl;
	unsigned long flags;
	unsigned block_size, block_size_bits;
	int err;
	bool should_wait = true;

	ntfs_debug("Entering.");
	if (NVolLogFileEmpty(vol)) {
		ntfs_debug("Done.");
		return true;
	}
	/*
	 * We cannot use ntfs_attr_set() because we may be still in the middle
	 * of a mount operation.  Thus we do the emptying by hand by first
	 * zapping the page cache pages for the $LogFile/$DATA attribute and
	 * then emptying each of the buffers in each of the clusters specified
	 * by the runlist by hand.
	 */
	block_size = sb->s_blocksize;
	block_size_bits = sb->s_blocksize_bits;
	vcn = 0;
	read_lock_irqsave(&log_ni->size_lock, flags);
	end_vcn = (log_ni->initialized_size + vol->cluster_size_mask) >>
			vol->cluster_size_bits;
	read_unlock_irqrestore(&log_ni->size_lock, flags);
	truncate_inode_pages(log_vi->i_mapping, 0);
	down_write(&log_ni->runlist.lock);
	rl = log_ni->runlist.rl;
	if (unlikely(!rl || vcn < rl->vcn || !rl->length)) {
map_vcn:
		err = ntfs_map_runlist_nolock(log_ni, vcn, NULL);
		if (err) {
			ntfs_error(sb, "Failed to map runlist fragment (error "
					"%d).", -err);
			goto err;
		}
		rl = log_ni->runlist.rl;
		BUG_ON(!rl || vcn < rl->vcn || !rl->length);
	}
	/* Seek to the runlist element containing @vcn. */
	while (rl->length && vcn >= rl[1].vcn)
		rl++;
	do {
		LCN lcn;
		sector_t block, end_block;
		s64 len;

		/*
		 * If this run is not mapped map it now and start again as the
		 * runlist will have been updated.
		 */
		lcn = rl->lcn;
		if (unlikely(lcn == LCN_RL_NOT_MAPPED)) {
			vcn = rl->vcn;
			goto map_vcn;
		}
		/* If this run is not valid abort with an error. */
		if (unlikely(!rl->length || lcn < LCN_HOLE))
			goto rl_err;
		/* Skip holes. */
		if (lcn == LCN_HOLE)
			continue;
		block = lcn << vol->cluster_size_bits >> block_size_bits;
		len = rl->length;
		if (rl[1].vcn > end_vcn)
			len = end_vcn - rl->vcn;
		end_block = (lcn + len) << vol->cluster_size_bits >>
				block_size_bits;
		/* Iterate over the blocks in the run and empty them. */
		do {
			struct buffer_head *bh;

			/* Obtain the buffer, possibly not uptodate. */
			bh = sb_getblk(sb, block);
			BUG_ON(!bh);
			/* Setup buffer i/o submission. */
			lock_buffer(bh);
			bh->b_end_io = end_buffer_write_sync;
			get_bh(bh);
			/* Set the entire contents of the buffer to 0xff. */
			memset(bh->b_data, -1, block_size);
			if (!buffer_uptodate(bh))
				set_buffer_uptodate(bh);
			if (buffer_dirty(bh))
				clear_buffer_dirty(bh);
			/*
			 * Submit the buffer and wait for i/o to complete but
			 * only for the first buffer so we do not miss really
			 * serious i/o errors.  Once the first buffer has
			 * completed ignore errors afterwards as we can assume
			 * that if one buffer worked all of them will work.
			 */
			submit_bh(WRITE, bh);
			if (should_wait) {
				should_wait = false;
				wait_on_buffer(bh);
				if (unlikely(!buffer_uptodate(bh)))
					goto io_err;
			}
			brelse(bh);
		} while (++block < end_block);
	} while ((++rl)->vcn < end_vcn);
	up_write(&log_ni->runlist.lock);
	/*
	 * Zap the pages again just in case any got instantiated whilst we were
	 * emptying the blocks by hand.  FIXME: We may not have completed
	 * writing to all the buffer heads yet so this may happen too early.
	 * We really should use a kernel thread to do the emptying
	 * asynchronously and then we can also set the volume dirty and output
	 * an error message if emptying should fail.
	 */
	truncate_inode_pages(log_vi->i_mapping, 0);
	/* Set the flag so we do not have to do it again on remount. */
	NVolSetLogFileEmpty(vol);
	ntfs_debug("Done.");
	return true;
io_err:
	ntfs_error(sb, "Failed to write buffer.  Unmount and run chkdsk.");
	goto dirty_err;
rl_err:
	ntfs_error(sb, "Runlist is corrupt.  Unmount and run chkdsk.");
dirty_err:
	NVolSetErrors(vol);
	err = -EIO;
err:
	up_write(&log_ni->runlist.lock);
	ntfs_error(sb, "Failed to fill $LogFile with 0xff bytes (error %d).",
			-err);
	return false;
}

#endif /* NTFS_RW */
