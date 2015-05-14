#include <include.h>
#include <global.h>
#include <xdata.h>

unsigned int  car_status,transnum;
unsigned long uiI;
unsigned short 	uiStartIndex;
int	iTCPHandle;
unsigned char Collect_Send_Trans(uchar ucPt)
{
	unsigned char 	*Ptr,dtemp[512],temp[512],aucBuf[512],aucBuf1[10],*Pt;
	unsigned char		ucIPAddress[5],aucCommuBuf[512],LRC,ucBuf[20];
  unsigned char 	ucResult;
	unsigned int 		uiTimesOut,len,length,i,offset=0,offset1,uiNum;
	unsigned short 	uiIndex,auioffset[3],OutLen;
	unsigned long	ulAmount,uiJ=0;
	
	ucResult=SUCCESS;
	
	while(1)
	{
		LRC=0;
		offset=0;
		memset(dtemp,0,sizeof(dtemp));
	
		/**/
//		if(S_RUNDATA_ucReprintFlag!=1)
		{
			if(transnum==DataSave0.TransInfoData.PtInfoData[ucPt-1].uiPurchaseNb)
				memcpy(&dtemp[offset],"E0",2);//
			else
				memcpy(&dtemp[offset],"D0",2);//ݰ
		}
		offset+=2;
		
		/**/
		uiI++;
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,3,&uiI);	
		memcpy(&dtemp[offset],aucBuf,3);
		offset+=3;
		
		/*Ӧñʶ*/	
		if(ucPt==2)
			memcpy(&dtemp[offset],"SMART",5);
		else
			memcpy(&dtemp[offset],"SDWKC",5);
		offset+=5;
		
		/*̻*/	
		memcpy(&dtemp[offset],DataSave0.ConstantParamData.aucMerchantID,15);
		offset+=15;
		
		/*ն˺*/	
		memcpy(&dtemp[offset],DataSave0.ConstantParamData.aucTerminalID,8);
		offset+=8;
		
		/*κ*/
		memset(aucBuf,0x20,sizeof(aucBuf));
		if(RUNDATA_ucReprintFlag!=1)
			bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
		else
			bcd_asc(aucBuf,DataSave0.TransInfoData.TransTotal.aucBatchNumber,24);	
		memcpy(&dtemp[offset],aucBuf,24);
		offset+=24;				
		
//		if(S_RUNDATA_ucReprintFlag!=1)
		{
			if(transnum==DataSave0.TransInfoData.PtInfoData[ucPt-1].uiPurchaseNb)
			{
				/*ܱ*/
				long_asc(&dtemp[offset],4,(ulong *)&DataSave0.TransInfoData.PtInfoData[ucPt-1].uiPurchaseNb);
				offset+=4;
				
				/*ܽ*/
				long_asc(&dtemp[offset],8,&DataSave0.TransInfoData.PtInfoData[ucPt-1].ulPurchaseAmount);
				offset+=8;
				
				/*ںʱ*/
				CFG_GetDateTime();
				memset(aucBuf,0x20,sizeof(aucBuf));
				bcd_asc(aucBuf,NormalTransData.aucDateTime,14);
				memcpy(&dtemp[offset],aucBuf,14);
				offset+=14;
			}
			else
			{
				offset1=0;
				uiNum=0;
				memset(aucBuf,0x20,sizeof(aucBuf));
				for(uiIndex=uiStartIndex;uiIndex<TRANS_MAXNB;uiIndex++)
				{
					if(DataSave0.TransInfoData.auiTransIndex[uiIndex])
					{
						XDATA_Read_SaveTrans_File(uiIndex);
						if(NormalTransData.ucTransType==TRANS_S_PURCHASE
							&&!NormalTransData.ucKYFlag
							&&NormalTransData.ucPtCode==ucPt)
						{
							uiStartIndex=uiIndex;
							/**/	
							memcpy(&aucBuf[offset1],"04",2);//
							offset1 +=2;
							
							/**/
							bcd_asc(&aucBuf[offset1],NormalTransData.aucUserCardNo,16);	
							offset1 +=19;
							
							/**/
							if(NormalTransData.ucCardType==CARD_CPU)
								memcpy(&aucBuf[offset1],"02",2);
							else
								memcpy(&aucBuf[offset1],"01",2);
							offset1 +=2;
							/*ԭ*/
							ulAmount=NormalTransData.ulTrueBalance;
								
							long_asc(&aucBuf[offset1],8,&ulAmount);	
							offset1 +=8;
							
							/*׽*/
							long_asc(&aucBuf[offset1],8,&NormalTransData.ulAmount);	
							offset1 +=8;
							
							/**/
							if(NormalTransData.ulTrueBalance<NormalTransData.ulAmount)
								ulAmount=0;
							else
								ulAmount=NormalTransData.ulTrueBalance-NormalTransData.ulAmount;
								
							long_asc(&aucBuf[offset1],8,&ulAmount);	
							offset1 +=8;
							
							/*ںʱ*/
							bcd_asc(&aucBuf[offset1],NormalTransData.aucDateTime,14);
							offset1 +=14;
							
							/*TAC*/
							memcpy(&aucBuf[offset1],NormalTransData.aucTraceTac,4);	
							offset1 +=6;
							
							/*ˮ*/
							long_asc(&aucBuf[offset1],6,&NormalTransData.ulTraceNumber);		
							offset1 +=8;
							
							/*ԭˮ*/
							offset1 +=8;
							
							/*Ԥֶ*/
							offset1 +=30;
		
							uiNum++;
							
							Uart_Printf("uiIndex=%d\n",uiIndex);
							Uart_Printf("uiStartIndex=%d\n",uiStartIndex);
							Uart_Printf("uiNum=%d\n",uiNum);
							if(uiNum==3)
							{
								break;		
							}
						}
					}
				}
				
				/*־*/
				transnum=transnum+uiNum;
				if(transnum<DataSave0.TransInfoData.PtInfoData[ucPt-1].uiPurchaseNb)
					memcpy(&dtemp[offset],"1",1);
				else
					memcpy(&dtemp[offset],"0",1);
				offset+=1;
				
				/*ͱ*/
				int_asc(&dtemp[offset],1,&uiNum);
				offset+=1;
				
				/*ϸ*/
				memcpy(&dtemp[offset],aucBuf,offset1);
				offset=offset+offset1;				
			}
		}
		/**/
//		ucResult=TCP_Open(&iTCPHandle);
//		if(ucResult!=SUCCESS) return ucResult;
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"...");
		memset(ucIPAddress,0x00,sizeof(ucIPAddress));
		memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulKFCHostIPAdd,sizeof(unsigned long));
		Uart_Printf("\n ucIPAddress = %03d.%03d.%03d.%03d \n ",
						ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
		Uart_Printf("\n uiHostPort = %04d",DataSave0.ConstantParamData.uiKFCHostPort);
		//Ĳȶԣ	
		for(i=0;i<3;i++)
		{
			ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiKFCHostPort);
			if(!ucResult)
				break;
		}
		if(!ucResult)
		{
			MSG_DisplayMsg( true,0,0,MSG_SENDING);
			memset(aucCommuBuf,0,sizeof(aucCommuBuf));
			aucCommuBuf[0]=CHAR_STX;
			int_bcd(&aucCommuBuf[1],2,&offset);
			memcpy(&aucCommuBuf[3],dtemp,offset);
			aucCommuBuf[3+offset]=CHAR_ETX;
			length=3+offset;
			for(i=1;i<=length;i++)
			{
				LRC ^= aucCommuBuf[i];
			}
			aucCommuBuf[4+offset]=LRC;
			OutLen=4+offset;
			Uart_Printf("\n*******Send  Packet Data*******\n");
			for(i=0;i<=OutLen;i++)
			{
				if((i+1)%20==0) Uart_Printf("\n");
	
				Uart_Printf("%02x ",aucCommuBuf[i]);
			}
			ucResult=TCP_Send(iTCPHandle,aucCommuBuf,(OutLen+1));	
		
			MSG_DisplayMsg( true,0,0,MSG_RECEIVING);
			memset(ucBuf,0x00,sizeof(ucBuf));
			memset(aucCommuBuf,0,sizeof(aucCommuBuf));
			memcpy(ucBuf,"ʹ    ",14);
			if( !ucResult )
			{
					for(i=1;i<=60;i++)
					{
						ucResult = TCP_Recv(iTCPHandle,aucCommuBuf,&OutLen,1);
						int_asc(ucBuf+10,2,&i);
						//Os__GB2312_display(1,0,ucBuf);
						if(!ucResult) break;
	
					}
	#ifdef TEST
						Uart_Printf("\n*********Receive Packet Data***********\n");	
						for(i=0;i<OutLen;i++)
						{
							if((i+1)%20==0) Uart_Printf("\n");
							Uart_Printf("%02x ",aucCommuBuf[i]);
						}
	#endif
	
					OutLen=OutLen-3;
	#ifdef TEST
					Uart_Printf("OutLen=%d\n",OutLen);
					Uart_Printf("\nReceive Data End ucResult:[%02x]",ucResult);
	#endif
					if( !ucResult )
					{
						memcpy(temp,&aucCommuBuf[3],OutLen);
			
						/*У*/
						/*uiJ=asc_long(&temp[2],3);
						if(uiJ!=uiI)
						{
							//Os__clr_display(255);
							//Os__GB2312_display(1,0,(unsigned char *)"У");
							SMC_MSG_WaitKey(4);
							return ERR_CANCEL;
						}*/
								
						/*ӦñʶУ*/
						if(ucPt==2)
						{
							if(memcmp(&temp[5],"SMART",5))
							{
								//Os__clr_display(255);
								//Os__GB2312_display(1,0,(unsigned char *)"ʶУ");
								MSG_WaitKey(4);
								ucResult=ERR_CANCEL;
							}	
						}
						else
						{
							if(memcmp(&temp[5],"SDWKC",5))
							{
								//Os__clr_display(255);
								//Os__GB2312_display(1,0,(unsigned char *)"ʶУ");
								MSG_WaitKey(4);
								ucResult=ERR_CANCEL;
							}			
						}
						
						/*ذУ*/
						if(!memcmp(&temp[0],"E1",2))
						{
							return SUCCESS;;
						}
						else 
						{	
							if(!memcmp(&temp[10],"00",2))
							{
								uiStartIndex++;
								continue;
							}
						}					
					}
					else
						return ucResult;
			}
		}
		else
			return(ucResult);
	}
		
	return ucResult;
}
unsigned char Collect_Send_Manage(uchar ucPt)
{
	unsigned char dtemp[512],temp[512],aucBuf[300];
  unsigned char ucResult,ucIPAddress[5],aucCommuBuf[512],LRC=0,ucBuf[20];
	unsigned int uiTimesOut,i,offset=0;
	unsigned long uiJ=0;
	unsigned short length,OutLen;

	ucResult=SUCCESS;

	uiI=0;
	memset(dtemp,0,sizeof(dtemp));
	
	/**/	
	memcpy(&dtemp[offset],"B0",2);
	offset+=2;
	
	/**/	
	uiI++;
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,3,&uiI);	
	memcpy(&dtemp[offset],aucBuf,3);
	offset+=3;
	
	/*Ӧñʶ*/	
	if(ucPt==2)
		memcpy(&dtemp[offset],"SMART",5);
	else
		memcpy(&dtemp[offset],"SDWKC",5);
	offset+=5;
	
	/*̻*/	
	memcpy(&dtemp[offset],DataSave0.ConstantParamData.aucMerchantID,15);
	offset+=15;
	
	/*ն˺*/	
	memcpy(&dtemp[offset],DataSave0.ConstantParamData.aucTerminalID,8);
	offset+=8;
	
	/*κ*/
	memset(aucBuf,0x20,sizeof(aucBuf));
	if(RUNDATA_ucReprintFlag!=1)
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	else
		bcd_asc(aucBuf,DataSave0.TransInfoData.TransTotal.aucBatchNumber,24);	
	memcpy(&dtemp[offset],aucBuf,24);
	offset+=24;
	
	/*ܱ*/
	long_asc(&dtemp[offset],4,(ulong *)&DataSave0.TransInfoData.PtInfoData[ucPt-1].uiPurchaseNb);
	offset+=4;
	
	/*ܽ*/
	long_asc(&dtemp[offset],8,&DataSave0.TransInfoData.PtInfoData[ucPt-1].ulPurchaseAmount);
	offset+=8;
	
	/*ںʱ*/
	CFG_GetDateTime();
	memset(aucBuf,0x20,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucDateTime,14);
	memcpy(&dtemp[offset],aucBuf,14);
	offset+=14;	
	
	/**/
	ucResult=TCP_Open(&iTCPHandle);
	if(ucResult!=SUCCESS) return ucResult;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"...");
	memset(ucIPAddress,0x00,sizeof(ucIPAddress));
	memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulKFCHostIPAdd,sizeof(unsigned long));
	Uart_Printf("\n ucIPAddress = %03d.%03d.%03d.%03d \n ",
					ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
	Uart_Printf("\n uiHostPort = %04d",DataSave0.ConstantParamData.uiKFCHostPort);
	//Ĳȶԣ	
	for(i=0;i<3;i++)
	{
		ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiKFCHostPort);
		if(!ucResult)
			break;
	}
	if(!ucResult)
	{
		MSG_DisplayMsg( true,0,0,MSG_SENDING);
		memset(aucCommuBuf,0,sizeof(aucCommuBuf));
		aucCommuBuf[0]=CHAR_STX;
		int_bcd(&aucCommuBuf[1],2,&offset);
		memcpy(&aucCommuBuf[3],dtemp,offset);
		aucCommuBuf[3+offset]=CHAR_ETX;
		length=3+offset;
		for(i=1;i<=length;i++)
		{
			LRC ^= aucCommuBuf[i];
		}
		aucCommuBuf[4+offset]=LRC;
		OutLen=4+offset;
		Uart_Printf("\n*******Send  Packet Data*******\n");
		for(i=0;i<=OutLen;i++)
		{
			if((i+1)%20==0) Uart_Printf("\n");

			Uart_Printf("%02x ",aucCommuBuf[i]);
		}
		ucResult=TCP_Send(iTCPHandle,aucCommuBuf,(OutLen+1));		
	
		MSG_DisplayMsg( true,0,0,MSG_RECEIVING);
		memset(ucBuf,0x00,sizeof(ucBuf));
		memset(aucCommuBuf,0,sizeof(aucCommuBuf));
		memcpy(ucBuf,"ʹ    ",14);
		if( !ucResult )
		{
				for(i=1;i<=60;i++)
				{
					ucResult = TCP_Recv(iTCPHandle,aucCommuBuf,&OutLen,1);
					int_asc(ucBuf+10,2,&i);
					//Os__GB2312_display(1,0,ucBuf);
					if(!ucResult) break;

				}
				Uart_Printf("OutLen=%d\n",OutLen);
				Uart_Printf("\n*********Receive Packet Data***********\n");	
				for(i=0;i<OutLen;i++)
				{
					if((i+1)%20==0) Uart_Printf("\n");
					Uart_Printf("%02x ",aucCommuBuf[i]);
				}

				OutLen=OutLen-3;
				Uart_Printf("OutLen=%d\n",OutLen);
				Uart_Printf("\nReceive Data End ucResult:[%02x]",ucResult);
				if( !ucResult )
				{
					memcpy(temp,&aucCommuBuf[3],OutLen);
							
					/*У*/
					/*uiJ=asc_long(&temp[2],3);
					if(uiJ!=uiI)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(1,0,(unsigned char *)"У");
						SMC_MSG_WaitKey(4);
						return ERR_CANCEL;
					}*/
		
					/*ӦñʶУ*/
					if(ucPt==2)
					{
						if(memcmp(&temp[5],"SMART",5))
						{
							//Os__clr_display(255);
							//Os__GB2312_display(1,0,(unsigned char *)"ʶУ");
							MSG_WaitKey(4);
							ucResult=ERR_CANCEL;
						}	
					}
					else
					{
						if(memcmp(&temp[5],"SDWKC",5))
						{
							//Os__clr_display(255);
							//Os__GB2312_display(1,0,(unsigned char *)"ʶУ");
							MSG_WaitKey(4);
							ucResult=ERR_CANCEL;
						}			
					}
				}
		}
	}

	transnum=0;
	uiStartIndex=0;
	
	if((!memcmp(&temp[10],"00",2))&&(ucResult==SUCCESS))
		ucResult=Collect_Send_Trans(ucPt);//ݰͽ
	
	TCP_Close(iTCPHandle);
	Uart_Printf("\n End TCP_Close");
	
	return ucResult;	
}
