/*
	SAND China
--------------------------------------------------------------------------
	FILE  iso8583.c								(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for ISO8583 package process.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>
#include <iso8583.h>
#include <xdata.h>


void ISO8583_MBFEClear(void)
{
	ushort uiI;
	for(uiI=0;uiI<ISO8583_MAXBITNUM;uiI++)
	{
		ISO8583_MBFE.Bit[uiI].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
		ISO8583_MBFE.Bit[uiI].uiOffset = 0;
		ISO8583_MBFE.Bit[uiI].uiLen = 0;
	}
	ISO8583_MBFE.uiBitBufLen = 0;
	memset(ISO8583_MBFE.aucBitBuf,0,ISO8583_MAXBITBUFLEN);
	ISO8583_MBFE.uiCommBufLen = 0;
	memset(ISO8583_MBFE.aucCommBuf,0,ISO8583_MAXCOMMBUFLEN);
}
unsigned char ISO8583_MBFESetBitValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen)
{
	const ISO8583BitAttr *pBitAttr;

	//util_Printf("\nMBFE Lt.cn\n");
	pBitAttr = MBFE_ISO8583BitAttrTab;


	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}

	/* uiMaxLen+1 is important, for N type data:) */
	if( uiInLen > pBitAttr[iBitNo].uiMaxLen+1)
	{
		return(ERR_ISO8583_INVALIDPACKLEN);
	}
	if( uiInLen + ISO8583_MBFE.uiBitBufLen > ISO8583_MAXBITBUFLEN )
	{
		return(ERR_ISO8583_OVERBITBUFLIMIT);
	}
	ISO8583_MBFE.Bit[iBitNo].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
	ISO8583_MBFE.Bit[iBitNo].uiOffset = ISO8583_MBFE.uiBitBufLen;
	ISO8583_MBFE.Bit[iBitNo].uiLen = uiInLen;
	if( uiInLen )
	{
		memcpy( &(ISO8583_MBFE.aucBitBuf[ISO8583_MBFE.uiBitBufLen]),
				pucInData,uiInLen);
		ISO8583_MBFE.uiBitBufLen += uiInLen;
	}
	ISO8583_MBFE.Bit[iBitNo].ucExistFlag = ISO8583_BITEXISTFLAG;
	return(SUCCESS);
}
unsigned char ISO8583_MBFEPackData(unsigned char *pucOutData, unsigned short *puiOutDataLen, short iOutLen)
{
	short iI;
	unsigned char aucBitmap[ISO8583_BITMAPLEN];
	unsigned char aucLen[4];
	unsigned char ucMask;
	unsigned char *pucPtr;
	unsigned short uiBitLen;
	unsigned short uiOffset;
	unsigned short uiPackLen;
	const ISO8583BitAttr *pBitAttr;

	memset(aucBitmap,0,sizeof(aucBitmap));
	memset(aucLen,0,sizeof(aucLen));

	//util_Printf("MBFE Lt.cn\n");
	pBitAttr = MBFE_ISO8583BitAttrTab;

	//Setup bitmap
	ucMask = 0x80;

	for(iI = 0; iI < ISO8583_MAXBITNUM;iI++)
	{
		if( ISO8583_MBFE.Bit[iI].ucExistFlag == ISO8583_BITEXISTFLAG)
		{
			aucBitmap[iI/8] |= ucMask;
		}
		ucMask = ucMask >> 1;
		if( !ucMask )
		{
			ucMask = 0x80;
		}
	}

	for(iI=0;iI<ISO8583_BITMAPLEN;iI++)
	{
		if( aucBitmap[ISO8583_BITMAPLEN-iI-1] )
		{
			break;
		}
	}

	if( iI == ISO8583_BITMAPLEN)
	{
		return(ERR_ISO8583_NODATAPACK);
	}

	pucPtr = pucOutData;

//	if( iOutLen > 2)
//	{
//		memcpy(pucPtr,ISO8583_MBFE.aucMsgID,2);
//		pucPtr += 2;
//	}else
//	{
//		return(ERR_ISO8583_OVERBUFLIMIT);
//	}

	if( iI >= 8)
	{
		if( (pucPtr-pucOutData)+8 > iOutLen)
		{
			return(ERR_ISO8583_OVERBUFLIMIT);
		}else
		{
			memcpy(pucPtr,aucBitmap,8);
			pucPtr += 8;
		}
	}else
	{
		if( (pucPtr-pucOutData)+ISO8583_BITMAPLEN > iOutLen)
		{
			return(ERR_ISO8583_OVERBUFLIMIT);
		}else
		{
			aucBitmap[0] |= 0x80;
			memcpy(pucPtr,aucBitmap,ISO8583_BITMAPLEN);
			pucPtr += ISO8583_BITMAPLEN;
		}
	}

	for(iI = 1;iI< ISO8583_MAXBITNUM; iI++)
	{
		if( ISO8583_MBFE.Bit[iI].ucExistFlag == ISO8583_BITEXISTFLAG)
		{
			uiBitLen = ISO8583_MBFE.Bit[iI].uiLen;
			uiOffset = ISO8583_MBFE.Bit[iI].uiOffset;
			util_Printf("ISO8583_MBFE:uiBitLen[%d]=%d\n",iI+1,uiBitLen);

			switch(pBitAttr[iI].ucAttr)
			{
			case ATTR_N:
				uiPackLen = (pBitAttr[iI].uiMaxLen+1)/2;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
#if 1
					asc_hex(pucPtr,uiPackLen,&ISO8583_MBFE.aucBitBuf[uiOffset],uiBitLen);
					pucPtr += uiPackLen;

#else
					if( uiBitLen%2)
					{
						asc_hex(pucPtr,uiPackLen,&ISO8583_MBFE.aucBitBuf[uiOffset],uiBitLen+1);
					}else
					{
						asc_hex(pucPtr,uiPackLen,&ISO8583_MBFE.aucBitBuf[uiOffset],uiBitLen);
					}
					pucPtr += uiPackLen;
#endif
				}
				break;
			case ATTR_BIN:
				uiPackLen = ((pBitAttr+iI)->uiMaxLen+1)/8;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					asc_hex(pucPtr,uiPackLen,&ISO8583_MBFE.aucBitBuf[uiOffset],uiBitLen);
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_AN:
			case ATTR_NS:
			case ATTR_ANS:
			case ATTR_XN:
				uiPackLen = (pBitAttr+iI)->uiMaxLen;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					memcpy(pucPtr,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen);
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_LLVARN:
			case ATTR_LLVARZ:
				uiPackLen = (uiBitLen+1)/2;
				if( (pucPtr-pucOutData+uiPackLen+1) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,1,&uiBitLen);
					pucPtr += 1;

					asc_bcd(pucPtr,uiPackLen,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen*2);
					pucPtr += uiPackLen;
					if( uiBitLen%2 )
					{
						*(pucPtr-1) &= 0xF0;
					}
				}
				break;
			case ATTR_LLLVARN:
			case ATTR_LLLVARZ:
				uiPackLen = (uiBitLen+1)/2;

				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					util_Printf("error iI=%d\n",iI);
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,2,&uiBitLen);
					pucPtr += 2;
                                   if ( uiPackLen*2 <= 256)
                                   {
                			    asc_bcd(pucPtr,uiPackLen,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen*2);
                                   }else
                                   {
                                        asc_bcd(pucPtr,127,&ISO8583_MBFE.aucBitBuf[uiOffset],254);
                                        asc_bcd(&pucPtr[127],uiPackLen-127,&ISO8583_MBFE.aucBitBuf[uiOffset+254],uiPackLen*2-254);
                                   }
					pucPtr += uiPackLen;
#if 1
					if( uiBitLen%2 )
					{
						*(pucPtr-1) &= 0xF0;
					}
#endif
				}
				break;
			case ATTR_LLVARAN:
			case ATTR_LLVARNS:
			case ATTR_LLVARANS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+1) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,1,&uiBitLen);
					pucPtr += 1;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += uiPackLen;
					}
				}
				break;
			case ATTR_LLVARANZS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+1) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,1,&uiBitLen);
					pucPtr += 1;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += (uiPackLen+1)/2;
					}
				}
				break;
			case ATTR_LLLVARANS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,2,&uiBitLen);
					pucPtr += 2;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += uiPackLen;
					}
				}
				break;
			case ATTR_LLLVARANZS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,2,&uiBitLen);
					pucPtr += 2;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583_MBFE.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += (uiPackLen+1)/2;
					}
				}
				break;
			default:
				return(ERR_ISO8583_INVALIDBITATTR);
			}
		}
	}
	*puiOutDataLen = (pucPtr - pucOutData);
	return(SUCCESS);
}
//
void ISO8583_Clear(void)
{
	ushort uiI;
	for(uiI=0;uiI<ISO8583_MAXBITNUM;uiI++)
	{
		ISO8583Data.Bit[uiI].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
		ISO8583Data.Bit[uiI].uiOffset = 0;
		ISO8583Data.Bit[uiI].uiLen = 0;
	}
	ISO8583Data.uiBitBufLen = 0;
	//memset(ISO8583Data.aucMsgID,0,ISO8583_MSGIDLEN);    //
	memset(ISO8583Data.aucBitBuf,0,ISO8583_MAXBITBUFLEN);
	ISO8583Data.uiCommBufLen = 0;
	memset(ISO8583Data.aucCommBuf,0,ISO8583_MAXCOMMBUFLEN);
}

