#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include "iconv.h"
#include "ostools.h"

int _g_com2_openned=0;
int _g_enable_lib_debug=1;

/** 广义二进制编码所对应的ASCII码 */
unsigned char Tb_ebcdic [128] =
{
    0x00,0x01,0x02,0x03,0x37,0x2D,0x2E,0x2F,0x16,0x05,0x25,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x3C,0x3D,0x32,0x26,0x18,0x19,0x3F,0x27,0x22,0x1D,0x1E,0x1F,
    0x40,0x5A,0x7F,0x7B,0x5B,0x6C,0x50,0x7D,0x4D,0x5D,0x5C,0x4E,0x6B,0x60,0x4B,0x61,
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0x7A,0x5E,0x4C,0x7E,0x6E,0x6F,
    0x7C,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,
    0xD7,0xD8,0xD9,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0x4A,0xE0,0x4F,0x5F,0x6D,
    0x7D,0x81,0x82,0x83,0X84,0x85,0x86,0x87,0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96,
    0x97,0x98,0x99,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xC0,0x6A,0xD0,0xA1,0x07
};

/** 定义每个月份所对应的天数 */
unsigned char B_fin_mois [] = { 0x31,0x31,0x28,0x31,0x30,0x31,
                        0x30,0x31,0x31,0x30,0x31,0x30,0x31
                      };


unsigned char *asc_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts, unsigned char Lgs)
{
    unsigned char I;
    unsigned char *Ptmp = Ptd;
    if ( Lgd > Lgs/2)
    {
        (void) memset( Ptd, 0x00, Lgd ) ;
        Ptd = Ptd + Lgd - ((Lgs + 1) / 2) ;
    }
    for ( I = 0 ; I < ((Lgs+1) / 2) ; I++)
    {
        if ( (!(Lgs % 2) && !I) || I ) *Ptd =  (*Pts++ << 4) & 0xF0 ;
        *Ptd = *Ptd + (*Pts++ & 0x0F)  ;
        Ptd++ ;
    }
    return Ptmp;
}

unsigned long asc_long(unsigned char *Pts, unsigned char Ls)
{
    unsigned char I;
    unsigned long Lg1,
    Lg2;

    Lg1 = 0 ;
    Lg2 = 1 ;
    Pts += Ls ;
    for (I = 0; I< Ls ; I++)
    {
        Lg1 += (Lg2 * (*--Pts & 0x0F) ) ;
        Lg2 *= 10 ;
    }

    return (Lg1) ;
}

static unsigned char Conv(unsigned char Oct)
{
    if (Oct >= 0x41)
        return ( (Oct - 7) & 0x0F );
    else
        return ( Oct & 0x0F );
}

unsigned char *asc_hex(unsigned char *Ptd, unsigned char Ld, unsigned char *Pts, unsigned char Ls)
{
    unsigned char I;
    unsigned char *Ptmp = Ptd;
    (void) memset( Ptd, 0x00, Ld ) ;

    Ptd = Ptd + Ld - ((Ls + 1) / 2) ;
    if ( Ls % 2 ) *Ptd++ = Conv(*Pts++) & 0x0F ;
    for ( I = 0 ; I < (Ls / 2) ; I++)
    {
        *Ptd =  (Conv(*Pts++) << 4) & 0xF0 ;
        *Ptd = *Ptd + (Conv(*Pts++) & 0x0F)  ;
        Ptd ++;
    }
    return Ptmp;
}

unsigned char *bcd_asc(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg)
{
    unsigned char I;
    unsigned char *Ptmp = Ptd;

    if ( Lg % 2 ) *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    for ( I = 0 ; I < (Lg / 2) ; I++)
    {
        *Ptd++ = ( (*Pts & 0xF0) >> 4 ) + 0x30 ;
        *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    }
    return Ptmp;
}

unsigned long bcd_long(unsigned char *Pts, unsigned char Ls)
{
    unsigned char I,
    Oct;
    unsigned long Lg1,
    Lg2;

    Lg1 = 0 ;
    Lg2 = 1 ;
    Pts += (Ls+1)/2;
    for (I = 0; I< Ls ; I++)
    {
        if ( I % 2) Oct = (*Pts >> 4 ) & 0x0F;
        else Oct = *--Pts & 0x0F;
        Lg1 += Lg2 * Oct ;
        if (Lg2 == 1000000000L ) Lg2 = 0 ;
        else Lg2 *= 10 ;
    }

    return (Lg1) ;
}

unsigned char *bcd_str(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg)
{
    *(Ptd +Lg) = 0x00;
    return( bcd_asc( Ptd, Pts, Lg ) );
}

