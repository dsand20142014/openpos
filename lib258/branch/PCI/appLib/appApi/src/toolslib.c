#include <string.h>
#include <ctype.h>

#include "toolslib.h"




/*
UCHAR B_fin_mois [] = { 0x31,0x31,0x28,0x31,0x30,0x31,
                        0x30,0x31,0x31,0x30,0x31,0x30,0x31
                      } ;

UCHAR Tb_ebcdic [128] =
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
*/
/* table 1 : permutation initiale */
UCHAR T1[] =
{
    57,49,41,33,25,17, 9, 1,
    59,51,43,35,27,19,11, 3,
    61,53,45,37,29,21,13, 5,
    63,55,47,39,31,23,15, 7,
    56,48,40,32,24,16, 8, 0,
    58,50,42,34,26,18,10, 2,
    60,52,44,36,28,20,12, 4,
    62,54,46,38,30,22,14, 6
};

/* table 2 : permutation finale */
UCHAR T2[] =
{
    39, 7,47,15,55,23,63,31,
    38, 6,46,14,54,22,62,30,
    37, 5,45,13,53,21,61,29,
    36, 4,44,12,52,20,60,28,
    35, 3,43,11,51,19,59,27,
    34, 2,42,10,50,18,58,26,
    33, 1,41, 9,49,17,57,25,
    32, 0,40, 8,48,16,56,24
};

/* table 3 : fonction d'expansion E  ( valeur - 1 ) */
UCHAR T3[] =
{
    31, 0, 1, 2, 3, 4,
    3, 4, 5, 6, 7, 8,
    7, 8, 9,10,11,12,
    11,12,13,14,15,16,
    15,16,17,18,19,20,
    19,20,21,22,23,24,
    23,24,25,26,27,28,
    27,28,29,30,31, 0
};

/* table 5 : fonction de permutation P */
UCHAR T5[] =
{
    15, 6,19,20,
    28,11,27,16,
    0,14,22,25,
    4,17,30, 9,
    1, 7,23,13,
    31,26, 2, 8,
    18,12,29, 5,
    21,10, 3,24
};

/* table 7 : table de choix 1 */
UCHAR T7_1_2[56] =
{
    56,48,40,32,24,16, 8,
    0,57,49,41,33,25,17,
    9, 1,58,50,42,34,26,
    18,10, 2,59,51,43,35,

    62,54,46,38,30,22,14,
    6,61,53,45,37,29,21,
    13, 5,60,52,44,36,28,
    20,12, 4,27,19,11, 3
};

/* table 8 : table de d?calage */
//	UCHAR T8[16] =
UCHAR T8[] =
{
    0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0
};

/* table 9 : table de choix 2 */
UCHAR T9[] =
{
    13,16,10,23, 0, 4,
    2,27,14, 5,20, 9,
    22,18,11, 3,25, 7,
    15, 6,26,19,12, 1,
    40,51,30,36,46,54,
    29,39,50,44,32,47,
    43,48,38,55,33,52,
    45,41,49,35,28,31
};

/* table 6 : s?lection de fonctions S1 ? S8 */
//	UCHAR T6[8][64] =
UCHAR T6[][64] =
{
    /* S1 */
    {
        14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
        0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
        4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
        15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13
    },

    /* S2 */
    {
        15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
        3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
        0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
        13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9
    },

    /* S3 */
    {
        10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
        13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
        13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
        1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12
    },

    /* S4 */
    {
        7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
        13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
        10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
        3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14
    },

    /* S5 */
    {
        2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
        14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
        4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
        11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3
    },

    /* S6 */
    {
        12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
        10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
        9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
        4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13
    },

    /* S7 */
    {
        4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
        13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
        1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
        6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12
    },

    /* S8 */
    {
        13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
        1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
        7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
        2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11
    }
};

/* Table pour eclatement des valeurs precedentes */
//UCHAR TE[16][4] =
UCHAR TE[][4] =
{
    {0,0,0,0},
    {0,0,0,1},
    {0,0,1,0},
    {0,0,1,1},
    {0,1,0,0},
    {0,1,0,1},
    {0,1,1,0},
    {0,1,1,1},
    {1,0,0,0},
    {1,0,0,1},
    {1,0,1,0},
    {1,0,1,1},
    {1,1,0,0},
    {1,1,0,1},
    {1,1,1,0},
    {1,1,1,1}
};


UCHAR res_codage[8];
UCHAR res_decodage[8];

static	  UCHAR Key[64];
static	  UCHAR input[64], /* data input */
output[64]; /* data output */
static	  UCHAR Kn[16][48];

/*
UCHAR  lrc( UCHAR *pointchar1, UCHAR *pointchar2, UCHAR valeur)
{
    UCHAR Lrc1;

    Lrc1 = valeur;
    while ( pointchar1 < pointchar2) Lrc1 ^= *pointchar1++;

    return (Lrc1);
}
*/
VOID somascii(UCHAR *chaine1, UCHAR *chaine2, UCHAR length)
{
    UCHAR	   carry, somme;
    UCHAR	   i;

    carry = 0;

    for ( i = length-1; i != 255; i--)
    {
        somme = chaine1[i] + chaine2[i] + carry;
        if ( somme > 0x69)
        {
            carry = 1;
            chaine2[i] = (somme - 0x6A) + '0';
        }
        else
        {
            carry = 0;
            chaine2[i] = somme -  '0';
        }
    }
}

