/* 

--------------------------------------------------------------------------
	FILE  ISO8583.c								
--------------------------------------------------------------------------
    INTRODUCTION          
    ============                                             
    Created :		2004-11-30		
    Last modified :	2004-11-30		
    Module :  Shanghai Smart Card Network             
    Purpose :                               
        Source file for ISO8583 package process. 
                                                             
    List of routines in file :                                            
                                                                          
    File history :                                                        
                                                                          
*/

#include <include.h>
#include <global.h>
#include <iso8583.h>
#include <xdata.h>
#include <toolslib.h>

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
/**/
#ifdef TEST
		Uart_Printf("iBitNo=%02x\n",iBitNo);
		Uart_Printf("ISO8583Data.Bit[iBitNo].uiLen=%3d\n",iI);
		Uart_Printf("iOutBufLen=%3d\n",iOutBufLen);
		Uart_Printf("-----------------------------------------------------\n");
#endif

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
		Uart_Printf("ERR_ISO8583_BITNOTEXIST\n");
#endif
*/
		return(ERR_ISO8583_BITNOTEXIST);
	}
}

unsigned char ISO8583_SetBitValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen)
{
	const ISO8583BitAttr *pBitAttr;

//	pBitAttr = DataSave0.ISO8583BitAttrTab;
	pBitAttr = Default_ISO8583BitAttrTab;
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
	Uart_Printf("iBitNo=%02x\n",iBitNo);
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

//	pBitAttr = DataSave0.ISO8583BitAttrTab;
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
#ifdef TEST
	unsigned short usI;
//	unsigned char  ucYuBit,displayBuf[512];
	Uart_Printf("\n Pack 8583 data !!!\n");
#endif		

	memset(aucBitmap,0,sizeof(aucBitmap));
	memset(aucLen,0,sizeof(aucLen));

//	pBitAttr = DataSave0.ISO8583BitAttrTab;
	pBitAttr = Default_ISO8583BitAttrTab;
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
#ifdef TEST
		Uart_Printf("aucBitmap:\n");
		for(usI=0;usI<16;usI++)
			Uart_Printf("%02x,",aucBitmap[usI]);
		Uart_Printf("\n");
		Uart_Printf("MsgId:\n");
		Uart_Printf("%02x%02x\n",ISO8583Data.aucMsgID[0],ISO8583Data.aucMsgID[1]);
#endif

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
#if 0
#ifdef TEST
			ucYuBit = pBitAttr[iI].ucAttr;
			Uart_Printf("field%d :attribute=%02x bitLen=%d\n",iI+1,ucYuBit,uiBitLen);
			memset(displayBuf,0x00,sizeof(displayBuf));
			if(
				(ISO8583Data.aucBitBuf[uiOffset]>='0')&&(ISO8583Data.aucBitBuf[uiOffset]<='9')
			  &&(ISO8583Data.aucBitBuf[uiOffset+1]>='0')&&(ISO8583Data.aucBitBuf[uiOffset+1]<='9')
			  )
			  {
				memcpy(displayBuf,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen);
			  }else
			  {
				hex_str(displayBuf,&ISO8583Data.aucBitBuf[uiOffset],uiBitLen*2);
			  }
			if(strlen((char *)displayBuf)<300)
				Uart_Printf("%s\n",displayBuf);
#endif
#endif	
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
					if( uiBitLen%2 )
					{
						*(pucPtr-1) &= 0xF0;
					}
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
#if 0
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
				Uart_Printf("uiPackLen1=%d\n",uiPackLen);
#endif
*/
				if( (pucPtr-pucOutData+uiPackLen+2) > iOutLen )
				{
					Uart_Printf("error iI=%d\n",iI);
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
#if 0
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
			case ATTR_LLLVARANS:
				uiPackLen = uiBitLen;
/*
#ifdef TEST
				Uart_Printf("uiPackLen2=%d\n",uiPackLen);
#endif
*/
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
			default:
//            	UTIL_PrintHexMsg(&iI,2);
//              UTIL_PrintHexMsg(&pBitAttr[iI],1);
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
//	unsigned char aucBuff[500];

Uart_Printf("\n FUNCTION : ISO8583_UnpackData \n");

	pucPtr = pucInData;
	pucBitBufPtr = ISO8583Data.aucBitBuf;
	pucBitBufStartPtr = ISO8583Data.aucBitBuf;

//	pBitAttr = DataSave0.ISO8583BitAttrTab;
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
/*
#ifdef TEST
{				
				unsigned short i;
				Uart_Printf("uiBitLen=%d\n",uiBitLen);
				for(i=0;i<uiBitLen;i++)
					Uart_Printf("%02x ",*(pucPtr+i));	
				Uart_Printf("\n");			
}
#endif
*/
					pucPtr += uiPackLen;
				}
				
				break;
			default:
				return(ERR_ISO8583_INVALIDBITATTR);
			}
/*
#ifdef TEST
			Uart_Printf("ISO8583Data.Bit=%02x,%d\n",iI,uiBitLen);
#endif
*/
			ISO8583Data.Bit[iI].uiLen = uiBitLen;
			ISO8583Data.Bit[iI].uiOffset = (unsigned short)(pucBitBufPtr-pucBitBufStartPtr);
			pucBitBufPtr += uiBitLen;
			ISO8583Data.Bit[iI].ucExistFlag = ISO8583_BITEXISTFLAG;
