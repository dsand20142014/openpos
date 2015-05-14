RR MODULE_NAME ": Could not claim GPU MMIO.\n");
		err = -EBUSY;
		goto out_err_1;
	}
	par->gpu_mem = ioremap_nocache(par->gpu_mem_base, par->gpu_mem_size);
	if (par->gpu_mem == NULL) {
		printk(KERN_ERR MODULE_NAME ": Could not map GPU MMIO.\n");
		err = -ENOMEM;
		goto out_err_2;
	}

	return 0;

out_err_2:
	release_mem_region(par->gpu_mem_base, par->gpu_mem_size);
out_err_1:
	iounmap(par->vdc_mem);
out_err_0:
	release_mem_region(par->vdc_mem_base, par->vdc_mem_size);
	return err;
}

/*
 * Unmap the VDC and GPU register spaces.
 */

static void vmlfb_disable_mmio(struct vml_par *par)
{
	iounmap(par->gpu_mem);
	release_mem_region(par->gpu_mem_base, par->gpu_mem_size);
	iounmap(par->vdc_mem);
	release_mem_region(par->vdc_mem_base, par->vdc_mem_size);
}

/*
 * Release and uninit the VDC and GPU.
 */

static void vmlfb_release_devices(struct vml_par *par)
{
	if (atomic_dec_and_test(&par->refcount)) {
		pci_set_drvdata(par->vdc, NULL);
		pci_disable_device(par->gpu);
		pci_disable_device(par->vdc);
	}
}

/*
 * Free up allocated resources for a device.
 */

static void __devexit vml_pci_remove(struct pci_dev *dev)
{
	struct fb_info *info;
	struct vml_info *vinfo;
	struct vml_par *par;

	info = pci_get_drvdata(dev);
	if (info) {
		vinfo = container_of(info, struct vml_info, info);
		par = vinfo->par;
		mutex_lock(&vml_mutex);
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		vmlfb_free_vram(vinfo);
		vmlfb_disable_mmio(par);
		vmlfb_release_devices(par);
		kfree(vinfo);
		kfree(par);
		mutex_unlock(&vml_mutex);
	}
}

static void vmlfb_set_pref_pixel_format(struct fb_var_screeninfo *var)
{
	switch (var->bits_per_pixel) {
	case 16:
		var->blue.offset = 0;
		var->blue.length = 5;
		var->green.offset = 5;
		var->green.length = 5;
		var->red.offset = 10;
		var->red.length = 5;
		var->transp.offset = 15;
		var->transp.length = 1;
		break;
	case 32:
		var->blue.offset = 0;
		var->blue.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->red.offset = 16;
		var->red.length = 8;
		var->transp.offset = 24;
		var->transp.length = 0;
		break;
	default:
		break;
	}

	var->blue.msb_right = var->green.msb_right =
	    var->red.msb_right = var->transp.msb_right = 0;
}

/*
 * Device initialization.
 * We initialize one vml_par struct per device and one vml_info
 * struct per pipe. Currently we have only one pipe.
 */

