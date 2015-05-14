#include <include.h>
#include <global.h>
#include <xdata.h>
#include <smart.h>
#include <osmifare.h>
#include "new_drv.h"

unsigned int Untouch_OWE_RstErrInfo(int ret)
{
	if(ret == KEY_DRV_ERROR)                                            
	{                                                                   
		printf("\n OWE_ERROR: open KER DRV ERR.");//ʧܴ;                                           
	}                                                                   
	else if(ret == MAG_DRV_ERROR)                                       
	{                                                                   
		printf("\n OWE_ERROR: open MAG DRV ERR.");//ſʧܴ;                                           
	}                                                                   
	else if(ret == MFR_DRV_ERROR)                                       
	{                                                                   
		printf("\n OWE_ERROR: open MFC DRV ERR.");//ǽӿʧܴ;                                         
	}                                                                   
	else if(ret == ICC_DRV_ERROR)                                       
	{                                                                   
		printf("\n OWE_ERROR: open ICC DRV ERR.");//ICʧܴ;                                           
	}                                                                   
	else                                                                
	{                                                                   
		printf("\n OWE_ERROR: other ERR.");//;                                                   
	}                                                                   
	
	return 	 ret;
}

unsigned char Untouch_Check_Err(unsigned char ucData)
{
	RunData.ucNeedRedo=0;
	switch(ucData)
	{
		case 0x00:
			return SUCCESS;
		case 0x04:
		case 0x0D:
			RunData.ucNeedRedo=1;
			return ucData;
		case 0x07:
			return ERR_PACKERROR;
		case 0x08:
			return ERR_DEVALCARD;
		case 0x0C:
			return ERR_NOTINSYS;
		case 0x11:
			return ERR_KEYVERIFY;
		case 0xff:
			return ERR_PINPADVERSION;
		case 0x10:
			RunData.ucNeedRedo=1;
			return ERR_READWRITECARD;
		default:
			return ERR_READWRITECARD;
	}
	
	return SUCCESS;
}
unsigned char Untouch_ReadCardNo_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	
	DrvIn.type      = 0x18;
	DrvIn.length	= 6;
	Uart_Printf("\nSend 18\n");
	memcpy(DrvIn.xxdata,"\xA0\xA1\xA2\xA3\xA4\xA5",6);
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is xiaoshu PinPad Receive Data [ DrvOut.xxdata ]\n");
	for(i = 0; i < sizeof(M1TransData)+1; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	
	if( ucResult == SUCCESS)
	{
		*ucStatus = DrvOut.xxdata[0];
		*ucOutDataLength = sizeof(M1TransData); 
		memcpy(pucOutData,&DrvOut.xxdata[1],sizeof(M1TransData));
		if(!memcmp(pucOutData,"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",10))
			*ucStatus=0x04;
		ucResult=Untouch_Check_Err(*ucStatus);
		
	}
	return(ucResult);
}


unsigned char Untouch_ReadPie2_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9],ucKeyArray;
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x19;
	DrvIn.length	= 16;
	
	memcpy(DrvIn.xxdata,RunData.aucKeyA,6);
	memcpy(&DrvIn.xxdata[6],RunData.aucPurchaseKey,6);
	switch(NormalTransData.ucPtCode)
	{
		case 0x01:
			if(RunData.ucKeyArrey)
				ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
			else
				ucKeyArray=KEYARRAY_PURSECARD1;
			break;
		case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
		case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
		default:break;
	}
	DrvIn.xxdata[12]=ucKeyArray;
	DrvIn.xxdata[13]=KEYINDEX_PURSECARD1_LOADMACKEY;
	DrvIn.xxdata[14]=ucKeyArray;
	DrvIn.xxdata[15]=KEYINDEX_PURSECARD1_LOADMACKEY+1;
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 19 send data\n");
	for(i = 0; i < 16; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 19 recieve data\n");
	for(i = 0; i < sizeof(M1TransData1)+1; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength = sizeof(M1TransData1); 
			memcpy(pucOutData,&DrvOut.xxdata[1],sizeof(M1TransData1));
			if(*ucStatus==0&&
				!memcmp(&pucOutData[1],"\x00\x00\x00\x00",4))
				*ucStatus=0x04;
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}

unsigned char Untouch_Purchase_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	unsigned long ulamount;	
	if(RunData.aucFunFlag[1])
		ulamount=min(NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount);
	else
		ulamount=min(NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance);
	DrvIn.type      = 0x1A;
	DrvIn.length	= 21;
	Uart_Printf("\nSend 1A\n");
	memcpy(&DrvIn.xxdata[0],NormalTransData.aucDateTime,7);
	long_tab(&DrvIn.xxdata[7],4,&ulamount);

	if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
		memcpy(&DrvIn.xxdata[11],&NormalTransData.aucPsamTerminal[2],4);
	else
		asc_bcd(&DrvIn.xxdata[11],4,DataSave0.ConstantParamData.aucOnlytermcode,8);

	long_bcd(&DrvIn.xxdata[15],4,&NormalTransData.ulTraceNumber);
	DrvIn.xxdata[19]=0x21;
	if(NormalTransData.ucReturnflag)
		DrvIn.xxdata[20]=0x03;
	else
		DrvIn.xxdata[20]=0x01;
{
unsigned char  i;
Uart_Printf("\n***PINPADѵ:*****\n");
for(i = 0; i < 21; i++)
{
Uart_Printf("%02x ",DrvIn.xxdata[i]);
}
Uart_Printf("\n");
}
	
	ucResult = Untouch_SendReceive_tmp();
{
unsigned char  i;
Uart_Printf("\n***PINPADѷص:*****\n");
for(i = 0; i < 26; i++)
{
Uart_Printf("%02x ",DrvOut.xxdata[i]);
}
Uart_Printf("\n");
}
	
	if( ucResult == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			if(( NULL != pucOutData)&&( NULL != ucStatus))
			{
				*ucStatus = DrvOut.xxdata[0];
				*ucOutDataLength = 26; 
				memcpy(pucOutData,&DrvOut.xxdata[1],26);
				if(!memcmp(DrvOut.xxdata,"\x00\x00\x00\x00\x00",5))
					*ucStatus=0x04;
			}
			else	
				return (ERR_READMEMORY);
		
		}		
		
	}
	else
		return ucResult;
	ucResult=Untouch_Check_Err(*ucStatus);
	
	return(ucResult);
}


unsigned char Untouch_ReadPierp_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9],ucKeyArray;
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x19;
	DrvIn.length	= 16;
	
	memcpy(DrvIn.xxdata,RunData.aucKeyA,6);
	memcpy(&DrvIn.xxdata[6],RunData.aucPurchaseKey,6);
	switch(NormalTransData.ucPtCode)
	{
		case 0x01:
			if(RunData.ucKeyArrey)
				ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
			else
				ucKeyArray=KEYARRAY_PURSECARD1;
			break;
		case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
		case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
		default:break;
	}
	DrvIn.xxdata[12]=ucKeyArray;
	DrvIn.xxdata[13]=KEYINDEX_PURSECARD1_LOADMACKEY;
	DrvIn.xxdata[14]=ucKeyArray;
	DrvIn.xxdata[15]=KEYINDEX_PURSECARD1_LOADMACKEY+1;
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 19 send data\n");
	for(i = 0; i < 16; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_rp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 19 recieve data\n");
	for(i = 0; i < sizeof(M1TransData1)+1; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength = sizeof(M1TransData1); 
			memcpy(pucOutData,&DrvOut.xxdata[1],sizeof(M1TransData1));
			if(*ucStatus==0&&
				!memcmp(&pucOutData[12],"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",10))
				*ucStatus=0x04;
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}

unsigned char Untouch_Load_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	unsigned long ulamount;	
	DrvIn.type      = 0x1B;
	DrvIn.length	= 56;
	Uart_Printf("\nSend 1B\n");
	memcpy(&DrvIn.xxdata[0],NormalTransData.aucDateTime,7);
	long_tab(&DrvIn.xxdata[7],4,&NormalTransData.ulAmount);

	if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
		memcpy(&DrvIn.xxdata[11],&NormalTransData.aucPsamTerminal[2],4);
	else
		asc_bcd(&DrvIn.xxdata[11],4,DataSave0.ConstantParamData.aucOnlytermcode,8);

	long_bcd(&DrvIn.xxdata[15],4,&NormalTransData.ulTraceNumber);
	DrvIn.xxdata[19]=0x62;
	memcpy(&DrvIn.xxdata[20],NormalTransData.aucExpiredDate,4);
	asc_bcd(&DrvIn.xxdata[24],8,NormalTransData.aucLoadTrace,16);
	memcpy(&DrvIn.xxdata[32],RunData.aucLoadKey,6);
	asc_bcd(&DrvIn.xxdata[39],3,DataSave0.ChangeParamData.aucCashierNo,6);
	memcpy(&DrvIn.xxdata[42],RunData.aucTotalLoad,4);
	memcpy(&DrvIn.xxdata[46],RunData.aucLoadMac,4);
	memcpy(&DrvIn.xxdata[50],RunData.aucWriteKey1,6);
{
unsigned char  i;
Uart_Printf("\n***PINPADֵ:*****\n");
for(i = 0; i < 57; i++)
{
Uart_Printf("%02x ",DrvIn.xxdata[i]);
}
Uart_Printf("\n");
}
	
	ucResult = Untouch_SendReceive_tmp();
{
unsigned char  i;
Uart_Printf("\n***PINPADֵص:*****\n");
for(i = 0; i < 26; i++)
{
Uart_Printf("%02x ",DrvOut.xxdata[i]);
}
Uart_Printf("\n");
}
	
	if( ucResult == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			if(( NULL != pucOutData)&&( NULL != ucStatus))
			{
				*ucStatus = DrvOut.xxdata[0];
				*ucOutDataLength = 26; 
				memcpy(pucOutData,&DrvOut.xxdata[1],26);
				if(!memcmp(DrvOut.xxdata,"\x00\x00\x00\x00\x00",5))
					*ucStatus=0x04;
			}
			else	
				return (ERR_READMEMORY);
		
		}		
		
	}
	else
		return ucResult;
	ucResult=Untouch_Check_Err(*ucStatus);
	
	return(ucResult);
}

unsigned char Untouch_GetTAC(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	
	DrvIn.type      = 0x10;
	DrvIn.length	= 21;
	DrvIn.xxdata[0]=0x01;
	long_tab(&DrvIn.xxdata[1],4,&NormalTransData.ulAmount);
	DrvIn.xxdata[5]=0x21;
	if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
		memcpy(&DrvIn.xxdata[6],&NormalTransData.aucPsamTerminal[2],4);
	else
		asc_bcd(&DrvIn.xxdata[6],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
	long_bcd(&DrvIn.xxdata[10],4,&NormalTransData.ulTraceNumber);
	memcpy(&DrvIn.xxdata[14],NormalTransData.aucDateTime,7);
	{
	unsigned char  i;
	Uart_Printf("\n***********TACjin:***********\n");
	for(i = 0; i <22; i++)
	{
	Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
	Uart_Printf("\n");
	}
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\n***********TACout:***********\n");
	for(i = 0; i < 5; i++)
	{
	Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
	Uart_Printf("\n");
	}
	if( ucResult == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			if(( NULL != pucOutData)&&( NULL != ucStatus))
			{
				*ucStatus = DrvOut.xxdata[0];
				*ucOutDataLength = 8; 
				memcpy(pucOutData,&DrvOut.xxdata[1],8);
				if(!memcmp(pucOutData,"\x00\x00\x00\x00",4))
					*ucStatus=0x04;
			}
			else	
				return (ERR_READMEMORY);
		
		}		
		
	}
	else
		return ucResult;
	ucResult=Untouch_Check_Err(*ucStatus);
	
	return(ucResult);
}

unsigned char Untouch_ReadSierial_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	
	MIFARE_WaitCard();
	return (MIFARE_ucOutStatus);
	
	if(NormalTransData.ucCardType==CARD_CPU)
		DrvIn.type      = 0x6F;
	else
		DrvIn.type      = 0x1F;
	DrvIn.length	= 0;
	
	ucResult = Untouch_SendReceive_Getcard(50);
	{
		unsigned char  i;
		for(i = 0; i < 8; i++)
		{
			Uart_Printf(" ");
		}
	}/**/

	if( ucResult == SUCCESS)
	{
		*ucStatus = DrvOut.xxdata[0];
		*ucOutDataLength = 4; 
		memcpy(pucOutData,&DrvOut.xxdata[1],4);
		if(!memcmp(&DrvOut.xxdata[1],"\x00\x00\x00\x00",4))
			*ucStatus=0x04;
		ucResult=Untouch_Check_Err(*ucStatus);
		
	}
	return(ucResult);
}

unsigned char Untouch_GetVersion_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	
	DrvIn.type      = 0x1E;
	DrvIn.length	= 0;
	
	Uart_Printf("\nSend 1e\n");
	memset(DrvIn.xxdata,0,sizeof(DrvIn.xxdata));
	ucResult = Untouch_SendReceive_tmp();
	{
		unsigned char  i;
		Uart_Printf("\nThis Is 1E RECV data\n");
		for(i = 0; i < 15; i++)
		{
			Uart_Printf("%02x ",DrvOut.xxdata[i]);
		}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		*ucStatus = DrvOut.xxdata[0];
		*ucOutDataLength = 15; 
		memcpy(pucOutData,&DrvOut.xxdata[1],15);
		if(memcmp(&DrvOut.xxdata[1],"WK",2))
			*ucStatus=0x04;
		ucResult=Untouch_Check_Err(*ucStatus);
		
	}
	return(ucResult);
}


unsigned char Untouch_ChangeStatus_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9];
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x13;
	DrvIn.length	= 3;
	
	DrvIn.xxdata[0]=RunData.ucTagetStatus;
	DrvIn.xxdata[1]=NormalTransData.ucMemFlag;
	DrvIn.xxdata[2]=NormalTransData.ucHopassFlag;
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 13 send data\n");
	for(i = 0; i < 3; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 13 recieve data\n");
	for(i = 0; i <4; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength =3; 
			memcpy(pucOutData,&DrvOut.xxdata[1],3);
			if(*ucStatus==0&&
				!memcmp(pucOutData,"\x00\x00\x00\x00",4))
				*ucStatus=0x04;
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}

unsigned char Untouch_ReadCardDetail_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;

	DrvIn.type      = 0x17;
	DrvIn.length	= 1;
	Uart_Printf("\nSend 17\n");
	memcpy(DrvIn.xxdata,&NormalTransData.ucTransPoint,1);
	
	ucResult = Untouch_SendReceive_tmp();
	
	{
	unsigned char  i;
	Uart_Printf("\n***PINPADȡϸ:*****\n");
	for(i = 0; i < 17; i++)
	{
	Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
	Uart_Printf("\n");
	}

	
	if( ucResult == SUCCESS)
	{
		if( !DrvOut.drv_status )
		{
			if(( NULL != pucOutData)&&( NULL != ucStatus))
			{
				*ucStatus = DrvOut.xxdata[0];
				*ucOutDataLength = 16; 
				memcpy(pucOutData,&DrvOut.xxdata[1],16);
				if(!memcmp(pucOutData,"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",16))
					*ucStatus=0x04;
			}
			else	
				return (ERR_READMEMORY);
		
		}		
		
	}
	else
	{
		return ucResult;
	}
	
	ucResult=Untouch_Check_Err(*ucStatus);
	
	return(ucResult);
}

unsigned char Untouch_SendReceive_Getcard(unsigned int uiTime)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptKey;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	
		
	while(1)
	{

		Os__xdelay(1);
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);
		Os__xdelay(1);
		
		//Os__abort_drv(drv_mmi);//add by jiang.gs 2005.11.11
		
		//ptKey = Os__get_key();
		uiTimesOut = uiTime;
		
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			/*G_RUNDATA_ucDialRetry++;
			if(G_RUNDATA_ucDialRetry == 5)
				return ERR_APP_TIMEOUT;
			return ERR_JTK_TMPDEFINE;*/
			return 0x04;
		}
		Os__xdelay(1);

	 	/*drv end*/
		Os__xdelay(1);

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}
		else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}
		
	}

}

unsigned char Untouch_SendReceive_tmp(void)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptKey;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	
		
	while(1)
	{

		ptKey = Os__get_key();
		Uart_Printf("why1\n");
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);
		Uart_Printf("why2\n");
				
		uiTimesOut = 500;
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING)&&(ptKey->gen_status != DRV_ENDED));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			/*S_RUNDATA_ucDialRetry++;
			if(S_RUNDATA_ucDialRetry == 5)
				return ERR_APP_TIMEOUT;
			return ERR_JTK_TMPDEFINE;*/
			return 0x04;
		}
		Uart_Printf("why3\n");
		/*key pressed*/
		if(ptKey->gen_status == DRV_ENDED)
	 	{
	 		if(ptKey->xxdata[1] == KEY_CLEAR)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return ERR_CANCEL;
	 		}
	 		else
	 		{
	 			ucAbort = true;
 				continue;
 			}
	 	}
	 	/*drv end*/
		Uart_Printf("why4\n");

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}
		Uart_Printf("why5\n");
	}

}

unsigned char Untouch_SendReceive_rp(void)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptKey;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	
		
	while(1)
	{

		ptKey = Os__get_key();
		Os__xdelay(20);
		Uart_Printf("why1\n");
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);
		Uart_Printf("why2\n");
				
		uiTimesOut = 500;
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING)&&(ptKey->gen_status != DRV_ENDED));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			/*S_RUNDATA_ucDialRetry++;
			if(S_RUNDATA_ucDialRetry == 5)
				return ERR_APP_TIMEOUT;
			return ERR_JTK_TMPDEFINE;*/
			return 0x04;
		}
		Uart_Printf("why3\n");
		/*key pressed*/
		if(ptKey->gen_status == DRV_ENDED)
	 	{
	 		if(ptKey->xxdata[1] == KEY_CLEAR)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return ERR_CANCEL;
	 		}
	 		else
	 		{
	 			ucAbort = true;
 				continue;
 			}
	 	}
	 	/*drv end*/
		Uart_Printf("why4\n");

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}
		Uart_Printf("why5\n");
	}

}

unsigned char Untouch_SendReceive_WaitCard(void)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptKey;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	
		
	while(1)
	{

		Uart_Printf("why1\n");
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);
		Uart_Printf("why2\n");
		
		//Os__abort_drv(drv_mmi);//add by jiang.gs 2005.11.11
		ptKey = Os__get_key();
		
		Os__xdelay(20);
		uiTimesOut = 50;
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING)&&(ptKey->gen_status != DRV_ENDED));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			/*S_RUNDATA_ucDialRetry++;
			if(S_RUNDATA_ucDialRetry == 5)
				return ERR_APP_TIMEOUT;
			return ERR_JTK_TMPDEFINE;*/
			return 0x04;
		}
		Uart_Printf("why3\n");
		/*key pressed*/
		if(ptKey->gen_status == DRV_ENDED)
	 	{
	 		if(ptKey->xxdata[1] == KEY_CLEAR)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return ERR_CANCEL;
	 		}
/*	 		else if(ptKey->xxdata[1] == KEY_ENTER)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return tg_ERR_END;
	 		}
*/	 		
	 		else
	 		{
	 			ucAbort = true;
 				continue;
 			}
	 	}
	 	/*drv end*/
		Uart_Printf("why4\n");

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}
		Uart_Printf("why5\n");
	}

}

unsigned char Untouch_SendReceive_WaitCard1(void)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptKey;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	
		
	while(1)
	{

		Uart_Printf("why1\n");
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);
		Uart_Printf("why2\n");
		
		Os__abort_drv(drv_mmi);//add by jiang.gs 2005.11.11
		
		ptKey = Os__get_key();
		Os__xdelay(20);
		uiTimesOut = 50;
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING)&&(ptKey->gen_status != DRV_ENDED));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			/*S_RUNDATA_ucDialRetry++;
			if(S_RUNDATA_ucDialRetry == 5)
				return ERR_APP_TIMEOUT;
			return ERR_JTK_TMPDEFINE;*/
			return 0x04;
		}
		Uart_Printf("why3\n");
		/*key pressed*/
		if(ptKey->gen_status == DRV_ENDED)
	 	{
	 		if(ptKey->xxdata[1] == KEY_CLEAR)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return ERR_CANCEL;
	 		}
	 		else if(ptKey->xxdata[1] == KEY_ENTER)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return KEY_ENTER;
	 		}
	 		else if(ptKey->xxdata[1] == KEY_F4)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return KEY_F4;
	 		}
/**/	 		
	 		else
	 		{
	 			ucAbort = true;
 				continue;
 			}
	 	}
	 	/*drv end*/
		Uart_Printf("why4\n");

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}
		Uart_Printf("why5\n");
	}

}
unsigned char Untouch_FindCard_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	int i;
	
	DrvIn.type      = 0x80;
	DrvIn.length	= 2;
	memcpy(DrvIn.xxdata,(uchar *)"\x52\x00",2);
	Uart_Printf("send 80");
	
	ucResult = Untouch_SendReceive_Getcard(50);
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 80 recieve data\n");
	for(i = 0; i <5; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		*ucStatus = DrvOut.xxdata[0];
		*ucOutDataLength = 4; 
		memcpy(pucOutData,&DrvOut.xxdata[1],4);
		if(!memcmp(&DrvOut.xxdata[1],"\x00\x00\x00\x00",4))
			*ucStatus=0x04;
		ucResult=Untouch_Check_Err(*ucStatus);
		
	}
	return(ucResult);
}


unsigned char Untouch_CardVerify_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9];
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x81;
	DrvIn.length	= 9;
	memcpy(DrvIn.xxdata,(uchar *)"\x00\x01\x01",3);
	memcpy(&DrvIn.xxdata[3],RunData.aucWriteKey1,6);
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 81 send data\n");
	for(i = 0; i < 9; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 81 recieve data\n");
	for(i = 0; i <4; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}



unsigned char Untouch_0_CardVerify_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9];
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x81;
	DrvIn.length	= 9;
	memcpy(DrvIn.xxdata,(uchar *)"\x00\x00\x00",3);
	memcpy(&DrvIn.xxdata[3],RunData.aucWriteKey2,6);
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 81 send data\n");
	for(i = 0; i < 9; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 81 recieve data\n");
	for(i = 0; i <4; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}


unsigned char Untouch_ReadBlock_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9];
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x82;
	DrvIn.length	= 1;
	
	DrvIn.xxdata[0]=4;
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 82 send data\n");
	for(i = 0; i < 3; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 82 recieve data\n");
	for(i = 0; i <17; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength =16; 
			memcpy(pucOutData,&DrvOut.xxdata[1],16);
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}



unsigned char Untouch_Read_0_Block_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[300],aucBuf[9];
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x82;
	DrvIn.length	= 1;
	
	DrvIn.xxdata[0]=0;
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 82 send data\n");
	for(i = 0; i < 3; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 82 recieve data\n");
	for(i = 0; i <17; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(( NULL != pucOutData)&&( NULL != ucStatus))
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength =16; 
			memcpy(pucOutData,&DrvOut.xxdata[1],16);
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}

unsigned char Untouch_ChangeBlock_tmp(unsigned char *ucStatus, unsigned char *aucIndata)
{
	unsigned char aucOutData[300],aucBuf[9];
 	unsigned char ucLen;
 	unsigned char aucCheck[20],ucResult = SUCCESS;
	
	DrvIn.type      = 0x83;
	DrvIn.length	= 17;
	DrvIn.xxdata[0]=4;
	memcpy(&DrvIn.xxdata[1],aucIndata,16);
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 83 send data\n");
	for(i = 0; i < 17; i++)
	{
		Uart_Printf("%02x ",DrvIn.xxdata[i]);
	}
		Uart_Printf("\n");
	}
	
	ucResult = Untouch_SendReceive_tmp();
	{
	unsigned char  i;
	Uart_Printf("\nThis Is 83 recieve data\n");
	for(i = 0; i <4; i++)
	{
		Uart_Printf("%02x ",DrvOut.xxdata[i]);
	}
		Uart_Printf("\n");
	}

	if( ucResult == SUCCESS)
	{
		if(NULL != ucStatus)
		{
			*ucStatus = DrvOut.xxdata[0];
		}
		else	
			return (ERR_READMEMORY);
		

		ucResult=Untouch_Check_Err(*ucStatus);		
	}   
	else
		return ucResult;
	
	return(ucResult);
}
unsigned char Untouch_WaitCard(void)
{
	unsigned char ucResult = SUCCESS,ucStatus;
	
	//if(NormalTransData.ucCardType==CARD_CPU)
	//	DrvIn.type      = 0x6F;
	//else
		DrvIn.type      = 0x1F;
	DrvIn.length	= 0;
	
	ucResult = Untouch_SendReceive_WaitCard();
	{
		unsigned char  i;
		for(i = 0; i < 8; i++)
		{
			Uart_Printf(" ");
		}
	}/**/

	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		if(!memcmp(&DrvOut.xxdata[1],"\x00\x00\x00\x00",4))
			ucStatus=0x04;
		ucResult=Untouch_Check_Err(ucStatus);
		
	}
	return(ucResult);
}
unsigned char Untouch_WaitCard1(void)
{
	unsigned char ucResult = SUCCESS,ucStatus;
	
	//if(NormalTransData.ucCardType==CARD_CPU)
	//	DrvIn.type      = 0x6F;
	//else
		DrvIn.type      = 0x1F;
	DrvIn.length	= 0;
	
	ucResult = Untouch_SendReceive_WaitCard1();
	{
		unsigned char  i;
		for(i = 0; i < 8; i++)
		{
			Uart_Printf(" ");
		}
	}/**/

	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		if(!memcmp(&DrvOut.xxdata[1],"\x00\x00\x00\x00",4))
			ucStatus=0x04;
		ucResult=Untouch_Check_Err(ucStatus);
		
	}
	return(ucResult);
}

unsigned int Untouch_WaitAllCard(void)
{
	unsigned int ret;            
	unsigned int uiI=0;
	unsigned char ucKey;
	unsigned char aucReq[8],aucCardId[16];
	unsigned char ucLen,ucCardClass;
	unsigned char aucRdInfo[64];
	unsigned char aucWrtInfo[64];
	unsigned char aucData[16];
	NEW_DRV_TYPE  new_drv;                                                    
	unsigned char icc_ATR[300];
	unsigned char  length1,ucResult;

	while(1)
	{
		Uart_Printf("\n1. Os_Wait_Event begin...");
		ret = Os_Wait_Event(KEY_DRV|MAG_DRV | MFR_DRV | ICC_DRV , &new_drv, 5000);
		//Uart_Printf("\n2. Os_Wait_Event:%02x",ret);
		//Uart_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ret == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					ucKey=new_drv.drv_data.xxdata[1];
					Uart_Printf("\n ucKey:%02x.", ucKey);
                    if(ucKey == KEY_CLEAR)
					return ERR_CANCEL;
				}
			}                                                                   
			else if(new_drv.drv_type == MAG_DRV)                                
			{                
				Uart_Printf("\n4. gen_status:%02x",new_drv.drv_data.gen_status);
				if(new_drv.drv_data.gen_status  == DRV_ENDED){
					NormalTransData.ucCardType=CARD_MAG;
					ucResult= MAG_GetTrackInfo((unsigned char *)&new_drv.drv_data);
				}
			}                                                                   
			else if(new_drv.drv_type == MFR_DRV)                                
			{                                                                   
				NormalTransData.ucCardType=CARD_M1;
				Uart_Printf("\n MFR DRE rest:%02x",ret);
				break;		
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if( new_drv.drv_data.xxdata[1] == 0x37 )
				{
					NormalTransData.ucCardType=CARD_MEM;
					length1 = new_drv.drv_data.xxdata[0];
                    memcpy(icc_ATR,(uchar*)&new_drv.drv_data,length1+4);
					if(icc_ATR[4]==ICC_ASY)
					{
						RunData.ucInputmode=1;
						NormalTransData.ucCardType=CARD_M1;
					}
					break;
				}
			}                                                                   
			else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				return ERR_CANCEL;			
			}	        
			Uart_Printf("\n OWE_NewDrvtts End.....\n");
			break;
		}                                                                         
		else                                                                      
		{                                                                         
			Untouch_OWE_RstErrInfo(ret);                                               
		}                                                                         
	}
	Os__xdelay(100*30);
	Uart_Printf("\n OWE_NewDrvtts rest:%02x.\n",ret);
	return 0;
}	

unsigned int Untouch_WaitAllCard1(void)
{
	unsigned int ret;            
	unsigned int uiI=0;
	unsigned char ucKey;
	unsigned char aucReq[8],aucCardId[16];
	unsigned char ucLen,ucCardClass;
	unsigned char aucRdInfo[64];
	unsigned char aucWrtInfo[64];
	unsigned char aucData[16];
	NEW_DRV_TYPE  new_drv;                                                    
	unsigned char icc_ATR[300];
	unsigned char  length1,ucResult;

	while(1)
	{
		Uart_Printf("\n1. Os_Wait_Event begin...");
		ret = Os_Wait_Event(KEY_DRV|MAG_DRV | MFR_DRV | ICC_DRV , &new_drv, 5000);
		//Uart_Printf("\n2. Os_Wait_Event:%02x",ret);
		//Uart_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ret == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					ucKey=new_drv.drv_data.xxdata[1];
					Uart_Printf("\n ucKey:%02x.", ucKey);
					if(ucKey==KEY_CLEAR)
					{
						return ERR_CANCEL;
					}
					if(ucKey==KEY_ENTER)
					{
						return KEY_ENTER;
					}
					if(ucKey==KEY_F4)
					{
						return KEY_F4;
					}
				}
			}                                                                   
			else if(new_drv.drv_type == MAG_DRV)                                
			{                
				Uart_Printf("\n4. gen_status:%02x",new_drv.drv_data.gen_status);
				if(new_drv.drv_data.gen_status  == DRV_ENDED){
					NormalTransData.ucCardType=CARD_MAG;
					ucResult= MAG_GetTrackInfo((unsigned char *)&new_drv.drv_data);
				}
			}                                                                   
			else if(new_drv.drv_type == MFR_DRV)                                
			{                                                                   
				NormalTransData.ucCardType=CARD_M1;
				Uart_Printf("\n MFR DRE rest:%02x",ret);
				break;		
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if( new_drv.drv_data.xxdata[1] == 0x37 )
				{
					NormalTransData.ucCardType=CARD_MEM;
					length1 = new_drv.drv_data.xxdata[0];
                    memcpy(icc_ATR,(uchar*)&new_drv.drv_data,length1+4);
					if(icc_ATR[4]==ICC_ASY)
					{
						RunData.ucInputmode=1;
						NormalTransData.ucCardType=CARD_M1;
					}
					break;
				}
			}                                                                   
			else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				return ERR_CANCEL;			
			}	        
			Uart_Printf("\n OWE_NewDrvtts End.....\n");
			break;
		}                                                                         
		else                                                                      
		{                                                                         
			Untouch_OWE_RstErrInfo(ret);                                               
		}                                                                         
	}
	Os__xdelay(100*30);
	Uart_Printf("\n OWE_NewDrvtts rest:%02x.\n",ret);
	return 0;
}	

/*

unsigned char Untouch_WaitAllCard(void)
{
	unsigned char 	ucResult,aucBuf[17],icc_ATR[300];
	unsigned int	uiTimesOut;
	unsigned char 	length1;
	DRV_OUT  		*ptKeyboard;
	DRV_OUT 		*pxIcc;
	DRV_OUT 		*pxMag;
	union 	seve_in eve_in;
	
	
	ucResult=SUCCESS;
	while(1)
	{
		pxMag = Os__mag_read();
		pxIcc = Os__ICC_insert();
		ucResult=MIFARE_WaitCard();		
		ptKeyboard=Os__get_key();
		Os__xdelay(20);
		uiTimesOut = 60*100;
//		Os__timer_start(&uiTimesOut);
		while((ptKeyboard->gen_status != DRV_ENDED)
				&&(pxMag->gen_status != DRV_ENDED)
				&&(pxIcc->gen_status != DRV_ENDED)&&(ucResult!=SUCCESS))
//		Os__timer_stop(&uiTimesOut);
		{
			ucResult=MIFARE_WaitCard();		
		}
		
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			uiTimesOut = 10;
			Os__timer_start(&uiTimesOut);
			while(uiTimesOut);
			Os__timer_stop(&uiTimesOut);
			return ERR_CANCEL;
		}else if(ptKeyboard->gen_status == DRV_ENDED)
		{
			unsigned char ch;
			
			ch = ptKeyboard->xxdata[1];
			if(ch==KEY_CLEAR)
			{
				Os__abort_drv(drv_mmi);
				return ERR_CANCEL;
			}
		}else if(pxIcc->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			NormalTransData.ucCardType=CARD_MEM;
			length1 = pxIcc->xxdata[0];
			memcpy(icc_ATR,pxIcc,length1+4);
			if(icc_ATR[4]==ICC_ASY)
			{
				RunData.ucInputmode=1;
				NormalTransData.ucCardType=CARD_M1;
			}
			break;		
		}else if(pxMag->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_icc);
			NormalTransData.ucCardType=CARD_MAG;
			ucResult= MAG_GetTrackInfo((unsigned char*)pxMag);
			break;		
		}else if(ucResult==SUCCESS)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			NormalTransData.ucCardType=CARD_M1;
			break;		
		}
		else if(ucResult==ERR_CANCEL)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			return ERR_CANCEL;
		}	
	}
	return SUCCESS;
}
unsigned char Untouch_WaitAllCard1(void)
{
	unsigned char 	ucResult,aucBuf[17],icc_ATR[300];
	unsigned int	uiTimesOut;
	unsigned char 	length1;
	DRV_OUT  		*ptKeyboard;
	DRV_OUT 		*pxIcc;
	DRV_OUT 		*pxMag;
	union 	seve_in eve_in;
	
	
	ucResult=SUCCESS;
	while(1)
	{
		pxMag = Os__mag_read();
		pxIcc = Os__ICC_insert();
		ucResult=MIFARE_WaitCard1();		
		ptKeyboard=Os__get_key();
		Os__xdelay(20);
		uiTimesOut = 60*100;
//		Os__timer_start(&uiTimesOut);
		while((ptKeyboard->gen_status != DRV_ENDED)
				&&(pxMag->gen_status != DRV_ENDED)
				&&(pxIcc->gen_status != DRV_ENDED)&&(ucResult!=SUCCESS))
//		Os__timer_stop(&uiTimesOut);
		{
			ucResult=MIFARE_WaitCard1();		
		}
		
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			uiTimesOut = 10;
			Os__timer_start(&uiTimesOut);
			while(uiTimesOut);
			Os__timer_stop(&uiTimesOut);
			return ERR_CANCEL;
		}else if(ptKeyboard->gen_status == DRV_ENDED)
		{
			unsigned char ch;
			
			ch = ptKeyboard->xxdata[1];
			if(ch==KEY_CLEAR)
			{
				Os__abort_drv(drv_mmi);
				return ERR_CANCEL;
			}
			if(ch==KEY_ENTER)
			{
				Os__abort_drv(drv_mmi);
				return KEY_ENTER;
			}
			if(ch==KEY_F4)
			{
				Os__abort_drv(drv_mmi);
				return KEY_F4;
			}
		}else if(pxIcc->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			NormalTransData.ucCardType=CARD_MEM;
			length1 = pxIcc->xxdata[0];
			memcpy(icc_ATR,pxIcc,length1+4);
			if(icc_ATR[4]==ICC_ASY)
			{
				RunData.ucInputmode=1;
				NormalTransData.ucCardType=CARD_M1;
			}
			break;		
		}else if(pxMag->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_icc);
			NormalTransData.ucCardType=CARD_MAG;
			ucResult= MAG_GetTrackInfo((unsigned char*)pxMag);
			break;		
		}else if(ucResult==SUCCESS)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			NormalTransData.ucCardType=CARD_M1;
			break;		
		}
		else if(ucResult==ERR_CANCEL)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			return ERR_CANCEL;
		}	
		else if(ucResult==KEY_ENTER)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			return KEY_ENTER;
		}	
		else if(ucResult==KEY_F4)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			return KEY_ENTER;
		}	
	}
	return SUCCESS;
}
*/
unsigned char Untouch_GetBitDate(uchar *inDate,uchar *outDate)
{
	unsigned char aucBitdate[10],aucNormaldate[10];
	unsigned char ucByte,ucByte1,ucByte2,ucByte3,ucByte4,ucByte5;
	int uiNum,uiNum1,uiNum2,uiNum3,uiNum4,uiNum5;
	memset(aucNormaldate,0,sizeof(aucNormaldate));
	memcpy(aucNormaldate,inDate,7);
	uiNum=bcd_long(&aucNormaldate[1], 2);
	ucByte=uiNum;
	uiNum1=bcd_long(&aucNormaldate[2], 2);
	ucByte1=uiNum1;
	uiNum2=bcd_long(&aucNormaldate[3], 2);
	ucByte2=uiNum2;
	uiNum3=bcd_long(&aucNormaldate[4], 2);
	ucByte3=uiNum3;
	uiNum4=bcd_long(&aucNormaldate[5], 2);
	ucByte4=uiNum4;
	uiNum5=bcd_long(&aucNormaldate[6], 2);
	ucByte5=uiNum5;
	aucBitdate[0]=(ucByte<<2)|(ucByte1>>2);
	aucBitdate[1]=(ucByte1<<6)|(ucByte2<<1)|(ucByte3>>4);
	aucBitdate[2]=(ucByte3<<4)|(ucByte4>>2);
	aucBitdate[3]=(ucByte4<<6)|ucByte5;
	memcpy(outDate,aucBitdate,4);
	return SUCCESS;
}

