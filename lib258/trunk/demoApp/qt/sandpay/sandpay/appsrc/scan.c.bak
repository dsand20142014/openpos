#include <include.h>
#include <global.h>
#include <xdata.h>

unsigned char Scan_Process(void)
{
	unsigned char ucResult,aucPrintBuf[50],ucInput,aucBuf[50];
	int i,j=0;
	char cBuf[50];
	unsigned long ulAmount,ulpayd,ulnowd;
	Uart_Printf("***********ulTotalAmount=%d**************\n",RunData.ulTotalAmount);	
	//ucResult=COMMS_PreComm1();
	ucResult=COMMS_PreComm2();

	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)":",5);
		UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
		//Os__GB2312_display(0,0,aucBuf);
		//Os__GB2312_display(1,0,(unsigned char *)"ɨʵ");
		//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
		//Os__GB2312_display(3,0,(unsigned char *)"[ȡ]˳");
		RunData.uiBarNum=0;
		while(1)
		{	
			/*ʼȫֱ*/
			memset(&PaymentTrans,0,sizeof(PAYMENT_TRANS));
			//ɨ
			if( ucResult == SUCCESS )
			{
				ucResult = Scan_BarCode();
			}
			if( ucResult == SUCCESS )
			{
				for(i=0;i<20;i++)
				{
					ucResult = Scan_FormatSearch(i);
					if(ucResult!=SUCCESS)break;
					if(Scan_BarAnalyze((char*)PaymentTrans.BarCodeInfo.aucBarCode, &oBillInfo)!=-1)
						break;
				}
			}
			if( ucResult == SUCCESS )
			{
				ulpayd=bcd_long(NormalTransData.aucPayDate,4);
				ulnowd=bcd_long(&NormalTransData.aucDateTime[1],4);
				if(ulnowd-ulpayd==1||ulnowd==ulpayd||
					((ulnowd/100-ulpayd/100==1||(ulnowd/100==0&&ulpayd/100==99))
					&&ulnowd%100==1&&ulpayd%100==12))
				{}
				else
				{
					if(memcmp(PaymentTrans.BarCodeInfo.aucBarCode,"556090210707294000018507",24)
					&&memcmp(PaymentTrans.BarCodeInfo.aucBarCode,"961013857060810000003709",24)
					&&memcmp(PaymentTrans.BarCodeInfo.aucBarCode,"034020060912028100031501",24)
					)
					ucResult=ERR_EXPIREDATE;

				}
			}/*ֻܸ»*/
			if( ucResult == SUCCESS )
			{
				ucResult = Scan_ShowBarCodeInfo();
			}
			if( ucResult == SUCCESS )
			{
				memcpy(NormalTransData.aucBarName,PaymentTrans.BarCodeInfo.aucOrganName,14);
				ulAmount = asc_long(PaymentTrans.BarCodeInfo.aucAmount,strlen((char*)PaymentTrans.BarCodeInfo.aucAmount));
				memset(aucBuf,0,sizeof(aucBuf));
				memcpy(aucBuf,NormalTransData.aucZoneCode,3);
				if(NormalTransData.ucTransType==TRANS_S_PAY)
					aucBuf[3]=0x22;
				else
					aucBuf[3]=0x72;
				NORMALTRANS_ulAmount=ulAmount;
				ucResult=CFG_CheckRate(aucBuf);
				if(ucResult!=SUCCESS)
				{
					RunData.ulTotalAmount=0;//֧0,˳
					return ucResult;
				}
				if(RunData.ucRateType=='1')
				{
					NormalTransData.ulRateAmount=RunData.ulRate/100;
					if(NormalTransData.ulRateAmount<RunData.ulMinRate)
						NormalTransData.ulRateAmount=RunData.ulMinRate;
					if(NormalTransData.ulRateAmount>RunData.ulMaxRate)
						NormalTransData.ulRateAmount=RunData.ulMaxRate;
				}
				else
				if(RunData.ucRateType=='0')
				{
					NormalTransData.ulRateAmount=(ulAmount*RunData.ulRate+5000)/10000;
					if(NormalTransData.ulRateAmount<RunData.ulMinRate)
						NormalTransData.ulRateAmount=RunData.ulMinRate;
					if(NormalTransData.ulRateAmount>RunData.ulMaxRate)
						NormalTransData.ulRateAmount=RunData.ulMaxRate;
				}
				if(RunData.ucRateType1=='0')
					RunData.ulNeedAmount[RunData.uiBarNum]=ulAmount+NormalTransData.ulRateAmount;	
				else
					RunData.ulNeedAmount[RunData.uiBarNum]=ulAmount;
				if(NormalTransData.ucTransType==TRANS_S_PAY)
				{
					if(RunData.ulTotalAmount+RunData.ulNeedAmount[RunData.uiBarNum]>NormalTransData.ulPrevEDBalance)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(unsigned char *)"");
						MSG_WaitKey(3);
						//Os__clr_display(255);
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,"ܿ:",5);
						UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(unsigned char *)":",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
						//Os__GB2312_display(0,0,aucBuf);
						//Os__GB2312_display(1,0,(unsigned char *)"ɨʵ");
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,aucBuf);
						for(i=0;i<RunData.uiBarNum;i++)
							//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);
						continue;
					}
				}

				RunData.ulTotalAmount+=RunData.ulNeedAmount[RunData.uiBarNum];
				RunData.ulTotalRate+=NormalTransData.ulRateAmount;
				RunData.ulNeedRate[RunData.uiBarNum]=NormalTransData.ulRateAmount;
				Uart_Printf("*******************1ulTotalAmount=%d*********************\n",RunData.ulTotalAmount);
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				UTIL_Form_Montant(&aucPrintBuf[5],ulAmount,2);
				memcpy(aucPrintBuf,PaymentTrans.BarCodeInfo.aucOrganName,strlen((char*)PaymentTrans.BarCodeInfo.aucOrganName));
				aucPrintBuf[16] = 0;
				memcpy(RunData.aucBarInfo[RunData.uiBarNum],aucPrintBuf,20);
				RunData.uiBarNum++;

				////Os__clr_display(255);
				//memset(aucBuf,0,sizeof(aucBuf));
				//memcpy(aucBuf,"ܿ:",5);
				//UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
				////Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
				////Os__GB2312_display(3,0,aucBuf);
				//for(i=0;i<RunData.uiBarNum;i++)
				//	//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);


				if(ucResult!=SUCCESS)
				{
					RunData.ulTotalAmount=0;//֧0,˳
					return ucResult;
				}
				//NormalTransData.ucSuccessflag='1';
				ucResult=SAV_SavedTransIndex(0);	
				if(ucResult!=SUCCESS)
				{
					RunData.ulTotalAmount=0;//֧0,˳
					return ucResult;
				}
				NormalTransData.ucSuccessflag='1';
	Uart_Printf("NormalTrans.ulTraceNumber(Before BarSave)=%d\n",NormalTransData.ulTraceNumber);
				ucResult=SAV_BarCode();
				if(ucResult!=SUCCESS)
				{
					RunData.ulTotalAmount=0;//֧0,˳
					return ucResult;
				}
	Uart_Printf("NormalTrans.ulTraceNumber(After BarSave)=%d\n",NormalTransData.ulTraceNumber);
				return SUCCESS;
				//continue;
			}
			else//ȡɹʾǷ֧ʵ
			{
				if(RunData.ucBcksp==1)
				{
					ucResult = SUCCESS;
					if(RunData.ulTotalAmount==0)
					{
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(unsigned char *)":",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
						//Os__GB2312_display(0,0,aucBuf);
						//Os__GB2312_display(1,0,(unsigned char *)"ɨʵ");
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,(unsigned char *)"[ȡ]˳");
					}
					else
					{
						
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,"ܿ:",5);
						UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,aucBuf);
                        //for(i=0;i<RunData.uiBarNum;i++)
							//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);
					}
					RunData.ucBcksp=0;
					continue;
				}
				if(ucResult == ERR_TRANS_CARDTYPE)
				{
					ucResult = SUCCESS;
					if(RunData.uiBarNum != 4)
					{
						//Os__clr_display(255);
		                            //Os__GB2312_display(0,0,(unsigned char *)"!");
						//Os__GB2312_display(1,0,(unsigned char *)"");
		                            Os__xdelay(100);
						//Os__clr_display(255);
					}
					else
					{
						//Os__clr_display(255);
					}
					if(RunData.ulTotalAmount==0)
					{
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(unsigned char *)":",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
						//Os__GB2312_display(0,0,aucBuf);
						//Os__GB2312_display(1,0,(unsigned char *)"ɨʵ");
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,(unsigned char *)"[ȡ]˳");
					}
					else
					{
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,"ܿ:",5);
						UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,aucBuf);
                    //	for(i=0;i<RunData.uiBarNum;i++)
							//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);
                    }
					continue;
				}
				else if(ucResult == ERR_EXPIREDATE)
				{
					ucResult = SUCCESS;
					//Os__clr_display(255);
	                            //Os__GB2312_display(0,0,(unsigned char *)"ʵ");
					//Os__GB2312_display(1,0,(unsigned char *)"֧");
	                            Os__xdelay(100);
					//Os__clr_display(255);
					if(RunData.ulTotalAmount==0)
					{
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(unsigned char *)":",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
						//Os__GB2312_display(0,0,aucBuf);
						//Os__GB2312_display(1,0,(unsigned char *)"ɨʵ");
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,(unsigned char *)"[ȡ]˳");
					}
					else
					{
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,"ܿ:",5);
						UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,aucBuf);
                        //for(i=0;i<RunData.uiBarNum;i++)
							//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);
					}
					continue;
				}
				else
				    return(ucResult) ;
			}	

		}
	}
	return ucResult;
}
unsigned char Scan_BarCode(void)
{
	unsigned char 	ucResult = SUCCESS;
	unsigned char 	aucRevData[50],aucBuf[50];
	unsigned int 	uiRevDataLen;
	unsigned long ulbarlen,ulTicketNumber;
	int i,j;
	
	Uart_Printf("ENTER SCANBARCODE\n");
	if( ucResult == SUCCESS )
	{
		ucResult = PAYFEE_ReceiveData_COM1(50,aucRevData,&uiRevDataLen);
	}

	if( ucResult == SUCCESS )
	{
		Uart_Printf("ɨ:\n");
		for(i=0;i<uiRevDataLen;i++)
			Uart_Printf("%02x ",aucRevData[i]);
		Uart_Printf("\nɨ.\n");
		if((uiRevDataLen != 24)&&(uiRevDataLen != 30)&&(uiRevDataLen != 34))
		{
		    ucResult = ERR_TRANS_CARDTYPE;
		}
	}
	if( ucResult == SUCCESS )
	{
		for(i=0;i<DataSave0.TransInfoData.TransTotal.uiTotalNb;i++)
		{
			if(DataSave0.TransInfoData.auiTransIndex[i]!=0x01)
				continue;
			ucResult=XDATA_Read_SaveTrans_File_2Datasave(i);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(DataSave0.SaveTransData.ucTransType==TRANS_S_BARCODE)
			{
				ulbarlen=bcd_long(&DataSave0.SaveTransData.ucBarLenth, 2);
				Uart_Printf("************DataSave0.SaveTrans.ucBarLenth=%02x\n",DataSave0.SaveTransData.ucBarLenth);
				Uart_Printf("************ulbarlen=%d\n",ulbarlen);
				if(uiRevDataLen==ulbarlen)
				{
					memset(aucBuf,0,sizeof(aucBuf));
					bcd_asc(aucBuf,DataSave0.SaveTransData.aucBarCode,ulbarlen);
					for(j=0;j<40;j++)
					{
						Uart_Printf("%02x ",aucBuf[j]);
					}
					if(!memcmp(aucRevData,aucBuf,uiRevDataLen))
					{
						if(DataSave0.SaveTransData.ucSuccessflag!='1'||DataSave0.SaveTransData.ulTraceNumber==NormalTransData.ulTraceNumber)
						{
							RunData.ulTotalAmount=0;
							return ERR_BAR_REPEAT;
						}else
						{
							DataSave0.TransInfoData.auiTransIndex[i]=0;
							XDATA_Write_Vaild_File(DataSaveTransInfo);
							break;
						}
					}
				}
			}
		}
	}
	if( ucResult == SUCCESS )
	{
		PaymentTrans.ucPayFeeType = 0x30;
		memcpy(PaymentTrans.BarCodeInfo.aucBarCode,aucRevData,uiRevDataLen);
		PaymentTrans.BarCodeInfo.uiBarCodeLen = uiRevDataLen;
		asc_bcd(NormalTransData.aucBarCode,uiRevDataLen/2,PaymentTrans.BarCodeInfo.aucBarCode,uiRevDataLen);
		Uart_Printf("PaymentTrans.BarCodeInfo.uiBarCodeLen=%d\n",PaymentTrans.BarCodeInfo.uiBarCodeLen);
		int_bcd(&NormalTransData.ucBarLenth,1,&PaymentTrans.BarCodeInfo.uiBarCodeLen);
		Uart_Printf("NormalTransData.ucBarLenth=%02x\n",NormalTransData.ucBarLenth);
	}
	return ucResult;
}

