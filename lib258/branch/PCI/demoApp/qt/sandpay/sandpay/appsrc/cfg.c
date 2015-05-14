#include <include.h>
#include <global.h>
#include <xdata.h>
#include <osmifare.h>
#include <stdio.h>
unsigned char CFG_ConstantParamVersion(void)
{
	unsigned char  aucBuf[49];
	
	memset(aucBuf,0,sizeof(aucBuf));
	CFG_GetVersionString(2,aucBuf);

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"ɼ֧");
	Os__display(1,0,(unsigned char *)aucBuf);
	Os__display(2,0,(unsigned char *)&aucBuf[16]); 
	//Os__GB2312_display(3,0,(unsigned char *)"Ӧú:194(00C2)"); 
	MSG_WaitKey(10);

    return(SUCCESS);
}



void CFG_GetVersionString(unsigned char ucFlag,unsigned char *pucBufOut)
{
	unsigned short usOffset=0;

	switch(ucFlag)
	{
	case 0: // 汾+
		// 汾
		memcpy(&pucBufOut[usOffset],CURRENT_VER,sizeof(CURRENT_VER)-1);
		usOffset+=sizeof(CURRENT_VER)-1;
		pucBufOut[usOffset]=' ';
		usOffset++;
		// 
		memcpy(&pucBufOut[usOffset],CURRENT_DATE,sizeof(CURRENT_DATE)-1);
		usOffset+=sizeof(CURRENT_DATE)-1;
		break;
	case 1: // Ŀ+汾
		// 汾
		memcpy(&pucBufOut[usOffset],CURRENT_PRJ,sizeof(CURRENT_PRJ)-1);
		usOffset+=sizeof(CURRENT_PRJ)-1;
		pucBufOut[usOffset]=' ';
		usOffset++;
		// 汾
		memcpy(&pucBufOut[usOffset],CURRENT_VER,sizeof(CURRENT_VER)-1);
		usOffset+=sizeof(CURRENT_VER)-1;
		break;
	default:	// Ŀ+汾+
		// Ŀ
		memcpy(&pucBufOut[usOffset],CURRENT_PRJ,sizeof(CURRENT_PRJ)-1);
		usOffset+=sizeof(CURRENT_PRJ)-1;
		pucBufOut[usOffset]=' ';
		usOffset++;
		// 汾
		memcpy(&pucBufOut[usOffset],CURRENT_VER,sizeof(CURRENT_VER)-1);
		usOffset+=sizeof(CURRENT_VER)-1;
		pucBufOut[usOffset]=' ';
		usOffset++;
		// 
		memcpy(&pucBufOut[usOffset],CURRENT_DATE,sizeof(CURRENT_DATE)-1);
		usOffset+=sizeof(CURRENT_DATE)-1;
		break;
	}
	
}


