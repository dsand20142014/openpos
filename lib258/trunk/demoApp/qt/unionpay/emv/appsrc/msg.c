
#include	<msg.h>
#include	<EMV41.h>
#include	<global.h>
#include <display.h>
#include	<xdata.h>
#include	<new_drv.h>

extern const DATAPROPERTY TermDataProperty[];
extern UCHAR 	MagTrack2[40];
UCHAR	MSG_OpenCom(void)
{
	UCHAR	ucResult;
 
	COM_PARAM CommParam =
	{
		1,     	/* Stop bit */
		9600, 	/* Speed */
		1,	   	/* MODEM 0 RS232 1 HDLC 2 */
		8,     	/* Data bit */
	  'N'    	/* No parity */
	};
//	MSG_ComClose();
	if(ConstParam.ucCommID ==0)
		ucResult = Os__init_com(&CommParam);
	else
//		ucResult = Os__init_com3(0x0303, 0x1400, 0x14);
		ucResult = Os__init_com3(0x0303, 0x0c00, 0x0c);
	if(ucResult) ucResult =EMVERROR_OPENCOM;

	return ucResult;

}

UCHAR MSG_ComSendData(PUCHAR pucBuff, USHORT uiLen)
{
	USHORT 	uiI;
	UCHAR	ucHigh,ucLow,ucLRC;

	//Os__clr_display(255);
	//Os__GB2312_display(2,2,(PUCHAR)"Send Data...");
	Os__com_flush2();
//	Os__xdelay(50);
//	Os__xdelay(5);
	if(MSG_ComSendByte(CHAR_STX))
		return EMVERROR_SENDDATA;

	ucLRC = 0x00;
	ucLow = uiLen&0xFF;
	ucHigh = (uiLen>>8)&0xFF;

	ucLRC ^= ucHigh;
	if(MSG_ComSendByte(ucHigh))
		return EMVERROR_SENDDATA;


	ucLRC^=ucLow;
	if(MSG_ComSendByte(ucLow))
		return EMVERROR_SENDDATA;


	for(uiI=0;uiI<uiLen;uiI++)
	{
		ucLRC^=*(pucBuff+uiI);
		if(MSG_ComSendByte(*(pucBuff+uiI)))
			return EMVERROR_SENDDATA;

	}

	ucLRC ^= 0x03;

	if(MSG_ComSendByte(CHAR_ETX))
		return EMVERROR_SENDDATA;

	if(MSG_ComSendByte(ucLRC))
		return EMVERROR_SENDDATA;


	return EMVERROR_SUCCESS;
}
UCHAR MSG_ComSendByte(UCHAR	ucSendChar)
{
	if(ConstParam.ucCommID ==0)
		Os__txcar(ucSendChar);
	else
		Os__txcar3(ucSendChar);
//	Os__xdelay(1);
	return EMVERROR_SUCCESS;

}

UCHAR MSG_ComReceiveByte(PUCHAR pucByte,USHORT uiTimeout)
{
	USHORT uiResult;
#if 0
	uiResult = Os__rxcar(uiTimeout);
#else


	if(ConstParam.ucCommID ==0)
		uiResult = Os__rxcar(uiTimeout);
	else
		uiResult = Os__rxcar3(uiTimeout);
#endif

	switch	( uiResult / 256 )
	{
	case EMVERROR_SUCCESS :
		*pucByte = uiResult % 256;
//		util_Printf("%02X ",*pucByte);
		return(EMVERROR_SUCCESS);
	default:
//		util_Printf("\nConstParam.ucCommID %02x",ConstParam.ucCommID);
		return EMVERROR_RECEIVEDATA;

	}
}

UCHAR MSG_ComRecvData(PUCHAR pBuff, USHORT *puiLen)
{

	USHORT uiI,uiDataLen,uiMaxLen;
	UCHAR  ucTemp,ucLRC;
	//Os__clr_display(255);
	//Os__GB2312_display(2,2,(PUCHAR)"Receive Data...");

	uiMaxLen =*puiLen;
	for(uiI=0;uiI<500;uiI++)
	{
		if(!MSG_ComReceiveByte(pBuff,10))
			if(pBuff[0]==0x02) break;
	}
	if(uiI>=500)
		return EMVERROR_RECEIVEDATA;

	ucLRC = 0;
	for(uiI=0;uiI<2;uiI++)
	{
		if(MSG_ComReceiveByte(pBuff+uiI+1,500)!=0)
			return EMVERROR_RECEIVEDATA;

		ucLRC^=*(pBuff+uiI+1);
	}

	uiDataLen=(unsigned int)(*(pBuff+1))*256 + *(pBuff+2);
	if(uiDataLen>uiMaxLen) return EMVERROR_OVERFLOW;

	for(uiI=0;uiI<uiDataLen;uiI++)
	{
		if(MSG_ComReceiveByte(pBuff+uiI,500)!=0)
			return EMVERROR_RECEIVEDATA;

		ucLRC^=*(pBuff+uiI);
	}



	if(MSG_ComReceiveByte(&ucTemp,500)!=0)
		return EMVERROR_RECEIVEDATA;

	if(ucTemp!=0x03)
		return EMVERROR_RECEIVEDATA;
	ucLRC ^= ucTemp;

	if(MSG_ComReceiveByte(&ucTemp,500)!=0)
		return EMVERROR_RECEIVEDATA;

	if(ucLRC!=ucTemp)
		return EMVERROR_RECEIVEDATA;

	*puiLen = uiDataLen;

	return EMVERROR_SUCCESS;
}


