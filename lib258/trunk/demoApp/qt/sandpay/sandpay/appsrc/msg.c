/* 
	SAND China
--------------------------------------------------------------------------
	FILE  msg.c									(Copyright SAND 2001)       
--------------------------------------------------------------------------
    INTRODUCTION          
    ============                                             
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network             
    Purpose :                               
        Source file for ISO8583 package process. 
                                                             
    List of routines in file :                                            
                                                                          
    File history :                                                        
                                                                          
*/

#include <include.h>
#include <global.h>
#include <display.h>
#include <xdata.h>
#include <osgraph.h>
#include <new_drv.h>

unsigned char   Os__xget_key(void)
{
	unsigned int ret;            
	unsigned char ucKey;
	NEW_DRV_TYPE  new_drv;                                                    

    Uart_Printf("\n1. Os_Wait_Os__xget_key Event begin...");
	ret = Os_Wait_Event(KEY_DRV  , &new_drv, 0);
	//Uart_Printf("\n2. Os_Wait_Event:%02x",ret);
	//Uart_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
	if(ret == 0)                                                              
	{                                                                         
		if(new_drv.drv_type == KEY_DRV)                                     
		{                                                                   
			if(new_drv.drv_data.gen_status== DRV_ENDED){
				ucKey=new_drv.drv_data.xxdata[1];
				Uart_Printf("\n ucKey:%02x.", ucKey);				
			}
		}                                                                   			
	} else                                                                      
	{                                                                         
		Untouch_OWE_RstErrInfo(ret);                                               
	}                                                                         
	
	Uart_Printf("\n OWE_NewDrvtts rest:%02x.\n",ret);
	return ucKey;
}	
unsigned char MSG_WaitKey(unsigned short uiTimeout)
{
	DRV_OUT 		*pdKey;
    unsigned int  uiSecond;
	unsigned char	ucKey;

	NEW_DRV_TYPE  new_drv;
	uchar ucResult=0;

    return KEY_ENTER;
	
	Uart_Printf("\n1. MSG_WaitKey Os_Wait_Event begin.uiTimeout:%d",uiTimeout);
	
	ucResult = Os_Wait_Event(KEY_DRV , &new_drv, 1000*uiTimeout);
	
	Uart_Printf("\n2. Os_Wait_Event ucResult:%02x",ucResult);
	//Uart_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
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
		Uart_Printf("\n OWE_NewDrvtts End.....\n");
	}else
	{
	}

    Uart_Printf("\nucKey=%02x\n",ucKey);
    return(ucKey);
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


void MSG_DisplayMsg( unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned int uiIndex)
{
	unsigned char aucMsg[MSG_MAXCHAR+1];

    if( ucClrDisp )
    {
        //Os__clr_display(255);
/*        //Os__clr_display(ucLine);*/
    }
    memset(aucMsg,0,sizeof(aucMsg));
	MSG_GetMsg(uiIndex,aucMsg,sizeof(aucMsg));
    
//    switch( DataSave0.MSGData.aucMSGType[uiIndex])
    switch( MSGTAB[uiIndex].aucMSGType)
    {
    case MSG_TYPEZHCN:
    	//Os__GB2312_display(ucLine, ucCol/2, (uchar *)(aucMsg));
    	break;
    default:
    	Os__display(ucLine,ucCol,aucMsg);
    	break;
    }

	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;
	memcpy(ProUiFace.ProToUi.aucLine1,aucMsg,strlen((char*)aucMsg));
	Os__xdelay(GUI_DELAY_TIMEOUT);

}