unsigned char CFG_ConstantParamBaseData(void)
{
    unsigned char   ucResult,ucYTJFlag=0;
    unsigned char   aucBuf[50],aucBakup[40],aucBakup1[40];
    unsigned short  uiSpeed,usI;
    unsigned long       traceNum,ulBakup;
    unsigned short  uiIndex;
    
    ucResult = SUCCESS;

    for(uiIndex = 0;uiIndex < TRANS_MAXNB;uiIndex++)
    {
        if(DataSave0.TransInfoData.auiTransIndex[uiIndex])
            break;
    }
      /* Terminal ID */
    if((uiIndex == TRANS_MAXNB) &&(ucResult == SUCCESS) 
                &&(DataSave0.ChangeParamData.ucIsLogonFlag!=FLAG_LOGON_OK) )
      {
      	
        memset(aucBuf,0,sizeof(aucBuf));
        MSG_DisplayMsg( true,0,0,MSG_TERMINALID);
        memcpy(aucBuf,DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
        if (UTIL_Input(1,true,8,8,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }
		     else
        {
        	if(memcmp(aucBuf,DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN)!=0)
        	{
			memcpy(aucBakup1,DataSave0.ConstantParamData.aucPinpadVersion,14);
        		memset(&DataSave0,0,sizeof(DATASAVEPAGE0));
		       memset(&DataSave1,0,sizeof(DATASAVEPAGE1));	
			XDATA_ClearAllFile();
			MAC_Load_Key(); 
   			memcpy(DataSave0.ConstantParamData.aucPinpadVersion,aucBakup1,14);
			RunData.ucWVFlag=1;
     			
        	}        	
            Os__saved_copy(aucBuf,
                DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
            ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
        }   
      }
      
       /* Merchant ID */
    if((uiIndex == TRANS_MAXNB) &&(ucResult == SUCCESS)
        &&(DataSave0.ChangeParamData.ucIsLogonFlag!=FLAG_LOGON_OK) )
    {
        memset(aucBuf,0,sizeof(aucBuf));
        MSG_DisplayMsg( true,0,0,MSG_MERCHANTID);
        memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
        if(UTIL_Input(1,true,15,15,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
        	if(memcmp(aucBuf,DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN)!=0)
		   {
			memcpy(aucBakup,DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
			memcpy(aucBakup1,DataSave0.ConstantParamData.aucPinpadVersion,14);
			memset(&DataSave0,0,sizeof(DATASAVEPAGE0));
			memset(&DataSave1,0,sizeof(DATASAVEPAGE1));	
			XDATA_ClearAllFile();
			MAC_Load_Key(); 
			memcpy(DataSave0.ConstantParamData.aucTerminalID,aucBakup,PARAM_TERMINALIDLEN);	
			memcpy(DataSave0.ConstantParamData.ucAPN,(uchar*)"CMNET",5);	
			memcpy(DataSave0.ConstantParamData.aucPinpadVersion,aucBakup1,14);
			RunData.ucWVFlag=1;
				
        		   		
        	}        	
            Os__saved_copy(aucBuf,
                DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
             ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
        }
    }
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"ģʽ");
		//Os__GB2312_display(1,0,(uchar *)"1.һ");
		//Os__GB2312_display(2,0,(uchar *)"2.һ");
//		//Os__GB2312_display(3,0,(uchar *)"3.KFC");
		aucBuf[0]=DataSave0.ConstantParamData.ucYTJFlag;
		aucBuf[1]=DataSave0.ConstantParamData.ucKFCFlag;
		aucBuf[2]=DataSave0.ConstantParamData.ucCom;
		
		{
			unsigned char ch;
			while(1)
			{
				ch=Os__xget_key();
				if(ch=='1')
				{
					DataSave0.ConstantParamData.ucYTJFlag=0;
					DataSave0.ConstantParamData.ucKFCFlag=0;
				}
				else if(ch=='2')
				{
					ucYTJFlag=1;
					DataSave0.ConstantParamData.ucKFCFlag=0;
					if(DataSave0.ConstantParamData.ucCom==0)
						DataSave0.ConstantParamData.ucCom=1;
				}
				/*else if(ch=='3')
				{
					DataSave0.ConstantParamData.ucYTJFlag=0;
					DataSave0.ConstantParamData.ucKFCFlag=1;
				}*/
				else if(ch==KEY_CLEAR)
					return SUCCESS;
				else
					continue;
				break;
			}
		}
		ucResult=XDATA_Write_Vaild_File(DataSaveConstant);	
		if(aucBuf[0]!=DataSave0.ConstantParamData.ucYTJFlag
			||aucBuf[1]!=DataSave0.ConstantParamData.ucKFCFlag
			||aucBuf[2]!=DataSave0.ConstantParamData.ucCom)
		{
			RunData.ucWVFlag=1;
		}
	}
	if(ucResult==SUCCESS&&DataSave0.ConstantParamData.ucKFCFlag==1)
	{
		if(ucResult == SUCCESS)
		{
			ulBakup=DataSave0.ConstantParamData.ulKFCHostIPAdd;
			memset(aucBuf,0,sizeof(aucBuf));
			//Os__clr_display(255);
	    	//Os__GB2312_display(0,0,(uchar *)"ݲɼַ:");  
		
			memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulKFCHostIPAdd,4);
			if (OSUTIL_InputIPV4Addr(0,0x31,1,aucBuf) != KEY_ENTER )  
			{
				ucResult = ERR_CANCEL;
			}else
			{
				memcpy(&DataSave0.ConstantParamData.ulKFCHostIPAdd, aucBuf, 4);
			}
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
			if(ucResult!=SUCCESS)	
				 return ucResult;  
			if(ulBakup!=DataSave0.ConstantParamData.ulKFCHostIPAdd)
			{
				RunData.ucWVFlag=1;
			}
		}
		
		if(ucResult == SUCCESS)
		{
			ulBakup=DataSave0.ConstantParamData.uiKFCHostPort;
			memset(aucBuf,0,sizeof(aucBuf));
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ݲɼ˿ں:"); 
			int_asc(aucBuf,6,&DataSave0.ConstantParamData.uiKFCHostPort);
			if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				DataSave0.ConstantParamData.uiKFCHostPort=asc_long(aucBuf,6);
			}
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
			if(ucResult!=SUCCESS)	
				 return ucResult;  
			if(ulBakup!=DataSave0.ConstantParamData.uiKFCHostPort)
			{
				RunData.ucWVFlag=1;
			}
		}	

	}
	if(ucResult==SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"ͨѶģʽ");
		//Os__GB2312_display(1,0,(uchar *)"1.  2.");
		//Os__GB2312_display(2,0,(uchar *)"3.GPRS  4.");
		//Os__GB2312_display(3,0,(uchar *)"5.CDMA");
		aucBuf[0]=DataSave0.ConstantParamData.ucYTJFlag;		
		{
			unsigned char ch;
			while(1)
			{
				ch=Os__xget_key();
				if(ch=='1')
				{
					Commun_flag=0x31;
					if(ucYTJFlag==1)
						DataSave0.ConstantParamData.ucYTJFlag=1;
				}
				else if(ch=='2')
				{
					Commun_flag=0x32;
					if(ucYTJFlag==1)
						DataSave0.ConstantParamData.ucYTJFlag=1;
				}
				else if(ch=='3')
				{
					Commun_flag=0x33;
					if(ucYTJFlag==1)
						DataSave0.ConstantParamData.ucYTJFlag=1;
				}
				else if(ch=='4')
				{
					Commun_flag=0x34;
					if(ucYTJFlag==1)
						DataSave0.ConstantParamData.ucYTJFlag=2;
				}
				else if(ch=='5')
				{
					Commun_flag=0x35;
					if(ucYTJFlag==1)
						DataSave0.ConstantParamData.ucYTJFlag=1;
				}
				else if(ch==KEY_CLEAR)
					return SUCCESS;
				else
					continue;
				break;
			}
		}
		ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		if(aucBuf[0]!=DataSave0.ConstantParamData.ucYTJFlag)
		{
			RunData.ucWVFlag=1;
		}
	}
    
   
	/* Comm mode */
	if(Commun_flag==0x32||Commun_flag==0x33||Commun_flag==0x35)
	{
		//IPַ
		if(ucResult == SUCCESS)
		{
			ulBakup=DataSave0.ConstantParamData.ulHostIPAdd;
			memset(aucBuf,0,sizeof(aucBuf));
			//Os__clr_display(255);
	    	//Os__GB2312_display(0,0,(uchar *)"IPַ:");  
		
			memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd,4);
			if (OSUTIL_InputIPV4Addr(0,0x31,1,aucBuf) != KEY_ENTER )  
			{
				ucResult = ERR_CANCEL;
			}else
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPAdd, aucBuf, 4);
				ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
			}
			if(ulBakup!=DataSave0.ConstantParamData.ulHostIPAdd)
			{
				RunData.ucWVFlag=1;
			}
		
		}
		
		if(ucResult == SUCCESS)
		{
            ulBakup=DataSave0.ConstantParamData.ulHostIPAdd;
			memset(aucBuf,0,sizeof(aucBuf));
			//Os__clr_display(255);
	    	//Os__GB2312_display(0,0,(uchar *)"˿ں:"); 
			int_asc(aucBuf,6,&DataSave0.ConstantParamData.uiHostPort);
			if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				DataSave0.ConstantParamData.uiHostPort=asc_long(aucBuf,6);
				ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
			}
			if(ulBakup!=DataSave0.ConstantParamData.uiHostPort)
			{
				RunData.ucWVFlag=1;
			}
		}
		if((Commun_flag==0x33)&&(ucResult == SUCCESS))
		{
			memset(aucBuf,0,sizeof(aucBuf));
			aucBuf[0] = 0x33;
			aucBuf[0] = aucBuf[0] - '0';
			Os__saved_copy(aucBuf,
					(unsigned char *)&DataSave0.ConstantParamData.ucDialRetry,
	                sizeof(unsigned char)); 
			DataSave0.ConstantParamData.uiDialWaitTime = 40;
			DataSave0.ConstantParamData.ucCommMode = PARAM_COMMMODE_GPRS;//xiaobin,2011-05-11,modify
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
	    		memset(aucBuf,0,sizeof(aucBuf));
	    		//Os__clr_display(255);
	    		//Os__GB2312_display(0,0,(uchar *)"APN:");
	    		memcpy(aucBuf,DataSave0.ConstantParamData.ucAPN,20);
	    		if (UTIL_Input(1,true,0,20,'A',aucBuf,0) != KEY_ENTER )
	    		{
	    			ucResult = ERR_CANCEL;
	    		}else
	    		{
	    			memset(DataSave0.ConstantParamData.ucAPN,0,21);
	    			memcpy(DataSave0.ConstantParamData.ucAPN,aucBuf,21);
	    			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
	    		}	
		}
		if((Commun_flag==0x35)&&(ucResult == SUCCESS))//gaolong,2011-07-19,add
		{
			DataSave0.ConstantParamData.ucCommMode = PARAM_COMMMODE_CDMA;
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}
	else if(Commun_flag==0x31)
	{
		if( ucResult == SUCCESS)
	    {
	        MSG_DisplayMsg( true,0,0,MSG_COMMMODE);
	        memset(aucBuf,0,sizeof(aucBuf));
	        aucBuf[0] = 0x32;
	        {
	            aucBuf[0] = aucBuf[0] - '0';
	            Os__saved_copy(aucBuf,
	                &DataSave0.ConstantParamData.ucCommMode,1);
	            ucResult=XDATA_Write_Vaild_File(DataSaveConstant);  
	        }
	    } 
		/* Dial connect wait seconds*/
	    if( ucResult == SUCCESS)
	    {
	        if(  (aucBuf[0] == PARAM_COMMMODE_MODEM)
	           ||(aucBuf[0] == PARAM_COMMMODE_HDLC)
	          )
	        {
	            if( ucResult == SUCCESS)
	            {
	                memset(aucBuf,0,sizeof(aucBuf));
	                aucBuf[0] = 0x33;
	                {
	                    aucBuf[0] = aucBuf[0] - '0';
	                    Os__saved_copy(aucBuf,
	                        (unsigned char *)&DataSave0.ConstantParamData.ucDialRetry,
	                        sizeof(unsigned char)); 
	                      ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
	                }
	            }
	            if( ucResult == SUCCESS)
	            {
	                memset(aucBuf,0,sizeof(aucBuf));
	                memcpy(aucBuf,"40",2);
	                {
	                    uiSpeed = (unsigned short )str_long(aucBuf);
	                    Os__saved_copy((unsigned char *)&uiSpeed,
	                        (unsigned char *)&DataSave0.ConstantParamData.uiDialWaitTime,
	                        sizeof(unsigned short)); 
	                     ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
	                }
	            }
	        }
	    }
	
	    /* Receive Data Timeout */
	    if( ucResult == SUCCESS)
	    {
	        memset(aucBuf,0,sizeof(aucBuf));
	        memcpy(aucBuf,"30",2);
	        {
	            uiSpeed = (unsigned short )str_long(aucBuf);
	            Os__saved_copy( (unsigned char *)&uiSpeed,
	                (unsigned char *)&DataSave0.ConstantParamData.uiReceiveTimeout,
	                sizeof(unsigned short)); 
	             ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
	        }
	    }
	
	    /* HOST Tel 1 */
	    if( ucResult == SUCCESS)
	    {
	      	memcpy(aucBakup,DataSave0.ConstantParamData.aucInitTelNumber1,PARAM_TELNUMBERLEN);
	        memset(aucBuf,0,sizeof(aucBuf));
	        //Os__clr_display(255);
	       	//Os__GB2312_display(0,0,(unsigned char *)"");
	      	memcpy(aucBuf,DataSave0.ConstantParamData.aucInitTelNumber1,PARAM_TELNUMBERLEN);
	        for(usI=0;usI<PARAM_TELNUMBERLEN;usI++)
	        {
	            if(aucBuf[usI] == '.')
	            {
			  memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
	                break;
	            }
	        }       
	        if (UTIL_Input(1,true,1,20,'N',aucBuf,0) != KEY_ENTER )
	        {
	            ucResult = ERR_CANCEL;
	        }else     
	        {
			usI=strlen((char *)aucBuf)+1;
	            aucBuf[strlen((char *)aucBuf)] = '.';
			memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
	            Os__saved_copy(aucBuf,
	                DataSave0.ConstantParamData.aucInitTelNumber1,PARAM_TELNUMBERLEN);
	              ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
	        }
		if(memcmp(aucBakup,DataSave0.ConstantParamData.aucInitTelNumber1,PARAM_TELNUMBERLEN))
		{
			RunData.ucWVFlag=1;
		}
	    }
	    /* HOST Tel 2 */
	    if( ucResult == SUCCESS)
	    {
	      	memcpy(aucBakup,DataSave0.ConstantParamData.aucInitTelNumber2,PARAM_TELNUMBERLEN);
	        memset(aucBuf,0,sizeof(aucBuf));
	        //Os__clr_display(255);
	        //Os__GB2312_display(0,0,(unsigned char *)"ݣ");
	      	memcpy(aucBuf,DataSave0.ConstantParamData.aucInitTelNumber2,PARAM_TELNUMBERLEN);
	        for(usI=0;usI<PARAM_TELNUMBERLEN;usI++)
	        {
	            if(aucBuf[usI] == '.')
	            {
			  memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
	                break;
	            }
	        }       
	        if (UTIL_Input(1,true,1,20,'N',aucBuf,0) != KEY_ENTER )
	        {
	            ucResult = ERR_CANCEL;
	        }else
	        {
			usI=strlen((char *)aucBuf)+1;
	            aucBuf[strlen((char *)aucBuf)] = '.';
			memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
	            Os__saved_copy(aucBuf,
	                DataSave0.ConstantParamData.aucInitTelNumber2,PARAM_TELNUMBERLEN);
	              ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
	        }
			if(memcmp(aucBakup,DataSave0.ConstantParamData.aucInitTelNumber2,PARAM_TELNUMBERLEN))
			{
				RunData.ucWVFlag=1;
			}
	    }
	}
	if( ucResult == SUCCESS)
    {
		ulBakup=DataSave0.ChangeParamData.ulTraceNumber;
        memset(aucBuf,0,sizeof(aucBuf));
        MSG_DisplayMsg( true,0,0,MSG_TRACENUM);
        long_asc(aucBuf,6,&DataSave0.ChangeParamData.ulTraceNumber);
        if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
            traceNum = asc_long(aucBuf,6);
            Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.ulTraceNumber,0,sizeof(unsigned long));
            Os__saved_copy((unsigned char *)&traceNum,(unsigned char *)&DataSave0.ChangeParamData.ulTraceNumber,sizeof(unsigned long)); 
            ucResult=XDATA_Write_Vaild_File(DataSaveChange); 
        }
		if(ulBakup!=DataSave0.ChangeParamData.ulTraceNumber)
			RunData.ucWVFlag=1;
    }
    return(ucResult);
	
}



