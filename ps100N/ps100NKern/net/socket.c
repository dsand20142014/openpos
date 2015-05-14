 /***************************************************************************\
|*                                                                           *|
|*       Copyright 1993-1999 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-1999 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
 \***************************************************************************/

/*
 * GPL licensing note -- nVidia is allowing a liberal interpretation of
 * the documentation restriction above, to merely say that this nVidia's
 * copyright and disclaimer should be included with all code derived
 * from this source.  -- Jeff Garzik <jgarzik@pobox.com>, 01/Nov/99 
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/riva_tbl.h,v 1.9 2002/01/30 01:35:03 mvojkovi Exp $ */


/*
 * RIVA Fixed Functionality Init Tables.
 */
static unsigned RivaTablePMC[][2] =
{
    {0x00000050, 0x00000000},
    {0x00000080, 0xFFFF00FF},
    {0x00000080, 0xFFFFFFFF}
};
static unsigned RivaTablePTIMER[][2] =
{
    {0x00000080, 0x00000008},
    {0x00000084, 0x00000003},
    {0x00000050, 0x00000000},
    {0x00000040, 0xFFFFFFFF}
};
static unsigned RivaTableFIFO[][2] =
{
    {0x00000000, 0x80000000},
    {0x00000800, 0x80000001},
    {0x00001000, 0x80000002},
    {0x00001800, 0x80000010},
    {0x00002000, 0x80000011},
    {0x00002800, 0x80000012},
    {0x00003000, 0x80000016},
    {0x00003800, 0x80000013}
};
static unsigned nv3TablePFIFO[][2] =
{
    {0x00000140, 0x00000000},
    {0x00000480, 0x00000000},
    {0x00000490, 0x00000000},
    {0x00000494, 0x00000000},
    {0x00000481, 0x00000000},
    {0x00000084, 0x00000000},
    {0x00000086, 0x00002000},
    {0x00000085, 0x00002200},
    {0x00000484, 0x00000000},
    {0x0000049C, 0x00000000},
    {0x00000104, 0x00000000},
    {0x00000108, 0x00000000},
    {0x00000100, 0x00000000},
    {0x000004A0, 0x00000000},
    {0x000004A4, 0x00000000},
    {0x000004A8, 0x00000000},
    {0x000004AC, 0x00000000},
    {0x000004B0, 0x00000000},
    {0x000004B4, 0x00000000},
    {0x000004B8, 0x00000000},
    {0x000004BC, 0x00000000},
    {0x00000050, 0x00000000},
    {0x00000040, 0xFFFFFFFF},
    {0x00000480, 0x00000001},
    {0x00000490, 0x00000001},
    {0x00000140, 0x00000001}
};
static unsigned nv3TablePGRAPH[][2] =
{
    {0x00000020, 0x1230001F},
    {0x00000021, 0x10113000},
    {0x00000022, 0x1131F101},
    {0x00000023, 0x0100F531},
    {0x00000060, 0x00000000},
    {0x00000065, 0x00000000},
    {0x00000068, 0x00000000},
    {0x00000069, 0x00000000},
    {0x0000006A, 0x00000000},
    {0x0000006B, 0x00000000},
    {0x0000006C, 0x00000000},
    {0x0000006D, 0x00000000},
    {0x0000006E, 0x00000000},
    {0x0000006F, 0x00000000},
    {0x000001A8, 0x00000000},
    {0x00000440, 0xFFFFFFFF},
    {0x00000480, 0x00000001},
    {0x000001A0, 0x00000000},
    {0x000001A2, 0x00000000},
    {0x0000018A, 0xFFFFFFFF},
    {0x00000190, 0x00000000},
    {0x00000142, 0x00000000},
    {0x00000154, 0x00000000},
    {0x00000155, 0xFFFFFFFF},
    {0x00000156, 0x00000000},
    {0x00000157, 0xFFFFFFFF},
    {0x00000064, 0x10010002},
    {0x00000050, 0x00000000},
    {0x00000051, 0x00000000},
    {0x00000040, 0xFFFFFFFF},
    {0x00000041, 0xFFFFFFFF},
    {0x00000440, 0xFFFFFFFF},
    {0x000001A9, 0x00000001}
};
static unsigned nv3TablePGRAPH_8BPP[][2] =
{
    {0x000001AA, 0x00001111}
};
static unsigned nv3TablePGRAPH_15BPP[][2] =
{
    {0x000001AA, 0x00002222}
};
static unsigned nv3TablePGRAPH_32BPP[][2] =
{
    {0x000001AA, 0x00003333}
};
static unsigned nv3TablePRAMIN[][2] =
{
    {0x00000500, 0x00010000},
    {0x00000501, 0x007FFFFF},
    {0x00000200, 0x80000000},
    {0x00000201, 0x00C20341},
    {0x00000204, 0x80000001},
    {0x00000205, 0x00C50342},
    {0x00000208, 0x80000002},
    {0x00000209, 0x00C60343},
    {0x0000020C, 0x80000003},
    {0x0000020D, 0x00DC0348},
    {0x00000210, 0x80000004},
    {0x00000211, 0x00DC0349},
    {0x00000214, 0x80000005},
    {0x00000215, 0x00DC034A},
    {0x00000218, 0x80000006},
    {0x00000219, 0x00DC034B},
    {0x00000240, 0x80000010},
    {0x00000241, 0x00D10344},
    {0x00000244, 0x80000011},
    {0x00000245, 0x00D00345},
    {0x00000248, 0x80000012},
    {0x00000249, 0x00CC0346},
    {0x0000024C, 0x80000013},
    {0x0000024D, 0x00D70347},
    {0x00000258, 0x80000016},
    {0x00000259, 0x00CA034C},
    {0x00000D05, 0x00000000},
    {0x00000D06, 0x00000000},
    {0x00000D07, 0x00000000},
    {0x00000D09, 0x00000000},
    {0x00000D0A, 0x00000000},
    {0x00000D0B, 0x00000000},
    {0x00000D0D, 0x00000000},
    {0x00000D0E, 0x00000000},
    {0x00000D0F, 0x00000000},
    {0x00000D11, 0x00000000},
    {0x00000D12, 0x00000000},
    {0x00000D13, 0x00000000},
    {0x00000D15, 0x00000000},
    {0x00000D16, 0x00000000},
    {0x00000D17, 0x00000000},
    {0x00000D19, 0x00000000},
    {0x00000D1A, 0x00000000},
    {0x00000D1B, 0x00000000},
    {0x00000D1D, 0x00000140},
    {0x00000D1E, 0x00000000},
    {0x00000D1F, 0x00000000},
    {0x00000D20, 0x10100200},
    {0x00000D21, 0x00000000},
    {0x00000D22, 0x00000000},
    {0x00000D23, 0x00000000},
    {0x00000D24, 0x10210200},
    {0x00000D25, 0x00000000},
    {0x00000D26, 0x00000000},
    {0x00000D27, 0x00000000},
    {0x00000D28, 0x10420200},
    {0x00000D29, 0x00000000},
    {0x00000D2A, 0x00000000},
    {0x00000D2B, 0x00000000},
    {0x00000D2C, 0x10830200},
    {0x00000D2D, 0x00000000},
    {0x00000D2E, 0x00000000},
    {0x00000D2F, 0x00000000},
    {0x00000D31, 0x00000000},
    {0x00000D32, 0x00000000},
    {0x00000D33, 0x00000000}
};
static unsigned nv3TablePRAMIN_8BPP[][2] =
{
    /*           0xXXXXX3XX For  MSB mono format */
    /*           0xXXXXX2XX For  LSB mono format */
    {0x00000D04, 0x10110203},
    {0x00000D08, 0x10110203},
    {0x00000D0C, 0x1011020B},
    {0x00000D10, 0x10118203},
    {0x00000D14, 0x10110203},
    {0x00000D18, 0x10110203},
    {0x00000D1C, 0x10419208},
    {0x00000D30, 0x10118203}
};
static unsigned nv3TablePRAMIN_15BPP[][2] =
{
    /*           0xXXXXX2XX For  MSB mono format */
    /*           0xXXXXX3XX For  LSB mono format */
    {0x00000D04, 0x10110200},
    {0x00000D08, 0x10110200},
    {0x00000D0C, 0x10110208},
    {0x00000D10, 0x10118200},
    {0x00000D14, 0x10110200},
    {0x00000D18, 0x10110200},
    {0x00000D1C, 0x10419208},
    {0x00000D30, 0x10118200}
};
static unsigned nv3TablePRAMIN_32BPP[][2] =
{
    /*           0xXXXXX3XX For  MSB mono format */
    /*           0xXXXXX2XX For  LSB mono format */
    {0x00000D04, 0x10110201},
    {0x00000D08, 0x10110201},
    {0x00000D0C, 0x10110209},
    {0x00000D10, 0x10118201},
    {0x00000D14, 0x10110201},
    {0x00000D18, 0x10110201},
    {0x00000D1C, 0x10419208},
    {0x00000D30, 0x10118201}
};
static unsigned nv4TableFIFO[][2] =
{
    {0x00003800, 0x80000014}
};
static unsigned nv4TablePFIFO[][2] =
{
    {0x00000140, 0x00000000},
    {0x00000480, 0x00000000},
    {0x00000494, 0x00000000},
    {0x00000481, 0x00000000},
    {0x0000048B, 0x00000000},
    {0x00000400, 0x00000000},
    {0x00000414, 0x00000000},
    {0x00000084, 0x03000100},  
    {0x00000085, 0x00000110},
    {0x00000086, 0x00000112},  
    {0x00000143, 0x0000FFFF},
    {0x00000496, 0x0000FFFF},
    {0x00000050, 0x00000000},
    {0x00000040, 0xFFFFFFFF},
    {0x00000415, 0x00000001},
    {0x00000480, 0x00000001},
    {0x00000494, 0x00000001},
    {0x00000495, 0x00000001},
    {0x00000140, 0x00000001}
};
static unsigned nv4TablePGRAPH[][2] =
{
    {0x00000020, 0x1231C001},
    {0x00000021, 0x72111101},
    {0x00000022, 0x11D5F071},
    {0x00000023, 0x10D4FF31},
    {0x00000060, 0x00000000},
    {0x00000068, 0x00000000},
    {0x00000070, 0x00000000},
    {0x00000078, 0x00000000},
    {0x00000061, 0x00000000},
    {0x00000069, 0x00000000},
    {0x00000071, 0x00000000},
    {0x00000079, 0x00000000},
    {0x00000062, 0x00000000},
    {0x0000006A, 0x00000000},
    {0x00000072, 0x00000000},
    {0x0000007A, 0x00000000},
    {0x00000063, 0x00000000},
    {0x0000006B, 0x00000000},
    {0x00000073, 0x00000000},
    {0x0000007B, 0x00000000},
    {0x00000064, 0x00000000},
    {0x0000006C, 0x00000000},
    {0x00000074, 0x00000000},
    {0x0000007C, 0x00000000},
    {0x00000065, 0x00000000},
    {0x0000006D, 0x00000000},
    {0x00000075, 0x00000000},
    {0x0000007D, 0x00000000},
    {0x00000066, 0x00000000},
    {0x0000006E, 0x00000000},
    {0x00000076, 0x00000000},
    {0x0000007E, 0x00000000},
    {0x00000067, 0x00000000},
    {0x0000006F, 0x00000000},
    {0x00000077, 0x00000000},
    {0x0000007F, 0x00000000},
    {0x00000058, 0x00000000},
    {0x00000059, 0x00000000},
    {0x0000005A, 0x00000000},
    {0x0000005B, 0x00000000},
    {0x00000196, 0x00000000},
    {0x000001A1, 0x01FFFFFF},
    {0x00000197, 0x00000000},
    {0x000001A2, 0x01FFFFFF},
    {0x00000198, 0x00000000},
    {0x000001A3, 0x01FFFFFF},
    {0x00000199, 0x00000000},
    {0x000001A4, 0x01FFFFFF},
    {0x00000050, 0x00000000},
    {0x00000040, 0xFFFFFFFF},
    {0x0000005C, 0x10010100},
    {0x000001C4, 0xFFFFFFFF},
    {0x000001C8, 0x00000001},
    {0x00000204, 0x00000000},
    {0x000001C3, 0x00000001}
};
static unsigned nv4TablePGRAPH_8BPP[][2] =
{
    {0x000001C9, 0x00111111},
    {0x00000186, 0x00001010},
    {0x0000020C, 0x03020202}
};
static unsigned nv4TablePGRAPH_15BPP[][2] =
{
    {0x000001C9, 0x00226222},
    {0x00000186, 0x00002071},
    {0x0000020C, 0x09080808}
};
static unsigned nv4TablePGRAPH_16BPP[][2] =
{
    {0x000001C9, 0x00556555},
    {0x00000186, 0x000050C2},
    {0x0000020C, 0x0C0B0B0B}
};
static unsigned nv4TablePGRAPH_32BPP[][2] =
{
    {0x000001C9, 0x0077D777},
    {0x00000186, 0x000070E5},
    {0x0000020C, 0x0E0D0D0D}
};
static unsigned nv4TablePRAMIN[][2] =
{
    {0x00000000, 0x80000010},
    {0x00000001, 0x80011145},
    {0x00000002, 0x80000011},
    {0x00000003, 0x80011146},
    {0x00000004, 0x80000012},
    {0x00000005, 0x80011147},
    {0x00000006, 0x80000013},
    {0x00000007, 0x80011148},
    {0x00000008, 0x80000014},
    {0x00000009, 0x80011149},
    {0x0000000A, 0x80000015},
    {0x0000000B, 0x8001114A},
    {0x0000000C, 0x80000016},
    {0x0000000D, 0x8001114F},
    {0x00000020, 0x80000000},
    {0x00000021, 0x80011142},
    {0x00000022, 0x80000001},
    {0x00000023, 0x80011143},
    {0x00000024, 0x80000002},
    {0x00000025, 0x80011144}, 
    {0x00000026, 0x80000003},
    {0x00000027, 0x8001114B},
    {0x00000028, 0x80000004},
    {0x00000029, 0x8001114C},
    {0x0000002A, 0x80000005},
    {0x0000002B, 0x8001114D},
    {0x0000002C, 0x80000006},
    {0x0000002D, 0x8001114E},
    {0x00000500, 0x00003000},
    {0x00000501, 0x01FFFFFF},
    {0x00000502, 0x00000002},
    {0x00000503, 0x00000002},
    {0x00000508, 0x01008043},
    {0x0000050A, 0x00000000},
    {0x0000050B, 0x00000000},
    {0x0000050C, 0x01008019},
    {0x0000050E, 0x00000000},
    {0x0000050F, 0x00000000},
#if 1
    {0x00000510, 0x01008018},
#else
    {0x00000510, 0x01008044},
#endif
    {0x00000512, 0x00000000},
    {0x00000513, 0x00000000},
    {0x00000514, 0x01008021},
    {0x00000516, 0x00000000},
    {0x00000517, 0x00000000},
    {0x00000518, 0x0100805F},
    {0x0000051A, 0x00000000},
    {0x0000051B, 0x00000000},
#if 1
    {0x0000051C, 0x0100804B},
#else
    {0x0000051C, 0x0100804A},
#endif
    {0x0000051E, 0x00000000},
    {0x0000051F, 0x00000000},
    {0x00000520, 0x0100A048},
    {0x00000521, 0x00000D01},
    {0x00000522, 0x11401140},
    {0x00000523, 0x00000000},
    {0x00000524, 0x0300A054},
    {0x00000525, 0x00000D01},
    {0x00000526, 0x11401140},
    {0x00000527, 0x00000000},
    {0x00000528, 0x0300A055},
    {0x00000529, 0x00000D01},
    {0x0000052A, 0x11401140},
    {0x0000052B, 0x00000000},
    {0x0000052C, 0x00000058},
    {0x0000052E, 0x11401140},
    {0x0000052F, 0x00000000},
    {0x00000530, 0x00000059},
    {0x00000532, 0x11401140},
    {0x00000533, 0x00000000},
    {0x00000534, 0x0000005A},
    {0x00000536, 0x11401140},
    {0x00000537, 0x00000000},
    {0x00000538, 0x0000005B},
    {0x0000053A, 0x11401140},
    {0x0000053B, 0x00000000},
    {0x0000053C, 0x0300A01C},
    {0x0000053E, 0x11401140},
    {0x0000053F, 0x00000000}
};
static unsigned nv4TablePRAMIN_8BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000302},
    {0x0000050D, 0x00000302},
    {0x00000511, 0x00000202},
    {0x00000515, 0x00000302},
    {0x00000519, 0x00000302},
    {0x0000051D, 0x00000302},
    {0x0000052D, 0x00000302},
    {0x0000052E, 0x00000302},
    {0x00000535, 0x00000000},
    {0x00000539, 0x00000000},
    {0x0000053D, 0x00000302}
};
static unsigned nv4TablePRAMIN_15BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000902},
    {0x0000050D, 0x00000902},
    {0x00000511, 0x00000802},
    {0x00000515, 0x00000902},
    {0x00000519, 0x00000902},
    {0x0000051D, 0x00000902},
    {0x0000052D, 0x00000902},
    {0x0000052E, 0x00000902},
    {0x00000535, 0x00000702},
    {0x00000539, 0x00000702},
    {0x0000053D, 0x00000902}
};
static unsigned nv4TablePRAMIN_16BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000C02},
    {0x0000050D, 0x00000C02},
    {0x00000511, 0x00000B02},
    {0x00000515, 0x00000C02},
    {0x00000519, 0x00000C02},
    {0x0000051D, 0x00000C02},
    {0x0000052D, 0x00000C02},
    {0x0000052E, 0x00000C02},
    {0x00000535, 0x00000702},
    {0x00000539, 0x00000702},
    {0x0000053D, 0x00000C02}
};
static unsigned nv4TablePRAMIN_32BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000E02},
    {0x0000050D, 0x00000E02},
   nt need_software_tstamp = sock_flag(sk, SOCK_RCVTSTAMP);
	struct timespec ts[3];
	int empty = 1;
	struct skb_shared_hwtstamps *shhwtstamps =
		skb_hwtstamps(skb);

	/* Race occurred between timestamp enabling and packet
	   receiving.  Fill in the current time for now. */
	if (need_software_tstamp && skb->tstamp.tv64 == 0)
		__net_timestamp(skb);

	if (need_software_tstamp) {
		if (!sock_flag(sk, SOCK_RCVTSTAMPNS)) {
			struct timeval tv;
			skb_get_timestamp(skb, &tv);
			put_cmsg(msg, SOL_SOCKET, SCM_TIMESTAMP,
				 sizeof(tv), &tv);
		} else {
			struct timespec ts;
			skb_get_timestampns(skb, &ts);
			put_cmsg(msg, SOL_SOCKET, SCM_TIMESTAMPNS,
				 sizeof(ts), &ts);
		}
	}


	memset(ts, 0, sizeof(ts));
	if (skb->tstamp.tv64 &&
	    sock_flag(sk, SOCK_TIMESTAMPING_SOFTWARE)) {
		skb_get_timestampns(skb, ts + 0);
		empty = 0;
	}
	if (shhwtstamps) {
		if (sock_flag(sk, SOCK_TIMESTAMPING_SYS_HARDWARE) &&
		    ktime2ts(shhwtstamps->syststamp, ts + 1))
			empty = 0;
		if (sock_flag(sk, SOCK_TIMESTAMPING_RAW_HARDWARE) &&
		    ktime2ts(shhwtstamps->hwtstamp, ts + 2))
			empty = 0;
	}
	if (!empty)
		put_cmsg(msg, SOL_SOCKET,
			 SCM_TIMESTAMPING, sizeof(ts), &ts);
}

