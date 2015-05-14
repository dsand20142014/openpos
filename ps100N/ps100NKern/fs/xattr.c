/*
 *  s2255drv.c - a driver for the Sensoray 2255 USB video capture device
 *
 *   Copyright (C) 2007-2008 by Sensoray Company Inc.
 *                              Dean Anderson
 *
 * Some video buffer code based on vivi driver:
 *
 * Sensoray 2255 device supports 4 simultaneous channels.
 * The channels are not "crossbar" inputs, they are physically
 * attached to separate video decoders.
 *
 * Because of USB2.0 bandwidth limitations. There is only a
 * certain amount of data which may be transferred at one time.
 *
 * Example maximum bandwidth utilization:
 *
 * -full size, color mode YUYV or YUV422P: 2 channels at once
 *
 * -full or half size Grey scale: all 4 channels at once
 *
 * -half size, color mode YUYV or YUV422P: all 4 channels at once
 *
 * -full size, color mode YUYV or YUV422P 1/2 frame rate: all 4 channels
 *  at once.
 *  (TODO: Incorporate videodev2 frame rate(FR) enumeration,
 *  which is currently experimental.)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/videodev2.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/smp_lock.h>
#include <media/videobuf-vmalloc.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <linux/vmalloc.h>
#include <linux/usb.h>

#define FIRMWARE_FILE_NAME "f2255usb.bin"



/* default JPEG quality */
#define S2255_DEF_JPEG_QUAL     50
/* vendor request in */
#define S2255_VR_IN		0
/* vendor request out */
#define S2255_VR_OUT		1
/* firmware query */
#define S2255_VR_FW		0x30
/* USB endpoint number for configuring the device */
#define S2255_CONFIG_EP         2
/* maximum time for DSP to start responding after last FW word loaded(ms) */
#define S2255_DSP_BOOTTIME      800
/* maximum time to wait for firmware to load (ms) */
#define S2255_LOAD_TIMEOUT      (5000 + S2255_DSP_BOOTTIME)
#define S2255_DEF_BUFS          16
#define S2255_SETMODE_TIMEOUT   500
#define MAX_CHANNELS		4
#define S2255_MARKER_FRAME	0x2255DA4AL
#define S2255_MARKER_RESPONSE	0x2255ACACL
#define S2255_RESPONSE_SETMODE  0x01
#define S2255_RESPONSE_FW       0x10
#define S2255_USB_XFER_SIZE	(16 * 1024)
#define MAX_CHANNELS		4
#define MAX_PIPE_BUFFERS	1
#define SYS_FRAMES		4
/* maximum size is PAL full size plus room for the marker header(s) */
#define SYS_FRAMES_MAXSIZE	(720*288*2*2 + 4096)
#define DEF_USB_BLOCK		S2255_USB_XFER_SIZE
#define LINE_SZ_4CIFS_NTSC	640
#define LINE_SZ_2CIFS_NTSC	640
#define LINE_SZ_1CIFS_NTSC	320
#define LINE_SZ_4CIFS_PAL	704
#define LINE_SZ_2CIFS_PAL	704
#define LINE_SZ_1CIFS_PAL	352
#define NUM_LINES_4CIFS_NTSC	240
#define NUM_LINES_2CIFS_NTSC	240
#define NUM_LINES_1CIFS_NTSC	240
#define NUM_LINES_4CIFS_PAL	288
#define NUM_LINES_2CIFS_PAL	288
#define NUM_LINES_1CIFS_PAL	288
#define LINE_SZ_DEF		640
#define NUM_LINES_DEF		240


/* predefined settings */
#define FORMAT_NTSC	1
#define FORMAT_PAL	2

#define SCALE_4CIFS	1	/* 640x480(NTSC) or 704x576(PAL) */
#define SCALE_2CIFS	2	/* 640x240(NTSC) or 704x288(PAL) */
#define SCALE_1CIFS	3	/* 320x240(NTSC) or 352x288(PAL) */
/* SCALE_4CIFSI is the 2 fields interpolated into one */
#define SCALE_4CIFSI	4	/* 640x480(NTSC) or 704x576(PAL) high quality */