void ISO8583_ClearBit(void)
{
	ushort uiI;
	for(uiI=0;uiI<ISO8583_MAXBITNUM;uiI++)
	{
		ISO8583Data.Bit[uiI].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
		ISO8583Data.Bit[uiI].uiOffset = 0;
		ISO8583Data.Bit[uiI].uiLen = 0;
	}
}

void ISO8583_ClearBitBuf(void)
{
	ISO8583Data.uiBitBufLen = 0;
	memset(ISO8583Data.aucBitBuf,0,ISO8583_MAXBITBUFLEN);
}

void ISO8583_ClearCommBuf(void)
{
	ISO8583Data.uiCommBufLen = 0;
	memset(ISO8583Data.aucCommBuf,0,ISO8583_MAXCOMMBUFLEN);
}

unsigned char ISO8583_GetMsgID(unsigned short *piMsgID)
{
	short iMsgID;

	iMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
	*piMsgID = iMsgID;
	return(SUCCESS);
}

unsigned char ISO8583_SetMsgID(unsigned short iMsgID)
{
	short_bcd(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN,&iMsgID);
	return(SUCCESS);
}
unsigned char ISO8583_GetBitValue(short iBitNo, unsigned char *pucOutData, short *puiOutDataLen, short iOutBufLen)
{
	short iI,iJ;

	if(!pucOutData )
	{
		return(ERR_ISO8583_INVALIDPARAM);
	}

	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}

	if( ISO8583Data.Bit[iBitNo].ucExistFlag == ISO8583_BITEXISTFLAG)
	{
		iI = ISO8583Data.Bit[iBitNo].uiLen;
		iJ = ISO8583Data.Bit[iBitNo].uiOffset;
/*
#ifdef TEST
		util_Printf("iBitNo=%02x\n",iBitNo);
		util_Printf("ISO8583Data.Bit[iBitNo].uiLen=%3d\n",iI);
		util_Printf("iOutBufLen=%3d\n",iOutBufLen);
		util_Printf("-----------------------------------------------------\n");
#endif
*/
		if( iI > iOutBufLen)
		{
			memcpy(pucOutData,&ISO8583Data.aucBitBuf[iJ],iOutBufLen);
			*puiOutDataLen = iOutBufLen;
		}else
		{
			memcpy(pucOutData,&ISO8583Data.aucBitBuf[iJ],iI);
			*puiOutDataLen = iI;
		}
		return(SUCCESS);
	}else
	{
/*
#ifdef TEST
		util_Printf("ERR_ISO8583_BITNOTEXIST\n");
#endif
*/
		return(ERR_ISO8583_BITNOTEXIST);
	}
}

unsigned char ISO8583_SetBitValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen)
{
	const ISO8583BitAttr *pBitAttr;

	if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_UNTOUCH)
	{
		util_Printf("\nUntouch ISO\n");
		pBitAttr = ISO8583BitAttrTab_Untouch;
	}
	else if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_MAG
		||G_NORMALTRANS_euCardSpecies == CARDSPECIES_EMV)
	{
		util_Printf("\nMagic & IC ISO");
		pBitAttr = Default_ISO8583BitAttrTab;
	}
	else
	{
		util_Printf("\nSet Bit SAND Lt.cn\n");
		pBitAttr = SAND_ISO8583BitAttrTab;
	}
	/* Can pack 0 length data in shanghai */
