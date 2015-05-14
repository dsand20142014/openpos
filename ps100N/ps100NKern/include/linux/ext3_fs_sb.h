/* p9100.c: P9100 frame buffer driver
 *
 * Copyright (C) 2003, 2006 David S. Miller (davem@davemloft.net)
 * Copyright 1999 Derrick J Brashear (shadow@dementia.org)
 *
 * Driver layout based loosely on tgafb.c, see that file for credits.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>
#include <asm/fbio.h>

#include "sbuslib.h"

/*
 * Local functions.
 */

static int p9100_setcolreg(unsigned, unsigned, unsigned, unsigned,
			   unsigned, struct fb_info *);
static int p9100_blank(int, struct fb_info *);

static int p9100_mmap(struct fb_info *, struct vm_area_struct *);
static int p9100_ioctl(struct fb_info *, unsigned int, unsigned long);

/*
 *  Frame buffer operations
 */

static struct fb_ops p9100_ops = {
	.owner			= THIS_MODULE,
	.fb_setcolreg		= p9100_setcolreg,
	.fb_blank		= p9100_blank,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
	.fb_mmap		= p9100_mmap,
	.fb_ioctl		= p9100_ioctl,
#ifdef CONFIG_COMPAT
	.fb_compat_ioctl	= sbusfb_compat_ioctl,
#endif
};

/* P9100 control registers */
#define P9100_SYSCTL_OFF	0x0UL
#define P9100_VIDEOCTL_OFF	0x100UL
#define P9100_VRAMCTL_OFF 	0x180UL
#define P9100_RAMDAC_OFF 	0x200UL
#define P9100_VIDEOCOPROC_OFF 	0x400UL

/* P9100 command registers */
#define P9100_CMD_OFF 0x0UL

/* P9100 framebuffer memory */
#define P9100_FB_OFF 0x0UL

/* 3 bits: 2=8bpp 3=16bpp 5=32bpp 7=24bpp */
#define SYS_CONFIG_PIXELSIZE_SHIFT 26 

#define SCREENPAINT_TIMECTL1_ENABLE_VIDEO 0x20 /* 0 = off, 1 = on */

struct p9100_regs {
	/* Registers for the system control */
	u32 sys_base;
	u32 sys_config;
	u32 sys_intr;
	u32 sys_int_ena;
	u32 sys_alt_rd;
	u32 sys_alt_wr;
	u32 sys_xxx[58];

	/* Registers for the video control */
	u32 vid_base;
	u32 vid_hcnt;
	u32 vid_htotal;
	u32 vid_hsync_rise;
	u32 vid_hblank_rise;
	u32 vid_hblank_fall;
	u32 vid_hcnt_preload;
	u32 vid_vcnt;
	u32 vid_vlen;
	u32 vid_vsync_rise;
	u32 vid_vblank_rise;
	u32 vid_vblank_fall;
	u32 vid_vcnt_preload;
	u32 vid_screenpaint_addr;
	u32 vid_screenpaint_timectl1;
	u32 vid_screenpaint_qsfcnt;
	u32 vid_screenpaint_timectl2;
	u32 vid_xxx[15];

	/* Registers for the video control */
	u32 vram_base;
	u32 vram_memcfg;
	u32 vram_refresh_pd;
	u32 vram_refresh_cnt;
	u32 vram_raslo_max;
	u32 vram_raslo_cur;
	u32 pwrup_cfg;
	u32 vram_xxx[25];

	/* Registers for IBM RGB528 Palette */
	u32 ramdac_cmap_wridx; 
	u32 ramdac_palette_data;
	u32 ramdac_pixel_mask;
	u32 ramdac_palette_rdaddr;
	u32 ramdac_idx_lo;
	u32 ramdac_idx_hi;
	u32 ramdac_idx_data;
	u32 ramdac_idx_ctl;
	u32 ramdac_xxx[1784];
};

struct p9100_cmd_parameng {
	u32 parameng_status;
	u32 parameng_bltcmd;
	u32 parameng_quadcmd;
};

struct p9100_par {
	spinlock_t		lock;
	struct p9100_regs	__iomem *regs;

	u32			flags;
#define P9100_FLAG_BLANKED	0x00000001

	unsigned long		which_io;
};

/**
 *      p9100_setcolreg - Optional f