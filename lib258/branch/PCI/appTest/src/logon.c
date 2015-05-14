#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <fsync_drvs.h>
#include <menu.h>

unsigned char LOGON_Online(void)
{
	unsigned char ucResult,ucI;
	unsigned char aucBuf[200];
	unsigned char aucDate[7];
	unsigned char aucTime[5],ucRIDIndex,ucAIDIndex,ucCAPKIndex;
	unsigned char aucTmpBuf[20],ucContinueFlag=false;
	NEW_DRV_TYPE new_drv;
	short iLen;

	

	G_NORMALTRANS_ucTransType = TRANS_LOGONON;
	clear_display();
	ucResult = SUCCESS;

	ISO8583_Clear();
	
	ISO8583_SetMsgID(800);
	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&DataSave0.ChangeParamData.ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber();
	   
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
	
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,"00",2);
	long_asc(&aucBuf[2],6,&DataSave0.ChangeParamData.ulBatchNumber);
	memcpy(&aucBuf[8],"001",3);
	ISO8583_SetBitValue(60,aucBuf,11);
	
	
	ISO8583_SetBitValue(63,aucBuf,3);

	ucResult = COMMS_GenSendReceive();
	if(ucResult == SUCCESS)
	{
	        memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(12,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult == SUCCESS)
		{
			memset(aucTime,0,sizeof(aucTime));
			memcpy(aucTime,aucBuf,4);
		}
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(13,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult == SUCCESS)
		{
                     Os__read_date(aucDate);
			memcpy(&aucDate[0],&aucBuf[2],2);
			memcpy(&aucDate[2],&aucBuf[0],2);
			Os__write_date(aucDate); 
			Os__write_time(aucTime);
		}
		
	}
	else 
	{
		clear_display();
                Os__GB2312_display(4,3,(uchar *)"终端无反应！\r\n");
		Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		return ucResult;

	}
	
	if( ucResult == SUCCESS)
	{
	        ucResult = UTIL_Single_StoreNewKey();
	}

	if( ucResult == SUCCESS)
	{
	        DataSave0.ChangeParamData.ucCashierLogonFlag=0x55;
		XDATA_Write_Vaild_File(DataSaveChange);
		clear_display();
	    	Os__GB2312_display(4,3,(uchar *)"签到成功！\r\n");
                Os_Wait_Event(KEY_DRV, &new_drv, 100000);
		//MSG_WaitKey(4);
	}
	else
	{
		clear_display();
                Os__GB2312_display(4,3,(uchar *)"密钥保存失败！\r\n");
                Os_Wait_Event(KEY_DRV, &new_drv, 100000);
                return ucResult;
	}
	
	return(ucResult);
}
