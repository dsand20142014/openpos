/* 
	SAND China
--------------------------------------------------------------------------
	FILE  cash.c								(Copyright SAND 2001)       
--------------------------------------------------------------------------
    INTRODUCTION
    ============                                             
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network             
    Purpose :                               
        Source file for cashier manage. 
                                                             
    List of routines in file :                                            
                                                                          
    File history :                                                        
                                                                          
*/

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
//	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"主管操作员密码");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
    	return(ERR_CANCEL);
	}
	if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,
			CASH_SYSCASHIERPASSLEN))
	{
		return(ERR_CASH_PASS);
	}
    do
    {
		MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERNO);
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( CASH_CheckCashierNo(&ucI,aucCashierNo) == ERR_CASH_NOTEXIST)
		{
              	ucResult = ERR_CASH_FILEFULL;
//            	MSG_DisplayMsg(true,1,0,MSG_CASH_FULL);
              	break;
		}
		if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
              	MSG_DisplayMenuMsg(true,1,0,MSG_CANCEL);
              	continue;
		}
		if( CASH_CheckCashierNo(&ucI,aucCashierNo) == ERR_CASH_EXIST)
		{
			ucResult = ERR_CASH_EXIST;
              	MSG_DisplayMenuMsg(true,1,0,MSG_CASH_EXIST);
              	continue;
		}
		MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERPASS);
		memset(aucCashierPass,0,sizeof(aucCashierPass));
		if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
              	MSG_DisplayMenuMsg(true,1,0,MSG_CANCEL);
              	continue;
		}
		CASH_UpdateCashierData(ucI,	aucCashierNo,aucCashierPass);
              MSG_DisplayMenuMsg( true,0,0,MSG_GOON);
	}while(Os__xget_key() == KEY_ENTER);
	
	return(ucResult);
}

unsigned char CASH_DelCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
	unsigned char ucI;
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
	unsigned char ucIndex;
   	/*Input Supervizor Password*/
//	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	
	ucIndex = DataSave0.ChangeParamData.ucCashierLogonIndex;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"主管操作员密码");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,
				CASH_SYSCASHIERPASSLEN))
	{
		return(ERR_CASH_PASS);
	}
    do
    {
		MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERNO);
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
		    MSG_DisplayMenuMsg(true,1,0,MSG_CANCEL);
            continue;
		}
		if( CASH_CheckCashierNo(&ucI,aucCashierNo) == ERR_CASH_NOTEXIST)
		{
			MSG_DisplayMsg(true,1,0,MSG_NOTEXIST);
            continue;
		}
		if( (!memcmp(aucCashierNo,DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucIndex],CASH_CASHIERNOLEN))
		  &&(DataSave0.ChangeParamData.ucCashierLogonFlag == CASH_LOGONFLAG)
		  )
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"签到柜员不能删除");
			continue;
		}	
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		memset(aucCashierPass,0,sizeof(aucCashierPass));
		CASH_UpdateCashierData(ucI,
					aucCashierNo,aucCashierPass);
		MSG_DisplayMenuMsg( true,0,0,MSG_GOON);
	}while(Os__xget_key() == KEY_ENTER);

	return(SUCCESS);
}

unsigned char CASH_ModiCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
	unsigned char ucIndex;

	MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERNO);
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( CASH_CheckCashierNo(&ucIndex,aucCashierNo) == ERR_CASH_NOTEXIST)
	{
		return(ERR_CASH_NOTEXIST);
	}
	
	MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERPASS);
	memset(aucCashierPass,0,sizeof(aucCashierPass));
	if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucIndex],aucCashierPass,CASH_CASHIERPASSLEN))
		{
			return(ERR_CASH_PASS);
		}else
		{
			MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERNEWPASS);
			memset(aucCashierPass,0,sizeof(aucCashierPass));
			if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
			{
				return(ERR_CANCEL);
			}
			CASH_UpdateCashierData(ucIndex,
						aucCashierNo,aucCashierPass);
		}						
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"修改成功");
		MSG_WaitKey(3);
		return(SUCCESS);
}

unsigned char CASH_PinReload(void)
{
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char ucIndex;

   	/*Input Supervizor Password*/
//	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"主管操作员密码");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,
				CASH_SYSCASHIERPASSLEN))
	{
		return(ERR_CASH_PASS);
	}

       MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERNO);
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( CASH_CheckCashierNo(&ucIndex,aucCashierNo) == ERR_CASH_NOTEXIST)
	{
		return(ERR_CASH_NOTEXIST);
	}
       CASH_UpdateCashierData(ucIndex,aucCashierNo,(uchar *)"0000");
	return(SUCCESS);

}
unsigned char CASH_CheckCashierNo(unsigned char *pucIndex,unsigned char *pucCashierNo)
{
	unsigned char ucI;
	UCHAR	bFirstFlag = true;

	for(ucI=0;ucI<CASH_MAXCASHIER;ucI++)
	{
		if( bFirstFlag == true)
		{
			if( !DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucI][0])
			{
				bFirstFlag = false;
				*pucIndex = ucI;
			}
		}
		if( !memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucI],pucCashierNo,CASH_CASHIERNOLEN))
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