unsigned char *char_bcd(unsigned char *Ptd ,unsigned char Lgd, unsigned char *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_bcd( Ptd, Lgd, &Lg2) );
}

unsigned char *char_asc(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_asc( Ptd, Lgd, &Lg2) );
}

unsigned char *char_str(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_str( Ptd, Lgd, &Lg2) );
}

unsigned short checksum(unsigned char *Pt, unsigned char *Ptf, unsigned short Val)
{
    while ( Pt < Ptf )
    {
        Val += *Pt++;
    }
    return ( Val );
}

unsigned char *hex_asc(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg)
{
    unsigned char I,
    *Pt0,
    *Ptmp = Ptd;

    Pt0 = Ptd ;
    if ( Lg % 2 ) *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    for ( I = 0 ; I < (Lg / 2) ; I++)
    {
        *Ptd++ = ( (*Pts & 0xF0) >> 4 ) + 0x30 ;
        *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    }
    while (Pt0 != Ptd)
    {
        if (*Pt0 >= 0x3A) *Pt0 += 7 ;
        Pt0++;
    }
    return Ptmp;
}

unsigned char *hex_str(unsigned char *Ptd, unsigned char *Pts, unsigned char Lg)
{
    *(Ptd +Lg) = 0x00;
    return( hex_asc( Ptd, Pts, Lg ) );
}

unsigned char *int_asc(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_asc( Ptd, Lgd, &Lg2) );
}

unsigned char *int_bcd(unsigned char *Ptd ,unsigned char Lgd,unsigned int *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_bcd( Ptd, Lgd, &Lg2) );
}

unsigned char *int_str(unsigned char *Ptd ,unsigned char Lgd,unsigned int *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_str( Ptd, Lgd, &Lg2) );
}

unsigned char *long_asc( unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts )
{
    unsigned char I,
    Oct,
    *Pt0,
    Tb[10],
    *Ptmp = Ptd;
    unsigned long Lg1,
    Lg2;

    Lg1 = *Pts;
    Lg2 = 100000000L ;
    for (I = 0; I< 5; I++)
    {
        Oct = (unsigned char)(Lg1 / Lg2) ;
        Tb[2*I] = Oct / 10 + 0x30 ;
        Tb[2*I+1] = Oct % 10 + 0x30;
        Lg1 = Lg1 % Lg2;
        Lg2 = Lg2 / 100;
    }

    (void) memset( Ptd, 0x30, Lgd ) ;
    Ptd += Lgd ;
    Pt0 = Ptd ;
    if ( Lgd > 10 ) Lgd = 10 ;
    for ( I=0; I < Lgd;I++) *--Ptd = Tb[9-I] ;

    return Ptmp;
}

unsigned char *long_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts)
{
    unsigned int I;
    unsigned char *Pt0,Tb[5], *Ptmp = Ptd;
    unsigned long Lg1,Lg2;

    Lg1 = *Pts;
    Lg2 = 100000000L ;
    for (I = 0; I< 5; I++)
    {
        Tb[I] = (unsigned char)(Lg1 / Lg2) ;
        Tb[I] = ((Tb[I] / 10 ) << 4 ) + (Tb[I] % 10);
        Lg1 = Lg1 % Lg2;
        Lg2 = Lg2 / 100;
    }

    (void) memset( Ptd, 0x00, Lgd ) ;
    Ptd += Lgd ;
    Pt0 = Ptd ;
    if ( Lgd > 5) Lgd = 5 ;
    for ( I=0; I < Lgd;I++) *--Ptd = Tb[4-I] ;

    return Ptmp;
}

unsigned char *long_str(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts)
{
    *(Ptd +Lgd) = 0x00;
    return( long_asc( Ptd, Lgd, Pts ) );
}

unsigned char *short_asc(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_asc( Ptd, Lgd, &Lg2) );
}

unsigned char *short_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_bcd( Ptd, Lgd, &Lg2) );
}

unsigned char *short_str(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_str( Ptd, Lgd, &Lg2) );
}

