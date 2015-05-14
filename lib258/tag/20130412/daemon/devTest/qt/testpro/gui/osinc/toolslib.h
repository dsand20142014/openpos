/*
	Operating System
--------------------------------------------------------------------------
	FILE  toolslib.h
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


#ifndef __TOOLSLIB_H
#define __TOOLSLIB_H
/*
#include <typelib.h>
*/

#include "osicc.h"

#ifndef UCHAR
#define UCHAR unsigned char
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef BOOL
#define BOOL unsigned char
#endif
#ifndef bool
//#define bool unsigned char
#endif
#ifndef ULONG
#define ULONG unsigned long
#endif
#ifndef USHORT
#define USHORT unsigned short
#endif
#ifndef UINT
#define UINT unsigned int
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Prototypes and literals ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

/* General structure :  Ptd - destination data pointer
                        Pts - source data pointer
                        Pt  - source and destinataion data pointer
                        Ld  - destination data length (bytes)
                        Ls  - source data length (bytes)
                        Lg  - source and destination data length (bytes)

   resulting parameters :
   . pointer to the byte immediatly free of the Bcd field resulting
   . pointer to the '\0' character of the Ascii string resulting
   . value of the result if it is a C type ( char , short , long )

 Remark : this informations will be precised for the special cases */

/* Remark : functions refering to 'int' data type assume the integer size is
   16 bits, i.e. it, in fact, correspond to the 'short' data type.
*/

/* Transferring functions */
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
/* M1BIB1.C */
void move_inc_ptr (
    UCHAR **Ptd,   /* Destination pointer is increased */
    UCHAR *Pts,
    UCHAR Len
) ;

UCHAR *move_255 (
    UCHAR *Ptd,
    UCHAR *Pts,
    UCHAR Len
) ;

/* M1BIB2.C */
/* Transferring a Length + Values to a string */
UCHAR *mov_to_str (
    UCHAR *Ptd ,
    UCHAR *Pts
) ;

/* Conversion functions */
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

/* M1BIB3.C */
/* EBCDIC values chain -> Ascii values chain */
UCHAR *asc_ebcdic(
    UCHAR *Pt,
    UCHAR Len
) ;

/* Ascii values chain -> EBCDIC values chain */
UCHAR ebcdic_asc(
    UCHAR *Pt,
    UCHAR Len
) ;

/* M1BIB4.C */
/* Bcd values chain -> C string type */
UCHAR *bcd_str(
    UCHAR *Ptd ,
    UCHAR *Pts ,
    UCHAR Len     /* source string length in bytes */
) ;

/* Bcd values chain -> Ascii values chain */
UCHAR *bcd_asc(
    UCHAR *Ptd ,
    UCHAR *Pts ,
    UCHAR Ls     /* source string length in quartets */
) ;

/* M1BIB5.C */
/* Bcd values chain -> Unsigned Long or after "casting" unsigned short or UCHAR */
ULONG bcd_long(
    UCHAR *Pts ,
    UCHAR Ls
) ;

/* M1BIB6.C */
/* Hexa values table -> C string type */
UCHAR *hex_str(
    UCHAR *Ptd ,
    UCHAR *Pts ,
    UCHAR Len     /* source string length in quartets */
) ;

/* Hexa values table -> Ascii values chain */
UCHAR *hex_asc(
    UCHAR *Ptd ,
    UCHAR *Pts ,
    UCHAR Len     /* source string length in quartets */
) ;

/* M1BIB7.C */
/* Ascii values chain -> Bcd values table */
UCHAR *asc_bcd(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts,
    UCHAR Ls
) ;
/* M1BIB8.C */
/* Ascii values chain -> Hexa values table */
UCHAR *asc_hex(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts,
    UCHAR Ls
) ;

/* M1BIB9.C */
/* Ascii values chain -> Unsigned Long */
ULONG asc_long(
    UCHAR *Pts,
    UCHAR Ls
) ;

/* M1BIB10.C */
/* C string -> EBCDIC values table */
UCHAR *str_ebcdic(
    UCHAR *Pt
) ;