EXPORT_SYMBOL_GPL(__sock_recv_timestamp);

static inline int __sock_recvmsg(struct kiocb *iocb, struct socket *sock,
				 struct msghdr *msg, size_t size, int flags)
{
	int err;
	struct sock_iocb *si = kiocb_to_siocb(iocb);

	si->sock = sock;
	si->scm = NULL;
	si->msg = msg;
	si->size = size;
	si->flags = flags;

	err = security_socket_recvmsg(sock, msg, size, flags);
	if (err)
		return err;

	return sock->ops->recvmsg(iocb, sock, msg, size, flags);
}

int sock_recvmsg(struct socket *sock, struct msghdr *msg,
		 size_t size, int flags)
{
	struct kiocb iocb;
	struct sock_iocb siocb;
	int ret;

	init_sync_kiocb(&iocb, NULL);
	iocb.private = &siocb;
	ret = __sock_recvmsg(&iocb, sock, msg, size, flags);
	if (-EIOCBQUEUED == ret)
		ret = wait_on_sync_kiocb(&iocb);
	return ret;
}

int kernel_recvmsg(struct socket *sock, struct msghdr *msg,
		   struct kvec *vec, size_t num, size_t size, int flags)
{
	mm_segment_t oldfs = get_fs();
	int result;

	set_fs(KERNEL_DS);
	/*
	 * the following is safe, since for compiler definitions of kvec and
	 * iovec are identical, yielding the same in-core layout and alignment
	 */
	msg->msg_iov = (struct iovec *)vec, msg->msg_iovlen = num;
	result = sock_recvmsg(sock, msg, size, flags);
	set_fs(oldfs);
	return result;
}