#if 0
	if( uiInLen == 0)
	{
		return(ERR_ISO8583_INVALIDVALUE);
	}
#endif

	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}
/*
#ifdef TEST
	util_Printf("iBitNo=%02x\n",iBitNo);
#endif
*/
	/* uiMaxLen+1 is important, for N type data:) */
	if( uiInLen > pBitAttr[iBitNo].uiMaxLen+1)
	{
		return(ERR_ISO8583_INVALIDPACKLEN);
	}
	if( uiInLen + ISO8583Data.uiBitBufLen > ISO8583_MAXBITBUFLEN )
	{
		return(ERR_ISO8583_OVERBITBUFLIMIT);
	}
	ISO8583Data.Bit[iBitNo].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
	ISO8583Data.Bit[iBitNo].uiOffset = ISO8583Data.uiBitBufLen;
	ISO8583Data.Bit[iBitNo].uiLen = uiInLen;
	if( uiInLen )
	{
		memcpy( &(ISO8583Data.aucBitBuf[ISO8583Data.uiBitBufLen]),
				pucInData,uiInLen);
		ISO8583Data.uiBitBufLen += uiInLen;
	}
	ISO8583Data.Bit[iBitNo].ucExistFlag = ISO8583_BITEXISTFLAG;
	return(SUCCESS);
}

unsigned char ISO8583_SetBitHexValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen)
{
	const ISO8583BitAttr *pBitAttr;

	//pBitAttr = Default_ISO8583BitAttrTab;
	if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_UNTOUCH)
	{
		util_Printf("Untouch ISO Hex\n");
		pBitAttr = ISO8583BitAttrTab_Untouch;
	}
	else if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_MAG
		||G_NORMALTRANS_euCardSpecies == CARDSPECIES_EMV)
	{
		util_Printf("Magic & IC ISO Hex\n");
		pBitAttr = Default_ISO8583BitAttrTab;
	}
	else
	{
		util_Printf("Set Hex SAND Lt.cn Hex\n");
		pBitAttr = SAND_ISO8583BitAttrTab;
	}

	if( uiInLen == 0)
	{
		return(ERR_ISO8583_INVALIDVALUE);
	}
	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}
	if( uiInLen > pBitAttr[iBitNo].uiMaxLen)
	{
		return(ERR_ISO8583_INVALIDPACKLEN);
	}
	if( uiInLen + ISO8583Data.uiBitBufLen > ISO8583_MAXBITBUFLEN )
	{
		return(ERR_ISO8583_OVERBITBUFLIMIT);
	}
	ISO8583Data.Bit[iBitNo].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
	ISO8583Data.Bit[iBitNo].uiOffset = ISO8583Data.uiBitBufLen;
	ISO8583Data.Bit[iBitNo].uiLen = uiInLen*2;
	hex_asc( &(ISO8583Data.aucBitBuf[ISO8583Data.uiBitBufLen]),
			pucInData,uiInLen*2);
	ISO8583Data.uiBitBufLen += uiInLen*2;
	ISO8583Data.Bit[iBitNo].ucExistFlag = ISO8583_BITEXISTFLAG;

	return(SUCCESS);
}

unsigned char ISO8583_CheckBit(short iBitNo)
{
	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}

	if( ISO8583Data.Bit[iBitNo].ucExistFlag == ISO8583_BITEXISTFLAG)
	{
		return(SUCCESS);
	}else
	{
		return(ERR_ISO8583_BITNOTEXIST);
	}
}
unsigned char ISO8583_RemoveBit(short iBitNo)
{
	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}
	ISO8583Data.Bit[iBitNo].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
	ISO8583Data.Bit[iBitNo].uiOffset = 0;
	ISO8583Data.Bit[iBitNo].uiLen = 0;
	return(SUCCESS);
}