unsigned char *str_bcd(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
{
    return( (unsigned char*) asc_bcd( Ptd, Lgd, Pts, strlen((char*) Pts) ));
}

unsigned char *str_hex(unsigned char *Ptd, unsigned char Lgd, unsigned char *Pts)
{
    return ( (unsigned char*) asc_hex( Ptd, Lgd , Pts, strlen((char*) Pts) )) ;
}

unsigned long str_long( unsigned char *Pts )
{
    return ( asc_long( Pts , strlen( (char*)Pts) ));
}

void format_date( unsigned char *Ptd, unsigned char *Pts )
{
    unsigned char i ;

    for ( i = 0 ; i < 7; i++)
    {
        *Ptd++ = *Pts++;
        if ( ( i == 1) || ( i == 3) ) *Ptd++ = '/';
    }
}

unsigned char ebcdic_asc (unsigned char *Pts ,unsigned char Lg)
{
    unsigned char I,J;

    for ( I = 0 ; I < Lg ; I++)
    {
        for ( J = 0; (J < 128) && (*Pts != Tb_ebcdic[J]) ; J++) ;
        if (J == 128) return 0;
        else *Pts++ = J ;
    }

    return 1;
}

unsigned char *asc_ebcdic(unsigned char *Pts ,unsigned char Lg)
{
    unsigned char I;
		unsigned char *Ptmp = Pts;
		
    for ( I = 0 ; I < Lg ; I++)
    {
        *Pts = Tb_ebcdic[*Pts] ;
        Pts ++;
    }

    return Ptmp;
}

unsigned char *str_ebcdic(unsigned char *Pts)
{
    return( (unsigned char*) asc_ebcdic( Pts, strlen((char*) Pts) ));
}

unsigned char *long_tab(unsigned char *Ptd ,unsigned char Lgd,unsigned long *Pts)
{
    unsigned char I;
    unsigned char *Ptmp = Ptd;

    (void) memset( Ptd, 0x00, Lgd ) ;
    if (Lgd > 4)
    {
        Ptd += ( Lgd-4) ;
        Lgd = 4;
    }
    for (I = (4-Lgd); I < 4 ; I++)
    {
        *Ptd++ = (unsigned char) (*Pts >> (24 - 8*I) );
    }
    return Ptmp;
}

unsigned long tab_long(unsigned char *Pts,unsigned char Ls)
{
    unsigned char I;
    unsigned long Lg1,Lg2;

    Lg1 = 0 ;
    Lg2 = 1 ;

    Pts += Ls ;

    for (I = 0; I< Ls ; I++)
    {
        Lg1 += Lg2 * (*--Pts);
        if (Lg2 == 0x01000000L ) Lg2 = 0 ;
        else Lg2 *= 256 ;
    }
    return (Lg1) ;
}

unsigned char *short_tab(unsigned char *Ptd, unsigned char Lgd, unsigned short *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_tab( Ptd, Lgd, &Lg2) );
}

unsigned char *int_tab(unsigned char *Ptd, unsigned char Lgd, unsigned int *Pts)
{
    unsigned long Lg2;

    Lg2 = *Pts;
    return((unsigned char*) long_tab( Ptd, Lgd, &Lg2) );
}

unsigned char is_asc_digit ( unsigned char *Pts, unsigned char Lgs )
{
    unsigned char I ;

    for (I = 0 ; I < Lgs ; I++) if ( ! isdigit( Pts[I]) ) return 0;
    return 1;
}

unsigned char is_str_digit ( unsigned char *Pts )
{
    unsigned char I ;

    for (I = 0 ; I < strlen( (char*) Pts) ; I++)
        if ( ! isdigit( Pts[I]) ) return 0;
    return 1;
}

unsigned char addbcd( unsigned char Oc1, unsigned char Oc2)
{
    unsigned char Val;

    if ( ( Val = (Oc1 & 0x0F) + (Oc2 & 0x0F) ) >= 0x0A ) Val += 0x06;
    if ( ( Val += (Oc1 & 0xF0) + (Oc2 & 0xF0) ) >= 0xA0 ) Val += 0x60;
    return(Val);
}

unsigned char subbcd ( unsigned char Oc1, unsigned char Oc2 )
{
    return( addbcd( Oc1 , addbcd( 0x99 - Oc2, 1 ) ) );
}

unsigned char sub_heure (unsigned char *Ptr, unsigned char *Pt1, unsigned char *Pt2 )
{
    unsigned char I ,
    B_x,
    Val,
    B_ov;

    Pt2 ++;
    Pt1 ++;
    Ptr ++;
    for ( I = 0 , B_ov = 0; I < 2 ; I++ )
    {
        Val = *Pt1-- ;

        *Ptr = addbcd( *Pt2--, B_ov) ;

        B_ov  = 0x00 ;

        if  ( Val < *Ptr )
        {
            ( I == 0 ) ? ( B_x = 0x60 ) : ( B_x = 0x24) ;

            Val = addbcd( Val, B_x ) ;

            B_ov = 0x01 ;
        }

        *Ptr = subbcd ( Val, *Ptr );
        Ptr --;
    }

    return (B_ov) ;
}