unsigned char Untouch_ReadCpu_tmp(unsigned char *ucStatus, unsigned char *pucOutData, 
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;
	unsigned char aucBuf[20];
	
	memset(aucBuf,0,sizeof(aucBuf));
	DrvIn.type      = 0x6F;
	DrvIn.length	= 0;
	Uart_Printf("\nSend 6f\n");
	
	ucResult = Untouch_SendReceive_tmp();
	{
		unsigned char  i;
		Uart_Printf("\nRecieve 6f\n");
		for(i = 0; i < 25; i++)
		{
			Uart_Printf("%02x ",DrvOut.xxdata[i]);
		}
		Uart_Printf("\n");
	}

	
	if( ucResult == SUCCESS)
	{
		*ucStatus = DrvOut.xxdata[0];
		if(!memcmp(&DrvOut.xxdata[6],aucBuf,16))
			*ucOutDataLength = 5; 
		else
			*ucOutDataLength = 21; 
		memcpy(pucOutData,&DrvOut.xxdata[1],*ucOutDataLength);
		if(!memcmp(pucOutData,"\x00\x00\x00\x00",4))
			*ucStatus=0x04;
		ucResult=Untouch_Check_Err(*ucStatus);
		
	}
	return(ucResult);
}
unsigned char Untouch_CheckCardApp(void)
{
	unsigned char ucResult = SUCCESS,ucKeyArray,ucCrcKeyIndex,ucKeyAIndex,ucPKeyIndex;
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20],aucBuf1[20],uctreat;
	int i;
	unsigned short usLen;
	unsigned char	aucCardId[32];
	DRV_OUT *pdKey;
	
    ////Os__clr_display(255);
    ////Os__GB2312_display(0,0,(uchar *)"Ժ...");
	if(ucResult == SUCCESS&&RunData.ucInputmode==0)
	{
		//pdKey = Os__get_key();
   		for(i=0;i<10;i++)
		{
	Uart_Printf("\n 20120625-000001-001  Untouch_CheckCardApp  .... Begin\n");
		
			memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
			ucResult = CMD_AntiRFcard(MifareCommandData.aucCommandOut,&MifareCommandData.ucOutLen);

	Uart_Printf("\n 20120625-000001-002  Untouch_CheckCardApp  .... [%02X]\n",ucResult);
			
			if(MIFARE_ucOutStatus == 0x00)
			{
				if(RunData.ucInputmode==0)
				memcpy(NormalTransData.aucCardSerial,&MifareCommandData.aucCommandOut[1],4);
				ucResult = SUCCESS;
				break;
			}
			else
			{
				if(MIFARE_ucOutStatus == 0x04)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(unsigned char*)"ſ>>>");
				}
				ucResult = MIFARE_FindErrCode(MIFARE_ucOutStatus);
				if( pdKey->gen_status == DRV_ENDED )
				{
					if( pdKey->xxdata[1] == KEY_CLEAR )
					{
						break;
					}
					else
					{
						//pdKey = Os__get_key();
					}
				}
			}
		}
		
		Os__abort_drv(drv_mmi);
		
		if(ucResult==SUCCESS)
		{
			memset(aucCardId,0,sizeof(aucCardId));
			ucResult = MIFARE_GetAts(aucCardId,&ucLen);// cpu ka xieyi panduan 
			//ucResult = Untouch_ReadCpu_tmp(&ucStatus,aucOutData,&ucLen);
		}
	}


		Uart_Printf("\n 20120625-000001-003  Untouch_CheckCardApp  .... [%02X]\n",ucResult);
		
	if(ucResult == SUCCESS || RunData.ucInputmode==1)
	{
		RunData.ucHave6F=1;
		ucResult=PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x00");
		if(ucResult == SUCCESS)
		{
			ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"SANDPAY",7);
		}
		if(ucResult != SUCCESS)
		{
			ucResult =  PBOC_ISOSelectFilebyAID(ICC1, (unsigned char*)"1PAY.SYS.DDF01",14);		
			if(ucResult == SUCCESS)
				ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"\xA0\x00\x00\x00\x03\x86\x98\x07\x02",9);
			if(ucResult == SUCCESS)
				ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"\x10\x00\x00\x00\x00\x00\x10\x00\x03",9);
		}
		if(ucResult == SUCCESS)
		{
			ucResult=PBOC_ISOReadBinarybySFI(ICC1, 0x15, 0, 30);
		}
		if(ucResult == SUCCESS)
		{
			if(memcmp(Iso7816Out.aucData,"SANDPAY",7))
				return ERR_NOTSUCHAPP;
		}
		else
			return ERR_NOTSUCHAPP;

	}
	else
	{
		MIFARE_GetVersion();//ǰ1ΰ汾
   		memset(aucOutData,0,sizeof(aucOutData));
	 	ucResult=MIFARE_GetCardInfo();//
		if(ucResult==SUCCESS)
		{
			bcd_asc(aucBuf1,&MifareCommandData.aucCommandOut[4],8);
			Uart_Printf("**Ρ:*****\n");
			{
			int i;
			for(i=0;i<8;i++)
				Uart_Printf("%02x ",aucBuf1[i]);
			Uart_Printf("\n");
			}
			if(!memcmp(aucBuf1,"9",1)&&MifareCommandData.aucCommandOut[1]!=0x55)
			{
				return ERR_NOTSUCHAPP;
			}
		}
		else
			return ERR_NOTSUCHAPP;
		/**/	
	}
		Uart_Printf("\n 20120625-000001-004  Untouch_CheckCardApp  .... [%02X]\n",ucResult);
	return SUCCESS;
}
unsigned char Untouch_ReadCardInfo(void)
{
	unsigned char ucResult = SUCCESS,ucKeyArray,ucCrcKeyIndex,ucKeyAIndex,ucPKeyIndex,ucLoadMacKeyIndex;
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20],aucBuf1[20],uctreat;
	int i;
	unsigned short usLen;
	unsigned char	aucCardId[32];
	unsigned long ulAmount;
	DRV_OUT *pdKey;
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"Ժ...");
	if(ucResult == SUCCESS&&RunData.ucInputmode==0)
	{
		//pdKey = Os__get_key();
   		for(i=0;i<10;i++)
		{
			memset(MifareCommandData.aucCommandOut,0,sizeof(MifareCommandData.aucCommandOut));
			ucResult = CMD_AntiRFcard(MifareCommandData.aucCommandOut,&MifareCommandData.ucOutLen);
			Uart_Printf("MIFARE_ucOutStatus=%02x\n",MifareCommandData.aucCommandOut[0]);
			if(MIFARE_ucOutStatus == 0x00)
			{
				if(RunData.ucInputmode==0)
				memcpy(NormalTransData.aucCardSerial,&MifareCommandData.aucCommandOut[1],4);
				ucResult = SUCCESS;
				break;
			}
			else
			{
				if(MIFARE_ucOutStatus == 0x04)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(unsigned char*)"ſ>>>");
				}
				ucResult = MIFARE_FindErrCode(MIFARE_ucOutStatus);
				if( pdKey->gen_status == DRV_ENDED )
				{
					if( pdKey->xxdata[1] == KEY_CLEAR )
					{
						break;
					}
					else
					{
					//	pdKey = Os__get_key();
					}
				}
			}
			
		}
		Os__abort_drv(drv_mmi);
		if(ucResult==SUCCESS)
		{
	   	memset(aucCardId,0,sizeof(aucCardId));
	   	ucResult = MIFARE_GetAts(aucCardId,&ucLen);
		}
	}
	if(ucResult == SUCCESS || RunData.ucInputmode==1)
	{
		ucResult=PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x00");
			
		if(ucResult == SUCCESS)
		{
			ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"SANDPAY",7);
		//ucResult=PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\xdf\xad");
		}
		if(ucResult != SUCCESS)
		{
			ucResult =  PBOC_ISOSelectFilebyAID(ICC1, (unsigned char*)"1PAY.SYS.DDF01",14);		
			if(ucResult == SUCCESS)
				ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"\xA0\x00\x00\x00\x03\x86\x98\x07\x02",9);
			if(ucResult == SUCCESS)
				ucResult=PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"\x10\x00\x00\x00\x00\x00\x10\x00\x03",9);
		}
		if(ucResult == SUCCESS)
		{
			ucResult=PBOC_ISOReadBinarybySFI(ICC1, 0x15, 0, 40);
		}
		if(ucResult == SUCCESS)
		{
			if(memcmp(Iso7816Out.aucData,"SANDPAY",7))
				return ERR_NOTSUCHAPP;
			memcpy(NormalTransData.aucUserCardNo,&Iso7816Out.aucData[10],8);

            Uart_Printf("000NormalTransData.aucUserCardNo~~~~~~~~~~~~~~~~~~~%s \n",NormalTransData.aucUserCardNo);
			if(RunData.ucInputmode==1)
				memcpy(NormalTransData.aucCardSerial,NormalTransData.aucUserCardNo,4);
			memcpy(NormalTransData.aucExpiredDate,&Iso7816Out.aucData[24],4);
			bcd_asc(NormalTransData.aucLoadTrace,&Iso7816Out.aucData[30],16);
			NormalTransData.ucPtCode=Iso7816Out.aucData[8];
			//NormalTransData.ucPtCode=0x01;
			if(NormalTransData.ucPtCode<0x01||NormalTransData.ucPtCode>0x03)
				NormalTransData.ucPtCode=0x01;
		}
		if(ucResult==SUCCESS)
		{
			Uart_Printf("֤pin\n");
			ucResult = PBOC_ISOVerifyPIN(ICC1,3,(uchar*)"\x88\x88\x88");
		}
		if(ucResult == SUCCESS)
		{
			ucResult=PBOC_ISOGetBalance(ICC1, 0x02);			
		}
		if(ucResult == SUCCESS)
			NormalTransData.ulPrevEDBalance = tab_long(Iso7816Out.aucData,4);
		if(ucResult == SUCCESS)
		{
			ucResult=PBOC_ISOReadBinarybySFI(ICC1, 0x16, 0, 41);
		}
		if(ucResult == SUCCESS)
		{
			memcpy(NormalTransData.aucZoneCode,&Iso7816Out.aucData[1],3);
			SleTransData.ucStatus=Iso7816Out.aucData[0];
			//memcpy(NormalTransData.aucZoneCode,"\x00\x10\x01",3);
			memcpy(NormalTransData.aucUserName,&Iso7816Out.aucData[4],18);
			//memcpy(NormalTransData.aucUserIDNo,&Iso7816Out.aucData[22],18);
			//NormalTransData.ucIDType=Iso7816Out.aucData[40];
			NormalTransData.ucCardType=CARD_CPU;
			NormalTransData.ucNewOrOld=1;
		}
		if(ucResult == SUCCESS)
		{
			uctreat=DataSave0.ConstantParamData.aucTreat[NormalTransData.ucPtCode-1];
			Uart_Printf("DataSave0.ConstantParamData.aucTreat=%02x\n",uctreat);
			if(uctreat=='0'
			||(NormalTransData.ucNewOrOld&&uctreat=='1')
			||(!NormalTransData.ucNewOrOld&&uctreat=='2'))
			{
				return ERR_CARDSUPPORT;
			}
		}
	}
	else
	{
   	Uart_Printf("\nUntouch Card11\n");
		MIFARE_GetVersion();//ǰ1ΰ汾
   		memset(aucOutData,0,sizeof(aucOutData));
	 	ucResult=MIFARE_GetCardInfo();//
		if(ucResult==SUCCESS)
		{
			memcpy(&M1TransData,&MifareCommandData.aucCommandOut[1],sizeof(M1TransData));
			memcpy(NormalTransData.aucUserCardNo,M1TransData.aucCardNo,10);
		       memcpy(NormalTransData.aucIssueDate,M1TransData.aucIssueBatch,4);
			memcpy(NormalTransData.aucVerifyCode,M1TransData.aucVerifyCode,4);
			memcpy(NormalTransData.aucCardSerial,M1TransData.aucUID,4);
			memcpy(NormalTransData.aucZoneCode,M1TransData.aucIssuerCode,3);
			RunData.ucCardVer1=M1TransData.ucCardVer1;
			NormalTransData.ucPtCode=M1TransData.ucFlatFlag;
			if(M1TransData.ucCardVer==0x55)
			{
				NormalTransData.ucNewOrOld=1;
			}
			else
			{
				NormalTransData.ucNewOrOld=0;
				NormalTransData.ucPtCode=0x01;
			}
			uctreat=DataSave0.ConstantParamData.aucTreat[NormalTransData.ucPtCode-1];
			if(uctreat=='0'
			||(NormalTransData.ucNewOrOld&&uctreat=='1')
			||(!NormalTransData.ucNewOrOld&&uctreat=='2'))
			{
				return ERR_CARDSUPPORT;
			}
			RunData.ucKeyArrey=0;
			if(NormalTransData.ucPtCode==0x03)
			{
				for(i=0;i<15;i++)
				{
					if(!memcmp(DataSave0.ChangeParamData.aucIssueKeyIndex[i],NormalTransData.aucZoneCode,2))
					{
						RunData.ucKeyArrey=i;
						break;
					}
				}
			}
			if(NormalTransData.ucPtCode==0x01)
			{
				bcd_asc(aucBuf1,NormalTransData.aucZoneCode,6);
				for(i=15;i<18;i++)
				{
					bcd_asc(aucBuf,DataSave0.ChangeParamData.aucIssueKeyIndex[i],6);
					if(!memcmp(aucBuf,aucBuf1,3))
					{
						RunData.ucKeyArrey=i;
						break;
					}
				}
			}
			if(NormalTransData.ucNewOrOld==0)
			{
				ucKeyArray=KEYARRAY_PURSECARD2;
				ucCrcKeyIndex=KEYINDEX_WKCARD1_CRCKEY1;
				ucKeyAIndex=KEYINDEX_WKCARD1_MESSAGEKEYA;
				ucPKeyIndex=KEYINDEX_WKCARD1_PURCHASEKEY;
			}
			else
			{
				switch(NormalTransData.ucPtCode)
				{
					case 0x01:
						if(RunData.ucKeyArrey)
							ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
						else
							ucKeyArray=KEYARRAY_PURSECARD1;
						break;
					case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
					case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
					default:break;
				}
				ucCrcKeyIndex=KEYINDEX_PURSECARD1_CRCKEY;
				ucKeyAIndex=KEYINDEX_PURSECARD1_MESSAGEKEYA;
				ucPKeyIndex=KEYINDEX_PURSECARD1_PURCHASEKEY;
				ucLoadMacKeyIndex=KEYINDEX_PURSECARD1_LOADMACKEY;
			}
			if((NormalTransData.aucUserCardNo[0]>>4)!=0x09||M1TransData.ucCardVer==0x55)
			{
				memcpy(aucBuf,NormalTransData.aucIssueDate,4);
				memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucCrcKeyIndex, aucBuf, aucBuf);	
				PINPAD_47_Crypt8ByteSingleKey(ucKeyArray, 
					ucCrcKeyIndex+1, aucBuf, aucBuf);	
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucCrcKeyIndex, aucBuf, aucBuf);	
				if(memcmp(aucBuf,NormalTransData.aucVerifyCode,4))
					return ERR_CARDCRC;

				memcpy(aucBuf1,&NormalTransData.aucIssueDate[2],2);
				memcpy(&aucBuf1[2],NormalTransData.aucCardSerial,4);
				memcpy(&aucBuf1[6],NormalTransData.aucVerifyCode,1);
				aucBuf1[7]=0x01;
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucKeyAIndex, aucBuf1, aucBuf);	
				PINPAD_47_Crypt8ByteSingleKey(ucKeyArray, 
					ucKeyAIndex+1, aucBuf, aucBuf);	
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucKeyAIndex, aucBuf, aucBuf);	
				memcpy(RunData.aucKeyA,aucBuf,6);

				memset(aucBuf,0,sizeof(aucBuf));
				aucBuf1[7]=0x02;
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucPKeyIndex, aucBuf1, aucBuf);
				PINPAD_47_Crypt8ByteSingleKey(ucKeyArray, 
					ucPKeyIndex+1, aucBuf,aucBuf);
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucPKeyIndex, aucBuf, aucBuf);
				memcpy(RunData.aucPurchaseKey,aucBuf,6);
			}
			else
			{
				memcpy(aucBuf1,NormalTransData.aucIssueDate,4);
				memcpy(&aucBuf1[4],NormalTransData.aucCardSerial,4);
				memcpy(&aucBuf1[8],NormalTransData.aucVerifyCode,4);
				aucBuf1[12]=0x02;
				memset(aucBuf,0,sizeof(aucBuf));
				usLen=sizeof(aucBuf);
				ucResult = SMART_Reset(2,aucBuf,&usLen);
				Uart_Printf("ucResult of reset=%02x\n",ucResult);
				if( ucResult != SMART_ASYNC) 
					return ERR_NOPSAM;
				ucResult = PBOC_ISOReadBinarybySFI(2,0x16,0,6);
				if( ucResult != SUCCESS) 
					return ucResult;
				memcpy(NormalTransData.aucPsamTerminal,Iso7816Out.aucData,6);
				ucResult = PBOC_ISOSelectFilebyFID(2,(uchar *)"\x10\x01");
				if( ucResult != SUCCESS) 
					return ucResult;
				ucResult = PBOC_ISOSpecialDes(2,aucBuf1);
				if( ucResult != SUCCESS) 
					return ucResult;
				memcpy(RunData.aucPurchaseKey,Iso7816Out.aucData,6);
				memcpy(RunData.aucKeyA,Iso7816Out.aucData,6);
			}
			if((NormalTransData.ucPtCode==1||NormalTransData.ucPtCode==2)&&DataSave0.ChangeParamData.aucTermFlag[5])
				ucResult=CFG_CheckFactoryBytes();
			if(ucResult==SUCCESS)
			{
		   	memset(aucOutData,0,sizeof(aucOutData));
			 	ucResult=MIFARE_ReadCardIssueInfo();
			}
		}
		if(ucResult==SUCCESS)
		{
			memcpy(&M1TransData1,&MifareCommandData.aucCommandOut[1],sizeof(M1TransData1));
			if(NormalTransData.ucNewOrOld==1)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				aucBuf[0]=M1TransData1.aucLoadAmount[3];
				aucBuf[1]=M1TransData1.aucLoadAmount[2];
				aucBuf[2]=M1TransData1.aucLoadAmount[1];
				aucBuf[3]=M1TransData1.aucLoadAmount[0];
				memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucLoadMacKeyIndex, aucBuf, aucBuf);	
				PINPAD_47_Crypt8ByteSingleKey(ucKeyArray, 
					ucLoadMacKeyIndex+1, aucBuf, aucBuf);	
				PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, 
					ucLoadMacKeyIndex, aucBuf, aucBuf);	
				if(memcmp(aucBuf,M1TransData1.aucLoadMac,4))
					return ERR_CARDMACERROR;
			}
			NormalTransData.ucMemFlag=M1TransData1.ucMemberFlag;
			NormalTransData.ucHopassFlag=M1TransData1.ucAccountFlag;
			NormalTransData.ucTransPoint=M1TransData1.ucPoint;
			memcpy(&NormalTransData.aucSamTace[2],M1TransData1.aucCount,2);
			bcd_asc(NormalTransData.aucLoadTrace,M1TransData1.aucLoadTrace,16);
			memcpy(NormalTransData.aucExpiredDate,M1TransData1.aucExpDate,4);
		       NormalTransData.ulPrevEDBalance = tab_long(M1TransData1.aucBalance, 4);
			memcpy(&SleTransData.ucStatus,&M1TransData1.ucState,1);
		}
		NormalTransData.ucCardType=CARD_M1;

	}
	if(ucResult==SUCCESS)
		NormalTransData.ulTrueBalance=NormalTransData.ulPrevEDBalance;
	return ucResult;
}
unsigned char Untouch_Bit2Asc_Date(uchar *aucdate,uchar *auctime,uchar *aucBit)
{
	unsigned char aucBuf[20],ucM,ucD,ucH,ucF;
	unsigned long ulm,uld,ulh,ulf;
	ucM=aucBit[0]>>4;
	ucD=(aucBit[0]<<1)&0x1f^(aucBit[1]>>7);
	ucH=(aucBit[1]>>2)&0x1f;
	ucF=(aucBit[1]<<4)&0x30^(aucBit[2]>>4);
	ulm=tab_long(&ucM,1);
	Uart_Printf("ulm=%0d\n",ulm);
	uld=tab_long(&ucD,1);
	Uart_Printf("uld=%0d\n",uld);
	ulh=tab_long(&ucH,1);
	Uart_Printf("ulh=%0d\n",ulh);
	ulf=tab_long(&ucF,1);
	Uart_Printf("ulf=%0d\n",ulf);
	long_asc(aucdate,2,&ulm);
	long_asc(&aucdate[2],2,&uld);
	long_asc(auctime,2,&ulh);
	long_asc(&auctime[2],2,&ulf);
	return SUCCESS;
}
unsigned char Untouch_Detail_Compare(uchar *CardDetail,NORMALTRANS Tran)
{
	unsigned char aucBuf[20],aucDatetime[20];
	unsigned long ulPrevEDBalance,ulAmount;
	ulAmount=Tran.ulAmount;
	if(RunData.aucFunFlag[1])
		ulPrevEDBalance=Tran.ulPrevEDBalance+Tran.ulYaAmount;
	else
	{
		ulPrevEDBalance=Tran.ulPrevEDBalance;
		if(Tran.ucReturnflag==true)
		{
			ulAmount=ulPrevEDBalance;
		}
	}
	if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
		memcpy(aucBuf,&NormalTransData.aucPsamTerminal[2],4);
	else
		asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucOnlytermcode,8);
	if(memcmp(&CardDetail[10],&aucBuf[1],3))
		return ERR_DIFFERENT_TERM;
	Untouch_Bit2Asc_Date(aucBuf, &aucBuf[4], CardDetail);
	bcd_asc(aucDatetime,Tran.aucDateTime,14);
	if(memcmp(aucBuf,&aucDatetime[4],8))
		return ERR_DIFFERENT_TIME;
	long_bcd(aucBuf,3,&Tran.ulTraceNumber);
	if(memcmp(aucBuf,&CardDetail[13],3))
		return ERR_DIFFERENT_TRACE;
	Uart_Printf("Untouch_Detail_Compare ulAmount=%d\n",ulAmount);
	long_tab(aucBuf,3,&ulAmount);
	aucBuf[4]=aucBuf[0];
	aucBuf[0]=aucBuf[2];
	aucBuf[2]=aucBuf[4];
	//aucBuf[2]=aucBuf[0];
	if(memcmp(aucBuf,&CardDetail[3],3))
		return ERR_DIFFERENT_MONEY;
	Uart_Printf("Untouch_Detail_Compare ulPrevEDBalance=%d\n",ulPrevEDBalance);
	long_tab(aucBuf,3,&ulPrevEDBalance);
	aucBuf[4]=aucBuf[0];
	aucBuf[0]=aucBuf[2];
	aucBuf[2]=aucBuf[4];
	//aucBuf[2]=aucBuf[0];
	if(memcmp(aucBuf,&CardDetail[6],3))
		return ERR_DIFFERENT_BALANCE;
	return SUCCESS;
}
unsigned char Untouch_CheckRedo(void)
{
	NORMALTRANS tnormal;
	unsigned char ucResult=SUCCESS,aucFensan[10],aucUseKey[20];
	unsigned char ucStatus,ucLen,aucOutData[30],aucBuf[20];
	int uiIndex,i;
	unsigned long ulAmount,ulPrevEDBalance;
	Uart_Printf("DataSave0.RedoTrans.ucRedoFlag=%02x\n",DataSave0.RedoTrans.ucRedoFlag);
	switch(DataSave0.RedoTrans.ucRedoFlag)
	{
		case 0:
			return SUCCESS;
		case 1:
			if(memcmp(NormalTransData.aucUserCardNo,DataSave0.RedoTrans.Tran.aucUserCardNo,8)
				||NormalTransData.ucCardType==CARD_MEM
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_PRELOAD
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_REFUND
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_CHANGECD
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_LOADONLINE
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_CASHLOAD)
			{
				ucResult=SAV_KyTran(1);
				return ucResult;
			}
			if(NormalTransData.ulPrevEDBalance==DataSave0.RedoTrans.Tran.ulPrevEDBalance)
			{
				DataSave0.RedoTrans.ucRedoFlag=0;
				ucResult=XDATA_Write_Vaild_File(DataSaveRedoTrans);
				return ucResult;
			}
			if((NormalTransData.ucCardType==CARD_CPU&&NormalTransData.ulPrevEDBalance!=0
			&&(DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_RETURNCARD||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_RETURNCARD_P
			||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_EXPPURCHASE))
			||(NormalTransData.ucCardType==CARD_M1&&(NormalTransData.ulPrevEDBalance!=0||SleTransData.ucStatus!=CARD_RECYCLED)
			&&(DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_RETURNCARD||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_RETURNCARD_P
			||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_EXPPURCHASE))
			||(NormalTransData.ulPrevEDBalance!=DataSave0.RedoTrans.Tran.ulPrevEDBalance-DataSave0.RedoTrans.Tran.ulAmount
			&&DataSave0.RedoTrans.Tran.ucTransType!=TRANS_S_RETURNCARD&&DataSave0.RedoTrans.Tran.ucTransType!=TRANS_S_RETURNCARD_P
			&&DataSave0.RedoTrans.Tran.ucTransType!=TRANS_S_EXPPURCHASE))
			{
				ucResult=SAV_KyTran(1);
				return ucResult;
			}
			else
			{
				if(NormalTransData.ucCardType==CARD_M1)
				{
					if(NormalTransData.ucTransPoint==DataSave0.RedoTrans.Tran.ucTransPoint+1
						||(RunData.ucCardVer1==0xAA&&NormalTransData.ucNewOrOld==1
						&&NormalTransData.ucTransPoint==1&&DataSave0.RedoTrans.Tran.ucTransPoint==9)
						||(RunData.ucCardVer1==0xAA&&NormalTransData.ucNewOrOld==0
						&&NormalTransData.ucTransPoint==1&&DataSave0.RedoTrans.Tran.ucTransPoint==15)
						||(RunData.ucCardVer1==0x55&&NormalTransData.ucTransPoint==1&&DataSave0.RedoTrans.Tran.ucTransPoint==3))
					{
						ucResult = MIFARE_ReadCardDetail();
						if(ucResult==SUCCESS)
						{
							if(Untouch_Detail_Compare(&MifareCommandData.aucCommandOut[1],DataSave0.RedoTrans.Tran)==SUCCESS)
							{
								ucResult=SAV_KyTran(0);
								if(ucResult==SUCCESS)
									PRT_PrintTicket();
								return ERR_END;
							}
							else
							{
								ucResult=SAV_KyTran(1);
								return ucResult;
							}
						}
						else
							return ucResult;
					}
				}
				else if(NormalTransData.ucCardType==CARD_CPU)
				{
					if(ucResult==SUCCESS)
						ucResult=PBOC_ISOReadRecordbySFI(ICC1, 0x18, 1);
					if(ucResult==SUCCESS)
					{
						ulAmount=DataSave0.RedoTrans.Tran.ulAmount;
						if(RunData.aucFunFlag[1])
							ulPrevEDBalance=DataSave0.RedoTrans.Tran.ulPrevEDBalance+DataSave0.RedoTrans.Tran.ulYaAmount;
						else
						{
							ulPrevEDBalance=DataSave0.RedoTrans.Tran.ulPrevEDBalance;
							if(DataSave0.RedoTrans.Tran.ucReturnflag==true)
							{
								ulAmount=ulPrevEDBalance;
							}
						}
						asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucOnlytermcode,8);
						long_tab(&aucBuf[4],4,&ulAmount);
						if(memcmp(aucBuf,&Iso7816Out.aucData[10],4)==0
							&&memcmp(&aucBuf[4],&Iso7816Out.aucData[5],4)==0
							&&memcmp(DataSave0.RedoTrans.Tran.aucDateTime,&Iso7816Out.aucData[16],7)==0)
						{
							if(SleTransData.ucStatus!=CARD_RECYCLED)
							{
								if(DataSave0.RedoTrans.Tran.ucReturnflag==true)
								{
									if(ucResult==SUCCESS)
									{
										ucResult=PBOC_ISOGetChallenge(ICC1);
									}
									if(ucResult==SUCCESS)
									{
										memset(aucBuf,0,sizeof(aucBuf));
										memcpy(aucBuf,Iso7816Out.aucData,4);
										memset(aucFensan,0,sizeof(aucFensan));
										memcpy(aucFensan,NormalTransData.aucCardSerial,4);
										memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
										PBOC_Fensan_key(aucFensan,
											KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
										des(aucBuf,aucBuf,aucUseKey);
										desm1(aucBuf,aucBuf,&aucUseKey[8]);
										des(aucBuf,aucBuf,aucUseKey);
										ucResult=PBOC_ISOExternalauth(ICC1, aucBuf);
									}
									if(ucResult==SUCCESS)
									{
										aucBuf[0]=CARD_RECYCLED;
										ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
									}
								}
						

							}
							ucResult=SAV_KyTran(0);
							if(ucResult==SUCCESS)
								PRT_PrintTicket();
							return ERR_END;
						}
						else
						{
							ucResult=SAV_KyTran(1);
							return ucResult;
						}
							
					}
				}
				
			}
		case 2:
			if(memcmp(NormalTransData.aucUserCardNo,DataSave0.RedoTrans.Tran.aucUserCardNo,8)
				||NormalTransData.ucCardType==CARD_MEM||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_PRELOAD
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_REFUND||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_CHANGECD
				||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_LOADONLINE||DataSave0.RedoTrans.Tran.ucTransType==TRANS_S_CASHLOAD)
			{
				ucResult=SAV_KyTran(1);
				return ucResult;
			}
			for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
			{
				if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==0)
				{
					uiIndex--;
					break;
				}
			}
			if(uiIndex>=0)
				XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
			if(uiIndex<0
				||DataSave0.TransInfoData.TransTotal.uiTotalNb==0
				||memcmp(DataSave0.SaveTransData.aucUserCardNo,DataSave0.RedoTrans.Tran.aucUserCardNo,8)
				||DataSave0.SaveTransData.ulAmount!=DataSave0.RedoTrans.Tran.ulAmount
				||memcmp(DataSave0.SaveTransData.aucDateTime,DataSave0.RedoTrans.Tran.aucDateTime,7)
				||DataSave0.SaveTransData.ulTraceNumber!=DataSave0.RedoTrans.Tran.ulTraceNumber)
			{
				ucResult=SAV_KyTran(0);
			}
			else
			{
				DataSave0.RedoTrans.ucRedoFlag=0;
				XDATA_Write_Vaild_File(DataSaveRedoTrans);
				memcpy(&NormalTransData,&DataSave0.RedoTrans.Tran,sizeof(NORMALTRANS));
			}
			if(ucResult==SUCCESS)
				PRT_PrintTicket();
			return ERR_END;
		case 3:
			if(memcmp(NormalTransData.aucUserCardNo,DataSave0.RedoTrans.Tran.aucUserCardNo,8))
			{
				ucResult=SAV_KyTran(1);
				return ucResult;
			}
			for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
			{
				if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==0)
				{
					uiIndex--;
					break;
				}
			}
			if(uiIndex>=0)
				XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
			if(uiIndex<0
				||DataSave0.TransInfoData.TransTotal.uiTotalNb==0
				||memcmp(DataSave0.SaveTransData.aucUserCardNo,DataSave0.RedoTrans.Tran.aucUserCardNo,8)
				||DataSave0.SaveTransData.ulAmount!=DataSave0.RedoTrans.Tran.ulAmount
				||memcmp(DataSave0.SaveTransData.aucDateTime,DataSave0.RedoTrans.Tran.aucDateTime,7)
				||DataSave0.SaveTransData.ulTraceNumber!=DataSave0.RedoTrans.Tran.ulTraceNumber)
			{
				ucResult=SAV_KyTran(0);
			}
			else
			{
				DataSave0.RedoTrans.ucRedoFlag=0;
				ucResult=XDATA_Write_Vaild_File(DataSaveRedoTrans);
				memcpy(&NormalTransData,&DataSave0.RedoTrans.Tran,sizeof(NORMALTRANS));
			}
			if(ucResult==SUCCESS)
				PRT_PrintTicket();
			return ERR_END;
		default:
			DataSave0.RedoTrans.ucRedoFlag=0;
			ucResult=XDATA_Write_Vaild_File(DataSaveRedoTrans);
			return ucResult;
		
	}
}
unsigned char Untouch_CkeckReload(void)
{
	int uiIndex,i,j;
	unsigned char ucResult=SUCCESS,aucBuf[20];
	unsigned char ucStatus,ucLen,aucOutData[30];
	NORMALTRANS tnormal;
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		 memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
		if(DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
                     ucResult=XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
			if(ucResult!=SUCCESS)
				return ucResult;
			if( DataSave0.SaveTransData.ucKYFlag==1
				&&DataSave0.SaveTransData.ucTransType==TRANS_S_PRELOAD
				&&!memcmp(DataSave0.SaveTransData.aucUserCardNo,NormalTransData.aucUserCardNo,8)
				)
			{
				if(NormalTransData.ucCardType==CARD_CPU)
				{
					//if(RunData.ucDxMobile)
					//	ucResult=PBOC_ISOSelectFilebyFID(ICC1,(UCHAR *)"\x6F\x20");
					if(ucResult==SUCCESS)
					{
						for(i=1;i<=10;i++)
						{
							if(PBOC_ISOReadRecordbySFI(ICC1, 0x18, i)==SUCCESS)
							{
								asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucOnlytermcode,8);
								long_tab(&aucBuf[4],4,&DataSave0.SaveTransData.ulAmount);
								Uart_Printf("aucbuf:\n");
								for(j=0;j<8;j++)
								{
									Uart_Printf("%02x ",aucBuf[j]);
								}
								Uart_Printf("\n");
								Uart_Printf("DataSave0.SaveTransData.aucDateTime:\n");
								for(j=0;j<7;j++)
								{
									Uart_Printf("%02x ",DataSave0.SaveTransData.aucDateTime[j]);
								}
								Uart_Printf("\n");
								if(memcmp(aucBuf,&Iso7816Out.aucData[10],4)==0
									&&memcmp(&aucBuf[4],&Iso7816Out.aucData[5],4)==0
									&&memcmp(DataSave0.SaveTransData.aucDateTime,&Iso7816Out.aucData[16],7)==0)
									//&&memcmp(&DataSave0.SaveTransData.aucSamTace[2],Iso7816Out.aucData,2)==0
								{
									DataSave0.SaveTransData.ucKYFlag=0;
									memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
									PBOC_ISOGetTac(0x06,&NormalTransData.aucSamTace[2]);
									memcpy(DataSave0.SaveTransData.aucTraceTac,Iso7816Out.aucData,4);
									ucResult=XDATA_Write_SaveTrans_File(uiIndex);
									DataSave0.TransInfoData.TransTotal.uiLoadNb+=1;
									DataSave0.TransInfoData.TransTotal.ulLoadAmount+=NORMALTRANS_ulAmount;
									DataSave0.TransInfoData.PtInfoData[NormalTransData.ucPtCode-1].uiLoadNb++;
									DataSave0.TransInfoData.PtInfoData[NormalTransData.ucPtCode-1].ulLoadAmount+=NormalTransData.ulAmount;
									if(XDATA_Write_Vaild_File(DataSaveTransInfo)!=SUCCESS)
									{
										//Os__GB2312_display(1,0,"ۼʧ");
										MSG_WaitKey(2);
									}
								 	if(SAV_IssueInfo(NormalTransData.aucZoneName)!=SUCCESS)
									{
										//Os__GB2312_display(1,0,"ۼʧ");
										MSG_WaitKey(2);
									}
									if(ucResult==SUCCESS)
									{
										PRT_PrintTicket();
									}
									break;
								}

							}
						}
						if(i>10)
						{
							//
							memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
							ucResult=Trans_ReversalPreload();
							if(ucResult==SUCCESS)
							{
								DataSave0.TransInfoData.auiTransIndex[uiIndex]=0;
								XDATA_Write_Vaild_File(DataSaveTransInfo);
							}
						}
					}
				}
				else if(NormalTransData.ucCardType==CARD_M1)
				{
					if(NormalTransData.ulPrevEDBalance==DataSave0.SaveTransData.ulPrevEDBalance
						&&!memcmp(NormalTransData.aucSamTace,DataSave0.SaveTransData.aucSamTace,4))
					{
						//
						memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
						ucResult=Trans_ReversalPreload();
						if(ucResult==SUCCESS)
						{
							DataSave0.TransInfoData.auiTransIndex[uiIndex]=0;
							XDATA_Write_Vaild_File(DataSaveTransInfo);
						}
					}else
					{
						ucResult=MIFARE_ReadCardDetail();
						if(ucResult==SUCCESS)
						{
							if(Untouch_Detail_Compare(&MifareCommandData.aucCommandOut[1],DataSave0.RedoTrans.Tran)==SUCCESS)
							{
								if(NormalTransData.ulPrevEDBalance==DataSave0.SaveTransData.ulPrevEDBalance+DataSave0.SaveTransData.ulAmount)
								{
									DataSave0.SaveTransData.ucKYFlag=0;
									memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
									MIFARE_GetTAC();
									memcpy(DataSave0.SaveTransData.aucTraceTac,&MifareCommandData.aucCommandOut[1],4);
									ucResult=XDATA_Write_SaveTrans_File(uiIndex);
									DataSave0.TransInfoData.TransTotal.uiLoadNb+=1;
									DataSave0.TransInfoData.TransTotal.ulLoadAmount+=NORMALTRANS_ulAmount;
									DataSave0.TransInfoData.PtInfoData[NormalTransData.ucPtCode-1].uiLoadNb++;
									DataSave0.TransInfoData.PtInfoData[NormalTransData.ucPtCode-1].ulLoadAmount+=NormalTransData.ulAmount;
									if(XDATA_Write_Vaild_File(DataSaveTransInfo)!=SUCCESS)
									{
										//Os__GB2312_display(1,0,"ۼʧ");
										MSG_WaitKey(2);
									}
								 	if(SAV_IssueInfo(NormalTransData.aucZoneName)!=SUCCESS)
									{
										//Os__GB2312_display(1,0,"ۼʧ");
										MSG_WaitKey(2);
									}
									if(ucResult==SUCCESS)
									{
										PRT_PrintTicket();
									}
								}
							}
						}
					}

				}
				else if(NormalTransData.ucCardType==CARD_MEM)
				{
					asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucTerminalID,8);
					if(memcmp(SleTransDataSp.LoadRecord.aucTid,aucBuf,4)==0
						&&tab_long(SleTransDataSp.LoadRecord.aucAmount,4)==DataSave0.SaveTransData.ulAmount
						&&tab_long(SleTransDataSp.LoadRecord.aucPreBalance,4)==DataSave0.SaveTransData.ulTrueBalance
						&&memcmp(&SleTransDataSp.LoadRecord.aucTime,&DataSave0.SaveTransData.aucDateTime[1],6)==0)
					{
						if(NormalTransData.ulPrevEDBalance==DataSave0.SaveTransData.ulPrevEDBalance+DataSave0.SaveTransData.ulAmount)
						{
							DataSave0.SaveTransData.ucKYFlag=0;
							memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
							CFG_CalcPreLoadTAC();	
							memcpy(DataSave0.SaveTransData.aucTraceTac,NormalTransData.aucTraceTac,4);
							ucResult=XDATA_Write_SaveTrans_File(uiIndex);
							DataSave0.TransInfoData.TransTotal.uiLoadNb+=1;
							DataSave0.TransInfoData.TransTotal.ulLoadAmount+=NORMALTRANS_ulAmount;
							DataSave0.TransInfoData.PtInfoData[NormalTransData.ucPtCode-1].uiLoadNb++;
							DataSave0.TransInfoData.PtInfoData[NormalTransData.ucPtCode-1].ulLoadAmount+=NormalTransData.ulAmount;
							if(XDATA_Write_Vaild_File(DataSaveTransInfo)!=SUCCESS)
							{
								//Os__GB2312_display(1,0,"ۼʧ");
								MSG_WaitKey(2);
							}
						 	if(SAV_IssueInfo(NormalTransData.aucZoneName)!=SUCCESS)
							{
								//Os__GB2312_display(1,0,"ۼʧ");
								MSG_WaitKey(2);
							}
							if(ucResult==SUCCESS)
							{
								PRT_PrintTicket();
							}
						}
					}
 					else if(NormalTransData.ulPrevEDBalance==DataSave0.SaveTransData.ulPrevEDBalance
					&&!memcmp(NormalTransData.aucSamTace,DataSave0.SaveTransData.aucSamTace,4))
					{
						//
						memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
						ucResult=Trans_ReversalPreload();
						if(ucResult==SUCCESS)
						{
							DataSave0.TransInfoData.auiTransIndex[uiIndex]=0;
							XDATA_Write_Vaild_File(DataSaveTransInfo);
						}
					}


				}
			}
		}
		 memcpy(&NormalTransData,&tnormal,sizeof(NORMALTRANS));
	}
	return ucResult;
}
unsigned char Untouch_M1Debit(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20];
	unsigned long  ulsieral,ulPrevEDBalance;
	if(RunData.aucFunFlag[1])
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
	else
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance;
	while(1)
   	{
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ȡ!!!");

			memcpy(&DataSave0.RedoTrans.Tran,&NormalTransData,sizeof(NORMALTRANS));
			DataSave0.RedoTrans.ucRedoFlag=1;
			XDATA_Write_Vaild_File(DataSaveRedoTrans);

			Uart_Printf("\n 2012-05-02  MIFARE_DecreaseBlance before .....\n");
			ucResult = MIFARE_DecreaseBlance();
			Uart_Printf("MIFARE_ucOutStatus=%02x\n",MifareCommandData.aucCommandOut[0]);
			{
				unsigned char  i;
				Uart_Printf("\n***ѷص:*****\n");
				for(i = 0; i < 27; i++)
				{
				Uart_Printf("%02x ",MifareCommandData.aucCommandOut[i]);
				}
				Uart_Printf("\n");
			}
		}
		if(ucResult == SUCCESS
			&&(tab_long(&MifareCommandData.aucCommandOut[13],4)!=(ulPrevEDBalance-NormalTransData.ulAmount))
			&&NormalTransData.ucReturnflag!=true)
			ucResult=0x04;
		if(ucResult == SUCCESS
			&&(tab_long(&MifareCommandData.aucCommandOut[13],4)!=0)
			&&NormalTransData.ucReturnflag==true)
			ucResult=0x04;
		
		if( ucResult == SUCCESS)
		{
			DataSave0.RedoTrans.ucRedoFlag=2;
			XDATA_Write_Vaild_File(DataSaveRedoTrans);

			memcpy(&NormalTransData.aucSamTace[2],&MifareCommandData.aucCommandOut[19],2);
			if(NormalTransData.ucReturnflag==true)
			{
				MIFARE_GetTAC();
				memcpy(NormalTransData.aucTraceTac,&MifareCommandData.aucCommandOut[1],4);
			}
			else
				memcpy(NormalTransData.aucTraceTac,&MifareCommandData.aucCommandOut[21],4);

			 break;
		}
		else
		{
			if(MIFARE_ucOutStatus==0x0D||MIFARE_ucOutStatus==0x04||ucResult==0x04)
			{
				while(1)
				{
					ucResult = MIFARE_GetCardDetailInfo();
					Uart_Printf("Untouch_ReadPie2_tmp's ucResult=%02x\n",ucResult);
					if(MIFARE_ucOutStatus!=0x04)
						break;
					else
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"Żԭ!!!");
						Os__xdelay(10);

					}
				}
				 if(ucResult==SUCCESS)
			 	{
					memcpy(&M1TransData1,&MifareCommandData.aucCommandOut[1],sizeof(M1TransData1));
					if(tab_long(M1TransData1.aucBalance,4)!=ulPrevEDBalance)
					{
						if(tab_long(M1TransData1.aucBalance,4)==ulPrevEDBalance-NormalTransData.ulAmount)
						{
							NormalTransData.ucTransPoint=M1TransData1.ucPoint;
							while(1)
							{
								ucResult = MIFARE_ReadCardDetail();
								Uart_Printf("Untouch_ReadCardDetail_tmp's ucResult=%02x\n",ucResult);
								if(MIFARE_ucOutStatus!=0x04)
									break;
								else
								{
									//Os__clr_display(255);
									//Os__GB2312_display(0,0,(uchar *)"Żԭ!!!");
									Os__xdelay(10);
								}
							}
							if(ucResult==SUCCESS)
								ucResult=Untouch_Detail_Compare(&MifareCommandData.aucCommandOut[1],NormalTransData);
							if(ucResult==SUCCESS)
							{
								MIFARE_GetTAC();
								memcpy(NormalTransData.aucTraceTac,&MifareCommandData.aucCommandOut[1],4);
								Uart_Printf("Untouch_GetTAC's ucResult=%02x\n",ucResult);
								ulsieral=tab_long(NormalTransData.aucSamTace,4);
								ulsieral++;
								long_tab(NormalTransData.aucSamTace,4,&ulsieral);
								DataSave0.RedoTrans.ucRedoFlag=2;
								ucResult=XDATA_Write_Vaild_File(DataSaveRedoTrans);
							}
							break;
					 	}
						else
							return ERR_DIFFERENT_AMOUNT;

					}
				}
					
			 }
			 else
		 	{
			 	break;
		 	}
		}
				
			
		 if(ucResult!=SUCCESS)
		 {
			DataSave0.RedoTrans.ucRedoFlag=0;
			XDATA_Write_Vaild_File(DataSaveRedoTrans);
			return ucResult;
		 }
   	}
	return ucResult;
}
unsigned char Untouch_M1Credit(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],tCardNo[10];
	unsigned long ulMaxAmount,ulAmount,ulPrevEDBalance,ulsieral;
	unsigned char ucStatus,ucLen,aucOutData[300],ucKeyArray;
	ulPrevEDBalance=NormalTransData.ulTrueBalance;
	Uart_Printf("true balance=%d\n",ulPrevEDBalance);
	Uart_Printf("NormalTransData.ulAmount=%d\n",NormalTransData.ulAmount);
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ucNewOrOld==1)
		{
			ulAmount=tab_long(M1TransData1.aucLoadAmount, 4)+NORMALTRANS_ulAmount;
			long_tab(RunData.aucTotalLoad,4,&ulAmount);
			memset(aucBuf,0,sizeof(aucBuf));
			aucBuf[0]=RunData.aucTotalLoad[3];
			aucBuf[1]=RunData.aucTotalLoad[2];
			aucBuf[2]=RunData.aucTotalLoad[1];
			aucBuf[3]=RunData.aucTotalLoad[0];
			memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
			switch(NormalTransData.ucPtCode)
			{
				case 0x01:
					if(RunData.ucKeyArrey)
						ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
					else
						ucKeyArray=KEYARRAY_PURSECARD1;
					break;
				case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
				case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
				default:break;
			}
			PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, KEYINDEX_PURSECARD1_LOADMACKEY,aucBuf,aucBuf);
			PINPAD_47_Crypt8ByteSingleKey(ucKeyArray, KEYINDEX_PURSECARD1_LOADMACKEY+1,aucBuf,aucBuf);
			PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray, KEYINDEX_PURSECARD1_LOADMACKEY,aucBuf,RunData.aucLoadMac);
		}
	}
	while(1)
	{
		if(ucResult==SUCCESS)
		{
			DataSave0.ReversalISO8583Data.ucValid=0;	
			ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
			if(ucResult!=SUCCESS)
				return ucResult;
			memcpy(&DataSave0.RedoTrans.Tran,&NormalTransData,sizeof(NORMALTRANS));
			DataSave0.RedoTrans.ucRedoFlag=1;
			XDATA_Write_Vaild_File(DataSaveRedoTrans);

			ucResult=MIFARE_LoadBlance();
		}
		if(ucResult == SUCCESS
			&&(tab_long(&MifareCommandData.aucCommandOut[13],4)!=(ulPrevEDBalance+NormalTransData.ulAmount))
		)
			ucResult=0x0D; 
		if(ucResult==SUCCESS)
		{
			NormalTransData.ulAfterEDBalance=tab_long(&MifareCommandData.aucCommandOut[13],4);
			memcpy(&NormalTransData.aucSamTace[2],&MifareCommandData.aucCommandOut[19],2);
			memcpy(NormalTransData.aucTraceTac,&MifareCommandData.aucCommandOut[21],4);
			break;
		}else
		if(ucResult==0x0D||MIFARE_ucOutStatus==0x04||MIFARE_ucOutStatus==0x0D)
		{
			if(ucResult==0x04)
			{
				DataSave0.ReversalISO8583Data.ucValid=DATASAVEVALIDFLAG;	
				XDATA_Write_Vaild_File(DataSaveReversalISO8583);
			}
			while(1)
			{
				ucResult = MIFARE_GetCardDetailInfo();
				Uart_Printf("Untouch_ReadPie2_tmp's ucResult=%02x\n",ucResult);
				Uart_Printf("Untouch_ReadPie2_tmp's ucResult=%02x\n",ucResult);
				if(MIFARE_ucOutStatus!=0x04)
					break;
				else
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Żԭ!!!");
					Os__xdelay(10);

				}
			}
			 if(ucResult==SUCCESS)
		 	{
				memcpy(&M1TransData1,&MifareCommandData.aucCommandOut[1],sizeof(M1TransData1));
				if(tab_long(M1TransData1.aucBalance,4)!=ulPrevEDBalance)
				{
					if(tab_long(M1TransData1.aucBalance,4)==ulPrevEDBalance+NormalTransData.ulAmount)
					{
						NormalTransData.ucTransPoint=M1TransData1.ucPoint;
						while(1)
						{
							ucResult = MIFARE_ReadCardDetail();
							Uart_Printf("Untouch_ReadCardDetail_tmp's ucResult=%02x\n",ucResult);
							Uart_Printf("Untouch_ReadCardDetail_tmp's ucResult=%02x\n",ucResult);
							if(MIFARE_ucOutStatus!=0x04)
								break;
							else
							{
								//Os__clr_display(255);
								//Os__GB2312_display(0,0,(uchar *)"Żԭ!!!");
								Os__xdelay(10);
							}
						}
						if(ucResult==SUCCESS)
							ucResult=Untouch_Detail_Compare(&MifareCommandData.aucCommandOut[1],NormalTransData);
						if(ucResult==SUCCESS)
						{
							MIFARE_GetTAC();
							memcpy(NormalTransData.aucTraceTac,&MifareCommandData.aucCommandOut[1],4);
							Uart_Printf("Untouch_GetTAC's ucResult=%02x\n",ucResult);
							ulsieral=tab_long(NormalTransData.aucSamTace,4);
							ulsieral++;
							long_tab(NormalTransData.aucSamTace,4,&ulsieral);
						}
						break;
				 	}
					else
						return ERR_DIFFERENT_AMOUNT;

				}
			}
				
		 }
		 else
		 	return ucResult;
		if(ucResult!=SUCCESS)
			return ucResult;
 	}
	 return ucResult;
 }