static void sock_aio_dtor(struct kiocb *iocb)
{
	kfree(iocb->private);
}

static ssize_t sock_sendpage(struct file *file, struct page *page,
			     int offset, size_t size, loff_t *ppos, int more)
{
	struct socket *sock;
	int flags;

	sock = file->private_data;

	flags = !(file->f_flags & O_NONBLOCK) ? 0 : MSG_DONTWAIT;
	if (more)
		flags |= MSG_MORE;

	return kernel_sendpage(sock, page, offset, size, flags);
}

static ssize_t sock_splice_read(struct file *file, loff_t *ppos,
			        struct pipe_inode_info *pipe, size_t len,
				unsigned int flags)
{
	struct socket *sock = file->private_data;

	if (unlikely(!sock->ops->splice_read))
		return -EINVAL;

	return sock->ops->splice_read(sock, ppos, pipe, len, flags);
}

static struct sock_iocb *alloc_sock_iocb(struct kiocb *iocb,
					 struct sock_iocb *siocb)
{
	if (!is_sync_kiocb(iocb)) {
		siocb = kmalloc(sizeof(*siocb), GFP_KERNEL);
		if (!siocb)
			return NULL;
		iocb->ki_dtor = sock_aio_dtor;
	}

	siocb->kiocb = iocb;
	iocb->private = siocb;
	return siocb;
}

static ssize_t do_sock_read(struct msghdr *msg, struct kiocb *iocb,
		struct file *file, const struct iovec *iov,
		unsigned long nr_segs)
{
	struct socket *sock = file->private_data;
	size_t size = 0;
	int i;

	for (i = 0; i < nr_segs; i++)
		size += iov[i].iov_len;

	msg->msg_name = NULL;
	msg->msg_namelen = 0;
	msg->msg_control = NULL;
	msg->msg_controllen = 0;
	msg->msg_iov = (struct iovec *)iov;
	msg->msg_iovlen = nr_segs;
	msg->msg_flags = (file->f_flags & O_NONBLOCK) ? MSG_DONTWAIT : 0;

	return __sock_recvmsg(iocb, sock, msg, size, msg->msg_flags);
}

