/*
	SAND China
--------------------------------------------------------------------------
	FILE  comms.c								(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file of communicate function.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>
#include <xdata.h>
#include <oslib.h>
#include <emvinterface.h>
#include <semaphore.h>
#ifdef READUNTOUCHCARD
#include <m_include.h>
#include <m_global.h>
#include <m_xdata.h>
#endif

//#define tCOMMPRINT

//#define COMM_COM3
unsigned char aucEncryptData[1025];

#define MBFE_BUF "\x00\x00\x00\x00\x53\x44\x30\x32\x30\x00"
#define MBFE_LBUF "\x00\x00\x00\x00\x53\x44\x30\x31\x30\x00\x00\x00"

#if PS300
void COMMS_Pre_CheckBase( void)
{

	if( Os__CheckBase()==SUCCESS )
		G_RUNDATA_ucPreDialFalg =  false;
	else
		G_RUNDATA_ucPreDialFalg = true;
}

unsigned char COMMS_CheckBase( void)
{

	unsigned char ucFlag;
	unsigned char ucResult;

	ucFlag=1;
	while(1)
	{
		if( Os__CheckBase()==SUCCESS )
		{
			return  SUCCESS;
		}
		if( ucFlag==1 )
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"请放回座机！");
			ucFlag = 0;
		}
	}
}
#endif
unsigned char KEY_COMMS_PreComm(void)
{
        unsigned char ucResult = SUCCESS;
        unsigned char ucSetApnResult;
        unsigned char ucI;
        unsigned char ucSBoardLen,aucBuf[30];


	COM_PARAM CommParam =
	{
		1,				/* Stop bit */
		1200,		/* Speed */
		2,				/* MODEM 0 RS232 1 HDLC 2 */
		8,				/* Data bit */
	   'N'				/* NO parity */
	};
        switch( DataSave0.ConstantParamData.ucKEYCOMM )
        {
            case PARAM_DOWNKEYCOMM_GPRS:
            case PARAM_DOWNKEYCOMM_HDLC:
            case PARAM_DOWNKEYCOMM_TCPIP:
                break;
            default:
                //Os__clr_display(255);
                //Os__GB2312_display(1,2,(UCHAR*)" 请检查");
                //Os__GB2312_display(2,0,(UCHAR*)" 未设置通讯方式");
                UTIL_WaitGetKey(5);
                return (ERR_CANCEL);
        }
	util_Printf("\nKEY-COMMS_PreComm");
	G_RUNDATA_ucHostConnectFlag	= false;

#if PS300
	COMMS_Pre_CheckBase();
	util_Printf("\nG_RUNDATA_ucPreDialFalg = %02x\n",G_RUNDATA_ucPreDialFalg);
#endif

	switch( DataSave0.ConstantParamData.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
			util_Printf("\n通讯参数:");
			for(ucI=0;ucI<10;ucI++)
			{
				Os__set_modem_parameter(ucI,*(&DataSave0.ConstantParamData.ModemParam.dp+ucI));
				util_Printf("[%02x] ",*(&DataSave0.ConstantParamData.ModemParam.dp+ucI));
			}
			util_Printf("\n");
			break;
		default:
			break;
	}

	switch( DataSave0.ConstantParamData.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
	#ifdef 	COMM_COM3
			switch(DataSave0.ConstantParamData.uiCommSpeed)
			{
				case 19200:
					ucParam3 = 0x11;
					break;
				case 38400:
					ucParam3 = 0x12;
					break;
				case 57600:
					ucParam3 = 0x13;
					break;
				default:	//9600
					ucParam3 = 0x0C;
					break;

			}
			uiParam1 = 0x0303;
			uiParam2 = (ucParam3<<8)&0xFF00;
		//	if(Os__init_com3(uiParam1,uiParam2,ucParam3) != OK)
			if(Os__init_com3(0x0303,0x0C00,0x0C) != OK)
			{
				return(ERR_OSFUNC);
			}
	#else
			if(Os__init_com(&CommParam) != OK)
			{
				return(ERR_OSFUNC);
			}
	#endif
			break;
		default:
			break;
	}
	memset(aucBuf,0,sizeof(aucBuf));

	memcpy(aucBuf,DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN);
	ucSBoardLen=strlen((char *)aucBuf);
	memcpy(&DialParam.phone_nr[0],aucBuf,PARAM_SWITCHLEN);

	util_Printf("\n正在预拨号phone_nr %s",DialParam.phone_nr);
	util_Printf("\nDataSave0.ConstantParamData.aucHostTel1 %s",DataSave0.ConstantParamData.aucHostTel1);
	util_Printf("重试:%02x\n",G_RUNDATA_ucDialRetry);

	switch( DataSave0.ConstantParamData.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
			DialParam.prefix[0] = 0;
			DialParam.auto_man = MANUAL_CALL;
			switch( G_RUNDATA_ucDialRetry%2 )
			{
				case 0:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTel1,
						sizeof(DataSave0.ConstantParamData.aucHostTel1));
					break;
				case 1:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTel2,
						sizeof(DataSave0.ConstantParamData.aucHostTel2));
					break;
				default:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTel1,
						sizeof(DataSave0.ConstantParamData.aucHostTel1));
					break;
			}

			util_Printf("\nDialParam.phone_nr=%s\n",DialParam.phone_nr);

#if PS300
			if (G_RUNDATA_ucPreDialFalg ==  false)
				G_RUNDATA_ucDialRetry ++;
#else
				G_RUNDATA_ucDialRetry ++;
#endif
			if ( Os__backgnd_dial(&DialParam) != STILL_DIALING )
			{
				return(ERR_OSFUNC);
			}
			break;

		case PARAM_DOWNKEYCOMM_GPRS:
			ucSetApnResult = OSGSM_GprsSetAPN(DataSave0.ConstantParamData.aucKEYAPN);
			util_Printf("===[COMSS PreComm]==[OSGSM_GprsSeAPN=%02x]=====\n",ucSetApnResult);
		case PARAM_COMMMODE_CDMA:
			ucResult = OSGSM_GprsDial();
			if( ucResult != SUCCESS )
  			{
  				MSG_DisplayNetworkErrMsg(ucResult);
  			}
			util_Printf("\nOSGSM_GprsDial %02x",ucResult);
			break;
		default:
			break;
	}
	return(SUCCESS);
}

unsigned char COMMS_PreComm(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucSetApnResult;//Kuaidc Add For Test GPRS_SetAPN;
	unsigned char ucI;
	unsigned char ucSBoardLen,aucBuf[30];

	COM_PARAM CommParam =
	{
		1,     /* Stop bit */
		1200,  /* Speed */
		2,	   /* MODEM 0 RS232 1 HDLC 2 */
		8,     /* Data bit */
	   'N'     /* NO parity */
	};

	util_Printf("\nCOMMS_PreComm");
	G_RUNDATA_ucHostConnectFlag	= false;
#if PS300
	COMMS_Pre_CheckBase();
	util_Printf("\nG_RUNDATA_ucPreDialFalg = %02x\n",G_RUNDATA_ucPreDialFalg);
#endif

	switch( DataSave0.ConstantParamData.ucCommMode )
	{
		case PARAM_COMMMODE_HDLC:
			for(ucI=0;ucI<10;ucI++)
			{
				Os__set_modem_parameter(ucI,*(&DataSave0.ConstantParamData.ModemParam.dp+ucI));
				util_Printf("\n通讯参数=%02x\n",*(&DataSave0.ConstantParamData.ModemParam.dp+ucI));
			}
			break;
		default:
			break;
	}

	switch( DataSave0.ConstantParamData.ucCommMode )
	{
		case PARAM_COMMMODE_MODEM:
		case PARAM_COMMMODE_HDLC:
	#ifdef 	COMM_COM3
			switch(DataSave0.ConstantParamData.uiCommSpeed)
			{
				case 19200:
					ucParam3 = 0x11;
					break;
				case 38400:
					ucParam3 = 0x12;
					break;
				case 57600:
					ucParam3 = 0x13;
					break;
				default:	//9600
					ucParam3 = 0x0C;
					break;

			}
			uiParam1 = 0x0303;
			uiParam2 = (ucParam3<<8)&0xFF00;
		//	if(Os__init_com3(uiParam1,uiParam2,ucParam3) != OK)
			if(Os__init_com3(0x0303,0x0C00,0x0C) != OK)
			{
				return(ERR_OSFUNC);
			}
	#else
			if(Os__init_com(&CommParam) != OK)
			{
				return(ERR_OSFUNC);
			}
	#endif
			break;
		default:
			break;
	}
	memset(aucBuf,0,sizeof(aucBuf));

	memcpy(aucBuf,DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN);
	ucSBoardLen=strlen((char *)aucBuf);
	memcpy(&DialParam.phone_nr[0],aucBuf,PARAM_SWITCHLEN);

	if (DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_MODEM
		||DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_HDLC)
	{
		util_Printf("\n正在预拨号phone_nr %s",DialParam.phone_nr);
		util_Printf("\nDataSave0.ConstantParamData.aucHostTelNumber1 %s",DataSave0.ConstantParamData.aucHostTelNumber1);
		util_Printf("重试:%02x\n",G_RUNDATA_ucDialRetry);
	}
	else
	{
		util_Printf("\n++++++++++正在网络预拨号+++++++++\n");
	}
	switch( DataSave0.ConstantParamData.ucCommMode )
	{
		case PARAM_COMMMODE_MODEM:
		case PARAM_COMMMODE_HDLC:
			DialParam.prefix[0] = 0;
			DialParam.auto_man = MANUAL_CALL;
			switch( G_RUNDATA_ucDialRetry%3 )
			{
				case 0:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTelNumber1,
						sizeof(DataSave0.ConstantParamData.aucHostTelNumber1));
					break;
				case 1:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTelNumber2,
						sizeof(DataSave0.ConstantParamData.aucHostTelNumber2));
					break;
				case 2:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTelNumber3,
						sizeof(DataSave0.ConstantParamData.aucHostTelNumber3));
					break;
				default:
					memcpy(&DialParam.phone_nr[ucSBoardLen],DataSave0.ConstantParamData.aucHostTelNumber1,
						sizeof(DataSave0.ConstantParamData.aucHostTelNumber1));
					break;
			}

			util_Printf("\nDialParam.phone_nr=%s\n",DialParam.phone_nr);

#if PS300
			if (G_RUNDATA_ucPreDialFalg ==  false)
				G_RUNDATA_ucDialRetry ++;
#else
				G_RUNDATA_ucDialRetry ++;
#endif
			if ( Os__backgnd_dial(&DialParam) != STILL_DIALING )
			{
				return(ERR_OSFUNC);
			}
			break;

		case PARAM_COMMMODE_GPRS:
			if (G_NORMALTRANS_ucTransType ==TRANS_SENDCOLLECTDATA)
			{
				ucSetApnResult = OSGSM_GprsSetAPN(DataSave0.ConstantParamData.aucSendAPN);
				util_Printf("===[COMSS PreComm]=数据采集=[OSGSM_GprsSeAPN=%02x]=====\n",ucSetApnResult);
			}
			else
			{
				ucSetApnResult = OSGSM_GprsSetAPN(DataSave0.ConstantParamData.aucAPN);
				util_Printf("===[COMSS PreComm]==[OSGSM_GprsSeAPN=%02x]=====\n",ucSetApnResult);
			}
		case PARAM_COMMMODE_CDMA:
			ucResult = OSGSM_GprsDial();
			util_Printf("==[COMSS PreComm]=++OSGSM_GprsDial:[%02x]\n",ucResult);
			if( ucResult != SUCCESS )
  			{
  				MSG_DisplayNetworkErrMsg(ucResult);
  				ucResult = ERR_CANCEL;
  			}
			util_Printf("\nOSGSM_GprsDial %02x",ucResult);
			break;
		default:
			break;
	}
	return(SUCCESS);
}

