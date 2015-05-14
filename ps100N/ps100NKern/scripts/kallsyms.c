 
			   const struct pci_device_id *entry)
{
	int err;

	if ((err = pci_enable_device(par->dev))) { 
		printk("i810fb_init: cannot enable device\n");
		return err;		
	}
	par->res_flags |= PCI_DEVICE_ENABLED;

	if (pci_resource_len(par->dev, 0) > 512 * 1024) {
		par->aperture.physical = pci_resource_start(par->dev, 0);
		par->aperture.size = pci_resource_len(par->dev, 0);
		par->mmio_start_phys = pci_resource_start(par->dev, 1);
	} else {
		par->aperture.physical = pci_resource_start(par->dev, 1);
		par->aperture.size = pci_resource_len(par->dev, 1);
		par->mmio_start_phys = pci_resource_start(par->dev, 0);
	}
	if (!par->aperture.size) {
		printk("i810fb_init: device is disabled\n");
		return -ENOMEM;
	}

	if (!request_mem_region(par->aperture.physical, 
				par->aperture.size, 
				i810_pci_list[entry->driver_data])) {
		printk("i810fb_init: cannot request framebuffer region\n");
		return -ENODEV;
	}
	par->res_flags |= FRAMEBUFFER_REQ;

	par->aperture.virtual = ioremap_nocache(par->aperture.physical, 
					par->aperture.size);
	if (!par->aperture.virtual) {
		printk("i810fb_init: cannot remap framebuffer region\n");
		return -ENODEV;
	}
  
	if (!request_mem_region(par->mmio_start_phys, 
				MMIO_SIZE, 
				i810_pci_list[entry->driver_data])) {
		printk("i810fb_init: cannot request mmio region\n");
		return -ENODEV;
	}
	par->res_flags |= MMIO_REQ;

	par->mmio_start_virtual = ioremap_nocache(par->mmio_start_phys, 
						  MMIO_SIZE);
	if (!par->mmio_start_virtual) {
		printk("i810fb_init: cannot remap mmio region\n");
		return -ENODEV;
	}

	return 0;
}

static void __devinit i810fb_find_init_mode(struct fb_info *info)
{
	struct fb_videomode mode;
	struct fb_var_screeninfo var;
	struct fb_monspecs *specs = &info->monspecs;
	int found = 0;
#ifdef CONFIG_FB_I810_I2C
	int i;
	int err = 1;
	struct i810fb_par *par = info->par;
#endif

	INIT_LIST_HEAD(&info->modelist);
	memset(&mode, 0, sizeof(struct fb_videomode));
	var = info->var;
#ifdef CONFIG_FB_I810_I2C
	i810_create_i2c_busses(par);

	for (i = 0; i < par->ddc_num + 1; i++) {
		err = i810_probe_i2c_connector(info, &par->edid, i);
		if (!err)
			break;
	}

	if (!err)
		printk("i810fb_init_pci: DDC probe successful\n");

	fb_edid_to_monspecs(par->edid, specs);

	if (specs->modedb == NULL)
		printk("i810fb_init_pci: Unable to get Mode Database\n");

	fb_videomode_to_modelist(specs->modedb, specs->modedb_len,
				 &info->modelist);
	if (specs->modedb != NULL) {
		const struct fb_videomode *m;

		if (xres && yres) {
			if ((m = fb_find_best_mode(&var, &info->modelist))) {
				mode = *m;
				found  = 1;
			}
		}

		if (!found) {
			m = fb_find_best_display(&info->monspecs, &info->modelist);
			mode = *m;
			found = 1;
		}

		fb_videomode_to_var(&var, &mode);
	}
#endif
	if (mode_option)
		fb_find_mode(&var, info, mode_option, specs->modedb,
			     specs->modedb_len, (found) ? &mode : NULL,
			     info->var.bits_per_pixel);

	info->var = var;
	fb_destroy_modedb(specs->modedb);
	specs->modedb = NULL;
}