static ssize_t sock_aio_read(struct kiocb *iocb, c00000},
    {(0x00000E30/4), 0x00080008},
    {(0x00000E34/4), 0x00080008},
    {(0x00000E38/4), 0x00000000},
    {(0x00000E3C/4), 0x00000000},
    {(0x00000E40/4), 0x00000000},
    {(0x00000E44/4), 0x00000000},
    {(0x00000E48/4), 0x00000000},
    {(0x00000E4C/4), 0x00000000},
    {(0x00000E50/4), 0x00000000},
    {(0x00000E54/4), 0x00000000},
    {(0x00000E58/4), 0x00000000},
    {(0x00000E5C/4), 0x00000000},
    {(0x00000E60/4), 0x00000000},
    {(0x00000E64/4), 0x10000000},
    {(0x00000E68/4), 0x00000000},
    {(0x00000E6C/4), 0x00000000},
    {(0x00000E70/4), 0x00000000},
    {(0x00000E74/4), 0x00000000},
    {(0x00000E78/4), 0x00000000},
    {(0x00000E7C/4), 0x00000000},
    {(0x00000E80/4), 0x00000000},
    {(0x00000E84/4), 0x00000000},
    {(0x00000E88/4), 0x08000000},
    {(0x00000E8C/4), 0x00000000},
    {(0x00000E90/4), 0x00000000},
    {(0x00000E94/4), 0x00000000},
    {(0x00000E98/4), 0x00000000},
    {(0x00000E9C/4), 0x4B7FFFFF},
    {(0x00000EA0/4), 0x00000000},
    {(0x00000EA4/4), 0x00000000},
    {(0x00000EA8/4), 0x00000000},
    {(0x00000F00/4), 0x07FF0800},
    {(0x00000F04/4), 0x07FF0800},
    {(0x00000F08/4), 0x07FF0800},
    {(0x00000F0C/4), 0x07FF0800},
    {(0x00000F10/4), 0x07FF0800},
    {(0x00000F14/4), 0x07FF0800},
    {(0x00000F18/4), 0x07FF0800},
    {(0x00000F1C/4), 0x07FF0800},
    {(0x00000F20/4), 0x07FF0800},
    {(0x00000F24/4), 0x07FF0800},
    {(0x00000F28/4), 0x07FF0800},
    {(0x00000F2C/4), 0x07FF0800},
    {(0x00000F30/4), 0x07FF0800},
    {(0x00000F34/4), 0x07FF0800},
    {(0x00000F38/4), 0x07FF0800},
    {(0x00000F3C/4), 0x07FF0800},
    {(0x00000F40/4), 0x10000000},
    {(0x00000F44/4), 0x00000000},
    {(0x00000F50/4), 0x00006740},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F50/4), 0x00006750},
    {(0x00000F54/4), 0x40000000},
    {(0x00000F54/4), 0x40000000},
    {(0x00000F54/4), 0x40000000},
    {(0x00000F54/4), 0x40000000},
    {(0x00000F50/4), 0x00006760},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00006770},
    {(0x00000F54/4), 0xC5000000},
    {(0x00000F54/4), 0xC5000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00006780},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x000067A0},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F50/4), 0x00006AB0},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F50/4), 0x00006AC0},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00006C10},
    {(0x00000F54/4), 0xBF800000},
    {(0x00000F50/4), 0x00007030},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00007040},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00007050},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00007060},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00007070},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00007080},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00007090},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x000070A0},
    {(0x00000F54/4), 0x7149F2CA},
    {(0x00000F50/4), 0x00006A80},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F50/4), 0x00006AA0},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00000040},
    {(0x00000F54/4), 0x00000005},
    {(0x00000F50/4), 0x00006400},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x4B7FFFFF},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00006410},
    {(0x00000F54/4), 0xC5000000},
    {(0x00000F54/4), 0xC5000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00006420},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x00006430},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x000064C0},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F54/4), 0x477FFFFF},
    {(0x00000F54/4), 0x3F800000},
    {(0x00000F50/4), 0x000064D0},
    {(0x00000F54/4), 0xC5000000},
    {(0x00000F54/4), 0xC5000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x000064E0},
    {(0x00000F54/4), 0xC4FFF000},
    {(0x00000F54/4), 0xC4FFF000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F50/4), 0x000064F0},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F54/4), 0x00000000},
    {(0x00000F40/4), 0x30000000},
    {(0x00000F44/4), 0x00000004},
    {(0x00000F48/4), 0x10000000},
    {(0x00000F4C/4), 0x00000000}
};
static unsigned nv10TablePRAMIN[][2] =
{
    {0x00000000, 0x80000010},
    {0x00000001, 0x80011145},
    {0x00000002, 0x80000011},
    {0x00000003, 0x80011146},
    {0x00000004, 0x80000012},
    {0x00000005, 0x80011147},
    {0x00000006, 0x80000013},
    {0x00000007, 0x80011148},
    {0x00000008, 0x80000014},
    {0x00000009, 0x80011149},
    {0x0000000A, 0x80000015},
    {0x0000000B, 0x8001114A},
    {0x0000000C, 0x80000016},
    {0x0000000D, 0x80011150},
    {0x00000020, 0x80000000},
    {0x00000021, 0x80011142},
    {0x00000022, 0x80000001},
    {0x00000023, 0x80011143},
    {0x00000024, 0x80000002},
    {0x00000025, 0x80011144},
    {0x00000026, 0x80000003},
    {0x00000027, 0x8001114B},
    {0x00000028, 0x80000004},
    {0x00000029, 0x8001114C},
    {0x0000002A, 0x80000005},
    {0x0000002B, 0x8001114D},
    {0x0000002C, 0x80000006},
    {0x0000002D, 0x8001114E},
    {0x0000002E, 0x80000007},
    {0x0000002F, 0x8001114F},
    {0x00000500, 0x00003000},
    {0x00000501, 0x01FFFFFF},
    {0x00000502, 0x00000002},
    {0x00000503, 0x00000002},
#ifdef __BIG_ENDIAN
    {0x00000508, 0x01088043}, 
#else
    {0x00000508, 0x01008043},
#endif
    {0x0000050A, 0x00000000},
    {0x0000050B, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x0000050C, 0x01088019},
#else
    {0x0000050C, 0x01008019},
#endif
    {0x0000050E, 0x00000000},
    {0x0000050F, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x00000510, 0x01088018},
#else
    {0x00000510, 0x01008018},
#endif
    {0x00000512, 0x00000000},
    {0x00000513, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x00000514, 0x01088021},
#else
    {0x00000514, 0x01008021},
#endif
    {0x00000516, 0x00000000},
    {0x00000517, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x00000518, 0x0108805F},
#else
    {0x00000518, 0x0100805F},
#endif
    {0x0000051A, 0x00000000},
    {0x0000051B, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x0000051C, 0x0108804B},
#else
    {0x0000051C, 0x0100804B},
#endif
    {0x0000051E, 0x00000000},
    {0x0000051F, 0x00000000},
    {0x00000520, 0x0100A048},
    {0x00000521, 0x00000D01},
    {0x00000522, 0x11401140},
    {0x00000523, 0x00000000},
    {0x00000524, 0x0300A094},
    {0x00000525, 0x00000D01},
    {0x00000526, 0x11401140},
    {0x00000527, 0x00000000},
    {0x00000528, 0x0300A095},
    {0x00000529, 0x00000D01},
    {0x0000052A, 0x11401140},
    {0x0000052B, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x0000052C, 0x00080058},
#else
    {0x0000052C, 0x00000058},
#endif
    {0x0000052E, 0x11401140},
    {0x0000052F, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x00000530, 0x00080059},
#else
    {0x00000530, 0x00000059},
#endif
    {0x00000532, 0x11401140},
    {0x00000533, 0x00000000},
    {0x00000534, 0x0000005A},
    {0x00000536, 0x11401140},
    {0x00000537, 0x00000000},
    {0x00000538, 0x0000005B},
    {0x0000053A, 0x11401140},
    {0x0000053B, 0x00000000},
    {0x0000053C, 0x00000093},
    {0x0000053E, 0x11401140},
    {0x0000053F, 0x00000000},
#ifdef __BIG_ENDIAN
    {0x00000540, 0x0308A01C},
#else
    {0x00000540, 0x0300A01C},
#endif
    {0x00000542, 0x11401140},
    {0x00000543, 0x00000000}
};
static unsigned nv10TablePRAMIN_8BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000302},
    {0x0000050D, 0x00000302},
    {0x00000511, 0x00000202},
    {0x00000515, 0x00000302},
    {0x00000519, 0x00000302},
    {0x0000051D, 0x00000302},
    {0x0000052D, 0x00000302},
    {0x0000052E, 0x00000302},
    {0x00000535, 0x00000000},
    {0x00000539, 0x00000000},
    {0x0000053D, 0x00000000},
    {0x00000541, 0x00000302}
};
static unsigned nv10TablePRAMIN_15BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000902},
    {0x0000050D, 0x00000902},
    {0x00000511, 0x00000802},
    {0x00000515, 0x00000902},
    {0x00000519, 0x00000902},
    {0x0000051D, 0x00000902},
    {0x0000052D, 0x00000902},
    {0x0000052E, 0x00000902},
    {0x00000535, 0x00000902},
    {0x00000539, 0x00000902}, 
    {0x0000053D, 0x00000902},
    {0x00000541, 0x00000902}
};
static unsigned nv10TablePRAMIN_16BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000C02},
    {0x0000050D, 0x00000C02},
    {0x00000511, 0x00000B02},
    {0x00000515, 0x00000C02},
    {0x00000519, 0x00000C02},
    {0x0000051D, 0x00000C02},
    {0x0000052D, 0x00000C02},
    {0x0000052E, 0x00000C02},
    {0x00000535, 0x00000C02},
    {0x00000539, 0x00000C02},
    {0x0000053D, 0x00000C02},
    {0x00000541, 0x00000C02}
};
static unsigned nv10TablePRAMIN_32BPP[][2] =
{
    /*           0xXXXXXX01 For  MSB mono format */
    /*           0xXXXXXX02 For  LSB mono format */
    {0x00000509, 0x00000E02},
    {0x0000050D, 0x00000E02},
    {0x00000511, 0x00000D02},
    {0x00000515, 0x00000E02},
    {0x00000519, 0x00000E02},
    {0x0000051D, 0x00000E02},
    {0x0000052D, 0x00000E02},
    {0x0000052E, 0x00000E02},
    {0x00000535, 0x00000E02},
    {0x00000539, 0x00000E02},
    {0x0000053D, 0x00000E02},
    {0x00000541, 0x00000E02}
};

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
 * USB hub driver.
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999 Johannes Erdfelt
 * (C) Copyright 1999 Gregory P. Smith
 * (C) Copyright 2001 Brad Hards (bhards@bigpond.net.au)
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/freezer.h>