unsigned char COMMS_CheckPreComm(void)
{
	DRV_OUT *pdKey;
	unsigned char ucResult,ucDialDispFlag;
	unsigned int uiTimeout;
	unsigned char ucI;

	ucI = 0;
	util_Printf("DataSave0.ConstantParamData.ucCommMode = %02x\n",DataSave0.ConstantParamData.ucCommMode);
	if(DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_GPRS
		||DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_CDMA
		||DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_TCPIP)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "正在连接网络...");
		//Os__GB2312_display(2, 0, (uchar *) "    请稍等...");
	}else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "正在拨号...");
		//Os__GB2312_display(3, 0, (uchar *) "按<取消键>退出");
	}

	util_Printf("\nCOMMS_CheckPreComm--11----\n");
	util_Printf("DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid---121--- = %02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
	util_Printf("G_RUNDATA_ucPreDialFalg=%02x,ucCommMode:%02x\n",G_RUNDATA_ucPreDialFalg,DataSave0.ConstantParamData.ucCommMode);
	switch( DataSave0.ConstantParamData.ucCommMode )
	{
		case PARAM_COMMMODE_MODEM:
		case PARAM_COMMMODE_HDLC:
			Os__abort_drv(drv_mmi);
			pdKey = Os__get_key();
			#if PS300
			if(G_RUNDATA_ucPreDialFalg)
			{
				do
				{
					COMMS_CheckBase();
					ucResult = COMMS_PreComm();
				}while(ucResult);
			}
			#endif
			for(;;)
			{
				ucResult = SUCCESS ;
				SEL_IncDialTotalCount();//累计拔号次数
				uiTimeout = DataSave0.ConstantParamData.uiDialWaitTime*100;
				ucDialDispFlag = true;
				Os__timer_start(&uiTimeout );
				do
				{
					#if PS300
						COMMS_CheckBase();
					#endif
					if(DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_GPRS
						||DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_CDMA
						||DataSave0.ConstantParamData.ucCommMode ==PARAM_COMMMODE_TCPIP)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "正在连接网络...");
						//Os__GB2312_display(2, 0, (uchar *) "    请稍等...");
					}else
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "正在拨号...");
						//Os__GB2312_display(3, 0, (uchar *) "按<取消键>退出");
					}

					ucResult = Os__check_backgnd_dial(CFG_CHECKDIALTIMEOUT);

					util_Printf("\nOs__check_backgnd_dial=%02x,%d\n",ucResult,uiTimeout);

					if( (ucResult == DIAL_OK)
						||(ucResult == DIAL_CONNECT)
					  )
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "已连接收单中心");
						//Os__GB2312_display(1, 0, (uchar *) "处理中...");
						//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
						Os__abort_drv(drv_mmi);
						break;
					}else if(ucResult == STILL_DIALING
						&&uiTimeout==0)
					{
						pdKey->xxdata[1] = KEY_ENTER;
						break;
					}
					else
					{
#if PS300
						if(ucResult != STILL_DIALING)
						{
							if(ucResult == DIAL_NO_TONE)
							{
								MSG_DisplayErrMsgNotWait_Dail(ucResult);
								break;
							}
							else
							{
								pdKey = Os__get_key();
								Os__xdelay(1);
								COMMS_FinComm();
								//Os__clr_display(255);
								//Os__GB2312_display(0, 0, (uchar *) "正在重拨号");
								//Os__GB2312_display(1, 0, (uchar *) "请稍等...");
								//Os__GB2312_display(3, 0, (uchar *) "按<取消键>退出");
								ucResult = COMMS_PreComm();
								continue;
							}
						}
#else
						if(ucResult != STILL_DIALING)
						{
							MSG_DisplayErrMsgNotWait_Dail(ucResult);
							break;
						}
#endif
					}
				}while(  (pdKey->gen_status == DRV_RUNNING)
									&&( ucResult == STILL_DIALING)
				    					#if PS100|PS400
					     				&&(uiTimeout)
					     				#endif
				 					);
				Os__timer_stop(&uiTimeout);
				if(pdKey->gen_status == DRV_ENDED)
				{
					if(pdKey->xxdata[1] == KEY_CLEAR)
					{
						return(ERR_CANCEL);
					}else
					if(pdKey->xxdata[1] == KEY_ENTER)
					{
						pdKey = Os__get_key();
						Os__xdelay(1);
						COMMS_FinComm();
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "正在重拨号");
						//Os__GB2312_display(1, 0, (uchar *) "请稍等...");
						//Os__GB2312_display(3, 0, (uchar *) "按<取消>键退出");
						ucResult = COMMS_PreComm();
						continue;
					}else
					{
						pdKey = Os__get_key();
						continue;
					}
				}
				if( ucResult == DIAL_CONNECT)
				{
					G_RUNDATA_ucHostConnectFlag = true;
					Os__abort_drv(drv_mmi);
					return(SUCCESS);
				}
				/* Retry Dial */
				COMMS_FinComm();
				if( ucI < DataSave0.ConstantParamData.ucDialRetry)
				{
					do{
						ucResult = COMMS_PreComm();
						ucI ++;
						if( ucResult == SUCCESS )
						{
							break;
						}else
						{
							COMMS_FinComm();
						}
					}while( ucI < DataSave0.ConstantParamData.ucDialRetry );
				}else
				{
					Os__abort_drv(drv_mmi);
					return(ERR_DIAL);
				}
				if( ucResult == SUCCESS )
				{
					continue;
				}else
				{
					Os__abort_drv(drv_mmi);
					return(ERR_DIAL);
				}
			}
		case PARAM_COMMMODE_GPRS:
		case PARAM_COMMMODE_CDMA:
			ucResult = OSGSM_GprsCheckDial(60*CFG_CHECKDIALTIMEOUT);
			util_Printf("\nOSGSM_GprsCheckDial %02x",ucResult);
			return ucResult;
			break;
#if PS300
		case PARAM_COMMMODE_TCPIP:
			COMMS_CheckBase();
			break;
#endif
		default:
			return(SUCCESS);
	}
}

unsigned char COMMS_TransOnline(void)
{
    /* restore iso8583data from backup */

	ISO8583_RestoreISO8583Data((unsigned char *)&ISO8583Data,
					(unsigned char *)&DataSave0.Trans_8583Data.SaveISO8583Data);

	return(COMMS_TransSendReceive());
}

unsigned char COMMS_TransSendReceive(void)
{
	unsigned char aucMAC[8],ucReFile;
	unsigned short uiLen;
	unsigned char ucResult;
	unsigned short SendMsgID,RecvMsgID;

    /* Save package to SendISO8583Data */
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
						(unsigned char *)&DataSave0.Trans_8583Data.SendISO8583Data);

	SendMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
	ucResult = ISO8583_PackData(ISO8583Data.aucCommBuf,&ISO8583Data.uiCommBufLen,
					ISO8583_MAXCOMMBUFLEN);
    /* Set MAC */
    if(  (ISO8583_CheckBit(64) == SUCCESS)
       ||(ISO8583_CheckBit(128) == SUCCESS)
      )
    {
		uiLen = ISO8583Data.uiCommBufLen-8;
		ucResult = UTIL_CalcGoldenCardSHMAC_Single(ISO8583Data.aucCommBuf,uiLen,aucMAC);
		memcpy( &ISO8583Data.aucCommBuf[uiLen],aucMAC,8);
	}

    /* SendReceive with host */
	uiLen = sizeof(ISO8583Data.aucCommBuf);

	util_Printf("\n-----------------SEND DATA--------------------------------\n");
	
	ISO8583_DumpData();

	ucResult = COMMS_SendReceive(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen,
					ISO8583Data.aucCommBuf,&uiLen);
	util_Printf("TransSendReceive-ucResult=%02x\n",ucResult);
	if( ucResult != SUCCESS)
	{
		Os__saved_copy((unsigned char *)"98",(unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucRevealEvent),2);
		ucReFile = XDATA_Write_Vaild_File(DataSaveTrans8583);
		if (ucReFile != SUCCESS)
		{
			MSG_DisplayErrMsg(ucReFile);
		}
		memset(&ISO8583Data,0x00,sizeof(ISO8583));
		return(ucResult);
	}

	ISO8583Data.uiCommBufLen = uiLen;
	ucResult =  ISO8583_UnpackData(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen);

	util_Printf("\n----------------RECV DATA--------------------------------\n");
	ISO8583_DumpData();

	util_Printf("[00001]------COMMS_TransSendReceiv--ucResult = %02x \n",ucResult);		
	if( ucResult != SUCCESS)
	{
		return(ucResult);
	}
	RecvMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
	if( RecvMsgID != SendMsgID + 10)
	{
		return(ERR_UNKNOWNTRANTYPE);
	}
	ucResult = ISO8583_CheckResponseValid();
	util_Printf("检查流水号:%ld\n",G_NORMALTRANS_ulTraceNumber);
	if( ucResult != SUCCESS)
	{
		/* If have response return, cancel reversal */
		util_Printf("------002-----ISO8583_CheckResponseValid----ucResult = %02x \n",ucResult);
		return(ucResult);
	}
    if(	 (ISO8583_CheckBit(64) == SUCCESS)
       ||(ISO8583_CheckBit(128) == SUCCESS)
      )
    {
			util_Printf("To be check MAC OK/KO\n");
	    	if(G_RUNDATA_ucTransType == TRANS_SETTLE)
			{
				return (SUCCESS);
			}
	    uiLen = ISO8583Data.uiCommBufLen-8;
		ucResult = UTIL_CalcGoldenCardSHMAC_Single(ISO8583Data.aucCommBuf,uiLen,aucMAC);
#if 1
		{
			unsigned char ucI;
			util_Printf("Check POs Load Mac!!!\n");
			for(ucI=0;ucI<8;ucI++)
				util_Printf("%02x, ",ISO8583Data.aucCommBuf[uiLen+ucI]);
			util_Printf("\n");
			for(ucI=0;ucI<8;ucI++)
				util_Printf("%02x, ",aucMAC[ucI]);
			util_Printf("\n");

		}
#endif
	    if( memcmp( &ISO8583Data.aucCommBuf[uiLen],aucMAC,8) )
	    {
			Os__saved_copy((unsigned char *)"A0",
					(unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucRevealEvent),2);
			XDATA_Write_Vaild_File(DataSaveTrans8583);
	    		return(ERR_ISO8583_MACERROR);
	    }
	}

	return(SUCCESS);

}

unsigned char COMMS_GenSendReceive(void)
{
	unsigned char aucMAC[8];
	unsigned short uiLen;
	unsigned char ucResult;
	unsigned short SendMsgID,RecvMsgID;

    /* Save package to SendISO8583Data */
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
						(unsigned char *)&DataSave0.Trans_8583Data.SendISO8583Data);
	util_Printf("\nSendISO8583Data11 len %d",DataSave0.Trans_8583Data.SendISO8583Data.Bit[55-1].uiLen);
	SendMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
#ifdef TEST
	util_Printf("SendMsgID=%d\n",SendMsgID);
#endif

	ucResult = ISO8583_PackData(ISO8583Data.aucCommBuf,&ISO8583Data.uiCommBufLen,
					ISO8583_MAXCOMMBUFLEN);
	if(ucResult!= SUCCESS)
	{
		return ucResult;
	}
    /* Set MAC */
    if(  (ISO8583_CheckBit(64) == SUCCESS)
       ||(ISO8583_CheckBit(128) == SUCCESS)
      )
    {
	    uiLen = ISO8583Data.uiCommBufLen-8;
		ucResult = UTIL_CalcGoldenCardSHMAC_Single(ISO8583Data.aucCommBuf,uiLen,
									aucMAC);
	    /* Copy MAC to commbuf */
	    memcpy( &ISO8583Data.aucCommBuf[uiLen],aucMAC,8);
	}

    /* SendReceive with host */
	uiLen = sizeof(ISO8583Data.aucCommBuf);
//	UTIL_PrintHexMsg(&ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen);

	ISO8583_DumpData();
	ucResult = COMMS_SendReceive(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen,
					ISO8583Data.aucCommBuf,&uiLen);
	if( ucResult != SUCCESS)
	{
		return(ucResult);
	}
	ISO8583Data.uiCommBufLen = uiLen;
	ucResult = ISO8583_UnpackData(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen);
	
	util_Printf("receive ===================\n");

	util_Printf("ISO8583Data.uiCommBufLen=%d\n",uiLen);
	for(uiLen=0;uiLen<51;uiLen++)
		util_Printf("%02x ",ISO8583Data.aucCommBuf[uiLen]);
	if(!( uiLen%10))
		util_Printf("\n");
	if( ucResult != SUCCESS)
	{
		return(ucResult);
	}
	RecvMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