unsigned char Untouch_FixedPurchase(void)
{
	unsigned char ucResult=SUCCESS,tCardNo[10];
	unsigned char aucBuf[30],ucStatus,aucBuf1[7],aucFensan[20],aucUseKey[20];
	unsigned char Date_Time[7],display[7],tmp_date[9],ucresult;
    	unsigned char month,day,hour,min,sec; 
	unsigned long ulAmount,ullong1,ullong2,ultTransnumber;
	unsigned char aucOutData[300];
 	unsigned char ucLen,ucJ,ucOffset,ucI,ucInput;
	int i;
	unsigned char aucBuf2[17];
	
	memset(aucBuf,0,sizeof(aucBuf));
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)":");
	ucResult=UTIL_InputAmount(1, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
	if(ucResult!=SUCCESS)
		return ucResult;
	while(1)
	{
		ucResult = SAV_SavedTransIndex(0);	
		if(ucResult!=SUCCESS)
			return ucResult;
		NormalTransData.ucTransType=TRANS_S_PURCHASE;
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"");
			//Os__GB2312_display(1,0,"()");
			//Os__GB2312_display(2,0,(uchar *)":");
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, ulAmount, 2);
			Os__display(2,5,aucBuf);
			//Os__GB2312_display(3,0,"[ȷ]޸Ľ");
			ucResult=Untouch_WaitAllCard1();
			if(ucResult==ERR_CANCEL)
				break;
			if(ucResult==KEY_ENTER)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)":");
				UTIL_InputAmount(1, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
				ucResult=SUCCESS;
				continue;

			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult == SUCCESS )
					SLE4442_ATR(aucBuf);

				if(ucResult==SUCCESS)
					ucResult = SLE4442_ReadCardData();
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();
				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_ReadCardInfo();

				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();

				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

				if(ucResult==SUCCESS)
					ucResult=Untouch_CheckRedo();

			}
			if(NormalTransData.ucCardType==CARD_MAG)
			{
				if(ucResult==SUCCESS)
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
					asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[20],2);
					asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[22],6);
					NormalTransData.aucExpiredDate[0]=0x20;
					asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
					ucResult=CFG_CheckCardValid();
				}
			}
		}
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[0])
				ucResult=ERR_NOFUNCTION;
		}
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,NormalTransData.aucZoneCode,3);
			if(NormalTransData.ucTransType==TRANS_S_PURCHASE)
				aucBuf[3]=0x21;
			if(NormalTransData.ucTransType==TRANS_S_TRANSPURCHASE)
				aucBuf[3]=0xC8;
			NORMALTRANS_ulAmount=ulAmount;
			ucResult=CFG_CheckRate(aucBuf);
			if(ucResult==SUCCESS)
			{
				Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
				Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
				Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
				if(RunData.ucRateType1=='0')
					NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
				else
				{
					if(NORMALTRANS_ulAmount<=NormalTransData.ulRateAmount)
						ucResult= ERR_LESSBALANCE;
					NORMALTRANS_ulAmount=ulAmount;
				}
			}
		}	
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ulRateAmount!=0)
			{
				ucResult=UTIL_DisPayFeeInfo();
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
			{
				Uart_Printf("RunData.aucFunFlag[0]=%02x\n",RunData.aucFunFlag[0]);
				Uart_Printf("RunData.aucWDFunFlag[0]=%02x\n",RunData.aucWDFunFlag[0]);
				Uart_Printf("RunData.aucWDFunFlag[2]=%02x\n",RunData.aucWDFunFlag[2]);
				Uart_Printf("RunData.aucZDFunFlag[2]=%02x\n",RunData.aucZDFunFlag[2]);
				if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
				&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2]
				&&NORMALTRANS_ulAmount<=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount
				&&NormalTransData.ucMemFlag!=0x31
				&&NormalTransData.ulRateAmount==0
				&&NormalTransData.ucTransType!=TRANS_S_TRANSPURCHASE)
				{
					ucResult=UTIL_DispRecycleInfo(NormalTransData.ulPrevEDBalance);
				}
				else
					ucResult= ERR_LESSBALANCE;
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.aucFunFlag[18])
			{
				if(NormalTransData.ulAmount>MAXTESTAMOUNT)
					ucResult = ERR_MAXTESTAMOUNT;
			}
		}
		if(ucResult==SUCCESS)
		{
			ucResult=UTIL_IncreaseTraceNumber(0);
			if(ucResult==SUCCESS)
			{
				switch(NormalTransData.ucCardType)
				{
					case CARD_M1:ucResult = Untouch_M1Debit();break;
					case CARD_CPU:ucResult = PBOC_DebitPart1();break;
					case CARD_MEM:
						ucResult = SLE4442_CardTrans();
						if(ucResult!=SUCCESS)
						{
							DataSave0.RedoTrans.ucRedoFlag=1;
							XDATA_Write_Vaild_File(DataSaveRedoTrans);
						}

						break;
					default:break;
				}
			}
		}
		ultTransnumber=NormalTransData.ulTraceNumber;
		if(ucResult==SUCCESS)
			ucResult=SAV_PurchaseSave();
		if(ucResult==SUCCESS)
		{
			NormalTransData.ulTraceNumber=ultTransnumber;
			PRT_PrintTicket();
		}
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
		if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2])
		{
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_PreComm();
			}
			STL_SendOfflineBatch();
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_FinComm();
			}
		}
	}
	return ucResult;
}
unsigned char Untouch_Purchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount,ultTransnumber;
	NORMALTRANS tNormal;
	if(RunData.ucExpire1Flag)
		return ERR_EXPIREDATE;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			//Os__clr_display(255);
            //Os__GB2312_display(0,0,(uchar *)"余额:");
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			Os__display(0,5,aucBuf);
            //Os__GB2312_display(1,0,(uchar *)"消费金额:");
			ulAmount=asc_long(ReceiveTransData.aucAmount,12);
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, ulAmount, 2);
			Os__display(2,0,aucBuf);
            //Os__GB2312_display(3,0,(uchar *)"按[确认]继续");
			if(MSG_WaitKey(30)!=KEY_ENTER)
				ucResult= ERR_CANCEL;
			
		}
		else
		{
			if(DataSave0.ChangeParamData.ulFixAmount)
			{
				//Os__clr_display(255);
                //Os__GB2312_display(0,0,(uchar *)"余额:");
				memset(aucBuf,0,sizeof(aucBuf));
				UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
				Os__display(0,5,aucBuf);
                //Os__GB2312_display(1,0,(uchar *)"消费金额:");
				ulAmount=DataSave0.ChangeParamData.ulFixAmount;
				memset(aucBuf,0,sizeof(aucBuf));
				UTIL_Form_Montant(aucBuf, ulAmount, 2);
				Os__display(2,0,aucBuf);
                //Os__GB2312_display(3,0,(uchar *)"按[确认]继续");
				if(MSG_WaitKey(30)!=KEY_ENTER)
					ucResult= ERR_CANCEL;
			}
			else
			{
#if 0
                memset(aucBuf,0,sizeof(aucBuf));
                UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
#ifdef GUI_PROJECT
				memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
				ProUiFace.ProToUi.uiLines=2;
                memcpy(ProUiFace.ProToUi.aucLine1,"卡内余额:",9);
				memcpy(&ProUiFace.ProToUi.aucLine1[9],aucBuf,strlen((char*)aucBuf));
                memcpy(ProUiFace.ProToUi.aucLine2,"请输消费金额:   0.01",16);
                ulAmount=1;
				ucResult = 0;
				sleep(1);
#else
				//Os__clr_display(255);
				if(!RunData.ucDxMobile)
				{
                    //Os__GB2312_display(0,0,(uchar *)"卡内余额:");
					Os__display(1,0,aucBuf);
				}
                //Os__GB2312_display(2,0,(uchar *)"请输消费金额:");

				// 2012_TEST DEL
				ucResult=UTIL_InputAmount(3, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
#endif
#else
                ulAmount=ProUiFace.UiToPro.ulAmount;
                NormalTransData.ulPrevEDBalance = ProUiFace.UiToPro.ulPrevEDBalance;
#endif
            }
		}
	}
    Uart_Printf("******************ulAmount= %ld\n",ulAmount);
	if(RunData.ucDxMobile&&!DataSave0.ConstantParamData.ucYTJFlag)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[0])
				return ERR_CARDNOFUN;
		}
	}
	if(ucResult==ERR_VOIDDEPOSITAMOUNT&&RunData.aucWDFunFlag[5])
	{
		ucResult=Trans_Query();
		if(ucResult!=SUCCESS)
			ucResult=ERR_VOIDDEPOSITAMOUNT;
	}
	Uart_Printf("===============RunData.uiTimesOut81=%d================\n",RunData.uiTimesOut);
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		if(NormalTransData.ucTransType==TRANS_S_PURCHASE)
			aucBuf[3]=0x21;
		if(NormalTransData.ucTransType==TRANS_S_TRANSPURCHASE)
			aucBuf[3]=0xC8;
        NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult==SUCCESS)
		{
			Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
			Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
			Uart_Printf("RunData.ucRateType2=%02x\n",RunData.ucRateType2);
			Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
			if(RunData.ucRateType1=='0')
			{
				if(RunData.ucRateType2=='0')
					NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;
				else if(ulAmount>NormalTransData.ulRateAmount)
					NORMALTRANS_ulAmount=ulAmount-NormalTransData.ulRateAmount;
			}
			else
			{
				if(NORMALTRANS_ulAmount<=NormalTransData.ulRateAmount)
					ucResult= ERR_LESSBALANCE;
				NORMALTRANS_ulAmount=ulAmount;
			}
		}
	}	
	Uart_Printf("===============RunData.uiTimesOut82=%d================\n",RunData.uiTimesOut);
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
		}
		else if(DataSave0.ConstantParamData.ucKFCFlag)
		{
			//Os__clr_display(255);
			memset(aucBuf,0,sizeof(aucBuf));
            //Os__GB2312_display(0,0,(uchar *)"消费金额");
			UTIL_Form_Montant(aucBuf,ulAmount,2);	
			Os__display(1,5,aucBuf);
            //Os__GB2312_display(2,0,(unsigned char *)"请确认");
			if(Os__xget_key()!=KEY_ENTER)
			{
				return ERR_CANCEL;
			}
			
		}
	}

	if(ucResult==SUCCESS)
	{
        Uart_Printf("\nNORMALTRANS_ulAmount=%d, %d \n",NORMALTRANS_ulAmount,NormalTransData.ulPrevEDBalance);
		if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
		{
			Uart_Printf("RunData.aucFunFlag[0]=%02x\n",RunData.aucFunFlag[0]);
			Uart_Printf("RunData.aucWDFunFlag[0]=%02x\n",RunData.aucWDFunFlag[0]);
			Uart_Printf("RunData.aucWDFunFlag[2]=%02x\n",RunData.aucWDFunFlag[2]);
			Uart_Printf("RunData.aucZDFunFlag[2]=%02x\n",RunData.aucZDFunFlag[2]);
			if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
			&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2]
			&&NORMALTRANS_ulAmount<=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount
			&&NormalTransData.ucMemFlag!=0x31
			&&NormalTransData.ulRateAmount==0
			&&NormalTransData.ucTransType!=TRANS_S_TRANSPURCHASE)
			{
				ucResult=UTIL_DispRecycleInfo(NormalTransData.ulPrevEDBalance);
			}
			else
				ucResult= ERR_LESSBALANCE;

            Uart_Printf("\n 2012-07-17 -----000000000001=[%02x]",ucResult);
		}
	}

    Uart_Printf("\n 2012-07-17 -----000000000002=[%02x]",ucResult);
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.aucFunFlag[18]&&NormalTransData.ucTransType!=TRANS_S_TRANSPURCHASE)
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
    Uart_Printf("\n 2012-07-17 -----000000000003=[%02x]",ucResult);
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult==SUCCESS)
		{
            Uart_Printf("\n 2012-07-17 -----000000000003-001=NormalTransData.ucCardType[%02x]",NormalTransData.ucCardType);
            switch(NormalTransData.ucCardType)
			{
				case CARD_M1:ucResult = Untouch_M1Debit();break;
				case CARD_CPU:ucResult = PBOC_DebitPart1();break;
				case CARD_MEM:
					ucResult = SLE4442_CardTrans();
					if(ucResult!=SUCCESS)
					{
						DataSave0.RedoTrans.ucRedoFlag=1;
                        Uart_Printf("aaaaaaa%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n\n");
						XDATA_Write_Vaild_File(DataSaveRedoTrans);
                        Uart_Printf("aaaaaaaaaaa222222222%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n\n");
					}
					break;
				default:break;
			}
		}
	}
    Uart_Printf("\n 2012-07-17 -----000000000004=[%02x]",ucResult);

	if(ucResult==SUCCESS)
		ucResult=SAV_PurchaseSave();
    Uart_Printf("===============RunData.uiTimesOut83=%d================\n",ucResult);
    if(ucResult==SUCCESS&&(DataSave0.ChangeParamData.aucTermFlag[2])&&DataSave0.ConstantParamData.ucYTJFlag)
	{
        Uart_Printf("===============RunData.uiTimesOut84=================\n");
		memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		if(NormalTransData.ucTransType==TRANS_S_RETURNCARD)	
        {
            STL_SendReturn();
            Uart_Printf("===============RunData.uiTimesOut85================\n");
        }
		else
			STL_SendOfflineBatch();
        Uart_Printf("===============RunData.uiTimesOut86=================\n");

		memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
	}

	if(DataSave0.ConstantParamData.ucYTJFlag)
    {
        Port_SendPortResult(ucResult);
        Uart_Printf("===============RunData.uiTimesOut87===============\n");
    }

	if(ucResult==SUCCESS)
    {
        Uart_Printf("===============RunData.uiTimesOut88================\n");
		//int uiK=0;
		//for(uiK=0;uiK<2;uiK++)
		//Os__beep();
#ifdef GUI_PROJECT
        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"消费成功",16);
        sleep(1);