#define COLOR_YUVPL	1	/* YUV planar */
#define COLOR_YUVPK	2	/* YUV packed */
#define COLOR_Y8	4	/* monochrome */
#define COLOR_JPG       5       /* JPEG */
#define MASK_COLOR      0xff
#define MASK_JPG_QUALITY 0xff00

/* frame decimation. Not implemented by V4L yet(experimental in V4L) */
#define FDEC_1		1	/* capture every frame. default */
#define FDEC_2		2	/* capture every 2nd frame */
#define FDEC_3		3	/* capture every 3rd frame */
#define FDEC_5		5	/* capture every 5th frame */

/*-------------------------------------------------------
 * Default mode parameters.
 *-------------------------------------------------------*/
#define DEF_SCALE	SCALE_4CIFS
#define DEF_COLOR	COLOR_YUVPL
#define DEF_FDEC	FDEC_1
#define DEF_BRIGHT	0
#define DEF_CONTRAST	0x5c
#define DEF_SATURATION	0x80
#define DEF_HUE		0

/* usb config commands */
#define IN_DATA_TOKEN	0x2255c0de
#define CMD_2255	0xc2255000
#define CMD_SET_MODE	(CMD_2255 | 0x10)
#define CMD_START	(CMD_2255 | 0x20)
#define CMD_STOP	(CMD_2255 | 0x30)
#define CMD_STATUS	(CMD_2255 | 0x40)

struct s2255_mode {
	u32 format;	/* input video format (NTSC, PAL) */
	u32 scale;	/* output video scale */
	u32 color;	/* output video color format */
	u32 fdec;	/* frame decimation */
	u32 bright;	/* brightness */
	u32 contrast;	/* contrast */
	u32 saturation;	/* saturation */
	u32 hue;	/* hue (NTSC only)*/
	u32 single;	/* capture 1 frame at a time (!=0), continuously (==0)*/
	u32 usb_block;	/* block size. should be 4096 of DEF_USB_BLOCK */
	u32 restart;	/* if DSP requires restart */
};


#define S2255_READ_IDLE		0
#define S2255_READ_FRAME	1

/* frame structure */
struct s2255_framei {
	unsigned long size;
	unsigned long ulState;	/* ulState:S2255_READ_IDLE, S2255_READ_FRAME*/
	void *lpvbits;		/* image data */
	unsigned long cur_size;	/* current data copied to it */
};

/* image buffer structure */
struct s2255_bufferi {
	unsigned long dwFrames;			/* number of frames in buffer */
	struct s2255_framei frame[SYS_FRAMES];	/* array of FRAME structures */
};

#define DEF_MODEI_NTSC_CONT	{FORMAT_NTSC, DEF_SCALE, DEF_COLOR,	\
			DEF_FDEC, DEF_BRIGHT, DEF_CONTRAST, DEF_SATURATION, \
			DEF_HUE, 0, DEF_USB_BLOCK, 0}

struct s2255_dmaqueue {
	struct list_head	active;
	struct s2255_dev	*dev;
	int			channel;
};

/* for firmware loading, fw_state */
#define S2255_FW_NOTLOADED	0
#define S2255_FW_LOADED_DSPWAIT	1
#define S2255_FW_SUCCESS	2
#define S2255_FW_FAILED		3
#define S2255_FW_DISCONNECTING  4

#define S2255_FW_MARKER		cpu_to_le32(0x22552f2f)
/* 2255 read states */
#define S2255_READ_IDLE         0
#define S2255_READ_FRAME        1
struct s2255_fw {
	int		      fw_loaded;
	int		      fw_size;
	struct urb	      *fw_urb;
	atomic_t	      fw_state;
	void		      *pfw_data;
	wait_queue_head_t     wait_fw;
	const struct firmware *fw;
};