#ifdef TEST
util_Printf("RecvMsgID=%d\n",RecvMsgID);
#endif

	if( RecvMsgID != SendMsgID + 10)
	{
		return(ERR_UNKNOWNTRANTYPE);
	}

	ISO8583_DumpData();
	/* Check response data valid */

	ucResult = ISO8583_CheckResponseValid();
	if( ucResult != SUCCESS)
	{
		return(ucResult);
	}

	return(SUCCESS);
}
unsigned char KEY_COMMS_CheckPreComm(void)
{
	DRV_OUT *pdKey;
	unsigned char ucResult,ucDialDispFlag;
	unsigned int uiTimeout;
	unsigned char ucI;

	ucI = 0;
	util_Printf("DataSave0.ConstantParamData.ucKEYCOMM = %02x\n",DataSave0.ConstantParamData.ucKEYCOMM);
	if(DataSave0.ConstantParamData.ucKEYCOMM ==PARAM_DOWNKEYCOMM_GPRS
		||DataSave0.ConstantParamData.ucKEYCOMM ==PARAM_DOWNKEYCOMM_TCPIP)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "正在连接网络...");
		//Os__GB2312_display(2, 0, (uchar *) "    请稍等...");
	}else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "正在拨号...");
		//Os__GB2312_display(3, 0, (uchar *) "按<取消键>退出");
	}

	util_Printf("\nKEY_COMMS_CheckPreComm--11----\n");
	util_Printf("G_RUNDATA_ucPreDialFalg=%02x\n",G_RUNDATA_ucPreDialFalg);
	switch( DataSave0.ConstantParamData.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
			Os__abort_drv(drv_mmi);
			pdKey = Os__get_key();
#if PS300
			if(G_RUNDATA_ucPreDialFalg)
			{
				do
				{
					COMMS_CheckBase();
					ucResult = KEY_COMMS_PreComm();
				}while(ucResult);
			}
#endif
			for(;;)
			{
				ucResult = SUCCESS ;
				SEL_IncDialTotalCount();//累计拔号次数
				uiTimeout = DataSave0.ConstantParamData.uiDialWaitTime*100;
				ucDialDispFlag = true;
				Os__timer_start(&uiTimeout );
				do
				{
					#if PS300
					COMMS_CheckBase();
					#endif
					if(DataSave0.ConstantParamData.ucKEYCOMM ==PARAM_DOWNKEYCOMM_TCPIP
						||DataSave0.ConstantParamData.ucKEYCOMM ==PARAM_DOWNKEYCOMM_GPRS)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "正在连接网络...");
						//Os__GB2312_display(2, 0, (uchar *) "    请稍等...");
					}else
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "正在拨号...");
						//Os__GB2312_display(3, 0, (uchar *) "按<取消键>退出");
					}

					ucResult = Os__check_backgnd_dial(CFG_CHECKDIALTIMEOUT);

					util_Printf("\nOs__check_backgnd_dial=%02x,%d\n",ucResult,uiTimeout);

					if( (ucResult == DIAL_OK)
						||(ucResult == DIAL_CONNECT)
					  )
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
						//Os__GB2312_display(1, 0, (uchar *) "处理中...");
						//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
						Os__abort_drv(drv_mmi);
						break;
					}else if(ucResult == STILL_DIALING
						&&uiTimeout==0)
					{
						pdKey->xxdata[1] = KEY_ENTER;
						break;
					}
					else
					{
#if PS300
						if(ucResult != STILL_DIALING)
						{
							if(ucResult == DIAL_NO_TONE)
							{
								MSG_DisplayErrMsgNotWait_Dail(ucResult);
								break;
							}
							else
							{
								pdKey = Os__get_key();
								KEY_COMMS_FinComm();
								//Os__clr_display(255);
								//Os__GB2312_display(0, 0, (uchar *) "正在重试拨号");
								//Os__GB2312_display(1, 0, (uchar *) "请稍等...");
								//Os__GB2312_display(3, 0, (uchar *) "按<取消键>退出");
								ucResult = ABC_COMMS_PreComm();
								continue;
							}
						}
#else
						if(ucResult != STILL_DIALING)
						{
							MSG_DisplayErrMsgNotWait_Dail(ucResult);
							break;
						}
#endif
					}
				}while(  (pdKey->gen_status == DRV_RUNNING)
									&&( ucResult == STILL_DIALING)
				    					#if PS100|PS400
					     				&&(uiTimeout)
					     				#endif
				 					);
				Os__timer_stop(&uiTimeout);
				if(pdKey->gen_status == DRV_ENDED)
				{
					if(pdKey->xxdata[1] == KEY_CLEAR)
					{
						return(ERR_CANCEL);
					}else
					if(pdKey->xxdata[1] == KEY_ENTER)
					{
						pdKey = Os__get_key();
						KEY_COMMS_FinComm();
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (uchar *) "正在重试拨号");
						//Os__GB2312_display(1, 0, (uchar *) "请稍等...");
						//Os__GB2312_display(3, 0, (uchar *) "按<取消>键退出");
						ucResult = KEY_COMMS_PreComm();
						continue;
					}else
					{
						pdKey = Os__get_key();
						continue;
					}
				}
				if( ucResult == DIAL_CONNECT)
				{
					G_RUNDATA_ucHostConnectFlag = true;
					Os__abort_drv(drv_mmi);
					return(SUCCESS);
				}
				/* Retry Dial */
				KEY_COMMS_FinComm();
				if( ucI < DataSave0.ConstantParamData.ucDialRetry)
				{
					do{
						ucResult = KEY_COMMS_PreComm();
						ucI ++;
						if( ucResult == SUCCESS )
						{
							break;
						}else
						{
							KEY_COMMS_FinComm();
						}
					}while( ucI < DataSave0.ConstantParamData.ucDialRetry );
				}else
				{
					Os__abort_drv(drv_mmi);
					return(ERR_DIAL);
				}
				if( ucResult == SUCCESS )
				{
					continue;
				}else
				{
					Os__abort_drv(drv_mmi);
					return(ERR_DIAL);
				}
			}
		default:
			return(SUCCESS);
	}
}
unsigned char COMMS_HDLC_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char ucResult;
	unsigned char aucBuf[512];
	unsigned char ucLRC;
	unsigned short uiI;
	unsigned short uiLen,uiSysLen,uiCVLen,uiTempLen;
	unsigned short uiTimeout;
	unsigned char ucByte;
	unsigned char *pucData;
	unsigned char ucFirstChar;
	unsigned char aucTPDU[6];
	unsigned char aucSysFlag[5];
	unsigned char aucCOMMVER[5];
	unsigned char aucTMP[4];

	//TPDU
	memset(aucTPDU,0,sizeof(aucTPDU));
	memset(aucTPDU,' ',sizeof(aucTPDU));
	memcpy(aucTPDU,xDATA_Constant.aucKEYTpdu,5);

	//系统标识 = 转发应用标识+平台标识
	memset(aucSysFlag,0,sizeof(aucSysFlag));
	memcpy(aucSysFlag,"\xBB\xBA",2);
	memcpy(&aucSysFlag[2],"11",2);
	uiSysLen = sizeof(aucSysFlag)-1;
	util_Printf("系统标识Len:[%d]\n",uiSysLen);

	//通讯版本
	memset(aucCOMMVER,0,sizeof(aucCOMMVER));
	memcpy(aucCOMMVER,"00001",5);
	uiCVLen = sizeof(aucCOMMVER);
	util_Printf("通讯版本Len:[%d]\n",uiCVLen);

	//预留字节
	memset(aucTMP,0,sizeof(aucTMP));
	memcpy(aucTMP,"\x00\x00\x00\x00",4);

	uiTempLen = sizeof(aucTMP);
	util_Printf("预留字节Len:[%d]\n",uiTempLen);

       util_Printf("\nTPDU\n");
	for(uiI=0;uiI<5;uiI++)
	    util_Printf("[%02x]",aucTPDU[uiI]);
	util_Printf("\nG_RUNDATA_ucHostConnectFlag = %02x\n",G_RUNDATA_ucHostConnectFlag);

	if( G_RUNDATA_ucHostConnectFlag == false )
	{
		if(  (ucResult = KEY_COMMS_CheckPreComm()) != SUCCESS)
		{
			return(ucResult);
		}
		SEL_IncDialConnectCnt();
	}

	/* Clear communication buffer first */
	Os__com_flush2();

	/* Send Data */
	//Os__clr_display(255);
	//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
	//Os__GB2312_display(1, 0, (uchar *) "处理中...");
	//Os__GB2312_display(2, 0, (uchar *) "发送数据包");

	/*SEND STX*/
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = CHAR_STX;

	uiLen = uiInLen+5;  	/* Include TPDU Len*/
	uiLen = uiLen+uiSysLen+uiCVLen+uiTempLen;

	util_Printf("包总长=[%d]---\n",uiLen);
	short_bcd(&aucBuf[1],2,&uiLen);

	util_Printf("第二字节数[%02x] [%02x]\n",aucBuf[1],aucBuf[2]);
	for(ucLRC=0,uiI=1;uiI<3;uiI++)
	{
		ucLRC ^= aucBuf[uiI];
	}
	util_Printf("发送第一组数据 STX+SUMLen:\n");