VOID difascii(UCHAR *chaine1, UCHAR *chaine2, UCHAR length)
{
    UCHAR	  carry;
    UCHAR	   i;

    carry = 0;

    for ( i = length-1; i != 255; i--)
    {
        if (chaine2[i] < (chaine1[i] + carry))
        {
            chaine2[i] += 10;
            chaine2[i] =   (chaine2[i] - (chaine1[i]+ carry)) + '0';
            carry = 1;
        }
        else
        {
            chaine2[i] =   (chaine2[i] - (chaine1[i] + carry)) + '0';
            carry = 0;
        }
    }
}

VOID date_plus_ou_moins_1_mois( UCHAR Add, UCHAR *Pt_date )
{
    UCHAR mois, annee;

    mois = (UCHAR) bcd_long( Pt_date+1, 2 );
    annee =(UCHAR) bcd_long( Pt_date  ,2);
    if ( Add )
    {
        mois++;
        if (mois==13)
        {
            mois = 1;
            annee++;
            if (annee==100)
            {
                annee=0;
            }
        }
    }
    else
    {
        mois--;
        if (mois==0)
        {
            mois = 12;
            if (annee==0)
            {
                annee=100;
            }
            annee--;
        }
    }
    char_bcd( Pt_date   ,1,&annee);
    char_bcd( Pt_date+1 ,1,&mois);
}
/*
unsigned short checksum( UCHAR *Pt, UCHAR *Ptf, unsigned short Val )
{
    while ( Pt < Ptf )
    {
        Val += *Pt++;
    }
    return ( Val );
}*/
VOID  inc_nombre( UINT *Pt_nombre )
{
    if (*Pt_nombre==0x999)
        *Pt_nombre=1;
    else
        (*Pt_nombre)++;
}
VOID format_montant(UCHAR *Ptd, ULONG * Pt_montant, UCHAR Nb_digit)
{
    UCHAR i;
    UCHAR Buf[20];
    ULONG var;

    Buf[19]=0;
    var=*Pt_montant;
    for (i=0;i<19;i++)
    {
        Buf[(18-i)]=(UCHAR)(( (var)%10 )+0x30);
        var=var/10;
        if (i==1)
        {
            Buf[16]=',';
            i++;
        }
    }
    memcpy(Ptd, &Buf[(16-Nb_digit)], (Nb_digit+4));

}
BOOL heure_correcte( UCHAR *heure )
{
    if (   ((heure[0] > '2')||(heure[0] < '0'))
            || ( !isdigit( heure[1]) )
            || ((heure[2] > '5')||(heure[2] < '0'))
            || ((heure[3] > '9')||(heure[3] < '0'))
            || (   (heure[0] =='2')
                   &&((heure[1] > '3')||(heure[1] < '0')) )
       )
        return(FALSE);
    return(TRUE);
}
BOOL date_correcte( UCHAR *date )
{
    if (  ((date[0] > '3')||(date[0] < '0'))
            ||( !isdigit(date[1] ) )
            ||((date[2] > '1')||(date[2] < '0'))
            ||( !isdigit(date[3] ) )
            ||(  (date[0] =='3')
                 &&((date[1] > '1')||(date[1] < '0')) )
            ||(  (date[2] =='1')
                 &&((date[3] > '2')||(date[3] < '0')) )
       )
        return(FALSE);
    return(TRUE);
}
UCHAR cal_luhn( UCHAR	 *carte)
{
    UCHAR	 k , fin_fic , luhn, l;
    UCHAR	 i;


    luhn = 0;
    for (fin_fic = 0; (carte[fin_fic] != '=') && (fin_fic < 19); fin_fic++);

    for (i = (fin_fic-2) , k = 1 ; i != 255; i--)
    {
        l = carte[i] & 0x0F;
        luhn += ((l + l*k) + ((l*k*2)/10) ) % 10;
        k = 1- k;
    }
    if ((luhn%10) == 0)  luhn = 0;
    else                 luhn = 10 - (luhn % 10);

    if ((carte[fin_fic - 1] & 0x0F) != luhn)  return(KO);
    else                                      return(OK);
}
VOID somheure(UCHAR *Pt1, UCHAR *Pt2)
{
    UCHAR heure, i;

    typedef struct
    {
        UCHAR borne;
        UCHAR retrait;
    } typ_lim;
    typ_lim Tb_lim[] = { {'9',0x0A}, {'5',0x06}, {'9',0x0A} };

    Pt1+= 4;
    Pt2+= 4;
    for ( i = 3; i != 255; i-- )
    {
        *--Pt2 += (*--Pt1 & 0x0f);

        if ( (i) && ( *Pt2 > Tb_lim[i-1].borne ))
        {
            *Pt2 -= Tb_lim[i-1].retrait;
            (*(Pt2-1))++;
        }
    }

    heure = ( (*Pt2&0x0f) << 4) + (*(Pt2+1)&0x0f);
    if ( heure >= 0x24) heure -= 0x24;

    *Pt2 = (heure / 0x10) + '0';
    *(Pt2+1) = (heure % 0x10) + '0';

}