#ifndef MODULE
static int __devinit i810fb_setup(char *options)
{
	char *this_opt, *suffix = NULL;

	if (!options || !*options)
		return 0;
	
	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!strncmp(this_opt, "mtrr", 4))
			mtrr = 1;
		else if (!strncmp(this_opt, "accel", 5))
			accel = 1;
		else if (!strncmp(this_opt, "extvga", 6))
			extvga = 1;
		else if (!strncmp(this_opt, "sync", 4))
			sync = 1;
		else if (!strncmp(this_opt, "vram:", 5))
			vram = (simple_strtoul(this_opt+5, NULL, 0));
		else if (!strncmp(this_opt, "voffset:", 8))
			voffset = (simple_strtoul(this_opt+8, NULL, 0));
		else if (!strncmp(this_opt, "xres:", 5))
			xres = simple_strtoul(this_opt+5, NULL, 0);
		else if (!strncmp(this_opt, "yres:", 5))
			yres = simple_strtoul(this_opt+5, NULL, 0);
		else if (!strncmp(this_opt, "vyres:", 6))
			vyres = simple_strtoul(this_opt+6, NULL, 0);
		else if (!strncmp(this_opt, "bpp:", 4))
			bpp = simple_strtoul(this_opt+4, NULL, 0);
		else if (!strncmp(this_opt, "hsync1:", 7)) {
			hsync1 = simple_strtoul(this_opt+7, &suffix, 0);
			if (strncmp(suffix, "H", 1)) 
				hsync1 *= 1000;
		} else if (!strncmp(this_opt, "hsync2:", 7)) {
			hsync2 = simple_strtoul(this_opt+7, &suffix, 0);
			if (strncmp(suffix, "H", 1)) 
				hsync2 *= 1000;
		} else if (!strncmp(this_opt, "vsync1:", 7)) 
			vsync1 = simple_strtoul(this_opt+7, NULL, 0);
		else if (!strncmp(this_opt, "vsync2:", 7))
			vsync2 = simple_strtoul(this_opt+7, NULL, 0);
		else if (!strncmp(this_opt, "dcolor", 6))
			dcolor = 1;
		else if (!strncmp(this_opt, "ddc3", 4))
			ddc3 = 3;
		else
			mode_option = this_opt;
	}
	return 0;
}
#endif

