return -EINVAL;

	decoder->current_std = current_std;
	decoder->route.input = route->input;
	decoder->route.output = route->output;

	v4l_dbg(1, debug, decoder->client,
			"Input set to: %d, std : %d",
			input_sel, current_std);

	return 0;
}

/**
 * ioctl_queryctrl - V4L2 decoder interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qctrl: standard V4L2 v4l2_queryctrl structure
 *
 * If the requested control is supported, returns the control information.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int
ioctl_queryctrl(struct v4l2_int_device *s, struct v4l2_queryctrl *qctrl)
{
	struct tvp514x_decoder *decoder = s->priv;
	int err = -EINVAL;

	if (qctrl == NULL)
		return err;

	switch (qctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		/* Brightness supported is (0-255),
		 */
		err = v4l2_ctrl_query_fill(qctrl, 0, 255, 1, 128);
		break;
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
		/* Saturation and Contrast supported is -
		 *	Contrast: 0 - 255 (Default - 128)
		 *	Saturation: 0 - 255 (Default - 128)
		 */
		err = v4l2_ctrl_query_fill(qctrl, 0, 255, 1, 128);
		break;
	case V4L2_CID_HUE:
		/* Hue Supported is -
		 *	Hue - -180 - +180 (Default - 0, Step - +180)
		 */
		err = v4l2_ctrl_query_fill(qctrl, -180, 180, 180, 0);
		break;
	case V4L2_CID_AUTOGAIN:
		/* Autogain is either 0 or 1*/
		memcpy(qctrl, &tvp514x_autogain_ctrl,
				sizeof(struct v4l2_queryctrl));
		err = 0;
		break;
	default:
		v4l_err(decoder->client,
			"invalid control id %d\n", qctrl->id);
		return err;
	}

	v4l_dbg(1, debug, decoder->client,
			"Query Control: %s : Min - %d, Max - %d, Def - %d",
			qctrl->name,
			qctrl->minimum,
			qctrl->maximum,
			qctrl->default_value);

	return err;
}

/**
 * ioctl_g_ctrl - V4L2 decoder interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @ctrl: pointer to v4l2_control structure
 *
 * If the requested control is supported, returns the control's current
 * value from the decoder. Otherwise, returns -EINVAL if the control is not
 * supported.
 */
static int
ioctl_g_ctrl(struct v4l2_int_device *s, struct v4l2_control *ctrl)
{
	struct tvp514x_decoder *decoder = s->priv;

	if (ctrl == NULL)
		return -EINVAL;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = decoder->tvp514x_regs[REG_BRIGHTNESS].val;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = decoder->tvp514x_regs[REG_CONTRAST].val;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = decoder->tvp514x_regs[REG_SATURATION].val;
		break;
	case V4L2_CID_HUE:
		ctrl->value = decoder->tvp514x_regs[REG_HUE].val;
		if (ctrl->value == 0x7F)
			ctrl->value = 180;
		else if (ctrl->value == 0x80)
			ctrl->value = -180;
		else
			ctrl->value = 0;

		break;
	case V4L2_CID_AUTOGAIN:
		ctrl->value = decoder->tvp514x_regs[REG_AFE_GAIN_CTRL].val;
		if ((ctrl->value & 0x3) == 3)
			ctrl->value = 1;
		else
			ctrl->value = 0;

		break;
	default:
		v4l_err(decoder->client,
			"invalid control id %d\n", ctrl->id);
		return -EINVAL;
	}

	v4l_dbg(1, debug, decoder->client,
			"Get Control: ID - %d - %d",
			ctrl->id, ctrl->value);
	return 0;
}

/**
 * ioctl_s_ctrl - V4L2 decoder interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @ctrl: pointer to v4l2_control structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW. Otherwise, returns -EINVAL if the control is not supported.
 */
static int
ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *ctrl)
{
	struct tvp514x_decoder *decoder = s->priv;
	int err = -EINVAL, value;

	if (ctrl == NULL)
		return err;

	value = (__s32) ctrl->value;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		if (ctrl->value < 0 || ctrl->value > 255) {
			v4l_err(decoder->client,
					"invalid brightness setting %d\n",
					ctrl->value);
			return -ERANGE;
		}
		err = tvp514x_write_reg(decoder->client, REG_BRIGHTNESS,
				value);
		if (err)
			return err;
		decoder->tvp514x_regs[REG_BRIGHTNESS].val = value;
		break;
	case V4L2_CID_CONTRAST:
		if (ctrl->value < 0 || ctrl->value > 255) {
			v4l_err(decoder->client,
					"invalid contrast setting %d\n",
					ctrl->value);
			return -ERANGE;
		}
		err = tvp514x_write_reg(decoder->client, REG_CONTRAST,
				value);
		if (err)
			return err;
		decoder->tvp514x_regs[REG_CONTRAST].val = value;
		break;
	case V4L2_CID_SATURATION:
		if (ctrl->value < 0 || ctrl->value > 255) {
			v4l_err(decoder->client,
					"invalid saturation setting %d\n",
					ctrl->value);
			return -ERANGE;
		}
		err = tvp514x_write_reg(decoder->client, REG_SATURATION,
				value);
		if (err)
			return err;
		decoder->tvp514x_regs[REG_SATURATION].val = value;
		break;
	case V4L2_CID_HUE:
		if (value == 180)
			value = 0x7F;
		else if (value == -180)
			value = 0x80;
		else if (value == 0)
			value = 0;
		else {
			v4l_err(decoder->client,
					"invalid hue setting %d\n",
					ctrl->value);
			return -ERANGE;
		}
		err = tvp514x_write_reg(decoder->client, REG_HUE,
				value);
		if (err)
			return err;
		decoder->tvp514x_regs[REG_HUE].val = value;
		break;
	case V4L2_CID_AUTOGAIN:
		if (value == 1)
			value = 0x0F;
		else if (value == 0)
			value = 0x0C;
		else {
			v4l_err(decoder->client,
					"invalid auto gain setting %d\n",
					ctrl->value);
			return -ERANGE;
		}
		err = tvp514x_write_reg(decoder->client, REG_AFE_GAIN_CTRL,
				value);
		if (err)
			return err;
		decoder->tvp514x_regs[REG_AFE_GAIN_CTRL].val = value;
		break;
	default:
		v4l_err(decoder->client,
			"invalid control id %d\n", ctrl->id);
		return err;
	}

	v4l_dbg(1, debug, decoder->client,
			"Set Control: ID - %d - %d",
			ctrl->id, ctrl->value);

	return err;
}