static int __devinit vml_pci_probe(struct pci_dev *dev,
				   const struct pci_device_id *id)
{
	struct vml_info *vinfo;
	struct fb_info *info;
	struct vml_par *par;
	int err = 0;

	par = kzalloc(sizeof(*par), GFP_KERNEL);
	if (par == NULL)
		return -ENOMEM;

	vinfo = kzalloc(sizeof(*vinfo), GFP_KERNEL);
	if (vinfo == NULL) {
		err = -ENOMEM;
		goto out_err_0;
	}

	vinfo->par = par;
	par->vdc = dev;
	atomic_set(&par->refcount, 1);

	switch (id->device) {
	case VML_DEVICE_VDC:
		if ((err = vmlfb_get_gpu(par)))
			goto out_err_1;
		pci_set_drvdata(dev, &vinfo->info);
		break;
	default:
		err = -ENODEV;
		goto out_err_1;
		break;
	}

	info = &vinfo->info;
	info->flags = FBINFO_DEFAULT | FBINFO_PARTIAL_PAN_OK;

	err = vmlfb_enable_mmio(par);
	if (err)
		goto out_err_2;

	err = vmlfb_alloc_vram(vinfo, vml_mem_requested,
			       vml_mem_contig, vml_mem_min);
	if (err)
		goto out_err_3;

	strcpy(info->fix.id, "Vermilion Range");
	info->fix.mmio_start = 0;
	info->fix.mmio_len = 0;
	info->fix.smem_start = vinfo->vram_start;
	info->fix.smem_len = vinfo->vram_contig_size;
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.visual = FB_VISUAL_TRUECOLOR;
	info->fix.ypanstep = 1;
	info->fix.xpanstep = 1;
	info->fix.ywrapstep = 0;
	info->fix.accel = FB_ACCEL_NONE;
	info->screen_base = vinfo->vram_logical;
	info->pseudo_palette = vinfo->pseudo_palette;
	info->par = par;
	info->fbops = &vmlfb_ops;
	info->device = &dev->dev;

	INIT_LIST_HEAD(&vinfo->head);
	vinfo->pipe_disabled = 1;
	vinfo->cur_blank_mode = FB_BLANK_UNBLANK;

	info->var.grayscale = 0;
	info->var.bits_per_pixel = 16;
	vmlfb_set_pref_pixel_format(&info->var);

	if (!fb_find_mode
	    (&info->var, info, vml_default_mode, NULL, 0, &defaultmode, 16)) {
		printk(KERN_ERR MODULE_NAME ": Could not find initial mode\n");
	}

	if (fb_alloc_cmap(&info->cmap, 256, 1) < 0) {
		err = -ENOMEM;
		goto out_err_4;
	}

	err = register_framebuffer(info);
	if (err) {
		printk(KERN_ERR MODULE_NAME ": Register framebuffer error.\n");
		goto out_err_5;
	}

	printk("Initialized vmlfb\n");

	return 0;

out_err_5:
	fb_dealloc_cmap(&info->cmap);
out_err_4:
	vmlfb_free_vram(vinfo);
out_err_3:
	vmlfb_disable_mmio(par);
out_err_2:
	vmlfb_release_devices(par);
out_err_1:
	kfree(vinfo);
out_err_0:
	kfree(par);
	return err;
}

static int vmlfb_open(struct fb_info *info, int user)
{
	/*
	 * Save registers here?
	 */
	return 0;
}

static int vmlfb_release(struct fb_info *info, int user)
{
	/*
	 * Restore registers here.
	 */

	return 0;
}

static int vml_nearest_clock(int clock)
{

	int i;
	int cur_index;
	int cur_diff;
	int diff;

	cur_index = 0;
	cur_diff = clock - vml_clocks[0];
	cur_diff = (cur_diff < 0) ? -cur_diff : cur_diff;
	for (i = 1; i < vml_num_clocks; ++i) {
		diff = clock - vml_clocks[i];
		diff = (diff < 0) ? -diff : diff;
		if (diff < cur_diff) {
			cur_index = i;
			cur_diff = diff;
		}
	}
	return vml_clocks[cur_index];
}

static int vmlfb_check_var_locked(struct fb_var_screeninfo *var,
				  struct vml_info *vinfo)
{
	u32 pitch;
	u64 mem;
	int nearest_clock;
	int clock;
	int clock_diff;
	struct fb_var_screeninfo v;

	v = *var;
	clock = PICOS2KHZ(var->pixclock);

	if (subsys && subsys->nearest_clock) {
		nearest_clock = subsys->nearest_clock(subsys, clock);
	} else {
		nearest_clock = vml_nearest_clock(clock);
	}

	/*
	 * Accept a 20% diff.
	 */

	clock_diff = nearest_clock - clock;
	clock_diff = (clock_diff < 0) ? -clock_diff : clock_diff;
	if (clock_diff > clock / 5) {
#if 0
		printk(KERN_DEBUG MODULE_NAME ": Diff failure. %d %d\n",clock_diff,clock);
#endif
		return -EINVAL;
	}

	v.pixclock = KHZ2PICOS(nearest_clock);