unsigned char CFG_ConstantParamDateTime(void)
{
    unsigned char Date_Time[7],display[7],tmp_date[7],ucresult;
    unsigned char days[12]={31,29,31,30,31,30,31,31,30,31,30,31};
    unsigned char month,day,hour,min,sec;  

    memset(Date_Time,0,sizeof(Date_Time));          
    Os__read_date(&Date_Time[0]);
    memcpy(&display[0],&Date_Time[4],2);
    memcpy(&display[2],&Date_Time[2],2);
    memcpy(&display[4],Date_Time,2);    
    display[6] = 0; 
    while(1)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,2,(unsigned char *)"");
        //Os__GB2312_display(1,0,(unsigned char *)":");
        Os__display(1,6,(uchar *)display);
        //Os__GB2312_display(2,0,(unsigned char *)":");
        Os__display(2,6,(unsigned char *)"yymmdd");
        memset(tmp_date,0,sizeof(tmp_date));
        if(UTIL_Input_XY(3,6,true,6,6,'N',tmp_date,0) != KEY_ENTER)
          return(ERR_CANCEL);
        Date_Time[0] = tmp_date[4];
        Date_Time[1] = tmp_date[5];
        Date_Time[2] = tmp_date[2];
        Date_Time[3] = tmp_date[3];
        Date_Time[4] = tmp_date[0];
        Date_Time[5] = tmp_date[1];
        Date_Time[6] = 0;
        ucresult=Os__write_date(Date_Time);
        month = asc_long(&tmp_date[2],2);
        day= asc_long(&tmp_date[4],2);
        if(( (asc_long(tmp_date,2))+2000)%4!=0)  days[1]=28;
        if((month>0x12) ||( day>0x31)/*||(month<=0)||(day<=0)*/||(day>days[month-1])) 
        {
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)"!");
            UTIL_GetKey(10);
            continue;
        }
        break;
    }   
        while(1)
    {
            memset(Date_Time,0,sizeof(Date_Time));  
            Os__read_time_sec(&Date_Time[0]);  
                //Os__clr_display(255);
            //Os__GB2312_display(0,2,(unsigned char *)"ʱ");
            //Os__GB2312_display(1,0,(unsigned char *)"ʱ:");
            Os__display(1,6,Date_Time);
               //Os__GB2312_display(2,0,(unsigned char *)":");
               Os__display(2,6,(unsigned char *)"hhmmss");
               memset(tmp_date,0,sizeof(tmp_date));    
            if(UTIL_Input_XY(3,6,true,6,6,'N',tmp_date,0) != KEY_ENTER)
              return(ERR_CANCEL);
            memcpy(Date_Time,&tmp_date,4);
            ucresult=Os__write_time(Date_Time);
            asc_bcd(&hour, 1, Date_Time, 2);
            asc_bcd(&min, 1, &Date_Time[2], 2);
            asc_bcd(&sec, 1, &Date_Time[4], 2); 
            if((hour>0x23)||(min>0x59)|| (sec>0x59) /*||(hour<0) ||( min<0) ||( sec<0)*/)//unsigned char cannot <0
             {
                    //Os__clr_display(255);
                    //Os__GB2312_display(1,0,(unsigned char *)"ʱ!");
                    UTIL_GetKey(10);
                    continue;
            }
            break;
    }
    return SUCCESS;
}



unsigned char CFG_ConstantParamPhoneNo(void)
{
    unsigned char ucResult;
    unsigned char aucBuf[21],ucCh;
    unsigned short uiTmp;

    ucResult = SUCCESS;
  
    if( ucResult == SUCCESS )
    {
        memset(aucBuf,0,sizeof(aucBuf));
        MSG_DisplayMsg( true,0,0,MSG_SWITCHBOARD);
        memcpy(aucBuf,DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN);
        if (UTIL_Input(1,true,0,20,'A',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
            ucResult = SUCCESS;
            Os__saved_copy(aucBuf,DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN);
            ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
        }
    }
    if( ucResult == SUCCESS )
    {
        //Os__clr_display(255);
        memset(aucBuf,0,sizeof(aucBuf));
        //Os__GB2312_display(0,0,(unsigned char *)"ųʱ");
        //Os__GB2312_display(1,0,(unsigned char *)"ǰ:");
	 short_asc(aucBuf,2,&DataSave0.ConstantParamData.uiDialWaitTime);
	 Os__display(1,5,aucBuf);
        //Os__GB2312_display(2,0,(unsigned char *)":");
        memset(aucBuf,0,sizeof(aucBuf));
	 if(UTIL_Input(3,false,2,2,'N',aucBuf, NULL)==KEY_ENTER)
	 {
		DataSave0.ConstantParamData.uiDialWaitTime=asc_long(aucBuf,2);
	       ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
	 }
	 	
    }
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"ûҪ");
    //Os__GB2312_display(1,0,(unsigned char *)"Ĭ");
    //Os__GB2312_display(2,0,(unsigned char *)"ǷĬ");
    //Os__GB2312_display(3,0,(unsigned char *)"ȷ/ȡ");
    if(MSG_WaitKey(30)==KEY_ENTER)
    {
        unsigned char aucModemParam[10]={0,0,8,6,115,8,1,3,0,0};  
        Os__saved_copy(aucModemParam,(uchar *)&DataSave0.ConstantParamData.ModemParam,10);
        ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
        return ucResult;
    }else
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"δѡĬ");
        //Os__GB2312_display(1,0,(unsigned char *)"ò");
        MSG_WaitKey(5);
    }
//    if( ucResult == SUCCESS )
    {
        aucBuf[0] = '1';
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"Ƶ/壺");
            //Os__GB2312_display(1,0,(unsigned char *)"1.Ƶ  2.");
            aucBuf[0] = '1';
            if (UTIL_Input(2,false,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
                if(aucBuf[0] == '1')
                    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dp,0,1);
                else
                    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dp,1,1);
                ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
           
                break;
            }
        }
    }
//    if( ucResult == SUCCESS )
    {
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"Ⲧ");
            //Os__GB2312_display(1,0,(unsigned char *)"1.  2.");
            aucBuf[0] = '1';
            if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
                if(aucBuf[0] == '1')
                    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.chdt,0,1);
                else
                    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.chdt,1,1);
               ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
                break;
            }
        }
    }
//    if( ucResult == SUCCESS )
    {
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"ժŵȴʱ");
            aucBuf[0]= '8';
            if (UTIL_Input(1,true,1,3,'N',(uchar *)aucBuf,0) != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
//Uart_Printf("aucBuf=%s\n",aucBuf);
                uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
//Uart_Printf("uiTmp1=%d\n",uiTmp); 
                if((uiTmp <4)||(uiTmp>255))
                {
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,(unsigned char *)"ֵ4");
                    //Os__GB2312_display(1,0,(unsigned char *)"255֮䣡");
                    MSG_WaitKey(8);
                    continue;
                }   
                ucCh = uiTmp;
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dt1,ucCh,1);
               ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
                break;
            }
        }
    }
//    if( ucResult == SUCCESS )
    {
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"ߵȴʱ䣺");
            aucBuf[0]='6';
            if (UTIL_Input(1,true,1,3,'N',aucBuf,0) != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
                uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
Uart_Printf("uiTmp2=%d\n",uiTmp);
                if(uiTmp>255)
                {
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,(unsigned char *)"ֵС");
                    //Os__GB2312_display(1,0,(unsigned char *)"255");
                    MSG_WaitKey(8);
                    continue;
                }   
                ucCh = uiTmp;
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dt2,ucCh,1);
              ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
                break;
            }
        }
    }
//    if( ucResult == SUCCESS )
    {
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"һ뱣ʱ");
            memcpy(aucBuf,"115",3);
            if (UTIL_Input(1,true,1,3,'N',aucBuf,0) != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
                uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
Uart_Printf("uiTmp3=%d\n",uiTmp);
                if((uiTmp < 70)||(uiTmp>255))
                {
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,(unsigned char *)"ֵ70");
                    //Os__GB2312_display(1,0,(unsigned char *)"255֮䣡");
                    MSG_WaitKey(8);
                    continue;
                }   
                ucCh = uiTmp;
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.ht,ucCh,1);
               ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
                break;
            }
        }
    }
//    if( ucResult == SUCCESS )
    {
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"żʱ䣺");
            aucBuf[0]='8';
            if (UTIL_Input(1,true,1,2,'N',aucBuf,0) != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
                uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
Uart_Printf("uiTmp4=%d\n",uiTmp);
                if((uiTmp < 7)||(uiTmp>25))
                {
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,(unsigned char *)"ֵ7");
                    //Os__GB2312_display(1,0,(unsigned char *)"25֮䣡");
                    MSG_WaitKey(8);
                    continue;
                }   
                ucCh = uiTmp;
Uart_Printf("uiTmp4=%d\n",ucCh);
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.wt,ucCh,1);
Uart_Printf("wt=%d \n",DataSave0.ConstantParamData.ModemParam.wt);
                ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
                break;
            }
        }
    }

//    if( ucResult == SUCCESS )
    {
        memset(aucBuf,0,sizeof(aucBuf));
        ucCh = 0;
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"ͬ첽ѡ");
        //Os__GB2312_display(1,0,(unsigned char *)"1.ͬ  2.첽");
        memset(aucBuf,0x00,sizeof(aucBuf));
        aucBuf[0] = '1';
        if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
            if(aucBuf[0] == '2')
                ucCh |=0x80;
            ucResult = SUCCESS;
Uart_Printf("ucCh1=%02x \n",ucCh);
        }
        if(ucResult == SUCCESS)
        {
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"ã");
            //Os__GB2312_display(1,0,(unsigned char *)"1-1200  2-2400");          
            if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
            }else
            {
                if(aucBuf[0] == '1')
                    ucCh |=0x00;
                else
                    ucCh |=0x20;
                ucResult = SUCCESS;
Uart_Printf("ucCh2=%02x \n",ucCh);
            }
        }
        if(ucResult == SUCCESS)
        {
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"·ʽ");
            //Os__GB2312_display(1,0,(unsigned char *)"1.CCITT  2.BELL");         
            memset(aucBuf,0x00,sizeof(aucBuf));
            aucBuf[0] = '1';
            if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
            }else
            {
                if(aucBuf[0] == '1')
                    ucCh |=0x00;
                else
                    ucCh |=0x10;
                ucResult = SUCCESS;
Uart_Printf("ucCh3=%02x \n",ucCh);
            }
        }
        if(ucResult == SUCCESS)
        {
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"Ӧʱʱ");
            //Os__GB2312_display(3,0,(unsigned char *)"");
            //Os__GB2312_display(1,0,(unsigned char *)"ǰ");
            //Os__GB2312_display(2,0,(unsigned char *)"ߺ");
            MSG_WaitKey(60);
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"1-5 2-8 3-11");
            //Os__GB2312_display(1,0,(unsigned char *)"4-14 5-17 6-20");           
            //Os__GB2312_display(2,0,(unsigned char *)"7-23 8-26 ");
            memset(aucBuf,0x00,sizeof(aucBuf));
            aucBuf[0]='2';
            if (UTIL_Input(3,true,1,1,'N',aucBuf,(uchar *)"12345678") != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
            }else
            {
                switch(aucBuf[0])
                {
                    case '1':
                        ucCh |= 0x00;
                        break;
                    case '2':
                        ucCh |= 0x01;
                        break;
                    case '3':
                        ucCh |= 0x02;
                        break;
                    case '4':
                        ucCh |= 0x03;
                        break;
                    case '5':
                        ucCh |= 0x04;
                        break;
                    case '6':
                        ucCh |= 0x05;
                        break;
                    case '7':
                        ucCh |= 0x06;
                        break;
                    case '8':
                        ucCh |= 0x07;
                        break;
                }
                ucResult = SUCCESS;
Uart_Printf("ucCh4=%02x \n",ucCh);
            }
        }
        if(ucResult == SUCCESS)
        {
            Os__saved_set(&DataSave0.ConstantParamData.ModemParam.ssetup,ucCh,1);
            ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
        }
    }
