#include <include.h>
#include <global.h>
#include <xdata.h>

unsigned char MENU_SetMarchName()
{
  unsigned char ucResult = SUCCESS;
  unsigned char ucKey = 0x00;
  UTIL_ClearGlobalData();
  UTIL_GetTerminalInfo(); 
  
  if( DataSave0.ConstantParamData.ucORDERFLAG != 0x31 || DataSave0.ConstantParamData.uclujiaomc !='1')
    ucResult = CFG_SetMarchName();
  else
  {
    
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(uchar *)"1.本机商户名");
    //Os__GB2312_display(1,0,(uchar *)"2.客户商户名");
  
    ucKey = UTIL_GetKey(30);
    util_Printf("\nucKey:%02x",ucKey);
    switch(ucKey)
    {
      case '1':
        ucResult = CFG_SetMarchName();
        break;
      case '2':
        ucResult = CFG_SetMarchName_Cust();
        break;
      case '3':
        break;      
      case KEY_CLEAR:
        break;
      default:
        break;
    }
  }
  return ucResult;
}

unsigned char  MenuSys_TerminalSet(void)
{
	unsigned char ucResult;
	SELMENU SysMenu;
	unsigned char ucPassWd[5];

	memset(&SysMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(NULL,	MSG_MERCHANTNAME,	NULL,	MENU_SetMarchName,			&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_SETDATETIME,			NULL,	CFG_ConstantParamDateTime,	&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_FPHONENO,					NULL,	CFG_ConstantParamPhoneNo,	&SysMenu);
	UTIL_GetMenu_Value(NULL,	SET_PRINTER,							NULL,	UTIL_SetPrinter,			&SysMenu);

	UTIL_GetMenu_Value(NULL,	MSG_SETPINPAD,					NULL,	UTIL_SetPinpad,				&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_CLEARTRANS,				NULL,	MenuSys_Init,				&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_INITDATA,					NULL,	UTIL_Backlight,				&SysMenu);
	//UTIL_GetMenu_Value(NULL,	MSG_UCSETREADER,					NULL,	UTIL_SetReader,				&SysMenu);

	ucResult = SUCCESS;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"请输密码：");
	memset(ucPassWd,0,sizeof(ucPassWd));
	if( UTIL_Input(1,true,4,4,'P',ucPassWd,NULL) == KEY_ENTER )
	{
		if(memcmp(ucPassWd,"\x38\x38\x38\x38",4))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,2,(unsigned char *)"密码错误");
			MSG_WaitKey(3);
			return(ERR_CANCEL);
		}
	}else
	{
		return(ERR_CANCEL);
	}
	while(ucResult == SUCCESS||ucResult==ERR_CANCEL)
	{
		ucResult = UTIL_DisplayMenu(&SysMenu);
		if(ucResult == ERR_END) break;
	}
	return SUCCESS;
}

unsigned char  MenuSys_Init(void)
{
	unsigned char ucResult;
	SELMENU SysMenu;
	unsigned char ucPassWd[5];

	memset(&SysMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(NULL,	MSG_CLEARTRANS,					NULL,	UTIL_ClearTrans,						&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_DELETE_REVERSAL_FLAG,	NULL,	UITL_Delete_Reversal_Flag,	&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_DELETE_SCRIPT_FLAG,		NULL,	UITL_Delete_Script_Flag,			&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_CLEARMEMERY,				NULL,	UTIL_ClearMemery,				&SysMenu);


	UTIL_GetMenu_Value(NULL,	MSG_CLEARCOLLECT,			NULL,	UTIL_ClearCollectData,						&SysMenu);



	ucResult = SUCCESS;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"请输密码：");
	memset(ucPassWd,0,sizeof(ucPassWd));
	if( UTIL_Input(1,true,4,4,'P',ucPassWd,NULL) == KEY_ENTER )
	{
		if(memcmp(ucPassWd,"\x38\x38\x38\x38",4))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,2,(unsigned char *)"密码错误");
			MSG_WaitKey(3);
			return(ERR_CANCEL);
		}
	}else
	{
		return(ERR_CANCEL);
	}
	while(ucResult == SUCCESS||ucResult==ERR_CANCEL)
	{
		ucResult = UTIL_DisplayMenu(&SysMenu);
		if(ucResult == ERR_END) break;
	}
	return SUCCESS;
}