/*
 * Copyright 2012 Sand Golden card, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file hm2057.c
 *
 * @brief hm2057 camera driver functions
 *
 * @ingroup Camera
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>

#include <mach/hardware.h>
#include <media/v4l2-int-device.h>
#include "mxc_v4l2_capture.h"

#include "hm2057.h"

#define MIN_FPS 5
#define MAX_FPS 30
#define DEFAULT_FPS 30

#define hm2057_XCLK_MIN 6000000
#define hm2057_XCLK_MAX 27000000

#define UXGA_SIZE  		 1600*1200
#define SXGA_SIZE      		 1280*1024
#define HD800_SIZE              1280*800
#define HD720_SIZE              1280*720
#define SVGA_SIZE                 800*600
#define VGA_SIZE                   640*480
#define CIF_SIZE                    352*288
#define QVGA_SIZE                320*240
#define QCIF_SIZE                  176*144


/*!
 * Maintains the information on the current state of the sesor.
 */
struct sensor {
	const struct hm2057_platform_data *platform_data;
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	struct v4l2_captureparm streamcap;
	bool on;

	/* control settings */
	int brightness;
	int hue;
	int contrast;
	int saturation;
	int red;
	int green;
	int blue;
	int ae_mode;

	u32 csi;
	u32 mclk;

} hm2057_data;

static struct regulator *io_regulator;
static struct regulator *core_regulator;
static struct regulator *analog_regulator;
static struct regulator *gpo_regulator;

extern void gpio_sensor_active(void);
extern void gpio_sensor_inactive(void);


//OK
static u8 hm2057_i2c_read(u16 reg)    
{
	struct i2c_client *client = hm2057_data.i2c_client;
	int i2c_ret;
	u8 value;
	u8 buf0[2], buf1[1];
	u16 addr = client->addr;
	u16 flags = client->flags;
	struct i2c_msg msg[2] = {
		{addr, flags, 2, buf0},
		{addr, flags | I2C_M_RD, 1, buf1},
	};

	buf0[0] = (reg & 0xff00) >> 8;
	buf0[1] = reg & 0xff;
	i2c_ret = i2c_transfer(client->adapter, msg, 2);
	if (i2c_ret < 0) {
		pr_debug("%s: read reg error : Reg 0x%02x\n", __func__, reg);
		return 0;
	}

	value = buf1[0];

	return value;
}



//OK
static int hm2057_i2c_write(u16 reg, u8 value)
{
	struct i2c_client *client = hm2057_data.i2c_client;
	u16 addr = client->addr;
	u16 flags = client->flags;
	u8 buf[4];
	int i2c_ret;
	struct i2c_msg msg = { addr, flags, 3, buf };

	buf[0] = (reg & 0xff00) >> 8;
	buf[1] = reg & 0xff;
	buf[2] = value;

	i2c_ret = i2c_transfer(client->adapter, &msg, 1);
	if (i2c_ret < 0) {
		pr_err("%s: write reg error : Reg 0x%02x = 0x%04x\n",
		       __func__, reg, value);
		return -EIO;
	}
	
	return i2c_ret;
}

//OK
static int32_t hm2057_i2c_write_table(
	struct hm2057_i2c_reg_conf const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;

	for (i = 0; i < num_of_items_in_table; i++) {
		rc = hm2057_i2c_write(reg_conf_tbl->waddr, reg_conf_tbl->wdata);
		if (rc < 0)
		{
			pr_alert("%s i2c write error.\n", __func__);
			return rc;
		}
		if (reg_conf_tbl->mdelay_time != 0)
			msleep(reg_conf_tbl->mdelay_time);
		reg_conf_tbl++;
	}

	return 0;
}

int camera_is_exist(void)
{
    return (hm2057_i2c_read(0x0001)==0x20&&hm2057_i2c_read(0x0002)==0x56);
}
EXPORT_SYMBOL(camera_is_exist);