#ifdef tCOMMPRINT
	PRINT_GB2312_xprint((unsigned char*)"发送第一组数据 包头",0x1d);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	hex_asc(aucPrintBuf,aucBuf,3*2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif

	for(uiI=0;uiI<3;uiI++)
	{
		util_Printf("[%02x] ",aucBuf[uiI]);
		if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
		{
			Os__txcar3(aucBuf[uiI]);
		}
		else
		{
		//util_Printf("结束第一组数据\n");
		//if(Os__txcar(aucBuf[uiI]) != OK )
		//{
		//	util_Printf("发送第一组数据出错\n");
		//	return(ERR_COMMS_SENDCHAR);
		//}
		}
	}
	util_Printf("\nucLRC=[%02x]\n",ucLRC);

	/* Send TPDU */
	util_Printf("发送第二组数据 TPDU:\n");

#ifdef tCOMMPRINT
	PRINT_GB2312_xprint((unsigned char*)"发送第二组数据 TPDU",0x1d);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	hex_asc(aucPrintBuf,aucTPDU,5*2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif

	for(uiI=0;uiI<5;uiI++)
	{
		util_Printf("[%02x] ",aucTPDU[uiI]);
		if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
		{
			Os__txcar3(aucBuf[uiI]);
		}
		else
		{
			//util_Printf("结束第二组数据\n");
			ucResult=Os__txcar(aucTPDU[uiI]);
			if(ucResult!= OK )
			{
				util_Printf("\n发送TPDU时出错 = %02x\n",ucResult);
				return(ERR_COMMS_SENDCHAR);
			}
		}
		ucLRC ^= aucTPDU[uiI];
	}
	util_Printf("\nucLRC=[%02x]\n",ucLRC);

	//发送应用标识
	util_Printf("发送应用标识:\n");
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,aucSysFlag,uiSysLen);
	memcpy(&aucBuf[uiSysLen],aucCOMMVER,uiCVLen);
	memcpy(&aucBuf[uiSysLen+uiCVLen],aucTMP,uiTempLen);

#ifdef tCOMMPRINT
	PRINT_GB2312_xprint((unsigned char *)"发送应用标识",0x1d);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	hex_asc(aucPrintBuf,aucBuf,(uiSysLen+uiCVLen+uiTempLen)*2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif

	for(uiI=0;uiI<(uiSysLen+uiCVLen+uiTempLen);uiI++)
	{
		util_Printf("[%02x] ",aucBuf[uiI]);
		if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
		{
			Os__txcar3(aucBuf[uiI]);
		}
		else
		{
			//util_Printf("结束发送应用标识数据\n");
			ucResult=Os__txcar(aucBuf[uiI]);
			if (ucResult!= OK )
			{
				util_Printf("\n发送应用标识 = %02x\n",ucResult);
				return(ERR_COMMS_SENDCHAR);
			}
		}
		ucLRC ^= aucBuf[uiI];
	}
	util_Printf("\nucLRC=[%02x]\n",ucLRC);


	util_Printf("发送第三组数据 ISO8583:\n");
	for(uiI=0;uiI<uiInLen;uiI++)
	{
		util_Printf("[%02x] ",*(pucInData+uiI));
		if(((uiI+1)%10) == 0)
			util_Printf("\n");
	}
	util_Printf("\n结束发送8583数据\n");

#ifdef tCOMMPRINT
	PRINT_GB2312_xprint((unsigned char*)"发送第三组数据",0x1d);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"长度:",5);
	hex_asc(&aucPrintBuf[5],uiInLen,2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	hex_asc(aucPrintBuf,pucInData,uiInLen*2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
	if ((uiInLen*2) > 44)
	{
		memset(aucPrintBuf1,0,sizeof(aucPrintBuf[128]));
		memcpy(aucPrintBuf1,&aucPrintBuf[44],(uiInLen*2)-44);
		PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
	}
#endif

	for(uiI=0;uiI<uiInLen;uiI++)
        {
            ucByte = *(pucInData+uiI);
            ucLRC ^= ucByte;
            if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
            {
                Os__txcar3(ucByte);
            }
            else
            {
                //util_Printf("结束发送第三组数据\n");
                if ( Os__txcar(ucByte) != OK )
                {
                    util_Printf("发送第三组数据 ISO8583 出错\n");
                    return(ERR_COMMS_SENDCHAR);
                }
            }
        }
	util_Printf("\nucLRC=[%02x]\n",ucLRC);

	/*Send ETX & LRC*/
	util_Printf("发送第四组数据 ETX+LRC:[%02x]\n",ucLRC);
	memset(aucBuf,0,sizeof(aucBuf));
	for (uiI=0;uiI<2;uiI++)
	{
		aucBuf[0] = CHAR_ETX;
		memcpy(&aucBuf[1],&ucLRC,1);
		util_Printf("[%02x] ",aucBuf[uiI]);
		if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
		{
			Os__txcar3(aucBuf[uiI]);
		}
		else
		{
			//util_Printf("结束发送第四组数据\n");
			if ( Os__txcar(aucBuf[uiI]) != OK )
			{
				util_Printf("发送第四组数据 ETX+LRC 出错\n");
				return(ERR_COMMS_SENDCHAR);
			}
		}
	}

#ifdef tCOMMPRINT
	PRINT_GB2312_xprint((unsigned char*)"发送第四组数据",0x1d);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	hex_asc(aucPrintBuf,aucBuf,2*2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif

	util_Printf("\nSEND OK AND Wait Receiveing\n");
	Os__xdelay(50);
	//Os__clr_display(255);
	//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
	//Os__GB2312_display(1, 0, (uchar *) "处理中...");
	//Os__GB2312_display(2, 0, (uchar *) "接收返回");

	/* Receive Data */
	Os__com_flush2();
	uiTimeout = DataSave0.ConstantParamData.uiReceiveTimeout*100;
	pucData = pucOutData;

	switch( DataSave0.ConstantParamData.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
		case PARAM_DOWNKEYCOMM_COMM:
			util_Printf("正在接收后台返回数据\nReceive--111---\n");

#ifdef tCOMMPRINT
			PRINT_GB2312_xprint((unsigned char*)"正在接收后台返回数据",0x1d);
#endif

			ucFirstChar = true;
			uiI = 0;
			do{
				ucResult = COMMS_KEYDOWNReceiveByte(&ucByte,&uiTimeout);
				//util_Printf("返回结果= %02x\n",ucResult);
#ifdef tCOMMPRINT
				PRINT_GB2312_xprint((unsigned char*)"返回结果:",0x1d);
				memset(aucPrintBuf,0,sizeof(aucPrintBuf));
				hex_asc(aucPrintBuf,&ucByte,2);
				PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif
				switch(ucResult)
				{
				case OK:
					if( (pucData-pucOutData) > *puiOutLen )
					{
						return(ERR_COMMS_RECVBUFFOVERLIMIT);
					}
					if( ucFirstChar == true )
					{
						if( ucByte != 0x60)	/* check valid tpdu header */
						{
							return(ERR_COMMS_ERRTPDU);
						}
						ucFirstChar = false;
					}
					if( uiI >= 18)		/* SKIP tpdu +uiSysLen+uiCVLen+uiTempLen*/
					{
						*pucData = ucByte;
						pucData ++;
					}
					uiI ++;
					uiTimeout = COMMS_RECVTIMEOUT;
					continue;
				case ERR_COMMS_RECVTIMEOUT:
					if( ucFirstChar == false )
					{
						*puiOutLen = uiI-5;
						break;
					}
					else
					{
						util_Printf("ERR_COMMS_RECVTIMEOUT31 uiI=%d\n",uiI);
						return(ucResult);
					}
				default:
					return(ucResult);
				}
				break;
		}while(1);
		break;
	default:
		break;
	}
	util_Printf("\nReceive--112---\n");
	return(SUCCESS);
}
unsigned char COMMS_NET_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char ucResult;
	unsigned char aucBuf[64];
	unsigned short uiI,uiOutLen;
	unsigned short uiSysLen,uiCVLen,uiTempLen;
	unsigned short uiTimeout;
	unsigned char ucLLen=2;
	unsigned char aucTPDU[6],ucTPDULen;
	unsigned char aucSysFlag[5];
	unsigned char aucCOMMVER[5];
	unsigned short usSendLen;
	unsigned char aucTMP[4],ucIPAddress[5];
	unsigned char aucSendBuff[ISO8583_MAXCOMMBUFLEN],aucReceiveBuff[ISO8583_MAXCOMMBUFLEN];
	int iTCPHandle,uiPacketLen,iJ;

	//TPDU
	memcpy(aucTPDU,xDATA_Constant.aucKEYTpdu,5);

	//系统标识 = 转发应用标识+平台标识
	memset(aucSysFlag,0,sizeof(aucSysFlag));
	memcpy(aucSysFlag,"\xBB\xBA",2);
	memcpy(&aucSysFlag[2],"11",2);
	uiSysLen = sizeof(aucSysFlag)-1;
	util_Printf("系统标识Len:[%d]\n",uiSysLen);

	//通讯版本
	memset(aucCOMMVER,0,sizeof(aucCOMMVER));
	memcpy(aucCOMMVER,"00001",5);
	uiCVLen = sizeof(aucCOMMVER);
	util_Printf("通讯版本Len:[%d]\n",uiCVLen);

	//预留字节
	memset(aucTMP,0,sizeof(aucTMP));
	memcpy(aucTMP,"\x00\x00\x00\x00",4);
	uiTempLen = sizeof(aucTMP);
	util_Printf("预留字节Len:[%d]\n",uiTempLen);
    //Os__clr_display(255);
     if( G_RUNDATA_ucHostConnectFlag == false )
    {
        if(  (ucResult = KEY_COMMS_CheckPreComm()) != SUCCESS)
        {
            util_Printf("\nCOMMS_CheckPreComm %02x",ucResult);
            return(ucResult);
        }
    }
    // 设置通讯版本 1020为斯玛特通讯版本
    ucResult = COMMSENCRYPT_SetVersion("1020");
    if( ucResult != SUCCESS )
    {
    	util_Printf("COMMSENCRYPT_SetVersion ucResult = %02x\n",ucResult);
    	COMMSENCRYPT_ErrMsg(ucResult);
    	return ERR_CANCEL;
    }
    util_Printf("\n Before TCP_Open");
    ucResult=TCP_Open(&iTCPHandle);
    util_Printf("\n End TCP_Open ucResult:[%02x]",ucResult);
	if (ucResult != SUCCESS) return(ucResult);

	memset(ucIPAddress,0x00,sizeof(ucIPAddress));
	memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulHostKEYIP,sizeof(unsigned long));
	for(uiI=0;uiI<3;uiI++)
	{
		util_Printf("\nTCP/IP(Encrypt)-in[uiI]=%d\n",uiI);
		util_Printf("\nTCP/IP(Encrypt) =%d, TCP_Connect:%d.\n",uiI,DataSave0.ConstantParamData.uiHostKEYPort);

		ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiHostKEYPort);
		util_Printf("备份IP地址：\nucIPAddress1=%02x.%02x.%02x.%02x\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
		util_Printf("\n备份端口=%d",DataSave0.ConstantParamData.uiHostKEYPort);
		util_Printf("\nTcp_Connect Out ucResult: [%02x]  \n",ucResult);
		if (!ucResult)	break;
	}

	if (!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
		//Os__GB2312_display(1, 0, (uchar *) "数据发送中...");
		//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
		util_Printf("\nConnect Server OK");

		memset(aucSendBuff,0x00,sizeof(aucSendBuff));
		usSendLen = 0;
		ucTPDULen=sizeof(aucTPDU)-1;
		uiPacketLen=ucTPDULen+uiInLen+uiSysLen+uiCVLen+uiTempLen; //包长[TPDU+ISO+系统标识+通讯版本+预留字节]
		usSendLen = uiPacketLen;

		/*aucSendBuff[0]=CHAR_STX;*/
		CONV_IntHex(&aucSendBuff[0],2,&uiPacketLen);
		memcpy(aucSendBuff+2,aucTPDU,ucTPDULen);
		memcpy(aucSendBuff+2+ucTPDULen,aucSysFlag,uiSysLen);
		memcpy(aucSendBuff+2+ucTPDULen+uiSysLen,aucCOMMVER,uiCVLen);
		memcpy(aucSendBuff+2+ucTPDULen+uiSysLen+uiCVLen,aucTMP,uiTempLen);
		memcpy(aucSendBuff+2+ucTPDULen+uiSysLen+uiCVLen+uiTempLen,pucInData,uiInLen);

		util_Printf("\n*******Send  Packet Data*******\n");
		for(uiI=0;uiI<uiPacketLen+2;uiI++)
		{
			if((uiI+1)%20==0) util_Printf("\n");

			util_Printf("%02x ",aucSendBuff[uiI]);
		}

		util_Printf("\nBegin Send Data");
		/*Add at 07-12-10*/
		memset(aucEncryptData,0,sizeof(aucEncryptData));
		memcpy(aucEncryptData,aucSendBuff,usSendLen+2);
		ucResult=DataEncrypt(aucEncryptData,usSendLen+2,DataSave0.ConstantParamData.aucTerminalID,
										aucEncryptData,&uiOutLen);
/*
		{
			int i;
			util_Printf("usSendLen=========%d ;uiOutLen=%d\n",usSendLen,uiOutLen);
			for( i=0;i<usSendLen;i++)
				util_Printf("%02x ",aucSendBuff[i]);
			util_Printf("\n");
		}
*/
		if(ucResult!=SUCCESS)
		{
			COMMSENCRYPT_ErrMsg(ucResult);
			ucResult = ERR_CANCEL;
		}
/*End*/
		ucResult=TCP_Send(iTCPHandle,aucEncryptData,uiOutLen);
		util_Printf("\nSend Data End ucResult:[%02x]",ucResult);

		uiTimeout = DataSave0.ConstantParamData.uiReceiveTimeout;
		if (!ucResult)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
			//Os__GB2312_display(1, 0, (uchar *) "等待接收数据...");
			//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
		}
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,"共计使用    秒",14);
		if( !ucResult )
		{
			util_Printf("\nBegin Receive Data");
			for(uiI=1;uiI<=uiTimeout;uiI++)
			{
				uiOutLen=ISO8583_MAXCOMMBUFLEN;
				ucResult = TCP_Recv(iTCPHandle,aucEncryptData,&uiOutLen ,1);
				util_Printf("TCP_Recv()----ucResult=%02x\n",ucResult);
				int_asc(aucBuf+10,2,(unsigned int*)&uiI);
				if(uiI==1)
				{
					//Os__clr_display(1);
					//Os__GB2312_display(1, 0, (uchar *) "数据接收中...");
				}
				//Os__GB2312_display(3,0,aucBuf);
				/*add 07*/
				if( ucResult == SUCCESS )
				{
					ucResult = DataDecrypt(aucEncryptData,aucEncryptData,&uiOutLen);
					if( ucResult == SUCCESS )
					{
						// 例子中不按照具体的报文进行处理，程序需针对实际数据进行相应处理
						memcpy(aucReceiveBuff,&aucEncryptData[0],uiOutLen);
					}
					else
					{
						COMMSENCRYPT_ErrMsg(ucResult);
						ucResult = ERR_CANCEL;
					}
				}
				/*end*/
				if( !ucResult )
				{
					util_Printf("\nReceive Data Len = %d\n",uiOutLen);
					*puiOutLen = uiOutLen;
					if(uiOutLen>2)
					{
#if 0
							util_Printf("\n*********Receive Packet Data***********\n");
							for(iJ=0;iJ<uiOutLen;iJ++)
							{
								if((uiI+1)%20==0) util_Printf("\n");
								util_Printf("%02x ",aucReceiveBuff[iJ]);
							}
							util_Printf("***************************END****************************\n");
#endif
						if(aucReceiveBuff[0]==0x60||aucReceiveBuff[0]==0x80)//first byte of APTU
						{
							util_Printf("--F--\n");
							memcpy(pucOutData,&aucReceiveBuff[ucTPDULen+6],uiOutLen-ucTPDULen-6);
							*puiOutLen-=(ucTPDULen+6);
						}
						else
						{
							util_Printf("--E--\n");
							memcpy(pucOutData,&aucReceiveBuff[ucTPDULen+ucLLen+6],uiOutLen-ucTPDULen-6-ucLLen);
							*puiOutLen -= (ucTPDULen+6+ucLLen);
						}
#if 0
							util_Printf("\n*********Receive Packet Data********pucOutData***puiOutLen=%d\n",*puiOutLen);
							for(iJ=0;iJ<*puiOutLen;iJ++)
							{
								if((uiI+1)%20==0) util_Printf("\n");
								util_Printf("%02x ",*(pucOutData+iJ));
							}
							util_Printf("********************END***********************\n");
#endif
						break;
					}
				}
			}
		}
	}
	else
	{
		OSGSM_GprsReset();
	}
	if (!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "数据接收完成！");
		//Os__GB2312_display(1, 0, (uchar *) "正在处理中...");
		//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
	}
	TCP_Close(iTCPHandle);
	return(ucResult);
}
unsigned char COMMS_SendReceiveKEY(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char ucResult;

	util_Printf("\n密钥下载通讯方式:[%02x]\n",DataSave0.ConstantParamData.ucKEYCOMM);
	util_Printf("[1]=拨号 [2]=GPRS [3]=TCP/IP\n");
	util_Printf("上送ISO8583数据Len=[%d]\n",uiInLen);
	switch (xDATA_Constant.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
		case PARAM_DOWNKEYCOMM_COMM:
			ucResult=COMMS_HDLC_SendReceive(pucInData,uiInLen,pucOutData,puiOutLen);
			break;
		case PARAM_DOWNKEYCOMM_GPRS:
		case PARAM_DOWNKEYCOMM_TCPIP:
			ucResult=COMMS_NET_SendReceive(pucInData,uiInLen,pucOutData,puiOutLen);
        	        if( ucResult != SUCCESS)
        	    	{
        	    		MSG_DisplayNetworkErrMsg(ucResult);
    				ucResult=ERR_END;
    				util_Printf("[CONNECT NET]===1===%02x\n",ucResult);
        	    	}
			break;
		default:
			//Os__clr_display(255);
			//Os__GB2312_display(1,1,(unsigned char *)"未知通讯方式");
			//Os__GB2312_display(2,0,(unsigned char *)" 请设置通讯方式");
			UTIL_GetKey(5);
			return(SUCCESS);
	}
#ifdef tCOMMPRINT
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"SRK: ",5);
	hex_asc(&aucPrintBuf[5],&ucResult,2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1d);
#endif
	util_Printf("通讯返回:[%02x]\n",ucResult);
	return (ucResult);
}
unsigned char COMMS_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
    unsigned char ucResult = SUCCESS;

    switch( DataSave0.ConstantParamData.ucCommMode )
    {
		case PARAM_COMMMODE_MODEM:
		case PARAM_COMMMODE_RS232:
		case PARAM_COMMMODE_HDLC:
			util_Printf("\n拨号方式时交易类型[%02x]\n需打冲正标志值:%02x\n",G_NORMALTRANS_ucTransType,G_RUNDATA_ucReversalFlag);
			util_Printf("\n[CONNECT MODEL]=[MODEM]/[RS232]/[HDLC]\n1COMMS_SendReceive:%02x\n",DataSave0.ConstantParamData.ucCommMode);
			ucResult=COMMS_MODEM_SendReceive(pucInData,uiInLen,pucOutData,puiOutLen);
			break;
		case PARAM_COMMMODE_TCPIP:
		case PARAM_COMMMODE_GPRS:
		case PARAM_COMMMODE_CDMA:
					util_Printf("\n[CONNECT MODEL]==[TCPIP]/[GPRS]/[CDMA]:%02x\n",DataSave0.ConstantParamData.ucCommMode);
					if(DataSave0.ConstantParamData.ucEncryptDataFlag=='0')
					{
			        	    ucResult=COMMS_TCPIP_SendReceive(pucInData,uiInLen,pucOutData,puiOutLen);
					}
					else
					{
						ucResult=COMMS_TCPIP_SendReceive_Encrypt(pucInData,uiInLen,pucOutData,puiOutLen);
					}
        	        if( ucResult != SUCCESS
        				&& DataSave0.ConstantParamData.ucCommMode !=PARAM_COMMMODE_TCPIP)
        	    	{
                            MSG_DisplayNetworkErrMsg(ucResult);
                            ucResult=ERR_END;
                            util_Printf("[CONNECT MODEL]===1===%02x\n",ucResult);
        	    	}
                	break;
            	default:
       		break;
    }
	util_Printf("[CONNECT MODEL]===2===%02x\n",ucResult);
    return ucResult;
}

