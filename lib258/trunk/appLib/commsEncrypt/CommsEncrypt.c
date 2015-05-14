#include <CommsEncrypt.h>

#define uchar unsigned char
#define ulong unsigned long
#define ushort unsigned short


extern uchar * asc_hex (uchar *ptDest, uchar lgDest, uchar *ptSrc, uchar lgSrc);
extern ulong bcd_long (uchar *ptSrc, uchar lgSrc);
extern uchar * asc_bcd (uchar *ptDest, uchar lgDest, uchar *ptSrc, uchar lgSrc);
extern ulong asc_long (uchar *ptSrc, uchar lgSrc);
extern uchar * short_bcd(uchar *Ptd ,uchar Ld,ushort  *Pts);
extern void  Os__read_time_sec (uchar *ptTime);
extern uchar Os__GB2312_display (uchar Line, uchar Column, uchar * Text);

extern void des(uchar *binput, uchar *boutput, uchar *bkey);
extern void desm1(uchar *binput, uchar *boutput, uchar *bkey);

/*主密钥索引KEYINDEX_NETCOMMS_MASTERKEY设定为0-3，把KEYINDEX_NETCOMMS_MASTERKEY*2后，
  取KEYINDEX_NETCOMMS_MASTERKEY*2位置密钥加密，KEYINDEX_NETCOMMS_MASTERKEY*2+0x01位置密钥解密*/
unsigned char  KEYINDEX_NETCOMMS_ARRAY;
unsigned short KEYINDEX_NETCOMMS_MASTERKEY;      /*主密钥索引*/


const unsigned char EncrptKey[3][NETCOMMS_KEYNUM*2][9]=
{
	{//1010
		"\x91\x40\xad\xb6\xb9\x4b\x7a\xc8",
		"\xdb\xb2\x0d\x11\xef\x23\x3a\xa2",
		"\x04\x28\xf1\xea\x5d\x48\x27\xd5",
		"\xa6\x3c\x04\x67\x78\x4e\xdc\x00",
		"\x56\x27\x01\xd7\x34\xbf\x97\x77",
		"\x8e\x93\x76\xf6\x50\x4c\x16\xf2",
		"\xe3\xdc\xbd\x9b\xb2\xd8\x20\xba",
		"\x5d\xae\xef\xcb\x92\xf5\xa4\x6d",
		"\xdf\x95\x70\x19\x94\x9f\x73\x62",
		"\x1c\x91\xd6\x47\xe5\xc4\xec\x5e",
		"\xe6\xd5\xbc\xb9\xf3\xd8\xfa\xb4",
		"\x34\x17\xc8\x87\x5a\xb5\xad\x21",
		"\x11\xbe\x7c\xee\xd9\x39\xf3\x59",
		"\xd8\x20\xba\x5d\xae\xef\x5a\xb5",
		"\xf3\xd8\xfa\x01\xd7\x34\xef\xcb",
		"\x92\x62\x1c\x91\x94\x9f\x73\x6a",
		"\x12\x0a\x04\xb9\xe3\xfb\x5a\x2f",
		"\x88\x57\x08\x33\x48\x0b\x28\xee",
		"\x07\xd4\x2e\x39\x25\x44\x71\x74",
		"\x47\x4f\x7a\x4d\xbc\xf7\xbb\x6c",

	},  
	{//1020
		"\xe6\xd5\xbc\xb3\xf3\xd8\xfa\xb4",
		"\x36\x17\xc8\x8e\x5a\xb5\xad\x21",
		"\x56\x27\x01\xd7\x34\xbf\x97\x47",
		"\x8e\x93\x76\xf6\x50\x4c\x16\xf2",
		"\xe3\xdc\xbd\x9b\xb2\xd8\x20\xba",
		"\x5d\xae\xef\xcb\x92\xf5\xa4\xfd",
		"\x11\xbe\x7c\xee\xd9\x69\xf3\x59",
		"\xd8\x20\xba\x5d\xae\xef\x5a\xb5",
		"\x91\x4f\xa7\xb6\xb9\x4b\x7a\xc8",
		"\x9b\xb2\x0d\x11\xef\x23\x3a\xa2",
		"\x04\x28\xf1\xea\x5d\x45\x27\xd5",
		"\xa6\x0c\x04\x67\x78\x4e\xdc\x00",
		"\xf3\xd8\xfa\x01\xd7\x3c\xef\xcb",
		"\x92\x6d\x1c\x91\x94\x9f\x73\x6a",
		"\xdf\x95\xf0\x19\x94\x9f\x73\x62",
		"\x1c\x91\xd6\x47\xea\xc4\xec\x5e",
		"\x12\x0a\x04\xb9\xe3\x5b\x5a\x2f",
		"\x80\x57\x08\x3a\x48\x0b\x28\xbe",
		"\x0a\xd4\x2e\x39\x25\x44\x71\x74",
		"\xf7\x4f\x7a\x4d\xbd\xf7\xbb\x6c",
	},
	{//1030
		"\xa3\x39\x29\x4b\x72\xc6\x82\x40",
		"\x21\x74\x71\xa1\x2b\x59\x30\xa1",
		"\x98\xb7\x28\x37\x12\x07\x48\x9a",
		"\x78\x36\x15\x61\x11\x32\xb8\x39",
		"\x91\x76\xc2\x58\x10\xa6\x12\x20",
		"\x16\xc5\x24\x86\x36\xc2\x15\x79",
		"\x85\x26\x27\xd3\x2b\x62\x63\x28",
		"\x73\x59\x40\x94\xdb\x40\x58\x40",
		"\x14\x00\x39\x66\xe9\xa1\x33\x81",
		"\xf5\x62\x00\xe8\x99\x28\x28\x24",
		"\xa6\x8c\x46\x86\x12\xf1\x47\xa3",
		"\xe0\x80\xf4\x28\x50\x17\x00\xa8",
		"\x13\x14\x70\x05\x12\x48\x95\x63",
		"\xd5\x87\x57\x39\x16\x4c\x45\x77",
		"\x13\x28\x28\x08\x1b\x09\x97\x04",
		"\xc1\x07\xa3\x31\x2b\x39\xa0\x11",
		"\x49\x59\x31\x07\x10\x7c\x1b\x17",
		"\xb8\x59\x7b\x44\x13\x43\x44\x53",
		"\x1c\x11\x06\x4b\x60\x77\x02\x90",
		"\xa6\x23\x46\x88\xa9\x56\xa9\x54",
	}

};
unsigned char   NETCOMMS_VERSION[5];
unsigned char	WorkKey[9]; 
unsigned char 	EncryptData[NETWORK_MAXDATALEN+1];
unsigned char 	DecryptData[NETWORK_MAXDATALEN+1];