static void hm2057_set_mclk(void)
{
	u32 tgt_xclk;	/* target xclk */

	tgt_xclk = hm2057_data.mclk;
	tgt_xclk = min(tgt_xclk, (u32)hm2057_XCLK_MAX);
	tgt_xclk = max(tgt_xclk, (u32)hm2057_XCLK_MIN);
	hm2057_data.mclk = tgt_xclk;

	pr_debug("   Setting mclk to %d MHz\n",
		tgt_xclk / 1000000);
	set_mclk_rate(&hm2057_data.mclk);
}

//OK
static int hm2057_init_mode(struct sensor *s)
{
	int ret = -1;
	//struct reg_value *setting;
	//int i, num;

	//pr_debug("In hm2057:hm2057_init_mode capturemode is %d\n",
	//	s->streamcap.capturemode);

	ret=hm2057_i2c_write_table(&hm2057_regs.init_tbl[0],
							hm2057_regs.init_tbl_size);

/*	if (s->streamcap.capturemode & V4L2_MODE_HIGHQUALITY) {
		s->pix.width = 1600;
		s->pix.height = 1200;
		ret=hm2057_i2c_write_table(&hm2057_regs.size_uxga_tbl[0],
							hm2057_regs.size_uxga_tbl_size);
	} else {
		s->pix.width = 320;
		s->pix.height = 240;
		ret=hm2057_i2c_write_table(&hm2057_regs.size_qvga_tbl[0],
							hm2057_regs.size_qvga_tbl_size);
	}*/

	return ret;
}

/* At present only support change to 15fps(only for SVGA mode) */
//OK
static int hm2057_set_fps(struct sensor *s, int fps)
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
static int hm2057_set_format(struct sensor *s, int format)
{
	int ret = 0;

	if (format == V4L2_PIX_FMT_RGB565) {
		if (hm2057_i2c_write(0x0027, 0x40) < 0)
		//if (hm2057_i2c_write(0x0027, 0x30) < 0)
			ret = -EPERM;
		/* set RGB565 format */
		if (hm2057_i2c_write(0x0000,0x01) < 0)
			ret = -EPERM;
		if (hm2057_i2c_write(0x0100,0x01) < 0)
			ret = -EPERM;
		if (hm2057_i2c_write(0x0101,0x01) < 0)
			ret = -EPERM;

		pr_debug("reg 0x0020 = 0x%02x \n", hm2057_i2c_read(0x0020));
	} else if (format == V4L2_PIX_FMT_YUV420) {
		/* set YUV420 format */
		if (hm2057_i2c_write(0x0027, 0x30) < 0)
			ret = -EPERM;
		
		if (hm2057_i2c_write(0x0000,0x01) < 0)
			ret = -EPERM;
		if (hm2057_i2c_write(0x0100,0x01) < 0)
			ret = -EPERM;
		if (hm2057_i2c_write(0x0101,0x01) < 0)
			ret = -EPERM;

	} else {
		pr_alert("format not supported!!\n");
	}

	return ret;
}