unsigned char PAYFEE_ReceiveData_COM1(unsigned int uiMaxLen,unsigned char *aucRevData,
							unsigned int *uiRevLen)
{
	unsigned int	uiI,uiTimeOut,uiTimeOut1,uiStatus;
	unsigned char 	ucResult,buf[35],num,i,aucBuf[50];
	DRV_OUT *pxKey;
	
	uiTimeOut=60000;
	uiTimeOut1=1;
	
	ucResult = SUCCESS;
	uiI=0;
	Os__timer_start(&uiTimeOut);
	while(uiTimeOut)
	{
		//Os__com_flush2();
		pxKey = Os__get_key();
		do{
			uiStatus = Os__rxcar3(uiTimeOut1*100);
			//uiStatus = Os__rxcar(uiTimeOut1*100);
			if ( uiStatus /256 == OK)
				break;
		  }while(pxKey->gen_status==DRV_RUNNING&&uiTimeOut);
		if((pxKey->gen_status==DRV_ENDED))
		{
			if(pxKey->xxdata[1]==KEY_CLEAR) 
			{
				Os__timer_stop(&uiTimeOut);
				RunData.ulTotalAmount=0;
				return ERR_CANCEL;
			}
			else if(pxKey->xxdata[1]=='1') 
			{
				Os__timer_stop(&uiTimeOut);
				//Os__clr_display(255);
				//Os__GB2312_display(0, 0, (uchar *)"ֹ:");
				memset(buf,0,sizeof(buf));
				if( UTIL_Input(1,true,1,34,'N',buf,NULL) != KEY_ENTER )
				{
					uiTimeOut=60000;
					uiTimeOut1=1;
					uiI=0;
					//Os__clr_display(255);
					if(RunData.ulTotalAmount==0)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(unsigned char *)"ɨʵ");
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,(unsigned char *)"[ȡ]˳");
					}
					else
					{
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,"ܿ:",5);
						UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
						//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
						//Os__GB2312_display(3,0,aucBuf);
                        //for(i=0;i<RunData.uiBarNum;i++)
							//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);
					}
					Os__timer_start(&uiTimeOut);
					continue;
				}
				Os__abort_drv(drv_mmi);
				if(RunData.uiBarNum==4)
				{
					Os__com_flush2();
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(unsigned char *)"һβܳ");
					//Os__GB2312_display(1,0,(unsigned char *)"4ʵ");
					//Os__GB2312_display(3,0,(unsigned char *)"");
					MSG_WaitKey(20);
					//Os__clr_display(255);
					RunData.ucBcksp=1;
					return ERR_CANCEL;;
				}		
				
				*uiRevLen = strlen((char *)buf);
				memcpy(aucRevData,buf,*uiRevLen);
				return SUCCESS;
			}
			else if(pxKey->xxdata[1]==KEY_ENTER) 
			{
				Os__timer_stop(&uiTimeOut);
				return ERR_CANCEL;
			}				
			else if(pxKey->xxdata[1]==KEY_BCKSP) 
			{
				Os__timer_stop(&uiTimeOut);
				if(RunData.ulTotalAmount==0)
					continue;
				else
				{
					DataSave0.TransInfoData.auiTransIndex[RunData.uiTransIndex-1]=0;
					RunData.uiTransIndex--;
					DataSave0.TransInfoData.TransTotal.ulTotalBarNb--;
					DataSave0.TransInfoData.TransTotal.uiTotalNb--;
					ucResult=XDATA_Write_Vaild_File(DataSaveTransInfo);
					if(ucResult!=SUCCESS)
						return ucResult;
					RunData.uiBarNum--;
					RunData.ulTotalAmount-=RunData.ulNeedAmount[RunData.uiBarNum];
					RunData.ulTotalRate-=RunData.ulNeedRate[RunData.uiBarNum];
					RunData.ucBcksp=1;
					memset(aucBuf,0,sizeof(aucBuf));
					memcpy(aucBuf,"ܿ:",5);
					UTIL_Form_Montant(&aucBuf[5],RunData.ulTotalAmount,2);
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(unsigned char *)"ɨʵ");
					//Os__GB2312_display(2,0,(unsigned char *)"[1]ֹ");
					//Os__GB2312_display(3,0,aucBuf);
					for(i=0;i<RunData.uiBarNum;i++)
						//Os__GB2312_display(i,0,RunData.aucBarInfo[i]);
					return ERR_CANCEL;
				}
			}				
			else
				continue;
		}
		
		Os__abort_drv(drv_mmi);
		if(RunData.uiBarNum==4)
		{
			Os__com_flush2();
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"һβܳ");
			//Os__GB2312_display(1,0,(unsigned char *)"4ʵ");
			//Os__GB2312_display(3,0,(unsigned char *)"");
			MSG_WaitKey(20);

			break;
		}
		aucRevData[uiI++]=uiStatus % 256;   
