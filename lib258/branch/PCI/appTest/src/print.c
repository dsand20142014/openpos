#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <oslib.h>
#include <fsync_drvs.h>

unsigned char PRINT_Deal(void)
{
	unsigned char i;
	unsigned char aucBuf[17];
	NEW_DRV_TYPE new_drv;
	  
	for(i=0;i<PRINTTIMES;i++)
    	{		    		
	    	if(i>0)
	    	{
		    	Os__clr_display(0);
		    	Os__clr_display(1);
		    	Os__clr_display(2);
		    	Os__clr_display(3);
		    	Os__clr_display(4);
		    	Os__clr_display(5);
		    	Os__clr_display(6);
		    	Os__clr_display(7);
		    	Os__clr_display(8);
		    	Os__GB2312_display(1,0,( unsigned char *)"Please tear tickets");
		    	Os__GB2312_display(2,0,( unsigned char *)"press anykey to continue");
		    	//UTIL_GetKey(5);
			Os_Wait_Event(KEY_DRV, &new_drv, 100000);
	    	}
	    	PRINT_Ticket(i);	
    	}
	return SUCCESS;
}



unsigned char PRINT_Ticket(unsigned char ucFlag)
{
	   unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
     unsigned char aucValue[PRINT_MAXCHAR+1];
    unsigned char index;
    unsigned short j,k,i;
    unsigned char temp[10];
    unsigned long ulAmount;
     unsigned char ucI;
	   unsigned long ulL;
	   unsigned char aucBuf[7];
	   unsigned char ucJ;
	   unsigned char DateTime[19];
	   
	   //Os__light_off();


	       if(!ucFlag)
	       {
	                  Os__GB2312_xprint((unsigned char *)"         商户联",FONT);
	       }else
	       {
	                  Os__GB2312_xprint((unsigned char *)"         客户联",FONT);
	       }
	        Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"   ",FONT);
		  
               memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"终端号:",7);
		memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"商户号:",7);
		memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"商户名称:",9);
		memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantName,
		strlen((char *)DataSave0.ConstantParamData.aucMerchantName));
		Os__GB2312_xprint(aucPrintBuf,FONT);

		


		
              
		
		Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	    	memcpy(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
		Os__GB2312_xprint((unsigned char *)"持卡人卡号：",FONT);
	    	Os__GB2312_xprint(aucPrintBuf,FONT);

              
		
                memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	    	memcpy(aucPrintBuf,"流水号:",7);
	    	long_asc(&aucPrintBuf[7],6,&G_NORMALTRANS_ulTraceNumber);
	    	aucPrintBuf[24] = 0;
	        Os__GB2312_xprint(aucPrintBuf,FONT);

			
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"有效期:",7);
		memcpy(&aucPrintBuf[7],&G_NORMALTRANS_ucExpiredDate[0],2);
		aucPrintBuf[9] = '/';
		memcpy(&aucPrintBuf[10],&G_NORMALTRANS_ucExpiredDate[2],2);
	    	Os__GB2312_xprint(aucPrintBuf,FONT);
   		Os__GB2312_xprint((unsigned char*)"  日期           时间",FONT);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	    	UTIL_format_date_bcd_str(aucPrintBuf,G_NORMALTRANS_aucDate);
	    	UTIL_format_time_bcd_str(&aucPrintBuf[16],G_NORMALTRANS_aucTime);
	    	aucPrintBuf[24] = 0;
		Os__GB2312_xprint(aucPrintBuf,FONT);
		Os__GB2312_xprint((unsigned char *)"交易类型:消费",FONT);
		if((G_NORMALTRANS_aucAuthCode[0]!= 0)&&((G_NORMALTRANS_aucAuthCode[0]!= 0x20)))
		{
		         memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"授权号:",7);
			memcpy(&aucPrintBuf[7],G_NORMALTRANS_aucAuthCode,TRANS_AUTHCODELEN);
		    	Os__GB2312_xprint(aucPrintBuf,FONT);
		    
		}
		
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"参考号:",7);
		memcpy(&aucPrintBuf[7],G_NORMALTRANS_aucRefNum,TRANS_REFNUMLEN);
	    	Os__GB2312_xprint(aucPrintBuf,FONT);
			
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		    memset(aucValue,0,sizeof(aucValue));    
		    memcpy(aucPrintBuf,"交易金额:",9);
		    UTIL_Form_Montant(aucValue,G_NORMALTRANS_ulAmount,2);
		    memcpy(&aucPrintBuf[9],aucValue,11);  
		
		Os__GB2312_xprint(aucPrintBuf,FONT);
		Os__GB2312_xprint((unsigned char *)"本人同意支付上述款项",FONT);
		if((ucFlag == 0)||(ucFlag == 2))
		    {
		    	   Os__GB2312_xprint((unsigned char *)"持卡人签名:",FONT); 
		        Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"   ",FONT);
		    }
		Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
		
	        Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"   ",FONT);
		Os__GB2312_xprint((unsigned char *)"   ",FONT);

	    return SUCCESS;
		  
}


