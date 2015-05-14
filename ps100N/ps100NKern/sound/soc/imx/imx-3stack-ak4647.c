v2659_regs.size_320x240_tbl_size);
	}
#endif

	return ret;
}

/* At present only support change to 15fps(only for SVGA mode) */
//OK
static int ov2659_set_fps(struct sensor *s, int fps)
{
	int ret = 0;
#if 0 //we will set this later,zengshu,2012-7-4
	if (i2c_smbus_write_byte_data(s->i2c_client, 0xff, 0x01) < 0) {
		pr_err("in %s,change to sensor addr failed\n", __func__);
		ret = -EPERM;
	}

	/* change the camera framerate to 15fps(only for SVGA mode) */
	if (i2c_smbus_write_byte_data(s->i2c_client, 0x11, 0x01) < 0) {
		pr_err("change camera to 15fps failed\n");
		ret = -EPERM;
	}
#endif

	return ret;
}

//OK
static int ov2659_set_format(struct sensor *s, int format)
{
	int ret = 0;

	if (format == V4L2_PIX_FMT_RGB565) {
		/* set RGB565 format */
		if (ov2659_i2c_write(0x4300, 0x60) < 0)
			ret = -EPERM;

	} else if (format == V4L2_PIX_FMT_YUV420) {
		/* set YUV420 format */
		if (ov2659_i2c_write(0x4300, 0x40) < 0)
			ret = -EPERM;
	} else {
		pr_alert("format not supported\n");
	}

	return ret;
}

/* --------------- IOCTL functions from v4l2_int_ioctl_desc --------------- */

/*!
 * ioctl_g_ifparm - V4L2 sensor interface handler for vidioc_int_g_ifparm_num
 * s: pointer to standard V4L2 device structure
 * p: pointer to standard V4L2 vidioc_int_g_ifparm_num ioctl structure
 *
 * Gets slave interface parameters.
 * Calculates the required xclk value to support the requested
 * clock parameters in p.  This value is returned in the p
 * parameter.
 *
 * vidioc_int_g_ifparm returns platform-specific information about the
 * interface settings used by the sensor.
 *
 * Given the image capture format in pix, the nominal frame period in
 * timeperframe, calculate the required xclk frequency.
 *
 * Called on open.
 */
 //OK
static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
	pr_alert("In ov2659:ioctl_g_ifparm\n");

	if (s == NULL) {
		pr_err("   ERROR!! no slave device set!\n");
		return -1;
	}

	memset(p, 0, sizeof(*p));
	p->u.bt656.clock_curr = ov2659_data.mclk;
	p->if_type = V4L2_IF_TYPE_BT656;
	p->u.bt656.mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT;
	p->u.bt656.clock_min = ov2659_XCLK_MIN;
	p->u.bt656.clock_max = ov2659_XCLK_MAX;

	return 0;
}

/*!
 * Sets the camera power.
 *
 * s  pointer to the camera device
 * on if 1, power is to be turned on.  0 means power is to be turned off
 *
 * ioctl_s_power - V4L2 sensor interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 * This is called on open, close, suspend and resume.
 */
 //OK
static int ioctl_s_power(struct v4l2_int_device *s, int on)
{
	struct sensor *sensor = s->priv;

	pr_alert("In ov2659:ioctl_s_power\n");

	if (on && !sensor->on) {
		if (io_regulator)
			if (regulator_enable(io_regulator) != 0)
				return -EIO;
		if (core_regulator)
			if (regulator_enable(core_regulator) != 0)
				return -EIO;
		if (gpo_regulator)
			if (regulator_enable(gpo_regulator) != 0)
				return -EIO;
		if (analog_regulator)
			if (regulator_enable(analog_regulator) != 0)
				return -EIO;
	} else if (!on && sensor->on) {
		if (analog_regulator)
			regulator_disable(analog_regulator);
		if (core_regulator)
			regulator_disable(core_regulator);
		if (io_regulator)
			regulator_disable(io_regulator);
		if (gpo_regulator)
			regulator_disable(gpo_regulator);
	}

	sensor->on = on;

	return 0;
}

