		struct fb_info *fb = cam->overlay_fb;
			if (!fb)
				return -1;
			if (g_vaddr_fb)
				iounmap(g_vaddr_fb);
			g_vaddr_fb = ioremap_cached(fb->fix.smem_start,
						    fb->fix.smem_len);
			if (!g_vaddr_fb)
				return -1;
		}
	} else if (cam->v4l2_fb.flags == V4L2_FBUF_FLAG_OVERLAY) {
		cfg->ch1_ptr = (unsigned int)cam->vf_bufs[0];
		cfg->ch1_ptr2 = (unsigned int)cam->vf_bufs[1];
		cfg->ch1_stride = cam->win.w.width;
	} else {
		struct fb_info *fb = cam->overlay_fb;

		if (!fb)
			return -1;

		cfg->ch1_ptr = fb->fix.smem_start;
		cfg->ch1_ptr += cam->win.w.top * fb->var.xres_virtual
		    * (fb->var.bits_per_pixel >> 3)
		    + cam->win.w.left * (fb->var.bits_per_pixel >> 3);
		cfg->ch1_ptr2 = cfg->ch1_ptr;
		cfg->ch1_stride = fb->var.xres_virtual;
	}

	return 0;
}

/*!
 * @brief Setup PrP configuration parameters.
 * @param cfg	Pointer to emma_prp_cfg structure
 * @param cam	Pointer to cam_data structure
 * @return	Zero on success, others on failure
 */
static int prp_v4l2_cfg(emma_prp_cfg * cfg, cam_data * cam)
{
	cfg->in_pix = PRP_PIXIN_YUYV;
	cfg->in_width = cam->crop_current.width;
	cfg->in_height = cam->crop_current.height;
	cfg->in_line_stride = cam->crop_current.left;
	cfg->in_line_skip = cam->crop_current.top;
	cfg->in_ptr = 0;
	cfg->in_csi = PRP_CSI_LOOP;
	memset(cfg->in_csc, 0, sizeof(cfg->in_csc));

	if (cam->overlay_on) {
		/* Convert V4L2 pixel format to PrP pixel format */
		switch (cam->v4l2_fb.fmt.pixelformat) {
		case V4L2_PIX_FMT_RGB332:
			cfg->ch1_pix = PRP_PIX1_RGB332;
			break;
		case V4L2_PIX_FMT_RGB32:
		case V4L2_PIX_FMT_BGR32:
			cfg->ch1_pix = PRP_PIX1_RGB888;
			break;
		case V4L2_PIX_FMT_YUYV:
			cfg->ch1_pix = PRP_PIX1_YUYV;
			break;
		case V4L2_PIX_FMT_UYVY:
			cfg->ch1_pix = PRP_PIX1_UYVY;
			break;
		case V4L2_PIX_FMT_RGB565:
		default:
			cfg->ch1_pix = PRP_PIX1_RGB565;
			break;
		}
		if ((cam->rotation == V4L2_MXC_ROTATE_90_RIGHT)
		    || (cam->rotation == V4L2_MXC_ROTATE_90_RIGHT_VFLIP)
		    || (cam->rotation == V4L2_MXC_ROTATE_90_RIGHT_HFLIP)
		    || (cam->rotation == V4L2_MXC_ROTATE_90_LEFT)) {
			cfg->ch1_width = cam->win.w.height;
			cfg->ch1_height = cam->win.w.width;
		} else {
			cfg->ch1_width = cam->win.w.width;
			cfg->ch1_height = cam->win.w.height;
		}

		if (set_ch1_addr(cfg, cam))
			return -1;
	} else {
		cfg->ch1_pix = PRP_PIX1_UNUSED;
		cfg->ch1_width = cfg->in_width;
		cfg->ch1_height = cfg->in_height;
	}
	cfg->ch1_scale.algo = 0;
	cfg->ch1_scale.width.num = cfg->in_width;
	cfg->ch1_scale.width.den = cfg->ch1_width;
	cfg->ch1_scale.height.num = cfg->in_height;
	cfg->ch1_scale.height.den = cfg->ch1_height;
	cfg->ch1_csi = PRP_CSI_EN;

	if (cam->capture_on || g_still_on) {
		switch (cam->v2f.fmt.pix.pixelformat) {
		case V4L2_PIX_FMT_YUYV:
			cfg->ch2_pix = PRP_PIX2_YUV422;
			break;
		case V4L2_PIX_FMT_YUV420:
			cfg->ch2_pix = PRP_PIX2_YUV420;
			break;
			/*
			 * YUV444 is not defined by V4L2.
			 * We support it in default case.
			 */
		default:
			cfg->ch2_pix = PRP_PIX2_YUV444;
			break;
		}
		cfg->ch2_width = cam->v2f.fmt.pix.width;
		cfg->ch2_height = cam->v2f.fmt.pix.height;
	} else {
		cfg->ch2_pix = PRP_PIX2_UNUSED;
		cfg->ch2_width = cfg->in_width;
		cfg->ch2_height = cfg->in_height;
	}
	cfg->ch2_scale.algo = 0;
	cfg->ch2_scale.width.num = cfg->in_width;
	cfg->ch2_scale.width.den = cfg->ch2_width;
	cfg->ch2_scale.height.num = cfg->in_height;
	cfg->ch2_scale.height.den = cfg->ch2_height;
	cfg->ch2_csi = PRP_CSI_EN;

	memset(cfg->scale, 0, sizeof(cfg->scale));
	cfg->scale[0].algo = cfg->ch1_scale.algo & 3;
	cfg->scale[1].algo = (cfg->ch1_scale.algo >> 2) & 3;
	cfg->scale[2].algo = cfg->ch2_scale.algo & 3;
	cfg->scale[3].algo = (cfg->ch2_scale.algo >> 2) & 3;

	prp_cfg_dump(cfg);

	if (prp_resize_check_ch2(cfg))
		return -1;

	if (prp_resize_check_ch1(cfg))
		return -1;

	return 0;
}