#endif
        Uart_Printf("===============Untouch_M1Debit----SUCCESS===============\n");
        PRT_PrintTicket();
	}
	if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2]&&!DataSave0.ConstantParamData.ucYTJFlag)
	{
		memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		if(NormalTransData.ucTransType!=TRANS_S_TRANSPURCHASE&&(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35))
		{
			COMMS_PreComm();
		}
		if(NormalTransData.ucTransType==TRANS_S_RETURNCARD)	
			STL_SendReturn();
		else if(NormalTransData.ucTransType==TRANS_S_TRANSPURCHASE)	
			STL_SendTransferPurchase();
		else
			STL_SendOfflineBatch();
		if(NormalTransData.ucTransType!=TRANS_S_TRANSPURCHASE&&(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35))
		{
			COMMS_FinComm();
		}
		memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
	}

	return ucResult;
}
unsigned char Untouch_PayNobar(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount,ultTransnumber,ulMaxPay;
	NORMALTRANS tNormal;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)":");
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			Os__display(0,5,aucBuf);
			//Os__GB2312_display(1,0,(uchar *)"ɷѽ:");
			ulAmount=asc_long(ReceiveTransData.aucAmount,12);
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, ulAmount, 2);
			Os__display(2,0,aucBuf);
			//Os__GB2312_display(3,0,(uchar *)"[ȷ]");
			if(MSG_WaitKey(30)!=KEY_ENTER)
				ucResult= ERR_CANCEL;
			
		}
		else
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)":");
			Os__display(1,0,aucBuf);
			//Os__GB2312_display(2,0,(uchar *)"ɷѽ:");
			ucResult=UTIL_InputAmount(3, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
		}
	}
	if(ucResult==ERR_VOIDDEPOSITAMOUNT&&RunData.aucWDFunFlag[5])
	{
		ucResult=Trans_Query();
		if(ucResult!=SUCCESS)
			ucResult=ERR_VOIDDEPOSITAMOUNT;
	}
	while(1)
	{
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,NormalTransData.aucZoneCode,3);

			aucBuf[3]=0x88;

			NORMALTRANS_ulAmount=ulAmount;
			ucResult=CFG_CheckRate(aucBuf);
			if(ucResult==SUCCESS)
			{
				Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
				Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
				Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
				if(RunData.ucRateType1=='0')
					NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
				else
				{
					if(NORMALTRANS_ulAmount<=NormalTransData.ulRateAmount)
						ucResult= ERR_LESSBALANCE;
					NORMALTRANS_ulAmount=ulAmount;
				}
			}
		}	
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ulRateAmount!=0)
			{
				ucResult=UTIL_DisPayFeeInfo();
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
			{
				if(!DataSave0.ConstantParamData.ucYTJFlag)
				{
					if(RunData.ucRateType=='1')
					{
						if(NormalTransData.ulPrevEDBalance<RunData.ulMinRate)
							ulMaxPay=0;
						else
						{
							if(NormalTransData.ulPrevEDBalance>RunData.ulRate/100)
								ulMaxPay=NormalTransData.ulPrevEDBalance-RunData.ulRate/100;
							else
								ulMaxPay=0;
						}
					}
					else
					{
						ulMaxPay=10000*NormalTransData.ulPrevEDBalance/(10000+RunData.ulRate);
						if(NormalTransData.ulPrevEDBalance<RunData.ulMinRate)
							ulMaxPay=0;
						else
						{
							if(ulMaxPay>NormalTransData.ulPrevEDBalance-RunData.ulMinRate)
								ulMaxPay=NormalTransData.ulPrevEDBalance-RunData.ulMinRate;
						}
						if(NormalTransData.ulPrevEDBalance>RunData.ulMaxRate)
						{
							if(ulMaxPay<NormalTransData.ulPrevEDBalance-RunData.ulMaxRate)
								ulMaxPay=NormalTransData.ulPrevEDBalance-RunData.ulMaxRate;
						}
					}
					ulMaxPay=ulMaxPay/10*10;
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"");
					//Os__GB2312_display(1,0,(uchar *)"ɽɷѽ:");
					memset(aucBuf,0,sizeof(aucBuf));
					UTIL_Form_Montant(aucBuf, ulMaxPay, 2);
					Os__display(2,5,aucBuf);
					//Os__GB2312_display(3,0,(uchar *)"[ȷ]-ۿ");
					if(MSG_WaitKey(60)==KEY_ENTER)
					{
						ulAmount=ulMaxPay;
						continue;
					}
				}
				ucResult= ERR_LESSBALANCE;
			}
		}
		break;
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult==SUCCESS)
		{
			switch(NormalTransData.ucCardType)
			{
				case CARD_M1:ucResult = Untouch_M1Debit();break;
				case CARD_CPU:ucResult = PBOC_DebitPart1();break;
				case CARD_MEM:
					ucResult = SLE4442_CardTrans();
					if(ucResult!=SUCCESS)
					{
						DataSave0.RedoTrans.ucRedoFlag=1;
						XDATA_Write_Vaild_File(DataSaveRedoTrans);
					}

					break;
				default:break;
			}
		}
	}
	ultTransnumber=NormalTransData.ulTraceNumber;
	if(ucResult==SUCCESS)
		ucResult=SAV_PayNobarSave();
	if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2]&&DataSave0.ConstantParamData.ucYTJFlag)
	{
		memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		STL_SendPayNobar();
		memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
	}
	if(DataSave0.ConstantParamData.ucYTJFlag)
		Port_SendPortResult(ucResult);
	if(ucResult==SUCCESS)
	{
		NormalTransData.ulTraceNumber=ultTransnumber;
		PRT_PrintTicket();
	}
	if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2]&&!DataSave0.ConstantParamData.ucYTJFlag)
	{
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		{
			COMMS_PreComm();
		}
		STL_SendPayNobar();
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		{
			COMMS_FinComm();
		}
	}
	return ucResult;

}
unsigned char Untouch_PTCPurchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount,ultTransnumber;
	NORMALTRANS tNormal;
	if(RunData.ucExpire1Flag)
		return ERR_EXPIREDATE;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ulMult==0)
		{
			ucResult=UTIL_SetMult();
		}
	}
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)":");
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			Os__display(0,5,aucBuf);
			//Os__GB2312_display(1,0,(uchar *)"ۿ:");
			ulAmount=asc_long(ReceiveTransData.aucAmount,12);
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, ulAmount, 2);
			Os__display(2,0,aucBuf);
			//Os__GB2312_display(3,0,(uchar *)"[ȷ]");
			if(MSG_WaitKey(30)!=KEY_ENTER)
				ucResult= ERR_CANCEL;
			
		}
		else
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)":");
			Os__display(1,0,aucBuf);
			//Os__GB2312_display(2,0,(uchar *)":");
			ucResult=UTIL_InputAmount(3, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
		}
	}
	if(ucResult==ERR_VOIDDEPOSITAMOUNT&&RunData.aucWDFunFlag[5])
	{
		ucResult=Trans_Query();
		if(ucResult!=SUCCESS)
			ucResult=ERR_VOIDDEPOSITAMOUNT;
	}
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ulMult)
		{
			if(ulAmount%DataSave0.ConstantParamData.ulMult)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"ۿΪ");
				memset(aucBuf,0,sizeof(aucBuf));
				UTIL_Form_Montant(aucBuf, DataSave0.ConstantParamData.ulMult, 2);
				memcpy(&aucBuf[11],(uchar *)"ı",6);
				//Os__GB2312_display(1,0,&aucBuf[1]);
				MSG_WaitKey(5);
				ucResult=ERR_CANCEL;
			}
		}
		
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);

		aucBuf[3]=0xAB;

		NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult==SUCCESS)
		{
			Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
			Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
			Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
			if(RunData.ucRateType1=='0')
				NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
			else
			{
				if(NORMALTRANS_ulAmount<=NormalTransData.ulRateAmount)
					ucResult= ERR_LESSBALANCE;
				NORMALTRANS_ulAmount=ulAmount;
			}
		}
	}	
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
		}
	}
	if(ucResult==SUCCESS)
	{
		if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
		{
			ucResult= ERR_LESSBALANCE;
		}
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult==SUCCESS)
		{
			switch(NormalTransData.ucCardType)
			{
				case CARD_M1:ucResult = Untouch_M1Debit();break;
				case CARD_CPU:ucResult = PBOC_DebitPart1();break;
				case CARD_MEM:
					ucResult = SLE4442_CardTrans();
					if(ucResult!=SUCCESS)
					{
						DataSave0.RedoTrans.ucRedoFlag=1;
						XDATA_Write_Vaild_File(DataSaveRedoTrans);
					}

					break;
				default:break;
			}
		}
	}
	ultTransnumber=NormalTransData.ulTraceNumber;
	if(ucResult==SUCCESS)
		ucResult=SAV_PTCPurchaseSave();
	if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2]&&DataSave0.ConstantParamData.ucYTJFlag)
	{
		memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		STL_SendPTCPurchase();
		memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
	}
	if(DataSave0.ConstantParamData.ucYTJFlag)
		Port_SendPortResult(ucResult);
	if(ucResult==SUCCESS)
	{
		NormalTransData.ulTraceNumber=ultTransnumber;
		PRT_PrintTicket();
	}
	if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2]&&!DataSave0.ConstantParamData.ucYTJFlag)
	{
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		{
			COMMS_PreComm();
		}
		STL_SendPTCPurchase();
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		{
			COMMS_FinComm();
		}
	}
	return ucResult;

}
unsigned char Untouch_Purchase_P(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],ucStatus,aucOutData[200],ucLen,KEY,ucpt;
	unsigned long ulAmount,ulPrevEDBalance;
	ucResult=SAV_SavedTransIndex(0);
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"γ㿨");
	Os__xdelay(10);
	Os__ICC_remove();
	memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,10);	//㿨
	ucpt=NormalTransData.ucPtCode;
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"ܽ");
		ucResult=UTIL_InputAmount(3, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
	}
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	while(1)
	{
		ucResult=SAV_SavedTransIndex(0);
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		NormalTransData.ucTransType=TRANS_S_PURCHASE_P;
		NormalTransData.ucReturnflag=0x00;
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			if(ucpt==0x01)
				//Os__GB2312_display(0,0,(uchar *)"Ρ");
			if(ucpt==0x02)
				//Os__GB2312_display(0,0,(uchar *)"˹ؿ");
			if(ucpt==0x03)
				//Os__GB2312_display(0,0,(uchar *)"Ա");
			//Os__GB2312_display(1,0,(uchar *)"<ȡ>˳");
			ucResult=Untouch_WaitAllCard();
			if(ucResult==ERR_CANCEL)
				break;
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MAG)
			{
				ucResult=ERR_CARDNOFUN;
			}
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult == SUCCESS )
					SLE4442_ATR(aucBuf);

				if(ucResult==SUCCESS)
					ucResult = SLE4442_ReadCardData();
				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();
				if(ucResult==SUCCESS)
					CFG_SetBatchNumber();
				if(ucResult==SUCCESS)
				{
					if(SleTransData.ucStatus==CARD_RECYCLED)
						ucResult=ERR_CARDRECYCLE;
					if(SleTransData.ucStatus==0x04)	
						ucResult=ERR_CARDNOACT;
				}

			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_ReadCardInfo();
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();

				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

				if(ucResult==SUCCESS)
					ucResult=Untouch_CheckRedo();
				if(ucResult==SUCCESS)
				{
					if(SleTransData.ucStatus==CARD_RECYCLED)
						ucResult=ERR_CARDRECYCLE;
					if(SleTransData.ucStatus==0x04)	
						ucResult=ERR_CARDNOACT;
				}

			}
			if(ucResult==SUCCESS)
			{
				if(NormalTransData.ucPtCode!=ucpt)
					ucResult=ERR_DIFPT;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.aucZDFunFlag[34]==0||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1))
					ucResult=ERR_NOFUNCTION;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.ucExpireFlag)
					ucResult=ERR_EXPIREDATE;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.ucExpire1Flag)
					ucResult=ERR_EXPIREDATE;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
				&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2]
				&&NormalTransData.ucMemFlag!=0x31)
				{
					ulPrevEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
				}
				else
					ulPrevEDBalance=NormalTransData.ulPrevEDBalance;
				if(ulPrevEDBalance==0)
					ucResult=ERR_LESSBALANCE;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
				&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2])
					NORMALTRANS_ulAmount=min(ulAmount,NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount);
				else
					NORMALTRANS_ulAmount=min(ulAmount,NormalTransData.ulPrevEDBalance);
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)":");
				UTIL_Form_Montant(aucBuf,NormalTransData.ulPrevEDBalance,2);	
				Os__display(0,5,aucBuf);
				if(DataSave0.ConstantParamData.ucAmoutInput==2)
					NORMALTRANS_ulAmount=NORMALTRANS_ulAmount/100*100;
				if(DataSave0.ConstantParamData.ucAmoutInput==1)
					NORMALTRANS_ulAmount=NORMALTRANS_ulAmount/10*10;
				//Os__GB2312_display(1,0,(unsigned char *)":");
				UTIL_Form_Montant(aucBuf,NORMALTRANS_ulAmount,2);	
				Os__display(1,5,aucBuf);
				//Os__GB2312_display(2,0,(unsigned char *)"[1]");
				//Os__GB2312_display(3,0,(unsigned char *)"[2]޸Ľ");
				KEY=0;
				while(1)
				{
					KEY=Os__xget_key();
					if((KEY=='1'&&NORMALTRANS_ulAmount!=0)||KEY=='2'||KEY==KEY_CLEAR)
						break;
				}
				if(KEY=='2')
				{
					memset(aucBuf,0,sizeof(aucBuf));
					UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)":");
					Os__display(1,0,aucBuf);
					//Os__GB2312_display(2,0,(uchar *)"ۿ:");
					while(1)
					{
						ucResult=UTIL_InputAmount(3, &NORMALTRANS_ulAmount,1,min(ulAmount,ulPrevEDBalance));
						if(ucResult!=ERR_VOIDDEPOSITAMOUNT)
							break;
					}
				}
				if(KEY==KEY_CLEAR)
					ucResult = ERR_CANCEL;
			}	
			if(ucResult==SUCCESS)
			{
				if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
				{
					Uart_Printf("RunData.aucFunFlag[0]=%02x\n",RunData.aucFunFlag[0]);
					Uart_Printf("RunData.aucWDFunFlag[0]=%02x\n",RunData.aucWDFunFlag[0]);
					Uart_Printf("RunData.aucWDFunFlag[2]=%02x\n",RunData.aucWDFunFlag[2]);
					Uart_Printf("RunData.aucZDFunFlag[2]=%02x\n",RunData.aucZDFunFlag[2]);
					if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
					&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2]
					&&NORMALTRANS_ulAmount<=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount
					&&NormalTransData.ucMemFlag!=0x31
					&&NormalTransData.ulRateAmount==0/*&&NormalTransData.ucCardType!=CARD_CPU*/)
					{
						ucResult=UTIL_DispRecycleInfo(NormalTransData.ulPrevEDBalance);
					}
					else
						ucResult= ERR_LESSBALANCE;
				}
			}
			if(ucResult==SUCCESS)
			{
				if(NormalTransData.aucFunFlag[18])
				{
					if(NormalTransData.ulAmount>MAXTESTAMOUNT)
						ucResult = ERR_MAXTESTAMOUNT;
				}
			}
			if(ucResult==SUCCESS)
			{
				ucResult=UTIL_IncreaseTraceNumber(0);
				if(ucResult==SUCCESS)
				{
					switch(NormalTransData.ucCardType)
					{
						case CARD_M1:ucResult = Untouch_M1Debit();break;
						case CARD_CPU:ucResult = PBOC_DebitPart1();break;
						case CARD_MEM:
							ucResult = SLE4442_CardTrans();
							if(ucResult!=SUCCESS)
							{
								DataSave0.RedoTrans.ucRedoFlag=1;
								XDATA_Write_Vaild_File(DataSaveRedoTrans);
							}

							break;
						default:break;
					}
				}
			}		
			if(ucResult==SUCCESS)
				ucResult=SAV_PurchaseSave_P();
			if( ucResult != SUCCESS)			
			{
				NORMALTRANS_ulAmount=0;
				MSG_DisplayErrMsg(ucResult);
			}
			if(ulAmount>NORMALTRANS_ulAmount)
				ulAmount-=NORMALTRANS_ulAmount;
			else
				ulAmount=0;
			if(ucResult==SUCCESS)
			{
				NormalTransData.ulNeedCash=ulAmount;
				PRT_PrintTicket();
			}
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)"γ");
			Os__xdelay(10);
			Os__ICC_remove();
			while(1)
			{
				ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
				if(ucResult==0x04)
				{
					ucResult= SUCCESS;
					break;
				}
				//Os__clr_display(1);
				//Os__GB2312_display(1,0,"ȡ");
			}
			if(ulAmount)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"ۿ");
				UTIL_DisplayAmount(1, ulAmount);
				//Os__GB2312_display(2,0,(uchar *)"1.");
				//Os__GB2312_display(3,0,(uchar *)"2.");
				KEY=0;
				while(KEY!='1'&&KEY!='2')
				{
					KEY=Os__xget_key();
				}
				if(KEY=='2')return SUCCESS;
			}
			else
				return SUCCESS;
		}	
	}

	return ucResult;

}
unsigned char Untouch_ZshPurchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount;
	NormalTransData.ucTransType=TRANS_S_ZSHPURCHASE;
	if(RunData.aucZDFunFlag[48]==0||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1))
		return ERR_NOFUNCTION;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)":");
		if(NormalTransData.ucNewOrOld==1)
			Os__display(0,9,(uchar *)"N");
		else
			Os__display(0,9,(uchar *)"O");
		if(NormalTransData.ucPtCode==0x02)
			Os__display(0,10,(uchar *)"S");
		else if(NormalTransData.ucPtCode==0x01)
			Os__display(0,10,(uchar *)"W");
		else
			Os__display(0,10,(uchar *)"M");
		Os__display(1,0,aucBuf);
		//Os__GB2312_display(2,0,(uchar *)":");
		ucResult=UTIL_InputAmount(3, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
	}
	/*if(ucResult==SUCCESS&&RunData.aucFunFlag[12]&&NormalTransData.ucPackFlag!=1)
	{
		PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
		PBOC_ISOGetChallenge(ICC1);
		Iso7816Out.aucData[4]=0;
		PBOC_Fensan_key(NormalTransData.aucUserCardNo,
			KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
		PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
		PBOC_ISOLock(aucMac);
	}ʧȻ*/
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		aucBuf[3]=0xA0;
		NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult!=SUCCESS)
			return ucResult;
		Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
		Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
		Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
		if(RunData.ucRateType1=='0')
			NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
		else
			NORMALTRANS_ulAmount=ulAmount;
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
			if(ucResult!=SUCCESS)
				return ucResult;
		}
	}	
	if(ucResult==SUCCESS)
	{
		if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
		{
			if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
			&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2]
			&&NORMALTRANS_ulAmount<=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount
			&&NormalTransData.ucMemFlag!=0x31
			&&NormalTransData.ulRateAmount==0/*&&NormalTransData.ucCardType!=CARD_CPU*/)
			{
				ucResult=UTIL_DispRecycleInfo(NormalTransData.ulPrevEDBalance);
			}
			else
				return ERR_LESSBALANCE;
		}
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult!=SUCCESS)
			return ucResult;
		switch(NormalTransData.ucCardType)
		{
			case CARD_M1:ucResult = Untouch_M1Debit();break;
			case CARD_CPU:ucResult = PBOC_DebitPart1();break;
			case CARD_MEM:ucResult = SLE4442_CardTrans();break;
			default:break;
		}
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_ZshPurchaseSave();
	if(ucResult==SUCCESS)
	{
		PRT_ZSH_PrintTicket();		
	}
	return ucResult;

}
unsigned char Untouch_ExpPurchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount,ultTransnumber;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		NormalTransData.ulAmount=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
		NormalTransData.ucReturnflag=true;
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult==SUCCESS)
		{
			switch(NormalTransData.ucCardType)
			{
				case CARD_M1:ucResult = Untouch_M1Debit();break;
				case CARD_CPU:ucResult = PBOC_DebitPart1();break;
				case CARD_MEM:
					ucResult = SLE4442_CardTrans();
					if(ucResult!=SUCCESS)
					{
						DataSave0.RedoTrans.ucRedoFlag=1;
						XDATA_Write_Vaild_File(DataSaveRedoTrans);
					}

					break;
				default:break;
			}
		}
	}
	ultTransnumber=NormalTransData.ulTraceNumber;
	if(ucResult==SUCCESS)
		ucResult=SAV_ExpPurchaseSave();
	if(ucResult==SUCCESS)
	{
		NormalTransData.ulTraceNumber=ultTransnumber;
		PRT_PrintTicket();
	}
	if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2]&&!DataSave0.ConstantParamData.ucYTJFlag)
	{
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		{
			COMMS_PreComm();
		}
		STL_SendExpPurchaseBatch();
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		{
			COMMS_FinComm();
		}
	}
	return ucResult;

}
unsigned char Untouch_OnlinePurchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],aucBuf1[20];
	unsigned long ulAmount;
	 short iLen,ivo;
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(NormalTransData.ucHandFlag)
	{
		while(1)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)" ");
				//Os__GB2312_display(2,0,(uchar *)"뿨:");
				memset(aucBuf,0,sizeof(aucBuf));
				if (UTIL_Input(3,true,16,16,'N',aucBuf,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,aucBuf,16);
				}
			}
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(uchar *)"ٴ뿨");
				memset(aucBuf1,0,sizeof(aucBuf1));
				if (UTIL_Input(3,true,16,16,'N',aucBuf1,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					if(memcmp(aucBuf,aucBuf1,16))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"2벻һ");
						MSG_WaitKey(4);
					}
					else
						break;
				}
			}
			if(ucResult!=SUCCESS)
				break;
		}	
		if(ucResult==SUCCESS)
			ucResult=CFG_SetBatchNumber();
		if(ucResult==SUCCESS)
		{
			ucResult=CFG_CheckCardTbl4Hand();
		}
		if(ucResult==SUCCESS)
		{
			RunData.ucPtcode=NormalTransData.ucPtCode;
			ucResult=CashierLogonProcess();
		}

	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"ѽ:");
		ucResult=UTIL_InputAmount(3, &ulAmount,1,999999999);
	}
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[20])
				return ERR_CARDNOFUN;
		}
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
			aucBuf[3]=0x70;
		else
			aucBuf[3]=0x19;
		NormalTransData.ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult!=SUCCESS)
			return ucResult;
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
			NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
		else
			NORMALTRANS_ulAmount=ulAmount;
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
			if(ucResult!=SUCCESS)
				return ucResult;
		}
	}		
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_OnlinePurchase();
		if(ucResult==ERR_HOSTCODE&&RunData.ucErrorExtCode==152)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			NormalTransData.ulPrevEDBalance=asc_long(aucBuf,12);
			NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
			ucResult=UTIL_DispRecycleInfo(NormalTransData.ulPrevEDBalance);
			if(ucResult==SUCCESS)
				ucResult=Trans_OnlinePurchase();
		}
		if(ucResult==ERR_HOSTCODE&&RunData.ucErrorExtCode==3113)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
			RunData.ulValueble=asc_long(aucBuf,12);
		}
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
			ucResult=SAV_OnlinePurchaseSave();
		else
			ucResult=SAV_OnlinePurchaseSave_P();
	}
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}
unsigned char Untouch_OnlineZshPurchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount;
	NormalTransData.ucTransType=TRANS_S_ZSHONLINEPURCHASE;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)":");
		ucResult=UTIL_InputAmount(3, &ulAmount,1,999999999);
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		aucBuf[3]=0xA1;
		NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult!=SUCCESS)
			return ucResult;
		Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
		Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
		Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
		if(RunData.ucRateType1=='0')
			NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
		else
			NORMALTRANS_ulAmount=ulAmount;
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
			if(ucResult!=SUCCESS)
				return ucResult;
		}
	}	
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_ZshOnlinePurchase();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_ZshOnlinePurchaseSave();
	if(ucResult==SUCCESS)
	{
		PRT_ZSH_PrintTicket();		
	}
	return ucResult;

}
unsigned char Untouch_OnlineJsZshPurchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount;
	NormalTransData.ucTransType=TRANS_S_JSZSHONLINEPURCHASE;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)":");
		ucResult=UTIL_InputAmount(3, &ulAmount,1,999999999);
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		aucBuf[3]=0xA2;
		NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult!=SUCCESS)
			return ucResult;
		Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
		Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
		Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
		if(RunData.ucRateType1=='0')
			NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
		else
			NORMALTRANS_ulAmount=ulAmount;
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
			if(ucResult!=SUCCESS)
				return ucResult;
		}
	}	
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_ZshOnlinePurchase();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_JsZshOnlinePurchaseSave();
	if(ucResult==SUCCESS)
	{
		PRT_ZSH_PrintTicket();		
	}
	return ucResult;

}
unsigned char Untouch_Pay(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount;
	while(1)
	{
		ucResult=SAV_SavedTransIndex(0);
		/*if(ucResult==SUCCESS&&RunData.aucFunFlag[12]&&NormalTransData.ucPackFlag!=1)
		{
			PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
			PBOC_ISOGetChallenge(ICC1);
			Iso7816Out.aucData[4]=0;
			PBOC_Fensan_key(NormalTransData.aucUserCardNo,
				KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
			PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
			PBOC_ISOLock(aucMac);
		}ʧȻ*/
		if(RunData.ucDxMobile)
		{
			if(ucResult==SUCCESS)
				ucResult=Untouch_DX_WaitCard();
			if(ucResult==SUCCESS)
			{
				if(!RunData.aucZDFunFlag[1])
					return ERR_CARDNOFUN;
			}
		}
		if(ucResult==SUCCESS)
			ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult==SUCCESS)
			ucResult = Scan_Process();
		Os__end_com3( );
		if(RunData.ulTotalAmount!=0)
			ucResult=SAV_SavedTransIndex(0);
		Uart_Printf("NormalTrans.ulTraceNumber(Out Scan)=%d\n",NormalTransData.ulTraceNumber);
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,NormalTransData.aucZoneCode,3);
			if(NormalTransData.ucTransType==TRANS_S_PAY)
				aucBuf[3]=0x22;
			NORMALTRANS_ulAmount=RunData.ulTotalAmount;
			NormalTransData.ulRateAmount=RunData.ulTotalRate;
			if(NormalTransData.ulRateAmount!=0)
			{
				ucResult=UTIL_DisPayFeeInfo();
				if(ucResult!=SUCCESS)
					return ucResult;
			}
		}	
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.aucFunFlag[18])
			{
				if(NormalTransData.ulAmount>MAXTESTAMOUNT)
					ucResult = ERR_MAXTESTAMOUNT;
			}
		}
		if(ucResult==SUCCESS)
		{
			switch(NormalTransData.ucCardType)
			{
				case CARD_M1:ucResult = Untouch_M1Debit();break;
				case CARD_CPU:ucResult = PBOC_DebitPart1();break;
				case CARD_MEM:ucResult = SLE4442_CardTrans();break;
				default:break;
			}
		}
		Uart_Printf("NormalTrans.ulTraceNumber(Before PaySave)=%d\n",NormalTransData.ulTraceNumber);
		if(ucResult==SUCCESS)
			ucResult=SAV_PaySave();
		if(ucResult==SUCCESS)
		{
			PRT_PrintTicket();
		}
		if(ucResult==SUCCESS)
		{
			memset(&RunData,0,sizeof(RUNDATA));
			NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"Ƿɷ");
			//Os__GB2312_display(1,0,"[ȷ] ");
			//Os__GB2312_display(2,0,"[]˳ ");
			if(MSG_WaitKey(30)!=KEY_ENTER)
				ucResult=ERR_CANCEL;
		}
		if(ucResult==SUCCESS)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,"ɷ");
				//Os__GB2312_display(1,0,"//ˢ");
				ucResult=Untouch_WaitAllCard();
				if(ucResult==ERR_CANCEL)
					break;
			}
			if(ucResult==SUCCESS)
			{
				if(NormalTransData.ucCardType==CARD_MEM)
				{
					if(ucResult == SUCCESS )
						SLE4442_ATR(aucBuf);

					if(ucResult==SUCCESS)
						ucResult = SLE4442_ReadCardData();
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();
					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();
					if(ucResult==SUCCESS)
						ucResult=Untouch_CheckRedo();

				}
				if(NormalTransData.ucCardType==CARD_M1)
				{
					if(ucResult==SUCCESS)
						ucResult=Untouch_ReadCardInfo();

					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();

					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();

					if(ucResult==SUCCESS)
						ucResult=Untouch_CheckRedo();

				}
				if(NormalTransData.ucCardType==CARD_MAG)
				{
					ucResult=ERR_CANCEL;
				}
			}
		}
		if(ucResult!=SUCCESS)
			break;
	}
	return ucResult;

}
unsigned char Untouch_OnlinePay(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],auctPin[8],auctCardNo[10];
	unsigned long ultBalance;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		NormalTransData.ucTransType=TRANS_S_QUERY;
		ucResult=Trans_Query();
		NormalTransData.ucTransType=TRANS_S_PAYONLINE;
	}
	if(ucResult==SUCCESS)
		NormalTransData.ulPrevEDBalance=RunData.ulValueble;
	if(ucResult==SUCCESS)//xiaobin,2011-05-24,add
	{
		while(1)
		{
			ucResult=SAV_SavedTransIndex(0);
			/*if(ucResult==SUCCESS&&RunData.aucFunFlag[12]&&NormalTransData.ucPackFlag!=1)
			{
				PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
				PBOC_ISOGetChallenge(ICC1);
				Iso7816Out.aucData[4]=0;
				PBOC_Fensan_key(NormalTransData.aucUserCardNo,
					KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
				PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
				PBOC_ISOLock(aucMac);
			}ʧȻ*/
			if(RunData.ucDxMobile)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_DX_WaitCard();
				if(ucResult==SUCCESS)
				{
					if(!RunData.aucZDFunFlag[21])
						return ERR_CARDNOFUN;
				}
			}
			if(ucResult==SUCCESS)
				ucResult = Scan_Process();
			Os__end_com3( );
			if(RunData.ulTotalAmount!=0)
				ucResult=SAV_SavedTransIndex(0);
			if(ucResult==SUCCESS)
			{
				NORMALTRANS_ulAmount=RunData.ulTotalAmount;
				NormalTransData.ulRateAmount=RunData.ulTotalRate;
				if(NormalTransData.ulRateAmount!=0)
				{
					ucResult=UTIL_DisPayFeeInfo();
					if(ucResult!=SUCCESS)
						return ucResult;
				}
			}	
			if(ucResult==SUCCESS)
			{
				if(NormalTransData.aucFunFlag[18])
				{
					if(NormalTransData.ulAmount>MAXTESTAMOUNT)
						ucResult = ERR_MAXTESTAMOUNT;
				}
			}
			if(ucResult==SUCCESS)
			{
				ucResult=Trans_OnlinePurchase();
			}
			if(ucResult==SUCCESS)
				ucResult=SAV_OnlinePaySave();
			if(ucResult==SUCCESS)
			{
				PRT_PrintTicket();
			}
			if(ucResult==SUCCESS)
			{
				RunData.ulTotalAmount=0;
				RunData.uiBarNum=0;
				RunData.ulTotalRate=0;
				memset(RunData.aucBarInfo,0,80);
				NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
				NormalTransData.ulPrevEDBalance-=NormalTransData.ulAmount;
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,"Ƿɷ");
				//Os__GB2312_display(1,0,"[ȷ] ");
				//Os__GB2312_display(2,0,"[]˳ ");
				if(MSG_WaitKey(30)!=KEY_ENTER)
					ucResult=ERR_CANCEL;
			}
			if(ucResult!=SUCCESS)
				break;
		}
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	return ucResult;

}
unsigned char Untouch_Mobile(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],tCardNo[10],ucInput;
	unsigned long ulAmount;
	NORMALTRANS tNormal;
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2, 0, "ȴûֻ");
		//Os__clr_display_pp(255);
		//Os__GB2312_display_pp(0, 0, "ֻ");
		memset(aucBuf,0,sizeof(aucBuf));
		if(UTIL_Input_pp(1, true, 11, 11, 'N',aucBuf )!=KEY_ENTER)
			return ERR_CANCEL;
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, "ûȷ");
		Os__display(1,0,aucBuf);
		//Os__GB2312_display(2, 0, "[1]ȷ");
		if(Os__xget_key()!='1')
			return ERR_CANCEL;
		memcpy(NormalTransData.aucBarCode,aucBuf,11);
	}
		
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)":");
		if(NormalTransData.ucNewOrOld==1)
			Os__display(0,9,(uchar *)"N");
		else
			Os__display(0,9,(uchar *)"O");
		if(NormalTransData.ucPtCode==0x02)
			Os__display(0,10,(uchar *)"S");
		else if(NormalTransData.ucPtCode==0x01)
			Os__display(0,10,(uchar *)"W");
		else
			Os__display(0,10,(uchar *)"M");
		Os__display(1,0,aucBuf);
//		//Os__GB2312_display(2,0,(uchar *)"ֵֻ:");
//		ucResult=UTIL_InputAmount(3, &ulAmount,1, DataSave0.ConstantParamData.ulMaxXiaoFeiAmount);
	}
#if 1
	if(ucResult==SUCCESS)
	{
		//Os__clr_display_pp(255);
		//Os__GB2312_display_pp(0,0,(uchar *)"  ڴ");
		//Os__GB2312_display_pp(1,0,(uchar *)"  Ժ...");
		//Os__GB2312_display(2,0,(uchar *)"ѡֵ:");
		//Os__GB2312_display(3,0,(uchar *)"1.50Ԫ  2.100Ԫ");
		ucInput=0;
		while(ucInput!='1'&&ucInput!='2'
			&&ucInput!=KEY_CLEAR&&ucInput!=ERR_APP_TIMEOUT)
				ucInput=MSG_WaitKey(60);
		switch(ucInput)
		{
			case '1':
				ulAmount=5000;
				memcpy(NormalTransData.aucProCode,"DFMBCHGVVV050CV",15);
				break;
			case '2':
				ulAmount=10000;
				memcpy(NormalTransData.aucProCode,"DFMBCHGVVV100CV",15);
				break;
			default:
				ucResult=ERR_CANCEL;
				break;
		}
	}
#endif
/*
	if(ucResult==SUCCESS&&RunData.aucFunFlag[12]&&NormalTransData.ucPackFlag!=1)
	{
		PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
		PBOC_ISOGetChallenge(ICC1);
		Iso7816Out.aucData[4]=0;
		PBOC_Fensan_key(NormalTransData.aucUserCardNo,
			KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
		PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
		PBOC_ISOLock(aucMac);
	}ʧȻ
*/
	if(ucResult==SUCCESS&&RunData.ucInputmode==0&&NormalTransData.ucCardType!=CARD_MEM)//xiaobin,2011-05-23,add
	{
		memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
		while(1)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"Żԭ");
			ucResult=MIFARE_WaitCard();
			if(ucResult!=SUCCESS)
				continue;
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"Ժ");
			ucResult=Untouch_ReadCardInfo();
			if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Żԭ");
				continue;
			}
			break;
		}
		if(ucResult==SUCCESS)
		{
			if(RunData.aucFunFlag[1])
			{
				if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
					NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
			}
		}
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		aucBuf[3]=0x27;
		NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult!=SUCCESS)
			return ucResult;
		Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
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
		Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
		Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
		if(RunData.ucRateType1=='0')
			NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
		else
			NORMALTRANS_ulAmount=ulAmount;
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
			if(ucResult!=SUCCESS)
				return ucResult;
		}
	}	
	if(ucResult==SUCCESS)
	{
		if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
		{
			if(RunData.aucFunFlag[0]&&RunData.aucWDFunFlag[0]
			&&RunData.aucWDFunFlag[2]&&RunData.aucZDFunFlag[2]
			&&NORMALTRANS_ulAmount<=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount
			&&NormalTransData.ucMemFlag!=0x31
			&&NormalTransData.ulRateAmount==0/*&&NormalTransData.ucCardType!=CARD_CPU*/)
			{
				ucResult=UTIL_DispRecycleInfo(NormalTransData.ulPrevEDBalance);
			}
			else
				return ERR_LESSBALANCE;
		}
	}
	if(ucResult==SUCCESS)//xiaobin,2011-05-24,add
	{
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
		if(ucResult!=SUCCESS)
			return ucResult;
		switch(NormalTransData.ucCardType)
		{
			case CARD_M1:ucResult = Untouch_M1Debit();break;
			case CARD_CPU:ucResult = PBOC_DebitPart1();break;
			case CARD_MEM:ucResult = SLE4442_CardTrans();break;
			default:break;
		}
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_MobileSave();//xiaobin,2011-05-09,modify
	if(ucResult==SUCCESS)//xiaobin,2011-05-09,modify
	{
		PRT_PrintTicket();
	}
	if(ucResult==SUCCESS)
	{
		Uart_Printf("ͽ\n");
		memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		ucResult=STL_SendOfflineBatch_M();//xiaobin,2011-05-09,add
		memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
	}
	if(ucResult==SUCCESS)
	{
		RunData.ulOldTraceNumber=NormalTransData.ulTraceNumber;
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		ucResult=Trans_Mobile();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"ֵɹ");
	}
	return ucResult;

}
unsigned char Untouch_MobileOnline(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],ucInput;
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2, 0, "ȴûֻ");
		//Os__clr_display_pp(255);
		//Os__GB2312_display_pp(0, 0, "ֻ");
		memset(aucBuf,0,sizeof(aucBuf));
		if(UTIL_Input_pp(1, true, 11, 11, 'N',aucBuf )!=KEY_ENTER)
			return ERR_CANCEL;
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, "ûȷ");
		Os__display(1,0,aucBuf);
		//Os__GB2312_display(2, 0, "[1]ȷ");
		if(Os__xget_key()!='1')
			return ERR_CANCEL;
		memcpy(NormalTransData.aucBarCode,aucBuf,11);
	}
/*
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"ֵֻ:");
		ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1,999999999);
	}
*/
#if 1
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__clr_display_pp(255);
		//Os__GB2312_display_pp(0,0,(uchar *)"  ڴ");
		//Os__GB2312_display_pp(1,0,(uchar *)"  Ժ...");
		//Os__GB2312_display(2,0,(uchar *)"ѡֵ:");
		//Os__GB2312_display(3,0,(uchar *)"1.50Ԫ  2.100Ԫ");
		ucInput=0;
		while(ucInput!='1'&&ucInput!='2'
			&&ucInput!=KEY_CLEAR&&ucInput!=ERR_APP_TIMEOUT)
				ucInput=MSG_WaitKey(60);
		switch(ucInput)
		{
			case '1':
				NormalTransData.ulAmount=5000;
				memcpy(NormalTransData.aucProCode,"DFMBCHGVVV050CV",15);
				break;
			case '2':
				NormalTransData.ulAmount=10000;
				memcpy(NormalTransData.aucProCode,"DFMBCHGVVV100CV",15);
				break;
			default:
				ucResult=ERR_CANCEL;
				break;
		}
	}
#endif
/*
	if(ucResult==SUCCESS&&RunData.aucFunFlag[12]&&NormalTransData.ucPackFlag!=1)
	{
		PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
		PBOC_ISOGetChallenge(ICC1);
		Iso7816Out.aucData[4]=0;
		PBOC_Fensan_key(NormalTransData.aucUserCardNo,
			KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
		PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
		PBOC_ISOLock(aucMac);
	}ʧȻ
*/
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)//xiaobin,2011-05-24,add
	{
		if(NormalTransData.aucFunFlag[18])
		{
			if(NormalTransData.ulAmount>MAXTESTAMOUNT)
				ucResult = ERR_MAXTESTAMOUNT;
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_OnlinePurchase();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_OnlineMobileSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	if(ucResult==SUCCESS)
	{
		RunData.ulOldTraceNumber=NormalTransData.ulTraceNumber;
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		ucResult=Trans_Mobile();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"ֵɹ");
	}
	return ucResult;

}
unsigned char Untouch_MobileResend(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	int uiIndex;
	NORMALTRANS tNormal;

	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	//Os__clr_display(255);
	memset(aucBuf,0,sizeof(aucBuf));
	//Os__GB2312_display(0, 0, "ۿˮ:");
	if( UTIL_Input(1,true,6,6,'N',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	RunData.ulOldTraceNumber=asc_long(aucBuf,6);
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			ucResult=XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(RunData.ulOldTraceNumber!= DataSave0.SaveTransData.ulTraceNumber)
			{
				continue;
			}
			if(DataSave0.SaveTransData.ucTransType!=TRANS_S_MOBILECHARGE
				&&DataSave0.SaveTransData.ucTransType!=TRANS_S_MOBILECHARGEONLINE)
			{
				return ERR_NOTRANS;
			}
			if( memcmp(DataSave0.SaveTransData.aucUserCardNo,NormalTransData.aucUserCardNo,10))
				return (ERR_NOTPROC);
			memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
				break;
		}
	}	
	if(uiIndex==TRANS_MAXNB)
		return ERR_NOTRANS;

	if(ucResult==SUCCESS)//xiaobin,2011-05-25,add
	{
		Uart_Printf("ͽ\n");
		memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		ucResult=STL_SendOfflineBatch_M();//xiaobin,2011-05-09,add
		memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
	}
	if(ucResult==SUCCESS)
	{
		RunData.ulOldTraceNumber=NormalTransData.ulTraceNumber;
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		ucResult=Trans_Mobile();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"ֵɹ");
	}
	return ucResult;

}
unsigned char Untouch_PreAuth(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"Ȩ:");
		ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1,999999999);
	}
	/*if(ucResult==SUCCESS&&RunData.aucFunFlag[12]=='1'&&NormalTransData.ucPackFlag!=1)
	{
		PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
		PBOC_ISOGetChallenge(ICC1);
		Iso7816Out.aucData[4]=0;
		PBOC_Fensan_key(NormalTransData.aucUserCardNo,
			KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
		PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
		PBOC_ISOLock(aucMac);
	}ʧȻ*/
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_PreAuth();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
			COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_PreAuthSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}


unsigned char Untouch_PreAuthVoid(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[30],aucBuf1[20];
	unsigned long ulPreBalance,ulAmount;
	ucResult = SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(NormalTransData.ucHandFlag)
	{
		while(1)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)" ԤȨ");
				//Os__GB2312_display(2,0,(uchar *)"뿨:");
				memset(aucBuf,0,sizeof(aucBuf));
				if (UTIL_Input(3,true,16,16,'N',aucBuf,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,aucBuf,16);
				}
			}
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(uchar *)"ٴ뿨");
				memset(aucBuf1,0,sizeof(aucBuf1));
				if (UTIL_Input(3,true,16,16,'N',aucBuf1,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					if(memcmp(aucBuf,aucBuf1,16))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"2벻һ");
						MSG_WaitKey(4);
					}
					else
						break;
				}
			}
			if(ucResult!=SUCCESS)
				break;
		}	
		if(ucResult==SUCCESS)
			ucResult=CFG_SetBatchNumber();
	}
	
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__GB2312_display(0, 0, "Ȩ:");
		if( UTIL_Input(1,true,14,14,'N',aucBuf,NULL) != KEY_ENTER )
		{
		       COMMS_FinComm();
			return(ERR_CANCEL);
		}
		memcpy(NormalTransData.aucAuthCode,aucBuf,14);
	}
	//RunData.ulOldTraceNumber=asc_long(aucBuf,6);	
	
	if(ucResult!=SUCCESS)
		return ucResult;
	//if((ucResult = UITL_displayCardNum())!= SUCCESS)
		//return ucResult;

	
	ucResult = UTIL_InputEncryptPIN();
	if(ucResult!=SUCCESS)
	{
	       COMMS_FinComm();
		return ERR_CANCEL;
	}
	
	
	ucResult=Trans_PreAuthVoid();
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	ucResult=SAV_PreAuthVoidSave();
	if(ucResult!=SUCCESS)
		return ucResult;
	
	PRT_PrintTicket();	
		
	return SUCCESS;
}
unsigned char Untouch_PreAuthFinish(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],aucBuf1[20];
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(NormalTransData.ucHandFlag)
	{
		while(1)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)" ԤȨ");
				//Os__GB2312_display(2,0,(uchar *)"뿨:");
				memset(aucBuf,0,sizeof(aucBuf));
				if (UTIL_Input(3,true,16,16,'N',aucBuf,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,aucBuf,16);
				}
			}
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(uchar *)"ٴ뿨");
				memset(aucBuf1,0,sizeof(aucBuf1));
				if (UTIL_Input(3,true,16,16,'N',aucBuf1,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					if(memcmp(aucBuf,aucBuf1,16))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"2벻һ");
						MSG_WaitKey(4);
					}
					else
						break;
				}
			}
			if(ucResult!=SUCCESS)
				break;
		}	
		if(ucResult==SUCCESS)
			ucResult=CFG_SetBatchNumber();
	}	
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"ɽ:");
		ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1,999999999);
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__GB2312_display(0, 0, "Ȩ:");
		if( UTIL_Input(1,true,14,14,'N',aucBuf,NULL) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		memcpy(NormalTransData.aucAuthCode,aucBuf,14);
	}
	/*if(ucResult==SUCCESS&&RunData.aucFunFlag[12]=='1'&&NormalTransData.ucPackFlag!=1)
	{
		PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
		PBOC_ISOGetChallenge(ICC1);
		Iso7816Out.aucData[4]=0;
		PBOC_Fensan_key(NormalTransData.aucUserCardNo,
			KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
		PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
		PBOC_ISOLock(aucMac);
	}ʧȻ*/
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_PreAuthFinish();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
			COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_PreAuthFinishSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}


unsigned char Untouch_PreAuthFinishVoid(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[30],aucBuf1[20];
	unsigned long ulPreBalance,ulAmount;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(NormalTransData.ucHandFlag)
	{
		while(1)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"ԤȨɳ");
				//Os__GB2312_display(2,0,(uchar *)"뿨:");
				memset(aucBuf,0,sizeof(aucBuf));
				if (UTIL_Input(3,true,16,16,'N',aucBuf,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,aucBuf,16);
				}
			}
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(uchar *)"ٴ뿨");
				memset(aucBuf1,0,sizeof(aucBuf1));
				if (UTIL_Input(3,true,16,16,'N',aucBuf1,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					if(memcmp(aucBuf,aucBuf1,16))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"2벻һ");
						MSG_WaitKey(4);
					}
					else
						break;
				}
			}
			if(ucResult!=SUCCESS)
				break;
		}	
		if(ucResult==SUCCESS)
			ucResult=CFG_SetBatchNumber();
	}		
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__GB2312_display(0, 0, "ˮ:");
		if( UTIL_Input(1,true,TRANS_TRACENUMLEN,TRANS_TRACENUMLEN,'N',
					aucBuf,NULL) != KEY_ENTER )
		{
	       	COMMS_FinComm();
			return(ERR_CANCEL);
		}
	}
	RunData.ulOldTraceNumber=asc_long(aucBuf,6);	
	ucResult = SAV_SavedTransIndex(1);
	
	if(ucResult!=SUCCESS)
		return ucResult;
	if((ucResult = UITL_displayCardNum())!= SUCCESS)
	{
	       COMMS_FinComm();
		return ucResult;
	}	
	ucResult = UTIL_InputEncryptPIN();
	if(ucResult!=SUCCESS)
	{
	       COMMS_FinComm();
		return ERR_CANCEL;
	}
	
	
	ucResult=Trans_PreAuthFinishVoid();
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	ucResult=SAV_PreAuthFinishVoidSave();
	if(ucResult!=SUCCESS)
		return ucResult;
	
	PRT_PrintTicket();	
		
	return SUCCESS;
}

unsigned char Untouch_Preload(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],tCardNo[10],aucUseKey[20],ucInput,aucData[100];
	unsigned long ulMaxAmount,ulAmount,ulPrevEDBalance,ulsieral,tBalance;
	unsigned char ucStatus,ucLen,aucOutData[300],ucKeyArray,aucFensan[10],tExp[4],aucRandom[8];
	if(RunData.aucFunFlag[1])
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
	else
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance;
	ulMaxAmount=999999999;
	ucResult=SAV_SavedTransIndex(0);
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[23])
				return ERR_CARDNOFUN;
		}
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Untouch_CkeckReload();
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)":");
		Os__display(1,0,aucBuf);
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			//Os__GB2312_display(2,0,(uchar *)"ֵ:");
			NormalTransData.ulAmount=asc_long(ReceiveTransData.aucAmount,12);
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulAmount, 2);
			Os__display(3,0,aucBuf);
			if(MSG_WaitKey(30)!=KEY_ENTER)
				ucResult= ERR_CANCEL;
			
		}
		else
		{
			//Os__GB2312_display(2,0,(uchar *)"ֵ:");
			ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,0, ulMaxAmount);
		}
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount>NormalTransData.ulMaxAmount)
			ucResult = ERR_OVERAMOUNT;
	}
	
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS&&NormalTransData.ulAmount==0)
	{
		ucResult=Trans_PreloadQuery();
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulAmount, 2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ɳ:");
			Os__display(1,0,aucBuf);
			//Os__GB2312_display(2,0,(uchar *)"[1]޸Ľ");
			//Os__GB2312_display(3,0,(uchar *)"[ȷ]");
			ucInput=0;
			while(ucInput!='1'&&ucInput!=KEY_ENTER
				&&ucInput!=KEY_CLEAR&&ucInput!=ERR_APP_TIMEOUT)
			{
				ucInput=MSG_WaitKey(60);
			}
			if(ucInput=='1')
			{
				//Os__GB2312_display(2,0,(uchar *)"ֵ:");
				ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, NormalTransData.ulAmount);
				if(ucResult==SUCCESS)
				{
					if(NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount>NormalTransData.ulMaxAmount)
						ucResult = ERR_OVERAMOUNT;
				}
			}else
			if(ucInput!=KEY_ENTER)
				ucResult=ERR_CANCEL;
		}
	}
	if(ucResult==SUCCESS&&RunData.ucInputmode==0&&NormalTransData.ucCardType!=CARD_MEM&&!RunData.ucDxMobile)
	{
		memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
		while(1)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"Żԭ");
			ucResult=MIFARE_WaitCard();
			if(ucResult!=SUCCESS)
				continue;
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"Ժ");
			ucResult=Untouch_ReadCardInfo();
			if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Żԭ");
				continue;
			}
			break;
		}
		if(ucResult==SUCCESS)
		{
			if(RunData.aucFunFlag[1])
			{
				if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
					NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
			}
		}
	}
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		if(ucResult==SUCCESS)
		{
			Uart_Printf("Ȧʼ\n");
			memset(aucBuf,0,sizeof(aucBuf));
			/*if(RunData.ucDxMobile)
				aucBuf[0]=0x01;
			else*/
				aucBuf[0]=0x00;
			long_tab(&aucBuf[1],4,&NormalTransData.ulAmount);
			asc_bcd(&aucBuf[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
			//memcpy(&aucBuf[5],&DataSave0.ConstantParamData.aucOnlytermcode[2],6);
			ucResult=PBOC_ISOInitForLoad(ICC1,aucBuf);
		}
		if(ucResult==SUCCESS)
		{
			memcpy(RunData.aucPreBalance,Iso7816Out.aucData,4);
			//NormalTransData.ulPrevEDBalance=tab_long(RunData.aucPreBalance,4);
			memcpy(RunData.aucTrace,&Iso7816Out.aucData[4],2);
			RunData.ucKeyVersion=Iso7816Out.aucData[6];
			RunData.ucArithId=Iso7816Out.aucData[7];
			memcpy(RunData.aucRandom,&Iso7816Out.aucData[8],4);
			memcpy(RunData.aucMAC1,&Iso7816Out.aucData[12],4);
			memcpy(&NormalTransData.aucSamTace[2],RunData.aucTrace,2);
		}
	}
	if(ucResult==SUCCESS)
		ucResult=Trans_Preload();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
			COMMS_FinComm();
	}
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		if(ucResult==SUCCESS)
		{
			NormalTransData.ulAfterEDBalance=tab_long(RunData.aucPreBalance,4)+NormalTransData.ulAmount;
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,NormalTransData.aucDateTime,7);
			memcpy(&aucBuf[7],RunData.aucMAC2,4);
			Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
								0, sizeof(unsigned char));	
			XDATA_Write_Vaild_File(DataSaveReversalISO8583);

			memcpy(&DataSave0.RedoTrans.Tran,&NormalTransData,sizeof(NORMALTRANS));
			DataSave0.RedoTrans.ucRedoFlag=1;
			XDATA_Write_Vaild_File(DataSaveRedoTrans);

			ucResult=PBOC_ISOLoad(ICC1, aucBuf, 11);

			if(ucResult!=SUCCESS)
			{
				if(RunData.ucNeedRedo==0&&ucResult!=0x04)
				{
					Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
									0x55, sizeof(unsigned char));	
					XDATA_Write_Vaild_File(DataSaveReversalISO8583);
					DataSave0.RedoTrans.ucRedoFlag=0;
					XDATA_Write_Vaild_File(DataSaveRedoTrans);
				}
			}
		}
		if(ucResult==SUCCESS)
		{
			DataSave0.RedoTrans.ucRedoFlag=2;
			XDATA_Write_Vaild_File(DataSaveRedoTrans);


			memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);
			memcpy(&NormalTransData.aucSamTace[2],RunData.aucTrace,2);
		}
		if(ucResult==SUCCESS)
		{
			PBOC_ISOGetChallenge(ICC1);
		}
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,Iso7816Out.aucData,4);
			memset(aucFensan,0,sizeof(aucFensan));
			memcpy(aucFensan,NormalTransData.aucCardSerial,4);
			memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
			PBOC_Fensan_key(aucFensan,
				KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
			des(aucBuf,aucBuf,aucUseKey);
			desm1(aucBuf,aucBuf,&aucUseKey[8]);
			des(aucBuf,aucBuf,aucUseKey);
			PBOC_ISOExternalauth(ICC1, aucBuf);
		}
		if(ucResult==SUCCESS)
		{
			if(!RunData.ucDxMobile)
				PBOC_UpdateBinaryFile(ICC1,0x15, 24, 4, NormalTransData.aucExpiredDate);
			else
			{
				PBOC_ISOGetChallenge(ICC1);
				memset(aucRandom,0,sizeof(aucRandom));
				memcpy(aucRandom,Iso7816Out.aucData,4);
				memcpy(aucData,"\x04\xD6\x95\x18\x08",5);
				memset(aucBuf,0,sizeof(aucBuf));
				memcpy(aucBuf,NormalTransData.aucExpiredDate,4);
				memcpy(&aucData[5],aucBuf,4);
				PBOC_MAC(aucRandom,aucUseKey,aucData,9,&aucBuf[4]);
				PBOC_UpdateBinaryFileMac(ICC1,0x15, 24, 4, aucBuf);
			}
		}
		if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
		{
			if(RunData.aucFunFlag[9])
				aucBuf[0]=0x01;
			else
				aucBuf[0]=0x04;
			ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
		}
	}
	if(NormalTransData.ucCardType==CARD_M1)
	{
		if(ucResult==SUCCESS)
		{
			memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
			tBalance=NormalTransData.ulPrevEDBalance;
			memcpy(tExp,NormalTransData.aucExpiredDate,4);
			while(1)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Żԭ");
				ucResult=MIFARE_WaitCard();
				if(ucResult!=SUCCESS)
					continue;
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Ժ");
				ucResult=Untouch_ReadCardInfo();
				if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)
					||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Żԭ");
					continue;
				}
				break;
			}
			memcpy(NormalTransData.aucExpiredDate,tExp,4);
		}
		if(ucResult==SUCCESS)
		{
			if(RunData.aucFunFlag[1])
			{
				if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
					NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
			}
			if(NormalTransData.ulPrevEDBalance!=tBalance)
				ucResult=ERR_DIFFERENT_BALANCE;
		}
		if(ucResult==SUCCESS)
			ucResult=Untouch_M1Credit();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucFunFlag[9]&&SleTransData.ucStatus==CARD_RECYCLED)
				RunData.ucTagetStatus=0x04;
			else
				RunData.ucTagetStatus=0x01;
			ucResult=MIFARE_ChangeStatus();
		}
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_LoadSave();
	if(DataSave0.ConstantParamData.ucYTJFlag)
		Port_SendPortResult(ucResult);
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}