/***********************************************************************
函数名称:COMMS_MODEM_SendReceive
创建时间:2007/01/16
创建人员:潘敏
功能描述:串口/拨号通讯
入口参数:unsigned char *pucInData    输入数据
			  unsigned short uiInLen	       输入数据长度
			  unsigned char *pucOutData 输出数据
			  unsigned short *puiOutLen    输出数据长度
返回值:			  0     成功
					其他失败
修改记录:
***********************************************************************/
unsigned char COMMS_MODEM_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char ucResult;
	unsigned char aucBuf[16];
	unsigned short uiI;
	unsigned short uiDataLen;
	int uiPacketLen;
	unsigned char ucTPDULen,ucPackgeHeadLen;
	unsigned short uiTimeout;
	unsigned char ucByte;
	unsigned char *pucData;
	unsigned char ucFirstChar ;
	unsigned char aucTPDU[6];
	unsigned char aucSandServer[3];
	unsigned char aucPackgeHead[7];
	unsigned char ucDispBuf2[3],ucReFile=SUCCESS;
	unsigned char ucDispBuf1[17];
	unsigned char ucDispCAPK[3];
	unsigned char ucDispBuf[17],ucIndex;
	unsigned char aucSendBuff[ISO8583_MAXCOMMBUFLEN];
	unsigned short usMBFELen=0;
	unsigned char aucMBFEBuf[3];

	//60 00 03 00 00
	aucTPDU[0] = 0x60;
	aucTPDU[1] = DataSave0.ConstantParamData.aucTpdu[0];
	aucTPDU[2] = DataSave0.ConstantParamData.aucTpdu[1];
	aucTPDU[3] = 0x00;
	aucTPDU[4] = 0x00;

	util_Printf("\nTPDU %02x %02x  \n",DataSave0.ConstantParamData.aucTpdu[0],DataSave0.ConstantParamData.aucTpdu[1]);

	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucPackgeHead,0,sizeof(aucPackgeHead));
	memcpy(aucBuf,"60",2);
	memcpy(&aucBuf[2],"22",2);

	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
		aucBuf[4] = '1';
	else
		aucBuf[4] = '0';

	memcpy(&aucBuf[5],"0000000",7);

	asc_bcd(aucPackgeHead,6,aucBuf,12);

	{
		int i;
		util_Printf("\n aucPackgeHead:");
		for(i=0;i<6;i++)
			util_Printf("%02x ",aucPackgeHead[i]);
		util_Printf("\n");
	}

	/*********************************************************/
	util_Printf("G_RUNDATA_ucHostConnectFlag = %02x\n",G_RUNDATA_ucHostConnectFlag);

	if( G_RUNDATA_ucHostConnectFlag == false )
	{
		util_Printf("-------comm--110---\n");
		if(  (ucResult = COMMS_CheckPreComm()) != SUCCESS)
		{
			return(ucResult);
		}
		SEL_IncDialConnectCnt();
	}
	util_Printf("-------comm--a110---\n");

	/* Clear communication buffer first */
	Os__com_flush2();

	/* Send Data */
	util_Printf("该交易存在冲正交易:%02x\n",G_RUNDATA_ucReversalFlag);
	util_Printf("Set冲正标志前=%02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
	if (G_RUNDATA_ucReversalFlag==0x31)
	{
		G_RUNDATA_bConectFlag=true;
		Os__saved_set((unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid),
						DATASAVEVALIDFLAG,sizeof(unsigned char));
		ucReFile =XDATA_Write_Vaild_File(DataSaveTrans8583);
		util_Printf("Set冲正标志后= %02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
		if (ucReFile != SUCCESS)
		{
			MSG_DisplayErrMsg(ucReFile);
			return(ERR_CANCEL);
		}
	}

	memset(ucDispBuf2,0,sizeof(ucDispBuf2));
	memset(ucDispBuf1,0,sizeof(ucDispBuf1));
	memset(ucDispCAPK,0,sizeof(ucDispCAPK));
	memset(ucDispBuf,0,sizeof(ucDispBuf));
	if(G_NORMALTRANS_ucTransType==TRANS_LOADPARAM
		&&DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOAD)
	{
		char_asc(ucDispCAPK, 2,&ucCAPKNum);
		ucIndex=ucTermCAPKNum+1;
	}
	if(G_NORMALTRANS_ucTransType==TRANS_LOADPARAM
		&&DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOAD)
	{
		char_asc(ucDispCAPK, 2,&SupportAIDNum);
		ucIndex=ucTermAIDNum+1;
	}
	char_asc(ucDispBuf2, 2,&ucIndex);

	strcpy((char *)ucDispBuf , "共需下载[  ]个");
	memcpy( &ucDispBuf[9], ucDispCAPK, 2);
	strcpy((char *)ucDispBuf1 , "正在下载第[  ]个");
	memcpy( &ucDispBuf1[11], ucDispBuf2, 2);

	//Os__clr_display(255);
	if((G_NORMALTRANS_ucTransType==TRANS_LOADPARAM
		||G_NORMALTRANS_ucTransType==TRANS_SENDSTATS)
		&&(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOAD
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOAD
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKQUERY
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOADEND
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDQUERY
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOADEND))
	{
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKQUERY)
			//Os__GB2312_display(0, 0, (uchar *) "----公钥查询----");
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOAD)
			//Os__GB2312_display(0, 0, (uchar *) "----公钥下载----");
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOADEND)
			//Os__GB2312_display(0, 0, (uchar *) "--公钥下载结束--");
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDQUERY)
			//Os__GB2312_display(0, 0, (uchar *) "---IC参数查询---");
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOAD)
			//Os__GB2312_display(0, 0, (uchar *) "---IC参数下载---");
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOADEND)
			//Os__GB2312_display(0, 0, (uchar *) "-IC参数下载结束-");

		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOAD
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOAD)
		{
			//Os__GB2312_display(1, 0, ucDispBuf);
			//Os__GB2312_display(2, 0, ucDispBuf1);
			//Os__GB2312_display(3, 0, (uchar *)"    请稍等...");
		}else
		{
			//Os__GB2312_display(1, 0, (uchar *) "已连接收单中心");
			//Os__GB2312_display(2, 0, (uchar *) "处理中...");
			//Os__GB2312_display(3, 0, (uchar *) "发送数据包");
		}
	}else
	{
		//Os__GB2312_display(0, 0, (uchar *) "已连接收单中心");
		//Os__GB2312_display(1, 0, (uchar *) "处理中...");
		//Os__GB2312_display(2, 0, (uchar *) "发送数据包");
	}

        memset(aucSendBuff,0x00,sizeof(aucSendBuff));
        ucTPDULen=sizeof(aucTPDU)-1;
        ucPackgeHeadLen = sizeof(aucPackgeHead)-1;

        uiPacketLen= ucTPDULen+ucPackgeHeadLen+uiInLen;
        memcpy(aucSendBuff,aucTPDU,ucTPDULen);

       util_Printf("M是否支持前置转发1:[%02x]\n",DataSave0.ConstantParamData.ucMBFEFlag);
       if (DataSave0.ConstantParamData.ucMBFEFlag == 0x31)
        {
            memset(aucSandServer,0,sizeof(aucSandServer));
            memcpy(aucSandServer,"\xAA\xBD",2);
            memcpy(aucSendBuff+ucTPDULen,aucSandServer,2);

            memcpy(aucSendBuff+ucTPDULen+2,aucPackgeHead,ucPackgeHeadLen);
            memcpy(aucSendBuff+ucTPDULen+2+ucPackgeHeadLen,pucInData,uiInLen);
            //if (DataSave0.ConstantParamData.ucMBFEFlag == 0x31)
            {
                   usMBFELen = ISO8583_MBFE.uiCommBufLen;
                   util_Printf("上送前置数据长度:%d\n",usMBFELen);
                  if ((G_NORMALTRANS_ucTransType==TRANS_PURCHASE
		        ||G_NORMALTRANS_ucTransType==TRANS_VOIDPURCHASE
		        ||G_NORMALTRANS_ucTransType == TRANS_REVERSAL)
		        && (G_RUNDATA_ucTransType != TRANS_SETTLE)
		        && (usMBFELen != 0)
		        )
		    {

		       util_Printf("给前置送的数据:\n");
		       for(uiI=0;uiI<usMBFELen;uiI++)
		            util_Printf("%02x ",ISO8583_MBFE.aucCommBuf[uiI]);
		       util_Printf("\n");
		       memcpy(aucSendBuff+ucTPDULen+2+ucPackgeHeadLen+uiInLen,ISO8583_MBFE.aucCommBuf,usMBFELen);
        		if (DataSave0.ConstantParamData.ucSendOrder=='1')                     //支持订单上送02
        		{
        		    memcpy(aucSendBuff+uiPacketLen+2+usMBFELen,MBFE_BUF,10);
        		    uiPacketLen+=usMBFELen+12;
        		}
        		else                                                                                                //仅支持前置转发01
        		{
        		    memcpy(aucSendBuff+uiPacketLen+2+usMBFELen,MBFE_LBUF,10);
        		    uiPacketLen+=12;
        		}

        		memset(aucMBFEBuf,0,sizeof(aucMBFEBuf));
        		CONV_IntHex(aucMBFEBuf,2,&usMBFELen);
        		util_Printf("给前置长度:%02x %02x\n",aucMBFEBuf[0],aucMBFEBuf[1]);
        		memcpy(aucSendBuff+uiPacketLen,aucMBFEBuf,2);
        	    }
        	    else if (DataSave0.ConstantParamData.ucSendOrder=='1')      //支持订单上送02x
        	    {
        	        memcpy(aucSendBuff+uiPacketLen+2,MBFE_BUF,12);
        	        uiPacketLen+=12;
        	    }
        	    else                                                                                      //仅支持前置转发上送01
		    {
		        memcpy(aucSendBuff+uiPacketLen+2,MBFE_LBUF,12);
		        uiPacketLen+=12;
	           }
	           uiPacketLen+=2;
            }

            //调试数据
            util_Printf("拨号前置发送数据:\n");
            for(uiDataLen=0;uiDataLen<uiPacketLen;uiDataLen++)
            {
                if (!(uiDataLen%10))    util_Printf("\n");
                util_Printf("%02x ",aucSendBuff[uiDataLen]);
            }
            util_Printf("\n");

            for (uiDataLen=0;uiDataLen<uiPacketLen;uiDataLen++)
            {
                if ( Os__txcar(aucSendBuff[uiDataLen]) != OK )
                {
                    return(ERR_COMMS_SENDCHAR);
                }
            }
        }
        else
        {
            memcpy(aucSendBuff+ucTPDULen,aucPackgeHead,ucPackgeHeadLen);
            memcpy(aucSendBuff+ucTPDULen+ucPackgeHeadLen,pucInData,uiInLen);

            //调试数据
            util_Printf("拨号发送数据:\n");
            for(uiDataLen=0;uiDataLen<uiPacketLen;uiDataLen++)
            {
                if (!(uiDataLen%10))    util_Printf("\n");
                util_Printf("%02x ",aucSendBuff[uiDataLen]);
            }
            util_Printf("\n");

            for (uiDataLen=0;uiDataLen<uiPacketLen;uiDataLen++)
            {
                if ( Os__txcar(aucSendBuff[uiDataLen]) != OK )
                {
                    return(ERR_COMMS_SENDCHAR);
                }
            }
        }

        util_Printf("\nSEND OK AND Wait Receiveing\n");

	if((G_NORMALTRANS_ucTransType==TRANS_LOADPARAM
		||G_NORMALTRANS_ucTransType==TRANS_SENDSTATS)
		&&(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOAD
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOAD
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKQUERY
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOADEND
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDQUERY
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOADEND))
	{
		if(DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_PKLOAD
		||DataSave0.ChangeParamData.euLoadParamType==LOADPARAMTYPE_AIDLOAD)
		{
		}else
		{
			//Os__GB2312_display(1, 0, (uchar *) "已连接收单中心");
			//Os__GB2312_display(2, 0, (uchar *) "处理中...");
			//Os__GB2312_display(3, 0, (uchar *) "接收返回");
		}
	}else
	{
		//Os__GB2312_display(0, 0, (uchar *) "已连接收单中心");
		//Os__GB2312_display(1, 0, (uchar *) "处理中...");
		//Os__GB2312_display(2, 0, (uchar *) "接收返回");
	}

	/* Receive Data */
	Os__com_flush2();
	uiTimeout = DataSave0.ConstantParamData.uiReceiveTimeout*100;
	pucData = pucOutData;
	switch( DataSave0.ConstantParamData.ucCommMode )
	{
	case PARAM_COMMMODE_MODEM:
	case PARAM_COMMMODE_RS232:
		break;
	default:
		util_Printf("Receive--111---\n");
		ucFirstChar = true;
		uiI = 0;
		do{
			ucResult = COMMS_ReceiveByte(&ucByte,&uiTimeout);
			switch(ucResult)
			{
			case OK:
				if( (pucData-pucOutData) > *puiOutLen )
				{
					return(ERR_COMMS_RECVBUFFOVERLIMIT);
				}
				if( ucFirstChar == true )
				{
					if( ucByte != 0x60)	/* check valid tpdu header */
					{
						return(ERR_COMMS_ERRTPDU);
					}
					ucFirstChar = false;
				}
				if( uiI >= 5+6)	/* skip tpdu */
				{
					*pucData = ucByte;
					pucData ++;
				}else		/*保存下发处理要求*/
				{
						if(uiI>=5)
							aucPackgeHead[uiI-5] = ucByte;
				}
				uiI ++;
				uiTimeout = COMMS_RECVTIMEOUT;
				continue;
			case ERR_COMMS_RECVTIMEOUT:
				if( ucFirstChar == false )
				{
					*puiOutLen = uiI-5-6;
					 DataSave0.ChangeParamData.ucPackgeHeadEve=aucPackgeHead[2]&0x0F;
					XDATA_Write_Vaild_File(DataSaveChange);

					util_Printf("\nucPackgeHeadEve %02X\n",DataSave0.ChangeParamData.ucPackgeHeadEve);
					util_Printf("ERR_COMMS_RECVTIMEOUT uiI=%d\n",uiI);

					break;
				}else
				{
					util_Printf("ERR_COMMS_RECVTIMEOUT22 uiI=%d\n",uiI);
					return(ucResult);
				}
			default:
				return(ucResult);
			}
			break;
		}while(1);
		break;
	}
	util_Printf("\nReceive--112---\n");
	return(SUCCESS);
}
/***********************************************************************
函数名称:COMMS_TCPIP_SendReceive
创建时间:2007/01/16
创建人员:潘敏
功能描述:网络/GPRS/CDMA通讯
入口参数:unsigned char *pucInData    输入数据
			  unsigned short uiInLen	       输入数据长度
			  unsigned char *pucOutData 输出数据
			  unsigned short *puiOutLen    输出数据长度
返回值:			  0     成功
					其他失败
修改记录:
***********************************************************************/
unsigned char COMMS_TCPIP_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen)
{
        unsigned char ucResult,ucReFile,ucIPAddress[5];
        unsigned int uiTimeout;
        int iTCPHandle,uiI,uiPacketLen;
        int iJ;
        unsigned char ucLLen=2;
        unsigned char aucBuf[20];
        unsigned char aucSendBuff[ISO8583_MAXCOMMBUFLEN];
        unsigned char aucReceiveBuff[ISO8583_MAXCOMMBUFLEN];
        unsigned char aucTPDU[6];
        unsigned char aucPackgeHead[7];
        unsigned char ucTPDULen;
        unsigned char ucPackgeHeadLen;
        unsigned short usMBFELen;
        unsigned char aucMBFEBuf[20];

	aucTPDU[0] = 0x60;
	aucTPDU[1] = DataSave0.ConstantParamData.aucTpdu[0];
	aucTPDU[2] = DataSave0.ConstantParamData.aucTpdu[1];
	aucTPDU[3] = 0x80;
	aucTPDU[4] = 0x00;

	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucPackgeHead,0,sizeof(aucPackgeHead));
	memcpy(aucBuf,"60",2);
	memcpy(&aucBuf[2],"22",2);
	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
		aucBuf[4] = '1';
	else
		aucBuf[4] = '0';

	memcpy(&aucBuf[5],"0000000",7);
	asc_bcd(aucPackgeHead,6,aucBuf,12);

    //Os__clr_display(255);
     if( G_RUNDATA_ucHostConnectFlag == false )
    {
        if(  (ucResult = COMMS_CheckPreComm()) != SUCCESS)
        {
            util_Printf("\nCOMMS_CheckPreComm %02x",ucResult);
            return(ucResult);
        }
    }
    util_Printf("\n Before TCP_Open");
    ucResult=TCP_Open(&iTCPHandle);
    util_Printf("\n End TCP_Open ucResult:[%02x]",ucResult);
    if(ucResult!=SUCCESS) return ucResult;
	util_Printf("选择IP:%02x\n",DataSave0.ConstantParamData.ucSelIP);
	if (DataSave0.ConstantParamData.ucSelIP == 1)
	{
		memset(ucIPAddress,0x00,sizeof(ucIPAddress));
		memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulHostIP,sizeof(unsigned long));
		for(uiI=0;uiI<3;uiI++)
		{
			util_Printf("\nTCP/IP-Out[uiI]=%d\n",uiI);
		util_Printf("\nTCP/IP(Encrypt) =%d, TCP_Connect:%d.\n",uiI,DataSave0.ConstantParamData.uiHostPort);			
			ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiHostPort);
			util_Printf("主IP地址：\nucIPAddress1=%02x.%02x.%02x.%02x\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
			util_Printf("\n主端口=%d",DataSave0.ConstantParamData.uiHostBackPort);
			util_Printf("\nTcp_Connect No Key Out ucResult1: [%02x]  \n",ucResult);
			if(ucResult != SUCCESS && uiI == 2)
			{
				DataSave0.ConstantParamData.ucSelIP = 2;
				XDATA_Write_Vaild_File(DataSaveConstant);
				if (DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
				{
					MSG_DisplayNetworkErrMsg(ucResult);
					ucResult = ERR_END;
					//Os__clr_display(255);
					//Os__GB2312_display(0, 1, (uchar *) " 主专线故障");
					//Os__GB2312_display(1, 2, (uchar *) " 请报修");
					//Os__GB2312_display(2, 0, (uchar *) "正切换备用线路IP");
					MSG_WaitKey(3);
					//Os__clr_display(255);
					//Os__GB2312_display(0, 0, (uchar *) "正在连接网络...");
					//Os__GB2312_display(2, 0, (uchar *) "    请稍等...");
				}
				break;
			}else
			{
			    if (!ucResult) break;
			}
		}
	}

	util_Printf("选择FIP:%02x\n",DataSave0.ConstantParamData.ucSelIP);
	if (DataSave0.ConstantParamData.ucSelIP == 2)
	{
		memset(ucIPAddress,0x00,sizeof(ucIPAddress));
		memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulBackIP,sizeof(unsigned long));
		for(uiI=0;uiI<3;uiI++)
		{
			util_Printf("\nTCP/IP-In[uiI]=%d\n",uiI);
		util_Printf("\nTCP/IP(Encrypt) =%d, TCP_Connect:%d.\n",uiI,DataSave0.ConstantParamData.uiHostBackPort);			

			ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiHostBackPort);
			util_Printf("备份IP地址：\nucIPAddress2=%02x.%02x.%02x.%02x\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
			util_Printf("\n备份端口=%d",DataSave0.ConstantParamData.uiHostBackPort);
			util_Printf("\n Tcp_Connect No Key Out ucResult2: [%02x]  \n",ucResult);
			if(ucResult != SUCCESS && uiI==2)
			{
				DataSave0.ConstantParamData.ucSelIP = 1;
				XDATA_Write_Vaild_File(DataSaveConstant);
				if (DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
				{
					MSG_DisplayNetworkErrMsg(ucResult);
					ucResult = ERR_END;
				}
				break;
			}
			else
			{
				if (!ucResult) break;
			}
		}
	}
	util_Printf("网络不加密该交易[%02x]存在冲正交易:%02x\n",G_NORMALTRANS_ucTransType,G_RUNDATA_ucReversalFlag);
	if (!ucResult && G_RUNDATA_ucReversalFlag == 0x31)
	{
		G_RUNDATA_bConectFlag=true;
		util_Printf("Set冲正标志前=%02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
		Os__saved_set((unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid),
						DATASAVEVALIDFLAG,sizeof(unsigned char));
		ucReFile=XDATA_Write_Vaild_File(DataSaveTrans8583);
		util_Printf("Set冲正标志后= %02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
        	if (ucReFile != SUCCESS)
        	{
        		MSG_DisplayErrMsg(ucReFile);
        		TCP_Close(iTCPHandle);
        		return(ERR_CANCEL);
        	}
	}

    if( !ucResult )
    {
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
		//Os__GB2312_display(1, 0, (uchar *) "数据发送中...");
		//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
		util_Printf("\nConnect Server OK");

		memset(aucSendBuff,0x00,sizeof(aucSendBuff));
		ucTPDULen=sizeof(aucTPDU)-1;
		ucPackgeHeadLen = sizeof(aucPackgeHead)-1;

		if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
		{
		    usMBFELen = ISO8583_MBFE.uiCommBufLen;
		    util_Printf("新加前置IS08583数据长度:%d\n",usMBFELen);
		    if ((DataSave0.ConstantParamData.ucSendOrder==0x31)
		        && (G_NORMALTRANS_ucTransType==TRANS_PURCHASE
		        ||G_NORMALTRANS_ucTransType==TRANS_VOIDPURCHASE
		        ||G_NORMALTRANS_ucTransType == TRANS_REVERSAL)
		        && (usMBFELen != 0)
		        )
		    {
        		 uiPacketLen=ucTPDULen+uiInLen+ucPackgeHeadLen+2+usMBFELen+12;
		    }
		    else
                  {
                    uiPacketLen=ucTPDULen+uiInLen+ucPackgeHeadLen+2+12;
                  }
		}
		else
		    uiPacketLen=ucTPDULen+uiInLen+ucPackgeHeadLen;

		CONV_IntHex(aucSendBuff,2,&uiPacketLen);
	        if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
                {
                  memcpy(aucSendBuff+2,aucTPDU,ucTPDULen);
                  memcpy(aucSendBuff+2+ucTPDULen,"\xAA\xBD",2);
                  memcpy(aucSendBuff+2+ucTPDULen+2,aucPackgeHead,ucPackgeHeadLen);
                  memcpy(aucSendBuff+2+ucTPDULen+2+ucPackgeHeadLen,pucInData,uiInLen);
                }
                else
                {
                    memcpy(aucSendBuff+2,aucTPDU,ucTPDULen);
                    memcpy(aucSendBuff+2+ucTPDULen,aucPackgeHead,ucPackgeHeadLen);
                    memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen,pucInData,uiInLen);
                }

		if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
		{
		    if ((DataSave0.ConstantParamData.ucSendOrder==0x31)
		        && (G_NORMALTRANS_ucTransType==TRANS_PURCHASE
		        ||G_NORMALTRANS_ucTransType==TRANS_VOIDPURCHASE
		        ||G_NORMALTRANS_ucTransType == TRANS_REVERSAL)
		        )
		    {
		       memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen,ISO8583_MBFE.aucCommBuf,usMBFELen);
        		memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen+usMBFELen,MBFE_BUF,10);
        		memset(aucMBFEBuf,0,sizeof(aucMBFEBuf));
        		CONV_IntHex(aucMBFEBuf,2,&usMBFELen);
        		memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen+usMBFELen+10,aucMBFEBuf,2);
        	    }
        	    else
                  {
                    memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen,MBFE_LBUF,12);
                  }
		}

