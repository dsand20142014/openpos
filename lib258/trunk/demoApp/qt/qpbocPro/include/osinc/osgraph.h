/*
	Operating System
--------------------------------------------------------------------------
	FILE  osgraph.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSGRAPH_H
#define __OSGRAPH_H

/* Select the character matrix for the EFT terminal display, for */
/* the pinpad display and for the printer */
void Os__select_CM(uchar display_CM, uchar pinpad_CM, uchar printer_CM);

/* List of existing character matrix : */
#define STANDARD    0
#define VIETNAMESE  1
#define HEBREW      2
#define SPECIAL     9
#define CHINESE     0x11
#define ARAB        0x12

/* Forward paper of nb step */
uchar Os__prn_forward(uchar nr_of_steps);

/* Backward paper of nb step */
uchar Os__prn_backward(uchar nr_of_steps);

/* Line display in GB2312 format */
uchar Os__GB2312_disp(uchar,uchar * pt_msg);

/* Line and column display in GB2312 format */
uchar Os__GB2312_display(uchar,uchar,uchar * pt_msg);

/* Graphic display */
uchar Os__graph_display(uchar,uchar,uchar * pt_msg,uchar LEN)reentrant;

/* GB2312 printing */
uchar Os__GB2312_xprint(uchar * pt_msg, uchar size);

/* Graphic printing */
uchar Os__graph_xprint(uchar * pt_msg,uchar LEN);

/* Light on the display */
uchar Os__light_on(void);

/* Light off the display */
uchar Os__light_off(void);

/* Conversion from ASCII string (max 24 characters) to GB2312 codes string */
uchar * Os__conv_ascii_GB2312(uchar * pt_ascii);

/* Conversion from ASCII string (characters from ' ' to '_') (max 32 char.) */
/* to ASMO string (reverse order) */
uchar * Os__conv_ascii_ASMO(uchar * pt_ascii);

/* Line and column display in GB2312 format on PIN-PAD */
uchar Os__GB2312_display_pp(uchar,uchar,uchar * pt_msg);

/* Graphic display on PIN-PAD */
uchar Os__graph_display_pp(uchar,uchar,uchar * pt_msg,uchar LEN)reentrant;

/* Download new characters in the matrix 9 */
uchar Os__new_character(uchar TPE_or_PINPAD, uchar charac_nr, uchar * drawing);

#endif