/*!
 * ioctl_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
 //OK
static int ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;
	int ret = 0;

	pr_alert("In ov2659:ioctl_g_parm\n");

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pr_alert("   type is V4L2_BUF_TYPE_VIDEO_CAPTURE\n");
		memset(a, 0, sizeof(*a));
		a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cparm->capability = sensor->streamcap.capability;
		cparm->timeperframe = sensor->streamcap.timeperframe;
		cparm->capturemode = sensor->streamcap.capturemode;
		ret = 0;
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_err("   type is not V4L2_BUF_TYPE_VIDEO_CAPTURE " \
			"but %d\n", a->type);
		ret = -EINVAL;
		break;

	default:
		pr_err("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ioctl_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;	/* target frames per secound */
	int ret = 0;

	pr_alert("In ov2659:ioctl_s_parm\n");

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pr_alert("   type is V4L2_BUF_TYPE_VIDEO_CAPTURE\n");

		/* Check that the new frame rate is allowed. */
		if ((timeperframe->numerator == 0)
		    || (timeperframe->denominator == 0)) {
			timeperframe->denominator = DEFAULT_FPS;
			timeperframe->numerator = 1;
		}
		tgt_fps = timeperframe->denominator
			  / timeperframe->numerator;

		if (tgt_fps > MAX_FPS) {
			timeperframe->denominator = MAX_FPS;
			timeperframe->numerator = 1;
		} else if (tgt_fps < MIN_FPS) {
			timeperframe->denominator = MIN_FPS;
			timeperframe->numerator = 1;
		}
		sensor->streamcap.timeperframe = *timeperframe;
		sensor->streamcap.capturemode =
				(u32)a->parm.capture.capturemode;

		ret = ov2659_init_mode(sensor);
		if (tgt_fps == 15)
			ov2659_set_fps(sensor, tgt_fps);
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_err("   type is not V4L2_BUF_TYPE_VIDEO_CAPTURE " \
			"but %d\n", a->type);
		ret = -EINVAL;
		break;

	default:
		pr_err("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ioctl_s_fmt_cap - V4L2 sensor interface handler for ioctl_s_fmt_cap
 * 		     set camera output format and resolution format
 *
 * @s: pointer to standard V4L2 device structure
 * @arg: pointer to parameter, according this to set camera
 *
 * Returns 0 if set succeed, else return -1
 */
 //OK
static int ioctl_s_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct sensor *sensor = s->priv;
	u32 format = f->fmt.pix.pixelformat;
	int size = 0, ret = 0;

	size = f->fmt.pix.width * f->fmt.pix.height;
	switch (format) {
	case V4L2_PIX_FMT_RGB565:
		if (size > 640 * 480)
			sensor->streamcap.capturemode = V4L2_MODE_HIGHQUALITY;
		else
			sensor->streamcap.capturemode = 0;
		ret = ov2659_init_mode(sensor);

		ret = ov2659_set_format(sensor, V4L2_PIX_FMT_RGB565);
		break;
	case V4L2_PIX_FMT_UYVY:
		if (size > 640 * 480)
			sensor->streamcap.capturemode = V4L2_MODE_HIGHQUALITY;
		else
			sensor->streamcap.capturemode = 0;
		ret = ov2659_init_mode(sensor);
		break;
	case V4L2_PIX_FMT_YUV420:
		if (size > 640 * 480)
			sensor->streamcap.capturemode = V4L2_MODE_HIGHQUALITY;
		else
			sensor->streamcap.capturemode = 0;
		ret = ov2659_init_mode(sensor);

		/* YUYV: width * 2, YY: width */
		ret = ov2659_set_format(sensor, V4L2_PIX_FMT_YUV420);
		break;
	default:
		pr_alert("case not supported\n");
		break;
	}

	return ret;
}

/*!
 * ioctl_g_fmt_cap - V4L2 sensor interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the sensor's current pixel format in the v4l2_format
 * parameter.
 */
 //OK
static int ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct sensor *sensor = s->priv;

	pr_alert("In ov2659:ioctl_g_fmt_cap.\n");

	f->fmt.pix = sensor->pix;

	return 0;
}

/*!
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the video_control[] array.  Otherwise, returns -EINVAL
 * if the control is not supported.
 */
 //OK
static int ioctl_g_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int ret = 0;

	pr_alert("In ov2659:ioctl_g_ctrl\n");

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		vc->value = ov2659_data.brightness;
		break;
	case V4L2_CID_HUE:
		vc->value = ov2659_data.hue;
		break;
	case V4L2_CID_CONTRAST:
		vc->value = ov2659_data.contrast;
		break;
	case V4L2_CID_SATURATION:
		vc->value = ov2659_data.saturation;
		break;
	case V4L2_CID_RED_BALANCE:
		vc->value = ov2659_data.red;
		break;
	case V4L2_CID_BLUE_BALANCE:
		vc->value = ov2659_data.blue;
		break;
	case V4L2_CID_EXPOSURE:
		vc->value = ov2659_data.ae_mode;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/*!
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
 //OK
static int ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int retval = 0;

	pr_alert("In ov2659:ioctl_s_ctrl %d\n", vc->id);

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		pr_alert("   V4L2_CID_BRIGHTNESS\n");
		break;
	case V4L2_CID_CONTRAST:
		pr_alert("   V4L2_CID_CONTRAST\n");
		break;
	case V4L2_CID_SATURATION:
		pr_alert("   V4L2_CID_SATURATION\n");
		break;
	case V4L2_CID_HUE:
		pr_alert("   V4L2_CID_HUE\n");
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		pr_alert(
			"   V4L2_CID_AUTO_WHITE_BALANCE\n");
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		pr_alert(
			"   V4L2_CID_DO_WHITE_BALANCE\n");
		break;
	case V4L2_CID_RED_BALANCE:
		pr_alert("   V4L2_CID_RED_BALANCE\n");
		break;
	case V4L2_CID_BLUE_BALANCE:
		pr_alert("   V4L2_CID_BLUE_BALANCE\n");
		break;
	case V4L2_CID_GAMMA:
		pr_alert("   V4L2_CID_GAMMA\n");
		break;
	case V4L2_CID_EXPOSURE:
		pr_alert("   V4L2_CID_EXPOSURE\n");
		break;
	case V4L2_CID_AUTOGAIN:
		pr_alert("   V4L2_CID_AUTOGAIN\n");
		break;
	case V4L2_CID_GAIN:
		pr_alert("   V4L2_CID_GAIN\n");
		break;
	case V4L2_CID_HFLIP:
		pr_alert("   V4L2_CID_HFLIP\n");
		break;
	case V4L2_CID_VFLIP:
		pr_alert("   V4L2_CID_VFLIP\n");
		break;
	default:
		pr_alert("   Default case\n");
		retval = -EPERM;
		break;
	}

	return retval;
}

/*!
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 */
 //OK
static int ioctl_init(struct v4l2_int_device *s)
{
	pr_alert("In ov2659:ioctl_init\n");

	return 0;
}

/*!
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.
 */
 //OK
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	