/*!
 * @brief PrP interrupt handler
 */
static irqreturn_t prp_isr(int irq, void *dev_id)
{
	int status;
	cam_data *cam = (cam_data *) dev_id;

	status = prphw_isr();

	if (g_still_on && (status & PRP_INTRSTAT_CH2BUF1)) {
		prp_still_stop(cam);
		cam->still_counter++;
		wake_up_interruptible(&cam->still_queue);
		/*
		 * Still & video capture use the same PrP channel 2.
		 * They are execlusive.
		 */
	} else if (cam->capture_on) {
		if (status & (PRP_INTRSTAT_CH2BUF1 | PRP_INTRSTAT_CH2BUF2)) {
			cam->enc_callback(0, cam);
		}
	}
	if (cam->overlay_on
	    && (status & (PRP_INTRSTAT_CH1BUF1 | PRP_INTRSTAT_CH1BUF2))) {
		if (cam->rotation != V4L2_MXC_ROTATE_NONE) {
			g_rotbuf = (status & PRP_INTRSTAT_CH1BUF1) ? 0 : 1;
			tasklet_schedule(&prp_vf_tasklet);
		} else if (cam->v4l2_fb.flags == V4L2_FBUF_FLAG_OVERLAY) {
			struct fb_gwinfo gwinfo;

			gwinfo.enabled = 1;
			gwinfo.alpha_value = 255;
			gwinfo.ck_enabled = 0;
			gwinfo.xpos = cam->win.w.left;
			gwinfo.ypos = cam->win.w.top;
			gwinfo.xres = cam->win.w.width;
			gwinfo.yres = cam->win.w.height;
			gwinfo.xres_virtual = cam->win.w.width;
			gwinfo.vs_reversed = 0;
			if (status & PRP_INTRSTAT_CH1BUF1)
				gwinfo.base = (unsign