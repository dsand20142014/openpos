	PDEBUG(3, "%d", *val);
	ov->hue = *val;

	return 0;
}

/* -------------------------------------------------------------------------- */

static int
sensor_set_picture(struct usb_ov511 *ov, struct video_picture *p)
{
	int rc;

	PDEBUG(4, "sensor_set_picture");

	ov->whiteness = p->whiteness;

	/* Don't return error if a setting is unsupported, or rest of settings
	 * will not be performed */

	rc = sensor_set_contrast(ov, p->contrast);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_brightness(ov, p->brightness);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_saturation(ov, p->colour);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_hue(ov, p->hue);
	if (FATAL_ERROR(rc))
		return rc;

	return 0;
}

static int
sensor_get_picture(struct usb_ov511 *ov, struct video_picture *p)
{
	int rc;

	PDEBUG(4, "sensor_get_picture");

	/* Don't return error if a setting is unsupported, or rest of settings
	 * will not be performed */

	rc = sensor_get_contrast(ov, &(p->contrast));
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_get_brightness(ov, &(p->brightness));
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_get_saturation(ov, &(p->colour));
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_get_hue(ov, &(p->hue));
	if (FATAL_ERROR(rc))
		return rc;

	p->whiteness = 105 << 8;

	return 0;
}

#if 0
// FIXME: Exposure range is only 0x00-0x7f in interlace mode
/* Sets current exposure for sensor. This only has an effect if auto-exposure
 * is off */
static inline int
sensor_set_exposure(struct usb_ov511 *ov, unsigned char val)
{
	int rc;

	PDEBUG(3, "%d", val);

	if (ov->stop_during_set)
		if (ov51x_stop(ov) < 0)
			return -EIO;

	switch (ov->sensor) {
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV7610:
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
		rc = i2c_w(ov, 0x10, val);
		if (rc < 0)
			goto out;

		break;
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	default:
	