unsigned char add_date_day( unsigned char *Ptr, unsigned char B_ov)
{
    unsigned char B_x,
    Bis,
    I,annee,tmp ;


    annee=((*Ptr & 0x0f)+(*Ptr >>4 )*10 ) ;
    Bis=((!(annee%4)) && (annee))?(1) :(0);

    Ptr ++;
    tmp=(*Ptr>>4)*10 + (*Ptr & 0x0F);
    if (tmp!=2)
        Bis=0;

    B_x = B_fin_mois[tmp] + Bis;

    Ptr+=2;
    for ( I = 0 ; I< 3 ; I++ )
    {
        Ptr --;
        *Ptr = addbcd(*Ptr, B_ov) ;

        B_ov  = 0x00 ;

        switch (I)
        {
        case 1 :
            B_x = 0x12 ;
            break ;

        case 2 :
            B_x = 0x99;

        default :
            break ;
        }

        if (*Ptr > B_x)
        {
            *Ptr = subbcd( *Ptr, B_x);
            B_ov = 1 ;
        }

    }
    return (B_ov) ;
}

unsigned char add_heure_attente (unsigned char *Ptr, unsigned char *Pt2 )
{
    unsigned char I ,
    Val,
    B_x,
    B_ov,
    Attente[2];

    str_bcd( &Attente[0], 2, Pt2 );

    Pt2 = &Attente[2];
    Ptr += 2;
    for ( I = 0 , B_ov = 0; I < 2 ; I++ )
    {
        ( I == 0 ) ? ( B_x = 0x60 ) : ( B_x = 0x24) ;

        Val = addbcd( *--Ptr, B_ov ) ;

        B_ov  = 0x00 ;

        if (( *Ptr = (Val & 0x0F) + (*--Pt2 & 0x0F) ) >= 0x0A )
            *Ptr += 0x06;
        if ( ( *Ptr += (Val & 0xF0) + (*Pt2 & 0xF0) ) >= B_x )
        {
            *Ptr = subbcd( *Ptr, B_x);
            B_ov = 1 ;
        }
    }

    return (B_ov) ;
}

unsigned char sub_date (unsigned char *Ptr, unsigned char *Pt1, unsigned char *Pt2, unsigned char B_ov)
{
    unsigned char B_x,
    Val,
    Bis,
    I,annee,tmp ;

    annee=((*Pt1 & 0x0f)+(*Pt1 >>4 )*10 ) ;
    Bis=((!(annee%4)) && (annee))?(1) :(0);

    Pt2 += 2;
    Pt1 += 2;
    Ptr += 2;
    for ( I = 0 ; I< 3 ; I++ )
    {
        Val = *Pt1-- ;

        *Ptr = addbcd( *Pt2--, B_ov) ;

        B_ov  = 0x00 ;

        if  ( Val < *Ptr )
        {
            switch (I)
            {
            case 0 :
            {
                tmp=(*Pt1>>4)*10 + (*Pt1 & 0x0F) -1;
                if (tmp!=2)
                    Bis=0;
                B_x = B_fin_mois[ tmp ] + Bis;
                break ;
            }

            case 1 :
                B_x = 0x12 ;
                break ;

            case 2 :
                B_x = 0x00 ;
                break ;
            }

            Val = addbcd ( Val, B_x );

            B_ov = 0x01 ;
            if (I==2)
            {
                if (  (Val <50 )
                        &&(*Ptr>= 50)
                   )
                    B_ov=0;
            }
        }

        *Ptr = subbcd ( Val, *Ptr );
        Ptr --;
    }

    return (B_ov) ;
}


unsigned char memnul( unsigned char *Pt, unsigned int Lg )
{
    unsigned int I;

    for ( I = 0; ((!Pt[I]) && (I < Lg)); I++ );
    if ( I == Lg ) return 1;
    else		   return 0;
}

unsigned char lrc_l( unsigned char *Pts, unsigned short Lgs, unsigned char Val_init )
{
    unsigned char  Lrc1 ;

    Lrc1 = Val_init ;
    while ( Lgs != 0) Lrc1 ^= *(Pts + (--Lgs));

    return (Lrc1);
}

unsigned char  lrc( unsigned char *pointchar1, unsigned char *pointchar2, unsigned char valeur)
{
    unsigned char Lrc1;

    Lrc1 = valeur;
    while ( pointchar1 < pointchar2) Lrc1 ^= *pointchar1++;

    return (Lrc1);
}

void move_inc_ptr ( unsigned char **Ptd, unsigned char *Pts , unsigned char Lg )
{
    (void) memmove( (char *) *Ptd, (char *)Pts, Lg);

    *Ptd += Lg;
}

