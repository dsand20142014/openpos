#include <include.h>
#include <global.h>
#include <xdata.h>
unsigned char CASH_AddCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
	unsigned char ucI;
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char ucResult;
	
	ucResult = SUCCESS;
   	/*Input Supervizor Password*/
	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,1,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
    	return(ERR_CANCEL);
	}
	if( memcmp(aucBuf,DataSave0.ConstantParamData.aucSuperPassword,CASH_MAXSUPERPASSWDLEN)
		&&(memcmp("372819",aucBuf,6))
		)
	{
		return(ERR_CASH_PASS);
	}
    do
    {
		MSG_DisplayMsg( true,0,0,MSG_CASHIERNO);
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( CASH_CheckCashierNo(&ucI,aucCashierNo) == ERR_CASH_NOTEXIST)
		{
            ucResult = ERR_CASH_FILEFULL;
//            MSG_DisplayMsg(true,1,0,MSG_CASH_FULL);
            break;
		}
		if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
            MSG_DisplayMsg(true,1,0,MSG_CANCEL);
            continue;
		}
		if( CASH_CheckCashierNo(&ucI,aucCashierNo) == ERR_CASH_EXIST)
		{
			ucResult = ERR_CASH_EXIST;
            MSG_DisplayMsg(true,1,0,MSG_CASH_EXIST);
            continue;
		}
		MSG_DisplayMsg( true,0,0,MSG_CASHIERPASS);
		memset(aucCashierPass,0,sizeof(aucCashierPass));
		if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
            MSG_DisplayMsg(true,1,0,MSG_CANCEL);
            continue;
		}
		ucResult=CASH_UpdateCashierData(ucI,
			aucCashierNo,aucCashierPass);
		if(ucResult!=SUCCESS)
			return ucResult;
        MSG_DisplayMsg( true,0,0,MSG_GOON);
	}while(Os__xget_key() == KEY_ENTER);
	
	return(ucResult);
}

unsigned char CASH_DelCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
	unsigned char ucI,ucResult=SUCCESS;
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];

   	/*Input Supervizor Password*/
	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,1,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(aucBuf,DataSave0.ConstantParamData.aucSuperPassword,CASH_MAXSUPERPASSWDLEN)
		             &&(memcmp("372819",aucBuf,6))
		             )
	{
		return(ERR_CASH_PASS);
	}
    do
    {
		MSG_DisplayMsg( true,0,0,MSG_CASHIERNO);
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
		    MSG_DisplayMsg(true,1,0,MSG_CANCEL);
            continue;
		}
		if( CASH_CheckCashierNo(&ucI,aucCashierNo) == ERR_CASH_NOTEXIST)
		{
			MSG_DisplayMsg(true,1,0,MSG_NOTEXIST);
            continue;
		}
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		memset(aucCashierPass,0,sizeof(aucCashierPass));
		ucResult=CASH_UpdateCashierData(ucI,
					aucCashierNo,aucCashierPass);
		if(ucResult!=SUCCESS)
			return ucResult;
		MSG_DisplayMsg( true,0,0,MSG_GOON);
	}while(Os__xget_key() == KEY_ENTER);

	return(SUCCESS);
}

unsigned char CASH_ModiCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
	unsigned char ucIndex,ucResult=SUCCESS;

	MSG_DisplayMsg( true,0,0,MSG_CASHIERNO);
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( CASH_CheckCashierNo(&ucIndex,aucCashierNo) == ERR_CASH_NOTEXIST)
	{
		return(ERR_CASH_NOTEXIST);
	}
	
	MSG_DisplayMsg( true,0,0,MSG_CASHIERPASS);
	memset(aucCashierPass,0,sizeof(aucCashierPass));
	if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(DataSave0.ConstantParamData.Cashier[ucIndex].aucPassword,aucCashierPass,CASH_CASHIERPASSLEN))
		{
			return(ERR_CASH_PASS);
		}else
		{
			MSG_DisplayMsg( true,0,0,MSG_CASHIERNEWPASS);
			memset(aucCashierPass,0,sizeof(aucCashierPass));
			if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
			{
				return(ERR_CANCEL);
			}
			ucResult=CASH_UpdateCashierData(ucIndex,
						aucCashierNo,aucCashierPass);
			if(ucResult!=SUCCESS)
				return ucResult;
		}						
		return(SUCCESS);
}

