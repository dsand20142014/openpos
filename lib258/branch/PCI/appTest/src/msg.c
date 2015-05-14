#include <include.h>
#include <Global.h>
#include <display.h>
#include <xdata.h>
#include <fsync_drvs.h>


unsigned char MSG_DisplayErrMsg(unsigned char ucErrorCode)
{
	unsigned char 	aucBuf[MSG_MAXCHAR+1];
	unsigned char 	ucType;
	unsigned short 	uiIndex1;
	unsigned char 	aucErrorCode[4];
	NEW_DRV_TYPE new_drv;

	UTIL_Beep();

	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));

	if(ucErrorCode == ERR_HOSTCODE)
	{
		ucErrorCode = (unsigned char)G_RUNDATA_ucErrorExtCode;
		for(uiIndex1=0;HostErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
		{
			if( ucErrorCode == HostErrMsg[uiIndex1].ucErrCode)
			{
				ucType = HostErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				break;
			}
		}	
	  if(HostErrMsg[uiIndex1].ucErrCode == 0xff) 
		{
			  ucType = HostErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
		}		
	}else
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
	}		
	
	if(ucErrorCode>0x63)
	{
		long_asc(aucErrorCode,3,(unsigned long *)&ucErrorCode);
		aucErrorCode[3]=0x00;
	}else if(ucErrorCode>0x09)
	{
		long_asc(aucErrorCode,2,(unsigned long *)&ucErrorCode);
		aucErrorCode[2]=0x00;	
	}else
	{
		long_asc(aucErrorCode,1,(unsigned long *)&ucErrorCode);
		aucErrorCode[1]=0x00;	
	}
	Os__clr_display(0);
	Os__clr_display(1);
	Os__clr_display(2);
	Os__clr_display(3);
	Os__clr_display(4);
	Os__clr_display(5);
	Os__clr_display(6);
	//Os__GB2312_display(0, 0, (uchar *)(aucBuf));
    	//Os__GB2312_display(1, 0, (uchar *)&aucBuf[16]);
	Os__GB2312_display(2,0,(uchar *)"返回码：");
    	Os__display(2,9,(uchar *)aucErrorCode);
	//MSG_WaitKey(4);
	Os_Wait_Event(KEY_DRV, &new_drv, 150000);
   	return(SUCCESS);
}

unsigned char MSG_WaitKey(unsigned short uiTimeout)
{
	DRV_OUT 		*pdKey;
	NEW_DRV_TYPE new_drv;
	unsigned int  uiSecond;
	unsigned char	ucKey;

    if( uiTimeout != 0 )
    {
		uiSecond = uiTimeout*100;
		Os__timer_start(&uiSecond);
		Os_Wait_Event(KEY_DRV, &new_drv, 150000);
		pdKey = &(new_drv.drv_data);
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
	Os_Wait_Event(KEY_DRV, &new_drv, 150000);
	pdKey = &(new_drv.drv_data);
	do{
	}while(pdKey->gen_status == DRV_RUNNING);
	if( pdKey->gen_status == DRV_ENDED)
	{
		ucKey = pdKey->xxdata[1];
	}	    
    }
   Os__abort_drv(drv_mmi);
    return(ucKey);
}