/**
 * ioctl_enum_fmt_cap - Implement the CAPTURE buffer VIDIOC_ENUM_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @fmt: standard V4L2 VIDIOC_ENUM_FMT ioctl structure
 *
 * Implement the VIDIOC_ENUM_FMT ioctl to enumerate supported formats
 */
static int
ioctl_enum_fmt_cap(struct v4l2_int_device *s, struct v4l2_fmtdesc *fmt)
{
	struct tvp514x_decoder *decoder = s->priv;
	int index;

	if (fmt == NULL)
		return -EINVAL;

	index = fmt->index;
	if ((index >= decoder->num_fmts) || (index < 0))
		return -EINVAL;	/* Index out of bound */

	if (fmt->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;	/* only capture is supported */

	memcpy(fmt, &decoder->fmt_list[index],
		sizeof(struct v4l2_fmtdesc));

	v4l_dbg(1, debug, decoder->client,
			"Current FMT: index - %d (%s)",
			decoder->fmt_list[index].index,
			decoder->fmt_list[index].description);
	return 0;
}

/**
 * ioctl_try_fmt_cap - Implement the CAPTURE buffer VIDIOC_TRY_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_TRY_FMT ioctl structure
 *
 * Implement the VIDIOC_TRY_FMT ioctl for the CAPTURE buffer type. This
 * ioctl is used to negotiate the image capture size and pixel format
 * without actually making it take effect.
 */
static int
ioctl_try_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct tvp514x_decoder *decoder = s->priv;
	int ifmt;
	struct v4l2_pix_format *pix;
	enum tvp514x_std current_std;

	if (f == NULL)
		return -EINVAL;

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		f->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	pix = &f->fmt.pix;

	/* Calculate height and width based on current standard */
	current_std = tvp514x_get_current_std(decoder);
	if (current_std == STD_INVALID)
		return -EINVAL;

	decoder->current_std = current_std;
	pix->width = decoder->std_list[current_std].width;
	pix->height = decoder->std_list[current_std].height;

	for (ifmt = 0; ifmt < decoder->num_fmts; ifmt++) {
		if (pix->pixelformat ==
			decoder->fmt_list[ifmt].pixelformat)
			break;
	}
	if (ifmt == decoder->num_fmts)
		ifmt = 0;	/* None of the format matched, select default */
	pix->pixelformat = decoder->fmt_list[ifmt].pixelformat;

	pix->field = V4L2_FIELD_INTERLACED;
	pix->bytesperline = pix->width * 2;
	pix->sizeimage = pix->bytesperline * pix->height;
	pix->colorspace = V4L2_COLORSPACE_SMPTE170M;
	pix->priv = 0;

	v4l_dbg(1, debug, decoder->client,
			"Try FMT: pixelformat - %s, bytesperline - %d"
			"Width - %d, Height - %d",
			decoder->fmt_list[ifmt].description, pix->bytesperline,
			pix->width, pix->height);
	return 0;
}

/**
 * ioctl_s_fmt_cap - V4L2 decoder interface handler for VIDIOC_S_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_S_FMT ioctl structure
 *
 * If the requested format is supported, configures the HW to use that
 * format, returns error code if format not supported or HW can't be
 * correctly configured.
 */
static int
ioctl_s_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct tvp514x_decoder *decoder = s->priv;
	struct v4l2_pix_format *pix;
	int rval;

	if (f == NULL)
		return -EINVAL;

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;	/* only capture is supported */

	pix = &f->fmt.pix;
	rval = ioctl_try_fmt_cap(s, f);
	if (rval)
		return rval;

		decoder->pix = *pix;

	return rval;
}

/**
 * ioctl_g_fmt_cap - V4L2 decoder interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the decoder's current pixel format in the v4l2_format
 * parameter.
 */