struct s2255_pipeinfo {
	u32 max_transfer_size;
	u32 cur_transfer_size;
	u8 *transfer_buffer;
	u32 state;
	void *stream_urb;
	void *dev;	/* back pointer to s2255_dev struct*/
	u32 err_count;
	u32 idx;
};

struct s2255_fmt; /*forward declaration */

struct s2255_dev {
	int			frames;
	int			users[MAX_CHANNELS];
	struct mutex		lock;
	struct mutex		open_lock;
	int			resources[MAX_CHANNELS];
	struct usb_device	*udev;
	struct usb_interface	*interface;
	u8			read_endpoint;

	struct s2255_dmaqueue	vidq[MAX_CHANNELS];
	struct video_device	*vdev[MAX_CHANNELS];
	struct list_head	s2255_devlist;
	struct timer_list	timer;
	struct s2255_fw	*fw_data;
	struct s2255_pipeinfo	pipes[MAX_PIPE_BUFFERS];
	struct s2255_bufferi		buffer[MAX_CHANNELS];
	struct s2255_mode	mode[MAX_CHANNELS];
	/* jpeg compression */
	struct v4l2_jpegcompression jc[MAX_CHANNELS];
	/* capture parameters (for high quality mode full size) */
	struct v4l2_captureparm cap_parm[MAX_CHANNELS];
	const struct s2255_fmt	*cur_fmt[MAX_CHANNELS];
	int			cur_frame[MAX_CHANNELS];
	int			last_frame[MAX_CHANNELS];
	u32			cc;	/* current channel */
	int			b_acquire[MAX_CHANNELS];
	/* allocated image size */
	unsigned long		req_image_size[MAX_CHANNELS];
	/* received packet size */
	unsigned long		pkt_size[MAX_CHANNELS];
	int			bad_payload[MAX_CHANNELS];
	unsigned long		frame_count[MAX_CHANNELS];
	int			frame_ready;
	/* if JPEG image */
	int                     jpg_size[MAX_CHANNELS];
	/* if channel configured to default state */
	int                     chn_configured[MAX_CHANNELS];
	wait_queue_head_t       wait_setmode[MAX_CHANNELS];
	int                     setmode_ready[MAX_CHANNELS];
	int                     chn_ready;
	struct kref		kref;
	spinlock_t              slock;
};
#define to_s2255_dev(d) container_of(d, struct s2255_dev, kref)

struct s2255_fmt {
	char *name;
	u32 fourcc;
	int depth;
};

/* buffer for one video frame */
struct s2255_buffer {
	/* common v4l buffer stuff -- must be first */
	struct videobuf_buffer vb;
	const struct s2255_fmt *fmt;
};

struct s2255_fh {
	struct s2255_dev	*dev;
	const struct s2255_fmt	*fmt;
	unsigned int		width;
	unsigned int		height;
	struct videobuf_queue	vb_vidq;
	enum v4l2_buf_type	type;
	int			channel;
	/* mode below is the desired mode.
	   mode in s2255_dev is the current mode that was last set */
	struct s2255_mode	mode;
	int			resources[MAX_CHANNELS];
};

/* current cypress EEPROM firmware version */
#define S2255_CUR_USB_FWVER	((3 << 8) | 6)
#define S2255_MAJOR_VERSION	1
#define S2255_MINOR_VERSION	14
#define S2255_RELEASE		0
#define S2255_VERSION		KERNEL_VERSION(S2255_MAJOR_VERSION, \
					       S2255_MINOR_VERSION, \
					       S2255_RELEASE)

/* vendor ids */
#define USB_S2255_VENDOR_ID	0x1943
#define USB_S2255_PRODUCT_ID	0x2255
#define S2255_NORMS		(V4L2_STD_PAL | V4L2_STD_NTSC)
/* frame prefix size (sent once every frame) */
#define PREFIX_SIZE		512