/*
VOID format_date( UCHAR *Ptd, UCHAR *Pts )
{
    UCHAR i ;

    for ( i = 0 ; i < 7; i++)
    {
        *Ptd++ = *Pts++;
        if ( ( i == 1) || ( i == 3) ) *Ptd++ = '/';
    }
}*/
UCHAR cle_mod7( UCHAR *Pt )
{
    UCHAR Cle;
    UCHAR I, J;

    for ( I = 0, J = 11, Cle = 0; I < 9 ; I++ , J--)
    {
        if ( J == 7) J--;
        Cle = ( Cle + ( *Pt++ & 0x0F) * J ) % 7;
    }
    return( Cle + 0x30 );
}
/*
BOOL memnul( UCHAR *Pt, UINT Lg )
{
    UINT I;

    for ( I = 0; ((!Pt[I]) && (I < Lg)); I++ );
    if ( I == Lg ) return(TRUE);
    else		   return(FALSE);
}*/
BOOL decoupe_ART ( UCHAR **Pt_buf, UINT *Pt_lgmax, UINT *Pt_lg, UINT *Pt_id )
{
    if ( *Pt_lgmax < 6 ) return(FALSE);

    *Pt_lg = (UINT) tab_long( *Pt_buf, 2) - 2;
    *Pt_buf = *Pt_buf+2 ;

    *Pt_id = (UINT) tab_long( *Pt_buf, 2);
    *Pt_buf = *Pt_buf+2 ;

    if (*Pt_lg > (*Pt_lgmax -= 4) )  return ( FALSE );
    else
    {
        *Pt_lgmax -= *Pt_lg;
        return ( TRUE );
    }
}