//    if( ucResult == SUCCESS )
    {
        while(1)
        {
            memset(aucBuf,0,sizeof(aucBuf));
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"ѭ");
            aucBuf[0]='3';
            if (UTIL_Input(1,true,1,1,'N',aucBuf,0) != KEY_ENTER )
            {
                ucResult = ERR_CANCEL;
                break;
            }else
            {
                uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
                ucCh = uiTmp;
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dtimes,ucCh,1);
                ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
                break;
            }
        }
    }
    {
        unsigned char ucI;
        Uart_Printf("\nModemParam = ");
        for(ucI = 0;ucI<10;ucI++)
            Uart_Printf("%d ",*(&DataSave0.ConstantParamData.ModemParam.dp+ucI));
            Uart_Printf("\n%d ",DataSave0.ConstantParamData.ModemParam.dp);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.chdt);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.dt1);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.dt2);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.ht);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.wt);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.ssetup);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.dtimes);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.timeout);
            Uart_Printf("%d ",DataSave0.ConstantParamData.ModemParam.asmode);
    }
    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.timeout,0,1);
    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.asmode,0,1);
   ucResult=XDATA_Write_Vaild_File(DataSaveConstant); 
   
    return(ucResult);
}


unsigned char CFG_ClearSettleProcess(void)
{
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,"清结算标志?",22);
    sleep(1);
#else
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"־?");
#endif
    //if(Os__xget_key()!=KEY_ENTER)
        //return SUCCESS;
		
	Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.ucSettleFlag,
									FLAG_SETTLE_UNDO,sizeof(unsigned char));
		  XDATA_Write_Vaild_File(DataSaveChange);							
	return SUCCESS;
}


unsigned char CFG_ClearReversalProcess(void)
{
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,"清冲正标志?",22);
    sleep(1);
#else
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"־?");
#endif
//	if(Os__xget_key()!=KEY_ENTER)
//		return SUCCESS;
		
	Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0, sizeof(unsigned char));
	  XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	return SUCCESS;
}
unsigned char CFG_ClearTransKeyFlag(void)
{
#ifdef GUI_PROJECT
        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"允许下传输密钥?",30);
        sleep(1);
 #else
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"´Կ?");
#endif
//	if(Os__xget_key()!=KEY_ENTER)
//		return SUCCESS;
		
	Os__saved_set((unsigned char *)(&DataSave0.ConstantParamData.ucTranskeyflag),0, sizeof(unsigned char));
	XDATA_Write_Vaild_File(DataSaveConstant);
	return SUCCESS;
}

unsigned char CFG_ClearRom(void)
{
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"Ƿڴ棿");
	//Os__GB2312_display(1,0,(uchar *)"ȷڴ");
	//Os__GB2312_display(2,0,(uchar *)"ȡ");
	if(Os__xget_key()==KEY_ENTER)
	{
		       memset(&DataSave0,0,sizeof(DATASAVEPAGE0));
		       memset(&DataSave1,0,sizeof(DATASAVEPAGE1));	
			XDATA_ClearAllFile();
			MAC_Load_Key(); 
		
		return SUCCESS;
	}else
		return SUCCESS;
}
unsigned char CFG_SetAmountInput(void)
{
	unsigned char ucInput;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"ģʽ");
	//Os__GB2312_display(1,0,(uchar *)"1.ԷΪλ");
	//Os__GB2312_display(2,0,(uchar *)"2.ԽΪλ");
	//Os__GB2312_display(3,0,(uchar *)"3.ԪΪλ");
	ucInput=Os__xget_key();
	switch(ucInput)
	{
		case '1':
			DataSave0.ConstantParamData.ucAmoutInput=0;
			XDATA_Write_Vaild_File(DataSaveConstant);
			break;
		case '2':
			DataSave0.ConstantParamData.ucAmoutInput=1;
			XDATA_Write_Vaild_File(DataSaveConstant);
			break;
		case '3':
			DataSave0.ConstantParamData.ucAmoutInput=2;
			XDATA_Write_Vaild_File(DataSaveConstant);
			break;
		default:
			break;
	}
	return SUCCESS;
}


unsigned char CFG_CheckFlag(void)
{
	if(DataSave0.ChangeParamData.ucInilizeFlag!=FLAG_INIT_OK)
		return ERR_NOTINIT;
	if(DataSave0.ChangeParamData.ucIsLogonFlag!=FLAG_LOGON_OK)
		return ERR_NOTLOGON;
	if(DataSave0.ChangeParamData.ucSettleFlag==FLAG_SETTLE_FAIL)	
		return ERR_SETTLE_FIRST;
		
	return SUCCESS;
}
unsigned char CFG_CheckFactoryBytes(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucStatus,ucLen,aucData[300],aucBuf[20];
	unsigned int    uiTimesOut;
	unsigned int    uiI;
	//if(ucResult==SUCCESS)
	//	ucResult = Untouch_FindCard_tmp(&ucStatus,aucData,&ucLen);
	ucResult = MifareCard_Init(aucData,&ucLen);
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ucNewOrOld==0)
			memcpy(RunData.aucWriteKey2,RunData.aucKeyA,6);
		else
			memcpy(RunData.aucWriteKey2,"\xA0\xA1\xA2\xA3\xA4\xA5",6);
		//ucResult = Untouch_0_CardVerify_tmp(&ucStatus,aucData,&ucLen);
		ucResult = CLASSIC_Auth(OSMIFARE_AUTH_TYPE_A,0x00,OSMIFARE_KEY_TYPE_A,RunData.aucWriteKey2);
	}						

	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = CLASSIC_ReadBlock(0,aucBuf);

		if(ucResult==SUCCESS)
			Data_XOR(0,aucBuf,16);
			memcpy(aucData,aucBuf,16);
			Uart_Printf("aucData:\n");
			for(uiI=0;uiI<16;uiI++)
			{
				Uart_Printf("%02x",aucData[uiI]);
			}
			Uart_Printf("\n");
		//ucResult = Untouch_Read_0_Block_tmp(&ucStatus,aucData,&ucLen);
	}
	if(ucResult==SUCCESS)
	{
		if(
		((aucData[8]==0xC9)&&(aucData[9]==0xBC)&&(aucData[10]==0xCE)&&(aucData[11]==0xA1)&&

		//
		(aucData[12]==0xB5)&&(aucData[13]==0xC2)&&(aucData[14]==0xBF)&&(aucData[15]==0xB5))||  // 

		//
		((aucData[8]==0xB5)&&(aucData[9]==0xC2)&&(aucData[10]==0xCE)&&(aucData[11]==0xA1)&&

		//
		(aucData[12]==0xBF)&&(aucData[13]==0xB5)&&(aucData[14]==0xC9)&&(aucData[15]==0xBC))|| // NXP

		//
		((aucData[8]==0x85)&&(aucData[9]==0x00)&&(aucData[10]==0xD2)&&(aucData[11]==0xF8)&&

		//
		(aucData[12]==0xBF)&&(aucData[13]==0xA8)&&(aucData[14]==0xCD)&&(aucData[15]==0xA8)) )   // 翨
			return SUCCESS;
		else
			return ERR_NOTSUCHAPP;
	}
	return ucResult;
}

