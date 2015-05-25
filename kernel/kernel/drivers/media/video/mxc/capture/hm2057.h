/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef hm2057_H
#define hm2057_H

#include <linux/types.h>
//#include <mach/camera.h>

extern struct hm2057_reg hm2057_regs;

enum hm2057_width {
	WORD_LEN,
	BYTE_LEN
};

struct hm2057_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
	enum hm2057_width width;
	unsigned short mdelay_time;
};

struct hm2057_reg {
   	const struct hm2057_i2c_reg_conf *init_tbl;
	uint16_t init_tbl_size;
   	const struct hm2057_i2c_reg_conf *preview_tbl;
	uint16_t preview_tbl_size;
   	const struct hm2057_i2c_reg_conf *snapshot_tbl;
	uint16_t snapshot_tbl_size;

	const struct hm2057_i2c_reg_conf *awb_tbl;
	uint16_t awb_tbl_size;
	const struct hm2057_i2c_reg_conf *mwb_cloudy_tbl;
	uint16_t mwb_cloudy_tbl_size;
	const struct hm2057_i2c_reg_conf *mwb_day_light_tbl;
	uint16_t mwb_day_light_tbl_size;
	const struct hm2057_i2c_reg_conf *mwb_fluorescent_tbl;
	uint16_t mwb_fluorescent_tbl_size;
	const struct hm2057_i2c_reg_conf *mwb_incandescent_tbl;
	uint16_t mwb_incandescent_tbl_size;

	const struct hm2057_i2c_reg_conf *effect_off_tbl;
	uint16_t effect_off_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_mono_tbl;
	uint16_t effect_mono_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_sepia_tbl;
	uint16_t effect_sepia_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_negative_tbl;
	uint16_t effect_negative_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_solarize_tbl;
	uint16_t effect_solarize_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_bluish_tbl;
	uint16_t effect_bluish_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_greenish_tbl;
	uint16_t effect_greenish_tbl_size;
	const struct hm2057_i2c_reg_conf *effect_reddish_tbl;
	uint16_t effect_reddish_tbl_size;

	const struct hm2057_i2c_reg_conf *scene_auto_tbl;
	uint16_t scene_auto_tbl_size;
	const struct hm2057_i2c_reg_conf *scene_night_tbl;
	uint16_t scene_night_tbl_size;

	const struct hm2057_i2c_reg_conf *size_qvga_tbl;
	uint16_t size_qvga_tbl_size;
	const struct hm2057_i2c_reg_conf *size_uxga_tbl;
	uint16_t size_uxga_tbl_size;

};




static const struct hm2057_i2c_reg_conf const size_uxga_tbl[] = {
//1600-1200	
#if 0
	{0x0006,0x00, BYTE_LEN, 0},
	{0x000D,0x00, BYTE_LEN, 0},
	{0x000E,0x00, BYTE_LEN, 0},
	{0x0125,0xDF, BYTE_LEN, 0},
	{0x0126,0x00, BYTE_LEN, 0},
	{0x011F,0x08, BYTE_LEN, 0},
	
	{0x05E4,0x0B, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x4A, BYTE_LEN, 0},
	{0x05E7,0x06, BYTE_LEN, 0},
	{0x05E8,0x0B, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0xBA, BYTE_LEN, 0},
	{0x05EB,0x04, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}
#endif
	{0x000D,0x00,BYTE_LEN, 0},
	{0x000E,0x00,BYTE_LEN, 0},
	{0x011F,0x88,BYTE_LEN, 0},
	{0x0125,0xDF,BYTE_LEN, 0},
	{0x0126,0x70,BYTE_LEN, 0},
	//{0x0131,0xAC,BYTE_LEN, 0},
	//{0x0366,0x20,BYTE_LEN, 0},// MinCTFCount, 08=20/4
	//{0x0433,0x40,BYTE_LEN, 0},// ABLC LPoint, 10=40/4
	//{0x0435,0x50,BYTE_LEN, 0},// ABLC UPoint, 14=50/4

	{0x05E4,0x0a,BYTE_LEN, 0},// Windowing
	{0x05E5,0x00,BYTE_LEN, 0},
	{0x05E6,0x49,BYTE_LEN, 0},
	{0x05E7,0x06,BYTE_LEN, 0},
	{0x05E8,0x0a,BYTE_LEN, 0},
	{0x05E9,0x00,BYTE_LEN, 0},
	{0x05EA,0xb9,BYTE_LEN, 0},
	{0x05EB,0x04,BYTE_LEN, 0},

	{0x0000,0x01,BYTE_LEN, 0},
	{0x0100,0x01,BYTE_LEN, 0},
	{0x0101,0x01,BYTE_LEN, 0}

};