/* M1BIB11.C */
/* C string -> Bcd values table */
UCHAR *str_bcd(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts
) ;

/* M1BIB12.C */
/* C string -> Hexa values table */
UCHAR *str_hex(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts
) ;

/* M1BIB13.C */
/* Ascii values chain -> Unsigned Long */
ULONG str_long(
    UCHAR *Pts
) ;

/* M1BIB14.C */
/* Unsigned Long -> BCD values chain */
UCHAR *long_bcd(
    UCHAR *Ptd ,
    UCHAR Ld,
    ULONG *Pts
) ;

/* M1BIB15.C */
/* Unsigned Long -> C string */
UCHAR *long_str(
    UCHAR *Ptd ,
    UCHAR Ld,
    ULONG *Pts
) ;

/* Unsigned Long -> Ascii values chain */
UCHAR *long_asc(
    UCHAR *Ptd ,
    UCHAR Ld,
    ULONG *Pts
) ;

/* M1BIB16.C */
/* Unsigned Long -> Table contening an Unsigned Long */
UCHAR *long_tab(
    UCHAR *Ptd,
    UCHAR Ld,
    ULONG *Pts
) ;

/* M1BIB48.C + M1BIB17.C */
/* Unsigned short -> BCD values chain */
UCHAR *short_bcd(
    UCHAR *Ptd ,
    UCHAR Ld,
    unsigned short  *Pts
) ;
UCHAR *int_bcd(
    UCHAR *Ptd ,
    UCHAR Ld,
    UINT  *Pts
) ;

/* M1BIB49.C + M1BIB18.C */
/* Unsigned short -> C string */
UCHAR *short_str(
    UCHAR *Ptd ,
    UCHAR Ld,
    unsigned short  *Pts
) ;
UCHAR *int_str(
    UCHAR *Ptd ,
    UCHAR Ld,
    UINT  *Pts
) ;

/* M1BIB50.C + M1BIB19.C */
UCHAR *short_tab (
    UCHAR *Ptd,
    UCHAR Ld,
    unsigned short  *Pts
) ;
UCHAR *int_tab (
    UCHAR *Ptd,
    UCHAR Ld,
    UINT  *Pts
) ;

/* M1BIB20.C */
/* Unsigned char -> C string */
UCHAR *char_str(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts
) ;

/* M1BIB21.C */
/* Unsigned char -> BCD values chain */
UCHAR *char_bcd(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts
) ;

/* M1BIB22.C */
/* Table contening an Unsigned Long -> Unsigned Long
   or after "casting" unsigned short or UCHAR */
ULONG tab_long(
    UCHAR *Pts,
    UCHAR Ls
);

/* M1BIB23.C */
/* TLV message setting */
BOOL extract_TLV (
    UCHAR **Pt_buf,      /* Area to treat */
    unsigned short  *Pt_maxlen,    /* Number of bytes */
    unsigned short  *Pt_len,       /* L */
    UCHAR *Pt_id         /* T */
);

/* M1BIB24.C */
/* Ascii numeric table verification */
BOOL is_asc_digit (
    UCHAR *Pts,
    UCHAR Ls
);


/* M1BIB25.C */
/* C numeric string verification */
BOOL is_str_digit ( UCHAR *Pts );

/* M1BIB26.C */
/* Lrc computing on a length */
UCHAR lrc_l( UCHAR *Pts, unsigned short Ls, UCHAR Init_val );

/* M1BIB27.C */
/* Difference between two dates */
UCHAR sub_date (
    UCHAR *Ptr,  /* result date C pointer in BCD */
    UCHAR *Pt1,  /* date A pointer in BCD */
    UCHAR *Pt2,  /* date B pointer in BCD */
    UCHAR B_ov   /* overflow byte for time difference */
);

/* Difference between two times */
UCHAR sub_time (
    UCHAR *Ptr,  /* result time C pointer in BCD */
    UCHAR *Pt1,  /* time A pointer in BCD */
    UCHAR *Pt2   /* time B pointer in BCD */
);

