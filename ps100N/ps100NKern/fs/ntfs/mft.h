mxc_streamon(cam_data *cam)
{
	struct mxc_v4l_frame *frame;
	int err = 0;

	pr_debug("In MVC:mxc_streamon\n");

	if (NULL == cam) {
		pr_err("ERROR! cam parameter is NULL\n");
		return -1;
	}

	if (list_empty(&cam->ready_q)) {
		pr_err("ERROR: v4l2 capture: mxc_streamon buffer has not been "
			"queued yet\n");
		return -EINVAL;
	}

	cam->capture_pid = current->pid;

	if (cam->enc_enable) {
		err = cam->enc_enable(cam);
		if (err != 0) {
			return err;
		}
	}

	cam->ping_pong_csi = 0;
	if (cam->enc_update_eba) {
		frame =
		    list_entry(cam->ready_q.next, struct mxc_v4l_frame, queue);
		list_del(cam->ready_q.next);
		list_add_tail(&frame->queue, &cam->working_q);
		err = cam->enc_update_eba(frame->buffer.m.offset,
					  &cam->ping_pong_csi);

		frame =
		    list_entry(cam->ready_q.next, struct mxc_v4l_frame, queue);
		list_del(cam->ready_q.next);
		list_add_tail(&frame->queue, &cam->working_q);
		err |= cam->enc_update_eba(frame->buffer.m.offset,
					   &cam->ping_pong_csi);
	} else {
		return -EINVAL;
	}

	cam->capture_on = true;

	return err;
}

/*!
 * Shut down the encoder job
 *
 * @param cam      structure cam_data *
 *
 * @return status  0 Success
 */
static int mxc_streamoff(cam_data *cam)
{
	int err = 0;

	pr_debug("In MVC:mxc_streamoff\n");

	if (cam->capture_on == false)
		return 0;

	if (cam->enc_disable) {
		err = cam->enc_disable(cam);
	}
	mxc_free_frames(cam);
	mxc_capture_inputs[cam->current_input].status |= V4L2_IN_ST_NO_POWER;
	cam->capture_on = false;
	return err;
}

/*!
 * Valid and adjust the overlay window size, position
 *
 * @param cam      structure cam_data *
 * @param win      struct v4l2_window  *
 *
 * @return 0
 */
static int verify_preview(cam_data *cam, struct v4l2_window *win)
{
	int i = 0, width_bound = 0, height_bound = 0;
	int *width, *height;
	struct fb_info *bg_fbi = NULL;
	bool foregound_fb;

	pr_debug("In MVC: verify_preview\n");

	do {
		cam->overlay_fb = (struct fb_info *)registered_fb[i];
		if (cam->overlay_fb == NULL) {
			pr_err("ERROR: verify_preview frame buffer NULL.\n");
			return -1;
		}
		if (strcmp(cam->overlay_fb->fix.id, "DISP3 BG") == 0)
			bg_fbi = cam->overlay_fb;
		if (strcmp(cam->overlay_fb->fix.id,
			    mxc_capture_outputs[cam->output].name) == 0) {
			if (strcmp(cam->overlay_fb->fix.id, "DISP3 FG") == 0)
				foregound_fb = true;
			break;
		}
	} while (++i < FB_MAX);

	if (foregound_fb) {
		width_bound = bg_fbi->var.xres;
		height_bound = bg_fbi->var.yres;

		if (win->w.width + win->w.left > bg_fbi->var.xres ||
		    win->w.height + win->w.top > bg_fbi->var.yres) {
			pr_err("ERROR: FG window position exceeds.\n");
			return -1;
		}
	} else {
		/* 4 bytes alignment for BG */
		width_bound = cam->overlay_fb->var.xres;
		height_bound = cam->overlay_fb->var.yres;

		if (cam->overlay_fb->var.bits_per_pixel == 24) {
			win->w.left -= win->w.left % 4;
		} else if (cam->overlay_fb->var.bits_per_pixel == 16) {
			win->w.left -= win->w.left % 2;
		}

		if (win->w.width + win->w.left > cam->overlay_fb->var.xres)
			win->w.width = cam->overlay_fb->var.xres - win->w.left;
		if (win->w.height + win->w.top > cam->overlay_fb->var.yres)
			win->w.height = cam->overlay_fb->var.yres - win->w.top;
	}

	/* stride line limitation */
	win->w.height -= win->w.height % 8;
	win->w.width -= win->w.width % 8;

	if (cam->rotation >= IPU_ROTATE_90_RIGHT) {
		height = &win->w.width;
		width = &win->w.height;
	} else {
		width = &win->w.width;
		height = &win->w.height;
	}

	if ((cam->crop_bounds.width / *width > 8) ||
	    ((cam->crop_bounds.width / *width == 8) &&
	     (cam->crop_bounds.width % *width))) {
		*width = cam->crop_bounds.width / 8;
		if (*width % 8)
			*width += 8 - *width % 8;
		if (*width + win->w.left > width_bound) {
			pr_err("ERROR: v4l2 capture: width exceeds "
				"resize limit.\n");
			return -1;
		}
		pr_err("ERROR: v4l2 capture: width exceeds limit. "
			"Resize to %d.\n",
			*width);
	}

	if ((cam->crop_bounds.height / *height > 8) ||
	    ((cam->crop_bounds.height / *height == 8) &&
	     (cam->crop_bounds.height % *height))) {
		*heig