unsigned char Untouch_Refund(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],tCardNo[10],aucUseKey[20];
	unsigned long ulMaxAmount,ulAmount,ulPrevEDBalance,ulsieral;
	unsigned char ucStatus,ucLen,aucOutData[300],ucKeyArray,aucFensan[10];
	if(RunData.aucFunFlag[1])
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
	else
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance;
	ulMaxAmount=999999999;
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[27])
				return ERR_CARDNOFUN;
		}
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ucCardType==CARD_CPU)
			ucResult=Trans_RefundQuery();
	}
	if(ucResult==SUCCESS&&RunData.ucInputmode==0&&NormalTransData.ucCardType!=CARD_MEM)
	{
		memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
		while(1)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"Żԭ");
			ucResult=MIFARE_WaitCard();
			if(ucResult!=SUCCESS)
				continue;
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"Ժ");
			ucResult=Untouch_ReadCardInfo();
			if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Żԭ");
				continue;
			}
			break;
		}
		if(ucResult==SUCCESS)
		{
			if(RunData.aucFunFlag[1])
			{
				if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
					NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
			}
		}
	}
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		if(ucResult==SUCCESS)
		{
			Uart_Printf("Ȧʼ\n");
			memset(aucBuf,0,sizeof(aucBuf));
			aucBuf[0]=0x00;
			long_tab(&aucBuf[1],4,&NormalTransData.ulAmount);
			asc_bcd(&aucBuf[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
			//memcpy(&aucBuf[5],&DataSave0.ConstantParamData.aucOnlytermcode[2],6);
			ucResult=PBOC_ISOInitForLoad(ICC1,aucBuf);
		}
		if(ucResult==SUCCESS)
		{
			memcpy(RunData.aucPreBalance,Iso7816Out.aucData,4);
			//NormalTransData.ulPrevEDBalance=tab_long(RunData.aucPreBalance,4);
			memcpy(RunData.aucTrace,&Iso7816Out.aucData[4],2);
			RunData.ucKeyVersion=Iso7816Out.aucData[6];
			RunData.ucArithId=Iso7816Out.aucData[7];
			memcpy(RunData.aucRandom,&Iso7816Out.aucData[8],4);
			memcpy(RunData.aucMAC1,&Iso7816Out.aucData[12],4);
		}
	}
	if(ucResult==SUCCESS)
		ucResult=Trans_Preload();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
			COMMS_FinComm();
	}
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		if(ucResult==SUCCESS)
		{
			NormalTransData.ulAfterEDBalance=tab_long(RunData.aucPreBalance,4)+NormalTransData.ulAmount;
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,NormalTransData.aucDateTime,7);
			memcpy(&aucBuf[7],RunData.aucMAC2,4);
			Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
								0, sizeof(unsigned char));	
			ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
			if(ucResult!=SUCCESS)
				return ucResult;
			ucResult=PBOC_ISOLoad(ICC1, aucBuf, 11);
			if(ucResult!=SUCCESS)
			{
				Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
								0x55, sizeof(unsigned char));	
				XDATA_Write_Vaild_File(DataSaveReversalISO8583);

			}
		}
		if(ucResult==SUCCESS)
		{
			memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);
			memcpy(&NormalTransData.aucSamTace[2],RunData.aucTrace,2);
		}
		if(ucResult==SUCCESS)
		{
			PBOC_ISOGetChallenge(ICC1);
		}
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,Iso7816Out.aucData,4);
			memset(aucFensan,0,sizeof(aucFensan));
			memcpy(aucFensan,NormalTransData.aucCardSerial,4);
			memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
			PBOC_Fensan_key(aucFensan,
				KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
			des(aucBuf,aucBuf,aucUseKey);
			desm1(aucBuf,aucBuf,&aucUseKey[8]);
			des(aucBuf,aucBuf,aucUseKey);
			PBOC_ISOExternalauth(ICC1, aucBuf);
		}
		if(ucResult==SUCCESS)
		{
			PBOC_UpdateBinaryFile(ICC1,0x15, 24, 4, NormalTransData.aucExpiredDate);
		}
		if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
		{
			aucBuf[0]=0x01;
			ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
		}
	}
	if(NormalTransData.ucCardType==CARD_M1)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_M1Credit();
		if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
		{
			RunData.ucTagetStatus=0x01;
			ucResult=MIFARE_ChangeStatus();
		}
	}
	if(NormalTransData.ucCardType==CARD_MEM)
	{
		if(ucResult==SUCCESS)
			ucResult = SLE4442_CardTrans();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_RefundSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}
unsigned char Untouch_OnlineRefund(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(RunData.aucWDFunFlag[7])
	{
		if(ucResult==SUCCESS)
		{
			RunData.ucPtcode=NormalTransData.ucPtCode;
			ucResult=CashierLogonProcess();
		}

	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"˻:");
		ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1,999999999);
	}
	/*if(ucResult==SUCCESS&&RunData.aucFunFlag[12]=='1'&&NormalTransData.ucPackFlag!=1)
	{
		PBOC_ISOSelectFilebyFID(ICC1, (uchar *)"\x3f\x01");
		PBOC_ISOGetChallenge(ICC1);
		Iso7816Out.aucData[4]=0;
		PBOC_Fensan_key(NormalTransData.aucUserCardNo,
			KEYINDEX_PURSECARD0_APPCTRLKEY,aucBuf);
		PBOC_MAC(Iso7816Out.aucData,aucBuf,(uchar *)"\x84\xCD\x00\x00\x04",5, aucMac);
		PBOC_ISOLock(aucMac);
	}ʧȻ*/
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5]&&RunData.aucFunFlag[11])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Trans_OnlineRefund();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
			COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
		ucResult=SAV_OnlineRefundSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}
unsigned char Untouch_Query(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[20];
	unsigned long ulAmount;
	short 			iLen;
	int i;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[40])
				return ERR_CARDNOFUN;
		}
	}
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS)
		ucResult=Trans_Query();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
			COMMS_FinComm();
	}
	if(ucResult==SUCCESS&&RunData.aucFunFlag[3])
	{
		ucResult=ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult==SUCCESS)
		{
			asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);
			NormalTransData.aucExpiredDate[0]=0x20;
		}
		
	}
	if(ucResult==SUCCESS)
	{
		if(RunData.aucFunFlag[17]==1)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf,RunData.ulCredit,2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"˶:");
			Os__display(1,0,aucBuf);
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf,RunData.ulBalance,2);
            //if(RunData.ucZfFlag=='1')
				//Os__GB2312_display(2,0,(uchar *)"Ƿ:");
        //	else
				//Os__GB2312_display(2,0,(uchar *)":");
			Os__display(2,5,aucBuf);
			memset(aucBuf,0,sizeof(aucBuf));
			/*if(RunData.ulValueble>=RunData.ulFreeze)
				ulAmount=RunData.ulValueble-RunData.ulFreeze;
			else
				ulAmount=0;*/
			ulAmount=RunData.ulValueble;
			UTIL_Form_Montant(aucBuf,ulAmount,2);
			//Os__GB2312_display(3,0,(uchar *)":");
			Os__display(3,5,aucBuf);
		}
		else
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ulAmount=RunData.ulValueble;
			UTIL_Form_Montant(aucBuf,ulAmount,2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)":");
			Os__display(1,0,aucBuf);
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,(unsigned char *)"Ч:",7);
			//Os__GB2312_display(2,0,aucBuf);
			memset(aucBuf,' ',sizeof(aucBuf)); 
			bcd_asc(&aucBuf[6],NormalTransData.aucExpiredDate,4);	
			aucBuf[10]='/';
			bcd_asc(&aucBuf[11],&NormalTransData.aucExpiredDate[2],2);	
			aucBuf[13]='/';
			bcd_asc(&aucBuf[14],&NormalTransData.aucExpiredDate[3],2);
			Uart_Printf("******aucBuf:%s\n",aucBuf);
			//Os__GB2312_display(3,0,aucBuf); 
		}
		MSG_WaitKey(30);
		//Os__clr_display(255);
	}
	return ucResult;
}
unsigned char Untouch_Sale(void)
{
	unsigned char aucBuf[200],aucOutData[200],aucPrintBuf[40],aucFensan[10],aucUseKey[20];
	unsigned char ucResult=SUCCESS,ucStatus,ucLen,ucOffset,index,ucI;
	int uiSale=0,uiDiscount,i;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=CashierLogonProcess();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	NormalTransData.ucTransType=TRANS_S_SALE;
	while(1)
	{
		ucResult=SAV_SavedTransIndex(0);
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"");
			//Os__GB2312_display(1,0,"()");
			ucResult=Untouch_WaitAllCard();
			if(ucResult==ERR_CANCEL)
			{
				//if(DataSave0.ChangeParamData.aucTermFlag[2])
				//	COMMS_FinComm();
				break;
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MAG)
			{
				ucResult=ERR_CARDNOFUN;
			}
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult == SUCCESS )
					SLE4442_ATR(aucBuf);
				if(ucResult==SUCCESS)
					ucResult = SLE4442_ReadCardData();
				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();
				if(ucResult==SUCCESS)
				{
					if(!RunData.aucZDFunFlag[3])
						ucResult=ERR_NOFUNCTION;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.ucPtcode!=NormalTransData.ucPtCode)
						ucResult=ERR_CARDTYPE;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.aucFunFlag[9])
						ucResult=ERR_CARDNOFUN;
				}
				if(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
				{
					if(ucResult==SUCCESS)
						ucResult=ERR_ONLINECARD;
				}
				else
				{
					if(ucResult==SUCCESS)
					{
						if(SleTransData.ucStatus!=0x04)
							ucResult=ERR_NOTUNACT;
					}
					if(ucResult==SUCCESS)
					{
						if(NormalTransData.ucZeroFlag==1)
							ucResult=ERR_NOTPROC;
					}
					if(ucResult==SUCCESS)
					{
						if(RunData.aucFunFlag[15])
						{
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ۿ");
							//Os__GB2312_display(1,0,"(000 - 100 %)");
							memset(aucBuf,0,sizeof(aucBuf));
							//uiDiscount=(1000-bcd_long(NormalTransData.aucDiscount,4))/10;
							uiDiscount=(1000-uiDiscount)/10;
							long_asc(aucBuf,3,(ulong *)&uiDiscount);
					            if (UTIL_Input(2,false,3,3,'N',aucBuf,NULL) != KEY_ENTER )
					            {
					                ucResult = ERR_CANCEL;
					               
					            }else
					            {
							uiDiscount=asc_long(aucBuf,3)*10;
							if(uiDiscount<=1000)
							{
								uiDiscount=1000-uiDiscount;
					            		long_bcd(NormalTransData.aucDiscount,2,(ulong *)&uiDiscount);
				            		}
					            	else
					            		ucResult=ERR_DISCOUNT;
					            }
						}
					}
					if(ucResult==SUCCESS)
					{
						CFG_SetBatchNumber();
						UTIL_IncreaseTraceNumber(0);
						if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
							ucOffset=63;
						else
							ucOffset=78;
						aucBuf[0]=0x01;
						ucResult=SLE4442_UpdateMainMemory(aucBuf[0],ucOffset);
						if(ucResult!=SUCCESS)
						{
							return ucResult;
						}
						ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,1);
						if(aucOutData[0]!=aucBuf[0])
							return ERR_WRITEMEMORY;
						if(RunData.aucFunFlag[15]&&NormalTransData.ucNewOrOld==0x01)
						{
							for(ucI=0;ucI<2;ucI++)
							{
								if(ucResult == SUCCESS)
								{
									ucResult=SLE4442_UpdateMainMemory(NormalTransData.aucDiscount[ucI],83+ucI);
								}
							}
						}
					}
				}
			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_ReadCardInfo();
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();

				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

				if(ucResult==SUCCESS)
					ucResult=Untouch_CheckRedo();
				if(ucResult==SUCCESS)
				{
					if(!RunData.aucZDFunFlag[3])
						ucResult=ERR_NOFUNCTION;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.ucPtcode!=NormalTransData.ucPtCode)
						ucResult=ERR_CARDTYPE;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.aucFunFlag[9])
						ucResult=ERR_CARDNOFUN;
				}
				if(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
				{
					if(ucResult==SUCCESS)
						ucResult=ERR_ONLINECARD;
				}
				else
				{
					if(ucResult==SUCCESS)
					{
						if(SleTransData.ucStatus!=0x04)
							ucResult=ERR_NOTUNACT;
					}
					if(ucResult==SUCCESS)
					{
						if(NormalTransData.ucZeroFlag==1)
							ucResult=ERR_NOTPROC;
					}
					/*if(ucResult==SUCCESS)
					{
						if(RunData.aucFunFlag[15])
						{
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ۿ");
							//Os__GB2312_display(1,0,"(000 - 100 %)");
							memset(aucBuf,0,sizeof(aucBuf));
							uiDiscount=(1000-bcd_long(NormalTransData.aucDiscount,4))/10;
							long_asc(aucBuf,3,&uiDiscount);
					            if (UTIL_Input(2,false,3,3,'N',aucBuf,NULL) != KEY_ENTER )
					            {
					                ucResult = ERR_CANCEL;
					                break;
					            }else
					            {
							uiDiscount=asc_long(aucBuf,3)*10;
							if(uiDiscount<=1000)
					            		long_bcd(NormalTransData.aucDiscount,2,&uiDiscount);
					            	else
					            		ucResult=ERR_DISCOUNT
					            }
						}
					}*/
					if(NormalTransData.ucCardType==CARD_M1)
					{

						if(ucResult==SUCCESS)
						{
							UTIL_IncreaseTraceNumber(0);
							RunData.ucTagetStatus=0x01;
							ucResult=MIFARE_ChangeStatus();
						}
					}
					else
					{
						if(ucResult==SUCCESS)
						{
							ucResult=PBOC_ISOGetChallenge(ICC1);
						}
						if(ucResult==SUCCESS)
						{
							memset(aucBuf,0,sizeof(aucBuf));
							memcpy(aucBuf,Iso7816Out.aucData,4);
							memset(aucFensan,0,sizeof(aucFensan));
							memcpy(aucFensan,NormalTransData.aucCardSerial,4);
							memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
							PBOC_Fensan_key(aucFensan,
								KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
							des(aucBuf,aucBuf,aucUseKey);
							desm1(aucBuf,aucBuf,&aucUseKey[8]);
							des(aucBuf,aucBuf,aucUseKey);
							ucResult=PBOC_ISOExternalauth(ICC1, aucBuf);
						}
						if(ucResult==SUCCESS)
						{
							UTIL_IncreaseTraceNumber(0);
							aucBuf[0]=0x01;
							ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
						}
					}
				}
			}
		}
		if(ucResult==SUCCESS)
		{
			memcpy(NormalTransData.aucCashierNo,DataSave0.ChangeParamData.aucCashierNo,6);
			NormalTransData.ulAmount=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
			ucResult=SAV_SaleSave();
		}
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ۿɹ");
			RunData.uiSaleCardNb++;
			RunData.ulSaleCardAmount+=NormalTransData.ulAmount;
			RunData.ulSaleCountAmount=RunData.ulSaleCountAmount+NormalTransData.ulAmount-NormalTransData.ulAmount*bcd_long(NormalTransData.aucDiscount,4)/1000;
			if(RunData.uiSaleCardNb==1)
			{
	
				PRT_ConstantParam();
				
				if(DataSave0.ConstantParamData.prnflag)
					Os__GB2312_xprint((unsigned char *)"---------------------",FONT); 
				else
					Os__xprint((unsigned char *)"---------------------");
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(aucPrintBuf,(uchar *)"Ա:",7);
				memcpy(&aucPrintBuf[7],NormalTransData.aucCashierNo,6);
				Os__GB2312_xprint(aucPrintBuf,FONT); 
				Os__GB2312_xprint((uchar *)"ۿͳƴӡ",FONT);
				Os__GB2312_xprint((unsigned char *)"κ:",FONT);	
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				aucPrintBuf[30]='\0';
				bcd_asc(aucPrintBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
				Os__GB2312_xprint(aucPrintBuf,FONT); 
				
				Os__xlinefeed(LINENUM*0.5);
				PRT_LocalDateTime();
				Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);

			}
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			bcd_asc(&aucPrintBuf[0],NormalTransData.aucUserCardNo,16);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			UTIL_Form_Montant(&aucPrintBuf[14], NormalTransData.ulAmount, 2);
			memcpy(aucPrintBuf,NormalTransData.aucLoadTrace,16);
			aucPrintBuf[0]=' ';
			if(NormalTransData.ucPtCode==0x02)
				aucPrintBuf[1]=' ';
			Os__GB2312_xprint(aucPrintBuf,FONT);
			Os__xprint((uchar*)"");

		}
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
		if(ucResult==SUCCESS&&DataSave0.ChangeParamData.aucTermFlag[2])
		{
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_PreComm();
			}
			STL_SendSale();
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_FinComm();
			}
		}
	}
	if(RunData.uiSaleCardNb)
		PRT_Sale();
	return SUCCESS;
}
unsigned char Untouch_SaleOnline(void)
{
	unsigned char aucBuf[200],aucBuf1[20],aucOutData[200],aucPrintBuf[40],aucFensan[10],aucUseKey[20],ucHandFlag;
	unsigned char ucResult=SUCCESS,ucStatus,ucLen,ucOffset,index,ucI,aucCard1[20],aucCard2[20],ucK;
	int uiSale=0,uiDiscount,i;
	unsigned long ulAmount;
	NORMALTRANS tnormal;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=CashierLogonProcess();
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	if(DataSave0.ChangeParamData.ucExpFlag1!=FLAG_LOGON_OK)
	{
		ucResult=ExpLogonProcess();
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}

	}
	NormalTransData.ucTransType=TRANS_S_SALEONLINE;
	while(1)
	{
		ucResult=SAV_SavedTransIndex(0);
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		NormalTransData.ucHandFlag=0;
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"");
			//Os__GB2312_display(1,0,"ˢʼ");
			//Os__GB2312_display(2,0,"[ȷ]俨");
			ucResult=Untouch_WaitAllCard1();
			Uart_Printf("ucResult=%02x\n",ucResult);
			if(ucResult==ERR_CANCEL)
			{
				//if(DataSave0.ChangeParamData.aucTermFlag[2])
				//	COMMS_FinComm();
				break;
			}
			if(ucResult==KEY_ENTER)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,"ʼ");
				memset(aucCard1,0,sizeof(aucCard1));
				if( UTIL_Input(1,true,16,16,'N',aucCard1,NULL) != KEY_ENTER )
				{
					ucResult=ERR_CANCEL;
					break;
				}
				asc_bcd(NormalTransData.aucUserCardNo,8,aucCard1,16);
				if(ucResult!=ERR_CANCEL)
				{
					ucResult=CFG_CheckCardTbl4Hand();
				}
				//Os__GB2312_display(2,0,"ֹ");
				memset(aucCard2,0,sizeof(aucCard2));
				memcpy(aucCard2,aucCard1,16);
				if( UTIL_Input(3,true,16,16,'N',aucCard2,NULL) != KEY_ENTER )
				{
					ucResult=ERR_CANCEL;
					break;
				}
				asc_bcd(NormalTransData.aucOldCardNo,8,aucCard2,16);
				NormalTransData.ucHandFlag=1;
				ucResult = SUCCESS;
				memcpy(&aucCard1[7],&aucCard1[8],8);
				memcpy(&aucCard2[7],&aucCard2[8],8);
				if(memcmp(aucCard1,aucCard2,15)>0)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,"ֹСʼ");
					MSG_WaitKey(3);
					ucResult=ERR_CANCEL;

				}
				/*if(ucResult==SUCCESS)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,"ۿ");
					//Os__GB2312_display(1,0,"(000 - 100 %)");
					memset(aucBuf,0,sizeof(aucBuf));
					//uiDiscount=(1000-bcd_long(NormalTransData.aucDiscount,4))/10;
					uiDiscount=(1000-uiDiscount)/10;
					long_asc(aucBuf,3,&uiDiscount);
					if (UTIL_Input(2,false,3,3,'N',aucBuf,NULL) != KEY_ENTER )
					{
						ucResult = ERR_CANCEL;

					}else
					{
						uiDiscount=asc_long(aucBuf,3)*10;
						if(uiDiscount<=1000)
						{
							uiDiscount=1000-uiDiscount;
				            		long_bcd(NormalTransData.aucDiscount,2,&uiDiscount);
			            		}
				            	else
				            		ucResult=ERR_DISCOUNT;
					}
				}*/
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucHandFlag==1)
			{
				NormalTransData.ucNewOrOld=1;
				CFG_SetBatchNumber();
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"ѡ֧ʽ");
				//Os__GB2312_display(1,0,(unsigned char *)"1.ֽ ");
				//Os__GB2312_display(2,0,(unsigned char *)"2.п");
				//Os__GB2312_display(3,0,(unsigned char *)"3.֧Ʊ ");
				ucK=0;
				while(ucK!='1'&&ucK!='2'&&ucK!='3'&&ucK!=ERR_APP_TIMEOUT&&ucK!=KEY_CLEAR)
					ucK=MSG_WaitKey(60);
				if(ucK=='1'||ucK=='2'||ucK=='3')
					NormalTransData.ucPackFlag=ucK;
				else
					ucResult=ERR_CANCEL;
				if(ucResult==SUCCESS&&RunData.aucExFunFlag[0])
				{
					UTIL_GetNewExp();
					while(1)
					{
						ucResult=SUCCESS;
						//Os__clr_display(255);
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(unsigned char *)"Ч:",9);
						//Os__GB2312_display(2,0,aucBuf);
						memset(aucBuf,0,sizeof(aucBuf));
						memset(aucBuf,' ',16);
						bcd_asc(&aucBuf[6],NormalTransData.aucNewExpiredDate,4);	
						aucBuf[10]='/';
						bcd_asc(&aucBuf[11],&NormalTransData.aucNewExpiredDate[2],2);	
						aucBuf[13]='/';
						bcd_asc(&aucBuf[14],&NormalTransData.aucNewExpiredDate[3],2);	
						//Os__GB2312_display(3,0,aucBuf);
						ucK=0;
						while(ucK!=KEY_ENTER&&ucK!=KEY_CLEAR&&ucK!=KEY_BCKSP)
							ucK=MSG_WaitKey(60);
						if(ucK==KEY_BCKSP)
						{
							memset(aucBuf,0,sizeof(aucBuf));
					            	if(UTIL_Input_XY(3,6,false,8,8,'N',aucBuf,0) != KEY_ENTER)
								ucResult= ERR_CANCEL;
							if(ucResult==SUCCESS)
							{
								memset(aucBuf1,0,sizeof(aucBuf1));
								asc_bcd(aucBuf1,4,aucBuf,8);
								ucResult=CFG_ChkExpireValid(aucBuf1);
								if(ucResult!=SUCCESS)
									MSG_DisplayErrMsg(ucResult);
							}
							if(ucResult==SUCCESS)
								asc_bcd(NormalTransData.aucNewExpiredDate,4,aucBuf,8);
						}
						else if(ucK==KEY_ENTER)
						{
							memcpy(NormalTransData.aucExpiredDate,NormalTransData.aucNewExpiredDate,4);
							ucResult=CFG_ChkExpireValid(NormalTransData.aucNewExpiredDate);
							if(ucResult!=SUCCESS)
								MSG_DisplayErrMsg(ucResult);
							else
								break;
						}
						else if(ucK==KEY_CLEAR)
						{
							ucResult=ERR_CANCEL;
							break;
						}
						
					}
				}				
				if(ucResult==SUCCESS)
					ucResult=Trans_Sale();
			}
			else
			{
				if(NormalTransData.ucCardType==CARD_MAG)
				{
					if(ucResult==SUCCESS)
					{
						asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
						asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[23],2);
						asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[25],6);
						NormalTransData.aucExpiredDate[0]=0x20;
						asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
						ucResult=CFG_CheckCardValid();
					}
					if(ucResult==SUCCESS)
					{
						memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,"ʼ");
						memset(aucCard1,0,sizeof(aucCard1));
						bcd_asc(aucCard1,NormalTransData.aucUserCardNo,16);
						Os__display(1,0,aucCard1);
						//Os__GB2312_display(2,0,(uchar *)"γ");
						Os__xdelay(10);
						Os__ICC_remove();
						//Os__GB2312_display(2,0,"ˢֹ");
						ucResult=Untouch_WaitAllCard1();
						if(ucResult==KEY_ENTER)
						{
							memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
							ucResult=SUCCESS;
						}
						else
						{
							if(ucResult == SUCCESS )
							{
								asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
								asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[23],2);
								asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[25],6);
								NormalTransData.aucExpiredDate[0]=0x20;
								asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
								ucResult=CFG_CheckCardValid();
							}
							if(ucResult == SUCCESS )
							{
								bcd_asc(aucCard1,DataSave1.CardTable.aucCardBin,12);
								bcd_asc(aucCard2,NormalTransData.aucUserCardNo,16);
								if(memcmp(aucCard1,aucCard2,bcd_long(&DataSave1.CardTable.ucCardBinLen,2))
									||tnormal.ucPtCode!=NormalTransData.ucPtCode
									||memcmp(tnormal.aucZoneCode,NormalTransData.aucZoneCode,4))
									ucResult=ERR_CARDTYPE;
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								memcpy(NormalTransData.aucUserCardNo,tnormal.aucUserCardNo,8);
							}
						}
					}
					if(ucResult==SUCCESS)
					{
						//Os__GB2312_display(2,0,"ֹ     ");
						memset(aucCard1,0,sizeof(aucCard1));
						bcd_asc(aucCard1,NormalTransData.aucOldCardNo,16);
						Os__display(3,0,aucCard1);
						if(MSG_WaitKey(60)!=KEY_ENTER)
							ucResult=ERR_CANCEL;
					}
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();
					if(ucResult==SUCCESS)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(unsigned char *)"ѡ֧ʽ");
						//Os__GB2312_display(1,0,(unsigned char *)"1.ֽ ");
						//Os__GB2312_display(2,0,(unsigned char *)"2.п");
						//Os__GB2312_display(3,0,(unsigned char *)"3.֧Ʊ ");
						ucK=0;
						while(ucK!='1'&&ucK!='2'&&ucK!='3'&&ucK!=ERR_APP_TIMEOUT&&ucK!=KEY_CLEAR)
							ucK=MSG_WaitKey(60);
						if(ucK=='1'||ucK=='2'||ucK=='3')
							NormalTransData.ucPackFlag=ucK;
						else
							ucResult=ERR_CANCEL;
						if(ucResult==SUCCESS&&RunData.aucExFunFlag[0])
						{
							UTIL_GetNewExp();
							while(1)
							{
								ucResult=SUCCESS;
								//Os__clr_display(255);
								memset(aucBuf,0,sizeof(aucBuf));
								memcpy(aucBuf,(unsigned char *)"Ч:",9);
								//Os__GB2312_display(2,0,aucBuf);
								memset(aucBuf,0,sizeof(aucBuf));
								memset(aucBuf,' ',16);
								bcd_asc(&aucBuf[6],NormalTransData.aucNewExpiredDate,4);	
								aucBuf[10]='/';
								bcd_asc(&aucBuf[11],&NormalTransData.aucNewExpiredDate[2],2);	
								aucBuf[13]='/';
								bcd_asc(&aucBuf[14],&NormalTransData.aucNewExpiredDate[3],2);	
								//Os__GB2312_display(3,0,aucBuf);
								ucK=0;
								while(ucK!=KEY_ENTER&&ucK!=KEY_CLEAR&&ucK!=KEY_BCKSP)
									ucK=MSG_WaitKey(60);
								if(ucK==KEY_BCKSP)
								{
									memset(aucBuf,0,sizeof(aucBuf));
							            	if(UTIL_Input_XY(3,6,false,8,8,'N',aucBuf,0) != KEY_ENTER)
										ucResult= ERR_CANCEL;
									if(ucResult==SUCCESS)
									{
										memset(aucBuf1,0,sizeof(aucBuf1));
										asc_bcd(aucBuf1,4,aucBuf,8);
										ucResult=CFG_ChkExpireValid(aucBuf1);
										if(ucResult!=SUCCESS)
											MSG_DisplayErrMsg(ucResult);
									}
									if(ucResult==SUCCESS)
										asc_bcd(NormalTransData.aucNewExpiredDate,4,aucBuf,8);
								}
								else if(ucK==KEY_ENTER)
								{
									memcpy(NormalTransData.aucExpiredDate,NormalTransData.aucNewExpiredDate,4);
									ucResult=CFG_ChkExpireValid(NormalTransData.aucNewExpiredDate);
									if(ucResult!=SUCCESS)
										MSG_DisplayErrMsg(ucResult);
									else
										break;
								}
								else if(ucK==KEY_CLEAR)
								{
									ucResult=ERR_CANCEL;
									break;
								}
								
							}
						}				
						if(ucResult==SUCCESS)
							ucResult=Trans_Sale();
					}
				}
				if(NormalTransData.ucCardType==CARD_MEM)
				{
					if(ucResult == SUCCESS )
						SLE4442_ATR(aucBuf);
					if(ucResult==SUCCESS)
						ucResult = SLE4442_ReadCardData();
					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();
					if(ucResult==SUCCESS)
					{
						if(!RunData.aucZDFunFlag[10])
							ucResult=ERR_NOFUNCTION;
					}
					if(ucResult==SUCCESS)
					{
						if(RunData.ucPtcode!=NormalTransData.ucPtCode)
							ucResult=ERR_CARDTYPE;
					}
					/*if(ucResult==SUCCESS)
					{
						if(RunData.aucFunFlag[9])
							ucResult=ERR_CARDNOFUN;
					}*/
					if(ucResult==SUCCESS)
					{
						/*if(ucResult==SUCCESS)
						{
							if(SleTransData.ucStatus!=0x04)
								ucResult=ERR_NOTUNACT;
						}
						if(ucResult==SUCCESS)
						{
							if(NormalTransData.ucZeroFlag==1)
								ucResult=ERR_NOTPROC;
						}*/
						if(ucResult==SUCCESS)
						{
							memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ʼ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucUserCardNo,16);
							Os__display(1,0,aucCard1);
							//Os__GB2312_display(2,0,(uchar *)"γ");
							Os__xdelay(10);
							Os__ICC_remove();
							//Os__GB2312_display(2,0,"ˢֹ");
							ucResult=Untouch_WaitAllCard1();
							if(ucResult==KEY_ENTER)
							{
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								ucResult=SUCCESS;
							}
							else
							{
								if(ucResult == SUCCESS )
									SLE4442_ATR(aucBuf);
								if(ucResult==SUCCESS)
									ucResult = SLE4442_ReadCardData();
								bcd_asc(aucCard1,DataSave1.CardTable.aucCardBin,12);
								bcd_asc(aucCard2,NormalTransData.aucUserCardNo,16);
								if(memcmp(aucCard1,aucCard2,bcd_long(&DataSave1.CardTable.ucCardBinLen,2))
									||tnormal.ucPtCode!=NormalTransData.ucPtCode
									||memcmp(tnormal.aucZoneCode,NormalTransData.aucZoneCode,4))
									ucResult=ERR_CARDTYPE;
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								memcpy(NormalTransData.aucUserCardNo,tnormal.aucUserCardNo,8);
							}
						}
						if(ucResult==SUCCESS)
						{
							//Os__GB2312_display(2,0,"ֹ     ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucOldCardNo,16);
							Os__display(3,0,aucCard1);
							if(MSG_WaitKey(60)!=KEY_ENTER)
								ucResult=ERR_CANCEL;
						}
						if(ucResult==SUCCESS)
						{
							if(RunData.aucFunFlag[15])
							{
								//Os__clr_display(255);
								//Os__GB2312_display(0,0,"ۿ");
								//Os__GB2312_display(1,0,"(000 - 100 %)");
								memset(aucBuf,0,sizeof(aucBuf));
								//uiDiscount=(1000-bcd_long(NormalTransData.aucDiscount,4))/10;
								uiDiscount=(1000-uiDiscount)/10;
								long_asc(aucBuf,3,(ulong *)&uiDiscount);
						            if (UTIL_Input(2,false,3,3,'N',aucBuf,NULL) != KEY_ENTER )
						            {
						                ucResult = ERR_CANCEL;
						               
						            }else
						            {
								uiDiscount=asc_long(aucBuf,3)*10;
								if(uiDiscount<=1000)
								{
									uiDiscount=1000-uiDiscount;
						            		long_bcd(NormalTransData.aucDiscount,2,(ulong *)&uiDiscount);
					            		}
						            	else
						            		ucResult=ERR_DISCOUNT;
						            }
							}
						}
						if(ucResult==SUCCESS)
						{
							NormalTransData.ulAmount=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,(unsigned char *)"ѡ֧ʽ");
							//Os__GB2312_display(1,0,(unsigned char *)"1.ֽ ");
							//Os__GB2312_display(2,0,(unsigned char *)"2.п");
							//Os__GB2312_display(3,0,(unsigned char *)"3.֧Ʊ ");
							ucK=0;
							while(ucK!='1'&&ucK!='2'&&ucK!='3'&&ucK!=ERR_APP_TIMEOUT&&ucK!=KEY_CLEAR)
								ucK=MSG_WaitKey(60);
							if(ucK=='1'||ucK=='2'||ucK=='3')
								NormalTransData.ucPackFlag=ucK;
							else
								ucResult=ERR_CANCEL;
							if(ucResult==SUCCESS&&RunData.aucExFunFlag[0])
							{
								UTIL_GetNewExp();
								while(1)
								{
									ucResult=SUCCESS;
									//Os__clr_display(255);
									memset(aucBuf,0,sizeof(aucBuf));
									memcpy(aucBuf,(unsigned char *)"Ч:",9);
									//Os__GB2312_display(2,0,aucBuf);
									memset(aucBuf,0,sizeof(aucBuf));
									memset(aucBuf,' ',16);
									bcd_asc(&aucBuf[6],NormalTransData.aucNewExpiredDate,4);	
									aucBuf[10]='/';
									bcd_asc(&aucBuf[11],&NormalTransData.aucNewExpiredDate[2],2);	
									aucBuf[13]='/';
									bcd_asc(&aucBuf[14],&NormalTransData.aucNewExpiredDate[3],2);	
									//Os__GB2312_display(3,0,aucBuf);
									ucK=0;
									while(ucK!=KEY_ENTER&&ucK!=KEY_CLEAR&&ucK!=KEY_BCKSP)
										ucK=MSG_WaitKey(60);
									if(ucK==KEY_BCKSP)
									{
										memset(aucBuf,0,sizeof(aucBuf));
								            	if(UTIL_Input_XY(3,6,false,8,8,'N',aucBuf,0) != KEY_ENTER)
											ucResult= ERR_CANCEL;
										if(ucResult==SUCCESS)
										{
											memset(aucBuf1,0,sizeof(aucBuf1));
											asc_bcd(aucBuf1,4,aucBuf,8);
											ucResult=CFG_ChkExpireValid(aucBuf1);
											if(ucResult!=SUCCESS)
												MSG_DisplayErrMsg(ucResult);
										}
										if(ucResult==SUCCESS)
											asc_bcd(NormalTransData.aucNewExpiredDate,4,aucBuf,8);
									}
									else if(ucK==KEY_ENTER)
									{
										memcpy(NormalTransData.aucExpiredDate,NormalTransData.aucNewExpiredDate,4);
										ucResult=CFG_ChkExpireValid(NormalTransData.aucNewExpiredDate);
										if(ucResult!=SUCCESS)
											MSG_DisplayErrMsg(ucResult);
										else
											break;
									}
									else if(ucK==KEY_CLEAR)
									{
										ucResult=ERR_CANCEL;
										break;
									}
									
								}
							}				
							if(ucResult==SUCCESS)
								ucResult=Trans_Sale();
						}
						/*if(ucResult==SUCCESS)
						{
							if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
								ucOffset=63;
							else
								ucOffset=78;
							aucBuf[0]=0x01;
							ucResult=SLE4442_UpdateMainMemory(aucBuf[0],ucOffset);
							if(ucResult!=SUCCESS)
							{
								return ucResult;
							}
							ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,1);
							if(aucOutData[0]!=aucBuf[0])
								return ERR_WRITEMEMORY;
							if(RunData.aucFunFlag[15]&&NormalTransData.ucNewOrOld==0x01)
							{
								for(ucI=0;ucI<2;ucI++)
								{
									if(ucResult == SUCCESS)
									{
										ucResult=SLE4442_UpdateMainMemory(NormalTransData.aucDiscount[ucI],83+ucI);
									}
								}
							}
						}*/
					}
				}
				if(NormalTransData.ucCardType==CARD_M1)
				{
					if(ucResult==SUCCESS)
						ucResult=Untouch_ReadCardInfo();
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();

					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();

					if(ucResult==SUCCESS)
						ucResult=Untouch_CheckRedo();
					if(ucResult==SUCCESS)
					{
						if(!RunData.aucZDFunFlag[10])
							ucResult=ERR_NOFUNCTION;
					}
					if(ucResult==SUCCESS)
					{
						if(RunData.ucPtcode!=NormalTransData.ucPtCode)
							ucResult=ERR_CARDTYPE;
					}
					/*if(ucResult==SUCCESS)
					{
						if(RunData.aucFunFlag[9])
							ucResult=ERR_CARDNOFUN;
					}*/
					if(ucResult==SUCCESS)
					{
						/*if(ucResult==SUCCESS)
						{
							if(SleTransData.ucStatus!=0x04)
								ucResult=ERR_NOTUNACT;
						}
						if(ucResult==SUCCESS)
						{
							if(NormalTransData.ucZeroFlag==1)
								ucResult=ERR_NOTPROC;
						}*/
						if(ucResult==SUCCESS)
						{
							memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ʼ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucUserCardNo,16);
							Os__display(1,0,aucCard1);
							while(1)
							{
								ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
								if(ucResult==0x04)
								{
									ucResult= SUCCESS;
									break;
								}
								//Os__clr_display(2);
								//Os__GB2312_display(2,0,"ȡ");
							}
							//Os__GB2312_display(2,0,"ˢֹ");
							ucResult=Untouch_WaitAllCard1();
							if(ucResult==KEY_ENTER)
							{
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								ucResult=SUCCESS;
							}
							else
							{
								if(ucResult == SUCCESS )
									SLE4442_ATR(aucBuf);
								if(ucResult==SUCCESS)
									ucResult = Untouch_ReadCardInfo();
								bcd_asc(aucCard1,DataSave1.CardTable.aucCardBin,12);
								bcd_asc(aucCard2,NormalTransData.aucUserCardNo,16);
								if(memcmp(aucCard1,aucCard2,bcd_long(&DataSave1.CardTable.ucCardBinLen,2))
									||tnormal.ucPtCode!=NormalTransData.ucPtCode
									||memcmp(tnormal.aucZoneCode,NormalTransData.aucZoneCode,4))
									ucResult=ERR_CARDTYPE;
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								memcpy(NormalTransData.aucUserCardNo,tnormal.aucUserCardNo,8);
							}
						}
						if(ucResult==SUCCESS)
						{
							//Os__GB2312_display(2,0,"ֹ     ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucOldCardNo,16);
							Os__display(3,0,aucCard1);
							if(MSG_WaitKey(60)!=KEY_ENTER)
								ucResult=ERR_CANCEL;
						}
						if(ucResult==SUCCESS)
						{
							if(RunData.aucFunFlag[15])
							{
								//Os__clr_display(255);
								//Os__GB2312_display(0,0,"ۿ");
								//Os__GB2312_display(1,0,"(000 - 100 %)");
								memset(aucBuf,0,sizeof(aucBuf));
								uiDiscount=(1000-bcd_long(NormalTransData.aucDiscount,4))/10;
								long_asc(aucBuf,3,(ulong *)&uiDiscount);
						            if (UTIL_Input(2,false,3,3,'N',aucBuf,NULL) != KEY_ENTER )
						            {
						                ucResult = ERR_CANCEL;
						                break;
						            }else
						            {
								uiDiscount=asc_long(aucBuf,3)*10;
								if(uiDiscount<=1000)
						            		long_bcd(NormalTransData.aucDiscount,2,(ulong *)&uiDiscount);
						            	else
						            		ucResult=ERR_DISCOUNT;
						            }
							}
						}/**/
						if(ucResult==SUCCESS)
						{
							NormalTransData.ulAmount=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,(unsigned char *)"ѡ֧ʽ");
							//Os__GB2312_display(1,0,(unsigned char *)"1.ֽ ");
							//Os__GB2312_display(2,0,(unsigned char *)"2.п");
							//Os__GB2312_display(3,0,(unsigned char *)"3.֧Ʊ ");
							ucK=0;
							while(ucK!='1'&&ucK!='2'&&ucK!='3'&&ucK!=ERR_APP_TIMEOUT&&ucK!=KEY_CLEAR)
								ucK=MSG_WaitKey(60);
							if(ucK=='1'||ucK=='2'||ucK=='3')
								NormalTransData.ucPackFlag=ucK;
							else
								ucResult=ERR_CANCEL;
							if(ucResult==SUCCESS&&RunData.aucExFunFlag[0])
							{
								UTIL_GetNewExp();
								while(1)
								{
									ucResult=SUCCESS;
									//Os__clr_display(255);
									memset(aucBuf,0,sizeof(aucBuf));
									memcpy(aucBuf,(unsigned char *)"Ч:",9);
									//Os__GB2312_display(2,0,aucBuf);
									memset(aucBuf,0,sizeof(aucBuf));
									memset(aucBuf,' ',16);
									bcd_asc(&aucBuf[6],NormalTransData.aucNewExpiredDate,4);	
									aucBuf[10]='/';
									bcd_asc(&aucBuf[11],&NormalTransData.aucNewExpiredDate[2],2);	
									aucBuf[13]='/';
									bcd_asc(&aucBuf[14],&NormalTransData.aucNewExpiredDate[3],2);	
									//Os__GB2312_display(3,0,aucBuf);
									ucK=0;
									while(ucK!=KEY_ENTER&&ucK!=KEY_CLEAR&&ucK!=KEY_BCKSP)
										ucK=MSG_WaitKey(60);
									if(ucK==KEY_BCKSP)
									{
										memset(aucBuf,0,sizeof(aucBuf));
								            	if(UTIL_Input_XY(3,6,false,8,8,'N',aucBuf,0) != KEY_ENTER)
											ucResult= ERR_CANCEL;
										if(ucResult==SUCCESS)
										{
											memset(aucBuf1,0,sizeof(aucBuf1));
											asc_bcd(aucBuf1,4,aucBuf,8);
											ucResult=CFG_ChkExpireValid(aucBuf1);
											if(ucResult!=SUCCESS)
												MSG_DisplayErrMsg(ucResult);
										}
										if(ucResult==SUCCESS)
											asc_bcd(NormalTransData.aucNewExpiredDate,4,aucBuf,8);
									}
									else if(ucK==KEY_ENTER)
									{
										memcpy(NormalTransData.aucExpiredDate,NormalTransData.aucNewExpiredDate,4);
										ucResult=CFG_ChkExpireValid(NormalTransData.aucNewExpiredDate);
										if(ucResult!=SUCCESS)
											MSG_DisplayErrMsg(ucResult);
										else
											break;
									}
									else if(ucK==KEY_CLEAR)
									{
										ucResult=ERR_CANCEL;
										break;
									}
									
								}
							}				
							if(ucResult==SUCCESS)
								ucResult=Trans_Sale();
						}
						/*if(NormalTransData.ucCardType==CARD_M1)
						{

							if(ucResult==SUCCESS)
							{
								//UTIL_IncreaseTraceNumber(0);
								RunData.ucTagetStatus=0x01;
								ucResult=Untouch_ChangeStatus_tmp(&ucStatus,aucOutData,&ucLen);
							}
						}
						else
						{
							if(ucResult==SUCCESS)
							{
								ucResult=PBOC_ISOGetChallenge(ICC1);
							}
							if(ucResult==SUCCESS)
							{
								memset(aucBuf,0,sizeof(aucBuf));
								memcpy(aucBuf,Iso7816Out.aucData,4);
								memset(aucFensan,0,sizeof(aucFensan));
								memcpy(aucFensan,NormalTransData.aucCardSerial,4);
								memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
								PBOC_Fensan_key(aucFensan,
									KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
								des(aucBuf,aucBuf,aucUseKey);
								desm1(aucBuf,aucBuf,&aucUseKey[8]);
								des(aucBuf,aucBuf,aucUseKey);
								ucResult=PBOC_ISOExternalauth(ICC1, aucBuf);
							}
							if(ucResult==SUCCESS)
							{
								UTIL_IncreaseTraceNumber(0);
								aucBuf[0]=0x01;
								ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
							}
						}*/
					}
				}
			}
		}
		if(ucResult==ERR_HOSTCODE&&RunData.ucErrorExtCode==3154)
		{
			for(i=0;i<TRANS_MAXNB;i++)
			{
				if(DataSave0.TransInfoData.auiTransIndex[i]!=0)
				{
					if(XDATA_Read_SaveTrans_File_2Datasave(i)==SUCCESS)
					{
						if(DataSave0.SaveTransData.ucTransType==TRANS_S_SALEONLINE
							&&!memcmp(DataSave0.SaveTransData.aucUserCardNo,NormalTransData.aucUserCardNo,8)
							&&!memcmp(DataSave0.SaveTransData.aucOldCardNo,NormalTransData.aucOldCardNo,8))
							break;
					}
				}
			}
			if(i==TRANS_MAXNB)
				ucResult=SUCCESS;
		}
		if(ucResult==SUCCESS)
		{
			memcpy(NormalTransData.aucCashierNo,DataSave0.ChangeParamData.aucCashierNo,6);
			ucResult=SAV_SaleOnlineSave();
		}
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ۿɹ");
			RunData.uiSaleCardNb++;
			RunData.ulSaleCardAmount+=NormalTransData.ulAmount;
			RunData.ulSaleCountAmount=RunData.ulSaleCountAmount+NormalTransData.ulAmount-NormalTransData.ulAmount*bcd_long(NormalTransData.aucDiscount,4)/1000;
			if(RunData.uiSaleCardNb==1)
			{
	
				PRT_ConstantParam();
				
				if(DataSave0.ConstantParamData.prnflag)
					Os__GB2312_xprint((unsigned char *)"---------------------",FONT); 
				else
					Os__xprint((unsigned char *)"---------------------");
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(aucPrintBuf,(uchar *)"Ա:",7);
				memcpy(&aucPrintBuf[7],NormalTransData.aucCashierNo,6);
				Os__GB2312_xprint(aucPrintBuf,FONT); 
				Os__GB2312_xprint((uchar *)"ۿ",FONT);
				Os__GB2312_xprint((unsigned char *)"κ:",FONT);	
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				aucPrintBuf[30]='\0';
				bcd_asc(aucPrintBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
				Os__GB2312_xprint(aucPrintBuf,FONT); 
				
				Os__xlinefeed(LINENUM*0.5);
				PRT_LocalDateTime();
				Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);

			}
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)"ʼ:",7);	
			bcd_asc(&aucPrintBuf[7],NormalTransData.aucUserCardNo,16);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)"ֹ:",7);	
			bcd_asc(&aucPrintBuf[7],NormalTransData.aucOldCardNo,16);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)":",7);	
			uiSale=1+bcd_long(&NormalTransData.aucOldCardNo[4],8)-bcd_long(&NormalTransData.aucUserCardNo[4],8);
			UTIL_Form_Montant(&aucPrintBuf[7],uiSale,0);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)":",7);	
			UTIL_Form_Montant(&aucPrintBuf[7],NormalTransData.ulAmount,2);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			if(bcd_long(NormalTransData.aucDiscount,4)>=10)
			{
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				aucPrintBuf[30]='\0';
				memcpy(aucPrintBuf,(unsigned char *)"ۿ:",7);
				ulAmount=(1000-bcd_long(NormalTransData.aucDiscount,4))/10;
				long_asc(&aucPrintBuf[11],2,&ulAmount);
				aucPrintBuf[13]='%';
				Os__GB2312_xprint(aucPrintBuf,FONT);
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				aucPrintBuf[30]='\0';
				memcpy(aucPrintBuf,(unsigned char *)"˶:",7);
				ulAmount=NormalTransData.ulAmount-NormalTransData.ulAmount*bcd_long(NormalTransData.aucDiscount,4)/1000;
				UTIL_Form_Montant(&aucPrintBuf[7],ulAmount,2);
				Os__GB2312_xprint(aucPrintBuf,FONT);
			}
			if(RunData.aucExFunFlag[0])
				PRT_ExpDateTime();			
			Os__xprint((uchar*)"");
			RunData.uiSaleCardDetail+=uiSale;
		}
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
	}
	if(RunData.uiSaleCardNb)
		PRT_SaleOnline();
	return SUCCESS;
}
unsigned char Untouch_BatchLoad(void)
{
	unsigned char aucBuf[200],aucOutData[200],aucPrintBuf[40],aucFensan[10],aucUseKey[20],ucHandFlag;
	unsigned char ucResult=SUCCESS,ucStatus,ucLen,ucOffset,index,ucI,aucCard1[20],aucCard2[20];
	int uiSale=0,uiDiscount,i;
	NORMALTRANS tnormal;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=CashierLogonProcess();
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	NormalTransData.ucTransType=TRANS_S_BATCHLOAD;
	while(1)
	{
		ucResult=SAV_SavedTransIndex(0);
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		NormalTransData.ucHandFlag=0;
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"ֵ");
			//Os__GB2312_display(1,0,"ˢʼ");
			//Os__GB2312_display(2,0,"[ȷ]俨");
			ucResult=Untouch_WaitAllCard1();
			Uart_Printf("ucResult=%02x\n",ucResult);
			if(ucResult==ERR_CANCEL)
			{
				//if(DataSave0.ChangeParamData.aucTermFlag[2])
				//	COMMS_FinComm();
				break;
			}
			if(ucResult==KEY_ENTER)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,"ʼ");
				memset(aucCard1,0,sizeof(aucCard1));
				if( UTIL_Input(1,true,16,16,'N',aucCard1,NULL) != KEY_ENTER )
				{
					ucResult=ERR_CANCEL;
					break;
				}
				asc_bcd(NormalTransData.aucUserCardNo,8,aucCard1,16);
				if(ucResult!=ERR_CANCEL)
				{
					ucResult=CFG_CheckCardTbl4Hand();
				}
				//Os__GB2312_display(2,0,"ֹ");
				memset(aucCard2,0,sizeof(aucCard2));
				memcpy(aucCard2,aucCard1,16);
				if( UTIL_Input(3,true,16,16,'N',aucCard2,NULL) != KEY_ENTER )
				{
					ucResult=ERR_CANCEL;
					break;
				}
				asc_bcd(NormalTransData.aucOldCardNo,8,aucCard2,16);
				NormalTransData.ucHandFlag=1;
				ucResult = SUCCESS;
				memcpy(&aucCard1[7],&aucCard1[8],8);
				memcpy(&aucCard2[7],&aucCard2[8],8);
				if(memcmp(aucCard1,aucCard2,15)>0)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,"ֹСʼ");
					MSG_WaitKey(3);
					ucResult=ERR_CANCEL;

				}
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucHandFlag==1)
			{
				NormalTransData.ucNewOrOld=1;
				CFG_SetBatchNumber();
				if(ucResult==SUCCESS)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,"ֵ");
					//Os__GB2312_display(2,0,(uchar *)"ֵ:");
					ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, 999999999);
				}
				if(ucResult==SUCCESS)
					ucResult=Trans_BatchLoad();
			}
			else
			{
				if(NormalTransData.ucCardType==CARD_MAG)
				{
					if(ucResult==SUCCESS)
					{
						asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
						asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[23],2);
						asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[25],6);
						NormalTransData.aucExpiredDate[0]=0x20;
						asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
						ucResult=CFG_CheckCardValid();
					}
					if(ucResult==SUCCESS)
					{
						memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,"ʼ");
						memset(aucCard1,0,sizeof(aucCard1));
						bcd_asc(aucCard1,NormalTransData.aucUserCardNo,16);
						Os__display(1,0,aucCard1);
						//Os__GB2312_display(2,0,(uchar *)"γ");
						Os__xdelay(10);
						Os__ICC_remove();
						//Os__GB2312_display(2,0,"ˢֹ");
						ucResult=Untouch_WaitAllCard1();
						if(ucResult==KEY_ENTER)
						{
							memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
							ucResult=SUCCESS;
						}
						else
						{
							if(ucResult == SUCCESS )
							{
								asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
								asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[23],2);
								asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[25],6);
								NormalTransData.aucExpiredDate[0]=0x20;
								asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
								ucResult=CFG_CheckCardValid();
							}
							if(ucResult == SUCCESS )
							{
								bcd_asc(aucCard1,DataSave1.CardTable.aucCardBin,12);
								bcd_asc(aucCard2,NormalTransData.aucUserCardNo,16);
								if(memcmp(aucCard1,aucCard2,bcd_long(&DataSave1.CardTable.ucCardBinLen,2))
									||tnormal.ucPtCode!=NormalTransData.ucPtCode
									||memcmp(tnormal.aucZoneCode,NormalTransData.aucZoneCode,4))
									ucResult=ERR_CARDTYPE;
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								memcpy(NormalTransData.aucUserCardNo,tnormal.aucUserCardNo,8);
							}
						}
					}
					if(ucResult==SUCCESS)
					{
						//Os__GB2312_display(2,0,"ֹ     ");
						memset(aucCard1,0,sizeof(aucCard1));
						bcd_asc(aucCard1,NormalTransData.aucOldCardNo,16);
						Os__display(3,0,aucCard1);
						if(MSG_WaitKey(60)!=KEY_ENTER)
							ucResult=ERR_CANCEL;
					}
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();
					if(ucResult==SUCCESS)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,"ֵ");
						//Os__GB2312_display(2,0,(uchar *)"ֵ:");
						ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, 999999999);
					}
					if(ucResult==SUCCESS)
						ucResult=Trans_BatchLoad();
				}				
				if(NormalTransData.ucCardType==CARD_MEM)
				{
					if(ucResult == SUCCESS )
						SLE4442_ATR(aucBuf);
					if(ucResult==SUCCESS)
						ucResult = SLE4442_ReadCardData();
					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();
					if(ucResult==SUCCESS)
					{
						if(!RunData.aucZDFunFlag[3])
							ucResult=ERR_NOFUNCTION;
					}
					if(ucResult==SUCCESS)
					{
						if(RunData.ucPtcode!=NormalTransData.ucPtCode)
							ucResult=ERR_CARDTYPE;
					}
					/*if(ucResult==SUCCESS)
					{
						if(RunData.aucFunFlag[9])
							ucResult=ERR_CARDNOFUN;
					}*/
					if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
					{
						if(ucResult==SUCCESS)
							ucResult=ERR_OFFLINECARD;
					}
					else
					{
						/*if(ucResult==SUCCESS)
						{
							if(SleTransData.ucStatus!=0x04)
								ucResult=ERR_NOTUNACT;
						}
						if(ucResult==SUCCESS)
						{
							if(NormalTransData.ucZeroFlag==1)
								ucResult=ERR_NOTPROC;
						}*/
						if(ucResult==SUCCESS)
						{
							memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ʼ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucUserCardNo,16);
							Os__display(1,0,aucCard1);
							//Os__GB2312_display(2,0,(uchar *)"γ");
							Os__xdelay(10);
							Os__ICC_remove();
							//Os__GB2312_display(2,0,"ˢֹ");
							ucResult=Untouch_WaitAllCard1();
							if(ucResult==KEY_ENTER)
							{
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								ucResult=SUCCESS;
							}
							else
							{
								if(ucResult == SUCCESS )
									SLE4442_ATR(aucBuf);
								if(ucResult==SUCCESS)
									ucResult = SLE4442_ReadCardData();
								bcd_asc(aucCard1,DataSave1.CardTable.aucCardBin,12);
								bcd_asc(aucCard2,NormalTransData.aucUserCardNo,16);
								if(memcmp(aucCard1,aucCard2,bcd_long(&DataSave1.CardTable.ucCardBinLen,2))
									||tnormal.ucPtCode!=NormalTransData.ucPtCode
									||memcmp(tnormal.aucZoneCode,NormalTransData.aucZoneCode,4))
									ucResult=ERR_CARDTYPE;
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								memcpy(NormalTransData.aucUserCardNo,tnormal.aucUserCardNo,8);
							}
						}
						if(ucResult==SUCCESS)
						{
							//Os__GB2312_display(2,0,"ֹ     ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucOldCardNo,16);
							Os__display(3,0,aucCard1);
							if(MSG_WaitKey(60)!=KEY_ENTER)
								ucResult=ERR_CANCEL;
						}
						if(ucResult==SUCCESS)
						{
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ֵ");
							//Os__GB2312_display(2,0,(uchar *)"ֵ:");
							ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, 999999999);
						}
						if(ucResult==SUCCESS)
							ucResult=Trans_BatchLoad();
					}
				}
				if(NormalTransData.ucCardType==CARD_M1)
				{
					if(ucResult==SUCCESS)
						ucResult=Untouch_ReadCardInfo();
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();

					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();

					if(ucResult==SUCCESS)
						ucResult=Untouch_CheckRedo();
					if(ucResult==SUCCESS)
					{
						if(!RunData.aucZDFunFlag[3])
							ucResult=ERR_NOFUNCTION;
					}
					if(ucResult==SUCCESS)
					{
						if(RunData.ucPtcode!=NormalTransData.ucPtCode)
							ucResult=ERR_CARDTYPE;
					}
					/*if(ucResult==SUCCESS)
					{
						if(RunData.aucFunFlag[9])
							ucResult=ERR_CARDNOFUN;
					}*/
					if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
					{
						if(ucResult==SUCCESS)
							ucResult=ERR_OFFLINECARD;
					}
					else
					{
						if(ucResult==SUCCESS)
						{
							memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ʼ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucUserCardNo,16);
							Os__display(1,0,aucCard1);
							while(1)
							{
								ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
								if(ucResult==0x04)
								{
									ucResult= SUCCESS;
									break;
								}
								//Os__clr_display(2);
								//Os__GB2312_display(2,0,"ȡ");
							}
							//Os__GB2312_display(2,0,"ˢֹ");
							ucResult=Untouch_WaitAllCard1();
							if(ucResult==KEY_ENTER)
							{
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								ucResult=SUCCESS;
							}
							else
							{
								if(ucResult == SUCCESS )
									SLE4442_ATR(aucBuf);
								if(ucResult==SUCCESS)
									ucResult = SLE4442_ReadCardData();
								bcd_asc(aucCard1,DataSave1.CardTable.aucCardBin,12);
								bcd_asc(aucCard2,NormalTransData.aucUserCardNo,16);
								if(memcmp(aucCard1,aucCard2,bcd_long(&DataSave1.CardTable.ucCardBinLen,2))
									||tnormal.ucPtCode!=NormalTransData.ucPtCode
									||memcmp(tnormal.aucZoneCode,NormalTransData.aucZoneCode,4))
									ucResult=ERR_CARDTYPE;
								memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8);
								memcpy(NormalTransData.aucUserCardNo,tnormal.aucUserCardNo,8);
							}
						}
						if(ucResult==SUCCESS)
						{
							//Os__GB2312_display(2,0,"ֹ     ");
							memset(aucCard1,0,sizeof(aucCard1));
							bcd_asc(aucCard1,NormalTransData.aucOldCardNo,16);
							Os__display(3,0,aucCard1);
							if(MSG_WaitKey(60)!=KEY_ENTER)
								ucResult=ERR_CANCEL;
						}
						if(ucResult==SUCCESS)
						{
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,"ֵ");
							//Os__GB2312_display(2,0,(uchar *)"ֵ:");
							ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, 999999999);
						}
						if(ucResult==SUCCESS)
							ucResult=Trans_BatchLoad();
					}
				}
			}
		}
		if(ucResult==ERR_HOSTCODE&&RunData.ucErrorExtCode==3154)
		{
			for(i=0;i<TRANS_MAXNB;i++)
			{
				if(DataSave0.TransInfoData.auiTransIndex[i]!=0)
				{
					if(XDATA_Read_SaveTrans_File_2Datasave(i)==SUCCESS)
					{
						if(DataSave0.SaveTransData.ucTransType==TRANS_S_BATCHLOAD
							&&!memcmp(DataSave0.SaveTransData.aucUserCardNo,NormalTransData.aucUserCardNo,8)
							&&!memcmp(DataSave0.SaveTransData.aucOldCardNo,NormalTransData.aucOldCardNo,8))
						{
							break;
						}
					}
				}
			}
			if(i==TRANS_MAXNB)
			{
				NormalTransData.ulAmount*=(1+bcd_long(&NormalTransData.aucOldCardNo[4],8)-bcd_long(&NormalTransData.aucUserCardNo[4],8));
				ucResult=SUCCESS;
			}
		}
		if(ucResult==SUCCESS)
		{
			memcpy(NormalTransData.aucCashierNo,DataSave0.ChangeParamData.aucCashierNo,6);
			ucResult=SAV_BatchLoadSave();
		}
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ֵɹ");
			RunData.uiSaleCardNb++;
			RunData.ulSaleCardAmount+=NormalTransData.ulAmount;
			RunData.ulSaleCountAmount=RunData.ulSaleCountAmount+NormalTransData.ulAmount-NormalTransData.ulAmount*bcd_long(NormalTransData.aucDiscount,4)/1000;
			if(RunData.uiSaleCardNb==1)
			{
	
				PRT_ConstantParam();
				
				if(DataSave0.ConstantParamData.prnflag)
					Os__GB2312_xprint((unsigned char *)"---------------------",FONT); 
				else
					Os__xprint((unsigned char *)"---------------------");
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(aucPrintBuf,(uchar *)"Ա:",7);
				memcpy(&aucPrintBuf[7],NormalTransData.aucCashierNo,6);
				Os__GB2312_xprint(aucPrintBuf,FONT); 
				Os__GB2312_xprint((uchar *)"ֵ",FONT);
				Os__GB2312_xprint((unsigned char *)"κ:",FONT);	
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				aucPrintBuf[30]='\0';
				bcd_asc(aucPrintBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
				Os__GB2312_xprint(aucPrintBuf,FONT); 
				
				Os__xlinefeed(LINENUM*0.5);
				PRT_LocalDateTime();
				Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);

			}
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)"ʼ:",7);	
			bcd_asc(&aucPrintBuf[7],NormalTransData.aucUserCardNo,16);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)"ֹ:",7);	
			bcd_asc(&aucPrintBuf[7],NormalTransData.aucOldCardNo,16);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)":",7);	
			uiSale=1+bcd_long(&NormalTransData.aucOldCardNo[4],8)-bcd_long(&NormalTransData.aucUserCardNo[4],8);
			UTIL_Form_Montant(&aucPrintBuf[7],uiSale,0);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)":",7);	
			UTIL_Form_Montant(&aucPrintBuf[7],NormalTransData.ulAmount,2);
			Os__GB2312_xprint(aucPrintBuf,FONT);
			//PRT_ExpDateTime();			
			Os__xprint((uchar*)"");
			RunData.uiSaleCardDetail+=uiSale;
		}
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
	}
	if(RunData.uiSaleCardNb)
		PRT_BatchLoad();
	return SUCCESS;
}
unsigned char Untouch_Detail(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20],ucI;
	for(ucI=1;ucI<=10;ucI++)
	{
		//Os__clr_display(255);
		ucResult=PBOC_ISOReadRecordbySFI(ICC1, 0x18, ucI);
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,&Iso7816Out.aucData[16],14);
		Os__display(0,0,aucBuf);
		UTIL_Form_Montant(aucBuf,tab_long(&Iso7816Out.aucData[5],4), 2);
		Os__display(1,0,aucBuf);
		if(Os__xget_key()==KEY_CLEAR)
			return ucResult;
		PBOC_ISOSelectFilebyAID(ICC1, (uchar *)"SANDPAY",7);
		PBOC_ISOGetTac(6,ucI);
	}
	return ucResult;

}