unsigned char MSG_DisplayErrMsgNotWait(unsigned char ucErrorCode)
{
	unsigned char aucBuf[MSG_MAXCHAR+1];
	unsigned char ucType;
	unsigned short uiIndex1;

	ucType = 0x30;
	memset(aucBuf,0,sizeof(aucBuf));
	if(ucErrorCode == ERR_HOSTCODE)
	{
		ucErrorCode = (unsigned char)RUNDATA_ucErrorExtCode;
		for(uiIndex1=0;uiIndex1<MSG_MAXHOSTMSG;uiIndex1++)
		{
			if( ucErrorCode == HostErrMsg[uiIndex1].ucErrCode)
			{
				ucType = HostErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				break;
			}
		}
		if( HostErrMsg[uiIndex1].ucErrCode == MSG_MAXHOSTMSG)
		{
		     ucType = HostErrMsg[uiIndex1].ucType;
		     memcpy(aucBuf,HostErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
	       }

	}else
	{	
		for(uiIndex1=0;uiIndex1<256;uiIndex1++)
		{
			if( ucErrorCode == ErrMsg[uiIndex1].ucErrCode)
			{
				ucType = ErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,ErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				break;
			}
		}
	}		
    //Os__clr_display(255);
    switch( ucType)
    {
    case MSG_TYPEZHCN:
    	//Os__GB2312_display(0, 0, (uchar *)(aucBuf));
    	break;
    default:
    	Os__display(0,0,aucBuf);
    	break;
    }
memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
ProUiFace.ProToUi.uiLines=1;
memcpy(ProUiFace.ProToUi.aucLine1,aucBuf,strlen((char*)aucBuf));
sleep(1);
	
	//MSG_WaitKey(4);
   	return(SUCCESS);
}

/*******************************************************************************************/
unsigned char MSG_DisplayMsgKey(unsigned char ucClrDisp,
					unsigned char ucLine,
					unsigned char ucCol,
					unsigned short uiIndex,
					unsigned short uiTimeout)
{
	unsigned char	aucMsg[MSG_MAXCHAR+1];
	
    if( ucClrDisp )
    {
        //Os__clr_display(ucLine);
    }
    
    memset(aucMsg,0,sizeof(aucMsg));
	MSG_GetMsg(uiIndex,aucMsg,sizeof(aucMsg));
    

    switch( MSGTAB[uiIndex].aucMSGType)
    {
    case MSG_TYPEZHCN:
    	//Os__GB2312_display(ucLine, ucCol/2, (uchar *)(aucMsg));

     
    	break;
    default:
    	Os__display(ucLine,ucCol,aucMsg);
    	break;
    }
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;
	memcpy(ProUiFace.ProToUi.aucLine1,aucMsg,strlen((char*)aucMsg));
	Os__xdelay(GUI_DELAY_TIMEOUT);
	
    return(MSG_WaitKey(uiTimeout));
}





unsigned char MSG_DisplayErrMsg(unsigned char ucErrorCode)
{
	unsigned char 	aucBuf[MSG_MAXCHAR+1];
	unsigned char 	ucType;
	unsigned short 	uiIndex1;
	unsigned char 	aucErrorCode[5];
	short 			iLen;
	UTIL_Beep();
	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));
#ifdef TEST
	Uart_Printf("ucErrorCode=%02x\n",ucErrorCode);
#endif
    if(RunData.ucPCCOMMFlag == 0x31)
    {
        memcpy(aucBuf,RunData.aucPCCOMMErrData,16);
        memcpy(aucErrorCode,RunData.aucPCCOMMErrCode,2);
        aucErrorCode[2] = 0;
        ucType = 0x30;
    }
    else if(ucErrorCode == ERR_HOSTCODE)
	{
		//ucErrorCode = (unsigned char)RUNDATA_ucErrorExtCode;
		if(RUNDATA_ucErrorExtCode>999)
		{
			short_asc(aucErrorCode,4,&RUNDATA_ucErrorExtCode);
			aucErrorCode[4]=0x00;
		}else if(RUNDATA_ucErrorExtCode>99)
		{
			short_asc(aucErrorCode,3,&RUNDATA_ucErrorExtCode);
			aucErrorCode[3]=0x00;
		}else if(RUNDATA_ucErrorExtCode>9)
		{
			long_asc(aucErrorCode,2,(ulong *)&RUNDATA_ucErrorExtCode);
			aucErrorCode[2]=0x00;	
		}else
		{
			long_asc(aucErrorCode,1,(ulong *)&RUNDATA_ucErrorExtCode);
			aucErrorCode[1]=0x00;	
		}
		if(RunData.aucErrorExplain[0]!=0)
			ucType=MSG_TYPEZHCN;
		else
			ucType=MSG_TYPEEN;
		memcpy(aucBuf,RunData.aucErrorExplain,16);
	}
	else if(ucErrorCode==ERR_WK_MIFARE)
	{
		ucErrorCode = (unsigned char)RUNDATA_ucErrorExtCode;
		for(uiIndex1=0;MifareErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
		{
			if( ucErrorCode == MifareErrMsg[uiIndex1].ucErrCode)
			{
				ucType = MifareErrMsg[uiIndex1].ucType;
				memcpy(aucBuf,MifareErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
				break;
			}
		}
		if(ErrMsg[uiIndex1].ucErrCode == 0xff) 
		{
			ucType = MifareErrMsg[uiIndex1].ucType;
			memcpy(aucBuf,MifareErrMsg[uiIndex1].aucErrMsg,MSG_MAXCHAR);
		}
		hex_asc(aucErrorCode,&ucErrorCode,2);
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
			memcpy(RunData.aucErrorExplain,aucBuf,16);
		}		
		if(ucErrorCode>0x63)
		{
			long_asc(aucErrorCode,3,(ulong *)&ucErrorCode);
			aucErrorCode[3]=0x00;
		}else if(ucErrorCode>0x09)
		{
			long_asc(aucErrorCode,2,(ulong *)&ucErrorCode);
			aucErrorCode[2]=0x00;	
		}else
		{
			long_asc(aucErrorCode,1,(ulong *)&ucErrorCode);
			aucErrorCode[1]=0x00;	
		}
	}		

	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=2;
	//Os__clr_display(255);
	switch( ucType)
	{
		case MSG_TYPEZHCN:
			//Os__GB2312_display(0, 0, (uchar *)(aucBuf));
			//Os__GB2312_display(1,0,(uchar *)"룺");
            //Os__display(1,9,(uchar *)aucErrorCode);

            memcpy(ProUiFace.ProToUi.aucLine1,"返回码: ",12);
            Uart_Printf("aucErrorCode~~~~~~~~~%s~~~~~~~~~%d~~~~~~~~~~~~\n",aucErrorCode,strlen(aucErrorCode));
            memcpy(&ProUiFace.ProToUi.aucLine1[12],aucErrorCode,strlen((char*)aucErrorCode));
			break;
		default:
            //Os__display(0,0,aucBuf);
			memcpy(&ProUiFace.ProToUi.aucLine1[0],aucBuf,strlen((char*)aucBuf));
			break;
	}
	if(ucErrorCode == ERR_HOSTCODE
		&&(RUNDATA_ucErrorExtCode==3113||RUNDATA_ucErrorExtCode==1308))
	{
		memset(aucBuf,0,sizeof(aucBuf));
		if(ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf))==SUCCESS)	
		{
			RunData.ulValueble=asc_long(aucBuf,12);
		}
		//Os__GB2312_display(2,0,(uchar *)":");
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf,RunData.ulValueble,2);
        //Os__display(3,5,(uchar *)aucBuf);

        memcpy(ProUiFace.ProToUi.aucLine2,"可用余额:",18);
        memcpy(&ProUiFace.ProToUi.aucLine2[18],aucBuf,strlen((char*)aucBuf));
	}
	Os__xdelay(GUI_DELAY_TIMEOUT);

    MSG_WaitKey(4);
	return(SUCCESS);
}

