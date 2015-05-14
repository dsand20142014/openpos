/*
 * Copyright 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
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
 * @file mx27_prphw.c
 *
 * @brief MX27 Video For Linux 2 capture driver
 *
 * @ingroup MXC_V4L2_CAPTURE
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "mx27_prp.h"

#define PRP_MIN_IN_WIDTH	32
#define PRP_MAX_IN_WIDTH	2044
#define PRP_MIN_IN_HEIGHT	32
#define PRP_MAX_IN_HEIGHT	2044

typedef struct _coeff_t {
	unsigned long coeff[2];
	unsigned long cntl;
} coeff_t[2][2];

static coe