//		Uart_Printf("Stx    %02x    %02x \n",uiStatus/256,uiStatus%256);
		break;
	}
	
	Os__timer_stop(&uiTimeOut);
	if(!uiTimeOut)
	{
		return ERR_COMMS_RECVTIMEOUT;
	}
//	Uart_Printf("Start Receiving Data\n");
	do
	{
		//uiStatus = Os__rxcar(uiTimeOut1*100);
		uiStatus = Os__rxcar3(uiTimeOut1*100);
		Os__xdelay(1);
//		for(i=0;i<10000;i++);
//		Uart_Printf("status%d    %02x    %02x\n",uiI,uiStatus/256,uiStatus%256);
		if(uiStatus/256!=OK) 
		{
			return(ERR_COMMS_RECVCHAR);
		}
		aucRevData[uiI]=uiStatus%256;
		if(aucRevData[uiI]==0x0D) break;
		uiI++;
	}while(1);
	*uiRevLen=uiI;
Uart_Printf("length= %d \n",uiI);
	
	return ucResult;
}

int CheckNumber(char* buffer,int buflen)
{
	int i;
	for(i=0;i<buflen;i++)
		if(buffer[i]>0x39||buffer[i]<0x30)return -1;
	return 0;
}
/*Уλɹ*/
int CheckBar_X25(char* buffer,int buflen)
{
	int index,sum=0;
	
	/*Ƿÿһλֵ*/
	if(CheckNumber(buffer,buflen) < 0)
	{
		Uart_Printf("зֵַ!");
		return -1;	
	}
	/*λ,һλУλ*/
	/*㹫ʽ:10-(λ*3+żλ)%10*/
	for(index = 0;index < buflen-1;index++)
	{
		if (0 == (index%2) )
			sum = sum + (buffer[index] - 0x30) * 3;
		else
			sum = sum + (buffer[index] - 0x30);
	}
	sum = sum % 10;
	sum = 10 - sum;
	Uart_Printf("sum=[%d]",sum);
	/*sum==10?sum=0:sum=sumжݵУλֵ*/
	if (sum == 10)	
		sum = 0;
	/*õУλʵʽУλбȽжǷȷ*/
	if ( sum == (buffer[buflen - 1] - 0x30) )
		return	0;
	else
		return	-1;
}