/*
UCHAR *int_asc(UCHAR *Ptd ,UCHAR Lgd,UINT *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_asc( Ptd, Lgd, &Lg2) );
}
UCHAR *short_asc(UCHAR *Ptd ,UCHAR Lgd,unsigned short *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_asc( Ptd, Lgd, &Lg2) );
}
UCHAR *char_asc( UCHAR *Ptd , UCHAR Lgd, UCHAR *Pts )
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_asc( Ptd, Lgd, &Lg2) );
}
BOOL is_str_digit ( UCHAR *Pts )
{
    UCHAR I ;

    for (I = 0 ; I < strlen( (char*) Pts) ; I++)
        if ( ! isdigit( Pts[I]) ) return (FALSE) ;
    return( TRUE );
}
UCHAR lrc_l( UCHAR *Pts, unsigned short Lgs, UCHAR Val_init )
{
    UCHAR  Lrc1 ;

    Lrc1 = Val_init ;
    while ( Lgs != 0) Lrc1 ^= *(Pts + (--Lgs));

    return (Lrc1);
}
UCHAR sub_heure (UCHAR *Ptr, UCHAR *Pt1, UCHAR *Pt2 )
{
    UCHAR I ,
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
UCHAR sub_date (UCHAR *Ptr, UCHAR *Pt1, UCHAR *Pt2, UCHAR B_ov)
{
    UCHAR B_x,
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
UCHAR add_heure_attente (UCHAR *Ptr, UCHAR *Pt2 )
{
    UCHAR I ,
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
UCHAR add_date_day( UCHAR *Ptr, UCHAR B_ov)
{
    UCHAR B_x,
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
UCHAR addbcd( UCHAR Oc1, UCHAR Oc2)
{
    UCHAR Val;

    if ( ( Val = (Oc1 & 0x0F) + (Oc2 & 0x0F) ) >= 0x0A ) Val += 0x06;
    if ( ( Val += (Oc1 & 0xF0) + (Oc2 & 0xF0) ) >= 0xA0 ) Val += 0x60;
    return(Val);
}
UCHAR subbcd ( UCHAR Oc1, UCHAR Oc2 )
{
    return( addbcd( Oc1 , addbcd( 0x99 - Oc2, 1 ) ) );
}
UCHAR cle_luhn_ok( UCHAR *Pt )
{
    UCHAR  luhn,
    l,
    nb,
    i,
    k;

    nb = *Pt ;
    for ( luhn = 0, i = nb - 1, k = 1; i != 0; i-- )
    {
        l = Pt[i] & 0x0F;
        luhn += ((l + l*k) + ((l*k*2)/10) ) % 10;
        k = 1- k;
    }
    luhn = (10 - (luhn % 10)) % 10;

    if ( (Pt[nb] &0x0F) != luhn ) return( luhn );
    else                  return( TRUE );
}
VOID format_heure( UCHAR *Ptd, UCHAR *Pts )
{
    UCHAR i ;

    for ( i = 0 ; i < 5 ; i++)
    {
        *Ptd++ = *Pts++;
        if ( i == 1) *Ptd++ = 'H';
    }
}
UCHAR *short_tab(
    UCHAR *Ptd ,
    UCHAR Lgd,
    unsigned short *Pts
)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_tab( Ptd, Lgd, &Lg2) );
}
UCHAR *int_tab(
    UCHAR *Ptd ,
    UCHAR Lgd,
    UINT *Pts
)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_tab( Ptd, Lgd, &Lg2) );
}
UCHAR *char_str( UCHAR *Ptd , UCHAR Lgd, UCHAR *Pts )
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_str( Ptd, Lgd, &Lg2) );
}
UCHAR *char_bcd(UCHAR *Ptd ,UCHAR Lgd,UCHAR *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_bcd( Ptd, Lgd, &Lg2) );
}
ULONG tab_long(UCHAR *Pts,UCHAR Ls)
{
    UCHAR I;
    ULONG Lg1,Lg2;

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
BOOL decoupe_TLV (UCHAR **Pt_buf,UINT  *Pt_lgmax,UINT  *Pt_lg,UCHAR *Pt_id)
{
    *Pt_id =  **Pt_buf;
    if ( *Pt_lgmax == 1 ) return(FALSE);

    if ( *(*Pt_buf + 1) == 0xff )
    {
        if ( *Pt_lgmax < 4 ) return( FALSE );
        *Pt_lg =  (*(*Pt_buf + 2)) * 256  + (*(*Pt_buf + 3)) ;
        *Pt_lgmax -= 4;
        *Pt_buf += 4;
    }
    else
    {
        *Pt_lg = *(*Pt_buf + 1) ;
        *Pt_lgmax -= 2;
        *Pt_buf += 2;
    }

    if (*Pt_lg > *Pt_lgmax)  return ( FALSE );
    else
    {
        *Pt_lgmax -= *Pt_lg;
        return ( TRUE );
    }
}
BOOL is_asc_digit ( UCHAR *Pts, UCHAR Lgs )
{
    UCHAR I ;

    for (I = 0 ; I < Lgs ; I++) if ( ! isdigit( Pts[I]) ) return (FALSE) ;
    return( TRUE );
}
ULONG str_long( UCHAR *Pts )
{
    return ( asc_long( Pts , strlen( (char*)Pts) ));
}
UCHAR *long_bcd(UCHAR *Ptd ,UCHAR Lgd,ULONG *Pts)
{
    UINT I;
    UCHAR *Pt0,Tb[5];
    ULONG Lg1,Lg2;

    Lg1 = *Pts;
    Lg2 = 100000000L ;
    for (I = 0; I< 5; I++)
    {
        Tb[I] = (UCHAR)(Lg1 / Lg2) ;
        Tb[I] = ((Tb[I] / 10 ) << 4 ) + (Tb[I] % 10);
        Lg1 = Lg1 % Lg2;
        Lg2 = Lg2 / 100;
    }

    (VOID) memset( Ptd, 0x00, Lgd ) ;
    Ptd += Lgd ;
    Pt0 = Ptd ;
    if ( Lgd > 5) Lgd = 5 ;
    for ( I=0; I < Lgd;I++) *--Ptd = Tb[4-I] ;

    return((UCHAR*)Pt0);
}
UCHAR *long_asc( UCHAR *Ptd, UCHAR Lgd, ULONG *Pts )
{
    UCHAR I,
    Oct,
    *Pt0,
    Tb[10];
    ULONG Lg1,
    Lg2;

    Lg1 = *Pts;
    Lg2 = 100000000L ;
    for (I = 0; I< 5; I++)
    {
        Oct = (UCHAR)(Lg1 / Lg2) ;
        Tb[2*I] = Oct / 10 + 0x30 ;
        Tb[2*I+1] = Oct % 10 + 0x30;
        Lg1 = Lg1 % Lg2;
        Lg2 = Lg2 / 100;
    }

    (VOID) memset( Ptd, 0x30, Lgd ) ;
    Ptd += Lgd ;
    Pt0 = Ptd ;
    if ( Lgd > 10 ) Lgd = 10 ;
    for ( I=0; I < Lgd;I++) *--Ptd = Tb[9-I] ;

    return((UCHAR*)Pt0);
}

UCHAR *long_str( UCHAR *Ptd, UCHAR Lgd, ULONG *Pts )
{
    *(Ptd +Lgd) = 0x00;
    return( long_asc( Ptd, Lgd, Pts ) );
}
UCHAR *long_tab(UCHAR *Ptd ,UCHAR Lgd,ULONG *Pts)
{
    UCHAR I;

    (VOID) memset( Ptd, 0x00, Lgd ) ;
    if (Lgd > 4)
    {
        Ptd += ( Lgd-4) ;
        Lgd = 4;
    }
    for (I = (4-Lgd); I < 4 ; I++)
    {
        *Ptd++ = (UCHAR) (*Pts >> (24 - 8*I) );
    }
    return((UCHAR*) Ptd );
}
UCHAR *short_bcd(UCHAR *Ptd ,UCHAR Lgd,unsigned short *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_bcd( Ptd, Lgd, &Lg2) );
}
UCHAR *int_bcd(UCHAR *Ptd ,UCHAR Lgd,UINT *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_bcd( Ptd, Lgd, &Lg2) );
}
UCHAR *short_str(UCHAR *Ptd ,UCHAR Lgd,unsigned short *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_str( Ptd, Lgd, &Lg2) );
}
UCHAR *int_str(UCHAR *Ptd ,UCHAR Lgd,UINT *Pts)
{
    ULONG Lg2;

    Lg2 = *Pts;
    return((UCHAR*) long_str( Ptd, Lgd, &Lg2) );
}
UCHAR *asc_bcd(UCHAR *Ptd ,UCHAR Lgd,UCHAR *Pts,UCHAR Lgs)
{
    UCHAR I;

    if ( Lgd > Lgs/2)
    {
        (VOID) memset( Ptd, 0x00, Lgd ) ;
        Ptd = Ptd + Lgd - ((Lgs + 1) / 2) ;
    }
    for ( I = 0 ; I < ((Lgs+1) / 2) ; I++)
    {
        if ( (!(Lgs % 2) && !I) || I ) *Ptd =  (*Pts++ << 4) & 0xF0 ;
        *Ptd = *Ptd + (*Pts++ & 0x0F)  ;
        Ptd++ ;
    }
    return((UCHAR*)Ptd);
}
UCHAR Conv(UCHAR Oct)
{
    if (Oct >= 0x41)
        return ( (Oct - 7) & 0x0F );
    else
        return ( Oct & 0x0F );
}
UCHAR *asc_hex(UCHAR *Ptd ,UCHAR Ld,UCHAR *Pts,UCHAR Ls)
{
    UCHAR I;

    (VOID) memset( Ptd, 0x00, Ld ) ;

    Ptd = Ptd + Ld - ((Ls + 1) / 2) ;
    if ( Ls % 2 ) *Ptd++ = Conv(*Pts++) & 0x0F ;
    for ( I = 0 ; I < (Ls / 2) ; I++)
    {
        *Ptd =  (Conv(*Pts++) << 4) & 0xF0 ;
        *Ptd = *Ptd + (Conv(*Pts++) & 0x0F)  ;
        Ptd ++;
    }
    return((UCHAR*)Ptd);
}

ULONG asc_long(UCHAR *Pts,UCHAR Ls)
{
    UCHAR I;
    ULONG Lg1,
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
UCHAR *str_ebcdic( UCHAR *Pt  )
{
    return( (UCHAR*) asc_ebcdic( Pt, strlen((char*) Pt) ));
}
UCHAR *str_bcd(UCHAR *Ptd ,UCHAR Lgd,UCHAR *Pts)
{
    return( (UCHAR*) asc_bcd( Ptd, Lgd, Pts, strlen((char*) Pts) ));
}
UCHAR *str_hex(UCHAR *Ptd ,UCHAR Lgd,UCHAR *Pts)
{
    return ( (UCHAR*) asc_hex( Ptd, Lgd , Pts, strlen((char*) Pts) )) ;
}
VOID move_inc_ptr ( UCHAR **Ptd, UCHAR *Pts , UCHAR Lg )
{
    (VOID) memmove( (char *) *Ptd, (char *)Pts, Lg);

    *Ptd += Lg;
}

UCHAR *move_255 ( UCHAR *Ptd, UCHAR *Pts , UCHAR Lg )
{
    (VOID) memmove( (char *)Ptd, (char *)Pts, Lg);

    return( Ptd + Lg );
}
UCHAR *mov_to_str(UCHAR *Ptd, UCHAR *Pts )
{
    (VOID) memmove( (char *)Ptd, (char *)(Pts+1), *Pts);
    Ptd += *Pts;
    *Ptd++ = 0x00;
    return((UCHAR*)Ptd);
}
UCHAR ebcdic_asc (UCHAR *Pts ,UCHAR Lg)
{
    UCHAR I,J;

    for ( I = 0 ; I < Lg ; I++)
    {
        for ( J = 0; (J < 128) && (*Pts != Tb_ebcdic[J]) ; J++) ;
        if (J == 128) return (FALSE) ;
        else *Pts++ = J ;
    }

    return(TRUE);
}
UCHAR *asc_ebcdic(UCHAR *Pts ,UCHAR Lg)
{
    UCHAR I;

    for ( I = 0 ; I < Lg ; I++)
    {
        *Pts = Tb_ebcdic[*Pts] ;
        Pts ++;
    }

    return(Pts + Lg );
}
UCHAR *bcd_asc( UCHAR *Ptd , UCHAR *Pts , UCHAR Lg)
{
    UCHAR I;


    if ( Lg % 2 ) *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    for ( I = 0 ; I < (Lg / 2) ; I++)
    {
        *Ptd++ = ( (*Pts & 0xF0) >> 4 ) + 0x30 ;
        *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    }
    return((UCHAR*)Ptd);
}

UCHAR *bcd_str( UCHAR *Ptd , UCHAR *Pts , UCHAR Lg )
{
    *(Ptd +Lg) = 0x00;
    return( bcd_asc( Ptd, Pts, Lg ) );
}
ULONG bcd_long( UCHAR *Pts, UCHAR Ls )
{
    UCHAR I,
    Oct;
    ULONG Lg1,
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
UCHAR *hex_asc( UCHAR *Ptd, UCHAR *Pts, UCHAR Lg )
{
    UCHAR I,
    *Pt0;

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
    return((UCHAR*)Ptd);
}

UCHAR *hex_str( UCHAR *Ptd, UCHAR *Pts, UCHAR Lg )
{
    *(Ptd +Lg) = 0x00;
    return( hex_asc( Ptd, Pts, Lg ) );
}
*/
VOID Ks(UCHAR *Key, UCHAR Kn[16][48])
{
    UCHAR cd[56];

    UCHAR zt[60] ;

    int n;
    UCHAR tmp11, tmp12, tmp21, tmp22;
    int i;
    UCHAR *Knn;

    /* choix 1 */
    for (i = 0; i < 56; i++)
    {
        cd[i] = Key[T7_1_2[i]];
    }

    for (n = 0; n < 16; n++)
    {
        /* rotation ? gauche du vecteur en fonction de l'indice */
        if (T8[n] == 0)
        {
            tmp11 = cd[0];
            tmp21 = cd[28];
            memcpy( zt , &cd[1] , 55 );
            memcpy( cd , zt     , 55 );
            //   memmove(&cd[0], &cd[1], 55); /* ce qui est en 1 va en 0 */
            cd[27] = tmp11;
            cd[55] = tmp21;
        }
        else
        {
            tmp11 = cd[0];
            tmp12 = cd[1];
            tmp21= cd[28];
            tmp22 = cd[29];

            memcpy( zt , &cd[2] , 54 );
            memcpy( cd , zt     , 54 );

            //			memmove(&cd[0], &cd[2], 54); /* ce qui est en 2 va en 0 */

            cd[26] = tmp11;
            cd[27] = tmp12;
            cd[54] = tmp21;
            cd[55] = tmp22;
        }
        /* choix 2 */
        Knn = Kn[n];
        for (i = 0; i < 48; i++)
        {
            Knn[i] = cd[T9[i]];
        }
    }
}