	if (var->xres > VML_MAX_XRES || var->yres > VML_MAX_YRES) {
		printk(KERN_DEBUG MODULE_NAME ": Resolution failure.\n");
		return -EINVAL;
	}
	if (var->xres_virtual > VML_MAX_XRES_VIRTUAL) {
		printk(KERN_DEBUG MODULE_NAME
		       ": Virtual resolution failure.\n");
		return -EINVAL;
	}
	switch (v.bits_per_pixel) {
	case 0 ... 16:
		v.bits_per_pixel = 16;
		break;
	case 17 ... 32:
		v.bits_per_pixel = 32;
		break;
	default:
		printk(KERN_DEBUG MODULE_NAME ": Invalid bpp: %d.\n",
		       var->bits_per_pixel);
		return -EINVAL;
	}

	pitch = ALIGN((var->xres * var->bits_per_pixel) >> 3, 0x40);
	mem = pitch * var->yres_virtual;
	if (mem > vinfo->vram_contig_size) {
		return -ENOMEM;
	}

	switch (v.bits_per_pixel) {
	case 16:
		if (var->blue.offset != 0 ||
		    var->blue.length != 5 ||
		    var->green.offset != 5 ||
		    var->green.length != 5 ||
		    var->red.offset != 10 ||
		    var->red.length != 5 ||
		    var->transp.offset != 15 || var->transp.length != 1) {
			vmlfb_set_pref_pixel_format(&v);
		}
		break;
	case 32:
		if (var->blue.offset != 0 ||
		    var->blue.length != 8 ||
		    var->green.offset != 8 ||
		    var->green.length != 8 ||
		    var->red.offset != 16 ||
		    var->red.length != 8 ||
		    (var->transp.length != 0 && var->transp.length != 8) ||
		    (var->transp.length == 8 && var->transp.offset != 24)) {
			vmlfb_set_pref_pixel_format(&v);
		}
		break;
	default:
		return -EINVAL;
	}

	*var = v;

	return 0;
}

static int vmlfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct vml_info *vinfo = container_of(info, struct vml_info, info);
	int ret;

	mutex_lock(&vml_mutex);
	ret = vmlfb_check_var_locked(var, vinfo);
	mutex_unlock(&vml_mutex);

	return ret;
}

static void vml_wait_vblank(struct vml_info *vinfo)
{
	/* Wait for vblank. For now, just wait for a 50Hz cycle (20ms)) */
	mdelay(20);
}

static void vmlfb_disable_pipe(struct vml_info *vinfo)
{
	struct vml_par *par = vinfo->par;

	/* Disable the MDVO pad */
	VML_WRITE32(par, VML_RCOMPSTAT, 0);
	while (!(VML_READ32(par, VML_RCOMPSTAT) & VML_MDVO_VDC_I_RCOMP)) ;

	/* Disable display planes */
	VML_WRITE32(par, VML_DSPCCNTR,
		    VML_READ32(par, VML_DSPCCNTR) & ~VML_GFX_ENABLE);
	(void)VML_READ32(par, VML_DSPCCNTR);
	/* Wait for vblank for the disable to take effect */
	vml_wait_vblank(vinfo);

	/* Next, disable display pipes */
	VML_WRITE32(par, VML_PIPEACONF, 0);
	(void)VML_READ32(par, VML_PIPEACONF);

	vinfo->pipe_disabled = 1;
}