/*šͨƶУλɹ*/
int CheckBar_Normal(char* buffer,int buflen,int modType)
{
	char tmpBuf[10];
	int i,j=10,sum=0;
	
	/*
	memset(tmpBuf,0,sizeof(tmpBuf));
	memcpy(tmpBuf,buffer,buflen);
	*/
	
	for(i = 1;i < modType;i ++)
		j*=10;
	
	Uart_Printf("j=[%d],modType=[%d]",j,modType);
	
	/*УλǰֵӣҪȡһλͨλźƶ*/
	for(i = 0;i < buflen;i ++)
	{
		sum = sum + (buffer[i] - 0x30);
		Uart_Printf("i=[%d],sum=[%d]",i,sum);
		sum = sum % j;
		Uart_Printf("i=[%d],sum=[%d]",i,sum);
	}
	Uart_Printf("sum=[%d]",sum);	
	Uart_Printf("%c%c",buffer[i],buffer[i+1]);
	Uart_Printf("Уλ=[%d]",(int)asc_long(&buffer[i],modType));				
	if(sum != (int)asc_long(&buffer[i],modType))
		return -1;
	else
		return 0;
}

/*ҵշʵӦù淶ط׼DB31/T 291-2003и¼B  add by fantao 20051017*/	
int CheckBar_DB31(char* buffer,int buflen,int modType) 
{
	int Ci[35],Wi[35];
	int C,i,sum;
	
	Wi[31]=3;/*ʼֵΪ3*/
	Ci[31]=buffer[31] - 0x30;
	for(i=30;i>=0;i--)
	{
		Ci[i]=buffer[i] - 0x30;
		Wi[i]=Wi[i+1]*10-97*(int)(Wi[i+1]*10/97);
	}
	sum=0;
	for(i=0;i<buflen;i++)
	{
		sum+=Ci[i]*Wi[i];	
	}
	C=98-(sum-97*(int)(sum/97));
	if(C != (int)asc_long(&buffer[i],modType))
		return -1;
	else
		return 0;
}

/*ݿȡйصַȡֵصݣ˳ŵĿַвĿַĳ*/
int AnalyMyString(char *sSour,int iLen,char *sDest)
{
	int i;
	char *pStr1;
	char *pStr2;
	int iNow;
	int iStart;
	int iEnd;	
	
	pStr1 = sSour;
	pStr2 = sDest;
	for(i = 0; i < iLen; i++)
	{
		switch(*pStr1)
		{
			case ',':
				pStr1 += 1;
				break;
			case '-':		
				iStart = (int)asc_long(pStr1 - 2,2);
				pStr1 += 1;
				iEnd   = (int)asc_long(pStr1 ,2);
				
				iNow = iStart + 1;
				while(iNow < iEnd)
				{
					int_asc(pStr2,2,&iNow);
					pStr2 += 2;
					iNow += 1;
				}
				
				break;
			default:
				*pStr2 = *pStr1;
				pStr2 += 1;
				pStr1 += 1;
				break;
		}		
	}	
	
	return pStr2 - sDest;
}

/*֧ݺϷƫ*/
int GetBillYearsBound(int *iUBound,int *iDBound)
{
//	union INFOCFG getinfo;
    *iUBound = 4;
    
    *iDBound = 4;
    
    Uart_Printf("ƫ:[%d]ƫ[%d]",*iUBound,*iDBound);
    
	return 0;
}
int CheckBillField234(char *sCode,char *sField,int iLenField)
{
	int iFieldCodeLen;
	char sFieldCode[250];
	int iRet;
	int i;
	
	memset(sFieldCode,0,sizeof(sFieldCode));
	
	/*˹̳򷵻صַΪ*/
	if((iFieldCodeLen = AnalyMyString(sField,iLenField,sFieldCode)) <= 0)
	{
		return -1;	
	}
	
	iRet = -1;
	for(i = 0;i < iFieldCodeLen;i += 2)
	{
		Uart_Printf("sCode=[%2.2s] sFieldCode=[%2.2s]",sCode,&sFieldCode[i]);
		if(memcmp(sCode,&sFieldCode[i],2) == 0)
		{
			iRet = 0;
			break;
		}		
	}	
	
	return iRet;
}