/*----------------------------------------------------------------

	 Figure 2. Calculation of f(R, K)

-----------------------------------------------------------------*/
VOID fonction(UCHAR *Knn, UCHAR *r, UCHAR *s)
{
    /* n est l'indice de 1 a 16 pour choisir la cle
       r est R.
       s est le r?sultat. */

    UCHAR x[32];
    unsigned long *px;
    int i, l;
    UCHAR c;
    UCHAR t;

    /* fonction E */    /*  + OU exclusif */
    /* S?lection Sn */
    for (i = 0, l = 0, px = (unsigned long *) x; i < 8;)
    {
        c = 32 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 8 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 4 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 2 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 1 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 16 * (r[T3[l]] ^ Knn[l]);
        l++;
        /* extraction de la valeur */
        t = T6[i][c];
        i++;
        /* Eclatement de la valeur; 4 bits -> 4 bytes */
//		*px = *(long *)TE[t];
		memcpy(px,TE[t],sizeof(unsigned long));
        px++;
    }
    /* fonction P */
    for (i = 0; i < 32; i++)
    {
        s[i] = x[T5[i]];
    }
}

/*----------------------------------------------------------------
 Permutations initiale et finale
-----------------------------------------------------------------*/
VOID permutation(UCHAR *org, UCHAR *tab)
//UCHAR *org;
//UCHAR *tab;
{
    UCHAR tmp[64];
    int i;


    memcpy(tmp, org, 64);
    for (i = 0; i < 64; i++)
    {
        org[i] = tmp[tab[i]];
    }
}
/*----------------------------------------------------------------

       Figure 1. Enciphering Computation.

-----------------------------------------------------------------*/
VOID chiffrement(UCHAR *xi, UCHAR *xo, UCHAR Kn[16][48])
//UCHAR *xi;
//UCHAR *xo;
//UCHAR Kn[16][48];
{
    UCHAR r[32], l[32];
    UCHAR rp[32], lp[32];

    int i;
    int n;

    memcpy(l, &xi[0], 32);
    memcpy(r, &xi[32], 32);

    for (n = 0; n < 16; n++)
    {
        memcpy(lp, r, 32);

        fonction(Kn[n], r, rp);
        for (i = 0; i < 32; i++)
        {
            r[i] =( ( l[i]) ^ (rp[i] )  ) ;
        }
        memcpy(l, lp, 32);
    }
    memcpy(&xo[0], r, 32);
    memcpy(&xo[32], l, 32);

}