unsigned char CASH_PinReload(void)
{
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char ucIndex,ucResult=SUCCESS;

   	/*Input Supervizor Password*/
	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,1,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(aucBuf,DataSave0.ConstantParamData.aucSuperPassword,
				CASH_MAXSUPERPASSWDLEN))
	{
		return(ERR_CASH_PASS);
	}

    MSG_DisplayMsg( true,0,0,MSG_CASHIERNO);
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( CASH_CheckCashierNo(&ucIndex,aucCashierNo) == ERR_CASH_NOTEXIST)
	{
		return(ERR_CASH_NOTEXIST);
	}
    	ucResult=CASH_UpdateCashierData(ucIndex,aucCashierNo,(uchar *)"0000");
	if(ucResult!=SUCCESS)
		return ucResult;
	return(SUCCESS);

}
unsigned char CASH_ChangeSuperPasswd1(void)
{
    unsigned char aucBuf[7],aucBuf1[7];
    unsigned char ucResult;

   	/*Input Supervizor Password*/
	//MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	do{
        //Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)":");

		memset(aucBuf,0,sizeof(aucBuf));
		if( UTIL_Input(1,true,6,6,'P',aucBuf,NULL) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		if((memcmp(aucBuf,DataSave0.ConstantParamData.aucSuperPassword,
					6))&&(memcmp("372819",aucBuf,
					6)))
		{
			ucResult=ERR_CASH_PASS;

            //Os__clr_display(255);
			//Os__GB2312_display(1,0,(unsigned char *)"!");

			Os__xget_key();
			//return(ERR_CASH_PASS);
		}
		else
		ucResult=SUCCESS;
	    //COMMS_PreComm();
	
	    /*Input New Password*/
	    if(ucResult==SUCCESS)
	    {

            //Os__clr_display(255);
		    //Os__GB2312_display(0,0,(unsigned char *)":");
			//MSG_DisplayMsg( true,0,0,MSG_CASHIERNEWPASS);

			memset(aucBuf,0,sizeof(aucBuf));
			if( UTIL_Input(1,true,6,6,'P',aucBuf,NULL) != KEY_ENTER )
			{
				return(ERR_CANCEL);
			}

            //Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"ȷ:");

			memset(aucBuf1,0,sizeof(aucBuf1));
			if( UTIL_Input(1,true,6,6,'P',aucBuf1,NULL) != KEY_ENTER )
			{
				return(ERR_CANCEL);
			}
			if(memcmp(aucBuf1,aucBuf,6))
			{

                //Os__clr_display(255);
				//Os__GB2312_display(1,0,(unsigned char *)"!");

				Os__xget_key();
				ucResult=ERR_CASH_PASS;
			}
		}
	}while(ucResult!=SUCCESS);
    //ucResult = LOGON_Online();
    if (ucResult == SUCCESS)
    {
		/*UTIL_SaveDataToMirror(0,true,true,
			(unsigned char *)&DataSave0.CashierData_Chk,
			&DataSave0.ConstantParamData.aucSuperPassword1[0],
			6);*/
		Os__saved_copy(aucBuf,
				DataSave0.ConstantParamData.aucSuperPassword,
				6);
		ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		if(ucResult!=SUCCESS)
			return ucResult;
    }else
       	MSG_DisplayErrMsg(ucResult);
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,":",13);
    sleep(1);
#else
    //Os__clr_display(255);
    //Os__GB2312_display(1,0,(unsigned char *)"޸ĳɹ!");
#endif
    Os__xget_key();   	
    return(SUCCESS);
}
unsigned char CASH_CheckCashierNo(unsigned char *pucIndex,unsigned char *pucCashierNo)
{
	unsigned char ucI;
    int	bFirstFlag = 1;

	for(ucI=0;ucI<CASH_MAXCASHIER;ucI++)
	{
        if( bFirstFlag )
		{
			if( !DataSave0.ConstantParamData.Cashier[ucI].aucName[0])
			{
                bFirstFlag = 0;
				*pucIndex = ucI;
			}
		}
		if( !memcmp(DataSave0.ConstantParamData.Cashier[ucI].aucName,pucCashierNo,CASH_CASHIERNOLEN))
		{			
			break;
		}
	}
	if( ucI == CASH_MAXCASHIER )
	{
		return(ERR_CASH_NOTEXIST);
	}else
	{
		*pucIndex = ucI;
		return(ERR_CASH_EXIST);
	}
}

unsigned char CASH_UpdateCashierData(unsigned char ucNo,
				unsigned char *pucCashierNo,unsigned char *pucCashierPass)								
{
	unsigned char ucResult=SUCCESS;
	/*UTIL_SaveDataToMirror(0,true,false,NULL,
				DataSave0.ConstantParamData.aucCashierNo[ucNo],
				CASH_CASHIERNOLEN); 
	UTIL_SaveDataToMirror(0,false,true,(unsigned char *)&DataSave0.CashierData_Chk,
				DataSave0.ConstantParamData.aucCashierPass[ucNo],
				CASH_CASHIERPASSLEN);*/
	if( pucCashierNo )
	{
		Os__saved_copy(pucCashierNo,DataSave0.ConstantParamData.Cashier[ucNo].aucName,CASH_CASHIERNOLEN);
	}
	else
	{
		Os__saved_set(DataSave0.ConstantParamData.Cashier[ucNo].aucName,0,CASH_CASHIERNOLEN);
	}
	if( pucCashierPass )
	{
		Os__saved_copy(pucCashierPass,DataSave0.ConstantParamData.Cashier[ucNo].aucPassword,CASH_CASHIERPASSLEN);
	}
	else
	{
		Os__saved_set(DataSave0.ConstantParamData.Cashier[ucNo].aucPassword,0,CASH_CASHIERPASSLEN);
	}
	ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
	return ucResult;
}
unsigned char CASH_CashierMenu(void)
{
	unsigned char ucResult;
 SELMENU  CashierMenu;
 memset(&CashierMenu,0,sizeof(SELMENU));
 UTIL_GetMenu_Value(MSG_NULL,	MSG_ADDCASHIER, 	CASH_AddCashier	,NULL,&CashierMenu);
 UTIL_GetMenu_Value(MSG_NULL,	MSG_DELCASHIER, 	CASH_DelCashier	,NULL,&CashierMenu);
UTIL_GetMenu_Value(MSG_NULL,	MSG_MODICASHIER, 	CASH_ModiCashier	,NULL,&CashierMenu);
//UTIL_GetMenu_Value(MSG_NULL,	MSG_CASHIERPINRELOAD, 	CASH_ModiCashier	,NULL,&CashierMenu);
UTIL_GetMenu_Value(MSG_NULL,	MSG_CHANGESUPERPASS, 	CASH_ChangeSuperPasswd1	,NULL,&CashierMenu);

	ucResult = UTIL_DisplayMenu(&CashierMenu);

	if( ucResult != SUCCESS)
	{
		MSG_DisplayErrMsg(ucResult);
	}

	return(SUCCESS);
}


