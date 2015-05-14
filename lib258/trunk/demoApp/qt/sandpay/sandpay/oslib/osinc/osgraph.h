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
#include <sand_types.h>
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

#endif