unsigned char MSG_DisplaySINGLEERRMSG(const SINGLEERRMSG ErrMsg[],unsigned char ucErrorCode)
{
	unsigned char 	aucBuf[MSG_MAXCHAR+1];
	unsigned char 	ucType;
	unsigned short 	uiIndex1;
	unsigned char 	aucErrorCode[3];


	ucType = 0;
	memset(aucBuf,0,sizeof(aucBuf));

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

	hex_str(aucErrorCode,&ucErrorCode,2);
	aucErrorCode[2] = 0x00;

	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=1;

	//Os__clr_display(255);
	switch( ucType)
	{
	case MSG_TYPEZHCN:
		//Os__GB2312_display(0, 0, (uchar *)(aucBuf));
		//Os__GB2312_display(1,0,(uchar *)"룺");
        //Os__display(1,9,(uchar *)aucErrorCode);
        memcpy(ProUiFace.ProToUi.aucLine1,"返回码：",12);

        memcpy(&ProUiFace.ProToUi.aucLine1[12],aucErrorCode,strlen((char*)aucErrorCode));

        Uart_Printf("~~~~~~~~~~~aucErrorCode~~~~~~~~~~~~~~%s   ****%d &&&&&&&&%d \n",aucErrorCode,strlen(aucErrorCode),strlen("返回码"));

        Uart_Printf("ProUiFace.ProToUi.aucLine1~~~~~~~~~~~~~~%s  *****%d \n",ProUiFace.ProToUi.aucLine1,strlen(ProUiFace.ProToUi.aucLine1));
		break;
	default:
        //Os__display(0,0,aucBuf);
		memcpy(ProUiFace.ProToUi.aucLine1,aucBuf,strlen((char*)aucBuf));
		break;
	}
	Os__xdelay(GUI_DELAY_TIMEOUT);

    MSG_WaitKey(4);
   	return(SUCCESS);
}
unsigned char MSG_GetErrMsg(unsigned char ucErrorCode,unsigned char*aucOutData)
{
	//unsigned char 	aucBuf[MSG_MAXCHAR+1];
	unsigned char 	ucType;
	unsigned short 	uiIndex1;
	unsigned char 	aucErrorCode[3];

	ucType = 0;
	
#ifdef TEST
		Uart_Printf("ucErrorCode=%02x\n",ucErrorCode);
#endif
	if(ucErrorCode==ERR_HOSTCODE)  
	{
		memcpy(aucOutData,RunData.aucErrorExplain,16);
			
	}
	else
	{	

#ifdef TEST
		Uart_Printf("ErrMsg\n");
#endif

		for(uiIndex1=0;ErrMsg[uiIndex1].ucErrCode<0xff;uiIndex1++)
		{
			if( ucErrorCode == ErrMsg[uiIndex1].ucErrCode)
			{
				ucType = ErrMsg[uiIndex1].ucType;
				memcpy(aucOutData,ErrMsg[uiIndex1].aucErrMsg,18);
				break;
			}
		}
		if(ErrMsg[uiIndex1].ucErrCode == 0xff)
		{
			ucType =ErrMsg[uiIndex1].ucType;
			memcpy(aucOutData,ErrMsg[uiIndex1].aucErrMsg,18);
		}		
		Uart_Printf("ErrMsgIndex=%d\n",uiIndex1);   
	}	
   	return(SUCCESS);
}

