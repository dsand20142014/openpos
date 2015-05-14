#ifndef NETWORK_H_INCLUDE
#define NETWORK_H_INCLUDE

/**
 * @defgroup 网络函数库
 * @author 熊剑
 * @brief 提供对网络进行操作的相关函数
 * @{
 */

/**
 * @fn int Os__check_network(unsigned short uiTimeout, int netType)
 * @brief 	 检查网络是否联通,uiTimeout为超时时间,单位为20ms
 *
 * @param uiTimeout		检查网络超时时间,单位为20ms
 * @param netType		网络类型,0表示GSM网络,1表示WIFI网络
 *
 * @return 		成功返回0,失败返回-1
 *
 */
int Os__check_network(unsigned short uiTimeout, int netType);
#endif // NETWORK_H_INCLUDE
/** @} end of etc_fns */
