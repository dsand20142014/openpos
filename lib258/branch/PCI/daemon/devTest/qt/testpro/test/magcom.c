#include "magcom.h"

extern void _print11(char *fmt,...);

unsigned char MAGCOM_PORT_PreCom2(void)
{	
	unsigned char ucResult;
	//unsigned char ucI;
	//unsigned char ucSBoardLen,aucBuf[17];
    //_print11("\n Precomm2..\n");
	ucResult=Os__init_com3(0x0303, 0x0c00, 0x0c);
    //_print11("\n Precomm2..end=%02x\n",ucResult);
	return  ucResult;// 9600
}
unsigned char MAGCOM_SendData(const unsigned char *pucInData,unsigned short uiInLen )
{
	unsigned int  uiI;
	uchar aucSndBuf[800];

	//³¤¶È³¬ÏÞ
	memset(aucSndBuf,0,sizeof(aucSndBuf));
	if(uiInLen <= sizeof(aucSndBuf))
		memcpy(aucSndBuf,pucInData,uiInLen);
	else
		return 0x02;
//	//_print11("\nBegin:\n");
	for(uiI=0; uiI<uiInLen;uiI++)
	{
//		_print11("%d.\n",uiI);
		Os__txcar3(aucSndBuf[uiI]);        
	}
//	_print11("\nEnd\n");

    return SUCCESS;
}
unsigned char MAGCOM_GetIsoTrack(uchar ucLen1,uchar ucStatu1,uchar* puciso1,
                              uchar ucLen2,uchar ucStatu2,uchar* puciso2,
                              uchar ucLen3,uchar ucStatu3,uchar* puciso3
                              )
{
	int uiOffset=0,uiLen=0,uiJ;
	int ucTrack1Len,ucTrack2Len,ucTrack3Len;	
	uchar ucTrack1Status,ucTrack2Status,ucTrack3Status;
	uchar aucTrack1[256],aucTrack2[256],aucTrack3[256];	
	uchar aucSndBuf[512],CRC,ucResult;

		memset(aucTrack1,0,sizeof(aucTrack1));
		memset(aucTrack2,0,sizeof(aucTrack2));
		memset(aucTrack3,0,sizeof(aucTrack3));	
        ucTrack1Len = ucLen1;
        ucTrack2Len = ucLen2;
        ucTrack3Len = ucLen3;
        ucTrack1Status = ucStatu1;
        ucTrack2Status = ucStatu2;
        ucTrack3Status = ucStatu3;
        memcpy(aucTrack1, puciso1, ucTrack1Len);
        memcpy(aucTrack2, puciso2, ucTrack2Len);
        memcpy(aucTrack3, puciso3, ucTrack3Len);
        //_print11("\n		0003.MAG_GetTrackInfo  ");
		// head		

        //_print11("\n		0003.MAG_GetTrackInfo ...001 ");
		uiOffset = 0;
		memset(aucSndBuf,0,sizeof(aucSndBuf));
        //_print11("\n		0003.MAG_GetTrackInfo ...002 ");
		aucSndBuf[uiOffset++] = 0x02;
		aucSndBuf[uiOffset++] = 0x00;
		aucSndBuf[uiOffset++] = 0x00;
		//-iso1 status lenlenlen data...
		aucSndBuf[uiOffset++] = 0xA1;
        //_print11("\n		0003.MAG_GetTrackInfo ...003 ");
		uiLen = 2+1+3+1+ucTrack1Len;
        //_print11("\n		0004.MAG_GetTrackInfo  ");
		long_tab(&aucSndBuf[uiOffset],2,&uiLen); uiOffset+=2;
		hex_asc(&aucSndBuf[uiOffset],&ucTrack1Status,2); uiOffset+=2;
		aucSndBuf[uiOffset++] = 0x20;
		long_asc(&aucSndBuf[uiOffset],3,&ucTrack1Len); uiOffset+=3;
		aucSndBuf[uiOffset++] = 0x20;
		if(ucTrack1Len)
		memcpy(&aucSndBuf[uiOffset],aucTrack1,ucTrack1Len); uiOffset+=ucTrack1Len;
		//-iso2 status lenlenlen data...
		aucSndBuf[uiOffset++] = 0xA2;
		uiLen = 2+1+3+1+ucTrack2Len;
        //_print11("\n		0005.MAG_GetTrackInfo  ");
		long_tab(&aucSndBuf[uiOffset],2,&uiLen); uiOffset+=2;
		hex_asc(&aucSndBuf[uiOffset],&ucTrack2Status,2); uiOffset+=2;
		aucSndBuf[uiOffset++] = 0x20;
		long_asc(&aucSndBuf[uiOffset],3,&ucTrack2Len); uiOffset+=3;
		aucSndBuf[uiOffset++] = 0x20;
		if(ucTrack2Len)
		memcpy(&aucSndBuf[uiOffset],aucTrack2,ucTrack2Len); uiOffset+=ucTrack2Len;			
		//-iso3
		aucSndBuf[uiOffset++] = 0xA3;
        uiLen = 2+1+3+1+ucTrack3Len;
        //_print11("\n		0006.MAG_GetTrackInfo  ");
		long_tab(&aucSndBuf[uiOffset],2,&uiLen); uiOffset+=2;
		hex_asc(&aucSndBuf[uiOffset],&ucTrack3Status,2); uiOffset+=2;
		aucSndBuf[uiOffset++] = 0x20;
		long_asc(&aucSndBuf[uiOffset],3,&ucTrack3Len); uiOffset+=3;
		aucSndBuf[uiOffset++] = 0x20;
        //_print11("\nBegin Test -- 5\n");
		if(ucTrack3Len)
			memcpy(&aucSndBuf[uiOffset],aucTrack3,ucTrack3Len); uiOffset+=ucTrack3Len;

		aucSndBuf[uiOffset++] = 0x03;

		uiLen = uiOffset-3-1;
        //_print11("\n		0007.MAG_GetTrackInfo  ");
		long_tab(&aucSndBuf[1],2,&uiLen);

		CRC = 0x00;
		for(uiJ = 1; uiJ < uiOffset ; uiJ++)
		{
			CRC ^= aucSndBuf[uiJ];
		}
		aucSndBuf[uiOffset++] = CRC;
        //_print11("\n		0008.MAG_GetTrackInfo  \n");
        ucResult = MAGCOM_PORT_PreCom2();
		if(ucResult==0x00)
            ucResult = MAGCOM_SendData(aucSndBuf,uiOffset);
        //_print11("\n		0009.MAG_GetTrackInfo....End.=%02x\n",ucResult);
        return ucResult;
}