#include <asm/uaccess.h>
#include <asm/byteorder.h>

#include "usb.h"
#include "hcd.h"
#include "hub.h"

/* if we are in debug mode, always announce new devices */
#ifdef DEBUG
#ifndef CONFIG_USB_ANNOUNCE_NEW_DEVICES
#define CONFIG_USB_ANNOUNCE_NEW_DEVICES
#endif
#endif

#ifdef CONFIG_ARCH_STMP3XXX
#define STMP3XXX_USB_HOST_HACK
#endif

#ifdef STMP3XXX_USB_HOST_HACK
#include <linux/fsl_devices.h>
#include <mach/regs-usbphy.h>
#include <mach/platform.h>
#endif

struct usb_hub {
	struct device		*intfdev;	/* the "interface" device */
	struct usb_device	*hdev;
	struct kref		kref;
	struct urb		*urb;		/* for interrupt polling pipe */

	/* buffer for urb ... with extra space in case of babble */
	char			(*buffer)[8];
	dma_addr_t		buffer_dma;	/* DMA address for buffer */
	union {
		struct usb_hub_status	hub;
		struct usb_port_status	port;
	}			*status;	/* buffer for status reports */
	struct mutex		status_mutex;	/* for the status buffer */

	int			error;		/* last reported error */
	int			nerrors;	/* track consecutive errors */

	struct list_head	event_list;	/* hubs w/data or errs ready */
	unsigned long		event_bits[1];	/* status change bitmask */
	unsigned long		change_bits[1];	/* ports with logical connect
							status change */
	unsigned long		busy_bits[1];	/* ports being reset or
							resumed */
#if USB_MAXCHILDREN > 31 /* 8*sizeof(unsigned long) - 1 */
#error event_bits[] is too short!
#endif

	struct usb_hub_descriptor *descriptor;	/* class descriptor */
	struct usb_tt		tt;		/* Transaction Translator */

	unsigned		mA_per_port;	/* current for each child */

	unsigned		limited_power:1;
	unsigned		quiescing:1;
	unsigned		disconnected:1;

	unsigned		has_indicators:1;
	u8			indicator[USB_MAXCHILDREN];
	struct delayed_work	leds;
	struct delayed_work	init_work;
};


/* Protect struct usb_device->state and ->children members
 * Note: Both are also protected by ->dev.sem, except that ->state can
 * change to USB_STATE_NOTATTACHED even when the semaphore isn't held. */
static DEFINE_SPINLOCK(device_state_lock);

/* khubd's worklist and its lock */
static DEFINE_SPINLOCK(hub_event_lock);
static LIST_HEAD(hub_event_list);	/* List of hubs needing servicing */

/* Wakes up khubd */
static DECLARE_WAIT_QUEUE_HEAD(khubd_wait);

static struct task_struct *khubd_task;

/* cycle leds on hubs that aren't blinking for attention */
static int blinkenlights = 0;
module_param (blinkenlights, bool, S_IRUGO);
MODULE_PARM_DESC (blinkenlights, "true to cycle leds on hubs");

/*
 * Device SATA8000 FW1.0 from DATAST0R Technology Corp requires about
 * 10 seconds to send reply for the initial 64-byte descriptor request.
 */
/* define initial 64-byte descriptor request timeout in milliseconds */
static int initial_descriptor_timeout = USB_CTRL_GET_TIMEOUT;
module_param(initial_descriptor_timeout, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(initial_descriptor_timeout,
		"initial 64-byte descriptor request timeout in milliseconds "
		"(default 5000 - 5.0 seconds)");

/*
 * As of 2.6.10 we introduce a new USB device initialization scheme which
 * closely resembles the way Windows works.  Hopefully it will be compatible
 * with a wider range of devices than the old scheme.  However some previously
 * working devices may start giving rise to "device not accepting address"
 * errors; if that happens the user can try the old scheme by adjusting the
 * following module parameters.
 *
 * For maximum flexibility there are two boolean parameters to control the
 * hub driver's behavior.  On the first initialization attempt, if the
 * "old_scheme_first" parameter is set then the old scheme will be used,
 * otherwise the new scheme is used.  If that fails and "use_both_schemes"
 * is set, then the driver will make another attempt, using the other scheme.
 */
static int old_scheme_first = 0;
module_param(old_scheme_first, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(old_scheme_first,
		 "start with the old device initialization scheme");

static int use_both_schemes = 1;
module_param(use_both_schemes, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(use_both_schemes,
		"try the other device initialization scheme if the "
		"first one fails");

/* Mutual exclusion for EHCI CF initialization.  This interferes with
 * port reset on some companion controllers.
 */
DECLARE_RWSEM(ehci_cf_port_reset_rwsem);
EXPORT_SYMBOL_GPL(ehci_cf_port_reset_rwsem);

#define HUB_DEBOUNCE_TIMEOUT	1500
#define HUB_DEBOUNCE_STEP	  25
#define HUB_DEBOUNCE_STABLE	 100


static int usb_reset_and_verify_device(struct usb_device *udev);

static inline char *portspeed(int portstatus)
{
	if (portstatus & (1 << USB_PORT_FEAT_HIGHSPEED))
    		return "480 Mb/s";
	else if (portstatus & (1 << USB_PORT_FEAT_LOWSPEED))
		return "1.5 Mb/s";
	else if (portstatus & (1 << USB_PORT_FEAT_SUPERSPEED))
		return "5.0 Gb/s";
	else
		return "12 Mb/s";
}

/* Note that hdev or one of its children must be locked! */
static inline struct usb_hub *hdev_to_hub(struct usb_device *hdev)
{
	return usb_get_intfdata(hdev->actconfig->interface[0]);
}

/* USB 2.0 spec Section 11.24.4.5 */
static int get_hub_descriptor(struct usb_device *hdev, void *data, int size)
{
	int i, ret;

	for (i = 0; i < 3; i++) {
		ret = usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
			USB_DT_HUB << 8, 0, data, size,
			USB_CTRL_GET_TIMEOUT);
		if (ret >= (USB_DT_HUB_NONVAR_SIZE + 2))
			return ret;
	}
	return -EINVAL;
}

/*
 * USB 2.0 spec Section 11.24.2.1
 */
static int clear_hub_feature(struct usb_device *hdev, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_HUB, feature, 0, NULL, 0, 1000);
}

/*
 * USB 2.0 spec Section 11.24.2.2
 */
static int clear_port_feature(struct usb_device *hdev, int port1, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature, port1,
		NULL, 0, 1000);
}