#if 0
            util_Printf("\n*******Send  Packet Data*******\n");
            if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
            {
                util_Printf("前置转发数据:\n");
                for(uiI=0;uiI<uiPacketLen+2;uiI++)
                {
                    if(!((uiI+1)%20)) util_Printf("\n");
                    util_Printf("%02x ",aucSendBuff[uiI]);
                }
            }
            else
            {
                util_Printf("银联数据:\n");
                for(uiI=0;uiI<uiPacketLen+2;uiI++)
                {
                    if(!((uiI+1)%20)) util_Printf("\n");
                    util_Printf("%02x ",aucSendBuff[uiI]);
                }
            }
#endif
		util_Printf("\nBegin Send Data");

		ucResult=TCP_Send(iTCPHandle,aucSendBuff,uiPacketLen+2);

		util_Printf("\n发送数据返回:ucResult:[%02x]",ucResult);

		uiTimeout = DataSave0.ConstantParamData.uiReceiveTimeout;
		//Os__timer_start(&uiTimeout);
		if (!ucResult)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
			//Os__GB2312_display(1, 0, (uchar *) "等待接收数据...");
			//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
		}
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,"共计使用    秒",14);
		if( !ucResult )
		{
			util_Printf("\nBegin Receive Data");

			for(uiI=1;uiI<=uiTimeout;uiI++)
			{
				*puiOutLen=ISO8583_MAXCOMMBUFLEN;
				ucResult = TCP_Recv(iTCPHandle,aucReceiveBuff,puiOutLen,1);
				int_asc(aucBuf+10,2,(unsigned int*)&uiI);
				if(uiI==1)
					//Os__GB2312_display(1, 0, (uchar *) "数据接收中...");
				//Os__GB2312_display(3,0,aucBuf);
				util_Printf("超时时间2:%d\n",uiTimeout);
				//if (!uiTimeout)
				//{
				//	ucResult = ERR_COMMS_RECVTIMEOUT;
				//	break;
				//}
				if( !ucResult )
				{
					util_Printf("\nReceive Data Len = %d\n",*puiOutLen);
					if(*puiOutLen>2)
					{
						#if 0
							util_Printf("\n*********Receive Packet Data***********\n");
							for(iJ=0;iJ<*puiOutLen;iJ++)
							{
								if((uiI+1)%20==0) util_Printf("\n");
								util_Printf("%02x ",aucReceiveBuff[iJ]);
							}
							util_Printf("\n");
						#endif
						if(aucReceiveBuff[0]==0x60||aucReceiveBuff[0]==0x80)//first byte of APTU
						{
						        util_Printf("-----无长度返回的数据\n");
							memcpy(aucPackgeHead,&aucReceiveBuff[5],6);
							memcpy(pucOutData,&aucReceiveBuff[ucTPDULen+6],*puiOutLen-ucTPDULen-6);
							*puiOutLen-=(ucTPDULen+6);
						}
						else
						{
						       util_Printf("-----加长度返回的数据\n");
							memcpy(aucPackgeHead,&aucReceiveBuff[7],6);
							memcpy(pucOutData,&aucReceiveBuff[ucTPDULen+ucLLen+6],*puiOutLen-ucTPDULen-6-ucLLen);
							*puiOutLen -= (ucTPDULen+6+ucLLen);
						}
						for(iJ=0;iJ<5;iJ++)
						util_Printf("包头数据[%d]=%02x\n",iJ,aucPackgeHead[iJ]);
                                          DataSave0.ChangeParamData.ucPackgeHeadEve=aucPackgeHead[2]&0x0F;
                                          XDATA_Write_Vaild_File(DataSaveChange);
                                          util_Printf("数据包头处理值:%02x\n",DataSave0.ChangeParamData.ucPackgeHeadEve);
						break;
					}
				}
			}
		}
	}
	else
	{
		OSGSM_GprsReset();
	}
	if (!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "数据接收完成！");
		//Os__GB2312_display(1, 0, (uchar *) "正在处理中...");
		//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
	}

    TCP_Close(iTCPHandle);
    if( ucResult != SUCCESS && DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
    {
        MSG_DisplayNetworkErrMsg(ucResult);
        ucResult=ERR_END;
        util_Printf("[_END---CONNECT MODEL]===1===%02x\n",ucResult);
    }
    util_Printf("通讯结束处理码=1===%02x\n",ucResult);
	return(ucResult);
}