void CASH_UpdateCashierData(unsigned char ucNo,
				unsigned char *pucCashierNo,unsigned char *pucCashierPass)								
{

	if( pucCashierNo )
	{
		Os__saved_copy(pucCashierNo,DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucNo],CASH_CASHIERNOLEN);
	}else
	{
		Os__saved_set(DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucNo],0,CASH_CASHIERNOLEN);
	}
	if( pucCashierPass )
	{
		Os__saved_copy(pucCashierPass,DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucNo],CASH_CASHIERPASSLEN);
	}else
	{
		Os__saved_set(DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucNo],0,CASH_CASHIERPASSLEN);
	}
	XDATA_Write_Vaild_File(DataSaveCashier);
}



unsigned char CASH_CashierQuery(void)
{
	unsigned char ucIndex;
	unsigned char aucCashierNo[CASH_MAXCASHIER+2][CASH_CASHIERNOLEN+1+8];
	unsigned char aucDispBuf[MAXLINECHARNUM+1];
	unsigned char ucI,key;
	unsigned char aucPassBuf[CASH_SYSCASHIERPASSLEN];

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"主管操作员密码");
	memset(aucPassBuf,0,sizeof(aucPassBuf));
	if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucPassBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(aucPassBuf,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,
				CASH_SYSCASHIERPASSLEN))
	{
		return(ERR_CASH_PASS);
	}

	memset(aucCashierNo , 0, sizeof(aucCashierNo));

	for(ucI=1 ,ucIndex=1 ;ucI<=CASH_MAXCASHIER ;ucI++ )
	{
		if( !DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucI-1][0])
		{
			continue;
		}else
		{
			memcpy(&aucCashierNo[ucIndex][0] ,"柜员  :   " , 10);
			char_asc(&aucCashierNo[ucIndex][5] , 1, &ucI);
			memcpy(&aucCashierNo[ucIndex][8] ,
					&DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucI-1], CASH_CASHIERNOLEN );
			ucIndex++;
		}
	}
	for(ucI=0 ;ucI<ucIndex ;ucI++)
	{
		memset(aucDispBuf , ' ', sizeof(aucDispBuf)-1);

		if(ucI == 0)
		{
			memcpy(aucDispBuf ,"主管操作员号:   " , 13);
			memcpy(&aucDispBuf[14] ,DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo , CASH_CASHIERNOLEN );
		}else
		{
			memcpy(aucDispBuf , &aucCashierNo[ucI] , CASH_CASHIERNOLEN+8);
		}
		//Os__clr_display(255);
		//Os__GB2312_display(1, 0, aucDispBuf);
		key = UTIL_WaitGetKey(30);
		if(key == KEY_CLEAR)
			break;
	}
	return SUCCESS ;
	
}


unsigned char CASH_InputAndChackManagerPass(unsigned char ucFlag)
{
	unsigned char ucDataBuf[8];
	unsigned char ucResult=SUCCESS;

return 0;	
	if (!ucFlag)
	{
		//Os__clr_display(255);
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,false,255);
		//Os__GB2312_display(1,0,(unsigned char *)"请输入主管密码:");
	}
	else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"请输入主管密码:");
	}
	memset(ucDataBuf,0,sizeof(ucDataBuf));
	if( UTIL_Input(2,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',ucDataBuf,NULL) != KEY_ENTER )
	{
		ucResult = ERR_CANCEL;
	}
	if(ucResult == SUCCESS)
	{
		if( memcmp(ucDataBuf,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,
					CASH_SYSCASHIERPASSLEN))
		{
			ucResult = ERR_CASH_PASS;
		}
	}
	return ucResult;
}
unsigned char CASH_GetLogonCasherNo(unsigned char *pucIndex)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
	unsigned char ucIndex;

	MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERNO);
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( CASH_CheckCashierNo(&ucIndex,aucCashierNo) == ERR_CASH_NOTEXIST)
	{
		return(ERR_CASH_NOTEXIST);
	}
	
	MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERPASS);
	memset(aucCashierPass,0,sizeof(aucCashierPass));
	if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucIndex],aucCashierPass,CASH_CASHIERPASSLEN))
	{
		return(ERR_CASH_PASS);
	}else
	{
			*pucIndex =ucIndex;
	}						
	return(SUCCESS);
}