static int
ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct tvp514x_decoder *decoder = s->priv;

	if (f == NULL)
		return -EINVAL;

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;	/* only capture is supported */

	f->fmt.pix = decoder->pix;

	v4l_dbg(1, debug, decoder->client,
			"Current FMT: bytesperline - %d"
			"Width - %d, Height - %d",
			decoder->pix.bytesperline,
			decoder->pix.width, decoder->pix.height);
	return 0;
}

/**
 * ioctl_g_parm - V4L2 decoder interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the decoder's video CAPTURE parameters.
 */
static int
ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct tvp514x_decoder *decoder = s->priv;
	struct v4l2_captureparm *cparm;
	enum tvp514x_std current_std;

	if (a == NULL)
		return -EINVAL;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;	/* only capture is supported */

	memset(a, 0, sizeof(*a));
	a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* get the current standard */
	current_std = tvp514x_get_current_std(decoder);
	if (current_std == STD_INVALID)
		return -EINVAL;

	decoder->current_std = current_std;

	cparm = &a->parm.capture;
	cparm->capability = V4L2_CAP_TIMEPERFRAME;
	cparm->timeperframe =
		decoder->std_list[current_std].standard.frameperiod;

	return 0;
}

/**
 * ioctl_s_parm - V4L2 decoder interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the decoder to use the input parameters, if possible. If
 * not possible, returns the appropriate error code.
 */
static int
ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct tvp514x_decoder *decoder = s->priv;
	struct v4l2_fract *timeperframe;
	enum tvp514x_std current_std;

	if (a == NULL)
		return -EINVAL;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;	/* only capture is supported */

	timeperframe = &a->parm.capture.timeperframe;

	/* get the current standard */
	current_std = tvp514x_get_current_std(decoder);
	if (current_std == STD_INVALID)
		return -EINVAL;

	decoder->current_std = current_std;

	*timeperframe =
	    decoder->std_list[current_std].standard.frameperiod;

	return 0;
}

/**
 * ioctl_g_ifparm - V4L2 decoder interface handler for vidioc_int_g_ifparm_num
 * @s: pointer to standard V4L2 device structure
 * @p: pointer to standard V4L2 vidioc_int_g_ifparm_num ioctl structure
 *
 * Gets slave interface parameters.
 * Calculates the required xclk value to support the requested
 * clock parameters in p. This value is returned in the p
 * parameter.
 */
static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
	struct tvp514x_decoder *decoder = s->priv;
	int rval;

	if (p == NULL)
		return -EINVAL;

	if (NULL == decoder->pdata->ifparm)
		return -EINVAL;

	rval = decoder->pdata->ifparm(p);
	if (rval) {
		v4l_err(decoder->client, "g_ifparm.Err[%d]\n", rval);
		return rval;
	}

	p->u.bt656.clock_curr = TVP514X_XCLK_BT656;

	return 0;
}

/**
 * ioctl_g_priv - V4L2 decoder interface handler for vidioc_int_g_priv_num
 * @s: pointer to standard V4L2 device structure
 * @p: void pointer to hold decoder's private data address
 *
 * Returns device's (decoder's) private data area address in p parameter
 */
static int ioctl_g_priv(struct v4l2_int_device *s, void *p)
{
	struct tvp514x_decoder *decoder = s->priv;

	if (NULL == decoder->pdata->priv_data_set)
		return -EINVAL;

	return decoder->pdata->priv_data_set(p);
}

/**
 * ioctl_s_power - V4L2 decoder interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int ioctl_s_power(struct v4l2_int_device *s, enum v4l2_power on)
{
	struct tvp514x_decoder *decoder = s->priv;
	int err = 0;

	switch (on) {
	case V4L2_POWER_OFF:
		/* Power Down Sequence */
		err =
		    tvp514x_write_reg(decoder->client, REG_OPERATION_MODE,
					0x01);
		/* Disable mux for TVP5146/47 decoder data path */
		if (decoder->pdata->power_set)
			err |= decoder->pdata->power_set(on);
		decoder->state = STATE_NOT_DETECTED;
		break;

	case V4L2_POWER_STANDBY:
		if (decoder->pdata->power_set)
			err = decoder->pdata->power_set(on);
		break;

	case V4L2_POWER_ON:
		/* Enable mux for TVP5146/47 decoder data path */
		if ((decoder->pdata->power_set) &&
				(decoder->state == STATE_NOT_DETECTED)) {
			int i;
			struct tvp514x_init_seq *int_seq =
				(struct tvp514x_init_seq *)
				decoder->id->driver_data;

			err = decoder->pdata->power_set(on);

			/* Power Up Sequence */
			for (i = 0; i < int_seq->no_regs; i++) {
				err |= tvp514x_write_reg(decoder->client,
						int_seq->init_reg_seq[i].reg,
						int_seq->init_reg_seq[i].val);
			}
			/* Detect the sensor is not already detected */
			err |= tvp514x_detect(decoder);
			if (err) {
				v4l_err(decoder->client,
						"Unable to detect decoder\n");
				return err;
			}
		}
		err |= tvp514x_configure(decoder);
		break;

	default:
		err = -ENODEV;
		break;
	}

	return err;
}