unsigned char COMMS_TCPIP_SendReceive_Encrypt(unsigned char *pucInData,unsigned short uiInLen,
                    unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char ucResult,ucIPAddress[5];	
	unsigned char ucLLen=2;
	unsigned char aucBuf[20];
	unsigned char aucSendBuff[ISO8583_MAXCOMMBUFLEN];
	unsigned char aucReceiveBuff[ISO8583_MAXCOMMBUFLEN];
	unsigned char aucTPDU[6];
	unsigned char aucPackgeHead[7];
	unsigned char ucTPDULen;
	unsigned char ucPackgeHeadLen;
	unsigned short  uiOutLen;
	unsigned short usSendLen;
	unsigned short usMBFELen;
	unsigned short uiTimeout;
	unsigned char aucMBFEBuf[20];
	int iTCPHandle,uiPacketLen;
    int result;

	int iJ;
	uint uiI=0;


	util_Printf("\n\n\n\n COMMS_TCPIP_SendReceive_Encrypt begin.....................\n");

	aucTPDU[0] = 0x60;
	aucTPDU[1] = DataSave0.ConstantParamData.aucTpdu[0];
	aucTPDU[2] = DataSave0.ConstantParamData.aucTpdu[1];
	aucTPDU[3] = 0x80;
	aucTPDU[4] = 0x00;
	
#ifdef GUI_PROJECT
//---
memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
ProUiFace.ProToUi.uiLines=1;
memcpy(ProUiFace.ProToUi.aucLine1,"正在连接",8);
//sleep(1);
        util_Printf("binsem1 1th wait wakeup+++++++++++++++++++++++++++");
result=sem_post(&binSem1);

if(result!=0)
{
    perror("error is +++++++++++++++++++++++++++*");
}
else
{
        util_Printf("binsem1 1th already wakeup+++++++++++++++++++++++++++");
}
sem_wait(&binSem2);
#endif

	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucPackgeHead,0,sizeof(aucPackgeHead));
	memcpy(aucBuf,"60",2);
	memcpy(&aucBuf[2],"22",2);
	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
		aucBuf[4] = '1';
	else
		aucBuf[4] = '0';

	memcpy(&aucBuf[5],"0000000",7);
	asc_bcd(aucPackgeHead,6,aucBuf,12);

    //Os__clr_display(255);
     if( G_RUNDATA_ucHostConnectFlag == false )
    {
        if(  (ucResult = COMMS_CheckPreComm()) != SUCCESS)
        {
            util_Printf("\nCOMMS_CheckPreComm %02x",ucResult);
            return(ucResult);
        }
    }
    // 设置通讯版本 1020为斯玛特通讯版本
    ucResult = COMMSENCRYPT_SetVersion("1020");
    if( ucResult != SUCCESS )
    {
    	util_Printf("COMMSENCRYPT_SetVersion ucResult = %02x\n",ucResult);
    	COMMSENCRYPT_ErrMsg(ucResult);
    	return ERR_CANCEL;
    }
    util_Printf("\n Before TCP_Open");
    ucResult=TCP_Open(&iTCPHandle);
    util_Printf("\n End TCP_Open ucResult:[%02x]",ucResult);
    if(ucResult!=SUCCESS) return ucResult;

	util_Printf("选择En-IP:%02x\n",DataSave0.ConstantParamData.ucSelIP);
	if (DataSave0.ConstantParamData.ucSelIP == 1)
	{
		memset(ucIPAddress,0x00,sizeof(ucIPAddress));
		memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulHostIP,sizeof(unsigned long));
		for(uiI=0;uiI<3;uiI++)
		{
			util_Printf("\nTCP/IP(Encrypt) =%d, TCP_Connect:%d.\n",uiI,DataSave0.ConstantParamData.uiHostPort);			
		
			ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiHostPort);
			util_Printf("主IP地址：\nucIPAddress1=%02x.%02x.%02x.%02x\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
			util_Printf("\n主端口=%d",DataSave0.ConstantParamData.uiHostBackPort);
			util_Printf("\n主[2008-04-30]Tcp_Connect Out ucResult: [%02x]  \n",ucResult);
			if (ucResult != SUCCESS && uiI==2)
			{
				DataSave0.ConstantParamData.ucSelIP = 2;
				XDATA_Write_Vaild_File(DataSaveConstant);
				if (DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
				{
					MSG_DisplayNetworkErrMsg(ucResult);
					ucResult = ERR_END;
#ifdef GUI_PROJECT
//--
memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
ProUiFace.ProToUi.uiLines=3;
memcpy(ProUiFace.ProToUi.aucLine1,"  主专线故障",12);
memcpy(ProUiFace.ProToUi.aucLine2,"  请报修",8);
memcpy(ProUiFace.ProToUi.aucLine3,"正切换备用线路IP",16);
//sleep(1);
        util_Printf("binsem1 2th wait wakeup+++++++++++++++++++++++++++");
        result=sem_post(&binSem1);
        if(result!=0)
        {
            perror("error is +++++++++++++++++++++++++++");
        }
        else
        {
                util_Printf("binsem1 2th already wakeup+++++++++++++++++++++++++++");
        }
sem_wait(&binSem2);
//--
memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
ProUiFace.ProToUi.uiLines=2;
memcpy(ProUiFace.ProToUi.aucLine1,"正在连接网络...",15);
memcpy(ProUiFace.ProToUi.aucLine2,"    请稍等...",13);
//sleep(1);
        util_Printf("binsem1 3th wait wakeup+++++++++++++++++++++++++++");
        result=sem_post(&binSem1);

        if(result!=0)
        {
            perror("error is +++++++++++++++++++++++++++");
        }
        else
        {
                util_Printf("binsem1 3th already wakeup+++++++++++++++++++++++++++");
        }
sem_wait(&binSem2);
#else
					//Os__clr_display(255);
					//Os__GB2312_display(0, 1, (uchar *) " 主专线故障");
					//Os__GB2312_display(1, 2, (uchar *) " 请报修");
					//Os__GB2312_display(2, 0, (uchar *) "正切换备用线路IP");
					MSG_WaitKey(3);
					//Os__clr_display(255);
					//Os__GB2312_display(0, 0, (uchar *) "正在连接网络...");
					//Os__GB2312_display(2, 0, (uchar *) "    请稍等...");
#endif					
				}
				break;
			}
			else
			{
				if (!ucResult) break;
			}
		}
	}
	util_Printf("选择Ep-IP:%02x\n",DataSave0.ConstantParamData.ucSelIP);
	if (DataSave0.ConstantParamData.ucSelIP == 2)
	{
		util_Printf("\nTCP/IP(Encrypt)-Out[uiI]=%d\n",uiI);
		memset(ucIPAddress,0x00,sizeof(ucIPAddress));
		memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulBackIP,sizeof(unsigned long));
		for(uiI=0;uiI<3;uiI++)
		{
			util_Printf("\nTCP/IP(Encrypt) =%d, TCP_Connect:%d.\n",uiI,DataSave0.ConstantParamData.uiHostBackPort);			
			ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiHostBackPort);
			util_Printf("备份IP地址：\nucIPAddress1=%02x.%02x.%02x.%02x\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
			util_Printf("\n备份端口=%d",DataSave0.ConstantParamData.uiHostBackPort);
			util_Printf("\n备份[2008-04-30]Tcp_Connect Out ucResult: [%02x]  \n",ucResult);
			if (ucResult != SUCCESS && uiI==2)
			{
				DataSave0.ConstantParamData.ucSelIP = 1;
				XDATA_Write_Vaild_File(DataSaveConstant);
				if (DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
				{
					MSG_DisplayNetworkErrMsg(ucResult);
					ucResult = ERR_END;
				}
				break;
			}
			else
			{
				if (!ucResult) break;
			}
		}
	}
	util_Printf("网络加密该交易存在冲正交易:%02x\n",G_RUNDATA_ucReversalFlag);
	if (!ucResult && G_RUNDATA_ucReversalFlag == 0x31)
	{
		G_RUNDATA_bConectFlag=true;
		util_Printf("Set冲正标志前=%02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
		Os__saved_set((unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid),
						DATASAVEVALIDFLAG,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveTrans8583);
		util_Printf("Set冲正标志后= %02x\n",DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid);
	}
       if( !ucResult )
       {
		util_Printf("\nConnect Server OK SEND DATA ing........\n");
#ifdef GUI_PROJECT
	//--		
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=3;	
	memcpy(ProUiFace.ProToUi.aucLine1,"已连接数据中心",14);
	memcpy(ProUiFace.ProToUi.aucLine2,"数据发送中...",13);
	memcpy(ProUiFace.ProToUi.aucLine3,"请稍候...",9);
	ProUiFace.ucProBar=1;
//sleep(1);
            util_Printf("binsem1 4th wait wakeup+++++++++++++++++++++++++++");
            result=sem_post(&binSem1);
            if(result!=0)
            {
                perror("error is +++++++++++++++++++++++++++");
            }
            else
            {
                    util_Printf("binsem1 4th already wakeup+++++++++++++++++++++++++++");
            }
    sem_wait(&binSem2);
#else
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
		//Os__GB2312_display(1, 0, (uchar *) "数据发送中...");
		//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
#endif
		memset(aucSendBuff,0x00,sizeof(aucSendBuff));
		usSendLen = 0;
		ucTPDULen=sizeof(aucTPDU)-1;
		ucPackgeHeadLen = sizeof(aucPackgeHead)-1;

		if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
		{
		    usMBFELen = ISO8583_MBFE.uiCommBufLen;
		    util_Printf("新加前置IS08583数据长度:%d\n",usMBFELen);
                  if ((DataSave0.ConstantParamData.ucSendOrder==0x31)
                     &&(G_NORMALTRANS_ucTransType==TRANS_PURCHASE
        		      ||G_NORMALTRANS_ucTransType==TRANS_VOIDPURCHASE
        		      ||G_NORMALTRANS_ucTransType==TRANS_REVERSAL)
        		&& (usMBFELen != 0)
		        )
		    {
        		 uiPacketLen=ucTPDULen+uiInLen+ucPackgeHeadLen+2+usMBFELen+12;
		    }
		    else
		    {
		        uiPacketLen=ucTPDULen+uiInLen+ucPackgeHeadLen+2+12;
		    }
		}
		else
		    uiPacketLen=ucTPDULen+uiInLen+ucPackgeHeadLen;

		usSendLen = uiPacketLen;
		CONV_IntHex(aucSendBuff,2,&uiPacketLen);

		if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
          {
            memcpy(aucSendBuff+2,aucTPDU,ucTPDULen);
            memcpy(aucSendBuff+2+ucTPDULen,"\xAA\xBD",2);
            memcpy(aucSendBuff+2+ucTPDULen+2,aucPackgeHead,ucPackgeHeadLen);
            memcpy(aucSendBuff+2+ucTPDULen+2+ucPackgeHeadLen,pucInData,uiInLen);
          }
          else
          {
              memcpy(aucSendBuff+2,aucTPDU,ucTPDULen);
              memcpy(aucSendBuff+2+ucTPDULen,aucPackgeHead,ucPackgeHeadLen);
              memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen,pucInData,uiInLen);
         }

		if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
		{
                  if ((DataSave0.ConstantParamData.ucSendOrder==0x31)
                    && (G_NORMALTRANS_ucTransType==TRANS_PURCHASE
		        ||G_NORMALTRANS_ucTransType==TRANS_VOIDPURCHASE
		        ||G_NORMALTRANS_ucTransType == TRANS_REVERSAL)
		        )
		    {
		       util_Printf("给前置送的数据:\n");
		       for(uiI=0;uiI<usMBFELen;uiI++)
		            util_Printf("%02x ",ISO8583_MBFE.aucCommBuf[uiI]);
		       util_Printf("\n");
		       memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen,ISO8583_MBFE.aucCommBuf,usMBFELen);
        		memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen+usMBFELen,MBFE_BUF,10);

        		memset(aucMBFEBuf,0,sizeof(aucMBFEBuf));
        		CONV_IntHex(aucMBFEBuf,2,&usMBFELen);
        		memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen+usMBFELen+10,aucMBFEBuf,2);
        	    }
        	    else
        	        memcpy(aucSendBuff+2+ucTPDULen+ucPackgeHeadLen+2+uiInLen,MBFE_LBUF,12);
		}
#if 0
            util_Printf("\n*******Send  Packet Data*******\n");
            if (DataSave0.ConstantParamData.ucMBFEFlag==0x31)   //需要前置转发
            {
                util_Printf("前置转发数据\n");
                for(uiI=0;uiI<uiPacketLen+2;uiI++)
                {
                    if (!((uiI+1)%20)) util_Printf("\n");
                    util_Printf("%02x ",aucSendBuff[uiI]);
                }
            }else
            {
                util_Printf("银联数据\n");
                for(uiI=0;uiI<uiPacketLen+2;uiI++)
                {
                    if (!((uiI+1)%20)) util_Printf("\n");
                    util_Printf("%02x ",aucSendBuff[uiI]);
                }
            }
#endif
		util_Printf("\nBegin Send Data");
/*Add at 07-12-10*/
		memset(aucEncryptData,0,sizeof(aucEncryptData));
		memcpy(aucEncryptData,aucSendBuff,usSendLen+2);
		ucResult=DataEncrypt(aucEncryptData,usSendLen+2,DataSave0.ConstantParamData.aucTerminalID,
										aucEncryptData,&uiOutLen);
		{
			int i;
			util_Printf("usSendLen=========%d ;uiOutLen=%d\n",usSendLen,uiOutLen);
			for( i=0;i<usSendLen;i++)
				util_Printf("%02x ",aucSendBuff[i]);
			util_Printf("\n");
		}

		if(ucResult!=SUCCESS)
		{
			COMMSENCRYPT_ErrMsg(ucResult);
			ucResult = ERR_CANCEL;
		}
