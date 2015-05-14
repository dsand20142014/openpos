/*
 * linux/drivers/video/hitfb.c -- Hitachi LCD frame buffer device
 *
 * (C) 1999 Mihai Spatar
 * (C) 2000 YAEGASHI Takeshi
 * (C) 2003, 2004 Paul Mundt
 * (C) 2003, 2004, 2006 Andriy Skulysh
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
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>

#include <asm/machvec.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/hd64461.h>
#include <cpu/dac.h>

#define	WIDTH 640

static struct fb_var_screeninfo hitfb_var __initdata = {
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.vmode		= FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo hitfb_fix __initdata = {
	.id		= "Hitachi HD64461",
	.type		= FB_TYPE_PACKED_PIXELS,
	.accel		= FB_ACCEL_NONE,
};

static inline void hitfb_accel_wait(void)
{
	while (fb_readw(HD64461_GRCFGR) & HD64461_GRCFGR_ACCSTATUS) ;
}

static inline void hitfb_accel_start(int truecolor)
{
	if (truecolor) {
		fb_writew(6, HD64461_GRCFGR);
	} else {
		fb_writew(7, HD64461_GRCFGR);
	}
}

static inline void hitfb_accel_set_dest(int truecolor, u16 dx, u16 dy,
					u16 width, u16 height)
{
	u32 saddr = WIDTH * dy + dx;
	if (truecolor)
		saddr <<= 1;

	fb_writew(width-1, HD64461_BBTDWR);
	fb_writew(height-1, HD64461_BBTDHR);

	fb_writew(saddr & 0xffff, HD64461_BBTDSARL);
	fb_writew(saddr >> 16, HD64461_BBTDSARH);

}

static inline void hitfb_accel_bitblt(int truecolor, u16 sx, u16 sy, u16 dx,
				      u16 dy, u16 width, u16 height, u16 rop,
				      u32 mask_addr)
{
	u32 saddr, daddr;
	u32 maddr = 0;

	height--;
	width--;
	fb_writew(rop, HD64461_BBTROPR);
	if ((sy < dy) || ((sy == dy) && (sx <= dx))) {
		saddr = WIDTH * (sy + height) + sx + width;
		daddr = WIDTH * (dy + height) + dx + width;
		if (mask_addr) {
			if (truecolor)
				maddr = ((width >> 3) + 1) * (height + 1) - 1;
			else
				maddr =
				    (((width >> 4) + 1) * (height + 1) - 1) * 2;

			fb_writew((1 << 5) | 1, HD64461_BBTMDR);
		} else
			fb_writew(1, HD64461_BBTMDR);
	} else {
		saddr = WIDTH * sy + sx;
		daddr = WIDTH * dy + dx;
		if (mask_addr) {
			fb_writew((1 << 5), HD64461_BBTMDR);
		} else {
			fb_writew(0, HD64461_BBTMDR);
		}
	}
	if (truecolor) {
		saddr <<= 1;
		daddr <<= 1;
	}
	fb_writew(width, HD64461_BBTDWR);
	fb_writew(height, HD64461_BBTDHR);
	fb_writew(saddr & 0xffff, HD64461_BBTSSARL);
	fb_writew(saddr >> 16, HD64461_BBTSSARH);
	fb_writew(daddr & 0xffff, HD64461_BBTDSARL);
	fb_writew(daddr >> 16, HD64461_BBTDSARH);
	if (mask_addr) {
		maddr += mask_addr;
		fb_writew(maddr & 0xffff, HD64461_BBTMARL);
		fb_writew(maddr >> 16, HD64461_BBTMARH);
	}
	hitfb_accel_start(truecolor);
}

static void hitfb_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
	if (rect->rop != ROP_COPY)
		cfb_fillrect(p, rect);
	else {
		hitfb_accel_wait();
		fb_writew(0x00f0, HD64461_BBTROPR);
		fb_writew(16, HD64461_BBTMDR);

		if (p->var.bits_per_pixel == 16) {
			fb_writew(((u32 *) (p->pseudo_palette))[rect->color],
				  HD64461_GRSCR);
			hitfb_accel_set_dest(1, rect->dx, rect->dy, rect->width,
					     rect->height);
			hitfb_accel_start(1);
		} else {
			fb_writew(rect->color, HD64461_GRSCR);
			hitfb_accel_set_dest(0, rect->dx, rect->dy, rect->width,
					     rect->height);
			hitfb_accel_start(0);
		}
	}
}

static void hitfb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
	hitfb_accel_wait();
	hitfb_accel_bitblt(p->var.bits_per_pixel == 16, area->sx, area->sy,
			   area->dx, area->dy, area->width, area->height,
			   0x00cc, 0);
}

static int hitfb_pan_display(struct fb_var_screeninfo *var,
			     struct fb_info *info)
{
	int xoffset = var->xoffset;
	int yoffset = var->yoffset;

	if (xoffset != 0)
		return -EINVAL;

	fb_writew((yoffset*info->fix.line_length)>>10, HD64461_LCDCBAR);

	return 0;
}

int hitfb_blank(int blank_mode, struct fb_info *info)
{
	unsigned short v;

	if (blank_mode) {
		v = fb_readw(HD64461_LDR1);
		v &= ~HD64461_LDR1_DON;
		fb_writew(v, HD64461_LDR1);

		v = fb_readw(HD64461_LCDCCR);
		v |= HD64461_LCDCCR_MOFF;
		fb_writew(v, HD64461_LCDCCR);

		v = fb_readw(HD64461_STBCR);
		v |= HD64461_STBCR_SLCDST