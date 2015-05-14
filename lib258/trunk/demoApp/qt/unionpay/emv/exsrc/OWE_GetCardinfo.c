
#include "include.h"
#include "global.h"
#include "new_drv.h"
unsigned char OWE_GetCardInfo(void)
{
	unsigned int ret;
	NEW_DRV_TYPE  new_drv;
	unsigned char ucKey;
	//seven *EventInfo;

	printf("\n\n\n 1 MainMenu_ReciveMsg Os_Wait_Event....\n");
    ret = Os_Wait_Event(MAG_DRV | KEY_DRV | ICC_DRV | MFR_DRV, &new_drv, GUI_DELAY_TIMEOUT);//MFR_DRV
	printf("\n 2 Os_Wait_Event=ret:%02x\n",ret);
	if(ret == 0)
	{
		printf("\n 3 new_drv.drv_type=ret:%02x",new_drv.drv_type);
		if(new_drv.drv_type == KEY_DRV)
		{
			//获得按键事件;
			if(new_drv.drv_data.gen_status== DRV_ENDED)
			{
                memset((char*)&ProUiFace.OweDrv, 0x00, sizeof(NEW_DRV_TYPE));
                memcpy((char*)&ProUiFace.OweDrv, (char*)&new_drv, sizeof(NEW_DRV_TYPE));

				ucKey=new_drv.drv_data.xxdata[1];
				printf("\n ucKey:%02x.", ucKey);                
			}
		}else if(new_drv.drv_type == MAG_DRV)
		{
		//获得磁条卡事件;
			printf("\n 2 MainMenu_ReciveMsg MAG_DRV....");
            if(new_drv.drv_data.gen_status  == DRV_ENDED){
                memset((char*)&ProUiFace.OweDrv, 0x00, sizeof(NEW_DRV_TYPE));
                memcpy((char*)&ProUiFace.OweDrv, (char*)&new_drv, sizeof(NEW_DRV_TYPE));

				//获得卡号
				memset(ProUiFace.UiToPro.CardInfo.aucCardAcc,0x00, sizeof(ProUiFace.UiToPro.CardInfo.aucCardAcc));
				memcpy(ProUiFace.UiToPro.CardInfo.aucCardAcc, (unsigned char *)&new_drv.drv_data, sizeof(NEW_DRV_TYPE));
				ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_SWIPECARD;
				ret = OWE_GetIsoTrack((unsigned char *)&new_drv.drv_data);

				if(ProUiFace.UiToPro.CardInfo.uiLenIso2 >= 16)
					ProUiFace.UiToPro.CardInfo.ucOweFlag= 0xAA;
			}            
		} else 
		if(new_drv.drv_type == ICC_DRV)
		{
            memset((char*)&ProUiFace.OweDrv, 0x00, sizeof(NEW_DRV_TYPE));
            memcpy((char*)&ProUiFace.OweDrv, (char*)&new_drv, sizeof(NEW_DRV_TYPE));

			// IC卡插卡结果处理;
			printf("\n 4 MainMenu_ReciveMsg ICC_DRV....");
			ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_INSERTCARD;
		} else
		if(new_drv.drv_type == MFR_DRV)
		{
			// 非接卡轮询结果处理;
            memset((char*)&ProUiFace.OweDrv, 0x00, sizeof(NEW_DRV_TYPE));
            memcpy((char*)&ProUiFace.OweDrv, (char*)&new_drv, sizeof(NEW_DRV_TYPE));

			printf("\n 4 MainMenu_ReciveMsg MFR_DRV....");
			ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_SAND_PUTCARD;
			G_NORMALTRANS_euCardSpecies = CARDSPECIES_EMV;

		} else
		if(new_drv.drv_type == DRV_TIMEOUT)
		{
		// 超时处理;
            printf("\n%dEntry into timeout",new_drv.drv_type);
		}
	} else
	{
		OWE_RstErrInfo(ret);
	}

	printf("\n-----------End-----------------\n\n\n\n");

}

