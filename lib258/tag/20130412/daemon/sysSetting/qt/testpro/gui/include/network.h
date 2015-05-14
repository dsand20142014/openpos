#ifndef NETWORK_H_INCLUDE
#define NETWORK_H_INCLUDE

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup 网络函数库
 * @author 熊剑
 * @brief 提供对网络进行操作的相关函数
 * @{
 */


typedef struct
{
	char ssid[100];
	char psk[50];
}WifiNetwork;


/**
 * @fn int Os__setSetting(WifiNetwork wifiInfo)
 * @brief 将wifi信息保存到系统配置文件中
 * @param wifiInfo wifi设置信息结构体
 *
 * @return 0 成功, < 0 出错.
 * @code
 *	int main(void)
 *	{
 *		WifiNetwork wifi;
 *		memset(&wifi,0,sizeof(WifiNetwork));
 *		sprintf(wifi.ssid,"YF_II");
 *		sprintf(wifi.psk,"SAND");
 *		return Os__setSetting(wifi);
 *	}
 * @endcode
 */
int Os__setWifi(WifiNetwork wifiInfo);

#ifdef __cplusplus
}
#endif


#endif // NETWORK_H_INCLUDE
/** @} end of etc_fns */