unsigned char Untouch_Void(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[30],aucBuf1[30];
	unsigned long ulPreBalance,ulAmount;
	NORMALTRANS tnormal;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_PreComm();
	if(NormalTransData.ucHandFlag)
	{
		while(1)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)" ѳ");
				//Os__GB2312_display(2,0,(uchar *)"뿨:");
				memset(aucBuf,0,sizeof(aucBuf));
				if (UTIL_Input(3,true,16,16,'N',aucBuf,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,aucBuf,16);
				}
			}
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(uchar *)"ٴ뿨");
				memset(aucBuf1,0,sizeof(aucBuf1));
				if (UTIL_Input(3,true,16,16,'N',aucBuf1,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					if(memcmp(aucBuf,aucBuf1,16))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"2벻һ");
						MSG_WaitKey(4);
					}
					else
						break;
				}
			}
			if(ucResult!=SUCCESS)
				break;
		}	
		if(ucResult==SUCCESS)
			ucResult=CFG_SetBatchNumber();
		if(ucResult==SUCCESS)
			ucResult=CFG_CheckCardTbl4Hand();
	}	
	if(RunData.aucWDFunFlag[6]||NormalTransData.ucHandFlag)
	{
		if(ucResult==SUCCESS)
		{
			if(!NormalTransData.ucHandFlag)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"Ȱγû");
				Os__xdelay(10);
				Os__ICC_remove();
			}
			RunData.ucPtcode=NormalTransData.ucPtCode;
			memcpy(&tnormal,&NormalTransData,sizeof(NORMALTRANS));
			ucResult=CashierLogonProcess();
			memcpy(&NormalTransData,&tnormal,sizeof(NORMALTRANS));
			NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		}

	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__GB2312_display(0, 0, "ˮ:");
		if( UTIL_Input(1,true,TRANS_TRACENUMLEN,TRANS_TRACENUMLEN,'N',
					aucBuf,NULL) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}
		RunData.ulOldTraceNumber=asc_long(aucBuf,6);	
	}
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
	}
	if(ucResult==SUCCESS)
		ucResult = SAV_SavedTransIndex(1);
	
	if(ucResult==SUCCESS)
		ucResult = UITL_displayCardNum();

	if(ucResult==SUCCESS&&RunData.aucFunFlag[5]&&RunData.aucFunFlag[11])
	{
		ucResult=UTIL_InputEncryptPIN();
	}
	
	
	if(ucResult==SUCCESS)
		ucResult=Trans_Void();
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	NormalTransData.ucReturnflag=false;
	if(NormalTransData.ucTransType==TRANS_S_VOID)
		ucResult=SAV_VoidSave();
	else
		ucResult=SAV_VoidSave_P();
	if(ucResult!=SUCCESS)
		return ucResult;
	
	PRT_PrintTicket();	
	return SUCCESS;
}


unsigned char Untouch_ChangePin(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[30];
	unsigned long ulPreBalance,ulAmount;
	ucResult=SUCCESS;	
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
	}
	

	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_PreComm();
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_ChangeEncryptPIN();
	}
	if(ucResult!=SUCCESS)
	{
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		       COMMS_FinComm();
		return ucResult;
	}
	
	
	ucResult=Trans_ChangePin();
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"޸ɹ!");
		MSG_WaitKey(3);
	}
		
	return ucResult;
}

unsigned char Untouch_Transfer(void)
{
	unsigned char ucResult = SUCCESS,aucOldZone[3],ucPt,tDx,tTcflag;
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20],aucBuf1[21],aucCheck[20];
	unsigned int    uiTimesOut;
	unsigned int    uiI;

	if(RunData.ucExpire1Flag)
		return ERR_EXPIREDATE;
	if(TRANS_MAXNB-DataSave0.TransInfoData.TransTotal.uiTotalNb<2)
		return ERR_TRANSFILEFULL;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult!=SUCCESS)
		return ucResult;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	if(NormalTransData.ucHandFlag)
	{
		while(1)
		{
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"   ");
				//Os__GB2312_display(2,0,(uchar *)"ת뿨:");
				memset(aucBuf,0,sizeof(aucBuf));
				if (UTIL_Input(3,true,16,16,'N',aucBuf,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					asc_bcd(NormalTransData.aucOldCardNo,8,aucBuf,16);
				}
			}
			if(ucResult==SUCCESS)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(uchar *)"ٴת뿨");
				memset(aucBuf1,0,sizeof(aucBuf1));
				if (UTIL_Input(3,true,16,16,'N',aucBuf1,0) != KEY_ENTER )
				{
					ucResult = ERR_CANCEL;
				}else
				{
					if(memcmp(aucBuf,aucBuf1,16))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"2벻һ");
						MSG_WaitKey(4);
					}
					else
						break;
				}
			}
			if(ucResult!=SUCCESS)
			{
				if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
				{
					COMMS_FinComm();
				}
				return ucResult;
			}
		}	
		if(ucResult==SUCCESS)
			ucResult=CFG_SetBatchNumber();
	}
	else
	{
		if(RunData.ucDxMobile)
		{
			if(ucResult==SUCCESS)
				ucResult=Untouch_DX_WaitCard();
			if(ucResult==SUCCESS)
			{
				if(!RunData.aucZDFunFlag[28])
					return ERR_CARDNOFUN;
			}
		}
		else
		{
			while(1)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,"");
				//Os__GB2312_display(1,0,"ȡ");
				ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
				if(ucResult==0x04)
					break;
			}
			ucResult = SUCCESS;
		}
		memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,10);
	}
	if(ucResult==SUCCESS)
		ucResult = Trans_StatusQuery();
	memcpy(aucOldZone,NormalTransData.aucZoneCode,3);
	ucPt=NormalTransData.ucPtCode;
	tDx=RunData.ucDxMobile;
	tTcflag=NormalTransData.aucFunFlag[18];
	RunData.ucDxMobile=0;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,"()ת");
	if(ucResult==SUCCESS)
	{
		ucResult = Untouch_WaitAllCard();
		if(ucResult==ERR_CANCEL)
		{
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_FinComm();
			}
			return ucResult;
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MAG)
			{
				ucResult=ERR_CARDNOFUN;
			}
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult == SUCCESS )
					SLE4442_ATR(aucBuf);

				if(ucResult==SUCCESS)
					ucResult = SLE4442_ReadCardData();
				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();
				if(ucResult==SUCCESS)
					CFG_SetBatchNumber();
				if(ucResult==SUCCESS)
				{
					if(SleTransData.ucStatus==CARD_RECYCLED)
						ucResult=ERR_CARDRECYCLE;
				}

			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_ReadCardInfo();
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();

				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

				if(ucResult==SUCCESS)
					ucResult=Untouch_CheckRedo();
				if(ucResult==SUCCESS)
				{
					if(SleTransData.ucStatus==CARD_RECYCLED)
						ucResult=ERR_CARDRECYCLE;
				}

			}
		}
			
	}	
	if(ucResult==SUCCESS)
	{
		if(RunData.ucExpireFlag||RunData.ucExpire1Flag)
			ucResult= ERR_EXPIREDATE;
	}
	if(ucResult==SUCCESS)
	{
		if(!memcmp(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,8))
		{
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_FinComm();
			}
			return ERR_SAMECARD;
		}
		if(NormalTransData.aucFunFlag[18]&&!tTcflag)
		{
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				COMMS_FinComm();
			}
			return ERR_TESTTRANSFER;
		}
		NormalTransData.ucTransType=TRANS_S_TRANSPURCHASE;
		ucResult=Untouch_Purchase();
		NormalTransData.ucTransType=TRANS_S_TRANSFER;
		RunData.ulOldTraceNumber=NormalTransData.ulTraceNumber;
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
	}

	if(ucResult==SUCCESS)
		ucResult=SAV_SavedTransIndex(0);

	memcpy(RunData.aucZoneCode,NormalTransData.aucZoneCode,3);
	memcpy(NormalTransData.aucZoneCode,aucOldZone,3);
	RunData.ucPt=NormalTransData.ucPtCode;
	NormalTransData.ucPtCode=ucPt;
	RunData.ucDxMobile=tDx;
	if(ucResult==SUCCESS)
		ucResult=Trans_Transfer();

	if(ucResult==SUCCESS)
	{
		memcpy(NormalTransData.aucUserCardNo,NormalTransData.aucOldCardNo,10);
		NormalTransData.ulAmount-=NormalTransData.ulRateAmount;
		NormalTransData.ulRateAmount=0;
		SAV_TransferSave();
	}
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	if(NormalTransData.ucHandFlag)
	{
		if( ucResult != SUCCESS)
		{
			MSG_DisplayErrMsg(ucResult);
		}
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		return ERR_CANCEL;
	}
	return ucResult;

}


