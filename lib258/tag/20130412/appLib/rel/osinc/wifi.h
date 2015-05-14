#ifndef WIFI_H_INCLUDED
#define WIFI_H_INCLUDED


/**
 * @defgroup wireless信息获取和设置函数
 * @author 熊剑
 * @brief 提供对wifi无线网卡进行设置和获取信息
 * @{
 */

typedef struct
{
	char addressMAC[50];		//MAC地址
	char essid[50];				//wifi的ID
	char frequency[50];			//频率
	char bitRates[50];			//波特率
	char quality[50];			//信号质量
	char sigLev[50];			//信号水平
}WIFISCAN;

/**
 * @fn int wifi_scan(char *ifname, WIFISCAN wifiscan[], int maxSize)
 * @brief 	获取wifi扫描的结果信息,包括当前可用wifi的ssid,信号强度加密方式等信息
 *
 * @param ifname		需要获取的网卡信息,一般为wlan0
 * @param wifiscan  	保存wifi扫描信息列表的结构体数组地址
 * @param maxSize		最大能保存wifi扫描信息的个数
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int wifi_scan(char *ifname, WIFISCAN wifiscan[], int maxSize);

#endif	/* WIFI_H_INCLUDED */
/** @} end of etc_fns */
