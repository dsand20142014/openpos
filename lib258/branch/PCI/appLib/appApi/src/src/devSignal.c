#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <sys/mman.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "imx_adc.h"
#include "devSignal.h"
#include "modules.h"
#include "updateService.h"
#include "param.h"
#include "sysparam.h"
#include "gps.h"

#define BATTERY_ADC_4 7.95  //  8.3-7.95
#define BATTERY_ADC_3 7.6  //  ~7.6
#define BATTERY_ADC_2 7.25  //  ~7.25
#define BATTERY_ADC_1 6.9  //  ~6.9


#define PRINTER_ADC 1960    //小于这个值，过热
#define BUTTON_ADC  2730     //小于这个值，纽扣电池电压低

#define ADC_CHANNEL_2	_IOWR('S', 8, int)  //打印机
#define IMX_ADC_ADPTER       	_IOWR('p', 0xb8, int)

#define SHMNAME "shm_dial"
#define FILE_SIZE 10 

void* add_r = NULL;

static int wifiType = 0;

/**
 * @fn int Os__ini_device_state()
 * @brief 	初始化设备状态交互需要的共享内存
 *
 * @return 		成功返回0,失败返回-1,失败的时候是进程还没起来
 */
int ini_device_state()
{
	int fd,ret;
	unsigned char buf[10];
	
	fd = shm_open(SHMNAME, O_RDWR, 00777);
	if(fd<0)
		return -1;

	ret = ftruncate(fd, FILE_SIZE);
	if(ret<0)
		return -1;

    add_r = mmap(NULL, FILE_SIZE, PROT_READ, MAP_SHARED, fd, SEEK_SET);
	if(NULL == add_r)
	{
		munmap(add_r, FILE_SIZE);
		return -2;
	}

	return 0;	
} 

/**
 * @fn void Os__destory_device_state()
 * @brief 	释放初始化时候的资源
 *
 * @return 		void
 */
void destory_device_state()
{
	if(add_r != NULL)
		munmap(add_r, FILE_SIZE);
}

/*
int offset 0 是否拨上号
	offset 1 信号

return
	0 无卡或未拨上号
	1 无信号或已经拨上号
	2-5	信号强度
*/
int gsm_get_state(int offset)
{
	unsigned char buf;
	memcpy((char *)&buf, add_r+offset,1);
	return buf;
}


//只有电池为3,充电完成为2,充电中是1,只有电源适配器为0
int charge_stateDetect(void)
{ 
	unsigned int charge_state;
	int fd_adc=0;
	unsigned char cpld_value = 0;
	int adc_value = 0;
	
	fd_adc  = open(ADC_DEVICE,O_RDWR);        
 	if(fd_adc == -1)
 	  return -2;	
	
	read(fd_adc, &cpld_value,1);
	//CPLD的cpld_value值定义3为没有电池或只有电源适配器,2为充电中,1为充电完成
	charge_state=cpld_value&0x03;
	
 	if(3 == charge_state)
 	{
 		//因直插入电源或只有电池CPLD值都为3,为了区分状态,查看ADC状态
 		//0表示适配器插入,1表示没有电源适配器
 		ioctl(fd_adc, IMX_ADC_ADPTER, &adc_value);
 		
 		if(adc_value == 0)
			adc_value = 0;
		else
			adc_value = 3;

	}
	else if(2 == charge_state)
		adc_value = 1;
	else if(1 == charge_state)
		adc_value = 2;
	//因硬件改动出现00状态值,但不确定此状态后续是否会继续存在
	else if(0 == charge_state)
	{
		//因直插入电源或只有电池CPLD值都为3,为了区分状态,查看ADC状态
 		//0表示适配器插入,1表示没有电源适配器
 		ioctl(fd_adc, IMX_ADC_ADPTER, &adc_value);
 		
 		if(adc_value == 0)
			adc_value = 0;
		else
			adc_value = 3;
	}
	close(fd_adc);
	return adc_value;
}