unsigned char ISO8583_PackData(unsigned char *pucOutData, unsigned short *puiOutDataLen, short iOutLen)
{
	short iI;
	unsigned char aucBitmap[ISO8583_BITMAPLEN];
	unsigned char aucLen[4];
	unsigned char ucMask;
	unsigned char *pucPtr;
	unsigned short uiBitLen;
	unsigned short uiOffset;
	unsigned short uiPackLen;
	const ISO8583BitAttr *pBitAttr;


	memset(aucBitmap,0,sizeof(aucBitmap));
	memset(aucLen,0,sizeof(aucLen));

	if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_UNTOUCH)
	{
		util_Printf("非接触卡\n");
		pBitAttr = ISO8583BitAttrTab_Untouch;
	}
	else if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_MAG
		||G_NORMALTRANS_euCardSpecies == CARDSPECIES_EMV)
	{
		util_Printf("磁条或芯片卡\n");
		pBitAttr = Default_ISO8583BitAttrTab;
	}
	else
	{
		util_Printf("Pack SAND Lt.cn\n");
		pBitAttr = SAND_ISO8583BitAttrTab;
	}
	//Setup bitmap
	ucMask = 0x80;

	for(iI = 0; iI < ISO8583_MAXBITNUM;iI++)
	{
		if( ISO8583Data.Bit[iI].ucExistFlag == ISO8583_BITEXISTFLAG)
		{
			aucBitmap[iI/8] |= ucMask;
		}
		ucMask = ucMask >> 1;
		if( !ucMask )
		{
			ucMask = 0x80;
		}
	}

	for(iI=0;iI<ISO8583_BITMAPLEN;iI++)
	{
		if( aucBitmap[ISO8583_BITMAPLEN-iI-1] )
		{
			break;
		}
	}

	if( iI == ISO8583_BITMAPLEN)
	{
		return(ERR_ISO8583_NODATAPACK);
	}

	pucPtr = pucOutData;

	if( iOutLen > 2)
	{
		memcpy(pucPtr,ISO8583Data.aucMsgID,2);
		pucPtr += 2;
	}else
	{
		return(ERR_ISO8583_OVERBUFLIMIT);
	}

	if( iI >= 8)
	{
		if( (pucPtr-pucOutData)+8 > iOutLen)
		{
			return(ERR_ISO8583_OVERBUFLIMIT);
		}else
		{
			memcpy(pucPtr,aucBitmap,8);
			pucPtr += 8;
		}
	}else
	{
		if( (pucPtr-pucOutData)+ISO8583_BITMAPLEN > iOutLen)
		{
			return(ERR_ISO8583_OVERBUFLIMIT);
		}else
		{
			aucBitmap[0] |= 0x80;
			memcpy(pucPtr,aucBitmap,ISO8583_BITMAPLEN);
			pucPtr += ISO8583_BITMAPLEN;
		}
	}

	for(iI = 1;iI< ISO8583_MAXBITNUM; iI++)
	{
		if( ISO8583Data.Bit[iI].ucExistFlag == ISO8583_BITEXISTFLAG)
		{
			uiBitLen = ISO8583Data.Bit[iI].uiLen;
			uiOffset = ISO8583Data.Bit[iI].uiOffset;
			util_Printf("ISO8583:uiBitLen[%d]=%d\n",iI+1,uiBitLen);

			switch(pBitAttr[iI].ucAttr)
			{
			case ATTR_N:
				uiPackLen = (pBitAttr[iI].uiMaxLen+1)/2;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
#if 1
					asc_hex(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen);
					pucPtr += uiPackLen;

#else
					if( uiBitLen%2)
					{
						asc_hex(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen+1);
					}else
					{
						asc_hex(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen);
					}
					pucPtr += uiPackLen;

#endif
				}
				break;
			case ATTR_BIN:
				uiPackLen = ((pBitAttr+iI)->uiMaxLen+1)/8;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					asc_hex(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen);
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_AN:
			case ATTR_NS:
			case ATTR_ANS:
			case ATTR_XN:
				uiPackLen = (pBitAttr+iI)->uiMaxLen;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					memcpy(pucPtr,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen);
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_LLVARN:
			case ATTR_LLVARZ:
				uiPackLen = (uiBitLen+1)/2;
				if( (pucPtr-pucOutData+uiPackLen+1) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,1,&uiBitLen);
					pucPtr += 1;
					asc_bcd(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen*2);
					pucPtr += uiPackLen;
#if 1
					if( uiBitLen%2 )
					{
						*(pucPtr-1) &= 0xF0;
					}
#endif
				}
				break;
			case ATTR_LLLVARN:
			case ATTR_LLLVARZ:
				uiPackLen = (uiBitLen+1)/2;
/*
#ifdef TEST
				util_Printf("uiPackLen1=%d\n",uiPackLen);
#endif
*/
				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					util_Printf("error iI=%d\n",iI);
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,2,&uiBitLen);
					pucPtr += 2;
                    if ( uiPackLen*2 <= 256)
                    {
						asc_bcd(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen*2);
                    }else
                    {
                        asc_bcd(pucPtr,127,&ISO8583Data.aucBitBuf[uiOffset],254);
                        asc_bcd(&pucPtr[127],uiPackLen-127,&ISO8583Data.aucBitBuf[uiOffset+254],uiPackLen*2-254);
                    }
					pucPtr += uiPackLen;
#if 1
					if( uiBitLen%2 )
					{
						*(pucPtr-1) &= 0xF0;
					}
#endif
				}
				break;
			case ATTR_LLVARAN:
			case ATTR_LLVARNS:
			case ATTR_LLVARANS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+1) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,1,&uiBitLen);
					pucPtr += 1;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += uiPackLen;
					}
				}
				break;
			case ATTR_LLVARANZS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+1) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,1,&uiBitLen);
					pucPtr += 1;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += (uiPackLen+1)/2;
					}
				}
				break;
			case ATTR_LLLVARANS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,2,&uiBitLen);
					pucPtr += 2;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += uiPackLen;
					}
				}
				break;
			case ATTR_LLLVARANZS:
				uiPackLen = uiBitLen;
				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					short_bcd(pucPtr,2,&uiBitLen);
					pucPtr += 2;
					if( uiPackLen )
					{
						memcpy(pucPtr,&ISO8583Data.aucBitBuf[uiOffset],uiPackLen);
						pucPtr += (uiPackLen+1)/2;
					}
				}
				break;
			default:
				return(ERR_ISO8583_INVALIDBITATTR);
			}
		}
	}
	*puiOutDataLen = (pucPtr - pucOutData);
	return(SUCCESS);
}

