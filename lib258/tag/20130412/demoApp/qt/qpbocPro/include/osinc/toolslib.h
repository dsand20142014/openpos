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

#include <typelib.h>

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
                   uchar **Ptd,   /* Destination pointer is increased */
                   uchar *Pts,
                   uchar Len
                  ) ;

uchar *move_255 (
                uchar *Ptd,
                uchar *Pts,
                uchar Len
               ) ;

/* M1BIB2.C */
/* Transferring a Length + Values to a string */
uchar *mov_to_str (
                  uchar *Ptd ,
                  uchar *Pts
                 ) ;

/* Conversion functions */
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

/* M1BIB3.C */
/* EBCDIC values chain -> Ascii values chain */
uchar *asc_ebcdic(
                 uchar *Pt,
                 uchar Len
                ) ;

/* Ascii values chain -> EBCDIC values chain */
uchar ebcdic_asc(
                 uchar *Pt,
                 uchar Len
                ) ;

/* M1BIB4.C */
/* Bcd values chain -> C string type */
uchar *bcd_str(
               uchar *Ptd ,
               uchar *Pts ,
               uchar Len     /* source string length in bytes */
              ) ;

/* Bcd values chain -> Ascii values chain */
uchar *bcd_asc(
               uchar *Ptd ,
               uchar *Pts ,
               uchar Ls     /* source string length in quartets */
              ) ;

/* M1BIB5.C */
/* Bcd values chain -> Unsigned Long or after "casting" ushort or uchar */
ulong bcd_long(
               uchar *Pts ,
               uchar Ls
              ) ;

/* M1BIB6.C */
/* Hexa values table -> C string type */
uchar *hex_str(
               uchar *Ptd ,
               uchar *Pts ,
               uchar Len     /* source string length in quartets */
              ) ;

/* Hexa values table -> Ascii values chain */
uchar *hex_asc(
               uchar *Ptd ,
               uchar *Pts ,
               uchar Len     /* source string length in quartets */
              ) ;

/* M1BIB7.C */
/* Ascii values chain -> Bcd values table */
uchar *asc_bcd(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts,
               uchar Ls
              ) ;
/* M1BIB8.C */
/* Ascii values chain -> Hexa values table */
uchar *asc_hex(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts,
               uchar Ls
              ) ;

/* M1BIB9.C */
/* Ascii values chain -> Unsigned Long */
ulong asc_long(
               uchar *Pts,
               uchar Ls
              ) ;

/* M1BIB10.C */
/* C string -> EBCDIC values table */
uchar *str_ebcdic(
                 uchar *Pt
                ) ;

/* M1BIB11.C */
/* C string -> Bcd values table */
uchar *str_bcd(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts
              ) ;

/* M1BIB12.C */
/* C string -> Hexa values table */
uchar *str_hex(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts
              ) ;

/* M1BIB13.C */
/* Ascii values chain -> Unsigned Long */
ulong str_long(
               uchar *Pts
              ) ;

/* M1BIB14.C */
/* Unsigned Long -> BCD values chain */
uchar *long_bcd(
               uchar *Ptd ,
               uchar Ld,
               ulong *Pts
              ) ;

/* M1BIB15.C */
/* Unsigned Long -> C string */
uchar *long_str(
               uchar *Ptd ,
               uchar Ld,
               ulong *Pts
              ) ;

/* Unsigned Long -> Ascii values chain */
uchar *long_asc(
               uchar *Ptd ,
               uchar Ld,
               ulong *Pts
              ) ;

/* M1BIB16.C */
/* Unsigned Long -> Table contening an Unsigned Long */
uchar *long_tab(
               uchar *Ptd,
               uchar Ld,
               ulong *Pts
              ) ;

/* M1BIB48.C + M1BIB17.C */
/* Unsigned short -> BCD values chain */
uchar *short_bcd(
               uchar *Ptd ,
               uchar Ld,
               ushort  *Pts
              ) ;
uchar *int_bcd(
               uchar *Ptd ,
               uchar Ld,
               uint  *Pts
              ) ;

/* M1BIB49.C + M1BIB18.C */
/* Unsigned short -> C string */
uchar *short_str(
               uchar *Ptd ,
               uchar Ld,
               ushort  *Pts
              ) ;
uchar *int_str(
               uchar *Ptd ,
               uchar Ld,
               uint  *Pts
              ) ;

/* M1BIB50.C + M1BIB19.C */
uchar *short_tab (
               uchar *Ptd,
               uchar Ld,
               ushort  *Pts
              ) ;
uchar *int_tab (
               uchar *Ptd,
               uchar Ld,
               uint  *Pts
              ) ;

/* M1BIB20.C */
/* Unsigned char -> C string */
uchar *char_str(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts
              ) ;

/* M1BIB21.C */
/* Unsigned char -> BCD values chain */
uchar *char_bcd(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts
              ) ;

/* M1BIB22.C */
/* Table contening an Unsigned Long -> Unsigned Long
   or after "casting" ushort or uchar */
ulong tab_long(
               uchar *Pts,
               uchar Ls
              );