UCHAR MSG_ComClose()
{
	if(ConstParam.ucCommID ==0)
		OSUART_Close1();
	else
		OSUART_Close2();
	return EMVERROR_SUCCESS;
}
unsigned char MSG_DisplayErrMsg(unsigned char ucErrorCode)
{
	unsigned char 	aucBuf[MSG_MAXCHAR+1];
	unsigned char 	aucDispBuf[MSG_MAXCHAR+1];
	unsigned char 	aucBufSub[MSG_MAXCHAR+1];
	unsigned char 	ucType;
	unsigned char 	aucERRCodeBuf[10];
	unsigned short 	uiIndex1;
	unsigned char 	aucErrorCode[4];
	unsigned char 	Flag;

	Flag = 0;
	util_Printf("\n UTIL_Beep2009-10-20 17:28.\n");
	util_Printf("MSG_DisplayErrMsg   ucErrorCode = %02x.\n",ucErrorCode);

	if((ucErrorCode == 0x36)
	||(ucErrorCode == 0x38)
	||(ucErrorCode == 0x39)
	||(ucErrorCode == 0x40)
	  )
	{
		MSG_DisplayErrMsgNotWait_Dail(ucErrorCode);
		return(SUCCESS);
	}

	UTIL_Beep();

	util_Printf("\n UTIL_Beep \n");
	
	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucERRCodeBuf,0,sizeof(aucERRCodeBuf));
	memset(aucDispBuf,0,sizeof(aucDispBuf));
	memset(aucBufSub,0,sizeof(aucBufSub));
	memset(aucErrorCode,' ',sizeof(aucErrorCode));

	util_Printf("ucErrorCode=%02x\n",ucErrorCode);

	if(ucErrorCode == ERR_HOSTCODE)
	{
		ucErrorCode = (unsigned char)G_RUNDATA_ucErrorExtCode;
		if(ucErrorCode == 93)
		{
#ifdef GUI_PROJECT
			memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
			ProUiFace.ProToUi.uiLines=4;	
			memcpy(ProUiFace.ProToUi.aucLine1,(uchar *)"    交易失败", strlen((char *)"    交易失败") );
			memcpy(ProUiFace.ProToUi.aucLine2,(uchar *)"该卡有可能是IC卡", strlen((char *)"该卡有可能是IC卡") );
			memcpy(ProUiFace.ProToUi.aucLine3,(uchar *)"如果该卡为IC卡，", strlen((char *)"如果该卡为IC卡，") );
			memcpy(ProUiFace.ProToUi.aucLine4,(uchar *)"请插卡做交易！", strlen((char *)"请插卡做交易！") );
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
			memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
			ProUiFace.ProToUi.uiLines=3;	
			memcpy(ProUiFace.ProToUi.aucLine1,(uchar *)"或者是交易非法", strlen((char *)"或者是交易非法") );
			memcpy(ProUiFace.ProToUi.aucLine2,(uchar *)"请联系发卡行", strlen((char *)"请联系发卡行") );
			memcpy(ProUiFace.ProToUi.aucLine3,(uchar *)"主机返回码：[93]", strlen((char *)"主机返回码：[93]") );				
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#else
			//Os__clr_display(255);
			//Os__GB2312_display(0, DISPCOLFIRST, (uchar *)"    交易失败");
			//Os__GB2312_display(1, DISPCOLFIRST, (uchar *)"该卡有可能是IC卡");
			//Os__GB2312_display(2, DISPCOLFIRST, (uchar *)"如果该卡为IC卡，");
			//Os__GB2312_display(3, DISPCOLFIRST, (uchar *)"请插卡做交易！");

			if(DataSave0.ConstantParamData.ucCashFlag == 0x31)//一体机模式
				MSG_WaitKey(3);
			else
				MSG_WaitKey(60);
			

			//Os__clr_display(255);
			//Os__GB2312_display(1, DISPCOLFIRST, (uchar *)"或者是交易非法");
			//Os__GB2312_display(2, DISPCOLFIRST, (uchar *)"请联系发卡行");
			//Os__GB2312_display(3, DISPCOLFIRST, (uchar *)"主机返回码：[93]");
			if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				MSG_WaitKey(3);
			else
				MSG_WaitKey(60);
#endif			
			return SUCCESS;
		}

#ifdef TEST
		util_Printf("\n ucErrorCode1=%02x.\n",ucErrorCode);
		util_Printf("G_RUNDATA_ucErrorExtCode=%d\n",G_RUNDATA_ucErrorExtCode);
#endif
		for(uiIndex1=0;HostErrMsg[uiIndex1].ucErrCode<255;uiIndex1++)
		{
			if( ucErrorCode == HostErrMsg[uiIndex1].ucErrCode)
			{
				ucType = HostErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				memcpy(aucBufSub,HostErrMsg[uiIndex1].aucErrMsgSub,MSG_MAXCHAR);
				break;
			}
		}
		if(HostErrMsg[uiIndex1].ucErrCode == 255)
		{
			ucType = HostErrMsg[uiIndex1].ucType;
			memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
		}
#ifdef TEST
		util_Printf("\n HostErrMsgIndex=%d\n",uiIndex1);
#endif

		if(G_RUNDATA_ucErrorExtCode == 100)
			memcpy(aucErrorCode ,"A0" ,2);
		else
			char_asc(aucErrorCode,2,&ucErrorCode);
	}
	else if (ucErrorCode == ERR_WRITEFILE)
        {
            ucErrorCode = (unsigned char)G_RUNDATA_ucErrorFileCode;
            for(uiIndex1=0;FileErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
            {
                if( ucErrorCode == FileErrMsg[uiIndex1].ucErrCode)
                {
                    ucType = FileErrMsg[uiIndex1].ucType;
                    memcpy(aucBuf,FileErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
                    break;
                }
            }
            if(FileErrMsg[uiIndex1].ucErrCode == 0xff)
            {
                ucType = FileErrMsg[uiIndex1].ucType;
                memcpy(aucBuf,FileErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
            }
            hex_asc(aucErrorCode,&ucErrorCode,2);
            Flag = 1;
        }
	else
	{
		for(uiIndex1=0;ErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
		{
			if( ucErrorCode == ErrMsg[uiIndex1].ucErrCode)
			{
				ucType = ErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,ErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				break;
			}
		}
		if(ErrMsg[uiIndex1].ucErrCode == 0xff)
		{
			ucType = ErrMsg[uiIndex1].ucType;
			memcpy(aucBuf,ErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
		}
		hex_asc(aucErrorCode,&ucErrorCode,2);
		Flag = 1;
	}
   //Os__clr_display(255);
   memcpy(aucERRCodeBuf,"返回码:",7);

    switch( ucType)
    {
        case MSG_TYPEZHCN:
            if(Flag == 0)
            {
                memcpy(&aucERRCodeBuf[7],aucErrorCode,2);
                memcpy(&aucDispBuf[0]," " ,1);
                memcpy(&aucDispBuf[1],aucBuf ,strlen((char*)aucBuf));
            }else
            {
                memcpy(&aucERRCodeBuf[7],aucErrorCode,2);
                //memcpy(&aucDispBuf[0]," " ,1);
                memcpy(&aucDispBuf[0],aucBuf , strlen((char*)aucBuf));
            }
#ifdef GUI_PROJECT
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=3;
	memcpy(ProUiFace.ProToUi.aucLine1,aucDispBuf, strlen((char *)aucDispBuf) );
	memcpy(ProUiFace.ProToUi.aucLine2,aucBufSub, strlen((char *)aucBufSub) );
	memcpy(ProUiFace.ProToUi.aucLine3,aucERRCodeBuf, strlen((char *)aucERRCodeBuf) );
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#else
            //Os__GB2312_display(1, DISPCOLFIRST, (uchar *)aucDispBuf);
            //Os__GB2312_display(2, DISPCOLFIRST, (uchar *)aucBufSub);
            //Os__GB2312_display(3, DISPCOLFIRST, (uchar *)aucERRCodeBuf);
#endif			
            break;
        default:
#ifdef GUI_PROJECT			
		memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
		ProUiFace.ProToUi.uiLines=1;
		memcpy(ProUiFace.ProToUi.aucLine1,aucBuf, strlen((char *)aucBuf) );
#else
		Os__display(0,DISPCOLFIRST,aucBuf);
#endif
            break;
    }
#ifdef GUI_PROJECT			
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#endif
	 if(DataSave0.ConstantParamData.ucCashFlag == 0x31
        || ucErrorCode==ERR_NOTRETURN)
            MSG_WaitKey(3);
	 else
		MSG_WaitKey(30);
	 
   	return(SUCCESS);
}

unsigned char MSG_DisplayErrMsgNotWait_Dail(unsigned char ucErrorCode)
{
	unsigned char aucBuf[MSG_MAXCHAR+1];
	unsigned char aucBufSub[MSG_MAXCHAR+1];
	unsigned char ucType;
	unsigned short uiIndex1;

	if(ucErrorCode == 0x3A)
	{
		return SUCCESS;
	}
	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucBufSub,0,sizeof(aucBufSub));
	if(ucErrorCode == ERR_HOSTCODE)
	{
		ucErrorCode = (unsigned char)G_RUNDATA_ucErrorExtCode;
		for(uiIndex1=0;uiIndex1<MSG_MAXHOSTMSG;uiIndex1++)
		{
			if( ucErrorCode == HostErrMsg[uiIndex1].ucErrCode)
			{
				ucType = HostErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				memcpy(aucBufSub,HostErrMsg[uiIndex1].aucErrMsgSub,MSG_MAXCHAR);
				break;
			}
		}

	}else
	{
		for(uiIndex1=0;uiIndex1<MSG_MAXERRMSG;uiIndex1++)
		{
			if( ucErrorCode == ErrMsg[uiIndex1].ucErrCode)
			{
				ucType = ErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,ErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				break;
			}
		}
	}
    switch( ucType)
    {
    	case MSG_TYPEZHCN:	
#ifdef GUI_PROJECT			
		memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
		memcpy(ProUiFace.ProToUi.aucLine1,"通迅失败",8);
		if(strlen((char*)aucBuf) !=0 ){
		ProUiFace.ProToUi.uiLines=3;	
		memcpy(ProUiFace.ProToUi.aucLine2,(uchar *)(aucBuf),strlen((char*)aucBuf));
		memcpy(ProUiFace.ProToUi.aucLine3,(uchar *)(aucBufSub),strlen((char*)aucBuf));
		}
		else{ 	
		ProUiFace.ProToUi.uiLines=1;	
		}
#else
		//Os__clr_display(255);
		//Os__GB2312_display(1, DISPCOLFIRST, (uchar *)"    通迅失败");
		if( strlen((char*)aucBuf)!=0 )
		{
			//Os__GB2312_display(2, DISPCOLFIRST, (uchar *)(aucBuf));
			//Os__GB2312_display(3, DISPCOLFIRST, (uchar *)(aucBufSub));
		}
#endif		
	    	break;
	default:
#ifdef GUI_PROJECT			
		memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
		ProUiFace.ProToUi.uiLines=1;	
		memcpy(ProUiFace.ProToUi.aucLine1,aucBuf,strlen((char*)aucBuf));	
#else
		//Os__clr_display(255);
		Os__display(0,DISPCOLFIRST,aucBuf);
#endif		
	    	break;
    }
#ifdef GUI_PROJECT			
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#endif		
	UTIL_WaitGetKey(4);
   	return(ERR_CANCEL);
}
/*******************************************************************************************/
unsigned char MSG_WaitKey(unsigned short uiTimeout)
{
	DRV_OUT 		*pdKey;
	unsigned int  uiSecond;
	unsigned char	ucKey;

	NEW_DRV_TYPE  new_drv;
	uchar ucResult=0;

	return KEY_ENTER;
	
	util_Printf("\n1. MSG_WaitKey Os_Wait_Event begin.uiTimeout:%d",uiTimeout);
	
	ucResult = Os_Wait_Event(KEY_DRV , &new_drv, 1000*uiTimeout);
	
	util_Printf("\n2. Os_Wait_Event ucResult:%02x",ucResult);
	//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
	if((ucResult == 0)&&(uiTimeout != 0))
	{                                                                         
		if(new_drv.drv_type == KEY_DRV)                                     
		{                                                                   
			if(new_drv.drv_data.gen_status== DRV_ENDED){
				pdKey = (DRV_OUT *)& new_drv.drv_data;
				if( pdKey->gen_status == DRV_ENDED)
			    {
			    	ucKey = pdKey->xxdata[1];
			    }

			}
		}                                                                   				                                                   
		else if(new_drv.drv_type == DRV_TIMEOUT)                            
		{    
		    	ucKey = 0;
		}	        
		util_Printf("\n OWE_NewDrvtts End.....\n");
	}else
	{

	}
/*			
    if( uiTimeout != 0 )
    {
	    uiSecond = uiTimeout*100;
		Os__timer_start(&uiSecond);
		pdKey = Os__get_key();
		do{
		}while(  (uiSecond !=0)
			   &&(pdKey->gen_status == DRV_RUNNING)
			  );
	    Os__timer_stop(&uiSecond);
	    if( uiSecond == 0)
	    {
	    	ucKey = ERR_APP_TIMEOUT;
	    	Os__abort_drv(drv_mmi);
	    }
	    if( pdKey->gen_status == DRV_ENDED)
	    {
	    	ucKey = pdKey->xxdata[1];
	    }
     }else
    {
		pdKey = Os__get_key();
		do{
		}while(pdKey->gen_status == DRV_RUNNING);
		if( pdKey->gen_status == DRV_ENDED)
		{
			ucKey = pdKey->xxdata[1];
		}
    }
   Os__abort_drv(drv_mmi);
   */
   util_Printf("\nucKey=%02x\n",ucKey);
    return(ucKey);
}

unsigned char MSG_DisplyTransType(unsigned char ucInTransType ,unsigned char Line ,unsigned char Col ,unsigned char ucClrFlag ,unsigned char ucClrLine)
{

	if(ucClrFlag)	//Os__clr_display(ucClrLine);
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;	

	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_PURCHASE:
			//Os__GB2312_display(0, 2, (uchar * )" (消费)");
   	memcpy(ProUiFace.ProToUi.aucLine1," (消费)",strlen((char*)" (消费)"));
   			break;
   		case TRANS_MOBILEAUTH:
			OSMMI_GB2312Display(0x31|0x80,0,0,(unsigned char *)" (手机预约消费) ");
   	memcpy(ProUiFace.ProToUi.aucLine1," (手机预约消费) ",strlen((char*)" (手机预约消费) "));
   			break;
   		case TRANS_VOIDMOBILEAUTH:
			OSMMI_GB2312Display(0x31|0x80,0,0,(unsigned char *)" (手机预约撤销) ");
	memcpy(ProUiFace.ProToUi.aucLine1," (手机预约撤销) ",strlen((char*)" (手机预约撤销) "));
			break;
		case TRANS_MOBILEREFUND:
			OSMMI_GB2312Display(0x31|0x80,0,0,(unsigned char *)" (手机预约退货) ");
	memcpy(ProUiFace.ProToUi.aucLine1," (手机预约退货) ",strlen((char*)" (手机预约退货) "));
			break;
		case TRANS_CREDITSALE:
			OSMMI_GB2312Display(0x31|0x80,0,0,(unsigned char *)" (分期付款消费) ");
	memcpy(ProUiFace.ProToUi.aucLine1," (分期付款消费) ",strlen((char*)" (分期付款消费) "));
			break;
		case TRANS_CREDITSREFUND:
			OSMMI_GB2312Display(0x31|0x80,0,0,(unsigned char *)" (分期付款退货) ");
	memcpy(ProUiFace.ProToUi.aucLine1," (分期付款退货) ",strlen((char*)" (分期付款退货) "));
			break;
		case TRANS_CREDITSVOID:
			OSMMI_GB2312Display(0x31|0x80,0,0,(unsigned char *)" (分期付款撤消) ");
	memcpy(ProUiFace.ProToUi.aucLine1," (分期付款撤消) ",strlen((char*)" (分期付款撤消) "));
			break;
		case TRANS_VOIDPURCHASE:
			//Os__GB2312_display(0, 1, (uchar * )" (消费撤销)");
	memcpy(ProUiFace.ProToUi.aucLine1," (消费撤销)",strlen((char*)" (消费撤销)"));
			break;
		case TRANS_PREAUTH:
			//Os__GB2312_display(0, 2, (uchar * )"(预授权)");
   	memcpy(ProUiFace.ProToUi.aucLine1,"(预授权)",strlen((char*)"(预授权)"));
   			break;
   		case TRANS_VOIDPREAUTH:
			//Os__GB2312_display(0, 1, (uchar * )"(预授权撤销)");
	memcpy(ProUiFace.ProToUi.aucLine1,"(预授权撤销)",strlen((char*)"(预授权撤销)"));
			break;
		case TRANS_VOIDPREAUTHFINISH:
			//Os__GB2312_display(0, 0, (uchar * )"(预授权完成撤销)");
	memcpy(ProUiFace.ProToUi.aucLine1,"(预授权完成撤销)",strlen((char*)"(预授权完成撤销)"));
			break;
		case TRANS_PREAUTHFINISH:
			//Os__GB2312_display(0, 0, (uchar * )"(预授权完成联机)");
	memcpy(ProUiFace.ProToUi.aucLine1,"(预授权完成联机)",strlen((char*)"(预授权完成联机)"));
			break;
		case TRANS_REFUND:
			//Os__GB2312_display(0, 2, (uchar * )" (退货)");
	memcpy(ProUiFace.ProToUi.aucLine1," (退货)",strlen((char*)" (退货)"));
			break;
		case TRANS_TIPADJUST:
		case TRANS_TIPADJUSTOK:
			//Os__GB2312_display(0, 1, (uchar * )" (离线调整)");
	memcpy(ProUiFace.ProToUi.aucLine1," (离线调整)",strlen((char*)" (离线调整)"));
			break;
		case TRANS_OFFPREAUTHFINISH:
			//Os__GB2312_display(0, 1, (uchar * )" (离线结算)");
	memcpy(ProUiFace.ProToUi.aucLine1," (离线结算)",strlen((char*)" (离线结算)"));
			break;
		case TRANS_UNDOOFF:
			//Os__GB2312_display(0, 1, (uchar * )" (离线撤销)");
	memcpy(ProUiFace.ProToUi.aucLine1," (离线撤销)",strlen((char*)" (离线撤销)"));
			break;
		case TRANS_PREAUTHSETTLE:
			//Os__GB2312_display(0, 0, (uchar * )"(预授权完成离线)");
	memcpy(ProUiFace.ProToUi.aucLine1,"(预授权完成离线)",strlen((char*)"(预授权完成离线)"));
			break;
		case TRANS_OFFPURCHASE:
			//Os__GB2312_display(0, 1, (uchar * )" (脱机消费)");
	memcpy(ProUiFace.ProToUi.aucLine1," (脱机消费)",strlen((char*)" (脱机消费)"));
			break;
		case TRANS_VOIDOFFPURCHASE:
			//Os__GB2312_display(0, 0, (uchar * )" (脱机消费撤销)");
	memcpy(ProUiFace.ProToUi.aucLine1," (脱机消费撤销)",strlen((char*)" (脱机消费撤销)"));
			break;
		case TRANS_QUERYBAL:
			//Os__GB2312_display(0, 1, (uchar * )" (余额查询)");
memcpy(ProUiFace.ProToUi.aucLine1," (余额查询)",strlen((char*)" (余额查询)"));
			break;
		case TRANS_PREAUTHADD:
			//Os__GB2312_display(0, 1, (uchar * )"(追加预授权)");
memcpy(ProUiFace.ProToUi.aucLine1,"(追加预授权)",strlen((char*)"(追加预授权)"));
			break;
		case TRANS_DEPOSIT:
			//Os__GB2312_display(0, 2, (uchar * )" (圈存)");
memcpy(ProUiFace.ProToUi.aucLine1," (圈存)",strlen((char*)" (圈存)"));
			break;
		case TRANS_CUPMOBILE:
			//Os__GB2312_display(0, 0, (uchar * )" (移动支付消费)");
memcpy(ProUiFace.ProToUi.aucLine1," (移动支付消费)",strlen((char*)" (移动支付消费)"));
			break;
		case TRANS_VOIDCUPMOBILE:
			//Os__GB2312_display(0, 0, (uchar * )" (移动支付撤销)");
memcpy(ProUiFace.ProToUi.aucLine1," (移动支付撤销)",strlen((char*)" (移动支付撤销)"));
			break;
		case TRANS_EC_ASSIGNLOAD:
			//Os__GB2312_display(0, 0, (uchar * )" (指定账户圈存)");
memcpy(ProUiFace.ProToUi.aucLine1," (指定账户圈存)",strlen((char*)" (指定账户圈存)"));
			break;
		case TRANS_EC_UNASSIGNLOAD:
			//Os__GB2312_display(0, 0, (uchar * )"(非指定账户圈存)");
memcpy(ProUiFace.ProToUi.aucLine1,"(非指定账户圈存)",strlen((char*)"(非指定账户圈存)"));
			break;
		case TRANS_EC_CASHLOAD:
			//Os__GB2312_display(0, 0, (uchar * )" (现金充值)");
memcpy(ProUiFace.ProToUi.aucLine1," (现金充值)",strlen((char*)" (现金充值)"));
			break;
		default:
			//Os__GB2312_display(0, 0, (uchar * )" ");
memcpy(ProUiFace.ProToUi.aucLine1," ",strlen((char*)" "));
			break;
	}
	
	if(g_ThreadFlag)
	{
		 sem_post(&binSem1);
    sem_wait(&binSem2);		
	}else
	{
   sleep(1);
	}
	return SUCCESS;
}

unsigned char MSG_GetMsg(unsigned short uiIndex,unsigned char *pucMsg,unsigned short uiMsgLen)
{
	unsigned short uiLen;
	unsigned char aucIndex[4];

   	uiLen = min(uiMsgLen,MSG_MAXCHAR);
   	memset(aucIndex,0,sizeof(aucIndex));
   	long_asc(aucIndex,3,(unsigned long *)&uiIndex);
    if( uiIndex < MSG_MAXNB)
    {
//    	memcpy(pucMsg,&(DataSave0.MSGData.aucMSGTab[uiIndex][0]),uiLen);
    	memcpy(pucMsg,&MSGTAB[uiIndex].aucMSGTab,uiLen);
    	if( !(*pucMsg) )
    	{
    		if( uiMsgLen > strlen((char *)MSG_DEFAULTMSGEN)+strlen((char *)aucIndex))
    		{
	    		strcpy((char *)pucMsg,(char *)aucIndex);
	    		strcat((char *)pucMsg,(char *)MSG_DEFAULTMSGEN);
	    	}
		    return(ERR_MSG_INVALIDVALUE);
    	}else
    	{
    		return(SUCCESS);
    	}
    }else
    {
		if( uiMsgLen > strlen((char *)MSG_INVALIDMSGEN)+strlen((char *)aucIndex))
		{
    		strcpy((char *)pucMsg,(char *)aucIndex);
    		strcat((char *)pucMsg,(char *)MSG_INVALIDMSGEN);
    	}
	    return(ERR_MSG_INVALIDINDEX);
	}
}

/*******************************************************************************************/
unsigned char MSG_DisplayMsgKey(unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned short uiIndex,
					unsigned short uiTimeout)
{
	unsigned char	aucMsg[MSG_MAXCHAR+1];

 	UTIL_Beep();

    if( ucClrDisp )
    {
        //Os__clr_display(255);
    }

	memset(aucMsg,0,sizeof(aucMsg));
	MSG_GetMsg(uiIndex,aucMsg,sizeof(aucMsg));
#ifdef GUI_PROJECT
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;	
	memcpy(ProUiFace.ProToUi.aucLine1,aucMsg, strlen((char *)aucMsg) );
	
	if(g_ThreadFlag == 0)
		sleep(1);
	else{
	    sem_post(&binSem1);
	    sem_wait(&binSem2);
	  }
#else
//    switch( DataSave0.MSGData.aucMSGType[uiIndex])
    switch( MSGTAB[uiIndex].aucMSGType)
    {
    case MSG_TYPEZHCN:
    	//Os__GB2312_display(ucLine, ucCol/2, (uchar *)(aucMsg));
//	return(OK);

    	break;
    default:
    	Os__display(ucLine,ucCol,aucMsg);
    	break;
    }
#endif	
    return(MSG_WaitKey(uiTimeout));
}


/*******************************************************************************************/
void MSG_DisplayMsg( unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned int uiIndex)
{
      unsigned char aucMsg[MSG_MAXCHAR+1];

      Os__light_on();

      if( ucClrDisp )
      {
            //Os__clr_display(255);
/*         //Os__clr_display(ucLine);*/
       }
       memset(aucMsg,0,sizeof(aucMsg));
//    if(ucClrDisp)
	MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,ucLine,ucCol,ucClrDisp,255);
       MSG_GetMsg(uiIndex,aucMsg,sizeof(aucMsg));
#ifdef GUI_PROJECT
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;
	memcpy(ProUiFace.ProToUi.aucLine1,aucMsg, strlen((char *)aucMsg) );
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);

#else
//    switch( DataSave0.MSGData.aucMSGType[uiIndex])
    switch( MSGTAB[uiIndex].aucMSGType)
    {
	    case MSG_TYPEZHCN:
//		if(ucClrDisp)
	    		//Os__GB2312_display(ucLine+1, ucCol/2, (uchar *)(aucMsg));
//		else
//			//Os__GB2312_display(ucLine, ucCol/2, (uchar *)(aucMsg));
	    	break;
	    default:
//		if(ucClrDisp)
	    		Os__display(ucLine+1,ucCol,aucMsg);
//		else
//			Os__display(ucLine,ucCol,aucMsg);
	    	break;
    }
#endif	
}


