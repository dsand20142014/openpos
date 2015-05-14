#ifndef DEV_SIGNAL_H_INCLUDED
#define DEV_SIGNAL_H_INCLUDED

#ifdef __cplusplus
extern "C"{
#endif


/**
*@def DEV_GPRS_SIGNALL
*@brief 0~4,表示信号强度,其中0表示无信号
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
*@brief 2表示充电完成,1表示电池充电中,0表示没有电池
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

#define USB_DEVICE			"/dev/sda1"
#define TF_DEVICE    		"/dev/mmcblk0p1"
#define ADC_DEVICE			"/dev/imx_adc"
#define PRINTER_DEVICE	"/dev/printer"



/**
 * @defgroup 获取设备状态信号值
 * @author 熊剑
 * @brief  获取设备状态信号值
 * @{
 */

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
 *		return 0;
 *	}
 * @endcode
 */
int Os__get_device_signal(int devType);

int Os_adc(const char *device, int channel);

#ifdef __cplusplus
}
#endif


#endif//DEV_SIGNAL_H_INCLUDED
/** @} end of etc_fns */