unsigned char COMMSENCRYPT_SetVersion(unsigned char *pucInData)
{
	unsigned short usVersion;
	
	KEYINDEX_NETCOMMS_ARRAY = 0;
	
	usVersion = (unsigned short)asc_long(pucInData,4);
	switch(usVersion)
	{
	case 1010:
		KEYINDEX_NETCOMMS_ARRAY = 0;
		memcpy(NETCOMMS_VERSION,pucInData,4);
		break;
	case 1020:
		KEYINDEX_NETCOMMS_ARRAY = 1;
		memcpy(NETCOMMS_VERSION,pucInData,4);
		break;
	case 1030:
		KEYINDEX_NETCOMMS_ARRAY = 2;
		memcpy(NETCOMMS_VERSION,pucInData,4);
		break;
	default:
		return ERR_NETCOMMS_VERSION;
	}
	return NETCOMMS_SUCCESS;
}	

/*产生工作密钥*/
unsigned char GetWorkKey(unsigned char aucRanNum[8])
{
    unsigned char aucTempDesData[8];
    unsigned char ucResult = NETCOMMS_SUCCESS;
  
    //Uart_Printf("\n");   
    //Uart_Printf("KEYINDEX_NETCOMMS_MASTERKEY*2=%02x\n",KEYINDEX_NETCOMMS_MASTERKEY*2);
    
    if( KEYINDEX_NETCOMMS_ARRAY > 2 )
	{
#ifdef NET_DEBUG
		Uart_Printf("KEYINDEX_NETCOMMS_ARRAY = %d\n",KEYINDEX_NETCOMMS_ARRAY);
#endif		
		return ERR_NETCOMMS_INDEXERROR;
	}
    
    /*取特定密钥对8字节aucRanNum作DES加密，结果存于aucTempDesData中*/
    des(aucRanNum,aucTempDesData,
    	(unsigned char*)&EncrptKey[KEYINDEX_NETCOMMS_ARRAY][KEYINDEX_NETCOMMS_MASTERKEY*2][0]);
    
	/*取特定密钥对8字节aucTempDesData作DES解密，结果存于aucTempDesData中*/
	desm1(aucTempDesData,aucTempDesData,
		(unsigned char*)&EncrptKey[KEYINDEX_NETCOMMS_ARRAY][KEYINDEX_NETCOMMS_MASTERKEY*2+0x01][0]);
	
	/*取特定密钥对8字节aucTempDesData作DES加密，结果存于aucTempDesData中*/
	des(aucTempDesData,WorkKey,
		(unsigned char*)&EncrptKey[KEYINDEX_NETCOMMS_ARRAY][KEYINDEX_NETCOMMS_MASTERKEY*2][0]);
                               	
    return(ucResult);
}
					                                         