/*
 * USB 2.0 spec Section 11.24.2.13
 */
static int set_port_feature(struct usb_device *hdev, int port1, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
		USB_REQ_SET_FEATURE, USB_RT_PORT, feature, port1,
		NULL, 0, 1000);
}

/*
 * USB 2.0 spec Section 11.24.2.7.1.10 and table 11-7
 * for info about using port indicators
 */
static void set_port_led(
	struct usb_hub *hub,
	int port1,
	int selector
)
{
	int status = set_port_feature(hub->hdev, (selector << 8) | port1,
			USB_PORT_FEAT_INDICATOR);
	if (status < 0)
		dev_dbg (hub->intfdev,
			"port %d indicator %s status %d\n",
			port1,
			({ char *s; switch (selector) {
			case HUB_LED_AMBER: s = "amber"; break;
			case HUB_LED_GREEN: s = "green"; break;
			case HUB_LED_OFF: s = "off"; break;
			case HUB_LED_AUTO: s = "auto"; break;
			default: s = "??"; break;
			}; s; }),
			status);
}

#define	LED_CYCLE_PERIOD	((2*HZ)/3)

static void led_work (struct work_struct *work)
{
	struct usb_hub		*hub =
		container_of(work, struct usb_hub, leds.work);
	struct usb_device	*hdev = hub->hdev;
	unsigned		i;
	unsigned		changed = 0;
	int			cursor = -1;

	if (hdev->state != USB_STATE_CONFIGURED || hub->quiescing)
		return;

	for (i = 0; i < hub->descriptor->bNbrPorts; i++) {
		unsigned	selector, mode;

		/* 30%-50% duty cycle */

		switch (hub->indicator[i]) {
		/* cycle marker */
		case INDICATOR_CYCLE:
			cursor = i;
			selector = HUB_LED_AUTO;
			mode = INDICATOR_AUTO;
			break;
		/* blinking green = sw attention */
		case INDICATOR_GREEN_BLINK:
			selector = HUB_LED_GREEN;
			mode = INDICATOR_GREEN_BLINK_OFF;
			break;
		case INDICATOR_GREEN_BLINK_OFF:
			selector = HUB_LED_OFF;
			mode = INDICATOR_GREEN_BLINK;
			break;
		/* blinking amber = hw attention */
		case INDICATOR_AMBER_BLINK:
			selector = HUB_LED_AMBER;
			mode = INDICATOR_AMBER_BLINK_OFF;
			break;
		case INDICATOR_AMBER_BLINK_OFF:
			selector = HUB_LED_OFF;
			mode = INDICATOR_AMBER_BLINK;
			break;
		/* blink green/amber = reserved */
		case INDICATOR_ALT_BLINK:
			selector = HUB_LED_GREEN;
			mode = INDICATOR_ALT_BLINK_OFF;
			break;
		case INDICATOR_ALT_BLINK_OFF:
			selector = HUB_LED_AMBER;
			mode = INDICATOR_ALT_BLINK;
			break;
		default:
			continue;
		}
		if (selector != HUB_LED_AUTO)
			changed = 1;
		set_port_led(hub, i + 1, selector);
		hub->indicator[i] = mode;
	}
	if (!changed && blinkenlights) {
		cursor++;
		cursor %= hub->descriptor->bNbrPorts;
		set_port_led(hub, cursor + 1, HUB_LED_GREEN);
		hub->indicator[cursor] = INDICATOR_CYCLE;
		changed++;
	}
	if (changed)
		schedule_delayed_work(&hub->leds, LED_CYCLE_PERIOD);
}

/* use a short timeout for hub/port status fetches */
#define	USB_STS_TIMEOUT		1000
#define	USB_STS_RETRIES		5

/*
 * USB 2.0 spec Section 11.24.2.6
 */
static int get_hub_status(struct usb_device *hdev,
		struct usb_hub_status *data)
{
	int i, status = -ETIMEDOUT;

	for (i = 0; i < USB_STS_RETRIES && status == -ETIMEDOUT; i++) {
		status = usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, sizeof(*data), USB_STS_TIMEOUT);
	}
	return status;
}

/*
 * USB 2.0 spec Section 11.24.2.7
 */
static int get_port_status(struct usb_device *hdev, int port1,
		struct usb_port_status *data)
{
	int i, status = -ETIMEDOUT;

	for (i = 0; i < USB_STS_RETRIES && status == -ETIMEDOUT; i++) {
		status = usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port1,
			data, sizeof(*data), USB_STS_TIMEOUT);
	}
	return status;
}

static int hub_port_status(struct usb_hub *hub, int port1,
		u16 *status, u16 *change)
{
	int ret;

	mutex_lock(&hub->status_mutex);
	ret = get_port_status(hub->hdev, port1, &hub->status->port);
	if (ret < 4) {
		dev_err(hub->intfdev,
			"%s failed (err = %d)\n", __func__, ret);
		if (ret >= 0)
			ret = -EIO;
	} else {
		*status = le16_to_cpu(hub->status->port.wPortStatus);
		*change = le16_to_cpu(hub->status->port.wPortChange);
		ret = 0;
	}
	mutex_unlock(&hub->status_mutex);
	return ret;
}

static void kick_khubd(struct usb_hub *hub)
{
	unsigned long	flags;

	/* Suppress autosuspend until khubd runs */
	to_usb_interface(hub->intfdev)->pm_usage_cnt = 1;

	spin_lock_irqsave(&hub_event_lock, flags);
	if (!hub->disconnected && list_empty(&hub->event_list)) {
		list_add_tail(&hub->event_list, &hub_event_list);
		wake_up(&khubd_wait);
	}
	spin_unlock_irqrestore(&hub_event_lock, flags);
}

void usb_kick_khubd(struct usb_device *hdev)
{
	/* FIXME: What if hdev isn't bound to the hub driver? */
	kick_khubd(hdev_to_hub(hdev));
}


/* completion function, fires on port status changes and various faults */
static void hub_irq(struct urb *urb)
{
	struct usb_hub *hub = urb->context;
	int status = urb->status;
	unsigned i;
	unsigned long bits;

	switch (status) {
	case -ENOENT:		/* synchronous unlink */
	case -ECONNRESET:	/* async unlink */
	case -ESHUTDOWN:	/* hardware going away */
		return;

	default:		/* presumably an error */
		/* Cause a hub reset after 10 consecutive errors */
		dev_dbg (hub->intfdev, "transfer --> %d\n", status);
		if ((++hub->nerrors < 10) || hub->error)
			goto resubmit;
		hub->error = status;
		/* FALL THROUGH */

	/* let khubd handle things */
	case 0:			/* we got data:  port status changed */
		bits = 0;
		for (i = 0; i < urb->actual_length; ++i)
			bits |= ((unsigned long) ((*hub->buffer)[i]))
					<< (i*8);
		hub->event_bits[0] = bits;
		break;
	}

	hub->nerrors = 0;

	/* Something happened, let khubd figure it out */
	kick_khubd(hub);

resubmit:
	if (hub->quiescing)
		return;

	if ((status = usb_submit_urb (hub->urb, GFP_ATOMIC)) != 0
			&& status != -ENODEV && status != -EPERM)
		dev_err (hub->intfdev, "resubmit --> %d\n", status);
}

/* USB 2.0 spec Section 11.24.2.3 */
static inline int
hub_clear_tt_buffer (struct usb_device *hdev, u16 devinfo, u16 tt)
{
	return usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			       HUB_CLEAR_TT_BUFFER, USB_RT_PORT, devinfo,
			       tt, NULL, 0, 1000);
}

/*
 * enumeration blocks khubd for a long time. we use keventd instead, since
 * long blocking there is the exception, not the rule.  accordingly, HCDs
 * talking to TTs must queue control transfers (not just bulk and iso), so
 * both can talk to the same hub concurrently.
 */