/*----------------------------------------------------------------

       Deciphering Computation.

-----------------------------------------------------------------*/

VOID dechiffrement(UCHAR *xi, UCHAR *xo, UCHAR Kn[16][48])
//UCHAR *xi;
//UCHAR *xo;
//UCHAR Kn[16][48];
{
    UCHAR r[32], l[32], rp[32], lp[32];

    int i;
    int n;

    memcpy(l, &xi[0], 32);
    memcpy(r, &xi[32], 32);

    for (n = 0; n < 16; n++)
    {
        memcpy(lp, r, 32);
        fonction(Kn[15 - n], r, rp);
        for (i = 0; i < 32; i++)
        {
            r[i] =( ( l[i] ) ^ ( rp[i] )) ;
        }
        memcpy(l, lp, 32);
    }

    memcpy(&xo[0], r, 32);
    memcpy(&xo[32], l, 32);
}

/*----------------------------------------------------------------
  Eclater 64 bits en 64 octets
-----------------------------------------------------------------*/
VOID eclater(UCHAR *buf_bit, UCHAR *byte)
//UCHAR *buf_bit;
//UCHAR *byte;
{
    int i;
    UCHAR m;

    for (i = 0; i < 8; i++)
    {
        for (m = 0x80; m != 0;  )  // m >>= 1)
        {
            if ((buf_bit[i] & m) != 0)
                *byte = 1;
            else
                *byte = 0;
            byte++;
            m=m/2 ;
        }
    }

}
/*----------------------------------------------------------------
  Compacter 64 octets en 64 bits
-----------------------------------------------------------------*/
VOID compacter(UCHAR *byte, UCHAR *buf_bit)
//UCHAR *byte;
//UCHAR *buf_bit;
{
    int i;
    UCHAR m, n;

    for (i = 0; i < 8; i++)
    {
        n = 0;
        for (m = 0x80; m != 0; )  //  m >>= 1)
        {
            if (*byte++)
                n = n | m;
            m=m/2 ;

        }
        buf_bit[i] = n;
    }
}



