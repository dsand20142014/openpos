/*
	Library Function
--------------------------------------------------------------------------
	FILE  libmisc.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-02-19		Xiaoxi Jiang
    Last modified :	2003-02-19		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include <tools.h>


unsigned char MISC_GetCharBitNB(unsigned char ucCh,unsigned char ucBit)
{
    unsigned char ucMask;
    unsigned char ucNB;

    ucMask = 0x80;
    ucNB = 0;
    if( !ucBit )
    {
        ucCh = ~ucCh;
    }
    do{
        if( ucCh & ucMask )
        {
            ucNB ++;
        }
        ucMask = ucMask >> 1;
    }while( ucMask );
    return(ucNB);
}