static int __devinit i810fb_init_pci (struct pci_dev *dev, 
				   const struct pci_device_id *entry)
{
	struct fb_info    *info;
	struct i810fb_par *par = NULL;
	struct fb_videomode mode;
	int i, err = -1, vfreq, hfreq, pixclock;

	i = 0;

	info = framebuffer_alloc(sizeof(struct i810fb_par), &dev->dev);
	if (!info)
		return -ENOMEM;

	par = info->par;
	par->dev = dev;

	if (!(info->pixmap.addr = kzalloc(8*1024, GFP_KERNEL))) {
		i810fb_release_resource(info, par);
		return -ENOMEM;
	}
	info->pixmap.size = 8*1024;
	info->pixmap.buf_align = 8;
	info->pixmap.access_align = 32;
	info->pixmap.flags = FB_PIXMAP_SYSTEM;

	if ((err = i810_allocate_pci_resource(par, entry))) {
		i810fb_release_resource(info, par);
		return err;
	}

	i810_init_defaults(par, info);

	if ((err = i810_alloc_agp_mem(info))) {
		i810fb_release_resource(info, par);
		return err;
	}

	i810_init_device(par);        

	info->screen_base = par->fb.virtual;
	info->fbops = &par->i810fb_ops;
	info->pseudo_palette = par->pseudo_palette;
	fb_alloc_cmap(&info->cmap, 256, 0);
	i810fb_find_init_mode(info);

	if ((err = info->fbops->fb_check_var(&info->var, info))) {
		i810fb_release_resource(info, par);
		return err;
	}

	fb_var_to_videomode(&mode, &info->var);
	fb_add_videomode(&mode, &info->modelist);

	i810fb_init_ringbuffer(info);
	err = register_framebuffer(info);

	if (err < 0) {
    		i810fb_release_resource(info, par); 
		printk("i810fb_init: cannot register framebuffer device\n");
    		return err;  
    	}   

	pci_set_drvdata(dev, info);
	pixclock = 1000000000/(info->var.pixclock);
	pixclock *= 1000;
	hfreq = pixclock/(info->var.xres + info->var.left_margin + 
			  info->var.hsync_len + info->var.right_margin);
	vfreq = hfreq/(info->var.yres + info->var.upper_margin +
		       info->var.vsync_len + info->var.lower_margin);

      	printk("I810FB: fb%d         : %s v%d.%d.%d%s\n"
      	       "I810FB: Video RAM   : %dK\n" 
	       "I810FB: Monitor     : H: %d-%d KHz V: %d-%d Hz\n"
	       "I810FB: Mode        : %dx%d-%dbpp@%dHz\n",
	       info->node,
	       i810_pci_list[entry->driver_data],
	       VERSION_MAJOR, VERSION_MINOR, VERSION_TEENIE, BRANCH_VERSION,
	       (int) par->fb.size>>10, info->monspecs.hfmin/1000,
	       info->monspecs.hfmax/1000, info->monspecs.vfmin,
	       info->monspecs.vfmax, info->var.xres, 
	       info->var.yres, info->var.bits_per_pixel, vfreq);
	return 0;
}

/***************************************************************
 *                     De-initialization                        *
 ***************************************************************/

static void i810fb_release_resource(struct fb_info *info, 
				    struct i810fb_par *par)
{
	struct gtt_data *gtt = &par->i810_gtt;
	unset_mtrr(par);

	i810_delete_i2c_busses(par);

	if (par->i810_gtt.i810_cursor_memory)
		agp_free_memory(gtt->i810_cursor_memory);
	if (par->i810_gtt.i810_fb_memory)
		agp_free_memory(gtt->i810_fb_memory);

	if (par->mmio_start_virtual)
		iounmap(par->mmio_start_virtual);
	if (par->aperture.virtual)
		iounmap(par->aperture.virtual);
	kfree(par->edid);
	if (par->res_flags & FRAMEBUFFER_REQ)
		release_mem_region(par->aperture.physical,
				   par->aperture.size);
	if (par->res_flags & MMIO_REQ)
		release_mem_region(par->mmio_start_phys, MMIO_SIZE);

	framebuffer_release(info);

}

static void __exit i810fb_remove_pci(struct pci_dev *dev)
{
	struct fb_info *info = pci_get_drvdata(dev);
	struct i810fb_par *par = info->par;

	unregister_framebuffer(info);  
	i810fb_release_resource(info, par);
	pci_set_drvdata(dev, N].sym[0]) != 'A') {
			if (_text <= table[i].addr)
				printf("\tPTR\t_text + %#llx\n",
					table[i].addr - _text);
			else
				printf("\tPTR\t_text - %#llx\n",
					_text - table[i].addr);
		} else {
			printf("\tPTR\t%#llx\n", table[i].addr);
		}
	}
	printf("\n");

	output_label("kallsyms_num_syms");
	printf("\tPTR\t%d\n", table_cnt);
	printf("\n");

	/* table of offset markers, that give the offset in the compressed stream
	 * every 256 symbols */
	markers = malloc(sizeof(unsigned int) * ((table_cnt + 255) / 256));
	if (!markers) {
		fprintf(stderr, "kallsyms failure: "
			"unable to allocate required memory\n");
		exit(EXIT_FAILURE);
	}

	output_label("kallsyms_names");
	off = 0;
	for (i = 0; i < table_cnt; i++) {
		if ((i & 0xFF) == 0)
			markers[i >> 8] = off;

		printf("\t.byte 0x%02x", table[i].len);
		for (k = 0; k < table[i].len; k++)
			printf(", 0x%02x", table[i].sym[k]);
		printf("\n");

		off += table[i].len + 1;
	}
	printf("\n");

	output_label("kallsyms_markers");
	for (i = 0; i < ((table_cnt + 255) >> 8); i++)
		printf("\tPTR\t%d\n", markers[i]);
	printf("\n");

	free(markers);

	output_label("kallsyms_token_table");
	off = 0;
	for (i = 0; i < 256; i++) {
		best_idx[i] = off;
		expand_symbol(best_table[i], best_table_len[i], buf);
		printf("\t.asciz\t\"%s\"\n", buf);
		off += strlen(buf) + 1;
	}
	printf("\n");

	output_label("kallsyms_token_index");
	for (i = 0; i < 256; i++)
		printf("\t.short\t%d\n", best_idx[i]);
	printf("\n");
}