int value=0;
int Os_adc(const char *device, int channel)
{
	int fd_adc=0;
	struct t_adc_convert_param convert_param;
	
	memset(&convert_param,0,sizeof(convert_param));
	convert_param.result[0] = 0;
	fd_adc = open(device,O_RDWR);
	if(fd_adc < 0)
	return -1;
	
	switch(channel)
	{
		case 1:
			//大电池
			convert_param.channel = GER_PURPOSE_ADC1;
			ioctl(fd_adc, IMX_ADC_INIT, NULL);
			ioctl(fd_adc, IMX_ADC_CONVERT, &convert_param);
			ioctl(fd_adc,IMX_ADC_DEINIT, NULL);
			value = convert_param.result[0];
			break;
		case 2: 
			//纽扣电池
			convert_param.channel = GER_PURPOSE_ADC0;
			ioctl(fd_adc, IMX_ADC_INIT, NULL);
			ioctl(fd_adc, IMX_ADC_CONVERT, &convert_param);
			ioctl(fd_adc,IMX_ADC_DEINIT, NULL);
			value = convert_param.result[0];
			break;
		case 3:
			//打印机状态
			ioctl(fd_adc,ADC_CHANNEL_2,&value);
			break;
		default:
		break;
	}
	close(fd_adc);  
	return value;
}

int battery_capability(void)
{
	int detect_result;
	double value = 0.0;
	
	value = devSignal_battery_voltage(CHANNEL_LI_BATT);

	if(value >= BATTERY_ADC_4)
		detect_result = 4;
	else if(value >= BATTERY_ADC_3)
		detect_result = 3;
	else if(value >= BATTERY_ADC_2)
		detect_result = 2;
	else if(value >= BATTERY_ADC_1)
		detect_result = 1;
	else
		detect_result = 0;

	return detect_result;
}

int battery_cell_capability(void)
{
	int value = 0, i, ret;
	
	/*
	for(i=0;i<3;i++)
		value += Os_adc(ADC_DEVICE,2);
	value = (int)(value/3.0);
	*/
	value = Os_adc(ADC_DEVICE,2);
	
	if(value < BUTTON_ADC)
		ret = 1;
	else
		ret = 0;
	
	return ret;
}


int masstorage_detect(int devType)
{
	int ret = 0;
	switch(devType)
	{
		case DEV_TF_DETECT:
			if(access(SPD_DEV_TF,F_OK) == 0)
				ret = 1;
			else if(access(SPD_DEV_TF_1,F_OK) == 0)
				ret = 1;
			break;
		case DEV_USB_DETECT:
			if(access(SPD_DEV_USB,F_OK) == 0)
				ret = 1;
			break;
	}
	
	return ret;
}

int _check_gps_state(void)
{
	int fd, ret = -1, count = 3, i;
	
	for(i=0;i<count;i++)
	{
		fd = open(GPS_DEV,O_RDWR|O_NOCTTY);
		if(fd <= 0)
			continue;
		ret = ioctl(fd,GPS_PWR_STATE,NULL);
		close(fd);
		break;
	}
	
	return ret;
}

int _check_wifi_state(void)
{
	
	
	unsigned int fd, intrface, retn = 0,retn2 = 0;
    char *wifi_name= "wlan0";
    struct ifreq buf[16];
    struct ifconf ifc;

	//老机器带了hub，新机器无hub，这里为了兼容两种设备
	//获取新老设备类型
	if(wifiType == 0)
	{
		SYS_SETTING setting;

		//判断wifi类型
		memset(&setting,0,sizeof(SYS_SETTING));
	 	strcpy(setting.section,"WIFI_SETTING");
	 	strcpy(setting.key,"WIFITYPE");
	 	PARAM_setting_get(&setting);	
	 	wifiType = atoi(setting.value);
	}
	
	//新机器无hub
	if(wifiType == 1)
		retn = access(WIFI_DEVICE,F_OK);
	//老机器有hub
	else if(wifiType == 2)
		retn = access(WIFI_DEVICE_OLD,F_OK);
	//无法判断机型
	else
		return 0;

	if(retn == -1)
	{
		//printf("%s NOT EXIT,PLS UPDATE YOUR DEVICE\n",WIFI_DEVICE);
		return 0;
	}
	
	retn = 1;
	
	//wifi模块启动下,检查wifi是否连接上
    if((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) 
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) 
        {
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0)
			{
				//wifi 已经连接
				if(strcmp(wifi_name,buf[intrface].ifr_name)==0) 
				{
					//wifi信号强度暂时写死
					retn = 5;
					break;
				}
			}
        }  
    }
    close(fd);

    return retn;
}