unsigned char CFG_CheckCardValid(void)
{
	unsigned char aucCheck[20],aucBuf[20],aucBuf1[20],aucFensan[20],aucUseKey[20];
	unsigned char ucResult=SUCCESS;
	unsigned long ulSieral;
	int i,ivo,iErrNum;

	Uart_Printf("\n 20120625-000004-0001  Untouch_Menu  .... [%02x]\n",ucResult);	
	
	memset(aucCheck,0,sizeof(aucCheck));
	aucCheck[0]=NormalTransData.ucPtCode;
	memcpy(&aucCheck[1],NormalTransData.aucZoneCode,4);
	ucResult=CFG_CheckCardZone(aucCheck);
	if(ucResult!=SUCCESS)	
	{
		Uart_Printf("check cardtabel err=%02x\n",ucResult);
		return ucResult;
	}

	Uart_Printf("\n 20120625-000004-0002  Untouch_Menu  .... [%02x]\n",ucResult);	
	
	UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucCardValue,2,RunData.aucWDFunFlag);
	UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,RunData.aucZDFunFlag);
	UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucExFunction,4,RunData.aucExFunFlag);
	if(DataSave1.ZoneCtrl.ucValFlag&0xF0)
		RunData.ucWeiKFlag=1;
	if(DataSave1.ZoneCtrl.ucValFlag&0x0F)
		RunData.ucWanTFlag=1;
	memcpy(NormalTransData.aucZoneName,DataSave1.ZoneCtrl.aucWaikangName,16);
	memcpy(NormalTransData.aucZoneName1,DataSave1.ZoneCtrl.aucHopassName,16);
	RunData.ucKeyArrey=0;
	if(NormalTransData.ucPtCode==0x03)
	{
		for(i=0;i<15;i++)
		{
			if(!memcmp(DataSave0.ChangeParamData.aucIssueKeyIndex[i],NormalTransData.aucZoneCode,2)
			&&memcmp(DataSave0.ChangeParamData.aucIssueKeyIndex[i],"\x00\x00\x00",3))
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
			if(!memcmp(aucBuf,aucBuf1,3)&&memcmp(aucBuf,"000000",6))
			{
				RunData.ucKeyArrey=i;
				break;
			}
		}
	}
	memset(aucCheck,0,sizeof(aucCheck));
	bcd_asc(aucCheck,NormalTransData.aucUserCardNo,20);
	ivo=bcd_long(&DataSave1.ZoneCtrl.ucVerifyOffset,2);
	if(ivo)
		memcpy(&aucCheck[ivo-1],&aucCheck[ivo],16-ivo);

	Uart_Printf("\n 20120625-000004-0003  Untouch_Menu  .... [%02x]\n",ucResult);	
	
	ucResult=CFG_CheckCardTbl(aucCheck);
	if(ucResult!=SUCCESS)	
	{
		Uart_Printf("check cardtabel err=%02x\n",ucResult);
		return ucResult;
	}

	Uart_Printf("\n 20120625-000004-0004  Untouch_Menu  .... [%02x]\n",ucResult);	
	if(RunData.ucNewLgFlag==1)
		RunData.aucFunFlag[2]=1;
	memcpy(NormalTransData.aucFunFlag,RunData.aucFunFlag,32);
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		if(RunData.aucFunFlag[19])
			memcpy(NormalTransData.aucCardSerial,NormalTransData.aucUserCardNo,4);

	}
	  //жϿ״̬	
	if(SleTransData.ucStatus==CARD_LOCKED)		
		return ERR_CARDLOCK;

	Uart_Printf("\n 20120625-000004-0005  Untouch_Menu  .... [%02x]\n",ucResult);	
	
	if(RunData.aucFunFlag[6])
	{
		memset(aucCheck,0,sizeof(aucCheck));
		aucCheck[0]=NormalTransData.ucPtCode;
		asc_bcd(&aucCheck[1],8,NormalTransData.aucLoadTrace,16);	
		ucResult=CFG_CheckBlackFeild(aucCheck);
		if(ucResult!=SUCCESS)	
			return ucResult;
		memset(aucCheck,0,sizeof(aucCheck));
		aucCheck[0]=NormalTransData.ucPtCode;
		ulSieral=tab_long(NormalTransData.aucCardSerial,4);	
		long_bcd(&aucCheck[1],8,&ulSieral);
		ucResult=CFG_CheckBlackFeild(aucCheck);
		if(ucResult!=SUCCESS)	
			return ucResult;
	}

	Uart_Printf("\n 20120625-000004-0006  Untouch_Menu  .... [%02x]\n",ucResult);	
		
	if(RunData.aucFunFlag[3]&&NormalTransData.ucTransType!=TRANS_S_CHANGEEXP
		&&(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
	)
	{
		ucResult=CFG_ChkExpireValid(NormalTransData.aucExpiredDate);
		if(ucResult!=SUCCESS)
		{
			if(RunData.aucZDFunFlag[46]&&NormalTransData.ulYaAmount!=0)
			{
				RunData.ucExpireFlag=1;
				ucResult=SUCCESS;
			}
			else
			{
				RunData.ucExpire1Flag=1;
				ucResult=SUCCESS;
			}
		}
	}
	if(!RunData.aucFunFlag[15])
		memset(NormalTransData.aucDiscount,0,2);
	if(NormalTransData.ulPrevEDBalance>NormalTransData.ulMaxAmount
		&&(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1))
		return ERR_OVERAMOUNT;

	Uart_Printf("\n 20120625-000004-0007  Untouch_Menu  .... [%02x]\n",ucResult);	
	
	/**/
	if(NormalTransData.ucCardType==CARD_CPU)
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
			if(ucResult!=SUCCESS)
			{
				if(Iso7816Out.ucSW1==0x63)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0, 0,(uchar *)"Ƭ֤ʧ");
					iErrNum=Iso7816Out.ucSW2-0xC0;
					memset(aucBuf,0,sizeof(aucBuf));
					int_asc(aucBuf,2,&iErrNum);
					if(iErrNum)
					{
						//Os__GB2312_display(1, 0,(uchar *)"ʣ:");
						Os__display(1,13,aucBuf);
					}
					else
						//Os__GB2312_display(1, 0,(uchar *)"֤Կ");
					MSG_WaitKey(5);
				}
			}
		}
	}

	Uart_Printf("\n 20120625-000004-0008  Untouch_Menu  .... [%02x]\n",ucResult);	
	
	return ucResult;
}

unsigned char CFG_CheckCardZone(unsigned char *InData)
{
	unsigned char aucCmpBuf[20],ucI,ucResult=SUCCESS;
	short FirstPos,LastPos,MidPos;
	int i;
	if(DataSave0.ChangeParamData.uiZoneCodeNum==0)
		return ERR_ZONECODE;
	FirstPos=0;
	LastPos = DataSave0.ChangeParamData.uiZoneCodeNum-1;

	ucResult=XDATA_Read_ZoneCtrls_File(0);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(aucCmpBuf,&DataSave1.ZoneCtrl.ucPtCode,4);
	if(memcmp(InData,aucCmpBuf,4)<0)
		return ERR_ZONECODE;
	if(memcmp(InData,aucCmpBuf,4)==0)
	{
		return SUCCESS;
	}

	ucResult=XDATA_Read_ZoneCtrls_File(DataSave0.ChangeParamData.uiZoneCodeNum-1);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(aucCmpBuf,&DataSave1.ZoneCtrl.ucPtCode,4);
	if(memcmp(InData,aucCmpBuf,4)>0)
		return ERR_ZONECODE;
	if(memcmp(InData,aucCmpBuf,4)==0)
	{
		return SUCCESS;
	}

	while(FirstPos<=LastPos)
	{
		MidPos=(FirstPos+LastPos)/2;
		ucResult=XDATA_Read_ZoneCtrls_File(MidPos);
		if(ucResult!=SUCCESS)
			return ucResult;
		memset(aucCmpBuf,0,sizeof(aucCmpBuf));
		memcpy(aucCmpBuf,&DataSave1.ZoneCtrl.ucPtCode,4);
		if(memcmp(InData,aucCmpBuf,4)<0)
			LastPos=MidPos-1;
		else if(memcmp(InData,aucCmpBuf,4)>0)
			FirstPos=MidPos+1;
		else
			return SUCCESS;
	}
	return ERR_ZONECODE;
}

unsigned char CFG_CheckBlackFeild(unsigned char *InData)
{
	unsigned char aucCmpBuf[20],aucCmpBuf1[20],ucI,ucResult=SUCCESS;
	short FirstPos,LastPos,MidPos,i;
	Uart_Printf("DataSave0.ChangeParamData.uiBlackFeildNum=%d\n",DataSave0.ChangeParamData.uiBlackFeildNum);
	if(DataSave0.ChangeParamData.uiBlackFeildNum==0)
		return SUCCESS;
	FirstPos=0;
	LastPos = DataSave0.ChangeParamData.uiBlackFeildNum-1;

	ucResult=XDATA_Read_Black_File(0);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memset(aucCmpBuf1,0,sizeof(aucCmpBuf1));
	aucCmpBuf[0]=DataSave1.BlackFeild.ucPtCode;
	aucCmpBuf1[0]=DataSave1.BlackFeild.ucPtCode;
	memcpy(&aucCmpBuf[1],DataSave1.BlackFeild.aucBeginCard,8);
	memcpy(&aucCmpBuf1[1],DataSave1.BlackFeild.aucEndCard,8);
	if(memcmp(InData,aucCmpBuf,9)<0)
		return SUCCESS;
	if((memcmp(InData,aucCmpBuf,9)>=0)&&(memcmp(InData,aucCmpBuf1,9)<=0))
		return ERR_INBLACKLIST;

	ucResult=XDATA_Read_Black_File(DataSave0.ChangeParamData.uiBlackFeildNum-1);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memset(aucCmpBuf1,0,sizeof(aucCmpBuf1));
	aucCmpBuf[0]=DataSave1.BlackFeild.ucPtCode;
	aucCmpBuf1[0]=DataSave1.BlackFeild.ucPtCode;
	memcpy(&aucCmpBuf[1],DataSave1.BlackFeild.aucBeginCard,8);
	memcpy(&aucCmpBuf1[1],DataSave1.BlackFeild.aucEndCard,8);
	if(memcmp(InData,aucCmpBuf1,9)>0)
		return SUCCESS;
	if((memcmp(InData,aucCmpBuf,9)>=0)&&(memcmp(InData,aucCmpBuf1,9)<=0))
		return ERR_INBLACKLIST;

	while(FirstPos<=LastPos)
	{
		MidPos=(FirstPos+LastPos)/2;
		ucResult=XDATA_Read_Black_File(MidPos);
		if(ucResult!=SUCCESS)
			return ucResult;
		aucCmpBuf[0]=DataSave1.BlackFeild.ucPtCode;
		aucCmpBuf1[0]=DataSave1.BlackFeild.ucPtCode;
		memcpy(&aucCmpBuf[1],DataSave1.BlackFeild.aucBeginCard,8);
		memcpy(&aucCmpBuf1[1],DataSave1.BlackFeild.aucEndCard,8);
		if(memcmp(InData,aucCmpBuf,9)<0)
			LastPos=MidPos-1;
		else if(memcmp(InData,aucCmpBuf1,9)>0)
			FirstPos=MidPos+1;
		else
			return ERR_INBLACKLIST;
	}
	return SUCCESS;
}

unsigned char CFG_CheckCardTable(unsigned char *InData,unsigned char ucpt,unsigned int uiCardBinLenth)
{
	unsigned char aucCmpBuf[20],ucI,ucResult=SUCCESS;
	short FirstPos,LastPos,MidPos;
	int i,j;
	Uart_Printf("ucpt=%d,uiCardBinLenth=%d\n",ucpt,uiCardBinLenth);
	Uart_Printf("uiCTNum=%d\n",DataSave0.ChangeParamData.uiCTNum[ucpt-1][uiCardBinLenth-5]);
	if(DataSave0.ChangeParamData.uiCTNum[ucpt-1][uiCardBinLenth-5]==0)
		return ERR_CARDUNTREAT;
	FirstPos=0;
	for(j=0;j<ucpt-1;j++)
	{
		for(i=0;i<8;i++)
			FirstPos+=DataSave0.ChangeParamData.uiCTNum[j][i];
	}
	for(i=0;i<uiCardBinLenth-5;i++)
	{
		FirstPos+=DataSave0.ChangeParamData.uiCTNum[j][i];
	}
	LastPos =FirstPos+DataSave0.ChangeParamData.uiCTNum[ucpt-1][uiCardBinLenth-5]-1;
	ucResult=XDATA_Read_CardTables_File(FirstPos);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(&aucCmpBuf[0],DataSave1.CardTable.aucCardBin,6);
	memcpy(&aucCmpBuf[6],DataSave1.CardTable.aucCardZone,3);
	if(memcmp(InData,aucCmpBuf,9)<0)
		return ERR_CARDUNTREAT;
	if(memcmp(aucCmpBuf,InData,9)==0)
	{
		return SUCCESS;
	}

	ucResult=XDATA_Read_CardTables_File(LastPos);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(&aucCmpBuf[0],DataSave1.CardTable.aucCardBin,6);
	memcpy(&aucCmpBuf[6],DataSave1.CardTable.aucCardZone,3);
	if(memcmp(InData,aucCmpBuf,9)>0)
		return ERR_CARDUNTREAT;
	if(memcmp(InData,aucCmpBuf,9)==0)
	{
		return SUCCESS;
	}

	while(FirstPos<=LastPos)
	{
		MidPos=(FirstPos+LastPos)/2;
		ucResult=XDATA_Read_CardTables_File(MidPos);
		if(ucResult!=SUCCESS)
			return ucResult;
		memset(aucCmpBuf,0,sizeof(aucCmpBuf));
		memcpy(&aucCmpBuf[0],DataSave1.CardTable.aucCardBin,6);
		memcpy(&aucCmpBuf[6],DataSave1.CardTable.aucCardZone,3);
		if(memcmp(InData,aucCmpBuf,9)<0)
			LastPos=MidPos-1;
		else if(memcmp(InData,aucCmpBuf,9)>0)
			FirstPos=MidPos+1;
		else
			return SUCCESS;
	}
	return ERR_CARDUNTREAT;
}