unsigned char ISO8583_UnpackData(unsigned char *pucInData, short iInDataLen)
{
	short iI;
	unsigned char *pucPtr;
	unsigned char *pucBitBufPtr;
	unsigned char *pucBitBufStartPtr;
	unsigned char aucBitmap[ISO8583_BITMAPLEN];
	unsigned char ucMask;
	unsigned short uiPackLen;
	unsigned short uiBitLen;
	unsigned short uiBitNo;
	const ISO8583BitAttr *pBitAttr;


	pucPtr = pucInData;
	pucBitBufPtr = ISO8583Data.aucBitBuf;
	pucBitBufStartPtr = ISO8583Data.aucBitBuf;

	//pBitAttr = Default_ISO8583BitAttrTab;
	if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_UNTOUCH)
	{
		util_Printf("Untouch ISO\n");
		pBitAttr = ISO8583BitAttrTab_Untouch;
	}
	else if (G_NORMALTRANS_euCardSpecies == CARDSPECIES_MAG
		||G_NORMALTRANS_euCardSpecies == CARDSPECIES_EMV)
	{
		util_Printf("Magic & IC ISO\n");
		pBitAttr = Default_ISO8583BitAttrTab;
	}
	else
	{
		util_Printf("UnPack SAND Lt.cn\n");
		pBitAttr = SAND_ISO8583BitAttrTab;
	}
	memcpy(ISO8583Data.aucMsgID,pucPtr,2);
	pucPtr += 2;

	memset(aucBitmap,0,sizeof(aucBitmap));
	memcpy(aucBitmap,pucPtr,ISO8583_BITMAPLEN/2);
	pucPtr += ISO8583_BITMAPLEN/2;
	if( aucBitmap[0] & 0x80)
	{
		memcpy(&aucBitmap[ISO8583_BITMAPLEN/2],pucPtr,ISO8583_BITMAPLEN/2);
		pucPtr += ISO8583_BITMAPLEN/2;
		uiBitNo = ISO8583_MAXBITNUM;
	}else
	{
		uiBitNo = ISO8583_MAXBITNUM/2;
	}

	ucMask = 0x80;

	ISO8583_ClearBit();
	ISO8583_ClearBitBuf();

	ucMask = 0x40;
	for(iI=1;iI<uiBitNo;iI++)
	{
		if( aucBitmap[iI/8] & ucMask)
		{
			switch(pBitAttr[iI].ucAttr)
			{
			case ATTR_N:
				uiBitLen = pBitAttr[iI].uiMaxLen;
				uiPackLen = (uiBitLen+1)/2;
				if( uiBitLen%2 )
				{
					bcd_asc( pucBitBufPtr,pucPtr,uiBitLen+1);
				}else
				{
					bcd_asc( pucBitBufPtr,pucPtr,uiBitLen);
				}
				pucPtr += uiPackLen;
				break;
			case ATTR_BIN:
				uiBitLen = (pBitAttr[iI].uiMaxLen)/4;
				uiPackLen = (uiBitLen+1)/2;
				bcd_asc( pucBitBufPtr,pucPtr,uiBitLen);
				pucPtr += uiPackLen;
				break;
			case ATTR_AN:
			case ATTR_NS:
			case ATTR_XN:
			case ATTR_ANS:
				uiBitLen = pBitAttr[iI].uiMaxLen;
				uiPackLen = uiBitLen;
				memcpy( pucBitBufPtr,pucPtr,uiBitLen);
				pucPtr += uiPackLen;
				break;
			case ATTR_LLVARN:
			case ATTR_LLVARZ:
				uiBitLen = (unsigned short)bcd_long(pucPtr,2);
				uiPackLen = (uiBitLen+1)/2;
				pucPtr += 1;
				if( uiBitLen > pBitAttr[iI].uiMaxLen)
				{
					return(ERR_ISO8583_SETBITLEN);
				}else
				{
					if( uiBitLen%2)
					{
						bcd_asc( pucBitBufPtr,pucPtr,uiBitLen+1);
					}else
					{
						bcd_asc( pucBitBufPtr,pucPtr,uiBitLen);
					}
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_LLLVARN:
			case ATTR_LLLVARZ:
				uiBitLen = (unsigned short)bcd_long(pucPtr,4);
				uiPackLen = (uiBitLen+1)/2;
				pucPtr += 2;
				if( uiBitLen > pBitAttr[iI].uiMaxLen)
				{
					return(ERR_ISO8583_SETBITLEN);
				}else
				{
					if( uiBitLen%2)
					{
						bcd_asc( pucBitBufPtr,pucPtr,uiBitLen+1);
					}else
					{
						bcd_asc( pucBitBufPtr,pucPtr,uiBitLen);
					}
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_LLVARAN:
			case ATTR_LLVARNS:
			case ATTR_LLVARANS:
				uiBitLen = (unsigned short)bcd_long(pucPtr,2);
				uiPackLen = uiBitLen;
				pucPtr += 1;
				if( uiBitLen > pBitAttr[iI].uiMaxLen)
				{
					return(ERR_ISO8583_SETBITLEN);
				}else
				{
					memcpy(pucBitBufPtr,pucPtr,uiBitLen);
					pucPtr += uiPackLen;
				}
				break;
			case ATTR_LLLVARANS:
				uiBitLen = (unsigned short)bcd_long(pucPtr,4);
				uiPackLen = uiBitLen;
				pucPtr += 2;
				if( uiBitLen > pBitAttr[iI].uiMaxLen)
				{
					return(ERR_ISO8583_SETBITLEN);
				}else
				{
					memcpy(pucBitBufPtr,pucPtr,uiBitLen);
					pucPtr += uiPackLen;
				}
				break;
			default:
				return(ERR_ISO8583_INVALIDBITATTR);
			}
/*
#ifdef TEST
			util_Printf("ISO8583Data.Bit=%02x,%d\n",iI,uiBitLen);
#endif
*/
			ISO8583Data.Bit[iI].uiLen = uiBitLen;
			ISO8583Data.Bit[iI].uiOffset = (unsigned short)(pucBitBufPtr-pucBitBufStartPtr);
			pucBitBufPtr += uiBitLen;
			ISO8583Data.Bit[iI].ucExistFlag = ISO8583_BITEXISTFLAG;
		}
		ucMask = ucMask >> 1;
		if( !ucMask )
		{
			ucMask = 0x80;
		}
		if( (pucPtr - pucInData) > iInDataLen)
		{
			return(ERR_ISO8583_UNPACKDATA);
		}
	}
	return(SUCCESS);
}

void ISO8583_DumpData(void)
{
	unsigned char aucDumpData[24+1];
	unsigned int uiI,uiJ,uiK;
	unsigned short uiBitLen,uiOffset;
/*
	{
		unsigned short uiI;
		util_Printf("\nISO8583Data.uiCommBufLen %d",ISO8583Data.uiCommBufLen);
		for(uiI=0;uiI<ISO8583Data.uiCommBufLen;uiI++)
		{
			if(!(uiI%16)) util_Printf("\n");
			util_Printf("%02X ",ISO8583Data.aucCommBuf[uiI]);
		}
	}
	*/
	util_Printf("\n====DumpData Begin 2009-10-16 13:36====\n MsgId:%02X %02X",ISO8583Data.aucMsgID[0],ISO8583Data.aucMsgID[1]);
	for(uiI=1;uiI<ISO8583_MAXBITNUM;uiI++)
	{
		if( ISO8583Data.Bit[uiI].ucExistFlag == ISO8583_BITEXISTFLAG)
		{
			uiBitLen = ISO8583Data.Bit[uiI].uiLen;
			uiOffset = ISO8583Data.Bit[uiI].uiOffset;
			memset(aucDumpData,0,sizeof(aucDumpData));
			util_Printf("\n");
			memcpy(aucDumpData,"BITNO:",6);
			uiJ = uiI+1;
			int_asc(&aucDumpData[6],3,&uiJ);
			//PRINT_GB2312_xprint(aucDumpData,0x00);
			util_Printf((char*)aucDumpData);
			if( uiBitLen > 0)
			{
				for(uiK=0;uiK<uiBitLen;uiK++)
				{
					if(!(uiK%16)) util_Printf("\n");
					util_Printf("%c",ISO8583Data.aucBitBuf[uiOffset+uiK]);
				}
//				for(uiJ=0;uiJ<uiBitLen;uiJ+=PRN_MAXCHAR)
//				{
//					uiLen = min(PRN_MAXCHAR,uiBitLen-uiJ);
//					memset(aucDumpData,0,sizeof(aucDumpData));
//					memcpy(aucDumpData,&ISO8583Data.aucBitBuf[uiOffset+uiJ],uiLen);
//					//PRINT_GB2312_xprint(aucDumpData,0x00);
//					//util_Printf((char*)aucDumpData);
//					util_Printf("\n");
//				}
			}
		}
	}
	util_Printf("\n*****DumpData End*****");
}


/*
void ISO8583_CheckBitAttrTab(void)
{
code ISO8583BitAttr	Default_ISO8583BitAttrTab[ISO8583_MAXBITNUM] =
{
	{0,				0	},	//Bit 1
	{ATTR_LLVARN,	20	},	//Bit 2
	{ATTR_N,		6	},  //Bit 3
	{ATTR_N,		12	},  //Bit 4
	{ATTR_N,		12	},  //Bit 5
	{ATTR_N,		12	},  //Bit 6
	{ATTR_N,		10	},  //Bit 7
	{ATTR_N,		8	},  //Bit 8
	{ATTR_N,		8	},  //Bit 9
	{ATTR_N,		8	},  //Bit 10
	{ATTR_N,		6	},  //Bit 11
	{ATTR_N,		6	},  //Bit 12
	{ATTR_N,		4	},  //Bit 13
	{ATTR_N,		4	},  //Bit 14
	{ATTR_N,		4	},  //Bit 15
	{ATTR_N,		4	},  //Bit 16
	{ATTR_N,		4	},  //Bit 17
	{ATTR_N,		4	},  //Bit 18
	{ATTR_N,		3	},  //Bit 19
	{ATTR_N,		3	},  //Bit 20
	{ATTR_N,		3	},  //Bit 21
	{ATTR_N,		3	},  //Bit 22
	{ATTR_N,		3	},  //Bit 23
	{ATTR_N,		3	},  //Bit 24
	{ATTR_N,		2	},  //Bit 25
	{ATTR_N,		2	},  //Bit 26
	{ATTR_N,		1	},  //Bit 27
	{ATTR_XN,		9	},  //Bit 28
	{ATTR_XN,		9	},  //Bit 29
	{ATTR_XN,		9	},  //Bit 30
	{ATTR_XN,		9	},  //Bit 31
	{ATTR_LLVARN,	11	},  //Bit 32
	{ATTR_LLVARANS,	11	},  //Bit 33
	{ATTR_LLVARNS,	28	},  //Bit 34
	{ATTR_LLVARZ,	37	},  //Bit 35
	{ATTR_LLLVARZ,	104	},  //Bit 36
	{ATTR_AN,		12	},  //Bit 37
	{ATTR_AN,		6	},  //Bit 38
	{ATTR_AN,		2	},  //Bit 39
	{ATTR_AN,		3	},  //Bit 40
	{ATTR_ANS,		8	},  //Bit 41
	{ATTR_ANS,		15	},  //Bit 42
	{ATTR_ANS,		40	},  //Bit 43
	{ATTR_LLVARANS,	25	},  //Bit 44
	{ATTR_LLVARANS,	76	},  //Bit 45
	{ATTR_LLLVARANS,999	},  //Bit 46
	{ATTR_LLLVARANS,999	},  //Bit 47
	{ATTR_LLLVARN,999	},  //Bit 48
	{ATTR_ANS,		3	},  //Bit 49
	{ATTR_ANS,		3	},  //Bit 50
	{ATTR_ANS,		3	},  //Bit 51
	{ATTR_BIN,		64	},  //Bit 52
	{ATTR_N,		16	},  //Bit 53
	{ATTR_LLLVARANS,120	},  //Bit 54
	{ATTR_LLLVARANS,999	},  //Bit 55
	{ATTR_LLLVARANS,999	},  //Bit 56
	{ATTR_LLLVARANS,999	},  //Bit 57
	{ATTR_LLLVARANS,999	},  //Bit 58
	{ATTR_LLLVARANS,999	},  //Bit 59
	{ATTR_LLLVARANS,999	},  //Bit 60
//	{ATTR_LLLVARN,	999	},  //Bit 60
	{ATTR_LLLVARANS,999	},  //Bit 61
	{ATTR_LLLVARANS,999	},  //Bit 62
	{ATTR_LLLVARANS,999	},  //Bit 63
//	{ATTR_LLLVARN,	999	},  //Bit 63
	{ATTR_BIN,		64	},  //Bit 64
	{ATTR_BIN,		1	},  //Bit 65
	{ATTR_N,		1	},  //Bit 66
	{ATTR_N,		2	},  //Bit 67
	{ATTR_N,		3	},  //Bit 68
	{ATTR_N,		3	},  //Bit 69
	{ATTR_N,		3	},  //Bit 70
	{ATTR_N,		4	},  //Bit 71
	{ATTR_N,		4	},  //Bit 72
	{ATTR_N,		6	},  //Bit 73
	{ATTR_N,		10	},  //Bit 74
	{ATTR_N,		10	},  //Bit 75
	{ATTR_N,		10	},  //Bit 76
	{ATTR_N,		10	},  //Bit 77
	{ATTR_N,		10	},  //Bit 78
	{ATTR_N,		10	},  //Bit 79
	{ATTR_N,		10	},  //Bit 80
	{ATTR_N,		10	},  //Bit 81
	{ATTR_N,		12	},  //Bit 82
	{ATTR_N,		12	},  //Bit 83
	{ATTR_N,		12	},  //Bit 84
	{ATTR_N,		12	},  //Bit 85
	{ATTR_N,		16	},  //Bit 86
	{ATTR_N,		16	},  //Bit 87
	{ATTR_N,		16	},  //Bit 88
	{ATTR_N,		16	},  //Bit 89
	{ATTR_N,		42	},  //Bit 90
	{ATTR_AN,		1	},  //Bit 91
	{ATTR_AN,		2	},  //Bit 92
	{ATTR_AN,		5	},  //Bit 93
	{ATTR_AN,		7	},  //Bit 94
	{ATTR_AN,		42	},  //Bit 95
	{ATTR_BIN,		64	},  //Bit 96
	{ATTR_XN,		17	},  //Bit 97
	{ATTR_ANS,		25	},  //Bit 98
	{ATTR_LLVARN,	11	},  //Bit 99
	{ATTR_LLVARN,	11	},  //Bit 100
	{ATTR_LLVARANS,	17	},  //Bit 101
	{ATTR_LLVARANS,	28	},  //Bit 102
	{ATTR_LLVARANS,	28	},  //Bit 103
	{ATTR_LLVARANS,	100	},  //Bit 104
	{ATTR_LLLVARANS,999	},  //Bit 105
	{ATTR_LLLVARANS,999	},  //Bit 106
	{ATTR_LLLVARANS,999	},  //Bit 107
	{ATTR_LLLVARANS,999	},  //Bit 108
	{ATTR_LLLVARANS,999	},  //Bit 109
	{ATTR_LLLVARANS,999	},  //Bit 110
	{ATTR_LLLVARANS,999	},  //Bit 111
	{ATTR_LLLVARANS,999	},  //Bit 112
	{ATTR_LLLVARANS,999	},  //Bit 113
	{ATTR_LLLVARANS,999	},  //Bit 114
	{ATTR_LLLVARANS,999	},  //Bit 115
	{ATTR_LLLVARANS,999	},  //Bit 116
	{ATTR_LLLVARANS,999	},  //Bit 117
	{ATTR_LLLVARANS,999	},  //Bit 118
	{ATTR_LLLVARANS,999	},  //Bit 119
	{ATTR_LLLVARANS,999	},  //Bit 120
	{ATTR_LLLVARANS,999	},  //Bit 121
	{ATTR_LLLVARANS,999	},  //Bit 122
	{ATTR_LLLVARANS,999	},  //Bit 123
	{ATTR_LLLVARANS,999	},  //Bit 124
	{ATTR_LLLVARANS,999	},  //Bit 125
	{ATTR_LLLVARANS,999	},  //Bit 126
	{ATTR_LLLVARANS,999	},  //Bit 127
	{ATTR_BIN,		64	}   //Bit 128
};
	unsigned short uiChk;

	uiChk = ISO8583_ComputeBitAttrTabChecksum();
	if( uiChk != DataSave0.ISO8583BitAttrTab_Chk )
	{
		Os__saved_copy(	(unsigned char *)&Default_ISO8583BitAttrTab[0],
						(unsigned char *)&DataSave0.ISO8583BitAttrTab[0],
						sizeof(ISO8583BitAttr)*ISO8583_MAXBITNUM);
		uiChk = ISO8583_ComputeBitAttrTabChecksum();
		Os__saved_copy( (unsigned char *)&uiChk,
						(unsigned char *)&DataSave0.ISO8583BitAttrTab_Chk,
						sizeof(unsigned short));
	}
}
*/
unsigned char ISO8583_CheckResponseValid(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[50];
	short iLen;
	unsigned short uiMsgID ;

	util_Printf("\n 检查ISO8583_CheckResponseValid\n");
	util_Printf("交易类型:[%02x]\n",G_NORMALTRANS_ucTransType);
	/* Check response package is response of sent package */
	/* Notice: bit 11 maybe different !!!*/
	/* Must add more process */
	/* 有部分交易无流水号，无法检查*/
#if 1
	if( (G_NORMALTRANS_ucTransType != TRANS_ONLINETEST)
	  &&(G_NORMALTRANS_ucTransType != TRANS_SENDSTATS)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOADPARAM)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOGONOFF)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOGONON)
	  &&(G_NORMALTRANS_ucTransType != TRANS_SETTLE)
	  &&(G_NORMALTRANS_ucTransType != TRANS_OPERATOR)
	  &&(G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	  &&(G_RUNDATA_ucTransType != TRANS_SETTLE)
	  )
	  {
			ucResult = ISO8583_CompareSentBit(3);
			util_Printf("Compare3 Filed [%02x]\n",ucResult);
			if (ucResult != SUCCESS)
			{
				G_RUNDATA_ucFiledErrNo = 3;
				return ucResult;
			}
	  }