/*
unsigned char OWE_GetIsoTrack(unsigned char *pucTrack)
{
    unsigned short uiI,usJJ,ucJ;
    unsigned char  *pucPtr,ucAccType;

    unsigned char ucISO2Status;
    unsigned int uiISO2Len;
    unsigned char aucISO2[38];
    unsigned char ucISO3Status;
    unsigned int uiISO3Len;
    unsigned char aucISO3[105];

    pucPtr = pucTrack;
    pucPtr += 2;

    // printf("Get:\n");
    //  for(uiI=0;uiI<100;uiI++)
    //      printf("%02X ",pucPtr[uiI]);
    //  printf("\n");

    // Track2 information
    ucISO2Status = *(pucPtr);

    pucPtr ++;

    uiISO2Len = *(pucPtr);

    //**判断磁道长度是否正确**
    if(uiISO2Len > TRANS_ISO2LEN || uiISO2Len<TRANS_ISO2MINLEN)
        return(ERR_MAG_TRACKDATA);

    pucPtr ++;

    printf("\nucISO2Status=%02x\n",ucISO2Status);
    printf("uiISO2Len=%d\n",uiISO2Len);


    for( ucJ=0; ucJ<uiISO2Len; ucJ++,pucPtr ++)
    {
        aucISO2[ucJ] =  *(pucPtr) | 0x30;
    }
    printf("\nTrack2 Data:%s\n",aucISO2);

    if (!memcmp(&aucISO2[20],"===",3))
        return (ERR_SANDMAGCARD);


	ProUiFace.UiToPro.CardInfo.uiLenIso2=uiISO2Len;
	memcpy(ProUiFace.UiToPro.CardInfo.aucIso2,aucISO2, uiISO2Len);

    //* Track3 information *
    ucISO3Status = *(pucPtr);

    pucPtr ++;
    uiISO3Len = *(pucPtr);

    //***判断磁道长度是否正确***
    if(uiISO3Len > TRANS_ISO3LEN)
        return(ERR_MAG_TRACKDATA);
    pucPtr ++;

    printf("Track3 ISO3Status=%02x\n",ucISO3Status);
    printf("Track3 uiISO3Len=%d\n",uiISO3Len);

    for( uiI=0; uiI<uiISO3Len; uiI++,pucPtr ++)
    {
        aucISO3[uiI] =  *(pucPtr) | 0x30;
    }
	//printf("Track3 Data:%s\n",aucISO3);
	//printf("Track3 uiISO3Len=%02x\n",uiISO3Len);
	ProUiFace.UiToPro.CardInfo.uiLenIso3=uiISO3Len;
	memcpy(ProUiFace.UiToPro.CardInfo.aucIso3,aucISO3, uiISO3Len);

	for(ucJ=0;ucJ<ProUiFace.UiToPro.CardInfo.uiLenIso2;ucJ++)
	{
		if(ProUiFace.UiToPro.CardInfo.aucIso2[ucJ] == '=')
		{
			memset(ProUiFace.UiToPro.CardInfo.aucMagInfo,0,20);
			memcpy(ProUiFace.UiToPro.CardInfo.aucMagInfo,ProUiFace.UiToPro.CardInfo.aucIso2, ucJ);
			break;
		}
	}
        printf("\n OWE_GetIsoTrack. aucCardAcc:[%d][%s].end.",ucJ,ProUiFace.UiToPro.CardInfo.aucMagInfo);
		
    return 0;
}
*/
//unsigned char MAG_GetTrackInfo(unsigned char *pucTrack)
unsigned char OWE_GetIsoTrack(unsigned char *pucTrack)
{
	unsigned short uiI,usJJ,ucJ;
	unsigned char  *pucPtr,ucAccType;

//	util_Printf("\n			1.MAG_GetTrackInfo ..................\n\n [ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
	util_Printf("\n		MAG_GetTrackInfo ucResult = [0001]\n");
	pucPtr = pucTrack;
	
	pucPtr += 2;
	/* Track2 information */
	G_EXTRATRANS_ucISO2Status = *(pucPtr);
	pucPtr ++;
	G_EXTRATRANS_uiISO2Len = *(pucPtr);

	util_Printf("		MAG_GetTrackInfo ucResult = [0002] --uiISO2Len: [%d]\n",G_EXTRATRANS_uiISO2Len);
	/***判断磁道长度是否正确***/
	if(G_EXTRATRANS_uiISO2Len > TRANS_ISO2LEN || G_EXTRATRANS_uiISO2Len<TRANS_ISO2MINLEN)
		return(ERR_MAG_TRACKDATA);

	util_Printf("		MAG_GetTrackInfo ucResult = [0003]\n");
		
	pucPtr ++;

	util_Printf("G_EXTRATRANS_ucISO2Status=%02x,%02x\n",G_EXTRATRANS_ucISO2Status,G_EXTRATRANS_uiISO2Len);

	for( ucJ=0; ucJ<G_EXTRATRANS_uiISO2Len; ucJ++,pucPtr ++)
	{
    		G_EXTRATRANS_aucISO2[ucJ] =  *(pucPtr) | 0x30;
	}
	util_Printf("Track2 Data:%s\n",G_EXTRATRANS_aucISO2);

       if (!memcmp(&G_EXTRATRANS_aucISO2[20],"===",3)){
		util_Printf("retrun ERR_SANDMAGCARD.\n");
		return (ERR_SANDMAGCARD);
       }

	ProUiFace.UiToPro.CardInfo.uiLenIso2=G_EXTRATRANS_uiISO2Len;
	memcpy(ProUiFace.UiToPro.CardInfo.aucIso2,G_EXTRATRANS_aucISO2, G_EXTRATRANS_uiISO2Len);


    /* Track3 information */
   	G_EXTRATRANS_ucISO3Status = *(pucPtr);

	pucPtr ++;
	G_EXTRATRANS_uiISO3Len = *(pucPtr);

	/***判断磁道长度是否正确***/
	if(G_EXTRATRANS_uiISO3Len > TRANS_ISO3LEN)
		return(ERR_MAG_TRACKDATA);
	pucPtr ++;

	util_Printf("Track3 ISO3Status=%02x,%02x\n",G_EXTRATRANS_ucISO3Status,G_EXTRATRANS_uiISO3Len);
//	util_Printf("\n			2.MAG_GetTrackInfo ..................\n\n [ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
	for( uiI=0; uiI<G_EXTRATRANS_uiISO3Len; uiI++,pucPtr ++)
        {
            G_EXTRATRANS_aucISO3[uiI] =  *(pucPtr) | 0x30;
        }
	ProUiFace.UiToPro.CardInfo.uiLenIso3=G_EXTRATRANS_uiISO3Len;
	memcpy(ProUiFace.UiToPro.CardInfo.aucIso3,G_EXTRATRANS_aucISO3, G_EXTRATRANS_uiISO3Len);


//util_Printf("Track3 Data:%s\n",G_EXTRATRANS_aucISO3);
//util_Printf("%s\n",G_EXTRATRANS_aucISO3);
//util_Printf("\n			3.MAG_GetTrackInfo ..................\n\n [ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
	if( (G_EXTRATRANS_ucISO2Status == SUCCESS_TRACKDATA)
	  &&( (G_EXTRATRANS_ucISO3Status == SUCCESS_TRACKDATA)
	    ||(G_EXTRATRANS_ucISO3Status == DRV_FULL)
		||(G_EXTRATRANS_ucISO3Status == DRV_FREE))
	  )
	{
		/**************取卡号后12位减一位******************/
		for(ucJ=0;ucJ<G_EXTRATRANS_uiISO2Len;ucJ++)
		{
			if(G_EXTRATRANS_aucISO2[ucJ] == '=')
			{
				/*************取卡号***************/
				//---------FANGBO MODIFY DATE:2006/01/05------
	util_Printf("\n	4.MAG_GetTrackInfo ..................\n\n 		[ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", 
	ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);				

			memset(ProUiFace.UiToPro.CardInfo.aucMagInfo,0,20);
			memcpy(ProUiFace.UiToPro.CardInfo.aucMagInfo,G_EXTRATRANS_aucISO2, ucJ);

				util_Printf("\nG_EXTRATRANS_aucISO2 =[%d]\n",ucJ);
				memset(G_NORMALTRANS_ucDispSourceAcc,0,20);
				memcpy(G_NORMALTRANS_ucDispSourceAcc, G_EXTRATRANS_aucISO2, ucJ);// max 20
				//util_Printf("G_NORMALTRANS_ucDispSourceAcc = %s\n",G_NORMALTRANS_ucDispSourceAcc);
	//util_Printf("\n	5.MAG_GetTrackInfo ..................\n\n 		[ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", 
	//ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
				
				G_NORMALTRANS_ucDispSourceAccLen = ucJ;
				asc_bcd(G_NORMALTRANS_ucExpiredDate,2,&G_EXTRATRANS_aucISO2[ucJ+1],4);
				//------------end----------------
				if(ucJ>=13)
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[ucJ-13],12);
				else
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[0],12);
				break;
			}
		}
		/***************************/
		if ((G_EXTRATRANS_ucISO3Status == DRV_FULL) ||
			(G_EXTRATRANS_ucISO3Status == DRV_FREE)
			)
		{
			G_NORMALTRANS_ucSourceAccType = '0';
			G_EXTRATRANS_uiISO3Len = 0;
		}else
		{
			for(uiI=0;uiI<G_EXTRATRANS_uiISO3Len;uiI++)
			{
				if(G_EXTRATRANS_aucISO3[uiI]=='=')
				{
					if(!G_EXTRATRANS_uiISO2Len)
					{
						if(uiI>=13)
							asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO3[uiI-13],12);
						else
							asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO3[0],12);
					}
				}
				break;
			}

			if(uiI>=G_EXTRATRANS_uiISO3Len)
				G_NORMALTRANS_ucSourceAccType = '0';
			else
			{
				usJJ = (unsigned short)asc_long(&G_EXTRATRANS_aucISO3[uiI+1],3);

				util_Printf("usJJ=%d\n",usJJ);

				if(usJJ == 156)
					uiI+=3;
				else
					uiI+=1;
				ucAccType = G_EXTRATRANS_aucISO3[uiI+27];
				switch(ucAccType)
				{
					case 0x30:
					case 0x31:
					case 0x32:
					case 0x33:
					case 0x34:
						G_NORMALTRANS_ucSourceAccType = '0';
						break;
					default:
						G_NORMALTRANS_ucSourceAccType = '0';
						break;
				}
			}
		}

		util_Printf("G_NORMALTRANS_ucSourceAccType=%02x\n",G_NORMALTRANS_ucSourceAccType);

		asc_bcd(G_NORMALTRANS_aucISO2,(G_EXTRATRANS_uiISO2Len+1)/2,G_EXTRATRANS_aucISO2,((G_EXTRATRANS_uiISO2Len*2)+1)/2);
		asc_bcd(G_NORMALTRANS_aucISO3,(G_EXTRATRANS_uiISO3Len+1)/2,G_EXTRATRANS_aucISO3,((G_EXTRATRANS_uiISO3Len*2)+1)/2);
		G_NORMALTRANS_ucISO2Len = G_EXTRATRANS_uiISO2Len;
		G_NORMALTRANS_ucISO3Len = G_EXTRATRANS_uiISO3Len;

		util_Printf("G_EXTRATRANS_aucISO2 ucJ = %d \n",ucJ) ;