#ifdef TEST
			{
			unsigned char aucBuff[500];
			/*Print the data */
			Uart_Printf("\nBitNo:%d",iI+1);
			memset(aucBuff,0x00,sizeof(aucBuff));
			memcpy(aucBuff,pucBitBufPtr-uiBitLen,uiBitLen);
			Uart_Printf("\n %s\n",aucBuff);
			}
#endif
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
	unsigned char aucDumpData[PRN_MAXCHAR+1];
	unsigned int uiI,uiJ,uiLen;
	unsigned short uiBitLen,uiOffset;
	Uart_Printf("enter ISO8583_DumpData\n");
	for(uiI=1;uiI<ISO8583_MAXBITNUM;uiI++)
	{
		if( ISO8583Data.Bit[uiI].ucExistFlag == ISO8583_BITEXISTFLAG)
		{
			uiBitLen = ISO8583Data.Bit[uiI].uiLen;
			uiOffset = ISO8583Data.Bit[uiI].uiOffset;
			memset(aucDumpData,0,sizeof(aucDumpData));
			memcpy(aucDumpData,"BITNO:",6);
			uiJ = uiI+1;
			int_asc(&aucDumpData[6],3,&uiJ);
			//Os__xprint(aucDumpData);
			Uart_Printf("%s\n",aucDumpData);
			if( uiBitLen > 0)
			{
				for(uiJ=0;uiJ<uiBitLen;uiJ+=PRN_MAXCHAR)
				{
					uiLen = min(PRN_MAXCHAR,uiBitLen-uiJ);
					memset(aucDumpData,0,sizeof(aucDumpData));
					memcpy(aucDumpData,&ISO8583Data.aucBitBuf[uiOffset+uiJ],uiLen);
					//Os__xprint(aucDumpData);
					Uart_Printf("%s\n",aucDumpData);
				}
			}
		}
	}
}



unsigned char ISO8583_CheckResponseValid(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[100];
	unsigned char aucDate[7];
	unsigned char aucTime[5];
	short iLen;
	
	/* Check response package is response of sent package */
	/* Notice: bit 11 maybe different !!!*/
	/* Must add more process */
Uart_Printf("\n FUNCTION:ISO8583_CheckResponseValid \n ");
#ifndef TURBOC	
	ucResult = ISO8583_CompareSentBit(11);
	if( ucResult != SUCCESS )
	{

#ifdef TEST
		Uart_Printf("ISO8583_CompareSentBit11\n");
#endif

		return(ucResult);
	}
#endif	
	ucResult = ISO8583_CompareSentBit(41);
	if( ucResult != SUCCESS )
	{

#ifdef TEST
		Uart_Printf("ISO8583_CompareSentBit41\n");
#endif

		return(ucResult);
	}
	/* Check response code */
	memset(aucBuf,0,sizeof(aucBuf));

	ucResult = ISO8583_GetBitValue(39,aucBuf,&iLen,sizeof(aucBuf));
	if( ucResult != SUCCESS )
	{
		return(ucResult);
	}
	RUNDATA_ucErrorExtCode = (unsigned short)asc_long(aucBuf,4);
	if(RUNDATA_ucErrorExtCode == 58)
	{
		NORMALTRANS_ulTraceNumber += 50;
		UTIL_IncreaseTraceNumber(1);
	}	
#ifdef TEST
	Uart_Printf("HostRespCode=%s\n",aucBuf);
#endif		
	
	if ( memcmp(aucBuf,"0000",4))
	{
		if(ISO8583_GetBitValue(55,aucBuf,&iLen,sizeof(aucBuf))==SUCCESS)
		{
			memcpy(RunData.aucErrorExplain,aucBuf,16);
		}
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
/*
#ifdef TEST
		Uart_Printf("ERR_ISO8583_BITNOTEXIST2,iBitNo=%02x\n",iBitNo);
#endif
*/
		return(ERR_ISO8583_BITNOTEXIST);
	}
	if( DataSave0.SendISO8583Data.Bit[iBitNo].ucExistFlag != ISO8583_BITEXISTFLAG)
	{
/*
#ifdef TEST
		Uart_Printf("ERR_ISO8583_BITNOTEXIST3\n");
#endif
*/
		return(ERR_ISO8583_BITNOTEXIST);
	}
	
	uiI = ISO8583Data.Bit[iBitNo].uiLen;
	uiJ = DataSave0.SendISO8583Data.Bit[iBitNo].uiLen;
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
	uiJ = DataSave0.SendISO8583Data.Bit[iBitNo].uiOffset;
	if( memcmp(aucBuf,&(DataSave0.SendISO8583Data.aucBitBuf[uiJ]),uiI) )
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
	Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
					0,sizeof(unsigned char));
	/* Copy Data from Global ISO8583Data */
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
					(unsigned char *)&DataSave0.ReversalISO8583Data.BackupISO8583Data);	
	Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
					DATASAVEVALIDFLAG,
					sizeof(unsigned char));
	ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	return ucResult;
}

unsigned short ISO8583_ComputeReversalISO8583DataChk()
{
	return(checksum((unsigned char *)&DataSave0.ReversalISO8583Data,
					(unsigned char *)&DataSave0.ReversalISO8583Data.BackupISO8583Data.uiBitBufLen,
					CHKINIT));
}
 