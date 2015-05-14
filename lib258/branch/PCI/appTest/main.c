#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <fsync_drvs.h>
#include <sand_main.h>
#include <menu.h>
//struct mutex	mu; /* * 需要增加一个解锁的函数以便在应用处理完事件后返回并通知OS而不是退出 */
void wakeUp()
{
	qt_main_entry_writemsg();
 //   mutex_lock(&mu);
//    bufferNotDel.wakeAll();
  //  mutex_unlock(&mu);
}


/*
 * 这里是接收到消息后的处理过程
 */
void manager(struct seven *EventInfo)
{
	Uart_Printf("manager\r\n");
	unsigned char ucResult ;
	unsigned char EventType;
	unsigned char EventNum;
	unsigned char tempbuf[250];
	NEW_DRV_TYPE new_drv;
	int i;
	

	EventType = (unsigned char )EventInfo->event_type;
	EventNum = (unsigned char )EventInfo->event_nr;

	
	ucResult = SUCCESS;

	//Os__light_on(); 把控制背光的函数Os__light_*全部删掉！
	EventInfo->pt_eve_out.treatment_status = NOK;
	UTIL_ClearGlobalData();
	Uart_Printf("\r\nbefore switch,EvenType+EventNum = %d\r\n",EventType + EventNum);
	Uart_Printf("EVESEL + 0xff = %d\r\n",EVESEL + 0xff);
	
	switch ( EventType+EventNum)
	{

		//case EVECIR + EVE_MAG_CARD:
		case  OSEVENTID_MAGCARD: // MAG card event
			Uart_Printf("\r\n+++++++++++++++++++++++MAG_CARD event\r\n");
			ucResult = MAG_GetTrackInfo(&(EventInfo->pt_eve_in.e_mag_card.mag_card[0]));
			if(!ucResult)
			{
				ucResult = Trans_Process();
				Uart_Printf("Trans_Process result = %d\r\n",ucResult);
			}
			if(( ucResult != SUCCESS)&&( ucResult != ERR_CANCEL))	
			{
				MSG_DisplayErrMsg(ucResult);
			}
			lcd_exit();
			EventInfo->pt_eve_out.treatment_status = OK;
			wakeUp();//处理完毕
	                break;

		case EVESEL & 0xff: //进入应用
		{
			Uart_Printf("EVESEL + 0XFF\r\n");
			while(1)
			{
				Uart_Printf("application\r\n");
				Os__GB2312_display(0,0,(uchar *)"1.签到");
				Os__GB2312_display(1,0,(uchar *)"2.设置");
				Os__GB2312_display(2,0,(uchar *)"3.主密钥下载");
				Os__GB2312_display(3,0,(uchar *)"4.随机数生成");
				Os_Wait_Event(KEY_DRV|MAG_DRV, &new_drv, 100000);
				if(new_drv.drv_type == KEY_DRV)
				{
					Uart_Printf("key select\r\n");
					Main_Menu(new_drv.drv_data.xxdata[1]);
					if(new_drv.drv_data.xxdata[1] == KEY_CLEAR)
						break;
					else 
						continue;
				}
				else if(new_drv.drv_type == MAG_DRV)
				{
					Uart_Printf("____________________________mag event\r\n");
					memset(tempbuf,0,sizeof(tempbuf));
					tempbuf[0] = 0;
					tempbuf[1] = 1;
					tempbuf[2] = 0;
					memcpy(&tempbuf[3],&new_drv.drv_data.xxdata[0],200);
					ucResult = MAG_GetTrackInfo(tempbuf);
					if(!ucResult)
                                	{
                                       		ucResult = Trans_Process();
						Uart_Printf("Trans_Process result = %d\r\n",ucResult);
                                	}
                       		        if(( ucResult != SUCCESS)&&( ucResult != ERR_CANCEL))
                     		        {
                                	        MSG_DisplayErrMsg(ucResult);
                                	}
					lcd_exit();
				}
			}

			EventInfo->pt_eve_out.treatment_status = OK;
			wakeUp();//处理完毕
		}
		break;
			
#if 0//这些已经不通用了			
		case EVECOM + EVE_POWER_ON:
			OnEve_power_on();
			break;
#endif			
		default:
			Uart_Printf("default\r\n");
			EventInfo->pt_eve_out.treatment_status = OK;
			wakeUp();
			break;
	}
}



/*
 * 需要增加一个主函数 - 接收消息
 */
int main(int argc,char** argv)
{
	struct seven   *appevent;
	unsigned char rec;
	NEW_DRV_TYPE new_drv;
	qt_main_entry_init(argc, argv);
	OnEve_power_on();
	
	//mutex_init(&mu);
    
	while (1) {
		//接收消息
		appevent = (struct seven *)qt_main_entry_readmsg();

		//收到消息后加锁
        	//mutex_lock(&mu);
 		//Os_Wait_Event(MAG_DRV | MFR_DRV | ICC_DRV , &new_drv, 100000);
		//处理消息
        	manager(appevent);
        
        	//等待解锁
//        	wait(G&mu);
        	//mutex_unlock(&mu);
        
        	//通知OS处理完毕
	}
}



