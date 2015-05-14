#include <include.h>
#include <Global.h>
#include <iso8583.h>
#include <xdata.h>


void ISO8583_Clear(void)
{
	unsigned short  uiI;
	
	for(uiI=0;uiI<ISO8583_MAXBITNUM;uiI++)
	{
		ISO8583Data.Bit[uiI].ucExistFlag = ISO8583_BITNOTEXISTFLAG;
		ISO8583Data.Bit[uiI].uiOffset = 0;
		ISO8583Data.Bit[uiI].uiLen = 0;
	}
	ISO8583Data.uiBitBufLen = 0;
	memset(ISO8583Data.aucBitBuf,0,ISO8583_MAXBITBUFLEN);
	ISO8583Data.uiCommBufLen = 0;
	memset(ISO8583Data.aucCommBuf,0,ISO8583_MAXCOMMBUFLEN);
}

unsigned char ISO8583_SetMsgID(unsigned short iMsgID)
{
	short_bcd(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN,&iMsgID);
	return(SUCCESS);
}

unsigned char ISO8583_SetBitValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen)
{

	const ISO8583BitAttr *pBitAttr;

	pBitAttr = Default_ISO8583BitAttrTab;

	iBitNo --;
	if(  (iBitNo > ISO8583_MAXBITNUM)
	   ||(iBitNo < 0)
	  )
	{
		return(ERR_ISO8583_INVALIDBIT);
	}

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
		return(ERR_ISO8583_BITNOTEXIST);
	}
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

	pBitAttr = Default_ISO8583BitAttrTab;
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

			switch(pBitAttr[iI].ucAttr)
			{
			case ATTR_N:
				uiPackLen = (pBitAttr[iI].uiMaxLen+1)/2;
				if( (pucPtr-pucOutData+uiPackLen) > iOutLen )
				{
					return(ERR_ISO8583_OVERBUFLIMIT);
				}else
				{
					asc_hex(pucPtr,uiPackLen,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen);
					pucPtr += uiPackLen;
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
					if( uiBitLen%2 )
					{
						*(pucPtr-1) &= 0xF0;
					}
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

	pBitAttr = Default_ISO8583BitAttrTab;
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





unsigned char ISO8583_CheckResponseValid(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[50];
	short iLen;
	unsigned short uiMsgID ;

	

	  if(G_NORMALTRANS_ucTransType != TRANS_LOGONON)
	  {
			ucResult = ISO8583_CompareSentBit(3);
			if (ucResult != SUCCESS)
			{
				return ucResult;
			}
	  }
	if(G_NORMALTRANS_ucTransType != TRANS_LOGONON)
	{
		ISO8583_GetMsgID(&uiMsgID);
		if(uiMsgID!=630)
		{
			ucResult = ISO8583_CompareSentBit(4);
			if( ucResult != SUCCESS )
			{
				return(ucResult);
			}
		}
	}

	if(G_NORMALTRANS_ucTransType != TRANS_LOGONON)
	 {
		ucResult = ISO8583_CompareSentBit(11);
		if( ucResult != SUCCESS )
		{
			return(ucResult);
		}
	}
	
	

	ucResult = ISO8583_CompareSentBit(41);
	if( ucResult != SUCCESS )
	{
		return(ucResult);
	}
	
	ucResult = ISO8583_CompareSentBit(42);
	if( ucResult != SUCCESS )
	{
		return(ucResult);
	}

	ucResult = ISO8583_GetBitValue(39,aucBuf,&iLen,sizeof(aucBuf));
	if( ucResult != SUCCESS )
	{
		return(ucResult);
	}
	if( (memcmp(aucBuf,"A0",2)==0)||(memcmp(aucBuf,"a0",2)==0) )
		G_RUNDATA_ucErrorExtCode=100;
	else
		G_RUNDATA_ucErrorExtCode = (unsigned short)asc_long(aucBuf,2);
       if ( memcmp(aucBuf,"00",2))
	{
		Uart_Printf("ISO8583_CompareSentBit39notOK\n");
		return(ERR_HOSTCODE);
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
		return(ERR_ISO8583_BITNOTEXIST);
	}
	if( DataSave0.Trans_8583Data.SendISO8583Data.Bit[iBitNo].ucExistFlag != ISO8583_BITEXISTFLAG)
	{
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


unsigned char ISO8583_GetMsgID(unsigned short *piMsgID)
{
	short iMsgID;

	iMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
	*piMsgID = iMsgID;
	return(SUCCESS);
}

void ISO8583_SaveISO8583Data(unsigned char *pucSrc,unsigned char *pucDest)
{
	Os__saved_copy((unsigned char *)pucSrc,(unsigned char *)pucDest,
					sizeof(BACKUPISO8583));
}

unsigned char ISO8583_SetBitHexValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen)
{
	const ISO8583BitAttr *pBitAttr;

	pBitAttr = Default_ISO8583BitAttrTab;
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