static int hm2057_set_window(int size)
{
	int ret = 0;
	
	switch(size)
	{
		case UXGA_SIZE:
			ret = hm2057_i2c_write_table(size_uxga_tbl,
							ARRAY_SIZE(size_uxga_tbl));
		break;
		case SXGA_SIZE:
			ret = hm2057_i2c_write_table(size_sxga_tbl,
							ARRAY_SIZE(size_sxga_tbl));
		break;
		case HD800_SIZE:
			ret = hm2057_i2c_write_table(size_hd800_tbl,
							ARRAY_SIZE(size_hd800_tbl));
		break;
		case HD720_SIZE:
			ret = hm2057_i2c_write_table(size_hd720_tbl,
							ARRAY_SIZE(size_hd720_tbl));
		break;

		case SVGA_SIZE:
			ret = hm2057_i2c_write_table(size_svga_tbl,
							ARRAY_SIZE(size_svga_tbl));
		break;

		case VGA_SIZE:
			ret = hm2057_i2c_write_table(size_vga_tbl,
							ARRAY_SIZE(size_vga_tbl));
		break;

		case CIF_SIZE:
			ret = hm2057_i2c_write_table(size_cif_tbl,
							ARRAY_SIZE(size_cif_tbl));
		break;
		case QVGA_SIZE:
		ret = hm2057_i2c_write_table(size_qvga_tbl,
							ARRAY_SIZE(size_qvga_tbl));
		break;

		case QCIF_SIZE:
		ret = hm2057_i2c_write_table(size_qcif_tbl,
							ARRAY_SIZE(size_qcif_tbl));
		break;

		default:
			ret = -1;

	}
	return(ret);
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
	//pr_debug("In hm2057:ioctl_g_ifparm\n");

	if (s == NULL) {
		pr_err("   ERROR!! no slave device set!\n");
		return -1;
	}

	memset(p, 0, sizeof(*p));
	p->u.bt656.clock_curr = hm2057_data.mclk;
	p->if_type = V4L2_IF_TYPE_BT656;
	p->u.bt656.mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT;
	p->u.bt656.clock_min = hm2057_XCLK_MIN;
	p->u.bt656.clock_max = hm2057_XCLK_MAX;

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
	u8 reg_tmp;
	unsigned long vir_camera_cntr;

	#define CAMERA_CNTR   0xA0900000

	//pr_debug("In hm2057:ioctl_s_power\n");   //pr_debug
	
	vir_camera_cntr = (unsigned long )ioremap(CAMERA_CNTR, 1);
        
	if (on ) //&& !sensor->on
	{	
	        reg_tmp =  readb(vir_camera_cntr) |(1 << 6);
		writeb( reg_tmp & ~(1 << 7), vir_camera_cntr);
		pr_debug("hm2057 poweron\n");
		/*reg_tmp =  readb(vir_camera_cntr);
		writeb( reg_tmp | (1 << 6), vir_camera_cntr);
		msleep(5);
		writeb( reg_tmp & ~(1 << 6), vir_camera_cntr);
		msleep(5);
		writeb( reg_tmp | (1 << 6), vir_camera_cntr);*/
		msleep(5);
	}
	else if (!on )  //&& sensor->on
	{
	        reg_tmp =  readb(vir_camera_cntr);
		writeb( reg_tmp |(1 << 7), vir_camera_cntr);
		pr_debug("hm2057 poweroff\n");
	}
	iounmap(vir_camera_cntr);
#if 0
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
#endif

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

	//pr_debug("In hm2057:ioctl_g_parm\n");

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pr_debug("   type is V4L2_BUF_TYPE_VIDEO_CAPTURE\n");
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

	//pr_debug("In hm2057:ioctl_s_parm\n");

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pr_debug("   type is V4L2_BUF_TYPE_VIDEO_CAPTURE\n");

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

		ret = hm2057_init_mode(sensor);
		if (tgt_fps == 15)
			hm2057_set_fps(sensor, tgt_fps);
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
		//ret = hm2057_init_mode(sensor);
		ret = hm2057_set_window(size);
		//pr_debug("capture size = %d ret = %d\n",size,ret);
		if(!ret)
			ret = hm2057_set_format(sensor, V4L2_PIX_FMT_RGB565);
		break;
	case V4L2_PIX_FMT_UYVY:
		if (size > 640 * 480)
			sensor->streamcap.capturemode = V4L2_MODE_HIGHQUALITY;
		else
			sensor->streamcap.capturemode = 0;
		ret = hm2057_init_mode(sensor);
		break;
	case V4L2_PIX_FMT_YUV420:
		if (size > 640 * 480)
			sensor->streamcap.capturemode = V4L2_MODE_HIGHQUALITY;
		else
			sensor->streamcap.capturemode = 0;
		ret = hm2057_init_mode(sensor);

		/* YUYV: width * 2, YY: width */
		ret = hm2057_set_format(sensor, V4L2_PIX_FMT_YUV420);
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

	//pr_debug("In hm2057:ioctl_g_fmt_cap.\n");

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

	//pr_debug("In hm2057:ioctl_g_ctrl\n");

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		vc->value = hm2057_data.brightness;
		break;
	case V4L2_CID_HUE:
		vc->value = hm2057_data.hue;
		break;
	case V4L2_CID_CONTRAST:
		vc->value = hm2057_data.contrast;
		break;
	case V4L2_CID_SATURATION:
		vc->value = hm2057_data.saturation;
		break;
	case V4L2_CID_RED_BALANCE:
		vc->value = hm2057_data.red;
		break;
	case V4L2_CID_BLUE_BALANCE:
		vc->value = hm2057_data.blue;
		break;
	case V4L2_CID_EXPOSURE:
		vc->value = hm2057_data.ae_mode;
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

	//pr_debug("In hm2057:ioctl_s_ctrl %d\n", vc->id);

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		pr_debug("   V4L2_CID_BRIGHTNESS\n");
		break;
	case V4L2_CID_CONTRAST:
		pr_debug("   V4L2_CID_CONTRAST\n");
		break;
	case V4L2_CID_SATURATION:
		pr_debug("   V4L2_CID_SATURATION\n");
		break;
	case V4L2_CID_HUE:
		pr_debug("   V4L2_CID_HUE\n");
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		pr_debug(
			"   V4L2_CID_AUTO_WHITE_BALANCE\n");
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		pr_debug(
			"   V4L2_CID_DO_WHITE_BALANCE\n");
		break;
	case V4L2_CID_RED_BALANCE:
		pr_debug("   V4L2_CID_RED_BALANCE\n");
		break;
	case V4L2_CID_BLUE_BALANCE:
		pr_debug("   V4L2_CID_BLUE_BALANCE\n");
		break;
	case V4L2_CID_GAMMA:
		pr_debug("   V4L2_CID_GAMMA\n");
		break;
	case V4L2_CID_EXPOSURE:
		pr_debug("   V4L2_CID_EXPOSURE\n");
		break;
	case V4L2_CID_AUTOGAIN:
		pr_debug("   V4L2_CID_AUTOGAIN\n");
		break;
	case V4L2_CID_GAIN:
		pr_debug("   V4L2_CID_GAIN\n");
		break;
	case V4L2_CID_HFLIP:
		pr_debug("   V4L2_CID_HFLIP\n");
		break;
	case V4L2_CID_VFLIP:
		pr_debug("   V4L2_CID_VFLIP\n");
		break;
	default:
		pr_debug("   Default case\n");
		retval = -EPERM;
		break;
	}

	return retval;
}

