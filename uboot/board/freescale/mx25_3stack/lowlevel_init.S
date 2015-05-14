/*
 * Copyright (c) 2009  Freescale Semiconductor
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

#include <asm/arch/mx25-regs.h>

.macro REG reg, val
	ldr r2, =\reg
	ldr r3, =\val
	str r3, [r2]
.endm

.macro REG8 reg, val
	ldr r2, =\reg
	ldr r3, =\val
	strb r3, [r2]
.endm

.globl lowlevel_init
lowlevel_init:

	//REG 0x53F80008, 0x20034000 // ARM clk = 399, AHB clk = 133
	REG 0x53F80008, 0x30030000 // ARM clk = 532, AHB clk = 133

	//REG 0x43FAC000, 0x00000002

	/* Init Debug Board CS5 */
	#if 0
	REG 0xB8002050, 0x0000D843
	REG 0xB8002054, 0x22252521
	REG 0xB8002058, 0x22220A00
	#endif

#if 0
    REG 0xB8002030, 0x0000D843 /* CS3_CSCRU */
    REG 0xB8002034, 0x22252321 /* CS3_CSCRL */
    REG 0xB8002038, 0x22220A00 /* CS3_CSCRA */
#endif

#if 1
    REG 0xB8002030, 0x0000E043 /* CS3_CSCRU */
    REG 0xB8002034, 0xFFFF2301 /* CS3_CSCRL */
    REG 0xB8002038, 0xFFFF7F00 /* CS3_CSCRA */
#endif

/*
 REG 0xB8002030, 0x12020802   CS3_CSCRU
    0001
	SP[31]:0   Supervisor Protect
	WP[30]:0   Write Protect
	BCD[29-28]: 01  Burst Clock Divisor

	0010
	BCS[27-24]:0010  Burst Clock Start


   0000
   PSZ[23-22]:  00 Page Size
   PME[21]:  0   Page Mode Emulation
   Sync[20]:  0  Synchronous Bust Mode Enable

   0010
   PSZ[19-16]: 0010 Date Output Length

   00*******
   CNC[15-14]: 10  chip Select Negation Clock Cycles

   00 1000******
   WSC[13-8]: 00 0001  Wait State Control

   0000
   EW[7] 0   nECB/WAIT
   WWS[6-4]  Write Wait State

   0010
   EDC[3-0]  0010 Extra Dead Cycles

   0x12028102
*/

/*
  REG 0xB8002034, 0x80330b03  CS3_CSCRL

  1000
  OEA[31-28]  1000   nOE Assert

  0000
  OEN[27-24]  0000   nOE Negate

  0011******
  EBWA[23-20] 0001   nEB Write Assert

  0011*****
  EBWN[19-16] 0001   nEB Write Negate

  0000
  CSA[15-12]  0000   nCS Assert

  1
  EBC[11]  1      Enable Byte Control

  011
  DSZ[10-8]  011  Data Port Size

  0000
  CSN[7-4]  0000  nCS Negate

  0
  PSR[3]  0   Pseudo SRAM Enable

  0
  CRE[2] 0   Control Register Enable

  1
  WAP[1] 1   Wrap Memory Mode

  1
  CSEN[0] 1  nCS Enable

  0x80110b03


*/



	/* MAX (Multi-Layer AHB Crossbar Switch) setup */
	/* MAX - priority for MX25 is (SDHC2/SDMA)>USBOTG>RTIC>IAHB>DAHB */
	ldr r0, =MAX_BASE
	ldr r1, =0x00002143
	str r1, [r0, #0x000]        /* for S0 */
	str r1, [r0, #0x100]        /* for S1 */
	str r1, [r0, #0x200]        /* for S2 */
	str r1, [r0, #0x300]        /* for S3 */
	str r1, [r0, #0x400]        /* for S4 */
	/* SGPCR - always park on last master */
	ldr r1, =0x10
	str r1, [r0, #0x010]        /* for S0 */
	str r1, [r0, #0x110]        /* for S1 */
	str r1, [r0, #0x210]        /* for S2 */
	str r1, [r0, #0x310]        /* for S3 */
	str r1, [r0, #0x410]        /* for S4 */
	/* MGPCR - restore default values */
	ldr r1, =0x0
	str r1, [r0, #0x800]        /* for M0 */
	str r1, [r0, #0x900]        /* for M1 */
	str r1, [r0, #0xA00]        /* for M2 */
	str r1, [r0, #0xB00]        /* for M3 */
	str r1, [r0, #0xC00]        /* for M4 */

	/* M3IF setup */
	ldr r1, =M3IF_BASE
	ldr r0, =0x00000001
	str r0, [r1]  /* M3IF control reg */

	#define AIPS1_BASE_ADDR      0x43F00000
	#define IOMUXC_BASE_ADDR   (AIPS1_BASE_ADDR + 0x000AC000)
	#define IOMUXGPR           (AIPS1_BASE_ADDR + 0x000AC000)
	#define MUX_CLKO           (IOMUXGPR + 0x0220)
	#define PAD_CLKO           (IOMUXGPR + 0x0414)

	#define DVS_CLKO           (IOMUXGPR + 0x0434)


	ldr r0, =MUX_CLKO
	mov r2, #0x10  //10000b
	str r2, [r0]

	ldr r0, =PAD_CLKO
	mov r2, #0x4 //100b
	str r2, [r0]

	ldr r0, =DVS_CLKO
	mov r2, #0x0
	str r2, [r0]

	/* default CLKO to 1/32 of the ARM core */
	ldr r0, =CCM_MCR
	ldr r1, =CCM_MCR
	bic r1, r1, #0x00F00000
	bic r1, r1, #0x7F000000
	#mov r2,     #0x5F000000
	mov r2,     #0x47000000
	#add r2, r2, #0x00200000
	add r2, r2, #0x00700000
	orr r1, r1, r2
	str r1, [r0]

	/* enable all the clocks */
	ldr r2, =0x1FFFFFFF
	ldr r0, =CCM_CGR0
	str r2, [r0]
	ldr r2, =0xFFFFFFFF
	ldr r0, =CCM_CGR1
	str r2, [r0]
	ldr r2, =0x000FDFFF
	ldr r0, =CCM_CGR2
	str r2, [r0]
	mov	pc, lr

