/*
	Library Function
--------------------------------------------------------------------------
	FILE  libconv.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-22		Xiaoxi Jiang
    Last modified :	2002-10-22		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include <tools.h>

void *CONV_AscBcd(unsigned char *pucDest,unsigned int uiDestLen,
				unsigned char *pucSrc,unsigned int uiSrcLen)
{
	unsigned int uiI;

    if( uiDestLen > uiSrcLen/2)
    {
		memset( pucDest, 0x00, uiDestLen ) ;
        pucDest = pucDest + uiDestLen - ((uiSrcLen + 1) / 2) ;
    }
    for ( uiI = 0 ; uiI < ((uiSrcLen+1) / 2) ; uiI++)
    {
		if ( (!(uiSrcLen % 2) && !uiI) || uiI )
			*pucDest =  (*pucSrc++ << 4) & 0xF0 ;
	    *pucDest = *pucDest + (*pucSrc++ & 0x0F)  ;
	    pucDest++ ;
    }
	return(pucDest);

}

void *CONV_AscHex(unsigned char *pucDest,unsigned int uiDestLen,
				unsigned char *pucSrc,unsigned int uiSrcLen)
{
	unsigned int uiI;

	memset( pucDest, 0x00, uiDestLen ) ;

    pucDest = pucDest + uiDestLen - ((uiSrcLen + 1) / 2) ;
    if ( uiSrcLen % 2 )
    	*pucDest++ = CONV_CharHex(*pucSrc++) & 0x0F ;
    for ( uiI = 0 ; uiI < (uiSrcLen / 2) ; uiI++)
    {
	    *pucDest =  (CONV_CharHex(*pucSrc++) << 4) & 0xF0 ;
	    *pucDest = *pucDest + (CONV_CharHex(*pucSrc++) & 0x0F)  ;
	    pucDest ++;
    }
	return(pucDest);

}

unsigned long CONV_AscLong(unsigned char *pucSrc,unsigned int uiLen)
{
	unsigned int uiI;
	unsigned long ulTmp1,ulTmp2;

    ulTmp1 = 0 ;
    ulTmp2 = 1 ;
    pucSrc += uiLen ;
    for (uiI = 0; uiI< uiLen ; uiI++)
    {
        ulTmp1 += (ulTmp2 * (*--pucSrc & 0x0F) ) ;
        ulTmp2 *= 10 ;
    }

    return (ulTmp1) ;
}

void *CONV_BcdAsc(unsigned char *pucDest,unsigned char *pucSrc, int uiDestLen)
{
	int uiI;

    if(uiDestLen % 2)
    	*pucDest++ = (*pucSrc++ & 0x0F) + 0x30 ;
    for(uiI = 0 ; uiI < (uiDestLen / 2) ; uiI++)
    {
        *pucDest++ = ( (*pucSrc & 0xF0) >> 4 ) + 0x30 ;
        *pucDest++ = (*pucSrc++ & 0x0F) + 0x30 ;
    }
	return( pucDest);
}

unsigned long CONV_BcdLong(unsigned char *pucSrc, unsigned int uiSrcLen)
{
	unsigned int uiI;
	unsigned char ucCh;
	unsigned long ulTmp1,ulTmp2;

    ulTmp1 = 0 ;
    ulTmp2 = 1 ;
    pucSrc += (uiSrcLen+1)/2;
    for (uiI = 0; uiI< uiSrcLen ; uiI++)
    {
        if(uiI % 2)
        	ucCh = (*pucSrc >> 4 ) & 0x0F;
        else
        	ucCh = *--pucSrc & 0x0F;
        ulTmp1 += ulTmp2 * ucCh ;
        if(ulTmp2 == 1000000000L )
        	ulTmp2 = 0 ;
        else
        	ulTmp2 *= 10 ;
    }
    return (ulTmp1);
}

void *CONV_BcdStr(unsigned char *pucDest,unsigned char *pucSrc,int uiDestLen)
{
	*(pucDest+uiDestLen) = 0x00;
	return(CONV_BcdAsc(pucDest,pucSrc,uiDestLen));
}

unsigned char CONV_BitStrCharBit(unsigned char *pucSrc)
{
    unsigned char ucCh;
    unsigned char ucMask;

    ucMask = 0x80;
    ucCh = 0x00;
    do{
        if( *pucSrc == '1')
        {
            ucCh |= ucMask;
        }
        pucSrc ++;
        ucMask >>= 1;
    }while(ucMask);

    return(ucCh);
}

void *CONV_CharAsc(unsigned char *pucDest,unsigned char ucLen,unsigned char *pucSrc)
{
	unsigned long ulData;

    ulData = *pucSrc;
	return(CONV_LongAsc( pucDest, ucLen, &ulData) );
}

void *CONV_CharBcd(unsigned char *pucDest,unsigned char ucLen,unsigned char *pucSrc)
{
	unsigned long ulData;

    ulData = *pucSrc;
	return(CONV_LongBcd( pucDest, ucLen, &ulData) );
}

void CONV_CharBitStr(unsigned char *pucDest,unsigned char ucCh)
{
    unsigned char ucMask;

    ucMask = 0x80;
    do{
        if(ucCh & ucMask )
        {
            *pucDest = '1';
        }else
        {
            *pucDest = '0';
        }
        pucDest ++;
        ucMask >>= 1;
    }while(ucMask);
}

unsigned char CONV_CharHex(unsigned char ucCh)
{
	if (ucCh >= 0x41)
		return ( (ucCh - 7) & 0x0F );
	else
		return ( ucCh & 0x0F );
}

void *CONV_CharStr(unsigned char *pucDest,unsigned char ucLen,unsigned char *pucSrc)
{
	unsigned long ulData;

    ulData = *pucSrc;
	return(CONV_LongStr( pucDest, ucLen, &ulData) );
}

unsigned char CONV_CharLowCase(unsigned char ucCh)
{
    if(  (ucCh >= 0x41 )
       &&(ucCh <= 0x5A )
      )
    {
        ucCh += 0x20;
    }
	return(ucCh);
}

unsigned char CONV_CharUpperCase(unsigned char ucCh)
{
    if(  (ucCh >= 0x61 )
       &&(ucCh <= 0x7A )
      )
    {
        ucCh -= 0x20;
    }
	return(ucCh);
}

void *CONV_HexAsc(unsigned char *pucDest,unsigned char *pucSrc,unsigned int uiLen)
{
	unsigned int uiI;
	unsigned char *pucPtr;

    pucPtr = pucDest ;
    if ( uiLen % 2 )
        *pucDest++ = (*pucSrc++ & 0x0F) + 0x30 ;
    for ( uiI = 0 ; uiI < (uiLen / 2) ; uiI++)
    {
        *pucDest++ = ( (*pucSrc & 0xF0) >> 4 ) + 0x30 ;
        *pucDest++ = (*pucSrc++ & 0x0F) + 0x30 ;
    }
    while (pucPtr != pucDest)
    {
        if (*pucPtr >= 0x3A)
            *pucPtr += 7 ;
        pucPtr++;
    }
	return((unsigned char*)pucDest);
}

unsigned long CONV_HexLong(unsigned char *pucSrc,unsigned int uiLen)
{
	unsigned int uiI;
	unsigned long ulI,ulJ;

    ulI = 0 ;
    ulJ = 1 ;

	pucSrc += uiLen ;

	for (uiI = 0; uiI< uiLen ; uiI++)
    {
		ulI += ulJ * (*--pucSrc);
        if (ulJ == 0x80000000L )
        	ulJ = 0 ;
        else ulJ *= 256 ;
    }
    return (ulI) ;
}

void *CONV_HexStr(unsigned char *pucDest,unsigned char *pucSrc,unsigned int uiLen)
{
	*(pucDest+uiLen) = 0x00;
	return( CONV_HexAsc(pucDest,pucSrc,uiLen));
}

void *CONV_IntAsc(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc)
{
	unsigned long ulData;

    ulData = *puiSrc;
	return(CONV_LongAsc( pucDest, uiLen, &ulData) );
}

void *CONV_IntBcd(unsigned char *pucDest, unsigned int uiLen,unsigned int *puiSrc)
{
	unsigned long ulTmp;

    ulTmp = *puiSrc;
	return(CONV_LongBcd( pucDest, uiLen, &ulTmp) );
}

void *CONV_IntHex(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc)
{
	unsigned long ulLen;

    ulLen = *puiSrc;
	return((unsigned char*)CONV_LongHex( pucDest, uiLen, &ulLen) );
}

void *CONV_IntStr(unsigned char *pucDest,unsigned int uiLen,unsigned int *puiSrc)
{
	unsigned long ulData;

    ulData = *puiSrc;
	return(CONV_LongStr( pucDest, uiLen, &ulData) );
}

void *CONV_LongAsc( unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc )
{
	unsigned int uiI;
	unsigned char ucCh;
	unsigned char aucTab[10];
	unsigned char *pucPtr;
	unsigned long ulTmp1,ulTmp2;

    ulTmp1 = *pulSrc;
    ulTmp2 = 100000000L ;
    for (uiI = 0; uiI< 5; uiI++)
    {
		ucCh = (unsigned char)(ulTmp1 / ulTmp2) ;
		aucTab[2*uiI] = ucCh / 10 + 0x30 ;
		aucTab[2*uiI+1] = ucCh % 10 + 0x30;
        ulTmp1 = ulTmp1 % ulTmp2;
        ulTmp2 = ulTmp2 / 100;
    }

	memset( pucDest, 0x30, uiLen ) ;
	pucDest += uiLen ;
    pucPtr = pucDest ;
	if ( uiLen > 10 )
		uiLen = 10 ;
	for( uiI=0; uiI < uiLen;uiI++)
		*--pucDest = aucTab[9-uiI] ;

	return(pucPtr);
}

void *CONV_LongBcd(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc)
{
	unsigned int uiI;
	unsigned long ulTmp1,ulTmp2;
	unsigned char *pucPtr;
	unsigned char aucTab[5];

    ulTmp1 = *pulSrc;
    ulTmp2 = 100000000L;
    for(uiI = 0; uiI< 5; uiI++)
    {
		aucTab[uiI] = (unsigned char)(ulTmp1 / ulTmp2) ;
        aucTab[uiI] = ((aucTab[uiI] / 10 ) << 4 ) + (aucTab[uiI] % 10);
        ulTmp1 = ulTmp1 % ulTmp2;
        ulTmp2 = ulTmp2 / 100;
    }

	memset( pucDest, 0x00, uiLen ) ;
    pucDest += uiLen ;
    pucPtr = pucDest ;
    if( uiLen  > 5)
    	uiLen = 5 ;
    for ( uiI=0; uiI < uiLen;uiI++)
    	*--pucDest = aucTab[4-uiI] ;

	return((void *)pucPtr);

}

void *CONV_LongHex(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc)
{
	unsigned int uiI;

	memset( pucDest, 0x00, uiLen ) ;
    if (uiLen > 4)
    {
        pucDest += ( uiLen-4) ;
        uiLen = 4;
    }
    for(uiI = (4-uiLen); uiI < 4 ; uiI++)
    {
		*pucDest++ = (unsigned char) (*pulSrc >> (24 - 8*uiI) );
    }
	return((unsigned char*) pucDest );
}

void *CONV_LongStr(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc )
{
	*(pucDest+uiLen) = 0x00;
	return(CONV_LongAsc(pucDest,uiLen,pulSrc));
}

void *CONV_ShortAsc(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc)
{
	unsigned long ulData;

    ulData = *puiSrc;
	return(CONV_LongAsc( pucDest, uiLen, &ulData) );
}

void *CONV_ShortBcd(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc)
{
	unsigned long ulTmp;

    ulTmp = *puiSrc;
	return(CONV_LongBcd( pucDest, uiLen, &ulTmp) );
}

void *CONV_ShortHex(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc)
{
	unsigned long ulLen;

    ulLen = *puiSrc;
	return((unsigned char*)CONV_LongHex( pucDest, uiLen, &ulLen) );
}

void *CONV_ShortStr(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc)
{
	unsigned long ulData;

    ulData = *puiSrc;
	return(CONV_LongStr( pucDest, uiLen, &ulData) );
}

void *CONV_StrBcd(unsigned char *pucDest,unsigned int uiDestLen,unsigned char *pucSrc)
{
	return(CONV_AscBcd(pucDest,uiDestLen,pucSrc,strlen((char *)pucSrc)));
}

void *CONV_StrHex(unsigned char *pucDest,unsigned int uiDestLen,unsigned char *pucSrc)
{
	return(CONV_AscHex(pucDest,uiDestLen,pucSrc,strlen((char *)pucSrc)));
}

unsigned long CONV_StrLong(unsigned char *pucSrc)
{
	return(CONV_AscLong(pucSrc,strlen((char *)pucSrc)));
}

void CONV_StrLowCase(char *pcStr)
{
    unsigned int uiI;
    unsigned char *pucPtr;

    pucPtr = (unsigned char *)pcStr;
    for(uiI=0;uiI<strlen(pcStr);uiI++)
    {
        *pucPtr = CONV_CharLowCase(*pucPtr);
        pucPtr ++;
    }
    return;
}

void CONV_StrUpperCase(char *pcStr)
{
    unsigned int uiI;
    unsigned char *pucPtr;

    pucPtr = (unsigned char *)pcStr;
    for(uiI=0;uiI<strlen(pcStr);uiI++)
    {
        *pucPtr = CONV_CharUpperCase(*pucPtr);
        pucPtr ++;
    }
	return;
}

void CONV_StrTrimRight(char *pcStr,char cCh)
{
	unsigned int uiLen;

	if( !pcStr )
	{
		return;
	}
	uiLen = strlen(pcStr);
	if( uiLen == 0)
		return;
	while( *(pcStr+uiLen-1) == cCh)
	{
		*(pcStr+uiLen-1) = 0;
		uiLen --;
		if( uiLen == 0)
			break;
	}
	return;
}

void CONV_StrTrimLeft(char *pcStr,char cCh)
{
	unsigned int uiLen;
	unsigned int uiOffset;
	unsigned int uiI;

	if( !pcStr )
	{
		return;
	}
	uiLen = strlen(pcStr);
	if( uiLen == 0)
		return;

	for(uiOffset=0;uiOffset<uiLen;uiOffset++)
	{
		if( *(pcStr+uiOffset) != cCh)
			break;
	}
	if( uiOffset == uiLen)
	{
		*pcStr = 0;
		return;
	}
	for(uiI=0;uiI<uiLen-uiOffset;uiI++)
	{
		*(pcStr+uiI) = *(pcStr+uiOffset+uiI);
	}
	*(pcStr+uiI) = 0;
	return;
}
