/*
 *  Copyright (c) 2009  Freescale Semiconductor
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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


#include <config.h>
#include <version.h>

.extern reset

#define DCDGEN(i,type, addr, data) \
dcd_##i:			;\
    .long type			;\
    .long addr			;\
    .long data

.globl _initheader
_initheader:
	b	reset
	.org 0x400
app_code_jump_v:	.long reset
app_code_barker:	.long 0xB1
app_code_csf:		.long 0x83F64400
hwcfg_ptr_ptr:		.long hwcfg_ptr
super_root_key:		.long 0x83F64000
hwcfg_ptr:		.long dcd_data
app_dest_ptr:		.long TEXT_BASE
dcd_data:		.long 0xB17219E9

#ifdef MXC_MEMORY_MDDR //Include CONFIG_IPS420_MDDR
dcd_len:		.long 12*15
#elif defined(CONFIG_IPS420)
dcd_len:		.long 12*42
#else
dcd_len:		.long 12*24
#endif

#if (defined(CONFIG_PS4KV2) || defined(CONFIG_IPS420)) //CPLD in on the CS0 addr.
DCDGEN( 1, 4, 0xB8002000, 0x0000E043) /* CS0_CSCRU */
DCDGEN( 2, 4, 0xB8002004, 0xffff2301) /* CS0_CSCRL */
DCDGEN( 3, 4, 0xB8002008, 0xffff7f00) /* CS0_CSCRA */
#else	//CPLD is on the CS3 addr.
DCDGEN( 1, 4, 0xB8002030, 0x0000E043) /* CS3_CSCRU */
DCDGEN( 2, 4, 0xB8002034, 0xffff2301) /* CS3_CSCRL */
DCDGEN( 3, 4, 0xB8002038, 0xffff7f00) /* CS3_CSCRA */
#endif

#ifdef MXC_MEMORY_MDDR
/* MDDR init */
DCDGEN( 4, 4, 0xB8001010, 0x00000004) /* enable mDDR */
DCDGEN( 5, 4, 0xB8001000, 0x92100000) /* precharge command */
DCDGEN( 6, 1, 0x80000400, 0x12344321) /* precharge all dummy write */
DCDGEN( 7, 4, 0xB8001000, 0xA2100000) /* auto-refresh command */
DCDGEN( 8, 4, 0x80000000, 0x12344321) /* dummy write for refresh */
DCDGEN( 9, 4, 0x80000000, 0x12344321) /* dummy write for refresh */
DCDGEN(10, 4, 0xB8001000, 0xB2100000) /* Load Mode Reg command - cas=3 bl=8 */
DCDGEN(11, 1, 0x80000033, 0xda)	/* dummy write -- address has the mode bits */
DCDGEN(12, 1, 0x81000000, 0xff) /* dummy write -- address has the mode bits */
DCDGEN(13, 4, 0xB8001000, 0x83216880)
DCDGEN(14, 4, 0xB8001004, 0x00295729)
#elif defined(CONFIG_IPS420)
DCDGEN( 4, 4, 0xB8001004, 0x0076E83A)	/* initial value for ESDCFG0 */
DCDGEN( 5, 4, 0xB800100C, 0x0076E83A)	/* initial value for ESDCFG1 */
DCDGEN( 6, 4, 0xB8001010, 0x00000204)	/* ESD_MISC */

DCDGEN( 7, 4, 0xB8001000, 0x92210000)	/* CS0 precharge command */
DCDGEN( 8, 4, 0x80000f00, 0x12344321)	/* CSD0 precharge all dummy write */
DCDGEN( 9, 4, 0xB8001008, 0x92210000)	/* CS0 precharge command */
DCDGEN( 10, 4, 0x90000f00, 0x12344321)	/* CSD1 precharge all dummy write */

DCDGEN( 11, 4, 0xB8001000, 0xB2210000)	/* Load Mode Register command */
DCDGEN( 12, 1, 0x82000000, 0xda)		/* dummy write Load EMR2 */
DCDGEN( 13, 1, 0x83000000, 0xda)		/* dummy write Load EMR3 */
DCDGEN( 14, 1, 0x81000400, 0xda)		/* dummy write Load EMR1; enable DLL */
DCDGEN( 15, 1, 0x80000333, 0xda)		/* dummy write Load MR; reset DLL */
DCDGEN( 16, 4, 0xB8001008, 0xB2210000)	/* Load Mode Register command */
DCDGEN( 17, 1, 0x92000000, 0xda) 	/* dummy write Load EMR2 */
DCDGEN( 18, 1, 0x93000000, 0xda) 	/* dummy write Load EMR3 */
DCDGEN( 19, 1, 0x91000400, 0xda) 	/* dummy write Load EMR1; enable DLL */
DCDGEN( 20, 1, 0x90000333, 0xda) 	/* dummy write Load MR; reset DLL */