void MSG_DisplayMenuMsg( unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned int uiIndex)
{
	unsigned char aucMsg[MSG_MAXCHAR+1];

	if( ucClrDisp )
	{
		//Os__clr_display(255);
	}
	memset(aucMsg,0,sizeof(aucMsg));
	MSG_GetMsg(uiIndex,aucMsg,sizeof(aucMsg));
#ifdef GUI_PROJECT	
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;
	memcpy(ProUiFace.ProToUi.aucLine1,aucMsg, strlen((char *)aucMsg) );
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#else
	switch( MSGTAB[uiIndex].aucMSGType)
	{
		case MSG_TYPEZHCN:
			//Os__GB2312_display(ucLine, ucCol/2, (uchar *)(aucMsg));
			break;
		default:
			Os__display(ucLine,ucCol,aucMsg);
			break;
	}
#endif	
}


/**************************************************************************************/
unsigned char MSG_DisplayNetworkErrMsg(unsigned char ucErrorCode)
{
	unsigned char 	aucBuf[MSG_MAXCHAR+1];
	unsigned char 	ucType;
	unsigned short 	uiIndex1;
	unsigned char 	aucErrorCode[3];

	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));
	UTIL_Beep();
	for(uiIndex1=0;NetErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
	{
		if( ucErrorCode == NetErrMsg[uiIndex1].ucErrCode)
		{
			ucType = NetErrMsg[uiIndex1].ucType;
			memcpy(aucBuf,NetErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
			break;
		}
	}
	if(NetErrMsg[uiIndex1].ucErrCode == 0xff)
	{
		ucType = NetErrMsg[uiIndex1].ucType;
		memcpy(aucBuf,NetErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
	}

	hex_str(aucErrorCode,&ucErrorCode,2);
	aucErrorCode[2] = 0x00;
	//Os__clr_display(255);
	switch( ucType)
	{
		case MSG_TYPEZHCN:
#ifdef GUI_PROJECT		
			//--		
			memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
			ProUiFace.ProToUi.uiLines=3;
			memcpy(ProUiFace.ProToUi.aucLine1,aucBuf, strlen((char *)aucBuf) );	
			memcpy(ProUiFace.ProToUi.aucLine2,"返回码：", strlen((char *)"返回码：") );
			memcpy(ProUiFace.ProToUi.aucLine3,aucErrorCode, strlen((char *)aucErrorCode) );
//sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#else
			//Os__GB2312_display(0, 0, (uchar *)(aucBuf));
			//Os__GB2312_display(1,0,(uchar *)"返回码：");
			Os__display(1,9,(uchar *)aucErrorCode);
#endif
			break;
		default:
#ifndef GUI_PROJECT		
			Os__display(0,0,aucBuf);
#endif
			
			break;
	}
	MSG_WaitKey(2);
	return(SUCCESS);
}

unsigned char MSG_GetErrMsg(unsigned char ucErrorCode,unsigned char *aucBuf)
{
	unsigned char 	ucType;
	unsigned short 	uiIndex1;


	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));
#ifdef TEST
		util_Printf("ucErrorCode=%02x\n",ucErrorCode);
#endif
	if(ucErrorCode==ERR_HOSTCODE)
	{
		ucErrorCode = (unsigned char)RunData.ucErrorExtCode;

		for(uiIndex1=0;HostErrMsg[uiIndex1].ucErrCode<255;uiIndex1++)
		{
			if( ucErrorCode == HostErrMsg[uiIndex1].ucErrCode/*(HostErrMsg[uiIndex1].ucErrCode/10*16+HostErrMsg[uiIndex1].ucErrCode%10)*/)
			{
				ucType = HostErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,18);
				break;
			}
		}
		if(HostErrMsg[uiIndex1].ucErrCode == 255)
		{
			ucType = HostErrMsg[uiIndex1].ucType;
			memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,18);
		}

		util_Printf("HostErrMsgIndex=%d\n",uiIndex1);

	}
	else
	{

#ifdef TEST
		util_Printf("ErrMsg\n");
#endif

		for(uiIndex1=0;ErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
		{
			if( ucErrorCode == ErrMsg[uiIndex1].ucErrCode)
			{
				ucType = ErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,ErrMsg[uiIndex1].aucErrMsg,18);
				break;
			}
		}
		if(ErrMsg[uiIndex1].ucErrCode == 0xff)
		{
			ucType =ErrMsg[uiIndex1].ucType;
			memcpy(aucBuf,ErrMsg[uiIndex1].aucErrMsg,18);
		}
		util_Printf("ErrMsgIndex=%d\n",uiIndex1);
	}
   	return(SUCCESS);
}