static const struct hm2057_i2c_reg_conf const size_sxga_tbl[] = {
//1280-1024	
	{0x0006,0x00, BYTE_LEN, 0},
	{0x000D,0x00, BYTE_LEN, 0},
	{0x000E,0x00, BYTE_LEN, 0},
	{0x011F,0x88, BYTE_LEN, 0},
	{0x0125,0xDF, BYTE_LEN, 0},
	{0x0126,0x70, BYTE_LEN, 0},
	//{0x0131,0xAC,BYTE_LEN, 0},
	//{0x0366,0x20,BYTE_LEN, 0},// MinCTFCount, 08=20/4
	//{0x0433,0x40,BYTE_LEN, 0},// ABLC LPoint, 10=40/4
	//{0x0435,0x50,BYTE_LEN, 0},// ABLC UPoint, 14=50/4
	
	
	{0x05E4,0x0A, BYTE_LEN, 0},//windows 0x00
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x09, BYTE_LEN, 0},
	{0x05E7,0x05, BYTE_LEN, 0},
	{0x05E8,0x0A, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0x09, BYTE_LEN, 0},
	{0x05EB,0x04, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};

static const struct hm2057_i2c_reg_conf const size_hd800_tbl[] = {
//1280-800	
	{0x0006,0x04, BYTE_LEN, 0},
	{0x000D,0x00, BYTE_LEN, 0},
	{0x000E,0x00, BYTE_LEN, 0},
	{0x0125,0xDF, BYTE_LEN, 0},
	{0x0126,0x00, BYTE_LEN, 0},
	{0x011F,0x08, BYTE_LEN, 0},
	
	{0x05E4,0x08, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x07, BYTE_LEN, 0},
	{0x05E7,0x05, BYTE_LEN, 0},
	{0x05E8,0x08, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0x27, BYTE_LEN, 0},
	{0x05EB,0x03, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};


static const struct hm2057_i2c_reg_conf const size_hd720_tbl[] = {
//1280-720	
	{0x0006,0x08, BYTE_LEN, 0},
	{0x000D,0x00, BYTE_LEN, 0},
	{0x000E,0x00, BYTE_LEN, 0},
	{0x0125,0xDF, BYTE_LEN, 0},
	{0x0126,0x00, BYTE_LEN, 0},
	{0x011F,0x08, BYTE_LEN, 0},
	
	{0x05E4,0x08, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x07, BYTE_LEN, 0},
	{0x05E7,0x05, BYTE_LEN, 0},
	{0x05E8,0x08, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0xD7, BYTE_LEN, 0},
	{0x05EB,0x02, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};


static const struct hm2057_i2c_reg_conf const size_svga_tbl[] = {
//800-600	
	{0x0006,0x00, BYTE_LEN, 0},
	{0x000D,0x01, BYTE_LEN, 0},
	{0x000E,0x11, BYTE_LEN, 0},
	{0x0125,0xDF, BYTE_LEN, 0},
	{0x0126,0x00, BYTE_LEN, 0},
	{0x011F,0x00, BYTE_LEN, 0},
	
	{0x05E4,0x06, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x25, BYTE_LEN, 0},
	{0x05E7,0x03, BYTE_LEN, 0},
	{0x05E8,0x07, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0x5F, BYTE_LEN, 0},
	{0x05EB,0x02, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};

static const struct hm2057_i2c_reg_conf const size_vga_tbl[] = {
//640-480	
	{0x0006,0x00, BYTE_LEN, 0},
	{0x000D,0x01, BYTE_LEN, 0},
	{0x000E,0x11, BYTE_LEN, 0},
	{0x0125,0xFF, BYTE_LEN, 0},
	{0x0126,0x70, BYTE_LEN, 0},
	{0x011F,0x80, BYTE_LEN, 0},

	{0x05E0,0xA0, BYTE_LEN, 0}, //scaler
	{0x05E1,0x00, BYTE_LEN, 0},
	{0x05E2,0xA0, BYTE_LEN, 0},
	{0x05E3,0x00, BYTE_LEN, 0},
	{0x05E4,0x04, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x83, BYTE_LEN, 0},
	{0x05E7,0x02, BYTE_LEN, 0},
	{0x05E8,0x06, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0xE5, BYTE_LEN, 0},
	{0x05EB,0x01, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};



static const struct hm2057_i2c_reg_conf const size_cif_tbl[] = {
//352-288	
	{0x0006,0x00, BYTE_LEN, 0},
	{0x000D,0x01, BYTE_LEN, 0},
	{0x000E,0x11, BYTE_LEN, 0},
	{0x0125,0xFF, BYTE_LEN, 0},
	{0x0126,0x72, BYTE_LEN, 0},
	{0x011F,0x80, BYTE_LEN, 0},

	{0x05E0,0x08, BYTE_LEN, 0}, //scaler
	{0x05E1,0x01, BYTE_LEN, 0},
	{0x05E2,0x08, BYTE_LEN, 0},
	{0x05E3,0x01, BYTE_LEN, 0},
	{0x05E4,0x14, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x73, BYTE_LEN, 0},
	{0x05E7,0x01, BYTE_LEN, 0},
	{0x05E8,0x04, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0x23, BYTE_LEN, 0},
	{0x05EB,0x01, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};

static const struct hm2057_i2c_reg_conf const size_qvga_tbl[] = {
	{0x000D,0x01, BYTE_LEN, 0},
	{0x000E,0x11, BYTE_LEN, 0},
	{0x011F,0x80, BYTE_LEN, 0},
	{0x0125,0xFF, BYTE_LEN, 0},
	{0x0126,0x70, BYTE_LEN, 0},
	
	{0x05E0,0x40, BYTE_LEN, 0}, //scaler
	{0x05E1,0x01, BYTE_LEN, 0},
	{0x05E2,0x40, BYTE_LEN, 0},
	{0x05E3,0x01, BYTE_LEN, 0},
	{0x05E4,0x02, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0x41, BYTE_LEN, 0},
	{0x05E7,0x01, BYTE_LEN, 0},
	{0x05E8,0x03, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0xF2, BYTE_LEN, 0},
	{0x05EB,0x00, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}
};

static const struct hm2057_i2c_reg_conf const size_qcif_tbl[] = {
//176-144	
	{0x0006,0x00, BYTE_LEN, 0},
	{0x000D,0x01, BYTE_LEN, 0},
	{0x000E,0x11, BYTE_LEN, 0},
	{0x0125,0xFF, BYTE_LEN, 0},
	{0x0126,0x02, BYTE_LEN, 0},
	{0x011F,0x80, BYTE_LEN, 0},

	{0x05E0,0x10, BYTE_LEN, 0}, //scaler
	{0x05E1,0x02, BYTE_LEN, 0},
	{0x05E2,0x10, BYTE_LEN, 0},
	{0x05E3,0x02, BYTE_LEN, 0},
	{0x05E4,0x0A, BYTE_LEN, 0},//windows
	{0x05E5,0x00, BYTE_LEN, 0},
	{0x05E6,0xB9, BYTE_LEN, 0},
	{0x05E7,0x00, BYTE_LEN, 0},
	{0x05E8,0x03, BYTE_LEN, 0},
	{0x05E9,0x00, BYTE_LEN, 0},
	{0x05EA,0x92, BYTE_LEN, 0},
	{0x05EB,0x00, BYTE_LEN, 0},

	{0x0000,0x01, BYTE_LEN, 0},
	{0x0100,0x01, BYTE_LEN, 0},
	{0x0101,0x01, BYTE_LEN, 0}

};

#endif /* hm2057_H */