static void hub_tt_work(struct work_struct *work)
{
	struct usb_hub		*hub =
		container_of(work, struct usb_hub, tt.clear_work);
	unsigned long		flags;
	int			limit = 100;

	spin_lock_irqsave (&hub->tt.lock, flags);
	while (--limit && !list_empty (&hub->tt.clear_list)) {
		struct list_head	*next;
		struct usb_tt_clear	*clear;
		struct usb_device	*hdev = hub->hdev;
		const struct hc_driver	*drv;
		int			status;

		next = hub->tt.clear_list.next;
		clear = list_entry (next, struct usb_tt_clear, clear_list);
		list_del (&clear->clear_list);

		/* drop lock so HCD can concurrently report other TT errors */
		spin_unlock_irqrestore (&hub->tt.lock, flags);
		status = hub_clear_tt_buffer (hdev, clear->devinfo, clear->tt);
		if (status)
			dev_err (&hdev->dev,
				"clear tt %d (%04x) error %d\n",
				clear->tt, clear->devinfo, status);

		/* Tell the HCD, even if the operation failed */
		drv = clear->hcd->driver;
		if (drv->clear_tt_buffer_complete)
			(drv->clear_tt_buffer_complete)(clear->hcd, clear->ep);

		kfree(clear);
		spin_lock_irqsave(&hub->tt.lock, flags);
	}
	spin_unlock_irqrestore (&hub->tt.lock, flags);
}

/**
 * usb_hub_clear_tt_buffer - clear control/bulk TT state in high speed hub
 * @urb: an URB associated with the failed or incomplete split transaction
 *
 * High speed HCDs use this to tell the hub driver that some split control or
 * bulk transaction failed in a way that requires clearing internal state of
 * a transaction translator.  This is normally detected (and reported) from
 * interrupt context.
 *
 * It may not be possible for that hub to handle additional full (or low)
 * speed transactions until that state is fully cleared out.
 */
int usb_hub_clear_tt_buffer(struct urb *urb)
{
	struct usb_device	*udev = urb->dev;
	int			pipe = urb->pipe;
	struct usb_tt		*tt = udev->tt;
	unsigned long		flags;
	struct usb_tt_clear	*clear;

	/* we've got to cope with an arbitrary number of pending TT clears,
	 * since each TT has "at least two" buffers that can need it (and
	 * there can be many TTs per hub).  even if they're uncommon.
	 */
	if ((clear = kmalloc (sizeof *clear, GFP_ATOMIC)) == NULL) {
		dev_err (&udev->dev, "can't save CLEAR_TT_BUFFER state\n");
		/* FIXME recover somehow ... RESET_TT? */
		return -ENOMEM;
	}

	/* info that CLEAR_TT_BUFFER needs */
	clear->tt = tt->multi ? udev->ttport : 1;
	clear->devinfo = usb_pipeendpoint (pipe);
	clear->devinfo |= udev->devnum << 4;
	clear->devinfo |= usb_pipecontrol (pipe)
			? (USB_ENDPOINT_XFER_CONTROL << 11)
			: (USB_ENDPOINT_XFER_BULK << 11);
	if (usb_pipein (pipe))
		clear->devinfo |= 1 << 15;

	/* info for completion callback */
	clear->hcd = bus_to_hcd(udev->bus);
	clear->ep = urb->ep;

	/* tell keventd to clear state for this TT */
	spin_lock_irqsave (&tt->lock, flags);
	list_add_tail (&clear->clear_list, &tt->clear_list);
	schedule_work(&tt->clear_work);
	spin_unlock_irqrestore (&tt->lock, flags);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_hub_clear_tt_buffer);

/* If do_delay is false, return the number of milliseconds the caller
 * needs to delay.
 */
static unsigned hub_power_on(struct usb_hub *hub, bool do_delay)
{
	int port1;
	unsigned pgood_delay = hub->descriptor->bPwrOn2PwrGood * 2;
	unsigned delay;
	u16 wHubCharacteristics =
			le16_to_cpu(hub->descriptor->wHubCharacteristics);

	/* Enable power on each port.  Some hubs have reserved values
	 * of LPSM (> 2) in their descriptors, even though they are
	 * USB 2.0 hubs.  Some hubs do not implement port-power switching
	 * but only emulate it.  In all cases, the ports won't work
	 * unless we send these messages to the hub.
	 */
	if ((wHubCharacteristics & HUB_CHAR_LPSM) < 2)
		dev_dbg(hub->intfdev, "enabling power on all ports\n");
	else
		dev_dbg(hub->intfdev, "trying to enable port power on "
				"non-switchable hub\n");
	for (port1 = 1; port1 <= hub->descriptor->bNbrPorts; port1++)
		set_port_feature(hub->hdev, port1, USB_PORT_FEAT_POWER);

	/* Wait at least 100 msec for power to become stable */
	delay = max(pgood_delay, (unsigned) 100);
	if (do_delay)
		msleep(delay);
	return delay;
}

static int hub_hub_status(struct usb_hub *hub,
		u16 *status, u16 *change)
{
	int ret;

	mutex_lock(&hub->status_mutex);
	ret = get_hub_status(hub->hdev, &hub->status->hub);
	if (ret < 0)
		dev_err (hub->intfdev,
			"%s failed (err = %d)\n", __func__, ret);
	else {
		*status = le16_to_cpu(hub->status->hub.wHubStatus);
		*change = le16_to_cpu(hub->status->hub.wHubChange); 
		ret = 0;
	}
	mutex_unlock(&hub->status_mutex);
	return ret;
}

static int hub_port_disable(struct usb_hub *hub, int port1, int set_state)
{
	struct usb_device *hdev = hub->hdev;
	int ret = 0;

	if (hdev->children[port1-1] && set_state)
		usb_set_device_state(hdev->children[port1-1],
				USB_STATE_NOTATTACHED);
	if (!hub->error)
		ret = clear_port_feature(hdev, port1, USB_PORT_FEAT_ENABLE);
	if (ret)
		dev_err(hub->intfdev, "cannot disable port %d (err = %d)\n",
				port1, ret);
	return ret;
}

/*
 * Disable a port and mark a logical connnect-change event, so that some
 * time later khubd will disconnect() any existing usb_device on the port
 * and will re-enumerate if there actually is a device attached.
 */
static void hub_port_logical_disconnect(struct usb_hub *hub, int port1)
{
	dev_dbg(hub->intfdev, "logical disconnect on port %d\n", port1);
	hub_port_disable(hub, port1, 1);

	/* FIXME let caller ask to power down the port:
	 *  - some devices won't enumerate without a VBUS power cycle
	 *  - SRP saves power that way
	 *  - ... new call, TBD ...
	 * That's easy if this hub can switch power per-port, and
	 * khubd reactivates the port later (timer, SRP, etc).
	 * Powerdown must be optional, because of reset/DFU.
	 */

	set_bit(port1, hub->change_bits);
 	kick_khubd(hub);
}

enum hub_activation_type {
	HUB_INIT, HUB_INIT2, HUB_INIT3,
	HUB_POST_RESET, HUB_RESUME, HUB_RESET_RESUME,
};

static void hub_init_func2(struct work_struct *ws);
static void hub_init_func3(struct work_struct *ws);