unsigned char Untouch_Retransfer(void)
{
	unsigned char ucResult = SUCCESS,aucOldZone[3],ucPt,aucZoneName1[16];
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20],aucBuf1[21],aucCheck[20];
	unsigned int    uiTimesOut;
	unsigned int    uiI,uiIndex;
	if(RunData.ucExpire1Flag)
		return ERR_EXPIREDATE;

	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,"ۿˮ");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,TRANS_TRACENUMLEN,TRANS_TRACENUMLEN,'N',
				aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			ucResult=XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(DataSave0.SaveTransData.aucOldTrace,aucBuf,6)
				&&DataSave0.SaveTransData.ucTransType==TRANS_S_TRANSFER)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,"ѳɹ");
				MSG_WaitKey(3);
				return ERR_CANCEL;
			}
		}

	}
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			ucResult=XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(DataSave0.SaveTransData.ulTraceNumber==asc_long(aucBuf,6)
				&&DataSave0.SaveTransData.ucTransType==TRANS_S_TRANSPURCHASE
				&&DataSave0.SaveTransData.ucKYFlag==0)
			{
				break;
			}
		}
	}
	if(uiIndex==TRANS_MAXNB)
		return ERR_NOTRANS;
	if(memcmp(NormalTransData.aucUserCardNo,DataSave0.SaveTransData.aucOldCardNo,8))
		return ERR_TRANSCARDDIF;
	if(ucResult==SUCCESS)
	{
		memcpy(aucZoneName1,NormalTransData.aucZoneName1,16);
		memcpy(aucOldZone,NormalTransData.aucZoneCode,3);
		ucPt=NormalTransData.ucPtCode;
		memcpy(DataSave0.SaveTransData.aucOldCardNo,NormalTransData.aucUserCardNo,10);
		memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));				
		NormalTransData.ucTransType=TRANS_S_TRANSFER;
		RunData.ulOldTraceNumber=NormalTransData.ulTraceNumber;
		long_asc(NormalTransData.aucOldTrace,6,&NormalTransData.ulTraceNumber);
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
	}

	if(ucResult==SUCCESS)
		ucResult=SAV_SavedTransIndex(0);

	memcpy(RunData.aucZoneCode,NormalTransData.aucZoneCode,3);
	memcpy(NormalTransData.aucZoneCode,aucOldZone,3);
	RunData.ucPt=NormalTransData.ucPtCode;
	NormalTransData.ucPtCode=ucPt;
	memcpy(NormalTransData.aucZoneName1,aucZoneName1,16);
	if(ucResult==SUCCESS)
		ucResult=Trans_Transfer();

	if(ucResult==SUCCESS)
	{
		memcpy(NormalTransData.aucUserCardNo,NormalTransData.aucOldCardNo,10);
		NormalTransData.ulAmount-=NormalTransData.ulRateAmount;
		NormalTransData.ulRateAmount=0;
		SAV_TransferSave();
	}
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}
unsigned char Untouch_LoadOnline(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],tCardNo[10],aucUseKey[20];
	unsigned long ulMaxAmount,ulAmount,ulPrevEDBalance,ulsieral;
	unsigned char ucStatus,ucLen,aucOutData[300],ucKeyArray,aucFensan[10];
	ulMaxAmount=999999999;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	//if(NormalTransData.ucTransType!=TRANS_S_CASHLOAD)
	if(DataSave0.ConstantParamData.ucYTJFlag==0)
	{
		ucResult=CashierLogonProcess();
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
	}
	while(1)
	{
		ucResult=SAV_SavedTransIndex(0);
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		if(ucResult==SUCCESS)
		{
			if(DataSave0.ConstantParamData.ucYTJFlag==0)
			{
				//Os__clr_display(255);
                //if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
					//Os__GB2312_display(0,0,"ֵֽ");
            //	else
					//Os__GB2312_display(0,0,"ֵ");
				//Os__GB2312_display(1,0,"//ˢ");
			}
			ucResult=Untouch_WaitAllCard();
			if(ucResult==ERR_CANCEL)
				break;
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult == SUCCESS )
					SLE4442_ATR(aucBuf);

				if(ucResult==SUCCESS)
					ucResult = SLE4442_ReadCardData();
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();
				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_ReadCardInfo();

				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();

				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

				if(ucResult==SUCCESS)
					ucResult=Untouch_CheckRedo();

			}
			if(NormalTransData.ucCardType==CARD_MAG)
			{
				if(ucResult==SUCCESS)
				{
					NormalTransData.ucCardType=CARD_MAG;
					asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
					asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[23],2);
					asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[25],6);
					NormalTransData.aucExpiredDate[0]=0x20;
					asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
					ucResult=CFG_CheckCardValid();
				}
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
			{
				if(!RunData.aucZDFunFlag[4])
					ucResult=ERR_NOFUNCTION;
			}
			else
			{
				if(!RunData.aucZDFunFlag[30])
					ucResult=ERR_NOFUNCTION;
			}
		}
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucFunFlag[8]&&NormalTransData.ucTransType==TRANS_S_LOADONLINE)
				ucResult=ERR_CARDNOFUN;
		}
		if(ucResult==SUCCESS&&!DataSave0.ConstantParamData.ucYTJFlag)
		{
			if(RunData.ucPtcode!=NormalTransData.ucPtCode)
				ucResult=ERR_CARDTYPE;
		}

		if((DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
			&&NormalTransData.ucCardType!=CARD_MAG
			||NormalTransData.ucTransType==TRANS_S_CASHLOAD)
			NormalTransData.ucCardLoadFlag=1;
		
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			//Os__clr_display(255);
			if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
			{
				//Os__GB2312_display(0,0,(uchar *)":");
				Os__display(1,0,aucBuf);
			}
			if(DataSave0.ConstantParamData.ucYTJFlag)
			{
				//Os__GB2312_display(2,0,(uchar *)"ֵ:");
				NormalTransData.ulAmount=asc_long(ReceiveTransData.aucAmount,12);
				memset(aucBuf,0,sizeof(aucBuf));
				UTIL_Form_Montant(aucBuf, NormalTransData.ulAmount, 2);
				Os__display(3,0,aucBuf);
				if(MSG_WaitKey(30)!=KEY_ENTER)
					ucResult= ERR_CANCEL;
				
			}
			else
			{
				//Os__GB2312_display(2,0,(uchar *)"ֵ:");
				ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, ulMaxAmount);
			}
		}
		if(RunData.aucFunFlag[1])
			ulPrevEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
		else
			ulPrevEDBalance=NormalTransData.ulPrevEDBalance;
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount>NormalTransData.ulMaxAmount)
				return ERR_OVERAMOUNT;
		}
		/*if(ucResult==SUCCESS&&RunData.aucFunFlag[5]&&NormalTransData.ucCardLoadFlag!=1)
		{
			ucResult=UTIL_InputEncryptPIN();
		}*/
		if(NormalTransData.ucCardLoadFlag==1)
		{
			if(ucResult==SUCCESS&&RunData.ucInputmode==0&&NormalTransData.ucCardType!=CARD_MEM)
			{
				memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
				while(1)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Żԭ");
					ucResult=MIFARE_WaitCard();
					if(ucResult!=SUCCESS)
						continue;
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Ժ");
					ucResult=Untouch_ReadCardInfo();
					if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"Żԭ");
						continue;
					}
					break;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.aucFunFlag[1])
					{
						if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
							NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
					}
				}
			}
			if(NormalTransData.ucCardType==CARD_CPU)
			{
				if(ucResult==SUCCESS)
				{
					Uart_Printf("Ȧʼ\n");
					memset(aucBuf,0,sizeof(aucBuf));
					aucBuf[0]=0x00;
					long_tab(&aucBuf[1],4,&NormalTransData.ulAmount);
					asc_bcd(&aucBuf[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
					//memcpy(&aucBuf[5],&DataSave0.ConstantParamData.aucOnlytermcode[2],6);
					ucResult=PBOC_ISOInitForLoad(ICC1,aucBuf);
				}
				if(ucResult==SUCCESS)
				{
					memcpy(RunData.aucPreBalance,Iso7816Out.aucData,4);
					//NormalTransData.ulPrevEDBalance=tab_long(RunData.aucPreBalance,4);
					memcpy(RunData.aucTrace,&Iso7816Out.aucData[4],2);
					RunData.ucKeyVersion=Iso7816Out.aucData[6];
					RunData.ucArithId=Iso7816Out.aucData[7];
					memcpy(RunData.aucRandom,&Iso7816Out.aucData[8],4);
					memcpy(RunData.aucMAC1,&Iso7816Out.aucData[12],4);
				}
			}
		}
		if(ucResult==SUCCESS)
			ucResult=Trans_Preload();
		if(NormalTransData.ucCardLoadFlag==1)
		{
			if(NormalTransData.ucCardType==CARD_CPU)
			{
				if(ucResult==SUCCESS)
				{
					NormalTransData.ulAfterEDBalance=tab_long(RunData.aucPreBalance,4)+NormalTransData.ulAmount;
					memset(aucBuf,0,sizeof(aucBuf));
					memcpy(aucBuf,NormalTransData.aucDateTime,7);
					memcpy(&aucBuf[7],RunData.aucMAC2,4);
					Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
										0, sizeof(unsigned char));	
					ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
					if(ucResult!=SUCCESS)
						return ucResult;
					ucResult=PBOC_ISOLoad(ICC1, aucBuf, 11);
					if(ucResult!=SUCCESS)
					{
						Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
										0x55, sizeof(unsigned char));	
						XDATA_Write_Vaild_File(DataSaveReversalISO8583);

					}
				}
				if(ucResult==SUCCESS)
				{
					memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);
					memcpy(&NormalTransData.aucSamTace[2],RunData.aucTrace,2);
				}
				if(ucResult==SUCCESS)
				{
					PBOC_ISOGetChallenge(ICC1);
				}
				if(ucResult==SUCCESS)
				{
					memset(aucBuf,0,sizeof(aucBuf));
					memcpy(aucBuf,Iso7816Out.aucData,4);
					memset(aucFensan,0,sizeof(aucFensan));
					memcpy(aucFensan,NormalTransData.aucCardSerial,4);
					memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
					PBOC_Fensan_key(aucFensan,
						KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
					des(aucBuf,aucBuf,aucUseKey);
					desm1(aucBuf,aucBuf,&aucUseKey[8]);
					des(aucBuf,aucBuf,aucUseKey);
					PBOC_ISOExternalauth(ICC1, aucBuf);
				}
				if(ucResult==SUCCESS)
				{
					PBOC_UpdateBinaryFile(ICC1,0x15, 24, 4, NormalTransData.aucExpiredDate);
				}
				if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
				{
					if(RunData.aucFunFlag[9])
						aucBuf[0]=0x01;
					else
						aucBuf[0]=0x04;
					ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
				}
			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_M1Credit();
				if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
				{
					if(RunData.aucFunFlag[9])
						RunData.ucTagetStatus=0x01;
					else
						RunData.ucTagetStatus=0x04;
					ucResult=MIFARE_ChangeStatus();
				}
			}	
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult==SUCCESS)
					ucResult = SLE4442_CardTrans();
			}
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
				ucResult=SAV_CashLoadSave();
			else
				ucResult=SAV_LoadOnlineSave();
		}
		if(DataSave0.ConstantParamData.ucYTJFlag)
			Port_SendPortResult(ucResult);
		if(ucResult==SUCCESS)
		{
			PRT_PrintTicket();
		}
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
		if(DataSave0.ConstantParamData.ucYTJFlag)
			break;
	}
	return SUCCESS;

}
unsigned char Untouch_LoadByCredit(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],tCardNo[10],aucUseKey[20];
	unsigned long ulMaxAmount,ulAmount,ulPrevEDBalance,ulsieral;
	unsigned char ucStatus,ucLen,aucOutData[300],ucKeyArray,aucFensan[10];
	ulMaxAmount=999999999;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=SAV_SavedTransIndex(0);
	NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
	NormalTransData.ucPackFlag=1;
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		////Os__GB2312_display(0,0,"пֵ");
		//Os__GB2312_display(1,0,"ˢ//ɼ¿");
		ucResult=Untouch_WaitAllCard();
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ucCardType==CARD_MEM)
		{
			if(ucResult == SUCCESS )
				SLE4442_ATR(aucBuf);

			if(ucResult==SUCCESS)
				ucResult = SLE4442_ReadCardData();
			if(ucResult==SUCCESS)
				ucResult=CFG_SetBatchNumber();
			if(ucResult==SUCCESS)
				ucResult=CFG_CheckCardValid();

		}
		if(NormalTransData.ucCardType==CARD_M1)
		{
			if(ucResult==SUCCESS)
				ucResult=Untouch_ReadCardInfo();

			if(ucResult==SUCCESS)
				ucResult=CFG_SetBatchNumber();

			if(ucResult==SUCCESS)
				ucResult=CFG_CheckCardValid();

			if(ucResult==SUCCESS)
				ucResult=Untouch_CheckRedo();

		}
		if(NormalTransData.ucCardType==CARD_MAG)
		{
			if(ucResult==SUCCESS)
			{
				NormalTransData.ucCardType=CARD_MAG;
				asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
				asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[23],2);
				asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[25],6);
				NormalTransData.aucExpiredDate[0]=0x20;
				asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
				ucResult=CFG_CheckCardValid();
			}
		}
	}
	if(ucResult==SUCCESS)
	{
		if(!RunData.aucZDFunFlag[12])
			ucResult=ERR_NOFUNCTION;
	}
	if(ucResult==SUCCESS)
	{
		if(!RunData.aucFunFlag[8])
			ucResult=ERR_CARDNOFUN;
	}
	if(ucResult==SUCCESS)
	{
		if(RunData.ucPtcode!=NormalTransData.ucPtCode)
			ucResult=ERR_CARDTYPE;
	}

	if((DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
		&&NormalTransData.ucCardType!=CARD_MAG)
		NormalTransData.ucCardLoadFlag=1;
	
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
		//Os__clr_display(255);
		if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
		{
			//Os__GB2312_display(0,0,(uchar *)":");
			Os__display(1,0,aucBuf);
		}
		//Os__GB2312_display(2,0,(uchar *)"ֵ:");
		ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, ulMaxAmount);
	}
	if(RunData.aucFunFlag[1])
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulYaAmount;
	else
		ulPrevEDBalance=NormalTransData.ulPrevEDBalance;
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount>NormalTransData.ulMaxAmount)
			ucResult=ERR_OVERAMOUNT;
	}
	if(ucResult==SUCCESS&&RunData.aucFunFlag[5]&&NormalTransData.ucCardLoadFlag!=1)
	{
		ucResult=UTIL_InputEncryptPIN();
	}

	//תп
	if(ucResult==SUCCESS)
		ucResult=MASAPP_Trans2Other();
	
	if(NormalTransData.ucCardLoadFlag==1)
	{
		if(ucResult==SUCCESS&&RunData.ucInputmode==0&&NormalTransData.ucCardType!=CARD_MEM)
		{
			memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
			while(1)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Żԭ");
				ucResult=MIFARE_WaitCard();
				if(ucResult!=SUCCESS)
					continue;
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Ժ");
				ucResult=Untouch_ReadCardInfo();
				if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Żԭ");
					continue;
				}
				break;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.aucFunFlag[1])
				{
					if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
						NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
				}
			}
		}
		if(NormalTransData.ucCardType==CARD_CPU)
		{
			if(ucResult==SUCCESS)
			{
				Uart_Printf("Ȧʼ\n");
				memset(aucBuf,0,sizeof(aucBuf));
				aucBuf[0]=0x00;
				long_tab(&aucBuf[1],4,&NormalTransData.ulAmount);
				asc_bcd(&aucBuf[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
				//memcpy(&aucBuf[5],&DataSave0.ConstantParamData.aucOnlytermcode[2],6);
				ucResult=PBOC_ISOInitForLoad(ICC1,aucBuf);
			}
			if(ucResult==SUCCESS)
			{
				memcpy(RunData.aucPreBalance,Iso7816Out.aucData,4);
				//NormalTransData.ulPrevEDBalance=tab_long(RunData.aucPreBalance,4);
				memcpy(RunData.aucTrace,&Iso7816Out.aucData[4],2);
				RunData.ucKeyVersion=Iso7816Out.aucData[6];
				RunData.ucArithId=Iso7816Out.aucData[7];
				memcpy(RunData.aucRandom,&Iso7816Out.aucData[8],4);
				memcpy(RunData.aucMAC1,&Iso7816Out.aucData[12],4);
			}
		}
	}

	if(ucResult==SUCCESS)
		ucResult=Trans_Preload();

	if(NormalTransData.ucCardLoadFlag==1)
	{
		if(NormalTransData.ucCardType==CARD_CPU)
		{
			if(ucResult==SUCCESS)
			{
				NormalTransData.ulAfterEDBalance=tab_long(RunData.aucPreBalance,4)+NormalTransData.ulAmount;
				memset(aucBuf,0,sizeof(aucBuf));
				memcpy(aucBuf,NormalTransData.aucDateTime,7);
				memcpy(&aucBuf[7],RunData.aucMAC2,4);
				Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
									0, sizeof(unsigned char));	
				ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
				if(ucResult!=SUCCESS)
					return ucResult;
				ucResult=PBOC_ISOLoad(ICC1, aucBuf, 11);
				if(ucResult!=SUCCESS)
				{
					Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
									0x55, sizeof(unsigned char));	
					XDATA_Write_Vaild_File(DataSaveReversalISO8583);

				}
			}
			if(ucResult==SUCCESS)
			{
				memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);
				memcpy(&NormalTransData.aucSamTace[2],RunData.aucTrace,2);
			}
			if(ucResult==SUCCESS)
			{
				PBOC_ISOGetChallenge(ICC1);
			}
			if(ucResult==SUCCESS)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				memcpy(aucBuf,Iso7816Out.aucData,4);
				memset(aucFensan,0,sizeof(aucFensan));
				memcpy(aucFensan,NormalTransData.aucCardSerial,4);
				memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
				PBOC_Fensan_key(aucFensan,
					KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
				des(aucBuf,aucBuf,aucUseKey);
				desm1(aucBuf,aucBuf,&aucUseKey[8]);
				des(aucBuf,aucBuf,aucUseKey);
				PBOC_ISOExternalauth(ICC1, aucBuf);
			}
			if(ucResult==SUCCESS)
			{
				PBOC_UpdateBinaryFile(ICC1,0x15, 24, 4, NormalTransData.aucExpiredDate);
			}
			if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
			{
				if(RunData.aucFunFlag[9])
					aucBuf[0]=0x01;
				else
					aucBuf[0]=0x04;
				ucResult=PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
			}
		}
		if(NormalTransData.ucCardType==CARD_M1)
		{
			if(ucResult==SUCCESS)
				ucResult=Untouch_M1Credit();
			if(ucResult==SUCCESS&&SleTransData.ucStatus==CARD_RECYCLED)
			{
				if(RunData.aucFunFlag[9])
					RunData.ucTagetStatus=0x01;
				else
					RunData.ucTagetStatus=0x04;
				ucResult=MIFARE_ChangeStatus();
			}
		}	
		if(NormalTransData.ucCardType==CARD_MEM)
		{
			if(ucResult==SUCCESS)
				ucResult = SLE4442_CardTrans();
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=SAV_LoadOnlineSave();
	}
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	if( ucResult != SUCCESS)			
	{
		MSG_DisplayErrMsg(ucResult);
	}
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"γ");
	Os__xdelay(10);
	Os__ICC_remove();
	while(1)
	{
		ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
		if(ucResult==0x04)
		{
			ucResult= SUCCESS;
			break;
		}
		//Os__clr_display(1);
		//Os__GB2312_display(1,0,"ȡ");
	}
	return SUCCESS;

}
unsigned char Untouch_Changecard(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char aucBuf[30],aucUsCd[30],ucStatus,aucFensan[20],aucUseKey[20];
	unsigned long ulAmount;
	unsigned char aucOutData[300];
 	unsigned char ucLen,tCardNo[20];
	int i;
	ucResult = SAV_SavedTransIndex(0);	
	if(ucResult!=SUCCESS)
		return ucResult;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,"");
		//Os__GB2312_display(1,0,"()¿");
		ucResult=Untouch_WaitAllCard();
		if(ucResult==ERR_CANCEL)
			return ucResult;
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ucCardType==CARD_MAG)
		{
			ucResult=ERR_CARDNOFUN;
		}
		if(NormalTransData.ucCardType==CARD_MEM)
		{
			if(ucResult == SUCCESS )
				SLE4442_ATR(aucBuf);

			if(ucResult==SUCCESS)
				ucResult = SLE4442_ReadCardData();
			if(ucResult==SUCCESS)
				ucResult=CFG_SetBatchNumber();
			if(ucResult==SUCCESS)
				ucResult=CFG_CheckCardValid();

		}
		if(NormalTransData.ucCardType==CARD_M1)
		{
			if(ucResult==SUCCESS)
				ucResult=Untouch_ReadCardInfo();

			if(ucResult==SUCCESS)
				ucResult=CFG_SetBatchNumber();

			if(ucResult==SUCCESS)
				ucResult=CFG_CheckCardValid();

			if(ucResult==SUCCESS)
				ucResult=Untouch_CheckRedo();

		}
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ulTrueBalance!=0)
			ucResult=ERR_AMOUNTNOTZERO;
	}
	if(ucResult==SUCCESS)
	{
		if(!RunData.aucZDFunFlag[43])
			ucResult=ERR_NOFUNCTION;
	}
	if(ucResult==SUCCESS)
	{
		if(RunData.ucPtcode!=NormalTransData.ucPtCode)
			ucResult=ERR_CARDTYPE;
	}
	
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__GB2312_display(0, 0, "ԭ:");
		if( UTIL_Input(1,true,16,16,'N',aucBuf,NULL) != KEY_ENTER )
		{
			ucResult=ERR_CANCEL;
		}
		if(ucResult==SUCCESS)
		{
			bcd_asc(aucUsCd,NormalTransData.aucUserCardNo,16);
			//if(memcmp(aucBuf,aucUsCd,4))
			//	ucResult= ERR_TRANS_CARDTYPE;
			asc_bcd(NormalTransData.aucOldCardNo,8,aucBuf,16);
		}
	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
        //if(RunData.aucFunFlag[11])
			//Os__GB2312_display(0,0,(uchar *)":");
    //	else
			//Os__GB2312_display(0,0,(uchar *)"֤:");
		if(RunData.aucFunFlag[11])
			ucResult=UTIL_InputEncryptPIN();
		else
		{
			memset(aucBuf,0,sizeof(aucBuf));
			if(UTIL_Input(1,true,6,6,'N',aucBuf,0)!=KEY_ENTER)
				ucResult= ERR_CANCEL;
			memcpy(RunData.aucPin,aucBuf,6);
		}
		if(NormalTransData.ucCardType==CARD_CPU)
		{
			if(ucResult==SUCCESS)
				ucResult=Trans_ChangeQuery();
			if(ucResult==SUCCESS&&RunData.ucInputmode==0&&NormalTransData.ucCardType!=CARD_MEM&&!RunData.ucDxMobile)
			{
				memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
				while(1)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Żԭ");
					ucResult=MIFARE_WaitCard();
					if(ucResult!=SUCCESS)
						continue;
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Ժ");
					ucResult=Untouch_ReadCardInfo();
					if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar *)"Żԭ");
						continue;
					}
					break;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.aucFunFlag[1])
					{
						if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
							NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
					}
				}
			}
			if(ucResult==SUCCESS)
			{
				Uart_Printf("Ȧʼ\n");
				memset(aucBuf,0,sizeof(aucBuf));
				aucBuf[0]=0x00;
				long_tab(&aucBuf[1],4,&NormalTransData.ulAmount);
				asc_bcd(&aucBuf[5],4,DataSave0.ConstantParamData.aucOnlytermcode,8);
				//memcpy(&aucBuf[5],&DataSave0.ConstantParamData.aucOnlytermcode[2],6);
				ucResult=PBOC_ISOInitForLoad(ICC1,aucBuf);
			}
			if(ucResult==SUCCESS)
			{
				memcpy(RunData.aucPreBalance,Iso7816Out.aucData,4);
				NormalTransData.ulPrevEDBalance=tab_long(RunData.aucPreBalance,4);
				memcpy(RunData.aucTrace,&Iso7816Out.aucData[4],2);
				RunData.ucKeyVersion=Iso7816Out.aucData[6];
				RunData.ucArithId=Iso7816Out.aucData[7];
				memcpy(RunData.aucRandom,&Iso7816Out.aucData[8],4);
				memcpy(RunData.aucMAC1,&Iso7816Out.aucData[12],4);
			}
		}
	}
	
	if(ucResult==SUCCESS)
		ucResult=Trans_ChangeCard();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
		
	if(NormalTransData.ucCardType==CARD_MEM)
	{
		if(ucResult==SUCCESS)
			ucResult = SLE4442_CardTrans();
	}
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		if(ucResult==SUCCESS)
		{
			NormalTransData.ulAfterEDBalance=NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount;
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,NormalTransData.aucDateTime,7);
			memcpy(&aucBuf[7],RunData.aucMAC2,4);
			Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
								0, sizeof(unsigned char));	
			ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
			if(ucResult==SUCCESS)
				ucResult=PBOC_ISOLoad(ICC1, aucBuf, 11);
			if(ucResult!=SUCCESS)
			{
				Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
								0x55, sizeof(unsigned char));	
				XDATA_Write_Vaild_File(DataSaveReversalISO8583);
			}
		}
		if(ucResult==SUCCESS)
		{
			memcpy(NormalTransData.aucTraceTac,Iso7816Out.aucData,4);
			memcpy(&NormalTransData.aucSamTace[2],RunData.aucTrace,2);
		}
		if(ucResult==SUCCESS)
		{
			PBOC_ISOGetChallenge(ICC1);
		}
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,Iso7816Out.aucData,4);
			memset(aucFensan,0,sizeof(aucFensan));
			memcpy(aucFensan,NormalTransData.aucCardSerial,4);
			memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
			PBOC_Fensan_key(aucFensan,
				KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
			des(aucBuf,aucBuf,aucUseKey);
			desm1(aucBuf,aucBuf,&aucUseKey[8]);
			des(aucBuf,aucBuf,aucUseKey);
			PBOC_ISOExternalauth(ICC1, aucBuf);
		}
		if(ucResult==SUCCESS)
		{
			aucBuf[0]=1;
			PBOC_UpdateBinaryFile(ICC1,0x16, 0, 1, aucBuf);
		}
	}
	if(NormalTransData.ucCardType==CARD_M1)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_M1Credit();
		if(ucResult==SUCCESS)
		{
			UTIL_IncreaseTraceNumber(0);
			RunData.ucTagetStatus=0x01;
			ucResult=MIFARE_ChangeStatus();
		}
	}

	
	if(ucResult==SUCCESS)
	{
		SAV_ChangeCDSave();
		PRT_PrintTicket();	
	}
	if( ucResult != SUCCESS)			
	{
		MSG_DisplayErrMsg(ucResult);
	}
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"γ");
	Os__xdelay(10);
	Os__ICC_remove();
	while(1)
	{
		ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
		if(ucResult==0x04)
		{
			ucResult= SUCCESS;
			break;
		}
		//Os__clr_display(1);
		//Os__GB2312_display(1,0,"ȡ");
	}
	
	return ucResult;
}

unsigned char Untouch_ChangeExp(void)
{
	unsigned char ucResult=SUCCESS,tCardNo[10],exptype;
	unsigned char aucBuf[30],ucStatus,aucBuf1[7],aucFensan[20],aucUseKey[20];
	unsigned char Date_Time[7],display[7],tmp_date[9];
    	unsigned char days[12]={31,29,31,30,31,30,31,31,30,31,30,31};
    	unsigned char month,day,hour,min,sec; 
	unsigned long ulAmount,ullong1,ullong2,ulExpNum=0;
	unsigned char aucOutData[300];
 	unsigned char ucLen,ucJ,ucOffset,ucI,ucInput;
	int i,iWt;
	unsigned char aucBuf2[17],aucWriteKey1[6];
	short iLen;
	
	ucResult = SAV_SavedTransIndex(0);	
	if(ucResult!=SUCCESS)
		return ucResult;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=CashierLogonProcess();
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	if(DataSave0.ChangeParamData.ucExpFlag!=FLAG_LOGON_OK)
	{
		ucResult=ExpLogonProcess();
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}

	}
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,":");
	//Os__GB2312_display(1,0,"1.");
	//Os__GB2312_display(2,0,"2.");
	ucInput=0;
	while(ucInput!='1'&&ucInput!='2'&&ucInput!=KEY_CLEAR)
		ucInput=MSG_WaitKey(60);
	if(ucInput==KEY_CLEAR)
		return ERR_CANCEL;
	else
		exptype=ucInput;
	while(1)
	{
		ucResult = SAV_SavedTransIndex(0);	
		if(ucResult!=SUCCESS)
			return ucResult;
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"");
			//Os__GB2312_display(1,0,"()");
			ucResult=Untouch_WaitAllCard();
			if(ucResult==ERR_CANCEL)
				break;
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(ucResult == SUCCESS )
					SLE4442_ATR(aucBuf);

				if(ucResult==SUCCESS)
					ucResult = SLE4442_ReadCardData();
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();
				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_ReadCardInfo();

				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();

				if(ucResult==SUCCESS)
					ucResult=CFG_CheckCardValid();

				if(ucResult==SUCCESS)
					ucResult=Untouch_CheckRedo();

			}
			if(NormalTransData.ucCardType==CARD_MAG)
			{
				if(ucResult==SUCCESS)
				{
					asc_bcd(NormalTransData.aucUserCardNo,8,EXTRATRANS_aucISO2,16);
					asc_bcd(&NormalTransData.ucPtCode,1,&EXTRATRANS_aucISO2[20],2);
					asc_bcd(NormalTransData.aucZoneCode,3,&EXTRATRANS_aucISO2[22],6);
					NormalTransData.aucExpiredDate[0]=0x20;
					asc_bcd(&NormalTransData.aucExpiredDate[1],3,&EXTRATRANS_aucISO3[21],6);
					ucResult=CFG_CheckCardValid();
				}
				if(ucResult==SUCCESS)
					ucResult=CFG_SetBatchNumber();
			}
		}
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[44])
				ucResult=ERR_NOFUNCTION;
		}
		if(ucResult==SUCCESS&&NormalTransData.ucCardType==CARD_M1)
			ucResult=Trans_M1KeyQuery();
		if(ucResult==SUCCESS)
			ucResult=UTIL_GetNewExp();
		if(exptype=='1'||ulExpNum==0)
		{
			if(ucResult==SUCCESS)
			{
				while(1)
				{
					ucResult=SUCCESS;
					//Os__clr_display(255);
					memset(aucBuf,0,sizeof(aucBuf));
					memcpy(aucBuf,(unsigned char *)"ԭЧ:",9);
					//Os__GB2312_display(0,0,aucBuf);
					memset(aucBuf,' ',sizeof(aucBuf));
					bcd_asc(&aucBuf[6],NormalTransData.aucExpiredDate,4);	
					aucBuf[10]='/';
					bcd_asc(&aucBuf[11],&NormalTransData.aucExpiredDate[2],2);	
					aucBuf[13]='/';
					bcd_asc(&aucBuf[14],&NormalTransData.aucExpiredDate[3],2);	
					//Os__GB2312_display(1,0,aucBuf);
					memset(aucBuf,' ',sizeof(aucBuf));
					memcpy(aucBuf,(unsigned char *)"Ч:",9);
					//Os__GB2312_display(2,0,aucBuf);
					memset(aucBuf,' ',sizeof(aucBuf));
					bcd_asc(&aucBuf[6],NormalTransData.aucNewExpiredDate,4);	
					aucBuf[10]='/';
					bcd_asc(&aucBuf[11],&NormalTransData.aucNewExpiredDate[2],2);	
					aucBuf[13]='/';
					bcd_asc(&aucBuf[14],&NormalTransData.aucNewExpiredDate[3],2);	
					//Os__GB2312_display(3,0,aucBuf);
					ucInput=0;
					while(ucInput!=KEY_ENTER&&ucInput!=KEY_CLEAR&&ucInput!=KEY_BCKSP)
						ucInput=MSG_WaitKey(60);
					if(ucInput==KEY_BCKSP)
					{
						memset(aucBuf,0,sizeof(aucBuf));
				            	if(UTIL_Input_XY(3,6,false,8,8,'N',aucBuf,0) != KEY_ENTER)
							ucResult= ERR_CANCEL;
						if(ucResult==SUCCESS)
						{
							memset(aucBuf1,0,sizeof(aucBuf1));
							asc_bcd(aucBuf1,4,aucBuf,8);
							ucResult=CFG_ChkExpireValid(aucBuf1);
							if(ucResult!=SUCCESS)
								MSG_DisplayErrMsg(ucResult);
						}
						if(ucResult==SUCCESS)
							asc_bcd(NormalTransData.aucNewExpiredDate,4,aucBuf,8);
					}
					else if(ucInput==KEY_ENTER)
						{
							ucResult=CFG_ChkExpireValid(NormalTransData.aucNewExpiredDate);
							if(ucResult!=SUCCESS)
								MSG_DisplayErrMsg(ucResult);
							else
								break;
						}
					else if(ucInput==KEY_CLEAR)
					{
						ucResult=ERR_CANCEL;
						break;
					}
					
				}
			}
			CFG_GetDateTime();
			if(ucResult==SUCCESS)
			{
				NormalTransData.ucPackFlag=0;
				if(ucResult==SUCCESS)
				{
					while(1)
					{
						//Os__clr_display(255);
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(unsigned char *)":",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);	
						//Os__GB2312_display(0,0,aucBuf);
						memset(aucBuf,0,sizeof(aucBuf));
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulAmount,2);	
						memcpy(aucBuf,(unsigned char *)":",7);
						//Os__GB2312_display(1,0,aucBuf);
						bcd_asc(aucBuf1,NormalTransData.aucZoneCode,6);
						if(NormalTransData.ulAmount!=0)
						{
							if(NormalTransData.ucPtCode==0x01
								&&memcmp(aucBuf1,(unsigned char *)"001",3))
							{
								//Os__GB2312_display(2,0,(unsigned char *)"ÿֻȡֽ");
								//Os__GB2312_display(3,0,(unsigned char *)"1.");
							}
							else
							{
								//Os__GB2312_display(2,0,(unsigned char *)"ѡȡʽ");
								//Os__GB2312_display(3,0,(unsigned char *)"1.ֽ 2.Ƭ");
							}
								
						}
						else
						{
							//Os__GB2312_display(3,0,(unsigned char *)"1.");
						}
						////Os__GB2312_display(3,0,(unsigned char *)"0.޸ѽ");
						ucInput=0;
						if(NormalTransData.ulAmount!=0
							&&(NormalTransData.ucPtCode!=0x01
							||!memcmp(aucBuf1,(unsigned char *)"001",3)))
						{
							while(ucInput!='1'&&ucInput!='2'&&ucInput!=KEY_BCKSP
								&&ucInput!=KEY_CLEAR&&ucInput!=ERR_APP_TIMEOUT)
							{
								ucInput=MSG_WaitKey(60);
							}
						}else
						{
							while(ucInput!='1'&&ucInput!=KEY_BCKSP
								&&ucInput!=KEY_CLEAR&&ucInput!=ERR_APP_TIMEOUT)
							{
								ucInput=MSG_WaitKey(60);
							}
						}
						if(ucInput=='2')
						{
							if(NormalTransData.ulAmount>NormalTransData.ulPrevEDBalance)
							{
								//Os__clr_display(255);
								//Os__GB2312_display(1,0,(unsigned char *)"");
								MSG_WaitKey(3);
							}
							else
							{
								NormalTransData.ucPackFlag='1';
								break;
							}
						}
						else if(ucInput=='1')
						{
							NormalTransData.ucPackFlag='2';
							break;
						}
						else if(ucInput==KEY_BCKSP)
						{
							//Os__clr_display(255);
							memset(aucBuf,0,sizeof(aucBuf));
							memcpy(aucBuf,(unsigned char *)"ǰ:",11);
							//Os__GB2312_display(0,0,aucBuf);
							memset(aucBuf,' ',sizeof(aucBuf));
							UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulAmount,2);	
							//Os__GB2312_display(1,0,aucBuf);
							memset(aucBuf,0,sizeof(aucBuf));
							memcpy(aucBuf,(unsigned char *)"Ľ:",11);
							//Os__GB2312_display(2,0,aucBuf);
							UTIL_InputAmount(3,&NormalTransData.ulAmount,0,99999999);	
						}
						else
						{
							ucResult=ERR_CANCEL;
							break;
						}
					}
				}
			}	
		}
		if(ucResult==SUCCESS)
		{
			if(NormalTransData.ulAmount>NormalTransData.ulPrevEDBalance
				&&NormalTransData.ucPackFlag=='1')
				ucResult=ERR_LESSBALANCE;
		}
		if(ucResult==SUCCESS
			&&((RunData.ucInputmode==0&&NormalTransData.ucCardType==CARD_CPU)
			||NormalTransData.ucCardType==CARD_M1))
		{
			memcpy(tCardNo,NormalTransData.aucUserCardNo,10);
			while(1)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Żԭ");
				ucResult=MIFARE_WaitCard();
				if(ucResult!=SUCCESS)
					continue;
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"Ժ");
				memcpy(aucWriteKey1,RunData.aucWriteKey1,6);
				ucResult=Untouch_ReadCardInfo();
				if((ucResult!=SUCCESS&&ucResult!=ERR_CANCEL)||memcmp(tCardNo,NormalTransData.aucUserCardNo,10))
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"Żԭ");
					continue;
				}
				break;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.aucFunFlag[1])
				{
					if(NormalTransData.ulPrevEDBalance>=NormalTransData.ulYaAmount)
						NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
				}
			}
		}

		if(ucResult == SUCCESS)
		{
			if(NormalTransData.ucPackFlag=='1'&&NormalTransData.ulAmount)
			{
				if(ucResult==SUCCESS)
				{
					Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
									0,sizeof(unsigned char));
					XDATA_Write_Vaild_File(DataSaveReversalISO8583);
					switch(NormalTransData.ucCardType)
					{
						case CARD_M1:ucResult = Untouch_M1Debit();break;
						case CARD_CPU:ucResult = PBOC_DebitPart1();break;
						case CARD_MEM:
							ucResult = SLE4442_CardTrans();
							if(ucResult!=SUCCESS)
							{
								DataSave0.RedoTrans.ucRedoFlag=1;
								XDATA_Write_Vaild_File(DataSaveRedoTrans);
							}

							break;
						default:break;
					}
				}
			}
		}
		if(ucResult == SUCCESS)
		{
			if(NormalTransData.ucCardType==CARD_MEM)
			{
				if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
					ucOffset=59;
				else
					ucOffset=72;
				memcpy(aucBuf,NormalTransData.aucNewExpiredDate,4);
				for(iWt=0;iWt<3;iWt++)
				{
					for(ucI=0;ucI<4;ucI++)
					{
						if(ucResult == SUCCESS)
						{
							ucResult=SLE4442_UpdateMainMemory(aucBuf[ucI],ucOffset+ucI);
						}
					}
					
					if(ucResult!=SUCCESS)
					{
						return ucResult;
					}
					
					memset(aucOutData,0,sizeof(aucOutData));
					ucResult= SLE4442_ReadMainMemory(aucOutData,ucOffset,4);
					
					if(!memcmp(aucOutData,aucBuf,4))
						break;
				}
				if(iWt==3)
					return ERR_WRITEMEMORY;
			}
			else if(NormalTransData.ucCardType==CARD_CPU)
			{
				if(ucResult==SUCCESS)
				{
					ucResult=PBOC_ISOGetChallenge(ICC1);
				}
				if(ucResult==SUCCESS)
				{
					memset(aucBuf,0,sizeof(aucBuf));
					memcpy(aucBuf,Iso7816Out.aucData,4);
					memset(aucFensan,0,sizeof(aucFensan));
					memcpy(aucFensan,NormalTransData.aucCardSerial,4);
					memcpy(&aucFensan[4],&NormalTransData.aucUserCardNo[4],4);
					PBOC_Fensan_key(aucFensan,
						KEYINDEX_PURSECARD0_UPDATEKEY,aucUseKey);
					des(aucBuf,aucBuf,aucUseKey);
					desm1(aucBuf,aucBuf,&aucUseKey[8]);
					des(aucBuf,aucBuf,aucUseKey);
					ucResult=PBOC_ISOExternalauth(ICC1, aucBuf);
				}
				if(ucResult==SUCCESS)
				{
					ucResult=PBOC_UpdateBinaryFile(ICC1,0x15, 24, 4, NormalTransData.aucNewExpiredDate);
				}
			}
			else if(NormalTransData.ucCardType==CARD_M1)
			{
				Uart_Printf("111111\n");
				if(ucResult==SUCCESS)
					ucResult = MifareCard_Init(aucOutData,&ucLen);
				if(ucResult==SUCCESS)
				{
					Uart_Printf("222222\n");
					memcpy(RunData.aucWriteKey1,aucWriteKey1,6);
					ucResult = CLASSIC_Auth(OSMIFARE_AUTH_TYPE_A,0x01,OSMIFARE_KEY_TYPE_B,RunData.aucWriteKey1);
				}
				if(ucResult==SUCCESS)
				{
					Uart_Printf("333333\n");
					ucResult = CLASSIC_ReadBlock(4,aucOutData);
					Data_XOR(0,aucOutData,16);
				}
				if(ucResult==SUCCESS)
				{
					Uart_Printf("444444\n");
					memcpy(aucOutData,NormalTransData.aucNewExpiredDate,4);
					if(NormalTransData.ucNewOrOld==0)
					{
						aucOutData[15]=0;
						for(i=0;i<15;i++)
						{
							aucOutData[15]^=aucOutData[i];
						}
					}
					aucOutData[15] = Data_XOR(0,aucOutData,15);
					ucResult = CLASSIC_WriteBlock(4,aucOutData);
					//ucResult = Untouch_ChangeBlock_tmp(&ucStatus,aucOutData);
				}
			}
			
		}

	Uart_Printf("55555555ucResult===%02x\n",ucResult);
		if(ucResult==SUCCESS)
		{		
			if(NormalTransData.ucPackFlag=='1'&&NormalTransData.ulAmount)
				NormalTransData.ulAfterEDBalance=NormalTransData.ulPrevEDBalance-NormalTransData.ulAmount;
			else
				NormalTransData.ulAfterEDBalance=NormalTransData.ulPrevEDBalance;
			//if(NormalTransData.ulAmount)
			//{
			SAV_ChangeExpSave();
			ulExpNum++;
			PRT_PrintTicket();	
			//}
			//else
			//{
			//	//Os__clr_display(255);
			//	//Os__GB2312_display(0,0,(uchar *)"ڳɹ");	
				//MSG_WaitKey(2);
			//	memset(aucBuf,' ',sizeof(aucBuf));
			//	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
			//	bcd_asc(&aucBuf[17],&NormalTransData.aucNewExpiredDate[1],6);
			//	Os__xprint(aucBuf);

			//}
		}
		if(ucResult == SUCCESS)
			ucResult = Trans_ChangeExp();
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
	}
	/**/
	return ucResult;
}


unsigned char Untouch_RefApl(void)
{
	unsigned char ucResult=SUCCESS,tCardNo[10];
	unsigned char aucBuf[30],ucStatus,aucBuf1[7];
	unsigned long ulAmount,ullong1,ullong2;
	unsigned char aucOutData[300];
 	unsigned char ucLen,ucJ,ucOffset,ucI,ucInput;
	int i;
	
	ucResult = SAV_SavedTransIndex(0);	
	if(ucResult!=SUCCESS)
		return ucResult;
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=CashierLogonProcess();
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"˻:");
	ucResult=UTIL_InputAmount(2, &NormalTransData.ulAmount,1, 999999999);
	if(ucResult!=SUCCESS)
		return ucResult;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"˿ϵ绰:");
        memset(aucBuf,0,sizeof(aucBuf));
        if(UTIL_Input(1,true,0,15,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
		memcpy(NormalTransData.aucBarCode,aucBuf,15);
        }
	if(ucResult==SUCCESS)
		ucResult=CFG_SetBatchNumber1();

	NormalTransData.ucPtCode=RunData.ucPtcode;
	memcpy(NormalTransData.aucCashierNo,DataSave0.ChangeParamData.aucCashierNo,6);
	ucResult=Trans_RefApl();
	if(ucResult!=SUCCESS)
		return ucResult;
	ucResult=SAV_RefAplSave();
	if(ucResult!=SUCCESS)
		return ucResult;
	PRT_PrintTicket();
	return ucResult;
}


