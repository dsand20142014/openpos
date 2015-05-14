

#include <include.h>
#include <global.h>
#include <xdata.h>
#include <osicc.h>
#include <smart.h>
#include <toolslib.h>



unsigned char PBOC_ISOSelectFilebyAID(unsigned char ucReader,
			unsigned char *pucAID,unsigned char ucAIDLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xA4;
	Iso7816In.ucP1  = 0x04;
	Iso7816In.ucP2  = 00;

	Iso7816In.ucP3 = ucAIDLen;
	memcpy(&Iso7816In.aucDataIn[0],pucAID,ucAIDLen);
	Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE4)) != OK)         //old is type4
		return(ucResult);
	
	return(PBOC_ISOCheckReturn());
}


unsigned char PBOC_ISOCheckReturn(void)
{

	Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
	Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];

    util_Printf("ucSW1=%02x\n",Iso7816Out.ucSW1);
    util_Printf("ucSW2=%02x\n",Iso7816Out.ucSW2);
	switch(Iso7816Out.ucSW1)
	{
	case 0x90:
		return(SMART_OK);
	case 0x63:
		if(  (Iso7816Out.ucSW2 >= 0xC0)
		   &&(Iso7816Out.ucSW2 <= 0xCF)
		  )
		{
			G_RUNDATA_ucPINRetry = Iso7816Out.ucSW2 & 0x0F;
#ifdef TEST
			util_Printf("SMART_CARD_ERROR1\n");
#endif
			return(SMART_CARD_ERROR);
		}
#ifdef TEST
			util_Printf("SMART_CARD_ERROR2\n");
#endif
		return(SMART_CARD_ERROR);
	default:
#ifdef TEST
			util_Printf("SMART_CARD_ERROR3\n");
#endif
		return(SMART_CARD_ERROR);
	}
}

void PBOC_SetIso7816Out(void)
{
	Iso7816Out.uiLen = sizeof(Iso7816Out.aucData);
	memset(Iso7816Out.aucData,0,Iso7816Out.uiLen);
}

unsigned char PBOC_ISOVerifyPIN(unsigned char ucReader,
			unsigned char ucInLen,unsigned char *pucPinData)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x20;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucInLen;

	memcpy(&Iso7816In.aucDataIn[0],pucPinData,ucInLen);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		{			
			return(ucResult);
		}
		
    return(PBOC_ISOCheckReturn());

}

unsigned char PBOC_ISOInternalAttestation(unsigned char ucReader,
			unsigned char *Challenge,unsigned char ucLen)
{
	unsigned char	ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0x88;
	Iso7816In.ucP1  = 0x01;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;

	memset(Iso7816In.aucDataIn,0,128);
	memcpy(Iso7816In.aucDataIn,Challenge,ucLen);

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE3)) != OK)
		return(ucResult);
	return(PBOC_ISOCheckReturn());
}


unsigned char PBOC_ISOGetResponse(unsigned char ucReader,
			unsigned char ucLen)
{
	unsigned char ucResult;

	Iso7816In.ucCla = 0x00;
	Iso7816In.ucIns = 0xC0;
	Iso7816In.ucP1  = 0x00;
	Iso7816In.ucP2  = 0x00;
	Iso7816In.ucP3  = ucLen;

	PBOC_SetIso7816Out();
	if( (ucResult = SMART_ISO(ucReader,
						SMART_ASYNC,
						(unsigned char *)&Iso7816In,
						Iso7816Out.aucData,
						&Iso7816Out.uiLen,
						TYPE2)) != OK)
		return(ucResult);
    return(PBOC_ISOCheckReturn());
}