unsigned char CFG_CheckCardTable4Hand(unsigned char *InData,unsigned char ucpt,unsigned int uiCardBinLenth)
{
	unsigned char aucCmpBuf[20],ucI,ucResult=SUCCESS;
	short FirstPos,LastPos,MidPos;
	int i,j;
	Uart_Printf("ucpt=%d,uiCardBinLenth=%d\n",ucpt,uiCardBinLenth);
	Uart_Printf("uiCTNum=%d\n",DataSave0.ChangeParamData.uiCTNum[ucpt-1][uiCardBinLenth-5]);
	if(DataSave0.ChangeParamData.uiCTNum[ucpt-1][uiCardBinLenth-5]==0)
		return ERR_CARDUNTREAT;
	FirstPos=0;
	for(j=0;j<ucpt-1;j++)
	{
		for(i=0;i<8;i++)
			FirstPos+=DataSave0.ChangeParamData.uiCTNum[j][i];
	}
	for(i=0;i<uiCardBinLenth-5;i++)
	{
		FirstPos+=DataSave0.ChangeParamData.uiCTNum[j][i];
	}
	LastPos =FirstPos+DataSave0.ChangeParamData.uiCTNum[ucpt-1][uiCardBinLenth-5]-1;
	ucResult=XDATA_Read_CardTables_File(FirstPos);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(&aucCmpBuf[0],DataSave1.CardTable.aucCardBin,6);
	if(memcmp(InData,aucCmpBuf,6)<0)
		return ERR_CARDUNTREAT;
	if(memcmp(aucCmpBuf,InData,6)==0)
	{
		return SUCCESS;
	}

	ucResult=XDATA_Read_CardTables_File(LastPos);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(&aucCmpBuf[0],DataSave1.CardTable.aucCardBin,6);
	if(memcmp(InData,aucCmpBuf,6)>0)
		return ERR_CARDUNTREAT;
	if(memcmp(InData,aucCmpBuf,6)==0)
	{
		return SUCCESS;
	}

	while(FirstPos<=LastPos)
	{
		MidPos=(FirstPos+LastPos)/2;
		ucResult=XDATA_Read_CardTables_File(MidPos);
		if(ucResult!=SUCCESS)
			return ucResult;
		memset(aucCmpBuf,0,sizeof(aucCmpBuf));
		memcpy(&aucCmpBuf[0],DataSave1.CardTable.aucCardBin,6);
		if(memcmp(InData,aucCmpBuf,6)<0)
			LastPos=MidPos-1;
		else if(memcmp(InData,aucCmpBuf,6)>0)
			FirstPos=MidPos+1;
		else
			return SUCCESS;
	}
	return ERR_CARDUNTREAT;
}


unsigned char CFG_CheckCardTbl(unsigned char *InData)
{
	unsigned char 	aucCmpBuf[20],aucBuf[100],ucResult=SUCCESS;
	int	ucI,i;
	unsigned short	usBinLen;
	if(DataSave0.ChangeParamData.uiCardTableNum==0)
		return ERR_CARDUNTREAT;
	
	for(ucI=12;ucI>=5;ucI--)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,InData,ucI);
		asc_bcd(aucCmpBuf,6,aucBuf,12);
		memcpy(&aucCmpBuf[6],NormalTransData.aucZoneCode,3);
		ucResult=CFG_CheckCardTable(aucCmpBuf,NormalTransData.ucPtCode,ucI);
		if(ucResult==SUCCESS)
			break;
		
	}
	if(ucResult==SUCCESS)
	{
		if(DataSave1.CardTable.ucTableStatus=='1')
			return ERR_CARDUNTREAT;
		
		/*if(NormalTransData.ucCardType!=DataSave1.CardTable.ucCardMedia)
			return ERR_MEDIADIFFRENT;*/
		NormalTransData.ulYaAmount=bcd_long(&DataSave1.CardTable.ucYaAmount,2)*100;
		UTIL_BitFlagAnalyze(DataSave1.CardTable.aucFunFlag,4,aucBuf);
		memcpy(RunData.aucFunFlag,aucBuf,32);
		if(RunData.aucFunFlag[1])
		{
			RunData.ucMustReturn=0;
			NormalTransData.ucZeroFlag=0;
			if(NormalTransData.ulPrevEDBalance<NormalTransData.ulYaAmount)
			{
				if(NormalTransData.ulPrevEDBalance!=0)
					RunData.ucMustReturn=1;
				else
					NormalTransData.ucZeroFlag=1;
				NormalTransData.ulPrevEDBalance=0;
			}
			else
				NormalTransData.ulPrevEDBalance-=NormalTransData.ulYaAmount;
		}
		NormalTransData.ulMaxAmount=bcd_long(DataSave1.CardTable.aucMaxValue,6)*100;
		//NormalTransData.ulCardValue=bcd_long(DataSave1.CardTable.aucCardValue,4)*100;
		//Uart_Printf("ulCardValue=%d\n",NormalTransData.ulCardValue);
		return SUCCESS;
	}
	return ERR_CARDUNTREAT;
}
unsigned char CFG_CheckCardTbl4Hand(void)
{
	unsigned char ucResult=SUCCESS,aucCheck[20],ucI,aucCmpBuf[20],ucJ,aucBuf[20];
	if(DataSave0.ChangeParamData.uiCardTableNum==0)
		return ERR_CARDUNTREAT;
	
	memset(aucCheck,0,sizeof(aucCheck));
	bcd_asc(aucCheck,NormalTransData.aucUserCardNo,20);
	/*ivo=bcd_long(&DataSave1.ZoneCtrl.ucVerifyOffset,2);
	if(ivo)
		memcpy(&aucCheck[ivo-1],&aucCheck[ivo],16-ivo);*/
	for(ucJ=1;ucJ<4;ucJ++)
	{
		for(ucI=12;ucI>=5;ucI--)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,aucCheck,ucI);
			asc_bcd(aucCmpBuf,6,aucBuf,12);
			ucResult=CFG_CheckCardTable4Hand(aucCmpBuf,ucJ,ucI);
			if(ucResult==SUCCESS)
				break;
		}
		if(ucResult==SUCCESS)
			break;
	}
	if(ucResult==SUCCESS)
	{
		if(DataSave1.CardTable.ucTableStatus=='1')
			return ERR_CARDUNTREAT;
		UTIL_BitFlagAnalyze(DataSave1.CardTable.aucFunFlag,4,aucBuf);
		memcpy(RunData.aucFunFlag,aucBuf,32);
		memcpy(NormalTransData.aucFunFlag,RunData.aucFunFlag,32);
		NormalTransData.ucPtCode=ucJ;
		memcpy(NormalTransData.aucZoneCode,DataSave1.CardTable.aucCardZone,3);
		NormalTransData.ucCardType=DataSave1.CardTable.ucCardMedia;
	}
	memset(aucCheck,0,sizeof(aucCheck));
	aucCheck[0]=NormalTransData.ucPtCode;
	memcpy(&aucCheck[1],NormalTransData.aucZoneCode,4);
	ucResult=CFG_CheckCardZone(aucCheck);
	if(ucResult!=SUCCESS)	
	{
		Uart_Printf("check cardtabel err=%02x\n",ucResult);
		return ucResult;
	}
	UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucCardValue,2,RunData.aucWDFunFlag);
	UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,RunData.aucZDFunFlag);
	UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucExFunction,8,RunData.aucExFunFlag);
	if(DataSave1.ZoneCtrl.ucValFlag&0xF0)
		RunData.ucWeiKFlag=1;
	if(DataSave1.ZoneCtrl.ucValFlag&0x0F)
		RunData.ucWanTFlag=1;
	memcpy(NormalTransData.aucZoneName,DataSave1.ZoneCtrl.aucWaikangName,16);
	memcpy(NormalTransData.aucZoneName1,DataSave1.ZoneCtrl.aucHopassName,16);
	return ucResult;
}
unsigned char CFG_CheckRate(unsigned char *InData)
{
	unsigned char aucCmpBuf[20],ucI,ucResult=SUCCESS,aucInData[10];
	short FirstPos,LastPos,MidPos;
	int i;
	NormalTransData.ulRateAmount=0;
	RunData.ucRateType = 0;
	RunData.ucRateType1 =0;
	RunData.ulRate=0;
	RunData.ulMaxRate=0;
	RunData.ulMinRate=0;
	if( DataSave0.ChangeParamData.uiRateTableNum==0)
		return SUCCESS;
	aucInData[0]=NormalTransData.ucPtCode;
	memcpy(&aucInData[1],InData,4);
	FirstPos=0;
	LastPos = DataSave0.ChangeParamData.uiRateTableNum-1;
	Uart_Printf("DataSave0.ChangeParamData.uiRateTableNum=%d\n",DataSave0.ChangeParamData.uiRateTableNum);
	ucResult=XDATA_Read_Rate_File(0);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(aucCmpBuf,&DataSave1.RateTable.ucPtCode,5);
	if(memcmp(aucInData,aucCmpBuf,5)<0)
		return SUCCESS;
	if(memcmp(aucInData,aucCmpBuf,5)==0)
	{
		if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax1,6)*100
			&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin1,6)*100)
		{
			RunData.ucRateType = DataSave1.RateTable.ucSubType1;
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate1,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		else
		if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax2,6)*100
			&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin2,6)*100)
		{
			RunData.ucRateType = DataSave1.RateTable.ucSubType2;
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate2,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		else
		if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax3,6)*100
			&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin3,6)*100)
		{
			RunData.ucRateType = DataSave1.RateTable.ucSubType3;
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate3,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		ucI=DataSave1.RateTable.ucRateType[0];
		RunData.ucRateType = ((ucI&0x80)>>7)+'0';
		ucI=DataSave1.RateTable.ucRateType[0];
		RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
		ucI=DataSave1.RateTable.ucRateType[0];
		RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
		//RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate,8);
		RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
		RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
		return SUCCESS;
	}

	ucResult=XDATA_Read_Rate_File(DataSave0.ChangeParamData.uiRateTableNum-1);
	if(ucResult!=SUCCESS)
		return ucResult;
	memset(aucCmpBuf,0,sizeof(aucCmpBuf));
	memcpy(aucCmpBuf,&DataSave1.RateTable.ucPtCode,5);
	for(i=0;i<5;i++)
		Uart_Printf("%02x ",aucCmpBuf[i]);
	Uart_Printf("\n");
	if(memcmp(aucInData,aucCmpBuf,5)>0)
		return SUCCESS;
	if(memcmp(aucInData,aucCmpBuf,5)==0)
	{
		Uart_Printf("NormalTransData.ulAmount=%d\n",NormalTransData.ulAmount);
		for(i=0;i<3;i++)
			Uart_Printf("%02x ",DataSave1.RateTable.aucMax1[i]);
		Uart_Printf("\n");
		for(i=0;i<3;i++)
			Uart_Printf("%02x ",DataSave1.RateTable.aucMin1[i]);
		Uart_Printf("\n");
		if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax1,6)*100
			&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin1,6)*100)
		{
			RunData.ucRateType = DataSave1.RateTable.ucSubType1;
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate1,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		else
		if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax2,6)*100
			&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin2,6)*100)
		{
			RunData.ucRateType = DataSave1.RateTable.ucSubType2;
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate2,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		else
		if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax3,6)*100
			&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin3,6)*100)
		{
			RunData.ucRateType = DataSave1.RateTable.ucSubType3;
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate3,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		ucI=DataSave1.RateTable.ucRateType[0];
		RunData.ucRateType = ((ucI&0x80)>>7)+'0';
		ucI=DataSave1.RateTable.ucRateType[0];
		RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
		ucI=DataSave1.RateTable.ucRateType[0];
		RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
		//RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate,6);
		RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
		RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
		return SUCCESS;
	}


	while(FirstPos<=LastPos)
	{
		MidPos=(FirstPos+LastPos)/2;
		ucResult=XDATA_Read_Rate_File(MidPos);
		if(ucResult!=SUCCESS)
			return ucResult;
		memset(aucCmpBuf,0,sizeof(aucCmpBuf));
		memcpy(aucCmpBuf,&DataSave1.RateTable.ucPtCode,5);
		if(memcmp(aucInData,aucCmpBuf,5)==0)
		{
			if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax1,6)*100
				&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin1,6)*100)
			{
				RunData.ucRateType = DataSave1.RateTable.ucSubType1;
				ucI=DataSave1.RateTable.ucRateType[0];
				RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
				ucI=DataSave1.RateTable.ucRateType[0];
				RunData.ucRateType2= ((ucI&0x20)>>5)+'0';
				RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate1,6);
				RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
				RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
				return SUCCESS;
			}
			else
			if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax2,6)*100
				&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin2,6)*100)
			{
				RunData.ucRateType = DataSave1.RateTable.ucSubType2;
				ucI=DataSave1.RateTable.ucRateType[0];
				RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
				ucI=DataSave1.RateTable.ucRateType[0];
				RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
				RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate2,6);
				RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
				RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
				return SUCCESS;
			}
			else
			if(NormalTransData.ulAmount<=bcd_long(DataSave1.RateTable.aucMax3,6)*100
				&&NormalTransData.ulAmount>=bcd_long(DataSave1.RateTable.aucMin3,6)*100)
			{
				RunData.ucRateType = DataSave1.RateTable.ucSubType3;
				ucI=DataSave1.RateTable.ucRateType[0];
				RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
				ucI=DataSave1.RateTable.ucRateType[0];
				RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
				RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate3,6);
				RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
				RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
				return SUCCESS;
			}
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType = ((ucI&0x80)>>7)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType1 = ((ucI&0x40)>>6)+'0';
			ucI=DataSave1.RateTable.ucRateType[0];
			RunData.ucRateType2 = ((ucI&0x20)>>5)+'0';
			//RunData.ulRate=bcd_long(DataSave1.RateTable.aucRate,6);
			RunData.ulMaxRate=bcd_long(DataSave1.RateTable.aucMax,6);
			RunData.ulMinRate=bcd_long(DataSave1.RateTable.aucMin,6);
			return SUCCESS;
		}
		if(memcmp(aucInData,aucCmpBuf,5)<0)
			LastPos=MidPos-1;
		else if(memcmp(aucInData,aucCmpBuf,5)>0)
			FirstPos=MidPos+1;

	}
	return SUCCESS;
}

