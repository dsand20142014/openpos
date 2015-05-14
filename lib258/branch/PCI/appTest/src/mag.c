#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <fsync_drvs.h>


unsigned char MAG_GetTrackInfo(unsigned char *pucTrack)
{
	unsigned short uiI,usJJ,ucJ;
	unsigned char  *pucPtr,ucAccType;

	pucPtr = pucTrack;
	pucPtr += 2;
	
	G_EXTRATRANS_ucISO2Status = *(pucPtr);

	pucPtr ++;

	G_EXTRATRANS_uiISO2Len = *(pucPtr);

	if(G_EXTRATRANS_uiISO2Len > TRANS_ISO2LEN || G_EXTRATRANS_uiISO2Len<TRANS_ISO2MINLEN)
		return(ERR_MAG_TRACKDATA);

	pucPtr ++;


	for( ucJ=0; ucJ<G_EXTRATRANS_uiISO2Len; ucJ++,pucPtr ++)
	{
    		G_EXTRATRANS_aucISO2[ucJ] =  *(pucPtr) | 0x30;
	}

       if (!memcmp(&G_EXTRATRANS_aucISO2[20],"===",3))
        return (ERR_SANDMAGCARD);

   	G_EXTRATRANS_ucISO3Status = *(pucPtr);

	pucPtr ++;
	G_EXTRATRANS_uiISO3Len = *(pucPtr);

	if(G_EXTRATRANS_uiISO3Len > TRANS_ISO3LEN)
		return(ERR_MAG_TRACKDATA);
	pucPtr ++;


	for( uiI=0; uiI<G_EXTRATRANS_uiISO3Len; uiI++,pucPtr ++)
        {
            G_EXTRATRANS_aucISO3[uiI] =  *(pucPtr) | 0x30;
        }


	if( (G_EXTRATRANS_ucISO2Status == SUCCESS_TRACKDATA)
	  &&( (G_EXTRATRANS_ucISO3Status == SUCCESS_TRACKDATA)
	    ||(G_EXTRATRANS_ucISO3Status == DRV_FULL)
		||(G_EXTRATRANS_ucISO3Status == DRV_FREE))
	  )
	{
		for(ucJ=0;ucJ<G_EXTRATRANS_uiISO2Len;ucJ++)
		{
			if(G_EXTRATRANS_aucISO2[ucJ] == '=')
			{
			
				memset(G_NORMALTRANS_ucDispSourceAcc,0,sizeof(G_NORMALTRANS_ucDispSourceAcc));
				memcpy(G_NORMALTRANS_ucDispSourceAcc, G_EXTRATRANS_aucISO2, ucJ);
				G_NORMALTRANS_ucDispSourceAccLen = ucJ;
				asc_bcd(G_NORMALTRANS_ucExpiredDate,2,&G_EXTRATRANS_aucISO2[ucJ+1],4);
				if(ucJ>=13)
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[ucJ-13],12);
				else
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[0],12);
				break;
			}
		}
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


		asc_bcd(G_NORMALTRANS_aucISO2,(G_EXTRATRANS_uiISO2Len+1)/2,G_EXTRATRANS_aucISO2,((G_EXTRATRANS_uiISO2Len*2)+1)/2);
		asc_bcd(G_NORMALTRANS_aucISO3,(G_EXTRATRANS_uiISO3Len+1)/2,G_EXTRATRANS_aucISO3,((G_EXTRATRANS_uiISO3Len*2)+1)/2);
		G_NORMALTRANS_ucISO2Len = G_EXTRATRANS_uiISO2Len;
		G_NORMALTRANS_ucISO3Len = G_EXTRATRANS_uiISO3Len;


			return(SUCCESS);
	}
}

unsigned char  MAG_DispCardNo(void)
{
	unsigned char ucResult;
	unsigned short ucJ;
	NEW_DRV_TYPE new_drv;

	while(1)
	{
		//Os__light_on();
		Os__clr_display(0);
		Os__clr_display(1);
		Os__clr_display(2);
		Os__clr_display(3);
		Os__clr_display(4);
		Os__clr_display(5);
		Os__clr_display(6);
		Os__clr_display(7);
		Os__GB2312_display(0, 0, (unsigned char * )"Card ID:");
		Os__display(1, 0, G_NORMALTRANS_ucDispSourceAcc);
		Os__display(2, 0, &G_NORMALTRANS_ucDispSourceAcc[16]);
		Os__GB2312_display(3, 0, (unsigned char * )"Press Enter to continue");

		//ucCardKey = UTIL_GetKey(30);
		Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		switch(new_drv.drv_data.xxdata[1])
		{
			case KEY_ENTER:
				ucResult = SUCCESS;
				break;
			case 99:
			case KEY_CLEAR:
				ucResult = ERR_CANCEL;
				break;
			default:
				continue;
		}
		break;
	}
	
	return ucResult;
}