/*----------------------------------------------------------------
  D E S
-----------------------------------------------------------------*/

VOID des(UCHAR *binput, UCHAR *boutput, UCHAR *bkey)
{

    eclater(binput, input);
    eclater(bkey, Key);
    Ks(Key, Kn);

    permutation(input, T1);
    chiffrement(input, output, Kn);

    permutation(output, T2);
    compacter(output, boutput);
}




VOID desm1(UCHAR *binput, UCHAR *boutput, UCHAR *bkey)
{

    eclater(binput, input);
    eclater(bkey, Key);
    Ks(Key, Kn);

    permutation(input, T1);
    dechiffrement(input, output, Kn);
    permutation(output, T2);
    compacter(output, boutput);

}

/*
int  TripleDES_Encrypt(UCHAR *binput, UCHAR *boutput, UCHAR *bkey)
{	
	return(TripleDES_Hardware(binput,8,SCC_ENCRYPT,0,bkey,16,boutput,0));    //k1=K3
}

int  TripleDES_Decrypt(UCHAR *binput, UCHAR *boutput, UCHAR *bkey)
{
	printf("456\n");
	return(TripleDES_Hardware(binput,8,SCC_DECRYPT,0,bkey,16,boutput,0));   //K1=K3
}


int SHA1_Hardware_Compute(UCHAR *pucInData,UINT uiInLen,UCHAR *pucDigest)
{
	return(HASH_Hardware(pucInData,uiInLen,HASH_SHA1,pucDigest));
}

int SHA256_Hardware_Compute(unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucDigest)
{
	return(HASH_Hardware(pucInData,uiInLen,HASH_SHA256,pucDigest));
}

*/

/*----------------------------------------------------------------
 Fin
-----------------------------------------------------------------*/



/* calcule le chiffre selon DES :
	- in=pt sur nombre ? coder (8 octets =64 bits)
	- kk= pt sur clef d'encodage (8 octets=56 bits + parite)
	(Rq: parite viree pour le calcul donc calcul sur 56 bits et non 64)
	- resultat (nombre code) ds res_codage sur 8 octets */
UCHAR * codage_des( UCHAR * in , UCHAR * kk )
{
    des( in , res_codage , kk);
    return(res_codage);
}



/* calcule le d?chiffre selon DES :
	- in=pt sur nombre cod? ? d?coder (8 octets =64 bits)
	- kk= pt sur clef d'encodage (8 octets=56 bits + parite)
	(Rq: parite viree pour le calcul donc calcul sur 56 bits et non 64)
	- resultat (nombre d?code) ds pt res_decodage sur 8 octets */
UCHAR * decodage_des( UCHAR * in , UCHAR * kk )
{
    desm1( in , res_decodage , kk);
    return(res_decodage);
}

/* calcule le chiffre selon EDE=(DES(Kn)(DES-1(Kn+1)(DES(Kn)))) :
	- in=pt sur nombre cod? ? d?coder (8 octets =64 bits)
    - kk= pt sur clef d'encodage (16 octets=Kn(8) + Kn+1(8))
	(Rq: parite viree pour le calcul donc calcul sur 56 bits et non 64)
	- resultat (nombre d?code) ds pt res_decodage sur 8 octets */
VOID codage_ede( UCHAR * in , UCHAR * kk , UCHAR * out)
{
    UCHAR cleKn1[8],cleKn2[8],res_tmp[8];

    memcpy(cleKn1,kk,8);
    des( in , out , cleKn1);
    memcpy(cleKn2,&kk[8],8);
    desm1(out, res_tmp, cleKn2);
    memset(cleKn2,0,8);
    des( res_tmp , out , cleKn1);
    memset(cleKn1,0,8);
    memset(res_tmp,0,8);
    return;
}

VOID codage_ded( UCHAR * in , UCHAR * kk , UCHAR * out)
{
    UCHAR cleKn1[8],cleKn2[8],res_tmp[8];

    memcpy(cleKn1,kk,8);
    desm1( in , out , cleKn1);
    memcpy(cleKn2,&kk[8],8);
    des(out, res_tmp, cleKn2);
    memset(cleKn2,0,8);
    desm1( res_tmp , out , cleKn1);
    memset(cleKn1,0,8);
    memset(res_tmp,0,8);
    return;
}