unsigned char  CFG_SetBatchNumber(void)
{
	unsigned char ucResult=SUCCESS;
	CFG_GetDateTime();
	if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
	{
		if(memcmp(DataSave0.ChangeParamData.aucBatchNumber,
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",12)==0)
		{
			memcpy(DataSave0.ChangeParamData.aucBatchNumber,&NormalTransData.aucDateTime[1],6);
			memcpy(&DataSave0.ChangeParamData.aucBatchNumber[6],&NormalTransData.aucUserCardNo[2],6);
			ucResult=XDATA_Write_Vaild_File(DataSaveChange);
		}	
	}
	return ucResult;
}


unsigned char   CFG_SetBatchNumber1(void)
{
	if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
	{
		if(memcmp(DataSave0.ChangeParamData.aucBatchNumber,
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",12)==0)
		{
			CFG_GetDateTime();
			memcpy(DataSave0.ChangeParamData.aucBatchNumber,&NormalTransData.aucDateTime[1],6);
			asc_bcd(&DataSave0.ChangeParamData.aucBatchNumber[6],4,DataSave0.ConstantParamData.aucTerminalID,8);
			memcpy(&DataSave0.ChangeParamData.aucBatchNumber[10],"\x00\x00",2);
			return XDATA_Write_Vaild_File(DataSaveChange);
		}	
	}
	return SUCCESS;
}

void CFG_GetDateTime(void)
{
	unsigned char aucBuf[10];
	
	memset(aucBuf,0,sizeof(aucBuf));
	Os__read_time_sec(aucBuf);//hhmmss
	asc_bcd(&NormalTransData.aucDateTime[4],3,aucBuf,6);
	NormalTransData.aucDateTime[0]=0x20;
	memset(aucBuf,0,sizeof(aucBuf));
	Os__read_date(aucBuf);	//\xDD\xMM\xYY;
	asc_bcd(&NormalTransData.aucDateTime[1],1,&aucBuf[4],2);
	asc_bcd(&NormalTransData.aucDateTime[2],1,&aucBuf[2],2);
	asc_bcd(&NormalTransData.aucDateTime[3],1,&aucBuf[0],2);	
}

unsigned char CFG_ChkExpireValid(const unsigned char *InDate)
{
	unsigned short 	uiYear,uiMonths,uiDate;
	
	CFG_GetDateTime();
	if(memcmp(&NormalTransData.aucDateTime[0],InDate,4)>0)
			return ERR_EXPIREDATE;
	uiYear=(unsigned short)bcd_long((uchar *)InDate,4);
	uiMonths=(unsigned short)bcd_long((uchar *)InDate+2,2);
	uiDate=(unsigned short)bcd_long((uchar *)(InDate+3),2);
	switch(uiMonths)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:	
		if(uiDate>31) 
			return ERR_EXPIREDATE;
		break;
	case 2:
		if((uiYear%4==0 && uiYear%100!=0)||(uiYear%400==0))
		{
			if(uiDate>29)
				return ERR_EXPIREDATE;
		}else
		{
			if(uiDate>28)
				return ERR_EXPIREDATE;
		}
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		if(uiDate>30)
			return ERR_EXPIREDATE;
		break;
	default:
		return ERR_EXPIREDATE;
	}	
	return SUCCESS;
}



unsigned char CFG_CalcPreLoadTAC(void)
{
	unsigned char aucBuf[100];
	unsigned short offset=0;
	unsigned char ucResult=SUCCESS;
	unsigned long ulAmount;
	
	memset(aucBuf,0,sizeof(aucBuf));
	//
	long_tab(&aucBuf[offset],4,&NormalTransData.ulAmount);	offset +=4;
	//Ӧͱʶ
	aucBuf[offset]=0x62;
	offset += 1;
	//ն˱
	asc_bcd(&aucBuf[offset],4,DataSave0.ConstantParamData.aucOnlytermcode,8);	offset +=4;
	//
	long_bcd(&aucBuf[offset+1],3,&NormalTransData.ulTraceNumber);
	offset +=4;
	//ʱ
	memcpy(&aucBuf[offset],NormalTransData.aucDateTime,7);	offset +=7;

	ucResult=UTIL_CalcSandTAC(aucBuf,offset,NormalTransData.aucTraceTac);
	
	return ucResult;
}


unsigned char CFG_CalcPurhaseTAC(void)
{
	unsigned char aucBuf[100];
	unsigned short offset=0;
	unsigned char ucResult=SUCCESS;						
	
	memset(aucBuf,0,sizeof(aucBuf));
	//
	long_tab(&aucBuf[offset],4,&NormalTransData.ulAmount);	offset +=4;
	//Ӧͱʶ
	aucBuf[offset]=0x21;
	offset += 1;
	//ն˱
	asc_bcd(&aucBuf[offset],4,DataSave0.ConstantParamData.aucOnlytermcode,8);	offset +=4;
	//
	long_bcd(&aucBuf[offset+1],3,&NormalTransData.ulTraceNumber);
	offset +=4;
	//ʱ
	memcpy(&aucBuf[offset],NormalTransData.aucDateTime,7);	offset +=7;
	
	ucResult=UTIL_CalcSandTAC(aucBuf,offset,NormalTransData.aucTraceTac);
	
	return ucResult;
}



unsigned char CFG_QueryList(void)
{
	unsigned short	uiIndex,uioldIndex;
	
	if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
	{
#ifdef GUI_PROJECT
        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"空交易！",7);
        sleep(1);
#else
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"ս");
		UTIL_GetKey(3);
#endif
		return SUCCESS;
	}
	uiIndex=0;
	uioldIndex=0;
	while(1)
	{
		unsigned char ch;
		Uart_Printf("uiIndex=%d\n",uiIndex);
		if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==1)
		{
			Uart_Printf("88888888888888888888888888888888888\n");
			memset(&NormalTransData,0,sizeof(NORMALTRANS));
		   	XDATA_Read_SaveTrans_File(uiIndex);
			UTIL_DisplayTrans();
		}else
		{
			if(ch==KEY_F3)
			{
				if(uiIndex>0)
				{
					uiIndex--;
					continue;
				}
			}
			if(uiIndex<TRANS_MAXNB-1)
			{
				uiIndex++;
				continue;
			}

			uiIndex=uioldIndex;
			continue;
		}
		ch=Os__xget_key();
		if(ch==KEY_CLEAR)
			return SUCCESS;
		else if (ch==KEY_F3)
		{
			if(uiIndex>0)
			{
				uioldIndex=uiIndex;
				uiIndex--;
			}
			continue;
		}else if((ch==KEY_ENTER)||(ch==KEY_F4))
		{
			if(uiIndex<TRANS_MAXNB-1)
			{
				uioldIndex=uiIndex;
				uiIndex++;
			}
			continue;
		}
	}
	return SUCCESS;
}
unsigned char CFG_PrintTables(void)
{
	int i;
	unsigned char aucBuf[40];
	Os__xprint((uchar*)"Zone:");
	for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
	{
		XDATA_Read_ZoneCtrls_File(i);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,&DataSave1.ZoneCtrl.ucPtCode,8);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,DataSave1.ZoneCtrl.aucCardValue,24);
		Os__xprint(aucBuf);
		
	}
	Os__xprint((uchar*)"CardTable:");
	for(i=0;i<DataSave0.ChangeParamData.uiCardTableNum;i++)
	{
		XDATA_Read_CardTables_File(i);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[0]='*';
		hex_asc(&aucBuf[1],&DataSave1.CardTable.ucPtCode,14);
		hex_asc(&aucBuf[16],&DataSave1.CardTable.ucCardBinLen,2);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,DataSave1.CardTable.aucCardBin,12);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,DataSave1.CardTable.aucCardValue,22);
		Os__xprint(aucBuf);
		
	}
	Os__xprint((uchar*)"Black:");
	for(i=0;i<DataSave0.ChangeParamData.uiBlackFeildNum;i++)
	{
		XDATA_Read_Black_File(i);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,DataSave1.BlackFeild.aucBeginCard,16);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,DataSave1.BlackFeild.aucEndCard,16);
		Os__xprint(aucBuf);
		
	}
	Os__xprint((uchar*)"Rate:");
	for(i=0;i<DataSave0.ChangeParamData.uiRateTableNum;i++)
	{
		XDATA_Read_Rate_File(i);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,DataSave1.RateTable.aucCardZone,24);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,&DataSave1.RateTable.ucSubType1,20);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,&DataSave1.RateTable.ucSubType2,20);
		Os__xprint(aucBuf);
		memset(aucBuf,0,sizeof(aucBuf));
		hex_asc(aucBuf,&DataSave1.RateTable.ucSubType3,20);
		Os__xprint(aucBuf);
		
	}
	Os__xlinefeed(5*LINENUM);
	return(SUCCESS);
}

