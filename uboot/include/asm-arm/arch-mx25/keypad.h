/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX51-3Stack Freescale board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _MXC_KEYPAD_H_
#define _MXC_KEYPAD_H_

#include <config.h>

#define GKEY_0  '0'
#define GKEY_1  '1'
#define GKEY_2  '2'
#define GKEY_3  '3'
#define GKEY_4  '4'
#define GKEY_5  '5'
#define GKEY_6  '6'
#define GKEY_7  '7'
#define GKEY_8  '8'
#define GKEY_9  '9'
#define GKEY_F1 0x40
#define GKEY_F2 0x41
#define GKEY_F3 0x42
#define GKEY_F4 0x43
#define GKEY_00 0x3C
#define GKEY_CANCEL 0x3B
#define GKEY_CLEAR  0x3D
#define GKEY_FEED   0x3E
#define GKEY_ENTER  0x3A
#define GKEY_CAMERA 0xCA

#define TEST_CANCEL_KEY_DEPRESS(k, e)  (((k) == (GKEY_CANCEL)) && (((e) == (KDepress))))

#endif