/**
 * @fn int _check_defaultDate(void)
 * @brief  check current date is 1970.1.1 or not.	
 * @author lichao
 * @date   2013.4.9
 *
 * @return if current date is 1970.1.1 return 1 else return 0.
 */
int _check_defaultDate(void)
{
	time_t now;
	struct tm *date;
	time(&now);
	date = localtime(&now);
	if( (date->tm_year == 70) && \
	    (date->tm_mon == 0) && \
	    (date->tm_mday == 1) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


int Os__get_device_signal(int devType)
{
	int ret = -1;

	switch(devType)
	{
	 	//gprs信号强度
		case DEV_GPRS_SIGNALL:
			if(add_r == NULL && ini_device_state() != 0)
				ret = 9;
			else
				ret = gsm_get_state(0);
			break;
		//gprs是否拨上号
		case DEV_GPRS_DIAL:
			if(add_r == NULL && ini_device_state() != 0)
				ret = 0;
			else
				ret = gsm_get_state(1);
			break;
		//usb是否插上
		case DEV_USB_DETECT:
			ret = masstorage_detect(DEV_USB_DETECT);
			break;
		//tf卡是否插上
		case DEV_TF_DETECT:
		  ret = masstorage_detect(DEV_TF_DETECT);
			break;
		//大电池电量
		case DEV_BATT_VALUE:
			ret = battery_capability();
			break;
		//是否充电
		case DEV_CHAG_BAR:
			ret = charge_stateDetect();
			break;
		//打印机缺纸和过热状态
		case DEV_PRINTER_STATE:
			ret = Os_adc(PRINTER_DEVICE,3);
			break;
		//纽扣电池是否没电
		case DEV_BUTTON_CELL:
		  ret = battery_cell_capability(); 
			break;
		case DEV_GPS:
			ret = _check_gps_state();
			break;
		//回收资源
		case DEV_DESTORY:
			destory_device_state();
			break;
		//wifi模块状态及信号强度
		case DEV_WIFI:
			ret = _check_wifi_state();
			break;
		//check the current date is 1970.1.1 or not
		case DEV_DEFAULT_DATE:
			ret = _check_defaultDate();
			break;
		//unknow type 
		default:
			break;
	}
    return ret;
}


/*
 *	VBAT=纽扣电池电压，AD1=AD1寄存器数值，R1=分压电阻1，R2=分压电阻2，Vadref=AD参考电压，ADmax=AD寄存器最大值。
 *	VBAT=[(R1+R2)*Vadref/(ADmax*R2)]*AD1
 *	其中，R1=36K,R2=112K,Vadref=2.5V,ADmax=4096
 *	VBAT=[148*2.5/(4096*112)]*AD1
 *
 *	Vli=锂电池电压，AD2=AD2寄存器数值，R1=分压电阻1，R2=分压电阻2，Vadref=AD参考电压，ADmax=AD寄存器最大值。
 *	Vli=[(R1+R2)*Vadref/(ADmax*R2)]*AD2
 *	其中，R1=100K,R2=40.2K,Vadref=2.5V,ADmax=4096
 *	Vli=[140.2*2.5/(4096*40.2)]*AD2
 */
double devSignal_battery_voltage(int channel)
{
	double value = 0;
	int tmp = 0;
	value = (double)Os_adc(ADC_DEVICE,channel);

	switch(channel)
	{
		//锂电池
		case CHANNEL_LI_BATT:
			value = (140.2*2.5/(4096*40.2))*value;
			break;
		//纽扣电池
		case CHANNEL_CELL_BATT:
			value = (148*2.5/(4096*112))*value;
			break;
	}

	//取小数点后两位,并进行四舍五入
	tmp = (int)(value*1000);
	if(tmp%10 >= 5)
		tmp += 10;
	value = (double)(tmp/10)*0.01;

	return value;
}