#endif
	if( (G_NORMALTRANS_ucTransType != TRANS_ONLINETEST)
	  &&(G_NORMALTRANS_ucTransType != TRANS_SENDSTATS)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOADPARAM)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOGONOFF)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOGONON)
	  &&(G_NORMALTRANS_ucTransType != TRANS_SETTLE)
	  &&(G_NORMALTRANS_ucTransType != TRANS_QUERYBAL)
	  &&(G_NORMALTRANS_ucTransType != TRANS_OPERATOR)
	  &&(G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	  &&(G_RUNDATA_ucTransType != TRANS_SETTLE)
	  )
	{
		ISO8583_GetMsgID(&uiMsgID);
		if(uiMsgID!=630)
		{
			ucResult = ISO8583_CompareSentBit(4);
			util_Printf("Compare 4 Filed [%02x]\n",ucResult);
			if( ucResult != SUCCESS )
			{
				G_RUNDATA_ucFiledErrNo = 41;
				return(ucResult);
			}
		}
	}

	if( (G_NORMALTRANS_ucTransType != TRANS_ONLINETEST)
	  &&(G_NORMALTRANS_ucTransType != TRANS_SENDSTATS)
	  &&(G_NORMALTRANS_ucTransType != TRANS_LOADPARAM)
	  &&(G_NORMALTRANS_ucTransType != TRANS_OPERATOR)
	  &&(G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	  )
	 {
		ucResult = ISO8583_CompareSentBit(11);
		util_Printf("Compare11 Filed [%02x]\n",ucResult);
		if( ucResult != SUCCESS )
		{
			G_RUNDATA_ucFiledErrNo = 11;
			return(ucResult);
		}
	}
	if (G_NORMALTRANS_ucTransType != TRANS_OPERATOR
		&& G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	{
		ucResult = ISO8583_GetBitValue(32,aucBuf,&iLen,sizeof(aucBuf));
		util_Printf("Compare32 Filed [%02x]\n",ucResult);
		if( ucResult == SUCCESS )
		{
			util_Printf("\nISO8583_CompareSentBit--32  iLen=%d\n",iLen);
			if( iLen!=8 )
			{
				G_RUNDATA_ucFiledErrNo = 32;
				return(ERR_FORMAT);
			}
			memcpy(G_NORMALTRANS_aucBankID,aucBuf,iLen);
		}
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(37,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult == SUCCESS &&G_NORMALTRANS_ucTransType!=TRANS_REVERSAL)
		{
			util_Printf("\naucBuf=1110202=%s",aucBuf);
			memcpy(G_NORMALTRANS_aucRefNum,aucBuf,TRANS_REFNUMLEN);
		}
	}

	ucResult = ISO8583_CompareSentBit(41);
	util_Printf("Compare41 Filed [%02x]\n",ucResult);
	if( ucResult != SUCCESS )
	{
		G_RUNDATA_ucFiledErrNo = 41;
		return(ucResult);
	}
	if (G_NORMALTRANS_ucTransType != TRANS_OPERATOR
		&& G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	{
		ucResult = ISO8583_CompareSentBit(42);
		util_Printf("Compare42 Filed [%02x]\n",ucResult);
		if( ucResult != SUCCESS )
		{
			G_RUNDATA_ucFiledErrNo = 42;
			return(ucResult);
		}
	}

	/* Check response code */
	memset(aucBuf,0,sizeof(aucBuf));
	if (G_NORMALTRANS_ucTransType == TRANS_OPERATOR
		||G_NORMALTRANS_ucTransType == TRANS_UPTMKEY)
	{
   		ucResult = ISO8583_GetBitValue(39,aucBuf,&iLen,sizeof(aucBuf));
		util_Printf("Compare39 Filed [%02x]\n",ucResult);
		if( ucResult != SUCCESS )
		{
			util_Printf("\n远程密钥--39\n");
			G_RUNDATA_ucFiledErrNo = 39;
			return(ucResult);
		}
		if (memcmp(aucBuf,"0000",4))
		{
			return(ERR_HOSTCODE);
		}
	}

	memset(aucBuf,0,sizeof(aucBuf));
	if(G_NORMALTRANS_ucTransType != TRANS_SETTLE
		&& G_NORMALTRANS_ucTransType != TRANS_OPERATOR
		&& G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	{
   		ucResult = ISO8583_GetBitValue(39,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult != SUCCESS )
		{
			util_Printf("\nISO8583_CompareSentBit--39\n");
			G_RUNDATA_ucFiledErrNo = 39;
			return(ucResult);
		}

		if( (memcmp(aucBuf,"A0",2)==0)||(memcmp(aucBuf,"a0",2)==0) )
			G_RUNDATA_ucErrorExtCode=100;
		else
			G_RUNDATA_ucErrorExtCode = (unsigned short)asc_long(aucBuf,2);

		if(G_RUNDATA_ucErrorExtCode == 94)
			UTIL_AddTraceNumber();

		util_Printf("\nHostRespCode=%02x %02x \n",aucBuf[0],aucBuf[1]);
		util_Printf("G_RUNDATA_ucErrorExtCode=%d\n",G_RUNDATA_ucErrorExtCode);

		if(memcmp(aucBuf,"77",2)==0)
		{
			Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.ucLogonModiBatch,
					1,sizeof(unsigned char));
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
		if ( memcmp(aucBuf,"00",2))
		{
			util_Printf("ISO8583_CompareSentBit39notOK\n");
			return(ERR_HOSTCODE);
		}
   }

	if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV
	   &&G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_INSERTCARD
	   &&(G_NORMALTRANS_ucTransType==TRANS_PURCHASE
	   ||G_NORMALTRANS_ucTransType==TRANS_PREAUTH
	   ||G_NORMALTRANS_ucTransType==TRANS_QUERYBAL)
	   &&(G_RUNDATA_ucTransType != TRANS_SETTLE)
	   )
	{
//bo		ucResult=EMV_CheckICRelateData();
		if( ucResult != SUCCESS )
		{
			util_Printf("ISO8583_CompareSentBit55\n");
			G_RUNDATA_ucFiledErrNo = 55;
			return(ucResult);
		}
	}
	return(SUCCESS);
}


unsigned char ISO8583_CompareSentBit(short iBitNo)
{
	unsigned char aucBuf[20];
	unsigned short uiI,uiJ;

	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}

	if( ISO8583Data.Bit[iBitNo].ucExistFlag != ISO8583_BITEXISTFLAG)
	{
		util_Printf("ERR_ISO8583_BITNOTEXIST2,iBitNo=%02x\n",iBitNo);
		return(ERR_ISO8583_BITNOTEXIST);
	}
	if( DataSave0.Trans_8583Data.SendISO8583Data.Bit[iBitNo].ucExistFlag != ISO8583_BITEXISTFLAG)
	{
		util_Printf("ERR_ISO8583_BITNOTEXIST3\n");
		return(ERR_ISO8583_BITNOTEXIST);
	}

	uiI = ISO8583Data.Bit[iBitNo].uiLen;
	uiJ = DataSave0.Trans_8583Data.SendISO8583Data.Bit[iBitNo].uiLen;
	if( uiI != uiJ)
	{
		return(ERR_ISO8583_COMPARE);
	}
	uiJ = ISO8583Data.Bit[iBitNo].uiOffset;
	if( uiI <= sizeof(aucBuf))
	{
		memcpy(aucBuf,&ISO8583Data.aucBitBuf[uiJ],uiI);
	}else
	{
		return(ERR_ISO8583_OVERBUFLIMIT);
	}
	uiJ = DataSave0.Trans_8583Data.SendISO8583Data.Bit[iBitNo].uiOffset;
	if( memcmp(aucBuf,&(DataSave0.Trans_8583Data.SendISO8583Data.aucBitBuf[uiJ]),uiI) )
	{
		return(ERR_ISO8583_COMPARE);
	}
	return(SUCCESS);
}


void ISO8583_SaveISO8583Data(unsigned char *pucSrc,unsigned char *pucDest)
{
	Os__saved_copy((unsigned char *)pucSrc,(unsigned char *)pucDest,
					sizeof(BACKUPISO8583));
}

void ISO8583_RestoreISO8583Data(unsigned char *pucSrc,unsigned char *pucDest)
{
	memcpy((unsigned char *)pucSrc,(unsigned char *)pucDest,
					sizeof(BACKUPISO8583));
}

unsigned char ISO8583_SaveReversalISO8583Data(void)
{
	unsigned char ucResult;
	ucResult=SUCCESS;

	Os__saved_set((unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid),
					0,sizeof(unsigned char));

	/* Copy Data from Global ISO8583Data */
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
					(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
#if 0 //通讯成功后置冲正标志
	Os__saved_set((unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid),
					DATASAVEVALIDFLAG,sizeof(unsigned char));
#endif
	ucResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
	return(ucResult);
}