static int ioctl_g_brightness(struct v4l2_int_device *s,unsigned char *pucValue)
{
	unsigned char ucValue;
	*pucValue = hm2057_i2c_read(0x385);
	ucValue =  hm2057_i2c_read(0x385);
	//pr_debug("light value=%02x\n",ucValue);
	return 0;
}

/*!
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 */
 //OK
static int ioctl_init(struct v4l2_int_device *s)
{
	//pr_debug("In hm2057:ioctl_init\n");

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
	struct sensor *sensor = s->priv;

	return hm2057_init_mode(sensor);
}

/*!
 * ioctl_dev_exit - V4L2 sensor interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the device when slave detaches to the master.
 */
 //OK
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	//pr_debug("In hm2057:ioctl_dev_exit\n");
	return 0;
}

/*!
 * This structure defines all the ioctls for this module and links them to the
 * enumeration.
 */
static struct v4l2_int_ioctl_desc hm2057_ioctl_desc[] = {
	{vidioc_int_dev_init_num, (v4l2_int_ioctl_func *)ioctl_dev_init},
	{vidioc_int_dev_exit_num, (v4l2_int_ioctl_func*)ioctl_dev_exit},
	{vidioc_int_s_power_num, (v4l2_int_ioctl_func *)ioctl_s_power},
	{vidioc_int_g_ifparm_num, (v4l2_int_ioctl_func *)ioctl_g_ifparm},
	{vidioc_int_g_brightness_num, (v4l2_int_ioctl_func *)ioctl_g_brightness},
/*	{vidioc_int_g_needs_reset_num,
				(v4l2_int_ioctl_func *)ioctl_g_needs_reset}, */
/*	{vidioc_int_reset_num, (v4l2_int_ioctl_func *)ioctl_reset}, */
	{vidioc_int_init_num, (v4l2_int_ioctl_func *)ioctl_init},
/*	{vidioc_int_enum_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_enum_fmt_cap}, */
/*	{vidioc_int_try_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_try_fmt_cap}, */
	{vidioc_int_g_fmt_cap_num, (v4l2_int_ioctl_func *)ioctl_g_fmt_cap},
	{vidioc_int_s_fmt_cap_num, (v4l2_int_ioctl_func*)ioctl_s_fmt_cap},
	{vidioc_int_g_parm_num, (v4l2_int_ioctl_func *)ioctl_g_parm},
	{vidioc_int_s_parm_num, (v4l2_int_ioctl_func *)ioctl_s_parm},
/*	{vidioc_int_queryctrl_num, (v4l2_int_ioctl_func *)ioctl_queryctrl}, */
	{vidioc_int_g_ctrl_num, (v4l2_int_ioctl_func *)ioctl_g_ctrl},
	{vidioc_int_s_ctrl_num, (v4l2_int_ioctl_func *)ioctl_s_ctrl},
	
};