#ifdef USEINSERTCARD
		if(G_EXTRATRANS_aucISO2[ucJ+5]=='2' ||(G_EXTRATRANS_aucISO2[ucJ+5]=='6'))
			return ERR_USEICCARDFIRST ;
		else if(G_NORMALTRANS_ucFallBackFlag==2)//如果磁条卡service code不为2,6,不能当做fallback交易
			G_NORMALTRANS_ucFallBackFlag=0;

#endif
			return(SUCCESS);
	}else
	{
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
#ifdef GUI_PROJECT
	ProUiFace.ProToUi.uiLines=2;	
   	memcpy(ProUiFace.ProToUi.aucLine2,"刷卡错误",strlen((char*)"刷卡错误"));
#else
		//Os__GB2312_display(1, 0, (uchar *)"刷卡错误");
		//Os__GB2312_display(2, 0, (uchar *)"请继续刷卡-->");
		//Os__GB2312_display(3, 0, (uchar *)"或按取消键退出");
#endif		
		return(ERR_END);
	}
}

unsigned int OWE_RstErrInfo(int ret)
{
    if(ret == KEY_DRV_ERROR)
    {
        printf("\n OWE_ERROR: open KER DRV ERR.");
    }
    else if(ret == MAG_DRV_ERROR)
    {
        printf("\n OWE_ERROR: open MAG DRV ERR.");
    }
    else if(ret == MFR_DRV_ERROR)
    {
        printf("\n OWE_ERROR: open MFC DRV ERR.");
    }
    else if(ret == ICC_DRV_ERROR)
    {
        printf("\n OWE_ERROR: open ICC DRV ERR.");
    }
    else
    {
        printf("\n OWE_ERROR: other ERR.");
    }
    return 	 ret;
}