/**
 * ioctl_init - V4L2 decoder interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the decoder device (calls tvp514x_configure())
 */
static int ioctl_init(struct v4l2_int_device *s)
{
	struct tvp514x_decoder *decoder = s->priv;

	/* Set default standard to auto */
	decoder->tvp514x_regs[REG_VIDEO_STD].val =
	    VIDEO_STD_AUTO_SWITCH_BIT;

	return tvp514x_configure(decoder);
}

/**
 * ioctl_dev_exit - V4L2 decoder interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the dev. at slave detach. The complement of ioctl_dev_init.
 */
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	return 0;
}

/**
 * ioctl_dev_init - V4L2 decoder interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master. Returns 0 if
 * TVP5146/47 device could be found, otherwise returns appropriate error.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	struct tvp514x_decoder *decoder = s->priv;
	int err;

	err = tvp514x_detect(decoder);
	if (err < 0) {
		v4l_err(decoder->client,
			"Unable to detect decoder\n");
		return err;
	}

	v4l_info(decoder->client,
		 "chip version 0x%.2x detected\n", decoder->ver);

	return 0;
}

static struct v4l2_int_ioctl_desc tvp514x_ioctl_desc[] = {
	{vidioc_int_dev_init_num, (v4l2_int_ioctl_func*) ioctl_dev_init},
	{vidioc_int_dev_exit_num, (v4l2_int_ioctl_func*) ioctl_dev_exit},
	{vidioc_int_s_power_num, (v4l2_int_ioctl_func*) ioctl_s_power},
	{vidioc_int_g_priv_num, (v4l2_int_ioctl_func*) ioctl_g_priv},
	{vidioc_int_g_ifparm_num, (v4l2_int_ioctl_func*) ioctl_g_ifparm},
	{vidioc_int_init_num, (v4l2_int_ioctl_func*) ioctl_init},
	{vidioc_int_enum_fmt_cap_num,
	 (v4l2_int_ioctl_func *) ioctl_enum_fmt_cap},
	{vidioc_int_try_fmt_cap_num,
	 (v4l2_int_ioctl_func *) ioctl_try_fmt_cap},
	{vidioc_int_g_fmt_cap_num,
	 (v4l2_int_ioctl_func *) ioctl_g_fmt_cap},
	{vidioc_int_s_fmt_cap_num,
	 (v4l2_int_ioctl_func *) ioctl_s_fmt_cap},
	{vidioc_int_g_parm_num, (v4l2_int_ioctl_func *) ioctl_g_parm},
	{vidioc_int_s_parm_num, (v4l2_int_ioctl_func *) ioctl_s_parm},
	{vidioc_int_queryctrl_num,
	 (v4l2_int_ioctl_func *) ioctl_queryctrl},
	{vidioc_int_g_ctrl_num, (v4l2_int_ioctl_func *) ioctl_g_ctrl},
	{vidioc_int_s_ctrl_num, (v4l2_int_ioctl_func *) ioctl_s_ctrl},
	{vidioc_int_querystd_num, (v4l2_int_ioctl_func *) ioctl_querystd},
	{vidioc_int_s_std_num, (v4l2_int_ioctl_func *) ioctl_s_std},
	{vidioc_int_s_video_routing_num,
		(v4l2_int_ioctl_func *) ioctl_s_routing},
};

static struct tvp514x_decoder tvp514x_dev = {
	.state = STATE_NOT_DETECTED,

	.fmt_list = tvp514x_fmt_list,
	.num_fmts = ARRAY_SIZE(tvp514x_fmt_list),

	.pix = {		/* Default to NTSC 8-bit YUV 422 */
		.width = NTSC_NUM_ACTIVE_PIXELS,
		.height = NTSC_NUM_ACTIVE_LINES,
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.field = V4L2_FIELD_INTERLACED,
		.bytesperline = NTSC_NUM_ACTIVE_PIXELS * 2,
		.sizeimage =
		NTSC_NUM_ACTIVE_PIXELS * 2 * NTSC_NUM_ACTIVE_LINES,
		.colorspace = V4L2_COLORSPACE_SMPTE170M,
		},

	.current_std = STD_NTSC_MJ,
	.std_list = tvp514x_std_list,
	.num_stds = ARRAY_SIZE(tvp514x_std_list),
	.v4l2_int_device = {
		.module = THIS_MODULE,
		.name = TVP514X_MODULE_NAME,
		.type = v4l2_int_type_slave,
	},
	.tvp514x_slave = {
		.ioctls = tvp514x_ioctl_desc,
		.num_ioctls = ARRAY_SIZE(tvp514x_ioctl_desc),
	},
};

/**
 * tvp514x_probe - decoder driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register decoder as an i2c client device and V4L2
 * device.
 */