DCDGEN( 21, 4, 0xB8001000, 0x92210000)	/* CS0 precharge command */
DCDGEN( 22, 1, 0x80000400, 0x12345678)	/* precharge all dummy write */
DCDGEN( 23, 4, 0xB8001008, 0x92210000)	/* CS0 precharge command */
DCDGEN( 24, 1, 0x90000400, 0x12345678)	/* precharge all dummy write */

DCDGEN( 25, 4, 0xB8001000, 0xA2210000)	/* select manual refresh mode */
DCDGEN( 26, 4, 0x80000000, 0x87654321)	/* manual refresh */
DCDGEN( 27, 4, 0x80000000, 0x87654321)	/* manual refresh twice */
DCDGEN( 28, 4, 0xB8001008, 0xA2210000)	/* select manual refresh mode */
DCDGEN( 29, 4, 0x90000000, 0x87654321)	/* manual refresh */
DCDGEN( 30, 4, 0x90000000, 0x87654321)	/* manual refresh twice */

DCDGEN( 31, 4, 0xB8001000, 0xB2210000)	/* Load Mode Register command */
DCDGEN( 32, 1, 0x80000233, 0xda)		/* Load MR; CL=3, BL=8, end DLL reset */
DCDGEN( 33, 1, 0x81000780, 0xda)		/* Load EMR1; OCD default */
DCDGEN( 34, 1, 0x81000400, 0xda)		/* Load EMR1; OCD exit */
DCDGEN( 35, 4, 0xB8001008, 0xB2210000)	/* Load Mode Register command */
DCDGEN( 36, 1, 0x90000233, 0xda) 	/* Load MR; CL=3, BL=8, end DLL reset */
DCDGEN( 37, 1, 0x91000780, 0xda) 	/* Load EMR1; OCD default */
DCDGEN( 38, 1, 0x91000400, 0xda) 	/* Load EMR1; OCD exit */

DCDGEN( 39, 4, 0xB8001000, 0x82216080)	/* normal mode */
DCDGEN( 40, 4, 0xB8001008, 0x82216080)	/* normal mode */

DCDGEN( 41, 4, 0x43FAC454, 0x00001000)	/* IOMUXC_SW_PAD_CTL_GRP_DDRTYPE(1-5) */
#else
/* DDR2 init */
DCDGEN( 4, 4, 0xB8001004, 0x0076E83A)	/* initial value for ESDCFG0 */
DCDGEN( 5, 4, 0xB8001010, 0x00000204)	/* ESD_MISC */
DCDGEN( 6, 4, 0xB8001000, 0x92210000)	/* CS0 precharge command */
DCDGEN( 7, 4, 0x80000f00, 0x12344321)	/* precharge all dummy write */
DCDGEN( 8, 4, 0xB8001000, 0xB2210000)	/* Load Mode Register command */
DCDGEN( 9, 1, 0x82000000, 0xda)		/* dummy write Load EMR2 */
DCDGEN(10, 1, 0x83000000, 0xda)		/* dummy write Load EMR3 */
DCDGEN(11, 1, 0x81000400, 0xda)		/* dummy write Load EMR1; enable DLL */
DCDGEN(12, 1, 0x80000333, 0xda)		/* dummy write Load MR; reset DLL */

DCDGEN(13, 4, 0xB8001000, 0x92210000)	/* CS0 precharge command */
DCDGEN(14, 1, 0x80000400, 0x12345678)	/* precharge all dummy write */

DCDGEN(15, 4, 0xB8001000, 0xA2210000)	/* select manual refresh mode */
DCDGEN(16, 4, 0x80000000, 0x87654321)	/* manual refresh */
DCDGEN(17, 4, 0x80000000, 0x87654321)	/* manual refresh twice */

DCDGEN(18, 4, 0xB8001000, 0xB2210000)	/* Load Mode Register command */
DCDGEN(19, 1, 0x80000233, 0xda)		/* Load MR; CL=3, BL=8, end DLL reset */
DCDGEN(20, 1, 0x81000780, 0xda)		/* Load EMR1; OCD default */
DCDGEN(21, 1, 0x81000400, 0xda)		/* Load EMR1; OCD exit */
DCDGEN(22, 4, 0xB8001000, 0x82216080)	/* normal mode */
DCDGEN(23, 4, 0x43FAC454, 0x00001000)	/* IOMUXC_SW_PAD_CTL_GRP_DDRTYPE(1-5) */
#endif

DCDGEN(99, 4, 0x53F80008, 0x30030000) /* 0x30030000:CLKCTL ARM=532 AHB=133;0x20034000: CLKCTL ARM=400 AHB=133*/
card_cfg:	.long UBOOT_IMAGE_SIZE