/*计算MAC*/
unsigned char CalcNetworkMAC(unsigned char *pucInData,
				             unsigned short uiInLen,
				             unsigned char *pucOutMAC)
{
	unsigned char aucHexData[8];
	unsigned char *pucPtr;
	unsigned short uiI,uiJ;

	pucPtr = pucInData;
#ifdef NET_DEBUG
	Uart_Printf("-----------------------------------------\n");
#endif	
	memset(aucHexData,0,sizeof(aucHexData));
	for (uiI=0;uiI<uiInLen;uiI += 8)
	{
#ifdef NET_DEBUG			
		Uart_Printf("Line:%02d: ",(uiI/8)+1);
#endif			
		for (uiJ=0;uiJ<8;uiJ++)
		{
#ifdef NET_DEBUG			
			Uart_Printf("%02x ",*(pucPtr + uiJ));
#endif			
			aucHexData[uiJ] ^= *(pucPtr + uiJ);
		}
#ifdef NET_DEBUG
		Uart_Printf("\n");
#endif		
		/*取工作密钥（位于密钥区第02行，第09列）对8字节aucHexData加密，结果寸于aucHexData8*/
		des(aucHexData,aucHexData,WorkKey);
		pucPtr += 8;
	}


    memcpy(pucOutMAC, aucHexData, 8);
#ifdef NET_DEBUG
    Uart_Printf("pucOutMAC=");
    for (uiI=0;uiI<8;uiI++)
    {
        Uart_Printf("%02x ", aucHexData[uiI]);
    }
    Uart_Printf("\n");
#endif    
    return (NETCOMMS_SUCCESS);
}