static int
tvp514x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tvp514x_decoder *decoder;
	int err;

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	decoder = kzalloc(sizeof(*decoder), GFP_KERNEL);
	if (!decoder)
		return -ENOMEM;

	if (!client->dev.platform_data) {
		v4l_err(client, "No platform data!!\n");
		err = -ENODEV;
		goto out_free;
	}

	*decoder = tvp514x_dev;
	decoder->v4l2_int_device.priv = decoder;
	decoder->pdata = client->dev.platform_data;
	decoder->v4l2_int_device.u.slave = &decoder->tvp514x_slave;
	memcpy(decoder->tvp514x_regs, tvp514x_reg_list_default,
			sizeof(tvp514x_reg_list_default));
	/*
	 * Fetch platform specific data, and configure the
	 * tvp514x_reg_list[] accordingly. Since this is one
	 * time configuration, no need to preserve.
	 */
	decoder->tvp514x_regs[REG_OUTPUT_FORMATTER2].val |=
			(decoder->pdata->clk_polarity << 1);
	decoder->tvp514x_regs[REG_SYNC_CONTROL].val |=
			((decoder->pdata->hs_polarity << 2) |
			(decoder->pdata->vs_polarity << 3));
	/*
	 * Save the id data, required for power up sequence
	 */
	decoder->id = (struct i2c_device_id *)id;
	/* Attach to Master */
	strcpy(decoder->v4l2_int_device.u.slave->attach_to,
			decoder->pdata->master);
	decoder->client = client;
	i2c_set_clientdata(client, decoder);

	/* Register with V4L2 layer as slave device */
	err = v4l2_int_device_register(&decoder->v4l2_int_device);
	if (err) {
		i2c_set_clientdata(client, NULL);
		v4l_err(client,
			"Unable to register to v4l2. Err[%d]\n", err);
		goto out_free;

	} else
		v4l_info(client, "Registered to v4l2 master %s!!\n",
				decoder->pdata->master);
	return 0;

out_free:
	kfree(decoder);
	return err;
}

/**
 * tvp514x_remove - decoder driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister decoder as an i2c client device and V4L2
 * device. Complement of tvp514x_probe().
 */
static int __exit tvp514x_remove(struct i2c_client *client)
{
	struct tvp514x_decoder *decoder = i2c_get_clientdata(client);

	if (!client->adapter)
		return -ENODEV;	/* our client isn't attached */

	v4l2_int_device_unregister(&decoder->v4l2_int_device);
	i2c_set_clientdata(client, NULL);
	kfree(decoder);
	return 0;
}
/*
 * TVP5146 Init/Power on Sequence
 */
static const struct tvp514x_reg tvp5146_init_reg_seq[] = {
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS1, 0x02},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS2, 0x00},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS3, 0x80},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x01},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS1, 0x60},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS2, 0x00},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS3, 0xB0},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x01},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x00},
	{TOK_WRITE, REG_OPERATION_MODE, 0x01},
	{TOK_WRITE, REG_OPERATION_MODE, 0x00},
};
static const struct tvp514x_init_seq tvp5146_init = {
	.no_regs = ARRAY_SIZE(tvp5146_init_reg_seq),
	.init_reg_seq = tvp5146_init_reg_seq,
};
/*
 * TVP5147 Init/Power on Sequence
 */
static const struct tvp514x_reg tvp5147_init_reg_seq[] =	{
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS1, 0x02},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS2, 0x00},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS3, 0x80},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x01},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS1, 0x60},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS2, 0x00},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS3, 0xB0},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x01},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS1, 0x16},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS2, 0x00},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS3, 0xA0},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x16},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS1, 0x60},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS2, 0x00},
	{TOK_WRITE, REG_VBUS_ADDRESS_ACCESS3, 0xB0},
	{TOK_WRITE, REG_VBUS_DATA_ACCESS_NO_VBUS_ADDR_INCR, 0x00},
	{TOK_WRITE, REG_OPERATION_MODE, 0x01},
	{TOK_WRITE, REG_OPERATION_MODE, 0x00},
};
static const struct tvp514x_init_seq tvp5147_init = {
	.no_regs = ARRAY_SIZE(tvp5147_init_reg_seq),
	.init_reg_seq = tvp5147_init_reg_seq,
};
/*
 * TVP5146M2/TVP5147M1 Init/Power on Sequence
 */
static const struct tvp514x_reg tvp514xm_init_reg_seq[] = {
	{TOK_WRITE, REG_OPERATION_MODE, 0x01},
	{TOK_WRITE, REG_OPERATION_MODE, 0x00},
};
static const struct tvp514x_init_seq tvp514xm_init = {
	.no_regs = ARRAY_SIZE(tvp514xm_init_reg_seq),
	.init_reg_seq = tvp514xm_init_reg_seq,
};
/*
 * I2C Device Table -
 *
 * name - Name of the actual device/chip.
 * driver_data - Driver data
 */
static const struct i2c_device_id tvp514x_id[] = {
	{"tvp5146", (unsigned long)&tvp5146_init},
	{"tvp5146m2", (unsigned long)&tvp514xm_init},
	{"tvp5147", (unsigned long)&tvp5147_init},
	{"tvp5147m1", (unsigned long)&tvp514xm_init},
	{},
};

MODULE_DEVICE_TABLE(i2c, tvp514x_id);

static struct i2c_driver tvp514x_i2c_driver = {
	.driver = {
		   .name = TVP514X_MODULE_NAME,
		   .owner = THIS_MODULE,
		   },
	.probe = tvp514x_probe,
	.remove = __exit_p(tvp514x_remove),
	.id_table = tvp514x_id,
};

