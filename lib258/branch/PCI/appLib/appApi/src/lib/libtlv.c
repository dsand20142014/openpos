/*
	Library Function
--------------------------------------------------------------------------
	FILE  libtlv.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2005-05-16		Xiaoxi Jiang
    Last modified :	2005-05-16		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include <tools.h>


unsigned char TLV_DecodeSAND(unsigned char *pucInData,unsigned int uiInLen,
                        TLV *pTLV,unsigned short *puiNbr)
{
    unsigned short uiI;
    unsigned short uiNbr;
    unsigned char ucTag;
    unsigned char ucLen;
    unsigned int uiLen;
    unsigned char *pucPtr;

    if( !puiNbr )
        return(TLVERR_INVALIDPARAM);

    uiI = 0;
    uiNbr = *puiNbr;
    *puiNbr = 0;
    pucPtr = pucInData;
    while((pucPtr-pucInData)<uiInLen)
    {
        ucTag = *pucPtr;
        if( !ucTag )
            break;
        pucPtr ++;
        if( *pucPtr & 0x80 )
        {
            ucLen = *pucPtr & 0x0F;
            pucPtr ++;
            uiLen = CONV_HexLong(pucPtr,ucLen);
            pucPtr += ucLen;
        }else
        {
            uiLen = *pucPtr;
            pucPtr ++;
        }
        if( uiI < uiNbr )
        {
            pTLV->uiTag = ucTag;
            pTLV->uiLen = uiLen;
            pTLV->pucPtr = pucPtr;
            pTLV ++;
        }
        uiI ++;
        pucPtr += uiLen;

    }
    *puiNbr = uiI;

    return(TLVERR_SUCCESS);

}

unsigned char TLV_AccessTagSAND(unsigned char *pucInData,unsigned int uiInLen,
                        unsigned short uiTag,TLV *pTLV)
{
    unsigned char ucTag;
    unsigned char ucLen;
    unsigned int uiLen;
    unsigned char *pucPtr;

    pucPtr = pucInData;
    while((pucPtr-pucInData)<uiInLen)
    {
        ucTag = *pucPtr;
        if( !ucTag )
            break;
        pucPtr ++;
        if( *pucPtr & 0x80 )
        {
            ucLen = *pucPtr & 0x0F;
            pucPtr ++;
            uiLen = CONV_HexLong(pucPtr,ucLen);
            pucPtr += ucLen;
        }else
        {
            uiLen = *pucPtr;
            pucPtr ++;
        }
        if( ucTag == uiTag )
        {
            pTLV->uiTag = ucTag;
            pTLV->uiLen = uiLen;
            pTLV->pucPtr = pucPtr;
            return(TLVERR_SUCCESS);
        }
        pucPtr += uiLen;

    }

    return(TLVERR_TAGNOTFOUND);

}