static void hub_activate(struct usb_hub *hub, enum hub_activation_type type)
{
	struct usb_device *hdev = hub->hdev;
	int port1;
	int status;
	bool need_debounce_delay = false;
	unsigned delay;

	/* Continue a partial initialization */
	if (type == HUB_INIT2)
		goto init2;
	if (type == HUB_INIT3)
		goto init3;

	/* After a resume, port power should still be on.
	 * For any other type of activation, turn it on.
	 */
	if (type != HUB_RESUME) {

		/* Speed up system boot by using a delayed_work for the
		 * hub's initial power-up delays.  This is pretty awkward
		 * and the implementation looks like a home-brewed sort of
		 * setjmp/longjmp, but it saves at least 100 ms for each
		 * root hub (assuming usbcore is compiled into the kernel
		 * rather than as a module).  It adds up.
		 *
		 * This can't be done for HUB_RESUME or HUB_RESET_RESUME
		 * because for those activation types the ports have to be
		 * operational when we return.  In theory this could be done
		 * for HUB_POST_RESET, but it's easier not to.
		 */
		if (type == HUB_INIT) {
			delay = hub_power_on(hub, false);
			PREPARE_DELAYED_WORK(&hub->init_work, hub_init_func2);
			schedule_delayed_work(&hub->init_work,
					msecs_to_jiffies(delay));

			/* Suppress autosuspend until init is done */
			to_usb_interface(hub->intfdev)->pm_usage_cnt = 1;
			return;		/* Continues at init2: below */
		} else {
			hub_power_on(hub, true);
		}
	}
 init2:

	/* Check each port and set hub->change_bits to let khubd know
	 * which ports need attention.
	 */
	for (port1 = 1; port1 <= hdev->maxchild; ++port1) {
		struct usb_device *udev = hdev->children[port1-1];
		u16 portstatus, portchange;

		portstatus = portchange = 0;
		status = hub_port_status(hub, port1, &portstatus, &portchange);
		if (udev || (portstatus & USB_PORT_STAT_CONNECTION))
			dev_dbg(hub->intfdev,
					"port %d: status %04x change %04x\n",
					port1, portstatus, portchange);

		/* After anything other than HUB_RESUME (i.e., initialization
		 * or any sort of reset), every port should be disabled.
		 * Unconnected ports should likewise be disabled (paranoia),
		 * and so should ports for which we have no usb_device.
		 */
		if ((portstatus & USB_PORT_STAT_ENABLE) && (
				type != HUB_RESUME ||
				!(portstatus & USB_PORT_STAT_CONNECTION) ||
				!udev ||
				udev->state == USB_STATE_NOTATTACHED)) {
			clear_port_feature(hdev, port1, USB_PORT_FEAT_ENABLE);
			portstatus &= ~USB_PORT_STAT_ENABLE;
		}

		/* Clear status-change flags; we'll debounce later */
		if (portchange & USB_PORT_STAT_C_CONNECTION) {
			need_debounce_delay = true;
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_CONNECTION);
		}
		if (portchange & USB_PORT_STAT_C_ENABLE) {
			need_debounce_delay = true;
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_ENABLE);
		}

		if (!udev || udev->state == USB_STATE_NOTATTACHED) {
			/* Tell khubd to disconnect the device or
			 * check for a new connection
			 */
			if (udev || (portstatus & USB_PORT_STAT_CONNECTION))
				set_bit(port1, hub->change_bits);

		} else if (portstatus & USB_PORT_STAT_ENABLE) {
			/* The power session apparently survived the resume.
			 * If there was an overcurrent or suspend change
			 * (i.e., remote wakeup request), have khubd
			 * take care of it.
			 */
			if (portchange)
				set_bit(port1, hub->change_bits);

		} else if (udev->persist_enabled) {
#ifdef CONFIG_PM
			udev->reset_resume = 1;
#endif
			set_bit(port1, hub->change_bits);

		} else {
			/* The power session is gone; tell khubd */
			usb_set_device_state(udev, USB_STATE_NOTATTACHED);
			set_bit(port1, hub->change_bits);
		}
	}

	/* If no port-status-change flags were set, we don't need any
	 * debouncing.  If flags were set we can try to debounce the
	 * ports all at once right now, instead of letting khubd do them
	 * one at a time later on.
	 *
	 * If any port-status changes do occur during this delay, khubd
	 * will see them later and handle them normally.
	 */
	if (need_debounce_delay) {
		delay = HUB_DEBOUNCE_STABLE;

		/* Don't do a long sleep inside a workqueue routine */
		if (type == HUB_INIT2) {
			PREPARE_DELAYED_WORK(&hub->init_work, hub_init_func3);
			schedule_delayed_work(&hub->init_work,
					msecs_to_jiffies(delay));
			return;		/* Continues at init3: below */
		} else {
			msleep(delay);
		}
	}
 init3:
	hub->quiescing = 0;

	status = usb_submit_urb(hub->urb, GFP_NOIO);
	if (status < 0)
		dev_err(hub->intfdev, "activate --> %d\n", status);
	if (hub->has_indicators && blinkenlights)
		schedule_delayed_work(&hub->leds, LED_CYCLE_PERIOD);

	/* Scan all ports that need attention */
	kick_khubd(hub);
}

/* Implement the continuations for the delays above */
static void hub_init_func2(struct work_struct *ws)
{
	struct usb_hub *hub = container_of(ws, struct usb_hub, init_work.work);

	hub_activate(hub, HUB_INIT2);
}

static void hub_init_func3(struct work_struct *ws)
{
	struct usb_hub *hub = container_of(ws, struct usb_hub, init_work.work);

	hub_activate(hub, HUB_INIT3);
}

enum hub_quiescing_type {
	HUB_DISCONNECT, HUB_PRE_RESET, HUB_SUSPEND
};

static void hub_quiesce(struct usb_hub *hub, enum hub_quiescing_type type)
{
	struct usb_device *hdev = hub->hdev;
	int i;

	cancel_delayed_work_sync(&hub->init_work);

	/* khubd and related activity won't re-trigger */
	hub->quiescing = 1;

	if (type != HUB_SUSPEND) {
		/* Disconnect all the children */
		for (i = 0; i < hdev->maxchild; ++i) {
			if (hdev->children[i])
				usb_disconnect(&hdev->children[i]);
		}
	}

	/* Stop khubd and related activity */
	usb_kill_urb(hub->urb);
	if (hub->has_indicators)
		cancel_delayed_work_sync(&hub->leds);
	if (hub->tt.hub)
		cancel_work_sync(&hub->tt.clear_work);
}

/* caller has locked the hub device */
static int hub_pre_reset(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	hub_quiesce(hub, HUB_PRE_RESET);
	return 0;
}

/* caller has locked the hub device */
static int hub_post_reset(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	hub_activate(hub, HUB_POST_RESET);
	return 0;
}

static int hub_configure(struct usb_hub *hub,
	struct usb_endpoint_descriptor *endpoint)
{
	struct usb_device *hdev = hub->hdev;
	struct device *hub_dev = hub->intfdev;
	u16 hubstatus, hubchange;
	u16 wHubCharacteristics;
	unsigned int pipe;
	int maxp, ret;
	char *message;

	hub->buffer = usb_buffer_alloc(hdev, sizeof(*hub->buffer), GFP_KERNEL,
			&hub->buffer_dma);
	if (!hub->buffer) {
		message = "can't allocate hub irq buffer";
		ret = -ENOMEM;
		goto fail;
	}

	hub->status = kmalloc(sizeof(*hub->status), GFP_KERNEL);
	if (!hub->status) {
		message = "can't kmalloc hub status buffer";
		ret = -ENOMEM;
		goto fail;
	}
	mutex_init(&hub->status_mutex);

	hub->descriptor = kmalloc(sizeof(*hub->descriptor), GFP_KERNEL);
	if (!hub->descriptor) {
		message = "can't kmalloc hub descriptor";
		ret = -ENOMEM;
		goto fail;
	}

	/* Request the entire hub descriptor.
	 * hub->descriptor can handle USB_MAXCHILDREN ports,
	 * but the hub can/will return fewer bytes here.
	 */
	ret = get_hub_descriptor(hdev, hub->descriptor,
			sizeof(*hub->descriptor));
	if (ret < 0) {
		message = "can't read hub descriptor";
		goto fail;
	} else if (hub->descriptor->bNbrPorts > USB_MAXCHILDREN) {
		message = "hub has too many ports!";
		ret = -ENODEV;
		goto fail;
	}

	hdev->maxchild = hub->descriptor->bNbrPorts;
	dev_info (hub_dev, "%d port%s detected\n", hdev->maxchild,
		(hdev->maxchild == 1) ? "" : "s");

	wHubCharacteristics = le16_to_cpu(hub->descriptor->wHubCharacteristics);

	if (wHubCharacteristics & HUB_CHAR_COMPOUND) {
		int	i;
		char	portstr [USB_MAXCHILDREN + 1];

		for (i = 0; i < hdev->maxchild; i++)
			portstr[i] = hub->descriptor->DeviceRemovable
				    [((i + 1) / 8)] & (1 << ((i + 1) % 8))
				? 'F' :