/**
 * tvp514x_init
 *
 * Module init function
 */
static int __init tvp514x_init(void)
{
	return i2c_add_driver(&tvp514x_i2c_driver);
}

/**
 * tvp514x_cleanup
 *
 * Module exit function
 */
static void __exit tvp514x_cleanup(void)
{
	i2c_del_driver(&tvp514x_i2c_driver);
}

module_init(tvp514x_init);
module_exit(tvp514x_cleanup);

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("TVP514X linux decoder driver");
MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*
 * Adaptec AIC79xx device driver for Linux.
 *
 * Copyright (c) 2000-2001 Adaptec Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: //depot/aic7xxx/linux/drivers/scsi/aic7xxx/aic79xx_osm.h#166 $
 *
 */
#ifndef _AIC79XX_LINUX_H_
#define _AIC79XX_LINUX_H_

#include <linux/types.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_spi.h>

/* Core SCSI definitions */
#define AIC_LIB_PREFIX ahd

/* Name space conflict with BSD queue macros */
#ifdef LIST_HEAD
#undef LIST_HEAD
#endif

#include "cam.h"
#include "queue.h"
#include "scsi_message.h"
#include "scsi_iu.h"
#include "aiclib.h"

/*********************************** Debugging ********************************/
#ifdef CONFIG_AIC79XX_DEBUG_ENABLE
#ifdef CONFIG_AIC79XX_DEBUG_MASK
#define AHD_DEBUG 1
#define AHD_DEBUG_OPTS CONFIG_AIC79XX_DEBUG_MASK
#else
/*
 * Compile in debugging code, but do not enable any printfs.
 */
#define AHD_DEBUG 1
#define AHD_DEBUG_OPTS 0
#endif
/* No debugging code. */
#endif

/********************************** Misc Macros *******************************/
#define	powerof2(x)	((((x)-1)&(x))==0)

/************************* Forward Declarations *******************************/
struct ahd_softc;
typedef struct pci_dev *ahd_dev_softc_t;
typedef struct scsi_cmnd      *ahd_io_ctx_t;

/******************************* Byte Order ***********************************/
#define ahd_htobe16(x)	cpu_to_be16(x)
#define ahd_htobe32(x)	cpu_to_be32(x)
#define ahd_htobe64(x)	cpu_to_be64(x)
#define ahd_htole16(x)	cpu_to_le16(x)
#define ahd_htole32(x)	cpu_to_le32(x)
#define ahd_htole64(x)	cpu_to_le64(x)

#define ahd_be16toh(x)	be16_to_cpu(x)
#define ahd_be32toh(x)	be32_to_cpu(x)
#define ahd_be64toh(x)	be64_to_cpu(x)
#define ahd_le16toh(x)	le16_to_cpu(x)
#define ahd_le32toh(x)	le32_to_cpu(x)
#define ahd_le64toh(x)	le64_to_cpu(x)

/************************* Configuration Data *********************************/
extern uint32_t aic79xx_allow_memio;
extern struct scsi_host_template aic79xx_driver_template;

/***************************** Bus Space/DMA **********************************/

typedef uint32_t bus_size_t;

typedef enum {
	BUS_SPACE_MEMIO,
	BUS_SPACE_PIO
} bus_space_tag_t;

typedef union {
	u_long		  ioport;
	volatile uint8_t __iomem *maddr;
} bus_space_handle_t;

typedef struct bus_dma_segment
{
	dma_addr_t	ds_addr;
	bus_size_t	ds_len;
} bus_dma_segment_t;

struct ahd_linux_dma_tag
{
	bus_size_t	alignment;
	bus_size_t	boundary;
	bus_size_t	maxsize;
};
typedef struct ahd_linux_dma_tag* bus_dma_tag_t;

typedef dma_addr_t bus_dmamap_t;

typedef int bus_dma_filter_t(void*, dma_addr_t);
typedef void bus_dmamap_callback_t(void *, bus_dma_segment_t *, int, int);

#define BUS_DMA_WAITOK		0x0
#define BUS_DMA_NOWAIT		0x1
#define BUS_DMA_ALLOCNOW	0x2
#define BUS_DMA_LOAD_SEGS	0x4	/*
					 * Argument is an S/G list not
					 * a single buffer.
					 */

#define BUS_SPACE_MAXADDR	0xFFFFFFFF
#define BUS_SPACE_MAXADDR_32BIT	0xFFFFFFFF
#define BUS_SPACE_MAXSIZE_32BIT	0xFFFFFFFF

int	ahd_dma_tag_create(struct ahd_softc *, bus_dma_tag_t /*parent*/,
			   bus_size_t /*alignment*/, bus_size_t /*boundary*/,
			   dma_addr_t /*lowaddr*/, dma_addr_t /*highaddr*/,
			   bus_dma_filter_t*/*filter*/, void */*filterarg*/,
			   bus_size_t /*maxsize*/, int /*nsegments*/,
			   bus_size_t /*maxsegsz*/, int /*flags*/,
			   bus_dma_tag_t */*dma_tagp*/);

void	ahd_dma_tag_destroy(struct ahd_softc *, bus_dma_tag_t /*tag*/);

