/*******************************************/
//
//					NEWSYNC.H
//
/*******************************************/


#ifndef __NEWSYNC_H__
#define __NEWSYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MACRO_CODE		0xff
#define SET_DATA_CODE   0x0f

#define EOC_CODE			0x00
#define CLR_RST_CODE		0x02
#define SET_RST_CODE		0x04
#define RST_10US_CODE	0x06
#define CLR_CLK_CODE		0x08
#define SET_CLK_CODE		0x0a
#define CLK_5US_CODE		0x0c
#define CLK_10US_CODE	0x0e
#define CLR_C4_CODE		0x10
#define SET_C4_CODE		0x12
#define CLR_C8_CODE		0x14
#define SET_C8_CODE		0x16
#define VPP_IDLE_CODE	0x18
#define VPP_12V_CODE		0x1a
#define VPP_15V_CODE		0x1c
#define VPP_21V_CODE		0x1e
#define CLR_IO_CODE		0x20
#define SET_IO_CODE		0x22
#define MSB_TO_IO_CODE	0x24
#define LSB_TO_IO_CODE	0x26
#define IO_TO_MSB_CODE	0x28
#define IO_TO_LSB_CODE	0x2a
#define TST_IO_HI_CODE  0x2c
#define TST_IO_LO_CODE	0x2e
#define EXE_BADLVL_CODE	0x30
#define WT_10US_CODE		0x32
#define WT_200US_CODE	0x34

#define ICC_STATE_BAD_LEVEL	'K'
#define ICC_STATE_CODE_LENGTH	'L'
#define ICC_STATE_CODE_PARAM	'M'
#define ICC_STATE_SET_DATA		'N'
#define ICC_STATE_EOC			'O'
#define ICC_STATE_CODE_VALUE	'P'
#define ICC_STATE_MACRO			'Q'
#define ICC_STATE_END_TST		'R'


#ifdef __cplusplus
}
#endif

#endif