/* PBOC Diversify Key */
VOID pboc_diversify_key(UCHAR *pucIn, UCHAR *pucOut, UCHAR *pucKey)
{
    UCHAR	aucDiversifyData[16];
    UCHAR	ucI;
    UCHAR	aucDataOut[16];


    memset(aucDiversifyData,0,sizeof(aucDiversifyData));
    memset(aucDataOut,0,sizeof(aucDataOut));

    memcpy(&aucDiversifyData[0],pucIn,8);
    for (ucI=0;ucI<8;ucI++)
    {
        aucDiversifyData[ucI+8] = aucDiversifyData[ucI] ^ 0xFF;
    }
    codage_ede( &aucDiversifyData[0] , pucKey , &aucDataOut[0]);
    codage_ede( &aucDiversifyData[8] , pucKey , &aucDataOut[8]);
    memcpy(pucOut,aucDataOut,16);
}

UINT minin(UINT a,UINT b)
{
    if (a>=b)
        return(b);
    else
        return(a);
}

VOID macgen(UCHAR *in,UINT len,UCHAR *key, UCHAR *out)
{
    UCHAR lp,thismove;
    UINT  pos;

#if 0
    memcpy(out,in,len);
#else
    memset(out,0,8);
    for (pos =0; pos < len; pos += (UINT)thismove)
    {
        thismove = minin(len-pos,8);
        for (lp=0;lp<thismove;lp++)
            out[lp] ^= in[lp+pos];

        des(out,out,key);
    }
#endif
}


VOID pboc_macgen(UCHAR *in,UINT len,UCHAR *key, UCHAR *out)
{
    UCHAR lp;
    UINT  pos,len2;
    UCHAR aucAddData[8];

    memcpy(aucAddData,"\x80\x00\x00\x00\x00\x00\x00\x00",8);

    len2 = len%8;
    if ( !len2 )
    {
        memcpy( in+len,aucAddData,8);
        len += 8;
    }
    else
    {
        memcpy( in+len, aucAddData,8-len2);
        len += (8-len2);
    }

    memset(out,0,8);
    for (pos =0; pos < len; pos += 8)
    {
        for (lp=0;lp<8;lp++)
            out[lp] ^= in[lp+pos];

        des(out,out,key);
    }
}

VOID schlum_tripemacgen(UCHAR *preferdata,UCHAR preferlen,UCHAR *initdata,
                        UCHAR *in,UINT len, UCHAR *out,
                        UCHAR *key)
{
    UCHAR lp;
    UINT  pos,len2;
    UCHAR aucAddData[8];
    UCHAR aucBuf[300];
    UINT  uiBufLen;

    memcpy(aucAddData,"\x80\x00\x00\x00\x00\x00\x00\x00",8);

    len2 = len%8;
    if ( !len2 )
    {
        memcpy( in+len,aucAddData,8);
        len += 8;
    }
    else
    {
        memcpy( in+len, aucAddData,8-len2);
        len += (8-len2);
    }
    for ( pos=0;pos< len; pos+= 8)
    {
        codage_ede( in+pos , key , in+pos);
    }
    memset(aucBuf,0,sizeof(aucBuf));
    memcpy(aucBuf,preferdata,preferlen);
    memcpy(&aucBuf[preferlen],in,len);
    uiBufLen = len+preferlen;
    len2 = uiBufLen%8;
    if ( !len2 )
    {
        memcpy( &aucBuf[uiBufLen],aucAddData,8);
        uiBufLen += 8;
    }
    else
    {
        memcpy( &aucBuf[uiBufLen], aucAddData,8-len2);
        uiBufLen += (8-len2);
    }

    memcpy(out,initdata,8);
    for (pos =0; pos < uiBufLen; pos += 8)
    {
        for (lp=0;lp<8;lp++)
            out[lp] ^= aucBuf[lp+pos];

        des(out,out,key);
    }
    desm1(out,out,key+8);
    des(out,out,key);
}

VOID get_mac(UCHAR *preferdata,UCHAR *in,UCHAR *initdata,
             UCHAR *out, UCHAR *key,UINT len)
{
    UCHAR lp;
    UINT  pos,len2;
    UCHAR aucAddData[8];
    UCHAR aucBuf[300];
    UINT  uiBufLen;

    memcpy(aucAddData,"\x80\x00\x00\x00\x00\x00\x00\x00",8);
    memset(aucBuf,0,sizeof(aucBuf));
    memcpy(aucBuf,preferdata,5);
    memcpy(&aucBuf[5],in,len);
    uiBufLen = len+5;
    len2 = uiBufLen%8;
    if ( !len2 )
    {
        memcpy( &aucBuf[uiBufLen],aucAddData,8);
        uiBufLen += 8;
    }
    else
    {
        memcpy( &aucBuf[uiBufLen], aucAddData,8-len2);
        uiBufLen += (8-len2);
    }
    memcpy(out,initdata,8);
    for (pos =0; pos < uiBufLen; pos += 8)
    {
        for (lp=0;lp<8;lp++)
            out[lp] ^= aucBuf[lp+pos];

        des(out,out,key);
    }
    desm1(out,out,key+8);
    des(out,out,key);

}