unsigned char PBOC_ISOSelectFilebyFID(unsigned char ucReader,
            unsigned char *pucFID)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xA4;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3  = 0x02;

    //PBOC_SetIso7816In();
    memcpy(&Iso7816In.aucDataIn[0],pucFID,2);
    Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if( (ucResult = SMART_ISO(ucReader,
                        SMART_ASYNC,
                        (unsigned char *)&Iso7816In,
                        Iso7816Out.aucData,
                        &Iso7816Out.uiLen,
                        TYPE4)) != OK)         //old is type4
        return(ucResult);

    return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOGetChallenge( unsigned char ucReader )
{
    unsigned char ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0x84;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3  = 0x04;

    PBOC_SetIso7816Out();
    if( (ucResult = SMART_ISO(ucReader,
                        SMART_ASYNC,
                        (unsigned char *)&Iso7816In,
                        Iso7816Out.aucData,
                        &Iso7816Out.uiLen,
                        TYPE2)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn());
}

unsigned char PBOC_ISOExtAuthentic(unsigned char ucReader,unsigned char ucKeyIndex,unsigned char *pucInData )
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0x82;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2	= ucKeyIndex;
    Iso7816In.ucP3 =  0X08;
    memcpy( &Iso7816In.aucDataIn[0],pucInData,8 );
    Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if( (ucResult = SMART_ISO(ucReader,
                        SMART_ASYNC,
                        (unsigned char *)&Iso7816In,
                        Iso7816Out.aucData,
                        &Iso7816Out.uiLen,
                        TYPE3)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn());
}


unsigned char PBOC_ISOEaserFile( unsigned char ucReader )
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x80;
    Iso7816In.ucIns = 0x0E;
    Iso7816In.ucP1 =  0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3  = 0;

    PBOC_SetIso7816Out();
    if( (ucResult = SMART_ISO(ucReader,
                        SMART_ASYNC,
                        (unsigned char *)&Iso7816In,
                        Iso7816Out.aucData,
                        &Iso7816Out.uiLen,
                        TYPE2)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_ISOCreateFile( unsigned char ucReader,unsigned char ucFID,unsigned char *pucInData )
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x80;
    Iso7816In.ucIns = 0xE0;
    Iso7816In.ucP1 =  0x00;
    Iso7816In.ucP2  = ucFID;
    Iso7816In.ucP3  = 7;

    memcpy(&Iso7816In.aucDataIn[0],pucInData,Iso7816In.ucP3);

    PBOC_SetIso7816Out();
    if( (ucResult = SMART_ISO(ucReader,
                        SMART_ASYNC,
                        (unsigned char *)&Iso7816In,
                        Iso7816Out.aucData,
                        &Iso7816Out.uiLen,
                        TYPE3)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn());
}
unsigned char PBOC_WriteMainKey(unsigned char ucReader,unsigned char *pucKey)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x80;
    Iso7816In.ucIns = 0xD4;
    Iso7816In.ucP1  = 0x01;
    Iso7816In.ucP2  = 0x00;

    Iso7816In.ucP3 = 0x15;
    memcpy(&Iso7816In.aucDataIn[0],pucKey,0x15);
    Iso7816In.aucDataIn[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if( (ucResult = SMART_ISO( ucReader,
                        SMART_ASYNC,
                        (unsigned char *)&Iso7816In,
                        Iso7816Out.aucData,
                        &Iso7816Out.uiLen,
                        TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn());
}


void PBOC_CPU_TEST(VOID)
{
 unsigned char aucData[100],ucResult;
    unsigned char aucRandom[8],aucBuf[100];
    unsigned int uiI;
    unsigned int usLen,ucLen;
	NEW_DRV_TYPE  new_drv;

util_Printf("\n CPU_Issue waite Insert ICC card--->>>>\n");
	ucResult = Os_Wait_Event(MAG_DRV | KEY_DRV | ICC_DRV , &new_drv, 10000);//MFR_DRV

	
    if(ucResult==SUCCESS)
    ucResult=PBOC_ISOSelectFilebyFID(0,(uchar *)"\x3F\x00");
    if(ucResult==SUCCESS)
    {
        util_Printf("选主目录成功\n");
        ucResult=PBOC_ISOGetChallenge(0);
        memset(aucRandom,0,sizeof(aucRandom));
        memcpy(aucRandom,Iso7816Out.aucData,4);
    }
    if(ucResult==SUCCESS)
    {
        util_Printf("取随即数成功\n");

        memset(aucBuf,0,sizeof(aucBuf));
        memcpy(aucBuf,aucRandom,4);
        for(uiI=0;uiI<8;uiI++)
            util_Printf("%02x ",aucBuf[uiI]);

        des(aucBuf,aucBuf,(uchar*)"WATCHDAT");
        util_Printf("\nWATCHDAT Des:");
        for(uiI=0;uiI<8;uiI++)
            util_Printf("%02x ",aucBuf[uiI]);

        desm1(aucBuf,aucBuf,(uchar*)"ATimeCOS");
        util_Printf("\nATimeCOS Desm1:");
        for(uiI=0;uiI<8;uiI++)
            util_Printf("%02x ",aucBuf[uiI]);

        des(aucBuf,aucBuf,(uchar*)"WATCHDAT");
        util_Printf("\nWATCHDAT Des:");
        for(uiI=0;uiI<8;uiI++)
            util_Printf("%02x ",aucBuf[uiI]);
    }
    if(ucResult==SUCCESS)
    {
        util_Printf("\ndes成功\n");
        ucResult = PBOC_ISOExtAuthentic(0,0,aucBuf );
    }
    if(ucResult==SUCCESS)
    {
        util_Printf("外部认证成功\n");
        ucResult = PBOC_ISOEaserFile(0);
    }
    if(ucResult==SUCCESS)
    {
        //建立key文件
        util_Printf("删除文件成功\n");
        memset(aucBuf,0,sizeof(aucBuf));
        memcpy(aucBuf,"\x3F\x00\x1C\xFF\xF0\xFF\xFF",7);//3F00 1CFFF0FFFF
        ucResult=PBOC_ISOCreateFile(0,0x00,aucBuf);
    }
    if(ucResult==SUCCESS)
    {
        //写key文件
        util_Printf("建立KEY文件成功\n");
        memset(aucBuf,0,sizeof(aucBuf));
        memcpy(aucBuf,"\x39\xF0\xF0\xAA\xFF",5);
        memcpy(&aucBuf[5],"WATCHDATATimeCOS",16);
        //memcpy(aucBuf,"\x39\xF0\xF0\xAA\xFF\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",0x15);
        ucResult=PBOC_WriteMainKey(0,aucBuf );
    }
    if(ucResult == SUCCESS)
    {
        util_Printf("\n密钥写入成功\n");
    }
    if(ucResult==SUCCESS)
    {
        //创建0005定长文件
        util_Printf("准备建立0005文件\n");
        memset(aucBuf,0,sizeof(aucBuf));
        memcpy(aucBuf,"\x28\x00\x65\xF0\xF0\xFF\xFF",7);//280065F0F0FFFF
        ucResult=PBOC_ISOCreateFile(0,0x05,aucBuf);
    }
    if(ucResult == SUCCESS)
    {
        util_Printf("\n0005文件建立成功\n");
    }

    ////Os__clr_display(255);
   // if(ucResult == SUCCESS)
   //     //Os__GB2312_display(0,0,(uchar *)"SUCCESS!");
   // else
   //     //Os__GB2312_display(0,0,(uchar *)"ERROR!");

    Os__xdelay(100);
    ////Os__GB2312_display(1,0,(uchar *)"ICC_remove");
    Os__ICC_remove();
    return ucResult;

}