int Scan_BarAnalyze(char *pBarCode ,  CDBBillInfo *pBillInfo)
{
	char *pStr;	/*õָ*/
	int iLen;	/*볤ȣУλǰݵܳ*/
	int i,j;
	int iMapLen;	/*λͼ*/
	int iBarLen;	/*볤ȣУλݵܳ*/
	int iSum;
	char sYear[5];
	char sMonth[3];
	char sDate[7];
	ulong lAmount;/*׽add by fantao 20051013*/
	int iYearUBound , iYearDBound;
	char *pStr1;/*ϹĽݵָadd by fantao 20051013*/
	
	/*̽ʱõʱ,ֻ̽ɹŽʱֵ͵ͨýṹӦֶ*/
	char tmpContractNo[31];
	char tmpContractName[21];
	char tmpAccDate[7];
	char tmpTakeNum[3];
	unsigned char   tmpAmount[11];
	
	/*ʼʱ*/
	memset(tmpContractNo,0,sizeof(tmpContractNo));
	memset(tmpContractName,0,sizeof(tmpContractName));
	memset(tmpAccDate,0,sizeof(tmpAccDate));
	memset(tmpTakeNum,0,sizeof(tmpTakeNum));
	memset(tmpAmount,0,sizeof(tmpAmount));
	Uart_Printf("\n[%s] ",pBillInfo->bill_name);
		
	/*˵źλͼʵʳ*/
	pStr = pBarCode;
	iBarLen=strlen(pStr);
	/*УĳǷȷ*/
	if (iBarLen != pBillInfo->bar_len)
	{
		return -1;
	}
	/*ýλͼϢ*/
	iMapLen = strlen(pBillInfo->field_map);
	
	/*жǷãͬšڴΡΡ*/
	/*ǰĳʼڶN͵ȫ0͵ȫո񡣶ڱ䳤Ķ'\0'*/
	/*λͼ*/
	for(i = 0;i < iMapLen; i++)
	{
		switch(pBillInfo->field_map[i])
		{
			case '1':  //鹫˾еĹ˾Ƿͬ
				if(memcmp(pStr,pBillInfo->field1,2) != 0)
				{
					Uart_Printf("˾[%2.2s] ",pStr);
					return -1;
				}
				
				pStr += 2;
				break;
			case '2':  // еǷͬ
				iLen = strlen(pBillInfo->field2);				
				if(CheckBillField234(pStr,pBillInfo->field2,iLen) < 0)
				{
					Uart_Printf("[%2.2s] ",pStr);
					return -1;
				}
				
				pStr += 2;
				break;
			case '3':  //еĴǷͬ
				iLen = strlen(pBillInfo->field3);
				if(CheckBillField234(pStr,pBillInfo->field3,iLen) < 0)
				{
					Uart_Printf("[%2.2s] ",pStr);
					return -1;
				}
				
				pStr += 2;				
				break;
			case '4':  //鵥λеĵλǷͬ
				iLen = strlen(pBillInfo->field4);
				if(CheckBillField234(pStr,pBillInfo->field4,iLen) < 0)
				{
					Uart_Printf("λ[%2.2s] ",pStr);
					return -1;
				}
				
				pStr += 2;				
				break;
			case '5':  /*ͬųȣʺšûššţ*/
				memcpy(tmpContractNo,pStr+pBillInfo->field5_off,pBillInfo->field5);
				memcpy(tmpContractName,pBillInfo->field5_name,strlen(pBillInfo->field5_name));
				pStr += pBillInfo->field5_off;
				pStr += pBillInfo->field5;
				break;
			case '6':  /*  */
				memset(sYear,0,sizeof(sYear));
				/*һЩ˵еĳȲ124ͣʱҪλ*/
				bcd_asc(sDate,NormalTransData.aucDateTime,6);
				if (pBillInfo->field6<4)
					memcpy(sYear,sDate,4-pBillInfo->field6);
				memcpy(sYear+4-pBillInfo->field6,pStr,pBillInfo->field6);
				 
				if(sDate[3]=='0'&&sYear[3]=='9'&&sDate[2]>'0'&&sDate[2]==sYear[2])
				{
					sYear[2]--;
				}
				asc_bcd(&NormalTransData.aucPayDate[0],1,&sYear[2],2);
				 
				Uart_Printf("sYear=[%s]",sYear);
				/*ǷʣsDateеĵǰж*/
				/*add by gyc 20050518ӻƫƵĺ*/
				if(GetBillYearsBound(&iYearUBound , &iYearDBound) < 0)
				{
					Uart_Printf("˵Ϸƫݳ.");
					return -1;
				}
				memset(sDate+4,0,sizeof(sDate)-4);
				Uart_Printf("ƫ:[%d+%d],[%d-%d]",atoi(sDate),iYearUBound,atoi(sDate),iYearDBound);
				/*modi by gyc 20050518 YEARDELAYEĳiYearDBound,YEARAHEADĳiYearUBound*/
				if ((atoi(sYear)<(atoi(sDate) - iYearDBound)) || (atoi(sYear)>(atoi(sDate) + iYearUBound)))
				{
					Uart_Printf("ݲsYear=[%s]",sYear);
					Uart_Printf("ӦڷΧ[%d]-[%d]",atoi(sDate) - iYearDBound,atoi(sDate) + iYearUBound);
					return -1;
				}
				pStr += pBillInfo->field6;
				break;
			case '7':  /*·  */
				memset(sMonth,0,sizeof(sMonth));
				memcpy(sMonth,pStr,pBillInfo->field7);
				Uart_Printf("sMonth=[%s]",sMonth);
				 
				asc_bcd(&NormalTransData.aucPayDate[1],1,sMonth,pBillInfo->field7);
				 
				if(    memcmp(pBillInfo->commpany_code,"888880001102900",15)==0  
				    || memcmp(pBillInfo->commpany_code,"888880001302900",15)==0	)
				{
				    if(  (atoi(sMonth)>=1 && atoi(sMonth)<= 12)
				       ||(atoi(sMonth)>=21 && atoi(sMonth)<= 32) 
				       ||(atoi(sMonth)>=41 && atoi(sMonth)<= 52)  )
				    {
						/*ϳֶθʽ*/
						memcpy(tmpAccDate,sYear,4);
						memcpy(tmpAccDate+4,sMonth,2);
						pStr += pBillInfo->field7;				    	
				    }	
				    else
				    {
						Uart_Printf("·ݲϷsMonth=[%s]",sMonth);
						return -1;				    	
				    }			 
				}
				else
				{
					if (atoi(sMonth)>12 ||atoi(sMonth)<1)
					{
						Uart_Printf("·ݲϷsMonth=[%s]",sMonth);
						return -1;
					}
					/*ϳֶθʽ*/
					memcpy(tmpAccDate,sYear,4);
					memcpy(tmpAccDate+4,sMonth,2);
					pStr += pBillInfo->field7;
			    }
			    break;
			case '8':  /*  */
				memset(tmpTakeNum,'0',sizeof(tmpTakeNum)-1);
				memcpy(tmpTakeNum+2-pBillInfo->field8,pStr,pBillInfo->field8);
				pStr += pBillInfo->field8;
				break;
			case '9':  /*ʵ */
				pStr += pBillInfo->field9;
				break;
			case 'A':  /*  */
			case 'a':
				memcpy(tmpAmount,pStr,pBillInfo->fieldA); 
				pStr += pBillInfo->fieldA;		
				break;
			case 'B':
			case 'b':	
				pStr += pBillInfo->fieldB;								
				break;			
			case 'C':
			case 'c':				
				break;		
			case 'D':
			case 'd':				
				break;		
			case 'E':
			case 'e':				
				break;		
			case 'F':
			case 'f':  /*Уλ */		
				switch(pBillInfo->check_type)
				{
					case 0: /* 25  */
						Uart_Printf("iBarLen=[%d],pBillInfo->fieldF=[%d]",iBarLen,pBillInfo->fieldF);
						if(CheckBar_X25(pBarCode,iBarLen) < 0)
						{
							Uart_Printf("CheckBar_X25 error[%d]",iBarLen);
							return -1;	
						}		
						break;
					case 1: /* ƶ ͨ*/
						iLen = pStr - pBarCode;
						Uart_Printf("iLen=[%d],pBillInfo->fieldF=[%d]",iLen,pBillInfo->fieldF);
						if(CheckBar_Normal(pBarCode,iLen,pBillInfo->fieldF) < 0)
						{
							Uart_Printf("CheckBar_Normal error[%d]",iLen);
							return -1;	
						}
						break;
					case 2:/*ҵշʵӦù淶ط׼DB31/T 291-2003и¼B  add by fantao 20051017*/	
						iLen = pStr - pBarCode;
						Uart_Printf("iLen=[%d],pBillInfo->fieldF=[%d]",iLen,pBillInfo->fieldF);
						if(CheckBar_DB31(pBarCode,iLen,pBillInfo->fieldF) < 0)
						{
							Uart_Printf("CheckBar_DB31 error[%d]",iLen);
							return -1;	
						}
						break;
				}		
				pStr += pBillInfo->fieldF;	
				break;	
		}
	}
	
	/*һʺ,ʺϹݱ浽ͨý׽ṹӦֶ*/
	Uart_Printf("һԽ,ڸͨýṹ帳ֵ.");
	/*õǰĹеĳ˻ϢֵͨýṹsCompanyCode,sCompanyName*/
	/*˻AN15 Ҳո ûдȫдո*/
	memcpy(PaymentTrans.BarCodeInfo.aucOrganCode,pBillInfo->commpany_code,strlen(pBillInfo->commpany_code));
	memcpy(PaymentTrans.BarCodeInfo.aucOrganName,pBillInfo->bill_name,strlen(pBillInfo->bill_name));
	Uart_Printf("˻[%s]",pBillInfo->commpany_code);
	Uart_Printf("˻[%s]",pBillInfo->bill_name);
	/*add by gyc 20050408ӻȡɷ*/
	PaymentTrans.ucPayFeeType = pBillInfo->bill_kind;
	/*ԭҪָ֣ڲ֣ͳһϺ*/
	/*˻*/
	if(memcmp(pBillInfo->commpany_code,"888880000502900",15) == 0)
	{
		PaymentTrans.ucAccType= 1;
	}
	else
	{
		PaymentTrans.ucAccType = pBillInfo->bill_type;
	}
	//Uart_Printf("ɷ[%d],ɷʹ[%d]",pTransData->ibillkind,pTransData->ibilltype);
	/*ͬ ͬ*/
	/*ͬAN30󲹿ո  Ʊ䳤*/
	//memcpy(PaymentTrans.aucOrderNum,tmpContractNo,strlen(tmpContractNo));
	//memcpy(pTransData->sContractName,tmpContractName,strlen(tmpContractName));
	Uart_Printf("ͬ[%s]",tmpContractNo);
	Uart_Printf("ͬ[%s]",tmpContractName);
	/**/
	/*YYYYMMɣҲոûдȫո*/
	memcpy(PaymentTrans.BarCodeInfo.aucAccountPeriod,tmpAccDate,strlen(tmpAccDate));
	Uart_Printf("[%s]",tmpAccDate);
	/**/
	/*N2ҿ0ûдȫ0*/
	memcpy(PaymentTrans.BarCodeInfo.aucCopyTimes,tmpTakeNum,strlen(tmpTakeNum));
	Uart_Printf("[%s]",tmpTakeNum);
	/**/
	/*N10ҿ㣬ûдȫ0*/
	memcpy(PaymentTrans.BarCodeInfo.aucAmount,tmpAmount,11); 
	if(!memcmp(NormalTransData.aucPayType,(uchar*)"\x01\x07",2))
		PaymentTrans.BarCodeInfo.aucAmount[strlen((char*)tmpAmount)]='0';
	Uart_Printf("[%s]",tmpAmount);
	
	Uart_Printf("ֵ.");
		
	return 0;
}
unsigned char Scan_FormatSearch(int index)
{
	memset(&oBillInfo,0,sizeof(CDBBillInfo));
	switch(index)
	{
		case 0:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"бˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002302900",15);
			memcpy(oBillInfo.field_map,(uchar *)"25678AF",7);
			//memcpy(oBillInfo.field1,);
			memcpy(oBillInfo.field2,(uchar *)"27-31",5);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=9;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=1;
			oBillInfo.field9=0;
			oBillInfo.fieldA=9;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"˺",4);
			oBillInfo.field5_off=-2;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x06",4);
			break;
		case 1:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=2;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002102900",15);
			memcpy(oBillInfo.field_map,(uchar *)"12567AF",7);
			memcpy(oBillInfo.field1,(uchar *)"06",2);
			memcpy(oBillInfo.field2,(uchar *)"20-26",5);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=6;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=0;
			oBillInfo.fieldA=9;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"˺",4);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x02",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x07",4);
			break;
		case 2:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=3;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002702900",15);
			memcpy(oBillInfo.field_map,(uchar *)"25867AF",7);
			//memcpy(oBillInfo.field1,(uchar *)"06",2);
			memcpy(oBillInfo.field2,(uchar *)"05,07,08",8);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=7;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=1;
			oBillInfo.field9=0;
			oBillInfo.fieldA=9;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"˺",4);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x03",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x09",4);
			break;
		case 3:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=4;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ֶˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002502900",15);
			memcpy(oBillInfo.field_map,(uchar *)"25678AF",7);
			//memcpy(oBillInfo.field1,);
			memcpy(oBillInfo.field2,(uchar *)"94,95,96,98",11);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=9;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=1;
			oBillInfo.field9=0;
			oBillInfo.fieldA=9;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"",6);
			oBillInfo.field5_off=-2;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x04",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x06",4);
			break;
		case 4:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=5;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ɽˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002202900",15);
			memcpy(oBillInfo.field_map,(uchar *)"1675AF",6);
			memcpy(oBillInfo.field1,(uchar *)"11",2);
			//memcpy(oBillInfo.field2,(uchar *)"05,07,08",5);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=8;
			oBillInfo.field6=1;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=0;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"û",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x05",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x07",4);
			break;
		case 5:
			oBillInfo.bill_kind=2;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880000502900",15);
			memcpy(oBillInfo.field_map,(uchar *)"4675AF",6);
			//memcpy(oBillInfo.field1,(uchar *)"11",2);
			//memcpy(oBillInfo.field2,(uchar *)"05,07,08",5);
			//memcpy(oBillInfo.field3,);
			memcpy(oBillInfo.field4,(uchar *)"68,51,52,53,54,55,56,57,58,59,60,61,62,63,65,66,67",50);
			oBillInfo.field5=10;
			oBillInfo.field6=1;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=0;
			oBillInfo.fieldA=8;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"",4);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x02\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x03",4);
			break;
		case 6:
			oBillInfo.bill_kind=3;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880001102900",15);
			memcpy(oBillInfo.field_map,(uchar *)"13675AF",7);
			memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"05,07,08",5);
			memcpy(oBillInfo.field3,(uchar *)"39,41-46,53,55,58-61,69,71-76,83,85,88-91",41);
			//memcpy(oBillInfo.field4,(uchar *)"68,51,52,53,54,55,56,57,58,59,60,61,62,63,65,66,67",50);
			oBillInfo.field5=6;
			oBillInfo.field6=4;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=0;
			oBillInfo.fieldA=7;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"",4);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x03\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x05",4);
			break;
		case 7:
			oBillInfo.bill_kind=3;
			oBillInfo.bill_type=2;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"б",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880001302900",15);
			memcpy(oBillInfo.field_map,(uchar *)"13675AF",7);
			memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"05,07,08",5);
			memcpy(oBillInfo.field3,(uchar *)"35,37,38,40,47-52,54,56,57,65,67,68,70,77-82,84,86,87",53);
			//memcpy(oBillInfo.field4,(uchar *)"68,51,52,53,54,55,56,57,58,59,60,61,62,63,65,66,67",50);
			oBillInfo.field5=6;
			oBillInfo.field6=4;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=0;
			oBillInfo.fieldA=7;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"",4);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x03\x02",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x05",4);
			break;
		case 8:
			oBillInfo.bill_kind=3;
			oBillInfo.bill_type=3;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ֶ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880001502900",15);
			memcpy(oBillInfo.field_map,(uchar *)"25679AF",7);
			//memcpy(oBillInfo.field1,(uchar *)"03",2);
			memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			//memcpy(oBillInfo.field4,(uchar *)"68,51,52,53,54,55,56,57,58,59,60,61,62,63,65,66,67",50);
			oBillInfo.field5=9;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=1;
			oBillInfo.fieldA=7;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"",6);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x03\x03",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x10",4);
			break;
		case 9:
			oBillInfo.bill_kind=4;
			oBillInfo.bill_type=2;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003102900",15);
			memcpy(oBillInfo.field_map,(uchar *)"B9FA",4);
			//memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			//memcpy(oBillInfo.field4,(uchar *)"68,51,52,53,54,55,56,57,58,59,60,61,62,63,65,66,67",50);
			oBillInfo.field5=0;
			oBillInfo.field6=0;
			oBillInfo.field7=0;
			oBillInfo.field8=0;
			oBillInfo.field9=1;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=11;
			oBillInfo.fieldF=2;
			oBillInfo.check_type=1;
			memcpy(oBillInfo.field5_name,(uchar *)"绰",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x04\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x02",4);
			break;
		case 10:
			oBillInfo.bill_kind=4;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=34;
			memcpy(oBillInfo.bill_name,(uchar *)"  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003102900",15);
			memcpy(oBillInfo.field_map,(uchar *)"145679AF",8);
			memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			memcpy(oBillInfo.field4,(uchar *)"10",2);
			oBillInfo.field5=12;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=2;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=2;
			oBillInfo.check_type=2;
			memcpy(oBillInfo.field5_name,(uchar *)"绰",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x04\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x02",4);
			break;
		case 11:
			oBillInfo.bill_kind=5;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=30;
			memcpy(oBillInfo.bill_name,(uchar *)"ƶ  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003302900",15);
			memcpy(oBillInfo.field_map,(uchar *)"B567AF",6);
			//memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			//memcpy(oBillInfo.field4,(uchar *)"10",2);
			oBillInfo.field5=11;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=0;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=3;
			oBillInfo.fieldF=2;
			oBillInfo.check_type=1;
			memcpy(oBillInfo.field5_name,(uchar *)"˺",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x05\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x01",4);
			break;
		case 12:
			oBillInfo.bill_kind=6;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ͨ  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003202900",15);
			memcpy(oBillInfo.field_map,(uchar *)"9675AF",6);
			//memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			//memcpy(oBillInfo.field4,(uchar *)"10",2);
			oBillInfo.field5=8;
			oBillInfo.field6=4;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=1;
			oBillInfo.fieldA=8;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=1;
			memcpy(oBillInfo.field5_name,(uchar *)"˺",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x06\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x04",4);
			break;
		case 13:
			oBillInfo.bill_kind=7;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=34;
			memcpy(oBillInfo.bill_name,(uchar *)"ͨ  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003602900",15);
			memcpy(oBillInfo.field_map,(uchar *)"145679AF",8);
			memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			memcpy(oBillInfo.field4,(uchar *)"50",2);
			oBillInfo.field5=12;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=2;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=2;
			oBillInfo.check_type=2;
			memcpy(oBillInfo.field5_name,(uchar *)"û˺",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x07\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x12",4);
			break;
		case 14:
			oBillInfo.bill_kind=8;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=34;
			memcpy(oBillInfo.bill_name,(uchar *)"ͨ  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003702900",15);
			memcpy(oBillInfo.field_map,(uchar *)"145679AF",8);
			memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			memcpy(oBillInfo.field4,(uchar *)"60",2);
			oBillInfo.field5=12;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=2;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=2;
			oBillInfo.check_type=2;
			memcpy(oBillInfo.field5_name,(uchar *)"û˺",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x08\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x13",4);
			break;
		case 15:
			oBillInfo.bill_kind=9;
			oBillInfo.bill_type=1;
			oBillInfo.bar_len=34;
			memcpy(oBillInfo.bill_name,(uchar *)"Ѷ  ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880003802900",15);
			memcpy(oBillInfo.field_map,(uchar *)"145679AF",8);
			memcpy(oBillInfo.field1,(uchar *)"03",2);
			//memcpy(oBillInfo.field2,(uchar *)"92",2);
			//memcpy(oBillInfo.field3,(uchar *)"37,38,40,47-52,54,56,57,67,68,70,77-82,84,86,87",47);
			memcpy(oBillInfo.field4,(uchar *)"70",2);
			oBillInfo.field5=12;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			oBillInfo.field8=0;
			oBillInfo.field9=2;
			oBillInfo.fieldA=10;
			oBillInfo.fieldB=0;
			oBillInfo.fieldF=2;
			oBillInfo.check_type=2;
			memcpy(oBillInfo.field5_name,(uchar *)"û˺",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x09\x01",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x13",4);
			break;
		case 16:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=6;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002802900",15);
			memcpy(oBillInfo.field_map,(uchar *)"675AF",5);
			//memcpy(oBillInfo.field1,);
			//memcpy(oBillInfo.field2,(uchar *)"27-31",5);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=11;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			//oBillInfo.field8=1;
			//oBillInfo.field9=0;
			oBillInfo.fieldA=8;
			//oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"û",8);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x06",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x11",4);
			break;
		case 17:
			oBillInfo.bill_kind=1;
			oBillInfo.bill_type=7;
			oBillInfo.bar_len=24;
			memcpy(oBillInfo.bill_name,(uchar *)"ζˮ",6);
			memcpy(oBillInfo.commpany_code,(uchar *)"888880002902900",15);
			memcpy(oBillInfo.field_map,(uchar *)"567AF",7);
			//memcpy(oBillInfo.field1,(uchar *)"06",2);
			//memcpy(oBillInfo.field2,(uchar *)"20-26",5);
			//memcpy(oBillInfo.field3,);
			//memcpy(oBillInfo.field4,);
			oBillInfo.field5=12;
			oBillInfo.field6=2;
			oBillInfo.field7=2;
			//oBillInfo.field8=0;
			//oBillInfo.field9=0;
			oBillInfo.fieldA=7;
			//oBillInfo.fieldB=0;
			oBillInfo.fieldF=1;
			oBillInfo.check_type=0;
			memcpy(oBillInfo.field5_name,(uchar *)"ͳֺ",6);
			oBillInfo.field5_off=0;
			//
			memcpy(NormalTransData.aucPayType,(uchar*)"\x01\x07",2);
			memcpy(NormalTransData.aucBank,"\x00\x00\x00\x16",4);
			break;
		default:
			return ERR_TRANS_CARDTYPE;
	}
	return SUCCESS;
}

unsigned char Scan_ShowBarCodeInfo(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucKey;
	unsigned char aucPrintBuf[17];
	unsigned long ulAmount;
	
	
	//Os__clr_display(255);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,PaymentTrans.BarCodeInfo.aucOrganName,16);
	//Os__GB2312_display(0,0,(unsigned char *)aucPrintBuf);
	
	
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,PaymentTrans.BarCodeInfo.aucAccountPeriod,6);
	//Os__display(1,0,(unsigned char *)aucPrintBuf);
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,":",5);
	ulAmount = asc_long(PaymentTrans.BarCodeInfo.aucAmount,strlen((char*)PaymentTrans.BarCodeInfo.aucAmount));
	UTIL_Form_Montant(&aucPrintBuf[5],ulAmount,2);
	NORMALTRANS_ulAmount=ulAmount;
	aucPrintBuf[16] = 0;
	//Os__GB2312_display(1,0,(unsigned char *)aucPrintBuf);
	
	//Os__GB2312_display(2,0,(unsigned char *)"[ȷ]--");
	//Os__GB2312_display(3,0,(unsigned char *)"[ȡ]--˳");
	Os__abort_drv(drv_mmi);
	while(1)
	{
		ucKey = Os__xget_key();
		if( (ucKey == KEY_ENTER) || (ucKey == KEY_CLEAR) )
		{
			break;
		}
	}
	switch(ucKey)
	{
		case KEY_ENTER:
			//NormalTransData.ulAmount=ulAmount;
			ucResult = SUCCESS;
			memcpy(NormalTransData.aucBarName,PaymentTrans.BarCodeInfo.aucOrganName,14);
			break;
		case KEY_CLEAR:
			ucResult = ERR_CANCEL;
			break;
	}
	return ucResult;
}