static struct v4l2_int_slave hm2057_slave = {
	.ioctls = hm2057_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(hm2057_ioctl_desc),
};

static struct v4l2_int_device hm2057_int_device = {
	.module = THIS_MODULE,
	.name = "hm2057",
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &hm2057_slave,
		},
};

/*!
 * hm2057 I2C attach function
 * Function set in i2c_driver struct.
 * Called by insmod hm2057_camera.ko.
 *
 * @param client            struct i2c_client*
 * @return  Error code indicating success or failure
 */
static int hm2057_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int retval;
	struct mxc_camera_platform_data *plat_data = client->dev.platform_data;

	pr_debug("In hm2057_probe (RH_BT565)\n");

	/* Set initial values for the sensor struct. */
	memset(&hm2057_data, 0, sizeof(hm2057_data));
	hm2057_data.i2c_client = client;
	hm2057_data.mclk = 24000000;
	hm2057_data.mclk = plat_data->mclk;
	hm2057_data.pix.pixelformat = V4L2_PIX_FMT_UYVY;
	hm2057_data.pix.width = 320;   // 320
	hm2057_data.pix.height = 240;   //240
	hm2057_data.streamcap.capability = V4L2_MODE_HIGHQUALITY
					   | V4L2_CAP_TIMEPERFRAME;
	hm2057_data.streamcap.capturemode = 0;
	hm2057_data.streamcap.timeperframe.denominator = DEFAULT_FPS;
	hm2057_data.streamcap.timeperframe.numerator = 1;

	gpio_sensor_active();
	hm2057_set_mclk();

	if (plat_data->io_regulator) {
		io_regulator =
		    regulator_get(&client->dev, plat_data->io_regulator);
		if (!IS_ERR(io_regulator)) {
			regulator_set_voltage(io_regulator, 2800000, 2800000);
			if (regulator_enable(io_regulator) != 0) {
				pr_err("%s:io set voltage error\n", __func__);
				goto err1;
			} else {
				dev_dbg(&client->dev,
					"%s:io set voltage ok\n", __func__);
			}
		} else
			io_regulator = NULL;
	}

	if (plat_data->core_regulator) {
		core_regulator =
		    regulator_get(&client->dev, plat_data->core_regulator);
		if (!IS_ERR(core_regulator)) {
			regulator_set_voltage(core_regulator,
					 1300000, 1300000);
			if (regulator_enable(core_regulator) != 0) {
				pr_err("%s:core set voltage error\n", __func__);
				goto err2;
			} else {
				dev_dbg(&client->dev,
					"%s:core set voltage ok\n", __func__);
			}
		} else
			core_regulator = NULL;
	}

	if (plat_data->analog_regulator) {
		analog_regulator =
		    regulator_get(&client->dev, plat_data->analog_regulator);
		if (!IS_ERR(analog_regulator)) {
			regulator_set_voltage(analog_regulator, 2000000, 2000000);
			if (regulator_enable(analog_regulator) != 0) {
				pr_err("%s:analog set voltage error\n",
					 __func__);
				goto err3;
			} else {
				dev_dbg(&client->dev,
					"%s:analog set voltage ok\n", __func__);
			}
		} else
			analog_regulator = NULL;
	}

	if (plat_data->gpo_regulator) {
		gpo_regulator =
		    regulator_get(&client->dev, plat_data->gpo_regulator);
		if (!IS_ERR(gpo_regulator)) {
			if (regulator_enable(gpo_regulator) != 0) {
				pr_err("%s:gpo3 set voltage error\n", __func__);
				goto err4;
			} else {
				dev_dbg(&client->dev,
					"%s:gpo3 set voltage ok\n", __func__);
			}
		} else
			gpo_regulator = NULL;
	}

	/* This function attaches this structure to the /dev/video0 device.
	 * The pointer in priv points to the hm2057_data structure here.*/
	hm2057_int_device.priv = &hm2057_data;
	retval = v4l2_int_device_register(&hm2057_int_device);

	
	return retval;