unsigned char Untouch_VoidRefApl(void)
{
	unsigned char ucResult=SUCCESS,tCardNo[10];
	unsigned char aucBuf[30],ucStatus,aucBuf1[7];
	unsigned long ulAmount,ullong1,ullong2;
	unsigned char aucOutData[300];
 	unsigned char ucLen,ucJ,ucOffset,ucI,ucInput;
	int i;
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
	ucResult=CashierLogonProcess();
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	//Os__clr_display(255);
	memset(aucBuf,0,sizeof(aucBuf));
	//Os__GB2312_display(0, 0, "ˮ:");
	if( UTIL_Input(1,true,TRANS_TRACENUMLEN,TRANS_TRACENUMLEN,'N',
				aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	RunData.ulOldTraceNumber=asc_long(aucBuf,6);	
	ucResult = SAV_SavedTransIndex(1);
	if(ucResult!=SUCCESS)
		return ucResult;
	if(NormalTransData.ucTransType!=TRANS_S_VOIDREFAPL)
		return ERR_NOTPROC;
	//Os__clr_display(255);
	memset(aucBuf,0,sizeof(aucBuf));
	UTIL_Form_Montant(aucBuf,NORMALTRANS_ulAmount,2);
	//Os__GB2312_display(2,0,(unsigned char *)":");
	Os__display(2,5,aucBuf);
	//Os__GB2312_display(3,4,(unsigned char *)"ȷ");
	if(MSG_WaitKey(30)!=KEY_ENTER)
		return ERR_CANCEL;
	NormalTransData.ucPtCode=RunData.ucPtcode;
	ucResult=Trans_VoidRefApl();
	if(ucResult!=SUCCESS)
		return ucResult;
	ucResult=SAV_VoidRefAplSave();
	if(ucResult!=SUCCESS)
		return ucResult;
	PRT_PrintTicket();
	return ucResult;
}
unsigned char Untouch_PreAuthProcess(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1;
	
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(TRANS_S_PREAUTH,MSG_PREAUTH,Untouch_PreAuth,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PREAUTHVOID,MSG_PREAUTHVOID,Untouch_PreAuthVoid,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PREAUTHFINISH,MSG_PREAUTHFINISH,Untouch_PreAuthFinish,&flag2,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PREAUTHFINISHVOID,  MSG_PREAUTHFINISHVOID,	Untouch_PreAuthFinishVoid,&flag2,&ManagementMenu);	
	if(RunData.ucDxMobile)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[47])
				return ERR_CARDNOFUN;
		}
	}
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	
	return ucResult;
}
unsigned char Untouch_PayAdmin(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1;
	
	SELMENU ManagementMenu;
	if(RunData.ucExpire1Flag)
		return ERR_EXPIREDATE;
	memset(&ManagementMenu,0,sizeof(SELMENU));
	if(RunData.aucZDFunFlag[1]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1))
		flag1=0;
	if(RunData.aucZDFunFlag[13]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1))
		flag2=0;
	if(RunData.aucZDFunFlag[21]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0))
		flag3=0;
	UTIL_GetMenu_Value(TRANS_S_PAY,MSG_PAY,Untouch_Pay,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PAYNOBAR,MSG_PAYNOBAR,Untouch_PayNobar,&flag2,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PAYONLINE,MSG_PAYONLINE,Untouch_OnlinePay,&flag3,&ManagementMenu);
	if(flag1||flag2||flag3)
	{
		if(flag1&&!flag2&&!flag3)
		{
			NormalTransData.ucTransType=TRANS_S_PAY;
			ucResult = Untouch_Pay();
		}else
		if(!flag1&&flag2&&!flag3)
		{
			NormalTransData.ucTransType=TRANS_S_PAYNOBAR;
			ucResult = Untouch_PayNobar();
		}else
		if(!flag1&&!flag2&&flag3)
		{
			NormalTransData.ucTransType=TRANS_S_PAYONLINE;
			ucResult = Untouch_OnlinePay();
		}else
			ucResult = UTIL_DisplayMenu(&ManagementMenu);
		
	}
	else
		ucResult=ERR_NOFUNCTION;
	return ucResult;
}

unsigned char Untouch_MobileAdmin(void)
{
	unsigned char ucResult=SUCCESS,flag1=0,flag2=0;
	
	SELMENU ManagementMenu;
	if(RunData.ucExpire1Flag)
		return ERR_EXPIREDATE;
	memset(&ManagementMenu,0,sizeof(SELMENU));
	if(DataSave1.CardTable.ucCountType=='2'||RunData.aucFunFlag[2]==0)
		flag1=1;
	if(DataSave1.CardTable.ucCountType=='2'||RunData.aucFunFlag[2]==1)
		flag2=1;
	UTIL_GetMenu_Value(TRANS_S_MOBILECHARGE,MSG_MOBILECHARGE,Untouch_Mobile,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_MOBILECHARGEONLINE,MSG_MOBILECHARGEONLINE,Untouch_MobileOnline,&flag2,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,MSG_MOBILERESEND,Untouch_MobileResend,NULL,&ManagementMenu);
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	return ucResult;
}
unsigned char Untouch_CardAdmin(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1,flag6=1,flag7=1,flag8=1,flag9=1,aucZDFunFlag[64];
	int i,j;
	SELMENU ManagementMenu;
	if(ucResult==SUCCESS)
		ucResult=CFG_CheckFlag();
	if(ucResult!=SUCCESS)
		return ucResult;
	RunData.ucPtcode=NormalTransData.ucTransType-0xF0;
	for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
	{
		ucResult=XDATA_Read_ZoneCtrls_File(i);
		if(ucResult!=SUCCESS)
			return ucResult;
		if(RunData.ucPtcode!=DataSave1.ZoneCtrl.ucPtCode)
			continue;
		UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
		for(j=0;j<64;j++)
			RunData.aucZDFunFlag[j]|=aucZDFunFlag[j];
	}
	if(RunData.aucZDFunFlag[3]!=1)
		flag1=0;
	if(RunData.aucZDFunFlag[4]!=1)
		flag2=0;
	if(RunData.aucZDFunFlag[43]!=1)
		flag3=0;
	if(RunData.aucZDFunFlag[44]!=1)
		flag4=0;
	if(RunData.aucZDFunFlag[29]!=1)
		flag5=0;
	if(RunData.aucZDFunFlag[30]!=1)
		flag6=0;
	if(RunData.aucZDFunFlag[9]!=1)
		flag7=0;
	if(RunData.aucZDFunFlag[10]!=1)
		flag8=0;
	if(RunData.aucZDFunFlag[12]!=1)
		flag9=0;
	memset(&ManagementMenu,0,sizeof(SELMENU));
	UTIL_GetMenu_Value(TRANS_S_SALE,MSG_SALE,Untouch_Sale,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_SALEONLINE,MSG_SALEONLINE,Untouch_SaleOnline,&flag8,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_LOADONLINE,MSG_LOADONLINE,Untouch_LoadOnline,&flag2,&ManagementMenu);
	//UTIL_GetMenu_Value(TRANS_S_LOADONLINE,MSG_LOADBYCREDIT,Untouch_LoadByCredit,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_CASHLOAD,MSG_CASHLOAD,Untouch_LoadOnline,&flag6,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_CHANGECD,MSG_CHANGECD,Untouch_Changecard,&flag3,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_CHANGEEXP,MSG_CHANEEXP,Untouch_ChangeExp,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_BATCHLOAD,MSG_BATCHLOAD,Untouch_BatchLoad,&flag7,&ManagementMenu);
	
	UTIL_GetMenu_Value(TRANS_S_REFAPL,MSG_REFAPL,Untouch_RefApl,&flag5,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOIDREFAPL,MSG_VOIDREFAPL,Untouch_VoidRefApl,&flag5,&ManagementMenu);
	if(flag1==0&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0)
		return ERR_NOFUNCTION;
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	if(DataSave0.ChangeParamData.ucSwitchIPFlag==1)
	{
		Trans_SwitchInform();
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	
	return ucResult;
}
unsigned char Untouch_CardAdminSelectPt(void)
{
	unsigned char ucResult=SUCCESS,flag1=0,flag2=0,flag3=0,flag4=0,aucZDFunFlag[64],aucPt[10];
	int i,j;
	SELMENU ManagementMenu;
	if(ucResult==SUCCESS)
		ucResult=CFG_CheckFlag();
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucPt,0,sizeof(aucPt));
	for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
	{
		ucResult=XDATA_Read_ZoneCtrls_File(i);
		if(ucResult!=SUCCESS)
			return ucResult;
		UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
		if(DataSave1.ZoneCtrl.ucPtCode>0&&DataSave1.ZoneCtrl.ucPtCode<10)
		{
			if(aucZDFunFlag[9]||aucZDFunFlag[10]||aucZDFunFlag[3]||aucZDFunFlag[4]
				||aucZDFunFlag[29]||aucZDFunFlag[43]||aucZDFunFlag[44]||aucZDFunFlag[30])
				aucPt[DataSave1.ZoneCtrl.ucPtCode-1]=1;
		}
	}
	memset(&ManagementMenu,0,sizeof(SELMENU));
	UTIL_GetMenu_Value(0xF1,MSG_WKADMIN,Untouch_CardAdmin,&aucPt[0],&ManagementMenu);
	UTIL_GetMenu_Value(0xF2,MSG_SMTADMIN,Untouch_CardAdmin,&aucPt[1],&ManagementMenu);
	UTIL_GetMenu_Value(0xF3,MSG_HYADMIN,Untouch_CardAdmin,&aucPt[2],&ManagementMenu);
	
	if(!memcmp(aucPt,"\x00\x00\x00",3))
		return ERR_NOFUNCTION;
	if(!memcmp(aucPt,"\x01\x00\x00",3))
	{
		NormalTransData.ucTransType=0xF1;
		return Untouch_CardAdmin();
	}
	if(!memcmp(aucPt,"\x00\x01\x00",3))
	{
		NormalTransData.ucTransType=0xF2;
		return Untouch_CardAdmin();
	}
	if(!memcmp(aucPt,"\x00\x00\x01",3))
	{
		NormalTransData.ucTransType=0xF3;
		return Untouch_CardAdmin();
	}
	
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	
	return ucResult;
}
unsigned char Untouch_HandChoice(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1,flag6=1;
	
	SELMENU ManagementMenu;
	if(ucResult==SUCCESS)
		ucResult=CFG_CheckFlag();
	if(ucResult!=SUCCESS)
		return ucResult;
	NormalTransData.ucHandFlag=1;

	NormalTransData.ucPtCode=1;
	NormalTransData.ucNewOrOld=1;
	NormalTransData.ucCardType=CARD_CPU;
	memcpy(NormalTransData.aucZoneName1,(uchar *)"ɼͨ",10);
	memset(&ManagementMenu,0,sizeof(SELMENU));
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE,MSG_ONLINEPURCHASE,Untouch_OnlinePurchase,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID,MSG_VOID,Untouch_Void,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,MSG_SURFING,Untouch_DX_Process,NULL,&ManagementMenu);
/*
	UTIL_GetMenu_Value(TRANS_S_TRANSFER,MSG_TRANSFER,Untouch_Transfer,&flag2,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PREAUTHFINISH,MSG_PREAUTHFINISH,Untouch_PreAuthFinish,&flag3,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PREAUTHFINISHVOID,MSG_PREAUTHFINISHVOID,Untouch_PreAuthFinishVoid,&flag3,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PREAUTHVOID,MSG_PREAUTHVOID,Untouch_PreAuthVoid,&flag3,&ManagementMenu);
	
	if(flag1==0&&flag2==0&&flag3==0)*/
	if(flag1==0)
		return ERR_NOFUNCTION;
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	
	return ucResult;
}
unsigned char Untouch_Choice(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1,flag6=1,flag7=1;
	unsigned char flag8=1,flag9=1,flaga=1,flagb=1,flagc=1,flagd=1,flage=1;
	int i;
	SELMENU ManagementMenu;

	
	/*if(SleTransData.ucStatus==0x04)
	{
		if(RunData.aucZDFunFlag[3]==1)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"δ");
			//Os__GB2312_display(1,0,(uchar *)"[ȷ]ۿ");
			if(Os__xget_key()!=KEY_ENTER)
				return ERR_CANCEL;
			return Untouch_Sale();
		}
		else
			return ERR_CARDNOACT;
	}*/
	Uart_Printf("NormalTransData.ucZeroFlag=%02x\n",NormalTransData.ucZeroFlag);
	memset(&ManagementMenu,0,sizeof(SELMENU));
	if(RunData.aucZDFunFlag[27]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
		||DataSave1.CardTable.ucCountType=='3')
		flaga=0;
	if(RunData.aucZDFunFlag[23]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
		||DataSave1.CardTable.ucCountType=='3')
		flag2=0;
	if(SleTransData.ucStatus==CARD_RECYCLED&&(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]==0))		
	{
		if(flaga==0&&flag2==0)
			return ERR_RECYCLE;
		flag1=0;flag4=0;flag3=0;flag6=0;flagc=0;
		flag9=0;flagb=0;flag5=0;flag7=0;flag8=0;
	}
	else
	{
		if(NormalTransData.ucZeroFlag==1
			&&(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1))
		{
			if(flag2==0&&flaga==0)
			{
				return ERR_MUSTRETURN;
			}
			flag1=0;flag4=0;flag3=0;flag6=0;flagc=0;
			flag9=0;flagb=0;flag5=0;flag7=0;flag8=0;
		}
		else
		{
			if(RunData.aucZDFunFlag[0]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
				||DataSave1.CardTable.ucCountType=='3')
				flag1=0;
			if(RunData.aucZDFunFlag[20]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
				||DataSave1.CardTable.ucCountType=='3')
				flag4=0;
			if(RunData.aucZDFunFlag[40]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0))
				flag3=0;
			if(RunData.aucZDFunFlag[1]!=1&&RunData.aucZDFunFlag[21]!=1&&RunData.aucZDFunFlag[13]!=1
				||DataSave1.CardTable.ucCountType=='3')
				flag6=0;
			if(RunData.aucZDFunFlag[47]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
				||DataSave1.CardTable.ucCountType=='3')
				flag5=0;/**/
			if(RunData.aucZDFunFlag[26]!=1||DataSave1.CardTable.ucCountType=='3')
				flag7=0;
			if(RunData.aucZDFunFlag[34]!=1||DataSave1.CardTable.ucCountType!='3')
				flag8=0;
			if(RunData.aucZDFunFlag[35]!=1||DataSave1.CardTable.ucCountType!='3')
				flag9=0;
			if(RunData.aucZDFunFlag[28]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
				||DataSave1.CardTable.ucCountType=='3')
				flagb=0;
			if(RunData.aucZDFunFlag[8]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
				||DataSave1.CardTable.ucCountType=='3')
				flagc=0;
			if(RunData.aucZDFunFlag[41]!=1)
				flagd=0;
		}
	}
	if(SleTransData.ucStatus==0x04)	
	{
		if(NormalTransData.ucZeroFlag==1&&flag2)
		{
			flag1=0;flag4=0;flag3=0;flag6=0;flaga=0;
			flag9=0;flagb=0;flag5=0;flag7=0;flag8=0;flagc=0;

		}
		else
			return ERR_CARDNOACT;
	}
	UTIL_GetMenu_Value(TRANS_S_PURCHASE,MSG_PURCHASE,Untouch_Purchase,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PURCHASE_P,MSG_PURCHASE,Untouch_Purchase_P,&flag8,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE,MSG_ONLINEPURCHASE,Untouch_OnlinePurchase,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID,MSG_VOID,Untouch_Void,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE_P,MSG_ONLINEPURCHASE_P,Untouch_OnlinePurchase,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID_P,MSG_VOID_P,Untouch_Void,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,  MSG_PAYADMIN,Untouch_PayAdmin,&flag6,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PREAUTH,  MSG_PREAUTHPROCESS,	  Untouch_PreAuthProcess,&flag5,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PRELOAD,  MSG_PRELOAD,	  Untouch_Preload,&flag2,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_MOBILECHARGE,  MSG_MOBILEADMIN,	  Untouch_MobileAdmin,&flag7,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_TRANSFER,MSG_TRANSFER,Untouch_Transfer,&flagb,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_QUERY,  MSG_QUERY,	  Untouch_Query,&flag3,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_REFUND,  MSG_REFUND,	  Untouch_Refund,&flaga,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PTCPURCHASE,MSG_PTCPURCHASE,Untouch_PTCPurchase,&flagc,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,  MSG_CHANEPIN,	  Untouch_ChangePin,&flagd,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_TRANSFER,MSG_RETRANSFER,Untouch_Retransfer,&flagb,&ManagementMenu);
	//UTIL_GetMenu_Value(NULL,  MSG_DETAIL,	  Untouch_Detail,NULL,&ManagementMenu);	
/* 2012_TEST DEL
	if(flag1==0&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0&&flag9==0&&flaga==0&&flagb==0&&flagc==0&&flagd==0)
		return ERR_NOFUNCTION;
	if(flag1==1&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0&&flag9==0&&flaga==0&&flagb==0&&flagc==0&&flagd==0)
	{
		NormalTransData.ucTransType=TRANS_S_PURCHASE;
		ucResult=Untouch_Purchase();
		return ucResult;
	}
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
*/

//  2012_TEST  ADD
	NormalTransData.ucTransType=TRANS_S_PURCHASE;
	ucResult=Untouch_Purchase();
	
	return ucResult;
}

unsigned char Untouch_Menu(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20];
	unsigned int    uiTimesOut;
	unsigned int    uiI;

	Uart_Printf("\n 20120625  Untouch_Menu  .... Begin\n");
	//return 0;
	
	if(Untouch_CheckCardApp()!=SUCCESS)
		return ERR_NOTSUCHAPP;

	if(ucResult==SUCCESS)
		ucResult=CFG_CheckFlag();

	if(ucResult==SUCCESS)
		ucResult=Untouch_ReadCardInfo();
	//if(ucResult==ERR_NOTSUCHAPP)
	//	return ucResult;
	if(ucResult==SUCCESS)
		ucResult=CFG_SetBatchNumber();

	if(ucResult==SUCCESS)
		ucResult=CFG_CheckCardValid();

	if(ucResult==SUCCESS)
		ucResult=Untouch_CheckRedo();
	
	if(ucResult==SUCCESS)
	{
		if(RunData.ucMustReturn)
		{
			if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
				ucResult= ERR_MUSTRETURN;
		}
	}
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			if(SleTransData.ucStatus==CARD_RECYCLED)
				ucResult=ERR_CARDRECYCLE;
 			else if(RunData.ucMustReturn)
			{
				if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
					ucResult= ERR_MUSTRETURN;
			}
			else
			{
				memset(aucBuf,0,sizeof(aucBuf));
				UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
				//Os__clr_display(255);
                //Os__GB2312_display(0,0,(uchar *)"卡内余额:");
				Os__display(1,0,aucBuf);
                //Os__GB2312_display(3,0,(uchar *)"任意键继续");
				MSG_WaitKey(10);
				//Os__clr_display(255);
			}
		}else
		{
			if(RunData.ucExpireFlag)
			{
				if(NormalTransData.ulYaAmount!=0)
				{
					if(SleTransData.ucStatus==0x04)		
						ucResult	= ERR_CARDNOACT;
					if(SleTransData.ucStatus==CARD_RECYCLED)		
						ucResult	= ERR_CARDRECYCLE;
					if(NormalTransData.ucZeroFlag==1)
						ucResult = ERR_EXPIREDATE;
					if(ucResult==SUCCESS)
					{
						//Os__clr_display(255);
                        //Os__GB2312_display(0,0,(uchar*)"卡已过期");
						memset(aucBuf,0,sizeof(aucBuf));
                        memcpy(aucBuf,(uchar*)"余额:",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
						//Os__GB2312_display(1,0,aucBuf);
						memset(aucBuf,0,sizeof(aucBuf));
                        memcpy(aucBuf,(uchar*)"押金:",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulYaAmount,2);
						//Os__GB2312_display(2,0,aucBuf);
                        //Os__GB2312_display(3,0,(uchar*)"[确认]后一次扣除s");
						if(Os__xget_key()==KEY_ENTER)
						{
							NormalTransData.ucTransType=TRANS_S_EXPPURCHASE;
							ucResult=Untouch_ExpPurchase();
						}
                        //else
							//Os__clr_display(255);
					}
				}
				else
					ucResult=ERR_EXPIREDATE;
			}
			else
				ucResult=Untouch_Choice();
		}

	}
	if(ucResult>ERR_END)
		MSG_DisplayErrMsg(ucResult);

    /*
    2012_TEST
	while(1)
	{
		ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
		if(ucResult==0x04)
			return SUCCESS;
		//Os__clr_display(1);
        //Os__GB2312_display(1,0,"请取卡");
	}
	*/

	//Os__clr_display(1);
    //Os__GB2312_display(1,0,"请取卡");
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,"消费成功，请取卡",13);
    sleep(1);
#endif
	MIFARE_CloseRF();
	OSDRV_Abort(14);
	MSG_WaitKey(1);
	if(DataSave0.ChangeParamData.ucSwitchIPFlag==1)
	{
		Trans_SwitchInform();
	}
	return ucResult;
}
unsigned char Untouch_Zsh_Process(void)
{
	unsigned char ucResult = SUCCESS,uctPtCode=0,aucZDFunFlag[64];
	unsigned char ucStatus,ucLen,aucOutData[300],aucBuf[20],aucZSHCardNo[21];
	unsigned int    uiTimesOut;
	unsigned int    uiI,i,j;
	ucResult=CFG_CheckFlag();
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"ɹ");
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
	}
	if(ucResult==SUCCESS)
	{
		RunData.ucPtcode=1;
		for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
		{
			ucResult=XDATA_Read_ZoneCtrls_File(i);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(RunData.ucPtcode!=DataSave1.ZoneCtrl.ucPtCode)
				continue;
			UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
			for(j=0;j<64;j++)
				RunData.aucZDFunFlag[j]|=aucZDFunFlag[j];
		}
	}
	if(RunData.aucZDFunFlag[55]==1)
	{
		while(1)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"ż");
			ucResult = MIFARE_WaitCard();
			if(ucResult==SUCCESS||ucResult==ERR_CANCEL)
				break;
		}		
		if(ucResult==SUCCESS)
		{
			if(ucResult==SUCCESS)
				ucResult=Untouch_ReadCardInfo();

			if(ucResult==SUCCESS)
				ucResult=CFG_SetBatchNumber();

			if(ucResult==SUCCESS)
				ucResult=CFG_CheckCardValid();
		}
		if(ucResult==SUCCESS)
		{
			if(!RunData.aucZDFunFlag[55])
			{
				ucResult=ERR_CARDNOFUN;
			}
		}
		if(ucResult==SUCCESS)
			ucResult=Untouch_OnlineJsZshPurchase();
		if( ucResult != SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		while(1)
		{
			if(Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen)==0x04)
				break;
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
		if( ucResult != SUCCESS)	
			return SUCCESS;
	}
	else
	{
		if(ucResult==SUCCESS)
			ucResult = Trans_GetTrace();
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,"Ƿ˾");
			//Os__GB2312_display(1,0,"[1]");
			//Os__GB2312_display(2,0,"[2]");
			while(1)
			{
				ucStatus=Os__xget_key();
				if(ucStatus=='1'||ucStatus=='2')
					break;
				if(ucStatus==KEY_CLEAR)
					return ERR_CANCEL;
			}
			if(ucStatus=='1')
			{
				while(1)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(unsigned char *)"˾");
					ucResult = MIFARE_WaitCard();
					if(ucResult==SUCCESS||ucResult==ERR_CANCEL)
						break;
				}		
				if(ucResult==SUCCESS)
				{
					if(ucResult==SUCCESS)
						ucResult=Untouch_ReadCardInfo();

					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();

					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();
				}
				if(ucResult==SUCCESS)
				{
					if(!RunData.aucFunFlag[16])
					{
						ucResult=ERR_NOTDRIVERCARD;
					}
				}
				if(ucResult==SUCCESS)
				{
					memcpy(NormalTransData.aucOldCardNo,NormalTransData.aucUserCardNo,10);
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,"Ƿ˾ֵ");
					//Os__GB2312_display(1,0,"[1]");
					//Os__GB2312_display(2,0,"[2]");
					while(1)
					{
						ucStatus=Os__xget_key();
						if(ucStatus=='1'||ucStatus=='2')
							break;
						if(ucStatus==KEY_CLEAR)
							return ERR_CANCEL;
					}
					if(ucStatus=='1')
						ucResult=Untouch_OnlineZshPurchase();

				}
				if( ucResult != SUCCESS)			
				{
					MSG_DisplayErrMsg(ucResult);
				}
				while(1)
				{
					if(Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen)==0x04)
						break;
					//Os__clr_display(1);
					//Os__GB2312_display(1,0,"ȡ");
				}
				if( ucResult != SUCCESS)	
					return SUCCESS;
			}
		}
		if(ucResult==SUCCESS)
		{
			while(1)
			{
				NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
				//Os__clr_display(255);
				memset(aucBuf,0,sizeof(aucBuf));
				bcd_asc(aucZSHCardNo,NormalTransData.aucBarCode,20);
				memcpy(aucBuf,"ʯͿ:",13);
				memcpy(&aucBuf[13],&aucZSHCardNo[1],3);
				//Os__GB2312_display(0,0,aucBuf);
				memset(aucBuf,0,sizeof(aucBuf));
				memcpy(aucBuf,&aucZSHCardNo[4],16);
				Os__display(1,0,aucBuf);
				//Os__GB2312_display(2,0,"()ɼ¿");
				//Os__GB2312_display(3,0,"<ȡ>");
				ucResult=Untouch_WaitAllCard();
				if(ucResult==ERR_CANCEL)
					break;
				if(ucResult==SUCCESS)
				{
					if(NormalTransData.ucCardType==CARD_MAG)
					{
						ucResult=ERR_CARDNOFUN;
					}
					if(NormalTransData.ucCardType==CARD_MEM)
					{
						if(ucResult == SUCCESS )
							SLE4442_ATR(aucBuf);

						if(ucResult==SUCCESS)
							ucResult = SLE4442_ReadCardData();
						if(ucResult==SUCCESS)
							ucResult=CFG_CheckCardValid();
						if(ucResult==SUCCESS)
							CFG_SetBatchNumber();

					}
					if(NormalTransData.ucCardType==CARD_M1)
					{
						if(ucResult==SUCCESS)
							ucResult=Untouch_ReadCardInfo();

						if(ucResult==SUCCESS)
							ucResult=CFG_SetBatchNumber();

						if(ucResult==SUCCESS)
							ucResult=CFG_CheckCardValid();

						if(ucResult==SUCCESS)
							ucResult=Untouch_CheckRedo();

					}
					if(ucResult==SUCCESS)
					{
						if(uctPtCode)
						{
							if(uctPtCode!=NormalTransData.ucPtCode)
								ucResult = ERR_DIFFCARDTYPE;
						}
					}
					if(ucResult==SUCCESS)
						ucResult=Untouch_ZshPurchase();
					if(ucResult==SUCCESS)
					{
						uctPtCode=NormalTransData.ucPtCode;
					}
				}
				if( ucResult != SUCCESS)			
				{
					MSG_DisplayErrMsg(ucResult);
				}
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"γ");
				Os__xdelay(10);
				Os__ICC_remove();
				while(1)
				{
					ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
					if(ucResult==0x04)
					{
						ucResult= SUCCESS;
						break;
					}
					//Os__clr_display(1);
					//Os__GB2312_display(1,0,"ȡ");
				}
			}
		}
	}
	if(DataSave0.TransInfoData.TransTotal.uiZshPurchaseNb+DataSave0.TransInfoData.TransTotal.uiZshOnlinePurchaseNb+DataSave0.TransInfoData.TransTotal.uiJsZshOnlinePurchaseNb!=0)
	{		
		PRT_ZSH_Total();
		if(DataSave0.ConstantParamData.printtimes==2)
		{
			if(DataSave0.ConstantParamData.prnflag)
				Os__xlinefeed(5*LINENUM);
			else
			{
				Os__xlinefeed(5*LINENUM);
				Os__xlinefeed(5*LINENUM);
				Os__xlinefeed(5*LINENUM);
			}
			Os__GB2312_xprint((unsigned char *)"ֵ",FONT);
			NormalTransData.ucPtCode=uctPtCode;
			PRT_ZSH_PrintTicket2();
		}
		ucResult=STL_Settle();
		if(ucResult!=SUCCESS)
			Os__GB2312_xprint((unsigned char *)"ʧ",FONT);
	}
	return ucResult;
}
unsigned char Untouch_DX_WaitCard(void)
{
	unsigned char aucOutData[300];
 	unsigned char ucLen,ucrd=0;
 	unsigned char ucResult,ucStatus,aucCardSerial[4],aucCardSerial1[4];
	int i,times;
	ucResult=SUCCESS;
	if(!DataSave0.ConstantParamData.ucYTJFlag||RunData.ucGdflag)
	{
		if(DataSave0.RedoTrans.ucRedoFlag)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)"ϴν׽δ֪");
			//Os__GB2312_display(2,0,(uchar *)"һֻ");
			ucrd=1;
		}
		else
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"뽫ֻ");
			//Os__GB2312_display(1,0,(uchar *)"");
		}
		ucResult=SMART_Elitel("\x80\x1D\x90\xB0\x01\x00\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x03\x00\x00\x01\x00\x00\x00\x00\x00",31,aucOutData,&ucLen);
		if(ucResult==SUCCESS)
		{
			if(memcmp(&aucOutData[2],"\x90\x00",2))
			{
				ucResult=ERR_NOTPROC;
			}
		}
		if(ucResult==SUCCESS)
		{
			times=60;
			while(times)
			{
				memset(aucOutData,0,sizeof(aucOutData));
				ucResult=SMART_Elitel("\x80\x05\x90\xB0\x04\x00\x01\x00",8,aucOutData,&ucLen);
				if(ucResult==ERR_CANCEL)
				{
					return ERR_CANCEL;
				}
				if(!memcmp(&aucOutData[2],"\x9C\x02",2))
					break;
			}
		}
		if(ucResult == SUCCESS)
		{
			memset(aucOutData,0,sizeof(aucOutData));
			ucResult=SMART_Elitel("\x80\x05\x90\xB0\x05\x00\x00",7,aucOutData,&ucLen);
			if(memcmp(&aucOutData[66],"\x90\x00",2))
				ucResult=ERR_NOTPROC;
			memcpy(NormalTransData.aucCardSerial,&aucOutData[62],4);
		}
		
	}
	RunData.uiTimesOut=6000;
	Os__timer_start(&RunData.uiTimesOut);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"Ժ...");
	//Os__clr_display_pp(255);
	//Os__GB2312_display_pp(0,0,(uchar *)"Ժ...");
	if(ucResult == SUCCESS)
		ucResult=PBOC_ISOSelectFilebyFID(ICC1, "\x3F\x00");
	if(ucResult == SUCCESS)
		ucResult=PBOC_ISOSelectFilebyAID(ICC1, "SANDPAY",7);
	if(ucResult == SUCCESS)
	{
		ucResult=PBOC_ISOReadBinarybySFI(ICC1, 0x15, 0, 0);
	}
	if(ucResult == SUCCESS)
	{
		if(memcmp(Iso7816Out.aucData,"SANDPAY",7))
			return ERR_NOTSUCHAPP;
		memcpy(NormalTransData.aucUserCardNo,&Iso7816Out.aucData[10],8);
		memcpy(NormalTransData.aucExpiredDate,&Iso7816Out.aucData[24],4);
		//bcd_asc(NormalTransData.aucLoadTrace,&Iso7816Out.aucData[30],16);
		NormalTransData.ucPtCode=Iso7816Out.aucData[8];
		if(NormalTransData.ucPtCode<0x01||NormalTransData.ucPtCode>0x03)
			NormalTransData.ucPtCode=0x01;
		UTIL_BitFlagAnalyze(&Iso7816Out.aucData[38],2,RunData.aucBakBits);

	}
	if(ucResult == SUCCESS)
		ucResult=PBOC_ISOGetBalance(ICC1, 0x02);
	if(ucResult == SUCCESS)
	{
		NormalTransData.ulPrevEDBalance = tab_long(Iso7816Out.aucData,4);
		NormalTransData.ulTrueBalance = tab_long(Iso7816Out.aucData,4);
	}
	if(ucResult == SUCCESS)
	{
		ucResult=PBOC_ISOReadBinarybySFI(ICC1, 0x16, 0, 0);
	}
	if(ucResult == SUCCESS)
	{
		memcpy(NormalTransData.aucZoneCode,&Iso7816Out.aucData[1],3);
		SleTransData.ucStatus=Iso7816Out.aucData[0];
		//memcpy(NormalTransData.aucZoneCode,"\x00\x10\x01",3);
		memcpy(NormalTransData.aucUserName,&Iso7816Out.aucData[4],18);
		//memcpy(NormalTransData.aucUserIDNo,&Iso7816Out.aucData[22],18);
		//NormalTransData.ucIDType=Iso7816Out.aucData[40];
		NormalTransData.ucCardType=CARD_CPU;
		NormalTransData.ucNewOrOld=1;
	}
	if(ucResult==SUCCESS)
		ucResult=CFG_SetBatchNumber();
	Uart_Printf("===============RunData.uiTimesOut78=%d================\n",RunData.uiTimesOut);
	if(ucResult==SUCCESS)
		ucResult=CFG_CheckCardValid();
	Uart_Printf("===============RunData.uiTimesOut80=%d================\n",RunData.uiTimesOut);

	if(ucResult==SUCCESS)
	{
		ucResult=Untouch_CheckRedo();
		if(ucrd==1&&ucResult==0)
		{
			//Os__clr_display(255);
            //if(RunData.uckeyi)
				//Os__GB2312_display(2,0,(uchar *)"ϴν״");
        //	else
				//Os__GB2312_display(2,0,(uchar *)"ϴνδɹ");
			//Os__GB2312_display(3,0,(uchar *)"ν׼");
			MSG_WaitKey(2);
		}
			
	}
	return ucResult;
}
unsigned char Untouch_DX_Choice(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1,flag6=1,flag7=1;
	unsigned char flag8=1,flag9=1,flaga=1,flagb=1,flagc=1,flagd=1,flage=1;
	int i;
	SELMENU ManagementMenu;
	Uart_Printf("NormalTransData.ucZeroFlag=%02x\n",NormalTransData.ucZeroFlag);
	memset(&ManagementMenu,0,sizeof(SELMENU));
	if(RunData.aucZDFunFlag[27]!=1)
		flaga=0;
	if(RunData.aucZDFunFlag[23]!=1)
		flag2=0;
	if(RunData.aucZDFunFlag[0]!=1)
		flag1=0;
	if(RunData.aucZDFunFlag[20]!=1)
		flag4=0;
	if(RunData.aucZDFunFlag[40]!=1)
		flag3=0;
	if(RunData.aucZDFunFlag[1]!=1)
		flag6=0;
	if(RunData.aucZDFunFlag[47]!=1)
		flag5=0;
	if(RunData.aucZDFunFlag[26]!=1)
		flag7=0;
	if(RunData.aucZDFunFlag[34]!=1)
		flag8=0;
	if(RunData.aucZDFunFlag[35]!=1)
		flag9=0;
	if(RunData.aucZDFunFlag[28]!=1)
		flagb=0;
	if(RunData.aucZDFunFlag[41]!=1)
		flagc=0;
	UTIL_GetMenu_Value(TRANS_S_PURCHASE,MSG_PURCHASE,Untouch_Purchase,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PURCHASE_P,MSG_PURCHASE,Untouch_Purchase_P,&flag8,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE,MSG_ONLINEPURCHASE,Untouch_OnlinePurchase,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID,MSG_VOID,Untouch_Void,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE_P,MSG_ONLINEPURCHASE_P,Untouch_OnlinePurchase,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID_P,MSG_VOID_P,Untouch_Void,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,  MSG_PAYADMIN,Untouch_PayAdmin,&flag6,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PREAUTH,  MSG_PREAUTHPROCESS,	  Untouch_PreAuthProcess,&flag5,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PRELOAD,  MSG_PRELOAD,	  Untouch_Preload,&flag2,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_MOBILECHARGE,  MSG_MOBILEADMIN,	  Untouch_MobileAdmin,&flag7,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_TRANSFER,MSG_TRANSFER,Untouch_Transfer,&flagb,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_QUERY,  MSG_QUERY,	  Untouch_Query,&flag3,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_REFUND,  MSG_REFUND,	  Untouch_Refund,&flaga,&ManagementMenu);	
	UTIL_GetMenu_Value(MSG_NULL,  MSG_CHANEPIN,	  Untouch_ChangePin,&flagc,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_TRANSFER,MSG_RETRANSFER,Untouch_Retransfer,&flagb,&ManagementMenu);
	
	//UTIL_GetMenu_Value(NULL,  MSG_DETAIL,	  Untouch_Detail,NULL,&ManagementMenu);	
	if(flag1==0&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0&&flag9==0&&flaga==0&&flagb==0)
		return ERR_NOFUNCTION;
	if(flag1==1&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0&&flag9==0&&flaga==0&&flagb==0)
	{
		NormalTransData.ucTransType=TRANS_S_PURCHASE;
		ucResult=Untouch_Purchase();
		return ucResult;
	}
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	
	return ucResult;
}
unsigned char Untouch_DX_Process(void)
{
	unsigned char aucBuf[300],aucZDFunFlag[64],aucOutData[255];
 	unsigned char ucLen;
 	unsigned char ucResult,ucStatus;
	int i,j,times;
	RunData.ucDxMobile=1;
	NormalTransData.ucHandFlag=0;
	ucResult=CFG_CheckFlag();
	if(DataSave0.ConstantParamData.ucYTJFlag)
	{
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"뽫ֻ");
			//Os__GB2312_display(1,0,(uchar *)"");
			ucResult=SMART_Elitel("\x80\x1D\x90\xB0\x01\x00\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x03\x00\x00\x01\x00\x00\x00\x00\x00",31,aucOutData,&ucLen);
		}
		if(ucResult==SUCCESS)
		{
			if(memcmp(&aucOutData[2],"\x90\x00",2))
			{
				ucResult=ERR_NOTPROC;
			}
		}
		if(ucResult==SUCCESS)
		{
			times=60;
			while(times)
			{
				memset(aucOutData,0,sizeof(aucOutData));
				ucResult=SMART_Elitel("\x80\x05\x90\xB0\x04\x00\x01\x00",8,aucOutData,&ucLen);
				if(ucResult==ERR_CANCEL)
				{
					return ERR_CANCEL;
				}
				if(!memcmp(&aucOutData[2],"\x9C\x02",2))
					break;
			}
		}
		if(ucResult == SUCCESS)
		{
			memset(aucOutData,0,sizeof(aucOutData));
			ucResult=SMART_Elitel("\x80\x05\x90\xB0\x05\x00\x00",7,aucOutData,&ucLen);
			if(memcmp(&aucOutData[66],"\x90\x00",2))
				ucResult=ERR_NOTPROC;
		}
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)":");
			Os__display(2,0,aucBuf);
			//Os__GB2312_display(3,0,(uchar *)"˳");
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)":");
			Os__display_pp(1,0,aucBuf);
			MSG_WaitKey(5);
		}
		return ucResult;
	}
	if(DataSave0.RedoTrans.ucRedoFlag)
	{
		if(ucResult==SUCCESS)
			ucResult=Untouch_DX_WaitCard();

	}
	if(ucResult==SUCCESS)
	{
		for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
		{
			ucResult=XDATA_Read_ZoneCtrls_File(i);
			if(ucResult!=SUCCESS)
				return ucResult;
			bcd_asc(aucBuf,DataSave1.ZoneCtrl.aucCardZone,6);
			if(memcmp(aucBuf,"023",3))
				continue;
			UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
			for(j=0;j<64;j++)
				RunData.aucZDFunFlag[j]|=aucZDFunFlag[j];
		}
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Untouch_DX_Choice();
	}
	if(NormalTransData.ucTransType==TRANS_S_TRANSFER)
	{
		while(1)
		{
			ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
			if(ucResult==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,"ȡ");
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"γ");
		Os__xdelay(10);
		Os__ICC_remove();
		return ERR_CANCEL;
	}
	return ucResult;
}
