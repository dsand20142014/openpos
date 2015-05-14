/*
 *  valkyriefb.c -- frame buffer device for the PowerMac 'valkyrie' display
 *
 *  Created 8 August 1998 by 
 *  Martin Costabel <costabel@wanadoo.fr> and Kevin Schoedel
 *
 *  Vmode-switching changes and vmode 15/17 modifications created 29 August
 *  1998 by Barry K. Nathan <barryn@pobox.com>.
 *
 *  Ported to m68k Macintosh by David Huggins-Daines <dhd@debian.org>
 *
 *  Derived directly from:
 *
 *   controlfb.c -- frame buffer device for the PowerMac 'control' display
 *   Copyright (C) 1998 Dan Jacobowitz <dan@debian.org>
 *
 *   pmc-valkyrie.c -- Console support for PowerMac "valkyrie" display adaptor.
 *   Copyright (C) 1997 Paul Mackerras.
 *
 *  and indirectly:
 *
 *  Frame buffer structure from:
 *    drivers/video/chipsfb.c -- frame buffer device for
 *    Chips & Technologies 65550 chip.
 *
 *    Copyright (C) 1998 Paul Mackerras
 *
 *    This file is derived from the Powermac "chips" driver:
 *    Copyright (C) 1997 Fabio Riccardi.
 *    And from the frame buffer device for Open Firmware-initialized devices:
 *    Copyright (C) 1997 Geert Uytterhoeven.
 *
 *  Hardware information from:
 *    control.c: Console support for PowerMac "control" display adaptor.
 *    Copyright (C) 1996 Paul Mackerras
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/selection.h>
#include <linux/init.h>
#include <linux/nvram.h>
#include <linux/adb.h>
#include <linux/cuda.h>
#include <asm/io.h>
#ifdef CONFIG_MAC
#include <asm/bootinfo.h>
#include <asm/macintosh.h>
#else
#include <asm/prom.h>
#endif
#include <asm/pgtable.h>

#include "macmodes.h"
#include "valkyriefb.h"

#ifdef CONFIG_MAC
/* We don't yet have functions to read the PRAM... perhaps we can
   adapt them from the PPC code? */
static int default_vmode = VMODE_640_480_67;
static int default_cmode = CMODE_8;
#else
static int default_vmode = VMODE_NVRAM;
static int default_cmode = CMODE_NVRAM;
#endif

struct fb_par_valkyrie {
	int	vmode, cmode;
	int	xres, yres;
	int	vxres, vyres;
	struct valkyrie_regvals *init;
};

struct fb_info_valkyrie {
	struct fb_info		info;
	struct fb_par_valkyrie	par;
	struct cmap_regs	__iomem *cmap_regs;
	unsigned long		cmap_regs_phys;
	
	struct valkyrie_regs	__iomem *valkyrie_regs;
	unsigned long		valkyrie_regs_phys;
	
	__u8			__iomem *frame_buffer;
	unsigned long		frame_buffer_phys;
	
	int			sense;
	unsigned long		total_vram;

	u32			pseudo_palette[16];
};

/*
 * Exported functions
 */
int valkyriefb_init(void);
int valkyriefb_setup(char*);

static int valkyriefb_check_var(struct fb_var_screeninfo *var,
				struct fb_info *info);
static int valkyriefb_set_par(struct fb_info *info);
static int valkyriefb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			     u_int transp, struct fb_info *info);
static int valkyriefb_blank(int blank_mode, struct fb_info *info);

static int read_valkyrie_sense(struct fb_info_valkyrie *p);
static void set_valkyrie_clock(unsigned char *params);
static int valkyrie_var_to_par(struct fb_var_screeninfo *var,
	struct fb_par_valkyrie *par, const struct fb_info *fb_info);

static int valkyrie_init_info(struct fb_info *info, struct fb_info_valkyrie *p);
static void valkyrie_par_to_fix(struct fb_par_valkyrie *par, struct fb_fix_screeninfo *fix);
static void valkyrie_init_fix(struct fb_fix_screeninfo *fix, struct fb_info_valkyrie *p);

static struct fb_ops valkyriefb_ops = {
	.owner =	THIS_MODULE,
	.fb_check_var =	valkyriefb_check_var,
	.fb_set_par =	valkyriefb_set_par,
	.fb_setcolreg =	valkyriefb_setcolreg,
	.fb_blank =	valkyriefb_blank,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/* Sets the video mode according to info->var */
static int valkyriefb_set_par(struct fb_info *info)
{
	struct fb_info_valkyrie *p = (struct fb_info_valkyrie *) info;
	volatile struct valkyrie_regs __iomem *valkyrie_regs = p->valkyrie_regs;
	struct fb_par_valkyrie *par = info->par;
	struct valkyrie_regvals	*init;
	int err;

	if ((err = valkyrie_var_to_par(&info->var, par, info)))
		return err;

	valkyrie_par_to_fix(par, &info->fix);

	/* Reset the valkyrie */
	out_8(&valkyrie_regs->status.r, 0);
	udelay(100);

	/* Initialize display timing registers */
	init = par->init;
	out_8(&valkyrie_regs->mode.r, init->mode | 0x80);
	out_8(&valkyrie_regs->depth.r, par->cmode + 3);
	set_valkyrie_clock(init->clock_params);
	udelay(100);

	/* Turn on display */
	out_8(&valkyrie_regs->mode.r, init->mode);

	return 0;
}

static inline int valkyrie_par