/* Add days to a date */
UCHAR add_date_day (
    UCHAR *Pt1,  /* date pointer in bcd and result */
    UCHAR B_ov   /* Number of days < 63 in bcd */
);

/* Add a time in BCD and a waiting delay in ascii */
UCHAR add_time_delay (
    UCHAR *Ptr,  /* time pointer (in BCD) and result */
    UCHAR *Pt2   /* pointer to the waiting delay in ascii */
);

/* M1BIB28.C */
/* Add two bytes in bcd */
UCHAR addbcd( UCHAR A, UCHAR B );

/* Substract two bytes in bcd */
UCHAR subbcd( UCHAR A, UCHAR B );

/* M1BIB29.C */
/* Luhn key verification on length + Iso 2 + '?' + \000 */
UCHAR luhn_key_ok( UCHAR *Pt); /* Remark: = TRUE if correct, luhn key value else */

/* M1BIB30.C */
/* Format an ascii time in xxHxx */
void format_time( UCHAR *Ptd, UCHAR *Pts);

/* M1BIB31.C */
/* Format an ascii date in xx/xx/xx */
void format_date( UCHAR *Ptd, UCHAR *Pts);

/* M1BIB32.C */
/* Compute the modulo 7 key */
UCHAR mod7_key( UCHAR *Pts );

/* M1BIB33.C */
/* Verify if a data area is null */
BOOL memnull( UCHAR *Pt, UCHAR Len);

/* M1BIB34.C */
/* Length Type Value message */
BOOL extract_LTV (
    UCHAR **Pt_buf,      /* Area to treat */
    unsigned short  *Pt_maxlen,    /* Number of bytes */
    unsigned short  *Pt_len,       /* L */
    unsigned short  *Pt_id         /* T */
);
/* M1BIB51.C + M1BIB35.C */
/* Unsigned short -> Ascii values chain */
UCHAR *short_asc(
    UCHAR *Ptd ,
    UCHAR Ld,
    unsigned short  *Pts
) ;
UCHAR *int_asc(
    UCHAR *Ptd ,
    UCHAR Ld,
    UINT  *Pts
) ;

/* M1BIB36.C */
/* Unsigned char -> Ascii values chain */
UCHAR *char_asc(
    UCHAR *Ptd ,
    UCHAR Ld,
    UCHAR *Pts
) ;

/* M1BIB37.C */
/* Increase a short and checks if it becomes null */
void inc_number ( unsigned short *Pt_number );

/* M1BIB38.C */
/* Format an amount */
void format_amount(UCHAR *Ptd, ULONG * Pt_amount, UCHAR Nr_digits);
/* Remark : Nb_digit is coded ! */

/* M1BIB39.C */
/* Time coherency checking */
BOOL check_time( UCHAR *time);

/* M1BIB40.C */
/* Date in format DDMMYY coherency checking : not checked on YY */
BOOL check_date( UCHAR *date );

/* M1BIB41.C */
/* Checks the Luhn key on a card number in ascii */
UCHAR luhn_key( UCHAR    *card_nr);

/* M1BIB42.C */
/* Add 2 times in ASCII */
void add_time(UCHAR *asc_time1, UCHAR *asc_time2);

/* M1BIB43.C */
/* Lrc of an area between two pointers */
UCHAR lrc(UCHAR *pointer1, UCHAR *pointer2, UCHAR init_value);

/* M1BIB44.C */
/* Sum 2 chain in Ascii */
void add_ascii(UCHAR *string1, UCHAR *string2, UCHAR length);

/* M1BIB45.C */
/* Differences between 2 chain in Ascii */
void sub_ascii(UCHAR *string1, UCHAR *string2, UCHAR length);

/* M1BIB46.C */
/* Substract or add a month to the date in BCD format (YYMM) which is pointed  */
void date_add_or_sub_1_month( UCHAR Add, UCHAR *Pt_date );

/* M1BIB47.C */
/* Compute the checksum with a starting value on an area */
unsigned short checksum( UCHAR *Pt, UCHAR *Ptf, unsigned short Val );
#define CHKINIT   0x5555       /* Starting value type dissymetric */

#endif