/* table lookup compression functions */

/* count all the possible tokens in a symbol */
static void learn_symbol(unsigned char *symbol, int len)
{
	int i;

	for (i = 0; i < len - 1; i++)
		token_profit[ symbol[i] + (symbol[i + 1] << 8) ]++;
}

/* decrease the count for all the possible tokens in a symbol */
static void forget_symbol(unsigned char *symbol, int len)
{
	int i;

	for (i = 0; i < len - 1; i++)
		token_profit[ symbol[i] + (symbol[i + 1] << 8) ]--;
}

/* remove all the invalid symbols from the table and do the initial token count */
static void build_initial_tok_table(void)
{
	unsigned int i, pos;

	pos = 0;
	for (i = 0; i < table_cnt; i++) {
		if ( symbol_valid(&table[i]) ) {
			if (pos != i)
				table[pos] = table[i];
			learn_symbol(table[pos].sym, table[pos].len);
			pos++;
		}
	}
	table_cnt = pos;
}

static void *find_token(unsigned char *str, int len, unsigned char *token)
{
	int i;

	for (i = 0; i < len - 1; i++) {
		if (str[i] == token[0] && str[i+1] == token[1])
			return &str[i];
	}
	return NULL;
}

/* replace a given token in all the valid symbols. Use the sampled symbols
 * to update the counts */
static void compress_symbols(unsigned char *str, int idx)
{
	unsigned int i, len, size;
	unsigned char *p1, *p2;

	for (i = 0; i < table_cnt; i++) {

		len = table[i].len;
		p1 = table[i].sym;

		/* find the token on the symbol */
		p2 = find_token(p1, len, str);
		if (!p2) continue;

		/* decrease the counts for this symbol's tokens */
		forget_symbol(table[i].sym, len);

		size = len;

		do {
			*p2 = idx;
			p2++;
			size -= (p2 - p1);
			memmove(p2, p2 + 1, size);
			p1 = p2;
			len--;

			if (size < 2) break;

			/* find the token on the symbol */
			p2 = find_token(p1, size, str);

		} while (p2);

		table[i].len = len;

		/* increase the counts for this symbol's new tokens */
		learn_symbol(table[i].sym, len);
	}
}

/* search the token with the maximum profit */
static int find_best_token(void)
{
	int i, best, bestprofit;

	bestprofit=-10000;
	best = 0;

	for (i = 0; i < 0x10000; i++) {
		if (token_profit[i] > bestprofit) {
			best = i;
			bestprofit = token_profit[i];
		}
	}
	return best;
}