int	ahd_dmamem_alloc(struct ahd_softc *, bus_dma_tag_t /*dmat*/,
			 void** /*vaddr*/, int /*flags*/,
			 bus_dmamap_t* /*mapp*/);

void	ahd_dmamem_free(struct ahd_softc *, bus_dma_tag_t /*dmat*/,
			void* /*vaddr*/, bus_dmamap_t /*map*/);

void	ahd_dmamap_destroy(struct ahd_softc *, bus_dma_tag_t /*tag*/,
			   bus_dmamap_t /*map*/);

int	ahd_dmamap_load(struct ahd_softc *ahd, bus_dma_tag_t /*dmat*/,
			bus_dmamap_t /*map*/, void * /*buf*/,
			bus_size_t /*buflen*/, bus_dmamap_callback_t *,
			void */*callback_arg*/, int /*flags*/);

int	ahd_dmamap_unload(struct ahd_softc *, bus_dma_tag_t, bus_dmamap_t);

/*
 * Operations performed by ahd_dmamap_sync().
 */
#define BUS_DMASYNC_PREREAD	0x01	/* pre-read synchronization */
#define BUS_DMASYNC_POSTREAD	0x02	/* post-read synchronization */
#define BUS_DMASYNC_PREWRITE	0x04	/* pre-write synchronization */
#define BUS_DMASYNC_POSTWRITE	0x08	/* post-write synchronization */

/*
 * XXX
 * ahd_dmamap_sync is only used on buffers allocated with
 * the pci_alloc_consistent() API.  Although I'm not sure how
 * this works on architectures with a write buffer, Linux does
 * not have an API to sync "coherent" memory.  Perhaps we need
 * to do an mb()?
 */
#define ahd_dmamap_sync(ahd, dma_tag, dmamap, offset, len, op)

/************************** Timer DataStructures ******************************/
typedef struct timer_list ahd_timer_t;

/********************************** Includes **********************************/
#ifdef CONFIG_AIC79XX_REG_PRETTY_PRINT
#define AIC_DEBUG_REGISTERS 1
#else
#define AIC_DEBUG_REGISTERS 0
#endif
#include "aic79xx.h"

/***************************** Timer Facilities *******************************/
#define ahd_timer_init init_timer
#define ahd_timer_stop del_timer_sync

/***************************** SMP support ************************************/
#include <linux/spinlock.h>

#define AIC79XX_DRIVER_VERSION "3.0"

/*************************** Device Data Structures ***************************/
/*
 * A per probed device structure used to deal with some error recovery
 * scenarios that the Linux mid-layer code just doesn't know how to
 * handle.  The structure allocated for a device only becomes persistent
 * after a successfully completed inquiry command to the target when
 * that inquiry data indicates a lun is present.
 */

typedef enum {
	AHD_DEV_FREEZE_TIL_EMPTY = 0x02, /* Freeze queue until active == 0 */
	AHD_DEV_Q_BASIC		 = 0x10, /* Allow basic device queuing */
	AHD_DEV_Q_TAGGED	 = 0x20, /* Allow full SCSI2 command queueing */
	AHD_DEV_PERIODIC_OTAG	 = 0x40, /* Send OTAG to prevent starvation */
} ahd_linux_dev_flags;

struct ahd_linux_device {
	TAILQ_ENTRY(ahd_linux_device) links;

	/*
	 * The number of transactions currently
	 * queued to the device.
	 */
	int			active;

	/*
	 * The currently allowed number of 
	 * transactions that can be queued to
	 * the device.  Must be signed for
	 * conversion from tagged to untagged
	 * mode where the device may have more
	 * than one outstanding active transaction.
	 */
	int			openings;

	/*
	 * A positive count indicates that this
	 * device's queue is halted.
	 */
	u_int			qfrozen;
	
	/*
	 * Cumulative command counter.
	 */
	u_long			commands_issued;

	/*
	 * The number of tagged transactions when
	 * running at our current opening level
	 * that have been successfully received by
	 * this device since the last QUEUE FULL.
	 */
	u_int			tag_success_count;
#define AHD_TAG_SUCCESS_INTERVAL 50

	ahd_linux_dev_flags	flags;

	/*
	 * Per device timer.
	 */
	struct timer_list	timer;

	/*
	 * The high limit for the tags variable.
	 */
	u_int			maxtags;

	/*
	 * The computed number of tags outstanding
	 * at the time of the last QUEUE FULL event.
	 */
	u_int			tags_on_last_queuefull;

	/*
	 * How many times we have seen a queue full
	 * with the same number of tags.  This is used
	 * to stop our adaptive queue depth algorithm
	 * on devices with a fixed number of tags.
	 */
	u_int			last_queuefull_same_count;
#define AHD_LOCK_TAGS_COUNT 50

	/*
	 * How many transactions have been queued
	 * without the device going idle.  We use
	 * this statistic to determine when to issue
	 * an ordered tag to prevent transaction
	 * starvation.  This statistic is only updated
	 * if the AHD_DEV_PERIODIC_OTAG flag is set
	 * on this device.
	 */
	u_int			commands_since_idle_or_otag;
#define AHD_OTAG_THRESH	500
};

