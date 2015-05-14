#include <string.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include "devSignal.h"
#include "sysglobal.h"
#include "updateService.h"
#include "param.h"
#include "sysparam.h"
#include "gsm.h"
#include "modules.h"

static unsigned short ucOSCurrAppID;


#define DRIVER_USED       _IOW('K',6,int)


/* 设置应用程序id，在多应用管理系统中每次调度前都对其进行设置 */
int  set_app_id(unsigned short appid)
{
    ucOSCurrAppID = appid;
}


static void _getSysParam(unsigned char *ucSource)
{
	char * ptr;
	int p=0;
	ptr=strstr(ucSource," #");

	if(ptr)
	{
		p=(unsigned char*)ptr-ucSource;
		*(ucSource+p)=0x00;
	}
}

SPARAM Os_readParam(void)
{
	int etcFile,etcSection;
	unsigned char ucPath[50];
	SPARAM _p;
	memset(&_p,0,sizeof(SPARAM));
	memset(ucPath,0,sizeof(ucPath));
	sprintf(ucPath,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSET);
 	etcFile=Os__param_loadFullPath(ucPath);
 	etcSection=Os__param_findSection(etcFile,"MODULE_SETTING",0);
	if(etcSection <= 0)
		return;
 	if(Os__param_get(etcSection,"MACHINE_SERIAL_NO",_p.aucSerialNo,sizeof(_p.aucSerialNo)) == 0)
 		_getSysParam(_p.aucSerialNo);
 	if(Os__param_get(etcSection,"PINPAD_SERIAL_NO",_p.aucPADSerialNo,sizeof(_p.aucPADSerialNo)) == 0)
 		_getSysParam(_p.aucPADSerialNo);
 		
 	Os__param_unload(etcFile);	
 	return _p;
}

void SYSGLOBAL_sys_poweron(void)
{
	SYS_SETTING setting;
	int b=0;
	
	//判断wifi类型
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"WIFI_SETTING");
 	strcpy(setting.key,"WIFITYPE");
 	PARAM_setting_get(&setting);	
 	b=atoi(setting.value);
 	//0表示未check过,已经check过的不再重复check
 	if(b == 0)
 	{
 		//有文件WIFI_DEVICE则表示是老机器,带HUB
 		if(access(WIFI_DEVICE,F_OK) == 0)
 			b = 2;
 		else
 			b = 1;
 		memset(setting.value,0,sizeof(setting.value));
 		sprintf(setting.value,"%d",b);
 		PARAM_setting_set(&setting);
 	}

	//初始化屏幕亮度
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"SCREEN_SETTING");
 	strcpy(setting.key,"BRIGHTNESS");
 	PARAM_setting_get(&setting);	
 	b=atoi(setting.value);
 	Os__lcd_bright(b);
 	
 	//初始化wifi拨号状态
 	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_ACTIVE");
 	strcpy(setting.key,"WIFI_OPEN");
 	PARAM_setting_get(&setting);
 	b=atoi(setting.value);
 	modules_wifi_active(b);
 	
 	
 	//初始化gps拨号状态
 	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_ACTIVE");
 	strcpy(setting.key,"GPS_OPEN");
 	PARAM_setting_get(&setting);
 	b=atoi(setting.value);
 	modules_gps_active(b);
 	
 	//每次开机时设置SMT状态为关闭(0)
 	modules_smt_active(0);
//	timer_init();
//	lcd_init();
	

}

/*
#define  GPRS_PWR	(1<<0)
#define  GPS_PWR	(1<<1)
#define  BAR_PWR	(1<<2)
#define  WIFI_PWR	(1<<3)
#define  LED1_PWR	(1<<4)
#define  LED2_PWR	(1<<5)
#define  LED3_PWR	(1<<6)
#define  LED4_PWR	(1<<7)
*/
void SYSGLOBAL_sys_suspend(void)
{
	int fd_m,ret=0;
	int wifi_open=0, gps_open=0, wifi_suspend_model = 0;
	unsigned int c;
	int brightValue = 0;
	SYS_SETTING setting;
	
	//get bright value
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"SCREEN_SETTING");
 	strcpy(setting.key,"BRIGHTNESS");
 	PARAM_setting_get(&setting);
 	brightValue = atoi(setting.value);

	//wifi
	ret = Os__get_device_signal(DEV_WIFI);
	if(ret==0)
		wifi_open=0;
	else
	{
		wifi_open=1;
		
		memset(&setting,0,sizeof(SYS_SETTING));
	 	strcpy(setting.section,"WIFI_SETTING");
	 	strcpy(setting.key,"WIFISUSPENDTYPE");
	 	PARAM_setting_get(&setting);
		wifi_suspend_model = atoi(setting.value);
	}
	
	
	//gps	
	ret = Os__get_device_signal(DEV_GPS);
	if(ret==0)		
		gps_open=0;
	else
		gps_open=1;	
  
  	c=(gps_open<<1)|(1<<0);
  	fd_m = open(MODULE_DEVICE, O_RDWR);
    ioctl(fd_m,DRIVER_USED,&c);
    
    if(fd_m >=0)
   		close(fd_m);

   	//gsm
   	system("/etc/ppp/ppp-off");
	while(1)
    {
    	ret=access("/var/lock/LCK..ttymxc1",F_OK);
    	if(ret!=0)
    		break;
    	
    	Os__xdelay(50);
    }
    
    
    if(wifi_open == 1 && wifi_suspend_model == 1)
    {
    	system("/etc/usbwifi/stop.sh");
	    while(1)
	    {
	    	ret=Os__get_device_signal(DEV_WIFI);
	    	
	    	if(ret==0)
	    	{
	    		break;	
	    	}
	    		
	    	Os__xdelay(50);		
	    }
	}

	Os__lcd_bright(0);
	
    printf("API SUSPEND\n");
 
    //suspend system
    system("echo mem > /sys/power/state");
   
     
    /**********************************/
    /*	休眠起来后					  */ 
    /**********************************/ 
    
    printf("API AWAKE\n"); 

 	Os__lcd_bright(brightValue);
	Os__beep();

    //初始化摄像头
	Os__camera_init(1, 1);
    Os__camera_close();
}

void SYSGLOBAL_sys_awake(void)
{
	int wifi_open = 0, wifi_suspend_model = 0;
	SYS_SETTING setting;
	
	// 获取wifi状态
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_ACTIVE");
 	strcpy(setting.key,"WIFI_OPEN");
 	PARAM_setting_get(&setting);
	
	wifi_open = atoi(setting.value);

	if(wifi_open == 1)
	{
		memset(&setting,0,sizeof(SYS_SETTING));
	 	strcpy(setting.section,"WIFI_SETTING");
	 	strcpy(setting.key,"WIFISUSPENDTYPE");
	 	PARAM_setting_get(&setting);
		wifi_suspend_model = atoi(setting.value);
		
		if(wifi_suspend_model == 1)
    		system("/etc/usbwifi/start.sh &");
    	else
    		system("/etc/usbwifi/awake.sh &");
	}
	
}