/* Channels on box are in reverse order */
static unsigned long G_chnmap[MAX_CHANNELS] = {3, 2, 1, 0};

static LIST_HEAD(s2255_devlist);

static int debug;
static int *s2255_debug = &debug;

static int s2255_start_readpipe(struct s2255_dev *dev);
static void s2255_stop_readpipe(struct s2255_dev *dev);
static int s2255_start_acquire(struct s2255_dev *dev, unsigned long chn);
static int s2255_stop_acquire(struct s2255_dev *dev, unsigned long chn);
static void s2255_fillbuff(struct s2255_dev *dev, struct s2255_buffer *buf,
			   int chn, int jpgsize);
static int s2255_set_mode(struct s2255_dev *dev, unsigned long chn,
			  struct s2255_mode *mode);
static int s2255_board_shutdown(struct s2255_dev *dev);
static void s2255_exit_v4l(struct s2255_dev *dev);
static void s2255_fwload_start(struct s2255_dev *dev, int reset);
static void s2255_destroy(struct kref *kref);
static long s2255_vendor_req(struct s2255_dev *dev, unsigned char req,
			     u16 index, u16 value, void *buf,
			     s32 buf_len, int bOut);

/* dev_err macro with driver name */
#define S2255_DRIVER_NAME "s2255"
#define s2255_dev_err(dev, fmt, arg...)					\
		dev_err(dev, S2255_DRIVER_NAME " - " fmt, ##arg)

#define dprintk(level, fmt, arg...)					\
	do {								\
		if (*s2255_debug >= (level)) {				\
			printk(KERN_DEBUG S2255_DRIVER_NAME		\
				": " fmt, ##arg);			\
		}							\
	} while (0)

static struct usb_driver s2255_driver;


/* Declare static vars that will be used as parameters */
static unsigned int vid_limit = 16;	/* Video memory limit, in Mb */

/* start video number */
static int video_nr = -1;	/* /dev/videoN, -1 for autodetect */

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Debug level(0-100) default 0");
module_param(vid_limit, int, 0644);
MODULE_PARM_DESC(vid_limit, "video memory limit(Mb)");
module_param(video_nr, int, 0644);
MODULE_PARM_DESC(video_nr, "start video minor(-1 default autodetect)");

/* USB device table */
static struct usb_device_id s2255_table[] = {
	{USB_DEVICE(USB_S2255_VENDOR_ID, USB_S2255_PRODUCT_ID)},
	{ }			/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, s2255_table);


#define BUFFER_TIMEOUT msecs_to_jiffies(400)

/* supported controls */
static struct v4l2_queryctrl s2255_qctrl[] = {
	{
	.id = V4L2_CID_BRIGHTNESS,
	.type = V4L2_CTRL_TYPE_INTEGER,
	.name = "Brightness",
	.minimum = -127,
	.maximum = 128,
	.step = 1,
	.default_value = 0,
	.flags = 0,
	}, {
	.id = V4L2_CID_CONTRAST,
	.type = V4L2_CTRL_TYPE_INTEGER,
	.name = "Contrast",
	.minimum = 0,
	.maximum = 255,
	.step = 0x1,
	.default_value = DEF_CONTRAST,
	.flags = 0,
	}, {
	.id = V4L2_CID_SATURATION,
	.type = V4L2_CTRL_TYPE_INTEGER,
	.name = "Saturation",
	.minimum = 0,
	.maximum = 255,
	.step = 0x1,
	.default_value = DEF_SATURATION,
	.flags = 0,
	}, {
	.id = V4L2_CID_HUE,
	.type = V4L2_CTRL_TYPE_INTEGER,
	.name = "Hue",
	.minimum = 0,
	.maximum = 255,
	.step = 0x1,
	.default_value = DEF_HUE,
	.flags = 0,
	}
};

static int qctl_regs[ARRAY_SIZE(s2255_qctrl)];

/* image formats.  */
static const struct s2255_fmt formats[] = {
	{
		.name = "4:2:2, planar, YUV422P",
		.fourcc = V4L2_PIX_FMT_YUV422P,
		.depth = 16

	}, {
		.name = "4:2:2, packed, YUYV",
		.fourcc = V4L2_PIX_FMT_YUYV,
		.depth = 16

	}, {
		.name = "4:2:2, packed, UYVY",
		.fourcc = V4L2_PIX_FMT_UYVY,
		.depth = 16
	}, {
		.name = "JPG",
		.fourcc = V4L2_PIX_FMT_JPEG,
		.depth = 24
	}, {
		.name = "8bpp GREY",
		.fourcc = V4L2_PIX_FMT_GREY,
		.depth = 8
	}
};

static int norm_maxw(struct video_device *vdev)
{
	return (vdev->current_norm & V4L2_STD_NTSC) ?
	    LINE_SZ_4CIFS_NTSC : LINE_SZ_4CIFS_PAL;
}

static int norm_maxh(struct video_device *vdev)
{
	return (vdev->current_norm & V4L2_STD_NTSC) ?
	    (NUM_LINES_1CIFS_NTSC * 2) : (NUM_LINES_1CIFS_PAL * 2);
}

static int norm_minw(struct video_device *vdev)
{
	return (vdev->current_norm & V4L2_STD_NTSC) ?
	    LINE_SZ_1CIFS_NTSC : LINE_SZ_1CIFS_PAL;
}

static int norm_minh(struct video_device *vdev)
{
	return (vdev->current_norm & V4L2_STD_NTSC) ?
	    (NUM_LINES_1CIFS_NTSC) : (NUM_LINES_1CIFS_PAL);
}


/*
 * TODO: fixme: move YUV reordering to hardware
 * converts 2255 planar format to yuyv or uyvy
 */
static void planar422p_to_yuv_packed(const unsigned char *in,
				     unsigned char *out,
				     int width, int height,
				     int fmt)
{
	unsigned char *pY;
	unsigned char *pCb;
	unsigned char *pCr;
	unsigned long size = height * width;
	unsigned int i;
	pY = (unsigned char *)in;
	pCr = (unsigned char *)in + height * width;
	pCb = (unsigned char *)in + height * width + (height * width / 2);
	for (i = 0; i < size * 2; i += 4) {
		out[i] = (fmt == V4L2_PIX_FMT_YUYV) ? *pY++ : *pCr++;
		out[i + 1] = (fmt == V4L2_PIX_FMT_YUYV) ? *pCr++ : *pY++;
		out[i + 2] = (fmt == V4L2_PIX_FMT_YUYV) ? *pY++ : *pCb++;
		out[i + 3] = (fmt == V4L2_PIX_FMT_YUYV) ? *pCb++ : *pY++;
	}
	return;
}

static void s2255_reset_dsppower(struct s2255_dev *dev)
{
	s2255_vendor_req(dev, 0x40, 0x0b0b, 0x0b0b, NULL, 0, 1);
	msleep(10);
	s2255_vendor_req(dev, 0x50, 0x0000, 0x0000, NULL, 0, 1);
	return;
}

/* kickstarts the firmware loading. from probe
 */
static void s2255_timer(unsigned long user_data)
{
	struct s2255_fw *data = (struct s2255_fw *)user_data;
	dprintk(100, "s2255 timer\n");
	if (usb_submit_urb(data->fw_urb, GFP_ATOMIC) < 0) {
		printk(KERN_ERR "s2255: can't submit urb\n");
		atomic_set(&data->fw_state, S2255_FW_FAILED);
		/* wake up anything waiting for the firmware */
		wake_up(&data->wait_fw);
		return;
	}
}


/* this loads the firmware asynchronously.
   Originally this was done synchroously in probe.
   But it is better to load it async