/* M1BIB23.C */
/* TLV message setting */
bool extract_TLV (
                   uchar **Pt_buf,      /* Area to treat */
                   ushort  *Pt_maxlen,    /* Number of bytes */
                   ushort  *Pt_len,       /* L */
                   uchar *Pt_id         /* T */
                 );

/* M1BIB24.C */
/* Ascii numeric table verification */
bool is_asc_digit (
                    uchar *Pts,
                    uchar Ls
                  );


/* M1BIB25.C */
/* C numeric string verification */
bool is_str_digit ( uchar *Pts );

/* M1BIB26.C */
/* Lrc computing on a length */
uchar lrc_l( uchar *Pts, ushort Ls, uchar Init_val );

/* M1BIB27.C */
/* Difference between two dates */
uchar sub_date (
                uchar *Ptr,  /* result date C pointer in BCD */
                uchar *Pt1,  /* date A pointer in BCD */
                uchar *Pt2,  /* date B pointer in BCD */
                uchar B_ov   /* overflow byte for time difference */
               );

/* Difference between two times */
uchar sub_time (
                 uchar *Ptr,  /* result time C pointer in BCD */
                 uchar *Pt1,  /* time A pointer in BCD */
                 uchar *Pt2   /* time B pointer in BCD */
                );

/* Add days to a date */
uchar add_date_day (
                     uchar *Pt1,  /* date pointer in bcd and result */
                     uchar B_ov   /* Number of days < 63 in bcd */
                    );

/* Add a time in BCD and a waiting delay in ascii */
uchar add_time_delay (
                 uchar *Ptr,  /* time pointer (in BCD) and result */
                 uchar *Pt2   /* pointer to the waiting delay in ascii */
                );

/* M1BIB28.C */
/* Add two bytes in bcd */
uchar addbcd( uchar A, uchar B );

/* Substract two bytes in bcd */
uchar subbcd( uchar A, uchar B );

/* M1BIB29.C */
/* Luhn key verification on length + Iso 2 + '?' + \000 */
uchar luhn_key_ok( uchar *Pt); /* Remark: = TRUE if correct, luhn key value else */

/* M1BIB30.C */
/* Format an ascii time in xxHxx */
void format_time( uchar *Ptd, uchar *Pts);

/* M1BIB31.C */
/* Format an ascii date in xx/xx/xx */
void format_date( uchar *Ptd, uchar *Pts);

/* M1BIB32.C */
/* Compute the modulo 7 key */
uchar mod7_key( uchar *Pts );

/* M1BIB33.C */
/* Verify if a data area is null */
bool memnull( uchar *Pt, uchar Len);

/* M1BIB34.C */
/* Length Type Value message */
bool extract_LTV (
                   uchar **Pt_buf,      /* Area to treat */
                   ushort  *Pt_maxlen,    /* Number of bytes */
                   ushort  *Pt_len,       /* L */
                   ushort  *Pt_id         /* T */
                  );
/* M1BIB51.C + M1BIB35.C */
/* Unsigned short -> Ascii values chain */
uchar *short_asc(
               uchar *Ptd ,
               uchar Ld,
               ushort  *Pts
              ) ;
uchar *int_asc(
               uchar *Ptd ,
               uchar Ld,
               uint  *Pts
              ) ;

/* M1BIB36.C */
/* Unsigned char -> Ascii values chain */
uchar *char_asc(
               uchar *Ptd ,
               uchar Ld,
               uchar *Pts
              ) ;

/* M1BIB37.C */
/* Increase a short and checks if it becomes null */
void inc_number ( ushort *Pt_number );

/* M1BIB38.C */
/* Format an amount */
void format_amount(uchar *Ptd, ulong * Pt_amount, uchar Nr_digits);
/* Remark : Nb_digit is coded ! */

/* M1BIB39.C */
/* Time coherency checking */
bool check_time( uchar *time);

/* M1BIB40.C */
/* Date in format DDMMYY coherency checking : not checked on YY */
bool check_date( uchar *date );

/* M1BIB41.C */
/* Checks the Luhn key on a card number in ascii */
uchar luhn_key( uchar    *card_nr);

/* M1BIB42.C */
/* Add 2 times in ASCII */
void add_time(uchar *asc_time1, uchar *asc_time2);

/* M1BIB43.C */
/* Lrc of an area between two pointers */
uchar lrc(uchar *pointer1, uchar *pointer2, uchar init_value);

/* M1BIB44.C */
/* Sum 2 chain in Ascii */
void add_ascii(uchar *string1, uchar *string2, uchar length);

/* M1BIB45.C */
/* Differences between 2 chain in Ascii */
void sub_ascii(uchar *string1, uchar *string2, uchar length);

/* M1BIB46.C */
/* Substract or add a month to the date in BCD format (YYMM) which is pointed  */
void date_add_or_sub_1_month( uchar Add, uchar *Pt_date );

/* M1BIB47.C */
/* Compute the checksum with a starting value on an area */
ushort checksum( uchar *Pt, uchar *Ptf, ushort Val );
#define CHKINIT   0x5555       /* Starting value type dissymetric */

#endif

