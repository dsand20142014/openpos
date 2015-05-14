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
	char encryption[2];				//wifi是否加密,0未加密,1加密
	char frequency[50];			//频率
	char bitRates[50];			//波特率
	char quality[50];			//信号质量
	char sigLev[50];			//信号水平
	int sigLevNum;				//信号强度分成四个等级,从1~4四个等级,4级为最强
}WIFISCAN;

typedef struct
{
	char addressMAC[50];		//MAC地址
	char essid[50];				//wifi的ID
	char encryption[2];			//wifi是否加密,0未加密,1加密
	char psk[50];				//wifi密码
	char curLink[2];				//标示当前使用的WIFI,1表示此组WIFI正在使用,0表示未使用
}WIFIINFO;

/**
 * @fn int WIFI_scan(char *ifname, WIFISCAN wifiscan[], int maxSize)
 * @brief 	获取wifi扫描的结果信息,包括当前可用wifi的ssid,信号强度加密方式等信息
 *
 * @param ifname		需要获取的网卡信息,一般为wlan0
 * @param wifiscan  	保存wifi扫描信息列表的结构体数组地址
 * @param maxSize		最大能保存wifi扫描信息的个数
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int WIFI_scan(char *ifname, WIFISCAN wifiscan[], int maxSize);

/**
 * @fn int WIFI_check_list(WIFIINFO *wifiinfo)
 * @brief 	检测已经保存的wifi列表中是否有此wifi连接信息,如果有将把列表中的信息拷贝到wifiinfo
 *
 * @param wifiinfo		wifi信息保存的地址,需要将MAC地址和essid先放在此wifiinfo结构中
 *
 * @return 		wifi信息已有返回0,wifi信息将拷贝到wifiinfo中,没有查找到返回-1,wifi MAC地址错误返回-2
 *
 */
int WIFI_check_list(WIFIINFO *wifiinfo);

/**
 * @fn int WIFI_remove_info(char *addressMAC)
 * @brief 	移除列表中已经存在的wifi信息
 *
 * @param addressMAC	wifi MAC地址
 *
 * @return		成功返回0,失败返回-1
 *
 */
int WIFI_remove_info(char *addressMAC);

/**
 * @fn int WIFI_set_info(WIFIINFO *wifiinfo)
 * @brief 	设置wifi信息到wpa配置文件中,并且更新list列表
 *
 * @param wifiinfo	wifi信息地址
 *
 * @return		成功返回0,失败返回-1
 *
 */
int WIFI_set_info(WIFIINFO *wifiinfo);

/**
 * @fn int WIFI_get_curMAC(char *addressMAC)
 * @brief 	获取当前使用的WIFI的MAC地址
 *
 * @param addressMAC	MAC地址
 *
 * @return		成功返回0,失败或当前未使用返回-1
 *
 */
int WIFI_get_curMAC(char *addressMAC);

/**
 * @fn void WIFI_set_suspendModel(int model)
 * @brief 设置wifi的休眠模式
 * @param model	wifi的休眠模式,0是指wifi自动休眠,1是指wifi断电休眠
 *
 * @return 无返回值
 */
void WIFI_set_suspendModel(int model);

/**
 * @fn int WIFI_get_suspendModel(void)
 * @brief 获取wifi的休眠模式
 * @param 无参数
 *
 * @return 返回0表示wifi自动休眠,1是指wifi断电休眠
 */
int WIFI_get_suspendModel(void);

/**
 * @fn void Os__suspend_wifi(void)
 * @brief 休眠wifi
 * @param 无参数
 *
 * @return 无返回值
 */
void Os__suspend_wifi(void);

/**
 * @fn void Os__awake_wifi(void)
 * @brief 唤醒wifi
 * @param 无参数
 *
 * @return 无返回值
 */
void Os__awake_wifi(void);
#endif	/* WIFI_H_INCLUDED */
/** @} end of etc_fns */