/* this is the core of the algorithm: calculate the "best" table */
static void optimize_result(void)
{
	int i, best;

	/* using the '\0' symbol last allows compress_symbols to use standard
	 * fast string functions */
	for (i = 255; i >= 0; i--) {

		/* if this table slot is empty (it is not used by an actual
		 * original char code */
		if (!best_table_len[i]) {

			/* find the token with the breates profit value */
			best = find_best_token();

			/* place i/*
 * Device driver for the SYMBIOS/LSILOGIC 53C8XX and 53C1010 family 
 * of PCI-SCSI IO processors.
 *
 * Copyright (C) 1999-2001  Gerard Roudier <groudier@free.fr>
 *
 * This driver is derived from the Linux sym53c8xx driver.
 * Copyright (C) 1998-2000  Gerard Roudier
 *
 * The sym53c8xx driver is derived from the ncr53c8xx driver that had been 
 * a port of the FreeBSD ncr driver to Linux-1.2.13.
 *
 * The original ncr driver has been written for 386bsd and FreeBSD by
 *         Wolfgang Stanglmeier        <wolf@cologne.de>
 *         Stefan Esser                <se@mi.Uni-Koeln.de>
 * Copyright (C) 1994  Wolfgang Stanglmeier
 *
 * Other major contributions:
 *
 * NVRAM detection and reading.
 * Copyright (C) 1997 Richard Waltham <dormouse@farsrobt.demon.co.uk>
 *
 *-----------------------------------------------------------------------------
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SYM53C8XX_H
#define SYM53C8XX_H


/*
 *  DMA addressing mode.
 *
 *  0 : 32 bit addressing for all chips.
 *  1 : 40 bit addressing when supported by chip.
 *  2 : 64 bit addressing when supported by chip,
 *      limited to 16 segments of 4 GB -> 64 GB max.
 */
#define	SYM_CONF_DMA_ADDRESSING_MODE CONFIG_SCSI_SYM53C8XX_DMA_ADDRESSING_MODE

/*
 *  NVRAM support.
 */
#if 1
#define SYM_CONF_NVRAM_SUPPORT		(1)
#endif

/*
 *  These options are not tunable from 'make config'
 */
#if 1
#define	SYM_LINUX_PROC_INFO_SUPPORT
#define SYM_LINUX_USER_COMMAND_SUPPORT
#define SYM_LINUX_USER_INFO_SUPPORT
#define SYM_LINUX_DEBUG_CONTROL_SUPPORT
#endif

/*
 *  Also handle old NCR chips if not (0).
 */
#define SYM_CONF_GENERIC_SUPPORT	(1)

/*
 *  Allow tags from 2 to 256, default 8
 */
#ifndef CONFIG_SCSI_SYM53C8XX_MAX_TAGS
#define CONFIG_SCSI_SYM53C8XX_MAX_TAGS	(8)
#endif

#if	CONFIG_SCSI_SYM53C8XX_MAX_TAGS < 2
#define SYM_CONF_MAX_TAG	(2)
#elif	CONFIG_SCSI_SYM53C8XX_MAX_TAGS > 256
#define SYM_CONF_MAX_TAG	(256)
#else
#define	SYM_CONF_MAX_TAG	CONFIG_SCSI_SYM53C8XX_MAX_TAGS
#endif

#ifndef	CONFIG_SCSI_SYM53C8XX_DEFAULT_TAGS
#define	CONFIG_SCSI_SYM53C8XX_DEFAULT_TAGS	SYM_CONF_MAX_TAG
#endif

/*
 *  Anyway, we configure the driver for at least 64 tags per LUN. :)
 */
#if	SYM_CONF_MAX_TAG <= 64
#define SYM_CONF_MAX_TAG_ORDER	(6)
#elif	SYM_CONF_MAX_TAG <= 128
#define SYM_CONF_MAX_TAG_ORDER	(7)
#else
#define SYM_CONF_MAX_TAG_ORDER	(8)
#endif

/*
 *  Max number of SG entries.
 */
#define SYM_CONF_MAX_SG		(96)

/*
 *  Driver setup structure.
 *
 *  This structure is initialized from linux config options.
 *  It can be overridden at b