unsigned char *move_255 ( unsigned char *Ptd, unsigned char *Pts , unsigned char Lg )
{
    (void) memmove( (char *)Ptd, (char *)Pts, Lg);

    return Ptd;
}

unsigned char *mov_to_str(unsigned char *Ptd, unsigned char *Pts )
{
	  unsigned char *Ptmp = Ptd;
    (void) memmove( (char *)Ptd, (char *)(Pts+1), *Pts);
    Ptd += *Pts;
    *Ptd++ = 0x00;
    return Ptmp;
}

void Uart_Printf(char *fmt,...)
{
    va_list ap;
    char string[512];

	if (strlen(fmt)>400)
		return;
    if (!_g_com2_openned && _g_enable_lib_debug ){
	    va_start(ap,fmt);
	    vsprintf(string,fmt,ap);
	    fprintf(stderr,string);
	    va_end(ap);
	}
}


int gb2312_utf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{
    iconv_t cd;
    if( (cd = iconv_open("utf-8","GB18030")) ==0 )
        return -1;
    memset(destbuf,0,destlen);
    char **source = (char**)&sourcebuf;
    char **dest   = (char**)&destbuf;

    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
        return -1;
    iconv_close(cd);
    
    return 0;

}

int utf8_gb2312(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{
    iconv_t cd;
    if( (cd = iconv_open("GB18030","utf-8")) ==0 )
        return -1;
    memset(destbuf,0,destlen);
    char **source = (char**)&sourcebuf;
    char **dest   = (char**)&destbuf;

    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
        return -1;
    iconv_close(cd);
    return 0;

}

static long DATE_LeapYearInt(int iYear)
{
	if((iYear%400 == 0) || ((iYear%4 == 0) && (iYear%100)))
	  	return(DATE_SUCCESS);
	else
		return(DATE_NOTLEAPYEAR_ERR);
}

static long DATE_MonthDaysInt(int iYear,int iMonth, int *piDays )
{
	if( !piDays )
		return(DATE_PARAM_ERR);

	switch(iMonth)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		*piDays = 31;
		return(DATE_SUCCESS);
	case 2:
		if( DATE_LeapYearInt(iYear) == DATE_SUCCESS)
			*piDays = 29;
		else
			*piDays = 28;
		return(DATE_SUCCESS);
	case 4:
	case 6:
	case 9:
	case 11:
		*piDays = 30;
		return(DATE_SUCCESS);
	default:
		return(DATE_INVALIDMONTH_ERR);
	}
}

long DATE_ValideDateStr(char *pcDate)
{
	int iI;
	int iYear;
	int iMonth;
	int iDay;
	long lResult;
	int iMonthDays;

	if( !pcDate )
		return(DATE_PARAM_ERR);

	for( iI = 0;iI<DATE_DATESTRLEN;iI++)
	{
		if((*(pcDate+iI) < '0') || (*(pcDate+iI) > '9'))
		{
			return(DATE_INVALIDCHAR_ERR);
		}
	}
	iYear = (int)asc_long((unsigned char *)pcDate,4);
	iMonth = (int)asc_long((unsigned char *)pcDate+4,2);
	iDay = (int)asc_long((unsigned char *)pcDate+6,2);

	if((iYear<1900) || (iYear>2500))
	{
		return(DATE_INVALIDYEAR_ERR);
	}

	lResult = DATE_MonthDaysInt(iYear,iMonth,&iMonthDays);
	if( lResult != DATE_SUCCESS)
		return(lResult);
	if(  (iDay < 1) || (iDay > iMonthDays))
		return(DATE_INVALIDDAY_ERR);
	return(DATE_SUCCESS);
}

long DATE_ValideTimeStr(char *pcTime)
{
	int iI;
	int iHour;
	int iMin;
	int iSec;

	if( !pcTime )
		return(DATE_PARAM_ERR);

	for( iI = 0;iI<DATE_TIMESTRLEN;iI++)
	{
		if((*(pcTime+iI) < '0') || (*(pcTime+iI) > '9'))
		{
			return(DATE_INVALIDCHAR_ERR);
		}
	}
	iHour = (int)asc_long((unsigned char *)pcTime,2);
	iMin = (int)asc_long((unsigned char *)pcTime+2,2);
	iSec = (int)asc_long((unsigned char *)pcTime+4,2);

	if((iHour<0) || (iHour>24))
	{
		return(DATE_INVALIDHOUR_ERR);
	}

	if((iMin<0) || (iMin>60))
	{
		return(DATE_INVALIDMIN_ERR);
	}

	if((iSec<0) || (iSec>60))
	{
		return(DATE_INVALIDSEC_ERR);
	}
	
	return(DATE_SUCCESS);
}