/*********************************************************************************
*函数名称：DataEncrypt
*创建人员：李宏
*功能描述：组装，加密上送报文
*入口参数：unsigned char  *pucInData          上送报文起始地址   
*          unsigned short usInDataLen         上送报文长度
*          unsigned char  aucTerminalID[8]    终端号
*          unsigned char  *pucOutData         加密后报文起始地址 
*          unsigned short *pusOutDataLen      加密报文长度
*返  回：
*
**********************************************************************************/
unsigned char DataEncrypt(unsigned char *pucInData, unsigned short usInDataLen, unsigned char aucTerminalID[8],
                          unsigned char *pucOutData, unsigned short *pusOutDataLen)
{
    unsigned char   aucMAC[8];
    unsigned char   Time[7];
    unsigned char   Date[7];
    unsigned char   *pucPtr,*pucTemp;
    unsigned char   ucResult;
    unsigned char   aucRanNum[8];
    unsigned short  usLen;
    unsigned short  usTemp;
      
    Uart_Printf("Enter DataEncrypt!\n");
    /*判断数据长度是否合法*/
    if (usInDataLen>NETWORK_MAXDATALEN-21)
    {
    	return (ERR_NETCOMMS_OVERBUFLIMIT);
    }	
    
    memset(aucMAC,0,sizeof(aucMAC));
    memset(EncryptData,0,sizeof(EncryptData));
    
    /*将原始报文存入临时空间：原始报文长度（2BCD）＋原始报文*/
    short_bcd(EncryptData,2,&usInDataLen);
    memcpy(&EncryptData[2],pucInData,usInDataLen);
    
    pucPtr = pucOutData;
    pucPtr += 2;
    
    /*版本号*/
    asc_bcd(pucPtr, 2, (uchar *)NETCOMMS_VERSION, 4);
    pucPtr += 2;
    
    /*产生随机主密钥索引*/
    Os__read_time_sec(Time);
    if( NETCOMMS_KEYNUM >= 10 )
	{
		//以2位秒来作为随机数
		usTemp=(unsigned short)asc_long(Time+4,2);
	}
	else
	{
		//以1位秒来作为随机数
		usTemp=(unsigned short)asc_long(Time+5,1);
	}
    KEYINDEX_NETCOMMS_MASTERKEY=usTemp%NETCOMMS_KEYNUM;
    Uart_Printf("check:%d\n",KEYINDEX_NETCOMMS_MASTERKEY);
    memcpy(pucPtr,(unsigned char *)&KEYINDEX_NETCOMMS_MASTERKEY,1);
    Uart_Printf("Index:%02x\n",*pucPtr);
    pucPtr += 1;
    
    /*产生随机数：4字节压缩终端号和4字节压缩的当前时间*/
    /*终端号*/
    memset(aucRanNum,0,sizeof(aucRanNum));
    asc_hex(aucRanNum,4,aucTerminalID, 8);
    
    /*压缩当前时间,格式：ddhhmmss*/
    memset(Date,0,sizeof(Date));
    Os__read_date(Date);
    asc_hex(&aucRanNum[4], 1, Date, 2);
    
    memset(Time, 0, sizeof(Time));
    Os__read_time_sec(Time);
    asc_hex(&aucRanNum[5], 3, Time, 6);
    
    memcpy(pucPtr, aucRanNum, 8);
    pucPtr += 8;
   	                                     
	/*产生工作密钥*/				                                     			                                   
    ucResult = GetWorkKey(aucRanNum);
    if( ucResult != NETCOMMS_SUCCESS )
	{
		return ucResult;
	}
    
    pucTemp=pucPtr;

#ifdef NET_DEBUG   
    {
		int im,ij,il;
		il=0;
		Uart_Printf("\ncheck:Send Origin DATA=%d\n",usInDataLen+2);
		for( im=0;im<usInDataLen+2;im=im+16 )
		{
			for( ij=0;ij<16;ij++ )
				Uart_Printf("%02x ",EncryptData[il++]);
			Uart_Printf("\n");	
		}
	}
#endif
    
    /*做DES加密后加入发送报文，最后不足8字节的用0x00补足*/
    for (usLen=0;usLen<usInDataLen+2;usLen+=8)
    {
        /*按每8个字节用工作密钥对aucOriginData内的数据进行加密，结果连在pucTemp后*/
        des(&EncryptData[usLen],pucTemp,WorkKey);
	    pucTemp += 8;
	}
		  
    
    /*计算MAC*/
    ucResult = CalcNetworkMAC(pucPtr, usLen, aucMAC);
    memcpy(pucTemp, aucMAC, 8);
    pucPtr = pucTemp + 8;
    
    /*计算报文总长度*/
    *pusOutDataLen = pucPtr - pucOutData;
    usLen= pucPtr - pucOutData-2;
    short_bcd(pucOutData, 2, &usLen);
    
#ifdef NET_DEBUG    
   pucPtr = pucOutData;
   {
		int im,ij,il;
		il=0;
		Uart_Printf("\ncheck:Send NETCOMMS_DATA=%d\n",*pusOutDataLen);
		for( im=0;im<*pusOutDataLen;im=im+16 )
		{
			for( ij=0;ij<16;ij++ )
				Uart_Printf("%02x ",*(pucPtr++));
			Uart_Printf("\n");	
		}
	}
#endif  
    return(NETCOMMS_SUCCESS);
  
} 