/********************* Definitions Required by the Core ***********************/
/*
 * Number of SG segments we require.  So long as the S/G segments for
 * a particular transaction are allocated in a physically contiguous
 * manner and are allocated below 4GB, the number of S/G segments is
 * unrestricted.
 */
#define	AHD_NSEG 128

/*
 * Per-SCB OSM storage.
 */
struct scb_platform_data {
	struct ahd_linux_device	*dev;
	dma_addr_t		 buf_busaddr;
	uint32_t		 xfer_len;
	uint32_t		 sense_resid;	/* Auto-Sense residual */
};

/*
 * Define a structure used for each host adapter.  All members are
 * aligned on a boundary >= the size of the member to honor the
 * alignment restrictions of the various platforms supported by
 * this driver.
 */
struct ahd_platform_data {
	/*
	 * Fields accessed from interrupt context.
	 */
	struct scsi_target *starget[AHD_NUM_TARGETS]; 

	spinlock_t		 spin_lock;
	struct completion	*eh_done;
	struct Scsi_Host        *host;		/* pointer to scsi host */
#define AHD_LINUX_NOIRQ	((uint32_t)~0)
	uint32_t		 irq;		/* IRQ for this adapter */
	uint32_t		 bios_address;
	resource_size_t		 mem_busaddr;	/* Mem Base Addr */
};

/************************** OS Utility Wrappers *******************************/
#define printf printk
#define M_NOWAIT GFP_ATOMIC
#define M_WAITOK 0
#define malloc(size, type, flags) kmalloc(size, flags)
#define free(ptr, type) kfree(ptr)

void ahd_delay(long);

/***************************** Low Level I/O **********************************/
uint8_t ahd_inb(struct ahd_softc * ahd, long port);
void ahd_outb(struct ahd_softc * ahd, long port, uint8_t val);
void ahd_outw_atomic(struct ahd_softc * ahd,
				     long port, uint16_t val);
void ahd_outsb(struct ahd_softc * ahd, long port,
			       uint8_t *, int count);
void ahd_insb(struct ahd_softc * ahd, long port,
			       uint8_t *, int count);

/**************************** Initialization **********************************/
int		ahd_linux_register_host(struct ahd_softc *,
					struct scsi_host_template *);

/*************************** Pretty Printing **********************************/
struct info_str {
	char *buffer;
	int length;
	off_t offset;
	int pos;
};

/******************************** Locking *************************************/
static inline void
ahd_lockinit(struct ahd_softc *ahd)
{
	spin_lock_init(&ahd->platform_data->spin_lock);
}

static inline void
ahd_lock(struct ahd_softc *ahd, unsigned long *flags)
{
	spin_lock_irqsave(&ahd->platform_data->spin_lock, *flags);
}

static inline void
ahd_unlock(struct ahd_softc *ahd, unsigned long *flags)
{
	spin_unlock_irqrestore(&ahd->platform_data->spin_lock, *flags);
}

/******************************* PCI Definitions ******************************/
/*
 * PCIM_xxx: mask to locate subfield in register
 * PCIR_xxx: config register offset
 * PCIC_xxx: device class
 * PCIS_xxx: device subclass
 * PCIP_xxx: device programming interface
 * PCIV_xxx: PCI vendor ID (only required to fixup ancient devices)
 * PCID_xxx: device ID
 */
#define PCIR_DEVVENDOR		0x00
#define PCIR_VENDOR		0x00
#define PCIR_DEVICE		0x02
#define PCIR_COMMAND		0x04
#define PCIM_CMD_PORTEN		0x0001
#define PCIM_CMD_MEMEN		0x0002
#define PCIM_CMD_BUSMASTEREN	0x0004
#define PCIM_CMD_MWRICEN	0x0010
#define PCIM_CMD_PERRESPEN	0x0040
#define	PCIM_CMD_SERRESPEN	0x0100
#define PCIR_STATUS		0x06
#define PCIR_REVID		0x08
#define PCIR_PROGIF		0x09
#define PCIR_SUBCLASS		0x0a
#define PCIR_CLASS		0x0b
#define PCIR_CACHELNSZ		0x0c
#define PCIR_LATTIMER		0x0d
#define PCIR_HEADERTYPE		0x0e
#define PCIM_MFDEV		0x80
#define PCIR_BIST		0x0f
#define PCIR_CAP_PTR		0x34

/* config registers for header type 0 devices */
#define PCIR_MAPS	0x10
#define PCIR_SUBVEND_0	0x2c
#define PCIR_SUBDEV_0	0x2e

/****************************** PCI-X definitions *****************************/
#define PCIXR_COMMAND	0x96
#define PCIXR_DEVADDR	0x98
#define PCIXM_DEVADDR_FNUM	0x0003	/* Function Number */
#define PCIXM_DEVADDR_DNUM	0x00F8	/* Device Number */
#define PCIXM_DEVADDR_BNUM	0xFF00	/* Bus Number */
#define PCIXR_STATUS	0x9A
#define PCIXM_STATUS_64BIT	0x0001	/* Active 64bit connection to device. */
#define PCIXM_STATUS_133CAP	0x0002	/* Device is 133MHz capable */
#define PCIXM_STATUS_SCDISC	0x0004	/* Split Completion Discarded */
#define PCIXM_S