err4:
	if (analog_regulator) {
		regulator_disable(analog_regulator);
		regulator_put(analog_regulator);
	}
err3:
	if (core_regulator) {
		regulator_disable(core_regulator);
		regulator_put(core_regulator);
	}
err2:
	if (io_regulator) {
		regulator_disable(io_regulator);
		regulator_put(io_regulator);
	}
err1:
	return -1;
}

/*!
 * hm2057 I2C detach function
 * Called on rmmod hm2057_camera.ko
 *
 * @param client            struct i2c_client*
 * @return  Error code indicating success or failure
 */
static int hm2057_remove(struct i2c_client *client)
{
	pr_debug("In hm2057_remove\n");

	v4l2_int_device_unregister(&hm2057_int_device);

	if (gpo_regulator) {
		regulator_disable(gpo_regulator);
		regulator_put(gpo_regulator);
	}

	if (analog_regulator) {
		regulator_disable(analog_regulator);
		regulator_put(analog_regulator);
	}

	if (core_regulator) {
		regulator_disable(core_regulator);
		regulator_put(core_regulator);
	}

	if (io_regulator) {
		regulator_disable(io_regulator);
		regulator_put(io_regulator);
	}
	gpio_sensor_inactive();
	return 0;
}

static const struct i2c_device_id hm2057_id[] = {
	{"hm2057", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, hm2057_id);

static struct i2c_driver hm2057_i2c_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "hm2057",
		  },
	.probe = hm2057_probe,
	.remove = hm2057_remove,
	.id_table = hm2057_id,
/* To add power management add .suspend and .resume functions */
};

/*!
 * hm2057 init function
 * Called by insmod hm2057_camera.ko.
 *
 * @return  Error code indicating success or failure
 */
static __init int hm2057_init(void)
{
	u8 err;

	pr_debug("In hm2057_init\n");

	err = i2c_add_driver(&hm2057_i2c_driver);
	if (err != 0)
		pr_err("%s:driver registration failed, error=%d \n",
			__func__, err);

	return err;
}

/*!
 * hm2057 cleanup function
 * Called on rmmod hm2057_camera.ko
 *
 * @return  Error code indicating success or failure
 */
static void __exit hm2057_clean(void)
{
	pr_debug("In hm2057_clean\n");
	i2c_del_driver(&hm2057_i2c_driver);
}

module_init(hm2057_init);
module_exit(hm2057_clean);

MODULE_AUTHOR("zengshu");
MODULE_DESCRIPTION("hm2057 Camera Driver");
MODULE_LICENSE("GPL");