/*********************************************************************************
*函数名称：DataDecrypt
*创建人员：李宏
*功能描述：解密报文
*入口参数：unsigned char  *pucInData          需解密报文起始地址   
*          unsigned char  *pucOutData         解密后报文起始地址 
*          unsigned short *pusOutDataLen      解密报文长度
*返  回：
*
**********************************************************************************/
unsigned char DataDecrypt(unsigned char *pucInData, //unsigned short usInDataLen,
                          unsigned char *pucOutData, unsigned short *pusOutDataLen) 
{
    unsigned char aucMAC[8];
    unsigned char ucResult;
    unsigned long ulDataLen;
    unsigned long ulLen;
    unsigned int  uI;
    
    memset(aucMAC, 0, sizeof(aucMAC));
    memset(DecryptData, 0, sizeof(DecryptData));
    
    /*取得报文总长度*/
    ulDataLen = bcd_long(pucInData, 4)+2;
    
    /*按密钥索引取得主密钥*/
    KEYINDEX_NETCOMMS_MASTERKEY =(short) *(pucInData + 4);
    
    Uart_Printf("KEYINDEX_NETCOMMS_MASTERKEY=%02x\n",KEYINDEX_NETCOMMS_MASTERKEY);
    if (KEYINDEX_NETCOMMS_MASTERKEY > NETCOMMS_KEYNUM )
    {
    	return (ERR_NETCOMMS_INDEXERROR);
    }
    
    /*对随机数取得通讯工作密钥*/
    ucResult = GetWorkKey(pucInData + 5);
    if( ucResult != NETCOMMS_SUCCESS )
    {
    	return(ucResult);
    }
    
    /*取得加密后报文长度(两个字节的报文长度）*/
    ulLen = ulDataLen-2-2-1-8-8;
    if(ulLen > NETWORK_MAXDATALEN)
    {
    	return (ERR_NETCOMMS_OVERBUFLIMIT);
    }
    /*计算MAC并判断*/
    ucResult = CalcNetworkMAC(pucInData+13, ulLen, aucMAC);
    if (memcmp(pucInData+ulDataLen-8, aucMAC, 8))
	{
        return (ERR_NETCOMMS_MACERROR);
	}
    
    /*解密加密的报文*/
    for (uI=0;uI<ulLen;uI+=8)
    {
        /*按每8个字节用工作密钥对pucInData中的有效数据进行解密，结果寸于ucDecryptData中*/
        desm1(&pucInData[13+uI],&DecryptData[uI],WorkKey);
	}			                                        
    
    Uart_Printf("ulLen=%d\n",ulLen);
    /*取得解密报文的长度（1,2字节）*/
    *pusOutDataLen = (short)bcd_long(&DecryptData[0], 4);
    
#ifdef NET_DEBUG    
    {
		int im,ij,il;
		il=0;
		Uart_Printf("\ncheck:Receive Original DATA Len=%d\n",*pusOutDataLen);
		for( im=0;im<*pusOutDataLen+2;im=im+16 )
		{
			for( ij=0;ij<16;ij++ )
				Uart_Printf("%02x ",DecryptData[il++]);
			Uart_Printf("\n");	
		}
	}
#endif	
    
    /*取得解密后的报文*/
    memcpy(pucOutData, &DecryptData[2], *pusOutDataLen);
    
    return (NETCOMMS_SUCCESS);
}	


void COMMSENCRYPT_ErrMsg(unsigned char ucResult)
{
	unsigned char aucErrCode[16+1];
	unsigned char aucPrintBuf[16+1];
	
	memset(aucErrCode,0,sizeof(aucErrCode));
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	
	switch(ucResult)
	{
	case ERR_NETCOMMS_VERSION:
		memcpy(aucPrintBuf,"通讯版本错",10);
		break;
	case ERR_NETCOMMS_OVERBUFLIMIT:
		memcpy(aucPrintBuf,"缓存超限",8);
		break;
	case ERR_NETCOMMS_INDEXERROR:
		memcpy(aucPrintBuf,"密钥索引错",10);
		break;
	case ERR_NETCOMMS_MACERROR:
		memcpy(aucPrintBuf,"MAC错",5);
		break;
	default:
		memcpy(aucPrintBuf,"通讯版本错",10);
		break;
	}
	
	sprintf(aucErrCode,"错误码:%02x",ucResult);
	
	Os__clr_display(255);
	Os__GB2312_display(0,0,aucErrCode);
	Os__GB2312_display(1,0,aucPrintBuf);
	Os__xget_key();
	return;
}