int CFG_AutoSettle(void)
{
	unsigned char	aucCheckDT[5],aucDateTime[5],aucSettleCycle[6]; 
	unsigned short	uiYear,uiI,uiH,uiD,uiM,uiY;
	unsigned short	uiDays;
	unsigned char   ucResult=SUCCESS;
	struct seven even;
	bcd_asc(aucSettleCycle,DataSave0.ChangeParamData.aucSettleCycle,6);
	if((DataSave0.ChangeParamData.ucIsLogonFlag==FLAG_LOGON_OK)
		&&(DataSave0.ChangeParamData.ucSettleFlag!=FLAG_SETTLE_FAIL))
	{
		uiDays = (unsigned short)asc_long(aucSettleCycle,2);
#ifdef TEST
Uart_Printf("uiDays=%d\n",uiDays);
#endif
		uiYear=(unsigned short)bcd_long(DataSave0.ChangeParamData.aucLogonDateTime,4);
		memset(aucDateTime,0,sizeof(aucDateTime));
		//yymmddhh
		memcpy(aucDateTime,&DataSave0.ChangeParamData.aucLogonDateTime[1],4);
		uiD=(unsigned short)bcd_long(&aucDateTime[2],2);
		uiM=(unsigned short)bcd_long(&aucDateTime[1],2);
		uiY=(unsigned short)bcd_long(&aucDateTime[0],2);
#ifdef TEST
		Uart_Printf("uiYear=%d uiD=%d uiM=%d uiY=%d \n",uiYear,uiD,uiM,uiY);
#endif	
		
		uiD += uiDays;
		switch(uiM)
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:	
			while(uiD>31)
			{
				uiD -= 31;
				uiM += 1;
			}
			break;
		case 2:
			if((uiYear%4==0 && uiYear%100!=0)||(uiYear%400==0))
			{
				while(uiD>29)
				{
					uiD -= 29;	
					uiM += 1;
				}
			}else
			{
				while(uiD>28)
				{
					uiD -= 28;
					uiM += 1;	
				}
			}
			break;
		default:
			while(uiD>30)
			{
				uiD -= 30;
				uiM += 1;
			}
			break;
		}
		
		if(uiM>12)
		{
			uiY +=1;
			uiM -=12;
		}
		CFG_GetDateTime();
		if(uiDays==0)return 0;
		memset(aucDateTime,0,sizeof(aucDateTime));
		short_bcd(aucDateTime,1,&uiY);
		short_bcd(&aucDateTime[1],1,&uiM);
		short_bcd(&aucDateTime[2],1,&uiD);
		asc_bcd(&aucDateTime[3],1,&aucSettleCycle[2],2);
		asc_bcd(&aucDateTime[4],1,&aucSettleCycle[4],2);
		
#ifdef TEST
		Uart_Printf("aucDateTime=%02x %02x %02x %02x\n",aucDateTime[0],aucDateTime[1],aucDateTime[2],aucDateTime[3]);
		Uart_Printf("LocalDateTime=%02x %02x %02x %02x\n",NormalTransData.aucDateTime[1],NormalTransData.aucDateTime[2],
												NormalTransData.aucDateTime[3],NormalTransData.aucDateTime[4]);
#endif		
			
		if(memcmp(&NormalTransData.aucDateTime[1],aucDateTime,5)>=0)
		{
			DataSave0.ChangeParamData.ucSettleFlag=FLAG_SETTLE_FAIL;
			ucResult=XDATA_Write_Vaild_File(DataSaveChange);
			if(DataSave0.ChangeParamData.aucTermFlag[9]&&DataSave0.ConstantParamData.ucYTJFlag!=2)
			{
				even.event_type = EVESEL;
				even.event_nr = EVE_MAN;
//				OSAPP_EventRun(&even);
				return 1;
			}
		}

		
	}
	return 0;
}
unsigned char CFG_NetTest(void)
{
	 unsigned short  uI,uiLen;
	 unsigned int  iOffset;
	 unsigned long  iTrace;
	 unsigned char  ucResult;
	 unsigned char aucBuf[200],aucBufOut[200],aucIPV4Addr[5];
	unsigned char acIp[20],Lrc;
	NormalTransData.ucTransType=TRANS_S_NETTEST;
	iTrace=1;
	do
	{
		iOffset=0;
		Lrc=0;
		aucBuf[iOffset]=0x00;iOffset+=1;
		aucBuf[iOffset]=0x71;iOffset+=1;
		aucBuf[iOffset]=0xFE;iOffset+=1;
		aucBuf[iOffset]=0xEF;iOffset+=1;
		aucBuf[iOffset]=0x30;iOffset+=1;
		aucBuf[iOffset]=0x31;iOffset+=1;
		long_asc(&aucBuf[iOffset],4,&iTrace);iOffset+=4;
		aucBuf[iOffset]=0x30;iOffset+=1;
		aucBuf[iOffset]=0x31;iOffset+=1;
		iOffset+=25;
		memcpy(aucIPV4Addr,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd1,4);
		sprintf((char *)acIp,"%3d.%3d.%3d.%3d:",aucIPV4Addr[0],aucIPV4Addr[1],aucIPV4Addr[2],aucIPV4Addr[3]);
		memcpy(&aucBuf[iOffset],acIp,16);iOffset+=16;
		int_asc(&aucBuf[iOffset],6,&DataSave0.ConstantParamData.uiHostPort1);iOffset+=6;
		iOffset+=10;
		for(uI=0;uI<iOffset;uI++)
		{
			Lrc^=aucBuf[uI];
		}
		aucBuf[iOffset]=Lrc;iOffset+=1;
		uiLen=150;
   		ucResult = COMMS_SendReceiveNetTest(aucBuf,iOffset,aucBufOut,&uiLen,IPAddress,HostPort);
		if(ucResult!=SUCCESS)
			break;
		iTrace+=1;
		Os__xprint(&aucBufOut[7]);
	}while(aucBufOut[6]=='1');
	return ucResult;
}
unsigned char MAC_Load_Key(void)
{
	 unsigned int  uI;
	 unsigned char  ucResult;
	 unsigned char ucMasterKey[2][9]={
										"\xc0\xee\xd3\xf1\xb4\xa8\xd0\xb4",
										"\xb5\xc4\xce\xa1\xbf\xb5\xbf\xa8",
	                                 } ;

	 //Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"MACնԿ");
	//Os__GB2312_display(1,0,(unsigned char *)"   Ժ....");
	 
	//"\xC0\xEE\xD3\xF1\xB4\xA8\xD0\xB4\xB5\xC4\xCE\xA1\xBF\xB5\xBF\xA8" 
	for (uI=0;uI<2;uI++)
	{
		memcpy(DataSave0.ConstantParamData.MacMasterKey[uI],ucMasterKey[uI],8);
	}    
        Os__saved_set(&DataSave0.ConstantParamData.Pinpadflag,0,1);
	ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
	
    return(ucResult);
}

