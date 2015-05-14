#ifndef __SAND_TYPES_H
#define __SAND_TYPES_H

/*..................... C libraries ................... */
/* Must be included before the redefinition of the 'int' type */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*..................... Predefined C types ................... */
#ifndef uchar
#define uchar   unsigned char
#endif
#ifndef ushort
#define ushort  unsigned short
#endif
#ifndef ulong
#define ulong   unsigned long
#endif

#ifndef UCHAR
#define UCHAR   unsigned char
#endif
#ifndef USHORT
#define USHORT  unsigned short
#endif
#ifndef ULONG
#define ULONG   unsigned long
#endif

/*
  For simulating old applications under Sim'Elite Win32 :
    redefinition of the integer to the short type (2 bytes size).
  Warning : There must not be any inclusion of the compiler's include files after this
  redefinition, otherwise, the functions prototype will not correspond to the real function.
  Remark : in every case, for compatibility reason between 8-16 bits and 32 bits platforms,
  the use of integer data should be avoided and they should be replaced by short data type.
*/
#ifdef REDEFINE_INT_TO_SHORT
#ifndef int
#define int     short
#endif
#endif

#ifndef uint
#define uint    unsigned int
#endif
#ifndef UINT
#define UINT    unsigned int
#endif

/*....................... Booleans .....................*/

#ifndef bool
//#define bool 	  uchar
#endif
#ifndef FALSE
#define FALSE  	  0x00
#endif
#ifndef TRUE
#define TRUE   	  0xFF
#endif
#ifndef true
#define true      0xFF
#endif
#ifndef false
#define false     0x00
#endif

/* RQ : different of FAUX and VRAI */

#define YES        0
#define NO         1

#define OK         0
#define KO         1
#define NOK        1

/*................... Words unknown for simulation on PC ...............*/

#if !defined(__C51__) && !defined(_XA_)
#ifndef code
#define code
#endif
#ifndef xdata
#define xdata
#endif
#ifndef bit
#define bit
#endif
#ifndef large
#define large
#endif
#ifndef small
#define small
#endif
#ifndef reentrant
#define reentrant
#endif
#endif

/*..................... Words unknown for C51 .................*/

#ifdef __C51__   // if compiling for c51
#ifndef far
#define far
#endif
#endif

/*..................... Words unknown for XA .................*/

#ifdef _XA_
#ifndef xdata
#define xdata
#endif
#ifndef large
#define large
#endif
#ifndef small
#define small
#endif
#ifndef reentrant
#define reentrant
#endif
#endif

#endif //__SAND_TYPES_H