/*End*/
		ucResult=TCP_Send(iTCPHandle,aucEncryptData,uiOutLen);
		util_Printf("\nSend Data End ucResult:[%02x]",ucResult);

		uiTimeout = DataSave0.ConstantParamData.uiReceiveTimeout;
		if (!ucResult)
		{

//--		
util_Printf("\n.jianggs---------[001].");
#ifdef GUI_PROJECT
			memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
			ProUiFace.ProToUi.uiLines=3;
			memcpy(ProUiFace.ProToUi.aucLine1,"已连接数据中心",14);
			memcpy(ProUiFace.ProToUi.aucLine2,"数据接收中...",13);
			memcpy(ProUiFace.ProToUi.aucLine3,"请稍候...",9);			
			ProUiFace.ucProBar=2;
			util_Printf("\n.jianggs---------[002].");	
//sleep(1);
            util_Printf("the 5th binSem1 wait wakeup+++++++++++++++++++++++++++++++++++++++++");
            result=sem_post(&binSem1);
            if(result!=0)
            {
                perror("error is +++++++++++++++++++++++++++");
            }
            else
            {
                    util_Printf("binsem1 5th already wakeup+++++++++++++++++++++++++++");
            }

            sem_wait(&binSem2);
#else
			//Os__clr_display(255);
			//Os__GB2312_display(0, 0, (uchar *) "已连接数据中心");
			//Os__GB2312_display(1, 0, (uchar *) "等待接收数据...");
			//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
#endif
		}
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,"共计使用    秒",14);
		if( !ucResult )
		{
			util_Printf("\nBegin Receive Data");
			for(uiI=1;uiI<=50;uiI++)
			{
//--		
util_Printf("\n.jianggs---------[003].");	
#ifdef GUI_PROJECT
memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
ProUiFace.ProToUi.uiLines=1;
int_asc(ProUiFace.ProToUi.aucLine1,2,&uiI);
ProUiFace.uiProgress=uiI;
util_Printf("\n.jianggs---------[004].");	
//sleep(1);
            util_Printf("the 6th binSem1 wait wakeup+++++++++++++++++++++++++++++++++++++++++");
            result=sem_post(&binSem1);
            if(result!=0)
            {
                perror("error is +++++++++++++++++++++++++++");
            }
            else
            {
                    util_Printf("binsem1 6th already wakeup+++++++++++++++++++++++++++");
            }

sem_wait(&binSem2);
#endif
				uiOutLen=ISO8583_MAXCOMMBUFLEN;
				ucResult = TCP_Recv(iTCPHandle,aucEncryptData,&uiOutLen ,1);
				util_Printf("TCP_Recv()----ucResult=%02x\n",ucResult);
#ifndef GUI_PROJECT
				int_asc(aucBuf+10,2,(unsigned int*)&uiI);
				if(uiI==1)
				{
					//Os__clr_display(1);
					//Os__GB2312_display(1, 0, (uchar *) "数据接收中...");
				}
				//Os__GB2312_display(3,0,aucBuf);
#endif				
/*add 07*/
				if( ucResult == SUCCESS )
				{
					ucResult = DataDecrypt(aucEncryptData,aucEncryptData,&uiOutLen);
					if( ucResult == SUCCESS )
					{
						// 例子中不按照具体的报文进行处理，程序需针对实际数据进行相应处理
						memcpy(aucReceiveBuff,&aucEncryptData[0],uiOutLen);
					}
					else
					{
						COMMSENCRYPT_ErrMsg(ucResult);
						ucResult = ERR_CANCEL;
					}
				}
/*end*/
				if( !ucResult )
				{
					util_Printf("\nReceive Data Len = %d\n",uiOutLen);
					*puiOutLen = uiOutLen;
					if(uiOutLen>2)
					{
						#if 0
							util_Printf("\n*********Receive Packet Data***********\n");
							for(iJ=0;iJ<uiOutLen;iJ++)
							{
								if((uiI+1)%20==0) util_Printf("\n");
								util_Printf("%02x ",aucReceiveBuff[iJ]);
							}
							util_Printf("***************************END****************************\n");
						#endif
						if(aucReceiveBuff[0]==0x60||aucReceiveBuff[0]==0x80)//first byte of APTU
						{
							util_Printf("-----无长度返回的数据\n");
							memcpy(aucPackgeHead,&aucReceiveBuff[5],6);
							memcpy(pucOutData,&aucReceiveBuff[ucTPDULen+6],uiOutLen-ucTPDULen-6);
							*puiOutLen-=(ucTPDULen+6);
						}
						else
						{
							util_Printf("-----加长度返回的数据\n");
							memcpy(aucPackgeHead,&aucReceiveBuff[7],6);
							memcpy(pucOutData,&aucReceiveBuff[ucTPDULen+ucLLen+6],uiOutLen-ucTPDULen-6-ucLLen);
							*puiOutLen -= (ucTPDULen+6+ucLLen);
						}

						//for(iJ=0;iJ<5;iJ++)
						//	util_Printf("包头数据[%d]=%02x\n",iJ,aucPackgeHead[iJ]);
						DataSave0.ChangeParamData.ucPackgeHeadEve=aucPackgeHead[2]&0x0F;
						XDATA_Write_Vaild_File(DataSaveChange);
						util_Printf("数据包头处理值:%02x\n",DataSave0.ChangeParamData.ucPackgeHeadEve);

						#if 0
							util_Printf("\n*********Receive Packet Data********pucOutData***puiOutLen=%d\n",*puiOutLen);
							for(iJ=0;iJ<*puiOutLen;iJ++)
							{
								if((uiI+1)%20==0) util_Printf("\n");
								util_Printf("%02x ",*(pucOutData+iJ));
							}
							util_Printf("********************END***********************\n");
						#endif
						break;
					}
				}
			}
		}
	}
	else
	{
		OSGSM_GprsReset();
	}
	util_Printf("\n.jianggs---------[005].");		
	if (!ucResult)
	{
		util_Printf("\n.jianggs---------[006].");	
#ifdef GUI_PROJECT		
		//--		
		util_Printf("\n9. slottimer1Done uiProgress:%d",ProUiFace.uiProgress);
		ProUiFace.uiProgress = 50;	
		memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
		ProUiFace.ProToUi.uiLines=3;
		memcpy(ProUiFace.ProToUi.aucLine1,"数据接收完成！",14);
		memcpy(ProUiFace.ProToUi.aucLine2,"正在处理中...",13);
		memcpy(ProUiFace.ProToUi.aucLine3,"请稍候...",9);		
//sleep(1);
        util_Printf("binsem1 7th wait wakeup+++++++++++++++++++++++++++");
        result=sem_post(&binSem1);
        if(result!=0)
        {
            perror("error is +++++++++++++++++++++++++++");
        }
        else
        {
                       util_Printf("binsem1 7th already wakeup+++++++++++++++++++++++++++");
        }

        sem_wait(&binSem2);
#else
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (uchar *) "数据接收完成！");
		//Os__GB2312_display(1, 0, (uchar *) "正在处理中...");
		//Os__GB2312_display(2, 0, (uchar *) "请稍候...");
#endif
	}
	util_Printf("\n.jianggs---------[007].");		
	TCP_Close(iTCPHandle);
	if( ucResult != SUCCESS && DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
	{
		MSG_DisplayNetworkErrMsg(ucResult);
		ucResult=ERR_END;
		util_Printf("[_END---CONNECT MODEL]===1===%02x\n",ucResult);
	}
	util_Printf("通讯结束处理码=2===%02x\n",ucResult);
	return(ucResult);
}
unsigned char COMMS_KEYDOWNReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout)
{
	unsigned short ucResult,ucReCOMM;

	//util_Printf("通讯:=[%02x]\n",xDATA_Constant.ucKEYCOMM);
#ifdef tCOMMPRINT
	memset(aucPRINTBUF,0,sizeof(aucPRINTBUF));
	memcpy(aucPRINTBUF,"回复通讯:",9);
	hex_asc(&aucPRINTBUF[9],&xDATA_Constant.ucKEYCOMM,2);
	PRINT_GB2312_xprint(aucPRINTBUF,0x1d);
#endif
	if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
		ucResult = Os__rxcar3(*puiTimeout);
	else
		ucResult = Os__rxcar(*puiTimeout);
#ifdef tCOMMPRINT
	memset(aucPRINTBUF,0,sizeof(aucPRINTBUF));
	memcpy(aucPRINTBUF,"等待回复:",9);
	hex_asc(&aucPRINTBUF[9],&ucResult,2);
	PRINT_GB2312_xprint(aucPRINTBUF,0x1d);
#endif
	switch	( ucResult / 256 )
	{
		case OK :
			*pucByte = ucResult % 256;
			ucReCOMM = OK;
			break;
		case COM_ERR_TO:
			ucReCOMM = ERR_COMMS_RECVTIMEOUT;
			break;
		default :
			util_Printf("\nRecv CHAR %d\n",ucResult / 256);
			ucReCOMM = ERR_COMMS_RECVCHAR;
			break;
	}
	return (ucReCOMM);
}
unsigned char COMMS_ReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout)
{
	unsigned short ucResult,ucReCOMM;

#ifdef COMM_COM3
		ucResult = Os__rxcar3(*puiTimeout);
#else
		ucResult = Os__rxcar(*puiTimeout);
#endif

	switch	( ucResult / 256 )
	{
		case OK :
			*pucByte = ucResult % 256;
			ucReCOMM = OK;
			break;
		case COM_ERR_TO:
			ucReCOMM = ERR_COMMS_RECVTIMEOUT;
			break;
		default :
			util_Printf("\nRecv CHAR %d\n",ucResult / 256);
			ucReCOMM = ERR_COMMS_RECVCHAR;
			break;
	}
	return (ucReCOMM);
}

unsigned char COMMS_FinComm(void)
{
	#if PS300
	if(G_RUNDATA_ucHostConnectFlag)
		COMMS_CheckBase();
	#endif
	util_Printf("\nCOMMS_FinComm");
	switch( DataSave0.ConstantParamData.ucCommMode )
	{
		case PARAM_COMMMODE_MODEM:
		case PARAM_COMMMODE_HDLC:
			util_Printf("    hang up step \n");
			Os__hang_up();
			//OSMODEM_Hangup();
			break;
		case PARAM_COMMMODE_GPRS:
			 OSGSM_GprsHangupDial();
			break;
		default:
			if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
				OSUART_Close2();
			util_Printf("    OSUART_Close2\n");
			break;
	}
	G_RUNDATA_ucHostConnectFlag = false;
	return(SUCCESS);
}

unsigned char KEY_COMMS_FinComm(void)
{
	#if PS300
	if(G_RUNDATA_ucHostConnectFlag)
		COMMS_CheckBase();
	#endif
	util_Printf("\nCOMMS_FinComm");
	switch( DataSave0.ConstantParamData.ucKEYCOMM )
	{
		case PARAM_DOWNKEYCOMM_HDLC:
			util_Printf("    hang up step \n");
			Os__hang_up();
			break;
		case PARAM_DOWNKEYCOMM_GPRS:
			 OSGSM_GprsHangupDial();
			break;
		default:
			if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
				OSUART_Close2();
			util_Printf("    OSUART_Close2\n");
			break;
	}
	G_RUNDATA_ucHostConnectFlag = false;
	return(SUCCESS);
}

unsigned char COMMS_SendCollectDataTCP(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	unsigned char ucResult,ucIPAddress[5];
	int iTCPHandle;
	unsigned int  uiI;
	unsigned char ucBuf[20];

	unsigned char aucTPDU[]={ 0x60,0x00,0x03,0x80,0x00,0xCA,0xCA,0x31,0x31};
	unsigned short uiLen=0;
	unsigned char aucCommuBuf[512];
	unsigned short OutLen=0;
	unsigned char i;

	util_Printf("\n 需上送数据长度uiInLen :[%3d]\n",uiInLen);
	ucResult=COMMS_CheckPreComm();
	if (ucResult)	return(ucResult);
    ucResult = COMMSENCRYPT_SetVersion("1020");
    if( ucResult != SUCCESS )
    {
    	util_Printf("COMMSENCRYPT_SetVersion ucResult = %02x\n",ucResult);
    	COMMSENCRYPT_ErrMsg(ucResult);
    	return ERR_CANCEL;
    }
    ucResult=TCP_Open(&iTCPHandle);
    util_Printf("\n End TCP_Open ucResult:[%02x]",ucResult);
    if(ucResult!=SUCCESS) return ucResult;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"正在连接采集中心");
	//Os__GB2312_display(1,0,(uchar *)"请稍候.....");

	memset(ucIPAddress,0x00,sizeof(ucIPAddress));
	memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulSendIPAddress,sizeof(unsigned long));
	util_Printf("\n ucIPAddress = %03d.%03d.%03d.%03d \n ",
					ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);

	util_Printf("\n uiHostPort = %04d",DataSave0.ConstantParamData.uiSendPort);
	//由于外网的不稳定性，最大连接三次
	for(i=0;i<3;i++)
	{
		util_Printf("\nTCP/IP(Encrypt) =%d, TCP_Connect:%d.\n",uiI,DataSave0.ConstantParamData.uiSendPort);				
		ucResult = TCP_Connect(iTCPHandle,tab_long(ucIPAddress,4),DataSave0.ConstantParamData.uiSendPort);
		if(!ucResult)		break;
	}

	util_Printf("\n Tcp_Connect Out ucResult: [%02x]  \n",ucResult);
    if( !ucResult )
    {
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"已连接采集中心");
		//Os__GB2312_display(1,0,(uchar *)"数据发送中");
		//Os__GB2312_display(2,0,(uchar *)"请稍候.....");
		//后台要求送tpdu跟整个包长(两个字节的bcd码)
		memset(aucCommuBuf,0,sizeof(aucCommuBuf));
		memset(ucBuf,0,sizeof(ucBuf));
		uiLen=uiInLen+9;
		util_Printf("uiLen=%d\n",uiLen);
		int_bcd(&aucCommuBuf[0],2,&uiLen);
		memcpy(&aucCommuBuf[2],aucTPDU,9); //8
		memcpy(&aucCommuBuf[11],pucInData,uiInLen);//10

#if	0
    	util_Printf("\nConnect Server OK");

		util_Printf("\n*******Send  Packet Data*******\n");
		for(uiI=0;uiI<uiLen+2;uiI++)
		{
			if(uiI%20==0) util_Printf("\n");
			util_Printf("%02x ",aucCommuBuf[uiI]);
		}
#endif

		util_Printf("\nBegin Send Data");
		ucResult=DataEncrypt(aucCommuBuf,(uiInLen+11),DataSave0.ConstantParamData.aucTerminalID,
										aucCommuBuf,&OutLen);

		if(ucResult!=SUCCESS)
		{
			COMMSENCRYPT_ErrMsg(ucResult);
			ucResult = ERR_CANCEL;
		}

#if	0
		util_Printf("Data after Encrypt.\n");
		for(uiI=0;uiI<OutLen;uiI++)
		{
			if(uiI%20==0) util_Printf("\n");
			util_Printf("%02x ",aucCommuBuf[uiI]);
		}
#endif

		ucResult=TCP_Send(iTCPHandle,aucCommuBuf,OutLen);

		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"采集数据发送成功");
		//Os__GB2312_display(1,0,(uchar *)"正在接收返回");
		//Os__GB2312_display(2,0,(uchar *)"请稍候.....");

		memset(ucBuf,0x00,sizeof(ucBuf));
		memset(aucCommuBuf,0,sizeof(aucCommuBuf));
		memcpy(ucBuf,"共计使用    秒",14);
		if( !ucResult )
		{
				for(uiI=1;uiI<=60;uiI++)
				{
					ucResult = TCP_Recv(iTCPHandle,aucCommuBuf,&OutLen,1);
					int_asc(ucBuf+10,2,&uiI);
					//Os__GB2312_display(1,0,ucBuf);
					if(!ucResult) break;

				}
/*
				util_Printf("\n*********Receive Packet Data***********\n");
				util_Printf("\nReceive Data Len befor Decrypt= %d\n",OutLen);
				for(uiI=0;uiI<OutLen;uiI++)
				{
					if(uiI%20==0) util_Printf("\n");
					util_Printf("%02x ",aucCommuBuf[uiI]);
				}
*/
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"采集数据接收成功");
				//Os__GB2312_display(1,0,(uchar *)"正在处理");
				//Os__GB2312_display(2,0,(uchar *)"请稍候.....");

				ucResult = DataDecrypt(aucCommuBuf,aucCommuBuf,&OutLen);
				*puiOutLen=OutLen;
				if (*puiOutLen==0)
					return(EMV_ERRDATALEN);

				util_Printf("OutLen=%d\n",OutLen);
				util_Printf("\nReceive Data End ucResult:[%02x]",ucResult);

				if( !ucResult )
				{
					*puiOutLen=OutLen-8;
					memcpy(pucOutData,&aucCommuBuf[8],*puiOutLen);
					util_Printf("\nReceive Data Len = %d\n",*puiOutLen);
					for(uiI=0;uiI<*puiOutLen;uiI++)
					{
						if((uiI+1)%20==0) util_Printf("\n");
						util_Printf("%02x ",pucOutData[uiI]);
					}
					util_Printf("\n");
				}else
				{
					COMMSENCRYPT_ErrMsg(ucResult);
					ucResult = ERR_CANCEL;
				}
		}
	}
   else
   		OSGSM_GprsReset();

	if(!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,1,(unsigned char *)"正在关闭连接");
	}
	TCP_Close(iTCPHandle);
	util_Printf("\n End TCP_Close%02x\n",ucResult);
	return(ucResult);
}
