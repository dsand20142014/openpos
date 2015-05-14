#include <fcntl.h>
#include <linux/ioctl.h>
#include <string.h>
#include <stdio.h>
#include "ostime.h"
#include "modules.h"
#include "sysparam.h"
#include "gps.h"

#define BEEP_GATE_OPEN              1
#define BEEP_GATE_CLOSE             0
 
#define BACKLIGHT_GATE_OPEN         1
#define BACKLIGHT_GATE_CLOSE        0
  
#define BEEP_BACKLIGHT_IOC_NUM		'K'

 
//蜂鸣器 
#define BEEP_ON		_IO(BEEP_BACKLIGHT_IOC_NUM, 1)
#define BEEP_OFF	_IO(BEEP_BACKLIGHT_IOC_NUM, 2)
  
//背光亮度调节 0~255
#define BACKLIGHT_SET	_IOW(BEEP_BACKLIGHT_IOC_NUM, 3, int)
   
//按键音开关 1：开按键音；0：关按键音
#define BEEP_GATE		_IOW(BEEP_BACKLIGHT_IOC_NUM, 4, int)
    
//LCD背光开关 1：开背光； 0：关背光
#define BACKLIGHT_GATE	_IOW(BEEP_BACKLIGHT_IOC_NUM, 5, int)

//调节亮度/对比度
int Os__lcd_bright(unsigned char ucBright)
{
    int fd;
	int b=ucBright;
	
	if(b>255)
		return MODULES_PARAM;
		
	fd = open(BEEP_BACKLIGHT_DEVICE, O_RDWR,0);
	
	if(fd < 0)
		return MODULES_ERRDEVICE;
	
    ioctl(fd,BACKLIGHT_SET,&b);  
	close(fd);
	return MODULES_SUCCESS;       
}

//打开背光或者打开屏幕
int Os__light_on(void)
{
    int fd,opt=0;
	
	opt=BACKLIGHT_GATE_OPEN;
	fd = open(BEEP_BACKLIGHT_DEVICE, O_RDWR,0) ;
	
	if(fd < 0)
		return MODULES_ERRDEVICE;
	
    ioctl(fd,BACKLIGHT_GATE,&opt);  
	close(fd);
	return MODULES_SUCCESS;       
}


int Os__key_beep(int opt)
{
    int fd;
	
	fd = open(BEEP_BACKLIGHT_DEVICE, O_RDWR,0) ;
	
	if(fd < 0)
		return MODULES_ERRDEVICE;
	
    ioctl(fd,BEEP_GATE,&opt);  
	close(fd);
	return MODULES_SUCCESS;       
}


//打开背光或者打开屏幕
int Os__light_off(void)
{
    int fd,opt=0;
	
	opt=BACKLIGHT_GATE_CLOSE;
	fd = open(BEEP_BACKLIGHT_DEVICE, O_RDWR,0) ;
	
	if(fd < 0)
		return MODULES_ERRDEVICE;
	
    ioctl(fd,BACKLIGHT_GATE,&opt);  
	close(fd);
	return MODULES_SUCCESS;       
}

//响蜂鸣器
int Os__beep(void)
{
    int fd;
    
	fd = open(BEEP_BACKLIGHT_DEVICE, O_RDWR,0) ;
	
	if(fd < 0)
		return MODULES_ERRDEVICE;
	
    ioctl(fd,BEEP_ON);
	Os__xdelay(25);
    ioctl(fd,BEEP_OFF);  
	close(fd);
	return MODULES_SUCCESS;       
}

//led灯
void Os__led_display(int status,int led)
{
	unsigned char ucBuf[50];
	
	if(status>1)
		return;
	if(led>4)	
		return;
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"echo %d > /sys/class/leds/led%d/brightness",status,led);
	system(ucBuf);
}


int modules_gps_active(int opt)
{
	int fd;
	SYS_SETTING sysSetting;
	fd = open(GPS_DEV,O_RDWR|O_NOCTTY);
	if(fd > 0)
	{
		//设置setting.ini中内容
		memset(&sysSetting,0,sizeof(SYS_SETTING));
		strcpy(sysSetting.section,"MODULE_ACTIVE");
		strcpy(sysSetting.key,"GPS_OPEN");
		
		if(opt == 1)
		{
			ioctl(fd,GPS_PWR_ON,NULL);
			sysSetting.value[0] = 0x31;
		}
		else
		{
			ioctl(fd,GPS_PWR_OFF,NULL);
			sysSetting.value[0] = 0x30;
		}
		PARAM_setting_set(&sysSetting);
		close(fd);
		return 0;
	}

	return -1;
}


void modules_smt_active(int opt)
{
	SYS_SETTING sysSetting;
	//设置setting.ini中内容
	memset(&sysSetting,0,sizeof(SYS_SETTING));
	strcpy(sysSetting.section,"MODULE_ACTIVE");
	strcpy(sysSetting.key,"SMT_OPEN");
	if(opt == 1)
	{
		sysSetting.value[0] = 0x31;
	}
	else
	{
		sysSetting.value[0] = 0x30;
	}
	PARAM_setting_set(&sysSetting);
}

void modules_wifi_active(int opt)
{
	SYS_SETTING sysSetting;
	//设置setting.ini中内容
	memset(&sysSetting,0,sizeof(SYS_SETTING));
	strcpy(sysSetting.section,"MODULE_ACTIVE");
	strcpy(sysSetting.key,"WIFI_OPEN");
	if(opt == 1)
	{
		sysSetting.value[0] = 0x31;
		system("/etc/usbwifi/start.sh");
	}
	else
	{
		sysSetting.value[0] = 0x30;
		system("/etc/usbwifi/stop.sh");
	}
	PARAM_setting_set(&sysSetting);
}

int modules_comdbg_status(void)
{
	int ret = 0;
	FILE *file;
  	char readBuf[200];
  	
	memset(readBuf,0,sizeof(readBuf));
	file=popen("pgrep -f comdbg.sh -l","r");
	if(file != NULL)
	{
		while(fgets(readBuf, 200, file) != NULL)
		{
			if(strstr(readBuf,"/daemon/comdbg.sh") != NULL)
			{
				ret = 1;
				break;
			}
		}
	}
	fclose(file);
	return ret;
} 

void modules_comdbg_open(void)
{
	if(modules_comdbg_status() == 0)
		system("/daemon/comdbg.sh &");
}
