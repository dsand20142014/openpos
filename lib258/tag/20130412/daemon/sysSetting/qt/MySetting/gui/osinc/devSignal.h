#ifndef DEV_SIGNAL_H_INCLUDED
#define DEV_SIGNAL_H_INCLUDED

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup 获取设备状态信号值
 * @author 熊剑,刘一峰
 * @brief  获取设备状态信号值
 * @{
 */

/**
*@def DEV_GPRS_SIGNALL
*@brief 0~5,表示信号强度,其中0表示无卡，1表示无信号，2-5表示信号 9表示拨号进程没开
**/
#define DEV_GPRS_SIGNALL  1

/**
*@def DEV_GPRS_DIAL
*@brief 0表示未拨上号,1表示拨号成功
**/
#define DEV_GPRS_DIAL     2

/**
*@def DEV_TF_DETECT
*@brief 0表示未插上tf卡，1表示检测到tf卡
**/
#define DEV_TF_DETECT     3

/**
*@def DEV_USB_DETECT
*@brief 0表示未插上usb，1表示检测到usb
**/
#define DEV_USB_DETECT    4

/**
*@def DEV_CHAG_BAR
*@brief 3表示只有电池,2表示充电完成,1表示电池充电中,0表示只接入电源适配器
**/
#define DEV_CHAG_BAR      5

/**
*@def DEV_BATT_VALUE
*@brief 0~4,表示电池电量信息,其中0表示电量耗尽
**/
#define DEV_BATT_VALUE    6

/**
*@def DEV_PRINTER_STATE
*@brief 0表示正常,1表示缺纸,2表示打印机过热,3表示缺纸加过热
**/
#define DEV_PRINTER_STATE   7 

/**
*@def DEV_BUTTON_CELL
*@brief 0纽扣电池正常，1标识纽扣电池低电压
**/
#define DEV_BUTTON_CELL   8

/**
*@def DEV_GPS
*@brief 0没有定位,1正在定位,-1获取状态失败
**/
#define DEV_GPS   	9

/**
*@def DEV_DESTORY
*@brief 回收dev资源
**/
#define DEV_DESTORY   	10

/**
 *@def DEV_WIFI
 *@brief 0表示wifi模块关闭,1表示wifi模块打开但未连接,2~5表示wifi信号强度
**/
#define DEV_WIFI		11

/**
*@def CHANNEL_LI_BATT
*@brief 锂电池
**/
#define	CHANNEL_LI_BATT		1

/**
*@def CHANNEL_CELL_BATT
*@brief 纽扣电池
**/
#define CHANNEL_CELL_BATT	2

#define ADC_DEVICE			"/dev/imx_adc"
#define PRINTER_DEVICE		"/dev/printer"
#define WIFI_DEVICE			"/sys/bus/usb/devices/1-1"  
#define WIFI_DEVICE_OLD		"/sys/bus/usb/devices/1-1.2" //老版本机器带了usb hub，

/**
 * @fn int Os__get_device_signal(int devType)
 * @brief	获取设备的信号值,如电池电量,gprs信号,usb设备是否存在,tf卡是否存在等
 *
 * @param devType	设备类型,详细见DEV_*类型
 *
 * @return 返回设备信号的当前值
 *
 * 例
 * @code
 *	int main()
 *	{
 *		int ret = -1;
 *		while(1)
 *		{
 *			ret = Os__get_device_signal(DEV_GPRS_SIGNALL);
 *			printf("the gprs signal is %d\n",ret);
 *			sleep(1);
 *			ret = Os__get_device_signal(DEV_GPRS_DIAL);
 *			printf("the gprs detect is %d\n",ret);
 *			sleep(1);
 *			ret = Os__get_device_signal(DEV_TF_DETECT);
 *			printf("the tf detect is %d\n",ret);
 *			sleep(1);
 *			ret = Os__get_device_signal(DEV_USB_DETECT);
 *			printf("the usb detect is %d\n",ret);
 *			sleep(1);
 *			ret = Os__get_device_signal(DEV_BATT_VALUE);
 *			printf("the bar value is %d\n",ret);
 *			sleep(1);
 *      	ret = Os__get_device_signal(DEV_CHAG_BAR);
 *      	printf("the chag bar is %d\n",ret);
 *      	sleep(1);
 *		}
 *		Os__get_device_signal(DEV_DESTORY);
 *		return 0;
 *	}
 * @endcode
 */
int Os__get_device_signal(int devType);


/**
 * @fn int Os_adc(const char *device, int channel)
 * @brief 获取大电池,纽扣电池及打印机状态值
 *
 * @param device	设备名称
 * @param channel	1表示大电池类型,2表示纽扣电池类型,3表示打印机状态
 *
 * @return 返回大电池,纽扣电池及打印机状态值
 */
int Os_adc(const char *device, int channel);

/**
 * @fn double devSignal_battery_voltage(int channel)
 * @brief 获取锂电池,纽扣电池的电压
 *
 * @param channel	CHANNEL_LI_BATT(1)表示锂电池类型,CHANNEL_CELL_BATT(2)表示纽扣电池类型
 *
 * @return 返回锂电池,纽扣电池电压值
 */
double devSignal_battery_voltage(int channel);

#ifdef __cplusplus
}
#endif

#endif//DEV_SIGNAL_H_INCLUDED
/** @} end of etc_fns */