#ifdef VERMILION_DEBUG
static void vml_dump_regs(struct vml_info *vinfo)
{
	struct vml_par *par = vinfo->par;

	printk(KERN_DEBUG MODULE_NAME ": Modesetting register dump:\n");
	printk(KERN_DEBUG MODULE_NAME ": \tHTOTAL_A         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_HTOTAL_A));
	printk(KERN_DEBUG MODULE_NAME ": \tHBLANK_A         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_HBLANK_A));
	printk(KERN_DEBUG MODULE_NAME ": \tHSYNC_A          : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_HSYNC_A));
	printk(KERN_DEBUG MODULE_NAME ": \tVTOTAL_A         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_VTOTAL_A));
	printk(KERN_DEBUG MODULE_NAME ": \tVBLANK_A         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_VBLANK_A));
	printk(KERN_DEBUG MODULE_NAME ": \tVSYNC_A          : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_VSYNC_A));
	printk(KERN_DEBUG MODULE_NAME ": \tDSPCSTRIDE       : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_DSPCSTRIDE));
	printk(KERN_DEBUG MODULE_NAME ": \tDSPCSIZE         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_DSPCSIZE));
	printk(KERN_DEBUG MODULE_NAME ": \tDSPCPOS          : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_DSPCPOS));
	printk(KERN_DEBUG MODULE_NAME ": \tDSPARB           : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_DSPARB));
	printk(KERN_DEBUG MODULE_NAME ": \tDSPCADDR         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_DSPCADDR));
	printk(KERN_DEBUG MODULE_NAME ": \tBCLRPAT_A        : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_BCLRPAT_A));
	printk(KERN_DEBUG MODULE_NAME ": \tCANVSCLR_A       : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_CANVSCLR_A));
	printk(KERN_DEBUG MODULE_NAME ": \tPIPEASRC         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_PIPEASRC));
	printk(KERN_DEBUG MODULE_NAME ": \tPIPEACONF        : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_PIPEACONF));
	printk(KERN_DEBUG MODULE_NAME ": \tDSPCCNTR         : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_DSPCCNTR));
	printk(KERN_DEBUG MODULE_NAME ": \tRCOMPSTAT        : 0x%08x\n",
	       (unsigned)VML_READ32(par, VML_RCOMPSTAT));
	printk(KERN_DEBUG MODULE_NAME ": End of modesetting register dump.\n");
}
#endif

static int vmlfb_set_par_locked(struct vml_info *vinfo)
{
	struct vml_par *par = vinfo->par;
	struct fb_info *info = &vinfo->info;
	struct fb_var_screeninfo *var = &info->var;
	u32 htotal, hactive, hblank_start, hblank_end, hsync_start, hsync_end;
	u32 vtotal, vactive, vblank_start, vblank_end, vsync_start, vsync_end;
	u32 dspcntr;
	int clock;

	vinfo->bytes_per_pixel = var->bits_per_pixel >> 3;
	vinfo->stride = ALIGN(var->xres_virtual * vinfo->bytes_per_pixel, 0x40);
	info->fix.line_length = vinfo->stride;

	if (!subsys)
		return 0;

	htotal =
	    var->xres + var->right_margin + var->hsync_len + var->left_margin;
	hactive = var->xres;
	hblank_start = var->xres;
	hblank_end = htotal;
	hsync_start = hactive + var->right_margin;
	hsync_end = hsync_start + var->hsync_len;

	vtotal =
	    var->yres + var->lower_margin + var->vsync_len + var->upper_margin;
	vactive = var->yres;
	vblank_start = var->yres;
	vblank_end = vtotal;
	vsync_start = vactive + var->lower_margin;
	vsync_end = vsync_start + var->vsync_len;

	dspcntr = VML_GFX_ENABLE | VML_GFX_GAMMABYPASS;
	clock = PICOS2KHZ(var->pixclock);

	if (subsys->nearest_clock) {
		clock = subsys->nearest_clock(subsys, clock);
	} else {
		clock = vml_nearest_clock(clock);
	}
	printk(KERN_DEBUG MODULE_NAME
	       ": Set mode Hfreq : %d kHz, Vfreq : %d Hz.\n", clock / htotal,
	       ((clock / htotal) * 1000) / vtotal);

	switch (var->bits_per_pixel) {
	case 16:
		dspcntr |= VML_GFX_ARGB1555;
		break;
	case 32:
		if (var->transp.length == 8)
			dspcntr |= VML_GFX_ARGB8888 | VML_GFX_ALPHAMULT;
		else
			dspcntr |= VML_GFX_RGB0888;
		break;
	default:
		return -EINVAL;
	}

	vmlfb_disable_pipe(vinfo);
	mb();

	if (subsys->set_clock)
		subsys->set_clock(subsys, clock);
	else
		return -EINVAL;

	VML_WRITE32(par, VML_HTOTAL_A, ((htotal - 1) << 16) | (hactive - 1));
	VML_WRITE32(par, VML_HBLANK_A,
		    ((hblank_end - 1) << 16) | (hblank_start - 1));
	VML_WRITE32(par, VML_HSYNC_A,
		    ((hsync_end - 1) << 16) | (hsync_start - 1));
	VML_WRITE32(par, VML_VTOTAL_A, ((vtotal - 1) << 16) | (vactive - 1));
	VML_WRITE32(par, VML_VBLANK_A,
		    ((vblank_end - 1) << 16) | (vblank_start - 1));
	VML_WRITE32(par, VML_VSYNC_A,
		    ((vsync_end - 1) << 16) | (vsync_start - 1));
	VML_WRITE32(par, VML_DSPCSTRIDE, vinfo->stride);
	VML_WRITE32(par, VML_DSPCSIZE,
		    ((var->yres - 1) << 16) | (var->xres - 1));
	VML_WRITE32(par, VML_DSPCPOS, 0x00000000);
	VML_WRITE32(par, VML_DSPARB, VML_FIFO_DEFAULT);
	VML_WRITE32(par, VML_BCLRPAT_A, 0x00000000);
	VML_WRITE32(par, VML_CANVSCLR_A, 0x00000000);
	VML_WRITE32(par, VML_PIPEASRC,
		    ((var->xres - 1) << 16) | (var->yres - 1));

	wmb();
	VML_WRITE32(par, VML_PIPEACONF, VML_PIPE_ENABLE);
	wmb();
	VML_WRITE32(par, VML_DSPCCNTR, dspcntr);
	wmb();
	VML_WRITE32(par, VML_DSPCADDR, (u32) vinfo->vram_start +
		    var->yoffset * vinfo->stride +
		    var->xoffset * vinfo->bytes_per_pixel);

	VML_WRITE32(par, VML_RCOMPSTAT, VML_MDVO_PAD_ENABLE);

	while (!(VML_READ32(par, VML_RCOMPSTAT) &
		 (VML_MDVO_VDC_I_RCOMP | VML_MDVO_PAD_ENABLE))) ;

	vinfo->pipe_disabled = 0;
#ifdef VERMILION_DEBUG
	vml_dump_regs(vinfo);
#endif

	return 0;
}

static int vmlfb_set_par(struct fb_info *info)
{
	struct vml_info *vinfo = container_of(info, struct vml_info, info);
	int ret;

	mutex_lock(&vml_mutex);
	list_del(&vinfo->head);
	list_add(&vinfo->head, (subsys) ? &global_has_mode : &global_no_mode);
	ret = vmlfb_set_par_locked(vinfo);

	mutex_unlock(&vml_mutex);
	return ret;
}

static int vmlfb_blank_locked(struct vml_info *vinfo)
{
	struct vml_par *par = vinfo->par;
	u32 cur = VML_READ32(par, VML_PIPEACONF);

	switch (vinfo->cur_blank_mode) {
	case FB_BLANK_UNBLANK:
		if (vinfo->pipe_disabled) {
			vmlfb_set_par_locked(vinfo);
		}
		VML_WRITE32(par, VML_PIPEACONF, cur & ~VML_PIPE_FORCE_BORDER);
		(void)VML_READ32(par, VML_PIPEACONF);
		break;
	case FB_BLANK_NORMAL:
		if (vinfo->pipe_disabled) {
			vmlfb_set_par_locked(vinfo);
		}
		VML_WRITE32(par, VML_PIPEACONF, cur | VML_PIPE_FORCE_BORDER);
		(void)VML_READ32(par, VML_PIPEACONF);
		break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
		if (!vinfo->pipe_disabled) {
			vmlfb_disable_pipe(vinfo);
		}
		break;
	case FB_BLANK_POWERDOWN:
		if (!vinfo->pipe_disabled) {
			vmlfb_disable_pipe(vinfo);
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int vmlfb_blank(int blank_mode, struct fb_info *info)
{
	struct vml_info *vinfo = container_of(info, struct vml_info, info);
	int ret;

	mutex_lock(&vml_mutex);
	vinfo->cur_blank_mode = blank_mode;
	ret = vmlfb_blank_locked(vinfo);
	mutex_unlock(&vml_mutex);
	return ret;
}

static int vmlfb_pan_display(struct fb_var_screeninfo *var,
			     struct fb_info *info)
{
	struct vml_info *vinfo = 